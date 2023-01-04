/* dwc_ui_memex.c */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Denis G. Lacroix	October 1989
**
**  ABSTRACT:
**
**	This is the module which contains miscellaneous support routines
**	for the MEMEX integration support
**
*--
*/

#if MEMEX
/*
**  Include Files
*/
#include "dwc_compat.h"

#include <string.h>
#include <stdio.h>

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <Xm/TextP.h>
#include <Xm/Text.h>
#include <DXm/DXmCSTextP.h>
#include <DXm/DXmCSText.h>
#include <Xm/SashP.h>
#include <X11/DECwI18n.h>
#include <DXm/DECspecific.h>
#ifdef vaxc
#pragma standard
#endif

#include "dwc_ui_dayslotswidgetp.h"	/* for DwcDswEntry */
#include "dwc_ui_timeslotwidgetp.h"
#include "dwc_ui_calendar.h"
#include "dwc_ui_memex.h"	
#include "dwc_ui_clipboard.h"		/* for CLIPTestSelectionWidget */
#include "dwc_ui_day.h"			/* for DAYIconsForDayItem */
#include "dwc_ui_file.h"		/* for FILECloseCalendar */
#include "dwc_ui_monthwidget.h"		/* for MWSetSelection */
#include "dwc_ui_uil_values_const.h"	/* for dwc_k_memex_timeslot_format  */
#include "dwc_ui_datestructures.h"	/* for dtb */
#include "dwc_ui_misc.h"		/* for MISCChangeView */
#include "dwc_ui_dateformat.h"		/* for DATEFORMATTimeToCS */
#include "dwc_ui_errorboxes.h"		/* for ERRORDisplayText... */
#include "dwc_ui_datefunctions.h"

extern	DwcWmChangeStateAtom;		/* setup in dwc_ui_calendar.h */


static lwk_status MEMEX_get_surr_cb PROTOTYPE ((
	lwk_ui			dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_object		*surrogates));

static Boolean MEMEX_get_surrs_for_sel_entries PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot));

static Boolean MEMEX_get_surrs_for_sel_entry PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot,
	int			item_id));

static Boolean MEMEX_get_surr_for_day PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot));

static Boolean MEMEX_get_surr_for_sel_month PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr));

static Boolean MEMEX_get_surr_for_month PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot));

static Boolean MEMEX_get_surr_for_year PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot));

static lwk_status MEMEX_create_surr_cb PROTOTYPE ((
	lwk_ui			dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_object		*surrogate));

static Boolean MEMEX_create_surr_for_sel_entry PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr));

static Boolean MEMEX_create_surr_for_day PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot));

static Boolean MEMEX_create_surr_for_sel_month PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr));

static Boolean MEMEX_create_surr_for_month PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot));

static Boolean MEMEX_create_surr_for_year PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot));

static lwk_status MEMEX_close_view_cb PROTOTYPE ((
	lwk_ui			dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data));

static lwk_status MEMEX_connect_cb PROTOTYPE ((
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_link		connection));

static void MEMEX_remove_icons_from_timeslots PROTOTYPE ((CalendarDisplay cd));

static lwk_status MEMEX_currency_change_cb PROTOTYPE ((
	lwk_ui			dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_integer		currency));

static void MEMEX_set_shell_iconstate PROTOTYPE ((
	Widget			toplevel,
	Widget			shell,
	long			state));

static lwk_closure MEMEX_surrogate_found_cb PROTOTYPE ((
	lwk_closure		closure,
	lwk_composite_linknet	composite_net,
	lwk_integer		domain,
	lwk_surrogate		*surrogate_ptr));

static Boolean MEMEX_highlight_tse PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_integer	follow_type,
	lwk_surrogate	surrogate,
	char		*filename,
	int		dsbot,
	int		item_id));

static Boolean MEMEX_change_view PROTOTYPE ((
	CalendarDisplay	cd,
	lwk_integer	follow_type,
	CalendarDisplay	*new_cd_ptr,
	show_kind	which_display,
	char		*filename,
	int		dsbot));

static void MEMEX_del_dangle_cb PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs));

static lwk_termination MEMEX_del_connection_cb PROTOTYPE ((
	lwk_closure	closure,
	lwk_object	set,
	lwk_domain	domain,
	lwk_any_pointer	*connection));

static void MEMEX_add_icon_to_timeslot PROTOTYPE ((
	CalendarDisplay	cd,
	int		index,
	lwk_surrogate	surrogate));

static Boolean MEMEX_get_surrs_for_today PROTOTYPE ((
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot));

static Boolean MEMEX_query_surrs PROTOTYPE ((
	lwk_dxm_ui		his_dwui,
	lwk_query_node		*query_expression,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	Widget			error_parent));

static lwk_termination MEMEX_del_linknet PROTOTYPE ((
	lwk_closure null,
	lwk_composite_linknet cnet,
	lwk_integer domain,
	lwk_linknet *linknet));

/*									
**  Static Data Definitions
*/

static	Boolean	hyperinitialized = TRUE;


/*
**  Query #1: this is a fully qualified query where everything is specified,
**  including: filename, dsbot and item id. The value of these properties
**  will be set at run time.
*/

static lwk_query_node dsbot_prop_sn_1 =
    {lwk_c_property_value, DWC_DSBOT_PROPERTY, 0};
static lwk_query_node dsbot_val_sn_1 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node dsbot_sn_1 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &dsbot_prop_sn_1,
     (lwk_any_pointer) &dsbot_val_sn_1};

static lwk_query_node item_id_prop_sn_1 =
    {lwk_c_property_value, DWC_ITEM_ID_PROPERTY, 0};
static lwk_query_node item_id_val_sn_1 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node item_id_sn_1 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &item_id_prop_sn_1,
     (lwk_any_pointer) &item_id_val_sn_1};

static lwk_query_node dsbot_item_id_sn_1 =
    {lwk_c_and, (lwk_any_pointer) &dsbot_sn_1, (lwk_any_pointer) &item_id_sn_1};

static lwk_query_node filename_prop_sn_1 =
    {lwk_c_property_value, DWC_FILE_PROPERTY, 0};
static lwk_query_node filename_val_sn_1 =
    {lwk_c_string_literal, 0, 0};
static lwk_query_node filename_sn_1 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &filename_prop_sn_1,
     (lwk_any_pointer) &filename_val_sn_1};

static lwk_query_node filename_dsbot_item_id_sn_1 =
    {lwk_c_and, (lwk_any_pointer) &filename_sn_1,
     (lwk_any_pointer) &dsbot_item_id_sn_1};

static lwk_query_node type_prop_sn_1 =
    {lwk_c_property_value, DWC_SURROGATE_TYPE, 0};
static lwk_query_node type_val_sn_1 =
    {lwk_c_string_literal, DWC_MEMEX_TIMESLOT_ENTRY, 0};
static lwk_query_node type_sn_1 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &type_prop_sn_1,
     (lwk_any_pointer) &type_val_sn_1};

static lwk_query_node type_filename_dsbot_item_id_sn_1 =
    {lwk_c_and, (lwk_any_pointer) &type_sn_1,
     (lwk_any_pointer) &filename_dsbot_item_id_sn_1};

static lwk_query_node fully_qualified_query_sn_1 =
    {lwk_c_has_properties, (lwk_any_pointer) &type_filename_dsbot_item_id_sn_1,
     0};

/*
**  Query #2:
*/

static lwk_query_node dsbot_prop_sn_2 =
    {lwk_c_property_value, DWC_DSBOT_PROPERTY, 0};
static lwk_query_node dsbot_val_sn_2 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node dsbot_sn_2 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &dsbot_prop_sn_2,
     (lwk_any_pointer) &dsbot_val_sn_2};

static lwk_query_node filename_prop_sn_2 =
    {lwk_c_property_value, DWC_FILE_PROPERTY, 0};
static lwk_query_node filename_val_sn_2 =
    {lwk_c_string_literal, 0, 0};
static lwk_query_node filename_sn_2 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &filename_prop_sn_2,
     (lwk_any_pointer) &filename_val_sn_2};

static lwk_query_node filename_dsbot_sn_2 =
   {lwk_c_and, (lwk_any_pointer) &filename_sn_2,
    (lwk_any_pointer) &dsbot_sn_2};

static lwk_query_node type_prop_sn_2 =
    {lwk_c_property_value, DWC_SURROGATE_TYPE, 0};
static lwk_query_node type_val_sn_2 =
    {lwk_c_string_literal, DWC_MEMEX_DAY, 0};
static lwk_query_node type_sn_2 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &type_prop_sn_2,
     (lwk_any_pointer) &type_val_sn_2};

static lwk_query_node type_filename_dsbot_sn_2 =
    {lwk_c_and, (lwk_any_pointer) &type_sn_2,
     (lwk_any_pointer) &filename_dsbot_sn_2};

static lwk_query_node fully_qualified_query_sn_2 =
    {lwk_c_has_properties, (lwk_any_pointer) &type_filename_dsbot_sn_2, 0};

/*
**  Query #3:
*/

static lwk_query_node dsbot_prop_sn_3 =
    {lwk_c_property_value, DWC_DSBOT_PROPERTY, 0};
static lwk_query_node dsbot_val_sn_3 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node dsbot_sn_3 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &dsbot_prop_sn_3,
     (lwk_any_pointer) &dsbot_val_sn_3};

static lwk_query_node filename_prop_sn_3 =
    {lwk_c_property_value, DWC_FILE_PROPERTY, 0};
static lwk_query_node filename_val_sn_3 =
    {lwk_c_string_literal, 0, 0};
static lwk_query_node filename_sn_3 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &filename_prop_sn_3,
     (lwk_any_pointer) &filename_val_sn_3};

static lwk_query_node filename_dsbot_sn_3 =
   {lwk_c_and, (lwk_any_pointer) &filename_sn_3,
    (lwk_any_pointer) &dsbot_sn_3};

static lwk_query_node type_prop_sn_3 =
    {lwk_c_property_value, DWC_SURROGATE_TYPE, 0};
