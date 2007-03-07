#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <hdf5.h>

#include "spimage.h"
#include "combine_images.h"


void reorder_images(Image ** list,int n,int * ordered_index){
  int i;
  Image ** list_copy = malloc(sizeof(Image *)*n);
  memcpy(list_copy,list,sizeof(Image *)*n);
  for(i = 0;i<n;i++){
    list[ordered_index[i]] = list_copy[i];
  }
  free(list_copy);
}

real background(real * list, int n){
  return list[n-40];
}

real * get_backgrounds(real ** w_plots, int n){
  real * back = malloc(sizeof(real)*n);
  int i;
  for(i = 0;i<n;i++){
    back[i] = background(w_plots[i],NBINS-20);
    printf("Background %d - %f\n",i,back[i]);
  }  
  return back;
}

real ** get_image_weights(real ** local_cc, real ** w_plots, int n){
  int i;
  int flag = 0;
  int * high_cc_begin = calloc(n-1,sizeof(int));
  int * high_cc_end = calloc(n-1,sizeof(int));
  int img = n-1;
  real ** wg = calloc(n,sizeof(real*));
  for(i = 0;i<n;i++){
    wg[i] = calloc(NBINS,sizeof(real));
    if(i < n-1){
      high_cc_begin[i] = -1;
      high_cc_end[i] = -1;
    }
  }
  for(i = NBINS-1;i>=0;i--){
    if(!flag && local_cc[img-1][i] > 0.95 && w_plots[img-1][i] > background(w_plots[img-1],NBINS)*2){
      flag = 1;
      high_cc_begin[img-1] = i;
    }
    if(flag && local_cc[img-1][i] < 0.95){
      high_cc_end[img-1] = i;
      flag = 0;
      img--;
    }
    /* No points in checking last image */
    if(img == 0){
      break;
    }
  }
  img = n-1;
  for(i = NBINS-1;i>=0;i--){
    if(img == 0){
      wg[img][i] = 1.0;
      continue;
    }
    if(i > high_cc_begin[img-1]){
      wg[img][i] = 1.0;
    }else if(i > high_cc_end[img-1]){
      wg[img][i] = 1.0-(real)(i-high_cc_begin[img-1])/(high_cc_end[img-1]-high_cc_begin[img-1]);
      wg[img-1][i] = (real)(i-high_cc_begin[img-1])/(high_cc_end[img-1]-high_cc_begin[img-1]);
    }else{
      img--;
      wg[img][i] = 1.0;
    }
  }				   
  free(high_cc_begin);
  free(high_cc_end);
  return wg;
}



Image * combine_images(Image  ** list,int * ordered_index,int n, real * scale,real ** image_weight, real * background){
  Image *res = imgcpy(list[0]);
  int x,y;
  int bin;
  real d;
  Image *img = list[0];
  int index = 0;
  int index2 = 0;
  int i;
  int * center_diff  = malloc(sizeof(int)*n);
  for(i = 0;i<n;i++){
    center_diff[i] = -pixel_to_index(list[0],list[0]->detector->image_center)+
      pixel_to_index(list[ordered_index[i]],list[ordered_index[i]]->detector->image_center);
  }
  for(x = 0;x<sp_cmatrix_cols(list[0]->image);x++){
    for(y = 0;y<sp_cmatrix_rows(list[0]->image);y++){
      /* calculate distance to center and extract pixel from appropriate image */
      d = (x-img->detector->image_center[0])*(x-img->detector->image_center[0]);
      d += (y-img->detector->image_center[1])*(y-img->detector->image_center[1]);
      d = sqrt(d);
      /* put in a bin between 0 and NBINS. */
      bin = (int)(NBINS*2.0*d/sqrt(sp_cmatrix_cols(img->image)*sp_cmatrix_cols(img->image)+
				   sp_cmatrix_rows(img->image)*sp_cmatrix_rows(img->image)));
      if(bin >= NBINS){
	bin = NBINS-1;
      }
      res->image->data[index] = 0;
      for(i = 0;i<n;i++){
	/* index2 = index - difference of image centers */
	index2 = index+center_diff[ordered_index[i]];
/*	index2 = index;*/
	if(index2 >= 0 && index2 < sp_cmatrix_size(list[0]->image)){
	  res->image->data[index] += (list[ordered_index[i]]->image->data[index2]-background[i])*scale[i]*image_weight[i][bin];
/*	}else{
	  res->image->data[index] += 0;*/
	}
      }
      index++;
    }
  }
  return res;
}
void plot_scaled_wplots(real ** w_plots,real * scale, int n){
  int i,k;
  char buffer[1024];
  FILE * file;
  for(i = 0;i<n;i++){
    sprintf(buffer,"%d-scaledW.plot",i);
    file = fopen(buffer,"w");
    for(k = 0;k<NBINS;k++){
      fprintf(file,"%f\n",(w_plots[i][k]-background(w_plots[i],NBINS))*scale[i]);	
    }
    fclose(file);
    sprintf(buffer,"%d-W.plot",i);
    file = fopen(buffer,"w");
    for(k = 0;k<NBINS;k++){
      fprintf(file,"%f\n",w_plots[i][k]);	
    }
    fclose(file);
  }
}

