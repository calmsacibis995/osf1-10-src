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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: XSelection.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:08:59 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifdef SEC_BASE

/*
	filename:
		XSelection.c
	   
	copyright:
		Copyright (c) 1989-1990 SecureWare Inc.
		ALL RIGHTS RESERVED

	function:
		X windows front end for Report Selection File options
		
	entry points:
		SelectionStart()
		SelectionOpen()
		SelectionClose()
		SelectionStop()

*/

#include "XAudit.h"
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>

#define CREATE 1
#define UPDATE 2
#define DELETE 3

extern  void    SelectionFileClose();

extern  AUDIT_SELECTION_STRUCT  selection_fillin;
extern  int     selection_action;
extern  char    *selection_filename,
		**msg_selection,    
		*msg_selection_text;

#if SEC_MAC
#define SUBJECT_MIN_SL	1
#define SUBJECT_MAX_SL	2
#define OBJECT_MIN_SL	3
#define OBJECT_MAX_SL	4
extern 	void
		AudSelnssOpen(),
		AudSelxssOpen(),
		AudSelnosOpen(),
		AudSelxosOpen();

extern	Widget
		CreateHeader(),
		CreatePushButton();
#endif /* SEC_MAC */

void    SelectionClose();

static int 
	group_count,
	object_count,
	selected_file,
	selection_active,
	selection_open,
	user_count;
		
static char 
	**msg_audit,
	*msg_audit_text,
	**group_names,
	**user_names;

static Widget
	etime_widget[5],
	groups_list_widget,
	head_label_widget,
	inner_form_widget,
	*objects_list,
	objects_scrolled_widget,
	form_widget,
	stime_widget[5],
	toggle_no_widget[AUDIT_MAX_EVENT],
	toggle_yes_widget[AUDIT_MAX_EVENT],
#if SEC_MAC
	object_label_widget,
	subject_label_widget,
	toggle_sub_min_sl,
	toggle_sub_max_sl,
	toggle_obj_min_sl,
	toggle_obj_max_sl,
#endif /* SEC_MAC */
	users_list_widget;
		
static XmString 
	*group_xmstrings,
	*user_xmstrings,
	xmstring;
		
static Display  
	*display;

static void
	MakeWidgets(), 
	CancelCallback(),
	SelectionObjectsWindow(),
#if SEC_MAC
	SLToggleCallback(),
#endif /* SEC_MAC */
		OKCallback();

static int
		GetSelectionFiles();
		
void 
SelectionStart()
{
	selection_active = FALSE;
	selection_open = FALSE;
}

