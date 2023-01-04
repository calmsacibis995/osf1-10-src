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
  Copyright (c) Digital Equipment Corporation, 1987, 1988, 1989, 1990
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.
*/
 

/* PROGRAM DESCRIPTION: 
 * 
 *	This is an X11 program to play the puzzle game.
 *	Puzzle is simple number scremble game where a series of numbers are
 *	randomly place in a square with one blank.  The objective is to reorder
 *	the numbers.
 * 
 * AUTHORS: 
 * 
 *	NFF
 *
 * CREATION DATE:	9-Jun-1987
 *
 * Modification Date:
 *
 *	Nov. 	1990	(ASP)	Converting to Motif.
 */


#include "puzzledefs.h"
#ifdef UNIX

#include <X11/Vendor.h>
#include <unistd.h>
#include <pwd.h>

#else

#include "vendor.h"

#endif

/* routines used for supporting multiple size icon. */
static void             set_icons_on_shell();
static Pixmap           fetch_bitmap_icon();
static char *           get_icon_index_name();
static void             set_iconify_pixmap();
void                    set_icons();


void 		NewGame ();
void 		setupboard ();

int 		outoforder ();
int 		GetRand ();

extern void 	exit();
extern void 	PlaceTiles ();
extern void 	FetchHeierchy ();
extern void 	CreateMain ();
extern void 	CreateGC ();
extern void 	CreateTiles ();


#define PUZZLE_75_width 75
#define PUZZLE_75_height 75
#define PUZZLE_75_x_hot 0
#define PUZZLE_75_y_hot 0
static char PUZZLE_75_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff,
   0xff, 0xab, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0xfe, 0xff, 0xff, 0x51,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xaa, 0xaa,
   0xa6, 0xaa, 0xaa, 0x02, 0x56, 0x55, 0x55, 0x51, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x56, 0x55, 0x55, 0x51, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xae, 0xaa,
   0xaa, 0xa8, 0xfa, 0xaa, 0xa6, 0xfe, 0xaf, 0x02, 0x56, 0x55, 0x55, 0x51,
   0xfd, 0x55, 0x55, 0x7d, 0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xfe, 0xaa,
   0xa6, 0xbe, 0xaa, 0x02, 0x56, 0x55, 0x55, 0x51, 0xf5, 0x55, 0x55, 0x7d,
   0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xfa, 0xaa, 0xa6, 0xfe, 0xab, 0x02,
   0x56, 0x55, 0x55, 0x51, 0xf5, 0x55, 0x55, 0x55, 0x5f, 0x05, 0xae, 0xaa,
   0xaa, 0xa8, 0xfa, 0xaa, 0xa6, 0xaa, 0xaf, 0x02, 0x56, 0x55, 0x55, 0x51,
   0xf5, 0x55, 0x55, 0x55, 0x5f, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xfa, 0xaa,
   0xa6, 0xaa, 0xaf, 0x02, 0x56, 0x55, 0x55, 0x51, 0xf5, 0x55, 0x55, 0x5d,
   0x5f, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xfa, 0xaa, 0xa6, 0xbe, 0xaf, 0x02,
   0x56, 0x55, 0x55, 0x51, 0xf5, 0x55, 0x55, 0xf5, 0x57, 0x05, 0xae, 0xaa,
   0xaa, 0xa8, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0x56, 0x55, 0x55, 0x51,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xaa, 0xaa,
   0xa6, 0xaa, 0xaa, 0x02, 0x56, 0x55, 0x55, 0x51, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xae, 0xaa, 0xaa, 0xa8, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x56, 0x55, 0x55, 0x51, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0x02, 0x00,
   0x00, 0xf8, 0xff, 0xff, 0xf7, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xa6, 0xaa, 0xaa, 0x02, 0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0x54, 0x55, 0x55, 0x53,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xfa, 0xaa, 0xaa, 0xfe, 0xab,
   0xa6, 0xfa, 0xab, 0x02, 0x54, 0x7d, 0x55, 0x53, 0xf5, 0x55, 0x55, 0x7d,
   0x5f, 0x05, 0xaa, 0xbe, 0xaa, 0xaa, 0xfa, 0xaa, 0xa6, 0xbe, 0xaf, 0x02,
   0x54, 0x7f, 0x55, 0x53, 0x7d, 0x55, 0x55, 0x7d, 0x5f, 0x05, 0xaa, 0xff,
   0xab, 0xaa, 0xfe, 0xaa, 0xa6, 0xbe, 0xaf, 0x02, 0x54, 0xdf, 0x57, 0x53,
   0xd5, 0x57, 0x55, 0xf5, 0x57, 0x05, 0xaa, 0xef, 0xab, 0xaa, 0xea, 0xab,
   0xa6, 0xfa, 0xab, 0x02, 0x54, 0xdf, 0x57, 0x53, 0xd5, 0x57, 0x55, 0x7d,
   0x5f, 0x05, 0xaa, 0xef, 0xab, 0xaa, 0xea, 0xab, 0xa6, 0xbe, 0xaf, 0x02,
   0x54, 0xdf, 0x57, 0x53, 0xd7, 0x57, 0x55, 0x7d, 0x5f, 0x05, 0xaa, 0xff,
   0xab, 0xaa, 0xef, 0xab, 0xa6, 0xbe, 0xaf, 0x02, 0x54, 0xfd, 0x55, 0x53,
   0xfd, 0x55, 0x55, 0xf5, 0x57, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xa6, 0xaa, 0xaa, 0x02, 0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0xfe, 0xff, 0xff, 0xfb,
   0xff, 0xff, 0xf7, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0x54, 0x55, 0x55, 0x53,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xa6, 0xaa, 0xaa, 0x02, 0x54, 0xfd, 0x55, 0x53, 0xff, 0x57, 0x55, 0xd5,
   0x57, 0x05, 0xaa, 0xee, 0xab, 0xaa, 0xaa, 0xaf, 0xa6, 0xea, 0xaf, 0x02,
   0x54, 0xd7, 0x57, 0x53, 0xd5, 0x57, 0x55, 0xf5, 0x57, 0x05, 0xaa, 0xea,
   0xab, 0xaa, 0xaa, 0xaf, 0xa6, 0xba, 0xaf, 0x02, 0x54, 0xd5, 0x57, 0x53,
   0xd5, 0x57, 0x55, 0xdd, 0x57, 0x05, 0xaa, 0xea, 0xab, 0xaa, 0xea, 0xab,
   0xa6, 0xae, 0xaf, 0x02, 0x54, 0xf5, 0x55, 0x53, 0xf5, 0x55, 0x55, 0xdf,
   0x57, 0x05, 0xaa, 0xfa, 0xaa, 0xaa, 0xfa, 0xaa, 0xa6, 0xfe, 0xaf, 0x02,
   0x54, 0x7d, 0x55, 0x53, 0xf5, 0x55, 0x55, 0xd5, 0x57, 0x05, 0xaa, 0xbe,
   0xaa, 0xaa, 0xfa, 0xaa, 0xa6, 0xaa, 0xaf, 0x02, 0x54, 0x5f, 0x55, 0x53,
   0xf5, 0x55, 0x55, 0xd5, 0x57, 0x05, 0xaa, 0xff, 0xab, 0xaa, 0xfa, 0xaa,
   0xa6, 0xaa, 0xaf, 0x02, 0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55,
   0x55, 0x05, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02,
   0x54, 0x55, 0x55, 0x53, 0x55, 0x55, 0x55, 0x55, 0x55, 0x05, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xa6, 0xaa, 0xaa, 0x02, 0x54, 0x55, 0x55, 0x53,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x05};

