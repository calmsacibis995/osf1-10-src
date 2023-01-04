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
** COPYRIGHT (c) 1989, 1990, 1991, 1992 BY
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
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/


/*
**++
**  Subsystem:
**	LinkWorks Manager
**
**  Version: V1.0
**
**  Abstract:
**	Window object private data definitions
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 13-Nov-89
**
**  Modification History:
**  X0.7    Pat	    13-Nov-89	add repository box structure and spec. pointer
**  X0.7-1  Pat     15-Nov-89   change _WindowPrivate structure
**  X0.7-2  Pat	    17-Nov-89	add uid file name
**  X0.7-3  Pat	    20-Nov-89   add shell widget
**  X0.7-4  ap	    23-Nov-89	declare SetCurrency routine
**  X0.7-5  ap	    24-Nov-89	add more includes and macros
**  X0.7-6  pa	    28-Nov-89	Ultrix code cleanup
**  X0.8    Pat	    6-Dec-89	remove rep box structure
**  X0.8-1  Pat	    11-Dec-89	Svn implementation
**  X0.8-2  ap	    13-Dec-89	Add the open forward declaration
**  X0.8-3  Pat	    18-Dec-89	add bitmap include files
**  X0.11   lg      25-Apr-90   Conditional compilation for XUI version
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



#include <lwk_dxm_def.h>

/*
**  Macro Definitions
*/

#define HS_FONT "ISO8859-1"
#define _FontList XmFontList

/*
**  DRM file names
*/
         
#ifdef VMS
#define _EnvWindowUidFileName	 "DECW$LWK_MGR_ENV"
#define _LbWindowUidFileName	 "DECW$LWK_MGR_LB"
#define _LinkWorksMgrUidFileName "DECW$LWK_MGR_MISC"
#else
#define _EnvWindowUidFileName	 "lwk_mgr_env"
#define _LbWindowUidFileName	 "lwk_mgr_lb"
#define _LinkWorksMgrUidFileName "lwk_mgr_misc"
#endif

/*
**  DRM identifers
*/

#define _DrmPrivateIdentifier	"windowprivate"

#define _DrmEnvironmentWindow	    "env_window"
#define _DwtEnvWinCopyrightTitle    "EnvWindowCopyrightTitle"
#define _DwtEnvWindowTitle	    "EnvWindowTitle"
#define _DwtEnvWindowIcon	    "DwtEnvWindowIcon"
#define _DwtEnvWindowIconName	    "EnvWindowIconName"

#define _DrmLinkbaseWindow	"env_window"
#define _DwtLbWindowTitleBase	"LbWindowTitleBase"
#define _DwtLbWindowTitle	"LbWindowTitle"
#define _DwtLbWindowIcon	"DwtLbWindowIcon"
#define _DwtLbWindowIconName	"LbWindowIconName"

#define _DrmLinkbaseBox		"lb_select_box"
#define _DrmLinkbaseCreateBox	"lb_create_box"
#define _DrmLinkbaseSaveBox	"lb_save_box"
#define _DrmLbCloseQuestBox	"lb_close_quest_box"
#define _DrmCautionBox          "env_caution_box"

#define _DwtWindowExpand	    "WindowExpand"
#define _DwtWindowExpandKeySym	    "ExpandMnemonic"
#define _DwtWindowCollapse	    "WindowCollapse"
#define _DwtWindowCollapseKeySym    "CollapseMnemonic"

#define _EnvRecordOnMenuLabel	"EnvRecordOnMenuLabel"
#define _EnvRecordOnKeySym	"EnvRecordOnMnemonic"
#define _EnvRecordOffMenuLabel  "EnvRecordOffMenuLabel"
#define _EnvRecordOffKeySym	"EnvRecordOffMnemonic"

#define _EnvActivateMenuLabel   "EnvActivateMenuLabel"
#define _EnvActivateKeySym	"EnvActivateMnemonic"
#define _EnvDeactivateMenuLabel "EnvDeactivateMenuLabel"
#define _EnvDeactivateKeySym	"EnvDeactivateMnemonic"

