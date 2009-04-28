#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * a, * out;
  if(argc < 3){
    printf("Usage: image_fft <input image> <output image>\n");
    return 0;
  }
  a = sp_image_read(argv[1],0);
  if(a->shifted){
    a = sp_image_shift(a);
  }
  out = sp_image_fft(a);

  int i;
  const i_max = sp_image_size(out);
  for (i = 0; i < i_max; i++) {
    out->mask->data[i] = 1;
  }

  sp_image_write(out,argv[2],0);
  return 0;
}
