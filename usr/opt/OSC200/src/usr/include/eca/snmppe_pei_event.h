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
 * @(#)$RCSfile: snmppe_pei_event.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 23:09:51 $
 */
/*
 **  Copyright (c) Digital Equipment Corporation, 1993
 **  All Rights Reserved.  Unpublished rights reserved
 **  under the copyright laws of the United States.
 **  
 **  The software contained on this media is proprietary
 **  to and embodies the confidential technology of 
 **  Digital Equipment Corporation.  Possession, use,
 **  duplication or dissemination of the software and
 **  media is authorized only pursuant to a valid written
 **  license from Digital Equipment Corporation.
 **
 **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 **  disclosure by the U.S. Government is subject to
 **  restrictions as set forth in Subparagraph (c)(1)(ii)
 **  of DFARS 252.227-7013, or in FAR 52.227-19, as
 **  applicable.
 **
 *
 * MODULE DESCRIPTION:
 *
 *    Common Agent SNMP Protocol Engine
 *
 * Module SNMPPE.H
 *      Contains API function prototypes for the SNMP Protocol
 *      Engine event handling functions used by EVD.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks Engineering
 *    D. McKenzie    March 1993
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *          This module is included into the compilations of modules that
 *          comprise the SNMP Protocol Engine for the Common Agent, as
 *          well as those EVD modules that call the 'pei_' event handling
 *          functions, namely:
 *              - pei_create_queue_handle()
 *              - pei_delete_queue_handle()
 *              - pei_report_event()
 *
 *    Purpose:
 *       This module contains the "pei_" function prototype definitions
 *       for the SNMP Protocol Engine event handling API functions.
 *
 * History
 *    D. McKenzie    Mar-93      Original Version.
 *
 */


/*
|
|   Include files required by snmppe_pei_event.h:
|
*/

#include "man.h"
#include "evd_defs.h"



/*
|
|   Define Prototypes for SNMP PE Event Handling Functions
|
*/

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
# define PROTOTYPE(args) args
#else
# define PROTOTYPE(args) ()
#endif

/* ------------------------------------------------------------------------
|   snmppe_carecv.c - For use by EVD ONLY!!!!!
*/

/* pei_create_queue_handle - Create an Event Queue Handle for EVD to use. */
man_status
pei_create_queue_handle PROTOTYPE((
 evd_queue_handle      **     /*-> EVD Queue Handle Pointer                  */
));

/* pei_delete_queue_handle - Delete an Event Queue Handle used by EVD.       */
man_status
pei_delete_queue_handle PROTOTYPE((
 evd_queue_handle      **     /*-> EVD Queue Handle Pointer                  */
));

/* pei_report_event - Post/Report an Event Queue Handle from EVD.            */
man_status
pei_report_event PROTOTYPE((
 evd_queue_handle      *,     /*-> EVD Queue Handle                          */
 object_id             *,     /*-> Object Class OID                          */
 avl                   *,     /*-> Object Instance Name AVL (not used)       */
 mo_time               *,     /*-> Event Time                                */
 object_id             *,     /*-> Event Type (Specific) of Event            */
 avl                   *      /*-> Event Parameters AVL                      */
));



