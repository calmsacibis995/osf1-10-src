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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: ims_g332.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/08/31 12:24:09 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
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
 *   of its software onequipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * support for INMOS G332
 * written by  Joel Gringorten
 */
#include <sys/types.h>
#include <sys/workstation.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/dec/ws/ims_g332.h>
#include <io/dec/ws/pmagdv.h>


/*
 * Position the cursor to a particular location.  Note that most of the
 * work is done by the main driver, which does bounds checking on hot spots,
 * and keeps track of the cursor location.
 */


/* ARGSUSED */
ims_g332_set_cursor_position(closure, sp, x, y)
	caddr_t closure;
	ws_screen_descriptor *sp;
	register int x, y;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	register int xt, yt;
	int s = spltty();
	xt = x + bti->fb_xoffset - bti->x_hot;
	yt = y + bti->fb_yoffset - bti->y_hot;
	write_g332(CURSOR_POSITION,  (xt << 12 | yt & 0xfff));
	splx(s);
	return(0);
}

int ims_g332_load_cursor(closure, screen, cursor, sync)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_cursor_data *cursor;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	ims_g332_reformat_cursor(bti, cursor);
	bti->x_hot = cursor->x_hot;
	bti->y_hot = cursor->y_hot;
	ims_g332_set_cursor_position(closure, screen, screen->x, screen->y);
	bti->dirty_cursor = 1;			/* cursor needs reloading */
	if (bti->enable_interrupt) 
		(*bti->enable_interrupt)(closure);
	else 
		ims_g332_load_formatted_cursor(bti);
	return(0);
}

static unsigned char flip[256] = {
0x0,  0x80,  0x40,  0xc0,  0x20,  0xa0,  0x60,  0xe0,
0x10,  0x90,  0x50,  0xd0,  0x30,  0xb0,  0x70,  0xf0,
0x8,  0x88,  0x48,  0xc8,  0x28,  0xa8,  0x68,  0xe8,
0x18,  0x98,  0x58,  0xd8,  0x38,  0xb8,  0x78,  0xf8,
0x4,  0x84,  0x44,  0xc4,  0x24,  0xa4,  0x64,  0xe4,
0x14,  0x94,  0x54,  0xd4,  0x34,  0xb4,  0x74,  0xf4,
0xc,  0x8c,  0x4c,  0xcc,  0x2c,  0xac,  0x6c,  0xec,
0x1c,  0x9c,  0x5c,  0xdc,  0x3c,  0xbc,  0x7c,  0xfc,
0x2,  0x82,  0x42,  0xc2,  0x22,  0xa2,  0x62,  0xe2,
0x12,  0x92,  0x52,  0xd2,  0x32,  0xb2,  0x72,  0xf2,
0xa,  0x8a,  0x4a,  0xca,  0x2a,  0xaa,  0x6a,  0xea,
0x1a,  0x9a,  0x5a,  0xda,  0x3a,  0xba,  0x7a,  0xfa,
0x6,  0x86,  0x46,  0xc6,  0x26,  0xa6,  0x66,  0xe6,
0x16,  0x96,  0x56,  0xd6,  0x36,  0xb6,  0x76,  0xf6,
0xe,  0x8e,  0x4e,  0xce,  0x2e,  0xae,  0x6e,  0xee,
0x1e,  0x9e,  0x5e,  0xde,  0x3e,  0xbe,  0x7e,  0xfe,
0x1,  0x81,  0x41,  0xc1,  0x21,  0xa1,  0x61,  0xe1,
0x11,  0x91,  0x51,  0xd1,  0x31,  0xb1,  0x71,  0xf1,
0x9,  0x89,  0x49,  0xc9,  0x29,  0xa9,  0x69,  0xe9,
0x19,  0x99,  0x59,  0xd9,  0x39,  0xb9,  0x79,  0xf9,
0x5,  0x85,  0x45,  0xc5,  0x25,  0xa5,  0x65,  0xe5,
0x15,  0x95,  0x55,  0xd5,  0x35,  0xb5,  0x75,  0xf5,
0xd,  0x8d,  0x4d,  0xcd,  0x2d,  0xad,  0x6d,  0xed,
0x1d,  0x9d,  0x5d,  0xdd,  0x3d,  0xbd,  0x7d,  0xfd,
0x3,  0x83,  0x43,  0xc3,  0x23,  0xa3,  0x63,  0xe3,
0x13,  0x93,  0x53,  0xd3,  0x33,  0xb3,  0x73,  0xf3,
0xb,  0x8b,  0x4b,  0xcb,  0x2b,  0xab,  0x6b,  0xeb,
0x1b,  0x9b,  0x5b,  0xdb,  0x3b,  0xbb,  0x7b,  0xfb,
0x7,  0x87,  0x47,  0xc7,  0x27,  0xa7,  0x67,  0xe7,
0x17,  0x97,  0x57,  0xd7,  0x37,  0xb7,  0x77,  0xf7,
0xf,  0x8f,  0x4f,  0xcf,  0x2f,  0xaf,  0x6f,  0xef,
0x1f,  0x9f,  0x5f,  0xdf,  0x3f,  0xbf,  0x7f,  0xff,
};

