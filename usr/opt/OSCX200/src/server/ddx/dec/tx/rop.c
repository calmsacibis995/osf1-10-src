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
/***********************************************************
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
/*
** File: 
**
**   rop.c --- RasterOps device interface
**
** Author: 
**
**   Joel Gringorten DEC-WSL
**
** Revisions:
**
**   29.08.91 Carver
**     - changed all interfaces to take RopPtr as first argument
**       RopPtr is defined in rop.h
**     - added some R5 conditional compilation
**
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <errno.h>


#include "misc.h"
#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "pixmap.h"
#include "input.h"
#include "cursorstr.h"
#include "regionstr.h"
#include "resource.h"
#include "dixstruct.h"

#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include "ws.h"

#include "mfb.h"
#include "xfb.h"
#include "rop.h"
#include "ropcolor.h"

#undef PSZ
#include "maskbits.h" /* LOCATION OF DUFF MACRO IN R5 --- DCC */

#include <machine/cpuconf.h>
extern int ws_cpu;
#define is3MAX	(ws_cpu == DS_5000)
#define is3min	(ws_cpu == DS_5000_100)

/* XXX this is so we can compile this file without an up-to-date (4.L) copy
    of machine/cpuconf.h.  This can cause terrible problems if you run
    a server compiled under 4.2 and run it on a later base system version
    if that base system uses a different value for DS_MAXINE.
*/
#ifndef DS_MAXINE
#define DS_MAXINE 29
#endif
#define isMaxine (ws_cpu == DS_MAXINE)

#if defined(__alpha) && defined(__osf__)
#define BANK_SW_DEFAULT 0
#else
/* this determines whether bank switching is turned on by default */
#define BANK_SW_DEFAULT (!(is3min || isMaxine ))
#endif


extern int wsScreenPrivateIndex;
extern int lastEventTime;
void wsQueryBestSize();
extern Bool wsScreenClose();

extern void wsQueryBestSize64();
extern void  wsQueryBestSize16();

extern int wsScreenInit();
extern int wsFd;

static ws_depth_descriptor depthDesc[MAXSCREENS];
static mapped[MAXSCREENS];
static char *planemask_addr;

extern	ws_event_queue	*queue;
#undef VSYNCFIXED
#ifdef VSYNCFIXED
#define CURRENT_TIME	queue->time
#else
#define CURRENT_TIME	GetTimeInMillis()
#endif

#ifdef __osf__TEMP
#include <io/dec/ws/pmagro.h> osf??
#include <io/ws/pmagro.h> ultrix ??
#else
#include "pmagro.h"
#endif

ropStruct ropInfo[MAXSCREENS];

int txDebugOption = 0;

static void
colorNameToColor( pname, pred, pgreen, pblue)
    char *      pname;
    u_int *     pred;
    u_int *     pgreen;
    u_int *     pblue;
{
    if ( *pname == '#')
    {
        pname++;                /* skip over # */
        sscanf( pname, "%2x", pred);
        *pred <<= 8;

        pname += 2;
        sscanf( pname, "%2x", pgreen);
        *pgreen <<= 8;

        pname += 2;
        sscanf( pname, "%2x", pblue);
        *pblue <<= 8;
    }
    else /* named color */
    {
        *pred = *pgreen = *pblue = 0; /*OsLookupColor thinks these are shorts*/
        OsLookupColor( 0 /*"screen", not used*/, pname, strlen( pname),
                pred, pgreen, pblue);
    }
}

void txUseMsg()
{
    ErrorF ("\n");
    ErrorF ("\n");
    ErrorF ("tx Device Dependent Usage\n");
    ErrorF ("Note - all tx arguments can take an optional screen number ``s''\n");
    ErrorF ("\n");
    ErrorF ("-txXorFix[s] n        		xor fix option\n");
    ErrorF ("-txBankSwitch[s] n    		bank switch option\n");
    ErrorF ("-txRootVisual[s] depth visual    	root visual\n");
}
void txProcessArguments (argc, argv)
    register int argc;
    register char *argv[];
{
    int			argind=0;
    int			skip;
    int 		screen;
    register 		int i;
    static int		Once = 0;

    /* There's no need to process arguments for each instance of a tx screen */
    if ( Once == 1 ) return;
    Once = 1;

  for ( argind = 0; argind < argc; argind++ ) {

    /* The next 3 secret -tx options are for debug and in case of emergency */
    if (ArgMatch(argv[argind], "-txXorFix", &screen)) {
	if (++argind < argc) {
	    MarkArgvUsed(argind-1, 2);
	    if (screen == -1) {
		for (i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_TXXORFIX;
		    screenArgs[i].txXorFix = atoi(argv[argind]);
		}
	    }
	    else {
		screenArgs[screen].flags |= ARG_TXXORFIX;
		screenArgs[screen].txXorFix = atoi(argv[argind]);
	    }
	}
	else
	    UseMsg();
    }
    else if (ArgMatch(argv[argind], "-txBankSwitch", &screen)) {
	if (++argind < argc) {
	    MarkArgvUsed(argind-1, 2);
	    if (screen == -1) {
		for (i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_TXBANKSW;
		    screenArgs[i].txBankSwitch = atoi(argv[argind]);
		}
	    }
	    else {
		screenArgs[screen].flags |= ARG_TXBANKSW;
		screenArgs[screen].txBankSwitch = atoi(argv[argind]);
	    }
	}
	else
	    UseMsg();
    }
    else if (ArgMatch(argv[argind], "-txRootVisual", &screen)) {
	if (++argind + 1 < argc) {
	    MarkArgvUsed(argind-1, 3);
	    if (screen == -1) {
		for (i = 0; i < MAXSCREENS; i++) {
		    screenArgs[i].flags |= ARG_TXRVISUAL;
		    screenArgs[i].txRootDepth = atoi(argv[argind]);
		    screenArgs[i].txRootClass = atoi(argv[argind + 1]);
		}
	    }
	    else {
		screenArgs[screen].flags |= ARG_TXRVISUAL;
		screenArgs[screen].txRootDepth = atoi(argv[argind]);
		screenArgs[screen].txRootClass = atoi(argv[argind + 1]);
	    }
	    argind++;
	}
	else
	    UseMsg();
    }
  }
  /* Since these are debugging options only, we'll parse for them
   * but we won't display their usage message. If we did it would
   * look like this...
  RegisterUseMsgProc(txUseMsg);
   */
}

/* This routine is almost identical to ws's fbInitProc and
 * should someday be re-written to use it where it can.
 */
