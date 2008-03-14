
#include "spimage.h"
#include "log.h"
#include "uwrapc.h"
#include "configuration.h"
#include "support.h"
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif

Image * get_updated_support(Image * input, real level , real radius, Options * opts){
  
  real max_int = 0;
  real avg_int = 0;
  Image * res;
  long long i;

  sp_image_write(input,"support_input.vtk",0);
  Image * t = sp_image_duplicate(input,SP_COPY_DATA);
  sp_image_dephase(t);
  res = gaussian_blur(t, radius);
  sp_image_write(res,"support_blur2.vtk",0);
  sp_image_free(t);
  sp_image_dephase(res);

  for(i = 0;i<sp_image_size(res);i++){
    if(max_int < sp_cabs(res->image->data[i])){
      max_int = sp_cabs(res->image->data[i]);
    }
    avg_int += sp_cabs(res->image->data[i]);
  }
  avg_int /= sp_c3matrix_size(res->image);

  for(i = 0;i<sp_c3matrix_size(res->image);i++){
    if(sp_cabs(res->image->data[i]) < max_int*level){
      res->image->data[i] = sp_cinit(0,0);
    }else{
      res->image->data[i] = sp_cinit(1,0);
    }
  }
  if(opts->support_mask){
    sp_image_add(res,opts->support_mask);
  }
  for(i = 0;i<sp_c3matrix_size(res->image);i++){
    if(sp_cabs(res->image->data[i])){
      res->image->data[i] = sp_cinit(1,0);
    }
  }
  return res;
}


