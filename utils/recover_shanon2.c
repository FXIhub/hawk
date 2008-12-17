#include <stdio.h>
#include "spimage.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <time.h>
#ifdef MPI
#include <mpi.h>
#endif
#include "recover_shanon2.h"




sp_list * sp_list_alloc(int init_size){
  sp_list * ret = sp_malloc(sizeof(sp_list));
  ret->data = sp_malloc(sizeof(real)*init_size);
  ret->used = 0;
  ret->size = init_size;
  return ret;
}

sp_sparse_matrix * sp_sparse_matrix_alloc(int rows, int cols){
  sp_sparse_matrix * ret = sp_malloc(sizeof(sp_sparse_matrix));
  ret->rows = rows;
  ret->cols = cols;
  ret->indexes = sp_list_alloc(10);
  ret->data = sp_list_alloc(10);  
  return ret;
}

int sp_sparse_matrix_size(sp_sparse_matrix * m){
  return m->rows*m->cols;
}


real sp_sparse_matrix_integrate(sp_sparse_matrix * m){
  real sum = 0;
  for(int i = 0;i<sp_list_size(m->data);i++){
    sum += sp_list_get(m->data,i);
  }
  return sum;
}

void sp_sparse_matrix_set(sp_sparse_matrix * m, int row, int col, real v){
  int index = col*m->rows+row;
  /* first check of the value is not already in the list */
  for(int i = 0;i<sp_list_size(m->indexes);i++){
    if(sp_list_get(m->indexes,i) == index){
      sp_list_set(m->data,i,v);
      return;
    }
  }  					     
  sp_list_append(m->indexes,index);
  sp_list_append(m->data,v);
}

int sp_sparse_matrix_n_non_zero_entries(sp_sparse_matrix * m){
  return sp_list_size(m->data);
}
real sp_list_get(sp_list * l, int n){
  return l->data[n];
}

void sp_list_set(sp_list * l, int n,real value){
  l->data[n] = value;
}

void sp_list_grow(sp_list * l){
  l->size *= 2;
  l->data = sp_realloc(l->data,sizeof(real)*l->size);
}

void sp_list_append(sp_list * l, real value){
  if(l->used == l->size){
    sp_list_grow(l);
  }
  l->data[l->used] = value;
  l->used++; 
}

int sp_list_size(sp_list * l){
  return l->used;
}

sp_sparse_matrix * image_to_sparse_matrix(Image * a){
  sp_sparse_matrix * ret = sp_sparse_matrix_alloc(sp_image_x(a),sp_image_y(a));
  for(int i = 0;i<sp_image_size(a);i++){
    if(sp_cabs(a->image->data[i])){
      sp_list_append(ret->data,sp_cabs(a->image->data[i]));
      sp_list_append(ret->indexes,i);
    }
  }
  return ret;
}

sp_sparse_matrix * matrix_to_sparse_matrix(sp_matrix * m){
  sp_sparse_matrix * ret = sp_sparse_matrix_alloc(m->rows,m->cols);
  for(int i = 0;i<sp_matrix_size(m);i++){
    if(m->data[i]){
      sp_list_append(ret->data,m->data[i]);
      sp_list_append(ret->indexes,i);
    }
  }
  return ret;
}

#ifdef MPI
void mpi_sync_images(Image * after){
  int id;
  int p;
  int target;
  MPI_Comm_rank(MPI_COMM_WORLD,&id);
  MPI_Comm_size(MPI_COMM_WORLD,&p);
  if(id == 0){
    Image * tmp = sp_image_malloc(sp_image_x(after),sp_image_y(after),sp_image_z(after));
    for(int i = 1;i<p;i++){
      MPI_Secv(img->image->data,sp_image_size(after)*2,MPI_FLOAT,i,0,MPI_COMM_WORLD);
    }
  }
}
#endif

int factorial(int n)
{
  int res = 1;
  int i;
  for (i = 2; i <= n; i++)
    res *= i;
  return res;
}

real calculate_prob(Image *ref, sp_matrix *img)
{
  real sum = 0.0;
  int count = 0;
  int i;

  for (i = 0; i < sp_matrix_size(img); i++) {
    count += (int) img->data[i];
  }
  for (i = 0; i < sp_matrix_size(img); i++) {
    if (sp_real(ref->image->data[i]) == 0.0) {
      return 0.0;
    } else if (img->data[i] != 0.0) {
      sum += ( -(real) count * sp_real(ref->image->data[i]) +
	       img->data[i] *
	       log((real) count * sp_real(ref->image->data[i])) -
	       log(factorial((int) img->data[i])));
    }
  }
  return sum;
}


