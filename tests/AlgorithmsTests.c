#include <time.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include "spimage.h"
#include "../configuration.h"
#include "../algorithms.h"
#include "../log.h"
#include "AllTests.h"
#include "5.1.09_crop25.h"
#include "4.2.04_scale25.h"

static gsl_rng * r = NULL;

Image * generate_from_poisson(Image * in,gsl_rng * r){
  Image * out = sp_image_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));
  for(int i = 0;i<sp_image_size(in);i++){
    if(sp_real(in->image->data[i]) != 0){
      sp_real(out->image->data[i]) = gsl_ran_poisson(r,sp_real(in->image->data[i]));
    }else{
      sp_real(out->image->data[i]) = 0;
    }
    printf("%d\n",i);
  }
  return out;
}

real sp_image_complex_correlation(Image * a, Image * b){
  Complex * x = a->image->data;
  Complex * y = b->image->data;
  int N = sp_image_size(a);
  double sum_sq_x = 0;
  double sum_sq_y = 0;
  double  sum_coproduct = 0;
  double mean_x = sp_real(x[1]);
  double mean_y = sp_real(y[1]);
  for(int i= 1;i<sp_image_size(a);i++){
    double sweep = (i - 1.0) / i;
    double delta_x = sp_cabs(x[i]) - mean_x;
    double delta_y = sp_cabs(y[i]) - mean_y;
    sum_sq_x += delta_x * delta_x * sweep;
    sum_sq_y += delta_y * delta_y * sweep;
    sum_coproduct += delta_x * delta_y * sweep;
    mean_x += delta_x / i;
    mean_y += delta_y / i;
  } 
  double pop_sd_x = sqrt( sum_sq_x / N);
  double pop_sd_y = sqrt( sum_sq_y / N );
  double cov_x_y = sum_coproduct / N;
  double correlation = cov_x_y / (pop_sd_x * pop_sd_y);
  return correlation;
}

real sp_image_entiomorph_correlation(Image * a, Image * b){
  double corr1 = sp_image_complex_correlation(a,b);
  Image * tmp = sp_image_duplicate(a,SP_COPY_DATA);
  int size = sp_image_size(tmp);
  for(int i = 0;i<sp_image_size(tmp);i++){
    tmp->image->data[i] = b->image->data[size-i-1];
  }
  double corr2 = sp_image_complex_correlation(a,tmp);
  if(corr1 < corr2){
    return corr2;
  }
  return corr1;
}

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

