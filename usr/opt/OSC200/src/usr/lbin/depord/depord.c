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
static	char *sccsid = "@(#)$RCSfile: depord.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/12/21 20:36:23 $";
#endif
/*
 */
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*	depord.c -
 *		order subsets wrt dependencies
 *
 *	depord s1 s2 ... sn
 *		print a list of subset codes sorted in dependency order
 *
 *	MODS:
 *	000	15-may-1989	ccb
 *	001	24-jul-1989	ccb
 *		add -d switch and clean up during ongoing qualification
 *	002	14-aug-1989	ccb
 *		finish qualification
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/dir.h>
#include	<stdio.h>
#include	<errno.h>
#include	<setld/list.h>
#include	<setld/setld.h>

extern int	errno;		/* errno(2) */
extern char	*sys_errlist[];	/* errno(2) */
extern int	optind;		/* getopt(3) */

extern int	ctrlerr;	/* libsetld(Ctrl.o) */

#define		F_DEBUG	0x0001			/* debug flag */
#define		DEBUG	(flags&F_DEBUG)		/* debug macro */

typedef struct QT
{
	struct QT	*q_next;
	NameT		q_subset;
} QT;
QT	*QAppend();
QT	*QCreate();
QT	*QMember();
void	QPrintList();

char		*prog;
unsigned	flags = 0;

main( argc, argv )
int argc;
char **argv;
{
	int		i;		/* getopt(3), misc */
	CtrlT		*c;
	CtrlRecT	*n,		/* new data pointer */
			*t;		/* temp pointer */

	ProdRecT	*prodp, *prodl;	/* bucket pointers */
	QT		*q = NULL;

	prog = *argv;

	/* check for debugging flag */
	while( (i = getopt( argc, argv, "d" )) != EOF )
	{
		switch( i )
		{
		case 'd':
			flags |= F_DEBUG;
			(void) fprintf( stderr, "%s: debug enabled\n", prog );
			break;
		default:
			exit(1);
		}
	}
	if( (argc -= optind) == 0 )
	{
		/* called with no args, exit now to avoid
		 *  the overhead incurred in processing the
		 *  database.
		*/
		exit( 0 );
	}
	argv += optind;
		
	/* convert the arglist *argv[] to a linked list
	 *  of QT structures
	*/
	while( argc-- )
		q = QAppend( q, QCreate( *argv++ ) );

	if( DEBUG )
	{
		(void) fprintf( stderr, "Before ordering:\n" );
		QPrintList( q );
	}

	/* read in all of the control files
	*/
	if( (c = CtrlOpen( ".", "r" )) == NULL )
	{
		(void) fprintf( stderr, "%s: cannot open '.' (%s)\n", prog,
			sys_errlist[errno] );
		exit(1);
	}

	for( i = 0, prodl = NULL; (t = CtrlRead( c )) != NULL; ++i )
	{
		/* do not store/process the data further if subset is not 
		   specified on command line */
		if( !QMember( q, t->ct_subset ) )
			continue;

		if( (n = CtrlRecNew()) == NULL )
		{
			(void) fprintf( stderr,
				"%s: cannot store control info (%s)\n", prog,
				sys_errlist[errno] );
			exit(1);
		}
		(void) CtrlRecCopy( n, t );

		if( DEBUG )
		{
			/* print a copy of the control record */
			(void) fprintf( stderr, "\n%s Ctrl Rec\n",
				CtrlGetFpath(c) );
			CtrlRecPrintList( stderr, n, 0 );
		}
		prodl = ProdRecAddCtrl( prodl, n );
	}

	if( DEBUG )
	{
		(void) fprintf( stderr, "\n%s: %d ctrl records input\n",
			prog, i );
		(void) fprintf( stderr, "\n%s: ctrlerr 0x%x (%s)\n",
			prog, ctrlerr, CtrlErrorString( ctrlerr ) );
	}

	/* Order the product info
	*/
	if( (prodl = ProdRecOrderByDeps( prodl, DEPS_OPEN )) == NULL )
	{
		(void) fprintf( stderr,
			"%s: dependency ordering error\n", prog );
	}

	if( DEBUG )
	{
		(void) fprintf( stderr, "\nAfter Ordering:\n" );
		ProdRecPrintList( stderr, prodl, 0 );
	}

	/* traverse the buckets
	*/
	for( prodp = prodl; prodp != NULL; prodp = prodp->p_next )
	{
		for( t = prodp->p_cp; t != NULL; t = t->ct_next )
		{
			(void) printf( "%s\n", t->ct_subset );
		}
	}

	exit(0);
}







/*	QT	*QAppend() -
 *		concatenate lists of element type QT
 *
 *	given:	QT *s - an element
 *		QT *t - another element
 *	does:	call ListAppend()
 *	return:	a pointer to the resulting list
*/

QT *QAppend( s, t )
QT *s, *t;
{
	return( (QT *) ListAppend( (ListT *) s, (ListT *) t ) );
}


/*	QT	*QCreate() -
 *		create a named QT
 *
 *	given:	char *s - a string for the subset field of the new QT
 *	does:	create a new QT, fill the subset field
 *	return:	a pointer to the new QT or NULL on error
*/

QT *QCreate( s )
char *s;
{
	QT	*q;

	if( (q = (QT *) malloc( sizeof(QT) )) == NULL )
		return( NULL );

	(void) NameSet( q->q_subset, s );
	q->q_next = NULL;
	return(q);
}



QT *QMember( q, s )
QT *q;
char *s;
{
	if( q == NULL )
		return( NULL );

	if( !strcmp( q->q_subset, s ) )
		return( q );

	return( QMember( q->q_next, s ) );
}

void QPrintList( q )
QT *q;
{
	while( q != NULL )
	{
		(void) fprintf( stderr, "%s ", q->q_subset );
		q = q->q_next;
	}
	(void) fprintf( stderr, "\n" );
}
/*END*/