real calculate_prob_sparse(Image *ref, sp_sparse_matrix *img, real ref_integral)
{
  real sum = 0.0;
  real count = 0;
  count = sp_sparse_matrix_integrate(img);
  sum -= count * ref_integral;

  for(int i = 0;i<sp_list_size(img->data);i++){
    int index = sp_list_get(img->indexes,i);
    real v = sp_list_get(img->data,i);
    if (sp_real(ref->image->data[index]) == 0.0) {
      return 0.0;
    }else{
      sum += v *  log((real) count * sp_real(ref->image->data[index])) -log(factorial((int) v));
    }
  }
  return sum;
}

real sp_image_correlation(Image * a, Image * b){
  Complex * x = a->image->data;
  Complex * y = b->image->data;
  int N = sp_image_size(a);
  double sum_sq_x = 0;
  double sum_sq_y = 0;
  double  sum_coproduct = 0;
  double mean_x = sp_real(x[1]);
  double mean_y = sp_real(y[1]);
  for(int i= 1;i<sp_image_size(a);i++){
    double sweep = (i - 1.0) / i;
    double delta_x = sp_real(x[i]) - mean_x;
    double delta_y = sp_real(y[i]) - mean_y;
    sum_sq_x += delta_x * delta_x * sweep;
    sum_sq_y += delta_y * delta_y * sweep;
    sum_coproduct += delta_x * delta_y * sweep;
    mean_x += delta_x / i;
    mean_y += delta_y / i;
  } 
  double pop_sd_x = sqrt( sum_sq_x / N);
  double pop_sd_y = sqrt( sum_sq_y / N );
  double cov_x_y = sum_coproduct / N;
  double correlation = cov_x_y / (pop_sd_x * pop_sd_y);
  return correlation;
}

sp_sparse_matrix * generate_sparse_from_poisson(Image * in,gsl_rng * r){
  sp_sparse_matrix * out = sp_sparse_matrix_alloc(sp_image_x(in),sp_image_y(in));  
  for(int i = 0;i<sp_sparse_matrix_size(out);i++){
    int v = gsl_ran_poisson(r,sp_real(in->image->data[i]));
    if(v){
      sp_list_append(out->data,v);
      sp_list_append(out->indexes,i);
    }
  }
  return out;
}

Image * generate_from_poisson(Image * in,gsl_rng * r){
  Image * out = sp_image_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));
  for(int i = 0;i<sp_image_size(in);i++){
    sp_real(out->image->data[i]) = gsl_ran_poisson(r,sp_real(in->image->data[i]));
  }
  return out;
}

sp_sparse_matrix ** get_rotated_samples(Image * a, int n_samples,gsl_rng * r,Image * sum){
 sp_sparse_matrix ** rotated_samples = malloc(sizeof(sp_sparse_matrix *)*n_samples);
 int perc = n_samples/100;
  for(int i = 0;i<n_samples;i++){
    if(i%perc == 0){
      printf(".");
      fflush(stdout);
    }
    Image * tmp = generate_from_poisson(a,r);
    sp_image_add(sum,tmp);
    int rot = gsl_rng_get(r)%4;
    Image * tmp2 = sp_image_rotate(tmp,sp_ZAxis,rot,0);    
    rotated_samples[i] = image_to_sparse_matrix(tmp2);
    sp_image_free(tmp);    
    sp_image_free(tmp2);    
  }
  return rotated_samples;
}

Image ** get_orient_samples(Image * a, int n_samples,gsl_rng * r){
 Image ** orient_samples = malloc(sizeof(Image *)*n_samples);
  for(int i = 0;i<n_samples;i++){
    orient_samples[i] = generate_from_poisson(a,r);
  }
  return orient_samples;
}

Image * get_image_avg(Image ** list,int n){
  Image * avg_samples = sp_image_alloc(sp_image_x(list[0]),sp_image_y(list[0]),sp_image_z(list[0]));
  for(int i = 0;i<n;i++){
    sp_image_add(avg_samples,list[i]);
  }
  return avg_samples;
}

void image_normalize(Image * a){
  real sum = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    sum += sp_real(a->image->data[i]);
  }
  sp_image_scale(a,1/sum);
}

