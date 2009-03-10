#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <spimage.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_statistics_double.h>
#include "gtm.h"


  
Params * init_params(){
  Params * p = malloc(sizeof(Params));
  p->N = 3000;
  p->K = 120;
  p->M = 60;
  p->L = 1;
  p->beta = 0.001;
  p->alpha = 0.01;
  init_data("./images","image",p,1600);

  p->Y = gsl_matrix_alloc(p->K,p->D);
  p->R = gsl_matrix_alloc(p->K,p->N);
  p->delta = gsl_matrix_alloc(p->K,p->N);
  p->G = gsl_vector_alloc(p->K);
  p->phiTGphi = gsl_matrix_alloc(p->M,p->M);
  p->inverse = gsl_matrix_alloc(p->M,p->M);
  p->W_b = gsl_vector_alloc(p->M);
  p->W_x = gsl_vector_alloc(p->M);
  p->phiTR = gsl_matrix_alloc(p->M,p->N);
  p->Rho = gsl_vector_alloc(p->N);
  gsl_vector_set_all(p->Rho,1.0);

  p->phi = init_phi(0.1,p->K,p->M); //init phi with sigma = 0.1
  p->W = init_W_matlab(p->T,p->phi,p->K,p->M,p->N,p->D,&(p->beta));
  p->screening_spec.numUpdates = 2;
  p->screening_spec.numEMcycles = 5;
  p->screening_spec.switchEmptyBinCriterion = 300;
  p->screening_spec.numScreeningTrials = 120;
  p->screening_spec.flippingKicksIn = 20;
  p->screening_spec.minNumLoopsWithFullOccupancy = 3*p->screening_spec.numEMcycles;
  p->iter_info.maxIter = 3000;
  p->iter_info.loopID = 0;
  p->iter_info.numEmptyBins = gsl_vector_alloc(p->iter_info.maxIter);
  p->iter_info.newCycle = gsl_vector_alloc(p->iter_info.maxIter);
  gsl_vector_set_all(p->iter_info.numEmptyBins,-1);
  gsl_vector_set_all(p->iter_info.newCycle,-1);

  return p;
}

void gtm_Screening(Params * p){
  gsl_matrix * gtmT = p->T;
  gsl_matrix * gtmFI = p->phi;
  gsl_matrix * gtmW = p->W;
  gsl_vector * gtmRho = p->Rho;
  double gtmAlpha = p->alpha;
  double gtmBeta = p->beta;
  int numUpdates = p->screening_spec.numUpdates;
  int numEMcycles = p->screening_spec.numEMcycles;
  int switchEmptyBinCriterion = p->screening_spec.switchEmptyBinCriterion;
  int gtmN = p->T->size1;
  int gtmD = p->T->size2;
  int ND = gtmN*gtmD;
  int gtmK = gtmFI->size1;
  int gtmMplus1 = gtmFI->size2;
  int loopID =  p->iter_info.loopID;
  gsl_vector * numEmptyBins = p->iter_info.numEmptyBins;
  gsl_vector * newCycle = p->iter_info.newCycle;
  int flag = 1000;
  gsl_matrix * gtmR = p->R;
  int maxIter = p->iter_info.maxIter;
  int maxIterInc = 500;  
  gsl_vector * gtmminDist = gsl_vector_alloc(gtmN);
  gsl_vector * gtmmaxDist = gsl_vector_alloc(gtmN);
  //  [gtmDIST,gtmminDist,gtmmaxDist] = gtm_dist(gtmT,gtmFI*gtmW,2);
  gsl_matrix * gtmY = gsl_matrix_alloc(gtmK,gtmD);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,gtmFI,gtmW,0.0,gtmY);      
  gsl_matrix * gtmDIST = gsl_gtm_dist(gtmT,gtmY,gtmminDist,gtmmaxDist);
  for(int update = 0;update<numUpdates;update++){
    for(int cycle = 0;cycle<numEMcycles;cycle++){
      //      gtm_resp(gtmDIST,gtmminDist,gtmmaxDist,gtmBeta,gtmD,2,gtmR);
      double gtmllhLog = gsl_gtm_resp(gtmDIST,gtmminDist,gtmmaxDist,gtmBeta,gtmD,gtmR);
      /*
        gtmR = gtmR*diag(gtmRho);
        loopID = loopID+1;
        if loopID>maxIter
            numEmptyBins = [numEmptyBins; zeros(maxIterInc,1)]; %#ok<AGROW>
            newCycle = [newCycle; nan(maxIterInc,1)]; %#ok<AGROW>
            maxIter = maxIter+maxIterInc;
        end
        emptyBin = findEmptyBins(gtmR,loopID<switchEmptyBinCriterion);
        numEmptyBins(loopID) = numel(emptyBin);
        if (cycle==1), newCycle(loopID) = 1; end
        FGF = FI_T*spdiags(sum(gtmR,2),0,gtmK,gtmK)*gtmFI;
        A = full(FGF+ALPHA/gtmBeta);
        if any(isnan(A)|isinf(A))
			flag = 1006;
            iterInfo = {loopID,numEmptyBins,newCycle,emptyBin,flag};
            return
        end
        gtmW = pinv(A)*(FI_T*(gtmR*gtmT));
        [gtmDIST,gtmminDist,gtmmaxDist] = gtm_dist(gtmT,gtmFI*gtmW,2);
    end
      */ 
    }
  }  
}

