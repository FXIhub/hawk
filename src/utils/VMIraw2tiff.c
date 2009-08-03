#include <stdlib.h>
#include <hdf5.h>
#include <errno.h>
#include "spimage.h"


typedef struct{
  int numimages,exptime,gain,threshold,countingmode,bgsub,counts,framerate,missed,timestamp,streamexists ;
}ExtraData;

typedef struct{
  int col1,bunchid,col3,col4,hour,minute,second;
  float col6,ion_tunnel,e_tunnel,ion_hall,e_hall;
}GMDData;


GMDData * get_values_from_GMD(FILE * GMD,int bunchid){
  if(!GMD){
    return 0;
  }
  printf("Searching for intensity on GMD...\n");
  fseek(GMD,0,SEEK_SET);
  char line[1024];
  char sid[1024];
  sprintf(sid," %d",bunchid);
  while(fgets(line,1024,GMD)){
    if(strstr(line,sid)){
      GMDData * res = malloc(sizeof(GMDData));

      /* bunch id found on this line.
	 Lets parse it */
      printf("Found line: %s\n",line);
      sscanf(line,"%d %d %d %d %d:%d:%d %f %f %f %f %f",&res->col1,&res->bunchid,&res->col3,&res->col4,&res->hour,&res->minute,&res->second,&res->col6,&res->ion_tunnel,&res->e_tunnel,&res->ion_hall,&res->e_hall);
      return res;         
    }
  }
  return 0;
}

/* This function transforms filename.ext into filename-x.ext */
void append_tag(char * output,char * basename, char * x){
  char * p = strstr(basename,".");
  if(!p){
    sprintf(output,"%s-%s",basename,x);
    return;
  }
  *p = 0;
  sprintf(output,"%s-%s.%s",basename,x,p+1);
  *p = '.';  
}

int main(int argc, char ** argv){
  Image * image;
  if(argc != 3 && argc != 4){
    printf("VMIraw2tiff <file.raw> <file.tiff> [GMD.txt]\n");
    exit(0);
  }
  FILE * in = fopen(argv[1],"r");
  FILE * GMD = 0;
  if(!in){
    perror("Could not open input file");
    exit(1);
  }
  if(argc >= 4){
    GMD = fopen(argv[3],"r");
    if(!GMD){
      perror("Could not GMD file");
    }
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
      //      append_tag(buffer,argv[2],bunch_id);
      GMDData * gd = get_values_from_GMD(GMD,bunch_id);
      char buffer2[1024];
      char buffer3[1024];
      sprintf(buffer2,"%d",bunch_id);
      append_tag(buffer3,argv[2],buffer2);   
      if(!gd){
	append_tag(buffer,buffer3,"xxxx");
      }else{
	sprintf(buffer2,"%f",gd->e_hall);
	append_tag(buffer,buffer3,buffer2);
	free(gd);
      }
      sp_image_write(image,buffer,SpColormapJet);
      sp_image_free(image);
      //      exit(0);
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
    ExtraData ed;
    fread(&ed,sizeof(ExtraData),1,in);
    printf("Timestamp - %d\n",ed.timestamp);
    GMDData * gd = get_values_from_GMD(GMD,ed.timestamp);
    char buffer[1024];
    char buffer2[1024];
    char buffer3[1024];
    sprintf(buffer2,"%d",ed.timestamp);
    append_tag(buffer3,argv[2],buffer2);   
    if(!gd){
      append_tag(buffer,buffer3,"xxxx");
    }else{
      sprintf(buffer2,"%f",gd->e_hall);
      append_tag(buffer,buffer3,buffer2);
      free(gd);
    }
    sp_image_write(image,buffer,SpColormapJet);
    
  }
  return 0;
}
