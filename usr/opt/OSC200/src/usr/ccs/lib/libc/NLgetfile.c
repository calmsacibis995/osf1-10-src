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
static char	*sccsid = "@(#)$RCSfile: NLgetfile.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 14:57:47 $";
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLgetfile, readbuf
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
 * NLgetfile.c	1.10  com/lib/c/nls,3.1,8943 9/25/89 19:00:51
 */

#include <defenv.h>
#include <string.h>
#include <fcntl.h>
#include <sys/NLchar.h>
#include <sys/stat.h>

#ifndef NULL
#define NULL (char *) 0
#endif

#define FAIL -1


struct NLctab *_NLctab = (struct NLctab *) 0;
struct NLcoldesc *_NLcoldesc = (struct NLcoldesc *) 0;
struct envpair *__paramtab = (struct envpair *) 0;

long NLfile_count=0;

/* NLfile_count is a sequence counter which tells other library routines,
   like NLstrtime(), when to flush cache of things in environment. */

static int readfbuf();
extern char *getenv();
extern char *sbrk();

/*
 * NAME: NLgetfile
 *
 * FUNCTION: Let an application temporarily change the NLFILE environment
 * 	     parameters and thereby select the current language. 
 *
 * RETURN VALUE DESCRIPTION: 0 upon success; -1 upopn failure.
 */
int
NLgetfile(char *s)
{
	NLfile_count++;

	/* set up environment list header structure */
	__paramtab = (struct envpair *) sbrk (sizeof (struct envpair));
	__paramtab->name = __paramtab->def = NULL ;
	__paramtab->next = (struct envpair *) 0;

	if (readfbuf (s) == FAIL)
		return(FAIL);
	else {
		__ovflg = (s != NULL);
		__bufp = __filbuf;
		NLgetctab(0);
		return(0);
	}
}

/*
 * NAME: readfbuf
 *
 * FUNCTION: allocate a buffer and read a file into the buffer.
 *
 * RETURN VALUE DESCRIPTION: 0 upon success; -1 upon failure.
 */
static int
readfbuf(infile)
char *infile;	/*----  a file name  ----*/
{
	int fd;	/*----  file descriptor  ----*/
	static short usemalloc = 0;	/*----  is malloc used ?  ----*/
	static short pastfirst = 0;	/*----  more than once  ----*/
	static unsigned long sbrksiz = 0;
	static char *sbrkreg;
	unsigned long bsiz;
	struct stat sbuf;

	/* If definition buffer not already set, read the file */

	if (!infile || !(*infile) ) {
		infile = getenv(NFILVAR);

		/* No file?  Time to go! */

		if (!infile || !(*infile)) {

			/*
			 * Free space only when malloc() was called
			 * initially.
			 */
			if ( usemalloc ) {
				free (__filbuf);
				usemalloc = 0;
			}
			__filbuf = NULL;
			return (NULL);
		}
	}

	/* If a file is obtained, stat it so as to determine
	   buffer size
	*/

	if (!stat(infile, &sbuf))
		bsiz = sbuf.st_size;

	/* but if we can't stat the file, get out! */

	else
		return (FAIL);

	if ((fd = open(infile, O_RDONLY, 0)) < 0)
		return (FAIL);
	else {
		/*
		 * Free buffer if malloc was called before.
		 */
		if ( usemalloc ) {
			free (__filbuf);
			usemalloc = 0;
		}

		/* 
		 * Malloc() is called only after the first time NLgetfile()
		 * is called.  This suppresses calls to malloc() in programs
		 * that have their own malloc() defined.
		 */
		if (pastfirst) {
			/* 
			 * We can now use malloc() to get space after
			 * the first time.  However, we can avoid calling
			 * malloc() if there's enough room in the sbrk'd space.
			 */
			if (bsiz > sbrksiz) {
				__filbuf = malloc((size_t)bsiz + 1);
				usemalloc++;
			}
			else 
				__filbuf = sbrkreg;
		}
		else {
			/*
			 * Allocate twice the size of the first read in.
			 * An heuristic method to avoid ever calling
			 * malloc() most of the time.
			 */
			sbrksiz = 2 * bsiz;
			sbrkreg = __filbuf = sbrk( sbrksiz );
			pastfirst++;
		}

		if (read(fd, __filbuf, (unsigned)bsiz) != bsiz) {
			close(fd);
			if (usemalloc) {
				free (__filbuf);
				usemalloc = 0;
			}
			else
				sbrk (-sbrksiz);
			__filbuf = NULL;
			return (FAIL);
		}
		__filbuf[bsiz] = '\0';
		close (fd);
		return (0);
	}
}
