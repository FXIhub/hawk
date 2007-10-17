#include <stdio.h>
#include <spimage.h>

real functional(sp_c3matrix * f, sp_c3matrix * u, real beta){
  real dudx;
  real dudy;
  real dudz;
  real int_norm = 0;
  real int_fide = 0;
  int x,y,z;
  for(x = 0;x<sp_c3matrix_x(u);x++){
    for(y = 0;y<sp_c3matrix_y(u);y++){
      for(z = 0;z<sp_c3matrix_z(u);z++){
	dudx = (sp_real(sp_c3matrix_get(u,(x+1)%sp_c3matrix_x(u),y,z))-
		sp_real(sp_c3matrix_get(u,(sp_c3matrix_x(u)+x-1)%sp_c3matrix_x(u),y,z)))/2;
	dudy = (sp_real(sp_c3matrix_get(u,x,(y+1)%sp_c3matrix_y(u),z))-
		sp_real(sp_c3matrix_get(u,x,(sp_c3matrix_y(u)+y-1)%sp_c3matrix_y(u),z)))/2;
	dudz = (sp_real(sp_c3matrix_get(u,x,y,(z+1)%sp_c3matrix_z(u)))-
		sp_real(sp_c3matrix_get(u,x,y,(sp_c3matrix_z(u)+z-1)%sp_c3matrix_z(u))))/2;
	int_norm += sqrt(dudx*dudx+dudy*dudy+dudz*dudz);
	int_fide += beta*(sp_real(sp_c3matrix_get(u,x,y,z))-sp_real(sp_c3matrix_get(f,x,y,z))*log(sp_cabs(sp_c3matrix_get(u,x,y,z))));
      }
    }
  }
  return int_norm+int_fide;
}

