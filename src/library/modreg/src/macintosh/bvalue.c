/* bvalue.f -- translated by f2c (version 19971204).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

doublereal bvalue(doublereal *t, integer *lent, doublereal *bcoef, integer *
	n, integer *k, doublereal *x, integer *jderiv)
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal ret_val;

    /* Local variables */
    static doublereal fkmj;
    static integer i__, j, mflag, jcmin, jcmax, jdrvp1;
    static doublereal aj[20];
    static integer jc;
    static doublereal dm[20], dp[20];
    static integer jj;
    extern /* Subroutine */ int interv(doublereal *, integer *, doublereal *,
	     integer *, integer *);
    static integer km1, imk, kmj, ilo, nmi;

/* alls  interv */

/* alculates value at  x	of  jderiv-th derivative of spline from b-repr. */
/*  the spline is taken to be continuous from the right. */

/* ******	 i n p u t ****** */
/*  t, bcoef, n, k......forms the b-representation of the spline	 f  to */
/* 	 be evaluated. specifically, */
/*  t.....knot sequence, of length  n+k, assumed nondecreasing. */
/*  bcoef.....b-coefficient sequence, of length	n . */
/*  n.....length of  bcoef  and dimension of s(k,t), */
/* 	 a s s u m e d	positive . */
/*  k.....order of the spline . */

/*  w a r n i n g . . .	 the restriction  k .le. kmax (=20)  is imposed */
/* 	 arbitrarily by the dimension statement for  aj, dm, dm	 below, */
/* 	 but is	 n o w h e r e	c h e c k e d  for. */

/*  x.....the point at which to evaluate . */
/*  jderiv.....integer giving the order of the derivative to be evaluated */
/* 	 a s s u m e d	to be zero or positive. */

/* ******	 o u t p u t  ****** */
/*  bvalue.....the value of the (jderiv)-th derivative of  f  at	 x . */

/* ******	 m e t h o d  ****** */
/*     the nontrivial knot interval  (t(i),t(i+1))  containing  x  is lo- */
/*  cated with the aid of  interv . the	k  b-coeffs of	f  relevant for */
/*  this interval are then obtained from	 bcoef (or taken to be zero if */
/*  not explicitly available) and are then differenced  jderiv  times to */
/*  obtain the b-coeffs of  (d**jderiv)f	 relevant for that interval. */
/*  precisely, with  j = jderiv, we have from x.(12) of the text that */

/*     (d**j)f  =  sum ( bcoef(.,j)*b(.,k-j,t) ) */

/*  where */
/* 		    / bcoef(.),			    ,  j .eq. 0 */
/* 		    / */
/*    bcoef(.,j)	 =  / bcoef(.,j-1) - bcoef(.-1,j-1) */
/* 		    / ----------------------------- ,  j .gt. 0 */
/* 		    /	 (t(.+k-j) - t(.))/(k-j) */

/*     then, we use repeatedly the fact that */

/*    sum ( a(.)*b(.,m,t)(x) )  =  sum ( a(.,x)*b(.,m-1,t)(x) ) */
/*  with */
/* 		  (x - t(.))*a(.) + (t(.+m-1) - x)*a(.-1) */
/*    a(.,x)  =	  --------------------------------------- */
/* 		  (x - t(.))	  + (t(.+m-1) - x) */

/*  to write  (d**j)f(x)	 eventually as a linear combination of b-splines */
/*  of order  1 , and the coefficient for  b(i,1,t)(x)  must then */
/*  be the desired number  (d**j)f(x). (see x.(17)-(19) of text). */

/* Arguments */
/*     dimension t(n+k) */
/*  current fortran standard makes it impossible to specify the length of */
/*  t  precisely without the introduction of otherwise superfluous */
/*  additional arguments. */
/* Local Variables */
    /* Parameter adjustments */
    --t;
    --bcoef;

    /* Function Body */
    ret_val = 0.f;
    if (*jderiv >= *k) {
	goto L99;
    }

/*  *** find  i	s.t.  1 .le. i .lt. n+k	 and  t(i) .lt. t(i+1) and */
/*      t(i) .le. x .lt. t(i+1) . if no such i can be found,  x	lies */
/*      outside the support of  the spline  f  and  bvalue = 0. */
/*      (the asymmetry in this choice of	 i  makes  f  rightcontinuous) */
    if (*x != t[*n + 1] || t[*n + 1] != t[*n + *k]) {
	goto L700;
    }
    i__ = *n;
    goto L701;