Image * get_support_from_patterson(Image * input, Options * opts){
  long long i;
  real level;
  real max_int = 0;
  
  Image * tmp_img = sp_image_duplicate(input,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_to_intensities(tmp_img);
  Image * patterson = sp_image_fft(tmp_img);

  sp_image_free(tmp_img); 
  sp_image_dephase(patterson);

  tmp_img = sp_image_shift(patterson);
  sp_image_free(patterson);
  patterson = tmp_img;

  level =  get_patterson_level(patterson, opts->patterson_blur_radius,opts);

  //sp_image_write(patterson,"autocorrelation.png",COLOR_JET|LOG_SCALE|SP_2D);
  sp_image_write(patterson,"autocorrelation.vtk",0);

  if(opts->patterson_blur_radius){
    tmp_img = gaussian_blur(patterson,opts->patterson_blur_radius);
    sp_image_free(patterson);
    patterson = tmp_img;
  }

  for(i = 0;i<sp_c3matrix_size(patterson->image);i++){
    if(max_int < sp_cabs(patterson->image->data[i])){
      max_int = sp_cabs(patterson->image->data[i]);
    }
  }


  for(i = 0;i<sp_c3matrix_size(patterson->image);i++){
    if(sp_cabs(patterson->image->data[i]) < max_int*level){
      patterson->image->data[i] = sp_cinit(0,0);
    }else{
      patterson->image->data[i] = sp_cinit(1,0);
    }
  }

  patterson->detector->image_center[0] = (sp_image_x(patterson)-1)/2;
  patterson->detector->image_center[1] = (sp_image_y(patterson)-1)/2;
  patterson->detector->image_center[2] = (sp_image_z(patterson)-1)/2;

  /* Apply oversampling square mask */
  if(opts->square_mask){
    for(i = 0;i<sp_image_size(patterson);i++){
      if(sp_image_dist(patterson,i,SP_TO_CENTER2) > sp_image_x(patterson)/4){
	patterson->image->data[i] = sp_cinit(0,0);
      }
    }
  }

  for(i = 0;i<sp_c3matrix_size(patterson->image);i++){
    if(sp_cabs(patterson->image->data[i]) > 1e-6){
      patterson->image->data[i] = sp_cinit(1,0);
    }else{
      patterson->image->data[i] = sp_cinit(0,0);
    }
  }
  if(patterson->num_dimensions == SP_2D){
    sp_image_write(patterson,"patterson_support.png",COLOR_JET);
  }else if(patterson->num_dimensions == SP_3D){
    sp_image_write(patterson,"patterson_support.vtk",0);
  }
  return patterson;  
}



/* This will return a support based not only on the level, but also on the variance of the image
   compared to the region. If it's above 3 sigma + average , it's included */
Image * get_filtered_support(Image * input, real level , real radius, Options * opts){
  real max_int = 0;
  real avg_int = 0;
  Image * res;
  Image * running_average;
  res = gaussian_blur(input, radius);
  if(sp_image_z(res) == 1){
    running_average = square_blur(res, radius, SP_2D);
  }else{
    running_average = square_blur(res, radius, SP_3D);
  }
  Image * absolute_error = sp_image_duplicate(input,SP_COPY_DATA|SP_COPY_MASK);
  Image * variance;
/*  Image * mask;
  Image * patterson_mask;*/
  long long i;
  for(i = 0;i<sp_c3matrix_size(input->image);i++){
    sp_real(absolute_error->image->data[i]) = (sp_cabs(res->image->data[i])-sp_cabs(running_average->image->data[i]))*
      (sp_cabs(res->image->data[i])-sp_cabs(running_average->image->data[i]));
    sp_imag(absolute_error->image->data[i]) = 0;
  }
  if(sp_image_z(absolute_error) == 1){
    variance = square_blur(absolute_error,radius,SP_2D);
  }else{
    variance = square_blur(absolute_error,radius,SP_3D);
  }
  sp_image_write(variance,"variance.vtk",0);
  sp_image_write(running_average,"r_avg.vtk",0);
  
  sp_image_dephase(res);
/*  mask = gaussian_blur(previous_support, radius/3);
  sp_image_dephase(mask);
  patterson_mask = gaussian_blur(patterson, radius);
  sp_image_dephase(patterson_mask);*/

  for(i = 0;i<sp_c3matrix_size(res->image);i++){
    if(max_int < sp_cabs(res->image->data[i])){
      max_int = sp_cabs(res->image->data[i]);
    }
    avg_int += sp_cabs(res->image->data[i]);
  }
  avg_int /= sp_c3matrix_size(res->image);
  for(i = 0;i<sp_c3matrix_size(res->image);i++){
    if(sp_cabs(res->image->data[i]) > max_int*level /*|| !mask->image->data[i] || !patterson->image->data[i]*/){
      res->image->data[i] = sp_cinit(1,0);
    }else if(sp_cabs(res->image->data[i]) > sp_cabs(running_average->image->data[i]) + 3* sqrt(sp_cabs(variance->image->data[i]))){
      res->image->data[i] = sp_cinit(1,0);
    }else{
      res->image->data[i] = sp_cinit(0,0);
    }
  }
  if(opts->support_mask){
    sp_image_add(res,opts->support_mask);
  }
  for(i = 0;i<sp_c3matrix_size(res->image);i++){
    if(sp_cabs(res->image->data[i])){
      res->image->data[i] = sp_cinit(1,0);
    }
  }

/*  sp_image_free(mask);
  sp_image_free(patterson_mask);*/
  sp_image_free(running_average);
  sp_image_free(absolute_error);
  sp_image_free(variance);
  return res;
}



/* This function returns the threshold level used for the support in the current iteration.
   This depends on the kind of algorithm used and user options.
   In the future it's possible that some support update algorithms cannot use a simple threshold
   method for defining the support so other methods might have to be used 
*/
real get_support_level(Image * input, real * previous_size , real radius, Log * log, Options * opts){
  static int stepped_flag = 0;
  real max_int = 0;
  long long new_size;
  real new_level;
  Image * res;
  real reduction = 0;
  real real_error_threshold;
  long long i;
  
  if(opts->support_update_algorithm == FIXED){
    /* Simplest case, constant level  */     
    return opts->new_level;
  }else if(opts->support_update_algorithm == STEPPED){
    /* Use a 2 step level, mostly only useful for HIO iterations */
    if(opts->cur_iteration > 40 && log->Ereal > 0.3){
      stepped_flag = 1;
    }
    if(stepped_flag){
      return opts->new_level*0.75;
    }
    return opts->new_level;
  }else if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
    /* Return a constant level untill the real space error reaches
       a threshold, then return -1
    */
    if(log->Ereal < opts->real_error_threshold){
      return opts->new_level;
    }else{
      return -1;
    }
  }else if(opts->support_update_algorithm ==  REAL_ERROR_ADAPTATIVE){
    /* Tries to reduce the support while keeping the real space error at a constant value */
    if(opts->real_error_threshold < 0){
      if(get_algorithm(opts,log) == HIO){
	real_error_threshold = (real)0.55;
      }else if(get_algorithm(opts,log) == RAAR){
	real_error_threshold = (real)0.20;
      }else{
	abort();
      }
    }else{
      real_error_threshold = opts->real_error_threshold;
    }

    reduction = 1-(real_error_threshold-log->Ereal)/10.0;

    res = gaussian_blur(input, radius);
    sp_image_dephase(res);
    
    for(i = 0;i<sp_c3matrix_size(res->image);i++){
      if(max_int < sp_cabs(res->image->data[i])){
	max_int = sp_cabs(res->image->data[i]);
      }
    }
    qsort(res->image->data,sp_c3matrix_size(res->image),sizeof(Complex),descend_complex_compare);
    if((*previous_size) < 0){
      for(i = 0;i<sp_c3matrix_size(res->image);i++){
	if(sp_cabs(res->image->data[i]) < -(*previous_size)){
	  break;
	}
      }
      *previous_size = i;
    }
    new_size = (*previous_size)*reduction;
    *previous_size = new_size;
    new_level = sp_cabs(res->image->data[new_size])/max_int;
    sp_image_free(res);
    return new_level;
  }else if(opts->support_update_algorithm == CONSTANT_AREA){
    /* Keeps a constant area for the support */    
    Image * t = sp_image_duplicate(input,SP_COPY_DATA);
    sp_image_dephase(t);
    res = gaussian_blur(t, radius);
    sp_image_free(t);
    qsort(res->image->data,sp_c3matrix_size(res->image),sizeof(Complex),descend_complex_compare);
    /* the level is always a fraction of the maximum value so we divide by the maximum (data[0]) */
    new_level = sp_cabs(res->image->data[(int)(sp_image_size(res)*opts->object_area)])/sp_cabs(res->image->data[0]);
    sp_image_free(res);
    return new_level;
  }else if(opts->support_update_algorithm == DECREASING_AREA){
    /* Keeps a constant area for the support */    
    Image * t = sp_image_duplicate(input,SP_COPY_DATA);
    sp_image_dephase(t);
    res = gaussian_blur(t, radius);
    sp_image_free(t);
    qsort(res->image->data,sp_c3matrix_size(res->image),sizeof(Complex),descend_complex_compare);
    /* the level is always a fraction of the maximum value so we divide by the maximum (data[0]) */
    new_level =  sp_cabs(res->image->data[(int)(sp_image_size(res)*get_object_area(opts))])/sp_cabs(res->image->data[0]);    
    sp_image_free(res);
    return new_level;
  }else{
    fprintf(stderr,"Unkown algorithm!\n");
    abort();
  }
  return 0;
}