void output_result(Image * a, Image * restore,FILE * log,int iter){
    double corr_restore = 0;
    corr_restore = sp_image_correlation(a,restore);
    Image * orient_restore = sp_image_duplicate(restore,SP_COPY_DATA);
    Image * rot_restore = sp_image_rotate(restore,sp_ZAxis,sp_90Degrees,0);    
    if(sp_image_correlation(a,rot_restore) > corr_restore){
      corr_restore = sp_image_correlation(a,rot_restore);
      sp_image_free(orient_restore);
      orient_restore = sp_image_duplicate(rot_restore,SP_COPY_DATA);
    }
    sp_image_free(rot_restore);
    rot_restore = sp_image_rotate(restore,sp_ZAxis,sp_180Degrees,0);
    if(sp_image_correlation(a,rot_restore) > corr_restore){
      corr_restore = sp_image_correlation(a,rot_restore);
      sp_image_free(orient_restore);
      orient_restore = sp_image_duplicate(rot_restore,SP_COPY_DATA);
    }
    sp_image_free(rot_restore);
    rot_restore = sp_image_rotate(restore,sp_ZAxis,sp_270Degrees,0);
    if(sp_image_correlation(a,rot_restore) > corr_restore){
      corr_restore = sp_image_correlation(a,rot_restore);
      sp_image_free(orient_restore);
      orient_restore = sp_image_duplicate(rot_restore,SP_COPY_DATA);
    }
    sp_image_free(rot_restore);

    printf("Iter = %d correlation = %f\n",iter,corr_restore);
    fprintf(log,"%d %f\n",iter,corr_restore);
    fflush(log);
    
    sp_image_write(orient_restore,"orient_restore.png",COLOR_GRAYSCALE);
    //    sp_image_write(restore,"restore.png",COLOR_GRAYSCALE);
}


void tomas_iteration3(Image * restore,Image * after,sp_matrix *** rotated_random_orient_samples,int n_samples){
  int increment = 1;
#ifdef MPI
  MPI_Comm_size(MPI_COMM_WORLD,&increment);
#endif  
  for(int i = 0;i<n_samples;i+=increment){
    real probs[4];
    for(int j = 0;j<4;j++){
      probs[j] = calculate_prob(restore,rotated_random_orient_samples[i][j]);
    }
    real max_probs = probs[0];
    for (int k = 1; k < 4; k++){
      if (probs[k] > max_probs){
	max_probs = probs[k];
      }
    }
    real sum = 0.0;
    for (int k = 0; k < 4; k++) {
      if (probs[k] - max_probs > -50.0) {
	probs[k] = exp(probs[k] - max_probs);
	sum += probs[k];
      } else {
	probs[k] = 0.0;
      }
    }
    for (int k = 0; k < 4; k++) {
      probs[k] /= sum;
    }
    for(int k = 0;k<sp_image_size(restore);k++){
      for(int j = 0;j<4;j++){	
	sp_real(after->image->data[k]) += probs[j]*rotated_random_orient_samples[i][j]->data[k];
      }
    }
    
  }
  /* normalize */
  image_normalize(after);
}


void tomas_iteration(Image * restore,Image * after,sp_sparse_matrix ** random_orient_samples,int n_samples){
  int increment = 1;
  /* calculate the integral of restore */
  real restore_integral = 0;
  for(int i = 0;i<sp_image_size(restore);i++){
    restore_integral += sp_real(restore->image->data[i]);
  }
#ifdef MPI
  MPI_Comm_size(MPI_COMM_WORLD,&increment);
#endif
  Image ** rot_after = malloc(sizeof(Image *)*4);
  for(int j = 0;j<4;j++){
     rot_after[j] = sp_image_rotate(after,sp_ZAxis,j,0);
  }
  float ** probs = malloc(sizeof(float *)*n_samples);
  for(int i = 0;i<n_samples;i+=increment){
    probs[i] = malloc(sizeof(float)*4);
  }
  for(int j = 0;j<4;j++){
    Image * rot_restore = sp_image_rotate(restore,sp_ZAxis,j,0);
    for(int i = 0;i<n_samples;i+=increment){
      probs[i][j] = calculate_prob_sparse(rot_restore,random_orient_samples[i],restore_integral);
    }
    sp_image_free(rot_restore);
  }
  
  for(int i = 0;i<n_samples;i+=increment){
    real max_probs = probs[i][0];
    for (int k = 1; k < 4; k++){
      if (probs[i][k] > max_probs){
	max_probs = probs[i][k];
      }
    }
    real sum = 0.0;
    for (int k = 0; k < 4; k++) {
      if (probs[i][k] - max_probs > -50.0) {
	probs[i][k] = exp(probs[i][k] - max_probs);
	sum += probs[i][k];
      } else {
	probs[i][k] = 0.0;
      }
    }
    for (int k = 0; k < 4; k++) {
      probs[i][k] /= sum;
    }
    
    for(int k = 0;k<sp_list_size(random_orient_samples[i]->data);k++){
      for(int j = 0;j<4;j++){	
	int index = sp_list_get(random_orient_samples[i]->indexes,k);
	real v = sp_list_get(random_orient_samples[i]->data,k);
	sp_real(rot_after[j]->image->data[index]) += probs[i][j]*v;
      }
    }    
  }
  /* Add all the after */
  for(int j = 0;j<4;j++){
    Image * tmp = sp_image_rotate(rot_after[j],sp_ZAxis,(4-j)%4,0);
    sp_image_free(rot_after[j]);
    sp_image_add(after,tmp);
    sp_image_free(tmp);
  }
  /* normalize */
  image_normalize(after);
  for(int i = 0;i<n_samples;i+=increment){
    free(probs[i]);
  }
  free(probs);
}

