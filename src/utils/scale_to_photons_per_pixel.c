#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"

/* This program tries to convert an image from arbitrary to photons per pixel scale by assuming that
   all the noise on a small region of the image is caused by poisson noise and that the signal in the region is constant.
   It then uses the fact that the standard deviation of the poisson is proportional to the number of photons in the signal 
*/

int main(int argc, char ** argv){
  Image * a;
  Image * mask;
  if(argc < 3){
    printf("Usage: scale_to_photons_per_pixel <image> <noise mask> [output]\n");
    printf(" The noise mask should be one for a small region of the image where there is a constant background.\n");
    
    return 0;
  }
  a = sp_image_read(argv[1],0);
  mask = sp_image_read(argv[2],0);
  real avg = 0;
  int mask_area = 0;
  for(int i = 0 ;i<sp_image_size(a);i++){
    if(sp_real(mask->image->data[i])){
      mask_area++;
      avg += sp_real(a->image->data[i]);
    }
  }
  avg /= mask_area;
  printf("Masked region size - %d\n",mask_area);
  printf("Mean signal in the region - %f\n",avg);
  real variance = 0;
  for(int i = 0 ;i<sp_image_size(a);i++){
    if(sp_real(mask->image->data[i])){
      variance += (avg - sp_real(a->image->data[i]))*(avg - sp_real(a->image->data[i]));
    }
  }
  variance /= mask_area;
  printf("Signal standard deviation in the region - %f\n",sqrt(variance));
  printf("Photons ver value - %f\n",avg/variance);
  return 0;
}
