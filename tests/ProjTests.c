
#include "AllTests.h"

void test_sp_proj_module(CuTest* tc)
{
  int size = 2;
  Image * a = sp_image_alloc(size,size,1);
  for(int i = 0;i<sp_image_size(a);i++){
    a->image->data[i] = sp_cinit(p_drand48(),p_drand48());
  }
  Image * exp = sp_image_alloc(size,size,1);
  for(int i = 0;i<sp_image_size(a);i++){
    exp->image->data[i] = sp_cinit(p_drand48(),0);
  }

  for(int i = 0;i<sp_image_size(a);i++){
    exp->mask->data[i] = 1;
  }


  Image * proj = sp_proj_module(a,exp);

  for(int i = 0;i<sp_image_size(a);i++){
    /* Check that the magnitude is the same as the experimental */
    CuAssertDblEquals(tc,sp_cabs(proj->image->data[i]),sp_cabs(exp->image->data[i]),fabs(REAL_EPSILON*sp_cabs(exp->image->data[i])));
    /* Check that the phase is the same as the input */
    CuAssertDblEquals(tc,sp_carg(proj->image->data[i]),sp_carg(a->image->data[i]),fabs(REAL_EPSILON*sp_carg(a->image->data[i])));
  }

}


void test_sp_proj_module_histogram(CuTest* tc)
{
  int size = 1000;
  float base = 10;
  Image * a = sp_image_alloc(size,size,1);
  for(int i = 0;i<sp_image_size(a);i++){
    a->image->data[i] = sp_cinit(sqrt(base)*p_drand48(),sqrt(base)*p_drand48());
  }
  Image * exp = sp_image_alloc(size,size,1);
  for(int i = 0;i<sp_image_size(a);i++){
    /* We have to add base to make sure the values are much above std_dev */
    exp->image->data[i] = sp_cinit(base+p_drand48(),0);
  }

  for(int i = 0;i<sp_image_size(a);i++){
    exp->mask->data[i] = 1;
  }

  Image * std_dev = sp_image_alloc(size,size,1);
  for(int i = 0;i<sp_image_size(a);i++){
    /* We have to keep the standard deviation small */
    std_dev->image->data[i] = sp_cinit(0.1,0);
  }

  Image * norm_sq_int = sp_image_alloc(size,size,1);

  Image * proj = sp_proj_module_histogram(a,exp,std_dev);
  for(int i = 0;i<sp_image_size(a);i++){
    /* Calculate normalized square intensities */
    sp_real(norm_sq_int->image->data[i]) = (sp_cabs2(exp->image->data[i])-sp_cabs2(proj->image->data[i]))/sp_real(std_dev->image->data[i]);
    /* Check that the phase is the same as the input */
    CuAssertDblEquals(tc,sp_carg(proj->image->data[i]),sp_carg(a->image->data[i]),1e-3);
  }
  /* Check that the average normal of the normaized squared magnitudes is 0*/
  double average = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    average += sp_real(norm_sq_int->image->data[i]);
  }
  average /= sp_image_size(a);  

  double variance = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    variance += (average-sp_real(norm_sq_int->image->data[i]))*(average-sp_real(norm_sq_int->image->data[i]));
  }
  variance /= sp_image_size(a);  
  CuAssertDblEquals(tc,average,0,1.0/size);
  CuAssertDblEquals(tc,variance,1,1.0/size);
}


CuSuite* proj_get_suite(void)
{
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_sp_proj_module);
  SUITE_ADD_TEST(suite, test_sp_proj_module_histogram);
  return suite;
}

