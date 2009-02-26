#include <stdlib.h>
#include <hdf5.h>
#include "spimage.h"
#include <getopt.h>
#include <gsl/gsl_sf_bessel.h>



typedef struct {
  real saturation;
  real background;
  int verbose;
  char input[1024];
  char output[1024];
  char dark[1024];
  char mask[1024];
  char filler_image[1024];
  real user_center_x;
  real user_center_y;
  real distance;
  real wavelength;
  real pixel_size;
}Options;

Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opts);
void remove_background(Image * img, Options * opt);
void mask_overexposure(Image * img,real saturation);
void subtract_dark(Image * img, Image * dark);
void fill_image_blanks(Image * a, char * filler_image);
void plot_shell_intensity(Image * a, Options * opts);
Image *  resolution_from_image(Image * a);
double ball_intensity(double radius, double d, double q);
int discrete_3d_spherically_symmetric_fourier_transform(real * r, real * f_of_r, int n, real ** _R, real ** _F_of_R,int new_n);
void normalize_f(real *f, int n);

void normalize_f(real *f, int n){
  double sum = 0;
  for(int i = 0;i<n;i++){
    sum += f[i];
  }
  for(int i = 0;i<n;i++){
    f[i] /= sum;
  }
}
int discrete_3d_spherically_symmetric_fourier_transform(real * r, real * f_of_r, int n, real ** _R, real ** _F_of_R, int new_n){
  /*
    According to Fundamentals of Crystallography By Carmelo Giacovazzo
    the spherically symmetric fourier transform is given by:
    F(R) = \frac{1}{R} \int_0^\infty 2 r f(r) sin(2 \pi r R) dr

    We're gonna evaluate between R = 0 and R = n/max_r
   */
  *_R = malloc(sizeof(double)*new_n);
  *_F_of_R = malloc(sizeof(double)*new_n);
  real * R = *_R;
  real * F_of_R = *_F_of_R;
  real max_r = r[n-1];
  for(int i = 0;i<new_n;i++){
    R[i] = i/max_r*((double)n/new_n);
  }
  F_of_R[0] = 0;
  for(int i = 1;i<new_n;i++){
    double sum = 0;
    for(int j = 0;j<n;j++){
      sum += 2*r[j]*f_of_r[j]*sin(2*M_PI*r[j]*R[i]);
    }
    sum /=  R[i];
    F_of_R[i] = sum;
  }
  return 0;  
}


double ball_intensity(double radius, double scale, double q){
  if(q == 0){
    return 0;
  }
  double s = 2*M_PI*radius*q;
  double F = gsl_sf_bessel_j1(s)*4*M_PI*radius*radius*radius/s;
  return scale*F*F;
}

Image *  resolution_from_image(Image * image){
  Image * res = sp_image_duplicate(image,SP_COPY_DATA|SP_COPY_DETECTOR);
  real d = image->detector->detector_distance;
  real w = image->detector->lambda;
  for(int i = 0;i<sp_image_size(image);i++){
    real h = sp_image_dist(image,i,SP_TO_CENTER);
    h *= image->detector->pixel_size[0];
    real theta = atan(h/d)/2;
    res->image->data[i] = sp_cinit(sin(theta)*2/w,0);
  }
  return res;
}


void plot_shell_intensity(Image * a, Options * opts){
  Image * resol = resolution_from_image(a);
  int nbins = sqrt(sp_image_size(a))/4;
  real * bin_sum = malloc(sizeof(bin_sum)*nbins);
  real * bin_res = malloc(sizeof(bin_sum)*nbins);
  int * bin_members = malloc(sizeof(bin_members)*nbins);
  real max_res = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    if(sp_real(resol->image->data[i]) > max_res){
      max_res = sp_real(resol->image->data[i]);
    }
  }
  for(int i = 0;i<nbins;i++){
    bin_sum[i] = 0;
    bin_members[i] = 0;    
    bin_res[i] = i*max_res/nbins;
  }

  for(int i = 0;i<sp_image_size(a);i++){
    if(a->mask->data[i] == 0){
      continue;
    }
    real res = sp_real(resol->image->data[i]);
    int bin = (res/max_res)*nbins;
    if(bin >= nbins){
      bin = nbins-1;
    }
    bin_sum[bin] += sp_cabs(a->image->data[i]);
    bin_members[bin]++;
  }
  FILE * out = fopen(opts->output,"w");
  char buffer[1024];
  strcpy(buffer,opts->output);
  strcat(buffer,"_real_space.data");
  FILE * rs = fopen(buffer,"w");
  for(int i =0;i<nbins;i++){
    if(bin_members[i] == 0){
      bin_sum[i] = 0;
    }else{
      bin_sum[i] /= bin_members[i];
    }
  }
  real * R;
  real * F_of_R;
  int new_n = nbins*20;
  discrete_3d_spherically_symmetric_fourier_transform(bin_res, bin_sum,nbins, &R, &F_of_R,new_n);
  normalize_f(F_of_R,nbins);

  for(int i =0;i<nbins;i++){
    fprintf(out,"%g %g %d\n",bin_res[i],bin_sum[i],bin_members[i]);
  }
  for(int i =0;i<nbins;i++){
    fprintf(rs,"%g %g\n",R[i],F_of_R[i]);
    printf("%g %g\n",R[i],F_of_R[i]);
  }
  fclose(rs);
  fclose(out);
}

