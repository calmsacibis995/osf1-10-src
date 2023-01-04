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
static char *BuildSystemHeader= "$Id: mainmenucbs.c,v 1.1.4.3 1993/07/30 19:50:43 Lynda_Rice Exp $";
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
 *	mainmenucbs.c - main menu call backs 
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

static char sccsid[] = "@(#)mainmenucbs.c	1.9	10:38:02 7/19/88";

#include <sys/types.h>
#include <sys/stat.h>

#ifdef  DEBUG
#include <stdio.h>
#endif  /* DEBUG */
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <DXm/DXmHelpB.h>
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

#ifdef HYPERHELP
extern	Opaque	help_context;
extern	void	help_error();
extern	Widget	toplevel;
#endif


/********************** Private Routines ************************/


/*******************************
 *******************************
 **
 ** Main Menu 'Files' menu callbacks
 **
 *******************************
 *******************************/


/********************************
 *
 *      OpenFilesActivateCallback
 *
 ********************************/

#define	DXWMAGIC	24	/* dimensions of dxwm window decoration + a bit!*/

static void
_OpenFileSelectorsFromCallBack(dxdiffdisplay)
	register DxDiffDisplayPtr dxdiffdisplay;
{
	if (DxDiffDisplayPtrLFileSelector(dxdiffdisplay) == (FileSelectorPtr)NULL ||
	    DxDiffDisplayPtrRFileSelector(dxdiffdisplay) == (FileSelectorPtr)NULL) {
		Arg		args[2];
		Dimension	x,y;

		DxDiffDisplayPtrLFileSelector(dxdiffdisplay) = (FileSelectorPtr)
			CreateFileSelector(MainADBPtrParent(DxDiffDisplayPtrDisplayADB(dxdiffdisplay)),
					   "leftfileselector",
					   "Left File",
					   (CoreArgListPtr)NULL,
					   (DialogBoxArgListPtr)NULL,
					   dxdiffdisplay,
					   dxdiffdisplay
		       );

		args[0].name = XmNx; args[0].value = (XtArgVal)&x;
		args[1].name = XmNy; args[1].value = (XtArgVal)&y;

		XtGetValues(XtParent(FileSelectorPtrFileSelector(DxDiffDisplayPtrLFileSelector(dxdiffdisplay))), args, 2);
		
		DxDiffDisplayPtrRFileSelector(dxdiffdisplay) = (FileSelectorPtr)
			CreateFileSelector(MainADBPtrParent(DxDiffDisplayPtrDisplayADB(dxdiffdisplay)),
					   "rightfileselector",
					   "Right File",
					   (CoreArgListPtr)NULL,
					   (DialogBoxArgListPtr)NULL,
					   dxdiffdisplay,
					   dxdiffdisplay
		       );

		args[0].value = x + DXWMAGIC;
		args[1].value = y + DXWMAGIC;

		XtSetValues(XtParent(FileSelectorPtrFileSelector(DxDiffDisplayPtrRFileSelector(dxdiffdisplay))), args, 2); /* offset it ! */
	} else {	/* they are unmapped */
		XtManageChild(FileSelectorPtrFileSelector(DxDiffDisplayPtrLFileSelector(dxdiffdisplay)));
		XtManageChild(FileSelectorPtrFileSelector(DxDiffDisplayPtrRFileSelector(dxdiffdisplay)));
	}

}

static VoidProc
OpenFilesActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	_OpenFileSelectorsFromCallBack((DxDiffDisplayPtr)clientd);
}

