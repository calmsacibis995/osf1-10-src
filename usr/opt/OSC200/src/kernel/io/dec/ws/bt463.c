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
static char	*sccsid = "@(#)$RCSfile: bt463.c,v $ $Revision: 1.2.11.2 $ (DEC) $Date: 1993/11/17 23:13:23 $";
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

#include <sys/types.h>
#include <sys/workstation.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/dec/uba/ubavar.h>
#include <io/dec/ws/bt463.h>
#include <io/dec/ws/pmagro.h> /* jmg - GET RID OF ME!!!! */

extern struct bt463info bt463_type[];
extern struct bt463info2 bt463_type2[];

#define BT463_RAISE_SPL()		splhigh()
#define	BT463_LOWER_SPL(_s)		splx(_s)


/*"bt463_clean_colormap"
 *
 *	Perform the actual loading of the lookup table to update
 *	it to pristine condition.
 */
void bt463_clean_colormap(closure)
	caddr_t closure;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;
	register int i;
	register struct bt463_color_cell *entry;
	register unsigned int color;
        register int s = BT463_RAISE_SPL();

	for (i = bti->min_dirty; i <= bti->max_dirty ; i++) {
		entry = &bti->cells[i];
		if (entry->dirty_cell) {
			if ( bti->screen_on == 0 && i == 0 )
			    continue;	/* skip cmap[0] if screen saver on */
			entry->dirty_cell = 0;
                        btp->addr_low = i & ADDR_LOW_MASK;    wbflush();
                        btp->addr_high = (i & ADDR_HIGH_MASK) >> 8; wbflush();
			btp->color_map = entry->red; 	wbflush();
			btp->color_map = entry->green; 	wbflush();
			btp->color_map = entry->blue; 	wbflush();
		}
	}
	if ( bti->screen_on == 0 && bti->cells[0].dirty_cell )
		bti->min_dirty = 0;	/* cmap[0] is left dirty */
	else {
		bti->min_dirty = BT463_CMAP_ENTRY_COUNT;
		bti->dirty_colormap = 0;
	}
        BT463_LOWER_SPL(s);
	bti->max_dirty = 0;
}

void
bt463_load_wid(dreg, btp, index, count, data)
	Fb_Device_Regs	*dreg;	/* -> Brooktree 463 register area. */
	struct bt463 *btp;
	int index;		/* =  first location in wid to load. */
	int count;		/* =  number of entries in wid to load. */
	Bt463_Wid_Cell  *data;	/* -> data to load into wid. */
{
	int data_i;	/* Index: entry in data now accessing. */

	btp->addr_low = index & ADDR_LOW_MASK;	wbflush();
	btp->addr_high = 3;			wbflush();
	if ( dreg != (Fb_Device_Regs *) NULL ) {
#ifdef __alpha
	    dreg->fb_control = 0x12;			wbflush();
#else /* __alpha */
	    dreg->fb_control = 0x2;			wbflush();
#endif /* __alpha */
	}

	for( data_i = 0; data_i < count; data_i++ )
	{
		btp->addr_low = (data_i+index) & ADDR_LOW_MASK;wbflush();
		btp->addr_high = 3;			wbflush();
		btp->bt_reg = data[data_i].low_byte;	wbflush();
		btp->bt_reg = data[data_i].middle_byte;	wbflush();
		btp->bt_reg = data[data_i].high_byte;	wbflush();
	}

	return;
}


