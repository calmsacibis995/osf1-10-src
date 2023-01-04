/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/* $XConsortium: pegInit.c,v 1.6 91/02/22 21:53:38 keith Exp $ */
/***********************************************************
Portions modified by Tektronix, Inc.  Copyright 1987 Tektronix, Inc.

Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#include "X.h"
#include "Xproto.h"
#include "screenint.h"
#include "input.h"
#include "cursor.h"
#include "miscstruct.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "resource.h"
#include "colormapst.h"
#include "dixstruct.h"
#include "mistruct.h"
#include "peg.h"
#ifdef NOTDEF
#include "drawable.h"
#include "pfb.h"
#endif

#ifdef	UTEK
#include "box/keyboard.h"
#endif	/* UTEK */

#ifdef	UTEKV
#include "redwing/keyboard.h"
#endif	/* UTEKV */

#include <sys/time.h>
#include <sys/resource.h>

#ifdef	UTEK
#include <sys/mman.h>
#endif	/* UTEK */

#ifdef	UTEKV
#include <sys/ipc.h>
#endif	/* UTEKV */

#include <sys/stat.h>
#include <sys/file.h>

#ifdef	XDEBUG
#include <sys/signal.h>
#endif	/*XDEBUG */

#undef NULL			/* misc.h defines this, but so does stdio.h */
#include <stdio.h>

#ifdef	UTEKV
#include <sys/fcntl.h>
#endif	/* UTEKV */

#ifdef	UTEKV


#define DEBUG_ERRNO_IS_SET 1
#define	DEBUG_SHMGET 1		/* add debug -shmget- & -shmat- messages */
#define	DEBUG_VISUALSTUFF 1	/* add debug visual type stuff messages */

 extern int	shmget();	/* memory mapper */
 extern caddr_t shmat();	/* memory attacher */

#ifndef	NBPC
#define	NBPC	(4096)	/* number of bytes per click (page) */
#endif	/* NBPC */

#ifndef	NCPS
#define	NCPS	(1024)	/* number of clicks (pages) per segment */
#endif	/* NCPS */

#ifndef	NBPS
#define	NBPS	(NBPC*NCPS)	/* number of bytes per segment */
#endif	/* NBPS */

/*
 * Redwing display board type #2 "handy" definitions::
 * (derived from "dislpay.h")
 */
#define	OFFSET_VIDEO_REG	(DS_VIDEO_CNTL_P - DS_DSP_BASE)
#define	OFFSET_CDP_0_REG	(DS_CDP_0_P - DS_DSP_BASE)
#define	OFFSET_STENCIL_FB	(DS_FB_STENCIL_P - DS_DSP_BASE)
#define	OFFSET_TWO_COLOR_FB	(DS_FB_TWO_COLOR_P - DS_DSP_BASE)
#define	OFFSET_PPM_FB		(DS_FB_PP_P - DS_DSP_BASE)
#define	DS_NUMBER_OF_BYTES	(DS_FB_PP_HI_P+1-DS_DSP_BASE)
#define	DS_NUMBER_OF_CLICKS	((DS_NUMBER_OF_BYTES+(NBPC-1)) / NBPC)
#define	DS_NUMBER_OF_SEGMENTS	((DS_NUMBER_OF_CLICKS+(NCPS-1)) / NCPS)

/* 
 * "shmget/shmat" flags:: Physical + Cache Inbibit + No Clear.
 */
#define PHYS_FLAGS 	(IPC_PHYS | IPC_CI | IPC_NOCLEAR)
#define PHYS_HIGH_ADDR	(0x60000000)  /* an area between u and display board */

#endif	/* UTEKV */

#define NUMSCREENS 1

static int FMonochrome = FALSE;

/*
 * PixmapFormatRec is depth, bitsPerPixel, scanlinePad
 */

static PixmapFormatRec	OneBitFormats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    0
};

static PixmapFormatRec	EightBitFormats[] = {
    1, 1, BITMAP_SCANLINE_PAD,	/* 1-bit deep */
    8, 8, BITMAP_SCANLINE_PAD,	/* 8-bit deep */
    0
};

#define	ROOTVISUAL	(0)
#define	ArrayLength(list)	((sizeof list)/(sizeof list[0]))

#define	TOGGLE		1
#define	NUMBER		2
#define	NUMBER_LIST	3

typedef struct _Control {
	char	*envVar;	/* environment variable name to examine */
	char	*fileVar;	/* control file variable to examine */
	char	*defVal;	/* default value string */
	union uval {		/* holds assignable addresses */
		Bool	toggle;	/* for flavor = TOGGLE */
		long	*nList;	/* for flavor = NUMBER_LIST */
		long	number;	/* for flavor = NUMBER */
	} *u;
	long	*count;		/* addr of count for NUMBER_LIST */
	int	flavor;		/* type of control variable. */
	int	allocated;	/* u->nList is from Xalloc */
} Control;

static Control	CList[] = {
    {	"JOYPAN",
	".joydisk.pan",
	"OFF",
	(union uval *) &pegInfo.kv.panEnabled,
	(long *) 0,
	TOGGLE,0  },
    {	"XPANSTOP",
	".joydisk.stop",
	"ON",
	(union uval *) &pegInfo.kv.panStop,
	(long *) 0,
	TOGGLE,0  },
    {	"XPANINERTIA",
	".joydisk.inertia",
	"1",
	(union uval *) &pegInfo.kv.panInertia,
	(long *) 0,
	NUMBER,0  },
    {	"XPANDELAYS",
	".joydisk.delays",
	"128 64 48 48 32 32 16 1 1 1 1 1 1 1 1 1",	/* milliseconds */
	(union uval *) &pegInfo.kv.panDelays,
	&pegInfo.kv.nPanDelays,
	NUMBER_LIST,0  },
    {	"XPANDELTAX",
	".joydisk.xdeltas",
	"1 1 2 2 3 3 4 4 5 5 6 6 7 7 8 8",
	(union uval *) &pegInfo.kv.panDeltaX,
	&pegInfo.kv.nPanDeltaX,
	NUMBER_LIST,0  },
    {	"XPANDELTAY",
	".joydisk.ydeltas",
	"1 1 2 2 3 3 4 4 5 5 6 6 7 7 8 8",
	(union uval *) &pegInfo.kv.panDeltaY,
	&pegInfo.kv.nPanDeltaY,
	NUMBER_LIST,0  },
    {	"XKEYDELAYS",
	".keyrepeat",
	"500 96 96 96 96 80 80 80 64 64 48 48 32 32", /* milliseconds */
	(union uval *) &pegInfo.kv.keyDelays,
	&pegInfo.kv.nKeyDelays,
	NUMBER_LIST,0  },
    { NULL }
};

InitInfo	pegInfo;
extern errno;


/*
 *	NAME
 *		AbortDDX - Device dependent cleanup
 *
 *	SYNOPSIS
 */
void
AbortDDX()
/*
 *	DESCRIPTION
 *		Routine required for dix/ddx interface.  Called by
 *		FatalError().  We don't need to do anything here
 *		because closing /dev/xdev does it all.
 *
 *	RETURNS
 *		None
 *
 */
{
}

/*
 *	NAME
 *		ddxGiveUp - Device dependent cleanup
 *
 *	SYNOPSIS
 */
void
ddxGiveUp()
/*
 *	DESCRIPTION
 *		Routine required for dix/ddx interface.  Called by
 *		by dix before normal server death.
 *		We don't need to do anything here
 *		because closing /dev/xdev does it all.
 *
 *	RETURNS
 *		None
 *
 */
{
}

/*
 *	NAME
 *		ddxProcessArgument - Process device-dependent command line args
 *
 *	SYNOPSIS
 */
int
ddxProcessArgument (argc, argv, i)
    /*ARGSUSED*/
    int argc;
    char *argv[];	/* in: commandline */
    int i;		/* in: current index into argv */
