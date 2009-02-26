#ifndef _TIFF2H5_H_
#define _TIFF2H5_H_

typedef struct {
  double x_center;
  double y_center;
  char input[1024];
  char output[1024];
  double detector_distance;
  double pixel_size;
  double lambda;
}Options;


Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opt);

#endif
