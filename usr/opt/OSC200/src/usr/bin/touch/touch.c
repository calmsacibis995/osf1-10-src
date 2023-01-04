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
static char rcsid[] = "@(#)$RCSfile: touch.c,v $ $Revision: 4.2.8.4 $ (DEC) $Date: 1993/10/11 19:25:51 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
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
 * 
 * 1.11  com/cmd/scan/touch.c, bos320 2/15/91 12:36:47
 */

/*
 *	Updates the access and modification times of each file or
 *	directory named.  Options include:
 *
 *		-a	Access time only
 *		-c	Do NOT create the file if it doesn't exist
 *		-m	Modification time only
 *		-f	force (the default case).
 *              -r ref_file     Use time from reference file
 *              -t time         Use the specified time instead of current time
 *
 *              touch format time with no flag is [mmddhhmm[yy]]
 *              touch format time with -t flag is [[CC]YY]MMDDhhmm[.SS]]
 *
 */                                                                   

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <utime.h>
#include <string.h>
#include <sys/mode.h>
#include <wchar.h>
#include <langinfo.h>
#include "touch_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd,MS_TOUCH,Num,Str)

#define YEAR_AND_CENTURY 1

struct	stat	stbuf;
int	status;

char	*cbp;
time_t	timbuf;
struct tm	touchtime;

static void usage();

const char *POSIX_fmts[] = { "%Y"" %m"" %d"" %H"" %M", "%y"" %m"" %d"" %H"" %M", "%m"" %d"" %H"" %M", NULL };
const char *obsolete_fmts[] = { "%m"" %d"" %H"" %M"" %y", "%m"" %d"" %H"" %M", NULL };

char* add_delimiters(int year_and_century_flg, char* tmp)
{
    char* new_buf;
    char* delim;
    int i, len, str_len;

    str_len = strlen(tmp);

    if ((new_buf = (char*) malloc(str_len + 8)) == NULL)
	     (void) fprintf(stderr,MSGSTR(TMALLOC, "touch: No memory\n"));

    new_buf[0] = '\0';

    if (year_and_century_flg) {
        strncpy(new_buf, tmp, 4);
	strcat(new_buf, " ");
	tmp += 4;
	str_len -= 4;
    }


    for(i = 0; i < str_len; i += len) {

	if (*tmp == '.') {
		len = 3;
		delim = "";
	}
	else {
		len = 2;
		delim = " ";
	}

	strncat(new_buf, tmp, len);
	strcat(new_buf, delim);
	tmp += len;
    }

    return (new_buf);
}


/*
 * NAME: gtime
 *                                                                    
 * FUNCTION: 	Convert ascii time value on command line into a
 *		the number of seconds since 1970.
 *		Accepts several different formats using strptime()
 *
 * 		Returns (time_t) -1 on failure
 */  

static time_t
gtime(char *timestring, const char *candidates[])
{
    char	*p = timestring;
    char	*q;
    char        *new_cbp;
    int		century=0;	/* Century delta to add */
    char	*fmt;

    for( fmt= (char *)*candidates++; fmt; fmt=(char *)*candidates++) {

	touchtime = *localtime(&timbuf);	/* Resets to right now */

	/* if the YEAR should contain the Century, then set the 
	   appropriate flag on the argument to add the delimeters in
	   the right place.
	*/
	if (strchr(fmt, 'Y'))
	    new_cbp = (char*) add_delimiters(YEAR_AND_CENTURY, timestring);
	else
	    new_cbp = (char*) add_delimiters(0, timestring);

	p = new_cbp;

	q = strptime(p, fmt, &touchtime);	/* Trial conversion */

	if (!q) continue;			/* Failed */

	if (*q == ' ') q++;   /* move past the blank that was inserted 
				 by add_delimiters */

	if (*q == '.')				/* Expecting seconds */
	  q = strptime(q, ".%S", &touchtime);

	if (!q || *q)				/* Format error or extra chars */
	  continue;

	/*
	 * Need to check tm_year for special posix values
	 */
	if ((touchtime.tm_year < 70) &&	/* Can't express times before 1970 */
	    (!strchr(fmt,'Y')))		/* and they didn't use explicit century */
	    touchtime.tm_year += 100;	/* Then assume they really meant 21st century */

	return mktime(&touchtime);
    }

    /*
     * Only reach here if no format could effectively convert the time string
     */

    return (time_t) -1;
}

