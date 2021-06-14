#include "lmnt/validation.h"
#include "helpers.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define DEFSTACK_LIMIT 16
static lmnt_validation_result check_defstack(lmnt_offset* defstack, size_t defstack_count, lmnt_offset next)
{
    if (defstack_count == 0)
        return LMNT_VALIDATION_OK;
    if (defstack_count >= DEFSTACK_LIMIT)
        return LMNT_VERROR_STACK_DEPTH;
    for (size_t i = 0; i < defstack_count - 1; ++i)
    {
        if (defstack[i] == next)
            return LMNT_VERROR_DEF_CYCLIC;
    }
    return LMNT_VALIDATION_OK;
}

static int32_t validate_string(const lmnt_archive* archive, lmnt_offset str_index)
{
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    // Do we have space for the string size?
    if (str_index + sizeof(archive_string_header) > hdr->strings_length)
        return LMNT_VERROR_STRING_HEADER;
    const archive_string_header* shdr = (const archive_string_header*)(get_strings_segment(archive) + str_index);
    str_index += sizeof(archive_string_header);
    // Zero-length strings are not valid
    if (shdr->size == 0)
        return LMNT_VERROR_STRING_SIZE;
    // Does this string extend beyond the end of the strings segment?
    if (shdr->size > (hdr->strings_length - str_index))
        return LMNT_VERROR_STRING_SIZE;
    // String entries must be 4-byte aligned
    if ((sizeof(archive_string_header) + shdr->size) % 4 != 0)
        return LMNT_VERROR_STRING_ALIGN;
    // Strings are C-strings: they must end in a null
    const char* last_char = get_strings_segment(archive) + str_index + shdr->size - 1;
    if (*last_char != '\0')
        return LMNT_VERROR_STRING_DATA;
    return sizeof(archive_string_header) + shdr->size;
}

static int32_t validate_code(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset code_index, size_t constants_count, size_t rw_stack_count, lmnt_offset* defstack, size_t defstack_count);

static int32_t validate_def_inner(const lmnt_archive* archive, lmnt_offset def_index, size_t constants_count, size_t rw_stack_count, lmnt_offset* defstack, size_t defstack_count)
{
    // Ensure we have no cycles and we're not going beyond our depth limit
    LMNT_V_OK_OR_RETURN(check_defstack(defstack, defstack_count, def_index));
    defstack[defstack_count] = def_index;
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    // Do we have space?
    if (def_index + sizeof(lmnt_def) > hdr->defs_length)
        return LMNT_VERROR_DEF_SIZE;
    const lmnt_def* dhdr = (const lmnt_def*)(get_defs_segment(archive) + def_index);
    def_index += sizeof(lmnt_def);
    // Is our name ref a valid string?
    int32_t svresult = validate_string(archive, dhdr->name);
    if (svresult < 0)
        return svresult;

    // Check we have enough stack space
    if (dhdr->stack_count_unaligned > rw_stack_count)
        return LMNT_VERROR_STACK_SIZE;

    if (!(dhdr->flags & LMNT_DEFFLAG_EXTERN))
    {
        // Is our code ref valid?
        int32_t cvresult = validate_code(archive, dhdr, dhdr->code, constants_count, dhdr->stack_count_unaligned, defstack, defstack_count);
        if (cvresult < 0)
            return cvresult;
    }

    // If interface/extern, check we have zero locals
    const lmnt_offset computed_stack_count = dhdr->args_count + dhdr->rvals_count;
    if ((dhdr->flags & (LMNT_DEFFLAG_EXTERN | LMNT_DEFFLAG_INTERFACE)) != 0 && dhdr->stack_count_unaligned != computed_stack_count)
        return LMNT_VERROR_DEF_HEADER;
    return sizeof(lmnt_def);
}

static int32_t validate_def(const lmnt_archive* archive, lmnt_offset def_index, size_t constants_count, size_t rw_stack_count)
{
    lmnt_offset defstack[DEFSTACK_LIMIT];
    return validate_def_inner(archive, def_index, constants_count, rw_stack_count, defstack, 0);
}

