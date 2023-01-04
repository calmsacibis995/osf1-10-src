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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/menu.c,v 1.1.4.2 1993/06/25 16:58:48 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	menu.c - generic menu handling stuff
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 29th April 1988
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

static char sccsid[] = "@(#)menu.c	1.7	17:45:33 2/21/89";

#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  DEBUG
#include <X11/Xlib.h>
#include <Xm/XmP.h>
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
 *	DestroyPushButton
 *
 ********************************/

DestroyPushButton(pushbutton)
	register PushButtonEntryPtr pushbutton;
{
		XtDestroyWidget(PushButtonEntryPtrWidget(pushbutton));
}

static void
PushButtonDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register PushButtonEntryPtr pushbutton = (PushButtonEntryPtr)clientd;

	if (PushButtonEntryPtrMainString(pushbutton) != (char *)NULL) {
		XtFree(PushButtonEntryPtrMainString(pushbutton));
	}

	if (PushButtonEntryPtrMainCString(pushbutton) != (XmString)NULL) {
		XtFree((char *)PushButtonEntryPtrMainCString(pushbutton));
	}

	if (PushButtonEntryPtrAlternativeString(pushbutton) != (char *)NULL) {
		XtFree(PushButtonEntryPtrAlternativeString(pushbutton));
	}

	if (PushButtonEntryPtrAlternativeCString(pushbutton) != (XmString)NULL) {
		XtFree((char *)PushButtonEntryPtrAlternativeCString(pushbutton));
	}

	XtFree((char *)pushbutton);
}

