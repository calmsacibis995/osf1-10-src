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
static char *rcsid = "@(#)$RCSfile: pmagdv.c,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/06/24 22:44:19 $";
#endif
/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
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
 * Modification History
 * 	Joel Gringorten  	16-Jul-91
 * 
 *	support for Maxine Baseboard graphics
 */

#include <sys/types.h>
#include <kern/queue.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/fbinfo.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/common/devdriver.h>
#include <hal/cpuconf.h>

#include <io/dec/ws/pmagdv.h>
#include <io/dec/ws/ims_g332.h>

extern struct fb_info fb_softc[];
extern struct controller *fbinfo[];
extern struct ims_g332info ims_g332_softc[];
extern struct ims_g332info ims_g332_type[];

void cfb_enable_interrupt();
/* all we need really do is install an interrupt enable function */
int pmagdv_attach(ctlr)
	register struct  controller *ctlr;
{
#ifdef NOTDEF
        register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
	/* we know we have a ims_g332 */
	register struct ims_g332info *bti = (struct ims_g332info *)fbp->cf.cc;
	bti->enable_interrupt = pmagdv_enable_interrupt;
#endif NOTDEF
}

void pmagdv_interrupt(ctlr, closure)
	register struct controller *ctlr;
	caddr_t closure;
{
	struct fb_info *fbp = (struct fb_info *) closure;
	struct ims_g332info *bti = (struct ims_g332info *)fbp->cf.cc;
#ifdef NOTDEF
	*(ctlr->addr + pmagdv_IREQ_OFFSET) = 0;	/* clear the interrupt */
	if (bti->dirty_cursor) bt_load_formatted_cursor(bti);
	if (bti->dirty_colormap) bt_clean_colormap(bti);
	tc_disable_option(fbinfo[fbp - fb_softc]);
#endif NOTDEF
}

void pmagdv_enable_interrupt(closure)
	caddr_t closure;
{
#ifdef NOTDEF
	int s;

	s = splhigh();
	tc_enable_option(fbinfo[unit]);
	register struct ims_g332info *bti = (struct ims_g332info *) closure;
	register int unit = bti - ims_g332_softc;
	register struct controller *ctlr = fbinfo[unit];
	*(ctlr->addr + pmagdv_IREQ_OFFSET) = 0;	/* clear the interrupt */
	splx(s);
#endif NOTDEF
}

caddr_t pmagdv_init_closure(closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
	int type;
{
	struct ims_g332info *bp = (struct ims_g332info *)closure;
	bp = bp + unit;			/* set to correct unit */
	*bp = ims_g332_type[type];		/* set to initial values */
	return (caddr_t)bp;
}


caddr_t  pmagdv_screen_init_closure (closure, address, unit, type)
	caddr_t closure;
	caddr_t address;
	int unit;
{
	struct fb_info  *fbp = (struct fb_info *) closure;
	register int i;
	i = 0;
	fbp = fbp + unit;
	fbp->depth[i].physaddr = (caddr_t)PHYS_TO_K0(fbp->depth[i].physaddr);
	return (caddr_t) &fbp->screen;
}
