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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxdiff/mainmenu.c,v 1.1.3.2 1993/06/24 20:20:14 Lynda_Rice Exp $";	/* BuildSystemHeader */
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
 *	mainmenu.c - main menu
 *
 *	Author:	Laurence P. G. Cable
 *
 *	Created : 30th April 1988
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
 *	Blundered! atlered the core arglist copy code so as it wont overwrite the destory callback info
 *	copied from the static description!
 */

static char sccsid[] = "@(#)mainmenu.c	1.2	07:47:15 5/10/88";

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

extern XtCallbackRec	OpenFilesCallbackList[],
#ifdef	VIEWBTNS
			ViewNextPrevCallbackList[],
#endif
			QuitCallbackList[],
#ifdef	SEARCH
			SetRECallbackList[],
			SearchLeftRightCallbackList[],
			FindNextPrevCallbackList[],
#endif
			SlaveVerticalScrollCallbackList[],
			SlaveHorizontalScrollCallBackList[],
			DiffRenderModeCallbackList[],
			RenderLineNumberingCallbackList[],
			DoDifferencesInNewCallbackList[],
			DoDifferencesCallbackList[],
#ifdef defined( HYPERHELP )
			HelpMenuOnContextSymbol[],
			VisualDiffsSymbol[],
			HelpMenuSymbol[],
			HelpOnContextSymbol[],
			AboutSymbol[],
			SelectFilesSymbol[],
			ChangeCharsSymbol[],
			DisplayDiffsSymbol[],
			DiffExitSymbol[],
			CompareNewSymbol[],
#else
			HelpCallbackList[],
#endif
                        NullCallbackList[];

extern XtCallbackRec	FilesPullDownCallbackList[],
#ifdef	SEARCH
			SearchPullDownCallbackList[],
#endif
			OptionsPullDownCallbackList[],
#ifdef defined( HYPERHELP )
			HelpPullDownCallbackList[],
#endif
			DifferencesPullDownCallbackList[];

extern	XtCallbackRec	PushButtonDestroyCallbackList[],
			PullDownMenuDestroyCallbackList[],
			PullDownDestroyCallbackList[],
			AMenuBarDestroyCallbackList[];


/********************************
 *
 *      Main Menu Template
 *
 ********************************/


/********************************
 *
 *      'Files' PullDown 
 *	'Open Files ....' 
 *
 ********************************/

static	PushButtonEntry	openfiles = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)OpenFilesCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) SelectFilesSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('O'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Open..."),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("openfiles"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};



/********************************
 *
 *      'Files' PullDown 
 *	'View Next' 
 *
 ********************************/

#ifdef	VIEWBTNS
static	PushButtonEntry	viewnext = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, (char *)NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack(ViewNextPrevCallbackList),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, (XmFontList)NULL),
	StaticInitPushButtonEntryWidget((Widget)NULL),
	StaticInitPushButtonEntryMainString("View Next"),
	StaticInitPushButtonEntryAlternativeString((char  *)NULL),
	StaticInitPushButtonEntryButtonName("viewnext"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString((XmString)NULL),
	StaticInitPushButtonEntryAlternativeCString((XmString)NULL)
};




/********************************
 *
 *      'Files' PullDown 
 *	'View Prev' 
 *
 ********************************/

static	PushButtonEntry	viewprev = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, (char *)NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack(ViewNextPrevCallbackList),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, (XmFontList)NULL),
	StaticInitPushButtonEntryWidget((Widget)NULL),
	StaticInitPushButtonEntryMainString("View Prev"),
	StaticInitPushButtonEntryAlternativeString((char  *)NULL),
	StaticInitPushButtonEntryButtonName("viewprev"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString((XmString)NULL),
	StaticInitPushButtonEntryAlternativeCString((XmString)NULL)
};
#endif	VIEWBTNS


/********************************
 *
 *      'Files' PullDown 
 *	'Quit' 
 *
 ********************************/