void 
SelectionOpen() 
{
	int         i,
			j;
	char        buf[255];
	Cardinal    n;
	Arg         args[20];
	Widget      w;            
	

	WorkingOpen(main_shell);

	object_count = 0;
	
	/* Load in all users and groups on the system */
	GetAllUsers(&user_count, &user_names);
	GetAllGroups(&group_count, &group_names);

	/* Malloc space for pointers to list xmstrings and create list items */
	user_xmstrings = (XmString *) Malloc(sizeof(XmString) * user_count);
	if (! user_xmstrings)
		MemoryError();
	for (i = 0; i < user_count; i++)
		user_xmstrings[i] = XmStringCreate(user_names[i], charset);

	group_xmstrings = (XmString *) Malloc(sizeof(XmString) * group_count);
	if (! group_xmstrings)
		MemoryError();
	for (i = 0; i < group_count; i++)
		group_xmstrings[i] = XmStringCreate(group_names[i], charset);

	if (selection_open) {
		
		/* set up the title of the second window */
		strcpy(buf, msg_selection[29+selection_action]);
		strcat(buf, msg_selection[33]);
		strcat(buf, selection_filename);
		xmstring = XmStringCreate(buf, charset);
		if (! xmstring)
			MemoryError();

		n = 0;
		XtSetArg(args[n], XmNlabelString,                  xmstring); n++;
		XtSetValues(head_label_widget, args, n);
		XmStringFree(xmstring);                          

		for (i= 0; i < AUDIT_MAX_EVENT; i++) {
			XmToggleButtonSetState(toggle_yes_widget[i], FALSE, FALSE); 
			XmToggleButtonSetState(toggle_no_widget[i], TRUE, FALSE); 
		}
		
		/* rebuild the user list widget */
		n = 0;
		XtSetArg(args[n], XmNitems,                    user_xmstrings); n++;
		XtSetArg(args[n], XmNitemCount,                    user_count); n++;
		XtSetValues(users_list_widget, args, n);
		XmListDeselectAllItems(users_list_widget);
	
		/* rebuild the group list widget */
		n = 0;
		XtSetArg(args[n], XmNitems,                   group_xmstrings); n++;
		XtSetArg(args[n], XmNitemCount,                   group_count); n++;
		XtSetValues(groups_list_widget, args, n);
		XmListDeselectAllItems(groups_list_widget);
	
		/* Build editable list of objects attached to inner_form_widget */
		SelectionObjectsWindow();
	
	}
	else {
		display = XtDisplay(main_shell);
		MakeWidgets();
		selection_open = TRUE;      /* selection widgets exist */
	}
	
	/* set time values per report selection file */
	XtSetArg(args[0], XmNvalue,          selection_fillin.s_day);
	XtSetValues(stime_widget[0], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.s_month);
	XtSetValues(stime_widget[1], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.s_year);
	XtSetValues(stime_widget[2], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.s_hour);
	XtSetValues(stime_widget[3], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.s_min);
	XtSetValues(stime_widget[4], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.e_day);
	XtSetValues(etime_widget[0], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.e_month);
	XtSetValues(etime_widget[1], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.e_year);
	XtSetValues(etime_widget[2], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.e_hour);
	XtSetValues(etime_widget[3], args, 1);
	XtSetArg(args[0], XmNvalue,          selection_fillin.e_min);
	XtSetValues(etime_widget[4], args, 1);
		
	if (selection_action == UPDATE) {
	
		/* set audit event toggles per report selection file */
		for (i = 0; i < AUDIT_MAX_EVENT; i++) {
			if (selection_fillin.events[i][0] == YESCHAR) {
				XmToggleButtonSetState(toggle_yes_widget[i], True, False); 
				XmToggleButtonSetState(toggle_no_widget[i],False, False); 
			}
			else {
				XmToggleButtonSetState(toggle_yes_widget[i], False, False);
				XmToggleButtonSetState(toggle_no_widget[i], True, False);
			}
		}

		/* Select user/group list items per report selection file */
		for(i = 0; i < selection_fillin.nusers; i++) {
			if (strlen(selection_fillin.users[i])) {
				for (j = 0; j < user_count; j++) {
					if (! strcmp (selection_fillin.users[i],
								                            user_names[j])) {
						XmListSelectItem(users_list_widget, 
								                    user_xmstrings[j], False);
						break;
					}
				}    
			}    
		}
		for (i = 0; i < selection_fillin.ngroups; i++) {
			if (strlen(selection_fillin.groups[i])) {
				for (j = 0; j < group_count; j++) {
					if (! strcmp (selection_fillin.groups[i], group_names[j])) {
						XmListSelectItem(groups_list_widget, 
								                   group_xmstrings[j], False);
						break;
					}
				}    
			}    
		}
		
		/* free up data structure */
		SelectionFreeTables(&selection_fillin);
	}    
	XSync(display, 0);

	CenterForm(form_widget);
	selection_active = TRUE;        /* selection widgets are managed */
	WorkingClose();
}    
	
