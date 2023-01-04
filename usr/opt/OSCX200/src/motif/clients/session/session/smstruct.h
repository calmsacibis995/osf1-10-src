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
#include	<X11/Xlib.h>
#include 	<X11/Intrinsic.h>
#include 	<Xm/Xm.h>
#include 	<Mrm/MrmPublic.h>

#define  ACCK_TERMLEN	84
#ifndef globaldef
#  define globaldef
#endif /* not globaldef */
#ifndef globalref
#  define globalref extern
#endif /* not globalref */

#include	"smshare.h"

#define	pt_size	1024
#define mbx_size 84
#define	quit	1
#define	save	2
#define	no	3
#define vue	1
#define term	2
#define cancel  1
#define yes     2
#define controller_max	10
#define prt_operation  1
#define create_operation  0
#define OPTION_LIST_SIZE 100
#define MAX_FILENAME_SIZE 256
#define MAX_DISPLAY_NAME_SIZE 256

extern	struct	screennumstruct
	{
	Widget	*the_widget;
	int *screennum;
	int operation;
	};

extern	struct smdata
	{
	Widget	toplevel;
	Widget		header_label;
	Widget		pause_label;
	Widget	create_menu;
	Widget	*create_button;
	int	create_count;
	Widget	sm_button;
	Widget	display_button;
	Widget	window_button;
	Widget	security_button;
	Widget	key_button;
	Widget	international_button;
	Widget	pointer_button;
	Widget	printer_button;
	Widget	appmenu_button;
	Widget	appdefs_button;
	Widget	autostart_button;
	Widget	use_last_button;
	Widget	use_system_button;
	Widget	save_current_button;
	Widget	err_window_id;
	Widget	caution_id;
        Widget  te_button;
        Widget  vue_button;
	Widget  pause_id;
        int     fcaution;
	int	err_ack; 
	Widget	pass2_text;
	Widget	pass2_label;
	Widget	print_es;
	Widget	print_pos;
	Widget	capture_pos;
	Widget	capture_es;
	Widget	quit_button;
	Widget	pause_button;
	int	just_managed_second;
	char		message[pt_size];
	int	resource_changed;
	Cursor	wait_cursor;
	Pixmap	icon;
	Pixmap  iconify;
	Pixmap	reverse_icon;
	Pixmap	reverse_iconify;
	Widget	end_caution_id;
	Widget	help_overview;
	Widget	help_about;
	Widget	help_widget;
	Widget	print_confirm_box_id;
	Widget	print_stext_id;
	char	print_destination[MAX_FILENAME_SIZE];
	Widget	screen_button;
	Widget	screen_confirm_box_id;
	Widget	*screen_confirm_list;
	struct	screennumstruct	screen_data;
	int	optionslist[OPTION_LIST_SIZE];
	char	display_name[MAX_DISPLAY_NAME_SIZE];
	};


extern	struct  termdata
	{
	short	dev_chn;
	short	term_chn;
	short	mbx_chn;
	short	mbx_unit;
	char	pt_buffer[pt_size];
	char	mbx_buffer[mbx_size];
	struct	statusblock	pty_status;
	struct	statusblock	mbx_status;
	};

extern	struct	creprc_struct
	{
    	unsigned    short   chan;
	unsigned    int	    pid;
    	struct	creprc_struct	*next;
	};