XtCallbackRec	OpenFilesCallbackList[] = {
			{ (XtCallbackProc)OpenFilesActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};


/********************************
 *
 *      ViewNextPrevActivateCallback
 *
 ********************************/

#ifdef	VIEWBTNS
static VoidProc
ViewNextPrevActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"ViewNextPrevActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	ViewNextPrevCallbackList[] = {
			{ ViewNextPrevActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};
#endif



/********************************
 *
 *      QuitActivateCallback
 *
 ********************************/

static VoidProc
QuitActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	extern	 DxDiffDisplayPtr maindxdiffdisplay;

	DxDiffDisplayPtr display = (DxDiffDisplayPtr)clientd;

#ifdef	DEBUG
	fprintf(stderr,"QuitActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */

#ifdef HYPERHELP
	if(display == maindxdiffdisplay)
		DXmHelpSystemClose(help_context, help_error, "Help System Error");
#endif

	XtDestroyWidget(MainADBPtrParent(DxDiffDisplayPtrMainADB(display)));
	DxDiffDisplayDestroyed(display);
}

XtCallbackRec	QuitCallbackList[] = {
			{ (XtCallbackProc)QuitActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};



#ifdef	SEARCH
/*******************************
 *******************************
 **
 ** Main Menu 'Search' menu callbacks
 **
 *******************************
 *******************************/


/********************************
 *
 *      SetREActivateCallback
 *
 ********************************/

static VoidProc
SetREActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"SetREActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	SetRECallbackList[] = {
			{ SetREActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};




/********************************
 *
 *      FindNextPrevActivateCallback
 *
 ********************************/

static VoidProc
FindNextPrevActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"FindNextPrevActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	FindNextPrevCallbackList[] = {
			{ FindNextPrevActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};




/********************************
 *
 *      SearchLeftRightActivateCallback
 *
 ********************************/

static VoidProc
SearchLeftRightActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"SearchLeftRightActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	SearchLeftRightCallbackList[] = {
			{ SearchLeftRightActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};
#endif	/* SEARCH */



/*******************************
 *******************************
 **
 ** Main Menu 'Options' menu callbacks
 **
 *******************************
 *******************************/


/********************************
 *
 *      SlaveVerticalScrollActivateCallback
 *
 ********************************/

static VoidProc
SlaveVerticalScrollActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	DifferenceBoxPtr	  differencebox = DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay));
	AMenuBarPtr		  mainmenubar = DxDiffDisplayPtrMenuBar(dxdiffdisplay);
	PullDownEntryPtr	  pulldown =  
		(PullDownEntryPtr)MenuEntryPtrPullDown(
			AMenuBarPtrEntries(mainmenubar)[(int)OptionsButton]);
	PullDownMenuEntryPtr	  pulldownentry = PullDownEntryPtrPullDownMenuEntry(pulldown);
	PushButtonEntryPtr	  pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)SlaveScrollVButton];

	if (differencebox->scrollmode == ScrollBoth) {
		SetDifferenceBoxScrollingMode(differencebox, ScrollOnlyOne);
		SetPushButtonToMainLabel(pushbutton);
	} else {
		if (differencebox->scrollmode == ScrollOnlyOne) {
			WhichFile	whichfile = (differencebox->lastscrolled == LeftFile) ? RightFile: LeftFile;
			TextDisplayPtr	text;
			HVScrollBarPtr	scrollbar;
			int		*top, lasttop;

			if (whichfile == LeftFile) {
				text = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay));
				scrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay));
				lasttop = *(top = &differencebox->toplnolf);
			} else {
				text = TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay));
				scrollbar = TextDisplayADBPtrVScroll(DxDiffDisplayPtrRightTextADB(dxdiffdisplay));
				lasttop = *(top = &differencebox->toplnorf);
			}

			SetDifferenceBoxScrollingMode(differencebox, ScrollBoth);	/* will cause slave scroll */
		
			_XmTextDisableRedisplay(TextDisplayPtrWidget(text), False);
			XmTextScroll(TextDisplayPtrWidget(text), *top - lasttop);

			UpdateTextHighLights(TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)),
					     TextDisplayADBPtrTextDisplay(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)),
					     differencebox);

			_XmTextEnableRedisplay(TextDisplayPtrWidget(text));

			SetPushButtonToAlternativeLabel(pushbutton);
		}
	}
}

