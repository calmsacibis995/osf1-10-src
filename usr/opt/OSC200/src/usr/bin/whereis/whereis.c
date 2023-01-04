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
static char rcsid[] = "@(#)$RCSfile: whereis.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/10/11 19:54:21 $";
#endif
/*
 * HISTORY
 */
/*	OSF1.1 BL 30
 * Revision 1.1.1.3  92/03/31  00:44:00  devrcs
 *  *** OSF1_1B25 version ***
 * 
 * Revision 2.10.2.3  1992/02/18  20:47:57  damon
 * 	Removed I B M  C O N F I D E N T I A L messages
 * 	[1992/02/14  19:39:56  damon]
 *
 * Revision 2.10.2.2  1992/01/28  17:27:11  collins
 * 	Fixed a problem with the command line parsing.
 * 	Print a usage message when no command is specified.
 * 	Updated the usage line to be more intelligible.
 * 	[1992/01/28  17:25:33  collins]
 * 
 * Revision 2.10  1991/08/16  09:16:42  devrcs
 * 	Remove NLgetamsg.
 * 	Add default case to switch.
 * 	[91/07/18  13:28:15  tom]
 * 
 * Revision 2.9  91/03/04  17:37:22  devrcs
 * 	Changed how command line parsing is done by using getopt()
 * 	system call. Also, changed the number of argument to getlist()
 * 	which now return the number of command line argumets it reads.
 * 	Corrected usage message.
 * 	[91/01/28  14:08:12  sgrainge]
 * 
 * Revision 2.8  90/10/31  14:57:46  devrcs
 * 	Added pathnames to bindirs array.
 * 	[90/10/11  20:36:30  sgrainge]
 * 
 * Revision 2.7  90/10/07  17:25:57  devrcs
 * 	Added flags; types and hit so that the
 * 	-u option works as expected. Also, rewrote
 * 	loop in main, removed do/while in favor of
 * 	while loop. Added bufout to provide a well
 * 	defined buffer that contains the output string.
 * 	[90/09/18  12:36:17  sgrainge]
 * 
 * Revision 2.6  90/09/23  16:31:56  devrcs
 * 	Move lint ifdef and sccsid
 * 	[90/09/07  16:27:04  sgrainge]
 * 
 * Revision 2.5  90/09/13  12:23:34  devrcs
 * 	General clean up of MSG, NLcatopen,
 * 	usage string.
 * 	[90/08/31  09:24:13  sgrainge]
 * 
 * Revision 2.4  90/07/06  00:07:43  devrcs
 * 	     Added rcs string
 * 	[90/07/01  20:19:53  alla]
 * 
 * Revision 2.3  90/06/22  21:25:44  devrcs
 * 	New AIX command.
 * 	[90/06/03  18:48:36  alla]
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * whereis.c   1.5  com/cmd/scan,3.1,9021 12/21/89 12:53:22";
 */


#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <locale.h>
#include <string.h>

#include "whereis_msg.h" 
nl_catd		catd;
#define MSGSTR(n,s) catgets(catd, MS_WHEREIS, n, s) 

nl_catd         catd;


static void findin();
static void find();
static void findv();
static void lookman();
static void lookbin();
static void looksrc();
static void lookup();
static void zerof();

static char *bindirs[] = {
	"/etc",
	"/etc/nls",
	"/sbin",
	"/usr/bin",
	"/usr/lbin",
	"/usr/lbin/spell",
	"/usr/ccs/lib",
	"/usr/lib",
	"/usr/local",
	"/usr/hosts",
	"/usr/sbin",
	"/usr/bin/X11", 
	"/usr/Wnn4",
	"/usr/bin/nemacs",
	0
};
static char *mandirs[] = {
	"/usr/share/cat/cat1",
	"/usr/share/cat/cat2",
	"/usr/share/cat/cat3",
	"/usr/share/cat/cat4",
	"/usr/share/cat/cat5",
	"/usr/share/cat/cat6",
	"/usr/share/cat/cat7",
	"/usr/share/cat/cat8",
	"/usr/share/man/man1",
	"/usr/share/man/man2",
	"/usr/share/man/man3",
	"/usr/share/man/man4",
	"/usr/share/man/man5",
	"/usr/share/man/man6",
	"/usr/share/man/man7",
	"/usr/share/man/man8",
	0
};
static char *srcdirs[]  = {
	"/usr/src/etc",
	"/usr/src/sbin",
	"/usr/src/bin",
	"/usr/src/lib",
	"/usr/src/local",
	"/usr/src/hosts",
	0
};

