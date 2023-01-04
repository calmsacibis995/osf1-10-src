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
static char *rcsid = "@(#)$RCSfile: nlgetamsg.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/09/23 18:30:32 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	nlgetamsg.c,v $
 * Revision 1.1.1.1  92/03/07  00:45:00  devrcs
 * *** OSF1_1B23 version ***
 * 
 * Revision 1.2  1991/08/16  09:43:56  devrcs
 * 	Fixed an 1.1 i18n bug: removed the usage of NLgetamsg
 * 	[91/08/13  03:04:01  aster]
 *
 * $OSF_EndLog$
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

#include <locale.h>

char *nlgetamsg(catalog, set, n, s)
    char *catalog;
    int set;
    int n;
    char *s;
{
    char *string;
    nl_catd catd;

    setlocale(LC_ALL, "" );
    catd = catopen(catalog, NL_CAT_LOCALE);

    string =catgets(catd, set, n, s);
    catclose(catd);
    return string;
}
