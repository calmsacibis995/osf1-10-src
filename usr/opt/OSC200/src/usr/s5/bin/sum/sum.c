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
static char	*sccsid = "@(#)$RCSfile: sum.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:57:51 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: sum
 *
 * ORIGINS: 3, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sum.c	1.12  com/cmd/files,3.1,9021 4/30/90 20:57:27
 */
/*
 * Sum bytes in file mod 2^16
 */

#include <stdio.h>
#include <locale.h>

#include "sum_msg.h"
/* there're only two messages, so efficency isn't an issue */
#define MSGSTR(Num,Str) NLgetamsg("sum.cat",MS_SUM,Num,Str) 

#define WDMSK 0177777L
#define BLOCK_SIZE	512		/*512 byte units for SVID-2 compliance*/

int	errflg = 0;
int 	oflag = 1;
int 	rflag = 0;

FILE	*f;
int	i;

/*
 * NAME: sum [-r|-o] file
 *                                                                    
 * FUNCTION: Displays the checksum and block count of a file.
 *   FLAGS: 
 *    -r     computes the checksum (rigorous byte-by-byte).
 *    -o     Algorithm computes the checksum using word by word computation.
 *		force -o to be default for SVID-2 compatibility
 */  
main(argc, argv)
int   argc;
char *argv[];
{
	(void) setlocale(LC_ALL,"");
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {

			/* avoid sum -ro or sum -or */
			if(argv[i][2] != '\0') {
				(void) fprintf(stderr, MSGSTR(USAGE,
					"usage: sum [-r|-o] file\n"));
				exit(1);
			}
				
			switch (argv[i][1]) {
			case 'o':
				break;
			case 'r':
				rflag++;
				oflag = 0;	/* for SVID-2 */
				break;
			default:
				(void) fprintf(stderr, MSGSTR(USAGE,
					"usage: sum [-r|-o] file\n"));
				exit(1);
			}
		}
		else {
			break;
		}
	}

	do {
		if (i < argc) {
			if ((f = fopen(argv[i], "r")) == NULL) {
				(void) fprintf(stderr, MSGSTR(OPENERR , 
					"sum: Can't open %s\n"), argv[i]);
				errflg += 10;
				continue;
			}
		}
		else
			f = stdin;

		if (oflag)
			sysV_sum(argc, argv);
		else
			bsd_sum(argc, argv);

		fclose(f);

	} while (++i < argc);

	exit(errflg);
}


/*
 * Execute a byte-by-byte computation (BSD4.3).
 */
bsd_sum(argc, argv)
int  argc;
char *argv[];
{
	register unsigned sum;
	register int c;
	register long nbytes;

	sum = 0;
	nbytes = 0;

	while ((c = getc(f)) != EOF) {
		nbytes++;
		if (sum&01)
			sum = (sum>>1) + 0x8000;
		else
			sum >>= 1;
		sum += c;
		sum &= 0xFFFF;
	}

	if (ferror(f)) {
		errflg++;
		(void) fprintf(stderr, MSGSTR(READERR, 
			"sum: read error on %s\n"), argv[i]?argv[i]:"-");
	}

	printf("%05u%6ld %s\n", sum, (nbytes + BLOCK_SIZE - 1) / BLOCK_SIZE, argv[i]?argv[i]:"" );
}



struct part {
	short unsigned hi,lo;
};
union hilo { /* this only works right in case short is 1/2 of long */
	struct part hl;
	long	lg;
} tempa, suma;


/*
 * Execute a word-by-word computation (AIX/SYS V).
 */
sysV_sum(argc, argv)
int  argc;
char *argv[];
{
	register long nbytes;
	register int ca;
	unsigned lsavhi, lsavlo;

	suma.lg = 0;
	nbytes = 0;

	while ((ca = getc(f)) != EOF) {
		nbytes++;
		suma.lg += ca & WDMSK;
	}

	if (ferror(f)) {
		errflg++;
		fprintf(stderr, MSGSTR(READERR,"sum: read error on %s\n"),
				argc > 1 ? argv[i] : "-");
	}

	tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
	lsavhi = (unsigned) tempa.hl.hi;
	lsavlo = (unsigned) tempa.hl.lo;
	printf("%u %ld", (unsigned) (lsavhi + lsavlo), 
				(nbytes+BLOCK_SIZE-1)/BLOCK_SIZE);

	/* always print file name , added for SVID-2 */
	printf(" %s", argv[i] == (char *) 0 ? "" : argv[i]);
	printf("\n");
}