gsl_matrix * gsl_matrix_from_bin(char * filename, int row, int col){
  gsl_matrix * A = gsl_matrix_alloc(row,col);
  FILE * f = fopen(filename,"r");
  for (int i = 0; i < col; i++) { 
    for(int j = 0;j<row;j++){
      double value;
      fread(&value,sizeof(value),1,f);
      gsl_matrix_set(A,j,i,value);
    }
  }
  return A;
}

 
double gsl_gtm_resp(gsl_matrix * DIST,gsl_vector * minDist, gsl_vector * maxDist,double beta, int D, gsl_matrix * _R){
  int K = DIST->size1;
  int N = DIST->size2;
  gsl_vector * distCorr;
  if(_R){
    // In calculation mode > 0, the distances between Gaussian centres
    // and data points are shifted towards being centred around zero,
    // w.r.t. the extreme (min- and max-) values.
    // Since the difference between any two distances is still the same, 
    // the responsabilities (R) will be the same. The advantage is that 
    // the risk of responsabilities dropping below zero (in finite precision) 
    // in the exponentiation below, due to large distances, is decreased.
    // However, while we CAN calculate with zero (0), we CAN'T calculate
    // with infinity (Inf). Hence, the shifting of distances must not be
    // so large that the exponentiation yields infinity as result.
    distCorr = gsl_vector_alloc(DIST->size2);
    for(int j = 0;j<DIST->size2;j++){
      gsl_vector_set(distCorr,j,(gsl_vector_get(maxDist,j) + gsl_vector_get(minDist,j))/2);
      // exp(709) < realmax < exp(710), plus a few digits margin to avoid
      // overflow when calculating rSum below.
      gsl_vector_set(distCorr,j,GSL_MIN(gsl_vector_get(distCorr,j),(gsl_vector_get(minDist,j)+700*(2/beta))));
      gsl_vector_view view = gsl_matrix_column(DIST,j);
      gsl_vector_add_constant(&view.vector,-gsl_vector_get(distCorr,j));
    }
  }
  gsl_matrix * R;
  if(_R){
    R = _R;
  }else{
    R = gsl_matrix_alloc(DIST->size1,DIST->size2);
  }
  for(int i = 0;i<R->size1;i++){
    for(int j = 0;j<R->size2;j++){
      gsl_matrix_set(R,i,j,exp((-beta/2)*gsl_matrix_get(DIST,i,j)));
    }
  }
  /* normalize columns of R */
  gsl_vector * R_col_sum = gsl_vector_alloc(R->size2);
  for(int j = 0;j<R->size2;j++){
    long double sum = 0;
    for(int i = 0;i<R->size1;i++){
      sum += gsl_matrix_get(R,i,j);
    }
    gsl_vector_set(R_col_sum,j,sum);
    gsl_vector_view view =  gsl_matrix_column(R,j);
    gsl_vector_scale(&view.vector,1/sum);
  }
  double llh = 0;
  if(_R){
    for(int i = 0;i<R_col_sum->size;i++){
      llh += log(gsl_vector_get(R_col_sum,i)) + gsl_vector_get(distCorr,i)*(-beta/2);
    }
    llh += N*((D/2.0)*log(beta/(2.0*M_PI)) - log(K));
  }else{
    for(int i = 0;i<R_col_sum->size;i++){
      llh += log(gsl_vector_get(R_col_sum,i));
    }
    llh += N*((D/2.0)*log(beta/(2*M_PI)) - log(K));
  }
  if(_R){
    gsl_vector_free(distCorr);
  }else{
    gsl_matrix_free(R);
  }
  gsl_vector_free(R_col_sum);
  return llh;
}

gsl_matrix * gsl_gtm_dist(gsl_matrix * A, gsl_matrix * B,gsl_vector * minDist, gsl_vector * maxDist){
  gsl_matrix * ret = gsl_matrix_alloc(B->size1,A->size1);
  if(A->size2 != B->size2){
    printf("number of cols must match!\n");
    return NULL;
  }
  for(int i = 0;i<A->size1;i++){
    for(int j = 0;j<B->size1;j++){
      double dist = 0;
      for(int k = 0;k<B->size2;k++){
	double tmp = gsl_matrix_get(A,i,k)-gsl_matrix_get(B,j,k);
	dist += tmp*tmp;	
      }
      gsl_matrix_set(ret,j,i,dist);
    }
  }
  if(minDist || maxDist){
    for(int j = 0;j<ret->size2;j++){
      gsl_vector_view view = gsl_matrix_column(ret,j);
      if(minDist){
	gsl_vector_set(minDist,j,gsl_vector_min(&view.vector));
      }
      if(maxDist){
	gsl_vector_set(maxDist,j,gsl_vector_max(&view.vector));
      }
    }
  }
  return ret;
}

double matrix_diag(gsl_matrix * A){
  int d = GSL_MIN(A->size1,A->size2);
  double sum = 0;
  for(int i = 0;i<d;i++){
    sum += gsl_matrix_get(A,i,i);
  }
  return sum;
}

double matrix_hash(const gsl_matrix * A){
  gsl_matrix * At = gsl_matrix_alloc(A->size2,A->size1);
  gsl_matrix * D = gsl_matrix_alloc(A->size1,A->size1);
  gsl_matrix_transpose_memcpy(At,A);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,A,At,0.0,D);    
  double ret = matrix_diag(D);
  gsl_matrix_free(At);
  gsl_matrix_free(D);
  return ret;
}

double vector_hash(const gsl_vector * V){
  double ret = 0;
  for(int i = 0;i<V->size;i++){
    ret += gsl_vector_get(V,i);
  }
  return ret;
}

void amnesic(int i,double * w1,double * w2){
  int n1=20;
  int n2=500;
  int m=1000;
  double L;
  if(i < n1){
    L=0;
  }else if( i >= n1 && i < n2){
    L=2*(i-n1)/(n2-n1);
  }else {
    L=2+(i-n2)/m;
  }
  *w1=(i-1-L)/i;    
  *w2=(1+L)/i;
}


