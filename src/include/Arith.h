/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ARITH_H_
#define ARITH_H_

#include "Platform.h"

/* Maybe get finite(.): */
#ifdef HAVE_IEEE754_H
#include <ieee754.h>		/* newer Linuxen */
#else
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>		/* others [Solaris 2.5.x], .. */
#endif
#endif

#include <math.h>
#ifdef Macintosh
# define finite(x) isfinite(x)
#else
# ifndef HAVE_FINITE
#  ifndef finite		/* Do not declare if macro! */
#   ifdef isfinite		/* HPUX math.h */
#     define finite(x) isfinite(x)
#   else
      int finite(double);
#   endif
#  endif
# endif
#endif


#ifdef __MAIN__
#define extern
#endif
extern double	R_tmp;		/*  Temporary Value used in NaN/Inf checks */
extern double	R_NaN;		/* IEEE NaN or -DBL_MAX */
extern double	R_PosInf;	/* IEEE Inf or DBL_MAX */
extern double	R_NegInf;	/* IEEE -Inf or -DBL_MAX */
extern int	R_NaInt;	/* NA_INTEGER etc */
extern double	R_NaReal;	/* NA_REAL */
#ifdef __MAIN__
#undef extern
#endif

#define NA_LOGICAL	R_NaInt
#define NA_INTEGER	R_NaInt
#define NA_FACTOR	R_NaInt
#define NA_REAL		R_NaReal
#define NA_STRING	R_NaString

#ifdef IEEE_754

int R_IsNA(double);		/* True for Real NA only */
int R_IsNaN(double);		/* True for special NaN, *not* for NA */

#define MATH_CHECK(call)	(call)
#define FINITE(x)		finite(x)
#define ISNAN(x)		(isnan(x)!=0)  /* -> True, *both* for NA | NaN */
						/* NOTE: some systems do
						 * not return 1 for
						 * TRUE!*/
#define ISNA(x)			R_IsNA(x) /* from ../main/arithmetic.c */

#else

#define MATH_CHECK(call)	(errno=0,R_tmp=call,(errno==0)?R_tmp:R_NaN)

#ifndef HAVE_FINITE
#define FINITE(x)               ((x)!= R_NaReal)
#else
#define FINITE(x)		finite(x)
#endif

#ifndef HAVE_ISNAN
#define ISNAN(x)                ((x)==R_NaReal)
#else
#define ISNAN(x)                (isnan(x) || (x)==R_NaReal)
#endif

#define ISNA(x)                 ((x)==R_NaReal)
/* never used. in HP-UX' c89 "NAN" is for a double const. :
 * #define NAN(x)			ISNAN(x)
 */
#endif

#endif