#define PUZZLE_50_width 50
#define PUZZLE_50_height 50
#define PUZZLE_50_x_hot 0
#define PUZZLE_50_y_hot 0
static char PUZZLE_50_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x55, 0x55, 0xab,
   0xaa, 0x02, 0xfe, 0xff, 0xa8, 0xaa, 0x52, 0x55, 0x01, 0x56, 0x55, 0x55,
   0x55, 0xab, 0xaa, 0x02, 0xae, 0xaa, 0xa8, 0xaf, 0xd2, 0x7f, 0x01, 0x56,
   0x55, 0xd5, 0x57, 0xab, 0xab, 0x02, 0xae, 0xaa, 0xa8, 0xaf, 0xd2, 0x57,
   0x01, 0x56, 0x55, 0x55, 0x57, 0xab, 0xbf, 0x02, 0xae, 0xaa, 0xa8, 0xaf,
   0x52, 0x7d, 0x01, 0x56, 0x55, 0x55, 0x57, 0xab, 0xba, 0x02, 0xae, 0xaa,
   0xa8, 0xaf, 0x52, 0x7d, 0x01, 0x56, 0x55, 0x55, 0x57, 0xab, 0xbb, 0x02,
   0xae, 0xaa, 0xa8, 0xaf, 0x52, 0x5f, 0x01, 0x56, 0x55, 0x55, 0x55, 0xab,
   0xaa, 0x02, 0xae, 0xaa, 0xa8, 0xaa, 0x52, 0x55, 0x01, 0x56, 0x55, 0x55,
   0x55, 0xab, 0xaa, 0x02, 0xaa, 0xaa, 0xfc, 0xff, 0xfb, 0xff, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x55, 0xa9, 0xaa, 0x52, 0x55,
   0x01, 0xaa, 0xaa, 0x55, 0x55, 0xab, 0xaa, 0x02, 0x54, 0x55, 0xa9, 0xaa,
   0x52, 0x55, 0x01, 0xaa, 0xaf, 0xd5, 0x5f, 0xab, 0xbe, 0x02, 0xd4, 0x55,
   0xa9, 0xae, 0x52, 0x77, 0x01, 0xea, 0xaa, 0x55, 0x57, 0xab, 0xff, 0x02,
   0xf4, 0x57, 0xa9, 0xaf, 0x52, 0x77, 0x01, 0xea, 0xae, 0x55, 0x5d, 0xab,
   0xbe, 0x02, 0xf4, 0x5f, 0xa9, 0xbe, 0x52, 0x77, 0x01, 0xea, 0xae, 0x55,
   0x5d, 0xab, 0xff, 0x02, 0xf4, 0x5f, 0xe9, 0xbe, 0x52, 0x77, 0x01, 0xea,
   0xaf, 0xd5, 0x5f, 0xab, 0xbe, 0x02, 0x54, 0x55, 0xa9, 0xaa, 0x52, 0x55,
   0x01, 0xaa, 0xaa, 0x55, 0x55, 0xab, 0xaa, 0x02, 0xfe, 0xff, 0xfd, 0xff,
   0xfb, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa,
   0x55, 0x55, 0x53, 0x55, 0x01, 0x54, 0x55, 0xa9, 0xaa, 0xaa, 0xaa, 0x02,
   0xaa, 0xaa, 0x55, 0x55, 0x53, 0x55, 0x01, 0xd4, 0x57, 0xe9, 0xbf, 0xaa,
   0xbe, 0x02, 0xea, 0xaf, 0x55, 0x5d, 0x53, 0x5f, 0x01, 0x74, 0x5d, 0xa9,
   0xbe, 0xaa, 0xbf, 0x02, 0xaa, 0xae, 0x55, 0x5d, 0xd3, 0x5d, 0x01, 0x54,
   0x57, 0xa9, 0xae, 0xea, 0xbe, 0x02, 0xaa, 0xab, 0x55, 0x57, 0xd3, 0x7f,
   0x01, 0xd4, 0x55, 0xa9, 0xaf, 0xaa, 0xbe, 0x02, 0xea, 0xaf, 0x55, 0x57,
   0x53, 0x5d, 0x01, 0xf4, 0x5f, 0xa9, 0xaf, 0xaa, 0xbe, 0x02, 0xaa, 0xaa,
   0x55, 0x55, 0x53, 0x55, 0x01, 0x54, 0x55, 0xa9, 0xaa, 0xaa, 0xaa, 0x02,
   0xaa, 0xaa, 0x55, 0x55, 0x53, 0x55, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};

