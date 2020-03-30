#pragma once

#include "command.hpp"

struct parse_command_arguments {
	bool no_validation;
};

class parse_command : public command
{
private:

public:
	void command_implementation() override
	{
	}

	static void configure(CLI::App& app, std::shared_ptr<common_command_arguments> const& common_arguments)
	{
		auto arguments = std::make_shared<parse_command_arguments>();

		auto command = app.add_subcommand("parse")->fallthrough();
		command->add_option("no-validation", arguments->no_validation, "Expression to evaluate.")->required();

		command->callback([common_arguments, arguments]() { execute_command(*common_arguments, *arguments); });
	}

private:
	static void execute_command(common_command_arguments const& common_arguments, parse_command_arguments const& arguments) {

		//Call into libelement

		//Parse command response into complier message and return to the appropriate streams
		cli::compiler_message message(10, cli::message_level::ERROR, "parse_command", std::vector<cli::trace_site>{});

		std::cout << message.serialize();
	}
};