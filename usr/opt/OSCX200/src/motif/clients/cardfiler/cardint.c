#if defined(OSF1) || defined(__osf__)
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/clients/cardfiler/cardint.c,v 1.1.1.3 92/06/29 10:21:36 Russ_Kuhn Exp $";
#endif
#endif
/*
**++

  Copyright (c) Digital Equipment Corporation, 
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	cardint.c
**
**  FACILITY:
**      OOTB Cardfiler
**
**  ABSTRACT:
**	Currently contains most of the DECwindows User Interface routines for
**	the Cardfiler.
**
**  AUTHORS:
**      Paul Reilly
**
**  RELEASER:
**
**  CREATION DATE:     17-DEC-1987
**
**  MODIFICATION HISTORY:
**
**    Version 3.0
**
**	7-dec-93 pjw	Changed to use img$shlib12 for Motif 1.2
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**--
**/


#include "cardglobaldefs.h"
#include "cardexterndefs.h"

#include <stdio.h>

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <Xm/CutPasteP.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/ArrowB.h>
#include <Xm/ScrolledW.h>
#include <DXm/DECspecific.h>
#include <DXm/DXmSvn.h>
#include <DXm/DXmCSText.h>

#ifdef VMS
#include "dwi18n_lib.h"
#else
#include <dwi18n_lib.h>
#endif

#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

Boolean same_card = FALSE;		/* Display same card */
Boolean GraphicSelected = FALSE;	/* The graphic was selected     */
/*static Time lasttime;*/
XmScrolledWindowWidget card_scroll_window;
Widget cardimagearea;
XmPushButtonWidget open_no_button, exit_no_button;
XmPushButtonWidget read_bitmap_button;
/* Callback structures and function definitions. */

static void button_timeouthandler();
static void SvnSelectCallBack();
static void SvnSelectAndDisplayCallBack();
static void SvnGetEntryCallBack();
static void SvnAttachToSourceCallBack();
void edit_pulling();
void print_ok_cb();
void print_cancel_cb();
void displaycardfromindex();
void clearproc();
void selectgraphicproc();
void deselectgraphicproc();
void copy_to_clipboard();
void no_op();
void mb3help();
void ButtonReleased();
void ButtonPressed();
void LeaveWindow();
void textwidgetcalls();
void CardPushFillHighlight();
void CardPushFillUnHighlight();
void PasteText();
void PasteGraphic();
void CutGraphic();
void CopyGraphic();
static int Clipboard_Copy_Callback();
card_pointer FindCardByIndex();

void change_undo_sensitivity();
Cursor WorkCursorCreate();
void WorkCursorDisplay();
void WorkCursorRemove();
void CursorDisplay();
void CursorRemove();

/* Routines in cardmain.c */
extern void renameindex();
extern void renameaction();
extern void file_select_action();
extern void card_file_select_action();
extern void noaction();
extern void retrieveaction();
extern void readgraphicaction();
extern void saveaction();
extern void clearaction();
extern void open_caution_action();
extern void deleteaction();
extern void exitsave();
extern void exitaction();
extern void enterfnameaction();
extern void mergeaction();
extern void clearstoreaction();
extern void exitsaveaction();
extern void gotoaction();
extern void findaction();
extern void readgraphicproc();
extern void cardhelp();
extern void indexhelp();
extern void on_context_activate_proc();
extern void help_done_proc();

/* Routines in cardio.c	*/
extern int Load_DDIF_Frame();

#define HIGHLIGHT	1
#define DrawMode(w)	((w)->label.drawing_mode)
#define HiLite(w)	((w)->pushbutton.fillHighlight)

Widget fetch_widget();

MrmCount regnum;
MrmRegisterArg regvec[] = {
    {"addproc", (caddr_t) addproc},
    {"card_file_select_action", (caddr_t) card_file_select_action},
    {"cardhelp", (caddr_t) cardhelp},
    {"clearaction", (caddr_t) clearaction},
    {"clearfunc", (caddr_t) clearfunc},
    {"closeproc", (caddr_t) closeproc},
    {"copyproc", (caddr_t) copyproc},
    {"create_button", (caddr_t) create_button},
    {"create_connection_menu", (caddr_t) CreateConnectionMenu},
    {"create_dialog", (caddr_t) create_dialog},
    {"create_text", (caddr_t) create_text},
    {"dialog_mapped", (caddr_t) dialog_mapped},
    {"cutproc", (caddr_t) cutproc},
    {"deleteaction", (caddr_t) deleteaction},
    {"delproc", (caddr_t) delproc},
    {"deselectgraphicproc", (caddr_t) deselectgraphicproc},
    {"duplicate", (caddr_t) duplicate},
    {"edit_pulling", (caddr_t) edit_pulling},
    {"exitproc", (caddr_t) exitproc},
    {"exitsave", (caddr_t) exitsave},
    {"enterfnameaction", (caddr_t) enterfnameaction},
    {"exitsaveaction", (caddr_t) exitsaveaction},
    {"file_select_action", (caddr_t) file_select_action},
    {"findaction", (caddr_t) findaction},
    {"findnextproc", (caddr_t) findnextproc},
    {"findproc", (caddr_t) findproc},
    {"gotoaction", (caddr_t) gotoaction},
    {"gotoproc", (caddr_t) gotoproc},
    {"help_done_proc", (caddr_t) help_done_proc},
    {"indexhelp", (caddr_t) indexhelp},
    {"mergeproc", (caddr_t) mergeproc},
    {"nextproc", (caddr_t) nextproc},
    {"noaction", (caddr_t) noaction},
    {"NoFunction", (caddr_t) NoFunction},
    {"on_context_activate_proc", (caddr_t) on_context_activate_proc},
    {"opencard", (caddr_t) opencard},
    {"open_caution_action", (caddr_t) open_caution_action},
    {"pasteproc", (caddr_t) pasteproc},
    {"previousproc", (caddr_t) previousproc},
    {"printproc", (caddr_t) printproc},
    {"readgraphicproc", (caddr_t) readgraphicproc},
    {"redrawbitmap", (caddr_t) redrawbitmap},
    {"renameaction", (caddr_t) renameaction},
    {"renameindex", (caddr_t) renameindex},
    {"restoreproc", (caddr_t) restoreproc},
    {"restore_settings_proc", (caddr_t) restore_settings_proc},
    {"retrievecards", (caddr_t) retrievecards},
    {"save_settings_proc", (caddr_t) save_settings_proc},
    {"selectgraphicproc", (caddr_t) selectgraphicproc},
    {"svn_select_callback", (caddr_t) SvnSelectCallBack},
    {"svn_selectanddisplay_callback", (caddr_t) SvnSelectAndDisplayCallBack},
    {"svn_getentry_callback", (caddr_t) SvnGetEntryCallBack},
    {"svn_attach_to_source_callback", (caddr_t) SvnAttachToSourceCallBack},
    {"storecards", (caddr_t) storecards},
    {"storeascards", (caddr_t) storeascards},
    {"textwidgetcalls", (caddr_t) textwidgetcalls},
    {"undeleteproc", (caddr_t) undeleteproc},
    {"undoproc", (caddr_t) undoproc},};

/* Translation and action tables used. */

static char index_translation_table[] =
	"Help<Btn1Down>:	Help(1)";
static caddr_t index_translation = (caddr_t) index_translation_table;
static XtTranslations index_translation_parsed;

static char card_translation_table[] =
	"~Help<Btn1Down>:	selectbitmap()\n\
	 Help<Btn1Down>:	Help(0)";
	/*<Expose>:		redrawbitmap()\n\*/

static caddr_t card_translation = (caddr_t) card_translation_table;
static XtTranslations card_translation_parsed;

static char card_parent_translation_table[] =
	"<PropertyNotify>:	icon_state_changed()\n\
	 Help<Btn1Down>:	Help(0)";
static caddr_t card_parent_translation =
      (caddr_t) card_parent_translation_table;
static XtTranslations card_parent_translation_parsed;

static char valuewindow_dummy_table[] =
	"<Key>a:		Dummy()\n\
	 Help<Btn1Down>:	Help(0)";
static caddr_t valuewindow_dummy = (caddr_t) valuewindow_dummy_table;
static XtTranslations valuewindow_dummy_parsed;

static char prev_next_table[] =
	"~Shift ~Ctrl ~Meta ~Help<Btn1Down>:	ButtonPressed()\n\
	 Any<LeaveWindow>:			LeaveWindow()\n\
	 ~Shift ~Ctrl ~Meta ~Help<Btn1Up>:	ButtonReleased()";
static caddr_t prev_next = (caddr_t) prev_next_table;
static XtTranslations prev_next_parsed;

static XmScrolledWindowWidget s_valuewindow;

void resize_card();
void redrawbitmap();
void selectbitmap();
void icon_state_changed();
XtActionsRec card_action_table[] = {
	{"resize_card", (XtActionProc) resize_card},
	{"redrawbitmap", (XtActionProc) redrawbitmap},
	{"selectbitmap", (XtActionProc) selectbitmap},
	{"icon_state_changed", (XtActionProc) icon_state_changed},
	{"Help", (XtActionProc) mb3help},
	{"ButtonPressed", (XtActionProc) ButtonPressed},
	{"ButtonReleased", (XtActionProc) ButtonReleased},
	{"LeaveWindow", (XtActionProc) LeaveWindow},
	{"Dummy", (XtActionProc) no_op}, NULL};

XtCallbackRec print_ok_cbs[] = {
	{(VoidProc) print_ok_cb, NULL},
	{NULL, NULL} };

XtCallbackRec print_cancel_cbs[] = {
	{(VoidProc) print_cancel_cb, NULL},
	{NULL, NULL} };


/*
**++
**  ROUTINE NAME: displaycardfromindex
**
**  FUNCTIONAL DESCRIPTION:
**	This routine takes an index value of the new card and
**	display the contents of the card in the Card Window.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void displaycardfromindex(cur_index)
    int cur_index;

{

    /* Just display it if cur_index is valid so
     * that the card window can take focus */
    if (cur_index != 0) {
	currentselection = FindCardByIndex(cur_index);
	if (currentselection != NULL) {
	    getcard(currentselection);
	    card_displayed = currentselection;
	    displaycard();
	}
    }
}

/*
**++
**  ROUTINE NAME: SvnSelectCallBack
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void SvnSelectCallBack(dummy, dummy2, CB)
    int dummy, dummy2;
    DXmSvnCallbackStruct *CB;
{

    card_pointer selected_card;
#ifdef SVN_DEBUG
    printf ("Entering SvnSelectCallBack\n"); 
#endif
    selected_card = (card_pointer) CB->entry_tag;
    currentselection = selected_card;
#ifdef SVN_DEBUG
    printf ("Leaving SvnSelectCallBack\n"); 
#endif
}

/*
**++
**  ROUTINE NAME: SvnSelectAndDisplayCallBack
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void SvnSelectAndDisplayCallBack(dummy, dummy2, CB)
    int dummy, dummy2;
    DXmSvnCallbackStruct *CB;
{
    card_pointer selected_card;
    int index;

#ifdef SVN_DEBUG
    printf ("Entering SvnSelectAndDisplayCallBack\n"); 
#endif
/*  Commented out to workaround Motif 1.2 problem
    WorkCursorDisplay();
*/
    lasttime = XtLastTimestampProcessed(dpy);
    selected_card = (card_pointer) CB->entry_tag;
    index = selected_card->index_number;
    displaycardfromindex(index);
