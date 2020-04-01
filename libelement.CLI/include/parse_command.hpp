#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace libelement::cli
{
	struct parse_command_arguments
	{
		bool no_validation;
	};

	class parse_command final : public command
	{
		common_command_arguments common_arguments;
		parse_command_arguments custom_arguments;

	public:
		parse_command(common_command_arguments common_arguments, parse_command_arguments custom_arguments)
			: common_arguments{ std::move(common_arguments) }, custom_arguments{ std::move(custom_arguments) }
		{
		}

		void execute() const override
		{
			//call into libelement
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, std::function<void(const command&)> callback)
		{
			const auto arguments = std::make_shared<parse_command_arguments>();

			const auto command = app.add_subcommand("parse")->fallthrough();
			command->add_option("no-validation", arguments->no_validation, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(parse_command(*common_arguments, *arguments)); });
		}
	};
}