static void 
OKCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	int         i, 
				j,
				touched;
	char        *object;
	Cardinal    count,
				n;
	Arg         args[20];
	XmString    *selected_xmstring;

	XSync(display, 0);

	if (selection_action == UPDATE) 
		SelectionUpdateFill(&selection_fillin);
		
	/* get which audit events have been selected */            
	for (i = 0; i < AUDIT_MAX_EVENT; i++) { 
		if (XmToggleButtonGetState(toggle_yes_widget[i]))
			selection_fillin.events[i][0] = YESCHAR;
		else
			selection_fillin.events[i][0] = NOCHAR;
	}        

	/* get names of selected users, if any */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,           &selected_xmstring); n++;
	XtSetArg(args[n], XmNselectedItemCount,                   &count); n++;
	XtGetValues(users_list_widget, args, n);
	for (j = 0; j < count; j++) {
		for (i = 0; i < user_count; i++)
			if (XmStringCompare(user_xmstrings[i], selected_xmstring[j])) 
				break;
		strcpy(selection_fillin.users[j], user_names[i]); 
	}    
	selection_fillin.nusers = count;
	strcpy(selection_fillin.users[count], "\0");

	/* get names of selected groups, if any */
	n = 0;
	XtSetArg(args[n], XmNselectedItems,           &selected_xmstring); n++;
	XtSetArg(args[n], XmNselectedItemCount,                   &count); n++;
	XtGetValues(groups_list_widget, args, n);
	for (j = 0; j < count; j++) {
		for (i = 0; i < group_count; i++)
			if (XmStringCompare(group_xmstrings[i], selected_xmstring[j])) 
				break;
		strcpy(selection_fillin.groups[j], group_names[i]); 
	}    
	selection_fillin.ngroups = count;
	strcpy(selection_fillin.groups[count], "\0");
 
	/* get names of objects if any */           
	for (i = j = 0; i < object_count; i++) {
		object = XmTextGetString(objects_list[i]);
		if (*object) {
			strcpy(selection_fillin.files[j], object); 
			j++;
			XtFree(object);
		}    
	}  
	selection_fillin.nfiles = j;
	strcpy(selection_fillin.files[j], "\0");

	/* pick up times entered, if any */
	object = XmTextGetString(stime_widget[0]);
	strcpy(selection_fillin.s_day, object);
	XtFree(object);
	object = XmTextGetString(stime_widget[1]);
	touched = FALSE;
	if (object[0] && islower(object[0])) {
		object[0] = toupper(object[0]);
		touched = TRUE;
	}
	if (object[1] && isupper(object[1])) {
		object[1] = tolower(object[1]);
		touched = TRUE;
	}
	if (object[2] && isupper(object[2])) {
		object[2] = tolower(object[2]);
		touched = TRUE;
	}
	if (touched)
		XmTextSetString(stime_widget[1], object);
	strcpy(selection_fillin.s_month, object);
	XtFree(object);
	object = XmTextGetString(stime_widget[2]);
	strcpy(selection_fillin.s_year, object);
	XtFree(object);
	object = XmTextGetString(stime_widget[3]);
	strcpy(selection_fillin.s_hour, object);
	XtFree(object);
	object = XmTextGetString(stime_widget[4]);
	strcpy(selection_fillin.s_min, object);
	XtFree(object);
	object = XmTextGetString(etime_widget[0]);
	strcpy(selection_fillin.e_day, object);
	XtFree(object);
	object = XmTextGetString(etime_widget[1]);
	touched = FALSE;
	if (object[0] && islower(object[0])) {
		object[0] = toupper(object[0]);
		touched = TRUE;
	}
	if (object[1] && isupper(object[1])) {
		object[1] = tolower(object[1]);
		touched = TRUE;
	}
	if (object[2] && isupper(object[2])) {
		object[2] = tolower(object[2]);
		touched = TRUE;
	}
	if (touched)
		XmTextSetString(etime_widget[1], object);
	strcpy(selection_fillin.e_month, object);
	XtFree(object);
	object = XmTextGetString(etime_widget[2]);
	strcpy(selection_fillin.e_year, object);
	XtFree(object);
	object = XmTextGetString(etime_widget[3]);
	strcpy(selection_fillin.e_hour, object);
	XtFree(object);
	object = XmTextGetString(etime_widget[4]);
	strcpy(selection_fillin.e_min, object);
	XtFree(object);

	XSync(display, 0);

	/* write it all out to the selection file */
	if (! SelectionWriteFile(&selection_fillin)) {
		SelectionClose();    
		SelectionFileReset();
	}    
}    

void 
SelectionClose() 
{
	int         i;
				
	WorkingOpen(main_shell);

	XSync(display, 0);

	/* objects_list has to be rebuilt every time through */
	if (object_count) {
		for (i = 0; i < object_count; i++)
			XtDestroyWidget(objects_list[i]);
		object_count = 0;
		free(objects_list);
		objects_list = NULL;
	}    
	if (user_names) { 
		free_cw_table(user_names);
		user_names = NULL;
	}
	if (group_names) { 
		free_cw_table(group_names);
		group_names = NULL;
	}
	if (user_xmstrings) {
		for (i = 0; i < user_count; i++)
			XmStringFree(user_xmstrings[i]);
		free(user_xmstrings);
		user_xmstrings = NULL;
	}
	if (group_xmstrings) {
		for (i = 0; i < group_count; i++)
			XmStringFree(group_xmstrings[i]);
		free(group_xmstrings);
		group_xmstrings = NULL;
	}
	
	user_count = 0;
	group_count = 0;
	
	if (selection_active) {    
		XtUnmanageChild(form_widget);
		selection_active = FALSE;
	}    
	if (save_memory) {
		if (selection_open) {
			XtDestroyWidget(form_widget);
			selection_open = FALSE;
		}
	}
	SelectionFreeTables(&selection_fillin);
	XSync(display, 0);
	WorkingClose();
}

void 
SelectionStop() 
{
	if (selection_open)
		XtDestroyWidget(form_widget);
}

static void 
CancelCallback(w, ptr, info) 
	Widget                  w; 
	char                    *ptr;
	XmListCallbackStruct    *info;
{
	SelectionClose();
	SelectionFileReset();
}

