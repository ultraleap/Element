# LMNT's Execution Model

The LMNT runtime can best be described as a variable-count register machine: each function has a fixed-size stack within which it can do whatever it wants.

## Setup

These are the steps that take an LMNT runtime from having an archive sat in external memory to being ready to execute:

1. Interpreter is initialised using `lmnt_init`
   * This involves handing the interpreter a static block of memory to work with
2. Archive is loaded into the interpreter using `lmnt_load_archive`
3. Archive is "prepared"/validated using `lmnt_prepare_archive`
   * This performs extensive validation on the archive's contents
     * The archive header describes the size of the archive's tables
     * Each string in the strings table is checked to ensure that its length matches and that it ends with a null terminator
     * Each def entry is checked to ensure it is internally consistent and any bases described are also valid defs
     * Code for every def is checked to ensure it does not attempt to run any illegal instructions or perform any out-of-bounds access
     * The data table is checked for consistency
     * The alignment of the data table and constants table is verified
4. A def is located via `lmnt_find_def`
5. The def's inputs are set via `lmnt_update_args`
6. Either:
   * The def can be executed in the interpreter using `lmnt_execute`
   * The def can be JIT-compiled using `lmnt_jit_compile` and then executed with `lmnt_jit_execute`


## Memory Model

The (data) memory model is identical between the interpreter and the JIT-compiled versions.

LMNT has a massively simplified stack model: there is only one stack frame, and there are no function calls. Essentially, in an LMNT function, everything is inlined. There is branching, which could allow for very simplified routines - but as a rule branching is discouraged, as it hurts performance and (if branches ever go "backwards") makes execution less predictable.

The layout of the LMNT memory area (handed to the interpreter on initialisation) is summarised below, using an example archive with the following properties:

* Memory area size is 2048 bytes
* Total archive size is 384 bytes, and contains one function
* Header, strings, defs, code and data total 256 bytes
* Constants and persistent variables make up 128 bytes (32 values, half each)
* There are 8 arguments and 8 return values in the function

### Current Model

| Address            | Contents                                    | Stack Index       |
| -----------------: | :------------------------------------------ | :---------------- |
| `0x01C0 - 0x07FF`  | Ephemeral stack area                        | `0x0030 - 0x01BF` |
| `0x01A0 - 0x01BF`  | Function return values                      | `0x0028 - 0x002F` |
| `0x0180 - 0x019F`  | Function arguments                          | `0x0020 - 0x0027` |
| `0x0100 - 0x017F`  | Persistent stack area (constants)           | `0x0000 - 0x001F` |
| `0x0000 - 0x00FF`  | Archive (header, strings, defs, code, data) |                   |

This may be surprising - the constants are on the stack?! This comes with some upsides:
* Access to constants located in the archive is free - they already have a stack location
* Loading the constants in is remarkably easy - we already did it when we loaded the archive
* Constants used by multiple functions can be deduplicated

It does, however, bring some limitations:
* It effectively limits the number of constants we can have around based on the stack addressing mechanism (the data table can be used for larger numbers of constants)
* It complicates generating archives - you have to know how many constants are in the entire archive before you can compile any functions!

## Execution

As you might expect, this differs substantially between the interpreter and the JIT compiler.

### Interpreter

Within `lmnt_execute`, after setting up the environment the main loop simply iterates over each instruction in the specified function, calling out to functions defined for each instruction using a jump table. The functions are given access to the interpreter context (and thus the stack), as well as the three arguments provided with the instruction.

Once the last instruction has been executed or a `RETURN` instruction is encountered, the loop exits, the return values are copied out from the stack into user-provided memory (do we want to allow users to just access these in-place rather than forcing a copy?) and control is returned to the user.

If the user requests interruption of execution, this is achieved by swapping out the jump table used to execute the instruction functions for one which unconditionally returns `LMNT_INTERRUPTED` for any instruction. This causes the loop to exit and return that same status to the user.


### JIT Compiler

First, the user must call `lmnt_jit_compile` to generate a compiled version of the specified function. The resulting pointer is a normal C-linkage function which will execute the LMNT function. The compilation process currently requires dynamic memory allocation, although there are plans to allow this to use a second static block.

Once this is done, the user can then call `lmnt_jit_execute` passing in the compiled function; this sets up the environment and then calls the compiled function, which executes and returns.

There is currently no way to interrupt execution of the JIT-compiled function since it's just native code executing; there are speculative platform-specific ways of achieving this, and functions in the library to provide the necessary data, but they would require getting your hands _very_ dirty (see: swapping the task's instruction pointer out from under it while it's suspended).

More detailed information about the implementation of the JIT compiler can be found [here](JIT.md).