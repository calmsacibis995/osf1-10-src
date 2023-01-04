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
 * @(#)$RCSfile: byte_swap.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/16 12:30:00 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * ntohs and ntohl -- byte swap short and long
 */
LEAF(byteswap_short)
#if	MIPSEL
XLEAF(ntohs)			# a0 = ??ab
XLEAF(htons)
#endif
	srl	v0,a0,8		# v0 = 0??a
	and	v0,0xff		# v0 = 000a
	sll	v1,a0,8		# v1 = ?ab0
	or	v0,v1		# v0 = ?aba
	and	v0,0xffff	# v0 = 00ba
	j	ra
	END(ntohs)

LEAF(byteswap_long)
#if	MIPSEL
XLEAF(ntohl)			# a0 = abcd
XLEAF(htonl)
#endif
	sll	v0,a0,24	# v0 = d000
	srl	v1,a0,24	# v1 = 000a
	or	v0,v0,v1	# v0 = d00a
	and	v1,a0,0xff00	# v1 = 00c0
	sll	v1,v1,8		# v1 = 0c00
	or	v0,v0,v1	# v0 = dc0a
	srl	v1,a0,8		# v1 = 0abc
	and	v1,v1,0xff00	# v1 = 00b0
	or	v0,v0,v1	# v0 = dcba
	j	ra
	END(ntohl)

#if	MIPSEB
/* Placeholders, just in case */
LEAF(ntohs)
XLEAF(htons)
XLEAF(ntohl)
XLEAF(htonl)
	move	v0,a0
	j	ra
	END(ntohs)
#endif