bt463_init(closure)
	caddr_t closure;
{
    register struct bt463info *bti = (struct bt463info *)closure;
    register volatile struct bt463 *dreg = bti->btaddr;    
    u_int	color_i;	      /* Index: next lut entry to set. */
    u_int	color_v;	      /* Value to load into lut. #### */
    int		i;
    static  Bt463_Wid_Cell  wids[BT463_WINDOW_TAG_COUNT] = 
      /* Window id values to load: */
      /* Low   Mid   High  xxxx */
      /* 0-7   8-15  16-23	*/
	{
                {0x00, 0x65, 0x00, 0x00}, /* ... 0 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 1 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 2 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 3 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 4 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 5 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 6 */
                {0x00, 0x65, 0x00, 0x00}, /* ... 7 */
                {0x00, 0x65, 0x20, 0x00}, /* ... 8 */
                {0x00, 0x65, 0x20, 0x00}, /* ... 9 */
                {0x00, 0x65, 0x20, 0x00}, /* ... A */
                {0x00, 0x65, 0x20, 0x00}, /* ... B */
                {0x00, 0x65, 0x20, 0x00}, /* ... C */
                {0x00, 0x65, 0x20, 0x00}, /* ... D */
                {0x00, 0x65, 0x20, 0x00}, /* ... E */
                {0x00, 0x65, 0x20, 0x00}, /* ... F */
	};
    static unsigned char data0[13] =
	{
	    0x40,
	    0x46,
	    0xC0,
	    0x00,
	    0xff,
	    0xff,
	    0xff,
	    0xff,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00
	};

    /*	    
     *	    Configure the RAMDAC.
     *	    Initialize pixel read mask, blink mask and test register.
     */
    dreg->addr_low = 1;		wbflush();
    dreg->addr_high = 2;	wbflush();
    for ( i = 0; i < 13; i++ ) {
	dreg->bt_reg = data0[i];
	wbflush();
    }

    /*
     *	    Initialize	wid rams.
     *		    (a)	    2-bit cursor interface on pins P24 and P25 ####
     *		    (b) 24-bit true color (8-bit uses identical rgb
     *							 components!)
     *		    (c) start color map at 0
     *	    The order of loads of the register is P0-P7, P8-P15, P16-P23.
     */
    bt463_load_wid(dreg, dreg, 0, BT463_WINDOW_TAG_COUNT, wids );

    /*
     * Done
     */
    return;
}

/*"bt463_init_color_map"
 *
 *	Initialize color map in Brooktree 463. Note the entire
 *	color map is initialized, both the 8-bit and the 24-bit
 *	portions.
 */

bt463_init_color_map(closure)
	caddr_t closure;    
{
	register struct bt463info *bti = (struct bt463info *)closure; 
	register volatile struct bt463 *btp = bti->btaddr;
	register int i;	
    	register volatile Fb_Device_Regs *dreg = (Fb_Device_Regs *)  btp;

#ifdef __alpha
	dreg->fb_control = 0x12;			wbflush();
#else /* __alpha */
	dreg->fb_control = 0x2;			wbflush();
#endif /* __alpha */

	bt463_init(closure);

	btp->addr_low = LUT_BASE_8 & ADDR_LOW_MASK; 	wbflush();
	btp->addr_high = (LUT_BASE_8 & ADDR_HIGH_MASK) >> 8; 	wbflush();
	btp->color_map = 0; 				wbflush();
	btp->color_map = 0; 			wbflush();
	btp->color_map = 0; 			wbflush();

        for(i = 1; i <256; i++) 
	{
	    btp->color_map = 0xffffff;			wbflush();
	    btp->color_map = 0xffffff;		wbflush();
	    btp->color_map = 0xffffff;		wbflush();
	}

	btp->addr_low = LUT_BASE_24 & ADDR_LOW_MASK; 	wbflush();
	btp->addr_high = (LUT_BASE_24 & ADDR_HIGH_MASK) >> 8; 	wbflush();

	for(i = 0; i <256; i++) 
	{
	    btp->color_map = i;			wbflush();
	    btp->color_map = i;			wbflush();
	    btp->color_map = i;			wbflush();
	}

	bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue 
		= 0xffff;
	bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue 
		= 0x0000;
	bt463_restore_cursor_color(closure, 0);

#ifdef __alpha
	dreg->fb_control = 0x92;		wbflush();
#else /* __alpha */
        dreg->fb_control = 0x82;		wbflush();
#endif /* __alpha */

}

/*"bt463_load_color_map_entry"
 *
 *	Load one or more entries in the bt463's color lookup table.
 *		= 0 if success
 *		= -1 if error occurred, (index too big)
 *      Two ways to look at colormap.  If map == 1, direct colormap
 *      access.  If map == 0, pseudocolor index 256 == direct color index 0.
 */
