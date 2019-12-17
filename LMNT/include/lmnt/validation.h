#ifndef LMNT_VALIDATION_H
#define LMNT_VALIDATION_H

#include <stdlib.h>
#include "lmnt/common.h"
#include "lmnt/archive.h"

lmnt_validation_result lmnt_archive_validate(const lmnt_archive* archive, lmnt_offset ro_stack_count, lmnt_offset rw_stack_count);

#endif
