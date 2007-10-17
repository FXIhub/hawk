#include <stdlib.h>
#include <math.h>
#include <spimage.h>
#include <minmaxtau.h>
#include <assert.h>


void rebracketxy(sp_vector * xmin,sp_vector * xmax,real x,real y,int init){
  if(init){
    xmin->data[0] = x;
    xmin->data[1] = y;
    xmax->data[0] = REAL_MAX;
    xmax->data[1] = -REAL_MAX;
    return;
  }
  if(y<0 && x<xmax->data[0]){
    xmax->data[0] = x;
    xmax->data[1] = y;
  }
  if(y>0 && x>xmin->data[0]){
    xmin->data[0] = x;
    xmin->data[1] = y;
  }
}
								       


void bisect2(double x0, double x1, double y0, double y1, double dy, sp_vector * xmax, sp_vector * xmin, double *xn, double * dyn){
  /*
    %[xn,dyn]=bisect2(x0,x1,y0,y1,dy,xmax,xmin)
    % use values at the interval between
    % x0 to x1 with x1 forced to be inside the brackets
    % and derivative dy
    % brackets are defined by
    % xmin=[x min; y min];
    % xmax=[x max; y max];
    %
   */
  double Dx,Dy,d2y;

  if(x1>xmax->data[0]){  /*%too big*/
    x1=xmax->data[0];
    y1=xmax->data[1];
  }
  if(x1<xmin->data[0]){ /* %too small */
    x1=xmin->data[0];
    y1=xmin->data[1];
  }

 /* %calculate differences */ 
  Dx=(x1-x0);
  Dy=(y1-y0);
  
  /* %derivative (average) */

  assert(Dx != 0);
  *dyn=Dy/Dx;


  /*   %second derivative (using 'dy' as first derivative at x0) */
  d2y=(Dy-Dx*dy)/(Dx*Dx);                /*   %second derivative */
  assert(d2y != 0);
  *xn=x0+(-dy-sqrt(dy*dy-4*y0*d2y))/d2y/2; /* %root (the one inside the brackets) */
  *dyn=dy+d2y*(*xn-x0);                     /* %update derivative */

/* %if we went out of the brackets using 2nd order
   %use false position method to be safe
*/
  if (*xn>xmax->data[0] || *xn<xmin->data[0]){
    *dyn=Dy/Dx;                         /*  %update derivative (linear) */
    *xn=x0-x1*(*dyn);
  }
}

sp_vector * gradL(sp_c3matrix * Gs, sp_c3matrix * Gns, sp_c3matrix * DGs, sp_c3matrix * DGns, Image * F0, real aa, real bb){
  sp_vector * dab = sp_vector_alloc(2);
  sp_c3matrix * Gab;
  sp_c3matrix * Gm;
  long long i;
  Complex x;
  /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
  Gab = sp_c3matrix_duplicate(Gs);    
  sp_c3matrix_add(Gab,Gns,NULL);

  x = sp_cinit(aa,0);
  sp_c3matrix_add(Gab,DGs,&x);
  x = sp_cinit(bb,0);
  sp_c3matrix_add(Gab,DGns,&x);
  /*
    Gm=Gab./abs(Gab).*F0;
  */
  Gm = sp_c3matrix_duplicate(Gab);
  for(i = 0;i<sp_c3matrix_size(Gab);i++){
    assert(sp_cabs(Gm->data[i]) != 0);
    if(F0->mask->data[i]){
      Gm->data[i] = sp_cscale(Gm->data[i],sp_real(F0->image->data[i])/sp_cabs(Gm->data[i]));
    }
  }
  
  /*

  dab(1)=-cprod(DGs,Gm-Gab);                      %-<DGs |[Pm -I]G>
  dab(2)=-cprod(DGns,Gm);                     %-<DGns|Pm G>

   */
  dab->data[1] = -sp_real(sp_c3matrix_froenius_prod(DGns,Gm));
  sp_c3matrix_sub(Gm,Gab);
  dab->data[0] = -sp_real(sp_c3matrix_froenius_prod(DGs,Gm));
  sp_c3matrix_free(Gm);
  sp_c3matrix_free(Gab);
  return dab;
}

