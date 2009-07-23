#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "spimage.h"


typedef struct{
  Image * image;
  Image * solution;
  char image_file[1024];
}Options;

void set_defaults(Options * opt){
  opt->image = 0;
  opt->solution = 0;
  opt->image_file[0] = 0;
}


void average_phase_error(Image * ra, Image * rb){
  Image * a = sp_image_fft(ra);
  Image * b = sp_image_fft(rb);
  int nbins = 50;
  real error[nbins];
  int n[nbins];
  real max_d = 0;
  for(int i =0 ;i<nbins;i++){
    n[i] = 0;
    error[i] = 0;
  }
  for(int i = 0;i<sp_image_size(a);i++){
    real d = sp_image_dist(a,i,SP_TO_CENTER);
    if(d > max_d){
      max_d = d;
    }
  }
  for(int i = 0;i<sp_image_size(a);i++){
    real d = sp_image_dist(a,i,SP_TO_CENTER);
    int bin = (nbins*d)/max_d;
    n[bin]++;
    real phi = fmod(2*M_PI+sp_carg(a->image->data[i])-sp_carg(b->image->data[i]),2*M_PI);
    error[bin] += sp_min(fabs(phi),fabs(2*M_PI-phi));
  }
  FILE * f = fopen("phase_error.data","w");
  for(int i =0 ;i<nbins;i++){
    if(n[i]){
      error[i] /= n[i];
    }
    fprintf(f,"%f %f %d\n",(max_d*i)/nbins,error[i],n[i]);
  }
  fclose(f);  
  sp_image_free(a);
  sp_image_free(b);
}

Options * parse_options(int argc, char ** argv){
  static char help_text[] = 
"    Options description:\n\
    \n\
    -i: Image file\n\
    -s: Solution file\n\
";
  static struct option long_options[] = {
    {"image", 1, 0, 'i'},
    {"solution", 1, 0, 's'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };
  static char optstring[] = "i:s:h";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);
  int c;
  while(1){
    c = getopt_long(argc,argv,optstring,long_options,NULL);
    if(c == -1){
      break;
    }
    switch(c){
    case 'i':
      res->image = sp_image_read(optarg,0);
      strcpy(res->image_file,optarg);
      break;
    case 's':
      res->solution = sp_image_read(optarg,0);
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


int main(int argc, char ** argv){
  Options * opts;  
  char buffer[1024];
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);

  if(!opts->image || !opts->solution){
    printf("Use %s -h for details on how to run this program\n",argv[0]);
    exit(0);
  }

  if(sp_image_size(opts->image) != sp_image_size(opts->solution)){
    printf("Images must have the same dimensions!\n");
    exit(0);
  }
  if(opts->solution->shifted){
    Image * tmp = sp_image_shift(opts->solution);
    sp_image_free(opts->solution);
    opts->solution =  tmp;
  }

  real sum_sol = 0;
  real sum_img = 0;
  for(int i = 0; i < sp_image_size(opts->image);i++){
    sum_sol += sp_cabs(opts->solution->image->data[i]);
    sum_img += sp_cabs(opts->image->image->data[i]);
  }

  real ratio = sum_sol/sum_img;
  for(int i = 0; i < sp_image_size(opts->image);i++){
    sp_real(opts->image->image->data[i]) *= ratio;
    sp_imag(opts->image->image->data[i]) *= ratio;
  }

  
  sp_image_dephase(opts->solution);
  sp_image_superimpose_fractional(opts->solution,opts->image,SpEnantiomorph|SpCorrectPhaseShift,2);
    //  sp_image_dephase(opts->image);
    //    sp_image_superimpose(opts->solution,opts->image,SpEnantiomorph);
  sprintf(buffer,"superimposed_%s.vtk",opts->image_file);
  sp_image_write(opts->image,buffer,0);
  sprintf(buffer,"superimposed_%s.h5",opts->image_file);
  sp_image_write(opts->image,buffer,0);
  printf("Real space R factor = %f\n",100*sp_image_rs_r_factor(opts->image,opts->solution));
  printf("Correlation coefficient = %f\n",sp_image_correlation_coefficient(opts->image,opts->solution));
  average_phase_error(opts->solution,opts->image);
  return 0;
}
