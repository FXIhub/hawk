#include <stdio.h>
#include "spimage.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <time.h>
#ifdef MPI
#include <mpi.h>
#endif

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

Image * generate_from_poisson(Image * in,gsl_rng * r){
  Image * out = sp_image_alloc(sp_image_x(in),sp_image_y(in),sp_image_z(in));
  for(int i = 0;i<sp_image_size(in);i++){
    sp_real(out->image->data[i]) = gsl_ran_poisson(r,sp_real(in->image->data[i]));
  }
  return out;
}

sp_matrix *** get_rotation_list(sp_matrix ** random_orient_samples, int n_samples){
  sp_matrix *** rotated_random_orient_samples = malloc(sizeof(sp_matrix **)*n_samples);
  for(int i = 0;i<n_samples;i++){
    rotated_random_orient_samples[i] = malloc(sizeof(sp_matrix *)*4);
    Image * tmp = sp_image_alloc(sp_matrix_rows(random_orient_samples[i]),sp_matrix_cols(random_orient_samples[i]),1);
    for(int k = 0;k<sp_image_size(tmp);k++){
      sp_real(tmp->image->data[k]) = random_orient_samples[i]->data[k];
    }
    for(int j = 0;j<4;j++){
						  
      Image * tmp2 = sp_image_rotate(tmp,sp_ZAxis,j,0);
      rotated_random_orient_samples[i][j] = sp_matrix_alloc(sp_image_x(tmp),sp_image_y(tmp));
      for(int k = 0;k<sp_image_size(tmp);k++){
	rotated_random_orient_samples[i][j]->data[k] = sp_real(tmp2->image->data[k]);
      }
      sp_image_free(tmp2);
    }
    sp_image_free(tmp);
      
  }
  return rotated_random_orient_samples;
}

sp_matrix ** get_random_orient_samples(Image ** orient_samples, int n_samples,gsl_rng * r){
  sp_matrix ** random_orient_samples = malloc(sizeof(sp_matrix *)*n_samples);
  for(int i = 0;i<n_samples;i++){
    //    printf("%d\n",i);
    int rot = gsl_rng_get(r)%4;
    Image * tmp = sp_image_rotate(orient_samples[i],sp_ZAxis,rot,0);
    random_orient_samples[i] = sp_matrix_alloc(sp_image_x(tmp),sp_image_y(tmp));
    for(int k = 0;k<sp_image_size(tmp);k++){
      random_orient_samples[i]->data[k] = sp_real(tmp->image->data[k]);
    }
    sp_image_free(tmp);    
  }
  return random_orient_samples;
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


void tomas_iteration(Image * restore,Image * after,sp_matrix ** random_orient_samples,int n_samples){
  int increment = 1;
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
      probs[i][j] = calculate_prob(rot_restore,random_orient_samples[i]);
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
    for(int k = 0;k<sp_image_size(restore);k++){
      for(int j = 0;j<4;j++){	
	sp_real(rot_after[j]->image->data[k]) += probs[i][j]*random_orient_samples[i]->data[k];
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

  Image ** orient_samples = get_orient_samples(a,n_samples,r);

  image_photons = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    image_photons += sp_real(orient_samples[0]->image->data[i]);
  }
  printf("Total photons in sample 1 - %f\n",image_photons);

  Image * avg_samples = get_image_avg(orient_samples,n_samples);
  printf("Creating samples...");
  fflush(stdout);
  sp_matrix ** random_orient_samples = get_random_orient_samples(orient_samples,n_samples,r);
  printf("done\n");

  sp_image_write(orient_samples[1],"sample.png",COLOR_GRAYSCALE);
  for(int i = 0;i<n_samples;i++){
    sp_image_free(orient_samples[i]);
  }

  Image * avg_random_samples = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  for(int i = 0;i<n_samples;i++){
    for(int k = 0;k<sp_image_size(avg_random_samples);k++){
      sp_real(avg_random_samples->image->data[k]) += random_orient_samples[i]->data[k];
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

  /* Initialize with one of the samples */
  /*  Image * restore = sp_image_duplicate(random_orient_samples[n_samples-1],SP_COPY_DATA);*/
  sp_image_write(a,"orig.png",COLOR_GRAYSCALE);
  sp_image_write(avg_samples,"avg_sample.png",COLOR_GRAYSCALE);
  sp_image_write(avg_random_samples,"avg_random_sample.png",COLOR_GRAYSCALE);
  for(int iter = 0;;iter++){
    //    tomas_iteration3(restore,after,rotated_random_orient_samples,n_samples);
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
  
  
