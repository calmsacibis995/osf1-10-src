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
/************************************************************************
Copyright 1990 by Digital Equipment Corporation, Maynard, Massachusetts.
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/


/*                                                                          */
/* SETMODE DECW_KEYBIND_FUNC P1 values - keybind commands                   */
/*                                                                          */
#define IO$K_DECW_KEYBIND_UNMAP 0
#define IO$K_DECW_KEYBIND_DEF 1
#define IO$K_DECW_KEYBIND_SEND_FAKE_KEY 2
/*                                                                          */
/* I18N Key Binding Definition Descriptior                                  */
/*                                                                          */
/*                                                                          */
/*PREFIX:kbdef$                                                             */
#define kbdef$m_lockdown_mod 1
#define kbdef$m_oneshot_mod 2
#define kbdef$m_normal_mod 4
#define KeyBindDef_LENGTH 9
struct KeyBindDef {
    union  {
        unsigned char kbdef$B_MASK;              /* type mask             */
        struct  {
            unsigned kbdef$v_lockdown_mod : 1; /* lock-down modifier keybind */
            unsigned kbdef$v_oneshot_mod : 1; /* one-shot modifier keybind  */
            unsigned kbdef$v_normal_mod : 1; /* normal key code binding     */
            unsigned kbdef$v_fill_17 : 5;
            } kbdef$r_mask_bits;
        } kbdef$r_kb_type;
    KeyCode kbdef$b_modkey;
    unsigned char kbdef$b_num_comb;
    struct  {
        unsigned char kbdef$b_combCharLength;
        KeyCode  kbdef$B_KC [2];
        } kbdef$r_keyComb [2];
    } ;
