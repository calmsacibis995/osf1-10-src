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
 *	@(#)$RCSfile: vsxxx.h,v $ $Revision: 1.2.9.2 $ (DEC) $Date: 1993/06/24 22:46:10 $
 */ 
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
#ifndef _VSXXX_H_
#define _VSXXX_H_

#define EVENT_LEFT_BUTTON	0x01
#define EVENT_MIDDLE_BUTTON	0x02
#define EVENT_RIGHT_BUTTON	0x03

#define EVENT_T_LEFT_BUTTON	0x01
#define EVENT_T_FRONT_BUTTON	0x02
#define EVENT_T_RIGHT_BUTTON	0x03
#define EVENT_T_BACK_BUTTON	0x04

/* puck buttons */

#define T_LEFT_BUTTON		0x02
#define T_FRONT_BUTTON		0x04
#define T_RIGHT_BUTTON		0x08
#define T_BACK_BUTTON		0x10

/* Mouse definitions */
#define MOTION_BUFFER_SIZE 100
#define SELF_TEST	'T'
#define INCREMENTAL	'R'
#define PROMPT		'D'

#define TABLET_NOPTR    0x13
#define TABLET_STYLUS   0x11
#define TABLET_PUCK     0x00


/* NOTE: these are the same as WSPR_XXX_BUTTON in sys/wsdevice.h */
#define VSXXX_RIGHT_BUTTON	0x01
#define VSXXX_MIDDLE_BUTTON	0x02
#define VSXXX_LEFT_BUTTON	0x04

#define VSXXX_X_SIGN	0x10		/* sign bit for X		*/
#define VSXXX_Y_SIGN	0x08		/* sign bit for Y		*/

#define START_FRAME	0x80		/* start of report frame bit	*/

#define UPDATE_POS	0x01

#ifdef FIXME
/* FIXME FIXME should be able to remove this 'cuz of ws_pointer_report?? */
/* Mouse report structure definition */
struct mouse_report {
	char state;			/* buttons and sign bits	*/
	short dx;			/* delta X since last change	*/
	short dy;			/* delta Y since last change	*/
	char bytcnt;			/* mouse report byte count	*/
};
#endif /* FIXME */

#ifdef KERNEL
int vsxxx_mouse_init();
int vsxxx_tablet_event();
int vsxxx_mouse_event();
void vsxxx_set_tablet_overhang();
#endif /* KERNEL */
#endif /*_VSXXX_H_*/
