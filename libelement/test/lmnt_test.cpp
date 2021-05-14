#include "element/common.h"
#include "element/interpreter.h"
#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#include "lmnt/interpreter.h"

#include "interpreter_internal.hpp"
#include "lmnt/compiler.hpp"

#include <array>
#include <cstdio>
#include <iostream>
#include <vector>

void log_callback(const element_log_message* msg, void* user_data)
{
    //TODO: This is a bit of a hack for now... Setting a constant length instead and using for both buffers
    const auto space = 512;
    char buffer[space];
    buffer[0] = '^';
    buffer[1] = '\0';
    const char* buffer_str = nullptr;
    if (msg->character - 1 >= 0)
    {
        const auto padding_count = msg->character - 1;
        for (auto i = 0; i < padding_count; ++i)
        {
            buffer[i] = ' ';
        }

        const auto end = padding_count + msg->length;
        for (auto i = padding_count; i < end; ++i)
        {
            buffer[i] = '^';
        }

        buffer[end] = '\0';

        buffer_str = &buffer[0];
    }

    std::vector<char> output_buffer_array;
    output_buffer_array.resize((size_t)msg->message_length + 4 * space);
    auto* const output_buffer = output_buffer_array.data();

    sprintf(output_buffer, "\n----------ELE%d %s\n%d| %s\n%d| %s\n\n%s\n----------\n\n",
            msg->message_code,
            msg->filename,
            msg->line,
            msg->line_in_source ? msg->line_in_source : "",
            msg->line,
            buffer_str,
            msg->message);

    printf("%s", output_buffer);
}


static std::vector<char> create_archive(const char* def_name, uint16_t args_count, uint16_t rvals_count, uint16_t stack_count, const std::vector<lmnt_value>& constants, const std::vector<lmnt_instruction>& function)
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
        0x00, 0x00,                                    // defs[0].name
        0x00, 0x00,                                    // defs[0].flags
        0x00, 0x00, 0x00, 0x00,                        // defs[0].code
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_unaligned
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_aligned
        args_count & 0xFF, (args_count >> 8) & 0xFF,   // defs[0].args_count
        rvals_count & 0xFF, (rvals_count >> 8) & 0xFF, // defs[0].rvals_count
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



int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("give me something to run!\n");
        printf("Usage: %s <function-definition> [<input> ...]", argv[0]);
        return 1;
    }
    std::vector<element_value> args;
    for (size_t i = 2; i < argc; ++i)
        args.emplace_back(std::stof(argv[i]));

    element_interpreter_ctx* context = nullptr;
    element_evaluator_ctx* econtext = nullptr;
    element_declaration* declaration = nullptr;
    element_instruction* instruction = nullptr;

    ELEMENT_OK_OR_RETURN(element_interpreter_create(&context));
    element_interpreter_set_log_callback(context, log_callback, nullptr);
    element_interpreter_load_prelude(context);

    float outputs[1];

    element_inputs input;
    element_outputs output;

    std::array<char, 2048> output_buffer_array{};
    char* output_buffer = output_buffer_array.data();

    element_lmnt_compiler_ctx lmnt_ctx;
    element_lmnt_compiled_function lmnt_output;
    std::vector<element_value> constants;
    lmnt_result lresult = LMNT_OK;
    lmnt_validation_result lvresult = LMNT_VALIDATION_OK;
    const char* loperation = "nothing lol";

    element_result result = element_interpreter_load_string(context, argv[1], "<input>");
    if (result != ELEMENT_OK)
    {
        printf("Output buffer too small");
        goto cleanup;
    }

    result = element_interpreter_find(context, "evaluate", &declaration);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_interpreter_compile_declaration(context, NULL, declaration, &instruction);
    if (result != ELEMENT_OK)
        goto cleanup;

    result = element_evaluator_create(context, &econtext);
    if (result != ELEMENT_OK)
        goto cleanup;

    input.values = args.data();
    input.count = args.size();

    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate_instruction(context, econtext, instruction, &input, &output);
    if (result != ELEMENT_OK)
        goto cleanup;

    sprintf(output_buffer + strlen(output_buffer), "Element: %s -> {", argv[1]);
    for (int i = 0; i < output.count; ++i)
    {
        sprintf(output_buffer + strlen(output_buffer), "%f", output.values[i]);
        if (i != output.count - 1)
        {
            sprintf(output_buffer + strlen(output_buffer), ", ");
        }
    }
    sprintf(output_buffer + strlen(output_buffer), "}\n");

    printf("%s", output_buffer);

    result = element_lmnt_compile_function(lmnt_ctx, instruction->instruction, constants, args.size(), lmnt_output);
    if (result != ELEMENT_OK)
    {
        printf("RUH ROH: %d\n", result);
        goto cleanup;
    }

    // for (const auto& in : lmnt_output.instructions)
    // {
    //     printf("Instruction: %s %04X %04X %04X\n", lmnt_get_opcode_info(in.opcode)->name, in.arg1, in.arg2, in.arg3);
    // }

    printf("Inputs: %zu, outputs: %zu, locals: %zu\n", lmnt_output.inputs_count, lmnt_output.outputs_count, lmnt_output.local_stack_count);

    {
        auto lmnt_archive_data = create_archive("evaluate", uint16_t(args.size()), uint16_t(output.count), uint16_t(lmnt_output.total_stack_count()), constants, lmnt_output.instructions);

        std::vector<char> lmnt_stack(32768);
        lmnt_ictx lctx;
        lmnt_result lresult;

        loperation = "init";
        printf("lmnt: doing %s\n", loperation);
        lresult = lmnt_init(&lctx, lmnt_stack.data(), lmnt_stack.size());
        if (lresult != LMNT_OK)
            goto lmnt_error;
            
        loperation = "archive load";
        printf("lmnt: doing %s\n", loperation);
        lresult = lmnt_load_archive(&lctx, lmnt_archive_data.data(), lmnt_archive_data.size());
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "archive prepare";
        printf("lmnt: doing %s\n", loperation);
        lresult = lmnt_prepare_archive(&lctx, &lvresult);
        if (lresult != LMNT_OK)
        {
            printf("LMNT validation error: %d\n", lvresult);
            goto lmnt_error;
        }

        printf("\n");
        lmnt_archive_print(&lctx.archive);

        loperation = "def search";
        printf("lmnt: doing %s\n", loperation);
        const lmnt_def* def = nullptr;
        lresult = lmnt_find_def(&lctx, "evaluate", &def);
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "setting args";
        printf("lmnt: %s\n", loperation);
        lresult = lmnt_update_args(&lctx, def, 0, args.data(), lmnt_offset(args.size()));
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "executing def";
        printf("lmnt: %s\n", loperation);
        std::vector<lmnt_value> lmnt_results(output.count);
        lresult = lmnt_execute(&lctx, def, lmnt_results.data(), lmnt_offset(lmnt_results.size()));
        if (lresult != lmnt_results.size())
            goto lmnt_error;

        for (size_t i = 0; i < lmnt_results.size(); ++i)
            printf("lmnt_results[%zu]: %f\n", i, lmnt_results[i]);
    }

lmnt_error:
    if (lresult != LMNT_OK)
    {
        printf("LMNT ERROR during %s: %d\n", loperation, lresult);
    }

cleanup:
    element_declaration_delete(&declaration);
    element_instruction_delete(&instruction);
    element_interpreter_delete(&context);
    if (result != ELEMENT_OK)
    {
        printf("ELEMENT ERROR: %d\n", result);
    }
    return result;
}