/* ARGSUSED */
int bt463_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;
	register ws_color_cell *entry;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;
	register int index = entry->index;
	int s;

        if ( map == 1 )
                index += 256;
	if ( map == 2 )
		index += 512;
	if ( index >= BT463_CMAP_ENTRY_COUNT ) 
		return -1;

	s = BT463_RAISE_SPL();
	bti->cells[index].red   = entry->red   >> 8;
	bti->cells[index].green = entry->green >> 8;
	bti->cells[index].blue  = entry->blue  >> 8;
	bti->cells[index].dirty_cell = 1;
	if (index < bti->min_dirty) bti->min_dirty = index;
	if (index > bti->max_dirty) bti->max_dirty = index;
	bti->dirty_colormap = 1;
	BT463_LOWER_SPL(s);
	if (bti->enable_interrupt) 
		(*bti->enable_interrupt)(closure);
	else 
		bt463_clean_colormap(bti);
	return 0;
}

/*"bt463_load_reg"
 *
 *	Load a command into the Brooktree 463 command register.
 */
bt463_load_reg(btp, reg, val)
	register volatile struct bt463 *btp;
	u_short reg;
	u_char val;
{
	btp->addr_low  = (reg & ADDR_LOW_MASK);		wbflush();
	btp->addr_high = (reg & ADDR_HIGH_MASK) >> 8;	wbflush();
	btp->bt_reg = val;				wbflush();
}

/*"bt463_video_on"
 *
 *	Turn on video display.
 */
int bt463_video_on(closure)
	caddr_t closure;
{
	register struct bt463info *bti		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;    
	register volatile Fb_Device_Regs *dreg = (Fb_Device_Regs *)  btp;
        register int s;

	if ( bti->screen_on ) 
	    return (0);

	s = BT463_RAISE_SPL();
	dreg->fb_control |= TC1_FBC_VIDEO_ON;	wbflush();
	bti->screen_on = 1;
        BT463_LOWER_SPL(s);
	return(0);
}

/*"bt463_video_off"
 *
 *	Turn off video display.
 */
int bt463_video_off(closure)
	caddr_t closure;
{
	register struct bt463info *bti 		= (struct bt463info *)closure;
	register volatile struct bt463 *btp 	= bti->btaddr;    
	register volatile Fb_Device_Regs *dreg = (Fb_Device_Regs *)  btp;
        register int s;

	if ( !bti->screen_on )
	    return (0);

	s = BT463_RAISE_SPL();
	dreg->fb_control &= ~TC1_FBC_VIDEO_ON;	wbflush();
	bti->screen_on = 0;
        BT463_LOWER_SPL(s);
	return(0);
}

 
bt463_recolor_cursor (closure, screen, fg, bg)
        caddr_t closure;
        ws_screen_descriptor *screen;
        ws_color_cell *fg, *bg;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        bti->cursor_fg = *fg;
        bti->cursor_bg = *bg;
        bt463_restore_cursor_color(closure, 1);
        return 0;
}

bt463_restore_cursor_color(closure, sync)
        caddr_t closure;
	int sync;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        register volatile struct bt463 *btp     = bti->btaddr;
        bti->dirty_cursormap = 1;       /* cursor needs recoloring */
	if (sync) {
            if (bti->enable_interrupt) {
                (*bti->enable_interrupt)(closure);
		return (0);
	    }
	}
	bt463_clean_cursormap(bti);
        return(0);
}

bt463_clean_cursormap(closure)
        caddr_t closure;
{
        register struct bt463info *bti  = (struct bt463info *)closure;
        register volatile struct bt463 *btp     = bti->btaddr;
        register int s = BT463_RAISE_SPL();

	btp->addr_low = (CURSOR_COLOR0 & ADDR_LOW_MASK);wbflush();
	btp->addr_high = (CURSOR_COLOR0 & ADDR_HIGH_MASK) >> 8;	wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8; 	wbflush(); /* Cursor location 0. */	
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8;	wbflush();

	btp->bt_reg = bti->cursor_fg.red >> 8; 	wbflush(); /*Cursor location 1. */	
	btp->bt_reg = bti->cursor_fg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_fg.blue >> 8; wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8;	wbflush(); /* Cursor location 2. */
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8; wbflush();

	btp->bt_reg = bti->cursor_bg.red >> 8; wbflush(); /* Cursor location 3. */	
	btp->bt_reg = bti->cursor_bg.green >> 8; wbflush();
	btp->bt_reg = bti->cursor_bg.blue >> 8; wbflush();
        BT463_LOWER_SPL(s);
        bti->dirty_cursormap = 0;
}


