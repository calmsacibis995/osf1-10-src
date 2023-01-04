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
/*-
 * sunCG4C.c --
 *	Functions to support the sun CG4 board as a memory frame buffer.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or MIT not be used in
advertising or publicity pertaining to distribution  of  the
software  without specific prior written permission. Sun and
M.I.T. make no representations about the suitability of this
software for any purpose. It is provided "as is" without any
express or implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#ifndef	lint
static char sccsid[] = "@(#)sunCG4C.c	1.4 6/1/87 Copyright 1987 Sun Micro";
#endif

#include    "sun.h"

#include    <sys/mman.h>
#include    <pixrect/memreg.h>
#include    <sundev/cg4reg.h>
#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"
#include    <struct.h>

/*-
 * The cg4 frame buffer is divided into several pieces.
 *	1) an array of 8-bit pixels
 *	2) a one-bit deep overlay plane
 *	3) an enable plane
 *	4) a colormap and status register
 *
 * XXX - put the cursor in the overlay plane
 */
#define	CG4_HEIGHT	900
#define	CG4_WIDTH	1152

typedef struct cg4c {
	u_char mpixel[128*1024];		/* bit-per-pixel memory */
	u_char epixel[128*1024];		/* enable plane */
	u_char cpixel[CG4_HEIGHT][CG4_WIDTH];	/* byte-per-pixel memory */
} CG4C, CG4CRec, *CG4CPtr;

#define CG4C_IMAGE(fb)	    ((caddr_t)((fb)->cpixel))
#define CG4C_IMAGEOFF	    ((off_t)0x0)
#define CG4C_IMAGELEN	    (((CG4_HEIGHT*CG4_WIDTH + 8191)/8192)*8192)
#define	CG4C_MONO(fb)	    ((caddr_t)(&(fb)->mpixel))
#define	CG4C_MONOLEN	    (128*1024)
#define	CG4C_ENABLE(fb)	    ((caddr_t)(&(fb)->epixel))
#define	CG4C_ENBLEN	    CG4C_MONOLEN

/*-
 *-----------------------------------------------------------------------
 * sunCG4CInit --
 *	Attempt to find and initialize a cg4 framebuffer used as mono
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Most of the elements of the ScreenRec are filled in. Memory is
 *	allocated for the frame buffer and the buffer is mapped. The
 *	video is enabled for the frame buffer...
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static Bool
sunCG4CInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    if (!cfbScreenInit (pScreen, CG4C_IMAGE((CG4CPtr)sunFbs[index].fb),
			sunFbs[index].info.fb_width,
			sunFbs[index].info.fb_height,
			monitorResolution, monitorResolution,
			sunFbs[index].info.fb_width))
    	return (FALSE);
    
    if (!sunScreenAllocate (pScreen))
	return FALSE;

    sunCGScreenInit (pScreen);

    if (!sunScreenInit (pScreen))
	return FALSE;

    sunSaveScreen( pScreen, SCREEN_SAVER_OFF );

    return cfbCreateDefColormap(pScreen);
}

/*-
 *--------------------------------------------------------------
 * sunCG4CSwitch --
 *      Enable or disable color plane 
 *
 * Results:
 *      Color plane enabled for select =0, disabled otherwise.
 *
 *--------------------------------------------------------------
 */
static void
sunCG4CSwitch (pScreen, select)
    ScreenPtr  pScreen;
    u_char     select;
{
    register int    *j, *end;
    CG4CPtr	    CG4Cfb;

    CG4Cfb = (CG4CPtr) sunFbs[pScreen->myNum].fb;

    j = (int *) CG4Cfb->epixel;
    end = j + (128 / sizeof (int)) * 1024;
    if (!select)                         
      while (j < end)
	*j++ = 0;
    else
      while (j < end)
	*j++ = ~0;
}

/*-
 *-----------------------------------------------------------------------
 * sunCG4CProbe --
 *	Attempt to find and initialize a cg4 framebuffer used as mono
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
Bool
sunCG4CProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the sunFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         fd;
    struct fbtype fbType;
    CG4CPtr	    CG4Cfb;

    if ((fd = sunOpenFrameBuffer(FBTYPE_SUN4COLOR, &fbType, index, fbNum,
				 argc, argv)) < 0)
	return FALSE;

#ifdef	_MAP_NEW
    if ((int)(CG4Cfb = (CG4CPtr) mmap((caddr_t) 0,
	     CG4C_MONOLEN + CG4C_ENBLEN + CG4C_IMAGELEN,
	     PROT_READ | PROT_WRITE,
	     MAP_SHARED | _MAP_NEW, fd, 0)) == -1) {
	Error("Mapping cg4c");
	(void) close(fd);
	return FALSE;
    }
#else	_MAP_NEW
    CG4Cfb = (CG4CPtr) valloc(CG4C_MONOLEN + CG4C_ENBLEN + CG4C_IMAGELEN);
    if (CG4Cfb == (CG4CPtr) NULL) {
	ErrorF("Could not allocate room for frame buffer.\n");
	return FALSE;
    }

    if (mmap((caddr_t) CG4Cfb, CG4C_MONOLEN + CG4C_ENBLEN + CG4C_IMAGELEN,
	     PROT_READ | PROT_WRITE,
	     MAP_SHARED, fd, 0) < 0) {
	Error("Mapping cg4c");
	(void) close(fd);
	return FALSE;
    }
#endif	_MAP_NEW

    sunFbs[index].fd = fd;
    sunFbs[index].info = fbType;
    sunFbs[index].fb = (pointer) CG4Cfb;
    sunFbs[index].EnterLeave = sunCG4CSwitch;
    sunSupportsDepth8 = TRUE;
    return TRUE;
}

Bool
sunCG4CCreate(pScreenInfo, argc, argv)
    ScreenInfo	  *pScreenInfo;
    int	    	  argc;
    char    	  **argv;
{
    int i;

    i = AddScreen(sunCG4CInit, argc, argv);
    if (i >= 0)
    {
	/* Now set the enable plane for screen 0 */
	sunCG4CSwitch(pScreenInfo->screens[i], i != 0);
	return TRUE;
    }
    return FALSE;
}
