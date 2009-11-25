#ifndef _CONFIG_H_
#define _CONFIG_H_


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define VERSION "1.22"

#include "libconfig.h"
#include "spimage.h"




#define OPTION_STRING_SIZE 10024


  typedef enum{HIO=0,RAAR,HPR,CFLIP,RAAR_CFLIP,ESPRESSO,HAAR,SO2D,RAAR_PROJ,HIO_PROJ,DIFF_MAP} Phasing_Algorithms;



typedef enum{FIXED=0,STEPPED,REAL_ERROR_CAPPED,REAL_ERROR_ADAPTATIVE,CONSTANT_AREA,DECREASING_AREA,COMPLEX_DECREASING_AREA} Support_Update_Algorithms;


typedef enum{GAUSSIAN_BLUR_REDUCTION=0,GEOMETRICAL_BLUR_REDUCTION} Blur_Reduction_Method;

  typedef enum{PHASES_FROM_SUPPORT=0, PHASES_ZERO,PHASES_RANDOM}Initial_Phases;

typedef enum {Type_Real=0, Type_Int, Type_String, 
	      Type_MultipleChoice,Type_Bool, Type_Group, Type_Image, Type_Slice, Type_Vector_Real, Type_Vector_Int,Type_Filename,Type_Directory_Name,Type_Map_Real}Variable_Type;

typedef enum {Id_Diffraction_Filename=0,Id_Real_Image_Filename,Id_Max_Blur_Radius,Id_Init_Level,
	      Id_Beta,Id_Iterations,Id_Support_Mask_Filename,Id_Init_Support_Filename,Id_Image_Guess_Filename,
	      Id_Noise,Id_Beamstop,Id_New_Level
	      ,Id_Iterations_To_Min_Blur,Id_Blur_Radius_Reduction_Method,Id_Min_Blur,
	      Id_Enforce_Real,Id_Log_File,Id_Commandline,Id_Output_Period,Id_Log_Output_Period,
	      Id_Algorithm,Id_Exp_Sigma,Id_Dyn_Beta,Id_Rand_Phases,Id_Rand_Intensities,Id_Cur_Iteration,
	      Id_Adapt_Thres,Id_Automatic,Id_Work_Dir,Id_Real_Error_Threshold,Id_Support_Update_Algorithm,
	      Id_Output_Precision,Id_Error_Reduction_Iterations_After_Loop,Id_Enforce_Positivity,
	      Id_Genetic_Optimization,Id_Charge_Flip_Sigma,Id_Espresso_Tau,Id_Rescale_Amplitudes,Id_Square_Mask,
	      Id_Patterson_Blur_Radius,Id_Remove_Central_Pixel_Phase,Id_Perturb_Weak_Reflections,Id_Nthreads,
	      Id_Break_Centrosym_Period,Id_Reconstruction_Finished,Id_Real_Error_Tolerance,Id_Root,
	      Id_Remove_Central_Pixel_phase,Id_Max_Iterations,Id_Patterson_Level_Algorithm,Id_Object_Area,
	      Id_Image_Blur_Period,Id_Image_Blur_Radius,Id_Iterations_To_Min_Object_Area,Id_Min_Object_Area,
	      Id_Current_Real_Space_Image,
	      Id_Current_Support,Id_Solution_File,Id_Phases_Min_Blur_Radius,Id_Phases_Max_Blur_Radius,
	      Id_Iterations_To_Min_Phases_Blur,
	      Id_Object_Area_Checkpoints,Id_Object_Area_at_Checkpoints,Id_Autocorrelation_Support_File,
	      Id_Filter_Intensities,Id_Beta_Checkpoints,Id_Beta_at_Checkpoints,Id_Gamma1,Id_Gamma2,Id_Support_Image_Averaging,
	      Id_Random_Seed,Id_Input_Files,Id_Input_Files_Amplitudes,Id_Initialization,Id_Input,Id_Logging,Id_Phasing,Id_Support,
	      Id_Autocorrelation_Area,Id_Remote_Work_Dir
}Variable_Id;
  
  
typedef enum {isSettableBeforeRun = 1, isSettableDuringRun = 2, isGettableBeforeRun = 4,
	      isGettableDuringRun = 8, isMandatory = 16, deprecated = 32,advanced = 64,withSpecialValue = 128} Variable_Properties;



