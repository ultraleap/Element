#ifndef LMNT_VALIDATION_H
#define LMNT_VALIDATION_H

#include <stdlib.h>
#include "lmnt/common.h"
#include "lmnt/archive.h"

lmnt_validation_result lmnt_archive_validate(const lmnt_archive* archive, size_t memory_size, size_t* stack_count);

#endif
