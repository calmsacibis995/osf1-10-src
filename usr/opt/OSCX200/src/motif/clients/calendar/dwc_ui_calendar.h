#ifndef _calendar_h_
#define _calendar_h_ 1
/* $Header$ */
/* #module dwc_ui_calendar.h */
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
**	Marios Cleovoulou, March-1988
**
**  ABSTRACT:
**
**	Generic calendar information.
**
**--
*/



/*
**  Include Files
*/
#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/XmP.h>		/* for XmDefaultFont... */
#include <Mrm/MrmPublic.h>	/* for MrmHierarchy */
#include <X11/StringDefs.h>	/* for XtRString */
#include <DXm/DECspecific.h>
#ifdef vaxc
#pragma standard
#endif

#if MEMEX
#include    <lwk_dxm_def.h>
#endif

#include "dwc_compat.h"
#include "dwc_ui_sloteditor_const.h"
#include "dwc_ui_calendar_const.h"
#include "dwc_ui_dayslotswidget.h"
#include "dwc_ui_yeardisplay.h"
#include "dwc_ui_monthwidget.h"	
#include "dwc_ui_layout.h"		/* for LayoutWidget */

/*	  
**  Replaces the gone DwtChildren and DwtNumChildren
*/	  
#define DWC_CHILDREN(w) \
    (((CompositeRec *)(w))->composite.children)
#define DWC_NUM_CHILDREN(w) \
    (((CompositeRec *)(w))->composite.num_children)

typedef enum {show_day, show_week, show_month, show_year} show_kind;


/*	  
**  This profile gets stored in the database in a big record. If you muck with
**  the middle of it you're going to make all the profile info useless when
**  users try to read in their old profiles. So, the moral is, if you are going
**  to change it, you better add the proper db upgrade procedure. The above is
**  why with and heigh are left as Cardinals insted of Dimensions as they should
**  be.
*/	  
typedef struct
{
    show_kind		first_display;

    Cardinal		day_width;
    Cardinal		day_height;

    Cardinal		week_width;
    Cardinal		week_height;

    Cardinal		month_width;
    Cardinal		month_height;

    Cardinal		year_width;
    Cardinal		year_height;

    unsigned char	first_day_of_week;
    unsigned char	days_to_include;
    unsigned char	work_days;

    Boolean		show_week_numbers;

    unsigned char	week_numbers_start_day;
    unsigned char	week_numbers_start_month;

    unsigned char	timeslot_size;
    
    unsigned char	padd_1;	/* for ultrix alignment */
    
    unsigned short int	work_minutes_start;
    unsigned short int	work_minutes_end;

    Boolean		day_show_months;   /* DayView Customize */
    Boolean		week_show_months;
    Boolean		fill_blank_days;

    Boolean		start_iconized;

    Boolean		time_am_pm;

    unsigned char	padd_2;	/* for ultrix alignment */

    Position		day_x;
    Position		day_y;
    
    Position		week_x;
    Position		week_y;
    
    Position		month_x;
    Position		month_y;
    
    Position		year_x;
    Position		year_y;
    
    /*
    **  Here we are at a word boundary.  70 bytes used so far....
    */

    Dimension		size_font_units;	/* ? */
    Position		screen_width;		/* ? */
    Position		screen_height;		/* ? */
    
    Boolean		directionRtoL;		/* FALSE */
    Boolean		day_show_notes;	/* TRUE  */
    Boolean		day_show_full_times;	/* FALSE */

    unsigned char	fine_increment;	/* 5 */    
    
    unsigned short int	lunch_minutes_start;	/* 12 * 60 */
    unsigned short int	lunch_minutes_end;	/* 13 * 60 */

    unsigned char	day_v_spacing;		/* 4? */
    unsigned char	day_h_spacing;		/* 3? */
    unsigned char       day_timebar_width;     /* 4? */

    Boolean		times_on_the_line;	/* FALSE */
    Boolean		timeslot_stacking;	/* FALSE */
    unsigned char	day_font_size;		/* (0, 1, 2) 1 */
    unsigned char	day_icon_size;		/* (0, 1, 2) 1 */

    unsigned char	default_entry_icon;	/* meeting icon */
    unsigned char	default_notes_icon;	/* pencil icon */

    Boolean		direct_scroll_coupling;/* FALSE */
    Boolean		entries_significant;	/* TRUE  */
    Boolean		notes_significant;	/* TRUE  */

    char		icon_text [32];	/* blank */
    Boolean		icon_show_text;	/* FALSE */
    Boolean		icon_nl_after_text;	/* FALSE */
    Boolean		icon_show_day;		/* TRUE */
    Boolean		icon_full_day;		/* TRUE */
    Boolean		icon_nl_after_day;	/* TRUE */
    Boolean		icon_show_time;	/* TRUE */

    unsigned short int	alarms [8];		/* 0, 5, ... */
    unsigned char	alarms_mask;		/* bits r-l  */

    Boolean		print_blank_days;	/* TRUE */
    Boolean		auto_click_on_clock;	/* TRUE */
    unsigned char	month_style_mask;	/* style_square */

    Position		alarm_x;
    Cardinal		alarm_width;
    Cardinal		alarm_height;
    Position		alarm_y;

    Position		sloteditor_x;
    Cardinal		sloteditor_width;
    Cardinal		sloteditor_height;
    Position		sloteditor_y;

    Position		noteeditor_x;
    Cardinal		noteeditor_width;
    Cardinal		noteeditor_height;
    Position		noteeditor_y;

    /*
    **  Now we are at a word boundary.  190 bytes used so far.
    */

    
} ProfileStructure;

