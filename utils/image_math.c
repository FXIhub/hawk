#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <matheval.h>
#include <ctype.h>
#include "spimage.h"


static char * sp_strdup(char * str){
  char * p = malloc(sizeof(char)*(strlen(str)+1));
  strcpy(p,str);
  return p;
}

typedef struct {
  char * output_filename;
  char ** input_filenames;
  int ninput_filenames;
  char * expression;
  void * evaluator;
  char ** variables;
  int nvariables;
  Image ** input_images;
  Image * output_image;
  int verbose;
}Options;

static void set_defaults(Options * opt);


#define CHECK(cond, msg) { if (!(cond)) { fprintf(stderr, "image_math error: %s\n", msg); exit(EXIT_FAILURE); } }

Options * parse_options(int argc, char ** argv){
  int c;
  static char help_text[] = 
    "    Options description:\n\
    \n\
    -e: Expression to be applied to input\n\
    -o: Output file\n\
    -v: Verbose\n\
    -h: Print help text\n\
    \n\
    In the expression (specified by -e), the first input dataset\n\
    (from left to right) is  referred to as d1, the second as d2,\n\
    and so on.\n\
    Expressions use a C-like  infix  notation,  with  most  standard\n\
    operators  and  mathematical functions (+, sin, etc.) being sup‐\n\
    ported.  This functionality is provided (and its features deter‐\n\
    mined) by GNU libmatheval.\n\
";
  static char optstring[] = "e:o:hv";
  Options * res = calloc(1,sizeof(Options));
  set_defaults(res);

  while(1){
    c = getopt(argc,argv,optstring);
    if(c == -1){
      break;
    }
    switch(c){
    case 'e':
      free(res->expression);
      res->expression = sp_strdup(optarg);
      break;
    case 'o':
      free(res->output_filename);
      res->output_filename = sp_strdup(optarg);
      break;
    case 'h':
      printf("%s",help_text);
      exit(0);
      break;
    case 'v':
      res->verbose = 1;
      break;
    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
    }
  }
  free(res->input_filenames);
  res->ninput_filenames = (argc-optind);
  res->input_filenames = malloc(sizeof(char *)*(argc-optind));
  for(int i = optind;i<argc;i++){
    res->input_filenames[i-optind] = sp_strdup(argv[i]);
  }
  return res;
}

void set_defaults(Options * opt){

  opt->output_filename = 0;
  opt->input_filenames = 0;
  opt->ninput_filenames = 0;
  opt->expression = 0;
  opt->evaluator = 0;
  opt->nvariables = 0;
  opt->variables = 0;
  opt->verbose = 0;
}

int main(int argc, char ** argv){
  int i,j;
  int flag;
  Options * opts;
  double * vals;
  if(argc < 2){
    printf("Try image_math -h for help\n");
    return 0;
  }
  opts = malloc(sizeof(Options));
  set_defaults(opts);
  opts = parse_options(argc,argv);

  for (i = 0; i < strlen(opts->expression); ++i){
    if (opts->expression[i] == '\n'){
      opts->expression[i] = ' '; /* matheval chokes on newlines */
    }
  }

  CHECK(opts->evaluator = evaluator_create(opts->expression),"error parsing symbolic expression");
  evaluator_get_variables(opts->evaluator, &opts->variables, &opts->nvariables);

  /* Check variable names */
  flag = 0;
  for(i = 0;i<opts->nvariables;i++){
    if(opts->variables[i][0] != 'd'){
      flag = i;
      break;
    }
    if(strlen(opts->variables[i]) < 2){
      flag = i;
      break;
    }
    for(j = 1;j<strlen(opts->variables[i]);j++){
      if(!isdigit(opts->variables[i][j])){
      flag = i;
      break;
      }
    }
    if(flag){
      break;
    }
  }
  if(flag){
    fprintf(stderr, "image_math error: unrecognized variable \"%s\"\n",
	    opts->variables[flag]);
    exit(EXIT_FAILURE); 
  }

  opts->input_images = malloc(sizeof(Image  *)*opts->ninput_filenames);
  /* Read Input files */
  for(i = 0;i<opts->ninput_filenames;i++){
    opts->input_images[i] = sp_image_read(opts->input_filenames[i],0);
  }
  /* Create output file */
  opts->output_image = sp_image_duplicate(opts->input_images[0],0);
  
  if (opts->verbose) {
    char *buf = evaluator_get_string(opts->evaluator);
    printf("Evaluating expression: %s\n", buf);
  }
  vals = malloc(sizeof(double)*opts->nvariables);
  for(i = 0;i<sp_image_size(opts->input_images[0]);i++){
    for(j = 0;j<opts->ninput_filenames;j++){
      vals[j] = sp_real(opts->input_images[j]->image->data[i]);
    }
    sp_real(opts->output_image->image->data[i]) = evaluator_evaluate(opts->evaluator, opts->ninput_filenames, opts->variables, vals);
    for(j = 0;j<opts->ninput_filenames;j++){
      vals[j] = sp_imag(opts->input_images[j]->image->data[i]);
    }
    sp_imag(opts->output_image->image->data[i]) = evaluator_evaluate(opts->evaluator, opts->ninput_filenames, opts->variables, vals);
  }
  sp_image_write(opts->output_image,opts->output_filename,0);
  return 0;
}
