/* kmns.f -- translated by f2c (version 19971204).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Code in this file based on Applied Statistics algorithms */
/* (C) Royal Statistical Society 1979 */

/* a minimal modification of AS136 to use double precision */
/* all variables are now declared. */
/* B.D. Ripley 1998/06/17 */

/* Subroutine */ int kmns(doublereal *a, integer *m, integer *n, doublereal *
	c__, integer *k, integer *ic1, integer *ic2, integer *nc, doublereal *
	an1, doublereal *an2, integer *ncp, doublereal *d__, integer *itran, 
	integer *live, integer *iter, doublereal *wss, integer *ifault)
{
    /* Initialized data */

    static doublereal big = 1e30;
    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer a_dim1, a_offset, c_dim1, c_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer indx;
    static doublereal temp;
    static integer i__, j, l;
    extern /* Subroutine */ int optra(doublereal *, integer *, integer *, 
	    doublereal *, integer *, integer *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    integer *, integer *), qtran(doublereal *, integer *, integer *, 
	    doublereal *, integer *, integer *, integer *, integer *, 
	    doublereal *, doublereal *, integer *, doublereal *, integer *, 
	    integer *);
    static doublereal aa, da, db, dc;
    static integer ii, ij, il;
    static doublereal dt[2];


/*     ALGORITHM AS 136  APPL. STATIST. (1979) VOL.28, NO.1 */

/*     Divide M points in N-dimensional space into K clusters so that */
/*     the within cluster sum of squares is minimized. */


/*     Define BIG to be a very large positive number */

    /* Parameter adjustments */
    --d__;
    --ic2;
    --ic1;
    a_dim1 = *m;
    a_offset = a_dim1 + 1;
    a -= a_offset;
    --wss;
    --live;
    --itran;
    --ncp;
    --an2;
    --an1;
    --nc;
    c_dim1 = *k;
    c_offset = c_dim1 + 1;
    c__ -= c_offset;

    /* Function Body */

    *ifault = 3;
    if (*k <= 1 || *k >= *m) {
	return 0;
    }
    *ifault = 0;

/*     For each point I, find its two closest centres, IC1(I) and */
/*     IC2(I).     Assign it to IC1(I). */

    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ic1[i__] = 1;
	ic2[i__] = 2;
	for (il = 1; il <= 2; ++il) {
	    dt[il - 1] = zero;
	    i__2 = *n;
	    for (j = 1; j <= i__2; ++j) {
		da = a[i__ + j * a_dim1] - c__[il + j * c_dim1];
		dt[il - 1] += da * da;
/* L10: */
	    }
	}
	if (dt[0] > dt[1]) {
	    ic1[i__] = 2;
	    ic2[i__] = 1;
	    temp = dt[0];
	    dt[0] = dt[1];
	    dt[1] = temp;
	}
	i__2 = *k;
	for (l = 3; l <= i__2; ++l) {
	    db = zero;
	    i__3 = *n;
	    for (j = 1; j <= i__3; ++j) {
		dc = a[i__ + j * a_dim1] - c__[l + j * c_dim1];
		db += dc * dc;
		if (db >= dt[1]) {
		    goto L50;
		}
/* L30: */
	    }
	    if (db < dt[0]) {
		goto L40;
	    }
	    dt[1] = db;
	    ic2[i__] = l;
	    goto L50;
L40:
	    dt[1] = dt[0];
	    ic2[i__] = ic1[i__];
	    dt[0] = db;
	    ic1[i__] = l;
L50:
	    ;
	}
    }

/*     Update cluster centres to be the average of points contained */
/*     within them. */

    i__2 = *k;
    for (l = 1; l <= i__2; ++l) {
	nc[l] = 0;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
/* L60: */
	    c__[l + j * c_dim1] = zero;
	}
/* L70: */
    }
    i__2 = *m;
    for (i__ = 1; i__ <= i__2; ++i__) {
	l = ic1[i__];
	++nc[l];
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
/* L80: */
	    c__[l + j * c_dim1] += a[i__ + j * a_dim1];
	}
/* L90: */
    }

/*     Check to see if there is any empty cluster at this stage */

    i__2 = *k;
    for (l = 1; l <= i__2; ++l) {
	if (nc[l] == 0) {
	    *ifault = 1;
	    return 0;
	}
	aa = (doublereal) nc[l];
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
/* L110: */
	    c__[l + j * c_dim1] /= aa;
	}

