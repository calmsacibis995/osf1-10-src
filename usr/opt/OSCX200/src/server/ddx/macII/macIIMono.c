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
/************************************************************ 
Copyright 1988 by Apple Computer, Inc, Cupertino, California
			All Rights Reserved

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies.

APPLE MAKES NO WARRANTY OR REPRESENTATION, EITHER EXPRESS,
OR IMPLIED, WITH RESPECT TO THIS SOFTWARE, ITS QUALITY,
PERFORMANCE, MERCHANABILITY, OR FITNESS FOR A PARTICULAR
PURPOSE. AS A RESULT, THIS SOFTWARE IS PROVIDED "AS IS,"
AND YOU THE USER ARE ASSUMING THE ENTIRE RISK AS TO ITS
QUALITY AND PERFORMANCE. IN NO EVENT WILL APPLE BE LIABLE 
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES RESULTING FROM ANY DEFECT IN THE SOFTWARE.

THE WARRANTY AND REMEDIES SET FORTH ABOVE ARE EXCLUSIVE
AND IN LIEU OF ALL OTHERS, ORAL OR WRITTEN, EXPRESS OR
IMPLIED.

************************************************************/
/*-
 * macIIMono.c --
 *	Functions for handling the macII video board with 1 bit/pixel.
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

#include    "macII.h"
#include    "resource.h"

/*-
 *-----------------------------------------------------------------------
 * macIIMonoSaveScreen --
 *	Disable the video on the frame buffer to save the screen.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Video enable state changes.
 *
 *-----------------------------------------------------------------------
 */
static Bool
macIIMonoSaveScreen (pScreen, on)
    ScreenPtr	  pScreen;
    int    	  on;
{
    if (on == SCREEN_SAVER_FORCER)
	SetTimeSinceLastInputEvent ();
    return FALSE;
}

/*-
 *-----------------------------------------------------------------------
 * macIIMonoInit --
 *	Initialize the macII framebuffer
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
Bool
macIIMonoInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    ColormapPtr pColormap;
    PixmapPtr   pPixmap;

    pScreen->SaveScreen = macIIMonoSaveScreen;
    pScreen->whitePixel = 0;
    pScreen->blackPixel = 1;

    if (!mfbScreenInit(pScreen,
			   macIIFbs[index].fb,
			   macIIFbs[index].info.v_right -
			   macIIFbs[index].info.v_left,
			   macIIFbs[index].info.v_bottom -
			   macIIFbs[index].info.v_top, 
			   macIIFbs[index].info.v_hres >> 16, 
			   macIIFbs[index].info.v_vres >> 16,
    			   macIIFbs[index].info.v_rowbytes * 8))
    {
	return (FALSE);
    }

    return (macIIScreenInit(pScreen) && mfbCreateDefColormap(pScreen));
}

/*-
 *-----------------------------------------------------------------------
 * macIIMonoProbe --
 *	Attempt to find and initialize a macII framebuffer
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Memory is allocated for the frame buffer and the buffer is mapped. 
 *
 *-----------------------------------------------------------------------
 */

Bool
macIIMonoProbe(pScreenInfo, index, fbNum, argc, argv)
    ScreenInfo	  *pScreenInfo;	/* The screenInfo struct */
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    int	    	  fbNum;    	/* Index into the macIIFbData array */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    int         i, oldNumScreens;

    if (macIIFbData[fbNum].probeStatus == probedAndFailed) {
	return FALSE;
    }

    if (macIIFbData[fbNum].probeStatus == neverProbed) {
	int         fd;
	fbtype fbType;

	if ((fd = macIIOpenFrameBuffer(FBTYPE_MACII, &fbType, index, fbNum,
				     argc, argv)) < 0) {
	    macIIFbData[fbNum].probeStatus = probedAndFailed;
	    return FALSE;
	}

	{
		static char *video_virtaddr = (char *) (120 * 1024 * 1024);
		struct video_map vmap;
		struct strioctl ctl; /* Streams ioctl control structure */

		/* map frame buffer to next 8MB segment boundary above 128M */
		video_virtaddr = video_virtaddr + (8 * 1024 * 1024); 
	        vmap.map_physnum = 0;
        	vmap.map_virtaddr = video_virtaddr;

		ctl.ic_cmd = VIDEO_MAP;
		ctl.ic_timout = -1;
		ctl.ic_len = sizeof(vmap);
		ctl.ic_dp = (char *)&vmap;
		if (ioctl(fd, I_STR, &ctl) == -1) {
			FatalError ("ioctl I_STR VIDEO_MAP failed");
			(void) close (fd);
			return (FALSE);
		}

    		macIIFbs[index].fb = 
		    (pointer)(video_virtaddr + fbType.v_baseoffset); 
		(void) close(fd);
	}

	macIIFbs[index].info = fbType;
	macIIFbData[fbNum].probeStatus = probedAndSucceeded;

    }

    /*
     * If we've ever successfully probed this device, do the following.
     */
    oldNumScreens = pScreenInfo->numScreens;
    i = AddScreen(macIIMonoInit, argc, argv);
    return (i >= oldNumScreens);
}

