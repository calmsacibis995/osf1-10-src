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
static char	*sccsid = "@(#)$RCSfile: bt459.c,v $ $Revision: 1.2.10.4 $ (DEC) $Date: 1993/11/17 23:13:11 $";
#endif 
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

/*
 *  Modification History
 *
 *	15-May-91 -- Joel gringorten
 *
 * 		Provide spl synchronization for IO
 *
 * support for Brooktree 459.
 * written by J. Gettys, from cfb original
 */

#include <data/ws_data.c>
#include <io/dec/ws/bt459.h>

static unsigned int bt_wbflush = 0;     /* default: non-merging writebuffer */

static void bt_load_reg();
static void bt_reformat_cursor();
void bt_restore_cursor_color();
void bt459_2_restore_cursor_color();

extern ws_screens screens[];

/*
 * Position the cursor to a particular location.  Note that most of the
 * work is done by the main driver, which does bounds checking on hot spots,
 * and keeps track of the cursor location.
 */

/* ARGSUSED */
bt_set_cursor_position(closure, sp, x, y)
    caddr_t closure;
    ws_screen_descriptor *sp;
    register int x, y;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;
    register int xt, yt;

    xt = x + bti->fb_xoffset - bti->x_hot;
    yt = y + bti->fb_yoffset - bti->y_hot;

    bt_load_seq(btp, CURSOR_X_LOW,
                bt_dup0_reg(btp, xt);
                bt_dup1_reg(btp, xt);
                bt_dup0_reg(btp, yt);
                bt_dup1_reg(btp, yt);
                );
    return 0;
}


bt_load_cursor(closure, screen, cursor, sync)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_cursor_data *cursor;
    int sync;
{
    register struct bt459info *bti = (struct bt459info *)closure;

    bti->x_hot = cursor->x_hot;
    bti->y_hot = cursor->y_hot;
    bt_set_cursor_position(closure, screen, screen->x, screen->y);

    bt_reformat_cursor( (unsigned char *) bti->bits, cursor);

    bti->dirty_cursor = 1;

    if (sync) {
        /* if vblank synchronization important...*/
        if (bt_intr_enabled(closure, bti)) /* XXX */
            return 0;
    }
    /* can't enable load at vblank or don't care, then just do it */
    return (bt_load_formatted_cursor(bti));
}