static lwk_query_node type_val_sn_3 =
    {lwk_c_string_literal, DWC_MEMEX_MONTH, 0};
static lwk_query_node type_sn_3 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &type_prop_sn_3,
     (lwk_any_pointer) &type_val_sn_3};

static lwk_query_node type_filename_dsbot_sn_3 =
    {lwk_c_and, (lwk_any_pointer) &type_sn_3,
     (lwk_any_pointer) &filename_dsbot_sn_3};

static lwk_query_node fully_qualified_query_sn_3 =
    {lwk_c_has_properties, (lwk_any_pointer) &type_filename_dsbot_sn_3, 0};

/*
**  Query #4:
*/

static lwk_query_node dsbot_prop_sn_4 =
    {lwk_c_property_value, DWC_DSBOT_PROPERTY, 0};
static lwk_query_node dsbot_val_sn_4 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node dsbot_sn_4 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &dsbot_prop_sn_4,
     (lwk_any_pointer) &dsbot_val_sn_4};

static lwk_query_node filename_prop_sn_4 =
    {lwk_c_property_value, DWC_FILE_PROPERTY, 0};
static lwk_query_node filename_val_sn_4 =
    {lwk_c_string_literal, 0, 0};
static lwk_query_node filename_sn_4 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &filename_prop_sn_4,
     (lwk_any_pointer) &filename_val_sn_4};

static lwk_query_node filename_dsbot_sn_4 =
   {lwk_c_and, (lwk_any_pointer) &filename_sn_4, (lwk_any_pointer) &dsbot_sn_4};

static lwk_query_node type_prop_sn_4 =
    {lwk_c_property_value, DWC_SURROGATE_TYPE, 0};
static lwk_query_node type_val_sn_4 =
    {lwk_c_string_literal, DWC_MEMEX_YEAR, 0};
static lwk_query_node type_sn_4 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &type_prop_sn_4,
     (lwk_any_pointer) &type_val_sn_4};

static lwk_query_node type_filename_dsbot_sn_4 =
    {lwk_c_and, (lwk_any_pointer) &type_sn_4,
     (lwk_any_pointer) &filename_dsbot_sn_4};

static lwk_query_node fully_qualified_query_sn_4 =
    {lwk_c_has_properties, (lwk_any_pointer) &type_filename_dsbot_sn_4, 0};


/*
**  Query #5:
**
**	asks for all the timeslot surrogates in a given Calendar database
**	and for a given day (dsbot)
*/

static lwk_query_node dsbot_prop_sn_5 =
    {lwk_c_property_value, DWC_DSBOT_PROPERTY, 0};
static lwk_query_node dsbot_val_sn_5 =
    {lwk_c_integer_literal, 0, 0};
static lwk_query_node dsbot_sn_5 =
    {lwk_c_integer_is_eql, (lwk_any_pointer) &dsbot_prop_sn_5,
     (lwk_any_pointer) &dsbot_val_sn_5};

static lwk_query_node filename_prop_sn_5 =
    {lwk_c_property_value, DWC_FILE_PROPERTY, 0};
static lwk_query_node filename_val_sn_5 =
    {lwk_c_string_literal, 0, 0};
static lwk_query_node filename_sn_5 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &filename_prop_sn_5,
     (lwk_any_pointer) &filename_val_sn_5};

static lwk_query_node filename_dsbot_sn_5 =
   {lwk_c_and, (lwk_any_pointer) &filename_sn_5, (lwk_any_pointer) &dsbot_sn_5};

static lwk_query_node type_prop_sn_5 =
    {lwk_c_property_value, DWC_SURROGATE_TYPE, 0};
static lwk_query_node type_val_sn_5 =
    {lwk_c_string_literal, DWC_MEMEX_TIMESLOT_ENTRY, 0};
static lwk_query_node type_sn_5 =
    {lwk_c_string_is_eql, (lwk_any_pointer) &type_prop_sn_5,
     (lwk_any_pointer) &type_val_sn_5};

static lwk_query_node type_filename_dsbot_sn_5 =
    {lwk_c_and, (lwk_any_pointer) &type_sn_5,
     (lwk_any_pointer) &filename_dsbot_sn_5};

static lwk_query_node fully_qualified_query_sn_5 =
    {lwk_c_has_properties, (lwk_any_pointer) &type_filename_dsbot_sn_5, 0};


/*
**  Global Data Definitions
*/


/*
**  External Data Declarations
*/

extern AllDisplaysRecord ads;


lwk_status MEMEXCreateDwUi
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    {
    lwk_status	    status;
    lwk_set	    surrogate_types_set,
		    operations_set;
    lwk_callback    callback;
    lwk_integer	    highlight_state;
    lwk_object	    current_net;
    XmString	    application_name;
    lwk_string	    status_text;
    MrmCode	    type;

    /*
    **	Create the DECwindows User Interface; the relevant object id is
    **	stored in a cd.
    */
#if 0
    application_name = XmStringCreateSimple("Calendar");
#else
    MrmFetchLiteral
    (
	cd->ads->hierarchy,
	"dwc_t_calendar_title_text_nc",
	XtDisplay(cd->toplevel),
	(caddr_t *)&application_name,
	&type  			/* It's a compound string */
    );
#endif

    status = lwk_create_dxm_ui((lwk_any_pointer) application_name,
	lwk_c_true, lwk_c_true, (lwk_any_pointer) cd->mainwid,
	(lwk_any_pointer) cd->pull_memex, &cd->hisdwui);

    XmStringFree(application_name);

	if (status != lwk_s_success) {

            /*	  
	    **  Explicitly set this to 0 so we don't choke on a garbage value
	    **	that may have been in there from before.
	    */	  
            cd->hisdwui = (lwk_dxm_ui)0;


            /*	  
	    **  Set a flag so we don't print out error messages more than once
	    */	  
            hyperinitialized = FALSE;

	    return status;
	}

    /*	  
    **  Let's see if hypersession set up the right properties on the root window
    **	so that we can do connections and stuff.
    */	  
    status = lwk_get_value(cd->hisdwui, lwk_c_p_recording_linknet,
			    lwk_c_domain_integer,
			    (lwk_any_pointer)&current_net);
    if (status != lwk_s_success) {
        /*	  
	**  Hypersession must not have run. Don't bother reporting it since we
	**  don't want to annoy the users if they didn't intend to use it. VMS
	**  QAR 548 in DECW-V3-INTERNAL qar db.
	*/	  
        hyperinitialized = FALSE;
    }

    /*
    **  Set the $CurrentHighlighting property of the DwUi to be the
    **	$DefaultHighlighting of the DwUi.
    */

    status = lwk_get_value(cd->hisdwui, lwk_c_p_default_highlight,
	lwk_c_domain_integer, (lwk_any_pointer) &highlight_state);
	
    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorGetDefHighFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    status = lwk_set_value(cd->hisdwui, lwk_c_p_appl_highlight,
	lwk_c_domain_integer, (lwk_any_pointer) &highlight_state,
	lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCurHighFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    cd->default_highlighting = TRUE;

    /*
    **	Register the Calendar surrogate types
    */

    status = lwk_create_set(lwk_c_domain_string, 0, &surrogate_types_set);
    status = lwk_add_string(surrogate_types_set, DWC_SURROGATE);
    status = lwk_set_value(cd->hisdwui, lwk_c_p_supported_surrogates,
	lwk_c_domain_set, (lwk_any_pointer) &surrogate_types_set,
	lwk_c_set_property);
	
    status = lwk_delete(&surrogate_types_set);

    /*
    **	Register Calendar operation types
    */

    status = lwk_create_set(lwk_c_domain_string, 0, &operations_set);
    status = lwk_add_string(operations_set,  DWC_MEMEX_VIEW_ENTRY);
    status = lwk_set_value(cd->hisdwui, lwk_c_p_supported_operations,
	 lwk_c_domain_set, (lwk_any_pointer) &operations_set,
	 lwk_c_set_property);
	 
    status = lwk_delete(&operations_set);

    /*
    **	Register the user data property to be a calendar display structure
    */

    status = lwk_set_value(cd->hisdwui, lwk_c_p_user_data,
	lwk_c_domain_closure, (lwk_any_pointer) &cd, lwk_c_set_property);

    /*
    **	Register the callbacks
    */

    callback = (lwk_callback) MEMEX_get_surr_cb;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_get_surrogate_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);
	
    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    callback = (lwk_callback) MEMEX_create_surr_cb;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_create_surrogate_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    callback = (lwk_callback) MEMEX_close_view_cb;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_close_view_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    callback = (lwk_callback) MEMEXApplyCallback;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_apply_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    callback = (lwk_callback) MEMEX_connect_cb;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_complete_link_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    callback = (lwk_callback) MEMEX_currency_change_cb;
    status = lwk_set_value(cd->hisdwui, lwk_c_p_environment_change_cb,
	lwk_c_domain_routine, (lwk_any_pointer) &callback, lwk_c_set_property);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorSetCallbackFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    /*
    **  That's it
    */
    return status;

}


/*	  
**  Delete the MEMEX stuff for this calendar display
*/	  
void MEMEXDeleteDwUi
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    lwk_status	    status;

    status = lwk_delete(&cd->hisdwui);
}

static lwk_status MEMEX_get_surr_cb
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_object		*surrogates_ptr)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data, surrogates_ptr)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
	lwk_object		*surrogates_ptr;
