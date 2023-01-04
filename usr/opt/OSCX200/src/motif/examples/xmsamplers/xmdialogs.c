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
static char *rcsid = "@(#)$RCSfile: xmdialogs.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 21:31:28 $";
#endif
/*
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/
/*
 * Motif Release 1.2
*/
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: xmdialogs.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 21:31:28 $"
#endif
#endif
/*
*  (c) Copyright 1989 HEWLETT-PACKARD COMPANY. */

/**---------------------------------------------------------------------
***	
***	file:		xmdialogs.c
***
***	project:	Motif Widgets example programs
***
***	description:	This program demonstrates the Motif dialog widgets.
***	
***	defaults:	xmdialogs.c depends on these defaults:
!
*allowShellResize:		true
*borderWidth:			0
*highlightThickness:		2
*traversalOn:			true
*keyboardFocusPolicy:		explicit
*fontList:			vr-20
!
xmdialogs*XmBulletinBoard*autoUnmanage:		true
xmdialogs*XmFileSelectionBox*autoUnmanage:	false
xmdialogs*XmForm.height:			350
xmdialogs*XmForm.width:				275
xmdialogs*XmRowColumn*XmPushButtonGadget.marginLeft:	5
xmdialogs*XmRowColumn*XmPushButtonGadget.marginRight:	5
xmdialogs*menu_bar*background:			#58f
!
***-------------------------------------------------------------------*/



/*-------------------------------------------------------------
**	Include Files
*/

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Xfuncs.h>

#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/Command.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleBG.h>
#include <Xm/ToggleB.h>
#include <Xm/MwmUtil.h>


/*-------------------------------------------------------------
**	Forward Declarations
*/

static Widget CreateMenuBar ();
static Widget CreateSelectionBox ();
static Widget CreateWorkArea ();
static Widget CreateDialogBox ();



/*-------------------------------------------------------------
**	Global Variables
*/

#define MENU_HELP		200
#define MENU_EXIT		201
#define MENU_RESET		202

#define DIALOG_ALLOW_OVERLAP	300
#define DIALOG_AUTO_UNMANAGE	301
#define DIALOG_DEFAULT_POSITION	302
#define DIALOG_CREATE		303
#define DIALOG_DESTROY		304
#define DIALOG_MANAGE		305
#define DIALOG_UNMANAGE		306

#define BULLETIN_BOARD		0
#define COMMAND			2
#define FILE_SELECTION_BOX	4
#define FORM			6
#define MESSAGE_BOX		9
#define SELECTION_BOX		13

#define NUM_ITEMS		17

static Widget	work_area_toggle;
static Widget	modeless_toggle;
static Widget	application_modal_toggle;
static Widget	system_modal_toggle;
static Widget	auto_unmanage_toggle;
static Widget	default_position_toggle;
static Widget	create_button;
static Widget	destroy_button;
static Widget	manage_button;
static Widget	unmanage_button;

static unsigned char	resize_policy = XmRESIZE_ANY;


/*	Dialog widget IDs and index of active dialog
*/
static Widget	dialog[NUM_ITEMS];
static int	selected_item_index;
static Boolean	selected_item_is_dialog = True;
static Widget	shell_parent;

Display		*display;	/*  Display		*/


/*	Dialog names for toplevel SelectionBox
*/
static char 	*item[NUM_ITEMS] =
{
	"bulletin board",
	"bulletin board dialog",
	"command",
	"error dialog",
	"file selection box",
	"file selection dialog",
	"form",
	"form dialog",
	"information dialog",
	"message box",
	"message dialog",
	"prompt dialog",
	"question dialog",
	"selection box",
	"selection dialog",
	"warning dialog",
	"working dialog",
};

/*	Creation functions for dialogs
*/ 
typedef Widget (*CreateProc)();
CreateProc	create_proc[NUM_ITEMS] =
{
	CreateDialogBox,
	XmCreateBulletinBoardDialog,
	CreateDialogBox,
	XmCreateErrorDialog,
	CreateDialogBox,
	XmCreateFileSelectionDialog,
	CreateDialogBox,
	XmCreateFormDialog,
	XmCreateInformationDialog,
	CreateDialogBox,
	XmCreateMessageDialog,
	XmCreatePromptDialog,
	XmCreateQuestionDialog,
	CreateDialogBox,
	XmCreateSelectionDialog,
	XmCreateWarningDialog,
	XmCreateWorkingDialog,
};



