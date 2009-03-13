#ifndef _UWRAP_H_
#define _UWRAP_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */





#include "configuration.h"
#include "log.h"



Image * fftout2img(real * fftout, Detector *det);
real * fft_centrosym(real * fftout, Detector *det);
real * fft_shift(real * fftout, Detector *det);
Image * get_support(Image * input, Options * opts);
Image * basic_hio_iteration(Image * fft_modulos, Image * real_in, Image * support, Options * opts,Log * log);
Image * get_newsupport(Image * input, real level , real radius, /*Image * prev_support, Image * patterson,*/ Options * opts);
Image * check_for_best_image(Image * in, Image * best, Image * support);
void fill_image_center(Image * in);
void output_to_log(Image * exp_amp,Image * real_in, Image * real_out, Image * fourier_out,Image * support, Options * opts, Log * log);
Image * basic_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			      Options * opts,Log * log);
real get_beta(Options * opts);
void set_rand_phases(Image * real_in, Image * diff);
void set_zero_phases(Image * real_in, Image * diff);
void set_rand_ints(Image * real_in, Image * img);
real get_new_threshold(Options * opts,Log * log);
int get_algorithm(Options * opts,Log  * log);
void init_log(Log * log);
void complete_reconstruction(Image * amp, Image * initial_support, Image * exp_sigma,
			     Options * opts, char * dir);
real get_newsupport_level(Image * input, real * previous_size , real radius, Log * log, Options * opts);
real get_blur_radius(Options * opts);
Image * get_filtered_support(Image * input, real level , real radius, Options * opts);
Image * basic_error_reduction_iteration(Image * exp_amp, Image * real_in, Image * support, 
					Options * opts, Log * log);
Image * basic_hpr_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log);
Image * basic_cflip_iteration(Image * exp_amp, Image * real_in, Image * support, 
			      Options * opts, Log * log);
Image * basic_raar_cflip_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
				   Options * opts, Log * log);
Image * basic_my_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
				Options * opts, Log * log);
void rescale_image(Image * a);
void centrosym_break_attempt(Image * a);
void init_reconstruction(Options * opts);
void enforce_parsevals_theorem(Image * master, Image * to_scale);

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */


#endif