/*  Commented out to workaround Motif 1.2 problem
    WorkCursorRemove();
*/
#ifdef SVN_DEBUG
    printf ("Leaving SvnSelectAndDisplayCallBack\n"); 
#endif
}

/*
**++
**  ROUTINE NAME: SvnAttachToSourceCallBack
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void SvnAttachToSourceCallBack(svnw, unused_tag, CB)
    Widget svnw;
    int unused_tag;
    DXmSvnCallbackStruct *CB;
{
#ifdef SVN_DEBUG
    printf ("SvnAttachToSourceCallBack called\n"); 
#endif
    svnfontlist = 0;
}

/*
**++
**  ROUTINE NAME: SvnGetEntryCallBack
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void SvnGetEntryCallBack(svnw, unused_tag, CB)
    Widget svnw;
    int unused_tag;
    DXmSvnCallbackStruct *CB;

{
    card_pointer card_ptr;
    XmString cs;
    int entry_number;
    Arg arglist[4];
    int ac;
    long cs_status, byte_count;

#ifdef SVN_DEBUG
    printf ("Entering SvnGetEntryCallBack\n"); 
#endif
    /* Get tag and entry number from Call back structure. */
    card_ptr = (card_pointer) CB->entry_tag;
    entry_number = CB->entry_number;

    DXmSvnDisableDisplay(svnlist);

    /* Do it only for the initial entry creation. */
    if (!card_ptr->svngetentrycalled) {
	card_ptr->svngetentrycalled = TRUE;

	/* Convert the index string into a XmString to pass to svn. */
	cs = DXmCvtFCtoCS(card_ptr->index, &byte_count, &cs_status);

	/* Make a call to SetEntry (). */
	DXmSvnSetEntry(svnlist, entry_number, 0, 11, 2, TRUE, card_ptr, TRUE);

	/* Set entry to have 2 components, first is the Memex Highlight Pixmap
	 * and the second is the Card index. */
	DXmSvnSetComponentText(svnlist, entry_number, 2, 20, 1, cs, NULL);
	XmStringFree(cs);
    }

    DXmSvnSetComponentPixmap(svnlist, entry_number, 1, 1, 3,
      memex_highlight_icon, 13, 11);
    if (!card_ptr->highlighted)
	DXmSvnSetComponentHidden(svnlist, entry_number, 1,
	  DXmSvnKdisplayAllModes);

    DXmSvnEnableDisplay(svnlist);
#ifdef SVN_DEBUG
    printf ("Leaving SvnGetEntryCallBack\n"); 
#endif
}

/*
**++
**  ROUTINE NAME: FindCardByIndex
**
**  FUNCTIONAL DESCRIPTION:
**	Find a card based on its index.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
card_pointer FindCardByIndex(index_number)
    int index_number;
{
    card_pointer curcard, newcard;
    int found;

    found = FALSE;
    curcard = first;

    while (!found) {
	if (curcard == last) {
	    if (curcard->index_number == index_number)
		newcard = curcard;
	    else
		newcard = NULL;
	    found = TRUE;
	} else if (curcard->index_number == index_number) {
	    newcard = curcard;
	    found = TRUE;
	}
	curcard = curcard->next;
    }
    return (newcard);
}

/*
**++
**  ROUTINE NAME: makewindows
**
**  FUNCTIONAL DESCRIPTION:
**	Make the windows used in the cardfiler. This includes
**	everything  except for the windows used in the card itself.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
makewindows()
{
    Arg args[10];
    int i, nac;
    Position x, y;
    Dimension in_wid, in_hyt;
    MrmType *dummyclass;
    Atom delete_window_atom;

    /* Add the action table */
    XtAppAddActions(app_context, card_action_table,
      XtNumber(card_action_table));

    /* Set up the parent window which will contain the card and other parts */
    regnum = sizeof(regvec) / sizeof(MrmRegisterArg);
    MrmRegisterNames(regvec, regnum);

    indexmainwindow =
      (XmMainWindowWidget) fetch_widget("indexmainwindow", indexparent);
    nac = 0;
    if (XtWidth(indexmainwindow) < 150) {
	XtSetArg(args[nac], XmNwidth, 150);
	nac++;
    }

    if (XtHeight(indexmainwindow) < 175) {
	XtSetArg(args[nac], XmNheight, 175);
	nac++;
    }

    if (nac > 0)
	XtSetValues((Widget)indexmainwindow, args, nac);

    XtManageChild((Widget)indexmainwindow);

    cardparent = XtAppCreateShell("CardfilerCard", CLASS_NAME,
      topLevelShellWidgetClass, dpy, NULL, 0);

    cardmainwindow =
      (XmMainWindowWidget) fetch_widget("cardmainwindow", cardparent);

    /*  Set up a callback for the Window manager's CLOSE option. */
    delete_window_atom =
      XmInternAtom(XtDisplay(cardparent), "WM_DELETE_WINDOW", FALSE);
    XmAddWMProtocolCallback(cardparent, delete_window_atom,
      (XtCallbackProc) closeproc, NULL);
    XmActivateWMProtocol(cardparent, delete_window_atom);
    XtSetArg(args[0], XmNdeleteResponse, XmDO_NOTHING);
    XtSetValues(cardparent, args, 1);
    nac = 0;
    if (XtWidth(cardmainwindow) < 170) {
	XtSetArg(args[nac], XmNwidth, 170); nac++;
    }

    if (XtHeight(cardmainwindow) < 170) {
	XtSetArg(args[nac], XmNheight, 170); nac++;
    }

    if (nac > 0) {
	XtSetValues((Widget)cardmainwindow, args, nac);
    }


    XtSetMappedWhenManaged(cardparent, FALSE);
    XtManageChild((Widget)cardmainwindow);

    /* Have to set up resize translation table here */
    card_parent_translation_parsed =
      XtParseTranslationTable(card_parent_translation_table);
    nac = 0;
    XtSetArg(args[nac], XtNallowShellResize, TRUE); nac++;
    XtSetValues(cardparent, args, nac);

    XtAugmentTranslations(cardparent, card_parent_translation_parsed);

#ifdef NO_ISL
    /* gray out include image button, if NO_ISL turned on */
    XtSetSensitive((Widget)read_bitmap_button, FALSE);
#endif

    /* Managed in the UIL file. Install Accel. cause <Return> not recornize by
     * "valuewindow".  Changed to XtInstallAccelerators for IFT2.*/
    /* XtManageChild(valuewindow);*/
    XtInstallAccelerators((Widget)valuewindow, cardparent);

}

/*
**++
**  ROUTINE NAME: create_button
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void create_button(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int type = (int) *tag;

    switch (type) {
	case k_printfile:
	    print_button = (XmPushButtonWidget) widget;
	    break;
	case k_printfileas:
	    print_as_button = (XmPushButtonWidget) widget;
	    break;
	case k_indexgoto:
	    goto_button = (XmPushButtonWidget) widget;
	    break;
	case k_indexfind:
	    find_button = (XmPushButtonWidget) widget;
	    break;
	case k_indexfindnext:
	    find_next_button = (XmPushButtonWidget) widget;
	    break;
	case k_cardgoto:
	    card_goto_button = (XmPushButtonWidget) widget;
	    break;
	case k_cardfind:
	    card_find_button = (XmPushButtonWidget) widget;
	    break;
	case k_cardfindnext:
	    card_find_next_button = (XmPushButtonWidget) widget;
	    break;
	case k_undelete:
	    undelete_button = (XmPushButtonWidget) widget;
	    break;
	case k_restore:
	    restore_button = (XmPushButtonWidget) widget;
	    break;
	case k_undo:
	    undo_button = (XmPushButtonWidget) widget;
	    break;
	case k_cut:
	    cut_button = (XmPushButtonWidget) widget;
	    break;
	case k_copy:
	    copy_button = (XmPushButtonWidget) widget;
	    break;
	case k_paste:
	    paste_button = (XmPushButtonWidget) widget;
	    break;
	case k_select_graphic:
	    select_graphic_button = (XmPushButtonWidget) widget;
	    break;
	case k_deselect_graphic:
	    deselect_graphic_button = (XmPushButtonWidget) widget;
	    break;
	case k_readbitmapfile:
	    read_bitmap_button = (XmPushButtonWidget) widget;
	    break;
    }
}

/*
**++
**  ROUTINE NAME: create_text
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void create_text(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int type = (int) *tag;
    Boolean status;

    switch (type) {
	case k_goto_text:
	    {
		goto_text = (DXmCSTextWidget) widget;
		break;
	    }
	case k_card_goto_text:
	    {
		card_goto_text = (DXmCSTextWidget) widget;
		break;
	    }
	case k_find_text:
	    {
		find_text = (DXmCSTextWidget) widget;
		break;
	    }
	case k_card_find_text:
	    {
		card_find_text = (DXmCSTextWidget) widget;
		break;
	    }
	case k_index_dialog_text:
	    {
		index_dialog_text = (DXmCSTextWidget) widget;
		break;
	    }
	case k_card_index_dialog_text:
	    {
		card_index_dialog_text = (DXmCSTextWidget) widget;
		break;
	    }
    }
}

/*
**++
**  ROUTINE NAME: dialog_mapped
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void dialog_mapped(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int type = (int) *tag;
    Boolean status;

    switch (type) {
	case k_goto_text:
	    {
		status = XmProcessTraversal(goto_text, XmTRAVERSE_CURRENT);
		break;
	    }
	case k_card_goto_text:
	    {
		status =
		  XmProcessTraversal(card_goto_text, XmTRAVERSE_CURRENT);
		break;
	    }
	case k_find_text:
	    {
		status = XmProcessTraversal(find_text, XmTRAVERSE_CURRENT);
		break;
	    }
	case k_card_find_text:
	    {
		status =
		  XmProcessTraversal(card_find_text, XmTRAVERSE_CURRENT);
		break;
	    }
	case k_index_dialog_text: 	/* create new card */
	    {
		status =
		  XmProcessTraversal(index_dialog_text, XmTRAVERSE_CURRENT);
		break;
	    }
	case k_card_index_dialog_text: 	/* create new card */
	    {
		status = XmProcessTraversal(card_index_dialog_text,
		  XmTRAVERSE_CURRENT);
		break;
	    }
    }

}

/*
**++
**  ROUTINE NAME: create_dialog
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void create_dialog(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int type = (int) *tag;
    Arg new_args[10];
    int nac;
    MrmType *dummyclass;

    switch (type) {
	case k_indexworkarea:
	    {
		indexworkarea = widget;
		break;
	    }
	case k_svnlist:
	    {
		svnlist = widget;
		break;
	    }
	case k_cardworkarea:
	    {
		cardworkarea = widget;

		/* Have to set up resize translation table here */
