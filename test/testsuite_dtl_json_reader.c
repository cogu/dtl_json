/*****************************************************************************
* \file      testsuite_dtl_json_reader.c
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     Unit tests for dtl_json reader
*
* Copyright (c) 2019-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stddef.h>
#include "dtl_type.h"
#include "CuTest.h"
#include "dtl_json.h"
#include "filestream.h"

#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static void test_json_read_false(CuTest* tc);
static void test_json_read_true(CuTest* tc);
static void test_json_read_null(CuTest* tc);
static void test_json_read_i32(CuTest* tc);
static void test_json_read_u32(CuTest* tc);
static void test_json_read_i64(CuTest* tc);
static void test_json_read_string(CuTest* tc);
static void test_json_read_escaped_string(CuTest* tc);
static void test_json_read_empty_list(CuTest *tc);
static void test_json_read_list_of_i32(CuTest* tc);
static void test_json_read_list_of_empty_lists(CuTest* tc);
static void test_json_read_list_of_list_i32(CuTest* tc);
static void test_json_read_empty_object(CuTest* tc);
static void test_json_read_object(CuTest* tc);
static void test_json_read_object_with_array(CuTest* tc);
static void test_json_read_array_of_objects(CuTest* tc);
static void test_json_loads(CuTest* tc);
static void test_json_load_file(CuTest* tc);
static void test_json_read_errors(CuTest* tc);
static void test_json_read_null_args(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testsuite_dtl_json_reader(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_json_read_false);
   SUITE_ADD_TEST(suite, test_json_read_true);
   SUITE_ADD_TEST(suite, test_json_read_null);
   SUITE_ADD_TEST(suite, test_json_read_i32);
   SUITE_ADD_TEST(suite, test_json_read_u32);
   SUITE_ADD_TEST(suite, test_json_read_i64);
   SUITE_ADD_TEST(suite, test_json_read_string);
   SUITE_ADD_TEST(suite, test_json_read_escaped_string);
   SUITE_ADD_TEST(suite, test_json_read_empty_list);
   SUITE_ADD_TEST(suite, test_json_read_list_of_i32);
   SUITE_ADD_TEST(suite, test_json_read_list_of_empty_lists);
   SUITE_ADD_TEST(suite, test_json_read_list_of_list_i32);
   SUITE_ADD_TEST(suite, test_json_read_empty_object);
   SUITE_ADD_TEST(suite, test_json_read_object);
   SUITE_ADD_TEST(suite, test_json_read_object_with_array);
   SUITE_ADD_TEST(suite, test_json_read_array_of_objects);
   SUITE_ADD_TEST(suite, test_json_loads);
   SUITE_ADD_TEST(suite, test_json_load_file);
   SUITE_ADD_TEST(suite, test_json_read_errors);
   SUITE_ADD_TEST(suite, test_json_read_null_args);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_json_read_false(CuTest* tc)
{
   const char *input1 = "false";
   const char *input2 = "   false";
   dtl_dv_t *result;
   dtl_sv_t *sv;
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_BOOL, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_bool(sv, &ok) == false);
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_BOOL, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_bool(sv, &ok) == false);
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_true(CuTest* tc)
{
   const char *input1 = "true";
   const char *input2 = "   true";
   dtl_dv_t *result;
   dtl_sv_t *sv;
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_BOOL, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_bool(sv, &ok) == true);
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_BOOL, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_bool(sv, &ok) == true);
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_null(CuTest* tc)
{
   const char *input1 = "null";
   const char *input2 = "   null";
   dtl_dv_t *result;
   dtl_sv_t *sv;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_NONE, dtl_sv_type(sv));
   dtl_dec_ref(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_NONE, dtl_sv_type(sv));
   dtl_dec_ref(result);
}

static void test_json_read_i32(CuTest* tc)
{
   const char *input1 = "123";
   const char *input2 = "0";
   const char *input3 = "   -10";
   bool ok;
   dtl_dv_t *result;
   dtl_sv_t *sv;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(sv));
   CuAssertIntEquals(tc, 123, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(sv));
   CuAssertIntEquals(tc, 0, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input3);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(sv));
   CuAssertIntEquals(tc, -10, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_u32(CuTest* tc)
{
   const char *input1 = "4294967295";
   const char *input2 = "2147483648";
   bool ok;
   dtl_dv_t *result;
   dtl_sv_t *sv;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_U32, dtl_sv_type(sv));
   CuAssertUIntEquals(tc, 4294967295U, dtl_sv_to_u32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_U32, dtl_sv_type(sv));
   CuAssertUIntEquals(tc, 2147483648U, dtl_sv_to_u32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_i64(CuTest* tc)
{
   const char *input1 = "-2147483649";
   bool ok;
   dtl_dv_t *result;
   dtl_sv_t *sv;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_I64, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_i64(sv, &ok) == -2147483649LL);
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_string(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_sv_t *sv;
   const char *input1 = "\"\"";
   const char *input2 = "\"Test\"";
   const char *input3 = "\"\343\202\204\343\201\202\343\200\202\"";
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "Test", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input3);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "\343\202\204\343\201\202\343\200\202", dtl_sv_to_cstr(sv, &ok));
   dtl_dv_delete(result);
}

static void test_json_read_escaped_string(CuTest* tc)
{
   const char *input = "\"line1\\nline2\\ttab\\\"quoted\\\"\"";
   bool ok;
   dtl_dv_t *result = dtl_json_load_cstr(input);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type(result));
   dtl_sv_t *sv = (dtl_sv_t*) result;
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "line1\nline2\ttab\"quoted\"", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_empty_list(CuTest *tc)
{
   dtl_dv_t *result;
   dtl_av_t *av;
   const char *input1 = "[]";
   const char *input2 = "[ ]";
   const char *input3 = "[]\r\n";
   const char *input4 = "[\n]\n";

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 0, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 0, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input3);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 0, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input4);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 0, dtl_av_length(av));
   dtl_dv_delete(result);
}

static void test_json_read_list_of_i32(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_av_t *av;
   const char *input1 = "[14, 12, 92]";
   const char *input2 = "[1,2,3,4,5,6,7,8,9,10]";
   const char *input3 = "[\n"
         "   1,\n"
         "   2,\n"
         "   3,\n"
         "   4,\n"
         "   5\n"
         "]";

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 3, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 10, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input3);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 5, dtl_av_length(av));
   dtl_dv_delete(result);
}

static void test_json_read_list_of_empty_lists(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_av_t *av;
   const char *input1 = "[[],[]]";
   const char *input2 = "[\n[\n]\n,\n[\n]\n,\n[\n]\n]\n";

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 2, dtl_av_length(av));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 3, dtl_av_length(av));
   dtl_dv_delete(result);
}

static void test_json_read_list_of_list_i32(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_av_t *av;
   dtl_av_t *inner_array;
   dtl_sv_t *sv;
   const char *input1 = "[ [1, 2, 3], [4] ]";
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 2, dtl_av_length(av));
   inner_array = (dtl_av_t*) dtl_av_value(av, 0);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type((dtl_dv_t*) inner_array));
   CuAssertIntEquals(tc, 3, dtl_av_length(inner_array));
   sv = (dtl_sv_t*) dtl_av_value(inner_array, 0);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type((dtl_dv_t*) sv));
   CuAssertIntEquals(tc, 1, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   sv = (dtl_sv_t*) dtl_av_value(inner_array, 1);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type((dtl_dv_t*) sv));
   CuAssertIntEquals(tc, 2, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   sv = (dtl_sv_t*) dtl_av_value(inner_array, 2);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type((dtl_dv_t*) sv));
   CuAssertIntEquals(tc, 3, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   inner_array = (dtl_av_t*) dtl_av_value(av, 1);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type((dtl_dv_t*) inner_array));
   sv = (dtl_sv_t*) dtl_av_value(inner_array, 0);
   CuAssertIntEquals(tc, DTL_DV_SCALAR, dtl_dv_type((dtl_dv_t*) sv));
   CuAssertIntEquals(tc, 4, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);
}

static void test_json_read_empty_object(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_hv_t *hv;
   const char *input1 = "{}";
   const char *input2 = " {  }\n";

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   hv = (dtl_hv_t*) result;
   CuAssertIntEquals(tc, 0, dtl_hv_length(hv));
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   hv = (dtl_hv_t*) result;
   CuAssertIntEquals(tc, 0, dtl_hv_length(hv));
   dtl_dv_delete(result);
}

static void test_json_read_object(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_hv_t *hv;
   dtl_sv_t *sv;
   const char *input1 = "{\n"
                        "   \"Key1\":\t\"Value1\"\n"
                        "}\n";

   const char *input2 = "{ \"Key1\": \"Value1\", \"Key2\" : \"Value2\"\n"
                         ",\"Key3\": true}";
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   hv = (dtl_hv_t*) result;
   CuAssertIntEquals(tc, 1, dtl_hv_length(hv));
   sv = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Key1");
   CuAssertPtrNotNull(tc, sv);
   CuAssertStrEquals(tc, "Value1", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   dtl_dv_delete(result);

   result = dtl_json_load_cstr(input2);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   hv = (dtl_hv_t*) result;
   CuAssertIntEquals(tc, 3, dtl_hv_length(hv));
   sv = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Key1");
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "Value1", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   sv = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Key2");
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(sv));
   CuAssertStrEquals(tc, "Value2", dtl_sv_to_cstr(sv, &ok));
   CuAssertTrue(tc, ok);
   sv = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Key3");
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, DTL_SV_BOOL, dtl_sv_type(sv));
   CuAssertTrue(tc, dtl_sv_to_bool(sv, &ok));
   dtl_dv_delete(result);
}

static void test_json_read_object_with_array(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_hv_t *hv;
   dtl_av_t *av;
   dtl_sv_t *sv;
   const char *input1 = "{\n"
                        "   \"Numbers\": [\n"
                        "      1,\n"
                        "      2\n"
                        "   ]\n"
                        "}";
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   hv = (dtl_hv_t*) result;
   CuAssertIntEquals(tc, 1, dtl_hv_length(hv));
   av = (dtl_av_t*) dtl_hv_get_cstr(hv, "Numbers");
   CuAssertPtrNotNull(tc, av);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type((dtl_dv_t*) av));
   CuAssertIntEquals(tc, 2, dtl_av_length(av));
   sv = (dtl_sv_t*) dtl_av_value(av, 0);
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, 1, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);
   sv = (dtl_sv_t*) dtl_av_value(av, 1);
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, 2, dtl_sv_to_i32(sv, &ok));
   CuAssertTrue(tc, ok);

   dtl_dv_delete(result);
}

static void test_json_read_array_of_objects(CuTest* tc)
{
   dtl_dv_t *result;
   dtl_hv_t *hv;
   dtl_av_t *av;
   dtl_sv_t *name;
   dtl_sv_t *value;
   const char *input1 =
         "[\n"
         "   { \"Name\": \"first\",  \"Value\": 94},\n"
         "   { \"Name\": \"second\", \"Value\": 219},\n"
         "   { \"Name\": \"third\",  \"Value\": 614},\n"
         "   { \"Name\": \"fourth\", \"Value\": 3}\n"
         "]";
   bool ok;

   result = dtl_json_load_cstr(input1);
   CuAssertIntEquals(tc, DTL_DV_ARRAY, dtl_dv_type(result));
   av = (dtl_av_t*) result;
   CuAssertIntEquals(tc, 4, dtl_av_length(av));

   hv = (dtl_hv_t*) dtl_av_value(av, 0);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type((dtl_dv_t*) hv));
   name = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Name");
   value = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Value");
   CuAssertPtrNotNull(tc, name);
   CuAssertPtrNotNull(tc, value);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(name));
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(value));
   CuAssertStrEquals(tc, "first", dtl_sv_to_cstr(name, &ok));
   CuAssertTrue(tc, ok);
   CuAssertIntEquals(tc, 94, dtl_sv_to_i32(value, &ok));
   CuAssertTrue(tc, ok);

   hv = (dtl_hv_t*) dtl_av_value(av, 1);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type((dtl_dv_t*) hv));
   name = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Name");
   value = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Value");
   CuAssertPtrNotNull(tc, name);
   CuAssertPtrNotNull(tc, value);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(name));
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(value));
   CuAssertStrEquals(tc, "second", dtl_sv_to_cstr(name, &ok));
   CuAssertTrue(tc, ok);
   CuAssertIntEquals(tc, 219, dtl_sv_to_i32(value, &ok));
   CuAssertTrue(tc, ok);

   hv = (dtl_hv_t*) dtl_av_value(av, 2);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type((dtl_dv_t*) hv));
   name = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Name");
   value = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Value");
   CuAssertPtrNotNull(tc, name);
   CuAssertPtrNotNull(tc, value);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(name));
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(value));
   CuAssertStrEquals(tc, "third", dtl_sv_to_cstr(name, &ok));
   CuAssertTrue(tc, ok);
   CuAssertIntEquals(tc, 614, dtl_sv_to_i32(value, &ok));
   CuAssertTrue(tc, ok);

   hv = (dtl_hv_t*) dtl_av_value(av, 3);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type((dtl_dv_t*) hv));
   name = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Name");
   value = (dtl_sv_t*) dtl_hv_get_cstr(hv, "Value");
   CuAssertPtrNotNull(tc, name);
   CuAssertPtrNotNull(tc, value);
   CuAssertIntEquals(tc, DTL_SV_STR, dtl_sv_type(name));
   CuAssertIntEquals(tc, DTL_SV_I32, dtl_sv_type(value));
   CuAssertStrEquals(tc, "fourth", dtl_sv_to_cstr(name, &ok));
   CuAssertTrue(tc, ok);
   CuAssertIntEquals(tc, 3, dtl_sv_to_i32(value, &ok));
   CuAssertTrue(tc, ok);

   dtl_dv_dec_ref(result);
}

static void test_json_loads(CuTest* tc)
{
   adt_str_t *str = adt_str_new_cstr("{\"test\": 42}");
   CuAssertPtrNotNull(tc, str);

   dtl_dv_t *result = dtl_json_loads(str);
   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));

   dtl_hv_t *hv = (dtl_hv_t*) result;
   dtl_sv_t *sv = (dtl_sv_t*) dtl_hv_get_cstr(hv, "test");
   CuAssertPtrNotNull(tc, sv);
   CuAssertIntEquals(tc, 42, dtl_sv_to_i32(sv, NULL));

   dtl_dec_ref(result);
   adt_str_delete(str);
}

static void test_json_load_file(CuTest* tc)
{
   const char *filename = "test_read_file.json";
   FILE *fh = fopen(filename, "w");
   CuAssertPtrNotNull(tc, fh);
   fputs("{\"file_key\": \"file_value\", \"number\": 100}", fh);
   fclose(fh);

   fh = fopen(filename, "r");
   CuAssertPtrNotNull(tc, fh);
   dtl_dv_t *result = dtl_json_load(fh);
   fclose(fh);
   remove(filename);

   CuAssertPtrNotNull(tc, result);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(result));
   dtl_hv_t *hv = (dtl_hv_t*) result;
   dtl_sv_t *sv_str = (dtl_sv_t*) dtl_hv_get_cstr(hv, "file_key");
   CuAssertPtrNotNull(tc, sv_str);
   CuAssertStrEquals(tc, "file_value", dtl_sv_to_cstr(sv_str, NULL));

   dtl_sv_t *sv_num = (dtl_sv_t*) dtl_hv_get_cstr(hv, "number");
   CuAssertPtrNotNull(tc, sv_num);
   CuAssertIntEquals(tc, 100, dtl_sv_to_i32(sv_num, NULL));

   dtl_dec_ref(result);
}

static void test_json_read_errors(CuTest* tc)
{
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr(""));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("bad_token"));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("\"unterminated"));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("[1, 2,"));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("{\"key\": }"));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("{: 123}"));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr("[1, 2}"));
}

static void test_json_read_null_args(CuTest* tc)
{
   CuAssertPtrEquals(tc, NULL, dtl_json_load(NULL));
   CuAssertPtrEquals(tc, NULL, dtl_json_loads(NULL));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_cstr(NULL));
   CuAssertPtrEquals(tc, NULL, dtl_json_load_bstr(NULL, NULL));
}
