#pragma once

#include <string>

#include "command.hpp"

namespace libelement::cli
{
	struct typeof_command_arguments 
	{
		std::string expression;

		std::string as_string() const
		{
			std::stringstream ss;
			ss << "evaluate --expression " << expression << " ";
			return ss.str();
		}
	};

	class typeof_command final : public command
	{
		typeof_command_arguments custom_arguments;

	public:
		typeof_command(common_command_arguments common_arguments, typeof_command_arguments custom_arguments)
			: command(std::move(common_arguments)), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			element_result result = ELEMENT_OK;
			result = setup(input);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_PARSE,"Failed to setup context");

			//call into libelement
			element_value outputs[1]{ 0 };
			const std::vector<trace_site> trace_site;

			return generate_response(result, outputs[0], trace_site);
		}

		std::string as_string() const override
		{
			std::stringstream ss;
			ss << custom_arguments.as_string() << " " << common_arguments.as_string();
			return ss.str();
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<typeof_command_arguments>();

			const auto command = app.add_subcommand("typeof")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(typeof_command(*common_arguments, *arguments)); });
		}
	};
}