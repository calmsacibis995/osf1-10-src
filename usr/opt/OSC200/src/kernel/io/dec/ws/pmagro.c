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
static char	*sccsid =  "@(#)$RCSfile: pmagro.c,v $ $Revision: 1.2.12.2 $ (DEC) $Date: 1993/06/24 22:44:27 $";
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

/*
 * Modification History
 *	23-Jan-91 --  Joel Gringorten  
 *
 *	Created from pmagaa.c.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <data/bt_data.c>
#include <io/common/devdriver.h>
#include <hal/cpuconf.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/fbinfo.h>
#include <sys/errno.h>
#include <sys/tty.h>
#include <machine/pmap.h>
#include <mach/kern_return.h>


#include <io/dec/ws/pmagro.h>
#include <machine/hal/kn15aa.h>            /* !!! KLUDGE !!! */

extern struct fb_info fb_softc[];
extern struct controller *fbinfo[];
extern struct bt463info bt463_softc[];
extern struct bt463info bt463_type[];
extern struct bt431info bt431_softc[];
extern struct bt431info bt431_type[];
extern int cpu;

extern caddr_t ws_map_region();

/* support for pip sleeps.  Since there's only one server running,
we don't worry about reentrancy of the sleep ioctls. */

static int pip_sleep_on_count;
static int pip_sleep_off_count;

pmagro_load_formatted_cursor();
pmagro_reformat_cursor();


#define PMAGRO_INTR_ENABLED(C,I)  (((I)->enable_interrupt) ? \
                                 ((*(I)->enable_interrupt)(C),1) : 0)


/*
 * macros for read/write of TCO_MAP_REGISTER
 */
#ifdef __alpha
/* NOTE: these are initially set-up for SPARSE space access */
#define READ_MAP_REGISTER() \
    *((u_int *)(ctlr->addr +  TCO_MAP_REGISTER))
#define WRITE_MAP_REGISTER(v) \
    { *((u_int *)(ctlr->addr +  TCO_MAP_REGISTER)) = (v); wbflush(); }
#else /* __alpha */
#define READ_MAP_REGISTER() \
    *(ctlr->addr +  TCO_MAP_REGISTER)
#define WRITE_MAP_REGISTER(v) \
    *(ctlr->addr +  TCO_MAP_REGISTER) = (v);
#endif /* __alpha */


/* all we need really do is install interrupt enable functions */
int pmagro_attach(ctlr)
    register struct controller *ctlr;
{
    register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
    /* we know we have a bt463 and bt431 */
    register struct bt463info *bti_463 = (struct bt463info *)fbp->cmf.cmc;
    register struct bt431info *bti_431 = (struct bt431info *)fbp->cf.cc;

    bti_463->enable_interrupt = pmagro_bt463_enable_interrupt;
    bti_431->enable_interrupt = pmagro_bt431_enable_interrupt;
}

void pmagro_interrupt(ctlr, closure)
    register struct controller *ctlr;
    caddr_t closure;
{
    static int junk;
    struct fb_info *fbp = (struct fb_info *) closure;
    struct bt463info *bti_463 = (struct bt463info *)fbp->cmf.cmc;
    struct bt431info *bti_431 = (struct bt431info *)fbp->cf.cc;

    junk = READ_MAP_REGISTER();
    junk &= ~TC1_FBC_INT_ENA;	  /* clear interrupt bit */
    WRITE_MAP_REGISTER(junk);


    if (bti_431->dirty_cursor) pmagro_load_formatted_cursor(bti_431);
    if (bti_463->dirty_colormap) bt463_clean_colormap(bti_463);
    if (bti_463->dirty_cursormap) bt463_clean_cursormap(bti_463);   
    if(pip_sleep_off_count) {
        Pip_Device_Regs *pregs =
                (Pip_Device_Regs *)(ctlr->addr + TCO_PIP_REGISTERS);
        if(pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE &&
                                        pip_sleep_off_count>0){
            pip_sleep_off_count--;
            pmagro_bt463_enable_interrupt(bti_463);
            return;
        } else {
            pip_sleep_off_count = 0;
            wakeup(&pip_sleep_off_count);
        }
    }
    if(pip_sleep_on_count) {
        Pip_Device_Regs *pregs =
                (Pip_Device_Regs *)(ctlr->addr + TCO_PIP_REGISTERS);
        if(!(pregs->control_status & TC1_CSR0_PIP_IS_ACTIVE) &&
                                        pip_sleep_on_count>0){
            pip_sleep_on_count--;
            pmagro_bt463_enable_interrupt(bti_463);
            return;
        } else {
            pip_sleep_on_count = 0;
            wakeup(&pip_sleep_on_count);
        }
    }
    tc_disable_option(fbinfo[fbp - fb_softc]);
}

