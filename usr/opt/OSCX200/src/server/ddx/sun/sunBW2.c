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
 * sunBW2.c --
 *	Functions for handling the sun BWTWO board.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 * Copyright (c) 1987 by Adam de Boor, UC Berkeley
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
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
static char sccsid[] = "%W %G Copyright 1987 Sun Micro";
#endif

/*-
 * Copyright (c) 1987 by Sun Microsystems,  Inc.
 */

#include    "sun.h"
#include    "resource.h"

#include    <sys/mman.h>
#include    <sundev/bw2reg.h>

extern caddr_t mmap();

typedef struct bw2 {
    u_char	image[BW2_FBSIZE];          /* Pixel buffer */
} BW2, BW2Rec, *BW2Ptr;

typedef struct bw2hr {
    u_char	image[BW2_FBSIZE_HIRES];          /* Pixel buffer */
} BW2HR, BW2HRRec, *BW2HRPtr;

/*-
 *-----------------------------------------------------------------------
 * sunBW2Init --
 *	Attempt to find and initialize a bw2 framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Most of the elements of the ScreenRec are filled in.  The
 *	video is enabled for the frame buffer...
 *
 *-----------------------------------------------------------------------
 */
/*ARGSUSED*/
static Bool
sunBW2Init (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    if (!mfbScreenInit(pScreen,
		       sunFbs[index].fb,
		       sunFbs[index].info.fb_width,
		       sunFbs[index].info.fb_height,
		       monitorResolution, monitorResolution,
		       sunFbs[index].info.fb_width))
	return (FALSE);

    if (!sunScreenAllocate (pScreen) || !sunScreenInit (pScreen))
	return FALSE;

    pScreen->whitePixel = 0;
    pScreen->blackPixel = 1;

    /*
     * Enable video output...? 
     */
    (void) sunSaveScreen(pScreen, SCREEN_SAVER_OFF);

    return mfbCreateDefColormap(pScreen);
}

/*-
 *-----------------------------------------------------------------------
 * sunBW2Probe --
 *	Attempt to find and initialize a bw2 framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped. 
 *
 *-----------------------------------------------------------------------
 */

/*ARGSUSED*/
Bool
sunBW2Probe(pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the sunFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         fd;
    struct fbtype fbType;
    int		pagemask, mapsize;
    caddr_t	addr, mapaddr;

    if ((fd = sunOpenFrameBuffer(FBTYPE_SUN2BW, &fbType, index, fbNum,
				 argc, argv)) < 0)
	return FALSE;

    /*
     * It's not precisely clear that we have to round up
     * fb_size to the nearest page boundary but there are
     * rumors that this is a good idea and that it shouldn't
     * hurt anything.
     */
    pagemask = getpagesize() - 1;
    mapsize = (fbType.fb_size + pagemask) & ~pagemask;
    addr = 0;

#ifndef _MAP_NEW
    /*
     * If we are running pre-SunOS 4.0 then we first need to
     * allocate some address range for mmap() to replace.
     */
    if ((addr = (caddr_t) valloc(mapsize)) == 0) {
        ErrorF("Could not allocate room for frame buffer.\n");
        (void) close(fd);
        return FALSE;
    }
#endif _MAP_NEW

    /*
     * In SunOS 4.0 the standard C library mmap() system call
     * wrapper will automatically add a _MAP_NEW flag for us.
     * In pre-4.0 mmap(), success returned 0 but now it returns the
     * newly mapped starting address. The test for mapaddr
     * being 0 below will handle this difference correctly.
     */
    if ((mapaddr = (caddr_t) mmap(addr, mapsize,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)0)) == (caddr_t) -1) {
        Error("mapping BW2");
        (void) close(fd);
        return FALSE;
    }

    if (mapaddr == 0)
        mapaddr = addr;

    sunFbs[index].fb = (pointer)mapaddr;
    sunFbs[index].fd = fd;
    sunFbs[index].info = fbType;
    sunFbs[index].EnterLeave = NULL;
    return TRUE;
}

Bool
sunBW2Create(pScreenInfo, argc, argv)
    ScreenInfo	  *pScreenInfo;
    int	    	  argc;
    char    	  **argv;
{
    return (AddScreen(sunBW2Init, argc, argv) >= 0);
}
