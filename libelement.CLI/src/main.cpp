#include <iostream>

#include <CLI/CLI.hpp>
#include <toml.hpp>

#include "compiler_message.hpp"
#include "command.hpp"

using namespace libelement::cli;

void log_callback(const element_log_message* const message)
{
	auto log = compiler_message(message);
	std::cout << log.serialize() << std::endl;
}

void command_callback(const command& command) 
{
	command.set_log_callback(log_callback);

	//feedback request
	auto request = compiler_message(command.as_string());
	std::cout << request.serialize() << std::endl;

	//callback in case we need access to the command for some compiler_message generation shenanigans
	auto input = compilation_input(command.get_common_arguments());
	auto response = command.execute(input);
	std::cout << response.serialize() << std::endl;
} 

int main(const int argc, char** argv)
{
	//parse arguments and construct exactly one required command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1);

	command::configure(app, command_callback);

	CLI11_PARSE(app, argc, argv);
}