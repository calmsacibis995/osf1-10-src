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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/dxdiffdisplay.c,v 1.1.4.2 1993/06/25 16:58:07 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	dxdiffdisplay.c - mainadb aggregate code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : May 14th 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	17 Aug 1988	Laurence P. G. Cable
 *
 *	Fix bug in InitDiffListBlk code to properly zero out struct
 *
 *	21st Sept 1988	Laurence P. G. Cable
 *
 *	Try and fix silly resource manager problem with multiple displays
 *
 *	06 Aug 1990	Colin Prosser
 *
 *	Fix storage allocation bugs and portability problems.
 *	Cures seg fault reported in UWS QAR 02624.
 */

static char sccsid[] = "@(#)dxdiffdisplay.c	1.9	19:02:08 10/4/88";


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


extern MainADBPtr CreateMainADB();
extern	PushButtonEntry nextdiff;
extern	PushButtonEntry prevdiff;


/********************************
 *
 *	DxDiffDestroyCallBack
 *
 ********************************/

static void 
DxDiffDisplayDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd,
		calld;
{
	XtFree((char *)(DxDiffDisplayPtr)clientd);
}

static	XtCallbackRec dxdiffdisplaydestroycallbacklist[] = {
	{ (VoidProc)DxDiffDisplayDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};

/********************************
 *
 *	NewDxDiffDisplay
 *
 ********************************/

static DxDiffDisplayPtr
NewDxDiffDisplay(copy)
	DxDiffDisplayPtr copy;
{
	register DxDiffDisplayPtr new;

	if ((new = (DxDiffDisplayPtr)XtMalloc(sizeof (DxDiffDisplay))) == (DxDiffDisplayPtr)NULL) {
		return new;	/* error */
	}

	if (copy != (DxDiffDisplayPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (DxDiffDisplayPtr));
	}

	return new;
}


/********************************
 *
 *	CreateDxDiffDisplay
 *
 ********************************/


static	DxDiffDisplay	dxdiffdisplay = {
	(MainADBPtr)NULL,
	(MainADBPtr)NULL,
	(AMenuBarPtr)NULL,
	(TextDisplayADBPtr)NULL,
	(TextDisplayADBPtr)NULL,
	(DiffRegionADBPtr)NULL,
	0,
	NULL,
	(FileSelectorPtr)NULL,
	False,
	(WhichFile)(-1)	/* neither */
};

#define	MainMenuBarBorderWidth	2
#define	PercentageWidth		43

int percentagewidth = PercentageWidth;

DxDiffDisplayPtr
CreateDxDiffDisplay(parent, name, core)
	Widget			parent;
	char			*name;
	CoreArgListPtr		core;
{
	register DxDiffDisplayPtr new;
	CoreArgList		  coreargs,sparecore;
	DialogBoxArgList	  dialogargs;
	ADBConstraintArgList	  constraintargs;

	if ((new = NewDxDiffDisplay(&dxdiffdisplay)) == (DxDiffDisplayPtr)NULL) {
		return new;	/* error */
	}

	InitCoreArgList(coreargs);
	InitCoreArgList(sparecore);
	InitDialogBoxArgList(dialogargs);
	InitADBConstraintArgList(constraintargs);

	/* create the main display */

	DialogBoxUnits(dialogargs) = XmPIXELS;
	DialogBoxTitleType(dialogargs) = XmSTRING;
	DialogBoxTitle(dialogargs) = NULL;
	DialogBoxStyle(dialogargs) = XmDIALOG_WORK_AREA;
	DialogBoxResize(dialogargs) = XmRESIZE_ANY;
	DialogBoxChildOverlap(dialogargs) = False;
	DialogBoxMarginHeight(dialogargs) = 0;
	DialogBoxMarginWidth(dialogargs) = 0;
	
	DxDiffDisplayPtrMainADB(new) = CreateMainADB(parent, "dxdiffmaindisplay", core, &dialogargs,
						     (ADBConstraintArgListPtr)NULL);
	
	if (DxDiffDisplayPtrMainADB(new) == (MainADBPtr)NULL) {
		XtFree((char *)new);
		return ((DxDiffDisplayPtr)NULL); /* error */
	}

	dxdiffdisplaydestroycallbacklist[0].closure = (caddr_t)new;
	XtAddCallbacks(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)), XmNdestroyCallback,
					dxdiffdisplaydestroycallbacklist);

	/* now create the main menu bar */


	ADBConstraintTopAttachment(constraintargs) = ADBConstraintLeftAttachment(constraintargs) =
	ADBConstraintRightAttachment(constraintargs) = XmATTACH_FORM;

	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_NONE;

	ADBConstraintTopOffset(constraintargs) = ADBConstraintBottomOffset(constraintargs) =
	ADBConstraintLeftOffset(constraintargs) = ADBConstraintRightOffset(constraintargs) = 0;

	ADBConstraintTopWidget(constraintargs) = ADBConstraintBottomWidget(constraintargs) =
	ADBConstraintLeftWidget(constraintargs) = ADBConstraintRightWidget(constraintargs) = NULL;


	GetCoreArgs(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)), &coreargs);
	CoreX(coreargs) = CoreY(coreargs) = MainMenuBarBorderWidth;
	CoreHeight(coreargs) = 1;	/* cheat */
	CoreWidth(coreargs) -= 2 * MainMenuBarBorderWidth;
	CoreBorderWidth(coreargs) = MainMenuBarBorderWidth;

	DxDiffDisplayPtrMenuBar(new) = (AMenuBarPtr)CreateMainMenu(
		MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)),
		&coreargs,&constraintargs, 
		(MenuBarArgListPtr)NULL, (caddr_t)new);

	if (DxDiffDisplayPtrMenuBar(new) == (AMenuBarPtr)NULL) {
		XtDestroyWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)));
		return (DxDiffDisplayPtr)NULL;
	}

