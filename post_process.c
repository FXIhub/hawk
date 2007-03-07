#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "spimage.h"
#include "post_process.h"


Options parse_options(int argc, char ** argv){
	int c;
  static char help_text[] = 
"    Options description:\n\
    \n\
    -i <file>: Input file or list file\n\
    -o <file>: Output file\n\
    -u <file>: Subtract file from the input file\n\
    -c <scaling factor>: Rescale image by a scaling factor\n\
    -a: Average the input files\n\
    -x: Cross correlate input files to correct for translations\n\
    -s: Calculate standard deviation of the input files\n\
    -r: Rotate input file by 180 degrees\n\
    -v: Print some statistics\n\
    -h: print this text\n\
";
  static char optstring[] = "i:o:u:c:axsvhr";
  Options res;
  set_defaults(&res);
  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'a':
      res.average = 1;
      break;
    case 'i':
      strcpy(res.input,optarg);
      break;
    case 'o':
      strcpy(res.output,optarg);
      break;
    case 'u':
      strcpy(res.subtract,optarg);
      break;
    case 'c':
      res.rescale = atof(optarg);
      break;
    case 's':
      res.std_dev = 1;
      break;
    case 'x':
      res.cross_correlate = 1;
      break;
    case 'r':
      res.rotate = 1;
      break;
    case 'v':
      res.verbose = 1;
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

void set_defaults(Options * opt){
  opt->cross_correlate = 0;
  opt->verbose = 0;
  opt->std_dev = 0;
  opt->rotate = 0;
  opt->average = 0;
  opt->rescale = 0;
  opt->input[0] = 0;
  opt->output[0] = 0;
  opt->subtract[0] = 0;
}


/* returns an array of images and sets list_size */
char ** read_filelist(char * filelist_filename,int * list_size){
  FILE * fp = fopen(filelist_filename,"r");
  char buffer[1024];
  char buffer2[1024];
  int a_s = 10;
  int n = 0;  
  char ** res;
  *list_size = 0;
  if(!filelist_filename[0]){
    return NULL;
  }
  fp = fopen(filelist_filename,"r");
  res =  malloc(sizeof(char *)*a_s);
  while(fgets(buffer,1024,fp)){
    if(n == a_s){
      a_s *= 2;
      res = realloc(res,sizeof(char *)*a_s);
    }
    sscanf(buffer,"%s",buffer2);
    res[n] = malloc(strlen(buffer2)+1);
    strcpy(res[n],buffer2);
    n++;
  }
  *list_size = n;
  return res;
  
}

Image * calculate_average(char ** img_list,int list_size,int cross_correlate){
  Image * res;
  Image * cc;
  Image * a;
  int size[2] = {sp_cmatrix_cols(a->image),sp_cmatrix_rows(a->image)};

  int i,j,trans;
  real max;
  if(!list_size){
    return NULL;
  }
  a = read_imagefile(img_list[0]);
  res = sp_image_duplicate(a,SP_COPY_DATA|SP_COPY_MASK);
  sp_image_free(a);
  fprintf(stderr,"Images processed - ");
  for(i = 1;i<list_size;i++){
    a = read_imagefile(img_list[i]);
    if(cross_correlate){
      cc = sp_image_cross_correlate(res, a, size);
      max = creal(sp_cmatrix_max(cc->image,&trans));
/*      trans /= 2;*/
      sp_image_free(cc);
    }else{
      trans = 0;
    }
    for(j = 0;j<sp_cmatrix_size(a->image);j++){
      res->image->data[j] += a->image->data[(sp_cmatrix_size(a->image)+j-trans)%(sp_cmatrix_size(a->image))];
    }
    sp_image_free(a);
  fprintf(stderr,".");
  }
  for(j = 0;j<sp_cmatrix_size(res->image);j++){
    res->image->data[j] /= list_size;
  }
  return res;    
}

int main(int argc, char ** argv){
  Options  opts; 
  char ** img_list;
  Image * tmp, *tmp2,*cc;
  int list_size;
  int i;
  real max;
  int trans;
  int temp_trans;
  opts = parse_options(argc,argv);
  if(opts.average){
    img_list = read_filelist(opts.input,&list_size);
    tmp = calculate_average(img_list,list_size,opts.cross_correlate);
    sp_image_write(tmp,opts.output,sizeof(real));
  }else if(opts.rotate){
    tmp = read_imagefile(opts.input);
    sp_image_reflect(tmp,1,SP_AXIS_XY);
    sp_image_write(tmp,opts.output,sizeof(real));    
  }else if(opts.subtract[0]){
    tmp = read_imagefile(opts.input);
    tmp2 = read_imagefile(opts.subtract);
    trans = 0;
    if(opts.cross_correlate){
      cc = sp_image_cross_correlate(tmp, tmp2,NULL);
      max = creal(sp_cmatrix_max(cc->image,&trans));
      sp_image_free(cc);
      max = creal(sp_cmatrix_max(tmp->image,&trans));
      max = creal(sp_cmatrix_max(tmp2->image,&temp_trans));
      trans -= temp_trans;
    }
    for(i = 0;i<sp_cmatrix_size(tmp->image);i++){ 
      tmp->image->data[i] -= tmp2->image->data[(sp_cmatrix_size(tmp->image)+i-trans)%(sp_cmatrix_size(tmp->image))];
    }
    sp_image_write(tmp,opts.output,sizeof(real));    
  }else if(opts.rescale){
    tmp = read_imagefile(opts.input);
    tmp2 = bilinear_rescale(tmp, sp_cmatrix_cols(tmp->image)*opts.rescale, sp_cmatrix_rows(tmp->image)*opts.rescale);
    sp_image_write(tmp2,opts.output,sizeof(real));        
  }
  return 0;
}
