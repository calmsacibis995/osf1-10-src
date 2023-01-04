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
static  char *sccsid = "@(#)$RCSfile: tclear.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 14:17:03 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 *	tclear.c -
 *		prepare a system for subset extraction with tar.
 *
 *	tclear < x.inv
 *		clears system of files which have the same names as files
 *	about to be extracted from distribution but have undesirable
 *	file types. These types are s, l, c, and b and are undesirable
 *	because tar cannot overwrite them to produce files which represent
 *	the archive on the distribution.
 *
 *	The clearing is done by reading the pathnames from the inventory
 *	records, lstat(2)ing the path and checking the file type ON DISK.
 *	S_IFLNK, S_IFCHR, and S_IFBLK are removed immediately. Any S_IFREG
 *	with a link count in excess of 1 is also removed.
 *
 *	History:
 *
 *	000	31-aug-1989	ccb
 *		New
 *
 */

#include	<sys/param.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<sys/mode.h>
#include	<errno.h>
#include	<stdio.h>
#include	<setld/setld.h>

extern int	errno;		/* errno(2) */
extern char	*sys_errlist[];	/* errno(2) */

/*! these definitions should be replaced when the appropriate routines
 *  become available in the library
*/
#define	InvRecGetPath(p)	((p)->i_path)
#define	InvRecGetType(p)	((p)->i_type)

#define M		stb.st_mode
#define	NLINK		stb.st_nlink

char *prog;

main( argc, argv )
int argc;
char **argv;
{
	InvT		*isp;	/* inventory stream pointer */
	InvRecT		*irp;	/* inventory record pointer */
	struct stat	stb;	/* stat buffer */
	int		errs;	/* error counter */

	prog = *argv;
	isp = InvInit( stdin );
	errs = 0;
	while( (irp = InvRead( isp )) != NULL )
	{
		/*! should use mediated references !*/
		if( lstat( InvRecGetPath( irp ), &stb ) != 0 )
		{
			/* file does not exist, no conflict is possible
			*/
			continue;
		}

		if( S_ISBLK(M) || S_ISCHR(M) || S_ISFIFO(M) || S_ISLNK(M) || 
			(!S_ISDIR(M) && NLINK > 1 ) )
		{
			/* file is one of b, c, p, s OR is linked to
			 *  something. Remove the file.
			*/
			if( unlink( InvRecGetPath( irp ) ) != 0 )
			{
				fprintf( stderr, "%s: cannot unlink %s (%s)\n",
					prog, irp->i_path, sys_errlist[errno] );
				++errs;
			}
		}
		else if( S_ISDIR(M) && InvRecGetType( irp ) != 'd' )
		{
			/* there is a directory on the disk in a place where
			 *  we wish to install something else
			*/
			fprintf( stderr,
				"%s: cannot install %s (%c) on a directory\n",
				prog, InvRecGetPath( irp ),
				InvRecGetType( irp ) );
			exit(1);
		}
	}
	exit( errs );
}
