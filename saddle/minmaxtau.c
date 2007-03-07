#include <stdlib.h>
#include <math.h>
#include <spimage.h>
#include <minmaxtau.h>


void rebracketxy(sp_vector * xmin,sp_vector * xmax,real x,real y,int init){
  if(init){
    xmin->data[0] = x;
    xmin->data[1] = y;
    xmin->data[0] = REAL_MAX;
    xmin->data[1] = -REAL_MAX;
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
								       


void bisect2(real x0, real x1, real y0, real y1, real dy, sp_vector * xmax, sp_vector * xmin, real *xn, real * dyn){
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
  real Dx,Dy,d2y;

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
  *dyn=Dy/Dx;

  /*   %second derivative (using 'dy' as first derivative at x0) */
  d2y=(Dy-Dx*dy)/(Dx*Dx);                /*   %second derivative */
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

sp_vector * gradL(sp_cmatrix * Gs, sp_cmatrix * Gns, sp_cmatrix * DGs, sp_cmatrix * DGns, sp_cmatrix * F0, real aa, real bb){
  sp_vector * dab = sp_vector_alloc(2);
  sp_cmatrix * Gab;
  sp_cmatrix * Gm;
  int i;
  Complex x;
  /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
  Gab = sp_cmatrix_duplicate(Gs);    
  sp_cmatrix_add(Gab,Gns,NULL);
  x = aa;
  sp_cmatrix_add(Gab,DGs,&x);
  x = bb;
  sp_cmatrix_add(Gab,DGns,&x);
  /*
    Gm=Gab./abs(Gab).*F0;
  */
  Gm = sp_cmatrix_duplicate(Gab);
  for(i = 0;i<sp_cmatrix_size(Gab);i++){
    Gm->data[i] = F0->data[i]*(Gm->data[i]/cabs(Gm->data[i]));
  }
  
  /*

  dab(1)=-cprod(DGs,Gm-Gab);                      %-<DGs |[Pm -I]G>
  dab(2)=-cprod(DGns,Gm);                     %-<DGns|Pm G>

   */
  dab->data[1] = -creal(sp_cmatrix_froebius_prod(DGns,Gm));
  sp_cmatrix_sub(Gm,Gab);
  dab->data[0] = -creal(sp_cmatrix_froebius_prod(DGs,Gm));
  sp_cmatrix_free(Gm);
  sp_cmatrix_free(Gab);
  return dab;
}

real linminab(sp_cmatrix * Gs, sp_cmatrix * Gns, sp_cmatrix * DGs, sp_cmatrix * DGns, sp_cmatrix * F0, sp_vector * ab, sp_vector * dab, sp_matrix * Habi, real TolY){
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
  real y,yold,dy;
  sp_vector * tmp;
  sp_vector * tmp2;
  sp_vector * xmin = sp_vector_alloc(2);
  sp_vector * xmax = sp_vector_alloc(2);
  sp_matrix * minus_Habi = sp_matrix_alloc(2,2);
  real dyn,xold,x;
  real Dx,Dy,xn;
  real errslp,d2y;
  int ii;
  

/*  fprintf(stdout,"linminab\n");*/

  sp_matrix_memcpy(minus_Habi,Habi);
  sp_matrix_scale(minus_Habi,-1);
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
  */

  /* start from 0 step length */
  x = 0;
  /* %function to send to 0 (but we are forcing it to go toward a min */
  /* y=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x)); */
  tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
  tmp2 = sp_matrix_vector_prod(minus_Habi,tmp);
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
    Dx=-y/dy;
    x=x+Dx;
    

    /*
      %calculate new value
      y=cprod(dab,-Habi*gradL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x));
    */
    tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
    tmp2 = sp_matrix_vector_prod(minus_Habi,tmp);
    y = sp_vector_dot_prod(dab,tmp2);
    sp_vector_free(tmp);
    sp_vector_free(tmp2);

    /*
    if abs(y)<TolY; return;end %small enough, get out!
    */
    if(fabs(y) < TolY){
      sp_vector_free(xmin);
      sp_vector_free(xmax);
      sp_matrix_free(minus_Habi);
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
      tmp2 = sp_matrix_vector_prod(minus_Habi,tmp);
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
      dyn=Dy/Dx;
      errslp=fabs((dyn-dy)/((dy+1*REAL_EPSILON)));
      errslp *= errslp;
      
      if(errslp>0.25){
	/*
	  dy=cprod(dab,-Habi*hesL(Gs,Gns,DGs,DGns,F0,ab(1)+dab(1)*x,ab(2)+dab(2)*x)*dab);
	*/
	tmp = gradL(Gs,Gns,DGs,DGns,F0,ab->data[0]+dab->data[0]*x,ab->data[1]+dab->data[1]*x);
	tmp2 = sp_matrix_vector_prod(minus_Habi,tmp);
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
	d2y=(Dy-Dx*dy)/(Dx*Dx);
	dy=dy+d2y*Dx;	    
      }
    }
    
  }  
/*  fprintf(stdout,"linminab roger and out\n");*/

  return x;
}


void Hfit(sp_matrix * _taui, sp_matrix * _dtaui, int iter, sp_vector * tau, sp_vector * dtau, sp_vector * Dtau, sp_matrix * Hi){
  /* linear fit */
  sp_matrix * dtaui = sp_matrix_alloc(2,iter);
  sp_matrix * taui = sp_matrix_alloc(2,iter);
  sp_vector * dtaum = sp_vector_alloc(2);
  sp_vector * taum = sp_vector_alloc(2);
  sp_matrix * x = sp_matrix_alloc(2,iter);
  sp_matrix * y = sp_matrix_alloc(2,iter);
  sp_matrix * x_trans = sp_matrix_alloc(2,iter);
  sp_matrix * y_trans = sp_matrix_alloc(2,iter);
  sp_matrix * H;
  sp_matrix * tmp;
  sp_matrix * tmp2;
  sp_vector * taur = sp_vector_alloc(2);
  real r;
  int i;
  int jj = 0;

  real mintaultmp;
  /*
   * dtaum=mean(dtaui,2);
   * taum=mean(taui,2);
   */

  for(i = 0;i<iter;i++){
    sp_matrix_set(dtaui,0,i,sp_matrix_get(_dtaui,0,i));
    sp_matrix_set(dtaui,1,i,sp_matrix_get(_dtaui,1,i));
    sp_matrix_set(taui,0,i,sp_matrix_get(_taui,0,i));
    sp_matrix_set(taui,1,i,sp_matrix_get(_taui,1,i));
  }
  for(i = 0;i<sp_matrix_cols(dtaui);i++){
    dtaum->data[0] += sp_matrix_get(dtaui,0,i);
    dtaum->data[1] += sp_matrix_get(dtaui,1,i);
    taum->data[0] += sp_matrix_get(taui,0,i);
    taum->data[1] += sp_matrix_get(taui,1,i);
  }
  dtaum->data[0] /= sp_matrix_cols(dtaui);
  dtaum->data[1] /= sp_matrix_cols(dtaui);
  taum->data[0] /= sp_matrix_cols(dtaui);
  taum->data[1] /= sp_matrix_cols(dtaui);
  /* x=[taui(1,:)-taum(1);taui(2,:)-taum(2)]; 
   * y=[dtaui(1,:)-dtaum(1);dtaui(2,:)-dtaum(2)];
   */
  for(i = 0;i<sp_matrix_cols(dtaui);i++){
    sp_matrix_set(x,0,i,sp_matrix_get(taui,0,i)-taum->data[0]);
    sp_matrix_set(x,1,i,sp_matrix_get(taui,1,i)-taum->data[1]);
    sp_matrix_set(y,0,i,sp_matrix_get(dtaui,0,i)-dtaum->data[0]);
    sp_matrix_set(y,1,i,sp_matrix_get(dtaui,1,i)-dtaum->data[1]);
  }
  sp_matrix_memcpy(x_trans,x);
  sp_matrix_transpose(x_trans);
  sp_matrix_memcpy(y_trans,y);
  sp_matrix_transpose(y_trans);
  
  /* %Fitted Hessian */
  /* H=(x*x')^-1*x*y'; */
  tmp = sp_matrix_mul(x,x_trans);
  sp_matrix_invert(tmp);
  tmp2 = sp_matrix_mul(x,y_trans);
  H = sp_matrix_mul(tmp,tmp2);
  sp_matrix_free(tmp);
  sp_matrix_free(tmp2);
  
  /* Hi=H^-1; */
  sp_matrix_memcpy(Hi,H);
  sp_matrix_invert(Hi);

  /* fitted root */
  tmp = sp_matrix_mul(Hi,dtaui);
  for(i = 0;i<sp_matrix_cols(dtaui);i++){
    taur->data[0] += -(sp_matrix_get(tmp,0,i)-sp_matrix_get(taui,0,i));
    taur->data[1] += -(sp_matrix_get(tmp,1,i)-sp_matrix_get(taui,1,i));
  }
  taur->data[0] /= dtaui->cols; 
  taur->data[1] /= dtaui->cols; 
  
  /* Newton with line search, go back to minimum value */
  /* Use smallest gradient */
  mintaultmp = 0;
  for(i = 0;i<sp_matrix_cols(dtaui);i++){
    r = sqrt(sp_matrix_get(dtaui,0,i)*sp_matrix_get(dtaui,0,i)+sp_matrix_get(dtaui,1,i)*sp_matrix_get(dtaui,1,i));
    if(r < mintaultmp || !mintaultmp){
      mintaultmp = r;
      jj = i;
    }
  }
  /* tau=taui(:,jj);  */
  /*go back to that point  */
  tau->data[0] = sp_matrix_get(taui,0,jj);
  tau->data[1] = sp_matrix_get(taui,1,jj);
  /* %use its gradient */
  /*  dtau=dtaui(:,jj); */
  dtau->data[0] = sp_matrix_get(dtaui,0,jj); 
  dtau->data[1] = sp_matrix_get(dtaui,1,jj);
  /* step to root */
  /* Dtau=tau-taur; */
  Dtau->data[0] = tau->data[0]-taur->data[0];
  Dtau->data[1] = tau->data[1]-taur->data[1];      
  sp_matrix_free(dtaui);
  sp_matrix_free(taui);
}

void goright(sp_matrix * H){
  int ND = H->rows;
  sp_vector *  rightdir = sp_vector_alloc(ND);
  sp_vector *  isright = sp_vector_alloc(ND);
  real Hmin;
  sp_matrix * redM = sp_matrix_alloc(ND,ND);
  sp_matrix * tmp;
  sp_matrix * tmp2;
  int i;
  /* rightdir=repmat([1,-1],1,ND/2); %alternate min-max */
  for(i = 0;i<ND/2;i++){
    rightdir->data[2*i] = 1;
    rightdir->data[2*i+1] = -1;
  }

  /* modify diagonal elements so that the sign is right */
  /* H(sub2ind(size(H),1:ND,1:ND))=rightdir.*abs(H(sub2ind(size(H),1:ND,1:ND))); */
  for(i = 0;i<ND;i++){
    sp_matrix_set(H,i,i,fabs(sp_matrix_get(H,i,i))*rightdir->data[i]);
  }
  /* %reduce wrong elements to 1/2 of smallest right element */
  /* diagH=diag(H) */
  /* isright=diagH'.*rightdir>0 */

  /* I have the slight feeling this isright is always gonna be filled with 1s */

  /* Hmin=min(abs(H(sub2ind(size(H),find(isright),find(isright))))); */
  Hmin = 0;
  for(i = 0;i<ND;i++){
    if(sp_matrix_get(H,i,i)*rightdir->data[i] > 0){
      sp_vector_set(isright,i,1);
      if(fabs(sp_matrix_get(H,i,i)) < Hmin || !Hmin){ 
	Hmin = fabs(sp_matrix_get(H,i,i));
      }
    }else{
      sp_vector_set(isright,i,0);
    }
  }
  /* redM=diag(sqrt(Hmin/1)./sqrt(abs(diag(H))).*(~isright'))+diag(isright); */
  for(i = 0;i<ND;i++){
    if(isright->data[i]){
      sp_matrix_set(redM,i,i,1);
    }else{
      sp_matrix_set(redM,i,i,sqrt(Hmin)/sqrt(fabs(sp_matrix_get(H,i,i))));
    }
  }
  /* H=redM*H*redM; */
  tmp = sp_matrix_mul(redM,H);
  tmp2 = sp_matrix_mul(tmp,redM);
  sp_matrix_memcpy(H,tmp2);
  sp_matrix_free(tmp);
  sp_matrix_free(tmp2);
  
  if(ND==4){
    /* make sure eigenvalues of alpha1,alpha2 hessian and beta1,beta2 hessian
     * H([1,3],[1,3]),H([2,4],[2,4]) are positive
     */
    if(sp_matrix_get(H,2,4)*sp_matrix_get(H,2,4) > sp_matrix_get(H,2,2)*sp_matrix_get(H,4,4)){
      /* remove diag components if too big */
      sp_matrix_set(H,2,4,0);
      sp_matrix_set(H,4,2,0);
    }
    if(sp_matrix_get(H,1,3)*sp_matrix_get(H,1,3) > sp_matrix_get(H,1,1)*sp_matrix_get(H,3,3)){
      /* remove diag components if too big */
      sp_matrix_set(H,1,3,0);
      sp_matrix_set(H,3,1,0);
    }
  }
}

int hesLtau(sp_cmatrix * Gs,sp_cmatrix * Gns,sp_cmatrix * DGs,sp_cmatrix * DGns,sp_cmatrix * F0,sp_vector * tau,sp_matrix * Hab, sp_matrix * Habi){
  sp_cmatrix * Gab;
  sp_cmatrix * Fratio;
  sp_cmatrix * tmp;
  sp_cmatrix * ph2;
  real rtmp;
  Complex x;
  int i;
  if(!tau){
    Gab = sp_cmatrix_duplicate(Gs);
    sp_cmatrix_add(Gab,Gns,NULL);
  }else{
    /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
    Gab = sp_cmatrix_duplicate(Gs);    
    sp_cmatrix_add(Gab,Gns,NULL);    
    x = tau->data[0];
    sp_cmatrix_add(Gab,DGs,&x);
    x = tau->data[1];
    sp_cmatrix_add(Gab,DGns,&x);
  }
/*  fprintf(stdout,"HesLtau\n"); */

/* Fratio = F0/abs(Gab);  */
  Fratio = sp_cmatrix_duplicate(F0);
  for(i = 0;i<sp_cmatrix_size(Gab);i++){
    Fratio->data[i] /= cabsr(Gab->data[i]);
  }
/* ph2 = (Gab/abs(Gab))^2 * Fratio */
  ph2 = sp_cmatrix_duplicate(Gab);
  for(i = 0;i<sp_cmatrix_size(ph2);i++){
    ph2->data[i] = (ph2->data[i]/cabsr(ph2->data[i]))*(ph2->data[i]/cabsr(ph2->data[i]))*Fratio->data[i];
  }
  /* Hab(0,0)=-cprod(DGs,Fratio.*DGs)/2+cprod(DGs,ph2.*conj(DGs))/2 +cnorm2(DGs); */
  tmp = sp_cmatrix_duplicate(Fratio);
  sp_cmatrix_mul_elements(tmp,DGs);  
  rtmp = creal(sp_cmatrix_froebius_prod(DGs,tmp))/-2;
  sp_cmatrix_conj(DGs);
  sp_cmatrix_free(tmp);
  tmp = sp_cmatrix_duplicate(ph2);
  sp_cmatrix_mul_elements(tmp,DGs);
  sp_cmatrix_conj(DGs);  
  rtmp += creal(sp_cmatrix_froebius_prod(DGs,tmp))/2;
  sp_cmatrix_free(tmp);
  rtmp += creal(sp_cmatrix_froebius_prod(DGs,DGs));
  sp_matrix_set(Hab,0,0,rtmp);

  /* ph2=conj(ph2).*DGns; */
  sp_cmatrix_conj(ph2);
  sp_cmatrix_mul_elements(ph2,DGns);  
  
  /* Fratio=Fratio.*DGns; */
  sp_cmatrix_mul_elements(Fratio,DGns);  
  
  
  /* Hab(1,1)=-cprod(Fratio,DGns)/2+cprod(ph2,conj(DGns))/2; */
  rtmp = -creal(sp_cmatrix_froebius_prod(Fratio,DGns))/2;
  sp_cmatrix_conj(DGns);  
  rtmp += creal(sp_cmatrix_froebius_prod(ph2,DGns))/2;
  sp_cmatrix_conj(DGns);  
  sp_matrix_set(Hab,1,1,rtmp);

  /* Hab(0,1)=-cprod(Fratio,DGs)/2+cprod(ph2,conj(DGs))/2; */
  rtmp = -creal(sp_cmatrix_froebius_prod(Fratio,DGs))/2;
  sp_cmatrix_conj(DGs);  
  rtmp += creal(sp_cmatrix_froebius_prod(ph2,DGs))/2;
  sp_cmatrix_conj(DGs);  
  sp_matrix_set(Hab,0,1,rtmp);

  /* Hab(2,1)=Hab(1,2); */
  sp_matrix_set(Hab,1,0,rtmp);
  if(Habi){
    sp_matrix_memcpy(Habi,Hab);
    sp_matrix_invert(Habi);
  }
  sp_cmatrix_free(Gab);
  sp_cmatrix_free(ph2);
  sp_cmatrix_free(Fratio);
  return 0;
}

int gradLtau(sp_cmatrix * Gs,sp_cmatrix * Gns,sp_cmatrix * DGs,sp_cmatrix * DGns,sp_cmatrix * F0,sp_vector * tau,sp_vector * dtau){

  Complex Gm,Gab;
  int i;
  dtau->data[0] = 0;
  dtau->data[1] = 0;
    /* Gab = Gs+Gns+tau(0)*DGs+tau(1)*DGns */
  /* Gm=Gab./abs(Gab).*F0 */
  /*
    dtau(1)=-cprod(DGs,Gm-Gab);                      %-<DGs |[Pm -I]G>
    dtau(2)=-cprod(DGns,Gm);                         %-<DGns|Pm G>    
  */
  if(!tau){
    for(i = 0;i<sp_cmatrix_size(DGs);i++){
      Gab = Gs->data[i]+Gns->data[i];
      Gm =F0->data[i]*Gab/cabsr(Gab);
      dtau->data[0] -= DGs->data[i]*conjr(Gm-Gab);
      dtau->data[1] -= DGns->data[i]*conjr(Gm);
    }
  }else{
    for(i = 0;i<sp_cmatrix_size(DGs);i++){
      Gab = Gs->data[i]+Gns->data[i]+tau->data[0]*DGs->data[i]+tau->data[1]*DGns->data[i];
      Gm =F0->data[i]*Gab/cabsr(Gab);
      dtau->data[0] -= DGs->data[i]*conjr(Gm-Gab);
      dtau->data[1] -= DGns->data[i]*conjr(Gm);
    }
  }
  return 0;
}


void gradLrho(sp_cmatrix * Gs,sp_cmatrix * Gns,sp_cmatrix * S,sp_cmatrix * F0,real * w, sp_cmatrix * DGs, sp_cmatrix * DGns){
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
  sp_cmatrix * Gm =sp_cmatrix_alloc(sp_cmatrix_rows(Gs),sp_cmatrix_cols(Gs));;
  sp_cmatrix * Gms;
  sp_cmatrix * tmp;
  int i;
  real norm = 1.0/sp_cmatrix_size(Gs);
  for(i = 0;i<sp_cmatrix_size(Gs);i++){
    Gm->data[i] = (Gs->data[i]+Gns->data[i])/cabs(Gs->data[i]+Gns->data[i])*F0->data[i];
  }
/*  Gm = sp_proj_module(tmp,F0);*/

  /*  Gms=fft2(ifft2(Gm).*S);        %½ Ps Pm G */
  tmp = sp_cmatrix_ifft(Gm);
  Gms = sp_cmatrix_alloc(sp_cmatrix_rows(Gm),sp_cmatrix_cols(Gm));
  for(i = 0;i<sp_cmatrix_size(Gs);i++){
    if(S->data[i]){
      Gms->data[i] = tmp->data[i];
    }
  }
/*  Gms = sp_proj_support(tmp,S); */
  
  sp_cmatrix_free(tmp);
  tmp = Gms;
  Gms = sp_cmatrix_fft(tmp);
  sp_cmatrix_free(tmp);
  /* normalize */
  sp_cmatrix_scale(Gms,norm);

  /* DGs=(Gms-Gs);                  %½ Ps (Pm-I)      %reversed sign */
  sp_cmatrix_memcpy(DGs,Gms);
  sp_cmatrix_sub(DGs,Gs);
  /* DGns=-(Gm-Gms);                %½ -Pns Pm G */ 
  sp_cmatrix_memcpy(DGns,Gms);
  sp_cmatrix_sub(DGns,Gm);
  if(w){
    /*DGns=DGns+(1-w)*Gns;       %½ -Pns (Pm -(1-w)I) G*/
    tmp = sp_cmatrix_duplicate(Gns);
    sp_cmatrix_scale(tmp,1-*w);
    sp_cmatrix_add(DGns,tmp,NULL);
    sp_cmatrix_free(tmp);
  }        
}

int minmaxtau(sp_cmatrix * Gs,sp_cmatrix * Gns,sp_cmatrix * DGs,sp_cmatrix * DGns,sp_cmatrix * F0,real TolY,int maxiter, sp_vector * tau, sp_matrix * Hi){
  static sp_vector * tauav = NULL;
  static sp_matrix * Hiav = NULL;
  static int nav = 0;
  real r;
  real RDGs_cp;
  real RDGns_cp;
  real mintaultmp;
  int i,jj;
  int ii;
  real x;

  /* data will be stored as "tau0[0],tau0[1],tau1[0],tau1[1],tau2[0],tau2[1],...*/
  sp_matrix * taui = sp_matrix_alloc(2,maxiter+1);
  sp_matrix * dtaui = sp_matrix_alloc(2,maxiter+1);
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
  sp_matrix * H = sp_matrix_alloc(2,2);
  sp_matrix * minus_Hi = sp_matrix_alloc(2,2);
  sp_vector * tau_plus_step = sp_vector_alloc(2);

  sp_vector * y = sp_vector_alloc(2);
  sp_vector * s = sp_vector_alloc(2);
  sp_vector * Hi_y = sp_vector_alloc(2);
  sp_vector * Hi_y_minus_s = sp_vector_alloc(2);
  sp_vector * s_minus_Hi_y = sp_vector_alloc(2);

  sp_vector * tmp_v = sp_vector_alloc(2);

  sp_matrix * DHi = sp_matrix_alloc(2,2);
  real Dtaulold;
    
  
  RDGs_cp = creal(sp_cmatrix_froebius_prod(DGs,DGs));
  RDGns_cp = creal(sp_cmatrix_froebius_prod(DGns,DGns));
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
  
  sp_matrix_set(taui,0,0,tau->data[0]);
  sp_matrix_set(taui,1,0,tau->data[1]);
  sp_matrix_set(dtaui,0,0,dtau->data[0]);
  sp_matrix_set(dtaui,1,0,dtau->data[1]);


  sp_vector_memcpy(dtauold,dtau);

  sp_vector_memcpy(dtau0,dtau);

  
  dtaul=sp_vector_norm(dtau);
  dtaulo=dtaul;

  dtaulmin=dtaul;
  sp_vector_memcpy(taumin,tau);
  sp_vector_memcpy(dtaumin,dtau);

  if(nav < 4){
    hesLtau(Gs,Gns,DGs,DGns,F0,NULL,H,Hi);
    goright(Hi);
  }else{
    /* %let's see if we can get away with this one */
    sp_matrix_memcpy(Hi,Hiav);
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
    sp_matrix_set(Hi,0,0,fabs(sp_matrix_get(Hi,0,0)));
    sp_matrix_set(Hi,1,1,-fabs(sp_matrix_get(Hi,1,1)));
    /* Dtau=-Hi*dtau; */
    sp_matrix_memcpy(minus_Hi,Hi);
    sp_matrix_scale(minus_Hi,-1);

    Dtau = sp_matrix_vector_prod(minus_Hi,dtau); /* newton step */
    
    if(((ii+11) % 20) == 0){ /* try a fit */
      Hfit(taui,dtaui,ii+1,tau,dtau,Dtau,Hi); /* linear fit  */
    }
    if(((ii+1) % 20) == 0){
      /* Newton with line search, go back to minimum value */
      /* Use smallest gradient */
      mintaultmp = 0;
      for(i = 0;i<ii;i++){
	r = sqrt(sp_matrix_get(dtaui,0,i)*sp_matrix_get(dtaui,0,i)+sp_matrix_get(dtaui,1,i)*sp_matrix_get(dtaui,1,i));
	if(r < mintaultmp || !mintaultmp){
	  mintaultmp = r;
	  jj = i;
	}
      }
      dtau->data[0] = sp_matrix_get(dtaui,0,jj);
      dtau->data[1] = sp_matrix_get(dtaui,1,jj);
      /* Hessian */
      hesLtau(Gs,Gns,DGs,DGns,F0,tau,H,Hi);
      /* Dtau=-Hi*dtau; */
      sp_matrix_memcpy(minus_Hi,Hi);
      sp_matrix_scale(minus_Hi,-1);
      Dtau = sp_matrix_vector_prod(minus_Hi,dtau); /* newton step */
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
    /* dtaui(:,ii+1)=dtau; */
    sp_matrix_set(dtaui,0,ii+1,dtau->data[0]);
    sp_matrix_set(dtaui,1,ii+1,dtau->data[1]);
    /* dtaul=sqrt(cnorm2(dtau)); */
    dtaul =  sp_vector_norm(dtau); /* length */
    
    /* if dtaul<dtaulmin; taumin=tau;dtaulmin=dtaul;dtaumin=dtau;end */
    if(dtaul < dtaulmin){
      sp_vector_memcpy(taumin,tau);
      dtaulmin = dtaul;
      sp_vector_memcpy(dtaumin,dtau);
    }
    
    sp_vector_memcpy(tau,tau_plus_step); /* new tau */ 
    sp_matrix_set(taui,0,ii+1,tau->data[0]);
    sp_matrix_set(taui,1,ii+1,tau->data[1]);
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
    Hi_y = sp_matrix_vector_prod(Hi,y);
    sp_vector_memcpy(Hi_y_minus_s,Hi_y);
    sp_vector_sub(Hi_y_minus_s,s);    
    /* erquad=sqrt(cnorm2(Hi*y-s)/(cnorm2(s)+cnorm2(Hi*y))); */
    erquad = sqrt(sp_vector_dot_prod(Hi_y_minus_s,Hi_y_minus_s)/(sp_vector_dot_prod(s,s)+sp_vector_dot_prod(Hi_y,Hi_y)));
/*    fprintf(stdout,"ii - %d   erquad - %f\n",ii,erquad); */
    if (erquad>1){ /* real hessian */
      fprintf(stdout,"iter=%d, erquad=%lf\n",ii,erquad);
      maxstep=sp_max(maxstep/4,minmaxstep);  /* %decrease step size */
      hesLtau(Gs,Gns,DGs,DGns,F0,tau,H,Hi);
      sp_matrix_memcpy(minus_Hi,Hi);
      sp_matrix_scale(minus_Hi,-1);
      Dtau = sp_matrix_vector_prod(minus_Hi,dtau); /* newton step */
      x=linminab(Gs,Gns,DGs,DGns,F0,tau,Dtau,Hi,-1); /*%optimize length*/
      sp_matrix_scale(Hi,x);
    }else if(erquad>1e-2){ /* SR1 update */
      /* erquad1=abs(y'*(s-Hi*y))/sqrt(cnorm2(y)*cnorm2(s-Hi*y)); */
      sp_vector_memcpy(s_minus_Hi_y,Hi_y_minus_s);
      sp_vector_scale(s_minus_Hi_y,-1);
      /* this line is *extremely* fishy */
      erquad1 = fabs(sp_vector_dot_prod(y,s_minus_Hi_y))/sqrt(sp_vector_dot_prod(y,y)*sp_vector_dot_prod(s_minus_Hi_y,s_minus_Hi_y));
/*      if(ii > 6){
	fprintf(stdout,"erquad1 - %f\n",erquad);
      }*/
      
      maxstep = sp_min(maxstep*2,fixmaxstep);
      if(erquad1>1e-2){
	/* extrapolate hessian using SR1 method */
	/* DHi=(s-Hi*y)*(s-Hi*y)'/((s-Hi*y)'*y); */
	DHi = sp_vector_outer_prod(s_minus_Hi_y,s_minus_Hi_y);
	sp_matrix_scale(DHi,1.0/sp_vector_dot_prod(s_minus_Hi_y,y));
	/* Hi=Hi+DHi; %see Nocedal & Wright, Num. Opt. Springer 99 */
	sp_matrix_add(Hi,DHi);
	sp_matrix_free(DHi);
      }

    }else{
      /* no update */
      maxstep = sp_min(maxstep*3,fixmaxstep);							 
    }
      
  }

  fprintf(stdout,"iter=%d\n",ii);

  if (ii==maxiter){
    sp_vector_memcpy(tau,taumin);
    dtaul=dtaulmin;
    fprintf(stdout,"max iterations exceeded, dtaul/dtau0l=%g, TolY=%g \n",dtaul/dtaul0,TolY);

    if(dtaul/dtaul0>.5){
      fprintf(stdout,"back to HIO \n");
      tau->data[0] =1;
      tau->data[1] = 0.75;
    }
  }
  if(!nav){
    /* initialize tau average */
    tauav = sp_vector_alloc(2);
    sp_vector_memcpy(tauav,tau);
    Hiav = sp_matrix_alloc(2,2);
    sp_matrix_memcpy(Hiav,Hi);
    nav = 1;
  }else{
    /* tauav=(tauav*nav+tau)/(nav+1); */
    sp_vector_scale(tauav,nav);
    sp_vector_add(tauav,tau);
    sp_vector_scale(tauav,1.0/(nav+1));
    /* Hiav=(Hiav*nav+Hi)/(nav+1); */
    sp_matrix_scale(Hiav,nav);
    sp_matrix_add(Hiav,Hi);
    sp_matrix_scale(Hiav,1.0/(nav+1));
    nav = sp_min(nav+1,5);        
  }
  sp_vector_free(s);
  sp_vector_free(y);
  sp_vector_free(Hi_y);
  sp_vector_free(Hi_y_minus_s);
  sp_vector_free(s_minus_Hi_y);
  sp_vector_free(tmp_v);
  sp_vector_free(tau_plus_step);
  sp_vector_free(Dtau);
  sp_vector_free(Dtauold);
  sp_vector_free(taumin);
  sp_vector_free(dtaumin);
  return ii;
}