static	PushButtonEntry	quit = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)QuitCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) DiffExitSymbol),
#endif	
	StaticInitPushButtonEntryMnemonic('x'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Exit"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("quit"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};






/********************************
 *
 *      'Files' PullDown 
 *
 ********************************/

static PushButtonEntryPtr filesmenubuttons[] = {
	&openfiles,
#ifdef	VIEWBTNS
	&viewnext,
	&viewprev,
#endif
	&quit
};

static	PullDownMenuEntry filespulldownmenu = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownMenuDestroyCallbackList),
	StaticInitPullDownMenuEntryWidget(NULL),
	StaticInitPullDownMenuEntryMenuName("filespulldown"),
	StaticInitPullDownMenuEntryNumButtons(sizeof filesmenubuttons / sizeof (PushButtonEntryPtr)),
	StaticInitPullDownMenuEntryPushButtons(filesmenubuttons)
};

static PullDownEntry filespulldown = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPullDownEntrySubMenuWidget(NULL),
	StaticInitPullDownEntryPullDownCallBack((XtArgVal)FilesPullDownCallbackList),
#ifdef HYPERHELP
	StaticInitPullDownEntryContextHelp((XtArgVal) SelectFilesSymbol),
#endif
	StaticInitPullDownEntryMnemonic('F'),
	StaticInitPullDownEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPullDownEntryWidget(NULL),
	StaticInitPullDownEntrySubMenuParent(NULL),
	StaticInitPullDownEntryEntryName("filespulldown"),
	StaticInitPullDownEntryMainString("File"),
	StaticInitPullDownEntryPullDownMenuEntry(&filespulldownmenu),
	StaticInitPullDownEntryMainCString(NULL)
};


/********************************
 *
 *      'Search' PullDown 
 *	'RE's....' 
 *
 ********************************/

#ifdef	SEARCH
static	PushButtonEntry	setres = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)SetRECallbackList),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("RE's..."),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("setres"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};



/********************************
 *
 *      'Search' PullDown 
 *	'Find Next' 
 *
 ********************************/

static	PushButtonEntry	findnext = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)FindNextPrevCallbackList),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Find Next"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("findnext"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};




/********************************
 *
 *      'Search' PullDown 
 *	'Find Prev' 
 *
 ********************************/

static	PushButtonEntry	findprev = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)FindNextPrevCallbackList),
	StaticInitPushButtonEntrySensitivity(False),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Find Prev"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("findprev"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Search' PullDown 
 *
 ********************************/

static PushButtonEntryPtr searchmenubuttons[] = {
	&setres,
	&findnext,
	&findprev
};

static	PullDownMenuEntry searchpulldownmenu = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, PullDownMenuDestroyCallbackList),
	StaticInitPullDownMenuEntryWidget((Widget)NULL),
	StaticInitPullDownMenuEntryMenuName("searchpulldown"),
	StaticInitPullDownMenuEntryNumButtons(sizeof searchmenubuttons / sizeof (PushButtonEntryPtr)),
	StaticInitPullDownMenuEntryPushButtons(searchmenubuttons)
};

static PullDownEntry searchpulldown = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, PullDownDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, (char *)NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPullDownEntrySubMenuWidget((Widget)NULL),
	StaticInitPullDownEntryPullDownCallBack(SearchPullDownCallbackList),
	StaticInitPullDownEntrySensitivity(False),
	StaticInitFontArgList(0, 0, (XmFontList)NULL),
	StaticInitPullDownEntryWidget((Widget)NULL),
	StaticInitPullDownEntrySubMenuParent((Widget)NULL),
	StaticInitPullDownEntryEntryName("searchpulldown"),
	StaticInitPullDownEntryMainString("Search"),
	StaticInitPullDownEntryPullDownMenuEntry(&searchpulldownmenu),
	StaticInitPullDownEntryMainCString((XmString)NULL)
};
#endif	SEARCH


/********************************
 *
 *      'Options' PullDown 
 *	'Slave Vertical Scrolling On/Off' 
 *
 ********************************/

