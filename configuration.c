#include <string.h>
#include <libconfig.h>
#include <stdlib.h>
#include <ctype.h>

#include "spimage.h"

#include "log.h"
#include "uwrapc.h"
#include "configuration.h"


Options global_options;

const int number_of_global_options = 51;

static char * get_path(const VariableMetadata * vm);
static int get_config_type(Variable_Type vt);

const VariableMetadata variable_metadata[100] = {
  {
    .variable_name = "ROOT",
    .variable_type = Type_Group,
    .id = Id_Root,
    .parent = NULL,
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.diffraction_filename)
  },
  {
    .variable_name = "amplitudes_file",
    .variable_type = Type_String,
    .id = Id_Diffraction_Filename,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.diffraction_filename)
  },
  {
    .variable_name = "real_image_filename",
    .variable_type = Type_String,
    .id = Id_Real_Image_Filename,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.real_image_filename)
  },
  {
    .variable_name = "max_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Max_Blur_Radius,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.max_blur_radius)
  },
  {
    .variable_name = "init_level",
    .variable_type = Type_Real,
    .id = Id_Init_Level,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.init_level)
  },
  {
    .variable_name = "beta",
    .variable_type = Type_Real,
    .id = Id_Beta,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.beta)
  },
  {
    .variable_name = "iterations",
    .variable_type = Type_Int,
    .id = Id_Iterations,
    .parent = &(variable_metadata[0]),
    .variable_properties = isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.iterations)
  },
  {
    .variable_name = "fixed_support_mask",
    .variable_type = Type_String,
    .id = Id_Support_Mask_Filename,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.support_mask_filename)
  },
  {
    .variable_name = "initial_support",
    .variable_type = Type_String,
    .id = Id_Init_Support_Filename,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.init_support_filename)
  },
  {
    .variable_name = "image_guess",
    .variable_type = Type_String,
    .id = Id_Image_Guess_Filename,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.image_guess_filename)
  },
  {
    .variable_name = "added_noise",
    .variable_type = Type_Real,
    .id = Id_Noise,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.noise)
  },
  {
    .variable_name = "beamstop_radius",
    .variable_type = Type_Real,
    .id = Id_Beamstop,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.beamstop)
  },
  {
    .variable_name = "support_intensity_threshold",
    .variable_type = Type_Real,
    .id = Id_New_Level,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.new_level)
  },
  {
    .variable_name = "iterations_to_min_blur",
    .variable_type = Type_Int,
    .id = Id_Iterations_To_Min_Blur,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.iterations_to_min_blur)
  },
  {
    .variable_name = "blur_radius_reduction_method",
    .variable_type = Type_MultipleChoice,
    .id = Id_Blur_Radius_Reduction_Method,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {GAUSSIAN_BLUR_REDUCTION,GEOMETRICAL_BLUR_REDUCTION,0},
    .list_valid_names = {"gaussian","geometrical",0},
    .variable_address = &(global_options.blur_radius_reduction_method)
  },
  {
    .variable_name = "minimum_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Min_Blur,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.min_blur)
  },
  {
    .variable_name = "logfile",
    .variable_type = Type_String,
    .id = Id_Log_File,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.log_file)
  },
  {
    .variable_name = "commandline",
    .variable_type = Type_String,
    .id = Id_Commandline,
    .parent = &(variable_metadata[0]),
    .variable_properties = isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.commandline)
  },
  {
    .variable_name = "output_period",
    .variable_type = Type_Int,
    .id = Id_Output_Period,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.output_period)
  },
  {
    .variable_name = "log_output_period",
    .variable_type = Type_Int,
    .id = Id_Log_Output_Period,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.log_output_period)
  },
  {
    .variable_name = "algorithm",
    .variable_type = Type_MultipleChoice,
    .id = Id_Algorithm,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {HIO,RAAR,HPR,CFLIP,RAAR_CFLIP,HAAR,SO2D,0},
    .list_valid_names = {"hio","raar","hpr","cflip","raar_cflip","haar","so2d",0},
    .variable_address = &(global_options.algorithm)
  },
  {
    .variable_name = "RAAR_sigma",
    .variable_type = Type_Real,
    .id = Id_Exp_Sigma,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.exp_sigma)
  },
  {
    .variable_name = "dynamic_beta",
    .variable_type = Type_Real,
    .id = Id_Dyn_Beta,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.dyn_beta)
  },
  {
    .variable_name = "random_initial_phases",
    .variable_type = Type_Bool,
    .id = Id_Rand_Phases,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.rand_phases)
  },
  {
    .variable_name = "random_initial_intensities",
    .variable_type = Type_Bool,
    .id = Id_Rand_Intensities,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.rand_intensities)
  },
  {
    .variable_name = "cur_iteration",
    .variable_type = Type_Int,
    .id = Id_Cur_Iteration,
    .parent = &(variable_metadata[0]),
    .variable_properties = isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.cur_iteration)
  },
  {
    .variable_name = "adapt_thres",
    .variable_type = Type_Bool,
    .id = Id_Adapt_Thres,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.adapt_thres)
  },
  {
    .variable_name = "automatic",
    .variable_type = Type_Bool,
    .id = Id_Automatic,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.dyn_beta)
  },
  {
    .variable_name = "work_directory",
    .variable_type = Type_String,
    .id = Id_Work_Dir,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.work_dir)
  },
  {
    .variable_name = "support_real_error_threshold",
    .variable_type = Type_Real,
    .id = Id_Real_Error_Threshold,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.real_error_threshold)
  },
  {
    .variable_name = "support_update_algorithm",
    .variable_type = Type_MultipleChoice,
    .id = Id_Support_Update_Algorithm,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {FIXED,STEPPED,REAL_ERROR_CAPPED,REAL_ERROR_ADAPTATIVE,CONSTANT_AREA,DECREASING_AREA,0},
    .list_valid_names = {"fixed","stepped","real_error_capped","real_error_adaptative","constant_area","decreasing_area",0},
    .variable_address = &(global_options.support_update_algorithm)
  },
  {
    .variable_name = "output_precision",
    .variable_type = Type_Int,
    .id = Id_Output_Precision,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.output_precision)
  },
  {
    .variable_name = "error_reduction_iterations_after_loop",
    .variable_type = Type_Int,
    .id = Id_Error_Reduction_Iterations_After_Loop,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.error_reduction_iterations_after_loop)
  },
  {
    .variable_name = "enforce_positivity",
    .variable_type = Type_Real,
    .id = Id_Enforce_Positivity,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.enforce_positivity)
  },
  {
    .variable_name = "genetic_optimization",
    .variable_type = Type_Bool,
    .id = Id_Genetic_Optimization,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.genetic_optimization)
  },
  {
    .variable_name = "charge_flip_sigma",
    .variable_type = Type_Real,
    .id = Id_Charge_Flip_Sigma,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.charge_flip_sigma)
  },
  {
    .variable_name = "rescale_amplitudes",
    .variable_type = Type_Bool,
    .id = Id_Rescale_Amplitudes,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.rescale_amplitudes)
  },
  {
    .variable_name = "square_mask",
    .variable_type = Type_Real,
    .id = Id_Square_Mask,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.square_mask)
  },
  {
    .variable_name = "patterson_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Patterson_Blur_Radius,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.patterson_blur_radius)
  },
  {
    .variable_name = "remove_central_pixel_phase",
    .variable_type = Type_Bool,
    .id = Id_Remove_Central_Pixel_phase,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.remove_central_pixel_phase)
  },
  {
    .variable_name = "perturb_weak_reflections",
    .variable_type = Type_Real,
    .id = Id_Perturb_Weak_Reflections,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.perturb_weak_reflections)
  },
  {
    .variable_name = "nthreads",
    .variable_type = Type_Int,
    .id = Id_Nthreads,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.nthreads)
  },
  {
    .variable_name = "break_centrosym_period",
    .variable_type = Type_Int,
    .id = Id_Break_Centrosym_Period,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.break_centrosym_period)
  },
  {
    .variable_name = "reconstruction_finished",
    .variable_type = Type_Bool,
    .id = Id_Reconstruction_Finished,
    .parent = &(variable_metadata[0]),
    .variable_properties = isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.reconstruction_finished)
  },
  {
    .variable_name = "real_error_tolerance",
    .variable_type = Type_Real,
    .id = Id_Real_Error_Tolerance,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.real_error_tolerance)
  },
  {
    .variable_name = "max_iterations",
    .variable_type = Type_Int,
    .id = Id_Max_Iterations,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.max_iterations)
  },
  {
    .variable_name = "patterson_level_algorithm",
    .variable_type = Type_MultipleChoice,
    .id = Id_Patterson_Level_Algorithm,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {FIXED,STEPPED,REAL_ERROR_CAPPED,REAL_ERROR_ADAPTATIVE,CONSTANT_AREA,DECREASING_AREA,0},
    .list_valid_names = {"fixed","stepped","real_error_capped","real_error_adaptative","constant_area","decreasing_area",0},
    .variable_address = &(global_options.patterson_level_algorithm)
  },
  {
    .variable_name = "object_area",
    .variable_type = Type_Real,
    .id = Id_Object_Area,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.object_area)
  },
  {
    .variable_name = "image_blur_period",
    .variable_type = Type_Int,
    .id = Id_Image_Blur_Period,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.image_blur_period)
  },
  {
    .variable_name = "image_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Image_Blur_Radius,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.image_blur_radius)
  },
  {
    .variable_name = "iterations_to_min_object_area",
    .variable_type = Type_Int,
    .id = Id_Iterations_To_Min_Object_Area,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.iterations_to_min_object_area)
  }
};



