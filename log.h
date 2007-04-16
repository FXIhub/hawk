#ifndef _LOG_H_
#define _LOG_H_

#define RUN_AVG_LEN 500

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
}Log;


void init_log(Log * log);

#endif