/*
 *	DESCRIPTION
 *		Process command line.	Part of dix/ddx interface.
 *
 *	RETURNS
 *		0 if argument is not device dependent, otherwise
 *		Count of number of elements of argv that are part of a 
 *		device dependent commandline option.
 *
 */
{
#ifdef XDEBUG
    if ( strncmp( argv[i], "-debug=", 7) == 0) {
	sscanf(argv[ i ] + 7, "%x", &xflg_debug); 
	return 1;
    }
#endif /* XDEBUG */
    if ( strcmp( argv[i], "-mono") == 0) {
	FMonochrome = TRUE;
	return 1;
    }
#ifdef NOTDEF
    if ( strcmp( argv[i], "-g") == 0) {
	FGammaCorrectionDisabled = FALSE;
	return 1;
    }
#endif

    return 0;
}

/*
 *	NAME
 *		ddxUseMsg - print use of device dependent commandline options
 *
 *	SYNOPSIS
 */
void
ddxUseMsg()
/*
 *	DESCRIPTION
 *		Print out correct use of device dependent commandline options.
 *		Part of dix/ddx interface.  Called by dix UseMsg routine.
 *
 *	RETURNS
 *		None
 *
 */
{
#ifdef XDEBUG
    ErrorF("-debug=hex mask        set debug bits; messages in /tmp/xdebug\n");
    ErrorF("-mono                  use monochrome frame buffer\n");
#endif /* XDEBUG */
    ErrorF("-g                     enable gamma correction\n");
}

/*
 *	This function checks to see if the line L conforms to the syntax for
 *	control option and value, and if so, puts these parsed out strings
 *	in the string space referenced by O & V.
 *	Returns success or failure (1 or 0).
 */
static Bool
ParseOptionAndValue(L, O, V)
    char *L;	/* in: line of text */
    char *O;	/* in/out: pointer to Option */
    char *V;	/* in/out: pointer to Value */
{
    char *ptr;
    char *colon;
    char *point;
    /*
     *	The assumed form is
     *			[white].option1[.option2][white]:[white]<value>
     *	where <value> is a series of one or more fields separated by white
     *
     *	Do some cheap checking for a colon and period
     *	and their relative placement
     */
    if ((colon = index(L, ':')) == NULL)
	return(False);
    if ((point = index(L, '.')) == NULL)
	return(False);
    if (point > colon)
	return(False);

    /*
     *	Parse out the option
     */
    L += strspn(L, " \t");	/* first cut off leading white */ 
    if ((ptr = strpbrk(L, " \t:")) == NULL) /* grab all up to separator */
	return(0);		/* there at least better be a colon */
    *ptr++ = '\0';		/* put null at first white or colon */
    (void) strcpy(O, L);	/* passes so far, so copy it out */
    /*
     *	Parse out the value
     */
    strcpy(V, ptr + strspn(ptr, " \t:")); /* copy up to white and colon */
    return(1);
}

/*
 *	This function fills an array of longs with V if all of V
 *	is good, otherwise the array argument is returned unchanged.
 *	The array is freed if it is changed.
 */
static void
ConvertValues(param, value)
    Control *param;	/* in/out: the address of the control parameter */
    char    *value;	/* in: the value string composed of fields */
{
    /*
     *	Rules:	If bad data anywhere, reject the whole value string
     *		and dont change	array.
     *		Bad data is any field not converting to a positive 
     *		integer
     */
    long *tmpArray;
    char *fieldPtr, *tmpValue;
    int len = 0;

    /*
     * We need to preserve the default value for possible server
     * resets.  Therefore we copy the default string value to
     * a local copy before calling strtok() which will alter the
     * string contents.
     */
    tmpValue = (char *) Xalloc((strlen(value) + 1));
    strcpy(tmpValue, value);
    fieldPtr = strtok(tmpValue, " \t\n");

    if (param->flavor == TOGGLE || param->flavor == NUMBER) {
	char *nextFieldPtr = strtok(0, " \t\n");

	if (nextFieldPtr == NULL) {/* nothing else trailing this */

	    if (param->flavor == TOGGLE) {
		if ((strcmp(fieldPtr, "ON") == 0)
		    || (strcmp(fieldPtr, "on") == 0))
		    param->u->toggle = True;
		else if ((strcmp(fieldPtr, "OFF") == 0)
		    || (strcmp(fieldPtr, "off") == 0))
		    param->u->toggle = False;
	    }
	    else if (param->flavor == NUMBER) {
		int number = atol(fieldPtr);
		if (number != 0)
		    param->u->number = number;
	    }
	}

    } else { /* must be a NUMBER_LIST */
	assert (param->flavor == NUMBER_LIST);
	tmpArray = (long *)Xalloc((len+1) * sizeof(long));
	while (fieldPtr != NULL) {
	    tmpArray = (long *)Xrealloc(tmpArray, (len+2) * sizeof(long));
	    tmpArray[ len ] = atol(fieldPtr);

	    /* if we've already parsed some numbers, error */
	    if (tmpArray[ len ] == 0) {
		Xfree( (char *) tmpArray);
		Xfree( tmpValue);
		return;
	    }
	    fieldPtr = strtok(0, " \t\n");
	    len++;
	}
	if (param->u->nList && param->allocated)
	    Xfree( (char *) param->u->nList );
	tmpArray[ len ] = 0;
	param->u->nList = tmpArray;
	param->allocated = TRUE;
	*param->count = len;
    }

    Xfree( tmpValue);
    return;
}

static ColormapPtr pegInstalledMap;

extern int TellLostMap(), TellGainedMap();

static void
pegStoreColors (pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    SvcColorDef	svcmap[256];
    register int i;

    if (pmap != pegInstalledMap)
	return;
    for (i = 0; i < ndef; i++)
    {
	svcmap[i].pixel = pdefs->pixel;
	svcmap[i].red = pdefs->red;
	svcmap[i].green = pdefs->green;
	svcmap[i].blue = pdefs->blue;
	pdefs++;
    }
    SetColorMap (ndef, svcmap);
}

static void
pegInstallColormap (cmap)
    ColormapPtr	cmap;
{
    register int i;
    register Entry *pent;
    register VisualPtr pVisual = cmap->pVisual;
    SvcColorDef	svcmap[256];

    if (cmap == pegInstalledMap)
	return;
    if (pegInstalledMap)
	WalkTree(pegInstalledMap->pScreen, TellLostMap,
		 (pointer) &(pegInstalledMap->mid));
    if ((pVisual->class | DynamicClass) == DirectColor) {
	for (i = 0; i < 256; i++) {
	    svcmap[i].pixel = i;
	    pent = &cmap->red[(i & pVisual->redMask) >>
			      pVisual->offsetRed];
	    svcmap[i].red = pent->co.local.red;
	    pent = &cmap->green[(i & pVisual->greenMask) >>
				pVisual->offsetGreen];
	    svcmap[i].green = pent->co.local.green;
	    pent = &cmap->blue[(i & pVisual->blueMask) >>
			       pVisual->offsetBlue];
	    svcmap[i].blue = pent->co.local.blue;
	}
    } else {
	for (i = 0, pent = cmap->red;
	     i < pVisual->ColormapEntries;
	     i++, pent++) {
	    svcmap[i].pixel = i;
	    if (pent->fShared) {
		svcmap[i].red = pent->co.shco.red->color;
		svcmap[i].green = pent->co.shco.green->color;
		svcmap[i].blue = pent->co.shco.blue->color;
	    }
	    else {
		svcmap[i].red = pent->co.local.red;
		svcmap[i].green = pent->co.local.green;
		svcmap[i].blue = pent->co.local.blue;
	    }
	}
    }
    pegInstalledMap = cmap;
    SetColorMap (256, svcmap);
    WalkTree(cmap->pScreen, TellGainedMap, (pointer) &(cmap->mid));
}

static void
pegUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    if (cmap == pegInstalledMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID,
							      RT_COLORMAP);

	    if (defMap)
		(*cmap->pScreen->InstallColormap)(defMap);
	    else
	        ErrorF("peg: Can't find default colormap\n");
	}
    }
}

pegListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    *pCmapList = pegInstalledMap->mid;
    return (1);
}

/*
 * This routine will eventually receive a signal, reinitializing the bell
 * strings.  This way, bells could be configured by an external program
 * and then signal X when it is ready.
 */
