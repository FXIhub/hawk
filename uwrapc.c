#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
#include <process.h>
#else
#include <sys/types.h>
#endif
#ifdef MPI
#include <mpi.h>
#endif


#include "spimage.h"
#include "uwrapc.h"
#include "configuration.h"
/*#include "ga_phasing.h"*/



/* NOTES:
   
  For some strange reason you cannot take half of the patterson diameter to
  limit the patterson. You need to use something like 1.8.
  
  When you start to see very negative peaks close to your patterson
  (on the outside), it means that the patterson was not big enough.

  
  When the support stops to include the image the algorithm degrades quite
  fast. This might be useful for discovering the edges of an object. 

  If the entire object is inside the support, the solution is stable.
*/


/*  There are bugs on the reseting of the log file structure and 
    options structure */


/* continue a reconstruction on this directory */
/* these will simply set image_guess and initial_support
   to the last real_out and support_ respectively */
void continue_reconstruction(Options * opts){
  
} 

int real_compare(const void * a, const void * b){
  if((*(real *)a) <(*(real *)b)){
    return -1;
  }else if((*(real *)a) == (*(real *)b)){
    return 0;
  }else{
    return 1;
  }
  return -2;
}

void rescale_image(Image * a){
  double sum = 0;
  int i;
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    sum += a->image->data[i];
  }
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    a->image->data[i] /= sum;
  } 
}

void centrosym_break_attempt(Image * a){
  int x,y;
  for(x = 0;x<sp_cmatrix_cols(a->image);x++){
    for(y = 0;y<sp_cmatrix_rows(a->image)-x;y++){
      a->image->data[x*sp_cmatrix_rows(a->image)+y] *= 0.5;
    }
  }  
}

void harmonize_sizes(Options * opts){
  int i;
  Image * exp;
  Image * tmp;
  exp = opts->diffraction;
  tmp = opts->support_mask;
  if(tmp &&
     (sp_cmatrix_cols(tmp->image) != sp_cmatrix_cols(exp->image) ||
      sp_cmatrix_rows(tmp->image) != sp_cmatrix_rows(exp->image))){
    fprintf(stderr,"Rescaling support_mask\n");
    tmp = bilinear_rescale(opts->support_mask,sp_cmatrix_cols(exp->image),sp_cmatrix_rows(exp->image));
    sp_image_free(opts->support_mask);
    opts->support_mask = tmp;
    /* Stop rounding errors in the supports */
    for(i = 0;i<sp_cmatrix_size(tmp->image);i++){
      if(creal(tmp->image->data[i]) < 1e-6){
	tmp->image->data[i] = 0;
      }else{
	tmp->image->data[i] = 1;
      }
    }
  }
  tmp = opts->init_support;
  if(tmp &&
     (sp_cmatrix_cols(tmp->image) != sp_cmatrix_cols(exp->image) ||
      sp_cmatrix_rows(tmp->image) != sp_cmatrix_rows(exp->image))){
    fprintf(stderr,"Rescaling init_support\n");
    tmp = bilinear_rescale(opts->init_support,sp_cmatrix_cols(exp->image),sp_cmatrix_rows(exp->image));
    sp_image_free(opts->init_support);
    opts->init_support = tmp;
    /* Stop rounding errors in the supports */
    for(i = 0;i<sp_cmatrix_size(tmp->image);i++){
      if(creal(tmp->image->data[i]) < 1e-1){
	tmp->image->data[i] = 0;
      }else{
	tmp->image->data[i] = 1;
      }
    }
  }
  tmp = opts->image_guess;
  if(tmp &&
     (sp_cmatrix_cols(tmp->image) != sp_cmatrix_cols(exp->image) ||
      sp_cmatrix_rows(tmp->image) != sp_cmatrix_rows(exp->image))){
    fprintf(stderr,"Rescaling image_guess\n");
    tmp = fourier_rescale(opts->image_guess,sp_cmatrix_cols(exp->image),sp_cmatrix_rows(exp->image));
    sp_image_free(opts->image_guess);
    opts->image_guess = tmp;
    sp_image_write(tmp,"rescaled_guess.png",COLOR_JET);
  }
}

real get_blur_radius(Options * opts){
  real a;
  if(opts->blur_radius_reduction_method == GAUSSIAN_BLUR_REDUCTION){
    a = (3.0*opts->cur_iteration/opts->iterations_to_min_blur)*(3.0*opts->cur_iteration/opts->iterations_to_min_blur)*0.5;
    return (opts->max_blur_radius-opts->min_blur)*exp(-a)+opts->min_blur;
  }else if(opts->blur_radius_reduction_method == GEOMETRICAL_BLUR_REDUCTION){
    a = exp(log(opts->min_blur/opts->max_blur_radius)/opts->iterations_to_min_blur);
    return opts->max_blur_radius*pow(a,opts->cur_iteration);
  }else{
    abort();
  }
  return -1;
}

