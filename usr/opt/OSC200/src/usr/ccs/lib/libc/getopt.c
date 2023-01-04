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
static char	*sccsid = "@(#)$RCSfile: getopt.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/09/23 18:29:34 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: getopt
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 *
 * 1.3  com/lib/c/env/getopt.c, 9123320, bos320 5/23/91 12:59:36
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak getopt = __getopt
#pragma weak opterr = __opterr
#pragma weak optind = __optind
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <nl_types.h>
#include "libc_msg.h"

#define ERR(s, c)	if (opterr){\
	(void) fputs(argv[0], stderr);\
	(void) fputs(s, stderr);\
	(void) fputc(c, stderr);\
	(void) fputc('\n', stderr);}

int	opterr = 1;		/* print errors?			*/
int	optind = 1;		/* index of next argv			*/
int	optopt;			/* current option we're looking at	*/
char	*optarg;		/* argument for current option		*/


/*
 * NAME:	getopt
 *
 * FUNCTION:	get flag letters from the argument vector
 *
 * NOTES:	Getopt scans the command line looking for specified flags.
 *		The flags may or may not require arguments.
 * 		If option followed by a ';', argument has optional param.
 *
 * return  VALUE DESCRIPTION:	returns '?' on an error (unrecognized flag or
 *		a recognized flag requiring an argument and no argument given),
 *              -1 on successful completion of the scan of the command line,
 *		else the recognized flag just encountered
 */  

int getopt( int	 argc,			/* number of command line arguments */
	    char * const *argv,		/* pointer to command line arguments */
	    char const *optstring)	/* string describing valid flags */
{
	int	c;
	char	*cp;
	nl_catd	catd;
	static int stringind = 1;

	optarg = "";		/* Defend against VSX4 test problems */

	if (stringind == 1) {
		if (optind >= argc || argv[optind] == NULL ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0') {
			return (-1);
		} else if (strcmp(argv[optind], "--") == 0) {
			optind++;
			return (-1);
		}
	}
	optopt = c = argv[optind][stringind];
	if (c == ':' || (cp = strchr(optstring, c)) == NULL) {
		/* check for illegal options */
		if (optstring[0] != ':') {
			catd = catopen(MF_LIBC, NL_CAT_LOCALE);
			ERR(catgets(catd, MS_LIBC, M_OPTILL, 
			    ": illegal option -- "), c);
			catclose (catd);
		}
		if (argv[optind][++stringind] == '\0') {
			optind++;
			stringind = 1;
		}
		return ('?');
	}
	if (*++cp == ':') {		/* parameter is needed */
		/* no blanks to separate option and parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind++][stringind+1];
		else if (++optind >= argc) {
			optarg = (char *) argv[optind];
			stringind = 1;
			if (optstring[0] != ':') {
				catd = catopen(MF_LIBC, NL_CAT_LOCALE);
				ERR(catgets(catd, MS_LIBC, M_OPTARG,
				    ": option requires an argument -- "), c);
				catclose(catd);
				return ('?');
			}
			return (':');
		} else
			optarg = (char *) argv[optind++];

		stringind = 1;
	/*
	 * OSF extention - ';' means optional parameter
	 * must follow option with no whitespace.
	 * To support obsolete syntax in the commands
	 */
	} else if (*cp == ';') {		/* optional parameter */
		if (argv[optind][stringind+1] != '\0')
			optarg = (char *) &argv[optind][stringind+1];
		else 
			optarg = NULL;
		stringind = 1;
		optind++;
	} else {			/* parameter not needed */
		/* if c is the last option update optind */
		if (argv[optind][++stringind] == '\0') {
			stringind = 1;
			optind++;
		}
		optarg = NULL;
	}
	return (c);
}