XtCallbackRec	SlaveVerticalScrollCallbackList[] = {
			{ (XtCallbackProc)SlaveVerticalScrollActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};


/********************************
 *
 *      SlaveHorizontalScrollActivateCallback
 *
 ********************************/

static VoidProc
SlaveHorizontalScrollActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	AMenuBarPtr		  mainmenubar = DxDiffDisplayPtrMenuBar(dxdiffdisplay);
	PullDownEntryPtr	  pulldown =  (PullDownEntryPtr)MenuEntryPtrPullDown(AMenuBarPtrEntries(mainmenubar)[(int)OptionsButton]);
	PullDownMenuEntryPtr	  pulldownentry = PullDownEntryPtrPullDownMenuEntry(pulldown);
	PushButtonEntryPtr	  pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)SlaveScrollHButton];

	if (DxDiffDisplayPtrHorizontalSlaveScroll(dxdiffdisplay)) {
		DxDiffDisplayPtrHorizontalSlaveScroll(dxdiffdisplay) = False;
		SetPushButtonToMainLabel(pushbutton);
	} else { 
		HVScrollBarPtr	scrollbar;
		XmScrollBarCallbackStruct cbs;
		Arg arg[1];
		
		DxDiffDisplayPtrHorizontalSlaveScroll(dxdiffdisplay) = True;
		SetPushButtonToAlternativeLabel(pushbutton);
		if (DxDiffDisplayPtrHorizontalScrolledLast(dxdiffdisplay) == LeftFile)
		    scrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay));
		else
		    scrollbar = TextDisplayADBPtrHScroll(DxDiffDisplayPtrRightTextADB(dxdiffdisplay));
		XtSetArg(arg[0], XmNvalue, &cbs.value);			
		XtGetValues((Widget)scrollbar, arg, 1);
		DoHorizontalScrollingFromCallBack(scrollbar, clientd, &cbs);
	}
}

XtCallbackRec	SlaveHorizontalScrollCallBackList[] = {
			{(XtCallbackProc) SlaveHorizontalScrollActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};


/********************************
 *
 *      DiffRenderModeActivateCallback
 *
 ********************************/

static VoidProc
DiffRenderModeActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	DifferenceBoxPtr	  differencebox = DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay));
	AMenuBarPtr		  mainmenubar = DxDiffDisplayPtrMenuBar(dxdiffdisplay);
	PullDownEntryPtr	  pulldown = (PullDownEntryPtr) MenuEntryPtrPullDown(AMenuBarPtrEntries(mainmenubar)[(int)OptionsButton]);
	PullDownMenuEntryPtr	  pulldownentry = PullDownEntryPtrPullDownMenuEntry(pulldown);
	PushButtonEntryPtr	  pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)RenderDiffsButton];

	if (differencebox->drawdiffsas == DrawDiffsAsLines) {
		SetDifferenceBoxPaintingStyle(differencebox, DrawDiffsAsFilledPolygons);
		SetPushButtonToMainLabel(pushbutton);
	} else {
		if (differencebox->drawdiffsas == DrawDiffsAsFilledPolygons) {
			SetDifferenceBoxPaintingStyle(differencebox, DrawDiffsAsLines);
			SetPushButtonToAlternativeLabel(pushbutton);
		}
	}
}

XtCallbackRec	DiffRenderModeCallbackList[] = {
			{(XtCallbackProc) DiffRenderModeActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};




/********************************
 *
 *      RenderLinesNumberingActivateCallback
 *
 ********************************/

static VoidProc
RenderLineNumberingActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	register DxDiffDisplayPtr dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	DifferenceBoxPtr	  differencebox = DiffRegionADBPtrDifferenceBox(DxDiffDisplayPtrDiffRegionADB(dxdiffdisplay));
	AMenuBarPtr		  mainmenubar = DxDiffDisplayPtrMenuBar(dxdiffdisplay);
	PullDownEntryPtr	  pulldown =  (PullDownEntryPtr)MenuEntryPtrPullDown(AMenuBarPtrEntries(mainmenubar)[(int)OptionsButton]);
	PullDownMenuEntryPtr	  pulldownentry = PullDownEntryPtrPullDownMenuEntry(pulldown);
	PushButtonEntryPtr	  pushbutton = PullDownMenuEntryPtrPushButtons(pulldownentry)[(int)RenderLineNumbersButton];

	if (differencebox->linenumbers) {
		SetDifferenceBoxDisplayLineNumbers(differencebox, False);
		SetPushButtonToMainLabel(pushbutton);
	} else {
		SetDifferenceBoxDisplayLineNumbers(differencebox, True);
		SetPushButtonToAlternativeLabel(pushbutton);
	}
}


XtCallbackRec	RenderLineNumberingCallbackList[] = {
			{(XtCallbackProc) RenderLineNumberingActivateCallback, 0 },
			{ (VoidProc)NULL, 0}
		};