#define _DwtSvnTitleName	"SvnTitleName"
#define _DwtSvnTitleActive	"SvnTitleActive"
#define _DwtSvnTitleRecord	"SvnTitleRecord"
#define _DwtSvnTitleLinkbase	"SvnTitleLinkbase"
#define _DwtSvnNetworkName	"SvnLinknetName"
#define _DwtSvnPathName		"SvnPathName"
#define _DwtSvnUnnamed		"SvnUnnamed"

#define _DwtPropNetCreateBoxTitle	"prop_linknet_create_box_title"
#define _DwtPropNetListCreateBoxTitle	"prop_net_list_create_box_title"
#define _DwtPropPathCreateBoxTitle	"prop_path_create_box_title"
#define _DwtPropPathListCreateBoxTitle	"prop_path_list_create_box_title"
#define _DwtPropNetShowBoxTitle		"prop_linknet_show_box_title"
#define _DwtPropNetListShowBoxTitle	"prop_net_list_show_box_title"
#define _DwtPropPathShowBoxTitle	"prop_path_show_box_title"
#define _DwtPropPathListShowBoxTitle	"prop_path_list_show_box_title"
#define _DwtPropDateTimeFormat		"prop_date_time_xnl_format"

#define _DwtCautionCreate       "CautionCreate"
#define _DwtCautionSave         "CautionSave"

#define _DrmCompositeNetIcon	"DwtSvnNetListIcon"
#define _DrmNetworkIcon		"DwtSvnLinknetIcon"
#define _DrmCompositePathIcon	"DwtSvnPathListIcon"
#define _DrmPathIcon		"DwtSvnPathIcon"
#define _DrmEmptyIcon		"DwtSvnEmptyIcon"
#define _DrmSegmentIcon		"DwtSvnSegmentIcon"

#define _DwtSvnActiveCheckMarkIcon	"DwtSvnCheckMarkIcon"
#define _DwtSvnNotActiveCheckMarkIcon   "DwtSvnClearCheckMarkIcon"
#define _DwtSvnPartActiveCheckMarkIcon	"DwtSvnDimCheckMarkIcon"

#define _MrmDefaultAttributeObjectName "DefaultAttributeObjectName"
#define _MrmDefaultNetworkListName     "DefaultLinknetListName"
#define _MrmDefaultPathListName	       "DefaultPathListName"
#define _MrmDefaultActiveNetworkListName "DefaultActiveLinknetListName"
#define _MrmDefaultActivePathIndexName "DefaultActivePathIndexName"
#define _MrmDefaultActivePathListName  "DefaultActivePathListName"
#define _MrmDefaultCurrentNetworkName  "DefaultRecordingLinknetName"
#define _MrmDefaultTrailListName       "DefaultTrailListName"
#define _MrmDefaultNetworkName	       "DefaultLinknetName"
#define _MrmDefaultAuthor	       "DefaultAuthor"
#define _MrmLinkbaseName	       "LinkbaseName"

/*
**  Temporary window positioning
*/

#define _DefaultX	200
#define _DefaultY	300
#define _DefaultWidth	200
#define _DefaultHeight	100

#define _IconWidth	17
#define _IconHeight	17
#define _XOffset	0
#define _YOffset        0


/*
**  Window Svn Components
*/

#define _EnvSvnIconComponent		1
#define _EnvSvnNameComponent		2
#define _EnvSvnActivateComponent	3
#define _EnvSvnRecordComponent		4
#define _EnvSvnLinkbaseComponent	5

#define _LbSvnIconComponent		1
#define _LbSvnNameComponent		2
#define _LbSvnLinkbaseComponent	3

#define _SvnIconComponent		1   /* Generic Icon component */
#define _SvnNameComponent		2   /* Generic Name component */

/*
**  Menu states
*/

#define _MenuDim    0
#define _MenuOn	    1
#define _MenuOff    2

