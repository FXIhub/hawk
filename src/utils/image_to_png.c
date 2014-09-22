#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"

#ifdef HAWK_UNIFIED
int convert_main(int argc, char ** argv){
#else
int main(int argc, char ** argv){
#endif
  Image * a;
  int i;
  if(argc < 3){
#ifdef HAWK_UNIFIED
    printf("Usage: hawk convert <image.h5> <image.png> [phase.png] [mask.png]\n");
#else
    printf("Usage: image_to_png <image.h5> <image.png> [phase.png] [mask.png]\n");
#endif
    return 0;
  }
  a = sp_image_read(argv[1],0);
  if(a->shifted){
    a = sp_image_shift(a);
  }
  sp_image_write(a,argv[2],SpColormapJet);
  //sp_image_write(a,argv[2],0);
  if(argc > 3){
    sp_image_write(a,argv[3],SpColormapJet|SpColormapPhase);
  }
  if(argc > 4){
    for(i = 0;i<sp_c3matrix_size(a->image);i++){
      a->image->data[i] = sp_cinit(a->mask->data[i],0);
    }
    sp_image_write(a,argv[4],SpColormapGrayScale);
  }
  return 0;
}
