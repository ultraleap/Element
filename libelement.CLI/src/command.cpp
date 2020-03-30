#include <CLI/CLI.hpp>

#include <command.hpp>
#include <parse_command.hpp>
#include <evaluate_command.hpp>
#include <typeof_command.hpp>

void cli::command::configure(CLI::App& app, std::function<void(const command&)> callback)
{
	auto arguments = std::make_shared<common_command_arguments>();
	app.add_option("--no-prelude", arguments->no_prelude, "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.");
	app.add_option("--packages", arguments->packages, "Element packages to load into the context.");
	app.add_option("--source-files", arguments->source_files, "Extra individual source files to load into the context.");
	app.add_option("--debug", arguments->debug, "Preserves debug information while compiling.");
	app.add_option("--verbosity", arguments->verbosity, "Verbosity of compiler messages.");
	app.add_option("--logjson", arguments->log_json, "Serializes log messages structured as Json instead of plain string.");

	parse_command::configure(app, arguments, callback);
	evaluate_command::configure(app, arguments, callback);
	typeof_command::configure(app, arguments, callback);
}

