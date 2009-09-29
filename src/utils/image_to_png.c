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
  a = sp_image_read(argv[1],0);
  if(a->shifted){
    a = sp_image_shift(a);
  }
    sp_image_write(a,argv[2],SpColormapJet|SpColormapLogScale);
    //  sp_image_write(a,argv[2],SpColormapJet);
  if(argc > 3){
    sp_image_write(a,argv[3],SpColormapJet|SpColormapLogScale|SpColormapPhase);
  }
  if(argc > 4){
    for(i = 0;i<sp_c3matrix_size(a->image);i++){
      a->image->data[i] = sp_cinit(a->mask->data[i],0);
    }
    sp_image_write(a,argv[4],SpColormapJet|SpColormapLogScale);
  }
  return 0;
}
