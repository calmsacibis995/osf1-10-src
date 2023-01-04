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
static char rcsid[] = "@(#)$RCSfile: unpack.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/10/11 19:30:55 $";
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
 * FUNCTIONS: unpack
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.17  com/cmd/files/unpack.c, cmdfiles, bos320, 9138320 9/12/91 18:18:59
 */
/*
 * unpack: expand files
 *	Huffman decompressor
 *	Usage:	pcat filename...
 *	or	unpack filename...
 */


#include <stdio.h>
#include <locale.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/access.h>
#include <sys/dir.h>
#include <string.h>

#include "unpack_msg.h"

nl_catd	catd;
#define MSGSTR(Num, Str) catgets(catd, MS_UNPACK, Num, Str)

static int getdict (void);
static int decode (void);
static void eprintf (char *);
static void expand (void);
static int getch (void);
static int getwd (void);
static void putch (char);
static void usage(void);

#ifdef lint
	int	_void_;
#	define VOID	_void_ = (int)
#else
#	define VOID
#endif

jmp_buf env;
struct	stat status;
static	struct	utimbuf times;
char	*argv0, *argvk;
short	errorm;

#define NAMELEN PATH_MAX+1
#define MAXBASE NAME_MAX-1
#define SUF0	'.'
#define SUF1	'z'
#define US	037
#define RS	036

#define BLKSIZE BUFSIZ


/* variables associated with i/o */
char	filename[NAMELEN+2];
short	infile;
short	outfile;
short	inleft;
char	*inp;
char	*outp;
char	inbuff[BUFSIZ];
char	outbuff[BUFSIZ];

/* the dictionary */
long	origsize;
short	maxlev;
short	intnodes[25];
char	*tree[25];
char	characters[256];
char	*eof;

/*
 * NAME: getdict
 *                                                                    
 * FUNCTION: read in the dictionary portion and build decoding 
 *           structures, return 1 if successful, 0 otherwise 
 */
static int
getdict (void)
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
	inleft = read (infile, &inbuff[0], BUFSIZ);
	if (inleft < 0) {
		eprintf (MSGSTR(READERR, ".z: read error"));
		return (0);
	}
	if (inbuff[0] != US)
		goto goof;

	if (inbuff[1] == US) {		/* oldstyle packing */
		if (setjmp (env))
			return (0);
		expand ();
		return (1);
	}
	if (inbuff[1] != RS)
		goto goof;

	inp = &inbuff[2];
	origsize = 0;
	for (i=0; i<4; i++)
		origsize = origsize*256 + ((*inp++) & 0377);
	maxlev = *inp++ & 0377;
	if (maxlev > 24) {
goof:		eprintf (MSGSTR(NOTPCKD, ".z: not in packed format"));
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
	inleft -= inp - &inbuff[0];
	if (inleft < 0)
		goto goof;
	if (inleft == 0 && origsize == 0)
		return (1);

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
	return (decode ());
}

/*
 * NAME: decode
 *                                                                    
 * FUNCTION: unpack the file, return 1 if successful, 0 otherwise 
 */
static int
decode (void)
{
	int bitsleft, c, i;
	int j, lev;
	char *p;

	outp = &outbuff[0];
	lev = 1;
	i = 0;
	while (1) {
		if (inleft <= 0) {
			inleft = read (infile, inp = &inbuff[0], BUFSIZ);
			if (inleft < 0) {
				eprintf (MSGSTR(RDERR1, ".z: read error"));
				return (0);
			}
		}
		if (--inleft < 0) {
uggh:			eprintf (MSGSTR(UNPCKERR, ".z: unpacking error"));
			return (0);
		}
		c = *inp++;
		bitsleft = 8;
		while (--bitsleft >= 0) {
			i *= 2;
			if (c & 0200)
				i++;
			c <<= 1;
			if ((j = i - intnodes[lev]) >= 0) {
				p = &tree[lev][j];
				if (p == eof) {
					c = outp - &outbuff[0];
					if (write (outfile, &outbuff[0], c) != c) {
wrerr:						eprintf (MSGSTR(WRITERR, ": write error"));
						return (0);
					}
					origsize -= c;
					if (origsize != 0)
						goto uggh;
					return (1);
				}
				*outp++ = *p;
				if (outp == &outbuff[BUFSIZ]) {
					if (write (outfile, outp = &outbuff[0], BUFSIZ) != BUFSIZ)
						goto wrerr;
					origsize -= BUFSIZ;
				}
				lev = 1;
				i = 0;
			} else
				lev++;
		}
	}
}

/*
 * NAME: unpack
 *                                                                    
 * FUNCTION: expand packed files
 */  
