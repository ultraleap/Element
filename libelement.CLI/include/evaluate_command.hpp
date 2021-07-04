#pragma once

#include <element/element.h>
#include <interpreter_internal.hpp>

#include "command.hpp"
#include "compiler_message.hpp"

#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#include "lmnt/interpreter.h"
#include "lmnt/compiler.hpp"
#include "lmnt/jit.h"

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

        element_object_model_ctx* compilation_context;
        auto result = element_object_model_ctx_create(context, &compilation_context);
        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                "Failed to create compilation context: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        element_object* expression_object;
        result = context->expression_to_object(nullptr, expression.c_str(), &expression_object);
        if (result != ELEMENT_OK) {
            context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);
            return compiler_message(error_conversion(result),
                "Failed to convert expression to object: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                compilation_input.get_log_json());
        }

        element_object* result_object;
        if (!custom_arguments.arguments.empty()) {
            std::vector<element::object_const_shared_ptr> objs;
            result = context->call_expression_to_objects(nullptr, custom_arguments.arguments.c_str(), objs);

            if (result != ELEMENT_OK) {
                context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);
                return compiler_message(error_conversion(result),
                    "Failed to convert call expression to objects: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                    compilation_input.get_log_json());
            }

            const int call_objects_count = static_cast<int>(objs.size());
            element_object** call_objects = new element_object*[call_objects_count];

            for (int i = 0; i < call_objects_count; ++i)
                call_objects[i] = new element_object{ std::move(objs[i]) };

            result = element_object_call(expression_object, compilation_context, call_objects, call_objects_count, &result_object);

            for (int i = 0; i < call_objects_count; ++i)
                element_object_delete(&call_objects[i]);

            delete[] call_objects;

            if (result != ELEMENT_OK) {
                context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);
                return compiler_message(error_conversion(result),
                    "Failed to call object with arguments: " + expression + " called with " + custom_arguments.arguments + " at compile-time with element_result " + std::to_string(result),
                    compilation_input.get_log_json());
            }
        } else {
            result = element_object_simplify(expression_object, compilation_context, &result_object);
            if (result != ELEMENT_OK) {
                context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);
                return compiler_message(error_conversion(result),
                    "Failed to compile object: " + expression + " at compile-time with element_result " + std::to_string(result),
                    compilation_input.get_log_json());
            }
        }

        element_instruction* result_instruction;
        result = element_object_to_instruction(result_object, &result_instruction);
        if (result != ELEMENT_OK) {
            context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);
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

        element_evaluator_ctx* evaluator;
        element_evaluator_create(context, &evaluator);
        result = element_interpreter_evaluate_instruction(context, evaluator, result_instruction, &input, &output);
        element_evaluator_delete(&evaluator);

        std::string interpreter_result_string = fmt::format("Element: {} -> {{", custom_arguments.expression + custom_arguments.arguments);
        for (int i = 0; i < output.count - 1; ++i) {
            interpreter_result_string += fmt::format("{}, ", output.values[i]);
        }
        interpreter_result_string += fmt::format("{}}}\n", output.values[output.count - 1]);
        std::cout << interpreter_result_string;

        if (result != ELEMENT_OK) {
            return compiler_message(error_conversion(result),
                fmt::format("Failed to evaluate: '{}' called with '{}' at compile-time with element_result '{}'",
                    expression, custom_arguments.arguments, result),
                compilation_input.get_log_json());
        }

        auto response = generate_response(ELEMENT_ERROR_UNKNOWN, element_outputs{}, compilation_input.get_log_json());

        if (common_arguments.target == Target::Interpreter)
            response = generate_response(result, output, compilation_input.get_log_json());

        if (common_arguments.target == Target::LMNT)
            response = execute_lmnt(compilation_input, result_instruction, input, output, false);

        if (common_arguments.target == Target::LMNTJit)
            response = execute_lmnt(compilation_input, result_instruction, input, output, true);

        context->global_scope->remove_declaration(element::identifier{ "<REMOVE>" }, context->cache_scope_find);

        return response;
    }

    [[nodiscard]] compiler_message execute_runtime(const compilation_input& compilation_input) const
    {
        const auto expression = custom_arguments.expression;

        constexpr auto max_output_size = 512;

        element_instruction* compiled_function;
        element_result result = element_interpreter_compile_expression(context, nullptr, expression.c_str(), &compiled_function);
        if (result != ELEMENT_OK) {
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

        if (!custom_arguments.arguments.empty()) {
            element_evaluator_ctx* evaluator;
            element_evaluator_create(context, &evaluator);
            result = element_interpreter_evaluate_call_expression(context, evaluator, custom_arguments.arguments.c_str(), &call_output);
            element_evaluator_delete(&evaluator);

            if (result != ELEMENT_OK) {
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

        element_evaluator_ctx* evaluator;
        element_evaluator_create(context, &evaluator);
        element_interpreter_evaluate_instruction(context, evaluator, compiled_function, &input, &output);
        element_evaluator_delete(&evaluator);

        std::string interpreter_result_string = fmt::format("Element: {} -> {{", custom_arguments.expression + custom_arguments.arguments);
        for (int i = 0; i < output.count - 1; ++i) {
            interpreter_result_string += fmt::format("{}, ", output.values[i]);
        }
        interpreter_result_string += fmt::format("{}}}\n", output.values[output.count - 1]);
        std::cout << interpreter_result_string;

        auto response = generate_response(ELEMENT_ERROR_UNKNOWN, element_outputs{}, compilation_input.get_log_json());

        if (common_arguments.target == Target::Interpreter)
            response = generate_response(result, output, compilation_input.get_log_json());

        if (common_arguments.target == Target::LMNT)
            response = execute_lmnt(compilation_input, compiled_function, input, output, false);

        if (common_arguments.target == Target::LMNTJit)
            response = execute_lmnt(compilation_input, compiled_function, input, output, true);

        element_instruction_delete(&compiled_function);

        return response;
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

private:
    static std::vector<char> create_archive(
        const char* def_name,
        uint16_t args_count,
        uint16_t rvals_count,
        uint16_t stack_count,
        const std::vector<lmnt_value>& constants,
        const std::vector<lmnt_instruction>& function,
        const lmnt_def_flags flags)
    {
        const size_t name_len = strlen(def_name);
        const size_t name_len_padded = LMNT_ROUND_UP(0x02 + name_len + 1, 4) - 2;
        const size_t instr_count = function.size();
        const size_t consts_count = constants.size();
        const size_t data_count = 0;
        assert(name_len_padded <= 0xFD);
        assert(instr_count <= 0x3FFFFFF0);
        assert(consts_count <= 0x3FFFFFFF);

        const size_t header_len = 0x1C;
        const size_t strings_len = 0x02 + name_len_padded;
        const size_t defs_len = 0x10;
        const size_t code_len = 0x04 + instr_count * sizeof(lmnt_instruction);
        const lmnt_loffset data_sec_count = 0;
        const size_t data_len = 0x04 + data_sec_count * (0x08 + 0x04 * data_count);
        const size_t consts_len = consts_count * sizeof(lmnt_value);

        const size_t total_size = header_len + strings_len + defs_len + code_len + data_len + consts_len;
        std::vector<char> buf;
        buf.resize(total_size);

        size_t idx = 0;
        const char header[] = {
            'L', 'M', 'N', 'T',
            0x00, 0x00, 0x00, 0x00,
            char(strings_len & 0xFF), char((strings_len >> 8) & 0xFF), char((strings_len >> 16) & 0xFF), char((strings_len >> 24) & 0xFF), // strings length
            char(defs_len & 0xFF), char((defs_len >> 8) & 0xFF), char((defs_len >> 16) & 0xFF), char((defs_len >> 24) & 0xFF),             // defs length
            char(code_len & 0xFF), char((code_len >> 8) & 0xFF), char((code_len >> 16) & 0xFF), char((code_len >> 24) & 0xFF),             // code length
            char(data_len & 0xFF), char((data_len >> 8) & 0xFF), char((data_len >> 16) & 0xFF), char((data_len >> 24) & 0xFF),             // data length
            char(consts_len & 0xFF), char((consts_len >> 8) & 0xFF), char((consts_len >> 16) & 0xFF), char((consts_len >> 24) & 0xFF)      // constants_length
        };
        memcpy(buf.data() + idx, header, sizeof(header));
        idx += sizeof(header);

        buf[idx] = name_len_padded & 0xFF;
        idx += 2;

        memcpy(buf.data() + idx, def_name, name_len);
        idx += name_len;
        for (size_t i = name_len; i < name_len_padded; ++i)
            buf[idx++] = '\0';

        const char def[] = {
            0x00, 0x00,                                                                          // defs[0].name
            static_cast<char>(flags & 0xFF), static_cast<char>((flags >> 8) & 0xFF),             // defs[0].flags
            0x00, 0x00, 0x00, 0x00,                                                              // defs[0].code
            static_cast<char>(stack_count & 0xFF), static_cast<char>((stack_count >> 8) & 0xFF), // defs[0].stack_count_unaligned
            static_cast<char>(args_count & 0xFF), static_cast<char>((args_count >> 8) & 0xFF),   // defs[0].args_count
            static_cast<char>(rvals_count & 0xFF), static_cast<char>((rvals_count >> 8) & 0xFF), // defs[0].rvals_count
            0x00, 0x00,                                                                          // defs[0].default_args_index
        };
        memcpy(buf.data() + idx, def, sizeof(def));
        idx += sizeof(def);

        memcpy(buf.data() + idx, (const char*)(&instr_count), sizeof(uint32_t));
        idx += sizeof(uint32_t);

        memcpy(buf.data() + idx, function.data(), instr_count * sizeof(lmnt_instruction));
        idx += instr_count * sizeof(lmnt_instruction);

        memcpy(buf.data() + idx, (const char*)(&data_sec_count), sizeof(lmnt_loffset));
        idx += sizeof(lmnt_loffset);

        memcpy(buf.data() + idx, constants.data(), consts_count * sizeof(lmnt_value));
        idx += consts_count * sizeof(lmnt_value);

        assert(idx == total_size);

        return buf;
    }

    compiler_message execute_lmnt(
        const compilation_input& compilation_input,
        element_instruction* instruction,
        element_inputs input,
        element_outputs output,
        bool jit) const
    {
        std::vector<lmnt_value> lmnt_results;

        element_lmnt_compiler_ctx lmnt_ctx;
        element_lmnt_compiled_function lmnt_output;
        std::vector<element_value> constants;
        lmnt_result lresult = LMNT_OK;
        lmnt_validation_result lvresult = LMNT_VALIDATION_OK;
        const char* loperation = "nothing ecksdee";

        auto result = element_lmnt_compile_function(lmnt_ctx, instruction->instruction, constants, input.count, lmnt_output);
        if (result != ELEMENT_OK) {
            printf("RUH ROH: %d\n", result);
            goto cleanup;
        }

        for (size_t i = 0; i < constants.size(); ++i) {
            printf("Constant[%04zX]: %f\n", i, constants[i]);
        }
        for (const auto& in : lmnt_output.instructions) {
            printf("Instruction: %s %04X %04X %04X\n", lmnt_get_opcode_info(in.opcode)->name, in.arg1, in.arg2, in.arg3);
        }

        {
            auto lmnt_archive_data = create_archive("evaluate", uint16_t(input.count), uint16_t(output.count), uint16_t(lmnt_output.total_stack_count()), constants, lmnt_output.instructions, lmnt_output.flags);

            std::vector<char> lmnt_stack(32768);
            lmnt_ictx lctx;
            lmnt_result lresult = LMNT_OK;

            loperation = "init";
            lresult = lmnt_init(&lctx, lmnt_stack.data(), lmnt_stack.size());
            if (lresult != LMNT_OK)
                goto lmnt_error;

            loperation = "archive load";
            lresult = lmnt_load_archive(&lctx, lmnt_archive_data.data(), lmnt_archive_data.size());
            if (lresult != LMNT_OK)
                goto lmnt_error;

            loperation = "archive prepare";
            lresult = lmnt_prepare_archive(&lctx, &lvresult);
            if (lresult != LMNT_OK) {
                printf("LMNT validation error: %d\n", lvresult);
                goto lmnt_error;
            }

            loperation = "def search";
            const lmnt_def* def = nullptr;
            lresult = lmnt_find_def(&lctx, "evaluate", &def);
            if (lresult != LMNT_OK)
                goto lmnt_error;

            loperation = "setting args";
            lresult = lmnt_update_args(&lctx, def, 0, input.values, lmnt_offset(input.count));
            if (lresult != LMNT_OK)
                goto lmnt_error;

            loperation = "executing def";
            lmnt_results.resize(output.count);
            lresult = lmnt_execute(&lctx, def, lmnt_results.data(), lmnt_offset(lmnt_results.size()));
            if (lresult != lmnt_results.size())
                goto lmnt_error;

            for (size_t i = 0; i < lmnt_results.size(); ++i)
                printf("lmnt_results[%zu]: %f\n", i, lmnt_results[i]);

            if (jit) {
                lmnt_results.resize(output.count);

                loperation = "jit compile";
                lmnt_jit_fn_data fndata;
                lresult = lmnt_jit_compile(&lctx, def, LMNT_JIT_TARGET_CURRENT, &fndata);
                if (lresult != LMNT_OK)
                    goto lmnt_error;

                loperation = "jit execute";
                lmnt_results.resize(output.count);
                lresult = lmnt_jit_execute(&lctx, &fndata, lmnt_results.data(), lmnt_offset(lmnt_results.size()));
                if (lresult != lmnt_results.size())
                    goto lmnt_error;

                for (size_t i = 0; i < lmnt_results.size(); ++i)
                    printf("lmnt_jit_results[%zu]: %f\n", i, lmnt_results[i]);
            }
        }

    lmnt_error:
        if (lresult != LMNT_OK) {
            printf("LMNT ERROR during %s: %d\n", loperation, lresult);
            result = ELEMENT_ERROR_UNKNOWN;
        }

    cleanup:
        element_outputs lmnt_outputs;
        lmnt_outputs.count = lmnt_results.size();
        lmnt_outputs.values = lmnt_results.data();

        return generate_response(result, lmnt_outputs, compilation_input.get_log_json());
    }

    evaluate_command_arguments custom_arguments;
};
} // namespace libelement::cli