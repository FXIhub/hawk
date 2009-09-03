#include "AllTests.h"

int main(){
  CuString *output = CuStringNew();
  CuSuite* suite = CuSuiteNew();
  CuSuiteAddSuite(suite, geometry_get_suite());
  CuSuiteRun(suite);
  CuSuiteSummary(suite, output);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
  return suite->failCount;  
}
