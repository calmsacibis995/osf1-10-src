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
static char *BuildSystemHeader= "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/dxdiff/misc.c,v 1.1.2.2 92/08/03 09:49:25 Dave_Hill Exp $";	/* BuildSystemHeader */
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
 *	misc.c - misc display handler code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 21st April 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 */

static char sccsid[] = "@(#)misc.c	1.11	19:02:14 10/4/88";


#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
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
#include "mainmenu.h"
#include "icon.h"

#define MOTIF_WM 1
#define DEC_WM 2
#ifndef XtNiconifyPixmap
#define XtNiconifyPixmap "iconifyPixmap"
#endif

/********************************
 *
 *	FillerDestroyCallBack
 *
 ********************************/

static void
FillerDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	XtFree((char *)(FillerPtr)clientd);
}

static XtCallbackRec fillerdestroycallbacklist[] = {
	{ (VoidProc)FillerDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};


 /********************************
 *
 *	NewFiller
 *
 ********************************/


static FillerPtr
NewFiller(copy)
	FillerPtr copy;
{
	register FillerPtr new;

	if ((new = (FillerPtr)XtMalloc(sizeof (Filler))) == (FillerPtr)NULL) {
		return new;	/* error */
	}

	if (copy != (FillerPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (Filler));
	}

	return new;
}


/********************************
 *
 *	CreateFiller
 *
 ********************************/


static Filler	filler = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)fillerdestroycallbacklist),
	StaticInitADBConstraintArgList(XmATTACH_NONE, XmATTACH_NONE,
				       XmATTACH_NONE, XmATTACH_NONE,
				       NULL, NULL,
				       NULL, NULL,
				       0, 0, 0, 0),
	(Widget)NULL
};

FillerPtr
CreateFiller(parent, name, core, constraints)
	Widget			parent;
	char 			*name;
	CoreArgListPtr		core;
	ADBConstraintArgListPtr	constraints;
{
	register FillerPtr new;

	if ((new = NewFiller(&filler)) == (FillerPtr)NULL) {
		return new;
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&FillerPtrCoreArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&FillerPtrADBConstraintArgList(new),
		      sizeof (ADBConstraintArgList));
	}


	fillerdestroycallbacklist[0].closure = (caddr_t)new;

	FillerPtrWidget(new) = (Widget)XmCreateDrawingArea(parent, name, 
					       (ArgList)&FillerPtrCoreArgList(new),
					       NumberOfArgsInArgListStruct(CoreArgList) + 
					       NumberOfArgsInArgListStruct(ADBConstraintArgList)
			       );

	if (FillerPtrWidget(new) != (Widget)NULL) {
		XtManageChild(FillerPtrWidget(new));
	}
	
	return new;
}


/********************************
 *
 *	ALabelDestroyCallBack
 *
 ********************************/

static void
ALabelDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	XtFree((char *)(ALabelPtr)clientd);
}

static XtCallbackRec alabeldestroycallbacklist[] = {
	{ (VoidProc)ALabelDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};


 /********************************
 *
 *	NewALabel
 *
 ********************************/


static ALabelPtr
NewALabel(copy)
	ALabelPtr copy;
{
	register ALabelPtr new;

	if ((new = (ALabelPtr)XtMalloc(sizeof (ALabel))) == (ALabelPtr)NULL) {
		return new;	/* error */
	}

	if (copy != (ALabelPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (ALabel));
	}

	return new;
}


/********************************
 *
 *	CreateALabel
 *
 ********************************/


static ALabel	alabel = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)alabeldestroycallbacklist),
	StaticInitADBConstraintArgList(XmATTACH_NONE, XmATTACH_NONE,
				       XmATTACH_NONE, XmATTACH_NONE,
				       NULL, NULL,
				       NULL, NULL,
				       0, 0, 0, 0),
	StaticInitLabelArgList(XmSTRING, NULL, 1, 1, XmALIGNMENT_END,
			       0, 0, 0, 0, False, False),
	NULL
};

ALabelPtr
CreateALabel(parent, name, core, constraints, label)
	Widget			parent;
	char 			*name;
	CoreArgListPtr		core;
	ADBConstraintArgListPtr	constraints;
	LabelArgListPtr		label;
{
	register ALabelPtr new;

	if ((new = NewALabel(&alabel)) == (ALabelPtr)NULL) {
		return new;
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&ALabelPtrCoreArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&ALabelPtrADBConstraintArgList(new),
		      sizeof (ADBConstraintArgList));
	}

	if (label != (LabelArgListPtr)NULL) {
		bcopy((char *)label, (char *)&ALabelPtrLabelArgList(new),
		      sizeof (LabelArgList));
	}


	alabeldestroycallbacklist[0].closure = (caddr_t)new;

	ALabelPtrWidget(new) = (Widget)XmCreateLabel(parent, name, 
			(ArgList)&ALabelPtrCoreArgList(new),
			NumberOfArgsInArgListStruct(CoreArgList) + 
			NumberOfArgsInArgListStruct(ADBConstraintArgList) +
			NumberOfArgsInArgListStruct(LabelArgList)
			);

	if (ALabelPtrWidget(new) != (Widget)NULL) {
		XtManageChild(ALabelPtrWidget(new));
	}
	
	return new;
}

