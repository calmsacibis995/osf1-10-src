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
/* BuildSystemHeader added automatically */
/* $Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/menu.h,v 1.1.4.2 1993/06/25 16:58:56 Lynda_Rice Exp $ */
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
 *	menu.h - menu code
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 28th April 1988
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

#ifndef	MENU_H
#define	MENU_H

/********************************
 *
 *	PushButtonEntry
 *
 ********************************/

typedef	enum	_labelactive	{
	NoLabel,
	MainLabel,
	AlternateLabel
} LabelActive;

typedef	struct	_pushbuttonentry {
	CoreArgList	core;
	LabelArgList	label;
	Arg		activatecallback;
#ifdef HYPERHELP
	Arg		contexthelp;
#endif
	Arg		mnemonic;
	Arg		accelerator;
	Arg		acceleratortext;
	Arg		sensitivity;
	FontArgList	font;			/* not used */

	Widget		w;
	char		*main,
			*alternative,
			*buttonname;

	LabelActive	currentlabel;

	XmString	maincs,
			alternativecs;
}	PushButtonEntry, *PushButtonEntryPtr;
	
#ifdef HYPERHELP

#define	InitPushButtonEntryPtrArgList(p)				\
					InitCoreArgList((p)->core);	\
					InitLabelArgList((p)->label);	\
					(p)->activatecallback.name = 	\
						XmNactivateCallback;	\
					(p)->contexthelp.name =		\
					        XmNhelpCallback;	\
					(p)->mnemonic.name =		\
					        XmNmnemonic;		\
					(p)->accelerator.name =		\
					        XmNaccelerator;		\
					(p)->acceleratortext.name =	\
					        XmNacceleratorText;	\
					(p)->sensitivity.name =		\
						XmNsensitive;		\
					InitFontArgList((p)->font)

#else

#define	InitPushButtonEntryPtrArgList(p)				\
					InitCoreArgList((p)->core);	\
					InitLabelArgList((p)->label);	\
					(p)->activatecallback.name = 	\
						XmNactivateCallback;	\
					(p)->mnemonic.name =		\
					        XmNmnemonic;		\
					(p)->accelerator.name =		\
					        XmNaccelerator;		\
					(p)->acceleratortext.name =	\
					        XmNacceleratorText;	\
					(p)->sensitivity.name =		\
						XmNsensitive;		\
					InitFontArgList((p)->font)

#endif

#define	InitPushButtonEntryArgList(p)	InitPushButtonEntryPtrArgList(&(p))

#define PushButtonEntryPtrCoreArgList(p)	((p)->core)
#define	PushButtonEntryCoreArgList(p)	PushButtonEntryPtrCoreArgList(&(p))

#define PushButtonEntryPtrLabelArgList(p)	((p)->label)
#define	PushButtonEntryLabelArgList(p)		\
					PushButtonEntryPtrLabelArgList(&(p))

#define	PushButtonEntryPtrActivateCallBack(p)	((p)->activatecallback.value)
#define	PushButtonEntryActivateCallBack(p)	\
					PushButtonEntryPtrActivateCallBack(&(p))

#define PushButtonEntryPtrContextHelp(p)	((p)->contexthelp.value)
#define	PushButtonEntryContextHelp(p)		\
					PushButtonEntryPtrContextHelp(&(p))

#define PushButtonEntryPtrMnemonic(p)		((p)->mnemonic.value)
#define	PushButtonEntryMnemonic(p)		\
					PushButtonEntryPtrMnemonic(&(p))

#define PushButtonEntryPtrAccelerator(p)	((p)->accelerator.value)
#define	PushButtonEntryAccelerator(p)		\
					PushButtonEntryPtrAccelerator(&(p))

#define PushButtonEntryPtrAcceleratorText(p)	((p)->acceleratortext.value)
#define	PushButtonEntryAcceleratorText(p)	\
					PushButtonEntryPtrAcceleratorText(&(p))

#define PushButtonEntryPtrSensitivity(p)	((p)->sensitivity.value)
#define	PushButtonEntrySensitivity(p)		\
					PushButtonEntryPtrSensitivity(&(p))

#define PushButtonEntryPtrFontArgList(p)	((p)->font)
#define	PushButtonEntryFontArgList(p)		\
					PushButtonEntryPtrFontArgList(&(p))

#define PushButtonEntryPtrWidget(p)	((p)->w)
#define	PushButtonEntryWidget(p)	PushButtonEntryPtrWidget(&(p))

#define PushButtonEntryPtrMainString(p)	((p)->main)
#define	PushButtonEntryMainString(p)	PushButtonEntryPtrMainString(&(p))

