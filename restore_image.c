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
#include "log.h"
#include "configuration.h"
#include "support.h"
#include "algorithms.h"



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





/* This routine tries to determine the experimental error on each pixel
   by doing a low pass filtering of the image using the initial support
   as constraints.
   
   If the image is perfect the low pass image should be exactly the same
   as the original image. Differences then are caused by experimental error.
   This allows us to have an idea of the error in each pixel. If the error
   is very large, we can consider the pixel tainted and mask it out.
*/
Image * estimate_error(Image * intensities, Image * support, Options * opts){
  
}

/* continue a reconstruction on this directory */
/* these will simply set image_guess and initial_support
   to the last real_out and support_ respectively */
void continue_reconstruction(Options * opts){
  
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

void complete_restoration(Image * amp, Image * initial_support, Options * opts, char * dir){

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
    real_out = basic_raar_iteration(amp,NULL, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HPR){
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == CFLIP){
    real_out = basic_cflip_iteration(amp, real_in, support,opts,&log);
  }else{
    fprintf(stderr,"Error: Undefined algorithm!\n");
    exit(-1);
  }

  radius = opts->max_blur_radius;
    
  for(;!opts->reconstruction_finished && opts->cur_iteration < opts->max_iterations;opts->cur_iteration++){

    if(opts->iterations && opts->cur_iteration%opts->iterations == opts->iterations-1){
      for(i = 0;i<opts->error_reduction_iterations_after_loop;i++){
	sp_image_free(real_in);
	real_in = real_out;
	real_out = basic_error_reduction_iteration(amp, real_in, support,opts,&log);
      }
      sp_image_free(prev_support);
      prev_support = sp_image_duplicate(support,SP_COPY_DATA|SP_COPY_MASK);
      sp_image_free(support);      
      support_threshold = get_support_level(real_out,&support_size,radius,&log,opts);
      log.threshold = support_threshold;
      if(support_threshold > 0){
	support =  get_updated_support(real_out,support_threshold, radius,opts);
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
      sprintf(buffer,"real_out_phase-%07d.png",opts->cur_iteration);
      sp_image_write(real_out,buffer,COLOR_JET|COLOR_PHASE);
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
    sp_image_free(real_in);
    real_in = real_out;
    if(get_algorithm(opts,&log) == HIO){     
      real_out = basic_hio_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == RAAR){     
      real_out = basic_raar_iteration(amp,NULL, real_in, support,opts,&log);
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
  
  f = fopen("restore_image.conf","rb");
  if(f){
    fclose(f);
    read_options_file("restore_image.conf",opts);
  }else{
	perror("Could not open restore_image.conf");
  }
  parse_options(argc,argv,opts);
  write_options_file("restore_image.confout",opts);
  sp_init_fft(opts->nthreads);
  if(opts->diffraction){
    img = sp_image_duplicate(opts->diffraction,SP_COPY_DATA|SP_COPY_MASK);
    sp_image_to_intensities(img);
  }else{
    fprintf(stderr,"Error: Diffraction pattern not specified!\n");
    exit(1);
  }
  sp_image_write(img,"intensities.png",COLOR_JET);

  if(!opts->init_support){
    opts->init_support = get_support_from_patterson(img,opts);
  }

     
  if(opts->support_mask){
    sp_image_add(opts->init_support,opts->support_mask);
  }

  for(i = 0;i<sp_cmatrix_size(opts->init_support->image);i++){
    if(opts->init_support->image->data[i]){
      opts->init_support->image->data[i] = 1;
    }
  }

  exp_sigma = estimate_error(img,opts->init_support,opts);

  sprintf(dir,"%s/",opts->work_dir);
  complete_restoration(img, opts->init_support, opts,dir);
  return 0;  
}