static unsigned char ims_g332_lookup_table[256] = {
 0x0,0x80,0x20,0xa0, 0x8,0x88,0x28,0xa8,
 0x2,0x82,0x22,0xa2, 0xa,0x8a,0x2a,0xaa,
0x40,0xc0,0x60,0xe0,0x48,0xc8,0x68,0xe8,
0x42,0xc2,0x62,0xe2,0x4a,0xca,0x6a,0xea,
0x10,0x90,0x30,0xb0,0x18,0x98,0x38,0xb8,
0x12,0x92,0x32,0xb2,0x1a,0x9a,0x3a,0xba,
0x50,0xd0,0x70,0xf0,0x58,0xd8,0x78,0xf8,
0x52,0xd2,0x72,0xf2,0x5a,0xda,0x7a,0xfa,
 0x4,0x84,0x24,0xa4, 0xc,0x8c,0x2c,0xac,
 0x6,0x86,0x26,0xa6, 0xe,0x8e,0x2e,0xae,
0x44,0xc4,0x64,0xe4,0x4c,0xcc,0x6c,0xec,
0x46,0xc6,0x66,0xe6,0x4e,0xce,0x6e,0xee,
0x14,0x94,0x34,0xb4,0x1c,0x9c,0x3c,0xbc,
0x16,0x96,0x36,0xb6,0x1e,0x9e,0x3e,0xbe,
0x54,0xd4,0x74,0xf4,0x5c,0xdc,0x7c,0xfc,
0x56,0xd6,0x76,0xf6,0x5e,0xde,0x7e,0xfe,
 0x1,0x81,0x21,0xa1, 0x9,0x89,0x29,0xa9,
 0x3,0x83,0x23,0xa3, 0xb,0x8b,0x2b,0xab,
0x41,0xc1,0x61,0xe1,0x49,0xc9,0x69,0xe9,
0x43,0xc3,0x63,0xe3,0x4b,0xcb,0x6b,0xeb,
0x11,0x91,0x31,0xb1,0x19,0x99,0x39,0xb9,
0x13,0x93,0x33,0xb3,0x1b,0x9b,0x3b,0xbb,
0x51,0xd1,0x71,0xf1,0x59,0xd9,0x79,0xf9,
0x53,0xd3,0x73,0xf3,0x5b,0xdb,0x7b,0xfb,
 0x5,0x85,0x25,0xa5, 0xd,0x8d,0x2d,0xad,
 0x7,0x87,0x27,0xa7, 0xf,0x8f,0x2f,0xaf,
0x45,0xc5,0x65,0xe5,0x4d,0xcd,0x6d,0xed,
0x47,0xc7,0x67,0xe7,0x4f,0xcf,0x6f,0xef,
0x15,0x95,0x35,0xb5,0x1d,0x9d,0x3d,0xbd,
0x17,0x97,0x37,0xb7,0x1f,0x9f,0x3f,0xbf,
0x55,0xd5,0x75,0xf5,0x5d,0xdd,0x7d,0xfd,
0x57,0xd7,0x77,0xf7,0x5f,0xdf,0x7f,0xff,
};

