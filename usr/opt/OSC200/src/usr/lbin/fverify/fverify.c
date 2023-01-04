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
static  char *sccsid = "@(#)$RCSfile: fverify.c,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/10/14 10:56:05 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/* fverify.c
 *
 * Name:	fverify
 * Purpose:	Verify OSF/1 installation.
 * Usage:	fverify [-yn] <inventory
 *			-y quietly repair everything
 *			-n just report inconsistencies
 *
 * Compile:	cc fverify.c getsum.c
 * Date:	1/11/84
 * Author:	Alan Delorey
 * Remarks:
 *  Verifies the existence and attributes of files.
 *  It reads input lines(using scanf) from the inventory until EOF is reached.
 *  It calls lstat to get info about each file, and compares to master list.
 *  The master attributes file should be stdin.
 *
 *	HISTORY:
 *	001	ccb 	09-03-1985
 *		Fix indirection bug preventing correct verification
 *		of directories and symbolic links.
 *		Increased performance in checksum().
 *		Overall 25% performance increase.
 *
 *	002	ccb	1986.03.15
 *		Add interactive repair, force repair, no repair.
 *		Add fast mode that ignores sizes and sums.
 *		Accelerate I/O.
 *		Stop repetitive checks for files hard linked together.
 *		Add mode checking for soft links and directories.
 *		Add time stamps for the output log.
 *
 *	003	ccb	1986.09.16
 *		drop in code for interrupt handling
 *		drop in code for correction of symbolic links
 *		clean up informational messages.
 *
 *	004	04-apr-1989	ccb
 *		general cleanup
 *		quieter logging
 *		extensions:
 *			device support in inventories
 *			referent checking
 *
 *	005	29-apr-1989	ccb
 *		Lint.
 *	006	14-jun-1989	ccb
 *		add check for malloc return value when allocating storage
 *			directory queue entries
 *
 *	007	24-jul-1989	ccb
 *		inlcude sys/dir.h for setld.h
 *		more lint
 *
 *	008	16-aug-1989	ccb
 *		change method for inserting non-existent directories
 *		into the directory queue to use an alphabetizing insert
 *		routine from the Inv package. This fixes problems caused
 *		by unsorted inventories.
 *
 *	009 	13-Nov-1990	jps
 *		fverify -p addition
 *
 *	010	08-feb-1991	ech
 *		1) change path of fverifylog from ./usr/adm to ./var/adm.
 *		2) ignore verification errors on permissions of symbolic links.
 *
 *	011	15-mar-1991	ech
 *		rename log file to fverify.log and move it to ./var/adm/syslog
 *
 *	012	21-jun-1991	ccb
 *		after checking sums and sizes, check if the file is
 *
 *		a symbolic link. If so, skip all other checks.
 *	013	12-sep-1991	ech
 *		move fverify.log to ./var/adm/smlogs
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/file.h>
#include	<sys/errno.h>
#include	<sys/time.h>
#include	<sys/dir.h>
#include	<signal.h>
#include	<string.h>
#include	<stdio.h>

#include	<setld/setld.h>

#define	VFYLOG	"./var/adm/smlogs/fverify.log"		/* logfile */

#define	PART(F)		( (F) & PARTIAL )
#define	ISVOL(F)	( (F) & INVVOLATILE )

/* command flag values
*/
#define	FIX	0x0001		/* fix all correctable problems */
#define	ASK	0x0002		/* ask if problem should be fixed */
#define PARTIAL	0x0004		/* partial verify, no volatile */
#define	DEFAULT	(FIX|ASK)	/* default ask first, then fix */

#define	UID	0
#define	GID	1
#define	PERMS	2

extern errno;			/* errno(2) */
extern char *sys_errlist[];	/* errno(2) */

/* routine types */
extern void	exit();		/* exit(3) */

/* global types */
char		buf[BUFSIZ];		/* generic buffer */
int		errs;			/* error counter */
int		fixed;			/* repair counter */
FILE		*logfp;			/* logfile file pointer */
int		pid;			/* process id, used for logging */
char		*prog;			/* program name pointer */
unsigned	flags = DEFAULT;	/* command flags */
FILE		*ttyin;			/* input tty file pointer */
FILE		*ttyout;		/* output tty file pointer */


