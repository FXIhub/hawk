#ifndef _IO_UTILS_H_
#define _IO_UTILS_H_ 1

#include <stdio.h>
#include <spimage.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum{DebugMessage=1,InformationMessage,WarningMessage,CriticalMessage}MessageType;

  void hawk_log(FILE * fp, const char *format, ...);
  void hawk_image_write(const Image * img, const char * filename, long long flags);
#if __STDC_VERSION__ >= 199901L
  /* Lets try our luck with variable argument macros */ 
#ifndef  _IO_UTILS_NO_MACRO_SUBSTITUTIONS_
#define  hawk_info(...) _hawk_warning(__FILE__,__LINE__,__VA_ARGS__)
#define  hawk_warning(...) _hawk_warning(__FILE__,__LINE__,__VA_ARGS__)
#define  hawk_fatal(...) _hawk_fatal(__FILE__,__LINE__,__VA_ARGS__)
#endif

   void _hawk_info(const char * file, int line,
					const char *format, ...);
   void _hawk_warning(const char * file, int line,
					const char *format, ...);
   void _hawk_fatal(const char * file, int line,
				      const char *format, ...);

#else
   void hawk_info(const char *format, ...);
   void hawk_warning(const char *format, ...);
   void hawk_fatal(const char *format, ...); 
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

  
#endif /* _IO_UTILS_H_ */ 
