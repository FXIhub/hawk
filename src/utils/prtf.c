
#include <stdlib.h>
#include <float.h>
#include <hdf5.h>
#include <fftw3.h>
#include <getopt.h>
#include "spimage.h"

/* Calculates the Phase Retrieval Transfer Function of a bunch of images
   (pixels which have at least in one reconstruction no defined phase (value = zero) lead to a zero value in the prtf)
*/


real phase_error(Image * a, Image * b){
  real error = 0;
  long long i;
  for(i = 0;i<sp_image_size(a);i++){
    real tmp_error = (sp_carg(a->image->data[i])-sp_carg(b->image->data[i]));
    /* make sure error is pi at most */
    while(tmp_error < -M_PI){
      tmp_error += 2*M_PI;
    }
    while(tmp_error > M_PI){
      tmp_error -= 2*M_PI;
    }
    error += fabs(tmp_error);
  }
  return error/sp_image_size(a);
}

void fourier_translation(Image * a, real t_x, real t_y, real t_z){
  /* fourier frequency x*/
  int f_x;
  /* fourier frequency y*/
  int f_y;
  /* fourier frequency z*/
  int f_z;
  long long i = 0;
  real nx_inv = 1.0/sp_image_x(a);
  real ny_inv = 1.0/sp_image_y(a);
  real nz_inv = 1.0/sp_image_z(a);
  int x,y,z;
  real two_pi = 2*M_PI;
  for(z = 0;z<sp_image_z(a);z++){    
    if(z < sp_image_z(a)/2){
      f_z = z;
    }else{
      f_z = -(sp_image_z(a)-z);
    }
    for(y = 0;y<sp_image_y(a);y++){
      if(y < sp_image_y(a)/2){
	f_y = y;
      }else{
	f_y = -(sp_image_y(a)-y);
      }
      for(x = 0;x<sp_image_x(a);x++){
	if(x < sp_image_x(a)/2){
	  f_x = x;
	}else{
	  f_x = -(sp_image_x(a)-x);
	}
	a->image->data[i] = sp_cmul(a->image->data[i],
				       sp_cinit(cos(f_x * t_x * nx_inv * two_pi+f_y * t_y * ny_inv * two_pi+f_z * t_z * nz_inv * two_pi),
						-sin(f_x * t_x * nx_inv * two_pi+f_y * t_y * ny_inv * two_pi+f_z * t_z * nz_inv * two_pi)));
	i++;
      }
    }
  }
}


/* This functions translates image b so that it's phases 
   are as close as possible to a.
   The translation is done in fourier space and both images
   should be in fourier space */
void maximize_overlap(Image * a, Image * b){
  int x,y,z;
  long long index;
  real t_max,max;
  Image * cc;
  Image * cc2;
  Image * tmp = sp_image_ifft(a);
  Image *tmp2 = sp_image_ifft(b);
  /* Check superposition with normal and rotated image */
  cc = sp_image_cross_correlate(tmp,tmp2,NULL);
  cc2 = sp_image_convolute(tmp,tmp2,NULL);

  sp_image_free(tmp);

  t_max = sp_image_max(cc,&index,&x,&y,&z);
  fprintf(stderr,"t_max = %f\n",t_max);
  max = sp_image_max(cc2,&index,&x,&y,&z);
  fprintf(stderr,"max = %f\n",max);
  if( max > t_max){
    fprintf(stderr,"Rotating image\n");
    /* Do the flip in real space */
    sp_image_reflect(tmp2,IN_PLACE,SP_ORIGO);
    tmp =  sp_image_fft(tmp2);
    /* normalize*/
    sp_image_scale(tmp,1.0/sp_image_size(tmp));
    sp_image_memcpy(b,tmp);
    
  }else{
    max = sp_image_max(cc,&index,&x,&y,&z);
  }
  sp_image_free(tmp2);

  if(x > sp_image_x(cc)/2){
    x = -(sp_image_x(cc)-x);
  }
  if(y > sp_image_y(cc)/2){
    y = -(sp_image_y(cc)-y);
  }
  if(z > sp_image_z(cc)/2){
    z = -(sp_image_z(cc)-z);
  }
  sp_image_free(cc);
  sp_image_free(cc2);
  if(x != 0 || y != 0 || z != 0){
    if(max > t_max){
      x = -x;
      y = -y;
      z = -z;
    }
    fprintf(stderr,"Translating by %d %d %d\n",x,y,z);
    fourier_translation(b,x,y,z);
  }
}

