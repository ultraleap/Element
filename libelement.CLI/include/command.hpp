#pragma once

#include <memory>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

struct common_command_arguments {
	bool no_prelude;
	std::vector<std::string> packages{};
	std::vector<std::string> source_files{};
	bool debug;
	std::string verbosity;
	bool log_json;
};


class command 
{
public:
	virtual void command_implementation() = 0;

public:
	static void configure(CLI::App& app, std::shared_ptr<common_command_arguments> const& arguments)
	{
		app.add_option("--no-prelude", arguments->no_prelude, "Prelude functionality is included. Warning: Without Prelude only unverified compiler intrinsics will be available.");
		app.add_option("--packages", arguments->packages, "Element packages to load into the context.");
		app.add_option("--source-files", arguments->source_files, "Extra individual source files to load into the context.");
		app.add_option("--debug", arguments->debug, "Preserves debug information while compiling.");
		app.add_option("--verbosity", arguments->verbosity, "Verbosity of compiler messages.");
		app.add_option("--logjson", arguments->log_json, "Serializes log messages structured as Json instead of plain string.");
	}
};