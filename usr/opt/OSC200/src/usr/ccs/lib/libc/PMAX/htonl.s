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
 *	@(#)$RCSfile: htonl.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:15:00 $
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

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * htonl -- host to network byte order for longs
 */

#include <mips/regdef.h>
#include <mips/asm.h>

LEAF(htonl)
#ifdef MIPSEL
	# start with abcd
	sll	v0,a0,24	# d000
	srl	v1,a0,24	# 000a
	or	v0,v1		# d00a
	srl	t0,a0,8		# 0abc
	and	t1,t0,0xff00	# 00b0
	and	v1,t0,0xff	# 000c
	sll	v1,16		# 0c00
	or	v1,t1		# 0cb0
	or	v0,v1		# dcba
#else
	move	v0,a0
#endif
	j	ra
.end htonl
