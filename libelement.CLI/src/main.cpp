#include <iostream>

#include <CLI/CLI.hpp>

#include <compiler_message.hpp>
#include <command.hpp>
#include <toml.hpp>

using namespace libelement::cli;

void command_callback(const command& command) {

	command.execute();

	//Parse command response into complier message and return to the appropriate streams
	compiler_message message(10, message_level::ERROR, "parse_command", std::vector<trace_site>{});

	std::cout << message.serialize();
}

int main(const int argc, char** argv)
{
	auto codes = message_codes("config/Messages.toml"); //not doing anything useful with this yet

	//Parse arguments and construct appropriate command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1); //require exactly one subcommand

	command::configure(app, command_callback);

	CLI11_PARSE(app, argc, argv);
}