int descend_real_compare(const void * pa,const void * pb){
  real a,b;
  a = *((real *)pa);
  b = *((real *)pb);
  if(a < b){
    return 1;
  }else if(a == b){
    return 0;
  }else{
    return -1;
  }
}

void complete_reconstruction(Image * amp, Image * initial_support, Image * exp_sigma,
			     Options * opts, char * dir){

  Image * support = NULL;
  Image * prev_support = NULL;
  Image * real_in = NULL;
  Image * real_out = NULL;
  Image * tmp = NULL;
  Image * tmp2;
  char buffer[1024];
  Log log;
  real radius;
  char prev_dir[1024];
  int stop = 0;
  real support_threshold = opts->new_level;
  real support_size = -support_threshold;
  const int stop_threshold = 10;
  int i;

  init_log(&log);
  log.threshold = support_threshold;
  opts->cur_iteration = 0;
  opts->reconstruction_finished = 0;
  opts->flog = NULL;
  if(opts->automatic){
    opts->algorithm = HIO;
  }
  
  support = sp_image_duplicate(initial_support,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_write(initial_support,"support.vtk",0);
  prev_support = sp_image_duplicate(initial_support,SP_COPY_DATA|SP_COPY_MASK);

  /* Set the initial guess */
  if(opts->image_guess){
    real_in = sp_image_duplicate(opts->image_guess,SP_COPY_DATA|SP_COPY_MASK);
  }else{
    real_in = sp_image_duplicate(support,SP_COPY_DATA|SP_COPY_MASK);
  }

  /* make sure we make the input complex */
  sp_image_rephase(real_in,SP_ZERO_PHASE);
  
  /* Set random phases if needed */
  if(opts->rand_intensities){
    set_rand_ints(real_in,amp);
    if(opts->image_guess){
      fprintf(stderr,"Warning: Using random intensities, with image guess. Think about it...\n");
    }
  }
  if(opts->rand_phases){
    set_rand_phases(real_in,amp);
    if(opts->image_guess){
      fprintf(stderr,"Warning: Using random phases, with image guess. Think about it...\n");
    }
  }

#if defined(_MSC_VER) || defined(__MINGW32__)
  _getcwd(prev_dir,1024);
  _mkdir(dir);
  _chdir(dir);
#else
  getcwd(prev_dir,1024);
  mkdir(dir,0755);
  chdir(dir);
#endif
  sp_image_write(support,"support.png",COLOR_JET);
  sp_image_write(real_in,"initial_guess.png",COLOR_JET);
  sp_image_write(real_in,"initial_guess.h5",sizeof(real));
  sp_image_write(initial_support,"initial_support.png",COLOR_JET);

  if(get_algorithm(opts,&log) == HIO){     
    real_out = basic_hio_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR){
    real_out = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HPR){
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == CFLIP){
    real_out = basic_cflip_iteration(amp, real_in, support,opts,&log);
  }else{
    fprintf(stderr,"Error: Undefined algorithm!\n");
    exit(-1);
  }

  radius = opts->max_blur_radius;
    
  for(;!opts->reconstruction_finished;opts->cur_iteration++){

    if(opts->iterations && opts->cur_iteration%opts->iterations == opts->iterations-1){
      for(i = 0;i<opts->error_reduction_iterations_after_loop;i++){
	sp_image_free(real_in);
	real_in = real_out;
	real_out = basic_error_reduction_iteration(amp, real_in, support,opts,&log);
      }
      sp_image_free(prev_support);
      prev_support = sp_image_duplicate(support,SP_COPY_DATA|SP_COPY_MASK);
      sp_image_free(support);      
      support_threshold = get_newsupport_level(real_out,&support_size,radius,&log,opts);
      log.threshold = support_threshold;
      if(support_threshold > 0){
	support =  get_newsupport(real_out,support_threshold, radius,opts);
/*	support =  get_filtered_support(real_out,support_threshold, radius,opts);*/
      }else{
	if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
	  exit(0);
	}else{
	  abort();
	}
      }
      if(opts->cur_iteration <= opts->iterations_to_min_blur){
	radius = get_blur_radius(opts);
      }
      if(/*opts->cur_iteration > 50 ||*/ (opts->automatic && opts->algorithm == RAAR && log.Ereal < 0.2)){
	stop++;
      }
      if(stop > stop_threshold){
	break;
      }
    }
    if(opts->cur_iteration%opts->output_period == opts->output_period-1){
      sprintf(buffer,"real_out-%07d.h5",opts->cur_iteration);
      sp_image_write(real_out,buffer,opts->output_precision);
      sprintf(buffer,"real_out-%07d.png",opts->cur_iteration);
      sp_image_write(real_out,buffer,COLOR_JET);
      sprintf(buffer,"support-%07d.png",opts->cur_iteration);
      sp_image_write(support,buffer,COLOR_JET);
      sprintf(buffer,"support-%07d.h5",opts->cur_iteration);
      sp_image_write(support,buffer,opts->output_precision);
      tmp = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
      for(i = 0;i<sp_cmatrix_size(tmp->image);i++){
	if(support->image->data[i]){
	  tmp->image->data[i] = cabs(tmp->image->data[i]);
	}else{
	  tmp->image->data[i] = 0;
	}
      }
      sprintf(buffer,"pre_pattern-%07d.h5",opts->cur_iteration);
      /*      tmp = zero_pad_image(tmp,sp_cmatrix_cols(tmp->image)*4,sp_cmatrix_rows(tmp->image)*4,1);
      sp_image_write(tmp,buffer,opts->output_precision);
      sprintf(buffer,"pre_pattern-%07d.png",opts->cur_iteration);
      sp_image_write(tmp,buffer,COLOR_JET|LOG_SCALE);
      */
      tmp2 = sp_image_fft(tmp); 
      sp_image_free(tmp);
      tmp = tmp2;
      for(i = 0;i<sp_cmatrix_size(tmp->image);i++){
	tmp->mask->data[i] = 1;
      }
      sprintf(buffer,"pattern-%07d.h5",opts->cur_iteration);
      sp_image_write(tmp,buffer,opts->output_precision);
      sprintf(buffer,"pattern-%07d.png",opts->cur_iteration);
      sp_image_write(tmp,buffer,COLOR_JET);
      sp_image_free(tmp);

    }    
    if(opts->break_centrosym_period && 
       opts->cur_iteration%opts->break_centrosym_period == opts->break_centrosym_period-1){
      centrosym_break_attempt(real_out);
    }
    sp_image_free(real_in);
    real_in = real_out;
    if(get_algorithm(opts,&log) == HIO){     
      real_out = basic_hio_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == RAAR){     
      real_out = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == HPR){     
      real_out = basic_hpr_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == CFLIP){     
      real_out = basic_cflip_iteration(amp, real_in, support,opts,&log);
    }
  }  

  sp_image_write(real_out,"real_out_final.h5",opts->output_precision);
  sp_image_write(real_out,"real_out_final.png",COLOR_JET);
  sprintf(buffer,"support-final.png");
  sp_image_write(support,buffer,COLOR_JET);
  sprintf(buffer,"support-final.h5");
  sp_image_write(support,buffer,opts->output_precision);
  tmp = sp_image_fft(real_out); 
  for(i = 0;i<sp_cmatrix_size(tmp->image);i++){
    tmp->mask->data[i] = 1;
  }
  sprintf(buffer,"pattern-final.h5");
  sp_image_write(tmp,buffer,opts->output_precision);
  sprintf(buffer,"pattern-final.png");
  sp_image_write(tmp,buffer,COLOR_JET);
  sp_image_free(tmp);
  
  sp_image_write(real_out,"phases_out_final.png",COLOR_PHASE|COLOR_JET);
  sp_image_free(support);
  sp_image_free(prev_support);
  sp_image_free(real_in);
  sp_image_free(real_out);
  #if defined(_MSC_VER) || defined(__MINGW32__)
  _chdir(dir);
#else
  chdir(dir);
#endif
  fclose(opts->flog);
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

void set_rand_phases(Image * real_in, Image * diff){
  Image * tmp = sp_image_duplicate(diff,SP_COPY_DATA|SP_COPY_MASK);
  Image * r;
  int i;

/*  tmp = sp_image_fft(real_in); */
/*  sp_image_smooth_edges(tmp,diff->mask,SP_GAUSSIAN,&value);*/
/*  sp_image_dephase(tmp); */
  sp_image_rephase(tmp,SP_RANDOM_PHASE);
  r = sp_image_ifft(tmp);
  
  for(i = 0;i<sp_cmatrix_size(real_in->image);i++){
    real_in->image->data[i] = r->image->data[i]/(sp_image_size(tmp));
  }
  sp_image_free(tmp);
  sp_image_free(r);	  
}


void set_rand_ints(Image * real_in, Image * img){
  int i;
  real sum = 0;
  int size_in = 0;
  for(i = 0;i<sp_cmatrix_size(img->image);i++){
    sum += img->image->data[i];    
    if(real_in->image->data[i]){
      size_in++;
    }
  }
  for(i = 0;i<sp_cmatrix_size(real_in->image);i++){
    if(real_in->image->data[i]){
      real_in->image->data[i] = (p_drand48()*sum)+(p_drand48()*sum)*I;
      real_in->image->data[i]  /= (size_in*sqrt(sp_cmatrix_size(img->image)));
    }  
  }
}

int get_algorithm(Options * opts,Log * log){
  if(opts->automatic){
    if(log->dSupSize > 0){
      opts->algorithm = RAAR;
    }
  }
  return opts->algorithm;
}

real get_beta(Options * opts){
  static const int beta0 = 0.75;
  int n = opts->cur_iteration;
  if(opts->dyn_beta){
    return beta0+(opts->beta-beta0)*(1-exp(-pow(n/opts->dyn_beta,3)));
  }
  return opts->beta;
}

real get_new_threshold(Options * opts,Log * log){
  static int flag = 0;
  if(opts->adapt_thres){
    if(opts->cur_iteration > 40 && log->Ereal > /*log->Efourier*0.8 */0.3){
      flag = 1;
    }
    if(flag){
      return opts->new_level*0.75;
    }
  }
  return opts->new_level;
}


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
  fprintf(opts->flog,"%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%f\t%f\t%f\t%f\n",iter,it_outer,
	  Ereal,Efourier,FcFo,SupSize,get_beta(opts),log->threshold,get_algorithm(opts,log),
	  log->dEreal,log->dSupSize,get_blur_radius(opts),log->int_cum_fluctuation);
  fflush(opts->flog);

  if(Ereal < opts->real_error_tolerance){
    opts->reconstruction_finished = 1;
  }
}  
  
     