void fill_image_blanks(Image * a, char * filler_image){
  Image * filler = sp_image_read(filler_image,0);
  if(!filler){
    return;
  }

  if(filler->shifted){
    Image * tmp = sp_image_shift(filler);
    sp_image_free(filler);
    filler = tmp;
  }
  real x,y,z;
  for(int i= 0;i < sp_image_size(a);i++){
    if(a->mask->data[i] == 0){
      sp_image_get_coords_from_index(a,i,&x,&y,&z,ImageCenter);
      if(x+filler->detector->image_center[0] >= 0 && x+filler->detector->image_center[0] < sp_image_x(filler) &&
	 y+filler->detector->image_center[1] >= 0 && y+filler->detector->image_center[1] < sp_image_y(filler) &&
	 z+filler->detector->image_center[2] >= 0 && z+filler->detector->image_center[2] < sp_image_z(filler)){
	int i2 = sp_image_get_index(filler,x+filler->detector->image_center[0],y+filler->detector->image_center[1],
				    z+filler->detector->image_center[2]);
	sp_real(a->image->data[i]) = sp_cabs(filler->image->data[i2]);
	a->mask->data[i] = 1;	  
      }
    }
  }
}

void subtract_dark(Image * img, Image * dark){
  int i;
  for(i = 0;i<sp_image_size(img);i++){
    sp_real(img->image->data[i]) -= sp_cabs(dark->image->data[i]);
    if(sp_real(img->image->data[i]) < 0){
      img->image->data[i] = sp_cinit(0,0);
    }
  }
}

void mask_overexposure(Image * img,real saturation){
  long long i;
  for(i = 0;i<sp_image_size(img);i++){
    /* 16 bit detector apparently */
    /* mask over 20k */
    if(sp_real(img->image->data[i]) >= saturation){
      img->mask->data[i] = 0;
      img->image->data[i] = sp_cinit(0,0);
    }
  }  
}

/* Search and mask overexposure */
void remove_background(Image * img, Options * opt){
  int i;
  for(i = 0;i<sp_image_size(img);i++){
    sp_real(img->image->data[i]) -= opt->background;
    if(sp_real(img->image->data[i]) < 0){
      sp_real(img->image->data[i]) = 0;
      sp_imag(img->image->data[i]) = 0;
    }
  }  
}

void set_defaults(Options * opts){
  opts->saturation = 0;
  opts->background = 0;
  opts->verbose = 0;
  opts->input[0] = 0;
  opts->output[0] = 0;
  opts->dark[0] = 0;
  opts->mask[0] = 0;
  opts->filler_image[0] = 0;
  opts->user_center_x = -1;
  opts->user_center_y = -1;
  opts->distance = 0;
  opts->wavelength = 0;
  opts->pixel_size = 0;
}



Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -d: Distance to detector in mm\n\
    -w: Wavelength in nm\n\
    -p: Pixel size in um\n\
    -s: Saturation level of the detector\n\
    -g: Background level of the detector\n\
    -m: Mask to mask out pixels\n\
    -c: User set image center (300x300)\n\
    -b: Background image file\n\
    -e: Image file used to add the extra\n\
        pixels used to fill the gaps in the mask\n\
    -v: Produce lots of output files for diagnostic\n\
    -h: print this text\n\
