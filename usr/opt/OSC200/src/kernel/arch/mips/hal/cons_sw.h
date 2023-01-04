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
 *	@(#)$RCSfile: cons_sw.h,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/14 16:05:26 $
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
/* 
 * derived from cons_sw.h	2.2	(ULTRIX)	11/13/89
 */


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
        int     (*c_mmap)();            /* console mmap routine         */
	struct tty *ttys;		/* tty structure for select	*/
};
