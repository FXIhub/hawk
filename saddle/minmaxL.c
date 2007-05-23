#include <minmaxtau.h>
#include <spimage.h>

void minmaxL(Image * Gs,Image * Gns,Image * F0,Image * S,int niter,int method,Image * DGs0,Image * DGns0,sp_matrix * Hab){
/*
  %[Gs,Gns,Hab,DGs0,DGns0]=minmaxL(Gs,Gns,F0,S,niter,method,DGs0,DGns0);
  % or
  %[Gs,Gns,Hab]=minmaxL(Gs,Gns,S,F0,niter,method);
  %
  % look for the minmax
  % methods:
  % 0 HIO,
  % 1 2D minmax optimization
  % 3 2D minmax optimization,  CG  !!NOT IMPLEMENTED!!
  % 4 4D (reduced memory) optimization   !!NOT IMPLEMENTED!!
*/
  int i;
  int maxiter = 50;
  real TolY = 1e-5;

  Image * tmp;
  sp_vector * tau = sp_vector_alloc(2);  
  if(method !=1 && method != -3){
    fprintf(stderr,"Method not implemented\n");
    return;
  }
  for(i = 0;i<niter;i++){
    if(method==-3){                             /* %ER */
      tmp = sp_image_duplicate(Gns,SP_COPY_DETECTOR);
      gradLrho(Gs->image,tmp->image,S->image,F0,0,DGs0->image,DGns0->image);   /*%new direction */
      sp_image_add(Gs,DGs0);
    }else if(method==1){
      gradLrho(Gs->image,Gns->image,S->image,F0,0,DGs0->image,DGns0->image);
      minmaxtau(Gs->image,Gns->image,DGs0->image,DGns0->image,F0,TolY,maxiter,tau,Hab);
      sp_image_scale(DGs0,tau->data[0]);
      sp_image_scale(DGns0,tau->data[1]);
      sp_image_add(Gs,DGs0);
      sp_image_add(Gns,DGns0);
    }

  }
  
}