XtCallbackRec	PushButtonDestroyCallbackList[] = {
	{ (VoidProc)PushButtonDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};
	


/********************************
 *
 *	NewPushButton
 *
 ********************************/

PushButtonEntryPtr
NewPushButton(copy)
	register PushButtonEntryPtr copy;
{
	register PushButtonEntryPtr new;

	if ((new = (PushButtonEntryPtr)XtMalloc(sizeof (PushButtonEntry))) == 
	    (PushButtonEntryPtr)NULL) {	/* error */
		return new;
	}

	if (copy != (PushButtonEntryPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (PushButtonEntry));

		if(PushButtonEntryPtrAcceleratorText(copy) != NULL) {
			PushButtonEntryPtrAcceleratorText(new) =
			(XtArgVal)XmStringLtoRCreate(
			PushButtonEntryPtrAcceleratorText(copy), "ISO8859-1");
		}

		if (PushButtonEntryPtrMainCString(copy) != (XmString)NULL) {
			PushButtonEntryPtrMainCString(new) = 
				XmStringCopy(PushButtonEntryPtrMainCString(copy));
		}
		if (PushButtonEntryPtrAlternativeCString(copy) != (XmString)NULL) {
			PushButtonEntryPtrAlternativeCString(new) = 
				XmStringCopy(PushButtonEntryPtrAlternativeCString(copy));
		}
		if (PushButtonEntryPtrMainString(copy) != (char *)NULL) {
			PushButtonEntryPtrMainCString(new) = 
			XmStringLtoRCreate(
			PushButtonEntryPtrMainString(copy), "ISO8859-1");
		}

		if (PushButtonEntryPtrAlternativeString(copy) != (char *)NULL) {
				PushButtonEntryPtrAlternativeCString(new) = 
			
			XmStringLtoRCreate(
			PushButtonEntryPtrAlternativeString(copy), "ISO8859-1");
		}
	} else {
		InitPushButtonEntryPtrArgList(new);
		PushButtonEntryPtrActivateCallBack(new) = (XtArgVal)NULL;
		PushButtonEntryPtrSensitivity(new) = False;
		PushButtonEntryPtrWidget(new) = (Widget)NULL;
		PushButtonEntryPtrMainString(new) = (char *)NULL;
		PushButtonEntryPtrAlternativeString(new) = (char *)NULL;
		PushButtonEntryPtrButtonName(new) = (char *)NULL;
		PushButtonEntryPtrCurrentLabel(new) = NoLabel;
		PushButtonEntryPtrMainCString(new) = (XmString)NULL;
		PushButtonEntryPtrAlternativeCString(new) = (XmString)NULL;
	}

	return new;
}


/********************************
 *
 *	CreatePushButton
 *
 ********************************/

void
CreatePushButton(parent, pushbutton)
	Widget		            parent;
	register PushButtonEntryPtr pushbutton;
{

	PushButtonDestroyCallbackList[0].closure = pushbutton;

	PushButtonEntryPtrWidget(pushbutton) = 
		(Widget)XmCreatePushButtonGadget(
			parent,
			PushButtonEntryPtrButtonName(pushbutton),
			(ArgList)&PushButtonEntryPtrLabelArgList(pushbutton),
			(int)NumberOfArgsBetween(&PushButtonEntryPtrLabelArgList(pushbutton),
				            PointerToArg(PushButtonEntryPtrSensitivity(pushbutton)))
		);

	if (PushButtonEntryPtrWidget(pushbutton) == (Widget)NULL) { /* error */
	}

	XtManageChild(PushButtonEntryPtrWidget(pushbutton));
}

/********************************
 *
 *	CreateCascadeButton
 *
 ********************************/

void
CreateCascadeButton(parent, pushbutton)
	Widget		            parent;
	register PushButtonEntryPtr pushbutton;
{
	PushButtonDestroyCallbackList[0].closure = pushbutton;

	PushButtonEntryPtrWidget(pushbutton) = 
		(Widget)XmCreateCascadeButtonGadget(
			parent,
			PushButtonEntryPtrButtonName(pushbutton),
			(ArgList)&PushButtonEntryPtrLabelArgList(pushbutton),
			NumberOfArgsBetween(&PushButtonEntryPtrLabelArgList(pushbutton),
				            PointerToArg(PushButtonEntryPtrSensitivity(pushbutton)))
		);

	if (PushButtonEntryPtrWidget(pushbutton) == (Widget)NULL) { /* error */
	}

	XtManageChild(PushButtonEntryPtrWidget(pushbutton));
}

/********************************
 *
 *	DestroyPullDownMenu
 *
 ********************************/

DestroyPullDownMenu(pulldownmenu)
	PullDownMenuEntryPtr pulldownmenu;
{
	XtDestroyWidget(PullDownMenuEntryPtrWidget(pulldownmenu));
}

static void
PullDownMenuDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register PullDownMenuEntryPtr pulldownmenu = (PullDownMenuEntryPtr)clientd;

	if (PullDownMenuEntryPtrMenuName(pulldownmenu) != (char *)NULL) {
		XtFree((char *)PullDownMenuEntryPtrMenuName(pulldownmenu));
	}

	if (PullDownMenuEntryPtrPushButtons(pulldownmenu) != (PushButtonEntryPtr *)NULL) {
		XtFree((char *)PullDownMenuEntryPtrPushButtons(pulldownmenu));
	}

	XtFree((char *)pulldownmenu);
}