char	sflag = 1;
char	bflag = 1;
char	mflag = 1;
char	**Sflag;
int	Scnt;
char	**Bflag;
int	Bcnt;
char	**Mflag;
int	Mcnt;
char	uflag;
int count;
int	print;
int	bytes;
int	types = 3;
int	hit;
char	*bufptr;
char	bufout[BUFSIZ];

/*
 *  NAME:  whereis <name>
 *
 *  FUNCTION:  
 * 	look for source, documentation and binaries
 *
 */

main(argc, argv)
	int argc;
	char *argv[];
{
	int c, stop, status, args;
	char **cp;
	extern char *optarg;
	extern int opterr, optind;

	(void) setlocale (LC_ALL, "");
	catd = catopen(MF_WHEREIS, NL_CAT_LOCALE);
	cp = argv;
	args = 1;
	cp++;
	status = stop = 0;
	if (argc <= 1) {
		usage();
		status = 1;
		exit(status);	/*DAL001*/
	}
	else {
		while (((c = getopt(argc,argv,"fubmsB:M:S:")) != EOF) &&
		       (!stop)) {
			switch (c) {
			case 'f':
				stop = 1;
				break;
			case 'S':
				args = getlist(&cp, &Sflag, &Scnt);
				optind += (args - 2); /* Move past arg list */
				break;
			case 'B':
				args = getlist(&cp, &Bflag, &Bcnt);
				optind += (args - 2); /* Move past arg list */
				break;
			case 'M':
				args = getlist(&cp, &Mflag, &Mcnt);
				optind += (args - 2); /* Move past arg list */
				break;
			case 's':
				zerof();
				sflag++;
				types++;
				break;
			case 'u':
				uflag++;
				break;
			case 'b':
				zerof();
				bflag++;
				types++;
				break;
			case 'm':
				zerof();
				mflag++;
				types++;
				break;
			case '?':
			default:
				usage();
				status = 1;
				break;
			}
			cp++;
		}
	}
	if (*cp == NULL) {
		usage();
		status = 1;
	}
	if (status == 0) {
		while (*cp != NULL) lookup(*cp++);
	}
	exit(status);
}

/*
 *  NAME:  usage
 *
 *  FUNCTION:  	If an error in encountered return usage to stdout.
 *
 *  RETURN VALUE:  none, message to stdout. 
 *
 */
usage()
{
	fprintf(stderr, MSGSTR(USAGE, 
"usage: whereis [-bms] [-u] [-BMS directory ... -f] program ...\n")); /*MSG*/
}

/*
 *  NAME:  getlist
 *
 *  FUNCTION:  	If the -B -S -M option is specified, this routine
 *		is called and an alternate search path will be defined.
 *		This search path is used to locate the specifed file.
 *
 *  RETURN VALUE:  number of path args, path parameter modified.
 *
 */
int
getlist(argvp, flagp, cntp)
	char ***argvp;
	char ***flagp;
	int *cntp;
{
	int count;

	(*argvp)++;
	*flagp = *argvp;
	*cntp = 0;
	for (count=1; **argvp != NULL && (*argvp)[0][0] != '-'; count++) 
			(*cntp)++, (*argvp)++;
	(*argvp)--;
	return(count);
}

/*
 *  NAME:  zerof
 *
 *  FUNCTION:  zero all the option flags
 *
 *  RETURN VALUE:  none
 *
 */
static void
zerof()
{

	if (sflag && bflag && mflag)
		types = sflag = bflag = mflag = 0;
	return;
}

/*
 *  NAME:  lookup
 *
 *  FUNCTION:   Take a filename, strip off path and suffix.  Call 
 *		look{bin,src,man} to search path and print out file
 *		location.
 *
 *  RETURN VALUE:  none, subroutines will print out the pathname.
 *
 */