static unsigned char bt_lookup_table[256] = {
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

static void bt_reformat_cursor(bits, cursor)
	register ws_cursor_data *cursor;
	register unsigned char *bits;
{
	register unsigned int cw, mw;
	register int i, j;
	register int nwords, shifts;
	register unsigned int mask, emask, omask;
	/* yuk, but C doesn't have good enough initialization */
	unsigned char *cbp = bits;
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
		for (j = 0; j < 8; j++)	 {
		    *cbp++ = bt_lookup_table[((cw << 4) | (mw & 0xf)) & 0xff];
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
bt_load_formatted_cursor(bti)
    register struct bt459info *bti;
{
    register volatile struct bt459 *btp = bti->btaddr;
    register char *cbp = (char *) bti->bits;
    register int i, mask;
    int      bad_ram, counter = 0;

    bad_ram = EINVAL;                   /* assume cursor is bad until loaded */
    mask = bti->mask;

    while (bad_ram && (counter++ < 25)) {
        /*
         * write cursor data to the chip - disable it first to reduce
         * flickering while loading, then re-enable it, if necessary.
         * nb: this precludes having to use vsync for cursor loading.
         */
        bt_load_seq(btp, CURSOR_CMD_REG,
                    btp->bt_reg = 0x0;
                    bt_addr_set(btp, CURSOR_RAM_BASE);
                    for (i = 0; i < 1024; i++)
                        bt_dup0_reg(btp, cbp[i]);
                    if (bti->screen_on && bti->on_off) {
                        bt_addr_set(btp, CURSOR_CMD_REG);
                        btp->bt_reg = 0xc0c0c0;
                    }
                    );
        /* set bad_ram to 0 now that new data is loaded */
        bad_ram = 0;

#ifdef notdef
        /* XXX should do load check based on chip version. */
        /* read and verify each byte of cursor map */
        bt_load_seq(btp, CURSOR_RAM_BASE,
                    for (i = 0; i < 1024; i++) {
                        register int data = btp->bt_reg & mask;
                        if (data != (_DUPBYTE0(cbp[i]) & mask))
                        {
                            bad_ram = EINVAL;
                            break;
                        }
                    }
                    );
#endif
    }
    bti->dirty_cursor = 0;              /* no longer dirty */

    return bad_ram;
}


bt_recolor_cursor (closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct bt459info *bti 	= (struct bt459info *)closure;

	bti->cursor_fg = *fg;
	bti->cursor_bg = *bg;
	bt_restore_cursor_color(closure);
	return 0;
}


bt_init_color_map(closure)
	caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;
    register int i;

    bt_addr_set(btp, COLOR_MAP_BASE);
    btp->color_map = 0;         WBFLUSH();
    btp->color_map = 0;         WBFLUSH();
    btp->color_map = 0;         WBFLUSH();
    btp->color_map = 0xffffff;  WBFLUSH();
    btp->color_map = 0xffffff;  WBFLUSH();
    btp->color_map = 0xffffff;  WBFLUSH();

    for (i = 2; i < 256; i++) {
        if (bti->reset) {
            btp->color_map = 0xffffff;  WBFLUSH();
            btp->color_map = 0xffffff;  WBFLUSH();
            btp->color_map = 0xffffff;  WBFLUSH();
        } else {
            register int gray = _DUPBYTE0(i);
            /*
             * load with color ramp for ucode
             */
            btp->color_map = gray;      WBFLUSH();
            btp->color_map = gray;      WBFLUSH();
            btp->color_map = gray;      WBFLUSH();
        }
    }

    bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue
        = 0xffffff;
    bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue
        = 0x000000;
    bt_restore_cursor_color(closure);

    return 0;
}

void bt_restore_cursor_color(closure)
    caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;

    bt_load_seq(btp, CURSOR_COLOR_2,
                bt_dup0_reg(btp, bti->cursor_bg.red >> 8);
                bt_dup0_reg(btp, bti->cursor_bg.green >> 8);
                bt_dup0_reg(btp, bti->cursor_bg.blue >> 8);
                bt_dup0_reg(btp, bti->cursor_fg.red >> 8);
                bt_dup0_reg(btp, bti->cursor_fg.green >> 8);
                bt_dup0_reg(btp, bti->cursor_fg.blue >> 8);
                );
}

bt_init_cursor(closure)
	caddr_t closure;
{
	register struct bt459info *bti 	= (struct bt459info *)closure;
	register volatile struct bt459 *btp 	= bti->btaddr;    
	register int i;

        bt_addr_set(btp, CURSOR_RAM_BASE);

	for (i = 0; i < 1024; i++) {
		btp->bt_reg = 0x0;		/* clear cursor on chip */
                WBFLUSH();
	}
        bt_recolor_cursor(closure, 0, 0x0, 0x0);

        return 0;
}

static unsigned char bt_CR1[] = {
    0x00,                            /* cmd reg 1 */
    0xc2,                            /* cmd reg 2 - X-windows cursor */
    0xff,                            /* pix rd msk */
    0x00,                            /* reserved */
    0x00,                            /* pix blink msk */
    0x00,                            /* reserved */
    0x00,                            /* ovrly rd msk */
    0x00,                            /* ovrly blink msk */
    0x00,                            /* interleave */
    0x00                             /* test */
    };
#define BT_CR1_SIZE     (sizeof(bt_CR1)/sizeof(bt_CR1[0]))

caddr_t bt_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp;
    struct bt459type *btt;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    /* see if we've already init'd the closure for this vdac already */
    addr = address + (vm_offset_t)bt459_type[type].btaddr;
    for (i = 0; i < nbt_softc; i++) {
        if (addr == (caddr_t)bti[i].btaddr) {
            bti[i].unit = unit;
            return (caddr_t)(&bti[i]);
        }
        else if (bti[i].btaddr == NULL) {
            nextunit = i;
            break;
        }
    }

    /* setup another bt459info struct, if possible */
    if (i >= nbt_softc) 
       return (caddr_t)0; /* No room in softc array! */ 

    bti += nextunit;
    nextunit += 1;
    btt = (struct bt459type *)bti;

    /* set to initial values */
    *btt = bt459_type[type];
    bti->screen_on = bti->on_off = 1;
    bti->unit = unit;

    /* update relative offsets to physical addresses */
    bti->btaddr = btp = (struct bt459 *)addr;
    if (bti->reset)
        bti->reset += (long) address;
    /*
     * should be called while still in console mode only!  (eg, no mouse
     * interrupts please)
     */
    if (bti->mask == 0) {
        bt_addr_set(btp, ID_REG);
        i = btp->bt_reg;
        /*
         * see how many valid vdacs there are out there.  the range of responses
         * are: 0x____4a (cfb/2da), 0x4a4a4a(hi/mid-3da), 0x4a____(lo-3da).
         * should check for final validity, but wouldn't know what to do anyway
         * if it didn't check out properly... all this can go away if/when the
         * bt459 gets fixed.
         */
        if ((i & 0xff0000) == 0x4a0000)
            bti->mask |= 0xff0000;
        if ((i & 0xff00) == 0x4a00)
            bti->mask |= 0xff00;
        if ((i & 0xff) == 0x4a)
            bti->mask |= 0xff;
    }

    /*
     * if the reset field is set, then...
     *
     * after writing CR0, must reset the vdac in order for the chip to
     * behave properly.  of course, there is no provision for a software
     * vdac reset by the Bt459 chip.  a write to hyperspace has been hacked
     * up for this purpose.  but, whatever gets written goes into addr_lo,
     * (3da) so reset addr for next command reg.
     */
    bt_addr_set(btp, CMD_REG_0);
    if (bti->reset) {
        btp->bt_reg = 0xc0c0c0;         WBFLUSH();
        *(int *)bti->reset = 0x0;       WBFLUSH();
    }
    else {
        btp->bt_reg = 0x404040;         WBFLUSH();
    }

    bt_addr_set(btp, CMD_REG_1);
    for (i = 0; i < BT_CR1_SIZE; i++)
        bt_dup0_reg(btp, bt_CR1[i]);

    bt_addr_set(btp, CURSOR_CMD_REG);
    bt_dup0_reg(btp, 0xc0c0c0);         /* enable plane 0+1 of cursor */

    for (i = 0; i < 12; i++) {
        btp->bt_reg = 0;                WBFLUSH();
    }
    WBFLUSH();

    return (caddr_t) bti;
}

