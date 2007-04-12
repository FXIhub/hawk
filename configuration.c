#include <string.h>
#include <libconfig.h>
#include <stdlib.h>

#include "spimage.h"
#include "uwrapc.h"
#include "configuration.h"

Options * set_defaults(){
  Options * opt = calloc(1,sizeof(Options));
  opt->diffraction = NULL;
  opt->real_image = NULL;
  opt->max_blur_radius = (real)3;
  opt->init_level = (real)0.04;
  opt->beta = (real)0.9;
  opt->iterations = 20;
  opt->support_mask = NULL;
  opt->init_support = NULL;
  opt->image_guess = NULL;
  opt->noise = (real)0;
  opt->beamstop = (real)0;
  opt->new_level = (real)0.20;
  opt->min_blur = (real)0.7;
  opt->output_period = 100;
  opt->log_output_period = 20;
  opt->algorithm = HIO;
  opt->exp_sigma = (real)0.05;
  opt->rand_phases = 0;
  opt->rand_intensities = 0;
  opt->dyn_beta = (real)0;
  opt->cur_iteration = 0;
  opt->adapt_thres = 0;
  opt->automatic = 0;
  opt->support_update_algorithm = REAL_ERROR_ADAPTATIVE;
  opt->real_error_threshold = -1;
  opt->iterations_to_min_blur = 7000;
  opt->blur_radius_reduction_method = GAUSSIAN_BLUR_REDUCTION;
  opt->output_precision = sizeof(real);
  opt->error_reduction_iterations_after_loop = 0;
  strcpy(opt->work_dir,".");
  sprintf(opt->log_file,"uwrapc.log");
  opt->enforce_positivity = 0;
  opt->genetic_optimization = 0;
  opt->charge_flip_sigma = (real)0.05;
  opt->rescale_amplitudes = 1;
  opt->square_mask = 0;
  opt->blur_patterson = 0;
  opt->remove_central_pixel_phase = 0;
  opt->nthreads = 1;
  opt->break_centrosym_period = 0;
  opt->reconstruction_finished = 0;
  opt->real_error_tolerance = -1;
  return opt;
}