Image * basic_hio_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_cmatrix_size(real_in->image);
  real beta = get_beta(opts);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);

  sp_image_rephase(pattern,SP_ZERO_PHASE);

  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      if(cabsr(fft_out->image->data[i])){
	pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabsr(fft_out->image->data[i]);
      }else{
	real phase = p_drand48()*2*M_PI;
	pattern->image->data[i] = cos(phase)*exp_amp->image->data[i]+I+sin(phase)*exp_amp->image->data[i];
      }
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i]){
      if(opts->enforce_real){
	real_out->image->data[i] = creal(real_in->image->data[i] - beta*real_out->image->data[i]);      
      }else{
	real_out->image->data[i] = (real_in->image->data[i] - beta*real_out->image->data[i]) ;      
      }
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


Image * basic_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  /*We're gonna have 3 kinds of images.
   With only real space constraints (Ps)
   With only fourier space constraints (Pm)
   With both constraints (PsPm) */
  static Image * exp_amp_minus_sigma = NULL;
  static Image * exp_amp_plus_sigma = NULL;
  Image * real_out;
  Image * fft_out;
  int i;
  int size = sp_cmatrix_size(real_in->image);
  real beta = get_beta(opts);
  real one_minus_2_beta = 1.0-2*beta;
  real tmp;
  static char * weak_reflections = NULL;
  real max = 0;
  real * array;
  fft_out = sp_image_fft(real_in);
  if(!exp_amp_minus_sigma){
    exp_amp_minus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      exp_amp_minus_sigma->image->data[i] -= exp_sigma->image->data[i];
    }
  }
  if(!exp_amp_plus_sigma){
    exp_amp_plus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      exp_amp_plus_sigma->image->data[i] += exp_sigma->image->data[i];
    }
  }

  if(opts->perturb_weak_reflections && !weak_reflections){
    weak_reflections = malloc(sp_cmatrix_size(exp_amp->image)*sizeof(char));
    array = malloc(sp_cmatrix_size(exp_amp->image)*sizeof(real));
    memcpy(array,exp_amp->image,sp_cmatrix_size(exp_amp->image)*sizeof(real));
    qsort(array,sp_cmatrix_size(exp_amp->image),sizeof(real),real_compare);
    /* get the weak reflections threshold */
    max = array[(int)(sp_cmatrix_size(exp_amp->image)*opts->perturb_weak_reflections)];
    free(array);
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      if(exp_amp->mask->data[i] && creal(exp_amp->image->data[i]) <= max){
	weak_reflections[i] = 1;
      }else{
	weak_reflections[i] = 0;
      }
    }
  }

  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(exp_amp->mask->data[i]){
      /* take the calculated phases and apply to the experimental intensities 
	 leaving room for error */
      if(cabs(fft_out->image->data[i]) < creal(exp_amp_minus_sigma->image->data[i])){
	if(fft_out->image->data[i]){
	  fft_out->image->data[i] = (exp_amp_minus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
	}else{
	  fft_out->image->data[i] = exp_amp_minus_sigma->image->data[i];
	}
      }else if(cabs(fft_out->image->data[i]) > creal(exp_amp_plus_sigma->image->data[i])){
	if(fft_out->image->data[i]){
	  fft_out->image->data[i] = (exp_amp_plus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
	}else{
	  fft_out->image->data[i] = exp_amp_plus_sigma->image->data[i];
	}
      }
    }
  }
  if(opts->remove_central_pixel_phase){
    fft_out->image->data[0] = cabs(fft_out->image->data[0]);
  }

  real_out = sp_image_ifft(fft_out);
  
  /* normalize */
  tmp = 1.0/size;
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){  
    real_out->image->data[i] *= tmp;
  }




  tmp = 1.0/size;
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
/*    real_out->r[i] += (1.0-support->image->data[i])*((-2*beta)*real_out->r[i]+beta*(real_in->r[i]));
    real_out->c[i] += (1.0-support->image->data[i])*((-2*beta)*real_out->c[i]+beta*(real_in->c[i]));
    real_out->image->data[i] = amplitude(real_out,i);*/
    if(!support->image->data[i]){
      real_out->image->data[i] = (one_minus_2_beta)*real_out->image->data[i]+beta*(real_in->image->data[i]);      
    }else{
/*      real_out->image->data[i] *= tmp;       */
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(fft_out);
  return real_out;
}