#define PUZZLE_32_width 32
#define PUZZLE_32_height 32
#define PUZZLE_32_x_hot 0
#define PUZZLE_32_y_hot 0
static char PUZZLE_32_bits[] = {
   0xff, 0x53, 0x35, 0x55, 0xab, 0xaa, 0xaa, 0xaa, 0x55, 0xd1, 0x35, 0x7d,
   0xab, 0xaa, 0xab, 0xae, 0x55, 0xd1, 0x35, 0x7d, 0xab, 0xaa, 0xab, 0xae,
   0x55, 0xd1, 0x35, 0x75, 0xab, 0xaa, 0xab, 0xba, 0x55, 0x51, 0x35, 0x55,
   0xab, 0xfa, 0xbf, 0xff, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa,
   0x55, 0x53, 0x35, 0x55, 0xfa, 0xea, 0xab, 0xbe, 0x5d, 0x53, 0x37, 0x77,
   0xfa, 0xaa, 0xab, 0xbe, 0x7d, 0x53, 0x37, 0x77, 0xea, 0xea, 0xaa, 0xbe,
   0x7d, 0xd3, 0x37, 0x5d, 0xaa, 0xaa, 0xaa, 0xaa, 0x55, 0x53, 0x35, 0x55,
   0xff, 0xfb, 0xbf, 0xff, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0xaa, 0xaa,
   0x55, 0x53, 0x35, 0x55, 0xfa, 0xea, 0xaf, 0xba, 0x55, 0x53, 0x37, 0x7d,
   0xea, 0xaa, 0xab, 0xb6, 0x75, 0xd3, 0x37, 0x7f, 0xfa, 0xaa, 0xab, 0xba,
   0x55, 0x53, 0x35, 0x55, 0xaa, 0xaa, 0xaa, 0xaa};

#define sm_puzzle_width 17
#define sm_puzzle_height 17
static char puzzle_17_bits[] = {
   0x00, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x02, 0x80, 0x00, 0xda, 0xb6, 0x00,
   0xda, 0xb6, 0x00, 0x02, 0x80, 0x00, 0xda, 0xb6, 0x00, 0xda, 0xb6, 0x00,
   0x02, 0x80, 0x00, 0xda, 0xb6, 0x00, 0xda, 0xb6, 0x00, 0x02, 0x80, 0x00,
   0xda, 0x86, 0x00, 0xda, 0x86, 0x00, 0x02, 0x80, 0x00, 0xfe, 0xff, 0x00,
   0x00, 0x00, 0x00};


MrmHierarchy		puzzle_hierarchy;	/* DRM database hierarch id */
Widget 			toplevel, main_window, workarea = NULL;
Widget 			settings_widget = NULL;
Widget 			help_message_widget = NULL;
Widget 			Messages_widget = NULL;
XmPushButtonWidget 	settings_ok;
XmScaleWidget 		level_scale;
XmRowColumnWidget		opt_popup_menu = NULL;

XrmDatabase		user_database = NULL;
XrmDatabase		system_database = NULL;
XrmDatabase		merged_database;

Display			*dpy;
Window			win = NULL;
tiledef 		tile [MAX_TILES];

XtWidgetGeometry 	pop_request, pop_reply;	/* Request, reply and result */
XtGeometryResult 	pop_result;		/* for pop geom requst	     */

Dimension		width, height;
Dimension		top_space, left_space;
Dimension		bottom_space, right_space;
Dimension		tile_width, tile_height;
Dimension		real_tile_width, real_tile_height;
int			num_across, num_tiles;
Arg 			args [20];
int 			ac, as;
XFontStruct     	*font;
GC			gcdraw, gcline, gcreverse;
int			lineheight;
int			linewidth = 0;

char			result [200];
int			InfoMessage = TRUE;
                    
int 			nochoice;
int			Number_Entered;
int 			GameOver;
int 			map [MAX_TILES] [MAX_TILES];
int 			empty_row, empty_col, moves, clicks;
int 			reversed = 0, oldorder;
int 			GameLevel = 4, oldlevel;
int 			BellVolume = 0;
int 			slidefactor = 0;
Boolean			from_newgame = FALSE;

XRectangle		rects [250];
int			rc;
XSegment		segs [300];
int			lc;

Cardinal		status;
char			appl_title [300];

char			base_defaults_name [300];
char			*defaults_name;
char			*system_defaults_name;

/*
 * Puzzle Resource specifications
 */
#ifdef VMS
char	xrm_level_name [] = "Puzzle*game_level.value";
char	xrm_level_class [] = "Puzzle*game_level.Value";

char	xrm_geometry_name [] = "Puzzle.geometry";
char	xrm_geometry_class [] = "Puzzle.Geometry";

char	xrm_x_name [] = "Puzzle.x";
char	xrm_x_class [] = "Puzzle.X";
char	xrm_y_name [] = "Puzzle.y";
char	xrm_y_class [] = "Puzzle.Y";
char	xrm_width_name [] = "Puzzle.width";
char	xrm_width_class [] = "Puzzle.Width";
char	xrm_height_name [] = "Puzzle.height";
char	xrm_height_class [] = "Puzzle.Height";
#else
char	xrm_level_name [] = "DXpuzzle*game_level.value";
char	xrm_level_class [] = "DXpuzzle*game_level.Value";

char	xrm_geometry_name [] = "DXpuzzle.geometry";
char	xrm_geometry_class [] = "DXpuzzle.Geometry";