static	PushButtonEntry	slavevertical = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)SlaveVerticalScrollCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) ChangeCharsSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('V'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Linked Vertical Scrolling On"),
	StaticInitPushButtonEntryAlternativeString("Linked Vertical Scrolling Off"),
	StaticInitPushButtonEntryButtonName("slavevertical"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};



/********************************
 *
 *      'Options' PullDown 
 *	'Slave Horizontal Scrolling On/Off' 
 *
 ********************************/

static	PushButtonEntry	slavehorizontal = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)SlaveHorizontalScrollCallBackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) ChangeCharsSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('H'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Linked Horizontal Scrolling On"),
	StaticInitPushButtonEntryAlternativeString("Linked Horizontal Scrolling Off"),
	StaticInitPushButtonEntryButtonName("slavehorizontal"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};




/********************************
 *
 *      'Options' PullDown 
 *	'Draw Diffs As Lines/Filled Polygons' 
 *
 ********************************/

static	PushButtonEntry	drawdiffs = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)DiffRenderModeCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) ChangeCharsSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('R'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Render Diffs As Lines"),
	StaticInitPushButtonEntryAlternativeString("Render Diffs As Filled Polygons"),
	StaticInitPushButtonEntryButtonName("drawdiffs"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Options' PullDown 
 *	'Display Line Numbers' 
 *
 ********************************/

static	PushButtonEntry	drawlinenumbers = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)RenderLineNumberingCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) ChangeCharsSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('N'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Display Diff Line Numbers"),
	StaticInitPushButtonEntryAlternativeString("No Diff Line Numbers"),
	StaticInitPushButtonEntryButtonName("drawlinenumbers"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Options' PullDown 
 *
 ********************************/

static PushButtonEntryPtr optionsmenubuttons[] = {
	&slavevertical,
	&slavehorizontal,
	&drawdiffs,
	&drawlinenumbers
};

static	PullDownMenuEntry optionspulldownmenu = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownMenuDestroyCallbackList),
	StaticInitPullDownMenuEntryWidget(NULL),
	StaticInitPullDownMenuEntryMenuName("optionspulldown"),
	StaticInitPullDownMenuEntryNumButtons(sizeof optionsmenubuttons / sizeof (PushButtonEntryPtr)),
	StaticInitPullDownMenuEntryPushButtons(optionsmenubuttons)
};

static PullDownEntry optionspulldown = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPullDownEntrySubMenuWidget(NULL),
	StaticInitPullDownEntryPullDownCallBack((XtArgVal)OptionsPullDownCallbackList),
#ifdef HYPERHELP
	StaticInitPullDownEntryContextHelp((XtArgVal) ChangeCharsSymbol),
#endif
	StaticInitPullDownEntryMnemonic('O'),
	StaticInitPullDownEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPullDownEntryWidget(NULL),
	StaticInitPullDownEntrySubMenuParent(NULL),
	StaticInitPullDownEntryEntryName("optionspulldown"),
	StaticInitPullDownEntryMainString("Options"),
	StaticInitPullDownEntryPullDownMenuEntry(&optionspulldownmenu),
	StaticInitPullDownEntryMainCString(NULL)
};





/********************************
 *
 *      'Differences' PullDown 
 *	'Do Differences' 
 *
 ********************************/

static	PushButtonEntry	dodifferences = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)DoDifferencesCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) DisplayDiffsSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('D'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Do Differences"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("dodifferences"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Differences' PullDown 
 *	'Do Differences In New' 
 *
 ********************************/

static	PushButtonEntry	dodifferencesinnew = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)DoDifferencesInNewCallbackList),
#ifdef HYPERHELP
	StaticInitPushButtonEntryContextHelp((XtArgVal) CompareNewSymbol),
