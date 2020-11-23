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
#include "log_errors.hpp"
#include "element/ast.h"
#include "ast/parser_internal.hpp"
#include "object_model/intrinsics/intrinsic.hpp"
#include "object_model/expressions/expression_chain.hpp"
#include "object_model/expressions/call_expression.hpp"
#include "object_model/intermediaries/function_instance.hpp"
#include "element/object.h"

element_result element_interpreter_create(element_interpreter_ctx** interpreter)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *interpreter = new element_interpreter_ctx();
    return ELEMENT_OK;
}

void element_interpreter_delete(element_interpreter_ctx** interpreter)
{
    if (!interpreter)
        return;

    delete *interpreter;
    *interpreter = nullptr;
}

element_result element_interpreter_load_string(element_interpreter_ctx* interpreter, const char* string, const char* filename)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!string || !filename)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    return interpreter->load(string, filename);
}

element_result element_interpreter_load_file(element_interpreter_ctx* interpreter, const char* file)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!file)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    return interpreter->load_file(file);
}

element_result element_interpreter_load_files(element_interpreter_ctx* interpreter, const char** files, const int files_count)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!files)
        return ELEMENT_ERROR_API_INVALID_INPUT;

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

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!package)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    //std::cout << fmt::format("load_package {}\n", package); //todo: proper logging
    return interpreter->load_package(package);
}

element_result element_interpreter_load_packages(element_interpreter_ctx* interpreter, const char** packages, const int packages_count)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!packages)
        return ELEMENT_ERROR_API_INVALID_INPUT;

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

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    return interpreter->load_prelude();
}

element_result element_interpreter_set_log_callback(element_interpreter_ctx* interpreter, element_log_callback log_callback, void* user_data)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    interpreter->set_log_callback(log_callback, user_data);
    return ELEMENT_OK;
}

element_result element_interpreter_set_parse_only(element_interpreter_ctx* interpreter, bool parse_only)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    interpreter->parse_only = parse_only;
    return ELEMENT_OK;
}

element_result element_interpreter_clear(element_interpreter_ctx* interpreter)
{
    assert(interpreter);

    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    return interpreter->clear();
}

void element_declaration_delete(element_declaration** declaration)
{
    if (!declaration)
        return;

    (*declaration)->decl = nullptr;
    delete *declaration;
    *declaration = nullptr;
}

void element_instruction_delete(element_instruction** instruction)
{
    if (!instruction)
        return;

    (*instruction)->instruction = nullptr;
    delete *instruction;
    *instruction = nullptr;
}

element_result element_interpreter_find(element_interpreter_ctx* interpreter, const char* path, element_declaration** declaration)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!path)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!declaration)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto* scope = interpreter->global_scope.get();

    const std::string delimiter = ".";
    const std::string full_path = path;
    const element::declaration* decl = nullptr;

    size_t start = 0;
    auto end = full_path.find(delimiter);
    if (end != std::string::npos)
    {
        //find all but last string
        while (end != std::string::npos)
        {
            const auto identifier = full_path.substr(start, end - start);

            start = end + delimiter.length();
            end = full_path.find(delimiter, start);

            decl = scope->find(element::identifier(identifier), false);
            scope = decl->get_scope();
        }
    }

    //find last string
    const auto identifier = full_path.substr(start, full_path.length() - start);
    decl = scope->find(element::identifier(identifier), false);
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

