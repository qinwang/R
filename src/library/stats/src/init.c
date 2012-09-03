/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001-12   The R Core Team.
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
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

#include <R.h>
#include <Rinternals.h>

#include "ctest.h"
#include "family.h"
#include "modreg.h"
#include "mva.h"
#include "nls.h"
#include "port.h"
#include "stats.h"
#include "statsR.h"
#include "ts.h"
#include <R_ext/Rdynload.h>
#include <R_ext/Visibility.h>

static R_NativePrimitiveArgType chisqsim_t[11] = {INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
					   INTSXP, REALSXP, INTSXP, REALSXP, INTSXP, REALSXP};
static R_NativePrimitiveArgType fishersim_t[10] = {INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
					   INTSXP, INTSXP, REALSXP, INTSXP, REALSXP};
static R_NativePrimitiveArgType d2_t[5] = {INTSXP, REALSXP, REALSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType dansari_t[4] = {INTSXP, REALSXP, INTSXP, INTSXP};
static R_NativePrimitiveArgType pansari_t[4] = {INTSXP, REALSXP, INTSXP, INTSXP};
static R_NativePrimitiveArgType qansari_t[4] = {INTSXP, REALSXP, INTSXP, INTSXP};

static R_NativePrimitiveArgType fexact_t[11] = {INTSXP, INTSXP, INTSXP, INTSXP, REALSXP,
					 REALSXP, REALSXP, REALSXP, REALSXP, INTSXP, INTSXP};

static R_NativePrimitiveArgType pkendall_t[3] = {INTSXP, REALSXP, INTSXP};

static R_NativePrimitiveArgType prho_t[5] = {INTSXP, REALSXP, REALSXP, INTSXP, LGLSXP};

static R_NativePrimitiveArgType swilk_t[6] = {REALSXP, INTSXP, INTSXP,
					      REALSXP, REALSXP, INTSXP};

static R_NativePrimitiveArgType band_ucv_bin_t[] = {INTSXP, INTSXP, REALSXP, INTSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType band_bcv_bin_t[] = {INTSXP, INTSXP, REALSXP, INTSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType band_phi4_bin_t[] = {INTSXP, INTSXP, REALSXP, INTSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType band_phi6_bin_t[] = {INTSXP, INTSXP, REALSXP, INTSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType band_den_bin_t[] = {INTSXP, INTSXP, REALSXP, REALSXP, INTSXP};

static R_NativePrimitiveArgType R_approx_t[] = {REALSXP, REALSXP, INTSXP, REALSXP, INTSXP, INTSXP, REALSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType R_approxtest_t[] = {REALSXP, REALSXP, INTSXP, INTSXP, REALSXP};
static R_NativePrimitiveArgType R_approxfun_t[] = {REALSXP, REALSXP, INTSXP, REALSXP, INTSXP, INTSXP, REALSXP, REALSXP, REALSXP};

static R_NativePrimitiveArgType loglin_t[] = {INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
					      REALSXP, REALSXP, INTSXP, INTSXP, REALSXP,
					      INTSXP, REALSXP, REALSXP, INTSXP, REALSXP,
					      INTSXP, INTSXP};

static R_NativePrimitiveArgType spline_coef_t[] = {INTSXP, INTSXP, REALSXP, REALSXP, REALSXP, REALSXP, REALSXP, REALSXP};
static R_NativePrimitiveArgType spline_eval_t[] = {INTSXP, INTSXP, REALSXP, REALSXP,
						   INTSXP, REALSXP, REALSXP, REALSXP, REALSXP, REALSXP};

static R_NativePrimitiveArgType lowesw_t[] = { REALSXP, INTSXP, REALSXP, INTSXP};
static R_NativePrimitiveArgType lowesp_t[] = {
    INTSXP, REALSXP, REALSXP, REALSXP, REALSXP, INTSXP, REALSXP};


#define CDEF(name)  {#name, (DL_FUNC) &name, sizeof(name ## _t)/sizeof(name ## _t[0]), name ##_t}

#define FDEF(name)  {#name, (DL_FUNC) &F77_SUB(name), sizeof(name ## _t)/sizeof(name ## _t[0]), name ##_t}

static const R_CMethodDef CEntries[]  = {
    {"chisqsim", (DL_FUNC) &chisqsim, 11, chisqsim_t},
    {"fisher_sim", (DL_FUNC) &fisher_sim, 10, fishersim_t},
    {"d2x2xk", (DL_FUNC) &d2x2xk, 5, d2_t},
    {"dansari", (DL_FUNC) &dansari, 4, dansari_t},
    {"fexact",   (DL_FUNC) &fexact, 11, fexact_t},
    {"pansari",  (DL_FUNC)&pansari, 4, pansari_t},
    {"pkendall", (DL_FUNC)  &pkendall, 3, pkendall_t},
    {"prho", (DL_FUNC) &prho, 5, prho_t},
    {"qansari",  (DL_FUNC) &qansari, 4, qansari_t},
    {"swilk2", (DL_FUNC) &swilk, 6, swilk_t},
    {"BDRksmooth", (DL_FUNC) &BDRksmooth, 8},
    {"loess_raw", (DL_FUNC) &loess_raw, 24},
    {"loess_dfit", (DL_FUNC) &loess_dfit, 13},
    {"loess_dfitse", (DL_FUNC) &loess_dfitse, 16},
    {"loess_ifit", (DL_FUNC) &loess_ifit, 8},
    {"loess_ise", (DL_FUNC) &loess_ise, 15},
    {"R_distance", (DL_FUNC) &R_distance, 7},
    {"acf", (DL_FUNC) &acf, 6},
    {"uni_pacf", (DL_FUNC) &uni_pacf, 3},
    {"artoma", (DL_FUNC) &artoma, 4},
    {"burg", (DL_FUNC) &burg, 6},
    {"multi_burg", (DL_FUNC) &multi_burg, 11},
    {"multi_yw", (DL_FUNC) &multi_yw, 10},
    {"R_intgrt_vec", (DL_FUNC) &R_intgrt_vec, 4},
    {"R_pp_sum", (DL_FUNC) &R_pp_sum, 4},
    {"HoltWinters", (DL_FUNC) &HoltWinters, 17},
    {"kmeans_Lloyd", (DL_FUNC) &kmeans_Lloyd, 9},
    {"kmeans_MacQueen", (DL_FUNC) &kmeans_MacQueen, 9},
    CDEF(R_approx),
    CDEF(R_approxfun),
    CDEF(R_approxtest),
    CDEF(band_ucv_bin),
    CDEF(band_bcv_bin),
    CDEF(band_phi4_bin),
    CDEF(band_phi6_bin),
    CDEF(band_den_bin),
    CDEF(loglin),
    CDEF(spline_coef),
    CDEF(spline_eval),
    {"signrank_free", (DL_FUNC) &stats_signrank_free, 0},
    {"wilcox_free", (DL_FUNC) &stats_wilcox_free, 0},
    {NULL, NULL, 0}
};

#define CALLDEF(name, n)  {#name, (DL_FUNC) &name, n}

static const R_CallMethodDef CallEntries[] = {
    {"R_cutree", (DL_FUNC) &R_cutree, 2},
    {"R_isoreg", (DL_FUNC) &R_isoreg, 1},
    {"R_monoFC_m", (DL_FUNC) &R_monoFC_m, 2},
    {"numeric_deriv", (DL_FUNC)&numeric_deriv, 4},
    {"nls_iter", (DL_FUNC)&nls_iter, 3},
    {"setup_starma", (DL_FUNC) &setup_starma, 8},
    {"free_starma", (DL_FUNC) &free_starma, 1},
    {"set_trans", (DL_FUNC) &set_trans, 2},
    {"arma0fa", (DL_FUNC) &arma0fa, 2},
    {"get_s2", (DL_FUNC) &get_s2, 1},
    {"get_resid", (DL_FUNC) &get_resid, 1},
    {"Dotrans", (DL_FUNC) &Dotrans, 2},
    {"arma0_kfore", (DL_FUNC) &arma0_kfore, 4},
    {"Starma_method", (DL_FUNC) &Starma_method, 2},
    {"Invtrans", (DL_FUNC) &Invtrans, 2},
    {"Gradtrans", (DL_FUNC) &Gradtrans, 2},
    {"ARMAtoMA", (DL_FUNC) &ARMAtoMA, 3},
    {"KalmanLike", (DL_FUNC) &KalmanLike, 11},
    {"KalmanFore", (DL_FUNC) &KalmanFore, 8},
    {"KalmanSmooth", (DL_FUNC) &KalmanSmooth, 9},
    {"ARIMA_undoPars", (DL_FUNC) &ARIMA_undoPars, 2},
    {"ARIMA_transPars", (DL_FUNC) &ARIMA_transPars, 3},
    {"ARIMA_Invtrans", (DL_FUNC) &ARIMA_Invtrans, 2},
    {"ARIMA_Gradtrans", (DL_FUNC) &ARIMA_Gradtrans, 2},
    {"ARIMA_Like", (DL_FUNC) &ARIMA_Like, 9},
    {"ARIMA_CSS", (DL_FUNC) &ARIMA_CSS, 6},
    {"TSconv", (DL_FUNC) &TSconv, 2},
    {"getQ0", (DL_FUNC) &getQ0, 2},
    {"port_ivset", (DL_FUNC) &port_ivset, 3},
    {"port_nlminb", (DL_FUNC) &port_nlminb, 9},
    {"port_nlsb", (DL_FUNC) &port_nlsb, 7},
    {"logit_link", (DL_FUNC) &logit_link, 1},
    {"logit_linkinv", (DL_FUNC) &logit_linkinv, 1},
    {"logit_mu_eta", (DL_FUNC) &logit_mu_eta, 1},
    {"binomial_dev_resids", (DL_FUNC) &binomial_dev_resids, 3},
    {"R_rWishart", (DL_FUNC) &R_rWishart, 3},
    {"Cdqrls", (DL_FUNC) &Cdqrls, 3},
    {"Cdist", (DL_FUNC) &Cdist, 4},
    {"cor", (DL_FUNC) &cor, 4},
    {"cov", (DL_FUNC) &cov, 4},
    {"updateform", (DL_FUNC) &updateform, 2},
    {"fft", (DL_FUNC) &fft, 2},
    {"mvfft", (DL_FUNC) &mvfft, 2},
    {"nextn", (DL_FUNC) &nextn, 2},
    {"r2dtable", (DL_FUNC) &r2dtable, 3},
    CALLDEF(cfilter, 4),
    CALLDEF(rfilter, 3),
    CALLDEF(lowess, 5),
    CALLDEF(DoubleCentre, 1),
    CALLDEF(BinDist, 5),
    CALLDEF(Rsm, 3),
    CALLDEF(tukeyline, 3),
    CALLDEF(runmed, 5),
    CALLDEF(influence, 4),
    CALLDEF(pSmirnov2x, 3),
    CALLDEF(pKolmogorov2x, 2),
    CALLDEF(pKS2, 2),
    {NULL, NULL, 0}
};

static const R_FortranMethodDef FortEntries[] = {
    FDEF(lowesw),
    FDEF(lowesp),
    {"setppr", (DL_FUNC) &F77_SUB(setppr), 6},
    {"smart", (DL_FUNC) &F77_SUB(smart), 16},
    {"pppred", (DL_FUNC) &F77_SUB(pppred), 5},
    {"setsmu", (DL_FUNC) &F77_SUB(setsmu), 0},
    {"rbart", (DL_FUNC) &F77_SUB(rbart), 20},
    {"bvalus", (DL_FUNC) &F77_SUB(bvalus), 7},
    {"supsmu", (DL_FUNC) &F77_SUB(supsmu), 10},
    {"hclust", (DL_FUNC) &F77_SUB(hclust), 11},
    {"hcass2", (DL_FUNC) &F77_SUB(hcass2), 6},
    {"kmns", (DL_FUNC) &F77_SUB(kmns), 17},
    {"eureka", (DL_FUNC) &F77_SUB(eureka), 6},
    {"stl", (DL_FUNC) &F77_SUB(stl), 18},
//    {"lminfl", (DL_FUNC) &F77_SUB(lminfl), 11},
    {NULL, NULL, 0}
};

#define EXTDEF(name, n)  {#name, (DL_FUNC) &name, n}

static const R_ExternalMethodDef ExtEntries[] = {
    EXTDEF(compcases, -1),
    EXTDEF(doD, 2),
    EXTDEF(deriv, 5),
    EXTDEF(modelframe, 9),
    EXTDEF(modelmatrix, 3),
    EXTDEF(termsform, 5),
    EXTDEF(do_fmin, 5),
    EXTDEF(nlm, 11),
    EXTDEF(zeroin2, 8),
    EXTDEF(optim, 5),
    EXTDEF(optimhess, 4),
    EXTDEF(call_dqags, 7),
    EXTDEF(call_dqagi, 7),

    // 1-arg distributions
    {"dchisq", (DL_FUNC) &distn2, 3},
    {"pchisq", (DL_FUNC) &distn2, 3},
    {"qchisq", (DL_FUNC) &distn2, 3},
    {"rchisq", (DL_FUNC) &Random1, 1},
    {"dexp", (DL_FUNC) &distn2, 3},
    {"pexp", (DL_FUNC) &distn2, 3},
    {"qexp", (DL_FUNC) &distn2, 3},
    {"rexp", (DL_FUNC) &Random1, 1},
    {"dgeom", (DL_FUNC) &distn2, 3},
    {"pgeom", (DL_FUNC) &distn2, 3},
    {"qgeom", (DL_FUNC) &distn2, 3},
    {"rgeom", (DL_FUNC) &Random1, 1},
    {"dpois", (DL_FUNC) &distn2, 3},
    {"ppois", (DL_FUNC) &distn2, 3},
    {"qpois", (DL_FUNC) &distn2, 3},
    {"rpois", (DL_FUNC) &Random1, 1},
    {"dt", (DL_FUNC) &distn2, 3},
    {"pt", (DL_FUNC) &distn2, 3},
    {"qt", (DL_FUNC) &distn2, 3},
    {"rt", (DL_FUNC) &Random1, 1},
    {"dsignrank", (DL_FUNC) &distn2, 3},
    {"psignrank", (DL_FUNC) &distn2, 3},
    {"qsignrank", (DL_FUNC) &distn2, 3},
    {"rsignrank", (DL_FUNC) &Random1, 1},

    // 2-arg distributions
    {"dbeta", (DL_FUNC) &distn3, 4},
    {"pbeta", (DL_FUNC) &distn3, 4},
    {"qbeta", (DL_FUNC) &distn3, 4},
    {"rbeta", (DL_FUNC) &Random2, 2},
    {"dbinom", (DL_FUNC) &distn3, 4},
    {"pbinom", (DL_FUNC) &distn3, 4},
    {"qbinom", (DL_FUNC) &distn3, 4},
    {"rbinom", (DL_FUNC) &Random2, 2},
    {"dcauchy", (DL_FUNC) &distn3, 4},
    {"pcauchy", (DL_FUNC) &distn3, 4},
    {"qcauchy", (DL_FUNC) &distn3, 4},
    {"rcauchy", (DL_FUNC) &Random2, 2},
    {"df", (DL_FUNC) &distn3, 4},
    {"pf", (DL_FUNC) &distn3, 4},
    {"qf", (DL_FUNC) &distn3, 4},
    {"rf", (DL_FUNC) &Random2, 2},
    {"dgamma", (DL_FUNC) &distn3, 4},
    {"pgamma", (DL_FUNC) &distn3, 4},
    {"qgamma", (DL_FUNC) &distn3, 4},
    {"rgamma", (DL_FUNC) &Random2, 2},
    {"dlnorm", (DL_FUNC) &distn3, 4},
    {"plnorm", (DL_FUNC) &distn3, 4},
    {"qlnorm", (DL_FUNC) &distn3, 4},
    {"rlnorm", (DL_FUNC) &Random2, 2},
    {"dlogis", (DL_FUNC) &distn3, 4},
    {"plogis", (DL_FUNC) &distn3, 4},
    {"qlogis", (DL_FUNC) &distn3, 4},
    {"rlogis", (DL_FUNC) &Random2, 2},
    {"dnbinom", (DL_FUNC) &distn3, 4},
    {"pnbinom", (DL_FUNC) &distn3, 4},
    {"qnbinom", (DL_FUNC) &distn3, 4},
    {"rnbinom", (DL_FUNC) &Random2, 2},
    {"dnbinom_mu", (DL_FUNC) &distn3, 4},
    {"pnbinom_mu", (DL_FUNC) &distn3, 4},
    {"qnbinom_mu", (DL_FUNC) &distn3, 4},  // exists but currently unused
    {"rnbinom_mu", (DL_FUNC) &Random2, 2},
    {"dnchisq", (DL_FUNC) &distn3, 4},
    {"pnchisq", (DL_FUNC) &distn3, 4},
    {"qnchisq", (DL_FUNC) &distn3, 4},
    {"rnchisq", (DL_FUNC) &Random2, 2},
    {"dnorm", (DL_FUNC) &distn3, 4},
    {"pnorm", (DL_FUNC) &distn3, 4},
    {"qnorm", (DL_FUNC) &distn3, 4},
    {"rnorm", (DL_FUNC) &Random2, 2},
    {"dnt", (DL_FUNC) &distn3, 4},
    {"pnt", (DL_FUNC) &distn3, 4},
    {"qnt", (DL_FUNC) &distn3, 4},
    {"ptukey", (DL_FUNC) &distn4, 5},
    {"qtukey", (DL_FUNC) &distn4, 5},
    {"dunif", (DL_FUNC) &distn3, 4},
    {"punif", (DL_FUNC) &distn3, 4},
    {"qunif", (DL_FUNC) &distn3, 4},
    {"runif", (DL_FUNC) &Random2, 2},
    {"dweibull", (DL_FUNC) &distn3, 4},
    {"pweibull", (DL_FUNC) &distn3, 4},
    {"qweibull", (DL_FUNC) &distn3, 4},
    {"rweibull", (DL_FUNC) &Random2, 2},
    {"dwilcox", (DL_FUNC) &distn3, 4},
    {"pwilcox", (DL_FUNC) &distn3, 4},
    {"qwilcox", (DL_FUNC) &distn3, 4},
    {"rwilcox", (DL_FUNC) &Random2, 2},

    // 3-arg distributions
    {"dhyper", (DL_FUNC) &distn4, 5},
    {"phyper", (DL_FUNC) &distn4, 5},
    {"qhyper", (DL_FUNC) &distn4, 5},
    {"rhyper", (DL_FUNC) &Random3, 3},
    {"dnbeta", (DL_FUNC) &distn4, 5},
    {"pnbeta", (DL_FUNC) &distn4, 5},
    {"qnbeta", (DL_FUNC) &distn4, 5},
    {"dnf", (DL_FUNC) &distn4, 5},
    {"pnf", (DL_FUNC) &distn4, 5},
    {"qnf", (DL_FUNC) &distn4, 5},

    {"rmultinom", (DL_FUNC) &Rmultinom, 3},
    {NULL, NULL, 0}
};


void attribute_visible R_init_stats(DllInfo *dll)
{
    R_registerRoutines(dll, CEntries, CallEntries, FortEntries, ExtEntries);
    R_useDynamicSymbols(dll, FALSE);
    R_forceSymbols(dll, TRUE);

    R_RegisterCCallable("stats", "nlminb_iterate", (DL_FUNC) nlminb_iterate);
    R_RegisterCCallable("stats", "nlsb_iterate", (DL_FUNC) nlsb_iterate);
    R_RegisterCCallable("stats", "Rf_divset", (DL_FUNC) Rf_divset);
}
