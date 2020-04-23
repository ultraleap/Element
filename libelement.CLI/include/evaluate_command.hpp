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

		std::string as_string() const
		{
			std::stringstream ss;
			ss << "--expression " << expression << " ";
			return ss.str();
		}
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
			element_result result = ELEMENT_OK;
			result = setup(input);
			if (result != ELEMENT_OK)
				return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			//call into libelement
			const element_function* fn;
			element_compiled_function* cfn;
			element_value outputs[1];
			std::vector<trace_site> trace_site{};

			//Not handling error responses propertly yet
			auto evaluate = "evaluate = " + custom_arguments.expression + ";";
			result = element_interpreter_load_string(ictx, evaluate.c_str(), "<input>");
			if (result != ELEMENT_OK)
				return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			//std::cout << std::endl << std::endl;

			//result = element_interpreter_print_ast(ictx, "Prelude\\Num.ele");
			//if (result != ELEMENT_OK)
			//	return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			//std::cout << std::endl << std::endl;

			//result = element_interpreter_print_ast(ictx, "<input>");
			//if (result != ELEMENT_OK)
			//	return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			//std::cout << std::endl << std::endl;

			result = element_interpreter_get_function(ictx, "evaluate", &fn);
			if (result != ELEMENT_OK)
				return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			result = element_interpreter_compile_function(ictx, fn, &cfn, nullptr);
			if (result != ELEMENT_OK)
				return compiler_message(message::PARSE_ERROR, message_level::ERROR);

			result = element_interpreter_evaluate_function(ictx, cfn, nullptr, 1, outputs, 1, nullptr);
			if (result != ELEMENT_OK)
				return compiler_message(message::PARSE_ERROR, message_level::ERROR);

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
			const auto arguments = std::make_shared<evaluate_command_arguments>();

			const auto command = app.add_subcommand("evaluate")->fallthrough();
			command->add_option("-e,--expression", arguments->expression, "Expression to evaluate.")->required();

			command->callback([callback, common_arguments, arguments]() { callback(evaluate_command(*common_arguments, *arguments)); });
		}
	};
}