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
#include "propagate.h"


Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -d: Distance to detector (mm)\n\
    -w: Wavelength (nm)\n\
    -p: pixel size (um)\n\
    -z: Delta z (nm)\n\
    -h: help\n\
";
  static char optstring[] = "i:o:d:w:p:z:h";
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
      res->delta_z = atof(optarg)/1.0e9;
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
  opt->pixel_size = 0;
  opt->input[0] = 0;
  opt->output[0] = 0;
  opt->delta_z = 0;
}


Image * get_fresnel_propagator(Image * in, real delta_z){
  Image * res = sp_image_duplicate(in,SP_COPY_DATA|SP_COPY_MASK);
  /* number of pixels */
  int nx, ny;
  /* pixel index */
  int x,y;
  /* physical location of pixel*/
  real px,py;
  nx = sp_image_x(in);
  ny = sp_image_y(in);
  real lambda = in->detector->lambda;

  real det_width = in->detector->pixel_size[0] * sp_image_x(in);
  real det_height = in->detector->pixel_size[1] * sp_image_y(in);

  for(x = 0;x<nx;x++){
    for(y = 0;y<ny;y++){
      /* 
	 Calculate the pixel coordinates in reciprocal space 	 
	 by dividing the physical position by detector_distance*wavelength.
	 
 	 CCD center at image_center

	 Upper left corner of the detector with negative x and positive y
      */
      px = ((x-in->detector->image_center[0])/nx)*det_width;
      py = ((in->detector->image_center[1]-y)/ny)*det_height;
      sp_image_set(res,x,y,0,sp_cinit(cos(-M_PI/(delta_z*lambda)*(px*px+py*py)),-sin(M_PI/(delta_z*lambda)*(px*px+py*py))));
    }
  }
  return res;
}

Image * get_fourier_fresnel_propagator(Image * in,sp_3matrix * k_x, sp_3matrix *k_y, real delta_z){
  Image * res = sp_image_duplicate(in,SP_COPY_DATA|SP_COPY_MASK);
  Image * tmp;
  /* number of pixels */
  int nx, ny;
  /* pixel index */
  int x,y;
  /* physical location of pixel*/
  real lambda = in->detector->lambda;
  real k_x2;
  real k_y2;
  nx = sp_image_x(in);
  ny = sp_image_y(in);
  for(x = 0;x<nx;x++){
    for(y = 0;y<ny;y++){
      k_x2 = sp_3matrix_get(k_x,x,y,0);
      k_x2 *= k_x2;
      k_y2 = sp_3matrix_get(k_y,x,y,0);
      k_y2 *= k_y2;
      sp_image_set(res,x,y,0,sp_cinit(cos((delta_z*lambda/(4*M_PI))*(k_x2+k_y2)),sin((delta_z*lambda/(4*M_PI))*(k_x2+k_y2))));
    }
  }
  tmp = sp_image_shift(res);
  sp_image_free(res);
  return tmp;
}

int main(int argc, char ** argv){
  Image * img;
  Image * fresnel_prop;
  Image * out;
  Options * opts;  
  char buffer[1024] = "";
  char buffer2[1024] = "";
  sp_3matrix *k_x,*k_y;
  FILE * f;
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  int i;

#if defined(_MSC_VER) || defined(__MINGW32__)
  _getcwd(buffer,1023);
#else
  getcwd(buffer,1023);
#endif
  strcat(buffer,"> ");
  for(i = 0;i<argc;i++){
    strcat(buffer,argv[i]);
    strcat(buffer," ");
  }
  opts = parse_options(argc,argv);
  /* write log */
  sprintf(buffer2,"%s.log",opts->output);
  f = fopen(buffer2,"w");
  fprintf(f,"%s\n",buffer);
  fclose(f);
  if(!opts->input[0] || !opts->output[0]){
    printf("Use propagate -h for details on how to run this program\n");
    exit(0);
  }

  Image *tmp = sp_image_read(opts->input,0);

  img = sp_image_fft(tmp);

  k_x = sp_3matrix_alloc(sp_image_x(img),sp_image_y(img),1);
  k_y = sp_3matrix_alloc(sp_image_x(img),sp_image_y(img),1);
  
  /* Make sure to set the image properties */
  /* convet all to meters */
  
  img->detector->detector_distance = opts->distance/1.0e3;
  img->detector->lambda = opts->lambda/1.0e9;
  img->detector->pixel_size[0] = opts->pixel_size/1.0e6;
  img->detector->pixel_size[1] = opts->pixel_size/1.0e6;
  img->detector->pixel_size[2] = opts->pixel_size/1.0e6;
  img->detector->image_center[0] = (sp_image_x(img)-1.0)/2.0;
  img->detector->image_center[1] = (sp_image_y(img)-1.0)/2.0;
  img->detector->image_center[2] = (sp_image_z(img)-1.0)/2.0;
  
  sp_image_fourier_coords(img,k_x,k_y,NULL);
  
  /* Convolute input with the fresnel propagator */
  fresnel_prop = get_fourier_fresnel_propagator(img,k_x,k_y,opts->delta_z);

  out = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_mul_elements(out,fresnel_prop);
  sp_image_write(sp_image_ifft(out),opts->output,0);   
  return 0;
}