#endif	/* prototype */
    {
    CalendarDisplay	cd = (CalendarDisplay)user_data;
    DwcMemexSurrogateList	*surrogate_list_head_ptr = NULL;
    DwcMemexSurrogateList	*surrogate_list_member_ptr = NULL;
    lwk_status		status,
			return_code;
    int			dsbot,
			selected_dsbot;

#ifdef DEBUG
    printf("GetSurrogate Callback\n");
#endif

    return_code = lwk_s_success;

    /*
    **  What's the guy seeing here anyway...
    */
    
    switch (cd->showing)
	{
	case show_day:
	
	    /*
	    **	We are looking at a day view
	    */
	    
	    if (MEMEX_get_surrs_for_sel_entries
		(cd, &surrogate_list_head_ptr, 	cd->dsbot))
	    {
		
		status = lwk_s_success;
	    }
	    else if ((cd->cb->select_type != MwNothingSelected) &&
		MEMEX_get_surr_for_sel_month(cd, &surrogate_list_head_ptr)) {
		
		status = lwk_s_success;
	    }
	    
	    else if (MEMEX_get_surr_for_day
		(cd, &surrogate_list_head_ptr, cd->dsbot))
	    {
		
		status = lwk_s_success;
	    }
	    
	    break;
	
	case show_month:
	    /*
	    **
	    */	
	    if ( MEMEX_get_surr_for_sel_month(cd, &surrogate_list_head_ptr) )
		{
		status = lwk_s_success;
		}		
	    break;
	
	case show_year:
	    /*
	    **
	    */
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, cd->yd->first_month, cd->yd->first_year);
	    if ( (cd->cb->select_type != MwNothingSelected) &&
		MEMEX_get_surr_for_sel_month(cd, &surrogate_list_head_ptr) )
		{
		status = lwk_s_success;		
		}
	    else if ( MEMEX_get_surr_for_year(cd, &surrogate_list_head_ptr,
		selected_dsbot) )
		{
		status = lwk_s_success;
		}		
	    break;
	
	case show_week:
	    printf("? Get surrogate for week surrogate not implemented\n");
	    status = lwk_s_failure;
	    break;
	
	default:
	    ERRORDisplayError(cd->mainwid, "ErrorUnkSurrogate");
	    status = lwk_s_failure;
	    break;
	}

    /*
    **  Things come back in a list; we need to package this
    **  into a set and hand it over to the caller
    */

    surrogate_list_member_ptr = surrogate_list_head_ptr;

    if (surrogate_list_member_ptr != NULL) {

	status = lwk_create_set(lwk_c_domain_surrogate, 3, surrogates_ptr);
	
	while (surrogate_list_member_ptr != NULL)
	    {
	    status = lwk_add_element((lwk_list) *surrogates_ptr, lwk_c_domain_surrogate,
		(lwk_any_pointer) &surrogate_list_member_ptr->surrogate);
	    surrogate_list_member_ptr =
		(DwcMemexSurrogateList *)surrogate_list_member_ptr->next;
	    }
    }
    else {
	*surrogates_ptr = lwk_c_null_object;
    }

    /*
    **  Free the internal list of surrogates
    */

    surrogate_list_member_ptr = surrogate_list_head_ptr;

    while (surrogate_list_member_ptr != NULL) {
	surrogate_list_head_ptr =
		(DwcMemexSurrogateList *)surrogate_list_member_ptr->next;
	XtFree((char *) surrogate_list_member_ptr);
	surrogate_list_member_ptr = surrogate_list_head_ptr;
    }
				
    /*
    **  That's it
    */

    return (return_code);

    }


static Boolean MEMEX_get_surrs_for_sel_entries
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
#endif	/* prototype */
{
    Time		    time;
    Widget		    select_widget;
    lwk_status		    return_code;
    int			    i;
    DwcDswEntry		    dsw_entry;
    DSWEntryDataStruct   dsw_entry_data;
    DwcDayItem		    di;

    return_code = lwk_s_failure;

    if ( CLIPTestSelectionWidget(cd, &select_widget) )
    {
	/*
	**  CLIPTestSelectionWidget returns TRUE if one of our custom
	**  widgets or text widgets own the selection.
	*/	
	if (select_widget == NULL)
	{
	    /*
	    **	It's one of our custom widgets which owns the selection,
	    **	not a text widget
	    */
	    for (i=0; i < cd->number_of_selected_entries; i++)
	    {
		dsw_entry = cd->selected_entries [i];
		DSWGetEntryData(dsw_entry, &dsw_entry_data);
		di = (DwcDayItem)dsw_entry_data.tag;

		/*	  
		**  New entry, hasn't yet been saved (ie not item number) so save it
		**	for the user.
		*/	  
		if (di == 0)
		{
		    time = MISCGetTimeFromAnyCBS (NULL);
		    (void)DSWRequestCloseOpenEntry
			((DayslotsWidget) dsw_entry_data.parent, time);
		    if (cd->number_of_selected_entries == 0)
		    /*	  
		    **  What has happened is that we had a new entry with no
		    **  text, and the DSWRequestCloseOpenEntry blew it away,
		    **  which means that our dsw_entry is no longer valid.
		    */	  
		    {
			return_code = lwk_s_failure;
			break;
		    }
		    else
		    {
			DSWGetEntryData(dsw_entry, &dsw_entry_data);
			di = (DwcDayItem)dsw_entry_data.tag;
		    }
		}		

                /*	  
		**  Make sure it wasn't an empty entry (and therefore gone)
		*/	  
                if (di != 0)
		{
		    if (MEMEX_get_surrs_for_sel_entry
		    (
			cd,
			surrogate_list_head_ptr_ptr,
			cd->dsbot,
			di->item_id)
		    )
		    {
			return_code = lwk_s_success;		
		    }
		    else
		    {
			break;
		    }
		}
	    }	
	}
	else
	{
	    /*
	    **  A text widget owns the selection: let's ignore it
	    */
	    return_code = lwk_s_failure;
	}
    }
    else
    {
	/*
	**  We just don't own the selection
	*/	
	return_code = lwk_s_failure;
    }

    /*
    **  That's it
    */
    return return_code == lwk_s_success;
}

static Boolean MEMEX_get_surrs_for_sel_entry
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot,
	int			item_id)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot, item_id)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
	int			item_id;
#endif	/* prototype */
    {
    Boolean	    return_value = FALSE;

    /*
    **	Set the query parameters
    */

    dsbot_val_sn_1.lwk_operand_1	= (lwk_any_pointer) &dsbot;
    item_id_val_sn_1.lwk_operand_1	= (lwk_any_pointer)&item_id;
    filename_val_sn_1.lwk_operand_1	= cd->filename;

    /*
    **  Do the query
    */

    if (MEMEX_query_surrs(cd->hisdwui,
	(lwk_query_node *) &fully_qualified_query_sn_1,
	surrogate_list_head_ptr_ptr, cd->mainwid)) {
	
	return_value = TRUE;
    }
    else {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurFail");
    }

    /*
    **  That's it
    */

    return return_value;
    }


static Boolean MEMEX_get_surr_for_day
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
#endif	/* prototype */
    {
    Boolean	    return_value = FALSE;

    /*
    **	Load arguments for the query
    */

    dsbot_val_sn_2.lwk_operand_1	= (lwk_any_pointer) &dsbot;
    filename_val_sn_2.lwk_operand_1	= cd->filename;

    /*
    **  Do the query
    */

    if (MEMEX_query_surrs(cd->hisdwui,
	(lwk_query_node *) &fully_qualified_query_sn_2,
	surrogate_list_head_ptr_ptr, cd->mainwid)) {
	
	return_value = TRUE;
    }
    else {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurFail");
    }

    /*
    **  That's it
    */

    return return_value;

    }


static Boolean MEMEX_get_surr_for_sel_month
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
#endif	/* prototype */
    {
    Boolean return_value;
    Arg	    arglist[5];
    int	    argcount;
    int	    month,
	    year;
    int	    dsbot;

    return_value = FALSE;

    switch (cd->cb->select_type)
	{
	case MwDaySelected:
	    dsbot = DATEFUNCDaysSinceBeginOfTime
		(cd->cb->select_day, cd->cb->select_month, cd->cb->select_year);
		
	    if (MEMEX_get_surr_for_day(cd, surrogate_list_head_ptr_ptr, dsbot))
		{
		return_value  =TRUE;
		}
	    break;
	    
	case MwWeekSelected:
	    printf("Week surrogate not implemented\n");
	    break;
	    
	case MwMonthSelected:
	    dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, cd->cb->select_month, cd->cb->select_year);
		
	    if ( MEMEX_get_surr_for_month(cd, surrogate_list_head_ptr_ptr,
		dsbot) )
		{
		return_value  =TRUE;
		}
	    break;
	    
	case MwYearSelected:
	    dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, 1, cd->cb->select_year);
		
	    if ( MEMEX_get_surr_for_year(cd, surrogate_list_head_ptr_ptr,
		dsbot) )
		{
		return_value  =TRUE;
		}
	    break;
	    
	case MwNothingSelected:
	
	    XtSetArg(arglist[0], DwcMwNmonth, &month);
	    XtSetArg(arglist[1], DwcMwNyear, &year);
	    XtGetValues(cd->month_display, arglist, 2);
	
	    dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, month, year);
		
	    if ( MEMEX_get_surr_for_month(cd, surrogate_list_head_ptr_ptr,
		dsbot) )
		{
		return_value  =TRUE;
		}
	    break;
	
	default:
	    printf("Default surrogate not implemented\n");
	    break;
	}

    /*
    **  That's it
    */

    return return_value;
    }


static Boolean MEMEX_get_surr_for_month
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
#endif	/* prototype */
    {
    Boolean	    return_value = FALSE;
	
    /*
    **	Load arguments for the query
    */

    dsbot_val_sn_3.lwk_operand_1	= (lwk_any_pointer) &dsbot;
    filename_val_sn_3.lwk_operand_1	= cd->filename;

    /*
    **  Do the query
    */

    if (MEMEX_query_surrs(cd->hisdwui,
	(lwk_query_node *) &fully_qualified_query_sn_3,
	surrogate_list_head_ptr_ptr, cd->mainwid)) {
	
	return_value = TRUE;
    }
    else {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurFail");
    }

    /*
    **  That's it
    */

    return return_value;
    }


static Boolean MEMEX_get_surr_for_year
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
#endif	/* prototype */
    {
    Boolean	    return_value = FALSE;
	
    /*
    **	Load arguments for the query
    */

    dsbot_val_sn_4.lwk_operand_1	= (lwk_any_pointer) &dsbot;
    filename_val_sn_4.lwk_operand_1	= cd->filename;

    /*
    **  Do the query
    */

    if (MEMEX_query_surrs(cd->hisdwui,
	(lwk_query_node *) &fully_qualified_query_sn_4,
	surrogate_list_head_ptr_ptr, cd->mainwid)) {
	
	return_value = TRUE;
    }
    else {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurFail");
    }

    /*
    **  That's it
    */

    return return_value;
    }


