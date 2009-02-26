/* Program to analyse and compare two images 
   For the moment it only does a simple division between the
   2 images


   A typical invocation is:
   "analyse_image <img1.h5> <img2.h5> <saturation>"
   
   Output is written to analyse_image.png and analyse_image.vtk

*/
#include <stdio.h>
#include <stdlib.h>
#include "spimage.h"


void write_hist(int * a, int size, char * filename){
  FILE * f;
  int i;
  f = fopen(filename,"w");
  for(i = 0;i<size;i++){
    fprintf(f,"%d %d\n",i,a[i]);
  }
  fclose(f);
}
/* creates intensity histogram for 8 bits or 16 bits images */
int * intensity_histogram(Image * a, int bits){
  int * res = calloc((1<<bits),sizeof(int));
  int i;
  for(i = 0;i<sp_cmatrix_size(a->image);i++){
    res[((int)a->image->data[i])]++;
  }
  return res;
}


int main(int argc, char ** argv){
  int i,j;
  Image * a;
  Image * b;
  Image * res;
  Image * res_den;
  Image * p_res;
  real saturation = 250;
  if(argc < 4){
    printf("Usage: %s <saturation> <img1.h5> ... <imgN.h5>\n",argv[0]);
    return 0;
  }
  saturation = atof(argv[1]);
  for(j = 3;j<argc;j++){
    a = read_imagefile(argv[j-1]);  
    if(!res){
      res = create_empty_img(a);
      res_den = create_empty_img(a);
    }
    b = read_imagefile(argv[j]);
    p_res = create_empty_img(a);
    if(sp_cmatrix_size(a->image) != sp_cmatrix_size(b->image)){
      printf("Error: images should have the same dimensions!\n");
    return 1;
    }
    for(i = 0;i<sp_cmatrix_size(a->image);i++){
      if(b->image->data[i] && a->image->data[i] &&
	 creal(b->image->data[i]) < saturation && creal(a->image->data[i]) < saturation){
	p_res->image->data[i] = a->image->data[i]/b->image->data[i];
	res_den->image->data[i] += 1;
      }else{
	p_res->image->data[i] = 0;
      }
    }
    add_image(res,p_res);
    freeimg(p_res);
    freeimg(a);
    freeimg(b);
  }

  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    if(res_den->image->data[i]){
      res->image->data[i] /= res_den->image->data[i];
    }
  }
  write_vtk(res,"analyze_image.vtk");
  write_png(res,"analyze_image.png",COLOR_JET);
  return 0;
}

