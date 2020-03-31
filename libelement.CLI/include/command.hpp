#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

namespace libelement::cli
{
	struct common_command_arguments
	{
		bool no_prelude = false;
		std::vector<std::string> packages{};
		std::vector<std::string> source_files{};
		bool debug = false;
		std::string verbosity;
		bool log_json = false;
	};

	class command
	{
	public:
		command() = default;
		command(const command&) = delete;
		command(command&&) = delete;
		virtual ~command() = default;
		virtual void execute() const = 0;

		static void configure(CLI::App& app, std::function<void(const command&)> callback);
	};
}