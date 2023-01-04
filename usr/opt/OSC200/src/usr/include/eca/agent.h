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
 * @(#)$RCSfile: agent.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:00 $
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
 *    This is the Agent's definition header file.
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    October 31, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, October 16th, 1991.
 *
 *    Put in prototyping.
 *
 */

#ifndef AGENT
#define AGENT

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif

#ifndef NOIPC
#include "mo.h"
#else
#include "man_data.h"
extern int moi_get_attributes();
extern int moi_set_attributes();
extern int moi_create_instance();
extern int moi_delete_instance();
extern int moi_invoke_action();
#endif

#include "man.h"

/*
 *  Internal data structures
 */


/*
 *  Definitions
 */

#define MAN_C_ISO_ATOMIC 1
#define MAN_C_ISO_BEST   0

#define MAN_C_ZERO_LEVEL_SCOPE    0

/*
 *  Management declarations
 */

man_status
msi_get_attributes PROTOTYPE((
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
avl * ,
int ,
management_handle *
)) ;

man_status
msi_set_attributes PROTOTYPE((
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
avl * ,
int ,
management_handle *
)) ;

man_status
msi_create_instance PROTOTYPE((
object_id * ,
avl * ,
avl * ,
avl * ,
avl * ,
avl * ,
int ,
management_handle *
)) ;

man_status
msi_delete_instance PROTOTYPE((
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
int ,
management_handle *
)) ;

man_status
msi_invoke_action PROTOTYPE((
object_id * ,
avl * ,
scope ,
avl * ,
avl * ,
int ,
object_id * ,
avl * ,
int ,
management_handle *
)) ;

#endif /* agent.h */