#define PushButtonEntryPtrMainCString(p)	((p)->maincs)
#define	PushButtonEntryMainCString(p)	PushButtonEntryPtrMainCString(&(p))


#define PushButtonEntryPtrAlternativeString(p)	((p)->alternative)
#define	PushButtonEntryAlternativeString(p)	\
				PushButtonEntryPtrAlternativeString(&(p))

#define PushButtonEntryPtrAlternativeCString(p)	((p)->alternativecs)
#define	PushButtonEntryAlternativeCString(p)	\
				PushButtonEntryPtrAlternativeCString(&(p))


#define PushButtonEntryPtrButtonName(p)	((p)->buttonname)
#define	PushButtonEntryButtonName(p)	PushButtonEntryPtrButtonName(&(p))

#define PushButtonEntryPtrCurrentLabel(p)	((p)->currentlabel)
#define	PushButtonEntryCurrentLabel(p)	PushButtonEntryPtrCurrentLabel(&(p))


#define	StaticInitPushButtonEntryActivateCallBack(cb)	\
	{ XmNactivateCallback, (cb) }

#define	StaticInitPushButtonEntryContextHelp(sn)	\
	{ XmNhelpCallback, (sn) }

#define	StaticInitPushButtonEntryMnemonic(sn)		\
	{ XmNmnemonic, (sn) }

#define	StaticInitPushButtonEntryAccelerator(s)	\
	{ XmNaccelerator, (XtArgVal)(s) }

#define	StaticInitPushButtonEntryAcceleratorText(s)	\
	{ XmNacceleratorText, (XtArgVal)(s) }

#define	StaticInitPushButtonEntrySensitivity(sn)	\
	{ XmNsensitive, (sn) }

#define	StaticInitPushButtonEntryWidget(w)		(w)
#define	StaticInitPushButtonEntryMainString(s)		(s)
#define	StaticInitPushButtonEntryAlternativeString(s)	(s)
#define	StaticInitPushButtonEntryMainCString(s)		(s)
#define	StaticInitPushButtonEntryAlternativeCString(s)	(s)
#define	StaticInitPushButtonEntryButtonName(s)		(s)
#define	StaticInitPushButtonEntryCurrentLabel(s)	(s)

/********************************
 *
 *	PullDownMenuEntry
 *
 ********************************/

typedef	struct	_pulldownmenuentry {
	CoreArgList		core;

	Widget			w;
	char			*menuname;
	unsigned int		numbuttons;
	PushButtonEntryPtr	*pushbuttons;
} PullDownMenuEntry, *PullDownMenuEntryPtr;

#define	InitPullDownMenuEntryPtrArgList(p) 	InitCoreArgList((p)->core)
#define	InitPullDownMenuEntryArgList(p)		\
					InitPullDownMenuEntryPtrArgList(&(p))

#define	PullDownMenuEntryPtrCoreArgList(p)	((p)->core)
#define	PullDownMenuEntryCoreArgList(p)		\
					PullDownMenuEntryPtrCoreArgList(&(p))

#define	PullDownMenuEntryPtrWidget(p)		((p)->w)
#define	PullDownMenuEntryWidget(p)		PullDownMenuEntryPtrWidget(&(p))

#define	PullDownMenuEntryPtrMenuName(p)		((p)->menuname)
#define	PullDownMenuEntryMenuName(p)		PullDownMenuEntryPtrMenuName(&(p))

#define	PullDownMenuEntryPtrNumButtons(p)	((p)->numbuttons)
#define	PullDownMenuEntryNumButtons(p)	\
					PullDownMenuEntryPtrNumButtons(&(p))

#define	PullDownMenuEntryPtrPushButtons(p)	((p)->pushbuttons)
#define	PullDownMenuEntryPushButtons(p)	\
					PullDownMenuEntryPtrPushButtons(&(p))


#define	StaticInitPullDownMenuEntryWidget(w)		(w)
#define	StaticInitPullDownMenuEntryMenuName(s)		(s)
#define	StaticInitPullDownMenuEntryNumButtons(n)	(n)
#define	StaticInitPullDownMenuEntryPushButtons(pb)	(pb)

/********************************
 *
 *	PullDownEntry
 *
 ********************************/

typedef	struct	_pulldownentry {
	CoreArgList		core;
	LabelArgList		label;
	Arg			submenuwidget;
	Arg			pulldowncallback;
#ifdef HYPERHELP
	Arg			contexthelp;
#endif
	Arg		mnemonic;
	Arg			sensitivity;
	FontArgList		font;		/* not used */
	

	Widget			w,
				submenuparent;	
	char 			*entryname,
				*main;
	PullDownMenuEntryPtr	pulldownmenuentry;
	XmString		maincs;
} PullDownEntry, *PullDownEntryPtr;