char	xrm_x_name [] = "DXpuzzle.x";
char	xrm_x_class [] = "DXpuzzle.X";
char	xrm_y_name [] = "DXpuzzle.y";
char	xrm_y_class [] = "DXpuzzle.Y";
char	xrm_width_name [] = "DXpuzzle.width";
char	xrm_width_class [] = "DXpuzzle.Width";
char	xrm_height_name [] = "DXpuzzle.height";
char	xrm_height_class [] = "DXpuzzle.Height";
#endif


/* routine to make a pixmap stolen from session mgr */
static Pixmap MakePixmap (dpy, root, data, width, height) 
Display *dpy;
Drawable root;
short *data;
Dimension width, height;
{
    Pixmap pid;
    unsigned long	ScreenNumber;

    ScreenNumber = XDefaultScreen (dpy);

    pid = XCreatePixmapFromBitmapData (dpy, root, data,
		(Dimension) width, (Dimension) height, 
		(unsigned long) WhitePixel (dpy, ScreenNumber),
		(unsigned long) BlackPixel (dpy, ScreenNumber), 1);
    return(pid);
}


#ifdef UNIX
/*
 * createHomeDirSpec(simple)
 *   char *simple;		File name of the desired file in the users
 *				home directory
 * Returns a file spec for the named file in the users home directory
 * space needs to be freed with XtFree
*/
static char *createHomeDirSpec(simple)
char	*simple;
{
	char *dir = NULL;
	char *spec;
	char *user;
	struct passwd *pw;
	int count = 0;
 
	while (count < 3) {
		switch (count) {
		      case 0:
			dir = (char *)getenv("HOME");
			break;
		      case 1:
			if((user = (char *)getenv("USER")) == NULL) {
				break;
			}
			if((pw  = getpwnam(user)) == NULL) {
				break;
			}
			dir = pw->pw_dir;
			break;
		      case 2:
			if((pw  = getpwuid(getuid())) == NULL) {
				break;
			}
			dir = pw->pw_dir;
			break;
		}
		count++;
		if(dir == NULL)
		  continue;
		if( !access(dir, F_OK) )
		  break;
	}
 
	spec = (char *) XtMalloc(strlen(dir)+strlen(simple)+2);
	strcpy(spec,dir);
	strcat(spec,"/");
	strcat(spec,simple);
	return(spec);
}
#endif


#ifdef COMBINE
int puzzle_main(argc, argv)
#else
int main(argc, argv)
#endif
int argc;
char **argv;
{
    FILE 	*inp;
    Pixmap 	icon_pixmap, sm_icon_pixmap;
    Widget 	unmanaged_toplevel;

    width = 0; height = 0;
    oldlevel = GameLevel;

    pop_request. request_mode = CWStackMode;
    pop_request. stack_mode = Above;

   /* Initialize DRM */
                 
    MrmInitialize ();
    DXmInitialize ();
 
    unmanaged_toplevel  = XtInitialize (NULL, CLASS_NAME, NULL, 0, &argc, argv);

    toplevel = XtAppCreateShell (APPL_NAME, CLASS_NAME, applicationShellWidgetClass,
				 XtDisplay(unmanaged_toplevel), NULL, 0);

    dpy = XtDisplay (toplevel);

    FetchHeierchy ();

    puzzle_get_uil_string (puzzle_hierarchy, "PUZZLE_APPLICATION_TITLE",
			   appl_title);

    /*  Add an event handler to track Reparent notify events.
     */
    XtAddEventHandler( toplevel, StructureNotifyMask, False,
                       set_icons_on_shell, None );

#ifdef UNIX
    puzzle_get_uil_string (puzzle_hierarchy, "PUZZLE_UNIX_DEFAULTS_NAME",
			   base_defaults_name);
    defaults_name = createHomeDirSpec (base_defaults_name);
    puzzle_get_uil_string (puzzle_hierarchy, "PUZZLE_UNIX_SYS_DEFAULTS_NAME",
			   base_defaults_name);
#else
    puzzle_get_uil_string (puzzle_hierarchy, "PUZZLE_VMS_DEFAULTS_NAME",
			   base_defaults_name);
    defaults_name = (char *) XtMalloc (strlen (base_defaults_name) + 2);
    strcpy (defaults_name, base_defaults_name);
    puzzle_get_uil_string (puzzle_hierarchy, "PUZZLE_VMS_SYS_DEFAULTS_NAME",
			   base_defaults_name);
#endif
    system_defaults_name = (char *) XtMalloc (strlen (base_defaults_name) + 2);
    strcpy (system_defaults_name, base_defaults_name);

    merged_database = XtDatabase (dpy);

    CreateMain ();
    num_across = GameLevel;
    num_tiles = (num_across * num_across) - 1;
    CreateTiles (workarea);
                                       
    srand (time (NULL));

    NewGame ();

/* Replaced by the set_icons call for supporting multiple size icons. */

/*  icon_pixmap = MakePixmap (dpy, XDefaultRootWindow (dpy),
		puzzle_bits, puzzle_width, puzzle_height);

    sm_icon_pixmap = MakePixmap (dpy, XDefaultRootWindow (dpy),
       		sm_puzzle_bits, sm_puzzle_width, sm_puzzle_height);

    ac = 0;
    XtSetArg (args [ac],  XmNtitle, appl_title); ac++;
    XtSetArg (args [ac],  XtNiconName, appl_title); ac++;
    XtSetArg (args [ac],  XtNiconPixmap, icon_pixmap); ac++;
    XtSetArg (args [ac],  XtNiconifyPixmap, sm_icon_pixmap); ac++; 
    XtSetArg (args [ac],  XtNallowShellResize, TRUE); ac++;
    XtSetValues (toplevel, args, ac);
*/

    /*  Set the icon pixmap after we have realized the shell.
     */
    set_icons( toplevel, puzzle_hierarchy );

    XtRealizeWidget (toplevel);

    CreateGC ();
    XtMainLoop ();
}		                          

