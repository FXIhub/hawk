#include <spimage.h>




static Image * get_physical_image(Image * input,int binning, int side_band_size){
  const int real_size[2] = {2048,4096};
  Image * output = sp_image_alloc(real_size[0]/binning,real_size[1]/binning,1); 
  for(int x = 0;x<sp_image_x(output)/2;x++){
    for(int y = 0;y<sp_image_y(output);y++){
      sp_image_set(output,x,y,0,sp_image_get(input,side_band_size/binning+x,y,0));
      sp_image_mask_set(output,x,y,0,1);
    }
  }
  for(int x = 0;x<sp_image_x(output)/2;x++){
    for(int y = 0;y<sp_image_y(output);y++){
      sp_image_set(output,sp_image_x(output)-1-x,y,0,sp_image_get(input,sp_image_x(input)-1-side_band_size/binning-x,y,0));
      sp_image_mask_set(output,sp_image_x(output)-1-x,y,0,1);
    }
  }
  return output;
}


static void remove_electronic_vertical_noise(Image * input,int binning){
  /* just average the top and bottom 400 pixels and use that for background */
  real * noise_line = malloc(sizeof(real)*sp_image_y(input));
  int * pixels_per_col = malloc(sizeof(int)*sp_image_y(input));
  int strip_size = 400/binning;
  for(int x = 0;x<sp_image_x(input);x++){
    noise_line[x] = 0;
    pixels_per_col[x] = 0;
  }
  for(int x = 0;x<sp_image_x(input);x++){
    for(int y = 4;y<strip_size;y++){
      pixels_per_col[x]++;
      noise_line[x]+= sp_real(sp_image_get(input,x,y,0));
    }
    for(int y = sp_image_size(input)-strip_size-1;y<sp_image_y(input);y++){
      pixels_per_col[x]++;
      noise_line[x]+= sp_real(sp_image_get(input,x,y,0));
    }
  }
  for(int x = 0;x<sp_image_x(input);x++){
    noise_line[x] /= pixels_per_col[x];    
    for(int y = 0;y<sp_image_y(input);y++){
      Complex v = sp_image_get(input,x,y,0);
      sp_real(v) -=  noise_line[x];
      sp_image_set(input,x,y,0,v);
    }
  }
}

static void remove_electronic_horizontal_noise(Image * output,Image * input,int binning,int side_band_size){
  real * noise_column = malloc(sizeof(real)*sp_image_y(output));
  int * pixels_per_line = malloc(sizeof(int)*sp_image_y(output));
  for(int y = 0;y<sp_image_y(output);y++){
    noise_column[y] = 0;
    pixels_per_line[y] = 0;
  }
  /* average the noise along the side_band */
  
  /* We'll start with the left side band */
  /* skip the first and last few pixels */
  for(int x = 2;x<side_band_size/binning-2;x++){
    for(int y = 0;y<sp_image_y(output);y++){
      pixels_per_line[y]++;
      noise_column[y]+= sp_real(sp_image_get(input,x,y,0));
    }
  }
  for(int y = 0;y<sp_image_y(output);y++){
    noise_column[y] /= pixels_per_line[y];
    for(int x = 0;x<sp_image_x(output)/2;x++){
      Complex v = sp_image_get(output,x,y,0);
      sp_real(v) -=  noise_column[y];
      sp_image_set(output,x,y,0,v);
    }
  }

  for(int y = 0;y<sp_image_y(output);y++){
    noise_column[y] = 0;
    pixels_per_line[y] = 0;
  }

  for(int y = 0;y<sp_image_y(output);y++){
    noise_column[y] = 0;
    pixels_per_line[y] = 0;
  }
  /* proceed with the right side band */  

  for(int x = 2;x<side_band_size/binning-2;x++){
    for(int y = 0;y<sp_image_y(output);y++){
      pixels_per_line[y]++;
      noise_column[y]+= sp_real(sp_image_get(input,sp_image_x(input)-1-x,y,0));
    }
  }
  for(int y = 0;y<sp_image_y(output);y++){
    noise_column[y] /= pixels_per_line[y];
    for(int x = sp_image_x(output)/2;x<sp_image_x(output);x++){
      Complex v = sp_image_get(output,x,y,0);
      sp_real(v) -=  noise_column[y];
      sp_image_set(output,x,y,0,v);
    }
  }

}


/*static void herringbone_removal1(Image * input){
  //just subtract the right side of the image from the left side 
  for(int x = 0;x<sp_image_x(input)/2;x++){
    for(int y = 0;y<sp_image_y(input);y++){
      Complex v = sp_image_get(input,x,y,0);
      Complex v2 = sp_image_get(input,sp_image_x(input)-x-1,y,0);
      sp_real(v) -= sp_real(v2);
      sp_image_set(input,x,y,0,v);
    }
  }
  for(int x=sp_image_x(input)/2;x<sp_image_x(input);x++){
    for(int y = 0;y<sp_image_y(input);y++){
      sp_image_set(input,x,y,0,sp_cinit(0,0));
    }
  }
}*/

Image * xcam_preprocess(Image * input){
  int binning = 1;
  int side_band_size = 52;
  if(sp_image_x(input) < 2048){
    binning = 2;
  }
  if(sp_image_x(input) < 1024){
    binning = 4;
  }
  remove_electronic_vertical_noise(input,binning);
  Image * output = get_physical_image(input,binning,side_band_size);
  remove_electronic_horizontal_noise(output,input,binning,side_band_size);
  //  herringbone_removal1(output);
  return output;
}