element_result element_interpreter_compile_declaration(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const element_declaration* declaration,
    element_instruction** instruction)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!declaration || !declaration->decl)
        return ELEMENT_ERROR_API_DECLARATION_IS_NULL;

    if (!instruction)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!options)
        options = &element_compiler_options_default;

    const element::compilation_context compilation_context(interpreter->global_scope.get(), interpreter);

    const bool declaration_is_nullary = declaration->decl->get_inputs().empty();
    const bool check_boundary = declaration_is_nullary ? options->check_valid_boundary_function_when_nullary : options->check_valid_boundary_function;
    if (check_boundary)
    {
        const auto result = valid_boundary_function(interpreter, compilation_context, options, declaration);
        if (result != ELEMENT_OK)
        {
            interpreter->log(result, "Tried to compile a function but it failed as it is not valid on the boundary");
            *instruction = nullptr;
            return result;
        }
    }

    element_result result = ELEMENT_OK;
    const auto compiled = compile_placeholder_expression(compilation_context, *declaration->decl, declaration->decl->get_inputs(), {});
    const auto* err = dynamic_cast<const element::error*>(compiled.get());
    if (err)
        result = err->log_once(interpreter->logger.get());

    if (!compiled || result != ELEMENT_OK)
    {
        interpreter->log(result, "Tried to compile placeholders but it failed.");
        *instruction = nullptr;
        return result;
    }

    auto instr = compiled->to_instruction();
    if (!instr)
    {
        interpreter->log(result, "Failed to compile declaration to an instruction tree.");
        *instruction = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    *instruction = new element_instruction{ std::move(instr) };
    return ELEMENT_OK;
}

element_result element_interpreter_evaluate_instruction(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const element_instruction* instruction,
    const element_inputs* inputs,
    element_outputs* outputs)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!instruction || !instruction->instruction)
        return ELEMENT_ERROR_API_INSTRUCTION_IS_NULL;

    if (!inputs)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    if (!outputs)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    element_evaluator_options opts{};

    if (options)
        opts = *options;

    const auto err = std::dynamic_pointer_cast<const element::error>(instruction->instruction);
    if (err)
        return err->log_once(interpreter->logger.get());

    const auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_instruction_tree);

    if constexpr (log_expression_tree)
        interpreter->log("\n------\nEXPRESSION\n------\n" + instruction_to_string(*instruction->instruction));

    std::size_t count = outputs->count;
    const auto result = element_evaluate(
        *interpreter,
        instruction->instruction,
        inputs->values,
        inputs->count,
        outputs->values,
        count,
        opts);
    outputs->count = static_cast<int>(count);

    if (result != ELEMENT_OK)
        interpreter->log(result, fmt::format("Failed to evaluate {}", instruction->instruction->typeof_info()), "<input>");

    return result;
}

element_result element_interpreter_compile_expression(
    element_interpreter_ctx* interpreter,
    const element_compiler_options* options,
    const char* expression_string,
    element_instruction** instruction)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!expression_string)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!instruction)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    element_object* object_ptr;
    auto result = interpreter->expression_to_object(options, expression_string, &object_ptr);

    *instruction = new element_instruction();

    if (result != ELEMENT_OK)
    {
        interpreter->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
        (*instruction)->instruction = nullptr;
        element_object_delete(&object_ptr);
        return result;
    }

    const auto* function_instance = dynamic_cast<const element::function_instance*>(object_ptr->obj.get());
    if (!function_instance)
    {
        interpreter->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
        auto instr = object_ptr->obj->to_instruction();
        if (!instr)
        {
            (*instruction)->instruction = nullptr;
            element_object_delete(&object_ptr);
            return ELEMENT_ERROR_UNKNOWN;
        }

        (*instruction)->instruction = std::move(instr);
        element_object_delete(&object_ptr);
        return ELEMENT_OK;
    }

    const element::compilation_context compilation_context(interpreter->global_scope.get(), interpreter);
    element_declaration declaration{ function_instance->declarer };
    result = valid_boundary_function(interpreter, compilation_context, options, &declaration);
    if (result != ELEMENT_OK)
    {
        interpreter->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
        interpreter->log(result, "Tried to compile a function but it failed as it is not valid on the boundary");
        *instruction = nullptr;
        element_object_delete(&object_ptr);
        return result;
    }
    
    result = ELEMENT_OK;
    const auto compiled = compile_placeholder_expression(compilation_context, *function_instance, function_instance->get_inputs(), {});
    const auto* err = dynamic_cast<const element::error*>(compiled.get());
    if (err)
        result = err->log_once(interpreter->logger.get());

    interpreter->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
    if (!compiled || result != ELEMENT_OK)
    {
        interpreter->log(result, "Tried to compile placeholders but it failed.");
        *instruction = nullptr;
        element_object_delete(&object_ptr);
        return result;
    }

    auto instr = compiled->to_instruction();
    if (!instr)
    {
        interpreter->log(result, "Failed to compile declaration to an instruction tree.");
        *instruction = nullptr;
        element_object_delete(&object_ptr);
        return ELEMENT_ERROR_UNKNOWN;
    }

    (*instruction)->instruction = std::move(instr);
    element_object_delete(&object_ptr);
    return ELEMENT_OK;
}

