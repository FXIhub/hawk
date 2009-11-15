#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "network_communication.h"

const char      prog_name[] = "hawk";


/* Issue error message to standard error stream and, if status value not
 * less than zero, terminate calling program.  Argument mode is intended
 * to describe error severity. */

  /* Lets try our luck with variable argument macros */ 

/*
  These functions are intended to be similar to those in libspimage sperror
*/
static void hawk_report(const char * file, int line, int status, char *mode,const char *format,va_list ap){
  char * buffer;
  int size[3] = {0,0,0};
  size[0] = snprintf(buffer,0, "%s: %s: ", prog_name, mode);
  size[1] = vsnprintf(buffer,0 , format, ap);
  size[2] = snprintf(buffer,0, " in %s:%d\n",file,line);
  buffer = malloc(sizeof(char)*size[0]+size[1]+size[2]+1);
  snprintf(buffer,size[0]+1, "%s: %s: ", prog_name, mode);
  vsnprintf(buffer+size[0],size[1]+1 , format, ap);
  snprintf(buffer+size[0]+size[1],size[2]+1, " in %s:%d\n",file,line);
  fprintf(stderr,"%s",buffer);
  if(strcmp(mode,"warning") == 0){
    rpc_send_warning_message(buffer);
  }
  if(strcmp(mode,"info") == 0){
    rpc_send_info_message(buffer);
  }
  if(strcmp(mode,"FATAL") == 0){
    rpc_send_critical_message(buffer);
  }
  free(buffer);
  /* I can't abort in network more or the critical message never gets to the server */
  if(!is_connected()){
    if (status >= 0){
      abort();
    }
   }
}

static void hawk_report2(int status,  char *mode,const char *format,va_list ap){
  char * buffer;
  int size[3] = {0,0,0};
  size[0] = snprintf(buffer,0, "%s: %s: ", prog_name, mode);
  size[1] = vsnprintf(buffer,0 , format, ap);
  size[2] = snprintf(buffer,0, "\n");
  buffer = malloc(sizeof(char)*size[0]+size[1]+size[2]+1);
  snprintf(buffer,size[0]+1, "%s: %s: ", prog_name, mode);
  vsnprintf(buffer+size[0],size[1]+1 , format, ap);
  snprintf(buffer+size[0]+size[1],size[2]+1, "\n");
    fprintf(stderr,"%s",buffer);
  rpc_send_warning_message(buffer);
  free(buffer);
  if (status >= 0){
    exit(status);
  }
}

void _hawk_warning(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file,line,-1, "warning", format,ap);
  va_end(ap);
}

void _hawk_info(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file,line,-1, "info", format,ap);
  va_end(ap);
}


void _hawk_fatal(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file, line,EXIT_FAILURE, "FATAL", format,ap);
  va_end(ap);
}


void hawk_warning(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(-1, "warning", format,ap);
  va_end(ap);
}

void hawk_info(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(-1, "info", format,ap);
  va_end(ap);
}

void hawk_log(FILE * fp,const char *format, ...){
  char * buffer;
  int size = 0;
  va_list ap;
  va_start(ap,format);
  size = vsnprintf(buffer,0 , format, ap);
  buffer = malloc(sizeof(char)*size+1);
  vsnprintf(buffer,size+1 , format, ap);
  fprintf(fp,"%s",buffer);
  rpc_send_log_line(buffer);
  free(buffer);
}

void hawk_fatal(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(EXIT_FAILURE, "FATAL", format,ap);
  va_end(ap);
}