XtCallbackRec	PullDownMenuDestroyCallbackList[] = {
	{ (VoidProc)PullDownMenuDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};

/********************************
 *
 *	NewPullDownMenu
 *
 ********************************/

PullDownMenuEntryPtr
NewPullDownMenu(copy)
	register PullDownMenuEntryPtr copy;
{
	register PullDownMenuEntryPtr new;

	if ((new = (PullDownMenuEntryPtr)XtMalloc(sizeof (PullDownMenuEntry))) ==
	    (PullDownMenuEntryPtr)NULL) {	/* error */
		return new;
	}

	if (copy != (PullDownMenuEntryPtr)NULL) {
		register unsigned int num;
		register PushButtonEntryPtr *npb,*opb;

		bcopy((char *)copy, (char *)new, sizeof (PullDownMenuEntry));

		if (PullDownMenuEntryPtrMenuName(copy) != (char *)NULL) {
			PullDownMenuEntryPtrMenuName(new) = 
				XtMalloc(strlen(PullDownMenuEntryPtrMenuName(copy)) + 1);

			if (PullDownMenuEntryPtrMenuName(new) != (char *)NULL)
				strcpy(PullDownMenuEntryPtrMenuName(new),
				       PullDownMenuEntryPtrMenuName(copy));

			if (!(num = PullDownMenuEntryPtrNumButtons(new)))
				return new;;

			if ((PullDownMenuEntryPtrPushButtons(new) = 
			     (PushButtonEntryPtr *)XtMalloc(num * sizeof (PushButtonEntryPtr)))
			    == (PushButtonEntryPtr *)NULL) {	/* error */
				return new;
			}

			for (npb = PullDownMenuEntryPtrPushButtons(new),
			     opb = PullDownMenuEntryPtrPushButtons(copy);
			     num-- > 0; npb++,opb++) {
				*npb = NewPushButton(*opb);
			}
		}
	} else {
		InitPullDownMenuEntryPtrArgList(new);
		PullDownMenuEntryPtrWidget(new) = (Widget)NULL;
		PullDownMenuEntryPtrMenuName(new) = (char *)NULL;
		PullDownMenuEntryPtrNumButtons(new) = 0;
		PullDownMenuEntryPtrPushButtons(new) = (PushButtonEntryPtr *)NULL;
	}
	
	return new;
}


/********************************
 *
 *	CreatePullDownMenu
 *
 ********************************/

#ifdef HYPERHELP
void
CreatePullDownMenu(parent, pulldownmenu, contexthelp)
	Widget				parent;
	register PullDownMenuEntryPtr	pulldownmenu;
	Arg				*contexthelp;
#else
void
CreatePullDownMenu(parent, pulldownmenu)
	Widget				parent;
	register PullDownMenuEntryPtr	pulldownmenu;
#endif
{
	register PushButtonEntryPtr	*buttons;
	register unsigned int		numbuttons;
	register Widget			widget = parent;

#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2)
	Arg arg_list[ 1 ];
#endif

	PullDownMenuDestroyCallbackList[0].closure = pulldownmenu;

	PullDownMenuEntryPtrWidget(pulldownmenu) =
		(Widget)XmCreatePulldownMenu(
			parent,
			PullDownMenuEntryPtrMenuName(pulldownmenu),
#ifdef HYPERHELP
			contexthelp, 1
#else
			(ArgList)NULL, 0
#endif
		);

	if (PullDownMenuEntryPtrWidget(pulldownmenu) == (Widget)NULL) {
		return;
	}


#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2)
	XtSetArg( arg_list[ 0 ], XmNtearOffModel, (XtArgVal)XmTEAR_OFF_ENABLED );
        XtSetValues( PullDownMenuEntryPtrWidget(pulldownmenu), arg_list, 1 );
#endif;
	for (buttons = PullDownMenuEntryPtrPushButtons(pulldownmenu),
	     numbuttons = PullDownMenuEntryPtrNumButtons(pulldownmenu);
	     numbuttons-- > 0; buttons++) {
		CreatePushButton(PullDownMenuEntryPtrWidget(pulldownmenu), *buttons);
	}
}

/********************************
 *
 *	DestroyPullDownEntry
 *
 ********************************/

DestroyPullDown(pulldown)
	register PullDownEntryPtr pulldown;
{
	XtDestroyWidget(PullDownEntryPtrWidget(pulldown));
}

static void
PullDownDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register PullDownEntryPtr pulldownentry;

	if (PullDownEntryPtrEntryName(pulldownentry) != (char *)NULL) {
		XtFree((char *)PullDownEntryPtrEntryName(pulldownentry));
	}

	if (PullDownEntryPtrMainString(pulldownentry) != (char *)NULL) {
		XtFree((char *)PullDownEntryPtrMainString(pulldownentry));
	}

	if (PullDownEntryPtrMainCString(pulldownentry) != (XmString)NULL) {
		XtFree((char *)PullDownEntryPtrMainCString(pulldownentry));
	}

	XtFree((char *)pulldownentry);
}