void pmagro_bt463_enable_interrupt(closure)
        caddr_t closure;
{
    register struct bt463info *bti = (struct bt463info *) closure;
    register int unit = bti->unit;
    register struct controller *ctlr = fbinfo[unit];
    int junk, s;

    s = splhigh();
    tc_enable_option(fbinfo[unit]);
    junk = READ_MAP_REGISTER();			/* get reg*/
    WRITE_MAP_REGISTER(junk & ~TC1_FBC_INT_ENA);/* clear int */
    WRITE_MAP_REGISTER(junk | TC1_FBC_INT_ENA);	/* enable intr */
    splx(s);
}
void pmagro_bt431_enable_interrupt(closure)
    caddr_t closure;
{
    register struct bt431info *bti = (struct bt431info *) closure;
    register int unit = bti->unit;
    register struct controller *ctlr = fbinfo[unit];
    int junk, s;

    s = splhigh();
    tc_enable_option(fbinfo[unit]);
    junk = READ_MAP_REGISTER();			/* get reg*/
    WRITE_MAP_REGISTER(junk & ~TC1_FBC_INT_ENA);/* clear int */
    WRITE_MAP_REGISTER(junk | TC1_FBC_INT_ENA);	/* enable intr */
    splx(s);
}

caddr_t pmagro_bt431_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    struct bt431info *bp = (struct bt431info *)closure;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    addr = (caddr_t) ( (vm_offset_t) address
		     + (vm_offset_t) bt431_type[type].btaddr );
    for ( i = 0; i < nbt_softc; i++ ) {
	if ( addr == (caddr_t) bp[i].btaddr ) {
	    bp[i].unit = unit;
	    return (caddr_t) (&bp[i]);
	}
	else if ( bp[i].btaddr == NULL ) {
	    nextunit = i;
	    break;
	}
    }

    if ( i >= nbt_softc )
	return (caddr_t) NULL;

    bp += nextunit;
    nextunit++;

    /*
     * Set to initial values
     */
    *bp = bt431_type[type];	    /* set to initial values */
    bp->unit = unit;

    bp->btaddr = (struct bt431 *) addr;

    return (caddr_t)bp;
}


caddr_t pmagro_bt463_init_closure(closure, address, unit, type)
    caddr_t closure;
    caddr_t address;
    int unit;
    int type;
{
    struct bt463info *bp = (struct bt463info *)closure;
    register caddr_t addr;
    register u_int i;
    register u_int nextunit = 0;

    addr = (caddr_t) ( (vm_offset_t) address
		     + (vm_offset_t) bt463_type[type].btaddr );
    for ( i = 0; i < nbt_softc; i++ ) {
	if ( addr == (caddr_t) bp[i].btaddr ) {
	    bp[i].unit = unit;
	    return (caddr_t) (&bp[i]);
	}
	else if ( bp[i].btaddr == NULL ) {
	    nextunit = i;
	    break;
	}
    }

    if ( i >= nbt_softc )
	return (caddr_t) NULL;

    bp += nextunit;
    nextunit++;

    /*
     * Set to initial values
     */
    *bp = bt463_type[type];	    /* set to initial values */
    bp->unit = unit;

    bp->btaddr = (struct bt463 *) addr;

    return (caddr_t) bp;
}

pmagro_recolor_cursor(closure, screen, fg, bg)
    caddr_t closure;
    ws_screen_descriptor *screen;
    ws_color_cell *fg, *bg;
{
    register struct bt431info *btii = (struct bt431info *)closure;
    register int unit = btii->unit;
    register struct fb_info *fbp = &fb_softc[unit];
    register struct bt463info *bti = (struct bt463info *) fbp->cmf.cmc;

    return (bt463_recolor_cursor( bti, screen, fg, bg));
}

pmagro_video_off(closure)
    caddr_t closure;
{
    register struct bt463info *btii = (struct bt463info *)closure;  
    register int unit = btii->unit;
    register struct fb_info *fbp = &fb_softc[unit];
    register struct bt431info *bti = (struct bt431info *) fbp->cf.cc;

    bt463_video_off(closure);
    if(bti->on_off) {
	bt431_cursor_on_off(bti, 0);
	bti->cursor_was_on = 1;
    }
}

pmagro_video_on(closure)
    caddr_t closure;
{
    register struct bt463info *btii = (struct bt463info *)closure;
    register int unit = btii->unit;
    register struct fb_info *fbp = &fb_softc[unit];
    register struct bt431info *bti = (struct bt431info *) fbp->cf.cc;

    bt463_video_on(closure);
    if(bti->cursor_was_on) {
	bt431_cursor_on_off(bti, 1);
	bti->cursor_was_on = 0;
    }
}