/*     Initialize AN1, AN2, ITRAN & NCP */
/*     AN1(L) = NC(L) / (NC(L) - 1) */
/*     AN2(L) = NC(L) / (NC(L) + 1) */
/*     ITRAN(L) = 1 if cluster L is updated in the quick-transfer stage, */
/*              = 0 otherwise */
/*     In the optimal-transfer stage, NCP(L) stores the step at which */
/*     cluster L is last updated. */
/*     In the quick-transfer stage, NCP(L) stores the step at which */
/*     cluster L is last updated plus M. */

	an2[l] = aa / (aa + one);
	an1[l] = big;
	if (aa > one) {
	    an1[l] = aa / (aa - one);
	}
	itran[l] = 1;
	ncp[l] = -1;
/* L120: */
    }
    indx = 0;
    i__2 = *iter;
    for (ij = 1; ij <= i__2; ++ij) {

/*     In this stage, there is only one pass through the data.   Each */
/*     point is re-allocated, if necessary, to the cluster that will */
/*     induce the maximum reduction in within-cluster sum of squares. */

	optra(&a[a_offset], m, n, &c__[c_offset], k, &ic1[1], &ic2[1], &nc[1]
		, &an1[1], &an2[1], &ncp[1], &d__[1], &itran[1], &live[1], &
		indx);

/*     Stop if no transfer took place in the last M optimal transfer */
/*     steps. */

	if (indx == *m) {
	    goto L150;
	}

/*     Each point is tested in turn to see if it should be re-allocated */
/*     to the cluster to which it is most likely to be transferred, */
/*     IC2(I), from its present cluster, IC1(I).   Loop through the */
/*     data until no further change is to take place. */

	qtran(&a[a_offset], m, n, &c__[c_offset], k, &ic1[1], &ic2[1], &nc[1]
		, &an1[1], &an2[1], &ncp[1], &d__[1], &itran[1], &indx);

/*     If there are only two clusters, there is no need to re-enter the */
/*     optimal transfer stage. */

	if (*k == 2) {
	    goto L150;
	}

/*     NCP has to be set to 0 before entering OPTRA. */

	i__1 = *k;
	for (l = 1; l <= i__1; ++l) {
/* L130: */
	    ncp[l] = 0;
	}
/* L140: */
    }

/*     Since the specified number of iterations has been exceeded, set */
/*     IFAULT = 2.   This may indicate unforeseen looping. */

    *ifault = 2;

/*     Compute within-cluster sum of squares for each cluster. */

L150:
    i__2 = *k;
    for (l = 1; l <= i__2; ++l) {
	wss[l] = zero;
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    c__[l + j * c_dim1] = zero;
/* L160: */
	}
    }
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ii = ic1[i__];
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    c__[ii + j * c_dim1] += a[i__ + j * a_dim1];
/* L170: */
	}
    }
    i__2 = *n;
    for (j = 1; j <= i__2; ++j) {
	i__1 = *k;
	for (l = 1; l <= i__1; ++l) {
/* L180: */
	    c__[l + j * c_dim1] /= (real) nc[l];
	}
	i__1 = *m;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    ii = ic1[i__];
	    da = a[i__ + j * a_dim1] - c__[ii + j * c_dim1];
	    wss[ii] += da * da;
/* L190: */
	}
    }

    return 0;
} /* kmns_ */



