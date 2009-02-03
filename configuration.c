#include <string.h>
#include <libconfig.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include <unistd.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
#include <process.h>
#else
#include <sys/types.h>
#endif

#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif

#include "spimage.h"

#include "configuration.h"


Options global_options;


static char * get_path(const VariableMetadata * vm);
static int get_config_type(Variable_Type vt);


char * get_path(const VariableMetadata * vm){
  const VariableMetadata * parent = vm->parent;
  char * buffer1 = sp_malloc(sizeof(char)*OPTION_STRING_SIZE);
  char * buffer2 = sp_malloc(sizeof(char)*OPTION_STRING_SIZE);
  strcpy(buffer1,vm->variable_name);
  while(parent->id != Id_Root){
    strcpy(buffer2,buffer1);
    sprintf(buffer1,"%s.",parent->variable_name);
    strcat(buffer1,buffer2);
    parent = parent->parent;
  }
  sp_free(buffer2);
  return buffer1;
}

int get_config_type(Variable_Type vt){
  if(vt == Type_Real){
    return CONFIG_TYPE_FLOAT;
  }else if(vt == Type_Int){
    return CONFIG_TYPE_INT;
  }else if(vt == Type_Filename){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_Directory_Name){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_String){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_MultipleChoice){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_Bool){
    return CONFIG_TYPE_BOOL;
  }else if(vt == Type_Group){
    return CONFIG_TYPE_GROUP;
  }else if(vt == Type_Vector_Real){
    return CONFIG_TYPE_ARRAY;
  }else if(vt == Type_Vector_Int){
    return CONFIG_TYPE_ARRAY;
  }else if(vt == Type_Map_Real){
    return CONFIG_TYPE_LIST;
  }
  fprintf(stderr,"Unkown variable type\n");
  abort();
  return -1;
}

  
  
void set_defaults(Options * opt){
  opt->diffraction = NULL;
  opt->diffraction_filename[0] = 0;
  opt->real_image = NULL;
  opt->real_image_filename[0] = 0;
  opt->max_blur_radius = (real)3;
  opt->init_level = (real)0.04;
  opt->beta = (real)0.9;
  opt->iterations = 20;
  opt->support_mask = NULL;
  opt->support_mask_filename[0] = 0;
  opt->init_support = NULL;
  opt->init_support_filename[0] = 0;
  opt->image_guess = NULL;
  opt->image_guess_filename[0] = 0;
  opt->noise = (real)0;
  opt->beamstop = (real)0;
  opt->new_level = (real)0.20;
  opt->min_blur = (real)0.7;
  opt->output_period = 100;
  opt->log_output_period = 20;
  opt->algorithm = HIO;
  opt->exp_sigma = (real)0.0;
  opt->intensities_std_dev_filename[0] = 0;
  opt->autocorrelation_support_filename[0] = 0;
  opt->rand_phases = 0;
  opt->rand_intensities = 0;
  opt->dyn_beta = (real)0;
  opt->cur_iteration = 0;
  opt->adapt_thres = 0;
  opt->automatic = 0;
  opt->support_update_algorithm = DECREASING_AREA;
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
  opt->patterson_blur_radius = (real)0;
  opt->remove_central_pixel_phase = 0;
  opt->nthreads = 1;
  opt->break_centrosym_period = 0;
  opt->reconstruction_finished = 0;
  opt->real_error_tolerance = -1;
  opt->max_iterations = 0;
  opt->object_area = 0;
  opt->patterson_level_algorithm = FIXED;
  opt->image_blur_radius = 0.0;
  opt->image_blur_period = 0;
  opt->min_object_area = 0;
  opt->iterations_to_min_object_area = 0;
  opt->is_running = 0;
  opt->solution_filename[0] = 0;
  opt->phases_min_blur_radius = 0;
  opt->phases_max_blur_radius = 0;
  opt->iterations_to_min_phases_blur = 0;
  opt->filter_intensities = 0;
  opt->object_area_checkpoints = NULL;
  opt->object_area_at_checkpoints = NULL;
  opt->beta_checkpoints = NULL;
  opt->beta_at_checkpoints = NULL;
  opt->gamma1 = -10000;
  opt->gamma2 = -10000;
  opt->support_image_averaging = 1;
  opt->random_seed = -1;
  opt->autocorrelation_area = 0.1;
  opt->beta_evolution = sp_smap_alloc(2);
  sp_smap_insert(opt->beta_evolution,0,0.9);
  sp_smap_insert(opt->beta_evolution,2000,0.9);
  opt->object_area_evolution = sp_smap_alloc(2);
  sp_smap_insert(opt->object_area_evolution,0,0.001);
  sp_smap_insert(opt->object_area_evolution,2000,0.001);
  opt->support_blur_evolution = sp_smap_alloc(2);
  sp_smap_insert(opt->support_blur_evolution,0,3.0);
  sp_smap_insert(opt->support_blur_evolution,5000,0.7);
  opt->phases_blur_evolution = sp_smap_alloc(2);
  sp_smap_insert(opt->phases_blur_evolution,0,0);
  sp_smap_insert(opt->phases_blur_evolution,5000,0);
}

