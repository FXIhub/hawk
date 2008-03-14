#ifndef _ALGORITHMS_H_
#define _ALGORITHMS_H_

int get_algorithm(Options * opts,Log * log);

Image * basic_hio_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log);

Image * basic_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
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
#endif
