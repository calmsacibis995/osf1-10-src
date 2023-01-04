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
 * $XConsortium: omronKbd.h,v 1.1 91/06/29 13:49:02 xguest Exp $
 *
 * Copyright 1991 by OMRON Corporation
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OMRON not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  OMRON makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * OMRON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL OMRON
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef	uniosu
# include <sys/deftbl.t>
#else /* not uniosu */
# ifdef luna88k
#  include <dev/deftbl.t>
# else
#  ifdef luna2
#   include <dev/deftbl.t>
#  else /* uniosb */
#   include <om68kdev/deftbl.t>
#  endif
# endif
#endif

#define  XK_KATAKANA
#include "keysym.h"

#define KS_KANA		0x1
#define KS_CTRL_L	0x2
#define KS_CTRL_R	0x4
#define KS_META_L	0x8
#define KS_META_R	0x10
#define KS_ALT_L	0x20
#define KS_ALT_R	0x40
#define KS_SUPER_L	0x80
#define KS_SUPER_R	0x100
#define KS_HYPER_L	0x200
#define KS_HYPER_R	0x400

#define KANA_KEY	11
#define CAPSLOCK_KEY	14

#define MIDDLE_BUTTON	1
#define RIGHT_BUTTON	2

#define AREPBUFSZ		32

#ifdef luna2
#define osfXK_Insert    0x1004FF63
#define osfXK_Copy      0x1004FF02
#define osfXK_Cut       0x1004FF03
#define osfXK_Paste     0x1004FF04

#define KB_JISJIS	0
#define KB_ASCII	1
#define KB_ASCIIJIS	2
#define KB_DEFAULT	3
#endif /* luna2*/

#ifndef UNUSE_DRV_TIME 
typedef struct _key_event {
	long	time;			
	unsigned char code;
} key_event;       
#endif

typedef	struct	_omronKeyPrvRec {
	/* device control */
	int fd;
	int type;
	int flags;
#ifdef uniosu
	struct termio old_term;
#else
	struct sgttyb old_term;
#endif
	/* ascii control */
	int offset;
	KeybdCtrl keybdCtrl;
	char semiEncodeDef[CODTBSZ];
	char semiEncode[CODTBSZ];
	KeyCode minkey,maxkey; 
	/* kana control */
#ifndef USE_KANA_SWITCH
	int key_state;		/* kana key status */
	char semiKanaEncode[CODTBSZ];
	KeyCode kana_minkey,kana_maxkey; 
	unsigned char kana_offset; 
#endif
}	omronKeyPrv,	*omronKeyPrvPtr;

extern int  omronKbdProc();
extern void omronKbdGiveUp();

extern void omronKbdEnqueueEvent();
extern unsigned char  *omronKbdGetEvents(); 

#ifndef UNUSE_DRV_TIME
extern void omronKbdEnqueueTEvent();
extern key_event *omronKbdGetTEvents(); 
#endif