#ifdef MPI
Image * mpi_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  /*We're gonna have 3 kinds of images.
   With only real space constraints (Ps)
   With only fourier space constraints (Pm)
   With both constraints (PsPm) */
  static Image * exp_amp_minus_sigma = NULL;
  static Image * exp_amp_plus_sigma = NULL;
  Image * real_out;
  Image * fft_out;
  int i;
  int size = sp_cmatrix_size(real_in->image);
  real beta = get_beta(opts);
  int id;
  int p;
  fft_out = image_mpi_fft(real_in);
  MPI_Comm_rank(MPI_COMM_WORLD,&id);
  MPI_Comm_size(MPI_COMM_WORLD,&p);
  
  if(!exp_amp_minus_sigma){
    exp_amp_minus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      exp_amp_minus_sigma->image->data[i] -= exp_sigma->image->data[i];
    }
  }
  if(!exp_amp_plus_sigma){
    exp_amp_plus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      exp_amp_plus_sigma->image->data[i] += exp_sigma->image->data[i];
    }
  }

  for(i = (id*sp_cmatrix_size(exp_amp->image))/p;i<(id+1)*sp_cmatrix_size(exp_amp->image)/p;i++){
    if(exp_amp->mask->data[i]){
      /* take the calculated phases and apply to the experimental intensities 
       leaving room for error */
      if(cabs(fft_out->image->data[i]) < creal(exp_amp_minus_sigma->image->data[i])){
	fft_out->image->data[i] = (exp_amp_minus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }else if(cabs(fft_out->image->data[i]) > creal(exp_amp_plus_sigma->image->data[i])){
	fft_out->image->data[i] = (exp_amp_plus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }
    }
  }
  real_out = sp_image_ifft(fft_out);

  for(i = (id*sp_cmatrix_size(real_out->image))/p;i<(id+1)*sp_cmatrix_size(real_out->image)/p;i++){  
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = (id*sp_cmatrix_size(real_out->image))/p;i<(id+1)*sp_cmatrix_size(real_out->image)/p;i++){  
    if(!support->image->data[i]){
      real_out->image->data[i] = (1-2*beta)*real_out->image->data[i]+beta*(real_in->image->data[i]);      
    }else{
      real_out->image->data[i] /= size;       
    }
  }
  if(opts->enforce_positivity){
    for(i = (id*sp_cmatrix_size(real_out->image))/p;i<(id+1)*sp_cmatrix_size(real_out->image)/p;i++){  
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_perioditerations == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(fft_out);
  return real_out;
}

#endif


Image * basic_error_reduction_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_cmatrix_size(real_in->image);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i]){
      real_out->image->data[i] = 0;
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


Image * basic_hpr_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_cmatrix_size(real_in->image);
  real beta = get_beta(opts);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i] || cabs(2*real_out->image->data[i]-real_in->image->data[i]) < cabs((1-beta)*(real_out->image->data[i]))){
      if(opts->enforce_real){
	real_out->image->data[i] = creal(real_in->image->data[i] - beta*real_out->image->data[i]);      
      }else{
	real_out->image->data[i] = real_in->image->data[i] - beta*real_out->image->data[i];      
      }
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


Image * basic_cflip_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  static char * weak_reflections = NULL;
  real * tmp;
  int i,j;
  int size = sp_cmatrix_size(real_in->image);
  real max = 0;
  fft_out = sp_image_fft(real_in);
  if(opts->perturb_weak_reflections && !weak_reflections){
    weak_reflections = malloc(sp_cmatrix_size(exp_amp->image)*sizeof(char));
    tmp = malloc(sp_cmatrix_size(exp_amp->image)*sizeof(real));
    memcpy(tmp,exp_amp->image,sp_cmatrix_size(exp_amp->image)*sizeof(real));
    qsort(tmp,sp_cmatrix_size(exp_amp->image),sizeof(real),real_compare);
    /* get the weak reflections threshold */
    max = tmp[(int)(sp_cmatrix_size(exp_amp->image)*opts->perturb_weak_reflections)];
    fprintf(stderr,"max - %f\nindex - %d\n",max,(int)(sp_cmatrix_size(exp_amp->image)*opts->perturb_weak_reflections));
    free(tmp);
    j = 0;
    for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
      if(exp_amp->mask->data[i] && creal(exp_amp->image->data[i]) <= max){
	weak_reflections[i] = 1;
	j++;
      }else{
	weak_reflections[i] = 0;
      }
    }
    fprintf(stderr,"marked %d reflections\n",j);  
  }

  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_cmatrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      if(opts->perturb_weak_reflections && weak_reflections[i]){
	/* take the calculated phases, rotate PI/2 and apply to the experimental intensities */
	/* BUG This does not seems like a PI/2 rotation to me! */
	pattern->image->data[i] = exp_amp->image->data[i]*conj(fft_out->image->data[i])/cabs(fft_out->image->data[i]);
      }else{
	/* take the calculated phases and apply to the experimental intensities */	
	pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* normalize */
    if(opts->enforce_real){
      real_out->image->data[i] = creal(real_out->image->data[i])/size;
    }else{
      real_out->image->data[i] /= size;
    }
    if(cabs(real_out->image->data[i]) > max){
      max = cabs(real_out->image->data[i]);
    }
  }
  for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
    /* charge flipping doesn't know about support */
    if(cabs(real_out->image->data[i]) < opts->charge_flip_sigma*max){
      real_out->image->data[i] *= -1;
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_cmatrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}




