#ifndef _LOG_H_
#define _LOG_H_

#define RUN_AVG_LEN 500

#include "configuration.h"

typedef struct{
  real Ereal;
  real Efourier;
  real FcFo;
  real SupSize;
  int iter;
  int it_outer;
  real Ereal_list[RUN_AVG_LEN];
  real Ereal_run_avg;
  real SupSize_list[RUN_AVG_LEN];
  real SupSize_run_avg;
  real dEreal;
  real dSupSize;
  real threshold;
  Image * cumulative_fluctuation;
  real int_cum_fluctuation;
  real sol_correlation;
}Log;


void init_log(Log * log);
void output_to_log(Image * exp_amp,Image * real_in, Image * real_out, Image * fourier_out,Image * support, Options * opts,Log * log);

#endif