L700:
    i__1 = *n + *k;
    interv(&t[1], &i__1, x, &i__, &mflag);
    if (mflag != 0) {
	goto L99;
    }
L701:
/*  *** if k = 1 (and jderiv = 0), bvalue = bcoef(i). */
    km1 = *k - 1;
    if (km1 > 0) {
	goto L1;
    }
    ret_val = bcoef[i__];
    goto L99;

/*  *** store the k b-spline coefficients relevant for the knot interval */
/*     (t(i),t(i+1)) in aj(1),...,aj(k) and compute dm(j) = x - t(i+1-j), */
/*     dp(j) = t(i+j) - x, j=1,...,k-1 . set any of the aj not obtainable */
/*     from input to zero. set any t.s not obtainable equal to t(1) or */
/*     to t(n+k) appropriately. */
L1:
    jcmin = 1;
    imk = i__ - *k;
    if (imk >= 0) {
	goto L8;
    }
    jcmin = 1 - imk;
    i__1 = i__;
    for (j = 1; j <= i__1; ++j) {
/* L5: */
	dm[j - 1] = *x - t[i__ + 1 - j];
    }
    i__1 = km1;
    for (j = i__; j <= i__1; ++j) {
	aj[*k - j - 1] = 0.f;
/* L6: */
	dm[j - 1] = dm[i__ - 1];
    }
    goto L10;
L8:
    i__1 = km1;
    for (j = 1; j <= i__1; ++j) {
/* L9: */
	dm[j - 1] = *x - t[i__ + 1 - j];
    }

L10:
    jcmax = *k;
    nmi = *n - i__;
    if (nmi >= 0) {
	goto L18;
    }
    jcmax = *k + nmi;
    i__1 = jcmax;
    for (j = 1; j <= i__1; ++j) {
/* L15: */
	dp[j - 1] = t[i__ + j] - *x;
    }
    i__1 = km1;
    for (j = jcmax; j <= i__1; ++j) {
	aj[j] = 0.f;
/* L16: */
	dp[j - 1] = dp[jcmax - 1];
    }
    goto L20;
L18:
    i__1 = km1;
    for (j = 1; j <= i__1; ++j) {
/* the following if() happens; e.g. in   pp <- predict(cars.spl, xx) */
/* - 	 if( (i+j) .gt.lent) write(6,9911) i+j,lent */
/* -  9911	 format(' i+j, lent ',2(i6,1x)) */
	dp[j - 1] = t[i__ + j] - *x;
/* L19: */
    }

L20:
    i__1 = jcmax;
    for (jc = jcmin; jc <= i__1; ++jc) {
/* L21: */
	aj[jc - 1] = bcoef[imk + jc];
    }

/* 		*** difference the coefficients	 jderiv	 times. */
    if (*jderiv == 0) {
	goto L30;
    }
    i__1 = *jderiv;
    for (j = 1; j <= i__1; ++j) {
	kmj = *k - j;
	fkmj = (doublereal) kmj;
	ilo = kmj;
	i__2 = kmj;
	for (jj = 1; jj <= i__2; ++jj) {
	    aj[jj - 1] = (aj[jj] - aj[jj - 1]) / (dm[ilo - 1] + dp[jj - 1]) * 
		    fkmj;
/* L23: */
	    --ilo;
	}
    }

/*  *** compute value at	 x  in (t(i),t(i+1)) of jderiv-th derivative, */
/*     given its relevant b-spline coeffs in aj(1),...,aj(k-jderiv). */
L30:
    if (*jderiv == km1) {
	goto L39;
    }
    jdrvp1 = *jderiv + 1;
    i__2 = km1;
    for (j = jdrvp1; j <= i__2; ++j) {
	kmj = *k - j;
	ilo = kmj;
	i__1 = kmj;
	for (jj = 1; jj <= i__1; ++jj) {
	    aj[jj - 1] = (aj[jj] * dm[ilo - 1] + aj[jj - 1] * dp[jj - 1]) / (
		    dm[ilo - 1] + dp[jj - 1]);
/* L33: */
	    --ilo;
	}
    }
L39:
    ret_val = aj[0];

L99:
    return ret_val;
} /* bvalue_ */

