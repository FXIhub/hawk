#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * a;
  int i;
  if(argc < 3){
    printf("Usage: image_to_png <image.h5> <image.png> [phase.png] [mask.png]\n");
    return 0;
  }
  a = read_imagefile(argv[1]);
  if(a->shifted){
    a = sp_image_shift(a);
  }
  sp_image_write(a,argv[2],COLOR_JET|LOG_SCALE);
  if(argc > 3){
    sp_image_write(a,argv[3],COLOR_JET|LOG_SCALE|COLOR_PHASE);
  }
  if(argc > 4){
    for(i = 0;i<sp_cmatrix_size(a->image);i++){
      a->image->data[i] = a->mask->data[i];
    }
    sp_image_write(a,argv[4],COLOR_JET|LOG_SCALE);
  }
  return 0;
}
