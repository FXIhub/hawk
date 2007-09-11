#include "spimage.h"
#include "log.h"
#include "uwrapc.h"
#include "configuration.h"
#include "algorithms.h"
#include "support.h"
#include "saddle/minmaxL.h"

int real_compare(const void * a, const void * b){
  if((*(real *)a) <(*(real *)b)){
    return -1;
  }else if((*(real *)a) == (*(real *)b)){
    return 0;
  }else{
    return 1;
  }
  return -2;
}

Image * basic_hio_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_c3matrix_size(real_in->image);
  real beta = get_beta(opts);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);

  sp_image_rephase(pattern,SP_ZERO_PHASE);

  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      if(cabsr(fft_out->image->data[i])){
	pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabsr(fft_out->image->data[i]);
/*      }else{
	real phase = p_drand48()*2*M_PI;
	pattern->image->data[i] = cos(phase)*exp_amp->image->data[i]+I+sin(phase)*exp_amp->image->data[i];*/
      }
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i]){
      if(opts->enforce_real){
	real_out->image->data[i] = creal(real_in->image->data[i] - beta*real_out->image->data[i]);      
      }else{
	real_out->image->data[i] = (real_in->image->data[i] - beta*real_out->image->data[i]) ;      
      }
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


Image * basic_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  /*We're gonna have 3 kinds of images.
   With only real space constraints (Ps)
   With only fourier space constraints (Pm)
   With both constraints (PsPm) */
  static Image * exp_amp_minus_sigma = NULL;
  static Image * exp_amp_plus_sigma = NULL;
  Image * real_out;

  Image * fft_out;
  Image * fft_in;
  long long i;
  long long size = sp_c3matrix_size(real_in->image);
  real beta = get_beta(opts);
  real one_minus_2_beta = 1.0-2*beta;
  real tmp;
  static char * weak_reflections = NULL;
  real max = 0;
  real * array;
  fft_out = sp_image_fft(real_in);
  fft_in = sp_image_duplicate(fft_out,SP_COPY_DATA|SP_COPY_MASK);
  if(!exp_amp_minus_sigma){
    exp_amp_minus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      exp_amp_minus_sigma->image->data[i] -= exp_sigma->image->data[i];
    }
  }
  if(!exp_amp_plus_sigma){
    exp_amp_plus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      exp_amp_plus_sigma->image->data[i] += exp_sigma->image->data[i];
    }
  }

  /* Weak reflections doesn't work for images where the data is
   * largerthan INT_MAX bits /Tomas
   */
  if(opts->perturb_weak_reflections && !weak_reflections){
    weak_reflections = sp_malloc(sp_c3matrix_size(exp_amp->image)*sizeof(char));
    array = sp_malloc(sp_c3matrix_size(exp_amp->image)*sizeof(real));
    memcpy(array,exp_amp->image,sp_c3matrix_size(exp_amp->image)*sizeof(real));
    qsort(array,sp_c3matrix_size(exp_amp->image),sizeof(real),real_compare);
    /* get the weak reflections threshold */
    max = array[(long long int)(sp_c3matrix_size(exp_amp->image)*opts->perturb_weak_reflections)];
    sp_free(array);
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      if(exp_amp->mask->data[i] && creal(exp_amp->image->data[i]) <= max){
	weak_reflections[i] = 1;
      }else{
	weak_reflections[i] = 0;
      }
    }
  }
  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(exp_amp->mask->data[i]){
      /* take the calculated phases and apply to the experimental intensities 
	 leaving room for error */
      if(cabs(fft_out->image->data[i]) < creal(exp_amp_minus_sigma->image->data[i])){
	if(fft_out->image->data[i]){
	  fft_out->image->data[i] = (exp_amp_minus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
	}else{
	  fft_out->image->data[i] = exp_amp_minus_sigma->image->data[i];
	}
      }else if(cabs(fft_out->image->data[i]) > creal(exp_amp_plus_sigma->image->data[i])){
	if(fft_out->image->data[i]){
	  fft_out->image->data[i] = (exp_amp_plus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
	}else{
	  fft_out->image->data[i] = exp_amp_plus_sigma->image->data[i];
	}
      }
    }
  }
  if(opts->remove_central_pixel_phase){
    fft_out->image->data[0] = cabs(fft_out->image->data[0]);
  }

  real_out = sp_image_ifft(fft_out);
  
  /* normalize */
  tmp = 1.0/size;
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){  
    real_out->image->data[i] *= tmp;
  }

  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* A bit of documentation about the equation:

     Rs = 2*Ps-I; Rm = 2*Pm-I

     RAAR = 1/2 * beta * (RsRm + I) + (1 - beta) * Pm;    
     RAAR = 2*beta*Ps*Pm+(1-2*beta)*Pm - beta * (Ps-I)

     Which reduces to:

     Inside the support: Pm
     Outside the support: (1 - 2*beta)*Pm + beta*I
     
    */    
    if(!support->image->data[i]){
      real_out->image->data[i] = (one_minus_2_beta)*real_out->image->data[i]+beta*(real_in->image->data[i]);      
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_in,support, opts,log);
  }
  sp_image_free(fft_out);
  sp_image_free(fft_in);
  return real_out;
}



