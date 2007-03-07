#include <stdlib.h>
#include <stdio.h>
#include <tiffio.h>
#include <hdf5.h>

int main(int argc, char ** argv){
  hid_t file_id,dataset_id,dataspace_id;
  int nstrips;
  int stripsize;
  int i,x,y;
  float * img;
  double * imgh5;
  hsize_t dims[2];
  TIFF * tif;
  if(argc != 3){
    printf("Transform an input 32 bit floating point\nTIFF file in a speden readable HDF5 file\n");
    printf("Usage: tiff2h5 <infile> <outfile> <center x> <center y> <distance\n");
    exit(0);
  }
  
  tif = TIFFOpen(argv[1], "r");
  TIFFPrintDirectory(tif,stdout,0);  
  nstrips = TIFFNumberOfStrips(tif);
  stripsize = TIFFStripSize(tif);
  img = malloc(nstrips*stripsize);
  for(i = 0;i<nstrips;i++){
    TIFFReadEncodedStrip(tif,i,&(img[i*stripsize/4]),stripsize);
  }
  TIFFClose(tif);
  /* Transpose image */
  imgh5 = malloc(nstrips*stripsize*2);
  for(x = 0;(unsigned int)x<(stripsize/sizeof(float));x++){
    for(y = 0;y<nstrips;y++){
      imgh5[x*nstrips+y] = img[y*(stripsize/sizeof(float))+x];
    }
    
  }
  /* write HDF5 */
  file_id = H5Fcreate(argv[2],H5F_ACC_TRUNC,H5P_DEFAULT,H5P_DEFAULT);
  dims[0] = stripsize/sizeof(float);
  dims[1] = nstrips;
  dataspace_id = H5Screate_simple( 2, dims, NULL );
  dataset_id = H5Dcreate(file_id,"/img",H5T_NATIVE_DOUBLE,dataspace_id,H5P_DEFAULT);
  H5Dwrite(dataset_id,H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,imgh5);
  H5Dclose(dataset_id);
  H5Sclose(dataspace_id);
  H5Fclose(file_id);


  return 0;
}