static void
lookup(cp)
	register char *cp;
{
	register char *dp;

	for (dp = cp; *dp; dp++)
		continue;
	for (; dp > cp; dp--) {
		if (*dp == '.') {
			*dp = 0;
			break;
		}
	}
	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (uflag) {
		print = 0;
		count = 0;
		sprintf(bufout,"%s:",cp);
		bytes = strlen(cp);
		bufptr = (char *) &bufout[1+bytes];	
	} else
		print = 1;
	hit = 0;
again:
	if (print && !uflag) {
		sprintf(bufout,"%s:",cp);
		bytes = strlen(cp);
		bufptr = (char *) &bufout[(1+bytes)];	
	}
	if (sflag) {
		looksrc(cp);
		if (!uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (bflag) {
		lookbin(cp);
		if (!uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (mflag) {
		lookman(cp);
		if (!uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	if ((print) || (hit != types))
		printf("%s\n",bufout);
	return;
}

/*
 *  NAME:  looksrc
 *
 *  FUNCTION:  Check for the file in the Source directories
 */
static void
looksrc(cp)
	char *cp;
{
	if (Sflag == 0) {
		find(srcdirs, cp);
	} else
		findv(Sflag, Scnt, cp);
	return;
}

/*
 *  NAME:  lookbin
 *
 *  FUNCTION:  Check for the file in the Binary directories
 */
static void
lookbin(cp)
	char *cp;
{
	if (Bflag == 0)
		find(bindirs, cp);
	else
		findv(Bflag, Bcnt, cp);
	return;
}

/*
 *  NAME:  lookman
 *
 *  FUNCTION:  Check for the file in the Man page directories
 */

static void
lookman(cp)
	char *cp;
{
	if (Mflag == 0) {
		find(mandirs, cp);
	} else
		findv(Mflag, Mcnt, cp);
	return;
}

/*
 *  NAME:  findv
 *
 *  FUNCTION:  	Check for file using the given path.  This is an
 *		alternate path.
 */
static void
findv(dirv, dirc, cp)
	char **dirv;
	int dirc;
	char *cp;
{

	while (dirc > 0)
		findin(*dirv++, cp), dirc--;
	return;
}

/*
 *  NAME:  find
 *
 *  FUNCTION:  	Check for file using the given path.  
 */
static void
find(dirs, cp)
	char **dirs;
	char *cp;
{

	while (*dirs)
		findin(*dirs++, cp);
	return;
}

/*
 *  NAME:  findin
 *
 *  FUNCTION:  	Actually check to see if the given file exists (or a like file)
 *		in the directory given.  If so, print it out.
 */
static void
findin(dir, cp)
	char *dir, *cp;
{
	DIR *dirp;
	struct dirent *dp;

	dirp = opendir(dir);
	if (dirp == NULL)
		return;
	while ((dp = readdir(dirp)) != NULL) {
		if (itsit(cp, dp->d_name)) {
			count++;
			if((print) || (uflag)) {
				sprintf(bufptr," %s/%s", dir, dp->d_name);
				bytes += 2+strlen(dir)+strlen(dp->d_name);
				bufptr = (char *) &bufout[1+bytes];	
			}
			if(uflag)
				if(count == 2)
					--hit;
				else	
					++hit;	
		}
	}
	closedir(dirp);
	return;
}

/*
 *  NAME:  itsit
 *
 *  FUNCTION:  	Returns 1 if the current file matches the current directory
 *		entry.  0 if no match.
*/
int
itsit(cp, dp)
	register char *cp, *dp;
{
	register int i = strlen(dp);

	if (dp[0] == 's' && dp[1] == '.' && itsit(cp, dp+2))
		return (1);
	while (*cp && *dp && *cp == *dp)
		cp++, dp++, i--;
	if (*cp == 0 && *dp == 0)
		return (1);
	while (isdigit((int)*dp))
		dp++;
	if (*cp == 0 && *dp++ == '.') {
		--i;
		while (i > 0 && *dp)
			if (--i, *dp++ == '.')
				return (*dp++ == 'C' && *dp++ == 0);
		return (1);
	}
	return (0);
}
