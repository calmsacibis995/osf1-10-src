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
/* @(#)$RCSfile: magic_data.h,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1993/01/08 16:37:09 $ */

/*
 * OSF/1 Release 1.0
 */
/* Derived from the work
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

struct matcher {
	char *start, *finish;
};

/*
 * External References:
 */
extern char *			fre_comp(char *sp);
extern struct matcher *		fre_exec(char *p1, char *p2);

struct	entry	{
	char	e_level;	/* 0 or 1 */
	int	e_off;		/* in bytes */
	int	e_retcode;	/* major and minor type info to return */
	char	e_type;
	char	e_opcode;
	union	{
		int	num;
		char	*str;
	}	e_value;
	char	*e_str;
};

typedef	struct entry	Entry;

struct	entry_init	{
	char	ei_level;	/* 0 or 1 */
	int	ei_off;		/* in bytes */
	int	ei_retcode;	/* major and minor type info to return */
	char	ei_type;
	char	ei_opcode;
	long	ei_value;
	char	*ei_str;
};

typedef struct entry_init Entry_init;

/*
**	Types
*/

#define	BYTE	0
#define	SHORT	2
#define	LONG	4
#define	STR	8

/*
**	Opcodes
*/

#define	EQ	0
#define	GT	1
#define	LT	2
#define	STRC	3	/* string compare */
#define	ANY	4
#define	SUB	64	/* or'ed in */


#define reg register

extern Entry *mtab;
extern int maxexecfn;