static ims_g332_reformat_cursor(bti, cursor)
	register ws_cursor_data *cursor;
	register struct ims_g332info *bti;
{
	register unsigned int cw, mw;
	register int i, j;
	register int nwords, shifts;
	register unsigned int mask, emask, omask;
	/* yuk, but C doesn't have good enough initialization */
	unsigned char *cbp = (unsigned char *) bti->bits;
	bzero(cbp, 1024);
	nwords = cursor->height;
	mask = 0xffffffff;
	if(cursor->width > 32) {
		nwords *= 2;
		shifts = 32 - (cursor->width - 32);
		emask = 0xffffffff;
		omask = (emask << shifts) >> shifts;
	}
	else {
		shifts = 32 - cursor->width;
		emask = omask = (mask << shifts) >> shifts;
	}

	for (i = 0; i < nwords; i++) {
		mask = emask;
		if (i & 1) mask = omask;
		cw = cursor->cursor[i] & mask;
		mw = cursor->mask[i] & mask;
		cw = cw & mw;
		for (j = 0; j < 8; j++)	 {
		    *cbp++ = ims_g332_lookup_table[flip[((cw << 4) |
(mw & 0xf)) & 0xff]];
		    cw >>= 4;
		    mw >>= 4;
		}
		if (cursor->width <= 32) cbp += 8;
	}
}
/* XXX should do load check based on chip version.
/*
 * given precomputed cursor, load it.
 */
ims_g332_load_formatted_cursor(bti)
	register struct ims_g332info *bti;
{
  	register int i;
	register short *cbp = (short *) bti->bits;
	register short cptr = CURSOR_STORE;
	int s = spltty();
	for (i = 0; i < 512; i++) {
	    write_g332 (cptr, (unsigned int)cbp[i]);
	    wbflush();
	    cptr += 1;
	}
	bti->dirty_cursor = 0;			/* no longer dirty */
	splx(s);
}


ims_g332_recolor_cursor (closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;

	bti->cursor_fg = *fg;
	bti->cursor_bg = *bg;
	ims_g332_restore_cursor_color(closure);
	return 0;
}


ims_g332_init_color_map(closure)
	caddr_t closure;
{
	register int s, i;
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	register int cptr = COLOR_PALETTE;    
	ims_g332_init(closure);
	ims_g332_init(closure); /* again? */
	ims_g332_init_cursor(closure);
	write_g332(cptr, 0);	/* black */
	cptr++;
    	for(i = 1; i <256; i++) {
		write_g332(cptr, 0xffffff);	/* white */
		cptr++;
	}
	bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue 
		= 0xffff;
	bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue 
		= 0x0000;


	ims_g332_restore_cursor_color(closure);

}

ims_g332_restore_cursor_color(closure)
	caddr_t closure;
{
    	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	register int s = spltty();
	register int cptr = CURSOR_PALETTE;    
	write_g332(cptr, 
	    (bti->cursor_bg.blue >> 8) << 16  | 
	    (bti->cursor_bg.green >> 8) << 8 |
	    (bti->cursor_bg.red >> 8) );
	cptr++;
	write_g332(cptr, 
	    (bti->cursor_fg.blue >> 8) << 16  | 
	    (bti->cursor_fg.green >> 8) << 8 |
	    (bti->cursor_fg.red >> 8) );
	cptr++;
	write_g332(cptr, 
	    (bti->cursor_fg.blue >> 8) << 16  | 
	    (bti->cursor_fg.green >> 8) << 8 |
	    (bti->cursor_fg.red >> 8) );
	splx(s);
}

