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
 * @(#)$RCSfile: extern_nil.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:00:43 $
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
 *    This header file contains the extern nil declarations used by either
 *    a protocol engine or a managed object.
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    December 6, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the include file name to reflect 14 character restriction.
 *
 *    Miriam Amos Nihart, November 11th, 1990.
 *
 *    Change the nil_access_control data type to avl *.
 *
 *    Miriam Amos Nihart, August 13th, 1991.
 *
 *    Change for the nil_avl and nil_filter to pointers.
 *
 *    Miriam Amos Nihart, November 8th, 1991.
 *
 *    Change for nil_access_control to pointer.
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Add extern declaration for the global flag moss_init_global.
 */

#ifndef MAN_EXTERN_NIL
#define MAN_EXTERN_NIL

#include "man_data.h"

/*
 *  Nil structures for RPC interface
 */

extern scope nil_scope ;
extern int nil_synchronization ;
extern uid nil_uid ;
extern mo_time nil_time ;
extern reply_type nil_reply_type ;
extern avl *nil_access_control ;
extern object_id nil_object_id ;
extern octet_string nil_octet_string ;
extern avl *nil_avl ;
extern avl *nil_filter ;
extern management_handle nil_management_handle ;
extern int nil_operation_id ;
extern int nil_mode ;

#ifdef RPCV2
extern int moss_init_global ;
#endif /* RPCV2 */

#endif /* extern_nil.h */
