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
static  char *sccsid = "@(#)$RCSfile: udelta.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/10/13 14:18:19 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * 	udelta.c
 *		comapare LEFT and RIGHT argument inventory files. Copy
 *	any LEFT record which has no corresponding RIGHT record to stdout.
 *
 *  Arguments-
 *	Two inventory files included on command line.
 *	udelta <file1> <file2>
 *
 *	MODIFICATIONS:
 *
 *	000 - 14-Feb-1989	Jon Wallace
 *	Created by Jon Wallace
 *
 *	001	22-mar-1989	ccb
 *	Minor commenting and readability enhancements
 *	Qualify for use with setld -u.
 *	Fix finish() to prevent from writing tail of RIGHT file
 *
 *	002	29-apr-1989	ccb
 *		Lint, fix a few external function definitions.
 *
 *	003	24-jul-1989	ccb
 *		lint
 *	004	15-may-1991	ech
 *		ported to OSF/1 
*/

#include	<sys/param.h>
#include        <sys/file.h>
#include        <string.h>
#include        <errno.h>
#include        <stdio.h>

/* extern ftypes */
extern void	exit();		/* exit(3) */
extern void	perror();	/* perror(3) */

/* extern vtypes */
extern	int	errno;

#define	OFF		0
#define	SET		1
#define	BYTESIZE	4096
#define	SCANFMT		"%*x%*d%*u%*hu%*hu%*ho%*s%*s%*1s%s%*s%*s"

#define	LEFT		0	/* readable index */
#define	RIGHT		1	/* readable index */


FILE    *fp[2];				/* input file pointers */
int	toggle[] = {1,0};		/* reverse switch */
int	wrtflg[] = {1,1};		/* was buffer[] written out */
int	i, cmp_result;			/* i=index, cmp_result=strcmp result */
char	*prog, *infile[2];		/* command line arguments */ 
char    buffer[2][BYTESIZE+1];		/* buffer for read and writes */
char    fmtstr[2][MAXPATHLEN+1];	/* format string buffer */
char	prestr[2][MAXPATHLEN+1];	/* previous format string buffer */
char	usage[30] = "Usage - udelta file1 file2";	/* Usage message */

main(argc,argv)
int     argc;
char    *argv[];
{
	/* Make sure there are two files included in the command
	 * line, then try to open both of them.  If successful open, go
	 * get the first record of each of the files.
	*/
	if (argc != 3)
	{
		(void) fprintf(stderr, "%s: %s \n", *argv, usage);
		exit(1);
	}
	prog = *argv++;

	prestr[LEFT][0] = prestr[RIGHT][0] = '\0';

	for (i=0; i < 2; ++i)
	{
		if ((fp[i] = fopen(argv[i], "r")) == NULL)
		{
			perror(prog);
			exit (1);
		}
		infile[i] = argv[i];
		getfile(i);
	}

	/* Compare the filename field of each record, of each file, against
	 *  each other using strcmp().  Follow the actions within the
	 *  decision making loop to determine whether or not to write the
	 *  record out to stdout.
	*/

	for(;;)	/* loop exits from getfile() */
	{
		if( (cmp_result = strcmp(fmtstr[LEFT], fmtstr[RIGHT])) == 0 )
		{
			/* path LEFT and path RIGHT match
			 *  no record is output
			 *  read new LEFT record
			 *  read new RIGHT record
			*/
			getfile(LEFT);
			getfile(RIGHT);
		}
		else if (cmp_result < 0)
		{
			/* LEFT record not in RIGHT file
			 *  write LEFT record
			 *  read LEFT record
			*/
			wrtfile(LEFT);
                	getfile(LEFT);
		}
		else
		{
			/* RIGHT record not in LEFT file
			 *  read RIGHT record (discard old record)
			*/
                	getfile(RIGHT);
		}
	}
}


/*
*
*	getfile() -
*		Pulls a record from a file until EOF, then uses
*	sscanf to select the filename field from each record.
*
*	Passed : Integer (j), which represents the index of variable
*		 arrays.
*
*	Return : nothing.
*
*/
getfile(j)
	int	j;
{
	--wrtflg[j];
        if (fgets(buffer[j], sizeof(buffer[j]), fp[j]) == NULL)
	{
		if ((wrtflg[toggle[j]] == SET) && (cmp_result != 0))
			wrtfile(toggle[j]);

		finish(toggle[j]);
	}
	else
	{
		++wrtflg[j];
		/*! sscanf() error checking needed */
        	(void) sscanf(buffer[j], SCANFMT, fmtstr[j]);
		if ((cmp_result = strcmp(prestr[j], fmtstr[j])) > 0)
		{
			(void) fprintf(stderr, "%s: %s is corrupt\n", prog,
				infile[j]);
			exit (1);
		}
	}
	(void) strcpy(prestr[j], fmtstr[j]);
}


/*
*	wrtfile() -
*		Writes a record to stdout.
*
*	Passed : Integer (j), which represents the index for variable
*		 arrays.
*
*	Return : nothing.
*
*/
wrtfile(j)
int	j;
{
	(void) fputs(buffer[j], stdout);
}


/*	finish() -
 *		Finishes writing records from one file to stdout, after 
 *	the other file has reached EOF, and comparing the strings is no
 *	longer necessary.
 *
 *	Passed : Integer (j), which represents the index for variable
 *		 arrays.
 *
 *	Return : Nothing.
 * 
*/

finish(j)
int j;
{
	if( j == RIGHT )
	{
		/* this indicates that the LEFT file has been completely
		 *  processed. DO NOT copy RIGHT records to stdout (001)
		*/
		exit(0);
	}

	while( fgets(buffer[j], sizeof(buffer[j]), fp[j]) != NULL )
		(void) fputs(buffer[j], stdout);

	exit(0);
}


