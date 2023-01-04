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
 * @(#)$RCSfile: nil.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:34:13 $
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
 *    This header file contains nil data structures used in the RPC interface.
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
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, November 11th, 1990.
 *
 *    Change the nil_access_control to an avl.
 *
 *    Kelly C. Green, February 8, 1991
 *
 *    change definition of nil_time to match new mo_time
 *
 *    Miriam Amos Nihart, August 12th, 1991
 *
 *    Initialize AVLs at compile time.  The nil_avl and nil_filter are now
 *    nil pointers to avl and filter.
 *
 *    Miriam Amos Nihart, November 8th, 1991.
 *
 *    Update the avl data types.  Also make the nil_access_control a nil
 *    pointer to an avl.
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Add the global variable moss_init_global.  This is used in the DCE
 *    kit to initialize threads.
 *
 */

#ifndef MAN_NIL
#define MAN_NIL

#include "man_data.h"
#include "moss.h"
#include "moss_private.h"
#ifdef RPCV2
#include "pthread.h"
#endif /* RPCV2 */

/*
 *  Nil structures for RPC interface
 */

scope nil_scope =  0 ;
int nil_synchronization = 0 ;
uid nil_uid = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;
mo_time nil_time = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;
reply_type nil_reply_type = 0 ;
object_id nil_object_id = { 0, ( unsigned int * )0 } ;
octet_string nil_octet_string = { 0, 0, ( char * )0 } ;
struct _avl_element nil_avl_element = { ( struct _avl_element * )0 ,
                                        ( struct _avl_element * )0 ,
                                        0, ( unsigned int * )0 ,
                                        0, 0, ( char * )0 ,
                                        0 ,
                                        0 ,
                                        0 ,
                                        TRUE } ;
#ifdef RPCV2
struct _avl _nil_avl = { &nil_avl_element, &nil_avl_element, 0, TRUE, 0, ( pthread_mutex_t * )0 } ;
#else
struct _avl _nil_avl = { &nil_avl_element, &nil_avl_element, 0, TRUE, 0 } ;
#endif /* RPCV2 */
avl *nil_avl = ( avl * )&_nil_avl ;
avl *nil_filter = ( avl * )&_nil_avl ;
avl *nil_access_control = ( avl * )&_nil_avl ;
management_handle nil_management_handle = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

int nil_operation_id = 0 ;
int nil_mode = 0 ;

/*
 *  Global Data
 */

#ifdef RPCV2
int moss_init_global = FALSE ;
#endif /* RPCV2 */

#endif /* nil.h */
