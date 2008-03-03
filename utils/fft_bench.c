#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

#include "spimage.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <time.h>
# include <windows.h>
#include <direct.h>
#ifdef _TIMEVAL_DEFINED /* also in winsock[2].h */
#define _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif /* _TIMEVAL_DEFINED */
#else
#  include <sys/time.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
int gettimeofday (struct timeval *tv, void* tz) 
{ 
  union { 
    long long ns100; /*time since 1 Jan 1601 in 100ns units */ 
    FILETIME ft; 
  } now; 

  GetSystemTimeAsFileTime (&now.ft); 
  tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL); 
  tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL); 
  return (0); 
}
#endif

int max_prime_factor(int a){
  int i;
  for(i = 2;i*i<=a;i++){
    while(a % i == 0 && i*i<=a){
      a /= i;
    }
  }
  return a;
}


void create_fft_bench_table(int iter, FILE * file, int nthreads){
  /* Check all sizes from 10^2 to 2000^2 with prime factors smaller than 8 */
  Image * test = sp_image_alloc(1,1,1);
  int i,j;
  int opt_size = 2000;
  int time_i,time_e;
  struct timeval tv_i;
  struct timeval tv_e;
  int min_time = 1000000000;
  sp_init_fft(nthreads);
  for(i = 200;i>190;i--){
    if(max_prime_factor(i) >= 8){
      fprintf(file,"%d\t%d\t%d\n",i,-1,opt_size);
      continue;
    }
    sp_image_realloc(test,i,i,i);
    test->scaled = 1; 
    test->phased = 1;
    test->shifted = 1;


    for(j = 0;j<sp_image_size(test);j++){
      sp_real(test->image->data[i]) = sp_box_muller(0,200);
      sp_imag(test->image->data[i]) = sp_box_muller(0,200);
    }
    gettimeofday(&tv_i,NULL);
    time_i = tv_i.tv_sec*1000000+tv_i.tv_usec;
    for(j = 0;j<iter;j++){
      sp_image_free(sp_image_fft(test));
      sp_image_free(sp_image_ifft(test));
    }    
    gettimeofday(&tv_e,NULL);
    time_e = tv_e.tv_sec*1000000+tv_e.tv_usec;
    if(time_e-time_i < min_time){
      min_time = time_e-time_i;
      opt_size = i;
      fprintf(file,"%d\t%d\t%d\n",i,time_e-time_i,opt_size);    
    }else{
      fprintf(file,"%d\t%d\t%d\n",i,-1,opt_size);    
    }

    fflush(file);
  }
}

int main(int argc, char ** argv){
  FILE * f;
  char buffer2[1024];
  char * home_dir;
  int nthreads = 1;
  if(argc > 1){
    nthreads = atoi(argv[1]);
  }
#if defined(_MSC_VER) || defined(__MINGW32__)
  home_dir = getenv("USERPROFILE");
  sprintf(buffer2,"%s/.uwrapc",home_dir);
  _mkdir(buffer2);
#else
  home_dir = getenv("HOME");
  sprintf(buffer2,"%s/.uwrapc",home_dir);
  mkdir(buffer2,0755);
#endif
  sprintf(buffer2,"%s/.uwrapc/fft_speed",home_dir);
  f = fopen(buffer2,"w");
  create_fft_bench_table(1,f,nthreads);
  fclose(f);
  return 0;
}