Image * get_newsupport(Image * input, real level , real radius/*,Image * previous_support,
								    Image * patterson*/, Options * opts){
  real max_int = 0;
  real avg_int = 0;
  Image * res;
/*  Image * mask;
  Image * patterson_mask;*/
  int i;
  res = gaussian_blur(input, radius);
  sp_image_dephase(res);
/*  mask = gaussian_blur(previous_support, radius/3);
  sp_image_dephase(mask);
  patterson_mask = gaussian_blur(patterson, radius);
  sp_image_dephase(patterson_mask);*/

  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(max_int < creal(res->image->data[i])){
      max_int = creal(res->image->data[i]);
    }
    avg_int += creal(res->image->data[i]);
  }
  avg_int /= sp_cmatrix_size(res->image);
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(creal(res->image->data[i]) < max_int*level /*|| !mask->image->data[i] || !patterson->image->data[i]*/){
      res->image->data[i] = 0;
    }else if(creal(input->image->data[i]) > max_int/10 /*&& patterson_mask->image->data[i]*/){
    /* strong negative points close to the support should be included */
    /* This are enhacements for beam stop associated problems */
      /*      patterson->image->data[i] = 1;*/
      res->image->data[i] = 1;
    }else{
      res->image->data[i] = 1;
    }
  }
  if(opts->support_mask){
    sp_image_add(res,opts->support_mask);
  }
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(res->image->data[i]){
      res->image->data[i] = 1;
    }
  }
