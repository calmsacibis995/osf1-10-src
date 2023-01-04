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
 * sunCG3C.c --
 *	Functions to support the sun CG3 board as a memory frame buffer.
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

#include    "sun.h"

#include    <sys/mman.h>
#include    <pixrect/memreg.h>
/*
#include    <sundev/cg4reg.h>
*/
#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"
#include    <struct.h>

/*-
 * The cg3 frame buffer is divided into several pieces.
 *	1) an array of 8-bit pixels
 *	2) a one-bit deep overlay plane
 *	3) an enable plane
 *	4) a colormap and status register
 *
 * XXX - put the cursor in the overlay plane
 */

#define CG3A_HEIGHT      900 
#define CG3A_WIDTH       1152
#define CG3B_HEIGHT	 768
#define CG3B_WIDTH	 1024

typedef struct cg3ac {
#ifdef sparc
	u_char mpixel[128*1024];		/* bit-per-pixel memory */
	u_char epixel[128*1024];		/* enable plane */
#endif
        u_char cpixel[CG3A_HEIGHT][CG3A_WIDTH];   /* byte-per-pixel memory */
} CG3AC, CG3ACRec, *CG3ACPtr;

typedef struct cg3bc {
#ifdef sparc
	u_char mpixel[128*1024];		/* bit-per-pixel memory */
	u_char epixel[128*1024];		/* enable plane */
#endif
        u_char cpixel[CG3B_HEIGHT][CG3B_WIDTH];   /* byte-per-pixel memory */
} CG3BC, CG3BCRec, *CG3BCPtr;

#define CG3AC_IMAGE(fb)      ((caddr_t)((fb)->cpixel))
#define CG3AC_IMAGELEN       (((sizeof (CG3AC) + 4095)/4096)*4096)
#define CG3BC_IMAGE(fb)      ((caddr_t)((fb)->cpixel))
#define CG3BC_IMAGELEN       (((sizeof (CG3BC) + 4095)/4096)*4096)

/*-
 *-----------------------------------------------------------------------
 * sunCG3CInit --
 *	Attempt to find and initialize a cg3 framebuffer
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
sunCG3CInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    if (!cfbScreenInit (pScreen, sunFbs[index].fb,
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

    sunSaveScreen (pScreen, SCREEN_SAVER_OFF);

    return cfbCreateDefColormap(pScreen);
}

/*-
 *-----------------------------------------------------------------------
 * sunCG3CProbe --
 *	Attempt to find and initialize a cg3 framebuffer
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
sunCG3CProbe (pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the sunFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         fd;
    struct fbtype fbType;
    pointer	fb;
    CG3ACPtr CG3ACfb;
    CG3BCPtr CG3BCfb;

    if ((fd = sunOpenFrameBuffer(FBTYPE_SUN3COLOR, &fbType, index, fbNum,
				 argc, argv)) < 0)
	return FALSE;

#ifdef	_MAP_NEW
    if (fbType.fb_width == CG3A_WIDTH) {
	if ((int)(CG3ACfb = (CG3ACPtr) mmap((caddr_t) 0,
	     CG3AC_IMAGELEN,
	     PROT_READ | PROT_WRITE,
	     MAP_SHARED | _MAP_NEW, fd, 0)) == -1) {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
	}
	fb = (pointer) CG3AC_IMAGE(CG3ACfb);
    }
    else if (fbType.fb_width == CG3B_WIDTH) {
	if ((int)(CG3BCfb = (CG3BCPtr) mmap((caddr_t) 0,
	     CG3BC_IMAGELEN,
	     PROT_READ | PROT_WRITE,
	     MAP_SHARED | _MAP_NEW, fd, 0)) == -1) {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
	}
	fb = (pointer) CG3BC_IMAGE(CG3BCfb);
    }
    else {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
    }
#else	_MAP_NEW
    if (fbType.fb_width == CG3A_WIDTH) {
	CG3ACfb = (CG3ACPtr) valloc(CG3AC_MONOLEN + 
	    CG3AC_ENBLEN + CG3AC_IMAGELEN);
	if (CG3ACfb == (CG3ACPtr) NULL) {
	    ErrorF("Could not allocate room for frame buffer.\n");
	    return FALSE;
	}

	if (mmap((caddr_t) CG3ACfb, CG3AC_MONOLEN + 
	    CG3AC_ENBLEN + CG3AC_IMAGELEN,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED, fd, 0) < 0) {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
	}
	fb = (pointer) CG3AC_IMAGE(CG3ACfb);
    }
    else if (fbType.fb_width == CG3B_WIDTH) {
	CG3BCfb = (CG3BCPtr) valloc(CG3BC_MONOLEN + 
	    CG3BC_ENBLEN + CG3BC_IMAGELEN);
	if (CG3BCfb == (CG3BCPtr) NULL) {
	    ErrorF("Could not allocate room for frame buffer.\n");
	    return FALSE;
	}

	if (mmap((caddr_t) CG3BCfb, CG3BC_MONOLEN + 
	    CG3BC_ENBLEN + CG3BC_IMAGELEN,
	    PROT_READ | PROT_WRITE,
	    MAP_SHARED, fd, 0) < 0) {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
	}
	fb = (pointer) CG3BC_IMAGE(CG3BCfb);
    }
    else {
	    Error("Mapping cg3c");
	    (void) close(fd);
	    return FALSE;
    }
#endif	_MAP_NEW

    sunFbs[index].fd = fd;
    sunFbs[index].info = fbType;
    sunFbs[index].fb = fb;
    sunSupportsDepth8 = TRUE;
    return TRUE;
}

/*ARGSUSED*/
Bool
sunCG3CCreate(pScreenInfo, argc, argv)
    ScreenInfo	  *pScreenInfo;
    int	    	  argc;
    char    	  **argv;
{
    return (AddScreen(sunCG3CInit, argc, argv) >= 0);
}