static Boolean MEMEX_query_surrs
#ifdef _DWC_PROTO_
	(
	lwk_dxm_ui		his_dwui,
	lwk_query_node		*query_expression,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	Widget			parent_for_error)
#else	/* no prototypes */
	(his_dwui, query_expression, surrogate_list_head_ptr_ptr,
	parent_for_error)
	lwk_dxm_ui		his_dwui;
	lwk_query_node		*query_expression;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	Widget			parent_for_error;
#endif	/* prototype */
{
    Boolean	    return_value = TRUE;
    lwk_status 	    status;
    lwk_termination callback_return;
    lwk_composite_linknet current_composite_net = (lwk_composite_linknet) lwk_c_null_object;
    lwk_string status_text;

    status = lwk_get_value(his_dwui, lwk_c_p_active_comp_linknet,
	lwk_c_domain_comp_linknet, (lwk_any_pointer) &current_composite_net);

    if ((status == lwk_s_success) &&
	(current_composite_net != (lwk_composite_linknet) lwk_c_null_object))
    {
	
	status = lwk_query(current_composite_net, lwk_c_domain_surrogate,
	    (lwk_query_expression) query_expression,
	    (lwk_closure) surrogate_list_head_ptr_ptr,
	    (lwk_callback) MEMEX_surrogate_found_cb, &callback_return);
	
	if (status != lwk_s_success) {
	    if (hyperinitialized) {
		hyperinitialized = FALSE;
		lwk_status_to_string(status, &status_text);
		ERRORDisplayText(parent_for_error, "ErrorNetQueryFail", status_text);
		lwk_delete_string(&status_text);
	    }
	    return_value = FALSE;
	}
    }
    else {
	return_value = FALSE;

	if (hyperinitialized) {
	    hyperinitialized = FALSE;
	    if (status != lwk_s_success) {
		lwk_status_to_string(status, &status_text);
		ERRORDisplayText(parent_for_error, "ErrorGetNetFail", status_text);
		lwk_delete_string(&status_text);
	    }
	}
    }

    return return_value;

}


static lwk_status MEMEX_create_surr_cb
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_object		*surrogate_ptr)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data, surrogate_ptr)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
	lwk_object		*surrogate_ptr;
#endif	/* prototype */
    {
    CalendarDisplay cd = (CalendarDisplay)user_data;
    CalendarBlock   *cb = cd->cb;
    lwk_status	    status;
    lwk_status	    return_code;
    int		    selected_dsbot;
    int		    index;
    Cardinal	    day;
    Cardinal	    month;
    Cardinal	    year;
		
#ifdef DEBUG
    printf("CreateSurrogate Callback\n");
#endif

    return_code = lwk_s_failure;

    /*
    **  What's the guy seeing here anyway...
    */

    switch (cd->showing) {
	
	case show_day:

            /*
	    **  We are looking at a day view
	    */
	
	    if (MEMEX_create_surr_for_sel_entry(cd, surrogate_ptr)) {

		/*
		**  if a time slot is selected, create a surrogate for it
		**  and highlight it if needed
		*/
		for (index = 0; index < cd->number_of_selected_entries; index++) {
		    if (cd->selected_entries[0] ==
			cd->dayslots.items[index]->entry) {
			break;
		    }
		}
		
		MEMEX_add_icon_to_timeslot(cd, index, *surrogate_ptr);
		
		return_code = lwk_s_success;
		}
		
	    else if ( (cd->cb->select_type != MwNothingSelected) &&
		MEMEX_create_surr_for_sel_month(cd, surrogate_ptr) )

		/*
		**  Create a surrogate for a month if it is selected
		*/
		
		{
		return_code = lwk_s_success;
		}
		
	    else if ( MEMEX_create_surr_for_day(cd, surrogate_ptr, cd->dsbot) ) {

		/*
		**  Otherwise if nothing is selected, create a surrogate
		**  for the whole day view.  This done because we can't dim the
		**  'set source' and set target' entries in the connection menu
		*/
		
		return_code = lwk_s_success;
		}
	    break;
	
	case show_week:
	    printf("- Creation of week surrogate not supported\n");
	    break;
	
	case show_month:
	
	    if ( MEMEX_create_surr_for_sel_month(cd, surrogate_ptr) )
		{
		return_code = lwk_s_success;
		}				
	    break;
	
	case show_year:
	
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, cd->yd->first_month, cd->yd->first_year);
		
	    if ( (cd->cb->select_type != MwNothingSelected) &&
		MEMEX_create_surr_for_sel_month(cd, surrogate_ptr) )
		{
		return_code = lwk_s_success;
		}
		
	    else if ( MEMEX_create_surr_for_year(cd, surrogate_ptr,
		selected_dsbot) )
		{
		return_code = lwk_s_success;			
		}	
	    break;
	
	default:
	    printf("- creation of default surrogate not supported\n");
	    break;
	
	}

    /*
    **  That's it
    */

    return return_code;
    }


static lwk_status MEMEX_close_view_cb
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
#endif	/* prototype */
{
    CalendarDisplay	    cd = (CalendarDisplay)user_data;

#ifdef DEBUG
    printf("CloseView Callback\n");
#endif
    
    /*
    **  Calendar doesn't fully support the Visit/GoTo semantic. It never closes
    **	down the current view but just iconizes itself.
    */
    MEMEX_set_shell_iconstate( cd->toplevel, cd->toplevel, IconicState);

    return lwk_s_success;
}


lwk_status MEMEXApplyCallback
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_surrogate		surrogate,
	lwk_string		operation,
	lwk_integer		follow_type)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data, surrogate, operation, follow_type)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
	lwk_surrogate		surrogate;
	lwk_string		operation;
	lwk_integer		follow_type;
#endif	/* prototype */
{
    Arg		    arglist[1];
    CalendarDisplay cd = (CalendarDisplay)user_data;
    CalendarDisplay new_cd;
    Cardinal	    day;
    Cardinal	    month;
    Cardinal	    year;
    XWMHints	    hints;
    lwk_status	    status;
    lwk_status	    return_code;
    lwk_string	    sur_subtype;
    lwk_string	    sur_filename;
    lwk_string	    surrogate_type;
    lwk_integer	    sur_dsbot;
    lwk_integer	    sur_item_id;
    lwk_string	    status_text;

#ifdef DEBUG
    printf("Apply Callback\n");
#endif

    return_code = lwk_s_success;

    /*
    **  Make sure that Calendar is de-iconised
    */
    MEMEX_set_shell_iconstate( cd->toplevel, cd->toplevel, NormalState);

    XRaiseWindow(XtDisplay(cd->toplevel), XtWindow(cd->toplevel));


    status = lwk_get_value(surrogate, lwk_c_p_surrogate_sub_type,
	lwk_c_domain_string, (lwk_any_pointer) &sur_subtype);
	
    if (status != lwk_s_success)
    {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorGetSurSubtypeFail", status_text);
	lwk_delete_string(&status_text);
    }

    if (strcmp(sur_subtype, DWC_SURROGATE) != 0)
    {
	ERRORDisplayError(cd->mainwid, "ErrorUnkSubtype");
    }
    else
    {

	/*
	**  Get the type of the calendar surrogate. It is a property on the
	**  surrogate 
	*/
	status = lwk_get_value
	(
	    surrogate,
	    DWC_SURROGATE_TYPE,
	    lwk_c_domain_string,
	    (lwk_any_pointer) &surrogate_type
	);

	if (status != lwk_s_success)
	{
	    lwk_status_to_string(status, &status_text);
	    ERRORDisplayText
		(cd->mainwid, "ErrorGetSurSubtypeFail", status_text);
	    lwk_delete_string(&status_text);
	}

	if (strcmp(surrogate_type, DWC_MEMEX_TIMESLOT_ENTRY) == 0 )
	{
	    status = lwk_get_value(surrogate, DWC_FILE_PROPERTY,
		lwk_c_domain_string, (lwk_any_pointer) &sur_filename);
	    
	    status = lwk_get_value(surrogate, DWC_DSBOT_PROPERTY,
		lwk_c_domain_integer, (lwk_any_pointer) &sur_dsbot);
	    
	    status = lwk_get_value(surrogate, DWC_ITEM_ID_PROPERTY,
		lwk_c_domain_integer, (lwk_any_pointer) &sur_item_id);

	    if ( strcmp(operation, DWC_MEMEX_VIEW_ENTRY) == 0)
	    {
		if ( !MEMEX_highlight_tse(cd, follow_type, surrogate,
		    sur_filename, sur_dsbot, sur_item_id) )
		{
		    ERRORReportError(cd->mainwid, "ErrorDanglingConnection",
			(XtCallbackProc) MEMEX_del_dangle_cb, 
			(caddr_t) surrogate);
		    
		    DATEFUNCDateForDayNumber
			(cd->dsbot, (int *)&day, (int *)&month, (int *)&year);
		    MISCChangeView(cd, show_day, day, month, year);
		}
	    }	
	    else
	    {
		printf("Unsupported operation for GoTo/Visit\n");
	    }
	}
	else if (strcmp(surrogate_type, DWC_MEMEX_OPEN_TIMESLOT_ENTRY) == 0)
	{
	    /*
	    **  This type of surrogate is not designed yet
	    */
	    printf("- The open_timeslot surrogate in apply cb is not supported\n");
	}
	else if (strcmp(surrogate_type, DWC_MEMEX_DAY) == 0)
	{
	    status = lwk_get_value(surrogate, DWC_FILE_PROPERTY,
		lwk_c_domain_string, (lwk_any_pointer) &sur_filename);
	    
	    status = lwk_get_value(surrogate, DWC_DSBOT_PROPERTY,
		lwk_c_domain_integer, (lwk_any_pointer) &sur_dsbot);

	    /*
	    **  We should check the operation here...
	    */
	    if ( strcmp(operation, DWC_MEMEX_VIEW_ENTRY) == 0)
	    {
		if (! MEMEX_change_view(cd, follow_type, &new_cd,
		    show_day, sur_filename, sur_dsbot) )
		{
		    ERRORReportError
		    (
			cd->mainwid,
			"ErrorDanglingConnection",
			(XtCallbackProc) MEMEX_del_dangle_cb,
			(caddr_t) cd
		    );
		}
	    }
	    else
	    {
		ERRORDisplayError(cd->mainwid, "ErrorUnsupOp");
	    }
	    
	}
	else if (strcmp(surrogate_type, DWC_MEMEX_MONTH) == 0)
	{
	    status = lwk_get_value(surrogate, DWC_FILE_PROPERTY,
		lwk_c_domain_string, (lwk_any_pointer) &sur_filename);
		
	    status = lwk_get_value(surrogate, DWC_DSBOT_PROPERTY,
		lwk_c_domain_integer, (lwk_any_pointer) &sur_dsbot);

	    /*
	    **  Check the operation here
	    */
	    if (strcmp(operation, DWC_MEMEX_VIEW_ENTRY) == 0)
	    {
		if ( ! MEMEX_change_view(cd, follow_type, &new_cd,
		    show_month, sur_filename, sur_dsbot) )
		{
		    ERRORReportError
		    (
			cd->mainwid,
			"ErrorDanglingConnection",
			(XtCallbackProc) MEMEX_del_dangle_cb,
			(caddr_t) cd
		    );
		}
	    }
	    else
	    {
		ERRORDisplayError(cd->mainwid, "ErrorUnsupOp");
	    }
	}
	    
	else if (strcmp(surrogate_type, DWC_MEMEX_YEAR) == 0)
	{
	    status = lwk_get_value(surrogate, DWC_FILE_PROPERTY,
		lwk_c_domain_string, (lwk_any_pointer) &sur_filename);
		
	    status = lwk_get_value(surrogate, DWC_DSBOT_PROPERTY,
		lwk_c_domain_integer, (lwk_any_pointer) &sur_dsbot);

	    /*
	    **  Check the operation here
	    */
	    if ( strcmp(operation, DWC_MEMEX_VIEW_ENTRY) == 0)
	    {
		if ( ! MEMEX_change_view(cd, follow_type, &new_cd,
		    show_year, sur_filename, sur_dsbot) )
		{
		    ERRORReportError
		    (
			cd->mainwid,
			"ErrorDanglingConnection",
			(XtCallbackProc) MEMEX_del_dangle_cb,
			(caddr_t) cd
		    );
		}
	    }
	    else
	    {
		ERRORDisplayError(cd->mainwid, "ErrorUnsupOp");
	    }
	}
	else
	{
	    ERRORDisplayError(cd->mainwid, "ErrorUnkSubtype");
	}
    }

    /*
    ** Confirm the Apply
    */
    status = lwk_confirm_apply (dw_ui, surrogate);

    /*
    **  That's it
    */
    return return_code;

    }