typedef struct {
  Image * diffraction;
  Image * amplitudes;
  char diffraction_filename[OPTION_STRING_SIZE];
  Image * real_image;
  char real_image_filename[OPTION_STRING_SIZE];
  real max_blur_radius;
  real init_level;
  real beta;
  int iterations;
  Image * support_mask;
  char support_mask_filename[OPTION_STRING_SIZE];
  Image * init_support;
  char init_support_filename[OPTION_STRING_SIZE];
  Image * image_guess;
  char image_guess_filename[OPTION_STRING_SIZE];
  real noise;
  real beamstop;
  real new_level;
  int iterations_to_min_blur;
  int blur_radius_reduction_method;
  real min_blur;
  int enforce_real;
  /* given sufficiently long parameters this will seg fault
     but i don't really care as the program in not secure in any way*/
  char log_file[OPTION_STRING_SIZE];  
  char commandline[OPTION_STRING_SIZE];
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
  char work_dir[OPTION_STRING_SIZE];
  char remote_work_dir[OPTION_STRING_SIZE];
  FILE * flog;
  real real_error_threshold;
  int support_update_algorithm;
  int output_precision;
  int error_reduction_iterations_after_loop;
  int enforce_positivity;
  int genetic_optimization;
  real charge_flip_sigma;
  real espresso_tau;
  int rescale_amplitudes;
  real square_mask;
  real patterson_blur_radius;
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
  Image ** current_real_space_image;
  Image ** current_support;
  int is_running;
  char solution_filename[OPTION_STRING_SIZE];
  Image * solution_image;
  int iterations_to_min_phases_blur;
  real phases_max_blur_radius;
  real phases_min_blur_radius;
  char intensities_std_dev_filename[OPTION_STRING_SIZE];
  Image * intensities_std_dev;
  char autocorrelation_support_filename[OPTION_STRING_SIZE];
  Image * autocorrelation_support;
  int filter_intensities;
  sp_vector * object_area_at_checkpoints;
  sp_vector * object_area_checkpoints;
  sp_vector * beta_at_checkpoints;
  sp_vector * beta_checkpoints;
  real gamma1;
  real gamma2;
  int support_image_averaging;
  int random_seed;
  real autocorrelation_area;
  sp_smap * object_area_evolution;
  sp_smap * threshold_evolution;
  sp_smap * support_blur_evolution;
  sp_smap * beta_evolution;
  sp_smap * phases_blur_evolution;
}Options;

typedef struct _VariableMetadata{
  const char * variable_name;
  const char * display_name;
  const Variable_Type variable_type;
  const Variable_Id id;
  const struct _VariableMetadata * parent;
  const Variable_Properties variable_properties;
  const int list_valid_values[20];
  /* No more than 20 possible values per list */
  const char * list_valid_names[20];
  void * variable_address;
  /* No more than 10240 characters in the documentation */
  const char documentation[10240];
  /* Pointer to a function that determines if the option
     makes sense in the current configuration */
  int (* dependencies)(const Options *);
  /* Pointer reserved for future use */
  void * reserved;  
}VariableMetadata;


extern Options global_options;
extern const int number_of_global_options;
extern VariableMetadata variable_metadata[201];

  void init_options_metadata(Options * opt);
  void read_options_file(const char * filename);
  void parse_options(int argc, char ** argv, Options * opt);
  void set_defaults(Options * opt);
  void write_options_file(const char * filename);
  real get_beta(Options * opts);
  real get_blur_radius(Options * opts);
  real get_object_area(Options * opts);
  real get_phases_blur_radius(Options * opts);
  real get_gamma1(Options * opts);
  real get_gamma2(Options * opts);
  int get_random_seed(Options * opts);

  int get_list_value_from_list_name(VariableMetadata * md,char * name);
  real bezier_map_interpolation(sp_smap * map, real x);

  int check_options_and_load_images(Options * opts);
#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */


#endif
