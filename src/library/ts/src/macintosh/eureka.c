/* eureka.f -- translated by f2c (version 19971204).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* ----------------------------------------------------------------------- */

/*  R : A Computer Language for Statistical Data Analysis */
/*  Copyright (C) 1977        B.D. Ripley */
/*  Copyright (C) 1999        the R Development Core Team */

/*  This program is free software; you can redistribute it and/or modify */
/*  it under the terms of the GNU General Public License as published by */
/*  the Free Software Foundation; either version 2 of the License, or */
/*  (at your option) any later version. */

/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*  GNU General Public License for more details. */

/*  You should have received a copy of the GNU General Public License */
/*  along with this program; if not, write to the Free Software */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA. */

/* ----------------------------------------------------------------------- */

/* Subroutine */ int eureka(integer *lr, doublereal *r__, doublereal *g, 
	doublereal *f, doublereal *var, doublereal *a)
{
    /* System generated locals */
    integer f_dim1, f_offset, i__1, i__2;

    /* Local variables */
    static doublereal hold, d__;
    static integer i__, j, k, l;
    static doublereal q, v;
    static integer l1, l2;


/*      solves Toeplitz matrix equation toep(r)f=g(1+.) */
/*      by Levinson's algorithm */
/*      a is a workspace of size lr, the number */
/*      of equations */

    /* Parameter adjustments */
    --a;
    --var;
    f_dim1 = *lr;
    f_offset = f_dim1 + 1;
    f -= f_offset;
    --g;
    --r__;

    /* Function Body */
    v = r__[1];
    d__ = r__[2];
    a[1] = 1.f;
    f[f_dim1 + 1] = g[2] / v;
    q = f[f_dim1 + 1] * r__[2];
    var[1] = (1 - f[f_dim1 + 1] * f[f_dim1 + 1]) * r__[1];
    if (*lr == 1) {
	return 0;
    }
    i__1 = *lr;
    for (l = 2; l <= i__1; ++l) {
	a[l] = -d__ / v;
	if (l > 2) {
	    l1 = (l - 2) / 2;
	    l2 = l1 + 1;
	    i__2 = l2;
	    for (j = 2; j <= i__2; ++j) {
		hold = a[j];
		k = l - j + 1;
		a[j] += a[l] * a[k];
/* L10: */
		a[k] += a[l] * hold;
	    }
	    if (l1 << 1 != l - 2) {
		a[l2 + 1] *= a[l] + 1.f;
	    }
	}
	v += a[l] * d__;
	f[l + l * f_dim1] = (g[l + 1] - q) / v;
	i__2 = l - 1;
	for (j = 1; j <= i__2; ++j) {
/* L40: */
	    f[l + j * f_dim1] = f[l - 1 + j * f_dim1] + f[l + l * f_dim1] * a[
		    l - j + 1];
	}
/*  estimate the innovations variance */
	var[l] = var[l - 1] * (1 - f[l + l * f_dim1] * f[l + l * f_dim1]);
	if (l == *lr) {
	    return 0;
	}
	d__ = 0.f;
	q = 0.f;
	i__2 = l;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    k = l - i__ + 2;
	    d__ += a[i__] * r__[k];
/* L50: */
	    q += f[l + i__ * f_dim1] * r__[k];
	}
/* L60: */
    }
    return 0;
} /* eureka_ */