static void 
MakeWidgets() 
{
	Arg         args[20];
	Cardinal    n;
	int         i,
				j;
	XmString    xmno,    
				xmstring,
				xmyes;
	Widget      action_rowcol_widget,
				event_label[AUDIT_MAX_EVENT],
				events_scrolled_widget,
		cancel_button,
		help_button,
		ok_button,
				groups_scrolled_widget,
				radio_widget[AUDIT_MAX_EVENT],
				tallest_widget,
				users_scrolled_widget,
				w,
				widest_widget,
				widg,
				work_area_widget;
	Dimension   item_height,
				item_width,
				total_height,
				total_height2,
				total_width;
	char        buf[50],
				item_name[30];
	XtWidgetGeometry    intended,
						preferred;
	XtGeometryResult    geom_result;                    
	 
	/***********************************************************************/
	/* Form widget                                                         */
	/***********************************************************************/
	XSync(display, 0);

	n = 0;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNallowOverlap,                     FALSE); n++;
	XtSetArg(args[n], XmNautoUnmanage,                     FALSE); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       TRUE); n++;
#endif
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
	form_widget = XtCreateWidget("SelectionFile",
		xmFormWidgetClass, mother_form, args, n);
	XtAddCallback(form_widget, XmNhelpCallback,
			HelpDisplayOpen, "audit,Selection");

	/***********************************************************************/
	/* Heading label                                                       */
	/***********************************************************************/
	strcpy(buf,msg_selection[selection_action+29]);
	strcat(buf, msg_selection[33]); 
	strcat(buf, selection_filename);
	xmstring =  XmStringCreate(buf, charset);
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
	XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
	head_label_widget = XmCreateLabelGadget(form_widget, 
			"Title", args, n);
	XtManageChild(head_label_widget);
	XmStringFree(xmstring);                          
	
	/* Start time label */
	xmstring = XmStringCreate(msg_selection[47], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                   head_label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNborderWidth,                                 0); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "StartTimeLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	/* Build start time edit fields */
	for (i = 0; i < 5; i++) {
		n = 0;
		XtSetArg(args[n], XmNcolumns,                 (i == 1 ? 3 : 2)); n++;
		XtSetArg(args[n], XmNmaxLength,               (i == 1 ? 3 : 2)); n++;
		XtSetArg(args[n], XmNeditMode,              XmSINGLE_LINE_EDIT); n++;
		XtSetArg(args[n], XmNeditable,                            TRUE); n++;
		XtSetArg(args[n], XmNtraversalOn,                         TRUE); n++;
		XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,              head_label_widget); n++;
		XtSetArg(args[n], XmNleftAttachment,           XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftOffset,                             0); n++;
		if (i == 0) {
			XtSetArg(args[n], XmNleftWidget,                         w); n++;
		} else {
			XtSetArg(args[n], XmNleftWidget,         stime_widget[i-1]); n++;
		}
		stime_widget[i] = XmCreateText(form_widget,
				"TimeEdit", args, n);
		XmAddTabGroup(stime_widget[i]);
		XtManageChild(stime_widget[i]);
		XtAddCallback(stime_widget[i], XmNhelpCallback,
			HelpDisplayOpen, "audit,Selection");
	
		/* build "dd mmm yy hh mm" labels under time text widgets */
		xmstring = XmStringCreate(msg_selection[i+42], charset);
		n = 0;
		XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
		XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,                 stime_widget[i]); n++;
		XtSetArg(args[n], XmNtopOffset,                               0); n++;
		XtSetArg(args[n], XmNborderWidth,                             0); n++;
		XtSetArg(args[n], XmNleftAttachment,            XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
		XtSetArg(args[n], XmNleftOffset,                              3); n++;
		if (i == 0) {
			XtSetArg(args[n], XmNleftWidget,                          w); n++;
		} else {
			XtSetArg(args[n], XmNleftWidget,          stime_widget[i-1]); n++;
		}
		widg = XmCreateLabelGadget(form_widget, "TimeLabel", 
			args, n);
		XtManageChild(widg);
		XmStringFree(xmstring);
	}
	
	/* End time label */
	xmstring = XmStringCreate(msg_selection[48], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                   head_label_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                    stime_widget[4]); n++;
	XtSetArg(args[n], XmNborderWidth,                                 0); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "EndTimeLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	for (i = 0; i < 5; i++) {
		n = 0;
		XtSetArg(args[n], XmNcolumns,                 (i == 1 ? 3 : 2)); n++;
		XtSetArg(args[n], XmNmaxLength,               (i == 1 ? 3 : 2)); n++;
		XtSetArg(args[n], XmNvalue,                               "  "); n++;
		XtSetArg(args[n], XmNeditMode,              XmSINGLE_LINE_EDIT); n++;
		XtSetArg(args[n], XmNeditable,                            TRUE); n++;
		XtSetArg(args[n], XmNtraversalOn,                         TRUE); n++;
		XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,              head_label_widget); n++;
		XtSetArg(args[n], XmNleftAttachment,           XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftOffset,                             0); n++;
		if (i == 0) { 
			XtSetArg(args[n], XmNleftWidget,                         w); n++;
		} else { 
			XtSetArg(args[n], XmNleftWidget,         etime_widget[i-1]); n++;
		}
		etime_widget[i] = XmCreateText(form_widget,
				"TimeEdit", args, n);
		XmAddTabGroup(etime_widget[i]);
		XtManageChild(etime_widget[i]);
	
		/* build "dd mmm yy hh mm" labels under time text widgets */
		xmstring = XmStringCreate(msg_selection[i+42], charset);
		if (! xmstring)
			MemoryError();
		n = 0;
		XtSetArg(args[n], XmNlabelString,                      xmstring); n++;
		XtSetArg(args[n], XmNtopAttachment,             XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget,                 etime_widget[i]); n++;
		XtSetArg(args[n], XmNtopOffset,                               0); n++;
		XtSetArg(args[n], XmNborderWidth,                             0); n++;
		XtSetArg(args[n], XmNleftAttachment,            XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNalignment,              XmALIGNMENT_CENTER); n++;
		XtSetArg(args[n], XmNleftOffset,                              3); n++;
		if (i == 0) {
			XtSetArg(args[n], XmNleftWidget,                          w); n++;
		} else {
			XtSetArg(args[n], XmNleftWidget,          etime_widget[i-1]); n++;
		}
		widg = XmCreateLabelGadget(form_widget, "TimeLabel", 
			args, n);
		XtManageChild(widg);
		XmStringFree(xmstring);
	}
	
	/***********************************************************************/
	/* Scrolled window widget to hold audit events information             */
	/***********************************************************************/
	xmstring = XmStringCreate(msg_selection[49], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString,                          xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,                 XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                widg); n++;
	XtSetArg(args[n], XmNleftAttachment,                  XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment,                  XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "EventsLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
	
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                              w); n++;
	XtSetArg(args[n], XmNleftAttachment,             XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNscrollingPolicy,              XmAUTOMATIC); n++;
	XtSetArg(args[n], XmNvisualPolicy,                  XmCONSTANT); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,       XmAS_NEEDED); n++;
	XtSetArg(args[n], XmNshadowThickness,                        0); n++;
	events_scrolled_widget = XtCreateWidget("Events", 
			xmScrolledWindowWidgetClass, form_widget, args, n);
	
	/***********************************************************************/
	/* Bulletin board that is placed inside the scrolled window above      */
	/* and that holds the audit event descriptions and radio 'on-off'      */
	/* boxes for each event.                                               */
	/***********************************************************************/
								                               
	n = 0;
	XtSetArg(args[n], XmNresizePolicy,             XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                True); n++;
	XtSetArg(args[n], XmNdialogStyle,        XmDIALOG_WORK_AREA); n++;
	work_area_widget = XmCreateForm(events_scrolled_widget, "Form", args, n);
	XtManageChild(work_area_widget);              
	
	/***********************************************************************/
	/* Do each audit event description and radio box with y/n toggles      */
	/***********************************************************************/
	total_width = 0;
	xmyes = XmStringCreate(msg_selection[50], charset);
	xmno = XmStringCreate(msg_selection[51], charset);
	if (! msg_audit)
	LoadMessage("msg_audit_events", &msg_audit, &msg_audit_text);

	for (i = 0; i < AUDIT_MAX_EVENT; i++) {
	
		/* Audit Event Description in a label widget */
		xmstring =  XmStringCreate(msg_audit[i], charset);
		n = 0;
		if (i == 0) { 
			 XtSetArg(args[n], XmNtopAttachment,       XmATTACH_FORM); n++;
		} else {
			 XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
			 XtSetArg(args[n], XmNtopWidget,       radio_widget[i-1]); n++;
		}
		XtSetArg(args[n], XmNleftAttachment,           XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNalignment,        XmALIGNMENT_BEGINNING); n++;
		XtSetArg(args[n], XmNlabelString,                   xmstring); n++;
		XtSetArg(args[n], XmNrecomputeSize,                    False); n++;
		event_label[i] = XmCreateLabelGadget(
					work_area_widget, "EventLabel", args, n);
		XtManageChild(event_label[i]);
		XmStringFree(xmstring);
		
		/* Catch width of widest audit event label widget */
		n = 0;
		XtSetArg(args[n], XmNwidth, &item_width); n++;
		XtGetValues(event_label[i], args, n);
		if (item_width > total_width) 
			total_width = item_width;
	
		/* Radio box which will contain yes and no pushbutton widgets */
		n = 0;
		if (i==0) {
			 XtSetArg(args[n], XmNtopAttachment,       XmATTACH_FORM); n++;
		}
		else {
			 XtSetArg(args[n], XmNtopAttachment,     XmATTACH_WIDGET); n++;
			 XtSetArg(args[n], XmNtopWidget,       radio_widget[i-1]); n++;
		}
		XtSetArg(args[n], XmNleftAttachment,         XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNleftWidget,              event_label[i]);n++;
		XtSetArg(args[n], XmNorientation,               XmHORIZONTAL); n++;
		XtSetArg(args[n], XmNentryClass,   xmToggleButtonWidgetClass); n++;
		XtSetArg(args[n], XmNradioAlwaysOne,                    TRUE); n++;
#ifdef TRAVERSAL
		XtSetArg(args[n], XmNtraversalOn,                       TRUE); n++;
#endif
		radio_widget[i] = XmCreateRadioBox(
						   work_area_widget, "RadioBox", args, n);
		XtManageChild(radio_widget[i]);
#ifdef TRAVERSAL
		XmAddTabGroup(radio_widget[i]);
#endif
	
		n = 0;
		XtSetArg(args[n], XmNlabelString,                      xmyes); n++; 
		XtSetArg(args[n], XmNset,                              FALSE); n++;
		toggle_yes_widget[i] = XmCreateToggleButton(
							radio_widget[i], "EventLabel", args, n);
		XtManageChild(toggle_yes_widget[i]);

		n = 0;
		XtSetArg(args[n], XmNlabelString,                       xmno); n++; 
		XtSetArg(args[n], XmNset,                               TRUE); n++;
		toggle_no_widget[i] = XmCreateToggleButton(
							radio_widget[i],"EventLabel", args, n);
		XtManageChild(toggle_no_widget[i]);
	}
	XmStringFree(xmyes);
	XmStringFree(xmno);
	
	/* Make all labels as wide as the widest label */
	for (i = 0; i < AUDIT_MAX_EVENT; i++) {
		n = 0;
		XtSetArg(args[n], XmNwidth, total_width); n++;
		XtSetValues(event_label[i], args, n);
	}
	
	/* Kill the horizontal scrollbar */
	n = 0;
	XtSetArg(args[n], XmNhorizontalScrollBar,                     &w); n++;
	XtGetValues(events_scrolled_widget, args, n);
	XtUnmanageChild(w);
	XtDestroyWidget(w);

	XtManageChild(events_scrolled_widget);

	XSync(display,0);

	/***********************************************************************/
	/* Scrolled window widgets to hold users and groups                    */
	/***********************************************************************/
	xmstring =  XmStringCreate(msg_selection[52], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                  xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,         XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftAttachment,          XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopWidget,      events_scrolled_widget); n++;
	XtSetArg(args[n], XmNalignment,          XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "UsersLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
		
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNtopOffset,                                 0); n++;
	XtSetArg(args[n], XmNleftAttachment,                XmATTACH_FORM); n++; 
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	users_scrolled_widget = XtCreateManagedWidget(
			"Users", xmScrolledWindowWidgetClass, 
			form_widget, args, n);

	n = 0;
	XtSetArg(args[n], XmNitems,                        user_xmstrings); n++;
	XtSetArg(args[n], XmNsensitive,                              TRUE); n++;
	XtSetArg(args[n], XmNitemCount,                        user_count); n++;
	XtSetArg(args[n], XmNselectionPolicy,           XmMULTIPLE_SELECT); n++; 
	XtSetArg(args[n], XmNlistSizePolicy,                   XmCONSTANT); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	users_list_widget = XtCreateManagedWidget("List",
			xmListWidgetClass, users_scrolled_widget, args, n);
