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
 * @(#)$RCSfile: vsxxx_data.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/07/30 18:29:13 $
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

/************************************************************************
 * Modification History
 * 3-March-93 -- Jay Estabrook
 *	       Created from ws_data.c for isolation of vsxxx info.
 *
 ************************************************************************/
#define _VSXXX_DATA_C_

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <io/common/devio.h>
#include <sys/buf.h>

#include <io/dec/ws/vsxxx.h>

#ifdef BINARY

extern ws_hardware_type		vsxxx_mouse_closure;
extern ws_pointer		vsxxx_mouse;


#else /*BINARY*/

/* vsxxx-specific public pointer data */
/* the *real* pointer stuff is copied from here */
/* during console initialization early in boot-up, */
/* by the hardware-specific console init code; */
/* for example, the vsxxx pointer particulars are */
/* copied during scc_cons_init processing on flamingo */

ws_hardware_type vsxxx_mouse_closure = {
    0,
    MS_VSXXX,
    3
};

extern ws_hardware_type mouse_closure;

/* FIXME FIXME - NULL function pointers should point to do_nothing routine */
extern void scc_enable_pointer(); /* to enable pointer interrupts */

char vsxxxaa_name[] = "VSXXXAA";
char vsxxxab_name[] = "VSXXXAB";

ws_pointer vsxxx_mouse = {
    MS_VSXXX,				/* hardware type */
    2,					/* 2 axis */
    (caddr_t)&mouse_closure,		/* private data */
    vsxxxaa_name,			/* name */
    NULL,				/* init_closure */
    NULL,				/* ioctl */
    vsxxx_mouse_init,			/* init_pointer */
    NULL,				/* reset_pointer */
    scc_enable_pointer,			/* enable_pointer */
    NULL,				/* disable_pointer */
    NULL,				/* set_pointer_mode */
    NULL,				/* get_pointer_info */
    NULL,				/* get_position_report */
    vsxxx_tablet_event,			/* do_tablet_event */
    vsxxx_mouse_event,			/* do_mouse_event */
    vsxxx_set_tablet_overhang,		/* set_tablet_overhang */
    0,
    { 1, 1, 1, 1 },			/* pointer rates */
    { 0, 0, 0 },			/* current cursor position */
    { DEFAULT_CURSOR },			/* cursor data */
    { 0, 0, 0, { 0, 0, 0, 0 },},	/* no entries in q for motion	*/
					/* inside this box.		*/
    { 0, 0, 0, { 864, 1024, 0, 0}},	/* prevent cursor from leaving	*/
    0, 0,				/* tablet scale x,y */
    0,					/* tablet_overhang: default (0) none */
					/* means no multiscreen */
    2200, 2200,				/* tablet max x,y */
    0, 0,				/* 0,0 is at bottom left of tablet */
    1					/* login screen is new screen */
};

#endif BINARY
