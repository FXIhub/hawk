#define _POSIX_C_SOURCE  199309L
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
#include <gsl/gsl_eigen.h>
#include "gtm.h"


  
Params * init_params(){
  Params * p = malloc(sizeof(Params));
  p->N = 3000;
  p->K = 120;
  p->M = 60;
  p->L = 1;
  p->beta = 0.001;
  p->alpha = 0.01;
  init_data(p,1600);

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
  p->screening_spec.flippingKicksIn = 0;
  p->screening_spec.minNumLoopsWithFullOccupancy = 3*p->screening_spec.numEMcycles;
  p->iter_info.maxIter = 3000;
  p->iter_info.loopID = 0;
  p->iter_info.numEmptyBins = gsl_vector_alloc(p->iter_info.maxIter);
  p->iter_info.newCycle = gsl_vector_alloc(p->iter_info.maxIter);
  p->iter_info.emptyBin = NULL;
  gsl_vector_set_all(p->iter_info.numEmptyBins,-1);
  gsl_vector_set_all(p->iter_info.newCycle,-1);

  p->gtmSamples = gsl_matrix_alloc(p->N,1);
  p->gtmSamples = gsl_matrix_from_bin("gtmSamples.bin",p->N,1);

  return p;
}

gsl_vector * find_empty_bins(const gsl_matrix * gtmR,int loopID, int switchEmptyBinCriterion){

  int K = gtmR->size1;
  int N = gtmR->size2;
  gsl_vector * occupancies = gsl_vector_alloc(gtmR->size1);
  gsl_vector_set_all(occupancies,0);
  gsl_vector * low_occupancy = gsl_vector_alloc(gtmR->size1);
  gsl_vector_set_all(low_occupancy,0);

  /* find the points that are responsible for each sample */
  for(unsigned int j = 0;j<gtmR->size2;j++){
    gsl_vector_const_view view = gsl_matrix_const_column(gtmR,j);
    size_t point = gsl_vector_max_index(&view.vector);
    gsl_vector_set(occupancies,point,gsl_vector_get(occupancies,point)+1);
  }
  for(unsigned int i = 0;i<gtmR->size1;i++){
    if(gsl_vector_get(occupancies,i) < 2){
      gsl_vector_set(low_occupancy,i,1);
    }
  }

  gsl_vector * weird_occupancy;
  if(loopID<switchEmptyBinCriterion){
    weird_occupancy = gsl_vector_alloc(gtmR->size1);
    double mu0 = (double)N/K;
    double sigma0 = sqrt(mu0);
    gsl_vector * z_less_than_minus_one = gsl_vector_alloc(gtmR->size1);
    for(unsigned int i = 0;i<gtmR->size1;i++){
      double z = (gsl_vector_get(occupancies,i)-mu0)/sigma0;
      gsl_vector_set(z_less_than_minus_one,i,z<-1);
    }
    for(unsigned int i = 0;i<gtmR->size1;i++){
      if(gsl_vector_get(z_less_than_minus_one,i)){
	/* there's a bug in the original matlab code that prevents it from checking the neighbours 
	
	   if(gsl_vector_get(z_less_than_minus_one,i) &&
	   gsl_vector_get(z_less_than_minus_one,(i-1+gtmR->size1)%gtmR->size1) &&
	   gsl_vector_get(z_less_than_minus_one,(i-2+gtmR->size1)%gtmR->size1) &&
	   gsl_vector_get(z_less_than_minus_one,(i+1+gtmR->size1)%gtmR->size1) &&
	   gsl_vector_get(z_less_than_minus_one,(i+2+gtmR->size1)%gtmR->size1)){*/
	gsl_vector_set(weird_occupancy,i,1);
      }else{
	gsl_vector_set(weird_occupancy,i,0);
      }
    }
  }

  /* return the indexes of the points that have low or weird occupancy */
  int ret_size = 0;
  gsl_vector * ret;
  int index = 0;
  for(unsigned int i= 0;i<low_occupancy->size;i++){
    if(gsl_vector_get(low_occupancy,i) != 0 || 
       (loopID<switchEmptyBinCriterion && gsl_vector_get(weird_occupancy,i) != 0)){
      ret_size++;
      index++;
    }
  }
  ret = gsl_vector_alloc(ret_size);
  gsl_vector_set_all(ret,0);
  
  index = 0;
  for(unsigned int i= 0;i<low_occupancy->size;i++){
    if(gsl_vector_get(low_occupancy,i) != 0 || 
       (loopID<switchEmptyBinCriterion &&gsl_vector_get(weird_occupancy,i) != 0)){
      gsl_vector_set(ret,index,i);
      index++;
    }
  }    
  return ret;
}