static ws_depth_descriptor *dp; /* jmg - figure out a better way.... */

pmagro_map_screen(closure, depths, screen, mp)
	caddr_t closure;
	ws_depth_descriptor *depths;
	ws_screen_descriptor *screen;
	ws_map_control *mp;
{
	register struct fb_info *fbp = (struct fb_info *)closure;
	int nbytes;
	vm_offset_t addr;

	dp = depths + mp->which_depth;
	if (mp->map_unmap == UNMAP_SCREEN) return (EINVAL);
#ifdef __alpha
	nbytes = 0x1600000; /* SPARSE address space size = 2*normal */
#else /* __alpha */
	if(cpu == DS_5000 || cpu == DS_5000_300) nbytes = 0x400000;
	else 
	    nbytes = 0xb00000;
#endif /* __alpha */
	addr = (vm_offset_t)(dp->physaddr - TCO_EIGHT_BIT); /* tc slot addr */
	addr = ((vm_offset_t)(addr) + 0x10000000);
	if ((dp->pixmap = ws_map_region(addr, NULL, nbytes, 0600,
					(int *) NULL)) == NULL) {
		return(ENOMEM);
	}

#if defined(__alpha)
	/* map DENSE space for Alpha */
	addr = (vm_offset_t)(dp->physaddr - TCO_EIGHT_BIT); /* tc slot addr */
	if ((dp->plane_mask = ws_map_region(addr, NULL, nbytes>>1, 0600,
					    (int *) NULL)) == NULL) {
		return(ENOMEM);
	}
#endif /* __alpha */

	return (0);
}	

pmagro_load_cursor(closure, screen, cursor, sync)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_cursor_data *cursor;
	int sync;
{
	register struct bt431info *bti = (struct bt431info *)closure;

	if(!bti->inited) {
	    bti->inited = 1;
	    bt431_init(closure);
	}
	pmagro_reformat_cursor(bti, cursor);
	bti->x_hot = cursor->x_hot;
	bti->y_hot = cursor->y_hot;
	bt431_set_cursor_position(closure, screen, screen->x, screen->y);
	bti->dirty_cursor = 1;			/* cursor needs reloading */
        if (sync) {
            if ( PMAGRO_INTR_ENABLED( closure, bti ) ) {
                return (0);
            }
        }
	pmagro_load_formatted_cursor(bti);
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




static pmagro_reformat_cursor(bti, cursor)
	register ws_cursor_data *cursor;
	register struct bt431info *bti;
{
	register unsigned int cw, mw;
	register int i, j;
	register int nwords, shifts;
	register unsigned int mask, emask, omask;
	unsigned short *cbp = (unsigned short *) bti->bits;

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
		for (j = 0; j < 4; j++)	 {
		 *cbp = ((flip[mw & 0xff]) | 
			  (((flip[(cw & 0xff) & (mw & 0xff)]))) << 8);
		    cbp++;
		    cw >>= 8;
		    mw >>= 8;
		}
		if (cursor->width <= 32) cbp += 4;
	}
}
pmagro_load_formatted_cursor(bti)
	register struct bt431info *bti;
{
   	register volatile struct bt431 *btp = bti->btaddr;
	register u_short *bgp = (u_short *) bti->bits;
  	register int  i;
        int s = splhigh();

	btp->addr_low = 0;	wbflush();
	btp->addr_high = 0;	wbflush();
	for(i=0; i<512; i++) {
	    btp->cursor_ram = *bgp;
	    bgp++;
	    wbflush();
	}
	btp->addr_low = 0;	wbflush();
	btp->addr_high = 0;	wbflush();
	for(i=0; i<16; i++) {
	}
        splx(s);
	bti->dirty_cursor = 0;			/* no longer dirty */
}

pmagro_ioctl(closure, cmd, data, flag)
        caddr_t closure;
        u_int cmd;
        caddr_t data;
        int flag;
{
        rop_wait_ioctl *rip = (rop_wait_ioctl *) data;
        int s;

        s = splhigh();
        switch (rip->command) {
            case PIP_WAIT_OFF:
                pip_sleep_off_count = 6;
                pmagro_bt463_enable_interrupt(&bt463_softc[rip->screen]);
                while (pip_sleep_off_count)
                     sleep(&pip_sleep_off_count, TTIPRI);
                break;
            case PIP_WAIT_ON:
                pip_sleep_on_count = 6;
                pmagro_bt463_enable_interrupt(&bt463_softc[rip->screen]);
                while (pip_sleep_on_count)
                   sleep(&pip_sleep_on_count, TTIPRI);
                break;
            default:
		splx(s);
                return -1;
        }
        splx(s);
        return 0;
}