main(argc,argv)
int argc;
char *argv[];
{
	int		c;	/* option char from getopt(3) */
	InvRecT		*mkq;	/* directories to be created */
	InvT		*i;	/* inventory pointer */
	InvRecT		*ip;	/* inventory record pointer */
	unsigned	mask;	/* verification bits, used as shorthand */
	InvRecT		*qp;	/* pointer for fresh ones */
	InvRecT		*rp;	/* record pointer for file as found on disk */

	(void) signal( SIGINT, SIG_IGN );	/* ignore keyboard interrupt */
	(void) signal( SIGQUIT, SIG_IGN );	/* ignore keyboard quit */

	prog = *argv;

	/* get command flags */
	while( (c = getopt( argc, argv, "npy" )) != EOF )
	{
		switch( c )
		{
		case 'n':
			/* report only, no fixes */
			flags &= ~(FIX|ASK);
			break;
		case 'p':
			/* partial verify */
			flags |= PARTIAL;
			break;
		case 'y':
			/* fix everything, don't ask */
			flags &= ~ASK;
			break;
		default:
			(void) fprintf( stderr, "%s: usage: %s [-npy] < inventory\n",
				prog, prog );
			exit(1);
		}
	}


	OpenLog();	/* start logging */
	OpenTTY();	/* set up terminal access */

	mkq = (InvRecT *) 0;

	/* read lines and check file attributes */
	i = InvInit( stdin );
	for( fixed = errs = 0; (ip = InvRead(i)) != NULL; )
	{
		rp = FVerify( ip );
		mask = ip->i_vflags;

		if( mask & I_PATH )
		{
			/* file not in system. this is recoverable in the
			 *  following cases:
			 *  type d: directory, queue for later creation
			 *  type s: symbolic link, stubbed.
			 *  type p: fifo, stubbed.
			 *  type l: hard link, stubbed.
			*/
			switch( ip->i_type )
			{
			case 'd':
				/* directory, missing directories are queued
				 *  for reconstruction later
				*/
				if( (qp = InvRecNew()) == NULL )
				{
					(void) sprintf( buf,
						"%s: out of memory\n", prog );
					Log( buf );
					WriteTTY( buf );
					exit(1);
				}
				(void) InvRecCopy( qp, ip );
				mkq = InvRecInsertAlpha( mkq, qp );
				break;
			case 'l':
			case 'p':
			case 's':
			default:
				(void) sprintf( buf, "%s: cannot stat (%s)\n",
					ip->i_path, sys_errlist[errno] );

				Log( buf );
				WriteTTY( buf );
				++errs;
			}
			continue;
		}

		if( mask & I_TYPE )
		{
			(void) sprintf( buf,
				"%s: file type '%c' should be '%c'\n",
				ip->i_path, rp->i_type, ip->i_type );

			Log( buf );
			WriteTTY( buf );
			++errs;
		}
		if( !PART(flags) || (PART(flags) && !ISVOL(ip->i_flags)) )
		{
			if( mask & I_SUM )
			{
				(void) sprintf( buf,
					"%s: checksum %05u should be %05u\n",
					ip->i_path, rp->i_sum, ip->i_sum );

				Log( buf );
				WriteTTY( buf );
				++errs;
			}
			if( mask & I_SIZE )
			{
				(void) sprintf( buf,
					"%s: size %d should be %d\n",
					ip->i_path, rp->i_size, ip->i_size );

				Log( buf );
				WriteTTY( buf );
				++errs;
			}
		}
		/* none of the other attributes are meaningful on
		 *  symbolic links, if the current file is a link, continue
		 *  with the next file.
		*/
		if( S_ISLNK( ip->i_mode ) )
			continue;

		if( mask & I_GID )
		{
			(void) sprintf( buf, "%s: gid %d should be %d\n",
				ip->i_path, rp->i_gid, ip->i_gid );

			Fix( GID, ip, rp );
		}

		if( mask & I_UID )
		{
			(void) sprintf( buf, "%s: uid %d should be %d\n",
				ip->i_path, rp->i_uid, ip->i_uid );

			Fix( UID, ip, rp );
		}

		if( mask & I_PERM )
		{
			char	ps1[11];
			
			/* establish private copy of one of the permissions
			 *  strings to avoid static overwrite
			*/
			(void) strcpy( ps1, PermString( ip->i_mode ) );
			(void) sprintf( buf,
				"%s: permissions %s should be %s\n",
				ip->i_path, PermString( rp->i_mode ), ps1 );

			Fix( PERMS, ip, rp );
		}

	}
	/* attempt to create directories queued from lstat failure */
	(void) umask(0);
	for( qp = mkq; qp != (InvRecT *) 0; qp = qp->i_next )
	{
		(void) sprintf( buf, "\tCreating directory %s\n", qp->i_path);
		Log( buf );
		WriteTTY( buf );

		/* attempt to create directory
		*/
		if( mkdir( qp->i_path, (int) qp->i_mode ) )
		{
			(void) sprintf( buf, "\tCannot create dir %s (%s)\n",
				qp->i_path, sys_errlist[errno] );

			++errs;
			Log( buf );
			WriteTTY( buf );
		}
		else
		{
			/*! should perform error checking */
			(void) chown( qp->i_path, qp->i_uid, qp->i_gid );
			(void) chmod( qp->i_path, (int) qp->i_mode);
		}
	}
	Exit();
}



