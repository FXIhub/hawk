#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

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
#include "uwrapc.h"
#include "configuration.h"
#include "algorithms.h"
#include "support.h"
#include "network_communication.h"



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



void rescale_image(Image * a){
  double sum = 0;
  int i;
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    sum += a->image->data[i];
  }
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    a->image->data[i] /= sum;
  } 
}

void centrosym_break_attempt(Image * a){
  int x,y,z;
  for(x = 0;x<sp_c3matrix_x(a->image);x++){
    for(y = 0;y<sp_c3matrix_y(a->image);y++){
      for(z = 0;z<sp_c3matrix_z(a->image);z++){
	a->image->data[z*sp_c3matrix_y(a->image)*sp_c3matrix_x(a->image)+
		       y*sp_c3matrix_x(a->image)+x] *= 0.5;
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
      if(creal(tmp->image->data[i]) < 1e-6){
	tmp->image->data[i] = 0;
      }else{
	tmp->image->data[i] = 1;
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
      if(creal(tmp->image->data[i]) < 1e-1){
	tmp->image->data[i] = 0;
      }else{
	tmp->image->data[i] = 1;
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
  sp_image_write(initial_support,"support.vtk",SP_3D);
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
  //sp_image_write(support,"support.png",COLOR_JET|SP_2D);
  //sp_image_write(real_in,"initial_guess.png",COLOR_JET|SP_2D);
  //sp_image_write(real_in,"initial_guess.h5",sizeof(real)|SP_2D);
  //sp_image_write(initial_support,"initial_support.png",COLOR_JET|SP_2D);
  sp_image_write(support,"support.vtk",0);

  if(get_algorithm(opts,&log) == HIO){     
    real_out = basic_hio_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == RAAR){
    real_out = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HPR){
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == CFLIP){
    real_out = basic_cflip_iteration(amp, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == HAAR){
    real_out = basic_haar_iteration(amp, exp_sigma, real_in, support,opts,&log);
  }else if(get_algorithm(opts,&log) == SO2D){
    real_out = basic_so2d_iteration(amp, exp_sigma, real_in, support,opts,&log);
  }else{
    fprintf(stderr,"Error: Undefined algorithm!\n");
    exit(-1);
  }

  radius = opts->max_blur_radius;
    
  for(;!opts->reconstruction_finished && (!opts->max_iterations || opts->cur_iteration < opts->max_iterations);opts->cur_iteration++){

    if(opts->image_blur_period && opts->cur_iteration%opts->image_blur_period == opts->image_blur_period-1){
      sp_image_free(real_in);
      real_in = gaussian_blur(real_out,opts->image_blur_radius);
      sp_image_memcpy(real_out,real_in);
    }

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
      //sp_image_write(real_out,buffer,opts->output_precision|SP_2D);
      sprintf(buffer,"real_out-%07d.png",opts->cur_iteration);
      //sp_image_write(real_out,buffer,COLOR_JET|SP_2D);
      sprintf(buffer,"real_out-%07d.vtk",opts->cur_iteration);
      sp_image_write(real_out,buffer,SP_3D);
      sprintf(buffer,"real_out_phase-%07d.png",opts->cur_iteration);
      //sp_image_write(real_out,buffer,COLOR_JET|COLOR_PHASE|SP_2D);
      sprintf(buffer,"support-%07d.png",opts->cur_iteration);
      //sp_image_write(support,buffer,COLOR_JET|SP_2D);
      sprintf(buffer,"support-%07d.h5",opts->cur_iteration);
      //sp_image_write(support,buffer,opts->output_precision|SP_2D);
      sprintf(buffer,"support-%07d.vtk",opts->cur_iteration);
      sp_image_write(support,buffer,SP_3D);
      tmp = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
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
      for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
	tmp->mask->data[i] = 1;
      }
      sprintf(buffer,"pattern-%07d.h5",opts->cur_iteration);
      //sp_image_write(tmp,buffer,opts->output_precision);
      sprintf(buffer,"pattern-%07d.png",opts->cur_iteration);
      //sp_image_write(tmp,buffer,COLOR_JET|LOG_SCALE);
      sprintf(buffer,"pattern-%07d.vtk",opts->cur_iteration);
      sp_image_write(tmp,buffer,SP_3D);
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
    }else if(get_algorithm(opts,&log) == HAAR){     
      real_out = basic_haar_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }else if(get_algorithm(opts,&log) == SO2D){     
      real_out = basic_so2d_iteration(amp, exp_sigma,real_in, support,opts,&log);
    }
  }  

  //sp_image_write(real_out,"real_out_final.h5",opts->output_precision|SP_2D);
  //sp_image_write(real_out,"real_out_final.png",COLOR_JET|SP_2D);
  sp_image_write(real_out,"real_out_final.vtk",SP_3D);
  //sprintf(buffer,"support-final.png");
  //sp_image_write(support,buffer,COLOR_JET|SP_2D);
  //sprintf(buffer,"support-final.h5");
  //sp_image_write(support,buffer,opts->output_precision|SP_2D);
  sprintf(buffer,"support-final.vtk");
  sp_image_write(support,buffer,opts->output_precision|SP_3D);
  tmp = sp_image_fft(real_out); 
  for(i = 0;i<sp_c3matrix_size(tmp->image);i++){
    tmp->mask->data[i] = 1;
  }
  sprintf(buffer,"pattern-final.h5");
  //sp_image_write(tmp,buffer,opts->output_precision);
  sprintf(buffer,"pattern-final.png");
  //sp_image_write(tmp,buffer,COLOR_JET);
  sprintf(buffer,"pattern-final.vtk");
  sp_image_write(tmp,buffer,SP_3D);
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
  fclose(opts->flog);
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
    real_in->image->data[i] = r->image->data[i]/(sp_image_size(tmp));
  }
  sp_image_free(tmp);
  sp_image_free(r);	  
}


void set_rand_ints(Image * real_in, Image * img){
  int i;
  real sum = 0;
  int size_in = 0;
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    sum += img->image->data[i];    
    if(real_in->image->data[i]){
      size_in++;
    }
  }
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    if(real_in->image->data[i]){
      real_in->image->data[i] = (p_drand48()*sum)+(p_drand48()*sum)*I;
      real_in->image->data[i]  /= (size_in*sqrt(sp_c3matrix_size(img->image)));
    }  
  }
}







int main(int argc, char ** argv){
  Image * img = NULL;
  Image * exp_sigma = NULL;
  char dir[1024];
  int i;
  FILE * f;
  Options * opts;
  void * socket = 0;
#ifdef MPI
  MPI_Init(&argc, &argv);
#endif  
  opts = set_defaults();

#ifdef NETWORK_SUPPORT
  /* Decide if should connect to hawk GUI or not */
  init_qt(argc,argv);
  char * server = 0;
  int server_port = 0;
  if(argc != 1){
    if(argc == 3 || argc == 5){
      for(i = 1;i<argc-1;i++){
	if(strcmp(argv[i],"-s") == 0){
	  server = argv[i+1];
	}else if(strcmp(argv[i],"-p") == 0){
	  server_port = atoi(argv[i+1]);
	}
      }
      socket = attempt_connection(server,server_port);
    }else{
      printf("Usage: uwrapc [-s server [-p port]]\n");
    }
  }
#else
  if(argc != 1){
    perror("uwrapc not compileed with network support!\n");
  }
#endif  
  srand(getpid());

  
  f = fopen("uwrapc.conf","rb");
  if(f){
    fclose(f);
    read_options_file("uwrapc.conf",opts);
    parse_options(argc,argv,opts);
    write_options_file("uwrapc.confout",opts);
  }else if(!socket){
    perror("Could not open uwrapc.conf");
  }
  if(socket){
    wait_for_server_instructions(socket);
  }
  sp_init_fft(opts->nthreads);
  if(opts->real_image){
    opts->diffraction = sp_image_fft(opts->real_image);    
    for(i = 0;i<sp_image_size(opts->diffraction);i++){
      opts->diffraction->mask->data[i] = 1;
    }
    //sp_image_write(opts->real_image,"real_image.png",COLOR_JET|SP_2D);
    sp_image_dephase(opts->diffraction);
  }
  if(opts->diffraction){
    img = sp_image_duplicate(opts->diffraction,SP_COPY_DATA|SP_COPY_MASK);
    sp_image_dephase(opts->diffraction);
    sp_image_to_amplitudes(img);
  }else{
    fprintf(stderr,"Error: either real_image_file or amplitudes_file have to be specified!\n");
    exit(1);
  }
  if(sp_image_z(img) == 1){
    sp_image_high_pass(img, opts->beamstop, SP_2D);
  }else{
    sp_image_high_pass(img, opts->beamstop, SP_3D);
  }
  sp_add_noise(img,opts->noise,SP_GAUSSIAN);
  if(opts->rescale_amplitudes){
    rescale_image(img);
  }
  //sp_image_write(img,"diffraction.png",COLOR_JET|SP_2D);

  if(!opts->init_support){
    opts->init_support = get_support_from_patterson(img,opts);
  }

  exp_sigma = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    exp_sigma->image->data[i] = img->image->data[i]*opts->exp_sigma;
  }

  /* Make sure to scale everything to the same size if needed */
  harmonize_sizes(opts);
  
  if(opts->support_mask){
    sp_image_add(opts->init_support,opts->support_mask);
  }

  for(i = 0;i<sp_c3matrix_size(opts->init_support->image);i++){
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
#ifdef NETWORK_SUPPORT
  cleanup_qt();
#endif

  return 0;  
}








