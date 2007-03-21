#include <stdlib.h>
#include <stdio.h>
#include <tiffio.h>
#include <hdf5.h>
#include <getopt.h>
#include "spimage.h"
#include "tiff2h5.h"

void set_defaults(Options * opt){
  opt->x_center = -1;
  opt->y_center = -1;
  opt->detector_distance = 50;
  opt->pixel_size = 0.020;
  opt->lambda = 32;
  opt->input[0] = 0;
  opt->output[0] = 0;
}


Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
"    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -x: Center of image x coordinate (690)\n\
    -y: Center of image y coordinate (608)\n\
    -d: Distance to detector (in mm) (50)\n\
    -p: Pixel Size (in mm) (0.020)\n\
    -l: Wavelength (in nm) (32)\n\
";
  static char optstring[] = "x:y:d:p:l:hi:o:";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'x':
      res->x_center = atof(optarg);
      break;
    case 'y':
      res->y_center = atof(optarg);
      break;
    case 'd':
      res->detector_distance = atof(optarg);
      break;
    case 'p':
      res->pixel_size= atof(optarg);
      break;
    case 'l':
      res->lambda = atof(optarg);
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


int main(int argc, char ** argv){
  Image * out;
  Image * autocorrelation;
  int width,height;
  char buffer[1024];
  out->detector = malloc(sizeof(Detector));
  Options * opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);

  if(!opts->input[0] || !opts->output[0]){
    printf("Use %s -h for details on how to run this program\n",argv[0]);
    exit(0);
  }
  out = sp_image_read(opts->input,0);

  if(opts->x_center == -1){
    opts->x_center = width/2;
  }
  if(opts->y_center == -1){
    opts->y_center = height/2;
  }
  out->detector->image_center[0] = opts->x_center;
  out->detector->image_center[1] = opts->y_center;
  out->detector->pixel_size = opts->pixel_size;
  out->detector->detector_distance = opts->detector_distance;
  out->detector->lambda = opts->lambda;
  /* write HDF5 */
  sp_image_write(out,opts->output,sizeof(real));
  autocorrelation = bilinear_rescale(sp_image_shift(sp_image_fft(sp_image_shift(out))),256,256);
  
  sp_image_write(autocorrelation,"autocorrelation.png",COLOR_JET|LOG_SCALE);
  sp_image_write(autocorrelation,"autocorrelation.vtk",0);
  sprintf(buffer,"%s.png",opts->output);
  sp_image_write(out,buffer,COLOR_JET|LOG_SCALE);
  sprintf(buffer,"%s.vtk",opts->output);
  sp_image_write(out,buffer,0);
  return 0;
}