/* returns 0 if succeeded, -1 if it couldn't (index too big) */
/* ARGSUSED */
int bt_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;		/* not used; only single map in this device */
	register ws_color_cell *entry;
{
	register struct bt459info *bti 		= (struct bt459info *)closure;
	register volatile struct bt459 *btp 	= bti->btaddr;
	register int index = entry->index;
	int s;
	
	if(index >= 256 || index <0) 
		return -1;
        IPLTTY(s);
	bti->cells[index].red   = entry->red   >> 8;
	bti->cells[index].green = entry->green >> 8;
	bti->cells[index].blue  = entry->blue  >> 8;
	bti->cells[index].dirty_cell = 1;
	if (index < bti->min_dirty) bti->min_dirty = index;
	if (index > bti->max_dirty) bti->max_dirty = index;
        /*
         * this just enables a vblank intr to load dirty color cells.
         * the caller must arrange for the cells to be loaded "manually"
         * if vblank intr's aren't being used!
         */
        if (!bti->dirty_colormap) {
            bti->dirty_colormap = 1;
            (void) bt_intr_enabled(closure, bti);
        }
        SPLX(s);
        return 0;
}

void bt_clean_colormap(closure)
    caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;
    register struct bt459_color_cell *entry = &bti->cells[bti->min_dirty];
    register int i, s, lasti = -2;

    /*
     * if screen saver is on, then we can defer everything til video_on
     * since it's all saved away in bti->cells[] anyway...
     */
    IPLTTY(s);                          /* no intrs when we're using */
                                        /* autoinc mode of vdac please! */
    if (bti->screen_on == 0 || bti->dirty_colormap == 0) {
        SPLX(s);
        return;
    }

    bt_addr_high(btp, 0x0);             /* only addr_low changes */

    for (i = bti->min_dirty; i <= bti->max_dirty ; i++, entry++)
    {
        if (entry->dirty_cell)
        {
            if (i != (lasti + 1))
                bt_addr_low(btp, i);
            bt_dup0_map(btp, entry->red);
            bt_dup0_map(btp, entry->green);
            bt_dup0_map(btp, entry->blue);
            entry->dirty_cell = 0;
            lasti = i;
        }
    }
    bti->min_dirty = 256;
    bti->max_dirty = 0;
    bti->dirty_colormap = 0;

    SPLX(s);
}

