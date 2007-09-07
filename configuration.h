#ifndef _CONFIG_H_
#define _CONFIG_H_

#define VERSION "1.13"

typedef enum{HIO=1,RAAR,HPR,CFLIP,RAAR_CFLIP,HAAR,SO2D} Phasing_Algorithms;



typedef enum{FIXED=1,STEPPED,REAL_ERROR_CAPPED,REAL_ERROR_ADAPTATIVE,CONSTANT_AREA,DECREASING_AREA} Support_Update_Algorithms;


typedef enum{GAUSSIAN_BLUR_REDUCTION=1,GEOMETRICAL_BLUR_REDUCTION} Blur_Reduction_Method;


typedef enum {Type_Real = 1, Type_Int, Type_String, Type_List,Type_Bool}Variable_Type;

typedef enum {Id_Diffraction_Filename=0,Id_Real_Image_Filename,Id_Max_Blur_Radius,Id_Init_Level,
	      Id_Beta,Id_Iterations,Id_Support_Mask_Filename,Id_Init_Support_Filename,Id_Image_Guess_Filename,
	      Id_Noise,Id_Beamstop,Id_New_Level
	      ,Id_Iterations_To_Min_Blur,Id_Blur_Radius_Reduction_Method,Id_Min_Blur,
	      Id_Enforce_Real,Id_Log_File,Id_Commandline,Id_Output_Period,Id_Log_Output_Period,
	      Id_Algorithm,Id_Exp_Sigma,Id_Dyn_Beta,Id_Rand_Phases,Id_Rand_Intensities,Id_Cur_Iteration,
	      Id_Adapt_Thres,Id_Automatic,Id_Work_Dir,Id_Real_Error_Threshold,Id_Support_Update_Algorithm,
	      Id_Output_Precision,Id_Error_Reduction_Iterations_After_Loop,Id_Enforce_Positivity,
	      Id_Genetic_Optimization,Id_Charge_Flip_Sigma,Id_Rescale_Amplitudes,Id_Square_Mask,
	      Id_Patterson_Blur_Radius,Id_Remove_Central_Pixel_Phase,Id_Perturb_Weak_Reflections,Id_Nthreads,
	      Id_Break_Centrosym_Period,Id_Reconstruction_Finished,Id_Real_Error_Tolerance,Id_Root,
	      Id_Remove_Central_Pixel_phase,Id_Max_Iterations,Id_Patterson_Level_Algorithm,Id_Object_Area,
	      Id_Image_Blur_Period,Id_Image_Blur_Radius,Id_Iterations_To_Min_Object_Area
}Variable_Id;


typedef enum {isSettableBeforeRun = 1, isSettableDuringRun = 2, isGettableBeforeRun = 4,
				 isGettableDuringRun = 8, isMandatory = 16} Variable_Properties;
typedef struct{
  const char * variable_name;
  const Variable_Type variable_type;
  const Variable_Id id;
  const Variable_Id parent;
  const Variable_Properties variable_properties;
  const int list_valid_values[10];
  /* No more than 10 possible values per list */
  const char * list_valid_names[10];
  const void * variable_address;
  /* We should also have a documentation field */
}VariableMetadata;


typedef struct {
  Image * diffraction;
  char diffraction_filename[1024];
  Image * real_image;
  char real_image_filename[1024];
  real max_blur_radius;
  real init_level;
  real beta;
  int iterations;
  Image * support_mask;
  char support_mask_filename[1024];
  Image * init_support;
  char init_support_filename[1024];
  Image * image_guess;
  char image_guess_filename[1024];
  real noise;
  real beamstop;
  real new_level;
  int iterations_to_min_blur;
  int blur_radius_reduction_method;
  real min_blur;
  int enforce_real;
  /* given sufficiently long parameters this will seg fault
     but i don't really care as the program in not secure in any way*/
  char log_file[1024];  
  char commandline[10024];
  int output_period;
  int log_output_period;
  int algorithm;
  real exp_sigma;
  real dyn_beta;
  int rand_phases;
  int rand_intensities;
  int cur_iteration;
  int adapt_thres;
  int automatic;
  char work_dir[1024];
  FILE * flog;
  real real_error_threshold;
  int support_update_algorithm;
  int output_precision;
  int error_reduction_iterations_after_loop;
  int enforce_positivity;
  int genetic_optimization;
  real charge_flip_sigma;
  int rescale_amplitudes;
  real square_mask;
  float patterson_blur_radius;
  int remove_central_pixel_phase;
  real perturb_weak_reflections;
  int nthreads;
  int break_centrosym_period;
  int reconstruction_finished;
  real real_error_tolerance;
  int max_iterations;
  int patterson_level_algorithm;
  real object_area;
  int image_blur_period;
  real image_blur_radius;
  int iterations_to_min_object_area;
  real min_object_area;
}Options;


void init_options_metadata(Options * opt);
void read_options_file(char * filename, Options * opt);
void parse_options(int argc, char ** argv, Options * opt);
Options * set_defaults(void);
void write_options_file(char * filename, Options * res);
real get_beta(Options * opts);
real get_blur_radius(Options * opts);
real get_object_area(Options * opts);

#endif
