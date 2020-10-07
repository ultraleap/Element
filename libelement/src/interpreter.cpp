#include "element/interpreter.h"

//STD
#include <algorithm>
#include <functional>
#include <cassert>
#include <filesystem>
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
#include "object_model/object_model_builder.hpp"
#include "object_model/declarations/function_declaration.hpp"
#include "object_model/error.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/expressions/expression_chain.hpp"

element_result element_interpreter_create(element_interpreter_ctx** interpreter)
{
    *interpreter = new element_interpreter_ctx();
    return ELEMENT_OK;
}

void element_interpreter_delete(element_interpreter_ctx** interpreter)
{
    delete *interpreter;
    *interpreter = nullptr;
}

element_result element_interpreter_load_string(element_interpreter_ctx* interpreter, const char* string, const char* filename)
{
    assert(interpreter);
    return interpreter->load(string, filename);
}

element_result element_interpreter_load_file(element_interpreter_ctx* interpreter, const char* file)
{
    assert(interpreter);
    return interpreter->load_file(file);
}

element_result element_interpreter_load_files(element_interpreter_ctx* interpreter, const char** files, const int files_count)
{
    assert(interpreter);

    std::vector<std::string> actual_files;
    actual_files.resize(files_count);
    for (auto i = 0; i < files_count; ++i)
    {
        //std::cout << fmt::format("load_file {}\n", files[i]); //todo: proper logging
        actual_files[i] = files[i];
    }

    return interpreter->load_files(actual_files);
}

element_result element_interpreter_load_package(element_interpreter_ctx* interpreter, const char* package)
{
    assert(interpreter);
    //std::cout << fmt::format("load_package {}\n", package); //todo: proper logging
    return interpreter->load_package(package);
}

element_result element_interpreter_load_packages(element_interpreter_ctx* interpreter, const char** packages, const int packages_count)
{
    assert(interpreter);

    std::vector<std::string> actual_packages;
    actual_packages.resize(packages_count);
    for (auto i = 0; i < packages_count; ++i)
    {
        //std::cout << fmt::format("load_packages {}\n", packages[i]); //todo: proper logging
        actual_packages[i] = packages[i];
    }

    return interpreter->load_packages(actual_packages);
}

element_result element_interpreter_load_prelude(element_interpreter_ctx* interpreter)
{
    assert(interpreter);
    return interpreter->load_prelude();
}

void element_interpreter_set_log_callback(element_interpreter_ctx* interpreter, void (*log_callback)(const element_log_message*, void*), void* user_data)
{
    assert(interpreter);
    interpreter->set_log_callback(log_callback, user_data);
}

void element_interpreter_set_parse_only(element_interpreter_ctx* interpreter, bool parse_only)
{
    interpreter->parse_only = parse_only;
}

element_result element_interpreter_clear(element_interpreter_ctx* interpreter)
{
    assert(interpreter);
    return interpreter->clear();
}

element_result element_delete_declaration(element_declaration** declaration)
{
    delete *declaration;
    *declaration = nullptr;
    return ELEMENT_OK;
}

element_result element_delete_object(element_object** object)
{
    delete *object;
    *object = nullptr;
    return ELEMENT_OK;
}

element_result element_interpreter_find(element_interpreter_ctx* interpreter, const char* path, element_declaration** declaration)
{
    const auto* decl = interpreter->global_scope->find(element::identifier(path), false);
    if (!decl)
    {
        *declaration = nullptr;
        interpreter->log(ELEMENT_ERROR_IDENTIFIER_NOT_FOUND, fmt::format("API - failed to find '{}'.", path));
        return ELEMENT_ERROR_IDENTIFIER_NOT_FOUND;
    }

    //todo: don't need to new
    *declaration = new element_declaration{ decl };
    return ELEMENT_OK;
}