int bt_video_off(closure)
    caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;
    register int s;

    if (bti->screen_on == 0)            /* not absolutely necessary, but */
        return 0;                       /* a little cleaner... */

    IPLTTY(s);
    bt_addr_set(btp, COLOR_MAP_BASE);
    btp->color_map = 0;         WBFLUSH();
    btp->color_map = 0;         WBFLUSH();
    btp->color_map = 0;         WBFLUSH();
    SPLX(s);

    bt_load_reg(btp,PIXEL_READ_MASK,0); /* force all pixels to cmap[0] */
    bt_load_reg(btp,CURSOR_CMD_REG,0);  /* turn off cursor */

    bti->cells[0].dirty_cell = 1;       /* request cmap[0] be restored */
    bti->min_dirty = 0;                 /* on video_on */
    bti->dirty_colormap = 1;            /* please... */
    bti->screen_on = 0;

    return 0;
}

int bt_video_on(closure)
    caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;
    register volatile struct bt459 *btp = bti->btaddr;
 /*   int sp, s;  */

    if (bti->screen_on)
        return 0;

    bti->screen_on = 1;

    if (bti->on_off)                            /* turn on cursor? */
        bt_load_reg(btp, CURSOR_CMD_REG, 0xc0c0c0);
    /* don't need to sync with vblank when just turning on video */
    bt_clean_colormap(bti);
    bt_load_reg(btp, PIXEL_READ_MASK, 0xffffff);

    return 0;
}

bt_cursor_on_off(closure, on_off)
	caddr_t closure;
	int on_off;
{
	register struct bt459info *bti		= (struct bt459info *)closure;
	register volatile struct bt459 *btp 	= bti->btaddr;    
	register int s = spltty();
        if (on_off)
		bt_load_reg(btp, CURSOR_CMD_REG, 0xc0c0c0); /* turn on cursor */
        else
		bt_load_reg(btp, CURSOR_CMD_REG, 0x00);	    /* turn off cursor */
	bti->on_off = on_off;
	splx(s);
	return(0);
}

void
bt_load_reg(btp, reg, val)
    register volatile struct bt459 *btp;
    u_int reg;
    u_int val;
{
    bt_load_seq(btp, reg,
                btp->bt_reg = val;
                WBFLUSH();
                );
}

bt_get_unit(closure)
    caddr_t closure;
{
    register struct bt459info *bti = (struct bt459info *)closure;

    return bti->unit;
}

void
bt_wb_nonmerging()
{
    bt_wbflush = 0;
}

void
bt_wb_merging()
{
    bt_wbflush = 1;
}

/*
 * Begin Type 2 routines for SFB+-like support
 */
#define BT459_WRITE(bti, addr, val) 			wbflush();	      \
    *(bti)->setup = (bti)->head_mask | (addr) | BT459_TYPE2_SETUP_WRITE;      \
    *(bti)->data = (val); wbflush();

