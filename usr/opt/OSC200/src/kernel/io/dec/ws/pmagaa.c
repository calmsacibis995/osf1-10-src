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
static char	*sccsid = "@(#)$RCSfile: pmagaa.c,v $ $Revision: 1.2.9.2 $ (DEC) $Date: 1993/06/24 22:44:11 $";
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
 *                                                                      *
 *                      Copyright (c) 1990 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/*
 * Modification History
 *	31-Aug-90 --  Joel Gringorten  
 *
 *	Created from vfb03.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <data/bt_data.c>
#include <io/common/devdriver.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/fbinfo.h>


#include <io/dec/ws/pmagaa.h>
#include <io/dec/ws/bt455.h>
#include <io/dec/ws/bt431.h>

extern struct fb_info fb_softc[];
extern struct controller *fbinfo[];
extern struct bt455info bt455_softc[];
extern struct bt455info bt455_type[];
extern struct bt431info bt431_softc[];
extern struct bt431info bt431_type[];


/* all we need really do is install interrupt enable functions */
int pmagaa_attach(ctlr)
        register struct controller *ctlr;
{
        register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
        /* we know we have a bt455 and bt431 */
        register struct bt455info *bti_455 = (struct bt455info *)fbp->cmf.cmc;
        register struct bt431info *bti_431 = (struct bt431info *)fbp->cf.cc;
        bti_455->enable_interrupt = pmagaa_bt455_enable_interrupt;
	bti_431->enable_interrupt = pmagaa_bt431_enable_interrupt;
}

void pmagaa_interrupt(ctlr, closure)
        register struct controller *ctlr;
        caddr_t closure;
{
	static int junk;
        struct fb_info *fbp = (struct fb_info *) closure;
        struct bt455info *bti_455 = (struct bt455info *)fbp->cmf.cmc;
        struct bt431info *bti_431 = (struct bt431info *)fbp->cf.cc;
	junk =  *(ctlr->addr +  PMAGAA_IREQ_OFFSET); /* clear */
        *(ctlr->addr + PMAGAA_IREQ_OFFSET) = 0; /* disable interrupt */
        if (bti_431->dirty_cursor) bt431_load_formatted_cursor(bti_431);
        if (bti_455->dirty_colormap) bt455_clean_colormap(bti_455);
	if (bti_455->dirty_cursormap) bt455_clean_cursormap(bti_455);	
        tc_disable_option(fbinfo[fbp - fb_softc]);
}

void pmagaa_bt455_enable_interrupt(closure)
        caddr_t closure;
{
        register struct bt455info *bti = (struct bt455info *) closure;
        register int unit = bti->unit;
        register struct controller *ctlr = fbinfo[unit];
	int junk, s;

	s = splhigh();
	tc_enable_option(fbinfo[unit]);
	*(ctlr->addr +  PMAGAA_IREQ_OFFSET) = 0;    /* disable intr */
        junk = *(ctlr->addr +  PMAGAA_IREQ_OFFSET); /* clear int */
	 *(ctlr->addr +  PMAGAA_IREQ_OFFSET) = 1;  /* enable intr */
	splx(s);
}

void pmagaa_bt431_enable_interrupt(closure)
        caddr_t closure;
{
        register struct bt431info *bti = (struct bt431info *) closure;
        register int unit = bti->unit;
        register struct controller *ctlr = fbinfo[unit];
	int junk, s;

	s = splhigh();
	tc_enable_option(fbinfo[unit]);
	*(ctlr->addr +  PMAGAA_IREQ_OFFSET) = 0;    /* disable intr */
        junk = *(ctlr->addr +  PMAGAA_IREQ_OFFSET); /* clear int */
	 *(ctlr->addr +  PMAGAA_IREQ_OFFSET) = 1;  /* enable intr */
	splx(s);
}

caddr_t pmagaa_bt431_init_closure(closure, address, unit, type)
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

	*bp = bt431_type[type];
	bp->unit = unit;

        bp->btaddr = (struct bt431 *) addr;

        return (caddr_t)bp;
}


caddr_t pmagaa_bt455_init_closure(closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
	int type;
{
        struct bt455info *bp = (struct bt455info *)closure;
	register caddr_t addr;
	register u_int i;
	register u_int nextunit = 0;

	addr = (caddr_t) ( (vm_offset_t) address
			 + (vm_offset_t) bt455_type[type].btaddr );
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

	*bp = bt455_type[type];
	bp->unit = unit;

        bp->btaddr = (struct bt455 *) addr;

        return (caddr_t)bp;
}

pmagaa_recolor_cursor(closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct bt431info *btii	= (struct bt431info *)closure;
	register int unit = btii->unit;
	register struct fb_info *fbp = &fb_softc[unit];

	return (bt455_recolor_cursor( (struct bt455info *) fbp->cmf.cmc,
		screen, fg, bg));
}

pmagaa_video_off(closure)
	caddr_t closure;
{
   	register struct bt455info *btii = (struct bt455info *)closure;	
	register int unit = btii->unit;
	register struct fb_info *fbp = &fb_softc[unit];
    	register struct bt431info *bti = (struct bt431info *) fbp->cf.cc;

	bt455_video_off(closure);
	if(bti->on_off) {
	    bt431_cursor_on_off(bti, 0);
	    bti->cursor_was_on = 1;
	}
}

pmagaa_video_on(closure)
	caddr_t closure;
{
	register struct bt455info *btii	= (struct bt455info *)closure;
	register int unit = btii->unit;
	register struct fb_info *fbp = &fb_softc[unit];
    	register struct bt431info *bti = (struct bt431info *) fbp->cf.cc;

	bt455_video_on(closure);
	if(bti->cursor_was_on) {
	    bt431_cursor_on_off(bti, 1);
	    bti->cursor_was_on = 0;
	}
}