/* This functions translates image b so that it's phases 
   are as close as possible to a.
   The translation is done in fourier space and both images
   should be in fourier space */
void maximize_phase_overlap(Image * a, Image * b){
  real error;
  real min;
  real x,y,z;
  real min_x = 0;
  real min_y = 0;
  real min_z = 0;
  Image * tmp;
  min = phase_error(a,b);
  for(x = -1;x<=1;x+= 1){
    for(y = -1;y<=1;y+= 1){
      for(z = -1;z<=1;z+= 1){
	tmp = sp_image_duplicate(b,SP_COPY_DATA|SP_COPY_MASK);
	fourier_translation(tmp,x,y,z);
	error = phase_error(a,tmp);
	sp_image_free(tmp);
	printf("Error - %f  x - %f y - %f z - %f\n",error,x,y,z);
	if(error < min){
	  min = error;
	  min_x = x;
	  min_y = y;
	  min_z = z;
	}
      }
    }
  }
  fourier_translation(b,min_x,min_y,min_z);

  printf("Min Error - %f  x - %f y - %f z - %f\n",error,min_x,min_y,min_z);
}

void rescale_image(Image * a){
  double sum = 0;
  long long i;
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    sum += sp_real(a->image->data[i]);
  }
  for(i = 0;i<sp_c3matrix_size(a->image);i++){
    sp_real(a->image->data[i]) /= sum;
  } 
}

