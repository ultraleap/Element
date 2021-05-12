# LMNTIL

LMNTIL (Element Intermediate Language) is a bytecode format suitable for executing Element functions in constrained environments.

The main aims of LMNTIL and this supporting library are:

* Speed: functions should be fast to execute, and not prohibitively slow to load or initialise
* Size: both code and data size should be as small as reasonably possible
* Predictability: the execution time of any given function should be predictable
* Robustness: the library must validate user input and be robust in the face of unexpected data
* Simplicity: the library should not be unnecessarily complex to use or to maintain

In particular, where possible the library should minimise use of dynamic memory allocation - ideally not requiring any at all.

These cannot all be achieved in all situations; for parts of the library which contradict these aims (for example, a JIT compiler cannot typically be described as simple), these parts should be made optional and configurable.

## Example usage

```c
#include <lmnt/interpreter.h>

// define interpreter context and memory area for it to use
lmnt_ictx ctx;
char mem[8192];
lmnt_result result;

// initialise interpreter context
result = lmnt_init(&ctx, mem, sizeof(mem));
assert(result == LMNT_OK);

// acquire the archive from the user
const char* archive = get_archive_from_user();
size_t archive_size = get_archive_size_from_user();

// load the archive (this can also be loaded in stages)
result = lmnt_load_archive(&ctx, archive, archive_size);
assert(result == LMNT_OK);

// prepare and validate the archive to ensure it's valid
result = lmnt_prepare_archive(&ctx, NULL);
assert(result == LMNT_OK);

// find the function we want to execute
const lmnt_def* def = NULL;
result = lmnt_find_def(&ctx.archive, "AddThreeNumbers", &def);
assert(result == LMNT_OK);

// specify our arguments and provide space for return values
lmnt_value args[] = {1.0f, 2.0f, 3.0f};
lmnt_value rvals[1];
const size_t args_count = sizeof(args) / sizeof(lmnt_value);
const size_t rvals_count = sizeof(rvals) / sizeof(lmnt_value);

// set our current arguments
result = lmnt_update_args(&ctx, def, 0, args, args_count);
assert(result == LMNT_OK);

// execute the function
result = lmnt_execute(&ctx, def, rvals, rvals_count);
assert(result >= rvals_count);

// process our results!
printf("%.1f + %.1f + %.1f = %.1f\n", args[0], args[1], args[2], rvals[0]);
// 1.0 + 2.0 + 3.0 = 6.0
```

## Additional documentation

Functions are provided using [binary archives](doc/Bytecode.md), and loaded and executed using this C library. The function's operations are encoded using a [defined instruction set](doc/Instructions.md), with each function using a pre-defined amount of stack space, including arguments and return values.

Functions can be executed either using a platform-agnostic C99 interpreter, or using a platform-specific JIT compiler (where supported) for additional speed. Information about the memory model and intended usage can be found [here](doc/ExecutionModel.md).
