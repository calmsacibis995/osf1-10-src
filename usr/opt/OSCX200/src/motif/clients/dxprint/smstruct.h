/*
!****************************************************************************
!*									    *
!*  COPYRIGHT (c) 1988, 1989, 1991, 1992				    *
!*  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
!*  ALL RIGHTS RESERVED.						    *
!* 									    *
!*  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
!*  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
!*  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
!*  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
!*  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
!*  TRANSFERRED.							    *
!* 									    *
!*  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
!*  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
!*  CORPORATION.							    *
!* 									    *
!*  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
!*  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
!* 									    *
!*									    *
!****************************************************************************
!++
! FACILITY:  PrintScreen
!
! ABSTRACT:
!
!
! ENVIRONMENT:
!
!	VAX/VMS operating system.
!
! AUTHOR:  Karen Brouillette December 1989
!
! Modified by:
!
!	29-Jan-1992	Edward P Luwish
!		Remove smdata.print_destination
!
!	09-Apr-1991	Edward P Luwish
!		Port to Motif UI
!
*/

/* BuildSystemHeader added automatically */
/* $Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/smstruct.h,v 1.2 91/12/30 12:48:20 devbld Exp $ */

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <X11/Xlib.h>
#include <Xm/XmP.h>
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#ifdef VMS
#include	<accdef.h>
#else
#define  ACC$K_TERMLEN	84
#endif

#if !defined(VMS) || defined(__DECC)
#ifndef globaldef
#define globaldef
#endif /* not globaldef */
#ifndef globalref
#define globalref extern
#endif /* not globalref */
#endif

#if defined(VMS)
#include <descrip.h>
#endif

#define	quit	1
#define	save	2
#define	no	3
#define prt_operation  1
#define create_operation  0

extern	struct	screennumstruct
	{
	Widget	*the_widget;
	int *screennum;
	int operation;
	};

extern	struct smdata
	{
	Widget	toplevel;
	Widget	message_area;
	Widget	header_label;
	Widget	pagesize_panel;
	Widget	sixel_panel;
	Widget	printer_button;
	Widget	postopbutton;
	Widget	sixopbutton;
	Widget	use_last_button;
	Widget	use_system_button;
	Widget	save_current_button;
	Widget	err_window_id;
	Widget	caution_id;
	Widget	print_es;
	Widget	print_pos;
	Widget	capture_pos;
	Widget	capture_es;
	Widget	quit_button;
	int	message_area_pos;
	int	resource_changed;
	Cursor	wait_cursor;
	Pixmap	icon;
	Pixmap  iconify;
	Pixmap	reverse_icon;
	Pixmap	reverse_iconify;
	Widget	end_caution_id;
/*	Widget	help_overview; */
/*	Widget	help_about; */
	Widget  help_window;
	Widget	help_version;
	Widget	help_context;
	Widget	help_help;
	Widget	help_widget;
	Widget	print_confirm_box_id;
	Widget	print_stext_id;
	char	*print_destination;
	Widget	screen_button;
	Widget	screen_confirm_box_id;
	Widget	*screen_confirm_list;
	struct	screennumstruct	screen_data;
	};

