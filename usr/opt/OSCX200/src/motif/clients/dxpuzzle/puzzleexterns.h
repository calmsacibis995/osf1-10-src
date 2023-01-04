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
 
extern	MrmHierarchy	puzzle_hierarchy;	/* DRM database hierarch id */
extern	Widget		toplevel, main_window;
extern	XmBulletinBoardWidget workarea;
extern	Widget 		settings_widget;
extern	Widget 		help_message_widget;
extern	Widget		Messages_widget;
extern	XmPushButtonWidget settings_ok;
extern	XmScaleWidget	level_scale;
extern	XmRowColumnWidget	opt_popup_menu;

extern	XrmDatabase	user_database;
extern	XrmDatabase	system_database;
extern	XrmDatabase	merged_database;

extern	Display		*dpy;
extern	Window		win;
extern	tiledef 	tile [MAX_TILES];
extern	Dimension	width, height;
extern	int		num_across, num_tiles;

extern	XtWidgetGeometry pop_request, pop_reply;/* Request, reply and result */
extern	XtGeometryResult pop_result;		/* for pop geom requst	     */

extern	Dimension	top_space, left_space;
extern	Dimension	bottom_space, right_space;
extern	Dimension	tile_width, tile_height;
extern	Dimension	real_tile_width, real_tile_height;
extern	Arg 		args [20];
extern	int 		ac, as;
extern	XFontStruct	*font;
extern	GC		gcdraw, gcline, gcreverse;
extern	int		lineheight;
extern	int		linewidth;

extern	char		result [200];
extern	int		InfoMessage;
                    
extern	int 		nochoice;
extern	int		Number_Entered;
extern	int 		GameOver;
extern	int 		map [MAX_TILES] [MAX_TILES];
extern	int 		empty_row, empty_col, moves, clicks;
extern	int 		reversed, oldorder;
extern	int 		GameLevel, oldlevel;
extern	int 		BellVolume;
extern	int 		slidefactor;
extern	Boolean		from_newgame;

extern	XRectangle	rects [250];
extern	int		rc;
extern	XSegment	segs [300];
extern	int		lc;
extern	char		*defaults_name;
extern  char		*system_defaults_name;

extern	char		xrm_level_name [];
extern	char		xrm_level_class [];

extern	char		xrm_geometry_name [];
extern	char		xrm_geometry_class [];

extern	char		xrm_x_name [];
extern	char		xrm_x_class [];
extern	char		xrm_y_name [];
extern	char		xrm_y_class [];
extern	char		xrm_width_name [];
extern	char		xrm_width_class [];
extern	char		xrm_height_name [];
extern	char		xrm_height_class [];
