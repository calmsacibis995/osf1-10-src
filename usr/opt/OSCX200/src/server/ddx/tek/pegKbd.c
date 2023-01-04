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
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/pegKbd.c,v 1.2 91/12/15 12:42:16 devrcs Exp $ */

#include "X.h"
#include "Xproto.h"
#include "keysym.h"
#include "input.h"
#include "miscstruct.h"
#include "screenint.h"
#include "keynames_xtl.h"

#ifdef	UTEK
#include <box/keyboard.h>
#endif	/* UTEK */

#ifdef	UTEKV
#include "redwing/keyboard.h"
#endif	/* UTEKV */

#include "peg.h"

#define PEGASUS_GLYPHS_PER_KEY	2

/*
 * There are no hardware restrictions on what modifiers can be bound to
 * keys, because it is all accomplished in software; i.e. the keyboard
 * itself is not trying to "shift" or "capslock" any of the keys.
 */
Bool
LegalModifier(key)
    BYTE key;
{
    return TRUE;
}

void
GetPEGKBMappings(pKeySyms, pModMap)
    KeySymsPtr pKeySyms;
    CARD8 *pModMap;
{
	int i;
	KeySym *map;

    for (i = 0; i < MAP_LENGTH; i++)
	pModMap[i] = NoSymbol;	/* make sure it is restored */

#define INDEX(in) ((in) + MINKEYCODE)
    pModMap[ INDEX( KBCapsLock ) ] = LockMask;
    pModMap[ INDEX( KBShiftL ) ] = ShiftMask;
    pModMap[ INDEX( KBShiftR ) ] = ShiftMask;
    pModMap[ INDEX( KBCtrl ) ] = ControlMask;
    pModMap[ INDEX( KBLeftArrow ) ] = Mod1Mask;
#undef INDEX

    map = (KeySym *)Xalloc(sizeof(KeySym) * 
	(MAP_LENGTH * PEGASUS_GLYPHS_PER_KEY));
    pKeySyms->mapWidth = PEGASUS_GLYPHS_PER_KEY;
    pKeySyms->map = map;
    /* this will shift everything up automatically */
    pKeySyms->minKeyCode = MINKEYCODE;
    pKeySyms->maxKeyCode = KBJoyRtDown + MINKEYCODE + NUM_COMPOSE_KEYCODES;
    /* Katakana kbd not supported here */

    for (i = 0; i < (MAP_LENGTH * PEGASUS_GLYPHS_PER_KEY); i++)
	    map[i] = NoSymbol;	/* make sure it is restored */

#define INDEX(in) ((in) * PEGASUS_GLYPHS_PER_KEY)
    map[INDEX( KBCapsLock )] = XK_Caps_Lock;
    map[INDEX( KBShiftL )] = XK_Shift_L;
    map[INDEX( KBShiftR )] = XK_Shift_R;
    map[INDEX( KBCtrl )] = XK_Control_L;
    map[INDEX( KBLeftArrow )] = XK_Meta_L;
    map[INDEX( KBBreak )] = XK_Break;
    map[INDEX( KBBackSpace )] = XK_BackSpace;
    map[INDEX( KBTab )] = XK_Tab;
    map[INDEX( KBLineFeed )] = XK_Linefeed;
    map[INDEX( KBReturn )] = XK_Return;
    map[INDEX( KBEsc )] = XK_Escape;
    map[INDEX( KBSpaceBar )] = XK_space;

    map[INDEX( KBQuote )] = XK_quoteright;
    map[INDEX( KBQuote ) + 1] = XK_quotedbl;

    map[INDEX( KBComma )] = XK_comma;
    map[INDEX( KBComma ) + 1] = XK_less;

    map[INDEX( KBHyphon )] = XK_minus;
    map[INDEX( KBHyphon ) + 1] = XK_underscore;

    map[INDEX( KBPeriod )] = XK_period;
    map[INDEX( KBPeriod ) + 1] = XK_greater;

    map[INDEX( KBSlash )] = XK_slash;
    map[INDEX( KBSlash ) + 1] = XK_question;

    map[INDEX( KB0 )] = XK_0;
    map[INDEX( KB1 )] = XK_1;
    map[INDEX( KB2 )] = XK_2;
    map[INDEX( KB3 )] = XK_3;
    map[INDEX( KB4 )] = XK_4;
    map[INDEX( KB5 )] = XK_5;
    map[INDEX( KB6 )] = XK_6;
    map[INDEX( KB7 )] = XK_7;
    map[INDEX( KB8 )] = XK_8;
    map[INDEX( KB9 )] = XK_9;

    map[INDEX( KB0 ) + 1] = XK_parenright;
    map[INDEX( KB1 ) + 1] = XK_exclam;
    map[INDEX( KB2 ) + 1] = XK_at;
    map[INDEX( KB3 ) + 1] = XK_numbersign;
    map[INDEX( KB4 ) + 1] = XK_dollar;
    map[INDEX( KB5 ) + 1] = XK_percent;
    map[INDEX( KB6 ) + 1] = XK_asciicircum;
    map[INDEX( KB7 ) + 1] = XK_ampersand;
    map[INDEX( KB8 ) + 1] = XK_asterisk;
    map[INDEX( KB9 ) + 1] = XK_parenleft;

    map[INDEX( KBSemiColon )] = XK_semicolon;
    map[INDEX( KBSemiColon ) + 1] = XK_colon;

    map[INDEX( KBEqual )] = XK_equal;
    map[INDEX( KBEqual ) + 1] = XK_plus;

    map[INDEX( KBA )] = XK_A;
    map[INDEX( KBB )] = XK_B;
    map[INDEX( KBC )] = XK_C;
    map[INDEX( KBD )] = XK_D;
    map[INDEX( KBE )] = XK_E;
    map[INDEX( KBF )] = XK_F;
    map[INDEX( KBG )] = XK_G;
    map[INDEX( KBH )] = XK_H;
    map[INDEX( KBI )] = XK_I;
    map[INDEX( KBJ )] = XK_J;
    map[INDEX( KBK )] = XK_K;
    map[INDEX( KBL )] = XK_L;
    map[INDEX( KBM )] = XK_M;
    map[INDEX( KBN )] = XK_N;
    map[INDEX( KBO )] = XK_O;
    map[INDEX( KBP )] = XK_P;
    map[INDEX( KBQ )] = XK_Q;
    map[INDEX( KBR )] = XK_R;
    map[INDEX( KBS )] = XK_S;
    map[INDEX( KBT )] = XK_T;
    map[INDEX( KBU )] = XK_U;
    map[INDEX( KBV )] = XK_V;
    map[INDEX( KBW )] = XK_W;
    map[INDEX( KBX )] = XK_X;
    map[INDEX( KBY )] = XK_Y;
    map[INDEX( KBZ )] = XK_Z;

    map[INDEX( KBBraceL )] = XK_bracketleft;	/* Yes! backwards! */
    map[INDEX( KBBraceL ) + 1] = XK_braceleft;

    map[INDEX( KBBackSlash )] = XK_backslash;
    map[INDEX( KBBackSlash ) + 1] = XK_quoteleft;

    map[INDEX( KBBraceR )] = XK_bracketright;
    map[INDEX( KBBraceR ) + 1] = XK_braceright;

    map[INDEX( KBBar )] = XK_bar;
    map[INDEX( KBBar ) + 1] = XK_asciitilde;

    map[INDEX( KBRubOut )] = XK_Delete;
    map[INDEX( KBEnter )] = XK_KP_Enter;
    map[INDEX( KPComma )] = XK_KP_Separator;
    map[INDEX( KPHyphon )] = XK_KP_Subtract;
    map[INDEX( KPPeriod )] = XK_KP_Decimal;

    map[INDEX( KP0 )] = XK_KP_0;
    map[INDEX( KP1 )] = XK_KP_1;
    map[INDEX( KP2 )] = XK_KP_2;
    map[INDEX( KP3 )] = XK_KP_3;
    map[INDEX( KP4 )] = XK_KP_4;
    map[INDEX( KP5 )] = XK_KP_5;
    map[INDEX( KP6 )] = XK_KP_6;
    map[INDEX( KP7 )] = XK_KP_7;
    map[INDEX( KP8 )] = XK_KP_8;
    map[INDEX( KP9 )] = XK_KP_9;

    map[INDEX( KBF5 )] = XK_F5;
    map[INDEX( KBF6 )] = XK_F6;
    map[INDEX( KBF7 )] = XK_F7;
    map[INDEX( KBF8 )] = XK_F8;
    map[INDEX( KBF9 )] = XK_KP_F1;
    map[INDEX( KBF10 )] = XK_KP_F2;
    map[INDEX( KBF11 )] = XK_KP_F3;
    map[INDEX( KBF12 )] = XK_KP_F4;
    map[INDEX( KBF1 )] = XK_F1;
    map[INDEX( KBF2 )] = XK_F2;
    map[INDEX( KBF3 )] = XK_F3;
    map[INDEX( KBF4 )] = XK_F4;

    map[INDEX( KBJoyRight )] = XK_Right;
    map[INDEX( KBJoyUp )] = XK_Up;
    map[INDEX( KBJoyLeft )] = XK_Left;
    map[INDEX( KBJoyDown )] = XK_Down;
/*
 * Not assigned 
 *	map[INDEX( KBJoyRtUp )] = XK_?;
 *	map[INDEX( KBJoyLfUp )] = XK_?;
 *	map[INDEX( KBJoyLfDown )] = XK_?;
 *	map[INDEX( KBJoyRtDown )] = XK_?;
 */
#undef INDEX
}
