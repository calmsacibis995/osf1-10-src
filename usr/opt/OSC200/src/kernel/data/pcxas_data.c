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
 * @(#)$RCSfile: pcxas_data.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/07/30 18:28:41 $
 */

#define _PCXAS_DATA_C_

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/workstation.h>
#include <sys/inputdevice.h>
#include <sys/wsdevice.h>
#include <io/common/devio.h>
#include <sys/buf.h>

#include <io/dec/ws/pcxas.h>
#include <io/dec/ws/pcxal.h>

#ifdef BINARY

extern ws_hardware_type pcxas_mouse_closure;
extern ws_pointer pcxas_mouse;

#else /*BINARY*/

/* PCXAS-specific public pointer data */
/* the *real* pointer stuff is copied from here */
/* during console initialization early in boot-up, */
/* by the hardware-specific console init code; */
/* for example, the vsxxx pointer particulars are */
/* copied during scc_cons_init processing on flamingo */

ws_hardware_type pcxas_mouse_closure = {
    0,
    MS_PCXAS,
    3
};

/* FIXME FIXME - temporary entries */
int pcxas_do_nothing_int();
void pcxas_do_nothing_void();
/* FIXME FIXME - temporary entries */

extern ws_hardware_type mouse_closure;

/* FIXME - probably will need a tablet name someday... */
char pcxas_name[] = "PCXAS";

ws_pointer pcxas_mouse = {
    MS_PCXAS,				/* hardware_type */
    2,					/* 2 axis */
    (caddr_t)&mouse_closure,		/* private data */
    pcxas_name,				/* name */
    pcxas_init_closure,			/* init_closure */
    pcxas_do_nothing_int,		/* ioctl */
    pcxas_init_pointer,			/* init_pointer */
    pcxas_reset_pointer,		/* reset_pointer */
    pcxas_enable_pointer,		/* enable_pointer */
    pcxas_disable_pointer,		/* disable_pointer */
    pcxas_do_nothing_void,		/* set_pointer_mode */
    pcxas_do_nothing_void,		/* get_pointer_info */
    pcxas_do_nothing_void,		/* get_position_report */
    pcxas_tablet_event,			/* do_tablet_event */
    pcxas_mouse_event,			/* do_mouse_event */
    pcxas_set_tablet_overhang,		/* set_tablet_overhang */
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
