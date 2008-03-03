/* This is a very specific program that extracts the red channel pixels from the raw
   data of a Canon EOS 5D image.   
*/ 

#include <stdio.h>
#include <stdlib.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * a;
  Image * b;
  char buffer[1024];
  int x,y;
  real max;
  if(argc != 4){
    printf("Usage: %s <img.h5> <R|G|B> <newimg.h5>\n",argv[0]);
    return 0;
  }
  if(argv[2][0] == 'B'){
    printf("Error: Blue channel not yet supported, sorry :-(\n");
    exit(1);
  }
  a = sp_image_read(argv[1],0);
  b = sp_image_alloc(sp_image_x(a)/2,sp_image_y(a)/2,1);
  if(argv[2][0] == 'R'){
    for(x = 0;x<sp_image_x(a);x++){
      if(x % 2){
	continue;
      }
      for(y = 0;y<sp_image_y(a);y++){
	if(y % 2){
	  continue;
	}
	sp_image_set(b,x/2,y/2,0,sp_image_get(a,x,y,0));
	sp_i3matrix_set(b->mask,x/2,y/2,0,sp_i3matrix_get(a->mask,x,y,0));
      }
    }
  }else if(argv[2][0] == 'G'){
    /* I'm not gonna use all the green detectors, just as many as for the red picture */
    for(x = 0;x<sp_image_x(a);x++){
      if(x % 2){
	continue;
      }
      for(y = 0;y<sp_image_y(a);y++){
	if(y % 2 == 0){
	  continue;
	}
	sp_image_set(b,x/2,y/2,0,sp_image_get(a,x,y,0));
	sp_i3matrix_set(b->mask,x/2,y/2,0,sp_i3matrix_get(a->mask,x,y,0));
      }
    }
    
  }
  sp_image_write(b,argv[3],sizeof(real));
  sprintf(buffer,"%s.vtk",argv[3]);
  sp_image_write(b,buffer,0);
  sprintf(buffer,"%s.png",argv[3]);
  sp_image_write(b,buffer,COLOR_JET|LOG_SCALE);
  /*write capped version */
  max = 0;
  for(int i = 0;i<sp_image_size(b);i++){
    if(sp_cabs(b->image->data[i]) > max){
      max = sp_cabs(b->image->data[i]);
    }    
  }
  /* Cap all values higher than 0.05 than the maximum for easier visualization */
  for(int i = 0;i<sp_image_size(b);i++){
    if(sp_cabs(b->image->data[i]) > max*0.05){
      sp_real(b->image->data[i]) = max*0.05;
      sp_imag(b->image->data[i]) = 0;
    }    
  }
  sprintf(buffer,"%s_capped.png",argv[3]);
  sp_image_write(b,buffer,COLOR_JET|LOG_SCALE);  
  return 0;
}
