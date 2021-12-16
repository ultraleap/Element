#include <element/element.h>
#include <interpreter_internal.hpp>
#include <iostream>
#include <fstream>

#include "lmnt/opcodes.h"
#include "lmnt/archive.h"
#include "lmnt/interpreter.h"
#include "lmnt/compiler.hpp"
#include "lmnt/jit.h"


// TODO: support data sections?

static std::vector<char> create_archive(
    const std::vector<element_lmnt_compiled_function>& defs,
    const std::vector<lmnt_value>& constants)
{
    // each element is [size_lo, size_hi, 'a', 'b', 'c', ..., '\0'] - so 2 + length + 1
    const size_t names_len = std::accumulate(defs.begin(), defs.end(), 0ULL,
        [](size_t i, const element_lmnt_compiled_function& d) { return i + LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4); });
    const size_t all_instr_count = std::accumulate(defs.begin(), defs.end(), 0ULL,
        [](size_t i, const element_lmnt_compiled_function& d) { return i + d.instructions.size(); });
    const size_t consts_count = constants.size();
    const size_t data_count = 0;
    assert(names_len <= 0xFC);
    assert(all_instr_count <= 0x3FFFFFF0);
    assert(consts_count <= 0x3FFFFFFF);

    const size_t header_len = 0x1C;
    const size_t strings_len = names_len;
    const size_t defs_len = 0x10 * defs.size();
    const size_t code_len = 0x04 * defs.size() + all_instr_count * sizeof(lmnt_instruction);
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

    size_t strings_start = idx;
    for (const auto& d : defs) {
        const size_t name_len = d.name.length();
        const size_t name_len_padded = LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4) - 2;
        buf[idx++] = char((name_len_padded) & 0xFF);
        buf[idx++] = char(((name_len_padded) >> 8) & 0xFF);

        memcpy(buf.data() + idx, d.name.data(), name_len);
        idx += name_len;
        buf[idx++] = '\0';
        while (idx % 4)
            buf[idx++] = '\0';
    }

    size_t string_idx = 0;
    size_t code_idx = 0;
    for (const auto& d : defs) {
        lmnt_def def;
        def.name = lmnt_offset(string_idx);
        def.flags = d.flags;
        def.code = lmnt_loffset(code_idx);
        def.stack_count = lmnt_offset(d.total_stack_count());
        def.args_count = lmnt_offset(d.inputs_count);
        def.rvals_count = lmnt_offset(d.outputs_count);
        def.default_args_index = lmnt_offset(0);

        memcpy(buf.data() + idx, &def, sizeof(def));
        idx += sizeof(def);
        string_idx += LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4);
        code_idx += (0x04 + d.instructions.size() * sizeof(lmnt_instruction));
    }

    for (const auto& d : defs) {
        uint32_t instr_count = uint32_t(d.instructions.size());
        memcpy(buf.data() + idx, &instr_count, sizeof(uint32_t));
        idx += sizeof(uint32_t);

        memcpy(buf.data() + idx, d.instructions.data(), instr_count * sizeof(lmnt_instruction));
        idx += instr_count * sizeof(lmnt_instruction);
    }

    // TODO: data sections
    memcpy(buf.data() + idx, (const char*)(&data_sec_count), sizeof(lmnt_loffset));
    idx += sizeof(lmnt_loffset);

    memcpy(buf.data() + idx, constants.data(), consts_count * sizeof(lmnt_value));
    idx += consts_count * sizeof(lmnt_value);

    assert(idx == total_size);

    return buf;
}

struct instruction_deleter
{
    void operator()(element_instruction* instr)
    {
        element_instruction_delete(&instr);
    }
};

element_result element_interpreter_export_lmnt(element_interpreter_ctx* context, const element_declaration** decls, size_t decls_count, char* buffer, size_t* bufsize)
{
    if (!context)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;
    if (!decls)
        return ELEMENT_ERROR_API_DECLARATION_IS_NULL;
    if (decls_count == 0)
        return ELEMENT_ERROR_API_INVALID_INPUT;
    if (!bufsize)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    using instruction = std::unique_ptr<element_instruction, instruction_deleter>;
    std::vector<instruction> functions;
    functions.reserve(decls_count);

    for (size_t i = 0; i < decls_count; ++i) {
        element_instruction* instr;
        ELEMENT_OK_OR_RETURN(element_interpreter_compile_declaration(context, nullptr, decls[i], &instr));
        functions.emplace_back(instr);
    }

    element_lmnt_compiler_ctx lmnt_ctx;

    std::unordered_map<element_value, size_t> candidate_constants;
    for (size_t i = 0; i < functions.size(); ++i) {
        ELEMENT_OK_OR_RETURN(element_lmnt_find_constants(lmnt_ctx, functions[i]->instruction, candidate_constants));
    }

    std::vector<element_lmnt_compiled_function> lmnt_functions(functions.size());
    std::vector<element_value> constants;
    constants.reserve(candidate_constants.size());
    static const size_t constant_threshold = 1;

    for (const auto& [value, count] : candidate_constants) {
        if (count >= constant_threshold)
            constants.push_back(value);
    }

    for (size_t i = 0; i < functions.size(); ++i) {
        size_t inputs_size = 0;
        ELEMENT_OK_OR_RETURN(element_instruction_get_function_inputs_size(functions[i].get(), &inputs_size));

        size_t outputs_size = 0;
        ELEMENT_OK_OR_RETURN(element_instruction_get_size(functions[i].get(), &outputs_size));

        std::string name = decls[i]->decl->get_name(); // get_qualified_name() ?
        ELEMENT_OK_OR_RETURN(element_lmnt_compile_function(lmnt_ctx, functions[i]->instruction, name, constants, inputs_size, lmnt_functions[i]));
    }

    auto lmnt_archive_data = create_archive(lmnt_functions, constants);

    if (buffer && lmnt_archive_data.size() > *bufsize)
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    *bufsize = lmnt_archive_data.size();
    if (buffer)
        memcpy(buffer, lmnt_archive_data.data(), lmnt_archive_data.size());

    return ELEMENT_OK;
}