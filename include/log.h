#ifndef _LOG_H_
#define _LOG_H_

#define RUN_AVG_LEN 500


typedef struct _Log{
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
  real dRho;
  real threshold;
  Image * cumulative_fluctuation;
  real int_cum_fluctuation;
  real sol_correlation;
  int timer;
}Log;

#include "configuration.h"


void init_log(Log * log);
void output_from_phaser(SpPhaser * ph,Options * opts, Log * log);
void output_to_log(const Image * exp_amp,const Image * real_in,const Image * real_out,const Image * fourier_out,const Image * support, Options * opts,Log * log);

#endif
