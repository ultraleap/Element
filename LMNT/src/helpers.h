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

#endif