float test_raar_success(float criteria, float * pgm,int runs, int iter_per_run,float oversampling){
  int success = 0;
  Image * a = sp_image_alloc(pgm[0],pgm[1],1);
  Image * s = sp_image_alloc(pgm[0],pgm[1],1);
  for(int i = 0;i<sp_image_size(a);i++){
    sp_real(a->image->data[i]) = pgm[i+3];
    sp_real(s->image->data[i]) = 1;
  }
  Image * real =  sp_image_edge_extend(a,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  sp_image_write(real,"real.png",COLOR_GRAYSCALE);
  Image * support = sp_image_edge_extend(s,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  Image * amp = sp_image_fft(real);
  sp_image_write(amp,"amp.png",LOG_SCALE|COLOR_GRAYSCALE);
  for(int i = 0;i<sp_image_size(amp);i++){
    amp->mask->data[i] = 1;
  }
  Options * opts = set_defaults();
  opts->beta = 0.9;
  opts->log_output_period = 1e6;
  for(int i = 0;i<runs;i++){
    sp_image_dephase(amp);
    sp_image_rephase(amp,SP_RANDOM_PHASE);
    Image * real_in = sp_image_ifft(amp);
    sp_image_scale(real_in,1.0/sp_image_size(real_in));
    sp_image_dephase(amp);
    sp_image_rephase(amp,SP_ZERO_PHASE);
    Image * real_out;
    for(int j = 0;j<iter_per_run;j++){
      real_out = serial_raar_iteration(amp,real_in,support,opts,NULL);
      sp_image_free(real_in);
      real_in = real_out;
    }
    float score = sp_image_entiomorph_correlation(real,real_out);
    //    printf("Final correlation = %f\n",score);
    //    sp_image_write(real_out,"real_out.png",COLOR_GRAYSCALE);
    if(score > criteria){
      success++;
    }
    sp_image_free(real_in);
  }
  sp_image_free(a);
  sp_image_free(s);
  sp_image_free(real);
  sp_image_free(support);
  sp_image_free(amp);
  return (float)success/runs;
}

float test_difference_map_run(Image * real, Image * support, Image * amp, Options * opts, int iter_per_run){
  sp_image_dephase(amp);
  sp_image_rephase(amp,SP_RANDOM_PHASE);
  Image * real_in = sp_image_ifft(amp);
  sp_image_scale(real_in,1.0/sp_image_size(real_in));
  sp_image_dephase(amp);
  sp_image_rephase(amp,SP_ZERO_PHASE);
  Image * real_out;
  for(int j = 0;j<iter_per_run;j++){
    real_out = serial_difference_map_iteration(amp,NULL,real_in,support,opts,NULL);
    sp_image_free(real_in);
    real_in = real_out;
  }
  float score = sp_image_entiomorph_correlation(real,real_out);
  //  printf("Final correlation = %f\n",score);
  //  sp_image_write(real_out,"real_out.png",COLOR_GRAYSCALE);
  sp_image_free(real_in);
  return score;
}

float test_difference_map_success(float criteria, float * pgm,int runs, int iter_per_run,float oversampling){
  int success = 0;
  Image * a = sp_image_alloc(pgm[0],pgm[1],1);
  Image * s = sp_image_alloc(pgm[0],pgm[1],1);
  for(int i = 0;i<sp_image_size(a);i++){
    sp_real(a->image->data[i]) = pgm[i+3];
    sp_real(s->image->data[i]) = 1;
  }
  Image * real =  sp_image_edge_extend(a,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  sp_image_write(real,"real.png",COLOR_GRAYSCALE);
  Image * support = sp_image_edge_extend(s,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  Image * amp = sp_image_fft(real);
  for(int i = 0;i<sp_image_size(amp);i++){
    amp->mask->data[i] = 1;
  }
  Options * opts = set_defaults();
  float sigma = 1.0/9;
  opts->beta = 0.9;
  float beta = opts->beta;
    opts->gamma1 =  -(4+(2+beta)*sigma + beta*sigma*sigma)/(beta*(4-sigma+sigma*sigma));
    opts->gamma2 = (3-beta)/(2*beta);
    opts->gamma1 = -1/beta;
    //    opts->gamma2 = 1/beta;
  //try to reproduce HIO Success!
  //  opts->gamma1 = -1;
  //  opts->gamma2 = 1/beta;
  opts->log_output_period = 1e6;
  for(int i = 0;i<runs;i++){
    float score = test_difference_map_run(real,support,amp,opts,iter_per_run);
    if(score > criteria){
      success++;
    }
  }
  sp_image_free(a);
  sp_image_free(s);
  sp_image_free(real);
  sp_image_free(support);
  sp_image_free(amp);
  return (float)success/runs;
}

float test_hio_success(float criteria, float * pgm,int runs, int iter_per_run,float oversampling, real photons_per_pixel){
  int success = 0;
  Image * a = sp_image_alloc(pgm[0],pgm[1],1);
  Image * s = sp_image_alloc(pgm[0],pgm[1],1);
  for(int i = 0;i<sp_image_size(a);i++){
    sp_real(a->image->data[i]) = pgm[i+3];
    sp_real(s->image->data[i]) = 1;
  }
  /* Scale image to the required number of photons per pixel */
  real ppp = sp_real(sp_image_integrate(a))/sp_image_size(a);
  sp_image_scale(a,photons_per_pixel/ppp);

  Image * real =  sp_image_edge_extend(a,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);
  sp_image_write(real,"real.png",COLOR_GRAYSCALE);
  Image * support = sp_image_edge_extend(s,oversampling*sp_image_x(a),SP_ZERO_PAD_EDGE,SP_2D);

  fprintf(stderr,"Generating sample...");
  Image * sample = generate_from_poisson(real,r);
  fprintf(stderr,"done");
  Image * amp = sp_image_fft(sample);
  sp_image_free(sample);

  for(int i = 0;i<sp_image_size(amp);i++){
    amp->mask->data[i] = 1;
  }
  Options * opts = set_defaults();
  opts->beta = 0.9;
  opts->log_output_period = 1e6;
  for(int i = 0;i<runs;i++){
    sp_image_dephase(amp);
    sp_image_rephase(amp,SP_RANDOM_PHASE);
    Image * real_in = sp_image_ifft(amp);
    sp_image_scale(real_in,1.0/sp_image_size(real_in));
    sp_image_dephase(amp);
    sp_image_rephase(amp,SP_ZERO_PHASE);
    Image * real_out;
    for(int j = 0;j<iter_per_run;j++){
      real_out = basic_hio_iteration(amp,real_in,support,opts,NULL);
      sp_image_free(real_in);
      real_in = real_out;
    }
    float score = sp_image_entiomorph_correlation(real,real_out);
           printf("Final correlation = %f\n",score);
    //        sp_image_write(real_out,"real_out.png",COLOR_GRAYSCALE);
    if(score > criteria){
      success++;
    }
    sp_image_free(real_in);
  }
  sp_image_free(a);
  sp_image_free(s);
  sp_image_free(real);
  sp_image_free(support);
  sp_image_free(amp);
  return (float)success/runs;
}

void test_hio(CuTest* tc){
  int iter = 200;
  float criteria = 0.99;
  float rate;
  real photons_per_pixel = 1000;
  rate = test_hio_success(criteria, standard_4_2_04_scale25,100,iter,1,photons_per_pixel);
  printf("4_2_04_scale25 HIO success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);
  rate = test_hio_success(criteria, standard_5_1_09_crop25,100,iter,1,photons_per_pixel);
  printf("5_1_09_crop25 HIO success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);
}

void test_raar(CuTest* tc){
  int iter = 200;
  float criteria = 0.99;
  float rate = 0;
  rate = test_raar_success(criteria, standard_4_2_04_scale25,100,iter,1);
  printf("4_2_04_scale25 RAAR success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);
  rate = test_raar_success(criteria, standard_5_1_09_crop25,100,iter,1);
  printf("5_1_09_crop25 RAAR success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);

}

void test_difference_map(CuTest* tc){
  int iter = 200;
  float criteria = 0.99;
  float rate;
  int runs = 100;
  rate = test_difference_map_success(criteria, standard_4_2_04_scale25,runs,iter,1);
  printf("4_2_04_scale25 DIFF MAP success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);
  rate = test_difference_map_success(criteria, standard_5_1_09_crop25,runs,iter,1);
  printf("5_1_09_crop25 DIFF MAP success rate with %d iterations and %3.2f criteria = %3.2f\n",iter,criteria,rate);
  CuAssertTrue(tc,rate > 0.1);
}

CuSuite* algorithms_get_suite(void)
{
  r = gsl_rng_alloc (gsl_rng_default);
  gsl_rng_set(r,time(NULL));
  sp_srand(time(NULL));
  CuSuite* suite = CuSuiteNew();
  //  SUITE_ADD_TEST(suite, test_raar);
  SUITE_ADD_TEST(suite, test_hio);
  //  SUITE_ADD_TEST(suite, test_difference_map);
  return suite;
}

