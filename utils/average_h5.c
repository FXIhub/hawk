#include <stdlib.h>
#include <stdio.h>
#include <hdf5.h>
#include "spimage.h"


int main(int argc, char ** argv){
  int i,n;
  Image * res = NULL;
  Image * cur;
  for(n = 1;n<argc;n++){
    cur =  read_imagefile(argv[n]);
    if(n == 1){
      res = imgcpy(cur);
    }else{
      add_image(res,cur);
    }
    freeimg(cur);    
  }
  for(i = 0;i<sp_cmatrix_size(res->image);i++){
    res->image->data[i] /= (n-1);
  }
  write_img(res,"average.h5",sizeof(real));
  return 0;
}
