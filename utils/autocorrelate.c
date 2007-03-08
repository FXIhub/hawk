/* Autocorrelate diffraction pattern. Usage:
   autocorrelate <img.h5>

   Output:
   img_autocorrelation.vtk and img_autocorrelation.png (log scale color jet)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spimage.h>

int main(int argc, char ** argv){
  Image * a;
  Image * b;
  char buffer[1024];
  char base[1024];
  int i;
  real max;
  real min;
  if(argc != 2){
    printf("Usage: %s <img.h5>\n"
	   "Output: img_autocorrelation.vtk and img_autocorrelation.png\n",argv[0]);
    exit(0);
  }
  a = sp_image_read(argv[1],0);
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    if(cabs(a->image->data[i]) > 55000){
      a->image->data[i] = 0;
    }
  }
  b = sp_image_cross_correlate(a,a,NULL);
  a = sp_image_shift(b);
  sprintf(base,"%s",argv[1]);
  base[strlen(base)-3] = 0;
  sprintf(buffer,"%s_autocorrelation.vtk",base);
  sp_image_write(a,buffer,0);
  sprintf(buffer,"%s_autocorrelation.png",base);
  sp_image_write(a,buffer,COLOR_JET|LOG_SCALE);
  max = 0;
  min = 1<< 20;
  for(i = 0;i<sp_cmatrix_size(b->image);i++){
    if(cabs(b->image->data[i]) > max){
      max = cabs(b->image->data[i]);
    }    
    if(cabs(b->image->data[i]) < min){
      min = cabs(b->image->data[i]);
    }    
  }
  /* Cap all values higher than 0.05 than the maximum for easier visualization */
  for(i = 0;i<sp_cmatrix_size(b->image);i++){
    if(cabs(b->image->data[i]) > max*0.15){
      b->image->data[i] = max*0.15;
    }    
  }
  sprintf(buffer,"%s_capped_autocorrelation.png",base);
  a = sp_image_shift(b);
  sp_image_write(a,buffer,COLOR_JET|LOG_SCALE);  
  return 0;
  
}
