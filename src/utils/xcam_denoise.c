#include <spimage.h>
#include "xcam.h"



int main(int argc, char ** argv){
  if(argc < 3){
    printf("Usage: %s <input image> <output image>\n",argv[0]);
    exit(1);
  }
  Image * input = sp_image_read(argv[1],0);
  Image * output = xcam_preprocess(input);
  sp_image_write(output,argv[2],0);
  return 0;
}
