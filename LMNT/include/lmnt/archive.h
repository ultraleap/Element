#ifndef LMNT_ARCHIVE_H
#define LMNT_ARCHIVE_H

#include <stdint.h>
#include "lmnt/common.h"
#include "lmnt/opcodes.h"
#include "lmnt/extcalls.h"

typedef uint32_t lmnt_archive_flags;
enum
{
    LMNT_ARCHIVE_NONE          = (0U << 0),
    LMNT_ARCHIVE_VALIDATED     = (1U << 0),
    LMNT_ARCHIVE_USES_EXTCALLS = (1U << 1),
};

typedef struct lmnt_archive
{
    const char* data;
    size_t size;
    lmnt_archive_flags flags;
} lmnt_archive;

#pragma pack(push, 1)
typedef struct lmnt_archive_header
{
    char magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t reserved0;
    uint8_t reserved1;
    uint32_t strings_length;
    uint32_t defs_length;
    uint32_t code_length;
    uint32_t data_length;
    uint32_t constants_length;
} lmnt_archive_header;

typedef uint16_t lmnt_def_flags;
enum
{
    LMNT_DEFFLAG_NONE      = (0U << 0),
    LMNT_DEFFLAG_INTERFACE = (1U << 0),
    LMNT_DEFFLAG_EXTERN    = (1U << 1),
    LMNT_DEFFLAG_LAMBDA    = (1U << 2),
};

typedef struct lmnt_def
{
    uint16_t length;
    lmnt_offset name; // string
    lmnt_def_flags flags;
    lmnt_loffset code; // code
    uint16_t stack_count_unaligned;
    uint16_t stack_count_aligned;
    uint16_t base_args_count;
    uint16_t args_count;
    uint16_t rvals_count;
    uint8_t bases_count;
} lmnt_def;

typedef struct lmnt_instruction
{
    lmnt_opcode opcode;
    lmnt_offset arg1; // various
    lmnt_offset arg2; // various
    lmnt_offset arg3; // various
} lmnt_instruction;

typedef struct lmnt_code
{
    lmnt_loffset instructions_count;
} lmnt_code;

typedef struct lmnt_data_header
{
    lmnt_loffset sections_count;
} lmnt_data_header;

typedef struct lmnt_data_section
{
    lmnt_loffset offset;
    lmnt_loffset count;
} lmnt_data_section;
#pragma pack(pop)

lmnt_result lmnt_archive_init(lmnt_archive* archive, const char* data, size_t size);
lmnt_result lmnt_archive_print(const lmnt_archive* archive);

lmnt_result lmnt_get_constant(const lmnt_archive* archive, uint32_t offset, lmnt_value* value);
lmnt_result lmnt_get_constants(const lmnt_archive* archive, uint32_t offset, const lmnt_value** value);
lmnt_result lmnt_get_constants_count(const lmnt_archive* archive, lmnt_offset* value);

int32_t lmnt_get_string(const lmnt_archive* archive, uint32_t offset, const char** ptr);

lmnt_result lmnt_get_def(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_def** def);
lmnt_result lmnt_get_def_code(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_code** code, const lmnt_instruction** instructions);
lmnt_result lmnt_find_def(const lmnt_archive* archive, const char* name, const lmnt_def** def);
lmnt_result lmnt_get_def_bases(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_loffset** bases);

lmnt_result lmnt_get_code(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_code** code);
lmnt_result lmnt_get_code_instructions(const lmnt_archive* archive, lmnt_loffset offset, const lmnt_instruction** instrs);

lmnt_result lmnt_get_data_sections_count(const lmnt_archive* archive, lmnt_offset* count);
lmnt_result lmnt_get_data_section(const lmnt_archive* archive, lmnt_offset index, const lmnt_data_section** section);
lmnt_result lmnt_get_data_block(const lmnt_archive* archive, const lmnt_data_section* section, const lmnt_value** block);

lmnt_result lmnt_update_def_extcalls(lmnt_archive* archive, const lmnt_extcall_info* table, size_t table_count);

#endif