real linminab(sp_c3matrix * Gs, sp_c3matrix * Gns, sp_c3matrix * DGs, sp_c3matrix * DGns, Image * F0, sp_vector * ab, sp_vector * dab, sp_3matrix * Habi, real TolY){
  /*
    % function x=linminab(Gs,Gns,DGs,DGns,F0,ab,dab,Habi);
    % line search for ab:
    % minimize <(Hab^-1 gradL(ab))|Hab^-1 gradL(ab+x*dab)>
    %
    % using Newton and bracketing, bisecant, and quasi newton methods
    % Gs,Gns, DGs, DGns, F0 are the usual
    % ab is the initial value
    % dab is the direction in wich we are going,
    % Habi is the inverse of the hessian
    %
   */
  int maxiter = 10; /* don't waste too much time */
  double y,yold,dy;
  sp_vector * tmp;
  sp_vector * tmp2;
  sp_vector * xmin = sp_vector_alloc(2);
  sp_vector * xmax = sp_vector_alloc(2);
  sp_3matrix * minus_Habi = sp_3matrix_alloc(2,2,1);
  double dyn,xold,x;
  double Dx,Dy,xn;
  double errslp,d2y;
  int ii;
  

/*  fprintf(stdout,"linminab\n");*/

  sp_3matrix_memcpy(minus_Habi,Habi);
  sp_3matrix_scale(minus_Habi,-1);
  if(TolY < 0){
    TolY = 1e-1;
  }

  /*
    %----------------------------------------------------------------
    %numerical derivative:, just to make sure that we are right
    % x=1e-4;
    % y1=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x));
    % y0=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1),ab(2)));
    % (y1-y0)/x;
    % and the analytic one:
    % dy=cprod(dab,-Habi*HesL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x)*dab);
    %-------------------------------------------------------------
r  */

  /* start from 0 step length */
  x = 0;
  /* %function to send to 0 (but we are forcing it to go toward a min */
  /* y=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x)); */
  tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
  tmp2 = sp_3matrix_vector_prod(minus_Habi,tmp);
  y = sp_vector_dot_prod(dab,tmp2);
  sp_vector_free(tmp);
  sp_vector_free(tmp2);

  /*  %to check for overshooting: we'll use bisecant and fpm if so */
  yold = y;
  xold = x;
  /* %derivative (at 0 it should be dy=-y if the Hessian is right) */
  dy = -y;
  /* %bracketing values (the 1 at the end initializes) */
  rebracketxy(xmin,xmax,x,y,1);
  
  for(ii = 0;ii<maxiter;ii++){
    /*

    %force the derivative to be negative: (climb out of max)
    dy=-abs(dy);
    */
    dy=-fabs(dy);
    /*
    %try newton step first
    Dx=-y/dy;
    x=x+Dx;
    */
    assert(dy != 0);
    Dx=-y/dy;
    assert(Dx != 0);
    x=x+Dx;
    

    /*
      %calculate new value
      y=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x));
    */
    tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
    tmp2 = sp_3matrix_vector_prod(minus_Habi,tmp);
    y = sp_vector_dot_prod(dab,tmp2);
    sp_vector_free(tmp);
    sp_vector_free(tmp2);

    /*
    if abs(y)<TolY; return;end %small enough, get out!
    */
    if(fabs(y) < TolY){
      sp_vector_free(xmin);
      sp_vector_free(xmax);
      sp_3matrix_free(minus_Habi);
      return x;
    }
    /*
    %bracket values
    [xmin,xmax]=rebracketxy(xmin,xmax,x,y);
    */
    rebracketxy(xmin,xmax,x,y,0);

    /*
    if y.*yold<0; %overshot: go back using a quadratic or false position
    */
    if(y*yold<0){
      /*
        [xn,dyn]=bisect2(xold,x,yold,y,dy,xmax,xmin);
      */
      bisect2(xold,x,yold,y,dy,xmax,xmin,&xn,&dyn);
      /*
	x=xn;           %new point
	dy=dyn;         %new derivative
      */
      x = xn;
      dy = dyn;
      /*
      y=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x));
      */
      tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
      tmp2 = sp_3matrix_vector_prod(minus_Habi,tmp);
      y = sp_vector_dot_prod(dab,tmp2);
      sp_vector_free(tmp);
      sp_vector_free(tmp2);
      /*
        if abs(y)<TolY; return;end %small enough, get out!
      */
      if(fabs(y) < TolY){
	return x;
      }
      /*
        [xmin,xmax]=rebracketxy(xmin,xmax,x,y);
        yold=y;
        xold=x;
      */
      rebracketxy(xmin,xmax,x,y,0);
      yold=y;
      xold=x;
    }else{
      /*    else% we didn't overshoot, keep going but update the hessian
	    
      Dx=(x-xold);
      Dy=(y-yold);
      */
      Dx=(x-xold);
      Dy=(y-yold);
      
      /*
        dyn=Dy/Dx; %true slope (linear)
        errslp=abs((dyn-dy)/(dy+i*eps))^2; %slope error
	
	I don't get the i on the previous line
      */
      assert(Dx != 0);
      dyn=Dy/Dx;
      errslp=fabs((dyn-dy)/((dy+1*REAL_EPSILON)));
      errslp *= errslp;
      
      if(errslp>0.25){
	/*
	  dy=cprod(dab,-Habi*hesL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x)*dab);
	*/
	tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
	tmp2 = sp_3matrix_vector_prod(minus_Habi,tmp);
	dy = sp_vector_dot_prod(dab,tmp2);
	sp_vector_free(tmp);
	sp_vector_free(tmp2);
      }else if(errslp > 0.1){
	/* elseif errslp>.1; dy=dyn; %new derivative, linear*/
	dy = dyn;
      }else{
	/*   %second derivative (assuming 'dy' as first derivative at x0)
	     d2y=(Dy-Dx*dy)/Dx.^2;
	     % xn=x0+(-dy-sqrt(dy.^2-4*y0*d2y))/d2y/2; %root
	     %           dy*(Dx+d2y*Dx^2*xx)
	     dy=dy+d2y*Dx;                        %update derivative
	*/	    
	assert(Dx != 0);
	d2y=(Dy-Dx*dy)/(Dx*Dx);
	dy=dy+d2y*Dx;	    
      }
    }
    
  }  
/*  fprintf(stdout,"linminab roger and out\n");*/

  return x;
}