gsl_matrix * gsl_pseudo_inverse(const gsl_matrix * A){
  gsl_matrix *U = gsl_matrix_alloc(A->size1,A->size2);
  gsl_matrix_memcpy(U,A);
  gsl_matrix *V = gsl_matrix_alloc(A->size2,A->size2);
  gsl_vector *S = gsl_vector_alloc(A->size2);
  
  gsl_vector * work = gsl_vector_alloc(A->size2);
  //    gsl_linalg_SV_decomp_jacobi(U, V, S);
  /* this kind of SVD is closer to the MATLAB even if less accurate */
  gsl_linalg_SV_decomp(U, V, S,work);

  /* tranpose V to really get V  */
  //  gsl_matrix_transpose(V);
  
  
  double tolerance = DBL_EPSILON*GSL_MAX(A->size1,A->size2)*gsl_vector_max(S);

  /* invert S */
  for (int i = 0; i < A->size2; i++) {
    if (fabs(gsl_vector_get(S,i)) > tolerance) {
      gsl_vector_set(S, i, 1.0/gsl_vector_get(S, i));
    }else{
      gsl_vector_set(S, i, 0);
    }
  }
  /* Do V*S */
  for(int i = 0;i<V->size2;i++){
    gsl_vector_view view = gsl_matrix_column(V,i);
    gsl_vector_scale(&view.vector,gsl_vector_get(S,i));
  }
  
  /* Do VS * U' */
  gsl_matrix * inv = gsl_matrix_alloc(A->size2,A->size1);
  gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,V,U,0.0,inv);    
  gsl_matrix * err = gsl_matrix_alloc(A->size2,A->size2);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,inv,A,0.0,err);  
  
  return inv;

}

void gsl_ccipca(const gsl_matrix * X,int k, int iteration, gsl_matrix * eigenvectors, gsl_vector * eigenvalues){
  /* I'm pretty sure this is correct now */
  /* number of times to update the eigenvector matrix */
  //  printf("Matrix hash of X = %1.20e\n",matrix_hash(X));
  int n = 2;
  int datadim = X->size1;
  int samplenum = X->size2;
  if(datadim > samplenum){
    fprintf(stderr,"No. of samples is less than the dimension. You have to choose how many eigenvectors to compute.\n");
    return;    
  }
  if(k > datadim){
    k = datadim;
  }
  int vectornum = k;
  int repeating = iteration;
  /* take the norm of the first k columns */
  gsl_vector * v_norm = gsl_vector_alloc(k);
  for(int i = 0;i<k;i++){
    gsl_vector_const_view view = gsl_matrix_const_column(X,i);
    gsl_vector_set(v_norm,i,gsl_blas_dnrm2(&view.vector));
  }
  //  printf("Vnorm hash = %1.20e\n",vector_hash(v_norm));
  /* initialize eigenvectors */
  gsl_matrix * V = eigenvectors;
  for(int i = 0;i<datadim;i++){
    for(int j = 0;j<vectornum;j++){
      gsl_matrix_set(V,i,j,gsl_matrix_get(X,i,j));
    }
  }
  
  printf("V hash = %1.20e\n",matrix_hash(V));
  gsl_vector * residue = gsl_vector_alloc(datadim);
  for(int iter = 0;iter < repeating;iter++){
    for(int i = 0;i<samplenum;i++){
      /* set residue */
      gsl_vector_const_view X_col = gsl_matrix_const_column(X,i);
      gsl_vector_memcpy(residue,&X_col.vector);

      double w1,w2;
      amnesic(n,&w1,&w2);
      n=n+1;
      for(int j= 0;j<vectornum;j++){
	/* first calculate this vector times the residue */
	double w2_V_residue = 0;
	gsl_vector_view V_col = gsl_matrix_column(V,j);
	gsl_blas_ddot(&V_col.vector,residue,&w2_V_residue);
	w2_V_residue *= w2;

	//	printf("w2*V'*residue = %1.20e\n",w2_V_residue);

	/* update V */
	// V(: , j) = w1 * V(:,j) + w2 * V(:,j)' * residue * residue / Vnorm(j);
	gsl_vector_scale(&V_col.vector,w1);
	gsl_blas_daxpy(w2_V_residue/gsl_vector_get(v_norm,j),residue,&V_col.vector);
	// Vnorm(j)=norm(V(:,j));
	gsl_vector_set(v_norm,j,gsl_blas_dnrm2(&V_col.vector));
	
	/* calculate residue*normedV*/
	gsl_vector * normedV = gsl_vector_alloc(V->size1);
	gsl_vector_memcpy(normedV,&V_col.vector);
	gsl_vector_scale(normedV,1.0/gsl_vector_get(v_norm,j));

	//residue = residue - residue' * normedV * normedV; 
	double residue_normedV;
	gsl_blas_ddot(residue,normedV,&residue_normedV);
	gsl_vector_scale(normedV,residue_normedV);	
	gsl_vector_sub(residue,normedV);
	gsl_vector_free(normedV);
      }
    }
  }
  
  /* Calculate the length of the eigen vectors (eigenvalues)*/
  gsl_vector * D = eigenvalues;
  for(int j = 0;j<vectornum;j++){
    gsl_vector_const_view V_col = gsl_matrix_const_column(V,j);
    gsl_vector_set(D,j,gsl_blas_dnrm2(&V_col.vector));
  }
  /* sort eigenvalues and eigenvectors in decreasing order */
  /* I'm gonna go with bubble sort again */
  /* sort variances in decreasing order using good old bubble sort */
  for (int i=0; i<vectornum; i++) {
    for (int j=0; j<vectornum-1-i; j++){
      if (gsl_vector_get(D,j+1) > gsl_vector_get(D,j)) {  /* compare the two neighbors */
	/* swap a[j] and a[j+1]      */
	gsl_vector_swap_elements(D,j,j+1);
	/* this is not really efficient but for now it's probably ok */
	gsl_matrix_swap_columns(V,j,j+1);
      }
    }
  }
  /* normalize eigenvectors*/
  for(int j = 0;j<vectornum;j++){
    gsl_vector_view V_col = gsl_matrix_column(V,j);
    gsl_vector_scale(&V_col.vector,1.0/gsl_vector_get(D,j));
  }
}

 

