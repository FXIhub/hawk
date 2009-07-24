#include <spimage.h>

typedef struct{
  double * f;
  int n_bins;
  int * bin_pop;
}Binned_Data;

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
   if(res->bin_pop[i] <= 0){
     fprintf(stderr,"Zero bin - %d!\n",i);
   }else{
     res->f[i] /= res->bin_pop[i];
   }
 }
 return res;
}

/*  The coherently flag tells the program to sum the image pixels coherently or not */
Binned_Data * bin_min_image_by_r(Image * a, int nshells){
  Binned_Data * res = sp_malloc(sizeof(Binned_Data));
  res->f = sp_malloc(sizeof(double)*nshells);
  res->bin_pop = sp_malloc(sizeof(int)*nshells);
  real max_dist = sqrt(sp_image_x(a)*sp_image_x(a)+sp_image_y(a)*sp_image_y(a)+
		       sp_image_z(a)*sp_image_z(a));
 max_dist /= 2;
 for(int i = 0;i<nshells;i++){
   res->f[i] = 1e20;
   res->bin_pop[i] = 0;
 }
 for(int i = 0;i<sp_image_size(a);i++){
   real dist = sp_image_dist(a,i,SP_TO_CENTER);
   int bin = sp_min(dist*nshells/max_dist,nshells-1);
   if(sp_cabs(a->image->data[i]) < res->f[bin]){
     res->f[bin] = sp_cabs(a->image->data[i]);
     res->bin_pop[bin] = 1;
   }
 }
 for(int i = 0;i<nshells;i++){
   if(res->bin_pop[i] <= 0){
     fprintf(stderr,"Zero bin - %d!\n",i);
     res->f[i] = 0;
   }else{
     res->f[i] /= res->bin_pop[i];
   }
 }
 return res;
}

int main(int argc, char ** argv){
  if(argc != 4){
    printf("Multiplies a given input image by a gaussian\n");
    printf("Usage: %s <input> <std_dev> <output image>\n",argv[0]);
    exit(0);
  }
  int nshells = 80;  
  Image * a = sp_image_read(argv[1],0);
  real max_dist = sqrt(sp_image_x(a)*sp_image_x(a)+sp_image_y(a)*sp_image_y(a)+
		       sp_image_z(a)*sp_image_z(a))/2;

  Binned_Data * bd = bin_min_image_by_r(a,nshells);
  real std_dev = atof(argv[2]);
  for(int i =0 ;i<sp_image_size(a);i++){
    real dist = sp_image_dist(a,i,SP_TO_CENTER);
    real c = exp(-dist*dist/(2*std_dev*std_dev));
    int bin = sp_min(dist*nshells/max_dist,nshells-1);
    sp_real(a->image->data[i]) -= bd->f[bin];
  }
  sp_image_write(a,argv[3],0);
}