typedef struct _DwcDayItemRecord
{
    int				    item_id;
    Cardinal			    start_time;
    Cardinal			    duration;
    int				    repeat_p1;
    int				    repeat_p2;
    int				    repeat_p3;
    int				    repeat_p4;	/* Not used		    */
    int				    repeat_p5;	/* Not used		    */
    int				    repeat_p6;	/* Not used		    */
    int				    alarms_number;
    unsigned short int		    *alarms_times;
    Cardinal			    icons_number;
    unsigned char		    *icons;
    Cardinal			    last_day;
    Cardinal			    last_time;
    DwcDswEntry			    entry;
    int				    flags;
    Boolean			    memexized;
} DwcDayItemRecord, *DwcDayItem;


typedef struct _DwcDaySlotsRecord
{
    Widget			widget;
    Cardinal			number_of_items;
    DwcDayItem			*items;
    DwcDayItem			default_item;
} DwcDaySlotsRecord, *DwcDaySlots;


typedef enum
{
    RCKNotRepeat,
    RCKThisInstance,
    RCKThisAndFuture,
    RCKAllInstances,
    RCKRepeatChange
} RepeatChangeKind;


typedef enum {SlotEditor, DaynoteEditor} WhichEditor;
	      
/*
**  Everthing associated with a calendar....
*/

struct _AllDisplaysRecord
{
    Widget			    root;		    /*		    */
    Widget			    requestor;		    /*		    */
    MrmHierarchy		    hierarchy;		    /*		    */ 
    struct _CalendarDisplayRecord  **cds;
    Cardinal			    number_of_calendars;   /*		    */
    Cursor			    wait_cursor;	    /*		    */
    Cursor			    inactive_cursor;	    /*		    */
    char			    *filespec;		    /*		    */
};

/* 
    The CustomButtons structure is needed because CalendarDisplaysRecord
    was too large for one of the Ultrix compilers.  Be careful
    when growing CalendarDisplaysRecord

*/

struct _CustomButtons
{
    Widget	tb_tss_60;
    Widget	tb_tss_30;
    Widget	tb_tss_20;
    Widget	tb_tss_15;
    Widget	tb_tss_12;
    Widget	tb_tss_10;
    Widget	tb_tss_06;
    Widget	tb_tss_05;
    Widget	tb_tss_04;
    Widget	tb_tss_03;
    Widget	tb_tss_02;
    Widget	tb_tss_01;
    Widget	fine_incr_60_pb;
    Widget	fine_incr_30_pb;
    Widget	fine_incr_20_pb;
    Widget	fine_incr_15_pb;
    Widget	fine_incr_12_pb;
    Widget	fine_incr_10_pb;
    Widget	fine_incr_6_pb;
    Widget	fine_incr_5_pb;
    Widget	fine_incr_4_pb;
    Widget	fine_incr_3_pb;
    Widget	fine_incr_2_pb;
    Widget	fine_incr_1_pb;
};

struct _CalendarDisplayRecord
{
    struct _AllDisplaysRecord	*ads;
    YearDisplay			*yd;
    YearDisplay			*day_yd;
    ProfileStructure		profile;
    ProfileStructure		original;
    DwcDaySlotsRecord		dayslots;
    DwcDaySlotsRecord		daynotes;
    show_kind			showing;
    show_kind			first_dpy; /* current first display */
    CalendarBlock		*cb;
    struct DWC$db_access_block	*cab;
    struct _AlarmOutRecord	**alarms_out;
    struct _SloteditorRecord	**ses;
    struct _DayItemUpdateRecord *timeslot_diu;
    DwcDswEntry			*selected_entries;
    MwTypeSelected		print_arg_type;
    struct _CustomButtons	custom_buttons;