#define _NotActiveCheckMark	    0
#define _ActiveCheckMark	    1
#define _PartActiveCheckMark	    2

#define _NotActive	0
#define _Active		1
#define _PartActive	2

#define _NotRetrievable     0
#define _Retrievable        1
#define _LbNotRetrievable   2

#define _NetworkSegment	    1
#define _PathSegment	    2  

/*
**  Error messages array max size
*/

#define _MaxMessageCount 4

/*
** Error messages definitions
*/

#define _LbNewVersionMsg "MsgLbNewVersion"

/*
**  Type Definitions
*/

typedef enum __IsExpandable {
	_DimItem,
	_EmptyEntry,
	_ExpandEntry,
	_CollapseEntry
    } _IsExpandable;

typedef enum __InsertEntry {
	_EndOfList,
	_IntoList,
	_BeforeEntry,
	_BeforeInto
    } _InsertEntry;

/*
**  Shorthand for Toolkit reason code in callbacks
*/

typedef XmAnyCallbackStruct *_Reason;
#define HISSvnCallbackStruct DXmSvnCallbackStruct

/*
**  Hierarchy storage structure for the SVN display list.
*/

typedef struct __SvnData {
	struct __SvnData     *siblings;	    /* pointer to siblings or NULL  */
	struct __SvnData     *children;	    /* pointer to children or NULL  */
	struct __SvnData     *parent;	    /* pointer to parent or NULL  */
	_Integer	    child_level;    /* number level of children     */
	_Integer	    child_cnt;      /* number of children           */
	_Integer	    active;
	_Integer	    retrievable;
	_Integer	    segment;
	_HsObject	    object;	    /* the LinkWorks Manager object */
	_AnyPtr		    private;	    /* pointer to the window private */
	_AnyPtr		    specific;	    /* pointer to the svn specific  */
    } _SvnDataInstance, *_SvnData;

/*
** Storage structure for iteration.
*/

typedef	struct	__SvnIterateData{
    _SvnData	head_svn_data;
    _SvnData	end_list;
    } _SvnIterateDataInstance, *_SvnIterateData;

/*
** Storage structure for svn selection information.
*/

typedef struct __SelectData {
	    _Integer	count;
	    _Integer	*entry;
	    _Integer    *component;
	    _SvnData	*svn_data;
	} _SelectDataInstance, *_SelectData;
	
/*
** Properties box private data.
*/

typedef
    struct __PropPrivate {
	struct __PropPrivate *next; /* pointer to the next box */
	lwk_domain	domain;	    /* type of box (e.g., Network) */
	_Boolean	active;	    /* _True = box is active */

	_Window		window;	    /* parent window (e.g., Environment) */
	_HsObject	hs_object;  /* current object, if active */
	_Boolean	new_object; /* _True = object being created */

					/* widget ids: */
	Widget		box_widget;	/*   entire box */
	Widget		name_widget;	/*   name */
	Widget		desc_widget;	/*   description */
	Widget		author_widget;	/*   author */
	Widget		date_widget;	/*   creation date */
	Widget		extra_widget;	/*   extra info */
	Widget		ok_button;	/*   ok button	*/
	Widget		apply_button;	/*   apply button */
	Widget		reset_button;	/*   reset button */
	Widget		cancel_button;  /*   cancel button */
	
					/* change indicators */
	_Boolean	name_changed;	/*   name */
	_Boolean	desc_changed;	/*   description */
    } _PropPrivateInstance, *_PropPrivate;

/*
**  Window private data
*/

typedef struct __EnvWindowPrivate {
	    Widget	record_button;
	    Widget	activate_button;
	    Widget	update_button;
	    lwk_object 	current_network;
	    lwk_object 	trail;
	    _SvnData	networks_head;
    	    _SvnData	paths_head;
	    _SvnData	networks_end;
	    _SvnData	paths_end;
	    _CString	activate;
	    KeySym	activate_keysym;
	    _CString	deactivate;
	    KeySym	deactivate_keysym;
	    _CString	record_on;
	    KeySym	record_on_keysym;
	    _CString	record_off;
	    KeySym	record_off_keysym;
	    _AnyPtr	environment;		/* customize box */
	    _AnyPtr	linkworksmgr;		/* customize box */
	} _EnvWindowPrivateInstance, *_EnvWindowPrivate;

