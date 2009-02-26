#include <spimage.h>
#include <getopt.h>

#include "image_to_reflections.h"




Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Diffraction Pattern file\n\
    -r: Real Space Model\n\
    -o: Output Reflection file\n\
    -w: Experimental Wavelength used (nm)\n\
    -d: Distance to Detector (mm)\n\
    -x: Pixel width (um)\n\
    -y: Pixel height (um)\n\
    -z: Pixel depth (um)\n\
    -s: Stride in pixels (box reduction factor)\n\
    -S: Sigma\n\
    -v: Produce lots of output files for diagnostic\n\
    -h: print this text\n\
";
  static char optstring[] = "i:r:o:w:d:x:y:vhs:z:S:";
  Options * opts = calloc(1,sizeof(Options));
  set_defaults(opts);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'i':
      strcpy(opts->intensity_filename,optarg);
      break;
    case 'r':
      strcpy(opts->object_filename,optarg);
      break;
    case 'o':
      strcpy(opts->reflections_filename,optarg);
      break;
    case 'w':
      /* the e-9 comes from the conversion from nm to m*/
      opts->wavelength = atof(optarg)*1e-9;
    case 'x':
      /* the e-6 comes from the conversion from um to m*/
      opts->px = atof(optarg)*1e-6;
      break;
    case 'y':
      opts->py = atof(optarg)*1e-6;
      break;
    case 'z':
      opts->pz = atof(optarg)*1e-6;
      break;
    case 's':
      opts->stride = atoi(optarg);
      break;
    case 'S':
      opts->sigma = atof(optarg);
      break;
    case 'd':
      /* the e-3 comes from the conversion from mm to m*/
      opts->distance = atof(optarg)*1e-3;;
      break;
    case 'v':
      opts->verbose = 1;
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  return opts;
}


void set_defaults(Options * opts){
  opts->intensity_filename[0] = 0;
  opts->object_filename[0] = 0;
  opts->reflections_filename[0] = 0;
  opts->verbose = 0;
  opts->wavelength = -1;
  opts->distance = -1;
  opts->px = -1;
  opts->py = -1;
  opts->pz = -1;
  opts->Fc = NULL;
  opts->intensity = NULL;
  opts->k_x = NULL;
  opts->k_y = NULL;
  opts->k_z = NULL;
  opts->stride = 1;
  opts->sigma = 0;
}


int image_index_to_coords(Image * in,int index,int * x, int * y, int * z, SpOrigin origin){
  int nx,ny,nz;
  nx = sp_image_x(in);
  ny = sp_image_y(in);
  nz = sp_image_z(in);
  if(origin == TopLeftCorner){
    *z = index/(ny*nx);
    *y = (index%(ny*nx))/nx;
    *x = index%(nx);
    return 0;
  }else if(origin == ImageCenter){
    *z = index/(ny*nx);
    *y = (index%(ny*nx))/nx;
    *x = index%(nx);
    *z -= in->detector->image_center[2];
    *y -= in->detector->image_center[1];
    *x -= in->detector->image_center[0];
    return 0;
  }else{
    return -1;
  }
  return -1;
}

void image_fourier_coords(Image * in, sp_3matrix ** k_x, sp_3matrix ** k_y, sp_3matrix ** k_z, SpImageType image_type){
  /* number of pixels */
  int nx, ny,nz;
  /* physical location of pixel*/
  real px,py,pz;
  /* reciprocal coordinates */
  real rx,ry,rz;
  real real_to_reciprocal = 1.0/(in->detector->detector_distance*in->detector->lambda);
  real ewald_radius = 1.0/in->detector->lambda;
  real distance_to_ewald_sphere_center;

  real det_x = in->detector->pixel_size[0] * sp_image_x(in);
  real det_y = in->detector->pixel_size[1] * sp_image_y(in);
  real det_z = in->detector->pixel_size[2] * sp_image_z(in);

  nx = sp_image_x(in);
  ny = sp_image_y(in);
  nz = sp_image_z(in);

  *k_x = sp_3matrix_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));
  *k_y = sp_3matrix_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));
  *k_z = sp_3matrix_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));

  /* Restrict the image_center to be on an integer pixel.
     This is required to make the Miller indeces come out as integers.*/
  in->detector->image_center[0] = (int)round(in->detector->image_center[0]);
  in->detector->image_center[1] = (int)round(in->detector->image_center[1]);
  in->detector->image_center[2] = (int)round(in->detector->image_center[2]);

  for(int z = 0;z<sp_image_z(in);z++){
    for(int y = 0;y<sp_image_y(in);y++){
      for(int x = 0;x<sp_image_x(in);x++){
	px = ((x-in->detector->image_center[0])/nx)*det_x;
	py = ((in->detector->image_center[1]-y)/ny)*det_y;
	pz = ((in->detector->image_center[2]-z)/nz)*det_z;
	
	rx = px*real_to_reciprocal;
	ry = py*real_to_reciprocal;
	rz = pz*real_to_reciprocal;
	
	if(image_type == FourierEwald2D){
	  /* Project pixel into Ewald sphere. */
	  distance_to_ewald_sphere_center = sqrt(rx*rx+ry*ry+ewald_radius*ewald_radius);
	  sp_3matrix_set(*k_x,x,y,0,rx * ewald_radius/distance_to_ewald_sphere_center);
	  sp_3matrix_set(*k_y,x,y,0,ry * ewald_radius/distance_to_ewald_sphere_center);
	  sp_3matrix_set(*k_z,x,y,0,ewald_radius-(ewald_radius * ewald_radius/distance_to_ewald_sphere_center));
	}else if(image_type == FourierPlane2D){
	  sp_3matrix_set(*k_x,x,y,0,rx);
	  sp_3matrix_set(*k_y,x,y,0,ry);
	  sp_3matrix_set(*k_z,x,y,0,0);
	}else if(image_type == FourierGrid3D){
	  sp_3matrix_set(*k_x,x,y,z,rx);
	  sp_3matrix_set(*k_y,x,y,z,ry);
	  sp_3matrix_set(*k_z,x,y,z,rz);
	}
      }
    }
  }
}