void gtm_Run(Params * p){
  int numScreeningTrials = p->screening_spec.numScreeningTrials;
  int minNumLoopsWithFullOccupancy = p->screening_spec.minNumLoopsWithFullOccupancy;
  
  int screeningNum = 1;
  
  int consecutiveFullOccupancyConditionSatisfied = 0;
  gsl_vector * numEmptyBins = p->iter_info.numEmptyBins;
  gsl_matrix * gtmFI = p->phi;
  gsl_matrix * gtmW = p->W;
  gsl_matrix * oldY =  gsl_matrix_alloc(p->K,p->D);
  while(screeningNum<numScreeningTrials){
    gtm_Screening(p);
    if(p->iter_info.loopID<minNumLoopsWithFullOccupancy){
      consecutiveFullOccupancyConditionSatisfied = 0;
    }else{
      consecutiveFullOccupancyConditionSatisfied = 1;
      for(unsigned int i = p->iter_info.loopID-minNumLoopsWithFullOccupancy;i<numEmptyBins->size;i++){
	if(gsl_vector_get(numEmptyBins,i) != 0){
	  consecutiveFullOccupancyConditionSatisfied = 0;
	  break;
	}
      }
    }
    
    if(!consecutiveFullOccupancyConditionSatisfied){
      if(screeningNum>=p->screening_spec.flippingKicksIn && (gsl_vector_max(p->iter_info.emptyBin) != 0)){
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,gtmFI,gtmW,0.0,p->Y);      
	double cc_threshold = 0.99;
	gtm_cc_flip_shuffle(p,cc_threshold);
	  //	Y = cc_flip_shuffle(tmT,oldY,gtmR,emptyBin,cc_threshold);
	//	Y = circshift(Y,ceil(rand(1)*(gtmK-1)));
	gsl_matrix * pinvFI = gsl_pseudo_inverse(gtmFI);
	gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,pinvFI,p->Y,0.0,gtmW);      
      }
      screeningNum = screeningNum+1;
    }else{
      break;
    }
    gtm_print_result(p);
    //    gtm_draw_result(gtmW,gtmR,gtmAlpha,gtmBeta,iterInfo,gtmSamples,gtmX);
  }
  gsl_matrix_free(oldY);
}