int int_hash(int * v, int n){
  long long res = 0;
  for(int i = 0;i<n;i++){
    res += (v[i]+1)*(i+1);
  }
  return res;
}


void write_gsl_matrix(gsl_matrix *m, char *filename)
{
  int i1,i2;
  FILE *f = fopen(filename,"wp");
  for (i1 = 0; i1 < m->size1; i1++) {
    for (i2 = 0; i2 < m->size2; i2++) {
      fprintf(f,"%g ",gsl_matrix_get(m,i1,i2));
    }
    fprintf(f,"\n");
  }
  fclose(f);
}

void save_gsl_matrix(gsl_matrix *m, char *filename)
{
  int i1,i2;
  Image *f = sp_image_alloc(m->size1,m->size2,1);
  for (i1 = 0; i1 < m->size1; i1++) {
    for (i2 = 0; i2 < m->size2; i2++) {
      sp_image_set(f,i1,i2,0,sp_cinit(gsl_matrix_get(m,i1,i2),0.0));
    }
  }
  sp_image_write(f,filename,0);
  sp_image_free(f);
}

gsl_matrix *read_gsl_matrix(char *filename)
{
  int i1,i2;
  Image *f = sp_image_read(filename,0);
  gsl_matrix *m = gsl_matrix_alloc(sp_image_x(f),sp_image_y(f));
  for (i1 = 0; i1 < m->size1; i1++) {
    for (i2 = 0; i2 < m->size2; i2++) {
      gsl_matrix_set(m,i1,i2,sp_real(sp_image_get(f,i1,i2,0)));
    }
  }
  sp_image_free(f);
  return m;
}


/* Read directly from the double precision data written out from matlab */
void init_data(char *dirname,char *file, Params * p,int image_size) //_T
{
  int n;
  double * variance = malloc(sizeof(double)*image_size);
  int * pixels_by_variance = malloc(sizeof(int)*image_size);
  int * good_pixels = malloc(sizeof(int)*image_size);
  gsl_matrix * myData = gsl_matrix_alloc(p->N,image_size);
  FILE * f = fopen("abbas_data.bin","r");
  for (n = 0; n < p->N; n++) { 
    for(int i = 0;i<image_size;i++){
      double value;
      fread(&value,sizeof(value),1,f);
      gsl_matrix_set(myData,n,i,value);
    }
  }
  /* Calculate pixel variance for each pixel */
  for(int i = 0;i<image_size;i++){
    int n = 0;
    double sum1 = 0;
    for(int j = 0;j<p->N;j++){
      n++;
      sum1 = sum1 + gsl_matrix_get(myData,j,i);
    }
    double mean = sum1/n;

    double sum2 = 0;
    for(int j = 0;j<p->N;j++){
      double err = gsl_matrix_get(myData,j,i) - mean;
      sum2 += err*err;
    }
    variance[i] =  sum2/(n - 1);
  }

  for(int i = 0;i<image_size;i++){
    pixels_by_variance[i] = i;
  }
  /* sort variances in decreasing order using good old bubble sort */
  for (int i=0; i<image_size-1; i++) {
    for (int j=0; j<image_size-1-i; j++){
      if (variance[j+1] > variance[j]) {  /* compare the two neighbors */
	/* swap a[j] and a[j+1]      */
	double tmp = variance[j];         
	variance[j] = variance[j+1];
	variance[j+1] = tmp;
	/* rearrange pixels by variance also */
	int tmpi = pixels_by_variance[j];
	pixels_by_variance[j] = pixels_by_variance[j+1];
	pixels_by_variance[j+1] = tmpi;
      }
    }
  }

  /* Calculate the good pixels */
  /* the good pixels must be the 1300 px with the highest variance 3 */
  int n_good_pixels = 0;

  for(int i= 0;i<1300;i++){
    int index = pixels_by_variance[i];
    int x = index/40-19;
    int y = index%40-19;
    /* the good pixels can't be inside a beamstop of radius 3 */
    if(x*x+y*y > 3*3){
      good_pixels[n_good_pixels++] = index;
    }
  }
  p->D = n_good_pixels;

  /* Sort good_pixels by increasing order to match matlab */
  for (int i=0; i<n_good_pixels-1; i++) {
    for (int j=0; j<n_good_pixels-1-i; j++){
      if (good_pixels[j+1] < good_pixels[j]) {  /* compare the two neighbors */
	/* swap a[j] and a[j+1]      */
	int tmp = good_pixels[j];         
	good_pixels[j] = good_pixels[j+1];
	good_pixels[j+1] = tmp;
      }
    }
  }
  printf("mask hash = %d\n",int_hash(good_pixels,n_good_pixels));
  
  p->T = gsl_matrix_alloc(p->N,p->D);
  
  for (n = 0; n < p->N; n++) {   
    for (int i = 0; i < n_good_pixels; i++) {
      double tmp = gsl_matrix_get(myData,n,good_pixels[i]);
      gsl_matrix_set(p->T,n,i,tmp);
    }
  }  
}

/* Alloc and fill phi.
 */
gsl_matrix * init_phi(double sigma, int K, int M)
{
  int m,k;
  double sum, tmp;
  gsl_matrix * phi = gsl_matrix_alloc(K,M+1);

  for (k = 0; k < K; k++) {
    sum = 0.0;
    for(m = 0; m < M; m++) {
      if ((double) k / (double) K - (double) m / (double) M > 0.5) {
	tmp = exp(-pow(((double) k / (double) K - 1.0 - (double) m / (double) M),2)/2.0/sigma/sigma);	
      } else if ((double) k / (double) K - (double) m / (double) M < -0.5) {
	tmp = exp(-pow(((double) k / (double) K + 1.0 - (double) m / (double) M),2)/2.0/sigma/sigma);      
      } else {
	tmp = exp(-pow(((double) k / (double) K - (double) m / (double) M),2)/2.0/sigma/sigma);
      }
      gsl_matrix_set(phi,k,m,tmp);
      sum += tmp;
    }
    /* normalize
    */
    /*    for (m = 0; m < M; m++) {
      gsl_matrix_set(phi,k,m,gsl_matrix_get(phi,k,m)/sum);
      }*/
  }

  /* set last column of ones */
  for (k = 0; k < K; k++) {
    gsl_matrix_set(phi,k,M,1);
  }

  /* read from matlab */
  phi = gsl_matrix_from_bin("phi.bin",120,61);
  return phi;
}