#ifdef TRAVERSAL
	XmAddTabGroup(users_list_widget);
#endif
	
	/* Build group listing title */
	xmstring = XmStringCreate(msg_selection[53], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                     xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,            XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,         events_scrolled_widget); n++;
	XtSetArg(args[n], XmNleftAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,         users_scrolled_widget); n++;
	XtSetArg(args[n], XmNalignment,             XmALIGNMENT_CENTER); n++;
	w = XmCreateLabelGadget(form_widget, "GroupsLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment,               XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                                 w); n++;
	XtSetArg(args[n], XmNtopOffset,                                 0); n++;
	XtSetArg(args[n], XmNleftAttachment,              XmATTACH_WIDGET); n++; 
	XtSetArg(args[n], XmNleftWidget,            users_scrolled_widget); n++;
	XtSetArg(args[n], XmNborderWidth,                               0); n++;
	XtSetArg(args[n], XmNscrollingPolicy,       XmAPPLICATION_DEFINED); n++;
	XtSetArg(args[n], XmNvisualPolicy,                     XmVARIABLE); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy,             XmSTATIC); n++;
	groups_scrolled_widget = XtCreateManagedWidget(
			"Groups", xmScrolledWindowWidgetClass, 
			form_widget, args, n);

	n = 0;
	XtSetArg(args[n], XmNitems,                       group_xmstrings); n++;
	XtSetArg(args[n], XmNsensitive,                              TRUE); n++;
	XtSetArg(args[n], XmNitemCount,                       group_count); n++;
	XtSetArg(args[n], XmNselectionPolicy,           XmMULTIPLE_SELECT); n++; 
	XtSetArg(args[n], XmNlistSizePolicy,                   XmCONSTANT); n++;
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                            TRUE); n++;
#endif
	groups_list_widget = XtCreateManagedWidget("List",
			xmListWidgetClass, groups_scrolled_widget, args, n);