";
  static char optstring[] = "c:s:hi:o:g:d:m:ve:b:d:w:p:";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  if(argc == 1){
    printf("Usage: circular_averaging [ options ]\n");
    printf("%s",help_text);
    exit(1);
  }

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'g':
      res->background = atof(optarg);
      break;
    case 's':
      res->saturation = atof(optarg);
      break;
    case 'i':
      strcpy(res->input,optarg);
      break;
    case 'o':
      strcpy(res->output,optarg);
      break;
    case 'b':
      strcpy(res->dark,optarg);
      break;
    case 'm':
      strcpy(res->mask,optarg);
      break;
    case 'c':
      res->user_center_x = atof(optarg);
      res->user_center_y = atof(strstr(optarg,"x")+1);
      break;
    case 'v':
      res->verbose = 1;
      break;
    case 'e':
      strcpy(res->filler_image,optarg);
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    case 'w':
      res->wavelength = atof(optarg);
      break;
    case 'd':
      res->distance = atof(optarg);
      break;
    case 'p':
      res->pixel_size = atof(optarg);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  return res;
}


void check_options(Options * opts){
  int flag = 0;
  if(opts->pixel_size == 0){
    printf("You have to specify the pixel size in um with -p!\n");
    flag = 1;
  }
  if(opts->wavelength == 0){
    printf("You have to specify the wavelength in nm with -w!\n");
    flag = 1;
  }
  if(opts->distance == 0){
    printf("You have to specify the detector distance in um with -d!\n");
    flag = 1;
  }
  if(opts->user_center_x == -1 && opts->user_center_y == -1 ){
    printf("You have to specify the image center with -c!\n");
    flag = 1;
  }
  if(opts->input[0] == 0){
    printf("You have to specify the input image with -i!\n");
    flag = 1;
  }
  if(opts->output[0] == 0){
    printf("You have to specify the output image with -o!\n");
    flag = 1;
  }
  if(flag){
    exit(flag);
  }
}

int main(int argc, char ** argv){
  Image * image;
  Options * opts = parse_options(argc,argv);
  check_options(opts);
  char buffer[1024];
  strcpy(buffer,opts->output);
  strcat(buffer,".log");
  
  FILE * log = fopen(buffer,"w");
  strcpy(buffer,argv[0]);
  for(int i = 1;i<argc;i++){
    strcat(buffer," ");
    strcat(buffer,argv[i]);    
  }
  strcat(buffer,"\n");    
  fprintf(log,buffer);
  image = sp_image_read(opts->input,0);
  
  image->detector->image_center[0] = opts->user_center_x;
  image->detector->image_center[1] = opts->user_center_y;
  image->detector->detector_distance = opts->distance*1e-3;
  image->detector->pixel_size[0] = opts->pixel_size*1e-6;
  image->detector->pixel_size[1] = opts->pixel_size*1e-6;
  image->detector->pixel_size[2] = opts->pixel_size*1e-6;
  image->detector->lambda = opts->wavelength*1e-9;
    
  if(opts->saturation){
    mask_overexposure(image,opts->saturation); 
  }
  if(opts->verbose){
     sp_image_write(image,"after_saturation_mask.png",COLOR_GRAYSCALE);
  }

  if(opts->dark[0]){
    if(opts->verbose){
      sp_image_write(image,"before_minus_dark.png",COLOR_JET|LOG_SCALE);
    }
    Image * dark = sp_image_read(opts->dark,0);
    subtract_dark(image,dark);
    if(opts->verbose){
      sp_image_write(image,"after_minus_dark.png",COLOR_JET|LOG_SCALE);
    }
  }
  if(opts->mask[0]){
    Image * mask = sp_image_read(opts->mask,0);
    if(sp_image_size(mask) != sp_image_size(image)){
      fprintf(stderr,"Mask file size different than image size\n");
      exit(1);
    }
    for(int i = 0;i<sp_image_size(mask);i++){
      if(sp_real(mask->image->data[i]) == 0){
	image->mask->data[i] = 0;
      }
    }
    if(opts->verbose){
      write_mask_to_png(image,"after_apllying_mask.png",COLOR_GRAYSCALE);
    }
  }
  /* remove flat background */
  remove_background(image,opts);

  /* output intensity by shell plot*/
  plot_shell_intensity(image,opts);

  return 0;
}
