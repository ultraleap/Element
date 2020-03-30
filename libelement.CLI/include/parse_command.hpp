#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace cli
{
	struct parse_command_arguments {
		bool no_validation;
	};

	class parse_command : public command
	{
	public:
		parse_command(const common_command_arguments& common_arguments, const parse_command_arguments& arguments)
			: common_arguments{ common_arguments }, arguments{ arguments }
		{
		}

	private:
		const common_command_arguments& common_arguments;
		const parse_command_arguments& arguments;

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

			command->callback([callback, common_arguments, arguments]() { callback(std::move(*std::make_unique<parse_command>(*common_arguments, *arguments))); });
		}
	};
}