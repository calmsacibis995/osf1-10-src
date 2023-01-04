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
 *	"@(#)cons_sw.h	9.2	(ULTRIX/OSF)	11/5/91"
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

#ifndef _CONS_SW_H_
#define _CONS_SW_H_

/* forward declaration required for C++ */
#ifdef __cplusplus
struct tty;
#endif

struct	cons_sw {
        int	system_type;		/* system ID			*/
	int	(*c_open)();		/* console open			*/
	int	(*c_close)();		/* console close		*/
	int	(*c_read)();		/* console read			*/
	int	(*c_write)();		/* console write		*/
	int	(*c_ioctl)();		/* console ioctl		*/
	int	(*c_rint)();		/* console receiver int		*/
	int	(*c_xint)();		/* console transmitter int	*/
	int	(*c_start)();		/* console start		*/
	int	(*c_stop)();		/* console stop			*/
	int	(*c_select)();		/* console select		*/
	int	(*c_putc)();		/* console putc			*/
	int	(*c_probe)();		/* console probe		*/
	int	(*c_getc)();		/* console getc			*/
	int	(*c_init)();		/* console init routine		*/
	int	(*c_param)();		/* console parameter routine	*/
	int	(*c_mmap)();		/* console mmap routine		*/
	struct tty *ttys;		/* tty structure for select	*/
};

struct vcons_init_sw {
        char	modname[9];		/* TURBOchannel module name	*/
	int	(*cons_init)();		/* virtual console init routine */
};

#endif
