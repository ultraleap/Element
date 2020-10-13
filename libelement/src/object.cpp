#include "element/object.h"

//STD

//LIBS

//SELF
#include "element/common.h"
#include "interpreter_internal.hpp"
#include "instruction_tree/evaluator.hpp"
#include "object_model/error.hpp"
#include "object_model/compilation_context.hpp"
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
    const element_object* object,
    const element_object* arguments,
    unsigned int arguments_count,
    element_object** output)
{
    const auto result = ELEMENT_OK;
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    std::vector<element::object_const_shared_ptr> args;
    args.reserve(arguments_count);

    for (auto i = 0U; i < arguments_count; ++i)
        args.push_back(arguments[i].obj);

    auto compiled = object->obj->call(compilation_context, args, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    return result;
}

element_result element_object_index(
    element_interpreter_ctx* context,
    const element_object* object,
    const char* index,
    element_object** output)
{
    const element_result result = ELEMENT_OK;
    const element::compilation_context compilation_context(context->global_scope.get(), context);

    auto compiled = object->obj->index(compilation_context, element::identifier{ index }, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    return result;
}

element_result element_object_to_instruction(const element_object* object, element_instruction** output)
{
    if (!object)
    {
        (*output)->instruction = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    auto instr = object->obj->to_instruction();

    if (!instr)
    {
        (*output)->instruction = nullptr;
        return ELEMENT_ERROR_UNKNOWN;
    }

    (*output)->instruction = std::move(instr);

    return ELEMENT_OK;
}

element_result element_object_to_log_message(const element_object* object, element_log_message* output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    const auto* err = dynamic_cast<const element::error*>(object->obj.get());
    if (err)
    {
        *output = err->get_log_message();
        return ELEMENT_OK;
    }

    return ELEMENT_ERROR_API_OBJECT_IS_NOT_ERROR;
}

element_result element_object_get_source_information(const element_object* object, element_source_information* output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    element_source_information src_info;
    const auto& obj_src_info = object->obj->source_info;
    src_info.character_start = obj_src_info.character_start;
    src_info.character_end = obj_src_info.character_end;
    src_info.line = obj_src_info.line;
    strcpy(src_info.filename, obj_src_info.filename);
    strcpy(src_info.line_in_source, obj_src_info.line_in_source->c_str());
    strcpy(src_info.text, obj_src_info.get_text().c_str());
    *output = src_info;

    return ELEMENT_OK;
}

element_result element_object_get_typeof(const element_object* object, char* buffer, int buffer_size)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (buffer_size < static_cast<int>(object->obj->typeof_info().size()))
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    strcpy(buffer, object->obj->typeof_info().c_str());

    return ELEMENT_OK;
}
