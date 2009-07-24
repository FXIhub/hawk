#include <spimage.h>

int main(int argc, char ** argv){
  if(argc != 4){
    printf("Superimposes a given input image on the master image\n");
    printf("Usage: %s <master image> <input image> <output image>\n",argv[0]);
    exit(0);
  }
  Image * a = sp_image_read(argv[1],0);
  Image * b = sp_image_read(argv[2],0);
  sp_image_superimpose(a,b,SpEnantiomorph|SpCorrectPhaseShift);
  /* after superimposing shift doesn't have much meaning*/
  b->shifted = 0;
  sp_image_write(b,argv[3],0);
}
