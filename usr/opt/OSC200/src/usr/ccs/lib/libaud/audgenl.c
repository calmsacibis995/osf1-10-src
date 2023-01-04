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
static char *rcsid = "@(#)$RCSfile: audgenl.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/04/01 20:19:39 $";
#endif

#include <sys/audit.h>
#include <sys/types.h>
#include <varargs.h>

/* variable format front-end for audgen(2) */
audgenl ( va_alist )
va_dcl
{
    va_list ap;
    u_int event;
    char tokenp[N_AUDTUPLES];
    char *audargv[N_AUDTUPLES];
    int i;

    va_start ( ap );
    event = va_arg ( ap, int );
    for ( i = 0; i < N_AUDTUPLES; i++ ) {
        if ( (tokenp[i] = va_arg ( ap, char )) == 0 ) break;
        if ( A_TOKEN_PTR(tokenp[i]) ) audargv[i] = va_arg ( ap, char * );
        else audargv[i] = (char *)va_arg ( ap, int );
    }
    va_end ( ap );

    i = audgen ( event, tokenp, audargv, NULL, NULL );
    return(i); 
}
