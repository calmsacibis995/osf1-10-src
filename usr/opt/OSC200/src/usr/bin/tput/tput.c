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
static char rcsid[] = "@(#)$RCSfile: tput.c,v $ $Revision: 4.3.7.3 $ (DEC) $Date: 1993/10/11 19:25:59 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: CMDTTY terminal control commands
 *
 * FUNCTIONS: main (tput.c)
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.6  com/cmd/tty/tput.c, cmdtty, bos320, 9130320 7/8/91 13:50:50
 */

/* tput - print terminal attribute
 *
 * return codes:
 * 	 0 	string written successfully
 *	 2	usage error
 *	 3	bad terminal type given
 *	 4	unknown capname
 * 
 *	 0	ok (if boolean capname -> TRUE)
 *	 1	(for boolean capname  -> FALSE)
 *
 * tput printfs (a value) if an INT capname was given (e.g. cols),
 *	putp's (a string) if a STRING capname was given (e.g. clear), and
 *	for BOOLEAN capnames (e.g. hard-copy) just returns the boolean value. 
 */

#include <locale.h>
#include <curses.h>
#include <curshdr.h>
#include <unistd.h>
#include <term.h>
#include <string.h>
#include <stdlib.h>

#include        <nl_types.h>
#include        "tput_msg.h"

nl_catd catd;
#define MSGSTR(num,str) catgets(catd,MS_TPUT,num,str)

void usage(void){
	fprintf(stderr, MSGSTR(USAGE, "usage: tput [-T term] capname\n"));
	exit(2);
}

