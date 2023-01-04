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
static char *rcsid = "@(#)$RCSfile: evd_recv.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:31:24 $";
#endif
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
 *    Common Agent Event Dispatcher for the U*IX Common Agent
 *
 * Module EVD_RECV.C
 *    Contains "Common Agent - Receive Functions" for the
 *    Event Dispatcher for the Common Agent.
 *    These are functions that "RECEIVE requests FROM" the Common Agent MOMs.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks Engineering
 *    D. McKenzie  February 1993    Initially For Common Agent V1.1.
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engine(s) accept requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.  Events
 *       that need to be brought to a management station's attention are 
 *       routed through the event dispatcher to the appropriate protocol
 *       engine (only SNMP-PE for V1.1) where they are then forwarded to
 *       the TRAP listeners/subscribers.
 *
 *    Purpose:
 *       This module contains the "receive" functions for the Event Dispatcher
 *       that are designed to handle event requests from the Common Agent
 *       Managed Object Modules (MOMs), and is responsible for shipping the
 *       event requests to the appropriate Common Agent protocol engine for
 *       eventual shipment to all interested management stations.
 *
 * History
 *      V1.1    Feb 1993    D. McKenzie (used "format" of snmppe_recv.c).
 *                          Original Version is 1.1 (not 1.0) to coincide with 
 *                          the Common Agent Release 1.1 for which this was 
 *                          new process was created.
 *

Module Overview:
---------------

This module contains the EVD function(s) that process event related requests
coming into EVD from the Common Agent MOMs.  The requests currently supported
are:
    - evd_create_queue_handle()
    - evd_delete_queue_handle()
    - evd_post_event()


Thread Overview:
---------------