void Hfit(sp_3matrix * _taui, sp_3matrix * _dtaui, int iter, sp_vector * tau, sp_vector * dtau, sp_vector * Dtau, sp_3matrix * Hi){
  /* linear fit */
  sp_3matrix * dtaui = sp_3matrix_alloc(2,iter,1);
  sp_3matrix * taui = sp_3matrix_alloc(2,iter,1);
  sp_vector * dtaum = sp_vector_alloc(2);
  sp_vector * taum = sp_vector_alloc(2);
  sp_3matrix * x = sp_3matrix_alloc(2,iter,1);
  sp_3matrix * y = sp_3matrix_alloc(2,iter,1);
  sp_3matrix * x_trans = sp_3matrix_alloc(2,iter,1);
  sp_3matrix * y_trans = sp_3matrix_alloc(2,iter,1);
  sp_3matrix * H;
  sp_3matrix * tmp;
  sp_3matrix * tmp2;
  sp_vector * taur = sp_vector_alloc(2);
  real r;
  long long i;
  int jj = 0;

  real mintaultmp;
  /*
   * dtaum=mean(dtaui,2);
   * taum=mean(taui,2);
   */

  for(i = 0;i<iter;i++){
    sp_3matrix_set(dtaui,0,i,0,sp_3matrix_get(_dtaui,0,i,0));
    sp_3matrix_set(dtaui,1,i,0,sp_3matrix_get(_dtaui,1,i,0));
    sp_3matrix_set(taui,0,i,0,sp_3matrix_get(_taui,0,i,0));
    sp_3matrix_set(taui,1,i,0,sp_3matrix_get(_taui,1,i,0));
  }
  for(i = 0;i<sp_3matrix_x(dtaui);i++){
    dtaum->data[0] += sp_3matrix_get(dtaui,0,i,0);
    dtaum->data[1] += sp_3matrix_get(dtaui,1,i,0);
    taum->data[0] += sp_3matrix_get(taui,0,i,0);
    taum->data[1] += sp_3matrix_get(taui,1,i,0);
  }
  dtaum->data[0] /= sp_3matrix_x(dtaui);
  dtaum->data[1] /= sp_3matrix_x(dtaui);
  taum->data[0] /= sp_3matrix_x(dtaui);
  taum->data[1] /= sp_3matrix_x(dtaui);
  /* x=[taui(1,:)-taum(1);taui(2,:)-taum(2)]; 
   * y=[dtaui(1,:)-dtaum(1);dtaui(2,:)-dtaum(2)];
   */
  for(i = 0;i<sp_3matrix_x(dtaui);i++){
    sp_3matrix_set(x,0,i,0,sp_3matrix_get(taui,0,i,0)-taum->data[0]);
    sp_3matrix_set(x,1,i,0,sp_3matrix_get(taui,1,i,0)-taum->data[1]);
    sp_3matrix_set(y,0,i,0,sp_3matrix_get(dtaui,0,i,0)-dtaum->data[0]);
    sp_3matrix_set(y,1,i,0,sp_3matrix_get(dtaui,1,i,0)-dtaum->data[1]);
  }
  sp_3matrix_memcpy(x_trans,x);
  sp_3matrix_transpose(x_trans);
  sp_3matrix_memcpy(y_trans,y);
  sp_3matrix_transpose(y_trans);
  
  /* %Fitted Hessian */
  /* H=(x*x')^-1*x*y'; */
  tmp = sp_3matrix_mul(x,x_trans);
  sp_3matrix_invert(tmp);
  tmp2 = sp_3matrix_mul(x,y_trans);
  H = sp_3matrix_mul(tmp,tmp2);
  sp_3matrix_free(tmp);
  sp_3matrix_free(tmp2);
  
  /* Hi=H^-1; */
  sp_3matrix_memcpy(Hi,H);
  sp_3matrix_invert(Hi);

  /* fitted root */
  tmp = sp_3matrix_mul(Hi,dtaui);
  for(i = 0;i<sp_3matrix_x(dtaui);i++){
    taur->data[0] += -(sp_3matrix_get(tmp,0,i,0)-sp_3matrix_get(taui,0,i,0));
    taur->data[1] += -(sp_3matrix_get(tmp,1,i,0)-sp_3matrix_get(taui,1,i,0));
  }
  taur->data[0] /= dtaui->x; 
  taur->data[1] /= dtaui->x; 
  
  /* Newton with line search, go back to minimum value */
  /* Use smallest gradient */
  mintaultmp = 0;
  for(i = 0;i<sp_3matrix_x(dtaui);i++){
    r = sqrt(sp_3matrix_get(dtaui,0,i,0)*sp_3matrix_get(dtaui,0,i,0)+sp_3matrix_get(dtaui,1,i,0)*sp_3matrix_get(dtaui,1,i,0));
    if(r < mintaultmp || !mintaultmp){
      mintaultmp = r;
      jj = i;
    }
  }
  /* tau=taui(:,jj);  */
  /*go back to that point  */
  tau->data[0] = sp_3matrix_get(taui,0,jj,0);
  tau->data[1] = sp_3matrix_get(taui,1,jj,0);
  /* %use its gradient */
  /*  dtau=dtaui(:,jj); */
  dtau->data[0] = sp_3matrix_get(dtaui,0,jj,0); 
  dtau->data[1] = sp_3matrix_get(dtaui,1,jj,0);
  /* step to root */
  /* Dtau=tau-taur; */
  Dtau->data[0] = tau->data[0]-taur->data[0];
  Dtau->data[1] = tau->data[1]-taur->data[1];      
  sp_3matrix_free(dtaui);
  sp_3matrix_free(taui);
  sp_3matrix_free(x_trans);
  sp_3matrix_free(y_trans);
}

