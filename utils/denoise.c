#include <stdio.h>
#include <spimage.h>

real functional(sp_cmatrix * f, sp_cmatrix * u, real beta){
  real dudx;
  real dudy;
  real int_norm = 0;
  real int_fide = 0;
  int x,y;
  for(x = 0;x<sp_cmatrix_cols(u);x++){
    for(y = 0;y<sp_cmatrix_rows(u);y++){
      dudx = (sp_cmatrix_get(u,y,(x+1)%sp_cmatrix_cols(u))-
	     sp_cmatrix_get(u,y,(sp_cmatrix_cols(u)+x-1)%sp_cmatrix_cols(u)))/2;
      dudy = (sp_cmatrix_get(u,(y+1)%sp_cmatrix_rows(u),x)-
	     sp_cmatrix_get(u,(sp_cmatrix_rows(u)+y-1)%sp_cmatrix_rows(u),x))/2;
      int_norm += sqrt(dudx*dudx+dudy*dudy);
      int_fide += beta*(sp_cmatrix_get(u,y,x)-sp_cmatrix_get(f,y,x)*log(cabs(sp_cmatrix_get(u,y,x))));
    }
  }
  return int_norm+int_fide;
}

/*
  Calculates the derivative of the error functional and returns it in d
*/
void functional_derivative(sp_cmatrix * f, sp_cmatrix * u, real beta, sp_cmatrix * d){
  sp_cmatrix * dudx = sp_cmatrix_alloc(sp_cmatrix_rows(f),sp_cmatrix_cols(u));
  sp_cmatrix * dudy = sp_cmatrix_alloc(sp_cmatrix_rows(f),sp_cmatrix_cols(u));
  int x,y;
  real tmp;
  real divergence;
  real dudxdx;
  real dudxdy;
  real dudydx;
  real dudydy;
  for(x = 0;x<sp_cmatrix_cols(u);x++){
    for(y = 0;y<sp_cmatrix_rows(u);y++){
      tmp = (sp_cmatrix_get(u,y,(x+1)%sp_cmatrix_cols(u))-
	     sp_cmatrix_get(u,y,(sp_cmatrix_cols(u)+x-1)%sp_cmatrix_cols(u)))/2;
      sp_cmatrix_set(dudx,y,x,tmp);
      tmp = (sp_cmatrix_get(u,(y+1)%sp_cmatrix_rows(u),x)-
	     sp_cmatrix_get(u,(sp_cmatrix_rows(u)+y-1)%sp_cmatrix_rows(u),x))/2;
      sp_cmatrix_set(dudy,y,x,tmp);
    }
  }
  
  for(x = 0;x<sp_cmatrix_cols(u);x++){
    for(y = 0;y<sp_cmatrix_rows(u);y++){
      dudxdx = (sp_cmatrix_get(dudx,y,(x+1)%sp_cmatrix_cols(u))-
		sp_cmatrix_get(dudx,y,(sp_cmatrix_cols(u)+x-1)%sp_cmatrix_cols(u)))/2;
      dudxdy = (sp_cmatrix_get(dudx,(y+1)%sp_cmatrix_rows(u),x)-
		sp_cmatrix_get(dudx,(sp_cmatrix_rows(u)+y-1)%sp_cmatrix_rows(u),x))/2;
      dudydx = (sp_cmatrix_get(dudy,y,(x+1)%sp_cmatrix_cols(u))-
		sp_cmatrix_get(dudy,y,(sp_cmatrix_cols(u)+x-1)%sp_cmatrix_cols(u)))/2;
      dudydy = (sp_cmatrix_get(dudy,(y+1)%sp_cmatrix_rows(u),x)-
		sp_cmatrix_get(dudy,(sp_cmatrix_rows(u)+y-1)%sp_cmatrix_rows(u),x))/2;      
      divergence = (dudxdx+dudxdy+dudydx+dudydy)/
	(sqrt(sp_cmatrix_get(dudy,y,x)*sp_cmatrix_get(dudy,y,x)+
	      sp_cmatrix_get(dudx,y,x)*sp_cmatrix_get(dudx,y,x)));      
      tmp = -divergence+beta/sp_cmatrix_get(u,y,x)*(sp_cmatrix_get(u,y,x)-sp_cmatrix_get(f,y,x));      
      sp_cmatrix_set(d,y,x,tmp);
    }
  }
  sp_cmatrix_free(dudy);
  sp_cmatrix_free(dudx);
}

/* Try to implement the algorithm from 
   Denoising images with Poisson noise statistics
*/


/* f is the input image and u is the initial guess of the solution
   beta is a parameter that indicates how close to the input image should
   the solution be. The higher beta the smaller the denoising is.
*/
void denoise(Image * f, Image * u, real beta, int max_iter){
  int i,j;
  Image * d = sp_image_alloc(sp_image_width(f),sp_image_height(u));
  real norm_d;
  for(i = 0;i<max_iter;i++){
    functional_derivative(f->image,  u->image,  beta, d->image);
    sp_image_sub(u,d);
    for(j = 0;j<sp_image_size(d);j++){
      u->image->data[i] = cabs(u->image->data[i]);
    }

    norm_d = 0;
 /*   if(i % 10 == 9){
      for(j = 0;j<sp_image_size(d);j++){
	norm_d += d->image->data[i]*d->image->data[i];
      }
      printf("E - %f |d| - %f\n",functional(f->image,u->image,beta),sqrt(norm_d));
    }*/
  }
  
}
 
 int main(int argc, char ** argv){
  Image * in;
  Image * out;
  if(argc < 5){
    printf("Usage: denoise <input image> <output image> <beta> <iterations>\n");
    exit(0);
  }
  in = read_imagefile(argv[1]);
  out = gaussian_blur(in,3);


  denoise(in,out,atof(argv[3]),atoi(argv[4]));
  sp_image_write(out,argv[2],sizeof(real));

  return 0;      
}