/*-------------------------------------------------------------
**	MenuCB
**		Process callback from PushButtons in PulldownMenus.
*/
void MenuCB (w, client_data, call_data) 
     Widget		w;		/*  widget id		*/
     XtPointer		client_data;	/*  data from application   */
     XtPointer		call_data;	/*  data from widget class  */
{
	switch ((int) client_data)
	{
		case MENU_EXIT:
			printf ("xmdialogs: exiting...\n");
			exit (0);

		case MENU_HELP:
			printf ("xmdialogs: help reqested\n");
			break;

		default:
			printf ("xmdialogs: unexpected tag in menu callback\n");
			break;
	}

}



/*-------------------------------------------------------------
**	ResizeCB
**		Process callback from PushButtons in PulldownMenus.
*/
void ResizeCB (w, client_data, call_data) 
     Widget		w;		/*  widget id		*/
     XtPointer		client_data;	/*  data from application   */
     XtPointer		call_data;	/*  data from widget class  */
{

	resize_policy = (unsigned char) client_data;
}



/*-------------------------------------------------------------
**	ListCB
**		Process callback from List in toplevel SelectionBox.
**		Update index of currently selected dialog.
*/
void ListCB (w, client_data, call_data) 
     Widget		w;		/*  widget id		*/
     XtPointer		client_data;	/*  data from application   */
     XtPointer		call_data;	/*  data from widget class  */
{
	Boolean		new_item_is_dialog = False;
	XmListCallbackStruct *cb = (XmListCallbackStruct *) call_data;


	selected_item_index = cb->item_position - 1;
	if ( (selected_item_index != BULLETIN_BOARD) &&
	     (selected_item_index != COMMAND) &&
	     (selected_item_index != FILE_SELECTION_BOX) &&
	     (selected_item_index != FORM) &&
	     (selected_item_index != MESSAGE_BOX) &&
	     (selected_item_index != SELECTION_BOX) )
		new_item_is_dialog = True;
	if (new_item_is_dialog != selected_item_is_dialog)
	{
		XmToggleButtonGadgetSetState (work_area_toggle,
						!new_item_is_dialog, True);
		XtSetSensitive (work_area_toggle, !new_item_is_dialog);

		XmToggleButtonGadgetSetState (modeless_toggle,
						new_item_is_dialog, True);
		XtSetSensitive (modeless_toggle, new_item_is_dialog);
		XmToggleButtonGadgetSetState (application_modal_toggle,
						False, True);
		XtSetSensitive (application_modal_toggle, new_item_is_dialog);
		XmToggleButtonGadgetSetState (system_modal_toggle,
						False, True);
		XtSetSensitive (system_modal_toggle, new_item_is_dialog);
		XmToggleButtonGadgetSetState (auto_unmanage_toggle,
						new_item_is_dialog, True);
		XtSetSensitive (auto_unmanage_toggle, new_item_is_dialog);
		XmToggleButtonGadgetSetState (default_position_toggle,
						new_item_is_dialog, True);
		XtSetSensitive (default_position_toggle, new_item_is_dialog);
		XtSetSensitive (manage_button, new_item_is_dialog);
		XtSetSensitive (unmanage_button, new_item_is_dialog);
	}
	selected_item_is_dialog = new_item_is_dialog;
}