/* This function returns the threshold level used for the patterson threshold in the first iteration.
   This depends on the kind of algorithm used and user options.
   In the future it's possible that some patterson selection algorithms cannot use a simple threshold
   method for defining the support so other methods might have to be used 
*/
real get_patterson_level(Image * input, real radius, Options * opts){
  Image * res;
  
  if(opts->patterson_level_algorithm == FIXED){
    /* Simplest case, constant level  */     
    return opts->init_level;
  }else if(opts->patterson_level_algorithm == CONSTANT_AREA){
    /* Keeps a constant area for the support */    
    if(radius){
      res = gaussian_blur(input, radius);
    }else{
      res = sp_image_duplicate(input,SP_COPY_DATA|SP_COPY_MASK);
    }
    sp_image_dephase(res);
    qsort(res->image->data,sp_c3matrix_size(res->image),sizeof(Complex),descend_complex_compare);
    /* the level is always a fraction of the maximum value so we divide by the maximum (data[0]) */
    return sp_cabs(res->image->data[(int)(sp_image_size(res)*opts->object_area)])/sp_cabs(res->image->data[0]);    
  }else{
    fprintf(stderr,"Unkown algorithm!\n");
    abort();
  }
  return 0;
}

int descend_real_compare(const void * pa,const void * pb){
  real a,b;
  a = *((real *)pa);
  b = *((real *)pb);
  if(a < b){
    return 1;
  }else if(a == b){
    return 0;
  }else{
    return -1;
  }
}

int descend_complex_compare(const void * pa,const void * pb){
  Complex a,b;
  a = *((Complex *)pa);
  b = *((Complex *)pb);
  if(sp_cabs(a) < sp_cabs(b)){
    return 1;
  }else if(sp_cabs(a) == sp_cabs(b)){
    return 0;
  }else{
    return -1;
  }
}