#ifdef TRAVERSAL
	XmAddTabGroup(groups_list_widget);
#endif
	
	/* which is wider? events_scrolled or groups_scrolled + users_scrolled? */
	n = 0;
	XtSetArg(args[n], XmNwidth,      &item_width); n++;
	XtGetValues(users_scrolled_widget, args, n);
	total_width = item_width + 10;

	n = 0;
	XtSetArg(args[n], XmNwidth,      &item_width); n++;
	XtGetValues(groups_scrolled_widget, args, n);
	total_width += item_width;

	n = 0;
	XtSetArg(args[n], XmNwidth,      &item_width); n++;
	XtGetValues(events_scrolled_widget, args, n);
	
	widest_widget = (item_width > total_width ? events_scrolled_widget : 
			groups_scrolled_widget);

	XSync(display, 0);

	/**********************************************************************/
	/* Object(file) list in scrolled window, titled                       */
	/**********************************************************************/

	/* Build object title */
	xmstring = XmStringCreate(msg_selection[54], charset);
	n = 0;
	XtSetArg(args[n], XmNlabelString,                    xmstring); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                          widg); n++;
	XtSetArg(args[n], XmNleftAttachment,          XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,                widest_widget); n++;
	w = XmCreateLabelGadget(form_widget, "FilesLabel", args, n);
	XtManageChild(w);
	XmStringFree(xmstring);
			   
	/* Build Scrolled area widget */
	n = 0;
	XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                        w); n++;
	XtSetArg(args[n], XmNleftAttachment,     XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget,           widest_widget); n++;
	XtSetArg(args[n], XmNborderWidth,                      1); n++;
	XtSetArg(args[n], XmNscrollingPolicy,        XmAUTOMATIC); n++;
	XtSetArg(args[n], XmNvisualPolicy,            XmCONSTANT); n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmAS_NEEDED); n++;
	XtSetArg(args[n], XmNshadowThickness,                  0); n++;
	objects_scrolled_widget  = XtCreateManagedWidget("Files", 
			xmScrolledWindowWidgetClass, form_widget, args, n);
	XtManageChild(objects_scrolled_widget);
	
	/* Create an inner form to hold the items within the scrolled area */
	n = 0;
	XtSetArg(args[n], XmNdefaultPosition,                   TRUE); n++;
	XtSetArg(args[n], XmNresizePolicy,              XmRESIZE_ANY); n++;
	XtSetArg(args[n], XmNrubberPositioning,                 True); n++;
	XtSetArg(args[n], XmNdialogStyle,         XmDIALOG_WORK_AREA); n++;
	inner_form_widget = XmCreateForm(objects_scrolled_widget, "Form", args, n);
	XtManageChild(inner_form_widget); 
	
	/* Build editable list of objects attached to inner_form_widget */
	SelectionObjectsWindow();
	