#define NBINS 100
int main(int argc, char ** argv){
  Image * img;
  Image * amps;
  Image * sum;
  Image * tmp;
  Image * tmpamps;
  Image * zeros;
  real avg_prtf;
  real max_res;
  Image * prtf;
  Image * avg_img;
  int bin;
  real v;
  long long i;
  char buffer2[1024] = "";
  char * output;
  FILE * f;
  int no_translation = 0;
  real bin_num[NBINS] = {0.0};
  int bin_den[NBINS] = {0};

  int c;
  //int opterr = 0;
  while((c = getopt(argc,argv,"tT")) != -1) {
    switch(c) {
    case('T'):
      no_translation = 1;
      break;
    case('t'):
      no_translation = 0;
      break;
    }
  }

  if((argc - optind) < 2){
    printf("Usage: %s [options] <output file>  <image1> [image2] ...\navailable options are:\n  -t   Translate the images to match densities (default)\n  -T   Do not translate images\n",argv[0]);
    exit(0);
  }
  output = argv[optind];
  sprintf(buffer2,"%s.log",output);
  f = fopen(buffer2,"w");
  for(i = 0;i<argc;i++){
    fprintf(f,"%s ",argv[i]);
  }
  fprintf(f,"\n");
  fclose(f);

  // first file: initialization
  img = sp_image_read(argv[optind+1],0);
  avg_img = sp_image_duplicate(img,SP_COPY_ALL);
  sum = sp_image_fft(img);
  amps = sp_image_duplicate(sum,SP_COPY_ALL);
  zeros = sp_image_alloc(sp_image_x(img),sp_image_y(img),sp_image_z(img));

  /* Normalize sum and count zeros */
  for(int j = 0;j<sp_image_size(sum);j++){
    if(sp_cabs(sum->image->data[j])>0.0){
      sum->image->data[j] = sp_cscale(sum->image->data[j],1.0/sp_cabs(sum->image->data[j]));
    }
    else{
      zeros->mask->data[j]+=1;
    }
  }
  sp_image_dephase(amps);
  sp_image_free(img);


  for(i = optind+2;i<argc;i++){
    img = sp_image_read(argv[i],0);
    if(!img){
      fprintf(stderr,"Could not open %s. Skipping.\n.",argv[i]);
      continue;
    }
    if (no_translation == 0) {
      sp_image_superimpose(avg_img,img,SpEnantiomorph);
      sp_image_phase_match(avg_img,img,2);
    }
    sp_image_add(avg_img,img);
    char buff2[1024];
    if(img->num_dimensions == SP_2D){
      sprintf(buff2,"%s-super.png",argv[i+1]);
      sp_image_write(img,buff2,SpColormapWheel);
    }
    tmp = sp_image_fft(img);
    tmpamps = sp_image_duplicate(tmp,SP_COPY_ALL);
    sp_image_dephase(tmpamps);
    sp_image_add(amps,tmpamps);

    /* Normalize tmp and count zeros */
    for(int j = 0;j<sp_image_size(tmp);j++){
      if(sp_cabs(tmp->image->data[j])>0.0){
	tmp->image->data[j] = sp_cscale(tmp->image->data[j],1.0/sp_cabs(tmp->image->data[j]));
      }
      else{
	zeros->mask->data[j]+=1;
      }
    }
    sp_image_add(sum,tmp);
    sp_image_free(img);
    sp_image_free(tmp);
    sp_image_free(tmpamps);
  }

  prtf = sp_image_duplicate(sum,SP_COPY_DATA|SP_COPY_MASK);
  for(i = 0;i<sp_image_size(prtf);i++){
    if(zeros->mask->data[i] != 0){
      prtf->image->data[i] = sp_cinit(0.0,0.0);
	}
    else{
      prtf->image->data[i] = sp_cscale(prtf->image->data[i], 1.0 / ( (double) argc -2.0)) ;
	}
  }
  sp_image_dephase(prtf);

  sp_image_mask_to_image(zeros,zeros);

  avg_prtf = 0;
  max_res = sp_image_dist(sum,(sp_image_z(sum)/2)*sp_image_y(sum)*sp_image_x(sum)+(sp_image_y(sum)/2)*sp_image_x(sum)+sp_image_x(sum)/2,SP_TO_CORNER);

  /*for(i = 0;i<NBINS;i++){
    bin_num[i] = 0;
    bin_den[i] = FLT_EPSILON;
  }
  */

  for(i = 0;i<sp_image_size(sum);i++){
    //    sp_real(prtf->image->data[i]) /= (sp_real(amps->image->data[i])+FLT_EPSILON);
    bin = (NBINS-1)*sp_image_dist(sum,i,SP_TO_CORNER)/max_res;
    bin_num[bin] += sp_real(prtf->image->data[i]);
    bin_den[bin] += 1;
    avg_prtf += sp_real(prtf->image->data[i]);
  }

  sprintf(buffer2,"%s-avg_fft.h5",output);
  sp_image_write(sum,buffer2,sizeof(real));
  sprintf(buffer2,"%s-avg_image.h5",output);
  sp_image_write(avg_img,buffer2,sizeof(real));
  if(avg_img->num_dimensions == SP_2D){
    sprintf(buffer2,"%s-avg_image-phase.png",output);
    sp_image_write(avg_img,buffer2,SpColormapWheel);
    sprintf(buffer2,"%s-avg_image.png",output);
    sp_image_write(avg_img,buffer2,SpColormapJet);
  }
  sprintf(buffer2,"%s-amps.h5",output);
  sp_image_write(amps,buffer2,sizeof(real));
  sprintf(buffer2,"%s-prtf.h5",output);
  sp_image_write(prtf,buffer2,sizeof(real));
  avg_prtf /= sp_image_size(sum);
  f = fopen(output,"w");  
  for(i = 0;i<NBINS;i++){
    if(bin_den[i] != 0){
      v = bin_num[i]/bin_den[i];
    }
    else{
      v = 0.0;
    }
    fprintf(f,"%f %f\n",i*max_res/NBINS,v);
  }
  return 0;
}
