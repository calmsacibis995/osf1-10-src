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
static char rcsid[] = "@(#)$RCSfile: cmp.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 16:01:07 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: cmp
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * cmp.c	1.12  com/cmd/files/cmp.c, bos320 4/26/91
 */

#include	<stdio.h>
#include	<ctype.h>
#include	<locale.h>
#include "cmp_msg.h"
#define	MSGSTR(Num, Str) catgets(catd,MS_CMP,Num,Str)

nl_catd catd;

FILE	*file1,*file2;              /* the files to be compared */

char	*arg;                       /* argument holder */

int	dflg = 0;                   /* difference flag */
int	lflg = 0;                   /* long output flag */
int	sflg = 0;                   /* short output flag */

long	line = 1;                   /* line number */
long	byte = 0;                   /* byte number */

/*
 * NAME: cmp [-l] [-s] file1 file2 
 *                                                                    
 * FUNCTION: Compares two files
 *                                                                    
 * NOTES:   Compares two files sending the differences to standard out.
 *          If a '-' is given for file1 or file2, then cmp reads from      
 *          standard in for that file.  Two hyphens cannot be entered for
 *          both input files.  The default output is that cmp displays
 *          nothing if the files are the same.  If the files differ, cmp
 *          displays the byte and line number at which the first 
 *          difference occurs.
 *          -l    Displays, for each difference, the byte number in
 *                decimal and the differing bytes in octal.
 *          -s    Returns only an exit value.
 *
 * RETURN VALUE DESCRIPTION: 
 *          0  indicates identical files.
 *          1  indicates different files.
 *          2  indicates inaccessible files or a missing argument.
 */  
main(argc, argv)
int argc;
char **argv;
{
	extern char 	*optarg;
	extern int	optind;
	int 	ch;
	int c1, c2;           /* current charcters from file1 and file2 */

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_CMP,NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "ls")) != EOF)
		switch(ch) {
		case 'l':		/* print all differences */
			lflg++;
			break;
		case 's':		/* silent run */
			sflg++;
			break;
		default:
			narg();
		}
	if (lflg && sflg)			    /* mutually exclusive */
		narg();

	argv += optind;
	argc -= optind;

	if (argc != 2 )				     /* ensure appropriate */
		narg();				     /* no. of arguments */
	arg = argv[0];
	if( arg[0]=='-' && arg[1]==0 )               /* file1 from stdin */
		file1 = stdin;
	else if((file1 = fopen(arg, "r")) == NULL)
		barg();                                      /* bad file */

	arg = argv[1];
	if( arg[0]=='-' && arg[1]==0 )               /* file2 from stdin */
		file2 = stdin;
	else if((file2 = fopen(arg, "r")) == NULL)
		barg();                                      /* bad file */

	if (file1 == stdin && file2 == stdin)
		narg();

	while(1) {                                  /* compare the files */
		c1 = getc(file1);
		c2 = getc(file2);
		byte++;
		if(c1 == c2) {
			if (c1 == '\n')
				line++;
			if(c1 == EOF) {
				if(dflg)
					exit(1);
				exit(0);
			}
		}
		else {
			if(sflg > 0)
				exit(1);
			if(c1 == EOF) {
				arg = argv[0];
				earg();		/* file1 shorter than file2 */
			}
			if(c2 == EOF)
				earg();		/* file2 shorter than file1 */
			if(lflg == 0) {		/* default output, exit */
				printf(MSGSTR(DIFF,
				 "%s %s differ: char %ld, line %ld\n"),
						argv[0], arg, byte, line);
			exit(1);
			}
			dflg = 1;
			printf("%6ld %3o %3o\n", byte, c1, c2);
		}
	} /* end of while */
}

/*
 * NAME: narg
 *                                                                    
 * FUNCTION:  print out usage statement and exit program.
 */  
narg()
{
	fprintf(stderr,MSGSTR(USG,"usage: cmp [-l] [-s] file1 file2\n"));
	exit(2);
}

/*
 * NAME: barg
 *                                                                    
 * FUNCTION: print out error message and exit program.
 */  
barg()
{
	if (sflg == 0)
		fprintf(stderr, MSGSTR(NOTOPEN,
					"cmp: cannot open %s\n"), arg);
	exit(2);
}

/*
 * NAME: earg
 *                                                                    
 * FUNCTION: file arg was shorter than the other file, print message
 *           and exit.
 */  
earg()
{
	fprintf(stderr, MSGSTR(EOFMSG,"cmp: EOF on %s\n"), arg);
	exit(1);
}
