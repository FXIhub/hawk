#include <spimage.h>
#include "xcam.h"

void find_image_center(Image * a){
  int x,y,z;
  long long index;
  sp_image_max(a,&index,&x,&y,&z);
  a->detector->image_center[0] = x;
  a->detector->image_center[1] = y;
  a->detector->image_center[2] = z;
}

double count_signal(Image * a,double mean_center[2]){
  double signal = 0;
  double radius = 100;
    for(int x = sp_max(0,mean_center[0]-radius); x<sp_min(sp_image_x(a)-1,mean_center[0]+radius);x++){
      for(int y = sp_max(0,mean_center[1]-radius); y<sp_min(sp_image_y(a)-1,mean_center[1]+radius);y++){
  //  for(int x = 0; x<sp_image_x(a)/4;x++){
  //    for(int y = sp_image_y(a)/4;y<sp_image_y(a)*3/4;y++){
      signal += sp_real(sp_image_get(a,x,y,0));
    }
  }
  return signal;
}

void remove_background(Image * a){
  /* average the background on the right side of the image */
  int n = 0;
  double bg = 0;
  for(int x = sp_image_x(a)/2;x<sp_image_x(a);x++){
    for(int y = 0;y<sp_image_y(a);y++){
      bg += sp_real(sp_image_get(a,x,y,0));
      n++;
    }
  }
  bg /= n;
  for(int i = 0;i<sp_image_size(a);i++){
    sp_real(a->image->data[i]) -= bg;
  }
}

int main(int argc, char ** argv){
  /* We're gonna use certain statistics to figure out if there's a hit or not.
     Those measures are:
     - total intensity
     - slope of the radially averaged intensities
     - standard deviation of the intensities on the region with negative slope 
   */
  double mean_center[2] = {0,0};
  double mean_total_intensity = 0;
  double total_intensity2 = 0;
  double * image_I = malloc(sizeof(double)*argc);
  int n = 0;
  for(int i = 1;i<argc;i++){
    n++;
    Image * a = sp_image_read(argv[i],0);
    Image * tmp = xcam_preprocess(a);
    sp_image_free(a);
    a = tmp;
    find_image_center(a);
    mean_center[0] += a->detector->image_center[0];
    mean_center[1] += a->detector->image_center[1];
    sp_image_free(a);
    printf("%d out of %d\n",i,argc);
  }
  mean_center[0] /= n;
  mean_center[1] /= n;  
  printf("Mean center = %f  %f\n",mean_center[0],mean_center[1]);
  for(int i = 1;i<argc;i++){
    Image * a = sp_image_read(argv[i],0);
    Image * tmp = xcam_preprocess(a);
    sp_image_free(a);
    a = tmp;
    remove_background(a);

    double v = count_signal(a,mean_center);
    image_I[i] = v;
    double delta = v-mean_total_intensity;
    mean_total_intensity += delta/n;
    total_intensity2 += delta*(v-mean_total_intensity);
    sp_image_free(a);
    printf("%d out of %d\n",i,argc);
  }
  mean_center[0] /= n;
  mean_center[1] /= n;
  total_intensity2 /= n;
  printf("I_Mean = %e I_Variance = %e\n",mean_total_intensity,total_intensity2);
  for(int i = 1;i<argc-1;i++){
    if(image_I[i] > mean_total_intensity + sqrt(total_intensity2)/3){
      printf("%s I=%e\n",argv[i],image_I[i]);
    }
  }
}
