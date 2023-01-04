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
/*
 * @(#)$RCSfile: moss_utc.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:34:04 $
 */
/*
 *  static char *sccsid = "%W%	DECwest	%G%"
 */
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
 *    This is the header file containing private definitions which provides
 *    interim support for manipulating UTC formatted time stamps without
 *    the assistance of DTSS.
 *
 *    Eventually DTSS will take over the world and this file will go away.
 *    In the meantime, it should help the rest of us get by.
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

#ifndef MOSS_UTC
#define MOSS_UTC

/*
 * UTC internal representation
 */

/*
 * WARNING - the following structures assume LITTLE-ENDIAN BYTE ORDER !!!
 */

typedef struct _utc
{
    unsigned int time_lo ;
    unsigned int time_hi ;
    unsigned int inacc_lo ;
    unsigned short inacc_hi ;
    unsigned tdf : 12 ;
    unsigned vers : 4 ;
} _utc;

#define UTC_K_VERSION 1

/*
 * Structure supporting 64 bit integer quantities.
 */

typedef struct _bits64
{
    unsigned int lo ;
    unsigned int hi ;
} _bits64 ;

extern
int 
utc_gettime() ;

#endif /* end of moss_utc.h */