Bool
ropInitProc(index, pScreen, argc, argv)
    int index;
    ScreenPtr pScreen;
    int argc;
    char **argv;
{
    register    		PixmapPtr pPixmap;
    ColormapPtr 		pcmap;
    int				dpix, dpiy, i;
    wsScreenPrivate 		*wsp;
    Bool 			success = FALSE;
    ws_depth_descriptor 	*dd;
    static ws_map_control 	mc;
    VisualPtr	    		pVisual;
    ColormapPtr	    		pCmap;
    int 			defaultRootDepth;
    Bool 			useBankSwitch, useXorFix, useBStore;
    void 			rop_pip_init();
    int				depthIndex;
    int				psn;	/* physical screen number */
    int 			defaultVisualClass;
    ScreenArgsRec		* scrArgs;

    /* for initializing color map entries */
    u_int blackred      = 0x0000;
    u_int blackgreen    = 0x0000; 
    u_int blackblue     = 0x0000;
    u_int whitered      = 0xffff;
    u_int whitegreen    = 0xffff;
    u_int whiteblue     = 0xffff;

    wsp = wsAllocScreenInfo(index, pScreen);

    psn = wsp->screenDesc->screen;
    scrArgs = wsp->args;

    txProcessArguments (argc, argv);

    lastEventTime = CURRENT_TIME;

    /* since driver does not support unmap (yet), only map screen once */
    if (! mapped[psn]) {
	depthDesc[psn].screen = psn;
	depthIndex = -1;
	for (i = 0; i < wsp->screenDesc->allowed_depths; i++) {
	    extern int forceDepth;

	    depthDesc[psn].which_depth = i;	
	    if (ioctl(wsFd, GET_DEPTH_INFO, &depthDesc[psn]) == -1) {
		ErrorF("GET_DEPTH_INFO failed");
		exit (1);
	    }
	    if (forceDepth)
		depthDesc[psn].depth = forceDepth;
	    switch (depthDesc[psn].bits_per_pixel) {
	    case 1:
	    case 8:
	    case 16:
	    case 32:
		break;
	    default:
		continue;
	    }
	    depthIndex = i;
  	}
	if (depthIndex == -1) return FALSE;

	mc.screen = psn;
	mc.which_depth = depthIndex;
	mc.map_unmap = MAP_SCREEN;

	if (ioctl(wsFd,  MAP_SCREEN_AT_DEPTH, &mc) == -1)    {
	    ErrorF("MAP_SCREEN_AT_DEPTH failed");
	    free ((char *) wsp);
	    return FALSE;
	}
	/* 
	 * reget the depth desc.  It now contains the user-mapped bitmap
	 * addr. 
	 */
	if (ioctl(wsFd, GET_DEPTH_INFO, &depthDesc[psn]) == -1) {
	    ErrorF("GET_DEPTH_INFO failed");
	    return FALSE;
	}
	if ( forceDepth )
	    depthDesc[psn].depth = forceDepth;
	mapped[psn] = TRUE;
    }

    /* ws routines knows how to initialize many functions, so call init. */
    if (wsScreenInit(index, pScreen, argc, argv) == -1) 
	return FALSE;

    dd = &depthDesc[psn];

    /* This will get turned on appropriately in wsInitScreenFinish */
    wsCursorControl(psn, CURSOR_OFF);

/* 
 * this is really dumb.  The driver has the screen geometry in mm.
 * The screen wants it stored as mm, but the damn interface passes
 * inches.  mm => inches => mm.  What a waste.  Should we change cfbscrinit.c?
 * -jmg.
 */

    if (scrArgs->flags & ARG_DPIX)
	dpix = scrArgs->dpix;
    else
        dpix =  ((wsp->screenDesc->width * 254) +
                 (wsp->screenDesc->monitor_type.mm_width * 5) ) /
                 (wsp->screenDesc->monitor_type.mm_width  * 10);

    if (scrArgs->flags & ARG_DPIY)
	dpiy = scrArgs->dpiy;
    else
        dpiy =  ((wsp->screenDesc->height * 254) +
                 (wsp->screenDesc->monitor_type.mm_height * 5) ) /
                 (wsp->screenDesc->monitor_type.mm_height * 10);

    defaultVisualClass = wsDefaultColorVisualClass(pScreen);

    defaultRootDepth = (defaultVisualClass == TrueColor
			|| defaultVisualClass == DirectColor) ? 24 : 8;
    useXorFix = TRUE;
    useBankSwitch = BANK_SW_DEFAULT;
    useBStore = TRUE;

#if LONG_BIT == 64 && defined(__osf__)
    rop_pip_init(psn, dd->pixmap, dd->plane_mask, useBankSwitch);
#else
    rop_pip_init(psn, dd->pixmap, useBankSwitch);
#endif

    if (!xfbScreenInit(pScreen, ropInfo[psn].bit8,
	wsp->screenDesc->width, wsp->screenDesc->height, dpix, dpiy,
	dd->fb_width, ropInfo[psn].bit24,
	defaultRootDepth, defaultVisualClass,
	useBankSwitch, useXorFix, useBStore)) {
	return (FALSE);
    }

    /* init hardware colormap procs: */
    if (!ropColorInit(pScreen)) {
	return (FALSE);
    }

    if(scrArgs->flags & ARG_BLACKVALUE)
	colorNameToColor(scrArgs->blackValue, &blackred,
			 &blackgreen, &blackblue); 

    if(scrArgs->flags & ARG_WHITEVALUE)
	colorNameToColor(scrArgs->whiteValue, &whitered, 
			&whitegreen, &whiteblue);


    /* copy of cfbCreateDefColormap, except variable colors (copied from
	R5-beta):
    */
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;

    if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &pCmap,
		       (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
		       0)
	!= Success)
	return FALSE;
    if ((AllocColor(pCmap, &whitered, &whitegreen, &whiteblue,
		    &(pScreen->whitePixel), 0) != Success) ||
	(AllocColor(pCmap, &blackred, &blackgreen, &blackblue,
		    &(pScreen->blackPixel), 0) != Success))
    {
	return FALSE;
    }
    (*pScreen->InstallColormap)(pCmap);

    /* Wrap screen close routine to avoid memory leak */
    wsp->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = wsScreenClose;

    planemask_addr = dd->plane_mask;
    if(wsp->screenDesc->cursor_width == 64)
	 pScreen->QueryBestSize = wsQueryBestSize64;
    else
	 pScreen->QueryBestSize = wsQueryBestSize16;

    return TRUE;
}
   
/**********************************************************
	PIP Support 
***********************************************************/
/*
 *      Frame buffer access indices:
 */
#define ACCESS_EIGHT_BIT        1
#define ACCESS_SELECTION        2
#define ACCESS_TRUE_COLOR       3
#define ACCESS_VIDEO_ENABLE     4

#define ACCESS_AS_WORDS         0x80
#define ACCESS_AS_HALF_WORDS    0x40
#define ACCESS_AS_BYTES         0x20
#define ACCESS_AS_BITS          0x10

#define ACCESS_TYPE_MASK        0x0f
#define ACCESS_WIDTH_MASK       0xf0

