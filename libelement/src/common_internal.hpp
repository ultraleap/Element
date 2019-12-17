#pragma once

#include "element/common.h"
#include <cstdlib>
#include <cstdint>


#define ENSURE_NOT_NULL(t) if (t == nullptr) { return ELEMENT_ERROR_INVALID_PTR; }
