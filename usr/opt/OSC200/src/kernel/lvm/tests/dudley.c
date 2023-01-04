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
static char	*sccsid = "@(#)$RCSfile: dudley.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:22:26 $";
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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/fcntl.h>
#include <sys/file.h>

char *progname;

main(argc, argv)
int argc;
char **argv;
{
char buf[8192];
int fd, count;
int rflag = 0, wflag = 0;
long pattern;
int i;
int c;
extern int opterr;
extern int optind;
extern char *optarg;
extern char optopt;
char mess[1024];
long getnumber();

	pattern = 0;
	while ((c = getopt(argc, argv, "rwp:")) != EOF) {
		switch (c) {
			case 'p': pattern = getnumber(optarg); break;
			case 'r': rflag++; break;
			case 'w': wflag++; break;
			default:
				return(-1);
		}
	}
	if (!rflag && !wflag) rflag++;

	if ((progname = rindex(argv[0], '/')) != NULL) {
		*progname++ = '\0';
	} else {
		progname = argv[0];
	}

	if ((fd = open(argv[optind], wflag?O_RDWR:O_RDONLY)) < 0) {
		sprintf(mess,"%s %s", progname, argv[optind]);
		perror(mess);
		return(-1);
	}
	if (pattern) {
		for (i = 0; i < 8192; i += sizeof(long))
			*(long *)(buf+i) = pattern;
	} else {
		bzero(buf, 8192);
	}
	for(;;) {
		if (rflag) {
			if ((count = read(fd, buf, 8192)) < 0) {
				perror("read");
				return(1);
			} else if (count == 0) {
				fprintf(stderr, "EOF\n");
				return(0);
			}
		}
		if (wflag) {
			if ((count = write(fd, buf, 8192)) < 0) {
				perror("write");
				return(1);
			} else if (count == 0) {
				fprintf(stderr, "EOF\n");
				return(0);
			}
		}
	}
}

long getnumber(s)
char *s;
{
extern long htol(), dtol(), otol(), btol();

	if (!isdigit(*s)) return(0);	/* actually, an error */

	if (*s != '0') return(atol(s));
	switch(*(s+1)) {
		case 'x': /* Hex constant: 0x... */
		case 'X': return(htol(s+2));

		case 't': /* Binary constant: 0b... */
		case 'T': return(dtol(s+2));

		case 'b': /* Binary constant: 0b... */
		case 'B': return(btol(s+2));

		default: /* Octal constant: 0... */
			return(otol(s+2));
	}
}

/* Hexadecimal string to integer conversion. NOT bullet-proof. Assumes that
 * the input is, in fact, a valid hex constant.
 */
long
htol( s )
char *s;
{
long result;
int c, digit;
	result = 0;
	while( c = *s++ ) {
		if( c <= '9' ) {
			digit = c - '0';
		} else {
			if (isupper(c)) c = tolower(c);
			digit = c - 'a' + 10;
		}
		result = result * 0x10 + digit;
	}
	return( result );
}

/* Decimal string to integer conversion. NOT bullet-proof. Assumes that
 * the input is, in fact, a valid hex constant.
 */
long
dtol( s )
char *s;
{
long result;
int c, digit;
	result = 0;
	while( c = *s++ ) {
		digit = c - '0';
		result = result * 10 + digit;
	}
	return( result );
}

/* Octal string to integer conversion. NOT bullet-proof. Assumes that
 * the input is, in fact, a valid octal constant.
 */
long
otol( s )
char *s;
{
long result;
int c, digit;
	result = 0;
	while( c = *s++ ) {
		digit = c - '0';
		result = result * 010 + digit;
	}
	return( result );
}

/* Binary string to integer conversion. NOT bullet-proof. Assumes that
 * the input is, in fact, a valid binary constant.
 */
long
btol( s )
char *s;
{
long result;
int c, digit;
	result = 0;
	while( c = *s++ ) {
		digit = c - '0';
		result = result * 2 + digit;
	}
	return( result );
}
