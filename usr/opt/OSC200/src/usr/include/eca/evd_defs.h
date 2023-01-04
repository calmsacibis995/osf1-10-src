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
 * @(#)$RCSfile: evd_defs.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:17 $
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
 * Common Agent Event Dispatcher
 *
 * Module EVD_DEFS.H
 *      Contains data structure definitions for using/interfacing to the 
 *      Common Agent Event Logger/Dispatcher process.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture Engineering
 *    D. McKenzie   February 1993
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *          This module is included into the compilations of modules that
 *          interface with the Common Agent Event Logger/Dispatcher process.
 *
 *    Purpose:
 *       This module contains the common data structure definitions used
 *       by the Common Agent Event Logger/Dispatcher process and the processes
 *       that interact with the Common Agent Event Logger/Dispatcher process.
 *
 * History
 *      V1.1    Feb 1993     D. McKenzie - Original version.
 */

#ifndef EVD_DEFS_H_ALREADY_INCLUDED
#define EVD_DEFS_H_ALREADY_INCLUDED


/* Module Overview: 
|
|   This file contains definitions for the C-structures and datatypes used
|   in the API's of the Common Agent Event Logger/Dispatcher process.
|
*/

/*
| Define a symbol to distinguish the compilers that allow argument lists in
| prototypes from those that don't.  Then define a macro to conditionalize
| prototype argument lists.  Note that two sets of parentheses are required.
| Example: char *f_foobar PROTOTYPE ((int *arg1, char arg2));
*/

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
# define PROTOTYPE(args) args
#else
# define PROTOTYPE(args) ()
#endif


/*
|==============================================================================
|  EVD Queue Handle
|
|   This data structure is viewed as an "opaque" data structure by the MOMs
|   that request a queue handle.  It is used by the MOM entities to express
|   a long term interest in a particular event queue inside of EVD.  The
|   queue handle must be provided to EVD in order to post events to an
|   event queue.
|
|   EVD assigns a queue handle to point to an EVD Event Queue Handle Block, 
|   but this is of no use to the MOM developer, as it is still viewed as 
|   "opaque".
|
|==============================================================================
*/

typedef long int evd_queue_handle;    /* MUST be the size of a pointer! */



/*
|==============================================================================
|  EVD Queue Access Mode
|
|   The queue access mode is an enumerated type defining EVD_GET and EVD_POST
|   as follows:
|
|==============================================================================
*/

typedef enum {
    EVD_GET,
    EVD_POST
} evd_queue_access_mode;



/*
|==============================================================================
|  EVD Boolean Definition
|
|   The boolean definition is an enumerated type defining TRUE and FALSE
|   as follows:
|
|==============================================================================
*/
/*
typedef enum {
    FALSE,
    TRUE
} boolean;
*/

/*
|==============================================================================
|  EVD Protocol Definition
|
|   The protocol definition is an enumerated type defining the various 
|   Common Agent protocols as follows:
|
|==============================================================================
*/

typedef enum {
    EVD_PROTOCOL_ANY,
    EVD_PROTOCOL_CMIP,
    EVD_PROTOCOL_SNMP,
    EVD_PROTOCOL_RPC
} evd_protocol;



/*
|==============================================================================
|  EVD Style Definition
|
|   The stype definition is an enumerated type defining the event posting style
|   as follows:
|
|==============================================================================
*/

typedef enum {
    EVD_STYLE_NONBLOCKING,
    EVD_STYLE_BLOCKING
} evd_style;


/*
|==============================================================================
| EVD API Function Prototypes:
|
|==============================================================================
*/

#ifndef PROTOTYPE
#  define PROTOTYPE( arg ) arg
#endif

man_status 
evd_create_queue_handle PROTOTYPE ((
 evd_queue_handle      **,
 avl                   *, 
 evd_queue_access_mode
));

man_status 
evd_delete_queue_handle PROTOTYPE ((
 evd_queue_handle **
));

man_status 
evd_post_event PROTOTYPE ((
 evd_queue_handle *,
 object_id        *,
 avl              *,
 mo_time          *,
 object_id        *,
 avl              *,
 uid              *,
 uid              *
));

#endif /* EVD_DEFS_H_ALREADY_INCLUDED */

