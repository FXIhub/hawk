#include <sys/stat.h>
#include <sys/types.h>
#include <gaul.h>
#include "spimage.h"
#include "uwrapc.h"

/* I have this statics here because gaul doesn't support any void * data structure passing to the callbacks */
static Image * support = NULL;
static Image * prev_support = NULL;
static Image * real_in = NULL;
static Image * real_out = NULL;
static Image * exp_sigma = NULL;
static Image * amp = NULL;
static Options * opts = NULL;
static Log my_log;
static real support_size;
static real radius;
static int stop;
static int stop_threshold;

entity *test_adaptation(population *pop, entity *child){
  entity        *adult;         /* Adapted solution. */
  double * img;
  Image * fc;
  int i;
  /*
   * We must generate a new solution by copying the original solution.
   * This function copys all genomic, and if appropriate, phenomic data.
   * It is never safe to adapt the solution in place.
   */
  adult = ga_entity_clone(pop, child);
  img = (double *)adult->chromosome[0];
  for(i = 0;i<TSIZE(amp);i++){
    real_in->r[i] = img[i];
    real_in->c[i] = img[i+TSIZE(amp)];
    real_in->image[i] = norm(real_in,i);
  }
  freeimg(real_out);
  if(get_algorithm(opts,&my_log) == HIO){     
    real_out = basic_hio_iteration(amp, real_in, support,opts,&my_log);
  }else if(get_algorithm(opts,&my_log) == RAAR){     
    real_out = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&my_log);
  }else if(get_algorithm(opts,&my_log) == HPR){     
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&my_log);
  }

  adult->fitness = 0;
  fc = image_fft(real_out);
  for(i = 0;i<TSIZE(amp);i++){
    img[i] = real_out->r[i];
    img[i+TSIZE(amp)] = real_out->c[i];
    adult->fitness -= fabs(fc->image[i] - amp->image[i]);
  }
  freeimg(fc);
  return adult;
}

boolean test_score(population *pop, entity *entity){
  int i;
  double * img = (double *)entity->chromosome[0];
  Image * fc;
  for(i = 0;i<TSIZE(amp);i++){
    if(!support->image[i]){
      entity->fitness -= sqrt(img[i]*img[i]+img[TSIZE(amp)+i]*img[TSIZE(amp)+i]);
    }
  }
  for(i = 0;i<TSIZE(amp);i++){
    real_out->r[i] = img[i];
    real_out->c[i] = img[i+TSIZE(amp)];
    real_out->image[i] = norm(real_out,i);
  }
  fc = image_fft(real_out);
  entity->fitness = 0;
  for(i = 0;i<TSIZE(fc);i++){
    if(amp->mask[i]){
      entity->fitness -= fabs(fc->image[i] - amp->image[i]);
    }
/*    if(!support->image[i]){
      entity->fitness -= real_out->image[i];
    }*/
  }	
  fprintf(stderr,"Fitness - %f\n",entity->fitness);

  freeimg(fc);
  return TRUE;
}


boolean test_generation_callback(int generation, population *pop){
  double * img = (double *)pop->entity_iarray[0]->chromosome[0];
  int i;
  char buffer[1024];
  real support_threshold;
  for(i = 0;i<TSIZE(amp);i++){
    real_out->image[i] = sqrt(img[i]*img[i]+img[TSIZE(amp)+i]*img[TSIZE(amp)+i]);
  }
  printf("%d: fitness = %f\n",generation,pop->entity_iarray[0]->fitness);


  if(opts->iterations && opts->cur_iteration%opts->iterations == opts->iterations-1){
    sprintf(buffer,"real_out-%05d.png",opts->cur_iteration);
    write_png(real_out,buffer,COLOR_JET);
    
    freeimg(prev_support);
    prev_support = imgcpy(support);
    freeimg(support);      
    support_threshold = get_newsupport_level(real_out,&support_size,radius,&my_log,opts);
    my_log.threshold = support_threshold;
    if(support_threshold > 0){
      /*	support =  get_newsupport(real_out,support_threshold, radius,opts);*/
      support =  get_filtered_support(real_out,support_threshold, radius,opts);
    }else{
      if(opts->support_update_algorithm == REAL_ERROR_CAPPED){
	exit(0);
      }else{
	abort();
      }
    }
    if(opts->cur_iteration <= opts->iterations_to_min_blur){
      radius = get_blur_radius(opts);
    }
    if(/*opts->cur_iteration > 50 ||*/ (opts->automatic && opts->algorithm == RAAR && my_log.Ereal < 0.2)){
      stop++;
    }
    if(stop > stop_threshold){
      exit(0);
    }
    sprintf(buffer,"support-%05d.png",opts->cur_iteration);    
    write_png(support,buffer,COLOR_JET);
  }
  /* restore original amplitudes */
  for(i = 0;i<TSIZE(amp);i++){
    real_out->image[i] = norm(real_out,i);
  }


  opts->cur_iteration++;
  return TRUE;
}