#if 0
	{	/* check to see that the help button (if any) is visible) and correct of not ! */

		Arg 		helpbutton;
		Widget		helpwidget = (Widget)NULL;

		helpbutton.name = XmNmenuHelpWidget;
		helpbutton.value = (XtArgVal)&helpwidget;

		XtGetValues(AMenuBarPtrWidget(DxDiffDisplayPtrMenuBar(new)), &helpbutton, 1);


		if (helpwidget != (Widget)NULL) {
			int t;

			GetCoreArgs(AMenuBarPtrWidget(DxDiffDisplayPtrMenuBar(new)), &sparecore);
			GetCoreArgs(helpwidget, &coreargs);

			if ((t = CoreY(coreargs) + CoreWidth(coreargs) + 2 * CoreBorderWidth(coreargs)) >=
			    CoreWidth(sparecore)) {
				t -= CoreWidth(sparecore);

				GetCoreArgs(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)), &coreargs);
				CoreWidth(coreargs) += t;

				XtResizeWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)),
					       CoreWidth(coreargs),CoreHeight(coreargs),
					       CoreBorderWidth(coreargs));
			}
		}
	}
#endif

	/* now create the the display ADB to contain the text displays and diff region */


	GetCoreArgs(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)), &coreargs);
	GetCoreArgs(AMenuBarPtrWidget(DxDiffDisplayPtrMenuBar(new)), &sparecore);

	CoreBorderWidth(coreargs) = 0;
	CoreX(coreargs) = 0;
	CoreY(coreargs) = CoreY(sparecore) + CoreBorderWidth(sparecore) + CoreHeight(sparecore);
	CoreHeight(coreargs) -= CoreY(coreargs);


	ADBConstraintTopAttachment(constraintargs) = XmATTACH_WIDGET;
	ADBConstraintTopWidget(constraintargs) = 
		(XtArgVal)AMenuBarPtrWidget(DxDiffDisplayPtrMenuBar(new));
	ADBConstraintBottomAttachment(constraintargs) = XmATTACH_FORM;

	DxDiffDisplayPtrDisplayADB(new) = CreateMainADB(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)),
							"dxdiffdisplay", &coreargs, &dialogargs, &constraintargs);

	if (DxDiffDisplayPtrDisplayADB(new) == (MainADBPtr)NULL) {
		XtDestroyWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)));
		return ((DxDiffDisplayPtr)NULL);
	}

	/* o.k so now lets create the text display's and the diff region */

	GetCoreArgs(MainADBPtrWidget(DxDiffDisplayPtrDisplayADB(new)), &coreargs);

	CoreX(coreargs) = CoreY(coreargs) = 0;
	CoreBorderWidth(coreargs) = 0;
	CoreWidth(coreargs) = (XtArgVal)((double)CoreWidth(coreargs) / 100.0 * (float)percentagewidth);

	ADBConstraintTopAttachment(constraintargs) = ADBConstraintLeftAttachment(constraintargs) = XmATTACH_FORM;
	ADBConstraintRightAttachment(constraintargs) = XmATTACH_SELF;

	nextdiff.mnemonic.value = 'N';
	prevdiff.mnemonic.value = 'P';

	DxDiffDisplayPtrLeftTextADB(new) = 
		(TextDisplayADBPtr)CreateTextDisplayADB(
			MainADBPtrWidget(DxDiffDisplayPtrDisplayADB(new)),
			"textregiondisplay", &coreargs, &constraintargs,
			LeftFile, (caddr_t)new
			   );

	if (DxDiffDisplayPtrLeftTextADB(new) == (TextDisplayADBPtr)NULL) {
		XtDestroyWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)));
		return ((DxDiffDisplayPtr)NULL);
	}

	GetCoreArgs(MainADBPtrWidget(DxDiffDisplayPtrDisplayADB(new)), &coreargs);
	GetCoreArgs(TextDisplayADBPtrWidget(DxDiffDisplayPtrLeftTextADB(new)), &sparecore);

	CoreBorderWidth(coreargs) = CoreBorderWidth(sparecore);
	CoreX(coreargs) = CoreWidth(coreargs) - CoreWidth(sparecore);
	CoreY(coreargs) = CoreY(sparecore);
	CoreWidth(coreargs) = CoreWidth(sparecore);

	ADBConstraintRightAttachment(constraintargs) = XmATTACH_FORM;
	ADBConstraintLeftAttachment(constraintargs) = XmATTACH_SELF;

	nextdiff.mnemonic.value = 'x';
	prevdiff.mnemonic.value = 'v';

	DxDiffDisplayPtrRightTextADB(new) = 
		(TextDisplayADBPtr)CreateTextDisplayADB(
			MainADBPtrWidget(DxDiffDisplayPtrDisplayADB(new)),
			"textregiondisplay", &coreargs, &constraintargs,
			RightFile, (caddr_t)new
			    );

	if (DxDiffDisplayPtrRightTextADB(new) == (TextDisplayADBPtr)NULL) {
		XtDestroyWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)));
		return ((DxDiffDisplayPtr)NULL);
	}

	GetCoreArgs(TextDisplayADBPtrWidget(DxDiffDisplayPtrLeftTextADB(new)), &sparecore);

	CoreX(coreargs) = CoreX(sparecore) + CoreWidth(sparecore) + CoreBorderWidth(sparecore);
	CoreHeight(coreargs) = CoreHeight(sparecore);
	CoreY(coreargs) = CoreY(sparecore);

	GetCoreArgs(TextDisplayADBPtrWidget(DxDiffDisplayPtrRightTextADB(new)), &sparecore);

	CoreWidth(coreargs) = CoreX(sparecore) - CoreX(coreargs) - CoreBorderWidth(sparecore);


	ADBConstraintRightAttachment(constraintargs) =  ADBConstraintLeftAttachment(constraintargs) = XmATTACH_WIDGET;
	ADBConstraintRightWidget(constraintargs) = 
		(XtArgVal)TextDisplayADBPtrWidget(DxDiffDisplayPtrRightTextADB(new));
	ADBConstraintLeftWidget(constraintargs) = 
		(XtArgVal)TextDisplayADBPtrWidget(DxDiffDisplayPtrLeftTextADB(new)); 

	DxDiffDisplayPtrDiffRegionADB(new) = 
		(DiffRegionADBPtr)CreateDiffRegionADB(
			MainADBPtrWidget(DxDiffDisplayPtrDisplayADB(new)),
			 "diffregion", &coreargs, &constraintargs, &dialogargs, 
			DxDiffDisplayPtrLeftTextADB(new),
			DxDiffDisplayPtrRightTextADB(new),
			new
		     );


	if (DxDiffDisplayPtrDiffRegionADB(new) == (DiffRegionADBPtr)NULL) {
		XtDestroyWidget(MainADBPtrWidget(DxDiffDisplayPtrMainADB(new)));
		return ((DxDiffDisplayPtr)NULL);
	}

	DxDiffDisplayPtrLFileSelector(new) = DxDiffDisplayPtrRFileSelector(new) = (FileSelectorPtr)NULL;
	
	return	new;
}