/*
  Calculates the derivative of the error functional and returns it in d
*/
void functional_derivative(sp_c3matrix * f, sp_c3matrix * u, real beta, sp_c3matrix * d){
  sp_c3matrix * dudx = sp_c3matrix_alloc(sp_c3matrix_x(f),sp_c3matrix_y(f),
					 sp_c3matrix_z(f));
  sp_c3matrix * dudy = sp_c3matrix_alloc(sp_c3matrix_x(f),sp_c3matrix_y(f),
					 sp_c3matrix_z(f));
  sp_c3matrix * dudz = sp_c3matrix_alloc(sp_c3matrix_x(f),sp_c3matrix_y(f),
					 sp_c3matrix_z(f));
  int x,y,z;
  real tmp;
  real divergence;
  real dudxdx;
  real dudxdy;
  real dudxdz;
  real dudydx;
  real dudydy;
  real dudydz;
  real dudzdx;
  real dudzdy;
  real dudzdz;
  
  for(x = 0;x<sp_c3matrix_x(u);x++){
    for(y = 0;y<sp_c3matrix_y(u);y++){
      for(z = 0;z<sp_c3matrix_z(u);z++){
	tmp = (sp_real(sp_c3matrix_get(u,(x+1)%sp_c3matrix_x(u),y,z))-sp_real(sp_c3matrix_get(u,(sp_c3matrix_x(u)+x-1)%sp_c3matrix_x(u),y,z)))/2;
	sp_c3matrix_set(dudx,x,y,z,sp_cinit(tmp,0));
	tmp = (sp_real(sp_c3matrix_get(u,x,(y+1)%sp_c3matrix_y(u),z))-sp_real(sp_c3matrix_get(u,x,(sp_c3matrix_y(u)+y-1)%sp_c3matrix_y(u),z)))/2;
	sp_c3matrix_set(dudy,x,y,z,sp_cinit(tmp,0));
	tmp = (sp_real(sp_c3matrix_get(u,x,y,(z+1)%sp_c3matrix_z(u)))-sp_real(sp_c3matrix_get(u,x,y,(sp_c3matrix_z(u)+z-1)%sp_c3matrix_z(u))))/2;
      }
    }
  }
  
  for(x = 0;x<sp_c3matrix_x(u);x++){
    for(y = 0;y<sp_c3matrix_y(u);y++){
      for(z = 0;z<sp_c3matrix_z(u);z++){
	dudxdx = (sp_real(sp_c3matrix_get(dudx,(x+1)%sp_c3matrix_x(u),y,z))-sp_real(sp_c3matrix_get(dudx,(sp_c3matrix_x(u)+x-1)%sp_c3matrix_x(u),y,z)))/2;
	dudxdy = (sp_real(sp_c3matrix_get(dudx,x,(y+1)%sp_c3matrix_y(u),z))-sp_real(sp_c3matrix_get(dudx,x,(sp_c3matrix_y(u)+y-1)%sp_c3matrix_y(u),z)))/2;
	dudxdz = (sp_real(sp_c3matrix_get(dudx,x,y,(z+1)%sp_c3matrix_z(u)))-sp_real(sp_c3matrix_get(dudx,x,y,(sp_c3matrix_z(u)+z-1)%sp_c3matrix_z(u))))/2;
	dudydx = (sp_real(sp_c3matrix_get(dudy,(x+1)%sp_c3matrix_x(u),y,z))-sp_real(sp_c3matrix_get(dudy,(sp_c3matrix_x(u)+x-1)%sp_c3matrix_x(u),y,z)))/2;
	dudydy = (sp_real(sp_c3matrix_get(dudy,x,(y+1)%sp_c3matrix_y(u),z))-sp_real(sp_c3matrix_get(dudy,x,(sp_c3matrix_y(u)+y-1)%sp_c3matrix_y(u),z)))/2;      
	dudydz = (sp_real(sp_c3matrix_get(dudy,x,y,(z+1)%sp_c3matrix_z(u)))-sp_real(sp_c3matrix_get(dudy,x,y,(sp_c3matrix_z(u)+z-1)%sp_c3matrix_z(u))))/2;
	dudzdx = (sp_real(sp_c3matrix_get(dudz,(x+1)%sp_c3matrix_x(u),y,z))-sp_real(sp_c3matrix_get(dudz,(sp_c3matrix_x(u)+x-1)%sp_c3matrix_x(u),y,z)))/2;
	dudzdy = (sp_real(sp_c3matrix_get(dudz,x,(y+1)%sp_c3matrix_y(u),z))-sp_real(sp_c3matrix_get(dudz,x,(sp_c3matrix_y(u)+y-1)%sp_c3matrix_y(u),z)))/2;
	dudzdz = (sp_real(sp_c3matrix_get(dudz,x,y,(z+1)%sp_c3matrix_z(u)))-sp_real(sp_c3matrix_get(dudz,x,y,(sp_c3matrix_z(u)+z-1)%sp_c3matrix_z(u))))/2;
	divergence = (dudxdx+dudxdy+dudxdz+dudydx+dudydy+
		      dudydz+dudzdx+dudzdy+dudzdz)/
	  (sqrt(sp_real(sp_c3matrix_get(dudx,x,y,z))*sp_real(sp_c3matrix_get(dudx,x,y,z))+
		sp_real(sp_c3matrix_get(dudy,x,y,z))*sp_real(sp_c3matrix_get(dudy,x,y,z))+
		sp_real(sp_c3matrix_get(dudz,x,y,z))*sp_real(sp_c3matrix_get(dudz,x,y,z))));
	tmp = -divergence+beta/sp_real(sp_c3matrix_get(u,x,y,z))*(sp_real(sp_c3matrix_get(u,x,y,z))-sp_real(sp_c3matrix_get(f,x,y,z)));      
	sp_c3matrix_set(d,x,y,z,sp_cinit(tmp,0));
      }
    }
  }
  sp_c3matrix_free(dudx);
  sp_c3matrix_free(dudy);
  sp_c3matrix_free(dudz);
}

/* Try to implement the algorithm from 
   Denoising images with Poisson noise statistics
*/


/* f is the input image and u is the initial guess of the solution
   beta is a parameter that indicates how close to the input image should
   the solution be. The higher beta the smaller the denoising is.
*/
void denoise(Image * f, Image * u, real beta, int max_iter){
  long long i,j;
  Image * d = sp_image_alloc(sp_image_x(f),sp_image_y(f),sp_image_z(f));
  real norm_d;
  for(i = 0;i<max_iter;i++){
    functional_derivative(f->image,  u->image,  beta, d->image);
    sp_image_sub(u,d);
    for(j = 0;j<sp_image_size(d);j++){
      u->image->data[i] = sp_cinit(sp_cabs(u->image->data[i]),0);
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
  in = sp_image_read(argv[1],0);
  out = gaussian_blur(in,3);



  denoise(in,out,atof(argv[3]),atoi(argv[4]));
  sp_image_write(out,argv[2],sizeof(real)|SP_3D);

  return 0;      
}