/* Subroutine */ int interv(doublereal *xt, integer *lxt, doublereal *x, 
	integer *left, integer *mflag)
{
    /* Initialized data */

    static integer ilo = 1;

    static integer istep, middle, ihi;

/* omputes  left = max( i ; 1 .le. i .le. lxt  .and.  xt(i) .le. x )  . */

/* ******	 i n p u t  ****** */
/*  xt.....a double sequence, of length	lxt , assumed to be nondecreasing */
/*  lxt.....number of terms in the sequence  xt . */
/*  x.....the point whose location with respect to the sequence	xt  is */
/* 	 to be determined. */

/* ******	 o u t p u t  ****** */
/*  left, mflag.....both integers, whose value is */

/*   1	  -1	  if		   x .lt.  xt(1) */
/*   i	   0	  if   xt(i)  .le. x .lt. xt(i+1) */
/*  lxt	   1	  if  xt(lxt) .le. x */

/* 	 in particular,	 mflag = 0 is the 'usual' case.	 mflag .ne. 0 */
/* 	 indicates that	 x  lies outside the halfopen interval */
/* 	 xt(1) .le. y .lt. xt(lxt) . the asymmetric treatment of the */
/* 	 interval is due to the decision to make all pp functions cont- */
/* 	 inuous from the right. */

/* ******	 m e t h o d  ****** */
/*  the program is designed to be efficient in the common situation that */
/*  it is called repeatedly, with  x  taken from an increasing or decrea- */
/*  sing sequence. this will happen, e.g., when a pp function is to be */
/*  graphed. the first guess for	 left  is therefore taken to be the val- */
/*  ue returned at the previous call and stored in the  l o c a l  varia- */
/*  ble	ilo . a first check ascertains that  ilo .lt. lxt (this is nec- */
/*  essary since the present call may have nothing to do with the previ- */
/*  ous call). then, if	xt(ilo) .le. x .lt. xt(ilo+1), we set  left = */
/*  ilo	and are done after just three comparisons. */
/*     otherwise, we repeatedly double the difference  istep = ihi - ilo */
/*  while also moving  ilo  and	ihi  in the direction of  x , until */
/* 		       xt(ilo) .le. x .lt. xt(ihi) , */
/*  after which we use bisection to get, in addition, ilo+1 = ihi . */
/*  left = ilo  is then returned. */

    /* Parameter adjustments */
    --xt;

    /* Function Body */
/*     save ilo	(a valid fortran statement in the new 1977 standard) */
    ihi = ilo + 1;
    if (ihi < *lxt) {
	goto L20;
    }
    if (*x >= xt[*lxt]) {
	goto L110;
    }
    if (*lxt <= 1) {
	goto L90;
    }
    ilo = *lxt - 1;
    ihi = *lxt;

L20:
    if (*x >= xt[ihi]) {
	goto L40;
    }
    if (*x >= xt[ilo]) {
	goto L100;
    }

/* 	       **** now x .lt. xt(ilo) . decrease  ilo	to capture  x . */
/* L30: */
    istep = 1;
L31:
    ihi = ilo;
    ilo = ihi - istep;
    if (ilo <= 1) {
	goto L35;
    }
    if (*x >= xt[ilo]) {
	goto L50;
    }
    istep <<= 1;
    goto L31;
L35:
    ilo = 1;
    if (*x < xt[1]) {
	goto L90;
    }
    goto L50;
/* 	       **** now x .ge. xt(ihi) . increase  ihi	to capture  x . */
L40:
    istep = 1;
L41:
    ilo = ihi;
    ihi = ilo + istep;
    if (ihi >= *lxt) {
	goto L45;
    }
    if (*x < xt[ihi]) {
	goto L50;
    }
    istep <<= 1;
    goto L41;
L45:
    if (*x >= xt[*lxt]) {
	goto L110;
    }
    ihi = *lxt;

/* 	    **** now xt(ilo) .le. x .lt. xt(ihi) . narrow the interval. */
L50:
    middle = (ilo + ihi) / 2;
    if (middle == ilo) {
	goto L100;
    }
/*     note. it is assumed that middle = ilo in case ihi = ilo+1 . */
    if (*x < xt[middle]) {
	goto L53;
    }
    ilo = middle;
    goto L50;
L53:
    ihi = middle;
    goto L50;
/* **** set output and return. */
L90:
    *mflag = -1;
    *left = 1;
    return 0;
L100:
    *mflag = 0;
    *left = ilo;
    return 0;
L110:
    *mflag = 1;
    *left = *lxt;
    return 0;
} /* interv_ */
