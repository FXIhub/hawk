#ifdef MPI
#include "cimage.h"
#include "mpi_comm.h"

void sync_img(Image * img){
  int id;
  int p;
  int i;
  int target;
  MPI_Status status;
  MPI_Comm_rank(MPI_COMM_WORLD,&id);
  MPI_Comm_size(MPI_COMM_WORLD,&p);
  for(i = 1;i<p;i++){
    if(i == id){
      continue;
    }
    if(id %2){
      target = (i+id)%p;
      MPI_Send(&(img->image[id*TSIZE(img)/p]),TSIZE(img)/p,MPI_FLOAT,id,target,0,MPI_COMM_WORLD);
      MPI_Receive(&(img->image[i*TSIZE(img)/p]),TSIZE(img)/p,MPI_FLOAT,target,id,0,MPI_COMM_WORLD,&status);
    }else{
      target = (id-i)%p;
      MPI_Receive(&(img->image[i*TSIZE(img)/p]),TSIZE(img)/p,MPI_FLOAT,i,target,0,MPI_COMM_WORLD,&status);
      MPI_Send(&(img->image[id*TSIZE(img)/p]),TSIZE(img)/p,MPI_FLOAT,target,i,0,MPI_COMM_WORLD);
    }
  }
}

#endif