/*
 *      Mapping register related macros and definitions:
 *
 *        MAP_IS_ON:
 *              Test if mapping hardware is turned on.
 *        MAP_3MAX_LOW_BITS:
 *      Low order bits (below mapping register level.)
 *        MAP_VALUE:
 *              Mapping register value.
 *        SET_MAP:
 *              Set the mapping value in the map register.
 */
#define MAP_IS_ON(map_reg)  ((*map_reg & 0x10) == 0)
#define MAP_3MAX_LOW_BITS 0x0001fffff
#define MAP_VALUE( addr ) (((addr) & 0x00e00000) >> 20)
#define SET_MAP(map_reg, value) *(map_reg) = \
        (*(map_reg) & 0xfffffff0) | TC1_FBC_VIDEO_ON | ((value) & 0x0000000f)

/* DCC MOD --- ADDED GET_MAP --- 11.06.91 */
#define GET_MAP(map_reg) (*(map_reg) & 0x0000000f)
/* END DCC MOD */

typedef union {         /* Pointer access aliases: */
        u_char  *byte;          /* ... access pixel as an 8-bit byte. */
        u_short *half_word;     /* ... access pixel as a 16-bit word. */
        u_int   integer;        /* ... unsigned integer for calculations. */
        u_int   *word;          /* ... access pixel as a 32-bit word. */
} Access;


/*
 *	lib3max.c:	Diagnostic support routines for peeking and poking
 *				Turbo Bus locations on a 3MAX.
 *
 *		Copyright(c) 1990, RasterOps, Inc.
 */
/* These are already included above or are unnecessary
#include <sys/types.h>
#include <sys/file.h>
#include "/sys/h/ioctl.h"
*/

#define CARRY 0x10000

/*
 *	Bit access definitions:
 */
static int	bit_get[] = {		/* Mask for getting bit within byte. */
	0x1, 0x2, 0x4, 0x8, 		/* ... */
	0x10, 0x20, 0x40, 0x80  	/* ... */
};								/* ... */
static int	bit_clear[] = {		/* Mask for clearing all but 1 bit within a byte. */
	~0x1, ~0x2, ~0x4, ~0x8, 	/* ... */
	~0x10, ~0x20, ~0x40, ~0x80 	/* ... */
};								/* ... */
static int	bit_shift[] = {		/* Shift to move select bit to bit 0. */
	0, 1, 2, 3, 				/* ... */
	4, 5, 6, 7,					/* ... */
};								/* ... */

/*"fill_eight_bit"
 *
 *	Fill a rectangular region in the 8-bit memory.
 */
fill_eight_bit(prop, left, top, width, height, color)
     RopPtr prop;	/* -> base address of turbo bus slot. */
     int left;		/* =  left side of region to fill in. */
     int top;		/* =  top side of region to fill in. */
     int width;		/* =  width of region to fill in. */
     int height;	/* =  height of region to fill in. */
     u_int color;	/* =  color to fill pixels with. */
{
  u_int	fb;		/* Address of frame buffer. */
  u_char *map_reg;	/* Location of mapping register. */
  u_char  *pixel;	/* Next pixel to fill with 'color'. */
  int x;		/* Index: next column to set pixel value in. */
  int x_end;		/* End of strip to be filled in x direction. */
  int y;		/* Index: next row to set pixel value in. */
  int y_end;		/* End of region to be in filled in y direction. */
  int old_map;		/* Save and restore map register */

  /*
   *	Set up mapping register and frame buffer addresses, 
   *    set mapping register value.
   *	Set loop check values and then loop filling in rows of the 
   *    frame buffer.
   */

  map_reg = prop->mapRegister;
  fb = (u_int)prop->bit8;

  if ( MAP_IS_ON(map_reg) ) 
    {
      old_map = GET_MAP(map_reg);
      SET_MAP(map_reg, MAP_VALUE(TCO_EIGHT_BIT));
    }
  
  x_end = left + width;
  y_end = top + height;
  
  for ( y = top; y < y_end; y++ )
    {
      pixel = (u_char *)( fb + (left + y * 1280) );
      for ( x = left; x < x_end; x++ )
	{
	  *pixel++ = color;
	}
    }

  if ( MAP_IS_ON(map_reg) )
    {
      SET_MAP(map_reg, old_map);
    }

}

/*"fill_video_enable"
 * 
 *  Fill a rectangular region in the video enable memory.
 */
fill_selection(prop, x1, y1, width, height, color)
     RopPtr	prop;		/* -> base address of turbo bus slot. */
     int     	x1;       /* =  left side of region to fill in. */
     int     	y1;        /* =  top side of region to fill in. */
     int     	width;      /* =  width of region to fill in. */
     int     	height;     /* =  height of region to fill in. */
     u_int   	color;      /* =  color to fill pixels with. */
{
  u_int   *fb;          /* Address of frame buffer. */
  int pitch;    	/* Number of bytes between scan lines. */
  u_int   *pixel;       /* Next set of 8 pixels to access in the fb. */
  u_int   startmask, endmask;
  int	    nmiddle, nmid;
  int	    x2, y2;
  int	    c;
  u_char *map_reg;		/* Location of mapping register. */
  int old_map;
  
  fb = (u_int *)(prop->SelectionPlane);

  color = color & 0xff;
  color = color | color << 8;
  color = color | color << 16;
  
#define ROP_PPW	32
#define ROP_PIM	(ROP_PPW - 1)
#define ROP_PWSH	5
#define ROP_SCRRIGHT(bits,shift)	(((unsigned int) (bits)) << (shift))
#define ROP_SCRLEFT(bits,shift)	(((unsigned int) (bits)) >> (shift))
  
  x2 = x1 + width;
  y2 = y1 + height;
  
  /* COMPENSATE FOR FUNNY PIXEL PACKING */

  x1 = (x1 & 7) + ((x1 >> 3) * 32);
  x2 = (x2 & 7) + ((x2 >> 3) * 32);
  width = x2 - x1;

  pitch = 1280 >> 3;
  
  c = x1 & ROP_PIM;
  
  pixel = fb + (y1 * pitch) + (x1 >> ROP_PWSH);
  startmask = 0;
  if (c + width < ROP_PPW) {
    nmiddle = 0;
    endmask = ROP_SCRRIGHT(~0,c) ^ ROP_SCRRIGHT(~0,c+width);
  } else {
    nmiddle = width;
    if (c) {
      startmask = ROP_SCRRIGHT(~0,c);
      nmiddle += (c - ROP_PPW);
      pitch--;
    }
    nmiddle >>= ROP_PWSH;
    endmask = ~ROP_SCRRIGHT(~0, x2 & ROP_PIM);
    pitch -= nmiddle;
  }
  while (height--) {
    if (startmask) {
      *pixel = *pixel & ~startmask | color & startmask;
      pixel++;
    }
    nmid = nmiddle;
    while (nmid--) {
      *pixel = color;
      pixel++;
    }
    if (endmask)
      *pixel = *pixel & ~endmask | color & endmask;
    pixel += pitch;
  }
}

