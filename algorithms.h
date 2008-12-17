#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

#include "log.h"

int get_algorithm(Options * opts,Log * log);

Image * basic_hio_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log);

Image * basic_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log);
Image * serial_raar_iteration(Image * exp_amp, Image * real_in, Image * support, 
			     Options * opts, Log * log);

Image * basic_raar_proj_iteration(Image * exp_amp, Image * int_std_dev, Image * real_in, Image * support, 
			     Options * opts, Log * log);

#ifdef MPI
Image * mpi_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			   Options * opts, Log * log);
#endif

Image * basic_error_reduction_iteration(Image * exp_amp, Image * real_in, Image * support, 
					Options * opts, Log * log);

Image * basic_hpr_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log);

Image * basic_cflip_iteration(Image * exp_amp, Image * real_in, Image * support, 
			      Options * opts, Log * log);

Image * basic_haar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log);

Image * basic_so2d_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log);

void phase_smoothening_iteration(Image * real_in, Options * opts, Log * log);
Image * serial_difference_map_f1(Image * real_in,Image * support, real gamma);
Image * serial_difference_map_f2(Image * exp_amp,Image * real_in, real gamma);
Image * serial_difference_map_iteration(Image * exp_amp, Image * real_in, Image * support, 
					Options * opts, Log * log);
#endif
