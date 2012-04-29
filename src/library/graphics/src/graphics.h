/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2012   The R Core Team.
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

#ifdef ENABLE_NLS
#include <libintl.h>
#undef _
#define _(String) dgettext ("grDevices", String)
#else
#define _(String) (String)
#endif

SEXP C_filledcontour(SEXP);
SEXP C_image(SEXP);
SEXP C_persp(SEXP);

SEXP C_abline(SEXP args);
SEXP C_arrows(SEXP args);
SEXP C_axis(SEXP args);
SEXP C_box(SEXP args);
SEXP C_clip(SEXP args);
SEXP C_convertX(SEXP args);
SEXP C_convertY(SEXP args);
SEXP C_dend(SEXP args);
SEXP C_dendwindow(SEXP args);
SEXP C_erase(SEXP args);
SEXP C_mtext(SEXP args);
SEXP C_path(SEXP args);
SEXP C_plotXY(SEXP args);
SEXP C_plot_window(SEXP args);
SEXP C_polygon(SEXP args);
SEXP C_raster(SEXP args);
SEXP C_rect(SEXP args);
SEXP C_segments(SEXP args);
SEXP C_strHeight(SEXP args);
SEXP C_strWidth (SEXP args);
SEXP C_symbols(SEXP args);
SEXP C_text(SEXP args);
SEXP C_title(SEXP args);
SEXP C_xspline(SEXP args);

