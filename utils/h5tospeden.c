#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hdf5.h>
#include "spimage.h"
#include "h5tospeden.h"

/* All the *2 here are caused by the fact that i'm taking a 2D picture and putting
 into a 3D grid with z = 2
*/ 
#define Z 2
void write_speden(Image * img, char * filename){
  hid_t dataspace_id;
  hid_t dataset_id;
  hid_t file_id;
  hid_t group_id;
  int status;
  hsize_t  dims[1];
  float values[2];
  float * data = malloc(sizeof(float)*TSIZE(img));
  int i,j,x,y;
  double dx,dy,r;
  file_id = H5Fcreate(filename,  H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  group_id = H5Gcreate(file_id, "/fobs", 0);
  dims[0] = 1;
  values[0] = TSIZE(img)*Z;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  dataset_id = H5Dcreate(file_id, "/fobs/Nfob", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, values);
  status = H5Dclose(dataset_id);

  for(i = 0;i<TSIZE(img);i++){
    for(j = 0;j<Z;j++){
      data[i+j*TSIZE(img)] = img->image[i];
    }
  }
  dims[0] = TSIZE(img)*Z;
  dataspace_id = H5Screate_simple( 1, dims, NULL );
  dataset_id = H5Dcreate(file_id, "/fobs/amp", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, data);
  status = H5Dclose(dataset_id);

  for(i = 0;i<TSIZE(img);i++){
    for(j = 0;j<Z;j++){
      data[i+j*TSIZE(img)] = sqrt(fabs(img->image[i]));
    }
  }
  dataset_id = H5Dcreate(file_id, "/fobs/sig", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, data);
  status = H5Dclose(dataset_id);

  for(i = 0;i<TSIZE(img);i++){
    for(j = 0;j<Z;j++){
      data[i+j*TSIZE(img)] = j;
    }
  }
  dataset_id = H5Dcreate(file_id, "/fobs/l", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, data);
  status = H5Dclose(dataset_id);

  i = 0;
  if(img->detector->detector_distance != 0){
    printf("Converting spherically projected image...\n");
    for(x = 0;x<img->detector->size[0];x++){
      for(y = 0;y<img->detector->size[1];y++){
	dx = (x-img->detector->image_center[0])*img->detector->pixel_size;
	dy = (y-img->detector->image_center[1])*img->detector->pixel_size;            
	r = sqrt(dx*dx+dy*dy+img->detector->detector_distance*img->detector->detector_distance);
	for(j = 0;j<Z;j++){
	  data[i+j*TSIZE(img)] = 1000*dx/(img->detector->lambda*r);
	}
	i++;
      }
    }
  }else{
    printf("Converting flat image...\n");
    for(x = 0;x<img->detector->size[0];x++){
      for(y = 0;y<img->detector->size[1];y++){
	dx = (x-img->detector->image_center[0])*img->detector->pixel_size;            
	for(j = 0;j<Z;j++){
	  data[i+j*TSIZE(img)] = 1000*dx/(img->detector->lambda*img->detector->detector_distance);
	}
	i++;
      }
    }
  }

  dataset_id = H5Dcreate(file_id, "/fobs/h", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, data);
  status = H5Dclose(dataset_id);

  i = 0;
  if(img->detector->detector_distance != 0){
    for(x = 0;x<img->detector->size[0];x++){
      for(y = 0;y<img->detector->size[1];y++){
	dx = (x-img->detector->image_center[0])*img->detector->pixel_size;
	dy = (y-img->detector->image_center[1])*img->detector->pixel_size;            
	r = sqrt(dx*dx+dy*dy+img->detector->detector_distance*img->detector->detector_distance);
	for(j = 0;j<Z;j++){
	  data[i+j*TSIZE(img)] = 1000*dy/(img->detector->lambda*r);
	}
	i++;
      }
    }
  }else{
    for(x = 0;x<img->detector->size[0];x++){
      for(y = 0;y<img->detector->size[1];y++){
	dy = (y-img->detector->image_center[1])*img->detector->pixel_size;            
	for(j = 0;j<Z;j++){
	  data[i+j*TSIZE(img)] = 1000*dy/(img->detector->lambda*img->detector->detector_distance);
	}
	i++;
      }
    }
  }

  dataset_id = H5Dcreate(file_id, "/fobs/k", H5T_NATIVE_FLOAT,
			 dataspace_id, H5P_DEFAULT);
  status = H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, data);
  status = H5Dclose(dataset_id);
  status = H5Sclose(dataspace_id);


  status = H5Fclose(file_id);

}

int main(int argc, char ** argv){
  Image * in;
  if(argc != 3){
    printf("Converts Image hdf5 files into speden hdf5 files\n");
    printf("Usage: h5tospeden <input file> <output file>\n");
    exit(0);
  }
  in = read_imagefile(argv[1]);
  write_speden(in,argv[2]);
  return 0;
}