typedef struct __LbWindowPrivate {
	    Widget	save_button;
	    _Integer	svn_mode;
	    _SvnData	comp_net_list;
	    _SvnData	comp_path_list;
	    _SvnData	net_list;
	    _SvnData	path_list;
	    _SvnData	end_comp_net_list;
	    _SvnData	end_comp_path_list;
	    _SvnData	end_net_list;
	    _SvnData	end_path_list;
	} _LbWindowPrivateInstance, *_LbWindowPrivate;

typedef struct __WindowPrivate {
	    struct __WindowPrivate *next;
	    _AnyPtr	specific;
	    _Boolean	new_window;
	    _Integer	opened_entries;
	    _Window	window;
	    _SvnData	svn_entries;
	    _SvnData	deleted_entries;
	    _MapTable	map_table;
	    _CString	expand_label;
	    KeySym	expand_keysym;
	    _CString	collapse_label;
	    KeySym	collapse_keysym;
	    lwk_dxm_ui	his_dwui;
	    Widget	parent;		
	    Widget	shell;
	    Widget	main_widget;
	    Widget	menubar;
	    Widget	connection_menu;
	    Widget	svn;
	    Widget	fileselection;
	    Widget	lbcreate_box;
	    Widget	lbcreate_name;
	    Widget	lbcreate_file;
	    _String	lbcreate_file_name;
	    Widget	lbcreate_ok_button;
	    Widget	lbsave_box;
	    Widget	message_box;
	    Widget	help_box;
	    Opaque	help_context;
	    Widget      caution_box;
	    Widget      question_yes_btn;
	    Widget      question_no_btn;
	    Widget      question_label;
	    Widget      question_box;
	    Widget	cut_button;
	    Widget	copy_button;
	    Widget	paste_button;
	    Widget	delete_button;
	    Widget	expand_button;
	    Widget	showprop_button;
	    _FontList	small_font;
	    _PropPrivate    linkbase_prop;
	    _PropPrivate    comp_net_prop;
	    _PropPrivate    network_prop;
	    _PropPrivate    comp_path_prop;
	    _PropPrivate    path_prop;
	} _WindowPrivateInstance, *_WindowPrivate;


/*
**  External Routine Declarations
*/

/*
** Environment window Svn public routines.
*/

_DeclareFunction(_Void EnvSvnEnvWindowInitialize,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnEnvWindowLoad,
    (_WindowPrivate private, _SvnData *entry,
    _HsObject envcontext));