char * get_path(const VariableMetadata * vm){
  const VariableMetadata * parent = vm->parent;
  char * buffer1 = malloc(sizeof(char)*OPTION_STRING_SIZE);
  char * buffer2 = malloc(sizeof(char)*OPTION_STRING_SIZE);
  strcpy(buffer1,vm->variable_name);
  while(parent->id != Id_Root){
    strcpy(buffer2,buffer1);
    sprintf(buffer1,"%s.",parent->variable_name);
    strcat(buffer1,buffer2);
  }
  free(buffer2);
  return buffer1;
}

int get_config_type(Variable_Type vt){
  if(vt == Type_Real){
    return CONFIG_TYPE_FLOAT;
  }else if(vt == Type_Int){
    return CONFIG_TYPE_INT;
  }else if(vt == Type_String){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_MultipleChoice){
    return CONFIG_TYPE_STRING;
  }else if(vt == Type_Bool){
    return CONFIG_TYPE_BOOL;
  }else if(vt == Type_Group){
    return CONFIG_TYPE_GROUP;
  }
  fprintf(stderr,"Unkown variable type\n");
  abort();
  return -1;
}

  
  
Options * set_defaults(){
  Options * opt = &global_options;
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
  return opt;
}

void read_options_file(char * filename, Options * res){
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
      if(config_lookup(&config,get_path(&(variable_metadata[i])))){	
	if(variable_metadata[i].variable_type == Type_String){
	  strcpy((char *)variable_metadata[i].variable_address,config_lookup_string(&config,get_path(&(variable_metadata[i]))));
	}else if(variable_metadata[i].variable_type == Type_Int){
	  *((int *)variable_metadata[i].variable_address) = config_lookup_int(&config,get_path(&(variable_metadata[i])));
	}else if(variable_metadata[i].variable_type == Type_Real){
	  if(config_setting_type(config_lookup(&config,get_path(&(variable_metadata[i])))) == CONFIG_TYPE_FLOAT){
	    *((real *)variable_metadata[i].variable_address) = config_lookup_float(&config,get_path(&(variable_metadata[i])));
	  }else{
	    *((real *)variable_metadata[i].variable_address) = config_lookup_int(&config,get_path(&(variable_metadata[i])));
	  }
	}else if(variable_metadata[i].variable_type == Type_Bool){
	  if(config_setting_type(config_lookup(&config,get_path(&(variable_metadata[i])))) == CONFIG_TYPE_BOOL){
	    *((int *)variable_metadata[i].variable_address) = config_lookup_bool(&config,get_path(&(variable_metadata[i])));
	  }else{
	    *((int *)variable_metadata[i].variable_address) = config_lookup_int(&config,get_path(&(variable_metadata[i])));
	  }

	}else if(variable_metadata[i].variable_type == Type_MultipleChoice){
	  /* Change string to lowercase for comparison */
	  char buffer[OPTION_STRING_SIZE];
	  strcpy(buffer,config_lookup_string(&config,get_path(&(variable_metadata[i]))));
	  for(int j = 0;j<strlen(buffer);j++){
	    buffer[j] = tolower(buffer[j]);
	  }
	  /* Compare with list of known good strings */
	  for(int j = 0;;j++){
	    if(variable_metadata[i].list_valid_names[j] == NULL){
	      fprintf(stderr,"Error invalid list value %s for option %s\n",
		      config_lookup_string(&config,get_path(&(variable_metadata[i]))),
		      variable_metadata[i].variable_name);
	      abort();
	    }
	    if(strcmp(variable_metadata[i].list_valid_names[j],buffer) == 0){
	      *((int *)variable_metadata[i].variable_address) = variable_metadata[i].list_valid_values[j];
	      break;
	    }
	  }
	}else{
	  fprintf(stderr,"Variable type not yet supported!\n");	
	  abort();
	}
      }
    }
  }

  if(global_options.diffraction_filename){    
    global_options.diffraction = sp_image_read(global_options.diffraction_filename,0);
  }else if(global_options.real_image_filename){
    global_options.real_image = sp_image_read(global_options.real_image_filename,0);
  }
