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
static char rcsid[] = "@(#)$RCSfile: comm.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 16:35:59 $";
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
 * FUNCTIONS: comm
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.10  com/cmd/files/comm.c, cmdfiles, bos320, 9132320b 7/17/91 14:31:15
 */
/*
**	process common lines of two files
*/

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <unistd.h>
#include <sys/limits.h>
#include <ctype.h>
#include "comm_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_COMM,Num,Str)

int	one;       /* display flags */
int	two;
int	three;

char	*ldr[3];    /* display spacing holder */

FILE	*ib1;         /* pointer to file1 */
FILE	*ib2;         /* pointer to file2 */
FILE	*openfil();     

/*
 * NAME: comm [ - [123] ] file1 file2
 *                                                                    
 * FUNCTION: Selects or rejects lines common to two sorted files.
 *           If you specify - for one of the file names, comm
 *           reads standard in.
 *           comm reads file1 and file2 and writes by default, a three-
 *           column ouput to standard output.
 *           column              output
 *             1           lines that are in only file1
 *             2           lines that are in only file2
 *             3           lines common to both files
 *           flags:
 *            -1           suppresses display of first column
 *            -2           suppresses display of second column
 *            -3           suppresses display of third column
 *
 * NOTE:  Both file1 and file2 should be sorted according to the collating
 *         sequence specified by the environment variable LC_COLLATE.
 */  
main(argc,argv)
char **argv;
{
	int	l;             /* index for display spacing */
	char	lb1[MAX_INPUT],lb2[MAX_INPUT];   /* input buffers */
	int	oc,badopt;
	int	strcollresult;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_COMM,NL_CAT_LOCALE);

	ldr[0] = "";            /* display spacing: column 1 */
	ldr[1] = "\t";                           /* column 2 */
	ldr[2] = "\t\t";                         /* column 3 */
	badopt = 0;
	l = 1;
	while ((oc = getopt(argc,argv,"123")) != -1 ) {
		switch(oc) {
		case '1':
			if(!one) {
				one = 1;
				ldr[2] = ldr[l--];
				ldr[1] = ldr[0];
			}
			break;
		case '2':
			if(!two) {
				two = 1;
				ldr[2] = ldr[l--];
			}
			break;
		case '3':
			three = 1;
			break;
		default:
			badopt++;	/* Bad option or option syntax */
		} /* switch(oc) */
	} /* while((oc=getopt...)!=-1) */
	if (badopt || argc-optind != 2)
		usage();
	ib1 = openfil(argv[optind]);         /* open file1 an file2 */
	ib2 = openfil(argv[optind+1]);

	if(rd(ib1,lb1) < 0) {         /* if read line from file1 is bad */
		if(rd(ib2,lb2) < 0)   /* if bad read line from file2 */
			exit(0);      /* exit program */
		copy(ib2,lb2,2);      /* else copy rest file2 to screen */
	}                         /* else  */
	if(rd(ib2,lb2) < 0)        /* if read from file 2 bad */
		copy(ib1, lb1, 1);    /* copy rest of file1 to screen */
	while(1) {              /* else */
		strcollresult = ( (strcollresult = strcoll(lb1,lb2)) == 0 ? strcollresult :
					(strcollresult < 0 ? 1 : 2));
		switch(strcollresult) {
			case 0:        /* lines equal */
				wr(lb1,3);    /* write in column 3 */
				if(rd(ib1,lb1) < 0) {  /* get next line file1 */
					if(rd(ib2,lb2) < 0)
						exit(0);
					copy(ib2,lb2,2);
				}
				if(rd(ib2,lb2) < 0)   /* get next line file2 */
					copy(ib1, lb1, 1);
				continue;

			case 1:    /* line1 less than line2 */
				wr(lb1,1);   /* write in column 1 */
				if(rd(ib1,lb1) < 0)  /* get next line file1 */
					copy(ib2, lb2, 2);
				continue;

			case 2:     /* line1 greater that line 2 */
				wr(lb2,2);       /* write in column 2 */
				if(rd(ib2,lb2) < 0) /* get next line file2 */
					copy(ib1, lb1, 1);
				continue;
		}
	}
}

/*
 * NAME: rd
 *                                                                    
 * FUNCTION:  read a line from a file.
 *                                                                    
 * RETURN VALUE DESCRIPTION:
 *          0 - successfully reading a line.
 *         -1 - error.
 */  
rd(file,buf)
FILE *file;
char *buf;
{

	int i, j;
	i = j = 0;

	while((j = getc(file)) != EOF) {  /* read character by character */
		*buf = j;
		if(*buf == '\n' || i > MAX_INPUT-2) {
			*buf = '\0';
			return(0);
		}
		i++;
		buf++;
	}
	return(-1);
}

/*
 * NAME: wr
 *                                                                    
 * FUNCTION: write string to standarded out in the proper column.
 */  
wr(str,n)
char *str;
{
	switch(n) {
		case 1:          /* check display flags */
			if(one)
				return;
			break;

		case 2:
			if(two)
				return;
			break;

		case 3:
			if(three)
				return;
	}                           /* if display flag not set then print */
	printf("%s%s\n",ldr[n-1],str); /* using display spacing characters */
}

/*
 * NAME: copy
 *                                                                    
 * FUNCTION: copy the rest of file to standard out in the proper column
 *           then exit program.
 */  
copy(ibuf,lbuf,n)
FILE *ibuf;
char *lbuf;
{
	do {
		wr(lbuf,n);
	} while(rd(ibuf,lbuf) >= 0);

	exit(0);
}

/*
 * NAME: openfil
 *                                                                    
 * FUNCTION:  open a file or use stdin
 *
 * RETURN VALUE DESCRIPTION:  returns a file pointer.
 */  
FILE *openfil(s)
char *s;
{
	FILE *b;
	if(s[0]=='-' && s[1]==0)
		b = stdin;
	else if((b=fopen(s,"r")) == NULL) {
		fprintf(stderr,MSGSTR(CANTOPEN,"comm: %s : %s\n"), s, strerror(errno));
		exit(2);
	}
	return(b);
}

/*
 * NAME: usage
 *                                                                    
 * FUNCTION:  displays the correct usage for comm.
 */  
usage()
{
	fprintf(stderr,MSGSTR(USAGE,"usage: comm [ -123 ] file1 file2\n")); /*MSG*/
	exit(2);
}