/* We're gonna scale all the images to the same scaled as the most exposed one */
real * get_relative_scale(real ** w_plots,real ** local_cc, int n){
  int i,j;
  int flag;
  real avg_scale;
  int bins;
  real * res = malloc(sizeof(real)*n);
  for(i = 0;i<n-1;i++){
    avg_scale = 0;
    bins = 0;
    flag = 0;
    for(j = 0;j<NBINS;j++){
      if(!flag && local_cc[i][j] > 0.95){
	flag = 1;
      }else if(flag && local_cc[i][j] < 0.95){
	break;
      }
      if(flag){
	avg_scale += (w_plots[i+1][j]-background(w_plots[i+1],NBINS))/(w_plots[i][j]-background(w_plots[i],NBINS));
	bins++;
      }
    }
    avg_scale /= bins;
    res[i] = avg_scale;
  }
  res[n-1] = 1.0;
  for(i = n-2;i>=0;i--){
    res[i] *= res[i+1];
  }
  return res;
}

/* Order from less exposure to more exposure */
int * order_by_exposure(real ** w_plots,int n){
  /* Just use the integral of the plots to order */
  int * res = malloc(sizeof(int)*n);
  real *intg = malloc(sizeof(real)*n);
  real last_min = 0;
  real cur_min = 1e10;
  int i,j;
  real ** tmp = malloc(sizeof(real*)*n);
  for(i =0 ;i<n;i++){
    intg[i] = 0;
    for(j = 0;j<NBINS;j++){
      intg[i] += w_plots[i][j];
    }
  }
  for(i = 0;i<n;i++){
    for(j = 0;j<n;j++){
      if(intg[j] < cur_min && intg[j] > last_min){
	cur_min = intg[j];
	res[i] = j;
      }
    }
    tmp[i] = w_plots[res[i]];
    last_min = cur_min;
    cur_min =1e10;
  }
  for(i = 0;i<n;i++){
    w_plots[i] = tmp[i];
  }
  free(tmp);
  free(intg);
  return res;  
}

real cross_correlation(real * x, real * y, int n){
  int i;
  real mx,my,sx,sy,sxy,denom,r;
  
  /* Calculate the mean of the two series x[], y[] */
  mx = 0;
  my = 0;   
  for (i=0;i<n;i++) {
    mx += x[i];
    my += y[i];
  }
  mx /= n;
  my /= n;
  
  /* Calculate the denominator */
  sx = 0;
  sy = 0;
  for (i=0;i<n;i++) {
    sx += (x[i] - mx) * (x[i] - mx);
    sy += (y[i] - my) * (y[i] - my);
  }
  denom = sqrt(sx*sy);
  
  sxy = 0;
  for (i=0;i<n;i++) {
    sxy += (x[i] - mx) * (y[i] - my);
  }
  r = sxy / denom;
  return r;
}

real * local_cross_correlation(real * x, real * y, int n, int window){
  real * res = malloc(sizeof(real)*n-window);
  int i;
  for(i = 0;i<n-window;i++){
    res[i] = cross_correlation(&(x[i]),&(y[i]),window);
  }
  return res;
}


