#ifndef _GTM_H_
#define _GTM_H_ 1

typedef struct{
  int numUpdates;
  int numEMcycles;
  int switchEmptyBinCriterion;
  int numScreeningTrials;
  int flippingKicksIn;
  int minNumLoopsWithFullOccupancy;
}ScreeningSpec;



typedef struct{
  int loopID;
  gsl_vector * numEmptyBins;
  gsl_vector * newCycle;
  gsl_vector * emptyBin;
  int flag;
  int maxIter;
}IterInfo;

typedef struct{
  int N; // number of samples
  int K; // number of latent points
  int M; // number of basis functions in latent space
  int L; // number of latent dimensions
  int D; // number of data dimensions
  gsl_matrix *T;    // N D
  gsl_matrix *R;    // K N
  gsl_matrix *W;    // M D
  gsl_matrix *phi;  // K M
  gsl_matrix *Y;    // K D
  
  gsl_matrix *delta;
  gsl_vector *G;
  gsl_matrix *phiTGphi;
  gsl_matrix *inverse;
  gsl_vector *W_b;
  gsl_vector *W_x;
  gsl_matrix *phiTR;

  gsl_vector *Rho;
  
  double beta;
  double alpha;
  ScreeningSpec screening_spec;
  IterInfo iter_info;

  gsl_matrix * gtmSamples;

}Params;

gsl_matrix * gsl_gtm_dist(gsl_matrix * A, gsl_matrix * B,gsl_vector * minDist,gsl_vector * maxDist);
double gsl_gtm_resp(gsl_matrix * DIST,gsl_vector * minDist, gsl_vector * maxDist,double beta, int D, gsl_matrix * _R);
Params * init_params();
void gsl_ccipca(const gsl_matrix * X,int k, int iteration, gsl_matrix * eigenvectors, gsl_vector * eigenvalues);
int int_hash(int * v, int n);
void write_gsl_matrix(gsl_matrix *m, char *filename);
void save_gsl_matrix(gsl_matrix *m, char *filename);
gsl_matrix *read_gsl_matrix(char *filename);
void init_data(Params * p,int image_size);
gsl_matrix * init_phi(double sigma, int K, int M);
gsl_matrix * init_W_matlab(const gsl_matrix * T,const gsl_matrix * phi,const int K,const int M,const int N,const int D,double * beta);
void calculate_Y(const gsl_matrix * phi,const gsl_matrix * W, gsl_matrix * Y);
int factorial(int value);
double log_factorial(int value);
void calculate_R(const int D,const int K,const int N,const gsl_matrix * Y,const gsl_matrix * T, gsl_matrix * delta,gsl_matrix * R, const double beta);
void calculate_R_no_beta(int D,int K,int N,gsl_matrix * T,gsl_matrix * Y,gsl_matrix * R);
void calculate_G(const int K,const int N,gsl_vector * G,const gsl_matrix * R);
void calculate_W(const int K,const int M,const gsl_vector * G,gsl_vector * W_b,gsl_vector * W_x,
		 const gsl_matrix * phi,gsl_matrix * phiTGphi,gsl_matrix * inverse,
		 const gsl_matrix * R,gsl_matrix * phiTR,const gsl_matrix * T, gsl_matrix * W,
		 double alpha,double beta);
void init_beta(int K,int D,gsl_matrix * Y,double beta);
double calculate_beta(int D,int N,int K,gsl_matrix * R,gsl_matrix * delta);
void gtm_Screening(Params * p);
gsl_matrix * gsl_pseudo_inverse(const gsl_matrix * A);
void gtm_Run(Params * p);
void gtm_Screening(Params * p);
void gtm_print_result(Params * p);
void gtm_minimize_difference(gsl_vector * A, const gsl_vector * B);
gsl_matrix * gsl_matrix_from_bin(char * filename, int row, int col);
gsl_matrix * gsl_gtm_dist2(gsl_matrix * A, gsl_matrix * B,gsl_vector * minDist, gsl_vector * maxDist);
void gtm_cc_flip_shuffle(Params * p, double cc_threshold);
#endif
