#include <stdlib.h>
#include <time.h>

#include "spimage.h"
#include "configuration.h"
#include "log.h"
#include "algorithms.h"
#include "io_utils.h"
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif

/*
static int hkl_inside_image(sp_vector * hkl, Image *a){
  if(a->num_dimensions == SP_2D){
    if(hkl->data[0] < sp_image_x(a)/2 &&
       hkl->data[0] >= -sp_image_x(a)/2 &&
       hkl->data[1] < sp_image_y(a)/2 &&
       hkl->data[1] >= -sp_image_y(a)/2){
       return 1;
    }
  }else if(a->num_dimensions == SP_3D){    
    if(hkl->data[0] < sp_image_x(a)/2 &&
       hkl->data[0] >= -sp_image_x(a)/2 &&
       hkl->data[1] < sp_image_y(a)/2 &&
       hkl->data[1] >= -sp_image_y(a)/2 &&
       hkl->data[2] < sp_image_z(a)/2 &&
       hkl->data[2] >= -sp_image_z(a)/2){
      return 1;
    }
  }
  return 0;
}
*/

/* this returns the hkl from the xyz of a shifted image in pixel units */
/*
static sp_vector * hkl_from_xyz(int x, int y, int z, Image * a){
  sp_vector * hkl = sp_vector_alloc(3);
  if(a->shifted){
    if(x < sp_image_x(a)/2){
      hkl->data[0] = x;
    }else{
      hkl->data[0] = x-sp_image_x(a);
    }
    if(y < sp_image_y(a)/2){
      hkl->data[1] = -y;
    }else{
      hkl->data[1] = sp_image_y(a)-y;
    }
    if(a->num_dimensions == SP_2D){
      hkl->data[2] = 0;
    }else{
      if(z < sp_image_z(a)/2){
	hkl->data[2] = -z;
      }else{
	hkl->data[2] = sp_image_z(a)-z;
      }
    }
    return hkl;
  }else{
    abort();
  }
  return NULL;
}
*/

/* this returns the hkl from the xyz of a shifted image in pixel units */
/*
static sp_vector * xyz_from_hkl(sp_vector * hkl, Image * a){
  sp_vector * xyz = sp_vector_alloc(3);
  if(a->shifted){
    if(hkl->data[0] >= 0){
      xyz->data[0] = hkl->data[0];
    }else{
      xyz->data[0] = sp_image_x(a)+hkl->data[0];
    }
    if(hkl->data[1] <=0){
      xyz->data[1] = -hkl->data[1];
    }else{
      xyz->data[1] = sp_image_y(a)-hkl->data[1];
    }
    if(hkl->data[2] <= 0){
    xyz->data[2] = -hkl->data[2];
    }else{
      xyz->data[2] = sp_image_z(a)-hkl->data[2];
    }
    return xyz;
  }else{
    abort();
  }
  return NULL;
}
*/

/* this does not really seem to work */
/* This will try to test the idea that in average phase(h1) = phase(h2) + phase(h1-h2) */
/*
static real phase_relation_error(Image * fourier_out){
  int n = 0;
  real sum_error = 0;
  const int num_points = 5;
  sp_vector * points[num_points];
  sp_vector * hkl_minus_point = sp_vector_alloc(3);
  for(int i = 0;i < num_points;i++){
    points[i] = sp_vector_alloc(3);
    points[i]->data[0] = (rand()%(sp_image_x(fourier_out)/2));
    points[i]->data[1] = -(rand()%(sp_image_y(fourier_out)/2));
    if(fourier_out->num_dimensions == SP_3D){
      points[i]->data[2] = -(rand()%(sp_image_z(fourier_out)/2));
    }else{
      points[i]->data[2] = 0;
    }
  }
  
*/
  /* only check every 27th point*/
/*  for(int z = 0; z < sp_image_z(fourier_out);z+=3){
    for(int y = 0; y < sp_image_y(fourier_out);y+=3){
      for(int x = 0; x < sp_image_x(fourier_out);x+=3){
	sp_vector * hkl = hkl_from_xyz(x,y,z,fourier_out);
	for(int i = 0;i<num_points;i++){
	  sp_vector_memcpy(hkl_minus_point,hkl);
	  sp_vector_sub(hkl_minus_point,points[i]);
	  if(hkl_inside_image(hkl_minus_point,fourier_out)){
	    sp_vector * hkl_minus_point_xyz = xyz_from_hkl(hkl_minus_point,fourier_out);
	    sp_vector * points_xyz = xyz_from_hkl(points[i],fourier_out);
	    real error = sp_carg(sp_image_get(fourier_out,points_xyz->data[0],points_xyz->data[1],points_xyz->data[2]))+
	      sp_carg(sp_image_get(fourier_out,hkl_minus_point_xyz->data[0],hkl_minus_point_xyz->data[1],hkl_minus_point_xyz->data[2]))-
	      sp_carg(sp_image_get(fourier_out,x,y,z));
	    sp_vector_free(hkl_minus_point_xyz);
	    sp_vector_free(points_xyz);
*/
	    /* make sure error is in [-Pi,Pi]*/
	    /*
	    while(error < M_PI){
	      error += 2*M_PI;
	    }
	    while(error > M_PI){
	      error -= 2*M_PI;
	    }
	    sum_error += fabs(error);
	    n++;
	  }
	}
	sp_vector_free(hkl);
      }
    }
  }
  return sum_error/n;
}
*/

