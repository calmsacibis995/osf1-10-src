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
/***********************************************************
Copyright 1987 by Tektronix, Beaverton, Oregon,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Tektronix or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

TEKTRONIX DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
 *	NAME
 *		keymap_xtl.c -- Keycode to Keysym mappings.
 *
 *	DESCRIPTION
 *		gfbXTLKBMappings() maps XTL keyboard raw key codes
 *		to Keysyms.
 *
 *
 */
#ifndef LINT
#ifdef RCS_ID
static char *rcsid=  "$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/keymap_xtl.c,v 1.2 91/12/15 12:42:16 devrcs Exp $";
#endif RCS_ID
#endif LINT

#include "X.h"
#include "Xmd.h"		/* for CARD8  */
#include "input.h"
#include "keynames_xtl.h"
#include "keysym.h"
#ifdef NOTDEF
#include "TEKkeysym.h"
#else
#include "DECkeysym.h"
#endif


/*
 *	NAME
 *		gfbGDSKBMappings - GDS KeyCode to KeySym mapping
 *
 *	SYNOPSIS
 */
void
gfbXTLKBMappings(pKeySyms, ModMap)
    KeySymsPtr pKeySyms;	/* out: pointer to KeySym structure	*/
    CARD8 *ModMap;		/* out: pointer to Modifier Map		*/
