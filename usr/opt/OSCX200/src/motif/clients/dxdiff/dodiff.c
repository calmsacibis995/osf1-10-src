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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/dxdiff/dodiff.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
 * Copyright 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 *
 *	dxdiff
 *
 *	dodiff.c - load up the display
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 31st May 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	06 Aug 1990	Colin Prosser
 *
 *	Fix storage allocation bugs and portability problems.
 *	Cures seg fault reported in UWS QAR 02624.
 */

static char sccsid[] = "@(#)dodiff.c	2.2";
 
#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "dxdiff.h"
#include "arglists.h"
#include "y.tab.h"
#include "filestuff.h"
#include "parsediff.h"
#include "alloc.h"
#include "differencebox.h"
#include "menu.h"
#include "text.h"
#include "display.h"
#include "displaymenu.h"

static char *diffcmd[] = { 
	"/bin/sh",
	"sh",
	"-c",
	"/usr/bin/diff $DxDiffLeftFile $DxDiffRightFile",
	(char *)NULL
};


/********************************
 *
 *     DoDiffs
 *
 ********************************/

Boolean
DoDiffs(display, lfile, rfile)
	DxDiffDisplayPtr display;
	char		 *lfile, 
			 *rfile;
{
	
	if (DxDiffDisplayPtrDiffList(display) != (DiffListBlkPtr)NULL) {
		FileInfoPtr filep, *filepp;
		
		filep = *(filepp = &TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(display))));
		if (filep != (FileInfoPtr)NULL) {
			FreeFile(filep);
			*filepp = (FileInfoPtr)NULL;
		}

		filep = *(filepp = &TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(display))));
		if (filep != (FileInfoPtr)NULL) {
			FreeFile(filep);
			*filepp = (FileInfoPtr)NULL;
		}

		InitDiffListBlk(DxDiffDisplayPtrDiffList(display)); /* free it */
		XtFree((char *)DxDiffDisplayPtrDiffList(display));
	}

	if ((DxDiffDisplayPtrDiffList(display) = (DiffListBlkPtr)XtMalloc(sizeof (DiffListBlk))) ==
	    (DiffListBlkPtr)NULL) {
		return False;	/* error */
	} else {
		bzero((char *)DxDiffDisplayPtrDiffList(display), sizeof (DiffListBlk));

		if ((DxDiffDisplayPtrDiffList(display)->caches = (BackEndCachePtr)XtMalloc(sizeof (BackEndCache))) ==
		    (BackEndCachePtr)NULL) {
			XtFree((char *)DxDiffDisplayPtrDiffList(display));
			return False;
		} else {
			DxDiffDisplayPtrDiffList(display)->caches->cachesempty = True;
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->nssc), 50);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->edcsc), 400);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->dflsc), 50);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->bfnsc), 10);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->cfnsc), 10);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->ofnsc), 10);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->ifnsc), 10);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->densc), 10);
			InitializeStoreCache(&(DxDiffDisplayPtrDiffList(display)->caches->dcnsc), 10);
		}
	}

	return InvokeDiff(diffcmd, lfile, rfile, DxDiffDisplayPtrDiffList(display));
}
	
/********************************
 *
 *     LoadDiffs
 *
 ********************************/

LoadDiffs(dxdiffdisplay, lfile, rfile, edcph, edcpt)
		DxDiffDisplayPtr dxdiffdisplay;
		char		 *lfile, 
				 *rfile;
		EdcPtr		 edcph,
				 edcpt;
{
	Arg		args[4];
	unsigned short	numlines, txtnumlines;
	AMenuBarPtr	menubar;
	CoreArgListPtr	core;
	unsigned int	inc;
	
	SetFileName(TextDisplayADBPtrFilename(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)), rfile);
	SetFileName(TextDisplayADBPtrFilename(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)), lfile);
	SetFileNameHighLight(TextDisplayADBPtrFilename(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)), True);
	SetFileNameHighLight(TextDisplayADBPtrFilename(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)), True);

	SetFileNameSensitivity(TextDisplayADBPtrFilename(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)), True);
	SetFileNameSensitivity(TextDisplayADBPtrFilename(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)), True);

	LoadTextDisplay(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)), rfile);

	LoadTextDisplay(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)), lfile);

	InitializeHighLightInfo(TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay))),
				edcph, 
				edcpt,
				0, RightFile);

	InitializeHighLightInfo(TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay))),
				edcph, 
				edcpt,
				0, LeftFile);


	ReLoadDifferenceBox(DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay)),
			     edcph, 
			     edcpt,
			     TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)))->numlines,
			     TextDisplayPtrFile(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)))->numlines

	);

	menubar = TextDisplayADBPtrMenuBar(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay));
	SetPushButtonSensitivity(MenuEntryPtrPushButton(AMenuBarPtrEntries(menubar)[(int)NextDiffButton]), True);
	SetPushButtonSensitivity(MenuEntryPtrPushButton(AMenuBarPtrEntries(menubar)[(int)PrevDiffButton]), True);
	
	menubar = TextDisplayADBPtrMenuBar(DxDiffDisplayPtrRightTextADB(dxdiffdisplay));
	SetPushButtonSensitivity(MenuEntryPtrPushButton(AMenuBarPtrEntries(menubar)[(int)NextDiffButton]), True);
	SetPushButtonSensitivity(MenuEntryPtrPushButton(AMenuBarPtrEntries(menubar)[(int)PrevDiffButton]), True);

	if (DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay))->vdtop != (EdcPtr)NULL) {
		UpdateTextHighLights(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)),
				     TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)),
				     DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay)));
	}
}
