#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <spimage.h>
/* This macro prevent the macros from the
   header file from screwing up our function definitions */
#define _IO_UTILS_NO_MACRO_SUBSTITUTIONS_

#include "network_communication.h"
#include "io_utils.h"
#include "configuration.h"
const char      prog_name[] = "hawk";


/* Issue error message to standard error stream and, if status value not
 * less than zero, terminate calling program.  Argument mode is intended
 * to describe error severity. */

  /* Lets try our luck with variable argument macros */ 

/*
  These functions are intended to be similar to those in libspimage sperror
*/
static void hawk_report(const char * file, int line, int status, MessageType type, const char *format,va_list ap){
  char * buffer = NULL;
  int size[3] = {0,0,0};
  size[0] = snprintf(buffer,0, "%s: ", prog_name);
  va_list ap_copy;
  va_copy(ap_copy,ap);
  size[1] = vsnprintf(buffer,0 , format, ap_copy);
  va_end(ap_copy); 
  size[2] = snprintf(buffer,0, " in %s:%d\n",file,line);
  buffer = malloc(sizeof(char)*size[0]+size[1]+size[2]+1);
  snprintf(buffer,size[0]+1, "%s: ", prog_name);
  vsnprintf(buffer+size[0],size[1]+1 , format, ap);
  snprintf(buffer+size[0]+size[1],size[2]+1, " in %s:%d\n",file,line);
  fprintf(stderr,"%s",buffer);
#ifdef NETWORK_SUPPORT
  rpc_send_message(type,buffer);
#endif
  free(buffer);  
  /* I can't abort in network more or the critical message never gets to the server */
  if(!is_connected()){
    if (status >= 0){
      /*
	Abort while providing interesting core dump for developers
	can produce some scary looking messages for the users, so 
	we'll use the more mild exit
	abort();
       */
      exit(1);
    }
   }
}

static void hawk_report2(int status, MessageType type,const char *format,va_list ap){
  char * buffer = NULL;
  int size[3] = {0,0,0};
  size[0] = snprintf(buffer,0, "%s: ", prog_name);
  va_list ap_copy;
  va_copy(ap_copy,ap);
  size[1] = vsnprintf(buffer,0 , format, ap_copy);
  va_end(ap_copy); 
  size[2] = snprintf(buffer,0, "\n");
  buffer = malloc(sizeof(char)*size[0]+size[1]+size[2]+1);
  snprintf(buffer,size[0]+1, "%s: ", prog_name);
  vsnprintf(buffer+size[0],size[1]+1 , format, ap);
  snprintf(buffer+size[0]+size[1],size[2]+1, "\n");
  fprintf(stderr,"%s",buffer);
#ifdef NETWORK_SUPPORT
  rpc_send_message(type,buffer);
#endif
  free(buffer);
  if (status >= 0){
    exit(status);
  }
}

void _hawk_warning(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file,line,-1, WarningMessage, format,ap);
  va_end(ap);
}

void _hawk_info(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file,line,-1, InformationMessage, format,ap);
  va_end(ap);
}


void _hawk_fatal(const char * file, int line,const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report(file, line,EXIT_FAILURE, CriticalMessage, format,ap);
  va_end(ap);
}


void hawk_warning(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(-1, WarningMessage, format,ap);
  va_end(ap);
}

void hawk_info(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(-1, InformationMessage, format,ap);
  va_end(ap);
}

void hawk_log(FILE * fp,const char *format, ...){
  char * buffer = NULL;
  int size = 0;
  va_list ap;
  va_start(ap,format);
  size = vsnprintf(buffer,0 , format, ap);
  va_end(ap); 
  va_start(ap, format);
  buffer = malloc(sizeof(char)*(size+1));
  vsnprintf(buffer,size+1 , format, ap);
  va_end(ap); 
  fprintf(fp,"%s",buffer);
#ifdef NETWORK_SUPPORT
  rpc_send_log_line(buffer);
#endif
  free(buffer);
}


void hawk_image_write(const Image * img, const char * filename, long long flags){
  /* save images if we're not connected to a server
     of if we have explicitly said to save them */
  if(!is_connected() || global_options.save_remote_files){
    sp_image_write(img,filename,flags);
  }
#ifdef NETWORK_SUPPORT
  rpc_send_image_output(filename,img);
#endif
}

void hawk_fatal(const char *format, ...){
  va_list ap;
  va_start(ap,format);
  hawk_report2(EXIT_FAILURE, CriticalMessage, format,ap);
  va_end(ap);
}

