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
  uwrapc_main(argc,argv);
  return 0;
}
