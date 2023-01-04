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
static  char *sccsid = "@(#)$RCSfile: ils.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 14:14:46 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 *	ils.c -
 *		list a named file in inventory format.
 *
 *	usage: ils file [file...]
 *
 *	000	29-apr-1989	ccb
 *		about a month old already. Running Lint
 *
 *	001	24-jul-1989	ccb
 *		Include <sys/dir.h> for setld.h
 *		Merge changes by Tungning Cherng recognizing -f file.
 *			'file' here contains a list of files to expand into
 *			inventory records.
 *		More Lint
 *		Continue transition to fully mediated structure references
 *
 *	002	15-may-1991	ech
 *		ported to OSF/1
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	<errno.h>
#include	<string.h>
#include	<stdio.h>
#include	<setld/setld.h>

extern int	errno;			/* errno(2) */
extern char	*sys_errlist[];		/* errno(2) */
extern int	optind;			/* getopt(3) */

void		exit();		/* exit(3) */

int	debug = 0;	/* debug flag */
int	fflg = 0;	/* a file contains files names */
char	*prog;		/* program name */

InvRecT		*i;	/* inventory record pointer */
InvT		*ip;	/* inventory pointer */
struct stat	stb;	/* stat(2) buffer */

main(argc, argv)
int argc;
char *argv[];
{
	int		c;		/* for use with getopt(3) */
	FILE		*fp;		/* for use with -f */
	char		line[132];	/* for use with -f */
	int		j;		/* for use with -f */

	prog = *argv;
	while( (c = getopt( argc, argv, "d:f" )) != EOF )
	{
		switch( c )
		{
		case 'd':
			++debug;
			break;
		case 'f':
			++fflg;
			break;
		default:
			(void) fprintf( stderr, "---usage: %s [-d] file...\n", prog );
			exit(1);
		}
	}

	ip = (InvT *) stdout;
	while( ++argv, --argc  )
	{
		if( fflg == 0 )
			getfile( *argv );	
		else
		{
			++argv; --argc;
			if( (fp = fopen(*argv, "r")) == NULL )
			{
				(void) fprintf( stderr,
					"%s: cannot open %s (%s)\n", prog,
					*argv, sys_errlist[errno] );
				exit(1);
			}
			while( fgets(line, sizeof(line),fp) != NULL )
			{
				for( j=0;
					line[j] != ' ' &&
					line[j] != '\t' &&
					line[j] !='\n'; j++)	
					;	
				line[j] = '\0';
				getfile(line);
			}
		}
	}
}

getfile(fn)
char *fn;
{
	if( lstat( fn, &stb ) )
	{
		(void) fprintf( stderr, "%s: cannot stat %s (%s)\n", prog,
			fn, sys_errlist[errno] );
		errno = 0;
		return;
	}
	if( debug )
	{
		(void) fprintf( stderr, "%s: debug: stb.st_size = %d\n", prog,
			stb.st_size );
	}

	/* generate an inventory record.
	 *  NOTE: this implemetation does not correctly handle hard
	 *   links. All hard links appear on the output as individual
	 *   file instances.
	*/
	i = StatToInv( &stb );
	if( debug )
	{
		(void) fprintf( stderr, "%s: debug: i->i_size = %d\n", prog,
			i->i_size );
	}
	i->i_flags = 0;
	(void) InvRecSetRev( i, "010" );
	(void) InvRecSetPath( i, fn );
	(void) InvRecSetSubset( i, "-" );
	(void) InvRecSetRef( i, "unknown" );

	/* check file type before generating checksum
	*/
	if( i->i_type == 'f' )
		i->i_sum = CheckSum( i->i_path );
	else
		i->i_sum = 0;

	(void) InvWrite( ip, i );
}

