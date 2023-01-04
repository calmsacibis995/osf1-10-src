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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:22:49 $";
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
/*
 * Copyright (c) 1984, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef	lint
char copyright[] =
"@(#) Copyright (c) 1984, 1986 Regents of the University of California.\n\
 All rights reserved.\n";
#endif	not lint

#define	CMU	1	/* OSF */

#include <stdio.h>
#include <ctype.h>
#if	CMU
#include <inline/inline.h>
#else
#include "inline.h"
#endif

/*
 * These are the pattern tables to be loaded
 */
#ifdef	CMU
	/* tables are now defined in machdep.c */
extern struct pats *inittables[];
extern struct pats *subset_inittables[];
#else	/* !CMU */
struct pats *vax_inittables[] = {
	language_ptab,
	libc_ptab,
	vax_libc_ptab,
	machine_ptab,
	vax_ptab,
	0
};

struct pats *vaxsubset_inittables[] = {
	language_ptab,
	libc_ptab,
	vaxsubset_libc_ptab,
	machine_ptab,
	vaxsubset_ptab,
	0
};
#endif	/* !CMU */
/*
 * Statistics collection
 */
struct stats {
	int	attempted;	/* number of expansion attempts */
	int	finished;	/* expansions done before end of basic block */
	int	lostmodified;	/* mergers inhibited by intervening mod */
	int	savedpush;	/* successful push/pop merger */
} stats;

extern char *strcpy();

char *whoami;
int lineno = 0;
int dflag;
#ifdef	CMU
int	print_warnings = 0;
#endif