static inline lmnt_validation_result validate_operand_stack_read(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg, lmnt_offset count, size_t constants_count, size_t rw_stack_count)
{
    return (arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_stack_write(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg, lmnt_offset count, size_t constants_count, size_t rw_stack_count)
{
    // Old: ensure we're not writing to a constant
    // return (arg >= constants_count && arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
    // New: allow this since this space also now includes persisted variables
    return (arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_immediate32(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, size_t constants_count, size_t rw_stack_count)
{
    return LMNT_VALIDATION_OK;
}

static inline lmnt_validation_result validate_operand_immediate16(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg, size_t constants_count, size_t rw_stack_count)
{
    return LMNT_VALIDATION_OK;
}

static inline lmnt_validation_result validate_operand_dataload_section(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg1, size_t constants_count, size_t rw_stack_count)
{
    lmnt_offset sec_count = validated_get_data_sections_count(archive);
    return (arg1 < sec_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_dataload_imm(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset size, size_t constants_count, size_t rw_stack_count)
{
    LMNT_V_OK_OR_RETURN(validate_operand_dataload_section(archive, def, arg1, constants_count, rw_stack_count));
    const lmnt_data_section* sec = validated_get_data_section(archive, arg1);
    return ((lmnt_loffset)arg2 + (lmnt_loffset)size <= sec->count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_defptr(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, lmnt_offset stack, size_t constants_count, size_t rw_stack_count, lmnt_offset* defstack, size_t defstack_count)
{
    const lmnt_loffset target_offset = LMNT_COMBINE_OFFSET(arglo, arghi);
    lmnt_validation_result dvresult = validate_def_inner(archive, target_offset, constants_count, rw_stack_count, defstack, defstack_count + 1);
    if (dvresult < 0) return dvresult;
    const lmnt_def* target = validated_get_def(archive, target_offset);
    // We can't call an interface
    if (target->flags & LMNT_DEFFLAG_INTERFACE)
        return LMNT_VERROR_DEF_FLAGS;
    // We can't call another local function, it has to be extern
    if (!(target->flags & LMNT_DEFFLAG_EXTERN))
        return LMNT_VERROR_DEF_FLAGS;
    LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, target, stack, target->args_count, constants_count, rw_stack_count));
    LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, target, stack + target->args_count, target->rvals_count, constants_count, rw_stack_count));
    return LMNT_VALIDATION_OK;
}

static inline lmnt_validation_result validate_operand_codeptr(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, size_t constants_count, size_t rw_stack_count)
{
    const lmnt_loffset target_offset = LMNT_COMBINE_OFFSET(arglo, arghi);
    const lmnt_code* code = validated_get_code(archive, def->code);
    // allow a target offset to be "one off the end" to signal branching to end-of-function
    return (target_offset <= code->instructions_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static lmnt_validation_result validate_instruction(const lmnt_archive* archive, const lmnt_def* def, lmnt_opcode code, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3, size_t constants_count, size_t rw_stack_count, lmnt_offset* defstack, size_t defstack_count)
{
    switch (code)
    {
    case LMNT_OP_NOOP:
    case LMNT_OP_RETURN:
        return LMNT_VALIDATION_OK;
    // stack, null, stack
    case LMNT_OP_ASSIGNSS:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_ASSIGNVV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_ASSIGNSV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // immlo, immhi, stack
    case LMNT_OP_ASSIGNIIS:
    case LMNT_OP_ASSIGNIBS:
        LMNT_V_OK_OR_RETURN(validate_operand_immediate32(archive, def, arg1, arg2, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_ASSIGNIIV:
    case LMNT_OP_ASSIGNIBV:
        LMNT_V_OK_OR_RETURN(validate_operand_immediate32(archive, def, arg1, arg2, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_DLOADIIS:
        LMNT_V_OK_OR_RETURN(validate_operand_dataload_imm(archive, def, arg1, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_DLOADIIV:
        LMNT_V_OK_OR_RETURN(validate_operand_dataload_imm(archive, def, arg1, arg2, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_DLOADIRS:
        LMNT_V_OK_OR_RETURN(validate_operand_dataload_section(archive, def, arg1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_DLOADIRV:
        LMNT_V_OK_OR_RETURN(validate_operand_dataload_section(archive, def, arg1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // imm, null, stack
    case LMNT_OP_DSECLEN:
        LMNT_V_OK_OR_RETURN(validate_operand_dataload_section(archive, def, arg1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // stack, stack, stack
    case LMNT_OP_ADDSS:
    case LMNT_OP_SUBSS:
    case LMNT_OP_MULSS:
    case LMNT_OP_DIVSS:
    case LMNT_OP_MODSS:
    case LMNT_OP_POWSS:
    case LMNT_OP_LOG:
    case LMNT_OP_MINSS:
    case LMNT_OP_MAXSS:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_ADDVV:
    case LMNT_OP_SUBVV:
    case LMNT_OP_MULVV:
    case LMNT_OP_DIVVV:
    case LMNT_OP_MODVV:
    case LMNT_OP_POWVV:
    case LMNT_OP_MINVV:
    case LMNT_OP_MAXVV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_POWVS:
    case LMNT_OP_MINVS:
    case LMNT_OP_MAXVS:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // trig: stack, null, stack
    case LMNT_OP_SIN:
    case LMNT_OP_COS:
    case LMNT_OP_TAN:
    case LMNT_OP_ASIN:
    case LMNT_OP_ACOS:
    case LMNT_OP_ATAN:
    case LMNT_OP_SQRTS:
    case LMNT_OP_LN:
    case LMNT_OP_LOG2:
    case LMNT_OP_LOG10:
    case LMNT_OP_ABSS:
    case LMNT_OP_FLOORS:
    case LMNT_OP_ROUNDS:
    case LMNT_OP_CEILS:
    case LMNT_OP_TRUNCS:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // trig: stack, stack, stack
    case LMNT_OP_ATAN2:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_SINCOS:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg2, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // sqrt: stack, null, stack
    case LMNT_OP_SQRTV:
    case LMNT_OP_ABSV:
    case LMNT_OP_FLOORV:
    case LMNT_OP_ROUNDV:
    case LMNT_OP_CEILV:
    case LMNT_OP_TRUNCV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_SUMV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_INDEXRIS:
        // arg1 is validated at runtime, but we can check stackref is valid
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, arg2, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_INDEXRIR:
        // args are validated at runtime, but we can check stackrefs are valid
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_CMP:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_CMPZ:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_BRANCHZ:
    case LMNT_OP_BRANCHNZ:
    case LMNT_OP_BRANCHPOS:
    case LMNT_OP_BRANCHNEG:
    case LMNT_OP_BRANCHUN:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        // fallthrough
    case LMNT_OP_BRANCH:
    case LMNT_OP_BRANCHCEQ:
    case LMNT_OP_BRANCHCNE:
    case LMNT_OP_BRANCHCLT:
    case LMNT_OP_BRANCHCLE:
    case LMNT_OP_BRANCHCGT:
    case LMNT_OP_BRANCHCGE:
    case LMNT_OP_BRANCHCUN:
        return validate_operand_codeptr(archive, def, arg2, arg3, constants_count, rw_stack_count);
    case LMNT_OP_ASSIGNCEQ:
    case LMNT_OP_ASSIGNCNE:
    case LMNT_OP_ASSIGNCLT:
    case LMNT_OP_ASSIGNCLE:
    case LMNT_OP_ASSIGNCGT:
    case LMNT_OP_ASSIGNCGE:
    case LMNT_OP_ASSIGNCUN:
        LMNT_V_OK_OR_RETURN(validate_operand_immediate16(archive, def, arg1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_immediate16(archive, def, arg2, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // extern call: deflo, defhi, imm
    case LMNT_OP_EXTCALL:
        return validate_operand_defptr(archive, def, arg1, arg2, arg3, constants_count, rw_stack_count, defstack, defstack_count);
    default:
        return LMNT_VERROR_BAD_INSTRUCTION;
    }
}

static int32_t validate_code(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset code_index, size_t constants_count, size_t rw_stack_count, lmnt_offset* defstack, size_t defstack_count)
{
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    if (code_index + sizeof(lmnt_code) > hdr->code_length)
        return LMNT_VERROR_CODE_HEADER;
    const lmnt_code* chdr = (const lmnt_code*)(get_code_segment(archive) + code_index);
    code_index += sizeof(lmnt_code);
    if (code_index + chdr->instructions_count * sizeof(lmnt_instruction) > hdr->code_length)
        return LMNT_VERROR_CODE_SIZE;

    // validate instructions
    const lmnt_instruction* instrs = (const lmnt_instruction*)(get_code_segment(archive) + code_index);
    for (size_t i = 0; i < chdr->instructions_count; ++i)
        LMNT_V_OK_OR_RETURN(validate_instruction(archive, def, instrs[i].opcode, instrs[i].arg1, instrs[i].arg2, instrs[i].arg3, constants_count, rw_stack_count, defstack, defstack_count));

    // check for backbranches
    bool has_backbranches = false;
    for (lmnt_loffset i = 0; i < chdr->instructions_count; ++i)
    {
        if (LMNT_IS_BRANCH_OP(instrs[i].opcode))
        {
            const lmnt_loffset target = LMNT_COMBINE_OFFSET(instrs[i].arg2, instrs[i].arg3);
            if (target == i)
                return LMNT_VERROR_DEF_CYCLIC;
            else if (target < i)
                has_backbranches = true;
        }
    }
    const bool def_backbranches = (def->flags & LMNT_DEFFLAG_HAS_BACKBRANCHES) != 0;
    if (has_backbranches != def_backbranches)
        return LMNT_VERROR_DEF_FLAGS;

    return sizeof(lmnt_code) + (chdr->instructions_count * sizeof(lmnt_instruction));
}


lmnt_validation_result lmnt_archive_validate(lmnt_archive* archive, size_t memory_size, size_t* stack_count)
{
    if (archive->size < sizeof(lmnt_archive_header))
        return LMNT_VERROR_HEADER_MAGIC;
    // Header
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)(archive->data);
    if (strcmp(hdr->magic, "LMNT") != 0)
        return LMNT_VERROR_HEADER_MAGIC;
    const size_t segs_len = (size_t)hdr->strings_length + (size_t)hdr->defs_length + (size_t)hdr->code_length + (size_t)hdr->data_length + (size_t)hdr->constants_length;
    if (archive->size != sizeof(lmnt_archive_header) + segs_len)
        return LMNT_VERROR_SEGMENTS_SIZE;

    // All segments must be 4-byte aligned
    if ((hdr->strings_length | hdr->defs_length | hdr->code_length | hdr->data_length | hdr->constants_length) & 0x03)
        return LMNT_VERROR_SEGMENTS_ALIGN;

    // Determine our total stack count based on available memory space and how far in the constants are
    const size_t total_stack_count = (memory_size - (get_constants_segment(archive) - archive->data)) / sizeof(lmnt_value);
    const size_t constants_count = (hdr->constants_length / sizeof(lmnt_value));
    const size_t rw_stack_count = total_stack_count - constants_count;

    // Strings
    lmnt_offset str_index = 0;
    while (str_index < hdr->strings_length)
    {
        int32_t svresult = validate_string(archive, str_index);
        if (svresult >= 0)
            str_index += svresult;
        else
            return svresult;
    }

    // Data - use validated functions as we know we can read *some* data from here
    lmnt_offset data_section_count = validated_get_data_sections_count(archive);
    if (sizeof(lmnt_data_header) + data_section_count * sizeof(lmnt_data_section) > hdr->data_length)
        return LMNT_VERROR_ACCESS_VIOLATION;

    for (lmnt_offset s = 0; s < data_section_count; ++s)
    {
        const lmnt_data_section* section = validated_get_data_section(archive, s);
        if (section->offset < sizeof(lmnt_data_header) + sizeof(lmnt_data_section) * data_section_count)
            return LMNT_VERROR_ACCESS_VIOLATION;
        if ((size_t)section->offset + (size_t)section->count >= hdr->data_length)
            return LMNT_VERROR_ACCESS_VIOLATION;
    }

    // Defs
    lmnt_offset def_index = 0;
    while (def_index < hdr->defs_length)
    {
        lmnt_validation_result dvresult = validate_def(archive, def_index, constants_count, rw_stack_count);
        if (dvresult >= 0)
            def_index += dvresult;
        else
            return dvresult;
    }

    if (stack_count)
        *stack_count = total_stack_count;

    archive->flags |= LMNT_ARCHIVE_VALIDATED;
    return LMNT_VALIDATION_OK;
}
