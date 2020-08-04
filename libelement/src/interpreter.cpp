#include "interpreter_internal.hpp"

//STD
#include <algorithm>
#include <functional>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

//LIBS
#include <fmt/format.h>

//SELF
#include "common_internal.hpp"
#include "etree/evaluator.hpp"
#include "etree/expressions.hpp"
#include "token_internal.hpp"
#include "configuration.hpp"
#include "object_model/object_model_builder.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/error.hpp"
#include "object_model/error_map.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/expressions/expression_chain.hpp"
#include "log_errors.hpp"
#include "element/ast.h"
#include "object_model/intrinsics/intrinsic.hpp"

void element_interpreter_ctx::Deleter::operator()(element::intrinsic* i) { delete i; }
void element_interpreter_ctx::Deleter::operator()(const element::intrinsic* i) { delete i; }

bool file_exists(const std::string& file)
{
    return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
}

bool directory_exists(const std::string& directory)
{
    return std::filesystem::exists(directory) && std::filesystem::is_directory(directory);
}

element_result element_interpreter_ctx::load(const char* str, const char* filename)
{
	//HACK: JM - Not a fan of this...
    const std::string file = filename;
    const auto starts_with_prelude = file.rfind("Prelude\\", 0) == 0;
	
    element_tokeniser_ctx* tokeniser;
    ELEMENT_OK_OR_RETURN(element_tokeniser_create(&tokeniser))

    tokeniser->logger = logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(tokeniser, element_tokeniser_delete);

    //create the file info struct to be used by the object model later
    element::file_information info;
    info.file_name = std::make_unique<std::string>(filename);
    //pass the pointer to the filename, so that the pointer stored in tokens matches the one we have
    ELEMENT_OK_OR_RETURN(element_tokeniser_run(tokeniser, str, info.file_name.get()->data()))
    if (tokeniser->tokens.empty())
        return ELEMENT_OK;

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
	
    if (log_tokens) {
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
	
    if (log_ast) {
        log("\n---\nAST\n---\n" + ast_to_string(parser.root));
    }


    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (parse_only)
    {
        element_ast_delete(parser.root);
        return ELEMENT_OK;
    }

    auto object_model = element::build_root_scope(this, parser.root, result);
    element_ast_delete(parser.root);

    if (result != ELEMENT_OK) {
        log(result, fmt::format("building object model failed with element_result {}", result), filename);
        return result;
    }

    result = global_scope->merge(std::move(object_model));
    if (result != ELEMENT_OK) {
        log(result, fmt::format("merging object models failed with element_result {}", result), filename);
        return result;
    }

    return result;
}

element_result element_interpreter_ctx::load_file(const std::string& file)
{
    if (!file_exists(file)) {
        const auto abs = std::filesystem::absolute(std::filesystem::path(file)).string();
        std::cout << fmt::format("file {} was not found at path {}\n",
            file, abs.c_str()); //todo: proper logging
        return ELEMENT_ERROR_FILE_NOT_FOUND;
    }

    std::string buffer;

    std::ifstream f(file);
    f.seekg(0, std::ios::end);
    buffer.resize(f.tellg());
    f.seekg(0);
    f.read(buffer.data(), buffer.size());

    const auto result = load(buffer.c_str(), file.c_str());
    if (result != ELEMENT_OK) {
        //std::cout << fmt::format("interpreter failed to parse file {}. element_result = {}\n", 
        //    file, result); //todo: proper logging
    }

    return result;
}

element_result element_interpreter_ctx::load_files(const std::vector<std::string>& files)
{
    element_result ret = ELEMENT_OK;

    for (const auto& filename : files) {
        if (!file_exists(filename)) {
            auto abs = std::filesystem::absolute(std::filesystem::path(filename)).string();
            std::cout << fmt::format("file {} was not found at path {}\n",
                filename, abs); //todo: proper logging
            continue; //todo: error handling
        }

        const element_result result = load_file(filename);
        if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
            ret = result;
    }

    return ret;
}

element_result element_interpreter_ctx::load_package(const std::string& package)
{
    auto package_path = "Content\\" + package;
    if (!directory_exists(package_path))
    {
        auto abs = std::filesystem::absolute(std::filesystem::path(package_path)).string();
        std::cout << fmt::format("package {} does not exist at path {}\n",
            package_path, abs); //todo: proper logging
        return ELEMENT_ERROR_DIRECTORY_NOT_FOUND;
    }

    element_result ret = ELEMENT_OK;

    for (const auto& file : std::filesystem::recursive_directory_iterator(package_path)) {
        const auto filename = file.path().string();
        const auto extension = file.path().extension().string();
        if (extension == ".ele")
        {
            const element_result result = load_file(file.path().string());
            if (result != ELEMENT_OK && ret == ELEMENT_OK) //todo: only returns first error
                ret = result;
        }
        else
        {
            std::cout << fmt::format("file {} in package {} has extension {} instead of '.ele'\n", 
                filename, package_path, extension); //todo: proper logging
        }
    }

    return ret;
}

element_result element_interpreter_ctx::load_packages(const std::vector<std::string>& packages)
{
    element_result ret = ELEMENT_OK;

    for (const auto& package : packages) {
        auto package_path = "Content\\" + package;
        if (!directory_exists(package_path)) {
            auto abs = std::filesystem::absolute(std::filesystem::path(package)).string();
            std::cout << fmt::format("package {} was not found at location {}\n",
                package, abs); //todo: proper logging
            continue; //todo: error handling
        }

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

    auto result = load_package("Prelude");
    if (result == ELEMENT_OK) {
        prelude_loaded = true;
        return result;
    }

    if (result == ELEMENT_ERROR_DIRECTORY_NOT_FOUND) {
        auto abs = std::filesystem::absolute(std::filesystem::path("Prelude")).string();
        std::cout << fmt::format("could not find prelude at {}\n", abs); //todo: proper logging
    }

    return result;
}

void element_interpreter_ctx::set_log_callback(LogCallback callback)
{
    logger = std::make_shared<element_log_ctx>();
    logger->callback = callback;
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

    logger->log(message, message_stage::ELEMENT_STAGE_MISC);
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
    //trees.clear();
    //names.reset();
    //ast_names.clear();

    return ELEMENT_OK;
}

element_result element_interpreter_create(element_interpreter_ctx** context)
{
    *context = new element_interpreter_ctx();
    return ELEMENT_OK;
}

void element_interpreter_delete(element_interpreter_ctx* context)
{
    delete context;
}

element_result element_interpreter_load_string(element_interpreter_ctx* context, const char* string, const char* filename)
{
    assert(context);
    return context->load(string, filename);
}

element_result element_interpreter_load_file(element_interpreter_ctx* context, const char* file)
{
    assert(context);
    return context->load_file(file);
}

element_result element_interpreter_load_files(element_interpreter_ctx* context, const char** files, const int files_count)
{
    assert(context);

    std::vector<std::string> actual_files;
    actual_files.resize(files_count);
    for (auto i = 0; i < files_count; ++i) {
        //std::cout << fmt::format("load_file {}\n", files[i]); //todo: proper logging
        actual_files[i] = files[i];
    }

    return context->load_files(actual_files);
}

element_result element_interpreter_load_package(element_interpreter_ctx* context, const char* package)
{
    assert(context);
    //std::cout << fmt::format("load_package {}\n", package); //todo: proper logging
    return context->load_package(package);
}

element_result element_interpreter_load_packages(element_interpreter_ctx* context, const char** packages, const int packages_count)
{
    assert(context);

    std::vector<std::string> actual_packages;
    actual_packages.resize(packages_count);
    for (auto i = 0; i < packages_count; ++i) {
        //std::cout << fmt::format("load_packages {}\n", packages[i]); //todo: proper logging
        actual_packages[i] = packages[i];
    }

    return context->load_packages(actual_packages);
}

element_result element_interpreter_load_prelude(element_interpreter_ctx* context)
{
    assert(context);
    return context->load_prelude();
}

void element_interpreter_parse_only_mode(element_interpreter_ctx* context, bool parse_only)
{
    assert(context);
    context->parse_only = parse_only;
}

void element_interpreter_set_log_callback(element_interpreter_ctx* context, void (*log_callback)(const element_log_message* const))
{
    assert(context);
    context->set_log_callback(log_callback);
}

element_result element_interpreter_clear(element_interpreter_ctx* context)
{
    assert(context);
    return context->clear();
}

element_result element_delete_declaration(element_interpreter_ctx* context, element_declaration** declaration)
{
    delete * declaration;
    *declaration = nullptr;
    return ELEMENT_OK;
}

element_result element_delete_object(element_interpreter_ctx* context, element_object** object)
{
    delete * object;
    *object = nullptr;
    return ELEMENT_OK;
}

element_result element_interpreter_find(element_interpreter_ctx* context, const char* path, element_declaration** declaration)
{
    const auto* decl = context->global_scope->find(element::identifier(path), false);
    if (!decl)
    {
        *declaration = nullptr;
        context->log(ELEMENT_ERROR_IDENTIFIER_NOT_FOUND, fmt::format("failed to find '{}'.", path));
        return ELEMENT_ERROR_IDENTIFIER_NOT_FOUND;
    }

    //todo: don't need to new
    *declaration = new element_declaration{decl};
    return ELEMENT_OK;
}

element_result valid_boundary_function(
    element_interpreter_ctx* context,
    const element::compilation_context& compilation_context,
    const element_compiler_options* options,
    const element_declaration* declaration)
{
    const auto func_decl = dynamic_cast<const element::function_declaration*>(declaration->decl);
    if (!func_decl)
        return ELEMENT_ERROR_UNKNOWN;

    const bool is_valid = func_decl->valid_at_boundary(compilation_context);
    if (!is_valid)
        return ELEMENT_ERROR_INVALID_BOUNDARY_FUNCTION_INTERFACE;

    return ELEMENT_OK;
}

element_result element_interpreter_compile(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_object** object)
{
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    if (!declaration->decl)
        return ELEMENT_ERROR_UNKNOWN;

    //todo: compiler option to disable/enable boundary function checking?
    if (!declaration->decl->get_inputs().empty())
    {
        auto result = valid_boundary_function(context, compilation_context, options, declaration);
        if (result != ELEMENT_OK)
        {
            context->log(result, "Tried to compile a function that requires inputs, making it a boundary function, but it can't be a boundary function.");
            *object = nullptr;
            return result;
        }
    }

    element_result result = ELEMENT_OK;
    auto compiled = compile_placeholder_expression(compilation_context, *declaration->decl, declaration->decl->get_inputs(), result, {});
    if(!compiled)
        return ELEMENT_ERROR_UNKNOWN;

    auto expression = compiled->to_expression();
    if (!expression)
        *object = new element_object{ std::move(compiled) };
    else
        *object = new element_object{ std::move(expression) };

    return ELEMENT_OK;
}

element_result element_interpreter_evaluate(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const element_object* object,
    const element_inputs* inputs,
    element_outputs* outputs)
{
    element_evaluator_options opts{};

    if (options)
        opts = *options;

    if (!object->obj)
    {
        assert(!"tried to evaluate something but it's nullptr");
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<const element::error>(object->obj);
    if (err)
        return err->log_once(context->logger.get());

    auto expr = object->obj->to_expression();
    if (!expr)
    {
        //todo: proper logging
        context->logger->log("evaluable is not an expression tree, so it can't be evaluated", ELEMENT_STAGE_EVALUATOR);
        return ELEMENT_ERROR_UNKNOWN;
    }
    else
    {
        const auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_expression_tree);

        if (log_expression_tree) {
            context->log("\n------\nEXPRESSION\n------\n" + expression_to_string(*expr));
        }
    }

    std::size_t count = outputs->count;
    const auto result = element_evaluate(
        *context,
        std::move(expr),
        inputs->values,
        inputs->count,
        outputs->values,
        count,
        opts);
    outputs->count = static_cast<int>(count);

    if (result != ELEMENT_OK) {
        context->log(result, fmt::format("Failed to evaluate {}", object->obj->typeof_info()), "<input>");
    }

    return result;
}

element_result element_interpreter_compile_expression(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const char* expression_string,
    element_object** object)
{
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    element_tokeniser_ctx* tokeniser;
    auto result = element_tokeniser_create(&tokeniser);
    if (result != ELEMENT_OK)
        return result;

    tokeniser->logger = context->logger;

    // Make a smart pointer out of the tokeniser so it's deleted on an early return
    auto tctx = std::unique_ptr<element_tokeniser_ctx, decltype(&element_tokeniser_delete)>(tokeniser, element_tokeniser_delete);

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
    context->src_context->file_info[data] = std::move(info);

    const auto log_tokens = flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
        context->log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = context->logger;
    parser.src_context = context->src_context;

    element_ast root(nullptr);
    //root.nearest_token = &tokeniser->cur_token;
    parser.root = &root;

    size_t first_token = 0;
    auto* ast = ast_new_child(&root, ELEMENT_AST_NODE_EXPRESSION);
    result = parser.parse_expression(&first_token, ast);
    if (result != ELEMENT_OK)
        return result;

    const auto log_ast = flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
        context->log("\n---\nAST\n---\n" + ast_to_string(parser.root));

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (context->parse_only)
    {
        root.children.clear();
        return ELEMENT_OK;
    }

    //todo: urgh, this is horrible now...
    element::deferred_expressions deferred_expressions;
    auto dummy_declaration = std::make_unique<element::function_declaration>(element::identifier{ "<REMOVE>" }, context->global_scope.get(), false);
    parser.root->nearest_token = &tokeniser->tokens[0];
    element::assign_source_information(context, dummy_declaration, parser.root);
    auto expression_chain = element::build_expression_chain(context, ast, dummy_declaration.get(), deferred_expressions, result);
    dummy_declaration->body = std::move(expression_chain);
    root.children.clear();

    if (result != ELEMENT_OK)
    {
        context->log(result, fmt::format("building object model failed with element_result {}", result), info.file_name->data());
        return result;
    }

    const bool success = context->global_scope->add_declaration(std::move(dummy_declaration));
    if (!success)
    {
        (*object)->obj = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto* found_dummy_decl = context->global_scope->find(element::identifier{ "<REMOVE>" }, false);
    assert(found_dummy_decl);
    auto compiled = found_dummy_decl->compile(compilation_context, found_dummy_decl->source_info);

    //stuff from below
    (*object)->obj = std::move(compiled);
    return ELEMENT_OK;
}

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs)
{
    //sure there is a shorthand for this
    element_object object;
    auto* object_ptr = &object;
    element_interpreter_compile_expression(context, nullptr, expression_string, &object_ptr);

    if (!object_ptr->obj)
    {
        //todo:
        context->log(ELEMENT_ERROR_UNKNOWN, "tried to evaluate an expression that failed to compile");
        outputs->count = 0;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<const element::error>(object_ptr->obj);
    if (err)
    {
        outputs->count = 0;
        return err->log_once(context->logger.get());
    }

    const auto compiled = object_ptr->obj->to_expression();
    if (!compiled)
    {
        context->log(ELEMENT_ERROR_SERIALISATION, "failed to serialise", "<REMOVE>");
        outputs->count = 0;
        return ELEMENT_ERROR_SERIALISATION;
    }
    else
    {
        const auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_expression_tree);

        if (log_expression_tree) {
            context->log("\n------\nEXPRESSION\n------\n" + expression_to_string(*compiled));
        }
    }

    float inputs[] = { 0 };
    element_inputs input;
    input.values = inputs;
    input.count = 1;

    const auto result = element_interpreter_evaluate(context, options, object_ptr, &input, outputs);

    //todo: remove declaration added to global scope
    //todo: remove file_info added to interpreter source context

    return result;
}

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* context,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    const int buffer_size)
{
    //sure there is a shorthand for this
    element_object object;
    auto* object_ptr = &object;
    element_interpreter_compile_expression(context, nullptr, expression_string, &object_ptr);

    if (!object_ptr->obj)
    {
        //todo:
        context->log(ELEMENT_ERROR_UNKNOWN, "tried to get typeof an expression that failed to compile");
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<const element::error>(object_ptr->obj);
    if (err)
        return err->log_once(context->logger.get());

    const auto typeof = object.obj->typeof_info();
    if (buffer_size < static_cast<int>(typeof.size()))
        return ELEMENT_ERROR_UNKNOWN;

    strncpy(buffer, typeof.c_str(), typeof.size());
    return ELEMENT_OK;
}

//element_result element_metainfo_for_evaluable(const element_evaluable* evaluable, element_metainfo** metainfo)
//{
//    //todo: error checking and stuff
//
//    *metainfo = new element_metainfo();
//    (*metainfo)->typeof = evaluable->evaluable->typeof_info();
//    (*metainfo)->code = evaluable->evaluable->to_code(0);
//
//    return ELEMENT_OK;
//}
//
//element_result element_metainfo_get_typeof(const element_metainfo* metainfo, char* buffer, const int buffer_size)
//{
//    if (buffer_size < static_cast<int>(metainfo->typeof.size()))
//        return ELEMENT_ERROR_UNKNOWN;
//
//    #define _CRT_SECURE_NO_WARNINGS
//    strncpy(buffer, metainfo->typeof.c_str(), metainfo->typeof.size());
//    #undef _CRT_SECURE_NO_WARNINGS
//
//    return ELEMENT_OK;
//}