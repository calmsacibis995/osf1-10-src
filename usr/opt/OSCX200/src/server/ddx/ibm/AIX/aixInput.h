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
 * $XConsortium: aixInput.h,v 1.2 91/07/16 12:59:56 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef AIXINPUT_H
#define AIXINPUT_H

/* The following are defined in AIX.h */

#define AIX_MIN_KEY  0x08
#define AIX_MAX_KEY (0x85 + AIX_MIN_KEY)
#define AIX_GLYPHS_PER_KEY 2

/*                                        Position
 *                          Base            Code
 */

#define Aix_Control_L    (AIX_MIN_KEY   +    0x3A)
#define Aix_Shift_L      (AIX_MIN_KEY   +    0x2C)
#define Aix_Shift_R      (AIX_MIN_KEY   +    0x39)
#define Aix_Caps_Lock    (AIX_MIN_KEY   +    0x1E)
#define Aix_Alt_L        (AIX_MIN_KEY   +    0x3C)
#define Aix_Alt_R        (AIX_MIN_KEY   +    0x3E)
#define Aix_Backspace    (AIX_MIN_KEY   +    0x0f)

#ifdef AIXps2
#define AIX_Control_R	 (AIX_MIN_KEY	+   0x40)
#endif

#define Aix_Num_Lock	 (AIX_MIN_KEY   +    0x5a)

#define NumLockMask	 Mod5Mask

#endif /* AIXINPUT_H */
