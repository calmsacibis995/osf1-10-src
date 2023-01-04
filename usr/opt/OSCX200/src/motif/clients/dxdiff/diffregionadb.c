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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/diffregionadb.c,v 1.1.4.2 1993/09/03 21:03:35 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	diffregionadb.c - diffregion aggregate code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : May 9th 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	16 Jan 1990	Colin Prosser
 *
 *	Fix calculation of logical line height to correspond with text
 *	widget and X protocol guidelines.  Clears problem where diffs
 *	didn't line up with corresponding text for many fonts.
 *
 *	1 Sep 1993	Lynda Rice
 *
 *	Changes due to implementation of GetVerticalSliderSize(), used to
 *	determine the number of rows on a screen.	
 */

static char sccsid[] = "@(#)diffregionadb.c	1.9 19:05:28 1/16/90";


#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
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


/********************************
 *
 *	DiffRegionADBDestroyCallBack
 *
 ********************************/

static void
DiffRegionADBDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	DiffRegionADBPtr diffregion = (DiffRegionADBPtr)clientd;

	XtFree((char *)diffregion);
}

static XtCallbackRec DiffRegionADBDestroyCallbackList[] = {
	{ (VoidProc)DiffRegionADBDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};


/********************************
 *
 *	NewDiffRegionADB
 *
 ********************************/

static DiffRegionADBPtr
NewDiffRegionADB(copy)
	DiffRegionADBPtr copy;
{
	register DiffRegionADBPtr new;

	if ((new = (DiffRegionADBPtr)XtMalloc(sizeof (DiffRegionADB))) == (DiffRegionADBPtr)NULL) {
		return new;	/* error */
	}

	if (copy != (DiffRegionADBPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (DiffRegionADB));
	}

	return new;
}


/********************************
 *
 *	CreateDiffRegionADB
 *
 ********************************/

static DiffRegionADB diffregionadb = {
	StaticInitCoreArgList(0, 0, 0, 0, 1, 
		(XtArgVal)DiffRegionADBDestroyCallbackList ),
	StaticInitDialogBoxArgList(XmPIXELS, XmSTRING, NULL, 
		XmDIALOG_WORK_AREA, XmRESIZE_ANY, False, 0, 0),
	StaticInitADBConstraintArgList(XmATTACH_FORM, XmATTACH_FORM, 
		XmATTACH_WIDGET, XmATTACH_WIDGET,
	       NULL, NULL, NULL, NULL,
	       0, 0, 0, 0),
	(Widget)NULL,
	(FillerPtr)NULL,
	(FillerPtr)NULL,
	(DifferenceBoxPtr)NULL
};

DiffRegionADBPtr
CreateDiffRegionADB(parent, name, core, constraints, dialog, lefttdadb, righttdadb, display)
	Widget			parent;
	char			*name;
	CoreArgListPtr		core;
	DialogBoxArgListPtr	dialog;
	ADBConstraintArgListPtr	constraints;
	TextDisplayADBPtr	lefttdadb,
				righttdadb;
	DxDiffDisplayPtr	display;
{
	register DiffRegionADBPtr new;
	CoreArgList		  coreargs,sparecore;
	CoreArgListPtr		  hsbcore;
	ADBConstraintArgList	  constraintargs;
	FillerPtr		  filler;
	int			  fontheight,
				  vmargins,
				  menuheight;
	XFontStruct		  *font;
	XmFontList		  *fontListP;
	int			  fontIndex;

	if ((new = NewDiffRegionADB(&diffregionadb)) == (DiffRegionADBPtr)NULL) {
		return new;	/* error */
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&DiffRegionADBPtrCoreArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&DiffRegionADBPtrConstraintArgList(new),
		      sizeof (ADBConstraintArgList) - sizeof (Arg));
	}

	if (dialog != (DialogBoxArgListPtr)NULL) {
		bcopy((char *)dialog, (char *)&DiffRegionADBPtrDialogBoxArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}
	DialogBoxResize(DiffRegionADBPtrDialogBoxArgList(new)) = XmRESIZE_ANY;

	CoreBorderWidth(DiffRegionADBPtrCoreArgList(new)) = 0;

	DiffRegionADBDestroyCallbackList[0].closure = new;

	DiffRegionADBPtrWidget(new) = (Widget)XmCreateForm(parent, name,
		  (ArgList)&DiffRegionADBPtrCoreArgList(new),
		  NumberOfArgsInArgListStruct(CoreArgList) +
		  NumberOfArgsInArgListStruct(ADBConstraintArgList) +
		  NumberOfArgsInArgListStruct(DialogBoxArgList)
				      );

	if (DiffRegionADBPtrWidget(new) == (Widget)NULL) {
		XtFree((char *)new);
		return (DiffRegionADBPtr)NULL;
	}

	XtManageChild(DiffRegionADBPtrWidget(new));

	InitCoreArgList(coreargs);
	InitCoreArgList(sparecore);
	InitADBConstraintArgList(constraintargs);

	/* create the top filler now ! */

	GetCoreArgs(FileNamePtrWidget(TextDisplayADBPtrFilename(lefttdadb)), &coreargs);
	GetCoreArgs(DiffRegionADBPtrWidget(new), &sparecore);
	CoreHeight(coreargs) = CoreHeight(coreargs) + 2;
	CoreWidth(coreargs) = CoreWidth(sparecore);

	ADBConstraintTopAttachment(constraintargs) = ADBConstraintLeftAttachment(constraintargs) = 
	ADBConstraintRightAttachment(constraintargs) = XmATTACH_FORM;

	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_NONE;

	ADBConstraintTopOffset(constraintargs) = ADBConstraintBottomOffset(constraintargs) =
	ADBConstraintLeftOffset(constraintargs) = ADBConstraintRightOffset(constraintargs) = 0;

	ADBConstraintTopWidget(constraintargs) = ADBConstraintBottomOffset(constraintargs) =
	ADBConstraintLeftWidget(constraintargs) = ADBConstraintRightOffset(constraintargs) = NULL;

	DiffRegionADBPtrTopFiller(new) = (FillerPtr)CreateFiller(
		DiffRegionADBPtrWidget(new), "topfiller",
		&coreargs, &constraintargs);

	if (DiffRegionADBPtrTopFiller(new) == (FillerPtr)NULL) {
		XtDestroyWidget(DiffRegionADBPtrWidget(new));
		return ((DiffRegionADBPtr)NULL);
	}


	/* create the bottom filler now ! */

	GetCoreArgs(AMenuBarPtrWidget(TextDisplayADBPtrMenuBar(lefttdadb)), &coreargs);
	GetCoreArgs(DiffRegionADBPtrWidget(new), &sparecore);
	CoreHeight(coreargs) = FindAMenuBarHeightFudge(TextDisplayADBPtrMenuBar(lefttdadb));

	CoreX(coreargs) = 0;
	CoreY(coreargs) = CoreHeight(sparecore) - (CoreHeight(coreargs) + 2 * CoreBorderWidth(coreargs));
	CoreY(coreargs) = CoreHeight(sparecore) - (CoreHeight(coreargs) + 2 * CoreBorderWidth(coreargs));
	CoreWidth(coreargs) = CoreWidth(sparecore);
	CoreHeight(coreargs) = CoreHeight(coreargs) + 2;

	ADBConstraintTopAttachment(constraintargs) = XmATTACH_NONE;
	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_FORM;

	DiffRegionADBPtrBottomFiller(new) = (FillerPtr)CreateFiller(
		DiffRegionADBPtrWidget(new), "bottomfiller",
		&coreargs, &constraintargs);

	if (DiffRegionADBPtrBottomFiller(new) == (FillerPtr)NULL) {
		XtDestroyWidget(DiffRegionADBPtrWidget(new));
		return ((DiffRegionADBPtr)NULL);
	}

	/* now create the differencebox itself !!! */

	ADBConstraintTopAttachment(constraintargs) =
	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_WIDGET;

	ADBConstraintTopWidget(constraintargs) = 
		(XtArgVal)FillerPtrWidget( DiffRegionADBPtrTopFiller(new));
	ADBConstraintBottomWidget(constraintargs) =  
		(XtArgVal) FillerPtrWidget(DiffRegionADBPtrBottomFiller(new));

	GetCoreArgs(TextDisplayPtrScrollWidget(TextDisplayADBPtrTextDisplay(lefttdadb)), &coreargs);
	GetCoreArgs(DiffRegionADBPtrWidget(new), &sparecore);
	CoreWidth(coreargs) = CoreWidth(sparecore);

	fontListP = (XmFontList*) TextDisplayPtrFontList(TextDisplayADBPtrTextDisplay(lefttdadb));
	if (fontListP == NULL) {
	    font = XLoadQueryFont(
		XtDisplay(TextDisplayPtrWidget(TextDisplayADBPtrTextDisplay(lefttdadb))), "fixed");
	} else {
	    _XmFontListSearch(fontListP, "default", &fontIndex, &font);
	}
	fontheight = font->ascent + font->descent;
/*
	fontheight = font->max_bounds.ascent + font->max_bounds.descent;
*/

	vmargins = TextMarginHeight(TextDisplayPtrTextArgList(TextDisplayADBPtrTextDisplay(lefttdadb)));

	DiffRegionADBPtrDifferenceBox(new) = (DifferenceBoxPtr)CreateDifferenceBox(
		DiffRegionADBPtrWidget(new), "differencebox",
		&coreargs, &constraintargs, 2,
		fontheight, vmargins, 0, 0,
		&app_options.linenumberforeground, font,
		display) ;

	if (DiffRegionADBPtrDifferenceBox(new) == (DifferenceBoxPtr)NULL) {
		XtDestroyWidget(DiffRegionADBPtrWidget(new));
		return ((DiffRegionADBPtr)NULL);
	}

	XtManageChild(DiffRegionADBPtrDifferenceBox(new)->window);

	return	new;
}