#ifdef NEW_LAYOUT
		card_translation_parsed =
		  XtParseTranslationTable(card_translation_table);
		valuewindow_dummy_parsed =
		  XtParseTranslationTable(valuewindow_dummy_table);
		nac = 0;
		XtSetArg(new_args[nac], XmNtranslations,
		  card_translation_parsed); nac++;
		XtSetValues((Widget)cardworkarea, new_args, nac);
#endif
		break;
	    }
	case k_card_scroll_window:
	    {
		card_scroll_window = (XmScrolledWindowWidget) widget;
		XmAddTabGroup(card_scroll_window);
		break;
	    }
	case k_valuewindow:
	    {
		valuewindow = (DXmCSTextWidget) widget;
		XmAddTabGroup(valuewindow);
		s_valuewindow = (XmScrolledWindowWidget) XtParent(widget);
		XmAddTabGroup(s_valuewindow);
		break;
	    }
	case k_cardimagearea:
	    {
		cardimagearea = widget;

		/* Have to set up resize translation table here */
		card_translation_parsed =
		  XtParseTranslationTable(card_translation_table);
		valuewindow_dummy_parsed =
		  XtParseTranslationTable(valuewindow_dummy_table);
		nac = 0;
		XtSetArg(new_args[nac], XmNtranslations,
		  card_translation_parsed); nac++;
		XtSetValues((Widget)cardimagearea, new_args, nac);
		break;
	    }
	case k_buttonbox:
	    {
		buttonbox = widget;
		XmAddTabGroup(buttonbox);
		break;
	    }
	case k_bb_close:
	    {
		bb_close = (XmPushButtonWidget) widget;
/*		prev_next_parsed = XtParseTranslationTable(prev_next_table);
**		nac = 0;
**		XtSetArg(new_args[nac], XmNtranslations, prev_next_parsed);
**		nac++;
*/
		break;
	    }
	case k_bb_button1:
	    {
		Dimension height;

		bb_button1 = (XmArrowButtonWidget) widget;
		nac = 0;
		XtSetArg(new_args[nac], XmNheight, &height);
		nac++;
		XtGetValues(widget, new_args, nac);

		nac = 0;
		XtSetArg(new_args[nac], XmNwidth, height);
		nac++;
		XtSetValues(widget, new_args, nac);

		prev_next_parsed = XtParseTranslationTable(prev_next_table);
		nac = 0;
		XtSetArg(new_args[nac], XmNtranslations, prev_next_parsed);
		nac++;

		break;
	    }
	case k_bb_button2:
	    {
		Dimension height;

		bb_button2 = (XmArrowButtonWidget) widget;
		nac = 0;
		XtSetArg(new_args[nac], XmNheight, &height);
		nac++;
		XtGetValues(widget, new_args, nac);

		nac = 0;
		XtSetArg(new_args[nac], XmNwidth, height);
		nac++;
		XtSetValues(widget, new_args, nac);

		prev_next_parsed = XtParseTranslationTable(prev_next_table);
		nac = 0;
		XtSetArg(new_args[nac], XmNtranslations, prev_next_parsed);
		nac++;

		break;
	    }
	case k_open_caution:
	    {
		if (open_no_button == NULL) 
			if (MrmFetchWidget(card_hierarchy, "open_no_button",
			    widget, &open_no_button, &dummyclass) != MrmSUCCESS)
				card_serror(NoCardMessage, FATAL);
		XtManageChild((Widget)open_no_button);
		break;
	    }
	case k_exit_dialog:
	    {
		if (exit_no_button == NULL) 
			if (MrmFetchWidget(card_hierarchy, "exit_no_button",
			    widget, &exit_no_button, &dummyclass) != MrmSUCCESS)
				card_serror(NoCardMessage, FATAL);
		XtManageChild((Widget)exit_no_button);
		break;
	    }

    }
}

/*
**++
**  ROUTINE NAME: displaycard
**
**  FUNCTIONAL DESCRIPTION:
**	Create both the index window and the filed window. Use only the
**	textarglist variable to save space (this stuff doesn't need
**	to hang around after text widget creation).
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
displaycard()
{
    Arg args[10];
    int intx, inty, status, nac = 0;
    long byte_count, cs_status;
    char *get_type, temp[MAX_FILE_LEN];
    Position x, y;
    Dimension in_wid, in_hyt, card_hyt;
    XtWidgetGeometry g, *gp, *gpr;
    XWMHints hints;
    XrmValue get_value;

    lasttime = XtLastTimestampProcessed(dpy);

	if (first_time_in) {
	    nac = 0;
	    XtTranslateCoords(indexparent, 0, 0, &x, &y);
	    in_wid = XtWidth(indexparent) + 2 * XtBorderWidth(indexparent);
	    in_hyt = XtHeight(indexparent) + 2 * XtBorderWidth(indexparent);
	    x = x + in_wid + 8;
	    y = y + 8;
	    status = XrmGetResource(merged_database, xrm_card_x, xrc_card_x,
	      &get_type, &get_value);

	    if (status != NULL) {
		intx = atoi(get_value. addr);
		x = (Position) intx;
	    }
	    status = XrmGetResource(merged_database, xrm_card_y, xrc_card_y,
	      &get_type, &get_value);

	    if (status != NULL) {
		inty = atoi(get_value. addr);
		y = (Position) inty;
	    }

	    XtSetArg(args[nac], XmNx, x); nac++;
	    XtSetArg(args[nac], XmNy, y); nac++;
	    if (nac > 0)
		XtSetValues(cardparent, args, nac);

	    if (!XtIsRealized(cardparent))
		XtRealizeWidget(cardparent);
	    resize_card();
            first_time_in = FALSE;
	}				/* first time in */

    if (same_card)
	same_card = FALSE;
    else {
	/* Set the card to the new text. */
	{
	    XmString cs_null;

	    DXmCSTextDisableRedisplay(valuewindow, FALSE);
	    cs_null = DXmCvtFCtoCS("", &byte_count, &cs_status);
	    DXmCSTextSetString(valuewindow, cs_null);
	    XmStringFree(cs_null);
	}
	{
	    XmString cs_ptr;

	    cs_ptr = DXmCvtFCtoCS(text, &byte_count, &cs_status);
	    DXmCSTextSetString(valuewindow, cs_ptr);
	    DXmCSTextSetCursorPosition(valuewindow, 0);
	    XmStringFree(cs_ptr);
	}
	DXmCSTextSetTopPosition(valuewindow, 0);
	DXmCSTextEnableRedisplay(valuewindow);

    }

    if (!card_mapped)
	XtMapWidget(cardparent);
    else
	redrawbitmap();


    /* Motif R4 method of Deiconify, works with MWM. It will always Deiconify
     * window. */
    XMapWindow(dpy, XtWindow(cardparent));
    if (DXIsXUIWMRunning(cardparent, FALSE)) {
	/* XUI R3 method of Deiconify, won't work with MWM use this if
	 * running under XUI WM.*/
	hints. flags = StateHint | IconPixmapHint;
	hints. initial_state = NormalState;
	hints. icon_pixmap = icon_pixmap;

	XSetWMHints(dpy, XtWindow(cardparent), &hints);
    }
    strcpy(temp, card_name);
    strcat(temp, card_displayed->index);

    /* Set the card Title and Icon name */
    {
	XmString cs_ptr;

	cs_ptr = DXmCvtFCtoCS(temp, &byte_count, &cs_status);
	DWI18n_SetTitle(cardparent, cs_ptr);
	DWI18n_SetIconName(cardparent, cs_ptr);
	XmStringFree(cs_ptr);
    }

    if (!card_mapped)
	card_mapped = TRUE;
    else
	set_focus_now(cardparent);
}

