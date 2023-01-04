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
 *	@(#)$RCSfile: workstation.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/24 22:47:11 $
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
/************************************************************************
 *									*
 *			Copyright (c)  1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 * ioctl assignments must not conflict with inputdevice.h.  IOCTL's for
 * screens and devices must have appropriate bit set, so that
 * unrecognized ones can be appropriately dispatched to screen and/or
 * input device drivers.
 */

#ifndef _WORKSTATION_H_
#define _WORKSTATION_H_


#define IOC_S 64		/* ioctl has screen as first element */
#define IOC_D 128		/* ioctl has device as first element */

/*
 * All pointers in these data structures are to be used by the client.
 */

typedef unsigned long EQTime;	/* in 1 millisecond units */

typedef struct {	
        short bottom;
        short right;
        short left;
        short top;
} ws_screen_box;

/* all screen related IOCTL's must have screen as first structure element */
typedef struct {
	short screen;
} ws_screen_ioctl;

/**************************************************************************
 * This is a PLACE-HOLDER!  Drivers that have their own ioctl's all use
 * 'w'#0, and must arrange for further differentiation in the data arg.
 * ALL real ioctl's defined by ws MUST be >0 !!!  Otherwise, a perfectly
 * good ioctl assignment in another file can be superceded by one in here.
 * See the PXG driver for an example.
 **************************************************************************/
#define WORKSTATION_PRIVATE   _IOW('w', (0|IOC_S), ws_screen_ioctl)
                                                                               
typedef struct {
    short version;		/* version of driver */
    short cpu;
    short num_screens_exist;	/* count of screens on this workstation	*/
    short num_devices_exist;	/* count of of other input devices	*/
    short console_screen; 	/* which screen is console		*/
    short console_keyboard;	/* device number of default keyboard	*/
    short console_pointer;	/* device number of default pointer    	*/
} ws_descriptor;

#define GET_WORKSTATION_INFO	_IOR('w',  1, ws_descriptor)


/*
 * Colormap structures.  Much hardware must only load the map at
 * vertical retrace time or you may get visual funnies.  So a system
 * call must be performed, and the data copied to be loaded later.
 * The ioctl will return an error if the colormap is not writable.
 *
 * This structure is carefully chosen to be identical to the X protocol 
 * structure, to avoid copies.
 */
typedef struct {
	unsigned int index;		/* cmap entry to load */
	unsigned short	red;
	unsigned short	green;
	unsigned short	blue;
	unsigned short  pad;
} ws_color_cell;

typedef struct {
    short	screen;		/* which screen */
    short	map;		/* which visual of the screen	*/
    ws_color_cell *cells;
    short	start;		/* which cell to begin writing	*/
    short	ncells;		/* how many cells to process	*/
} ws_color_map_data;

#define WRITE_COLOR_MAP	_IOW('w',  (2|IOC_S), ws_color_map_data)

typedef struct {
	short screen;		/* which screen to move cursor on */
        short x;
        short y;
} ws_cursor_position;

#define SET_CURSOR_POSITION	_IOW('w', (3|IOC_S), ws_cursor_position)

/* 
 * note that the cursor and mask must be padded to 32 bits.  This is
 * natural for an X server to provide, as it uses a bitmap padded to int
 * size as its internal representation.  As the X server has been informed
 * what size cursor the hardware supports, cursors may be arbitrarily
 * transformed if they are too bit for the hardware.
 */
typedef struct {
	short screen;			/* which is to display cursor	*/
	short width, height;		/* in pixels			*/
	short x_hot;			/* maintained in driver		*/
	short y_hot;			/*        to avoid races	*/
	unsigned int *cursor;		/* in user space 		*/
	unsigned int *mask;
} ws_cursor_data;
#define LOAD_CURSOR _IOW('w', (5|IOC_S), ws_cursor_data)

typedef struct {
	short screen;			/* which is to display cursor	*/
	ws_color_cell foreground;	/* mono systems convert to HIV	*/
	ws_color_cell background;	/* and use contrasting colors	*/
} ws_cursor_color;
#define RECOLOR_CURSOR _IOW('w', (6|IOC_S), ws_cursor_color)

/*
 * monitor types		
 * May be used by server to determine RGB description to use.
 */
#define MONOCHROME 0
#define COLOR	1

#define P4              1               /* P4 phosphor */
#define PW              2               /* paper white */
#define PVRE            3               /* yellow-orange EL, approx 585 nm */
#define SONY            4               /* Sony color */
#define MAT             5               /* Matshusita color */
#define PHILLIPS        6               /* Phillips color */
#define HITACHI         7               /* Hitachi color */

                         /* who knows, presume 19"      */
#define MONITOR_VRUNK    { "VRUNK ", 333, 281, COLOR, 0 }
                         /* 19" Color Matsushita        */
#define MONITOR_VR299    { "VR299 ", 333, 281, COLOR, MAT }
                         /* 19" Color Phillips  */
#define MONITOR_VR290    { "VR290 ", 333, 281, COLOR, PHILLIPS }
                         /* 19" Mono DEC                */
#define MONITOR_VR260    { "VR260 ", 333, 281, MONOCHROME, P4 }
                         /* 19" Mono DEC improved       */
#define MONITOR_VR262    { "VR262 ", 333, 281, MONOCHROME, P4 }
                         /* 15" Monochrome Hitachi      */
#define MONITOR_VR150    { "VR150 ", 240, 203, MONOCHROME, PW }
                         /* VRE01 flat panel    */
#define MONITOR_VRE01    { "VRE01 ", 346, 292, MONOCHROME, PVRE}
                         /* 15" Color Hitachi   */
#define MONITOR_VR160    { "VR160 ", 240, 203, COLOR, HITACHI }
                         /* 16" Sony (wow!)     */
#define MONITOR_VR297    { "VR297 ", 280, 236, COLOR, SONY }
                         /* 19" Sony Trinitron, VRT19 manual, pg. 9 */
#define MONITOR_VRT19    { "VRT19 ", 343, 274, COLOR, SONY}
                         /* 16" DEC VGA (Taiwan)     */
#define MONITOR_VRC16    { "VRC16 ", 280, 236, COLOR, P4 }
                                                                               
typedef struct {
	char type[6];		/* name of monitor.  e.g. VR297 	*/
	short mm_width;		/* width, height in mm. 		*/
	short mm_height;
	short color_or_mono;	/* is monitor color or monochrome 	*/
	short phosphor_type;	/* phosphor type for RGB computation 	*/
} ws_monitor;

/*
 * Screen information block.  Some of this hair is caused in case
 * someone builds displays with multi-mapped access at different depths.
 */

typedef struct {
    short screen;		/* screen being described (IN)		*/
    ws_monitor monitor_type;	/* monitor type				*/
    char moduleID[10];		/* exact string from option ROM		*/
    short width;		/* pixels of visible screen		*/
    short height;		/* pixels of visible screen		*/
    short root_depth;		/* which depth is the root 		*/
    short allowed_depths;	/* number of depths present		*/
    short nvisuals;		/* number of visual types of screen 	*/
    short x, y;			/* current pointer position 		*/
    short row, col;		/* current text position		*/
    short max_row, max_col;	/* maximum row, col text position 	*/
    short f_width, f_height;	/* console font width and height	*/
    short cursor_width;		/* maximal size cursor for		*/
    short cursor_height;	/*	this screen.			*/
    short min_installed_maps;	/* number of visual types		*/
    short max_installed_maps;	/* max number of visual types   	*/
} ws_screen_descriptor;

/*
 * macros for cursor maintenance during emulation,
 *  where:
 *		sp == *ws_screen_descriptor
 */
#define ONE_LINE(sp)		((sp)->max_col)
#define ONE_PAGE(sp)		((sp)->max_col*(sp)->max_row)

#define TOP_LINE(sp)		(0)
#define BOTTOM_LINE(sp)		(((sp)->max_row - 1) * ONE_LINE(sp))

#define BEG_OF_LINE(sp, pos)	((pos) - ((pos) % ONE_LINE((sp))))
#define CURRENT_COLUMN(sp, pos)	((pos) % ONE_LINE((sp)))

#define ROW_POS(sp, row)	(ONE_LINE((sp)) * (row))
#define CHAR_POS(sp, row, col)	(ROW_POS((sp),(row)) + (col))
#define CUR_POS(sp) 		(ROW_POS((sp),(sp)->row) + ((sp)->col))
#define CUR_ROW_POS(sp)		(ONE_LINE((sp)) * ((sp)->row))

#define ROW_FROM_POS(sp, pos)	((pos) / ONE_LINE((sp)))
#define COL_FROM_POS(sp, pos)	((pos) % ONE_LINE((sp)))

/* given screen number, fills in all other fields about screen */
#define GET_SCREEN_INFO	_IOWR('w', (7|IOC_S), ws_screen_descriptor)

typedef struct {
    short screen;		/* which screen to query depth of (in)	*/
    short which_depth;		/* which of allowed depths to query (in)*/
    short fb_width;		/* frame buffer width in pixels		*/
    short fb_height;		/* frame buffer height in pixels 	*/
    short depth;		/* returns the depth (out)		*/
    short bits_per_pixel;	/* stride of pixel (out)		*/
    short scanline_pad;
    caddr_t physaddr;		/* phys. address (used by driver only)	*/
    caddr_t pixmap;		/* user space address at this depth 	*/
    caddr_t plane_mask_phys;	/* physical addr of plane mask (if any) */
    caddr_t plane_mask;		/* plane mask (if any) 			*/
} ws_depth_descriptor;
    
#define GET_DEPTH_INFO _IOWR('w', (8|IOC_S), ws_depth_descriptor)

#define UNMAP_SCREEN 0		/* unmap the screen (not implemented)	*/
#define MAP_SCREEN 1		/* map the screen to user space		*/

/* for frame buffer type devices, controls mapping at a depth		*/
typedef struct {
    short screen;		/* which screen to map			*/
    short which_depth;		/* at which supported depth		*/
    short map_unmap;		/*  map or unmap (not supported) screen */
} ws_map_control;

#define MAP_SCREEN_AT_DEPTH _IOW('w', (9|IOC_S), ws_map_control)

#if 0
/* ioctl 10 is defined in pxinfo.h, for PX_MAP_SCREEN*/
See comment above for ioctl[0], then look below at
GET_VISUAL_INFO...
#endif

/* the following visual classes are directly from X.h */

#ifndef StaticGray
#define StaticGray              0
#define GrayScale               1
#define StaticColor             2
#define PseudoColor             3
#define TrueColor               4
#define DirectColor             5
#endif

typedef struct {
    short screen;		/* which screen (in)			*/
    short which_visual;		/* which visual of screen (in) 		*/
    short screen_class;		/* class of visual 			*/
    short depth;		/* number of bits per pixel		*/
    unsigned long red_mask, green_mask, blue_mask; /* mask of subfields */
    short bits_per_rgb;		/* bits per RGB 			*/
    int color_map_entries;
} ws_visual_descriptor;

/* given screen number and visual number, return visual information 	*/
#define GET_VISUAL_INFO	_IOWR('w', (10|IOC_S), ws_visual_descriptor)

/* turn video on or off */

#define SCREEN_OFF 0
#define SCREEN_ON 1
typedef struct {
    short screen;		/* which screen 			*/
    short control;		/* what to do, SCREEN_ON or SCREEN_OFF 	*/
} ws_video_control;
    
#define VIDEO_ON_OFF	_IOW('w',  (11|IOC_S), ws_video_control)

typedef struct {
    short screen;
    short device;
    long  data;
} ws_tablet_control;

#define SET_TABLET_OVERHANG _IOW('w',  (13|IOC_S|IOC_D), ws_tablet_control)
                                                                               
#define CONCRETE_WALL -1	/* no connection to adjacent screen	*/

typedef struct {
    short screen;		/* which screen				*/
    ws_screen_box adj_screens;  /* which screen to attach the edge to   */
} ws_edge_connection;

#define SET_EDGE_CONNECTION _IOW('w', (14|IOC_S), ws_edge_connection)
#define GET_EDGE_CONNECTION _IOWR('w', (15|IOC_S), ws_edge_connection)

#define CURSOR_ON 1
#define CURSOR_OFF 0
typedef struct {
    short screen;		/* which screen 			*/
    short control;		/* what to do, SCREEN_ON or SCREEN_OFF 	*/
} ws_cursor_control;
    
#define CURSOR_ON_OFF	_IOW('w',  (16|IOC_S), ws_cursor_control)


typedef struct {
	short screen;		/* which screen */
	ws_monitor monitor_type;/* monitor type */
} ws_monitor_type;
#define SET_MONITOR_TYPE _IOW('w', (17|IOC_S), ws_monitor_type)


#endif /* !workstation_DEFINED */
