#include <iostream>

#include <CLI/CLI.hpp>

#include <compiler_message.hpp>
#include <command.hpp>
#include <toml.hpp>

void command_callback(const cli::command& command) {

	command.execute();

	//Parse command response into complier message and return to the appropriate streams
	cli::compiler_message message(10, cli::message_level::ERROR, "parse_command", std::vector<cli::trace_site>{});

	std::cout << message.serialize();
}

int main(int argc, char** argv)
{
	auto message_codes = cli::message_codes("config/Messages.toml"); //not doing anything useful with this yet

	//Parse arguments and construct appropriate command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1); //require exactly one subcommand

	cli::command::configure(app, [](const cli::command& command) { command_callback(command); });

	CLI11_PARSE(app, argc, argv);
}

