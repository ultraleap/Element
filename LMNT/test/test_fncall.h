#include "CUnit/CUnitCI.h"
#include "lmnt/interpreter.h"
#include "testhelpers.h"
#include <stdio.h>
#include <stdbool.h>

#if !defined(TESTSETUP_INCLUDED)
#error "This file cannot be included without a testsetup header already having been included"
#endif


static lmnt_result test_extcall_good(const lmnt_ictx* ctx, const lmnt_extcall_info* callinfo, const lmnt_value* args, lmnt_value* rvals)
{
    rvals[0] = args[0] * 4;
    return LMNT_OK;
}

static lmnt_result test_extcall_bad(const lmnt_ictx* ctx, const lmnt_extcall_info* callinfo, const lmnt_value* args, lmnt_value* rvals)
{
    return -123456789;
}

static const char extcall_name[] = "extcall";

static archive create_archive_array_with_extcall(const char* def_name, uint16_t args_count, uint16_t rvals_count, uint16_t stack_count, uint32_t instr_count, uint32_t data_count, uint32_t consts_count, ...)
{
    const size_t name_len = strlen(def_name);
    const size_t extname_len = strlen(extcall_name);
    assert(name_len <= 0xFE);
    assert(instr_count <= 0x3FFFFFF0);
    assert(consts_count <= 0x3FFFFFFF);

    const size_t header_len = 0x1C;
    const size_t strings_len = (0x02 + name_len + 1) + (0x02 + extname_len + 1);
    const size_t defs_len = 0x15 * 2;
    uint32_t code_len = 0x04 + instr_count * sizeof(lmnt_instruction);
    const uint32_t code_padding = (4 - ((header_len + strings_len + defs_len + code_len) % 4)) % 4;
    code_len += code_padding;
    const lmnt_loffset data_sec_count = (data_count > 0);
    uint32_t data_len = 0x04 + data_sec_count * (0x08 + 0x04 * data_count);
    const uint32_t consts_len = consts_count * sizeof(lmnt_value);

    const size_t total_size = header_len + strings_len + defs_len + code_len + data_len + consts_len;
    char* buf = (char*)calloc(total_size, sizeof(char));

    size_t idx = 0;
    const char header[] = {
        'L', 'M', 'N', 'T',
        0x00, 0x00, 0x00, 0x00,
        strings_len & 0xFF, (strings_len >> 8) & 0xFF, (strings_len >> 16) & 0xFF, (strings_len >> 24) & 0xFF, // strings length
        defs_len & 0xFF, (defs_len >> 8) & 0xFF, (defs_len >> 16) & 0xFF, (defs_len >> 24) & 0xFF, // defs length
        code_len & 0xFF, (code_len >> 8) & 0xFF, (code_len >> 16) & 0xFF, (code_len >> 24) & 0xFF, // code length
        data_len & 0xFF, (data_len >> 8) & 0xFF, (data_len >> 16) & 0xFF, (data_len >> 24) & 0xFF, // data length
        consts_len & 0xFF, (consts_len >> 8) & 0xFF, (consts_len >> 16) & 0xFF, (consts_len >> 24) & 0xFF // constants_length
    };
    memcpy(buf + idx, header, sizeof(header));
    idx += sizeof(header);

    buf[idx] = (name_len + 1) & 0xFF;
    idx += 2;
    memcpy(buf + idx, def_name, name_len);
    idx += name_len;
    buf[idx++] = '\0';
    buf[idx] = (extname_len + 1) & 0xFF;
    idx += 2;
    memcpy(buf + idx, extcall_name, extname_len);
    idx += extname_len;
    buf[idx++] = '\0';

    const char def[] = {
        0x15, 0x00, // defs[0].length
        0x00, 0x00, // defs[0].name
        0x00, 0x00, // defs[0].flags
        0x00, 0x00, 0x00, 0x00, // defs[0].code
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_unaligned
        stack_count & 0xFF, (stack_count >> 8) & 0xFF, // defs[0].stack_count_aligned
        0x00, 0x00, // defs[0].base_args_count
        args_count & 0xFF, (args_count >> 8) & 0xFF, // defs[0].args_count
        rvals_count & 0xFF, (rvals_count >> 8) & 0xFF, // defs[0].rvals_count
        0x00        // defs[0].bases_count
    };
    memcpy(buf + idx, def, sizeof(def));
    idx += sizeof(def);

    const char extcall[] = {
        0x15, 0x00, // defs[1].length
        (uint8_t)(0x02 + name_len + 1), 0x00, // defs[1].name
        LMNT_DEFFLAG_EXTERN, 0x00, // defs[1].flags
        0x00, 0x00, 0x00, 0x00, // defs[1].code
        0x02, 0x00, // defs[1].stack_count_unaligned
        0x02, 0x00, // defs[1].stack_count_aligned
        0x00, 0x00, // defs[1].base_args_count
        0x01, 0x00, // defs[1].args_count
        0x01, 0x00, // defs[1].rvals_count
        0x00        // defs[1].bases_count
    };
    memcpy(buf + idx, extcall, sizeof(extcall));
    idx += sizeof(extcall);

    memcpy(buf + idx, (const char*)(&instr_count), sizeof(uint32_t));
    idx += sizeof(uint32_t);

    va_list args;
    va_start(args, consts_count);
    for (size_t i = 0; i < instr_count; ++i) {
        for (size_t j = 0; j < 8; ++j) {
            buf[idx++] = va_arg(args, int); // actually char, but va_arg requires int
        }
    }

    idx += code_padding;
    memcpy(buf + idx, (const char*)(&data_sec_count), sizeof(lmnt_loffset));
    idx += sizeof(lmnt_loffset);
    if (data_sec_count) {
        const lmnt_data_section sec = {sizeof(lmnt_data_header) + sizeof(lmnt_data_section), data_count};
        memcpy(buf + idx, (const char*)(&sec), sizeof(lmnt_data_section));
        idx += sizeof(lmnt_data_section);

        for (size_t i = 0; i < data_count; ++i) {
            lmnt_value val = (lmnt_value)va_arg(args, double); // actually lmnt_value, but va_arg requires double
            memcpy(buf + idx, (const char*)(&val), sizeof(val));
            idx += sizeof(val);
        }
    }

    for (size_t i = 0; i < consts_count; ++i) {
        // TODO: handle lmnt_value not being float
        lmnt_value val = (lmnt_value)va_arg(args, double); // actually lmnt_value, but va_arg requires double
        memcpy(buf + idx, (const char*)(&val), sizeof(val));
        idx += sizeof(val);
    }

    assert(idx == total_size);

    archive a = {buf, total_size};
    return a;
}



