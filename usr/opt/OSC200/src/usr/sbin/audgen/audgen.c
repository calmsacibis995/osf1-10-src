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
static char *rcsid = "@(#)$RCSfile: audgen.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/05 20:13:33 $";
#endif

/* command line interface to audgen() */
#include <sys/audit.h>
main ( argc, argv )
int argc;
char *argv[];
{
    char tmask[N_AUDTUPLES];
    int i;

    if ( argc == 1 ) {
        printf ( "Usage: param[s]\n" );
        exit(1);
    }

    for ( i = 0; i < argc-1; i++ ) tmask[i] = T_CHARP;
    for ( ; i < N_AUDTUPLES; i++ ) tmask[i] = '\0';
    if ( audgen ( AUDGEN8, tmask, &argv[1], (char *)0, (long *)0 ) == -1 ) {
        perror ( "audgen" );
        exit(1);
    }
    else exit(0);
}
