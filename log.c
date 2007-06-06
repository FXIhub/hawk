#include <time.h>

#include "spimage.h"
#include "configuration.h"
#include "log.h"
#include "algorithms.h"

void output_to_log(Image * exp_amp,Image * real_in, Image * real_out, Image * fourier_out,Image * support, Options * opts,Log * log){
  int it_outer;
  time_t t;
  real Ereal = 0;
  real Ereal_den = 0;
  real Efourier = 0;
  real Efourier_den = 0;
  real FcFo = 0;
  int FcFo_den = 0;
  real SupSize = 0;
  int i;
  int iter = opts->cur_iteration;
  static Image * previous_out = NULL;
/*  char buffer[1024];*/

  if(!log->cumulative_fluctuation){
    log->cumulative_fluctuation = sp_image_duplicate(real_in,SP_COPY_DETECTOR);
  }
  if(iter % opts->iterations == opts->iterations/2){
    if(!previous_out){
      previous_out = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
    }else{
      log->int_cum_fluctuation = 0;
      for(i = 0;i<sp_cmatrix_size(real_in->image);i++){
	/*      log->cumulative_fluctuation->image->data[i] += fabs(real_in->image->data[i] - real_out->image->data[i]);*/
	log->int_cum_fluctuation += fabs(previous_out->image->data[i] - real_out->image->data[i]);    
      }
      /*    sprintf(buffer,"cum_fluct-%06d.png",iter);
	    sp_image_write(log->cumulative_fluctuation,buffer,sizeof(real));*/
      sp_image_free(previous_out);     
      previous_out = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
    }
  }

  if(opts->iterations){
    it_outer = (iter)/opts->iterations;
  }else{
    it_outer = 0;
  }
  if(!opts->flog){
    opts->flog = fopen(opts->log_file,"w");
    fprintf(opts->flog,"# Uwrapc version %s compiled on %s at %s\n",VERSION,__DATE__,__TIME__);
    time(&t);
    
    fprintf(opts->flog,"# Date: %s\n",ctime(&t));
    fprintf(opts->flog,"# Command line:\n# %s\n#\n",opts->commandline);
    fprintf(opts->flog,"# It(in) - Number of iterations of the inner loop\n");
    fprintf(opts->flog,"# It(out) - Number of iterations of the outer loop\n");
    fprintf(opts->flog,"# Ereal - Real Space error. Eq 16 in Ref 1\n");
    fprintf(opts->flog,"# Efourier - Fourier Space error. Eq 17 in Ref 1.\n"
	    "#\t   Basically Xray R factor\n");
    fprintf(opts->flog,"# <Fc/Fo> - Simply the average of calculated\n"
	    "#\t  amplitude divided by experimental one\n");
    fprintf(opts->flog,"# SupSize - Support size in relation to image size (%%)\n");
    fprintf(opts->flog,"# Ref 1.  High-resolution ab initio three-dimensional\n"
	    "#\tX-ray diffraction microscopy\n\n");
    
    fprintf(opts->flog,"#It(in)\tIt(out)\tEreal\t\tEfourier\t<Fc/Fo>\t\tSupSize\n");        

    fprintf(opts->flog,"@TYPE xy\n");
    fprintf(opts->flog,"@ view 0.15, 0.15, 0.75, 0.8\n");
    fprintf(opts->flog,"@ legend on\n");
    fprintf(opts->flog,"@ legend box on\n");
    fprintf(opts->flog,"@ legend loctype view\n");
    fprintf(opts->flog,"@ legend 0.78, 0.8\n");
    fprintf(opts->flog,"@ legend length 2\n");
    fprintf(opts->flog,"@ s0 legend \"It(out) \\N\"\n");
/*    fprintf(log,"@ s0 legend \"It(in) \\N\"\n");*/
    fprintf(opts->flog,"@ s1 legend \"Ereal \\N\"\n");
    fprintf(opts->flog,"@ s2 legend \"Efourier \\N\"\n");
    fprintf(opts->flog,"@ s3 legend \"<Fc/Fo> \\N\"\n");
    fprintf(opts->flog,"@ s4 legend \"SupSize(%%) \\N\"\n");
    fprintf(opts->flog,"@ s5 legend \"Beta \\N\"\n");
    fprintf(opts->flog,"@ s6 legend \"Threshold \\N\"\n");
    fprintf(opts->flog,"@ s7 legend \"Algorithm \\N\"\n");
    fprintf(opts->flog,"@ s8 legend \"dEreal \\N\"\n");
    fprintf(opts->flog,"@ s9 legend \"dSupSize \\N\"\n");
    fprintf(opts->flog,"@ s10 legend \"Blur Radius \\N\"\n");
    fprintf(opts->flog,"@ s11 legend \"Int Cum Fluct \\N\"\n");
    fprintf(opts->flog,"@ s12 legend \"Object Area \\N\"\n");

  }
  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(support->image->data[i]){
      SupSize++;
      Ereal_den += cabsr(real_out->image->data[i])*cabsr(real_out->image->data[i]);
    }else{
      Ereal += cabsr(real_out->image->data[i])*cabsr(real_out->image->data[i]);
    }
    if(exp_amp->image->data[i] && exp_amp->mask->data[i]){
      Efourier += (cabsr(fourier_out->image->data[i])-cabsr(exp_amp->image->data[i]))*(cabsr(fourier_out->image->data[i])-cabsr(exp_amp->image->data[i]));
      Efourier_den += cabsr(exp_amp->image->data[i])*cabsr(exp_amp->image->data[i]);
      FcFo += (cabsr(fourier_out->image->data[i])/cabsr(exp_amp->image->data[i]));
      FcFo_den++;
    }
  }  
  Ereal /= Ereal_den;
  Efourier /= Efourier_den;
  FcFo /= FcFo_den;
  SupSize /= sp_cmatrix_size(exp_amp->image);
  SupSize *= 100;
  log->iter = iter;
  log->it_outer = it_outer;
  log->Ereal = Ereal;
  log->Efourier = Efourier;
  log->FcFo = FcFo;
  log->SupSize = SupSize;
  log->Ereal_list[(opts->cur_iteration/opts->log_output_period)%RUN_AVG_LEN] = Ereal;
  log->SupSize_list[(opts->cur_iteration/opts->log_output_period)%RUN_AVG_LEN] = SupSize;
  log->dEreal = -log->Ereal_run_avg;
  log->dSupSize = -log->SupSize_run_avg;
  log->Ereal_run_avg = 0;
  log->SupSize_run_avg = 0;
  for(i = 0;i<MIN((opts->cur_iteration/opts->log_output_period),RUN_AVG_LEN);i++){
    log->Ereal_run_avg += log->Ereal_list[i];
    log->SupSize_run_avg += log->SupSize_list[i];
  }
  if(i){
    log->Ereal_run_avg /= i;
    log->SupSize_run_avg /= i;
  }

  log->dEreal += log->Ereal_run_avg;
  log->dSupSize += log->SupSize_run_avg;
  fprintf(opts->flog,"%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%f\t%f\t%f\t%f\t%f\n",iter,it_outer,
	  Ereal,Efourier,FcFo,SupSize,get_beta(opts),log->threshold,get_algorithm(opts,log),
	  log->dEreal,log->dSupSize,get_blur_radius(opts),log->int_cum_fluctuation,get_object_area(opts));
  fflush(opts->flog);

  if(Ereal < opts->real_error_tolerance){
    opts->reconstruction_finished = 1;
  }
}  
  
     

void init_log(Log * log){
  int i;
  log->dEreal = 1;
  log->Ereal = 1;
  log->dSupSize = -1;
  log->SupSize = 1;
  log->threshold = (real)0.20;
  log->Ereal_run_avg = 0;
  log->SupSize_run_avg = 0;
  for(i = 0;i<RUN_AVG_LEN;i++){
    log->Ereal_list[i] = 0;
    log->SupSize_list[i] = 0;
  }
  log->cumulative_fluctuation = NULL;
  log->int_cum_fluctuation = 0;
}


