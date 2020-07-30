#pragma once

#include <string>

#include "command.hpp"

namespace libelement::cli
{
	struct typeof_command_arguments 
	{
		std::string expression;

		[[nodiscard]] std::string as_string() const
		{
			std::stringstream ss;
			ss << "typeof --expression " << expression << " ";
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

		[[nodiscard]] compiler_message execute(const compilation_input& compilation_input) const override
		{
			element_result result = ELEMENT_OK;
			result = setup(compilation_input);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_PARSE,"Failed to setup context");

			//call into libelement
			const std::vector<trace_site> trace_site{};

			std::string typeof(256, '\0');

			const auto expression = custom_arguments.expression;

			result = element_interpreter_typeof_expression(context, nullptr, expression.c_str(), typeof.data(), typeof.size());
			if (result != ELEMENT_OK) {
				auto type = ELEMENT_ERROR_UNKNOWN;
				if (result > 0)
					type = static_cast<message_type>(result);
				return compiler_message(type, "Failed to compile: " + expression + " with element_result " + std::to_string(result));
			}

			return generate_response(result, typeof);

#ifdef LEGACY_COMPILER
			result = element_interpreter_get_internal_typeof(context, typeof.c_str(), "<input>", internal_typeof_string.data(), 256);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_UNKNOWN, "Failed to get internal type of '" + typeof + "'");

			return generate_response(result, internal_typeof_string, trace_site);
#endif
		}

		[[nodiscard]] std::string as_string() const override
		{
			std::stringstream ss;
			ss << custom_arguments.as_string() << " " << common_arguments.as_string();
			return ss.str();
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<typeof_command_arguments>();

			const auto command = app.add_subcommand("typeof")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to get the type of.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(typeof_command(*common_arguments, *arguments)); });
		}
	};
}