/* Subroutine */ int optra(doublereal *a, integer *m, integer *n, doublereal 
	*c__, integer *k, integer *ic1, integer *ic2, integer *nc, doublereal 
	*an1, doublereal *an2, integer *ncp, doublereal *d__, integer *itran, 
	integer *live, integer *indx)
{
    /* Initialized data */

    static doublereal big = 1e30;
    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer a_dim1, a_offset, c_dim1, c_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer i__, j, l, l1, l2;
    static doublereal r2, da, db, dc, dd, de, df;
    static integer ll;
    static doublereal rr, al1, al2, alt, alw;


/*     ALGORITHM AS 136.1  APPL. STATIST. (1979) VOL.28, NO.1 */

/*     This is the optimal transfer stage. */

/*     Each point is re-allocated, if necessary, to the cluster that */
/*     will induce a maximum reduction in the within-cluster sum of */
/*     squares. */


/*     Define BIG to be a very large positive number. */

    /* Parameter adjustments */
    --d__;
    --ic2;
    --ic1;
    a_dim1 = *m;
    a_offset = a_dim1 + 1;
    a -= a_offset;
    --live;
    --itran;
    --ncp;
    --an2;
    --an1;
    --nc;
    c_dim1 = *k;
    c_offset = c_dim1 + 1;
    c__ -= c_offset;

    /* Function Body */

/*     If cluster L is updated in the last quick-transfer stage, it */
/*     belongs to the live set throughout this stage.   Otherwise, at */
/*     each step, it is not in the live set if it has not been updated */
/*     in the last M optimal transfer steps. */

    i__1 = *k;
    for (l = 1; l <= i__1; ++l) {
	if (itran[l] == 1) {
	    live[l] = *m + 1;
	}
/* L10: */
    }
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	++(*indx);
	l1 = ic1[i__];
	l2 = ic2[i__];
	ll = l2;

/*     If point I is the only member of cluster L1, no transfer. */

	if (nc[l1] == 1) {
	    goto L90;
	}

/*     If L1 has not yet been updated in this stage, no need to */
/*     re-compute D(I). */

	if (ncp[l1] == 0) {
	    goto L30;
	}
	de = zero;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    df = a[i__ + j * a_dim1] - c__[l1 + j * c_dim1];
	    de += df * df;
/* L20: */
	}
	d__[i__] = de * an1[l1];

/*     Find the cluster with minimum R2. */

L30:
	da = zero;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    db = a[i__ + j * a_dim1] - c__[l2 + j * c_dim1];
	    da += db * db;
/* L40: */
	}
	r2 = da * an2[l2];
	i__2 = *k;
	for (l = 1; l <= i__2; ++l) {

/*     If I >= LIVE(L1), then L1 is not in the live set.   If this is */
/*     true, we only need to consider clusters that are in the live set */
/*     for possible transfer of point I.   Otherwise, we need to consider */
/*     all possible clusters. */

	    if (i__ >= live[l1] && i__ >= live[l] || l == l1 || l == ll) {
		goto L60;
	    }
	    rr = r2 / an2[l];
	    dc = zero;
	    i__3 = *n;
	    for (j = 1; j <= i__3; ++j) {
		dd = a[i__ + j * a_dim1] - c__[l + j * c_dim1];
		dc += dd * dd;
		if (dc >= rr) {
		    goto L60;
		}
/* L50: */
	    }
	    r2 = dc * an2[l];
	    l2 = l;
L60:
	    ;
	}
	if (r2 < d__[i__]) {
	    goto L70;
	}

/*     If no transfer is necessary, L2 is the new IC2(I). */

	ic2[i__] = l2;
	goto L90;

/*     Update cluster centres, LIVE, NCP, AN1 & AN2 for clusters L1 and */
/*     L2, and update IC1(I) & IC2(I). */

L70:
	*indx = 0;
	live[l1] = *m + i__;
	live[l2] = *m + i__;
	ncp[l1] = i__;
	ncp[l2] = i__;
	al1 = (doublereal) nc[l1];
	alw = al1 - one;
	al2 = (doublereal) nc[l2];
	alt = al2 + one;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    c__[l1 + j * c_dim1] = (c__[l1 + j * c_dim1] * al1 - a[i__ + j * 
		    a_dim1]) / alw;
	    c__[l2 + j * c_dim1] = (c__[l2 + j * c_dim1] * al2 + a[i__ + j * 
		    a_dim1]) / alt;
/* L80: */
	}
	--nc[l1];
	++nc[l2];
	an2[l1] = alw / al1;
	an1[l1] = big;
	if (alw > one) {
	    an1[l1] = alw / (alw - one);
	}
	an1[l2] = alt / al2;
	an2[l2] = alt / (alt + one);
	ic1[i__] = l2;
	ic2[i__] = l1;
L90:
	if (*indx == *m) {
	    return 0;
	}
/* L100: */
    }
    i__1 = *k;
    for (l = 1; l <= i__1; ++l) {

/*     ITRAN(L) = 0 before entering QTRAN.   Also, LIVE(L) has to be */
/*     decreased by M before re-entering OPTRA. */

	itran[l] = 0;
	live[l] -= *m;
/* L110: */
    }

    return 0;
} /* optra_ */