/*
 *	DESCRIPTION
 *		Builds the KeyCode to KeySym map and Modifier map.
 *
 *	RETURNS
 *		The KeySym map via pKeySyms structure.
 *		The Modifier map via ModMap.
 */
{
    int i;
    KeySym *map;

    for (i = 0; i < MAP_LENGTH; i++) {
	ModMap[i] = NoSymbol;   /* make certain everything is reset */
    }

    /* if you edit this, also edit server/include/site.h DEFAULT_AUTOREPEATS */
    ModMap[PhysKeyToKeyCode(KEY_Lock) ] = LockMask;
    ModMap[PhysKeyToKeyCode(KEY_ShiftL) ] = ShiftMask;
    ModMap[PhysKeyToKeyCode(KEY_ShiftR) ] = ShiftMask;
    ModMap[PhysKeyToKeyCode(KEY_Ctrl) ] = ControlMask;
    /* this entry is deleted now now that compose is implemented */
    /* ModMap[PhysKeyToKeyCode(KEY_Compose) ] = Mod1Mask; */
    ModMap[PhysKeyToKeyCode(KEY_Menu) ] = Mod1Mask;

    map = (KeySym *)Xalloc( sizeof(KeySym) * (MAP_LENGTH * GDS_GLYPHS_PER_KEY));
    pKeySyms->minKeyCode = MIN_GDS_KEYCODE;
    pKeySyms->maxKeyCode = MAX_GDS_KEYCODE + 
			   (MAX_GDS_KEYCODE - MIN_GDS_KEYCODE + 1);
    /* add duplicate set for Katakana; number of actual keycodes assumed to be
       greater than NUM_COMPOSE_KEYCODES (96 min) */
    pKeySyms->mapWidth   = GDS_GLYPHS_PER_KEY;
    pKeySyms->map = map;

    for (i = 0; i < (MAP_LENGTH * GDS_GLYPHS_PER_KEY); i++) {
	    map[i] = NoSymbol;  /* make certain everything is reset */
    }

    map[INDEX(PhysKeyToKeyCode(KEY_Lock)  )] = XK_Caps_Lock;
    map[INDEX(PhysKeyToKeyCode(KEY_ShiftL))] = XK_Shift_L;
    map[INDEX(PhysKeyToKeyCode(KEY_ShiftR))] = XK_Shift_R;
    map[INDEX(PhysKeyToKeyCode(KEY_Ctrl)  )] = XK_Control_L;

#ifdef TekXK_Tek
    map[INDEX(PhysKeyToKeyCode(KEY_SErase))] = TekXK_SErase;
#endif

    map[INDEX(PhysKeyToKeyCode(KEY_Break)    )] = XK_Break;
    map[INDEX(PhysKeyToKeyCode(KEY_BackSpace))] = XK_BackSpace;
    map[INDEX(PhysKeyToKeyCode(KEY_Tab)      )] = XK_Tab;
    map[INDEX(PhysKeyToKeyCode(KEY_Linefeed) )] = XK_Linefeed;
    map[INDEX(PhysKeyToKeyCode(KEY_Return)   )] = XK_Return;
    map[INDEX(PhysKeyToKeyCode(KEY_Escape)   )] = XK_Escape;
    map[INDEX(PhysKeyToKeyCode(KEY_Space)    )] = XK_space;

    map[INDEX(PhysKeyToKeyCode(KEY_Quote)  )] = XK_quoteright;
    map[INDEX(PhysKeyToKeyCode(KEY_Quote))+1] = XK_quotedbl;
    map[INDEX(PhysKeyToKeyCode(KEY_Comma)  )] = XK_comma;
    map[INDEX(PhysKeyToKeyCode(KEY_Minus)  )] = XK_minus;
    map[INDEX(PhysKeyToKeyCode(KEY_Minus))+1] = XK_underscore;
    map[INDEX(PhysKeyToKeyCode(KEY_Period) )] = XK_period;
    map[INDEX(PhysKeyToKeyCode(KEY_Slash)  )] = XK_slash;
    map[INDEX(PhysKeyToKeyCode(KEY_Slash))+1] = XK_question;

    map[INDEX(PhysKeyToKeyCode(KEY_0)  )] = XK_0;
    map[INDEX(PhysKeyToKeyCode(KEY_0))+1] = XK_parenright;
    map[INDEX(PhysKeyToKeyCode(KEY_1)  )] = XK_1;
    map[INDEX(PhysKeyToKeyCode(KEY_1))+1] = XK_exclam;
    map[INDEX(PhysKeyToKeyCode(KEY_2)  )] = XK_2;
    map[INDEX(PhysKeyToKeyCode(KEY_2))+1] = XK_at;
    map[INDEX(PhysKeyToKeyCode(KEY_3)  )] = XK_3;
    map[INDEX(PhysKeyToKeyCode(KEY_3))+1] = XK_numbersign;
    map[INDEX(PhysKeyToKeyCode(KEY_4)  )] = XK_4;
    map[INDEX(PhysKeyToKeyCode(KEY_4))+1] = XK_dollar;
    map[INDEX(PhysKeyToKeyCode(KEY_5)  )] = XK_5;
    map[INDEX(PhysKeyToKeyCode(KEY_5))+1] = XK_percent;
    map[INDEX(PhysKeyToKeyCode(KEY_6)  )] = XK_6;
    map[INDEX(PhysKeyToKeyCode(KEY_6))+1] = XK_asciicircum;
    map[INDEX(PhysKeyToKeyCode(KEY_7)  )] = XK_7;
    map[INDEX(PhysKeyToKeyCode(KEY_7))+1] = XK_ampersand;
    map[INDEX(PhysKeyToKeyCode(KEY_8)  )] = XK_8;
    map[INDEX(PhysKeyToKeyCode(KEY_8))+1] = XK_asterisk;
    map[INDEX(PhysKeyToKeyCode(KEY_9)  )] = XK_9;
    map[INDEX(PhysKeyToKeyCode(KEY_9))+1] = XK_parenleft;

    map[INDEX(PhysKeyToKeyCode(KEY_SemiColon)  )] = XK_semicolon;
    map[INDEX(PhysKeyToKeyCode(KEY_SemiColon))+1] = XK_colon;

    map[INDEX(PhysKeyToKeyCode(KEY_Equal)  )] = XK_equal;
    map[INDEX(PhysKeyToKeyCode(KEY_Equal))+1] = XK_plus;

    map[INDEX(PhysKeyToKeyCode(KEY_a))] = XK_A;
    map[INDEX(PhysKeyToKeyCode(KEY_b))] = XK_B;
    map[INDEX(PhysKeyToKeyCode(KEY_c))] = XK_C;
    map[INDEX(PhysKeyToKeyCode(KEY_d))] = XK_D;
    map[INDEX(PhysKeyToKeyCode(KEY_e))] = XK_E;
    map[INDEX(PhysKeyToKeyCode(KEY_f))] = XK_F;
    map[INDEX(PhysKeyToKeyCode(KEY_g))] = XK_G;
    map[INDEX(PhysKeyToKeyCode(KEY_h))] = XK_H;
    map[INDEX(PhysKeyToKeyCode(KEY_i))] = XK_I;
    map[INDEX(PhysKeyToKeyCode(KEY_j))] = XK_J;
    map[INDEX(PhysKeyToKeyCode(KEY_k))] = XK_K;
    map[INDEX(PhysKeyToKeyCode(KEY_l))] = XK_L;
    map[INDEX(PhysKeyToKeyCode(KEY_m))] = XK_M;
    map[INDEX(PhysKeyToKeyCode(KEY_n))] = XK_N;
    map[INDEX(PhysKeyToKeyCode(KEY_o))] = XK_O;
    map[INDEX(PhysKeyToKeyCode(KEY_p))] = XK_P;
    map[INDEX(PhysKeyToKeyCode(KEY_q))] = XK_Q;
    map[INDEX(PhysKeyToKeyCode(KEY_r))] = XK_R;
    map[INDEX(PhysKeyToKeyCode(KEY_s))] = XK_S;
    map[INDEX(PhysKeyToKeyCode(KEY_t))] = XK_T;
    map[INDEX(PhysKeyToKeyCode(KEY_u))] = XK_U;
    map[INDEX(PhysKeyToKeyCode(KEY_v))] = XK_V;
    map[INDEX(PhysKeyToKeyCode(KEY_w))] = XK_W;
    map[INDEX(PhysKeyToKeyCode(KEY_x))] = XK_X;
    map[INDEX(PhysKeyToKeyCode(KEY_y))] = XK_Y;
    map[INDEX(PhysKeyToKeyCode(KEY_z))] = XK_Z;

    map[INDEX(PhysKeyToKeyCode(KEY_LBrace)   )] = XK_bracketleft;
    map[INDEX(PhysKeyToKeyCode(KEY_LBrace)) +1] = XK_braceleft;
    map[INDEX(PhysKeyToKeyCode(KEY_VertBar)  )] = XK_backslash;
    map[INDEX(PhysKeyToKeyCode(KEY_VertBar))+1] = XK_bar;
    map[INDEX(PhysKeyToKeyCode(KEY_RBrace)   )] = XK_bracketright;
    map[INDEX(PhysKeyToKeyCode(KEY_RBrace)) +1] = XK_braceright;
    map[INDEX(PhysKeyToKeyCode(KEY_Tilde)    )] = XK_quoteleft;
    map[INDEX(PhysKeyToKeyCode(KEY_Tilde))  +1] = XK_asciitilde;

    map[INDEX(PhysKeyToKeyCode(KEY_RubOut))] = XK_Delete;

    map[INDEX(PhysKeyToKeyCode(KEY_Enter)    )] = XK_KP_Enter;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_Comma) )] = XK_KP_Separator;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_Minus) )] = XK_KP_Subtract;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_Period))] = XK_KP_Decimal;

    map[INDEX(PhysKeyToKeyCode(KEY_KP_0))] = XK_KP_0;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_1))] = XK_KP_1;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_2))] = XK_KP_2;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_3))] = XK_KP_3;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_4))] = XK_KP_4;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_5))] = XK_KP_5;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_6))] = XK_KP_6;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_7))] = XK_KP_7;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_8))] = XK_KP_8;
    map[INDEX(PhysKeyToKeyCode(KEY_KP_9))] = XK_KP_9;

    map[INDEX(PhysKeyToKeyCode(KEY_F1))] = XK_F1;
    map[INDEX(PhysKeyToKeyCode(KEY_F2))] = XK_F2;
    map[INDEX(PhysKeyToKeyCode(KEY_F3))] = XK_F3;
    map[INDEX(PhysKeyToKeyCode(KEY_F4))] = XK_F4;
    map[INDEX(PhysKeyToKeyCode(KEY_F5))] = XK_F5;
    map[INDEX(PhysKeyToKeyCode(KEY_F6))] = XK_F6;
    map[INDEX(PhysKeyToKeyCode(KEY_F7))] = XK_F7;
    map[INDEX(PhysKeyToKeyCode(KEY_F8))] = XK_F8;