void goright(sp_3matrix * H){
  int ND = H->y;
  sp_vector *  rightdir = sp_vector_alloc(ND);
  sp_vector *  isright = sp_vector_alloc(ND);
  real Hmin;
  sp_3matrix * redM = sp_3matrix_alloc(ND,ND,0);
  sp_3matrix * tmp;
  sp_3matrix * tmp2;
  int i;
  /* rightdir=repmat([1,-1],1,ND/2); %alternate min-max */
  for(i = 0;i<ND/2;i++){
    rightdir->data[2*i] = 1;
    rightdir->data[2*i+1] = -1;
  }

  /* modify diagonal elements so that the sign is right */
  /* H(sub2ind(size(H),1:ND,1:ND))=rightdir.*abs(H(sub2ind(size(H),1:ND,1:ND))); */
  for(i = 0;i<ND;i++){
    sp_3matrix_set(H,i,i,0,fabs(sp_3matrix_get(H,i,i,0))*rightdir->data[i]);
  }
  /* %reduce wrong elements to 1/2 of smallest right element */
  /* diagH=diag(H) */
  /* isright=diagH'.*rightdir>0 */

  /* I have the slight feeling this isright is always gonna be filled with 1s */

  /* Hmin=min(abs(H(sub2ind(size(H),find(isright),find(isright))))); */
  Hmin = 0;
  for(i = 0;i<ND;i++){
    if(sp_3matrix_get(H,i,i,0)*rightdir->data[i] > 0){
      sp_vector_set(isright,i,1);
      if(fabs(sp_3matrix_get(H,i,i,0)) < Hmin || !Hmin){ 
	Hmin = fabs(sp_3matrix_get(H,i,i,0));
      }
    }else{
      sp_vector_set(isright,i,0);
    }
  }
  /* redM=diag(sqrt(Hmin/1)./sqrt(abs(diag(H))).*(~isright'))+diag(isright); */
  for(i = 0;i<ND;i++){
    if(isright->data[i]){
      sp_3matrix_set(redM,i,i,0,1);
    }else{
      sp_3matrix_set(redM,i,i,0,sqrt(Hmin)/sqrt(fabs(sp_3matrix_get(H,i,i,0))));
    }
  }
  /* H=redM*H*redM; */
  tmp = sp_3matrix_mul(redM,H);
  tmp2 = sp_3matrix_mul(tmp,redM);
  sp_3matrix_memcpy(H,tmp2);
  sp_3matrix_free(tmp);
  sp_3matrix_free(tmp2);
  sp_3matrix_free(redM);
  if(ND==4){
    /* make sure eigenvalues of alpha1,alpha2 hessian and beta1,beta2 hessian
     * H([1,3],[1,3]),H([2,4],[2,4]) are positive
     */
    if(sp_3matrix_get(H,2,4,0)*sp_3matrix_get(H,2,4,0) > sp_3matrix_get(H,2,2,0)*sp_3matrix_get(H,4,4,0)){
      /* remove diag components if too big */
      sp_3matrix_set(H,2,4,0,0);
      sp_3matrix_set(H,4,2,0,0);
    }
    if(sp_3matrix_get(H,1,3,0)*sp_3matrix_get(H,1,3,0) > sp_3matrix_get(H,1,1,0)*sp_3matrix_get(H,3,3,0)){
      /* remove diag components if too big */
      sp_3matrix_set(H,1,3,0,0);
      sp_3matrix_set(H,3,1,0,0);
    }
  }
}

int hesLtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,sp_vector * tau,sp_3matrix * Hab, sp_3matrix * Habi){
  sp_c3matrix * Gab;
  sp_c3matrix * Fratio;
  sp_c3matrix * tmp;
  sp_c3matrix * ph2;
  real rtmp;
  Complex x;
  long long i;
  if(!tau){
    Gab = sp_c3matrix_duplicate(Gs);
    sp_c3matrix_add(Gab,Gns,NULL);
  }else{
    /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
    Gab = sp_c3matrix_duplicate(Gs);    
    sp_c3matrix_add(Gab,Gns,NULL);    
    x = sp_cinit(tau->data[0],0);
    sp_c3matrix_add(Gab,DGs,&x);
    x = sp_cinit(tau->data[1],0);
    sp_c3matrix_add(Gab,DGns,&x);
  }
/*  fprintf(stdout,"HesLtau\n"); */

/* Fratio = F0/abs(Gab);  */
  Fratio = sp_c3matrix_duplicate(F0->image);
  for(i = 0;i<sp_c3matrix_size(Gab);i++){
    assert(sp_cabs(Gab->data[i]) != 0);
    if(F0->mask->data[i]){
      sp_real(Fratio->data[i]) /= sp_cabs(Gab->data[i]);
    }else{
      Fratio->data[i] = sp_cinit(1,0);
    }
  }
/* ph2 = (Gab/abs(Gab))^2 * Fratio */
  ph2 = sp_c3matrix_duplicate(Gab);
  for(i = 0;i<sp_c3matrix_size(ph2);i++){
    assert(sp_cabs(ph2->data[i]) != 0);
    ph2->data[i] = sp_cmul(sp_cmul(sp_cscale(ph2->data[i],1.0/sp_cabs(ph2->data[i])),(sp_cscale(ph2->data[i],1.0/sp_cabs(ph2->data[i])))),Fratio->data[i]);
  }
  /* Hab(0,0)=-cprod(DGs,Fratio.*DGs)/2+cprod(DGs,ph2.*conj(DGs))/2 +cnorm2(DGs); */
  tmp = sp_c3matrix_duplicate(Fratio);
  sp_c3matrix_mul_elements(tmp,DGs);  
  rtmp = sp_real(sp_c3matrix_froenius_prod(DGs,tmp))/-2;
  sp_c3matrix_conj(DGs);
  sp_c3matrix_free(tmp);
  tmp = sp_c3matrix_duplicate(ph2);
  sp_c3matrix_mul_elements(tmp,DGs);
  sp_c3matrix_conj(DGs);  
  rtmp += sp_real(sp_c3matrix_froenius_prod(DGs,tmp))/2;
  sp_c3matrix_free(tmp);
  rtmp += sp_real(sp_c3matrix_froenius_prod(DGs,DGs));
  sp_3matrix_set(Hab,0,0,0,rtmp);

  /* ph2=conj(ph2).*DGns; */
  sp_c3matrix_conj(ph2);
  sp_c3matrix_mul_elements(ph2,DGns);  
  
  /* Fratio=Fratio.*DGns; */
  sp_c3matrix_mul_elements(Fratio,DGns);  
  
  
  /* Hab(1,1)=-cprod(Fratio,DGns)/2+cprod(ph2,conj(DGns))/2; */
  rtmp = -sp_real(sp_c3matrix_froenius_prod(Fratio,DGns))/2;
  sp_c3matrix_conj(DGns);  
  rtmp += sp_real(sp_c3matrix_froenius_prod(ph2,DGns))/2;
  sp_c3matrix_conj(DGns);  
  sp_3matrix_set(Hab,1,1,0,rtmp);

  /* Hab(0,1)=-cprod(Fratio,DGs)/2+cprod(ph2,conj(DGs))/2; */
  rtmp = -sp_real(sp_c3matrix_froenius_prod(Fratio,DGs))/2;
  sp_c3matrix_conj(DGs);  
  rtmp += sp_real(sp_c3matrix_froenius_prod(ph2,DGs))/2;
  sp_c3matrix_conj(DGs);  
  sp_3matrix_set(Hab,0,1,0,rtmp);

  /* Hab(2,1)=Hab(1,2); */
  sp_3matrix_set(Hab,1,0,0,rtmp);
  if(Habi){
    sp_3matrix_memcpy(Habi,Hab);
    sp_3matrix_invert(Habi);
  }
  sp_c3matrix_free(Gab);
  sp_c3matrix_free(ph2);
  sp_c3matrix_free(Fratio);
  return 0;
}

int gradLtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,sp_vector * tau,sp_vector * dtau){
  Complex Gm,Gab;
  long long i;
  Complex dtau0 = sp_cinit(0,0);
  Complex dtau1 = sp_cinit(0,0);
  dtau->data[0] = 0;
  dtau->data[1] = 0;
    /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
  /* Gm=Gab./abs(Gab).*F0 */
  /*
    dtau(1)=-cprod(DGs,Gm-Gab);                      %-<DGs |[Pm -I]G>
    dtau(2)=-cprod(DGns,Gm);                         %-<DGns|Pm G>    
  */
  if(!tau){
    for(i = 0;i<sp_c3matrix_size(DGs);i++){
      Gab = sp_cadd(Gs->data[i],Gns->data[i]);
      assert(sp_cabs(Gab) != 0);
      if(F0->mask->data[i]){
	Gm = sp_cscale(Gab,sp_real(F0->image->data[i])/sp_cabs(Gab));
      }else{
	Gm = Gab;
      }
      dtau0 = sp_csub(dtau0,sp_cmul(DGs->data[i],sp_cconj(sp_csub(Gm,Gab))));
      dtau1 = sp_csub(dtau1,sp_cmul(DGns->data[i],sp_cconj(Gm)));
    }
  }else{
    for(i = 0;i<sp_c3matrix_size(DGs);i++){
      Gab = sp_cadd(sp_cadd(Gs->data[i],Gns->data[i]),sp_cadd(sp_cscale(DGs->data[i],tau->data[0]),sp_cscale(DGns->data[i],tau->data[1])));
      assert(sp_cabs(Gab) != 0);
      if(F0->mask->data[i]){
	Gm =sp_cscale(Gab,sp_real(F0->image->data[i])/sp_cabs(Gab));
      }else{
	Gm = Gab;
      }
      dtau0 = sp_csub(dtau0,sp_cmul(DGs->data[i],sp_cconj(sp_csub(Gm,Gab))));
      dtau1 = sp_csub(dtau1,sp_cmul(DGns->data[i],sp_cconj(Gm)));
    }
  }

#warning Maybe this is very wrong and dtau should be complex
  dtau->data[0] = sp_cabs(dtau0);
  dtau->data[1] = sp_cabs(dtau1);
  return 0;
}


