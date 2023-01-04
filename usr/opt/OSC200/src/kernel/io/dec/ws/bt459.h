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
 *	@(#)$RCSfile: bt459.h,v $ $Revision: 1.2.10.4 $ (DEC) $Date: 1993/11/17 23:13:16 $
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
 *			Copyright (c) 1989 by				*
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

#ifndef	_BT459_H_
#define	_BT459_H_

#include <io/common/devdriver.h> 

#ifdef	__alpha
struct bt459 {
    volatile u_int addr_low;
    volatile u_int addr_high;
    volatile u_int bt_reg;
    volatile u_int color_map;
};
#else	/* __alpha */
struct bt459 {
    int               addr_low;         /* +0x00 <~c1,~c0> */
    int               addr_high;        /* +0x04 <~c1, c0> */
    int               bt_reg;           /* +0x08 < c1,~c0> */
    int               color_map;        /* +0x0c < c1, c0> */
};
#endif	/* __alpha */


#define _DUPBYTE0(X) \
    (((((int)(X))<<16)&0xff0000)|((((int)(X))<<8)&0xff00)|(((int)(X))&0xff))
#define _DUPBYTE1(X) \
    (((((int)(X))<<8)&0xff0000)|(((int)(X))&0xff00)|((((int)(X))>>8)&0xff))
#define _DUPBYTE2(X) \
    ((((int)(X))&0xff0000)|((((int)(X))>>8)&0xff00)|((((int)(X))>>16)&0xff))


/*
 * Sometimes, the 2DA/3DA systems doesn't vsync on VDAC updates, so
 * carefully set the spl level in order to make each VDAC transaction
 * atomic.
 */
#if 1
#if	__alpha
#define IPLTTY(L)               {(L)=splhigh();}
#else
#define IPLTTY(L)               {(L)=getspl();if(whatspl(L)<SPLTTY)(L)=splhigh();}
#endif
#define SPLX(L)                 splx(L)
#else
#define IPLTTY(L)
#define SPLX(L)
#endif

#define bt_addr_low(V,A)        { (V)->addr_low=_DUPBYTE0(A); wbflush(); }
#define bt_addr_high(V,A)       { (V)->addr_high=_DUPBYTE1(A); wbflush(); }
#define bt_addr_set(V,A)        { (V)->addr_high=_DUPBYTE1(A); WBFLUSH(); \
                                  bt_addr_low(V,A); }

#define bt_dup0_reg(V,I)        { (V)->bt_reg = _DUPBYTE0(I); WBFLUSH(); }
#define bt_dup1_reg(V,I)        { (V)->bt_reg = _DUPBYTE1(I); WBFLUSH(); }

#define bt_dup0_map(V,I)        { (V)->color_map=_DUPBYTE0(I);WBFLUSH(); }
#define bt_dup1_map(V,I)        { (V)->color_map=_DUPBYTE1(I);WBFLUSH(); }
#define bt_dup2_map(V,I)        { (V)->color_map=_DUPBYTE2(I);WBFLUSH(); }

#define bt_load_seq(V,A,S)      { register int s; IPLTTY(s);    \
                                  bt_addr_set(V,A);             \
                                  S;                            \
                                  SPLX(s); }

#define bt_intr_enabled(C,I)    (((I)->enable_interrupt) ? \
                                 ((*(I)->enable_interrupt)(C),1) : 0)
/*
 * the reason a different color cell representation is used is
 * to reduce kernel memory usage.  This form is sufficient for
 * a 256 entry color map, and saves 8 bytes/cell, or 2k bytes/screen.
 */
struct bt459_color_cell {
	unsigned char dirty_cell;
	unsigned char red;	/* only need 8 bits */
	unsigned char green;
	unsigned char blue;
};

#define BT459_MAXX              1279
#define BT459_MAXY              1023

/*
 * Note on modifications for SFB+:  The SFB+/TGA-type modules define a
 * rather different mechanism for accessing ramdacs and implement the
 * cursor with some opaque structures.  This design doesn't really fit
 * in with the existing 459/463 types so I've created a parallel
 * structure set (...info2, ...type2, etc.) to describe this new access
 * method.
 */

