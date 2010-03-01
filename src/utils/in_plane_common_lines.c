#include <spimage.h>

Image * rotate_image(const Image * a, const SpRotation * rot){
  sp_vector * pos = sp_vector_alloc(3);
  Image * ret = sp_image_duplicate(a,SP_COPY_DETECTOR);
  sp_image_fill(ret,sp_cinit(0,0));
  for(int z = 0;z<sp_image_z(a);z++){
    sp_vector_set(pos,2,z-a->detector->image_center[2]);
    for(int y = 0;y<sp_image_y(a);y++){
      sp_vector_set(pos,1,y-a->detector->image_center[1]);
      for(int x = 0;x<sp_image_x(a);x++){
	sp_vector_set(pos,0,x-a->detector->image_center[0]);
	sp_vector * rot_pos = sp_matrix_vector_prod(rot,pos);
	float new_x = sp_vector_get(rot_pos,0)+a->detector->image_center[0];
	if(new_x < 0 || new_x >= sp_image_x(ret)){
	  continue;
	}
	float new_y = sp_vector_get(rot_pos,1)+a->detector->image_center[1];
	if(new_y < 0 || new_y >= sp_image_y(ret)){
	  continue;
	}
	float new_z = sp_vector_get(rot_pos,2)+a->detector->image_center[2];
	if(new_z < 0 || new_z >= sp_image_z(ret)){
	  continue;
	}
	sp_image_set(ret,x,y,z,sp_cinit(sp_image_interp(a,new_x,new_y,new_z),0));
	sp_vector_free(rot_pos);
      }
    }    
  }
  sp_vector_free(pos);
  return ret;
}

int main(int argc, char ** argv){
  if(argc < 3){
    printf("Usage: %s <input image> <steps>\n",argv[0]);
    exit(1);
  }
  const Image * a = sp_image_read(argv[1],0);
  const int steps = atoi(argv[2]);
  for(int i = 0;i<steps;i++){
  /* We have to use the inverse rotation because we need to transform gridded coordinates from
     the output image to floating point coordinates in the input image which we can then use
     to interpolate and get a nice value */
    SpRotation * rot = sp_rot_euler(-i*2.0*M_PI/steps,0,0);
    Image * rot_a = rotate_image(a,rot);
    char buffer[1024];
    sp_rot_free(rot);
    sp_image_sub(rot_a,a);
    sprintf(buffer,"%s-%06d.h5",argv[1],i);
    sp_image_write(rot_a,buffer,0);        
    sprintf(buffer,"%s-%06d.png",argv[1],i);
    sp_image_write(rot_a,buffer,SpColormapJet);        
    sp_image_free(rot_a);

  }
  sp_image_free(a);
}
