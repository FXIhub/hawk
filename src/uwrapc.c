#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include <unistd.h>

#ifdef MPI
#include <mpi.h>
#endif
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif


#include "spimage.h"
#include "log.h"
#include "uwrapc.h"
#include "configuration.h"
#include "algorithms.h"
#include "support.h"
#include "network_communication.h"


void get_intensities_noise(Options * opts){
  int i;
  if(opts->intensities_std_dev_filename[0]){
    /* read standard deviations from a file */
    opts->intensities_std_dev = sp_image_read(opts->intensities_std_dev_filename,0);
  }else if(opts->autocorrelation_support_filename[0]){
    opts->autocorrelation_support = sp_image_read(opts->autocorrelation_support_filename,0);
    /* Make sure the autocorrelation support is shifted */
    if(!opts->autocorrelation_support->shifted){
      Image * tmp = sp_image_shift(opts->autocorrelation_support);
      sp_image_free(opts->autocorrelation_support);
      opts->autocorrelation_support = tmp;
    }
    /* Make sure the support is all 1 and 0 */
    for(i = 0;i<sp_image_size(opts->autocorrelation_support);i++){
      if(sp_real(opts->autocorrelation_support->image->data[i])){
	sp_real(opts->autocorrelation_support->image->data[i]) = 1;
      }
    }
    opts->intensities_std_dev = sp_image_noise_estimate(opts->amplitudes,opts->autocorrelation_support);
  }else if(opts->exp_sigma){
    opts->intensities_std_dev = sp_image_duplicate(opts->amplitudes,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_c3matrix_size(opts->amplitudes->image);i++){
      sp_real(opts->intensities_std_dev->image->data[i]) = sp_real(opts->amplitudes->image->data[i])*opts->exp_sigma;
    }
  }else{
    /* Take the square root of the intensity as the noise */
    opts->intensities_std_dev = sp_image_duplicate(opts->amplitudes,SP_COPY_DATA|SP_COPY_MASK);
  }
}



/* continue a reconstruction on this directory */
/* these will simply set image_guess and initial_support
   to the last real_out and support_ respectively */

/*void continue_reconstruction(Options * opts){
  
  } */


void synchronize_image_data(Image **  real_out, Image ** support){
  if(global_options.current_real_space_image != real_out){
    sp_image_free(*real_out);
    *real_out = *global_options.current_real_space_image;
    sp_free(global_options.current_real_space_image);
    global_options.current_real_space_image = real_out;
  }
  if(global_options.current_support != support){
    sp_image_free(*support);
    *support = *global_options.current_support;
    sp_free(global_options.current_support);
    global_options.current_support = support;
  }
}


/* Make sure that */
/*void rescale_images(Image * a,Image * b){
  double sum = 0;
  int i;
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    sum += sp_real(a->image->data[i]);
  }
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    sp_real(a->image->data[i]) /= sum;
  } 
  }*/

void enforce_parsevals_theorem(Image * master, Image * to_scale){
  int i;
  double sum_m = 0;
  double sum_s = 0;
  double f;
  Image * tmp;
  if(sp_image_size(master) != sp_image_size(to_scale)){
    fprintf(stderr,"Cannot enforce parsevals_theorem on images of different sizes!\n");
    return;
  }

  /* 
     In theory I should be able to rescale them without the need to explicitly calculate the fourier transform
     We calculate the fourier transform using unscaled FTT so the result is sqrt(image_size) bigger than the predicted
     by the parsevals_theorem. So we could do the same without the FFT by dividinf f by sqrt(image_size)
  */
  tmp = sp_image_fft(to_scale);
  for(i = 0;i<sp_image_size(master);i++){
    sum_m += sp_real(master->image->data[i])*sp_real(master->image->data[i])+sp_imag(master->image->data[i])*sp_imag(master->image->data[i]);
/*    sum_s += sp_real(to_scale->image->data[i])*sp_real(to_scale->image->data[i])+sp_imag(to_scale->image->data[i])*sp_imag(to_scale->image->data[i]);*/
    sum_s += sp_real(tmp->image->data[i])*sp_real(tmp->image->data[i])+sp_imag(tmp->image->data[i])*sp_imag(tmp->image->data[i]);
  }
  sp_image_free(tmp);
  f = sqrt(sum_m/sum_s);
  /*  printf("Scaling to_scale by %f\n",f); */
  for(i = 0;i<sp_image_size(master);i++){
    sp_real(to_scale->image->data[i]) *= f;
    sp_imag(to_scale->image->data[i]) *= f;
  }  
}