#ifdef nomore

/* DCC MOD --- RESTORES THE ADDRESS MAP UPON RETURN --- 11.06.91 */

/*"fill_selection"
 *
 *	Fill a rectangular region in the selection memory. (Note this memory
 *	does not require a setting of the mapping register!)
 *
 * (edg 7/26/91) Rewrote routine and based it on MFBSOLIDFILLAREA() to make
 * things much faster.  The old code was rather inefficient.
 */
fill_selection(prop, left, top, width, height, color)
    RopPtr	prop;		/* -> base address of turbo bus slot. */
    int		left;		/* =  left side of region to fill in. */
    int		top;		/* =  top side of region to fill in. */
    int		width;		/* =  width of region to fill in. */
    int		height;		/* =  height of region to fill in. */
    u_int	color;		/* =  color to fill pixels with. */
{
#define nlwidth 1280/8  /* number of longwords per scanline */
    u_int startmask;	/* bits to modify for left ragged edge */
    u_int endmask;	/* bits to modify for right ragged edge */
    u_int startbits, endbits;
    int index, nlwMiddle, nlw, nlwExtra;
    u_int *p;

    /* p = adr of select memory + row offset + column offset: */
    p = (u_int *)(prop->SelectionPlane) + (top * nlwidth) + (left >> 3);

    index = left & 0x7;  /* bit index into word */

    if ((index + width) < 8) {
	/* paint a narrow strip with ragged left and right edges */
	startmask = ~((1 << index) - 1);  /* set up left edge */
	startmask &= (1 << (index + width)) - 1;  /* fix up right edge */
	startbits = startmask & color;
	Duff(height, *p = *p & ~startmask | startbits; p += nlwidth);
    } else {
	startmask = (index == 0) ? 0 : ~((1 << index) - 1);
	endmask = (1 << ((left + width) & 0x7)) - 1;
	nlwMiddle = (startmask) ? ((width - (8 - index)) >> 3) : (width >> 3);
	nlwExtra = nlwidth - nlwMiddle;
	startbits = color & startmask;
	endbits = color & endmask;

	if (startmask && endmask) {
	    nlwExtra -= 1;
	    while (height--) {
		nlw = nlwMiddle;
		*p = *p & ~startmask | startbits;
		p++;
		Duff(nlw, *p++ = color);
		*p = *p & ~endmask | endbits;
		p += nlwExtra;
	    }
	} else if (startmask && !endmask) {
	    nlwExtra -= 1;
	    while (height--) {
		nlw = nlwMiddle;
		*p = *p & ~startmask | startbits;
		p++;
		Duff(nlw, *p++ = color);
		p += nlwExtra;
	    }
	} else if (!startmask && endmask) {
	    while (height--) {
		nlw = nlwMiddle;
		Duff(nlw, *p++ = color);
		*p = *p & ~endmask | endbits;
		p += nlwExtra;
	    }
	} else /* no ragged bits at either end */ {
	    while (height--) {
		nlw = nlwMiddle;
		Duff(nlw, *p++ = color);
		p += nlwExtra;
	    }
	}
    }
}

#endif nomore

/*"fill_true_color"
 *
 *	Fill a rectangular region in the 24-bit true color memory.
 */
fill_true_color(prop, left, top, width, height, color)
     RopPtr	prop;		/* -> base address of turbo bus slot. */
     int	left;		/* =  left side of region to fill in. */
     int	top;		/* =  top side of region to fill in. */
     int	width;		/* =  width of region to fill in. */
     int	height;		/* =  height of region to fill in. */
     u_int	color;			/* =  color to fill pixels with. */
{
  u_int	fb_mapped; /* Address of frame buffer in 3max or 3min address space. */
  u_int	fb_real;   /* Address of frame buffer  in real (non-3max) space. */
  u_int	map_left;  /* Mapping register value at left end of line. */
  u_int	map_right; /* Mapping register value at right end of line. */
  u_char *map_reg; /* Location of mapping register. */
  u_int	*pixel;	/* Where to store next pixel value (is map or = pixel_real.) */
  u_int	pixel_offset; /* Offset of pixel within turbo bus slot. */
  int x;	/* Loop index: next column to set pixel in. */
  int x_end;	/* X coordinate to stop filling with. */
  int y;	/* Loop index: next row to set pixel in. */
  int y_end;	/* Y coordinate to stop filling with. */
  int old_map;	/* to save and restore map <dcc> */
  
  /*
   *	Set up mapping register and frame buffer addresses. Set frame buffer
   *	offset within turbo bus slot. Set loop comparison values.
   */
  map_reg = prop->mapRegister;
  fb_mapped = (u_int)prop->MappedArea;
  fb_real = (u_int)prop->bit24;
  x_end = left+width;
  y_end = top+height;
  
  if ( MAP_IS_ON(map_reg) )
    {
      old_map = GET_MAP(map_reg);
    }

  /*
   *	Loop filling scan lines. For each scan line:
   *	  (1) If no mapping just fill the pixels.
   *
   *	  (2) If mapping (i.e., 3MAX) calculate the starting and ending 
   *		  map register values:i
   *	        (a) If they are the same just fill the line. 
   *		(b) If the map register value changes on the line then set 
   *				it before each pixel.
   */
  for ( y = top; y < y_end; y++ )
    {
      if ( !MAP_IS_ON(map_reg) )
	{
	  pixel = (u_int *)(fb_real + 4 * (left + y * 1280));
	  for ( x = left; x < x_end; x++ )
	    {
	      *pixel++ = color;
	    }
	}
      
      else
	{
	  pixel_offset = TCO_TRUE_COLOR + 4 * (left + y * 1280);
	  map_left = MAP_VALUE(pixel_offset);
	  map_right = MAP_VALUE( pixel_offset + width * 4 );
	  if ( (map_left == map_right) )
	    {
	      SET_MAP(map_reg, map_left);
	      pixel = (u_int *)(fb_mapped | (pixel_offset&MAP_3MAX_LOW_BITS));
	      for ( x = left; x < x_end; x++ )
		{
		  *pixel++ = color;
		}
	    }
	  else
	    {
	      for ( x = left; x < x_end; x++ )
		{
		  SET_MAP(map_reg, MAP_VALUE(pixel_offset));
		  pixel = (u_int *)(fb_mapped | (pixel_offset&MAP_3MAX_LOW_BITS));
		  *pixel = color;
		  pixel_offset+=4;
		}
	    }
	}
    }
  
  if ( MAP_IS_ON(map_reg) )
    {
      SET_MAP(map_reg, old_map);
    }

}