/********************************
 *
 *	FreeBackEndStore
 *
 ********************************/

void
FreeBackEndStore(caches)
	BackEndCachePtr	caches;
{
	if (caches->cachesempty)
		return;

	_freestorecache(&(caches->nssc));
	_freestorecache(&(caches->edcsc));
	_freestorecache(&(caches->dflsc));
	_freestorecache(&(caches->bfnsc));
	_freestorecache(&(caches->cfnsc));
	_freestorecache(&(caches->ofnsc));
	_freestorecache(&(caches->ifnsc));
	_freestorecache(&(caches->densc));
	_freestorecache(&(caches->dcnsc));

	caches->cachesempty = True;
}

/********************************
 *
 *	InitDiffListBlk
 *
 ********************************/

InitDiffListBlk(difflistblkptr)
	register DiffListBlkPtr difflistblkptr;
{
	register NodeCommonPtr next, destroy;

	if (difflistblkptr == (DiffListBlkPtr)NULL) 
		return;

	if (difflistblkptr->caches != (BackEndCachePtr)NULL) {
		FreeBackEndStore(difflistblkptr->caches);
	} else {
		difflistblkptr->caches = (BackEndCachePtr)NULL;
	}
	
	bzero((char *)difflistblkptr, sizeof (DiffListBlk) - sizeof (difflistblkptr->caches));
}