#define BT459_CX_TYPE           0
#define	BT459_PX_TYPE		1
#define	BT459_PXG_TYPE		2
#define BT459_HX_TYPE           3


#define	BT459_SFBP_TYPE2	0

#define	SFBP_SFBP_TYPE		0

struct bt459type {
    volatile struct bt459 *btaddr;      /* device phys address */
    short fb_xoffset;                   /* offset to video */
    short fb_yoffset;
    short min_dirty, max_dirty;         /* range of dirty entries */
    caddr_t reset;                      /* reset address/CR0 select */
    u_int mask;                         /* for cursor verification */
};

struct bt459info {
    volatile struct bt459 *btaddr;      /* device phys address */
    short fb_xoffset;                   /* offset to video */
    short fb_yoffset;
    short min_dirty, max_dirty;         /* range of dirty entries */
    caddr_t reset;                      /* reset address/CR0 select */
    u_int mask;                         /* for cursor verification */
    /***************************************************************
     * fields above this line MUST match struct bt459type exactly!
     ***************************************************************/
    u_int unit;                         /* device unit */
    char screen_on;                     /* whether screen is on */
    char on_off;                        /* whether cursor is on */
    char dirty_cursor;                  /* has cursor been reloaded? */
    char dirty_colormap;                /* has cmap been reloaded? */
    short x_hot;                        /* hot spot of current cursor */
    short y_hot;
    ws_color_cell cursor_fg;
    ws_color_cell cursor_bg;
    void (*enable_interrupt)();         /* enables one interrupt at V.R. */
    u_long bits[256];                   /* 1KB */
    struct bt459_color_cell cells[256]; /* 1KB */
};

#define	BT459_TYPE2_SETUP_HEAD_MASK	0x00000001
#define	BT459_TYPE2_SETUP_RW_MASK	(BT459_TYPE2_SETUP_WRITE|BT459_TYPE2_SETUP_READ)
#define	BT459_TYPE2_SETUP_WRITE		0x00000000
#define	BT459_TYPE2_SETUP_READ		0x00000002
#define	BT459_TYPE2_SETUP_C0_MASK	0x00000004
#define	BT459_TYPE2_SETUP_C1_MASK	0x00000008
#define	BT459_TYPE2_SETUP_C2_MASK	0x00000010

#define	BT459_TYPE2_DATA_WRITE_SHIFT	0
#define	BT459_TYPE2_DATA_READ0_SHIFT	8
#define	BT459_TYPE2_DATA_READ1_SHIFT	16

#define	BT459_TYPE2_ADDR_LOW		0
#define	BT459_TYPE2_ADDR_HIGH		(BT459_TYPE2_SETUP_C0_MASK)
#define	BT459_TYPE2_CMD_CURS		(BT459_TYPE2_SETUP_C1_MASK)
#define	BT459_TYPE2_CMD_CMAP		(BT459_TYPE2_SETUP_C0_MASK|BT459_TYPE2_SETUP_C1_MASK)

struct bt459type2 {
    volatile unsigned int *setup;	/* pointer to setup register */
    volatile unsigned int *data;	/* pointer to data register */
    unsigned int head_mask;		/* selector for multi-head */
    short fb_xoffset;                   /* offset to video */
    short fb_yoffset;
    short min_dirty, max_dirty;         /* range of dirty entries */
    caddr_t reset;                      /* reset address/CR0 select */
    u_int mask;                         /* for cursor verification */
};

struct bt459info2 {
    volatile unsigned int *setup;	/* pointer to setup register */
    volatile unsigned int *data;	/* pointer to data register */
    unsigned int head_mask;		/* selector for multi-head */
    short fb_xoffset;                   /* offset to video */
    short fb_yoffset;
    short min_dirty, max_dirty;         /* range of dirty entries */
    caddr_t reset;                      /* reset address/CR0 select */
    u_int mask;                         /* for cursor verification */
    /***************************************************************
     * fields above this line MUST match struct bt459type2 exactly!
     ***************************************************************/
    u_int unit;                         /* device unit */
    char screen_on;                     /* whether screen is on */
    char on_off;                        /* whether cursor is on */
    char dirty_cursor;                  /* has cursor been reloaded? */
    char dirty_colormap;                /* has cmap been reloaded? */
    short x_hot;                        /* hot spot of current cursor */
    short y_hot;
    ws_color_cell cursor_fg;
    ws_color_cell cursor_bg;
    void (*enable_interrupt)();         /* enables one interrupt at V.R. */
    u_long bits[256];                   /* 1KB */
    struct bt459_color_cell cells[256]; /* 1KB */
};