/*-------------------------------------------------------------
**	GetDialogArguments
**		Set up arglist for dialog creation functions.
*/
void GetDialogArguments (al, ac_rtn) 
ArgList		al;
int		*ac_rtn;
{
	Boolean		auto_unmanage, default_position;
	unsigned char	dialog_style;
	XmString	title_string;
	int	ac = 0;

	auto_unmanage = XmToggleButtonGadgetGetState (auto_unmanage_toggle);
	default_position = XmToggleButtonGadgetGetState (default_position_toggle);
	if (XmToggleButtonGadgetGetState (application_modal_toggle))
		dialog_style = XmDIALOG_APPLICATION_MODAL;
	else if (XmToggleButtonGadgetGetState (system_modal_toggle))
		dialog_style = XmDIALOG_SYSTEM_MODAL;
	else if (XmToggleButtonGadgetGetState (modeless_toggle))
		dialog_style = XmDIALOG_MODELESS;
	else
		dialog_style = XmDIALOG_WORK_AREA;
	
	XtSetArg (al[ac], XmNdialogStyle, dialog_style);  ac++;
	XtSetArg (al[ac], XmNautoUnmanage, auto_unmanage);  ac++;
	XtSetArg (al[ac], XmNdefaultPosition, default_position);  ac++;

	title_string = XmStringCreateLtoR (item[selected_item_index],
				XmSTRING_DEFAULT_CHARSET);
	XtSetArg (al[ac], XmNdialogTitle, title_string);  ac++;
	XtSetArg (al[ac], XmNresizePolicy, resize_policy);  ac++;

	*ac_rtn = ac;
}



/*-------------------------------------------------------------
**	CreateDialogBox
**		Create TopLevelShell and dialog child.
*/
static Widget CreateDialogBox (as_p, name, d_al, d_ac)
Widget		as_p;		/*  parent for shell	*/
String		name;		/*  widget name		*/
ArgList		d_al;		/*  arglist for sb	*/
int		d_ac;		/*  arg count for sb	*/
{
	Widget		as;		/*  DialogShell		*/
	Arg		as_al[10];	/*  arglist for shell	*/
	int		as_ac;		/*  argcount for shell	*/
	ArgList		_d_al;		/*  arglist for dialog	*/
	Widget		d;		/*  new dialog widget	*/


	/*	Create TopLevelShell parent.
	*/
	as_ac = 0;
	XtSetArg (as_al[as_ac], XmNallowShellResize, True);  as_ac++;
	as = XtCreatePopupShell (name, topLevelShellWidgetClass,
			as_p, as_al, as_ac);


	/*	Allocate arglist, copy args, add dialog type arg.
	*/
	_d_al = (ArgList) XtMalloc (sizeof (Arg) * (d_ac + 1));

	bcopy (d_al, _d_al, sizeof (Arg) * d_ac);


	/*	Set dialog type as needed, create dialog box.
	*/
	switch ((int) selected_item_index)
	{
		case BULLETIN_BOARD:
			d = XmCreateBulletinBoard (as, name, _d_al, d_ac);
			break;
		case COMMAND:
			XtSetArg (_d_al[d_ac], XmNdialogType, 
					XmDIALOG_COMMAND);  d_ac++;
			d = XmCreateCommand (as, name, _d_al, d_ac);
			break;
		case FILE_SELECTION_BOX:
			XtSetArg (_d_al[d_ac], XmNdialogType,
					XmDIALOG_FILE_SELECTION);  d_ac++;
			d = XmCreateFileSelectionBox (as, name, _d_al, d_ac);
			break;
		case FORM:
			d = XmCreateForm (as, name, _d_al, d_ac);
			break;
		case MESSAGE_BOX:
			XtSetArg (_d_al[d_ac], XmNdialogType,
					XmDIALOG_MESSAGE);  d_ac++;
			d = XmCreateMessageBox (as, name, _d_al, d_ac);
			break;
		case SELECTION_BOX:
			XtSetArg (_d_al[d_ac], XmNdialogType,
					XmDIALOG_SELECTION);  d_ac++;
			d = XmCreateSelectionBox (as, name, _d_al, d_ac);
			break;
		default:
			printf("xmdialogs: unexpected tag in CreateDialogBox");
			break;
	}

	/*	Manage and realize.
	*/
	XtManageChild (d);
	XtRealizeWidget (as);	
	XtPopup (as, XtGrabNone);


	/*	Free args, return.
	*/
	XtFree ((XtPointer)_d_al);
	return (d);
}