/*  sp_image_free(mask);
  sp_image_free(patterson_mask);*/
  return res;
}


/* Important warning! Previous size gets overwritten by the new size */
real get_newsupport_level(Image * input, real * previous_size , real radius, Log * log, Options * opts){
  static int stepped_flag = 0;
  real max_int = 0;
  int new_size;
  real new_level;
  Image * res;
  real reduction = 0;
  real real_error_threshold;
/*  Image * mask;
  Image * patterson_mask;*/
  int i;
  if(opts->support_update_algorithm == FIXED){
    return opts->new_level;
  }else if(opts->support_update_algorithm == STEPPED){
    if(opts->cur_iteration > 40 && log->Ereal > 0.3){
      stepped_flag = 1;
    }
    if(stepped_flag){
      return opts->new_level*0.75;
    }
    return opts->new_level;
  }else if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
    if(log->Ereal < opts->real_error_threshold){
      return opts->new_level;
    }else{
      return -1;
    }
  }else if(opts->support_update_algorithm ==  REAL_ERROR_ADAPTATIVE){
    
    if(opts->real_error_threshold < 0){
      if(get_algorithm(opts,log) == HIO){
	real_error_threshold = (real)0.55;
      }else if(get_algorithm(opts,log) == RAAR){
	real_error_threshold = (real)0.20;
      }else{
	abort();
      }
    }else{
      real_error_threshold = opts->real_error_threshold;
    }

    reduction = 1-(real_error_threshold-log->Ereal)/10.0;

    
    res = gaussian_blur(input, radius);
    sp_image_dephase(res);
    /*  mask = gaussian_blur(previous_support, radius/3);
	sp_image_dephase(mask);
	patterson_mask = gaussian_blur(patterson, radius);
	sp_image_dephase(patterson_mask);*/
    
    for(i = 0;i<sp_cmatrix_size(res->image);i++){
      if(max_int < creal(res->image->data[i])){
	max_int = creal(res->image->data[i]);
      }
    }
    qsort(res->image,sp_cmatrix_size(res->image),sizeof(real),descend_real_compare);
    if((*previous_size) < 0){
      for(i = 0;i<sp_cmatrix_size(res->image);i++){
	if(creal(res->image->data[i]) < -(*previous_size)){
	  break;
	}
    }
      *previous_size = i;
    }
    new_size = (*previous_size)*reduction;
    *previous_size = new_size;
    new_level = creal(res->image->data[new_size])/max_int;
    sp_image_free(res);
    return new_level;
  }else{
    fprintf(stderr,"Unkown algorithm!\n");
    abort();
  }
  return 0;
}
  
