#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"


real gaussian(real distance,real radius){
  return 1/(2*M_PI*radius*radius)*exp(-distance*distance/(2*radius*radius));
}

Image * localized_blur(Image * in, Image * mask, double blur){
  Image * out = sp_image_duplicate(in,SP_COPY_MASK|SP_COPY_DATA|SP_COPY_DETECTOR);
  real kernel_cutoff = blur*5;
  for(int i = 0;i<sp_image_size(in);i++){
    real px,py,pz;
    if(sp_real(mask->image->data[i]) != 0){
      continue;
    }
    sp_image_get_coords_from_index(in,i,&px,&py,&pz,TopLeftCorner);
    double gaussian_sum = 0;
    double convolution = 0;
    for(int x = floor(px-kernel_cutoff);x<ceil(px+kernel_cutoff);x++){
      if(x < 0 || x >= sp_image_x(in)){
	continue;
      }
      for(int y = floor(py-kernel_cutoff);y<ceil(px+kernel_cutoff);y++){
	if(y < 0 || y >= sp_image_y(in)){
	  continue;
	}
	for(int z = floor(pz-kernel_cutoff);z<ceil(pz+kernel_cutoff);z++){
	  if(z < 0 || z >= sp_image_z(in)){
	    continue;
	  }
	  if(sp_real(sp_image_get(mask,x,y,z)) != 0){
	    double distance = (x-px)*(x-px)+(y-py)*(y-py)+(z-pz)*(z-pz);
	    distance = sqrt(distance);
	    double gauss = gaussian(distance,blur);
	    gaussian_sum += gauss;
	    /* it's a good point, we're gonna use it */
	    convolution += sp_real(sp_image_get(in,x,y,z))*gauss;
	  }
	}
      }
    }
    out->image->data[i] = sp_cinit(convolution/*/gaussian_sum*/,0);
  }
  return out;
}


Image * localized_blur_fourier(Image * in, Image * mask, double blur,int iter){
  Image * out = sp_image_duplicate(in,SP_COPY_MASK|SP_COPY_DATA|SP_COPY_DETECTOR);
  for(int i = 0;i<iter;i++){
    Image * tmp = gaussian_blur(out,blur);
    for(int j = 0;j<sp_image_size(in);j++){
      if(sp_real(mask->image->data[j]) == 0){
	out->image->data[j] = tmp->image->data[j];
      }
    }
    sp_image_free(tmp);
  }
  return out;
}

int main(int argc, char ** argv){
  Image * img;
  Image * mask;
  double blur_radius = 0;
  if(argc != 5){
    printf("Usage: localized_blur <image.h5> <mask.h5> <blur radius> <output.h5>\n");
    return 0;
  }
  img = sp_image_read(argv[1],0);
  mask = sp_image_read(argv[2],0);
  blur_radius = atof(argv[3]);
  //  Image * out = localized_blur(img,mask,blur_radius);
  Image * out = localized_blur_fourier(img,mask,blur_radius,10);
  sp_image_write(out,argv[4],0);
  return 0;
}
  
