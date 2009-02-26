#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "spimage.h"


Image * remove_window(Image * fo_with_phases){
  Image * real_img = image_rev_fft(fo_with_phases);
  Image * res;
  int i;
  for(i = 0;i<sp_cmatrix_size(real_img->image);i++){
    if(dist_to_center(i,real_img) > 30){
      real_img->image->data[i] = 0;
    }
  }
  write_png(real_img,"cleaned_image.png",COLOR_JET);
  res = image_fft(real_img);
  for(i= 0;i<sp_cmatrix_size(res->image);i++){
    res->image->data[i] /= sp_cmatrix_size(res->image);
  }
  return res;
}

int main(int argc, char ** argv){
  Image * fobs;
  Image * filter_fobs;
  Image * sol;
  Image * fc;
  Image * pos_fc;
  Image * fo_fc;
  Image * fo_with_fc_phases;
  Image * windowless_pattern;
  Image * windowless_real;
  Image * tmp;
  Image * phases;
  Image * image_ratio;
  Image * unshifted_windowless_pattern;
  Image * fc_phases;
  double R;
  double avg_fobs = 0;
  double avg_fcalc = 0;
  double avg_filter_fobs = 0;
  double low_intensity_cutoff = 0;
  int n = 0;
  int i;
  if(argc < 3){
    fprintf(stderr,"Usage: %s <fobs.h5> <solution.h5>\n",argv[0]);
    exit(0);
  }
  fobs = read_imagefile(argv[1]);
/*  for(i = 0;i<sp_cmatrix_size(fobs);i++){
    if(dist_to_axis(i,fobs) < 20){
      fobs->mask[i] = 0;
    }
  }*/
  sol = read_imagefile(argv[2]);
  if(!sol->phased){
    fprintf(stderr,"Solution not phased! You probably switched the arguments.\n");
    exit(1);
  }
  /*just to bypass a previous bug */
  sol->shifted = 0;
  fc = image_fft(sol);


  pos_fc = imgcpy(sol);
  dephase(pos_fc);
  pos_fc = image_fft(pos_fc);

  write_png(pos_fc,"pos_fc.png",COLOR_JET|LOG_SCALE);  
  write_img(pos_fc,"pos_fc.h5",sizeof(real));
  image_ratio = imgcpy(pos_fc);
  for(i = 0;i<sp_cmatrix_size(pos_fc->image);i++){
    if(fobs->mask->data[i] && fobs->image->data[i]){
      image_ratio->image->data[i] = pos_fc->image->data[i] / fobs->image->data[i];
      
    }
  }
  write_png(image_ratio,"pos_fc_image_ratio.png",COLOR_JET);
  write_img(image_ratio,"pos_fc_image_ratio.h5",sizeof(real));
  for(i = 0;i<sp_cmatrix_size(pos_fc->image);i++){
    if(fobs->mask->data[i] && fobs->image->data[i]){
      image_ratio->image->data[i] = fc->image->data[i] / fobs->image->data[i];
      
    }
  }
  write_png(image_ratio,"fc_image_ratio.png",COLOR_JET);
  write_img(image_ratio,"fc_image_ratio.h5",sizeof(real));


  exit(0);

  fc_phases = sp_image_get_phases(fc);
  for(i = 0;i<sp_cmatrix_size(fc_phases->image);i++){
    fc_phases->image->data[i] = creal(fc_phases->image->data[i])*fabs(creal(fc_phases->image->data[i]))+
      cimag(fc_phases->image->data[i])*fabs(cimag(fc_phases->image->data[i]))*I;
  }
/*  write_png(fc_phases,"fc_phases.png",COLOR_JET);*/
  write_img(fc_phases,"fc_phases.h5",sizeof(real));
  write_img(image_rev_fft(fc_phases),"fc_phases_fft.h5",sizeof(real));
/*  write_png(shift_quadrants(image_rev_fft(fc_phases)),"fc_phases_fft.png",COLOR_JET|LOG_SCALE);*/
  tmp = cross_correlate_img(sol,sol,NULL);
  tmp->shifted = 1;
  write_img(shift_quadrants(tmp),"autoconvolution.h5",sizeof(real));
/*  write_img(cross_correlate_img(sol,sol),"autocorrelation.h5",sizeof(real));*/
  fo_with_fc_phases = imgcpy(fc);
  for(i = 0;i<sp_cmatrix_size(fo_with_fc_phases->image);i++){
    if(fobs->mask->data[i]){
      fo_with_fc_phases->image->data[i] = fc->image->data[i]*cabs(fobs->image->data[i])/cabs(fc->image->data[i]);
    }
  }
  tmp = image_rev_fft(fo_with_fc_phases);
  for(i = 0;i<sp_cmatrix_size(fo_with_fc_phases->image);i++){
    if(square_dist_to_center(i,tmp) > 120){
      tmp->image->data[i] = 0;
    }
  }
/*  dephase(tmp);*/
  write_png(tmp,"tmp.png",COLOR_JET);
  filter_fobs = image_fft(tmp);
  for(i = 0;i<sp_cmatrix_size(filter_fobs->image);i++){
    filter_fobs->image->data[i] /= sp_cmatrix_size(filter_fobs->image);
  }

  
  fo_with_fc_phases->shifted = 1;
/*  filter_fobs = low_pass_square_filter(fo_with_fc_phases,120);*/
  R = r_factor(fobs,fc,low_intensity_cutoff);
  n = 0;
  for(i = 0;i<sp_cmatrix_size(fobs->image);i++){
    if(fobs->mask->data[i]){
      avg_fobs += cabs(fobs->image->data[i]);
      avg_fcalc += cabs(fc->image->data[i]);
      avg_filter_fobs += cabs(filter_fobs->image->data[i]);
      n++;
    }
  }
  avg_fobs /= n;
  avg_fcalc /= n;
  avg_filter_fobs /= n;
  /* Assume 4 time oversample */
  printf("R factor - %f\n",R);
  R = r_factor(filter_fobs,fc,low_intensity_cutoff);
  write_png(fobs,"fobs.png",COLOR_JET);
  write_png(filter_fobs,"filter_fobs.png",COLOR_JET);
  fo_fc = imgcpy(fobs);
  for(i = 0;i<sp_cmatrix_size(fobs->image);i++){
    if(fo_fc->mask->data[i] && cabs(fobs->image->data[i]) > low_intensity_cutoff ){
      fo_fc->image->data[i] = cabs(fo_fc->image->data[i]-fc->image->data[i])/cabs(fo_fc->image->data[i]+fc->image->data[i]);
    }else{
      fo_fc->image->data[i] = 0;
    }
  }
  write_png(fo_fc,"fo_minus_fc.png",COLOR_JET);
  printf("filter R factor - %f\n",R);
  printf("<fobs> - %f\n",avg_fobs);
  printf("<fcalc> - %f\n",avg_fcalc);
  printf("<filter_fobs> - %f\n",avg_filter_fobs);
  write_png(image_rev_fft(fo_with_fc_phases),"fo_with_fc_phases.png",COLOR_GRAYSCALE);
  windowless_pattern = remove_window(fo_with_fc_phases);
  windowless_real = imgcpy(windowless_pattern);

  /* use fobs mask on windowless_pattern */
  dephase(windowless_pattern);
  for(i = 0;i<sp_cmatrix_size(windowless_pattern->image);i++){
    windowless_pattern->mask->data[i] = fobs->mask->data[i];
    if(!fobs->mask->data[i]){
      windowless_pattern->image->data[i] = 0;
    }
  }
  write_png(windowless_pattern,"windowless_pattern.png",COLOR_JET);
  write_img(windowless_pattern,"windowless_pattern.h5",sizeof(real));
  unshifted_windowless_pattern = shift_quadrants(windowless_pattern);
  write_img(unshifted_windowless_pattern,"windowless_pattern2.h5",sizeof(real));
  write_png(unshifted_windowless_pattern,"windowless_pattern2.png",COLOR_JET);
  phases = sp_image_get_phases(sol);
  write_png(phases,"sol_phases.png",COLOR_JET);
  write_img(phases,"sol_phases.h5",sizeof(real));



  /**/
  tmp = image_rev_fft(windowless_real);
  make_real(tmp,IN_PLACE);
  freeimg(windowless_real);
  windowless_real = image_fft(tmp);
  for(i = 0;i<sp_cmatrix_size(windowless_real->image);i++){
    windowless_real->image->data[i] /= sp_cmatrix_size(windowless_real->image);
  }

  /* use fobs mask on windowless_real */
  dephase(windowless_real);
  for(i = 0;i<sp_cmatrix_size(windowless_real->image);i++){
    windowless_real->mask->data[i] = fobs->mask->data[i];
    if(!fobs->mask->data[i]){
      windowless_real->image->data[i] = 0;
    }
  }

  write_png(windowless_real,"windowless_real.png",COLOR_JET|LOG_SCALE);
  write_img(windowless_real,"windowless_real.h5",sizeof(real));
  average_centrosymetry(windowless_real);
  image_ratio = imgcpy(windowless_real);
  for(i = 0;i<sp_cmatrix_size(windowless_real->image);i++){
    if(windowless_real->mask->data[i] && windowless_real->image->data[i]){
      image_ratio->image->data[i] = windowless_pattern->image->data[i] / windowless_real->image->data[i];
      
    }
  }
  write_png(image_ratio,"image_ratio.png",COLOR_JET);
  write_img(image_ratio,"image_ratio.h5",sizeof(real));


  return 0;
}