/*
 *  Callback routine which sets the icon pixmaps for Reparenting
 *  window managers.
 */
static void
set_icons_on_shell( shell, user_data, event )
    Widget   	    shell;
    caddr_t 	    user_data;	/* unused */
    XEvent   	    *event;
{
    XIconSize       *size_list;
    int	    	    num_sizes;
    Display 	    *dpy = XtDisplay( shell );
    Window  	    root_window = XDefaultRootWindow( dpy );
    XReparentEvent  *reparent = (XReparentEvent *) &event->xreparent;

    if ( event->type != ReparentNotify )
    	return;

    /* Ignore reparents back to the root window.
     */
    if ( reparent->parent == root_window )
    	return;

    /*  Set the icons for this shell.
     */
    if ( ! XGetIconSizes( dpy, root_window, &size_list, &num_sizes ) )
        return;
    else
    {
    	XFree( size_list );
    	set_icons( shell, puzzle_hierarchy );
    }

};  /* end of set_icons_on_shell */


/*
 *  Sets the icon and iconify pixmaps for the given shell widget.
 */
void
set_icons( shell, hierarchy_id )
    Widget  	    shell;
    MrmHierarchy    hierarchy_id;
{
    Display 	    	*dpy = XtDisplay( shell );
    Screen  	    	*scr = XtScreen( shell );
    unsigned int    	icon_size;
    char	    	*icon_name;
    static char     	*shell_icon_sizes[] = { "75", "50", "32", "17" };
    static int	    	num_sizes = XtNumber( shell_icon_sizes );
    static unsigned int	current_icon_size = 0;
    static Pixmap   	icon_pixmap = 0;
    static Pixmap   	iconify_pixmap = 0;
/* I18N change */
    XmString	appl_title_cs;
    long	byte_count, cvt_status;
/* I18n change */

    /* Determine the icon pixmap name and size to fetch.
     */
    icon_name = get_icon_index_name( dpy, "ICON_PIXMAP", &icon_size, 
    	    	    	    	     shell_icon_sizes, num_sizes );
    if ( icon_name != NULL )
    {
    	/*  If the icon sizes are different we need to free the current
    	 *  ones, and re-fetch new icons.  We assume that re-fetching
    	 *  new icons is an infrequent operation, so we don't cache the 
    	 *  old icons.
    	 */
    	if ( ( current_icon_size != 0 )	    	    	/* Icon exists.     */
    	     && ( current_icon_size != icon_size ) )	/* New icon needed. */
    	{
    	    if ( icon_pixmap )
    	    	XFreePixmap( dpy, icon_pixmap );
    	    if ( ( iconify_pixmap ) && ( iconify_pixmap != icon_pixmap ) )
    	    	XFreePixmap( dpy, iconify_pixmap );
    	    icon_pixmap = 0;
    	    iconify_pixmap = 0;
    	    current_icon_size = 0;
    	}
    	if ( current_icon_size == 0 )
    	{
    	    current_icon_size = icon_size;
    	    icon_pixmap = fetch_bitmap_icon(dpy, scr, icon_size);
    	}
    	XtFree( icon_name );
    	icon_name = NULL;
    }
    else    /* Can't get icon sizes for some reason */
    	return;

    /* Fetch the iconify pixmap for compatibility with the XUI window manager.
     */
    if ( DXIsXUIWMRunning( shell, 0 ) )
    {
    	if ( icon_size == 17 )  	    /* Don't fetch icon twice */
    	    iconify_pixmap = icon_pixmap;
    	else if ( icon_size > 17 )
    	    iconify_pixmap = fetch_bitmap_icon(dpy, scr, 17);
    }

    /* Set the icon pixmap on the shell.
     */
    if ( icon_pixmap )
    {
    ac = 0;
/* I18N cnanges */
    appl_title_cs = DXmCvtFCtoCS (appl_title, &byte_count, &cvt_status);
    DWI18n_SetTitle(toplevel, appl_title_cs);
    DWI18n_SetIconName(toplevel, appl_title_cs);
    XmStringFree (appl_title_cs);

/*  XtSetArg (args [ac],  XmNtitle, appl_title); ac++;
    XtSetArg (args [ac],  XtNiconName, appl_title); ac++; */
/* end I18n changes */

    XtSetArg (args [ac],  XtNiconPixmap, icon_pixmap); ac++;
/*  XtSetArg (args [ac],  XtNiconifyPixmap, sm_icon_pixmap); ac++; */
    XtSetArg (args [ac],  XtNallowShellResize, TRUE); ac++;
    XtSetValues (toplevel, args, ac);
/*
    	XWMHints    *wmhints = NULL;

    	wmhints = XGetWMHints( dpy, XtWindow( shell ) );
    	if ( wmhints != NULL )
    	{
    	    wmhints->flags |= IconPixmapHint;
    	    wmhints->icon_pixmap = icon_pixmap;
    	    XSetWMHints( dpy, XtWindow( shell ), wmhints );
    	    XFree( wmhints );
    	}
*/
    }

    /* Set the iconify pixmap for the XUI window manager 
     */
    if ( (iconify_pixmap) && (XtIsRealized(shell)) )
    	set_iconify_pixmap( shell, iconify_pixmap );

};  /* end of set_icons */


/*
 *	From the icon_size requested, create a pixmap from the icon bitmap
 *	data corresponded to the size requested.
 */
