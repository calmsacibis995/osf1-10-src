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
 * @(#)$RCSfile: vgainfo.h,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/03 14:30:41 $
 */
#ifndef _VGAINFO_H_
#define _VGAINFO_H_ 1

/* Created using fbinfo.h as template. */

#define NDEPTHS 1			/* all current hardware just has one*/
#define NVISUALS 1

/* FIXME FIXME - do we need all the entry points? */
/* FIXME FIXME - init and bot may be same... */

typedef struct {
	int (*probe)();		/* probe for specific type */
	int (*attach)();	/* attach */
	void (*save)();		/* save state */
	void (*restore)();	/* restore state */
	void (*init)();		/* init */
	void (*adjust)();	/* adjust */
	void (*print)();	/* print */
	void (*interrupt)();	/* interrupt */
	void (*bot)();          /* beginning of time (console init)*/
} vga_adapter_functions;

/*
 * the reason a different color cell representation is used is
 * to reduce kernel memory usage.  This form is sufficient for
 * a 256 entry color map, and saves 8 bytes/cell, or 2k bytes/screen.
 */
struct vga_color_cell 
{
        unsigned char dirty_cell;
        unsigned char red;	      /* only need 8 bits */
        unsigned char green;
        unsigned char blue;
};



struct vga_info {
	ws_screen_descriptor screen;
	ws_depth_descriptor depth[NDEPTHS];
	ws_visual_descriptor visual[NVISUALS];
	ws_screen_functions sf;
	ws_color_map_functions cmf;
	ws_cursor_functions cf;
	vga_adapter_functions af;
	unsigned int attribute;		/* char mode attribute */
	unsigned int unit;
	unsigned int board_id;
	unsigned char screen_on;
	unsigned char cursor_on;
	unsigned char dirty_cursor;
	unsigned char dirty_colormap;
	short min_dirty;
	short max_dirty;
	short x_hot;
	short y_hot;
	ws_color_cell cursor_fg;
	ws_color_cell cursor_bg;
	u_int bits[256];  /* 1KB */
	struct vga_color_cell cells[256];  /* 1KB */
};

#endif /* !_VGAINFO_H_ */
