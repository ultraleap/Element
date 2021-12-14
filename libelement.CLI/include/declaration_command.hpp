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
    bool definition = false;

    [[nodiscard]] std::string as_string() const
    {
        std::stringstream ss;
        ss << "declaration --name \"" << name << "\"";
        if (definition)
            ss << " --definition";
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

        size_t code_size = 0;
        result = element_declaration_to_code(decl, custom_arguments.definition, nullptr, &code_size);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to convert declaration to code with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }
        std::string code(code_size, '\0');
        result = element_declaration_to_code(decl, custom_arguments.definition, code.data(), &code_size);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to convert declaration to code with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }
        code.resize(code_size - 1);

        return compiler_message(code, compilation_input.get_log_json());
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

        command->add_option("-n,--name", arguments->name, "Name of the Element declaration to print.")
            ->required();
        command->add_flag("--definition", arguments->definition, "Gets the full function definition rather than just the declaration.");

        command->callback([callback, common_arguments, arguments]() {
            declaration_command cmd(*common_arguments, *arguments);
            callback(cmd);
        });
    }

private:
    declaration_command_arguments custom_arguments;
};
} // namespace libelement::cli