bt459_2_load_cursor(closure, screen, cursor, sync)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_cursor_data *cursor;
    int sync;
{
    register struct bt459info2 *bti = (struct bt459info2 *)closure;
    ws_screens *wsp;

    bti->x_hot = cursor->x_hot;
    bti->y_hot = cursor->y_hot;
    wsp = &screens[screen->screen];
    (*wsp->cf->set_cursor_position)(closure, screen, screen->x, screen->y);

    bt_reformat_cursor( (unsigned char *) bti->bits, cursor);

    bti->dirty_cursor = 1;

    if ( sync && bti->enable_interrupt ) {
        /*
	 * if vblank synchronization important...
	 */
	(*bti->enable_interrupt)( closure );
	return 0;
    }
    /* can't enable load at vblank or don't care, then just do it */
    return (bt459_2_load_formatted_cursor(bti));
}

/*
 * given precomputed cursor, load it.
 */
bt459_2_load_formatted_cursor(bti)
    register struct bt459info2 *bti;
{
    register char *cbp = (char *) bti->bits;
    register int i, mask;
    int      counter = 0;
    int	     s;

    mask = bti->mask;

        /*
         * write cursor data to the chip - disable it first to reduce
         * flickering while loading, then re-enable it, if necessary.
         * nb: this precludes having to use vsync for cursor loading.
         */
	IPLTTY(s);

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_LOW
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = CURSOR_CMD_REG;			wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_HIGH
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = ( CURSOR_CMD_REG >> 8 );		wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_CMD_CURS
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = 0;					wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_LOW
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = CURSOR_RAM_BASE;			wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_HIGH
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = ( CURSOR_RAM_BASE >> 8 );		wbflush();
#if 1
	*bti->setup = bti->head_mask
		    | BT459_TYPE2_CMD_CURS
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
#endif
	for ( i = 0; i < 1024; i ++ ) {
#if 0
	    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, cbp[i]);
#else
	    *bti->data = cbp[i];			wbflush();
#endif
	}

	if (bti->screen_on && bti->on_off) {
	    *bti->setup = bti->head_mask
			| BT459_TYPE2_ADDR_LOW
			| BT459_TYPE2_SETUP_WRITE;	wbflush();
	    *bti->data = CURSOR_CMD_REG;		wbflush();

	    *bti->setup = bti->head_mask
			| BT459_TYPE2_ADDR_HIGH
			| BT459_TYPE2_SETUP_WRITE;	wbflush();
	    *bti->data = ( CURSOR_CMD_REG >> 8 );	wbflush();

	    *bti->setup = bti->head_mask
			| BT459_TYPE2_CMD_CURS
			| BT459_TYPE2_SETUP_WRITE;	wbflush();
	    *bti->data = 0xc0;				wbflush();
	}

	SPLX(s);

    bti->dirty_cursor = 0;              /* no longer dirty */

    return 0;
}


bt459_2_recolor_cursor (closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct bt459info2 *bti = (struct bt459info2 *) closure;

	bti->cursor_fg = *fg;
	bti->cursor_bg = *bg;
	bt459_2_restore_cursor_color(closure);
	return 0;
}



bt459_2_init_color_map(closure)
	caddr_t closure;
{
    register struct bt459info2 *bti = (struct bt459info2 *) closure;
    register int i;
    int s;

    IPLTTY(s);

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = COLOR_MAP_BASE;		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( COLOR_MAP_BASE >> 8 );	wbflush();
#if 0
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0xff);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0xff);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, 0xff);
#else
    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CMAP
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = 0;				wbflush();
    *bti->data = 0;				wbflush();
    *bti->data = 0;				wbflush();
    *bti->data = 0xff;				wbflush();
    *bti->data = 0xff;				wbflush();
    *bti->data = 0xff;				wbflush();
#endif
    for (i = 2; i < 256; i++) {
#if 0
	BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, i);
	BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, i);
	BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, i);
#else
	/*
	 * load with color ramp for ucode
	 */
	*bti->data = i;				wbflush();
	*bti->data = i;				wbflush();
	*bti->data = i;				wbflush();
#endif
    }

    SPLX(s);

    bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue
        = 0xffffff;
    bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue
        = 0x000000;

    bt459_2_restore_cursor_color(closure);

    return 0;
}


