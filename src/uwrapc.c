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
#include "rpcdefaultport.h"
#include "io_utils.h"

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
   to the last real_space and support_ respectively */

/*void continue_reconstruction(Options * opts){
  
  } */


void synchronize_image_data(Image **  real_space, Image ** support){
  if(global_options.current_real_space_image != real_space){
    sp_image_free(*real_space);
    *real_space = *global_options.current_real_space_image;
    sp_free(global_options.current_real_space_image);
    global_options.current_real_space_image = real_space;
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
    hawk_warning("Cannot enforce parsevals_theorem on images of different sizes!");
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
  exp = opts->amplitudes;
  tmp = opts->support_mask;
  if(tmp &&
     (sp_c3matrix_x(tmp->image) != sp_c3matrix_x(exp->image) ||
      sp_c3matrix_y(tmp->image) != sp_c3matrix_y(exp->image) ||
      sp_c3matrix_z(tmp->image) != sp_c3matrix_z(exp->image))){
    hawk_warning("Rescaling support_mask");
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
    hawk_warning("Rescaling init_support");
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
    hawk_warning("Rescaling image_guess");
    tmp = fourier_rescale(opts->image_guess,sp_c3matrix_x(exp->image),sp_c3matrix_y(exp->image),sp_c3matrix_z(exp->image));
    sp_image_free(opts->image_guess);
    opts->image_guess = tmp;
    hawk_image_write(tmp,"rescaled_guess.png",SpColormapJet);
  }
}

void output_initial_images( const Image * real_in,const Image * initial_support){
  hawk_image_write(initial_support,"initial_support.h5",0);
  hawk_image_write(real_in,"initial_guess.h5",sizeof(real));
}

void complete_reconstruction_clean(Image * amp, Image * initial_support, Image * exp_sigma,
			     Options * opts, char * dir){  

  SpPhasingAlgorithm * alg = NULL;
  Log log;
  init_log(&log);
  opts->flog = NULL;
  opts->cur_iteration = 0;

  SpPhasingConstraints phasing_constraints = SpNoConstraints;
  if(opts->enforce_real && opts->enforce_positivity){
    phasing_constraints |= SpPositiveRealObject;
  }else if(opts->enforce_real){
    phasing_constraints |= SpRealObject;
  }else if(opts->enforce_positivity){
    phasing_constraints |= SpPositiveComplexObject;
  }
  if(opts->enforce_centrosymmetry){
    phasing_constraints |= SpCentrosymmetricObject;
  }
  if(get_algorithm(opts,&log) == HIO){
    alg = sp_phasing_hio_alloc(opts->beta_evolution,phasing_constraints);
  }
  if(get_algorithm(opts,&log) == RAAR){
    alg = sp_phasing_raar_alloc(opts->beta_evolution,phasing_constraints);
  }
  if(get_algorithm(opts,&log) == DIFF_MAP){
    alg = sp_phasing_diff_map_alloc(opts->beta_evolution,get_gamma1(opts),get_gamma2(opts),phasing_constraints);
  }
  SpSupportArray * sup_alg = NULL;
  if(opts->support_update_algorithm == FIXED){
    sup_alg = sp_support_array_init(sp_support_threshold_alloc(opts->support_blur_evolution,opts->threshold_evolution),
				    opts->iterations);
  }
  if(opts->support_update_algorithm == DECREASING_AREA){
    sup_alg = sp_support_array_init(sp_support_area_alloc(opts->support_blur_evolution,opts->object_area_evolution),
				    opts->iterations);;
  }
  if (opts->support_update_algorithm == TEMPLATE_AREA){
    sup_alg = sp_support_array_init(sp_support_template_alloc(opts->init_support,opts->template_blur_radius,
							      opts->template_area_evolution),opts->iterations);
  }
  if (opts->support_update_algorithm == STATIC){
    sup_alg = sp_support_array_init(sp_support_static_alloc(),opts->iterations);
  }
  if(!alg || !sup_alg){
    hawk_fatal("Algorithm is NULL!\nBlame the programmer!");
  }
  if (opts->support_closure_radius > 0) {
    sp_support_array_append(sup_alg,sp_support_close_alloc(opts->support_closure_radius));
  }
  SpPhaser * ph = sp_phaser_alloc();
  sp_phaser_init(ph,alg,sup_alg,SpEngineAutomatic);
  sp_phaser_set_amplitudes(ph,amp);
  sp_phaser_init_model(ph,opts->image_guess,0);
  //sp_phaser_init_support(ph,initial_support,0,0);
  if (opts->init_support == NULL) {
    sp_phaser_init_support(ph,initial_support,SpSupportFromPatterson,opts->init_level);
  } else {
    sp_phaser_init_support(ph,initial_support,0,0);
  }
  output_initial_images(sp_phaser_model(ph),initial_support);
  while(opts->max_iterations == 0 || opts->cur_iteration < opts->max_iterations){
    char buffer[1024];
    int to_output = opts->output_period-(ph->iteration)%opts->output_period;
    int to_log = opts->log_output_period-(ph->iteration)%opts->log_output_period;
    int to_iterate = sp_min(to_output,to_log);
    sp_phaser_iterate(ph,to_iterate);
    opts->cur_iteration = ph->iteration;
    if(to_iterate == to_log){
      output_from_phaser(ph,opts,&log);
    }
    if(to_iterate == to_output){
      sprintf(buffer,"real_space-%07d.h5",ph->iteration-1);
      //hawk_image_write(sp_phaser_model(ph),buffer,opts->output_precision);
      hawk_image_write(sp_phaser_model_with_support(ph),buffer,opts->output_precision);
      sprintf(buffer,"support-%07d.h5",ph->iteration-1);
      hawk_image_write(sp_phaser_support(ph),buffer,opts->output_precision);
      sprintf(buffer,"fourier_space-%07d.h5",ph->iteration-1);
      hawk_image_write(sp_phaser_fmodel_with_mask(ph),buffer,opts->output_precision);
    }
  }
  
}

void complete_reconstruction(Image * amp, Image * initial_support, Image * exp_sigma,
			     Options * opts, char * dir){
  Image * support = NULL;
  Image * prev_support = NULL;
  Image * real_in = NULL;
  Image * real_space = NULL;
  Image * tmp = NULL;
  Image * tmp2;
  Image * real_space_sum = sp_image_alloc(sp_image_x(initial_support),sp_image_y(initial_support),sp_image_z(initial_support));
  int real_space_sum_n = 0;
  global_options.current_support = &support;
  global_options.current_real_space_image = &real_space;
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

  if((get_algorithm(opts,&log) == HIO || get_algorithm(opts,&log) == RAAR || get_algorithm(opts,&log) == DIFF_MAP) &&
     (opts->support_update_algorithm == FIXED ||
      opts->support_update_algorithm == DECREASING_AREA ||
      opts->support_update_algorithm == TEMPLATE_AREA ||
      opts->support_update_algorithm == STATIC) && opts->support_image_averaging == 1){ 
    /* use new libspimage backend */
    printf("use clean reconstruction\n");
    return complete_reconstruction_clean(amp,initial_support,exp_sigma,
				  opts,dir);
  }else if(sp_cuda_get_device_type() == SpCUDAHardwareDevice  ||
	   sp_cuda_get_device_type() == SpCUDAEmulatedDevice){
    hawk_warning("Cannot use CUDA for this particular configuration.");
  }


  init_log(&log);

  log.threshold = support_threshold;
  opts->cur_iteration = 0;
  opts->reconstruction_finished = 0;
  opts->flog = NULL;
  if(opts->automatic){
    opts->algorithm = HIO;
  }
  
  /* clear real_space_sum */
  real_space_sum_n = 0;
  for(int i = 0;i<sp_image_size(real_space_sum);i++){
    sp_real(real_space_sum->image->data[i]) = 0;
    sp_imag(real_space_sum->image->data[i]) = 0;
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

  hawk_image_write(initial_support,"support.vtk",SP_3D);
  prev_support = sp_image_duplicate(initial_support,SP_COPY_DATA|SP_COPY_MASK);


  real_in = sp_image_duplicate(opts->image_guess,SP_COPY_DATA|SP_COPY_MASK);
  hawk_image_write(real_in,"initial_guess.vtk",SP_3D);

  if(amp->num_dimensions == SP_2D){
    tmp2 = sp_image_shift(amp);
    hawk_image_write(tmp2,"initial_support.png",SpColormapJet);
    sp_image_free(tmp2);
  }
  hawk_image_write(initial_support,"initial_support.h5",0);

  /* make sure we make the input complex */
  sp_image_rephase(real_in,SP_ZERO_PHASE);
  

  if(real_in->num_dimensions == SP_2D){
    hawk_image_write(support,"support.png",SpColormapGrayScale);
    hawk_image_write(real_in,"initial_guess.png",SpColormapJet);
    hawk_image_write(real_in,"initial_guess.h5",sizeof(real));
    hawk_image_write(initial_support,"initial_support.png",SpColormapGrayScale);
  }else if(real_in->num_dimensions == SP_3D){
    hawk_image_write(support,"support.vtk",0);
  }

  if(get_algorithm(opts,&log) == HIO){     
    real_space = basic_hio_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR){
    real_space = basic_raar_iteration(amp,opts->intensities_std_dev, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HPR){
    real_space = basic_hpr_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == CFLIP){
    real_space = basic_cflip_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == ESPRESSO) {
    real_space = basic_espresso_iteration(amp, real_in, support, opts, &log);
  }else if(get_algorithm(opts,&log) == HAAR){
    real_space = basic_haar_iteration(amp, NULL, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == SO2D){
    real_space = basic_so2d_iteration(amp, NULL, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR_PROJ){
    real_space = basic_raar_proj_iteration(amp, opts->intensities_std_dev, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HIO_PROJ){
    real_space = basic_hio_proj_iteration(amp, opts->intensities_std_dev, real_in, support, opts, &log);
  }else if(get_algorithm(opts,&log) == DIFF_MAP){
    real_space = serial_difference_map_iteration(amp,real_in, support,opts,&log);
  }else{
    hawk_fatal("Error: Undefined algorithm!");
  }

  radius = opts->max_blur_radius;


  /* Change the status to running */
  opts->is_running = 1;
    
  for(;!opts->reconstruction_finished && (!opts->max_iterations || opts->cur_iteration < opts->max_iterations);opts->cur_iteration++){
    /* I'm only going to allow changes to images in the beggining of each iteration */
    synchronize_image_data(&real_space,&support);

    /* Add current real_space to the average real_space*/
    if(opts->iterations-opts->cur_iteration%opts->iterations <= opts->support_image_averaging){
      sp_image_add(real_space_sum,real_space);
      real_space_sum_n++;
    }

    if(opts->image_blur_period && opts->cur_iteration%opts->image_blur_period == opts->image_blur_period-1){
      sp_image_free(real_in);
      real_in = sp_gaussian_blur(real_space,opts->image_blur_radius);
      sp_image_memcpy(real_space,real_in);
    }

    if(opts->iterations && opts->cur_iteration%opts->iterations == opts->iterations-1){
      sp_image_scale(real_space_sum,1.0/real_space_sum_n);
      for(i = 0;i<opts->error_reduction_iterations_after_loop;i++){
	sp_image_free(real_in);
	real_in = real_space;
	real_space = basic_error_reduction_iteration(amp, real_in, support,opts,&log);
      }
      if(get_phases_blur_radius(opts)){
	phase_smoothening_iteration(real_space,opts,&log);
      }
      sp_image_free(prev_support);
      prev_support = sp_image_duplicate(support,SP_COPY_DATA|SP_COPY_MASK);
      sp_image_free(support);
      if (opts->support_update_algorithm == TEMPLATE_AREA) {
	support = get_support_from_initial_support(opts);
      } else {
	support_threshold = get_support_level(real_space_sum,&support_size,radius,&log,opts);
	log.threshold = support_threshold;
	if(support_threshold > 0){
	  support =  get_updated_support(real_space_sum,support_threshold, radius,opts);
	}else{
	  if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
	    exit(0);
	  }else{
	    abort();
	  }
	}
      }
      if(opts->filter_intensities){
	filter_intensities_with_support(amp,real_space_sum,support,opts);
      }
      if(opts->cur_iteration <= opts->iterations_to_min_blur){
	radius = get_blur_radius(opts);
      }
      /* clear real_space_sum */
      real_space_sum_n = 0;
      for(int i = 0;i<sp_image_size(real_space_sum);i++){
	sp_real(real_space_sum->image->data[i]) = 0;
 	sp_imag(real_space_sum->image->data[i]) = 0;
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
	sprintf(buffer,"real_space-%07d.png",opts->cur_iteration);
	hawk_image_write(real_space,buffer,SpColormapJet);
	sprintf(buffer,"real_space_phase-%07d.png",opts->cur_iteration);
	//	hawk_image_write(real_space,buffer,SpColormapWheel|COLOR_WEIGHTED_PHASE);
	hawk_image_write(real_space,buffer,SpColormapWheel|SpColormapPhase);
	sprintf(buffer,"support-%07d.png",opts->cur_iteration);
	hawk_image_write(support,buffer,SpColormapGrayScale);
      }
      if(real_in->num_dimensions == SP_3D){
	sprintf(buffer,"real_space-%07d.vtk",opts->cur_iteration);
	hawk_image_write(real_space,buffer,0);
	sprintf(buffer,"support-%07d.vtk",opts->cur_iteration);
	hawk_image_write(support,buffer,0);
      }
      sprintf(buffer,"real_space-%07d.h5",opts->cur_iteration);
      hawk_image_write(real_space,buffer,opts->output_precision);
      sprintf(buffer,"support-%07d.h5",opts->cur_iteration);
      hawk_image_write(support,buffer,opts->output_precision);
		
      tmp = sp_image_duplicate(real_space,SP_COPY_DATA|SP_COPY_MASK);
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
	if(sp_real(support->image->data[i])){
	  ;
	}else{
	  tmp->image->data[i] = sp_cinit(0,0);
	}
      }
      sprintf(buffer,"pre_fourier_space-%07d.h5",opts->cur_iteration);
      /*      tmp = zero_pad_image(tmp,sp_cmatrix_cols(tmp->image)*4,sp_cmatrix_rows(tmp->image)*4,1);
      hawk_image_write(tmp,buffer,opts->output_precision);
      sprintf(buffer,"pre_fourier_space-%07d.png",opts->cur_iteration);
      hawk_image_write(tmp,buffer,SpColormapJet|LOG_SCALE);
      */
      tmp2 = sp_image_fft(tmp); 
      sp_image_free(tmp);
      tmp = tmp2;
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
	tmp->mask->data[i] = 1;
      }
      sprintf(buffer,"fourier_space-%07d.h5",opts->cur_iteration);
      hawk_image_write(tmp,buffer,opts->output_precision);
      tmp2 = sp_image_shift(tmp);

      if(tmp2->num_dimensions == SP_2D){
	sprintf(buffer,"fourier_space-%07d.png",opts->cur_iteration);
	hawk_image_write(tmp2,buffer,SpColormapJet);
	sp_image_free(tmp2);
      }

      /*      sprintf(buffer,"fourier_space-%07d.vtk",opts->cur_iteration);
      hawk_image_write(tmp,buffer,SP_3D);
      sp_image_free(tmp);*/

    }    
    if(opts->break_centrosym_period && 
       opts->cur_iteration%opts->break_centrosym_period == opts->break_centrosym_period-1){
      centrosym_break_attempt(real_space);
    }
    sp_image_free(real_in);
    real_in = real_space;
    if(get_algorithm(opts,&log) == HIO){     
      real_space = basic_hio_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == RAAR){     
      real_space = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == HPR){     
      real_space = basic_hpr_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == CFLIP){     
      real_space = basic_cflip_iteration(amp, real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == ESPRESSO){
      real_space = basic_espresso_iteration(amp, real_in, support, opts, &log);
    }else if(get_algorithm(opts,&log) == HAAR){     
      real_space = basic_haar_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == SO2D){     
      real_space = basic_so2d_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == RAAR_PROJ){ 
      real_space = basic_raar_proj_iteration(amp, opts->intensities_std_dev,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == HIO_PROJ){
      real_space = basic_hio_proj_iteration(amp, opts->intensities_std_dev, real_in, support, opts, &log);
    }else if(get_algorithm(opts,&log) == DIFF_MAP){     
      real_space = serial_difference_map_iteration(amp,real_in, support,opts,&log);
    }

  }  

  //hawk_image_write(real_space,"real_space_final.h5",opts->output_precision|SP_2D);
  //hawk_image_write(real_space,"real_space_final.png",SpColormapJet|SP_2D);
  //  hawk_image_write(real_space,"real_space_final.vtk",0);
  hawk_image_write(real_space,"real_space-final.h5",0);
  //sprintf(buffer,"support-final.png");
  //hawk_image_write(support,buffer,SpColormapJet|SP_2D);
  //sprintf(buffer,"support-final.h5");
  hawk_image_write(support,"support-final.h5",0);
  //sprintf(buffer,"support-final.vtk");
  //  hawk_image_write(support,buffer,opts->output_precision);
  tmp = sp_image_fft(real_space); 
  for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
    tmp->mask->data[i] = 1;
  }
  sprintf(buffer,"fourier_space-final.h5");
  //hawk_image_write(tmp,buffer,opts->output_precision);
  //  sprintf(buffer,"fourier_space-final.png");
  //hawk_image_write(tmp,buffer,SpColormapJet);
  //  sprintf(buffer,"fourier_space-final.vtk");
  hawk_image_write(tmp,buffer,0);
  sp_image_free(tmp);
  
  //hawk_image_write(real_space,"phases_out_final.png",SpColormapPhase|SpColormapJet);
  sp_image_free(support);
  sp_image_free(prev_support);
  sp_image_free(real_in);
  sp_image_free(real_space);
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
  r->shifted = 1;
  sp_image_free(tmp);
  tmp = sp_image_shift(r);  
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    real_in->image->data[i] = sp_cscale(tmp->image->data[i],1.0/(sp_image_size(r)));
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
  r->shifted = 1;
  sp_image_free(tmp);
  tmp = sp_image_shift(r);    
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    real_in->image->data[i] = sp_cscale(tmp->image->data[i],1.0/(sp_image_size(tmp)));
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
      hawk_image_write(opts->real_image,buffer,SpColormapJet);
    }
    sp_image_dephase(opts->diffraction);
  }
  if(opts->diffraction){
    if(opts->diffraction->shifted == 0){
      opts->amplitudes = sp_image_shift(opts->diffraction);
    }else{
      opts->amplitudes = sp_image_duplicate(opts->diffraction,SP_COPY_DATA|SP_COPY_MASK);
    }
    sp_image_dephase(opts->diffraction);
    sp_image_to_amplitudes(opts->amplitudes);
  }else{
    hawk_fatal("Error: either real_image_file or intensities_file have to be specified!");
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
  if(opts->debug_level > 0){
    char buffer[OPTION_STRING_SIZE*2+1];
    strcpy(buffer,opts->work_dir);
    strcat(buffer,"/");
    strcat(buffer,"diffraction.vtk");
    
    hawk_image_write(opts->amplitudes,buffer,0);
    
    strcpy(buffer,opts->work_dir);
    strcat(buffer,"/");
    strcat(buffer,"debug_diffraction.h5");
  
    hawk_image_write(opts->amplitudes,buffer,0);
  }

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
    if(opts->realspace_starting_point == 0){
      set_rand_phases(opts->image_guess,opts->amplitudes);
    }else if(opts->realspace_starting_point == 1){
      set_rand_ints(opts->image_guess,opts->amplitudes);
    }else if(opts->realspace_starting_point == 2){
      set_zero_phases(opts->image_guess,opts->amplitudes);
    }else if(opts->realspace_starting_point == 3){
      /* no need to do anything */
    }else{
      hawk_fatal("Error: unexpected option value");
    }
  }


  /* Set random phases if needed */
  /*  if(opts->rand_intensities){
    set_rand_ints(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      hawk_warning("Warning: Using random intensities, with image guess. Think about it...");
    }
  }
  if(opts->rand_phases == PHASES_RANDOM){
    set_rand_phases(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      hawk_warning("Warning: Using random phases, with image guess. Think about it...");
    }
  }else if(opts->rand_phases == PHASES_ZERO){
    set_zero_phases(opts->image_guess,opts->amplitudes);
    if(opts->image_guess_filename[0]){
      hawk_warning("Warning: Using zero phases, with image guess. Think about it...");
    }
  }
  */

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


int uwrapc_network_main(int argc, char ** argv){
#ifndef NETWORK_SUPPORT
  hawk_fatal("uwrapc_network_main reached without network support!");
#else
  init_qt(argc,argv);
  char * server = 0;
  int server_port = 0;
  int key = 0;
  if(argc == 1){
    /* don't try to connect to any server*/
    return uwrapc_from_file(argc,argv);
  }else if(argc == 2){
    server = argv[1];
    server_port = rpcDefaultPort;
  }else if(argc == 3){
    server = argv[1];
    server_port = atoi(argv[2]);
  }else if(argc == 3){
    server = argv[1];
    server_port = atoi(argv[2]);
  }else if(argc == 4){
    server = argv[1];
    server_port = atoi(argv[2]);
    key = atoi(argv[3]);
  }else{
    printf("Usage: uwrapc [server [port [key]]]\n");
    return 0;
  }
  attempt_connection(server,server_port,key);  
  return start_event_loop();
#endif
}



int uwrapc_start(Options * opts){
  if(check_options_and_load_images(opts)){
    return -1;
  }
  char buffer[OPTION_STRING_SIZE*2+1];
#if defined(_MSC_VER) || defined(__MINGW32__)
  _mkdir(opts->work_dir);
#else
  mkdir(opts->work_dir,0755);
#endif
  strcpy(buffer,opts->work_dir);
  strcat(buffer,"/");
  strcat(buffer,"uwrapc.confout");
  write_options_file(buffer);
  srand(get_random_seed(opts));
  
  init_reconstruction(opts);
  /* cleanup stuff */
  if(opts->init_support){
    sp_image_free(opts->init_support);
  }
  if(opts->diffraction){
    sp_image_free(opts->diffraction);
  }
  if(opts->amplitudes){
    sp_image_free(opts->amplitudes);
  }
  if(opts->intensities_std_dev){
    sp_image_free(opts->intensities_std_dev);
  }
#ifdef _USE_DMALLOC
  dmalloc_shutdown();
#endif
  return 0;  
}

int uwrapc_from_file(){
  FILE * f;
  Options * opts = &global_options;
  f = fopen("uwrapc.conf","rb");
  if(f){
    fclose(f);
    read_options_file("uwrapc.conf");
  }else{
    perror("Could not open uwrapc.conf");
  }
  return uwrapc_start(opts);
}

int uwrapc_main(int argc, char ** argv){
#ifdef MPI 
  MPI_Init(&argc, &argv);
#endif    
  Options * opts = &global_options;
  set_defaults(opts);
#ifdef NETWORK_SUPPORT
  return uwrapc_network_main(argc,argv);
#else
  return uwrapc_from_file();
#endif
  return 0;
}
