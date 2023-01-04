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
static  char *sccsid = "@(#)$RCSfile: getsum.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 13:45:00 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	getsum.c
 *		checksum routines
 *
 *	mods:
 *	000	08-mar-1989	ccb
 *		This comment added. Originally derived from sum(1) in BSD 4.2
 *		sources in the summer of 1985.
 *	001	16-jun-1989	ccb
 *		lint. cleaned up return types.
*/

#include	<sys/types.h>
#include	<sys/file.h>
#include	<stdio.h>
#include	<errno.h>

#define	GSBUFSIZ	(1024*8)

/*LINTLIBRARY*/

extern int	errno;			/* errno(2) */
extern char	*sys_errlist[];		/* errno(2) */

extern char	*prog;

u_short getsum();

u_short CheckSum(path)
char *path;
{
	return(getsum(path));
}

u_short getsum(path)
char *path;
{
	char			buf[GSBUFSIZ];
	int			fd, nread;
	register unsigned	i, sum;
	register char		*t;

	if( (fd = open(path,O_RDONLY,0)) < 0 )
	{
		(void) fprintf(stderr,"%s: can't open %s (%s)\n", prog, path,
			sys_errlist[errno]);
		errno = -1;
		return(0);
	}

	/* checksum done here */
	for( sum = 0; (nread = read(fd,buf,GSBUFSIZ)) > 0; )
	{
		for( i=0, t = buf; i<nread; ++t, ++i )
		{
			if( sum & 01 )
				sum = (sum>>1) + 0x8000;
			else
				sum >>= 1;
			sum += (unsigned char) (*t);
			sum &= 0xFFFF;
		}
	}
	(void) close(fd);
	return( (u_short) sum);
}