All the functions in this module are executed exclusively by the listening
subordinate "receiving" thread (for V1.1 the main thread becomes the subordinate
receiving thread).  The 'queue" used for receiving events is virtually "1"
element long; each event is processed to completion before returning control 
to the calling processes.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (i.e., Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------
evd_create_queue_handle The function entered by the Common Agent MOMs (via 
                        RPC/IPC in an rpc/threaded environment) to request 
                        creation of a queue handle for later use in posting 
                        events to.  A MOM need only create a single queue 
                        handle to handle all event reporting.  If desired, a 
                        MOM can create seperate queue handles, each for a 
                        different class of events.  For Common Agent V1.1, 
                        the former is all that is recommended.

evd_delete_queue_handle The function entered by the Common Agent MOMs (via
                        RPC/IPC in an rpc/threaded environment) to request 
                        deletion of an existing queue handle that is no longer
                        needed.

evd_post_event          The function entered by the Common Agent MOMs (via
                        RPC/IPC in an rpc/threaded environment) to post an
                        event, using a queue handle previously obtained via the
                        evd_create_queue_handle().  


MODULE INTERNAL FUNCTIONS:

Function Name            Synopsis
-------------            --------
evd_find_queue_handle   Given the queue handle from a Common Agent request, 
                        this function searches EVD's internal list of
                        queue handle blocks (the "queue_handle_list" in the 
                        'Big Picture' structure) for the queue handle block 
                        that corresponds to the queue handle returned in the
                        evd_create_queue_handle() call.

evd_add_queue_handle    Attaches an already allocated event queue handle 
                        structure to the list of event queue handles strung
                        off of the queue_handle_list in the big picture.

evd_remove_queue_handle Detaches (does NOT free) an existing event queue handle 
                        structure from the list of event queue handles strung
                        off of the queue_handle_list in the big picture.
       
evd_snmp_octet_to_oid   Converts an octet string to an oid according to
                        RFC1212.
*/

/* Module-wide Stuff */

/*  KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
 |  typedefs unless you've got _OSF_SOURCE turned on.
 */

#if defined(__osf__) && !defined(_OSF_SOURCE)
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE
#else
# include <sys/types.h>
#endif

#include <stdio.h>
#include <syslog.h>
#include <strings.h>
#include <malloc.h>

/* includes required for "evd.h" */
#include "moss.h"
#include "man.h"
#include <netinet/in.h>
#include "evd_defs.h"
#include "evd.h"
#include "snmppe_pei_event.h"


#define EVENT_UID_MAX 99999999

/*
|
|   Define Prototypes for Module-Local Functions
|
*/

/* evd_find_queue_handle - Search for Queue Handle Block Entry */
static man_status
evd_find_queue_handle PROTOTYPE((
 evd_global_data_t    *,    /*-> Big Picture Structure pointer for EVD */
 event_queue_handle_t *     /* Event Queue Handle pointer              */
));


/* evd_add_queue_handle - Add an Event Queue Handle Block Entry */
static void
evd_add_queue_handle PROTOTYPE((
 evd_global_data_t    *,    /*-> Big Picture Structure pointer for EVD */
 event_queue_handle_t *     /* Event Queue Handle pointer              */
));


/* evd_remove_queue_handle - Remove an Event Queue Handle Block Entry */
static man_status
evd_remove_queue_handle PROTOTYPE((
 evd_global_data_t    *,    /*-> Big Picture Structure pointer for EVD */
 event_queue_handle_t *     /* Event Queue Handle pointer              */
));


/* creates an oid from an octet string */
static man_status
evd_snmp_octet_to_oid PROTOTYPE((
 octet_string *,            /* data         */
 object_id    **,           /* partial_oid  */
 int                        /* tag          */
));


/* evd_set_event_uid - Set the Event UID for an event. */
static void
evd_set_event_uid PROTOTYPE((
 evd_global_data_t  *,      /*-> Big Picture Structure pointer for EVD */
 uid                *       /*-> Event UID to be set                   */
));


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**	evd_create_queue_handle - This routine is used by a manageable object
**	                          to obtain a handle to an existing named 
**                                event queue (For EVD V1.1, this is NULL).
**
**  FORMAL PARAMETERS:
**
**	handle
**	    The Opaque handle returned to the caller upon successful
**	    completion.  The handle argument is the address of pointer into
**	    which the address of the queue handle is written "opaquely".
**
**	queue_name
**	    The name of the target event queue.  The queue_name argument is the
**	    address of an AVL specifying the name of the target event queue.
**	    If the NULL pointer is passed, the default queue is assumed.
**
**	access_mode
**	    The mode in which the queue is to be accessed.  The access_mode
**	    argument MUST specify EVD_POST since the queue handle will be used
**          by a manageable object to post events.
**
**  RETURN VALUE:
**
**      MAN_STATUS - Status.  See Error Codes below.
**
**  SIDE EFFECTS:
**
**	none
**
**  DESIGN:
**
**      This routine must be called in user mode.
**
**  ERROR CODES:
**
**	MAN_C_SUCCESS - 
**	    A queue handle was successfully allocated and initialized.
**	MAN_C_INSUFFICIENT_RESOURCES -
**	    Process virtual memory could not be allocated for queue handle.
**	MAN_C_BAD_PARAMETER -
**	    An invalid queue access mode was specified.
**
**
**---------------------------------------------------------------------------*/

man_status
evd_create_queue_handle (handle, queue_name, access_mode)

 evd_queue_handle       **handle;
 avl                     *queue_name;
 evd_queue_access_mode    access_mode;
{
    char                       msg[LINEBUFSIZE];  /* Message build buffer */
    event_queue_handle_t      *this_handle;
    extern evd_global_data_t  *evd_global_dataP; 
    evd_global_data_t         *bp;


    /* set up "big picture" global data pointer */
    bp = evd_global_dataP;
    *handle = (evd_queue_handle *) NULL; /* initially */

    /* Queue name is IGNORED for V1.1 */
/*
    if (queue_name != NULL)
        return (MAN_C_BAD_PARAMETER);
*/

    /* Queue access mode MUST be EVD_POST */
    if (access_mode != EVD_POST)
    {
        sprintf (msg, MSG(emsg038, "E038 - Invalid Queue Access Mode (%d)"),
                 access_mode);
        SYSLOG (LOG_ERR, msg);
        return (MAN_C_BAD_PARAMETER);
    }

    /* allocate an event queue handle */
    this_handle = (event_queue_handle_t *) 
                   malloc (sizeof(event_queue_handle_t));
    if (this_handle == NULL)
    {
        sprintf (msg, MSG(emsg039, 
                 "E039 - No memory for allocating queue handle (%d)"),
                 access_mode);
        SYSLOG (LOG_ERR, msg);
        return (MAN_C_INSUFFICIENT_RESOURCES); /* Don't CRASH! */
    }

    /* init queue handle, then attach it to the list held inside big picture */
    this_handle->check_value  = VALID_EVD_HANDLE;
    this_handle->access_mode  = access_mode;
    this_handle->self_handleP = (evd_queue_handle *) this_handle;
    this_handle->next         = NULL;

    evd_add_queue_handle (bp, this_handle);

    *handle = (evd_queue_handle *) this_handle;
    return (MAN_C_SUCCESS);

} /* end of evd_create_queue_handle */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**	evd_delete_queue_handle - This routine is called by a manageable object
**	                          to delete a queue handle that was previously
**                                obtained by calling evd_create_queue_handle.
**
**  FORMAL PARAMETERS:
**
**	handle
**	    The opaque queue handle returned to the caller by
**	    evd_create_queue_handle.  The handle argument is the address of a
**	    pointer to the queue handle to be deleted.
**
**  RETURN VALUE:
**
**      MAN_STATUS - Status (see below).
**
**  SIDE EFFECTS:
**
**      It is assumed for this release that for V1.1, evd_delete_queue_handle()
**      is handled in lock-step to its completion, therefore no MUTEX
**      locking is performed on the queue handle list inside the global data,
**      nor on the checking and setting of the queue handle's check_value.
**
**  DESIGN:
**
**      This routine must be called in user mode.
**
**  RETURN CODES:
**
**	MAN_C_BAD_HANDLE -
**	    The handle argument does not point to a valid handle
**	MAN_C_SUCCESS -
**	    The handle argument was successfully deallocated.
**
**--------------------------------------------------------------------------*/

man_status
evd_delete_queue_handle (handle)
 evd_queue_handle      **handle;
{
    char                   msg[LINEBUFSIZE];  /* Message build buffer */
    event_queue_handle_t  *this_handle;
    man_status             status;

    extern evd_global_data_t  *evd_global_dataP; 
    evd_global_data_t         *bp;


    /* set up "big picture" global data pointer */
    bp = evd_global_dataP;

    /* verify that the queue handle is legit */
    this_handle = (event_queue_handle_t *) *handle;
    status = evd_remove_queue_handle (bp, this_handle);

    if (status != MAN_C_SUCCESS)
    {
        sprintf (msg, MSG(emsg040,
                 "E040 - Invalid queue handle supplied (%ld)"),
                 (long int) this_handle);
        SYSLOG (LOG_ERR, msg);
    }
    else
    {
        free (this_handle);
        *handle = (evd_queue_handle *) NULL;
    }

    return (status);

}  /* end of evd_delete_queue_handle() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**	evd_post_event - This routine is called by a manageable object to post
**                       an event.
**
**  FORMAL PARAMETERS:
**
**	handle
**	    The opaque handle returned to the caller by
**	    evd_create_queue_handle.  The handle argument is the address of
**	    the event queue handle.
**
**	object_class
**	    The object class of the managed object that is posting the event.
**	    the object_class argument is address of the object ID which
**	    specifies the class.
**
**	instance_name
**	    The instance name of the managed object that is posting the event.
**	    The instance_name argument is the address of the AVL which
**	    specifies the name. **** IGNORED FOR CA V1.1 ****
**
**	event_time
**	    The time the event occurred in BinABsTim format.  The event_time
**	    argument is the address of the mo_time.  The event_time argument is
**	    optional.  If the NULL pointer is passed, the current time will be
**	    used. **** IGNORED FOR CA V1.1 ****
**
**	gen_event_type
**	    The generic event type identifier.  The event_type argument is the
**          address of the object ID which specifies the generic type of event
**          that occurred.
**
**	event_parameters
**	    The event parameters.  The event_parameters argument is the address
**	    of the AVL specifying the event parameters.
**
**	event_uid
**	    The uid assigned to the event by evd_post_event.  The event_uid
**	    argument is the address of a UID structure into which the event UID
**	    is writen.  If the NULL pointer is passed, the event uid is not
**	    returned.
**
**	entity_uid
**	    The uid assigned to the managed object posting the event.  The
**	    MO_UID argument is the address of the UID structure specifying the
**	    uid of the managed object. **** IGNORED FOR CA V1.1 ****
**
**  SIDE EFFECTS:
**
**      No MUTEX locking is performed on any data, as it is assumed for the V1.1
**      version of the Common Agent that each call into EVD is processed in
**      lock-step to completion with no contention for resources.
**
**  DESIGN:
**
**      This routine must be called in user mode.
**
**  ERROR CODES:
**
**	MAN_C_SUCCESS -
**          All went well.
**	MAN_C_PROCESSING_FAILURE -
**	    An unexpected error occurred, such as failure to obtain a
**          queue handle from the PE.
**	MAN_C_BAD_HANDLE -
**	    The handle argument does not point to a valid handle.
**      MAN_C_BAD_PARAMETER - 
**          One of the API function call arguments is invalid.
**	MAN_C_INSUFFICIENT_RESOURCES -
**	    Unable to allocate process virtual memory.
**
**      Also could be an error code passed back from pei_create_queue_handle()
**      or pei_report_event().
**
**
**--------------------------------------------------------------------------*/

man_status
evd_post_event (handle, 
                object_class, 
                instance_name, 
                event_time, 
                gen_event_type,
                event_parameters, 
                event_uid, 
                entity_uid)

 evd_queue_handle    *handle;
 object_id           *object_class;
 avl                 *instance_name;
 mo_time             *event_time;
 object_id           *gen_event_type;
 avl                 *event_parameters;
 uid                 *event_uid;
 uid                 *entity_uid;
{
    char        msg[LINEBUFSIZE];   /* Error Message build buffer        */
    int         retries;            /* retry count if PE not responding  */
                                    /* due to bad ("old") handle         */
    event_queue_handle_t      *this_handle;
    man_status                 status;
    evd_global_data_t         *bp;

    extern evd_global_data_t  *evd_global_dataP; 


    /* set up "big picture" global data pointer */
    bp = evd_global_dataP;

    /* verify that the queue handle is legit */
    this_handle = (event_queue_handle_t *) handle;

    status = evd_find_queue_handle (bp, this_handle);

    if (status != MAN_C_SUCCESS)
    {
        sprintf (msg, MSG(emsg041,
                 "E041 - Invalid queue handle supplied (%ld)"),
                 (long int) this_handle);
        SYSLOG (LOG_ERR, msg);
    }

    /*
     * Lock this handle - mostly because we want exclusive access to it.
     * (NOT FOR Common Agent V1.1)
     */
/*
    CHECK_PTHREAD_STATUS( pthread_mutex_lock (&this_handle->lock_m));
*/

    /* Validate the structure of each required argument; this checking is   */
    /* more lenient than that performed inside the PE.  (The PE duplicates  */
    /* much of this checking; I did this because we may have another        */
    /* interface someday...)                                                */
    if (status == MAN_C_SUCCESS)
    {
        /* validate the structure of the object class OID */
        if ( (object_class != NULL) &&
             ((object_class->count < 1) || (object_class->value == NULL)) )
        {
            sprintf (msg, MSG(emsg042, 
                     "E042 - Invalid object class OID passed in"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }

        /* validate the structure of the instance_name avl */
/***** IGNORED FOR CA V1.1 ****
        else if ( (instance_name == NULL) )
        {
            status = MAN_C_BAD_PARAMETER;
        }

        else if ( moss_avl_reset (instance_name) != MAN_C_SUCCESS )
        {
            status = MAN_C_BAD_PARAMETER;
        }
*/

        /* validate the structure of the event time */
/***** IGNORED FOR CA V1.1 ****
        else if (event_time == NULL)
        {
            status = MAN_C_BAD_PARAMETER;
        }
*/

        /* validate the structure of the gen_event_type OID */
        else if ( (gen_event_type == NULL) ||
                  (gen_event_type->count != 1) || 
                  (gen_event_type->value == NULL) ||
                  (*gen_event_type->value < 0) || 
                  (*gen_event_type->value > 6) )
        {
            sprintf (msg, MSG(emsg043, 
                     "E043 - Invalid generic event_type arg passed in"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }

        /* validate the structure of the event_parameters avl */
        else if (event_parameters == NULL)
        {
            sprintf (msg, MSG(emsg044, 
                     "E044 - NULL event_parameters avl passed in"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }

        else if ( moss_avl_reset (event_parameters) != MAN_C_SUCCESS )
        {
            sprintf (msg, MSG(emsg045, 
                     "E045 - moss_avl_reset on event_parameters failed"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }

        /* ignore the structure of the event_uid uid; it's optional */

        /* validate the structure of the entity_uid uid */
/***** IGNORED FOR CA V1.1 ****
        else if (entity_uid == NULL)
        {
            status = MAN_C_BAD_PARAMETER;
        }
*/

        /* EVD V1.1: We assume the SNMP Protocol Engine is the ONLY PE!!  */
        /*                                                                */
        /*           When more than one protocol engine exists, then we   */
        /*           will need to check for the "Well Known" OID prefixes */
        /*           defined at back of ISO_DEFS.H, and pick the proper   */
        /*           PE based on the value(s) of the OIDs in the args.    */

        /* if we don't yet have a handle into the PE, get one */
        if ( (status == MAN_C_SUCCESS) && (bp->snmp_queue_handle == NULL) )
        {
            retries = 5;
            do
            {
                /* add a thread-safe "sleep()" call (2 seconds?) to get */
                /* really fancy; for now, just a retry is sufficient.   */

                status = pei_create_queue_handle (&bp->snmp_queue_handle);
            }
            while ( (retries-- > 0) && (status != MAN_C_SUCCESS) );

            /* can't get a queue handle; PE must be dead, so give up */
            if (status != MAN_C_SUCCESS)
            {
                sprintf (msg, MSG(emsg046, 
                  "E046 - Unable to obtain queue handle from protocol engine"));
                SYSLOG (LOG_ERR, msg);
                status = MAN_C_PROCESSING_FAILURE;
            }
        }

        /* if all went well, post the event to the SNMP protocol engine */
        if ((status == MAN_C_SUCCESS) && 
            (bp->snmp_trap_listeners > 0) &&
            (bp->snmp_traps_disabled != 1))
        {
            status = pei_report_event (bp->snmp_queue_handle, 
                                       object_class, 
                                       instance_name, 
                                       event_time, 
                                       gen_event_type,
                                       event_parameters);

            /* Handle case where snmp_pe may have been re-started; obtain */
            /* a new queue handle, then retry the pei_report_event() call */
            if (status == MAN_C_BAD_HANDLE)
            {
                retries = 5;
                do
                {
                    /* add a thread-safe "sleep()" call (2 seconds?) to get */
                    /* really fancy; for now, just a retry is sufficient.   */

                    status = pei_create_queue_handle (&bp->snmp_queue_handle);
                }
                while ( (retries-- > 0) && (status != MAN_C_SUCCESS) );

                if (status == MAN_C_SUCCESS)
                {
                    /* if this call fails, don't retry; just return status */
                    status = pei_report_event (bp->snmp_queue_handle, 
                                               object_class, 
                                               instance_name, 
                                               event_time, 
                                               gen_event_type,
                                               event_parameters);
                }
                else /* couldn't get a handle; PE must be dead */
                {
                    sprintf (msg, MSG(emsg047,
                  "E047 - Unable to obtain queue handle from protocol engine"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                }
            }

            /* set event_uid if the event was processed OK */
            if ( (status == MAN_C_SUCCESS) && (event_uid != NULL) )
            {
                evd_set_event_uid (bp, event_uid);
            }
        }
    }

/*
    CHECK_PTHREAD_STATUS( pthread_mutex_unlock (&this_handle->lock_m));
*/

    return (status);

}  /* end of evd_post_event */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**      evd_find_queue_handle - Search for the Event Queue Handle Block Entry
**                              that matches the given queue handle.
**
**  INPUTS:
**
**    bp
**      This is a pointer to the Big Picture structure containing all the 
**      "global" context information needed by EVD to operate.
**
**    handleP
**      This is (potentially) a pointer to a valid and active event queue 
**      handle structure.
**      
**
**  OUTPUTS:
**
**
**  BIRD'S EYE VIEW:
**    Context:
**          The caller is one of the functions in this module that needs to
**          acquire the context corresponding to the reply just received.
**
**    Purpose:
**      This function performs the necessary search to return the
**      context needed to process the reply (a service block and the
**      varbind entry block that corresponds to the reply).
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      >>if MTHREADS
**          if (attempt to acquire queue_handle_list mutex failed)
**              <CRASH "Exxx - acquisition of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**          <scan list of handles for a VALID matching handle>
**
**      >>if MTHREADS
**          if (attempt to release queue_handle_list mutex failed)
**              <CRASH "Exxx - release of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**
**  OTHER THINGS TO KNOW:
**      None.
**
**
**--------------------------------------------------------------------------*/

static man_status
evd_find_queue_handle (bp, handleP)

 evd_global_data_t    *bp;       /*-> Big Picture Structure pointer for EVD  */
 event_queue_handle_t *handleP;  /*-> Queue handle to match */
{
    char                  msg[LINEBUFSIZE];  /* Error message build buffer */
    man_status            status;
    event_queue_handle_t *tempP;  /*-> Temporary Queue handle */

#ifdef MTHREADS
    /* if (attempt to acquire the Queue Handle List mutex failed) */
    if (pthread_mutex_lock (&bp->queue_handle_list_m) != 0) 
    {
        sprintf (msg, MSG(emsg030, 
              "E030 - acquisition of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif

    /* scan list of handles for a VALID matching handle */
    status = MAN_C_BAD_HANDLE;
    tempP = bp->queue_handle_list;

    while (tempP)
    {
        if (tempP == handleP)
        {
            if (handleP->check_value == VALID_EVD_HANDLE)
	    {
                status = MAN_C_SUCCESS;
                break;
	    }
            else
            {
                status = MAN_C_BAD_HANDLE;
                break;
            }
        }
        else
            tempP = tempP->next;
    }

#ifdef MTHREADS
    /* if (attempt to release the Queue Handle List mutex failed) */
    if (pthread_mutex_unlock (&bp->queue_handle_list_m) != 0) 
    { 
        sprintf (msg, MSG(emsg031, 
                 "E031 - release of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif

    return (status);

} /* end of evd_find_queue_handle() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**      evd_remove_queue_handle - Search for the Event Queue Handle Block 
**                                Entry that matches the given queue handle, 
**                                then dequeue the entry from the list, and 
**                                mark it as INVALID.
**
**  INPUTS:
**
**    bp
**      This is a pointer to the Big Picture structure containing all the 
**      "global" context information needed by EVD to operate. 
**
**    handleP
**      This is (potentially) a pointer to a valid and active event queue 
**      handle structure.
**      
**
**  OUTPUTS:
**
**      MAN_C_SUCCESS - 
**         All went well.
**      MAN_C_BAD_HANDLE - 
**         Queue Handle is invalid.
**
**      
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is one of the functions in this module that needs to
**        acquire the context corresponding to the reply just received.
**
**    Purpose:
**        This function performs the necessary search to return the
**        context needed to process the reply (a service block and the
**        varbind entry block that corresponds to the reply).
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      >>if MTHREADS
**          if (attempt to acquire queue_handle_list mutex failed)
**              <CRASH "Exxx - acquisition of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**          <scan list of handles for a VALID matching handle>
**          if (not found)
**              <return MAN_C_BAD_HANDLE>
**          else
**              <dequeue it from list and mark as INVALID>
**
**      >>if MTHREADS
**          if (attempt to release queue_handle_list mutex failed)
**              <CRASH "Exxx - release of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**
**  OTHER THINGS TO KNOW:
**      None.
**
**--------------------------------------------------------------------------*/

static man_status
evd_remove_queue_handle (bp, handleP)

 evd_global_data_t    *bp;       /*-> Big Picture Structure pointer for EVD  */
 event_queue_handle_t *handleP;  /*-> Queue handle to match */
{
    char                  msg[LINEBUFSIZE];  /* Error message build buffer */
    man_status            status;
    event_queue_handle_t *currentP, *prevP;  /* Temporary Queue handle ptrs */


#ifdef MTHREADS
    /* if (attempt to acquire the Queue Handle List mutex failed) */
    if (pthread_mutex_lock (&bp->queue_handle_list_m) != 0) 
    {
        sprintf (msg, MSG(emsg032, 
              "E032 - acquisition of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif


    /* if first element in list, reset big picture's list pointer */
    status = MAN_C_BAD_HANDLE;  /* assume the worst */
    if (bp->queue_handle_list == NULL)
        return (status);

    currentP = bp->queue_handle_list;
    if (currentP == handleP)
    {
        if (handleP->check_value == VALID_EVD_HANDLE)
        {
            bp->queue_handle_list  = handleP->next;
            currentP->check_value  = INVALID_EVD_HANDLE;
            currentP->self_handleP = NULL;
            status = MAN_C_SUCCESS;
        }
    }

    else /* handle is NOT first in list; scan rest of list */ 
    {
        prevP = bp->queue_handle_list;
        currentP = prevP->next;

        while (currentP)
        {
            if (currentP == handleP)
            {
                if (handleP->check_value == VALID_EVD_HANDLE)
                {
                    currentP->check_value = INVALID_EVD_HANDLE;
                    prevP->next = currentP->next;
                    status = MAN_C_SUCCESS;
                }
                else
                {
                    status = MAN_C_BAD_HANDLE; /* error logged in calling fcn */
                }
                break;
            }
            else /* reset pointers and re-do the test */
            {
                prevP = currentP;
                currentP = currentP->next;
            }
        }
    }

#ifdef MTHREADS
    /* if (attempt to release the Queue Handle List mutex failed) */
    if (pthread_mutex_unlock (&bp->queue_handle_list_m) != 0) 
    { 
        sprintf (msg, MSG(emsg033, 
                 "E033 - release of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif

    return (status);

} /* end of evd_remove_queue_handle() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**      evd_add_queue_handle - Add the given Event Queue Handle Block to the 
**                             Queue Handle List strung off of the Big Picture.
**
**  FORMAL PARAMETERS:
**
**    bp
**      This is a pointer to the Big Picture structure containing all the 
**      "global" context information needed by EVD to operate.
**
**    handleP 
**      This is (potentially) a pointer to a valid and active event queue 
**      handle structure.
**      
**
**  OUTPUTS:
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is one of the functions in this module that needs to
**        acquire the context corresponding to the reply just received.
**
**    Purpose:
**        This function performs the necessary search to return the
**        context needed to process the reply (a service block and the
**        varbind entry block that corresponds to the reply).
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      >>if MTHREADS
**          if (attempt to acquire queue_handle_list mutex failed)
**              <CRASH "Exxx - acquisition of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**          if (queue handle list empty>
**              <make handle be first in list>
**          else
**              <add queue handle to end of list>
**
**      >>if MTHREADS
**          if (attempt to release queue_handle_list mutex failed)
**              <CRASH "Exxx - release of Queue Handle List mutex failed, 
**               errno = %d">
**      >>endif
**
**
**  OTHER THINGS TO KNOW:
**      None.
**
**
**--------------------------------------------------------------------------*/

static void
evd_add_queue_handle (bp, handleP)

 evd_global_data_t    *bp;       /*-> Big Picture Structure pointer for EVD  */
 event_queue_handle_t *handleP;  /*-> Queue handle to match */
{
    char                  msg[LINEBUFSIZE];  /* Message build buffer */
    man_status            status;
    event_queue_handle_t *tempP;  /* Temporary Event Queue handle ptr */

#ifdef MTHREADS
    /* if (attempt to acquire the Queue Handle List mutex failed) */
    if (pthread_mutex_lock (&bp->queue_handle_list_m) != 0) 
    {
        sprintf (msg, MSG(emsg034, 
              "E034 - acquisition of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif

    if (bp->queue_handle_list == NULL)
    {
        bp->queue_handle_list = handleP;
    }
    else
    {
        /* go to end of queue handle list, then attach new element at end */
        tempP = bp->queue_handle_list;
        while (tempP)
        {
            if (tempP->next == NULL)
            {
                tempP->next = handleP;
                break;
            }
            else
                tempP = tempP->next;
        }
    }

#ifdef MTHREADS
    /* if (attempt to release the Queue Handle List mutex failed) */
    if (pthread_mutex_unlock (&bp->queue_handle_list_m) != 0) 
    { 
        sprintf (msg, MSG(emsg035, 
                 "E035 - release of Queue Handle List mutex failed, errno=%d"),
                 errno);
        CRASH(msg);
    }
#endif

} /* end of evd_add_queue_handle() */


/*
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_snmp_octet_to_oid - This routine creates an oid from an octet 
**                            string as outlined in the RFC 1212 section 
**                            4.1.6 (3) and Mark Sylor's white paper - 
**                            "SNMP in EMA".  The octet is handled as a
**                            count and string.  The first "arc" in the oid 
**                            is the count with each octet following as a 
**                            single "arc".  For example, the octet string 
**                            "abc" would be the oid 3.a.b.c, where the
**                            octet a, b, c are expanded out to an integer.
**
**   ARGUMENTS:
**
**     data           
**        The address of an octet string.
**     
**     partial_oid
**        The address of a pointer to an oid.
**
**     tag
**        Either ASN1_C_OCTET_STRING or ASN1_C_PRINTABLE_STRING.
**  
**
**   RETURN VALUES:
**  
**      Value (man_status) returned by moss_octet_to_oid(), including
**      MAN_C_SUCCESS, or
**
**      MAN_C_INSUFFICIENT_RESOURCES - 
**          Resource failure in processing the operation.
**
**  
**   SIDE_EFFECTS:
**    
**      None.
**
**--------------------------------------------------------------------------*/

static man_status
evd_snmp_octet_to_oid (data, partial_oid, tag)

 octet_string  *data;
 object_id    **partial_oid;
 int            tag;
{
    man_status return_status;
    int *string;
    int *tmp_str;
    int i;
    char *tmp_char_p = data->string;
    char c;
    octet_string tmp_string;


    tmp_string.length = sizeof(int) + (data->length * sizeof(int));
    string = (int *) malloc (tmp_string.length);
    if (string == NULL)
        return (MAN_C_INSUFFICIENT_RESOURCES);

    memset ( (void *) string, '\0', tmp_string.length);
    *string = data->length;
    tmp_str = string + 1;
    for (i = 0; i < data->length; i++)
    {
        c = *tmp_char_p;
        *tmp_str = (int) c;
        tmp_str++;
        tmp_char_p++;
    }

    tmp_string.string = (char *) string;
    tmp_string.data_type = tag;

    return_status = moss_octet_to_oid (&tmp_string, partial_oid);
    free (string);
    return (return_status);

} /* end of evd_snmp_octet_to_oid() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**      evd_set_event_uid - Create an Event UID for an event just posted.
**
**  FORMAL PARAMETERS:
**
**      bp
**          This is a pointer to the Big Picture structure containing all the 
**          "global" context information needed by EVD to operate.
**
**	event_uid
**	    The uid assigned to the event by evd_post_event.  The event_uid
**	    argument is the address of a UID structure into which the event UID
**	    is writen.  If the NULL pointer is passed, the event uid is not
**	    returned.
**
**  OUTPUTS:
**
**      Sets the event UID.
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is one of the functions in this module that needs to
**        acquire the context corresponding to the reply just received.
**
**    Purpose:
**        This function performs the necessary search to return the
**        context needed to process the reply (a service block and the
**        varbind entry block that corresponds to the reply).
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      >>if MTHREADS
**          if (attempt to acquire event_uid mutex failed)
**              <CRASH "Exxx - acquisition of Event UID mutex failed, 
**               errno = %d">
**      >>endif
**
**          <set event UID to current UID value, then increment/reset UID value>
**
**      >>if MTHREADS
**          if (attempt to release event_uid mutex failed)
**              <CRASH "Exxx - release of Event UID mutex failed, 
**               errno = %d">
**      >>endif
**
**
**  OTHER THINGS TO KNOW:
**      None.
**
**
**--------------------------------------------------------------------------*/

static void
evd_set_event_uid (bp, event_uid)

 evd_global_data_t  *bp;        /*-> Big Picture Structure pointer for EVD */
 uid                *event_uid; /*-> Event UID to be set                   */
{
    char msg[LINEBUFSIZE];  /* Message build buffer */


#ifdef MTHREADS
    /* if (attempt to acquire the Event UID mutex failed) */
    if (pthread_mutex_lock (&bp->event_uid_m) != 0) 
    {
        sprintf (msg,
                 MSG(emsg036, "E036 - acquisition of Event UID mutex failed, errno = %d"),
                 errno);
        CRASH(msg);
    }
#endif

    /* set event UID to current UID value, then increment UID value */
    *(int *)event_uid = bp->event_uid;
    if (bp->event_uid == EVENT_UID_MAX)
        bp->event_uid = 0;
    else
        bp->event_uid++;

#ifdef MTHREADS
    /* if (attempt to release the Event UID mutex failed) */
    if (pthread_mutex_unlock (&bp->event_uid_m) != 0) 
    { 
        sprintf (msg,
                 MSG(emsg037, "E037 - release of Event UID mutex failed, errno = %d"),
                 errno);
        CRASH(msg);
    }
#endif

} /* end of evd_set_event_uid() */

