#ifndef _OUTPUT_PROJECTION_H_
#define _OUTPUT_PROJECTION_H_ 1

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

Image * apply_output_projection(const Image * input, Output_Projection type,
				const Image * amp);

#ifdef __cplusplus
}  /* extern "C" */
#endif /* __cplusplus */

#endif
