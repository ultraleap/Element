#include "lmnt/validation.h"
#include "lmnt/helpers.h"

#include <string.h>

static int32_t validate_string(const lmnt_archive* archive, lmnt_offset str_index)
{
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    // Do we have space for the string size?
    if (str_index + sizeof(archive_string_header) > hdr->strings_length)
        return LMNT_VERROR_STRING_HEADER;
    const archive_string_header* shdr = (const archive_string_header*)(get_strings_segment(archive) + str_index);
    str_index += sizeof(archive_string_header);
    // Does this string extend beyond the end of the strings segment?
    if (shdr->size > (hdr->strings_length - str_index))
        return LMNT_VERROR_STRING_SIZE;
    // Strings are C-strings: they must end in a null
    const char* last_char = get_strings_segment(archive) + str_index + shdr->size - 1;
    if (*last_char != '\0')
        return LMNT_VERROR_STRING_DATA;
    return sizeof(archive_string_header) + shdr->size;
}

static int32_t validate_code(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset code_index, lmnt_offset constants_count, lmnt_offset rw_stack_count);

#define DEFSTACK_LIMIT 16
static int32_t validate_def_inner(const lmnt_archive* archive, lmnt_offset def_index, lmnt_offset constants_count, lmnt_offset rw_stack_count, lmnt_offset* defstack, size_t defstack_count, lmnt_offset disallowed_flags)
{
    defstack[defstack_count] = def_index;
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    // Do we have space for the required components?
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

    // Check we don't have any flags which aren't allowed in this context
    if (dhdr->flags & disallowed_flags)
        return LMNT_VERROR_DEF_FLAGS;

    if (!(dhdr->flags & LMNT_DEFFLAG_EXTERN))
    {
        // Is our code ref valid?
        int32_t cvresult = validate_code(archive, dhdr, dhdr->code, constants_count, dhdr->stack_count_unaligned);
        if (cvresult < 0)
            return cvresult;
    }

    // If interface/extern, check we have zero locals
    const lmnt_offset computed_stack_count = dhdr->bases_count + dhdr->args_count + dhdr->rvals_count;
    if ((dhdr->flags | (LMNT_DEFFLAG_EXTERN & LMNT_DEFFLAG_INTERFACE)) != 0 && dhdr->stack_count_unaligned != computed_stack_count)
        return LMNT_VERROR_DEF_HEADER;

    // Bases
    if (def_index + sizeof(lmnt_loffset) * dhdr->bases_count > hdr->defs_length)
        return LMNT_VERROR_DEF_SIZE;
    const lmnt_loffset* bases = (const lmnt_loffset*)(get_defs_segment(archive) + def_index);
    for (size_t i = 0; i < dhdr->bases_count; ++i)
    {
        // Make sure we don't have any cycles in our bases
        for (size_t j = 0; j < defstack_count; ++j)
        {
            if (defstack[j] == bases[i])
                return LMNT_VERROR_DEF_CYCLIC;
        }
        // Make sure we're not going beyond our cycle stack limit
        if (defstack_count + 1 >= DEFSTACK_LIMIT)
            return LMNT_VERROR_STACK_DEPTH;
        // Check that this base is a valid def
        lmnt_validation_result bvresult = validate_def_inner(archive, bases[i], constants_count, rw_stack_count, defstack, defstack_count + 1, LMNT_DEFFLAG_NONE);
        if (bvresult < 0)
            return bvresult;
    }
    return sizeof(lmnt_def) + (sizeof(lmnt_loffset) * dhdr->bases_count);
}

static int32_t validate_def(const lmnt_archive* archive, lmnt_offset def_index, lmnt_offset constants_count, lmnt_offset rw_stack_count, lmnt_offset disallowed_flags)
{
    lmnt_offset defstack[DEFSTACK_LIMIT];
    return validate_def_inner(archive, def_index, constants_count, rw_stack_count, defstack, 0, disallowed_flags);
}

static inline lmnt_validation_result validate_operand_stack_read(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg, lmnt_offset count, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    return (arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_stack_write(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arg, lmnt_offset count, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    // Old: ensure we're not writing to a constant
    // return (arg >= constants_count && arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
    // New: allow this since this space also now includes persisted variables
    return (arg + count <= constants_count + rw_stack_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static inline lmnt_validation_result validate_operand_immediate(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    return LMNT_VALIDATION_OK;
}

static inline lmnt_validation_result validate_operand_defptr(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, lmnt_offset stack, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    const lmnt_loffset target_offset = LMNT_COMBINE_OFFSET(arglo, arghi);
    // Disallow the target being an interface: you can't call those...
    lmnt_validation_result dvresult = validate_def(archive, target_offset, constants_count, rw_stack_count, LMNT_DEFFLAG_INTERFACE);
    if (dvresult < 0) return dvresult;
    const lmnt_def* target;
    lmnt_result defresult = lmnt_get_def(archive, target_offset, &target);
    if (defresult != LMNT_OK)
        return LMNT_VERROR_DEF_HEADER;
    LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, target, stack, target->args_count, constants_count, rw_stack_count));
    LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, target, stack + target->args_count, target->rvals_count, constants_count, rw_stack_count));
    return LMNT_VALIDATION_OK;
}

