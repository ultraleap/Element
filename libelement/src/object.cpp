#include "element/common.h"
#include "element/object.h"
#include "object_model/compilation_context.hpp"
#include "interpreter_internal.hpp"
#include "instruction_tree/evaluator.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"

element_result element_declaration_to_object(
    const element_declaration* declaration,
    element_object** output)
{
    auto wrapped_declaration = std::make_shared<const element::declaration_wrapper>(declaration->decl);
    *output = new element_object{ std::move(wrapped_declaration) };
    return ELEMENT_OK;
}

element_result element_object_compile(
    element_interpreter_ctx* context, 
    const element_object* compilable,
    element_object** output)
{
    const element_result result = ELEMENT_OK;

    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto compiled = compilable->obj->compile(compilation_context, compilable->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    return result;
}

element_result element_object_call(
    element_interpreter_ctx* context,
    const element_object* callable,
    const element_object arguments[],
    unsigned int arguments_count,
    element_object** output)
{
    const auto result = ELEMENT_OK;
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    std::vector<element::object_const_shared_ptr> args;
    args.reserve(arguments_count);

    for (auto i = 0U; i < arguments_count; ++i)
        args.push_back(arguments[i].obj);

    auto compiled = callable->obj->call(compilation_context, args, callable->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    return result;
}

element_result element_object_index(
    element_interpreter_ctx* context,
    const element_object* indexable,
    const char* index,
    element_object** output)
{
    const element_result result = ELEMENT_OK;
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto compiled = indexable->obj->index(compilation_context, element::identifier{ index }, indexable->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    return result;
}