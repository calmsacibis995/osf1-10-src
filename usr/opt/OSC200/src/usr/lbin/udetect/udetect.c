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
static  char *sccsid = "@(#)$RCSfile: udetect.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 21:43:07 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*	
 *	udetect.c -
 *		write records in inventory differing from system state.
 *
 *	udetect < invfile		; report sum, size, type, date diffs
 *	udetect -s < invfile	; (strict) report all diffs
 *	udetect -m < invfile	; (missing) report missing files
 *	udetect -t < invfile	; (file type) report type diffs
 *
 *	HISTORY:
 *	000	2-feb-1989	Chas. Bennett
 *		New.
 *
 *	001	29-apr-1989	ccb
 *		lint.
 *
 *	002	24-jul-1989	ccb
 *		More Lint.
 *		Include <sys/dir.h> for setld.h
 *
 *	003	15-may-1991	ccb
 *		ported to OSF/1.  
*/
#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<sys/mode.h>
#include	<stdio.h>
#include	<setld/setld.h>

extern void	exit();		/* exit(3) */

/*	program option switch flag values
*/
#define	STRICT	0x0001		/* print record on any discrepancy */
#define	MISSING	0x0002		/* print record for missing files */
#define	DEBUG	0x0004		/* enable debugging output */
#define TYPE	0x0008		/* print record for file type discrepency */
#define TYPEMSGLEN	31	/* string length of output record */

char		*prog;		/* program name pointer */
unsigned	flags;		/* command flags */

main(argc,argv)
int argc;
char *argv[];
{
	char		ftype[TYPEMSGLEN+1];
	int		opt;	/* option pointer for use with getopt */
	unsigned	mask;	/* mask of file differnces */
	InvRecT		*r,	/* real file data */
			*i;	/* working inventory record */
	InvT		*inp,	/* input inventory pointer */
			*outp;	/* output inventory pointer */

	prog = *argv;

	while( (opt = getopt( argc, argv, "dmst" )) != EOF )
	{
		switch( opt )
		{
		case 'd':
			flags |= DEBUG;
			break;
		case 'm':
			flags |= MISSING;
			break;
		case 's':
			flags |= STRICT;
			break;
		case 't':
			flags |= TYPE;
			break;
		case '?':
			(void) fprintf(stderr, "usage: %s -[dmst]\n", prog);
			exit(1);
		}
	}

	inp = InvInit(stdin);
	outp = InvInit(stdout);

	/* read lines and check file attributes */
	while( (i = InvRead( inp )) )
	{
		/* get the mask of file differences */
		r = FVerify(i);
		mask = i->i_vflags;

		if( flags & DEBUG )
			(void) fprintf( stderr, "mask, file: %04x, %s\n", mask,
				i->i_path );

		/* are file types different? */
		if( flags & TYPE )
		{
		    if( (mask & I_TYPE) && (r != NULL) )
		    {
			bzero(ftype,TYPEMSGLEN);
			if( (r->i_type == 'd') )
			{
			    switch( i->i_type )
			    {
		    		case 'd':
			     		strncpy(ftype,"Directory",TYPEMSGLEN);
			     		break;
		    		case 'f':
			     		strncpy(ftype,"File",TYPEMSGLEN);
			     		break;
		    		case 's':
			     		strncpy(ftype,"Symbolic Link to",TYPEMSGLEN);
			     		break;
		    		case 'l':
			     		strncpy(ftype,"Hard Link to",TYPEMSGLEN);
			     		break;
			    }
			}
			else if( (r->i_type == 's') && (i->i_type == 'd') )
				strncpy(ftype,"Directory",TYPEMSGLEN);

			if( *ftype != NULL )
			{
				printf("%s  ",i->i_path);
				printf("should be %s  ",ftype);

				if( strcmp(i->i_ref,"none") != 0 )
					printf("%s\n",i->i_ref);
				else
					printf("\n");
			}
		    }
	 	    continue;
		}

		/* is the file there? */
		if( mask & I_PATH )
		{
			if( flags & MISSING )
				(void) InvWrite( outp, i );
			continue;
		}

		if( flags & STRICT && mask & (I_VALL) )
		{
			(void) InvWrite( outp, i );
			continue;
		}

		if( mask & I_VDATA )
			(void) InvWrite( outp, i );
	}
	exit(0);
}


