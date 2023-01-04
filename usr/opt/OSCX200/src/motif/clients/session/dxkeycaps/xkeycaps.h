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
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#ifndef _XKEYCAPS_H_
#define _XKEYCAPS_H_

#ifdef ONE_DOT_TWO
#define def_char_set XmFONTLIST_DEFAULT_TAG
#else
#define def_char_set XmSTRING_DEFAULT_CHARSET
#endif


#include "KeyWidget.h"

struct key {
  KeyCode keycode;		/* The raw scancode this key generates */

  char *top_keysym;		/* The strings actually printed on the key */
  char *bottom_keysym;		/*  If one of these is 0, there's only one */
#ifdef DO_FOUR
  char *topRight_keysym;
  char *bottomRight_keysym;
#endif

  unsigned short width;		/* Width of the key in some arbitrary units */
  unsigned short height;	/* Height of the key in some arbitrary units */

  /* The default state of this key: which bucky-bits it sets, and which
     keysyms it generates by default.  We use this to implement a command
     to reset the keyboard to the default state, since the X server itself
     doesn't retain that information after the keymap has been altered.
   */
  unsigned long default_mods;
  KeySym default_keysyms[8];
  
  KeyWidget widget;		/* Backpointer; filled in at run-time */

};

struct row {
  unsigned int nkeys;
  unsigned int height;
  struct key *keys;
};

struct keyboard {
  char *name;
  unsigned int nrows;
  struct row *rows;
  int default_scale;
  int horiz_border, vert_border;
};


extern char *progname;

extern struct keyboard **all_kbds;
extern struct keyboard *base_kbds [];
extern struct keyboard *pc102s [];
/*
**  defines for keyboard hardware types
*/
#define KB_UNKNOWN  -1
#define KB_PCXAL    0
#define KB_LK201    1
#define KB_LK401    2
#define KB_LK421    3
#define KB_LK443    4
#define KB_LK444    5
#define KB_NCD      6
/*
**  these defines reflect the order of the array of tables
**  that's defined in all-kbds.h
*/
#define DENMARK         0
#define GERMANY         1
#define SWISS_GERMAN    2
#define UK              3
#define SPAIN           4
#define FRANCE          5
#define CANADIEN        6
#define SWISS_FRENCH    7
#define ITALY           8
#define DUTCH           9
#define NORWAY          10
#define PORTUGAL        11
#define FINNISH         12
#define SWEDEN          13
#define BELGIUM         14
/*
**  these defines reflect the table from /usr/lib/X11/xdm/Xkeymaps, which
**  in turn, reflects the console's table of keyboard language. That
**  tables looks something like:
**  ENV #   CODE    TITLE                   KEYBOARD/LANGUAGE   MODEL
**   0      30      Dansk                   danish              -AD
**   1      32      Deutsch                 austrian_german     -AG ?
**   2      34      Deutsch(Schweiz)        swiss_german        -CH
**   3      36      English(American)       us                  -AA
**   4      38      English(British/Irish)  uk                  -AE
**   5      3a      Espanol                 spanish             -AS ?
**   6      3c      Francais                belgian_french      -AP
**   7      3e      Francais(Canadien)      canadian_french -AC/-CC
**   8      40      Francais(SuisseRomande) swiss_french -AP/-CH ?
**   9      42      Italiano                italian             -AI
**  10      44      Nederlands              dutch_us            -GH ?
**  11      46      Norsk                   norwegian           -AN
**  12      48      Portugues               portuguese          -AV
**  13      4a      Suomi                   finnish             -CA ?
**  14      4c      Svenska                 swedish             -CA ?
**  15      4e      Vlaams                  flemish             -AB ?
*/
#define DANSK           0x30
#define DEUTSCH         0x32
#define DEUTSCH_SCHWEIZ 0x34
#define AMERICAN        0x36
#define BRITISH_IRISH   0x38
#define ESPANOL         0x3a
#define FRANCAIS        0x3c
#define FRANCAIS_CANADIEN   0x3e
#define FRANCAIS_SUISSEROMANDE 0x40
#define ITALIANO        0x42
#define NEDERLANDS      0x44
#define NORSK           0x46
#define PORTUGUES       0x48
#define SUOMI           0x4a
#define SVENSKA         0x4c
#define VLAAMS          0x4e

#endif /* _XKEYCAPS_H_ */