/********************************
 *
 *	FileNameDestroyCallBack
 *
 ********************************/

static void
FileNameDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	register FileNamePtr fnp = (FileNamePtr)clientd;

	if (FileNamePtrFile(fnp) != (char *)NULL) {
		XtFree(FileNamePtrFile(fnp));
	}

	XtFree((char *)(FileNamePtr)clientd);
}

static XtCallbackRec FileNameDestroyCallbackList[] = {
	{ (VoidProc)FileNameDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};

/********************************
 *
 *	NewFileName
 *
 ********************************/

static FileNamePtr
NewFileName(copy)
	FileNamePtr copy;
{
	register FileNamePtr new;

	if ((new = (FileNamePtr)XtMalloc(sizeof (FileName))) == (FileNamePtr)NULL) {
		return new; 	/* error */
	}

	if (copy != (FileNamePtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (FileName));
	}

	return new;
}

/********************************
 *
 *	CreateFileName
 *
 ********************************/

static	FileName filename  = {
	StaticInitCoreArgList( 0, 0, 0, 0, 0, (XtArgVal)FileNameDestroyCallbackList),
	StaticInitADBConstraintArgList(XmATTACH_FORM, XmATTACH_SELF, XmATTACH_FORM, XmATTACH_FORM,
				       NULL, NULL, NULL, NULL,
				       0, 0, 0, 0),
	StaticInitLabelArgList(XmSTRING, NULL, 1, 1, XmALIGNMENT_END,
			       0, 0, 0, 0, False, False),
	StaticInitFileNameSensitivity(False),
	StaticInitFontArgList(0, 0, NULL),
	NULL,
	NULL,
	NULL
};

FileNamePtr
CreateFileName(parent, name, core, constraints, label)
	Widget			parent;
	char			*name;
	CoreArgListPtr		core;
	ADBConstraintArgListPtr	constraints;
	LabelArgListPtr		label;
{
	register FileNamePtr new;

	if ((new = NewFileName(&filename)) == (FileNamePtr)NULL) {
		return new; 	/* error */
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&FileNamePtrCoreArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&FileNamePtrConstraintArgList(new),
		      sizeof (ADBConstraintArgList));
	}

	if (label != (LabelArgListPtr)NULL) {
		bcopy((char *)label, (char *)&FileNamePtrLabelArgList(new),
		      sizeof (LabelArgList));
	} else {
		LabelLabel(FileNamePtrLabelArgList(new)) = (XtArgVal)
		XmStringLtoRCreate(" ", "ISO8859-1"); /* to avoid getting widget name */
	}

	FileNameDestroyCallbackList[0].closure = new;

	FileNamePtrWidget(new) = (Widget)XmCreateLabel(parent, name,
					(ArgList)&FileNamePtrCoreArgList(new),
					NumberOfArgsInArgListStruct(CoreArgList) +
					NumberOfArgsInArgListStruct(ADBConstraintArgList) +
					NumberOfArgsInArgListStruct(LabelArgList) + 1
				 );

	if (FileNamePtrWidget(new)) {
		XtManageChild(FileNamePtrWidget(new));
	}

	{	/* get the font info */
		Arg	font;

		font.name = XmNfontList;
;
		font.value = (XtArgVal)&FileNamePtrFontList(new);

		XtGetValues(FileNamePtrWidget(new), &font, 1);
	}
	return new;
}


/********************************
 *
 *	SetFileName
 *
 ********************************/

void
SetFileName(fnp, filename)
	FileNamePtr	fnp;
	char		*filename;
{
	int len;

	if (LabelLabel(FileNamePtrLabelArgList(fnp)) != NULL) {
		XtFree((char *)LabelLabel(FileNamePtrLabelArgList(fnp)));
	}

	if (FileNamePtrFile(fnp) != (char *)NULL) {
		XtFree(FileNamePtrFile(fnp));
		FileNamePtrFile(fnp) = (char *)NULL;
	}

	if ((FileNamePtrFile(fnp) = XtMalloc((len = strlen(filename)) + 1)) != (char *)NULL) {
		strcpy(FileNamePtrFile(fnp), filename);
	}

	if (FileNamePtrFontList(fnp) != (XmFontList)NULL) {
		Arg		arg;
		int		fontIndex;
		Dimension	width = 0;
		XFontStruct	*font;

		arg.name = XmNwidth;
		arg.value = (XtArgVal) &width;

		XtGetValues(FileNamePtrWidget(fnp), &arg, 1);
		_XmFontListSearch(FileNamePtrFontList(fnp), "default",
			&fontIndex, &font);
		arg.value = (XTextWidth(font, filename, len) > width)
			       ? XmALIGNMENT_END : XmALIGNMENT_CENTER;
		arg.name = XmNalignment;

		XtSetValues(FileNamePtrWidget(fnp), &arg, 1);
	}

	LabelLabel(FileNamePtrLabelArgList(fnp)) = (XtArgVal)
		XmStringLtoRCreate(filename , "ISO8859-1");

	XtSetValues(FileNamePtrWidget(fnp),
		    PointerToArg(LabelLabel(FileNamePtrLabelArgList(fnp))),
		    1
	);
}



