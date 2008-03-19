#ifndef _PROCESS_IMAGE_H_
#define _PROCESS_IMAGE_H_

typedef struct {
  real saturation;
  real beamstop;
  real resolution;
  real background;
  real oversampling_factor;
  real cross_removal;
  int centrosymetry;
  int pad;
  int verbose;
  int shift_quadrants;
  char input[1024];
  char output[1024];
  char dark[1024];
  char mask[1024];
  char noise[1024];
  real user_center_x;
  real user_center_y;
  real high_pass_radius;
  real high_pass_transition_smoothness;
}Options;



Image * imgcpy(Image * in);
real dist_to_corner(int i, Image * in);
real dist_to_center(int i, Image * in);
real dist_to_cross(int i, Image * in);
/*Image * read_imagefile(char * filename);*/
Image *  average_centrosymetry(Image * in);
Image * make_image_square(Image * in);
void mask_overexposure(Image * img, real saturation);
void remove_background(Image * img,Options * opts);
void write_image(char * filename, Image * img);
void intensity_to_amplitudes(Image * img);
Image * limit_resolution(Image * img, int resolution);
Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opt);
Image * limit_sampling(Image * img, real oversampling_factor, real cutoff);
void subtract_dark(Image * img, Image * dark);
#endif
