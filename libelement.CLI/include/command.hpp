#pragma once

#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include "element/interpreter.h"

#include "compiler_message.hpp"

namespace libelement::cli
{
	struct common_command_arguments
	{
		bool no_prelude = false;
		bool debug = false;
		bool log_json = false;
		std::string verbosity;
		std::vector<std::string> packages{};
		std::vector<std::string> source_files{};

		std::string as_string() const 
		{
			std::stringstream ss;
			
			if (no_prelude)
				ss << "--no-prelude ";

			if (debug)
				ss << "--debug ";

			if (log_json)
				ss << "--log-json ";

			if (!verbosity.empty())
				ss << "--verbosity " << verbosity << " ";

			if (!packages.empty()) 
			{
				ss << "--packages ";

				for (const auto& package : packages) 
					ss << package << " ";
			}

			if (!source_files.empty())
			{
				ss << "--source-files ";

				for (const auto& source_file : source_files)
					ss << source_file << " ";
			}

			return ss.str();
		}
	};

	//not sure if this is required, depends on how we choose to pipe information into libelement
	class compilation_input
	{
		common_command_arguments common_arguments;

	public:
		compilation_input(common_command_arguments arguments)
			: common_arguments{ std::move(arguments) }
		{
			common_arguments.source_files = select(common_arguments.source_files, file_exists);
			common_arguments.packages = select(common_arguments.packages, directory_exists);
		}

		const bool get_no_prelude() const
		{
			return common_arguments.no_prelude;
		}

		const std::vector<std::string>& get_source_files() const 
		{
			return common_arguments.source_files;
		}

		const std::vector<std::string>& get_packages() const
		{
			return common_arguments.packages;
		}

		const bool get_debug() const
		{
			return common_arguments.debug;
		}

		const std::string get_verbosity() const
		{
			return common_arguments.verbosity;
		}

		const bool get_log_json() const
		{
			return common_arguments.log_json;
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
	protected:
		common_command_arguments common_arguments;
		element_interpreter_ctx* ictx;

	public:
		command(common_command_arguments common_arguments)
			: common_arguments{ std::move(common_arguments) }
		{
			element_interpreter_create(&ictx);
		}

		virtual ~command() {

			element_interpreter_delete(ictx);
		}

		//remove copy/move to be certain no implcit conversion is happening
		command(const command& other) = delete;
		command(command&& other) = delete;
		command& operator=(const command& other) = delete;
		command& operator=(command&& other) = delete;

	public:
		common_command_arguments get_common_arguments() const 
		{
			return common_arguments;
		}

		element_result parse(const compilation_input& input) const
		{
			return load_source_files(input);
		}

		virtual compiler_message execute(const compilation_input& input) const = 0;
		virtual std::string as_string() const = 0;

		using callback = std::function<void(const command&)>;
		static void configure(CLI::App& app, command::callback callback);

		compiler_message generate_response(element_result result, element_value value, const std::vector<trace_site>& trace_site = std::vector<libelement::cli::trace_site>()) const
		{
			switch (result)
			{
			case ELEMENT_OK:
				return compiler_message(message::SUCCESS, message_level::INFORMATION, std::to_string(value), trace_site);
			default:
				return compiler_message(message::UNKNOWN_ERROR, message_level::ERROR, std::to_string(value), trace_site);
			}
		}

	private:
		element_result load_source_files(const compilation_input& input) const
		{
			element_result result = ELEMENT_OK;
			for (auto& file : input.get_source_files())
			{
				std::string buffer;

				std::ifstream f(file);
				f.seekg(0, std::ios::end);
				buffer.resize(f.tellg());
				f.seekg(0);
				f.read(buffer.data(), buffer.size());

				result = element_interpreter_load_string(ictx, buffer.c_str(), file.c_str());
				element_interpreter_clear(ictx);

				//early return
				if (result != ELEMENT_OK)
					return result;
			}

			return result;
		}
	};
}