/*
**++
**  ROUTINE NAME: cutproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure cuts the currently selected text from the card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
cutproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char *temp, *selection;
    int numbytes, xtnumber, insertposition;
    long cs_status, byte_count;
    Arg textargs[10];
    Boolean hasSelection;
    DXmCSTextPosition left, right;

    WorkCursorDisplay();

    if (GraphicSelected)
	CutGraphic(reason);
    else {
	hasSelection = DXmCSTextGetSelectionInfo(valuewindow, &left, &right);
	if (hasSelection) {
	    XmString cs_ptr;

	    cs_ptr = DXmCSTextGetSelection(valuewindow);
	    selection = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
	    XmStringFree(cs_ptr);
	    if ((right - left) > 0) {
		XmString cs_ptr, cs_null;

		cs_ptr = DXmCSTextGetString(valuewindow);
		if (cs_ptr == NULL) {
		    temp = XtMalloc(1);
		    temp[0] = '\0';
		} else
		    temp = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
		XmStringFree(cs_ptr);
		strcpy(undostring, temp);
		XtFree(temp);
		JustCutPasted = TRUE;
		strcpy(selection_buff, selection);
		DXmCSTextClearSelection(valuewindow);
		cs_null = DXmCvtFCtoCS("", &byte_count, &cs_status);
		DXmCSTextReplace(valuewindow, left, right, cs_null);
		XmStringFree(cs_null);
		xtnumber = 0;
		XtSetArg(textargs[xtnumber], XmNcursorPosition, left);
		xtnumber++;
		XtSetValues((Widget)valuewindow, textargs, xtnumber);
		copy_to_clipboard(reason);
	    }
	} else
	    display_message("NoSelectionError");
    }
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: clearproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure cuts the bitmap from the card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void clearproc()
{
    int rem, save_width;
    int numbytes, xtnumber;
    int insertposition;
    long cs_status, byte_count;
    char *selection;
    char *temp;
    Arg textargs[10];
    Boolean hasSelection;
    DXmCSTextPosition left, right;

    if (GraphicSelected) {
	XmString cs_ptr;

	rem = bmp_width % 32;
	if (rem == 0)
	    save_width = bmp_width;
	else
	    save_width = bmp_width - rem + 32;

	if ((save_width > 0) && (bmp_height > 0))
	    mybcopy(bitmap, undo_bitmap, (bmp_height * save_width) / 8);
	undo_bmp_height = bmp_height;
	undo_bmp_width = bmp_width;

	bmp_height = 0;
	bmp_width = 0;
	undotype = CARDFILER_GRAPHIC_CHANGED;
	JustCutPasted = TRUE;

	cs_ptr = DXmCSTextGetString(valuewindow);
	if (cs_ptr == NULL) {
	    temp = XtMalloc(1);
	    temp[0] = '\0';
	} else
	    temp = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
	XmStringFree(cs_ptr);
	strcpy(text, temp);
	XtFree(temp);

	displaycard();
	bitmap_changed = TRUE;
	change_undo_sensitivity();
    } else {
	hasSelection = DXmCSTextGetSelectionInfo(valuewindow, &left, &right);
	if (hasSelection) {
	    XmString cs_ptr;

	    cs_ptr = DXmCSTextGetSelection(valuewindow);
	    selection = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
	    XmStringFree(cs_ptr);

	    if ((right - left) > 0) {
		XmString cs_ptr, cs_null;

		cs_ptr = DXmCSTextGetString(valuewindow);
		if (cs_ptr == NULL) {
		    temp = XtMalloc(1);
		    temp[0] = '\0';
		} else
		    temp = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
		XmStringFree(cs_ptr);
		strcpy(undostring, temp);
		XtFree(temp);
		undotype = CARDFILER_TEXT_CHANGED;
		JustCutPasted = TRUE;
		strcpy(selection_buff, selection);
		DXmCSTextClearSelection(valuewindow);
		cs_null = DXmCvtFCtoCS("", &byte_count, &cs_status);
		DXmCSTextReplace(valuewindow, left, right, cs_null);
		XmStringFree(cs_null);
		xtnumber = 0;
		XtSetArg(textargs[xtnumber], XmNcursorPosition, left);
		xtnumber++;
		XtSetValues((Widget)valuewindow, textargs, xtnumber);
		change_undo_sensitivity();
	    }
	} else
	    display_message("NoSelectionError");
    }
}

/*
**++
**  ROUTINE NAME: copyproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure copies the selected text into the buffer to
**	later be pasted.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
copyproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char *selection;
    int numbytes, insertposition;
    long cs_status, byte_count;
    Boolean hasSelection;
    DXmCSTextPosition left, right;

    WorkCursorDisplay();

    if (GraphicSelected)
	CopyGraphic(reason);
    else {
	hasSelection = DXmCSTextGetSelectionInfo(valuewindow, &left, &right);
	if (hasSelection) {
	    XmString cs_ptr;

	    cs_ptr = DXmCSTextGetSelection(valuewindow);
	    selection = DXmCvtCStoFC(cs_ptr, &byte_count, &cs_status);
	    XmStringFree(cs_ptr);
	    if ((right - left) > 0) {
		strcpy(selection_buff, selection);
		copy_to_clipboard(reason);
	    }
	    XtFree(selection);
	} else
	    display_message("NoSelectionError");
    }
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: selectgraphicproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure marks the graphic of the card selected.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void selectgraphicproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (!GraphicSelected) {
	GraphicSelected = TRUE;
	DXmCSTextClearSelection(valuewindow);
	same_card = TRUE;
	displaycard();
    }
}

/*
**++
**  ROUTINE NAME: deselectgraphicproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure deselects the selected graphic region of a card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void deselectgraphicproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    if (GraphicSelected) {
	GraphicSelected = FALSE;
	same_card = TRUE;
	displaycard();
    }
}

/*
**++
**  ROUTINE NAME: CutGraphic
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure copies the selected text into the buffer to
**	later be pasted.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CutGraphic(cb_reason)
    XmAnyCallbackStruct *cb_reason;
{
    CopyGraphic(cb_reason);
    clearproc();
}

/*
**++
**  ROUTINE NAME: Clipboard_Copy_Callback
**
**  FUNCTIONAL DESCRIPTION:
**	ISL action routine to copy DDIF stream piece by piece to the clipboard.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static int Clipboard_Copy_Callback(bufptr, buflen, usrparam)
    char *bufptr;
    int buflen;
    int usrparam;
{
    int st, dummy;

    st = XmClipboardCopy(dpy, XtWindow(indexparent), item_id, "DDIF", bufptr,
      buflen, dummy, &dummy);
    return (1);
}

#ifdef VMS
void InitImaging();
globaldef Boolean imagingInited = FALSE;
globaldef unsigned long int (*ImagingCopy)();
globaldef unsigned long int (*ImagingCreateDDIFStream)();
globaldef unsigned long int (*ImagingCreateFrame)();
globaldef unsigned long int (*ImagingImportBitmap)();
globaldef unsigned long int (*ImagingExportDDIFFrame)();
globaldef void (*ImagingDeleteDDIFStream)();
globaldef unsigned long int (*ImagingImportDDIFFrame)();
globaldef unsigned long int (*ImagingGetFrameAttributes)();
globaldef unsigned long int (*ImagingDecompress)();
globaldef unsigned long int (*ImagingExportBitmap)();
globaldef void (*ImagingDeleteFrame)();
globaldef unsigned long int (*ImagingOpenDDIFFile)();
globaldef int (*ImagingCloseDDIFFile)();

/*
**++
**  ROUTINE NAME: InitImaging
**
**  FUNCTIONAL DESCRIPTION:
**	Dynamically image activate the imaging shareable library.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void InitImaging()
{
    int status;

#define UPDATE_DESCR(name,string)   \
    name .dsc$w_length = sizeof(string) - 1, name .dsc$a_pointer = string;

#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    $DESCRIPTOR(imagingFile, "IMG$SHRLIB12");
#else
    $DESCRIPTOR(imagingFile, "IMG$SHRLIB"); 	/* Motif 1.1.3 */
#endif /* Motif 1.2 */

    $DESCRIPTOR(imagingSymbol, "IMG$COPY");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol, &ImagingCopy);

    UPDATE_DESCR(imagingSymbol, "IMG$CREATE_DDIF_STREAM");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingCreateDDIFStream);

    UPDATE_DESCR(imagingSymbol, "IMG$CREATE_FRAME");
    status =
      LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol, &ImagingCreateFrame);

    UPDATE_DESCR(imagingSymbol, "IMG$IMPORT_BITMAP");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingImportBitmap);

    UPDATE_DESCR(imagingSymbol, "IMG$EXPORT_DDIF_FRAME");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingExportDDIFFrame);

    UPDATE_DESCR(imagingSymbol, "IMG$DELETE_DDIF_STREAM");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingDeleteDDIFStream);

    UPDATE_DESCR(imagingSymbol, "IMG$IMPORT_DDIF_FRAME");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingImportDDIFFrame);

    UPDATE_DESCR(imagingSymbol, "IMG$GET_FRAME_ATTRIBUTES");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingGetFrameAttributes);

    UPDATE_DESCR(imagingSymbol, "IMG$DECOMPRESS");
    status =
      LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol, &ImagingDecompress);

    UPDATE_DESCR(imagingSymbol, "IMG$EXPORT_BITMAP");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingExportBitmap);

    UPDATE_DESCR(imagingSymbol, "IMG$DELETE_FRAME");
    status =
      LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol, &ImagingDeleteFrame);

    UPDATE_DESCR(imagingSymbol, "IMG$OPEN_DDIF_FILE");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingOpenDDIFFile);

    UPDATE_DESCR(imagingSymbol, "IMG$CLOSE_DDIF_FILE");
    status = LIB$FIND_IMAGE_SYMBOL(&imagingFile, &imagingSymbol,
      &ImagingCloseDDIFFile);

    imagingInited = TRUE;
}
#else
Boolean imagingInited = TRUE;
#endif

/*
**++
**  ROUTINE NAME: CopyGraphic
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure copies the selected text into the buffer to
**	later be pasted.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CopyGraphic(cb)
    XmAnyCallbackStruct *cb;
{
#ifndef NO_ISL
    char *image_data, *ddif_buf;
    int st;
    unsigned long int pixel_order, bits_per_pixel, pixels_per_scanline, 
	scanline_count, scanline_stride, image_size, image_id, new_id, 
	set_index, context, bytes;
    long cs_status, byte_count;
    struct PUT_ITMLST set_attributes[7];
    GC gc_copy;
    Window mw;
    XmString l_clip_label;
    XImage *ximage;
    XKeyEvent *event = (XKeyEvent *) cb->event;

    INIT_IMAGING;

    mw = XtWindow(indexparent);

    undotype = CARDFILER_GRAPHIC_CHANGED;
    JustCutPasted = TRUE;

    l_clip_label = DXmCvtFCtoCS(clip_label, &byte_count, &cs_status);

    st = XmClipboardStartCopy(dpy, mw, l_clip_label, event->time, indexparent,
      0, &item_id);
    XmStringFree(l_clip_label);
    bits_per_pixel = 1;
    pixel_order = ImgK_StandardPixelOrder;
    bytes = bitmap_image->bytes_per_line;
    pixels_per_scanline = bitmap_image->width;
    scanline_count = bitmap_image->height;
    scanline_stride = 8 * bytes;	/* 8 bits per byte */
    image_data = bitmap_image->data;
    image_size = bytes * scanline_count;
    start_set_itemlist(set_attributes, set_index);
    put_set_item(set_attributes, set_index, Img_PixelOrder, pixel_order);
    put_set_item(set_attributes, set_index, Img_BitsPerPixel, bits_per_pixel);
    put_set_item(set_attributes, set_index, Img_PixelsPerLine,
      pixels_per_scanline);
    put_set_item(set_attributes, set_index, Img_NumberOfLines, scanline_count);
    put_set_item(set_attributes, set_index, Img_ScanlineStride,
      scanline_stride);
    end_set_itemlist(set_attributes, set_index);
    ddif_buf = XtMalloc(BITMAP_SIZE);
    context = ImgCreateDDIFStream(ImgK_ModeExport, ddif_buf, BITMAP_SIZE, NULL,
      Clipboard_Copy_Callback, 0);
    image_id = ImgCreateFrame(set_attributes, ImgK_StypeBitonal);
    ImgImportBitmap(image_id, image_data, image_size, 0, 0, 0, 0);
    ImgExportDDIFFrame(image_id, NULL, context, 0, 0, 0, 0, 0);
    ImgDeleteDDIFStream(context);
    XmClipboardEndCopy(dpy, mw, item_id);
    XtFree(ddif_buf);
    change_undo_sensitivity();
#endif
}

