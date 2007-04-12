#ifndef _CONFIG_H_
#define _CONFIG_H_

#define HIO 1
#define RAAR 2
#define HPR 3
#define CFLIP 4
#define RAAR_CFLIP 5


#define FIXED 1
#define STEPPED 2
#define REAL_ERROR_CAPPED 3
#define REAL_ERROR_ADAPTATIVE 4

#define GAUSSIAN_BLUR_REDUCTION 1
#define GEOMETRICAL_BLUR_REDUCTION 2

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
  int blur_patterson;
  int remove_central_pixel_phase;
  real perturb_weak_reflections;
  int nthreads;
  int break_centrosym_period;
  int reconstruction_finished;
  real real_error_tolerance;
  int max_iterations;
}Options;

void read_options_file(char * filename, Options * opt);
void parse_options(int argc, char ** argv, Options * opt);
Options * set_defaults(void);
void write_options_file(char * filename, Options * res);

#endif
