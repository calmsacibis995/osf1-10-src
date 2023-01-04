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
static char	*sccsid = "@(#)$RCSfile: iconv.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/11 16:59:13 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDICONV)
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.4  com/cmd/iconv/iconv.c, , bos320, 9134320c 8/19/91 21:45:11
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <locale.h>
#include <iconv.h>
#include <errno.h>

#include "iconv_msg.h"

nl_catd	catd;

#define MSGSTR(Num, Str)	catgets(catd, MS_ICONV, Num, Str)

#define ALLOC_UNIT	4096	

/*
 * Defines for checking the return value of iconv
 */
#define ICONV_DONE(r)	(r == 0)
#define ICONV_INVAL(r)	(r && (errno == EILSEQ))
#define ICONV_OVER(r)	(r && (errno == E2BIG))
#define ICONV_TRUNC(r)	(r && (errno == EINVAL))

int
main (int argc, char *argv[])
{
    	int c;			/* command line option 			*/
    	extern char *optarg;	/* command line argument		*/
	extern int optind;	/* 					*/
	extern int opterr;	/* 					*/
    	char *fcode=(char *)0;	/* original code page 			*/
    	char *tcode=(char *)0;	/* converting code page 		*/
	char *inptr;		/* Pointer used for input buffer	*/
	char *outptr;		/* Pointer used for output buffer	*/
	char *inbuf; 		/* input buffer 			*/
	char *outbuf; 		/* output buffer 			*/
	size_t inleft;		/* number of bytes left in inbuf 	*/
	size_t outleft;		/* number of bytes left in outbuf 	*/
	iconv_t cd;		/* conversion descriptor 		*/
	int	rc;		/* return code of iconv_exec 		*/
	int inputlen;		/* number of bytes to be converted	*/
	int bytesread; 		/* number of bytes read into input buf 	*/
	int fdin=0;		/* input file descriptor 		*/
	int fdout=1;		/* output file descriptor (stdout) 	*/
	int inbufsiz;		/* size of input buffer			*/
	int outbufsiz;		/* size of output buffer		*/
	int i;			/* counter for file processing loop	*/

	setlocale(LC_MESSAGES, "");
   	catd = catopen(MF_ICONV, NL_CAT_LOCALE);

	opterr = 0;
    	while ((c = getopt(argc, argv, "f:t:")) != EOF) {
        	switch(c) {
		case 'f': fcode = optarg; break;
		case 't': tcode = optarg; break;
		case '?':
			fprintf(stderr,
				MSGSTR(M_USAGE, 
				"Usage: iconv -f <from> -t <to> [<infile>]\n"));
			exit(1);
        	}
    	} 

    	if (!fcode || !tcode) {
		fprintf(stderr, 
		MSGSTR(M_USAGE, "Usage: iconv -f <from> -t <to> [<infile>]\n"));
     		exit(1);
    	}

	/*
	 *	open iconv converter.
	 */
	if ((cd = iconv_open(tcode, fcode)) == (iconv_t)-1) {
		fprintf(stderr,
			MSGSTR(M_CANNOTCONV, "iconv: cannot open converter\n"));
		exit(1);
	}

	inbufsiz = outbufsiz = ALLOC_UNIT;
	if (!(inbuf = malloc(inbufsiz))) {
		fprintf(stderr,
			MSGSTR(M_NOMEMORY,
			"iconv: unable to allocate enough memory\n"));
		exit(1);
	}
	if (!(outbuf = malloc(outbufsiz))) {
		free(inbuf);
		fprintf(stderr,
			MSGSTR(M_NOMEMORY,
			"iconv: unable to allocate enough memory\n"));
		exit(1);
	}

	/*
	 * If no file argument is given, then read from stdin
	 */
	if ((rc = optind) >= argc) {
		static char *stdin_argv[2] = { "-", NULL } ;

		rc = 0 ; argc = 1 ;
		argv = stdin_argv ;
	}
	for (i = rc ; i < argc ; i++) {
		if (strcmp(argv[i], "-")) { 	/* read from file, not stdin */
        		if ((fdin = open(argv[i], O_RDONLY)) < 0) {
				fprintf(stderr,
					MSGSTR(M_CANNOTFILE,
					"iconv: input file cannot be opened\n"));
            			exit(1);
        		}
    		}
		else
			fdin = 0;	/* read from stdin */

		inleft = 0;
		for (;;) {
			/*
			 * if any bytes are left over, they would be in the
			 * beginning of the buffer on the next read().  This does
			 * not apply when the loop is entered for the first time.
			 */
			inptr = inbuf;
			outptr = outbuf;
			outleft = outbufsiz;
			bytesread = read(fdin, inptr + inleft, inbufsiz - inleft);
			if (bytesread < 0) {
				perror("iconv: read");
				exit(1);
			}

			if ((inputlen = inleft += bytesread) == 0)
				break;		/* no more input */

			rc = iconv(cd, &inptr, &inleft, &outptr, &outleft);

			if (write(fdout, outbuf, outbufsiz - outleft) < 0) {
				perror("iconv: write");
				exit(1);
			}

			
			if (ICONV_TRUNC(rc)) {
				if (inbuf == inptr) {
					if (inleft != inbufsiz) {
						if (bytesread) continue;
						iconv_close(cd);
						free(inbuf);
						free(outbuf);
						/*
						 * if the last character in input
						 * file is a truncated char, 
						 * consider it an invalid character.
						 */
						fprintf(stderr, MSGSTR(M_TRUNC,
							"iconv: truncated character found\n"));
						exit(2); 
					}
					inbufsiz += ALLOC_UNIT;
					if (!(inbuf = realloc(inbuf, inbufsiz))) {
						fprintf(stderr,
						MSGSTR(M_NOMEMORY,
						"iconv: unable to allocate enough memory\n"));
						exit(1);
					}
				}
				else {
					/* Data left in input buffer for next time */
					memcpy(inbuf, inbuf + inputlen - inleft, 
						inleft);
				}
			} else if (ICONV_INVAL(rc)) {
				/* Error in the input buffer */
				fprintf(stderr,
					MSGSTR(M_INVALIDCHAR, "iconv: invalid character found\n"));
				exit(2);
			} else if (ICONV_OVER(rc)) {
				if (inbuf == inptr) {
					outbufsiz += ALLOC_UNIT;
					if (!(outbuf = realloc(outbuf, outbufsiz))) {
						fprintf(stderr,
						MSGSTR(M_NOMEMORY,
						"iconv: unable to allocate enough memory\n"));
						exit(1);
					}
				}
				else {
					/* output buffer too small to hold the 
					 * converted data.  inleft has the number 
					 * of bytes to be converted. 
					 */
					memcpy(inbuf, inbuf + inputlen - inleft, 
						inleft);
				}
			}
    		}
	if (fdin != 0)	/* Close file if it's not stdin */
		close(fdin);
	}
	iconv_close(cd);
	free(inbuf);
	free(outbuf);
    	exit(0);
/*NOTREACHED*/
	return(0);
}
