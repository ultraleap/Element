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
#include "object_model/expressions/expression_chain.hpp"
#include "object_model/expressions/call_expression.hpp"

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
    const auto starts_with_prelude = file.rfind("Prelude/", 0) == 0;
    element_tokeniser_ctx* tokeniser;
    ELEMENT_OK_OR_RETURN(element_tokeniser_create(&tokeniser));

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
    ELEMENT_OK_OR_RETURN(element_tokeniser_run(tokeniser, str, info.file_name.get()->data()));
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
    ELEMENT_OK_OR_RETURN(result);

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

    auto package_path = "ElementPackages/" + actual_package_name;
    if (!directory_exists(package_path))
    {
        auto abs = std::filesystem::absolute(std::filesystem::path(package_path)).string();
        std::string msg = fmt::format("package {} does not exist at path {}\n",
                                      package_path, abs);
        element_result result = ELEMENT_ERROR_DIRECTORY_NOT_FOUND;
        log(result, msg);
        return result;
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
        auto package_path = "ElementPackages/" + package;
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

void element_interpreter_ctx::log(element_result code, const std::string & message) const
{
    if (logger == nullptr)
    {
        return;
    }
    logger->log(*this, code, message);
}

void element_interpreter_ctx::log(const std::string& message) const
{
    if (logger == nullptr)
        return;

    logger->log(message, element_stage::ELEMENT_STAGE_MISC);
}


element_result element_interpreter_ctx::call_expression_to_objects(
    const element_compiler_options* options,
    const char* call_expression_string,
    std::vector<element::object_const_shared_ptr>& objects)
{
    if (!call_expression_string)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    element_tokeniser_ctx* tokeniser;
    auto result = element_tokeniser_create(&tokeniser);
    if (result != ELEMENT_OK)
        return result;

    tokeniser->logger = logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto element_tokeniser_delete_ptr = [](element_tokeniser_ctx* tokeniser) {
        element_tokeniser_delete(&tokeniser);
    };

    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(element_tokeniser_delete_ptr)>(tokeniser, element_tokeniser_delete_ptr);

    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>("<REMOVE>");

    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    result = element_tokeniser_run(tokeniser, call_expression_string, info.file_name->data());
    if (result != ELEMENT_OK)
        return result;

    if (tokeniser->tokens.empty())
        return ELEMENT_OK;

    const auto total_lines_parsed = tokeniser->line;

    //lines start at 1
    for (auto i = 0; i < total_lines_parsed; ++i)
        info.source_lines.emplace_back(std::make_unique<std::string>(tokeniser->text_on_line(i + 1)));

    auto* const data = info.file_name->data();
    //todo: remove file_info added to interpreter source interpreter
    src_context->file_info[data] = std::move(info);

    const auto log_tokens = flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
        log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = logger;
    parser.src_context = src_context;

    element_ast root(nullptr);
    parser.root = &root;
    parser.current_token = tokeniser->get_token(0, result);
    root.nearest_token = parser.current_token;

    if (result != ELEMENT_OK)
        return result;

    result = parser.parse_exprlist(root);
    if (result != ELEMENT_OK)
        return result;

    const auto log_ast = flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
        log("\n---\nAST\n---\n" + ast_to_string(parser.root));

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (parse_only)
    {
        root.children.clear();
        return ELEMENT_OK;
    }

    auto declaration = std::make_unique<element::function_declaration>(element::identifier{ "DUMMY" }, global_scope.get(), element::function_declaration::kind::expression_bodied);
    auto chain = std::make_unique<element::expression_chain>(declaration.get());
    chain->expressions.emplace_back(std::make_unique<element::call_expression>(nullptr)); //create empty expression so build_call_expression doesn't fail
    assign_source_information(this, chain, parser.root->children[0].get());
    auto expr = element::build_call_expression(this, parser.root->children[0].get(), chain.get(), result);
    auto* call_expr = static_cast<const element::call_expression*>(expr.get());

    root.children.clear();

    if (result != ELEMENT_OK)
    {
        log(result, fmt::format("building object model failed with element_result {}", result), src_context->file_info[data].file_name->data());
        return result;
    }

    const element::compilation_context compilation_context(global_scope.get(), this);

    for (const auto& arg : call_expr->arguments)
        objects.emplace_back(arg->compile(compilation_context, call_expr->source_info));

    return ELEMENT_OK;
}

element_result element_interpreter_ctx::call_expression_to_objects(
    const element_compiler_options* options,
    const char* call_expression,
    element_object** objects,
    int* object_count)
{
    if (!call_expression)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!objects)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    std::vector<element::object_const_shared_ptr> objs;
    call_expression_to_objects(options, call_expression, objs);

    *objects = new element_object[objs.size()];
    *object_count = objs.size();

    for (int i = 0; i < objs.size(); ++i)
        (*objects)[i].obj = std::move(objs[i]);

    return ELEMENT_OK;
}