/*-------------------------------------------------------------
**	WorkAreaCB
**		Process callback from PushButtons in SelectionBox work area.
*/
void WorkAreaCB (w, client_data, call_data) 
     Widget		w;		/*  widget id		*/
     XtPointer		client_data;	/*  data from application   */
     XtPointer		call_data;	/*  data from widget class  */
{
	Arg		al[10];		/*  arg list		*/
	int		ac = 0;		/*  arg count		*/

	register int	index 	= selected_item_index;

	switch ((int) client_data)
	{
		case DIALOG_CREATE:
			if (dialog[index])
			XtDestroyWidget (XtParent(dialog[index]));
			GetDialogArguments (al, &ac);
			dialog[index] = (*create_proc[index]) (shell_parent,
						item[index], al, ac);
			break;
		case DIALOG_DESTROY:
			if (dialog[index])
			{
				XtUnmanageChild (dialog[index]);
				XtDestroyWidget (XtParent(dialog[index]));
				dialog[index] = NULL;
			}
			break;
		case DIALOG_MANAGE:
			if (!dialog[index])
			{
				GetDialogArguments (al, &ac);
				dialog[index] =
					(*create_proc[index]) (shell_parent,
						item[index], al, ac);
			}
			XtManageChild (dialog[index]);
			break;
		case DIALOG_UNMANAGE:
			if (dialog[index])
				XtUnmanageChild (dialog[index]);
			break;
		default:
			printf ("xmdialogs: unexpected tag in WorkAreaCB\n");
			break;
	}
}



/*-------------------------------------------------------------
**	CreateMenuBar
**		Create MenuBar in MainWindow
*/
static Widget CreateMenuBar (parent)
     Widget		parent;
{
	Widget		menu_bar;	/*  RowColumn	 	*/
	Widget		cascade;	/*  CascadeButton	*/
	Widget		menu_pane;	/*  RowColumn	 	*/
	Widget		button;		/*  PushButton		*/

	Arg		al[10];		/*  arg list		*/
	register int	ac;		/*  arg count		*/


	/*	Create MenuBar.
	*/
	ac = 0;
	menu_bar = XmCreateMenuBar (parent, "menu_bar", al, ac);
	XtManageChild (menu_bar);


	/*	Create "Options" PulldownMenu.
	*/
	ac = 0;
	menu_pane = XmCreatePulldownMenu (menu_bar, "menu_pane", al, ac);

	ac = 0;
	button = XmCreatePushButton (menu_pane, "Exit", al, ac);
	XtAddCallback (button, XmNactivateCallback, MenuCB, (XtPointer)MENU_EXIT);
	XtManageChild (button);

	ac = 0;
	XtSetArg (al[ac], XmNsubMenuId, menu_pane);  ac++;
	cascade = XmCreateCascadeButton (menu_bar, "Actions", al, ac);
	XtManageChild (cascade);


	/*	Create "Help" button.
	*/
	ac = 0;
	cascade = XmCreateCascadeButton (menu_bar, "Help", al, ac);
	XtAddCallback (cascade, XmNactivateCallback, MenuCB, (XtPointer)MENU_HELP);
	XtManageChild (cascade);

	ac = 0;
	XtSetArg (al[ac], XmNmenuHelpWidget, cascade);  ac++;
	XtSetValues (menu_bar, al, ac);

	return (menu_bar);
}



