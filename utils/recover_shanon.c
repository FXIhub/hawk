#include <stdio.h>
#include "spimage.h"
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <time.h>

 int factorial(int n)
{
  int res = 1;
  int i;
  for (i = 2; i <= n; i++)
    res *= i;
  return res;
}

real calculate_prob(Image *ref, Image *img)
{
  real sum = 0.0;
  int count = 0;
  int i;

  for (i = 0; i < sp_image_size(img); i++) {
    count += (int) sp_real(img->image->data[i]);
  }
  for (i = 0; i < sp_image_size(img); i++) {
    if (sp_real(ref->image->data[i]) == 0.0) {
      return 0.0;
    } else if (sp_real(img->image->data[i]) != 0.0) {
      sum += ( -(real) count * sp_real(ref->image->data[i]) +
               sp_real(img->image->data[i]) *
               log((real) count * sp_real(ref->image->data[i])) -
               log(factorial((int) sp_real(img->image->data[i]))));
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

int main(int argc, char ** argv){
  gsl_rng * r = gsl_rng_alloc (gsl_rng_taus2);
  gsl_rng_set(r,time(NULL));
  FILE * log = fopen("recover_shanon.log","w");

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

  Image ** orient_samples = malloc(sizeof(Image *)*n_samples);
  for(int i = 0;i<n_samples;i++){
    orient_samples[i] = generate_from_poisson(a,r);
  }

  image_photons = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    image_photons += sp_real(orient_samples[0]->image->data[i]);
  }
  printf("Total photons in sample 1 - %f\n",image_photons);

  Image * avg_samples = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  for(int i = 0;i<n_samples;i++){
    sp_image_add(avg_samples,orient_samples[i]);
  }
  printf("Creating samples...");
  fflush(stdout);
  Image ** random_orient_samples = malloc(sizeof(Image *)*n_samples);
  for(int i = 0;i<n_samples;i++){
    int rot = gsl_rng_get(r)%4;
    if(rot == 0){
      random_orient_samples[i] = sp_image_duplicate(orient_samples[i],SP_COPY_DATA);
    }else if(rot == 1){
      random_orient_samples[i] = sp_image_rotate(orient_samples[i],sp_ZAxis,sp_90Degrees,0);
    }else if(rot == 2){
      random_orient_samples[i] = sp_image_rotate(orient_samples[i],sp_ZAxis,sp_180Degrees,0);
    }else if(rot == 3){
      random_orient_samples[i] = sp_image_rotate(orient_samples[i],sp_ZAxis,sp_270Degrees,0);
    }
  }
  printf("done\n");
  Image * avg_random_samples = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  for(int i = 0;i<n_samples;i++){
    sp_image_add(avg_random_samples,random_orient_samples[i]);
  }
  
  /* Create rotation list */
  printf("Creating rotation list...");
  fflush(stdout);
  Image *** rotated_random_orient_samples = malloc(sizeof(Image **)*n_samples);
  for(int i = 0;i<n_samples;i++){
    rotated_random_orient_samples[i] = malloc(sizeof(Image *)*4);
    for(int j = 0;j<4;j++){
      rotated_random_orient_samples[i][j] = sp_image_rotate(random_orient_samples[i],sp_ZAxis,j,0);
    }
    printf("%d\n",i);
  }
  printf("done\n");


  /* Initialize with uniform distribution in [0,1) */
  Image * restore = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  Image * after = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  real sum = 0;
  for(int i = 0;i<sp_image_size(a);i++){
    restore->image->data[i] = sp_cinit(gsl_rng_uniform(r),0);
    sum += sp_real(restore->image->data[i]);
  }
  sp_image_scale(restore,1/sum);

  /* Initialize with one of the samples */
  /*  Image * restore = sp_image_duplicate(random_orient_samples[n_samples-1],SP_COPY_DATA);*/
  sp_image_write(a,"orig.png",COLOR_GRAYSCALE);
  sp_image_write(orient_samples[1],"sample.png",COLOR_GRAYSCALE);
  sp_image_write(avg_samples,"avg_sample.png",COLOR_GRAYSCALE);
  sp_image_write(avg_random_samples,"avg_random_sample.png",COLOR_GRAYSCALE);
  for(int iter = 0;;iter++){
    /*  Clear after */ 
          for(int i = 0;i<sp_image_size(a);i++){
      sp_real(after->image->data[i]) = 0;
      }
    for(int i = 0;i<n_samples;i++){
      double max_corr = -10000000000000000;
      int max_j = -1;
      for(int j = 0;j<4;j++){
	/*	real corr = sp_image_correlation(restore,rotated_random_orient_samples[i][j]);*/
	real corr = calculate_prob(restore,rotated_random_orient_samples[i][j]);
	if(corr > max_corr){
	  max_corr = corr;
	  max_j = j;
	}
	/*	if(corr< 0){
	  continue;
	  }*/
	//	corr *= corr;
	//	corr *= corr;
	//	corr = sqrt(fabs(corr));
	/*	for(int k = 0;k<sp_image_size(a);k++){
	  sp_real(restore->image->data[k]) += corr*sp_real(rotated_random_orient_samples[i][j]->image->data[k]);
	  }*/
      }
      if(max_j == -1){
	abort();
      }
      /*      for(int k = 0;k<sp_image_size(a);k++){
	sp_real(restore->image->data[k]) += sp_real(rotated_random_orient_samples[i][max_j]->image->data[k]);
	}*/

      for(int k = 0;k<sp_image_size(a);k++){
	sp_real(after->image->data[k]) += sp_real(rotated_random_orient_samples[i][max_j]->image->data[k]);
      }

    }
    Image * tmp = restore;
    restore = after;
    after = tmp;
    /* normalize */
    for(int i = 0;i<sp_image_size(a);i++){
      sp_real(after->image->data[i]) = 0;
      sum += sp_real(restore->image->data[i]);
    }
    sp_image_scale(restore,1/sum);
    
    
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

    printf("correlation - %f\n",corr_restore);
    fprintf(log,"%d %f\n",iter,corr_restore);
    if(iter % 20 == 0){
      fflush(log);
      sp_image_write(orient_restore,"orient_restore.png",COLOR_GRAYSCALE);
      sp_image_write(restore,"restore.png",COLOR_GRAYSCALE);
    }
  }
  return 0;
}
  
  