_DeclareFunction(_Void EnvSvnEnvWindowDisplayEnv,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnEnvWindowExpand,
    (_WindowPrivate private, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnEnvWindowToggleRecording,
    (_WindowPrivate private, _SelectData select_data));

_DeclareFunction(_Void EnvSvnEnvWindowToggleActivate,
    (_WindowPrivate private, _SelectData select_data));

_DeclareFunction(_Void EnvSvnEnvWindowRemoveEntry,
    (_WindowPrivate private, _SvnData svn_data, Time timestamp));

_DeclareFunction(_Void EnvSvnEnvWindowInsertEntry,
    (_WindowPrivate private, lwk_object object, lwk_domain domain));

_DeclareFunction(_Void EnvSvnEnvWindowAddActions,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnEnvWindowUpdateEntry,
    (_WindowPrivate private, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnEnvWindowEntryTransfer,
    (Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));

_DeclareFunction(_Void EnvSvnEnvWindowSelectionsDrag,
    (Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));

/*
** Linkbase window svn public routines.
*/

_DeclareFunction(_Void EnvSvnLbWindowInitialize,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnLbWindowLoadLinkbase,
    (_WindowPrivate private, _SvnData *lb_entry, _HsObject linkbase));

_DeclareFunction(_Void EnvSvnLbWindowDisplayLb,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnLbWindowExpand,
    (_WindowPrivate private, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnLbWindowRemoveEntry,
    (_WindowPrivate private, _SvnData svn_data, Time timestamp));

_DeclareFunction(_Void EnvSvnLbWindowInsertEntry,
    (_WindowPrivate private, lwk_object object, lwk_domain domain, _Boolean
    existing));

_DeclareFunction(_Void EnvSvnLbWindowAddActions,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnLbWindowEntryTransfer,
    (Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));

_DeclareFunction(_Void EnvSvnLbWindowSelectionsDrag,
    (Widget w, _WindowPrivate private, DXmSvnCallbackStruct *cb));


/*
** Svn Window public routines.
*/

_DeclareFunction(_SvnData EnvSvnWindowCreateSvnData,
    (_WindowPrivate private, _SvnData parent, _Integer child_level));

_DeclareFunction(_Void EnvSvnWindowFreeSvnData,
    (_SvnData svn_data));
_DeclareFunction(_Void EnvSvnWindowFreeSvnSiblData,
    (_SvnData svn_data));

_DeclareFunction(_SvnData EnvSvnWindowInsertBeforeInto,
    (_SvnData ref_svn_data, _SvnData svn_data));

_DeclareFunction(_SvnData EnvSvnWindowInsertBefore,
    (_SvnData *head_svn_data, _SvnData ref_svn_data, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnWindowInsertEndOfList,
    (_SvnData *head_svn_data, _SvnData svn_data, _Boolean list));

_DeclareFunction(_SvnData EnvSvnWindowInsertInto,
    (_SvnData ref_svn_data, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnWindowInsertAfter,
    (_SvnData *end_list, _SvnData svn_data));

_DeclareFunction(_Void EnvSvnWindowRetrieveEntry,
    (_SvnData svn_data, _Callback iterate_call));

_DeclareFunction(_Void EnvSvnWindowRemoveEntry,
    (_WindowPrivate private, _SvnData *list, _SvnData
    *end_list, _SvnData svn_data, _Boolean free));

_DeclareFunction(_Void EnvSvnWindowDisplayEntry,
    (_WindowPrivate private, _SvnData previous_entry, _SvnData entry, _Callback
    set_entry));

_DeclareFunction(_Void EnvSvnWindowClearDisplay,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvSvnWindowSelection,
    (Widget w, _WindowPrivate private, HISSvnCallbackStruct *cb));

_DeclareFunction(_Void EnvSvnWindowUnSelection,
    (Widget w, _WindowPrivate private, HISSvnCallbackStruct *cb));

_DeclareFunction(_Void EnvSvnWindowExpand,
    (_WindowPrivate private, _SvnData svn_data, _Callback set_entry));

_DeclareFunction(_Void EnvSvnWindowCollapse,
    (_WindowPrivate private, _SvnData svn_data));

_DeclareFunction(_Boolean EnvSvnWindowGetSelection,
    (_WindowPrivate private, _GetSelectOperation operation,
     _SelectData *select_data));

_DeclareFunction(_Void EnvSvnWindowFreeSelection,
    (_SelectData selection));

_DeclareFunction(_Void EnvSvnWindowSelect,
    (_WindowPrivate private, _HsObject hs_object));

_DeclareFunction(_Void EnvSvnWindowSetHighlight,
    (_WindowPrivate private, _Boolean turn_on));

_DeclareFunction(_IsExpandable EnvSvnWindowIsExpandable,
    (_WindowPrivate private, _SvnData svn_data, _Integer entry_number,
     _Integer component));

_DeclareFunction(_SvnData EnvSvnWindowCreateHsObject,
    (_SvnData head_svn_data, lwk_domain domain, lwk_object *object));

_DeclareFunction(Pixmap EnvSvnWindowSetPixmap,
	(_WindowPrivate private, lwk_domain domain));

_DeclareFunction(_Boolean EnvSvnWindowSetIndex,
	(lwk_domain domain));

_DeclareFunction(_Void EnvSvnWindowGetSelectedEntry,
	(_Widget svn, _SvnData *entry));
	
_DeclareFunction(_Void EnvSvnWindowSetEntriesNonRetr,
	(_WindowPrivate private, _SvnData *entries, _Integer count));

	
/*
** Environment window DW public routines.
*/

_DeclareFunction(_Void EnvOpDWEnvWindowDelete,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvOpDWEnvWindowSetCurrency,
    (_WindowPrivate private, _CurrencyFlag currency, _AnyPtr value));

_DeclareFunction(_Void EnvOpDWEnvWindowClose,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvOpDWEnvWindowDisplay,
    (_WindowPrivate private, _EnvContext envcontext));

_DeclareFunction(_Void EnvDWEnvWinUpdateCurrencyCNet,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvDWEnvWinUpdateCurrencyCPath,
    (_WindowPrivate private));

_DeclareFunction(_Boolean EnvDWEnvWindowCheckPathState,
    (_EnvWindowPrivate private, _SvnData entry));

/*
** Linkbase window DW public routines.
*/

_DeclareFunction(_WindowPrivate EnvOpDWLbWindowCreate,
    (_LbWindow lbwindow, _Widget parent));

_DeclareFunction(_Void EnvOpDWLbWindowOpen,
    (_WindowPrivate private, _Cardinal width, _Cardinal height, _Position x,
     _Position y));

_DeclareFunction(_Void EnvOpDWLbWindowDelete,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvOpDWLbWindowClose,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvOpDWLbWindowDisplay,
    (_WindowPrivate private, _HsObject linkbase));

_DeclareFunction(_Void EnvDWLbWindowOpenLinkbase,
    (_WindowPrivate private, _HsObject object));

_DeclareFunction(_Void EnvDWLbWindowOpenLbInWindow,
    (_WindowPrivate private, lwk_linkbase linkbase));


/*
** General DW public routines.
*/

_DeclareFunction(_Void EnvDwSetCursor,
    (Widget widget, _CursorType cursor_type));

_DeclareFunction(_Void EnvDWGetStringLiteral,
    (_String identifier, _String *literal));

_DeclareFunction(_Void EnvDWGetCStringLiteral,
    (_String identifier, _CString *literal));

_DeclareFunction(_Void EnvDWGetKeySym,
    (_String identifier, KeySym *key));

_DeclareFunction(_Void EnvDWGetIntegerValue,
    (_String identifier, _Integer *value));

_DeclareFunction(MrmHierarchy EnvDWOpenDRMHierarchy,
    (_String filename));

_DeclareFunction(_Void EnvDWCloseDRMHierarchy,
    (MrmHierarchy hierarchy));

_DeclareFunction(_Void EnvDWRegisterDRMNames,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvDWLbBox,
    (_WindowPrivate private, _Boolean new));

_DeclareFunction(_Void EnvDWCaution,
    (_WindowPrivate private, _CString label,
    _Callback callback_proc, _AnyPtr user_data, _Boolean cancel));

_DeclareFunction(_Void EnvDWQuestion,
    (_WindowPrivate private, _Callback yes_callback, _Callback no_callback,
     _String identifier));

_DeclareFunction(_Void EnvDWShowPropDismiss,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvDWFetchIcon,
    (_WindowPrivate private, _String icon_literal, Pixmap *icon,
     _Boolean BW));

_DeclareFunction(_Void EnvDWFetchBitmap,
    (Widget shell, _String icon_literal, Pixmap *icon));

_DeclareFunction(_Void EnvDWSetExpandButtonLabel,
    (_WindowPrivate private, _SelectData select_data));

_DeclareFunction(_Void EnvDWSetEditButtons,
    (_WindowPrivate private, _Boolean sensitive));

_DeclareFunction(_Void EnvDWGetTime,
    (_Reason reason, Time *timestamp));

_DeclareFunction(_Boolean EnvDWIsXUIWMRunning,
    (Widget widget));

_DeclareFunction(_Void EnvDWFitFormDialogOnScreen,
    (Widget formdialog));

_DeclareFunction(_Void EnvDWGetObjectName,
    (_SvnData svn_data, _DDIFString *ddif_name));

_DeclareFunction(_Void EnvDWGetObjectEncoding,
    (_SvnData svn_data, lwk_encoding *encoding, lwk_integer *encoding_size));

_DeclareFunction(_String EnvDWGetIconIndexName,
    (Display *dpy, _String root_icon_literal, unsigned int *icon_size_rtn,
    char **supported_icon_sizes, int num_supported_sizes));

_DeclareFunction(_Void EnvDWSetTitle,
    (Widget widget, _CString name));

_DeclareFunction(_Void EnvDWSetIconName,
    (Widget widget, _CString name));


/*
**  Miscellaneous routines
*/

#define _StringToXmString(String) EnvDWStringToXmString((String))

_DeclareFunction(XmString EnvDWStringToXmString,
    (char *string));

/*
**  Help routines
*/

_DeclareFunction(Widget EnvDwMessageGetToplevel,());

/*
**  Help routines
*/

_DeclareFunction(_Void EnvHelpPopupHelp,
    (Widget w, _CString topic, _Reason reason));

_DeclareFunction(_Void EnvHelpOnContextTracking,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(_Void EnvHelpSvnHelp,
    (Widget w, _CString topic, HISSvnCallbackStruct *cb));
    

/*
** Clipboard routines.
*/

_DeclareFunction(_Void EnvDWClipboardCopyToClipboard,
    (_WindowPrivate private, _SelectData select_data, Time timestamp));

_DeclareFunction(_Void EnvDWClipboardCopyFromClipboard,
    (_WindowPrivate private, lwk_object *object, Time timestamp));

_DeclareFunction(_Boolean EnvDWClipboardInquireNextPaste,
    (_WindowPrivate private, Time timestamp));

/*
** Global selection support routines.
*/

_DeclareFunction(_Void EnvDWQuickCopyInitialize,
    (_WindowPrivate private));

_DeclareFunction(_Void EnvDWQuickCopyAddHandler,
    (_WindowPrivate private, XtActionsRec *action_table, int table_size));

_DeclareFunction(_Boolean EnvDWQuickCopyGetSelection,
    (_Widget widget, Time time, lwk_object *object, _Boolean move));

_DeclareFunction(_Boolean EnvDWQuickCopyConvertSelection,
    (_Widget widget, Atom *selection, Atom *target, Atom *type, caddr_t *value,
    int  *length, int *format));

_DeclareFunction(_Void EnvDWQuickCopyLoseSelection,
    (_Widget widget, Atom *selection));

_DeclareFunction(_Void EnvDWQuickCopyConversionDone,
    (_Widget widget, Atom *selection, Atom *target));

_DeclareFunction(_Void EnvDWQuickCopyDisown,
    (_WindowPrivate private, Time timestamp));

/*
**  Display message
*/

#define _DisplayRootMessage(Status, Count, Severity) \
    EnvDWRootMessageBox((Status), (Count), (Severity))

_DeclareFunction(_Void EnvDWRootMessageBox,
    (hs_status *status, _Integer count, _MsgSeverityFlag Severity));

/*
**  Position widget by offset
*/

_DeclareFunction(_Void PositionWidgetOffset, (Widget parent, Widget child));

/*
**  Special macro definitions
*/

#define _GetEnvWindowPrivate EnvEnvWindowGetEnvWindowPrivate()

_DeclareFunction(_AnyPtr EnvEnvWindowGetEnvWindowPrivate, (_Void));

/*
** Main UID file open routine
*/

_DeclareFunction(_Void EnvDwOpenMainHierarchy, ());
_DeclareFunction(MrmHierarchy EnvDwGetMainHierarchy, ());

