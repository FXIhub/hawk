#include <spimage.h>
#include <time.h>

/* 
   This program calculates the diffraction pattern obtained by exposing an object with a gaussian beam multiple times, 
   with the beam centered at a different position each time.
   The intensity of the beam is normally distributed with a mean on the beam position and standard deviation intensity sigma.
   Tha phase of the beam is radially symetric around the beam center but sinusoidal along the radius.

   Then this image is back fourier transformed using the phases obtained by fourier transforming the object.
 */

int main(int argc, char ** argv){
  char buffer[1024];
  if(argc < 7){
    printf("Usage: %s <object> <number of shots> <beam position sigma> <beam width sigma> <intensity sigma> <phase period>\n",argv[0]);
    return 1;
  }
  real width_sigma = atof(argv[4]);
  real phi_period = atof(argv[6]);
  real i_sigma = atof(argv[5]);
  real r_sigma = atof(argv[3]);
  int n_shots = atoi(argv[2]);
  Image * a = sp_image_read(argv[1],0);
  sp_srand(time(NULL));
  if(!a){
    sp_error_fatal("Could not read input!");
  }
  Image * diff = sp_image_alloc(sp_image_x(a),sp_image_y(a),sp_image_z(a));
  diff->scaled = 0;
  for(int i = 0;i<n_shots;i++){
    real base_int = fabs(sp_box_muller(1,i_sigma));
    real beam_x = sp_box_muller(sp_image_x(a)/2,r_sigma);
    real beam_y = sp_box_muller(sp_image_y(a)/2,r_sigma);
    Image * tmp = sp_image_duplicate(a,SP_COPY_DATA);
    for(int z = 0;z<sp_image_z(tmp);z++){    
      for(int y = 0;y<sp_image_y(tmp);y++){
	for(int x = 0;x<sp_image_x(tmp);x++){
	  real dist_to_beam_center = sqrt(fabs(x-beam_x)*fabs(x-beam_x)+fabs(y-beam_y)*fabs(y-beam_y));
	  real intensity = base_int*1/(sqrt(2*M_PI)*width_sigma)*exp(-dist_to_beam_center*dist_to_beam_center/(2*width_sigma*width_sigma));
	  real phase = dist_to_beam_center/phi_period;
	  sp_image_set(tmp,x,y,z,sp_crot(sp_cscale(sp_image_get(tmp,x,y,z),sqrt(intensity)),phase));
	}
      }
    }
    sprintf(buffer,"tmp-%04d.png",i);
    sp_image_write(tmp,buffer,COLOR_GRAYSCALE);
    sprintf(buffer,"tmp-%04d-phase.png",i);
    sp_image_write(tmp,buffer,COLOR_PHASE|COLOR_WHEEL);

    Image * diff_tmp = sp_image_fft(tmp);
    diff_tmp->scaled = 1;
    sp_image_to_intensities(diff_tmp);    
    sprintf(buffer,"diff_tmp-%04d.png",i);
    sp_image_write(sp_image_shift(diff_tmp),buffer,COLOR_JET|LOG_SCALE);
    sp_image_add(diff,diff_tmp);
    sp_image_free(diff_tmp);
    sp_image_free(tmp);
    
  }
  sp_image_write(sp_image_shift(diff),"diff.png",COLOR_JET|LOG_SCALE);
  Image * phases = sp_image_fft(a);
  Image * autocorr = sp_image_fft(diff);
  sp_image_write(sp_image_shift(autocorr),"autocorrelation.png",COLOR_GRAYSCALE|LOG_SCALE);
  sp_image_to_amplitudes(diff);
  for(int i = 0;i<sp_image_size(a);i++){
    phases->image->data[i] = sp_cscale(phases->image->data[i],sp_cabs(diff->image->data[i])/sp_cabs(phases->image->data[i]));
  }
  Image * real_space = sp_image_ifft(phases);
  sp_image_write(real_space,"real_space.png",COLOR_GRAYSCALE);
  return 0;
}
