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
static char rcsid[] = "@(#)$RCSfile: initkeypad.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/12 21:37:33 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "initkeypad.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:21:56"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _init_keypad, _addone
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

#ifdef	 	KEYPAD
static struct map *_addone();
/*
 * Make up the needed array of map structures for dealing with the keypad.
 */
#define MAXKEYS 75      /* number of keys we understand */

/*
 * NAME:        _init_keypad
 */

struct map *
_init_keypad()
{
	struct map *r, *p;

	r = (struct map *) calloc(MAXKEYS, sizeof (struct map));
	p = r;

	/* If down arrow key sends \n, don't map it. */
	if (key_down && strcmp(key_down, "\n"))
		p = _addone(p, key_down,	KEY_DOWN,	"down");
	p = _addone(p, key_up,		KEY_UP,		"up");
	/* If left arrow key sends \b, don't map it. */
	if (key_left && strcmp(key_left, "\b"))
		p = _addone(p, key_left,	KEY_LEFT,	"left");
	p = _addone(p, key_right,	KEY_RIGHT,	"right");
	p = _addone(p, key_home,	KEY_HOME,	"home");
	/* If backspace key sends \b, don't map it. */
	/* if (key_backspace && strcmp(key_backspace, "\b")) */
	p = _addone(p, key_backspace,   KEY_BACKSPACE,  "bs");
	p = _addone(p, key_enter,       KEY_ENTER,      "nl");	    /* 002 */
	p = _addone(p, key_help,        KEY_HELP,       "help");	/* 001 */
	p = _addone(p, key_back_tab,    KEY_BTAB,       "btab");	/* 001 */
	p = _addone(p, key_command,     KEY_COMMAND,    "cmd");		/* 001 */
	p = _addone(p, key_end,         KEY_END,        "end");		/* 001 */
	p = _addone(p, key_select,      KEY_SELECT,     "sel");		/* 001 */
	p = _addone(p, key_f0,		KEY_F(0),	lab_f0?lab_f0:"f0");
	p = _addone(p, key_f1,		KEY_F(1),	lab_f1?lab_f1:"f1");
	p = _addone(p, key_f2,		KEY_F(2),	lab_f2?lab_f2:"f2");
	p = _addone(p, key_f3,		KEY_F(3),	lab_f3?lab_f3:"f3");
	p = _addone(p, key_f4,		KEY_F(4),	lab_f4?lab_f4:"f4");
	p = _addone(p, key_f5,		KEY_F(5),	lab_f5?lab_f5:"f5");
	p = _addone(p, key_f6,		KEY_F(6),	lab_f6?lab_f6:"f6");
	p = _addone(p, key_f7,		KEY_F(7),	lab_f7?lab_f7:"f7");
	p = _addone(p, key_f8,		KEY_F(8),	lab_f8?lab_f8:"f8");
	p = _addone(p, key_f9,		KEY_F(9),	lab_f9?lab_f9:"f9");
	p = _addone(p, key_f10,         KEY_F(10),      "f10");
	p = _addone(p, key_f11,         KEY_F(11),      "f11");
	p = _addone(p, key_f12,         KEY_F(12),      "f12");
	p = _addone(p, key_f13,         KEY_F(13),      "f13");
	p = _addone(p, key_f14,         KEY_F(14),      "f14");
	p = _addone(p, key_f15,         KEY_F(15),      "f15");
	p = _addone(p, key_f16,         KEY_F(16),      "f16");
	p = _addone(p, key_f17,         KEY_F(17),      "f17");
	p = _addone(p, key_f18,         KEY_F(18),      "f18");
	p = _addone(p, key_f19,         KEY_F(19),      "f19");
	p = _addone(p, key_f20,         KEY_F(20),      "f20");
	p = _addone(p, key_f21,         KEY_F(21),      "f21");
	p = _addone(p, key_f22,         KEY_F(22),      "f22");
	p = _addone(p, key_f23,         KEY_F(23),      "f23");
	p = _addone(p, key_f24,         KEY_F(24),      "f24");
	p = _addone(p, key_f25,         KEY_F(25),      "f25");
	p = _addone(p, key_f26,         KEY_F(26),      "f26");
	p = _addone(p, key_f27,         KEY_F(27),      "f27");
	p = _addone(p, key_f28,         KEY_F(28),      "f28");
	p = _addone(p, key_f29,         KEY_F(29),      "f29");
	p = _addone(p, key_f30,         KEY_F(30),      "f30");
	p = _addone(p, key_f31,         KEY_F(31),      "f31");
	p = _addone(p, key_f32,         KEY_F(32),      "f32");
	p = _addone(p, key_f33,         KEY_F(33),      "f33");
	p = _addone(p, key_f34,         KEY_F(34),      "f34");
	p = _addone(p, key_f35,         KEY_F(35),      "f35");
	p = _addone(p, key_f36,         KEY_F(36),      "f36");
	p = _addone(p, key_dl,		KEY_DL,		"dl");
	p = _addone(p, key_il,          KEY_IL,         "il");
	p = _addone(p, key_dc,		KEY_DC,		"dc");
	p = _addone(p, key_ic,		KEY_IC,		"ic");
	p = _addone(p, key_eic,		KEY_EIC,	"eic");
	p = _addone(p, key_clear,	KEY_CLEAR,	"clear");
	p = _addone(p, key_eos,		KEY_EOS,	"eos");
	p = _addone(p, key_eol,		KEY_EOL,	"eol");
	p = _addone(p, key_sf,		KEY_SF,		"sf");
	p = _addone(p, key_sr,		KEY_SR,		"sr");
	p = _addone(p, key_npage,	KEY_NPAGE,	"npage");
	p = _addone(p, key_ppage,	KEY_PPAGE,	"ppage");
	p = _addone(p, key_stab,	KEY_STAB,	"stab");
	p = _addone(p, key_ctab,	KEY_CTAB,	"ctab");
	p = _addone(p, key_catab,	KEY_CATAB,	"catab");
	p = _addone(p, key_ll,		KEY_LL,		"ll");
	p = _addone(p, key_a1,		KEY_A1,		"a1");
	p = _addone(p, key_a3,		KEY_A3,		"a3");
	p = _addone(p, key_b2,		KEY_B2,		"b2");
	p = _addone(p, key_c1,		KEY_C1,		"c1");
	p = _addone(p, key_c3,		KEY_C3,		"c3");
	p = _addone(p, key_action,      KEY_ACTION,     "action");
	p -> keynum = 0;		/* termination convention */
#ifdef DEBUG
	if(outf) fprintf(outf,
		"return key structure %x, ending at %x\n", r, p);
#endif
	return r;
}

/*
 * NAME:        _addone
 *
 * FUNCTION:
 *
 *      Map text into num, updating the map structure p.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Label is currently unused, but is an English description
 *      of what the key is labelled, similar to the name in vi.
 */

static
struct map *
_addone(p, text, num, label)
struct map *p;
char *text;
int num;
char *label;
{
	if (text) {
		strcpy(p->label, label);
		strcpy(p->sends, text);
		p->keynum = num;
#ifdef DEBUG
		if(outf) fprintf(outf,
			"have key label %s, sends '%s', value %o\n",
			p->label, p->sends, p->keynum);
#endif
		p++;
	}
	return p;
}

#endif		/* KEYPAD */
