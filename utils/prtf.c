#include <stdlib.h>
#include <float.h>
#include <hdf5.h>
#include <fftw3.h>
#include "spimage.h"

/* Calculates the Phase Retrieval Transfer Function of a bunch of images */


real phase_error(Image * a, Image * b){
  real error = 0;
  int i;
  for(i = 0;i<sp_image_size(a);i++){
    error += fabs(cargr(a->image->data[i])-cargr(b->image->data[i]));
  }
  return error/sp_image_size(a);
}

void fourier_translation(Image * a, real t_x, real t_y){
  /* fourier frequency x*/
  int f_x;
  /* fourier frequency y*/
  int f_y;
  int i = 0;
  real nx_inv = 1.0/sp_image_width(a);
  real ny_inv = 1.0/sp_image_height(a);
  int x,y;
  real two_pi = 2*M_PI;
  for(x = 0;x<sp_image_width(a);x++){    
    if(x < sp_image_width(a)/2){
      f_x = x;
    }else{
      f_x = -(sp_image_width(a)-x);
    }
    for(y = 0;y<sp_image_height(a);y++){
      if(y < sp_image_height(a)/2){
	f_y = y;
      }else{
	f_y = -(sp_image_height(a)-y);
      }
      a->image->data[i++] *= cexp(-I * f_x * t_x * nx_inv * two_pi) * cexp(-I * f_y * t_y * ny_inv * two_pi);
    }
  }
}


/* This functions translates image b so that it's phases 
   are as close as possible to a.
   The translation is done in fourier space and both images
   should be in fourier space */
void maximize_overlap(Image * a, Image * b){
  int index,x,y;
  Image * cc;
  Image * tmp = sp_image_ifft(a);
  Image *tmp2 = sp_image_ifft(b);
  cc = sp_image_cross_correlate(tmp,tmp2,NULL);
  sp_image_free(tmp);
  sp_image_free(tmp2);

  sp_image_max(cc,&index,&x,&y);
  if(x > sp_image_width(cc)/2){
    x = -(sp_image_width(cc)-x);
  }
  if(y > sp_image_height(cc)/2){
    y = -(sp_image_height(cc)-y);
  }
  sp_image_free(cc);
  if(x != 0 || y != 0){
    fourier_translation(b,x,y);
  }
}

/* This functions translates image b so that it's phases 
   are as close as possible to a.
   The translation is done in fourier space and both images
   should be in fourier space */
void maximize_phase_overlap(Image * a, Image * b){
  real error;
  real min;
  real x;
  real y;
  real min_x = 0;
  real min_y = 0;
  Image * tmp;
  min = phase_error(a,b);
  for(x = -1;x<=1;x+= 1){  
    for(y = -1;y<=1;y+= 1){  
      tmp = sp_image_duplicate(b,SP_COPY_DATA|SP_COPY_MASK);
      fourier_translation(tmp,x,y);
      error = phase_error(a,tmp);
      sp_image_free(tmp);
      printf("Error - %f  x - %f y - %f\n",error,x,y);
      if(error < min){
	min = error;
	min_x = x;
	min_y = y;	
      }
    }
  }
  fourier_translation(b,min_x,min_y);


  printf("Min Error - %f  x - %f y - %f\n",error,min_x,min_y);
}

void rescale_image(Image * a){
  double sum = 0;
  int i;
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    sum += a->image->data[i];
  }
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    a->image->data[i] /= sum;
  } 
}

#define NBINS 100
int main(int argc, char ** argv){
  Image * img;
  Image * amps;
  Image * sum;
  Image * tmp;
  real bins[NBINS];
  int bin_count[NBINS];
  real avg_prtf;
  real max_res;
  Image * prtf;
  int bin;
  int i;
  if(argc < 3){
    printf("Usage: %s <image1> [image2] ...\n",argv[0]);
    exit(0);
  }

  img = read_imagefile(argv[1]);
  sp_image_dephase(img);
  sum = sp_image_fft(img);
  amps = sp_image_fft(img);
  sp_image_dephase(amps);
  sp_image_free(img);
  for(i = 1;i<argc-2;i++){
    img = read_imagefile(argv[i+1]);
    sp_image_dephase(img);
    tmp = sp_image_fft(img);
    maximize_overlap(sum,tmp);
    sp_image_add(sum,tmp);
    sp_image_dephase(tmp);
    sp_image_add(amps,tmp);
    sp_image_free(img);
    sp_image_free(tmp);
  }
  prtf = sp_image_duplicate(sum,SP_COPY_DATA|SP_COPY_MASK);
  avg_prtf = 0;
  max_res = sp_image_dist(sum,(sp_image_width(sum)/2.0)*sp_image_height(sum)+sp_image_height(sum)/2.0,SP_TO_CORNER);
  for(i = 0;i<NBINS;i++){
    bins[i] = 0;
    bin_count[i] = 0;
  }
  sp_image_dephase(prtf);
  for(i = 0;i<sp_image_size(sum);i++){
    prtf->image->data[i] /= (amps->image->data[i]+FLT_EPSILON);
    avg_prtf += prtf->image->data[i];
    bin = (NBINS-1)*sp_image_dist(sum,i,SP_TO_CORNER)/max_res;
    bins[bin] += prtf->image->data[i];
    bin_count[bin]++;
  }
  sp_image_write(sum,"avg_fft.h5",sizeof(real));
  sp_image_write(amps,"amps.h5",sizeof(real));
  sp_image_write(prtf,"prtf.h5",sizeof(real));
  avg_prtf /= sp_image_size(sum);
/*  printf("Average PRTF - %f\n",avg_prtf);
  printf("Resolution bined PRTF\n");*/
  for(i = 0;i<NBINS;i++){
    if(bin_count[i]){
      bins[i] /= bin_count[i];
    }
    printf("%f %f\n",i*max_res/NBINS,bins[i]);    
  }
  return 0;
}
