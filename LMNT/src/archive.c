#include "lmnt/archive.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#include "lmnt/extcalls.h"
#include "lmnt/validation.h"
#include "helpers.h"


//
// Archive handling
//

lmnt_result lmnt_archive_init(lmnt_archive* archive, const char* data, size_t size)
{
    if (!data)
        return LMNT_ERROR_INVALID_PTR;

    archive->data = data;
    archive->size = size;
    archive->flags = LMNT_ARCHIVE_NONE;
    return LMNT_OK;
}

lmnt_result lmnt_get_constant(const lmnt_archive* archive, uint32_t offset, lmnt_value* value)
{
    LMNT_ENSURE_VALIDATED(archive);
    *value = validated_get_constant(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_constants(const lmnt_archive* archive, uint32_t offset, const lmnt_value** value)
{
    LMNT_ENSURE_VALIDATED(archive);
    *value = validated_get_constants(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_constants_count(const lmnt_archive* archive, lmnt_offset* value)
{
    LMNT_ENSURE_VALIDATED(archive);
    *value = validated_get_constants_count(archive);
    return LMNT_OK;
}

int32_t lmnt_get_string(const lmnt_archive* archive, uint32_t offset, const char** ptr)
{
    LMNT_ENSURE_VALIDATED(archive);
    const uint16_t size = *(uint16_t*)(get_strings_segment(archive) + offset);
    if (ptr)
        *ptr = get_strings_segment(archive) + offset + sizeof(uint16_t);
    return (int32_t)size;
}

lmnt_result lmnt_get_def(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_def** def)
{
    LMNT_ENSURE_VALIDATED(archive);
    *def = validated_get_def(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_def_bases(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_loffset** bases)
{
    LMNT_ENSURE_VALIDATED(archive);
    *bases = validated_get_def_bases(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_def_code(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_code** code, const lmnt_instruction** instrs)
{
    LMNT_ENSURE_VALIDATED(archive);
    const char* const base = get_defs_segment(archive) + offset;
    const lmnt_def* hdr = (const lmnt_def*)base;
    *code = validated_get_code(archive, hdr->code);
    *instrs = validated_get_code_instructions(archive, hdr->code);
    return LMNT_OK;
}

lmnt_result lmnt_find_def(const lmnt_archive* archive, const char* name, const lmnt_def** def)
{
    LMNT_ENSURE_VALIDATED(archive);
    const char* pos = get_defs_segment(archive);
    const char* end = get_code_segment(archive);
    while (pos < end)
    {
        uint16_t len = *(uint16_t*)(pos);
        assert(len > 0);

        lmnt_offset nameoffset = *(lmnt_offset*)(pos + sizeof(uint16_t));
        const char* testname;
        int32_t nameresult = lmnt_get_string(archive, nameoffset, &testname);
        if (nameresult < 0) return nameresult;
        if (strncmp(name, testname, nameresult) == 0)
            return lmnt_get_def(archive, (lmnt_loffset)(pos - get_defs_segment(archive)), def);
        pos += len;
    }
    return LMNT_ERROR_NOT_FOUND;
}

lmnt_result lmnt_get_code(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_code** code)
{
    LMNT_ENSURE_VALIDATED(archive);
    *code = validated_get_code(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_code_instructions(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_instruction** instrs)
{
    LMNT_ENSURE_VALIDATED(archive);
    *instrs = validated_get_code_instructions(archive, offset);
    return LMNT_OK;
}

lmnt_result lmnt_get_data_sections_count(const lmnt_archive* archive, lmnt_offset* count)
{
    LMNT_ENSURE_VALIDATED(archive);
    *count = validated_get_data_sections_count(archive);
    return LMNT_OK;
}

lmnt_result lmnt_get_data_section(const lmnt_archive* archive, lmnt_offset index, const lmnt_data_section** section)
{
    LMNT_ENSURE_VALIDATED(archive);
    *section = validated_get_data_section(archive, index);
    return LMNT_OK;
}

lmnt_result lmnt_get_data_block(const lmnt_archive* archive, const lmnt_data_section* section, const lmnt_value** block)
{
    LMNT_ENSURE_VALIDATED(archive);
    *block = validated_get_data_block(archive, section->offset);
    return LMNT_OK;
}


lmnt_result lmnt_update_def_extcalls(lmnt_archive* archive, const lmnt_extcall_info* table, size_t table_count)
{
    LMNT_ENSURE_VALIDATED(archive);
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    size_t defindex = 0;
    while (defindex < hdr->defs_length)
    {
        lmnt_def* def = (lmnt_def*)(get_defs_segment(archive) + defindex);
        // If this def is an extern with a function body...
        if ((def->flags & LMNT_DEFFLAG_EXTERN) && !(def->flags & LMNT_DEFFLAG_INTERFACE))
        {
            const char* name = validated_get_string(archive, def->name);
            size_t index;
            LMNT_OK_OR_RETURN(lmnt_extcall_find_index(table, table_count, name, def->args_count, def->rvals_count, &index));
            def->code = (lmnt_loffset)index;
            // Mark archive as using extcalls
            archive->flags |= LMNT_ARCHIVE_USES_EXTCALLS;
        }
        defindex += def->length;
    }
    return LMNT_OK;
}


lmnt_result lmnt_archive_print(const lmnt_archive* archive)
{
    LMNT_ENSURE_VALIDATED(archive);
    // Header
    lmnt_loffset offset = 0;
    const lmnt_archive_header* hdr = (lmnt_archive_header*)(archive->data);
    LMNT_PRINTF("Header\n");
    LMNT_PRINTF("    Magic: %s\n", hdr->magic);
    LMNT_PRINTF("    Version: %u.%u\n", (uint32_t)hdr->version_major, (uint32_t)hdr->version_minor);
    LMNT_PRINTF("    Table Length, Strings: %u\n", hdr->strings_length);
    LMNT_PRINTF("    Table Length, Defs: %u\n", hdr->defs_length);
    LMNT_PRINTF("    Table Length, Code: %u\n", hdr->code_length);
    LMNT_PRINTF("    Table Length, Constants: %u\n", hdr->constants_length);

    LMNT_PRINTF("\nStrings\n");
    offset = 0;
    while (offset < hdr->strings_length)
    {
        const archive_string_header* shdr = (const archive_string_header*)(get_strings_segment(archive) + offset);
        LMNT_PRINTF("    [0x%04X] String (%2u): %s\n", offset, (uint32_t)shdr->size, get_strings_segment(archive) + offset + sizeof(archive_string_header));
        offset += sizeof(archive_string_header) + shdr->size;
    }

    LMNT_PRINTF("\nDefs\n");
    offset = 0;
    while (offset < hdr->defs_length)
    {
        const lmnt_def* dhdr = (const lmnt_def*)(get_defs_segment(archive) + offset);
        const char* name = "<INVALID>";
        lmnt_get_string(archive, dhdr->name, &name);
        if (strlen(name) == 0) name = "<anonymous>";
        LMNT_PRINTF("    [0x%04X] %s (%s)\n", offset, name, (dhdr->flags & LMNT_DEFFLAG_EXTERN) ? "extern" : ((dhdr->flags & LMNT_DEFFLAG_INTERFACE) ? "interface" : "function"));
        LMNT_PRINTF("                 Code offset: 0x%04X\n", (uint32_t)dhdr->code);
        LMNT_PRINTF("                 Stack count: %u / %u\n", (uint32_t)dhdr->stack_count_unaligned, (uint32_t)dhdr->stack_count_aligned);
        LMNT_PRINTF("                 Bases count: %u\n", (uint32_t)dhdr->bases_count);
        LMNT_PRINTF("                 Args count: %u\n", (uint32_t)dhdr->args_count);
        LMNT_PRINTF("                 RVals count: %u\n", (uint32_t)dhdr->rvals_count);
        offset += sizeof(lmnt_def);

        const lmnt_loffset* bases = (const lmnt_loffset*)(get_defs_segment(archive) + offset);
        for (size_t i = 0; i < dhdr->bases_count; ++i)
        {
            const lmnt_def* basedef;
            if (lmnt_get_def(archive, bases[i], &basedef) == LMNT_OK)
            {
                const char* basename = "<INVALID>";
                lmnt_get_string(archive, basedef->name, &basename);
                LMNT_PRINTF("                 Bases[%zu]: %s\n", i, basename);
            }
            else
            {
                LMNT_PRINTF("                 Bases[%zu]: <INVALID-DEF>\n", i);
            }
        }
        offset += sizeof(lmnt_loffset) * dhdr->bases_count;
    }

    LMNT_PRINTF("\nCode\n");
    offset = 0;
    while (offset + sizeof(lmnt_code) < hdr->code_length)
    {
        const lmnt_code* chdr = (const lmnt_code*)(get_code_segment(archive) + offset);
        LMNT_PRINTF("    [0x%04X] Function body:\n", offset);
        offset += sizeof(lmnt_code);

        const lmnt_instruction* insts = (const lmnt_instruction*)(get_code_segment(archive) + offset);
        for (size_t i = 0; i < chdr->instructions_count; ++i)
            LMNT_PRINTF("                 Instructions[%3zu]: %10s %04X %04X %04X\n", i, lmnt_opcode_info[insts[i].opcode].name, insts[i].arg1, insts[i].arg2, insts[i].arg3);

        offset += chdr->instructions_count * sizeof(lmnt_instruction);
    }

    LMNT_PRINTF("\nConstants\n");
    offset = 0;
    while (offset < hdr->constants_length)
    {
        LMNT_PRINTF("    [0x%04X] Constant: %f\n", offset, *(lmnt_value*)(get_constants_segment(archive) + offset));
        offset += sizeof(lmnt_value);
    }

    LMNT_PRINTF("\n");
    return LMNT_OK;
}