element_result valid_boundary_function(
    element_interpreter_ctx* interpreter,
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
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_object** object)
{
    if (!options)
        options = &element_compiler_options_default;

    const element::compilation_context compilation_context(interpreter->global_scope.get(), interpreter);

    if (!declaration->decl)
        return ELEMENT_ERROR_UNKNOWN;

    //todo: compiler option to disable/enable boundary function checking?
    const bool declaration_is_nullary = declaration->decl->get_inputs().empty();
    const bool check_boundary = declaration_is_nullary ? options->check_valid_boundary_function_when_nullary : options->check_valid_boundary_function;
    if (check_boundary)
    {
        const auto result = valid_boundary_function(interpreter, compilation_context, options, declaration);
        if (result != ELEMENT_OK)
        {
            interpreter->log(result, "Tried to compile a function but it failed as it is not valid on the boundary");
            *object = nullptr;
            return result;
        }
    }

    element_result result = ELEMENT_OK;
    auto compiled = compile_placeholder_expression(compilation_context, *declaration->decl, declaration->decl->get_inputs(), result, {});
    if (!compiled || result != ELEMENT_OK)
    {
        interpreter->log(result, "Tried to compile placeholders but it failed.");
        *object = nullptr;
        return result;
    }

    if (options->desired_result == element_compiler_options::compiled_result_kind::AUTOMATIC)
    {
        auto instruction = compiled->to_instruction();
        if (!instruction)
            *object = new element_object{ std::move(compiled) };
        else
            *object = new element_object{ std::move(instruction) };
    }
    else if (options->desired_result == element_compiler_options::compiled_result_kind::INSTRUCTION_TREE_ONLY)
    {
        *object = new element_object { compiled->to_instruction() };
        if (!*object)
        {
            interpreter->log(result, "Failed to compile declaration to an instruction tree.");
            return ELEMENT_ERROR_UNKNOWN;
        }
    }
    else if (options->desired_result == element_compiler_options::compiled_result_kind::OBJECT_MODEL_ONLY)
    {
        *object = new element_object{ std::move(compiled) };
    }

    return ELEMENT_OK;
}

element_result element_interpreter_evaluate(
    element_interpreter_ctx* interpreter,
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
        return err->log_once(interpreter->logger.get());

    auto expr = object->obj->to_instruction();
    if (!expr)
    {
        //todo: proper logging
        interpreter->logger->log("element_object is not an instruction tree, so it can't be evaluated", ELEMENT_STAGE_EVALUATOR);
        return ELEMENT_ERROR_UNKNOWN;
    }
    else
    {
        const auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_instruction_tree);

        if (log_expression_tree)
        {
            interpreter->log("\n------\nEXPRESSION\n------\n" + instruction_to_string(*expr));
        }
    }

    std::size_t count = outputs->count;
    const auto result = element_evaluate(
        *interpreter,
        std::move(expr),
        inputs->values,
        inputs->count,
        outputs->values,
        count,
        opts);
    outputs->count = static_cast<int>(count);

    if (result != ELEMENT_OK)
    {
        interpreter->log(result, fmt::format("Failed to evaluate {}", object->obj->typeof_info()), "<input>");
    }

    return result;
}

