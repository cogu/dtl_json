/*****************************************************************************
* \file      testsuite_dtl_json_writer.c
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     Unit tests for dtl_json writer
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
#include <stdint.h>
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
static void test_json_write_true(CuTest* tc);
static void test_json_write_false(CuTest* tc);
static void test_json_write_null(CuTest* tc);
static void test_json_write_i32(CuTest* tc);
static void test_json_write_i64(CuTest* tc);
static void test_json_write_i32_list_no_indent(CuTest* tc);
static void test_json_write_i32_list_with_indent(CuTest* tc);
static void test_json_write_i32_list_inside_list_with_indent(CuTest* tc);
static void test_json_write_i32_list_inside_list_no_indent(CuTest* tc);
static void test_json_write_u32(CuTest* tc);
static void test_json_write_u32_list_no_indent(CuTest* tc);
static void test_json_write_u32_list_with_indent(CuTest* tc);
static void test_json_write_small_object_no_indent(CuTest* tc);
static void test_json_write_small_object_with_indent(CuTest* tc);
static void test_json_write_small_objects_with_indent(CuTest* tc);
static void test_json_write_string(CuTest* tc);
static void test_json_write_string_list_no_indent(CuTest* tc);
static void test_json_write_string_list_with_indent(CuTest* tc);
static void test_json_write_utf8_string_list_with_indent(CuTest* tc);
static void test_json_dump_file(CuTest* tc);
static void test_json_write_null_args(CuTest* tc);

//////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
CuSuite* testsuite_dtl_json_writer(void)
{
   CuSuite* suite = CuSuiteNew();

   SUITE_ADD_TEST(suite, test_json_write_true);
   SUITE_ADD_TEST(suite, test_json_write_false);
   SUITE_ADD_TEST(suite, test_json_write_null);
   SUITE_ADD_TEST(suite, test_json_write_i32);
   SUITE_ADD_TEST(suite, test_json_write_i64);
   SUITE_ADD_TEST(suite, test_json_write_i32_list_no_indent);
   SUITE_ADD_TEST(suite, test_json_write_i32_list_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_i32_list_inside_list_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_i32_list_inside_list_no_indent);
   SUITE_ADD_TEST(suite, test_json_write_u32);
   SUITE_ADD_TEST(suite, test_json_write_u32_list_no_indent);
   SUITE_ADD_TEST(suite, test_json_write_u32_list_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_small_object_no_indent);
   SUITE_ADD_TEST(suite, test_json_write_small_object_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_small_objects_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_string);
   SUITE_ADD_TEST(suite, test_json_write_string_list_no_indent);
   SUITE_ADD_TEST(suite, test_json_write_string_list_with_indent);
   SUITE_ADD_TEST(suite, test_json_write_utf8_string_list_with_indent);
   SUITE_ADD_TEST(suite, test_json_dump_file);
   SUITE_ADD_TEST(suite, test_json_write_null_args);

   return suite;
}

//////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
static void test_json_write_true(CuTest* tc)
{
   const int indent = 3;
   dtl_sv_t *sv;
   adt_str_t *output;

   sv = dtl_sv_make_bool(true);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "true", adt_str_cstr(output));
   dtl_sv_delete(sv);
   adt_str_delete(output);
}

static void test_json_write_false(CuTest* tc)
{
   const int indent = 3;
   dtl_sv_t *sv;
   adt_str_t *output;

   sv = dtl_sv_make_bool(false);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "false", adt_str_cstr(output));
   dtl_sv_delete(sv);
   adt_str_delete(output);
}

static void test_json_write_null(CuTest* tc)
{
   const int indent = 0;
   dtl_sv_t *sv = dtl_sv_none();
   CuAssertPtrNotNull(tc, sv);
   adt_str_t *output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "null", adt_str_cstr(output));
   dtl_dec_ref(sv);
   adt_str_delete(output);

   dtl_dv_t *dv_null = dtl_dv_null();
   CuAssertPtrNotNull(tc, dv_null);
   output = dtl_json_dumps(dv_null, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "null", adt_str_cstr(output));
   dtl_dv_delete(dv_null);
   adt_str_delete(output);
}

static void test_json_write_i32(CuTest* tc)
{
   const int indent = 3;
   dtl_sv_t *sv;
   adt_str_t *output;

   sv = dtl_sv_make_i32(-1);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "-1", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_set_i32(sv, 0);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "0", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_set_i32(sv, 1);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "1", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_set_i32(sv, INT32_MIN);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "-2147483648", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_set_i32(sv, INT32_MAX);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "2147483647", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_delete(sv);
}

static void test_json_write_i64(CuTest* tc)
{
   const int indent = 0;
   dtl_sv_t *sv = dtl_sv_make_i64(-2147483649LL);
   CuAssertPtrNotNull(tc, sv);
   adt_str_t *output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "-2147483649", adt_str_cstr(output));
   dtl_sv_delete(sv);
   adt_str_delete(output);

   sv = dtl_sv_make_u64(9999999999ULL);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "9999999999", adt_str_cstr(output));
   dtl_sv_delete(sv);
   adt_str_delete(output);
}

static void test_json_write_i32_list_no_indent(CuTest* tc)
{
   const int indent = 0;
   int32_t i;
   dtl_av_t *av;
   adt_str_t *output;

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 10; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(av, (dtl_dv_t*) sv, false);
   }
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "[0, 1, 2, 3, 4, 5, 6, 7, 8, 9]", adt_str_cstr(output));
   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_i32_list_with_indent(CuTest* tc)
{
   const int indent = 3;
   int32_t i;
   dtl_av_t *av;
   adt_str_t *output;
   const char *expected = "[\n"
         "   0,\n"
         "   1,\n"
         "   2,\n"
         "   3,\n"
         "   4,\n"
         "   5,\n"
         "   6,\n"
         "   7,\n"
         "   8,\n"
         "   9\n"
         "]";

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 10; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(av, (dtl_dv_t*) sv, false);
   }
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_i32_list_inside_list_with_indent(CuTest* tc)
{
   const int indent = 3;
   int32_t i;
   dtl_av_t *av;
   dtl_av_t *inner_list1;
   dtl_av_t *inner_list2;
   dtl_av_t *inner_list3;
   dtl_av_t *inner_list4;
   adt_str_t *output;
   const char *expected = "[\n"
         "   [\n"
         "      0,\n"
         "      1,\n"
         "      2\n"
         "   ],\n"
         "   [\n"
         "      3,\n"
         "      4,\n"
         "      5\n"
         "   ],\n"
         "   [\n"
         "      6,\n"
         "      7,\n"
         "      [\n"
         "         8,\n"
         "         9\n"
         "      ]\n"
         "   ]\n"
         "]";

   inner_list1 = dtl_av_new();
   inner_list2 = dtl_av_new();
   inner_list3 = dtl_av_new();
   inner_list4 = dtl_av_new();
   av = dtl_av_new();
   CuAssertPtrNotNull(tc, inner_list1);
   CuAssertPtrNotNull(tc, inner_list2);
   CuAssertPtrNotNull(tc, inner_list3);
   CuAssertPtrNotNull(tc, inner_list4);
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 3; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list1, (dtl_dv_t*) sv, false);
   }
   for (i = 3; i < 6; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list2, (dtl_dv_t*) sv, false);
   }
   for (i = 8; i < 10; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list4, (dtl_dv_t*) sv, false);
   }
   dtl_av_push(inner_list3, (dtl_dv_t*) dtl_sv_make_i32(6), false);
   dtl_av_push(inner_list3, (dtl_dv_t*) dtl_sv_make_i32(7), false);
   dtl_av_push(inner_list3, (dtl_dv_t*) inner_list4, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list1, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list2, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list3, false);
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_i32_list_inside_list_no_indent(CuTest* tc)
{
   const int indent = 0;
   int32_t i;
   dtl_av_t *av;
   dtl_av_t *inner_list1;
   dtl_av_t *inner_list2;
   dtl_av_t *inner_list3;
   dtl_av_t *inner_list4;
   adt_str_t *output;
   const char *expected = "[[0, 1, 2], [3, 4, 5], [6, 7, [8, 9]]]";

   inner_list1 = dtl_av_new();
   inner_list2 = dtl_av_new();
   inner_list3 = dtl_av_new();
   inner_list4 = dtl_av_new();
   av = dtl_av_new();
   CuAssertPtrNotNull(tc, inner_list1);
   CuAssertPtrNotNull(tc, inner_list2);
   CuAssertPtrNotNull(tc, inner_list3);
   CuAssertPtrNotNull(tc, inner_list4);
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 3; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list1, (dtl_dv_t*) sv, false);
   }
   for (i = 3; i < 6; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list2, (dtl_dv_t*) sv, false);
   }
   for (i = 8; i < 10; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_i32(i);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(inner_list4, (dtl_dv_t*) sv, false);
   }
   dtl_av_push(inner_list3, (dtl_dv_t*) dtl_sv_make_i32(6), false);
   dtl_av_push(inner_list3, (dtl_dv_t*) dtl_sv_make_i32(7), false);
   dtl_av_push(inner_list3, (dtl_dv_t*) inner_list4, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list1, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list2, false);
   dtl_av_push(av, (dtl_dv_t*) inner_list3, false);
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_u32(CuTest* tc)
{
   const int indent = 0;
   dtl_sv_t *sv;
   adt_str_t *output;

   sv = dtl_sv_make_u32(0);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "0", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_sv_set_u32(sv, UINT32_MAX);
   CuAssertPtrNotNull(tc, sv);
   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "4294967295", adt_str_cstr(output));
   adt_str_delete(output);

   dtl_dec_ref(sv);
}

static void test_json_write_u32_list_no_indent(CuTest* tc)
{
   const int indent = 0;
   int32_t i;
   dtl_av_t *av;
   adt_str_t *output;

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 5; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_u32(UINT32_MAX);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(av, (dtl_dv_t*) sv, false);
   }
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "[4294967295, 4294967295, 4294967295, 4294967295, 4294967295]", adt_str_cstr(output));
   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_u32_list_with_indent(CuTest* tc)
{
   const int indent = 3;
   int32_t i;
   dtl_av_t *av;
   adt_str_t *output;

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   for (i = 0; i < 5; i++)
   {
      dtl_sv_t *sv = dtl_sv_make_u32(UINT32_MAX);
      CuAssertPtrNotNull(tc, sv);
      dtl_av_push(av, (dtl_dv_t*) sv, false);
   }
   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, "[\n"
         "   4294967295,\n"
         "   4294967295,\n"
         "   4294967295,\n"
         "   4294967295,\n"
         "   4294967295\n"
         "]", adt_str_cstr(output));
   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_small_object_no_indent(CuTest* tc)
{
   const int indent = 0;
   dtl_hv_t *hv;
   adt_str_t *output;
   const char *expected = "{\"logging-enable\": true, \"port\": 5000}";

   hv = dtl_hv_new();
   CuAssertPtrNotNull(tc, hv);
   dtl_hv_set_cstr(hv, "port", (dtl_dv_t*) dtl_sv_make_i32(5000), false);
   dtl_hv_set_cstr(hv, "logging-enable", (dtl_dv_t*) dtl_sv_make_bool(true), false);
   output = dtl_json_dumps((dtl_dv_t*) hv, indent, true);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_hv_delete(hv);
   adt_str_delete(output);
}

static void test_json_write_small_object_with_indent(CuTest* tc)
{
   const int indent = 3;
   dtl_hv_t *hv;
   adt_str_t *output;
   const char *expected = "{\n"
         "   \"logging-enable\": true,\n"
         "   \"port\": 5000\n"
         "}";

   hv = dtl_hv_new();
   CuAssertPtrNotNull(tc, hv);
   dtl_hv_set_cstr(hv, "port", (dtl_dv_t*) dtl_sv_make_i32(5000), false);
   dtl_hv_set_cstr(hv, "logging-enable", (dtl_dv_t*) dtl_sv_make_bool(true), false);
   output = dtl_json_dumps((dtl_dv_t*) hv, indent, true);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_hv_delete(hv);
   adt_str_delete(output);
}

static void test_json_write_small_objects_with_indent(CuTest* tc)
{
   const int indent = 3;
   dtl_hv_t *hv;
   dtl_hv_t *global;
   dtl_hv_t *server;
   adt_str_t *output;
   const char *expected = "{\n"
         "   \"Global\": {\n"
         "      \"KeepAliveTimeout\": 15,\n"
         "      \"MaxClients\": 512,\n"
         "      \"Timeout\": 300\n"
         "   },\n"
         "   \"ServerConfig\": {\n"
         "      \"HostNameLookupEnable\": false,\n"
         "      \"Port\": 80\n"
         "   }\n"
         "}";

   hv = dtl_hv_new();
   global = dtl_hv_new();
   server = dtl_hv_new();
   CuAssertPtrNotNull(tc, hv);
   CuAssertPtrNotNull(tc, global);
   CuAssertPtrNotNull(tc, server);
   dtl_hv_set_cstr(global, "Timeout", (dtl_dv_t*) dtl_sv_make_i32(300), false);
   dtl_hv_set_cstr(global, "MaxClients", (dtl_dv_t*) dtl_sv_make_i32(512), false);
   dtl_hv_set_cstr(global, "KeepAliveTimeout", (dtl_dv_t*) dtl_sv_make_i32(15), false);
   dtl_hv_set_cstr(server, "Port", (dtl_dv_t*) dtl_sv_make_i32(80), false);
   dtl_hv_set_cstr(server, "HostNameLookupEnable", (dtl_dv_t*) dtl_sv_make_bool(false), false);
   dtl_hv_set_cstr(hv, "Global", (dtl_dv_t*) global, false);
   dtl_hv_set_cstr(hv, "ServerConfig", (dtl_dv_t*) server, false);
   output = dtl_json_dumps((dtl_dv_t*) hv, indent, true);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_hv_delete(hv);
   adt_str_delete(output);
}

static void test_json_write_string(CuTest* tc)
{
   const int indent = 0;
   dtl_sv_t *sv;
   adt_str_t *output;
   const char *expected = "\"String Test\"";

   sv = dtl_sv_make_cstr("String Test");
   CuAssertPtrNotNull(tc, sv);

   output = dtl_json_dumps((dtl_dv_t*) sv, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_sv_delete(sv);
   adt_str_delete(output);
}

static void test_json_write_string_list_no_indent(CuTest* tc)
{
   const int indent = 0;
   dtl_av_t *av;
   adt_str_t *output;
   const char *expected = "[\"Test 1\", \"Test 2\", \"Test 3\"]";

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 1"), false);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 2"), false);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 3"), false);

   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_string_list_with_indent(CuTest* tc)
{
   const int indent = 3;
   dtl_av_t *av;
   adt_str_t *output;
   const char *expected = "[\n"
         "   \"Test 1\",\n"
         "   \"Test 2\",\n"
         "   \"Test 3\"\n"
         "]";

   av = dtl_av_new();
   CuAssertPtrNotNull(tc, av);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 1"), false);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 2"), false);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_cstr("Test 3"), false);

   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_write_utf8_string_list_with_indent(CuTest* tc)
{
   const int indent = 3;
   dtl_av_t *av;
   adt_str_t *output;
   adt_str_t *str1;
   adt_str_t *str2;
   const char *expected = "[\n"
         "   \"\343\201\212\343\201\257\343\202\210\343\201\206\343\201\224\343\201\226\343\201\204\343\201\276\343\201\231\343\200\202\",\n"
         "   \"\343\201\223\343\202\223\343\201\260\343\202\223\343\201\257\343\200\202\"\n"
         "]";

   av = dtl_av_new();
   str1 = adt_str_new_cstr("\343\201\212\343\201\257\343\202\210\343\201\206\343\201\224\343\201\226\343\201\204\343\201\276\343\201\231\343\200\202");
   str2 = adt_str_new_cstr("\343\201\223\343\202\223\343\201\260\343\202\223\343\201\257\343\200\202");
   CuAssertIntEquals(tc, 10, adt_str_length(str1));
   CuAssertIntEquals(tc, 6, adt_str_length(str2));
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_str(str1), false);
   dtl_av_push(av, (dtl_dv_t*) dtl_sv_make_str(str2), false);
   adt_str_delete(str1);
   adt_str_delete(str2);

   output = dtl_json_dumps((dtl_dv_t*) av, indent, false);
   CuAssertPtrNotNull(tc, output);
   CuAssertStrEquals(tc, expected, adt_str_cstr(output));

   dtl_av_delete(av);
   adt_str_delete(output);
}

static void test_json_dump_file(CuTest* tc)
{
   const char *filename = "test_dump_file.json";
   FILE *fh = fopen(filename, "w");
   CuAssertPtrNotNull(tc, fh);

   dtl_hv_t *hv = dtl_hv_new();
   dtl_hv_set_cstr(hv, "name", (dtl_dv_t*) dtl_sv_make_cstr("dump_test"), false);
   dtl_hv_set_cstr(hv, "count", (dtl_dv_t*) dtl_sv_make_i32(10), false);

   dtl_error_t res = dtl_json_dump((dtl_dv_t*) hv, fh, 0, true);
   CuAssertIntEquals(tc, DTL_NO_ERROR, res);
   fclose(fh);
   dtl_dec_ref(hv);

   fh = fopen(filename, "r");
   CuAssertPtrNotNull(tc, fh);
   dtl_dv_t *loaded = dtl_json_load(fh);
   fclose(fh);
   remove(filename);

   CuAssertPtrNotNull(tc, loaded);
   CuAssertIntEquals(tc, DTL_DV_HASH, dtl_dv_type(loaded));
   dtl_hv_t *loaded_hv = (dtl_hv_t*) loaded;
   dtl_sv_t *name = (dtl_sv_t*) dtl_hv_get_cstr(loaded_hv, "name");
   CuAssertPtrNotNull(tc, name);
   CuAssertStrEquals(tc, "dump_test", dtl_sv_to_cstr(name, NULL));

   dtl_sv_t *count = (dtl_sv_t*) dtl_hv_get_cstr(loaded_hv, "count");
   CuAssertPtrNotNull(tc, count);
   CuAssertIntEquals(tc, 10, dtl_sv_to_i32(count, NULL));

   dtl_dec_ref(loaded);
}

static void test_json_write_null_args(CuTest* tc)
{
   CuAssertPtrEquals(tc, NULL, dtl_json_dumps(NULL, 0, false));
   CuAssertTrue(tc, dtl_json_dump(NULL, NULL, 0, false) != DTL_NO_ERROR);
}