/*

void init_W_pca2()
{
  int d1,d2,n,k;
  W = gsl_matrix_alloc(M,D);
  gsl_matrix *S = gsl_matrix_alloc(D,D);
  gsl_vector *mean = gsl_vector_alloc(D);
  gsl_matrix *T_pca = gsl_matrix_alloc(N,D);

  gsl_matrix *U_pca = gsl_matrix_alloc(D,D);
  gsl_vector *L_pca = gsl_vector_alloc(D);
  gsl_vector *work_pca = gsl_vector_alloc(D);

  int time1,time2;

  time1 = clock();
  for (d1 = 0; d1 < D; d1++) {
    gsl_vector_set(mean,d1,0.0);
    for (n = 0; n < N; n++) {
      gsl_vector_set(mean,d1,gsl_vector_get(mean,d1) +
		     gsl_matrix_get(T,n,d1));
    }
  }
  gsl_vector_scale(mean, 1.0 / (double) N);
  time2 = clock();
  printf("pca loop1: %g sec\n",
	 (double) (time2-time1) / (double) CLOCKS_PER_SEC);

  time1 = clock();
  for (d1 = 0; d1 < D; d1++) {
    if (d1 %100 == 0) printf("%d (%d)\n",d1,D);
    for (d2 = 0; d2 < D; d2++) {
      gsl_matrix_set(S,d1,d2,0.0);
      for (n = 0; n < N; n++) {
	gsl_matrix_set(S,d1,d2,gsl_matrix_get(S,d1,d2) +
		       (gsl_matrix_get(T,n,d1) - gsl_vector_get(mean,d1)) *
		       (gsl_matrix_get(T,n,d2) - gsl_vector_get(mean,d2)));
      }
    }
  }
  gsl_matrix_scale(S,1.0/(double) (N-1));
  time2 = clock();
  printf("pca loop2: %g sec\n",
	 (double) (time2-time1) / (double) CLOCKS_PER_SEC);

  time1 = clock();
  gsl_linalg_SV_decomp(S, U_pca, L_pca, work_pca);

  //gsl_linalg_SV_decomp_jacobi(S, U_pca, L_pca);
  time2 = clock();
  printf("pca sv_decomp: %g sec\n",
	 (double) (time2-time1) / (double) CLOCKS_PER_SEC);

  time1 = clock();
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,T,S,0.0,T_pca);
  time2 = clock();
  printf("pca matrix mul: %g sec\n",
	 (double) (time2-time1) / (double) CLOCKS_PER_SEC);

  time1 = clock();
  for (k = 0; k < K; k++) {
    if (k%10 == 0) printf("%d (%d)\n",k,K);
    for (d1 = 0; d1 < D; d1++) {
      gsl_matrix_set(Y,k,d1,sqrt(gsl_vector_get(L_pca,0)) *
		     gsl_matrix_get(U_pca,d1,0) *
		     cos(2.0 * M_PI * (double) k / (double) K) +
		     sqrt(gsl_vector_get(L_pca,1)) *
		     gsl_matrix_get(U_pca,d1,1) *
		     sin(2.0 * M_PI * (double) k / (double) K) +
		     gsl_vector_get(mean,d1));
      
    }
  }
  time2 = clock();
  printf("pca loop3: %g sec\n",
	 (double) (time2-time1) / (double) CLOCKS_PER_SEC);

  //left inverse

  gsl_matrix *phi_inverse = gsl_matrix_alloc(M,K);
  gsl_matrix *phiTphi_inverse = gsl_matrix_alloc(M,M);
  gsl_matrix *phiTphi = gsl_matrix_alloc(M,M);

  gsl_permutation *perm = gsl_permutation_alloc(M);
  

  gsl_matrix *V_inverse = gsl_matrix_alloc(M,M);
  gsl_vector *S_inverse = gsl_vector_alloc(M);
  gsl_vector *work_inverse = gsl_vector_alloc(M);
  
  gsl_matrix *phi_svd = gsl_matrix_alloc(K,M);
  gsl_matrix_memcpy(phi_svd,phi);
  gsl_linalg_SV_decomp(phi, V_inverse, S_inverse, work_inverse);

  int m1,m2;  
  for (m1 = 0; m1 < M; m1++) {
    if (gsl_vector_get(S_inverse,m1) != 0.0) {
      gsl_vector_set(S_inverse, m1, 1.0/gsl_vector_get(S_inverse, m1));
    }
  }

  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++) {
      gsl_matrix_set(V_inverse,m1,m2,
		     gsl_matrix_get(V_inverse,m1,m2)*
		     gsl_vector_get(S_inverse,m2));
    }
  }
  gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,V_inverse,phi_svd,0.0,phi_inverse);


  gsl_matrix *id = gsl_matrix_alloc(M,M);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,phi_inverse,phi,0.0,id);

  int m;
  for (m = 0; m < M; m++) {
    //for (k = 0; k < K; k++) {
    printf("%d: %g\n",m,gsl_matrix_get(id,m,m));
  }

  gsl_matrix_free(phi_inverse);
  gsl_matrix_free(phiTphi_inverse);
  gsl_matrix_free(phiTphi);
  gsl_permutation_free(perm);


  gsl_matrix_free(S);
  gsl_vector_free(mean);
  gsl_matrix_free(T_pca);
  gsl_matrix_free(U_pca);
  gsl_vector_free(L_pca);
  gsl_vector_free(work_pca);
}

*/

