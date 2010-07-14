#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>
#include <hdf5.h>
#include <fftw3.h>
#include <getopt.h>
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif

#include "spimage.h"
#include "process_image.h"


void fill_image_blanks(Image * a, char * filler_image){
  Image * filler = sp_image_read(filler_image,0);
  if(filler->shifted){
    Image * tmp = sp_image_shift(filler);
    sp_image_free(filler);
    filler = tmp;
  }
  real x,y,z;
  if(!filler){
    return;
  }
  for(int i= 0;i < sp_image_size(a);i++){
    if(a->mask->data[i] == 0){
      sp_image_get_coords_from_index(a,i,&x,&y,&z,SpImageCenter);
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

Image * centrosymetry_average(Image * img){
  int x,y,z;
  int xs,ys,zs;
  int i,is;
  Image * out = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  real num;
  int den;
  for(x = 0;x<sp_c3matrix_x(img->image);x++){
    xs = 2*img->detector->image_center[0]-x;
    if(xs >= 0 && xs < sp_c3matrix_x(img->image)){
      for(y = 0;y<sp_c3matrix_y(img->image);y++){
	ys = 2*img->detector->image_center[1]-y;
	if(ys >= 0 && ys < sp_c3matrix_y(img->image)){
	  for(z = 0;z<sp_c3matrix_z(img->image);z++){
	    zs = 2*img->detector->image_center[2]-z;
	    if(zs >= 0 && zs <sp_c3matrix_z(img->image)){

	      i = z*sp_c3matrix_y(img->image)*sp_c3matrix_x(img->image)+
		y*sp_c3matrix_x(img->image)+x;
	      is = zs*sp_c3matrix_y(img->image)*sp_c3matrix_x(img->image)+
		ys*sp_c3matrix_x(img->image)+xs;
	      den = 0;
	      num = 0;
	      if(img->mask->data[i]){
		den++;
		num += sp_cabs(img->image->data[i]);
	      }
	      if(img->mask->data[is]){
		den++;
		num += sp_cabs(img->image->data[is]);
	      }
	      if(den){
		out->image->data[i] = sp_cinit(num/den,0);
		out->mask->data[i] = 1;
		out->image->data[is] = sp_cinit(num/den,0);
		out->mask->data[is] = 1;
	      }else{
		out->image->data[i] = sp_cinit(0,0);
		out->mask->data[i] = 0;
		out->image->data[is] = sp_cinit(0,0);
		out->mask->data[is] = 0;
	      }
	    }
	  }
	}
      }      
    }
  }
  return out;
}

void subtract_dark(Image * img, Image * dark){
  int i;
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    sp_real(img->image->data[i]) -= sp_cabs(dark->image->data[i]);
    if(sp_real(img->image->data[i]) < 0){
      img->image->data[i] = sp_cinit(0,0);
    }
  }
}

Image * speed_pad_image(Image * in){
  char buffer[1024];
  char buffer2[1024];
  char * home_dir;
  FILE * f;
  int size,time,opt_size;
  #if defined(_MSC_VER) || defined(__MINGW32__)
  home_dir = getenv("USERPROFILE");
#else
  home_dir = getenv("HOME");
#endif
  sprintf(buffer2,"%s/.uwrapc/fft_speed",home_dir);  
  f = fopen(buffer2,"r");
  if(!f){
    /* For some reason can't access benchmark file */
    return sp_image_duplicate(in,SP_COPY_DATA|SP_COPY_MASK);
  }
  while(fgets(buffer,1023,f)){
    sscanf(buffer,"%d\t%d\t%d\t",&size,&time,&opt_size);
    if(size == sp_c3matrix_x(in->image)){
      /* we have a match */
      return zero_pad_image(in,opt_size,opt_size,1,1);
    }else if(size < sp_c3matrix_x(in->image)){
      return sp_image_duplicate(in,SP_COPY_DATA|SP_COPY_MASK);
    }
  }
  return NULL;
}


real sum_square(Image * in, int x1, int y1, int z1, int x2, int y2, int z2){
  int i,j,k;
  real sum = 0;
  for(i = x1;i <= x2;i++){
    for(j = y1;j <= y2;j++){
      for(k = z1; k <= z2;k++){
	sum += sp_cabs(in->image->data[k*sp_c3matrix_y(in->image)*sp_c3matrix_x(in->image)+j*sp_c3matrix_x(in->image)+i]);
      }
    }
  }
  return sum;
}


real sum_square_edge(Image * in, int x1, int y1, int x2, int y2){
  int i,j;
  real sum = 0;
  for(i = x1;i <= x2;i++){
    for(j = y1;j<=y2;j++){
      if(i != x1 && i != x2){
	sum += sp_cabs(in->image->data[j*sp_c3matrix_x(in->image)+x1]);
	sum += sp_cabs(in->image->data[j*sp_c3matrix_x(in->image)+x2]);
	break;
      }
      sum += sp_cabs(in->image->data[j*sp_c3matrix_y(in->image)+i]);
    }
  }
  return sum;
}


real max_square_edge(Image * in, int x1, int y1, int x2, int y2){
  int i,j;
  real max = 0;
  for(i = x1;i <= x2;i++){
    for(j = y1;j<=y2;j++){
      if(i != x1 && i != x2){
	if(max < sp_real(in->image->data[j*sp_c3matrix_x(in->image)+x1])){
	  max = sp_real(in->image->data[j*sp_c3matrix_x(in->image)+x1]);
	}else if(max < sp_real(in->image->data[j*sp_c3matrix_x(in->image)+x2])){
	  max = sp_real(in->image->data[j*sp_c3matrix_x(in->image)+x2]);
	}
	break;
      }
      if(max < sp_real(in->image->data[j*sp_c3matrix_y(in->image)+i])){
	max = sp_real(in->image->data[j*sp_c3matrix_y(in->image)+i]);
      }
    }
  }
  return max;
}


Image * limit_sampling(Image * img, real oversampling_factor, real cutoff){
  /* The space limiting criteria will be everything 10x smaller than the patterson cutoff */
  /* I'm gonna take the patterson of a blurred version of the diffraction pattern due to "hot pixels" and "blue spots" */
  Image * blur_pat = sp_image_patterson(sp_gaussian_blur(img,5));
  Image * pat = sp_image_patterson(img);
  Image * resampled;
  Image * s_pat;
  Image * out;
  real max = 0;
  real current_max = 0;  
  int i;
  sp_image_write(pat,"pat.png",SpColormapJet);
  max = sp_c3matrix_max(blur_pat->image,NULL);
  for(i = 0;;i++){
    current_max = max_square_edge(blur_pat,i,i,sp_c3matrix_x(blur_pat->image)-i-1, sp_c3matrix_x(blur_pat->image)-i-1);
    if(max * cutoff < current_max){
      break;
    }
  }
  sp_image_free(blur_pat);
  s_pat = sp_image_shift(pat);
  sp_image_free(pat);
  out = bilinear_rescale(img,(sp_c3matrix_x(s_pat->image)/2-i)*oversampling_factor*2,(sp_c3matrix_x(s_pat->image)/2-i)*oversampling_factor*2,1);
  resampled = sp_image_low_pass(s_pat,(sp_c3matrix_x(s_pat->image)/2-i)*oversampling_factor,1);
  sp_image_free(s_pat);
  return out;
}

Image * downsample(Image * img, real downsample_factor){
  int size_x = sp_c3matrix_x(img->image)/downsample_factor;
  int size_y = sp_c3matrix_y(img->image)/downsample_factor;
/*  Image * mask;
  Image * low_passed = low_pass_gaussian_filter(img,size);
  write_png(low_passed,"low_passed.png",SpColormapJet|SpColormapLogScale);
  mask = sp_image_duplicate(low_passed,SP_COPY_DATA|SP_COPY_MASK);
  memcpy(mask->image,mask->mask,sp_cmatrix_size(mask->image)*sizeof(real));
  write_png(mask,"low_passed_mask.png",SpColormapJet|SpColormapLogScale);
  sp_image_free(mask);*/

  Image * downsampled =  bilinear_rescale(img,size_x,size_y,1);
  sp_image_write(downsampled,"downsampled.png",SpColormapJet|SpColormapLogScale);
/*  mask = sp_image_duplicate(downsampled,SP_COPY_DATA|SP_COPY_MASK);
  memcpy(mask->image,mask->mask,sp_cmatrix_size(mask->image)*sizeof(real));
  write_png(mask,"downsampled_mask.png",SpColormapJet|SpColormapLogScale);
  sp_image_free(low_passed);
  sp_image_free(mask);*/
  return downsampled;    
}


/* Search and mask overexposure */
void mask_overexposure(Image * img,real saturation){
  long long i;
  
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
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
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    sp_real(img->image->data[i]) -= opt->background;
    if(sp_real(img->image->data[i]) < 0){
      sp_real(img->image->data[i]) = 0;
      sp_imag(img->image->data[i]) = 0;
    }
  }  
}



/* Remove all signal deemed too small */
void remove_noise(Image * img, Image * noise){
  long long i;
/*  Image * variance = image_local_variance(noise,rectangular_window(10,10,10,10,0));*/
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    if((sp_real(img->image->data[i]) - sp_real(noise->image->data[i])) < sqrt(sp_real(img->image->data[i]))){
      sp_real(img->image->data[i]) = 0;
    }else{
      sp_real(img->image->data[i]) -= sp_real(noise->image->data[i]);
    }
  }
}


