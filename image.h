#ifndef _IMAGE_H_
#define _IMAGE_H_

typedef struct{
  int size[2];
  double imageCenter[2];
}Detector;


typedef struct{
  double * img;
  double * complex_img;
  double * mask;
  Detector * detector;
}Image;

#endif
