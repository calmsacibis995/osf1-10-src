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
static char	*sccsid = "@(#)$RCSfile: pcat.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 17:42:13 $";
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
 * FUNCTIONS: pcat
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * pcat.c	1.6  com/cmd/files,3.1,9013 11/18/89 12:41:59
 */
/*
 * pcat: view packed files
 *	Huffman decompressor  (new files to standard output)
 *	Usage:	pcat filename...
 */


#include "pcat_msg.h"
#define MSGSTR(id,ds) catgets(catd, MS_PCAT, id, ds)
static nl_catd catd;
#include <stdio.h>
#include <locale.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

jmp_buf env;
char	*argv0, *argvk;
short	errorm;

#define	BLKSIZE	512
#define NAMELEN PATH_MAX+1
#define MAXBASE NAME_MAX-1
#define	SUF0	'.'
#define	SUF1	'z'
#define US 037
#define RS 036

/* variables associated with i/o */
char	filename [NAMELEN+2];
short	infile;
short	outfile;
short	inleft;
char	*inp;
char	*outp;
char	inbuff [BLKSIZE];
char	outbuff [BLKSIZE];

/* the dictionary */
long	origsize;
short	maxlev;
short	intnodes [25];
char	*tree [25];
char	characters [256];
char	*eof;

/*
 * NAME: getdict
 *                                                                    
 * FUNCTION:  read in the dictionary portion and build decoding 
 *            structures, return 1 if successful, 0 otherwise 
 */
getdict ()
{
	int c, i, nchildren;
	/*
	 * check two-byte header
	 * get size of original file,
	 * get number of levels in maxlev,
	 * get number of leaves on level i in intnodes[i],
	 * set tree[i] to point to leaves for level i
	 */
	eof = &characters[0];
	inbuff[6] = 25;
	inleft = read(infile, &inbuff[0], BLKSIZE);
	if (inleft < 0) {
		eprintf (MSGSTR(M_BADRD, ".z: read error"));
		return (0);
	}
	if (inbuff[0] != US) goto goof;
#ifdef COMPATABILITY
	if (inbuff[1] == US) { /* oldstyle packing */
		if (setjmp(env)) return (0);
		expand();
		return (1);
	}
#endif
	if (inbuff[1] != RS) goto goof;

	inp = &inbuff[2];
	origsize = 0;
	for (i=0; i<4; i++) {
		origsize = origsize*256 + ((*inp++) & 0377);
	}
	maxlev = *inp++ & 0377;
	if (maxlev > 24) {
goof:		eprintf (MSGSTR(M_FORMT, ".z: not in packed format"));
		return (0);
	}
	for (i=1; i<=maxlev; i++)
		intnodes[i] = *inp++ & 0377;
	for (i=1; i<=maxlev; i++) {
		tree[i] = eof;
		for (c=intnodes[i]; c>0; c--) {
			if (eof >= &characters[255])
				goto goof;
			*eof++ = *inp++;
		}
	}
	*eof++ = *inp++;
	intnodes[maxlev] += 2;
	inleft -= inp-&inbuff[0];
	if (inleft < 0)
		goto goof;

	/*
	 * convert intnodes[i] to be number of
	 * internal nodes possessed by level i
	 */
	nchildren = 0;
	for (i=maxlev; i>=1; i--) {
		c = intnodes[i];
		intnodes[i] = nchildren /= 2;
		nchildren += c;
	}
	return (decode());
}

/*
 * NAME: decode
 *                                                                    
 * FUNCTION:  unpack the file, return 1 if successful, 0 otherwise 
 */
decode ()
{
	int bitsleft, c, i;
	int j, lev;
	char *p;
	outp = &outbuff[0];
	lev = 1;
	i = 0;
	while (1) {
		if (inleft <= 0) {
			inleft = read(infile, inp = &inbuff[0], BLKSIZE);
			if (inleft < 0) {
				eprintf (MSGSTR(M_BADRD, ".z: read error"));
				return (0);
			}
		}
		if (--inleft < 0) {
uggh:			eprintf (MSGSTR(M_FAILD, ".z: unpacking error"));
			return (0);
		}
		c = *inp++;
		bitsleft = 8;
		while (--bitsleft >= 0) {
			i *= 2;
			if (c & 0200)
				i++;
			c <<= 1;
			if ((j = i-intnodes[lev]) >= 0) {
				p = &tree[lev][j];
				if (p == eof) {
					c = outp-&outbuff[0];
					if (write(outfile, outbuff, c) != c) {
wrerr:						eprintf (MSGSTR(M_BADWR, ": write error"));
						return (0);
					}
					origsize -= c;
					if (origsize != 0) goto uggh;
					return(1);
				}
				*outp++ = *p;
				if (outp == &outbuff[BLKSIZE]) {
					if (write(outfile, outp = &outbuff[0], BLKSIZE) != BLKSIZE)
						goto wrerr;
					origsize -= BLKSIZE;
				}
				lev = 1;
				i = 0;
			} else
				lev++;
		}
	}
}


