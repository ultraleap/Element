#include <iostream>

#include <CLI/CLI.hpp>
#include <compiler_message.hpp>
#include <toml.hpp>

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

	std::cout << std::filesystem::current_path() << std::endl;

	//TEST: Toml parsing
	auto message_codes = cli::message_codes("config/Messages.toml");

	//TODO: Parse arguments and construct appropriate command
	CLI::App app{ "CLI interface for libelement" };
	app.set_help_all_flag("--help-all", "Expand all help");
	app.require_subcommand(1, 1); //require exactly one subcommand
	
	bool no_prelude;
	app.add_option("--no-prelude", no_prelude, "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.");

	std::vector<std::string> packages {};
	app.add_option("--packages", packages, "Element packages to load into the context.");

	std::vector<std::string> source_files {};
	app.add_option("--source-files", source_files, "Extra individual source files to load into the context.");

	bool debug;
	app.add_option("--debug", debug, "Preserves debug information while compiling.");

	std::string verbosity;
	app.add_option("--verbosity", verbosity, "Verbosity of compiler messages.");

	bool log_json;
	app.add_option("--logjson", log_json, "Serializes log messages structured as Json instead of plain string.");

	//expression
	auto evaluate_command = app.add_subcommand("evaluate")->fallthrough();

	std::string expression;
	evaluate_command->add_option("-e,--expression", expression, "Expression to evaluate.")->required();

	//parse
	auto parse_command = app.add_subcommand("parse")->fallthrough();

	bool no_validation;
	parse_command->add_option("--no-validation", no_validation, "Skip validating files after parsing. Only syntax correctness will be checked. Issues such as invalid/duplicate identifiers will not be caught.")->required();

	//typeof
	auto typeof_command = app.add_subcommand("typeof")->fallthrough();

	std::string typeof_expression;
	typeof_command->add_option("-e,--expression", typeof_expression, "Expression to evaluate.")->required();

	//hmm... args need to be parsed upfront, so all possible options need to be populated before CLI11_PARSE
	//can CLI11 be called again after initial parsing is complete?
	CLI11_PARSE(app, argc, argv);

	if (*evaluate_command) {
		std_out("evaluate_command");
	}

	if (*parse_command) {
		std_out("parse_command");
	}

	if (*typeof_command) {
		std_out("typeof_command");
	}

	//Parse command response into complier message and return to the appropriate streams
	cli::compiler_message message(10, cli::message_level::ERROR, "Hello World!", std::vector<cli::trace_site>{});

	std::cout << message.serialize();
}

