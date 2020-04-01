#include <iostream>

#include <CLI/CLI.hpp>

#include <compiler_message.hpp>
#include <command.hpp>
#include <toml.hpp>

using namespace libelement::cli;

void command_callback(const command& command) {

	//callback in case we need access to the command for some compiler_message generation shenanigans
	command.execute();

	//parse command response into complier message and return to the appropriate streams
	compiler_message message(10, message_level::ERROR, "parse_command", std::vector<trace_site>{});

	std::cout << message.serialize();
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