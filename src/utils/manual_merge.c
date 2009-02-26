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
  if(argc != 2){
    fprintf(stderr,"At least 2 arguments required!\n");
    return 1;
  }
  fd = fopen(argv[1],"r");
  while(fgets(line,1024,fd)){
    n = sscanf(line,"%lf %lf %s %s",&time,&saturation,img,back);
    tmp = sp_image_read(img,0);
    if(res == NULL){
      res = sp_image_duplicate(tmp,0);
      den = sp_image_duplicate(tmp,0);
    }
    

    if(n == 4){
      back_tmp = sp_image_read(back,0);
      sp_image_sub(tmp,back_tmp);
      sp_image_free(back_tmp);
      /* deal with saturation */
      for(i = 0;i<sp_image_size(tmp);i++){
	if(sp_cabs(tmp->image->data[i])+ sp_cabs(back_tmp->image->data[i]) < saturation){
	  sp_real(res->image->data[i]) += sp_real(tmp->image->data[i])/time;
	  sp_imag(res->image->data[i]) += sp_imag(tmp->image->data[i])/time;
	  sp_real(den->image->data[i])++;
	}
      }
    }else{
      for(i = 0;i<sp_image_size(tmp);i++){
	if(sp_cabs(tmp->image->data[i])  < saturation){
	  sp_real(res->image->data[i]) += sp_real(tmp->image->data[i])/time;
	  sp_imag(res->image->data[i]) += sp_imag(tmp->image->data[i])/time;
	  sp_real(den->image->data[i])++;
	}
      }
    }
    sp_image_free(tmp);
  }
  for(i = 0;i<sp_image_size(res);i++){
    if(sp_real(den->image->data[i])){
      sp_real(res->image->data[i]) /= sp_real(den->image->data[i]);
      sp_imag(res->image->data[i]) /= sp_real(den->image->data[i]);
    }else{
      res->mask->data[i] = 0;
      /* put some unreasonable number on it */
      sp_real(res->image->data[i]) = saturation*2;
      sp_imag(res->image->data[i]) = 0;
    }
  }
  sp_image_write(res,argv[2],sizeof(real));
  sp_image_free(res);
  return 0;
}