/*
 * NAME: pcat file
 *                                                                    
 * FUNCTION: view packed files
 */  
main(argc, argv)
short argc; char *argv[];
{       int i, k;
	int sep;
	char *cp;
	int fcount = 0; /* failure count */

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_PCAT,NL_CAT_LOCALE);
	argv0 = argv[0];
	for (k = 1; k<argc; k++)
	{
		errorm = 0;
		sep = -1;  cp = filename;
		for (i=0; i < (NAMELEN-3) && (*cp = argv[k][i]); i++)
			if (*cp++ == '/') sep = i;
		if (cp[-1]==SUF1 && cp[-2]==SUF0)
		{       argv[k][i-2] = '\0'; /* Remove suffix and try again */
			k--;
			continue;
		}

		fcount++;  /* expect the worst */
		argvk = argv[k];
		if (i >= (NAMELEN-3) || (i - sep) > MAXBASE) 
		{       eprintf (MSGSTR(M_2LONG, ": file name too long"));
			goto done;
		}
		*cp++ = SUF0;  *cp++ = SUF1;  *cp = '\0';
		if ((infile = open(filename, 0)) == -1)
		{       eprintf (MSGSTR(M_NOPEN, ".z: cannot open"));
			goto done;
		}

		outfile = 1; 	/* standard output */

		if (getdict()) {		/* unpack */
			fcount--; 	/* success after all */
		}
		close(infile);
done:		if (errorm) fprintf (stderr, "\n");
	}
	return (fcount);
}

eprintf(s) char *s; {
	if (!errorm) {
		errorm = 1;
		fprintf (stderr, "%s: %s", argv0, argvk);
	}
	fprintf (stderr, s);
}

#ifdef COMPATABILITY
/*
 * NAME: expand
 *                                                                    
 * FUNCTION:  this code is for unpacking files, which were packed 
 *            using the previous algorithm 
 */
int Tree [1024];
expand()
{       int tp, bit, word;
	int keysize, i, *t;

	outp = outbuff;
	inp = &inbuff[2];
	inleft -= 2;
	origsize = ((long int ) (unsigned) getwd())*256*256;
	origsize += (unsigned) getwd();
	t = Tree;
	for ( keysize=getwd(); keysize--; )
	{       if ((i = getch()) == 0377)
			*t++ = getwd();
		else
			*t++ = i & 0377;
	}

	bit = tp = 0;
	for (;;)
	{       if (bit <= 0)
		{       word = getwd();
			bit = 16;
		}
		tp += Tree[tp + (word<0)];
		word <<= 1;  bit--;
		if (Tree[tp] == 0)
		{       putch(Tree[tp+1]);
			tp = 0;
			if ((origsize -= 1) == 0) {
				write (outfile, outbuff, outp-&outbuff[0]);
				return;
			}
		}
	}
}

getch() {
	if (inleft <= 0) {
		inleft = read (infile, inp=inbuff, BLKSIZE);
		if (inleft < 0) {
			eprintf (MSGSTR(M_BADRD, ".z: read error"));
			longjmp (env, 1);
		}
	}
	inleft--;
	return (*inp++ & 0377);
}

getwd() {
	char c;
	int d;
	c = getch();
	d = getch();
	d <<= 8;
	d |= (c & 0377);
	return (d);
}

putch(c) char c; {
	int n;
	*outp++ = c;
	if (outp == &outbuff[BLKSIZE]) {
		n = write (outfile, outp=outbuff, BLKSIZE);
		if (n < BLKSIZE) {
			eprintf (MSGSTR(M_BADWR, ": write error"));
			longjmp (env, 2);
		}
	}
}
#endif
