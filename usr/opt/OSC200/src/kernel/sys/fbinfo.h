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
 *	@(#)$RCSfile: fbinfo.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 18:45:39 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Derived from fbinfo.h	4.1	(ULTRIX)	8/10/90
 */

#ifndef fbinfo_DEFINED
#define fbinfo_DEFINED 1

#define NDEPTHS 1			/* all current hardware just has one*/
#define NVISUALS 1

struct fb_info {
	ws_screen_descriptor screen;
	ws_depth_descriptor depth[NDEPTHS];
	ws_visual_descriptor visual[NVISUALS];
	ws_screen_functions sf;
	ws_color_map_functions cmf;
	ws_cursor_functions cf;
	int (*attach)();	/* called at attach time (if defined)	*/
	void (*interrupt)();	/* called at interrupt time (if defined)*/
        void (*bot)();          /* called at beginning of time (console init)*/
};

#endif /* !fbinfo_DEFINED */