/*
**++
**  ROUTINE NAME: copy_to_clipboard
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure copies the buffer to the clipboard
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void copy_to_clipboard(cb)
    XmAnyCallbackStruct *cb;

{
    int status, status1;
    long cvt_status, byte_count;
    unsigned int id, length, data_id;
    Window mw;
    XmString l_clip_label;
    XKeyEvent *event = (XKeyEvent *) cb->event;

    mw = XtWindow(indexparent);

    l_clip_label = DXmCvtFCtoCS(clip_label, &byte_count, &cvt_status);

    length = strlen(selection_buff);
    status = XmClipboardStartCopy(dpy, mw, l_clip_label, event->time,
      indexparent, 0, &id);
    XmStringFree(l_clip_label);

    if (status != ClipboardSuccess)
	display_message("NoBeginClip");
    else {
	XmString cs_ptr;

	cs_ptr = DXmCvtFCtoCS(selection_buff, &byte_count, &cvt_status);
	status = DWI18n_ClipboardCopy(dpy, mw, id, 0, cs_ptr);
	XmStringFree(cs_ptr);
	if (status != ClipboardSuccess)
	    display_message("NoClipCopy");
	status1 = XmClipboardEndCopy(dpy, mw, id);
	if (status1 != ClipboardSuccess)
	    display_message("NoEndClip");
	if ((status == ClipboardSuccess) && (status1 == ClipboardSuccess)) {
	    undotype = CARDFILER_TEXT_CHANGED;
	    change_undo_sensitivity();
	}
    }
}

/*
**++
**  ROUTINE NAME: pasteproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'pastes' or inserts the text from a copy or a cut
**	into the text.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
pasteproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int unsigned long length;
    int status;
    Window mw;

    WorkCursorDisplay();

    mw = XtWindow(indexparent);

    status = XmClipboardLock(dpy, mw);
    if (status != ClipboardSuccess)
	display_message("NoBeginClip");
    else {
	length = 0;
	status = XmClipboardInquireLength(dpy, mw, "DDIF", &length);
	if (length > 0) {
	    undotype = CARDFILER_GRAPHIC_CHANGED;
	    JustCutPasted = TRUE;
	    PasteGraphic(length);
	} else {
	    length = 0;
	    status =
	      XmClipboardInquireLength(dpy, mw, "COMPOUND_TEXT", &length);
	    if (length > 0)
		PasteText();
	    else {
		length = 0;
		status = XmClipboardInquireLength(dpy, mw, "STRING", &length);
		if (length > 0)
		    PasteText();
		else
		    display_message("NoPasteCopy");
	    }
	}
	status = XmClipboardUnlock(dpy, mw, 1);
	if (status != ClipboardSuccess)
	    display_message("NoEndClip");
    }
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: PasteText
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'pastes' or inserts the text from a copy or a cut
**	into the text.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void PasteText()
{
    char *buff, *temp;
    int i, id, length, status, xtnumber, insertposition, numbytes, text_length;
    long cvt_status, byte_count;
    Window mw;
    XmString cs_select;

    insertposition = 0;
    insertposition = DXmCSTextGetCursorPosition(valuewindow);
    mw = XtWindow(indexparent);

    status = DWI18n_ClipboardPaste(dpy, mw, &cs_select, &id);
    if (status == ClipboardSuccess) {
	XmString cs_temp;

	buff = DXmCvtCStoFC(cs_select, &byte_count, &cvt_status);
	/* strcpy(selection_buff, buff); */
	/* numbytes = strlen(selection_buff); */
	numbytes = strlen(buff);
	/* XtFree(buff); */
	cs_temp = DXmCSTextGetString(valuewindow);
	if (cs_temp == NULL) {
	    temp = XtMalloc(1);
	    temp[0] = '\0';
	} else
	    temp = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
	text_length = strlen(temp);
	/* strcpy(undostring, temp); */
	/* XtFree(temp); */
	JustCutPasted = TRUE;
	if ((numbytes + text_length) > TEXT_LENGTH)
	    display_message("SelectionTooBigError");
	else {
	    DXmCSTextReplace(valuewindow, insertposition, insertposition,
	      cs_select);
	    strcpy(undostring, temp);
	    strcpy(selection_buff, buff);
	    undotype = CARDFILER_TEXT_CHANGED;
	    change_undo_sensitivity();
	}
        XtFree(temp);
	XtFree(buff);
    }
    XmStringFree(cs_select);
}

/*
**++
**  ROUTINE NAME: PasteGraphic
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'pastes' or inserts the text from a copy or a cut
**	into the text.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void PasteGraphic(buflen)
    unsigned long int buflen;
{
#ifndef NO_ISL
    char *ddif_buf, *temp_str;
    int id, length, rem, save_width;
    unsigned long int context, status; 
    Window mw;

    INIT_IMAGING;

    mw = XtWindow(indexparent);

    rem = bmp_width % 32;
    if (rem == 0)
	save_width = bmp_width;
    else
	save_width = bmp_width - rem + 32;

    if ((save_width > 0) && (bmp_height > 0))
	mybcopy(bitmap, undo_bitmap, (bmp_height * save_width) / 8);
    undo_bmp_height = bmp_height;
    undo_bmp_width = bmp_width;

    ddif_buf = XtMalloc(buflen);
    status =
      XmClipboardRetrieve(dpy, mw, "DDIF", ddif_buf, buflen, &length, &id);

    if ((status == ClipboardSuccess) && (length > 0)) {
	context =
	  ImgCreateDDIFStream(ImgK_ModeImport, ddif_buf, buflen, 0, 0, 0);
	status = Load_DDIF_Frame(context);
    }

    if (status != OK_STATUS) {
	undotype = CARDFILER_NO_UNDO;

	switch (status) {
	    case BMP_TOOLARGE:
		display_message("GraphicTooLarge");
		break;

	    case NOT_VALID_TYPE:
		display_message("GraphicMultiPlane");
		break;

	    case DDIF_NOGRAPHIC:
		PasteText();
		break;

	    default:
		display_message("DDIFReadError");
		break;
	}
    } else {
	XmString cs_temp;
	long byte_count, cvt_status;

	cs_temp = DXmCSTextGetString(valuewindow);
	if (cs_temp == NULL) {
	    temp_str = XtMalloc(1);
	    temp_str[0] = '\0';
	} else
	    temp_str = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	XmStringFree(cs_temp);
	strcpy(text, temp_str);
	XtFree(temp_str);

	bitmap_changed = TRUE;
	displaycard();
	change_undo_sensitivity();
    }
    XtFree(ddif_buf);
#endif
}

/*
**++
**  ROUTINE NAME: textwidgetcalls
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'undoes' the last cut, paste or undo.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void textwidgetcalls(widget, tag, cbs)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *cbs;
{
    Boolean TextSelected;
    char *temp_str;
    int reason = (int) cbs->reason;
    /* DXmCSTextPosition left, right; */

#ifdef DEBUG
    printf("textwidgetcalls called\n");
#endif
    if (reason == XmCR_MOVING_INSERT_CURSOR) {
#ifdef DEBUG
	printf("reason = XmCR_MOVING_INSERT_CURSOR\n");
#endif
        /* TextSelected = DXmCSTextGetSelectionInfo(valuewindow, left, right); */
        TextSelected = DXmCSTextHasSelection(valuewindow); 
	if (GraphicSelected && TextSelected) {
	    XmString cs_temp;
	    long byte_count, cvt_status;

	    GraphicSelected = FALSE;
	    cs_temp = DXmCSTextGetString(valuewindow);
	    if (cs_temp == NULL) {
		temp_str = XtMalloc(1);
		temp_str[0] = '\0';
	    } else
		temp_str = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
	    XmStringFree(cs_temp);
	    strcpy(text, temp_str);
	    XtFree(temp_str);
	    same_card = TRUE;
	    displaycard();
	}
    }
#ifdef NO_USE
    else if (reason == XmCR_VALUE_CHANGED) {
	if (JustCutPasted)
	    JustCutPasted = FALSE;
	else
	    undotype = CARDFILER_NO_UNDO;
    }
#endif
}

/*
**++
**  ROUTINE NAME: undoproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'undoes' the last cut, paste or undo.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
undoproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    int rem, save_width, numbytes, text_length;
    char *buff, *temp;
    char temp_bitmap[BITMAP_SIZE];	/* bitmap CUT from card */
    Dimension temp_bmp_height = 0;	/* height of CUT bitmap */
    Dimension temp_bmp_width = 0;	/* width of CUT bitmap */
    XmString cs_temp;
    long byte_count, cvt_status;

    WorkCursorDisplay();
    cs_temp = DXmCSTextGetString(valuewindow);
    if (cs_temp == NULL) {
	temp = XtMalloc(1);
	temp[0] = '\0';
    } else
	temp = DXmCvtCStoFC(cs_temp, &byte_count, &cvt_status);
    XmStringFree(cs_temp);
    text_length = strlen(temp);

    switch (undotype) {
	case CARDFILER_TEXT_CHANGED:
	    {
		XmString cs_temp;

		cs_temp = DXmCvtFCtoCS(undostring, &byte_count, &cvt_status);
		DXmCSTextSetString(valuewindow, cs_temp);
		XmStringFree(cs_temp);
	    }
	    strcpy(undostring, temp);
	    undotype = CARDFILER_TEXT_CHANGED;
	    JustCutPasted = TRUE;
	    change_undo_sensitivity();
	    WorkCursorRemove();
	    break;

	case CARDFILER_GRAPHIC_CHANGED:
	    /* Save the current bitmap into a temporary bitmap */
	    rem = bmp_width % 32;
	    if (rem == 0)
		save_width = bmp_width;
	    else
		save_width = bmp_width - rem + 32;

	    if ((save_width > 0) && (bmp_height > 0))
		mybcopy(bitmap, temp_bitmap, (bmp_height * save_width) / 8);
	    temp_bmp_height = bmp_height;
	    temp_bmp_width = bmp_width;

	    /* Replace the current bitmap with the undo bitmap */
	    rem = undo_bmp_width % 32;
	    if (rem == 0)
		save_width = undo_bmp_width;
	    else
		save_width = undo_bmp_width - rem + 32;

	    if ((save_width > 0) && (undo_bmp_height > 0))
		mybcopy(undo_bitmap, bitmap,
		  (undo_bmp_height * save_width) / 8);

	    bmp_height = undo_bmp_height;
	    bmp_width = undo_bmp_width;

	    /* Replace the undo bitmap with the temporary bitmap */
	    rem = temp_bmp_width % 32;
	    if (rem == 0)
		save_width = temp_bmp_width;
	    else
		save_width = temp_bmp_width - rem + 32;

	    if ((save_width > 0) && (bmp_height > 0))
		mybcopy(temp_bitmap, undo_bitmap,
		  (bmp_height * save_width) / 8);
	    undo_bmp_height = temp_bmp_height;
	    undo_bmp_width = temp_bmp_width;
	    bitmap_changed = TRUE;
	    undotype = CARDFILER_GRAPHIC_CHANGED;
	    JustCutPasted = TRUE;
	    change_undo_sensitivity();
	    displaycard();
	    WorkCursorRemove();
	    break;

	default:
	    WorkCursorRemove();
	    display_message("NothingToUndoError");
	    break;
    }					/* end SWITCH */
    XtFree(temp);
}