static void
pegInitBells(sig)
	int	sig;
{
	char	bellname[ BUFSIZ ], *string;
	static Bool	initialized = FALSE;
	struct stat	st;
	int	fd, i, red;

	if (sig || ! initialized) {
		initialized = TRUE;
		pegInfo.bells.fd = -1;
		for (i=0; i<8; i++) {
			sprintf(bellname, BELLNAME, i);
#ifdef	UTEK
			fd = open(bellname, O_RDONLY);
#endif	/* UTEK */
#ifdef	UTEKV
			fd = open(bellname, O_RDONLY);
#endif	/* UTEKV */
			if (fd < 0 || fstat(fd, &st) < 0) {
				if (fd >= 0)
					close(fd);
				Error(bellname);
				continue;
			}
			string = (char *)Xalloc(st.st_size);
			if (string == NULL) {
				close(fd);
				ErrorF("can't alloc space for %s\n", bellname);
				continue;
			}
			red = read(fd, string, st.st_size);
			close(fd);
			if (red != st.st_size) {
				ErrorF("can't read from %s", bellname);
				Error("");
				continue;
			}
#ifdef NOTDEF
			if (pegInfo.bells.str[ i ])
				Xfree (pegInfo.bells.str[ i ]);
#endif
			pegInfo.bells.len[ i ] = st.st_size;
			pegInfo.bells.str[ i ] = string;
		}

		if (pegInfo.bells.fd > 0)
			close(pegInfo.bells.fd); /* just in case this helps */
#ifdef	UTEK
		pegInfo.bells.fd = open("/dev/bell", O_WRONLY);
#endif	/* UTEK */
#ifdef	UTEKV
		pegInfo.bells.fd = open("/dev/bell", O_WRONLY);
#endif	/* UTEKV */
		if (pegInfo.bells.fd < 0)
			Error("/dev/bell");
		/*
		 * Insist on the file descriptor being > 0 so we can
		 * tell if it has been opened or not with having to initialize
		 * pegInfo.bells.fd to -1.
		 */
		if (pegInfo.bells.fd == 0) {
			fd = dup(pegInfo.bells.fd);
			if (fd < 0)
				Error("dup on /dev/bell");
			close(pegInfo.bells.fd);
			pegInfo.bells.fd = fd;
		}
	}
}


/*
 *	NAME
 *		xtlInitBells - Initialization for XTL Keyboard bells.
 *
 *	SYNOPSIS
 */
static void
xtlInitBells()
/*
 *	DESCRIPTION
 *		Opens /dev/bell.
 *
 *	RETURNS
 *		None
 *
 */
{

    pegInfo.bells.fd = open("/dev/bell", O_WRONLY);
    if (pegInfo.bells.fd < 0)
	Error("/dev/bell");
}