void plot_local_cc(real ** w_plots, int n){
  int i,j,k;
  real * list;
  char buffer[1024];
  FILE * file;
  int window = 20;
  for(i = 0;i<n;i++){
    for(j = i+1;j<n;j++){
      list = local_cross_correlation(w_plots[i],w_plots[j],NBINS,window);
      sprintf(buffer,"%d-%d-localcc.plot",i,j);
      file = fopen(buffer,"w");
      for(k = 0;k<NBINS-window;k++){
	fprintf(file,"%f\n",list[k]);	
      }
      fclose(file);
      free(list);
    }
  }
  
}


real ** get_local_cc(real ** w_plots, int n){
  int i;
  int window = 20;
  real ** local_cc = malloc(sizeof(real *)*n-1);
  for(i = 0;i<n-1;i++){
    local_cc[i] = local_cross_correlation(w_plots[i],w_plots[i+1],NBINS,window);
  }
  return local_cc;
}


real * wilson_plot(Image * img){
  int i,x,y;
  real d;
  int bin;
  real * value = malloc(sizeof(real)*NBINS);
  real * members = malloc(sizeof(real)*NBINS);
  for(i = 0;i<NBINS;i++){
    value[i] = 0;
    members[i] = 0;
  }
  for(x = 0;x<sp_cmatrix_cols(img->image);x++){
    for(y = 0;y<sp_cmatrix_rows(img->image);y++){
      d = (x-img->detector->image_center[0])*(x-img->detector->image_center[0]);
      d += (y-img->detector->image_center[1])*(y-img->detector->image_center[1]);
      d = sqrt(d);
      /* put in a bin between 0 and NBINS. */
      bin = (int)(NBINS*2.0*d/sqrt(sp_cmatrix_cols(img->image)*sp_cmatrix_cols(img->image)+
				   sp_cmatrix_rows(img->image)*sp_cmatrix_rows(img->image)));
      if(bin >= NBINS){
	bin = NBINS-1;
      }
      value[bin] += img->image->data[x*sp_cmatrix_rows(img->image)+y];
      members[bin]++;
      
    }
  }
  for(i = 0;i<NBINS;i++){
    value[i] /= members[i];
  }
  free(members);
  return value;
}

void mask_saturated(Image ** img, int n, real saturated){
  int i,j;
  for(i= 0;i<n;i++){
    for(j= 0;j<sp_cmatrix_size(img[i]->image);j++){
      if(cabs(img[i]->image->data[j]) > saturated){
	img[i]->mask->data[j] = 0;
      }
    }
  }
}


Image **  make_image_ratio(Image ** list,real * background, int n){
  Image ** image_ratio = malloc(sizeof(Image *)*n-1);
  int i,j;
  int * center_diff  = malloc(sizeof(int)*n);
  real avg_ratio = 0;
  int points = 0;
  for(i = 0;i<n;i++){
    center_diff[i] = -pixel_to_index(list[0],list[0]->detector->image_center)+
      pixel_to_index(list[i],list[i]->detector->image_center);
  }
  
  for(i = 0;i<n-1;i++){
    image_ratio[i] = create_empty_img(list[i]);    
    for(j = 0;j<sp_cmatrix_size(list[i]->image);j++){
      image_ratio[i]->mask->data[j] = 0;
      if(j+center_diff[i] >= 0 && j+center_diff[i] < sp_cmatrix_size(list[i]->image) &&
	 j+center_diff[i+1] >= 0 && j+center_diff[i+1] < sp_cmatrix_size(list[i+1]->image)){
	/* Only count points which are 3x above background */
	if(list[i]->mask->data[j] && list[i+1]->mask->data[j] && creal(list[i]->image->data[j]-6*background[i]) > 0){
	  image_ratio[i]->image->data[j] = (list[i+1]->image->data[j+center_diff[i+1]]-background[i+1])/
	    (list[i]->image->data[j+center_diff[i]]-background[i]);
	  image_ratio[i]->mask->data[j] = 1;
	  points++;
	  avg_ratio += image_ratio[i]->image->data[j];
	}else{
	     image_ratio[i]->image->data[j] = 0;
	     image_ratio[i]->mask->data[j] = 0;
	}
      }
    }
    avg_ratio /= points;
    printf("Ratio - %f\n",avg_ratio);
    avg_ratio = 0;
    points = 0;
  }
  
  return image_ratio;
}