void gtm_cc_flip_shuffle(Params * p, double cc_threshold){
  gsl_matrix * T = p->T;
  gsl_matrix * Y = p->Y;
  int K = p->Y->size1;
  gsl_vector * emptyBin = p->iter_info.emptyBin;
  gsl_matrix * gtmR = p->R;
  gsl_vector * nodeID = gsl_vector_alloc(gtmR->size2);
  gsl_vector * temp  = gsl_vector_alloc(K);
  gsl_vector_set_all(temp,0);  
  for(unsigned int j = 0;j<gtmR->size2;j++){
    gsl_vector_view view = gsl_matrix_column(gtmR,j);
    gsl_vector_set(nodeID,j,gsl_vector_max_index(&view.vector));
  }
  sp_list * branch_start = sp_list_alloc(10);
  sp_list * branch_end = sp_list_alloc(10);
  unsigned int n_branches = 0;  
  for(unsigned int i = 0;i<emptyBin->size;i++){
    gsl_vector_set(temp,gsl_vector_get(emptyBin,i),1);    
  }
  /* Take differences */
  for(unsigned int i = 0;i<temp->size-1;i++){
    gsl_vector_set(temp,i,gsl_vector_get(temp,i+1)-gsl_vector_get(temp,i));
  }
  sp_list_append(branch_start,0);
  for(unsigned int i = 0;i<temp->size-1;i++){
    if(gsl_vector_get(temp,i) == -1){
      sp_list_append(branch_start,i+1);
    }
  }
  n_branches = sp_list_size(branch_start);
  sp_list * branch_ends_plus_empty = sp_list_alloc(0); // branch-ends (with ensuing empty bins)
  for(unsigned int i = 1;i<n_branches;i++){    
    sp_list_append(branch_ends_plus_empty,sp_list_get(branch_start,i)-1);
  
  }  
  sp_list_append(branch_ends_plus_empty,K-1);

  for(unsigned int i = 0;i<temp->size-1;i++){
    if(gsl_vector_get(temp,i) == 1){
      sp_list_append(branch_end,i);
    }
  }
  if(sp_list_size(branch_end) < n_branches){
    sp_list_append(branch_end,K-1);
  }
  gsl_vector * branch_length = gsl_vector_alloc(n_branches);
  for(unsigned int i = 0;i<n_branches;i++){
    gsl_vector_set(branch_length,i,sp_list_get(branch_ends_plus_empty,i)-sp_list_get(branch_start,i)+1);
  }
  /* start with the longest branch */
  int selected = gsl_vector_max_index(branch_length);
  sp_list * candidatePool = sp_list_alloc(n_branches);
  for(unsigned int i = 0;i<n_branches;i++){
    sp_list_append(candidatePool,i);
  }
  sp_list * current_chain = sp_list_alloc(0);
  for(unsigned int i = sp_list_get(branch_start,selected);i<=sp_list_get(branch_ends_plus_empty,selected);i++){
    sp_list_append(current_chain,i);
  }
  int e0 = sp_list_get(branch_start,selected);
  int e1 = sp_list_get(branch_end,selected);
  sp_list_remove_all(candidatePool,selected);
  
  gsl_matrix * mean_T_at = gsl_matrix_alloc(K,T->size2);
  
  
  sp_list * nodesOfInterest = sp_list_alloc(0);
  for(unsigned int i = 0;i<sp_list_size(branch_start);i++){
    sp_list_append(nodesOfInterest,sp_list_get(branch_start,i));
  }
  sp_list_union(nodesOfInterest,branch_end);
  for(unsigned int i = 0;i<sp_list_size(nodesOfInterest);i++){
    int nn = sp_list_get(nodesOfInterest,i);
    int sum = 0;
    gsl_vector_view mean_T_at_row = gsl_matrix_row(mean_T_at,nn);
    for(unsigned int j = 0;j<T->size1;j++){
      if(nn == gsl_vector_get(nodeID,j)){
	gsl_vector_view T_row = gsl_matrix_row(T,j);
	sum++;
	gsl_vector_add(&mean_T_at_row.vector,&T_row.vector);
      }
    }
    gsl_vector_scale(&mean_T_at_row.vector,1.0/sum);
  }
  while(sp_list_size(candidatePool)){
    gsl_matrix * cc = gsl_matrix_alloc(n_branches,4);
    gsl_matrix_set_all(cc,-1);
    gsl_vector_view T0_view = gsl_matrix_row(mean_T_at,e0);
    gsl_vector_view T1_view = gsl_matrix_row(mean_T_at,e1);
    gsl_vector * T0 = &(T0_view.vector);
    gsl_vector * T1 = &(T1_view.vector);
    for(unsigned int i = 0;i<sp_list_size(candidatePool);i++){
      int jj = sp_list_get(candidatePool,i);
      gsl_vector_view Y0_view = gsl_matrix_row(mean_T_at,sp_list_get(branch_start,jj));
      gsl_vector_view Y1_view = gsl_matrix_row(mean_T_at,sp_list_get(branch_end,jj));
      gsl_vector * Y0 = &(Y0_view.vector);
      gsl_vector * Y1 = &(Y1_view.vector);
      gsl_matrix_set(cc,jj,0,gsl_stats_correlation(Y0->data, Y0->stride, T0->data, T0->stride, T0->size));
      gsl_matrix_set(cc,jj,1,gsl_stats_correlation(Y0->data, Y0->stride, T1->data, T1->stride, T0->size));
      gsl_matrix_set(cc,jj,2,gsl_stats_correlation(Y1->data, Y1->stride, T0->data, T0->stride, T0->size));
      gsl_matrix_set(cc,jj,3,gsl_stats_correlation(Y1->data, Y1->stride, T1->data, T1->stride, T0->size));
    }
    /* find a random candidate that is over the cc_threshold */
    double threshold = cc_threshold*gsl_matrix_max(cc);
    int n_candidates = 0;
    for(unsigned int i = 0;i<cc->size1;i++){
      for(unsigned int j = 0;j<cc->size2;j++){
	if(gsl_matrix_get(cc,i,j) > threshold){
	  n_candidates++;
	}
      }
    }
    int candidate = p_drand48()*n_candidates;
    n_candidates = 0;
    int config = 0;
    for(unsigned int i = 0;i<cc->size1;i++){
      for(unsigned int j = 0;j<cc->size2;j++){
	if(gsl_matrix_get(cc,i,j) > threshold){
	  if(n_candidates == candidate){
	    // we have our men
	    selected = i;
	    config = j;
	  }
	  n_candidates++;
	}
      }
    }
    gsl_matrix_free(cc);
    if(config == 0){
      for(unsigned int i = sp_list_get(branch_start,selected);i<=sp_list_get(branch_ends_plus_empty,selected);i++){
	sp_list_insert(current_chain,0,i);
      }
      e0 = sp_list_get(branch_end,selected);
    }else if(config == 1){
      for(unsigned int i = sp_list_get(branch_start,selected);i<=sp_list_get(branch_ends_plus_empty,selected);i++){
	sp_list_append(current_chain,i);
      }
      e1 = sp_list_get(branch_end,selected);
    }else if(config == 3){
      for(unsigned int i = sp_list_get(branch_ends_plus_empty,selected);i>=sp_list_get(branch_start,selected);i--){
	sp_list_insert(current_chain,0,i);
      }
      e0 = sp_list_get(branch_start,selected);
    }else if(config == 4){
      for(unsigned int i = sp_list_get(branch_ends_plus_empty,selected);i>=sp_list_get(branch_start,selected);i--){
	sp_list_append(current_chain,i);
      }
      e1 = sp_list_get(branch_start,selected);
    }
    sp_list_remove_all(candidatePool,selected);
  }
  for(int i = 0;i<K;i++){
    int found = 0;
    for(unsigned int j = 0;j<sp_list_size(current_chain);j++){
      if(sp_list_get(current_chain,j) == i){
	found = 1;
	break;
      }
    }
    if(!found){
      sp_list_append(current_chain,i);
    }
  }
  /* swap the lines of Y according to current_chain */

  gsl_matrix * work_Y = gsl_matrix_alloc(Y->size1,Y->size2);
  for(unsigned int i =0 ;i<Y->size1;i++){
    gsl_vector_view Y_view = gsl_matrix_row(Y,sp_list_get(current_chain,i));
    gsl_vector_view work_Y_view = gsl_matrix_row(Y,i);
    gsl_vector_memcpy(&work_Y_view.vector,&Y_view.vector);
  }
  gsl_matrix_memcpy(Y,work_Y);
  gsl_matrix_free(work_Y);
  sp_list_free(branch_start);
  sp_list_free(branch_end);
  gsl_vector_free(branch_length);
  gsl_vector_free(nodeID);
  gsl_vector_free(temp);
  sp_list_free(branch_ends_plus_empty);
  sp_list_free(current_chain);
  sp_list_free(candidatePool);
}