static Bool
pegStructInit (pScreen, argc, argv)
    ScreenPtr pScreen;
    /*ARGSUSED*/
    int argc;		/* these two may NOT be changed */
    /*ARGSUSED*/
    char **argv;
{
    int i;
    FILE *filePtr;
    char line[ BUFSIZ ];
    char option[ BUFSIZ ];
    char value[ BUFSIZ ];
    char usrControlFile[ BUFSIZ ];
    char *home, *envVal;
    unsigned int pixMsk;

#ifdef XDEBUG

    extern int	SetDebug();

#ifdef	UTEK
    signal(SIGIO, SetDebug);
#endif	/* UTEK */

#ifdef	UTEKV
    signal(SIGPOLL, SetDebug);
#endif	/* UTEKV */

#endif /* XDEBUG */

    pegInfo.pScr = pScreen;

    /*
     * Initialize the width and height variables.
     * Perhaps save the current hardware CDP registers.
     */
#ifdef	UTEK
    pegInfo.scrHeight = BITMAP_Y(pegInfo.softp);
    pegInfo.width = BITMAP_X(pegInfo.softp);
    pegInfo.height = BITMAP_Y(pegInfo.softp);
    pegInfo.mmScreenX = MM_SCREEN_X(pegInfo.softp);
    pegInfo.mmScreenY = MM_SCREEN_Y(pegInfo.softp);
    pegInfo.entries = 1 << pegInfo.depth;
#endif	/* UTEK */

#ifdef	UTEKV				       /* Redwing */
#ifdef	M4810
    pegInfo.scrHeight = BITMAP_Y(pegInfo.dsp);	/* 1024  (width set below)*/
    pegInfo.width = BITMAP_X(pegInfo.dsp);	/* 2048 */
    pegInfo.height = BITMAP_Y(pegInfo.dsp);	/* 1024 */
    pegInfo.entries = 1 << pegInfo.depth;
#endif	/* M4810 */
#endif	/* UTEKV */

    /*
     *	Process Control Variables
     */
    /* Set defaults */
    for(i = 0; CList[i].envVar != NULL; i++) {
	ConvertValues(&CList[i], CList[i].defVal);
    }

    TekConfig(&pegInfo.mmScreenX, &pegInfo.mmScreenY);
    /*
     * If the Tek configuration database does not exist, we will not have
     * the correct values for screen
     * sizes.  The values should be -1 in this case; use these values instead:
     * 4316/17 GMA-201/303 357mm (x) X 268mm (y)
     * 4406    GMA-201     357mm (x) X 268mm (y)
     * 4405/4315           235mm (x) X 175mm (y)
     * 4319 (19")	   343mm (x) X 274mm (y)
     * XD88/10 Redwing (19") 343mm (x) X 274mm (y)
     */

#ifdef	UTEK
    switch (CPU_BOARD(pegInfo.softp)) { 
    case HC_CPU_4406PLUS:
	pegInfo.scrWidth = SCREEN_X(pegInfo.softp);
	if (pegInfo.mmScreenX <= 0 || pegInfo.mmScreenY <= 0) {
	    pegInfo.mmScreenX = 357;
	    pegInfo.mmScreenY = 268;
	}
	break;
    case HC_CPU_MULTIPLANE:
    case HC_CPU_CROW:
	pegInfo.scrWidth = SCREEN_X(pegInfo.softp);
	if (pegInfo.mmScreenX <= 0 || pegInfo.mmScreenY <= 0) {
	    if (CPU_BOARD(pegInfo.softp) == HC_CPU_CROW) { /* 4319 */
		pegInfo.mmScreenX = 343;
		pegInfo.mmScreenY = 274;
	    } else { /* 4316/7 */
		pegInfo.mmScreenX = 357;
		pegInfo.mmScreenY = 268;
	    }
	}
#endif	/* UTEK */

#ifdef	UTEKV		/* only Redwing supported */
#ifdef	M4810		/* only Redwing supported */
	pegInfo.scrWidth = SCREEN_X(pegInfo.dsp);
	pegInfo.mmScreenX = 343;	/* XD88/10 Redwing */
	pegInfo.mmScreenY = 274;
#endif	/* M4810 */
#endif	/* UTEKV */

        /*
         * If we are running with -mono option, then we need to initialize
         * the CDP registers.  We should only need to do this once because
         * the registers should never get changed.
	 *
	 * NOTE: A "feature" of UnlockDisplay() is that it resets all
	 * CDP registers to powerup defaults.  We must therefore set
	 * up our system to live with this.
         */
#ifdef UTEK
	if (FMonochrome) {
	    int	i;

	    printf ("Current write0Reg: 0x%x\n", pegInfo.cdpCtl->write0Reg);
	    printf ("Current write1Reg: 0x%x\n", pegInfo.cdpCtl->write1Reg);
	    printf ("Current planeEnableReg: 0x%x\n", pegInfo.cdpCtl->planeEnableReg);
	    printf ("Current filterReg: 0x%x\n", pegInfo.cdpCtl->filterReg);
	    printf ("Current maskRegSet: 0x%x\n", pegInfo.cdpCtl->maskRegSet);
	    for (i = 0; i < 5; i++)
	    printf ("Current extra[%d]: 0x%x\n", i, pegInfo.cdpCtl->pad[i]);

            pixMsk = (1 << N_PLANE(pegInfo.softp)) - 1;
            pegInfo.cdpCtl->write1Reg = 0;		/* FOREGROUND */
	    if (DISPLAY_TYPE(pegInfo.softp) == 2)	/* (color) */
		pegInfo.cdpCtl->write0Reg = 1;		/* BACKGROUND */
	    else
		pegInfo.cdpCtl->write0Reg = pixMsk;	/* BACKGROUND */
            pegInfo.cdpCtl->planeEnableReg = pixMsk;
            pegInfo.cdpCtl->filterReg = 0;
            pegInfo.cdpCtl->maskRegSet = ~0;
        }
#endif /* UTEK */

#ifdef UTEKV
	if (FMonochrome) {
            pixMsk = (1 << N_PLANE(pegInfo.dsp)) - 1;
            pegInfo.cdpCtl->write1Reg = 0;		/* FOREGROUND */
	    if (DISPLAY_TYPE(pegInfo.dsp) == DISPLAY_2)	/* Redwing - color */
		pegInfo.cdpCtl->write0Reg = 1;		/* BACKGROUND */
	    else
		pegInfo.cdpCtl->write0Reg = pixMsk;	/* BACKGROUND */
            pegInfo.cdpCtl->planeEnableReg = pixMsk;
            pegInfo.cdpCtl->filterReg = 0;
            pegInfo.cdpCtl->maskRegSet = ~0;
        }
#endif /* UTEKV */

#ifdef	UTEK
	break;

    case HC_CPU_4405PLUS:
	pegInfo.scrWidth = BITMAP_X(pegInfo.softp);
	if (pegInfo.mmScreenX <= 0 || pegInfo.mmScreenY <= 0) {
	    pegInfo.mmScreenX = 235;
	    pegInfo.mmScreenY = 175;
	}
#endif	/* UTEK */

	CList[0].defVal = "ON"; /* panning is on for 4315 */
	/*
	 * The 4315/4405+ workstations can use the X10 capability
	 * of customizing devices.  This is not present in any X11
	 * products.
	 */
	/*
	 *	Process control files
	 */	
	if ((filePtr = fopen(XSYSCONTROLS, "r")) != NULL) {
	    while (fgets(line, sizeof(line), filePtr) != NULL) {
		if (ParseOptionAndValue(line, option, value)) {
		    for (i = 0; CList[i].envVar != (char *) NULL; i++)
			if (strcmp(option, CList[i].fileVar) == 0)
			    ConvertValues(&CList[i], value);
		}
	    }
	    fclose(filePtr);
	}

	home = getenv("HOME");
	if (home != NULL) {
	    strcpy(usrControlFile, home);
	    strcat(usrControlFile, XUSRCONTROLS);
	}

	if ((filePtr = fopen(usrControlFile, "r")) != NULL) {
	    while (fgets(line, sizeof(line), filePtr) != NULL) {
		if (ParseOptionAndValue(line, option, value)) {
		    for (i = 0; strcmp(CList[i].envVar, (char *) NULL); i++)
			if (strcmp(option, CList[i].fileVar) == 0)
			    ConvertValues(&CList[i], value);
		}
	    }
	    fclose(filePtr);
	}

	/*
	 *	Process control environment variables
	 */
	for (i = 0; strcmp(CList[i].envVar, (char *) NULL); i++)
	    if ((envVal = getenv(CList[i].envVar)) != NULL)
		ConvertValues(&CList[i], envVal);
#ifdef	UTEK
	break;
    default:
	FatalError("cpu board unknown=%x\n", CPU_BOARD(pegInfo.softp));
	/*NOTREACHED*/
    }
#endif	/* UTEK */


    /*
     * ensure that delay lists are correct.
     */
    if (pegInfo.kv.nPanDelays != pegInfo.kv.nPanDeltaX
     || pegInfo.kv.nPanDelays != pegInfo.kv.nPanDeltaY) {
	ErrorF("The number of pan deltas must equal the number");
	ErrorF("of pan delays\n");
	return (False);
    }

    /*
     * The Powerup Default Colormap is saved in this routine
     * but is assumed to exist only for one screen.  If multiple screens
     * are implemented, use arrays of size MAXSCREENS to store multiple
     * default colormaps.
     *
     * These default entries will be used by
     * pfbCreateColormap to initialize the colormaps.  The default
     * entries use the X10 data structure ColorDef since that matches the 
     * datastructure used by the SVC calls that manipulate the hardware
     * colormaps.
     *
     * ColorDefDefault will be used by pfbCreateColormap.
     * Note that they are allocated and assigned only once.
     */
    if (pegInfo.depth > 1 && pegInfo.colorDefDefault == NULL) {
	int nEntries = pegInfo.entries;

	pegInfo.colorDefDefault =
	    (SvcColorDefPtr)Xalloc(sizeof(SvcColorDef)*pegInfo.entries);
	if (pegInfo.colorDefDefault == NULL)
	    return (False);

	debug7(("pegScreenInit() - Get default hardware colormap\n"));

	/*
	 * Set machine default "powerup" colormap by passing NULL ptr::
	 */

#ifdef UTEKV
	nEntries = 0;
	if (SetColorMap(nEntries, (SvcColorDefPtr) 0 ) == -1) {
	    ErrorF("SetColorMap returned -failure-\n");
	    ErrorF("nEntries = %d\n", nEntries);
	    ErrorF("errno = %d\n", errno);
	    Error("\n");
	    return(False);
	  }

	/*
	 * Now fetch that machine default "powerup" colormap::
	 */
	if (GetColorMap(&nEntries, pegInfo.colorDefDefault) == -1) {
	    ErrorF("GetColorMap returned -failure-\n");
	    ErrorF("errno = %d\n", errno);
	    Error("\n");
	    return(False);
	  }
#else
	/* original 4310 series code..... */
	SetColorMap(nEntries, (SvcColorDefPtr) 0 );/* sets default color map */
	GetColorMap(&nEntries, pegInfo.colorDefDefault);  /* returns cm_size */
#endif

	assert(nEntries == pegInfo.entries);

#ifdef NOTDEF
	debug7(("pegScreenInit() - Print ColorDefDefault\n"));
	debugPrintCdef(nEntries,pegInfo.colorDefDefault);
#endif
    }

    return (True);
}

#ifdef	UTEK

/*----------------------- M4310 series pegVmScreenInit -----------------------*/

