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
#include "object_model/intermediaries/function_instance.hpp"
#include "object_model/expressions/expression_chain.hpp"

void element_object_delete(element_object** object)
{
    if (!object)
        return;

    delete *object;
    *object = nullptr;
}

element_result element_object_model_ctx_create(element_interpreter_ctx* interpreter, element_object_model_ctx** output)
{
    if (!interpreter)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *output = new element_object_model_ctx;
    (*output)->ctx = std::make_unique<element::compilation_context>(interpreter->global_scope.get(), interpreter);

    return ELEMENT_OK;
}

void element_object_model_ctx_delete(element_object_model_ctx** context)
{
    if (!context)
        return;

    delete *context;
    *context = nullptr;
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

element_result element_object_simplify(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    auto compiled = object->obj->compile(*context->ctx, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    if ((*output)->obj->is_error())
        return (*output)->obj->log_any_error(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_call(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** arguments,
    unsigned int arguments_count,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!arguments && arguments_count > 0)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    std::vector<element::object_const_shared_ptr> args;
    args.reserve(arguments_count);

    for (auto i = 0U; i < arguments_count; ++i)
        args.push_back(arguments[i]->obj);

    auto compiled = object->obj->call(*context->ctx, std::move(args), object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    if ((*output)->obj->is_error())
        return (*output)->obj->log_any_error(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_call_with_placeholders(
    const element_object* object,
    element_object_model_ctx* context,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto placeholder_offset = context->ctx->boundaries[0].size;

    auto compiled = compile_placeholder_expression(
        *context->ctx,
        *object->obj,
        object->obj->get_inputs(),
        {},
        placeholder_offset,
        0);

    *output = new element_object{ std::move(compiled) };

    if ((*output)->obj->is_error())
        return (*output)->obj->log_any_error(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_index(
    const element_object* object,
    element_object_model_ctx* context,
    const char* index,
    element_object** output)
{
    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (!index)
        return ELEMENT_ERROR_API_INVALID_INPUT;

    auto compiled = object->obj->index(*context->ctx, element::identifier{ index }, object->obj->source_info);
    *output = new element_object{ std::move(compiled) };

    if ((*output)->obj->is_error())
        return (*output)->obj->log_any_error(context->ctx->get_logger());

    return ELEMENT_OK;
}

element_result element_object_to_instruction(const element_object* object, element_object_model_ctx* context, element_instruction** output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!context || !context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *output = new element_instruction;
    auto instr = object->obj->to_instruction(*context->ctx->interpreter);

    if (!instr) {
        (*output)->instruction = nullptr;
        (*output)->cache = {};
        return ELEMENT_ERROR_SERIALISATION;
    }

    (*output)->cache = element::instruction_cache(instr.get());
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
    if (err) {
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

element_result element_object_get_name(const element_object* object, char* buffer, size_t buffer_size)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!buffer)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    const auto str = object->obj->get_name();
    if (buffer_size < str.size())
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    strcpy(buffer, str.c_str());
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

element_result element_object_get_inputs(const element_object* object, element_ports** inputs)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!inputs)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *inputs = new element_ports();
    for (const auto& input : object->obj->get_inputs()) {
        auto port = std::unique_ptr<element_port, element_ports::port_deleter>(
            new element_port{ &input },
            [](element_port* port) {
                element_port_delete(&port);
            });

        (*inputs)->ports.push_back(std::move(port));
    }

    return ELEMENT_OK;
}

element_result element_object_get_output(const element_object* object, element_port** output)
{
    if (!object || !object->obj)
        return ELEMENT_ERROR_API_OBJECT_IS_NULL;

    if (!output)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *output = new element_port{ &object->obj->get_output() };
    return ELEMENT_OK;
}

element_result element_ports_get_port(const element_ports* ports, size_t index, element_port** port)
{
    if (!ports)
        return ELEMENT_ERROR_API_PORTS_IS_NULL;

    if (!port)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    if (index >= ports->ports.size())
        return ELEMENT_ERROR_API_INVALID_INPUT;

    *port = ports->ports[index].get();
    return ELEMENT_OK;
}

element_result element_ports_get_count(const element_ports* ports, size_t* count)
{
    if (!ports)
        return ELEMENT_ERROR_API_PORTS_IS_NULL;

    if (!count)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *count = ports->ports.size();
    return ELEMENT_OK;
}

void element_ports_delete(element_ports** ports)
{
    if (!ports)
        return;

    delete *ports;
    *ports = nullptr;
}

element_result element_port_get_name(element_port* port, const char** name)
{
    if (!port)
        return ELEMENT_ERROR_API_PORT_IS_NULL;

    if (!name)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *name = port->port->get_name().c_str();
    return ELEMENT_OK;
}

element_result element_port_get_constraint_annotation(element_port* port, const char** annotation)
{
    if (!port)
        return ELEMENT_ERROR_API_PORT_IS_NULL;

    if (!annotation)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto* type = port->port->get_annotation();
    if (!type)
        return ELEMENT_OK;

    *annotation = type->to_string().c_str();
    return ELEMENT_OK;
}

element_result element_port_get_constraint_object(element_port* port, element_object_model_ctx* object_model_context, element_object** object)
{
    if (!port)
        return ELEMENT_ERROR_API_PORT_IS_NULL;

    if (!object_model_context || !object_model_context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto* decl = port->port->resolve_annotation(*object_model_context->ctx);
    if (!decl)
        return ELEMENT_OK;

    *object = new element_object();
    (*object)->obj = decl->compile(*object_model_context->ctx, {});

    return ELEMENT_OK;
}

element_result element_port_get_default_object(element_port* port, element_object_model_ctx* object_model_context, element_object** object)
{
    if (!port)
        return ELEMENT_ERROR_API_PORT_IS_NULL;

    if (!object_model_context || !object_model_context->ctx)
        return ELEMENT_ERROR_API_OBJECT_MODEL_CTX_IS_NULL;

    if (!object)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto* default_chain = port->port->get_default();
    if (!default_chain)
        return ELEMENT_OK;

    *object = new element_object();
    (*object)->obj = default_chain->compile(*object_model_context->ctx, {});

    return ELEMENT_OK;
}

void element_port_delete(element_port** port)
{
    if (!port)
        return;

    delete *port;
    *port = nullptr;
}
