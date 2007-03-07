/* This is a very specific program that extracts the red channel pixels from the raw
   data of a Canon EOS 300D image.   
*/ 

#include <stdio.h>
#include <stdlib.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * a;
  Image * b;
  char buffer[1024];
  int x,y,i;
  real max;
  if(argc != 4){
    printf("Usage: %s <img.h5> <R|G|B> <newimg.h5>\n",argv[0]);
    return 0;
  }
  if(argv[2][0] == 'B'){
    printf("Error: Blue channel not yet supported, sorry :-(\n");
    exit(1);
  }
  a = read_imagefile(argv[1]);
  b = create_empty_img(a);
  b->detector->size[0] /= 2;
  b->detector->size[1] /= 2;
  realloc(b->image,TSIZE(b)*sizeof(real));
  realloc(b->mask,TSIZE(b)*sizeof(real));
  i = 0;
  if(argv[2][0] == 'R'){
    for(x = 0;x<a->detector->size[0];x++){
      if(x % 2){
	continue;
      }
      for(y = 0;y<a->detector->size[1];y++){
	if(y % 2){
	  continue;
	}
	b->image[i] = a->image[x*a->detector->size[1]+y];
	b->mask[i] = a->mask[x*a->detector->size[1]+y];
	i++;
      }
    }
  }else if(argv[2][0] == 'G'){
    /* I'm not gonna use all the green detectors, just as many as for the red picture */
    for(x = 0;x<a->detector->size[0];x++){
      if(x % 2){
	continue;
      }
      for(y = 0;y<a->detector->size[1];y++){
	if(y % 2 == 0){
	  continue;
	}
	b->image[i] = a->image[x*a->detector->size[1]+y];
	b->mask[i] = a->mask[x*a->detector->size[1]+y];
	i++;
      }
    }
    
  }
  write_img(b,argv[3],sizeof(real));
  sprintf(buffer,"%s.vtk",argv[3]);
  write_vtk(b,buffer);
  sprintf(buffer,"%s.png",argv[3]);
  write_png(b,buffer,COLOR_JET|LOG_SCALE);
  /*write capped version */
  max = 0;
  for(i = 0;i<TSIZE(b);i++){
    if(b->image[i] > max){
      max = b->image[i];
    }    
  }
  /* Cap all values higher than 0.05 than the maximum for easier visualization */
  for(i = 0;i<TSIZE(b);i++){
    if(b->image[i] > max*0.05){
      b->image[i] = max*0.05;
    }    
  }
  sprintf(buffer,"%s_capped.png",argv[3]);
  write_png(b,buffer,COLOR_JET|LOG_SCALE);  
  return 0;
}