/*
 * Beginning of 463 support of SFB+-type options
 */


/* bt463_2_clean_colormap
 *
 *	Perform the actual loading of the lookup table to update
 *	it to pristine condition.
 */
void bt463_2_clean_colormap(closure)
	caddr_t closure;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    register int i, nextcell = -1;
    register struct bt463_color_cell *entry;
    register int s = BT463_RAISE_SPL();

    for (i = bti->min_dirty; i <= bti->max_dirty ; i++) {
	entry = &bti->cells[i];
	if (entry->dirty_cell) {
	    entry->dirty_cell = 0;
	    if ( i != nextcell ) {
		*bti->setup = bti->head_mask
			    | BT463_TYPE2_ADDR_LOW;		wbflush();
		*bti->data = i;					wbflush();

		*bti->setup = bti->head_mask
			    | BT463_TYPE2_ADDR_HIGH;		wbflush();
		*bti->data = ( i >> 8 );			wbflush();

		*bti->setup = bti->head_mask
			    | BT463_TYPE2_CMD_CMAP;		wbflush();
	    }
	    *bti->data = entry->red;				wbflush();
	    *bti->data = entry->green;				wbflush();
	    *bti->data = entry->blue;				wbflush();
	    nextcell = i+1;
    	}
    }
    BT463_LOWER_SPL(s);
    bti->min_dirty = BT463_CMAP_ENTRY_COUNT;
    bti->max_dirty = 0;

    return;
}


void
bt463_2_load_wid( bti, index, count, data )
	struct bt463info2 *bti;
	int index;
	int count;
	Bt463_Wid_Cell  *data;
{
    int data_i;

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_LOW;		wbflush();
    *bti->data = index;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_HIGH;	wbflush();
    *bti->data = 3;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_CMD_CURS;		wbflush();

    for( data_i = 0; data_i < count; data_i++ ) {
	*bti->data = data[data_i].low_byte;	wbflush();
	*bti->data = data[data_i].middle_byte;	wbflush();
	*bti->data = data[data_i].high_byte;	wbflush();
    }

    return;
}


bt463_2_init(closure)
	caddr_t closure;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    int		i;

    static unsigned char data0[13] = {
	    0x48,
	    0x48,
	    0xC0,
	    0x00,
	    0xff,
	    0xff,
	    0xff,
	    0xff,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00
    };

    /*	    
     *	    Configure the RAMDAC.
     *	    Initialize pixel read mask, blink mask and test register.
     */
    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_LOW;		wbflush();
    *bti->data = 1;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_HIGH;	wbflush();
    *bti->data = 2;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_CMD_CURS;		wbflush();
    for ( i = 0; i < 13; i++ ) {
	*bti->data = data0[i];			wbflush();
    }

    /*
     * Done
     */
    return;
}


/* bt463_2_init_color_map
 *
 *	Initialize color map in Brooktree 463. Note the entire
 *	color map is initialized, both the 8-bit and the 24-bit
 *	portions.
 */

