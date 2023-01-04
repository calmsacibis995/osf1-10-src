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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: ctype.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:14:12 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.9  com/cmd/sh/sh/ctype.c, cmdsh, bos320, 9125320 6/6/91 23:10:13
 */

#include	"defs.h"

uchar_t	_ctype1[] =
{
/*	000	001	002	003	004	005	006	007	*/
	_EOF,	0,	0,	0,	0,	0,	0,	0,

/*	bs	ht	nl	vt	np	cr	so	si	*/
	0,	_TAB,	_EOR,	0,	0,	0,	0,	0,

	0,	0,	0,	0,	0,	0,	0,	0,

	0,      0,      0,      0,      0,      0,      0,      0,

/*	sp	!	"	#	$	%	&	'	*/
	_SPC,	0,	_DQU,	0,	_DOL1,	0,	_AMP,	0,

/*	(	)	*	+	,	-	.	/	*/
	_BRA,	_KET,	0,	0,	0,	0,	0,	0,

/*	0	1	2	3	4	5	6	7	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	8	9	:	;	<	=	>	?	*/
	0,	0,	0,	_SEM,	_LT,	0,	_GT,	0,

/*	@	A	B	C	D	E	F	G	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	H	I	J	K	L	M	N	O	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	P	Q	R	S	T	U	V	W	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	X	Y	Z	[	\	]	^	_	*/
	0,	0,	0,	0,	_BSL,	0,	_HAT,	0,

/*	`	a	b	c	d	e	f	g	*/
	_LQU,	0,	0,	0,	0,	0,	0,	0,

/*	h	i	j	k	l	m	n	o	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	p	q	r	s	t	u	v	w	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	x	y	z	{	|	}	~	del	*/
	0,	0,	0,	0,	_BAR,	0,	0,	0
};


uchar_t	_ctype2[] =
{
/*	000	001	002	003	004	005	006	007	*/
	0,	0,	0,	0,	0,	0,	0,	0,

/*	bs	ht	nl	vt	np	cr	so	si	*/
	0,	0,	0,	0,	0,	0,	0,	0,

	0,	0,	0,	0,	0,	0,	0,	0,

#ifdef _SBCS
	0,	0,	0,	0,	_FSH,	_FSH,	_FSH,	_FSH,
#else
	0,	0,	0,	0,	0,	0,	0,	0,
#endif

/*	sp	!	"	#	$	%	&	'	*/
	0,	_PCS,	0,	_NUM,	_DOL2,	0,	0,	0,

/*	(	)	*	+	,	-	.	/	*/
	0,	0,	_AST,	_PLS,	0,	_MIN,	0,	0,

/*	0	1	2	3	4	5	6	7	*/
	_DIG,	_DIG,	_DIG,	_DIG,	_DIG,	_DIG,	_DIG,	_DIG,

/*	8	9	:	;	<	=	>	?	*/
	_DIG,	_DIG,	0,	0,	0,	_EQ,	0,	_QU,

/*	@	A	B	C	D	E	F	G	*/
	_AT,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,

/*	H	I	J	K	L	M	N	O	*/
	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,

/*	P	Q	R	S	T	U	V	W	*/
	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,	_UPC,

/*	X	Y	Z	[	\	]	^	_	*/
	_UPC,	_UPC,	_UPC,	0,	0,	0,	0,	_UPC,

/*	`	a	b	c	d	e	f	g	*/
	0,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,

/*	h	i	j	k	l	m	n	o	*/
	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,

/*	p	q	r	s	t	u	v	w	*/
	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,	_LPC,

/*	x	y	z	{	|	}	~	del	*/
	_LPC,	_LPC,	_LPC,	_CBR,	0,	_CKT,	0,	0
};
