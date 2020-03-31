#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace libelement::cli
{
	struct parse_command_arguments
	{
		bool no_validation;
	};

	class parse_command : public command
	{
	private:
		common_command_arguments common_arguments;
		parse_command_arguments arguments;

	public:
		parse_command(common_command_arguments common_arguments, parse_command_arguments arguments)
			: common_arguments{ std::move(common_arguments) }, arguments{ std::move(arguments) }
		{
		}

	public:
		void execute() const override
		{
			//Call into libelement
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, std::function<void(const command&)> callback)
		{
			auto arguments = std::make_shared<parse_command_arguments>();

			auto command = app.add_subcommand("parse")->fallthrough();
			command->add_option("no-validation", arguments->no_validation, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(parse_command(*common_arguments, *arguments)); });
		}
	};
}