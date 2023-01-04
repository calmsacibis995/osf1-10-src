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
 *	@(#)$RCSfile: ttyloc.h,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 02:12:53 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 *
 */

#ifndef	_SYS_TTYLOC_H_
#define _SYS_TTYLOC_H_

struct ttyloc
{
    int tlc_hostid;		/* host identifier of location (on Internet) */
    int tlc_ttyid;		/* terminal identifier of location (on host) */
};

/*
 *  Pseudo host location of Front Ends
 */
#define TLC_FEHOST	((128<<24)|(2<<16)|(254<<8)|255)
#define TLC_SPECHOST	((0<<24)|(0<<16)|(0<<8)|0)

/*
 *  Pseudo terminal index of console
 */
#define TLC_CONSOLE	((unsigned short)0177777)
#define TLC_DISPLAY	((unsigned short)0177776)

/*
 *  Constants
 */
#define TLC_UNKHOST	((long)(0))
#define TLC_UNKTTY	((long)(-1))
#define TLC_DETACH	((long)(-2))
#define TLC_TACTTY	((long)(-3))		/* unused by kernel */
#define TLC_RANDOMTTY	((long)(-4))		/* unused by kernel */

#ifdef	KERNEL
#include <sys/kernel.h>		/* for hostid */

/*
 *  IP address of local host (as determined by the network module)
 */
#define TLC_MYHOST	(hostid)

#endif	/* KERNEL */
#endif	/* _SYS_TTYLOC_H_ */