/*"fill_video_enable"
 * 
 *  Fill a rectangular region in the video enable memory.
 */
fill_video_enable(prop, x1, y1, width, height, color)
     RopPtr	prop;		/* -> base address of turbo bus slot. */
     int     	x1;       /* =  left side of region to fill in. */
     int     	y1;        /* =  top side of region to fill in. */
     int     	width;      /* =  width of region to fill in. */
     int     	height;     /* =  height of region to fill in. */
     u_int   	color;      /* =  color to fill pixels with. */
{
  u_int   *fb;          /* Address of frame buffer. */
  int pitch;    	/* Number of bytes between scan lines. */
  u_int   *pixel;       /* Next set of 8 pixels to access in the fb. */
  u_int   startmask, endmask;
  int	    nmiddle, nmid;
  int	    x2, y2;
  int	    c;
  u_char *map_reg;		/* Location of mapping register. */
  int old_map;
  
  /*
   *	Set up mapping register and frame buffer addresses, 
   *    set mapping register value if
   *	mapping is enabled. Initialize loop termination values.
   */
  map_reg = prop->mapRegister;
  if ( MAP_IS_ON(map_reg) )
    {
      fb = (u_int *)prop->MappedArea;
      old_map = GET_MAP(map_reg);
      SET_MAP(map_reg, MAP_VALUE(TCO_VIDEO_ENABLE));
    }
  else
    {
      fb = (u_int *)prop->VideoEnablePlane;
    }
  
  color = color & 0xff;
  color = color | color << 8;
  color = color | color << 16;
  
#define ROP_PPW	32
#define ROP_PIM	(ROP_PPW - 1)
#define ROP_PWSH	5
#define ROP_SCRRIGHT(bits,shift)	(((unsigned int) (bits)) << (shift))
#define ROP_SCRLEFT(bits,shift)	(((unsigned int) (bits)) >> (shift))
  
  x2 = x1 + width;
  y2 = y1 + height;
  
  /* COMPENSATE FOR FUNNY PIXEL PACKING */

  x1 = (x1 & 7) + ((x1 >> 3) * 32);
  x2 = (x2 & 7) + ((x2 >> 3) * 32);
  width = x2 - x1;

  pitch = 1280 >> 3;
  
  c = x1 & ROP_PIM;
  
  pixel = fb + (y1 * pitch) + (x1 >> ROP_PWSH);
  startmask = 0;
  if (c + width < ROP_PPW) {
    nmiddle = 0;
    endmask = ROP_SCRRIGHT(~0,c) ^ ROP_SCRRIGHT(~0,c+width);
  } else {
    nmiddle = width;
    if (c) {
      startmask = ROP_SCRRIGHT(~0,c);
      nmiddle += (c - ROP_PPW);
      pitch--;
    }
    nmiddle >>= ROP_PWSH;
    endmask = ~ROP_SCRRIGHT(~0, x2 & ROP_PIM);
    pitch -= nmiddle;
  }
  while (height--) {
    if (startmask) {
      *pixel = *pixel & ~startmask | color & startmask;
      pixel++;
    }
    nmid = nmiddle;
    while (nmid--) {
      *pixel = color;
      pixel++;
    }
    if (endmask)
      *pixel = *pixel & ~endmask | color & endmask;
    pixel += pitch;
  }

  if ( MAP_IS_ON(map_reg) )
    {
      SET_MAP(map_reg, old_map);
    }
}

#ifdef nomore

/*"fill_video_enable"
 *
 *	Fill a rectangular region in the video enable memory. 
 */
fill_video_enable(prop, left, top, width, height, color)
     RopPtr	prop;		/* -> base address of turbo bus slot. */
     int	left;		/* =  left side of region to fill in. */
     int	top;		/* =  top side of region to fill in. */
     int	width;		/* =  width of region to fill in. */
     int	height;		/* =  height of region to fill in. */
     u_int	color;		/* =  color to fill pixels with. */
{
  u_int	clear_mask;		/* Mask used to clear a bit in the memory. */
  u_int	fb;			/* Address of frame buffer. */
  u_int	get_mask;		/* Mask used to set a bit in the memory. */
  u_char *map_reg;		/* Location of mapping register. */
  u_int	*pixel;			/* Next set of 8 pixels to access in the fb. */
  int x;			/* Index: next column to set pixel value in. */
  int x_end_left;   /* Left end of strip to be filled a bit at a time */
  int x_end_middle; /* Middle of strip to be filled which has all 8 pixels */
  int x_end_right;  /* Right end of strip to be filled a bit at a time */
  int y;			/* Index: next row to set pixel value in. */
  int y_end;		/* End of region to be in filled in y direction. */
  int old_map;
  
  /*
   *	Set up mapping register and frame buffer addresses, 
   *    set mapping register value if
   *	mapping is enabled. Initialize loop termination values.
   */
  map_reg = prop->mapRegister;
  if ( MAP_IS_ON(map_reg) )
    {
      fb = (u_int)prop->MappedArea;
      old_map = GET_MAP(map_reg);
      SET_MAP(map_reg, MAP_VALUE(TCO_VIDEO_ENABLE));
    }
  else
    {
      fb = (u_int)prop->VideoEnablePlane;
    }
  
  x_end_right = left+width;
  x_end_middle = (x_end_right/8)*8;
  x_end_left = ((left+7)/8)*8;
  if ( x_end_left > x_end_right ) x_end_left = x_end_right;
  y_end = top+height;
  
  /*
   *	Fill left strip (left side pixels not in a full byte.)
   */
  for ( x = left; x < x_end_left; x++ )
    {
      pixel = (u_int *)(fb + 4 * (x/8 + top * (1280/8)));
      clear_mask = bit_clear[x%8];
      get_mask = bit_get[x%8];
      for ( y = top; y < y_end; y++ )
	{
	  *pixel = (*pixel & clear_mask) | (color & get_mask);
	  pixel += (1280/8);
	}
    }
  
  /*
   *	Fill the middle strip (pixels which are in full bytes.)
   */
  for ( ; x < x_end_middle; x+= 8 )
    {
      pixel = (u_int *)(fb + 4 * (x/8 + top*(1280/8)));
      for ( y = top; y < y_end; y++ )
	{
	  *pixel = color;
	  pixel += (1280/8);
	}
    }
  
  /*
   *	Fill in right strip (right side pixels not in a full byte.)
   */
  for ( ; x < x_end_right; x++ )
    {
      pixel = (u_int *)(fb + 4 * (x/8 + top * (1280/8)));
      clear_mask = bit_clear[x%8];
      get_mask = bit_get[x%8];
      for ( y = top; y < y_end; y++ )
	{
	  *pixel = (*pixel & clear_mask) | (color & get_mask);
	  pixel += (1280/8);
	}
    }

  if ( MAP_IS_ON(map_reg) )
    {
      SET_MAP(map_reg, old_map);
    }
}

