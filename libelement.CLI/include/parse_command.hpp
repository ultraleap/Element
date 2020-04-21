#pragma once

#include "command.hpp"
#include "compiler_message.hpp"

namespace libelement::cli
{
	struct parse_command_arguments
	{
		bool no_validation;

		std::string as_string() const
		{
			std::stringstream ss;
			ss << "--parse ";

			if (no_validation)
				ss << "--no-validation ";

			return ss.str();
		}
	};

	class parse_command final : public command
	{
		parse_command_arguments custom_arguments;

	public:
		parse_command(common_command_arguments common_arguments, parse_command_arguments custom_arguments)
			: command(common_arguments), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			element_result result = ELEMENT_OK;
			result = setup(input);

			if (result != ELEMENT_OK)
			{
				return generate_response("False");
			}

			return generate_response("True");
		}

		std::string as_string() const override
		{
			std::stringstream ss;
			ss << custom_arguments.as_string() << " " << common_arguments.as_string();
			return ss.str();
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<parse_command_arguments>();

			const auto command = app.add_subcommand("parse")->fallthrough();
			command->add_flag("--no-validation", arguments->no_validation, "Expression to evaluate.");

			command->callback([callback, common_arguments, arguments]() { callback(parse_command(*common_arguments, *arguments)); });
		}
	};
}