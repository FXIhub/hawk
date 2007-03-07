#include <stdlib.h>
#include <hdf5.h>
#include <spimage.h>

int main(int argc, char ** argv){
  char buffer[1024];
  int file_id,dataset_id,space;
  hsize_t dims[2];
  unsigned char * img_i;
  int i;
  int status;
  Image * image;
  if(argc != 3 && argc != 4 ){
    printf("gif2h5image <file.gif> <file.h5> [-u]\n");
    printf("The optional -u means unscalled data.\n");
    exit(0);
  }
  /* convert gif to h5*/
  sprintf(buffer,"gif2h5 %s %s",argv[1],argv[2]);
  system(buffer);
  /* extract the info from the new h5 */
  file_id = H5Fopen(argv[2],H5F_ACC_RDWR,H5P_DEFAULT);
  sprintf(buffer,"/%s/Image0",argv[1]);
  dataset_id = H5Dopen(file_id, buffer);
  space = H5Dget_space(dataset_id);
  H5Sget_simple_extent_dims(space,dims,NULL);
  image->image = sp_cmatrix_alloc(dims[1],dims[0]);
  img_i = malloc(sizeof(unsigned char)*dims[0]*dims[1]);
  status = H5Dread(dataset_id, H5T_STD_U8LE, H5S_ALL, H5S_ALL,
		   H5P_DEFAULT, img_i);
  H5Dclose(dataset_id);
  H5Fclose(file_id);
  image = malloc(sizeof(Image));
  image->detector = malloc(sizeof(Detector));
  for(i = 0;(unsigned int)i<dims[0]*dims[1];i++){
    image->image->data[i] = img_i[i];
  }
  image->mask = sp_imatrix_alloc(dims[1],dims[0]);
  for(i= 0;i<sp_cmatrix_size(image->image);i++){
    image->mask->data[i] =1;
  }
  if(argc == 4){
    image->scaled = 0;
  }else{
    image->scaled = 1;  
  }
  image->phased = 0;  
  sp_image_write(image,argv[2],sizeof(real));
  free(img_i);
  return status;
}
