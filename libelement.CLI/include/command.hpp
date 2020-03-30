#pragma once

#include <memory>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

namespace cli
{
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
		virtual void execute() const = 0;

	public:
		static void configure(CLI::App& app, std::function<void(const command&)> callback);
	};

	class command_factory
	{

	};
}