/*-------------------------------------------------------------
**	CreateSelectionBox
**		Create top level SelectionBox.
*/
static Widget CreateSelectionBox (parent)
Widget		parent;
{
	Widget		selection_box;	/*  SelectionBox	*/
	Widget		list;		/*  List		*/
	Widget		text;		/*  Text		*/
	Widget		kid[5];		/*  buttons		*/
	Arg		al[10];		/*  arg list		*/
	register int	ac;		/*  arg count		*/
	register int	i;		/*  counter		*/
	XmString	list_item[NUM_ITEMS];	/*  list items	*/
	XmStringCharSet	charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
	Widget		hsbar, vsbar;	/*  ScrollBars		*/
	XrmValue	pixel_data;



	/*	Set up items for List.
	*/
	for ( i = 0;  i < NUM_ITEMS;  i++ )
		list_item[i] = XmStringCreateLtoR (item[i], charset);


	/*	Create toplevel SelectionBox.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNshadowThickness, 1);  ac++;
	XtSetArg (al[ac], XmNshadowType, XmSHADOW_OUT);  ac++;
	XtSetArg (al[ac], XmNtextString, list_item[0]);  ac++;
	XtSetArg (al[ac], XmNlistItems, list_item);  ac++;
	XtSetArg (al[ac], XmNlistItemCount, NUM_ITEMS);  ac++;
	XtSetArg (al[ac], XmNlistLabelString, 
			XmStringCreateLtoR ("Motif Dialog Widgets",
						 charset));  ac++;
	XtSetArg (al[ac], XmNselectionLabelString, 
			XmStringCreateLtoR ("Active Dialog", charset));  ac++;
	selection_box = XmCreateSelectionBox (parent, "selection_box",
			al, ac);


	/*	Register callbacks for SelectionBox list.
	*/
	list = XmSelectionBoxGetChild (selection_box, XmDIALOG_LIST);
	XtAddCallback (list, XmNbrowseSelectionCallback, ListCB, NULL);
	XtAddCallback (list, XmNdefaultActionCallback, ListCB, NULL);
	shell_parent = list;


	/*	Set colors of recessed widgets.
	*/
	if (DefaultDepthOfScreen(XDefaultScreenOfDisplay(display)) > 1)
	{
		text = XmSelectionBoxGetChild (selection_box, XmDIALOG_TEXT);
		XtSetArg (al[0], XmNhorizontalScrollBar, &hsbar);
		XtSetArg (al[1], XmNverticalScrollBar, &vsbar);
		XtGetValues (XtParent (list), al, 2);

		_XmSelectColorDefault (selection_box, NULL, &pixel_data);
		XtSetArg (al[0], XmNbackground, *((Pixel *)pixel_data.addr));

		XtSetValues (list, al, 1);
		XtSetValues (text, al, 1);
		XtSetValues (hsbar, al, 1);
		XtSetValues (vsbar, al, 1);
	}


	/*	Unmanage unneeded children.
	*/
	i = 0;
	kid[i++] = XmSelectionBoxGetChild (selection_box, XmDIALOG_SEPARATOR);
	kid[i++] = XmSelectionBoxGetChild (selection_box, XmDIALOG_OK_BUTTON);
	kid[i++] = XmSelectionBoxGetChild (selection_box, XmDIALOG_CANCEL_BUTTON);
	kid[i++] = XmSelectionBoxGetChild (selection_box, XmDIALOG_APPLY_BUTTON);
	kid[i++] = XmSelectionBoxGetChild (selection_box, XmDIALOG_HELP_BUTTON);
	XtUnmanageChildren (kid, i);

	return (selection_box);
}



