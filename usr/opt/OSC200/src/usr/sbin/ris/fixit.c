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
static char *rcsid = "@(#)$RCSfile: fixit.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/09/29 16:58:27 $";
#endif

/*
 * fixit: program to patch a binary file and replace instance
 * of string1 with string2.
 */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */

#define NETLOAD "./netload"	/* name of netload to modify */
#define NETTEXT "./nettext"	/* name of text file containing string names */
char *progname="fixit"; 	/* for printing error messages */
int f1, f2;

/* WARNING! these NAMES must match the ones defined in netload.c! */
char *kernelid="KERNELIMAGE:";
char *serverid="SERVERNAME:";

main(argc, argv)
int argc;
char *argv[];
{
	/* XXX: TODO: make this whole thing generic, for use elsewhere? */

	/* If given an argument, then this is the name of
	 * the file to modify rather than "./netload".
	 */
	if (argc == 1) {
		f1 = open(NETLOAD, O_RDWR);
		if (f1 < 0) {
			printf("%s: cannot open %s, errno=%d\n",
				progname, NETLOAD, errno);
			exit(-1);
		}
	} else {
		f1 = open(argv[1], O_RDWR);
		if (f1 < 0) {
			printf("%s: cannot open %s, errno=%d\n",
				progname, argv[1], errno);
			exit(-1);
		}
	}
	f2 = open(NETTEXT, O_RDONLY);
	if (f2 < 0) {
		printf("%s: cannot open %s, errno=%d\n",
			progname, NETTEXT, errno);
		exit(-1);
	}

	replace(kernelid);

	if (close(f1) < 0)
		printf("%s: close failed for %s, errno=%d\n",
			progname, NETLOAD, errno);
	if (close(f2) < 0)
		printf("%s: close failed for %s, errno=%d\n",
			progname, NETTEXT, errno);

	/* start again, for the second string */
	f1 = open(NETLOAD, O_RDWR);
	if (f1 < 0) {
		printf("%s: cannot open %s, errno=%d\n",
			progname, NETLOAD, errno);
		exit(-1);
	}
	f2 = open(NETTEXT, O_RDONLY);
	if (f2 < 0) {
		printf("%s: cannot open %s, errno=%d\n",
			progname, NETTEXT, errno);
		exit(-1);
	}

	replace(serverid);

	if (close(f1) < 0)
		printf("%s: close failed for %s, errno=%d\n",
			progname, NETLOAD, errno);
	if (close(f2) < 0)
		printf("%s: close failed for %s, errno=%d\n",
			progname, NETTEXT, errno);
	exit(0);
}

/*
 * Replace string "s" from NETTEXT into NETLOAD.
 * Globals f1 and f2 are open file descriptors from the 2 relevant files,
 * f1 is the image we are modifying, f2 is the readonly file which
 * contains the new value of the strings.
 */
replace(s)
char *s;
{

	int ret;
	char buf[32];

	/* locate text string(s) for replacement */
	ret = find(f1, s);
	if (ret < 0) {
		printf("%s: cannot locate string %s in %s\n",
			progname, s, NETLOAD);
		exit(-1);
	}
	ret = find(f2, s);
	if (ret < 0) {
		printf("%s: cannot locate string %s in %s\n",
			progname, s, NETTEXT);
		exit(-1);
	}

	/* substitute new string here */
	while ((ret = read(f2, buf, 1)) == 1 && (buf[0] != '\n') ) {
		ret = write(f1, buf, 1);
		if (ret != 1) {
			printf("%s: write failed to %s, errno=%d\n",
				progname, NETLOAD, errno);
			exit(-1);
		}
	}
	if (ret < 0) {
		printf("%s: read failed from %s, premature EOF, errno=%d\n",
			progname, NETLOAD, errno);
		exit(-1);
	}
	buf[0] = NULL;	/* null terminate */
	ret = write(f1, buf, 1);
	if (ret < 0) {
		printf("%s: write failed to %s, errno=%d\n",
			progname, NETLOAD, errno);
		exit(-1);
	}
	return(0);
}

/*
 * find the string s in file descriptor fd;
 * return with fd pointing to the first character position
 * after string s, if found.
 */
find(fd, s)
int fd;
char *s;	
{
	char temp[32]; /* just long enough to hold "KERNELIMAGE:" */
	char buf[16];  /* just 1 character long, really */
	int count;

	strcpy(temp, s); /* save a copy */
	count = 0;
	while (read(fd, buf, 1) > 0) {
		if (buf[0] == temp[count]) {
			if (++count == strlen(temp)) {
				return(0); /* done, fp is where we want it */
			}
		} else {
			count=0; /* start over */
		}
	}
	/* fallthrough error */
	return(-1); /* string not found */
}
