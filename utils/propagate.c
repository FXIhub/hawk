#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <hdf5.h>
#include <fftw3.h>
#include <getopt.h>
#include <spimage.h>





Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -d: Distance to detector (mm)\n\
    -w: Wavelength (nm)\n\
    -W: Detector width (mm)\n\
    -H: Detector height (mm)\n\
    -z: Delta z (nm)\n\
    -h: help\n\
";
  static char optstring[] = "i:o:d:w:W:H:z:h";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'd':
      res->distance = atof(optarg);
      break;
    case 'w':
      res->lambda = atof(optarg);
      break;
    case 'p':
      res->pixel_size = atof(optarg);
      break;
    case 'z':
      res->delta_z = atof(optarg);
      break;
    case 'i':
      strcpy(res->input,optarg);
      break;
    case 'o':
      strcpy(res->output,optarg);
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

void set_defaults(Options * opt){
  opt->distance = 0;
  opt->lambda = 0;
  opt->width = 0;
  opt->height = 0;
  opt->input[0] = 0;
  opt->output[0] = 0;
  opt->delta_z = 0;
}



int main(int argc, char ** argv){
  Image * img;
  Image * out;
  Options * opts;  
  char buffer[1024] = "";
  char buffer2[1024] = "";
  sp_matrix *k_x,*k_y;
  int i;
  FILE * f;
  int tmp;
  real max =0;
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  /* write log */
  sprintf(buffer2,"%s.log",opts->output);
  f = fopen(buffer2,"w");
  fprintf(f,"%s\n",buffer);
  fclose(f);
  if(!opts->input[0] || !opts->output[0]){
    printf("Use propagate -h for details on how to run this program\n");
    exit(0);
  }

  img = sp_image_read(opts->input,0);
  if(img->shifted){
    img = sp_image_shift(img);
  }

  k_x = sp_matrix_alloc(sp_image_width(img),sp_image_height(img));
  k_y = sp_matrix_alloc(sp_image_width(img),sp_image_height(img));
  
  /* Make sure to set the image properties */
  img->detector->distance_to_detector = opts->distance;
  img->detector->lambda = opts->lambda;
  img->detector->pixel_size = opts->pixel_size;
  sp_image_fourier_coords(img,k_x,k_y,NULL);
  return 0;
}
