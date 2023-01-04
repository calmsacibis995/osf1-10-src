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
static char rcsid[] = "@(#)$RCSfile: wc.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/10/11 19:54:18 $";
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
 * FUNCTIONS: wc
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.11  com/cmd/files/wc.c, cmdfiles, bos320, 9130320 7/2/91 16:44:57
 */

#include        <ctype.h>
#include	<wchar.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/stat.h>
#include	<locale.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>

#include 	"wc_msg.h"

#define MAXPARMS 2
#define MSGSTR(Num,Str) catgets(catd,MS_WC,Num,Str)

nl_catd catd;

unsigned char buf[BUFSIZ+1];     /* buffer for file/stdin reads */
unsigned char wd[MAXPARMS+1];    /* holds parameter list */
unsigned char *wdp;		 /* pointer to parameter list */

wchar_t wc;		    /* used for wide char processing */
int	mb_cur_max;	    /* the maximum no. of bytes for multibyte
			       characters in the current locale */

static void wcp (long int, long int, long int, long int);

/*
 * NAME: wc [-klwc] names
 *                                                                    
 * FUNCTION: Counts the number of lines, words and characters in a file.
 *           -k       counts actual characters, not bytes.
 *           -l       counts lines only.
 *           -w       counts words only.
 *           -c       counts bytes only. 
 */  