/* Get amplitudes */
void intensity_to_amplitudes(Image * img){
  long long i;
  img->scaled = 1;
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    if(img->mask->data[i]){
      sp_real(img->image->data[i]) = sqrt(sp_real(img->image->data[i]));
    }else{
      sp_real(img->image->data[i]) = 0;
    }
  }
}



Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -s: Saturation level of the detector (20000)\n\
    -a: Downsample by x times\n\
    -g: Background level of the detector (550)\n\
    -b: Beamstop size in pixels (55)\n\
    -m: Mask to mask out pixels\n\
    -p: Pad image to improve fft speed\n\
    -r: Maximum resolution to use in pixels (400)\n\
    -C: Use centrosymetry average\n\
    -c: User set image center (300x300)\n\
    -d: Dark image file\n\
    -S: Shift quadrants\n\
    -n: Noise image file\n\
    -f: High pass filter of a given radius\n\
    -t: Smoothness of the transition zone between 0 and 1 in the high\n\
        pass filter mathematically corresponds to the denominator of\n\
        exponential used in the fermi dirac distribution filter.\n\
    -e: Image file used to add the extra pixels used to fill the gaps in the mask\n\
    -v: Produce lots of output files for diagnostic\n\
    -h: print this text\n\
";
  static char optstring[] = "c:Cx:s:b:r:hi:o:g:a:pd:m:vSn:f:t:e:";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'a':
      res->oversampling_factor = atof(optarg);
      break;
    case 'g':
      res->background = atof(optarg);
      break;
    case 's':
      res->saturation = atof(optarg);
      break;
    case 'b':
      res->beamstop= atof(optarg);
      break;
    case 'r':
      res->resolution = atof(optarg);
      break;
    case 'x':
      res->cross_removal = atof(optarg);
      break;
    case 'i':
      strcpy(res->input,optarg);
      break;
    case 'o':
      strcpy(res->output,optarg);
      break;
    case 'n':
      strcpy(res->noise,optarg);
      break;
    case 'd':
      strcpy(res->dark,optarg);
      break;
    case 'm':
      strcpy(res->mask,optarg);
      break;
    case 'C':
      res->centrosymetry = 1;
      break;
    case 'c':
      res->user_center_x = atof(optarg);
      res->user_center_y = atof(strstr(optarg,"x")+1);
      break;
    case 'p':
      res->pad = 1;
      break;
    case 'v':
      res->verbose = 1;
      break;
    case 'f':
      res->high_pass_radius = atof(optarg);
      break;
    case 't':
      res->high_pass_transition_smoothness = atof(optarg);
      break;
    case 'e':
      strcpy(res->filler_image,optarg);
      break;

    case 'S':
      res->shift_quadrants = 1;
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  if(res->oversampling_factor &&
     res->user_center_x >= 0 &&
     res->user_center_y >= 0){
    res->user_center_x /= res->oversampling_factor;
    res->user_center_y /= res->oversampling_factor;    
  }
  return res;
}