    Widget			toplevel;
    Widget			mainwid;
    Widget			wrapper;
    Widget			menubar;
    Widget			main_help_widget;
    Widget			active_widget;
    Widget			button_timer_widget;
    /*
    **  Month display widgets...
    */

    LayoutWidget		lw_month_display;
    Widget			month_scrollbar;
    Widget			month_display;

    /*
    **  Week display widgets etc...
    */


    /*
    **  Day display widgets etc...
    */

    LayoutWidget		lw_day_display;
    Widget			day_label;
    Widget			pb_day_time;
    Widget			day_scrollbar;
    LayoutWidget		lw_day_scrollwin;
    Widget			day_wrkhrsbar;
    Widget			pb_day_next;
    Widget			pb_day_last;
    Widget			day_sep_1;
    Widget			day_sep_2;
    Widget			db_repeat_questions;
    Widget			lb_repeat_prompt;
    Widget			rb_repeat_questions;
    Widget			tb_repeat_this;
    Widget			tb_repeat_future;
    Widget			tb_repeat_all;
    Widget			pb_repeat_ok;
    Widget			pb_repeat_cancel;
    Widget			timeslot_small_font_label;
    Widget			timeslot_medium_font_label;
    Widget			timeslot_large_font_label;
    Widget			day_daynote_default_icon_tb;
    Widget			day_slot_default_icon_tb;
    Widget			day_default_icon_box_widget;
    Widget			month_style_slash_pb;
    Widget			month_style_cross_pb;
    Widget			month_style_clean_pb;
    Widget			alarm_days_tb;
    Widget			alarm_days_scale;
    Widget			auto_click_clock_tb;
    Widget			month_style_menu;
    Widget			box_current_day_tb;
    Widget			pull_entry; 
    Widget			pb_open_new;
    Widget			db_open_new;
    Widget			sloteditor_widget_array [11];
    Widget			pb_open;
    Widget			pb_close;
    Widget			db_nameas;
    Widget			pb_nameas;
    Widget			tw_nameas;
    Widget			pb_remove;
    Widget			pb_delete;
    Widget			pb_file_include;          
    Widget			pb_file_exit;          
    Widget			db_print;
    Widget			pb_print;
    Widget			db_print_dg;
    Widget			db_print_lb_week;
    Widget			db_print_lb_date;
    Widget			db_print_lb_start;
    Widget			db_print_lb_finish;
    Widget			tb_print_create;
    Widget			tb_print_append;
    Widget			tw_print;
    Widget			sw_print;
    Widget			tb_print_limit;
    /*
    **  Edit menu pushbuttons
    */
    
    Widget			pb_edit_cut;              
    Widget			pb_edit_copy;             
    Widget			pb_edit_paste;
    Widget			pb_edit_clear;            
    Widget			pb_edit_selall;           
							       
    /*                                                         
    **  View menu pushbuttons                                  
    */                                                         
							       
    Widget			pb_selected;              
    Widget			pb_today;                 
    Widget			pb_day;                   
    Widget			pb_week;                  
    Widget			pb_month;                 
    Widget			pb_year;                  

    /*
    **  Schedule menu pushbutton/pull-downs
    */

    Widget			pb_entry_menu;
    Widget			pb_delete_menu;
    Widget			pb_close_menu;
    Widget			pb_reset_menu;
    
    /*
    **  Customize menu.
    */

    Widget			pb_custom_save;                  
    
    /*
    **  Customize settings dialog box.
    */

    Widget			general_pb;
    Widget			dayview_pb;
    Widget			iconbox_pb;
    Widget			alarms_pb;
    Widget			general_db;
    Widget			dayview_db;
    Widget			iconbox_db;
    Widget			alarms_db;
    Widget			day_show_months;
    Widget			day_entry_significant;
    Widget			day_daynote_significant;
    Widget			wkd [7];              
    Widget			month_swn;            
    Widget			month_fbd;
    Widget			fdow_option_menu;
    Widget			fdow [7];

    /*
    **  These are used in the customize general box to allow the user to
    **  select the day of a particular month from which to start counting
    **  weeks.
    */
    Widget			week_num_start_month_om;
    Widget			week_num_start_month_pbs[12];
    Widget			week_num_start_day_scale;

