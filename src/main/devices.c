/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2008  Robert Gentleman, Ross Ihaka and the
 *			      R Development Core Team
 *  Copyright (C) 2002--2005  The R Foundation
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


 *  This is an extensive reworking by Paul Murrell of an original
 *  quick hack by Ross Ihaka designed to give a superset of the
 *  functionality in the AT&T Bell Laboratories GRZ library.
 *
 */

/* This should be regarded as part of the graphics engine */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Defn.h>
#include <Graphics.h>
#include <R_ext/GraphicsBase.h> /* registerBase */

/*
 *  DEVICE FUNCTIONS
 *
 *  R allows there to be (up to 64) multiple devices in
 *  existence at the same time.	 Only one device is the
 *  active device and all drawing occurs in this device
 *
 *  Each device has its own set of graphics parameters
 *  so that switching between devices, switches between
 *  their graphical contexts (e.g., if you set the line
 *  width on one device then switch to another device,
 *  don't expect to be using the line width you just set!)
 *
 *  Each device has additional device-specific graphics
 *  parameters which the device driver (i.e., NOT this
 *  generic graphics code) is wholly responsible for
 *  maintaining (including creating and destroying special
 *  resources such as X11 windows).
 *
 *  Each device has a display list which records every
 *  graphical operation since the last Rf_dpptr(dd)->newPage;
 *  this is used to redraw the output on the device
 *  when it is resized and to copy output from one device
 *  to another (this can be disabled, which is the default
 *  for postscript).
 *
 *  NOTE: that graphical operations should only be
 *  recorded in the displayList if they are "guaranteed"
 *  to succeed (to avoid heaps of error messages on a
 *  redraw) which means that the recording should be the
 *  last thing done in a graphical operation (see do_*
 *  in plot.c).
 *
 */

static int R_CurrentDevice = 0;
static int R_NumDevices = 1;
/* 
   R_MaxDevices is defined in Rgraphics.h to be 64.  Slots are
   initiialized to be NULL, and returned to NULL when a device is
   removed.

   Slot 0 is the null device, and slot 63 is keep empty as a sentinel
   for over-allocation: if a driver fails to call
   R_CheckDeviceAvailable and uses this slot the device it allocated
   will be killed.

   'active' means has been successfully opened and is not in the
   process of being closed and destroyed.  We do this to allow for GUI
   callbacks starting to kill a device whilst another is being killed.
 */
static GEDevDesc* R_Devices[R_MaxDevices];
static Rboolean active[R_MaxDevices];

/* a dummy description to point to when there are no active devices */

static GEDevDesc nullDevice;

/* In many cases this is used to mean that the current device is
   the null device, and in others to mean that there is no open device.
   The two condiions are currently the same, as no way is provided to
   select the null device (selectDevice(0) immediately opens a device).

   But watch out if you intend to change the logic of any of this.
*/

/* Used in grid */
int NoDevices(void)
{
    return (R_NumDevices == 1 || R_CurrentDevice == 0);
}


int NumDevices(void)
{
    return R_NumDevices;
}


GEDevDesc* GEcurrentDevice(void)
{
    /* If there are no active devices
     * check the options for a "default device".
     * If there is one, start it up. */
    if (NoDevices()) {
	SEXP defdev = GetOption(install("device"), R_BaseEnv);
	if (isString(defdev) && length(defdev) > 0) {
	    SEXP devName = install(CHAR(STRING_ELT(defdev, 0)));
	    /*  Not clear where this should be evaluated, since
		grDevices need not be in the search path.
		So we look for it first on the global search path.
	    */
	    defdev = findVar(devName, R_GlobalEnv);
	    if(defdev != R_UnboundValue) {
		PROTECT(defdev = lang1(devName));
		eval(defdev, R_GlobalEnv);
		UNPROTECT(1);
	    } else {
		/* Not globally visible: 
		   try grDevices namespace if loaded.
		   The option is unlikely to be set if it is not loaded,
		   as the default setting is in grDevices:::.onLoad.
		*/
		SEXP ns = findVarInFrame(R_NamespaceRegistry, 
					 install("grDevices"));
		if(ns != R_UnboundValue && 
		   findVar(devName, ns) != R_UnboundValue) {
		    PROTECT(defdev = lang1(devName));
		    eval(defdev, ns);
		    UNPROTECT(1);
		} else
		    error(_("no active or default device"));
	    }
	} else if(TYPEOF(defdev) == CLOSXP) {
	    PROTECT(defdev = lang1(defdev));
	    eval(defdev, R_GlobalEnv);
	    UNPROTECT(1);
	} else
	    error(_("no active or default device"));
    }
    return R_Devices[R_CurrentDevice];
}

