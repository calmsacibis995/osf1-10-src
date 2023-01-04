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
static char *rcsid = "@(#)$RCSfile: utc_dummy.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:35:53 $";
#endif
#ifndef lint
static char *sccsid = "%W%	ULTRIX	%G%" ;
#endif

/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    The following contains a dummy version of utc_gettime for those
 *    sites lacking DTSS.  The routine simply returns a failure,
 *    causing moss_get_time to construct the time value itself.
 *
 * Routines:
 *
 *    utc_gettime()
 *
 * Author:
 *
 *    Kelly C. Green
 *
 * Date:
 *
 *    January 14, 1991
 *
 * Revision History :
 *
 */

/*
 *  Support header files
 */  

/*
 *  External
 */


int
utc_gettime()

/*
 *
 * Function Description:
 *
 *    Always returns -1.
 *
 * Parameters:
 *
 *    none
 *
 * Return value:
 *
 *    always -1
 *
 * Side effects:
 *
 *    moss_get_time invokes an alternate strategy for obtaining the time.
 *
 */

{
    return ( -1 ) ;
}

/* end of utc_dummy.c */
