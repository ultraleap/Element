#include <iostream>

#include <CLI/CLI.hpp>
#include <compiler_message.hpp>
#include <toml.hpp>

int main(int argc, char** argv)
{

	std::cout << std::filesystem::current_path() << std::endl;

	auto message_codes = cli::message_codes("config/Messages.toml");

	CLI::App app{ "CLI interface for libelement" };

	std::string filename = "default";
	app.add_option("-f,--file", filename, "A  help string");


	CLI11_PARSE(app, argc, argv);

	cli::compiler_message message(10, cli::message_level::ERROR, "Hello World!", std::vector<cli::trace_site>{});

	std::cout << message.serialize();
}