#include <hdf5.h>
#include <stdio.h>
#include <stdlib.h>

#if _MSC_VER
#pragma warning(disable: 4244)
#endif

int main(int argc, char ** argv){
  char buffer[1024];
  char * dataset;
  void * data;
  hid_t file_id,loc_id,group_id,dataset_id,datatype,dataspace;
  hsize_t size;
  if(argc != 4){
    printf("Usage: h5extract <full path to dataset> <infile> <outfile>\n");
    exit(0);
  }
  strcpy(buffer,argv[1]);
  file_id = H5Fopen(argv[2],  H5F_ACC_RDWR, H5P_DEFAULT );
  if(strrchr(buffer,'/')){
    *(strrchr(buffer,'/')) = 0;
    group_id = H5Gopen(file_id,buffer);
    loc_id = group_id;
    dataset = &(buffer[strlen(buffer)+1]);
  }else{
    loc_id = file_id;
    dataset = argv[1];
  }
  dataset_id = H5Dopen(loc_id,dataset);
  size = H5Dget_storage_size(dataset_id);
  data = malloc(size);
  datatype = H5Dget_type( dataset_id);
  dataspace = H5Dget_space( dataset_id);
  H5Dread(dataset_id,datatype , H5S_ALL,H5S_ALL,H5P_DEFAULT,data);
  file_id = H5Fcreate(argv[3],H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
  dataset_id = H5Dcreate(file_id,dataset,datatype,dataspace,H5P_DEFAULT);
  H5Dwrite(dataset_id,datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT,data);
  
  return 0;
}