/********************************
 *
 *	SetFileNameHighLight
 *
 ********************************/

void
SetFileNameHighLight(fnp, state)
	FileNamePtr	fnp;
	Boolean		state;
{
	Arg	highlight;
	
	highlight.name = "fillHighlight";
	highlight.value = state;

	XtSetValues(FileNamePtrWidget(fnp), &highlight, 1);
}

/********************************
 *
 *	SetFileNameSensitivity
 *
 ********************************/

void
SetFileNameSensitivity(fnp, state)
	FileNamePtr	fnp;
	Boolean		state;
{
	FileNamePtrSensitivity(fnp) = state;

	XtSetValues(FileNamePtrWidget(fnp), PointerToArg(FileNamePtrSensitivity(fnp)), 1);
}

/********************************
 *
 *	MainADBDestroyCallBack
 *
 ********************************/

static void
MainADBDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	XtFree((char *)(MainADBPtr)clientd);
}

static XtCallbackRec mainadbdestroycallbacklist[] = {
	{ (VoidProc)MainADBDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};

/********************************
 *
 *	NewMainADB
 *
 ********************************/

static MainADBPtr
NewMainADB(copy)
	MainADBPtr copy;
{
	register MainADBPtr new;

	if ((new = (MainADBPtr)XtMalloc(sizeof (MainADB))) == (MainADBPtr)NULL) {
		return new;	/* error */
	}

	if (copy != (MainADBPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (MainADB));
	}

	return new;
}

/********************************
 *
 *	CreateMainADB
 *
 ********************************/

static	MainADB	mainadb = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)mainadbdestroycallbacklist),
	StaticInitDialogBoxArgList(XmPIXELS, XmSTRING, NULL,
				   XmDIALOG_WORK_AREA,XmRESIZE_ANY, False, 0, 0),
	StaticInitADBConstraintArgList( XmATTACH_NONE, XmATTACH_NONE,
				        XmATTACH_NONE, XmATTACH_NONE,
					NULL, NULL,
					NULL, NULL,
					0, 0, 0, 0),
	NULL,
	NULL
};

MainADBPtr
CreateMainADB(parent, name, core, dialog, constraints)
	Widget			parent;
	char			*name;
	CoreArgListPtr		core;
	DialogBoxArgListPtr	dialog;
	ADBConstraintArgListPtr	constraints;
{
	register MainADBPtr new;

	if ((new = NewMainADB(&mainadb)) == (MainADBPtr)NULL) {
		return new;	/* error */
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&MainADBPtrCoreArgList(new),
		      sizeof (CoreArgList) - sizeof (Arg));
	}

	if (dialog != (DialogBoxArgListPtr)NULL) {
		bcopy((char *)dialog, (char *)&MainADBPtrDialogBoxArgList(new),
		      sizeof (DialogBoxArgList));
	}
	DialogBoxResize(MainADBPtrDialogBoxArgList(new));

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&MainADBPtrConstraintArgList(new),
		      sizeof (ADBConstraintArgList));
	}

	MainADBPtrParent(new) = parent;

	MainADBPtrWidget(new) = (Widget)XmCreateForm(parent, name, 
					(ArgList)&MainADBPtrCoreArgList(new),
					NumberOfArgsInArgListStruct(CoreArgList) + 
					NumberOfArgsInArgListStruct(DialogBoxArgList) +
					NumberOfArgsInArgListStruct(ADBConstraintArgList) - 1
				);

	if (MainADBPtrWidget(new) != (Widget)NULL) {
		XtManageChild(MainADBPtrWidget(new));
	}

	return new;
}

/********************************
 *
 *	GetCoreArgs
 *
 ********************************/

