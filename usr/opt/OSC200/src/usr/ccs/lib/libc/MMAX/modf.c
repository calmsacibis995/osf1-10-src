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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: modf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:03:40 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* @(#)modf.c	3.1 */
/*  */
/* 2/26/91 */
/*LINTLIBRARY*/
/*
 * $ Header: modf.c,v 10.1 87/11/04 09:58:43 mbj Exp $
 *	replaces System 5.2 version
 *
 * The function
 *	modf(value, iptr)
 * returns the signed fractional part of value and stores the (signed) integer part
 *	indirectly through iptr.
 *
 * Implementation note:
 *	This is a non-portable implementation which works for IEEE floating point only.
 */

#include <nan.h>
#include <values.h>

#define	FRACTSIZE	52
#define	EXPBIAS		1023

static long himask[] =
	{
	0xFFF00000,
	0xFFF80000,
	0xFFFC0000,
	0xFFFE0000,
	0xFFFF0000,
	0xFFFF8000,
	0xFFFFC000,
	0xFFFFE000,
	0xFFFFF000,
	0xFFFFF800,
	0xFFFFFC00,
	0xFFFFFE00,
	0xFFFFFF00,
	0xFFFFFF80,
	0xFFFFFFC0,
	0xFFFFFFE0,
	0xFFFFFFF0,
	0xFFFFFFF8,
	0xFFFFFFFC,
	0xFFFFFFFE,
	0xFFFFFFFF,
	};


static long lomask[] =
	{
	0x00000000,
	0x80000000,
	0xC0000000,
	0xE0000000,
	0xF0000000,
	0xF8000000,
	0xFC000000,
	0xFE000000,
	0xFF000000,
	0xFF800000,
	0xFFC00000,
	0xFFE00000,
	0xFFF00000,
	0xFFF80000,
	0xFFFC0000,
	0xFFFE0000,
	0xFFFF0000,
	0xFFFF8000,
	0xFFFFC000,
	0xFFFFE000,
	0xFFFFF000,
	0xFFFFF800,
	0xFFFFFC00,
	0xFFFFFE00,
	0xFFFFFF00,
	0xFFFFFF80,
	0xFFFFFFC0,
	0xFFFFFFE0,
	0xFFFFFFF0,
	0xFFFFFFF8,
	0xFFFFFFFC,
	0xFFFFFFFE,
	0xFFFFFFFF
	};

double modf(value, iptr)
	double value; /* don't declare register, because of KILLNaN! */
	register double *iptr;
	{
	register double fraction;
	register int exp;

	KILLNaN(value); /* raise exception on Not-a-Number (3b only) */
	exp = ((union __NaN *)&value)->ds.de - EXPBIAS;
	if (exp >= FRACTSIZE)
		{
		*iptr = value;
		fraction = 0;
		}
	else if (exp < 0)
		{
		*iptr = 0;
		fraction = value;
		}
	else
		{
		*iptr = value;
		if (exp < 20)
			{
			((union __NaN *)iptr)->ds.dflow = 0;
			((union __NaN *)iptr)->ds.dfhi &= himask[exp];
			}
		else
			((union __NaN *)iptr)->ds.dflow &= lomask[exp - 20];
		fraction = value - *iptr;
		}
	return (fraction);
	}