#ifdef TekXK_Tek
    map[INDEX(PhysKeyToKeyCode(KEY_Dialog))] = TekXK_Dialog;
    map[INDEX(PhysKeyToKeyCode(KEY_Setup) )] = TekXK_Setup;
    map[INDEX(PhysKeyToKeyCode(KEY_Copy)  )] = TekXK_Copy;
#endif
    map[INDEX(PhysKeyToKeyCode(KEY_Menu)  )] = XK_Meta_L;
    /* must use Meta, because xterm converts Menu to an escape sequence */

    map[INDEX(PhysKeyToKeyCode(KEY_Cursor_R))] = XK_Right;
    map[INDEX(PhysKeyToKeyCode(KEY_Cursor_U))] = XK_Up;
    map[INDEX(PhysKeyToKeyCode(KEY_Cursor_L))] = XK_Left;
    map[INDEX(PhysKeyToKeyCode(KEY_Cursor_D))] = XK_Down;

    map[INDEX(PhysKeyToKeyCode(KEY_Help)   )] = XK_Help;
    map[INDEX(PhysKeyToKeyCode(KEY_Do)     )] = XK_Execute;
    map[INDEX(PhysKeyToKeyCode(KEY_Compose))] = XK_Multi_key;
#ifdef TekXK_Tek
    map[INDEX(PhysKeyToKeyCode(KEY_Tek)    )] = TekXK_Tek;
