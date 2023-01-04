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

/*
 * support for INMOS G332
 * written by  Joel Gringorten
 */


#ifndef _IMS_G332_H_
#define _IMS_G332_H_

#define bitSet(v,d) ((v) |= (d))
#define bitClear(v,d) ((v) &= ~(d))

/*
 * the reason a different color cell representation is used is
 * to reduce kernel memory usage.  This form is sufficient for
 * an 256 entry color map, and saves 8 bytes/cell, or 2k bytes/screen.
 */
struct ims_g332_color_cell {
	unsigned char dirty_cell;
	unsigned char red;	/* only need 8 bits */
	unsigned char green;
	unsigned char blue;
};

struct ims_g332info {
	unsigned int ctl_a;			/* phys address of the vdac */
	char screen_on;			/* whether screen is on */
	char on_off;			/* whether cursor is on...*/
	char dirty_cursor;		/* has cursor been reloaded?*/
	char dirty_colormap;		/* has cmap been reloaded?*/
	short fb_xoffset;		/* offset to video */
	short fb_yoffset;
	short x_hot;			/* hot spot of current cursor*/
	short y_hot;
	void (*enable_interrupt)();	/* enables one interrupt at V.R. */
	ws_color_cell saved_entry;
	ws_color_cell cursor_fg;
	ws_color_cell cursor_bg;
	short min_dirty, max_dirty;	/* range of dirty entries */
	unsigned long bits[256];	/* only put zero items after here*/
	struct ims_g332_color_cell cells[256];
};


/* registers */
#define BOOT_LOCATION	0x000
#define DATA_PATH	0x021 /* 0x021-0x03e */
#define MASK		0x040
#define CONTROL_A	0x060
#define CONTROL_B	0x070
#define TOP_OF_SCREEN	0x080
#define CURSOR_PALETTE	0x0a1 /* 0x0a1-0x0a3 */
#define CHECKSUM	0x0c0 /* 0x0c0-0x0c2 */
#define CURSOR_POSITION 0x0c7
#define COLOR_PALETTE	0x100 /* 0x100-0x1ff */
#define CURSOR_STORE	0x200 /* 0x200-0x3ff */



/* datapath registers */

#define HALF_SYNC	0x021
#define BACK_PORCH	0x022
#define DISPLAY		0x023
#define SHORTDISPLAY	0x024
#define BROADPULSE	0x025
#define G332_VSYNC	0x026
#define VPREEQUALISE	0x027
#define VPOSTEQUALISE	0x028
#define VBLANK		0x029
#define VDISPLAY	0x02a
#define LINETIME	0x02b
#define LINESTART       0x02c
#define MEMINIT		0x02d
#define TRANSFERDELAY	0x02e



/* control register  A (0x060) bits */

#define DISABLE_VTG		0x000000
#define ENABLE_VTG		0x000001
#define VTG			0x000001

#define NON_INTERLACE		0x000000
#define INTERLACE		0x000002
#define SCREEN_FORMAT		0x000002

#define EIA_FORMAT		0x000000
#define CCIR_FORMAT		0x000004
#define INTERLACE_STANDARD	0x000004

#define MASTER			0x000000
#define SLAVE			0x000008
#define OPERATION_MODE		0x000008

#define TESSELATED_SYNC		0x000000
#define PLAIN_SYNC		0x000010
#define FRAME_FLYBACK		0x000010

#define COMPOSITE_SYNC          0x000000
#define SEPARATE_SYNC           0x000020
#define DIGITAL_SYNC		0x000020

#define COMPOSITE_VIDEO         0x000000
#define VIDEO_ONLY              0x000040
#define ANALOGUE_VIDEO		0x000040

#define NO_BLANK_PEDESTAL       0x000000
#define BLANKING_PEDESTAL       0x000080
#define BLANK_LEVEL		0x000080

#define CBLANK_IS_OUTPUT        0x000000
#define CBLANK_IS_INPUT         0x000100
#define BLANK_IO		0x000100

#define DELAYED_CBLANK          0x000000
#define UNDELAYED_CBLANK        0x000200
#define BLANK_FUNCTION		0x000200

#define NO_ACTION               0x000000
#define SCREEN_BLANKING         0x000400
#define FORCE_BLANKING		0x000400

#define BLANKING_ENABLED        0x000000
#define BLANKING_DISABLED       0x000800
#define TURN_OFF_BLANKING	0x000800

#define VRAM_INCREMENT		0x003000

#define DMA_ENABLED		0x000000
#define DMA_DISABLED            0x004000
#define TURN_OFF_DMA		0x004000

#define DELAYED_SAMPLING        0x008000


#define SYNC_DELAY		0x070000

#define NON_INTERLEAVED         0x000000
#define INTERLEAVED             0x040000
#define PIXEL_INTERLEAVING	0x040000

#define BITS_PER_PIXEL		0x380000

#define CURSOR_ENABLE		0x000000
#define CURSOR_DISABLE		0x800000


#define INCREMENT_1		0x000000
#define INCREMENT_256		0x001000
#define INCREMENT_512		0x002000
#define INCREMENT_1024		0x003000


/* boot location bits */

#define PLL_MULTIPLIER		0x00001f
#define CLOCK_SOURCE		0x000020
#define MICRO_PORT_ALIGMENT	0x000040

#define ALIGNMENT_32		0x000000
#define ALIGNMENT_64		0x000040

#define EXTERNAL_X1		0x000000
#define PLL_CLOCK		0x000020

/* vesa monitor defs? -- should be moved to seperate file.  */

#define PIXELS_PER_SCREEN_UNIT(d)	((d)/4)
#define HALF_LINES(d)			((d)*2)
#define SCREEN_UNITS(d)			((d)/4)


#define MULTIPLICATION_FACTOR 12	/* 6.25MHZ * 12 = 75Mhz */

/*                           	pixels		             */
#define HORIZONTAL_SYNC_PULSES  132   
#define DISPLAYED_PIXELS 	1024
#define HORIZONTAL_SCAN_LINE 	1304
#define HORIZONTAL_BACK_PORCH 	132
#define HORIZONTAL_FRONT_PORCH  /* 20 */ 16

/*                            	lines		      	     */
#define VERTICAL_SYNC_PULSE	6
#define VERTICAL_FRONT_PORCH	1
#define VERTICAL_BACK_PORCH	1
#define VERTICAL_LINES		797
#define DISPLAYED_LINES		768
#define BLANKED_LINES		21



int ims_g332_load_cursor();
int ims_g332_init_color_map();
int ims_g332_load_color_map_entry();
int ims_g332_load_cursor();
int ims_g332_recolor_cursor();
int ims_g332_set_cursor_position();
int ims_g332_cursor_on_off();
int ims_g332_load_formatted_cursor();
int ims_g332_video_on();
int ims_g332_video_off();
void ims_g332_clean_colormap();

extern struct ims_g332info ims_g332_softc[];

#endif /*_IMS_G332_H_*/