element_result element_interpreter_ctx::expression_to_object(
    const element_compiler_options* options,
    const char* expression_string,
    element_object** object)
{
    if (!expression_string)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!object)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *object = new element_object();

    const element::compilation_context compilation_context(global_scope.get(), this);

    element_tokeniser_ctx* tokeniser;
    auto result = element_tokeniser_create(&tokeniser);
    if (result != ELEMENT_OK)
        return result;

    tokeniser->logger = logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto element_tokeniser_delete_ptr = [](element_tokeniser_ctx* tokeniser) {
        element_tokeniser_delete(&tokeniser);
    };

    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(element_tokeniser_delete_ptr)>(tokeniser, element_tokeniser_delete_ptr);

    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>("<REMOVE>");

    //hack: forcing terminal on expression
    std::string expr = std::string(expression_string);
    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    result = element_tokeniser_run(tokeniser, expr.c_str(), info.file_name->data());
    if (result != ELEMENT_OK)
        return result;

    if (tokeniser->tokens.empty())
        return ELEMENT_OK;

    const auto total_lines_parsed = tokeniser->line;

    //lines start at 1
    for (auto i = 0; i < total_lines_parsed; ++i)
        info.source_lines.emplace_back(std::make_unique<std::string>(tokeniser->text_on_line(i + 1)));

    auto* const data = info.file_name->data();
    //todo: remove file_info added to interpreter source interpreter
    src_context->file_info[data] = std::move(info);

    const auto log_tokens = flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
        log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = logger;
    parser.src_context = src_context;

    element_ast root(nullptr);
    parser.root = &root;
    parser.current_token = tokeniser->get_token(0, result);
    root.nearest_token = parser.current_token;

    if (result != ELEMENT_OK)
        return result;

    result = parser.parse_expression(root);
    if (result != ELEMENT_OK)
        return result;

    const auto log_ast = flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
        log("\n---\nAST\n---\n" + ast_to_string(parser.root));

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (parse_only)
    {
        root.children.clear();
        return ELEMENT_OK;
    }

    auto dummy_identifier = element::identifier{ "<REMOVE>" };
    auto dummy_declaration = std::make_unique<element::function_declaration>(dummy_identifier, global_scope.get(), element::function_declaration::kind::expression_bodied);
    parser.root->nearest_token = &tokeniser->tokens[0];
    element::assign_source_information(this, dummy_declaration, parser.root);
    auto expression_chain = build_expression_chain(this, parser.root->children[0].get(), dummy_declaration.get(), result);
    dummy_declaration->body = std::move(expression_chain);

    root.children.clear();

    if (result != ELEMENT_OK)
    {
        log(result, fmt::format("building object model failed with element_result {}", result), info.file_name->data());
        return result;
    }

    bool success = global_scope->add_declaration(std::move(dummy_declaration));
    if (!success)
    {
        (*object)->obj = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto* found_dummy_decl = global_scope->find(dummy_identifier, false);
    assert(found_dummy_decl);
    auto compiled = found_dummy_decl->compile(compilation_context, found_dummy_decl->source_info);

    if (!compiled)
    {
        (*object)->obj = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    (*object)->obj = std::move(compiled);

    const auto* err = dynamic_cast<const element::error*>((*object)->obj.get());
    if (err)
        return err->log_once(logger.get());

    return ELEMENT_OK;
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