#endif nomore

/* END DCC MOD */

#ifdef NOTDEF

/*"map_slot"
 *
 *	Map in an Turbo Bus slot so it can be accessed. 
 *
 *		-> base of turbo bus slot if success
 *		=  0 if failure
 */
caddr_t map_slot()
{
	caddr_t	address;	/* Address of mapped slot. */
	int		fd;			/* File descriptor for slot being mapped. */

	if ( (fd = open("/dev/console", O_RDWR)) < 0 )
	{
		return 0;
	}
	if ( ioctl(fd, GET_RASTEROPS_SLOT, &address) == -1 )
	{
		printf("map_slot: unable to map slot!\n");
		return 0;
	}
	return address;
}
#endif NOTDEF

#ifndef __alpha 
/**************************************
 **************************************
 **************************************
 **************************************
 **  This code is unused.	     **
 **************************************
 **************************************
 **************************************
 **************************************/
/*"parse_access"
 *
 *	Parse the location and width of Turbo slot access to be made given
 *	the command line arguments. The parameters passed to this routine
 *	are the same ones received by 'main' from the system (e.g., argv[0]
 *	is the command name.) 
 *	Default access widths by frame buffer type are:
 *		Eight bit:    byte
 *		Selection:    bit 
 *		True Color:   word
 *		Video enable: bit 
 *	Default frame buffer to access is eight bit.
 *
 * 		= access to be made (see diag.h)
 *		= 0 if an error was detected
 */
parse_access(argc, argv)
	int		argc;		/* = number of command arguments to examine. */
	char	*argv[];	/* = vector of pointers to command arguments. */
{
	int		argv_i;		/* Index: entry in argv currently examining. */
	int		char_i;		/* Index: character within argv[argv_i] examining. */
	int		fb;			/* Frame buffer access is for. */
	int		width;		/* Width of access. */

	/*
	 *	Process command arguments looking for access specifications.
	 */
	fb = 0;
	width = 0;
	for ( argv_i = 1; argv_i < argc; argv_i++ )
	{
		if ( *argv[argv_i] == '-' )
		{
			for ( char_i = 1; argv[argv_i][char_i]; char_i++ )
			{
				switch ( argv[argv_i][char_i] )
				{
				  case 'b':
				  case '1':
					width = ACCESS_AS_BYTES;
				  case 'e':
					fb = ACCESS_EIGHT_BIT;
					break;
				  case 'h':
				  case '2':
					width = ACCESS_AS_HALF_WORDS;
					break;
				  case 's':
					fb = ACCESS_SELECTION;
					break;
				  case 't':
					fb = ACCESS_TRUE_COLOR;
					break;
				  case 'v':
					fb = ACCESS_VIDEO_ENABLE;
					break;
				  case 'w':
				  case '4':
					width = ACCESS_AS_WORDS;
					break;
				}
			}
		}
	}

	/*
	 *	Perform error checking and default access width if it was not
	 *	specified.
	 */
	switch( fb )
	{
	  default:
		fb = ACCESS_EIGHT_BIT;
	  case ACCESS_EIGHT_BIT:
		if ( width == 0 ) width = ACCESS_AS_BYTES;
		break;
	  case ACCESS_SELECTION:
		if ( width == 0 ) width = ACCESS_AS_BITS;
		break;
	  case ACCESS_TRUE_COLOR:
		switch ( width )
		{
		  case ACCESS_AS_BYTES:
		  case ACCESS_AS_HALF_WORDS:
			printf(" *** Access to true color frame buffer must be on word boundaries\n");
			return 0;
		  default:
			width = ACCESS_AS_WORDS;
			break;
		}
		break;
	  case ACCESS_VIDEO_ENABLE:
		if ( width == 0 ) width = ACCESS_AS_BITS;
		break;
	}

	return fb | width;
}

/*"parse_access_help"
 *
 *	Print out description of access command modifiers, which are parsed
 *	by parse_access(). (This routine is called by the various aaa_map() 
 *	routines in the diagnostic utilities.
 */
parse_access_help()
{
	printf("   -b indicates byte access\n");
	printf("   -h indicates halfword (2 byte) access\n");
	printf("   -e indicates 8-bit frame buffer\n");
	printf("   -s indicates selection memory\n");
	printf("   -t indicates 24-bit frame buffer\n");
	printf("   -v indicates video enable memory\n");
	printf("   -w indicates word (4 byte) access\n");
}

/*"peek"
 *
 *	Return the contents of a location in a Turbo Bus slot's
 *	address space.
 *
 *		= contents of the location
 */
u_int peek(base, offset, width)
	u_int	base;		/* -> base of turbo channel slot. */
	u_int 	offset;		/* -> location within turbo channel slot to get. */
	int		width;		/* =  size of location (1, 2, or 4). */
{
	Access	access;		/* Pointer aliases. */
	u_int	*map_reg;	/* Frame buffer control register address. */

	/*
	 *	The 3MAX implementation uses a mapping register, which maps
	 *	addresses between 0x200000 and 0x3fffff to the range 'offset'
	 *	through 'offset'+0x1fffff. Set the mapping register to 
	 *	map the correct range, and set the reference address for the
	 *	access to be within the range base+0x000000 to base+0x3fffff.
	 */
	map_reg = (u_int *)(base + TCO_MAP_REGISTER);
	if ( !MAP_IS_ON(map_reg) || (offset < 0x00200000) )
	{
		access.integer = base + offset;
	}
	else
	{
		SET_MAP(map_reg, MAP_VALUE(offset));
		access.integer = base + TCO_3MAX_MAP_AREA + (offset & 0x001fffff);
	}
		

	/*
	 *	Perform the access based on the width required:
	 */
	switch( width )
	{
	  case 1:
		return (u_int)*access.byte;
	  case 2:
		return (u_int)*access.half_word;
	  case 4:
		return (u_int)*access.word;
	  default:
		printf("peek: Unknown width specification %d encountered\n", width);
		return 0;
	}
}

/*"pixel_get"
 *
 *	Get the value of a pixel given the offset of its frame buffer,
 *	and its ordinal number within its frame buffer. 
 *
 *		= value of pixel
 */
