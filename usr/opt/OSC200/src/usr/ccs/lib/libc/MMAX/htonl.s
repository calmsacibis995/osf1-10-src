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
 *	@(#)$RCSfile: htonl.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:02:41 $
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
/* htonl:	Convert host long int to network long int
 * Calling sequence:	netint = htonl(hostint);
 * Returns: long int in net byte order
 */
#include "SYS.h"

	.file	"htonl.s"
	.text
	.align	1
ENTRY(htonl)
	movb	SP(4),r0		# Reorder bytes in r0 as ret val
	lshw	$8,r0
	movb	SP(5),r0
	lshd	$8,r0
	movb	SP(6),r0
	lshd	$8,r0
	movb	SP(7),r0
	EXIT
	ret	$0
