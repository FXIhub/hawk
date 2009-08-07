#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "spimage.h"

 
/* Reads a white space separated list of floating point values and stores them in a matrix of predetermined size */

Image * read_3matrix(char * filename, int x, int y, int z){
  FILE * f = fopen(filename,"r");
  unsigned int bufsiz = 100000;
  char * buffer = malloc(sizeof(char)*bufsiz);
  char *nptr,*endptr;
  long long i = 0;
  Image * res = sp_image_alloc(x,y,z);
  res->phased = 0;
  res->scaled = 0;
  for(i =0 ;i<x*y*z;i++){
    res->mask->data[i] = 1;
  }
  res->shifted = 0;
  res->detector->image_center[0] = 0;
  res->detector->image_center[1] = 0;
  res->detector->image_center[2] = 0;
  res->detector->wavelength = -1;
  res->detector->pixel_size[0] = -1;
  res->detector->pixel_size[1] = -1;
  res->detector->pixel_size[2] = -1;
  res->detector->detector_distance = -1;
  if(!f){
    perror("Error opening matrix");
    return NULL;
  }
  i = 0;

  while(fgets(buffer,bufsiz,f)){
    if(strlen(buffer) == bufsiz){
      printf("Please increase bufsiz or split your lines a bit!\n");
      exit(1);
    }
    nptr = buffer;
    endptr = NULL;
    while(1){
      res->image->data[i++] = sp_cinit(strtod(nptr,&endptr),0);
      if(nptr == endptr){
	i--;
	break;
      }else{
	nptr = endptr;
      }
    }
  }
  fclose(f);
  if(i != x*y){
    printf("Error dimension don't match\n");
    printf("Expected %d numbers but read, %lld\n",x*y*z,i);
  }else{
    printf("Read %lld values out of %d expected\n",i,x*y*z);
  }
  return res;
}

int main(int argc, char ** argv){
  Image * res;
  if(argc != 6){
    printf("Usage: %s <matrix_file> <nx> <ny> <nz> <image.h5>\n",argv[0]);
    exit(0);
  }
  res = read_3matrix(argv[1],atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
  sp_image_write(res,argv[5],sizeof(real));  
  return 0;
}
