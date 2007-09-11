#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hdf5.h>
#include <mpi.h>

#include "spimage.h"



int main(int argc, char ** argv)
{

  int x,y,z;
  long long i;

  Image * density = sp_image_alloc(100,100,100);
  printf("density allocated\n");
  printf("set\n");
  for(x=0; x<sp_image_x(density); x++){
    for(y=0; y<sp_image_y(density); y++){
      for(z=0; z<sp_image_z(density); z++){
	if((x-45)*(x-45)+(y-47)*(y-47)+(z-40)*(z-40) <= 5){
	  sp_image_set(density,x,y,z,5);
	}
	if((x-55)*(x-55)+(y-47)*(y-47)+(z-49)*(z-49) <= 9){
	  sp_image_set(density,x,y,z,4);
	}
	if((x-50)*(x-50)+(y-57)*(y-57)+(z-52)*(z-52) <= 12){
	  sp_image_set(density,x,y,z,7);
	}
	if((x-50)*(x-50)+(y-50)*(y-50)+(z-58)*(z-58) <= 8){
	  sp_image_set(density,x,y,z,3);
	}
      }
    }
  }

  Image * img = sp_image_fft(density);

  for(i=0; i<sp_image_size(img); i++){
    img->mask->data[i] = 1;
  }

  //Image * img = sp_image_generate_pattern(100);
  Image * tmp = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);

  sp_image_conj(tmp);
  sp_image_mul_elements(img,tmp);
  img->phased = 0;
  img->shifted = 0;

  Image * img_s = sp_image_shift(img);
  sp_image_write(img_s,"spheres.h5",0);

  sp_image_free(tmp);

  tmp = sp_image_read("spheres.h5",0);

  return 0;

}