static lwk_status MEMEX_connect_cb
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_link		connection)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data, connection)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
	lwk_link		connection;
#endif	/* prototype */
    {
    lwk_status	    return_code;
    CalendarDisplay cd = (CalendarDisplay)user_data;

#ifdef DEBUG
    printf("Connnect Callback\n");
#endif

    return_code = lwk_s_success;

    /*
    **	Calendar isn't very smart about MEMEX integration, and just
    **	discards this callback. Extended integration may come one
    **	day though.
    */

    return return_code;

    }


static void MEMEX_remove_icons_from_timeslots
#ifdef _DWC_PROTO_
	(CalendarDisplay cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
    
{
    unsigned char	    *icons;
    Cardinal		    num_icons;
    int			    i;

    for (i = 0; i < cd->dayslots.number_of_items; i++)
    {
	cd->dayslots.items[i]->memexized = False;
	
	DAYGetIconsForDayItem(cd->dayslots.items[i], &icons, &num_icons) ;

	DSWSetEntryIcons
	(
	    (DayslotsWidget) cd->dayslots.widget,
	    cd->dayslots.items[i]->entry,
	    icons,
	    num_icons
	);
    }
}    

static lwk_status MEMEX_currency_change_cb
#ifdef _DWC_PROTO_
	(
	lwk_ui	    		dw_ui,
	lwk_reason		reason,
	XmAnyCallbackStruct	*dxm_info,
	lwk_closure		user_data,
	lwk_integer		currency)
#else	/* no prototypes */
	(dw_ui, reason, dxm_info, user_data, currency)
	lwk_ui			dw_ui;
	lwk_reason		reason;
	XmAnyCallbackStruct	*dxm_info;
	lwk_closure		user_data;
	lwk_integer		currency;
#endif	/* prototype */
{
    CalendarDisplay	cd = (CalendarDisplay)user_data;
    lwk_environment_change	currency_code = (lwk_environment_change)currency;
    lwk_status		return_code;
    lwk_status		status;
    lwk_integer		highlight_state;
    Cardinal		day;
    Cardinal		month;
    Cardinal		year;
    lwk_string		status_text;
    lwk_composite_linknet	cnet;
#ifdef DEBUG
    printf("CurrencyChange Callback\n");
#endif

    switch (currency_code)
    {
    case lwk_c_env_active_comp_linknet:
	/*
	**  We've got a valid hypersomething out there so we can do queries
	**  once again.
	*/
	hyperinitialized = TRUE;

	/*
	**  Get the current composite network from the Memex cache.
	**  Delete it from the Memex cache so that the next
	**  lwk_get_value on the current composite net will update
	**  the Memex cache.
	*/
	status = lwk_get_value
	(
	    cd->hisdwui,
	    lwk_c_p_active_comp_linknet,
	    lwk_c_domain_comp_linknet,
	    (lwk_any_pointer) &cnet
	);

	if ((status == lwk_s_success) && (cnet != lwk_c_null_object))
	{
	    lwk_termination termination;

	    status = lwk_iterate
	    (
		cnet,
		lwk_c_domain_linknet,
		(lwk_closure) 0,
		MEMEX_del_linknet,
		&termination
	    );

	    if (status == lwk_s_success) lwk_delete (&cnet);
	}

	/**
	*** Note, this is a fall through into the next case.
	**/

    case lwk_c_env_appl_highlight:

	if (cd->showing == show_day)
	{
	    /*
	    **  If the day view is displayed, update the display
	    */
	    DATEFUNCDateForDayNumber
		(cd->dsbot, (int *)&day, (int *)&month, (int *)&year);
	    MISCChangeView(cd, show_day, day, month, year);
	    MEMEX_remove_icons_from_timeslots (cd);
	    MEMEXSurrogateHighlightForDay (cd);
	}
	cd->default_highlighting = FALSE;
	
	break;

    case lwk_c_env_pending_source:
    case lwk_c_env_pending_target:
    case lwk_c_env_follow_destination:

	MEMEXSurrogateHighlightForDay (cd);
	break;

    case lwk_c_env_default_highlight:

	/*
	**  Set the $CurrentHighlighting property of the DwUi to be the
	**	$DefaultHighlighting of the DwUi only if the user hasn't
	**	expressed any preferences through the dialog box 
	*/

	if (cd->default_highlighting)
	{
	    status = lwk_get_value
	    (
		cd->hisdwui,
		lwk_c_p_default_highlight,
		lwk_c_domain_integer,
		(lwk_any_pointer) &highlight_state
	    );

	    if (status != lwk_s_success)
	    {
		lwk_status_to_string (status, &status_text);
		ERRORDisplayText
		    (cd->mainwid, "ErrorGetDefHighFail", status_text);
		lwk_delete_string (&status_text);
	    }

	    status = lwk_set_value
	    (
		cd->hisdwui,
		lwk_c_p_appl_highlight,
		lwk_c_domain_integer,
		(lwk_any_pointer) &highlight_state,
		lwk_c_set_property
	    );

	    if (status != lwk_s_success)
	    {
		lwk_status_to_string (status, &status_text);
		ERRORDisplayText
		    (cd->mainwid, "ErrorSetCurHighFail", status_text);
		lwk_delete_string (&status_text);
	    }

	    cd->default_highlighting = TRUE;
	}

	break;

#if 0
    case lwk_c_env_active_comp_linknet:

	/*	  
	**  We've got a valid hypersomething out there so we can do queries
	**	once again.
	*/	  
	hyperinitialized = TRUE;
#endif
/*	case lwk_c_env_recording_linknet:
    case lwk_c_env_active_path:
    case lwk_c_env_active_comp_path:
    case lwk_c_env_recording_path:
    case lwk_c_env_followed_step:
    case lwk_c_env_new_link:
    case lwk_c_current_surrogate:
    case lwk_c_current_origin:
    case lwk_c_env_default_operation:
    case lwk_c_env_default_relationship: */
    default:
	break;
    }

    return lwk_s_success;
}

static Boolean MEMEX_create_surr_for_sel_entry
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr)
#else	/* no prototypes */
	(cd, surrogate_ptr)
	CalendarDisplay	cd;
	lwk_object	*surrogate_ptr;