#ifdef HYPERHELP

#define	InitPullDownEntryPtrArgList(p)					 \
			InitCoreArgList((p)->core);			 \
			InitLabelArgList((p)->label);			 \
			(p)->submenuwidget.name = XmNsubMenuId;	 \
			(p)->pulldowncallback.name = XmNcascadingCallback; \
			(p)->contexthelp.name = XmNhelpCallback;	 \
			(p)->mnemonic.name = XmNmnemonic;		 \
			(p)->sensitivity.name = XmNsensitive;		 \
			InitFontArgList((p)->font)
#else

#define	InitPullDownEntryPtrArgList(p)					 \
			InitCoreArgList((p)->core);			 \
			InitLabelArgList((p)->label);			 \
			(p)->submenuwidget.name = XmNsubMenuId;	 \
			(p)->pulldowncallback.name = XmNcascadingCallback; \
			(p)->mnemonic.name = XmNmnemonic;		 \
			(p)->sensitivity.name = XmNsensitive;		 \
			InitFontArgList((p)->font)
#endif

#define	InitPullDownEntryArgList(p)	InitPullDownEntryPtrArgList(&(p))

#define	PullDownEntryPtrCoreArgList(p)	((p)->core)
#define	PullDownEntryCoreArgList(p)	PullDownEntryPtrCoreArgList(&p))

#define	PullDownEntryPtrLabelArgList(p)	((p)->label)
#define	PullDownEntryLabelArgList(p)	PullDownEntryPtrLabelArgList(&p))

#define	PullDownEntryPtrSubMenuWidget(p)	((p)->submenuwidget.value)
#define	PullDownEntrySubMenuWidget(p)		\
				PullDownEntryPtrSubMenuWidget(&(p))

#define	PullDownEntryPtrPullDownCallBack(p)	((p)->pulldowncallback.value)
#define	PullDownEntryPullDownCallBack(p)	\
				PullDownEntryPtrPullDownCallBack(&p))

#define	PullDownEntryPtrContextHelp(p)	((p)->contexthelp.value)
#define	PullDownEntryContextHelp(p)	PullDownEntryPtrContextHelp(&p))

#define	PullDownEntryPtrMnemonic(p)	((p)->mnemonic.value)
#define	PullDownEntryMnemonic(p)	PullDownEntryPtrMnemonic(&p))


#define	PullDownEntryPtrSensitivity(p)	((p)->sensitivity.value)
#define	PullDownEntrySensitivity(p)	PullDownEntryPtrSensitivity(&p))

#define	PullDownEntryPtrFontArgList(p)	((p)->font)
#define	PullDownEntryFontArgList(p)	PullDownEntryPtrFontArgList(&p))

#define	PullDownEntryPtrWidget(p)	((p)->w)
#define	PullDownEntryWidget(p)		PullDownEntryPtrWidget(&p))

#define	PullDownEntryPtrSubMenuParent(p)	((p)->submenuparent)
#define	PullDownEntrySubMenuParent(p)		\
				PullDownEntryPtrSubMenuParent(&p))

#define	PullDownEntryPtrMainString(p)	((p)->main)
#define	PullDownEntryMainString(p)	PullDownEntryPtrMainString(&p))

#define	PullDownEntryPtrMainCString(p)	((p)->maincs)
#define	PullDownEntryMainCString(p)	PullDownEntryPtrMainCString(&p))

#define	PullDownEntryPtrEntryName(p)	((p)->entryname)
#define	PullDownEntryEntryName(p)	PullDownEntryPtrEntryName(&p))

#define	PullDownEntryPtrPullDownMenuEntry(p)	((p)->pulldownmenuentry)
#define	PullDownEntryPullDownMenuEntry(p)		\
				PullDownEntryPullDownMenuEntry(&(p))

#define	StaticInitPullDownEntrySubMenuWidget(w)		{ XmNsubMenuId, (w) }
#define	StaticInitPullDownEntryPullDownCallBack(w)	{ XmNcascadingCallback, (w) }
#define	StaticInitPullDownEntryContextHelp(s)		{ XmNhelpCallback, (s) }
#define	StaticInitPullDownEntryMnemonic(s)		{ XmNmnemonic, (s) }
#define	StaticInitPullDownEntrySensitivity(s)		{ XmNsensitive, (s) }
#define	StaticInitPullDownEntryWidget(w)		(w)
#define	StaticInitPullDownEntrySubMenuParent(w)		(w)
#define	StaticInitPullDownEntryMainString(s)		(s)
#define	StaticInitPullDownEntryMainCString(s)		(s)
#define	StaticInitPullDownEntryEntryName(s)		(s)
#define	StaticInitPullDownEntryPullDownMenuEntry(e)	(e)