static Bool
pegVmScreenInit(argc, argv)
    /*ARGSUSED*/
    int argc;		/* these two may NOT be changed */
    /*ARGSUSED*/
    char **argv;
{

    struct rlimit rl;
    int	bitmapx = pegInfo.softp->sc_hdwr.screen_u.hc_screen.bitmap_x,
	bitmapy = pegInfo.softp->sc_hdwr.screen_u.hc_screen.bitmap_y,
	pixelSize,
	twoColorSize,
	configSize,
	pageSize = getpagesize(),
	offset;
    long topAddr;	/* address calculation can get complex... */
    caddr_t pixelFb = pegInfo.softp->sc_hdwr.screen_u.hc_screen.pixel_fb,
	    twoColorFb = pegInfo.softp->sc_hdwr.screen_u.hc_screen.fb_addr,
	    oneColorFb =
		pegInfo.softp->sc_hdwr.screen_u.hc_screen.mono_stencil_fb,
	    videoCtlAddr = pegInfo.softp->sc_hdwr.screen_u.hc_screen.mono_ctl,
	    configAddr = pegInfo.softp->sc_info.sc_regs.config_physaddr;

    /*
     * If the mapping is done, don't do it twice.
     */
    if (pegInfo.twoColorFb)
	return (True);

    /*
     * When we are all done, the stack will look like this for systems
     * containing a packed mode frame buffer:
     *    (frame buffers only shown here.  TOP is to left):
     *
     *		stack  unmapped  pixel-FB  1color-FB  2color-FB  page 
     *		      |        |                                      |
     *	---------------         ---------------------------------------
     */

    rl.rlim_cur = STACK_SIZE;
    rl.rlim_max = STACK_SIZE;
    if (setrlimit(RLIMIT_STACK, &rl) < 0) {
	ErrorF("Cannot set stack limit to 0x%x\n", STACK_SIZE);
	return (False);
    }

    /*
     * The top of the frame buffer address space will be at least
     * one page below the stack limit and page-aligned.  Use a stack
     * variable address to start with.  Then assume that the top of
     * the very stack is the page boundary above that.  Subtract off
     * STACK_SIZE plus one page and that will be the address following
     * the last byte in the first frame buffer.
     */
    topAddr = (long)&topAddr;
    topAddr &= ( ~ (pageSize - 1));
    topAddr -= STACK_SIZE + pageSize;

    /*
     * Figure out the size needed for each frame buffer.  Note that the one
     * color frame buffer is the same size as the two color.
     */
    assert((bitmapx & 0x7) == 0);
    twoColorSize = (bitmapx >> 3) * bitmapy;

    if (pixelFb)
	pixelSize = (bitmapx * bitmapy * N_PLANE(pegInfo.softp)) / 8;

    /*
     * Now allocate space, moving downward from topAddr, starting with
     * the pixel framebuffer.
     */
    if (pixelFb) {
	topAddr -= pixelSize;
	/*
	 * The 4316 and 4317 have a strange requirement that the frame
	 * buffer be mapped at an ODD one-megabyte boundary.  This is
	 * not true for Raven, et. al.
	 */
	if (CPU_BOARD(pegInfo.softp) == HC_CPU_MULTIPLANE) {
	    if (N_PLANE(pegInfo.softp) == 4) {
		topAddr &= ( ~ (ONE_MB - 1));	/* 1 Meg boundary */
		if ((topAddr & ONE_MB) == 0)
		    topAddr -= ONE_MB; 		/* odd 1 Meg boundary */
	    }
	    /* XXX
	     *  This is only here for our Raven prototype.  The prototype must
	     *  have a 4 MB alignment.  A real Raven doesn't need this special
	     *  alignment.
	     */
	    else if (N_PLANE(pegInfo.softp) == 8) {
		topAddr &= ( ~ (FOUR_MB - 1));
	    }
	}
	/*
	 * Real Raven has no alignment restriction.  mmap has a page
	 * alignment restriction.
	 */

	pegInfo.pixelFb = (caddr_t)topAddr;
	if (mmap(M_PHYS, pixelFb, topAddr, pixelSize, SELF, SHARED) < 0) {
	    ErrorF("Can't map pixel frame buffer\n");
	    return (False);
	}

	topAddr = topAddr - twoColorSize;
	pegInfo.oneColorFb = (caddr_t)topAddr;
	if (mmap(M_PHYS, oneColorFb, topAddr, twoColorSize, SELF, SHARED) < 0) {
	    ErrorF("Can't map one-color frame buffer\n");
	    return (False);
	}
    }

    /*
     * The 4315 and 4406plus require that the frame buffer be mapped
     * at a boundary equal to the frame buffer size (256K).
     */
    if (CPU_BOARD(pegInfo.softp) == HC_CPU_4405PLUS
     || CPU_BOARD(pegInfo.softp) == HC_CPU_4406PLUS)
	topAddr &= ( ~ (twoColorSize - 1));

    topAddr = topAddr - twoColorSize;
    pegInfo.twoColorFb = (caddr_t)topAddr;
    if (mmap(M_PHYS, twoColorFb, topAddr, twoColorSize, SELF, SHARED) < 0) {
	ErrorF("Can't map two-color frame buffer\n");
	return (False);
    }
    /*
     * In many algorithms in pfb, we need to access one longword at a memory
     * location one longword less than the legitimate source address.  To
     * allow source drawables in the 2 color frame buffer, we need now to
     * allocate the memory word preceding twoColorFb.  We will actually
     * allocate a page because mmap requires us to use page aligned
     * addresses
     */
    topAddr -= pageSize;
    if (mmap(M_PHYS, twoColorFb, topAddr, pageSize, SELF, SHARED) < 0) {
	ErrorF("Can't map two-color frame buffer buffer\n");
	return (False);
    }

    /*
     * map software configuration block.
     * Note that we must align the hardware address on a page boundary,
     * remembering our offset.
     */
    configSize = (sizeof(struct soft_config) + pageSize - 1)
			& ( ~ (pageSize - 1));
    topAddr -= configSize + pageSize; /* skip one page */
    offset = (int)configAddr - ((int)configAddr & ~(pageSize-1));
    if (mmap(M_PHYS, configAddr-offset,
      topAddr, configSize, SELF, SHARED) < 0) {
	ErrorF("Can't map configuration block\n");
	return (False);
    }

    /* if they aren't identical, we mapped it wrong */
    assert(bcmp(topAddr+offset,pegInfo.softp,sizeof(struct soft_config)) == 0);

    pegInfo.softp = (struct soft_config *) (topAddr + offset);
    pegInfo.dsp = (XDisplayState *)pegInfo.softp->sc_eventque;

    /*
     * map video control register and
     *			setup pointer to kernel - video state shadow var..
     */
    topAddr -= pageSize * 2; /* skip one page */
    offset = (int)videoCtlAddr - ((int)videoCtlAddr & ~(pageSize-1));
    pegInfo.videoCtl = (caddr_t) topAddr + offset;
    if (mmap(M_PHYS, videoCtlAddr-offset, topAddr, pageSize, SELF, SHARED)<0) {
	ErrorF("Can't map video control register\n");
	return (False);
    }
    pegInfo.videostate = (unsigned short *) &pegInfo.softp->sc_videostate;

    /*
     * map Color Data Path (CDP) control register.
     */
    if (pixelFb) {
	caddr_t	cdpCtl;

	cdpCtl = (caddr_t)pegInfo.softp->sc_hdwr.screen_u.hc_screen.color_ctl;
	topAddr -= pageSize * 2; /* skip one page */
	offset = (int)cdpCtl - ((int)cdpCtl & ~(pageSize-1));
	pegInfo.cdpCtl = (COLOR_CNTL *) (topAddr + offset);
	if (mmap(M_PHYS, cdpCtl-offset, topAddr, pageSize, SELF, SHARED) < 0) {
	    ErrorF("Can't map CDP\n");
	    return (False);
	}
    }
    else {
	/* ???
	 * This is here so that we can set color control registers harmlessly
	 * on 4315/4406.  We can't set them willynilly on monochrome systems
	 * because on n-plane systems in -mono mode, this could break stuff.
	 * We could map cpdCtl to harmless memory on these systems too, but
	 * do we want to lose the ability to set these regs on n-plane
	 * systems?
	 */
	pegInfo.cdpCtl = 
	    (COLOR_CNTL *) Xalloc(sizeof(COLOR_CNTL));
    }

    return (True);
}

/*--------------- end of M4310 series pegVmScreenInit -----------------------*/

#endif	/* UTEK */



#ifdef	UTEKV
/*----------------------- M4810 series pegVmScreenInit -----------------------*/

