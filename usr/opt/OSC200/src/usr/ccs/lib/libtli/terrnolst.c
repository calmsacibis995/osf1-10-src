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
static char *rcsid = "@(#)$RCSfile: terrnolst.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/05/12 16:25:39 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#ifdef	XTI
#include <xti.h>
#else
#include <tiuser.h>
#endif
#include <errno.h>

char	* t_errlist[] = {
	"No error specified",				/* 0  no error	*/
	"Incorrect address format",			/* 1  TBADDADDR	*/
	"Incorrect option format",			/* 2  TBADOPT	*/
	"Incorrect permissions",			/* 3  TACCESS	*/
	"Illegal transport fd",				/* 4  TBADF	*/
	"Could not allocate address",			/* 5  TNOADDR	*/
	"Out of state",					/* 6  TOUTSTATE	*/
	"Bad call sequence number",			/* 7  TBADSEQ	*/
	"System error",					/* 8  TSYSERR	*/
	"Event requires attention",			/* 9  TLOOK	*/
	"Illegal amount of data",			/* 10 TBADDATA	*/
	"Buffer not large enough",			/* 11 TBUFOVFLOW*/
	"Flow control",					/* 12 TFLOW	*/
	"No data",					/* 13 TNODATA	*/
	"No disconnect indication available",		/* 14 TNODIS	*/
	"No unitdata error indication available",	/* 15 TNOUDERR	*/
	"Bad flags",					/* 16 TBADFLAG	*/
	"No orderly release indication available",	/* 17 TNOREL	*/
	"Primitive not supported",			/* 18 TNOTSUPPORT */
	"State is in process of changing",		/* 19 TSTATECHNG*/
#ifdef XTI
	"Unsupported struct-type requested",		/* 20 TNOSTRUCTYPE */
	"Invalid transport provider name",		/* 21 TBADNAME	*/
	"Qlen is zero",					/* 22 TBADQLEN	*/
	"Address in use"				/* 23 TADDRBUSY	*/
/* XPG4 */
	"Outstanding connection indications",		/* 24 TINDOUT	*/
	"Transport provider mismatch",			/* 25 TPROVMISMATCH */
	"Resfd specified to accept with qlen > 0",	/* 26 TRESQLEN	*/
	"Resfd not bound to same address as fd",		/* 27 TRESADDR	*/
	"Incoming connection queue is full",		/* 28 TQFULL	*/
	"XTI protocol error"				/* 29 TPROTO	*/
#endif
};

int	t_nerr = sizeof(t_errlist) / sizeof(t_errlist[0]);