gsl_matrix * init_W_matlab(const gsl_matrix * T,const gsl_matrix * phi, const int K,const int M,const int N,const int D, double * beta){
  gsl_matrix * W = gsl_matrix_alloc(M+1,D);
  gsl_vector * image_mean = gsl_vector_alloc(D);
  int L_pca = 4;
  gsl_matrix * eVts = gsl_matrix_alloc(D,L_pca+1);
  gsl_vector * eVls = gsl_vector_alloc(L_pca+1);
  gsl_matrix * T_transpose = gsl_matrix_alloc(T->size2,T->size1);
  /* subtract the mean value for each pixel*/
  for(int i = 0;i<D;i++){
    double mean = 0;
    for(int j = 0;j<N;j++){
      mean += gsl_matrix_get(T,j,i);      
    }
    mean /= N;
    gsl_vector_set(image_mean,i,mean);
  }
  for(int i = 0;i<D;i++){
    for(int j = 0;j<N;j++){
      gsl_matrix_set(T_transpose,i,j,gsl_matrix_get(T,j,i)-gsl_vector_get(image_mean,i));
    }
  }  
  gsl_matrix * A = gsl_matrix_alloc(D,L_pca);
  gsl_ccipca(T_transpose,L_pca+1,15,eVts,eVls);

  /*
  T_transpose = gsl_matrix_from_bin("T_trans.bin",1276,3000);
  

  for(int i =0;i<D;i++){
    for(int j = 0;j<L_pca;j++){
      gsl_matrix_set(A,i,j,gsl_matrix_get(eVts,i,j)*sqrt(gsl_vector_get(eVls,j)));
    }
  }
  */

  /* Numerical innacuracies in ccipca make it diverge from the matlab result */
  A = gsl_matrix_from_bin("A_pci.bin",1276,4);
  gsl_matrix * projection = gsl_matrix_alloc(N,L_pca);
  gsl_matrix * A_inv = gsl_pseudo_inverse(A);
  //  gsl_matrix * A_inv = gsl_matrix_from_bin("Ainv.bin",4,1276);
  gsl_blas_dgemm(CblasTrans,CblasTrans,1.0,T_transpose,A_inv,0.0,projection);  
  gsl_matrix * sqDist = gsl_gtm_dist(projection,projection,NULL,NULL);
  /* Initialize PCA_Trace with the first row of projection */
  gsl_matrix * PCA_Trace = gsl_matrix_alloc(K,L_pca);
  for(int i = 0;i<PCA_Trace->size1;i++){
    for(int j = 0;j<PCA_Trace->size2;j++){
      if(i == 0){
	gsl_matrix_set(PCA_Trace,i,j,gsl_matrix_get(projection,0,j));
      }else{
	gsl_matrix_set(PCA_Trace,i,j,0);
      }
    }
  }
  char * off_limit_list = malloc(sizeof(char)*sqDist->size1);
  for(int i = 0;i<sqDist->size1;i++){
    off_limit_list[i] = 0;
  }
  int ptr0 = 0;
  off_limit_list[ptr0] = 1;
  int skip = 2*floor(N/K); 
  double extraSkipProb = (double)(N%K)/K;
  const gsl_rng_type * Type=gsl_rng_default;
  gsl_rng * rng=gsl_rng_alloc (Type);

  int ptr1;
  for(int j = 1;j<K;j++){
    int points_to_skip = skip;
    if(gsl_rng_uniform(rng) < extraSkipProb){
      points_to_skip++;
    }
    /* find the points_to_skip closest points to point ptr0 */
    gsl_vector_view view = gsl_matrix_row(sqDist,ptr0);
    gsl_permutation * id = gsl_permutation_alloc(sqDist->size1);
    gsl_sort_vector_index(id,&view.vector);
    for(int i = 0;i<points_to_skip;i++){
      off_limit_list[gsl_permutation_get(id,i)] = 1;
    }
    /* Find the first id that is not on the off_limit_list */
    int candidate = -1;
    for(int i = points_to_skip;i<sqDist->size1;i++){
      if(off_limit_list[gsl_permutation_get(id,i)] == 0){
	candidate = gsl_permutation_get(id,i);
	break;
      }
    }
    if(candidate != -1){
      ptr1 = candidate;
    }else{
      /* No candidates found */
      /* take a random one from the skipped points */
      int index = floor(gsl_rng_uniform(rng)*points_to_skip);
      ptr1 = gsl_permutation_get(id,index);
    }
    view = gsl_matrix_row(projection,ptr1);
    gsl_matrix_set_row(PCA_Trace,j,&(view.vector));
    ptr0 = ptr1;
  }
  /* normalise X to ensure 1:1 mapping of variances and calculate W
     as the solution of the equation: FI*W = normX*A' */

  for(int i = 0;i<L_pca;i++){
    gsl_vector_view view = gsl_matrix_column(PCA_Trace,i);
    double mean = gsl_stats_mean((&(view.vector))->data,(&(view.vector))->stride,(&(view.vector))->size);
    double std = gsl_stats_sd((&(view.vector))->data,(&(view.vector))->stride,(&(view.vector))->size);
    gsl_vector_add_constant(&(view.vector),-mean);
    gsl_vector_scale(&(view.vector),1.0/std);
  }

  /* calculate W*/
  //W = pinv(FI)*(normX*A');
  gsl_matrix * normX_At = gsl_matrix_alloc(PCA_Trace->size1,A->size1);
  gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,PCA_Trace,A,0.0,normX_At);
  gsl_matrix * phi_inv = gsl_pseudo_inverse(phi);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,phi_inv,normX_At,0.0,W);
  

  // add data mean (removed by gtm_pca)
  // W(Mplus1,:) = W(Mplus1,:)+mean(T);
  /* Don't do this because matlab doesn't do also */

  /*  for(int i = 0;i<D;i++){
    gsl_vector_const_view cview = gsl_matrix_const_column(T,i);
    double mean = gsl_stats_mean((&(cview.vector))->data,(&(cview.vector))->stride,(&(cview.vector))->size);
    gsl_matrix_set(W,M,i,gsl_matrix_get(W,M,i)+mean);
  }
  */

  /* initialize beta */ 
  *beta = 1.0/gsl_vector_get(eVls,L_pca);
  return W;
}

