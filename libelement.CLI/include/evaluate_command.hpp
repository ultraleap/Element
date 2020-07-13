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

		compiler_message execute(const compilation_input& compilation_input) const override
		{
			auto result = setup(compilation_input);
			if (result != ELEMENT_OK)
				return compiler_message(error_conversion(result), "Failed to setup context"); //todo
		
			const std::vector<trace_site> trace_site{};

			//Not handling error responses properly yet
			const auto evaluate = "evaluate = " + custom_arguments.expression + ";";
			result = element_interpreter_load_string(context, evaluate.c_str(), "<input>");
			if (result != ELEMENT_OK) {
				return compiler_message(error_conversion(result), "Failed to parse: " + evaluate + " with element_result " + std::to_string(result));
			}

			element_compilable* compilable;
			result = element_interpreter_find(context, "evaluate", &compilable);
			if (result != ELEMENT_OK) {
				return compiler_message(error_conversion(result), "Failed to find: " + evaluate + " with element_result " + std::to_string(result));
			}

			struct element_evaluable* evaluable;
			result = element_interpreter_compile(context, nullptr, compilable, &evaluable);
			if (result != ELEMENT_OK) {
				return compiler_message(error_conversion(result), "Failed to compile: " + evaluate + " with element_result " + std::to_string(result));
			}

			element_value inputs[] = { 1, 2 };
			element_inputs input;
			input.values = inputs;
			input.count = 2;

			element_value outputs[512];
			element_outputs output;
			output.values = outputs;
			output.count = 512;

			result = element_interpreter_evaluate(context, nullptr, evaluable, &input, &output);
			if (result != ELEMENT_OK) {
				return compiler_message(error_conversion(result), "Failed to evaluate: " + evaluate + " with element_result " + std::to_string(result));
			}

			return generate_response(result, output, trace_site);
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