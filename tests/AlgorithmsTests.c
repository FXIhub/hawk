#include "spimage.h"
#include "../configuration.h"
#include "../algorithms.h"
#include "../log.h"
#include "AllTests.h"
#include "5.1.09_crop25.h"

real image_rel_error(Image * a, Image * b){
  real error = 0;
  real den = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    real diff = sp_real(a->image->data[i])-sp_real(b->image->data[i]);
    error += diff*diff;
    diff = sp_imag(a->image->data[i])-sp_imag(b->image->data[i]);
    error += diff*diff;
    den += sp_cabs2(a->image->data[i]);
  }
  return error/den;
}

void test_hio(CuTest* tc){
  /* Read the input test image */
  sp_srand(time(NULL));
  Image * a = sp_image_alloc(standard_5_1_09_crop25[0],standard_5_1_09_crop25[1],1);
  Image * s = sp_image_alloc(standard_5_1_09_crop25[0],standard_5_1_09_crop25[1],1);
  for(int i = 0;i<sp_image_size(a);i++){
    sp_real(a->image->data[i]) = standard_5_1_09_crop25[i+3];
    sp_real(s->image->data[i]) = 1;
  }
  Image * real = sp_image_edge_extend(a,sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  Image * support = sp_image_edge_extend(s,sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  Image * amp = sp_image_fft(real);
  sp_image_write(sp_image_ifft(amp),"amp_ifft.png",COLOR_GRAYSCALE);
  Image * exp_sigma = sp_image_alloc(sp_image_x(amp),sp_image_y(amp),1);
  for(int i = 0;i<sp_image_size(amp);i++){
    amp->mask->data[i] = 1;
    sp_real(exp_sigma->image->data[i]) = 0;
  }
  sp_image_dephase(amp);
  sp_image_rephase(amp,SP_RANDOM_PHASE);
  Image * real_in = sp_image_ifft(amp);
  sp_image_dephase(amp);
  sp_image_rephase(amp,SP_ZERO_PHASE);

  sp_image_write(amp,"amp.png",LOG_SCALE|COLOR_GRAYSCALE);
  sp_image_scale(real_in,1.0/sp_image_size(real_in));
  Options * opts = malloc(sizeof(Options));
  opts->beta = 1;
  opts->log_output_period = 1e6;
  Image * real_out;
  printf("error = %f\n",image_rel_error(real,real_in));
  sp_image_write(real,"real.png",COLOR_GRAYSCALE);
  sp_image_write(real_in,"real_in.png",COLOR_GRAYSCALE);
  double min_error = 1000;
  for(int i = 0;i<2000;i++){
    //    real_out = basic_hio_iteration(amp,real_in,support,opts,NULL);
    real_out = basic_raar_iteration(amp,exp_sigma,real_in,support,opts,NULL);
    sp_image_free(real_in);
    real_in = real_out;
    printf("error = %f\n",image_rel_error(real,real_out));
    float error = image_rel_error(real,real_out);
    if(error < min_error){
      sp_image_write(real_out,"real_out.png",COLOR_GRAYSCALE);
      min_error = error;
    }
  }
}

CuSuite* algorithms_get_suite(void)
{
  CuSuite* suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, test_hio);
  return suite;
}

