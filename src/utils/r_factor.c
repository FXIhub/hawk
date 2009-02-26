#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "spimage.h"


typedef struct{
  Image * rs_img;
  Image * fs_img;
  Image * rs_sol;
  Image * fs_sol;
  Image * sol;
  Image * img;
  int dephase_rs_img;
  char image_file[1024];
  char output_file[1024];
  real max_res;
}Options;

void set_defaults(Options * opt){
  opt->rs_img = 0;
  opt->fs_img = 0;
  opt->rs_sol = 0;
  opt->fs_sol = 0;
  opt->image_file[0] = 0;
  opt->dephase_rs_img = 0;
  opt->output_file[0] = 0;
  opt->max_res = 0;
}


Options * parse_options(int argc, char ** argv){
  static char help_text[] = 
"    Options description:\n\
    \n\
    -I: Real Space Image file\n\
    -i: Fourier Space Image file\n\
    -S: Real Space Solution file\n\
    -s: Fourier Space Solution file\n\
    -d: Dephase Real space Image\n\
    -o: Output File with the R factors\n\
";
  static struct option long_options[] = {
    {"rs_image", 1, 0, 'I'},
    {"fs_image", 1, 0, 'i'},
    {"rs_solution", 1, 0, 'S'},
    {"fs_solution", 1, 0, 's'},
    {"max_res", 1, 0, 'r'},
    {"dephase", 0, 0, 'd'},
    {"output", 1, 0, 'o'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };
  static char optstring[] = "I:i:S:s:hdo:r:";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);
  int c;
  while(1){
    c = getopt_long(argc,argv,optstring,long_options,NULL);
    if(c == -1){
      break;
    }
    switch(c){
    case 'd':
      res->dephase_rs_img = 1;
      break;
    case 'I':
      res->rs_img = sp_image_read(optarg,0);
      strcpy(res->image_file,optarg);
      break;
    case 'i':
      res->fs_img = sp_image_read(optarg,0);
      strcpy(res->image_file,optarg);
    case 'S':
      res->rs_sol = sp_image_read(optarg,0);
      break;
    case 's':
      res->fs_sol = sp_image_read(optarg,0);
      break;
    case 'o':
      strcpy(res->output_file,optarg);
      break;
    case 'r':
      res->max_res = atof(optarg);
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  return res;
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
   bins[bin] += sp_cabs(a->image->data[i]);
   nbin[bin]++;
 }
 for(int i = 0;i<nshells;i++){
   bins[i] /= nbin[i];
 }
 return bins;
}
 

real * res_dep_r_factor(Image * a, Image * b, int nshells){
  real * bin_dif = sp_malloc(sizeof(real)*nshells);
  real * bin_sum = sp_malloc(sizeof(real)*nshells);
  for(int i =0;i<nshells;i++){
    bin_dif[i] = 0;
    bin_sum[i] = 0;
  }
  real max_dist = sqrt(sp_image_x(a)*sp_image_x(a)+sp_image_y(a)*sp_image_y(a)+
		       sp_image_z(a)*sp_image_z(a));
  max_dist /= 2;
  if(a->phased && b->phased){
    for(int i = 0;i<sp_image_size(a);i++){
      real dist = sp_image_dist(a,i,SP_TO_CENTER);
      int bin = sp_min(dist*nshells/max_dist,nshells-1);
      bin_dif[bin] += fabs(sp_cabs(a->image->data[i])-sp_cabs(b->image->data[i]));
      bin_sum[bin] += fabs(sp_cabs(a->image->data[i])+sp_cabs(b->image->data[i]));      
/*
      bin_dif[bin] += sp_cabs(sp_csub(a->image->data[i],b->image->data[i]));
      bin_sum[bin] += sp_cabs(sp_cadd(a->image->data[i],b->image->data[i]));      */
    }
  }else{
    for(int i = 0;i<sp_image_size(a);i++){
      real dist = sp_image_dist(a,i,SP_TO_CENTER);
      int bin = sp_min(dist*nshells/max_dist,nshells-1);
      bin_dif[bin] += fabs(sp_cabs(a->image->data[i])-sp_cabs(b->image->data[i]));
      bin_sum[bin] += fabs(sp_cabs(a->image->data[i])+sp_cabs(b->image->data[i]));      
    }
  }
  for(int i =0;i<nshells;i++){
    bin_dif[i] /= bin_sum[i];
  }
  sp_free(bin_sum);
  return bin_dif;
}


real total_r_factor(Image * a, Image * b){
    real dif = 0;
    real sum = 0;
    for(int i = 0;i<sp_image_size(a);i++){
      dif += fabs(sp_cabs(a->image->data[i])-sp_cabs(b->image->data[i]));
      sum += fabs(sp_cabs(a->image->data[i])+sp_cabs(b->image->data[i]));      
    }
    return dif/sum;
}

real image_avg(Image * a){
  real res = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    res += sp_cabs(a->image->data[i]);
  }
  res /= sp_image_size(a);
  return res;
}

