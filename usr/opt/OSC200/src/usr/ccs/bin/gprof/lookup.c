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
static char	*sccsid = "@(#)$RCSfile: lookup.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/09 07:12:26 $";
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
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#include "gprof.h"

    /*
     *	look up an address in a sorted-by-address namelist
     *	    this deals with misses by mapping them to the next lower 
     *	    entry point.
     */
nltype *
nllookup( address )
    unsigned long	address;
{
    register unsigned long	low;
    register unsigned long	middle;
    register unsigned long	high;
#   ifdef DEBUG
	register int	probes;

	probes = 0;
#   endif DEBUG
    for ( low = 0 , high = nname - 1 ; low != high ; ) {
#	ifdef DEBUG
	    probes += 1;
#	endif DEBUG
	middle = ( high + low ) >> 1;
	if ( nl[ middle ].value <= address && nl[ middle+1 ].value > address ) {
#	    ifdef DEBUG
		if ( debug & LOOKUPDEBUG ) {
		    printf( "[nllookup] %d (%d) probes\n" , probes , nname-1 );
		}
#	    endif DEBUG
	    return &nl[ middle ];
	}
	if ( nl[ middle ].value > address ) {
	    high = middle;
	} else {
	    low = middle + 1;
	}
    }
    fprintf( stderr , "[nllookup] binary search fails???\n" );
    return (nltype *)0L;
}

arctype *
arclookup( parentp , childp )
    nltype	*parentp;
    nltype	*childp;
{
    arctype	*arcp;

    if ( parentp == 0 || childp == 0 ) {
	printf( "[arclookup] parentp == 0 || childp == 0\n" );
	return 0;
    }
#   ifdef DEBUG
	if ( debug & LOOKUPDEBUG ) {
	    printf( "[arclookup] parent %s child %s\n" ,
		    parentp -> name , childp -> name );
	}
#   endif DEBUG
    for ( arcp = parentp -> children ; arcp ; arcp = arcp -> arc_childlist ) {
#	ifdef DEBUG
	    if ( debug & LOOKUPDEBUG ) {
		printf( "[arclookup]\t arc_parent %s arc_child %s\n" ,
			arcp -> arc_parentp -> name ,
			arcp -> arc_childp -> name );
	    }
#	endif DEBUG
	if ( arcp -> arc_childp == childp ) {
	    return arcp;
	}
    }
    return 0;
}
