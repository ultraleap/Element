#include "interpreter_internal.hpp"

//STD
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

//LIBS
#include <fmt/format.h>

//SELF
#include "element/ast.h"
#include "instruction_tree/evaluator.hpp"
#include "ast/parser_internal.hpp"
#include "common_internal.hpp"
#include "token_internal.hpp"
#include "configuration.hpp"
#include "log_errors.hpp"
#include "object_model/intrinsics/intrinsic.hpp"
#include "object_model/object_model_builder.hpp"
#include "object_model/error_map.hpp"

void element_interpreter_ctx::Deleter::operator()(element::intrinsic* i) const { delete i; }
void element_interpreter_ctx::Deleter::operator()(const element::intrinsic* i) const { delete i; }

static bool file_exists(const std::string& file)
{
    return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
}

static bool directory_exists(const std::string& directory)
{
    return std::filesystem::exists(directory) && std::filesystem::is_directory(directory);
}

element_result element_interpreter_ctx::load_into_scope(const char* str, const char* filename, element::scope* src_scope)
{
    //HACK: JM - Not a fan of this...
    const std::string file = filename;
    const auto starts_with_prelude = file.rfind("Prelude\\", 0) == 0;
    element_tokeniser_ctx* tokeniser;
    ELEMENT_OK_OR_RETURN(element_tokeniser_create(&tokeniser))

    auto element_tokeniser_delete_ptr = [](element_tokeniser_ctx* tokeniser) {
        element_tokeniser_delete(&tokeniser);
    };

    tokeniser->logger = logger;
    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(element_tokeniser_delete_ptr)>(tokeniser, element_tokeniser_delete_ptr);

    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>(filename);
    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    ELEMENT_OK_OR_RETURN(element_tokeniser_run(tokeniser, str, info.file_name.get()->data()))
    const auto total_lines_parsed = tokeniser->line;

    for (auto i = 0; i < total_lines_parsed; ++i)
    {
        //lines start at 1
        info.source_lines.emplace_back(std::make_unique<std::string>(tokeniser->text_on_line(i + 1)));
    }

    auto* const data = info.file_name->data();
    src_context->file_info[data] = std::move(info);

    const auto log_tokens = starts_with_prelude
                                ? flag_set(logging_bitmask, log_flags::output_prelude) && flag_set(logging_bitmask, log_flags::output_tokens)
                                : flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
    {
        log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));
    }

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = logger;
    parser.src_context = src_context;

    auto result = parser.ast_build();
    ELEMENT_OK_OR_RETURN(result)

    const auto log_ast = starts_with_prelude
                             ? flag_set(logging_bitmask, log_flags::output_prelude) && flag_set(logging_bitmask, log_flags::output_ast)
                             : flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
    {
        log("\n---\nAST\n---\n" + ast_to_string(parser.root));
    }

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (parse_only)
    {
        element_ast_delete(&parser.root);
        return ELEMENT_OK;
    }

    auto object_model = element::build_root_scope(this, parser.root, result);
    element_ast_delete(&parser.root);

    if (result != ELEMENT_OK)
    {
        log(result, fmt::format("building object model failed with element_result {}", result), filename);
        return result;
    }

    result = src_scope->merge(std::move(object_model));
    if (result != ELEMENT_OK)
    {
        log(result, fmt::format("merging object models failed with element_result {}", result), filename);
        return result;
    }

    return result;
}

element_result element_interpreter_ctx::load(const char* str, const char* filename)
{
    return load_into_scope(str, filename, global_scope.get());
}

element_result element_interpreter_ctx::load_file(const std::string& file)
{
    const auto abs = std::filesystem::absolute(std::filesystem::path(file)).string();

    if (!file_exists(abs))
    {
        std::cout << fmt::format("file {} was not found at path {}\n", file, abs.c_str()); //todo: proper logging
        return ELEMENT_ERROR_FILE_NOT_FOUND;
    }

    std::string buffer;

    std::ifstream f(abs);
    f.seekg(0, std::ios::end);
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(buffer.data(), buffer.size());

    const auto result = load(buffer.c_str(), abs.c_str());
    return result;
}

element_result element_interpreter_ctx::load_files(const std::vector<std::string>& files)
{
    element_result ret = ELEMENT_OK;

    for (const auto& filename : files)
    {
        const element_result result = load_file(filename);
        if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_package(const std::string& package)
{
    const auto last_dash = package.find_last_of('-');
    auto actual_package_name = package;
    if (last_dash != std::string::npos)
        actual_package_name = package.substr(0, last_dash);

    auto package_path = "ElementPackages\\" + actual_package_name;
    if (!directory_exists(package_path))
    {
        auto abs = std::filesystem::absolute(std::filesystem::path(package_path)).string();
        std::cout << fmt::format("package {} does not exist at path {}\n",
                                 package_path, abs); //todo: proper logging
        return ELEMENT_ERROR_DIRECTORY_NOT_FOUND;
    }

    element_result ret = ELEMENT_OK;

    for (const auto& file : std::filesystem::recursive_directory_iterator(package_path))
    {
        const auto filename = file.path().string();
        const auto extension = file.path().extension().string();
        if (extension == ".ele")
        {
            const element_result result = load_file(file.path().string());
            if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
                ret = result;
        }
        else if (extension != ".bond")
        {
            std::cout << fmt::format("file {} in package {} has extension {} instead of '.ele' or '.bond'\n",
                                     filename, package_path, extension); //todo: proper logging
        }
    }

    return ret;
}

element_result element_interpreter_ctx::load_packages(const std::vector<std::string>& packages)
{
    element_result ret = ELEMENT_OK;

    for (const auto& package : packages)
    {
        auto package_path = "ElementPackages\\" + package;
        const auto result = load_package(package);
        if (result != ELEMENT_OK && ret != ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_prelude()
{
    if (prelude_loaded)
        return ELEMENT_ERROR_PRELUDE_ALREADY_LOADED;

    const auto result = load_package("Prelude");
    if (result == ELEMENT_OK)
    {
        prelude_loaded = true;
        return result;
    }

    if (result == ELEMENT_ERROR_DIRECTORY_NOT_FOUND)
    {
        auto abs = std::filesystem::absolute(std::filesystem::path("Prelude")).string();
        std::cout << fmt::format("could not find prelude at {}\n", abs); //todo: proper logging
    }

    return result;
}

void element_interpreter_ctx::set_log_callback(LogCallback callback, void* user_data)
{
    logger = std::make_shared<element_log_ctx>();
    logger->callback = callback;
    logger->user_data = user_data;
}

void element_interpreter_ctx::log(element_result code, const std::string& message, const std::string& filename) const
{
    if (logger == nullptr)
        return;

    logger->log(*this, code, message, filename);
}

void element_interpreter_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return;

    logger->log(message, element_stage::ELEMENT_STAGE_MISC);
}

element_interpreter_ctx::element_interpreter_ctx()
{
    element::detail::register_errors();
    element::detail::register_log_errors();

    // TODO: hack, remove
    global_scope = std::make_unique<element::scope>(nullptr, nullptr);
    clear();
    src_context = std::make_shared<element::source_context>();
}

element_result element_interpreter_ctx::clear()
{
    //todo: ?
    //trees.clear();
    //names.reset();
    //ast_names.clear();

    return ELEMENT_OK;
}