/* FIXME: remove in due course */
DevDesc *Rf_CurrentDevice(void)
{
    return (DevDesc *) GEcurrentDevice();
}

GEDevDesc* GEGetDevice(int i)
{
    return R_Devices[i];
}

DevDesc* Rf_GetDevice(int i)
{
    return (DevDesc *) R_Devices[i];
}


void R_CheckDeviceAvailable(void)
{
    if (R_NumDevices >= R_MaxDevices - 1)
	error(_("too many open devices"));
}

Rboolean R_CheckDeviceAvailableBool(void)
{
    if (R_NumDevices >= R_MaxDevices - 1) return FALSE;
    else return TRUE;
}

void attribute_hidden InitGraphics(void)
{
    int i;
    SEXP s, t;

    R_Devices[0] = &nullDevice;
    active[0] = TRUE;
    for (i = 1; i < R_MaxDevices; i++) {
	R_Devices[i] = NULL;
	active[i] = FALSE;
    }

    /* init .Device and .Devices */
    PROTECT(s = mkString("null device"));
    gsetVar(install(".Device"), s, R_BaseEnv);
    PROTECT(t = mkString("null device"));
    gsetVar(install(".Devices"), CONS(t, R_NilValue), R_BaseEnv);
    UNPROTECT(2);

    /* Register the base graphics system with the graphics engine
     */
    registerBase();
}


static SEXP getSymbolValue(char *symbolName)
{
    SEXP t;
    t = findVar(install(symbolName), R_BaseEnv);
    return t;
}


int curDevice(void)
{
    return R_CurrentDevice;
}


int nextDevice(int from)
{
    if (R_NumDevices == 1)
	return 0;
    else {
	int i = from;
	int nextDev = 0;
	while ((i < (R_MaxDevices-1)) && (nextDev == 0))
	    if (active[++i]) nextDev = i;
	if (nextDev == 0) {
	    /* start again from 1 */
	    i = 0;
	    while ((i < (R_MaxDevices-1)) && (nextDev == 0))
		if (active[++i]) nextDev = i;
	}
	return nextDev;
    }
}


int prevDevice(int from)
{
    if (R_NumDevices == 1)
	return 0;
    else {
	int i = from;
	int prevDev = 0;
	while ((i > 1) && (prevDev == 0))
	    if (active[--i]) prevDev = i;
	if (prevDev == 0) {
	    /* start again from R_MaxDevices */
	    i = R_MaxDevices;
	    while ((i > 1) && (prevDev == 0))
		if (active[--i]) prevDev = i;
	}
	return prevDev;
    }
}


void GEaddDevice(GEDevDesc *gdd)
{
    int i;
    Rboolean appnd;
    SEXP s, t;
    GEDevDesc *oldd;
    DevDesc *dd = (DevDesc *)gdd; /* temporary */

    PROTECT(s = getSymbolValue(".Devices"));

    if (!NoDevices())  {
	oldd = GEcurrentDevice();
	oldd->dev->deactivate(oldd->dev);
    }

    /* find empty slot for new descriptor */
    i = 1;
    if (CDR(s) == R_NilValue)
	appnd = TRUE;
    else {
	s = CDR(s);
	appnd = FALSE;
    }
    while (R_Devices[i] != NULL) {
	i++;
	if (CDR(s) == R_NilValue)
	    appnd = TRUE;
	else
	    s = CDR(s);
    }
    R_CurrentDevice = i;
    R_NumDevices++;
    R_Devices[i] = gdd;
    active[i] = TRUE;

    GEregisterWithDevice(gdd);
    gdd->dev->activate(gdd->dev);

    /* maintain .Devices (.Device has already been set) */
    PROTECT(t = ScalarString(STRING_ELT(getSymbolValue(".Device"), 0)));
    if (appnd)
	SETCDR(s, CONS(t, R_NilValue));
    else
	SETCAR(s, t);

    UNPROTECT(2);

    copyGPar(Rf_dpptr(dd), Rf_gpptr(dd));
    GReset(dd);

    /* In case a device driver did not call R_CheckDeviceAvailable
       before starting its allocation, we complete the allocation and
       then call killDevice here.  This ensures that the device gets a
       chance to deallocate its resources and the current active
       device is restored to a sane value. */
    if (i == R_MaxDevices - 1) {
        killDevice(i);
        error(_("too many open devices"));
    }
}