Image * get_support(Image * input, Options * opts){
  int i,x,y,dx,dy;
  real level = opts->init_level;
  real integr = 0;
  real avg;
  real max_int = 0;
  real max_dist = 0;
  real dist = 0;
  Image * patterson = sp_image_fft(input);
  Image * tmp_img;
  for(i = 0;i<sp_cmatrix_size(input->image);i++){
    input->image->data[i] *= input->image->data[i];      
  }

  sp_image_dephase(patterson);
  for(i = 0;i<sp_cmatrix_size(input->image);i++){
    input->image->data[i] = sqrt(input->image->data[i]);
    if(!input->mask->data[i]){
      input->image->data[i] = 0;
    }
  }
  tmp_img = sp_image_shift(patterson);
  sp_image_free(patterson);
  patterson = tmp_img;

  /* contrast stretch */
/*  sp_image_adaptative_constrast_stretch(patterson,10,10); */

  sp_image_write(patterson,"autocorrelation.png",COLOR_JET|LOG_SCALE);
  sp_image_write(patterson,"autocorrelation.vtk",0);
  for(i = 0;i<sp_cmatrix_size(patterson->image);i++){
    integr += patterson->image->data[i];
    if(max_int < creal(patterson->image->data[i])){
      max_int = creal(patterson->image->data[i]);
    }
  }
  avg = integr/i;

  for(x = 0;x<sp_cmatrix_cols(patterson->image);x++){
    for(y = 0;y<sp_cmatrix_rows(patterson->image);y++){
      if(patterson->image->data[x*sp_cmatrix_rows(patterson->image)+y]){
	dx = x-(sp_cmatrix_cols(patterson->image)-1.0)/2.0;
	dy = y-(sp_cmatrix_rows(patterson->image)-1.0)/2.0;
	dist = sqrt(dx*dx+dy*dy);
	if(dist > max_dist){
	  max_dist = dist;
	}
      }
    }
  }

  for(x = 0;x<sp_cmatrix_cols(patterson->image);x++){
    for(y = 0;y<sp_cmatrix_rows(patterson->image);y++){
      if(patterson->image->data[x*sp_cmatrix_rows(patterson->image)+y]){
	dx = x-(sp_cmatrix_cols(patterson->image)-1.0)/2.0;
	dy = y-(sp_cmatrix_rows(patterson->image)-1.0)/2.0;
	dist = sqrt(dx*dx+dy*dy);
	/* ATTENTION for some strange reason using max_dist/2, the natural
	 choice makes the algorithm go stupid most of the times */
/*	if(dist > max_dist/1.2){
	  patterson->image->data[x*sp_cmatrix_rows(patterson->image)+y] = 0;
	}*/
      }
    }
  }
  sp_image_write(patterson,"support2.png",COLOR_JET);

  for(i = 0;i<sp_cmatrix_size(patterson->image);i++){
    if(creal(patterson->image->data[i]) < max_int*level){
      patterson->image->data[i] = 0;
    }else{
      patterson->image->data[i] = 1;
    }
  }
  sp_image_write(patterson,"support3.png",COLOR_JET);
  if(opts->blur_patterson){
    tmp_img = gaussian_blur(patterson,3);
    sp_image_write(tmp_img,"support4.png",COLOR_JET);
    sp_image_write(tmp_img,"support4.vtk",0);
    sp_image_free(patterson);
    patterson = sp_image_duplicate(tmp_img,SP_COPY_DATA|SP_COPY_MASK);
    sp_image_free(tmp_img);
  }

  patterson->detector->image_center[0] = (sp_cmatrix_cols(patterson->image)-1)/2;
  patterson->detector->image_center[1] = (sp_cmatrix_rows(patterson->image)-1)/2;
  /* Apply oversampling square mask */
  if(opts->square_mask){
    for(i = 0;i<sp_cmatrix_size(patterson->image);i++){
      if(sp_image_dist(patterson,i,SP_TO_CENTER) > sp_cmatrix_cols(patterson->image)/4){
	patterson->image->data[i] = 0;
      }
    }
  }

  for(i = 0;i<sp_cmatrix_size(patterson->image);i++){
    if(creal(patterson->image->data[i]) > 1e-6){
      patterson->image->data[i] = 1;
    }else{
      patterson->image->data[i] = 0;
    }
  }
  sp_image_write(patterson,"support5.png",COLOR_JET);
  sp_image_write(patterson,"support5.vtk",0);
  return patterson;  
}