/* Calculates Y from W and phi.
 */
void calculate_Y(const gsl_matrix * phi,const gsl_matrix * W, gsl_matrix * Y)
{
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,phi,W,0.0,Y);
}

int factorial(int value)
{
  int i;
  int ret = 1;
  for (i = 2; i <= value; i++) {
    ret *= i;
  }
  return ret;
}

/* Calculates log(value). The function is slow but can
   handle large values. This should be tabulated.
*/
double log_factorial(int value)
{
  int i;
  double ret = 0;
  for (i = 2; i <= value; i++) {
    ret += log(i);
  }
  return ret;
}

/* Calculates R from Y and T.
 */
void calculate_R(const int D,const int K,const int N,const gsl_matrix * Y,const gsl_matrix * T, gsl_matrix * delta,gsl_matrix * R,const double beta)
{
  int k,n,d;

  for (k = 0; k < K; k++) {
    for (n = 0; n < N; n++) {
      gsl_matrix_set(delta,k,n,0.0);
      for (d = 0; d < D; d++) {
	gsl_matrix_set(delta,k,n,gsl_matrix_get(delta,k,n) +
		       pow(gsl_matrix_get(T,n,d)-gsl_matrix_get(Y,k,d),2));
      }
    }
  }

  double sum;
  for (n = 0; n < N; n++) {
    sum = 0.0;
    for (k = 0; k < K; k++) {
      /* the prefactor can be omitted since we normalize later on
      gsl_matrix_set(R,k,n,pow((beta/2.0/M_PI),-(double)D/2.0)*
		     exp(-beta/2.0*gsl_matrix_get(delta,k,n))/(double) K);
      */
      gsl_matrix_set(R,k,n,exp(-beta/2.0*gsl_matrix_get(delta,k,n))/(double) K);
      sum += gsl_matrix_get(R,k,n);
    }
    if (sum > 0.0)  {
      for (k = 0; k < K; k++) {
	gsl_matrix_set(R,k,n,gsl_matrix_get(R,k,n)/sum);
      }
    } else {
      for (k = 0; k < K; k++) {
	gsl_matrix_set(R,k,n,0.0);
      }
    }
  }
}

void calculate_R_no_beta(int D,int K,int N,gsl_matrix * T,gsl_matrix * Y,gsl_matrix * R){
  int n,k,d;
  double sum,resp;
  for (n = 0; n < N; n++) {
    sum = 0.0;
    for (k = 0; k < K; k++) {
      resp = 0.0;
      for (d = 0; d < D; d++) {
	if (gsl_matrix_get(Y,k,d) > 0.0) {
	  resp -= pow(gsl_matrix_get(T,n,d)-gsl_matrix_get(Y,k,d),2)/
	    2.0/gsl_matrix_get(Y,k,d)/(double) K;
	}
	//printf("%g %g\n",resp,gsl_matrix_get(Y,k,d));
      }
      gsl_matrix_set(R,k,n,exp(resp));
      sum += gsl_matrix_get(R,k,n);
    }
    sum  = 1.0 / sum;
    for (k = 0; k < K; k++) {
      gsl_matrix_set(R,k,n,gsl_matrix_get(R,k,n) * sum);
    }
  }
}

void calculate_G(const int K,const int N,gsl_vector * G,const gsl_matrix * R)
{
  int k,n;
  for (k = 0; k < K; k++) {
    gsl_vector_set(G,k,0.0);
    for (n = 0; n < N; n++) {
      gsl_vector_set(G,k,gsl_vector_get(G,k) + gsl_matrix_get(R,k,n));
    }
  }
}

