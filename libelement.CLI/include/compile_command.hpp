#pragma once

#include <element/element.h>
#include <interpreter_internal.hpp>
#include <iostream>
#include <fstream>

#include "command.hpp"
#include "compiler_message.hpp"

#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#include "lmnt/interpreter.h"
#include "lmnt/compiler.hpp"
#include "lmnt/jit.h"

namespace libelement::cli
{
struct compile_command_arguments
{
    std::string expression;
    std::string arguments;
    std::string output_path;
    std::string name;

    [[nodiscard]] std::string as_string() const
    {
        std::stringstream ss;
        ss  << "command --expression \"" << expression
            << "\" --output-path \"" << output_path
            << "\" --name \"" << name << "\"";
        if (!arguments.empty())
            ss << " --arguments \"" << arguments << "\"";
        return ss.str();
    }
};

class compile_command final : public command
{
public:
    compile_command(common_command_arguments common_arguments,
        compile_command_arguments custom_arguments)
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

        return compile(compilation_input);
    }

    [[nodiscard]] compiler_message compile(const compilation_input& compilation_input) const
    {
        // TODO: construct expression using object model to reconstruct function's arguments

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

        if (common_arguments.target == Target::LMNT)
            response = compile_lmnt(compilation_input, compiled_function, input, output, custom_arguments.output_path);

        if (common_arguments.target == Target::LMNTJit)
            response = compile_lmnt(compilation_input, compiled_function, input, output, custom_arguments.output_path);

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
        const auto arguments = std::make_shared<compile_command_arguments>();

        auto* command = app.add_subcommand("compile")->fallthrough();

        command->add_option("-e,--expression", arguments->expression,
                    "Expression to evaluate.")
            ->required();
        command->add_option("-o,--output-path", arguments->output_path,
                    "Path to output generated file to.")
            ->required();
        command->add_option("-n,--name", arguments->name,
                    "Name of the resulting LMNT function.")
            ->required();

        command->add_option("-a,--arguments", arguments->arguments,
            "Arguments for performing call expression to given expression.");

        command->callback([callback, common_arguments, arguments]() {
            compile_command cmd(*common_arguments, *arguments);
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

    compiler_message compile_lmnt(
        const compilation_input& compilation_input,
        element_instruction* instruction,
        element_inputs input,
        element_outputs output,
        std::string output_path) const
    {
        element_lmnt_compiler_ctx lmnt_ctx;
        element_lmnt_compiled_function lmnt_output;
        std::vector<element_value> constants;

        auto result = element_lmnt_compile_function(lmnt_ctx, instruction->instruction, constants, input.count, lmnt_output);
        if (result != ELEMENT_OK) {
            printf("RUH ROH: %d\n", result);
            return generate_response(result, "failed to compile LMNT function", compilation_input.get_log_json());
        }

        for (size_t i = 0; i < constants.size(); ++i) {
            printf("Constant[%04zX]: %f\n", i, constants[i]);
        }
        for (const auto& in : lmnt_output.instructions) {
            printf("Instruction: %s %04X %04X %04X\n", lmnt_get_opcode_info(in.opcode)->name, in.arg1, in.arg2, in.arg3);
        }

        auto lmnt_archive_data = create_archive(
            custom_arguments.name.c_str(),
            uint16_t(input.count),
            uint16_t(output.count),
            uint16_t(lmnt_output.total_stack_count()),
            constants,
            lmnt_output.instructions,
            lmnt_output.flags);

        {
            std::ofstream ofs(output_path, std::ios::out | std::ios::binary);
            ofs.write(lmnt_archive_data.data(), lmnt_archive_data.size());
        }

        return generate_response(result, "", compilation_input.get_log_json());
    }

    compile_command_arguments custom_arguments;
};
} // namespace libelement::cli