#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
#include "spimage.h"


int main(int argc, char ** argv){
  int i,n;
  Image * res = NULL;
  Image * cur;
  for(n = 1;n<argc;n++){
    cur =  sp_image_read(argv[n],0);
    if(n == 1){
      res = sp_image_duplicate(cur,SP_COPY_DATA|SP_COPY_MASK);
    }else{
      sp_image_add(res,cur);
    }
    sp_image_free(cur);    
  }
  for(i = 0;i<sp_image_size(res);i++){
    sp_real(res->image->data[i]) /= (n-1);
    sp_imag(res->image->data[i]) /= (n-1);
  }
  sp_image_write(res,"average.h5",sizeof(real));
  return 0;
}
