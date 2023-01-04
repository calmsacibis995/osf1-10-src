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
/*
** COPYRIGHT (c) 1988, 1989, 1990, 1991, 1992 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
a**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	LinkWorks Services User Interface
**
**  Version: V1.0
**
**  Abstract:
**	DXmUi object private data definitions.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 7-Oct-88
**
**  Modification History:
**	BL4  dpr  24-Jan-89 -- some serious clean up after BL4 code review
**	     lg    5-Mar-90 -- XUI->Motif conversion
**  X0.16   WWC  21-Feb-91  include lwk_dxm_def.h
**  X0.16-1 WWC  27-Feb-91  VMS:    his$decwindows.uid --> lwk$dxmui.uid
**			    ULTRIX: his_decwindows.uid --> lwk_dxmui
**--
*/


/*
**  Include Files
*/



#include <Xm/XmP.h>
/*
** DECWmHints.h is included by a toolkit include file on VMS.
** This doesn't happen on other platforms.
*/
#ifndef VMS
#include <X11/DECWmHints.h> 
#endif

#include <Mrm/MrmPublic.h>
#include <DXm/DXmSvn.h>
#include <Xm/Text.h>
#include <X11/Shell.h>



#include <lwk_dxm_def.h>

/*
**  Macro Definitions
*/

#define _StatusToHelpKey(Status) LwkDXmStatusToHelpKey((Status))
_DeclareFunction(_CString LwkDXmStatusToHelpKey, (_Status status));

#define _MaxMessageCount 4

#define _DrmPrivateIdentifier "LwkPrivate"
#define _DrmMenuIdentifier "LwkMenu"

#define _MrmLinkMenu "LinkMenu"
#define _MrmLinkMenuAccel "LinkMenuAccel"

#define _MrmLinkMenuHelpKey "MenuHelpKey"

#define _MrmHighlightOnLiteral "HighlightOnLiteral"
#define _MrmHighlightOnMnemonic "HighlightOnMnemonic"
#define _MrmHighlightOffLiteral "HighlightOffLiteral"
#define _MrmHighlightOffMnemonic "HighlightOffMnemonic"
#define _MrmDBoxIcon "DBoxIcon"

#define _IconWidth 20
#define _IconHeight 16

#ifdef VMS
#define _UidFileName "lwk$dxmui"
#else
#define _UidFileName "lwk_dxmui_1_2"
#endif

/*
**  Type Definitions
*/

/*
**  Shorthand for Toolkit reason code in callbacks
*/

typedef XmAnyCallbackStruct *_Reason;

/*
**  DwUi Private Data
*/
                 
typedef struct __LinkInfoInstance {
	    _Link link;
	    _String *opr_ids;
	    _String *opr_names;
	    _Integer op_cnt;
	    _Boolean out_going;
	} _LinkInfoInstance, *_LinkInfo;

typedef struct __StepInfoInstance {
	    _String *opr_ids;
	    _String *opr_names;
	    _Integer op_cnt;
	} _StepInfoInstance, *_StepInfo;

typedef struct __HistoryInfoInstance {
	    _Step step;
	    _String *origin_opr_ids;
	    _String *origin_opr_names;
	    _String *dest_opr_ids;
	    _String *dest_opr_names;
	    _Integer origin_op_cnt;
	    _Integer dest_op_cnt;
 	} _HistoryInfoInstance, *_HistoryInfo;

typedef struct __MenuWidgets {
	    _AnyPtr private;
	    Widget menu;
	    Widget go_to;
	    Widget visit;
	    Widget highlight_toggle;
	    Widget highlight;
	    Widget step;
	    Widget undo;
	    Widget show_history;
	    Widget set_source;
	    Widget set_target;
	    Widget complete_link;
	    Widget annotate;
	    Widget complete_link_dialog;
	    Widget show_links;
	    XmString highlight_on_literal;
	    KeySym highlight_on_keysym;
	    XmString highlight_off_literal;
	    KeySym highlight_off_keysym;
	    Boolean default_accelerators;
	    Widget menu_entry;
	    Boolean enabled_annotate;
	} _MenuWidgetsInstance, *_MenuWidgets;