#endif

    map[INDEX(PhysKeyToKeyCode(KEY_Find)    )] = XK_Find;
    map[INDEX(PhysKeyToKeyCode(KEY_Insert)  )] = XK_Insert;
    map[INDEX(PhysKeyToKeyCode(KEY_Remove)  )] = DXK_Remove; /* DEC's value */
    map[INDEX(PhysKeyToKeyCode(KEY_Select)  )] = XK_Select;
    map[INDEX(PhysKeyToKeyCode(KEY_Previous))] = XK_Prior;
    map[INDEX(PhysKeyToKeyCode(KEY_Next)    )] = XK_Next;

    map[INDEX(PhysKeyToKeyCode(KEY_PF1))] = XK_KP_F1;
    map[INDEX(PhysKeyToKeyCode(KEY_PF2))] = XK_KP_F2;
    map[INDEX(PhysKeyToKeyCode(KEY_PF3))] = XK_KP_F3;
    map[INDEX(PhysKeyToKeyCode(KEY_PF4))] = XK_KP_F4;

#ifdef TekXK_Tek
    map[INDEX(PhysKeyToKeyCode(KEY_Hold)  )] = TekXK_Hold;
    map[INDEX(PhysKeyToKeyCode(KEY_GErase))] = TekXK_GErase;
    map[INDEX(PhysKeyToKeyCode(KEY_DErase))] = TekXK_DErase;
#endif
    map[INDEX(PhysKeyToKeyCode(KEY_Cancel))] = XK_Cancel;

    map[INDEX(PhysKeyToKeyCode(KEY_Greater)  )] = XK_less;
    map[INDEX(PhysKeyToKeyCode(KEY_Greater))+1] = XK_greater;

/* 
 * GDS:
 * The cursor pad key values for NorthWest (NW), NE, SW, and SE are
 * not directly mapped;  instead, the gds ProcessInputEvent() routine will
 * catch the GDS report and expand it into two events.
 *
 * 4310:
 * The kernel converts cursor pad key values for NW, NE, SW, and SE to
 * two events.
 */

}