u_int pixel_get(base, fb_offset, pixel_offset)
	u_int	base;			/* -> base of turbo channel slot virt. addr. */ 
	u_int	fb_offset;		/* =  location within turbo channel slot of fb. */
	u_int	pixel_offset;	/* =  number within frame buffer of pixel. */
{
	Access		access;			/* Pointer alias for accessing pixel on Turbo bus. */
	int			bit_number;		/* Number of bit within byte. */
	u_char		*map_reg;		/* Frame buffer control register address. */

	map_reg = (u_char *)(base + 0x40030);
	switch ( fb_offset )
	{
      case TCO_EIGHT_BIT:
        if ( MAP_IS_ON(map_reg) ) SET_MAP(map_reg, MAP_VALUE(TCO_EIGHT_BIT));
        access.integer = base + fb_offset + pixel_offset;
        return *access.byte;
      case TCO_SELECTION:
		access.integer = base + fb_offset + ((pixel_offset/8)*4);
		bit_number = pixel_offset % 8;
		return (*access.word & bit_get[bit_number]) >> bit_shift[bit_number];
      case TCO_TRUE_COLOR:
        if ( MAP_IS_ON(map_reg) ) 
        {    
            SET_MAP(map_reg, MAP_VALUE(fb_offset+pixel_offset*4));
            access.integer = base + TCO_3MAX_MAP_AREA + ((pixel_offset*4)&MAP_3MAX_LOW_BITS);
        }    
		else
		{
			access.integer = base + fb_offset + (pixel_offset*4); 
		}
        return *access.word;
      case TCO_VIDEO_ENABLE:
        if ( MAP_IS_ON(map_reg) ) 
        { 
            SET_MAP(map_reg, MAP_VALUE(TCO_VIDEO_ENABLE));
            access.integer = base + TCO_3MAX_MAP_AREA + ((pixel_offset/8)*4);
        }    
		else
		{
        	access.integer = base + fb_offset + ((pixel_offset/8)*4);
		}
        bit_number = pixel_offset % 8;
		return (*access.word & bit_get[bit_number]) >> bit_shift[bit_number];
	}
}

/*"pixel_set"
 *
 *	Set the value of a pixel given the offset of its frame buffer,
 *	and its ordinal number within its frame buffer. 
 *
 */
u_int pixel_set(base, fb_offset, pixel_offset, value)
	u_int	base;			/* -> base of turbo channel slot virt. addr. */ 
	u_int	fb_offset;		/* =  location within turbo channel slot of fb. */
	u_int	pixel_offset;	/* =  number within frame buffer of pixel. */
	u_int	value;			/* =  value for pixel. */
{
	Access		access;			/* Pointer alias for accessing pixel on Turbo bus. */
	int			bit_number;		/* Number of bit within byte. */
	u_char		*map_reg;		/* Frame buffer control register address. */

	map_reg = (u_char *)(base + TCO_MAP_REGISTER);
	switch ( fb_offset )
	{
	  case TCO_EIGHT_BIT:
		if ( MAP_IS_ON(map_reg) ) SET_MAP(map_reg, 2);
		access.integer = base + TCO_EIGHT_BIT + pixel_offset;
		*access.byte = value;
		break;
	  case TCO_SELECTION:
		access.integer = base + fb_offset + ((pixel_offset/8)*4);
		bit_number = pixel_offset % 8;
		*access.word = (*access.word & bit_clear[bit_number]) | 
			((value & 0x1)	<< bit_shift[bit_number]);
		break;
	  case TCO_TRUE_COLOR:
		if ( MAP_IS_ON(map_reg) ) 
		{
            SET_MAP(map_reg, MAP_VALUE(fb_offset+pixel_offset*4));
            access.integer = base + TCO_3MAX_MAP_AREA + ((pixel_offset*4)&MAP_3MAX_LOW_BITS);
        }    
		else
		{
			access.integer = base + fb_offset + (pixel_offset*4); 
		}
		*access.word = value;
		break;
	  case TCO_VIDEO_ENABLE:
		if ( MAP_IS_ON(map_reg) )
		{
			if ( MAP_VALUE(*map_reg) != MAP_VALUE(TCO_VIDEO_ENABLE) )
				SET_MAP(map_reg, MAP_VALUE(TCO_VIDEO_ENABLE));
			access.integer = base + TCO_3MAX_MAP_AREA + ((pixel_offset/8)*4);
		}
		else
		{
			access.integer = base + fb_offset + ((pixel_offset/8)*4);
		}
		bit_number = pixel_offset % 8;
		*access.word = (*access.word & bit_clear[bit_number]) | 
			((value & 0x1)	<< bit_shift[bit_number]);
		break;
	}
	return;
}

/*"poke"
 *
 *	Set the contents of a location in a Turbo Bus slot's
 *	address space.
 *
 */
u_int poke(base, offset, width, value)
	u_int	base;		/* -> base of turbo channel slot. */
	u_int 	offset;		/* -> location within turbo channel slot to set. */
	int		width;		/* =  size of location (1, 2, or 4). */
	u_int	value;		/* =  value to place in location. */
{
	Access	access;			/* Pointer aliases. */
	u_int	*map_reg;	/* Frame buffer control register address. */

	/*
	 *	The 3MAX implementation uses a mapping register, which maps
	 *	addresses between 0x200000 and 0x3fffff to the range 'offset'
	 *	through 'offset'+0x1fffff. Set the mapping register to 
	 *	map the correct range, and set the reference address for the
	 *	access to be within the range base+0x000000 to base+0x3fffff.
	 */
	map_reg = (u_int *)(base + TCO_MAP_REGISTER);
	if ( MAP_IS_ON(map_reg) && (offset >= TCO_3MAX_MAP_AREA) )
	{
		SET_MAP(map_reg, MAP_VALUE(offset));
		access.integer = base + TCO_3MAX_MAP_AREA + (offset & MAP_3MAX_LOW_BITS);
	}
	else
	{
		access.integer = base + offset;
	}
		
	/*
	 *	Perform the access based on the width required:
	 */
	switch( width )
	{
	  case 1:
		*access.byte = value;
		break;
	  case 2:
		*access.half_word = value;
		break;
	  case 4:
		*access.word = value;
		break;
	  default:
		printf("poke: Unknown width specification %d encountered\n", width);
	}
	return 0;
}
 
/*"vector"
 *
 *	Draw a vector given a slot base and starting offset, as well as the
 *	vector to be drawn. This vector generator always draws vectors in the positive
 *  v direction. The u direction of the vector is controlled by the pitch value.
 *  The reason a u,v coordinate notation is used is that the meaning of u and v
 *  will be different depending on the vector being drawn:
 *      if delta x >= delta y then u = x, v = y
 *      if delta x <  delta y then u = y, v = x
 */
