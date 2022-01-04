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
    // each element must also be 4-byte aligned
    const size_t names_len = std::accumulate(defs.begin(), defs.end(), 0ULL,
        [](size_t i, const element_lmnt_compiled_function& d) { return i + LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4); });
    // total length of all instructions in the code table (not including headers)
    const size_t all_instr_count = std::accumulate(defs.begin(), defs.end(), 0ULL,
        [](size_t i, const element_lmnt_compiled_function& d) { return i + d.instructions.size(); });
    const size_t consts_count = constants.size();
    const size_t data_count = 0;
    assert(names_len <= 0xFC);
    assert(all_instr_count <= 0x3FFFFFF0);
    assert(consts_count <= 0x3FFFFFFF);

    const size_t header_len = 0x1C;
    const size_t strings_len = names_len;
    // defs are constant size
    const size_t defs_len = 0x10 * defs.size();
    // code table entries are 4 bytes of header and then instructions
    const size_t code_len = 0x04 * defs.size() + all_instr_count * sizeof(lmnt_instruction);
    // we always write the number of data sections even if that number is zero
    const lmnt_loffset data_sec_count = 0;
    const size_t data_len = 0x04 + data_sec_count * (0x08 + 0x04 * data_count);
    // constants are just raw data
    const size_t consts_len = consts_count * sizeof(lmnt_value);

    // total size must be equal to the header + the tables
    // we sanity-check against this at the end
    const size_t total_size = header_len + strings_len + defs_len + code_len + data_len + consts_len;
    std::vector<char> buf;
    buf.resize(total_size);

    // track current index in the output buffer
    size_t idx = 0;

    lmnt_archive_header header;
    memset(&header, 0, sizeof(header));
    header.magic[0] = 'L';
    header.magic[1] = 'M';
    header.magic[2] = 'N';
    header.magic[3] = 'T';
    header.version_major = 0;
    header.version_minor = 0;
    header.strings_length = static_cast<uint32_t>(strings_len);
    header.defs_length = static_cast<uint32_t>(defs_len);
    header.code_length = static_cast<uint32_t>(code_len);
    header.data_length = static_cast<uint32_t>(data_len);
    header.constants_length = static_cast<uint32_t>(consts_len);

    memcpy(buf.data() + idx, &header, sizeof(header));
    idx += sizeof(header);

    size_t strings_start = idx;
    for (const auto& d : defs) {
        const size_t name_len = d.name.length();
        // we need the padded length of the string *without* the length bytes, so -2 at the end
        const size_t name_len_padded = LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4) - 2;
        buf[idx++] = char((name_len_padded >> 0) & 0xFF);
        buf[idx++] = char((name_len_padded >> 8) & 0xFF);

        memcpy(buf.data() + idx, d.name.data(), name_len);
        idx += name_len;
        // always add a null...
        buf[idx++] = '\0';
        // ... and then pad to 4-byte alignment
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
        // we wrote the strings in the same order we're writing the defs
        // so we can get away with just upping the index with each one
        string_idx += LMNT_ROUND_UP(0x02 + d.name.length() + 1, 4);
        // we'll also write the code in the same order
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

element_result element_interpreter_export_lmnt(
    element_interpreter_ctx* context,
    const element_declaration** decls,
    const char** funcnames,
    size_t decls_count,
    char* buffer,
    size_t* bufsize)
{
    if (!context)
        return ELEMENT_ERROR_API_INTERPRETER_CTX_IS_NULL;
    if (!decls || !funcnames)
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

    // we need to get all the constants we want in the archive ahead of time, from all functions
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

        ELEMENT_OK_OR_RETURN(element_lmnt_compile_function(lmnt_ctx, functions[i]->instruction, funcnames[i], constants, inputs_size, lmnt_functions[i]));
    }

    auto lmnt_archive_data = create_archive(lmnt_functions, constants);

    size_t current_bufsize = *bufsize;
    // always write the size of the archive back out to the user
    *bufsize = lmnt_archive_data.size();

    // if the user gave us a buffer, check it's big enough and write to it
    if (buffer) {
        if (lmnt_archive_data.size() > current_bufsize)
            return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

        memcpy(buffer, lmnt_archive_data.data(), lmnt_archive_data.size());
    }

    return ELEMENT_OK;
}