#pragma once

#include <command.hpp>
#include <compiler_message.hpp>

namespace cli
{
	struct typeof_command_arguments {
		std::string expression;
	};

	class typeof_command : public command
	{
	public:
		typeof_command(common_command_arguments common_arguments, typeof_command_arguments arguments)
			: common_arguments{ std::move(common_arguments) }, arguments{ std::move(arguments) }
		{
		}

	private:
		common_command_arguments common_arguments;
		typeof_command_arguments arguments;

	public:
		void execute() const override
		{
			//Call into libelement
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, std::function<void(const command&)> callback)
		{
			auto arguments = std::make_shared<typeof_command_arguments>();

			auto command = app.add_subcommand("typeof")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(typeof_command(*common_arguments, *arguments)); });
		}
	};
}