void centrosym_break_attempt(Image * a){
  int x,y,z;
  for(x = 0;x<sp_c3matrix_x(a->image);x++){
    for(y = 0;y<sp_c3matrix_y(a->image);y++){
      for(z = 0;z<sp_c3matrix_z(a->image);z++){
	sp_real(a->image->data[z*sp_c3matrix_y(a->image)*sp_c3matrix_x(a->image)+
		       y*sp_c3matrix_x(a->image)+x]) *= 0.5;
	sp_imag(a->image->data[z*sp_c3matrix_y(a->image)*sp_c3matrix_x(a->image)+
		       y*sp_c3matrix_x(a->image)+x]) *= 0.5;
      }
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
     (sp_c3matrix_x(tmp->image) != sp_c3matrix_x(exp->image) ||
      sp_c3matrix_y(tmp->image) != sp_c3matrix_y(exp->image) ||
      sp_c3matrix_z(tmp->image) != sp_c3matrix_z(exp->image))){
    fprintf(stderr,"Rescaling support_mask\n");
    tmp = bilinear_rescale(opts->support_mask,sp_c3matrix_x(exp->image),sp_c3matrix_y(exp->image),sp_c3matrix_z(exp->image));
    sp_image_free(opts->support_mask);
    opts->support_mask = tmp;
    /* Stop rounding errors in the supports */
    for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
      if(sp_real(tmp->image->data[i]) < 1e-6){
	tmp->image->data[i] = sp_cinit(0,0);
      }else{
	tmp->image->data[i] = sp_cinit(1,0);
      }
    }
  }
  tmp = opts->init_support;
  if(tmp &&
     (sp_c3matrix_x(tmp->image) != sp_c3matrix_x(exp->image) ||
      sp_c3matrix_y(tmp->image) != sp_c3matrix_y(exp->image) ||
      sp_c3matrix_z(tmp->image) != sp_c3matrix_z(exp->image))){
    fprintf(stderr,"Rescaling init_support\n");
    tmp = bilinear_rescale(opts->init_support,sp_c3matrix_x(exp->image),sp_c3matrix_y(exp->image),sp_c3matrix_z(exp->image));
    sp_image_free(opts->init_support);
    opts->init_support = tmp;
    /* Stop rounding errors in the supports */
    for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
      if(sp_real(tmp->image->data[i]) < 1e-1){
	tmp->image->data[i] = sp_cinit(0,0);
      }else{
	tmp->image->data[i] = sp_cinit(1,0);
      }
    }
  }
  tmp = opts->image_guess;
  if(tmp &&
     (sp_c3matrix_x(tmp->image) != sp_c3matrix_x(exp->image) ||
      sp_c3matrix_y(tmp->image) != sp_c3matrix_y(exp->image) ||
      sp_c3matrix_z(tmp->image) != sp_c3matrix_z(exp->image))){
    fprintf(stderr,"Rescaling image_guess\n");
    tmp = fourier_rescale(opts->image_guess,sp_c3matrix_x(exp->image),sp_c3matrix_y(exp->image),sp_c3matrix_z(exp->image));
    sp_image_free(opts->image_guess);
    opts->image_guess = tmp;
    sp_image_write(tmp,"rescaled_guess.png",COLOR_JET);
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
  Image * real_out_sum = sp_image_alloc(sp_image_x(initial_support),sp_image_y(initial_support),sp_image_z(initial_support));
  int real_out_sum_n = 0;
  global_options.current_support = &support;
  global_options.current_real_space_image = &real_out;
  char buffer[1024];
  Log log;
  real radius;
  char prev_dir[1024];
  int stop = 0;
  real support_threshold = opts->new_level;
  real support_size = -support_threshold;
  const int stop_threshold = 10;
  int i;

#if defined(_MSC_VER) || defined(__MINGW32__)
  _getcwd(prev_dir,1024);
  _chdir(dir);
#else
  getcwd(prev_dir,1024);
  chdir(dir);
#endif


  init_log(&log);

  log.threshold = support_threshold;
  opts->cur_iteration = 0;
  opts->reconstruction_finished = 0;
  opts->flog = NULL;
  if(opts->automatic){
    opts->algorithm = HIO;
  }
  
  /* clear real_out_sum */
  real_out_sum_n = 0;
  for(int i = 0;i<sp_image_size(real_out_sum);i++){
    sp_real(real_out_sum->image->data[i]) = 0;
    sp_imag(real_out_sum->image->data[i]) = 0;
  }


  support = sp_image_duplicate(initial_support,SP_COPY_DATA|SP_COPY_MASK);
  /* Initialize support size. Necessary for difference map to work reliably */
  log.SupSize = 0;
  for(int i = 0;i<sp_image_size(support);i++){
    if(sp_real(support->image->data[i])){
      log.SupSize +=1;
    }
  }
  log.SupSize /=sp_image_size(support);
  log.SupSize *=100;

  sp_image_write(initial_support,"support.vtk",SP_3D);
  prev_support = sp_image_duplicate(initial_support,SP_COPY_DATA|SP_COPY_MASK);


  real_in = sp_image_duplicate(opts->image_guess,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_write(real_in,"initial_guess.vtk",SP_3D);

  if(amp->num_dimensions == SP_2D){
    tmp2 = sp_image_shift(amp);
    sp_image_write(tmp2,"initial_support.png",COLOR_JET);
    sp_image_free(tmp2);
  }

  /* make sure we make the input complex */
  sp_image_rephase(real_in,SP_ZERO_PHASE);
  

  if(real_in->num_dimensions == SP_2D){
    sp_image_write(support,"support.png",COLOR_GRAYSCALE);
    sp_image_write(real_in,"initial_guess.png",COLOR_JET);
    sp_image_write(real_in,"initial_guess.h5",sizeof(real));
    sp_image_write(initial_support,"initial_support.png",COLOR_GRAYSCALE);
  }else if(real_in->num_dimensions == SP_3D){
    sp_image_write(support,"support.vtk",0);
  }

  if(get_algorithm(opts,&log) == HIO){     
    real_out = basic_hio_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR){
    real_out = basic_raar_iteration(amp,opts->intensities_std_dev, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HPR){
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == CFLIP){
    real_out = basic_cflip_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HAAR){
    real_out = basic_haar_iteration(amp, NULL, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == SO2D){
    real_out = basic_so2d_iteration(amp, NULL, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR_PROJ){
    real_out = basic_raar_proj_iteration(amp, opts->intensities_std_dev, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HIO_PROJ){
    real_out = basic_hio_proj_iteration(amp, opts->intensities_std_dev, real_in, support, opts, &log);
  }else if(get_algorithm(opts,&log) == DIFF_MAP){
    real_out = serial_difference_map_iteration(amp,real_in, support,opts,&log);
  }else{
    fprintf(stderr,"Error: Undefined algorithm!\n");
    exit(-1);
  }

  radius = opts->max_blur_radius;


  /* Change the status to running */
  opts->is_running = 1;
    
  for(;!opts->reconstruction_finished && (!opts->max_iterations || opts->cur_iteration < opts->max_iterations);opts->cur_iteration++){
    /* I'm only going to allow changes to images in the beggining of each iteration */
    synchronize_image_data(&real_out,&support);

    /* Add current real_out to the average real_out*/
    if(opts->iterations-opts->cur_iteration%opts->iterations <= opts->support_image_averaging){
      sp_image_add(real_out_sum,real_out);
      real_out_sum_n++;
    }

    if(opts->image_blur_period && opts->cur_iteration%opts->image_blur_period == opts->image_blur_period-1){
      sp_image_free(real_in);
      real_in = gaussian_blur(real_out,opts->image_blur_radius);
      sp_image_memcpy(real_out,real_in);
    }

    if(opts->iterations && opts->cur_iteration%opts->iterations == opts->iterations-1){
      sp_image_scale(real_out_sum,1.0/real_out_sum_n);
      for(i = 0;i<opts->error_reduction_iterations_after_loop;i++){
	sp_image_free(real_in);
	real_in = real_out;
	real_out = basic_error_reduction_iteration(amp, real_in, support,opts,&log);
      }
      if(get_phases_blur_radius(opts)){
	phase_smoothening_iteration(real_out,opts,&log);
      }
      sp_image_free(prev_support);
      prev_support = sp_image_duplicate(support,SP_COPY_DATA|SP_COPY_MASK);
      sp_image_free(support);
      support_threshold = get_support_level(real_out_sum,&support_size,radius,&log,opts);
      log.threshold = support_threshold;
      if(support_threshold > 0){
	support =  get_updated_support(real_out_sum,support_threshold, radius,opts);
      }else{
	if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
	  exit(0);
	}else{
	  abort();
	}
      }
      if(opts->filter_intensities){
	filter_intensities_with_support(amp,real_out_sum,support,opts);
      }
      if(opts->cur_iteration <= opts->iterations_to_min_blur){
	radius = get_blur_radius(opts);
      }
      /* clear real_out_sum */
      real_out_sum_n = 0;
      for(int i = 0;i<sp_image_size(real_out_sum);i++){
	sp_real(real_out_sum->image->data[i]) = 0;
 	sp_imag(real_out_sum->image->data[i]) = 0;
      }

      if(/*opts->cur_iteration > 50 ||*/ (opts->automatic && opts->algorithm == RAAR && log.Ereal < 0.2)){
	stop++;
      }
      if(stop > stop_threshold){
	break;
      }
    }
    if(opts->cur_iteration%opts->output_period == opts->output_period-1){
      if(real_in->num_dimensions == SP_2D){
	sprintf(buffer,"real_out-%07d.png",opts->cur_iteration);
	sp_image_write(real_out,buffer,COLOR_JET);
	sprintf(buffer,"real_out_phase-%07d.png",opts->cur_iteration);
	//	sp_image_write(real_out,buffer,COLOR_WHEEL|COLOR_WEIGHTED_PHASE);
	sp_image_write(real_out,buffer,COLOR_WHEEL|COLOR_PHASE);
	sprintf(buffer,"support-%07d.png",opts->cur_iteration);
	sp_image_write(support,buffer,COLOR_GRAYSCALE);
	sprintf(buffer,"amplitudes-%07d.png",opts->cur_iteration);
	sp_image_write(amp,buffer,COLOR_JET);
	
      }
      if(real_in->num_dimensions == SP_3D){
	sprintf(buffer,"real_out-%07d.vtk",opts->cur_iteration);
	sp_image_write(real_out,buffer,0);
	sprintf(buffer,"support-%07d.vtk",opts->cur_iteration);
	sp_image_write(support,buffer,0);
	sprintf(buffer,"amplitudes-%07d.vtk",opts->cur_iteration);
	sp_image_write(amp,buffer,0);
      }
      sprintf(buffer,"real_out-%07d.h5",opts->cur_iteration);
      sp_image_write(real_out,buffer,opts->output_precision);
      sprintf(buffer,"support-%07d.h5",opts->cur_iteration);
      sp_image_write(support,buffer,opts->output_precision);
      sprintf(buffer,"amplitudes-%07d.h5",opts->cur_iteration);
      sp_image_write(amp,buffer,0);
	
      tmp = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
	if(sp_real(support->image->data[i])){
	  ;
	}else{
	  tmp->image->data[i] = sp_cinit(0,0);
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
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
	tmp->mask->data[i] = 1;
      }
      sprintf(buffer,"pattern-%07d.h5",opts->cur_iteration);
      sp_image_write(tmp,buffer,opts->output_precision);
      tmp2 = sp_image_shift(tmp);

      if(tmp2->num_dimensions == SP_2D){
	sprintf(buffer,"pattern-%07d.png",opts->cur_iteration);
	sp_image_write(tmp2,buffer,COLOR_JET);
	sp_image_free(tmp2);
      }

      /*      sprintf(buffer,"pattern-%07d.vtk",opts->cur_iteration);
      sp_image_write(tmp,buffer,SP_3D);
      sp_image_free(tmp);*/

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
    }else if(get_algorithm(opts,&log) == HAAR){     
      real_out = basic_haar_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == SO2D){     
      real_out = basic_so2d_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == RAAR_PROJ){ 
      real_out = basic_raar_proj_iteration(amp, opts->intensities_std_dev,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == HIO_PROJ){
      real_out = basic_hio_proj_iteration(amp, opts->intensities_std_dev, real_in, support, opts, &log);
    }else if(get_algorithm(opts,&log) == DIFF_MAP){     
      real_out = serial_difference_map_iteration(amp,real_in, support,opts,&log);
    }

  }  

  //sp_image_write(real_out,"real_out_final.h5",opts->output_precision|SP_2D);
  //sp_image_write(real_out,"real_out_final.png",COLOR_JET|SP_2D);
  sp_image_write(real_out,"real_out_final.vtk",0);
  //sprintf(buffer,"support-final.png");
  //sp_image_write(support,buffer,COLOR_JET|SP_2D);
  //sprintf(buffer,"support-final.h5");
  //sp_image_write(support,buffer,opts->output_precision|SP_2D);
  sprintf(buffer,"support-final.vtk");
  sp_image_write(support,buffer,opts->output_precision);
  tmp = sp_image_fft(real_out); 
  for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
    tmp->mask->data[i] = 1;
  }
  sprintf(buffer,"pattern-final.h5");
  //sp_image_write(tmp,buffer,opts->output_precision);
  sprintf(buffer,"pattern-final.png");
  //sp_image_write(tmp,buffer,COLOR_JET);
  sprintf(buffer,"pattern-final.vtk");
  sp_image_write(tmp,buffer,0);
  sp_image_free(tmp);
  
  //sp_image_write(real_out,"phases_out_final.png",COLOR_PHASE|COLOR_JET);
  sp_image_free(support);
  sp_image_free(prev_support);
  sp_image_free(real_in);
  sp_image_free(real_out);
  #if defined(_MSC_VER) || defined(__MINGW32__)
  _chdir(dir);
#else
  chdir(dir);
#endif
  if(opts->flog){
    fclose(opts->flog);
  }
  if(log.cumulative_fluctuation){
    sp_image_free(log.cumulative_fluctuation);
  }
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
  
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    real_in->image->data[i] = sp_cscale(r->image->data[i],1.0/(sp_image_size(tmp)));
  }
  sp_image_free(tmp);
  sp_image_free(r);	  
}


void set_zero_phases(Image * real_in, Image * diff){
  Image * tmp = sp_image_duplicate(diff,SP_COPY_DATA|SP_COPY_MASK);
  Image * r;
  int i;

/*  tmp = sp_image_fft(real_in); */
/*  sp_image_smooth_edges(tmp,diff->mask,SP_GAUSSIAN,&value);*/
/*  sp_image_dephase(tmp); */
  sp_image_rephase(tmp,SP_ZERO_PHASE);
  r = sp_image_ifft(tmp);
  
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    real_in->image->data[i] = sp_cscale(r->image->data[i],1.0/(sp_image_size(tmp)));
  }
  sp_image_free(tmp);
  sp_image_free(r);	  
}


void set_rand_ints(Image * real_in, Image * img){
  int i;
  real sum = 0;
  int size_in = 0;
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    sum += sp_real(img->image->data[i]);    
    if(sp_real(real_in->image->data[i])){
      size_in++;
    }
  }
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    if(sp_real(real_in->image->data[i])){
      real_in->image->data[i] = sp_cinit((p_drand48()*sum),(p_drand48()*sum));
      real_in->image->data[i]  = sp_cscale(real_in->image->data[i],1.0/(size_in*sqrt(sp_c3matrix_size(img->image))));
    }  
  }
}





