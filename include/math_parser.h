#ifndef _MATH_PARSER_H_
#define _MATH_PARSER_H_ 1

#include <spimage.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum{MathOutputError,MathOutputScalar,MathOutputImage}MathOutputType;

typedef struct{
  MathOutputType type;
  Image * image;
  Complex scalar;
  char * error_msg;
}Math_Output;

Math_Output * evaluate_math_expression(char * expression,Image ** image_list);

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif
