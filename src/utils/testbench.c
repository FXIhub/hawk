#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <gsl/gsl_multimin.h>

#include "spimage.h"

#include "kinsol.h"
#include "nvector_serial.h"
#include "sundialstypes.h"
#include "sundialsmath.h"
#include "kinspgmr.h"


#define Ith(v,i)    NV_Ith_S(v,i)   
#define FTOL   RCONST(1.e-9)  /* function tolerance */
#define STOL   RCONST(1.e-9) /* step tolerance */

typedef struct{
  Image * autocorrelation;
  Image * deconvoluted;
  Image * current_solution;
}Userdata;



static int check_flag(void *flagvalue, char *funcname, int opt)
{
  int *errflag;

  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, 
            "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
    funcname);
    return(1);
  }

  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr,
              "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
      funcname, *errflag);
      return(1); 
    }
  }

  else if (opt == 2 && flagvalue == NULL) {
    fprintf(stderr,
            "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
    funcname);
    return(1);
  }

  return(0);
}


static void error_func(N_Vector u, N_Vector f, void *f_data){
  int i;
  realtype * fdata = NV_DATA_S(f);
  realtype * udata = NV_DATA_S(u);
  Userdata * data = (Userdata *)f_data;

  data->deconvoluted->image = udata;
  data->deconvoluted->amplitudes = udata;

  data->current_solution = cross_correlate_img(data->deconvoluted,data->deconvoluted);
  for(i = 0;i<TSIZE(data->deconvoluted);i++){
    fdata[i] = data->current_solution->image[i] - data->autocorrelation->image[i];
  }  
  freeimg(data->current_solution);

}

double my_f (const gsl_vector *v, void *params){
  Userdata *data = (Userdata *)params;
  double error = 0;
  int i;
  data->deconvoluted->image = v->data;
  data->deconvoluted->amplitudes = v->data;
  
  data->current_solution = cross_correlate_img(data->deconvoluted,data->deconvoluted);
  for(i = 0;i<TSIZE(data->deconvoluted);i++){
    error += fabs(data->current_solution->image[i] - data->autocorrelation->image[i]);
  }    
  freeimg(data->current_solution);
  return error;
}

static void set_initial_guess(N_Vector u, Userdata * data){
  realtype *udata;
  int i;
  
  udata = NV_DATA_S(u);
  for(i = 0;i<TSIZE(data->deconvoluted);i++){
    udata[i] = fabs(data->autocorrelation->image[i]);
  }
}

static int solve_it(void *kmem, N_Vector u, N_Vector s, int glstr){
  int flag;
  flag = KINSol(kmem, u, glstr, s, s);
  check_flag(&flag, "KINSol", 1);
  printf("flag - %d\n",flag);
  return(0);

}

/*int main(int argc, char ** argv){
  N_Vector u, s, c;
  int glstr, flag;
  void *kmem;
  realtype fnormtol, scsteptol;
  Image * solution = read_imagefile(argv[1]);
  Image * autocorrelation = cross_correlate_img(solution,solution);
  int i;  
  int maxl,maxlrst;
  int size = TSIZE(autocorrelation);
  realtype one = 2.0;
  Userdata data;
  srand48(30);


  data.autocorrelation = autocorrelation;
  data.deconvoluted = imgcpy(solution);
  data.current_solution = imgcpy(solution);
  
  srand48(time(NULL));

  write_png(autocorrelation,"autocorr.png",COLOR_JET|LOG_SCALE);
  u = N_VNew_Serial(size);
  set_initial_guess(u,&data);
  s = N_VNew_Serial(size);
  c = N_VNew_Serial(size);

  free(data.deconvoluted->image);


  N_VConst_Serial(1.0,s);
  N_VConst_Serial(1.0,c);
  fnormtol=FTOL; scsteptol=STOL;

  glstr = KIN_INEXACT_NEWTON;
  kmem = KINCreate();
  flag = KINSetFdata(kmem, &data); 
  flag = KINSetConstraints(kmem, c);
  flag = KINSetFuncNormTol(kmem, fnormtol);
  flag = KINSetScaledStepTol(kmem, scsteptol);
  flag = KINSetNumMaxIters(kmem,1);
  flag = KINMalloc(kmem, error_func, u);
  maxl = 400; 
  maxlrst = 20;
  flag = KINSpgmr(kmem, maxl);
  flag = KINSpgmrSetMaxRestarts(kmem, maxlrst);


  solve_it(kmem, u, s, glstr);
  data.deconvoluted->image = NV_DATA_S(u);
  data.deconvoluted->amplitudes = NV_DATA_S(u);

  write_png(data.deconvoluted,"best_guess.png",COLOR_JET|LOG_SCALE);
  write_img(data.deconvoluted,"best_guess.h5",sizeof(real));
  write_png(cross_correlate_img(data.deconvoluted,data.deconvoluted),"best_guess_corr.png",COLOR_JET|LOG_SCALE);
  return 0;  
}

*/
int main(int argc, char ** argv)
{
  Image * solution = read_imagefile(argv[1]);
  Image * autocorrelation = cross_correlate_img(solution,solution);
  int i;  
  size_t np = TSIZE(autocorrelation);  
  Userdata data;
  double * tmp;


  data.autocorrelation = autocorrelation;
  data.deconvoluted = imgcpy(solution);
  data.current_solution = imgcpy(solution);
  
  const gsl_multimin_fminimizer_type *T =
    gsl_multimin_fminimizer_nmsimplex;
  gsl_multimin_fminimizer *s = NULL;
  gsl_vector *ss, *x;
  gsl_multimin_function minex_func;
  
  size_t iter = 0;
  int status;
  double size;
  
  /* Initial vertex size vector */
  ss = gsl_vector_alloc (np);
     
  /* Set all step sizes to 1 */
  gsl_vector_set_all (ss, 1.0);
     
  /* Starting point */
  x = gsl_vector_alloc (np);
/*  gsl_vector_set_all (x, 1.0);*/
  tmp = gsl_vector_ptr(x,0);
  for(i = 0;i<np;i++){
    tmp[i] = 0;
  }
  /* Initialize method and iterate */
  minex_func.f = &my_f;
  minex_func.n = np;
  minex_func.params = (void *)&data;
     
  s = gsl_multimin_fminimizer_alloc (T, np);
  gsl_multimin_fminimizer_set (s, &minex_func, x, ss);
  
  do{
    iter++;
    status = gsl_multimin_fminimizer_iterate(s);
    
    if (status)
      break;
    
    size = gsl_multimin_fminimizer_size (s);
    status = gsl_multimin_test_size (size, 1e-2);
    
    if (status == GSL_SUCCESS){
      printf ("converged to minimum at\n");
    }
    
    printf ("%5d ", iter);
    for (i = 0; i < np; i++){
      printf ("%10.3e ", gsl_vector_get (s->x, i));
    }
    printf ("f() = %7.3f size = %.3f\n", s->fval, size);
  }
  while (status == GSL_CONTINUE && iter < 100);

  data.deconvoluted->image = gsl_vector_ptr(x,0);
  data.deconvoluted->amplitudes = gsl_vector_ptr(x,0);

  write_png(data.deconvoluted,"best_guess.png",COLOR_JET|LOG_SCALE);
  write_img(data.deconvoluted,"best_guess.h5",sizeof(real));
  write_png(cross_correlate_img(data.deconvoluted,data.deconvoluted),"best_guess_corr.png",COLOR_JET|LOG_SCALE);  
  gsl_vector_free(x);
  gsl_vector_free(ss);
  gsl_multimin_fminimizer_free (s);
  
  return status;
}
