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
 * macIIColor.c --
 *	Functions to support the macII color board as a memory frame buffer.
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
/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,p and  distribute   this
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

#include    "colormap.h"
#include    "colormapst.h"
#include    "resource.h"

extern int TellLostMap(), TellGainedMap();

extern int consoleFd;

static struct ColorSpec {
	unsigned short value;
	unsigned short red;
	unsigned short green;
	unsigned short blue;
}; 

static void
macIIColorUpdateColormap(pScreen, cmap)
    ScreenPtr	pScreen;
    ColormapPtr	cmap;
{
    register int i;
    register Entry *pent;
    struct ColorSpec Map[256];
    register VisualPtr pVisual = cmap->pVisual;
    int fd;
    struct strioctl ctl; /* Streams ioctl control structure */
    struct CntrlParam pb;
    struct VDEntryRecord vde;

    if ((pVisual->class | DynamicClass) == DirectColor) {
	for (i = 0; i < 256; i++) {
	    Map[i].value = i;
	    pent = &cmap->red[(i & pVisual->redMask) >>
			      pVisual->offsetRed];
	    if (pent->fShared)
		Map[i].red = pent->co.shco.red->color;
	    else
		Map[i].red = pent->co.local.red;
	    pent = &cmap->green[(i & pVisual->greenMask) >>
				pVisual->offsetGreen];
	    if (pent->fShared)
		Map[i].green = pent->co.shco.green->color;
	    else
		Map[i].green = pent->co.local.green;
	    pent = &cmap->blue[(i & pVisual->blueMask) >>
			       pVisual->offsetBlue];
	    if (pent->fShared)
		Map[i].blue = pent->co.shco.blue->color;
	    else
		Map[i].blue = pent->co.local.blue;
	}
    } else {
	for (i = 0, pent = cmap->red;
	     i < pVisual->ColormapEntries;
	     i++, pent++) {
	    if (pent->fShared) {
		Map[i].value = i;
		Map[i].red = pent->co.shco.red->color;
		Map[i].green = pent->co.shco.green->color;
		Map[i].blue = pent->co.shco.blue->color;
	    }
	    else {
		Map[i].value = i;
		Map[i].red = pent->co.local.red;
		Map[i].green = pent->co.local.green;
		Map[i].blue = pent->co.local.blue;
	    }
	}
    }

    /* 
     * Unfortunately we must slam the entire colormap into
     * the video boards CLUT 'cause SuperMac (and others?)
     * insist on swallowing the whole map at once. The MacOS
     * does it this way. 
     */

    if (consoleFd <= 0) {
	    fd = open("/dev/console",O_RDWR);
    } else {
	    fd = consoleFd;
    }
    if (fd <= 0) FatalError("Open Failed for VIDEO_CONTROL. \n");

    ctl.ic_cmd = VIDEO_CONTROL;
    ctl.ic_timout = -1;
    ctl.ic_len = sizeof(pb);
    ctl.ic_dp = (char *)&pb;

    vde.csTable = (char *) Map;
    vde.csStart = 0;
    vde.csCount = 255;	/* not actually count, but count-1 */

#define noQueueBit 0x0200
#define SetEntries 0x3
    pb.qType = macIIFbs[pScreen->myNum].slot;
    pb.ioTrap = noQueueBit;
    pb.ioCmdAddr = (char *) -1;
    pb.csCode = SetEntries;
    * (char **) pb.csParam = (char *) &vde;

    if (ioctl(fd, I_STR, &ctl) == -1) {
	    FatalError ("ioctl I_STR VIDEO_CONTROL failed");
	    (void) close (fd);
    }

    if (consoleFd <=0) close(fd);

}

/*-
 *-----------------------------------------------------------------------
 * macIIColorSaveScreen --
 *	Preserve the color screen by turning on or off the video
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Video state is switched
 *
 *-----------------------------------------------------------------------
 */
