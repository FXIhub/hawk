/* Program to manually scale and combine images 
   Takes as input a text file with the following format

   <relative exposure time> <saturation> <image file>  [background image file]
   
   Example:
   
   1   250 1.h5   1_back.h5
   2.54   250 2.h5   2_back.h5
   10.2   250 3.h5   3_back.h5

   And a file name where to write the output.

   A typical invocation is:
   "manual_merge exposure.txt merged.h5"

*/

#include <stdio.h>
#include "spimage.h"


int main(int argc, char ** argv){
  FILE * fd;
  char line[1024];
  char img[1024];
  char back[1024];
  double time;
  double saturation;
  int n,i;
  Image * tmp;
  Image * back_tmp;
  Image * res = NULL;
  Image * den = NULL;
  fd = fopen(argv[1],"r");
  while(fgets(line,1024,fd)){
    n = sscanf(line,"%lf %lf %s %s",&time,&saturation,img,back);
    tmp = read_imagefile(img);
    if(res == NULL){
      res = create_empty_img(tmp);
      den = create_empty_img(tmp);
    }


    if(back[0]){
      back_tmp = read_imagefile(back);
      sp_image_sub(tmp,back_tmp);
      freeimg(back_tmp);
      /* deal with saturation */
      for(i = 0;i<TSIZE(tmp);i++){
	if(tmp->image[i]+ back_tmp->image[i] < saturation){
	  res->image[i] += tmp->image[i]/time;
	  den->image[i]++;
	}
      }
    }else{
      for(i = 0;i<TSIZE(tmp);i++){
	if(tmp->image[i]  < saturation){
	  res->image[i] += tmp->image[i]/time;
	  den->image[i]++;
	}
      }
    }
    freeimg(tmp);
  }
  for(i = 0;i<TSIZE(res);i++){
    if(den->image[i]){
      res->image[i] /= den->image[i];
    }else{
      res->mask[i] = 0;
      res->image[i] = saturation*2;
    }
  }
  write_img(res,argv[2],sizeof(real));
  freeimg(res);
  return 0;
}