#endif	/* prototype */
    {
    Time		    time;
    char		    *temp_string,
			    *entry_text;
    XmString		    entry_ctext,
			    date_time_ctext;
    int			    temp_integer;
    Widget		    select_widget;
    int			    i;
    Boolean		    return_code;
    lwk_status		    status;
    DwcDswEntry		    dsw_entry;
    DSWEntryDataStruct   dsw_entry_data;
    DwcMemexSurrogateList	    *surrogate_list_ptr = (DwcMemexSurrogateList *) 0;
    DwcMemexSurrogateList	    *surrogate_list;
    DwcDayItem		    di;
    dtb			    date_time;
    long		    cvt_byte_count, cvt_status;
    Opaque		    date_time_ddif;
    lwk_string		    status_text;
    lwk_integer		    tmp_int;

    if ( CLIPTestSelectionWidget(cd, &select_widget) )
	{
	/*
	**  CLIPTestSelectionWidget returns TRUE if one of our custom
	**  widgets or text widgets own the selection.
	*/	

	if (select_widget == NULL) {
	    
	    /*
	    **	It's one of our custom widgets which owns the selection
	    **
	    **  *** For now we only process the first widget selected
	    **	in the list. If we want to process all the selected entries, we
	    **	should use the following loop statement:
	    **	
	    **	for (i = 0; i < cd->number_of_selected_entries; i++) { ... }
	    **	
	    */

	    i = 0; /* We are only interested in the first entry selected */
	    
	    dsw_entry = cd->selected_entries[i];
	    DSWGetEntryData(dsw_entry, &dsw_entry_data);
	    di = (DwcDayItem)dsw_entry_data.tag;

            /*	  
	    **  New entry, hasn't yet been saved (ie not item number) so save it
	    **	for the user.
	    */	  
            if (di == 0) {
		time = MISCGetTimeFromAnyCBS (NULL);
		(void) DSWRequestCloseOpenEntry
		    ((DayslotsWidget) dsw_entry_data.parent, time);
		DSWGetEntryData(dsw_entry, &dsw_entry_data);
		di = (DwcDayItem)dsw_entry_data.tag;
		if (di == 0) {
                    /*	  
		    **  The user hadn't typed anything in, so it wasn't saved so
		    **	we have zilch.
		    */	  
                    return(FALSE);
		}
	    }		

            /*	  
	    **  For now we don't support daynotes. Daynotes should really have
	    **	their own surrogate but they don't as of yet. Andre promised to
	    **	do it soon. 8-)
	    */	  
            if (di->duration > 0) {
		/*
		**  Create a surrogate and set its sub-type and its calendar
		**  sub-type 
		*/
		
		status = lwk_create(lwk_c_domain_surrogate, surrogate_ptr);

		if (status != lwk_s_success) {
		    lwk_status_to_string(status, &status_text);
		    ERRORDisplayText(cd->mainwid,
			"ErrorCreateSurrogateFail", status_text);
		    lwk_delete_string(&status_text);
		}

		temp_string = DWC_SURROGATE;
		
		status = lwk_set_value(*surrogate_ptr, lwk_c_p_surrogate_sub_type,
		    lwk_c_domain_string, (lwk_any_pointer) &temp_string,
		    lwk_c_set_property);

		temp_string = DWC_MEMEX_TIMESLOT_ENTRY;
		
		status = lwk_set_value(*surrogate_ptr, DWC_SURROGATE_TYPE,
		    lwk_c_domain_string, (lwk_any_pointer) &temp_string,
		    lwk_c_set_property);
		
		/*
		**  Time to build a meaningful description
		*/
		
		DATEFUNCDateForDayNumber
		(
		    cd->dsbot,
		    &date_time.day,
		    &date_time.month,
		    &date_time.year
		);
		date_time.weekday = DATEFUNCDayOfWeek
		    (date_time.day, date_time.month, date_time.year);
		date_time.hour	 = di->start_time / 60;
		date_time.minute = di->start_time % 60;
		date_time_ctext = DATEFORMATTimeToCS
		    (dwc_k_memex_timeslot_format, &date_time);
		entry_text = DSWGetEntryText(dsw_entry);
		entry_ctext = XmStringCreateSimple(entry_text);
		XmStringConcat(date_time_ctext, entry_ctext);
		
		date_time_ddif = (Opaque)DXmCvtCStoDDIF
		    (date_time_ctext, &cvt_byte_count, &cvt_status);
		if (cvt_status == DXmCvtStatusFail)
		    {
		    ERRORDisplayError(cd->mainwid, "ErrorConvertFail");
		    return(FALSE);
		    }
		status = lwk_set_value(*surrogate_ptr, lwk_c_p_description,
		    lwk_c_domain_ddif_string,
		    (lwk_any_pointer) &date_time_ddif, lwk_c_set_property);

		/*
		**  Set the other properties for the timeslot surrogate
		*/

		status = lwk_set_value(*surrogate_ptr, DWC_FILE_PROPERTY,
		    lwk_c_domain_string, (lwk_any_pointer) &cd->filename,
		    lwk_c_set_property);
		
		status = lwk_set_value(*surrogate_ptr, DWC_DSBOT_PROPERTY,
		    lwk_c_domain_integer, (lwk_any_pointer) &cd->dsbot,
		    lwk_c_set_property);
		
		tmp_int = (lwk_integer) di->item_id;
		status = lwk_set_value(*surrogate_ptr, DWC_ITEM_ID_PROPERTY,
		    lwk_c_domain_integer, (lwk_any_pointer) &tmp_int,
		    lwk_c_set_property);

		return_code = TRUE;
	    }
	    else {
		return_code = FALSE;
	    }
	}
	else {
	
	    return_code = FALSE;
	}
	
    }
    else {
    
	return_code = FALSE;
    }

    /*
    **  That's it
    */

    return return_code;

    }


static Boolean MEMEX_create_surr_for_day
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot)
#else	/* no prototypes */
	(cd, surrogate_ptr, dsbot)
	CalendarDisplay	cd;
	lwk_object	*surrogate_ptr;
	int		dsbot;
#endif	/* prototype */
    {
    char		    *temp_string;
    Boolean		    return_code;
    lwk_status		    status;
    dtb			    date_time;
    XmString		    date_time_ctext;
    long		    cvt_byte_count, cvt_status;
    Opaque		    date_time_ddif;
    lwk_string		    status_text;

    return_code = FALSE;

    status = lwk_create(lwk_c_domain_surrogate, surrogate_ptr);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorCreateSurrogateFail", status_text);
	lwk_delete_string(&status_text);
    }

    temp_string = DWC_SURROGATE;

    status = lwk_set_value(*surrogate_ptr, lwk_c_p_surrogate_sub_type,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);

    temp_string = DWC_MEMEX_DAY;

    status = lwk_set_value(*surrogate_ptr, DWC_SURROGATE_TYPE,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);
	
    DATEFUNCDateForDayNumber
	(dsbot, &date_time.day, &date_time.month, &date_time.year);
    date_time.weekday = DATEFUNCDayOfWeek
	(date_time.day, date_time.month, date_time.year);
    date_time_ctext = DATEFORMATTimeToCS (dwc_k_memex_day_format, &date_time);

    date_time_ddif = (Opaque)DXmCvtCStoDDIF
	(date_time_ctext, &cvt_byte_count, &cvt_status);
    if (cvt_status == DXmCvtStatusFail)
	{
	ERRORDisplayError(cd->mainwid, "ErrorConvertFail");
	return(FALSE);
	}
    status = lwk_set_value(*surrogate_ptr, lwk_c_p_description,
	lwk_c_domain_ddif_string, (lwk_any_pointer) &date_time_ddif,
	lwk_c_set_property);
    
    status = lwk_set_value(*surrogate_ptr, DWC_FILE_PROPERTY,
	lwk_c_domain_string, (lwk_any_pointer) &cd->filename,
	lwk_c_set_property);
	
    status = lwk_set_value(*surrogate_ptr, DWC_DSBOT_PROPERTY,
	lwk_c_domain_integer, (lwk_any_pointer) &dsbot, lwk_c_set_property);
	
    return_code = TRUE;

    /*
    **  That's it
    */

    return return_code;

    }


static Boolean MEMEX_create_surr_for_sel_month
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr)
#else	/* no prototypes */
	(cd, surrogate_ptr)
	CalendarDisplay	cd;
	lwk_object	*surrogate_ptr;
#endif	/* prototype */
    {
    Boolean	    return_code;
    CalendarBlock   *cb = cd->cb;
    Arg		    arglist[5];
    int		    argcount;
    int		    month,
		    year;
    int		    selected_dsbot;

    return_code = FALSE;

    switch (cb->select_type)
	{
	case MwDaySelected:
	
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime
		(cb->select_day, cb->select_month, cb->select_year);
		
	    if ( MEMEX_create_surr_for_day(cd, surrogate_ptr,
		selected_dsbot) )
		{
		return_code = TRUE;
		}						
	    break;
	    
	case MwWeekSelected:
	
	    break;
	    
	case MwMonthSelected:
	
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, cb->select_month, cb->select_year);
		
	    if ( MEMEX_create_surr_for_month(cd, surrogate_ptr,
		selected_dsbot) )
		{
		return_code = TRUE;
		}		    
	    break;
	    
	case MwYearSelected:
	
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime
		(1, 1, cb->select_year);
		
	    if ( MEMEX_create_surr_for_year(cd, surrogate_ptr,
		selected_dsbot) )
		{
		return_code = TRUE;			
		}
	    break;
	    
	case MwNothingSelected:
	
	    XtSetArg(arglist[0], DwcMwNmonth, &month);
	    XtSetArg(arglist[1], DwcMwNyear, &year);
	    XtGetValues(cd->month_display, arglist, 2);
	
	    selected_dsbot = DATEFUNCDaysSinceBeginOfTime (1, month, year);
		
	    if ( MEMEX_create_surr_for_month(cd, surrogate_ptr,
		selected_dsbot) )
		{
		return_code = TRUE;
		}
		
	default:
	    break;
	}

    /*
    **  That's it
    */

    return return_code;

    }


static Boolean MEMEX_create_surr_for_month
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot)
#else	/* no prototypes */
	(cd, surrogate_ptr, dsbot)
	CalendarDisplay	cd;
	lwk_object	*surrogate_ptr;
	int		dsbot;
