/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1997-1998   Saikat DebRoy and the
 *                            R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ../appl/uncmin.f
   -- translated by f2c (version of 1 June 1993 23:00:00).
   -- and hand edited by Saikat DebRoy
   */

#include <math.h>
#include "S.h"
#include "Applic.h"
#include "PrtUtil.h" /* Rprintf and printRealVector */

#ifndef max
#define max(a, b) ((a) < (b) ? (b) : (a))
#endif
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

extern double d1mach(int);
extern int i1mach(int);

extern double F77_NAME(ddot)(int *, double *, int *, double *, 
			       int *);
extern double F77_NAME(dnrm2)(int *, double *, int *);
extern int F77_NAME(dtrsl)(double *, int *, int *, double *, int *,
			     int *);
extern int F77_NAME(dscal)(int *, double *, double *, int *);

/* CC    subroutines  mvmlt[lsu] should be REPLACED by BLAS ones! */
/* CC */
/* CC--- choldc(nr,n,a,diagmx,tol,addmax)	 is ``choleski + tolerance'' */
/* CC    ------ */
/* CC    it should make use of BLAS routines as [linkpack's dpofa!] */

/*     subroutine fdhess */

/* 	this subroutine calculates a numerical approximation to the upper */
/* 	triangular portion of the second derivative matrix (the hessian). */
/* 	algorithm a5.6.2 from dennis and schnable (1983), numerical methods */
/* 	for unconstrained optimization and nonlinear equations, */
/* 	prentice-hall, 321-322. */

/* 	programmed by richard h. jones, january 11, 1989 */

/* 	input to subroutine */

/* 	   n......the number of parameters */
/* 	   x......vector of parameter values */
/* 	   fval...double precision value of function at x */
/* 	   fun....a function provided by the user which must be */
/*	   declared as external in the calling program.  its call must */
/*         be of the call fun(n,x,state,fval) where fval is the */
/*	   computed value of the function */
/*         state..information other than x and n that fun requires. */
/*	   state is not modified in fdhess (but can be modified by fun). */
/* 	   nfd... first dimension of h in the calling program */

/* 	output from subroutine */

/* 	    h.....an n by n matrix of the approximate hessian */

/* 	work space */

/* 	    step....a real array of length n */
/* 	    f.......a double precision array of length n */

/* Subroutine */ 
void
fdhess(int n, double *x, double fval, fcn_p fun, void
       *state, double *h, int nfd, double *step, double *f,
       int ndigit, double *typx)
{
  int i, j;
  double tempi, tempj, fii, eta, fij;

  eta = pow(10.0, -ndigit/3.0);
  for (i = 0; i < n; ++i) {
    step[i] = eta * max(x[i], typx[i]);
    if (typx[i] < 0.) {
      step[i] = -step[i];
    }
    tempi = x[i];
    x[i] += step[i];
    step[i] = x[i] - tempi;
    (*fun)(n, x, &f[i], state);
    x[i] = tempi;
  }
  for (i = 0; i < n; ++i) {
    tempi = x[i];
    x[i] += step[i] * 2.;
    (*fun)(n, x, &fii, state);
    h[i + i * nfd] = (fval - f[i] + (fii - f[i]))/(step[i] * step[i]);
    x[i] = tempi + step[i];
    for (j = i + 1; j < n; ++j) {
      tempj = x[j];
      x[j] += step[j];
      (*fun)(n, x, &fij, state);
      h[i + j * nfd] = (fval - f[i] + (fij - f[j]))/(step[i] * step[j]);
      x[j] = tempj;
    }
    x[i] = tempi;
  }
} /* fdhess */

/* 	subroutine d1fcn */

/* 	purpose */

/* 	dummy routine to prevent unsatisfied external diagnostic */
/* 	when specific analytic gradient function not supplied. */


static void
d1fcn(int n, double *x, double *g, void *state)
{

} /* d1fcn */

/* 	subroutine d2fcn */

/* 	purpose */

/* 	dummy routine to prevent unsatisfied external diagnostic */
/* 	when specific analytic hessian function not supplied. */

static void
d2fcn(int nr, int n, double *x, double *h, void *state)
{

} /* d2fcn */

/* 	subroutine mvmltl */

/* 	purpose */

/* 	compute y=lx */
/* 	where l is a lower triangular matrix stored in a */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	     --> lower triangular (n*n) matrix */
/* 	x(n)	     --> operand vector */
/* 	y(n)	    <--	 result vector */

/* 	note */

/* 	x and y cannot share storage */

static void
mvmltl(int nr, int n, double *a, double *x, double *y)
{
  int i, j;
  double sum;

  for (i = 0; i < n; ++i) {
    sum = 0.;
    for (j = 0; j <= i; ++j) {
      sum += a[i + j * nr] * x[j];
    }
    y[i] = sum;
  }
} /* mvmltl */

/* 	subroutine mvmltu */

/* 	purpose */

/* 	compute y=(l+)x */
/* 	where l is a lower triangular matrix stored in a */
/* 	(l-transpose (l+) is taken implicitly) */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(nr,1)	      --> lower triangular (n*n) matrix */
/* 	x(n)	     --> operand vector */
/* 	y(n)	    <--	 result vector */

/* 	note */

/* 	x and y cannot share storage */

static void
mvmltu(int nr, int n, double *a, double *x, double *y)
{
  int i, length, one = 1;

  for (i = 0, length = n; i < n; --length, ++i) {
    y[i] = F77_CALL(ddot)(&length, &a[i + i * nr], &one, &x[i], &one);
  }
} /* mvmltu */

/* 	subroutine mvmlts */

/* 	purpose */

/* 	compute y=ax */
/* 	where "a" is a symmetric (n*n) matrix stored in its lower */
/* 	triangular part and x,y are n-vectors */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	     --> symmetric (n*n) matrix stored in */
/* 			 lower triangular part and diagonal */
/* 	x(n)	     --> operand vector */
/* 	y(n)	    <--	 result vector */

/* 	note */

/* 	x and y cannot share storage. */

static void
mvmlts(int nr, int n, double *a, double *x, double *y)
{
  int i, j;
  double sum;

  for (i = 0; i < n; ++i) {
    sum = 0.;
    for (j = 0; j <= i; ++j) {
      sum += a[i + j * nr] * x[j];
    }
    for (j = i+1; j < n; ++j) {
      sum += a[j + i * nr] * x[j];
    }
    y[i] = sum;
  }
} /* mvmlts */

/* 	subroutine lltslv */

/* 	purpose */

/* 	solve ax=b where a has the form l(l-transpose) */
/* 	but only the lower triangular part, l, is stored. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	     --> matrix of form l(l-transpose). */
/* 			 on return a is unchanged. */
/* 	x(n)	    <--	 solution vector */
/* 	b(n)	     --> right-hand side vector */

/* 	note */

/* 	if b is not required by calling program, then */
/* 	b and x may share the same storage. */

static void
lltslv(int nr, int n, double *a, double *x, double *b)
{
  int job = 0, info;

  if( x != b) Memcpy(x, b, n);
  F77_CALL(dtrsl)(a, &nr, &n, x, &job, &info);
  job = 10;
  F77_CALL(dtrsl)(a, &nr, &n, x, &job, &info);
} /* lltslv */

/* 	subroutine choldc */

/* 	purpose */

/* 	find the perturbed l(l-transpose) [written ll+] decomposition */
/* 	of a+d, where d is a non-negative diagonal matrix added to a if */
/* 	necessary to allow the cholesky decomposition to continue. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	    <--> on entry: matrix for which to find perturbed */
/* 			      cholesky decomposition */
/* 			 on exit:  contains l of ll+ decomposition */
/* 			 in lower triangular part and diagonal of "a" */
/* 	diagmx	     --> maximum diagonal element of "a" */
/* 	tol	     --> tolerance */
/* 	addmax	    <--	 maximum amount implicitly added to diagonal of "a" */
/* 			 in forming the cholesky decomposition of a+d */
/* 	internal variables */

/* 	aminl	 smallest element allowed on diagonal of l */
/* 	amnlsq	 =aminl**2 */
/* 	offmax	 maximum off-diagonal element in column of a */


/* 	description */

/* 	the normal cholesky decomposition is performed.	 however, if at any */
/* 	point the algorithm would attempt to set l(i,i)=sqrt(temp) */
/* 	with temp < tol*diagmx, then l(i,i) is set to sqrt(tol*diagmx) */
/* 	instead.  this is equivalent to adding tol*diagmx-temp to a(i,i) */

static void
choldc(int nr, int n, double *a, double diagmx, double tol, double *addmax)
{
  double tmp1, tmp2;
  int i, j, k;
  double aminl, offmax, amnlsq;
  double sum;

  *addmax = 0.0;
  aminl = sqrt(diagmx * tol);
  amnlsq = aminl * aminl;

  /* 	form row i of l */

  for (i = 0; i < n; ++i) {

    /* 	find diagonal elements of l */

    sum = 0.;
    for (k = 0; k < i; ++k) {
      sum += a[i + k * nr] * a[i + k * nr];
    }
    tmp1 = a[i + i * nr] - sum;
    if (tmp1 >= amnlsq) {
      a[i + i * nr] = sqrt(tmp1);
    } else {
      /* 	find maximum off-diagonal element in row */
      offmax = 0.;
      for (j = 0; j < i; ++j) {
	if ((tmp2 = fabs(a[i + j * nr])) > offmax) {
	  offmax = tmp2;
	}
      }
      if (offmax <= amnlsq) {
	offmax = amnlsq;
      }

      /* add to diagonal element to allow cholesky decomposition to continue */

      a[i + i * nr] = sqrt(offmax);
      if((tmp2 = offmax - tmp1) > *addmax)
	*addmax = tmp2;
    }
    /* 	find i,j element of lower triangular matrix */
    for (j = 0; j < i; ++j) {
      sum = 0.;
      for (k = 0; k < j; ++k) {
	sum += a[i + k * nr] * a[j + k * nr];
      }
      a[i + j * nr] = (a[i + j * nr] - sum) / a[j + j * nr];
    }
  }
} /* choldc */

/* 	subroutine qraux1 */

/* 	purpose */

/* 	interchange rows i,i+1 of the upper hessenberg matrix r, */
/* 	columns i to n */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of matrix */
/* 	r[n*n]	    <--> upper hessenberg matrix */
/* 	i	     --> index of row to interchange (i < n-1) */

static void
qraux1(int nr, int n, double *r, int i)
{
  double tmp;
  double *r1, *r2;
  r1 = r + i + i * nr;
  r2 = r1 + 1;

  while(n-- > i) {
    tmp = *r1;
    *r1 = *r2;
    *r2 = tmp;
    r1 += nr;
    r2 += nr;
  }
} /* qraux1 */

/* 	subroutine qraux2 */

/* 	purpose */

/* 	pre-multiply r by the jacobi rotation j(i,i+1,a,b) */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of matrix */
/* 	r(n,n)	    <--> upper hessenberg matrix */
/* 	i	     --> index of row */
/* 	a	     --> scalar */
/* 	b	     --> scalar */

static void
qraux2(int nr, int n, double *r, int i, double a, double b)
{
  double c, s;
  double y, z, den;
  double *r1, *r2;

  den = sqrt(a * a + b * b);
  c = a / den;
  s = b / den;

  r1 = r + i + i*nr;
  r2 = r1 + 1;

  while(n-- > i) {
    y = *r1;
    z = *r2;
    *r1 = c * y - s * z;
    *r2 = s * y + c * z;
    r1 += nr;
    r2 += nr;
  }
} /* qraux2_ */

/* 	subroutine qrupdt */

/* 	purpose */

/* 	find an orthogonal (n*n) matrix (q*) and an upper triangular (n*n) */
/* 	matrix (r*) such that (q*)(r*)=r+u(v+) */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	    <--> on input:  contains r */
/* 			 on output: contains (r*) */
/* 	u(n)	     --> vector */
/* 	v(n)	     --> vector */

