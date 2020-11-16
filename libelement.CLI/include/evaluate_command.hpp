#pragma once

#include <element/element.h>
#include <../../libelement/src/interpreter_internal.hpp>

#include "command.hpp"
#include "compiler_message.hpp"

namespace libelement::cli
{
    struct evaluate_command_arguments
    {
        std::string expression;
        std::string arguments;

        [[nodiscard]] std::string as_string() const
        {
            std::stringstream ss;
            ss << "evaluate --expression \"" << expression << "\"";
            if (!arguments.empty())
                ss << " --arguments \"" << arguments << "\"";
            return ss.str();
        }
    };

    class evaluate_command final : public command
    {
        evaluate_command_arguments custom_arguments;

    public:
        evaluate_command(common_command_arguments common_arguments,
                         evaluate_command_arguments custom_arguments)
            : command(std::move(common_arguments))
            , custom_arguments{ std::move(custom_arguments) }
        {}

        [[nodiscard]] compiler_message execute(const compilation_input& compilation_input) const override
        {
            const auto result = setup(compilation_input);
            if (result != ELEMENT_OK)
                return compiler_message(error_conversion(result),
                                        "Failed to setup context",
                                        compilation_input.get_log_json()); // todo

            if (compilation_input.get_compiletime())
                return execute_compiletime(compilation_input);

            return execute_runtime(compilation_input);
        }

        [[nodiscard]] compiler_message execute_compiletime(const compilation_input& compilation_input) const
        {
            const auto expression = custom_arguments.expression;

            element_compilation_ctx* compilation_context;
            auto result = element_create_compilation_ctx(context, &compilation_context);
            if (result != ELEMENT_OK)
            {
                return compiler_message(error_conversion(result),
                                        "Failed to create compilation context: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                        compilation_input.get_log_json());
            }

            element_object* expression_object;
            result = context->expression_to_object(nullptr, expression.c_str(), &expression_object);
            if (result != ELEMENT_OK)
            {
                context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
                return compiler_message(error_conversion(result),
                                        "Failed to convert expression to object: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                        compilation_input.get_log_json());
            }

            element_object* result_object;
            if (!custom_arguments.arguments.empty())
            {
                element_object* call_objects;
                int call_objects_count;
                result = context->call_expression_to_objects(nullptr, custom_arguments.arguments.c_str(), &call_objects, &call_objects_count);
                if (result != ELEMENT_OK)
                {
                    for (int i = 0; i < call_objects_count; ++i)
                    {
                        element_object* obj = &call_objects[i];
                        element_delete_object(&obj);
                    }
                    context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
                    return compiler_message(error_conversion(result),
                                            "Failed to convert call expression to objects: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                            compilation_input.get_log_json());
                }

                result = element_object_call(expression_object, compilation_context, call_objects, call_objects_count, &result_object);
                if (result != ELEMENT_OK)
                {
                    for (int i = 0; i < call_objects_count; ++i)
                    {
                        element_object* obj = &call_objects[i];
                        element_delete_object(&obj);
                    }
                    context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
                    return compiler_message(error_conversion(result),
                                            "Failed to call object with arguments: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                            compilation_input.get_log_json());
                }
            }
            else
            {
                result = element_object_compile(expression_object, compilation_context, &result_object);
                if (result != ELEMENT_OK)
                {
                    context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
                    return compiler_message(error_conversion(result),
                                            "Failed to compile object: " + expression + " at compile-time with element_result " + std::to_string(result),
                                            compilation_input.get_log_json());
                }
            }

            element_instruction* result_instruction;
            result = element_object_to_instruction(result_object, &result_instruction);
            if (result != ELEMENT_OK)
            {
                context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });
                return compiler_message(error_conversion(result),
                                        "Failed to convert object to instruction: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                        compilation_input.get_log_json());
            }

            constexpr auto max_output_size = 512;
            element_inputs input;
            input.values = nullptr;
            input.count = 0;

            element_outputs output;
            element_value outputs_buffer[max_output_size];
            output.values = outputs_buffer;
            output.count = max_output_size;
            result = element_interpreter_evaluate(context, nullptr, result_instruction, &input, &output);

            context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" });

            if (result != ELEMENT_OK)
            {
                return compiler_message(error_conversion(result),
                                        "Failed to evaluate: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                                        compilation_input.get_log_json());
            }

            return generate_response(result, output, compilation_input.get_log_json());
        }

        [[nodiscard]] compiler_message execute_runtime(const compilation_input& compilation_input) const
        {
            const auto expression = custom_arguments.expression;

            constexpr auto max_output_size = 512;

            element_instruction* compiled_function;
            element_result result = element_interpreter_compile_expression(context, nullptr, expression.c_str(), &compiled_function);
            if (result != ELEMENT_OK)
            {
                element_instruction_delete(&compiled_function);
                return compiler_message(error_conversion(result),
                                        "Failed to compile: " + expression + " at runtime with element_result " + std::to_string(result),
                                        compilation_input.get_log_json());
            }

            element_outputs call_output;
            std::array<element_value, max_output_size> call_outputs_buffer;
            call_output.values = call_outputs_buffer.data();
            call_output.count = max_output_size;

            element_inputs input;
            input.values = nullptr;
            input.count = 0;

            if (!custom_arguments.arguments.empty())
            {
                result = element_interpreter_evaluate_call_expression(context, nullptr, custom_arguments.arguments.c_str(), &call_output);
                if (result != ELEMENT_OK)
                {
                    element_instruction_delete(&compiled_function);
                    return compiler_message(error_conversion(result),
                                            "Failed to evaluate: " + expression + " called with " + custom_arguments.arguments + " at runtime with element_result " + std::to_string(result),
                                            compilation_input.get_log_json());
                }

                input.values = call_output.values;
                input.count = call_output.count;
            }

            element_value outputs_buffer[max_output_size];
            element_outputs output;
            output.values = outputs_buffer;
            output.count = max_output_size;

            element_interpreter_evaluate(context, nullptr, compiled_function, &input, &output);
            element_instruction_delete(&compiled_function);

            return generate_response(result, output, compilation_input.get_log_json());
        }

        [[nodiscard]] std::string as_string() const override
        {
            std::stringstream ss;
            ss << custom_arguments.as_string() << " " << common_arguments.as_string();
            return ss.str();
        }

        static void configure(CLI::App& app,
            const std::shared_ptr<common_command_arguments>& common_arguments,
            callback callback)
        {
            const auto arguments = std::make_shared<evaluate_command_arguments>();

            auto* command = app.add_subcommand("evaluate")->fallthrough();

            command->add_option("-e,--expression", arguments->expression,
                             "Expression to evaluate.")
                ->required();

            command->add_option("-a,--arguments", arguments->arguments,
                                "Arguments for performing call expression to given expression.");

            command->callback([callback, common_arguments, arguments]() {
                evaluate_command cmd(*common_arguments, *arguments);
                callback(cmd);
            });
        }
    };
} // namespace libelement::cli