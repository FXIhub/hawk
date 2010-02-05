#include <spimage.h>

int main(int argc, char ** argv){
  if(argc < 2){
    printf("Usage: pnccd_split <pnccd_image.h5>\n");
    return 0;
  }
  Image * a = sp_image_read(argv[1],0);
  if(!a){
    sp_error_fatal("Error reading %s",argv[1]);
  }
  if(sp_image_x(a) % 1024 || sp_image_y(a) != 1024){
    sp_error_fatal("Expecting a nframes*1024x1024 input image");
  }
  int nframes = sp_image_x(a) / 1024;
  Image * out[2][2];
  for(int frame = 0;frame<nframes;frame++){
    for(int position = 0;position<2;position++){
      out[frame][position] = sp_image_alloc(1024,512,1);
      for(int x = 0;x<1024;x++){
	for(int y = 0;y<512;y++){
	  sp_image_set(out[frame][position],x,y,0,sp_image_get(a,x+frame*1024,y+position*512,0));
	}
      }
    }
  }
  char buffer[1024];
  char basename[1024];
  strcpy(basename,argv[1]);
  for(int i = strlen(basename)-1;i;i--){
    char c = basename[i];
    basename[i] = 0;
    if(c == '.'){
      break;
    }
  }
  if(nframes == 1){
    sprintf(buffer,"%s_back_top.h5",basename);
    sp_image_write(out[0][0],buffer,0);
    sprintf(buffer,"%s_back_bottom.h5",basename);
    sp_image_write(out[0][1],buffer,0);
  }else if(nframes == 2){
    sprintf(buffer,"%s_front_top.h5",basename);
    sp_image_write(out[0][0],buffer,0);
    sprintf(buffer,"%s_front_bottom.h5",basename);
    sp_image_write(out[0][1],buffer,0);
    sprintf(buffer,"%s_back_top.h5",basename);
    sp_image_write(out[1][0],buffer,0);
    sprintf(buffer,"%s_back_bottom.h5",basename);
    sp_image_write(out[1][1],buffer,0);
  }
  return 0;
}