element_result element_interpreter_evaluate_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    element_outputs* outputs)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!expression_string)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!outputs)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    element_instruction instruction;
    auto* instruction_ptr = &instruction;
    auto result = element_interpreter_compile_expression(interpreter, nullptr, expression_string, &instruction_ptr);
    if (result != ELEMENT_OK)
    {
        outputs->count = 0;
        return result;
    }

    constexpr auto log_expression_tree = flag_set(logging_bitmask, log_flags::debug | log_flags::output_instruction_tree);
    if constexpr (log_expression_tree)
        interpreter->log("\n------\nEXPRESSION\n------\n" + instruction_to_string(*instruction.instruction));

    float inputs[] = { 0 };
    element_inputs input;
    input.values = inputs;
    input.count = 1;

    result = element_interpreter_evaluate_instruction(interpreter, options, instruction_ptr, &input, outputs);

    return result;
}

element_result element_interpreter_typeof_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* expression_string,
    char* buffer,
    const int buffer_size)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!expression_string)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!buffer)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    element_object* object_ptr;
    const auto result = interpreter->expression_to_object(nullptr, expression_string, &object_ptr);
    interpreter->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });

    if (result != ELEMENT_OK)
    {
        //todo:
        interpreter->log(result, "tried to get typeof an expression that failed to compile");
        element_object_delete(&object_ptr);
        return result;
    }

    const auto typeof = object_ptr->obj->typeof_info();
    if (buffer_size < static_cast<int>(typeof.size()))
    {
        //todo:
        interpreter->log(ELEMENT_ERROR_API_INSUFFICIENT_BUFFER, fmt::format("buffer size of {} isn't sufficient for string of {} characters", buffer_size, typeof.size()));
        element_object_delete(&object_ptr);
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;
    }

    strncpy(buffer, typeof.c_str(), typeof.size());
    element_object_delete(&object_ptr);
    return ELEMENT_OK;
}

element_result element_interpreter_evaluate_call_expression(
    element_interpreter_ctx* interpreter,
    const element_evaluator_options* options,
    const char* call_expression,
    element_outputs* outputs)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!call_expression)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    if (!outputs)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    std::vector<element::object_const_shared_ptr> arguments;
    const auto result = interpreter->call_expression_to_objects(nullptr, call_expression, arguments);
    if (result != ELEMENT_OK)
        return result;

    std::vector<std::vector<element_value>> serialised_arguments;
    for (const auto& arg : arguments)
    {
        const auto* err = dynamic_cast<const element::error*>(arg.get());
        if (err)
            return err->log_once(interpreter->logger.get());

        const auto instruction = arg->to_instruction();

        serialised_arguments.emplace_back();
        serialised_arguments.back().resize(1024);

        const auto eval_result = element_evaluate(
            *interpreter,
            instruction,
            {},
            serialised_arguments.back(),
            {});

        if (eval_result != ELEMENT_OK)
            return eval_result;
    }

    int serialised_size = 0;
    for (const auto& arg : serialised_arguments)
        serialised_size += arg.size();

    if (serialised_size > outputs->count)
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    outputs->count = 0;

    for (const auto& arg : serialised_arguments)
    {
        std::copy_n(arg.data(), arg.size(), outputs->values + outputs->count);
        outputs->count += arg.size();
    }

    return ELEMENT_OK;
}