real image_std_dev(Image * a, real avg){
  real res = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    res += (sp_cabs(a->image->data[i])-avg)*(sp_cabs(a->image->data[i])-avg);
  }
  res /= sp_image_size(a);
  return sqrt(res);
}


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


/* Scales image B to image A using a linear regression */
void linear_regression_scale_img(Image * a, Image * b){
  /* ploting b->image->data[i] with respect to a->image->data[i] should give a straight
   line with slope 1. That's what we're exploiting here */
  /* From simple linear regression theory the line y = a + b * x has:
   
  b = sum{(xi-x_avg)(yi-y_avg)}/sum{(xi-x_avg)^2}
  a = y_avg - b * x_avg

*/
  real a_avg = image_avg(a);
  real b_avg = image_avg(b);
  real slope = 0;
  real inter = 0;
  real up = 0;
  real down = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    up += (sp_cabs(b->image->data[i])-b_avg)*(sp_cabs(a->image->data[i])-a_avg);
    down += (sp_cabs(a->image->data[i])-a_avg)*(sp_cabs(a->image->data[i])-a_avg);
  }
  slope = up/down;
  inter = b_avg - slope * a_avg;

  /* Now scale b */

  for(int i = 0;i<sp_image_size(a);i++){
    real mag = sp_cabs(b->image->data[i]);
    if(mag < inter){
      sp_real(b->image->data[i]) = 0;
      sp_imag(b->image->data[i]) = 0;
    }else{
      sp_real(b->image->data[i]) *= (mag-inter)/mag;
      sp_imag(b->image->data[i]) *= (mag-inter)/(mag);
      sp_real(b->image->data[i]) /= slope;
      sp_imag(b->image->data[i]) /= slope;
    }
  }
}

int main(int argc, char ** argv){
  Options * opts;  
  int nshells = 141;
  real * shell_res = NULL;
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);


  if(opts->dephase_rs_img){
    sp_image_dephase(opts->rs_img);
  }
  if(opts->rs_sol && opts->rs_img){
    sp_image_superimpose(opts->rs_sol,opts->rs_img,SP_ENANTIOMORPH);
  }
/*  sprintf(buffer,"superimposed_%s.vtk",opts->image_file);
  sp_image_write(opts->rs_img,buffer,0);*/

  if(opts->rs_sol){
    opts->sol = sp_image_fft(opts->rs_sol);
  }else{
    opts->sol = opts->fs_sol;
  }
  if(opts->rs_img){
    opts->img = sp_image_fft(opts->rs_img);
  }else{
    opts->img = opts->fs_img;
  }
  if(!opts->img || !opts->sol){
    printf("Use %s -h for details on how to run this program\n",argv[0]);
    exit(0);
  }

  if(sp_image_size(opts->img) != sp_image_size(opts->sol)){
    printf("Images must have the same dimensions!\n");
    exit(0);
  }



  real ratio = image_avg(opts->sol)/image_avg(opts->img);
  for(int i = 0;i<sp_image_size(opts->sol);i++){
   sp_real(opts->img->image->data[i]) *= ratio;
   sp_imag(opts->img->image->data[i]) *= ratio;
  }
    
  linear_regression_scale_img(opts->sol, opts->img);
  real * r_factors = res_dep_r_factor(opts->sol, opts->img, nshells);
  fprintf(stderr,"Total r factor - %f\n",total_r_factor(opts->sol,opts->img));
  if(opts->max_res){
    shell_res = malloc(sizeof(real)*nshells);
    for(int i = 0;i<nshells;i++){
      shell_res[i] = 1.0/((1.0/opts->max_res)*((i+0.5)/nshells));
    }
  }

  if(opts->output_file[0]){
    write_array_file(opts->output_file,nshells,r_factors,shell_res);
  }else{
    write_array_file("r_factors.data",nshells,r_factors,shell_res);
  }

  write_array_file("solution_int_dist.data",nshells,bin_image_by_r(opts->sol,nshells),shell_res);
  write_array_file("image_int_dist.data",nshells,bin_image_by_r(opts->img,nshells),shell_res);
  return 0;
}
