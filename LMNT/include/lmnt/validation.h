#ifndef LMNT_VALIDATION_H
#define LMNT_VALIDATION_H

#include <stdlib.h>
#include "lmnt/common.h"
#include "lmnt/archive.h"

lmnt_validation_result lmnt_archive_validate(lmnt_archive* archive, size_t memory_size, size_t* stack_count);

#define LMNT_ENSURE_VALIDATED(a) {\
    if (!((a)->flags & LMNT_ARCHIVE_VALIDATED)) \
        return LMNT_ERROR_UNPREPARED_ARCHIVE; \
}

#endif
