#pragma once

#include <string>

#include "command.hpp"

namespace libelement::cli
{
	struct typeof_command_arguments 
	{
		std::string expression;
	};

	class typeof_command final : public command
	{
		typeof_command_arguments custom_arguments;

	public:
		typeof_command(common_command_arguments common_arguments, typeof_command_arguments custom_arguments)
			: command(common_arguments), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			//call into libelement

			//default move constructor should trigger on return value assignment, right?
			return compiler_message(10, message_level::ERROR, "typeof_command", std::vector<trace_site>{});
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<typeof_command_arguments>();

			const auto command = app.add_subcommand("typeof")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(typeof_command(*common_arguments, *arguments)); });
		}
	};
}