#endif	/* prototype */
    {
    char		    *temp_string;
    Boolean		    return_code;
    lwk_status		    status;
    dtb			    date_time;
    XmString		    date_time_ctext;
    DwcMemexSurrogateList   *surrogate_list_head = (DwcMemexSurrogateList *) 0;
    DwcMemexSurrogateList   *surrogate_list;
    long		    cvt_byte_count, cvt_status;
    Opaque		    date_time_ddif;
    lwk_string		    status_text;

    return_code = TRUE;

    status = lwk_create(lwk_c_domain_surrogate, surrogate_ptr);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorCreateSurrogateFail", status_text);
	lwk_delete_string(&status_text);
    }

    temp_string = DWC_SURROGATE;

    status = lwk_set_value(*surrogate_ptr, lwk_c_p_surrogate_sub_type,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);

    temp_string = DWC_MEMEX_MONTH;

    status = lwk_set_value(*surrogate_ptr, DWC_SURROGATE_TYPE,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);
	
    DATEFUNCDateForDayNumber
	(dsbot, &date_time.day, &date_time.month, &date_time.year);
    date_time_ctext = DATEFORMATTimeToCS(dwc_k_memex_month_format, &date_time);

    date_time_ddif = (Opaque)DXmCvtCStoDDIF(date_time_ctext, &cvt_byte_count,
				    &cvt_status);
    if (cvt_status == DXmCvtStatusFail)
	{
	ERRORDisplayError(cd->mainwid, "ErrorConvertFail");
	return(FALSE);
	}
    status = lwk_set_value(*surrogate_ptr, lwk_c_p_description,
	lwk_c_domain_ddif_string, (lwk_any_pointer) &date_time_ddif,
	lwk_c_set_property);

    status = lwk_set_value(*surrogate_ptr, DWC_FILE_PROPERTY,
	lwk_c_domain_string, (lwk_any_pointer) &cd->filename,
	lwk_c_set_property);
	
    status = lwk_set_value(*surrogate_ptr, DWC_DSBOT_PROPERTY,
	lwk_c_domain_integer, (lwk_any_pointer) &dsbot, lwk_c_set_property);

    /*
    **  That's it
    */

    return return_code;

    }


static Boolean MEMEX_create_surr_for_year
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_object	*surrogate_ptr,
	int		dsbot)
#else	/* no prototypes */
	(cd, surrogate_ptr, dsbot)
	CalendarDisplay	cd;
	lwk_object	*surrogate_ptr;
	int		dsbot;
#endif	/* prototype */
    {
    char		    *temp_string;
    Boolean		    return_code;
    lwk_status		    status;
    dtb			    date_time;
    XmString		    date_time_ctext;
    DwcMemexSurrogateList   *surrogate_list_head = (DwcMemexSurrogateList *) 0;
    DwcMemexSurrogateList   *surrogate_list;
    long		    cvt_byte_count, cvt_status;
    Opaque		    date_time_ddif;
    lwk_string		    status_text;

    return_code = TRUE;

    status = lwk_create(lwk_c_domain_surrogate, surrogate_ptr);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorCreateSurrogateFail", status_text);
	lwk_delete_string(&status_text);
    }

    temp_string = DWC_SURROGATE;

    DATEFUNCDateForDayNumber
	(dsbot, &date_time.day, &date_time.month, &date_time.year);
    date_time_ctext = DATEFORMATTimeToCS(dwc_k_memex_year_format, &date_time);

    date_time_ddif = (Opaque)DXmCvtCStoDDIF(date_time_ctext, &cvt_byte_count,
				    &cvt_status);
    if (cvt_status == DXmCvtStatusFail)
	{
	ERRORDisplayError(cd->mainwid, "ErrorConvertFail");
	return(FALSE);
	}
    status = lwk_set_value(*surrogate_ptr, lwk_c_p_description,
	lwk_c_domain_ddif_string, (lwk_any_pointer) &date_time_ddif,
	lwk_c_set_property);

    status = lwk_set_value(*surrogate_ptr, lwk_c_p_surrogate_sub_type,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);
	
    temp_string = DWC_MEMEX_YEAR;

    status = lwk_set_value(*surrogate_ptr, DWC_SURROGATE_TYPE,
	lwk_c_domain_string, (lwk_any_pointer) &temp_string,
	lwk_c_set_property);
	
    status = lwk_set_value(*surrogate_ptr, DWC_FILE_PROPERTY,
	lwk_c_domain_string, (lwk_any_pointer) &cd->filename,
	lwk_c_set_property);
	
    status = lwk_set_value(*surrogate_ptr, DWC_DSBOT_PROPERTY,
	lwk_c_domain_integer, (lwk_any_pointer) &dsbot, lwk_c_set_property);
	
    /*
    **  That's it
    */

    return return_code;

    }


static lwk_closure MEMEX_surrogate_found_cb
#ifdef _DWC_PROTO_
	(
	lwk_closure		closure,
	lwk_composite_linknet	composite_net,
	lwk_integer		domain,
	lwk_surrogate		*surrogate_ptr)
#else	/* no prototypes */
	(closure, composite_net, domain, surrogate_ptr)
	lwk_closure		closure;
	lwk_composite_linknet	composite_net;
	lwk_integer		domain;
	lwk_surrogate		*surrogate_ptr;
#endif	/* prototype */
{
    DwcMemexSurrogateList   *surrogate_list_new_ptr,
			    *surrogate_list_member_ptr,
			    **surrogate_list_head_ptr_ptr =
				(DwcMemexSurrogateList **)closure;

    surrogate_list_new_ptr = (DwcMemexSurrogateList *) XtMalloc
	(sizeof(DwcMemexSurrogateList));

    surrogate_list_new_ptr->next = NULL;
    surrogate_list_new_ptr->surrogate = *surrogate_ptr;

    if (*surrogate_list_head_ptr_ptr == NULL)
    {
	*surrogate_list_head_ptr_ptr = surrogate_list_new_ptr;
    }
    else
    {
	/*
	**
	*/
	surrogate_list_member_ptr = *surrogate_list_head_ptr_ptr;
	
	while (surrogate_list_member_ptr->next != NULL)
	{
	    surrogate_list_member_ptr = surrogate_list_member_ptr->next;
	}
	
	surrogate_list_member_ptr->next = surrogate_list_new_ptr;
    }

    /*
    **  That's it
    */

    return (lwk_closure)0;

}

static Boolean MEMEX_highlight_tse
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_integer	follow_type,
	lwk_surrogate	surrogate,
	char		*filename,
	int		dsbot,
	int		item_id)
#else	/* no prototypes */
	(cd, follow_type, surrogate, filename, dsbot, item_id)
	CalendarDisplay	cd;
	lwk_integer	follow_type;
	lwk_surrogate	surrogate;
	char		*filename;
	int		dsbot;
	int		item_id;
#endif	/* prototype */
    {
    lwk_integer		    highlight_state;
    lwk_status		    status;
    CalendarDisplay	    new_cd;
    Boolean		    return_code;
    int			    i;
    int			    time_value;

    return_code = FALSE;

    if (MEMEX_change_view(cd, follow_type, &new_cd,
	show_day, filename, dsbot))
	{
	cd = new_cd;
	}
    else
	{
        return return_code;
	}
	
    /*
    **	Ok, we are now looking at the right day within the right
    **	database; let's look for the entry corresponding to
    **	item_id:
    */

    for (i = 0; i < cd->dayslots.number_of_items; i++) {
    
	if (cd->dayslots.items[i]->item_id == item_id) {

	    status = lwk_get_value(cd->hisdwui, lwk_c_p_appl_highlight,
		lwk_c_domain_integer, (lwk_any_pointer) &highlight_state);

	    /*
	    **	Make sure it's viewable by scrolling the display. 
	    */
	    
	    time_value = cd->dayslots.items[i]->start_time;
	
	    if (time_value > TimeOffset)
		time_value = time_value - TimeOffset;
		
	    DSWMoveDayslotsToTime
		((DayslotsWidget) cd->dayslots.widget, time_value);

	    /*
	    **  Highlight the surrogate if needed
	    */
		
	    MEMEX_add_icon_to_timeslot(cd, i, surrogate);
	
	    /*
	    **  Check if we need to select the destination
	    */
	    
	    if (highlight_state & lwk_c_hl_select_destination)
	    {

		/*
		**  Let's select it (we need the time, so we'll make a dummy
		**  round trip to the server to get it).  The HIS Services
		**  should be really be giving us this time stamp...
		*/


		CLIPSMSetSelected
		    (cd, cd->dayslots.items[i]->entry, CurrentTime);

		TSWTraverseToText (cd->dayslots.items[i]->entry->timeslot);
	    }

	    return_code = TRUE;
	
	    break;
	}
    }
    	
    /*
    **  That's it
    */

    return return_code;

    }    


static Boolean MEMEX_change_view
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	lwk_integer	follow_type,
	CalendarDisplay	*new_cd_ptr,
	show_kind	which_display,
	char		*filename,
	int		dsbot)
#else	/* no prototypes */
	(cd, follow_type, new_cd_ptr, which_display, filename, dsbot)
	CalendarDisplay	cd;
	lwk_integer	follow_type;
	CalendarDisplay	*new_cd_ptr;
	show_kind	which_display;
	char		*filename;
	int		dsbot;
#endif	/* prototype */
    {
    Boolean	    return_code;
    Cardinal	    day,
		    month,
		    year;
    int		    i;
    Boolean	    found_it;

    return_code = FALSE;

    if ( strcmp(cd->filename, filename) )
	{
	/*
	**  Rats, wrong database: we'll look into the array of
	**  already-opened databases for 'filename', and we don't find
	**  anything, we'll go and get the right one.
	*/
	found_it = FALSE;
	for (i=0; i<ads.number_of_calendars; i++)
	    {
	    if ( strcmp(ads.cds[i]->filename, filename) == 0 )
		{
		found_it = TRUE;
		*new_cd_ptr = ads.cds[i];
		break;
		}
	    }
	
	if (found_it) {
	
	    switch ( follow_type ) {

		case lwk_c_follow_go_to:
		    FILECloseCalendar(cd, CurrentTime);
		    break;
		
		case lwk_c_follow_visit:
		    break;
		
		default:
		    break;
		
	    }
	}
	else {
	
	    switch (follow_type) {
	    
		case lwk_c_follow_go_to:
		    cd->delete_pending = TRUE;	    
		    break;
		
		case lwk_c_follow_visit:
		    cd->delete_pending = FALSE;	    
		    break;
		
		default:
		    break;
		
	    }
	    if (FILEOpenCalendar(cd->ads, filename, new_cd_ptr)) {
	    
	    }
	    else {
		return return_code;
	    }	    
	}
	cd = *new_cd_ptr;
	}
    else {
	*new_cd_ptr = cd;
    }

    DATEFUNCDateForDayNumber (dsbot, (int *)&day, (int *)&month, (int *)&year);

    switch (which_display) {
    
	case show_day:
/*	    if ( (cd->showing != show_day) || (cd->dsbot != dsbot) )*/
	    MISCChangeView(cd, show_day, day, month, year);		
	    return_code = TRUE;
	    break;
	    
	case show_month:
	    MWSetSelection(cd->month_display, MwNothingSelected, day, 1, month, year);
	    MISCChangeView(cd, show_month, day, month, year);
	    return_code = TRUE;
	    break;
	    
	case show_year:
	    MWSetSelection(cd->month_display, MwNothingSelected, day, 1, month, year);
	    MISCChangeView(cd, show_year, day, month, year);
	    return_code = TRUE;
	    break;
	    
	case show_week:
	default:
	    break;
    }

    /*
    **  That's it
    */

    return return_code;

    }
    