Pixmap
fetch_bitmap_icon(dpy, scr, icon_size)
    Display 	    *dpy;
    Screen  	    *scr;
    int		    icon_size;
{
    int	    	    status;
    Pixmap  	    pixmap_rtn;
    Window  	    root = XDefaultRootWindow (dpy);

    switch (icon_size) {
	case  75 :
	    pixmap_rtn = MakePixmap (dpy, root, PUZZLE_75_bits, icon_size, icon_size);
	    break;

	case  50 :
	    pixmap_rtn = MakePixmap (dpy, root, PUZZLE_50_bits, icon_size, icon_size);
	    break;

	case  32 :
	    pixmap_rtn = MakePixmap (dpy, root, PUZZLE_32_bits, icon_size, icon_size);
	    break;

	case  17 :
	    pixmap_rtn = MakePixmap (dpy, root, puzzle_17_bits, icon_size, icon_size);
	    break;

    }
    return (Pixmap) pixmap_rtn;

};   /* end of fetch_bitmap_icon */


/*
 *  Finds the largest icon supported by the window manager and returns
 *  a string which represents that icon in UIL.
 */
static char *
get_icon_index_name( dpy, root_index_name, icon_size_rtn, 
    	    	supported_icon_sizes, num_supported_sizes )
    Display 	    *dpy;
    char    	    *root_index_name;
    unsigned int    *icon_size_rtn;
    char    	    **supported_icon_sizes;
    int	    	    num_supported_sizes;
{
    XIconSize	*size_list;
    int	    	num_sizes;
    int	    	cursize;
    int	    	i;
    char    	*icon_index = NULL;
    int	    	icon_size;
    char    	*icon_size_ptr;
    Boolean 	found_icon_size = False;

    *icon_size_rtn = 0;	    /* Initial value */

    if ( ! XGetIconSizes( dpy, XDefaultRootWindow( dpy ), 
    	    	    	  &size_list, &num_sizes ) )
    	return ( NULL );

    /* Find the largest icon supported by the window manager.
     */
    cursize = 0;
    for ( i = 1; i < num_sizes; i++ )
    {
    	if ( ( size_list[i].max_width >= size_list[cursize].max_width )
    	      && ( size_list[i].max_height >= size_list[cursize].max_height ) )
    	    cursize = i;
    }
    if ( ( size_list[cursize].max_width <= 0 ) 
    	 || ( size_list[cursize].max_height <= 0 ) )
    {
    	XFree( size_list );
    	return ( NULL );
    }

    /* Find the largest supported icon.
     */
    for ( i = 0; i < num_supported_sizes; i++ )
    {
    	icon_size = atoi( supported_icon_sizes[i] );
    	if ( ( icon_size <= size_list[cursize].max_width )
    	      && ( icon_size <= size_list[cursize].max_height ) )
    	{
    	    icon_size_ptr = supported_icon_sizes[i];
    	    found_icon_size = TRUE;
    	    break;
    	}
    }
    XFree( size_list );

    /*  Build the icon index name
     *  
     *     format: root_index_name + "_" + icon_size_ptr + "X" + icon_size_ptr
     */
    if ( found_icon_size )
    {
    	icon_index = (char *) XtMalloc( strlen( root_index_name ) 	+
    	    	    	    	    	sizeof( "_" )	    	    	+
    	    	    	    	        ( 2 * sizeof( icon_size_ptr ) ) +
    	    	    	    	    	1 );    /* for \0 char */
    	strcpy( icon_index, root_index_name );
    	strcat( icon_index, "_" );
    	strcat( icon_index, icon_size_ptr );
    	strcat( icon_index, "X" );
    	strcat( icon_index, icon_size_ptr );

    	*icon_size_rtn = (unsigned int) icon_size;
    }

    return( icon_index );

};  /* end of get_icon_index_name */


/*
 *  Sets the iconify pixmap in the DEC_WM_HINTS property for the 
 *  given shell widget.
 */
static void
set_iconify_pixmap( shell, iconify_pixmap )
    Widget shell;
    Pixmap iconify_pixmap;
{
typedef unsigned long int   INT32;
typedef struct {
        INT32 value_mask;
        INT32 iconify_pixmap;
        INT32 icon_box_x;
        INT32 icon_box_y;
        INT32 tiled;
        INT32 sticky;
        INT32 no_iconify_button;
        INT32 no_lower_button;
        INT32 no_resize_button;
} internalDECWmHintsRec, *internalDECWmHints;

#define WmNumDECWmHintsElements ( sizeof( internalDECWmHintsRec ) / sizeof( INT32 ) )

    internalDECWmHints	  prop = 0;
    internalDECWmHintsRec prop_rec;
    Atom    	    	  decwmhints;
    Atom    	    	  actual_type;
    int     	    	  actual_format;
    long    	    	  leftover;
    unsigned long   	  nitems;
    Boolean 	    	  free_prop = True;

    decwmhints = XmInternAtom( XtDisplay( shell ), "DEC_WM_HINTS", False );

    if ( XGetWindowProperty( XtDisplay( shell ), XtWindow( shell ), decwmhints,
    	    	0L, (long)WmNumDECWmHintsElements, False, decwmhints, 
    	    	&actual_type, &actual_format, &nitems, 
    	    	&leftover, (unsigned char **)&prop ) 
    	    != Success ) 
    	return;
    if ( ( actual_type != decwmhints ) 
    	 || ( nitems < WmNumDECWmHintsElements ) || ( actual_format != 32 ) )
    {
    	if ( prop != 0 ) XFree ( (char *)prop );

    	/*  An empty "value" was returned, so create a new one.
    	 */
    	free_prop = False;
    	prop = (internalDECWmHints *) &prop_rec;
    	prop->value_mask    	= 0;
    	prop->icon_box_x 	= -1;
    	prop->icon_box_y 	= -1;
    	prop->tiled 	    	= False;
    	prop->sticky     	= False;
    	prop->no_iconify_button	= False;
    	prop->no_lower_button	= False;
    	prop->no_resize_button	= False;
    }

    prop->value_mask	 |= DECWmIconifyPixmapMask;
    prop->iconify_pixmap  = iconify_pixmap;

    XChangeProperty( XtDisplay( shell ), XtWindow( shell ), decwmhints, 
    	    	     decwmhints, 32, PropModeReplace, 
    	    	     (unsigned char *) prop, WmNumDECWmHintsElements );

    if ( free_prop )
    	if ( prop != 0 ) XFree ( (char *)prop );

};  /* end of set_iconify_pixmap */

 

