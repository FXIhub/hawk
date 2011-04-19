#include "spimage.h"
#include "configuration.h"
#include "output_projection.h"


Image * apply_output_projection(const Image * input, Output_Projection type,const Image * amp){
  Image * ret = NULL;
  if(type == IntensitiesProjection){
    Image * tmp = sp_image_fft(input);
    sp_proj_module(tmp,amp,SpInPlace);
    Image * tmp2 = sp_image_ifft(tmp);
    sp_image_free(tmp);
    /* and normalize */
    sp_image_scale(tmp2,1.0/sp_image_size(tmp2));
    ret = sp_image_duplicate(input,SP_COPY_ALL);
    for(int i = 0;i<sp_image_size(ret);i++){
      ret->image->data[i] = tmp2->image->data[i];
    }
    sp_image_free(tmp2);
  }else{
    ret = sp_image_duplicate(input,SP_COPY_ALL);
  }
  return ret;
}