real * get_scale_from_ratio(Image ** image_ratio, int n){
  int i,j;
  int points;
  double avg_ratio;
  real * scale = malloc(sizeof(real)*(n+1));
  for(i =0 ;i<n;i++){
    points = 0;
    avg_ratio = 0;
    for(j = 0 ;j<sp_cmatrix_size(image_ratio[i]->image);j++){
      if(image_ratio[i]->mask->data[j]){
	points++;
	avg_ratio += image_ratio[i]->image->data[j];
      }      
    }
    avg_ratio/= points;
    scale[i] = avg_ratio;
  }
  scale[n] = 1;
  for(i =n-1 ;i>=0;i--){
    scale[i] *= scale[i+1];
  }
  return scale;
}

int main(int argc, char ** argv){
  Image ** list;
  Image ** image_ratio;
  real ** w_plots;
  real ** local_cc;
  char  buffer[1024];
  int i,j;
  FILE * file;
  int * ordered_index;
  real * scale;
  real ** image_weights;
  real * backgrounds;
  Image * comb_img;
  int saturation = 240;

  if(argc < 3){
    printf("Usage: combine_images <image1> [image2] [image3] ... <outimage>\n");
    exit(0);
  }
  if(fopen(argv[argc-1],"r") != NULL){
    printf("The specified output file already exist!\n");
    printf("Usage: combine_images <image1> [image2] [image3] ... <outimage>\n");
    exit(0);
  }
  list = malloc(sizeof(Image *)*argc-2);
  w_plots = malloc(sizeof(real *)*argc-2);
  for(i = 1 ;i<argc-1;i++){
    list[i-1] = read_imagefile(argv[i]);
    find_center(list[i-1],&(list[i-1]->detector->image_center[0]),&(list[i-1]->detector->image_center[1]));
/*    list[i-1]->detector->image_center[0] = 1138.000000;
    list[i-1]->detector->image_center[1] = 1608.500000;*/
    w_plots[i-1] = wilson_plot(list[i-1]);
    sprintf(buffer,"%s.plot",argv[i]);
    file = fopen(buffer,"w");
    for(j = 0;j<NBINS;j++){
      fprintf(file,"%f\n",w_plots[i-1][j]);
    }    
    fclose(file);      

  }
  /* This will actually alter the order of the wilson plots */
  ordered_index = order_by_exposure(w_plots,argc-2);
  /* This will alter the order of the images so they go
     from the less exposed to the more exposed */
  reorder_images(list,argc-2,ordered_index);

  backgrounds = get_backgrounds(w_plots,argc-2);

  mask_saturated(list,argc-2,saturation);

  image_ratio = make_image_ratio(list,backgrounds,argc-2);

  for(i = 0;i<argc-3;i++){
    sprintf(buffer,"ratio-%d.png",i);
    write_png(image_ratio[i],buffer,COLOR_JET);
    sprintf(buffer,"ratio-%d.vtk",i);
    write_vtk(image_ratio[i],buffer);
  }

  local_cc = get_local_cc(w_plots,argc-2);  
/*  plot_local_cc(local_cc,argc-2);*/
  scale =  get_relative_scale(w_plots,local_cc, argc-2);
  scale =  get_scale_from_ratio(image_ratio, argc-3);
  for(i = 0;i<argc-2;i++){
    printf("%d - %f\n",i,scale[i]);
  }
  plot_scaled_wplots(w_plots,scale,argc-2);
  image_weights = get_image_weights(local_cc,w_plots,argc-2);
  comb_img = combine_images(list,ordered_index,argc-2,scale,image_weights,backgrounds);
  write_img(comb_img,"comb_img.h5",sizeof(real));
  write_png(comb_img,"comb_img.png",COLOR_JET|LOG_SCALE);
  write_vtk(comb_img,"comb_img.vtk");
  return 0;
}