void NewGame ()
{
int i, r, c, tile_num;

  from_newgame = TRUE;
  GameOver = FALSE;
  moves = clicks = r = c = 0;
  for (i = 0; i < num_tiles; i++) {
    if (reversed) tile_num = num_tiles - i - 1;
    else tile_num = i;
    tile [tile_num]. row = r; tile [tile_num]. col = c;
    map [r] [c] = tile_num;
    if ((c = (c+1) % num_across) == 0) r++;
  }                             
  map [num_across-1] [num_across-1] = EMPTY;

  if (win != NULL) XClearWindow (dpy, win);
  setupboard ();
  PlaceTiles ();

  if (win != NULL) DrawWindow (0, 0, width, height);

} /* NewGame () */




/*
 * GetRand (x) returns a random number between 1 and X
 * Input:	 X - Maximum random number
 * Output:	 The random number
 * Side Effects: none
 *
 * NOTE: There is a bug in tha VAX c random number generator function.
 *	 The rand () function is cyclic on powers of 2 less than 2**16.
 *	 The simple fix is to always use an odd number and then adjust
 *	 the result (ignore 0's).
 */
int GetRand (x)
int x;
{
int res;

  res = 0;
  if (x % 2 == 0) {x++; while (res == 0) res = rand () % x; }
  else { res = rand () % x; res++; }
  return (res);

} /* GetRand (x) */


/*
 * setupboard () randomizes the puzzle to create a new puzzle
 *
 * Input:	 None
 *
 * Ouput:	 None
 *
 * Side Effects: tile, map
 */
void setupboard ()
{
int i, r, c, k, loc;
int num_moves;

  r = c = num_across - 1;
  num_moves = 1000; i = 0;

  while (i < num_moves) {
    k = GetRand (4);
    switch (k) {
	case 1: { if (r) {
		    r--; tile [map [r] [c]]. row++; loc = map [r] [c]; i++;
		    map [r+1] [c] = map [r] [c]; map [r] [c] = EMPTY;
		} /* if */
		break; }                                     

	case 2: { if (c < num_across - 1) {
   		    c++; tile [map [r] [c]]. col--; loc = map [r] [c]; i++;
		    map [r] [c-1] = map [r] [c]; map [r] [c] = EMPTY;
		} /* if */
		break; }

	case 3: { if (r < num_across - 1) {
		    r++; tile [map [r] [c]]. row--; loc = map [r] [c]; i++;
		    map [r-1] [c] = map [r] [c]; map [r] [c] = EMPTY;
		} /* if */
		break; }

	case 4: { if (c) {
		    c--; tile [map [r] [c]]. col++; loc = map [r] [c]; i++;
		    map [r] [c+1] = map [r] [c]; map [r] [c] = EMPTY;
		} /* if */
		break; }

    } /* switch */

  } /* while */ 
  empty_row = r; empty_col = c;

} /* setupboard */


/*
 * makemove (from) moves a square (or column of squares) toward the 
 * empty square.
 *
 * Input:	 from - the end square to slide from
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
makemove (from)
int from;
{  
int i, r, c, dist, dir, sq, xinc, yinc;

  Number_Entered = 0;  /* A move is made. zero number entered from key board. */
  
  /* if tried to move a non existing tile,  do nothing */
  if ((from < 0) || (from > (num_tiles-1)))  return;

  clicks++;
  r = tile [from]. row; c = tile [from]. col;

  if (r == empty_row) {
    if (c < empty_col) dir = 2;
    else dir = 4;
    dist = abs (empty_col - c);
  } /* if */
  else if (c == empty_col) {
    if (r < empty_row) dir = 3;
    else dir = 1;
    dist = abs (empty_row - r);
  } /* if */
  else { JogWiggle (from); return (0); }

  moves = moves + dist;
  while (dist) {
    xinc = yinc = 0;
    switch (dir) {
   	case 1: { sq = map [r - dist + 1] [c]; yinc = -1; break; }
	case 2: { sq = map [r] [c + dist - 1]; xinc = 1; break; }
	case 3: { sq = map [r + dist - 1] [c]; yinc = 1; break; }
	case 4: { sq = map [r] [c - dist + 1]; xinc = -1; break; }
    } /* switch */
    slide (sq, xinc, yinc);
    dist--;
  } /* while */
  
} /* makemove (from) */


/*
 * slide (from) moves a square toward the empty square.
 *
 * Input:	 from - the square to slide
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
slide (from, xinc, yinc)
int from, xinc, yinc;
{
Position	x, y;
Position	destx, desty;
Position	nextx, nexty;
Dimension	tilesize;
int 		times, i, r, c;

    r = tile [from]. row; c = tile [from]. col;

    destx = empty_col * tile_width + left_space;
    desty = empty_row * tile_height + top_space;

    if (c == empty_col) tilesize = tile_height;
    else tilesize = tile_width;
   
    if (slidefactor == 0) slidefactor = tilesize;
    times = tilesize / slidefactor; 
    xinc = slidefactor * xinc; yinc = slidefactor * yinc;
    nextx = c * tilesize + xinc + left_space;
    nexty = r * tilesize + yinc + top_space;

/*
    for (i = 0; i < times; i++) {
      ac = 0;
      XtSetArg (args [ac], XmNx, nextx); ac++;
      XtSetArg (args [ac], XmNy, nexty); ac++;
      XtSetValues (tile [from]. wid, args, ac);
      nextx = nextx + xinc; nexty = nexty + yinc;
    }
    ac = 0;
    XtSetArg (args [ac], XmNx, destx); ac++;
    XtSetArg (args [ac], XmNy, desty); ac++;
    XtSetValues (tile [from]. wid, args, ac);
*/
/*
    XFillRectangle (dpy, win, gcreverse, empty_col*tile_width + left_space,
		empty_row*tile_height + top_space, tile_width, tile_height);
*/

    tile [from]. x = destx;
    tile [from]. y = desty;
    drawtile (from, TRUE);

    tile [from]. row = empty_row; tile [from]. col = empty_col;
    map [empty_row] [empty_col] = from;
    map [r] [c] = EMPTY;

    empty_row = r; empty_col = c;

    XFillRectangle (dpy, win, gcline, empty_col*tile_width + left_space,
		empty_row*tile_height + top_space, tile_width, tile_height);
} /* slide (from, xinc, yinc) */



