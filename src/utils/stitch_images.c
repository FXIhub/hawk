#include <spimage.h>
#include "geometry_constraints.h"

typedef struct{
  char id[1024];
  real dx;
  real dy;
  real dz;
  real theta;  
  affine_transform * t;
  affine_transform * t_inv;
}ImageGeometry;

ImageGeometry * read_geometry_file(char * filename, int * n_images){
  FILE * fp = fopen(filename,"r");
  const int max_line = 1024;
  char line[max_line];
  fgets(line,max_line,fp);
  sscanf(line,"Total Images: %d",n_images);
  ImageGeometry * ret = malloc(sizeof(ImageGeometry)*(*n_images));
  for(int i = 0;i<*n_images;i++){
    fgets(line,max_line,fp);
    sscanf(line,"Identifier: %s",ret[i].id);
    fgets(line,max_line,fp);
    sscanf(line,"dx: %f",&ret[i].dx);
    fgets(line,max_line,fp);
    sscanf(line,"dy: %f",&ret[i].dy);
    fgets(line,max_line,fp);
    sscanf(line,"dz: %f",&ret[i].dz);
    fgets(line,max_line,fp);
    sscanf(line,"theta: %f",&ret[i].theta);
  }
  fclose(fp);
  return ret;

}


void check_geometry(ImageGeometry * geom, int n_images){
  /* check that dz is the same for all images as we don't
     support different dz at the moment */
  real dz = geom[0].dz;
  for(int i = 0;i<n_images;i++){
    if(geom[i].dz != dz){
      fprintf(stderr,"Can't stitch images with different dz. image0.dz = %g image%d.dz = %g\n",dz,i,geom[i].dz);
      abort();
    }
  }
}

void find_corners(ImageGeometry * geom, Image ** images, int n_images, int * x0, int *y0, int * x1, int *y1){
  //  *x0 = geom
  sp_vector * p = sp_vector_alloc(2);
  sp_vector_set(p,0,0);
  sp_vector_set(p,1,0);

  sp_vector * Tp = apply_affine_transform(geom[0].t,p);
  printf("%g x %g\n",sp_vector_get(Tp,0),sp_vector_get(Tp,1));
  *x0 = sp_vector_get(Tp,0);
  *y0 = sp_vector_get(Tp,1);
  *x1 = sp_vector_get(Tp,0);
  *y1 = sp_vector_get(Tp,1);
  sp_vector_free(Tp);

  for(int i = 0;i<n_images;i++){
    for(int x = 0; x<sp_image_x(images[i]);x+=sp_image_x(images[i])-1){
      for(int y = 0; y<sp_image_y(images[i]);y+=sp_image_y(images[i])-1){
	sp_vector_set(p,0,x);
	sp_vector_set(p,1,y);
	Tp = apply_affine_transform(geom[i].t,p);
	if(floor(sp_vector_get(Tp,0)) < *x0){
	  *x0 = floor(sp_vector_get(Tp,0));
	}
	if(floor(sp_vector_get(Tp,1)) < *y0){
	  *y0 = floor(sp_vector_get(Tp,1));
	}
	if(ceil(sp_vector_get(Tp,0)) > *x1){
	  *x1 = ceil(sp_vector_get(Tp,0));
	}
	if(ceil(sp_vector_get(Tp,1)) > *y1){
	  *y1 = ceil(sp_vector_get(Tp,1));
	}
	printf("%g x %g\n",sp_vector_get(Tp,0),sp_vector_get(Tp,1));
	sp_vector_free(Tp);
      }
    }
  }
  printf("corners at %dx%d and %dx%d\n",*x0,*y0,*x1,*y1);
}

Image * stitch_images(ImageGeometry * geom, Image ** images, int n_images){
  int x0,y0;
  int x1,y1;
  Image * ret = NULL;
  /* calculate affine transforms */
  for(int i = 0;i<n_images;i++){
    geom[i].t = affine_transfrom_from_parameters(sp_image_x(images[i]),sp_image_y(images[i]),geom[i].dx,geom[i].dy,1.0f,0.0,geom[i].theta);
    geom[i].t_inv = affine_transfrom_from_parameters(sp_image_x(images[i]),sp_image_y(images[i]),geom[i].dx,geom[i].dy,1.0f,0.0,geom[i].theta);
    affine_transform_invert(geom[i].t_inv);
  }
  find_corners(geom,images,n_images,&x0,&y0,&x1,&y1);
  ret = sp_image_alloc(x1-x0+1,y1-y0+1,1);
  sp_vector * pos = sp_vector_alloc(2);
  for(int x = x0;x<=x1;x++){
    for(int y = y0;y<=y1;y++){
      sp_vector_set(pos,0,x);
      sp_vector_set(pos,1,y);
      real v = 0;
      int mask = 0;
      sp_image_mask_set(ret,x-x0,y-y0,0,0);
      for(int i = 0;i<n_images;i++){
	sp_vector * t_pos = apply_affine_transform(geom[i].t_inv,pos);
	//	sp_vector * t_pos = apply_affine_transform_inverse(geom[i].t,pos);
	real rx = sp_vector_get(t_pos,0);
	real ry = sp_vector_get(t_pos,1);
	if(sp_image_contains_coordinates(images[i],rx,ry,0)){
	  v = sp_image_interp(images[i],rx,ry,0);
	  mask = 1;
	  break;
	  //	  printf("%f x %f inside image %d\n",rx,ry,i);
	}
      }
      sp_image_set(ret,x-x0,y-y0,0,sp_cinit(v,0));
      sp_image_mask_set(ret,x-x0,y-y0,0,mask);
    }
  }
  ret->detector->image_center[0] = -(x0-1);
  ret->detector->image_center[1] = -(y0-1);
  return ret;
}

int main(int argc, char ** argv){
  if(argc < 5){
    printf("Usage: %s <geometry file> <output image> <imageA> <imageB> ...\n",argv[0]);
    exit(1);
  }
  int n_images;
  ImageGeometry * geom = read_geometry_file(argv[1],&n_images);

  if(n_images != argc-3){
    fprintf(stderr,"Number of input images doesn't match geometry specification!\n");
    abort();
  }
  check_geometry(geom,n_images);

  Image ** images = malloc(sizeof(Image*)*n_images);
  for(int i = 3;i<argc;i++){
    images[i-3] = sp_image_read(argv[i],0);
  }

  Image * stitched = stitch_images(geom,images,n_images);
  sp_image_write(stitched,argv[2],0);
  return 0;
}
