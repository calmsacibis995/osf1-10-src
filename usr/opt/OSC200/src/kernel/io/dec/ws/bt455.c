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
static char	*sccsid = "@(#)$RCSfile: bt455.c,v $ $Revision: 1.2.3.3 $ (DEC) $Date: 1992/03/19 15:15:31 $";
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
 *
 *	15-May-91 -- Joel gringorten
 *
 * 		Provide spl synchronization for IO
 *
 *	 September, 1990	Joel Gringorten
 *
 *		Created.
 */

#include <sys/types.h>
#include <sys/workstation.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <io/common/devdriver.h>

#include <io/dec/ws/vfb03.h>
#include <io/dec/ws/bt455.h>

extern struct controller *fbinfo[];
extern struct bt455info bt455_softc[];
extern struct bt455info bt455_type[];


bt455_init_color_map(closure)
	caddr_t closure;
{
	register int i;
	register struct bt455info *bti 	= (struct bt455info *)closure;
	register volatile struct bt455 *btp 	= bti->btaddr;    

	btp->cmap_addr = 0;		wbflush();
	btp->clear_addr = 0;		wbflush();

	btp->color_map = 0; 		wbflush();
	btp->color_map = 0; 		wbflush();
	btp->color_map = 0; 		wbflush();

	btp->color_map = 0x00; 		wbflush();
	btp->color_map = 0xff; 		wbflush();
	btp->color_map = 0x00; 		wbflush();

    	for(i = 2; i <16; i++) {
		btp->color_map = 0x0;	wbflush();
		btp->color_map = 0x0;	wbflush();
		btp->color_map = 0x0;	wbflush();
	}

	bti->cursor_fg = 0xff;
	bti->cursor_bg = 0x00;
	bt455_restore_cursor_color(closure);
}


/* returns 0 if succeeded, -1 if it couldn't (index too big) */
/* shouldn't be here */

/* ARGSUSED */
int bt455_load_color_map_entry(closure, map, entry)
	caddr_t closure;
	int map;		
	register ws_color_cell *entry;
{
	return (-1);
}

void bt455_clean_colormap(closure)
	caddr_t closure;
{
	register struct bt455info *bti 		= (struct bt455info *)closure;
	register volatile struct bt455 *btp 	= bti->btaddr;
}


int bt455_video_off(closure)
	caddr_t closure;
{
	register struct bt455info *bti 		= (struct bt455info *)closure;
	register volatile struct bt455 *btp 	= bti->btaddr;    
	register int s = spltty();
	btp->cmap_addr = 1;		wbflush();
	btp->clear_addr = 0;		wbflush();
	btp->color_map = 0; 		wbflush();
	btp->color_map = 0; 		wbflush();
	btp->color_map = 0; 		wbflush();
	splx(s);
	return(0);
}

int bt455_video_on(closure)
	caddr_t closure;
{
	register struct bt455info *bti		= (struct bt455info *)closure;
	register volatile struct bt455 *btp 	= bti->btaddr;    
	register int s = spltty();
	btp->cmap_addr = 1;		wbflush();
	btp->clear_addr = 0;		wbflush();
	btp->color_map = 0x00; 		wbflush();
	btp->color_map = 0xff; 		wbflush();
	btp->color_map = 0x00; 		wbflush();
	splx(s);
	return(0);
}

bt455_recolor_cursor (closure, screen, fg, bg)
	caddr_t closure;
	ws_screen_descriptor *screen;
	ws_color_cell *fg, *bg;
{
	register struct bt455info *bti 	= (struct bt455info *)closure;
	bti->cursor_fg = fg->green >> 8;
	bti->cursor_bg = bg->green >> 8;
	bt455_restore_cursor_color(closure);
	return 0;
}


bt455_restore_cursor_color(closure)
	caddr_t closure;
{
    	register struct bt455info *bti 	= (struct bt455info *)closure;
	bti->dirty_cursormap = 1;	/* cursor needs recoloring */
	if (bti->enable_interrupt) 
		(*bti->enable_interrupt)(closure);
	else 
		bt455_clean_cursormap(bti);
	return(0);


}

bt455_clean_cursormap(closure)
	caddr_t closure;
{
    	register struct bt455info *bti 	= (struct bt455info *)closure;
	register volatile struct bt455 *btp 	= bti->btaddr;
	register int s = spltty();
	/* 
	 * funny hardware
	 * colormaps 8 and 9 are cursor background 
	 * overlay is cursor foreground 
	 */
	btp->cmap_addr = 8;		wbflush();
	btp->clear_addr = 0;		wbflush();
	btp->color_map = 0; 		wbflush();
	btp->color_map = bti->cursor_bg; wbflush();
	btp->color_map = 0; 		wbflush();
	btp->color_map  = 0; 		wbflush();
	btp->color_map = bti->cursor_bg; wbflush();
	btp->color_map  = 0; 		wbflush();
	btp->overlay = 0;		wbflush();
	btp->overlay = bti->cursor_fg;   wbflush();
	btp->overlay = 0;		wbflush();
	bti->dirty_cursormap = 0;
	splx(s);
}