void image_fourier_coords_to_hkl(Options * opts){
  opts->H = sp_3matrix_alloc(sp_image_x(opts->intensity),sp_image_y(opts->intensity),sp_image_z(opts->intensity));
  opts->K = sp_3matrix_alloc(sp_image_x(opts->intensity),sp_image_y(opts->intensity),sp_image_z(opts->intensity));
  opts->L = sp_3matrix_alloc(sp_image_x(opts->intensity),sp_image_y(opts->intensity),sp_image_z(opts->intensity));
  
  /* Calculate inverse cell sizes */
  assert(sp_3matrix_get(opts->k_x,opts->intensity->detector->image_center[0],opts->intensity->detector->image_center[1],opts->intensity->detector->image_center[2]) == 0);
  assert(sp_3matrix_get(opts->k_y,opts->intensity->detector->image_center[0],opts->intensity->detector->image_center[1],opts->intensity->detector->image_center[2]) == 0);
  assert(sp_3matrix_get(opts->k_z,opts->intensity->detector->image_center[0],opts->intensity->detector->image_center[1],opts->intensity->detector->image_center[2]) == 0);
  real inv_a = fabs(sp_3matrix_get(opts->k_x,opts->intensity->detector->image_center[0]+opts->stride,
				   opts->intensity->detector->image_center[1],
				   opts->intensity->detector->image_center[2]));
  real inv_b = fabs(sp_3matrix_get(opts->k_y,opts->intensity->detector->image_center[0],
				   opts->intensity->detector->image_center[1]+opts->stride,
				   opts->intensity->detector->image_center[2]));
  real inv_c = fabs(sp_3matrix_get(opts->k_z,opts->intensity->detector->image_center[0],
				   opts->intensity->detector->image_center[1],
				   opts->intensity->detector->image_center[2]+opts->stride));

  opts->cell.a = 1.0/inv_a;
  opts->cell.b = 1.0/inv_b;
  opts->cell.c = 1.0/inv_c;

  /* In practise with rectangular pixels the cell is always 90 90 90 */
  opts->cell.alpha = 90.0;
  opts->cell.beta = 90.0;
  opts->cell.gamma = 90.0;

  for(int i = 0;i<sp_image_size(opts->intensity);i++){
    opts->H->data[i] = opts->k_x->data[i] / inv_a;
    opts->K->data[i] = opts->k_y->data[i] / inv_b;
    opts->L->data[i] = opts->k_z->data[i] / inv_c;
  }
}

