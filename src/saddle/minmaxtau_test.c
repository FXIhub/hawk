#include <spimage.h>
#include "minmaxtau.h"
#include "minmaxL.h"
#include <sys/time.h>

#include <time.h>


int main(){
  Image * lena = sp_image_read("lena_support256.h5",0);
  Image * F0 = sp_image_fft(lena); 
  for(int i = 0;i<sp_image_size(F0);i++){
    F0->mask->data[i] = 1;
  }
    F0->mask->data[42342] = 0;
  Image * G;
  sp_image_dephase(F0);
  G = sp_image_duplicate(F0,SP_COPY_DATA|SP_COPY_MASK);
  sp_srand(15);
  sp_image_rephase(G,SP_RANDOM_PHASE);
  
  Image * gs;
  Image * gns;
  Image * Gs;
  Image * Gns;
  Image * DGs0 = sp_image_duplicate(F0,SP_COPY_DETECTOR);
  Image * DGns0 = sp_image_duplicate(F0,SP_COPY_DETECTOR);
  Image * S = sp_image_duplicate(F0,SP_COPY_DETECTOR);
  Image * tmp, *tmp2;
  long t_gradLrho = 0;
  long t_minmaxtau = 0;

  sp_3matrix * Hab = sp_3matrix_alloc(2,2,1);
  int method = 1;
  int ii,x,y,i;
  sp_image_dephase(S);
  ii = 0;  
  for(x = 0;x<sp_image_x(S);x++){
    for(y = 0;y<sp_image_y(S);y++){
      if(x < sp_image_x(S)/4 ||
	 x > (sp_image_x(S)*3)/4 ||
	 y < sp_image_y(S)/4 ||
	 y > (sp_image_y(S)*3)/4){
	sp_real(S->image->data[ii]) = 0;
	sp_imag(S->image->data[ii]) = 0;
      }else{
	sp_real(S->image->data[ii]) = 1;
	sp_imag(S->image->data[ii]) = 0;
      }
      ii++;
    }
  }
  sp_c3matrix_set(S->image,sp_image_x(S)/4,sp_image_y(S)*3/4,0,sp_cinit(0,0));
  sp_c3matrix_set(S->image,sp_image_x(S)*3/4,sp_image_y(S)/4,0,sp_cinit(0,0));

  
  gs = sp_image_ifft(G);
  gns = sp_image_duplicate(gs,SP_COPY_DATA|SP_COPY_MASK);
  /* normalize */
  sp_image_scale(gs,1.0/sp_image_size(gs));
  sp_image_scale(gns,1.0/sp_image_size(gns));

  for(i = 0;i<sp_image_size(S);i++){
    if(sp_real(S->image->data[i])){
      sp_real(gns->image->data[i]) = 0;
      sp_imag(gns->image->data[i]) = 0;
    }else{
      sp_real(gs->image->data[i]) = 0;
      sp_imag(gs->image->data[i]) = 0;
    }
  }
  Gs = sp_image_fft(gs);
  Gns = sp_image_fft(gns);

  if(0){
    Gs = sp_image_read("Gs.h5",0);
    Gns = sp_image_read("Gns.h5",0);
    F0 = sp_image_read("F0.h5",0);
  }

  for(i = 0;i<30000;i++){
    printf("Iteration %i\n",i);
    if(i % 20 == 19){
      minmaxL(Gs,Gns,F0,S,1,-3,DGs0,DGns0,Hab);
    }else{
      minmaxL(Gs,Gns,F0,S,1,method,DGs0,DGns0,Hab);
    }
    if(i % 5 == 4){
      tmp = sp_image_duplicate(Gs,SP_COPY_DATA|SP_COPY_MASK);
      tmp2 = sp_image_ifft(tmp);
      sp_image_free(tmp);
      sp_image_write(tmp2,"g.png",SpColormapGrayScale);
      sp_image_free(tmp2);
    }
  }
  printf("t_gradLrho - %ld usec\n",t_gradLrho);
  printf("t_minmaxtau - %ld usec\n",t_minmaxtau);
  sp_image_free(Gs);
  sp_image_free(Gns);
  sp_image_free(S);
  sp_image_free(F0);
  sp_image_free(DGs0);
  sp_image_free(DGns0);
  
  sp_3matrix_free(Hab);
  return 0;
}