void read_options_file(char * filename, Options * res){
  config_t config;
  const char * tmp;
  config_init(&config);
  if(!config_read_file(&config,filename)){
    fprintf(stderr,"Error parsing %s on line %d:\n,%s\n",
	    filename,config_error_line(&config),config_error_text(&config));
    exit(1);
  }

  if((tmp = (char *)config_lookup_string(&config,"amplitudes_file"))){
    res->diffraction = sp_image_read(tmp,0);
    strcpy(res->diffraction_filename,tmp);
  }else if((tmp = config_lookup_string(&config,"real_image_file"))){
    res->real_image = sp_image_read(tmp,0);
    strcpy(res->real_image_filename,tmp);
  }
  
  if(config_lookup(&config,"initial_blur_radius")){
    res->max_blur_radius = config_lookup_float(&config,"initial_blur_radius");
  }
  if(config_lookup(&config,"patterson_threshold")){
    res->init_level = config_lookup_float(&config,"patterson_threshold");
  }
  if(config_lookup(&config,"beta")){
    res->beta = config_lookup_float(&config,"beta");
  }
  if(config_lookup(&config,"innerloop_iterations")){
    res->iterations = config_lookup_int(&config,"innerloop_iterations");
  }
  if((tmp = config_lookup_string(&config,"fixed_support_mask"))){
    res->support_mask = sp_image_read(tmp,0);
    strcpy(res->support_mask_filename,tmp);
  }
  if((tmp = config_lookup_string(&config,"initial_support"))){
    res->init_support = sp_image_read(tmp,0);
    strcpy(res->init_support_filename,tmp);
  }
  if((tmp = config_lookup_string(&config,"image_guess"))){
    res->image_guess = sp_image_read(tmp,0);
    strcpy(res->image_guess_filename,tmp);
  }
  if(config_lookup(&config,"added_noise")){
    res->noise = config_lookup_float(&config,"added_noise");
  }
  if(config_lookup(&config,"beamstop_radius")){
    res->beamstop = config_lookup_float(&config,"beamstop_radius");
  }
  
  if((tmp = config_lookup_string(&config,"support_update_algorithm"))){
    if(strcmp(tmp,"fixed") == 0){
      res->support_update_algorithm = FIXED;      
      if(config_lookup(&config,"support_intensity_threshold")){
	res->new_level = config_lookup_float(&config,"support_intensity_threshold");
      }
    }else if(strcmp(tmp,"stepped") == 0){
      res->support_update_algorithm = STEPPED;
      if(config_lookup(&config,"support_intensity_threshold")){
	res->new_level = config_lookup_float(&config,"support_intensity_threshold");
      }
    }else if(strcmp(tmp,"real_error_capped") == 0){
      res->support_update_algorithm = REAL_ERROR_CAPPED;
      if(config_lookup(&config,"support_intensity_threshold")){
	res->new_level = config_lookup_float(&config,"support_intensity_threshold");
      }
      if(config_lookup(&config,"support_real_error_threshold")){
	res->real_error_threshold = config_lookup_float(&config,"support_real_error_threshold");
      }
    }else if(strcmp(tmp,"real_error_adaptative") == 0){
      res->support_update_algorithm = REAL_ERROR_ADAPTATIVE;
      if(config_lookup(&config,"support_real_error_threshold")){
	res->real_error_threshold = config_lookup_float(&config,"support_real_error_threshold");
      }
    }else{
      fprintf(stderr,"Warning: Unrecongnized support update algorithm \"%s\". Using default.\n",tmp);
    }    

  }

  if((tmp = config_lookup_string(&config,"output_precision"))){
    if(strcmp(tmp,"float") == 0 || strcmp(tmp,"single") == 0){
      res->output_precision = sizeof(float);      
    }else if(strcmp(tmp,"double") == 0){
      res->output_precision = sizeof(double);
    }
  }
  if(config_lookup(&config,"iterations_to_min_blur")){
    res->iterations_to_min_blur = config_lookup_int(&config,"iterations_to_min_blur");
  }
  if((tmp = config_lookup_string(&config,"blur_radius_reduction_method"))){
    if(strcmp(tmp,"gaussian") == 0){
      res->blur_radius_reduction_method = GAUSSIAN_BLUR_REDUCTION;      
    }else if(strcmp(tmp,"geometrical") == 0){
      res->blur_radius_reduction_method = GEOMETRICAL_BLUR_REDUCTION;
    }
  }
  if(config_lookup(&config,"minimum_blur_radius")){
    res->min_blur = config_lookup_float(&config,"minimum_blur_radius");
  }
  if(config_lookup(&config,"enforce_reality")){
    res->enforce_real = config_lookup_int(&config,"enforce_reality");
  }
  if(config_lookup_string(&config,"logfile")){
    strcpy(res->log_file,config_lookup_string(&config,"logfile"));
  }
  if(config_lookup(&config,"output_period")){
    res->output_period = config_lookup_int(&config,"output_period");  
  }
  if(config_lookup(&config,"log_output_period")){
    res->log_output_period = config_lookup_int(&config,"log_output_period");  
  }
  if(config_lookup_string(&config,"algorithm")){
    tmp = config_lookup_string(&config,"algorithm");    
    if(strcmp(tmp,"RAAR") == 0|| strcmp(tmp,"raar") == 0){
      res->algorithm = RAAR;
    }else if(strcmp(tmp,"HIO") == 0|| strcmp(tmp,"hio") == 0){
      res->algorithm = HIO;
    }else if(strcmp(tmp,"HPR") == 0|| strcmp(tmp,"hpr") == 0){
      res->algorithm = HPR;
    }else if(strcmp(tmp,"CFLIP") == 0|| strcmp(tmp,"cflip") == 0){
      res->algorithm = CFLIP;
    }else if(strcmp(tmp,"RAAR_CFLIP") == 0|| strcmp(tmp,"raar_cflip") == 0){
      res->algorithm = RAAR_CFLIP;
    }else{
      fprintf(stderr,"Warning: Unrecongnized algorithm \"%s\". Using default.\n",tmp);
    }    
  }
  if(config_lookup(&config,"RAAR_sigma")){
    res->exp_sigma = config_lookup_float(&config,"RAAR_sigma");  
  }
  if(config_lookup(&config,"dynamic_beta")){
    res->dyn_beta = config_lookup_int(&config,"dynamic_beta");  
  }
  if(config_lookup(&config,"error_reduction_iterations_after_loop")){
    res->error_reduction_iterations_after_loop = 
      config_lookup_int(&config,"error_reduction_iterations_after_loop");  
  }
  if(config_lookup(&config,"random_initial_intensities")){
    res->rand_intensities = config_lookup_int(&config,"random_initial_intensities");  
  }
  if(config_lookup(&config,"random_initial_phases")){
    res->rand_phases = config_lookup_int(&config,"random_initial_phases");  
  }
  if(config_lookup(&config,"enforce_positivity")){
    res->enforce_positivity = config_lookup_int(&config,"enforce_positivity");  
  }
  if(config_lookup(&config,"genetic_optimization")){
    res->genetic_optimization = config_lookup_int(&config,"genetic_optimization");  
  }
  if(config_lookup(&config,"work")){
    strcpy(res->work_dir,config_lookup_string(&config,"work_directory"));  
  }  
  if(config_lookup(&config,"charge_flip_sigma")){
    res->charge_flip_sigma = config_lookup_float(&config,"charge_flip_sigma");  
  }
  if(config_lookup(&config,"rescale_amplitudes")){
    res->rescale_amplitudes = config_lookup_int(&config,"rescale_amplitudes");  
  }
  if(config_lookup(&config,"square_mask")){
    res->square_mask = config_lookup_float(&config,"square_mask");  
  }
  if(config_lookup(&config,"blur_patterson")){
    res->blur_patterson = config_lookup_int(&config,"blur_patterson");  
  }
  if(config_lookup(&config,"remove_central_pixel_phase")){
    res->remove_central_pixel_phase = config_lookup_int(&config,"remove_central_pixel_phase");  
  }
  if(config_lookup(&config,"perturb_weak_reflections")){
    res->perturb_weak_reflections = config_lookup_float(&config,"perturb_weak_reflections");  
  }
  if(config_lookup(&config,"nthreads")){
    res->nthreads = config_lookup_int(&config,"nthreads");
  }
  if(config_lookup(&config,"break_centrosym_period")){
    res->break_centrosym_period = config_lookup_int(&config,"break_centrosym_period");
  }
  if(config_lookup(&config,"real_error_tolerance")){
    res->real_error_tolerance = config_lookup_float(&config,"real_error_tolerance");  
  }
  if(config_lookup(&config,"max_iterations")){
    res->max_iterations = config_lookup_int(&config,"max_iterations");  
  }


}


