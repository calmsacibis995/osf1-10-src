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
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/cardfiler/cardmmx.c,v 1.1.4.2 1993/09/09 17:05:30 Susan_Ng Exp $";
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
**	cardmmx.c
**
**  FACILITY:
**      OOTB Cardfiler
**
**  ABSTRACT:
**	The HyperInformation support routines for CardFiler
**
**  AUTHORS:
**      Doug Rayner
**
**  RELEASER:
**
**  CREATION DATE:     27-DEC-1989
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/
#include "cardglobaldefs.h"
#include "cardexterndefs.h"
#include <DXm/DXmSvn.h>
#ifndef NO_MEMEX
#include <lwk_dxm_def.h>
#endif

extern int pending_link;
extern void displaycardbyid();
extern void display_from_link();

#ifndef NO_MEMEX
#define CheckSuccess(Message, Status, Fatal) \
    { \
        lwk_string string; \
        if (Status != lwk_s_success) { \
            lwk_status_to_string(Status, &string); \
            /* printf("?%s -- status = %s\n", Message, string); */ \
            lwk_delete_string(&string); \
            if (Fatal) \
                exit(0); \
        } \
    } \


/* HyperInformation Support */
typedef struct _NetworkList {
    struct _NetworkList *next;
    lwk_linknet network;
} NetworkList;

#define SurrogateType "DWCF"
#define CardFile "%Container"

static Widget ConnectionMenu;
static lwk_dxm_ui DwUi;
static SurrogateList *Surrogates;
static lwk_integer Highlighting;
static lwk_reason create_surr_reason;	/* Save the reason for a
					 * CreateSurrogate callback to be used 
					 * for the DoMenuAction call later.
					 */

static lwk_composite_linknet CurrentCNet =
  (lwk_composite_linknet) lwk_c_null_object;
static Boolean CurrentNetValid = FALSE;

static lwk_linknet CurrentNetwork = (lwk_linknet) lwk_c_null_object;
static Boolean CurrentCNetValid = FALSE;

static NetworkList *Networks = 0;

static XWMHints hints;			/* save Surrogate information    */
static lwk_string file;			/* when an Apply comes in case   */
static lwk_integer card_id;		/* the file is not saved or it   */
static lwk_status status;		/* is untitled in which case we  */
static lwk_persistent container;	/* confirm the apply first, then */

/* Selection Expression for lwk_query */
static lwk_query_node TypeLiteral = {lwk_c_string_literal,
  (lwk_any_pointer) SurrogateType, (lwk_any_pointer)0};
static lwk_query_node TypePropVal = {lwk_c_property_value,
  (lwk_any_pointer) lwk_c_p_surrogate_sub_type, (lwk_any_pointer)0};
static lwk_query_node FileLiteral = {lwk_c_string_literal,
  (lwk_any_pointer) global_fname, (lwk_any_pointer)0};
static lwk_query_node FilePropVal = {lwk_c_property_value,
  (lwk_any_pointer) CardFile, (lwk_any_pointer)0};
static lwk_query_node StrEql1 = {lwk_c_string_is_eql,
  (lwk_any_pointer) & TypePropVal, (lwk_any_pointer) & TypeLiteral};
static lwk_query_node StrEql2 = {lwk_c_string_is_eql,
  (lwk_any_pointer) & FilePropVal, (lwk_any_pointer) & FileLiteral};
static lwk_query_node And = {lwk_c_and, (lwk_any_pointer) & StrEql1,
  (lwk_any_pointer) & StrEql2};
static lwk_query_node HasProps = {lwk_c_has_properties,
  (lwk_any_pointer) & And, (lwk_any_pointer)0};

/* Forward routine Declarations */
static lwk_status GetSurrogate();
static Boolean CreateSurrogateList();
static lwk_status CreateSurrogate();
static lwk_status CloseView();
static lwk_status Apply();
static lwk_status Connect();
static void CheckEntries();
static lwk_status CurrencyChange();
static void UpdateCompositeNet();
static void QuerySurrogates();
static lwk_closure BuildNetworkList();
static lwk_closure BuildSurrogateList();