static Bool
macIIColorSaveScreen (pScreen, on)
    ScreenPtr	  pScreen;
    int    	  on;
{
    if (on == SCREEN_SAVER_FORCER)
	SetTimeSinceLastInputEvent ();
    return FALSE;
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorInstallColormap --
 *	Install given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	Existing map is uninstalled.
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorInstallColormap(cmap)
    ColormapPtr	cmap;
{
    register ColormapPtr installedMap = 
			 macIIFbs[cmap->pScreen->myNum].installedMap;

    if (cmap == installedMap)
	return;
    if (installedMap)
	WalkTree(installedMap->pScreen, TellLostMap,
		 (char *) &(installedMap->mid));
    macIIFbs[cmap->pScreen->myNum].installedMap = cmap;
    macIIColorUpdateColormap(cmap->pScreen, cmap);
    WalkTree(cmap->pScreen, TellGainedMap, (char *) &(cmap->mid));
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorUninstallColormap --
 *	Uninstall given colormap.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	default map is installed
 *	All clients requesting ColormapNotify are notified
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorUninstallColormap(cmap)
    ColormapPtr	cmap;
{
    if (cmap == macIIFbs[cmap->pScreen->myNum].installedMap) {
	Colormap defMapID = cmap->pScreen->defColormap;

	if (cmap->mid != defMapID) {
	    ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID, RT_COLORMAP);

	    if (defMap)
		(*cmap->pScreen->InstallColormap)(defMap);
	    else
	        ErrorF("macIIColor: Can't find default colormap\n");
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorListInstalledColormaps --
 *	Fills in the list with the IDs of the installed maps
 *
 * Results:
 *	Returns the number of IDs in the list
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
static int
macIIColorListInstalledColormaps(pScreen, pCmapList)
    ScreenPtr	pScreen;
    Colormap	*pCmapList;
{
    *pCmapList = (macIIFbs[pScreen->myNum].installedMap)->mid;
    return (1);
}


/*-
 *-----------------------------------------------------------------------
 * macIIColorStoreColors --
 *	Sets the pixels in pdefs into the specified map.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	None
 *
 *-----------------------------------------------------------------------
 */
static void
macIIColorStoreColors(pmap, ndef, pdefs)
    ColormapPtr	pmap;
    int		ndef;
    xColorItem	*pdefs;
{
    if (pmap == macIIFbs[pmap->pScreen->myNum].installedMap)
	macIIColorUpdateColormap(pmap->pScreen, pmap);
}

/*-
 *-----------------------------------------------------------------------
 * macIIColorInit --
 *	Attempt to find and initialize a color framebuffer 
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
Bool
macIIColorInit (index, pScreen, argc, argv)
    int	    	  index;    	/* The index of pScreen in the ScreenInfo */
    ScreenPtr	  pScreen;  	/* The Screen to initialize */
    int	    	  argc;	    	/* The number of the Server's arguments. */
    char    	  **argv;   	/* The arguments themselves. Don't change! */
{
    /* set up procs which aren't handled by cfb */
    /* save screen proc */
    pScreen->SaveScreen = macIIColorSaveScreen;
    /* colormap procs */
    pScreen->InstallColormap = macIIColorInstallColormap;
    pScreen->UninstallColormap = macIIColorUninstallColormap;
    pScreen->ListInstalledColormaps = macIIColorListInstalledColormaps;
    pScreen->StoreColors = macIIColorStoreColors;

    if (!cfbScreenInit (pScreen, macIIFbs[index].fb,
			    macIIFbs[index].info.v_right -
			    macIIFbs[index].info.v_left,
			    macIIFbs[index].info.v_bottom -
		            macIIFbs[index].info.v_top,
			    macIIFbs[index].info.v_hres >> 16,
			    macIIFbs[index].info.v_vres >> 16,
			    macIIFbs[index].info.v_rowbytes))
    {
	return (FALSE);
    }

    /* make sure the color map gets installed */

    macIIFbs[index].installedMap = NULL;

    (void) macIIColorSaveScreen( pScreen, SCREEN_SAVER_OFF );

    return (macIIScreenInit(pScreen) && cfbCreateDefColormap(pScreen));
}