#endif
	StaticInitPushButtonEntryMnemonic('N'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Do Differences In New"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("dodifferencesinnew"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Differences' PullDown 
 *
 ********************************/

static PushButtonEntryPtr differencesmenubuttons[] = {
	&dodifferences,
	&dodifferencesinnew
};

static	PullDownMenuEntry differencespulldownmenu = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownMenuDestroyCallbackList),
	StaticInitPullDownMenuEntryWidget(NULL),
	StaticInitPullDownMenuEntryMenuName("differencespulldown"),
	StaticInitPullDownMenuEntryNumButtons(sizeof differencesmenubuttons / sizeof (PushButtonEntryPtr)),
	StaticInitPullDownMenuEntryPushButtons(differencesmenubuttons)
};

static PullDownEntry differencespulldown = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPullDownEntrySubMenuWidget(NULL),
	StaticInitPullDownEntryPullDownCallBack((XtArgVal)DifferencesPullDownCallbackList),
#ifdef HYPERHELP
	StaticInitPullDownEntryContextHelp((XtArgVal) DisplayDiffsSymbol),
#endif
	StaticInitPullDownEntryMnemonic('D'),
	StaticInitPullDownEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPullDownEntryWidget(NULL),
	StaticInitPullDownEntrySubMenuParent(NULL),
	StaticInitPullDownEntryEntryName("differencespulldown"),
	StaticInitPullDownEntryMainString("Differences"),
	StaticInitPullDownEntryPullDownMenuEntry(&differencespulldownmenu),
	StaticInitPullDownEntryMainCString(NULL)
};










/********************************
 *
 *	'Help' 
 *
 ********************************/


#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2 || defined (HYPERHELP))

/********************************
 *
 *      'Help' PullDown 
 *	'On Context'
 *
 ********************************/

static	PushButtonEntry	oncontext = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
#ifdef HYPERHELP
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)HelpMenuOnContextSymbol),
	StaticInitPushButtonEntryContextHelp((XtArgVal) HelpOnContextSymbol),
#else
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)NullCallbackList),
#endif
	StaticInitPushButtonEntryMnemonic('C'),
	StaticInitPushButtonEntryAccelerator("Shift<Key>osfHelp"),
	StaticInitPushButtonEntryAcceleratorText("Shift+Help"),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Context-Sensitive Help"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("oncontext"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};


/********************************
 *
 *      'Help' PullDown 
 *	'On Window'
 *
 ********************************/

static	PushButtonEntry	onwindow = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
#ifdef HYPERHELP
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)VisualDiffsSymbol),
	StaticInitPushButtonEntryContextHelp((XtArgVal) VisualDiffsSymbol),
#else
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)NullCallbackList),
#endif
	StaticInitPushButtonEntryMnemonic('O'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Overview"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("onwindow"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};




/********************************
 *
 *      'Help' PullDown 
 *	'On Help'
 *
 ********************************/

static	PushButtonEntry	onhelp = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
#ifdef HYPERHELP
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)HelpMenuSymbol),
	StaticInitPushButtonEntryContextHelp((XtArgVal) HelpMenuSymbol),
#else
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)NullCallbackList),
#endif
	StaticInitPushButtonEntryMnemonic('H'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Using Help"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("onhelp"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Help' PullDown 
 *	'On Version'
 *
 ********************************/

static	PushButtonEntry	onversion = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
#ifdef HYPERHELP
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)AboutSymbol),
	StaticInitPushButtonEntryContextHelp((XtArgVal) AboutSymbol),
#else
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)NullCallbackList),
#endif
	StaticInitPushButtonEntryMnemonic('P'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Product Information"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("onversion"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};





/********************************
 *
 *      'Help' PullDown 
 *
 ********************************/

static PushButtonEntryPtr helpmenubuttons[] = {
	&oncontext,
	&onwindow,
	&onhelp,
	&onversion
};

static	PullDownMenuEntry helppulldownmenu = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownMenuDestroyCallbackList),
	StaticInitPullDownMenuEntryWidget(NULL),
	StaticInitPullDownMenuEntryMenuName("helppulldown"),
	StaticInitPullDownMenuEntryNumButtons(sizeof helpmenubuttons / sizeof (PushButtonEntryPtr)),
	StaticInitPullDownMenuEntryPushButtons(helpmenubuttons)
};