    Widget			om_tss;
    Widget			om_first_dpy;
    Widget                      first_dpy_year;  
    Widget                      first_dpy_month; 
    Widget                      first_dpy_week;  
    Widget                      first_dpy_day;   
    Widget                      start_icon_tb;  
    Widget			time_am_pm;
    Widget			pb_work_hours_start_hours;     
    Widget			pb_work_hours_start_minutes;   
    Widget			lb_work_hours_start_ampm;      
    Widget			pb_work_hours_finish_hours;    
    Widget			pb_work_hours_finish_minutes;  
    Widget			lb_work_hours_finish_ampm;     
    Widget			pb_options_cust;
    Widget			pb_options_mark;
    Widget			pb_mark_default;
    Widget			pb_mark_normal;
    Widget			pb_mark_work_day;
    Widget			pb_mark_non_work_day;
    Widget			pb_mark_special;
    Widget			db_open;
    Widget			db_include_file;
    Widget			cb_delete;
    Widget			r_to_l_tb;
    Widget			fine_incr_om;
    Widget			stacking_om;
    Widget			stack_r_l_pb;
    Widget			stack_top_down_pb;
    Widget			font_small_tb;
    Widget			font_medium_tb;
    Widget			font_large_tb;
    Widget			icon_small_tb;
    Widget			icon_medium_tb;
    Widget			icon_large_tb;
    Widget			scrollbar_coupling_tb;
    Widget			times_on_line_tb;
    Widget			show_full_times_tb;
    Widget			show_day_notes_tb;
    Widget			vertical_spacing_scale;
    Widget			horizontal_spacing_scale;
    Widget			timebar_width_scale;
    Widget			lunch_start_hour_pb;
    Widget			lunch_start_minute_pb;
    Widget			lunch_start_am_pm_lb;
    Widget			lunch_finish_hour_pb;
    Widget			lunch_finish_minute_pb;
    Widget			lunch_finish_am_pm_lb;
    Widget			print_blank_tb;
    Widget			iconbox_show_text_tb;
    Widget			iconbox_text_stext;
    Widget			iconbox_nl_after_text_toggle;
    Widget			iconbox_show_day_toggle;
    Widget			iconbox_full_day_toggle;
    Widget			iconbox_nl_after_day_toggle;
    Widget			iconbox_show_time_toggle;   
    Widget			alarm_entry_tb;
    Widget			alarm_hours_tb;
    Widget			alarm_hours_scale;
    Widget			alarm_minutes_tb;
    Widget			alarm_minutes_scale;
    Widget			alarm_min_bis_tb;
    Widget			alarm_min_bis_scale;
    Widget			pull_memex;
    /*
    ** Confirmation buttons in customize dialogs.
    */
    Widget			pb_gen_controls;
    Widget			pb_gen_dummy1;
    Widget			pb_gen_dummy2;
    Widget			pb_gen_dummy3;
    Widget			pb_gen_dummy4;
    Widget			pb_icon_controls;
    Widget			   	pb_icon_dummy1;
    Widget			pb_icon_dummy2;
    Widget			pb_icon_dummy3;
    Widget			pb_icon_dummy4;
    Widget			pb_alarm_controls;
    Widget			pb_alarm_dummy1;
    Widget			pb_alarm_dummy2;
    Widget			pb_alarm_dummy3;
    Widget			pb_alarm_dummy4;
    Widget			pb_day_controls;
    Widget			pb_day_dummy1;
    Widget			pb_day_dummy2;
    Widget			pb_day_dummy3;
    Widget			fm_nameas_controls;

    XtIntervalId		button_timer_id;
    XtIntervalId		clock_id;
    XtIntervalId		alarm_id;
    XmFontList			icon_day_fontlist;
    XmFontList			icon_month_fontlist;
    XmFontList			title_day_fontlist;

    Cardinal			day;
    Cardinal			month;
    Cardinal			year;
    Cardinal			last_day;
    Cardinal			last_month;
    Cardinal			last_year;
    Cardinal			alarms_out_number;	
    Cardinal			number_of_sloteditors;	
    Cardinal			number_of_selected_entries;
    Cardinal			last_timer_day;
    Cardinal			print_arg_day;
    Cardinal			print_arg_week;
    Cardinal			print_arg_month;
    Cardinal			print_arg_year;
    Cardinal			print_arg_start;
    Cardinal			print_arg_end;