static void MEMEX_del_dangle_cb
#ifdef _DWC_PROTO_
	(
	Widget			w,
	caddr_t			surrogate,
	XmAnyCallbackStruct	*cbs)
#else	/* no prototypes */
	(w, surrogate, cbs)
	Widget			w;
	caddr_t			surrogate;
	XmAnyCallbackStruct	*cbs;
#endif	/* prototype */

{
    lwk_termination cb_return_value = 0;
    lwk_integer	    count = 0;
    lwk_status	    status;
    lwk_set	    connection_set;
    lwk_string	    status_text;
    CalendarDisplay cd;

    (void)MISCFindCalendarDisplay(&cd, w);
    
    if (cbs->reason == (int)XmCR_OK)
    {

    /*
    **	How can we delete the connection? the only thing we have is a surrogate
    **	Should we check the $interconnections property of the surrogate? This
    **	surrogate can be source or target so should we delete all the
    **	connections contained in its $inter_connections list? That's what I am
    **	doing anyway...
    */

  	status = lwk_get_value(surrogate, lwk_c_p_inter_links,
  	    lwk_c_domain_set, (lwk_any_pointer) &connection_set);

  	status = lwk_get_value(connection_set, lwk_c_p_element_count,
  	    lwk_c_domain_integer, (lwk_any_pointer) &count);

  	status = lwk_iterate(connection_set, lwk_c_domain_persistent,
  	    (lwk_closure)cd, (lwk_callback) MEMEX_del_connection_cb,
	    &cb_return_value);

	if (status != lwk_s_success) {
	    lwk_status_to_string(status, &status_text);
	    ERRORDisplayText(cd->mainwid, "ErrorIterateConnSetFail", status_text);
	    lwk_delete_string(&status_text);
	}

	if (cb_return_value != (lwk_termination) 0)
	    ERRORDisplayError(cd->mainwid, "ErrorDeleteConnFail");

  	status = lwk_delete(&connection_set);
  
	if (status != lwk_s_success) {
	    lwk_status_to_string(status, &status_text);
	    ERRORDisplayText(cd->mainwid, "ErrorDeleteSurConnFail", status_text);
	    lwk_delete_string(&status_text);
	}
	
    }

}


static lwk_termination MEMEX_del_connection_cb
#ifdef _DWC_PROTO_
	(
	lwk_closure	closure,
	lwk_object	set,
	lwk_domain	domain,
	lwk_any_pointer	*connection)
#else	/* no prototypes */
	(closure, set, domain, connection)
	lwk_closure	closure;
	lwk_object	set;
	lwk_domain	domain;
	lwk_any_pointer	*connection;
#endif	/* prototype */
    {
    lwk_status	    status;
    lwk_termination return_value = 0;
    lwk_string	    status_text;
    CalendarDisplay cd = (CalendarDisplay)closure;

    status = lwk_delete(connection);

    if (status != lwk_s_success) {
	lwk_status_to_string(status, &status_text);
	ERRORDisplayText(cd->mainwid, "ErrorDeleteConnFail", status_text);
	lwk_delete_string(&status_text);
    }
    
    if (status != lwk_s_success)
	return((lwk_termination) status);

    return (return_value);

    }



static void MEMEX_add_icon_to_timeslot
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd,
	int		index,
	lwk_surrogate	surrogate)
#else	/* no prototypes */
	(cd, index, surrogate)
	CalendarDisplay	cd;
	int		index;
	lwk_surrogate	surrogate;
#endif	/* prototype */
    
{
    lwk_status 		    status;
    unsigned char	    *icons;
    Cardinal		    num_icons;
    lwk_boolean		    highlight;

    status = lwk_surrogate_is_highlighted (cd->hisdwui, surrogate, &highlight);

    if (highlight)
    {
	cd->dayslots.items[index]->memexized = True;
	
	DAYGetIconsForDayItem(cd->dayslots.items[index], &icons, &num_icons);
	
	DSWSetEntryIcons
	(
	    (DayslotsWidget) cd->dayslots.widget,
	    cd->dayslots.items[index]->entry,
	    icons,
	    num_icons
	);
			     
/*	cd->dayslots.items[index]->memexized = FALSE; */
    }
}    

void MEMEXSurrogateHighlightForDay
#ifdef _DWC_PROTO_
	(
	CalendarDisplay	cd)
#else	/* no prototypes */
	(cd)
	CalendarDisplay	cd;
#endif	/* prototype */
{
    int				i;
    int				item_id;
    lwk_status			status;
    lwk_surrogate		surrogate;
    DwcMemexSurrogateList	*surrogate_list;
    DwcMemexSurrogateList	*surrogate_list_head =
				    (DwcMemexSurrogateList *) 0;

    /*
    **  Get the surrogates for the day, if any
    */
		
    if (MEMEX_get_surrs_for_today(cd, &surrogate_list_head, cd->dsbot))
    {
	if (surrogate_list_head != (DwcMemexSurrogateList *) 0)
	{
	    surrogate_list = surrogate_list_head;

	    /*
	    **	Run dowm the list of surrogates and get their item_id. For each of
	    **	decide if it needs to be highlighted.
	    */
	    while (surrogate_list != NULL)
	    {
		surrogate = surrogate_list->surrogate;
	
		status = lwk_get_value
		(
		    surrogate,
		    DWC_ITEM_ID_PROPERTY,
		    lwk_c_domain_integer,
		    (lwk_any_pointer) &item_id
		);

		for (i = 0; i < cd->dayslots.number_of_items; i++)
		{
		    if (cd->dayslots.items[i]->item_id == item_id)
		    {
			MEMEX_add_icon_to_timeslot (cd, i, surrogate);
			break;
		    }
		}
		surrogate_list = (DwcMemexSurrogateList *)surrogate_list->next;

	    }

	    /*
	    **  Free the internal surrogate list
	    */
	    surrogate_list = surrogate_list_head;

	    while (surrogate_list != NULL)
	    {
		surrogate_list_head =
		    (DwcMemexSurrogateList *)surrogate_list->next;
		XtFree((char *)surrogate_list);
		surrogate_list = surrogate_list_head;
	    }

	}
    }
    else
    {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurFail");
    }

}

static Boolean MEMEX_get_surrs_for_today
#ifdef _DWC_PROTO_
	(
	CalendarDisplay		cd,
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr,
	int			dsbot)
#else	/* no prototypes */
	(cd, surrogate_list_head_ptr_ptr, dsbot)
	CalendarDisplay		cd;
	DwcMemexSurrogateList	**surrogate_list_head_ptr_ptr;
	int			dsbot;
#endif	/* prototype */
    {
    Boolean	    return_value = FALSE;

    /*
    **	Load arguments for the query
    */

    dsbot_val_sn_5.lwk_operand_1	= (lwk_any_pointer) &dsbot;
    filename_val_sn_5.lwk_operand_1	= cd->filename;

    /*
    **  Do the query
    */

    if (MEMEX_query_surrs(cd->hisdwui,
	(lwk_query_node *) &fully_qualified_query_sn_5,
	surrogate_list_head_ptr_ptr, cd->mainwid)) {
	
	return_value = TRUE;
    }
    else {
        /*	  
	**  Only report if the hyper stuff got initialized.
	*/	  
        if (hyperinitialized)
	    ERRORDisplayError(cd->mainwid, "ErrorQuerySurItemsFail");
    }

    /*
    **  That's it
    */

    return return_value;
    
    }



static void MEMEX_set_shell_iconstate 
#ifdef _DWC_PROTO_
	(
	Widget			toplevel,
	Widget			shell,
	long			state)
#else	/* no prototypes */
	(toplevel, shell, state)
	Widget			toplevel;
	Widget			shell;
	long			state;
#endif	/* prototype */
{
    XEvent event;
    Screen *scrn = XtScreen(toplevel);
    Display *dpy = XtDisplay(toplevel);

    /*
    ** Most window managers should support this.
    */
    event.xclient.type = ClientMessage;
    event.xclient.display = dpy;
    event.xclient.window = XtWindow(shell);
    event.xclient.message_type = DwcWmChangeStateAtom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = state;
    if (event.xclient.message_type != None)
	XSendEvent
	(
	    dpy,
	    RootWindowOfScreen(scrn),
	    False,
	    SubstructureRedirectMask|SubstructureNotifyMask,
	    &event
	);
    /*
    ** If XUI WM has been run, then we do this also, just to make sure.
    */
    if (MISCIsXUIWMRunning(shell, False))
    {
	XWMHints hints;
	hints.flags = StateHint;
	hints.initial_state = state;

	XSetWMHints (dpy, XtWindow(shell), &hints);
    }
}

static lwk_termination MEMEX_del_linknet
#ifdef _DWC_PROTO_
	(
	lwk_closure null,
	lwk_composite_linknet cnet,
	lwk_integer domain,
	lwk_linknet *linknet
	)
#else
	(null, cnet, domain, linknet)
	lwk_closure null;
	lwk_composite_linknet cnet;
	lwk_integer domain;
	lwk_linknet *linknet;
#endif
{
    lwk_status  status;
    
    status = lwk_delete(linknet);

    if (status == lwk_s_success)
	return ((lwk_termination) 1);
    else
	return ((lwk_termination) 0);
}
#endif /* MEMEX */