void read_options_file(char * filename){
  config_t config;
  config_init(&config);
  if(!config_read_file(&config,filename)){
    fprintf(stderr,"Error parsing %s on line %d:\n,%s\n",
	    filename,config_error_line(&config),config_error_text(&config));
    exit(1);
  }


  /* Loop around all the options and put them inside their groups */
  for(int i = 0; i< number_of_global_options; i++){
    if(variable_metadata[i].variable_type != Type_Group && (variable_metadata[i].variable_properties & isSettableBeforeRun)){
      char * path = get_path(&(variable_metadata[i]));   
      if(config_lookup(&config,path)){	
	if(variable_metadata[i].variable_type == Type_String||
	   variable_metadata[i].variable_type == Type_Filename ||
	   variable_metadata[i].variable_type == Type_Directory_Name){
	  strcpy((char *)variable_metadata[i].variable_address,config_lookup_string(&config,path));
	}else if(variable_metadata[i].variable_type == Type_Int){
	  *((int *)variable_metadata[i].variable_address) = config_lookup_int(&config,path);
	}else if(variable_metadata[i].variable_type == Type_Real){
	  if(config_setting_type(config_lookup(&config,path)) == CONFIG_TYPE_FLOAT){
	    *((real *)variable_metadata[i].variable_address) = config_lookup_float(&config,path);
	  }else{
	    *((real *)variable_metadata[i].variable_address) = config_lookup_int(&config,path);
	  }
	}else if(variable_metadata[i].variable_type == Type_Vector_Real || variable_metadata[i].variable_type == Type_Vector_Int ){
	  if(config_setting_type(config_lookup(&config,path)) == CONFIG_TYPE_ARRAY){
	    int length = config_setting_length(config_lookup(&config,path));	    
	    if(length){
	      int type = config_setting_type(config_setting_get_elem(config_lookup(&config,path),0));
	      if(type == CONFIG_TYPE_FLOAT){
		*((sp_vector **)variable_metadata[i].variable_address) = sp_vector_alloc(length);
		sp_vector * v = *((sp_vector **)variable_metadata[i].variable_address);
		for(int j = 0;j<config_setting_length(config_lookup(&config,path));j++){
		  sp_vector_set(v,j, config_setting_get_float_elem(config_lookup(&config,path),j));
		}
	      }else if(type == CONFIG_TYPE_INT){
		*((sp_vector **)variable_metadata[i].variable_address) = sp_vector_alloc(length);
		sp_vector * v = *((sp_vector **)variable_metadata[i].variable_address);
		for(int j = 0;j<config_setting_length(config_lookup(&config,path));j++){
		  sp_vector_set(v,j, config_setting_get_int_elem(config_lookup(&config,path),j));
		}
		
	      }
	    }
	  }
	}else if(variable_metadata[i].variable_type == Type_Bool){
	  if(config_setting_type(config_lookup(&config,path)) == CONFIG_TYPE_BOOL){
	    *((int *)variable_metadata[i].variable_address) = config_lookup_bool(&config,path);
	  }else{
	    *((int *)variable_metadata[i].variable_address) = config_lookup_int(&config,path);
	  }

	}else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	  /* Change string to lowercase for comparison */
	  char buffer[OPTION_STRING_SIZE];
	  strcpy(buffer,config_lookup_string(&config,path));
	  for(unsigned int j = 0;j<strlen(buffer);j++){
	    buffer[j] = tolower(buffer[j]);
	  }
	  /* Compare with list of known good strings */
	  for(int j = 0;;j++){
	    if(variable_metadata[i].list_valid_names[j] == NULL){
	      fprintf(stderr,"Error invalid list value %s for option %s\n",
		      config_lookup_string(&config,path),
		      variable_metadata[i].variable_name);
	      abort();
	    }
	    if(strcmp(variable_metadata[i].list_valid_names[j],buffer) == 0){
	      *((int *)variable_metadata[i].variable_address) = variable_metadata[i].list_valid_values[j];
	      break;
	    }
	  }
	}else if(variable_metadata[i].variable_type == Type_Map_Real){
	  /* FM: this is all wrong as the children are nameless*/
	  char * path = get_path(&(variable_metadata[i]));   
	  if(config_lookup(&config,path)){	
	    config_setting_t * keys = config_setting_get_elem(config_lookup(&config,path),0);
	    config_setting_t * values = config_setting_get_elem(config_lookup(&config,path),1);	   
	    int length = config_setting_length(keys);
	    if(length){
	      int type = config_setting_type(config_setting_get_elem(keys,0));
	      if(type == CONFIG_TYPE_FLOAT){
		*((sp_smap **)variable_metadata[i].variable_address) = sp_smap_alloc(length);
		sp_smap * m = *((sp_smap **)variable_metadata[i].variable_address);
		for(int j = 0;j<config_setting_length(keys);j++){
		  sp_smap_insert(m, config_setting_get_float_elem(keys,j),config_setting_get_float_elem(values,j));
		}
	      }else{
		sp_error_fatal("Map datatype not supported!");
	      }
	    }
	  }else{
	    sp_error_fatal("Keys field missing in Map Type!");	    
	  }
	}else{
	  fprintf(stderr,"Variable type not yet supported!\n");	
	  abort();
	}
      }
      sp_free(path);
    }
  }

  /* Make sure option is set and is not empty */
  if(global_options.diffraction_filename && strcmp(global_options.diffraction_filename,"")){    
    global_options.diffraction = sp_image_read(global_options.diffraction_filename,0);
  }else if(global_options.real_image_filename  && strcmp(global_options.real_image_filename,"")){
    global_options.real_image = sp_image_read(global_options.real_image_filename,0);
  }else{
    sp_error_fatal("Neither diffraction nor real image specified!");
  }
  
  if(global_options.support_mask_filename  && strcmp(global_options.support_mask_filename,"")){
    global_options.support_mask = sp_image_read(global_options.support_mask_filename,0);
  }
}


