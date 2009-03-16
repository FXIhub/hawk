#ifndef _ANALYSE_MASK_H_
#define _ANALYSE_MASK_H_

typedef struct {
  char pattern[1024];
  char amplitudes[1024];
  char output[1024];
}Options;


Options * parse_options(int argc, char ** argv);
void set_defaults(Options * opt);

#endif