GetCoreArgs(widget, core)
	Widget		widget;
	CoreArgListPtr	core;
{
	unsigned short 	x,y,w,h,bw;	/* what a pain */

	CorePtrX(core) = (XtArgVal)&x;
	CorePtrY(core) = (XtArgVal)&y;
	CorePtrWidth(core) = (XtArgVal)&w;
	CorePtrHeight(core) = (XtArgVal)&h;
	CorePtrBorderWidth(core) = (XtArgVal)&bw;

	XtGetValues(widget, (ArgList)core, 5);

	CorePtrX(core) = (unsigned int)x;
	CorePtrY(core) = (unsigned int)y;
	CorePtrWidth(core) = (unsigned int)w;
	CorePtrHeight(core) = (unsigned int)h;
	CorePtrBorderWidth(core) = (unsigned int)bw;
}

/********************************
 *
 *	InitialConfigureOptions()
 *
 ********************************/

void
InitialConfigureOptions(dxdiffdisplay)
	register DxDiffDisplayPtr dxdiffdisplay;
{
	DifferenceBoxPtr	  differencebox = DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay));
	AMenuBarPtr		  mainmenubar = DxDiffDisplayPtrMenuBar(dxdiffdisplay);
	PullDownEntryPtr	  pulldown =  (PullDownEntryPtr)MenuEntryPtrPullDown(AMenuBarPtrEntries(mainmenubar)[(int)OptionsButton]);
	PullDownMenuEntryPtr	  pulldownentry = PullDownEntryPtrPullDownMenuEntry(pulldown);
	PushButtonEntryPtr	  pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)RenderLineNumbersButton];

	if (app_options.displaylinenumbers) {
		SetDifferenceBoxPaintLineNumbersOn(differencebox);
		SetPushButtonToAlternativeLabel(pushbutton);
	} else {
		SetDifferenceBoxPaintLineNumbersOff(differencebox);
		SetPushButtonToMainLabel(pushbutton);
	}
		
	pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)SlaveScrollVButton];
	if (app_options.slaveverticalscrolling) {
		SetDifferenceBoxScrollBoth(differencebox);
		SetPushButtonToAlternativeLabel(pushbutton);
	} else {
		SetDifferenceBoxScrollOne(differencebox);
		SetPushButtonToMainLabel(pushbutton);
	}

	pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)SlaveScrollHButton];
	if ((DxDiffDisplayPtrHorizontalSlaveScroll(dxdiffdisplay) = app_options.slavehorizontalscrolling)) {
		SetPushButtonToAlternativeLabel(pushbutton);
	} else {
		SetPushButtonToMainLabel(pushbutton);
	}

	pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)RenderDiffsButton];
	if (app_options.drawdiffsaslines) {
		SetDifferenceBoxToDrawDiffsAsLines(differencebox);
		SetPushButtonToAlternativeLabel(pushbutton);
	} else {
		SetDifferenceBoxToDrawDiffsAsFilledPolygons(differencebox);
		SetPushButtonToMainLabel(pushbutton);
	}
}

void
dxdiffIconCreate(w)
Widget w;
{
    static int WhichWM = -1;
    char *bits;
    int width, height;
    static Pixmap icon_pixmap = (Pixmap) 0;
    static Pixmap big_pixmap = (Pixmap) 0;
    static Pixmap small_pixmap = (Pixmap) 0;
    Arg arg[2];
    XIconSize *icon_size_list;
    int numsizes;
    Display *dpy;

    if (WhichWM == -1) {
	dpy = XtDisplay(w);
	if (XGetIconSizes(dpy, XtScreen(w)->root, 
		&icon_size_list, &numsizes)) {
	    if (numsizes > 0) {
		if (icon_size_list[0].width_inc > 1) {
		    WhichWM = DEC_WM;
		} else {
		    WhichWM = MOTIF_WM;
		}
	    } else {
		WhichWM = MOTIF_WM;
	    }
	}
    }
    width = icon_width;
    height = icon_height;
    bits = icon_bits;

    if (WhichWM == MOTIF_WM) {
	XtSetArg(arg[0], XtNiconPixmap, &icon_pixmap);
	XtGetValues(w, arg, 1);
	if (icon_pixmap == (Pixmap) 0) {
	    XtSetArg(arg[0], XtNiconPixmap,
		XCreateBitmapFromData(dpy, XtScreen(w)->root,
				      bits, width, height));
		XtSetValues(w, arg, 1);
	}
    } else {
	/* DEC Window Manager */
	if (big_pixmap == (Pixmap) 0) {
	    big_pixmap = XCreateBitmapFromData(dpy, XtScreen(w)->root,
					       icon_bits,
					       icon_width, icon_height);
	}
	if (small_pixmap == (Pixmap) 0) {
	    small_pixmap = XCreateBitmapFromData(dpy, XtScreen(w)->root,
						 iconify_bits,
						 iconify_width, iconify_height);
	}
	XtSetArg(arg[0], XtNiconPixmap, big_pixmap);
	XtSetArg(arg[1], XtNiconifyPixmap, small_pixmap);
	XtSetValues(w, arg, 2);
    }
}