/********************************
 *
 *	CreateNewDxDiffDisplay
 *
 ********************************/

static	int		 numactive = 0;
static	DxDiffDisplayPtr dxdiffdisplayvector[32], *firstfree = dxdiffdisplayvector;

DxDiffDisplayPtr
CreateNewDxDiffDisplay(parent, name, core)
	Widget		parent;
	char		*name;
	CoreArgListPtr	core;	/* not used ! */
{
	extern	Widget		 toplevel;

	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)NULL;

	if (numactive <= sizeof dxdiffdisplayvector / sizeof (DxDiffDisplayPtr)) {
		int	idx = firstfree - dxdiffdisplayvector;

		if (parent == (Widget)NULL) {	/* create a new Application shell */
			parent = XtAppCreateShell(DXDIFFNAME, DXDIFFCLASS, applicationShellWidgetClass,
						  XtDisplay(toplevel), 0, 0);

			if (parent == (Widget)NULL) {
				return dxdiffdisplay;
			}
		}

		if ((dxdiffdisplay = CreateDxDiffDisplay(parent, name, core)) == (DxDiffDisplayPtr)NULL)
			return dxdiffdisplay;

		numactive++;
		*firstfree++ = dxdiffdisplay;
		
		while (*firstfree != (DxDiffDisplayPtr)NULL &&
		       firstfree <= dxdiffdisplayvector + (sizeof dxdiffdisplayvector / sizeof(DxDiffDisplayPtr)))
			firstfree++;	/* find the next free slot in the vector */

		DxDiffDisplayPtrDiffList(dxdiffdisplay) = (DiffListBlkPtr)NULL;
		DxDiffDisplayPtrDisplayIdx(dxdiffdisplay) = idx;

		{
			char	buf[80];
			Arg	arg;

    		        /* Only print :x if this is not the main window. */
		        if ( DxDiffDisplayPtrDisplayIdx(dxdiffdisplay) == 0 )
		            sprintf(buf,"Differences" );
			else sprintf(buf, "%s: %1d", "Differences", DxDiffDisplayPtrDisplayIdx(dxdiffdisplay)); 

			arg.name = XtNtitle;
			arg.value = (XtArgVal)buf;

			XtSetValues(MainADBPtrParent(DxDiffDisplayPtrMainADB(dxdiffdisplay)), &arg, 1);
		}


		if (parent != toplevel) {
			XtRealizeWidget(parent);
		}
	}

	InitialConfigureOptions(dxdiffdisplay);

	return dxdiffdisplay;
}

/********************************
 *
 *	DxDiffDisplayDestroyed
 *
 ********************************/

DxDiffDisplayDestroyed(dxdiffdisplay)
	register DxDiffDisplayPtr	dxdiffdisplay;
{
	extern	Widget	toplevel;

	numactive--;
	if (dxdiffdisplayvector + DxDiffDisplayPtrDisplayIdx(dxdiffdisplay) < firstfree) {
		firstfree = dxdiffdisplayvector + DxDiffDisplayPtrDisplayIdx(dxdiffdisplay);
		*firstfree = (DxDiffDisplayPtr)NULL;
	}

	if (MainADBPtrParent(DxDiffDisplayPtrMainADB(dxdiffdisplay)) == toplevel) {
		exit(0);
	}
}