void calculate_W(const int K,const int M,const gsl_vector * G,gsl_vector * W_b,gsl_vector * W_x,
		 const gsl_matrix * phi,gsl_matrix * phiTGphi,gsl_matrix * inverse,
		 const gsl_matrix * R,gsl_matrix * phiTR,const gsl_matrix * T, gsl_matrix * W,
		 double alpha,double beta)
{
  int m1,m2,k;

  /* calculate phiT * G * phi */

  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++) {
      gsl_matrix_set(phiTGphi,m1,m2,0.0);
      for (k = 0; k < K; k++) {
	gsl_matrix_set(phiTGphi,m1,m2,gsl_matrix_get(phiTGphi,m1,m2) +
		       gsl_matrix_get(phi,k,m1) * gsl_vector_get(G,k) *
		       gsl_matrix_get(phi,k,m2));
      }
    }
  }
  // phiTGphi is symmetric (checked)


  for (m1 = 0; m1 < M; m1++) {
    gsl_matrix_set(phiTGphi,m1,m1,gsl_matrix_get(phiTGphi,m1,m1) +
		   alpha/beta);
    /* without beta
    gsl_matrix_set(phiTGphi,m1,m1,gsl_matrix_get(phiTGphi,m1,m1));
    */
  }
  
  //gsl_permutation_init(perm_gsl);

  if (gsl_linalg_cholesky_decomp(phiTGphi) != 0)
    printf("cholesky decomp return != 0\n");

  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++)
      gsl_vector_set(W_b,m2,0.0);
    gsl_vector_set(W_b,m1,1.0);
    gsl_linalg_cholesky_solve(phiTGphi,W_b,W_x);
    for (m2 = 0; m2 < M; m2++) 
      gsl_matrix_set(inverse,m2,m1,gsl_vector_get(W_x,m2));
  }

  
  /* cholesky stuff, hopefully not needed
  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++) {
      inverse[m1][m2] = 0.0;
    }
  }

  for (m1 = M-1; m1 >= 0; m1--) {
    inverse[m1][m1] = 1.0 / gsl_matrix_get(&phiTGphi_gsl.matrix,m1,m1);
    for (m2 = m1+1; m2 < M; m2++) {
      for (m3 = m1+1; m3 < M; m3++) {
	inverse[m2][m1] += inverse[m2][m3] * gsl_matrix_get(&phiTGphi_gsl.matrix,m3,m1);
      }
    }
    for (m2 = m1+1; m2 < M; m2++) {
      inverse[m2][m1] = -inverse[m1][m1]*inverse[m2][m1];
    }
  }
  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++) {
      tmp = 0.0;
      for (m3 = 0; m3 < M; m3++) { 
	tmp += inverse[m1][m3] * inverse[m2][m3];
      }
      gsl_matrix_set(inverse_gsl,m1,m2,tmp);
    }
  }
  */

  //print the matrix * inverse diagonal (should be only ones (not the case))
  /*
  double unit[M][M];
  for (m1 = 0; m1 < M; m1++) {
    for (m2 = 0; m2 < M; m2++) {
      unit[m1][m2] = 0.0;
      for (m3 = 0; m3 < M; m3++) {
	unit[m1][m2] += phiTGphi[m1][m3] * gsl_matrix_get(inverse_gsl,m3,m2);
      }
    }
  }
  for (m1 = 1; m1 < M-1; m1++) {
    printf("%g ",unit[m1][m1]);
  }
  printf("\n");
  */
  /*
  for (m1 = 0; m1 < M; m1++) {
    tmp = 1.0/phiTGphi[m1][m1];
    for (m2 = 0; m2 < M; m2++) {
      phiTGphi[m1][m2] *= tmp;
      inverse[m1][m2] *= tmp;
    }
    for (m2 = m1+1; m2 < M; m2++) {
      tmp = -phiTGphi[m2][m1];
      for (m3 = 0; m3 < M; m3++) {
	phiTGphi[m2][m3] += phiTGphi[m1][m3]*tmp;
	inverse[m2][m3] += inverse[m1][m3]*tmp;
      }
    }
  }

  for (m1 = M-1; m1 >= 0; m1--) {
    tmp = 1.0/phiTGphi[m1][m1];
    for (m2 = 0; m2 < M; m2++) {
      phiTGphi[m1][m2] *= tmp;
      inverse[m1][m2] *= tmp;
    }
    for (m2 = m1-1; m2 >= 0; m2--) {
      tmp = -phiTGphi[m2][m1];
      for (m3 = 0; m3 < M; m3++) {
	phiTGphi[m2][m3] += phiTGphi[m1][m3]*tmp;
	inverse[m2][m3] += inverse[m1][m3]*tmp;
      }
    }
  }
  */

  gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,phi,R,0.0,phiTR);

  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,inverse,phiTR,0.0,phiTR);

  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,phiTR,T,0.0,W);
}

void init_beta(int K,int D,gsl_matrix * Y,double beta)
{
  int k1,k2,d;
  double dist,min;
  double beta_inv = 0.0;
  for (k1 = 0; k1 < K; k1++) {
    min = 1e10;
    for (k2 = 0; k2 < K; k2++) {
      if (k1 != k2) {
	dist = 0.0;
	for (d = 0; d < D; d++) {
	  dist += pow(gsl_matrix_get(Y,k1,d)-gsl_matrix_get(Y,k2,d),2);
	}
	if (dist < min) {
	  min = dist;
	}
      }
    }
    beta_inv += min;
  }
  beta_inv /= (double) K;
  beta = 1.0/beta_inv;
  printf("beta = %g\n",beta);
}
 
double calculate_beta(int D,int N,int K,gsl_matrix * R,gsl_matrix * delta)
{
  int n,k;
  double beta_inv = 0.0;
  double beta;

  for (n = 0; n < N; n++) {
    for (k = 0; k < K; k++) {
      beta_inv += gsl_matrix_get(R,k,n) * gsl_matrix_get(delta,k,n);
    }
  }
  printf("beta_inv = %g\n",beta_inv);
  beta_inv /= (double) (N*D);// and D
  printf("after scaling = %g\n",beta_inv);
  beta = 1.0/beta_inv;
  return beta;
}

/* Run with argument "1" to use a saved starting W.
 */
int main(int argc, char **argv)
{
  Params * p = init_params();
  gtm_Screening(p);
  calculate_Y(p->phi,p->W,p->Y);
  init_beta(p->K,p->D,p->Y,p->beta);
  calculate_R_no_beta(p->D,p->K,p->N,p->T,p->Y,p->R);


  write_gsl_matrix(p->T,"T.data");
  write_gsl_matrix(p->phi,"phi.data");
  write_gsl_matrix(p->W,"W.data");
  write_gsl_matrix(p->Y,"Y.data");
  write_gsl_matrix(p->R,"R.data");
  write_gsl_matrix(p->delta,"delta.data");

  int i;
  char buffer[1000];
  for (i = 0; i < 100; i++) {
    printf("%d\n",i);

    calculate_G(p->K,p->N,p->G,p->R);
    calculate_W(p->K,p->M,p->G,p->W_b,p->W_x,p->phi,p->phiTGphi,p->inverse,p->R,p->phiTR,p->T,p->W,p->alpha,p->beta);
    //calculate_beta();
    //beta = 0.5;
    printf("beta = %g\n",p->beta);
    sprintf(buffer,"W_%d.data",i);
    write_gsl_matrix(p->W,buffer);
    
    sprintf(buffer,"delta_%d.data",i);
    write_gsl_matrix(p->delta,buffer);

    calculate_Y(p->phi,p->W,p->Y);
    sprintf(buffer,"Y_%d.data",i);
    write_gsl_matrix(p->Y,buffer);

    calculate_R_no_beta(p->D,p->K,p->N,p->T,p->Y,p->R);
    sprintf(buffer,"R_%d.data",i);
    write_gsl_matrix(p->R,buffer);
  }
}
