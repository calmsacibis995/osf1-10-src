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
/*
 * $XConsortium: mipsKbd.h,v 1.4 91/07/18 22:58:32 keith Exp $
 *
 * Copyright 1991 MIPS Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of MIPS not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  MIPS makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * MIPS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL MIPS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* $Header: /usr/sde/x11/rcs/x11/src/./server/ddx/mips/mipsKbd.h,v 1.2 91/12/15 12:42:16 devrcs Exp $ */
#ifndef __DDX_MIPSKBD_H
#define	__DDX_MIPSKBD_H

#define DEFAULT_KEYBOARD	-1
#define XT_KEYBOARD		0
#define AT_KEYBOARD		1
#define UNIX1_KEYBOARD		2

/* mod defines */

#define MC	(ControlMask)
#define MS	(ShiftMask)
#define ML	(LockMask)
#define M1	(Mod1Mask)

/* keyboard device name */

#define	KEYDEV		"/dev/keybd"

/* led commands */

#define LEDupdate	0	/* Update leds */
#define LEDreset	1	/* Reset leds */
#define LEDScrollLock	2	/* Scroll lock light on/off */
#define LEDCapsLock	3	/* Caps lock light on/off */
#define LEDNumLock	4	/* Num lock light on/off */
#define LED1		5	/* xled1 light on/off */
#define LED2		6	/* xled2 light on/off */
#define LED3		7	/* xled3 light on/off */

/* keyboard private structure */

typedef
struct  _keybdPriv {
    int		fd;
    short	unit;		/* which keyboard */
    short	cap;		/* Device capabilities */
    short	type;		/* keyboard type */
} KeybdPriv, *KeybdPrivPtr;

typedef struct {
    KeySymsRec	keySyms;
    CARD8	*modMap;
    int		offset;
    void	(*keybdEvent) ();
} KeybdType_t;

/* keyboard defines */

extern KeybdPriv	keybdPriv;
extern KeybdType_t	keybdType[];
extern void		xtKeybdEvent();
extern void		atKeybdEvent();
extern void		unix1KeybdEvent();
extern KeySym		xt_KeyMap[];
extern CARD8		xt_ModMap[];
extern KeySym		at_KeyMap[];
extern CARD8		at_ModMap[];
extern KeySym		unix1_KeyMap[];
extern CARD8		unix1_ModMap[];

#endif /* __DDX_MIPSKBD_H */