/* FIXME; remove in due course */
void Rf_addDevice(DevDesc *dd)
{
    GEaddDevice((GEDevDesc *) dd);
}


/* This should be called if you have a DevDesc or a GEDevDesc
 * and you want to find the corresponding device number
 */

int GEdeviceNumber(GEDevDesc *dd)
{
    int i;
    for (i = 1; i < R_MaxDevices; i++)
	if (R_Devices[i] == dd) return i;
    return 0;
}

/* FIXME when KillDevice is removed */
static int deviceNumber(DevDesc *dd)
{
    int i;
    for (i = 1; i < R_MaxDevices; i++)
	if (R_Devices[i] == (GEDevDesc *) dd) return i;
    return 0;
}

/* This should be called if you have a NewDevDesc
 * and you want to find the corresponding device number
 */
int ndevNumber(NewDevDesc *dd)
{
    int i;
    for (i = 1; i < R_MaxDevices; i++)
	if (R_Devices[i] != NULL && R_Devices[i]->dev == dd)
	    return i;
    return 0;
}


/* Incorrectly declared old version */
int devNumber(DevDesc *dd)
{
    return ndevNumber((NewDevDesc *) dd);
}

int selectDevice(int devNum)
{
    /* Valid to select nullDevice, but that will open a new device.
       See ?dev.set.
     */
    if((devNum >= 0) && (devNum < R_MaxDevices) && 
       (R_Devices[devNum] != NULL) && active[devNum]) 
    {
	GEDevDesc *gdd;

	if (!NoDevices()) {
	    GEDevDesc *oldd = GEcurrentDevice();
	    oldd->dev->deactivate(oldd->dev);
	}

	R_CurrentDevice = devNum;

	/* maintain .Device */
	gsetVar(install(".Device"),
		elt(getSymbolValue(".Devices"), devNum),
		R_BaseEnv);

	gdd = GEcurrentDevice(); /* will start a device if current is null */
	if (!NoDevices()) /* which it always will be */
	    gdd->dev->activate(gdd->dev);
	return devNum;
    }
    else
	return selectDevice(nextDevice(devNum));
}

/* historically the close was in the [kK]illDevices.
   only use findNext= TRUE when shutting R dowm, and .Device[s] are not
   updated.
*/
static
void removeDevice(int devNum, Rboolean findNext)
{
    /* Not vaild to remove nullDevice */
    if((devNum > 0) && (devNum < R_MaxDevices) && 
       (R_Devices[devNum] != NULL) && active[devNum]) 
    {
	int i;
	SEXP s;
	GEDevDesc *g = R_Devices[devNum];

	active[devNum] = FALSE; /* stops it being selected again */
	R_NumDevices--;

	if(findNext) {
	    /* maintain .Devices */
	    PROTECT(s = getSymbolValue(".Devices"));
	    for (i = 0; i < devNum; i++) s = CDR(s);
	    SETCAR(s, mkString(""));
	    UNPROTECT(1);

	    /* determine new current device */
	    if (devNum == R_CurrentDevice) {
		R_CurrentDevice = nextDevice(R_CurrentDevice);
		/* maintain .Device */
		gsetVar(install(".Device"),
			elt(getSymbolValue(".Devices"), R_CurrentDevice),
			R_BaseEnv);

		/* activate new current device */
		if (R_CurrentDevice) {
		    GEDevDesc *gdd = GEcurrentDevice();
		    DevDesc *dd = (DevDesc *) gdd;
		    gdd->dev->activate(gdd->dev);
		    copyGPar(Rf_dpptr(dd), Rf_gpptr(dd));
		    GReset(dd);
		}
	    }
	}
	g->dev->close(g->dev);
	GEdestroyDevDesc(g);
	R_Devices[devNum] = NULL;
    }
}

/* FIXME; remove in due course */
void Rf_KillDevice(DevDesc *dd)
{
    removeDevice(deviceNumber(dd), TRUE);
}

void GEkillDevice(GEDevDesc *gdd)
{
    removeDevice(GEdeviceNumber(gdd), TRUE);
}

