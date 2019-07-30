#include <stdio.h>
#include <stdlib.h>
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif


CuSuite* testsuite_dtl_json_writer(void);
CuSuite* testsuite_dtl_json_reader(void);

void RunAllTests(void)
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
   RunAllTests();
   return 0;
}

void vfree(void *arg)
{
   free(arg);
}
