#pragma once

#include <element/element.h>
#include <interpreter_internal.hpp>
#include <string>
#include <iostream>
#include <fstream>

#include "command.hpp"
#include "compiler_message.hpp"

#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#include "lmnt/interpreter.h"
#include "lmnt/compiler.hpp"
#include "lmnt/jit.h"

namespace libelement::cli
{
struct declaration_command_arguments
{
    std::string name;

    [[nodiscard]] std::string as_string() const
    {
        std::stringstream ss;
        ss  << "declaration --name \"" << name << "\"";
        return ss.str();
    }
};

class declaration_command final : public command
{
public:
    declaration_command(common_command_arguments common_arguments,
        declaration_command_arguments custom_arguments)
        : command(std::move(common_arguments))
        , custom_arguments{ std::move(custom_arguments) }
    {}

    [[nodiscard]] compiler_message execute(const compilation_input& compilation_input) const override
    {
        const auto result = setup(compilation_input);
        if (result != ELEMENT_OK)
            return compiler_message(error_conversion(result),
                "Failed to setup context",
                compilation_input.get_log_json());

        return get_declaration(compilation_input);
    }

    [[nodiscard]] compiler_message get_declaration(const compilation_input& compilation_input) const
    {
        element_declaration* decl = nullptr;
        element_result result = element_interpreter_find(context, custom_arguments.name.c_str(), &decl);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to find: " + custom_arguments.name + " with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        element_object_model_ctx* octx = nullptr;
        result = element_object_model_ctx_create(context, &octx);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to create object model with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        element_object* object = nullptr;
        result = element_declaration_to_object(decl, &object);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to create object with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        std::string declstr;
        result = get_declaration_string(octx, decl, object, declstr);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to generate declaration string with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        return compiler_message(declstr, compilation_input.get_log_json());
    }

    [[nodiscard]] std::string as_string() const override
    {
        std::stringstream ss;
        ss << custom_arguments.as_string() << " " << common_arguments.as_string();
        return ss.str();
    }

    static void configure(CLI::App& app,
        const std::shared_ptr<common_command_arguments>& common_arguments,
        callback callback)
    {
        const auto arguments = std::make_shared<declaration_command_arguments>();

        auto* command = app.add_subcommand("declaration")->fallthrough();

        command->add_option("-n,--name", arguments->name,
                    "Name of the Element declaration to print.")
            ->required();

        command->callback([callback, common_arguments, arguments]() {
            declaration_command cmd(*common_arguments, *arguments);
            callback(cmd);
        });
    }

private:
    declaration_command_arguments custom_arguments;

    element_result get_declaration_string(element_object_model_ctx* octx, element_declaration* decl, element_object* object, std::string& declstring) const
    {
        std::string name(512, '\0');
        size_t size = name.size();
        ELEMENT_OK_OR_RETURN(element_declaration_get_qualified_name(decl, name.data(), &size));
        name.resize(name.find_first_of('\0'));
        element_ports* inputs = nullptr;
        ELEMENT_OK_OR_RETURN(element_object_get_inputs(object, &inputs));
        size_t inputs_count = 0;
        ELEMENT_OK_OR_RETURN(element_ports_get_count(inputs, &inputs_count));
        element_port* output = nullptr;
        ELEMENT_OK_OR_RETURN(element_object_get_output(object, &output));

        std::stringstream ss;
        ss << name;
        if (inputs_count) ss << "(";
        for (size_t i = 0; i < inputs_count; ++i) {
            const element_port* input_i = nullptr;
            ELEMENT_OK_OR_RETURN(element_ports_get_port(inputs, i, &input_i));
            // name
            const char* input_name = nullptr;
            ELEMENT_OK_OR_RETURN(element_port_get_name(input_i, &input_name));
            // constraint annotation
            const char* input_constraint = nullptr;
            if (element_port_get_constraint_annotation(input_i, &input_constraint) != ELEMENT_OK)
                input_constraint = "any";
            // default value, if present
            std::string default_object_value;
            element_object* default_object = nullptr;
            if (element_port_get_default_object(input_i, octx, &default_object) == ELEMENT_OK) {
                // does it have a name?
                default_object_value.resize(512, '\0');
                // if (element_object_get_name(default_object, default_object_value.data(), default_object_value.size()) != ELEMENT_OK) {
                    // no name, get it as a constant
                    element_instruction* default_object_instr;
                    ELEMENT_OK_OR_RETURN(element_object_to_instruction(default_object, &default_object_instr));
                    size_t bufsize = default_object_value.size();
                    ELEMENT_OK_OR_RETURN(element_instruction_to_code(default_object_instr, default_object_value.data(), &bufsize));
                // }
                default_object_value.resize(default_object_value.find_first_of('\0'));
            }

            if (i) ss << ", ";
            ss << input_name << ":" << input_constraint;
            if (!default_object_value.empty())
                ss << " = " << default_object_value;
        }
        if (inputs_count) ss << "):";

        const char* output_constraint = nullptr;
        if (element_port_get_constraint_annotation(output, &output_constraint) != ELEMENT_OK)
            output_constraint = "any";
        ss << output_constraint;

        declstring = ss.str();
        return ELEMENT_OK;
    }
};
} // namespace libelement::cli