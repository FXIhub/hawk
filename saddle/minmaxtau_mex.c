#include <mex.h>
#include <minmaxtau.h>

real mxArray_to_real(const mxArray * a){
  return *mxGetPr(a);
}

mxArray * real_to_mxArray(const real x){
  mxArray * ret = mxCreateDoubleMatrix(1,1, mxREAL);
  *mxGetPr(ret) = x;
  return ret;
}

sp_vector * mxArray_to_vector(const mxArray * a){
  int rows;
  int cols;
  double * rdata;
  int i;
  sp_vector * ret;
  rdata = mxGetPr(a);
  rows = mxGetN(a);
  cols = mxGetM(a);  
  ret = sp_vector_alloc(rows*cols);
  for(i = 0;i<sp_vector_size(ret);i++){
    sp_vector_set(ret,i,rdata[i]);    
  }
  return ret;
}

mxArray * sp_vector_to_mxArray(const sp_vector * v){
  double * rdata;
  int i;
  mxArray * ret = mxCreateDoubleMatrix(1,sp_vector_size(v), mxREAL);
  rdata = mxGetPr(ret);
  for(i = 0;i<sp_vector_size(v);i++){
    rdata[i] = sp_vector_get(v,i);
  }
  return ret;
}

sp_matrix * mxArray_to_matrix(const mxArray * a){
  int rows;
  int cols;
  double * rdata;
  int i,j,k;
  sp_matrix * ret;
  rdata = mxGetPr(a);
  rows = mxGetN(a);
  cols = mxGetM(a);  
  ret = sp_matrix_alloc(rows,cols);
  /* Matlab is Column major like we are. Yuppie! */
  k = 0;
  for(i = 0;i<sp_matrix_cols(ret);i++){
    for(j = 0;j<sp_matrix_rows(ret);j++){
      sp_matrix_set(ret,j,i,rdata[k++]);    
    }      
  }
  return ret;
}

mxArray * sp_matrix_to_mxArray(const sp_matrix * m){
  double * rdata;
  int i,j,k;
  mxArray * ret = mxCreateDoubleMatrix(sp_matrix_rows(m),sp_matrix_cols(m), mxREAL);
  rdata = mxGetPr(ret);
  /* Matlab is Column major like we are. Yuppie! */
  k = 0;
  for(i = 0;i<sp_matrix_cols(m);i++){
    for(j = 0;j<sp_matrix_rows(m);j++){
      rdata[k++] = sp_matrix_get(m,j,i);
    }      
  }
  return ret;
}

Image * mxArray_to_Image(const mxArray * a){
  double * rdata;
  double * idata;
  int rows;
  int cols;
  int i = 0;
  Image * ret;
  rdata = mxGetPr(a);
  idata = mxGetPi(a);
  rows = mxGetN(a);
  cols = mxGetM(a);  
  ret = create_new_img(cols,rows);

  /* Matlab is Column major like we are. Yuppie! */
  if(idata){
    sp_image_rephase(ret,SP_ZERO_PHASE);
    for(i = 0;i<cols*rows;i++){
      ret->image->data[i] = rdata[i]+idata[i]*I;
    }
  }else{
    for(i = 0;i<cols*rows;i++){
      ret->image->data[i] = rdata[i];
    }
  }
  return ret;
    
}

void mexFunction(int nlhs, mxArray ** plhs, int nrhs, const mxArray ** prhs){
  Image * Gs = mxArray_to_Image(prhs[0]);
  Image * Gns = mxArray_to_Image(prhs[1]);
  Image * DGs = mxArray_to_Image(prhs[2]);
  Image * DGns = mxArray_to_Image(prhs[3]);
  Image * F0 = mxArray_to_Image(prhs[4]);
  int maxiter = 50;
  real TolY = 1e-5;
  int ii;
  sp_vector * tau = sp_vector_alloc(2);
  sp_matrix * Hi = sp_matrix_alloc(2,2);
  
/*  write_img(Gs,"Gs.h5",sizeof(real));
  write_img(Gns,"Gns.h5",sizeof(real));
  write_img(DGs,"DGs.h5",sizeof(real));
  write_img(DGns,"DGns.h5",sizeof(real));
  write_img(F0,"F0.h5",sizeof(real));*/
  if(nrhs > 5){
    TolY = mxArray_to_real(prhs[5]);
  }
  if(nrhs > 6){

    maxiter = mxArray_to_real(prhs[6]);
  }

  ii = minmaxtau(Gs->image,Gns->image,DGs->image,DGns->image,F0->image,TolY,maxiter,tau,Hi);
  if(nlhs > 0){
    plhs[0] = sp_vector_to_mxArray(tau);
  }
  if(nlhs > 1){
    plhs[1] = sp_matrix_to_mxArray(Hi);
  }
  if(nlhs > 2){
    plhs[2] = real_to_mxArray(ii);  
  }
  sp_image_free(Gs);
  sp_image_free(Gns);
  sp_image_free(DGs);
  sp_image_free(DGns);
  sp_image_free(F0);
  sp_matrix_free(Hi);
  sp_vector_free(tau);
}