/* This will return a support based not only on the level, but also on the variance of the image
   compared to the region. If it's above 3 sigma + average , it's included */
Image * get_filtered_support(Image * input, real level , real radius, Options * opts){
  real max_int = 0;
  real avg_int = 0;
  Image * res = gaussian_blur(input, radius);
  Image * running_average = square_blur(res, radius);
  Image * absolute_error = sp_image_duplicate(input,SP_COPY_DATA|SP_COPY_MASK);
  Image * variance;
/*  Image * mask;
  Image * patterson_mask;*/
  int i;
  for(i = 0;i<sp_cmatrix_size(input->image);i++){
    absolute_error->image->data[i] = (res->image->data[i]-running_average->image->data[i])*(res->image->data[i]-running_average->image->data[i]);
  }
  variance = square_blur(absolute_error,radius);
  sp_image_write(variance,"variance.vtk",0);
  sp_image_write(running_average,"r_avg.vtk",0);
  
  sp_image_dephase(res);
/*  mask = gaussian_blur(previous_support, radius/3);
  sp_image_dephase(mask);
  patterson_mask = gaussian_blur(patterson, radius);
  sp_image_dephase(patterson_mask);*/

  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(max_int < creal(res->image->data[i])){
      max_int = creal(res->image->data[i]);
    }
    avg_int += res->image->data[i];
  }
  avg_int /= sp_cmatrix_size(res->image);
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(creal(res->image->data[i]) > max_int*level /*|| !mask->image->data[i] || !patterson->image->data[i]*/){
      res->image->data[i] = 1;
    }else if(creal(res->image->data[i]) > creal(running_average->image->data[i]) + 3* sqrt(variance->image->data[i])){
      res->image->data[i] = 1;
    }else{
      res->image->data[i] = 0;
    }
  }
  if(opts->support_mask){
    sp_image_add(res,opts->support_mask);
  }
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(res->image->data[i]){
      res->image->data[i] = 1;
    }
  }

/*  sp_image_free(mask);
  sp_image_free(patterson_mask);*/
  sp_image_free(running_average);
  sp_image_free(absolute_error);
  sp_image_free(variance);
  return res;
}



int main(int argc, char ** argv){
  Image * img = NULL;
  Image * exp_sigma = NULL;
  char dir[1024];
  int i;
  FILE * f;
  Options * opts;
#ifdef MPI
  MPI_Init(&argc, &argv);
#endif  
  opts = set_defaults();

  
  srand(getpid());
  
  f = fopen("uwrapc.conf","rb");
  if(f){
    fclose(f);
    read_options_file("uwrapc.conf",opts);
  }else{
	perror("Could not open uwrapc.conf");
  }
  parse_options(argc,argv,opts);
  write_options_file("uwrapc.confout",opts);
  sp_init_fft(opts->nthreads);
  if(opts->real_image){
    img= sp_image_fft(opts->real_image);
    sp_image_dephase(img);
  }else if(opts->diffraction){
    img = sp_image_duplicate(opts->diffraction,SP_COPY_DATA|SP_COPY_MASK);
  }else{
    fprintf(stderr,"Error: either -d or -i have to be specified!\n");
    exit(1);
  }
  sp_image_high_pass(img, opts->beamstop);
  sp_add_noise(img,opts->noise,SP_GAUSSIAN);
  if(opts->rescale_amplitudes){
    rescale_image(img);
  }
  sp_image_write(img,"diffraction.png",COLOR_JET);

  if(!opts->init_support){
    opts->init_support = get_support(img,opts);
  }

   

  exp_sigma = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  for(i = 0;i<sp_cmatrix_size(img->image);i++){
    exp_sigma->image->data[i] = img->image->data[i]*opts->exp_sigma;
  }

  /* Make sure to scale everything to the same size if needed */
  harmonize_sizes(opts);
  
  if(opts->support_mask){
    sp_image_add(opts->init_support,opts->support_mask);
  }

  for(i = 0;i<sp_cmatrix_size(opts->init_support->image);i++){
    if(opts->init_support->image->data[i]){
      opts->init_support->image->data[i] = 1;
    }
  }

  if(opts->automatic){
    for(i = 0;i<1000;i++){
      sprintf(dir,"%s/%04d/",opts->work_dir,i);
      if(opts->genetic_optimization){
/*	genetic_reconstruction(img, initial_support, exp_sigma, opts,dir)*/;
      }else{
	complete_reconstruction(img, opts->init_support, exp_sigma, opts,dir);
      }
    }
  }else{
    sprintf(dir,"%s/",opts->work_dir);
    if(opts->genetic_optimization){
/*      genetic_reconstruction(img, initial_support, exp_sigma, opts,dir)*/;
    }else{
      complete_reconstruction(img, opts->init_support, exp_sigma, opts,dir);
    }
  }
  return 0;  
}








