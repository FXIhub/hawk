#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include <unistd.h>

#ifdef MPI
#include <mpi.h>
#endif
#ifdef _USE_DMALLOC
#include <dmalloc.h>
#endif


#include "spimage.h"
#include "log.h"
#include "uwrapc.h"
#include "configuration.h"
#include "algorithms.h"
#include "support.h"
#include "network_communication.h"

int main(int argc, char ** argv){
  FILE * f;
  Options * opts = &global_options;
  void * socket = 0;
#ifdef MPI
  MPI_Init(&argc, &argv);
#endif  
  set_defaults(opts);

#ifdef NETWORK_SUPPORT
  /* Decide if should connect to hawk GUI or not */
  init_qt(argc,argv);
  char * server = 0;
  int server_port = 0;
  int i;
  if(argc != 1){
    if(argc == 3 || argc == 5){
      for(i = 1;i<argc-1;i++){
	/*      for(i = 1;i<argc-1;i++){ */
	if(strcmp(argv[i],"-s") == 0){
	  server = argv[i+1];
	}else if(strcmp(argv[i],"-p") == 0){
	  server_port = atoi(argv[i+1]);
	}
      }
      socket = attempt_connection(server,server_port);
    }else{
      printf("Usage: uwrapc [-s server [-p port]]\n");
    }
  }
#else
  if(argc != 1){
    perror("uwrapc not compileed with network support!\n");
  }
#endif  

  
  f = fopen("uwrapc.conf","rb");
  if(f){
    fclose(f);
    read_options_file("uwrapc.conf");
    check_options_and_load_images(opts);
    char buffer[OPTION_STRING_SIZE*2+1];
#if defined(_MSC_VER) || defined(__MINGW32__)
    _mkdir(opts->work_dir);
#else
    mkdir(opts->work_dir,0755);
#endif
    strcpy(buffer,opts->work_dir);
    strcat(buffer,"/");
    strcat(buffer,"uwrapc.confout");
    write_options_file(buffer);
  }else if(!socket){
    perror("Could not open uwrapc.conf");
  }
  srand(get_random_seed(opts));

#ifdef NETWORK_SUPPORT
  if(socket){
    wait_for_server_instructions(socket);
  }
#endif
  init_reconstruction(opts);
  /* cleanup stuff */
  if(opts->init_support){
    sp_image_free(opts->init_support);
  }
  if(opts->diffraction){
    sp_image_free(opts->diffraction);
  }
  if(opts->amplitudes){
    sp_image_free(opts->amplitudes);
  }
  if(opts->intensities_std_dev){
    sp_image_free(opts->intensities_std_dev);
  }
#ifdef NETWORK_SUPPORT
  cleanup_and_free_qt();
#endif
#ifdef _USE_DMALLOC
  dmalloc_shutdown();
#endif
  return 0;  
}



