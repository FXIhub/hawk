#include <hdf5.h>
#include <spimage.h>

int main(int argc, char ** argv){
  Image * img;
  Image * tmp;
  char filename[1024];
  int i;
  if(argc < 2){
    fprintf(stderr,"Usage: %s <hdf5 file> [vtk file] [vtk phase file] [mask vtk file]\n",argv[0]);
  }
  img = read_imagefile(argv[1]);
  if(argc == 2){
    strcpy(filename,argv[1]);
    strcat(filename,".vtk");
  }else{
    strcpy(filename,argv[2]);
  }
  if(img->shifted){
    tmp = sp_image_shift(img);
    sp_image_free(img);
    img = tmp;
  }
  write_vtk(img,filename);
  if(argc > 3){
    tmp = sp_image_get_phases(img);
    write_vtk(tmp,argv[3]);
    sp_image_free(tmp);
  }
  if(argc > 4){
    for(i = 0;i<sp_cmatrix_size(img->image);i++){
      img->image->data[i] = img->mask->data[i];
    }
    write_vtk(img,argv[4]);
  }
  return 0;
}

