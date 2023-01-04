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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getopt.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:05:32 $";
#endif
/* 
 * COMPONENT_NAME: UUCP getopt.c
 * 
 * FUNCTIONS: ERR, getopt 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
1.3  /com/cmd/uucp/getopt.c, bos320 6/15/90 23:59:13";
*/
#include "uucp.h"
/* VERSION( getopt.c	5.2 -  -  ); */

/*	getopt.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#define ERR(s, c)	if(opterr){\
	(void) fputs(argv[0], stderr);\
	(void) fprintf(stderr, s);\
	(void) fputc(c, stderr);\
	(void) fputc('\n', stderr);}

extern int strcmp();
extern char *strchr();

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(MSGSTR(MSG_GOPT_1, ": illegal option -- "), c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(MSGSTR(MSG_GOPT_2, 
			    ": option requires an argument -- "), c);
			sp = 1;
			return('?');
#ifndef NO_MINUS
		} else
			optarg = argv[optind++];
#else NO_MINUS
		} else {
			optarg = argv[optind++];
			if (*optarg == '-') {
			    ERR(MSGSTR(MSG_GOPT_2, 
			      ": option requires an argument -- "), c);
			    sp = 1;
			    return('?');
			}
		}
#endif NO_MINUS
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}