void CreateConnectionMenu(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    ConnectionMenu = widget;
    return;
}

void SetHighlighting(newcards)
    Boolean newcards;
{
    int entry_number;
    lwk_boolean highlighted;
    lwk_integer current;
    lwk_status status;
    card_pointer card;
    Boolean old;
    Boolean changed;
    SurrogateList *list;

    /* If there are new cards in the index, recalculate which Surrogates
     * describe which cards. */
    if (newcards) {
	SurrogateList *all;

	card = first;
	while (card != NULL) {
	    FreeSurrogates(card);
	    card = card->next;
	    if (card == first)
		break;
	}
	QuerySurrogates(&HasProps);
	all = Surrogates;
	while (all != NULL) {
	    CheckEntries(all);
	    all = all->next;
	}
    }

    /* Get the Current Highlighting */
    status = lwk_get_value(DwUi, lwk_c_p_appl_highlight, lwk_c_domain_integer,
      &current);
    CheckSuccess("GetValue failed", status, FALSE);
    if (status != lwk_s_success)
	return;

    /* If the highlighting has been toggled, remember that we need to
     * re-validate the svn entries. */
    if ((current & lwk_c_hl_on) == (Highlighting & lwk_c_hl_on))
	changed = FALSE;
    else
	changed = TRUE;
    Highlighting = current;

    /* Loop over all the entries and reset their highlight flags */
    card = first;
    while (card != NULL) {
	/* If Highlighting is on, see if this entry should be highlighted. */
	highlighted = lwk_c_false;
	if ((Highlighting & lwk_c_hl_on) != 0) {
	    list = card->surrogates;
	    while (list != 0) {
		status = lwk_surrogate_is_highlighted(DwUi, list->surrogate,
		  &highlighted);
		CheckSuccess("SurrogateIsHighlighted failed", status, FALSE);
		if (highlighted == lwk_c_true)
		    break;
		list = list->next;
	    }
	}
	old = card->highlighted;

	if (highlighted == lwk_c_true)
	    card->highlighted = TRUE;
	else
	    card->highlighted = FALSE;

	card->highlighted_changed = old != card->highlighted;
	changed = changed || card->highlighted_changed;
	card = card->next;
	if (card == first)
	    break;
    }

    /* If something changed, re-validate all the SVN entries */
    if (changed) {
	card = first;
	DXmSvnDisableDisplay(svnlist);
	while (card != last) {

	    /* Set the Pixmap components only if SvnGetEntryCallback           
	     * has been called. */
	    if (card->svngetentrycalled) {
		/* Get the SVN entry number to the card */
		if (card->highlighted_changed) {
		    entry_number = DXmSvnGetEntryNumber(svnlist, card);
		    DXmSvnSetComponentHidden(svnlist, entry_number, 1,
		      DXmSvnKdisplayNone);
		    DXmSvnInvalidateEntry(svnlist, entry_number);
		}
	    }
	    card = card->next;
	    if (card == first)
		break;
	}
	DXmSvnEnableDisplay(svnlist);
    }
}

void FreeSurrogates(card)
    card_pointer card;
{
    SurrogateList *list, *temp;

    list = card->surrogates;
    while (list != NULL) {
	temp = list;
	list = list->next;
	XtFree((char*) temp);
    }
    card->surrogates = NULL;
    return;
}

