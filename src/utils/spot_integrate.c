#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * a;
  int i,j;
  double sum;
  FILE * fp;
  if(argc < 3){
    printf("Usage: image_to_png <output.data> <image1.h5> [image2.h5] [image3.h5]... \n");
    return 0;
  }
  fp = fopen(argv[1],"w");
  for(i = 2;i<argc;i++){
    a = sp_image_read(argv[i],0);
    sum = 0;
    for(j = 0;j<sp_image_size(a);j++){
      sum += sp_cabs(a->image->data[j]);
    }
    fprintf(fp,"%d %f\n",i-2,sum);
  }
  return 0;
}