void set_defaults(Options * opt){
  opt->background = 0;
  opt->saturation = 0;
  opt->beamstop = 0;
  opt->resolution = 0;
  opt->input[0] = 0;
  opt->dark[0] = 0;
  opt->noise[0] = 0;
  opt->output[0] = 0;
  opt->oversampling_factor = 0;
  opt->cross_removal = 0;
  opt->pad = 0;
  opt->mask[0] = 0;
  opt->filler_image[0] = 0;
  opt->verbose = 0;
  opt->high_pass_radius = 0;
  opt->high_pass_transition_smoothness = 1;
  opt->centrosymetry = 0;
  opt->shift_quadrants = 0;
  opt->user_center_x = -1;
  opt->user_center_y = -1;
}


void high_pass_filter(Image * a, Options * opt){
  int i;
  real factor;
  real distance;
  for(i = 0;i<sp_image_size(a);i++){
    distance = sp_image_dist(a,i,SP_TO_CENTER);
    factor = 1-1/(1+exp((distance-opt->high_pass_radius)/opt->high_pass_transition_smoothness));
    sp_real(a->image->data[i]) *= factor;
    sp_imag(a->image->data[i]) *= factor;
    /* If we're making the value basically zero we can afford to make the mask 1 because we know
     that it's value is a well known zero */
    if(factor < 1e-4){
      a->mask->data[i] = 1;
    }
  }
}