static void
qrupdt(int nr, int n, double *a, double *u, double *v)
{
  int i, j, k;
  double t1, t2;
  int ii;

  /* 	determine last non-zero in u(.) */

  for(k = n-1; k > 0 && u[k] == 0.0; k--)
    ;

  /* 	(k-1) jacobi rotations transform */
  /* 	    r + u(v+) --> (r*) + (u(1)*e1)(v+) */
  /* 	which is upper hessenberg */

  if (k > 0) {
    ii = k;
    while(ii > 0) {
      i = ii - 1;
      if (u[i] == 0.0) {
	qraux1(nr, n, a, i);
	u[i] = u[ii];
      } else {
	qraux2(nr, n, a, i, u[i], -u[ii]);
	u[i] = sqrt(u[i] * u[i] + u[ii] * u[ii]);
      }
      ii = i;
    }
  }

  /* 	r <-- r + (u(1)*e1)(v+) */

  for (j = 0; j < n; ++j) {
    a[j * nr] += u[0] * v[j];
  }

  /* 	(k-1) jacobi rotations transform upper hessenberg r */
  /* 	to upper triangular (r*) */

  for (i = 0; i < k; ++i) {
    if (a[i + i * nr] == 0.) {
      qraux1(nr, n, a, i);
    } else {
      t1 = a[i + i * nr];
      t2 = -a[i + 1 + i * nr];
      qraux2(nr, n, a, i, t1, t2);
    }
  }
} /* qrupdt_ */

/* 	subroutine tregup */

/* 	purpose */

