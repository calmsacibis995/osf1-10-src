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
 * @(#)$RCSfile: moss_inet.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:03:02 $
 */
/*
 *  static char *sccsid = "%W%	DECwest	%G%" ;
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
 *    This the header file containing public definitions for MOSS Internet.
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    May 4th, 1990.
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990
 *
 *    Change the file name to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, July 13th, 1990
 *
 *    Change the names to be more descriptive.
 */

#ifndef	MOSS_INET_TYPES
#define MOSS_INET_TYPES

/*
 * Define the INET encoding. INET tags are defined like ASN1 types.
 */

/*
 * IP Data types - taken from RFC 1065 
 */

/* ???
        INET_C_SMI_NETWORK_ADDRESS=APPLICATION( )
*/

#define INET_C_SMI_IP_ADDRESS    APPLICATION( 0 ) 
#define INET_C_SMI_COUNTER       APPLICATION( 1 ) 
#define INET_C_SMI_GAUGE         APPLICATION( 2 ) 
#define INET_C_SMI_TIME_TICKS    APPLICATION( 3 ) 
#define INET_C_SMI_OPAQUE        APPLICATION( 4 ) 

#endif /* end of file moss_inet_types.h */

