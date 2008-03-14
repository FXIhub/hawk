#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "spimage.h"

typedef struct{
  double * f;
  int n_bins;
  int * bin_pop;
}Binned_Data;


void write_array_file(char  * filename, int n,Binned_Data * y, real * x){
  FILE  * fp = fopen(filename,"w");
  for(int i = 0;i<n;i++){
    if(x){
      fprintf(fp,"%f\t%e\t%d\n",x[i],y->f[i],y->bin_pop[i]);
    }else if(y){
      fprintf(fp,"%d\t%e\t%d\n",i,y->f[i],y->bin_pop[i]);
    }
  }
  fclose(fp);
}

/*  The coherently flag tells the program to sum the image pixels coherently or not */
Binned_Data * bin_image_by_r(Image * a, int nshells){
  Binned_Data * res = sp_malloc(sizeof(Binned_Data));
  res->f = sp_malloc(sizeof(double)*nshells);
  res->bin_pop = sp_malloc(sizeof(int)*nshells);
  real max_dist = sqrt(sp_image_x(a)*sp_image_x(a)+sp_image_y(a)*sp_image_y(a)+
		       sp_image_z(a)*sp_image_z(a));
 max_dist /= 2;
 for(int i = 0;i<nshells;i++){
   res->f[i] = 0;
   res->bin_pop[i] = 0;
 }
 for(int i = 0;i<sp_image_size(a);i++){
   real dist = sp_image_dist(a,i,SP_TO_CENTER);
   int bin = sp_min(dist*nshells/max_dist,nshells-1);
   res->f[bin] += sp_cabs(a->image->data[i]);
   res->bin_pop[bin]++;
 }
 for(int i = 0;i<nshells;i++){
   if(res->bin_pop <= 0){
     fprintf(stderr,"Zero bin - %d!\n",i);
   }
   res->f[i] /= res->bin_pop[i];
 }
 return res;
}

int main(int argc, char ** argv){
  int nshells = 141;
  Binned_Data * out;
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
  out = bin_image_by_r(a,nshells);
  write_array_file("shells.data",nshells,out,shell_res);
  return 0;
}
