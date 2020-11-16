#include "element/object.h"

//STD

//LIBS

//SELF
#include "element/common.h"
#include "interpreter_internal.hpp"
#include "instruction_tree/evaluator.hpp"
#include "object_model/object_internal.hpp"
#include "object_model/error.hpp"
#include "object_model/compilation_context.hpp"
#include "object_model/intermediaries/declaration_wrapper.hpp"

void element_object_delete(element_object** object)
{
    if (!object)
        return;

    delete *object;
    *object = nullptr;
    return;
}

element_result element_compilation_ctx_create(element_interpreter_ctx* interpreter, element_compilation_ctx** output)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *output = new element_compilation_ctx;
    (*output)->ctx = std::make_unique<element::compilation_context>(interpreter->global_scope.get(), interpreter);

    return ELEMENT_OK;
}

element_result element_compilation_ctx_delete(element_compilation_ctx** context)
{
    if (!context)
        return ELEMENT_OK;

    delete *context;
    (*context) = nullptr;

    return ELEMENT_OK;
}

element_result element_declaration_to_object(
    const element_declaration* declaration,
    element_object** output)
{
    if (!declaration || !declaration->decl)
        return ELEMENT_ERROR_API_DECLARATION_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    auto wrapped_declaration = std::make_shared<const element::declaration_wrapper>(declaration->decl);
    *output = new element_object{ std::move(wrapped_declaration) };
    return ELEMENT_OK;
}

element_result element_object_compile(
    const element_object* object,
    element_compilation_ctx* context,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    auto compiled = object->obj->compile(*context->ctx, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    const auto* err = dynamic_cast<const element::error*>((*output)->obj.get());
    if (err)
        return err->log_once(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_call(
    const element_object* object,
    element_compilation_ctx* context,
    const element_object* arguments,
    unsigned int arguments_count,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!arguments && arguments_count > 0)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    std::vector<element::object_const_shared_ptr> args;
    args.reserve(arguments_count);

    for (auto i = 0U; i < arguments_count; ++i)
        args.push_back(arguments[i].obj);

    auto compiled = object->obj->call(*context->ctx, std::move(args), object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    const auto* err = dynamic_cast<const element::error*>((*output)->obj.get());
    if (err)
        return err->log_once(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_index(
    const element_object* object,
    element_compilation_ctx* context,
    const char* index,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_COMPILATION_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!index)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    auto compiled = object->obj->index(*context->ctx, element::identifier{ index }, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    const auto* err = dynamic_cast<const element::error*>((*output)->obj.get());
    if (err)
        return err->log_once(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_to_instruction(const element_object* object, element_instruction** output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *output = new element_instruction;
    auto instr = object->obj->to_instruction();

    if (!instr)
    {
        (*output)->instruction = nullptr;
        return ELEMENT_ERROR_SERIALISATION;
    }

    (*output)->instruction = std::move(instr);
    return ELEMENT_OK;
}

element_result element_object_to_log_message(const element_object* object, element_log_message* output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

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

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    element_source_information src_info;
    const auto& obj_src_info = object->obj->source_info;
    src_info.character_start = obj_src_info.character_start;
    src_info.character_end = obj_src_info.character_end;
    src_info.line = obj_src_info.line;
    src_info.filename = (char*)calloc(strlen(obj_src_info.filename) + 1, sizeof(char));
    strcpy(src_info.filename, obj_src_info.filename);
    src_info.line_in_source = (char*)calloc(obj_src_info.line_in_source->length() + 1, sizeof(char));
    strcpy(src_info.line_in_source, obj_src_info.line_in_source->c_str());
    src_info.text = (char*)calloc(obj_src_info.get_text().length() + 1, sizeof(char));
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

    if (!buffer)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    strcpy(buffer, object->obj->typeof_info().c_str());

    return ELEMENT_OK;
}
