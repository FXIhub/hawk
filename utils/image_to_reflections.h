#ifndef _IMAGE_TO_REFLECTIONS_H_
#define _IMAGE_TO_REFLECTIONS_H_



/*
  FourierEwald2D means that the image corresponds to a real diffraction pattern of a object,
  so in Fourier space it is the fourier transform of the object sampled on the ewald sphere.

  FourierPlane2D means that the image correponds to the fourier transform of the object sampled
  in a plane normal to the beam. This correponds to a pure projection image.

  FourierGrid3D correponds to a series of FourierPlane2D arranged in a grid, or to the fourier
  transform of the object sampled on a grid, as is the case with the fast fourier transform.
*/
typedef enum{FourierEwald2D,FourierPlane2D,FourierGrid3D}SpImageType;


typedef struct{
  real a;
  real b;
  real c;
  real alpha;
  real beta;
  real gamma;   
}Cell;

typedef struct {
  char intensity_filename[1024];
  Image * intensity;
  char object_filename[1024];
  Image * object;
  char reflections_filename[1024];
  Image * reflections;
  real wavelength;
  real distance;
  real px;
  real py;  
  real pz;  
  int verbose;
  sp_3matrix * k_x;
  sp_3matrix * k_y;
  sp_3matrix * k_z;
  sp_3matrix * H;
  sp_3matrix * K;
  sp_3matrix * L;
  Image * Fc;
  Cell cell;
  int stride;
  real sigma;
}Options;

Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opt);
int image_index_to_coords(Image * in,int index,int * x, int * y, int * z, SpOrigin origin);
void image_fourier_coords(Image * in, sp_3matrix ** k_x, sp_3matrix ** k_y, sp_3matrix ** k_z, SpImageType image_type);
void image_fourier_coords_to_hkl(Options * opts);

#endif
