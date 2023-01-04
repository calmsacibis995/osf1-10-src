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
static char rcsid[] = "@(#)$RCSfile: test.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 16:26:50 $";
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
 */

/*
 *      test expression
 *      [ expression ]
 */

#include	"defs.h"
#include <sys/types.h>
#include <sys/stat.h>

int	ap, ac;
uchar_t	**av;

test(argn, com)
uchar_t	*com[];
int	argn;
{
	ac = argn;
	av = com;
	ap = 1;
	if (eq(com[0],"["))
	{
		if (!eq(com[--ac], "]"))
			failed("test", MSGSTR(M_RBRACKET,"] missing"));
	}
	com[ac] = 0;
	if (ac <= 1)
		return(1);
	return(exp() ? 0 : 1);
}

uchar_t *
nxtarg(mt)
{
	if (ap >= ac)
	{
		if (mt)
		{
			ap++;
			return(0);
		}
		failed("test", MSGSTR(M_ARGEXP,"argument expected"));
	}
	return(av[ap++]);
}

exp()
{
	int	p1;
	uchar_t	*p2;

	p1 = e1();
	p2 = nxtarg(1);
	if (p2 != 0)
	{
		if (eq(p2, "-o"))
			return(p1 | exp());

		if (eq(p2, "]") && !eq(p2, ")"))
			failed("test", MSGSTR(M_SYNMSG,(char *)synmsg));
	}
	ap--;
	return(p1);
}

e1()
{
	int	p1;
	uchar_t	*p2;

	p1 = e2();
	p2 = nxtarg(1);

	if ((p2 != 0) && eq(p2, "-a"))
		return(p1 & e1());
	ap--;
	return(p1);
}

e2()
{
	if (eq(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3()
{
	int	p1;
	register uchar_t	*a;
	uchar_t	*p2;
	long	atol();
	long	int1, int2;

	a = nxtarg(0);
	if (eq(a, "("))
	{
		p1 = exp();
		if (!eq(nxtarg(0), ")"))
			failed("test",MSGSTR(M_RPAREN,") expected"));
		return(p1);
	}
	p2 = nxtarg(1);
	ap--;
	if ((p2 == 0) || (!eq(p2, "=") && !eq(p2, "!=")))
	{
		if (eq(a, "-r"))
			return(tio(nxtarg(0), 4));
		if (eq(a, "-w"))
			return(tio(nxtarg(0), 2));
		if (eq(a, "-x"))
			return(tio(nxtarg(0), 1));
		if (eq(a, "-d"))
			return(filtyp(nxtarg(0), S_IFDIR));
		if (eq(a, "-c"))
			return(filtyp(nxtarg(0), S_IFCHR));
		if (eq(a, "-b"))
			return(filtyp(nxtarg(0), S_IFBLK));
		if (eq(a, "-f"))
			return(filtyp(nxtarg(0), S_IFREG));
		if (eq(a, "-e")) {
			struct stat statb;
		    	return( lstat((char *)nxtarg(0), &statb) == 0 ? 1:0 );
		}
		if (eq(a, "-u"))
			return(ftype(nxtarg(0), S_ISUID));
		if (eq(a, "-g"))
			return(ftype(nxtarg(0), S_ISGID));
		if (eq(a, "-k"))
			return(ftype(nxtarg(0), S_ISVTX));
		if (eq(a, "-p"))
			return(filtyp(nxtarg(0),S_IFIFO));
   		if (eq(a, "-s"))
			return(fsizep(nxtarg(0)));
		if (eq(a, "-t"))
		{
			if (ap >= ac)		/* no args */
				return(isatty(1));
			else if (eq((a = nxtarg(0)), "-a") || eq(a, "-o"))
			{
				ap--;
				return(isatty(1));
			}
			else
				return(isatty(atoi((char *)a)));
		}
                if (eq(a, "-L"))
                {
                        struct stat statb;
                        if (lstat((char *)nxtarg(0), &statb) < 0)
                                return(0);
                        return((statb.st_mode&S_IFMT)==S_IFLNK);
                }
		if (eq(a, "-n"))
			return(!eq(nxtarg(0), ""));
		if (eq(a, "-z"))
			return(eq(nxtarg(0), ""));
	}

	p2 = nxtarg(1);
	if (p2 == 0)
		return(!eq(a, ""));
	if (eq(p2, "-a") || eq(p2, "-o"))
	{
		ap--;
		return(!eq(a, ""));
	}
	if (eq(p2, "="))
		return(eq(nxtarg(0), a));
	if (eq(p2, "!="))
		return(!eq(nxtarg(0), a));
	int1 = atol((char *)a);
	int2 = atol((char *)nxtarg(0));
	if (eq(p2, "-eq"))
		return(int1 == int2);
	if (eq(p2, "-ne"))
		return(int1 != int2);
	if (eq(p2, "-gt"))
		return(int1 > int2);
	if (eq(p2, "-lt"))
		return(int1 < int2);
	if (eq(p2, "-ge"))
		return(int1 >= int2);
	if (eq(p2, "-le"))
		return(int1 <= int2);

	bfailed(btest, MSGSTR(M_BADOP,(char *)badop), p2);
/* NOTREACHED */
}

tio(a, f)
uchar_t	*a;
int	f;
{
	if (access(a, f) == 0)
		return(1);
	else
		return(0);
}

ftype(f, field)
uchar_t	*f;
int	field;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	if ((statb.st_mode & field) == field)
		return(1);
	return(0);
}

filtyp(f,field)
uchar_t	*f;
int field;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	if ((statb.st_mode & S_IFMT) == field)
		return(1);
	else
		return(0);
}



fsizep(f)
uchar_t	*f;
{
	struct stat statb;

	if (stat((char *)f, &statb) < 0)
		return(0);
	return(statb.st_size > 0);
}

/*
 * fake diagnostics to continue to look like original
 * test(1) diagnostics
 */
bfailed(s1, s2, s3) 
uchar_t	*s1;
uchar_t	*s2;
uchar_t	*s3;
{
	prp();
	prs(s1);
	if (s2)
	{
		prs(colon);
		prs(s2);
		prs(s3);
	}
	newline();
	exitsh(ERROR);
}
