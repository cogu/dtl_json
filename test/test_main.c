/*****************************************************************************
* \file      test_main.c
* \author    Conny Gustafsson
* \date      2019-07-02
* \brief     Unit test entry point for dtl_json
*
* Copyright (c) 2019-2026 Conny Gustafsson
* SPDX-License-Identifier: MIT
* See LICENSE in project root for full license terms.
******************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

CuSuite* testsuite_dtl_json_writer(void);
CuSuite* testsuite_dtl_json_reader(void);

static void run_all_tests(void)
{
   CuString *output = CuStringNew();
   CuSuite* suite = CuSuiteNew();

   CuSuiteAddSuite(suite, testsuite_dtl_json_writer());
   CuSuiteAddSuite(suite, testsuite_dtl_json_reader());

   CuSuiteRun(suite);
   CuSuiteSummary(suite, output);
   CuSuiteDetails(suite, output);
   printf("%s\n", output->buffer);
   CuSuiteDelete(suite);
   CuStringDelete(output);
}

int main(void)
{
   run_all_tests();
   return 0;
}

#ifdef MEM_LEAK_CHECK
void vfree(void *arg)
{
   free(arg);
}
#endif