static PullDownEntry helppulldown = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PullDownDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPullDownEntrySubMenuWidget(NULL),
#ifdef HYPERHELP
	StaticInitPullDownEntryPullDownCallBack((XtArgVal)HelpPullDownCallbackList),
	StaticInitPullDownEntryContextHelp((XtArgVal) HelpMenuSymbol),
#else
	StaticInitPullDownEntryPullDownCallBack((XtArgVal)NullCallbackList),
#endif
	StaticInitPullDownEntryMnemonic('H'),
	StaticInitPullDownEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPullDownEntryWidget(NULL),
	StaticInitPullDownEntrySubMenuParent(NULL),
	StaticInitPullDownEntryEntryName("helppulldown"),
	StaticInitPullDownEntryMainString("Help"),
	StaticInitPullDownEntryPullDownMenuEntry(&helppulldownmenu),
	StaticInitPullDownEntryMainCString(NULL)
};

#else

static	PushButtonEntry	help = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)PushButtonDestroyCallbackList),
	StaticInitLabelArgList(XmSTRING, NULL, 0, 0, XmALIGNMENT_CENTER, 2, 2, 2, 2, True, False),
	StaticInitPushButtonEntryActivateCallBack((XtArgVal)HelpCallbackList),
	StaticInitPullDownEntryMnemonic('H'),
	StaticInitPushButtonEntryAccelerator(NULL),
	StaticInitPushButtonEntryAcceleratorText(NULL),
	StaticInitPushButtonEntrySensitivity(True),
	StaticInitFontArgList(0, 0, NULL),
	StaticInitPushButtonEntryWidget(NULL),
	StaticInitPushButtonEntryMainString("Help"),
	StaticInitPushButtonEntryAlternativeString(NULL),
	StaticInitPushButtonEntryButtonName("help"),
	StaticInitPushButtonEntryCurrentLabel(NoLabel),
	StaticInitPushButtonEntryMainCString(NULL),
	StaticInitPushButtonEntryAlternativeCString(NULL)
};

#endif /* HYPERHELP */





/********************************
 *
 *	'Main Menu'
 *
 ********************************/

static MenuEntry filesme =	{
	EntryIsPullDown,
	(PushButtonOrPullDownPtr)&filespulldown
};

#ifdef	SEARCH
static MenuEntry searchme = {
	EntryIsPullDown,
	(PushButtonOrPullDownPtr)&searchpulldown
};
#endif

static MenuEntry optionsme = {
	EntryIsPullDown,
	(PushButtonOrPullDownPtr)&optionspulldown
};

static MenuEntry differencesme = {
	EntryIsPullDown,
	(PushButtonOrPullDownPtr)&differencespulldown
};

static MenuEntry helpme = {
#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2 || defined (HYPERHELP))
	EntryIsPullDown,
	(PushButtonOrPullDownPtr)&helppulldown
#else
	EntryIsCascadeButton,
	(PushButtonOrPullDownPtr)&help
#endif
};

static MenuEntry *mainmenuentries[] = {
	&filesme,
#ifdef	SEARCH
	&searchme,
#endif
	&optionsme,
	&differencesme,
	&helpme
};

static AMenuBar mainmenubar = {
	StaticInitCoreArgList(0, 0, 0, 0, 0, (XtArgVal)AMenuBarDestroyCallbackList),
	StaticInitADBConstraintArgList(XmATTACH_FORM,XmATTACH_SELF,
				       XmATTACH_FORM, XmATTACH_FORM,
				       NULL, NULL,
				       NULL, NULL,
				       0,0,0,0),
	StaticInitMenuBarArgList(XmHORIZONTAL, XmMENU_BAR),
	StaticInitAMenuBarWidget(NULL),
	StaticInitAMenuBarMenuName("mainmenu"),
	StaticInitAMenuBarNumEntries(sizeof (mainmenuentries) /
				     sizeof (MenuEntryPtr)),
	StaticInitAMenuBarEntries(mainmenuentries)
};



/********************************
 *
 *	CreateNewMainMenu
 *
 ********************************/