/********************************
 *
 *	MenuEntry
 *
 ********************************/

typedef	enum 	_menuentryenum {
	EntryIsPushButton,
	EntryIsPullDown,
	EntryIsCascadeButton
} MenuEntryEnum;

typedef	union _pushbuttonorpulldown {
	PushButtonEntry	pushbutton;
	PullDownEntry	pulldown;
} *PushButtonOrPullDownPtr;

typedef	struct	_menuentry {
	MenuEntryEnum		type;
	PushButtonOrPullDownPtr	entry;
} MenuEntry, *MenuEntryPtr;

#define	MenuEntryType(p)	MenuEntryPtrType(&(p))
#define	MenuEntryPtrType(p)	((p)->type)

#define	MenuEntryPullDown(p)	MenuEntryPtrPullDown(&(p))
#define	MenuEntryPtrPullDown(p)	((p)->entry)

#define	MenuEntryPushButton(p)		MenuEntryPtrPushButton(&(p))
#define	MenuEntryPtrPushButton(p)	((p)->entry)

#define	MenuEntryPullDownEntryPtr(p)	\
					MenuEntryPtrPullDownEntryPtr(&(p))
#define	MenuEntryPtrPullDownEntryPtr(p)		(&((p)->entry->pulldown))

#define	MenuEntryPushButtonEntryPtr(p)	\
					MenuEntryPtrPushButtonEntryPtr(&(p))
#define	MenuEntryPtrPushButtonEntryPtr(p)	(&((p)->entry->pushbutton))

#define	StaticInitMenuEntryType(t)	(t)
#define	StaticInitMenuEntryEntry(e)	(e)

#define	StaticInitMenuEntryPushButton(pb)				\
	{								\
		StaticInitMenuEntryType(EntryIsPushButton),		\
		StaticInitMenuEntryEntry((PushButtonOrPullDownPtr)&(pb))	\
	}

#define	StaticInitMenuEntryPullDown(pd)					\
	{								\
		StaticInitMenuEntryType(EntryIsPullDown),		\
		StaticInitMenuEntryEntry((PushButtonOrPullDownPtr)&(pd))	\
	}



/********************************
 *
 *	AMenuBar
 *
 ********************************/

typedef	struct	_amenubar	{
	CoreArgList		core;
	ADBConstraintArgList	constraints;
	MenuBarArgList		menubar;

	Widget			w;
	char			*menuname;
	unsigned int		numentries;
	MenuEntryPtr		*entries;
} AMenuBar, *AMenuBarPtr;

#define	InitAMenuBarPtrArgList(p)				   \
			InitCoreArgList((p)->core);	   	   \
			InitADBConstraintArgList((p)->constraints);\
			InitMenuBarArgList((p)->menubar)

#define	InitAMenuBarArgList(p)	InitAMenuBarPtrArgList(&(p))

#define	AMenuBarPtrCoreArgList(p)	((p)->core)
#define	AMenuBarCoreArgList(p)		AMenuBarPtrCoreArgList(&(p))

#define	AMenuBarPtrConstraintArgList(p)	((p)->constraints)
#define	AMenuBarConstraintArgList(p)	AMenuBarPtrConstraintArgList(&(p))

#define	AMenuBarPtrMenuBarArgList(p)	((p)->menubar)
#define	AMenuBarMenuBarArgList(p)	AMenuBarPtrMenuBarArgList(&(p))

#define	AMenuBarPtrWidget(p)	((p)->w)
#define	AMenuBarWidget(p)	AMenuBarPtrWidget(&(p))

#define	AMenuBarPtrMenuName(p)	((p)->menuname)
#define	AMenuBarMenuName(p)	AMenuBarPtrMenuName(&(p))

#define	AMenuBarPtrNumEntries(p)	((p)->numentries)
#define	AMenuBarNumEntries(p)		AMenuBarPtrNuNummEntries(&(p))

#define	AMenuBarPtrEntries(p)	((p)->entries)
#define	AMenuBarEntries(p)	AMenuBarPtrEntries(&(p))

#define	StaticInitAMenuBarWidget(w)		(w)
#define	StaticInitAMenuBarMenuName(s)		(s)
#define	StaticInitAMenuBarNumEntries(n)		(n)
#define	StaticInitAMenuBarEntries(e)		(e)

#endif	MENU_H
