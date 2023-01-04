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
** xgc
**
** xgc.h
*/

#include "constants.h"

typedef struct {
  char *string;
  int    code;
} StringTable;

typedef struct {
  Display  *dpy;		/* the display! */
  Screen   *scr;		/* the screen! */
  Window    win;		/* the window the test runs in */
  GC        gc;			/* the GC! */
  GC        miscgc;		/* used for doing stuff when we don't want
				   to change the normal GC */
  XGCValues gcv;		/* a separate copy of what's in the GC,
				   since we're not allowed to look in it */
  Pixmap    tile;		/* what we tile with */
  Pixmap    stipple;		/* what we stipple with */
  XImage   *image;		/* image for GetImage & PutImage */
  int       test;		/* which test is being run */
  float     percent;		/* percentage of test to run */
  Pixel     foreground;
  Pixel     background;
  char      *fontname;
} XStuff;                       /* All the stuff that only X needs to
                                   know about */

typedef struct {
  char name[40];  		/* name as it will appear on the screen */
  char text[40];	       	/* Xgc command it translates to */
  int num_commands;		/* number of command buttons inside it */
  int columns;			/* how many columns of command buttons; if
				   0, then there's only one row */
  struct {
    char name[40];		/* name as it will appear on the screen */
    char text[40];   		/* Xgc command it translates to */
  } command_data[MAXCHOICES];
} ChoiceStuff;			/* All the info needed to deal with a 
				   choice widget */
typedef struct {
  char *name;
  char *text;
  int code;
} XgcData;

typedef struct {
  struct {
    char *name;
    char *text;
    int   num_toggles;
    int   columns;
  } choice;
  XgcData *data;
} XgcStuff;

typedef struct {
  Widget label;
  int size;
  WidgetList widgets;
} ChoiceDesc;

/************/
