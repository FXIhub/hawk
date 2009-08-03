#include <getopt.h>
#include "spimage.h"
#include "analyse_mask.h"

void set_defaults(Options * opt){
  opt->pattern[0] = 0;
  opt->amplitudes[0] = 0;
  opt->output[0] = 0;
}


Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
"    Options description:\n\
    \n\
    -p: Pattern file\n\
    -a: Amplitudes file (containing the mask)\n\
    -o: Output prefix\n\
    -h: Display this text\n\
";
  static char optstring[] = "p:a:o:hi:";
  Options * res = calloc(1,sizeof(Options));
  int flag = 0;
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'p':
      strcpy(res->pattern,optarg);
      break;
    case 'a':
      strcpy(res->amplitudes,optarg);
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
  if(!res->pattern[0]){
    fprintf(stderr,"You HAVE to specify a pattern!\n");    
    flag = 1;
  }
  if(!res->amplitudes[0]){
    fprintf(stderr,"You HAVE to specify an amplitudes file!\n");    
    flag = 1;
  }
  if(flag){
    exit(1);
  }
  return res;
}


int main(int argc, char **argv)
{
  int i;
  Options * opts = malloc(sizeof(Options));
  //set_defaults(opts);
  opts = parse_options(argc,argv);

  Image *pattern = sp_image_read(opts->pattern,0);
  Image *amplitudes = sp_image_read(opts->amplitudes,0);
  const int max_i = sp_image_size(pattern);

  if(!opts->pattern[0] || !opts->amplitudes[0]){
    printf("Use %s -h for details on how to run this program\n",argv[0]);
    exit(0);
  }

  if (sp_image_x(pattern) != sp_image_x(amplitudes) ||
      sp_image_y(pattern) != sp_image_y(amplitudes)) {
    printf("Size of patterna and amplitudes doesn't match\n");
    exit(0);
  }
  
  Image *inside_mask = sp_image_alloc(sp_image_x(pattern),sp_image_y(pattern),1);
  Image *outside_mask = sp_image_alloc(sp_image_x(pattern),sp_image_y(pattern),1);
  for (i = 0; i < max_i; i++) {
    if (amplitudes->mask->data[i] > 0.0) {
      outside_mask->image->data[i] = pattern->image->data[i];
      inside_mask->image->data[i] = sp_cinit(0.0,0.0);
    } else {
      outside_mask->image->data[i] = sp_cinit(0.0,0.0);
      inside_mask->image->data[i] = pattern->image->data[i];
    }
  }

  Image *inside_fft = sp_image_fft(inside_mask);
  Image *outside_fft = sp_image_fft(outside_mask);

  char buffer[1024];

  if (opts->output[0]) {
    sprintf(buffer,"%s_inside_mask.png",opts->output);
    sp_image_write(sp_image_shift(inside_mask),buffer,SpColormapJet);
    sprintf(buffer,"%s_outside_mask.png",opts->output);
    sp_image_write(sp_image_shift(outside_mask),buffer,SpColormapJet);
    sprintf(buffer,"%s_inside_mask_real.png",opts->output);
    sp_image_write(inside_fft,buffer,SpColormapJet|SpColormapLogScale);
    sprintf(buffer,"%s_inside_mask_real_phase.png",opts->output);
    sp_image_write(inside_fft,buffer,SpColormapWheel|SpColormapWeightedPhase|SpColormapLogScale);
    sprintf(buffer,"%s_outside_mask_real.png",opts->output);
    sp_image_write(outside_fft,buffer,SpColormapJet|SpColormapLogScale);
    sprintf(buffer,"%s_outside_mask_real_phase.png",opts->output);
    sp_image_write(outside_fft,buffer,SpColormapWheel|SpColormapWeightedPhase|SpColormapLogScale);
  } else {
    sp_image_write(sp_image_shift(inside_mask),"inside_mask.png",SpColormapJet);
    sp_image_write(sp_image_shift(outside_mask),"outside_mask.png",SpColormapJet);
    sp_image_write(inside_fft,"inside_mask_real.png",SpColormapJet|SpColormapLogScale);
    sp_image_write(inside_fft,"inside_mask_real_phase.png",SpColormapWheel|SpColormapWeightedPhase|SpColormapLogScale);
    sp_image_write(outside_fft,"outside_mask_real.png",SpColormapJet|SpColormapLogScale);
    sp_image_write(outside_fft,"outside_mask_real_phase.png",SpColormapWheel|SpColormapWeightedPhase|SpColormapLogScale);
  }
  sp_image_free(inside_mask);
  sp_image_free(outside_mask);
  sp_image_free(inside_fft);
  sp_image_free(outside_fft);
}