typedef struct __HistoryWidgets {
	    Widget dialog;
	    Widget steps;
	    Widget operations;
	    Widget show;
	    Widget visit;
	    Widget cancel;
	    Widget listform;
	    Widget visitform;
	    Widget showform;
	    Widget buttonsform;
	    XmString goto_literal;
	    XmString visit_literal;
	    XmString implicit_goto_literal;
	} _HistoryWidgetsInstance, *_HistoryWidgets;

typedef struct __StepWidgets {
	    Widget dialog;
	    Widget origin_desc;
	    Widget origin_type;
	    Widget step_desc;
	    Widget follow_go_to;
	    Widget follow_visit;
	    Widget operation;
	    Widget dest_type;
	    Widget dest_desc;
	    Widget ok_button;
	    Widget cancel_button;
	    Widget apply_button;
	    Widget reset_button;
	    _Boolean origin_desc_changed;
	    _Boolean step_desc_changed;
	    _Boolean follow_type_changed;
	    _Boolean operation_changed;
	    _Boolean dest_desc_changed;
	} _StepWidgetsInstance, *_StepWidgets;

typedef struct __LinksWidgets {
	    Widget dialog;
	    Widget caution;
	    Widget links;
	    Widget operations;
	    Widget show;
	    Widget delete;
	    Widget go_to;
	    Widget visit;
	    Widget cancel;
	    Widget listform;
	    Widget editform;
	    Widget navigateform;
	    Widget buttonsform;
	} _LinksWidgetsInstance, *_LinksWidgets;

typedef struct __LinkWidgets {
	    Widget dialog;
	    Widget source_desc;
	    Widget source_type;
	    Widget link_type;
	    Widget link_desc;
	    Widget target_type;
	    Widget target_desc;
	    Widget retain_source;
	    Widget retain_target;
	    Widget ok_button;
	    Widget apply_button;
	    Widget reset_button;
	    Widget cancel_button;
	    _Boolean source_desc_changed;
	    _Boolean link_type_changed;
	    _Boolean link_desc_changed;
	    _Boolean target_desc_changed;
	    _Boolean dialog_on_screen;
	} _LinkWidgetsInstance, *_LinkWidgets;

typedef struct __HighlightWidgets {
	    Widget dialog;
	    Widget highlight;
	    Widget source;
	    Widget target;
	    Widget orphan;
	    Widget pending_source;
	    Widget pending_target;
	    Widget destination;
	    Widget ok_button;
	    Widget apply_button;
	    Widget reset_button;
	    Widget cancel_button;
	} _HighlightWidgetsInstance, *_HighlightWidgets;

typedef struct __WIPListStruct {
	    _CString message_text;
	    struct __WIPBoxStruct *current_box;
	    struct __WIPBoxStruct *idle_box;
	} _WIPListStructInstance, *_WIPListStruct;


typedef struct __WIPBoxStruct {
	    Widget wipbox;
	    _String property_name;
	    struct __WIPBoxStruct *next;
	} _WIPBoxStructInstance, *_WIPBoxStruct;

typedef struct __DXmUiPrivate {
	    struct __DXmUiPrivate *next;
	    _DXmUi dwui;              
	    XmString appl_name;
	    _FollowType	links_follow_type;
	    int	selected_link_entry;
	    int	selected_link_op_entry;
	    int	link_info_size;
	    _LinkInfo link_info;
	    int	selected_history_entry;
	    int selected_history_op_entry;
	    int history_op_cnt;
	    int history_info_size;
	    _HistoryInfo history_info;
	    int selected_step_op_entry;
 	    _StepInfo step_info;
	    _Link link_to_link;
	    _HighlightFlags highlighting;
	    Widget main_widget;
	    Widget message_box;
	    Widget help_box;
	    Opaque help_context;
	    _MenuWidgets pulldown;
	    _HistoryWidgets history;
	    _StepWidgets step;
	    _LinksWidgets links;
	    _LinkWidgets complete_link;
	    _LinkWidgets show_link;
	    _HighlightWidgets highlight;
	    Pixmap menu_icon;
	    Pixmap dbox_icon;
	    int dbox_icon_size;
	    Pixmap dbox_iconify_pixmap;
	    Pixmap menu_icon_hlight;
	    Pixmap link_in_icon;
	    Pixmap link_out_icon;
	    Pixmap source_icon;
	    Pixmap target_icon;
	    Pixmap source_and_target_icon;
	    Pixmap target_and_source_icon;
	    Pixmap step_icon;
	    Pixmap origin_icon;
	    Pixmap destination_icon;
	    Pixmap origin_and_destination_icon;
	    _WIPListStruct wip;
	} _DXmUiPrivateInstance, *_DXmUiPrivate;


