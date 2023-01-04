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
static char	*sccsid = "@(#)$RCSfile: inline_machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:08 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#include <stdio.h>
#include <ctype.h>

#include <inline/inline.h>

extern char *strcpy();
extern char *strcat();
extern char *index();

/*
 * The routines and tables in this file must be rewritten
 * for each new machine that this program is ported to.
 */
extern struct pats mmax_ptab[];
extern struct pats ns32000_ptab[];
extern struct pats unix_ptab[];
extern struct pats mach_ptab[];

struct pats *inittables[] = {
	ns32000_ptab,
	mmax_ptab,
	unix_ptab,
	mach_ptab,
	0
};

struct pats *subset_inittables[] = {
	ns32000_ptab,
	mmax_ptab,
	unix_ptab,
	mach_ptab,
	0
};

/*
 * Instruction stop table.
 * All instructions that implicitly modify any of the temporary
 * registers, change control flow, or implicitly loop must be
 * listed in this table. It is used to find the end of a basic
 * block when scanning backwards through the instruction stream
 * trying to merge the inline expansion.
 */
struct inststoptbl inststoptable[] = {
	{ "beq" }, { "bne" }, { "bcs" }, { "bcc" }, { "bhi" },
	{ "bls" }, { "bgt" }, { "ble" }, { "bfs" }, { "bfc" },
	{ "blo" }, { "bhs" }, { "blt" }, { "bge" },

	{ "br" }, { "jump" },  { "caseb" }, { "casew" }, { "cased" },
	{ "bsr" }, { "jsr" }, { "ret" }, { "rett" }, { "reti" },

	{ "cmpsb" }, { "cmpsw" }, { "cmpsd" }, { "cmpst" },
	{ "movsb" }, { "movsw" }, { "movsd" }, { "movst" },
	{ "skpsb" }, { "skpsw" }, { "skpsd" }, { "skpst" },

	{ "acbb" }, { "acbw" }, { "acbd" },
	{ "enter" }, { "exit" }, { "save" }, { "restore" },

	{ "cxp"}, { "cxpd"}, { "rxp"}, { "dei"}, { "mei"},
	{ "" }
};

/*
 * Check to see if a line is a candidate for replacement.
 * Return pointer to name to be looked up in pattern table.
 */
char *
doreplaceon(cp)
	char *cp;
{

	if (cp[3] != '\t' && cp[3] != ' ')
		return (0);

	if (bcmp(cp, "bsr", 3) != 0 && bcmp(cp, "jsr", 3) != 0)
		return (0);
	/*
	 *	Skip the leading identifier character.
	 */

	for (cp += 3; *cp == ' ' || *cp == '\t' || *cp == '@' || *cp == '?'; cp++)
		;
	return (cp);
}

/*
 * Find out how many arguments the function is being called with.
 * A return value of -1 indicates that the count can't be determined.
 */
int
countargs(cp)
	char *cp;
{
	return (-1);
}

/*
 * Find the next argument to the function being expanded.
 */
nextarg(argc, argv)
	int argc;
	char *argv[];
{
	register char *lastarg = argv[2];
#ifdef	DEBUG
	register int i;
	printf("nextarg: %d\n", argc);
	for (i=0; i< argc; printf("#%s# ", argv[i++]));
	printf("\n");
#endif	DEBUG
	if (argc == 3 &&
	    bcmp(argv[0], "movd", 4) == 0 &&
	    bcmp(argv[1], "tos", 3) == 0 &&
	    lastarg[0] == 'r' && isdigit(lastarg[1]) && lastarg[2] == '\0')
		return (lastarg[1] - '0');
	return (-1);
}

/*
 * Determine whether the current line pushes an argument.
 */
ispusharg(argc, argv)
	int argc;
	char *argv[];
{
#ifdef	DEBUG
	register int i;
	printf("ispusharg: %d\n", argc);
	for (i=0; i< argc; printf("#%s# ", argv[i++]));
	printf("\n");
#endif	DEBUG
	if (argc < 2)
		return (0);
	if (bcmp(argv[argc - 1], "tos", 3) == 0)
		return (1);
	return (0);
}

/*
 * Determine which (if any) registers are modified
 * Return register number that is modified, -1 if none are modified.
 */
modifies(argc, argv)
	int argc;
	char *argv[];
{
	/*
	 * For the Sequent Balance and Encore Multimax
	 * all we care about are r0 to r2
	 */
	register char *lastarg = argv[argc - 1];
#ifdef	DEBUG
	register int i;
	printf("modifies: %d\n", argc);
	for (i=0; i< argc; printf("#%s# ", argv[i++]));
	printf("\n");
#endif	DEBUG
	if (lastarg[0] == 'r' && isdigit(lastarg[1]) && lastarg[2] == '\0')
		return (lastarg[1] - '0');
	return (-1);
}

/*
 * Rewrite the instruction in (argc, argv) to store its
 * contents into arg instead of onto the stack. The new
 * instruction is placed in the buffer that is provided.
 */
rewrite(instbuf, argc, argv, target)
	char *instbuf;
	int argc;
	char *argv[];
	int target;
{

	switch (argc) {
	case 0:
		instbuf[0] = '\0';
		fprintf(stderr, "blank line to rewrite?\n");
		return;
	case 1:
		sprintf(instbuf, "\t%s\n", argv[0]);
		fprintf(stderr, "rewrite?-> %s", instbuf);
		return;
	case 2:
		sprintf(instbuf, "\t%s\tr%d\n", argv[0], target);
		return;
	case 3:
		sprintf(instbuf, "\t%s\t%s,r%d\n", argv[0], argv[1], target);
		return;
	case 4:
		sprintf(instbuf, "\t%s\t%s,%s,r%d\n",
			argv[0], argv[1], argv[2], target);
		return;
	case 5:
		sprintf(instbuf, "\t%s\t%s,%s,%s,r%d\n",
			argv[0], argv[1], argv[2], argv[3], target);
		return;
	default:
		sprintf(instbuf, "\t%s\t%s", argv[0], argv[1]);
		argc -= 2, argv += 2;
		while (argc-- > 0) {
			(void) strcat(instbuf, ",");
			(void) strcat(instbuf, *argv++);
		}
		(void) strcat(instbuf, "\n");
		fprintf(stderr, "rewrite?-> %s", instbuf);
		return;
	}
}

/*
 * Do any necessary post expansion cleanup.
 */
/*ARGSUSED*/
cleanup(numargs)
	int numargs;
{
	register char *cp = line[bufhead], *lp;

	fgets(line[bufhead], MAXLINELEN, stdin);

	if (lp = index(cp, LABELCHAR))
		cp = lp + 1;
	for (; isspace(*cp); cp++) ;
	
	if (numargs > 2 && !bcmp(cp, "adjspb", 6) )
		pre_read = 0;
	else
		pre_read = 1;
	return;
}