main (argc,argv)
char **argv;
{
	int err;
	int xcode;
	int opt;
	char cap[256];
	char *term=NULL;

	setlocale(LC_ALL,"") ;
	catd = catopen(MF_TPUT, NL_CAT_LOCALE);
	
	while ((opt = getopt(argc, argv, "T:")) != -1) {
		switch(opt) {
			case 'T':
				term = optarg;
				break;
			default:
				usage();
				/* break; */
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();
	(void)strncpy(cap, argv[0], sizeof(cap)-1);

	if (term)
		setupterm(term,1,&err);
	else
		setupterm(NULL,1,&err); 	/* use TERM environment var */

	if (err <= 0) {
	    if (term) {
		fprintf(stderr,
			MSGSTR(BADTRM1, "tput: unknown terminal \"%s\"\n"),
			       term);
		exit(3);
	    } else {
		fprintf(stderr,
			MSGSTR(BSDTRM2, "tput: unknown terminal \"%s\"\n"),
			       getenv("TERM"));
		exit(3);
	    }
	}
	xcode = 0;
	if (strcmp(cap, "reset") == 0)
		do_init(1);
	else if (strcmp(cap, "init") == 0)
		do_init(0);
	else if (strcmp(cap, "longname") == 0)
		printf("%s\n", longname());
	else {
		xcode = capindex(cap);
		resetterm();
	}
	exit(xcode);
}



capindex (cap)
char *cap;
{
	if (strcmp(cap,	"bw") == 0)	return(1-auto_left_margin);
	if (strcmp(cap,	"am") == 0)	return(1-auto_right_margin);
#ifdef beehive_glitch
	if (strcmp(cap,	"xsb") == 0)	return(1-beehive_glitch);
#endif
	if (strcmp(cap,	"xhp") == 0)	return(1-ceol_standout_glitch);
	if (strcmp(cap,	"xenl") == 0)	return(1-eat_newline_glitch);
	if (strcmp(cap,	"eo") == 0)	return(1-erase_overstrike);
	if (strcmp(cap,	"gn") == 0)	return(1-generic_type);
	if (strcmp(cap,	"hc") == 0)	return(1-hard_copy);
	if (strcmp(cap,	"km") == 0)	return(1-has_meta_key);
	if (strcmp(cap,	"hs") == 0)	return(1-has_status_line);
	if (strcmp(cap,	"in") == 0)	return(1-insert_null_glitch);
	if (strcmp(cap,	"da") == 0)	return(1-memory_above);
	if (strcmp(cap,	"db") == 0)	return(1-memory_below);
	if (strcmp(cap,	"mir") == 0)	return(1-move_insert_mode);
	if (strcmp(cap,	"msgr") == 0)	return(1-move_standout_mode);
	if (strcmp(cap,	"os") == 0)	return(1-over_strike);
	if (strcmp(cap,	"eslok") == 0)	return(1-status_line_esc_ok);
	if (strcmp(cap,	"xt") == 0)	return(1-teleray_glitch);
	if (strcmp(cap,	"hz") == 0)	return(1-tilde_glitch);
	if (strcmp(cap,	"ul") == 0)	return(1-transparent_underline);
	if (strcmp(cap,	"xon") == 0)	return(1-xon_xoff);

	if (strcmp(cap,	"cols") == 0) 	{ printf("%d\n",columns); return(0); }
	if (strcmp(cap,	"it") == 0) 	{ printf("%d\n",init_tabs); return(0); }
	if (strcmp(cap,	"lines") == 0) 	{ printf("%d\n",lines); return(0); }
	if (strcmp(cap,	"lm") == 0) 	{ printf("%d\n",lines_of_memory); return(0); }
	if (strcmp(cap,	"xmc") == 0) { printf("%d\n",magic_cookie_glitch); return(0); }
	if (strcmp(cap,	"pb") == 0)  { printf("%d\n",padding_baud_rate); return(0); }
	if (strcmp(cap,	"vt") == 0) 	{ printf("%d\n",virtual_terminal); return(0); }
	if (strcmp(cap,	"wsl") == 0) { printf("%d\n",width_status_line); return(0); }

	if (strcmp(cap,	"cbt") == 0)	{ putp(back_tab);	return(0); }
	if (strcmp(cap,	"bel") == 0)	{ putp(bell);	return(0); }
	if (strcmp(cap,	"cr") == 0)	{ putp(carriage_return);	return(0); }
	if (strcmp(cap,	"csr") == 0)	{ putp(change_scroll_region);	return(0); }
	if (strcmp(cap,	"tbc") == 0)	{ putp(clear_all_tabs);	return(0); }
	if (strcmp(cap,	"clear") == 0)	{ putp(clear_screen);	return(0); }
	if (strcmp(cap,	"el") == 0)	{ putp(clr_eol);	return(0); }
	if (strcmp(cap,	"ed") == 0)	{ putp(clr_eos);	return(0); }
	if (strcmp(cap,	"hpa") == 0)	{ putp(column_address);	return(0); }
	if (strcmp(cap,	"cmdch") == 0)	{ putp(command_character);	return(0); }
	if (strcmp(cap,	"cup") == 0)	{ putp(cursor_address);	return(0); }
	if (strcmp(cap,	"cud1") == 0)	{ putp(cursor_down);	return(0); }
	if (strcmp(cap,	"home") == 0)	{ putp(cursor_home);	return(0); }
	if (strcmp(cap,	"civis") == 0)	{ putp(cursor_invisible);	return(0); }
	if (strcmp(cap,	"cub1") == 0)	{ putp(cursor_left);	return(0); }
	if (strcmp(cap,	"mrcup") == 0)	{ putp(cursor_mem_address);	return(0); }
	if (strcmp(cap,	"cnorm") == 0)	{ putp(cursor_normal);	return(0); }
	if (strcmp(cap,	"cuf1") == 0)	{ putp(cursor_right);	return(0); }
	if (strcmp(cap,	"ll") == 0)	{ putp(cursor_to_ll);	return(0); }
	if (strcmp(cap,	"cuu1") == 0)	{ putp(cursor_up);	return(0); }
	if (strcmp(cap,	"cvvis") == 0)	{ putp(cursor_visible);	return(0); }
	if (strcmp(cap,	"dch1") == 0)	{ putp(delete_character);	return(0); }
	if (strcmp(cap,	"dl1") == 0)	{ putp(delete_line);	return(0); }
	if (strcmp(cap,	"dsl") == 0)	{ putp(dis_status_line);	return(0); }
	if (strcmp(cap,	"hd") == 0)	{ putp(down_half_line);	return(0); }
	if (strcmp(cap,	"smacs") == 0)	{ putp(enter_alt_charset_mode);	return(0); }
	if (strcmp(cap,	"blink") == 0)	{ putp(enter_blink_mode);	return(0); }
	if (strcmp(cap,	"bold") == 0)	{ putp(enter_bold_mode);	return(0); }
	if (strcmp(cap,	"smcup") == 0)	{ putp(enter_ca_mode);	return(0); }
	if (strcmp(cap,	"smdc") == 0)	{ putp(enter_delete_mode);	return(0); }
	if (strcmp(cap,	"dim") == 0)	{ putp(enter_dim_mode);	return(0); }
	if (strcmp(cap,	"smir") == 0)	{ putp(enter_insert_mode);	return(0); }
	if (strcmp(cap,	"prot") == 0)	{ putp(enter_protected_mode);	return(0); }
	if (strcmp(cap,	"rev") == 0)	{ putp(enter_reverse_mode);	return(0); }
	if (strcmp(cap,	"invis") == 0)	{ putp(enter_secure_mode);	return(0); }
	if (strcmp(cap,	"smso") == 0)	{ putp(enter_standout_mode);	return(0); }
	if (strcmp(cap,	"smul") == 0)	{ putp(enter_underline_mode);	return(0); }
	if (strcmp(cap,	"ech") == 0)	{ putp(erase_chars);	return(0); }
	if (strcmp(cap,	"rmacs") == 0)	{ putp(exit_alt_charset_mode);	return(0); }
	if (strcmp(cap,	"sgr0") == 0)	{ putp(exit_attribute_mode);	return(0); }
	if (strcmp(cap,	"rmcup") == 0)	{ putp(exit_ca_mode);	return(0); }
	if (strcmp(cap,	"rmdc") == 0)	{ putp(exit_delete_mode);	return(0); }
	if (strcmp(cap,	"rmir") == 0)	{ putp(exit_insert_mode);	return(0); }
	if (strcmp(cap,	"rmso") == 0)	{ putp(exit_standout_mode);	return(0); }
	if (strcmp(cap,	"rmul") == 0)	{ putp(exit_underline_mode);	return(0); }
	if (strcmp(cap,	"flash") == 0)	{ putp(flash_screen);	return(0); }
	if (strcmp(cap,	"ff") == 0)	{ putp(form_feed);	return(0); }
	if (strcmp(cap,	"fsl") == 0)	{ putp(from_status_line);	return(0); }
	if (strcmp(cap,	"is1") == 0)	{ putp(init_1string);	return(0); }
	if (strcmp(cap,	"is2") == 0)	{ putp(init_2string);	return(0); }
	if (strcmp(cap,	"is3") == 0)	{ putp(init_3string);	return(0); }
	if (strcmp(cap,	"if") == 0)	{ putp(init_file);	return(0); }
	if (strcmp(cap,	"ich1") == 0)	{ putp(insert_character);	return(0); }
	if (strcmp(cap,	"il1") == 0)	{ putp(insert_line);	return(0); }
	if (strcmp(cap,	"ip") == 0)	{ putp(insert_padding);	return(0); }
	if (strcmp(cap,	"kbs") == 0)	{ putp(key_backspace);	return(0); }
	if (strcmp(cap,	"ktbc") == 0)	{ putp(key_catab);	return(0); }
	if (strcmp(cap,	"kclr") == 0)	{ putp(key_clear);	return(0); }
	if (strcmp(cap,	"kctab") == 0)	{ putp(key_ctab);	return(0); }
	if (strcmp(cap,	"kdch1") == 0)	{ putp(key_dc);	return(0); }
	if (strcmp(cap,	"kdl1") == 0)	{ putp(key_dl);	return(0); }
	if (strcmp(cap,	"kcud1") == 0)	{ putp(key_down);	return(0); }
	if (strcmp(cap,	"krmir") == 0)	{ putp(key_eic);	return(0); }
	if (strcmp(cap,	"kel") == 0)	{ putp(key_eol);	return(0); }
	if (strcmp(cap,	"ked") == 0)	{ putp(key_eos);	return(0); }
	if (strcmp(cap,	"kf0") == 0)	{ putp(key_f0);	return(0); }
	if (strcmp(cap,	"kf1") == 0)	{ putp(key_f1);	return(0); }
	if (strcmp(cap,	"kf10") == 0)	{ putp(key_f10);	return(0); }
	if (strcmp(cap,	"kf2") == 0)	{ putp(key_f2);	return(0); }
	if (strcmp(cap,	"kf3") == 0)	{ putp(key_f3);	return(0); }
	if (strcmp(cap,	"kf4") == 0)	{ putp(key_f4);	return(0); }
	if (strcmp(cap,	"kf5") == 0)	{ putp(key_f5);	return(0); }
	if (strcmp(cap,	"kf6") == 0)	{ putp(key_f6);	return(0); }
	if (strcmp(cap,	"kf7") == 0)	{ putp(key_f7);	return(0); }
	if (strcmp(cap,	"kf8") == 0)	{ putp(key_f8);	return(0); }
	if (strcmp(cap,	"kf9") == 0)	{ putp(key_f9);	return(0); }
	if (strcmp(cap,	"khome") == 0)	{ putp(key_home);	return(0); }
	if (strcmp(cap,	"kich1") == 0)	{ putp(key_ic);	return(0); }
	if (strcmp(cap,	"kil1") == 0)	{ putp(key_il);	return(0); }
	if (strcmp(cap,	"kcub1") == 0)	{ putp(key_left);	return(0); }
	if (strcmp(cap,	"kll") == 0)	{ putp(key_ll);	return(0); }
	if (strcmp(cap,	"knp") == 0)	{ putp(key_npage);	return(0); }
	if (strcmp(cap,	"kpp") == 0)	{ putp(key_ppage);	return(0); }
	if (strcmp(cap,	"kcuf1") == 0)	{ putp(key_right);	return(0); }
	if (strcmp(cap,	"kind") == 0)	{ putp(key_sf);	return(0); }
	if (strcmp(cap,	"kri") == 0)	{ putp(key_sr);	return(0); }
	if (strcmp(cap,	"khts") == 0)	{ putp(key_stab);	return(0); }
	if (strcmp(cap,	"kcuu1") == 0)	{ putp(key_up);	return(0); }
	if (strcmp(cap,	"rmkx") == 0)	{ putp(keypad_local);	return(0); }
	if (strcmp(cap,	"smkx") == 0)	{ putp(keypad_xmit);	return(0); }
	if (strcmp(cap,	"lf0") == 0)	{ putp(lab_f0);	return(0); }
	if (strcmp(cap,	"lf1") == 0)	{ putp(lab_f1);	return(0); }
	if (strcmp(cap,	"lf10") == 0)	{ putp(lab_f10);	return(0); }
	if (strcmp(cap,	"lf2") == 0)	{ putp(lab_f2);	return(0); }
	if (strcmp(cap,	"lf3") == 0)	{ putp(lab_f3);	return(0); }
	if (strcmp(cap,	"lf4") == 0)	{ putp(lab_f4);	return(0); }
	if (strcmp(cap,	"lf5") == 0)	{ putp(lab_f5);	return(0); }
	if (strcmp(cap,	"lf6") == 0)	{ putp(lab_f6);	return(0); }
	if (strcmp(cap,	"lf7") == 0)	{ putp(lab_f7);	return(0); }
	if (strcmp(cap,	"lf8") == 0)	{ putp(lab_f8);	return(0); }
	if (strcmp(cap,	"lf9") == 0)	{ putp(lab_f9);	return(0); }
	if (strcmp(cap,	"smm") == 0)	{ putp(meta_on);	return(0); }
	if (strcmp(cap,	"rmm") == 0)	{ putp(meta_off);	return(0); }
	if (strcmp(cap,	"nel") == 0)	{ putp(newline);	return(0); }
	if (strcmp(cap,	"pad") == 0)	{ putp(pad_char);	return(0); }
	if (strcmp(cap,	"dch") == 0)	{ putp(parm_dch);	return(0); }
	if (strcmp(cap,	"dl") == 0)	{ putp(parm_delete_line);	return(0); }
	if (strcmp(cap,	"cud") == 0)	{ putp(parm_down_cursor);	return(0); }
	if (strcmp(cap,	"ich") == 0)	{ putp(parm_ich);	return(0); }
	if (strcmp(cap,	"indn") == 0)	{ putp(parm_index);	return(0); }
	if (strcmp(cap,	"il") == 0)	{ putp(parm_insert_line);	return(0); }
	if (strcmp(cap,	"cub") == 0)	{ putp(parm_left_cursor);	return(0); }
	if (strcmp(cap,	"cuf") == 0)	{ putp(parm_right_cursor);	return(0); }
	if (strcmp(cap,	"rin") == 0)	{ putp(parm_rindex);	return(0); }
	if (strcmp(cap,	"cuu") == 0)	{ putp(parm_up_cursor);	return(0); }
	if (strcmp(cap,	"pfkey") == 0)	{ putp(pkey_key);	return(0); }
	if (strcmp(cap,	"pfloc") == 0)	{ putp(pkey_local);	return(0); }
	if (strcmp(cap,	"pfx") == 0)	{ putp(pkey_xmit);	return(0); }
	if (strcmp(cap,	"mc0") == 0)	{ putp(print_screen);	return(0); }
	if (strcmp(cap,	"mc4") == 0)	{ putp(prtr_off);	return(0); }
	if (strcmp(cap,	"mc5") == 0)	{ putp(prtr_on);	return(0); }
	if (strcmp(cap,	"rep") == 0)	{ putp(repeat_char);	return(0); }
	if (strcmp(cap,	"rs1") == 0)	{ putp(reset_1string);	return(0); }
	if (strcmp(cap,	"rs2") == 0)	{ putp(reset_2string);	return(0); }
	if (strcmp(cap,	"rs3") == 0)	{ putp(reset_3string);	return(0); }
	if (strcmp(cap,	"rf") == 0)	{ putp(reset_file);	return(0); }
	if (strcmp(cap,	"rc") == 0)	{ putp(restore_cursor);	return(0); }
	if (strcmp(cap,	"vpa") == 0)	{ putp(row_address);	return(0); }
	if (strcmp(cap,	"sc") == 0)	{ putp(save_cursor);	return(0); }
	if (strcmp(cap,	"ind") == 0)	{ putp(scroll_forward);	return(0); }
	if (strcmp(cap,	"ri") == 0)	{ putp(scroll_reverse);	return(0); }
	if (strcmp(cap,	"sgr") == 0)	{ putp(set_attributes);	return(0); }
	if (strcmp(cap,	"hts") == 0)	{ putp(set_tab);	return(0); }
	if (strcmp(cap,	"wind") == 0)	{ putp(set_window);	return(0); }
	if (strcmp(cap,	"ht") == 0)	{ putp(tab);	return(0); }
	if (strcmp(cap,	"tsl") == 0)	{ putp(to_status_line);	return(0); }
	if (strcmp(cap,	"uc") == 0)	{ putp(underline_char);	return(0); }
	if (strcmp(cap,	"hu") == 0)	{ putp(up_half_line);	return(0); }
	if (strcmp(cap,	"iprog") == 0)	{ putp(init_prog);	return(0); }
	if (strcmp(cap,	"ka1") == 0)	{ putp(key_a1);	return(0); }
	if (strcmp(cap,	"ka3") == 0)	{ putp(key_a3);	return(0); }
	if (strcmp(cap,	"kb2") == 0)	{ putp(key_b2);	return(0); }
	if (strcmp(cap,	"kc1") == 0)	{ putp(key_c1);	return(0); }
	if (strcmp(cap,	"kc3") == 0)	{ putp(key_c3);	return(0); }
	if (strcmp(cap,	"mc5p") == 0)	{ putp(prtr_non);	return(0); }
	if (strcmp(cap,	"box1") == 0)	{ putp(box_chars_1);	return(0); }
	if (strcmp(cap,	"box2") == 0)	{ putp(box_chars_2);	return(0); }
	if (strcmp(cap,	"batt1") == 0)	{ putp(box_attr_1);	return(0); }
	if (strcmp(cap,	"batt2") == 0)	{ putp(box_attr_2);	return(0); }
	if (strcmp(cap,	"colb0") == 0)	{ putp(color_bg_0);	return(0); }
	if (strcmp(cap,	"colb1") == 0)	{ putp(color_bg_1);	return(0); }
	if (strcmp(cap,	"colb2") == 0)	{ putp(color_bg_2);	return(0); }
	if (strcmp(cap,	"colb3") == 0)	{ putp(color_bg_3);	return(0); }
	if (strcmp(cap,	"colb4") == 0)	{ putp(color_bg_4);	return(0); }
	if (strcmp(cap,	"colb5") == 0)	{ putp(color_bg_5);	return(0); }
	if (strcmp(cap,	"colb6") == 0)	{ putp(color_bg_6);	return(0); }
	if (strcmp(cap,	"colb7") == 0)	{ putp(color_bg_7);	return(0); }
	if (strcmp(cap,	"colf0") == 0)	{ putp(color_fg_0);	return(0); }
	if (strcmp(cap,	"colf1") == 0)	{ putp(color_fg_1);	return(0); }
	if (strcmp(cap,	"colf2") == 0)	{ putp(color_fg_2);	return(0); }
	if (strcmp(cap,	"colf3") == 0)	{ putp(color_fg_3);	return(0); }
	if (strcmp(cap,	"colf4") == 0)	{ putp(color_fg_4);	return(0); }
	if (strcmp(cap,	"colf5") == 0)	{ putp(color_fg_5);	return(0); }
	if (strcmp(cap,	"colf6") == 0)	{ putp(color_fg_6);	return(0); }
	if (strcmp(cap,	"colf7") == 0)	{ putp(color_fg_7);	return(0); }
	if (strcmp(cap,	"font0") == 0)	{ putp(font_0);	return(0); }
	if (strcmp(cap,	"font1") == 0)	{ putp(font_1);	return(0); }
	if (strcmp(cap,	"font2") == 0)	{ putp(font_2);	return(0); }
	if (strcmp(cap,	"font3") == 0)	{ putp(font_3);	return(0); }
	if (strcmp(cap,	"font4") == 0)	{ putp(font_4);	return(0); }
	if (strcmp(cap,	"font5") == 0)	{ putp(font_5);	return(0); }
	if (strcmp(cap,	"font6") == 0)	{ putp(font_6);	return(0); }
	if (strcmp(cap,	"font7") == 0)	{ putp(font_7);	return(0); }
	if (strcmp(cap,	"kbtab") == 0)	{ putp(key_back_tab);	return(0); }
	if (strcmp(cap,	"kdo") == 0)	{ putp(key_do);	return(0); }
	if (strcmp(cap,	"kcmd") == 0)	{ putp(key_command);	return(0); }
	if (strcmp(cap,	"kcpn") == 0)	{ putp(key_command_pane);	return(0); }
	if (strcmp(cap,	"kend") == 0)	{ putp(key_end);	return(0); }
	if (strcmp(cap,	"khlp") == 0)	{ putp(key_help);	return(0); }
	if (strcmp(cap,	"knl") == 0)	{ putp(key_newline);	return(0); }
	if (strcmp(cap,	"knpn") == 0)	{ putp(key_next_pane);	return(0); }
	if (strcmp(cap,	"kpcmd") == 0)	{ putp(key_prev_cmd);	return(0); }
	if (strcmp(cap,	"kppn") == 0)	{ putp(key_prev_pane);	return(0); }
	if (strcmp(cap,	"kquit") == 0)	{ putp(key_quit);	return(0); }
	if (strcmp(cap,	"ksel") == 0)	{ putp(key_select);	return(0); }
	if (strcmp(cap,	"kscl") == 0)	{ putp(key_scroll_left);	return(0); }
	if (strcmp(cap,	"kscr") == 0)	{ putp(key_scroll_right);	return(0); }
	if (strcmp(cap,	"ktab") == 0)	{ putp(key_tab);	return(0); }
	if (strcmp(cap,	"kmpf1") == 0)	{ putp(key_smap_in1);	return(0); }
	if (strcmp(cap,	"kmpt1") == 0)	{ putp(key_smap_out1);	return(0); }
	if (strcmp(cap,	"kmpf2") == 0)	{ putp(key_smap_in2);	return(0); }
	if (strcmp(cap,	"kmpt2") == 0)	{ putp(key_smap_out2);	return(0); }
	if (strcmp(cap,	"kmpf3") == 0)	{ putp(key_smap_in3);	return(0); }
	if (strcmp(cap,	"kmpt3") == 0)	{ putp(key_smap_out3);	return(0); }
	if (strcmp(cap,	"kmpf4") == 0)	{ putp(key_smap_in4);	return(0); }
	if (strcmp(cap,	"kmpt4") == 0)	{ putp(key_smap_out4);	return(0); }
	if (strcmp(cap,	"kmpf5") == 0)	{ putp(key_smap_in5);	return(0); }
	if (strcmp(cap,	"kmpt5") == 0)	{ putp(key_smap_out5);	return(0); }
	if (strcmp(cap,	"apstr") == 0)	{ putp(appl_defined_str);	return(0); }
	if (strcmp(cap,	"kmpf6") == 0)	{ putp(key_smap_in6);	return(0); }
	if (strcmp(cap,	"kmpt6") == 0)	{ putp(key_smap_out6);	return(0); }
	if (strcmp(cap,	"kmpf7") == 0)	{ putp(key_smap_in7);	return(0); }
	if (strcmp(cap,	"kmpt7") == 0)	{ putp(key_smap_out7);	return(0); }
	if (strcmp(cap,	"kmpf8") == 0)	{ putp(key_smap_in8);	return(0); }
	if (strcmp(cap,	"kmpt8") == 0)	{ putp(key_smap_out8);	return(0); }
	if (strcmp(cap,	"kmpf9") == 0)	{ putp(key_smap_in9);	return(0); }
	if (strcmp(cap,	"kmpt9") == 0)	{ putp(key_smap_out9);	return(0); }
	if (strcmp(cap,	"ksf1") == 0)	{ putp(key_sf1);	return(0); }
	if (strcmp(cap,	"ksf2") == 0)	{ putp(key_sf2);	return(0); }
	if (strcmp(cap,	"ksf3") == 0)	{ putp(key_sf3);	return(0); }
	if (strcmp(cap,	"ksf4") == 0)	{ putp(key_sf4);	return(0); }
	if (strcmp(cap,	"ksf5") == 0)	{ putp(key_sf5);	return(0); }
	if (strcmp(cap,	"ksf6") == 0)	{ putp(key_sf6);	return(0); }
	if (strcmp(cap,	"ksf7") == 0)	{ putp(key_sf7);	return(0); }
	if (strcmp(cap,	"ksf8") == 0)	{ putp(key_sf8);	return(0); }
	if (strcmp(cap,	"ksf9") == 0)	{ putp(key_sf9);	return(0); }
	if (strcmp(cap,	"ksf10") == 0)	{ putp(key_sf10);	return(0); }
	if (strcmp(cap,	"kf11") == 0)	{ putp(key_f11);	return(0); }
	if (strcmp(cap,	"kf12") == 0)	{ putp(key_f12);	return(0); }
	if (strcmp(cap,	"kf13") == 0)	{ putp(key_f13);	return(0); }
	if (strcmp(cap,	"kf14") == 0)	{ putp(key_f14);	return(0); }
	if (strcmp(cap,	"kf15") == 0)	{ putp(key_f15);	return(0); }
	if (strcmp(cap,	"kf16") == 0)	{ putp(key_f16);	return(0); }
	if (strcmp(cap,	"kf17") == 0)	{ putp(key_f17);	return(0); }
	if (strcmp(cap,	"kf18") == 0)	{ putp(key_f18);	return(0); }
	if (strcmp(cap,	"kf19") == 0)	{ putp(key_f19);	return(0); }
	if (strcmp(cap,	"kf20") == 0)	{ putp(key_f20);	return(0); }
	if (strcmp(cap,	"kf21") == 0)	{ putp(key_f21);	return(0); }
	if (strcmp(cap,	"kf22") == 0)	{ putp(key_f22);	return(0); }
	if (strcmp(cap,	"kf23") == 0)	{ putp(key_f23);	return(0); }
	if (strcmp(cap,	"kf24") == 0)	{ putp(key_f24);	return(0); }
	if (strcmp(cap,	"kf25") == 0)	{ putp(key_f25);	return(0); }
	if (strcmp(cap,	"kf26") == 0)	{ putp(key_f26);	return(0); }
	if (strcmp(cap,	"kf27") == 0)	{ putp(key_f27);	return(0); }
	if (strcmp(cap,	"kf28") == 0)	{ putp(key_f28);	return(0); }
	if (strcmp(cap,	"kf29") == 0)	{ putp(key_f29);	return(0); }
	if (strcmp(cap,	"kf30") == 0)	{ putp(key_f30);	return(0); }
	if (strcmp(cap,	"kf31") == 0)	{ putp(key_f31);	return(0); }
	if (strcmp(cap,	"kf32") == 0)	{ putp(key_f32);	return(0); }
	if (strcmp(cap,	"kf33") == 0)	{ putp(key_f33);	return(0); }
	if (strcmp(cap,	"kf34") == 0)	{ putp(key_f34);	return(0); }
	if (strcmp(cap,	"kf35") == 0)	{ putp(key_f35);	return(0); }
	if (strcmp(cap,	"kf36") == 0)	{ putp(key_f36);	return(0); }
	if (strcmp(cap,	"kact") == 0)	{ putp(key_action);	return(0); }

	fprintf(stderr, MSGSTR(CAPNAME, "tput: unknown capname %s\n"), cap);
	return(4);
}	

/*
 * Do the initialize/reset sequence:
 * 
 * 	- Run the init program
 * 	- Put out the init/reset strings (is1/rs1, is2/rs2, is3/rs3)
 * 	- Output contents of init/reset file (if/rf)
 * 	- If terminal has tab character (ht) turn off tab expansion
 *		reset tabs and set them every 8 spaces
 * 	  otherwise turn tab expansion on
 */
int
do_init(reset)
int reset;			/* true if we are doing reset */
{
	char *filename;
	struct termios term;

	/*
	 * if the reset strings aren't present, we use the init strings
	 */
	if (reset && 
	   (reset_1string || reset_2string || reset_3string || reset_file )) {
		putp(reset_1string);
		putp(reset_2string);
		putp(reset_3string);
		filename = reset_file;
	} else {
		if (init_prog)		/* run the init program */
			system(init_prog);

		putp(init_1string);
		putp(init_2string);
		putp(init_3string);
		filename = init_file;
	}

	if (filename != NULL) {
		FILE *f;
		char line[LINE_MAX];

		if ((f = fopen(filename, "r")) != NULL) {
			while (fgets(line, sizeof(line), f) != NULL) {
				fputs(line,stdout);
			}
			fclose(f);
		}
	}

	tcgetattr(1, &term);
	if (tab) {
		term.c_oflag &= ~OXTABS;
		tcsetattr(1, TCSADRAIN, &term);
					/* set tabs every 8 spaces */
		putp(clear_all_tabs);
		settabs();
	} else {			/* expand tabs to spaces */
		term.c_oflag |= OXTABS;
		tcsetattr(1, TCSADRAIN, &term);
	}

}

static char buf[1024];
static char *p;

settabs()
{
	int i, column;
	int copy();

	p = buf;
	*p++ = '\r';

	column = 1;
	while (column <= (columns - 8)) {
		for (i=1; i<= 8 ; i++)
			*p++ = ' ';
		tputs(set_tab, 1, copy); /* have to use tput for padding */
		column += 8;
	}
	*p++ = '\r';
	*p = '\0';	/* better safe than sorry */

	putp(buf);

}

copy(c)
char c;
{
	*p++ = c;
}
