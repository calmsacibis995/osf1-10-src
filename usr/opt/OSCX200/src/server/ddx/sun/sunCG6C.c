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
 * sunCG6C.c --
 *	Functions to support the sun CG6 board as a memory frame buffer.
 */

/****************************************************************/
/* Modified from  sunCG4C.c for X11R3 by Tom Jarmolowski	*/
/****************************************************************/

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

#include    "sun.h"

#ifdef FBTYPE_SUNFAST_COLOR
#include    <sys/mman.h>
#include    <pixrect/memreg.h>
#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"
#include    <cfb.h>
#include    <struct.h>

#define CG6_VBASE	0x70000000

#define CG6_IMAGE(fb)	    ((caddr_t)(&(fb)->cpixel))
#define CG6_IMAGEOFF	    ((off_t)0x16000)
#define CG6_GXOFF	    ((off_t)0)

/*-
 *-----------------------------------------------------------------------
 * sunCG6Init --
 *	Attempt to find and initialize a cg6 framebuffer
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
sunCG6CInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int	i;
    char    *pbits;
    int	    w, h;

    pbits =  (char *) (sunFbs[index].fb + CG6_IMAGEOFF);
    w = sunFbs[index].info.fb_width;
    h = sunFbs[index].info.fb_height;
    if (!cfbSetupScreen (pScreen, pbits, w, h,
			monitorResolution, monitorResolution, w))
	return FALSE;

#ifdef sparc
    if (!sunGXInit (pScreen, &sunFbs[index]))
	return FALSE;
#endif

    if (!cfbFinishScreenInit(pScreen, pbits, w, h,
			monitorResolution, monitorResolution, w))
	return FALSE;

    if (!sunScreenAllocate (pScreen))
	return FALSE;

    sunCGScreenInit (pScreen);

    if (!sunScreenInit (pScreen))
	return FALSE;

    sunSaveScreen (pScreen, SCREEN_SAVER_OFF);

    return cfbCreateDefColormap(pScreen);
}

/*-
 *-----------------------------------------------------------------------
 * sunCG6Probe --
 *	Attempt to find and initialize a cg6 framebuffer
 *
 * Results:
 *	TRUE if everything went ok. FALSE if not.
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped.
 *
 *-----------------------------------------------------------------------
 */
Bool
sunCG6CProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the sunFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         fd;
    struct fbtype fbType;
    int		pagemask, mapsize;
    int		imagelen;
    caddr_t	mapaddr;
    caddr_t	addr;

    if ((fd = sunOpenFrameBuffer(FBTYPE_SUNFAST_COLOR, &fbType, index, fbNum,
				 argc, argv)) < 0)
	return FALSE;

    pagemask = getpagesize() - 1;
    imagelen = CG6_IMAGEOFF + fbType.fb_width * fbType.fb_height;
    mapsize = (imagelen + pagemask) & ~pagemask;
    addr = 0;

#ifndef	_MAP_NEW
    addr = (caddr_t) valloc(mapsize);
    if (addr == (caddr_t) NULL) {
	ErrorF("Could not allocate room for frame buffer.\n");
	(void) close (fd);
	return FALSE;
    }
#endif	_MAP_NEW

    /*
     * Should always try to MAP_PRIVATE first.  We want to have a private
     * copy of the registers on context switching if possible.
     */
    mapaddr = (caddr_t) mmap((caddr_t) addr,
	     mapsize,
	     PROT_READ | PROT_WRITE,
	     MAP_PRIVATE, fd, CG6_VBASE);

    if (mapaddr == (caddr_t) -1)
    {
    	mapaddr = (caddr_t) mmap((caddr_t) addr,
	     	 mapsize,
	     	 PROT_READ | PROT_WRITE,
	     	 MAP_SHARED, fd, CG6_VBASE);
    }

    if (mapaddr == (caddr_t) -1) {
	Error("Mapping cg6c");
	(void) close(fd);
	return FALSE;
    }

    if (mapaddr == 0)
        mapaddr = addr;

    sunFbs[index].fd = fd;
    sunFbs[index].info = fbType;
    sunFbs[index].fb = (pointer) mapaddr;
    sunSupportsDepth8 = TRUE;
    return TRUE;
}

Bool
sunCG6CCreate(pScreenInfo, argc, argv)
    ScreenInfo	  *pScreenInfo;
    int	    	  argc;
    char    	  **argv;
{
    int i;

    i = AddScreen(sunCG6CInit, argc, argv);
    if (i >= 0)
	return TRUE;
    return FALSE;
}
#endif /* FBTYPE_SUNFAST_COLOR */