    unsigned int		alarm_day;
    int				filler_1;
    int				day_scroll_min;
    int				day_scroll_max;
    int				width_of_screen;
    int				height_of_screen;
    int				repeat_interval;	
    int                         slot_start;                 
    int                         slot_end;                   
    int                         dsbot;                      
    int				tss;		/* current timeslot size */
    int				curr_ysmon;	/* current month for week 1 */

    unsigned short int		work_hours_start_time;
    unsigned short int		work_hours_finish_time;
    unsigned short int		alarm_time;
    unsigned short int		lunch_hours_start_time;
    unsigned short int		lunch_hours_finish_time;
	
    Dimension			screen_font_size;

    Boolean			read_only;
    Boolean			iconised;
    Boolean			zero_time_up;
    Boolean			button_timer_fired;
    Boolean			current_is_entry;
    Boolean			db_print_up;
    Boolean			delete_pending;
    Boolean			db_nameas_created;
    Boolean			db_print_created;
    Boolean			db_general_up;
    Boolean			db_general_created;
    Boolean			db_dayview_up;
    Boolean			db_dayview_created;
    Boolean			db_iconbox_up;
    Boolean			db_iconbox_created;
    Boolean			db_alarms_up;
    Boolean			db_alarms_created;
    Boolean			auto_scroll_day;
    Boolean			default_highlighting;	/* Memex */

    unsigned char		current_entry_icon;
    unsigned char		current_notes_icon;
    char			*filespec;
    char			*filename;
    XmString			print_arg_text;
    char			*appl_title;
    char			*alarm_icon_title;	/* ???!!! Zap this*/

#if MEMEX
    lwk_dxm_ui			hisdwui;
#endif

    Boolean			mapped;
    Opaque			help_context;
    Boolean			db_print_text_changed;
};


struct _AlarmOutRecord
{
    Widget				popup;
    int					item_id;
    Cardinal				day;
    Widget				date_lb;
    Widget				start_lb;
    Widget				finish_lb;
    Widget				alarm_tw;
    Widget				alarm_ibw;
    Cardinal				time;
    struct _CalendarDisplayRecord	*cd;	/* Back pointer	    */
    Cardinal				duration;
    Cardinal				alarm_time;
    Cardinal				alarm_day;
};

struct _SloteditorRecord
{
    struct _CalendarDisplayRecord	*cd;	/* Back pointer	    */
    Widget				file_selection_widget;
    Widget				sloteditor_widget_array
	[TIMESLOT_WIDGET_ARRAY_SIZE];
    WhichEditor				editor;	/*		    */
    Cardinal				dsbot;
    Cardinal				start_time;
    Cardinal				duration;
    Cardinal				repeat_interval;
    Cardinal				attributes;
    Cardinal				work_nonwork;
    Cardinal				move;
    Cardinal				current_scale;
    Cardinal				scale_latest_value[4];
    int					cwd_day_of_month;
    struct _DwcDayItemRecord		*old;
    DwcDswEntry				link;
    DwcDaySlots				slots;
    Boolean				in_use;	/*		    */
    Boolean				text_changed;
    Boolean				stext_changed;
    Boolean				icons_changed;
    Boolean				repeat_changed;
    Boolean				condition;
    Boolean				cwd_cond_forward;
    Boolean				alarms_changed;
    Boolean				ignore_scale_value_changed;
    Boolean				flag_changed;
    XmString				text;
};	
	    
struct _DayItemUpdateRecord
{
    struct _CalendarDisplayRecord	*cd;
    struct _SloteditorRecord		*se;
    struct _DwcDayItemRecord		*old;
    struct _DwcDayItemRecord		*new;
    Cardinal				dsbot;
    RepeatChangeKind			kind;
    DwcDaySlots				slots;    	    
    Boolean				changed;
    XmString				text;
};


typedef struct _AllDisplaysRecord     AllDisplaysRecord,     *AllDisplays;
typedef struct _CalendarDisplayRecord CalendarDisplayRecord, *CalendarDisplay;
typedef struct _SloteditorRecord      SloteditorRecord,	     *Sloteditor;
typedef struct _DaynoteRecord	      DaynoteRecord,	     *Daynote;
typedef struct _DayItemRecord         DayItemRecord,         *DayItem;
typedef struct _DayItemUpdateRecord   DayItemUpdateRecord,   *DayItemUpdate;
typedef struct _AlarmOutRecord	      AlarmOutRecord,	     *AlarmOut;

#define CalendarTopAppName    "Calendar"
#ifdef VMS
#define CalendarTopClassName	"DECW$CALENDAR"
#else
#define CalendarTopClassName	"DXcalendar"
#endif

