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
* Data Table
* Constants Table

All segments must be 4-byte aligned to the start of the archive; the format provides for this without padding in most cases. Padding is prohibited except where explicitly specified.


### File Header

The file header is a constant size, and contains information about the file:
* Magic (4 bytes, `['L' 'M' 'N' 'T']`)
* File format version (2 x uint8, currently `[0, 0]`)
* Two reserved bytes
* Length (in bytes) of all other segments

The header can be summarised as:
```c
struct {
    const char magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t reserved0;
    uint8_t reserved1;
    uint32_t strings_length;
    uint32_t defs_length;
    uint32_t code_length;
    uint32_t data_length;
    uint32_t constants_length;
}
```


### Strings Table

The strings table contains a deduplicated list of all strings required by the archive. This typically consists of function names for searching at runtime.

The format of each entry can be summarised as:
```c
struct {
    uint16_t length;
    char value[length];
}
```
Note that all strings are stored as UTF-8 and are C-strings (i.e. the last character must be `'\0'`). The length field is in bytes, **not** characters and its value includes the null terminator.

Each entry must be aligned to 4 bytes; this must achieved by adding to length and appending zeros to the end of the string until `sizeof(uint16_t) + length` is a multiple of 4.

The table consists of a set of these entries, with no intermediate padding other than that required for 4-byte alignment.


### Definitions Table

The definitions table contains details of each function which can be executed within this archive.

The format can be summarised as:
```c
struct {
    uint16_t name;
    uint16_t flags;
    uint32_t code;
    uint16_t stack_count_unaligned;
    uint16_t stack_count_aligned;
    uint16_t args_count;
    uint16_t rvals_count;
}
```

* `name`: the offset (in bytes) into the string table where the name of this function can be found.
* `flags`: a set of flags which may apply to this function.
* `code`: an offset (in bytes) into the code table where the body of this function may be found. Unused if the `extern` flag is set.
* `stack_count_unaligned`: the total count (in elements) of stack required for this function to execute, including arguments and return values.
* `stack_count_aligned`: as above, but ensuring alignment for SIMD. Currently unused.
* `args_count`: the total count (in elements) of arguments accepted by this function.
* `rvals_count`: the total count (in elements) of values returned by this function.

The table consists of a set of these entries, with no intermediate padding.

The set of flags which may apply to a function definition are:

* `LMNT_DEFFLAG_EXTERN`: the function has an external definition which the LMNT host may have knowledge of
* `LMNT_DEFFLAG_INTERFACE`: the function has no body, and thus to be callable it must have a valid external definition
* `LMNT_DEFFLAG_HAS_BACKBRANCHES`: the function contains backward-facing branches (e.g. looping) and thus has no guarantee of halting

### Code Table

The code table contains a set of instructions to be executed.

The format can be summarised as:
```c
struct {
    uint32_t instructions_count;
    instruction instructions[instructions_count];
}
```
... where `instruction` is ...
```c
struct {
    uint16_t opcode;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t arg3;
}
```
Inputs are filled from the left, and outputs are filled from the right; thus, `arg1` and `arg2` are most commonly used for inputs and `arg3` is typically for results. For some instructions, inputs or outputs are specified across two arguments (`arg1` and `arg2` for inputs, `arg2` and `arg3` for outputs). In all of these cases, the first argument is the low half, and the second argument is the high half.

The table consists of a set of these entries, with no intermediate padding.


### Data Table

This is a table of data sections, each of which has a defined length, and consists entirely of LMNT value constants. These sections have LMNT instructions to retrieve data from them.

The start of the table is a header, which can be summarised as:

```c
struct {
    uint32_t section_count;
}
```

After the header, there are `section_count` entries describing each section, with its byte offset into the table and the number of constants in the section:

```c
struct {
    uint32_t offset;
    uint32_t count;
}
```

The remainder of the table consists of the data values present in the various sections. Sections _are_ permitted to overlap, as long as the entire section starts after the metadata at the start of the table, and ends before the end of the table.

### Constants Table

This is a table of constants used by functions within this archive. When loaded by the interpreter, the start of a function's stack will be the start of this table, allowing for easy "inline" access to these constants.

The table consists of a set of deduplicated `lmnt_value` objects, with no other metadata.

### Instructions

A detailed list of instructions and their arguments can be found [here](Instructions.md).