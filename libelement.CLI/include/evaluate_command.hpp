#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace libelement::cli
{
	struct evaluate_command_arguments 
	{
		std::string expression;
	};

	class evaluate_command final : public command
	{
		common_command_arguments common_arguments;
		evaluate_command_arguments custom_arguments;

	public:
		evaluate_command(common_command_arguments common_arguments, evaluate_command_arguments custom_arguments)
			: common_arguments{ std::move(common_arguments) }, custom_arguments{ std::move(custom_arguments) }
		{
		}

		void execute() const override
		{
			//Call into libelement
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, std::function<void(const command&)> callback)
		{
			auto arguments = std::make_shared<evaluate_command_arguments>();

			auto command = app.add_subcommand("evaluate")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(evaluate_command(*common_arguments, *arguments)); });
		}
	};
}