#ifdef MPI
Image * mpi_raar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  printf("mpi raar iteration\n");
  /*We're gonna have 3 kinds of images.
   With only real space constraints (Ps)
   With only fourier space constraints (Pm)
   With both constraints (PsPm) */
  static Image * exp_amp_minus_sigma = NULL;
  static Image * exp_amp_plus_sigma = NULL;
  Image * real_out;
  Image * fft_out;
  int i;
  int size = sp_c3matrix_size(real_in->image);
  real beta = get_beta(opts);
  int id;
  int p;
  fft_out = image_mpi_fft(real_in);
  MPI_Comm_rank(MPI_COMM_WORLD,&id);
  MPI_Comm_size(MPI_COMM_WORLD,&p);
  
  if(!exp_amp_minus_sigma){
    exp_amp_minus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      exp_amp_minus_sigma->image->data[i] -= exp_sigma->image->data[i];
    }
  }
  if(!exp_amp_plus_sigma){
    exp_amp_plus_sigma = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      exp_amp_plus_sigma->image->data[i] += exp_sigma->image->data[i];
    }
  }

  for(i = (id*sp_c3matrix_size(exp_amp->image))/p;i<(id+1)*sp_c3matrix_size(exp_amp->image)/p;i++){
    if(exp_amp->mask->data[i]){
      /* take the calculated phases and apply to the experimental intensities 
       leaving room for error */
      if(cabs(fft_out->image->data[i]) < creal(exp_amp_minus_sigma->image->data[i])){
	fft_out->image->data[i] = (exp_amp_minus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }else if(cabs(fft_out->image->data[i]) > creal(exp_amp_plus_sigma->image->data[i])){
	fft_out->image->data[i] = (exp_amp_plus_sigma->image->data[i])*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }
    }
  }
  real_out = sp_image_ifft(fft_out);

  for(i = (id*sp_c3matrix_size(real_out->image))/p;i<(id+1)*sp_c3matrix_size(real_out->image)/p;i++){  
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = (id*sp_c3matrix_size(real_out->image))/p;i<(id+1)*sp_c3matrix_size(real_out->image)/p;i++){  
    if(!support->image->data[i]){
      real_out->image->data[i] = (1-2*beta)*real_out->image->data[i]+beta*(real_in->image->data[i]);      
    }else{
      real_out->image->data[i] /= size;       
    }
  }
  if(opts->enforce_positivity){
    for(i = (id*sp_c3matrix_size(real_out->image))/p;i<(id+1)*sp_c3matrix_size(real_out->image)/p;i++){  
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_perioditerations == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(fft_out);
  return real_out;
}

#endif


Image * basic_error_reduction_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_c3matrix_size(real_in->image);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i]){
      real_out->image->data[i] = 0;
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


Image * basic_hpr_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  int i;
  int size = sp_c3matrix_size(real_in->image);
  real beta = get_beta(opts);
  fft_out = sp_image_fft(real_in);
  
  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      /* take the calculated phases and apply to the experimental intensities */
      pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* normalize */
    real_out->image->data[i] /= size;
  }
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* Treat points outside support*/
    if(!support->image->data[i] || cabs(2*real_out->image->data[i]-real_in->image->data[i]) < cabs((1-beta)*(real_out->image->data[i]))){
      if(opts->enforce_real){
	real_out->image->data[i] = creal(real_in->image->data[i] - beta*real_out->image->data[i]);      
      }else{
	real_out->image->data[i] = real_in->image->data[i] - beta*real_out->image->data[i];      
      }
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}



