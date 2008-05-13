#include "configuration.h"



VariableMetadata variable_metadata[100] = {
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
    .variable_name = "real_image_file",
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
    .variable_name = "patterson_threshold",
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
    .variable_name = "innerloop_iterations",
    .variable_type = Type_Int,
    .id = Id_Iterations,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
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
    .list_valid_values = {HIO,RAAR,HPR,CFLIP,RAAR_CFLIP,HAAR,SO2D,RAAR_PROJ,0},
    .list_valid_names = {"hio","raar","hpr","cflip","raar_cflip","haar","so2d","raar_proj",0},
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
    .variable_name = "enforce_real",
    .variable_type = Type_Int,
    .id = Id_Enforce_Real,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.enforce_real)
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
  },
  {
    .variable_name = "min_object_area",
    .variable_type = Type_Real,
    .id = Id_Min_Object_Area,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.min_object_area)
  },
  {
    .variable_name = "current_real_space_image",
    .variable_type = Type_Image,
    .id = Id_Current_Real_Space_Image,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableDuringRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.current_real_space_image)
  },
  {
    .variable_name = "current_support",
    .variable_type = Type_Image,
    .id = Id_Current_Support,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableDuringRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.current_support)
  },
  {
    .variable_name = "solution_file",
    .variable_type = Type_String,
    .id = Id_Solution_File,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isSettableDuringRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.solution_filename)
  },
  {
    .variable_name = "phases_min_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Phases_Min_Blur_Radius,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.phases_min_blur_radius)
  },
  {
    .variable_name = "phases_max_blur_radius",
    .variable_type = Type_Real,
    .id = Id_Phases_Max_Blur_Radius,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.phases_max_blur_radius)
  },
  {
    .variable_name = "iterations_to_min_phases_blur",
    .variable_type = Type_Int,
    .id = Id_Iterations_To_Min_Phases_Blur,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.iterations_to_min_phases_blur)
  },
  {
    .variable_name = "intensities_std_dev_file",
    .variable_type = Type_String,
    .id = Id_Solution_File,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.intensities_std_dev_filename)
  },
  {
    .variable_name = "autocorrelation_support_file",
    .variable_type = Type_String,
    .id = Id_Autocorrelation_Support_File,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isGettableBeforeRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.autocorrelation_support_filename)
  },
  {
    .variable_name = "filter_intensities",
    .variable_type = Type_Bool,
    .id = Id_Filter_Intensities,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.filter_intensities)
  },
  {
    .variable_name = "object_area_checkpoints",
    .variable_type = Type_Vector_Int,
    .id = Id_Object_Area_Checkpoints,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.object_area_checkpoints)
  },
  {
    .variable_name = "object_area_at_checkpoints",
    .variable_type = Type_Vector_Real,
    .id = Id_Object_Area_at_Checkpoints,
    .parent = &(variable_metadata[0]),
    .variable_properties = isSettableBeforeRun|isSettableDuringRun|isGettableBeforeRun|isGettableDuringRun,
    .list_valid_values = {0},
    .list_valid_names = {0},
    .variable_address = &(global_options.object_area_at_checkpoints)
  }
};


/* Don't forget to update this one!! */
const int number_of_global_options = 64;