#if SEC_MAC
	/* For CMW we need subject/object min & max SL buttons */
	subject_label_widget = CreateHeader(form_widget, msg_selection[61], 
		objects_scrolled_widget, False, groups_scrolled_widget);
	toggle_sub_min_sl = CreatePushButton(form_widget, msg_selection[62],
		False, objects_scrolled_widget, False, subject_label_widget);
	toggle_sub_max_sl = CreatePushButton(form_widget, msg_selection[63],
		False, objects_scrolled_widget, False, toggle_sub_min_sl);
	object_label_widget = CreateHeader(form_widget, msg_selection[64], 
		toggle_sub_min_sl, False, groups_scrolled_widget);
	toggle_obj_min_sl = CreatePushButton(form_widget, msg_selection[62],
		False, toggle_sub_min_sl, False, subject_label_widget);
	toggle_obj_max_sl = CreatePushButton(form_widget, msg_selection[63],
		False, toggle_sub_min_sl, False, toggle_obj_min_sl);
	XtAddCallback(toggle_sub_min_sl, XmNactivateCallback, SLToggleCallback,
		SUBJECT_MIN_SL);
	XtAddCallback(toggle_sub_max_sl, XmNactivateCallback, SLToggleCallback,
		SUBJECT_MAX_SL);
	XtAddCallback(toggle_obj_min_sl, XmNactivateCallback, SLToggleCallback,
		OBJECT_MIN_SL);
	XtAddCallback(toggle_obj_max_sl, XmNactivateCallback, SLToggleCallback,
		OBJECT_MAX_SL);