/*	Exit() -
 *		log exit information and exit
*/

Exit()
{
	long	t;

	(void) sprintf(buf,"\t%d %s\n\t%d %s\n", errs,
		"verification errors encountered.", fixed,
		"corrections performed.");

	WriteTTY(buf);
	Log(buf);

	
	(void) sprintf( buf, "%s%d end logging at %s\n", prog, pid,
			asctime( localtime( ((void) time(&t),&t) ) ) );

	Log( buf );
	exit(errs-fixed);
}



/*	Fix() -
 *		fix a problem
 *
 *	given:	int code - problem code in [UID,GID,PERM]
 *		InvRecT *p - inventory record
 *		InvRecT *q - file snapshot
 *	does:	fixes the problem with the file
 *	return:	nothing
 *	effect:	will increment global int fixed if fix was made
*/

Fix( code, p, q )
int code;
InvRecT *p;
InvRecT *q;
{
	int		FixUid(), FixGid(), FixPerm();
	static int	(*ftab[])() = { FixUid, FixGid, FixPerm };

	++errs;
	Log( buf );
	WriteTTY( buf );

	if( ShouldFix() )
	{
		if( (*ftab[code]) ( p, q ) )
		{
			/* something went wrong
			*/
			(void) sprintf( buf, "\tcannot correct %s (%s)\n",
				p->i_path, sys_errlist[errno] );
		}
		else
		{
			/* OK
			*/
			(void) sprintf( buf, "\t%s corrected.\n", p->i_path );
			++fixed;
		}
		Log( buf );
		WriteTTY( buf );
	}
}



/*	FixGid() -
 *		gid is wrong, do something
 *
 *	given:	InvRecT *p - pointer to inventory record from input
 *		InvRecT *q - inventory record created from checking the state
 *			of the file on the disk
 *	does:	fix the gid if the user wants it fixed
 *	return:	nothing
*/

FixGid(p, q)
InvRecT *p, *q;
{
	if( chown( p->i_path, q->i_uid, p->i_gid ) )
		return(-1);

	q->i_gid = p->i_gid;	/* update reality */
	return(0);
}



/*	FixPerm() -
 *		permission modes are wrong, do something
 *
 *	given:	InvRecT *p - pointer to the inventory record from input
 *		InvRecT *q - unused. present to keep lint quiet because of
 *			calling sequence in Fix().
 *	does:	fix permissions
 *	return:	0 if successful, -1 otherwise
*/

/*ARGSUSED*/
FixPerm( p, q )
InvRecT *p;
InvRecT *q;
{
	mode_t	perms;	/* mode bits to be used */
	int	n;	/* index for readlink(2) */

	perms = PERM( p->i_mode );
	if( !chmod( p->i_path, (int) perms ) )
		return(0);
	return(-1);
}



