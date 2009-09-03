#include "AllTests.h"

#define _ALL_TESTS_ 1

int RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	CuSuiteAddSuite(suite, algorithms_get_suite());
	CuSuiteAddSuite(suite, geometry_get_suite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	return suite->failCount;
}



int main(void)
{
  return RunAllTests();
}