bt463_2_init_color_map(closure)
	caddr_t closure;    
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure; 
    register int i;	

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_LOW;		wbflush();
    *bti->data = 0;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_HIGH;	wbflush();
    *bti->data = 0;				wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_CMD_CURS;		wbflush();
    *bti->data = 0x00;				wbflush();
    *bti->data = 0x00;				wbflush();
    *bti->data = 0x00;				wbflush();
    for(i = 1; i <256; i++) {
	*bti->data = 0xff;			wbflush();
	*bti->data = 0xff;			wbflush();
	*bti->data = 0xff;			wbflush();
    }
    *bti->data = 0x00;				wbflush();
    *bti->data = 0x00;				wbflush();
    *bti->data = 0x00;				wbflush();
    for(i = 1; i <256; i++) {
	*bti->data = 0xff;			wbflush();
	*bti->data = 0xff;			wbflush();
	*bti->data = 0xff;			wbflush();
    }

    bti->cursor_fg.red = bti->cursor_fg.green = bti->cursor_fg.blue 
		= 0xffff;
    bti->cursor_bg.red = bti->cursor_bg.green = bti->cursor_bg.blue 
		= 0x0000;
    bt463_2_restore_cursor_color( closure, 0 );

    return 0;
}


/* bt463_load_color_map_entry
 *
 *	Load one or more entries in the bt463's color lookup table.
 *		= 0 if success
 *		= -1 if error occurred, (index too big)
 *      Two ways to look at colormap.  If map == 1, direct colormap
 *      access.  If map == 0, pseudocolor index 256 == direct color index 0.
 */
/* ARGSUSED */
int bt463_2_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;
	register ws_color_cell *entry;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    register int index = entry->index;
    int s;

    if ( map == 1 )
	index += 256;
    if ( map == 2 )
	index += 512;
    if ( index >= BT463_CMAP_ENTRY_COUNT ) 
	return -1;

    s = BT463_RAISE_SPL();
    bti->cells[index].red   = entry->red   >> 8;
    bti->cells[index].green = entry->green >> 8;
    bti->cells[index].blue  = entry->blue  >> 8;
    bti->cells[index].dirty_cell = 1;
    if (index < bti->min_dirty) bti->min_dirty = index;
    if (index > bti->max_dirty) bti->max_dirty = index;
    bti->dirty_colormap = 1;
    BT463_LOWER_SPL(s);
    if (bti->enable_interrupt) 
	(*bti->enable_interrupt)(closure);
    else 
	bt463_2_clean_colormap(bti);

    return 0;
}


bt463_2_recolor_cursor (closure, screen, fg, bg)
        caddr_t closure;
        ws_screen_descriptor *screen;
        ws_color_cell *fg, *bg;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    bti->cursor_fg = *fg;
    bti->cursor_bg = *bg;
    bt463_2_restore_cursor_color(closure, 1);
    return 0;
}


bt463_2_restore_cursor_color(closure, sync)
        caddr_t closure;
	int sync;
{
    register struct bt463info2 *bti  = (struct bt463info2 *) closure;

    bti->dirty_cursormap = 1;       /* cursor needs recoloring */
    if (sync) {
	if (bti->enable_interrupt) {
	    (*bti->enable_interrupt)(closure);
	    return (0);
	}
    }
    bt463_2_clean_cursormap(bti);
    return(0);
}


bt463_2_clean_cursormap(closure)
        caddr_t closure;
{
    register struct bt463info2 *bti = (struct bt463info2 *) closure;
    register int s = BT463_RAISE_SPL();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_LOW;		wbflush();
    *bti->data = CURSOR_COLOR0;			wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_ADDR_HIGH;	wbflush();
    *bti->data = ( CURSOR_COLOR0 >> 8 );	wbflush();

    *bti->setup = bti->head_mask
		| BT463_TYPE2_CMD_CURS;		wbflush();

    *bti->data = ( bti->cursor_bg.red >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.green >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.blue >> 8 );		wbflush();

    *bti->data = ( bti->cursor_fg.red >> 8 );		wbflush();
    *bti->data = ( bti->cursor_fg.green >> 8 );		wbflush();
    *bti->data = ( bti->cursor_fg.blue >> 8);		wbflush();

    *bti->data = ( bti->cursor_bg.red >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.green >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.blue >> 8 );		wbflush();

    *bti->data = ( bti->cursor_bg.red >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.green >> 8 );		wbflush();
    *bti->data = ( bti->cursor_bg.blue >> 8 );		wbflush();

    BT463_LOWER_SPL(s);
    bti->dirty_cursormap = 0;
    return 0;
}