int main(int argc, char ** argv){
  gsl_rng * r = gsl_rng_alloc (gsl_rng_taus2);
  //  gsl_rng_set(r,time(NULL));
  FILE * log = fopen("recover_shanon.log","w");

#ifdef MPI
  MPI_Init(&argc, &argv);
#endif

  int n_samples = 500;
  if(argc != 2 && argc != 3 && argc != 4 ){
    printf("recover_shanon <input_image> [total n photons scattered] [n samples]\n");
    printf("The defaults are n photons = n pixels and n samples = 500\n");
    exit(0);
  }
  if(argc >= 4){
    n_samples = atoi(argv[3]);
  }
  Image * a = sp_image_read(argv[1],0);
  double image_photons = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    image_photons += sp_real(a->image->data[i]);
  }
  int n_photons = sp_image_size(a);
  if(argc >= 3){
    n_photons = atoi(argv[2]);
  }
  sp_image_scale(a,n_photons/image_photons);

  image_photons = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    image_photons += sp_real(a->image->data[i]);
  }
  printf("Total photons %f\n",image_photons);
  printf("Average photon count %f\n",image_photons/sp_image_size(a));

  printf("Creating rotated samples...");
  fflush(stdout);
  Image * avg_samples = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  sp_sparse_matrix ** random_orient_samples = get_rotated_samples(a,n_samples,r,avg_samples);

  //  Image ** orient_samples = get_orient_samples(a,n_samples,r);
  printf("done\n");

  image_photons = sp_sparse_matrix_integrate(random_orient_samples[0]);
  printf("Total photons in sample 1 - %f\n",image_photons);


  //  Image * avg_samples = get_image_avg(orient_samples,n_samples);
  /*  printf("Rotating samples...");
  fflush(stdout);
  sp_matrix ** random_orient_samples = get_random_orient_samples(orient_samples,n_samples,r);
  printf("done\n");*/

  //  sp_image_write(random_orient_samples[1],"sample.png",COLOR_GRAYSCALE);
  /*  for(int i = 0;i<n_samples;i++){
    sp_image_free(orient_samples[i]);
  }
  free(orient_samples);*/

  Image * avg_random_samples = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  for(int i = 0;i<n_samples;i++){
    for(int k = 0;k<sp_list_size(random_orient_samples[i]->data);k++){
      int index = sp_list_get(random_orient_samples[i]->indexes,k);
      real value = sp_list_get(random_orient_samples[i]->data,k);
      sp_real(avg_random_samples->image->data[index]) += value;
	}
  }
  
  /* Create rotation list */
  /*  printf("Creating rotation list...");
  fflush(stdout);
  sp_matrix *** rotated_random_orient_samples = get_rotation_list(random_orient_samples,  n_samples);
  printf("done\n");*/


  /* Initialize with uniform distribution in [0,1) */
  Image * restore = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  Image * after = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  for(int i = 0;i<sp_image_size(a);i++){
    restore->image->data[i] = sp_cinit(gsl_rng_uniform(r),0);
  }
  image_normalize(restore);

  sp_image_write(a,"orig.png",COLOR_GRAYSCALE);
  sp_image_write(avg_samples,"avg_sample.png",COLOR_GRAYSCALE);
  sp_image_write(avg_random_samples,"avg_random_sample.png",COLOR_GRAYSCALE);
  for(int iter = 0;;iter++){
    tomas_iteration(restore,after,random_orient_samples,n_samples);
    Image * tmp = restore;
    restore = after;
    after = tmp;

    for(int i = 0;i<sp_image_size(a);i++){
      sp_real(after->image->data[i]) = 0;
    }
    
    output_result(a,restore,log,iter);
  }
  return 0;
}
  
  