/*	FixUid() -
 *		uid is wrong, do something
 *
 *	given:	InvRecT *p - pointer to the inventory record from input
 *		InvRecT *q - inventory record generated when checking the
 *			state of the file on the disk
 *	does:	fix the uid if the user wants it fixed
 *	return:	nothing
*/

FixUid( p, q )
InvRecT *p, *q;
{
	if( chown( p->i_path, p->i_uid, q->i_gid ) )
		return(-1);

	q->i_uid = p->i_uid;	/* update reality */
	return(0);
}




/*	Log() -
 *		write entry to logfile
 *
 *	given:	char *p - pointer to a buffer to be logged
 *	does:	write the buffer to the logfile
 *	return:	nothing
*/

Log(p)
char *p;
{
	(void) fputs( p, logfp );
}



/*	OpenTTY()
 *		get terminal to talk to
 *
 *	given:	nothing, uses global flags
 *	does:	if ASK, open /dev/tty for interaction with user
 *	return:	nothing, may exit if terminal interaction is not possible
*/

OpenTTY()
{
	ttyout = stderr;
	if( !(flags&ASK) )
	{	/* tty access unnecessary */
		return;
	}

	if( (ttyin = fopen("/dev/tty", "r")) == NULL )
	{
		(void) fprintf( stderr, "%s: cannot open /dev/tty (%s)\n",
			prog, sys_errlist[errno] );
		exit(1);
	}

	if( (flags&ASK) && !isatty( fileno(ttyout) ) )
	{
		/* fverify is supposed to ask but can't.
		 *  disable prompts and fixes.
		*/
		(void) sprintf( buf,
			"error output is not a tty, fixes disabled\n", prog );

		Log( buf );
		WriteTTY( buf );
		flags &= ~(FIX|ASK);
	}
}



/*	OpenLog()
 *		open the logfile
 *
 *	given:	nothing
 *	does:	opens a channel to the logfile and writes an initial
 *		entry
 *	return: nothing
*/

OpenLog()
{
	long	t;	/* for use with ctime(3) */

	if( freopen( VFYLOG, "a", stdout) == (FILE *) 0 )
	{
		(void) fprintf(stderr, "%s: cannot open log file (%s)\n",
			prog, sys_errlist[errno]);
		errno=0;
	}
	logfp = stdout;
	pid = getpid();
	(void) sprintf( buf, "%s%d (%x) begin logging at %s\n", prog, pid,
		flags, asctime( localtime( ((void) time(&t),&t) ) ) );
	Log( buf );
}



/*	WriteTTY() -
 *		write a buffer to the terminal
 *
 *	given:	char *p - a pointer to the buffer to be written
 *	does:	puts the buffer on the terminal output stream
 *	return:	nothing
*/

WriteTTY(p)
char *p;
{
	(void) fputs( p, ttyout );
}



/*	ShouldFix() -
 *		determine if a discrepancy should be fixed
 *
 *	given:	uses global flags
 *	does:	determines if fix is needed based on flags and/or user input
 *	return:	1 if fix is needed, 0 if not.
 *		may exit program on user input.
*/

ShouldFix()
{
	char	*cp;	/* user input token pointer */

	if( (flags & (FIX|ASK)) == (FIX|ASK) )
	{
		/* interactive fixing */
		for( ;; )
		{
			WriteTTY( "Fix (y/n/q) [n]? " );

			(void) fgets( buf, sizeof(buf), ttyin );

			/* elide begining spaces */
			if( (cp = strtok( buf, "\t " )) == NULL || *cp == 'n' ||
				*cp == 'N' || *cp == '\n' )
			{
				/* no or default assumed
				*/
				return(0);
			}
			else if( *cp == 'q' || *cp == 'Q' )
			{
				/* user wants out
				*/
				Exit();
			}
			else if( *cp == 'Y' || *cp == 'y' )
			{
				/* fix it */
				return( 1 );
			}
			/* invalid input, go back to top of the loop
			*/
		}
	}
	/* non-interactive, return fix flag value */
	return( flags & FIX );
}

/*END*/