/*******************************
 *******************************
 **
 ** Main Menu 'Differences' menu callbacks
 **
 *******************************
 *******************************/



/********************************
 *
 *      DoDifferencesActivateCallback
 *
 ********************************/

static void
_DoDifferencesFromCallBack(dxdiffdisplay, lfile, rfile)
  register DxDiffDisplayPtr dxdiffdisplay;
  char 	*lfile, *rfile;
{
    struct stat statBuf;
    int ok;

    Boolean fspresent = (DxDiffDisplayPtrLFileSelector(dxdiffdisplay) != 
			 (FileSelectorPtr)NULL);

    if (lfile == (char *)NULL) {
	if (fspresent && 
	        FileSelectorPtrFile(DxDiffDisplayPtrLFileSelector(
					      dxdiffdisplay)) != (char *)NULL) {
	    lfile = FileSelectorPtrFile(DxDiffDisplayPtrLFileSelector(
								dxdiffdisplay));
	} else {	/* none selected - use the current file */
	    lfile = FileNamePtrFile(TextDisplayADBPtrFilename(
                              DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)));
	}
    }

    if (rfile == (char *)NULL) {
	if (fspresent && FileSelectorPtrFile(DxDiffDisplayPtrRFileSelector(
                dxdiffdisplay)) != (char *)NULL) {
	    rfile = FileSelectorPtrFile(DxDiffDisplayPtrRFileSelector(
                                        dxdiffdisplay));
	} else {	/* none selected - use the current file */
	    rfile = FileNamePtrFile(TextDisplayADBPtrFilename(
                       DxDiffDisplayPtrRightTextADB(dxdiffdisplay)));
	}
    }


    if (lfile == NULL) {
	CreateMessageBox(MainADBPtrWidget(DxDiffDisplayPtrMainADB(
                                                            dxdiffdisplay)),
			 "dxdiffdisplayerror", (CoreArgListPtr)NULL,
			 (DialogBoxArgListPtr)NULL, (LabelArgListPtr)NULL,
			 "Please specify a left file", NULL );
	return;
    }

    if (rfile == NULL) {
	CreateMessageBox(MainADBPtrWidget(DxDiffDisplayPtrMainADB(
                                                            dxdiffdisplay)),
			 "dxdiffdisplayerror", (CoreArgListPtr)NULL,
			 (DialogBoxArgListPtr)NULL, (LabelArgListPtr)NULL,
			 "Please specify a right file", NULL );
	return;
    }

    /* Check for directories */

    ok = stat( lfile, &statBuf );	/* Get file status */
    if (! (statBuf.st_mode & S_IFREG)) {
	CreateMessageBox(MainADBPtrWidget(DxDiffDisplayPtrMainADB(
                                                            dxdiffdisplay)),
			 "dxdiffdisplayerror", (CoreArgListPtr)NULL,
			 (DialogBoxArgListPtr)NULL, (LabelArgListPtr)NULL,
			 "Left file is not a regular file", NULL );
	return;
    }
	
    ok = stat( rfile, &statBuf );	/* Get file status */
    if (! (statBuf.st_mode & S_IFREG)) {
	CreateMessageBox(MainADBPtrWidget(DxDiffDisplayPtrMainADB(
                                                            dxdiffdisplay)),
			 "dxdiffdisplayerror", (CoreArgListPtr)NULL,
			 (DialogBoxArgListPtr)NULL, (LabelArgListPtr)NULL,
			 "Right file is not a regular file", NULL );
	return;
    }

    if (DoDiffs(dxdiffdisplay, lfile, rfile)) {
	LoadDiffs(dxdiffdisplay, lfile, rfile, 
		  DxDiffDisplayPtrDiffList(dxdiffdisplay)->edchead,
		  DxDiffDisplayPtrDiffList(dxdiffdisplay)->edctail);
    } else {	/* oops - error */
	ReportDiffErrors(dxdiffdisplay);
    }
}

static VoidProc
DoDifferencesActivateCallback(w, clientd, calld)
	Widget	w;	
	caddr_t	clientd;
	caddr_t	calld;
{
	_DoDifferencesFromCallBack((DxDiffDisplayPtr)clientd, (char *)NULL, (char *)NULL);
}