void gtm_minimize_difference(gsl_vector * A, const gsl_vector * B){  
  if(A->size != B->size){
    return;
  }
 
  /* Instead of using the simplex minimization i'm simply gonna
     do vector sum of all the vectors defined by the angle B-A
     and compare their angles */
  Complex sum = sp_cinit(0,0);
  for(unsigned int i = 0;i<A->size;i++){
    sp_real(sum) += cos(gsl_vector_get(B,i)-gsl_vector_get(A,i));
    sp_imag(sum) += sin(gsl_vector_get(B,i)-gsl_vector_get(A,i));
  }
  double angle = sp_carg(sum);
  double res0 = 0;
  for(unsigned int i = 0;i<A->size;i++){
    double tmp = gsl_vector_get(B,i)-(gsl_vector_get(A,i)+angle);
    if(fabs(tmp) > M_PI){
      tmp = 2*M_PI-fabs(tmp);
    }
    res0 += tmp*tmp;
  }
  res0 = sqrt(res0/A->size);
  
  /* Try flipping */
  for(unsigned int i = 0;i<A->size;i++){
    gsl_vector_set(A,i,-gsl_vector_get(A,i));
  }
  sum = sp_cinit(0,0);
  for(unsigned int i = 0;i<A->size;i++){
    sp_real(sum) += cos(gsl_vector_get(B,i)-gsl_vector_get(A,i));
    sp_imag(sum) += sin(gsl_vector_get(B,i)-gsl_vector_get(A,i));
  }
  angle = sp_carg(sum);
  double res1 = 0;
  for(unsigned int i = 0;i<A->size;i++){
    double tmp = gsl_vector_get(B,i)-(gsl_vector_get(A,i)+angle);
    if(fabs(tmp) > M_PI){
      tmp = 2*M_PI-fabs(tmp);
    }
    res1 += tmp*tmp;
  }
  res1 = sqrt(res1/A->size);
  
  /* Flip them back */
  for(unsigned int i = 0;i<A->size;i++){
    gsl_vector_set(A,i,-gsl_vector_get(A,i));
  }
  double residual = 0;
  if(res0<res1){
    residual = res0;
  }else{
    residual = res1;
  }
  fprintf(stdout,"Average angle error - %f\n",residual*180.0/M_PI);
}

