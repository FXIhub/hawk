#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
#include "spimage.h"


int main(int argc, char ** argv){
  int i,n;
  if(argc =! 3){
    printf("Usage: %s [input] [output]\n",argv[0]);
    exit(0);
  }
  Image * a = sp_image_read(argv[1],0);
  for(int i = 0;i<sp_image_size(a);i++){
    a->image->data[i] = sp_cinit(sp_cabs2(a->image->data[i]),0);
  }
  sp_image_write(a,argv[2],0);
  return 0;
}
