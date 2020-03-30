#pragma once

#include "command.hpp"

struct evaluate_command_arguments {
	std::string expression;
};

class evaluate_command : public command
{
public:
	 void command_implementation() override 
	 {
	 }

	 static void configure(CLI::App& app, std::shared_ptr<common_command_arguments> const& common_arguments)
	 {
		 auto arguments = std::make_shared<evaluate_command_arguments>();

		 auto command = app.add_subcommand("evaluate")->fallthrough();
		 command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

		 command->callback([common_arguments, arguments]() { execute_command(*common_arguments, *arguments); });
	 }

private:
	static void execute_command(common_command_arguments const& common_arguments, evaluate_command_arguments const& arguments) {

		//Call into libelement

		//Parse command response into complier message and return to the appropriate streams
		cli::compiler_message message(10, cli::message_level::ERROR, "evaluate_command", std::vector<cli::trace_site>{});

		std::cout << message.serialize();
	}
};