XtCallbackRec	DoDifferencesCallbackList[] = {
			{(XtCallbackProc) DoDifferencesActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};






/********************************
 *
 *      DoDifferencesInNewActivateCallback
 *
 ********************************/

static VoidProc
DoDifferencesInNewActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
	extern	 CoreArgList	  initcore;
	extern	 DxDiffDisplayPtr maindxdiffdisplay;

	register DxDiffDisplayPtr newdxdiffdisplay, dxdiffdisplay = (DxDiffDisplayPtr)clientd;
	char 	 *lfile, *rfile;
	Boolean	 fspresent = (DxDiffDisplayPtrLFileSelector(dxdiffdisplay) != (FileSelectorPtr)NULL);


	if ((newdxdiffdisplay = (DxDiffDisplayPtr)CreateNewDxDiffDisplay(
	    (Widget)NULL, "dxdiffdisplay", &initcore)) == (DxDiffDisplayPtr)NULL) {
		return;	/* error */
	}


	if (fspresent && FileSelectorPtrFile(DxDiffDisplayPtrLFileSelector(dxdiffdisplay)) != (char *)NULL) {
			lfile = FileSelectorPtrFile(DxDiffDisplayPtrLFileSelector(dxdiffdisplay));
	} else {	/* none selected - use the current file */
		lfile = FileNamePtrFile(TextDisplayADBPtrFilename(DxDiffDisplayPtrLeftTextADB(dxdiffdisplay)));
	}

	if (fspresent && FileSelectorPtrFile(DxDiffDisplayPtrRFileSelector(dxdiffdisplay)) != (char *)NULL) {
			rfile = FileSelectorPtrFile(DxDiffDisplayPtrRFileSelector(dxdiffdisplay));
	} else {	/* none selected - use the current file */
		rfile = FileNamePtrFile(TextDisplayADBPtrFilename(DxDiffDisplayPtrRightTextADB(dxdiffdisplay)));
	}
				
	_DoDifferencesFromCallBack(newdxdiffdisplay, lfile, rfile);
}

