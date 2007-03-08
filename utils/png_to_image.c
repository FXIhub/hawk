#include <stdlib.h>
#include <hdf5.h>
#include "spimage.h"

int main(int argc, char ** argv){
  Image * image;
  if(argc != 3 && argc != 4 ){
    printf("png_to_image <file.png> <file.h5> [-u]\n");
    printf("The optional -u means unscalled data.\n");
    exit(0);
  }

  image = sp_image_read(argv[1],0);
  sp_image_write(image,argv[2],sizeof(real));
  return 0;
}