static Bool
pegVmScreenInit(argc, argv)
    int argc;		/* these two may NOT be changed */
    char **argv;
{

#ifdef	M4810 /*------------- Redwing ---------------------------------------*/

    int	bitmapx, bitmapy, pixelSize, twoColorSize, configSize, pageSize, offset;
    int shMemId;	/* shared memory return id */
    caddr_t dspBaseAddr; /* mapped address of the base of the display board. */
    caddr_t caddrTemp;	 /* handy reg */
    caddr_t segmentAddr; /* pleasant reg */

    /*
     * If the mapping is done, don't do it twice.
     */
    if (pegInfo.twoColorFb)
	return(True);

    /* 
     * Here is what is done in this "VmScreen" init code::
     *
     *	o   Fetch the X driver's physical address for the "shared memory"
     *	    driver-to-server interface (display state and event queue).
     *	    Note: This is done by making an ioctl call on the x-dev "eventFd"
     *	    file descriptor.
     *
     *	o   Setup a shared/cache-inhibited/no-clear mapping of the
     *	    "shared-memory" segment to a "higher" address. Note that the
     *	    physical address of the "shared-memory" page(s) usually on the
     *	    order of 0x0024A000/0x00024C000. This memory's "segment" will
     *	    be mapped to a base of 0x60000000 (above the "u" area in kernel
     *      space however (0x40000000)
     *	    and below the display board's base address: (0x80000000).
     *	    "pegInit.dsp" points to this "shared-memory" "base".
     *	    
     *	    
     *	o   The Display Board's devices/memory/registers will be mapped
     *	    into -one- segment, extending from "DS_DSP_BASE" (0x80000000)
     *	    "DS_FB_PP_HI_P" (0x803fffff) (4MegaBytes);  mapped "two-to-one"
     *	    at the same user virtual address as the kernel virtual address
     *	    (and the same as the physical addresses).
     *
     *	    Therefore, references to the individual display board devices
     *	    will be from a device address calculated from the "segment base"
     *	    plus the "offset" from the "base" address of the display board.
     *	    
     *	    
     *	    
     *	o  Notes:
     *	    1) NBPC (NumberOfBytesPerClick) = 4096 
     *	    2) NCPS (NumberOfClicksPerSegment) = 1024
     *	    
     *	    
     *	    
     *							Steve Jensen
     */


    /* 
     *  Get page size... for future calculations.
     */ 
    pageSize = getpagesize();
    assert (pageSize == NBPC);

    /*
     * Get "shared" memory display structure/event buffer physical address.
     */
    if (ioctl(pegInfo.eventFd, CE_GETXDISPLAYSTRUCT, &caddrTemp) < 0) {
	ErrorF("CE_GETXDISPLAYSTRUCT returned value less than 0");
	Error("errno = ");
	return(False);
      }

#ifdef	DEBUG_SHMGET
    ErrorF("ioctl: CE_GETXDISPLAYSTRUCT returns -- \n");
    ErrorF("config. address = 0x%x\n", caddrTemp);
    ErrorF("\n");
#endif	/* DEBUG_SHMGET */

    /*
     * round size of displaystate to the greater page size boundary.
     */
    configSize = (sizeof(struct displaystate) + pageSize-1) & (~(pageSize-1));
    offset = (int) caddrTemp;	/* save the "offset" into 'the' segment */

    /*
     * map to segment aligned and a size of one segment.
     */
    shMemId = shmget(IPC_PRIVATE, NBPS, PHYS_FLAGS,
 				(int)caddrTemp & (~(NBPS-1)));

    if (shMemId == -1) {
	ErrorF("Can't -shmget- Xdriver shared block\n");
	ErrorF("mapping with: IPC_PRIVATE | IPC_CI | IPC_NOCLEAR\n");
	ErrorF("address = 0x%x\n", caddrTemp);
	Error("errno = ");
	return(False);
      }

    caddrTemp = ((caddr_t)(PHYS_HIGH_ADDR));
    segmentAddr = (caddr_t) (shmat(shMemId, caddrTemp, 0));

    if ((int) segmentAddr == -1) {
	ErrorF("Can't -shmat- shared segment\n");
	ErrorF("mapping with flags: 0\n");
	ErrorF("Id = 0x%x\n", shMemId);
	ErrorF("address = 0x%x\n", caddrTemp);
	ErrorF("segment address returned = 0x%x\n", segmentAddr);
	Error("errno = ");
	return(False);
      }

    /*
     * extract offset within segment::
     */
    offset = offset & (NBPS-1);

    /*
     * compute "real" address::
     */
    pegInfo.dsp = (XDisplayState *) (caddr_t) ((int) segmentAddr + offset);

#ifdef	DEBUG_SHMGET
    ErrorF("Mapping of shared segment sucessful.\n");
    ErrorF("shared segment mapped to virtual address = 0x%x\n", pegInfo.dsp);
    ErrorF("shared segment offset = 0x%x\n", offset);
    ErrorF("\n");
#endif	/* DEBUG_SHMGET */

    bitmapx = pegInfo.dsp->ds_fbBitmap_x;
    bitmapy = pegInfo.dsp->ds_fbBitmap_y;

#ifdef	DEBUG_SHMGET
    ErrorF("dumping -the- items of interest from -dsp-:\n");
    ErrorF("ds_maxx = %d\n", pegInfo.dsp->ds_maxx);
    ErrorF("ds_maxy = %d\n", pegInfo.dsp->ds_maxy);
    ErrorF("ds_minx = %d\n", pegInfo.dsp->ds_minx);
    ErrorF("ds_minxy= %d\n", pegInfo.dsp->ds_miny);
    ErrorF("ds_cWidth = %d\n", pegInfo.dsp->ds_cWidth);
    ErrorF("ds_cHeight = %d\n", pegInfo.dsp->ds_cHeight);
    ErrorF("ds_ltime = 0x%x\n", pegInfo.dsp->ds_ltime);
    ErrorF("ds_displayModel = %d\n", pegInfo.dsp->ds_displayModel);
    ErrorF("ds_ramdacDepth = %d\n", pegInfo.dsp->ds_ramdacDepth);
    ErrorF("ds_fbDepth = %d\n", pegInfo.dsp->ds_fbDepth);
    ErrorF("ds_fbBitmapx = %d\n", bitmapx);
    ErrorF("ds_fbBitmapy = %d\n", bitmapy);
    ErrorF("ds_fbScreen_x = %d\n", pegInfo.dsp->ds_fbScreen_x);
    ErrorF("ds_fbScreen_y = %d\n", pegInfo.dsp->ds_fbScreen_y);
    ErrorF("ds_fbLong_incr = %d\n", pegInfo.dsp->ds_fbLong_incr);
    ErrorF("ds_fbShort_incr = %d\n", pegInfo.dsp->ds_fbShort_incr);
    ErrorF("ds_fbByte_incr = %d\n", pegInfo.dsp->ds_fbByte_incr);
    ErrorF("\n");
    ErrorF("dumping -misc- items of interest from -dsp-:\n");
    ErrorF("videostate = 0x%x\n", pegInfo.dsp->videostate);
    ErrorF("keyboardlangidstyle = 0x%x\n", pegInfo.dsp->keyboardlangidstyle);
    ErrorF("xconsolepid = %d\n", pegInfo.dsp->xconsolepid);
    ErrorF("consolemsgbuffer = %d\n", ((int) pegInfo.dsp->consolemsgbuffer));
    ErrorF("\n");
#endif	/* DEBUG_SHMGET */

    assert(bitmapx != 0);
    assert(bitmapy != 0);

    shMemId = shmget(IPC_PRIVATE, DS_NUMBER_OF_BYTES, PHYS_FLAGS, DS_DSP_BASE);
    if (shMemId == -1) {
	ErrorF("Can't -shmget- display board\n");
	ErrorF("mapping with: IPC_PRIVATE | IPC_CI | IPC_NOCLEAR\n");
	ErrorF("size = 0x%x\n", DS_NUMBER_OF_BYTES);
	ErrorF("address = 0x%x\n", DS_DSP_BASE);
	Error("errno = ");
	return(False);
      }

    caddrTemp = shmat(shMemId, DS_DSP_BASE, 0);
    if ((int) caddrTemp == -1) { /* remember display board addrs = 0x80000000 */
	ErrorF("Can't -shmat- display board\n");
	ErrorF("mapping with flags: 0\n");
	ErrorF("Id = 0x%x\n", shMemId);
	ErrorF("address = 0x%x\n", DS_DSP_BASE);
	ErrorF("shmat returned = 0x%x\n", caddrTemp);
	Error("errno = ");
	return(False);
      }

#ifdef	DEBUG_SHMGET
    ErrorF("Display board mapped to address 0x%x  by -shmat-\n", caddrTemp);
    ErrorF("\n");
#endif	/* DEBUG_SHMGET */

    dspBaseAddr = caddrTemp;	/* establish the "display board" base address */
    pegInfo.twoColorFb = (caddr_t) ((int) dspBaseAddr + OFFSET_TWO_COLOR_FB);
    pegInfo.oneColorFb = (caddr_t) ((int) dspBaseAddr + OFFSET_STENCIL_FB);
    pegInfo.pixelFb = (caddr_t) ((int) dspBaseAddr + OFFSET_PPM_FB);
    pegInfo.cdpCtl = (COLOR_CNTL *) ((int) dspBaseAddr + OFFSET_CDP_0_REG);
    pegInfo.videoCtl = (caddr_t) ((int) dspBaseAddr + OFFSET_VIDEO_REG);
    pegInfo.videostate = (unsigned char *)
			 &pegInfo.dsp->videostate; /* shadow reg */

#ifdef	DEBUG_SHMGET
    ErrorF("Display board is mapped at: 0x%x\n", dspBaseAddr);
    ErrorF("Two Color Frame buffer mapped at: 0x%x\n", pegInfo.twoColorFb);
    ErrorF("One Color/Stencil Frame buffer at: 0x%x\n", pegInfo.oneColorFb);
    ErrorF("PPM Frame buffer at: 0x%x\n", pegInfo.pixelFb);
    ErrorF("CDPs mapped at: 0x%x\n", pegInfo.cdpCtl);
    ErrorF("Video control-status register mapped at: 0x%x\n", pegInfo.videoCtl);
    ErrorF("Video control SHADOW var. mapped at: 0x%x\n", pegInfo.videostate);
#endif	/* DEBUG_SHMGET */

    /*
     * -Bill is going to think about this ... may mot be true for 88K blt code--
     *
     * In many algorithms in pfb, we need to access one longword at a memory
     * location one longword less than the legitimate source address.  To
     * allow source drawables in the 2 color frame buffer, we need now to
     * allocate the memory word preceding twoColorFb.  We will actually
     * allocate a page because mmap requires us to use page aligned
     * addresses
     */

#endif	/* M4810 **------------- Redwing -------------------------------------*/

    return (True);
}

