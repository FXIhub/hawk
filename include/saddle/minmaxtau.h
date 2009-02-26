#ifndef _MINMAXTAU_H_
#define _MINMAXTAU_H_ 1

#include <spimage.h>

/*! Finds the saddle point min_tau(1) max_tau(2) L(G+tau(1)DGs+tau(2)DGns)
 *
 * Adapted from S. Marchesini's Matlab program
 *
 * Input: 
 * Gs,Gns,DGs, DGns, F0 are the usual:
 * Gs/Gns:   FT of gs/gns, image inside/outside support
 * DGs/DGns: FT of steps Dgs/Dgns
 * F0:       Diffraction amplitude
 * TolX:     Tolerance in X, typically .01
 * maxiter:  maximum iterations, typically 50;
 *
 * Output:
 * tau:      saddle point
 * Hi:       inverse Hessian
 * niter:    number of iterations
 *
 * Returns 0 if successful or a negative error number otherwise
 *
 */
int minmaxtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,real TolY,int maxiter, sp_vector * tau, sp_3matrix * Hi);


/*! calculate the gradient (|d/da |,|d/db |)      L(Gs+Gns+a*DGs+b+DGns)
 *    
 * inputs:
 * Gs FT of gs, density inside support
 * Gns            "     outside  "
 * DGs          step    inside   "
 * DGns         step    outside  "
 * tau          add tau[0]*DGs+tau[1]*DGns before calculating the gradient
 * outputs:
 *
 * dtau[0]=<DGs |[Ps-Pm] (Gs+Gns+a*DGs+b*DGns)>
 * dtau[1]=<DGns|[Ps-Pm] (Gs+Gns+a*DGs+b*DGns)>
 *
 */
int gradLtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,sp_vector * tau,sp_vector * dtau);

/*! Calculate the hessian of L(Gs+Gns+a*DGs+b+DGns)
 *  
 *  Calculate the hessian  |d²/da²    d²/da db|      L(Gs+Gns+a*DGs+b+DGns)
 *                         |d²/db da  d²/db²  |
 *
 *  where a=tau(1), b=tau(2)
 *  inputs: 
 *  Gs FT of gs, density inside support
 *  Gns            "     outside  "
 *  DGs          step    inside   "
 *  DGns         step    outside  "
 *  a,b          add a*DGs+b*DGns before calculating the gradient
 *
 *  outputs:
 *  Hab, Habi: hessian and its inverse
 * 
 *  Hab, and Habi must point to initialized matrices of the appropriate size
 * 
 */


int hesLtau(sp_c3matrix * Gs,sp_c3matrix * Gns,sp_c3matrix * DGs,sp_c3matrix * DGns,Image * F0,sp_vector * tau,sp_3matrix * Hab, sp_3matrix * Habi);


/*! calculate step (gradient with one sign reversal),
 *  %                      
 *  % inputs: 
 *  % Gs FT of gs, density inside support
 *  % Gns            "     outside  "
 *  % S            support
 *  % F0           amplitude
 *  % w            weigth for  ?(I-Pm)G?²-w?(I-Ps)G?², default=1
 *  % outputs:
 *  % DGs          step    inside   "       =Ps (Pm-I)
 *  % DGns         step    outside  "       =-Pns Pm G
*/
void gradLrho(sp_c3matrix *Gs,sp_c3matrix * Gns,sp_c3matrix * S,Image * F0,real * w,  sp_c3matrix * DGs, sp_c3matrix * DGns);
#endif
