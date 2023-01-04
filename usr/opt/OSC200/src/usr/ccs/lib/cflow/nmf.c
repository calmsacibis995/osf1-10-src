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
static char	*sccsid = "@(#)$RCSfile: nmf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:41:10 $";
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
#ifndef lint
#ifndef _NOIDENT

#endif
#endif


/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <locale.h>

#define SYMSTART 11
#define SYMCLASS 9

extern void exit();
extern char *strcpy(), *strncpy();

main(argc, argv)
char	*argv[];
{
	char name[BUFSIZ], buf[BUFSIZ], *fname = NULL, *pty;
	register char *p;
	int nsize, tysize, lineno;

	setlocale(LC_ALL, "");
	strcpy(name, argc > 1? argv[1] : "???");
	if (argc > 2)
		fname = argv[2];
	else
		fname = "???";
	while (gets(buf))
	{
		p = buf;
		while (*p != ' ' && *p != '|')
			++p;
		nsize = p - buf;
		do ; while (*p++ != '|');		/* skip rem of name */
		do ; while (*p++ != '|');		/* skip value */
		do ; while (*p++ != '|');		/* skip class */
		while (*p == ' ')
			++p;
		if (*p != '|')
		{
			pty = p++;
			while (*p != '|')
				++p;
			tysize = p - pty;
		}
		else
			pty = (char *) NULL;
		++p;
		do ; while (*p++ != '|');		/* skip size */
		while (*p == ' ')
			++p;
		lineno = atoi(p);
		do ; while (*p++ != '|');		/* and xlated line */
		while (*p == ' ')
			++p;
		if (!strncmp(p, ".text", 5) || !strncmp(p, ".data", 5))
		{					/* it's defined */
			strncpy(name, buf, nsize);
			name[nsize] = '\0';
			printf("%s = ", name);
			if (pty)
				printf("%.*s", tysize, pty);
			else
			{
				fputs("???", stdout);
				if (!strncmp(p, ".text", 5))
					fputs("()", stdout);
			}
			printf(", <%s %d>\n", fname, lineno);
		}
		else
			printf("%s : %.*s\n", name, nsize, buf);
	}
	exit(0);
	/*NOTREACHED*/
}
