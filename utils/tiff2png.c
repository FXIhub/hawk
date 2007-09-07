#include <stdlib.h>
#include <stdio.h>
#include <tiffio.h>
#include <hdf5.h>
#include <getopt.h>
#include "spimage.h"
#include "tiff2h5.h"

void set_defaults(Options * opt){
  opt->x_center = 690;
  opt->y_center = 608;
  opt->detector_distance = 50;
  opt->pixel_size = 0.020;
  opt->lambda = 32;
  opt->input[0] = 0;
  opt->output[0] = 0;
}


Options * parse_options(int argc, char ** argv){
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
  static struct option long_options[] = {
    {"x_center", 1, 0, 'x'},
    {"y_center", 1, 0, 'y'},
    {"detector_d", 1, 0, 'd'},
    {"pixel_size", 1, 0, 'p'},
    {"wavelength", 1, 0, 'l'},
    {"input", 1, 0, 'i'},
    {"output", 1, 0, 'o'},
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
  };
  static char optstring[] = "x:y:d:p:l:hi:o:";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);
  int c;
  while(1){
    c = getopt_long(argc,argv,optstring,long_options,NULL);
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
  Options * opts;  
  char buffer[1024];
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);

  if(!opts->input[0] || !opts->output[0]){
    printf("Use %s -h for details on how to run this program\n",argv[0]);
    exit(0);
  }

  Image * out = sp_image_read(opts->input,0);
  /* write HDF5 */
  sprintf(buffer,"%s.png",opts->output);
  sp_image_write(out,buffer,COLOR_JET|LOG_SCALE);
  sprintf(buffer,"%s.vtk",opts->output);
  /*  write_vtk(out,buffer);*/
  return 0;
}
