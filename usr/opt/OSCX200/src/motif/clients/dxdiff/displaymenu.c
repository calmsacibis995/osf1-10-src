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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/displaymenu.c,v 1.1.4.2 1993/06/25 16:57:57 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	display.c - display handler code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 5th May 1988
 *
 *
 *	Description
 *	-----------
 *
 *
 *	Modification History
 *	------------ -------
 *	
 *	8th May 1988	Laurence P. G. Cable
 *
 *	Blundered! altered the core arglist copy code so as not to overwrite
 *	the destroy callback info copied from the static block!
 *
 *	31st May 1988	Laurence P. G. Cable
 *
 *	Sorry! Ive pulled the edit option I just dont have the time!
 */

static char sccsid[] = "@(#)displaymenu.c	1.2	18:19:19 5/10/88";


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
#include "displaymenu.h"

#ifdef	EDITBTN
extern	XtCallbackRec	SkipToNextPrevDiffCallbackList[],
			EditCallbackList[];
#else	EDITBTN
extern	XtCallbackRec	SkipToNextPrevDiffCallbackList[];
#endif	EDITBTN

extern	XtCallbackRec	PushButtonDestroyCallbackList[],
#ifdef HYPERHELP
			MovingBetweenSymbol[],
#endif
			AMenuBarDestroyCallbackList[];


/********************************
 *
 *      Display Menu Template
 *
 ********************************/


/********************************
 *
 *      Display Menu 'Next Difference'
 *
 ********************************/


PushButtonEntry	nextdiff = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)SkipToNextPrevDiffCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) MovingBetweenSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('N'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Next Diff"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("nextdiff"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};




/********************************
 *
 *      Display Menu 'Prev Difference'
 *
 ********************************/


PushButtonEntry	prevdiff = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)SkipToNextPrevDiffCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) MovingBetweenSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('P'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Prev Diff"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("prevdiff"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      Display Menu 'Edit'
 *
 ********************************/

#ifdef	EDITBTN
static	PushButtonEntry	edit = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, (char *)NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack(EditCallbackList),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, (XmFontList)NULL),
	StaticInitPushButtonEntryWidget((Widget)NULL),
	StaticInitPushButtonEntryMainString("Edit"),
	StaticInitPushButtonEntryAlternativeString((char  *)NULL),
	StaticInitPushButtonEntryButtonName("edit"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString((XmString)NULL),
	StaticInitPushButtonEntryAlternativeCString((XmString)NULL)
};
#endif	EDITBTN

/********************************
 *
 *	Display Menu
 *
 ********************************/

static MenuEntry nextdiffme = {
	EntryIsCascadeButton,
	(PushButtonOrPullDownPtr)&nextdiff
};

static MenuEntry prevdiffme = {
	EntryIsCascadeButton,
	(PushButtonOrPullDownPtr)&prevdiff
};

#ifdef	EDITBTN
static MenuEntry editme = {
	EntryIsCascadeButton,
	(PushButtonOrPullDownPtr)&edit
};
#endif	EDITBTN

static MenuEntry *displaymenuentries[] = {
	&nextdiffme,
#ifdef	EDITBTN
	&prevdiffme,
	&editme
#else	EDITBTN
	&prevdiffme
#endif	EDITBTN
};



static AMenuBar displaymenubar = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)AMenuBarDestroyCallbackList),
	StaticInitADBConstraintArgList(XmATTACH_FORM,XmATTACH_SELF,
				       XmATTACH_FORM, XmATTACH_FORM,
				       NULL, NULL,
				       NULL, NULL,
				       0,0,0,0),
	StaticInitMenuBarArgList(XmHORIZONTAL, XmMENU_BAR),
	StaticInitAMenuBarWidget(NULL),
	StaticInitAMenuBarMenuName("displaymenu"),
	StaticInitAMenuBarNumEntries(sizeof (displaymenuentries) /
				     sizeof (MenuEntryPtr)),
	StaticInitAMenuBarEntries(displaymenuentries)
};

/********************************
 *
 *      CreateNewDisplayMenu
 *
 ********************************/

AMenuBarPtr
CreateDisplayMenu(parent, core, constraints, menubar, closure)
        Widget                  parent;
        CoreArgListPtr          core;
        ADBConstraintArgListPtr constraints;
        MenuBarArgListPtr       menubar;
        caddr_t                 closure;
{
        register AMenuBarPtr          new;
	register PushButtonEntryPtr   pb;


	if ((new = (AMenuBarPtr)NewAMenuBar(&displaymenubar)) == (AMenuBarPtr)NULL) {
		return new;
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&AMenuBarPtrCoreArgList(new),
		      sizeof(CoreArgList) - sizeof (Arg));	/* save the destroy callback */
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&AMenuBarPtrConstraintArgList(new),
		      sizeof(ADBConstraintArgList));
	}

	if (menubar != (MenuBarArgListPtr)NULL) {
		bcopy((char *)menubar, (char *)&AMenuBarPtrMenuBarArgList(new),
		      sizeof(MenuBarArgList));
	}


	pb = (PushButtonEntryPtr)MenuEntryPtrPushButton(
		AMenuBarPtrEntries(new)[(int)NextDiffButton]);
	LabelLabel(PushButtonEntryPtrLabelArgList(pb)) = (XtArgVal)
		PushButtonEntryPtrMainCString(pb);
	PushButtonEntryPtrCurrentLabel(pb) = MainLabel;


	pb = (PushButtonEntryPtr)MenuEntryPtrPushButton(
		AMenuBarPtrEntries(new)[(int)PrevDiffButton]);
	LabelLabel(PushButtonEntryPtrLabelArgList(pb)) = (XtArgVal)
		PushButtonEntryPtrMainCString(pb);
	PushButtonEntryPtrCurrentLabel(pb) = MainLabel;


#ifdef	EDITCMD
	pb = MenuEntryPtrPushButton(AMenuBarPtrEntries(new)[(int)EditButton]);
	LabelLabel(PushButtonEntryPtrLabelArgList(pb)) =
		PushButtonEntryPtrMainCString(pb);
	PushButtonEntryPtrCurrentLabel(pb) = MainLabel;
#endif	EDITCMD


	SkipToNextPrevDiffCallbackList[0].closure = closure;
#ifdef	EDITCMD
	EditCallbackList[0].tag = closure;
#endif	EDITCMD

	CreateAMenuBar(parent, new);

	return new;
}
