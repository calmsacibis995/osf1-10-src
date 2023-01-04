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
#include "smconstants.h"

#define	mapped	0
#define iconified 1

#define ismanaged 1
#define notmanaged 0

#define	caps_value 0
#define shift_value 1

#define	enable_value 1
#define disable_value 0

#define F1_value 0
#define F2_value 1

#define none_mod 0
#define ctrl_mod 1
#define shift_mod 2

#define black	0
#define white	65535

#define right_handed	0
#define left_handed 1

#define small_icon 1
#define big_icon 0

#define	none	0
#define	slow	1
#define	medium	2
#define	fast	3
#define no_accel 4

#define mstate  0x00000001
#define mheader 0x00000002
#define mhistory  0x00000004
#define mcols 0x00000008
#define mrows 0x00000010
#define mpausetext 0x00000020
#define msmx 0x00000040
#define mconfirm 0x00000080
#define msmy 0x00000100

#define mratio 0x00000001
#define msaver 0x00000002
#define mcolor 0x00000004
#define mformat 0x00000008
#define mfile 0x00000010
#define mprtprompt 0x00000020
#define mrotateprompt 0x00000040

#define mbell 0x00000001
#define mbellvolume 0x00000002
#define mclick 0x00000004
#define mclickvolume 0x00000008
#define mautorepeat 0x00000010
#define mlockstate 0x00000020
#define mpointer 0x00000040
#define mcompose 0x00000080
#define moperator 0x00000100
#define moperator_mod 0x00000200
#define mkeyboard 0x00000400
#define mkeyboard_clear  0xFFFFFBFF
#define mkeyboard_mask 0x000007FF

#define mptforeground 0x00000001
#define mptbackground 0x00000002
#define mptbutton 0x00000004
#define mptaccel 0x00000008
#define mptdouble 0x00000010
#define mptcursor 0x00000020
#define mpt_mask 0x0000003F

#define minlanguage 0x00000001

#define mwinactive 0x00000001
#define mwininactive 0x00000002
#define mwinforeground 0x00000004
#define mwinbackground 0x00000008
#define mwinborder 0x00000010
#define mwincolors 0x0000001F
#define mdissaver 0x00000020
#define mdissaverseconds 0x00000040
#define mdispattern 0x00000080
#define mdisforeground 0x00000100
#define mdisbackground 0x00000200
#define mwiniconstyle 0x00000400
#define mwinbordercolor 0x00000800
#define mwinformforeground 0x00001000
#define mwinexe 0x00002000
#define mdisplay_mask 0x00002FFF
#define mwinmgr 0x00002400

#define mscreen_applprompt 0x00000001
#define mscreen_prtprompt 0x00000002
#define mscreen_applscreennum 0x00000004
#define mscreen_prtscreennum 0x00000008
#define mscreen_mask 0x0000000f

extern	struct smattr_di
	{
	unsigned int changed;
	Widget	sm_attr_id;
	Widget	state_id;
	unsigned int	startup_state;
	Widget	mapped_id;
	Widget	icon_id;
	Widget	header_id;
	char	*header_text;
	Widget	pause_id;
	char	*pause_text;
	Widget	history_id;
	unsigned int	history;
	unsigned int	cols;
	unsigned int	rows;
	unsigned int	x;
	unsigned int	y;
	unsigned int	end_confirm;
	unsigned int	end_confirm_id;
	char	managed;
        unsigned int    terms;
        unsigned int    vues;
	};

extern	struct prtattr_di
	{
	unsigned int changed;
	Widget	prt_attr_id;
	Widget	ratio_id;
	unsigned    int	ratio;
	Widget	r1to1_id;
	Widget	r2to1_id;
	Widget	prtcolor_id;
	unsigned    int	color;
	Widget	bw_id;
	Widget	grey_id;
	Widget	color_id;
	Widget	saver_id;
	unsigned    int	saver;
	Widget	positive_id;
	Widget	negative_id;
	Widget	format_id;
	unsigned    int	format;
	Widget	sixel_id;
	Widget	postscript_id;
	Widget	bitmap_id;
	Widget	capture_file_id;
	char capture_file[256];
	Widget file_prompt_id;
	unsigned    int	file_prompt;
	char	managed;
        Widget rotate_prompt_id;
        unsigned    int rotate_prompt;
	};

