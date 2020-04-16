#pragma once

#include "element/token.h"
#include "element/ast.h"
#include "element/interpreter.h"

#include "command.hpp"
#include "compiler_message.hpp"

namespace libelement::cli
{
	struct evaluate_command_arguments 
	{
		std::string expression;
	};

	class evaluate_command final : public command
	{
		evaluate_command_arguments custom_arguments;

	public:
		evaluate_command(common_command_arguments common_arguments, evaluate_command_arguments custom_arguments)
			: command(common_arguments), custom_arguments{ std::move(custom_arguments) }
		{
		}

		compiler_message execute(const compilation_input& input) const override
		{
			std::vector<trace_site> what_to_put_in_here{};

			////call into libelement
			//const element_function* fn;
			//element_compiled_function* cfn;
			//element_value outputs[1];

			////what do I do here as we don't know the function by name
			//element_interpreter_get_function(ictx, "Tomato", &fn);
			//element_interpreter_compile_function(ictx, fn, &cfn, nullptr);
			//auto result = element_interpreter_evaluate_function(ictx, cfn, nullptr, 1, outputs, 1, nullptr);

			element_result result = 0;
			element_value outputs[1];

			return generate_response(result, outputs[0], what_to_put_in_here);
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