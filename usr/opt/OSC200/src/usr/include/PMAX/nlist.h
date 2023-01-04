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
 *       @(#)$RCSfile: nlist.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/28 20:00:11 $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/nlist.h,v 4.2.4.2 1992/01/28 20:00:11 Al_Delorey Exp $ */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NLIST_H
#define _NLIST_H

#if __vax || __u3b || _M32 || __u3b15 || __u3b5 || __u3b2
/* symbol table entry structure */
struct nlist
{
	char		*n_name;	/* symbol name */
	long		n_value;	/* value of symbol */
	short		n_scnum;	/* section number */
	unsigned short	n_type;		/* type and derived type */
	char		n_sclass;	/* storage class */
	char		n_numaux;	/* number of aux. entries */
};
#endif

#if __pdp11 || __u370
struct nlist
{
	char		n_name[8];	/* symbol name */
#if __pdp11
	int		n_type;		/* type of symbol in .o */
#else
	char		n_type;
	char		n_other;
	short		n_desc;
#endif
	unsigned	n_value;	/* value of symbol */
};
#endif

#if __mips__

#ifdef __osf__
#define N_UNDF  0x0             /* undefined */
#endif

struct nlist {
    char *n_name;
    unsigned long n_value;
    short n_type;		/* 0 if not there, 1 if found */
    short reserved;
};
#endif	/* __mips__ */

#endif	/* _NLIST_H */