void print_output(Options * opts){
  FILE * fp = stdout;
  Image * Fc = NULL;
  if(opts->object){
    if(opts->Fc->shifted){
      Fc = sp_image_shift(opts->Fc);
    }else{
      Fc = opts->Fc;
    }
  }
  if(opts->reflections_filename[0]){
    fp = fopen(opts->reflections_filename,"w");
    if(!fp){
      sp_error_fatal("Could not open %s for writing! Aborting.",opts->reflections_filename);
    }
  }
  /* Print file header */
  fprintf(fp,"#!/bin/sh\n");
  fprintf(fp,"# This file contains a list of reflections in a text\n");
  fprintf(fp,"# format suitable to be processed by CCP4's f2mtz.\n");
  fprintf(fp,"# To process issue to following command:\n");
  fprintf(fp,"f2mtz HKLIN $0 HKLOUT $0.mtz << +\n");
  fprintf(fp,"SKIP 20\n");
  fprintf(fp,"TITLE Hawk to MTZ\n");
  fprintf(fp,"CELL %f %f %f %f %f %f\n",1e10*opts->cell.a,1e10*opts->cell.b,1e10*opts->cell.c,opts->cell.alpha,opts->cell.beta,opts->cell.gamma);
  if(opts->object){
    fprintf(fp,"LABOUT H K L IMEAN SIGIMEAN FC PHIC\n");
    fprintf(fp,"CTYPE  H H H J Q F P\n");
    fprintf(fp,"FORMAT '(3F5.0,1X,F10.3,1X,F10.3,1X,F10.3,1X,F10.3)'\n");
    fprintf(fp,"SYMM P1\n");
    fprintf(fp,"+\n");
    fprintf(fp,"exit\n");
    for(int z = 0;z<sp_image_z(opts->intensity);z++){
      int logic_z = z-opts->intensity->detector->image_center[2];
      for(int y = 0;y<sp_image_y(opts->intensity);y++){
	int logic_y = y-opts->intensity->detector->image_center[1];
	for(int x = 0;x<sp_image_x(opts->intensity);x++){
	  int logic_x = x-opts->intensity->detector->image_center[0];
	  real H = sp_3matrix_get(opts->H,x,y,z);
	  real K = sp_3matrix_get(opts->K,x,y,z);
	  real L = sp_3matrix_get(opts->L,x,y,z);
	  Complex F = sp_image_get(Fc,logic_x+Fc->detector->image_center[0],logic_y+Fc->detector->image_center[1],logic_z+Fc->detector->image_center[2]);
	  if(H-(int)H < 0.01 ){
	    if(K-(int)K < 0.01 ){
	      if(L-(int)L < 0.01){
		/* We are on an integer HKL or we used a stride bigger than 100. Hopefully the former */
		/* Changing H for some strange reason that i don't really understand  */
		fprintf(fp,"%5d%5d%5d %10.3e %10.3e %10.3e %10.3e\n",(int)-round(H),(int)round(K),(int)round(L),sp_real(sp_image_get(opts->intensity,x,y,z)),opts->sigma,sp_cabs(F),sp_carg(F)*180/M_PI);
	      }
	    }
	  }
	}
      }
    }
  }else{
    fprintf(fp,"LABOUT H K L I\n");
    fprintf(fp,"CTYPE  H H H J\n");
    fprintf(fp,"FORMAT '(3F5.0,1X,F10.3)'\n");
    fprintf(fp,"SYMM P1\n");
    fprintf(fp,"+\n");
    fprintf(fp,"exit\n");

    for(int z = 0;z<sp_image_z(opts->intensity);z++){
      for(int y = 0;y<sp_image_y(opts->intensity);y++){
	for(int x = 0;x<sp_image_x(opts->intensity);x++){
	  real H = sp_3matrix_get(opts->H,x,y,z);
	  real K = sp_3matrix_get(opts->K,x,y,z);
	  real L = sp_3matrix_get(opts->L,x,y,z);
	  if(fabs(H-(int)H) < 0.01 ){
	    if(fabs(K-(int)K) < 0.01 ){
	      if(fabs(L-(int)L) < 0.01 ){
		/* We are on an integer HKL or we used a stride bigger than 100. Hopefully the former */
		fprintf(fp,"%5d%5d%5d %10.3e\n",(int)round(H),(int)round(K),(int)round(L),sp_real(sp_image_get(opts->intensity,x,y,z)));
	      }
	    }
	  }
	}
      }
    }
  }

  
}

int main(int argc, char ** argv){
  Options * opts =  parse_options(argc,argv);
  if(!opts->intensity_filename[0]){
    printf("Usage: image_to_reflections <-i pattern.h5> [-r object.h5] [-o reflection.txt] [-w wavelength] [-d distance] [-x px] [-y py]\n");
    exit(0);
  }
  opts->intensity = sp_image_read(opts->intensity_filename,0);
  if(!opts->intensity){
    sp_error_fatal("Cannot open file: %s",opts->intensity_filename);
  }
  if(opts->object_filename[0]){
    opts->object = sp_image_read(opts->object_filename,0);
    if(!opts->object){
      sp_error_fatal("Cannot open file: %s",opts->object_filename);
    }
  }

  /* Apply options */
  if(opts->wavelength > 0){
    opts->intensity->detector->lambda = opts->wavelength;
  }
  if(opts->distance > 0){
    opts->intensity->detector->detector_distance = opts->distance;
  }
  if(opts->px > 0){
      opts->intensity->detector->pixel_size[0] = opts->px;
    }
  if(opts->py > 0){
    opts->intensity->detector->pixel_size[1] = opts->py;
  }
  if(opts->pz > 0){
    opts->intensity->detector->pixel_size[2] = opts->pz;
  }

  image_fourier_coords(opts->intensity, &(opts->k_x), &(opts->k_y),&(opts->k_z),FourierGrid3D);
  image_fourier_coords_to_hkl(opts);
  if(opts->object){
    opts->Fc = sp_image_fft(opts->object);
    sp_image_write(opts->Fc,"Fc.h5",0);
  }

  print_output(opts);
  return 0;
}