/*--------------- end of M4810 series pegVmScreenInit -----------------------*/

#endif	/* UTEKV */

static Bool (*wrappedCloseScreen)();

static Bool
pegCloseScreen (i, pScreen)
    int		i;
    ScreenPtr	pScreen;
{
    int	ret;

    pScreen->CloseScreen = wrappedCloseScreen;
    ret =  (*pScreen->CloseScreen) (i, pScreen);
    (void) close (pegInfo.bells.fd);
    (void) close (pegInfo.eventFd);
}

/*
 * Screen Init for tek servers
 */
Bool
pegScreenInit (index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;		/* these two may NOT be changed */
    char **argv;
{
    ColormapPtr	pColormap;
    CARD16	zero = 0, ones = ~0;
    int		i, j;
    int		useCfb;
    int		resolution;

    useCfb = pegInfo.depth != 1 && !FMonochrome;

    if (! pegVmScreenInit(argc, argv))
	return (False);

    if (! pegStructInit(pScreen, argc, argv))
	return (False);

    resolution = (254 * pegInfo.scrWidth) / (10 * pegInfo.mmScreenX);

    if (useCfb)
    {
	if (! cfbScreenInit (pScreen, (pointer) pegInfo.pixelFb,
			     pegInfo.scrWidth,
			     pegInfo.scrHeight,
			     resolution, resolution,
			     pegInfo.width))
	    return FALSE;
    	pScreen->InstallColormap	= pegInstallColormap;
    	pScreen->UninstallColormap     	= pegUninstallColormap;
    	pScreen->ListInstalledColormaps	= pegListInstalledColormaps;
	pScreen->StoreColors            = pegStoreColors;
    }
    else
    {
	if (! mfbScreenInit (pScreen, (pointer) pegInfo.twoColorFb,
			     pegInfo.scrWidth,
			     pegInfo.scrHeight,
			     resolution, resolution,
			     pegInfo.width))
	    return FALSE;
    }

    wrappedCloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = pegCloseScreen;

#ifdef	UTEK
    /*
     * If display is a 4315,
     *	check if joydisk panning is enabled,
     *	set initial viewport to middle of display
     */
    if (CPU_BOARD(pegInfo.softp) == HC_CPU_4405PLUS) {
	SvcCursorXY vp;

	/*
	 * Set the initial viewport to the middle of the screen
	 */
	vp.x = pegInfo.dsp->ds_x - (pegInfo.softp->sc_vscreen_x>>1),
	vp.y = pegInfo.dsp->ds_y - (pegInfo.softp->sc_vscreen_y>>1);
	SetViewport(&vp);
    }
#endif	/* UTEK */

    ioctl(pegInfo.eventFd, CE_SETCONS, &pegInfo.consolePid);
    ioctl(pegInfo.eventFd, CE_EVENTS, NULL);

    /*
     * turn on screen.
     * X handles blanking, so turn off hardware screensaver
     * insure (one plane) screen 0 => black, 1=> white
     */
    DisplayOn();
    TimeoutOff();

#ifdef	UTEK
    if (CPU_BOARD(pegInfo.softp) == HC_CPU_4405PLUS || 
	    CPU_BOARD(pegInfo.softp) == HC_CPU_4406PLUS)
	InvertVideo();
#endif	/* UTEK */

    pScreen->SaveScreen = pegSaveScreen;

    pScreen->DisplayCursor              = pegDisplayCursor;
    pScreen->RealizeCursor = pegRealizeCursor;
    pScreen->UnrealizeCursor = pegUnrealizeCursor;
    pScreen->SetCursorPosition = pegSetCursorPosition;
    pScreen->CursorLimits = pegCursorLimits;
    pScreen->PointerNonInterestBox = pegPointerNonInterestBox;
    pScreen->ConstrainCursor = pegConstrainCursor;
    pScreen->RecolorCursor = pegRecolorCursor;
/*
    pScreen->QueryBestSize = pegQueryBestSize;
*/

    pScreen->mmWidth = pegInfo.mmScreenX;
    pScreen->mmHeight = pegInfo.mmScreenY;

    SaveScreens(SCREEN_SAVER_OFF, ScreenSaverReset);

    if (useCfb)
    {
	if (!cfbCreateDefColormap (pScreen))
	    return FALSE;
    }
    else
    {
	pScreen->whitePixel = 0;
	pScreen->blackPixel = 1;
	if (!mfbCreateDefColormap (pScreen))
	    return FALSE;
    }

#ifdef	UTEK
    /*
     * Change the graphics routines so that we
     * can deal with the cursor first.
     */
    if (pegInfo.softwareCursor)
	miDCInitialize ();		/* no interface yet */

#endif	/* UTEK */
    return TRUE;
}

void
InitOutput(screenInfo, argc, argv)
    ScreenInfo *screenInfo;
    int argc;
    char **argv;
{
    int i, cpu;
    PixmapFormatPtr	formats;

#ifdef	UTEK
#ifdef SABER
    static struct soft_config *soft_config_pointer = (struct soft_config *)0xff3b000;
    /* Hack - since Saber has its own C-runtime crt.o, it doesn't know about
       this global.  So I initialize it to the address observed when running a
       simple C program normally on a 4317. */
#else
    extern struct soft_config	*soft_config_pointer;
#endif /* SABER */
#endif	/* UTEK */

#ifdef XDEBUG
    setuid(0); /* for core dumps */
#endif /* XDEBUG */

    /*
     * We must open /dev/xdev before we set any CDP registers or set
     * any colormap entries because the "open" causes the kernel to
     * save the display state so that it can restore it when
     * xdev gets closed.
     */
#if defined UTEK | defined UTEKV
    if ((pegInfo.eventFd = open("/dev/xdev", O_RDONLY)) < 0) {
	FatalError("can't open /dev/xdev\n");
    }
#endif	/* UTEK | UTEKV*/

#ifdef	UTEK
    pegInfo.softp = soft_config_pointer;
#endif	/* UTEK */

#ifdef	UTEKV
    if ((pegInfo.eventFd = open("/dev/xdev", O_RDONLY)) < 0) {
	FatalError("can't open /dev/xdev\n");
    }
#endif	/* UTEKV */

    screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

#ifdef	UTEK
    /*
     * There are a variety of screens that we can configure for.
     * 4315, 4406, 4316, 4316 pretending to be mono, 4317 4317 pretending
     * to be 4316, 4317 pretending to be mono.  Choose here.
     * Note that a small number of important items in the pegInfo
     * structure are assigned here.  The rest are assigned in pegStructInit.
     */
    cpu = CPU_BOARD(pegInfo.softp);
    if (cpu == HC_CPU_4405PLUS
     || cpu == HC_CPU_4406PLUS
    ) {
	formats = OneBitFormats;
	pegInfo.depth = 1;
	pegInfo.fAvailableCDP = 0;

	if (cpu == HC_CPU_4405PLUS || cpu == HC_CPU_4406PLUS)
	    pegInfo.softwareCursor = True;

    } else if (cpu == HC_CPU_MULTIPLANE || cpu == HC_CPU_CROW) {
	pegInfo.fAvailableCDP = 1;
	if (N_PLANE(pegInfo.softp) == 8) {
	    formats = EightBitFormats;
	    pegInfo.depth = 8;
	}
	else if (N_PLANE(pegInfo.softp) == 4) {
	    FatalError("This server can only accomodate 1 or 8 bit screens\n");
	}
    } else if (cpu == HC_CPU_CEM) {
	FatalError("This server can only accomodate 4310 series machines\n", 
	    pegInfo.softp->sc_hdwr.hc_cpu_board);
    } else {
	/* add blackbird, redwing here */
	FatalError("cpu board unknown=%x\n", 
	    pegInfo.softp->sc_hdwr.hc_cpu_board);
    }

#endif	/* UTEK */

#ifdef	UTEKV
#ifdef	M4810
#ifdef notdef

------ because the xdev mapping for the shared memeory has not happend YET !! --

	assert (N_PLANE(pegInfo.dsp) == 8);	/* hardwire display type #8 */

	pegInfo.fAvailableCDP = 1;		/* yep, its got CDPs */

	if (N_PLANE(pegInfo.dsp) == 8) {
	    formats = EightBitFormats;
	    pegInfo.depth = 8;
	}

#else  /* notdef */

/*
 *			hack for Redwing !!!!!!
 *
 *	--------- cheap hack for now .... fix the ordering of the shared memory
 *	-----------			  later !!!!
 */

	pegInfo.fAvailableCDP = 1;
	formats = EightBitFormats;
	pegInfo.depth = 8;

#endif /* notdef */
#endif	/* M4810 */
#endif	/* UTEKV */

    pegInfo.entries = 1 << pegInfo.depth;

    for (i=0; formats[i].depth; i++)
	screenInfo->formats[i] = formats[i];

    screenInfo->numPixmapFormats = i;

    AddScreen(pegScreenInit, argc, argv);
    /*
     * smash the current time of day as the value is completely
     * bogus
     */
    currentTime.milliseconds = GetTimeInMillis();
    currentTime.months = 0;
}

void
pegEventInit()
{
    BoxRec	box;
    int		i;

#ifdef XPEG_TANDEM
    if (!pegTandem)
    {
#endif /* XPEG_TANDEM */
	    pegInfo.queue = &pegInfo.dsp->ds_q;
				/* 
				 * For redwing remember that this is
				 * mapped at 0x60000000 + the kernel
				 * virtual address.
				 */
#ifdef XPEG_TANDEM
    }
    else
    {
	    pegInfo.queue = (EventQueue *)Xalloc(sizeof(EventQueue));
	    pegInfo.queue->size = NXEVENTS;
	    pegInfo.queue->events = (Event *)Xalloc(sizeof(Event) * NXEVENTS);
	    pegInfo.queue->head = 0;
	    pegInfo.queue->tail = 0;
	    pegInfo.width = 640;
	    pegInfo.height = 480;
    }
#endif /* XPEG_TANDEM */
    pegInfo.qLimit = pegInfo.queue->size - 1;

    /*
     * We're interested in everything.
     */
    box.y2 = box.y1 = box.x1 = box.x2 = 0;
    pegPointerNonInterestBox( pegInfo.pScr, &box);

    /*
     * initialize timeouts.
     */
    for (i=0; i < N_TIMERS; i++)
	pegInfo.kv.timer[ i ].key = -1;
}

void
InitInput(argc, argv)
    /*ARGSUSED*/
    int argc;
    /*ARGSUSED*/
    char **argv;
{
    DevicePtr p, k;

    /*
     * Initialize events and such...
     */
#ifdef XTESTEXT1
/*
 * For Tek workstations,
 *      Use BREAK key as COMMAND_KEY.
 *      This key has the same hardware code for 4315/6/7 keyboard
 *      as it does for xtal/tnt/Raven keyboard.
 *      We need to add 8 to the hardware key report to turn it into a X11
 *      keycode.
 */
    xtest_command_key = KEY_Break + 8;
#endif /* XTESTEXT1 */

#ifndef MOTION_BUFFER_OFF
    gfbInitMotionQueue(&pegInfo.motionQueue, MOTION_BUFFER_SIZE);
#endif /* MOTION_BUFFER_OFF */

    pegEventInit();
    InitKeybdState();

#ifdef	UTEK
    if ((KEYIDSTYLE(pegInfo.softp) ==  KB_STYLE_VT200) ||
	(KEYIDSTYLE(pegInfo.softp) ==  KB_STYLE_YELLOW_JACKET))
#endif	/* UTEK */

#ifdef	UTEKV
    if ((KEYIDSTYLE(pegInfo.dsp) ==  KB_STYLE_VT200) ||
	(KEYIDSTYLE(pegInfo.dsp) ==  KB_STYLE_YELLOW_JACKET))
#endif	/* UTEKV */
      {
	xtlInitBells();
    } else {
	pegInitBells(FALSE);
    }

    p = AddInputDevice(pegMouseProc, TRUE);

    k = AddInputDevice(pegKeybdProc, TRUE);

    RegisterPointerDevice(p);
    RegisterKeyboardDevice(k);
}

#ifdef XTESTEXT1
/*
 *	NAME
 *		pegInputSynthesisExt -- dummy routine for proper linking
 *
 *	SYNOPSIS
 */
pegInputSynthesisExt()
/*
 *	DESCRIPTION
 *		This routine exists just to cause linking of input
 *		synthesis extension routines residing in this directory.
 *		This routine should never get called.
 *
 *	RETURNS
 *		None
 *
 */
{
#ifdef	THIS_IS_BROKEN
    doassert();
#else	/* THIS_IS_BROKEN */
    assert(FALSE);
#endif	/* THIS_IS_BROKEN */

    XTestGenerateEvent(0, 0, 0, 0, 0);
    XTestJumpPointer(0, 0, 0);
    XTestGetPointerPos((short) 0, (short) 0);
}
#endif /* XTESTEXT1 */


/*
 *	NAME
 *		TekConfig - Do Tek-specific configuration
 *
 *	SYNOPSIS
 */
int
TekConfig(mmScreenX, mmScreenY)
    int *mmScreenX;		/* mm X dimension of screen */
    int *mmScreenY;		/* mm Y dimension of screen */
/*
 *	DESCRIPTION
 *		Get info from Tek-specific configuration file.
 *		Currently this is screen dimensions and gamma
 *		correction file.  If the database is accessed
 *		successfully, mmScreenX and mmScreenY will have
 *		correct values set; otherwise they will be set to -1.
 *
 *		Note: we used to use dbm for both displayenv database
 *		and rgb database.  This routine handled the problems
 *		associated with this in RCS rev: 1.27. 
 *
 *	RETURNS
 *		0 if database was accessed successfully
 *		-1 otherwise
 *
 */
{
#ifdef KNOW_NOW
    int ret;
    
    ret = DisplayEnv_init();
    if (ret != 0) {
	*mmScreenX = *mmScreenY = -1;
    } else {
	*mmScreenX = DisplayLookUpMillimeterRasterSizeX();
	*mmScreenY = DisplayLookUpMillimeterRasterSizeY();
    }
#else
    *mmScreenX = -1;
    *mmScreenY = -1;
#endif
}

/*
 *	NAME
 *		gfbNullFunction - Illegal function for unwanted function vectors
 *
 *	SYNOPSIS
 */
void
gfbNullFunction()
/*
 *	DESCRIPTION
 *		Call Fatal Error, warning that we shouldn't have gotten here.
 *		Use this as the value of a function vector which you don't
 *		want called.
 *
 *	RETURNS
 *		None
 *
 */
{
#ifdef XDEBUG
    FatalError("gfbNullFunction called.  Unused function vector dereferenced\n");
#endif /* XDEBUG */
}