void init_reconstruction(Options * opts){
  char dir[1024];
  int i;
  sp_init_fft(opts->nthreads);
  if(opts->real_image){
    opts->diffraction = sp_image_fft(opts->real_image);    
    opts->diffraction->scaled = 1;
    for(i = 0;i<sp_image_size(opts->diffraction);i++){
      opts->diffraction->mask->data[i] = 1;
    }
    if(opts->real_image->num_dimensions == SP_2D){
      char buffer[OPTION_STRING_SIZE*2+1];
      strcpy(buffer,opts->work_dir);
      strcat(buffer,"/");
      strcat(buffer,"real_image.png");
      sp_image_write(opts->real_image,buffer,COLOR_JET);
    }
    sp_image_dephase(opts->diffraction);
  }
  if(opts->diffraction){
    opts->amplitudes = sp_image_duplicate(opts->diffraction,SP_COPY_DATA|SP_COPY_MASK);
    sp_image_dephase(opts->diffraction);
    sp_image_to_amplitudes(opts->amplitudes);
  }else{
    fprintf(stderr,"Error: either real_image_file or amplitudes_file have to be specified!\n");
    exit(1);
  }
  if(sp_image_z(opts->amplitudes) == 1){
    sp_image_high_pass(opts->amplitudes, opts->beamstop, SP_2D);
  }else{
    sp_image_high_pass(opts->amplitudes, opts->beamstop, SP_3D);
  }
  sp_add_noise(opts->amplitudes,opts->noise,SP_GAUSSIAN);
/*  if(opts->rescale_amplitudes){
    rescale_image(opts->amplitudes);
  }*/
  char buffer[OPTION_STRING_SIZE*2+1];
  strcpy(buffer,opts->work_dir);
  strcat(buffer,"/");
  strcat(buffer,"diffraction.vtk");
  
  sp_image_write(opts->amplitudes,buffer,0);

  if(!opts->init_support_filename[0]){
    opts->init_support = get_support_from_patterson(opts->amplitudes,opts);
  }else{
    opts->init_support = sp_image_read(opts->init_support_filename,0);
  }

  if(!opts->solution_filename[0]){
    opts->solution_image = NULL;
  }else{
    opts->solution_image = sp_image_read(opts->solution_filename,0);
  }


  /* Try to estimate standard deviations */
  get_intensities_noise(opts);

  /* Make sure to scale everything to the same size if needed */
  harmonize_sizes(opts);
  
  if(opts->support_mask){
    sp_image_add(opts->init_support,opts->support_mask);
  }

  for(i = 0;i<sp_c3matrix_size(opts->init_support->image);i++){
    if(sp_real(opts->init_support->image->data[i])){
      opts->init_support->image->data[i] = sp_cinit(1,0);
    }
  }

  /* Set the initial guess */
  if(opts->image_guess_filename[0]){
    opts->image_guess = sp_image_read(opts->image_guess_filename,0);
  }else{
    opts->image_guess = sp_image_duplicate(opts->init_support,SP_COPY_DATA|SP_COPY_MASK);
  }


  /* Set random phases if needed */
  if(opts->rand_intensities){
    set_rand_ints(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      fprintf(stderr,"Warning: Using random intensities, with image guess. Think about it...\n");
    }
  }
  if(opts->rand_phases == PHASES_RANDOM){
    set_rand_phases(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      fprintf(stderr,"Warning: Using random phases, with image guess. Think about it...\n");
    }
  }else if(opts->rand_phases == PHASES_ZERO){
    set_zero_phases(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      fprintf(stderr,"Warning: Using zero phases, with image guess. Think about it...\n");
    }
  }


  if(opts->rescale_amplitudes){
    enforce_parsevals_theorem(opts->amplitudes,opts->image_guess);
  }
  

  if(opts->automatic){
    for(i = 0;i<1000;i++){
      sprintf(dir,"%s/%04d/",opts->work_dir,i);
      if(opts->genetic_optimization){
/*	genetic_reconstruction(img, initial_support, exp_sigma, opts,dir)*/;
      }else{
	complete_reconstruction(opts->amplitudes, opts->init_support, opts->intensities_std_dev, opts,dir);
      }
    }
  }else{
    sprintf(dir,"%s/",opts->work_dir); 
    if(opts->genetic_optimization){
/*      genetic_reconstruction(img, initial_support, opts->amplitudes_sigma, opts,dir)*/;
    }else{
      complete_reconstruction(opts->amplitudes, opts->init_support, opts->intensities_std_dev, opts,dir);
    }
  }

}




