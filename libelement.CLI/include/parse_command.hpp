#pragma once

#include "command.hpp"

struct parse_command_arguments : common_command_arguments {
	bool no_validation;
};

class parse_command : public command
{
private:

public:
	void command_implementation() override
	{
	}

	static void configure(CLI::App& app)
	{
		auto arguments = std::make_shared<parse_command_arguments>();
		command::configure(app, *arguments);

		auto command = app.add_subcommand("parse")->fallthrough();
		command->add_option("no-validation", arguments->no_validation, "Expression to evaluate.")->required();

		command->callback([arguments]() { execute_command(*arguments); });
	}

private:
	static void execute_command(parse_command_arguments const& opt) {

		//Call into libelement

		//Parse command response into complier message and return to the appropriate streams
		cli::compiler_message message(10, cli::message_level::ERROR, "parse_command", std::vector<cli::trace_site>{});

		std::cout << message.serialize();
	}
};