vector(base, fb_offset, color, xa, ya, xb, yb)
	u_int	base;			/* -> start of turbo bus slot. */
	u_int	fb_offset;		/* =  offset within slot of frame buffer. */
    int     color;          /* =  color to write vector as. */
	int		xa;				/* =  x component of vector start. */
	int		ya;				/* =  y component of vector start. */
	int		xb;				/* =  x component of vector end. */
	int		yb;				/* =  y component of vector end. */
{
	
    int     abs_dx;         /* ||xb - xa||. */
    int     abs_dy;         /* ||yb - ya||. */
	Access	access;			/* Pointer alias for referencing fb. */
    int     dx;             /* xb - xa. */
    int     dy;             /* yb - ya. */
    int     length;         /* Max(delta_x, delta_y). */
    int     pixel_offset;   /* Offset in pixels to draw next vector point at. */
    int     old_v_carry;    /* Value of carry bit prior to adding slope. */
    int     slope;          /* delta v / delta u. */
    int     start;          /* Starting offset for the vector. */
    int     u_increment;    /* Increment to offset for u movements. */
    int     uv_increment;   /* Increment to offset for simulatneous u,v movements. */
    int     v;              /* Current v position relative to original start offset. */
 
    dx = xb - xa;
    dy = yb - ya;
    abs_dx = dx >= 0 ? dx : -dx;
    abs_dy = dy >= 0 ? dy : -dy;
 
    /*  Positive delta y: use xa,ya as the starting point of the vector.
     */
    if ( dy >= 0 )
    {
        start = xa + (ya * 1280);
        if ( abs_dx > abs_dy )
        {
            length = abs_dx+1;
            slope = CARRY * abs_dy / dx;
            u_increment = dx > 0 ? 1 : -1;
            uv_increment = 1280 + u_increment;
        }
        else if ( abs_dy != 0 )
        {
            length = abs_dy+1;
            slope = CARRY * abs_dx / dy;
            u_increment = 1280;
            uv_increment = dx > 0 ? u_increment+1 : u_increment-1;
        }
        else    /* xa and xb are the same point! */
        {
            return;
        }
 
    }
    /*  Negative delta y: use xb, yb as the starting point of the vector.
     */
    else
    {
        start = xb + (yb * 1280);
        if ( abs_dx > abs_dy )
        {
            length = abs_dx+1;
            slope = CARRY * abs_dy / dx;
            u_increment = dx < 0 ? 1 : -1;
            uv_increment = 1280 + u_increment;
        }
        else if ( abs_dy != 0 )
        {
            length = abs_dy+1;
            slope = CARRY * abs_dx / dy;
            u_increment = 1280;
            uv_increment = dx < 0 ? u_increment+1 : u_increment-1;
        }
    }
 
	/*	
	 *	Loop generating actual vector, based on which frame buffer 
	 *	is being accessed. 
     *	  (v = slope)
     */
	v = CARRY / 2;
	pixel_offset = start;
 
	while ( --length >= 0 )
	{
		pixel_set(base, fb_offset, pixel_offset, color);
		old_v_carry = v & CARRY;
		v += slope;
		if ( ( v & CARRY ) != old_v_carry )
		{
			pixel_offset += uv_increment;
		}
		else
		{
			pixel_offset += u_increment;
		}
	}
}

#endif /* unused code, __alpha */


static void
#if LONG_BIT == 64 && defined(__alpha)
rop_pip_init(psn, sparse_addr, dense_addr, useBankSwitch)
  int psn;
  unsigned char *sparse_addr, *dense_addr;
  Bool useBankSwitch;
{
    int i;
    ropStruct *rs;
    rs = &ropInfo[psn];
    rs->sparse_board 	= sparse_addr;
    rs->sparse_bit8 	= rs->sparse_board + TCO_EIGHT_BIT;
    rs->sparse_bit24 	= rs->sparse_board + TCO_TRUE_COLOR;
    rs->mapRegister 	= rs->sparse_board + TCO_MAP_REGISTER;
    rs->MappedArea 	= rs->sparse_board + TCO_3MAX_MAP_AREA;
    rs->dutyCycle 	= rs->sparse_board + TCO_DUTY_CYCLE;

    rs->board 		= dense_addr;
    rs->bit8 		= rs->board + TCO_EIGHT_BIT_DENSE;
    rs->bit24 		= rs->board + TCO_TRUE_COLOR_DENSE;
    rs->dense_board 	= rs->board;
    rs->dense_bit8 	= rs->bit8;
    rs->dense_bit24 	= rs->bit24;
    rs->SelectionPlane 	= rs->dense_board + TCO_SELECTION_DENSE;
    rs->VideoEnablePlane= rs->dense_board + TCO_VIDEO_ENABLE_DENSE;
    rs->pipRegisters 	= rs->dense_board + TCO_PIP_REGISTERS_DENSE;

    fill_selection(rs, 0, 0, 1280, 1024, 0x00);
    *rs->mapRegister = useBankSwitch ? 0x82 : 0x92;
}
#else
rop_pip_init(psn, addr, useBankSwitch)
  int psn;
  unsigned char *addr;
  Bool useBankSwitch;
{
    int i;
    ropStruct *rs;
    rs = &ropInfo[psn];
    rs->board = addr;
    rs->bit8 = rs->board + TCO_EIGHT_BIT;
    rs->bit24 = rs->board + TCO_TRUE_COLOR;
    rs->SelectionPlane = rs->board + TCO_SELECTION;
    rs->VideoEnablePlane = rs->board + TCO_VIDEO_ENABLE;
    rs->mapRegister = rs->board + TCO_MAP_REGISTER;
    rs->MappedArea = rs->board + TCO_3MAX_MAP_AREA;
    rs->pipRegisters = rs->board + TCO_PIP_REGISTERS;
    rs->dutyCycle = rs->board + TCO_DUTY_CYCLE;

    fill_selection(rs, 0, 0, 1280, 1024, 0x00);
    *rs->mapRegister = useBankSwitch ? 0x82 : 0x92;
}
#endif

#include "pixmapstr.h"
/*
Paint the region "pRegion" in the selection memory with the value "hwCmap".
*/
void
ropSetSelect(pScreen, pRegion, hwCmap)
    ScreenPtr	pScreen;
    RegionPtr	pRegion;
    int		hwCmap;
{
    int nbox, w, h;
    BoxPtr pbox;
    u_int fill;
    RopPtr prop;

    prop = &ropInfo[wsPhysScreenNum(pScreen)];
    fill = (hwCmap == 1) ? 0xff : 0x00;
    nbox = REGION_NUM_RECTS(pRegion);
    pbox = REGION_RECTS(pRegion);

    while (nbox--) {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;
	fill_selection(prop, pbox->x1, pbox->y1, w, h, fill);
	pbox++;
    }
}

/*
Inputs: bank = -1 for 0x2, 0 for 0x4, 1 for 0x6, 2 for 0x8, 3 for 0xA
*/
void
ropSetBank(pScreen, bank)
    ScreenPtr	pScreen;
    int		bank;
{
    unsigned char *mapReg = ropInfo[wsPhysScreenNum(pScreen)].mapRegister;

    if (bank < -1 || bank > 3) {
	bank = -1;
    }
    *mapReg = (*mapReg & 0xe0) | ((bank + 2) * 2);
}
