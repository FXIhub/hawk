#include <spimage.h>
#include <getopt.h>

typedef struct{
  int crop_x;
  int crop_y;
  int verbose;
  int shift;
  char input[1024];
  char output[1024];
  float scale_percent;
}Options;


void set_defaults(Options * opt){
  opt->crop_x = 0;
  opt->crop_y = 0;
  opt->verbose = 0;
  opt->shift = 0;
  opt->input[0] = 0;
  opt->output[0] = 0;
  opt->scale_percent = 100;
}

Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -i: Input file\n\
    -o: Output file\n\
    -c: Crop image around the center to size (ex: -c 1024x1024)\n\
    -s: Percentage to scale image size with (ex: -s 25 meaning 4x reduction)\n\
    -t: Shift the image before output\n\
    -v: Produce lots of output files for diagnostic\n\
    -h: print this text\n\
   \n\
    No te that the operation are applied in the same order\n\
    as they are listed above.\n\
";
  static char optstring[] = "i:o:c:s:tvh";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'i':
      strcpy(res->input,optarg);
      break;
    case 'o':
      strcpy(res->output,optarg);
      break;
    case 'c':
      res->crop_x = atof(optarg);
      res->crop_y = atof(strstr(optarg,"x")+1);
      break;
    case 's':
      res->scale_percent = atof(optarg);
      break;
    case 't':
      res->shift = 1;
    case 'v':
      res->verbose = 1;
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  return res;
}


int main(int argc, char ** argv){
  Options * opts = parse_options(argc,argv);
  if(!opts->input[0] || !opts->output[0]){
    printf("Error: input or output image unspecified!\n");
    printf("Use image_edit -h for details on how to run this program\n");
    exit(0);
  }
  Image * input = sp_image_read(opts->input,0);
  int x1,y1,x2,y2;
  Image * cropped = input;
  if (opts->crop_x != 0 && opts->crop_y != 0) {
    x1 = input->detector->image_center[0]-opts->crop_x/2;
    y1 = input->detector->image_center[1]-opts->crop_y/2;
    x2 = x1+opts->crop_x-1;
    y2 = y1+opts->crop_y-1;
    cropped = rectangle_crop(input,x1,y1,x2,y2);
  }
  Image * scaled = cropped;
  if(opts->scale_percent != 100){
    scaled = bilinear_rescale(cropped,sp_image_x(cropped)*opts->scale_percent/100.0,sp_image_y(cropped)*opts->scale_percent/100.0,1);
  }
  int output_flags = 0;
  char out_name[1000];
  
  if (strrchr(opts->output,'.') && strcmp(strrchr(opts->output,'.'),".png") == 0) {
    output_flags = SpColormapJet;
  }

  if(opts->shift) {
    Image *shifted = sp_image_shift(scaled);
    sp_image_write(shifted,opts->output,output_flags);
  } else {
    sp_image_write(scaled,opts->output,output_flags);
  }
  return 0;
}
