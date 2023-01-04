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

/*
**
**  ABSTRACT:
**
**      This file defines various data structures that are needed 
**	by the server dix layer to process the special i18n key
**	binding definition entries found in the keysym map file.
**  
*/
#ifndef KEYBIND_H
#define KEYBIND_H

#define DRVKB_SUPPORT_INIT_VALUE   2
#define DRVKEYBIND_FAILURE         0
#define DRVKEYBIND_SUCCESS         1
#define KEYBIND_PTR_ENTRY_INDEX    0xFF
#define KEYBIND_PTR_ENTRY_IS_EMPTY 0
#define MAX_NUM_KEYBIND_ALLOWED    3
#define MAX_NUM_KEYCODE_IN_KCOMB   2

#define DIFFERENT_FROM_CURRENT_KB  TRUE
#define SAME_AS_CURRENT_KB         FALSE

#define ROW_SIZE_MODKEYMAP	   8
#define ROW_POS_MOD1		   3
#define ENTRYINDEX_MAX_NUM	   3

#define FOUR_COLUMN_KEYMAP	   4

#define KBTYPE_LOCKDOWN		   1
#define KBTYPE_ONESHOT		   2
#define KBTYPE_NORMAL		   3

#define PASS_INIT		   1
#define KEYMAPPROPERTY_CREATED     2

#define UNMAP_ALL 0
#define MOD_MAPPING_CHANGED 1

#ifndef DECWDEF_H
#define DECWDEF_H
#include "decwdef.h"
#endif /* DECWDEF_H */

#include "X.h"

typedef struct _KeyBindLis {
    unsigned long itemCount;
    struct KeyBindDef KBDef[MAX_NUM_KEYBIND_ALLOWED];
}KeyBindLis;

/* This is what's in the index entry 6E of the keysym map, which serves
 * as an pointer entry to different types of key binding definition.
 */

typedef struct _KeyBindPtrEntry {
    KeyCode KBEntryIndex3;   /*index to 3rd key binding definition */
    KeyCode KBEntryIndex2;   /*index to 2nd key binding definition */
    KeyCode KBEntryIndex1;   /*index to 1st key binding definition */
    KeyCode NumKeyBind;      /* number of binding to define */
} KeyBindPtrEntry;


typedef struct _KeyBindEntry{
    char    ignore;
    char    NumKeyComb;  /* number of key combination for this type */
    KeyCode EquivKeyCode;/* equivalent keycode to return when the key 
                            combination is entered */
    char    KBType;      /* 1=lockdown, 2=one-shot, 3=normal */
    KeyCode KeyComb2[MAX_NUM_KEYCODE_IN_KCOMB]; /* key 2, key 1 of the 2nd key combination */
    KeyCode KeyComb1[MAX_NUM_KEYCODE_IN_KCOMB]; /* key 2, key 1 of the 1st key combination */
}KeyBindEntry;

typedef struct _KBModKey_roadmap{
    struct KeyBindDef *pKBDef;           /* modifier's key binding definition */
    unsigned short  ModKeyMap_row_index; /* row index to modifier key map */
    unsigned short  ModKeyMap_col_index; /* column index to modifier key map */
} KBModKey_roadmap; 

typedef struct _KeyCombDef{
    char    combCharLength;
    char    KC[MAX_NUM_KEYCODE_IN_KCOMB];
}KeyCombDef;

#endif /* KEYBIND_H */