/*
 * int JogWiggle (r, c) returns a square number if row r and col c is a valid
 * move.  If the squares is not legal move, the square is "wiggled"
 *                                                                 
 * Input:	 r, c - row and col of the pointer when the button was pressed
 *
 * Ouput:	 A -1 for illegal move, the square number for legal move
 *
 * Side Effects: None
 */
JogWiggle (sq)
int sq;
{
int r, c;

  r = tile [sq]. row; c = tile [sq]. col;

  if (r > empty_row) {
    if (c > empty_col) {
      jog (sq, -1, -1);
      wiggle (map [r-1] [c]);
      wiggle (map [r] [c-1]);

      if ((r - 1 == empty_row) && (c - 1 == empty_col))
        XFillRectangle (dpy, win, gcline, empty_col*tile_width + left_space,
		empty_row*tile_height + top_space, tile_width, tile_height);
      else drawtile (map [r-1] [c-1], TRUE);
    } 
    else {
	jog (sq, 1, -1);
	wiggle (map [r-1] [c]);
	wiggle (map [r] [c+1]);
    }  
  } 
  else { if (c > empty_col) {
	   jog (sq, -1, 1);
	   wiggle (map [r+1] [c]);
	   wiggle (map [r] [c-1]);
         } 
       	else {
	   jog (sq, 1, 1);
	   wiggle (map [r+1] [c]);
	   wiggle (map [r] [c+1]);
         } 
  } 
  wiggle (sq);
  if (BellVolume) XBell (dpy, BellVolume);

} /* */


/*
 * jog (sq, xinc, yinc) moves a tile a small amount and back that can't move
 *
 * Input:	 sq - the sqaure to be moved
 *		 xinc, yinc - the the direction to move
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
jog (sq, xinc, yinc)
int sq, xinc, yinc;
{
Position	xpos, ypos;
int 		i, r, c;
static 		int incr [4] = {2, 2, -2, -2};

  r = tile [sq]. row; c = tile [sq]. col;

  xpos = c * tile_width + left_space;
  ypos = r * tile_height + top_space;

  for (i = 0; i < 4; i++) {
    xpos = xpos + xinc*incr [i];
    ypos = ypos + yinc*incr [i];
    tile [sq]. x = xpos;
    tile [sq]. y = ypos;
    drawtile (sq, TRUE);
  }	

  xpos = c * tile_width + left_space;
  ypos = r * tile_height + top_space;

  tile [sq]. x = xpos;
  tile [sq]. y = ypos;
  drawtile (sq, TRUE);

} /* jog (sq, xinc, yinc) */


/*                                 
 * wiggle (sq) moves a square back and forth that was next to a square that
 * could not legally move
 *
 * Input:	 sq - the square to be wiggled
 *
 * Ouput:	 None
 *
 * Side Effects: None
 */
wiggle (sq)
int sq;
{
Position	xpos, ypos;
int 		i, r, c;
                                    
  r = tile [sq]. row; c = tile [sq]. col;

  xpos = tile [sq]. col * tile_width + left_space;
  ypos = tile [sq]. row * tile_height + top_space;

  tile [sq]. x = xpos + 2;
  tile [sq]. y = ypos + 2;
  drawtile (sq, TRUE);

  tile [sq]. x = xpos;
  tile [sq]. y = ypos;
  drawtile (sq, TRUE);

  if ((r < (num_across - 1)) && !((r + 1 == empty_row) && (c == empty_col))) {
    drawtile (map [r+1] [c], TRUE);
  }

  if ((c < (num_across - 1)) && !((r == empty_row) && (c + 1 == empty_col))) {
    drawtile (map [r] [c+1], TRUE);
  }

} /* wiggle (sq) */



/*
 * int outoforder () tests the current arangement of the squares for proper
 * solution.  Returns true if squares represent a solution, False otherwise.
 *
 * Input:	 None
 *
 * Ouput:	 True if squares are in a solution, False otherwise
 *
 * Side Effects: None
 */                                  
int outoforder()
{
int i, r, c;

  r = c = 0;
  for (i = 0; i < num_tiles; i++) {
    if (reversed)
      if (map [r] [c] != (num_tiles - 1 - i))
        return (-1);
      else ;
    else if (map [r] [c] != i) return (-1);
    if ((c = (c+1) % num_across) == 0) r++;
  } /* for */

  return (0);

} /* int outoforder() */


/************************************************************************
 * puzzle_get_uil_string
 *
 *	Fetches a UIL string using DRM.  The UI utility routine cannot
 *	be used since a context block does not exist at this point.
 ************************************************************************
 */
puzzle_get_uil_string (hierarchy, name_str, buffer)
MrmHierarchy	hierarchy;
char		*name_str;
char		*buffer;
{
    Cardinal			status;
    caddr_t			*value;
    MrmCode			*type;

    /* Now get the literal using DRM. */
    status = MrmFetchLiteral (
	hierarchy,
	name_str,
	dpy,
	&value,
	&type);

    strcpy(buffer, value);
    XtFree(value);

}