void parse_options(int argc, char ** argv, Options * res){
}


void write_options_file(char * filename, Options * res){
  config_t config;
  config_setting_t * root;
  config_setting_t * s;
  config_init(&config);
  root = config_root_setting(&config);
  if(res->diffraction){
    s = config_setting_add(root,"amplitudes_file",CONFIG_TYPE_STRING);
    config_setting_set_string(s,res->diffraction_filename);
  }
  if(res->real_image){
    s = config_setting_add(root,"real_image_file",CONFIG_TYPE_STRING);
    config_setting_set_string(s,res->real_image_filename);
  }
  s = config_setting_add(root,"initial_blur_radius",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->max_blur_radius);
  s = config_setting_add(root,"patterson_threshold",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->init_level);
  s = config_setting_add(root,"beta",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->beta);
  s = config_setting_add(root,"innerloop_iterations",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->iterations);
  if(res->support_mask){
    s = config_setting_add(root,"fixed_support_mask",CONFIG_TYPE_STRING);
    config_setting_set_string(s,res->support_mask_filename);
  }
  if(res->init_support){
    s = config_setting_add(root,"initial_support",CONFIG_TYPE_STRING);
    config_setting_set_string(s,res->init_support_filename);
  }
  if(res->image_guess){
    s = config_setting_add(root,"image_guess",CONFIG_TYPE_STRING);
    config_setting_set_string(s,res->image_guess_filename);
  }
  s = config_setting_add(root,"added_noise",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->noise);
  s = config_setting_add(root,"beamstop_radius",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->beamstop);
  s = config_setting_add(root,"support_intensity_threshold",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->new_level);
/*  s = config_setting_add(root,"support_real_error_threshold",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->);*/
  s = config_setting_add(root,"iterations_to_min_blur",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->iterations_to_min_blur);
  s = config_setting_add(root,"minimum_blur_radius",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->min_blur); 
  s = config_setting_add(root,"enforce_reality",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->enforce_real);
  s = config_setting_add(root,"logfile",CONFIG_TYPE_STRING);
  config_setting_set_string(s,res->log_file);
  s = config_setting_add(root,"output_period",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->output_period);
  s = config_setting_add(root,"log_output_period",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->log_output_period);
  s = config_setting_add(root,"algorithm",CONFIG_TYPE_STRING);
  if(res->algorithm == RAAR){
    config_setting_set_string(s,"RAAR");
  }else if(res->algorithm == HIO){
    config_setting_set_string(s,"HIO");
  }else if(res->algorithm == HPR){
    config_setting_set_string(s,"HPR");
  }else if(res->algorithm == CFLIP){
    config_setting_set_string(s,"CFLIP");
  }else if(res->algorithm == RAAR_CFLIP){
    config_setting_set_string(s,"RAAR_CFLIP");
  }
  s = config_setting_add(root,"blur_radius_reduction_method",CONFIG_TYPE_STRING);
  if(res->blur_radius_reduction_method == GAUSSIAN_BLUR_REDUCTION){
    config_setting_set_string(s,"gaussian");
  }else if(res->blur_radius_reduction_method == GEOMETRICAL_BLUR_REDUCTION){
    config_setting_set_string(s,"geometrical");
  }else{
    fprintf(stderr,"Error: unkown blur_radius_reduction_method!\n");
    abort();
  }
  s = config_setting_add(root,"RAAR_sigma",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->exp_sigma);
  s = config_setting_add(root,"dynamic_beta",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->dyn_beta);
  s = config_setting_add(root,"random_initial_intensities",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->rand_intensities);
  s = config_setting_add(root,"random_initial_phases",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->rand_phases);
  s = config_setting_add(root,"work_directory",CONFIG_TYPE_STRING);
  config_setting_set_string(s,res->work_dir);


  s = config_setting_add(root,"support_update_algorithm",CONFIG_TYPE_STRING);
  if(res->support_update_algorithm == FIXED){
    config_setting_set_string(s,"fixed");
  }else if(res->support_update_algorithm == STEPPED){
    config_setting_set_string(s,"stepped");
  }else if(res->support_update_algorithm == REAL_ERROR_CAPPED){
    config_setting_set_string(s,"real_error_capped");
  }else if(res->support_update_algorithm == REAL_ERROR_ADAPTATIVE){
    config_setting_set_string(s,"real_error_adaptative");
  }
  s = config_setting_add(root,"support_real_error_threshold",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->real_error_threshold);
  s = config_setting_add(root,"output_precision",CONFIG_TYPE_STRING);
  if(res->output_precision == sizeof(float)){
    config_setting_set_string(s,"single");
  }else if(res->output_precision == sizeof(double)){
    config_setting_set_string(s,"double");
  }

  s = config_setting_add(root,"error_reduction_iterations_after_loop",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->error_reduction_iterations_after_loop);

  s = config_setting_add(root,"enforce_positivity",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->enforce_positivity);

  s = config_setting_add(root,"genetic_optimization",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->genetic_optimization);

  s = config_setting_add(root,"charge_flip_sigma",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->charge_flip_sigma);

  s = config_setting_add(root,"rescale_amplitudes",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->rescale_amplitudes);

  s = config_setting_add(root,"square_mask",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->square_mask);

  s = config_setting_add(root,"blur_patterson",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->blur_patterson);

  s = config_setting_add(root,"remove_central_pixel_phase",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->remove_central_pixel_phase);

  s = config_setting_add(root,"perturb_weak_reflections",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->perturb_weak_reflections);

  s = config_setting_add(root,"nthreads",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->nthreads);

  s = config_setting_add(root,"break_centrosym_period",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->break_centrosym_period);

  s = config_setting_add(root,"real_error_tolerance",CONFIG_TYPE_FLOAT);
  config_setting_set_float(s,res->real_error_tolerance);

  s = config_setting_add(root,"max_iterations",CONFIG_TYPE_INT);
  config_setting_set_int(s,res->max_iterations);

  config_write_file(&config,filename);
}