static inline lmnt_validation_result validate_operand_codeptr(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset arglo, lmnt_offset arghi, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    const lmnt_loffset target_offset = LMNT_COMBINE_OFFSET(arglo, arghi);
    const lmnt_code* code;
    LMNT_OK_OR_RETURN(lmnt_get_code(archive, def->code, &code));
    return (target_offset < code->instructions_count) ? LMNT_VALIDATION_OK : LMNT_VERROR_ACCESS_VIOLATION;
}

static lmnt_validation_result validate_instruction(const lmnt_archive* archive, const lmnt_def* def, lmnt_opcode code, lmnt_offset arg1, lmnt_offset arg2, lmnt_offset arg3, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    switch (code)
    {
    case LMNT_OP_NOOP:
    case LMNT_OP_RETURN:
        return LMNT_VALIDATION_OK;
    case LMNT_OP_INDEXDIS:
        // arg1 is validated at runtime, but we can check stackref is valid
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, arg2, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_INDEXDID:
        // args are validated at runtime, but we can check stackrefs are valid
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg3, 1, constants_count, rw_stack_count));
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
        LMNT_V_OK_OR_RETURN(validate_operand_immediate(archive, def, arg1, arg2, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_ASSIGNIIV:
    case LMNT_OP_ASSIGNIBV:
        LMNT_V_OK_OR_RETURN(validate_operand_immediate(archive, def, arg1, arg2, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    // stack, stack, stack
    case LMNT_OP_ADDSS:
    case LMNT_OP_SUBSS:
    case LMNT_OP_MULSS:
    case LMNT_OP_DIVSS:
    case LMNT_OP_MODSS:
    case LMNT_OP_POWSS:
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
    case LMNT_OP_ABSS:
    case LMNT_OP_FLOORS:
    case LMNT_OP_ROUNDS:
    case LMNT_OP_CEILS:
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
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 4, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_SUMV:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 4, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_write(archive, def, arg3, 1, constants_count, rw_stack_count));
        return LMNT_VALIDATION_OK;
    case LMNT_OP_CMP:
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg1, 1, constants_count, rw_stack_count));
        LMNT_V_OK_OR_RETURN(validate_operand_stack_read(archive, def, arg2, 1, constants_count, rw_stack_count));
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
    // extern call: deflo, defhi, imm
    case LMNT_OP_EXTCALL:
        return validate_operand_defptr(archive, def, arg1, arg2, arg3, constants_count, rw_stack_count);
    default:
        return LMNT_VERROR_BAD_INSTRUCTION;
    }
}

static int32_t validate_code(const lmnt_archive* archive, const lmnt_def* def, lmnt_offset code_index, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)archive->data;
    if (code_index + sizeof(lmnt_code) > hdr->code_length)
        return LMNT_VERROR_CODE_HEADER;
    const lmnt_code* chdr = (const lmnt_code*)(get_code_segment(archive) + code_index);
    code_index += sizeof(lmnt_code);
    if (code_index + chdr->instructions_count * sizeof(lmnt_instruction) > hdr->code_length)
        return LMNT_VERROR_CODE_SIZE;

    const lmnt_instruction* instrs = (const lmnt_instruction*)(get_code_segment(archive) + code_index);
    for (size_t i = 0; i < chdr->instructions_count; ++i)
        LMNT_V_OK_OR_RETURN(validate_instruction(archive, def, instrs[i].opcode, instrs[i].arg1, instrs[i].arg2, instrs[i].arg3, constants_count, rw_stack_count));
    return sizeof(lmnt_code) + (chdr->instructions_count * sizeof(lmnt_instruction));
}


lmnt_validation_result lmnt_archive_validate(const lmnt_archive* archive, lmnt_offset constants_count, lmnt_offset rw_stack_count)
{
    if (archive->size < sizeof(lmnt_archive_header))
        return LMNT_VERROR_HEADER_MAGIC;
    // Header
    const lmnt_archive_header* hdr = (const lmnt_archive_header*)(archive->data);
    if (strcmp(hdr->magic, "LMNT") != 0)
        return LMNT_VERROR_HEADER_MAGIC;
    const size_t segs_len = (size_t)hdr->strings_length + (size_t)hdr->defs_length + (size_t)hdr->code_length + (size_t)hdr->constants_length;
    if (archive->size != sizeof(lmnt_archive_header) + segs_len)
        return LMNT_VERROR_SEGMENTS_SIZE;

    // The constants table MUST be 8-byte aligned within the archive
    const size_t constants_idx = sizeof(lmnt_archive_header) + hdr->strings_length + hdr->defs_length + hdr->code_length;
    if ((constants_idx % 8) != 0)
        return LMNT_VERROR_CONSTANTS_ALIGN;

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

    // Defs
    lmnt_offset def_index = 0;
    while (def_index < hdr->defs_length)
    {
        lmnt_validation_result dvresult = validate_def(archive, def_index, constants_count, rw_stack_count, LMNT_DEFFLAG_NONE);
        if (dvresult >= 0)
            def_index += dvresult;
        else
            return dvresult;
    }

    return LMNT_VALIDATION_OK;
}