XtCallbackRec	DoDifferencesInNewCallbackList[] = {
			{ (XtCallbackProc)DoDifferencesInNewActivateCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};

XtCallbackRec	NullCallbackList[] = {
			{ (VoidProc)NULL, 0 }
		};


/*******************************
 *******************************
 **
 ** Main Menu 'Help' menu callbacks
 **
 *******************************
 *******************************/

static Widget helpDialog = NULL;
static Widget main_window = NULL;
static Widget app_shell = NULL;

static Widget
GetAppShell (client)
Widget client;
{
    Widget lastWidget = client;
    for (app_shell = client;
             app_shell &&
               (XtClass(app_shell) != (WidgetClass)topLevelShellWidgetClass); ) {
	    lastWidget = app_shell;
            app_shell = XtParent(app_shell);
    }
    if (!app_shell)
	app_shell = lastWidget;
    return app_shell;
}


/********************************
 *
 *      HelpActivateCallback
 *
 ********************************/

VoidProc
HelpActivateCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;

#ifdef HYPERHELP
{
	DXmHelpSystemDisplay(help_context, dxdiff_help, "topic", 
			     (char *) clientd, help_error, "Help System Error");
}



/*
 * Switch into context-sensitive mode and call the selected widget's
 * context-sensitive help callback.
 */

VoidProc
HelpOnContextCallback(w, clientd, calld)
	Widget  w;
	caddr_t clientd;
	caddr_t calld;

{
	DXmHelpOnContext(toplevel, FALSE);
}


/*
 * The mapping of help callbacks (both explicit and context-sensitive) to
 * the dxdiff.decw_book symbol which is appropriate for each.  This would
 * typically be in the uil files but since dxdiff has none ....
 */

XtCallbackRec	HelpMenuOnContextSymbol[] = {
			{(XtCallbackProc) HelpOnContextCallback, "help_menu_oncontext" },
			{ (VoidProc)NULL, 0 }
		};

/* Define a macro to make these mappings easier and more readable */

#define MapHelpSymbol(rec, sym)						 \
	XtCallbackRec rec[] = {						 \
			  {(XtCallbackProc) HelpActivateCallback, sym }, \
			  {(VoidProc) NULL, 0}				 \
		      };

MapHelpSymbol(VisualDiffsSymbol, "visual_diffs");
MapHelpSymbol(HelpMenuSymbol, "help_menu");
MapHelpSymbol(HelpOnContextSymbol, "help_menu_oncontext");
MapHelpSymbol(AboutSymbol, "about");
MapHelpSymbol(SelectFilesSymbol, "select_files");
MapHelpSymbol(ChangeCharsSymbol, "change_chars");
MapHelpSymbol(DisplayDiffsSymbol, "display_diffs");
MapHelpSymbol(DiffExitSymbol, "diff_exit");
MapHelpSymbol(CompareNewSymbol, "compare_new");
MapHelpSymbol(MovingBetweenSymbol, "moving_between");

#else

{
	static XmString help_librarySpec = (XmString) NULL;
	static XmString help_applicationName;
	static XmString topic;
	Arg help_args[5];
	int ac = 0;

#ifdef	DEBUG
	fprintf(stderr,"HelpActivateCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
	if (!help_librarySpec) {
	    help_librarySpec = XmStringCreateSimple("/usr/lib/X11/help/dxdiff");
	    help_applicationName = XmStringCreateSimple("dxdiff");
	}
	if (clientd == NULL) {
	    topic = XmStringCreateSimple("About");
	} else {
	    topic = XmStringCreateSimple(clientd);
	}

	XtSetArg(help_args[ac], DXmNfirstTopic, topic);ac++;
	XtSetArg(help_args[ac], DXmNlibrarySpec, help_librarySpec);ac++;
	XtSetArg(help_args[ac], DXmNlibraryType, DXmTextLibrary);ac++;
	XtSetArg(help_args[ac], DXmNapplicationName, help_applicationName);ac++;
	if (!helpDialog) {
	    (void) GetAppShell(w);
	    DXmInitialize();
	    helpDialog = DXmCreateHelpDialog(app_shell, "helpDialog",
						help_args, ac);
	}

	if (helpDialog) {
	    XtSetValues(helpDialog, help_args, ac);
	    XtManageChild(helpDialog);
	}
	XmStringFree(topic);
}

XtCallbackRec	HelpCallbackList[] = {
			{(XtCallbackProc) HelpActivateCallback, "About" },
			{ (VoidProc)NULL, 0 }
		};

#endif  /* HYPERHELP */



/*******************************
 *******************************
 **
 ** Main Menu Pull Down menu callbacks
 **
 *******************************
 *******************************/


/********************************
 *
 *      FilesPullDownCallback
 *
 ********************************/

static VoidProc
FilesPullDownCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"FilesPullDownCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	FilesPullDownCallbackList[] = {
			{(XtCallbackProc) FilesPullDownCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};


/********************************
 *
 *      SearchPullDownCallback
 *
 ********************************/

#ifdef	SEARCH
static VoidProc
SearchPullDownCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"SearchPullDownCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	SearchPullDownCallbackList[] = {
			{ SearchPullDownCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};
#endif


/********************************
 *
 *      OptionsPullDownCallback
 *
 ********************************/

static VoidProc
OptionsPullDownCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"OptionsPullDownCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	OptionsPullDownCallbackList[] = {
			{(XtCallbackProc) OptionsPullDownCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};


/********************************
 *
 *      DifferencesPullDownCallback
 *
 ********************************/

static VoidProc
DifferencesPullDownCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"DifferencesPullDownCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	DifferencesPullDownCallbackList[] = {
			{(XtCallbackProc) DifferencesPullDownCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};

#ifdef HYPERHELP

/********************************
 *
 *      HelpPullDownCallback
 *
 ********************************/

static VoidProc
HelpPullDownCallback(w, clientd, calld)
	Widget	w;
	caddr_t	clientd;
	caddr_t	calld;
{
#ifdef	DEBUG
	fprintf(stderr,"HelpPullDownCallback(0x%x, 0x%x, 0x%x)\n",
	        w, clientd, calld);
#endif	/* DEBUG */
}

XtCallbackRec	HelpPullDownCallbackList[] = {
			{(XtCallbackProc) HelpPullDownCallback, 0 },
			{ (VoidProc)NULL, 0 }
		};

#endif
