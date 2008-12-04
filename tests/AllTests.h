#ifndef _ALLTESTS_H_
#define _ALLTESTS_H_ 1


#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "spimage.h"

#include "CuTest.h"
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif


#define CuAssertComplexEquals(__tc,___a,___b,__delta) do{\
    Complex __a = ___a;\
    Complex __b = ___b;\
    CuAssertTrue(__tc,fabs(sp_real(sp_csub(__a, __b))) < __delta && fabs(sp_imag(sp_csub(__a, __b))) < __delta);\
  }while(0)


CuSuite* algorithms_get_suite();

#endif
