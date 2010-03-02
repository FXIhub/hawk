#include <spimage.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_double.h>


double row_median(Image * a, int x, int y, int length){
  double * buffer = malloc(sizeof(double)*length);
  for(int xi = x;xi<x+length;xi++){
    buffer[xi-x] = sp_real(sp_image_get(a,xi,y,0));
  }
  gsl_sort(buffer,1,length);
  double ret =  gsl_stats_median_from_sorted_data(buffer,1,length);
  free(buffer);
  return ret;  
}

void remove_row_median(Image * a, int x, int y, int length){
  double median = row_median(a,x,y,length);
  for(int xi = x;xi<x+length;xi++){
    sp_image_set(a,xi,y,0,sp_cinit(sp_real(sp_image_get(a,xi,y,0))-median,0));
  }  
}

double column_median(Image * a, int x, int y, int length){
  double * buffer = malloc(sizeof(double)*length);
  for(int yi = y;yi<y+length;yi++){
    buffer[yi-y] = sp_real(sp_image_get(a,x,yi,0));
  }
  gsl_sort(buffer,1,length);
  double ret =  gsl_stats_median_from_sorted_data(buffer,1,length);
  free(buffer);
  return ret;  
}

void remove_column_median(Image * a, int x, int y, int length){
  double median = column_median(a,x,y,length);
  for(int yi = y;yi<y+length;yi++){
    sp_image_set(a,x,yi,0,sp_cinit(sp_real(sp_image_get(a,x,yi,0))-median,0));
  }  
}

void remove_modes_from_panel(Image * a, int x, int y){
  const int nrows = 128;
  const int ncols = 512;
  for(int row = 0;row<nrows;row++){
    remove_row_median(a, x, y+row, ncols);
  }
  for(int column = 0;column<ncols;column++){
    //    remove_column_median(a, x+column, y, nrows);
  }
}

void remove_modes_from_image(Image * a){
  const int ccdxn = 4;
  const int ccdyn = 8;
  const int ncols = 512;
  const int nrows = 128;
  for(int ccdx = 0;ccdx<ccdxn;ccdx++){
    int x = ccdx*ncols;
    for(int ccdy = 0;ccdy<ccdyn;ccdy++){
      int y = ccdy*nrows;
      remove_modes_from_panel(a,x,y);
    }
  }
}

Image * poor_mans_median(Image * a){
  Image * ret = sp_image_duplicate(a,SP_COPY_ALL);
  for(int x = 0;x<sp_image_x(a);x++){
    for(int y = 0;y<sp_image_y(a);y++){
      int zero_flag = 0;
      for(int dx = -1;dx<=1;dx++){
	int xx = (x+dx+sp_image_x(a))%sp_image_x(a);
	for(int dy = -1;dy<=1;dy++){
	  int yy = (y+dy+sp_image_y(a))%sp_image_y(a);
	  if(sp_real(sp_image_get(a,xx,yy,0)) < 0){
	    zero_flag = 1;
	    break;
	  }
	}
      }
      if(zero_flag){
	sp_image_set(ret,x,y,0,sp_cinit(0,0));
      }
    }
  }
  return ret;
}

int main(int argc, char ** argv){
  Image * a = sp_image_read(argv[1],0);
  remove_modes_from_image(a);
  sp_image_write(a,"out.h5",0);
  sp_image_write(poor_mans_median(a),"out2.h5",0);
  return 0;
}