#endif /* SEC_MAC */

	/* which is taller? events_scrolled + users_scrolled or */
	/*                  objects_scrolled? */
	
	n = 0;
	XtSetArg(args[n], XmNheight,      &item_height); n++;
	XtGetValues(events_scrolled_widget, args, n);
	total_height = item_height;

	n = 0;
	XtSetArg(args[n], XmNheight,      &item_height); n++;
	XtGetValues(users_scrolled_widget, args, n);
	total_height += item_height;

	n = 0;
	XtSetArg(args[n], XmNheight,      &item_height); n++;
	XtGetValues(objects_scrolled_widget, args, n);
	total_height2 = item_height;

	tallest_widget = (total_height > total_height2 ? users_scrolled_widget : 
			objects_scrolled_widget);

	/**********************************************************************/
	/* Create the OK, CANCEL, HELP buttons                                */
	/**********************************************************************/
	CreateThreeButtons (form_widget, tallest_widget,
				 &ok_button, &cancel_button, &help_button);
	XtAddCallback(ok_button, XmNactivateCallback, OKCallback, NULL);
	XtAddCallback(cancel_button, XmNactivateCallback, CancelCallback, NULL);
	XtAddCallback(help_button, XmNactivateCallback,
			HelpDisplayOpen, "audit,Selection");
}
		
static void 
SelectionObjectsWindow()
{
	Arg         args[20];
	Cardinal    n;
	int         i,
			j,
			sb_spacing;
	Widget      w;
	Dimension   item_height,
			item_width,
			total_height,
			total_width;
	char        buf[50],
			item_name[30];

	/* Create a list of editable fields, each with one object name */
	object_count = selection_fillin.nfiles;
	objects_list = (Widget *) Malloc(sizeof(Widget *) * object_count);
	
	for (i = 0; i < object_count; i++) {
		n = 0;
		if (i == 0){
			XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
		}
		else {
			XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
			XtSetArg(args[n], XmNtopWidget,   objects_list[i-1]); n++;
		}
		XtSetArg(args[n], XmNtopOffset,                     3);n++;
		XtSetArg(args[n], XmNleftAttachment,    XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNmaxLength,              PATH_MAX); n++;
		XtSetArg(args[n], XmNshadowThickness,               0); n++;
		XtSetArg(args[n], XmNvalue, selection_fillin.files[i]); n++;
		XtSetArg(args[n], XmNeditMode,     XmSINGLE_LINE_EDIT); n++;
		XtSetArg(args[n], XmNtraversalOn,                TRUE); n++;
		XtSetArg(args[n], XmNeditable,                   TRUE); n++;
		objects_list[i] = XtCreateWidget("TextEdit", xmTextWidgetClass, 
				inner_form_widget, args, n);
		XmAddTabGroup(objects_list[i]);
	}
	
	/***********************************************************************/
	/* Manually resize the window to be large enough to hold the form,     */
	/* no matter what size text is used.                                   */
	/***********************************************************************/
	
	n = 0;
	XtSetArg(args[n], XmNheight,                   &item_height); n++;
	XtSetArg(args[n], XmNwidth,                    &total_width); n++;
	XtGetValues(objects_list[0], args, n);
	
	n = 0;
	XtSetArg(args[n], XmNverticalScrollBar,                  &w); n++;
	XtSetArg(args[n], XmNspacing,                   &sb_spacing); n++;
	XtGetValues(objects_scrolled_widget, args, n);
	
	n = 0;
	XtSetArg(args[n], XmNwidth,                     &item_width); n++;
	XtGetValues(w, args, n);

	n = 0;
	XtSetArg(args[n], XmNheight,          4 * (item_height + 6)); n++;
	XtSetArg(args[n], XmNwidth, 
					 item_width + sb_spacing + 15 + total_width); n++;
	XtSetValues(objects_scrolled_widget, args, n);

	XtManageChildren(objects_list, object_count); 
}

#if SEC_MAC
static void 
SLToggleCallback(w, ptr, info) 
	Widget      w; 
	caddr_t	*ptr;
	caddr_t    	*info;
{
	XtUnmanageChild(form_widget);
	switch ((int) ptr) {
	case SUBJECT_MIN_SL:
		AudSelnssOpen();
		break;

	case SUBJECT_MAX_SL:
		AudSelxssOpen();
		break;

	case OBJECT_MIN_SL:
		AudSelnosOpen();
		break;

	case OBJECT_MAX_SL:
		AudSelxosOpen();
		break;

	}
}

void
SelectionReset()
{
	XtManageChild(form_widget);
}
#endif /* SEC_MAC */

#endif /* SEC_BASE */
