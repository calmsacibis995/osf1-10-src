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
static char rcsid[] = "@(#)$RCSfile: stak.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/11/10 23:54:28 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 *	1.11  com/cmd/sh/sh/stak.c, 9121320k, bos320 12/6/90 09:35:02
 */

#include	"defs.h"
#include	<sys/access.h>
#include	<sys/param.h>
#include <sys/types.h>
#include <sys/mman.h>
/* #define BRKMAX 2*8192 */

/* ========	storage allocation	======== */

uchar_t *
getstak(asize)			/* allocate requested stack */
int	asize;
{
	register uchar_t	*oldstak;
	register ptrdiff_t	size;

	locstak();	/* make sure stack is big enough */
	size = round(asize, BYTESPERWORD);
	oldstak = stakbot;
	staktop = stakbot += size;
	needmem (stakbot + BRKINCR);
	return(oldstak);
}

/*
 * set up stack for local use
 * should be followed by `endstak'
 */
uchar_t *
locstak()
{
	/*
	 * In shared case we must have enough for any possible 
	 * glob since malloc is the standard library version 
	 * - so get BRKMAX always. N.B. this must be larger 
	 * than the (NLS) max path name and the CPYSTR in io.c
	 */
	if (brkend - stakbot < BRKMAX)
		if (growstak())
			error(MSGSTR(M_NOSPACE,(char *) nospace));
	needmem (stakbot + BRKINCR);
	return(stakbot);
}

uchar_t *
savstak()
{
	assert(staktop == stakbot);
	return(stakbot);
}

uchar_t *
endstak(argp)		/* tidy up after `locstak' */
register uchar_t	*argp;
{
	register uchar_t	*oldstak;

	*argp++ = 0;
	oldstak = stakbot;
	stakbot = staktop = (uchar_t *)round(argp, BYTESPERWORD);
	return(oldstak);
}

tdystak(x)		/* try to bring stack back to x */
register uchar_t	*x;
{
	staktop = stakbot = MAX(x, stakbas);
	rmtemp(x);
}

stakchk()
{
/* 
 * If we have allocated additional space then unmap it here.
 */
	if (brkend - stakbas > BRKMAX+BRKMAX) {
	   munmap((caddr_t) (stakbas +  BRKMAX+BRKMAX) , (brkend - stakbas ) - BRKMAX+BRKMAX);
	   brkend = stakbas +  BRKMAX+BRKMAX;
	 }

}

uchar_t *
cpystak(x)
uchar_t	*x;
{
	register uchar_t    *argp = locstak ();
	assert(argp+strlen(x)+2 < brkend);
	locstak();	/* make sure its big enough */
	return (endstak(movstr(x,argp)));
}

int
growstak()
{
	uchar_t	*oldbase;
	unsigned int	size;
	if (stakbot == 0)
	{
		size = BRKMAX+BRKMAX;
		/*stakbot = (uchar_t *)malloc(2*size);*/
		stakbot = (uchar_t *) mmap ( (caddr_t) 0,(size_t) size  ,  PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_VARIABLE, -1, 0);
		brkend = stakbot + size;
		stakbas = staktop = stakbot;
		return stakbas?0:-1;
	}
        /* 
	 * If we need more space then mmap it after the space we
	 * got before. This restores the old sbrk behavior.
	 */
	size = (Rcheat(brkend) - Rcheat(stakbas));
	size += BRKMAX;
	oldbase = stakbas;

	stakbas = (uchar_t *) mmap ( (caddr_t) brkend,(size_t) BRKMAX,  PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS|MAP_VARIABLE, -1, 0);
	if (stakbas = brkend){
		stakbas=oldbase;
		brkend = brkend +BRKMAX;
		return 0;
	}
	else return -1;
}
