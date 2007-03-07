#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "spimage.h"
#include "deautocorrelate.h"

Image * estimate_lambda(Image * R_tilde){
  /* find the point furthest away from the center of the autocorrelation */
  int i,point;
  real max_dist = 0;
  Image * res = imgcpy(R_tilde);
  for(i = 0;i<sp_cmatrix_size(R_tilde->image);i++){
    if(R_tilde->image->data[i] && dist_to_corner(i,R_tilde) > max_dist){
      point = i;
      max_dist = dist_to_corner(i,R_tilde);
    }
  }
  for(i = 0;i<sp_cmatrix_size(R_tilde->image);i++){
    if(R_tilde->image->data[(i+point)%sp_cmatrix_size(R_tilde->image)] == 0){
      res->image->data[i] = 0;
    }
  }
  for(i = 0;i<sp_cmatrix_size(R_tilde->image);i++){
    res->image->data[i] = 1+p_drand48()/10.0;
  }
  return res;
}

Image * schulz_snyder_iteration(Image * lambda, Image * R_tilde){
  Image * res = imgcpy(lambda);
  /* big summation we need a double image */
  double * sum = malloc(sizeof(double)*sp_cmatrix_size(lambda->image));
  real normalization = 1/sqrt(integrated_intensity(R_tilde));
  int x,y;
  int csym = 0;
  Image * R_lambda = cross_correlate_img(lambda,lambda,NULL);
  memset(sum,0,sizeof(double)*sp_cmatrix_size(lambda->image));
  for(x = 0;x<sp_cmatrix_size(lambda->image);x++){
/*    if((x)%10000 == 0){
      printf("%6.4f%\n",100.0*x/sp_cmatrix_size(lambda->image));      
    }*/
    if(!lambda->image->data[x]){
      continue;
    }
    for(y = 0;y<sp_cmatrix_size(lambda->image);y++){
      if(R_tilde->image->data[y] && lambda->image->data[(x+y)%sp_cmatrix_size(lambda->image)] && R_lambda->image->data[y]){
	csym = ((sp_cmatrix_cols(R_tilde->image)-y/sp_cmatrix_rows(R_tilde->image))%sp_cmatrix_cols(R_tilde->image))*sp_cmatrix_rows(R_tilde->image)+
	  (sp_cmatrix_rows(R_tilde->image)-y%sp_cmatrix_rows(R_tilde->image))%sp_cmatrix_rows(R_tilde->image);
	sum[x] += lambda->image->data[(x+y)%sp_cmatrix_size(lambda->image)]*0.5*
	  (R_tilde->image->data[y] + (R_tilde->image->data[csym]))/R_lambda->image->data[y];
/*	sum[x] += lambda->image->data[(x+y)%sp_cmatrix_size(lambda->image)]*1*
	  (R_tilde->image->data[y])/R_lambda->image->data[y];*/
      }
      
    }
    sum[x] *= normalization*lambda->image->data[x];
  }
  for(x = 0;x<sp_cmatrix_size(res->image);x++){
    res->image->data[x] = sum[x];
    if(creal(res->image->data[x]) < 1e-5){
      res->image->data[x] = 0;
    }
  }
  free(sum);
  freeimg(R_lambda);
  return res;
}


Image * fast_schulz_snyder_iteration(Image * lambda, Image * R_tilde){
  Image * res = imgcpy(lambda);
  /* big summation we need a double image */
  static real normalization = 0;
  int i;
  int csym = 0;
  Image * R_over_R_lambda = cross_correlate_img(lambda,lambda,NULL);
  Image * correlation;

  if(!normalization){
    normalization = 1/sqrt(integrated_intensity(R_tilde));
  }
  for(i = 0;i<sp_cmatrix_size(R_over_R_lambda->image);i++){
    csym = ((sp_cmatrix_cols(R_tilde->image)-i/sp_cmatrix_rows(R_tilde->image))%sp_cmatrix_cols(R_tilde->image))*sp_cmatrix_rows(R_tilde->image)+
      (sp_cmatrix_rows(R_tilde->image)-i%sp_cmatrix_rows(R_tilde->image))%sp_cmatrix_rows(R_tilde->image);
    if(R_over_R_lambda->image->data[i]){
      R_over_R_lambda->image->data[i] = (0.5*R_tilde->image->data[i]+R_tilde->image->data[csym])/R_over_R_lambda->image->data[i];
    }else{
      R_over_R_lambda->image->data[i] = 0;
    }
  }
  correlation = cross_correlate_img(lambda,R_over_R_lambda,NULL);
  
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    res->image->data[i] = correlation->image->data[i]*normalization*lambda->image->data[i];
/*    if(res->image->data[i] < 1e-5){
      res->image->data[i] = 0;
    }*/
  }
  freeimg(R_over_R_lambda);
  freeimg(correlation);
  return res;
}

Image * intersect_images(Image * a, Image * b){
  Image * res = imgcpy(a);
  int i;
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    if(a->image->data[i] && b->image->data[i]){
      res->image->data[i] = 1;
    }else{
      res->image->data[i] = 0;
    }
  }
  return res;    
}

real get_image_max(Image * a){
  return creal(sp_cmatrix_max(a->image,NULL));
}

Image * get_support(Image * a){
  Image *res = imgcpy(a);
  real thres = (real)0.68;
  real local_thres;
  real max_dist = dist_to_center(0,a)/3;
  int i;
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    a->image->data[i] = log(a->image->data[i]+1);
  }
  thres *= get_image_max(a);
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    local_thres = thres/(30.0*(dist_to_corner(i,a)/max_dist)+1);
    /* gaussian decay from center to edge */
    local_thres = thres * (1+exp(-9*dist_to_corner(i,a)*dist_to_corner(i,a)/(2*max_dist*max_dist)));
/*    local_thres = thres;*/
    if(creal(a->image->data[i]) < local_thres){
      res->image->data[i] = 0;
    }else{
      res->image->data[i] = 1;
    }
  }
  return res;
}

int main(int argc, char ** argv){
  Image * img;
  Image * ac;
  Image * lambda;
  Image * new_lambda;
  Image * tmp;
  int i;
  char buffer[1024];
  img = read_imagefile(argv[1]);
  dephase(img);

  ac = cross_correlate_img(img,img,NULL);
  ac->shifted = 1;
  ac = limit_resolution(ac,256);

  write_png(ac,"ac.png",COLOR_JET | LOG_SCALE);
/*  A = get_support(ac);
  write_png(A,"ac_sup.png",COLOR_JET);*/
  lambda = estimate_lambda(ac);
/*  lambda = imgcpy(ac);*/
  write_png(lambda,"init_lambda.png",COLOR_JET);
  write_img(lambda,"init_lambda.h5",sizeof(real));
  for(i = 0;i<100000;i++){
    new_lambda = fast_schulz_snyder_iteration(lambda, ac);
    freeimg(lambda);
    lambda = new_lambda;
    if(i%20 == 0){
      lambda->shifted = 1;
      tmp = shift_quadrants(lambda);
      sprintf(buffer,"lambda-%06d.png",i);
      write_png(tmp,buffer,COLOR_JET|LOG_SCALE);
      sprintf(buffer,"lambda-%06d.vtk",i);
      write_vtk(tmp,buffer);
      freeimg(tmp);
    }
  }
  return 0;  
}