main(argc, argv)
	int argc;
	char *argv[];
{
	register char *cp, *lp;
	register char *bufp;
	register struct pats *pp, **php;
	struct pats **tablep;
	register struct inststoptbl *itp, **ithp;
	int size;
	extern char *index();
	int subset = 0;

	whoami = argv[0];
	argc--;
	argv++;
	while (argc > 0 && argv[0][0] == '-') {
		switch(argv[0][1]) {

		case 's':
			subset++;
			break;

		case 'd':
			dflag++;
			break;

#ifdef	CMU
		case 'w':
			print_warnings = 1;
			break;
#endif
		default:
			break;
		}
		argc--, argv++;
	}
	if (argc > 0)
		freopen(argv[0], "r", stdin);
	if (argc > 1)
		freopen(argv[1], "w", stdout);
	/*
	 * Set up the hash table for the patterns.
	 */
#ifdef	CMU
	if (subset)
		tablep = subset_inittables;
	else
		tablep = inittables;
#else
	if (subset)
		tablep = vaxsubset_inittables;
	else
		tablep = vax_inittables;
#endif
	for ( ; *tablep; tablep++) {
		for (pp = *tablep; pp->name[0] != '\0'; pp++) {
			php = &patshdr[hash(pp->name, &size)];
			pp->size = size;
			pp->next = *php;
			*php = pp;
		}
	}
	/*
	 * Set up the hash table for the instruction stop table.
	 */
	for (itp = inststoptable; itp->name[0] != '\0'; itp++) {
		ithp = &inststoptblhdr[hash(itp->name, &size)];
		itp->size = size;
		itp->next = *ithp;
		*ithp = itp;
	}
	/*
	 * check each line and replace as appropriate
	 */
	buftail = bufhead = 0;
	bufp = line[0];
#ifdef	CMU
	pre_read = 0;
	while (pre_read || fgets(bufp, MAXLINELEN, stdin)) {
		/*
		 * We had to read the next line after the "call" to
		 * see if it was a adjspb which we'd flush.  If not
		 * then we have pre_read and must process the line.
		 * It could be another call, ...
		 */
		if (pre_read) {
			bufp = line[bufhead];
			pre_read = 0;
		}
#else
	while (fgets(bufp, MAXLINELEN, stdin)) {
#endif
		lineno++;
		lp = index(bufp, LABELCHAR);
		if (lp != NULL) {
			for (cp = bufp; cp < lp; cp++)
				if (!isalnum(*cp))
					break;
			if (cp == lp) {
				bufp = newline();
				if (*++lp == '\n') {
					emptyqueue();
					continue;
				}
				(void) strcpy(bufp, lp);
				*lp++ = '\n';
				*lp = '\0';
				emptyqueue();
			}
		}
		for (cp = bufp; isspace(*cp); cp++)
			/* void */;
		if ((cp = doreplaceon(cp)) == 0) {
			bufp = newline();
			continue;
		}
		for (pp = patshdr[hash(cp, &size)]; pp; pp = pp->next) {
			if (pp->size == size && bcmp(pp->name, cp, size) == 0) {
				if (argcounterr(pp->args, countargs(bufp), pp->name)) {
					pp = NULL;
					break;
				}
				expand(pp->replace);
				bufp = line[bufhead];
				break;
			}
		}
		if (!pp) {
			emptyqueue();
			fputs(bufp, stdout);
		}
	}
	emptyqueue();
	if (dflag)
		fprintf(stderr, "%s: %s %d, %s %d, %s %d, %s %d\n",
			whoami,
			"attempts", stats.attempted,
			"finished", stats.finished,
			"inhibited", stats.lostmodified,
			"merged", stats.savedpush);
	exit(0);
}

/*
 * Integrate an expansion into the assembly stream
 */
expand(replace)
	char *replace;
{
	register int curptr;
	char *nextreplace, *argv[MAXARGS];
	int argc, argreg, foundarg, mod = 0, args = 0;
	char parsebuf[BUFSIZ];

	stats.attempted++;
	for (curptr = bufhead; ; ) {
		nextreplace = copyline(replace, line[bufhead]);
		argc = parseline(line[bufhead], argv, parsebuf);
		argreg = nextarg(argc, argv);
		if (argreg == -1)
			break;
		args++;
		for (foundarg = 0; curptr != buftail; ) {
			curptr = PRED(curptr);
			argc = parseline(line[curptr], argv, parsebuf);
			if (isendofblock(argc, argv))
				break;
			if (foundarg = ispusharg(argc, argv))
				break;
			mod |= 1 << modifies(argc, argv);
		}
		if (!foundarg)
			break;
		replace = nextreplace;
		if (mod & (1 << argreg)) {
			stats.lostmodified++;
			if (curptr == buftail) {
				(void)newline();
				break;
			}
			(void)newline();
		} else {
			stats.savedpush++;
			rewrite(line[curptr], argc, argv, argreg);
			mod |= 1 << argreg;
		}
	}
	if (argreg == -1)
		stats.finished++;
	emptyqueue();
#ifdef	multimax
	/*
	 *	Do local label generation because multimax assembler can't.
	 */
	resetlabels();
	do {
	    replace = copyline(replace,parsebuf);
	    expandlabels(parsebuf);
	    fputs(parsebuf,stdout);
	} while (*replace != '\0');
#else
	fputs(replace, stdout);
#endif
	cleanup(args);
}

/*
 * Parse a line of assembly language into opcode and arguments.
 */
parseline(linep, argv, linebuf)
	char *linep;
	char *argv[];
	char *linebuf;
{
	register char *bufp = linebuf, *cp = linep;
	register int argc = 0;
	for (;;) {
		/*
		 * skip over white space
		 */
		while (isspace(*cp))
			cp++;
#if	defined(CMU) && defined(ns32000)
		if (*cp == '\0' || *cp == COMMENTCHAR)
#else
		if (*cp == '\0')
#endif
			return (argc);
		/*
		 * copy argument
		 */
		if (argc == MAXARGS - 1) {
			fprintf(stderr, "instruction too long->%s", linep);
			return (argc);
		}
		argv[argc++] = bufp;
		while (!isspace(*cp) && *cp != ARGSEPCHAR && *cp != COMMENTCHAR)
			*bufp++ = *cp++;
		*bufp++ = '\0';
		if (*cp == COMMENTCHAR)
			return (argc);
		if (*cp == ARGSEPCHAR)
			cp++;
	}
}

/*
 * Check for instructions that end a basic block.
 */
isendofblock(argc, argv)
	int argc;
	char *argv[];
{
	register struct inststoptbl *itp;
	int size;

	if (argc == 0)
		return (0);
	for (itp = inststoptblhdr[hash(argv[0], &size)]; itp; itp = itp->next)
		if (itp->size == size && bcmp(argv[0], itp->name, size) == 0)
			return (1);
	return (0);
}

/*
 * Copy a newline terminated string.
 * Return pointer to character following last character copied.
 */
char *
copyline(from, to)
	register char *from, *to;
{

	while (*from != '\n')
		*to++ = *from++;
	*to++ = *from++;
	*to = '\0';
	return (from);
}

/*
 * Check for a disparity between the number of arguments a function
 * is called with and the number which we expect to see.
 * If the error is unrecoverable, return 1, otherwise 0.
 */
argcounterr(args, callargs, name)
	int args, callargs;
	char *name;
{
	register char *cp;
	char namebuf[MAXLINELEN];

	if (args == callargs)
		return (0);
	cp = strcpy(namebuf, name);
	while (*cp != '\0' && *cp != '\n')
		++cp;
	if (*cp == '\n')
		*cp = '\0';
	if (callargs >= 0) {
		fprintf(stderr,
		"%s: error: arg count mismatch, %d != %d for '%s' at line %d\n",
			whoami, callargs, args, namebuf, lineno);
		return (1);
	}
#ifdef	CMU
	if (print_warnings)
#endif
	fprintf(stderr,
		"%s: warning: can't verify arg count for '%s' at line %d\n",
		whoami, namebuf, lineno);
	return (0);
}

/*
 * open space for next line in the queue
 */
char *
newline()
{
	bufhead = SUCC(bufhead);
	if (bufhead == buftail) {
		fputs(line[buftail], stdout);
		buftail = SUCC(buftail);
	}
	return (line[bufhead]);
}

/*
 * empty the queue by printing out all its lines.
 */
emptyqueue()
{
	while (buftail != bufhead) {
		fputs(line[buftail], stdout);
		buftail = SUCC(buftail);
	}
}

/*
 * Compute the hash of a string.
 * Return the hash and the size of the item hashed
 */
hash(cp, size)
	char *cp;
	int *size;
{
	register char *cp1 = cp;
	register int hash = 0;

#if	defined(CMU) && defined(balance)
	while (*cp1 && *cp1 != '\n' && *cp1 != COMMENTCHAR)
		hash += (int)*cp1++;
	*size = cp1 - cp;
#else
	while (*cp1 && *cp1 != '\n')
		hash += (int)*cp1++;
	*size = cp1 - cp + 1;
#endif
	hash &= HSHSIZ - 1;
	return (hash);
}

#ifdef	multimax

/*
 *	resetlabels resets local label expansion
 */
resetlabels()
{
    int	i;

    labelbufp = labelbuf;
    for ( i=0; i < LOC_LABEL_LIMIT; i++)
    	labelstrings[i] = '\0';
}

/*
 *	translate label returns the string for local label n.  It is entered
 *	into the table if it hasn't been seen before.
 */
 
char *
translatelabel(num)
int	num;
{

    if (num >= LOC_LABEL_LIMIT) {
	fprintf(stderr,"Local label %d is too large: use smaller number.\n",
	    num);
	exit(1);
    }
    if (labelstrings[num] == (char *)0) {
	/*
	 *	String starts here in buffer; put in prefix, then number of
	 *	this label, and advance pointer to next place in buffer.
	 */
	labelstrings[num] = labelbufp;
	*labelbufp++ = '.';
	*labelbufp++ = 'L';
	*labelbufp++ = 'i';
	sprintf(labelbufp, "%d", labelnum++);
	while (*labelbufp++ != '\0');
    }

    return(labelstrings[num]);
}

/*
 *	expandlabels expands all the labels in the line passed.
 */
expandlabels(thisline)
char *thisline;
{
    char *cp, *bp;
    char tempbuf[MAXLINELEN];
    char temp;
    int	 num;

    cp = thisline;

    while (1) {    
	/*
	 *	Look for digit that starts label.  Return if we
	 *	fall off the end
	 */
	while (!isdigit(*cp)) {
	    if (*cp == '\0' || *cp == COMMENTCHAR)
		return;
	    cp++;
	}

	/*
	 *	Mark the potential start
	 */
	bp = cp;

	/*
	 *	Scan to end of number.
	 */
	do {
	    cp++;
	} while(isdigit(*cp));

	/*
	 *	If we got a label, translate it in place.  This requires
	 *	that tail end of string be saved and then restored.  Set
	 *	cp past end to continue scan.
	 */
	if (*cp == LOC_LABEL_CHAR) {
	    cp++;
	    strcpy(tempbuf,cp);
	    sscanf(bp, "%d", &num);
	    sprintf(bp, "%s", translatelabel(num));
	    do {
		bp++;
	    } while (*bp != '\0');
	    strcpy(bp, tempbuf);
	    cp = bp;
	}
    }
}
#endif

#ifdef	i386
bcmp(s1, s2, n)
register char *s1, *s2;
register int n;
{

	while (n--)
		if (*s1++ != *s2++)
			return(1);
	return(0);
}
#endif
