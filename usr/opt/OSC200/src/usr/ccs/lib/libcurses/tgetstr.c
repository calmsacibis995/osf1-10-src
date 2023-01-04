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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tgetstr.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:41:10 $";
#endif
/*
 * HISTORY
 */
/*** "tgetstr.c  1.6  com/lib/curses,3.1,9008 12/14/89 17:53:30"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   tgetstr
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *	NOTE:
 * 		This module is not actually built in to the library
 * 		Is is generated from termcap.form.
 *		See the Makefile and termcap.ed.
 */


#include "cursesext.h"

#define	two( s1, s2 )	(s1 + 256 * s2 )
#define	twostr( str )	two( *str, str[ 1 ] )

/*
 * NAME:        tgetstr
 *
 * FUNCTION:
 *
 *      Simulation of termcap using terminfo.
 */

char *
tgetstr(id, area)
char *id, *area;
{
	char *rv;

	switch (twostr(id)) {
	case two('b','t'): rv = back_tab; break;
	case two('b','l'): rv = bell; break;
	case two('c','r'): rv = carriage_return; break;
	case two('c','s'): rv = change_scroll_region; break;
	case two('c','t'): rv = clear_all_tabs; break;
	case two('c','l'): rv = clear_screen; break;
	case two('c','e'): rv = clr_eol; break;
	case two('c','d'): rv = clr_eos; break;
	case two('c','h'): rv = column_address; break;
	case two('C','C'): rv = command_character; break;
	case two('c','m'): rv = cursor_address; break;
	case two('d','o'): rv = cursor_down; break;
	case two('h','o'): rv = cursor_home; break;
	case two('v','i'): rv = cursor_invisible; break;
	case two('l','e'): rv = cursor_left; break;
	case two('C','M'): rv = cursor_mem_address; break;
	case two('v','e'): rv = cursor_normal; break;
	case two('n','d'): rv = cursor_right; break;
	case two('l','l'): rv = cursor_to_ll; break;
	case two('u','p'): rv = cursor_up; break;
	case two('v','s'): rv = cursor_visible; break;
	case two('d','c'): rv = delete_character; break;
	case two('d','l'): rv = delete_line; break;
	case two('d','s'): rv = dis_status_line; break;
	case two('h','d'): rv = down_half_line; break;
	case two('a','s'): rv = enter_alt_charset_mode; break;
	case two('m','b'): rv = enter_blink_mode; break;
	case two('m','d'): rv = enter_bold_mode; break;
	case two('t','i'): rv = enter_ca_mode; break;
	case two('d','m'): rv = enter_delete_mode; break;
	case two('m','h'): rv = enter_dim_mode; break;
	case two('i','m'): rv = enter_insert_mode; break;
	case two('m','k'): rv = enter_secure_mode; break;
	case two('m','p'): rv = enter_protected_mode; break;
	case two('m','r'): rv = enter_reverse_mode; break;
	case two('s','o'): rv = enter_standout_mode; break;
	case two('u','s'): rv = enter_underline_mode; break;
	case two('e','c'): rv = erase_chars; break;
	case two('a','e'): rv = exit_alt_charset_mode; break;
	case two('m','e'): rv = exit_attribute_mode; break;
	case two('t','e'): rv = exit_ca_mode; break;
	case two('e','d'): rv = exit_delete_mode; break;
	case two('e','i'): rv = exit_insert_mode; break;
	case two('s','e'): rv = exit_standout_mode; break;
	case two('u','e'): rv = exit_underline_mode; break;
	case two('v','b'): rv = flash_screen; break;
	case two('f','f'): rv = form_feed; break;
	case two('f','s'): rv = from_status_line; break;
	case two('i','1'): rv = init_1string; break;
	case two('i','s'): rv = init_2string; break;
	case two('i','2'): rv = init_3string; break;
	case two('i','f'): rv = init_file; break;
	case two('i','c'): rv = insert_character; break;
	case two('a','l'): rv = insert_line; break;
	case two('i','p'): rv = insert_padding; break;
	case two('k','b'): rv = key_backspace; break;
	case two('k','a'): rv = key_catab; break;
	case two('k','C'): rv = key_clear; break;
	case two('k','t'): rv = key_ctab; break;
	case two('k','D'): rv = key_dc; break;
	case two('k','L'): rv = key_dl; break;
	case two('k','d'): rv = key_down; break;
	case two('k','M'): rv = key_eic; break;
	case two('k','E'): rv = key_eol; break;
	case two('k','S'): rv = key_eos; break;
	case two('k','0'): rv = key_f0; break;
	case two('k','1'): rv = key_f1; break;
	case two('k',';'): rv = key_f10; break;
	case two('k','2'): rv = key_f2; break;
	case two('k','3'): rv = key_f3; break;
	case two('k','4'): rv = key_f4; break;
	case two('k','5'): rv = key_f5; break;
	case two('k','6'): rv = key_f6; break;
	case two('k','7'): rv = key_f7; break;
	case two('k','8'): rv = key_f8; break;
	case two('k','9'): rv = key_f9; break;
	case two('k','h'): rv = key_home; break;
	case two('k','I'): rv = key_ic; break;
	case two('k','A'): rv = key_il; break;
	case two('k','l'): rv = key_left; break;
	case two('k','H'): rv = key_ll; break;
	case two('k','N'): rv = key_npage; break;
	case two('k','P'): rv = key_ppage; break;
	case two('k','r'): rv = key_right; break;
	case two('k','F'): rv = key_sf; break;
	case two('k','R'): rv = key_sr; break;
	case two('k','T'): rv = key_stab; break;
	case two('k','u'): rv = key_up; break;
	case two('k','e'): rv = keypad_local; break;
	case two('k','s'): rv = keypad_xmit; break;
	case two('l','0'): rv = lab_f0; break;
	case two('l','1'): rv = lab_f1; break;
	case two('l','a'): rv = lab_f10; break;
	case two('l','2'): rv = lab_f2; break;
	case two('l','3'): rv = lab_f3; break;
	case two('l','4'): rv = lab_f4; break;
	case two('l','5'): rv = lab_f5; break;
	case two('l','6'): rv = lab_f6; break;
	case two('l','7'): rv = lab_f7; break;
	case two('l','8'): rv = lab_f8; break;
	case two('l','9'): rv = lab_f9; break;
	case two('m','o'): rv = meta_off; break;
	case two('m','m'): rv = meta_on; break;
	case two('n','w'): rv = newline; break;
	case two('p','c'): rv = pad_char; break;
	case two('D','C'): rv = parm_dch; break;
	case two('D','L'): rv = parm_delete_line; break;
	case two('D','O'): rv = parm_down_cursor; break;
	case two('I','C'): rv = parm_ich; break;
	case two('S','F'): rv = parm_index; break;
	case two('A','L'): rv = parm_insert_line; break;
	case two('L','E'): rv = parm_left_cursor; break;
	case two('R','I'): rv = parm_right_cursor; break;
	case two('S','R'): rv = parm_rindex; break;
	case two('U','P'): rv = parm_up_cursor; break;
	case two('p','k'): rv = pkey_key; break;
	case two('p','l'): rv = pkey_local; break;
	case two('p','x'): rv = pkey_xmit; break;
	case two('p','s'): rv = print_screen; break;
	case two('p','f'): rv = prtr_off; break;
	case two('p','o'): rv = prtr_on; break;
	case two('r','p'): rv = repeat_char; break;
	case two('r','1'): rv = reset_1string; break;
	case two('r','2'): rv = reset_2string; break;
	case two('r','3'): rv = reset_3string; break;
	case two('r','f'): rv = reset_file; break;
	case two('r','c'): rv = restore_cursor; break;
	case two('c','v'): rv = row_address; break;
	case two('s','c'): rv = save_cursor; break;
	case two('s','f'): rv = scroll_forward; break;
	case two('s','r'): rv = scroll_reverse; break;
	case two('s','a'): rv = set_attributes; break;
	case two('s','t'): rv = set_tab; break;
	case two('w','i'): rv = set_window; break;
	case two('t','a'): rv = tab; break;
	case two('t','s'): rv = to_status_line; break;
	case two('u','c'): rv = underline_char; break;
	case two('h','u'): rv = up_half_line; break;
	case two('i','P'): rv = init_prog; break;
	case two('K','1'): rv = key_a1; break;
	case two('K','3'): rv = key_a3; break;
	case two('K','2'): rv = key_b2; break;
	case two('K','4'): rv = key_c1; break;
	case two('K','5'): rv = key_c3; break;
	case two('p','O'): rv = prtr_non; break;
	case two('b','x'): rv = box_chars_1; break;
	case two('b','y'): rv = box_chars_2; break;
	case two('B','x'): rv = box_attr_1; break;
	case two('B','y'): rv = box_attr_2; break;
	case two('d','0'): rv = color_bg_0; break;
	case two('d','1'): rv = color_bg_1; break;
	case two('d','2'): rv = color_bg_2; break;
	case two('d','3'): rv = color_bg_3; break;
	case two('d','4'): rv = color_bg_4; break;
	case two('d','5'): rv = color_bg_5; break;
	case two('d','6'): rv = color_bg_6; break;
	case two('d','7'): rv = color_bg_7; break;
	case two('c','0'): rv = color_fg_0; break;
	case two('c','1'): rv = color_fg_1; break;
	case two('c','2'): rv = color_fg_2; break;
	case two('c','3'): rv = color_fg_3; break;
	case two('c','4'): rv = color_fg_4; break;
	case two('c','5'): rv = color_fg_5; break;
	case two('c','6'): rv = color_fg_6; break;
	case two('c','7'): rv = color_fg_7; break;
	case two('f','0'): rv = font_0; break;
	case two('f','1'): rv = font_1; break;
	case two('f','2'): rv = font_2; break;
	case two('f','3'): rv = font_3; break;
	case two('f','4'): rv = font_4; break;
	case two('f','5'): rv = font_5; break;
	case two('f','6'): rv = font_6; break;
	case two('f','7'): rv = font_7; break;
	case two('k','O'): rv = key_back_tab; break;
	case two('k','i'): rv = key_do; break;
	case two('k','c'): rv = key_command; break;
	case two('k','W'): rv = key_command_pane; break;
	case two('k','w'): rv = key_end; break;
	case two('k','q'): rv = key_help; break;
	case two('k','n'): rv = key_newline; break;
	case two('k','v'): rv = key_next_pane; break;
	case two('k','p'): rv = key_prev_cmd; break;
	case two('k','V'): rv = key_prev_pane; break;
	case two('k','Q'): rv = key_quit; break;
	case two('k','U'): rv = key_select; break;
	case two('k','z'): rv = key_scroll_left; break;
	case two('k','Z'): rv = key_scroll_right; break;
	case two('k','o'): rv = key_tab; break;
	case two('K','v'): rv = key_smap_in1; break;
	case two('K','V'): rv = key_smap_out1; break;
	case two('K','w'): rv = key_smap_in2; break;
	case two('K','W'): rv = key_smap_out2; break;
	case two('K','x'): rv = key_smap_in3; break;
	case two('K','X'): rv = key_smap_out3; break;
	case two('K','y'): rv = key_smap_in4; break;
	case two('K','Y'): rv = key_smap_out4; break;
	case two('K','z'): rv = key_smap_in5; break;
	case two('K','Z'): rv = key_smap_out5; break;
	case two('z','a'): rv = appl_defined_str; break;
	default: rv = NULL;
	}
	return rv;
}