int main(int argc, char ** argv){
  Image * img;
  Image * out;
  Image * dark;
  Image * noise;
  Image * mask;

  Options * opts;  
  char buffer[1024] = "";
  char buffer2[1024] = "";
  int i;
  FILE * f;
  int tmp;
  real max =0;
  opts = malloc(sizeof(Options));
  set_defaults(opts);
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
    printf("Use process_image -h for details on how to run this program\n");
    exit(0);
  }

  img = sp_image_read(opts->input,0);
  printf("1 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->dark[0]){
    if(opts->verbose){
      sp_image_write(img,"before_minus_dark.png",SpColormapJet|SpColormapLogScale);
    }
    dark = sp_image_read(opts->dark,0);
    subtract_dark(img,dark);
    if(opts->verbose){
      sp_image_write(img,"after_minus_dark.png",SpColormapJet|SpColormapLogScale);
    }
  }
  if(opts->noise[0]){
    if(opts->verbose){
      sp_image_write(img,"before_minus_noise.png",SpColormapJet|SpColormapLogScale);
    }
    noise = sp_image_read(opts->noise,0);
    remove_noise(img,noise);
    if(opts->verbose){
      sp_image_write(img,"after_minus_noise.png",SpColormapJet|SpColormapLogScale);
    }
  }
  printf("2 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->mask[0]){
    mask = sp_image_read(opts->mask,0);
    if(sp_c3matrix_size(mask->image) != sp_c3matrix_size(img->image)){
      fprintf(stderr,"Mask file size different than image size\n");
      exit(1);
    }
    for(i = 0;i<sp_c3matrix_size(mask->image);i++){
      if(sp_real(mask->image->data[i]) == 0){
	img->mask->data[i] = 0;
      }
    }
    if(opts->verbose){
      write_mask_to_png(img,"after_apllying_mask.png",SpColormapGrayScale);
    }
  }
  printf("3 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
/*  sp_image_write(img,"really_before.png",SpColormapJet);*/

/*  write_vtk(img,"before.vtk");*/
/*  out =  make_unshifted_image_square(img);
  sp_image_write(out,"square.png",SpColormapJet);
  write_mask_to_png(out,"square_mask.png",SpColormapJet);
  sp_image_free(img);
  img = sp_image_duplicate(out,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_free(out);*/

  if(opts->saturation){
    mask_overexposure(img,opts->saturation); 
  }
  printf("4 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->verbose){
     write_mask_to_png(img,"after_saturation_mask.png",SpColormapJet);
  }

  printf("5 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->verbose){
    sp_image_write(img,"before_smooth.vtk",0);
  }

  printf("6 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  real value = 5;
  Image * soft_edge = sp_image_duplicate(img,SP_COPY_MASK|SP_COPY_DATA);
  sp_image_smooth_edges(soft_edge,soft_edge->mask,SP_GAUSSIAN,&value);
  if(opts->verbose){
    sp_image_write(soft_edge,"after_smooth.vtk",0);
  }
  printf("7 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  
  /*  autocorrelation = sp_image_patterson(soft_edge);
  sp_image_adaptative_constrast_stretch(autocorrelation,20,20);
  sp_image_write(autocorrelation,"autocorrelation.vtk",0);*/
  printf("8 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));

  if(opts->oversampling_factor){
    out = downsample(img,opts->oversampling_factor);
    sp_image_free(img);
    img = sp_image_duplicate(out,SP_COPY_DATA|SP_COPY_MASK);
    sp_image_free(out);
  }
  printf("8 (%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->verbose){
    sp_image_write(img,"after_resampling.png",SpColormapJet|SpColormapLogScale);
    write_mask_to_png(img,"after_resampling_mask.png",SpColormapJet);
  }
  
  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    if(img->mask->data[i] && sp_real(img->image->data[i]) > max){
      max = sp_real(img->image->data[i]);
    }
  }
  printf("max - %f\n",max);

  remove_background(img,opts);
  /*  if(!img->scaled){
    intensity_to_amplitudes(img);
    }*/

  printf("(%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->user_center_x > 0){
    img->detector->image_center[0] = opts->user_center_x;
    img->detector->image_center[1] = opts->user_center_y;
  }else{
    sp_find_center(img,&(img->detector->image_center[0]),&(img->detector->image_center[1]),&(img->detector->image_center[2]));
  }
  tmp = MIN(img->detector->image_center[0],(sp_c3matrix_x(img->image)-img->detector->image_center[0]));
  tmp = MIN(tmp,img->detector->image_center[1]);
  tmp = MIN(tmp,(sp_c3matrix_y(img->image)-img->detector->image_center[1]));
  printf("Minimum distance from center to image edge - %d\n",tmp);

  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    if(sp_image_dist(img,i,SP_TO_CENTER) < opts->beamstop /*&& img->img[i] < 1000*/){
      sp_real(img->image->data[i]) = 0;
      img->mask->data[i] = 0;
    }
    if(sp_image_dist(img,i,SP_TO_AXIS) < opts->cross_removal){
      sp_real(img->image->data[i]) = 0;
      img->mask->data[i] = 0;
    }
  }
  if(opts->verbose){
    sp_image_write(img,"after_beamstop.png",SpColormapJet);
  }


  if(opts->filler_image[0]){
    fill_image_blanks(img,opts->filler_image);
  }

  printf("(%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->centrosymetry){
    out = centrosymetry_average(img);
    sp_image_free(img);
    img = out;
  }  
  if(opts->shift_quadrants){
    out = sp_image_shift(img);
    if(opts->verbose){
      sp_image_write(out,"after_shift.png",SpColormapJet);
      write_mask_to_png(out,"after_shift_mask.png",SpColormapJet);
    }
  }else{
    out = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  }
  sp_image_free(img);
  if(opts->resolution){
    if(!out->shifted){
      Image * tmp = sp_image_shift(out);
      Image * tmp2 = sp_image_low_pass(tmp, opts->resolution,SP_2D);
      sp_image_free(tmp);
      img = sp_image_shift(tmp2);
      sp_image_free(tmp2);
    }else{
      img = sp_image_low_pass(out, opts->resolution,SP_2D);
    }
  }else{
    img = sp_image_duplicate(out,SP_COPY_DATA|SP_COPY_MASK);
  }
  if(opts->high_pass_radius){
    high_pass_filter(img,opts);
  }
  if(opts->verbose){
    sp_image_write(img,"after_shift_and_lim.png",SpColormapJet);
    write_mask_to_png(img,"after_shift_and_lim_mask.png",SpColormapJet);
  }

  printf("(%i,%i,%i)\n",sp_image_x(img),sp_image_y(img),sp_image_z(img));
  if(opts->pad){
    out = speed_pad_image(img);
  }else{
    out = sp_image_duplicate(img,SP_COPY_DATA|SP_COPY_MASK);
  }

  for(i = 0;i<sp_c3matrix_size(img->image);i++){
    if(!img->mask->data[i]){
      sp_real(img->image->data[i]) = 0;
    }  
  }
  
  if(sp_image_z(out) == 1){
    out->num_dimensions = SP_2D;
  }else{
    out->num_dimensions = SP_3D;
  }

  sp_image_write(out,opts->output,sizeof(real));
  return 0;
}
