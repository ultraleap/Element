# LMNT Bytecode Format

## Aims
The bytecode should above all be fast to execute, and minimise additional memory usage. The bytecode should also be as compact as is feasible.

## Format (v0, unstable)
A single bytecode archive can contain multiple functions to be executed. All integers are encoded as little-endian.

The structure of the file is made up of multiple segments:
* File Header
* Strings Table
* Definitions Table
* Code Table
* Constants Table


### File Header

The file header is a constant size, and contains information about the file:
* Magic (4 bytes, `['L' 'M' 'N' 'T']`)
* File format version (2 x uint8, currently `[0, 0]`)
* Two reserved bytes
* Length (in bytes) of all other segments

The header can be summarised as:
```
struct {
    const char magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t reserved0;
    uint8_t reserved1;
    uint32_t strings_length;
    uint32_t defs_length;
    uint32_t code_length;
    uint32_t constants_length;
}
```


### Strings Table

The strings table contains a deduplicated list of all strings required by the archive. This typically consists of function names for searching at runtime.

The format of each entry can be summarised as:
```
struct {
	uint16_t length;
	char value[length];
}
```
Note that all strings are stored as UTF-8 and are C-strings (i.e. the last character must be '\0'). The length field is in bytes, **not** characters and its value includes the null terminator.

The table consists of a set of these entries. There must not be any padding between entries or at the end of the table.


### Definitions Table

The definitions table contains details of each function which can be executed within this archive.

The format can be summarised as:
```
struct {
    uint16_t length;
    uint16_t name;
    uint16_t flags;
    uint32_t code;
    uint16_t stack_count_unaligned;
    uint16_t stack_count_aligned;
    uint16_t base_args_count;
    uint16_t args_count;
    uint16_t rvals_count;
    uint8_t bases_count;
	uint32_t bases[bases_count];
}
```

* `length`: the full length of this entry, in bytes. (Note: this field has become less necessary over time and may be removed.)
* `name`: the offset (in bytes) into the string table where the name of this function can be found.
* `flags`: a set of flags which may apply to this function.
* `code`: an offset (in bytes) into the code table where the body of this function may be found. Unused if the `extern` flag is set.
* `stack_count_unaligned`: the total count (in elements) of stack required for this function to execute, including arguments and return values.
* `stack_count_aligned`: as above, but ensuring alignment for SIMD. Currently unused.
* `base_args_count`: the total count (in elements) of arguments provided by this function's base types.
* `args_count`: the total count (in elements) of arguments accepted by this function.
* `rvals_count`: the total count (in elements) of values returned by this function.

The table consists of a set of these entries. There must not be any padding between entries or at the end of the table.


### Code Table

The code table contains a set of instructions to be executed.

The format can be summarised as:
```
struct {
	uint32_t instructions_count;
	instruction instructions[instructions_count];
}
```
... where `instruction` is ...
```
struct {
	uint16_t opcode;
	uint16_t arg1;
	uint16_t arg2;
	uint16_t arg3;
}
```
Inputs are filled from the left, and outputs are filled from the right; thus, `arg1` and `arg2` are most commonly used for inputs and `arg3` is typically for results. For some instructions, inputs or outputs are specified across two arguments (`arg1` and `arg2` for inputs, `arg2` and `arg3` for outputs). In all of these cases, the first argument is the low half, and the second argument is the high half.

The table consists of a set of these entries. Padding is allowed between entries as well as at the end of the table, where it may be necessary (see next).


### Constants Table

This is a table of constants used by functions within this archive. When loaded by the interpreter, the start of a function's stack will be the start of this table.

The table consists of a set of deduplicated element values, with no other metadata.

**This table must be aligned to a minimum of 8 bytes within the archive data. Pad the end of the previous table if needed to achieve this.**


### Instructions

A detailed list of instructions and their arguments can be found [here](Instructions.md).