void
main(argc, argv)
int argc;
char *argv[];
{
    register int c;
    struct utimbuf times;
    int mflg=-1, aflg=-1, cflg=0, rflg=0, tflg=0;
    int	optc, fd;
    int tlen;			/* Used to count string length */
    char *reffile;
    extern int optind;
    extern char *optarg;
    const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

    time_t	atime,mtime;	/* Access and Modify Times */

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_TOUCH, NL_CAT_LOCALE);

    while ((optc=getopt(argc, argv, "amcfr:t:?")) != EOF)
        switch(optc) {
	case 'm':
	    mflg=1;		/* Modification times only    */
	    if(aflg<0) aflg=0;	/* Was -a defaulted? */
	    break;
	case 'a':		/* Access  times only         */
	    aflg=1;
	    if(mflg<0) mflg=0;	/* Was -m defaulted? */
	    break;
	case 'c':		/* Don't creat if isn't there */
	    cflg++;
	    break;
	case 'f':		/* silent compatibility case  */
	    break;
	case 'r':
	    rflg++;
	    reffile = optarg;
	    break;
	case 't':
	    tflg++;
	    cbp = optarg;
	    break;
	default:
	    usage();
	}

    /* -r and -t options are mutually exclusive */
    if ((optind == argc) || (rflg && tflg))	/* Not enough arguments */
        usage();

    atime = mtime = timbuf = time(NULL);	/* Start with now as modification time */
    touchtime = *localtime(&timbuf);


    tlen = strlen(argv[optind]);

    if (tflg) {
	timbuf = gtime(cbp, POSIX_fmts);
	if (timbuf == (time_t)-1) {
	    (void) fprintf(stderr,MSGSTR(TCONV, "touch: bad conversion\n"));
	    exit(2);
	}
	atime = timbuf;
	mtime = timbuf;
	
    } else if (rflg) {
	if ( (stat(reffile, &stbuf) == -1) ) {
	    fprintf(stderr, "%s:", reffile);
	    perror("stat");
	    exit(2);
	}
	atime = stbuf.st_ctime;
	mtime = stbuf.st_mtime;

    } else if(((argc-optind) >=2) &&
       (tlen==8 || tlen==10) ) {
	/*
	 * POSIX says that if no -t or -r was specified and at least two operands
	 * are used then the first is assumed to be a date, otherwise the first
	 * operand shall be assumed to be a file operand.
	 * This just might be MMDDhhmm[yy]
	 */
	char *ts=argv[optind];

	for( ;tlen>0; tlen--)
	    if (!isdigit(*ts++)) break;

	if ( !tlen ) {
	    cbp = argv[optind++];
	    timbuf = gtime(cbp, obsolete_fmts);
	    if (timbuf == (time_t)-1) {
		fputs( MSGSTR(TCONV, "touch: bad conversion\n"), stderr);
		exit(2);
	    }
	    atime = timbuf;
	    mtime = timbuf;
	}

    }

    /*
     * Now, loop thru the argument list, and adjust the times on each file
     * as required.
     */

    for(c=optind; c<argc; c++) {
	if(stat(argv[c], &stbuf)) {	/* Does file exist? */
	    if (cflg) {			/* Nope!  Don't create it either */
		status++;
		continue;
	    }
	    else if (errno == ENOENT) {
		if ((fd = creat (argv[c], mode)) < 0) {
		    fprintf(stderr, MSGSTR(TCREATE, "touch: %s cannot create\n"),
			    argv[c]);
		    status++;
		    continue;
		}
		(void) close(fd);
		if(stat(argv[c], &stbuf)) {
		    fprintf(stderr,MSGSTR(TSTAT, "touch: %s cannot stat\n"),argv[c]);
		    status++;
		    continue;
		}
	    } else {
		fprintf(stderr, "touch %s:", argv[c]);
		perror(NULL);
	    }
	}

	times.modtime = (mflg)? mtime : stbuf.st_mtime;
	times.actime = (aflg) ? atime : stbuf.st_atime;

	if(utime(argv[c], &times)) {
	    fprintf(stderr,MSGSTR(TCHTIME, "touch: cannot change times on %s\n"),
		    argv[c]);
	    status++;
	    continue;
	}
    }
    exit(status);
}

static void
usage()
{
    fputs(MSGSTR(TUSAGE,
	 "Usage: touch [-acfm] [-r ref_file |-t [[CC]YY]MMDDhhmm[.SS]] " \
	 "file ...\n" \
	 "       touch [-acfm] [MMDDhhmm[yy]] file ...\n"),
	  stderr);
    exit(2);
}