void
main(int argc, char **argv)
{
	unsigned char *p1, *p2;
	char *curp,*cure;		/* current and endpointers in the 
					   buffer */

	int bytesread;			/* no. of bytes read from disk */
	int mbcnt;			/* no. of bytes in a character */
	int fd = 0;			/* file descripter, default stdin */
	int leftover;			/* no. of bytes left over in the  
					   buffer that need remaining bytes
					   for conversion to wide char */

	int filect = 0;			/* file count */
	long wordct;			/* word count */
	long twordct=0;			/* total word count */
	long linect;			/* line count */
	long tlinect=0;			/* total line count */
	long bytect;			/* byte count */
	long tbytect=0;			/* total byte count */
	long charct;			/* actual character count */
	long tcharct=0;			/* total character count */
	int mflag = 0;	    		/* 'm' parameter */
	int lflag = 0;		    	/* 'l' parameter */
	int wflag = 0;			/* 'w' parameter */
	int cflag = 0;          	/* 'c' parameter */
	int c;
	int token;
	int status = 0;

	int mbcodeset;			/* Boolean flag to indicate if 
					   current code set is a multibyte
					   code set. */

	(void) setlocale(LC_ALL,"");

	catd = catopen(MF_WC, NL_CAT_LOCALE);
					
	wdp = wd;
	while ((c=getopt(argc,argv,"mklcw")) != EOF) {
		switch (c) {
		      case 'k':
		      case 'm':
				if (!mflag++)
				    *wdp++ = c;
				break;
		      case 'l':	
				if (!lflag++)
				    *wdp++ = c;
				break;
		      case 'w':
				if (!wflag++)
				    *wdp++ = c;
				break;
		      case 'c':
				if (!cflag++)
				    *wdp++ = c;
				break;
		      default:
				fprintf(stderr,MSGSTR(USAGE, "usage: wc [-clmw] [file ...]\n")); 
				exit(2);
		}
	}

	*wdp = '\0';			/* terminate with delimiter */
	wdp = wd;			/* point back to beginning */

	if (!mflag & !lflag & !wflag & !cflag)   /* default mode "lwc" */
	    strcpy((char *)wdp,"lwc");		

	mb_cur_max = MB_CUR_MAX;	/* max no. of bytes in a multibyte
					   char for current locale. */

	if (mb_cur_max > 1)
	    mbcodeset = 1;
	else
	    mbcodeset = 0;

	do {
		if(optind < argc) {
		    if ((fd=open(argv[optind], O_RDONLY)) < 0) {
			fprintf(stderr,MSGSTR(CANTOPEN,   
				"wc: %s : %s\n"), argv[optind], strerror(errno)); 
			status = 2;
			continue;
		    }
		    else filect++;
		}
		p1 = p2 = buf;
		linect = 0;
		wordct = 0;
		bytect = 0;
		token = 0;
		charct = 0;

  /* count lines, words and characters but check options before printing */

		if (mbcodeset) { 		/* I18N support */
		    leftover = 0;
		    for(;;) {
                        if (!(bytesread = read(fd, (char *)buf+leftover, BUFSIZ-leftover)))
                            break;
                        else if (bytesread < 0) {
			    fprintf(stderr,MSGSTR(EREAD,   
			    	    "wc: %s : %s\n"), argv[optind], strerror(errno)); 
                            exit(2);
                        }
			buf[leftover+bytesread] = '\0';  /* protect partial
							    reads */
			bytect += bytesread;
			curp = (char *) buf;
			cure = (char *) (buf + bytesread + leftover);
			leftover = 0;
			for (;curp < cure;) {
			    /* convert to wide character */
			    mbcnt = mbtowc(&wc, curp, mb_cur_max);
			    if (mbcnt <= 0) {
				if (mbcnt == 0) {
				     /* null string, handle exception */	
				     mbcnt = 1;
				}
				else if (cure-curp >= mb_cur_max) {
					/* invalid multibyte.  handle */
					/* one byte at a time */
					wc = *(unsigned char *)curp;
					mbcnt = 1;
				     }
				     else {
					/* needs more data from next read */
					leftover = cure - curp;
					strncpy((char *)buf, curp, leftover);
					break;
				     }
			    }
			    curp += mbcnt;
			    charct++;

			    /* count real characters */
			    if (!iswspace(wc)) {
				if (!token) {
				 	wordct++;
					token++;
				}
				continue;
			    }
			    token = 0;
			    if (wc == L'\n') linect++;
			} 		/* end wide char conversion */
		    }  			/* end read more bytes */
		}
		else { 				/* single byte support */
		    for(;;) {
			if(p1 >= p2) {
				p1 = buf;
                                if (!(c = read(fd, (char *)p1, BUFSIZ)))
                                    break;
                                else if (c < 0) {
			            fprintf(stderr,MSGSTR(EREAD,   
			    	            "wc: %s : %s\n"), argv[optind], strerror(errno)); 
                                    exit(2);
                                }
				bytect += c;
				charct += c;
				p2 = p1 + c;
			}
			c = *p1++;
			if (c > ' ') {
		 	    if(!token) {
				wordct++;
				token++;
		 	    }
		 	    continue;
			}
			/* reordered these cases to get a bit more speed */
			if (c == ' ' || c == '\t')
			     token = 0;
			else if (c == '\n') {
				token = 0;
				linect++;
			}
		    }      /* end for loop */
		}          /* end dual path for I18N */

		/* print lines, words, chars/bytes */
		wcp(linect, wordct, charct, bytect);

		if (filect) 
		    printf(" %s\n", argv[optind]);
		else
		    printf("\n");

		close(fd);
		tlinect += linect;
		twordct += wordct;
		tbytect += bytect;
		tcharct += charct;
	} while(++optind<argc);		/* process next file */

	if(filect >= 2) {		/* print totals for multiple files */
		wcp(tlinect, twordct, tcharct, tbytect);
		printf(MSGSTR(TOTAL," total\n"));   
	}
	exit(status);
}

/*
 * NAME: wcp
 *                                                                    
 * FUNCTION: check options then print out the requested numbers.
 *           Note:  the pointer used by wcp is external to wcp.      
 */  
static void
wcp(long int linect, long int wordct, long int charct, long int bytect)
{
	char *wdp = (char *) wd; /* Private copy for checking flags */


	while (*wdp) {
	   switch (*wdp++) {
		case 'l':
	   		printf(" %9lu", linect);
			break;
		case 'w':
	   		printf(" %9lu", wordct);
			break;
		case 'c':
	   		printf(" %9lu", bytect);
			break;
		case 'm':
	   		printf(" %9lu", charct);
			break;
	   }
	}
}

