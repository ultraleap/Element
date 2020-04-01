#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <compiler_message.hpp>

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

	//not sure if this is required, depends on how we choose to pipe information into libelement
	class compilation_input
	{
		common_command_arguments common_arguments;

	public:
		compilation_input(common_command_arguments common_arguments)
			: common_arguments{ std::move(common_arguments) }
		{
			common_arguments.source_files = select(common_arguments.source_files, file_exists);
			common_arguments.packages = select(common_arguments.packages, directory_exists);
		}

	private:
		static bool file_exists(const std::string& file)
		{
			return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
		}

		static bool directory_exists(const std::string& directory)
		{
			return std::filesystem::exists(directory) && std::filesystem::is_directory(directory);
		}

		template<typename T, typename Predicate> std::vector<T> select(const std::vector<T>& container, Predicate predicate)
		{
			std::vector<T> result;
			std::copy_if(container.begin(), container.end(), back_inserter(result), predicate);
			return result;
		}
	};

	class command
	{
		common_command_arguments common_arguments;

	public:
		command(common_command_arguments common_arguments)
			: common_arguments{ std::move(common_arguments) }
		{
		}

		common_command_arguments get_common_arguments() const 
		{
			return common_arguments;
		}

		using callback = std::function<void(const command&)>;

		virtual compiler_message execute(const compilation_input& input) const = 0;

		static void configure(CLI::App& app, command::callback callback);
	};
}