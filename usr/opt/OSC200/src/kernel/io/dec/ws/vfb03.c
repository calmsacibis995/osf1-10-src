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
static char	*sccsid = "@(#)$RCSfile: vfb03.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/10/23 14:07:44 $";
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
 *
 * November, 1989	Jim Gettys
 *
 *		Complete rewrite for multiscreen support.  Boy how kludges
 *		accumulate over time....
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/common/devdriver.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <sys/fbinfo.h>
#include <io/dec/ws/vfb03.h>
#include <io/dec/ws/bt459.h>

extern struct fb_info fb_softc[];
extern struct controller *fbinfo[];

void cfb_enable_interrupt();
/* all we need really do is install an interrupt enable function */
int vfb03_attach(ctlr)
	register struct controller *ctlr;
{
        register struct fb_info *fbp = &fb_softc[ctlr->ctlr_num];
	/* we know we have a bt459 */
	register struct bt459info *bti = (struct bt459info *)fbp->cf.cc;
	bti->enable_interrupt = vfb03_enable_interrupt;
}

void vfb03_interrupt(ctlr, closure)
	register struct controller *ctlr;
	caddr_t closure;
{
	struct fb_info *fbp = (struct fb_info *) closure;
	struct bt459info *bti = (struct bt459info *)fbp->cf.cc;
	*(ctlr->addr + VFB03_IREQ_OFFSET) = 0;	/* clear the interrupt */
	if (bti->dirty_cursor) bt_load_formatted_cursor(bti);
	if (bti->dirty_colormap) bt_clean_colormap(bti);
	tc_disable_option(fbinfo[fbp - fb_softc]);
}

void vfb03_enable_interrupt(closure)
	caddr_t closure;
{
	register struct bt459info *bti = (struct bt459info *) closure;
	register int unit = bti - bt459_softc;
	register struct controller *ctlr = fbinfo[unit];
	int s;

	s = splhigh();
	tc_enable_option(fbinfo[unit]);
	*(ctlr->addr + VFB03_IREQ_OFFSET) = 0;	/* clear the interrupt */
	splx(s);
}