/* 	decide whether to accept xpls=x+sc as the next iterate and update the */
/* 	trust region dlt. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate x[k-1] */
/* 	f	     --> function value at old iterate, f(x) */
/* 	g(n)	     --> gradient at old iterate, g(x), or approximate */
/* 	a(n,n)	     --> cholesky decomposition of hessian in */
/* 			 lower triangular part and diagonal. */
/* 			 hessian or approx in upper triangular part */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in tregup (but can be */
/*			 modified by fcn). */
/* 	sc(n)	     --> current step */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	nwtake	     --> boolean, =.true. if newton step taken */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	dlt	    <--> trust region radius */
/* 	iretcd	    <--> return code */
/* 			   =0 xpls accepted as next iterate; */
/* 			      dlt trust region for next iteration. */
/* 			   =1 xpls unsatisfactory but accepted as next iterate */
/* 			      because xpls-x .lt. smallest allowable */
/* 			      step length. */
/* 			   =2 f(xpls) too large.  continue current iteration */
/* 			      with new reduced dlt. */
/* 			   =3 f(xpls) sufficiently small, but quadratic model */
/* 			      predicts f(xpls) sufficiently well to continue */
/* 			      current iteration with new doubled dlt. */
/* 	xplsp(n)    <--> workspace [value needs to be retained between */
/* 			 succesive calls of k-th global step] */
/* 	fplsp	    <--> [retain value between successive calls] */
/* 	xpls(n)	    <--	 new iterate x[k] */
/* 	fpls	    <--	 function value at new iterate, f(xpls) */
/* 	mxtake	    <--	 boolean flag indicating step of maximum length used */
/* 	ipr	     --> device to which to send output */
/* 	method	     --> algorithm to use to solve minimization problem */
/* 			   =1 line search */
/* 			   =2 double dogleg */
/* 			   =3 more-hebdon */
/* 	udiag(n)     --> diagonal of hessian in a(.,.) */

static void
tregup(int nr, int n, double *x, double f, double *g, double *a, fcn_p
       fcn, void *state, double *sc, double *sx, int nwtake, double
       stepmx, double steptl, double *dlt, int *iretcd, double *xplsp,
       double *fplsp, double *xpls, double *fpls, int *mxtake, int
       method, double *udiag)
{
  double dltf;
  double temp1, temp2;
  int i, j, one = 1;
  double dltfp, dltmp;
  double rln, slp;

  *mxtake = 0;
  for (i = 0; i < n; ++i) {
    xpls[i] = x[i] + sc[i];
  }
  (*fcn)(n, xpls, fpls, state);
  dltf = *fpls - f;
  slp = F77_CALL(ddot)(&n, g, &one, sc, &one);

  /* 	next statement added for case of compilers which do not optimize */
  /* 	evaluation of next "if" statement (in which case fplsp could be */
  /* 	undefined). */

  if (*iretcd == 4) {
    *fplsp = 0.;
  }
  if (*iretcd == 3 && (*fpls >= *fplsp || dltf > slp * 1e-4)) {
    /* 	reset xpls to xplsp and terminate global step */

    *iretcd = 0;
    for (i = 0; i < n; ++i) {
      xpls[i] = xplsp[i];
    }
    *fpls = *fplsp;
    *dlt *= .5;
  } else {

    /* 	fpls too large */

    if (dltf > slp * 1e-4) {
      rln = 0.;
      for (i = 0; i < n; ++i) {
	temp1 = fabs(xpls[i]);
	temp2 = 1.0/sx[i];
	temp1 = fabs(sc[i])/max(temp1, temp2);
	if(temp1 > rln)
	  rln = temp1;
      }
      if (rln < steptl) {
	/*  cannot find satisfactory xpls sufficiently distinct from x */
	*iretcd = 1;
      } else {
	/*  reduce trust region and continue global step */
	*iretcd = 2;
	dltmp = -slp * *dlt / ((dltf - slp) * 2.);
	if (dltmp < *dlt * .1) {
	  *dlt *= .1;
	} else {
	  *dlt = dltmp;
	}
      }
    } else {
      /* fpls sufficiently small */

      dltfp = 0.;
      if (method == 2) {
	for (i = 0; i < n; ++i) {
	  temp1 = 0.;
	  for (j = i; j < n; ++j) {
	    temp1 += a[j + i * nr] * sc[j];
	  }
	  dltfp += temp1 * temp1;
	}
      } else {
	for (i = 0; i < n; ++i) {
	  dltfp += udiag[i] * sc[i] * sc[i];
	  temp1 = 0.;
	  for (j = i+1; j < n; ++j) {
	    temp1 += a[i + j * nr] * sc[i] * sc[j];
	  }
	  dltfp += temp1 * 2.;
	}
      }
      dltfp = slp + dltfp / 2.;
      if (*iretcd != 2 && (fabs(dltfp - dltf) <= fabs(dltf) * 0.1 &&
			   nwtake && *dlt <= stepmx * .99)) {
			     
	/* double trust region and continue global step */
	
	*iretcd = 3;
	for (i = 0; i < n; ++i) {
	  xplsp[i] = xpls[i];
	}
	*fplsp = *fpls;
	temp1 = *dlt * 2.0;
	*dlt = min(temp1, stepmx);
      } else {
	/* 	    accept xpls as next iterate.  choose new trust region. */
      
	*iretcd = 0;
	if (*dlt > stepmx * .99) {
	  *mxtake = 1;
	}
	if (dltf >= dltfp * .1) {
	  /* 	      decrease trust region for next iteration */

	  *dlt *= .5;
	} else {
	  /* 	      check whether to increase trust region for next iteration */

	  if (dltf <= dltfp * .75) {
	    temp1 = *dlt * 2.0;
	    *dlt = min(temp1, stepmx);
	  }
	}
      }
    }
  }
} /* tregup */

/* 	subroutine lnsrch */

/* 	purpose */

/* 	find a next newton iterate by line search. */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate:	x[k-1] */
/* 	f	     --> function value at old iterate, f(x) */
/* 	g(n)	     --> gradient at old iterate, g(x), or approximate */
/* 	p(n)	     --> non-zero newton step */
/* 	xpls(n)	    <--	 new iterate x[k] */
/* 	fpls	    <--	 function value at new iterate, f(xpls) */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in lnsrch (but can be */
/*			 modified by fcn). */
/* 	iretcd	    <--	 return code */
/* 	mxtake	    <--	 boolean flag indicating step of maximum length used */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	sx(n)	     --> diagonal scaling matrix for x */

/* 	internal variables */

/* 	sln		 newton length */
/* 	rln		 relative length of newton step */

static void
lnsrch(int n, double *x, double f, double *g, double *p, double *xpls,
       double *fpls, fcn_p fcn, void *state, int *mxtake, int *iretcd,
       double stepmx, double steptl, double *sx)
{
    double disc;
    double a, b;
    int i, one = 1, firstback = 1;
    double t1, t2, t3, almbda, tlmbda, rmnlmb;
    double scl, rln, sln, slp;
    double temp1, temp2;
    double pfpls = 0.0, plmbda = 0.0; /* -Wall */

    *mxtake = 0;
    *iretcd = 2;
    temp1 = 0.;
    for (i = 0; i < n; ++i) {
	temp1 += sx[i] * sx[i] * p[i] * p[i];
    }
    sln = sqrt(temp1);
    if (sln > stepmx) {
	/* 	newton step longer than maximum allowed */
	scl = stepmx / sln;
	F77_CALL(dscal)(&n, &scl, p, &one);
	sln = stepmx;
    }
    slp = F77_CALL(ddot)(&n, g, &one, p, &one);
    rln = 0.0;
    for (i = 0; i < n; ++i) {
	temp1 = fabs(x[i]);
	temp2 = 1.0/sx[i];
	temp1 = fabs(p[i])/ max(temp1, temp2);
	rln = max(rln, temp1);
    }
    rmnlmb = steptl / rln;
    almbda = 1.0;

    /* 	check if new iterate satisfactory.  generate new lambda if necessary. */

    while(*iretcd > 1) {
	for (i = 0; i < n; ++i)
	    xpls[i] = x[i] + almbda * p[i];
	(*fcn)(n, xpls, fpls, state);
	if (*fpls <= f + slp * 1e-4 * almbda) {
	    /* solution found */

	    *iretcd = 0;
	    if (almbda == 1. && sln > stepmx * .99) {
		*mxtake = 1;
	    }
	    return;
	} else {
	    /* solution not (yet) found */

	    /* First find a point with a finite value */
	    if (almbda < rmnlmb) {
		/* no satisfactory xpls found sufficiently distinct from x */

		*iretcd = 1;
		return;
	    } else {
		/* 	calculate new lambda */
		/* modifications by BDR 2000/01/05 to cover non-finite values */		
		if (*fpls == DBL_MAX) {
		    almbda *= 0.1;
		    firstback = 1;
		} else {
		    if (firstback) {
			/* 	first backtrack: quadratic fit */
			tlmbda = -almbda * slp / ((*fpls - f - slp) * 2.);
			firstback = 0;
		    } else {
			/* 	all subsequent backtracks: cubic fit */
			t1 = *fpls - f - almbda * slp;
			t2 = pfpls - f - plmbda * slp;
			t3 = 1. / (almbda - plmbda);
			a = t3 * (t1 / (almbda * almbda) - t2 / (plmbda * plmbda));
			b = t3 * (t2 * almbda / (plmbda * plmbda) - t1 * plmbda / (almbda * 
										   almbda));
			disc = b * b - a * 3. * slp;
			if (disc > b * b) {
			    /* 	only one positive critical point, must be minimum */

			    tlmbda = (-b + ((a < 0)? - sqrt(disc): sqrt(disc)))/(a * 3.);
			} else {
			    /* 	both critical points positive, first is minimum */
			    tlmbda = (-b + ((a < 0)? sqrt(disc): -sqrt(disc)))/ (a * 3.);
			}
			if (tlmbda > almbda * .5) {
			    tlmbda = almbda * .5;
			}
		    }
		    plmbda = almbda;
		    pfpls = *fpls;
		    if (tlmbda < almbda * .1) {
			almbda *= .1;
		    } else {
			almbda = tlmbda;
		    }
		}
	    }
	}
    }
} /* lnsrch */

/* 	subroutine dogstp */

/* 	purpose */

/* 	find new step by double dogleg algorithm */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	g(n)	     --> gradient at current iterate, g(x) */
/* 	a(n,n)	     --> cholesky decomposition of hessian in */
/* 			 lower part and diagonal */
/* 	p(n)	     --> newton step */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	rnwtln	     --> newton step length */
/* 	dlt	    <--> trust region radius */
/* 	nwtake	    <--> boolean, =.true. if newton step taken */
/* 	fstdog	    <--> boolean, =.true. if on first leg of dogleg */
/* 	ssd(n)	    <--> workspace [cauchy step to the minimum of the */
/* 			 quadratic model in the scaled steepest descent */
/* 			 direction] [retain value between successive calls] */
/* 	v(n)	    <--> workspace  [retain value between successive calls] */
/* 	cln	    <--> cauchy length */
/* 			 [retain value between successive calls] */
/* 	eta		 [retain value between successive calls] */
/* 	sc(n)	    <--	 current step */
/* 	ipr	     --> device to which to send output */
/* 	stepmx	     --> maximum allowable step size */

/* 	internal variables */

/* 	cln		 length of cauchy step */

static void
dogstp(int nr, int n, double *g, double *a, double *p, double *sx,
       double rnwtln, double *dlt, int *nwtake, int *fstdog, double
       *ssd, double *v, double *cln, double *eta, double *sc, double
       stepmx)
{
  double alam, beta;
  int i, j, one = 1;
  double alpha, tmp, dot1, dot2;

  /* 	can we take newton step */

  if (rnwtln <= *dlt) {
    *nwtake = 1;
    for (i = 0; i < n; ++i) {
      sc[i] = p[i];
    }
    *dlt = rnwtln;
    return;
  }

  /* 	newton step too long */
  /* 	cauchy step is on double dogleg curve */

  *nwtake = 0;
  if (*fstdog) {
    /* 	  calculate double dogleg curve (ssd) */
    *fstdog = 0;
    alpha = 0.;
    for (i = 0; i < n; ++i) {
      alpha += g[i] * g[i] / (sx[i] * sx[i]);
    }
    beta = 0.;
    for (i = 0; i < n; ++i) {
      tmp = 0.;
      for (j = i; j < n; ++j) {
	tmp += a[j + i * nr] * g[j] / (sx[j] * sx[j]);
      }
      beta += tmp * tmp;
    }
    for (i = 0; i < n; ++i) {
      ssd[i] = -(alpha / beta) * g[i] / sx[i];
    }
    *cln = alpha * sqrt(alpha) / beta;
    *eta = (alpha * .8 * alpha / 
	    (-beta * F77_CALL(ddot)(&n, g, &one, p, &one))) + .2;
    for (i = 0; i < n; ++i) {
      v[i] = *eta * sx[i] * p[i] - ssd[i];
    }
    if (*dlt == -1.) {
      *dlt = min(*cln, stepmx);
    }
  }
  if (*eta * rnwtln <= *dlt) {

    /* 	  take partial step in newton direction */
    for (i = 0; i < n; ++i) {
      sc[i] = *dlt / rnwtln * p[i];
    }
  } else {
    if (*cln >= *dlt) {
      /* 	    take step in steepest descent direction */

      for (i = 0; i < n; ++i) {
	sc[i] = *dlt / *cln * ssd[i] / sx[i];
      }
    } else {
      /* calculate convex combination of ssd and eta*p */
      /* which has scaled length dlt */
      dot1 = F77_CALL(ddot)(&n, v, &one, ssd, &one);
      dot2 = F77_CALL(ddot)(&n, v, &one, v, &one);
      alam = (-dot1 + sqrt(dot1 * dot1 - dot2 * (*cln * *cln - *dlt *
						 *dlt))) / dot2;
      for (i = 0; i < n; ++i) {
	sc[i] = (ssd[i] + alam * v[i]) / sx[i];
      }
    }
  }
} /* dogstp */

/* 	subroutine dogdrv */

/* 	purpose */

/* 	find a next newton iterate (xpls) by the double dogleg method */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate x[k-1] */
/* 	f	     --> function value at old iterate, f(x) */
/* 	g(n)	     --> gradient  at old iterate, g(x), or approximate */
/* 	a(n,n)	     --> cholesky decomposition of hessian */
/* 			 in lower triangular part and diagonal */
/* 	p(n)	     --> newton step */
/* 	xpls(n)	    <--	 new iterate x[k] */
/* 	fpls	    <--	 function value at new iterate, f(xpls) */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in dogdrv (but can be */
/*			 modified by fcn). */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	dlt	    <--> trust region radius */
/* 			 [retain value between successive calls] */
/* 	iretcd	    <--	 return code */
/* 			   =0 satisfactory xpls found */
/* 			   =1 failed to find satisfactory xpls sufficiently */
/* 			      distinct from x */
/* 	mxtake	    <--	 boolean flag indicating step of maximum length used */
/* 	sc(n)	     --> workspace [current step] */
/* 	wrk1(n)	     --> workspace (and place holding argument to tregup) */
/* 	wrk2(n)	     --> workspace */
/* 	wrk3(n)	     --> workspace */
/* 	ipr	     --> device to which to send output */

static void
dogdrv(int nr, int n, double *x, double f, double *g, double *a, double *p, 
       double *xpls, double *fpls, fcn_p fcn, void *state, double *sx,
       double stepmx, double steptl, double *dlt, int *iretcd, int *mxtake,
       double *sc, double *wrk1, double *wrk2, double *wrk3, int
       *itncnt)
{
  int i;
  double fplsp;
  int fstdog, nwtake;
  double rnwtln, eta, cln, tmp;

  *iretcd = 4;
  fstdog = 1;
  tmp = 0.;
  for (i = 0; i < n; ++i) {
    tmp += sx[i] * sx[i] * p[i] * p[i];
  }
  rnwtln = sqrt(tmp);

  while(*iretcd > 1) {
    /* 	find new step by double dogleg algorithm */

    dogstp(nr, n, g, a, p, sx, rnwtln, dlt, &nwtake,
	   &fstdog, wrk1, wrk2, &cln, &eta, sc, stepmx);

    /* 	check new point and update trust region */

    tregup(nr, n, x, f, g, a, (fcn_p)fcn, state, sc, sx, nwtake, stepmx,
	   steptl, dlt, iretcd, wrk3, &fplsp, xpls, fpls, mxtake,
	   2, wrk1);
  }
} /* dogdrv */

/* 	 subroutine hookst */

/* 	 purpose */

/* 	 find new step by more-hebdon algorithm */

/* 	 parameters */

/* 	 nr	      --> row dimension of matrix */
/* 	 n	      --> dimension of problem */
/* 	 g(n)	      --> gradient at current iterate, g(x) */
/* 	 a(n,n)	      --> cholesky decomposition of hessian in */
/* 			  lower triangular part and diagonal. */
/* 			  hessian or approx in upper triangular part */
/* 	 udiag(n)     --> diagonal of hessian in a(.,.) */
/* 	 p(n)	      --> newton step */
/* 	 sx(n)	      --> diagonal scaling matrix for n */
/* 	 rnwtln	      --> newton step length */
/* 	 dlt	     <--> trust region radius */
/* 	 amu	     <--> [retain value between successive calls] */
/* 	 dltp	      --> trust region radius at last exit from this routine */
/* 	 phi	     <--> [retain value between successive calls] */
/* 	 phip0	     <--> [retain value between successive calls] */
/* 	 fstime	     <--> boolean. =.true. if first entry to this routine */
/* 			  during k-th iteration */
/* 	 sc(n)	     <--  current step */
/* 	 nwtake	     <--  boolean, =.true. if newton step taken */
/* 	 wrk0(n)      --> workspace */
/* 	 epsm	      --> machine epsilon */

static void
hookst(int nr, int n, double *g, double *a, double *udiag, double *p,
       double *sx, double rnwtln, double *dlt, double *amu, double
       dltp, double *phi, double *phip0, int *fstime, double *sc,
       int *nwtake, double *wrk0, double epsm)
{
  int one = 1, job = 0, info;
  double phip;
  int i, j;
  double amulo, amuup, hi;
  double addmax, stepln;
  double alo;
  double temp1, temp2;
/* 	 hi and alo are constants used in this routine. */
/* 	 change here if other values are to be substituted. */
  hi = 1.5;
  alo = .75;

  if (rnwtln <= hi * *dlt) {
    /* 	take newton step */

    *nwtake = 1;
    for (i = 0; i < n; ++i) {
      sc[i] = p[i];
    }
    *dlt = min(*dlt, rnwtln);
    *amu = 0.;
    return;
  } else {
    /* 	newton step not taken */
    *nwtake = 0;
    if (*amu > 0.) {
      *amu -= (*phi + dltp) * (dltp - *dlt + *phi) / (*dlt * *phip0);
    }
    *phi = rnwtln - *dlt;
    if (*fstime) {
      for (i = 0; i < n; ++i) {
	wrk0[i] = sx[i] * sx[i] * p[i];
      }
      /* 	  solve l*y = (sx**2)*p */
      F77_CALL(dtrsl)(a, &nr, &n, wrk0, &job, &info);
      /* Computing 2nd power */
      temp1 = F77_CALL(dnrm2)(&n, wrk0, &one);
      *phip0 = -(temp1 * temp1) / rnwtln;
      *fstime = 0;
    }
    phip = *phip0;
    amulo = -(*phi) / phip;
    amuup = 0.;
    for (i = 0; i < n; ++i) {
      amuup += g[i] * g[i] / (sx[i] * sx[i]);
    }
    amuup = sqrt(amuup) / *dlt;

    while (1) {
      /* 	test value of amu; generate next amu if necessary */
      if (*amu < amulo || *amu > amuup) {
	temp1 = sqrt(amulo * amuup);
	temp2 = amuup * .001;
	*amu = max(temp1, temp2);
      }

      /* 	copy (h,udiag) to l */
      /* 	where h <-- h+amu*(sx**2) [do not actually change (h,udiag)] */
      for (i = 0; i < n; ++i) {
	a[i + i * nr] = udiag[i] + *amu * sx[i] * sx[i];
	for (j = 0; j < i; ++i) {
	  a[i + j * nr] = a[j + i * nr];
	}
      }
      
      /* 	factor h=l(l+) */

      temp1 = sqrt(epsm);
      choldc(nr, n, a, 0.0, temp1, &addmax);

      /* 	solve h*p = l(l+)*sc = -g */

      for (i = 0; i < n; ++i) {
	wrk0[i] = -g[i];
      }
      lltslv(nr, n, a, sc, wrk0);

      /* reset h.  note since udiag has not been destroyed we need do */
      /* nothing here.  h is in the upper part and in udiag, still intact */

      stepln = 0.;
      for (i = 0; i < n; ++i) {
	stepln += sx[i] * sx[i] * sc[i] * sc[i];
      }
      stepln = sqrt(stepln);
      *phi = stepln - *dlt;
      for (i = 0; i < n; ++i) {
	wrk0[i] = sx[i] * sx[i] * sc[i];
      }
      F77_CALL(dtrsl)(a, &nr, &n, wrk0, &job, &info);
      temp1 = F77_CALL(dnrm2)(&n, wrk0, &one);
      phip = -(temp1 * temp1) / stepln;
      if ((alo * *dlt <= stepln && stepln <= hi * *dlt) || (amuup -
							    amulo > 0.)) {

	/* 	  sc is acceptable hookstep */

	break;
      } else {
	/* 	  sc not acceptable hookstep.  select new amu */
	temp1 = (*amu - *phi)/phip;
	amulo = max(amulo, temp1);
	if (*phi < 0.) {
	  amuup = min(amuup,*amu);
	}
	*amu -= stepln * *phi / (*dlt * phip);
      }
    }
  }
} /* hookst */

/* 	subroutine hookdr */

/* 	purpose */

/* 	find a next newton iterate (xpls) by the more-hebdon method */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate x[k-1] */
/* 	f	     --> function value at old iterate, f(x) */
/* 	g(n)	     --> gradient at old iterate, g(x), or approximate */
/* 	a(n,n)	     --> cholesky decomposition of hessian in lower */
/* 			 triangular part and diagonal. */
/* 			 hessian in upper triangular part and udiag. */
/* 	udiag(n)     --> diagonal of hessian in a(.,.) */
/* 	p(n)	     --> newton step */
/* 	xpls(n)	    <--	 new iterate x[k] */
/* 	fpls	    <--	 function value at new iterate, f(xpls) */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in hookdr (but can be */
/*			 modified by fcn). */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	dlt	    <--> trust region radius */
/* 	iretcd	    <--	 return code */
/* 			   =0 satisfactory xpls found */
/* 			   =1 failed to find satisfactory xpls sufficiently */
/* 			      distinct from x */
/* 	mxtake	    <--	 boolean flag indicating step of maximum length used */
/* 	amu	    <--> [retain value between successive calls] */
/* 	dltp	    <--> [retain value between successive calls] */
/* 	phi	    <--> [retain value between successive calls] */
/* 	phip0	    <--> [retain value between successive calls] */
/* 	sc(n)	     --> workspace */
/* 	xplsp(n)     --> workspace */
/* 	wrk0(n)	     --> workspace */
/* 	epsm	     --> machine epsilon */
/* 	itncnt	     --> iteration count */
/* 	ipr	     --> device to which to send output */

static void
hookdr(int nr, int n, double *x, double f, double *g, double *a,
       double *udiag, double *p, double *xpls, double *fpls, fcn_p fcn, 
       void *state, double *sx, double stepmx, double steptl, double *
       dlt, int *iretcd, int *mxtake, double *amu, double *
       dltp, double *phi, double *phip0, double *sc, double *
       xplsp, double *wrk0, double epsm, int itncnt)
{
  double beta;
  int i, j;
  double alpha, fplsp;
  int fstime, nwtake;
  double rnwtln, tmp;

  *iretcd = 4;
  fstime = 1;
  tmp = 0.;
  for (i = 0; i < n; ++i) {
    tmp += sx[i] * sx[i] * p[i] * p[i];
  }
  rnwtln = sqrt(tmp);

  if (itncnt == 1) {
    *amu = 0.;

    /* 	if first iteration and trust region not provided by user, */
    /* 	compute initial trust region. */

    if (*dlt == -1.) {

      alpha = 0.;
      for (i = 0; i < n; ++i) {
	alpha += g[i] * g[i] / (sx[i] * sx[i]);
      }
      beta = 0.;
      for (i = 0; i < n; ++i) {
	tmp = 0.;
	for (j = i; j < n; ++j) {
	  tmp += a[j + i * nr] * g[j] / (sx[j] * sx[j]);
	}
	beta += tmp * tmp;
      }
      *dlt = alpha * sqrt(alpha) / beta;
      *dlt = min(*dlt, stepmx);
    }
  }
  while(*iretcd > 1) {

    /* 	find new step by more-hebdon algorithm */

    hookst(nr, n, g, a, udiag, p, sx, rnwtln, 
	   dlt, amu, *dltp, phi, phip0, &fstime, sc, &nwtake, wrk0,
	   epsm);
    *dltp = *dlt;

    /* 	check new point and update trust region */

    tregup(nr, n, x, f, g, a, (fcn_p)fcn, state, sc, sx, nwtake, stepmx,
	   steptl, dlt, iretcd, xplsp, &fplsp, xpls, fpls, mxtake,
	   3, udiag);
  }
} /* hookdr */

/* 	subroutine secunf */

/* 	purpose */

/* 	update hessian by the bfgs unfactored method */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate, x[k-1] */
/* 	g(n)	     --> gradient or approximate at old iterate */
/* 	a(n,n)	    <--> on entry: approximate hessian at old iterate */
/* 			   in upper triangular part (and udiag) */
/* 			 on exit:  updated approx hessian at new iterate */
/* 			   in lower triangular part and diagonal */
/* 			 [lower triangular part of symmetric matrix] */
/* 	udiag	     --> on entry: diagonal of hessian */
/* 	xpls(n)	     --> new iterate, x[k] */
/* 	gpls(n)	     --> gradient or approximate at new iterate */
/* 	epsm	     --> machine epsilon */
/* 	itncnt	     --> iteration count */
/* 	rnf	     --> relative noise in optimization function fcn */
/* 	iagflg	     --> =1 if analytic gradient supplied, =0 otherwise */
/* 	noupdt	    <--> boolean: no update yet */
/* 			 [retain value between successive calls] */
/* 	s(n)	     --> workspace */
/* 	y(n)	     --> workspace */
/* 	t(n)	     --> workspace */

static void
secunf(int nr, int n, double *x, double *g, double *a, double *udiag,
       double *xpls, double *gpls, double epsm, int itncnt, double
       rnf, int iagflg, int *noupdt, double *s, double *y, double *t)
{
  double ynrm2;
  int i, j, one = 1;
  double snorm2;
  int skpupd;
  double gam, tol, den1, den2;
  double temp1, temp2;


/* 	copy hessian in upper triangular part and udiag to */
/* 	lower triangular part and diagonal */

  for (i = 0; i < n; ++i) {
    a[i + i * nr] = udiag[i];
    for (j = 0; j < i; ++j) {
      a[i + j * nr] = a[j + i * nr];
    }
  }

  *noupdt = (itncnt == 1);

  for (i = 0; i < n; ++i) {
    s[i] = xpls[i] - x[i];
    y[i] = gpls[i] - g[i];
  }
  den1 = F77_CALL(ddot)(&n, s, &one, y, &one);
  snorm2 = F77_CALL(dnrm2)(&n, s, &one);
  ynrm2 = F77_CALL(dnrm2)(&n, y, &one);
  if (den1 < sqrt(epsm) * snorm2 * ynrm2) {
    return;
  }

  mvmlts(nr, n, a, s, t);
  den2 = F77_CALL(ddot)(&n, s, &one, t, &one);
  if (*noupdt) {
    /* 	  h <-- [(s+)y/(s+)hs]h */

    gam = den1 / den2;
    den2 = gam * den2;;
    for (j = 0; j < n; ++j) {
      t[j] *= gam;
      for (i = j; i < n; ++i) {
	a[i + j * nr] *= gam;
      }
    }
    *noupdt = 0;
  }
  skpupd = 1;

  /* 	check update condition on row i */

  for (i = 0; i < n; ++i) {
    temp1 = fabs(g[i]);
    temp2 = fabs(gpls[i]);
    tol = rnf * max(temp1, temp2);
    if (iagflg == 0) {
      tol /= sqrt(rnf);
    }
    if (fabs(y[i] - t[i]) >= tol) {
      skpupd = 0;
      break;
    }
  }
  if (skpupd) {
    return;
  }

  /* 	  bfgs update */

  for (j = 0; j < n; ++j) {
    for (i = j; i < n; ++i) {
      a[i + j * nr] = a[i + j * nr] + y[i] * y[j] / den1 
	- t[i] * t[j] / den2;
    }
  }
} /* secunf */

/* 	subroutine secfac */

/* 	purpose */

/* 	update hessian by the bfgs factored method */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> old iterate, x[k-1] */
/* 	g(n)	     --> gradient or approximate at old iterate */
/* 	a(n,n)	    <--> on entry: cholesky decomposition of hessian in */
/* 			   lower part and diagonal. */
/* 			 on exit:  updated cholesky decomposition of hessian */
/* 			   in lower triangular part and diagonal */
/* 	xpls(n)	     --> new iterate, x[k] */
/* 	gpls(n)	     --> gradient or approximate at new iterate */
/* 	epsm	     --> machine epsilon */
/* 	itncnt	     --> iteration count */
/* 	rnf	     --> relative noise in optimization function fcn */
/* 	iagflg	     --> =1 if analytic gradient supplied, =0 itherwise */
/* 	noupdt	    <--> boolean: no update yet */
/* 			 [retain value between successive calls] */
/* 	s(n)	     --> workspace */
/* 	y(n)	     --> workspace */
/* 	u(n)	     --> workspace */
/* 	w(n)	     --> workspace */

static void
secfac(int nr, int n, double *x, double *g, double *a, double *xpls,
       double *gpls, double epsm, int itncnt, double rnf, int iagflg, 
       int *noupdt, double *s, double *y, double *u, double *w)
{
  double ynrm2;
  int i, j, one = 1;
  double snorm2, reltol;
  int skpupd;
  double alp, den1, den2;
  double temp1, temp2;

  *noupdt = (itncnt == 1);

  for (i = 0; i < n; ++i) {
    s[i] = xpls[i] - x[i];
    y[i] = gpls[i] - g[i];
  }
  den1 = F77_CALL(ddot)(&n, s, &one, y, &one);
  snorm2 = F77_CALL(dnrm2)(&n, s, &one);
  ynrm2 = F77_CALL(dnrm2)(&n, y, &one);

  if (den1 < sqrt(epsm) * snorm2 * ynrm2)
    return;

  mvmltu(nr, n, a, s, u);
  den2 = F77_CALL(ddot)(&n, u, &one, u, &one);

  /* 	l <-- sqrt(den1/den2)*l */

  alp = sqrt(den1 / den2);
  if (*noupdt) {
    for (j = 0; j < n; ++j) {
      u[j] = alp * u[j];
      for (i = j; i < n; ++i) {
	a[i + j * nr] *= alp;
      }
    }
    *noupdt = 0;
    den2 = den1;
    alp = 1.;
  }
  /* 	w = l(l+)s = hs */

  mvmltl(nr, n, a, u, w);
  if (iagflg == 0) {
    reltol = sqrt(rnf);
  } else {
    reltol = rnf;
  }
  skpupd = 1;
  for (i = 0; i < n; ++i) {
    temp1 = fabs(g[i]);
    temp2 = fabs(gpls[i]);
    skpupd = (fabs(y[i] - w[i]) < reltol * max(temp1, temp2));
    if(!skpupd)
      break;
  }

  if(skpupd)
    return;

  /* 	  w=y-alp*l(l+)s */

  for (i = 0; i < n; ++i) {
    w[i] = y[i] - alp * w[i];
  }

  /* 	  alp=1/sqrt(den1*den2) */

  alp /= den1;

  /* 	  u=(l+)/sqrt(den1*den2) = (l+)s/sqrt((y+)s * (s+)l(l+)s) */

  for (i = 0; i < n; ++i) {
    u[i] *= alp;
  }

  /* 	  copy l into upper triangular part.  zero l. */

  for (i = 1; i < n; ++i) {
    for (j = 0; j < i; ++j) {
      a[j + i * nr] = a[i + j * nr];
      a[i + j * nr] = 0.;
    }
  }

  /* 	  find q, (l+) such that  q(l+) = (l+) + u(w+) */

  qrupdt(nr, n, a, u, w);

  /* 	  upper triangular part and diagonal of a now contain updated */
  /* 	  cholesky decomposition of hessian.  copy back to lower */
  /* 	  triangular part. */

  for (i = 1; i < n; ++i) {
    for (j = 0; j < i; ++j) {
      a[i + j * nr] = a[j + i * nr];
    }
  }
} /* secfac */

/* 	subroutine chlhsn */

/* 	purpose */

/* 	find the l(l-transpose) [written ll+] decomposition of the perturbed */
/* 	model hessian matrix a+mu*i(where mu\0 and i is the identity matrix) */
/* 	which is safely positive definite.  if a is safely positive definite */
/* 	upon entry, then mu=0. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	    <--> on entry; "a" is model hessian (only lower */
/* 			 triangular part and diagonal stored) */
/* 			 on exit:  a contains l of ll+ decomposition of */
/* 			 perturbed model hessian in lower triangular */
/* 			 part and diagonal and contains hessian in upper */
/* 			 triangular part and udiag */
/* 	epsm	     --> machine epsilon */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	udiag(n)    <--	 on exit: contains diagonal of hessian */

/* 	internal variables */

/* 	tol		 tolerance */
/* 	diagmn		 minimum element on diagonal of a */
/* 	diagmx		 maximum element on diagonal of a */
/* 	offmax		 maximum off-diagonal element of a */
/* 	offrow		 sum of off-diagonal elements in a row of a */
/* 	evmin		 minimum eigenvalue of a */
/* 	evmax		 maximum eigenvalue of a */

/* 	description */

/* 	1. if "a" has any negative diagonal elements, then choose mu>0 */
/* 	such that the diagonal of a:=a+mu*i is all positive */
/* 	with the ratio of its smallest to largest element on the */
/* 	order of sqrt(epsm). */

/* 	2. "a" undergoes a perturbed cholesky decomposition which */
/* 	results in an ll+ decomposition of a+d, where d is a */
/* 	non-negative diagonal matrix which is implicitly added to */
/* 	"a" during the decomposition if "a" is not positive definite. */
/* 	"a" is retained and not changed during this process by */
/* 	copying l into the upper triangular part of "a" and the */
/* 	diagonal into udiag.  then the cholesky decomposition routine */
/* 	is called.  on return, addmax contains maximum element of d. */

/* 	3. if addmax=0, "a" was positive definite going into step 2 */
/* 	and return is made to calling program.	otherwise, */
/* 	the minimum number sdd which must be added to the */
/* 	diagonal of a to make it safely strictly diagonally dominant */
/* 	is calculated.	since a+addmax*i and a+sdd*i are safely */
/* 	positive definite, choose mu=min(addmax,sdd) and decompose */
/* 	a+mu*i to obtain l. */

static void
chlhsn(int nr, int n, double *a, double epsm, double *sx, double *udiag)
{
  int i, j;
  double evmin, evmax;
  double addmax, diagmn, diagmx, offmax, offrow, posmax;
  double sdd, amu, tol, tmp;


  /* 	scale hessian */
  /* 	pre- and post- multiply "a" by inv(sx) */

  for (j = 0; j < n; ++j) {
    for (i = j; i < n; ++i) {
      a[i + j * nr] /= sx[i] * sx[j];
    }
  }

  /* 	step1 */
  /* 	----- */
  /* 	note:  if a different tolerance is desired throughout this */
  /* 	algorithm, change tolerance here: */

  tol = sqrt(epsm);

  diagmx = a[0];
  diagmn = a[0];
  if (n > 1) {
    for (i = 1; i < n; ++i) {
      tmp = a[i + i * nr];
      if(tmp < diagmn)
	diagmn = tmp;
      if(tmp > diagmx)
	diagmx = tmp;
    }
  }
  posmax = max(diagmx, 0.0);

  if (diagmn <= posmax * tol) {
    amu = tol * (posmax - diagmn) - diagmn;
    if (amu == 0.) {
      /* 	find largest off-diagonal element of a */
      offmax = 0.;
      for (i = 1; i < n; ++i) {
	for (j = 0; j < i; ++j) {
	  if ((tmp = fabs(a[i + j * nr])) > offmax) {
	    offmax = tmp;
	  }
	}
      }
      amu = offmax;
      if (amu == 0.) {
	amu = 1.;
      } else {
	amu *= tol + 1.;
      }
    }
    /* 	a=a + mu*i */
    for (i = 0; i < n; ++i) {
      a[i + i * nr] += amu;
    }
    diagmx += amu;
  }

  /* 	copy lower triangular part of "a" to upper triangular part */
  /* 	and diagonal of "a" to udiag */
  for (i = 0; i < n; ++i) {
    udiag[i] = a[i + i * nr];
    for (j = 0; j < i; ++j) {
      a[j + i * nr] = a[i + j * nr];
    }
  }
  choldc(nr, n, a, diagmx, tol, &addmax);


  /* 	step3 */

  /* 	if addmax=0, "a" was positive definite going into step 2, */
  /* 	the ll+ decomposition has been done, and we return. */
  /* 	otherwise, addmax>0.  perturb "a" so that it is safely */
  /* 	diagonally dominant and find ll+ decomposition */

  if (addmax > 0.0) {

    /* 	restore original "a" (lower triangular part and diagonal) */

    for (i = 0; i < n; ++i) {
      a[i + i * nr] = udiag[i];
      for (j = 0; j < i; ++j) {
	a[i + j * nr] = a[j + i * nr];
      }
    }

    /* 	find sdd such that a+sdd*i is safely positive definite */
    /* 	note:  evmin<0 since a is not positive definite; */

    evmin = 0.;
    evmax = a[0];
    for (i = 0; i < n; ++i) {
      offrow = 0.;
      for (j = 0; j < i; ++j) {
	offrow += fabs(a[i + j * nr]);
      }
      for (j = i+1; j < n; ++j) {
	offrow += fabs(a[j + i * nr]);
      }
      tmp  = a[i + i * nr] - offrow;
      if(tmp < evmin)
	evmin = tmp;
      tmp  = a[i + i * nr] + offrow;
      if(tmp > evmax)
	evmax = tmp;
    }
    sdd = tol * (evmax - evmin) - evmin;

    /* 	perturb "a" and decompose again */

    amu = min(sdd, addmax);
    for (i = 0; i < n; ++i) {
      a[i + i * nr] += amu;
      udiag[i] = a[i + i * nr];
    }

    /* 	 "a" now guaranteed safely positive definite */

    choldc(nr, n, a, 0.0, tol, &addmax);
  }
  /* 	unscale hessian and cholesky decomposition matrix */

  for (j = 0; j < n; ++j) {
    for (i = j; i < n; ++i) {
      a[i + j * nr] *= sx[i];
    }
    for (i = 0; i < j; ++i) {
      a[i + j * nr] *= sx[i] * sx[j];
    }
    udiag[j] *= sx[j] * sx[j];
  }
} /* chlhsn */

/* 	subroutine hsnint */

/* 	purpose */

/* 	provide initial hessian when using secant updates */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	a(n,n)	    <--	 initial hessian (lower triangular matrix) */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	method	     --> algorithm to use to solve minimization problem */
/* 			   =1,2 factored secant method used */
/* 			   =3	unfactored secant method used */

static void
hsnint(int nr, int n, double *a, double *sx, int method)
{
  int i, j;

  for (i = 0; i < n; ++i) {
    if (method == 3) {
      a[i + i * nr] = sx[i] * sx[i];
    } else {
      a[i + i * nr] = sx[i];
    }
    for (j = 0; j < i; ++j) {
      a[i + j * nr] = 0.;
    }
  }
} /* hsnint */


/* 	subroutine fstofd */

/* 	purpose */

/* 	find first order forward finite difference approximation "a" to the */
/* 	first derivative of the function defined by the subprogram "fname" */
/* 	evaluated at the new iterate "xpls". */

/* 	for optimization use this routine to estimate: */
/* 	1) the first derivative (gradient) of the optimization function "fcn */
/* 	   analytic user routine has been supplied; */
/* 	2) the second derivative (hessian) of the optimization function */
/* 	   if no analytic user routine has been supplied for the hessian but */
/* 	   one has been supplied for the gradient ("fcn") and if the */
/* 	   optimization function is inexpensive to evaluate */

/* 	note */

/* 	_m=1 (optimization) algorithm estimates the gradient of the function */
/* 	     (fcn).   fcn(x) # f: r(n)-->r(1) */
/* 	_m=n (systems) algorithm estimates the jacobian of the function */
/* 	     fcn(x) # f: r(n)-->r(n). */
/* 	_m=n (optimization) algorithm estimates the hessian of the optimizatio */
/* 	     function, where the hessian is the first derivative of "fcn" */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	m	     --> number of rows in a */
/* 	n	     --> number of columns in a; dimension of problem */
/* 	xpls(n)	     --> new iterate:  x[k] */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in fstofd (but can be */
/*			 modified by fcn). */
/* 	fpls(m)	     --> _m=1 (optimization) function value at new iterate: */
/* 			      fcn(xpls) */
/* 			 _m=n (optimization) value of first derivative */
/* 			      (gradient) given by user function fcn */
/* 			 _m=n (systems)	 function value of associated */
/* 			      minimization function */
/* 	a(nr,n)	    <--	 finite difference approximation (see note).  only */
/* 			 lower triangular matrix and diagonal are returned */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	rnoise	     --> relative noise in fcn [f(x)] */
/* 	fhat(m)	     --> workspace */
/* 	icase	     --> =1 optimization (gradient) */
/* 			 =2 systems */
/* 			 =3 optimization (hessian) */

/* 	internal variables */

/* 	stepsz - stepsize in the j-th variable direction */

static void
fstofd(int nr, int m, int n, double *xpls, fcn_p fcn, void *state,
       const double *fpls, double *a, double *sx, double rnoise,
       double *fhat, int icase)
{
  int i, j;
  double xtmpj;
  double stepsz;
  double temp1, temp2;


  /* 	find j-th column of a */
  /* 	each column is derivative of f(fcn) with respect to xpls(j) */

  for (j = 0; j < n; ++j) {
    temp1 = fabs(xpls[j]);
    temp2  = 1.0/sx[j];
    stepsz = sqrt(rnoise) * max(temp1, temp2);
    xtmpj = xpls[j];
    xpls[j] = xtmpj + stepsz;
    (*fcn)(n, xpls, fhat, state);
    xpls[j] = xtmpj;
    for (i = 0; i < m; ++i) {
      a[i + j * nr] = (fhat[i] - fpls[i]) / stepsz;
    }
  }
  if (icase == 3 && n > 1) {
    /* 	if computing hessian, a must be symmetric */
    for (i = 1; i < m; ++i) {
      for (j = 0; j < i; ++i) {
	a[i + j * nr] = (a[i + j * nr] + a[j + i * nr]) / 2.0;
      }
    }
  }
} /* fstofd */

/* 	subroutine fstocd */

/* 	purpose */

/* 	find central difference approximation g to the first derivative */
/* 	(gradient) of the function defined by fcn at the point x. */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	x	     --> point at which gradient is to be approximated. */
/* 	fcn	     --> name of subroutine to evaluate function. */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in fstocd (but can be */
/*			 modified by fcn). */
/* 	sx	     --> diagonal scaling matrix for x. */
/* 	rnoise	     --> relative noise in fcn [f(x)]. */
/* 	g	    <--	 central difference approximation to gradient. */


static void
fstocd(int n, double *x, fcn_p fcn, void *state, double *sx, double
       rnoise, double *g)
{
  int i;
  double stepi, fplus, fminus, xtempi;
  double temp1, temp2;

  /* 	find i th  stepsize, evaluate two neighbors in direction of i th */
  /* 	unit vector, and evaluate i th	component of gradient. */

  for (i = 0; i < n; ++i) {
    xtempi = x[i];
    temp1 = fabs(xtempi);
    temp2 = 1.0/sx[i];
    stepi = pow(rnoise, 1.0/3.0) * max(temp1, temp2);
    x[i] = xtempi + stepi;
    (*fcn)(n, x, &fplus, state);
    x[i] = xtempi - stepi;
    (*fcn)(n, x, &fminus, state);
    x[i] = xtempi;
    g[i] = (fplus - fminus) / (stepi * 2.);
  }
} /* fstocd */

/* 	subroutine sndofd */

/* 	purpose */

/* 	find second order forward finite difference approximation "a" */
/* 	to the second derivative (hessian) of the function defined by the subp */
/* 	"fcn" evaluated at the new iterate "xpls" */

/* 	for optimization use this routine to estimate */
/* 	1) the second derivative (hessian) of the optimization function */
/* 	   if no analytical user function has been supplied for either */
/* 	   the gradient or the hessian and if the optimization function */
/* 	   "fcn" is inexpensive to evaluate. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	xpls(n)	     --> new iterate:	x[k] */
/* 	fcn	     --> name of subroutine to evaluate function */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in sndofd (but can be */
/*			 modified by fcn). */
/* 	fpls	     --> function value at new iterate, f(xpls) */
/* 	a(n,n)	    <--	 finite difference approximation to hessian */
/* 			 only lower triangular matrix and diagonal */
/* 			 are returned */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	rnoise	     --> relative noise in fname [f(x)] */
/* 	stepsz(n)    --> workspace (stepsize in i-th component direction) */
/* 	anbr(n)	     --> workspace (neighbor in i-th direction) */


static void
sndofd(int nr, int n, double *xpls, fcn_p fcn, void *state, double
       fpls, double *a, double *sx, double rnoise, double *stepsz,
       double *anbr)
{
  double fhat;
  int i, j;
  double xtmpi, xtmpj;
  double temp1, temp2;

  /* 	find i-th stepsize and evaluate neighbor in direction */
  /* 	of i-th unit vector. */

  for (i = 0; i < n; ++i) {
    xtmpi = xpls[i];
    temp1 = fabs(xtmpi);
    temp2 = 1.0/sx[i];
    stepsz[i] = pow(rnoise, 1.0/3.0) * max(temp1, temp2);
    xpls[i] = xtmpi + stepsz[i];
    (*fcn)(n, xpls, &anbr[i], state);
    xpls[i] = xtmpi;
  }

  /* 	calculate row i of a */

  for (i = 0; i < n; ++i) {
    xtmpi = xpls[i];
    xpls[i] = xtmpi + stepsz[i] * 2.;
    (*fcn)(n, xpls, &fhat, state);
    a[i + i * nr] = ((fpls - anbr[i]) + (fhat - anbr[i]))/(stepsz[i] *
							   stepsz[i]);

    /* 	calculate sub-diagonal elements of column */
    if(i == 0) {
      xpls[i] = xtmpi;
      continue;
    }
    xpls[i] = xtmpi + stepsz[i];
    for (j = 0; j < i; ++j) {
      xtmpj = xpls[j];
      xpls[j] = xtmpj + stepsz[j];
      (*fcn)(n, xpls, &fhat, state);
      a[i + j*nr] = ((fpls - anbr[i]) + (fhat -
					 anbr[j]))/(stepsz[i]*stepsz[j]);
      xpls[j] = xtmpj;
    }
    xpls[i] = xtmpi;
  }
} /* sndofd */

/* 	subroutine grdchk */

/* 	purpose */

/* 	check analytic gradient against estimated gradient */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	x(n)	     --> estimate to a root of fcn */
/* 	fcn	     --> name of subroutine to evaluate optimization function */
/* 			 must be declared external in calling routine */
/* 			      fcn:  r(n) --> r(1) */
/*      state       <--> information other than x and n that fcn requires. */
/*      	         state is not modified in grdchk (but can be */
/*			 modified by fcn). */
/* 	f	     --> function value:  fcn(x) */
/* 	g(n)	     --> gradient:  g(x) */
/* 	typsiz(n)    --> typical size for each component of x */
/* 	sx(n)	     --> diagonal scaling matrix:  sx(i)=1./typsiz(i) */
/* 	fscale	     --> estimate of scale of objective function fcn */
/* 	rnf	     --> relative noise in optimization function fcn */
/* 	analtl	     --> tolerance for comparison of estimated and */
/* 			 analytical gradients */
/* 	wrk1(n)	     --> workspace */
/* 	msg	    <--	 message or error code */
/* 			   on output: =-21, probable coding error of gradient */

static void
grdchk(int n, double *x, fcn_p fcn, void *state, double f, double *g,
       double *typsiz, double *sx, double fscale, double rnf,
       double analtl, double *wrk1, int *msg)
{
  int i;
  double gs;
  double wrk;
  double temp1, temp2;
  
  /* 	compute first order finite difference gradient and compare to */
  /* 	analytic gradient. */

  fstofd(1, 1, n, x, (fcn_p)fcn, state, &f, wrk1, sx, rnf, &wrk, 1);
  for (i = 0; i < n; ++i) {
    temp1 = fabs(x[i]);
    temp2 = typsiz[i];
    temp1 = fabs(f);
    temp2 = max(temp1, temp2);
    gs = max(temp1, fscale) / temp2;
    temp2 = fabs(g[i]);
    if (fabs(g[i] - wrk1[i]) > max(temp2, gs) * analtl) {
      *msg = -21;
      return;
    }
  }
} /* grdchk */

/* 	subroutine heschk */

/* 	purpose */

/* 	check analytic hessian against estimated hessian */
/* 	 (this may be done only if the user supplied analytic hessian */
/* 	  d2fcn fills only the lower triangular part and diagonal of a) */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> estimate to a root of fcn */
/* 	fcn	     --> name of subroutine to evaluate optimization function */
/* 			 must be declared external in calling routine */
/* 			      fcn:  r(n) --> r(1) */
/* 	d1fcn	     --> name of subroutine to evaluate gradient of fcn. */
/* 			 must be declared external in calling routine */
/* 	d2fcn	     --> name of subroutine to evaluate hessian of fcn. */
/* 			 must be declared external in calling routine */
/*      state       <--> information other than x and n that fcn, */
/*	                 d1fcn and d2fcn requires. */
/*      	         state is not modified in heschk (but can be */
/*			 modified by fcn, d1fcn or d2fcn). */
/* 	f	     --> function value:  fcn(x) */
/* 	g(n)	    <--	 gradient:  g(x) */
/* 	a(n,n)	    <--	 on exit:  hessian in lower triangular part and diag */
/* 	typsiz(n)    --> typical size for each component of x */
/* 	sx(n)	     --> diagonal scaling matrix:  sx(i)=1./typsiz(i) */
/* 	rnf	     --> relative noise in optimization function fcn */
/* 	analtl	     --> tolerance for comparison of estimated and */
/* 			 analytical gradients */
/* 	iagflg	     --> =1 if analytic gradient supplied */
/* 	udiag(n)     --> workspace */
/* 	wrk1(n)	     --> workspace */
/* 	wrk2(n)	     --> workspace */
/* 	msg	    <--> message or error code */
/* 			   on input : if =1xx do not compare anal + est hess */
/* 			   on output: =-22, probable coding error of hessian */

static void
heschk(int nr, int n, double *x, fcn_p fcn, fcn_p d1fcn, d2fcn_p d2fcn,
       void *state, double f, double *g, double *a, double *typsiz,
       double *sx, double rnf, double analtl, int iagflg, double
       *udiag, double *wrk1, double *wrk2, int *msg)
{
  int i, j;
  double hs;
  double temp1, temp2;

  /* compute finite difference approximation a to the hessian. */

  if (iagflg == 1) {
    fstofd(nr, n, n, x, (fcn_p)d1fcn, state, g, a, sx, rnf, wrk1, 3);
  } else {
    sndofd(nr, n, x, (fcn_p)fcn, state, f, a, sx, rnf, wrk1, wrk2);
  }

  /* copy lower triangular part of "a" to upper triangular part */
  /* and diagonal of "a" to udiag */
  for (j = 0; j < n; ++j) {
    udiag[j] = a[j + j * nr];
    for (i = j+1; i < n; ++i) {
      a[j + i * nr] = a[i + j * nr];
    }
  }

  /* compute analytic hessian and compare to finite difference */
  /* approximation. */
  (*d2fcn)(nr, n, x, a, state);
  for (j = 0; j < n; ++j) {
    temp1 = fabs(x[j]);
    temp2 = typsiz[j];
    temp2 = max(temp1, temp2);
    temp1 = fabs(g[j]);
    hs = max(temp1, 1.0) / temp2;
    temp2 = fabs(udiag[j]);
    if (fabs(a[j + j * nr] - udiag[j]) > max(temp2, hs) * analtl) {
      *msg = -22;
      return;
    }
    for (i = j+1; i < n; ++i) {
      temp1 = a[i + j * nr];
      temp2 = fabs(temp1 - a[j + i * nr]);
      temp1 = fabs(temp1);
      if (temp2 > max(temp1, hs) * analtl) {
	*msg = -22;
	return;
      }
    }
  }
} /* heschk */

/* 	subroutine optstp */

/* 	unconstrained minimization stopping criteria */

/* 	find whether the algorithm should terminate, due to any */
/* 	of the following: */
/* 	1) problem solved within user tolerance */
/* 	2) convergence within user tolerance */
/* 	3) iteration limit reached */
/* 	4) divergence or too restrictive maximum step (stepmx) suspected */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	xpls(n)	     --> new iterate x[k] */
/* 	fpls	     --> function value at new iterate f(xpls) */
/* 	gpls(n)	     --> gradient at new iterate, g(xpls), or approximate */
/* 	x(n)	     --> old iterate x[k-1] */
/* 	itncnt	     --> current iteration k */
/* 	icscmx	    <--> number consecutive steps .ge. stepmx */
/* 			 [retain value between successive calls] */
/* 	itrmcd	    <--	 termination code */
/* 	gradtl	     --> tolerance at which relative gradient considered close */
/* 			 enough to zero to terminate algorithm */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	sx(n)	     --> diagonal scaling matrix for x */
/* 	fscale	     --> estimate of scale of objective function */
/* 	itnlim	     --> maximum number of allowable iterations */
/* 	iretcd	     --> return code */
/* 	mxtake	     --> boolean flag indicating step of maximum length used */
/* 	msg	     --> if msg includes a term 8, suppress output */


static void
optstp(int n, double *xpls, double fpls, double *gpls, double *x, int
       itncnt, int *icscmx, int *itrmcd, double gradtl, double steptl,
       double *sx, double fscale, int itnlim, int iretcd, int mxtake,
       int *msg)
{
  double d;
  int i;
  double relgrd;
  int jtrmcd;
  double relstp, rgx, rsx;
  double temp1, temp2;

  *itrmcd = 0;

  /* 	last global step failed to locate a point lower than x */

  if (iretcd == 1) {
    jtrmcd = 3;
  } else {

    /* 	find direction in which relative gradient maximum. */
    /* 	check whether within tolerance */

    temp1 = fabs(fpls);
    d = max(temp1, fscale);
    rgx = 0.;
    for (i = 0; i < n; ++i) {
      temp1 = fabs(xpls[i]);
      temp2 = 1.0/sx[i];
      relgrd = fabs(gpls[i]) * max(temp1, temp2) / d;
      rgx = max(rgx, relgrd);
    }
    jtrmcd = 1;
    if (rgx > gradtl) {
      
      if (itncnt == 0) {
	return;
      }

      /* find direction in which relative stepsize maximum */
      /* check whether within tolerance. */

      rsx = 0.;
      for (i = 0; i < n; ++i) {
	temp1 = fabs(xpls[i]);
	temp2 = 1.0/sx[i];
	relstp = fabs(xpls[i] - x[i]) / max(temp1, temp2);;
	rsx = max(rsx, relstp);
      }
      jtrmcd = 2;
      if (rsx > steptl) {

	/* 	check iteration limit */

	jtrmcd = 4;
	if (itncnt < itnlim) {

	  /* 	check number of consecutive steps \ stepmx */

	  if (!mxtake) {
	    *icscmx = 0;
	    return;
	  } else {
	    ++(*icscmx);
	    if (*icscmx < 5) {
	      return;
	    }
	    jtrmcd = 5;
	  }
	}
      }
    }
  }
  *itrmcd = jtrmcd;
} /* optstp */

/* 	subroutine optchk */

/* 	purpose */

/* 	check input for reasonableness */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	x(n)	     --> on entry, estimate to root of fcn */
/* 	typsiz(n)   <--> typical size of each component of x */
/* 	sx(n)	    <--	 diagonal scaling matrix for x */
/* 	fscale	    <--> estimate of scale of objective function fcn */
/* 	gradtl	     --> tolerance at which gradient considered close */
/* 			 enough to zero to terminate algorithm */
/* 	itnlim	    <--> maximum number of allowable iterations */
/* 	ndigit	    <--> number of good digits in optimization function fcn */
/* 	epsm	     --> machine epsilon */
/* 	dlt	    <--> trust region radius */
/* 	method	    <--> algorithm indicator */
/* 	iexp	    <--> expense flag */
/* 	iagflg	    <--> =1 if analytic gradient supplied */
/* 	iahflg	    <--> =1 if analytic hessian supplied */
/* 	stepmx	    <--> maximum step size */
/* 	msg	    <--> message and error code */
/* 	ipr	     --> device to which to send output */

static void
optchk(int n, double *x, double *typsiz, double *sx, double *fscale,
       double gradtl, int *itnlim, int *ndigit, double epsm, double
       *dlt, int *method, int *iexp, int *iagflg, int *iahflg, double
       *stepmx, int *msg)
{
  int i;
  double stpsiz;


  /* 	check that parameters only take on acceptable values. */
  /* 	if not, set them to default values. */

  if (*method < 1 || *method > 3) {
    *method = 1;
  }
  if (*iagflg != 1) {
    *iagflg = 0;
  }
  if (*iahflg != 1) {
    *iahflg = 0;
  }
  if (*iexp != 0) {
    *iexp = 1;
  }
  if (*msg / 2 % 2 == 1 && *iagflg == 0) {
    goto L830;
  }
  if (*msg / 4 % 2 == 1 && *iahflg == 0) {
    goto L835;
  }

  /* 	check dimension of problem */

  if (n <= 0) {
    goto L805;
  }
  if (n == 1 && *msg % 2 == 0) {
    goto L810;
  }

  /* 	compute scale matrix */

  for (i = 0; i < n; ++i) {
    if (typsiz[i] == 0.) {
      typsiz[i] = 1.;
    }
    if (typsiz[i] < 0.) {
      typsiz[i] = -typsiz[i];
    }
    sx[i] = 1. / typsiz[i];
  }

  /* 	check maximum step size */

  if (*stepmx > 0.) {
    goto L20;
  }
  stpsiz = 0.;
  for (i = 0; i < n; ++i) {
    stpsiz += x[i] * x[i] * sx[i] * sx[i];
  }
  stpsiz = sqrt(stpsiz);
  *stepmx = max(stpsiz, 1) * 1e3;
 L20:
  /* 	check function scale */
  if (*fscale == 0.) {
    *fscale = 1.;
  } else if (*fscale < 0.) {
    *fscale = -(*fscale);
  }

  /* 	check gradient tolerance */
  if (gradtl < 0.) {
    goto L815;
  }

  /* 	check iteration limit */
  if (*itnlim <= 0) {
    goto L820;
  }

  /* 	check number of digits of accuracy in function fcn */
  if (*ndigit == 0) {
    goto L825;
  } else if (*ndigit < 0) {
    *ndigit = (int) (-log10(epsm));
  }

  /* 	check trust region radius */
  if (*dlt <= 0.) {
    *dlt = -1.;
  } else if (*dlt > *stepmx) {
    *dlt = *stepmx;
  }
  return;

/* 	error exits */

/* %805 write(ipr,901) n */
/* %    msg=-1 */
 L805:
  *msg = -1;
  goto L895;
  /* %810 write(ipr,902) */
  /* %    msg=-2 */
 L810:
  *msg = -2;
  goto L895;
  /* %815 write(ipr,903) gradtl */
  /* %    msg=-3 */
 L815:
  *msg = -3;
  goto L895;
  /* %820 write(ipr,904) itnlim */
  /* %    msg=-4 */
 L820:
  *msg = -4;
  goto L895;
  /* %825 write(ipr,905) ndigit */
  /* %    msg=-5 */
 L825:
  *msg = -5;
  goto L895;
  /* %830 write(ipr,906) msg,iagflg */
  /* %    msg=-6 */
 L830:
  *msg = -6;
  goto L895;
  /* %835 write(ipr,907) msg,iahflg */
  /* %    msg=-7 */
 L835:
  *msg = -7;
 L895:
  return;
} /* optchk */

/*
 *  PURPOSE
 *
 *  Print information.
 *
 *  PARAMETERS
 *
 *  nr	   --> row dimension of matrix
 *  n	   --> dimension of problem
 *  x(n)   --> iterate x[k]
 *  f	   --> function value at x[k]
 *  g(n)   --> gradient at x[k]
 *  a(n,n) --> hessian at x[k]
 *  p(n)   --> step taken
 *  itncnt --> iteration number k
 *  iflg   --> flag controlling info to print
 */

static void 
result(int nr, int n, const double x[], double f, const double g[],
       const double *a, const double p[], int itncnt, int iflg)
{
    /* Print iteration number */

    Rprintf("iteration = %d\n", itncnt);

    /* Print step */

    if (iflg != 0) {
        Rprintf("Step:\n");
        printRealVector((double *)p, n, 1);
    }

    /* Print current iterate */

    Rprintf("Parameter:\n");
    printRealVector((double *)x, n, 1);

    /* Print function value */

    Rprintf("Function Value\n");
    printRealVector((double *)&f, 1, 1);

    /* Print gradient */

    Rprintf("Gradient:\n");
    printRealVector((double *)g, n, 1);

#ifdef NEVER
    /* Print Hessian */
    /* We don't do this because the printRealMatrix */
    /* code takes a SEXP rather than a double*. */
    /* We could do something ugly like use fixed */
    /* e format but that would be UGLY */

    if (iflg != 0) {
    }
#endif

    Rprintf("\n");
}

static void
optdrv_end(int nr, int n, double *xpls, double *x, double *gpls,
	   double *g, double *fpls, double f, double *a, double *p,
	   int itncnt, int itrmcd, int *msg,
	   void (*print_result)(int, int, const double *, double,
				const double *, const double *, const
				double *, int, int))
{
  int i;

  /* 	termination */
  /* 	reset xpls,fpls,gpls,  if previous iterate solution */

  if (itrmcd == 3) {
    *fpls = f;
    for (i = 0; i < n; ++i) {
      xpls[i] = x[i];
      gpls[i] = g[i];
    }
  }
  if (*msg / 8 % 2 == 0) {
    (*print_result)(nr, n, xpls, *fpls, gpls, a, p, itncnt, 0);
  }
  *msg = 0;
}

/* 	subroutine optdrv */

/* 	purpose */

/* 	driver for non-linear optimization problem */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> on entry: estimate to a root of fcn */
/* 	fcn	     --> name of subroutine to evaluate optimization function */
/* 			 must be declared external in calling routine */
/* 				   fcn: r(n) --> r(1) */
/* 	d1fcn	     --> (optional) name of subroutine to evaluate gradient */
/* 			 of fcn.  must be declared external in calling routine */
/* 	d2fcn	     --> (optional) name of subroutine to evaluate */
/*	                 hessian of of fcn.  must be declared external */
/*			 in calling routine */
/*      state       <--> information other than x and n that fcn, */
/*	                 d1fcn and d2fcn requires. */
/*      	         state is not modified in optdrv (but can be */
/*			 modified by fcn, d1fcn or d2fcn). */
/* 	typsiz(n)    --> typical size for each component of x */
/* 	fscale	     --> estimate of scale of objective function */
/* 	method	     --> algorithm to use to solve minimization problem */
/* 			   =1 line search */
/* 			   =2 double dogleg */
/* 			   =3 more-hebdon */
/* 	iexp	     --> =1 if optimization function fcn is expensive to */
/* 			 evaluate, =0 otherwise.  if set then hessian will */
/* 			 be evaluated by secant update instead of */
/* 			 analytically or by finite differences */
/* 	msg	    <--> on input:  (.gt.0) message to inhibit certain */
/* 			   automatic checks */
/* 			 on output: (.lt.0) error code; =0 no error */
/* 	ndigit	     --> number of good digits in optimization function fcn */
/* 	itnlim	     --> maximum number of allowable iterations */
/* 	iagflg	     --> =1 if analytic gradient supplied */
/* 	iahflg	     --> =1 if analytic hessian supplied */
/* 	dlt	     --> trust region radius */
/* 	gradtl	     --> tolerance at which gradient considered close */
/* 			 enough to zero to terminate algorithm */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	xpls(n)	    <--> on exit:  xpls is local minimum */
/* 	fpls	    <--> on exit:  function value at solution, xpls */
/* 	gpls(n)	    <--> on exit:  gradient at solution xpls */
/* 	itrmcd	    <--	 termination code */
/* 	a(n,n)	     --> workspace for hessian (or estimate) */
/* 			 and its cholesky decomposition */
/* 	udiag(n)     --> workspace [for diagonal of hessian] */
/* 	g(n)	     --> workspace (for gradient at current iterate) */
/* 	p(n)	     --> workspace for step */
/* 	sx(n)	     --> workspace (for diagonal scaling matrix) */
/* 	wrk0(n)	     --> workspace */
/* 	wrk1(n)	     --> workspace */
/* 	wrk2(n)	     --> workspace */
/* 	wrk3(n)	     --> workspace */
/* 	itncnt	     current iteration, k  {{was `internal'}} */


/* 	internal variables */

/* 	analtl		 tolerance for comparison of estimated and */
/* 			 analytical gradients and hessians */
/* 	epsm		 machine epsilon */
/* 	f		 function value: fcn(x) */
/* 	rnf		 relative noise in optimization function fcn. */
/* 			      noise=10.**(-ndigit) */

static void
optdrv(int nr, int n, double *x, fcn_p fcn, fcn_p d1fcn, d2fcn_p
       d2fcn, void *state, double *typsiz, double fscale, int method,
       int iexp, int *msg, int ndigit, int itnlim, int iagflg, int
       iahflg, double dlt, double gradtl, double stepmx, double
       steptl, double *xpls, double *fpls, double *gpls, int *itrmcd,
       double *a, double *udiag, double *g, double *p, double *sx,
       double *wrk0, double *wrk1, double *wrk2, double *wrk3, int
       *itncnt)
{
  double dltp, epsm, phip0, f;
  int i;
  int iretcd;
  double analtl;
  int icscmx;
  int mxtake;
  double dlpsav = 0.0, phisav = 0.0, dltsav = 0.0;  /* -Wall */
  double amusav = 0.0, phpsav = 0.0;                /* -Wall */
  int noupdt;
  double phi, amu, rnf, wrk;

  for (i = 0; i < n; ++i) {
    p[i] = 0.;
  }
  *itncnt = 0;
  iretcd = -1;
  epsm = d1mach(4);
  optchk(n, x, typsiz, sx, &fscale, gradtl, &itnlim, &ndigit, epsm,
	 &dlt, &method, &iexp, &iagflg, &iahflg, &stepmx, msg);
  if (*msg < 0) {
    return;
  }
  rnf = pow(10.0, -ndigit);
  rnf = max(rnf, epsm);
  analtl = sqrt(rnf);
  analtl = max(0.01, analtl);

  /* 	evaluate fcn(x) */

  (*fcn)(n, x, &f, state);

  /* 	evaluate analytic or finite difference gradient and check analytic */
  /* 	gradient, if requested. */

  if (iagflg == 0) {
    fstofd(1, 1, n, x, (fcn_p)fcn, state, &f, g, sx, rnf, &wrk, 1);
  } else {
    (*d1fcn)(n, x, g, state);
    if (*msg / 2 % 2 == 0) {
      grdchk(n, x, (fcn_p)fcn, state, f, g, typsiz, sx, fscale, rnf, 
	     analtl, wrk1, msg);
      if (*msg < 0) {
	return;
      }
    }
  }
  optstp(n, x, f, g, wrk1, *itncnt, &icscmx, itrmcd, gradtl, 
	 steptl, sx, fscale, itnlim, iretcd, mxtake, msg);
  if (*itrmcd != 0) {
    optdrv_end(nr, n, xpls, x, gpls, g, fpls, f, a, p, *itncnt,
	       3, msg, result);
    return;
  }

  if (iexp == 1) {
    /* 	if optimization function expensive to evaluate (iexp=1), then */
    /* 	hessian will be obtained by secant updates.  get initial hessian. */

    hsnint(nr, n, a, sx, method);
  } else {

    /* 	evaluate analytic or finite difference hessian and check analytic */
    /* 	hessian if requested (only if user-supplied analytic hessian */
    /* 	routine d2fcn fills only lower triangular part and diagonal of a). */

    if (iahflg == 0) {
      if (iagflg == 1) {
	fstofd(nr, n, n, x, (fcn_p)d1fcn, state, g, a, sx,
	       rnf, wrk1, 3);
      }
      if (iagflg != 1) {
	sndofd(nr, n, x, (fcn_p)fcn, state, f, a, sx, rnf, wrk1, wrk2);
      }
    } else {
      if (*msg / 4 % 2 == 1) {
	(*d2fcn)(nr, n, x, a, state);
      } else {
	heschk(nr, n, x, (fcn_p)fcn, (fcn_p)d1fcn, (d2fcn_p)d2fcn,
	       state, f, g, a, typsiz, sx, rnf, analtl, iagflg,
	       udiag, wrk1, wrk2, msg);

	/* 	    heschk evaluates d2fcn and checks it against the finite */
	/* 	    difference hessian which it calculates by calling fstofd */
	/* 	    (if iagflg .eq. 1) or sndofd (otherwise). */

	if (*msg < 0) {
	  return;
	}
      }
    }
  }
  if (*msg / 8 % 2 == 0) {
    result(nr, n, x, f, g, a, p, *itncnt, 1);
  }

  /* 	iteration */

  while(1) {
    ++(*itncnt);

    /* 	find perturbed local model hessian and its ll+ decomposition */
    /* 	(skip this step if line search or dogstep techniques being used with */
    /* 	secant updates.	 cholesky decomposition l already obtained from */
    /* 	secfac.) */

    if (iexp == 1 && method != 3) {
      goto L105;
    }
 L103:
    chlhsn(nr, n, a, epsm, sx, udiag);
 L105:

    /* solve for newton step: ap=-g */

    for (i = 0; i < n; ++i) {
      wrk1[i] = -g[i];
    }
    lltslv(nr, n, a, p, wrk1);

    /* 	decide whether to accept newton step  xpls=x + p */
    /* 	or to choose xpls by a global strategy. */

    if (iagflg == 0 && method != 1) {
      dltsav = dlt;
      if (method != 2) {
	amusav = amu;
	dlpsav = dltp;
	phisav = phi;
	phpsav = phip0;
      }
    }
    switch(method) {
    case 1:
      lnsrch(n, x, f, g, p, xpls, fpls, (fcn_p)fcn, state, &mxtake,
	     &iretcd, stepmx, steptl, sx);
      break;
    case 2:
      dogdrv(nr, n, x, f, g, a, p, xpls, fpls, (fcn_p)fcn, state,
	     sx, stepmx, steptl, &dlt, &iretcd, &mxtake, wrk0, wrk1,
	     wrk2, wrk3, itncnt);
      break;
    case 3:
      hookdr(nr, n, x, f, g, a, udiag, p, xpls, fpls, (fcn_p)fcn,
	     state, sx, stepmx, steptl, &dlt, & iretcd, &mxtake, &amu,
	     &dltp, &phi, &phip0, wrk0, wrk1 , wrk2, epsm, *itncnt);
      break;
    }

    /* 	if could not find satisfactory step and forward difference */
    /* 	gradient was used, retry using central difference gradient. */

    if (iretcd == 1 && iagflg == 0) {
      /* 	 set iagflg for central differences */

      iagflg = -1;
      fstocd(n, x, (fcn_p)fcn, state, sx, rnf, g);
      if (method == 1) {
	goto L105;
      }
      dlt = dltsav;
      if (method == 2) {
	goto L105;
      }
      amu = amusav;
      dltp = dlpsav;
      phi = phisav;
      phip0 = phpsav;
      goto L103;
    }
    /* 	calculate step for output */
    for (i = 0; i < n; ++i) {
      p[i] = xpls[i] - x[i];
    }

    /* 	calculate gradient at xpls */
    switch(iagflg) {
    case -1:
      /* central difference gradient */
      fstocd(n, xpls, (fcn_p)fcn, state, sx, rnf, gpls);
      break;
    case 0:
      /* forward difference gradient */
      fstofd(1, 1, n, xpls, (fcn_p)fcn, state, fpls, gpls, sx, rnf,
	     &wrk, 1);
      break;
    default:
      /* analytic gradient */
      (*d1fcn)(n, xpls, gpls, state);
    }

    /* 	check whether stopping criteria satisfied */
    optstp(n, xpls, *fpls, gpls, x, *itncnt, &icscmx, itrmcd, 
	   gradtl, steptl, sx, fscale, itnlim, iretcd, mxtake,
	   msg);
    if(*itrmcd != 0) break;

    /* 	evaluate hessian at xpls */
    if (iexp != 0) {
      if (method == 3) {
	secunf(nr, n, x, g, a, udiag, xpls, gpls, epsm, *itncnt, rnf,
		iagflg, &noupdt, wrk1, wrk2, wrk3);
      } else {
	secfac(nr, n, x, g, a, xpls, gpls, epsm, *itncnt, rnf, iagflg,
		&noupdt, wrk0, wrk1, wrk2, wrk3);
      }
    } else {
      if (iahflg != 1) {
	if (iagflg == 1) {
	  fstofd(nr, n, n, xpls, (fcn_p)d1fcn, state, gpls, a, sx,
		 rnf, wrk1, 3);
	}
	if (iagflg != 1) {
	  sndofd(nr, n, xpls, (fcn_p)fcn, state, *fpls, a, sx, rnf,
		 wrk1, wrk2);
	}
      } else {
	(*d2fcn)(nr, n, xpls, a, state);
      }
    }
    if (*msg / 16 % 2 == 1) {
      result(nr, n, xpls, *fpls, gpls, a, p, *itncnt, 1);
    }
    
    /* 	x <-- xpls  and	 g <-- gpls  and  f <-- fpls */

    f = *fpls;
    for (i = 0; i < n; ++i) {
      x[i] = xpls[i];
      g[i] = gpls[i];
    }
  }

  optdrv_end(nr, n, xpls, x, gpls, g, fpls, f, a, p, *itncnt,
	     *itrmcd, msg, result);
} /* optdrv */

/* 	subroutine dfault */

/* 	purpose */

/* 	set default values for each input variable to */
/* 	minimization algorithm. */

/* 	parameters */

/* 	n	     --> dimension of problem */
/* 	x(n)	     --> initial guess to solution (to compute max step size) */
/* 	typsiz(n)   <--	 typical size for each component of x */
/* 	fscale	    <--	 estimate of scale of minimization function */
/* 	method	    <--	 algorithm to use to solve minimization problem */
/* 	iexp	    <--	 =0 if minimization function not expensive to evaluate */
/* 	msg	    <--	 message to inhibit certain automatic checks + output */
/* 	ndigit	    <--	 number of good digits in minimization function */
/* 	itnlim	    <--	 maximum number of allowable iterations */
/* 	iagflg	    <--	 =0 if analytic gradient not supplied */
/* 	iahflg	    <--	 =0 if analytic hessian not supplied */
/* 	dlt	    <--	 trust region radius */
/* 	gradtl	    <--	 tolerance at which gradient considered close enough */
/* 			 to zero to terminate algorithm */
/* 	stepmx	    <--	 value of zero to trip default maximum in optchk */
/* 	steptl	    <--	 tolerance at which successive iterates considered */
/* 			 close enough to terminate algorithm */

static void
dfault(int n, double *x, double *typsiz, 
       double *fscale, int *method, int *iexp, int *msg, 
       int *ndigit, int *itnlim, int *iagflg, int *iahflg, 
       double *dlt, double *gradtl, double *stepmx, double *steptl)
{
  double epsm;
  int i;

  /* set typical size of x and minimization function */
  for (i = 0; i < n; ++i) {
    typsiz[i] = 1.;
  }
  *fscale = 1.;

  /* set tolerances */

  *dlt = -1.;
  epsm = d1mach(4);
  *gradtl = pow(epsm, 1.0/3.0);
  *stepmx = 0.;
  *steptl = sqrt(epsm);

  /* set flags */

  *method = 1;
  *iexp = 1;
  *msg = 0;
  *ndigit = -1;
  *itnlim = 150;
  *iagflg = 0;
  *iahflg = 0;
} /* dfault_ */

/* 	subroutine optif0 */

/* 	purpose */

/* 	provide simplest interface to minimization package. */
/* 	user has no control over options. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> initial estimate of minimum */
/* 	fcn	     --> name of routine to evaluate minimization function. */
/* 			 must be declared external in calling routine. */
/*      state       <--> information other than x and n that fcn requires */
/*      	         state is not modified in optif0 (but can be */
/*			 modified by fcn). */
/* 	xpls(n)	    <--	 local minimum */
/* 	fpls	    <--	 function value at local minimum xpls */
/* 	gpls(n)	    <--	 gradient at local minimum xpls */
/* 	itrmcd	    <--	 termination code */
/* 	a(n,n)	     --> workspace */
/* 	wrk(n,9)     --> workspace */

void
optif0(int nr, int n, double *x, fcn_p fcn, void *state,
       double *xpls, double *fpls, double *gpls, int *itrmcd, double
       *a, double *wrk)
{
  int iexp;
  int iagflg, iahflg;
  double fscale, gradtl;
  int ndigit;
  int method, itnlim, itncnt;
  double steptl;
  double stepmx, dlt;
  int msg;

  /* Function Body */
  dfault(n, x, &wrk[nr], &fscale, &method, &iexp, &msg, &ndigit,
	 &itnlim, &iagflg, &iahflg, &dlt, &gradtl, &stepmx, &steptl);
  optdrv(nr, n, x, (fcn_p)fcn, (fcn_p)d1fcn, (d2fcn_p)d2fcn, state,
	 &wrk[nr * 3], fscale, method, iexp, &msg, ndigit,
	 itnlim, iagflg, iahflg, dlt, gradtl, stepmx, steptl,
	 xpls, fpls, gpls, itrmcd, a, wrk, &wrk[nr], &wrk[nr * 2],
	 &wrk[nr * 4], &wrk[nr * 5], &wrk[nr * 6], &wrk[nr * 7],
	 &wrk[nr * 8], &itncnt);
} /* optif0 */

/* ---- this one is called from ../main/optimize.c : --------------- */
/* 	subroutine optif9 */

/* 	purpose */

/* 	provide complete interface to minimization package. */
/* 	user has full control over options. */

/* 	parameters */

/* 	nr	     --> row dimension of matrix */
/* 	n	     --> dimension of problem */
/* 	x(n)	     --> on entry: estimate to a root of fcn */
/* 	fcn	     --> name of subroutine to evaluate optimization function */
/* 			 must be declared external in calling routine */
/* 				   fcn: r(n) --> r(1) */
/* 	d1fcn	     --> (optional) name of subroutine to evaluate gradient */
/* 			 of fcn.  must be declared external in calling routine */
/* 	d2fcn	     --> (optional) name of subroutine to evaluate hessian of */
/* 			 of fcn.  must be declared external in calling routine */
/*      state       <--> information other than x and n that fcn, */
/*	                 d1fcn and d2fcn requires. */
/*      	         state is not modified in optif9 (but can be */
/*			 modified by fcn, d1fcn or d2fcn). */
/* 	typsiz(n)    --> typical size for each component of x */
/* 	fscale	     --> estimate of scale of objective function */
/* 	method	     --> algorithm to use to solve minimization problem */
/* 			   =1 line search */
/* 			   =2 double dogleg */
/* 			   =3 more-hebdon */
/* 	iexp	     --> =1 if optimization function fcn is expensive to */
/* 			 evaluate, =0 otherwise.  if set then hessian will */
/* 			 be evaluated by secant update instead of */
/* 			 analytically or by finite differences */
/* 	msg	    <--> on input:  (.gt.0) message to inhibit certain */
/* 			   automatic checks */
/* 			 on output: (.lt.0) error code; =0 no error */
/* 	ndigit	     --> number of good digits in optimization function fcn */
/* 	itnlim	     --> maximum number of allowable iterations */
/* 	iagflg	     --> =1 if analytic gradient supplied */
/* 	iahflg	     --> =1 if analytic hessian supplied */
/* 	dlt	     --> trust region radius */
/* 	gradtl	     --> tolerance at which gradient considered close */
/* 			 enough to zero to terminate algorithm */
/* 	stepmx	     --> maximum allowable step size */
/* 	steptl	     --> relative step size at which successive iterates */
/* 			 considered close enough to terminate algorithm */
/* 	xpls(n)	    <--> on exit:  xpls is local minimum */
/* 	fpls	    <--> on exit:  function value at solution, xpls */
/* 	gpls(n)	    <--> on exit:  gradient at solution xpls */
/* 	itrmcd	    <--	 termination code */
/* 	a(n,n)	     --> workspace for hessian (or estimate) */
/* 			 and its cholesky decomposition */
/* 	wrk(n,8)     --> workspace */
/* 	itncnt	    <--> iteration count */

void
optif9(int nr, int n, double *x, fcn_p fcn, fcn_p d1fcn, d2fcn_p
       d2fcn, void *state, double *typsiz, double fscale, int method,
       int iexp, int *msg, int ndigit, int itnlim, int iagflg, int iahflg,
       double dlt, double gradtl, double stepmx, double steptl, double
       *xpls, double *fpls, double *gpls, int *itrmcd, double *a,
       double *wrk, int *itncnt)
{
  optdrv(nr, n, x, (fcn_p)fcn, (fcn_p)d1fcn, (d2fcn_p)d2fcn, state,
	 typsiz, fscale, method, iexp, msg, ndigit, itnlim, iagflg,
	 iahflg, dlt, gradtl, stepmx, steptl, xpls, fpls, gpls,
	 itrmcd, a, wrk, wrk + nr, wrk + nr * 2, wrk + nr * 3, wrk +
	 nr * 4, wrk + nr * 5, wrk + nr * 6, wrk + nr * 7, itncnt);
} /* optif9 */