void CreateDwUi()
{
    lwk_status status;
    lwk_string string;
    lwk_string operation;
    lwk_callback callback;
    lwk_surrogate surrogate;
    lwk_boolean hyperinvoked;
    lwk_any_pointer name;
    long cs_status, byte_count;

    /* Initialize HIS */
    status = lwk_initialize(&hyperinvoked, &operation, &surrogate);
    CheckSuccess("HIS Initialize failed", status, TRUE);

    /* Create a DwUi */
#ifdef VMS
    name = (lwk_any_pointer) DXmCvtFCtoCS("DECW$CardFiler", &byte_count,
      &cs_status);
#else
    name = (lwk_any_pointer) DXmCvtFCtoCS("DXcardfiler", &byte_count,
      &cs_status);
#endif
    status = lwk_create_dxm_ui(name, lwk_c_true, lwk_c_true, indexparent,
      ConnectionMenu, &DwUi);
    XmStringFree(name);
    CheckSuccess("CreateDwUi failed", status, TRUE);

    /* Set the $SupportedSurrogateTypes property of the DwUi. */
    string = SurrogateType;
    status = lwk_set_value(DwUi, lwk_c_p_supported_surrogates,
      lwk_c_domain_string, &string, lwk_c_add_property);
    CheckSuccess("SetValue failed", status, FALSE);

    /* Set the $SupportedOperations property of the DwUi (View, Edit) */
    string = "View";
    status = lwk_set_value(DwUi, lwk_c_p_supported_operations,
      lwk_c_domain_string, &string, lwk_c_add_property);
    CheckSuccess("SetValue failed", status, FALSE);

    string = "Edit";
    status = lwk_set_value(DwUi, lwk_c_p_supported_operations,
      lwk_c_domain_string, &string, lwk_c_add_property);
    CheckSuccess("SetValue failed", status, FALSE);

    /* Set the callback properties of the DwUi */
    callback = (lwk_callback) GetSurrogate;
    status = lwk_set_value(DwUi, lwk_c_p_get_surrogate_cb,
      lwk_c_domain_routine, &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    callback = (lwk_callback) CreateSurrogate;
    status = lwk_set_value(DwUi, lwk_c_p_create_surrogate_cb,
      lwk_c_domain_routine, &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    callback = (lwk_callback) CloseView;
    status = lwk_set_value(DwUi, lwk_c_p_close_view_cb, lwk_c_domain_routine,
      &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    callback = (lwk_callback) Apply;
    status = lwk_set_value(DwUi, lwk_c_p_apply_cb, lwk_c_domain_routine,
      &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    callback = (lwk_callback) Connect;
    status = lwk_set_value(DwUi, lwk_c_p_complete_link_cb,
      lwk_c_domain_routine, &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    callback = (lwk_callback) CurrencyChange;
    status = lwk_set_value(DwUi, lwk_c_p_environment_change_cb,
      lwk_c_domain_routine, &callback, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    /* Set the $CurrentHighlighting property of the DwUi to be the
     * $DefaultHighlighting of the DwUi. */
    status = lwk_get_value(DwUi, lwk_c_p_default_highlight,
      lwk_c_domain_integer, &Highlighting);
    CheckSuccess("GetValue failed", status, FALSE);

    status = lwk_set_value(DwUi, lwk_c_p_appl_highlight, lwk_c_domain_integer,
      &Highlighting, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);

    if (hyperinvoked)
	Apply(DwUi, (lwk_reason)0, NULL, (lwk_closure)0, surrogate, operation,
	  lwk_c_follow_go_to);
    return;
}

void MMXexit()
{
    lwk_status status;

    /* Delete the DwUi Object used by each pseudo-application */
    status = lwk_delete(&DwUi);
    CheckSuccess("Delete failed", status, FALSE);
    return;
}

void RedoLink()
{
    lwk_status status;
    lwk_dxm_menu_action do_action;

    /* Check LinkWorks user operation saved and reissue the Link operation
     * again. */
    if (create_surr_reason == lwk_c_reason_start_link)
	do_action = lwk_c_dxm_menu_start_link;
    else
	do_action = lwk_c_dxm_menu_comp_link;

    status = lwk_do_dxm_menu_action(DwUi, do_action);
    CheckSuccess("Lwk Do Menu Action failed.", status, FALSE);
}

static lwk_status GetSurrogate(ui, reason, dxm_info, null, surrogates)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
    lwk_object *surrogates;
{
    int index_number;

    /* The GetSurrogate callback from DwUi -- ask VList for the selection,
     * and return the list of Surrgates pertaining to that entry */
    if (currentselection == NULL)
	*surrogates = (lwk_object) lwk_c_null_object;
    else if (currentselection->surrogates == (SurrogateList *) 0)
	*surrogates = (lwk_object) lwk_c_null_object;
    else if (currentselection->surrogates->next == (SurrogateList *) 0)
	*surrogates = currentselection->surrogates->surrogate;
    else if (!CreateSurrogateList(currentselection->surrogates, surrogates))
	return lwk_s_failure;

    return lwk_s_success;
}

static Boolean CreateSurrogateList(list, set)
    SurrogateList *list;
    lwk_set *set;
{
    lwk_status status;

    /* Create a Set object */
    status = lwk_create_set(lwk_c_domain_surrogate, 5, set);
    CheckSuccess("CreateSet failed", status, FALSE);

    if (status != lwk_s_success)
	return FALSE;

    while (list != (SurrogateList *) 0) {

	/* Add the Surrogates to the Set one at a time */
	status =
	  lwk_add_element(*set, lwk_c_domain_surrogate, &list->surrogate);
	CheckSuccess("AddElement failed", status, FALSE);
	if (status != lwk_s_success)
	    return FALSE;
	list = list->next;
    }
    return TRUE;
}

static lwk_status CreateSurrogate(ui, reason, dxm_info, null, surrogate)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
    lwk_surrogate *surrogate;
{
    char *tp;
    char desc[100];
    lwk_status status;
    lwk_ddif_string ddifstring;
    lwk_string file;
    lwk_integer card_id;
    lwk_linknet network;
    SurrogateList *list;
    XmString cstring;
    long byte_count, dxm_rstatus;
    long cs_status, cs_byte_count;

    /* The CreateSurrogate callback from DwUi -- return a known Surrogate
     * which pertains to selected entry (preferably one in the Current
     * Network), otherwise create a new Surrogate. */

    if (currentselection == NULL)
	return lwk_s_failure;

    if (!CurrentNetValid &&
      currentselection->surrogates != (SurrogateList *) 0) {
	status = lwk_get_value(ui, lwk_c_p_recording_linknet,
	  lwk_c_domain_linknet, &CurrentNetwork);
	CheckSuccess("GetValue failed", status, FALSE);
	CurrentNetValid = TRUE;
    }
    list = currentselection->surrogates;

    /* Look for a known Surrogate in the Current Network */
    while (list != (SurrogateList *) 0) {
	status = lwk_get_value(list->surrogate, lwk_c_p_container,
	  lwk_c_domain_persistent, &network);
	CheckSuccess("GetValue failed", status, FALSE);
	if (network == CurrentNetwork) {
	    *surrogate = list->surrogate;
	    return lwk_s_success;
	}
	list = list->next;
    }

    /* None in Current Network -- any other known Surrogate will do */
    if (currentselection->surrogates != (SurrogateList *) 0) {
	*surrogate = currentselection->surrogates->surrogate;
	return lwk_s_success;
    }

    /* No known Surrogate, so create one. */
    /* check if cards has a associated file name. If not, warning user to enter
     * one. */
    if (!has_file_name) {
	/* save the callback reason to be used latest to complete the user
	 * operation. */
	create_surr_reason = reason;
	if (enter_fname_dialog == NULL)
	    enter_fname_dialog = (XmMessageBoxWidget) fetch_widget(
	      "enter_fname_dialog", indexparent);
	XtManageChild((Widget) enter_fname_dialog);

	/* return with a null surrogate. */
	*surrogate = NULL;
	return lwk_s_success;
    }
    status = lwk_create(lwk_c_domain_surrogate, surrogate);
    CheckSuccess("Create failed", status, FALSE);
    if (status != lwk_s_success)
	return lwk_s_failure;

    tp = SurrogateType;
    status = lwk_set_value(*surrogate, lwk_c_p_surrogate_sub_type,
      lwk_c_domain_string, &tp, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);
    if (status != lwk_s_success)
	return lwk_s_failure;

    file = (lwk_string) global_fname;
    card_id = (lwk_integer) currentselection->card_id;
    sprintf(desc, "CardFile %s, Card_Id %x", file, card_id);
/*  cstring = (lwk_any_pointer) DXmCvtFCtoCS(desc, &cs_byte_count, &cs_status); */
    cstring = (XmString) DXmCvtFCtoCS(desc, &cs_byte_count, &cs_status);
    ddifstring = (Opaque) DXmCvtCStoDDIF(cstring, &byte_count, &dxm_rstatus);
    status = lwk_set_value(*surrogate, lwk_c_p_description,
      lwk_c_domain_ddif_string, &ddifstring, lwk_c_set_property);
    XmStringFree(cstring);
    CheckSuccess("SetValue failed", status, FALSE);
    if (status != lwk_s_success)
	return lwk_s_failure;

    status = lwk_set_value(*surrogate, CardFile, lwk_c_domain_string, &file,
      lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);
    if (status != lwk_s_success)
	return lwk_s_failure;

    status = lwk_set_value(*surrogate, "Card_id", lwk_c_domain_integer,
      &card_id, lwk_c_set_property);
    CheckSuccess("SetValue failed", status, FALSE);
    if (status != lwk_s_success)
	return lwk_s_failure;

    /* everything is fine, marked that something has been changed in this file.
     */
    file_changed = TRUE;
    return lwk_s_success;
}

static lwk_status CloseView(ui, reason, dxm_info, null)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
{
    XWMHints hints;
    XEvent event;
    Screen *scrn;

    /* Iconifying the cardwindow under MWM. Just do it first. */
    scrn = XtScreen(cardparent);
    event.xclient.type = ClientMessage;
    event.xclient.display = dpy;
    event.xclient.window = XtWindow(cardparent);
    event.xclient.message_type = XmInternAtom(dpy, "WM_CHANGE_STATE", False);
    event.xclient.format = 32;
    event.xclient.data.l[0] = IconicState;
    XSendEvent(dpy, RootWindowOfScreen(scrn), False,
      SubstructureRedirectMask | SubstructureNotifyMask, &event);

    scrn = XtScreen(indexparent);
    event.xclient.window = XtWindow(indexparent);
    XSendEvent(dpy, RootWindowOfScreen(scrn), False,
      SubstructureRedirectMask | SubstructureNotifyMask, &event);

    /* Iconify the windows under the XUI R3 Window Manager. */
    if (DXIsXUIWMRunning(cardparent, FALSE)) {
	hints.flags = StateHint;
	hints.initial_state = IconicState;
	XSetWMHints(XtDisplay(cardparent), XtWindow(cardparent), &hints);
	XSetWMHints(XtDisplay(indexparent), XtWindow(indexparent), &hints);
    }
    CardDeIconified = FALSE;
    IconState = FALSE;
    GrabFocus = FALSE;

    return lwk_s_success;
}

static lwk_status Apply(ui, reason, dxm_info, null, surrogate, operation,
  follow_type)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
    lwk_surrogate surrogate;
    lwk_string operation;
    lwk_integer follow_type;
{

    /* Given a Surrogate, find the Entry which pertains, and display that entry
     * Get the values of the properties of this Surrogate */
    status = lwk_get_value(surrogate, CardFile, lwk_c_domain_string, &file);
    if (status != lwk_s_success)
	file = (lwk_string) lwk_c_null_object;

    status =
      lwk_get_value(surrogate, "Card_id", lwk_c_domain_integer, &card_id);
    if (status != lwk_s_success)
	card_id = (lwk_integer) - 1;

    /* Confirm the Apply */
    status = lwk_confirm_apply(DwUi, surrogate);
    CheckSuccess("Apply Confirmation failed", status, FALSE);

    /* If the Surrogate was in no container, delete it */
    status = lwk_get_value(surrogate, lwk_c_p_container,
      lwk_c_domain_persistent, &container);
    CheckSuccess("Get Value failed", status, TRUE);
    if (container == (lwk_persistent) lwk_c_null_object) {
	status = lwk_delete(&surrogate);
	CheckSuccess("Delete failed", status, TRUE);
    }

    /* At this point we should be able to complete the request. We have to do
     * it now in case the previous file as not been saved or the card is
     * untitled, in either case, this involves using dialogs boxes, And
     * does not allow us to come back here. */

    /* If necessary, save the changes, then Open the proper Card File.
     * Only do it if the current file is not the same as "file" and changes
     * have been made to it. */

    if (card_displayed != NULL)
	check_and_save(card_displayed);

    if ((strcmp(file, global_fname) != 0) && (file_changed)) {
	pending_link = TRUE;
	if (open_caution == NULL)
	    open_caution =
	      (XmMessageBoxWidget) fetch_widget("open_caution", indexparent);
	XtManageChild((Widget) open_caution);
    } else
	display_from_link();

    return lwk_s_success;
}

void display_from_link()
{
    /* Make sure that the index window is visible */

    /* MWM R4 method of Deiconifying a window. Always do it. */
    XMapWindow(XtDisplay(indexparent), XtWindow(indexparent));

    /* XUI R3 way of Deiconifying window. */
    if (DXIsXUIWMRunning(indexparent, FALSE)) {
	hints.flags = StateHint;
	hints.initial_state = NormalState;
	XSetWMHints(XtDisplay(indexparent), XtWindow(indexparent), &hints);
    }

    /* Find the appropriate entry by id and position it in the display */
    retrieveaction(file);
    displaycardbyid((unsigned int) card_id);
}

static lwk_status Connect(ui, reason, dxm_info, null, connection)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
    lwk_link connection;

{
    lwk_status status;
    SurrogateList *list;
    lwk_surrogate surrogate;
    lwk_string surrogate_type;

    /* The reason we need to make these checks is because the Source and
     * Target Surrogates of the Connection may have been copied to the
     * Current Network after they were created. */

    /* See if the Source of the Connection is for one of this application's
     * Entries */
    status = lwk_get_value(connection, lwk_c_p_source, lwk_c_domain_surrogate,
      &surrogate);

    CheckSuccess("GetValue failed", status, FALSE);

    if (status == lwk_s_success &&
      surrogate != (lwk_surrogate) lwk_c_null_object) {
	list = Surrogates;

	/* Only check if we don't already know about this Surrogate */

	while (list != NULL) {
	    if (list->surrogate == surrogate)
		break;
	    list = list->next;
	}

	if (list == NULL) {
	    status = lwk_get_value(surrogate, lwk_c_p_surrogate_sub_type,
	      lwk_c_domain_string, &surrogate_type);
	    CheckSuccess("GetValue failed", status, FALSE);
	    if (status == lwk_s_success) {
		if (strcmp(SurrogateType, surrogate_type) == 0) {
		    list = (SurrogateList *) XtMalloc(sizeof(SurrogateList));
		    list->surrogate = surrogate;
		    list->next = Surrogates;
		    Surrogates = list;
		    CheckEntries(list);
		}
		status = lwk_delete_string(&surrogate_type);
		CheckSuccess("DeleteString failed", status, FALSE);
	    }
	}
    }

    /* See if the Target of the Connection is for one of this application's
     * Entries */
    status = lwk_get_value(connection, lwk_c_p_target, lwk_c_domain_surrogate,
      &surrogate);
    CheckSuccess("GetValue failed", status, FALSE);
    if (status == lwk_s_success &&
      surrogate != (lwk_surrogate) lwk_c_null_object) {
	list = Surrogates;

	/* Only check if we don't already know about this Surrogate */
	while (list != NULL) {
	    if (list->surrogate == surrogate)
		break;
	    list = list->next;
	}

	if (list == NULL) {
	    status = lwk_get_value(surrogate, lwk_c_p_surrogate_sub_type,
	      lwk_c_domain_string, &surrogate_type);
	    CheckSuccess("GetValue failed", status, FALSE);
	    if (status == lwk_s_success) {
		if (strcmp(SurrogateType, surrogate_type) == 0) {
		    list = (SurrogateList *) XtMalloc(sizeof(SurrogateList));
		    list->surrogate = surrogate;
		    list->next = Surrogates;
		    Surrogates = list;
		    CheckEntries(list);
		}
		status = lwk_delete_string(&surrogate_type);
		CheckSuccess("DeleteString failed", status, FALSE);
	    }
	}
    }
    return lwk_s_success;
}

static void CheckEntries(list)
    SurrogateList *list;
{
    card_pointer card;
    lwk_string file;
    lwk_integer card_id;
    lwk_status status;
    SurrogateList *new;

    /* Get the values of the properties of this Surrogate */
    status =
      lwk_get_value(list->surrogate, CardFile, lwk_c_domain_string, &file);

    if (status != lwk_s_success)
	file = (lwk_string) lwk_c_null_object;

    status = lwk_get_value(list->surrogate, "Card_id", lwk_c_domain_integer,
      &card_id);

    if (status != lwk_s_success)
	card_id = (lwk_integer) - 1;

    /* If there is a card_id but the id is greater than the 
     * "highest_card_id_used" this must be connection made to a new card 
     * but that card was not save in a file.  So, this ID should not be used 
     * again. */
    if ((card_id > highest_card_id_used) && (card_id != -1))
	highest_card_id_used = card_id;

    /* Loop over all the Cards to regenerate Surrogate lists */
    card = first;
    while (card != NULL) {
	if (file != (lwk_string) lwk_c_null_object &&
	  strcmp(global_fname, file) == 0 && card_id != (lwk_integer) - 1 &&
	  (card->card_id == card_id)) {
	    new = (SurrogateList *) XtMalloc(sizeof(SurrogateList));
	    new->surrogate = list->surrogate;
	    new->next = card->surrogates;
	    card->surrogates = new;
	}
	card = card->next;
	if (card == first)
	    break;
    }

    /* Don't forget to delete the property values returned by GetValue */
    status = lwk_delete_string(&file);
    CheckSuccess("DeleteString failed", status, FALSE);
    return;
}

static lwk_status CurrencyChange(ui, reason, dxm_info, null, currency)
    lwk_ui ui;
    lwk_reason reason;
    XmAnyCallbackStruct *dxm_info;
    lwk_closure null;
    lwk_integer currency;
{

    /* The CurrencyChange callback from DwUi -- see which currency changed
     * and take the appropriate action. */
    switch (currency) {
	case lwk_c_env_pending_source: 
	case lwk_c_env_pending_target: 
	case lwk_c_env_follow_destination: 
	case lwk_c_env_appl_highlight: 

	    /* SetHighlighting(FALSE);*/ /* tin change */
	    SetHighlighting(TRUE);
	    break;

	case lwk_c_env_active_comp_linknet: 
	    UpdateCompositeNet();
	    break;

	case lwk_c_env_recording_linknet: 
	    CurrentNetValid = FALSE;
	    break;

	default: 
	    break;
    }
    return lwk_s_success;
}

static void UpdateCompositeNet()
{
    /* The Current Composite Network is no longer valid */
    CurrentCNetValid = FALSE;

    /* Update Highlighting if necessary */
    SetHighlighting(TRUE);

    return;
}

static void QuerySurrogates(query)
    lwk_query_expression query;
{
    lwk_status status;
    lwk_termination termination;
    lwk_composite_linknet cnet;
    NetworkList *old, *new, *prev;
    SurrogateList *list, *temp;
    card_pointer card;

    /* Query the current Composite Network for Surrogates of a given
     * subtype, and return a list of those which are found. Get the
     * Current Composite Network, the list of Networks which are contained
     * in the Current Composite Network (so we can delete them if required),
     * and the relevant Surrogates which are contained in the Current
     * Composite Network (so we can do highlighting, and respond to HIS
     * callbacks). */

    if (!CurrentCNetValid) {

	/* Delete and clear any old Networks */
	old = Networks;
	Networks = (NetworkList *) 0;

	while (old != (NetworkList *) 0) {
	    status = lwk_delete(&old->network);
	    CheckSuccess("Delete failed", status, FALSE);
	    prev = old;
	    old = old->next;
	    XtFree((char*) prev);
	}

	/* Delete any old Composite Network, and get the new one. */
	if (CurrentCNet != (lwk_composite_linknet) lwk_c_null_object) {
	    status = lwk_delete(&CurrentCNet);
	    CheckSuccess("Delete failed", status, FALSE);
	}

	status = lwk_get_value(DwUi, lwk_c_p_active_comp_linknet,
	  lwk_c_domain_comp_linknet, &CurrentCNet);
	CheckSuccess("GetValue failed", status, FALSE);
	if (status == lwk_s_success &&
	  CurrentCNet != (lwk_composite_linknet) lwk_c_null_object) {

	    /* Generate a new Networks list. */
	    status = lwk_iterate(CurrentCNet, lwk_c_domain_linknet,
	      (lwk_closure)0, BuildNetworkList, &termination);
	    CheckSuccess("Iterate failed", status, FALSE);
	}

	/* We've now updated the Current Composite Network */
	CurrentCNetValid = TRUE;
    }

    /* Free the old Surrogate list */
    list = Surrogates;
    while (list != (SurrogateList *) 0) {
	temp = list;
	list = list->next;
	XtFree((char*)temp);
    }
    Surrogates = (SurrogateList *) 0;

    /* Get the new list of requested Surrogates */
    if (CurrentCNet != (lwk_composite_linknet) lwk_c_null_object) {
	status = lwk_query(CurrentCNet, lwk_c_domain_surrogate, query,
	  (lwk_closure)0, BuildSurrogateList, &termination);
	CheckSuccess("Query failed", status, FALSE);
    }
    return;
}

static lwk_termination BuildNetworkList(null, cnet, domain, network)
    lwk_closure null;
    lwk_composite_linknet cnet;
    lwk_integer domain;
    lwk_linknet *network;
{
    NetworkList *list;

    /* Check for redundant Networks */
    list = Networks;
    while (list != (NetworkList *) 0) {
	if (list->network == *network)
	    break;
	list = list->next;
    }

    /* If we haven't seen it yet, add it to the list */
    if (list == (NetworkList *) 0) {
	list = (NetworkList *) XtMalloc(sizeof(NetworkList));
	list->network = *network;
	list->next = Networks;
	Networks = list;
    }
    return (lwk_termination)0;
}

static lwk_termination BuildSurrogateList(null, network, domain, surrogate)
    lwk_closure null;
    lwk_linknet network;
    lwk_integer domain;
    lwk_surrogate *surrogate;
{
    SurrogateList *list;

    /* Add this Surrogate to the list */
    list = (SurrogateList *) XtMalloc(sizeof(SurrogateList));
    list->surrogate = *surrogate;
    list->next = Surrogates;
    Surrogates = list;
    return (lwk_termination)0;
}
#else
void CreateConnectionMenu(widget, tag, reason)
    Widget widget;
    XtPointer *tag;
    XmAnyCallbackStruct *reason;
{
    /* unpost linkworks menu */
    XtDestroyWidget(widget);
}

void CreateDwUi()
{
}

void display_from_link()
{
}

void FreeSurrogates(card)
    card_pointer card;
{
}

void MMXexit()
{
}

void RedoLink()
{
}

void SetHighlighting(newcards)
    Boolean newcards;
{
}
#endif