/*
**++
**  ROUTINE NAME: save_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
**	Save the current size and position of the index and card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
save_settings_proc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char resource_value[20], resource_name[256], resource_class[256];
    Dimension width, height, sash;
    Position x, y;
    XrmValue put_value;
    int ac;
    Arg al[2];

    WorkCursorDisplay();
    if (user_database == NULL)
	user_database = XrmGetFileDatabase(defaults_name);

    width = XtWidth(indexparent);
    height = XtHeight(indexparent);
    XtTranslateCoords(indexparent, 0, 0, &x, &y);
    x = x - XtBorderWidth(indexparent);
    y = y - XtBorderWidth(indexparent);

    sprintf(resource_value, "%d", x);
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;

    XrmPutResource(&user_database, xrm_index_x, XtRString, &put_value);

    sprintf(resource_value, "%d", y);
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;

    XrmPutResource(&user_database, xrm_index_y, XtRString, &put_value);

    sprintf(resource_value, "%d", width);
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;

    XrmPutResource(&user_database, xrm_index_width, XtRString, &put_value);

    sprintf(resource_value, "%d", height);
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;

    XrmPutResource(&user_database, xrm_index_height, XtRString, &put_value);

    width = XtWidth(cardparent);
    height = XtHeight(cardparent);
    XtTranslateCoords(cardparent, 0, 0, &x, &y);
    x = x - XtBorderWidth(cardparent);
    y = y - XtBorderWidth(cardparent);

    if ((width > 0) && (height > 0)) {
	sprintf(resource_value, "%d", x);
	put_value. addr = resource_value;
	put_value. size = strlen(resource_value) + 1;

	XrmPutResource(&user_database, xrm_card_x, XtRString, &put_value);

	sprintf(resource_value, "%d", y);
	put_value. addr = resource_value;
	put_value. size = strlen(resource_value) + 1;

	XrmPutResource(&user_database, xrm_card_y, XtRString, &put_value);

	sprintf(resource_value, "%d", width);
	put_value. addr = resource_value;
	put_value. size = strlen(resource_value) + 1;

	XrmPutResource(&user_database, xrm_card_width, XtRString, &put_value);

	sprintf(resource_value, "%d", height);
	put_value. addr = resource_value;
	put_value. size = strlen(resource_value) + 1;

	XrmPutResource(&user_database, xrm_card_height, XtRString, &put_value);

	height = XtHeight(card_scroll_window);

	sprintf(resource_value, "%d", height);
	put_value. addr = resource_value;
	put_value. size = strlen(resource_value) + 1;

	XrmPutResource(&user_database, xrm_image_height, XtRString, &put_value);

    }

    XrmPutFileDatabase(user_database, defaults_name);
    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: restore_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
**	Restore the current size and position of the index and card.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
restore_settings_proc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    char *get_type, resource_name[256], resource_class[256];
    int ac, status, intx, inty, intwidth, intheight;
    Arg args[10];
    Dimension width, height;
    Position x, y;
    XrmValue get_value;

    WorkCursorDisplay();
    if (system_database == NULL)
	system_database = XrmGetFileDatabase(system_defaults_name);

    intx = 200;
    inty = 200;
    intwidth = 240;
    intheight = 300;

    status =
      XrmGetResource(system_database, 	/* Database. */
      xrm_index_x, 			/* Resource's ASCIZ name. */
      xrc_index_x, 			/* Resource's ASCIZ class. */
      &get_type, 			/* Resource's type (out). */
      &get_value);			/* Address to return value. */
    if (status != NULL)
	intx = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_index_y, xrc_index_y,
      &get_type, &get_value);
    if (status != NULL)
	inty = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_index_width, xrc_index_width,
      &get_type, &get_value);
    if (status != NULL)
	intwidth = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_index_height,
      xrc_index_height, &get_type, &get_value);
    if (status != NULL)
	intheight = atoi(get_value. addr);

    x = (Position) intx;
    y = (Position) inty;
    width = (Dimension) intwidth;
    height = (Dimension) intheight;

    ac = 0;
    XtSetArg(args[ac], XmNx, x);
    ac++;
    XtSetArg(args[ac], XmNy, y);
    ac++;
    XtSetValues(indexparent, args, ac);
    ac = 0;
    XtSetArg(args[ac], XmNwidth, width);
    ac++;
    XtSetArg(args[ac], XmNheight, height);
    ac++;
    XtSetValues((Widget)indexmainwindow, args, ac);

    intx = x + width + 8;
    inty = y + 8;
    intwidth = 340;
    intheight = 210;

    status = XrmGetResource(system_database, xrm_card_x, xrc_card_x, &get_type,
      &get_value);
    if (status != NULL)
	intx = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_card_y, xrc_card_y, &get_type,
      &get_value);
    if (status != NULL)
	inty = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_card_width, xrc_card_width,
      &get_type, &get_value);
    if (status != NULL)
	intwidth = atoi(get_value. addr);

    status = XrmGetResource(system_database, xrm_card_height, xrc_card_height,
      &get_type, &get_value);
    if (status != NULL)
	intheight = atoi(get_value. addr);

    x = (Position) intx;
    y = (Position) inty;
    width = (Dimension) intwidth;
    height = (Dimension) intheight;

    ac = 0;
    XtSetArg(args[ac], XmNx, x);
    ac++;
    XtSetArg(args[ac], XmNy, y);
    ac++;
    XtSetValues(cardparent, args, ac);
    ac = 0;
    XtSetArg(args[ac], XmNwidth, width);
    ac++;
    XtSetArg(args[ac], XmNheight, height);
    ac++;
    XtSetValues((Widget)cardmainwindow, args, ac);

    WorkCursorRemove();
}

/*
**++
**  ROUTINE NAME: WorkCursorCreate
**
**  FUNCTIONAL DESCRIPTION:
**	This routine creates a "wait" cursor
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
Cursor WorkCursorCreate(wid)
    Widget wid;				/* Widget to be used to create the
					 * cursor.  The fields used are the
					 * display and the colormap.  Any
					 * widget  with the same display and
					 * colormap can be used later with this
					 * cursor */
{
    int status;
    Cursor cursor;
    Font font;
    XColor fcolor, bcolor, dummy;

    font = XLoadFont(XtDisplay(wid), "DECw$Cursor");
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "Black",
      &fcolor, &dummy);
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "White",
      &bcolor, &dummy);
    cursor = XCreateGlyphCursor(XtDisplay(wid), font, font, decw$c_wait_cursor,
      decw$c_wait_cursor + 1, &fcolor, &bcolor);
    return cursor;
}

/*
**++
**  ROUTINE NAME: WorkCursorDisplay
**
**  FUNCTIONAL DESCRIPTION:
**	This routine displays a cursor a watch cursor in both the index and card
**	windows.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorDisplay()
{
    /* Display the watch cursor in the Index window */
    if (watch_cursor == NULL) {
	watch_cursor = DXmCreateCursor(indexparent, decw$c_wait_cursor);
    }

    CursorDisplay(indexparent, indexparent, watch_cursor);

    /* If the card window exists, Display the watch cursor */
    if (card_mapped) {
	if (card_watch_cursor == NULL) {
	    card_watch_cursor =
	      DXmCreateCursor(cardparent, decw$c_wait_cursor);
	}
	CursorDisplay(cardparent, cardparent, card_watch_cursor);
    }
    /* Use XtAddGrab to enable toolkit filtering of events */
    XtAddGrab(indexparent, TRUE, FALSE);
    XFlush(dpy);
}

/*
**++
**  ROUTINE NAME: WorkCursorRemove
**
**  FUNCTIONAL DESCRIPTION:
**	Return to the normal cursor in both windows
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorRemove()
{
    CursorRemove(indexparent, indexparent);
    if (card_mapped)
	CursorRemove(cardparent, cardparent);
    /* Use XtRemoveGrab to disable toolkit filtering of events */
    XtRemoveGrab(indexparent);
}

/*
**++
**  ROUTINE NAME: CursorDisplay
**
**  FUNCTIONAL DESCRIPTION:
**	This routine displays a cursor, and initiates toolkit filtering
**	of input events
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CursorDisplay(cursor_wid, grab_wid, cursor)
    Widget cursor_wid;			/*  Widget to display wait cursor in
					 *  (Usually application main window)
					 */
    Widget grab_wid;			/*  Widget to use in call to XtAddGrab.
					 *  This widget will be the only widget
					 *  to have all events passed to it.
					 *  It is easiest to use an unmapped
					 *  widget. */
    Cursor cursor;			/*  Cursor to display; does not HAVE to
					 *  be created by CursorCreate */
{
    /* Use XDefineCursor to set the window to the supplied cursor */
    XDefineCursor(XtDisplay(cursor_wid), XtWindow(cursor_wid), cursor);
}

/*
**++
**  ROUTINE NAME: CursorRemove
**
**  FUNCTIONAL DESCRIPTION:
**	This routine un-displays a cursor, and terminates toolkit filtering
**	of input events
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CursorRemove(cursor_wid, grab_wid)
    Widget cursor_wid;			/*  Same two widgets used in call to
					 *  CursorDisplay */
    Widget grab_wid;
{
    /* Use XUndefineCursor to undo the cursor */
    XUndefineCursor(XtDisplay(cursor_wid), XtWindow(cursor_wid));
}

/*
**++
**  ROUTINE NAME: printproc
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure 'pastes' or inserts the text from a copy or a cut
**	into the text.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
printproc(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
#ifndef NO_PWG
    int text_length, argc;
    int type = (int) *tag;
    int bm_present = FALSE;
    long cs_status, byte_count;
    char *dummy_name;
    FILE *fp;
    card_pointer current;
    Arg arglist[20];
    Arg al[2];
    XmString CS;

    if ((fp = fopen(pid_print_file, WRITE_BINARY)) == NULL)
	display_message("OpenPrintError");
    else {
	if (card_displayed != NULL)
	    check_and_save(card_displayed);

	XtSetArg(al[0], XmNcolumns, &text_length);
	XtGetValues((Widget)valuewindow, al, 1);

	if ((type == k_print_all) || (type == k_print_all_now)) {
	    current = first;
	    while (current->next != NULL) {
		if ((bmp_width != 0) || (bmp_height != 0))
		    bm_present = TRUE;
		outputonecard(fp, current, text_length);
		fprintf(fp, "\n\n");
		current = current->next;
	    }
	} else {
	    if ((bmp_width != 0) || (bmp_height != 0))
		bm_present = TRUE;
	    outputonecard(fp, card_displayed, text_length);
	}

	fclose(fp);

	if ((type == k_print_one_now) || (type == k_print_one))
	    PrintFromIndex = FALSE;
	else
	    PrintFromIndex = TRUE;

	if (print_widget_id == NULL) {

	    WorkCursorDisplay();
	    CS = DXmCvtFCtoCS(print_title, &byte_count, &cs_status);
	    argc = 0;
	    XtSetArg(arglist[argc], XmNdialogTitle, CS);
	    argc++;
	    XtSetArg(arglist[argc], XmNdefaultPosition, TRUE);
	    argc++;
	    XtSetArg(arglist[argc], DXmNfileNameList, printfilename);
	    argc++;
	    XtSetArg(arglist[argc], DXmNfileNameCount, 1);
	    argc++;
	    XtSetArg(arglist[argc], DXmNprintFormatList,
	      us_supplied_data_syntax);
	    argc++;
	    XtSetArg(arglist[argc], DXmNprintFormatCount, 1);
	    argc++;
	    XtSetArg(arglist[argc], XmNokCallback, print_ok_cbs);
	    argc++;
	    XtSetArg(arglist[argc], XmNcancelCallback, print_cancel_cbs);
	    argc++;

	    print_widget_id =
	      DXmCreatePrintDialog(indexparent, "IndexPrint", arglist, argc);

	    card_print_widget_id =
	      DXmCreatePrintDialog(cardparent, "CardPrint", arglist, argc);

	    XmStringFree(CS);

	    WorkCursorRemove();

	}

	if ((type == k_print_one_now) || (type == k_print_all_now)) {
	    XtSetArg(al[0], DXmNdeleteFile, TRUE);
	    if (type == k_print_one_now) {
		XtSetValues(card_print_widget_id, al, 1);
		DXmPrintWgtPrintJob(card_print_widget_id, printfilename, 1);
	    } else {
		XtSetValues(print_widget_id, al, 1);
		DXmPrintWgtPrintJob(print_widget_id, printfilename, 1);
	    }
	} else {
	    if (type == k_print_one) {
		XtManageChild(card_print_widget_id);
	    } else {
		XtManageChild(print_widget_id);
	    }
	}
    }
    if (bm_present)
	display_message("NoPrintBitmaps");
#endif
}

/*
**++
**  ROUTINE NAME: outputonecard
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
outputonecard(fp, current, text_length)
    FILE *fp;
    card_pointer current;
    int text_length;
{
    char *temp, *temp1, *temp2;
    int length, index;

    /* Read in the data for the current card that we're writing out. */
    readonecard(current, &temp_card1);

    /* Write out the index */
    fprintf(fp, "%s\n", temp_card1. index);

    /* Get text */
    temp = temp_card1. text;
    temp1 = temp;
    length = strlen(temp1);
    while (temp1 <= temp + length) {

	/* Find next newline and replace with null to print it. */
	temp2 = (char *) myindex(temp1, '\n');

	/* special condition if no more newlines (last line) */
	if (temp2 == 0)
	    temp2 = temp + length;

	index = temp2 - temp1;
	temp1[index] = '\0';
	fprintf(fp, "%s\n", temp1);

	/* put newline back in (may not be needed but put in anyway) */
	temp1[index] = '\n';
	temp2++;
	temp1 = temp2;
    }

}