Image * basic_cflip_iteration(Image * exp_amp, Image * real_in, Image * support, 
			    Options * opts, Log * log){
  Image * real_out;
  Image * fft_out;
  Image * pattern;
  static char * weak_reflections = NULL;
  real * tmp;
  int i,j;
  int size = sp_c3matrix_size(real_in->image);
  real max = 0;
  fft_out = sp_image_fft(real_in);
  if(opts->perturb_weak_reflections && !weak_reflections){
    weak_reflections = sp_malloc(sp_c3matrix_size(exp_amp->image)*sizeof(char));
    tmp = sp_malloc(sp_c3matrix_size(exp_amp->image)*sizeof(real));
    memcpy(tmp,exp_amp->image->data,sp_c3matrix_size(exp_amp->image)*sizeof(real));
    qsort(tmp,sp_c3matrix_size(exp_amp->image),sizeof(real),descend_complex_compare);
    /* get the weak reflections threshold */
    max = tmp[(int)(sp_c3matrix_size(exp_amp->image)*opts->perturb_weak_reflections)];
    fprintf(stderr,"max - %f\nindex - %d\n",max,(int)(sp_c3matrix_size(exp_amp->image)*opts->perturb_weak_reflections));
    sp_free(tmp);
    j = 0;
    for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
      if(exp_amp->mask->data[i] && creal(exp_amp->image->data[i]) <= max){
	weak_reflections[i] = 1;
	j++;
      }else{
	weak_reflections[i] = 0;
      }
    }
    fprintf(stderr,"marked %d reflections\n",j);  
  }

  
  pattern = sp_image_duplicate(exp_amp,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_rephase(pattern,SP_ZERO_PHASE);
  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(!exp_amp->mask->data[i]){
      /*
	use the calculated amplitudes for the places
	masked out
      */
      pattern->image->data[i] = fft_out->image->data[i];
    }else{
      if(opts->perturb_weak_reflections && weak_reflections[i]){
	/* take the calculated phases, rotate PI/2 and apply to the experimental intensities */
	pattern->image->data[i] = exp_amp->image->data[i]*I*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }else{
	/* take the calculated phases and apply to the experimental intensities */	
	pattern->image->data[i] = exp_amp->image->data[i]*fft_out->image->data[i]/cabs(fft_out->image->data[i]);
      }
    }
  }
  real_out = sp_image_ifft(pattern);
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* normalize */
    if(opts->enforce_real){
      real_out->image->data[i] = creal(real_out->image->data[i])/size;
    }else{
      real_out->image->data[i] /= size;
    }
    if(cabs(real_out->image->data[i]) > max){
      max = cabs(real_out->image->data[i]);
    }
  }
  for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
    /* charge flipping doesn't know about support */
    if(cabs(real_out->image->data[i]) < opts->charge_flip_sigma*max){
      real_out->image->data[i] *= -1;
    }
  }
  if(opts->enforce_positivity){
    for(i = 0;i<sp_c3matrix_size(real_out->image);i++){
      real_out->image->data[i] =  fabs(creal(real_out->image->data[i]))+fabs(cimag(real_out->image->data[i]))*I;
    }
  }

  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_out,support, opts,log);
  }
  sp_image_free(pattern);
  sp_image_free(fft_out);
  return real_out;
}


int get_algorithm(Options * opts,Log * log){
  if(opts->automatic){
    if(log->dSupSize > 0){
      opts->algorithm = RAAR;
    }
  }
  return opts->algorithm;
}



/*! Haugazeau operator Q that describes the projector onto the intersection of two half spaces.

 * Check "A strongly convergent reflection method for finding the projection onto the intersection of two closed convex sets in a Hilbert space"
 * Journal of Approximation Theory Volume 141 ,  Issue 1  (July 2006) Pages: 63 - 69 
 * http://people.ok.ubc.ca/bauschke/Research/39.pdf
 *
 */

