#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "spimage.h"


void write_array_file(char  * filename, int n,real * y, real * x){
  FILE  * fp = fopen(filename,"w");
  for(int i = 0;i<n;i++){
    if(x){
      fprintf(fp,"%f\t%f\n",x[i],y[i]);
    }else if(y){
      fprintf(fp,"%d\t%f\n",i,y[i]);
    }
  }
  fclose(fp);
}

/*  The coherently flag tells the program to sum the image pixels coherently or not */
real * bin_image_by_r(Image * a, int nshells){
 real max_dist = sqrt(sp_image_x(a)*sp_image_x(a)+sp_image_y(a)*sp_image_y(a)+
		       sp_image_z(a)*sp_image_z(a));
 max_dist /= 2;
 real * bins = sp_malloc(sizeof(real)*nshells);
 int * nbin = sp_malloc(sizeof(int)*nshells);
 for(int i = 0;i<nshells;i++){
   bins[i] = 0;
   nbin[i] = 0;
 }
 for(int i = 0;i<sp_image_size(a);i++){
   real dist = sp_image_dist(a,i,SP_TO_CENTER);
   int bin = sp_min(dist*nshells/max_dist,nshells-1);
   bins[bin] += sp_cabs(a->image->data[i])*sp_cabs(a->image->data[i]);
   nbin[bin]++;
 }
 for(int i = 0;i<nshells;i++){
   bins[i] /= nbin[i];
 }
 return bins;
}

int main(int argc, char ** argv){
  int nshells = 141;
  real * shell_res;
  if(argc != 3){
    printf("Usage: shell_plot [image] [max res]\n");
    exit(0);
  }
  real max_res = atof(argv[2]);

  if(max_res){
    shell_res = malloc(sizeof(real)*nshells);
    for(int i = 0;i<nshells;i++){
      shell_res[i] = 1.0/((1.0/max_res)*((i+0.5)/nshells));
    }
  }
  Image * a= sp_image_read(argv[1],0);
  write_array_file("shells.data",nshells,bin_image_by_r(a,nshells),shell_res);
  return 0;
}