void gradLrho(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * S,Image * F0,real * w, sp_c3matrix * DGs, sp_c3matrix * DGns){
/*
  %[DGs,DGns]=gradLrho(Gs,Gns,S,F0,w);
  %
  % calculate step (gradient with one sign reversal),
  %                      
  % inputs: 
  % Gs FT of gs, density inside support
  % Gns            "     outside  "
  % S            support
  % F0           amplitude
  % w            weigth for  ?(I-Pm)G?²-w?(I-Ps)G?², default=1
  % outputs:
  % DGs          step    inside   "       =Ps (Pm-I)
  % DGns         step    outside  "       =-Pns Pm G
*/
  /*  Gm=F0.*(Gs+Gns)./abs(Gs+Gns);  %½ Pm G */
  sp_c3matrix * Gm =sp_c3matrix_alloc(sp_c3matrix_x(Gs),sp_c3matrix_y(Gs),1);;
  sp_c3matrix * Gms;
  sp_c3matrix * tmp;
  long long i;
  real norm = 1.0/sp_c3matrix_size(Gs);
  for(i = 0;i<sp_c3matrix_size(Gs);i++){
    assert(sp_cabs(sp_cadd(Gs->data[i],Gns->data[i])) != 0);
    if(F0->mask->data[i]){
      Gm->data[i] = sp_cscale((sp_cadd(Gs->data[i],Gns->data[i])),sp_real(F0->image->data[i])/sp_cabs(sp_cadd(Gs->data[i],Gns->data[i])));
    }else{
      Gm->data[i] = sp_cadd(Gs->data[i],Gns->data[i]);
    }
  }
/*  Gm = sp_proj_module(tmp,F0);*/

  /*  Gms=fft2(ifft2(Gm).*S);        %½ Ps Pm G */
  tmp = sp_c3matrix_ifft(Gm);
  Gms = sp_c3matrix_alloc(sp_c3matrix_x(Gm),sp_c3matrix_y(Gm),0);
  for(i = 0;i<sp_c3matrix_size(Gs);i++){
    if(sp_real(S->data[i])){
      Gms->data[i] = tmp->data[i];
    }
  }
/*  Gms = sp_proj_support(tmp,S); */
  
  sp_c3matrix_free(tmp);
  tmp = Gms;
  Gms = sp_c3matrix_fft(tmp);
  sp_c3matrix_free(tmp);
  /* normalize */
  sp_c3matrix_scale(Gms,sp_cinit(norm,0));

  /* DGs=(Gms-Gs);                  %½ Ps (Pm-I)      %reversed sign */
  sp_c3matrix_memcpy(DGs,Gms);
  sp_c3matrix_sub(DGs,Gs);
  /* DGns=-(Gm-Gms);                %½ -Pns Pm G */ 
  sp_c3matrix_memcpy(DGns,Gms);
  sp_c3matrix_sub(DGns,Gm);
  if(w){
    /*DGns=DGns+(1-w)*Gns;       %½ -Pns (Pm -(1-w)I) G*/
    tmp = sp_c3matrix_duplicate(Gns);
    sp_c3matrix_scale(tmp,sp_cinit((1-*w),0));
    sp_c3matrix_add(DGns,tmp,NULL);
    sp_c3matrix_free(tmp);
  }        
  sp_c3matrix_free(Gms);
  sp_c3matrix_free(Gm);
}

int minmaxtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,real TolY,int maxiter, sp_vector * tau, sp_3matrix * Hi){
  static sp_vector * tauav = NULL;
  static sp_3matrix * Hiav = NULL;
  static int nav = 0;
  real r;
  real RDGs_cp;
  real RDGns_cp;
  real mintaultmp;
  int i,jj;
  int ii;
  real x;

  /* data will be stored as "tau0[0],tau0[1],tau1[0],tau1[1],tau2[0],tau2[1],...*/
  sp_3matrix * taui = sp_3matrix_alloc(2,maxiter+1,1);
  sp_3matrix * dtaui = sp_3matrix_alloc(2,maxiter+1,1);
  sp_vector * dtau = sp_vector_alloc(2);
  real dtaul;
  real Dtaul;
  real dtaulo;
  sp_vector * dtau0 = sp_vector_alloc(2);
  sp_vector * dtauold = sp_vector_alloc(2);
  real dtaulmin;
  sp_vector * taumin = sp_vector_alloc(2);
  sp_vector * dtaumin = sp_vector_alloc(2);
  real maxstep;
  real minmaxstep;
  real fixmaxstep;
  real erquad;
  real erquad1;
  sp_vector * Dtauold = sp_vector_alloc(2);
  sp_vector * Dtau = sp_vector_alloc(2);
  sp_3matrix * H = sp_3matrix_alloc(2,2,1);
  sp_3matrix * minus_Hi = sp_3matrix_alloc(2,2,1);
  sp_vector * tau_plus_step = sp_vector_alloc(2);

  sp_vector * y = sp_vector_alloc(2);
  sp_vector * s = sp_vector_alloc(2);
  sp_vector * Hi_y = sp_vector_alloc(2);
  sp_vector * Hi_y_minus_s = sp_vector_alloc(2);
  sp_vector * s_minus_Hi_y = sp_vector_alloc(2);

  sp_vector * tmp_v = sp_vector_alloc(2);

  sp_3matrix * DHi = sp_3matrix_alloc(2,2,1);
  real Dtaulold;
    
  
  RDGs_cp = sp_real(sp_c3matrix_froenius_prod(DGs,DGs));
  RDGns_cp = sp_real(sp_c3matrix_froenius_prod(DGns,DGns));
  /* this is the same as tauove but faster */
  real dtaul0 = sqrt(RDGs_cp*RDGs_cp+RDGns_cp*RDGns_cp); 
  
  if(!tauav){ /* initialize tau*/
    tau->data[0] = 1;
    tau->data[1] = 0.75; /* HIO values */
  }else{
    tau->data[0] = tauav->data[0];
    tau->data[1] = tauav->data[1]; /* use previous averages */
  }
  gradLtau(Gs,Gns,DGs,DGns,F0,tau,dtau); /* get gradient */
  
  sp_3matrix_set(taui,0,0,0,tau->data[0]);
  sp_3matrix_set(taui,1,0,0,tau->data[1]);
  sp_3matrix_set(dtaui,0,0,0,dtau->data[0]);
  sp_3matrix_set(dtaui,1,0,0,dtau->data[1]);


  sp_vector_memcpy(dtauold,dtau);

  sp_vector_memcpy(dtau0,dtau);

  
  dtaul=sp_vector_norm(dtau);
  dtaulo=dtaul;

  dtaulmin=dtaul;
  sp_vector_memcpy(taumin,tau);
  sp_vector_memcpy(dtaumin,dtau);

  if(nav < 4){
#ifdef DEBUG_SPO
    printf("Calculating explicit Hessian\n");
#endif
    hesLtau(Gs,Gns,DGs,DGns,F0,NULL,H,Hi);
    goright(Hi);
  }else{
    /* %let's see if we can get away with this one */
#ifdef DEBUG_SPO
    printf("Copying previous Hessian\n");
#endif
    sp_3matrix_memcpy(Hi,Hiav);
  }
  
  sp_vector_memcpy(Dtauold,tau);
  Dtaulold=sp_vector_norm(tau);
  /*start with a small step*/
  maxstep=.3;                                         
  /* smallest step limit */
  minmaxstep=.1;                                      
  /* maximum  step limit */
  fixmaxstep=1;

  for(ii = 0;ii<maxiter;ii++){
    sp_vector_memcpy(dtauold,dtau);
    dtaulo=dtaul;
    /* force it to go in the descent-ascent direction */
    sp_3matrix_set(Hi,0,0,0,fabs(sp_3matrix_get(Hi,0,0,0)));
    sp_3matrix_set(Hi,1,1,0,-fabs(sp_3matrix_get(Hi,1,1,0)));
    /* Dtau=-Hi*dtau; */
    sp_3matrix_memcpy(minus_Hi,Hi);
    sp_3matrix_scale(minus_Hi,-1);

    Dtau = sp_3matrix_vector_prod(minus_Hi,dtau); /* newton step */
    
    if(((ii+11) % 20) == 0){ /* try a fit */
      Hfit(taui,dtaui,ii+1,tau,dtau,Dtau,Hi); /* linear fit  */
#ifdef DEBUG_SPO
      printf("Trying linear fit for Hi\n");
#endif

    }
    if(((ii+1) % 20) == 0){
      /* Newton with line search, go back to minimum value */
      /* Use smallest gradient */
      mintaultmp = 0;

#ifdef DEBUG_SPO
      printf("Trying newton with line search\n");
#endif
      
      jj = 0;
      for(i = 0;i<ii;i++){
	r = sqrt(sp_3matrix_get(dtaui,0,i,0)*sp_3matrix_get(dtaui,0,i,0)+sp_3matrix_get(dtaui,1,i,0)*sp_3matrix_get(dtaui,1,i,0));
	if(r < mintaultmp || !mintaultmp){
	  mintaultmp = r;
	  jj = i;
	}
      }
      dtau->data[0] = sp_3matrix_get(dtaui,0,jj,0);
      dtau->data[1] = sp_3matrix_get(dtaui,1,jj,0);
      /* Hessian */
      hesLtau(Gs,Gns,DGs,DGns,F0,tau,H,Hi);
      /* Dtau=-Hi*dtau; */
      sp_3matrix_memcpy(minus_Hi,Hi);
      sp_3matrix_scale(minus_Hi,-1);
      Dtau = sp_3matrix_vector_prod(minus_Hi,dtau); /* newton step */
      x = linminab(Gs,Gns,DGs,DGns,F0,tau,Dtau,Hi,-1); /* step length */
      /* Dtau = Dtau*x;*/
      sp_vector_scale(Dtau,x);
      maxstep=fixmaxstep;                   /* increase limit */      	
    }

    Dtaul = sp_vector_norm(Dtau);
    x = sp_min(Dtaul,maxstep)/Dtaul;  /* limit step */
    /* dtau=gradLtau(Gs,Gns,DGs,DGns,F0,tau+Dtau*x); */
    sp_vector_memcpy(tau_plus_step,Dtau);
    sp_vector_scale(tau_plus_step,x);
    sp_vector_add(tau_plus_step,tau);    
    gradLtau(Gs,Gns,DGs,DGns,F0,tau_plus_step,dtau); /* new gradient */
    if(dtau->data[0]-dtauold->data[0] == 0 && dtau->data[1]-dtauold->data[1] == 0){
      /* we're not moving, lets get out */
      ii = maxiter;
      break;
    }
    assert(dtau->data[0]-dtauold->data[0] != 0 || dtau->data[1]-dtauold->data[1] != 0);

    /* dtaui(:,ii+1)=dtau; */
    sp_3matrix_set(dtaui,0,ii+1,dtau->data[0],0);
    sp_3matrix_set(dtaui,1,ii+1,dtau->data[1],0);
    /* dtaul=sqrt(cnorm2(dtau)); */
    dtaul =  sp_vector_norm(dtau); /* length */
    
    /* if dtaul<dtaulmin; taumin=tau;dtaulmin=dtaul;dtaumin=dtau;end */
    if(dtaul < dtaulmin){
      sp_vector_memcpy(taumin,tau);
      dtaulmin = dtaul;
      sp_vector_memcpy(dtaumin,dtau);
    }
    
    sp_vector_memcpy(tau,tau_plus_step); /* new tau */ 
    sp_3matrix_set(taui,0,ii+1,0,tau->data[0]);
    sp_3matrix_set(taui,1,ii+1,0,tau->data[1]);
    sp_vector_memcpy(Dtauold,Dtau);
    if(dtaul/dtaul0<TolY){
      break; /* good enough, get out */
    }
    
    /* y=dtau-dtauold;                                 %gradient step */
    sp_vector_memcpy(y,dtau);
    sp_vector_sub(y,dtauold);
    
    /*    s=Dtau*x; */
    sp_vector_memcpy(s,Dtau); /* step */
    sp_vector_scale(s,x);

    /* how bad is the quadratic model? */
    /* Hi*y-s */
    Hi_y = sp_3matrix_vector_prod(Hi,y);
    sp_vector_memcpy(Hi_y_minus_s,Hi_y);
    sp_vector_sub(Hi_y_minus_s,s);    
    /* erquad=sqrt(cnorm2(Hi*y-s)/(cnorm2(s)+cnorm2(Hi*y))); */
    assert((sp_vector_dot_prod(s,s)+sp_vector_dot_prod(Hi_y,Hi_y)) != 0);
    erquad = sqrt(sp_vector_dot_prod(Hi_y_minus_s,Hi_y_minus_s)/(sp_vector_dot_prod(s,s)+sp_vector_dot_prod(Hi_y,Hi_y)));
#ifdef DEBUG_SPO
    fprintf(stdout,"ii - %d   erquad - %f\n",ii,erquad); 
#endif
    if (erquad>1){ /* real hessian */
#ifdef DEBUG_SPO
      printf("Real Hessian update\n");
#endif

#ifdef DEBUG_SPO
      fprintf(stdout,"iter=%d, erquad=%lf\n",ii,erquad);
#endif
      maxstep=sp_max(maxstep/4,minmaxstep);  /* %decrease step size */
      hesLtau(Gs,Gns,DGs,DGns,F0,tau,H,Hi);
      sp_3matrix_memcpy(minus_Hi,Hi);
      sp_3matrix_scale(minus_Hi,-1);
      Dtau = sp_3matrix_vector_prod(minus_Hi,dtau); /* newton step */
      x=linminab(Gs,Gns,DGs,DGns,F0,tau,Dtau,Hi,-1); /*%optimize length*/
      sp_3matrix_scale(Hi,x);
    }else if(erquad>1e-2){ /* SR1 update */
#ifdef DEBUG_SPO
      printf("SR1 update\n");
#endif

      /* erquad1=abs(y'*(s-Hi*y))/sqrt(cnorm2(y)*cnorm2(s-Hi*y)); */
      sp_vector_memcpy(s_minus_Hi_y,Hi_y_minus_s);
      sp_vector_scale(s_minus_Hi_y,-1);
      /* this line is *extremely* fishy */
      assert(sqrt(sp_vector_dot_prod(y,y)*sp_vector_dot_prod(s_minus_Hi_y,s_minus_Hi_y)) != 0);
      erquad1 = fabs(sp_vector_dot_prod(y,s_minus_Hi_y))/sqrt(sp_vector_dot_prod(y,y)*sp_vector_dot_prod(s_minus_Hi_y,s_minus_Hi_y));
/*      if(ii > 6){
	fprintf(stdout,"erquad1 - %f\n",erquad);
      }*/
      
      maxstep = sp_min(maxstep*2,fixmaxstep);
      if(erquad1>1e-2){
	/* extrapolate hessian using SR1 method */
	/* DHi=(s-Hi*y)*(s-Hi*y)'/((s-Hi*y)'*y); */
	DHi = sp_vector_outer_prod(s_minus_Hi_y,s_minus_Hi_y);
	assert(sp_vector_dot_prod(s_minus_Hi_y,y) != 0);
	sp_3matrix_scale(DHi,1.0/sp_vector_dot_prod(s_minus_Hi_y,y));
	/* Hi=Hi+DHi; %see Nocedal & Wright, Num. Opt. Springer 99 */
	sp_3matrix_add(Hi,DHi);
	sp_3matrix_free(DHi);
      }

    }else{
      /* no update */
#ifdef DEBUG_SPO
      printf("No update\n");
#endif

      maxstep = sp_min(maxstep*3,fixmaxstep);							 
    }
    sp_vector_free(Dtau);
  }

#ifdef DEBUG_SPO
  fprintf(stdout,"iter=%d\n",ii);
#endif

  if (ii==maxiter){
    sp_vector_memcpy(tau,taumin);
    dtaul=dtaulmin;
#ifdef DEBUG_SPO
    fprintf(stdout,"max iterations exceeded, dtaul/dtau0l=%g, TolY=%g \n",dtaul/dtaul0,TolY);
#endif

    if(dtaul/dtaul0>.5){
#ifdef DEBUG_SPO
      fprintf(stdout,"back to HIO \n");
#endif
      tau->data[0] =1;
      tau->data[1] = 0.75;
    }
  }
  if(!nav){
    /* initialize tau average */
    tauav = sp_vector_alloc(2);
    sp_vector_memcpy(tauav,tau);
    Hiav = sp_3matrix_alloc(2,2,1);
    sp_3matrix_memcpy(Hiav,Hi);
    nav = 1;
  }else{
    /* tauav=(tauav*nav+tau)/(nav+1); */
    sp_vector_scale(tauav,nav);
    sp_vector_add(tauav,tau);
    sp_vector_scale(tauav,1.0/(nav+1));
    /* Hiav=(Hiav*nav+Hi)/(nav+1); */
    sp_3matrix_scale(Hiav,nav);
    sp_3matrix_add(Hiav,Hi);
    sp_3matrix_scale(Hiav,1.0/(nav+1));
    nav = sp_min(nav+1,5);        
  }
  sp_vector_free(s);
  sp_vector_free(y);
  sp_vector_free(Hi_y);
  sp_vector_free(Hi_y_minus_s);
  sp_vector_free(s_minus_Hi_y);
  sp_vector_free(tmp_v);
  sp_vector_free(tau_plus_step);
  sp_vector_free(Dtauold);
  sp_vector_free(taumin);
  sp_vector_free(dtaumin);
  return ii;
}