element_result element_interpreter_compile_expression(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const char* expression_string,
    element_object** object)
{
    const element::compilation_context compilation_context(interpreter->global_scope.get(), interpreter);

    element_tokeniser_ctx* tokeniser;
    auto result = element_tokeniser_create(&tokeniser);
    if (result != ELEMENT_OK)
        return result;

    tokeniser->logger = interpreter->logger;

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
    interpreter->src_context->file_info[data] = std::move(info);

    const auto log_tokens = flag_set(logging_bitmask, log_flags::debug | log_flags::output_tokens);

    if (log_tokens)
        interpreter->log("\n------\nTOKENS\n------\n" + tokens_to_string(tokeniser));

    element_parser_ctx parser;
    parser.tokeniser = tokeniser;
    parser.logger = interpreter->logger;
    parser.src_context = interpreter->src_context;

    element_ast root(nullptr);
    //root.nearest_token = &tokeniser->cur_token;
    parser.root = &root;

    size_t first_token = 0;
    auto* ast = parser.root->new_child(ELEMENT_AST_NODE_EXPRESSION);
    result = parser.parse_expression(&first_token, ast);
    if (result != ELEMENT_OK)
        return result;

    const auto log_ast = flag_set(logging_bitmask, log_flags::debug | log_flags::output_ast);

    if (log_ast)
        interpreter->log("\n---\nAST\n---\n" + ast_to_string(parser.root));

    //parse only enabled, skip object model generation to avoid error codes with positive values
    //i.e. errors returned other than ELEMENT_ERROR_PARSE
    if (interpreter->parse_only)
    {
        root.children.clear();
        return ELEMENT_OK;
    }

    //todo: urgh, this is horrible now...
    element::deferred_expressions deferred_expressions;
    auto dummy_identifier = element::identifier{ "<REMOVE>" };
    auto dummy_declaration = std::make_unique<element::function_declaration>(dummy_identifier, interpreter->global_scope.get(), element::function_declaration::kind::expression_bodied);
    parser.root->nearest_token = &tokeniser->tokens[0];
    element::assign_source_information(interpreter, dummy_declaration, parser.root);
    auto expression_chain = element::build_expression_chain(interpreter, ast, dummy_declaration.get(), deferred_expressions, result);

    if (deferred_expressions.empty())
    {
        dummy_declaration->body = std::move(expression_chain);
    }
    else
    {
        for (auto& [identifier, expression] : deferred_expressions)
        {
            element_result output_result = ELEMENT_OK;
            auto lambda = element::build_lambda_declaration(interpreter, identifier, expression, dummy_declaration->our_scope.get(), output_result);
            if (output_result != ELEMENT_OK)
                throw;

            const auto is_added = dummy_declaration->our_scope->add_declaration(std::move(lambda));
            if (!is_added)
            {
                //todo: error
            }
        }

        auto lambda_return_decl = std::make_unique<element::function_declaration>(element::identifier::return_identifier, dummy_declaration->our_scope.get(), element::function_declaration::kind::expression_bodied);
        element::assign_source_information(interpreter, lambda_return_decl, parser.root);
        lambda_return_decl->body = std::move(expression_chain);

        dummy_declaration->body = std::move(lambda_return_decl);
    }

    root.children.clear();

    if (result != ELEMENT_OK)
    {
        interpreter->log(result, fmt::format("building object model failed with element_result {}", result), info.file_name->data());
        return result;
    }

    bool success = interpreter->global_scope->add_declaration(std::move(dummy_declaration));
    if (!success)
    {
        (*object)->obj = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto* found_dummy_decl = interpreter->global_scope->find(dummy_identifier, false);
    assert(found_dummy_decl);
    auto compiled = found_dummy_decl->compile(compilation_context, found_dummy_decl->source_info);

    success = interpreter->global_scope->remove_declaration(dummy_identifier);
    if (!success)
    {
        (*object)->obj = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    //stuff from below
    (*object)->obj = std::move(compiled);
    return ELEMENT_OK;
}

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs)
{
    //sure there is a shorthand for this
    element_object object;
    auto* object_ptr = &object;
    element_interpreter_compile_expression(interpreter, nullptr, expression_string, &object_ptr);

    if (!object_ptr->obj)
    {
        //todo:
        interpreter->log(ELEMENT_ERROR_UNKNOWN, "tried to evaluate an expression that failed to compile");
        outputs->count = 0;
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<const element::error>(object_ptr->obj);
    if (err)
    {
        outputs->count = 0;
        return err->log_once(interpreter->logger.get());
    }

    const auto compiled = object_ptr->obj->to_instruction();
    if (!compiled)
    {
        interpreter->log(ELEMENT_ERROR_SERIALISATION, "failed to serialise", "<REMOVE>");
        outputs->count = 0;
        return ELEMENT_ERROR_SERIALISATION;
    }
    else
    {
        const auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_instruction_tree);

        if (log_expression_tree)
        {
            interpreter->log("\n------\nEXPRESSION\n------\n" + instruction_to_string(*compiled));
        }
    }

    float inputs[] = { 0 };
    element_inputs input;
    input.values = inputs;
    input.count = 1;

    const auto result = element_interpreter_evaluate(interpreter, options, object_ptr, &input, outputs);

    //todo: remove declaration added to global scope
    //todo: remove file_info added to interpreter source interpreter

    return result;
}

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    const int buffer_size)
{
    //sure there is a shorthand for this
    element_object object;
    auto* object_ptr = &object;
    element_interpreter_compile_expression(interpreter, nullptr, expression_string, &object_ptr);

    if (!object_ptr->obj)
    {
        //todo:
        interpreter->log(ELEMENT_ERROR_UNKNOWN, "tried to get typeof an expression that failed to compile");
        return ELEMENT_ERROR_UNKNOWN;
    }

    const auto err = std::dynamic_pointer_cast<const element::error>(object_ptr->obj);
    if (err)
        return err->log_once(interpreter->logger.get());

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