boolean test_seed(population *pop, entity *adam){
  int i;

  /* Checks. */
  if (!pop) die("Null pointer to population structure passed.");
  if (!adam) die("Null pointer to entity structure passed.");  
  /* Seeding. */
  for(i = 0;i<TSIZE(amp);i++){
    ((double *)adam->chromosome[0])[i] = real_out->r[i]*(1-random_double(0.1));
    ((double *)adam->chromosome[0])[TSIZE(amp)+i] = real_out->c[i]*(1.0-random_double(0.1));
  }
  return TRUE;
}


void genetic_reconstruction(Image * _amp, Image * initial_support, Image * _exp_sigma,
			     Options * _opts, char * dir){

  char prev_dir[1024];
  real support_threshold = _opts->new_level;
  population *pop;			/* Population of solutions. */

  random_seed(23091975);
  stop_threshold = 10;
  stop = 0;
  support_size = -support_threshold;
  opts = _opts;
  amp = _amp;
  exp_sigma = _exp_sigma;
  
  init_log(&my_log);
  my_log.threshold = support_threshold;
  opts->cur_iteration = 0;
  opts->flog = NULL;
  if(opts->automatic){
    opts->algorithm = HIO;
  }
  
  support = imgcpy(initial_support);
  prev_support = imgcpy(initial_support);

  /* Set the initial guess */
  if(opts->image_guess){
    real_in = imgcpy(opts->image_guess);
  }else{
    real_in = imgcpy(support);
  }

  /* make sure we make the input complex */
  rephase(real_in);
  
  /* Set random phases if needed */
  if(opts->rand_phases){
    /*    set_rand_phases(real_in,img);*/
    set_rand_ints(real_in,amp);
  }

  getcwd(prev_dir,1024);
  mkdir(dir,0755);
  chdir(dir);
  write_png(support,"support.png",COLOR_JET);
  write_png(real_in,"initial_guess.png",COLOR_JET);
  write_png(initial_support,"initial_support.png",COLOR_JET);

  if(get_algorithm(opts,&my_log) == HIO){     
    real_out = basic_hio_iteration(amp, real_in, support,opts,&my_log);
  }else if(get_algorithm(opts,&my_log) == RAAR){
    real_out = basic_raar_iteration(amp,exp_sigma, real_in, support,opts,&my_log);
  }else if(get_algorithm(opts,&my_log) == HPR){
    real_out = basic_hpr_iteration(amp, real_in, support,opts,&my_log);
  }else{
    fprintf(stderr,"Error: Undefined algorithm!\n");
    exit(-1);
  }

  radius = opts->max_blur_radius;

  
  pop = ga_genesis_double(
			  3,			/* const int              population_size */
			  1,			/* const int              num_chromo */
			  TSIZE(amp)*2,	/* const int              len_chromo */
			  test_generation_callback,/* GAgeneration_hook      generation_hook */
			  NULL,			/* GAiteration_hook       iteration_hook */
			  NULL,			/* GAdata_destructor      data_destructor */
			  NULL,			/* GAdata_ref_incrementor data_ref_incrementor */
			  test_score,		/* GAevaluate             evaluate */
			  test_seed,		/* GAseed                 seed */
			  test_adaptation,	/* GAadapt                adapt */
			  ga_select_one_bestof2,	/* GAselect_one           select_one */
			  ga_select_two_bestof2,	/* GAselect_two           select_two */
			  ga_mutate_double_singlepoint_drift,	/* GAmutate               mutate */
			  ga_crossover_double_doublepoints,	/* GAcrossover            crossover */
			  NULL,			/* GAreplace              replace */
			  NULL			/* vpointer	User data */
			  );


  ga_population_set_parameters(
       pop,				/* population      *pop */
       GA_SCHEME_LAMARCK_ALL,		/* const ga_scheme_type     scheme */
       GA_ELITISM_PARENTS_SURVIVE,	/* const ga_elitism_type   elitism */
       0.8,				/* double  crossover */
       0.2,				/* double  mutation */
       0.0      		        /* double  migration */
                              );

  ga_evolution(
       pop,				/* population	*pop */
       500				/* const int	max_generations */
              );

  ga_extinction(pop);
  exit(EXIT_SUCCESS);
}