/*
**++
**  ROUTINE NAME: print_ok_cb
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**	widget - the scrollbar widget
**	tag - not used
**	other - scrollbar event structure for this callback
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void print_ok_cb(widget, tag, other)
    Widget widget;
    int *tag;
    unsigned long *other;
{
#ifndef NO_PWG
    Arg al[2];

    XtSetArg(al[0], DXmNdeleteFile, TRUE);

    if (PrintFromIndex) {
	set_focus_now(indexparent);
	XtUnmanageChild(print_widget_id);
	XtSetValues(print_widget_id, al, 1);
	DXmPrintWgtPrintJob(print_widget_id, printfilename, 1);
    } else {
	set_focus_now(cardparent);
	XtUnmanageChild(card_print_widget_id);
	XtSetValues(card_print_widget_id, al, 1);
	DXmPrintWgtPrintJob(card_print_widget_id, printfilename, 1);
    }
#endif
}

/*
**++
**  ROUTINE NAME: print_cancel_cb
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**	widget - the print widget
**	tag - not used
**	other - not used
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void print_cancel_cb(widget, tag, other)
    Widget widget;
    int *tag;
    unsigned long *other;
{
#ifndef NO_PWG
    if (PrintFromIndex) {
	set_focus_now(indexparent);
	XtUnmanageChild(print_widget_id);
    } else {
	set_focus_now(cardparent);
	XtUnmanageChild(card_print_widget_id);
    }
#ifdef VMS
    delete(pid_print_file);
#else
    unlink(pid_print_file);
#endif
#endif
}

/*
**++
**  ROUTINE NAME: edit_pulling
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure sets the "graying" of the edit menu buttons
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void edit_pulling(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    Boolean hasSelection, CanUndo;
    DXmCSTextPosition left, right;

    if (GraphicSelected) {
	hasSelection = TRUE;
    } else {
	hasSelection = DXmCSTextGetSelectionInfo(valuewindow, &left, &right);
    }

    change_undo_sensitivity();
    XtSetSensitive((Widget)cut_button, hasSelection);
    XtSetSensitive((Widget)copy_button, hasSelection);
}
/*
**++
**  ROUTINE NAME: change_undo_sensitivity
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure sets the "graying" of the undo button
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void change_undo_sensitivity()
{
    Boolean CanUndo;

    if (undotype == CARDFILER_NO_UNDO)
	CanUndo = FALSE;
    else
	CanUndo = TRUE;

    XtSetSensitive((Widget)undo_button, CanUndo);
}

/*
**++
**  ROUTINE NAME: display_message
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure creates a message dialog box. It takes the message as
**	a parameter, creates and maps the dialog box and then waits until
**	the user hits the continue button before allowing the program to
**	continue.
**
**  FORMAL PARAMETERS:
**	message - pointer to the message to be displayed
**	parentwindow - window in which this dialog box
**		should be centered
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
display_message(message)
    char *message;
{
    Arg message_args[1];
    MrmType *dummyclass;

    XtSetArg(message_args[0], XmNmessageString, message);

    XBell(dpy, 0);
    if (card_mapped) {
	if (card_message_dialog == (XmMessageBoxWidget) NULL) {
	    if (MrmFetchWidget(card_hierarchy, "card_message_dialog",
	      cardworkarea, &card_message_dialog, &dummyclass) != MrmSUCCESS)
		card_serror(NoCardMessage, FATAL);
	}
	if (!XtIsManaged(card_message_dialog)) {
	    MrmFetchSetValues(card_hierarchy, card_message_dialog,
	      message_args, 1);
	    XtManageChild((Widget)card_message_dialog);
	}
    } else {
	if (message_dialog == (XmMessageBoxWidget) NULL) {
	    if (MrmFetchWidget(card_hierarchy, "message_dialog", indexworkarea,
	      &message_dialog, &dummyclass) != MrmSUCCESS)
		card_serror(NoCardMessage, FATAL);
	}
	if (!XtIsManaged(message_dialog)) {
	    MrmFetchSetValues(card_hierarchy, message_dialog, message_args, 1);
	    XtManageChild((Widget)message_dialog);
	}
    }
}

display_error(message)
    char *message;
{
    Arg message_args[1];
    MrmType *dummyclass;

    XBell(dpy, 0);
    XtSetArg(message_args[0], XmNmessageString, message);
    XBell(dpy, 0);
    if (error_message_dialog == (XmMessageBoxWidget) NULL) {
	if (MrmFetchWidget(card_hierarchy, "error_dialog", indexworkarea,
	   &error_message_dialog, &dummyclass) != MrmSUCCESS)
	    card_serror(NoCardError, FATAL);
    }
    if (!XtIsManaged(error_message_dialog)) {
	MrmFetchSetValues(card_hierarchy, error_message_dialog, message_args, 1);
	XtManageChild((Widget)error_message_dialog);
    }
}

/*
**++
**  ROUTINE NAME: card_serror
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**	message - pointer to the message to be displayed
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
card_serror(message, level)
    char *message;
    int level;
{
    fprintf(stderr, message);
    if (level == FATAL)
	exit(ERROR_STATUS);
}

/*
**++
**  ROUTINE NAME: resize_card
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure handles configure events on the card work area.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void resize_card(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
    int xtnumber;
    Arg new_args[10];
#ifdef NEW_LAYOUT
    unsigned int borderwidth;
    Dimension val_width, val_height, width, height;
    Position bb_y, val_y;
    Window wid;
#endif 
    Position bb_x;
    Dimension width2, height2;

#ifdef NEW_LAYOUT
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNwidth, &width);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNheight, &height);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNborderWidth, &borderwidth);
    xtnumber++;
    XtGetValues((Widget)cardworkarea, new_args, xtnumber);

    /* get button box size */
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNwidth, &width2);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNheight, &height2);
    xtnumber++;
    XtGetValues(buttonbox, new_args, xtnumber);

    if (width <= 53)
	val_width = 50;
    else
	val_width = width;

    if (height <= (height2 + bmp_height + 38))
	val_height = 38;
    else
	val_height = height - height2 - bmp_height;

    val_width = val_width - 2 * borderwidth;
    val_height = val_height - 2 * borderwidth;

    if ((prev_width == val_width) && (prev_height == val_height) &&
      (total_prev_height == height))
	return;

    if (bmp_height == 0)
	val_y = 0;
    else {
	val_y = bmp_height + 1;
	val_height = val_height - 1;
    }

    prev_width = val_width;
    prev_height = val_height;

    if (val_y > (height - height2 - 38))
	val_y = height - height2 - 38;

    XtMoveWidget((Widget)s_valuewindow, 0, val_y);
    XtResizeWidget((Widget)s_valuewindow, val_width, val_height, borderwidth);

    /* Move buttons and button box before moving and resize Text. */

    if ((total_prev_width == width) && (total_prev_height == height))
	return;

    total_prev_width = width;
    total_prev_height = val_y + val_height + height2;

    bb_y = (Position) val_height + val_y;
    if (bb_y <= 0)
	bb_y = 1;
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNy, bb_y);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNwidth, width);
    xtnumber++;
    XtSetValues(buttonbox, new_args, xtnumber);
#endif

    /* Center the Close Button by getting the
       size of the button and then simply /2 */
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNwidth, &width2);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNheight, &height2);
    xtnumber++;
    XtGetValues((Widget)bb_close, new_args, xtnumber);

    bb_x = (Position) width2 / 2;
    if (bb_x <= 0)
	bb_x = 1;
    bb_x = -bb_x;
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNleftAttachment, XmATTACH_POSITION);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNleftPosition, 50);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNleftOffset, bb_x);
    xtnumber++;
    XtSetValues((Widget)bb_close, new_args, xtnumber);

    /* make the arrow buttons square by getting the height and 
     * setting width equal to the height */
    /* left arrow button */
    xtnumber = 0;
    XtSetArg(new_args[xtnumber], XmNwidth, &width2);
    xtnumber++;
    XtSetArg(new_args[xtnumber], XmNheight, &height2);
    xtnumber++;
    XtGetValues((Widget)bb_button1, new_args, xtnumber);

    if (width2 != height2) {
	xtnumber = 0;
	XtSetArg(new_args[xtnumber], XmNwidth, height2);
	xtnumber++;
	XtSetValues((Widget)bb_button1, new_args, xtnumber);
    }

    /* right arrow button */
    xtnumber = 0; 
    XtSetArg(new_args[xtnumber], XmNwidth, &width2);
    xtnumber++; 
    XtSetArg(new_args[xtnumber], XmNheight, &height2);
    xtnumber++;
    XtGetValues((Widget)bb_button2, new_args, xtnumber);

    if (width2 != height2) {
	xtnumber = 0;
	XtSetArg(new_args[xtnumber], XmNwidth, height2);
	xtnumber++;
	XtSetValues((Widget)bb_button2, new_args, xtnumber);
    }
}

