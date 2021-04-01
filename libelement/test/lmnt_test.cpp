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


static std::vector<char> create_archive(const char* def_name, uint16_t args_count, uint16_t rvals_count, uint16_t stack_count, std::vector<lmnt_value> constants, std::vector<lmnt_instruction> function)
{
    const size_t name_len = strlen(def_name);
    const size_t instr_count = function.size();
    const size_t consts_count = constants.size();
    const size_t data_count = 0;
    assert(name_len <= 0xFE);
    assert(instr_count <= 0x3FFFFFF0);
    assert(consts_count <= 0x3FFFFFFF);

    const size_t header_len = 0x1C;
    const size_t strings_len = 0x02 + name_len + 1;
    const size_t defs_len = 0x15;
    uint32_t code_len = 0x04 + instr_count * sizeof(lmnt_instruction);
    const uint32_t code_padding = (4 - ((header_len + strings_len + defs_len + code_len) % 4)) % 4;
    code_len += code_padding;
    const lmnt_loffset data_sec_count = 0;
    uint32_t data_len = 0x04 + data_sec_count * (0x08 + 0x04 * data_count);
    const uint32_t consts_len = consts_count * sizeof(lmnt_value);

    const size_t total_size = header_len + strings_len + defs_len + code_len + data_len + consts_len;
    std::vector<char> buf;
    buf.resize(total_size);

    size_t idx = 0;
    const char header[] = {
        'L', 'M', 'N', 'T',
        0x00, 0x00, 0x00, 0x00,
        strings_len & 0xFF, (strings_len >> 8) & 0xFF, (strings_len >> 16) & 0xFF, (strings_len >> 24) & 0xFF, // strings length
        defs_len & 0xFF, (defs_len >> 8) & 0xFF, (defs_len >> 16) & 0xFF, (defs_len >> 24) & 0xFF,             // defs length
        code_len & 0xFF, (code_len >> 8) & 0xFF, (code_len >> 16) & 0xFF, (code_len >> 24) & 0xFF,             // code length
        data_len & 0xFF, (data_len >> 8) & 0xFF, (data_len >> 16) & 0xFF, (data_len >> 24) & 0xFF,             // data length
        consts_len & 0xFF, (consts_len >> 8) & 0xFF, (consts_len >> 16) & 0xFF, (consts_len >> 24) & 0xFF      // constants_length
    };
    memcpy(buf.data() + idx, header, sizeof(header));
    idx += sizeof(header);

    buf[idx] = (name_len + 1) & 0xFF;
    idx += 2;

    memcpy(buf.data() + idx, def_name, name_len);
    idx += name_len;
    buf[idx++] = '\0';

    const char def[] = {
        0x15, 0x00,                                    // defs[0].length
        0x00, 0x00,                                    // defs[0].name
        0x00, 0x00,                                    // defs[0].flags
        0x00, 0x00, 0x00, 0x00,                        // defs[0].code
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_unaligned
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_aligned
        0x00, 0x00,                                    // defs[0].base_args_count
        args_count & 0xFF, (args_count >> 8) & 0xFF,   // defs[0].args_count
        rvals_count & 0xFF, (rvals_count >> 8) & 0xFF, // defs[0].rvals_count
        0x00                                           // defs[0].bases_count
    };
    memcpy(buf.data() + idx, def, sizeof(def));
    idx += sizeof(def);

    memcpy(buf.data() + idx, (const char*)(&instr_count), sizeof(uint32_t));
    idx += sizeof(uint32_t);

    memcpy(buf.data() + idx, function.data(), instr_count * sizeof(lmnt_instruction));
    idx += instr_count * sizeof(lmnt_instruction);

    idx += code_padding;
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
        printf("Usage: %s <function-definition> [<input> ...]");
        return 1;
    }
    std::vector<element_value> args;
    for (size_t i = 2; i < argc; ++i)
        args.emplace_back(std::stof(argv[i]));

    element_interpreter_ctx* context = nullptr;
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
    std::vector<lmnt_instruction> lmnt_output;
    std::vector<element_value> constants;
    lmnt_result lresult = LMNT_OK;
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

    input.values = args.data();
    input.count = args.size();

    output.values = outputs;
    output.count = 1;

    result = element_interpreter_evaluate_instruction(context, NULL, instruction, &input, &output);
    if (result != ELEMENT_OK)
        goto cleanup;

    sprintf(output_buffer + strlen(output_buffer), "%s -> {", argv[1]);
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

    for (const auto& in : lmnt_output)
    {
        printf("Instruction: %s %04X %04X %04X\n", lmnt_get_opcode_info(in.opcode)->name, in.arg1, in.arg2, in.arg3);
    }

    {
        uint16_t stack_count = 8; // TODO: be able to get this out
        auto lmnt_archive_data = create_archive("evaluate", args.size(), output.count, stack_count, constants, lmnt_output);

        std::vector<char> lmnt_stack(32768);
        lmnt_ictx lctx;
        lmnt_result lresult;

        loperation = "ictx init";
        printf("doing %s\n", loperation);
        lresult = lmnt_ictx_init(&lctx, lmnt_stack.data(), lmnt_stack.size());
        if (lresult != LMNT_OK)
            goto lmnt_error;
            
        loperation = "archive load";
        printf("doing %s\n", loperation);
        lresult = lmnt_ictx_load_archive(&lctx, lmnt_archive_data.data(), lmnt_archive_data.size());
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "archive prepare";
        printf("doing %s\n", loperation);
        lresult = lmnt_ictx_prepare_archive(&lctx, nullptr);
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "def search";
        printf("doing %s\n", loperation);
        const lmnt_def* def = nullptr;
        lresult = lmnt_ictx_find_def(&lctx, "evaluate", &def);
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "setting args";
        printf("%s\n", loperation);
        lresult = lmnt_update_args(&lctx, def, 0, args.data(), args.size());
        if (lresult != LMNT_OK)
            goto lmnt_error;

        loperation = "executing def";
        printf("%s\n", loperation);
        std::vector<lmnt_value> lmnt_results(output.count);
        lresult = lmnt_execute(&lctx, def, lmnt_results.data(), lmnt_results.size());
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
    return result;
}