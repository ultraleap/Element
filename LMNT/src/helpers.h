#ifndef LMNT_HELPERS_H
#define LMNT_HELPERS_H

#include "lmnt/archive.h"


#pragma pack(push, 1)
typedef struct
{
    uint16_t size;
} archive_string_header;
#pragma pack(pop)


//
// Access helper functions
//

static inline const lmnt_archive_header* get_header(const lmnt_archive* archive)
{
    return (const lmnt_archive_header*)archive->data;
}

static inline const char* get_strings_segment(const lmnt_archive* archive)
{
    return archive->data + sizeof(lmnt_archive_header);
}

static inline const char* get_defs_segment(const lmnt_archive* archive)
{
    return get_strings_segment(archive) + get_header(archive)->strings_length;
}

static inline const char* get_code_segment(const lmnt_archive* archive)
{
    return get_defs_segment(archive) + get_header(archive)->defs_length;
}

static inline const char* get_data_segment(const lmnt_archive* archive)
{
    return get_code_segment(archive) + get_header(archive)->code_length;
}

static inline const char* get_constants_segment(const lmnt_archive* archive)
{
    return get_data_segment(archive) + get_header(archive)->data_length;
}

static inline lmnt_offset value_to_offset(lmnt_value v)
{
    // fast floor
    return ((lmnt_offset)v) - (v < (lmnt_offset)v);
}

static inline size_t value_to_size_t(lmnt_value v)
{
    // fast floor
    return ((size_t)v) - (v < (size_t)v);
}



static inline lmnt_value validated_get_constant(const lmnt_archive* archive, uint32_t offset)
{
    return *(lmnt_value*)(get_constants_segment(archive) + offset);
}

static inline const lmnt_value* validated_get_constants(const lmnt_archive* archive, uint32_t offset)
{
    return (const lmnt_value*)(get_constants_segment(archive) + offset);
}

static inline lmnt_offset validated_get_constants_count(const lmnt_archive* archive)
{
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    return (lmnt_offset)(hdr->constants_length / sizeof(lmnt_value));
}

static inline const lmnt_def* validated_get_def(const lmnt_archive* archive, lmnt_loffset offset)
{
    return (const lmnt_def*)(get_defs_segment(archive) + offset);
}

static inline const lmnt_loffset* validated_get_def_bases(const lmnt_archive* archive, lmnt_loffset offset)
{
    return (const lmnt_loffset*)(get_defs_segment(archive) + offset + sizeof(lmnt_def));
}

static inline const lmnt_code* validated_get_code(const lmnt_archive* archive, lmnt_loffset offset)
{
    return (const lmnt_code*)(get_code_segment(archive) + offset);
}

static inline const lmnt_instruction* validated_get_code_instructions(const lmnt_archive* archive, lmnt_loffset offset)
{
    return (const lmnt_instruction*)(get_code_segment(archive) + offset + sizeof(lmnt_code));
}

static inline lmnt_offset validated_get_data_sections_count(const lmnt_archive* archive)
{
    return ((const lmnt_data_header*)get_data_segment(archive))->sections_count;
}

static inline const lmnt_data_section* validated_get_data_section(const lmnt_archive* archive, lmnt_offset index)
{
    return ((const lmnt_data_section*)(get_data_segment(archive) + sizeof(lmnt_data_header) + sizeof(lmnt_data_section) * index));
}

static inline const lmnt_value* validated_get_data_block(const lmnt_archive* archive, lmnt_loffset offset)
{
    return (const lmnt_value*)(get_data_segment(archive) + offset);
}

#endif
