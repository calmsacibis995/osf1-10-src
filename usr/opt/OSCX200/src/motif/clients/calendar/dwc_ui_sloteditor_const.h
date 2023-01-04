#ifndef _dwc_ui_sloteditor_const_h_
#define _dwc_ui_sloteditor_const_h_
/* $Header$ */
/* DWC_UI_SLOTEDITOR_CONST.H "V3.0-004" */
/*
**
**  Copyright (c) 1990
**  by DIGITAL Equipment Corporation, Maynard, Mass.
**
**  This software is furnished under a license and may be used and  copied
**  only  in  accordance  with  the  terms  of  such  license and with the
**  inclusion of the above copyright notice.  This software or  any  other
**  copies  thereof may not be provided or otherwise made available to any
**  other person.  No title to and ownership of  the  software  is  hereby
**  transferred.
**
**  The information in this software is subject to change  without  notice
**  and  should  not  be  construed  as  a commitment by DIGITAL Equipment
**  Corporation.
**
**  DIGITAL assumes no responsibility for the use or  reliability  of  its
**  software on equipment which is not supplied by DIGITAL.
**
**++
**
** Facility:
**
** Abstract:
**	The constants are indexes into the view's widget array.
**
** Author:
**	Denis G. Lacroix
**
** Creation Date: 1-Feb-89
**
** Modification History:
**
** V3.0-004 Paul Ferwerda				    21-Nov-1990
**		Converted from SDL file to H file to aid ULTRIX builds.
**    2-003	Denis G. Lacroix			    29-Mar-1990
**		Added new constants for new widgets in the dialog box.
**    2-002	Ken Cowan				    14-Mar-1989
**		Change name of NUM_FYI local symbol to NUM_WIDGETS.
**    2-001	Denis G. Lacroix			    1-Feb-1990
**		Initial entry.
*/ 

/* indexes in the widget array                                              */
#define k_ts_popup_attached_db 0

#define k_ts_include_menu_item 1
#define k_ts_cut_menu_item 2
#define k_ts_copy_menu_item 3
#define k_ts_paste_menu_item 4
#define k_ts_clear_menu_item 5
#define k_ts_selectall_menu_item 6

#define k_ts_ok_button 7
#define k_ts_delete_button 8
#define k_ts_reset_button 9
#define k_ts_cancel_button 10

#define k_ts_date_label 11
#define k_ts_from_label 12
#define k_ts_to_label 13

#define k_ts_timeslot_stext 14

#define k_ts_entry_time_toggle 15
#define k_ts_short_toggle 16
#define k_ts_medium_toggle 17
#define k_ts_long_toggle 18
#define k_ts_vlong_toggle 19
#define k_ts_short_val_toggle 20        /* MUST be k_ts_short_toggle + 4    */
#define k_ts_medium_val_toggle 21       /* MUST be k_ts_medium_toggle + 4   */
#define k_ts_long_val_toggle 22         /* MUST be k_ts_long_toggle + 4     */
#define k_ts_vlong_val_toggle 23        /* MUST be k_ts_vlong_toggle + 4    */
#define k_ts_alarm_scale 24

#define k_ts_repeat_optmenu 25
#define	k_ts_none_button 26
#define	k_ts_daily_button 27
#define	k_ts_weekly_button 28
#define	k_ts_fortnight_button 29
#define	k_ts_fourweek_button 30
#define	k_ts_monthly_button 31
#define	k_ts_bimonthly_button 32
#define	k_ts_quarterly_button 33
#define	k_ts_triannually_button 34
#define k_ts_biannually_button 35
#define k_ts_annually_button 36

#define k_ts_attributes_optmenu 37
#define k_ts_nth_day_button 38
#define k_ts_nth_day_end_button 39
#define k_ts_nth_xday_button 40
#define k_ts_last_weekday_button 41
#define k_ts_special_day_menu_entry 42
#define k_ts_the_day_at_or_label 43
#define k_ts_after_6_button 44          /* Do *NOT* reorder or take appart! */
#define k_ts_after_5_button 45
#define k_ts_after_4_button 46
#define k_ts_after_3_button 47
#define k_ts_after_2_button 48
#define k_ts_after_1_button 49
#define k_ts_after_0_button 50
#define k_ts_before_0_button 51
#define k_ts_before_1_button 52
#define k_ts_before_2_button 53
#define k_ts_before_3_button 54
#define k_ts_before_4_button 55
#define k_ts_before_5_button 56
#define k_ts_before_6_button 57

#define k_ts_condition_toggle 58

#define k_ts_work_nonwork_optmenu 59
#define k_ts_work_button 60
#define k_ts_nonwork_button 61

#define k_ts_move_optmenu 62
#define k_ts_skip_button 63
#define k_ts_forward_button 64
#define k_ts_backward_button 65

#define k_ts_flags_toggle 66

#define k_ts_select_ibw 67
#define k_ts_order_ibw 68

#define k_ts_icon_1_toggle 69           /* Do *NOT* reorder and take apart  */
#define k_ts_icon_2_toggle 70
#define k_ts_icon_3_toggle 71
#define k_ts_icon_4_toggle 72
#define k_ts_icon_5_toggle 73
#define k_ts_icon_6_toggle 74
#define k_ts_icon_7_toggle 75
#define k_ts_icon_8_toggle 76
#define k_ts_icon_9_toggle 77
#define k_ts_icon_10_toggle 78
#define k_ts_icon_11_toggle 79
#define k_ts_icon_12_toggle 80
#define k_ts_icon_13_toggle 81
#define k_ts_icon_14_toggle 82
#define k_ts_icon_15_toggle 83
#define k_ts_icon_16_toggle 84
#define k_ts_icon_17_toggle 85
#define k_ts_icon_18_toggle 86
#define k_ts_icon_19_toggle 87
#define k_ts_icon_20_toggle 88
#define k_ts_icon_21_toggle 89
#define k_ts_icon_22_toggle 90
#define k_ts_icon_23_toggle 91
#define k_ts_icon_24_toggle 92

/* size of the timeslot widget array                                        */
#define TIMESLOT_WIDGET_ARRAY_SIZE 93

/* file menu item chosen                                                    */

#define k_include 0

/* repeat interval                                                          */
#define k_ts_none 0
#define k_ts_daily 1
#define k_ts_weekly 2
#define k_ts_fortnight 3
#define k_ts_fourweek 4
#define k_ts_monthly 5
#define k_ts_bimonthly 6
#define k_ts_quarterly 7
#define k_ts_biannually 8
#define k_ts_annually 9

#endif /* end of _dwc_ui_sloteditor_const_h_ */