#define CalendarDisplayAppName    "Calendar"
#define CalendarDisplayClassName    "CalendarDisplay"	

/*
**  UID files
*/

#ifdef	VMS
#define	    DwcTCalendarUIDFile		"DECW$CALENDAR"
#else	/*VMS*/
#define		DwcTCalendarUIDFile	"DXcalendar"
#endif	/*VMS*/

#define DwcNiconDayFontList	"iconDayFontList"
#define DwcCIconDayFontList	"IconDayFontList"
#define DwcNiconMonthFontList	"iconMonthFontList"
#define DwcCIconMonthFontList	"IconMonthFontList"
#define DwcNtitleDayFontList	"titleDayFontList"
#define DwcCTitleDayFontList	"TitleDayFontList"

typedef enum {wt_none,  wt_menu,  wt_dialog,  wt_radio, wt_label,
	      wt_pushb, wt_toggleb, wt_hline, wt_help,  wt_scale,
	      wt_files, wt_filew, wt_pushd, wt_stext, wt_modald,
	      gt_label, gt_hline, gt_pushb, gt_pushd, gt_toggleb} wt_kinds ;

typedef struct widget_tree {
    wt_kinds		widget_type ;
    Cardinal		widget ;
    struct widget_tree	*branch ;
    XtCallbackList	callback ;
    char		*text ;
} widget_tree, *widget_tree_ptr ;

/*
**  This is the hackery we need to load and reference the pixmap defined in
**  UIL.
*/



#define k_pixmap_meeting    0
#define k_pixmap_faces	    1
#define k_pixmap_slides	    2
#define k_pixmap_announce   3
#define k_pixmap_car	    4
#define k_pixmap_plane	    5
#define k_pixmap_phone	    6
#define k_pixmap_letter	    7
#define k_pixmap_pencil	    8
#define k_pixmap_note	    9
#define k_pixmap_milestone  10
#define k_pixmap_leave	    11
#define k_pixmap_coffee	    12
#define k_pixmap_lunch	    13
#define k_pixmap_doctor	    14
#define k_pixmap_money	    15
#define k_pixmap_info	    16
#define k_pixmap_exclam	    17
#define k_pixmap_question   18
#define k_pixmap_cake	    19
#define k_pixmap_flag	    20
#define k_pixmap_saints	    21
#define k_pixmap_school	    22
#define k_pixmap_games	    23


#define k_pixmap_setable_start	0   /* Don't change!			    */
#define k_pixmap_setable_end	23

#define k_pixmap_memex		251
#define k_pixmap_bell		252
#define k_pixmap_repeat		253
#define k_pixmap_repeatstart	254
#define k_pixmap_repeatend	255

#define k_pixmap_computed_start	251
#define k_pixmap_computed_end	255 /* Don't change!			    */

#define k_pixmap_count		k_pixmap_computed_end	-   \
				k_pixmap_computed_start +   \
				k_pixmap_setable_end	-   \
				k_pixmap_setable_start	+   \
				2

void CALCreateYearDisplay PROTOTYPE ((CalendarDisplay cd));

XtAppContext CALGetAppContext PROTOTYPE (());

void CALStartupCalendar PROTOTYPE ((CalendarDisplay cd));

void CALDestroyCalendar PROTOTYPE ((CalendarDisplay cd));

void CALStartTimeTextWorkProc PROTOTYPE ((CalendarDisplay cd));

/*
** Locale identification macros for Asian locales.
*/
#define IsAsianLocale(s) (((s) != NULL) && \
	((((s)[0]=='j') && ((s)[1]=='a')) || \
	 (((s)[0]=='k') && ((s)[1]=='o')) || \
	 (((s)[0]=='z') && ((s)[1]=='h'))))

#define IsJaLocale(s) (((s) != NULL) && \
	(((s)[0]=='j') && ((s)[1]=='a')))
#define IsKoLocale(s) (((s) != NULL) && \
	(((s)[0]=='k') && ((s)[1]=='o')))
#define IsCNLocale(s) (((s) != NULL) && \
	(((s)[0]=='z') && ((s)[1]=='h') && ((s)[2]=='_') && \
	 ((s)[3]=='C') && ((s)[4]=='N')))
#define IsTWLocale(s) (((s) != NULL) && \
	(((s)[0]=='z') && ((s)[1]=='h') && ((s)[2]=='_') && \
	 ((s)[3]=='T') && ((s)[4]=='W')))

#endif /* _calendar_h_ */