void bt459_2_restore_cursor_color(closure)
    caddr_t closure;
{
    register struct bt459info2 *bti = (struct bt459info2 *) closure;
    int s;

    IPLTTY(s);

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CURSOR_COLOR_2;		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( CURSOR_COLOR_2 >> 8 );	wbflush();
#if 0
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_bg.red);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_bg.green);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_bg.blue);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_fg.red);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_fg.green);
    BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bti->cursor_fg.blue);
#else
    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = bti->cursor_bg.red;		wbflush();
    *bti->data = bti->cursor_bg.green;		wbflush();
    *bti->data = bti->cursor_bg.blue;		wbflush();
    *bti->data = bti->cursor_fg.red;		wbflush();
    *bti->data = bti->cursor_fg.green;		wbflush();
    *bti->data = bti->cursor_fg.blue;		wbflush();
#endif
    SPLX(s);

    return;

}

bt459_2_init_cursor(closure)
	caddr_t closure;
{
    register struct bt459info2 *bti = (struct bt459info2 *) closure;
    register int i;

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CURSOR_RAM_BASE;		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( CURSOR_RAM_BASE >> 8 );	wbflush();
#if 1
    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
#endif
    for (i = 0; i < 1024; i++) {
#if 0
	BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, 0x00);
#else
	*bti->data = 0x00;			wbflush();
#endif
    }

    bt459_2_recolor_cursor(closure, 0, 0x0, 0x0);

    return 0;
}

static unsigned char bt459_2_CR1[] = {
    0x00,                            /* cmd reg 1 */
    0xc2,                            /* cmd reg 2 - X-windows cursor */
    0xff,                            /* pix rd msk */
    0x00,                            /* reserved */
    0x00,                            /* pix blink msk */
    0x00,                            /* reserved */
    0x00,                            /* ovrly rd msk */
    0x00,                            /* ovrly blink msk */
    0x00,                            /* interleave */
    0x00                             /* test */
    };
#define BT459_2_CR1_SIZE     (sizeof(bt459_2_CR1)/sizeof(bt459_2_CR1[0]))

caddr_t bt459_2_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    register struct bt459info2 *bti = (struct bt459info2 *) closure;
    struct bt459type2 *btt;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    /*
     * see if we've already init'd the closure for this vdac already
     */
    addr = address + (vm_offset_t) bt459_type2[type].setup;
    for (i = 0; i < nbt_softc; i++) {
        if (addr == (caddr_t) bti[i].setup) {
            bti[i].unit = unit;
            return (caddr_t)(&bti[i]);
        }
        else if (bti[i].setup == NULL) {
            nextunit = i;
            break;
        }
    }

    /* setup another bt459info struct, if possible */
    if (i >= nbt_softc) 
       return (caddr_t) 0; /* No room in softc array! */ 

    bti += nextunit;
    nextunit += 1;
    btt = (struct bt459type2 *) bti;

    /* set to initial values */
    *btt = bt459_type2[type];
    bti->screen_on = bti->on_off = 1;
    bti->unit = unit;

    /* update relative offsets to physical addresses */
    bti->setup = (unsigned int *) addr;
    bti->data = (unsigned int *) ( address
				 + (vm_offset_t) bt459_type2[type].data );

    /*
     * should be called while still in console mode only!  (eg, no mouse
     * interrupts please)
     */
    if (bti->mask == 0) {
	bti->mask = 0xff;
    }

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CMD_REG_0;			wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( CMD_REG_0 >> 8 );		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = 0x40;				wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CMD_REG_1;			wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( CMD_REG_1 >> 8 );		wbflush();
#if 1
    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
#endif
    for (i = 0; i < BT459_2_CR1_SIZE; i++) {
#if 0
	BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, bt459_2_CR1[i]);
#else
	*bti->data = bt459_2_CR1[i];		wbflush();
#endif
    }

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_LOW
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = CURSOR_CMD_REG;		wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = ( CURSOR_CMD_REG >> 8 );	wbflush();

    *bti->setup = bti->head_mask
		| BT459_TYPE2_CMD_CURS
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = 0xc0;				wbflush();

    for (i = 0; i < 12; i++) {
#if 0
	BT459_WRITE(bti, BT459_TYPE2_CMD_CURS, 0x0);
#else
        *bti->data = 0;				wbflush();
#endif
    }

    return (caddr_t) bti;
}