/*-------------------------------------------------------------
**	CreateWorkArea
**		Create work area Form in SelectionBox.
**		Create RadioBox for dialog style.
**		Create RowColumn for dialog attributes.
*/
static Widget CreateWorkArea (parent)
Widget		parent;
{
	Widget		row_column;	/*  RowColumn		*/
	Widget		menu_pane;	/*  RowColumn	 	*/
	Widget		box;		/*  Form		*/
	Widget		button;		/*  PushButton		*/
	Widget		default_button;	/*  PushButton		*/
	Widget		frame0, frame1, frame2, frame3;	/*  Frames	*/

	Arg		al[10];		/*  arg list		*/
	register int	ac;		/*  arg count		*/

	XmString	label_string = NULL;


	/*	Create outer Form box.
	*/
	ac = 0;
	box = XmCreateForm (parent, "work_area", al, ac);
	XtManageChild (box);


	/*	Create RadioBox and dialog style toggles.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNshadowType, XmSHADOW_ETCHED_IN);  ac++;
	XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
	frame0 = XmCreateFrame (box, "frame", al, ac);
	XtManageChild (frame0);

	ac = 0;
	menu_pane = XmCreatePulldownMenu (frame0, "menu_pane", al, ac);

	ac = 0;
	default_button = XmCreatePushButton (menu_pane, "any", al, ac);
	XtAddCallback (default_button, XmNactivateCallback, ResizeCB, (XtPointer)XmRESIZE_ANY);
	XtManageChild (default_button);

	button = XmCreatePushButton (menu_pane, "grow", al, ac);
	XtAddCallback (button, XmNactivateCallback, ResizeCB, (XtPointer)XmRESIZE_GROW);
	XtManageChild (button);

	button = XmCreatePushButton (menu_pane, "none", al, ac);
	XtAddCallback (button, XmNactivateCallback, ResizeCB, (XtPointer)XmRESIZE_NONE);
	XtManageChild (button);

	label_string = XmStringCreateLtoR ("resize policy",
			XmSTRING_DEFAULT_CHARSET);
	ac = 0;
	XtSetArg (al[ac], XmNlabelString, label_string);  ac++;
	XtSetArg (al[ac], XmNmenuHistory, default_button);  ac++;
	XtSetArg (al[ac], XmNsubMenuId, menu_pane);  ac++;
	row_column = XmCreateOptionMenu (frame0, "row_column1", al, ac);
	XtManageChild (row_column);
	if (label_string)
		XtFree ((XtPointer)label_string);



	/*	Create RadioBox and dialog style toggles.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNshadowType, XmSHADOW_ETCHED_IN);  ac++;
	XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
	XtSetArg (al[ac], XmNtopWidget, frame0);  ac++;
	XtSetArg (al[ac], XmNtopOffset, 10);  ac++;
	frame1 = XmCreateFrame (box, "frame", al, ac);
	XtManageChild (frame1);

	ac = 0;
	XtSetArg (al[ac], XmNentryClass, xmToggleButtonGadgetClass);  ac++;
	row_column = XmCreateRadioBox (frame1, "row_column1", al, ac);
	XtManageChild (row_column);

	ac = 0;
	XtSetArg (al[ac], XmNset, True);  ac++;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	work_area_toggle =
		XmCreateToggleButtonGadget (row_column,
					"work area", al, ac);
	XtManageChild (work_area_toggle);

	ac = 0;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	modeless_toggle =
		XmCreateToggleButtonGadget (row_column, 
					"modeless", al, ac);
	XtManageChild (modeless_toggle);

	ac = 0;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	application_modal_toggle = 
		XmCreateToggleButtonGadget (row_column,
					"application modal", al, ac);
	XtManageChild (application_modal_toggle);

	ac = 0;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	system_modal_toggle =
		XmCreateToggleButtonGadget (row_column,
					"system modal", al, ac);
	XtManageChild (system_modal_toggle);


	/*	Create RowColumn and dialog attribute toggles.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNshadowType, XmSHADOW_ETCHED_IN );  ac++;
	XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
	XtSetArg (al[ac], XmNtopWidget, frame1);  ac++;
	XtSetArg (al[ac], XmNtopOffset, 10);  ac++;
	XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
	frame2 = XmCreateFrame (box, "frame", al, ac);
	XtManageChild (frame2);

	ac = 0;
	row_column = XmCreateRowColumn (frame2, "row_column2", al, ac);
	XtManageChild (row_column);

	ac = 0;
	XtSetArg (al[ac], XmNset, True);  ac++;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	auto_unmanage_toggle =
		XmCreateToggleButtonGadget (row_column,
						"auto unmanage", al, ac);
	XtManageChild (auto_unmanage_toggle);

	ac = 0;
	XtSetArg (al[ac], XmNset, True);  ac++;
	XtSetArg (al[ac], XmNshadowThickness, 0);  ac++;
	default_position_toggle = 
		XmCreateToggleButtonGadget (row_column,
						"default position", al, ac);
	XtManageChild (default_position_toggle);


	/*	Create RowColumn with action buttons.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNshadowType, XmSHADOW_ETCHED_IN );  ac++;
	XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
	XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
	XtSetArg (al[ac], XmNtopWidget, frame2);  ac++;
	XtSetArg (al[ac], XmNtopOffset, 10);  ac++;
	XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
	frame3 = XmCreateFrame (box, "work_area", al, ac);
	XtManageChild (frame3);

	ac = 0;
	XtSetArg (al[ac], XmNpacking, XmPACK_COLUMN);  ac++;
	XtSetArg (al[ac], XmNnumColumns, 2);  ac++;
	row_column = XmCreateRowColumn (frame3, "row_column3", al, ac);
	XtManageChild (row_column);

	ac = 0;
	create_button =
		XmCreatePushButtonGadget (row_column, "create", al, ac);
	XtAddCallback (create_button, XmNactivateCallback, WorkAreaCB,
			(XtPointer)DIALOG_CREATE);
	XtManageChild (create_button);

	ac = 0;
	destroy_button =
		XmCreatePushButtonGadget (row_column, "destroy", al, ac);
	XtAddCallback (destroy_button, XmNactivateCallback, WorkAreaCB,
			(XtPointer)DIALOG_DESTROY);
	XtManageChild (destroy_button);

	ac = 0;
	manage_button =
		XmCreatePushButtonGadget (row_column, "manage", al, ac);
	XtAddCallback (manage_button, XmNactivateCallback, WorkAreaCB,
			(XtPointer)DIALOG_MANAGE);
	XtManageChild (manage_button);

	ac = 0;
	unmanage_button =
		XmCreatePushButtonGadget (row_column, "unmanage", al, ac);
	XtAddCallback (unmanage_button, XmNactivateCallback, WorkAreaCB,
			(XtPointer)DIALOG_UNMANAGE);
	XtManageChild (unmanage_button);


	return (box);
}



/*-------------------------------------------------------------
**	main
**		Initialize toolkit.
**		Create MainWindow and subwidgets.
**		Realize toplevel widgets.
**		Process events.
*/
#ifdef DEC_MOTIF_BUG_FIX
int main (argc,argv)
#else
void main (argc,argv)
#endif
int  argc;
char **argv;
{
	Widget		app_shell;	/*  ApplicationShell 	*/
	Widget		main;		/*  MainWindow	 	*/
	Widget		menu_bar;	/*  Frame	 	*/
	Widget		work_area;	/*  SelectionBox	*/
	Widget		form;		/*  Form		*/
	XtAppContext	app_context;	/*  App Context		*/

	Arg		al[10];		/*  arg list		*/
	register int	ac;		/*  arg count		*/

	XmListCallbackStruct	cb;

	/*	Initialize toolkit and open display.
	*/
	XtToolkitInitialize ();
	app_context = XtCreateApplicationContext();
	display = XtOpenDisplay (app_context, NULL, argv[0], "XMdemos",
			NULL, 0, &argc, argv);
	if (!display)
	{
		XtWarning ("xmdialogs: can't open display, exiting...");
		exit (0);
	}


	/*	Create ApplicationShell.
	*/
	app_shell = XtAppCreateShell (NULL, "XMdemos",
			applicationShellWidgetClass, display, NULL, 0);


	/*	Create MainWindow.
	*/
	ac = 0;
	XtSetArg (al[ac], XmNscrollingPolicy, XmAPPLICATION_DEFINED);  ac++;
	main = XmCreateMainWindow (app_shell, "main", al, ac);
	XtManageChild (main);


	/*	Create MenuBar in MainWindow.
	*/
	menu_bar = CreateMenuBar (main);
	XtManageChild (menu_bar);


	/*	Create toplevel SelectionBox.
	*/
	work_area = CreateSelectionBox (main);
	XtManageChild (work_area);


	/*	Create work area in SelectionBox.
	*/
	form = CreateWorkArea (work_area);
	XtManageChild (form);


	/*	Set areas of MainWindow.
	*/
	XmMainWindowSetAreas (main, menu_bar, NULL, NULL, NULL,
					work_area);


	/*	Realize toplevel widgets.
	*/
	XtRealizeWidget (app_shell);


	/*	Fake List callback to initialize selected item data.
	*/
	cb.item_position = 1;
	ListCB (NULL, NULL, (XtPointer) &cb);


	/*	Process events.
	*/
	XtAppMainLoop (app_context);
}