void output_from_phaser(SpPhaser * ph,Options * opts, Log * log){
  output_to_log(sp_phaser_amplitudes(ph),sp_phaser_old_model(ph),sp_phaser_model(ph),sp_phaser_fmodel(ph),sp_phaser_support(ph),opts,log);
  
}

void output_to_log(const Image * exp_amp,const Image * real_in, const Image * real_out,const Image * fourier_out,const Image * support, Options * opts,Log * log){
  int it_outer;
  time_t t;
  real Ereal = 0;
  real Ereal_den = 0;
  real Efourier = 0;
  real Efourier_den = 0;
  real FcFo = 0;
  real FcFo_den = 0;
  real SupSize = 0;
  long long i;
  int iter = opts->cur_iteration;
  static int sol_superimpose_counter = 0;
  static Image * previous_out = NULL;
/*  char buffer[1024];*/
  if(!sp_image_is_valid(real_in)){
    hawk_fatal("Real space input image is invalid!");
  }
  if(!sp_image_is_valid(real_out)){
    hawk_fatal("Real space output image is invalid!");
  }

  if(!log->cumulative_fluctuation){
    log->cumulative_fluctuation = sp_image_duplicate(real_in,SP_COPY_DETECTOR);
  }
  log->dRho = 0;
  for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
    log->dRho += fabs(sp_cabs(real_in->image->data[i]) - sp_cabs(real_out->image->data[i]));
  }
  if(iter % opts->iterations == opts->iterations/2){
    if(!previous_out){
      previous_out = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
    }else{
      log->int_cum_fluctuation = 0;
      for(i = 0;i<sp_c3matrix_size(real_in->image);i++){
	/*      log->cumulative_fluctuation->image->data[i] += fabs(real_in->image->data[i] - real_out->image->data[i]);*/
	log->int_cum_fluctuation += fabs(sp_cabs(previous_out->image->data[i]) - sp_cabs(real_out->image->data[i]));    
      }
      /*    sprintf(buffer,"cum_fluct-%06d.png",iter);
	    sp_image_write(log->cumulative_fluctuation,buffer,sizeof(real));*/
      sp_image_free(previous_out);     
      previous_out = sp_image_duplicate(real_out,SP_COPY_DATA|SP_COPY_MASK);
    }
  }

  if(opts->iterations){
    it_outer = (iter)/opts->iterations;
  }else{
    it_outer = 0;
  }
  if(!opts->flog){
    opts->flog = fopen(opts->log_file,"w");
    hawk_log(opts->flog,"# Uwrapc version %s compiled on %s at %s\n",VERSION,__DATE__,__TIME__);
    time(&t);
    
    hawk_log(opts->flog,"# Date: %s\n",ctime(&t));
    hawk_log(opts->flog,"# Command line:\n# %s\n#\n",opts->commandline);
    hawk_log(opts->flog,"# It(in) - Number of iterations of the inner loop\n");
    hawk_log(opts->flog,"# It(out) - Number of iterations of the outer loop\n");
    hawk_log(opts->flog,"# Ereal - Real Space error. Eq 16 in Ref 1\n");
    hawk_log(opts->flog,"# Efourier - Fourier Space error. Eq 17 in Ref 1.\n"
	    "#\t   Basically Xray R factor\n");
    hawk_log(opts->flog,"# <Fc/Fo> - Simply the average of calculated\n"
	    "#\t  amplitude divided by experimental one\n");
    hawk_log(opts->flog,"# SupSize - Support size in relation to image size (%%)\n");
    hawk_log(opts->flog,"# Ref 1.  High-resolution ab initio three-dimensional\n"
	    "#\tX-ray diffraction microscopy\n\n");
    
    hawk_log(opts->flog,"#It(in)\tIt(out)\tEreal\t\tEfourier\t<Fc/Fo>\t\tSupSize\n");        

    hawk_log(opts->flog,"@TYPE xy\n");
    hawk_log(opts->flog,"@ view 0.15, 0.15, 0.75, 0.8\n");
    hawk_log(opts->flog,"@ legend on\n");
    hawk_log(opts->flog,"@ legend box on\n");
    hawk_log(opts->flog,"@ legend loctype view\n");
    hawk_log(opts->flog,"@ legend 0.78, 0.8\n");
    hawk_log(opts->flog,"@ legend length 2\n");
    hawk_log(opts->flog,"@ s0 legend \"It(out) \\N\"\n");
/*    hawk_log(log,"@ s0 legend \"It(in) \\N\"\n");*/
    hawk_log(opts->flog,"@ s1 legend \"Ereal \\N\"\n");
    hawk_log(opts->flog,"@ s2 legend \"Efourier \\N\"\n");
    hawk_log(opts->flog,"@ s3 legend \"<Fc/Fo> \\N\"\n");
    hawk_log(opts->flog,"@ s4 legend \"SupSize(%%) \\N\"\n");
    hawk_log(opts->flog,"@ s5 legend \"Beta \\N\"\n");
    hawk_log(opts->flog,"@ s6 legend \"Threshold \\N\"\n");
    hawk_log(opts->flog,"@ s7 legend \"Algorithm \\N\"\n");
    hawk_log(opts->flog,"@ s8 legend \"dEreal \\N\"\n");
    hawk_log(opts->flog,"@ s9 legend \"dSupSize \\N\"\n");
    hawk_log(opts->flog,"@ s10 legend \"Blur Radius \\N\"\n");
    hawk_log(opts->flog,"@ s11 legend \"Int Cum Fluct \\N\"\n");
    hawk_log(opts->flog,"@ s12 legend \"Object Area \\N\"\n");
    hawk_log(opts->flog,"@ s13 legend \"Phase Relation Error \\N\"\n");
    hawk_log(opts->flog,"@ s14 legend \"Correlation with solution \\N\"\n");
    hawk_log(opts->flog,"@ s15 legend \"Phase Blur Radius \\N\"\n");
    hawk_log(opts->flog,"@ s16 legend \"Delta Rho \\N\"\n");
    hawk_log(opts->flog,"@ s17 legend \"Iterations/s \\N\"\n");

  }
  for(i = 0;i<sp_c3matrix_size(exp_amp->image);i++){
    if(sp_real(support->image->data[i])){
      SupSize++;
      Ereal_den += sp_cabs(real_out->image->data[i])*sp_cabs(real_out->image->data[i]);
    }else{
      Ereal += sp_cabs(real_out->image->data[i])*sp_cabs(real_out->image->data[i]);
    }
    if(sp_real(exp_amp->image->data[i]) && exp_amp->mask->data[i]){
      Efourier += (sp_cabs(fourier_out->image->data[i])-sp_cabs(exp_amp->image->data[i]))*(sp_cabs(fourier_out->image->data[i])-sp_cabs(exp_amp->image->data[i]));
      Efourier_den += sp_cabs(exp_amp->image->data[i])*sp_cabs(exp_amp->image->data[i]);
      FcFo += sp_cabs(fourier_out->image->data[i]);
      FcFo_den += sp_cabs(exp_amp->image->data[i]) ;
    }
  }  
  Ereal /= Ereal_den;
  Efourier /= Efourier_den;
  FcFo /= FcFo_den;
  SupSize /= sp_c3matrix_size(exp_amp->image);
  SupSize *= 100;
  log->iter = iter;
  log->it_outer = it_outer;
  log->Ereal = Ereal;
  log->Efourier = Efourier;
  log->FcFo = FcFo;
  log->SupSize = SupSize;
  log->Ereal_list[(opts->cur_iteration/opts->log_output_period)%RUN_AVG_LEN] = Ereal;
  log->SupSize_list[(opts->cur_iteration/opts->log_output_period)%RUN_AVG_LEN] = SupSize;
  log->dEreal = -log->Ereal_run_avg;
  log->dSupSize = -log->SupSize_run_avg;
  log->Ereal_run_avg = 0;
  log->SupSize_run_avg = 0;
  log->sol_correlation = 0;
  for(i = 0;i<MIN((opts->cur_iteration/opts->log_output_period),RUN_AVG_LEN);i++){
    log->Ereal_run_avg += log->Ereal_list[i];
    log->SupSize_run_avg += log->SupSize_list[i];
  }
  if(i){
    log->Ereal_run_avg /= i;
    log->SupSize_run_avg /= i;
  }

  log->dEreal += log->Ereal_run_avg;
  log->dSupSize += log->SupSize_run_avg;
  if(opts->solution_image){
    if(sol_superimpose_counter%10 == 0){
      sp_image_superimpose(real_out,opts->solution_image,SpEnantiomorph);
    }
    sol_superimpose_counter++;
    log->sol_correlation = sp_image_correlation_coefficient(opts->solution_image,real_out);
  }
  hawk_log(opts->flog,"%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n",iter,it_outer,
	   Ereal,Efourier,FcFo,SupSize,get_beta(opts),log->threshold,get_algorithm(opts,log),
	   log->dEreal,log->dSupSize,get_blur_radius(opts),log->int_cum_fluctuation,get_object_area(opts),0.0,
	   log->sol_correlation,get_phases_blur_radius(opts),log->dRho,iter*1.0e6/sp_timer_elapsed(log->timer));
  
  fflush(opts->flog);

  if(Ereal < opts->real_error_tolerance){
    opts->reconstruction_finished = 1;
  }
}  
  
     

void init_log(Log * log){
  int i;
  log->dEreal = 1;
  log->Ereal = 1;
  log->dSupSize = -1;
  log->SupSize = 1;
  log->threshold = (real)0.20;
  log->Ereal_run_avg = 0;
  log->SupSize_run_avg = 0;
  for(i = 0;i<RUN_AVG_LEN;i++){
    log->Ereal_list[i] = 0;
    log->SupSize_list[i] = 0;
  }
  log->cumulative_fluctuation = NULL;
  log->int_cum_fluctuation = 0;
  log->timer = sp_timer_start();
}


