#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <hdf5.h>
#include "spimage.h"

#define LINEAR 1


/* The idea is to stretch a set of pixels so that after stretching
 the max_angle is equal to the parameter */
/* I'm gonna use tan alpha/ sin alpha as stretching factor */
/* Using tan alpha / (sin (alpha /2) *2 ) is another option */ 
double * stretch_pixels(Image * img,double max_angle){
  double * new_pixels;
  double  max_dist = dist_to_center(0,img);
  double angle;
  double max_ini_angle = max_angle*cos(max_angle);
  double r = max_dist/sin(max_ini_angle);
  double factor;
  double max_factor = tan(max_angle)/sin(max_angle);
  int i;
  int x,y;
  new_pixels = malloc(sizeof(double)*2*TSIZE(img));
  for(i = 0;i<TSIZE(img);i++){
    angle = asin(dist_to_center(i,img)/r);

    /* scale intensities due to the differences in solid angle */
    img->amplitudes[i] *= 1/cos(angle)*cos(angle)*cos(angle);

    if(angle){
      factor = (tan(angle)/sin(angle))/max_factor;
    }else{
      factor = 1;
    }
    x = i/img->detector->size[1];
    y = i%img->detector->size[1];
    
    new_pixels[i*2] = (x-img->detector->image_center[0])*factor+img->detector->image_center[0];
    new_pixels[i*2+1] = (y-img->detector->image_center[1])*factor+img->detector->image_center[1];
  }
  return new_pixels;
}

Image * regrid_image(Image * img,double * new_pixels, int inter){
  Image * out = imgcpy(img);
  int i;
  double t,u;
  for(i = 0;i<TSIZE(out);i++){
    out->amplitudes[i] = 0;
  }
  if(inter == LINEAR){
    for(i = 0;i<TSIZE(img);i++){
	/* Do linear interpolation and skip */
	int px0 = (int)new_pixels[i*2];
	int py0 = (int)new_pixels[i*2+1];
	if(px0 < 0 || py0 < 0 || px0+1 >= img->detector->size[0] || py0+1 >= img->detector->size[1]){
	  out->amplitudes[i] = 0;
	  out->mask[i] = 0;
	  continue;
	}
	t = (new_pixels[i*2]-px0);
	u = (new_pixels[i*2+1]-py0);
	out->amplitudes[i] =  (1.0-t) * (1.0-u) * img->amplitudes[px0*img->detector->size[1]+py0]+
	  t * (1.0-u) * img->amplitudes[(px0+1)*img->detector->size[1]+py0]+
	  t * u * img->amplitudes[(px0+1)*img->detector->size[1]+py0+1]+
	  (1.0-t)* u * img->amplitudes[(px0)*img->detector->size[1]+py0+1];

    }    
  }
  return out;
}

int main(int argc, char ** argv){
  double max_angle = 0;
  Image * out;
  double * new_pixels;
  Image * img;
  if(argc < 4){
    printf("Stretches and scales appropriately an image\n");
    printf("Usage: stretch <image> <angle at top left> <out_image> [x center] [y center]");
    exit(0);
  }
  max_angle = atof(argv[2]);
  img = read_tiff(argv[1]);
  if(argc == 6){
    img->detector->image_center[0] = atof(argv[4]);
    img->detector->image_center[1] = atof(argv[5]);
  }
  new_pixels = stretch_pixels(img,max_angle);
  out = regrid_image(img,new_pixels,LINEAR);
  write_tiff(out, argv[3]);
  return 0;
}
