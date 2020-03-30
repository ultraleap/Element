#include <iostream>

#include <CLI/CLI.hpp>
#include <compiler_message.hpp>
#include <toml.hpp>

#include <parse_command.hpp>
#include <evaluate_command.hpp>
#include <typeof_command.hpp>

//Probably overkill to pass these in as std::function callbacks
void std_out(const std::string& data)
{
	std::cout << data << std::endl;
}

void std_err(const std::string& data)
{
	std::cerr << data << std::endl;
}

int main(int argc, char** argv)
{
	auto message_codes = cli::message_codes("config/Messages.toml");

	//Parse arguments and construct appropriate command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1); //require exactly one subcommand


	auto arguments = std::make_shared<common_command_arguments>();
	command::configure(app, arguments);
	parse_command::configure(app, arguments);
	evaluate_command::configure(app, arguments);
	typeof_command::configure(app, arguments);

	CLI11_PARSE(app, argc, argv);
}