/*
**  External Routine Declarations
*/
_DeclareFunction(void LwkDXmHelpContextSensitiveHelp,
    (Widget w, _CString topic, _Reason reason));
_DeclareFunction(_DXmUiPrivate LwkDXmCreate,
    (_DXmUi dwui, _CString appl_name, _Boolean create_menu,
	_Boolean default_accelerators, Widget main_window, Widget menu_entry));
_DeclareFunction(void LwkDXmDelete,
    (_DXmUiPrivate private));
_DeclareFunction(MrmHierarchy LwkDXmOpenDRMHierarchy, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmCloseDRMHierarchy, (MrmHierarchy hierarchy));
_DeclareFunction(void LwkDXmCompleteLinkCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmCompleteLink,
    (_DXmUiPrivate private, _Link link, _Boolean iff_visible,
	_Closure closure));
_DeclareFunction(void LwkDXmLinkPopup,
    (_LinkWidgets widgets, Widget avoid,
	_Boolean pending_delete, _Reason reason));
_DeclareFunction(void LwkDXmLinkDisplay,
    (_DXmUiPrivate private, _Link link, _LinkWidgets widgets));
_DeclareFunction(void LwkDXmShowLinkBtnSensitivity,
    (_LinkWidgets widgets, _Boolean sensitive));
_DeclareFunction(void LwkDXmShowLinkCancelBtnDef,
    (_LinkWidgets widgets));
_DeclareFunction(void LwkDXmLinkUpdate,
    (_Link link, _LinkWidgets widgets));
_DeclareFunction(void LwkDXmShowLinksCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmFreeLinkInfo, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmShowLinks,
    (_DXmUiPrivate private, _List_of(_Link) links,
	_List_of(_List_of(_String)) opr_ids,
	_List_of(_List_of(XmString)) opr_names,
	_List_of(_Boolean) directions,
	_FollowType follow_type, _Boolean iff_visible,
	_Boolean update, _Closure closure));
_DeclareFunction(void LwkDXmShowHistoryCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmFreeHistoryInfo,
    (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmShowHistory,
    (_DXmUiPrivate private, _List_of(_Step) steps,
	_List_of(_List_of(_String)) origin_opr_ids,
	_List_of(_List_of(XmString)) origin_opr_names,
	_List_of(_List_of(_String)) dest_opr_ids,
	_List_of(_List_of(XmString)) dest_opr_names,
	_Boolean iff_visible, _Closure closure));
_DeclareFunction(void LwkDXmStepInfoCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmFreeStepInfo, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmStepUpdate, (_Step step, _DXmUiPrivate private));
_DeclareFunction(void LwkDXmHighlightCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmShowLinkCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmFreeWIPBoxInfo, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmMessageBoxCreate, (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmMessageBox,
    (_DXmUiPrivate private, lwk_status *status, _Integer count));
_DeclareFunction(void LwkDXmHelpBoxCreate,
    (_DXmUiPrivate private));
_DeclareFunction(void LwkDXmPositionWidget,
    (Widget new_widget, Widget avoid_widget));
_DeclareFunction(XmString LwkDXmStringToXmString,
    (char *string));
_DeclareFunction(_CString LwkDXmConcatCString,
    (_CString cstring1, _CString cstring2));

_DeclareFunction(Boolean LwkDXmIsXUIWMRunning,
    (Widget widget));
_DeclareFunction(void LwkDXmSetXUIDialogTitle,
    (Widget dialog, _CString part1, _CString part2));

_DeclareFunction(void LwkDXmSetShellIconState,
    (Widget shell, long state));
_DeclareFunction(void LwkDXmSetIconsOnShell,
    (Widget shell, _DXmUiPrivate private, XEvent *event,
	Boolean *continue_to_dispatch));


#define _StringToXmString(String) LwkDXmStringToXmString((String))
