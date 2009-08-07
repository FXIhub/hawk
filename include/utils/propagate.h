#ifndef _PROCESS_IMAGE_H_
#define _PROCESS_IMAGE_H_

typedef struct {
  char input[1024];
  char output[1024];
  real distance;
  real wavelength;
  real pixel_size;
  real delta_z;
}Options;



Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opt);
Image * get_fresnel_propagator(Image * in, real delta_z);

#endif
