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
 * @(#)$RCSfile: agent_priv.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:14:11 $
 */

/*
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    This is the Agent's private function declaration header file.
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    October 31,1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14, 1990.
 *
 *    Rename file to be with 14 character restriction.
 *
 *    Miriam Amos Nihart, October 16, 1991.
 *
 *    Put in prototyping.
 *
 */

#ifndef AGENT_PRIVATE
#define AGENT_PRIVATE

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif

#include "man.h"

man_status
man_mgmt_request PROTOTYPE((
int ,
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
avl * ,
int ,
management_handle * ,
object_id * ,
avl * ,
avl * ,
avl *
)) ;

man_status
man_dispatch PROTOTYPE((
octet_string * ,
int ,
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
avl * ,
int ,
management_handle * ,
object_id * ,
avl * ,
avl * ,
avl * 
)) ;

#endif /* agent_private.h */