ims_g332_init_cursor(closure)
	caddr_t closure;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
 
	/* nothing to do? */
}

ims_g332_init(closure)
	caddr_t closure;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;

	/* first pull the reset line on the chip for >500ns */
	PMAGDV_SSR_BASE &= ~(1<<6); /* set reset (active low) */
	DELAY(50);   	/* that ought to do it */
	PMAGDV_SSR_BASE |= (1<<6); /* reset reset */
	DELAY(50);

	write_g332(BOOT_LOCATION,    (MULTIPLICATION_FACTOR & PLL_MULTIPLIER)
				 | PLL_CLOCK
				 | ALIGNMENT_32
	);
	
	
	bti->ctl_a = (		DISABLE_VTG
			     | NON_INTERLACE 
			     | EIA_FORMAT
			     | MASTER
		             | TESSELATED_SYNC
			     | COMPOSITE_SYNC
			     | COMPOSITE_VIDEO
			     | NO_BLANK_PEDESTAL
			     | CBLANK_IS_OUTPUT
			     | DELAYED_CBLANK
			     | NO_ACTION     
			     | BLANKING_ENABLED
			     | (VRAM_INCREMENT & ( 0<<12) )
			     | DMA_ENABLED
			     | (SYNC_DELAY & ( 0<<15) )
			     | NON_INTERLEAVED
			     | (DELAYED_SAMPLING & ( 0<<19) )
			     | (BITS_PER_PIXEL & (3<<20))
			     | CURSOR_DISABLE
	);
	write_g332(CONTROL_A,  bti->ctl_a);

	write_g332(HALF_SYNC,	      SCREEN_UNITS( HORIZONTAL_SYNC_PULSES/2));
	write_g332(BACK_PORCH,	      SCREEN_UNITS( HORIZONTAL_BACK_PORCH));
	write_g332(DISPLAY,	      SCREEN_UNITS( DISPLAYED_PIXELS));
	write_g332(SHORTDISPLAY,      SCREEN_UNITS( (HORIZONTAL_SCAN_LINE/2) - 
						    (HORIZONTAL_SYNC_PULSES + 
						     HORIZONTAL_BACK_PORCH +
						     HORIZONTAL_FRONT_PORCH )
						));
	write_g332(BROADPULSE,	      SCREEN_UNITS( (HORIZONTAL_SCAN_LINE/2) -
						    HORIZONTAL_FRONT_PORCH));
	write_g332(LINETIME,	      SCREEN_UNITS(   HORIZONTAL_SCAN_LINE));
	write_g332(G332_VSYNC,	      HALF_LINES(     VERTICAL_SYNC_PULSE));
	write_g332(VPREEQUALISE,      HALF_LINES(     VERTICAL_FRONT_PORCH));
	write_g332(VPOSTEQUALISE,     HALF_LINES(     VERTICAL_BACK_PORCH));
	write_g332(VBLANK,	      HALF_LINES(     BLANKED_LINES));
	write_g332(VDISPLAY,	      HALF_LINES(     DISPLAYED_LINES));

	write_g332(LINESTART, 16);
	write_g332(MEMINIT, /* 246 */ 10);
	write_g332(TRANSFERDELAY, 10);
	write_g332(MASK,0x00ffffff);  

	bitSet(bti->ctl_a, ENABLE_VTG);
	bitClear(bti->ctl_a, CURSOR_DISABLE);
	write_g332(CONTROL_A, bti->ctl_a);
}