int
main (int argc, char *argv[])
{
	int i, k;
	int sep, pcat = 0;
	char *cp;
	int fcount = 0;		/* failure count */

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_UNPACK, NL_CAT_LOCALE);

	argv0 = basename(argv[0]);

	if(argv0[0] == 'p')
		pcat++;	/* User entered pcat (or /xx/xx/pcat) */

	if (argc > 1 && strcmp(argv[1], "--") == 0) {  /* support -- for XPG4 */
		argc--;
		argv++;
	}
	if (argc <= 1)
		usage();

	for (k=1; k<argc; k++) {
		errorm = 0;
		sep = -1;
		cp = filename;
		argvk = argv[k];
		for (i=0; i < (NAMELEN-3) && (*cp = argvk[i]); i++)
			if (*cp++ == '/')
				sep = i;
		if (cp[-1] == SUF1 && cp[-2] == SUF0) {
			argvk[i-2] = '\0'; /* Remove suffix and try again */
			k--;
			continue;
		}

		fcount++;	/* expect the worst */
		if (i >= (NAMELEN-3) || (i - sep) > MAXBASE) {
			eprintf (MSGSTR(NMLENERR, ": file name too long"));
			goto done;
		}
		*cp++ = SUF0;
		*cp++ = SUF1;
		*cp = '\0';
		if ((infile = open (filename, 0)) == -1) {
			eprintf (MSGSTR(CANTOPNZ, ".z: cannot open"));
			goto done;
		}

		if (pcat)
			outfile = 1;	/* standard output */
		else {
			if (stat (argvk, &status) != -1) {
				eprintf (MSGSTR(FILEXST, ": already exists"));
				goto done;
			}
			VOID fstat ((int)infile, &status);
			if (status.st_nlink != 1) {
				eprintf (MSGSTR(LINKERR, 
					".z: Warning: file has links"));
			}


			/* create file with minimum permissions. */
			/* Assign permissions later */
			if ((outfile = creat (argvk, 0600)) == -1) {
				eprintf (MSGSTR(CREATERR, ": cannot create"));
				goto done;
			}

			VOID chown (argvk, status.st_uid, status.st_gid);
		}


		if (getdict ()) {	/* unpack */
			fcount--; 	/* success after all */
			if (!pcat) {
				eprintf (MSGSTR(UNPCKD, ": unpacked"));
				VOID unlink (filename);

				VOID chmod (argvk, status.st_mode);
				/*
				 * preserve acc & mod dates
				 */
				times.actime = status.st_atime;
				times.modtime = status.st_mtime;
				VOID utime (argvk, &times);
			}
		}
		else
			if (!pcat)
				VOID unlink (argvk);
done:		if (errorm)
			VOID fprintf (stderr, "\n");
		VOID close (infile);
		if (!pcat)
			VOID close (outfile);
	}
	return (fcount);
}

/*
 * NAME: eprintf
 *                                                                    
 * FUNCTION: print error messages
 */  
static void
eprintf (char *s)
{
	if (!errorm) {
		errorm = 1;
		VOID fprintf (stderr, "%s: %s", argv0, argvk);
	}
	VOID fprintf (stderr, s);
}

/*
 * NAME: expand
 *                                                                    
 * FUNCTION:  This code is for unpacking files that
 *            were packed using the previous algorithm.
 */
int	Tree[1024];
static void
expand (void)
{
	int tp, bit;
	short word;
	int keysize, i, *t;

	outp = outbuff;
	inp = &inbuff[2];
	inleft -= 2;
	origsize = ((long) (unsigned) getwd ())*256*256;
	origsize += (unsigned) getwd ();
	t = Tree;
	for (keysize = getwd (); keysize--; ) {
		if ((i = getch ()) == 0377)
			*t++ = getwd ();
		else
			*t++ = i & 0377;
	}

	bit = tp = 0;
	for (;;) {
		if (bit <= 0) {
			word = getwd ();
			bit = 16;
		}
		tp += Tree[tp + (word<0)];
		word <<= 1;
		bit--;
		if (Tree[tp] == 0) {
			putch (Tree[tp+1]);
			tp = 0;
			if ((origsize -= 1) == 0) {
				(void)write (outfile, outbuff, outp - outbuff);
				return;
			}
		}
	}
}

/*
 * NAME: getch
 *                                                                    
 * FUNCTION:  get next character from input file
 */  
static int
getch (void)
{
	if (inleft <= 0) {
		inleft = read (infile, inp = inbuff, BUFSIZ);
		if (inleft < 0) {
			eprintf (MSGSTR(ZRDERR, ".z: read error"));
			longjmp (env, 1);
		}
	}
	inleft--;
	return (*inp++ & 0377);
}

/*
 * NAME: getwd
 *                                                                    
 * FUNCTION:  get word from file
 */  
static int
getwd (void)
{
	char c;
	int d;
	c = getch ();
	d = getch ();
	d <<= 8;
	d |= c & 0377;
	return (d);
}

/*
 * NAME: putch
 *                                                                    
 * FUNCTION: put character into out buffer
 */  
static void
putch (char c)
{
	int n;

	*outp++ = c;
	if (outp == &outbuff[BUFSIZ]) {
		n = write (outfile, outp = outbuff, BUFSIZ);
		if (n < BUFSIZ) {
			eprintf (MSGSTR(WRITERR1, ": write error"));
			longjmp (env, 2);
		}
	}
}

static void
usage(void)
{
	fprintf(stderr, MSGSTR(USAGE, "Usage: %s file...\n"), argv0);
	exit(1);
}