void killDevice(int devNum)
{
    removeDevice(devNum, TRUE);
}


/* Used by front-ends via R_CleanUp to shutdown all graphics devices
   at the end of a session. Not the same as graphics.off(), and leaves
   .Devices and .Device in an invalid state. */
void KillAllDevices(void)
{
    /* Avoid lots of activation followed by removal of devices
       while (R_NumDevices > 1) killDevice(R_CurrentDevice);
    */
    int i;
    for(i = R_MaxDevices-1; i > 0; i--) removeDevice(i, FALSE);
    R_CurrentDevice = 0;  /* the null device, for tidyness */

    /* <FIXME> Disable this for now */
    /*
     * Free the font and encoding structures used by
     * PostScript, Xfig, and PDF devices
     */
    /* freeType1Fonts();
       </FIXME>*/

    /* FIXME: There should really be a formal graphics finaliser
     * but this is a good proxy for now.
     */
    GEunregisterSystem(baseRegisterIndex);
}

/* FIXME: remove in due course */
void recordGraphicOperation(SEXP op, SEXP args, DevDesc *dd)
{
    GErecordGraphicOperation(op, args, ((GEDevDesc*) dd));
}

/* FIXME:  NewFrameConfirm should be a standard device function */
#ifdef Win32
Rboolean winNewFrameConfirm(void);
#endif

void NewFrameConfirm(void)
{
    unsigned char buf[16];
#ifdef Win32
	int i;
	Rboolean haveWindowsDevice;
	SEXP dotDevices = findVar(install(".Devices"), R_BaseEnv); /* This is a pairlist! */
#endif

    if(!R_Interactive) return;
#ifdef Win32
    for(i = 0; i < curDevice(); i++)  /* 0-based */
	dotDevices = CDR(dotDevices);
    haveWindowsDevice =
	strcmp(CHAR(STRING_ELT(CAR(dotDevices), 0)), "windows") == 0;
    
    if (!haveWindowsDevice || !winNewFrameConfirm())
#endif
	R_ReadConsole(_("Hit <Return> to see next plot: "), buf, 16, 0);
}

#define checkArity_length					\
    checkArity(op, args);					\
    if(!LENGTH(CAR(args)))					\
	error(_("argument must have positive length"))

SEXP attribute_hidden do_devcontrol(SEXP call, SEXP op, SEXP args, SEXP env)
{
    int listFlag;
    GEDevDesc *gdd = GEcurrentDevice();

    checkArity(op, args);
    if(PRIMVAL(op) == 0) { /* dev.control */
	listFlag = asLogical(CAR(args));
	if(listFlag == NA_LOGICAL) error(_("invalid argument"));
	GEinitDisplayList(gdd);
	gdd->dev->displayListOn = listFlag ? TRUE: FALSE;
    } else { /* dev.displaylist */
	listFlag = gdd->dev->displayListOn;
    }
    return ScalarLogical(listFlag);
}

SEXP attribute_hidden do_devcopy(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity_length;
    GEcopyDisplayList(INTEGER(CAR(args))[0] - 1);
    return R_NilValue;
}

SEXP attribute_hidden do_devcur(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity(op, args);
    return ScalarInteger(curDevice() + 1);
}

SEXP attribute_hidden do_devnext(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity_length;
    return ScalarInteger( nextDevice(INTEGER(CAR(args))[0] - 1) + 1 );
}

SEXP attribute_hidden do_devprev(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity_length;
    return ScalarInteger( prevDevice(INTEGER(CAR(args))[0] - 1) + 1 );
}

SEXP attribute_hidden do_devset(SEXP call, SEXP op, SEXP args, SEXP env)
{
    int devNum = INTEGER(CAR(args))[0] - 1;
    checkArity(op, args);
    return ScalarInteger( selectDevice(devNum) + 1 );
}

SEXP attribute_hidden do_devoff(SEXP call, SEXP op, SEXP args, SEXP env)
{
    checkArity_length;
    killDevice(INTEGER(CAR(args))[0] - 1);
    return R_NilValue;
}

/* A common construction in some graphics devices */
GEDevDesc *desc2GEDesc(NewDevDesc *dd)
{
    int i;
    for (i = 1; i < R_MaxDevices; i++)
	if (R_Devices[i] != NULL && R_Devices[i]->dev == dd)
	    return R_Devices[i];
    /* shouldn't happen ... */
    return R_Devices[0];
}