/* returns 0 if succeeded, -1 if it couldn't (index too big) */
/* ARGSUSED */
int ims_g332_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;	/* not used; only single map in this device */
	register ws_color_cell *entry;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	register int index = entry->index;
	int s;
	if(index >= 256) 
		return -1;
	s = spltty();
	bti->cells[index].red   = entry->red   >> 8;
	bti->cells[index].green = entry->green >> 8;
	bti->cells[index].blue  = entry->blue  >> 8;
	bti->cells[index].dirty_cell = 1;
	if (index < bti->min_dirty) bti->min_dirty = index;
	if (index > bti->max_dirty) bti->max_dirty = index;
	bti->dirty_colormap = 1;
	splx(s);
	if (bti->enable_interrupt) 
		(*bti->enable_interrupt)(closure);
	else 
		ims_g332_clean_colormap(bti);
	return 0;
}

/* 
 * NOTE!  Maxine baseboard video has BLUE and RED guns reversed!
 */

void ims_g332_clean_colormap(closure)
	caddr_t closure;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	register int i;
	register struct ims_g332_color_cell *entry;
	register int cptr;
	int s = spltty();
	cptr = COLOR_PALETTE + bti->min_dirty;
	for (i = bti->min_dirty; i <= bti->max_dirty ; i++) {
		entry = &bti->cells[i];
		if (entry->dirty_cell) {
			entry->dirty_cell = 0;
			write_g332(cptr, (u_int)entry->blue << 16 |
			(u_int)entry->green << 8 | (u_int)entry->red);

			cptr++;
		}
	}
	bti->dirty_colormap = 0;
	bti->min_dirty = 256;
	bti->max_dirty = 0;
	splx(s);
}

int ims_g332_video_off(closure)
	caddr_t closure;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	int s = spltty();
	bitSet(bti->ctl_a, FORCE_BLANKING);
	splx(s);
        write_g332(CONTROL_A, bti->ctl_a);
	return(0);
}

int ims_g332_video_on(closure)
	caddr_t closure;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	int s = spltty();
	bitClear(bti->ctl_a, FORCE_BLANKING);
	splx(s);
        write_g332(CONTROL_A, bti->ctl_a);
	return(0);
}

ims_g332_cursor_on_off(closure, on_off)
	caddr_t closure;
	int on_off;
{
	register struct ims_g332info *bti = (struct ims_g332info *)closure;
	int s = spltty();
	if (on_off) { 
	        /* turn on cursor */
		bitClear(bti->ctl_a, CURSOR_DISABLE);
	} else {
		bitSet(bti->ctl_a, CURSOR_DISABLE);
	}
	write_g332(CONTROL_A, bti->ctl_a);
	splx(s);
}
		
/*  The high 8 bits of the register go to the "video Buffer reg."
 *  Then we write the low 16 bits to the vdac.  All 24 bits get written
 *  to the vdac this way.  Don't shoot me, I'm just the programmer.
 */


write_g332(reg, val)
	short reg;
	u_int val;
{
	u_short *vdac_reg = (u_short *)
	  (((unsigned int)reg << 4) + (unsigned int) PMAGDV_VDAC_WRITE_ADDR);
	u_short lowbits = val & 0xffff;	
	u_short highbits = (val & 0xff0000) >> 8; 
	PMAGDV_VIDEO_BUFFER_BASE = highbits;
					/* bits 16 thru 24 go here */
	*vdac_reg = lowbits;	/* load bottom 16 bits direct */
}

/*
 * Routines that need to read/mod/write the Control_A reg, would 
 * normally like to read it.  Unfortunately, reads to the vdeo buffer
 * reg are unreliable.  Therefore, we cache control_a instead in bti->ctl_a.
 * Ie, this routine remains unused.
 */

unsigned int read_g332(reg)
	short reg;
{
	u_short *vdac_reg = (u_short *)
	  (((unsigned int)reg << 4) + (unsigned int) PMAGDV_VDAC_READ_ADDR);
	u_int low = *vdac_reg & 0x0000ffff;
	u_int high = PMAGDV_VIDEO_BUFFER_BASE & 0x0000ff00;
	u_int val = (high << 8 ) | low;
	return val;
}
