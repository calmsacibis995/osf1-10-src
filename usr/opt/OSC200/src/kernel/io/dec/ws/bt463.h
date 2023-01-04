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
 *	@(#)$RCSfile: bt463.h,v $ $Revision: 1.2.12.2 $ (DEC) $Date: 1993/11/17 23:13:30 $
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

/*	Modification history:
 *
 *	Created based on bt459.h
 *		Andy Goodrich
 *		December 1990
 */

#ifndef bt463_DEFINED
#define bt463_DEFINED 1

/*
 * Description of interesting 463 features
 */
#define	BT463_WINDOW_TAG_COUNT		16
#define	BT463_CMAP_ENTRY_COUNT		528


/*
 *	Layout of hardware when viewed from Turbo Bus:
 */
struct bt463 {
#ifdef __alpha
/* adjustments for SPARSE address space access */
    volatile unsigned int addr_low;
    volatile unsigned int addr_high;
    volatile unsigned int bt_reg;
    volatile unsigned int color_map;
#else /* __alpha */
    volatile char addr_low;
    char pad1[3];
    volatile char addr_high;
    char pad2[3];
    volatile char bt_reg;
    char pad3[3];
    volatile char color_map;
    char pad4[3];
#endif /* __alpha */
};

/*
 * The reason a different color cell representation is used is
 * to reduce kernel memory usage.  This form is sufficient for
 * an 256 entry color map, and saves 8 bytes/cell, or 2k bytes/screen.
 */
typedef struct bt463_color_cell {
	unsigned char dirty_cell;
	unsigned char red;	/* only need 8 bits */
	unsigned char green;
	unsigned char blue;
} Bt463_Color_Cell;

typedef struct bt463_wid_cell {
	unsigned char low_byte;		/* Low order 8 bits of wid P0-P7. */
	unsigned char middle_byte;	/* Middle 8 bits of wid P8-P15. */
	unsigned char high_byte;	/* High order 8 bits of wid P0-P7. */
	unsigned char unused;
} Bt463_Wid_Cell;

/*
 *	Closure information for Brooktree 463 RAMDAC
 */

/*
 * Note on modifications for SFB+:  The SFB+/TGA-type modules define a
 * rather different mechanism for accessing ramdacs and implement the
 * cursor with some opaque structures.  This design doesn't really fit
 * in with the existing 459/463 types so I've created a parallel
 * structure set (...info2, ...type2, etc.) to describe this new access
 * method.
 */

#define	BT463_TCO_TYPE			0
#define	BT463_PV_TYPE			1

#define	BT463_SFBP_TYPE2		0

struct bt463info {
        volatile struct bt463  *btaddr;	/* hardware location in virt. mem. */
	char		type;		/* Module type of bt */
	char 		screen_on;	/* whether screen is on */
	char 		dirty_colormap;	/* has cmap been reloaded?*/
	char 		dirty_cursormap; /* has curs map been reloaded?*/
	short 		fb_xoffset;	/* offset to video */
	short 		fb_yoffset;	/* ... */
	int		unit;		/* owning module's unit number */
	void    (*enable_interrupt)();	/* enables one interrupt at V.R. */
	caddr_t		cursor_closure;	/* hack hook into cursor code */
	ws_color_cell    saved_entry;
	ws_color_cell    cursor_fg;	/* current cursor foreground color. */
	ws_color_cell    cursor_bg;	/* current cursor background color. */
	short 	  min_dirty;/* range of dirty entries needing updating. */
	short	  max_dirty;				/* ... */	
	Bt463_Color_Cell cells[BT463_CMAP_ENTRY_COUNT];
};

#define	BT463_TYPE2_SETUP_HEAD_MASK	0x00000001
#define	BT463_TYPE2_SETUP_RW_MASK	0x00000002
#define	BT463_TYPE2_SETUP_C0_MASK	0x00000004
#define	BT463_TYPE2_SETUP_C1_MASK	0x00000008
#define	BT463_TYPE2_SETUP_C2_MASK	0x00000010

#define	BT463_TYPE2_DATA_WRITE_SHIFT	0
#define	BT463_TYPE2_DATA_READ0_SHIFT	8
#define	BT463_TYPE2_DATA_READ1_SHIFT	16

struct bt463info2 {
	volatile unsigned int *setup;	/* ramdac setup register */
	volatile unsigned int *data;	/* ramdac data register */
	unsigned int	head_mask;
	char		type;		/* Module type of bt */
	char 		screen_on;	/* whether screen is on */
	char 		dirty_colormap;	/* has cmap been reloaded?*/
	char 		dirty_cursormap; /* has curs map been reloaded?*/
	short 		fb_xoffset;	/* offset to video */
	short 		fb_yoffset;	/* ... */
	int		unit;		/* owning module's unit number */
	void    (*enable_interrupt)();	/* enables one interrupt at V.R. */
	caddr_t		cursor_closure;	/* hack hook into cursor code */
	ws_color_cell    cursor_fg;	/* current cursor foreground color. */
	ws_color_cell    cursor_bg;	/* current cursor background color. */
	short 	  min_dirty;/* range of dirty entries needing updating. */
	short	  max_dirty;				/* ... */	
	Bt463_Color_Cell cells[BT463_CMAP_ENTRY_COUNT];
};

/*
 * Address registers
 */
#define CURSOR_COLOR0           0x0100
#define CURSOR_COLOR1           0x0101
#define BT463_ID_REG            0x0200
#define COMMAND_REG_0           0x0201
#define COMMAND_REG_1           0x0202
#define COMMAND_REG_2           0x0203

#define P0_P7_READ_MASK         0x0205
#define P8_P15_READ_MASK        0x0206
#define P16_P23_READ_MASK       0x0207
#define P24_P27_READ_MASK       0x0208

#define P0_P7_BLINK_MASK        0x0209
#define P8_P15_BLINK_MASK       0x020a
#define P16_P23_BLINK_MASK      0x020b
#define P24_P27_BLINK_MASK      0x020c

#define BT463_TEST_REG          0x020d

#define INPUT_SIGNATURE         0x020E
#define OUTPUT_SIGNATURE        0x020F

#define REVISION_REG            0x0220

#define	WINDOW_TYPE_TABLE	0x0300

/*
 *    Window Type Field Definitions:
 */
#define SET_WT_SHIFT( x ) ( ((x) & 0x1f) )
#define SET_WT_PLANES(x)  ( ((x) & 0x0f) <<  5 )
#define SET_WT_MODE(x)    ( ((x) & 0x07) <<  9 )
#define SET_WT_OVERLAY(x) ( ((x) & 0x01) << 12 )
#define SET_WT_OV_MASK(x) ( ((x) & 0x0f) << 13 )
#define SET_WT_LUT_ADDR(x)( ((x) & 0x3f) << 17 )
#define SET_WT_BYPASS(x)  ( ((x) & 0x01) << 23 )

#define WT_VALUE(shift, planes, mode, overlay, ov_mask, lut_addr, bypass) \
	(SET_WT_SHIFT(shift) | SET_WT_PLANES(planes) | SET_WT_MODE(mode) \
	| SET_WT_OVERLAY(overlay) | SET_WT_OV_MASK(ov_mask) | SET_WT_LUT_ADDR(lut_addr) \
	| SET_WT_BYPASS(bypass) )

#define GET_WT_SHIFT( x ) ( (x & 0x00001f) )
#define GET_WT_PLANES(x)  ( (x & 0x0001e0) >>  5 )
#define GET_WT_MODE(x)    ( (x & 0x000e00) >>  9 )
#define GET_WT_OVERLAY(x) ( (x & 0x001000) >> 12 )
#define GET_WT_OV_MASK(x) ( (x & 0x01e000) >> 13 )
#define GET_WT_LUT_ADDR(x)( (x & 0x7e0000) >> 17 )
#define GET_WT_BYPASS(x)  ( (x & 0x800000) >> 23 )

/*####*/

#define LUT_BASE_24 0x100  /* Base offset into lut of true color colormap. */
#define LUT_BASE_8  0x000 /* Base offset into lut of eight bit colormap. */

#define ADDR_LOW_MASK 0x00ff /* Mask to obtain bits for 'addr_low' register. */
#define ADDR_HIGH_MASK	0xff00 /* Mask to obtain bits for 'addr_high' reg. */

int bt463_init_color_map();
int bt463_load_color_map_entry();
int bt463_video_on();
int bt463_video_off();
void bt463_clean_colormap();
int bt463_recolor_cursor();
void bt463_load_wid();

extern struct bt463info bt463_softc[];
extern struct bt463info bt463_type[];

#define	BT463_TYPE2_SETUP_HEAD_MASK	0x00000001
#define	BT463_TYPE2_SETUP_RW_MASK	0x00000002
#define	BT463_TYPE2_SETUP_C0_MASK	0x00000004
#define	BT463_TYPE2_SETUP_C1_MASK	0x00000008
#define	BT463_TYPE2_SETUP_C2_MASK	0x00000010

#define	BT463_TYPE2_DATA_WRITE_SHIFT	0
#define	BT463_TYPE2_DATA_READ0_SHIFT	8
#define	BT463_TYPE2_DATA_READ1_SHIFT	16

#define	BT463_TYPE2_ADDR_LOW		0
#define	BT463_TYPE2_ADDR_HIGH		(BT463_TYPE2_SETUP_C0_MASK)
#define	BT463_TYPE2_CMD_CURS		(BT463_TYPE2_SETUP_C1_MASK)
#define	BT463_TYPE2_CMD_CMAP		(BT463_TYPE2_SETUP_C0_MASK|BT463_TYPE2_SETUP_C1_MASK)

int bt463_2_init_color_map();
int bt463_2_load_color_map_entry();
int bt463_2_video_on();
int bt463_2_video_off();
void bt463_2_clean_colormap();
int bt463_2_recolor_cursor();
void bt463_2_load_wid();

extern struct bt463info2 bt463_softc2[];
extern struct bt463info2 bt463_type2[];

#endif /* !bt463_DEFINED */