void gtm_print_result(Params * p){
  gsl_vector * best_X = gsl_vector_alloc(p->R->size2);
  /* find the Y point responsible for each image */
  for(unsigned int j = 0;j<p->R->size2;j++){
    gsl_vector_view view = gsl_matrix_column(p->R,j);
    size_t index = gsl_vector_max_index(&view.vector);
    gsl_vector_set(best_X,j,index*2.0*M_PI/p->R->size1);
  }  
  gsl_vector_view view = gsl_matrix_column(p->gtmSamples,0);
  gtm_minimize_difference(best_X,&view.vector);
  fprintf(stdout,"Iteration - %d\n",p->iter_info.loopID);
  gsl_vector_free(best_X);
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
  //  int maxIterInc = 500;  
  gsl_vector * gtmminDist = gsl_vector_alloc(gtmN);
  gsl_vector * gtmmaxDist = gsl_vector_alloc(gtmN);
  //  [gtmDIST,gtmminDist,gtmmaxDist] = gtm_dist(gtmT,gtmFI*gtmW,2);
  gsl_matrix * gtmY = gsl_matrix_alloc(gtmK,gtmD);
  gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,gtmFI,gtmW,0.0,gtmY);      
  gsl_matrix * gtmDIST = gsl_gtm_dist(gtmT,gtmY,gtmminDist,gtmmaxDist);
  
  gsl_matrix * last_FGF = gsl_matrix_alloc(gtmMplus1,gtmMplus1);

  for(int update = 0;update<numUpdates;update++){
    gsl_vector * lambdaT = gsl_vector_alloc(gtmMplus1);
    gsl_eigen_symm_workspace * eig_work = gsl_eigen_symm_alloc (gtmMplus1);
    for(int cycle = 0;cycle<numEMcycles;cycle++){
      //      gtm_resp(gtmDIST,gtmminDist,gtmmaxDist,gtmBeta,gtmD,2,gtmR);
      double gtmllhLog = gsl_gtm_resp(gtmDIST,gtmminDist,gtmmaxDist,gtmBeta,gtmD,gtmR);
      fprintf(stderr,"gtmllhLog = %e\n",gtmllhLog);
      for(unsigned int i  = 0;i < gtmR->size2;i++){
	gsl_vector_view view = gsl_matrix_column(gtmR,i);
	gsl_vector_scale(&view.vector,gsl_vector_get(gtmRho,i));
      }
      loopID = loopID+1;
      if (loopID>=maxIter){
	fprintf(stderr,"max iterations exceeded!\n");
	abort();
	/* extend numEmptyBins */
	/*	numEmptyBins = [numEmptyBins; zeros(maxIterInc,1)]; %#ok<AGROW>
            newCycle = [newCycle; nan(maxIterInc,1)]; %#ok<AGROW>
            maxIter = maxIter+maxIterInc;*/
      }
      if(p->iter_info.emptyBin){
	gsl_vector_free(p->iter_info.emptyBin);
      }
      p->iter_info.emptyBin = find_empty_bins(gtmR,loopID,switchEmptyBinCriterion);
      gsl_vector_set(numEmptyBins,loopID,p->iter_info.emptyBin->size);
      if (cycle==1){
	gsl_vector_set(newCycle,loopID,1);
      }

      gsl_matrix * FGF = gsl_matrix_alloc(gtmMplus1,gtmMplus1);
      gsl_matrix * FI_T_sumR = gsl_matrix_alloc(gtmMplus1,gtmK);
      gsl_matrix_transpose_memcpy(FI_T_sumR,gtmFI);
      
      for(unsigned int i = 0;i<gtmR->size1;i++){
	long double sum = 0;
	for(unsigned int j = 0;j<gtmR->size2;j++){
	  sum += gsl_matrix_get(gtmR,i,j);
	}	      
	gsl_vector_view view = gsl_matrix_column(FI_T_sumR,i);
	gsl_vector_scale(&view.vector,sum);
      }
      gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,FI_T_sumR,gtmFI,0.0,FGF);          
      
      gsl_matrix_memcpy(last_FGF,FGF);

      
      gsl_matrix_free(FI_T_sumR);
      /* Add gtmAlpha/gtmBeta to the diagonal except the last cell */
      for(unsigned int i = 0;i<FGF->size1-1;i++){
	gsl_matrix_set(FGF,i,i,gsl_matrix_get(FGF,i,i)+gtmAlpha/gtmBeta);
      }
      gsl_matrix * A = FGF;
      for(unsigned int i = 0;i<A->size1;i++){
	for(unsigned int j = 0;j<A->size2;j++){
	  double v = gsl_matrix_get(A,i,j);
	  if(isnan(v) || isinf(v)){
	    flag = 1006;
	    return;
	  }
	}
      }
      gsl_matrix * RT = gsl_matrix_alloc(gtmK,gtmD);
      gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,gtmR,gtmT,0.0,RT);          
      gsl_matrix * FI_T_RT = gsl_matrix_alloc(gtmMplus1,gtmD);
      gsl_blas_dgemm(CblasTrans,CblasNoTrans,1.0,gtmFI,RT,0.0,FI_T_RT);
      gsl_matrix_free(RT);
      gsl_matrix * pinv_A = gsl_pseudo_inverse(A);
      gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,pinv_A,FI_T_RT,0.0,gtmW);
      gsl_matrix_free(FI_T_RT);
      gsl_matrix_free(pinv_A);
      gsl_matrix_free(A);

      gsl_blas_dgemm(CblasNoTrans,CblasNoTrans,1.0,gtmFI,gtmW,0.0,gtmY);      
      gtmDIST = gsl_gtm_dist(gtmT,gtmY,gtmminDist,gtmmaxDist);
    }

    gsl_eigen_symm(last_FGF,lambdaT,eig_work);

    gsl_vector_scale(lambdaT,gtmBeta);
    double gamma = 0;

    for(unsigned int i = 0 ;i<lambdaT->size;i++){
      gamma += gsl_vector_get(lambdaT,i)/(gsl_vector_get(lambdaT,i) + gtmAlpha);
    }
    gamma *= gtmD;
    long double W_sum = 0;
    for(unsigned int i = 0;i<gtmW->size1;i++){
      for(unsigned int j = 0;j<gtmW->size2;j++){
	W_sum += gsl_matrix_get(gtmW,i,j)*gsl_matrix_get(gtmW,i,j);
      }
    }
    gtmAlpha = gamma / W_sum;
    long double DIST_R_sum = 0;
    for(unsigned int i = 0;i<gtmR->size1;i++){
      for(unsigned int j = 0;j<gtmR->size2;j++){
	DIST_R_sum += gsl_matrix_get(gtmDIST,i,j)*gsl_matrix_get(gtmR,i,j);
      }
    }
    gtmBeta = (ND-gamma)/DIST_R_sum;
    if (gtmAlpha<0){
      flag = 1003;
      break;
    }

    if (gtmBeta<0){
      flag = 1004;
      break;
    }
    gsl_vector_free(lambdaT);
    gsl_eigen_symm_free(eig_work);
  }
  gsl_matrix_free(last_FGF);
  p->iter_info.loopID = loopID;  
  p->beta = gtmBeta;
  p->alpha = gtmAlpha;
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
    for(unsigned int j = 0;j<DIST->size2;j++){
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
  for(unsigned int i = 0;i<R->size1;i++){
    for(unsigned int j = 0;j<R->size2;j++){
      gsl_matrix_set(R,i,j,exp((-beta/2)*gsl_matrix_get(DIST,i,j)));
    }
  }
  /* normalize columns of R */
  gsl_vector * R_col_sum = gsl_vector_alloc(R->size2);
  for(unsigned int j = 0;j<R->size2;j++){
    long double sum = 0;
    for(unsigned int i = 0;i<R->size1;i++){
      sum += gsl_matrix_get(R,i,j);
    }
    gsl_vector_set(R_col_sum,j,sum);
    gsl_vector_view view =  gsl_matrix_column(R,j);
    gsl_vector_scale(&view.vector,1/sum);
  }
  double llh = 0;
  if(_R){
    for(unsigned int i = 0;i<R_col_sum->size;i++){
      llh += log(gsl_vector_get(R_col_sum,i)) + gsl_vector_get(distCorr,i)*(-beta/2);
    }
    llh += N*((D/2.0)*log(beta/(2.0*M_PI)) - log(K));
  }else{
    for(unsigned int i = 0;i<R_col_sum->size;i++){
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
  //  struct timespec tp_i;
  //  struct timespec tp_e;
  gsl_matrix * ret;
  if(A->size2 != B->size2){
    printf("number of cols must match!\n");
    return NULL;
  }

  if(B->size2 > 1){
    ret = gsl_gtm_dist2(A,B);
  }else{
    ret = gsl_matrix_alloc(B->size1,A->size1);
    for(unsigned int i = 0;i<A->size1;i++){
      for(unsigned int j = 0;j<B->size1;j++){
	double dist = 0;
	for(unsigned int k = 0;k<B->size2;k++){
	  double tmp = gsl_matrix_get(A,i,k)-gsl_matrix_get(B,j,k);
	  dist += tmp*tmp;	
	}
	gsl_matrix_set(ret,j,i,dist);
      }
    }
  }
  if(minDist || maxDist){
    for(unsigned int j = 0;j<ret->size2;j++){
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



gsl_matrix * gsl_gtm_dist2(gsl_matrix * A, gsl_matrix * B){
  //  struct timespec tp_i;
  //  struct timespec tp_e;
  gsl_matrix * ret = gsl_matrix_alloc(B->size1,A->size1);
  gsl_matrix_set_all(ret,1);
  if(A->size2 != B->size2){
    printf("number of cols must match!\n");
    return NULL;
  }
  /*
    Summing over components of matrices, we can make use of a 'matrix 
    version' of the identity: (a - b)^2 == a^2 + b^2 - 2*a*b, 
    which yields faster execution. When T and Y consist of single columns 
    (which may be the case with calls from gtm_gbf), this has to be handled 
    as a special case. 
  */
  //  clock_gettime(CLOCK_REALTIME,&tp_i);
  gsl_blas_dgemm(CblasNoTrans,CblasTrans,1.0,B,A,0.0,ret);      
  //  clock_gettime(CLOCK_REALTIME,&tp_e);
  //  fprintf(stderr,"gsl_blas_dgemm time - %f ms\n",(tp_e.tv_sec-tp_i.tv_sec)*1000+(tp_e.tv_nsec-tp_i.tv_nsec)/1000000.0);

  gsl_vector * aa = gsl_vector_alloc(A->size1);
  gsl_vector * bb = gsl_vector_alloc(B->size1);

  for(unsigned int i = 0;i<A->size1;i++){
    double sum = 0;
    gsl_vector_view view = gsl_matrix_row(A,i);
    gsl_blas_ddot(&view.vector,&view.vector,&sum);
    gsl_vector_set(aa,i,sum);
  }
  for(unsigned int i = 0;i<B->size1;i++){
    double sum = 0;
    gsl_vector_view view = gsl_matrix_row(B,i);
    gsl_blas_ddot(&view.vector,&view.vector,&sum);
    gsl_vector_set(bb,i,sum);
  }
  for(unsigned int i = 0;i<ret->size1;i++){
    for(unsigned int j = 0;j<ret->size2;j++){
      gsl_matrix_set(ret,i,j,-2*gsl_matrix_get(ret,i,j)+gsl_vector_get(bb,i)+gsl_vector_get(aa,j));
    }
  }
  gsl_vector_free(aa);
  gsl_vector_free(bb);
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
  for(unsigned int i = 0;i<V->size;i++){
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
  for (unsigned int i = 0; i < A->size2; i++) {
    if (fabs(gsl_vector_get(S,i)) > tolerance) {
      gsl_vector_set(S, i, 1.0/gsl_vector_get(S, i));
    }else{
      gsl_vector_set(S, i, 0);
    }
  }
  /* Do V*S */
  for(unsigned int i = 0;i<V->size2;i++){
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
  FILE *f = fopen(filename,"wp");
  for (unsigned int i1 = 0; i1 < m->size1; i1++) {
    for (unsigned int i2 = 0; i2 < m->size2; i2++) {
      fprintf(f,"%g ",gsl_matrix_get(m,i1,i2));
    }
    fprintf(f,"\n");
  }
  fclose(f);
}

void save_gsl_matrix(gsl_matrix *m, char *filename)
{
  Image *f = sp_image_alloc(m->size1,m->size2,1);
  for (unsigned int i1 = 0; i1 < m->size1; i1++) {
    for (unsigned int i2 = 0; i2 < m->size2; i2++) {
      sp_image_set(f,i1,i2,0,sp_cinit(gsl_matrix_get(m,i1,i2),0.0));
    }
  }
  sp_image_write(f,filename,0);
  sp_image_free(f);
}

gsl_matrix *read_gsl_matrix(char *filename)
{
  Image *f = sp_image_read(filename,0);
  gsl_matrix *m = gsl_matrix_alloc(sp_image_x(f),sp_image_y(f));
  for (unsigned int i1 = 0; i1 < m->size1; i1++) {
    for (unsigned int i2 = 0; i2 < m->size2; i2++) {
      gsl_matrix_set(m,i1,i2,sp_real(sp_image_get(f,i1,i2,0)));
    }
  }
  sp_image_free(f);
  return m;
}


/* Read directly from the double precision data written out from matlab */
void init_data(Params * p,int image_size)
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


gsl_matrix * init_W_matlab(const gsl_matrix * T,const gsl_matrix * phi, const int K,const int M,const int N,const int D, double * beta){
  gsl_matrix * W = gsl_matrix_alloc(M+1,D);
  gsl_vector * image_mean = gsl_vector_alloc(D);
  int L_pca = 4;
  //  gsl_matrix * eVts = gsl_matrix_alloc(D,L_pca+1);
  gsl_matrix * eVls_matrix = gsl_matrix_alloc(L_pca+1,1);
  eVls_matrix = gsl_matrix_from_bin("eVls.bin",L_pca+1,L_pca+1);  
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

  //  gsl_ccipca(T_transpose,L_pca+1,15,eVts,eVls);
  /* Use eVls from matlab */
  for(unsigned int i = 0;i<eVls_matrix->size1;i++){
    gsl_vector_set(eVls,i,gsl_matrix_get(eVls_matrix,i,i));
  }

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
  for(unsigned int i = 0;i<PCA_Trace->size1;i++){
    for(unsigned int j = 0;j<PCA_Trace->size2;j++){
      if(i == 0){
	gsl_matrix_set(PCA_Trace,i,j,gsl_matrix_get(projection,0,j));
      }else{
	gsl_matrix_set(PCA_Trace,i,j,0);
      }
    }
  }
  char * off_limit_list = malloc(sizeof(char)*sqDist->size1);
  for(unsigned int i = 0;i<sqDist->size1;i++){
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
    for(unsigned int i = points_to_skip;i<sqDist->size1;i++){
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
int main()
{
  Params * p = init_params();
  gtm_Run(p);
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
