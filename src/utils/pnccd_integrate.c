#include <spimage.h>

int main(int argc, char ** argv){
  double integration[2] = {0,0};
  if(argc < 2){
    printf("Usage: pnccd_integrate <pnccd_image.h5>\n");
    return 0;
  }
  Image * a = sp_image_read(argv[1],0);
  if(!a){
    sp_error_fatal("Error reading %s",argv[1]);
  }
  if(sp_image_x(a) != 2048 || sp_image_y(a) != 1024){
    sp_error_fatal("Expecting a 2048x1024 input image");
  }
  for(int frame = 0;frame<2;frame++){
    for(int x = 0;x<1024;x++){
      for(int y = 0;y<1024;y++){
	integration[frame] += sp_real(sp_image_get(a,x+frame*1024,y,0));
      }
    }
  }
  printf("%g %g",integration[0],integration[1]);
}
