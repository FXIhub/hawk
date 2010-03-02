#include <spimage.h>

int main(int argc, char ** argv){
  if(argc < 5){
    printf("Usage: %s <object_with_modes> <masked_amplitudes> <threshold> <output> [use constraints]\n",argv[0]);
    exit(0);
  }
  Image * obj = sp_image_read(argv[1],0);
  Image * amp = sp_image_read(argv[2],0);
  real threshold = atof(argv[3]);
  Image * sup = sp_image_duplicate(obj,SP_COPY_ALL);
  real max = sp_image_max(obj,0,0,0,0);
  for(int i= 0;i<sp_image_size(sup);i++){
    if(sp_cabs(sup->image->data[i]) > threshold*max){
      sup->image->data[i] = sp_cinit(1,0);
    }else{
      sup->image->data[i] = sp_cinit(0,0);
    }
  }
  sp_image_write(sup,"input_support.h5",0);
  real beta = 0.9;
  SpPhasingAlgorithm * alg;
  if(argv > 5){
    alg = sp_phasing_er_alloc(SpPositiveRealObject);
  }else{
    alg = sp_phasing_er_alloc(SpNoConstraints);
  }
  //  SpPhasingAlgorithm * alg = sp_phasing_raar_alloc(beta,SpNoConstraints);
  SpSupportAlgorithm * sup_alg = NULL;
  SpPhaser * ph = sp_phaser_alloc();
  sp_phaser_init(ph,alg,sup_alg,SpEngineAutomatic);
  sp_phaser_set_objective(ph,SpRecoverAmplitudes);
  sp_phaser_init_support(ph,sup,0,0);
  Image * phased_amplitudes = sp_image_fft(obj);
  for(int i = 0;i<sp_image_size(obj);i++){
    phased_amplitudes->mask->data[i] = amp->mask->data[i];
    if(!amp->mask->data[i]){
      phased_amplitudes->image->data[i] = sp_cinit(0,0);
    }
  }
  sp_image_write(phased_amplitudes,"input_phased_amplitudes.h5",0);
  Image * zeroed_obj = sp_image_ifft(phased_amplitudes);
  sp_image_scale(zeroed_obj,1.0/sp_image_size(obj));
  sp_phaser_set_phased_amplitudes(ph,phased_amplitudes);
  if(sp_phaser_init_model(ph,zeroed_obj,0)){
    fprintf(stderr,"Error while initializing model!\n");
  }
  sp_image_write(zeroed_obj,"zeroed_obj.h5",0);
  int iter = 500;
  if(sp_phaser_iterate(ph,iter)){
    fprintf(stderr,"Error while iterating!\n");
  }
  Image * out = sp_image_duplicate(sp_phaser_model(ph),SP_COPY_ALL);
  Image * output_amplitudes = sp_image_fft(out);
  for(int i = 0;i<sp_image_size(obj);i++){
    if(amp->mask->data[i]){
      output_amplitudes->image->data[i] = phased_amplitudes->image->data[i];
    }
  }
  sp_image_write(output_amplitudes,"output_phased_amplitudes.h5",0);
  out = sp_image_ifft(output_amplitudes);
  sp_image_scale(out,1.0/sp_image_size(out));
  sp_image_write(out,argv[4],0);

}
