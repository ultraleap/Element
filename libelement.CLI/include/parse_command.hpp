#pragma once

#include "command.hpp"
#include "compiler_message.hpp"

namespace libelement::cli
{
    struct parse_command_arguments
    {
        bool no_validation;

        [[nodiscard]] std::string as_string() const
        {
            std::stringstream ss;
            ss << "parse ";

            if (no_validation)
                ss << "--no-validation ";

            return ss.str();
        }
    };

    class parse_command final : public command
    {
        parse_command_arguments custom_arguments;

    public:
        parse_command(common_command_arguments common_arguments,
                      parse_command_arguments custom_arguments)
            : command(std::move(common_arguments))
            , custom_arguments{
                std::move(custom_arguments)
            }
        {
            element_interpreter_set_parse_only(context, custom_arguments.no_validation);
        }

        [[nodiscard]] compiler_message
        execute(const compilation_input& compilation_input) const override
        {
            const auto result = setup(compilation_input);
            if (result != ELEMENT_OK)
            {
                // todo: the test runner expects at least one PARSE_ERROR
                const auto parse_error = compiler_message(ELEMENT_ERROR_PARSE, "failed when parsing",
                                                          compilation_input.get_log_json());
                std::cout << parse_error.serialize() << std::endl;
                // expects the last thing to be True/False (not json)
                return compiler_message("False", compilation_input.get_log_json());
            }

            return compiler_message("True", compilation_input.get_log_json());
        }

        [[nodiscard]] std::string as_string() const override
        {
            std::stringstream ss;
            ss << custom_arguments.as_string() << " " << common_arguments.as_string();
            return ss.str();
        }

        static void
        configure(CLI::App& app,
                  const std::shared_ptr<common_command_arguments>& common_arguments,
                  command::callback callback)
        {
            const auto arguments = std::make_shared<parse_command_arguments>();

            auto* const command = app.add_subcommand("parse")->fallthrough();
            command->add_flag("--no-validation", arguments->no_validation,
                              "Expression to evaluate.");

            command->callback([callback, common_arguments, arguments]() {
                parse_command cmd(*common_arguments, *arguments);
                callback(cmd);
            });
        }
    };
} // namespace libelement::cli