/* returns 0 if succeeded, -1 if it couldn't (index too big) */
/* ARGSUSED */
int bt459_2_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;		/* not used; only single map in this device */
	register ws_color_cell *entry;
{
	register struct bt459info2 *bti = (struct bt459info2 *) closure;
	register int index = entry->index;
	int s;
	
	if(index >= 256 || index <0) 
		return -1;

        IPLTTY(s);

	bti->cells[index].red   = entry->red   >> 8;
	bti->cells[index].green = entry->green >> 8;
	bti->cells[index].blue  = entry->blue  >> 8;
	bti->cells[index].dirty_cell = 1;
	if (index < bti->min_dirty) bti->min_dirty = index;
	if (index > bti->max_dirty) bti->max_dirty = index;

        /*
         * this just enables a vblank intr to load dirty color cells.
         * the caller must arrange for the cells to be loaded "manually"
         * if vblank intr's aren't being used!
         */
        if (!bti->dirty_colormap) {
            bti->dirty_colormap = 1;
            (void) bt_intr_enabled(closure, bti);
        }
        SPLX(s);
        return 0;
}


void bt459_2_clean_colormap(closure)
    caddr_t closure;
{
    register struct bt459info2 *bti = (struct bt459info2 *) closure;
    register struct bt459_color_cell *entry = &bti->cells[bti->min_dirty];
    register int i, s, lasti = -2;

    /*
     * if screen saver is on, then we can defer everything til video_on
     * since it's all saved away in bti->cells[] anyway...
     */
    IPLTTY(s);                          /* no intrs when we're using */
                                        /* autoinc mode of vdac please! */
    if (bti->screen_on == 0 || bti->dirty_colormap == 0) {
        SPLX(s);
        return;
    }

    *bti->setup = bti->head_mask
		| BT459_TYPE2_ADDR_HIGH
		| BT459_TYPE2_SETUP_WRITE;	wbflush();
    *bti->data = 0;				wbflush();

    for (i = bti->min_dirty; i <= bti->max_dirty ; i++, entry++)
    {
        if (entry->dirty_cell)
        {
            if (i != (lasti + 1)) {
		*bti->setup = bti->head_mask
			    | BT459_TYPE2_ADDR_LOW
			    | BT459_TYPE2_SETUP_WRITE;	wbflush();
		*bti->data = i;					wbflush();
#if 1
		*bti->setup = bti->head_mask
			    | BT459_TYPE2_CMD_CMAP
			    | BT459_TYPE2_SETUP_WRITE;	wbflush();
#endif
	    }
#if 0
	    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, entry->red);
	    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, entry->green);
	    BT459_WRITE(bti, BT459_TYPE2_CMD_CMAP, entry->blue);
#else
	    *bti->data = entry->red;				wbflush();
	    *bti->data = entry->green;				wbflush();
	    *bti->data = entry->blue;				wbflush();
#endif
            entry->dirty_cell = 0;
            lasti = i;
        }
    }
    bti->min_dirty = 256;
    bti->max_dirty = 0;
    bti->dirty_colormap = 0;

    SPLX(s);

    return;
}


bt459_2_cursor_on_off(closure, on_off)
	caddr_t closure;
	int on_off;
{
	register struct bt459info2 *bti = (struct bt459info2 *) closure;
	register int s = spltty();
	register unsigned int cmd;

        if (on_off)
	    cmd = 0xc0;
	else
	    cmd = 0x00;

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_LOW
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = CURSOR_CMD_REG;			wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_ADDR_HIGH
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = (CURSOR_CMD_REG >> 8);		wbflush();

	*bti->setup = bti->head_mask
		    | BT459_TYPE2_CMD_CURS
		    | BT459_TYPE2_SETUP_WRITE;	wbflush();
	*bti->data = cmd;				wbflush();

	bti->on_off = on_off;

	splx(s);
	return(0);
}