XtCallbackRec	PullDownDestroyCallbackList[] = {
	{ (VoidProc)PullDownDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};


/********************************
 *
 *	NewPullDownEntry
 *
 ********************************/

PullDownEntryPtr
NewPullDownEntry(copy)
	register PullDownEntryPtr copy;
{
	register PullDownEntryPtr new;

	if ((new = (PullDownEntryPtr)XtMalloc(sizeof (PullDownEntry)))
	    == (PullDownEntryPtr)NULL) {	/*  error */
		return new;
	}

	if (copy != (PullDownEntryPtr)NULL) {
		bcopy((char *)copy, (char *)new, sizeof (PullDownEntry));

		if (PullDownEntryPtrEntryName(copy) != (char *)NULL) {
			PullDownEntryPtrEntryName(new) =
				XtMalloc(strlen(PullDownEntryPtrEntryName(copy)) + 1);
			if (PullDownEntryPtrEntryName(new) != (char *)NULL) {
				strcpy(PullDownEntryPtrEntryName(new),
				       PullDownEntryPtrEntryName(copy));
			}
		}

		if (PullDownEntryPtrMainCString(copy) != (XmString)NULL) {
			PullDownEntryPtrMainCString(new) =
				XmStringCopy(PullDownEntryPtrMainCString(copy));
		}
		if (PullDownEntryPtrMainString(copy) != (char *)NULL) {
			PullDownEntryPtrMainString(new) =
				XtMalloc(strlen(PullDownEntryPtrMainString(copy) + 1));
			if (PullDownEntryPtrMainString(new) != (char *)NULL) {
					strcpy(PullDownEntryPtrMainString(new),
					       PullDownEntryPtrMainString(copy));
				}
			if (PullDownEntryPtrMainCString(new) == (XmString)NULL) {
				PullDownEntryPtrMainCString(new) =
				XmStringLtoRCreate(
				PullDownEntryPtrMainString(new), "ISO8859-1");
			}
		}

		if (PullDownEntryPtrPullDownMenuEntry(copy) != (PullDownMenuEntryPtr)NULL) {
			PullDownEntryPtrPullDownMenuEntry(new) = 
				NewPullDownMenu(PullDownEntryPtrPullDownMenuEntry(copy));
		}
	} else {
		InitPullDownEntryPtrArgList(new);
		PullDownEntryPtrSubMenuWidget(new) = NULL;
		PullDownEntryPtrSensitivity(new) = False;
		PullDownEntryPtrWidget(new) = (Widget)NULL;
		PullDownEntryPtrEntryName(new) = (char *)NULL;
		PullDownEntryPtrMainString(new) = (char *)NULL;
		PullDownEntryPtrPullDownMenuEntry(new) = (PullDownMenuEntryPtr)NULL;
	}
	
	return new;
}

/********************************
 *
 *	CreatePullDownEntry
 *
 ********************************/

void
CreatePullDownEntry(parent, pulldownentry)
	Widget			  parent;
	register PullDownEntryPtr pulldownentry;
{

#ifdef HYPERHELP
	CreatePullDownMenu(PullDownEntryPtrSubMenuParent(pulldownentry), 
			   PullDownEntryPtrPullDownMenuEntry(pulldownentry),
			   &(pulldownentry->contexthelp));
#else
	CreatePullDownMenu(PullDownEntryPtrSubMenuParent(pulldownentry), 
			   PullDownEntryPtrPullDownMenuEntry(pulldownentry));
#endif

	if (PullDownMenuEntryPtrWidget( PullDownEntryPtrPullDownMenuEntry(pulldownentry)) 
	    == (Widget)NULL) {
		return;	/* error */
	}

	PullDownDestroyCallbackList[0].closure = pulldownentry;

	PullDownEntryPtrSubMenuWidget(pulldownentry) =
		(XtArgVal)PullDownMenuEntryPtrWidget(PullDownEntryPtrPullDownMenuEntry(pulldownentry));

	PullDownEntryPtrWidget(pulldownentry) =
		(Widget)XmCreateCascadeButton(
			parent,
			PullDownEntryPtrEntryName(pulldownentry),
			(ArgList)&PullDownEntryPtrLabelArgList(pulldownentry),
			NumberOfArgsBetween(&PullDownEntryPtrLabelArgList(pulldownentry),
				            &PullDownEntryPtrSensitivity(pulldownentry))
		);

	if (PullDownEntryPtrWidget(pulldownentry) == (Widget)NULL) {
	}	/* error */

	XtManageChild(PullDownEntryPtrWidget(pulldownentry));
}

/********************************
 *
 *	DestroyMenuEntry
 *
 ********************************/

DestroyMenuEntry(menuentry)
	register MenuEntryPtr menuentry;
{
#if	0	/* we dont actually do this because the widget destroy callback should */
	switch (MenuEntryPtrType(menuentry)) {
		EntryIsPushButton:
		EntryIsCascadeButton:
			DestroyPushButton(MenuEntryPtrPushButtonPtr(menuentry));
			break;

		EntryIsPullDown:
			DestroyPullDown(MenuEntryPtrPullDownPtr(menuentry));
			break;

		default:
			break;
	}
#endif

	XtFree((char *)menuentry);
}


/********************************
 *
 *	NewMenuEntry
 *
 ********************************/

MenuEntryPtr
NewMenuEntry(copy)
	register MenuEntryPtr copy;
{
	register MenuEntryPtr new;

	if ((new = (MenuEntryPtr)XtMalloc(sizeof (MenuEntry))) == (MenuEntryPtr)NULL) {
		return new;
	}

	if (copy == (MenuEntryPtr)NULL) {
		MenuEntryPtrType(new) = (MenuEntryEnum)(-1); /* ?? */
		MenuEntryPtrPullDown(new) = NULL;
	} else {
		switch (MenuEntryPtrType(copy)) {

			case EntryIsPushButton:
				MenuEntryPtrType(new) = EntryIsPushButton;
				MenuEntryPtrPushButton(new) = (PushButtonOrPullDownPtr)
					NewPushButton(MenuEntryPtrPushButton(copy));
				break;

			case EntryIsPullDown:
				MenuEntryPtrType(new) = EntryIsPullDown;
				MenuEntryPtrPullDown(new) =(PushButtonOrPullDownPtr)
					NewPullDownEntry(MenuEntryPtrPullDown(copy));
				break;

			case EntryIsCascadeButton:
				MenuEntryPtrType(new) = EntryIsCascadeButton;
				MenuEntryPtrPushButton(new) = (PushButtonOrPullDownPtr)
					NewPushButton(MenuEntryPtrPushButton(copy));
				break;

			default:	/* error */
				break;
		}
	}

	return new;
}

/********************************
 *
 *	CreateMenuEntry
 *
 ********************************/

void
CreateMenuEntry(parent, menuentry)
	Widget		      parent;
	register MenuEntryPtr menuentry;
{
	switch (MenuEntryPtrType(menuentry)) {

		case EntryIsPushButton:
			CreatePushButton(parent, MenuEntryPtrPushButton(menuentry));
			break;

		case EntryIsPullDown:
			CreatePullDownEntry(parent, MenuEntryPtrPullDown(menuentry));
			break;

		case EntryIsCascadeButton:
			CreateCascadeButton(parent, MenuEntryPtrPushButton(menuentry));
			break;

		default:	/* error */
			break;
	}
}
		

/********************************
 *
 *	DestroyAMenuBar
 *
 ********************************/

DestroyAMenuBar(menubar)
	register AMenuBarPtr menubar;
{
	XtDestroyWidget(AMenuBarPtrWidget(menubar));
}

static void
AMenuBarDestroyCallBack(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register AMenuBarPtr   menubar;
	register MenuEntryPtr *entries;
	int	 	      num;

	if (AMenuBarPtrMenuName(menubar) != (char *)NULL) {
		XtFree((char *)AMenuBarPtrMenuName(menubar));
	}
	
	num = AMenuBarPtrNumEntries(menubar);

	for (entries = AMenuBarPtrEntries(menubar); num-- > 0; entries++)
		DestroyMenuEntry(*entries);

	if (AMenuBarPtrEntries(menubar) != NULL) {
		XtFree((char *)AMenuBarPtrEntries(menubar));
	}
	
	XtFree((char *)menubar);
}

XtCallbackRec	AMenuBarDestroyCallbackList[] = {
	{ (VoidProc)AMenuBarDestroyCallBack, 0 },
	{ (VoidProc)NULL, 0 }
};


/********************************
 *
 *	NewAMenuBar
 *
 ********************************/

AMenuBarPtr
NewAMenuBar(copy)
	register AMenuBarPtr copy;
{
	register AMenuBarPtr new;

	if ((new = (AMenuBarPtr)XtMalloc(sizeof(AMenuBar))) == (AMenuBarPtr)NULL) {
		return new;
	}

	if (copy != (AMenuBarPtr)NULL) {
		register MenuEntryPtr *nme,*ome;
		register unsigned int num;
 
		bcopy((char *)copy, (char *)new, sizeof (AMenuBar));

		if (AMenuBarPtrMenuName(copy) != (char *)NULL) {
			AMenuBarPtrMenuName(new) =
				XtMalloc(strlen(AMenuBarPtrMenuName(copy)) + 1);
			if (AMenuBarPtrMenuName(new) != (char *)NULL) {
				strcpy(AMenuBarPtrMenuName(new),
				       AMenuBarPtrMenuName(copy));
			}
		}

		if (!(num = AMenuBarPtrNumEntries(copy)))
			return new;

		if ((AMenuBarPtrEntries(new) =
		     (MenuEntryPtr *)XtMalloc(num * sizeof(MenuEntryPtr))) == NULL) {
			return new;
		}

		for (nme = AMenuBarPtrEntries(new),
		     ome = AMenuBarPtrEntries(copy);
		     num-- > 0; nme++,ome++) {
				*nme = NewMenuEntry(*ome);
		}
	} else {
		InitAMenuBarPtrArgList(new);
		AMenuBarPtrWidget(new) = (Widget)NULL;
		AMenuBarPtrMenuName(new) = (char *)NULL;
		AMenuBarPtrNumEntries(new) = 0;
		AMenuBarPtrEntries(new) = (MenuEntryPtr *)NULL;
	}

	return new;
}

/********************************
 *
 *	CreateAMenuBar
 *
 ********************************/

void
CreateAMenuBar(parent, amenubar)
	Widget		     parent;
	register AMenuBarPtr amenubar;
{
	register MenuEntryPtr *entries;
	register unsigned int numentries;

	AMenuBarDestroyCallbackList[0].closure = amenubar;

	AMenuBarPtrWidget(amenubar) = (Widget)XmCreateMenuBar(
					parent,
					AMenuBarPtrMenuName(amenubar),
					(ArgList)&AMenuBarPtrConstraintArgList(amenubar),
					NumberOfArgsInArgListStruct(ADBConstraintArgList) +
					NumberOfArgsInArgListStruct(MenuBarArgList)
				      );

	if (AMenuBarPtrWidget(amenubar) == (Widget)NULL) {	/* error */
		return;
	}

	XtManageChild(AMenuBarPtrWidget(amenubar));


	for (entries = AMenuBarPtrEntries(amenubar),
	     numentries = AMenuBarPtrNumEntries(amenubar);
	     numentries-- > 0;
	     entries++) {
		if (MenuEntryPtrType(*entries) == EntryIsPullDown) {
			PullDownEntryPtrSubMenuParent(MenuEntryPtrPullDownEntryPtr(*entries)) = AMenuBarPtrWidget(amenubar);
		}
		CreateMenuEntry(AMenuBarPtrWidget(amenubar), *entries);
	}
}


/********************************
 *
 *	FindAMenuBarHeightFudge
 *
 ********************************/

int
FindAMenuBarHeightFudge(amenubar)
	register AMenuBarPtr	amenubar;
{
	Arg	 	      args[2];
	register MenuEntryPtr *entries;
	register unsigned int numentries;
	Dimension	      borderwidth;
	XmFontList	      *fontListP = NULL;
	XFontStruct	      *xfont;
	int		      fontIndex;

	
	args[0].name = XmNborderWidth;
	args[0].value = (XtArgVal)&borderwidth;

	args[1].name = XmNfontList;
;
	args[1].value = (XtArgVal)&fontListP;


	for (entries = AMenuBarPtrEntries(amenubar),
	     numentries = AMenuBarPtrNumEntries(amenubar);
	     numentries-- > 0;
	     entries++) {
		int			width = 0, height = 0, left = 0, right = 0, top = 0, bottom = 0,
					mw,    mh,     ml,   mr,    mt,  mb, h;
		Widget			obj,label;
		extern	WidgetClass	xmRowColumnWidgetClass;

		switch (MenuEntryPtrType(*entries)) {
			case EntryIsPushButton:
			case EntryIsCascadeButton:
				label = PushButtonEntryPtrWidget(MenuEntryPtrPushButtonEntryPtr(*entries));

				obj = label;
				while (obj != (Widget)NULL && !XmIsGadget(obj))
					obj = XtParent(obj);
				
				width = height = right = left = top = bottom= 2;
				break;
			case EntryIsPullDown:
				obj = label = PullDownEntryPtrWidget(MenuEntryPtrPullDownEntryPtr(*entries));
			break;
		}
		
		if (!XtIsSubclass(obj, xmRowColumnWidgetClass))
			obj = AMenuBarPtrWidget(amenubar);

			mw = mh = ml = mr = mt = mb = 2;

		if (height > mh) mh = height;
		if (top > mt) mt = top;
		if (bottom > mb) mb = bottom;

		XtGetValues(obj, args, sizeof args / sizeof args[0]);
		
		if (fontListP == NULL) {
		    xfont = XLoadQueryFont(XtDisplay(obj), "fixed");
		} else {
		    _XmFontListSearch(fontListP, "default", &fontIndex, &xfont);
		}

		return (2 * (borderwidth + mh) + mt + mb + (xfont->ascent + xfont->descent)) + 30;
	}

}


/********************************
 *
 *	SetPushButtonSensitivity
 *
 ********************************/

void
SetPushButtonSensitivity(pushbutton, state)
	register PushButtonEntryPtr pushbutton;
	Boolean			    state;
{
	PushButtonEntryPtrSensitivity(pushbutton) = state;
	XtSetValues(PushButtonEntryPtrWidget(pushbutton),
		    PointerToArg(PushButtonEntryPtrSensitivity(pushbutton)),
		    1);
}



/********************************
 *
 *	SetPushButtonToMainLabel
 *
 ********************************/

void
SetPushButtonToMainLabel(pushbutton)
	register PushButtonEntryPtr pushbutton;
{
	if (PushButtonEntryPtrMainString(pushbutton) != (char *)NULL) {
		PushButtonEntryPtrMainCString(pushbutton) =
		    XmStringLtoRCreate(
			PushButtonEntryPtrMainString(pushbutton), "ISO8859-1");
	}

	if (PushButtonEntryPtrMainCString(pushbutton) != (XmString)NULL) {
		LabelLabel(PushButtonEntryPtrLabelArgList(pushbutton)) =
			(XtArgVal)PushButtonEntryPtrMainCString(pushbutton);
	} else
		return;

	PushButtonEntryPtrCurrentLabel(pushbutton) = MainLabel;
	
	XtSetValues(PushButtonEntryPtrWidget(pushbutton),
		    PointerToArg(LabelLabel(PushButtonEntryPtrLabelArgList(pushbutton))),
		    1);
}



/********************************
 *
 *	SetPushButtonToAlternativeLabel
 *
 ********************************/

void
SetPushButtonToAlternativeLabel(pushbutton)
	register PushButtonEntryPtr pushbutton;
{
	if (PushButtonEntryPtrAlternativeString(pushbutton) != (char *)NULL) {
		PushButtonEntryPtrAlternativeCString(pushbutton) =
		    XmStringLtoRCreate(
		    PushButtonEntryPtrAlternativeString(pushbutton), "ISO8859-1");
	}

	if (PushButtonEntryPtrAlternativeCString(pushbutton) != (XmString)NULL) {
		LabelLabel(PushButtonEntryPtrLabelArgList(pushbutton)) =
			(XtArgVal)PushButtonEntryPtrAlternativeCString(pushbutton);
	} else
		return;
	
	PushButtonEntryPtrCurrentLabel(pushbutton) = AlternateLabel;

	XtSetValues(PushButtonEntryPtrWidget(pushbutton),
		    PointerToArg(LabelLabel(PushButtonEntryPtrLabelArgList(pushbutton))),
		    1);
}