/*
**++
**  ROUTINE NAME: redrawbitmap
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure handles configure events on the card work area.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void redrawbitmap(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
    int rem, save_width, ac = 0;
    Arg al[5];
    Window win;
    XGCValues gcv;

    if (card_mapped)
	if (GrabFocus) {
	    set_focus_now(cardparent);
	    GrabFocus = FALSE;
	}

    if (bmp_width == 0) {
	/* No bitmap, set the graphic buttons insensitive. */
	XtSetSensitive((Widget)deselect_graphic_button, FALSE);
	XtSetSensitive((Widget)select_graphic_button, FALSE);
	if (XtIsManaged(card_scroll_window)) 
	    XtUnmanageChild((Widget)card_scroll_window);
    } else {
	if (!XtIsManaged(card_scroll_window)) 
	    XtManageChild((Widget)card_scroll_window);
	win = XtWindow(cardimagearea);
	if (bitmap_gc == NULL) {
	    bitmap_pm = XCreatePixmap(dpy, win, bitmap_pm_wid, bitmap_pm_hyt, 1);

	    XtSetArg(al[ac], XmNforeground, &bitmap_fg);
	    ac++;
	    XtSetArg(al[ac], XmNbackground, &bitmap_bg);
	    ac++;
	    XtGetValues((Widget)cardimagearea, al, ac);

	    gcv. foreground = bitmap_fg;
	    gcv. background = bitmap_bg;

	    pixmap_gc =
	      XCreateGC(dpy, bitmap_pm, GCForeground | GCBackground, &gcv);
	    bitmap_gc = XCreateGC(dpy, win, GCForeground | GCBackground, &gcv);

	    gcv. foreground = bitmap_bg;
	    gcv. background = bitmap_fg;

	    bitmap_clear_gc =
	      XCreateGC(dpy, win, GCForeground | GCBackground, &gcv);
	}

	if ((bmp_width > bitmap_pm_wid) || (bmp_height > bitmap_pm_hyt)) {
	    XFreePixmap(dpy, bitmap_pm);
	    bitmap_pm_wid = bmp_width;
	    bitmap_pm_hyt = bmp_height;
	    bitmap_pm =
	      XCreatePixmap(dpy, win, bitmap_pm_wid, bitmap_pm_hyt, 1);
	}

	if ((prev_bmp_width > bmp_width) || (prev_bmp_height > bmp_height))
	    XFillRectangle(dpy, win, bitmap_clear_gc, 0, 0, prev_bmp_width,
	      prev_bmp_height);

	if (bitmap_image != NULL)
	    XDestroyImage(bitmap_image);
	bitmap_image = XCreateImage(dpy, XDefaultVisual(dpy, DefaultScreen(
	  dpy)), 1, XYPixmap, 0, bitmap, bmp_width, bmp_height, 32, 0);
/*
**	bitmap_image->bitmap_bit_order = NATIVE_BIT_ORDER;
**	bitmap_image->byte_order = NATIVE_BYTE_ORDER;
*/
	XPutImage(dpy, bitmap_pm, pixmap_gc, bitmap_image, 0, 0, 0, 0,
	  bmp_width, bmp_height);

	if (GraphicSelected) {
	    XtSetSensitive((Widget)deselect_graphic_button, TRUE);
	    XtSetSensitive((Widget)select_graphic_button, FALSE);
	    XCopyPlane(dpy, bitmap_pm, win, bitmap_clear_gc, 0, 0, bmp_width,
	      bmp_height, 0, 0, 1);
	} else {
	    XtSetSensitive((Widget)deselect_graphic_button, FALSE);
	    XtSetSensitive((Widget)select_graphic_button, TRUE);
	    XCopyPlane(dpy, bitmap_pm, win, bitmap_gc, 0, 0, bmp_width,
	      bmp_height, 0, 0, 1);
	}
	/* set XmNdrawingArea to size of bitmap */
	ac = 0;
	XtSetArg(al[ac], XmNwidth, bmp_width);
	ac++;
	XtSetArg(al[ac], XmNheight, bmp_height);
	ac++;
	XtSetValues((Widget)cardimagearea, al, ac);

    }
    prev_bmp_height = bmp_height;
    prev_bmp_width = bmp_width;
}

/*
**++
**  ROUTINE NAME: selectbitmap
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure handles configure events on the card work area.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void selectbitmap(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
    if (GraphicSelected)
	deselectgraphicproc(NULL,NULL,NULL);
    else 
	selectgraphicproc(NULL,NULL,NULL);
}

/*
**++
**  ROUTINE NAME: icon_state_changed
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure handles configure events on the index work area.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
/*   XUI property
static	char		state_changed_atom [] = "DEC_WM_ICON_STATE";
*/
/*    Motif WM  */
static char state_changed_atom[] = "WM_STATE";

void icon_state_changed(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
    int format_return;
    unsigned long bytes_after_return;
    unsigned char *prop;
    unsigned long nitems_return;
    Atom icon_atom, type_return;
    Window win;
    XPropertyEvent *property;		/* Property notification. */

    property = (XPropertyEvent *) event;

    win = XtWindow(cardparent);

    /* Return the atom only if it exists. */
    icon_atom = XInternAtom(dpy, state_changed_atom, TRUE);

    if (icon_atom == (property->atom)) {
	XGetWindowProperty(dpy, win, icon_atom, 0, 1, FALSE, AnyPropertyType,
	  &type_return, &format_return, &nitems_return, &bytes_after_return,
	  &prop);

	if (*prop) {
	    if (*prop == 3) {		/* icon state */
		CardDeIconified = FALSE;
		IconState = FALSE;
		GrabFocus = FALSE;
#ifdef DEBUG
		printf("card window iconified\n");
#endif
	    } else {
		if (*prop == 1) {	/* normal state */
		    CardDeIconified = TRUE;
		    IconState = TRUE;
		    GrabFocus = TRUE;
		    redrawbitmap(NULL);
#ifdef DEBUG
		    printf("card window de-iconified\n");
#endif
		}			/* endif prop =1     */
	    }				/* end else          */
	}				/* endif (* prop)    */
    }					/* endif (icon_atom) */
}					/* end of routine    */

/*
**++
**  ROUTINE NAME: mb3help
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is never called.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mb3help(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
    XmString Frame;
    long cs_status, byte_count;

    Frame = DXmCvtFCtoCS("Overview", &byte_count, &cs_status);
    DisplayHelp(Frame, atoi(*params));
    XmStringFree(Frame);
}

/*
**++
**  ROUTINE NAME: ButtonPressed
**
**  FUNCTIONAL DESCRIPTION:
**	Translation callback for Prev/Next Button Pressed
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void ButtonPressed(widget, event, argv, argc)
    Widget widget;
    XButtonEvent *event;
    char **argv;
    int argc;
{
    XmPushButtonWidget pbw;

    pbw = (XmPushButtonWidget) widget;

    timer_went_off = FALSE;
    button_timerid = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)pbw), 
			470, button_timeouthandler, pbw);

    CardPushFillHighlight(pbw);
}

/*
**++
**  ROUTINE NAME: ButtonReleased
**
**  FUNCTIONAL DESCRIPTION:
**	Translation callback for Prev/Next Button Released
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void ButtonReleased(widget, event, argv, argc)
    Widget widget;
    XButtonEvent *event;
    char **argv;
    int argc;
{
    XmPushButtonWidget pbw;

    pbw = (XmPushButtonWidget) widget;

    if (button_timerid != NULL) {
	XtRemoveTimeOut(button_timerid);
	button_timerid = NULL;
    }

    if (!timer_went_off) {
	if (pbw == (XmPushButtonWidget) bb_button2)
	    nextproc();
	else
	    previousproc();
    }
    CardPushFillUnHighlight(pbw);
}

/*
**++
**  ROUTINE NAME: LeaveWindow
**
**  FUNCTIONAL DESCRIPTION:
**	Translation callback when mouse leaves a button
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void LeaveWindow(widget, event, argv, argc)
    Widget widget;
    XButtonEvent *event;
    char **argv;
    int argc;
{
    XmPushButtonWidget pbw;

    pbw = (XmPushButtonWidget) widget;

    if (button_timerid != NULL) {
	XtRemoveTimeOut(button_timerid);
	button_timerid = NULL;
    }
    CardPushFillUnHighlight(pbw);
}

/*
**++
**  ROUTINE NAME: button_timeouthandler
**
**  FUNCTIONAL DESCRIPTION:
**	Translation callback for Prev/Next Button Released
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void button_timeouthandler(closure, id)
    XtPointer closure;
    XtIntervalId id;
{
    XmPushButtonWidget pbw;

    pbw = (XmPushButtonWidget) closure;

    if (pbw == (XmPushButtonWidget) bb_button2)
	nextproc();
    else
	previousproc();

    button_timerid = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)pbw), 
			400, (XtTimerCallbackProc)button_timeouthandler, pbw);
    timer_went_off = TRUE;
}

/*
**++
**  ROUTINE NAME: no_op
**
**  FUNCTIONAL DESCRIPTION:
**	This procedure is never called.
**
**  FORMAL PARAMETERS:
**	event - the XEvent that is causing this routine to be called.
**	param - client data that is not used in this case.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void no_op(widget, event, params, number)
    Widget widget;
    XEvent *event;
    char **params;
    int number;
{
}

/*
**++
**  ROUTINE NAME: fetch_widget
**
**  FUNCTIONAL DESCRIPTION:
**	Fetches a widget from the MRM database.
**	Basically an interface on top of MrmFetchWidget.
**
**  FORMAL PARAMETERS:
**	name - name of widget to be fetched.
**	parent - widget to be the parent of widget fetched.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	widget id of widget fetched
**
**  SIDE EFFECTS:
**--
**/
Widget fetch_widget(name, parent)
    char *name;
    Widget parent;
{
    MrmType *dummyclass;
    Widget widget;

    dummyclass = NULL;

    if (parent_realized)
	WorkCursorDisplay();

    if (MrmFetchWidget(card_hierarchy, name, parent, &widget, &dummyclass) !=
      MrmSUCCESS)
	card_serror(NoCardWidget, FATAL);
    else {
	if (parent_realized)
	    WorkCursorRemove();
	return (widget);
    }

}

/*
**++
**  ROUTINE NAME: name_to_widget
**
**  FUNCTIONAL DESCRIPTION:
**	Gets a widget id from a reference widget and a name string.
**	Basically an interface on top of XtNameToWidget.
**
**  FORMAL PARAMETERS:
**	reference_widget - widget to start the search from.
**	name_string - list of widget names starting from the reference
**		widget down to the wanted widget.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
Widget name_to_widget(reference_widget, name_string)
    Widget reference_widget;
    char *name_string;
{
    Widget temp;

    if (reference_widget == NULL)
	card_serror(NoCardReference, FATAL);

    temp = XtNameToWidget(reference_widget, name_string);

    if (temp == NULL)
	card_serror(NoCardFetch, FATAL);
    else
	return (temp);
}

/*
**++
**  ROUTINE NAME: CardPushFillHighlight
**
**  FUNCTIONAL DESCRIPTION:
**	This routine causes the widget to become highlighted
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CardPushFillHighlight(w)
    Widget w;
{
#ifdef SPECIAL

    XmPushButtonWidget pb = (XmPushButtonWidget) w;
    /*  This routine handles both widgets and gadgets */
    if (XtIsSubclass(pb, xmPushButtonWidgetClass)) {
	if (!HiLite(pb)) {
	    HiLite(pb) = TRUE;
	    DrawMode(pb) |= HIGHLIGHT;
	    if (XtIsRealized(pb))
		(*xmLabelWidgetClass->core_class.expose)(pb, NULL, NULL);
	}
    } else {
	XmPushButtonGadget pg = (XmPushButtonGadget) w;
	Widget p = XtParent(w);
    }
#endif
}

/*
**++
**  ROUTINE NAME: CardPushFillUnHighlight
**
**  FUNCTIONAL DESCRIPTION:
**	This routine causes the widget to become unhighlighted
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CardPushFillUnHighlight(w)
    Widget w;
{
#ifdef SPECIAL

    XmPushButtonWidget pb = (XmPushButtonWidget) w;
    /*  This routine handles both widgets and gadgets */
    if (XtIsSubclass(pb, xmPushButtonWidgetClass)) {
	if (HiLite(pb)) {
	    DrawMode(pb) &= ~HIGHLIGHT;
	    HiLite(pb) = FALSE;
	    if (XtIsRealized(pb))
		(*xmLabelWidgetClass->core_class.expose)(pb, NULL, NULL);
	}
    } else {
	XmPushButtonGadget pg = (XmPushButtonGadget) w;
	Widget p = XtParent(w);
    }
#endif
}
