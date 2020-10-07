#include "element/common.h"
#include "element/interpreter_object_model.h"
#include "object_model/compilation_context.hpp"
#include "interpreter_internal.hpp"
#include "instruction_tree/evaluator.hpp"

element_result element_object_model_compile(
    element_interpreter_ctx* context,
    const element_compiler_options* options, 
    const element_declaration* declaration,
    element_object** object)
{
    if (!options)
        options = &element_compiler_options_default;

    const element_result result = ELEMENT_OK;

    const element::compilation_context compilation_context(context->global_scope.get(), context);
    auto compiled = declaration->decl->compile(compilation_context, declaration->decl->source_info);

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
        *object = new element_object{ compiled->to_instruction() };
        if (!*object)
        {
            context->log(result, "Failed to resolve to an instruction tree.");
            return ELEMENT_ERROR_UNKNOWN;
        }
    }
    else if (options->desired_result == element_compiler_options::compiled_result_kind::OBJECT_MODEL_ONLY)
    {
        *object = new element_object{ std::move(compiled) };
    }

    return result;
}

element_result element_object_model_call(
    element_interpreter_ctx* context,
    const element_compiler_options* options, 
    const element_declaration* callable,
    element_object** object)
{
    if (!options)
        options = &element_compiler_options_default;

    const element_result result = ELEMENT_OK;

    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto compiled = callable->decl->call(compilation_context, {}, callable->decl->source_info);
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
        *object = new element_object{ compiled->to_instruction() };
        if (!*object)
        {
            context->log(result, "Failed to resolve to an instruction tree.");
            return ELEMENT_ERROR_UNKNOWN;
        }
    }
    else if (options->desired_result == element_compiler_options::compiled_result_kind::OBJECT_MODEL_ONLY)
    {
        *object = new element_object{ std::move(compiled) };
    }

    return result;
}

element_result element_object_model_index(
    element_interpreter_ctx* context,
    const element_compiler_options* options,
    const element_declaration* indexable,
    const char* index,
    element_object** object)
{
    if (!options)
        options = &element_compiler_options_default;

    const element_result result = ELEMENT_OK;
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto compiled = indexable->decl->index(compilation_context, element::identifier{ index }, indexable->decl->source_info);
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
        *object = new element_object{ compiled->to_instruction() };
        if (!*object)
        {
            context->log(result, "Failed to resolve to an instruction tree.");
            return ELEMENT_ERROR_UNKNOWN;
        }
    }
    else if (options->desired_result == element_compiler_options::compiled_result_kind::OBJECT_MODEL_ONLY)
    {
        *object = new element_object{ std::move(compiled) };
    }

    return result;
}