/* Subroutine */ int qtran(doublereal *a, integer *m, integer *n, doublereal 
	*c__, integer *k, integer *ic1, integer *ic2, integer *nc, doublereal 
	*an1, doublereal *an2, integer *ncp, doublereal *d__, integer *itran, 
	integer *indx)
{
    /* Initialized data */

    static doublereal big = 1e30;
    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer a_dim1, a_offset, c_dim1, c_offset, i__1, i__2;

    /* Local variables */
    static integer i__, j, icoun, istep, l1, l2;
    static doublereal r2, da, db, dd, de, al1, al2, alt, alw;


/*     ALGORITHM AS 136.2  APPL. STATIST. (1979) VOL.28, NO.1 */

/*     This is the quick transfer stage. */
/*     IC1(I) is the cluster which point I belongs to. */
/*     IC2(I) is the cluster which point I is most likely to be */
/*         transferred to. */
/*     For each point I, IC1(I) & IC2(I) are switched, if necessary, to */
/*     reduce within-cluster sum of squares.  The cluster centres are */
/*     updated after each step. */


/*     Define BIG to be a very large positive number */

    /* Parameter adjustments */
    --d__;
    --ic2;
    --ic1;
    a_dim1 = *m;
    a_offset = a_dim1 + 1;
    a -= a_offset;
    --itran;
    --ncp;
    --an2;
    --an1;
    --nc;
    c_dim1 = *k;
    c_offset = c_dim1 + 1;
    c__ -= c_offset;

    /* Function Body */

/*     In the optimal transfer stage, NCP(L) indicates the step at which */
/*     cluster L is last updated.   In the quick transfer stage, NCP(L) */
/*     is equal to the step at which cluster L is last updated plus M. */

    icoun = 0;
    istep = 0;
L10:
    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	++icoun;
	++istep;
	l1 = ic1[i__];
	l2 = ic2[i__];

/*     If point I is the only member of cluster L1, no transfer. */

	if (nc[l1] == 1) {
	    goto L60;
	}

/*     If ISTEP > NCP(L1), no need to re-compute distance from point I to */
/*     cluster L1.   Note that if cluster L1 is last updated exactly M */
/*     steps ago, we still need to compute the distance from point I to */
/*     cluster L1. */

	if (istep > ncp[l1]) {
	    goto L30;
	}
	da = zero;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    db = a[i__ + j * a_dim1] - c__[l1 + j * c_dim1];
	    da += db * db;
/* L20: */
	}
	d__[i__] = da * an1[l1];

/*     If ISTEP >= both NCP(L1) & NCP(L2) there will be no transfer of */
/*     point I at this step. */

L30:
	if (istep >= ncp[l1] && istep >= ncp[l2]) {
	    goto L60;
	}
	r2 = d__[i__] / an2[l2];
	dd = zero;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    de = a[i__ + j * a_dim1] - c__[l2 + j * c_dim1];
	    dd += de * de;
	    if (dd >= r2) {
		goto L60;
	    }
/* L40: */
	}

/*     Update cluster centres, NCP, NC, ITRAN, AN1 & AN2 for clusters */
/*     L1 & L2.   Also update IC1(I) & IC2(I).   Note that if any */
/*     updating occurs in this stage, INDX is set back to 0. */

	icoun = 0;
	*indx = 0;
	itran[l1] = 1;
	itran[l2] = 1;
	ncp[l1] = istep + *m;
	ncp[l2] = istep + *m;
	al1 = (doublereal) nc[l1];
	alw = al1 - one;
	al2 = (doublereal) nc[l2];
	alt = al2 + one;
	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    c__[l1 + j * c_dim1] = (c__[l1 + j * c_dim1] * al1 - a[i__ + j * 
		    a_dim1]) / alw;
	    c__[l2 + j * c_dim1] = (c__[l2 + j * c_dim1] * al2 + a[i__ + j * 
		    a_dim1]) / alt;
/* L50: */
	}
	--nc[l1];
	++nc[l2];
	an2[l1] = alw / al1;
	an1[l1] = big;
	if (alw > one) {
	    an1[l1] = alw / (alw - one);
	}
	an1[l2] = alt / al2;
	an2[l2] = alt / (alt + one);
	ic1[i__] = l2;
	ic2[i__] = l1;

/*     If no re-allocation took place in the last M steps, return. */

L60:
	if (icoun == *m) {
	    return 0;
	}
/* L70: */
    }
    goto L10;
} /* qtran_ */
