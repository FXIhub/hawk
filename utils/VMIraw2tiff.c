#include <stdlib.h>
#include <hdf5.h>
#include <errno.h>
#include "spimage.h"


typedef struct{
  int numimages,exptime,gain,threshold,countingmode,bgsub,counts,framerate,missed,timestamp,streamexists ;
}ExtraData;

int main(int argc, char ** argv){
  Image * image;
  if(argc != 3 ){
    printf("VMIraw2tiff <file.raw> <file.tiff>\n");
    exit(0);
  }
  FILE * in = fopen(argv[1],"r");
  if(!in){
    perror("Could not open input file");
    exit(1);
  }
  int width,height;
  fread((void *)&width,sizeof(int),1,in);
  fread((void *)&height,sizeof(int),1,in);
  printf("Read an %dx%d image\n",width,height);
  if(strstr(argv[1],".sss")){
    /* We have the stack of images case */ 
    int num_images;
    fread((void *)&num_images,sizeof(int),1,in);
    for(int k = 0;k<num_images;k++){
      image = sp_image_alloc(width,height,1);
      int bunch_id;
      fread((void *)&bunch_id,sizeof(int),1,in);
      printf("Bunch Id - %d\n",bunch_id);
      int i =0;
      char * data  = malloc(sizeof(int)*width*height);
      fread(data,sizeof(char),width*height,in);
      char buffer[1024];
      for(int y = 0;y<height;y++){
	for(int x = 0;x<width;x++){
	  sp_image_set(image,x,y,0,sp_cinit(data[i],0));
	  i++;		   
	}
      }
      free(data);
      sprintf(buffer,"%s-%d.tif",argv[2],bunch_id);
      //      sp_image_write(image,buffer,COLOR_JET);
      sp_image_free(image);
    }
  }else{
    /* We have the single image case */ 
    int * data  = malloc(sizeof(int)*width*height);
    fread(data,sizeof(int),width*height,in);

    image = sp_image_alloc(width,height,1);
    int i =0;
    for(int y = 0;y<height;y++){
      for(int x = 0;x<width;x++){
	sp_image_set(image,x,y,0,sp_cinit(data[i],0));
	i++;		   
      }
    }
    sp_image_write(image,argv[2],COLOR_JET);
    ExtraData ed;
    fread(&ed,sizeof(ExtraData),1,in);
    printf("Timestamp - %d\n",ed.timestamp);
  }
  return 0;
}
