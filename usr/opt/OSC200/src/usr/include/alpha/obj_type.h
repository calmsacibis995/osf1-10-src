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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/alpha/obj_type.h,v 1.1.2.3 1992/12/08 18:13:19 Don_Anderson Exp $ */

#ifndef _OBJ_TYPE_H
#define _OBJ_TYPE_H

#include "obj.h"

typedef struct obj_type {
	struct obj	*obj;
	unsigned long	init;		/* whether to init with this type */
	long		type;		/* index to aux we are processing */

	/* the rest are fields we fill in */
	TIR		ti;
	unsigned long	skip;		/* how many tq's & bt to ignore */
	long		bt_symbol;
	long		bt_type;
	unsigned long	bt_low;		/* for range */
	unsigned long	bt_high;	/* for range */
	unsigned long	tq;
	unsigned long	tq_index;	/* ongoing index to next aux */
	unsigned long   tq_save;
	long		symbol;
	unsigned long	low;
	unsigned long	high;
	unsigned long	stride;
	unsigned long	width;
	unsigned long	dimension;
	unsigned long	dimcount;
	unsigned long	offset;
	unsigned long	precision;
	unsigned long	isproc;
	long 		nodesym;
	unsigned int	language;
} OBJ_TYPE, *pOBJ_TYPE;
#define cbOBJ_TYPE (sizeof(OBJ_TYPE))

extern AUXU usertype[];
extern int tinx;

/*

basic type represented in a size other than the def size:
	tq		tqNil
	bt		bt whatever
	width		number of bits to represent the type

basic type:
	tq		tqNil
	bt		bt whatever
	width		0

Pointer:
	tq		tqPtr

Function returning:
	tq		tqFunc

Array:
	tq		tqArray
	type		rindex to subscript's type TIR
	low		low index
	high		high index
	stride		stride in bits

Set:
Indirect
	bt		btSet or btIndirect
	type		type index to tir of set or indirect type.

Enumerated type:
	bt		btEnum
	symbol		symbol index to enum block

Range:
	bt		btRange
	type		type index to TIR to type we are subranging
	low		low range value
	high		high range value

C union:
Structures:
	bt		btUnion or btStruct
	symbol		symbol index to block defining union or struct

basic type is a Typedef:
	bt		btTypedef
	symbol		symbol index to typedef symbol

FORTRAN_common:
FORTRAN_equivalence:
	bt		btStruct
	symbol		symbol index to block defining struct

*/


#endif /* _OBJ_TYPE_H */
