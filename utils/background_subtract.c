#include <spimage.h>

char ** read_filelist(char * filename, int  * n){
  char buffer[1024];
  FILE * f = fopen(filename,"r");
  *n = 0;
  while(fgets(buffer,1024,f)){
    (*n)++;
  }
  fclose(f);
  char ** res = malloc(sizeof(char *)*(*n));
  f = fopen(filename,"r");
  for(int i = 0; i < *n;i++){
    fscanf(f,"%s",buffer);
    res[i] = malloc((strlen(buffer)+1)*sizeof(char));
    strcpy(res[i],buffer);
  }
  return res;
}

int main(int argc, char ** argv){
  Image * back;
  Image * tmp;
  char buffer[1024];
  int i;
  if(argc < 3){
    printf("background_subtract <background filelist> <image filelist>\n");
    exit(0);
  }
  int bg_list_n;
  char ** bg_list = read_filelist(argv[1],&bg_list_n);
  int img_list_n;
  char ** img_list = read_filelist(argv[2],&img_list_n);
  back = sp_image_read(bg_list[0],0);
  for(i = 1;i<bg_list_n;i++){
    tmp = sp_image_read(bg_list[i],0);
    sp_image_add(back,tmp);    
    sp_image_free(tmp);
  }
  sp_image_scale(back,1.0/bg_list_n);

  for(i = 0;i<img_list_n;i++){
    tmp = sp_image_read(img_list[i],0);
    strcpy(buffer,"bg_sub_");
    strcat(buffer,img_list[i]);    
    sp_image_sub(tmp,back);    
    sp_image_write(tmp,buffer,COLOR_JET|LOG_SCALE);
  }  
}