static Image * Q_operator(Image * x, Image * y, Image *z){
  sp_c3matrix * x_minus_y = sp_c3matrix_duplicate(x->image);
  sp_c3matrix * y_minus_z = sp_c3matrix_duplicate(y->image);
  sp_c3matrix * z_minus_y;
  sp_c3matrix_sub(x_minus_y,y->image);
  sp_c3matrix_sub(y_minus_z,z->image);
  z_minus_y = sp_c3matrix_duplicate(y_minus_z);
  sp_c3matrix_scale(z_minus_y,-1);
  Complex s;
  
  Complex pi = sp_c3matrix_froenius_prod(x_minus_y,y_minus_z);
  Complex mu = sp_c3matrix_froenius_prod(x_minus_y,x_minus_y);
  Complex nu = sp_c3matrix_froenius_prod(y_minus_z,y_minus_z);
  Complex rho = mu*nu-pi*pi;
  if(cabs(rho) == 0 && cabs(pi) >= 0){
    sp_c3matrix_free(x_minus_y);
    sp_c3matrix_free(y_minus_z);
    sp_c3matrix_free(z_minus_y);
    return sp_image_duplicate(z,SP_COPY_DATA|SP_COPY_MASK);
  }else if(cabs(rho) > 0 && cabs(pi*nu) >= cabs(rho)){
    s = cabs(1+pi/nu);
    printf("s - %f\n",cabs(s));
    sp_c3matrix_add(x->image,z_minus_y,&s);
    sp_c3matrix_free(x_minus_y);
    sp_c3matrix_free(y_minus_z);
    sp_c3matrix_free(z_minus_y);
    return sp_image_duplicate(x,SP_COPY_DATA|SP_COPY_MASK);
  }else if(cabs(rho) > 0 && cabs(pi*nu) < cabs(rho)){
    s = cabs(pi*nu/rho);
    sp_c3matrix_add(y->image,x_minus_y,&s);
    s = cabs(mu*nu/rho);
    printf("s - %f\n",cabs(s));
    sp_c3matrix_add(y->image,z_minus_y,&s);
    sp_c3matrix_free(x_minus_y);
    sp_c3matrix_free(y_minus_z);
    sp_c3matrix_free(z_minus_y);
    return sp_image_duplicate(y,SP_COPY_DATA|SP_COPY_MASK);
  }else{
    fprintf(stderr,"Cannot reach here!\n");
    abort();
  }
  return NULL;
}


Image * basic_haar_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  static Image * x = NULL;
  Image * y;
  Image * z;
  Image * ret;
  if(!x){
    x = sp_image_duplicate(real_in,SP_COPY_DATA|SP_COPY_MASK);
  }
  y = real_in;
  z = basic_raar_iteration(exp_amp,exp_sigma,real_in,support,opts,log);  
  ret = Q_operator(x,y,z);
  sp_image_free(z);
  return ret;
}

Image * basic_so2d_iteration(Image * exp_amp, Image * exp_sigma, Image * real_in, Image * support, 
			     Options * opts, Log * log){
  static sp_3matrix * Hab = NULL;
  static Image * DGs0 = NULL;
  static Image * DGns0 = NULL;
  static Image * Gs = NULL;
  static Image * Gns = NULL;
  if(!Hab){
    Hab = sp_3matrix_alloc(2,2,2);
  }
  if(!DGs0){
    DGs0 = sp_image_duplicate(real_in,SP_COPY_DETECTOR);
    DGns0 = sp_image_duplicate(real_in,SP_COPY_DETECTOR);    
  }
  if(!Gs){
    Image * gs = sp_image_duplicate(real_in,SP_COPY_DETECTOR);
    Image * gns = sp_image_duplicate(real_in,SP_COPY_DETECTOR);
    for(int i = 0;i< sp_image_size(real_in);i++){
      if(support->image->data[i]){
	gs->image->data[i] = real_in->image->data[i];
	gns->image->data[i] = 0;
      }else{
	gs->image->data[i] = 0;
	gns->image->data[i] = real_in->image->data[i];
      }
    }
    Gs = sp_image_fft(gs);
    Gns = sp_image_fft(gns);    
    sp_image_free(gs);
    sp_image_free(gns);
  }
  Image * fft_in = sp_image_duplicate(Gs,SP_COPY_DATA);
  sp_image_add(fft_in,Gns);

  minmaxL(Gs,Gns,exp_amp,support,1,1,DGs0,DGns0,Hab);

  sp_image_add(Gs,Gns);
  Image * real_out = sp_image_ifft(Gs);
  sp_image_scale(real_out,1.0/sp_image_size(real_out));
  sp_image_sub(Gs,Gns);
  if(opts->cur_iteration%opts->log_output_period == opts->log_output_period-1){
    output_to_log(exp_amp,real_in, real_out, fft_in,support, opts,log);
  }
  sp_image_free(fft_in);
  return real_out;
}