AMenuBarPtr
CreateMainMenu(parent, core, constraints, menubar, closure)
	Widget			parent;
	CoreArgListPtr		core;
	ADBConstraintArgListPtr	constraints;
	MenuBarArgListPtr	menubar;
	caddr_t			closure;
{
	register AMenuBarPtr 	      new;
	register PushButtonEntryPtr   pb,*ppb;
	register PullDownEntryPtr     pd;
	register PullDownMenuEntryPtr pdm;
	register unsigned int         num;
	Arg	 		      helppushbutton;
	

	if ((new = (AMenuBarPtr)NewAMenuBar(&mainmenubar)) == NULL) {
		return new;
	}

	if (core != (CoreArgListPtr)NULL) {
		bcopy((char *)core, (char *)&AMenuBarPtrCoreArgList(new),
		      sizeof(CoreArgList) - sizeof (Arg));	/* dont overwrite ther destroy callback */
	}

	if (constraints != (ADBConstraintArgListPtr)NULL) {
		bcopy((char *)constraints, (char *)&AMenuBarPtrConstraintArgList(new),
		      sizeof(ADBConstraintArgList));
	}

	if (menubar != (MenuBarArgListPtr)NULL) {
		bcopy((char *)menubar, (char *)&AMenuBarPtrMenuBarArgList(new),
		      sizeof(MenuBarArgList));
	}

#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2 || defined (HYPERHELP))
	pd = (PullDownEntryPtr)MenuEntryPtrPullDown(
		AMenuBarPtrEntries(new)[(int)HelpButton]);
	LabelLabel(PullDownEntryPtrLabelArgList(pd)) = 
		(XtArgVal)PullDownEntryPtrMainCString(pd);

	pdm = PullDownEntryPtrPullDownMenuEntry(pd);

	for (num = PullDownMenuEntryPtrNumButtons(pdm),
	     ppb = PullDownMenuEntryPtrPushButtons(pdm);
	     num -- > 0;
	     ppb++) {
		LabelLabel(PushButtonEntryPtrLabelArgList(*ppb)) =
			(XtArgVal)PushButtonEntryPtrMainCString(*ppb);
		PushButtonEntryPtrCurrentLabel(*ppb) = MainLabel;
	}
#else
	pb = (PushButtonEntryPtr)MenuEntryPtrPushButton(
		AMenuBarPtrEntries(new)[(int)HelpButton]);
	LabelLabel(PushButtonEntryPtrLabelArgList(pb)) =
		(XtArgVal)PushButtonEntryPtrMainCString(pb);
	PushButtonEntryPtrCurrentLabel(pb) = MainLabel;
#endif

	pd = (PullDownEntryPtr)MenuEntryPtrPullDown(
		AMenuBarPtrEntries(new)[(int)FilesButton]);
	LabelLabel(PullDownEntryPtrLabelArgList(pd)) = 
		(XtArgVal)PullDownEntryPtrMainCString(pd);

	pdm = PullDownEntryPtrPullDownMenuEntry(pd);

	for (num = PullDownMenuEntryPtrNumButtons(pdm),
	     ppb = PullDownMenuEntryPtrPushButtons(pdm);
	     num -- > 0;
	     ppb++) {
		LabelLabel(PushButtonEntryPtrLabelArgList(*ppb)) =
			(XtArgVal)PushButtonEntryPtrMainCString(*ppb);
		PushButtonEntryPtrCurrentLabel(*ppb) = MainLabel;
	}

#ifdef	SEARCH
	pd = MenuEntryPtrPullDown(AMenuBarPtrEntries(new)[(int)SearchButton]);
	LabelLabel(PullDownEntryPtrLabelArgList(pd)) =
		PullDownEntryPtrMainCString(pd);

	pdm = PullDownEntryPtrPullDownMenuEntry(pd);

	for (num = PullDownMenuEntryPtrNumButtons(pdm),
	     ppb = PullDownMenuEntryPtrPushButtons(pdm);
	     num -- > 0;
	     ppb++) {
		LabelLabel(PushButtonEntryPtrLabelArgList(*ppb)) =
			PushButtonEntryPtrMainCString(*ppb);
		PushButtonEntryPtrCurrentLabel(*ppb) = MainLabel;
	}
#endif

	pd = (PullDownEntryPtr)MenuEntryPtrPullDown(
		AMenuBarPtrEntries(new)[(int)OptionsButton]);
	LabelLabel(PullDownEntryPtrLabelArgList(pd)) =
		(XtArgVal)PullDownEntryPtrMainCString(pd);

	pdm = PullDownEntryPtrPullDownMenuEntry(pd);

	for (num = PullDownMenuEntryPtrNumButtons(pdm),
	     ppb = PullDownMenuEntryPtrPushButtons(pdm);
	     num -- > 0;
	     ppb++) {
		LabelLabel(PushButtonEntryPtrLabelArgList(*ppb)) =
			(XtArgVal)PushButtonEntryPtrMainCString(*ppb);
		PushButtonEntryPtrCurrentLabel(*ppb) = MainLabel;
	}

	pd = (PullDownEntryPtr)MenuEntryPtrPullDown(
		AMenuBarPtrEntries(new)[(int)DifferencesButton]);
	LabelLabel(PullDownEntryPtrLabelArgList(pd)) =
		(XtArgVal)PullDownEntryPtrMainCString(pd);

	pdm = PullDownEntryPtrPullDownMenuEntry(pd);

	for (num = PullDownMenuEntryPtrNumButtons(pdm),
	     ppb = PullDownMenuEntryPtrPushButtons(pdm);
	     num -- > 0;
	     ppb++) {
		LabelLabel(PushButtonEntryPtrLabelArgList(*ppb)) =
			(XtArgVal)PushButtonEntryPtrMainCString(*ppb);
		PushButtonEntryPtrCurrentLabel(*ppb) = MainLabel;
	}

#ifdef	VIEWBTNS
	ViewNextPrevCallbackList[0].closure = closure;
#endif
	OpenFilesCallbackList[0].closure = closure;
	QuitCallbackList[0].closure = closure;
#ifdef	SEARCH
	SetRECallbackList[0].closure = closure;
	SearchLeftRightCallbackList[0].closure = closure;
	FindNextPrevCallbackList[0].closure = closure;
#endif
	SlaveVerticalScrollCallbackList[0].closure = closure;
	SlaveHorizontalScrollCallBackList[0].closure = closure;
	DiffRenderModeCallbackList[0].closure = closure;
	RenderLineNumberingCallbackList[0].closure = closure;
	DoDifferencesInNewCallbackList[0].closure = closure;
	DoDifferencesCallbackList[0].closure = closure;
	FilesPullDownCallbackList[0].closure = closure;
#ifdef	SEARCH
	SearchPullDownCallbackList[0].closure = closure;
#endif
	OptionsPullDownCallbackList[0].closure = closure;
	DifferencesPullDownCallbackList[0].closure = closure;

	CreateAMenuBar(parent, new);

#if (((XmVERSION == 1) && (XmREVISION >=2)) || XmVERSION >= 2 || defined (HYPERHELP))
        pd = (PullDownEntryPtr)MenuEntryPtrPullDown(
		AMenuBarPtrEntries(new)[(int)HelpButton]);
	helppushbutton.name = XmNmenuHelpWidget;
	helppushbutton.value = (XtArgVal)PullDownMenuEntryPtrWidget(pd);
	XtSetValues(AMenuBarPtrWidget(new), &helppushbutton, 1);

#else
	pb = (PushButtonEntryPtr)MenuEntryPtrPushButton(
		AMenuBarPtrEntries(new)[(int)HelpButton]);
	helppushbutton.name = XmNmenuHelpWidget;
	helppushbutton.value = (XtArgVal)PushButtonEntryPtrWidget(pb);
	XtSetValues(AMenuBarPtrWidget(new), &helppushbutton, 1);
#endif

	return new;
}
