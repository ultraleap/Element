#pragma once

#include <element/common.h>

#include "element/interpreter.h"

#include "command.hpp"
#include "compiler_message.hpp"

namespace libelement::cli
{
	struct evaluate_command_arguments 
	{
		std::string expression;

		std::string as_string() const
		{
			std::stringstream ss;
			ss << "evaluate --expression " << expression << " ";
			return ss.str();
		}
	};

	class evaluate_command final : public command
	{
		evaluate_command_arguments custom_arguments;

	public:
		evaluate_command(common_command_arguments common_arguments, evaluate_command_arguments custom_arguments)
			: command(std::move(common_arguments)), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			auto result = setup(input);
			if (result != ELEMENT_OK)
				return compiler_message(ELEMENT_ERROR_PARSE, "Failed to setup context");

			//call into libelement
			const element_function* fn;
			element_compiled_function* cfn;
		
			const std::vector<trace_site> trace_site{};

			//Not handling error responses properly yet
			const auto evaluate = "evaluate = " + custom_arguments.expression + ";";
			result = element_interpreter_load_string(ictx, evaluate.c_str(), "<input>");
			if (result != ELEMENT_OK) {
				message_type type = ELEMENT_ERROR_UNKNOWN;
				if (result > 0)
					type = static_cast<message_type>(result);
				return compiler_message(type, "Failed to parse: " + evaluate + " with element_result " + std::to_string(result));
			}

			result = element_interpreter_get_function(ictx, "evaluate", &fn);
			if (result != ELEMENT_OK) {
				message_type type = ELEMENT_ERROR_UNKNOWN;
				if (result > 0)
					type = static_cast<message_type>(result);
				return compiler_message(type, "Failed to find: " + evaluate + " with element_result " + std::to_string(result));
			}

			result = element_interpreter_compile_function(ictx, fn, &cfn, nullptr);
			if (result != ELEMENT_OK) {
				message_type type = ELEMENT_ERROR_UNKNOWN;
				if (result > 0)
					type = static_cast<message_type>(result);
				return compiler_message(type, "Failed to compile: " + evaluate + " with element_result " + std::to_string(result));
			}

			//HACK: just to get multiple values serializing, do something better here!
			const auto size = get_outputs_size(ictx, cfn);
			element_value outputs[256];
			result = element_interpreter_evaluate_function(ictx, cfn, nullptr, 1, outputs, size, nullptr);
			if (result != ELEMENT_OK) {
				message_type type = ELEMENT_ERROR_UNKNOWN;
				if (result > 0)
					type = static_cast<message_type>(result);
				return compiler_message(type, "Failed to evaluate: " + evaluate + " with element_result " + std::to_string(result));
			}

			return generate_response(result, outputs, size, trace_site);
		}

		std::string as_string() const override
		{
			std::stringstream ss;
			ss << custom_arguments.as_string() << " " << common_arguments.as_string();
			return ss.str();
		}

		static void configure(CLI::App& app, const std::shared_ptr<common_command_arguments>& common_arguments, command::callback callback)
		{
			const auto arguments = std::make_shared<evaluate_command_arguments>();

			const auto command = app.add_subcommand("evaluate")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(evaluate_command(*common_arguments, *arguments)); });
		}
	};
}