#if 0


  
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

  if((tmp = config_lookup_string(&config,"patterson_level_algorithm"))){
    if(strcmp(tmp,"fixed") == 0){
      res->patterson_level_algorithm = FIXED;      
    }else if(strcmp(tmp,"constant_area") == 0){
      res->patterson_level_algorithm = CONSTANT_AREA;
      if(config_lookup(&config,"object_area")){
	res->object_area = config_lookup_float(&config,"object_area");
      }
    }
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
    }else if(strcmp(tmp,"constant_area") == 0){
      res->support_update_algorithm = CONSTANT_AREA;
      if(config_lookup(&config,"object_area")){
	res->object_area = config_lookup_float(&config,"object_area");
      }
    }else if(strcmp(tmp,"decreasing_area") == 0){
      res->support_update_algorithm = DECREASING_AREA;
      if(config_lookup(&config,"object_area")){
	res->object_area = config_lookup_float(&config,"object_area");
      }
      if(config_lookup(&config,"min_object_area")){
	res->min_object_area = config_lookup_float(&config,"min_object_area");
      }
      if(config_lookup(&config,"iterations_to_min_object_area")){
	res->iterations_to_min_object_area = config_lookup_int(&config,"iterations_to_min_object_area");
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
    }else if(strcmp(tmp,"HAAR") == 0|| strcmp(tmp,"haar") == 0){
      res->algorithm = HAAR;
    }else if(strcmp(tmp,"SO2D") == 0|| strcmp(tmp,"so2d") == 0){
      res->algorithm = SO2D;
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
  if(config_lookup(&config,"patterson_blur_radius")){
    res->patterson_blur_radius = config_lookup_float(&config,"patterson_blur_radius");  
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
  if(config_lookup(&config,"image_blur_period")){
    res->image_blur_period = config_lookup_int(&config,"image_blur_period");  
  }
  if(config_lookup(&config,"image_blur_radius")){
    res->image_blur_radius = config_lookup_float(&config,"image_blur_radius");  
  }

#endif
}


void parse_options(int argc, char ** argv, Options * res){
}


void write_options_file(char * filename, Options * res){
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
      if(variable_metadata[i].variable_type == Type_Group && variable_metadata[i].id != Id_Root){
	/* it's a group */
	if(config_lookup(&config,variable_metadata[i].variable_name) == NULL){
	  /* it still doesn't exist */
	  if((s = config_lookup(&config,variable_metadata[i].parent->variable_name)) != NULL){
	    /* parent exists */
	    config_setting_add(s,variable_metadata[i].variable_name,variable_metadata[i].variable_type);
	    n_groups--;
	  }
	  if(variable_metadata[i].parent->id == Id_Root){
	    /* parent is Root */
	    config_setting_add(root,variable_metadata[i].variable_name,variable_metadata[i].variable_type);
	    n_groups--;
	  }
	}
      }
    }
  }
	  
  /* Loop around all the options and put them inside their groups */
  for(int i = 0; i< number_of_global_options; i++){
    config_setting_t * parent;
    if(variable_metadata[i].variable_type != Type_Group && (variable_metadata[i].variable_properties & isSettableBeforeRun)){
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
      if(variable_metadata[i].variable_type == Type_String){
	config_setting_set_string(s,(char *)variable_metadata[i].variable_address);
      }else if(variable_metadata[i].variable_type == Type_Int){
	config_setting_set_int(s,*((int *)variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_Real){
	config_setting_set_float(s,*((real *)variable_metadata[i].variable_address));
      }else if(variable_metadata[i].variable_type == Type_Bool){
	config_setting_set_bool(s,*((int *)variable_metadata[i].variable_address));
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
      }else{
	fprintf(stderr,"Variable type not yet supported!\n");	
	abort();
      }
    }
  }

  config_write_file(&config,filename);
}


real get_beta(Options * opts){
  static const int beta0 = 0.75;
  int n = opts->cur_iteration;
  if(opts->dyn_beta){
    return beta0+(opts->beta-beta0)*(1-exp(-pow(n/opts->dyn_beta,3)));
  }
  return opts->beta;
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


real get_object_area(Options * opts){
  real a;
  if(opts->support_update_algorithm == CONSTANT_AREA){
    return opts->object_area;
  }else if(opts->support_update_algorithm == DECREASING_AREA){
    a = (3.0*opts->cur_iteration/opts->iterations_to_min_object_area)*(3.0*opts->cur_iteration/opts->iterations_to_min_object_area)*0.5;
    return (opts->object_area-opts->min_object_area)*exp(-a)+opts->min_object_area;
  }
  return -1;
}
