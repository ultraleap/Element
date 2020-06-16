#pragma once

#include <filesystem>
#include <string>
#include <utility>
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
		bool no_parse_trace = false;
		message_level verbosity = message_level::Information;
		std::vector<std::string> packages{};
		std::vector<std::string> source_files{};

		[[nodiscard]] std::string as_string() const
		{
			std::stringstream ss;
			
			if (no_prelude)
				ss << "--no-prelude ";

			if (debug)
				ss << "--debug ";

			if (log_json)
				ss << "--logjson ";

			if (no_parse_trace)
				ss << "--no-parse-trace ";

			if (verbosity != message_level::Unknown)
				ss << "--verbosity " << static_cast<int>(verbosity) << " ";

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
        explicit compilation_input(common_command_arguments arguments)
			: common_arguments{ std::move(arguments) }
		{
			common_arguments.source_files = select(common_arguments.source_files, file_exists);
			common_arguments.packages = select(common_arguments.packages, directory_exists);
		}

        [[nodiscard]] bool get_no_prelude() const
		{
			return common_arguments.no_prelude;
		}

		[[nodiscard]] const std::vector<std::string>& get_source_files() const
		{
			return common_arguments.source_files;
		}

		[[nodiscard]] const std::vector<std::string>& get_packages() const
		{
			return common_arguments.packages;
		}

		[[nodiscard]] bool get_debug() const
		{
			return common_arguments.debug;
		}

		[[nodiscard]] message_level get_verbosity() const
		{
			return common_arguments.verbosity;
		}

		[[nodiscard]] bool get_log_json() const
		{
			return common_arguments.log_json;
		}

	private:
		[[nodiscard]] static bool file_exists(const std::string& file)
		{
			return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
		}

		[[nodiscard]] static bool directory_exists(const std::string& directory)
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
		element_interpreter_ctx* context;

	public:
        explicit command(common_command_arguments common_arguments)
			: common_arguments{ std::move(common_arguments) }
		{
			element_interpreter_create(&context);
		}

		virtual ~command() {

			element_interpreter_delete(context);
		}

		//remove copy/move to be certain no implcit conversion is happening
		command(const command& other) = delete;
		command(command&& other) = delete;
		command& operator=(const command& other) = delete;
		command& operator=(command&& other) = delete;

		[[nodiscard]] common_command_arguments get_common_arguments() const
		{
			return common_arguments;
		}

		[[nodiscard]] virtual compiler_message execute(const compilation_input& input) const = 0;
		[[nodiscard]] virtual std::string as_string() const = 0;

		using callback = std::function<void(const command&)>;
		using log_callback = void (*)(const element_log_message* const);
		static void configure(CLI::App& app, command::callback callback);

		static compiler_message generate_response(const element_result result, element_outputs output, std::vector<trace_site> trace_stack = std::vector<libelement::cli::trace_site>())
		{
			std::string data;
			for (auto i = 0; i < output.count; i++) 
			{
				if (std::isnan(output.values[i]))
					data += "NaN";
				else if (output.values[i] == std::numeric_limits<float>::infinity())
					data += "Infinity";
				else if (output.values[i] == -std::numeric_limits<float>::infinity())
					data += "-Infinity";
				else
					data += std::to_string(output.values[i]);

				if (i >= output.count - 1)
					data += " ";
			}
			
			return generate_response(result, data, std::move(trace_stack));
		}

		static compiler_message generate_response(const element_result result, const std::string& value, const std::vector<trace_site> trace_stack = std::vector<libelement::cli::trace_site>())
		{
			switch (result)
			{
			case ELEMENT_OK:
				return compiler_message(value, trace_stack);
			default:
				return compiler_message(ELEMENT_ERROR_UNKNOWN, value, trace_stack);
			}
		}

		void set_log_callback(const command::log_callback log_callback) const {

			element_interpreter_set_log_callback(context, log_callback);
		}

	protected:
		[[nodiscard]] element_result setup(const compilation_input& input) const
		{
			element_result result = ELEMENT_OK;
			if (!input.get_no_prelude()) {
				result = element_interpreter_load_prelude(context);
				if (result != ELEMENT_OK) 
				{
					//TODO: Better solution for this? Forces a parse error on any file load error
					auto parse_error = compiler_message(ELEMENT_ERROR_PARSE, "Parsing failed");
					std::cout << parse_error.serialize() << std::endl;
					
					return result;
				}
			}

			auto packages = convert(input.get_packages());
			auto packages_count = static_cast<int>(packages.size());
			if (packages_count > 0) {
				result = element_interpreter_load_packages(context, &packages[0], packages_count);
				if (result != ELEMENT_OK)
				{
					//TODO: Better solution for this? Forces a parse error on any file load error
					auto parse_error = compiler_message(ELEMENT_ERROR_PARSE, "Parsing failed");
					std::cout << parse_error.serialize() << std::endl;

					return result;
				}
			}

			auto source_files = convert(input.get_source_files());
			auto source_file_count = static_cast<int>(source_files.size());
			if (source_file_count > 0) {
				
				result = element_interpreter_load_files(context, &source_files[0], source_file_count);
				if (result != ELEMENT_OK) 
				{
					//TODO: Better solution for this? Forces a parse error on any file load error
					auto parse_error = compiler_message(ELEMENT_ERROR_PARSE, "Parsing failed");
					std::cout << parse_error.serialize() << std::endl;
					
					return result;
				}
			}

			return ELEMENT_OK;
		}

	protected:
		static std::vector<const char*> convert(const std::vector<std::string>& input)
		{
			std::vector<const char*> output;
			output.reserve(input.size());

			for (size_t i = 0; i < input.size(); ++i)
				output.push_back(input[i].c_str());

			return output;
		}
	};
}