extern	struct keyattr_di
	{
	unsigned int changed;
	Widget	key_attr_id;
	Widget	bellonoff_id;
	unsigned    int	bell;
	Widget	bellenable_id;
	Widget	belldisable_id;
	Widget	bell_id;
	unsigned    int	bell_volume;
	Widget	clickonoff_id;
	unsigned    int	click;
	Widget	clickenable_id;
	Widget	clickdisable_id;
	Widget	click_id;
	unsigned    int	click_volume;
	Widget	autorepeat_id;
	unsigned    int	autorepeat;
	Widget	repeatenable_id;
	Widget	repeatdisable_id;
	unsigned    int	lock;
	Widget	lock_id;
	Widget	caps_id;
	Widget	shift_id;
	unsigned    int	operator;
	Widget	operator_id;
	Widget	operatorf1_id;
	Widget	operatorf2_id;
	unsigned    int	operator_mod;
	Widget	operator_mod_id;
	Widget	operator_ctrl_id;
	Widget	operator_shift_id;
	Widget	keyboard_id;
	Widget  kbdprobe_id;
	XmString	keyboard_selected;
	char	keyboard[256];
	char	managed;
	};

extern	struct	color_data
    {
    Widget  widget_list[10];
    unsigned	int by_name;
    char    name[50];
    XColor  color;
    XColor  mix;
    int     mix_allocated;
    int     mix_changed;
    char    managed;
    };

extern	struct	pointattr_di
    {
    unsigned int    changed;
    Widget  point_attr_id;
    struct  color_data	foreground;
    struct  color_data	background;
    Widget  button_id;
    unsigned	int button;
    Widget  right_id;
    Widget  left_id;
    Widget  accel_id;
    Widget  none_id, slow_id, med_id, fast_id;
    unsigned	int accel;
    int	accel_num;
    int	accel_denom;
    int	accel_threshold;
    Widget  double_id;
    unsigned	int double_time;
    Widget  cursor_id;
    int cursor_index;
    XmString   cursor_selected;
    char	managed;
    };

extern	struct	interattr_di
    {
    unsigned int    changed;
    Widget  inter_attr_id;
    Widget  SetLanguageListBox;
    int	    list_item_edited;
    int	    list_item_number;
    char	managed;
    };

extern	struct	windowattr_di
    {
    unsigned	int changed;
    Widget  window_attr_id;
    struct  color_data	activetitle;
    struct  color_data	inactivetitle;
    struct  color_data	foreground;
    struct  color_data	background;
    struct  color_data	winmgrbordercolor;
    struct  color_data	winmgrformforeground;
    unsigned	int borderwidth;
    Widget  border_id;
    struct  color_data	screen_background;
    struct  color_data	screen_foreground;
    Widget	saveronoff_id;
    unsigned    int	saver;
    Widget	saverenable_id;
    Widget	saverdisable_id;
    Widget	saver_id;
    unsigned	int saver_seconds;
    unsigned	int pattern_index;
    int		pattern_selected;
    Widget	icon_id;
    unsigned    int	icon_style;
    Widget	icon_small_id;
    Widget	icon_large_id;
    char	managed;
    Widget	wmexe_id;
    unsigned	int wmexe;
    Widget	wmexe_default_id;
    Widget	wmexe_other_id;
    Widget	wmother_text_id;
    char	*wmother;
    Widget	patterncurrent_id;
    Widget	patternfg_id;
    Widget	patternbg_id;
    Widget	patterndefault_id;
    Widget	palette_id;
    };

extern struct securityattr_di
	{
	Widget sec_attr_id;
 	int reset_listbox;
	int hosts_changed;
	char	managed;
	};

extern struct screenattr_di
	{
	unsigned int changed;
	Widget scr_attr_id;
	Widget appl_prompt_id;
	Widget prt_prompt_id;
	Widget *appl_scale_id;
	Widget *prt_scale_id;
	int appl_screennum;
	int appl_prompt;
	int prt_screennum;
	int prt_prompt;
	char	managed;
	};

extern struct appmenuattr_di
	{
	unsigned int changed;
	Widget menu_attr_id;
	Widget def_list_id;
	Widget menu_list_id;
	char	managed;
	};

extern struct appdefattr_di
	{
	unsigned int changed;
	Widget menu_attr_id;
	Widget menu_enter_id;
	Widget menu_text_id;
	Widget command_text_id;
	Widget def_list_id;
	XrmDatabase temp_db_id;
	char	managed;
	char	**remove_list;
	int	num_allocated;
	int	num_used;
	Widget menu_remove_id;
	};

extern struct autostartattr_di
	{
	unsigned int changed;
	Widget menu_attr_id;
	Widget def_list_id;
	Widget menu_list_id;
	char	managed;
	};