static void test_extcall_direct(void)
{
    lmnt_extcall_info extcalls[] = {
        { "extcall", 1, 1, (lmnt_extcall_fn)(&test_extcall_good) }
    };
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_extcalls_set(ctx, extcalls, 1), LMNT_OK);

    archive a = create_archive_array_with_extcall("test", 3, 1, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x15, 0x00, 0x02)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "extcall", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 12.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -7.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -28.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0f, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UPDATE_ARGS(ctx, fndata, 0, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && signbit(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    extcalls[0].function = (lmnt_extcall_fn)(&test_extcall_bad);

    a = create_archive_array_with_extcall("test", 3, 1, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x15, 0x00, 0x02)
    );
    TEST_LOAD_ARCHIVE(ctx, "extcall", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), -123456789);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_extcall_indirect(void)
{
    lmnt_extcall_info extcalls[] = {
        { "extcall", 1, 1, (lmnt_extcall_fn)(&test_extcall_good) }
    };
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_extcalls_set(ctx, extcalls, 1), LMNT_OK);

    archive a = create_archive_array_with_extcall("test", 3, 1, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x15, 0x00, 0x02)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    lmnt_value rvals[1];
    const size_t rvals_count = sizeof(rvals)/sizeof(lmnt_value);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 12.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, -1.0f, -4.0f, -7.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], -28.0, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 0.0f, 0.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_DOUBLE_EQUAL(rvals[0], 0.0f, FLOAT_ERROR_MARGIN);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 0.0f, 0.0f, nanf(""));
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isnan(rvals[0]));

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f, -INFINITY);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), rvals_count);
    CU_ASSERT_TRUE(isinf(rvals[0]) && signbit(rvals[0]));

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);


    extcalls[0].function = (lmnt_extcall_fn)(&test_extcall_bad);

    a = create_archive_array_with_extcall("test", 3, 1, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x15, 0x00, 0x02)
    );
    TEST_LOAD_ARCHIVE(ctx, "test", a, fndata);
    delete_archive_array(a);

    TEST_UPDATE_ARGS(ctx, fndata, 0, 1.0f, 2.0f, 3.0f);
    CU_ASSERT_EQUAL(TEST_EXECUTE(ctx, fndata, rvals, rvals_count), -123456789);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}

static void test_extcall_recursion(void)
{
    lmnt_extcall_info extcalls[] = {
        { "extcall", 1, 1, (lmnt_extcall_fn)(&test_extcall_good) }
    };
    CU_ASSERT_EQUAL_FATAL(lmnt_ictx_extcalls_set(ctx, extcalls, 1), LMNT_OK);

    archive a = create_archive_array_with_extcall("test", 3, 1, 4, 1, 0, 0,
        LMNT_OP_BYTES(LMNT_OP_EXTCALL, 0x00, 0x00, 0x02)
    );
    test_function_data fndata = { NULL, NULL };
    TEST_LOAD_ARCHIVE_FAILS_VALIDATION(ctx, "test", a, fndata, LMNT_ERROR_INVALID_ARCHIVE, LMNT_VERROR_DEF_CYCLIC);
    delete_archive_array(a);

    TEST_UNLOAD_ARCHIVE(ctx, a, fndata);
}



MAKE_REGISTER_SUITE_FUNCTION(fncall,
    CUNIT_CI_TEST(test_extcall_direct),
    CUNIT_CI_TEST(test_extcall_indirect),
    CUNIT_CI_TEST(test_extcall_recursion)
);