struct sfbp_curs_info {
    volatile unsigned int *xy_reg;
    volatile unsigned int *valid;
    short fb_xoffset;                   /* offset to video */
    short fb_yoffset;
    u_int unit;                         /* device unit */
    char on_off;                        /* whether cursor is on */
    char dirty_cursor;                  /* has cursor been reloaded? */
    char dirty_cursormap;                /* has cmap been reloaded? */
    short x_hot;                        /* hot spot of current cursor */
    short y_hot;
    short last_row;
    ws_color_cell cursor_fg;
    ws_color_cell cursor_bg;
    void (*enable_interrupt)();         /* enables one interrupt at V.R. */
    unsigned int bits[256];                   /* 1KB */
};

/*
#define ADDR_LOW_MASK		0x00ff
#define ADDR_HIGH_MASK		0xff00
*/

#define COLOR_MAP_BASE          0x0000

#define	OVERLAY_COLOR_BASE	0x0100
#define CURSOR_COLOR_1		0x0181
#define	CURSOR_COLOR_2		0x0182
#define	CURSOR_COLOR_3		0x0183
#define	ID_REG			0x0200
#define CMD_REG_0		0x0201
#define	CMD_REG_1		0x0202
#define CMD_REG_2		0x0203
#define PIXEL_READ_MASK		0x0204
#define PIXEL_BLINK_MASK	0x0206
#define OVERLAY_READ_MASK	0x0208
#define OVERLAY_BLINK_MASK	0x0209
#define	INTERLEAVE_REG		0x020a
#define	TEST_REG		0x020b
#define	RED_SIGNATURE		0x020c
#define GREEN_SIGNATURE		0x020d
#define BLUE_SIGNATURE		0x020e
#define CURSOR_CMD_REG		0x0300
#define CURSOR_X_LOW		0x0301
#define CURSOR_X_HIGH		0x0302
#define CURSOR_Y_LOW		0x0303
#define CURSOR_Y_HIGH		0x0304
#define WINDOW_X_LOW		0x0305
#define WINDOW_X_HIGH		0x0306
#define WINDOW_Y_LOW		0x0307
#define WINDOW_Y_HIGH		0x0308
#define WINDOW_WIDTH_LOW	0x0309
#define WINDOW_WIDTH_HIGH	0x030a
#define	WINDOW_HEIGHT_LOW	0x030b
#define WINDOW_HEIGHT_HIGH 	0x030c
#define CURSOR_RAM_BASE		0x0400

caddr_t bt_init_closure();

int  bt_load_formatted_cursor();
int  bt_recolor_cursor();
int  bt_set_cursor_position();
int  bt_load_cursor();
int  bt_load_color_map_entry();
int  bt_get_unit();

int  bt_init_color_map();
int  bt_cursor_on_off();
int  bt_video_on();
int  bt_video_off();
void bt_clean_colormap();

void bt_wb_nonmerging();
void bt_wb_merging();


extern caddr_t  bt459_2_init_closure();

int  bt459_2_load_formatted_cursor();
int  bt459_2_recolor_cursor();
int  bt459_2_load_cursor();
int  bt459_2_load_color_map_entry();

int  bt459_2_init_color_map();
int  bt459_2_cursor_on_off();
void bt459_2_clean_colormap();

extern struct bt459info bt459_softc[];
extern struct bt459type bt459_type[];
extern int              nbt_types;
extern int              nbt_softc;

extern struct bt459info2 bt459_softc2[];
extern struct bt459type2 bt459_type2[];
extern int		 nbt2_types;
extern int		 nbt2_softc;

extern struct sfbp_curs_info sfbp_curs_softc[];
extern struct sfbp_curs_info sfbp_curs_type[];

#endif /* _BT459_H_*/