/*
  This function writes a config file where groups are always children of ROOT.
   It does *not* support nested groups!
 */
void write_options_file(const char * filename){
  config_t config;
  config_setting_t * root;
  config_setting_t * s;
  config_init(&config);
  root = config_root_setting(&config);
  int n_groups = 0;
  /* first calculate the total number of groups that exist */
  for(int i = 0; i< number_of_global_options; i++){
    if(variable_metadata[i].variable_type == Type_Group && variable_metadata[i].id != Id_Root){
      n_groups++;
    }
  }
  /* Now create all groups */
  while(n_groups){
    for(int i = 0; i< number_of_global_options; i++){      
      if(variable_metadata[i].variable_type == Type_Group && 
	 variable_metadata[i].id != Id_Root){
	/* it's a group */
	if(config_lookup(&config,variable_metadata[i].variable_name) == NULL){
	  /* it still doesn't exist */
	  if((s = config_lookup(&config,variable_metadata[i].parent->variable_name)) != NULL){
	    /* parent exists */
	    config_setting_add(s,variable_metadata[i].variable_name,get_config_type(variable_metadata[i].variable_type));
	    n_groups--;
	  }
	  if(variable_metadata[i].parent->id == Id_Root){
	    /* parent is Root */
	    config_setting_add(root,variable_metadata[i].variable_name,get_config_type(variable_metadata[i].variable_type));
	    n_groups--;
	  }
	}
      }
    }
  }
	  
  /* Loop around all the options and put them inside their groups */
  for(int i = 0; i< number_of_global_options; i++){
    config_setting_t * parent;
    if(variable_metadata[i].variable_type != Type_Group && 
       (variable_metadata[i].variable_properties & isSettableBeforeRun) && 
       ((variable_metadata[i].variable_properties & deprecated) == 0)){
      if(variable_metadata[i].parent->id == Id_Root){
	parent = root;
      }else{
	parent = config_lookup(&config,variable_metadata[i].parent->variable_name);
      }
      if(!parent){
	fprintf(stderr,"Could not find parent of %s\n",variable_metadata[i].variable_name);
	abort();
      }
      s = config_setting_add(parent,variable_metadata[i].variable_name,get_config_type(variable_metadata[i].variable_type));
      if(!s){
	sp_error_fatal("Could not add config setting");
      }
      if(variable_metadata[i].variable_type == Type_String || 
	 variable_metadata[i].variable_type == Type_Filename ||
	 variable_metadata[i].variable_type == Type_Directory_Name){
	config_setting_set_string(s,(char *)variable_metadata[i].variable_address);
      }else if(variable_metadata[i].variable_type == Type_Int){
	config_setting_set_int(s,*((int *)variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_Vector_Real){
	sp_vector * v = *((sp_vector **)variable_metadata[i].variable_address);
	if(v){
	  for(unsigned int j= 0;j<sp_vector_size(v);j++){
	    config_setting_set_float_elem(s,-1,sp_vector_get(v,j));
	  }
	}
      }else if(variable_metadata[i].variable_type == Type_Vector_Int){
	sp_vector * v = *((sp_vector **)variable_metadata[i].variable_address);
	if(v){
	  for(unsigned int j= 0;j<sp_vector_size(v);j++){
	    config_setting_set_int_elem(s,-1,(int)sp_vector_get(v,j));
	  }
	}
      }else if(variable_metadata[i].variable_type == Type_Bool){
	config_setting_set_bool(s,*((int *)variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_Real){
	config_setting_set_float(s,*((real *)variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	for(int j = 0;;j++){
	  if(variable_metadata[i].list_valid_names[j] == NULL){
	    fprintf(stderr,"Error invalid list value\n");
	    abort();
	  }
	  if(*((int *)variable_metadata[i].variable_address) == variable_metadata[i].list_valid_values[j]){
	    config_setting_set_string(s,variable_metadata[i].list_valid_names[j]);
	    break;
	  }
	}
      }else if(variable_metadata[i].variable_type == Type_Map_Real){
	parent = s;
	s = config_setting_add(parent,"keys",CONFIG_TYPE_ARRAY);
	sp_smap * map = *((sp_smap **)variable_metadata[i].variable_address);
	sp_list * keys = sp_smap_get_keys(map);
	sp_list * values = sp_smap_get_values(map);
	for(unsigned int i = 0;i<sp_list_size(keys);i++){
	  config_setting_set_float_elem(s,-1,sp_list_get(keys,i));
	}
	s = config_setting_add(parent,"values",CONFIG_TYPE_ARRAY);
	for(unsigned int i = 0;i<sp_list_size(values);i++){
	  config_setting_set_float_elem(s,-1,sp_list_get(values,i));
	}
      }else{
	fprintf(stderr,"Variable type not yet supported!\n");	
	abort();
      }
    }
  }

  config_write_file(&config,filename);
}


static real interpolate_on_checkpoints(sp_vector * values,sp_vector * checkpoints, Options * opts,  int interpolation){
  int left = INT_MIN;
  int left_i;
  int right = INT_MAX;
  int right_i;    
  if(sp_vector_size(values) != sp_vector_size(checkpoints)){
    sp_error_fatal("Number of checkpoint elements doesn't match number of values!");
  }
  for(unsigned int i = 0;i<sp_vector_size(checkpoints);i++){
    real iter = sp_vector_get(checkpoints,i);
    if(iter <= opts->cur_iteration && iter > left){
      left = iter;
      left_i = i;
    }
    if(iter >= opts->cur_iteration && iter < right){
      right = iter;
      right_i = i;
    }
  }
  if(left == INT_MIN && right == INT_MAX){
    sp_error_fatal("Cannot find boundaries on object area!");
  }else if(left == INT_MIN){
    /* cur_iteration is before first checkpoint */
    return sp_vector_get(values,right_i);
  }else if(right == INT_MAX){
    /* cur_iteration is after last checkpoint */
    return sp_vector_get(values,left_i);
  }else if(right == left){
    /* cur_iteration is at a checkpoint */
    return sp_vector_get(values,left_i);
  }else{
      /* cur_iteration is between checkpoints */
    if(interpolation == 1){
      /* We're gonna use a Fermi-Dirac distribution to make a smooth shift between checkpoints 
	 We'll use kT = u/5 with u = 1 and restrict to the range e/u [0,2]
       */
      real f = 2.0*(opts->cur_iteration-left)/(real)(right-left);
      real n = 1.0/(exp((f-1.0)*5)+1.0);
      real delta = sp_vector_get(values,left_i)-sp_vector_get(values,right_i);
      /*      return (delta/2.0)*(1.0+cos(f*M_PI))+sp_vector_get(opts->object_area_at_checkpoints,right_i);*/
      return (delta)*(n)+sp_vector_get(values,right_i);
    }    
  }
  sp_error_fatal("Cannot reach here!");
  return -1;
}

real get_beta(Options * opts){
  static const float beta0 = 0.75;
  if(opts->beta_evolution){
    return bezier_map_interpolation(opts->beta_evolution,opts->cur_iteration);
  }
  if(opts->beta_checkpoints){
    if(!opts->beta_at_checkpoints){
      sp_error_fatal("When you specify beta_checkpoints you also need to specify the corresponding beta_at_checkpoints!");
    }
    return interpolate_on_checkpoints(opts->beta_at_checkpoints,opts->beta_checkpoints,opts,1);
  }
  int n = opts->cur_iteration;
  if(opts->dyn_beta){
    return beta0+(opts->beta-beta0)*(1-exp(-pow(n/opts->dyn_beta,3)));
  }
  return opts->beta;
}

real get_blur_radius(Options * opts){
  real a;
  if(opts->support_blur_evolution){
    return bezier_map_interpolation(opts->support_blur_evolution,opts->cur_iteration);
  }
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


real get_phases_blur_radius(Options * opts){
  real a;
  if(opts->phases_blur_evolution){
    return bezier_map_interpolation(opts->phases_blur_evolution,opts->cur_iteration);
  }

  if(opts->iterations_to_min_phases_blur){
    a = (3.0*opts->cur_iteration/opts->iterations_to_min_phases_blur)*(3.0*opts->cur_iteration/opts->iterations_to_min_phases_blur)*0.5;
    return (opts->phases_max_blur_radius-opts->phases_min_blur_radius)*exp(-a)+opts->phases_min_blur_radius;
  }
  return 0;    
}


real get_object_area(Options * opts){
  real a;
  if(opts->object_area_evolution){
    return bezier_map_interpolation(opts->object_area_evolution,opts->cur_iteration);
  }
  if(opts->object_area_checkpoints){
    int left = INT_MIN;
    int left_i = -1;
    int right = INT_MAX;
    int right_i = -1;
    if(!opts->object_area_at_checkpoints){
      sp_error_fatal("When you specify object_area_checkpoints you also need to specify the corresponding object_area_at_checkpoints!");
    }
    if(sp_vector_size(opts->object_area_checkpoints) != sp_vector_size(opts->object_area_at_checkpoints)){
      sp_error_fatal("Number of elements of object_area_at_checkpoints does not match object_area_checkpoints!");
    }
    for(unsigned int i = 0;i<sp_vector_size(opts->object_area_checkpoints);i++){
      real value = sp_vector_get(opts->object_area_checkpoints,i);
      if(value <= opts->cur_iteration && value > left){
	left = value;
	left_i = i;
      }
      if(value >= opts->cur_iteration && value < right){
	right = value;
	right_i = i;
      }
    }
    if(left == INT_MIN && right == INT_MAX){
      sp_error_fatal("Cannot find boundaries on object area!");
    }else if(left == INT_MIN){
      /* cur_iteration is before first checkpoint */
      return sp_vector_get(opts->object_area_at_checkpoints,right_i);
    }else if(right == INT_MAX){
      /* cur_iteration is after last checkpoint */
      return sp_vector_get(opts->object_area_at_checkpoints,left_i);
    }else if(right == left){
      /* cur_iteration is at a checkpoint */
      return sp_vector_get(opts->object_area_at_checkpoints,left_i);
    }else{
      /* cur_iteration is between checkpoints */
      /* Now we're gonna do a trigonometric interpolation using a cos curve */
      
      /* We're gonna use a Fermi-Dirac distribution to make a smooth shift between checkpoints 
	 We'll use kT = u/5 with u = 1 and restrict to the range e/u [0,2]
       */
      
      real f = 2.0*(opts->cur_iteration-left)/(real)(right-left);
      real n = 1.0/(exp((f-1.0)*5)+1.0);
      real delta = sp_vector_get(opts->object_area_at_checkpoints,left_i)-sp_vector_get(opts->object_area_at_checkpoints,right_i);
      /*      return (delta/2.0)*(1.0+cos(f*M_PI))+sp_vector_get(opts->object_area_at_checkpoints,right_i);*/
      return (delta)*(n)+sp_vector_get(opts->object_area_at_checkpoints,right_i);
    }    
  }else if(opts->support_update_algorithm == CONSTANT_AREA){
    return opts->object_area;
  }else if(opts->support_update_algorithm == DECREASING_AREA){
    if(opts->cur_iteration > opts->iterations_to_min_object_area){
      return opts->min_object_area;
    }
    real f = opts->cur_iteration/opts->iterations_to_min_object_area;
    a = (3.0*f)*(3.0*f)*0.5;
    return (opts->object_area-opts->min_object_area)*exp(-a)+opts->min_object_area;
  }
  return -1;
}


real get_gamma1(Options * opts){
  if(opts->gamma1 == -10000){
    /* Optimal value according to 
       Veit Elser 2003 "Random projections and the optimization of an algorithm for phase retrieval 
    */
    real beta = get_beta(opts);
    real sigma = 0.1;
    if(sigma > 0.5){
      sigma = 0.5;
    }
    return -(4+(2+beta)*sigma + beta*sigma*sigma)/(beta*(4-sigma+sigma*sigma));
  }
  return opts->gamma1;
}

real get_gamma2(Options * opts){
  if(opts->gamma2 == -10000){
    /* Optimal value according to 
       Veit Elser 2003 "Random projections and the optimization of an algorithm for phase retrieval 
    */
    real beta = get_beta(opts);
    return (3-beta)/(2*beta);
  }
  return opts->gamma2;
}

int get_random_seed(Options * opts){
  if(opts->random_seed == -1){
    opts->random_seed = getpid();
  }
  return opts->random_seed;
}



real bezier_map_interpolation(sp_smap * map, real x){
  sp_list * keys = sp_smap_get_keys(map);
  sp_list * values = sp_smap_get_values(map);
  unsigned int idx = 0;
  for(idx = 0;idx<sp_list_size(keys);idx++){
    if(x < sp_list_get(keys,idx)){
      break;
    }
  }
  if(idx == 0){
    return sp_list_get(values,0);
  }
  if(idx == sp_list_size(keys)){
    return sp_list_get(values,sp_list_size(keys)-1);
  }
  /* Cubic Bezier curve taken from http://en.wikipedia.org/wiki/Bézier_curve */
  real p0y = sp_list_get(values,idx-1);
  real p0x = sp_list_get(keys,idx-1);
  real p3y = sp_list_get(values,idx);
  real p3x = sp_list_get(keys,idx);
  real t = (2 - 2*(p0x - p3x)*
	    pow(8*p0x*p3x*x - 2*p3x*pow(p0x,2) - 4*x*pow(p0x,2) + 
		2*pow(p0x,3) - 2*p0x*pow(p3x,2) - 4*x*pow(p3x,2) + 
		2*pow(p3x,3) + pow(pow(p0x - p3x,4)*
				   (6*p0x*p3x - 16*p0x*x - 16*p3x*x + 5*pow(p0x,2) +
				    5*pow(p3x,2) + 16*pow(x,2)),0.5),
		-0.3333333333333333) +
	    2*pow(p0x - p3x,-1)*
	    pow(8*p0x*p3x*x - 2*p3x*pow(p0x,2) - 4*x*pow(p0x,2) +
		2*pow(p0x,3) - 2*p0x*pow(p3x,2) - 4*x*pow(p3x,2) +
		2*pow(p3x,3) + pow(pow(p0x - p3x,4)*
				   (6*p0x*p3x - 16*p0x*x -
				    16*p3x*x + 5*pow(p0x,2) + 
				     5*pow(p3x,2) + 16*pow(x,2)),0.5)
		,0.3333333333333333)
	    )/4.;  
  return 3*p0y*t*pow(1 - t,2) + p0y*pow(1 - t,3) +
    3*p3y*(1 - t)*pow(t,2) + p3y*pow(t,3);
}
