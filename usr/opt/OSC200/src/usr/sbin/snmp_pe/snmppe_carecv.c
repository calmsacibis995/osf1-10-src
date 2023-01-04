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
static char *rcsid = "@(#)$RCSfile: snmppe_carecv.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:33:31 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1991, 1992
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
 * Common Agent SNMP Protocol Engine
 *
 * Module SNMPPE_CARECV.C
 *      Contains "Common Agent - Receive Functions" for the
 *      SNMP Protocol Engine for the Comman Agent.
 *      These are functions "RECEIVING replies FROM" the Common Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   October 1991
 *      with a big Tip 'o The Hat to Mary Walker/UEG whose original
 *      routines this modules' functions are directly derived from.
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accept requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that information.
 *       The main function has been entered, a PDU received, converted
 *       to Common Agent requests which have been issued.  Now the incoming
 *       replies must be handled.
 *
 *    Purpose:
 *       This module contains the functions that are entered when a reply
 *       from the Common Agent is generated.
 *
 * History
 *      V1.0    October 1991            D. D. Burns/Mary Walker
 *
 *      V1.x    April 14, 1992          Miriam Amos Nihart
 *      "Fixed in carecv_avl_to_oid() for dealing with constructed identifier.
 *      Need to increment the internal avl pointer so as not to read the
 *      same avl item twice, specifically the SOC item."
 *
 *      V1.1    May 1992                D. D. Burns
 *      Change "rollclass" to "rollababy" to support more proper rolling for
 *      GET-NEXT.
 *
 *      V1.2    Sept 1992                D. D. Burns
 *      Change "rollababy" to "rollattrib" to support
 *      more (!) proper rolling for GET-NEXT.
 *
 * NOTE:
 *       1) Change logic that seeks end of instance AVL to use
 *          avl_exit_construction.  (not yet done due to purported bugs in
 *          avl_exit construction)

Module Overview:
---------------

This module contains the SNMP PE function(s) that processing replies
coming into SNMP PE from the Common Agent that are to be transformed
back into response PDUs.  This module also contains the functions that
process event related requests coming into SNMP PE from the Common
Agent MOMs.


Thread Overview:
---------------

All the functions in this module are executed exclusively by the
subordinate "receiving" thread.

The event related functions in this module are executed exclusively by the listening
subordinate "receiving" thread of the SNMP PE.  The 'queue" used for receiving
events is virtually "1" element long; each event is processed to completion 
before returning control to the calling process.



MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------
pei_send_get_reply      The function entered by the Common Agent (via RPC
                        in an rpc/threaded environment) to reply to a
                        msi_get_attributes() invocation.  This function
                        extracts the necessary information from it's
                        argument list and records it in the appropriate spots
                        in the SNMP PE database (in the service block and
                        the varbind entry block within the service block) for
                        the given reply for subsequent handling by the
                        sending thread.

pei_send_action_reply   Operates as pei_send_get_reply() does, but for an
                        msi_invoke_action() call serving a "get-next" request.

pei_send_set_reply      Same as pei_send_get_reply() above, but for an
                        msi_set_attributes() call.

pei_send_delete_reply   A stub function containing error-logging code to
                        record any instance of this function being entered,
                        as it should never be entered by the Common Agent
                        (not used by SNMP).

pei_send_create_reply   Same as pei_send_delete_reply.

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

Function Name           Synopsis
-------------           --------
carecv_extract_event_parms  
                        Extracts required event parameters out of the
                        event_parameters AVL passed in through the
                        evd_post_event() API.

carecv_find_vb_entry    Given the invoke-id from a Common Agent reply, this
                        function searches the SNMP PE's internal list of
                        PDUs-being-processed (the "service_list" in the 'Big
                        Picture' structure) for the 'service_block' that
                        corresponds to the PDU from which this replies'
                        original Common Agent request was generated.  It
                        then searches the varbind entry list in that
                        service block for the varbind entry block(s) that
                        directly corresponds to the reply just received.

carecv_avl_to_oid       Converts information received from the Common Agent
                        in AVL form into an SNMP OID (which is placed inside
                        a Protocol Engine-based AVL) that can be submitted by
                        the sending thread code to standard PE functions for
                        resending in SNMP PDU-form to the SNMP manager.

man_snmp_octet_to_oid   Converts an octet string to an oid according to
                        RFC1212. (Tip 'o The Hat to M. Nihart).
*/

/* Module-wide Stuff */
#if defined(__osf__) && !defined(_OSF_SOURCE)
/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
   typedefs unless you've got _OSF_SOURCE turned on. */
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

/* includes required for "snmppe.h" */
#include "port_cmip.h"                  /* Contains nested headers */
#include "moss.h"
#include <netinet/in.h>
#define OID_DEF
#include "snmppe.h"

/* includes required for the ASN.1 call(s) */
/* #include "snmppe_interface_def.h" */
#include "snmppe_msg.h"
#include "snmppe_snmplib.h"


/* This defines prototypes for the standard "pei_*" Protocol Engine */
/* send-request  functions.                                         */
#ifndef NOIPC
#include "pe.h"
#endif /* NOIPC */

#include "moss_inet.h"
#include <arpa/inet.h>
#include "evd_defs.h"

#define MAX_ENTERPRISE_OID_SIZE 100
#define NO_ENTERPRISE_TRAP       99
#define NO_SPECIFIC_TRAP         -1
#define NO_EVENT_TIME             0
#define NO_AGENT_ADDRESS          0

#define EVENT_UID_MAX 999999

/*
|=============================================================================
| This macro builds an error message using a specified OID variable as the
| source of an OID is to appear in the message.  The message is built
| in buffer named "msg".
|
| This macro serves to make the code a little easier and hide the details
| of the conversion of the oid to text.
|=============================================================================
*/
#define BLD_OID_ERRMSG( template, oid )                                    \
    {                                                                      \
    char    *textual_oid;   /* Temp storage for error message           */ \
    BOOL    release=TRUE;   /* TRUE: Release the textual OID storage    */ \
                                                                           \
    if (moss_oid_to_text(oid, NULL,NULL,NULL, &textual_oid)                \
        != MAN_C_SUCCESS) {                                                \
        textual_oid = "<unable to convert OID to text>";                   \
        release = FALSE;                                                   \
        }                                                                  \
    sprintf(msg, template, textual_oid);                                   \
    if (release == TRUE)                                                   \
       free(textual_oid);                                                  \
    }


/*
|
|   Define Prototypes for Module-Local Functions
|
*/

/* carecv_find_vb_entry - Search for Service Block Varbind List Entry */
static void
carecv_find_vb_entry PROTOTYPE((
big_picture_t   **,        /*->> Big Picture Structure pointer for SNMP PE */
service_t       **,        /*->> Service Blk pointer found                 */
varbind_t       **,        /*->> Varbind Entry pointer found               */
int                        /* Invoke ID for search                         */
));


/* carecv_avl_to_oid - Convert AVL-based data to SNMP OID */
static void
carecv_avl_to_oid PROTOTYPE((
big_picture_t   *,             /*->Big Picture Structure pointer for SNMP PE */
varbind_t       *,             /*->Varbind Entry pointer found               */
avl             *,             /*->Object Instance AVL from CA reply         */
avl             *              /*->Attribute List AVL from CA reply          */
));


/* creates an oid from an octet string */
static man_status
man_snmp_octet_to_oid PROTOTYPE((
octet_string *,         /* data         */
object_id    **,        /* partial_oid  */
int                     /* tag          */
));


/* carecv_extract_event_parms - Extract required event parameters from AVL. */
static man_status
carecv_extract_event_parms PROTOTYPE ((
 big_picture_t      *,    /*-> Big Picture Structure pointer for SNMP PE */
 avl                *,    /*-> Event Parameters avl                      */
 object_id          *,    /*-> Enterprise OID                            */
 unsigned int       *,    /*-> Event time                                */
 int                *,    /*-> Event type OID                            */
 unsigned int       *,    /*-> Agent IP address                          */
 int                *,    /*-> Specific Trap number OID                  */
 int                *,    /*-> Generic Trap number OID                   */
 MCC_T_Descriptor   *     /*-> Descriptor containing ASN.1 varbind args  */
));


/* evd_set_event_uid - Set the Event UID for an event. */
static void
evd_set_event_uid PROTOTYPE((
 big_picture_t      *,    /*-> Big Picture Structure pointer for SNMP PE */
 uid                *     /*-> Event UID to be set                       */
));

/*
|
|   Declare "Well Known" OIDs for Parsing Event Arguments
|
*/

/*-------------------------------
| Event Type OID Argument
*-------------------------------*/

static
unsigned int
    prefix_event_eventtype_array[ PREFIX_EVENTTYPE_LENGTH ] = 
        { PREFIX_EVENTTYPE_SEQ };

static
object_id
    prefix_event_eventtype_oid = { PREFIX_EVENTTYPE_LENGTH,
                                   prefix_event_eventtype_array };


/*-------------------------------
| Enterprise OID Event Argument
*-------------------------------*/

static
unsigned int
    prefix_event_enterprise_array[ PREFIX_ENTERPRISE_LENGTH ] = 
        { PREFIX_ENTERPRISE_SEQ };

static
object_id
    prefix_event_enterprise_oid = { PREFIX_ENTERPRISE_LENGTH,
                                    prefix_event_enterprise_array };


/*----------------------------------
| Agent Address OID Event Argument
*----------------------------------*/

static
unsigned int
    prefix_event_agentaddr_array[ PREFIX_AGENTADDR_LENGTH ] = 
        { PREFIX_AGENTADDR_SEQ };

static
object_id
    prefix_event_agentaddr_oid = { PREFIX_AGENTADDR_LENGTH,
                                   prefix_event_agentaddr_array };


/*---------------------------------
| Generic Trap OID Event Argument
*---------------------------------*/

static
unsigned int
    prefix_event_generictrap_array[ PREFIX_GENERICTRAP_LENGTH ] = 
        { PREFIX_GENERICTRAP_SEQ };

static
object_id
    prefix_event_generictrap_oid = { PREFIX_GENERICTRAP_LENGTH,
                                     prefix_event_generictrap_array };


/*----------------------------------
| Specific Trap OID Event Argument
*----------------------------------*/

static
unsigned int
    prefix_event_specifictrap_array[ PREFIX_SPECIFICTRAP_LENGTH ] = 
        { PREFIX_SPECIFICTRAP_SEQ };

static
object_id
    prefix_event_specifictrap_oid = { PREFIX_SPECIFICTRAP_LENGTH,
                                      prefix_event_specifictrap_array };


/*-------------------------------
| Eventtime OID Event Argument
*-------------------------------*/

static
unsigned int
    prefix_event_eventtime_array[ PREFIX_EVENTTIME_LENGTH ] = 
        { PREFIX_EVENTTIME_SEQ };

static
object_id
    prefix_event_eventtime_oid = { PREFIX_EVENTTIME_LENGTH,
                                   prefix_event_eventtime_array };


/*--------------------------------
| Varbind Arg OID Event Argument
*--------------------------------*/

static
unsigned int
    prefix_event_varbindarg_array[ PREFIX_VARBINDARG_LENGTH ] = 
        { PREFIX_VARBINDARG_SEQ };

static
object_id
    prefix_event_varbindarg_oid = { PREFIX_VARBINDARG_LENGTH,
                                    prefix_event_varbindarg_array };



/* carecv_find_vb_entry - Search for Service Block Varbind List Entry */
/* carecv_find_vb_entry - Search for Service Block Varbind List Entry */
/* carecv_find_vb_entry - Search for Service Block Varbind List Entry */

static void
carecv_find_vb_entry( bp_addr, svc, vbe, invoke_id )

big_picture_t   **bp_addr; /*->> Big Picture Structure pointer for SNMP PE */
service_t       **svc;     /*->> Service Blk pointer found                 */
varbind_t       **vbe;     /*->> Varbind Entry pointer found               */
int             invoke_id; /* Invoke ID for search                         */


/*
INPUTS:

    "bp_addr" is the address of a pointer to the Big Picture structure containing
    all the "global" context information needed by SNMP PE to operate.  This
    is RETURNED.

    "svc" is the address of a pointer (to a Service Block) that is to
    be set upon return to point to the Service Block for the reply whose
    invoke_id is supplied.

    "vbe" is the address of a pointer to the Varbind entry block (in
    the same Service Block returned by "svc") that corresponds to the PDU's
    varbind entry to which the reply with "invoke_id" corresponds.  The
    varbind block address is returned to this pointer.

    "invoke_id" is the invoke_id as received in the reply.


OUTPUTS:

    This function always returns the Big Picture pointer.

    If the "invoke_id" corresponds to a service block and varbind entry
    block (within that service block) that can be found in the SNMP PE
    database (database=Big Picture's "service_list"), then both the
    service block address and varbind entry block address are returned.

    If either the service or varbind entry block cannot be found, then
    the corresponding pointer is set NULL.

    In a multiply-threaded/RPC environment access to the Service List is
    protected by a mutex in the Big Picture.


BIRD'S EYE VIEW:
    Context:
        The caller is one of the functions in this module that needs to
        acquire the context corresponding to the reply just received.

    Purpose:
        This function performs the necessary search to return the
        context needed to process the reply (a service block and the
        varbind entry block that corresponds to the reply).


ACTION SYNOPSIS OR PSEUDOCODE:

    <grab the big picture pointer and return it>

if MTHREADS
    if (attempt to acquire service_list mutex failed)
        <CRASH "Mxxx - acquisition of Service List mutex failed, errno = %d">
endif

    <convert invoke-id into just the service-block "svc_count">

    <return NULL to service and varbind entry block pointers>

    for (all service blocks)
        if (service-block service-count matches invoke id's svc count)
            for (all varbind entry blocks)
                if (varbind entry id matches the invoke id)
                    <return service block and varbind entry block addresses> 
if MTHREADS
                    if (attempt to release service_list mutex failed)
                        <CRASH "Mxxx - release of Service List mutex failed, errno = %d">
endif
                    return
if MTHREADS
    if (attempt to release service_list mutex failed)
        <CRASH "Mxxx - release of Service List mutex failed, errno = %d">
endif


OTHER THINGS TO KNOW:

    Well, we violate the "No Global Data" policy with code in this
    function which references the stored pointer to the "Big Picture"
    structure in the module in "snmppe_main.c".  There doesn't seem
    to be an easy way, given the thread situation, to avoid this.  In
    any event, it is a pretty mild violation.

    Once the Big Picture is obtained, we can search the "service_list"
    for the desired information.

<TC>    In function "carecv_find_vb_entry()" . . .
<TC>    The "service_list" may be undergoing changes as another sending thread
<TC>    adds entries to it while a receiving thread is busy trying to find
<TC>    an entry on it.  Consequently it is protected by a mutex in a multiply
<TC>    threaded environment.
<TC>
<TC>    If the convention (established in "snmppe_casend.c") for the functions
<TC>    that issue requests to the CA is religiously followed (that is, the
<TC>    functions never return until all replies have been received), then
<TC>    THIS function should never fail to find a service block and varbind
<TC>    entry corresponding to the invoke-id it is given (provided that didn't
<TC>    get trashed).
<TC>
<TC>    Should that convention be violated, a service block may be dumped while
<TC>    a reply is outstanding for it, and this function will fail to find it.
<TC>    Such a failure is logged, and the reply is dropped.
<TC>
<TC>    In general, dropping a reply may cause a sending thread to think its
<TC>    gotten a valid reply when it really hasn't.  Something is really
<TC>    wrong if this happens, a bogus reply may ultimately be sent to the
<TC>    inquiring SNMP manager.
*/


{
/* Yep, External.  See OTHERTHINSGTOKNOW in carecv_find_vb_entry() */
extern big_picture_t *big_picture;

int     desired_svc_count;      /* We're gonna search for this service block */

big_picture_t   *bp;            /* (Local name, used in macros)              */
varbind_t       *vbe_scanner;   /* scanning ptr for the Varbind entry list   */
service_t       *svc_scanner;   /* scanning ptr for the Service Block list   */


/* grab the big picture pointer and return it
|
| "big_picture" is globally accessible cell in module "snmppe_main.c"
| that has been initialized by the sending thread.
*/
*bp_addr = bp = big_picture;

#ifdef MTHREADS
/* if (attempt to acquire the Service List mutex failed) */
if (pthread_mutex_lock(&bp->service_list_m) != 0) {
    char    buf[LINEBUFSIZE];       /* Error message build buffer       */
    sprintf( buf,
     MSG(msg085, "M085 - acquisition of Service List mutex failed, errno = %d"),
            errno);
    CRASH( buf );
    }
#endif

/* convert invoke-id into just the service-block "svc_count"
|
|  This invoke-id was drawn from the varbind entry block id cell which
|  was constructed like this (in "netio_deserialize_pdu()"):
|
|                           (bottom 3 bytes)   (bottom 1 byte)
| vb_current->vb_entry_id = (svc_count << 8) | (vb_count & 0xFF);
|
| All we're interested in here is the "svc_count" value.
*/
desired_svc_count = (invoke_id >> 8) & 0xFFFFFF;

/* return NULL to service and varbind entry block pointers
|  (assume we fail)
*/
*vbe = NULL;
*svc = NULL;

/* for (all service blocks) */
for (svc_scanner = bp->service_list;
     svc_scanner != NULL;
     svc_scanner = svc_scanner->next ) {

    /* if (service-block service-count matches invoke id's svc count) */
    if (svc_scanner->svc_count == desired_svc_count) {

        /* for (all varbind entry blocks) */
        for (vbe_scanner = svc_scanner->varbind_list;
             vbe_scanner != NULL;
             vbe_scanner = vbe_scanner->next) {

            /* if (varbind entry id matches the invoke id) */
            if (vbe_scanner->vb_entry_id == invoke_id) {

                /* return service block and varbind entry block addresses
                |  (We found 'em!)
                */
                *vbe = vbe_scanner;
                *svc = svc_scanner;
#ifdef MTHREADS
                /* if (attempt to release service_list mutex failed) */
                if (pthread_mutex_unlock(&bp->service_list_m) != 0) { 
                    char    buf[LINEBUFSIZE]; /* Error message build buffer */
                    sprintf( buf,
                     MSG(msg086, "M086 - release of Service List mutex failed, errno = %d"),
                            errno);
                    CRASH( buf );
                    }
#endif
                return;         /* RETURN on success */
                }               /* if "we matched varbind entry" */
            }           /* for all varbind entry blocks */
        }       /* if "we matched service block" */
    }   /* for all service blocks */

/*
| If we fall out here, the invoke id didn't specify a valid service block
| and varbind entry.  This is an anomalous situation, and we want to log it.
| However, rather than logging it here "near" the error according to convention
| we allow the caller to log it because the caller has access to the OID
| which should appear in the message to allow easier debugging.
*/

#ifdef MTHREADS
/* if (attempt to release the Service List mutex failed) */
if (pthread_mutex_unlock(&bp->service_list_m) != 0) { 
    char    buf[LINEBUFSIZE];       /* Error message build buffer   */
    sprintf( buf,
            MSG(msg087, "M087 - release of Service List mutex failed, errno = %d"),
            errno);
    CRASH( buf );
    }
#endif

}

/* carecv_avl_to_oid - Convert AVL-based data to SNMP OID */
/* carecv_avl_to_oid - Convert AVL-based data to SNMP OID */
/* carecv_avl_to_oid - Convert AVL-based data to SNMP OID */

static void
carecv_avl_to_oid( bp, vbe, obj_instance, attrib_list )

big_picture_t   *bp;           /*->Big Picture Structure pointer for SNMP PE */
varbind_t       *vbe;          /*->Varbind Entry pointer found               */
avl             *obj_instance; /*->Object Instance AVL from CA reply         */
avl             *attrib_list;  /*->Attribute List AVL from CA reply          */


/*
INPUTS:

    "bp" is the address of a pointer to the Big Picture structure containing
    all the "global" context information needed by SNMP PE to operate.

    "svc" is the pointer to the Service Block for the PDU one of whose 
    varbind entries corresponds to reply is being processed.

    "vbe" is the address of the Varbind entry block (in the "svc" Service
    Block) that corresponds to the CA reply being processed (as described
    above).

    "obj_instance" is the object instance AVL from the reply.

    "attrib_list" is the attribute list AVL from the reply.


OUTPUTS:

    This function leaves "tracks" in the varbind entry block indicating
    how this reply fared.  In particular:

    * If the processing performed by this function was successful, then
      a single element is created in the "out_entry" AVL in the varbind entry
      block containing the "reconstructed" SNMP OID (containing the
      instance information), and also containing any data requested by
      a GET or GET-NEXT.  The "reply_error" cell in the varbind entry block
      is set to "noError", signaling success.

    * If there was a problem processing this request, it is mapped to a
      "genErr", which is left in the "reply_error" cell signaling failure.
      Depending on the source of the problem within this function, the
      error may be logged internally.


BIRD'S EYE VIEW:
    Context:
        The caller is one of the functions in this module that needs to
        convert Common Agent reply information "back" into SNMP format.

    Purpose:
        This function performs the necessary jiving to massage-up
        an SNMP OID in an AVL entry w/data so that the sending thread
        can ultimately repackage it into a bona-fide SNMP PDU.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (there is no object class OID in the varbind entry block)
        <SYSLOG - "Mxxx - Object Class information for reply is missing">
        <show "genErr" in reply_error>
        <return>

    if (attempt to reset the object instance AVL failed)
        <SYSLOG - "Mxxx - Attempt to reset object instance AVL failed %d">
        <show "genErr" in reply_error>
        <return>

    while (last element not encountered)

        if (attempt to point to next element failed)
            <SYSLOG - "Mxxx - Attempt to point failed %d">
            <show "genErr" in reply_error>
            <return>

    if (attempt to point BACKWARD one element failed)
        <SYSLOG - "Mxxx - Attempt to point backward failed %d">
        <show "genErr" in reply_error>
        <return>

    if (current element tag is END OF SEQUENCE)
        while (current element tag is not START OF SEQUENCE)
            if (attempt to point to PREVIOUS element failed)
                <SYSLOG - "Mxxx - Attempt to point failed %d">
                <show "genErr" in reply_error>
                <return>

    <show no instance-fragment OID>
    <show no new instance OID>

    <step pointer to MIR instance information down to the bottom-most class>

    for (each piece of MIR instance information for this object class)

        if (attempt to point to next element in instance AVL failed)
            <SYSLOG "Mxxx - Attempt to point to next instance element failed %d">
            <show "genErr" in reply_error>
            <release storage for instance oid>
            <return>

        if (MIR instance tag does not match AVL tag)
            <SYSLOG "Mxxx - MIR instance tag %hex doesn't match AVL tag %hex">
            <show "genErr" in reply_error>
            <release storage for instance oid>
            <return>

        if (MIR instance OID does not match AVL OID)
            <SYSLOG "Mxxx - MIR instance OID (oid) doesn't match AVL OID (oid)">
            <show "genErr" in reply_error>
            <release storage for instance oid>
            <return>

        if (AVL octet_string length is NON-ZERO)

            switch (MIR Instance Tag)

                case INET_C_SMI_COUNTER:
                case INET_C_SMI_GAUGE:
                    (* NOTE: These values are UNSIGNED! *)
                case ASN1_C_INTEGER:
                    (* NOTE: Integers are SIGNED! *)
                    if (attempt to create an instance-fragment oid with one
                        arc failed)
                        <CRASH - "Mxxx - Creation of instance OID failed %d">
                    break;

                case ASN1_C_PRINTABLE_STRING:
                case ASN1_C_OCTET_STRING:
                    if (attempt to create inst oid from octet string FAILED)
                        <CRASH - "Mxxy - Creation of instance OID failed %d">
                    break;

                case INET_C_SMI_IP_ADDRESS:
                    <construct printable string for INET address>
                    if (conversion of string to oid fails)
                        <CRASH - "Mxxz - Creation of instance OID failed %d">
                    break;

                case ASN1_C_OBJECTID:
                    (* NOTE: When implementing this, remember that Object ID *)
                    (*       arcs are unsigned.                              *)
                default:
                    <SYSLOG
                     "Mxxx - Unsupported SNMP Instance translation for
                       Tag %hex, Object Class (oid)">
                    <show "genErr" in reply_error>
                    <release storage for instance oid>
                    <return>

            if (attempt to concatenate inst-fragment onto instance oid failed)
                <CRASH "Mxxx - Concatenation of instance oid fragment failed
                               %d>
            <swap temporary oid and new-arcs oid pointers>
            if (attempt to release storage for temp oird failed)
                <CRASH "Mxxx - Release of temporary oid  failed %d>

            if (attempt to release storage for inst-fragment oid failed)
                <CRASH "Mxxx - Release of instance oid fragment failed %d>

        (* End of Loop *)

    (* Sanity check, be sure we're at the end of the instance AVL *)

    <show tag as -1>
    <save status from attempt to point to next element in instance AVL>

    if (status is not valid or tag is not "end of construction")
        <SYSLOG "Mxxx- Ill-formed AVL for object class (oid) Tag %d Status %d">
        <show "genErr" in reply_error>
        if (instance oid exists)
            if (attempt to free instance oid fails)
                <CRASH "Mxxx - Instance OID free attempt failed">
        <return>

    (* Obtain the attribute oid and concatenate either the instance oid *)
    (* generated above or ".0" onto the end to form the full SNMP OID   *)
    if (attempt to reset the attribute list AVL failed)
        <CRASH "Mxxx - AVL reset on attribute list AVL failed %d">

    if (point attempt on the attribute list AVL failed)
        <CRASH "Mxxx - AVL point on attribute list AVL failed %d">

    if (instance oid has a zero length)
        if (attempt to create an oid with one arc of "0" failed)
            <CRASH "Mxxx - oid creation attempt failed %d">

    if (attempt to concatenate instance oid onto attribute list oid failed)
        <CRASH "Mxxx - oid concatenation attempt failed %d">

    if (attempt to release storage for instance oid failed)
        <CRASH "Mxxx - Release attempt for instance oid storage failed %d>

    if (attempt to init the "out_entry" varbind block AVL failed)
        <CRASH "Mxxx - Init attempt for "out_entry" AVL failed %d">

    if (attempt to add full oid, attribute modifier tag and data failed)
        <CRASH "Mxxx - Add attempt to output AVL failed %d">

    if (attempt to free full oid failed)
        <CRASH "Mxxx - OID free storage attempt failed %d">

    <set reply_error cell to "noError" to signal success to sending thread>


OTHER THINGS TO KNOW:

    This function contains the back-flip-from-a-handstand manipulations
    required to "encode" instance information back into an SNMP-conformant
    object identifier.

    The requirements for encoding instance information into an OID are 
    described in Section 4.1.6 "Mapping of the Index Clause" on
    Pages 8 and 9 of RFC1212.  This function endeavors to implement all
    of the encoding algorithms currently required to encode known instances
    in the MIB (but not all the types described in RFC1212).

    This function uses MIR-derived information about the datatypes of
    the "identifier attributes" (the entries in the INDEX clause) for
    the particular object class.  It is the same information that the
    sending thread got when it decoded the original SNMP object id.  The
    sending thread leaves this MIR-derived information in the varbind
    entry block in cells "class_oid" and "inst_list" specifically so
    we need not look the same stuff up again here.  This scheme presumes
    that the MOM never rolls the object class on a get-next (ie the
    object_class OID returned in the reply is the same one submitted in
    the request).    
*/

{
char            msg[LINEBUFSIZE]; /* Error Message build buffer         */
object_id       *oid;             /* Object Instance OID                */
object_id       *new_arcs;        /* New Instance arcs being created    */
object_id       *inst_frag;       /* Fragment of new Instance Arcs      */
object_id       *temp_oid;        /* Temporary OID pointer              */
object_id       *full_oid=NULL;   /* Final reconstructed SNMP OID       */
unsigned int    modifier;         /* AVL modifier                       */
unsigned int    tag;              /* AVL Tag                            */
octet_string    *os;              /* AVL Octet String data structure    */
int             last=FALSE;       /* AVL "last" flag                    */
snmp_instance_t *MIR_inst;        /* Scanner for MIR-derived inst list  */
man_status      status;           /* General purpose status holder      */


/* if (there is no object class OID in the varbind entry block) */
if (vbe->class_oid.count == 0) {
    SYSLOG(LOG_ERR, MSG(msg088, "M088 - Object Class information for reply is missing"));
    vbe->reply_error = genErr;   /* show "genErr" in reply_error */
    return;
    }

/* if (attempt to reset the object instance AVL failed) */
if ((status = moss_avl_reset(obj_instance)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg089, "M089 - Attempt to reset obj_inst AVL failed: %d"), status);
    SYSLOG( LOG_ERR, msg );
    vbe->reply_error = genErr;   /* show "genErr" in reply_error */
    return;
    }

/*
| What we do now is walk to the last element in the AVL because this
| element corresponds to the instance information that must be translated
| back into SNMP OID instance arcs.  The last element in the AVL may be
| a constructed sequence OR just a single element.  Either way, we have
| to adjust the AVL "read point" so that the next forward "point" gets either
| the first element in the constructed sequence or the only element there.
|
|  Walk down to the bitter end first, it should be an ASN1_C_EOC that marks
|  the end of the entire sequence that makes up the instance name
|
|  while (last element not encountered)
*/
/*
| NOTE: When "snmp_higher_class_hack" code in snmppe_casend.c is removed,
|       this "walk-to-the-bottom" code should be removed also, so that
|       higher-class identifier attribute values participate in rebuilding
|       the original SNMP OID
*/
while (last != TRUE ) {

    status = moss_avl_point(obj_instance, &oid, &modifier, &tag, &os, &last);

    /* if (attempt to point to next element failed) */
    if (status != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg090, "M090 - Attempt to step forward failed %d %d"),
                status,
                tag);
        SYSLOG( LOG_ERR, msg );
        vbe->reply_error = genErr;   /* show "genErr" in reply_error */
        return;
        }
    }

/*
|  Now we back up one AVL element.  This should skip the EOC that must
|  be at the end of all object instance AVLs.
|
| if (attempt to point BACKWARD one element failed)
*/
status = moss_avl_backwards_point(obj_instance,
                                  &oid,
                                  &modifier,
                                  &tag,
                                  &os,
                                  &last);
if (status != MAN_C_SUCCESS) {
    sprintf(msg,MSG(msg199, "M199 - Attempt to point backward failed %d"), status);
    SYSLOG( LOG_ERR, msg );
    vbe->reply_error = genErr;   /* show "genErr" in reply_error */
    return;
    }

/*
|  Now we back up one more AVL element.  If we encounter an ASN1_C_EOC, then
|  the last element is a constructed sequence, and we've got to back up
|  to the beginning of it.
|
| if (attempt to point BACKWARD one element failed)
*/
status = moss_avl_backwards_point(obj_instance,
                                  &oid,
                                  &modifier,
                                  &tag,
                                  &os,
                                  &last);
if (status != MAN_C_SUCCESS) {
    sprintf(msg,MSG(msg196, "M196 - Attempt to point backward failed %d"), status);
    SYSLOG( LOG_ERR, msg );
    vbe->reply_error = genErr;   /* show "genErr" in reply_error */
    return;
    }


/* if (current element tag is END OF SEQUENCE) */
if (tag == ASN1_C_EOC) {
    /* while (current element tag is not START OF SEQUENCE) */
    while (tag != ASN1_C_SEQUENCE) {
        status = moss_avl_backwards_point(obj_instance,
                                          &oid,
                                          &modifier,
                                          &tag,
                                          &os,
                                          &last);
        /* if (attempt to point to PREVIOUS element failed) */
        if (status != MAN_C_SUCCESS) {
            sprintf(msg,MSG(msg197, "M197 - Attempt to point backward failed %d"), status);
            SYSLOG( LOG_ERR, msg );
            vbe->reply_error = genErr;   /* show "genErr" in reply_error */
            return;
            }        
        }

    /*
    | move internal pointer forward so the next moss_avl_point will not
    | reread the SOC.
    */
    status = moss_avl_next( obj_instance ) ;
    if ( status != MAN_C_SUCCESS ) {
        sprintf(msg,
                MSG(msg243, "M243 - Attempt to move avl pointer forward failed %d"),
                status);
        SYSLOG( LOG_ERR, msg );
        vbe->reply_error = genErr;      /* show "genErr" as the reply */
        return;
        }
    }

/* show no instance-fragment OID */
/* show no new instance OID      */
temp_oid = new_arcs = inst_frag = NULL;

/* step pointer to MIR instance information down to the bottom-most class
|
| We're skipping all of the MIR-derived instance info for the higher-level
| 'containing' classes with this manuever since only the bottom-most class
| instance info actually needs to be translated back into SNMP OID arcs.
*/
for (MIR_inst = vbe->inst_list;         /* Start at the top . . .        */
     MIR_inst->next_class != NULL;      /* Stop when we reach the bottom */
     MIR_inst=MIR_inst->next_class);    /* . . . step down a class       */

/* for (each piece of MIR instance information for this object class) */
for (; MIR_inst != NULL; MIR_inst=MIR_inst->next) {

    /* if (attempt to point to next element in instance AVL failed) */
    status = moss_avl_point(obj_instance, &oid, &modifier, &tag, &os, &last);
    if (status != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg091, "M091 - Point attempt on instance AVL failed %d"), status);
        SYSLOG( LOG_ERR, msg );
        vbe->reply_error = genErr;   /* show "genErr" in reply_error */

        /* release storage for instance oid */
        if (new_arcs != NULL) {
            if ((status = moss_free_oid(new_arcs)) != MAN_C_SUCCESS) {
                CRASH(MSG(msg092, "M092 - Free oid failed"));
                }
            }
        return;
        }

    /* if (MIR instance tag does not match AVL tag) */
    if (MIR_inst->AVLtag != tag) {
        sprintf(msg, MSG(msg093, "M093 - MIR instance tag %d doesn't match AVL tag %d"),
                MIR_inst->AVLtag, tag);
        SYSLOG( LOG_ERR, msg );
        vbe->reply_error = genErr;   /* show "genErr" in reply_error */

        /* release storage for instance oid */
        if (new_arcs != NULL) {
            if ((status = moss_free_oid(new_arcs)) != MAN_C_SUCCESS) {
                CRASH(MSG(msg094, "M094 - Free oid failed"));
                }
            }
        return;
        }

    /* if (MIR instance OID does not match AVL OID) */
    if (moss_compare_oid(oid, &MIR_inst->inst_oid) != MAN_C_EQUAL) {
        /* Create the text string representation of both OIDs */
        char    *mir_oid_text;
        char    *avl_oid_text;

        if (moss_oid_to_text(&MIR_inst->inst_oid,
                             NULL,
                             NULL,
                             NULL,
                             &mir_oid_text) != MAN_C_SUCCESS)
            mir_oid_text = "";

        if (moss_oid_to_text(oid,
                             NULL,
                             NULL,
                             NULL,
                             &avl_oid_text) != MAN_C_SUCCESS)
            avl_oid_text = "";

        /*
        | NOTE: The following error can occur if OIDs hardwired into a MOM
        |       don't match the OIDs in the MSL that describes the object
        |       supported by the MOM.
        */
        sprintf(msg, MSG(msg095, "M095 - MIR instance OID (%s) doesn't match AVL OID (%s)"),
                mir_oid_text, avl_oid_text);

        SYSLOG( LOG_ERR, msg );

        if (strlen(mir_oid_text) > 0) free(mir_oid_text);
        if (strlen(avl_oid_text) > 0) free(avl_oid_text);

        vbe->reply_error = genErr;   /* show "genErr" in reply_error */
        return;
        }

    /* if (AVL octet_string length is NON-ZERO)
    |  (In other words, if we really do have instance data to crunch)
    */
    if (os->length > 0) {

        switch (MIR_inst->AVLtag) {     /* MIR Instance Tag */

            /* ============================================================= */
            case INET_C_SMI_COUNTER:
            case INET_C_SMI_GAUGE:
                /* NOTE: These values are UNSIGNED! */
            case ASN1_C_INTEGER:
                /* NOTE: Integers are SIGNED! */

                /* if (attempt to create an instance-fragment oid with one
                |       arc failed)
                */
                if ((status = moss_create_oid(1,
                                              (unsigned int  *)os->string,
                                              &inst_frag)) != MAN_C_SUCCESS) {
                    sprintf(msg,
                            MSG(msg096, "M096 - Creation of instance-frag OID failed %d"),
                            status);
                    CRASH( msg );
                    }
                break;

            /* ============================================================= */
            case ASN1_C_PRINTABLE_STRING:
            case ASN1_C_OCTET_STRING:
                /* Tip 'o the Hat to Miriam Nihart here        */
                /* if (attempt to create inst oid from octet string FAILED) */
                if ((status = man_snmp_octet_to_oid(os,
                                                    &inst_frag,
                                                    MIR_inst->AVLtag
                                                    )) != MAN_C_SUCCESS) {
                    sprintf(msg,
                            MSG(msg097, "M097 - Creation of instance-frag OID failed %d"),
                            status);
                    CRASH( msg );
                    }
                break;

            /* ============================================================= */
            case INET_C_SMI_IP_ADDRESS:
                {
                /* construct printable string for INET address */
                /* Tip 'o the Hat to Mary Walker here          */
                char *ip_addr ;
                struct in_addr *in_p ;

                in_p = ( struct in_addr * )os->string ;
                ip_addr = inet_ntoa( *in_p ) ;

                /* if (conversion of string to oid fails) */
                if ((status = moss_text_to_oid(ip_addr, &inst_frag))
                    != MAN_C_SUCCESS) {
                    sprintf(msg,
                            MSG(msg098, "M098 - Creation of instance-frag OID failed %d"),
                            status);
                    CRASH( msg );
                    }
                }
                break;

            case ASN1_C_NULL:
                /* NOTE: ASN1_C_NULL are just SIGNED Integers! */

                /* verify structure of valid instance info.  Octet MUST have 
                 | a length = size of an int, and the string value must be 0.
                 */
                if ( (os->length != sizeof(int) ) || 
                     (os->string == NULL) ||
                     ( *(int *)(os->string) != (int) 0) )
		{
                    sprintf(msg,
                            MSG(msg299, "M299 - Validation of octet string failed %d"),
                            status);
                    CRASH( msg );
		}

                /* if (attempt to create an instance-fragment oid with one
                |       arc failed)
                */
                if ((status = moss_create_oid(1,
                                              (unsigned int  *)os->string,
                                              &inst_frag)) != MAN_C_SUCCESS) {
                    sprintf(msg,
                            MSG(msg096, "M096 - Creation of instance-frag OID failed %d"),
                            status);
                    CRASH( msg );
                    }
                break;


            /* ============================================================= */
            /* case ASN1_C_OBJECTID:                                         */
                /* NOTE: When implementing this, remember that Object ID     */
                /*       arcs are unsigned.                                  */

            default:
                {
                char    *class_oid_text;

                if (moss_oid_to_text(&vbe->class_oid,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &class_oid_text) != MAN_C_SUCCESS)
                    class_oid_text = "";

                sprintf(msg,
         MSG(msg099, "M099 - Unsupported SNMP Instance translation for Tag %d, Class (%s)"),
                        MIR_inst->AVLtag,
                        class_oid_text);
                if (strlen(class_oid_text) > 0) free(class_oid_text);

                SYSLOG( LOG_ERR, msg );

                /* Blow off the instance oid we were building */
                if (new_arcs != NULL) {
                    if ((status = moss_free_oid(new_arcs)) != MAN_C_SUCCESS) {
                        CRASH(MSG(msg100, "M100 - Free oid failed"));
                        }
                    }
                vbe->reply_error = genErr;   /* show "genErr" in reply_error */
                return;
                }
            }

        /*
        | At this point, we now have another bit of instance information
        | that arrived in an element of the instance AVL converted to OID
        | form in "inst_frag" (a fragment of the instance OID).  We want to
        | tack this fragment onto the grand instance OID (new_arcs) we're
        | building (that, at the finish, will contain all the instance arcs)
        | and reset things for the next potential pass through the loop.
        */

        /* if (attempt to concatenate inst-fragment onto inst oid failed) */
        if ((status = moss_concat_oids(new_arcs, inst_frag, &temp_oid))
            != MAN_C_SUCCESS) {
            CRASH(MSG(msg101, "M101 - Concatenation of instance oid fragment failed"));
            }

        /* if (attempt to release storage for old new-arcs failed) */
        if (   new_arcs != NULL
            && ((status = moss_free_oid(new_arcs)) != MAN_C_SUCCESS)) {
             CRASH(MSG(msg102, "M102 - Release of old new-arc oid failed"));
             }

        /* swap temporary oid and new-arcs oid pointers
        |  We blew off the storage associated with "new_arcs" above, the
        |  value of "temp_oid" becomes the new "new_arcs"
        */
        new_arcs = temp_oid;
        temp_oid = NULL;        /* just like we're starting over */

        /* if (attempt to release storage for inst-fragment oid failed) */
        if ((status = moss_free_oid(inst_frag)) != MAN_C_SUCCESS) {
             CRASH(MSG(msg103, "M103 - Release of old inst-frag oid failed"));
             }

        inst_frag = NULL;       /* show NULL for inst_frag oid pointer */

        }       /* if (os.length > 0) */

    }   /* End of Loop */

/* ============================================================================
| Sanity check, be sure we're at the end of the instance AVL
*/
tag = -1;       /* show tag as -1 */

/* save status from attempt to point to next element in instance AVL */
status = moss_avl_point(obj_instance, &oid, &modifier, &tag, &os, &last);

/* if (status is not valid or tag is not "end of construction") */
if ((status != MAN_C_SUCCESS)  ||  (tag != ASN1_C_EOC)) {
    char    *class_oid_text;

    if (moss_oid_to_text(oid, NULL, NULL, NULL, &class_oid_text)
        != MAN_C_SUCCESS) class_oid_text = "";

    sprintf(msg, MSG(msg104, "M104- Ill-formed AVL for object class (%s) Tag %d Status %d"),
            class_oid_text,
            tag,
            status);
    if (strlen(class_oid_text) > 0) free(class_oid_text);

    SYSLOG( LOG_ERR, msg );

    vbe->reply_error = genErr;   /* show "genErr" in reply_error */

    /* if (instance oid exists) */
    if (new_arcs != NULL) {
        /* if (attempt to free instance oid fails) */
        if (moss_free_oid(new_arcs) != MAN_C_SUCCESS) {
            CRASH(MSG(msg105, "M105 - Instance OID free attempt failed"));
            }
        }
   return;
   }

/*
| Now obtain the attribute oid and concatenate either the instance oid
| generated above or ".0" onto the end to form the full SNMP OID.
*/

/* ============================================================================
| Go for the attribute oid
|
| if (attempt to reset the attribute list AVL failed)
*/
if ((status = moss_avl_reset(attrib_list)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg106, "M106 - AVL reset on attribute list AVL failed %d"), status);
    CRASH( msg );
    }

/* if (point attempt on the attribute list AVL failed) */
status = moss_avl_point(attrib_list, &oid, &modifier, &tag, &os, &last);
if (status != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg107, "M107 - Point attempt on instance AVL failed %d"), status);
    CRASH(  msg );
    }

/* ============================================================================
| Come up with something for an instance oid section if we haven't already
| generated one above
|
| if (instance oid has a zero length)
*/
if (new_arcs == NULL) {

    /* if (attempt to create an oid with one arc of "0" failed) */
    if ((status = moss_text_to_oid("0", &new_arcs)) != MAN_C_SUCCESS) {
        CRASH(MSG(msg108, "M108 - oid creation attempt failed"));
        }
    }

/* ============================================================================
| This is it:  We finally reconstitute a full-bodied SNMP OID
|
|  if (attempt to concatenate instance oid onto attribute list oid failed)
*/
if ((status = moss_concat_oids(oid, new_arcs, &full_oid)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg109, "M109 - OID Concatenation failed %d"), status);
    CRASH(  msg );
    }

/* if (attempt to free instance oid fails) */
if (moss_free_oid(new_arcs) != MAN_C_SUCCESS) {
    CRASH(MSG(msg110, "M110 - Instance OID free attempt failed"));
    }

/* ============================================================================
| Now build the SNMP OID w/data into the "outbound" AVL in the varbind entry
| block for use by the sending thread when it builds the response PDU.
|
| if (attempt to init the "out_entry" varbind block AVL failed)
*/
if ((status = moss_avl_init(&(vbe->out_entry))) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg111, "M111 - Init attempt for 'out_entry' AVL failed %d"),
            status);
    CRASH( msg );
    }

/* if (attempt to add full oid, attribute modifier tag and data failed) */
if ((status = moss_avl_add(vbe->out_entry,
                           full_oid,
                           modifier,
                           tag,
                           os
                           )) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg112, "M112 - ADD attempt for 'out_entry' AVL failed %d"),
            status);
    CRASH( msg );
    }

/* if (attempt to free full oid failed) */
if (moss_free_oid(full_oid) != MAN_C_SUCCESS) {
    CRASH(MSG(msg113, "M113 - Instance OID free attempt failed"));
    }

/* set reply_error cell to "noError" to signal success to sending thread */
vbe->reply_error = noError;

return;
}

/* pei_send_get_reply - Receiving Thread Reply Entry Point from CA for "GET" */
/* pei_send_get_reply - Receiving Thread Reply Entry Point from CA for "GET" */
/* pei_send_get_reply - Receiving Thread Reply Entry Point from CA for "GET" */

int
pei_send_get_reply (  /*ARGSUSED*/  /* <-- Suppresses warns from CodeCtr */
                   pe_handle ,
                   invoke_identifier ,
                   reply ,
                   object_class ,
                   object_instance ,
                   object_uid ,
                   operation_time ,
                   attribute_list ,
                   more_replies
                   )
man_binding_handle
                pe_handle ;        /* binding handle                         */
int             invoke_identifier; /* the operation id for the mgmt operation*/
reply_type      reply;             /* type of reply data                     */
object_id       *object_class;     /* the object class id of the mo          */
avl             *object_instance;  /* the object instance name the mgmt      */
                                   /*  operation was performed on            */
uid             *object_uid;       /* uid of the instance                    */
mo_time         operation_time;    /* time operation was performed           */
avl             *attribute_list;   /* return data                            */
int             more_replies;      /* boolean indication more data or not    */
                                   /* (should always be FALSE for SNMP)      */
/*
INPUTS:

    The arguments are all taken to be exactly as described in the
    "Developer's Guide to Writing Managed Objects" for the
    "moss_send_get_reply()" argument.

    The only additional information particular to the SNMP PE is the format
    of the value of the "invoke_identifier".  The value of this field
    matches the value of the "vb_entry_id" cell in the varbind block
    that corresponds to this reply.  This value is formatted by function
    "netio_deserialize_pdu()" and also discussed in the documentation
    (in "snmppe.h") for the varbind entry declaration "varbind_t".

OUTPUTS:

    This function loads a status code (that is later examined by the sending
    thread) to indicate the status of the varbind entry that corresponds
    to this reply.  The value to be stored is determined by logic in this
    module.

    Also stored into the "out_entry" AVL in the varbind entry for this
    reply is the full SNMP OID (plus attribute tag, modifier and data for
    retrieved by the GET) in a single element in this AVL.

    When processing is FULLY COMPLETE, the receiving thread executing
    this code releases (potentially) the sending thread to examine the
    varbind entry block by returning!

    Summary:

       * Sets "reply_error" in varbind entry block
       * Loads one element into "out_entry" AVL
       * Increments "replies_rcvd"

BIRD'S EYE VIEW:
    Context:
        The caller is the Common Agent on behalf of a Managed Object Module.

    Purpose:
        This function handles the Common Agent reply by storing the
        returned information and reply status in the SNMP PE database
        for later examination by the sending thread.

ACTION SYNOPSIS OR PSEUDOCODE:

    <attempt to find the service block and varbind entry block for invoke-id>

    if (more replies is TRUE)
        <SYSLOG "Mxxx - More than one reply to pei_send_get_reply">

    <set local reply_error to "noError">

    switch (reply status code)

        (* SUCCESS *)
        case MAN_C_SUCCESS:
            (* leave the status as "noError" *)
            <break>

        (* LOG AND GENERR *)
        case MAN_C_CLASS_INSTANCE_CONFLICT:
        case MAN_C_COMPLEXITY_LIMITATION:
        case MAN_C_INSUFFICIENT_RESOURCES:
        case MAN_C_INVALID_FILTER:
        case MAN_C_INVALID_USE_OF_WILDCARD:
        default:
            <SYSLOG - "Mxxx - Improper reply code %d, object class (oid)">

        (* FALL THRU *)

        (* JUST GENERR *)
        case MAN_C_ACCESS_DENIED:
        case MAN_C_DIRECTIVE_NOT_SUPPORTED:
        case MAN_C_PROCESSING_FAILURE:
            <set local reply_error code to "genERR">
            <break>

        case MAN_C_NO_SUCH_OBJECT_INSTANCE:
            <set local reply_error code to "noSuch">
            <break>

        case MAN_C_GET_LIST_ERROR:
            if (reset of attribute list AVL failed)
                <CRASH "Mxxx - AVL reset on attribute list failed %d (oid)>

            if (point to first entry an attribute list AVL failed)
                <CRASH "Mxxx - AVL point on attribute list failed %d (oid)>

            if (modifier from AVL point is MAN_C_NO_SUCH_ATTRIBUTE_ID)
                <set local reply_error code to "noSuch">
            else
                <set local reply_error code to "genERR">
            break;

    if (service block and varbind entry block pointers are returned)

        if (local reply_error is "noError")
            <perform avl_to_oid translation>
        else
            <set varbind list entry reply_code to local value>
        <increment count of replies received for this PDU in service block>
        (* NOTE: Once this increment is performed, no code here may modify *)
        (*       either the service block nor the varbind entry block.     *)
        <set svc and vbe pointers to NULL>

    else
        <SYSLOG "Mxxx - Failed to find svc(%d) or vbe(%d) blocks for (oid)>

    <return>                                                                   

OTHER THINGS TO KNOW:

    The "replies_rcvd" cell need not be protected by a mutex because there
    is only one receiving thread and only the receiving thread changes
    the value of this cell once the sending thread starts issuing Common
    Agent requests.

    As noted in the code, once this cell is incremented, this receiving
    thread code must not attempt to modify either the service block nor
    the varbind entry block.

*/

{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
service_t       *svc;              /* Pointer for the Service Block      */
varbind_t       *vbe;              /* Pointer for Varbind Entry Block    */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
e_status_t      local_reply_error; /* Local storage for varbind blk code */
man_status      status;            /* General purpose status holder      */
object_id       *oid;              /* AVL oid                            */
unsigned int    modifier;          /* AVL modifier                       */
unsigned int    tag;               /* AVL tag                            */
octet_string    *os;               /* AVL octet_string                   */
int             last;              /* AVL "last" flag boolean            */


/* attempt to find the service block and varbind entry block for invoke-id
|
| We do this at the outset in order to load "bp" correctly.  The SYSLOG
| and CRASH macros require this value to be set.  We find out later whether
| or not we really found the varbind entry block associated with the
| invoke_identifier.
*/
carecv_find_vb_entry(&bp, &svc, &vbe, invoke_identifier);

/* if (more replies is TRUE) */
if (more_replies == TRUE) {
    SYSLOG(LOG_ERR,
         MSG(msg114, "M114 - More than 1 reply to pei_send_get_reply, timeout may occur"));
    }

/* set local reply_error to "noError" */
local_reply_error = noError;

/* ============================================================================
| Handle all the different reply code values first.
*/
switch (reply) {     /* Common Agent reply status code */

    /* SUCCESS */
    case MAN_C_SUCCESS:
        /* leave the status as "noError" */
        break;

    /* LOG AND GENERR */
    case MAN_C_CLASS_INSTANCE_CONFLICT:
    case MAN_C_COMPLEXITY_LIMITATION:
    case MAN_C_INSUFFICIENT_RESOURCES:
    case MAN_C_INVALID_FILTER:
    case MAN_C_INVALID_USE_OF_WILDCARD:
    default:
        {
        char xmsg[LINEBUFSIZE];         /* Extended error msg built here */
        sprintf(xmsg,
                MSG(msg115, "M115 - Improper GET reply code %d, object class (%%s)"),
                reply);
        BLD_OID_ERRMSG( xmsg, object_class );
        SYSLOG( LOG_ERR, msg );
        };
    /* =============== FALL THRU ============== */
    /* =============== FALL THRU ============== */
    /* =============== FALL THRU ============== */

    /* JUST GENERR */
    case MAN_C_ACCESS_DENIED:
    case MAN_C_DIRECTIVE_NOT_SUPPORTED:
    case MAN_C_PROCESSING_FAILURE:
        /* set local reply_error code to "genERR" */
        local_reply_error = genErr;
        break;

    case MAN_C_NO_SUCH_OBJECT_INSTANCE:
        /* set local reply_error code to "noSuch" */
        local_reply_error = noSuch;
        break;

    case MAN_C_GET_LIST_ERROR:
        /* if (reset of attribute list AVL failed) */
        if ((status = moss_avl_reset(attribute_list)) != MAN_C_SUCCESS) {
            char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
            sprintf(xmsg,
                    MSG(msg116, "M116 - attrlist AVL reset failed: %d, object class (%%s)"),
                    status);
            BLD_OID_ERRMSG( xmsg, object_class );
            CRASH( msg );
            };

        /* if (point to first entry an attribute list AVL failed) */
        if ((status = moss_avl_point(attribute_list,
                                     &oid,
                                     &modifier,
                                     &tag,
                                     &os,
                                     &last)) != MAN_C_SUCCESS) {
            char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
            sprintf(xmsg,
                MSG(msg117, "M117 - AVL point on attr-list failed: %d, object class (%%s)"),
                    status);
            BLD_OID_ERRMSG( xmsg, object_class );
            CRASH( msg );
            };

        /* if (modifier from AVL point is MAN_C_NO_SUCH_ATTRIBUTE_ID) */
        if (modifier == MAN_C_NO_SUCH_ATTRIBUTE_ID)
            /* set local reply_error code to "noSuch" */
            local_reply_error = noSuch;
        else
            /* set local reply_error code to "genERR" */
            local_reply_error = genErr;
        break;
    }

/* ============================================================================
| if (service block and varbind entry block pointers are returned)
|
| . . in other words, if the invoke_identifier specified a valid
|     varbind entry block corresponding to this reply. . .
*/
if (svc != NULL && vbe != NULL) {

    /* if (local reply_error is "noError")
    |
    | if everything up to here seemed OK, then perform conversions on
    | the object instance and attribute list AVLs (along with class info)
    | to create an AVL in the "out_entry" slot in the varbind entry block
    | for return to the SNMP manager.
    */
    if (local_reply_error == noError) {
        /* perform avl_to_oid translation, store answers @ vbe 
        |
        | This function sets vbe->reply_error according to how processing
        | in the function goes.
        */
        carecv_avl_to_oid(bp, vbe, object_instance, attribute_list);
        }
    else { /* . . . there was a screw-up discovered, bag out */
        /* set varbind list entry reply_code to local value */
        vbe->reply_error = local_reply_error;
        }

    /* increment count of replies received for this PDU in service block
    |
    |  NOTE: Once this increment is performed, no code here may modify
    |        either the service block nor the varbind entry block.
    */
    svc->replies_rcvd += 1;

    /* set svc and vbe pointers to NULL */
    svc = NULL;  /* (Coding precaution against maintenance changes) */
    vbe = NULL;
    }

else {  /* Report the inconsistency: Couldn't find service/varbind blocks */
    char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
    sprintf(xmsg,
            MSG(msg118, "M118 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)"),
            svc, vbe);
    BLD_OID_ERRMSG( xmsg, object_class );
    SYSLOG( LOG_ERR, msg );
    }

return(MAN_C_SUCCESS);
}

/* pei_send_set_reply - Receiving Thread Reply Entry Point from CA for "SET" */
/* pei_send_set_reply - Receiving Thread Reply Entry Point from CA for "SET" */
/* pei_send_set_reply - Receiving Thread Reply Entry Point from CA for "SET" */

int
pei_send_set_reply (  /*ARGSUSED*/  /* <-- Suppresses warns from CodeCtr */
                   pe_handle ,
                   invoke_identifier ,
                   reply ,
                   object_class ,
                   object_instance ,
                   object_uid ,
                   operation_time ,
                   attribute_list ,
                   more_replies
                   )
man_binding_handle
                pe_handle ;        /* binding handle                         */
int             invoke_identifier; /* the operation id for the mgmt operation*/
reply_type      reply;             /* type of reply data                     */
object_id       *object_class;     /* the object class id of the mo          */
avl             *object_instance;  /* the object instance name the mgmt      */
                                   /*  operation was performed on            */
uid             *object_uid;       /* uid of the instance                    */
mo_time         operation_time;    /* time operation was performed           */
avl             *attribute_list;   /* return data                            */
int             more_replies;      /* boolean indication more data or not    */
                                   /* (should always be FALSE for SNMP)      */
/*
INPUTS:

    The arguments are all taken to be exactly as described in the
    "Developer's Guide to Writing Managed Objects" for the
    "moss_send_set_reply()" argument.

    The only additional information particular to the SNMP PE is the format
    of the value of the "invoke_identifier".  The value of this field
    matches the value of the "vb_entry_id" cell in the varbind block
    that corresponds to this reply.  This value is formatted by function
    "netio_deserialize_pdu()" and also discussed in the documentation
    (in "snmppe.h") for the varbind entry declaration "varbind_t".

OUTPUTS:

    This function loads a status code (that is later examined by the sending
    thread) to indicate the status of the varbind entry that corresponds
    to this reply.  The value to be stored is determined by logic in this
    module.

    When processing is FULLY COMPLETE, the receiving thread executing
    this code releases (potentially) the sending thread to examine the
    varbind entry block by returning!

    Summary:

       * Sets "reply_error" in varbind entry block
       * Loads one element into "out_entry" AVL
       * Increments "replies_rcvd"

BIRD'S EYE VIEW:
    Context:
        The caller is the Common Agent on behalf of a Managed Object Module.

    Purpose:
        This function handles the Common Agent reply by storing the
        returned information and reply status in the SNMP PE database
        for later examination by the sending thread.

ACTION SYNOPSIS OR PSEUDOCODE:

    <attempt to find the service block and varbind entry block for invoke-id>

    if (more replies is TRUE)
        <SYSLOG "Mxxx - More than one reply to pei_send_get_reply">

    <set local reply_error to "noError">

    switch (reply status code)

        (* SUCCESS *)
        case MAN_C_SUCCESS:
            (* leave the status as "noError" *)
            <break>

        (* SNMP Bad Value *)
        case MAN_C_CONSTRAINT_VIOLATION:
            <set local reply_error code to "badValue">
            <break>

        (* LOG AND GENERR *)
        case MAN_C_CLASS_INSTANCE_CONFLICT:
        case MAN_C_COMPLEXITY_LIMITATION:
        case MAN_C_INSUFFICIENT_RESOURCES:
        case MAN_C_INVALID_FILTER:
        case MAN_C_INVALID_USE_OF_WILDCARD:
        default:
            <SYSLOG - "Mxxx - Improper reply code %d, object class (oid)">

        (* FALL THRU *)

        (* JUST GENERR *)
        case MAN_C_ACCESS_DENIED:
        case MAN_C_DIRECTIVE_NOT_SUPPORTED:
        case MAN_C_PROCESSING_FAILURE:
            <set local reply_error code to "genERR">
            <break>

        case MAN_C_NO_SUCH_OBJECT_INSTANCE:
            <set local reply_error code to "noSuch">
            <break>

        case MAN_C_SET_LIST_ERROR:
            if (reset of attribute list AVL failed)
                <CRASH "Mxxx - AVL reset on attribute list failed %d (oid)>

            if (point to first entry an attribute list AVL failed)
                <CRASH "Mxxx - AVL point on attribute list failed %d (oid)>

            if (modifier from AVL point is MAN_C_NO_SUCH_ATTRIBUTE_ID or
                                           MAN_C_NO_SUCH_ATTRIBUTE_GROUP)
                <set local reply_error code to "noSuch">

            else if (modifier from AVL point is MAN_C_READ_ONLY_ATTRIBUTE)
                <set local reply_error code to "readOnly">
            else if (modifier from AVL point is MAN_C_INVALID_ATTRIBUTE_VALUE)
                <set local reply_error code to "badValue">
            else
                <set local reply_error code to "genERR">
            break;

    if (service block and varbind entry block pointers are returned)

        if (local reply_error is "noError")
            <perform avl_to_oid translation>
        else
            <set varbind list entry reply_code to local value>
        <increment count of replies received for this PDU in service block>
        (* NOTE: Once this increment is performed, no code here may modify *)
        (*       either the service block nor the varbind entry block.     *)
        <set svc and vbe pointers to NULL>

    else
        <SYSLOG "Mxxx - Failed to find svc(%d) or vbe(%d) blocks for (oid)>

    <return>                                                                   

OTHER THINGS TO KNOW:

    The "replies_rcvd" cell need not be protected by a mutex because there
    is only one receiving thread and only the receiving thread changes
    the value of this cell once the sending thread starts issuing Common
    Agent requests.

    As noted in the code, once this cell is incremented, this receiving
    thread code must not attempt to modify either the service block nor
    the varbind entry block.

*/

{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
service_t       *svc;              /* Pointer for the Service Block      */
varbind_t       *vbe;              /* Pointer for Varbind Entry Block    */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
e_status_t      local_reply_error; /* Local storage for varbind blk code */
man_status      status;            /* General purpose status holder      */
object_id       *oid;              /* AVL oid                            */
unsigned int    modifier;          /* AVL modifier                       */
unsigned int    tag;               /* AVL tag                            */
octet_string    *os;               /* AVL octet_string                   */
int             last;              /* AVL "last" flag boolean            */


/* attempt to find the service block and varbind entry block for invoke-id
|
| We do this at the outset in order to load "bp" correctly.  The SYSLOG
| and CRASH macros require this value to be set.  We find out later whether
| or not we really found the varbind entry block associated with the
| invoke_identifier.
*/
carecv_find_vb_entry(&bp, &svc, &vbe, invoke_identifier);

/* if (more replies is TRUE) */
if (more_replies == TRUE) {
    SYSLOG(LOG_ERR,
         MSG(msg207, "M207 - More than 1 reply to pei_send_get_reply, timeout may occur"));
    }

/* set local reply_error to "noError" */
local_reply_error = noError;

/* ============================================================================
| Handle all the different reply code values first.
*/
switch (reply) {     /* Common Agent reply status code */

    /* SUCCESS */
    case MAN_C_SUCCESS:
        /* leave the status as "noError" */
        break;
        
    /* LOG AND GENERR */
    case MAN_C_CLASS_INSTANCE_CONFLICT:
    case MAN_C_COMPLEXITY_LIMITATION:
    case MAN_C_CONSTRAINT_VIOLATION:
    case MAN_C_INSUFFICIENT_RESOURCES:
    case MAN_C_INVALID_FILTER:
    case MAN_C_INVALID_USE_OF_WILDCARD:
    default:
        {
        char xmsg[LINEBUFSIZE];         /* Extended error msg built here */
        sprintf(xmsg,
                MSG(msg208, "M208 - Improper SET reply code %d, object class (%%s)"),
                reply);
        BLD_OID_ERRMSG( xmsg, object_class );
        SYSLOG( LOG_ERR, msg );
        };
    /* =============== FALL THRU ============== */
    /* =============== FALL THRU ============== */
    /* =============== FALL THRU ============== */

    /* JUST GENERR */
    case MAN_C_ACCESS_DENIED:
    case MAN_C_DIRECTIVE_NOT_SUPPORTED:
    case MAN_C_PROCESSING_FAILURE:
        /* set local reply_error code to "genERR" */
        local_reply_error = genErr;
        break;

    case MAN_C_NO_SUCH_OBJECT_INSTANCE:
        /* set local reply_error code to "noSuch" */
        local_reply_error = noSuch;
        break;

    case MAN_C_SET_LIST_ERROR:
        /* if (reset of attribute list AVL failed) */
        if ((status = moss_avl_reset(attribute_list)) != MAN_C_SUCCESS) {
            char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
            sprintf(xmsg,
                    MSG(msg209, "M209 - attrlist AVL reset failed: %d, object class (%%s)"),
                    status);
            BLD_OID_ERRMSG( xmsg, object_class );
            CRASH( msg );
            };

        /* if (point to first entry an attribute list AVL failed) */
        if ((status = moss_avl_point(attribute_list,
                                     &oid,
                                     &modifier,
                                     &tag,
                                     &os,
                                     &last)) != MAN_C_SUCCESS) {
            char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
            sprintf(xmsg,
                MSG(msg210, "M210 - AVL point on attr-list failed: %d, object class (%%s)"),
                    status);
            BLD_OID_ERRMSG( xmsg, object_class );
            CRASH( msg );
            };

        /* if (modifier from AVL point is MAN_C_NO_SUCH_ATTRIBUTE_ID) */
        if (   modifier == MAN_C_NO_SUCH_ATTRIBUTE_ID
            || modifier == MAN_C_NO_SUCH_ATTRIBUTE_GROUP) {
            /* set local reply_error code to "noSuch" */
            local_reply_error = noSuch;
            }

        else if (modifier == MAN_C_READ_ONLY_ATTRIBUTE) {
            /* set local reply_error code to "genErr" (readOnly NOT permitted)*/
            local_reply_error = noSuch; /* see item (1), pg 26, RFC 1157 */
            }

        else if (modifier == MAN_C_INVALID_ATTRIBUTE_VALUE) {
            /* set local reply_error code to "badValue" */
            local_reply_error = badValue;
            }

        else
            /* set local reply_error code to "genErr" */
            local_reply_error = genErr;
        break;
    }

/* ============================================================================
| if (service block and varbind entry block pointers are returned)
|
| . . in other words, if the invoke_identifier specified a valid
|     varbind entry block corresponding to this reply. . .
*/
if (svc != NULL && vbe != NULL) {

    /* if (local reply_error is "noError")
    |
    | if everything up to here seemed OK, then perform conversions on
    | the object instance and attribute list AVLs (along with class info)
    | to create an AVL in the "out_entry" slot in the varbind entry block
    | for return to the SNMP manager.
    */
    if (local_reply_error == noError) {
        /* perform avl_to_oid translation, store answers @ vbe 
        |
        | This function sets vbe->reply_error according to how processing
        | in the function goes.
        */
        carecv_avl_to_oid(bp, vbe, object_instance, attribute_list);
        }
    else { /* . . . there was a screw-up discovered, bag out */
        /* set varbind list entry reply_code to local value */
        vbe->reply_error = local_reply_error;
        }

    /* increment count of replies received for this PDU in service block
    |
    |  NOTE: Once this increment is performed, no code here may modify
    |        either the service block nor the varbind entry block.
    */
    svc->replies_rcvd += 1;

    /* set svc and vbe pointers to NULL */
    svc = NULL;  /* (Coding precaution against maintenance changes) */
    vbe = NULL;
    }

else {  /* Report the inconsistency: Couldn't find service/varbind blocks */
    char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
    sprintf(xmsg,
            MSG(msg211, "M211 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)"),
            svc, vbe);
    BLD_OID_ERRMSG( xmsg, object_class );
    SYSLOG( LOG_ERR, msg );
    }

return(MAN_C_SUCCESS);
}

/* pei_send_action_reply - Rcv-Thread Reply Entry Point from CA "GET-NEXT" */
/* pei_send_action_reply - Rcv-Thread Reply Entry Point from CA "GET-NEXT" */
/* pei_send_action_reply - Rcv-Thread Reply Entry Point from CA "GET-NEXT" */

int
pei_send_action_reply(  /*ARGSUSED*/  /* <-- Suppresses warns from CodeCtr */
                      pe_handle ,
                      invoke_identifier ,
                      reply ,
                      object_class ,
                      object_instance ,
                      object_uid ,
                      operation_time ,
                      action_type,
                      action_response_type,
                      attribute_list ,
                      more_replies
                     )
man_binding_handle
                pe_handle ;        /* binding handle                         */
int             invoke_identifier; /* the operation id for the mgmt operation*/
reply_type      reply;             /* type of reply data                     */
object_id       *object_class;     /* the object class id of the mo          */
avl             *object_instance;  /* the object instance name the mgmt      */
                                   /*  operation was performed on            */
uid             *object_uid;       /* uid of the instance                    */
mo_time         operation_time;    /* time operation was performed           */
object_id       *action_type;
object_id       *action_response_type;
avl             *attribute_list;   /* return data                            */
int             more_replies;      /* boolean indication more data or not    */
                                   /* (should always be FALSE for SNMP)      */

/*
INPUTS:

    The arguments are all taken to be exactly as described in the
    "Developer's Guide to Writing Managed Objects" for the
    "moss_send_action_reply()" argument.

    The only additional information particular to the SNMP PE is the format
    of the value of the "invoke_identifier".  The value of this field
    matches the value of the "vb_entry_id" cell in the varbind block
    that corresponds to this reply.  This value is formatted by function
    "netio_deserialize_pdu()" and also discussed in the documentation
    (in "snmppe.h") for the varbind entry declaration "varbind_t".

OUTPUTS:

    This function loads a status code (that is later examined by the sending
    thread) to indicate the status of the varbind entry that corresponds
    to this reply.  The value to be stored is determined by logic in this
    module.

    Also stored into the "out_entry" AVL in the varbind entry for this
    reply is the full SNMP OID (plus attribute tag, modifier and data for
    retrieved by the GET) in a single element in this AVL.

    When processing is FULLY COMPLETE, the receiving thread executing
    this code releases (potentially) the sending thread to examine the
    varbind entry block by returning!

    Summary:

       * Sets "reply_error" in varbind entry block
       * Loads one element into "out_entry" AVL
       * Increments "replies_rcvd"

BIRD'S EYE VIEW:
    Context:
        The caller is the Common Agent on behalf of a Managed Object Module.

    Purpose:
        This function handles the Common Agent reply by storing the
        returned information and reply status in the SNMP PE database
        for later examination by the sending thread.

ACTION SYNOPSIS OR PSEUDOCODE:

    <attempt to find the service block and varbind entry block for invoke-id>

    if (more replies is TRUE)
        <SYSLOG "Mxxx - More than one reply to pei_send_get_reply">

    <set local reply_error to "noError">

    switch (reply status code)

        (* SUCCESS *)
        case MAN_C_SUCCESS:
            (* leave the status as "noError" *)
            <break>

        (* LOG AND ROLL TO NEXT ATTRIBUTE *)
        case MAN_C_COMPLEXITY_LIMITATION:
        case MAN_C_DUPLICATE_ARGUMENT:
        case MAN_C_INVALID_ARGUMENT_VALUE:
        case MAN_C_INVALID_FILTER:
        case MAN_C_INVALID_USE_OF_WILDCARD:
        case MAN_C_NO_SUCH_ARGUMENT:
        case MAN_C_PROCESSING_FAILURE:
        case MAN_C_REQUIRED_ARGUMENT_OMITTED:
        default:
            <SYSLOG - "Mxxx - Improper reply code %d, object class (oid)">

        (* FALL THRU *)

        (* JUST ROLL TO NEXT ATTRIBUTE *)
        case MAN_C_ACCESS_DENIED:
        case MAN_C_CLASS_INSTANCE_CONFLICT:
        case MAN_C_DIRECTIVE_NOT_SUPPORTED:
        case MAN_C_INSUFFICIENT_RESOURCES:
        case MAN_C_NO_SUCH_ACTION:
        case MAN_C_NO_SUCH_OBJECT_INSTANCE:
            <set local reply_error code to "rollattrib">
            <break>

    if (service block and varbind entry block pointers are returned)

        if (local reply_error is "noError")
            <perform avl_to_oid translation>
        else
            <set varbind list entry reply_code to local value>
        <increment count of replies received for this PDU in service block>
        (* NOTE: Once this increment is performed, no code here may modify *)
        (*       either the service block nor the varbind entry block.     *)
        <set svc and vbe pointers to NULL>

    else
        <SYSLOG "Mxxx - Failed to find svc(%d) or vbe(%d) blocks for (oid)>

    <return>                                                                   

OTHER THINGS TO KNOW:

    We don't expect to get "MAN_C_NO_SUCH_CLASS" returned via this entry
    point, because we expect the original "msi_*" call to be rejected when
    the Common Agent asks MOLD (on our behalf) to find the MOM to which the
    request should be routed.

    The "replies_rcvd" cell need not be protected by a mutex because there
    is only one receiving thread and only the receiving thread changes
    the value of this cell once the sending thread starts issuing Common
    Agent requests.

    As noted in the code, once this cell is incremented, this receiving
    thread code must not attempt to modify either the service block nor
    the varbind entry block.

*/

#define LOGIT() fprintf(bp->log_state.log_file, buf)
{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
service_t       *svc;              /* Pointer for the Service Block      */
varbind_t       *vbe;              /* Pointer for Varbind Entry Block    */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
e_status_t      local_reply_error; /* Local storage for varbind blk code */
char            buf[LINEBUFSIZE];  /* Trace Message build buffer         */
int             ss;                /* "Start String" index for Trace messages */
extern char *e_status_string[];

/* attempt to find the service block and varbind entry block for invoke-id
|
| We do this at the outset in order to load "bp" correctly.  The SYSLOG
| and CRASH macros require this value to be set.  We find out later whether
| or not we really found the varbind entry block associated with the
| invoke_identifier.
*/
carecv_find_vb_entry(&bp, &svc, &vbe, invoke_identifier);

/* if (more replies is TRUE) */
if (more_replies == TRUE) {
    SYSLOG(LOG_ERR, MSG(msg193, "M193 - More than one reply to pei_send_get_reply"));
    }

/* set local reply_error to "noError" */
local_reply_error = noError;

IFLOGGING( L_TRACE ) {
    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "'pei_send_action_reply' entered with status = %d\n",
            reply);
    LOGIT();
    }

/* ============================================================================
| Handle all the different reply code values first.
*/
switch (reply) {     /* Common Agent reply status code */

    /* SUCCESS */
    case MAN_C_SUCCESS:
        break;       /* leave the status as "noError" */

    /* LOG AND ROLL TO NEXT ATTRIBUTE */
    case MAN_C_COMPLEXITY_LIMITATION:
    case MAN_C_DUPLICATE_ARGUMENT:
    case MAN_C_INVALID_ARGUMENT_VALUE:
    case MAN_C_INVALID_FILTER:
    case MAN_C_INVALID_USE_OF_WILDCARD:
    case MAN_C_NO_SUCH_ARGUMENT:
    case MAN_C_PROCESSING_FAILURE:
    case MAN_C_REQUIRED_ARGUMENT_OMITTED:
    default:
        {
        char xmsg[LINEBUFSIZE];         /* Extended error msg built here */
        sprintf(xmsg,
           MSG(msg194, "M194 - Improper GET-NEXT reply code (%d)=(%s), object class (%%s)"),
                reply, get_man_status(reply));
        BLD_OID_ERRMSG( xmsg, object_class );
        SYSLOG( LOG_ERR, msg );
        };
    /* =============== FALL THRU TO ROLL ============== */
    /* =============== FALL THRU TO ROLL ============== */
    /* =============== FALL THRU TO ROLL ============== */

    /* JUST ROLL TO NEXT ATTRIBUTE */
    case MAN_C_ACCESS_DENIED:
    case MAN_C_CLASS_INSTANCE_CONFLICT:
    case MAN_C_DIRECTIVE_NOT_SUPPORTED:
    case MAN_C_INSUFFICIENT_RESOURCES:
    case MAN_C_NO_SUCH_ACTION:
    case MAN_C_NO_SUCH_OBJECT_INSTANCE:
        /* set local reply_error code to "rollattrib" */
        local_reply_error = rollattrib;
        break;

    }

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "local_reply_error status set to '%s'\n",
             e_status_string[local_reply_error]);
    LOGIT();
    }

/* ============================================================================
| if (service block and varbind entry block pointers are returned)
|
| . . in other words, if the invoke_identifier specified a valid
| varbind entry block corresponding to this reply. . .
*/
if (svc != NULL && vbe != NULL) {

    /* if (local reply_error is "noError")
    |
    | if everything up to here seemed OK, then perform conversions on
    | the object instance and attribute list AVLs (along with class info)
    | to create an AVL in the "out_entry" slot in the varbind entry block
    | for return to the SNMP manager.
    */
    if (local_reply_error == noError) {

        IFLOGGING( L_TRACE ) {
            sprintf(&buf[ss],
                    "Performing AVL to OID translation in pei function\n");
            LOGIT();
            }

        /* perform avl_to_oid translation, store answers @ vbe 
        |
        | This function sets vbe->reply_error according to how processing
        | in the function goes.
        */
        carecv_avl_to_oid(bp, vbe, object_instance, attribute_list);
        }
    else { /* . . . there was a screw-up discovered, bag out */
        /* set varbind list entry reply_code to local value */
        vbe->reply_error = local_reply_error;
        }

    /* increment count of replies received for this PDU in service block
    |
    |  NOTE: Once this increment is performed, no code here may modify
    |        either the service block nor the varbind entry block.
    */
    svc->replies_rcvd += 1;

    /* set svc and vbe pointers to NULL */
    svc = NULL;  /* (Coding precaution against maintenance changes) */
    vbe = NULL;
    }

else {  /* Report the inconsistency: Couldn't find service/varbind blocks */
    char xmsg[LINEBUFSIZE];   /* Extended error msg built here */
    sprintf(xmsg,
            MSG(msg195, "M195 - Failed to find svc(%d) or vbe(%d) blocks for (%%s)"),
            svc, vbe);
    BLD_OID_ERRMSG( xmsg, object_class );
    SYSLOG( LOG_ERR, msg );
    }

return(MAN_C_SUCCESS);
}

/* pei_send_create_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */
/* pei_send_create_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */
/* pei_send_create_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */

int
pei_send_create_reply(  /*ARGSUSED*/  /* <-- Suppresses warns from CodeCtr */
                      pe_handle ,
                      invoke_identifier ,
                      reply ,
                      object_class ,
                      object_instance ,
                      object_uid ,
                      operation_time ,
                      attribute_list
                     )
man_binding_handle
                pe_handle ;        /* binding handle                         */
int             invoke_identifier; /* the operation id for the mgmt operation*/
reply_type      reply;             /* type of reply data                     */
object_id       *object_class;     /* the object class id of the mo          */
avl             *object_instance;  /* the object instance name the mgmt      */
                                   /*  operation was performed on            */
uid             *object_uid;       /* uid of the instance                    */
mo_time         operation_time;    /* time operation was performed           */
avl             *attribute_list;   /* return data                            */
/*
INPUTS:

    The arguments are all taken to be exactly as described in the
    "Developer's Guide to Writing Managed Objects".

OUTPUTS:

    This function always logs an error message and simply returns because
    it should never be entered for SNMP.

BIRD'S EYE VIEW:
    Context:
        The caller is the Common Agent on behalf of a very confused
        Managed Object Module.
        
    Purpose:
        This function is simply a stub, as the logical corresponding to
        this routine is not supported by SNMP.

ACTION SYNOPSIS OR PSEUDOCODE:

    <grab the big picture pointer so we can issue a SYSLOG macro>
    <build the object-class oid into the message>
    <SYSLOG "Mxxx - Improper Entry at reply entry point 'Create' for (oid)">
    <return>

OTHER THINGS TO KNOW:

   
*/

{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
extern big_picture_t *big_picture;


/* grab the big picture pointer so we can issue a SYSLOG macro 
|
| "big_picture" is globally accessible cell in module "snmppe_main.c"
| that has been initialized by the sending thread.
*/
bp = big_picture;

/* build the object-class oid into the message */
BLD_OID_ERRMSG(
               MSG(msg119, "M119 - Improper Entry at reply entry point 'Create' for (%s)"),
               object_class
               );

SYSLOG( LOG_ERR, msg );

} /* end of pei_send_create_reply() */

/* pei_send_delete_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */
/* pei_send_delete_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */
/* pei_send_delete_reply - Rcv Thread Reply Entry Point from CA for "CREATE" */

int
pei_send_delete_reply(  /*ARGSUSED*/  /* <-- Suppresses warns from CodeCtr */
                      pe_handle ,
                      invoke_identifier ,
                      reply ,
                      object_class ,
                      object_instance ,
                      object_uid ,
                      operation_time ,
                      attribute_list ,
                      more_replies
                     )
man_binding_handle
                pe_handle ;        /* binding handle                         */
int             invoke_identifier; /* the operation id for the mgmt operation*/
reply_type      reply;             /* type of reply data                     */
object_id       *object_class;     /* the object class id of the mo          */
avl             *object_instance;  /* the object instance name the mgmt      */
                                   /*  operation was performed on            */
uid             *object_uid;       /* uid of the instance                    */
mo_time         operation_time;    /* time operation was performed           */
avl             *attribute_list;   /* return data                            */
int             more_replies;      /* boolean indication more data or not    */
                                   /* (should always be FALSE for SNMP)      */
/*
INPUTS:

    The arguments are all taken to be exactly as described in the
    "Developer's Guide to Writing Managed Objects".

OUTPUTS:

    This function always logs an error message and simply returns because
    it should never be entered for SNMP.

BIRD'S EYE VIEW:
    Context:
        The caller is the Common Agent on behalf of a very confused
        Managed Object Module.
        
    Purpose:
        This function is simply a stub, as the logical corresponding to
        this routine is not supported by SNMP.

ACTION SYNOPSIS OR PSEUDOCODE:

    <grab the big picture pointer so we can issue a SYSLOG macro>
    <build the object-class oid into the message>
    <SYSLOG "Mxxx - Improper Entry at reply entry point 'Delete' for (oid)">
    <return>

OTHER THINGS TO KNOW:

   
*/

{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
extern big_picture_t *big_picture;


/* grab the big picture pointer so we can issue a SYSLOG macro 
|
| "big_picture" is globally accessible cell in module "snmppe_main.c"
| that has been initialized by the sending thread.
*/
bp = big_picture;

/* build the object-class oid into the message */
BLD_OID_ERRMSG(MSG(msg120, "M120 - Improper Entry at reply entry point 'Delete' for (%s)"),
               object_class
               );

SYSLOG( LOG_ERR, msg );

} /* end of pei_send_delete_reply() */


static man_status
man_snmp_octet_to_oid(
                      data ,
                      partial_oid ,
                      tag
                     )

/*
 *
 * Function description:
 *
 *    This is the routine creates an oid from an octet string as
 *    outlined in the RFC 1212 section 4.1.6 (3) and Mark Sylor's
 *    white paper - "SNMP in EMA".  The octet is handled as a
 *    count and string.  The first "arc" in the oid is the count
 *    with each octet following as a single "arc".  For example,
 *    the octet string "abc" would be the oid 3.a.b.c, where the
 *    octet a, b, c are expanded out to an integer.
 *
 * Arguments:
 *
 *    data           the address of an octet string
 *    partial_oid    the address of a pointer to an oid
 *    tag            either ASN1_C_OCTET_STRING or ASN1_C_PRINTABLE_STRING
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Management operation received and processed successfully
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *
 * Side effects:
 *
 */

octet_string *data ;
object_id **partial_oid ;
int tag ;

{
    man_status return_status ;
    int *string ;
    int *tmp_str ;
    int i ;
    char *tmp_char_p = data->string ;
    char c ;
    octet_string tmp_string ;

    tmp_string.length = sizeof( int ) + ( data->length * sizeof( int ) ) ;
    string = ( int * )malloc( tmp_string.length ) ;
    if( string == NULL )
        return( MAN_C_INSUFFICIENT_RESOURCES ) ;

    memset( ( void * )string, '\0', tmp_string.length ) ;
    *string = data->length ;
    tmp_str = string + 1 ;
    for( i = 0 ; i < data->length ; i++ )
    {
        c = *tmp_char_p ;
        *tmp_str = ( int )c ;
        tmp_str ++ ;
        tmp_char_p ++ ;
    }

    tmp_string.string = ( char * )string ;
    tmp_string.data_type = tag ;

    return_status = moss_octet_to_oid( &tmp_string, partial_oid ) ;

    free( string ) ;

    return( return_status ) ;

} /* end of man_snmp_octet_to_oid() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**	evd_create_queue_handle - This routine is used by a manageable object
**	                          to obtain a handle to an existing named 
**                                event queue (For CA V1.1, this is NULL).
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
    extern big_picture_t      *big_picture;
    big_picture_t             *bp;


    /* Queue name is IGNORED for CA V1.1 */
/*
    if (queue_name != NULL)
        return (MAN_C_BAD_PARAMETER);
*/

    /* Queue access mode MUST be EVD_POST */
    if (access_mode != EVD_POST)
    {
        sprintf (msg, MSG(msg328, "M328 - Invalid Queue Access Mode (%d)"),
                 access_mode);
        SYSLOG (LOG_ERR, msg);
        return (MAN_C_BAD_PARAMETER);
    }

    /* set up the one and only SNMP_PE event queue handle */
    bp = big_picture;
    bp->event_queue_handle = (evd_queue_handle *) big_picture;
    *handle = bp->event_queue_handle;
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
**      It is assumed for Common Agent V1.1 that evd_delete_queue_handle()
**      is handled in lock-step to its completion, therefore no MUTEX
**      locking is performed on the queue handle list inside the global data.
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

    char                       msg[LINEBUFSIZE];  /* Message build buffer */
    extern big_picture_t      *big_picture;
    big_picture_t             *bp;


    /* init local "big picture" pointer */
    bp = big_picture;

    /* verify that queue handle is address of big_picture */
    if (*handle != (evd_queue_handle *) bp->event_queue_handle)
    {
        sprintf (msg, MSG(msg301, "M301 - Invalid Queue Handle (%lx)"),
                 (long int) *handle);
        SYSLOG (LOG_ERR, msg);
        return (MAN_C_BAD_HANDLE);
    }

    *handle = (evd_queue_handle *) NULL;
    return (MAN_C_SUCCESS);

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
**	    the event queue handle.  **** IGNORED for CA V1.1 ****
**
**	object_class
**	    The object class of the managed object that is posting the event.
**	    The object_class argument is address of the object ID which
**	    specifies the class; this is used as the default value for the
**          Enterprise OID (if not specified in event parameters avl) while 
**          parsing the event_parameters avl.
**
**	instance_name
**	    The instance name of the managed object that is posting the event.
**	    The instance_name argument is the address of the AVL which
**	    specifies the name. **** IGNORED for CA V1.1 ****
**
**	event_time
**	    The time the event occurred in BinABsTim format.  The event_time
**	    argument is the address of the mo_time.  The event_time argument is
**	    optional.  If the NULL pointer is passed, the current time will be
**	    used.  **** IGNORED for CA V1.1 ****
**
**	gen_event_type
**	    The generic event type identifier.  The event_type argument is the 
**          address of the object ID which specifies the "generic" SNMP trap 
**          type.
**
**	event_parameters
**	    The event parameters.  The event_parameters argument is the address
**	    of the AVL specifying the event parameters.
**
**	event_uid
**	    The uid assigned to the event by evd_post_event.  The event_uid
**	    argument is the address of a UID structure into which the event UID
**	    is written.  If the NULL pointer is passed, the event uid is not
**	    returned.
**
**	entity_uid
**	    The uid assigned to the managed object posting the event.  The
**	    MO_UID argument is the address of the UID structure specifying the
**	    uid of the managed object. **** IGNORED FOR CA V1.1 ****
**
**
**  SIDE EFFECTS:
**
**      No MUTEX locking is performed on any data; it is assumed for the V1.1
**      version of the Common Agent that each call into SNMP PE is processed in
**      lock-step to completion with no contention for resources.
**
**  DESIGN:
**
**      This routine must be called in user mode.
**
**  ERROR CODES:
**
**      MAN_C_SUCCESS - 
**          All went well.
**	MAN_C_PROCESSING_FAILURE -
**	    An unexpected error occurred.
**	MAN_C_BAD_HANDLE -
**	    The handle argument does not point to a valid handle.
**      MAN_C_BAD_PARAMETER - 
**          One of the API function call arguments is invalid.
**	MAN_C_INSUFFICIENT_RESOURCES -
**	    Unable to allocate process virtual memory.
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
    extern  big_picture_t *big_picture;
    big_picture_t         *bp;

    int               i;
    char              msg[LINEBUFSIZE]; /* Error Message build buffer  */
    man_status        status = MAN_C_SUCCESS, moss_status;

    unsigned int      agent_addr;
    MCC_T_Descriptor  agent_addr_desc;
    MCC_T_Descriptor  *agent_addr_descP;

    unsigned int      enterprise_oid_array[MAX_ENTERPRISE_OID_SIZE];
    object_id         enterprise_oid;

    char              varbind_buf[VAR_BIND_SIZE];
    MCC_T_Descriptor  varbind_info_desc;
    MCC_T_Descriptor  *varbind_info_descP;

    unsigned int      local_event_time;
    MCC_T_Descriptor  event_time_desc;
    MCC_T_Descriptor  *event_time_descP = NULL;

    int               local_event_type; /* should be same as specific trap   */
    int               specific_trap;    /*   (both are from event_parms avl) */

    int               generic_trap;     /* should be same as gen_event_type  */
                                        /*   (this is from event_parms avl)  */

    /* init local "big picture" pointer */
    bp = big_picture;

    /* verify that the queue handle is legit */
/***** ??? IGNORE THE HANDLE FOR NOW; JUST POST THE EVENT!  See CAU011 QAR #29.
    if (handle != bp->event_queue_handle)
    {
        sprintf (msg, MSG(msg302, "M302 - Invalid queue handle (%lx)"),
                 (long int) handle);
        SYSLOG (LOG_ERR, msg);
        return (MAN_C_BAD_HANDLE); (* OK to return here; no locks obtained *)
    }
*****/

    /*
     * Lock this handle - mostly because we want exclusive access to it.
     * (NOT FOR Common Agent V1.1)
     */
/*****
    CHECK_PTHREAD_STATUS( pthread_mutex_lock (&bp->event_lock_m));
*****/

    /* Validate the structure of each argument (all are optional    */
    /* except the event parameters AVL).                            */

    /* validate the structure of the object class (enterprise) OID */
    if ( (object_class == NULL) ||
         (object_class->count < 1) || 
         (object_class->count > MAX_ENTERPRISE_OID_SIZE) ||
         (object_class->value == NULL) )
    {
        sprintf (msg, MSG(msg303, "M303 - Invalid object class OID"));
        SYSLOG (LOG_ERR, msg);
        status = MAN_C_BAD_PARAMETER;
    }

    /* validate the structure of the instance_name avl */
/***** IGNORED FOR CA V1.1 ****
    else if (instance_name == NULL)
    {
        status = MAN_C_BAD_PARAMETER;
    }

    else if ( moss_avl_reset (instance_name) != MAN_C_SUCCESS)
    {
        status = MAN_C_BAD_PARAMETER;
    }
*****/

    /* validate the structure of the event time */
/***** IGNORED FOR CA V1.1 ****
    else if (event_time == NULL)
    {
        status = MAN_C_BAD_PARAMETER;
    }
*****/

    /* validate the structure and contents of the gen_event_type OID */
    else if ( (gen_event_type == NULL) ||
              (gen_event_type->count != 1) || (gen_event_type->value == NULL) ||
              (*gen_event_type->value < 0) || (*gen_event_type->value > 6) )
    {
        sprintf (msg, MSG(msg304, "M304 - Invalid event_type OID"));
        SYSLOG (LOG_ERR, msg);
        status = MAN_C_BAD_PARAMETER;
    }

    /* validate the structure of the event_parameters avl */
    else if (event_parameters == NULL)
    {
        sprintf (msg, MSG(msg305, "M305 - NULL event_parameters"));
        SYSLOG (LOG_ERR, msg);
        status = MAN_C_BAD_PARAMETER;
    }

    else if (moss_avl_reset (event_parameters) != MAN_C_SUCCESS)
    {
        sprintf (msg, 
               MSG(msg306, "M306 - moss_avl_reset on event_parameters failed"));
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
*****/

    /* extract event arguments from event_parameters avl; we use these    */
    /* arguments as the PRIMARY source of Enterprise OID, Agent Address,  */
    /* and Event Time.  If they are not found or specified in the         */
    /* event_parameters avl, we'll use the respective arguments specified */
    /* in the evd_post_event() call.  If they are not specified, we'll    */
    /* use the defaults.  For arguments where there are no defaults, if   */
    /* they are not specified, this is an ERROR.                          */

    if (status == MAN_C_SUCCESS)
    {
        /* initialize local copies of event arguments */

        /* use the "object_class" argument specified in the        */
        /* evd_post_event() call as the "default" Enterprise OID.  */
        enterprise_oid.count = object_class->count;
        enterprise_oid.value = &enterprise_oid_array[0];
        for (i = 0; i < object_class->count; i++)
            enterprise_oid_array[i] = object_class->value[i];

        local_event_time     = NO_EVENT_TIME;
        local_event_type     = NO_SPECIFIC_TRAP;
        specific_trap        = NO_SPECIFIC_TRAP;
        generic_trap         = NO_ENTERPRISE_TRAP; 
        agent_addr           = NO_AGENT_ADDRESS;

        /* construct a descriptor for the varbind list ASN.1 buffer to be */
        /* parsed into from the Event Parameters avl                      */
        varbind_info_desc.mcc_b_class     = DSC_K_CLASS_S;
        varbind_info_desc.mcc_b_dtype     = DSC_K_DTYPE_T;
        varbind_info_desc.mcc_b_flags     = 0;
        varbind_info_desc.mcc_b_ver       = 1;
        varbind_info_desc.mcc_w_maxstrlen = VAR_BIND_SIZE;
        varbind_info_desc.mcc_w_curlen    = 0;
        varbind_info_desc.mcc_a_pointer   = (unsigned char *) varbind_buf;
        varbind_info_desc.mcc_l_id        = 0;
        varbind_info_desc.mcc_l_dt        = 0;

        status = carecv_extract_event_parms (bp,
                                             event_parameters,
                                             &enterprise_oid,
                                             &local_event_time,
                                             &local_event_type,
                                             &agent_addr,
                                             &specific_trap,
                                             &generic_trap,
                                             &varbind_info_desc);
    }

    /* the Enterprise OID value MUST 'still' match the "object_class" value */
    if (status == MAN_C_SUCCESS)
    {
        moss_status = moss_compare_oid (object_class, &enterprise_oid);
        if (moss_status != MAN_C_EQUAL)
        {
            sprintf (msg, MSG(msg309, 
        "M309 - Object Class OID mismatch with event parm avl Enterprise oid"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }
    }

    /* the gen_event_type arg MUST match the "generic trap" value if obtained */
    if (status == MAN_C_SUCCESS)
    {
        if ( (generic_trap != NO_ENTERPRISE_TRAP) &&
             (*gen_event_type->value != generic_trap) )
        {
            sprintf (msg, MSG(msg325, 
"M325 - Generic Trap event_type mismatch with event parm avl generic_trap_num"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
        }
    }

    /* the specific_trap MUST match the "event" value (both obtained from  */
    /* the event parameters avl; BOTH are required according to the API)   */
    if ( (specific_trap == NO_SPECIFIC_TRAP) ||
         (local_event_type == NO_SPECIFIC_TRAP) ||
         (local_event_type != specific_trap))
    {
        sprintf (msg, 
       MSG(msg307, "M307 - No Specific trap number passed in event parms avl"));
        SYSLOG (LOG_ERR, msg);
        status = MAN_C_BAD_PARAMETER;
    }

    /* if any of the optional arguments were not specified in the   */
    /* event_parameters avl, try using the API arguments passed in, */
    /* OR (if not specified) let netio_send_trap() fill these in... */
    if (status == MAN_C_SUCCESS)
    {
        /* check Event Time */
        if (local_event_time == NO_EVENT_TIME)
        {
            /* try "event_time" API arg passed in evd_post_event() call */
            /* **** IGNORED FOR CA V1.1 **** */
        }
        else /* set up event_time descriptor fields (NOT FOR CA V1.1) */
	{
            /* **** IGNORED FOR CA V1.1 **** */
        }

        /* Check to be sure we have a valid generic_trap value if the        */
        /* generic trap value indicates the trap is NOT enterprise-specific  */
        if (generic_trap != enterpriseSpecific)
        {
            /* currently we will only accept LINK_UP and LINK_DOWN generic */
            /* trap events through the EVD mechanism                       */
            if ((generic_trap != linkUp) && (generic_trap != linkDown))
            {
                sprintf (msg, 
              MSG(msg308, "M308 - Unsupported MOM-generated Generic trap (%d)"),
                         generic_trap);
                SYSLOG (LOG_ERR, msg);
                status = MAN_C_BAD_PARAMETER;
            }

            if (specific_trap != 0)
            {
                sprintf (msg, MSG(msg326, 
              "M326 - Invalid Specific trap number (%d) for generic trap (%d)"),
                         specific_trap, generic_trap);
                SYSLOG (LOG_ERR, msg);
                status = MAN_C_BAD_PARAMETER;
	    }
        }
    }

    /* Common Agent V1.1: The SNMP Protocol Engine is the only PE currently */

    /* If all went well, ship the event to the SNMP trap listeners */
    /* if there are any out there according to snmp_pe.conf file   */

    if ((status == MAN_C_SUCCESS) && 
        (bp->trap_list != NULL) && (bp->snmp_traps_disabled != 1))
    {
        if (varbind_info_desc.mcc_w_curlen == 0)
            varbind_info_descP = NULL;
        else
            varbind_info_descP = &varbind_info_desc;

        /* We treat the IP address as an unsigned integer */
        if (agent_addr == 0)
        {
            agent_addr_descP = NULL;
        }
        else
        {
            agent_addr_desc.mcc_w_maxstrlen = 
            agent_addr_desc.mcc_w_curlen    = sizeof(unsigned int);
            agent_addr_desc.mcc_a_pointer   = (unsigned char *) &agent_addr;
            agent_addr_descP = &agent_addr_desc;
        }
        
        if (local_event_time == NO_EVENT_TIME)
            event_time_descP = NULL;
        else
            event_time_descP = &event_time_desc;

        /* send the trap */
        netio_send_trap (bp,
                         &enterprise_oid,
                         agent_addr_descP,
                         generic_trap,
                         specific_trap,
/* ??? IGNORED for CA V1.1; replaced with NULL ===>  event_time_descP, */
                         NULL,
                         varbind_info_descP);

        /* set event_uid if the event was processed OK */
        if (event_uid != NULL)
        {
            evd_set_event_uid (bp, event_uid);
        }
    }

/*
    CHECK_PTHREAD_STATUS( pthread_mutex_unlock (&bp->event_lock_m));
*/

    return (status);

}  /* end of evd_post_event */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**	carecv_extract_event_parms - Extract each event argument from the
**                                   event_parameters avl, and put the values
**                                   into the return arguments.
**
**  FORMAL PARAMETERS:
**
**      bp
**          The pointer to the Big Picture structure containing
**          all the "global" context information needed by SNMP PE to operate.
**
**	event_parameters
**	    The event parameters.  The event_parameters argument is the address
**	    of the AVL specifying the event parameters.
**
**	enterprise_oidP
**	    Address of where to put the Enterprise OID ("object class") of the 
**          managed object that is posting the event.  Extracted
**          from the event parameters avl.  MUST match the object class.
**
**	event_timeP
**	    Address of where to put the event time when the event was posted
**          from the event parameters avl (event_time argument is optional).
**
**	event_typeP
**	    Address of where to put the event type (specific trap number)
**          from the event parameters avl.  MUST match specific_trapP.
**
**      agent_addrP
**          Address of where to put the simple (integer) agent address from the
**          event parameters avl.
**
**	specific_trapP
**	    Address of where to put the specific trap number extracted
**          from the event parameters avl.
**
**	generic_trapP
**	    Address of where to put the generic trap number extracted
**          from the event parameters avl.
**
**	varbind_args_descP
**	    Address of descriptor where the varbind arguments extracted
**          from the event parameters avl will go (in ASN.1 format).
**
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
**          Event parameters were extracted successfully.
**      MAN_C_BAD_PARAMETER - 
**          A bad parameter was detected in the event parameters avl.
**      MAN_C_PROCESSING_FAILURE - 
**          Unable to successfully format one or more arguments from the
**          event paraameters avl.
**
**
**---------------------------------------------------------------------------*/

static man_status
carecv_extract_event_parms (bp,
                            event_parameters,
                            enterprise_oidP,
                            event_timeP,
                            event_typeP,
                            agent_addrP,
                            specific_trapP,
                            generic_trapP,
                            varbind_args_descP)
                            
 big_picture_t      *bp;
 avl                *event_parameters;   /* IN: AVL of ALL event parms */
 object_id          *enterprise_oidP;    /* OUT */
 unsigned int       *event_timeP;        /* OUT */
 int                *event_typeP;        /* OUT */
 unsigned int       *agent_addrP;        /* OUT */
 int                *specific_trapP;     /* OUT; MUST be same as event_typeP */
 int                *generic_trapP;      /* OUT */
 MCC_T_Descriptor   *varbind_args_descP; /* OUT */
{
    unsigned int   tag;
    unsigned int   modifier;
    octet_string  *os;
    object_id     *oid, new_oid;
    int            last_one, asn_status;
    man_status     status, moss_error;
    char           msg[LINEBUFSIZE];       /* Error message build buffer   */
    int            i;     
    int            got_eventtype_oid=0;
    int            got_enterprise_oid=0;
    int            got_eventtime=0;
    int            got_agentaddr=0;
    int            got_specifictrap_oid=0;
    int            got_generictrap_oid=0;
    int            varbindarg_count=0;
    int            start_of_real_oid;
    avl           *varbindarg_avl=NULL;
    BOOL           build_start, build_end;


    /* Event parameters AVL has already been reset OK when we arrive here;  */
    /* We have to walk through it, extracting and validating each trap      */
    /* argument;  those arguments that are optional will be defaulted.      */
    /* Any required argument that is mal-formed or invalid will result in   */
    /* NO trap being sent.  Many trap arguments from the event_parameters   */
    /* list are also specified in some of the arguments in the actual       */
    /* evd_post_event() API function call.  If we can't get them out of     */
    /* the event parameters avl, we will try to use those others before     */
    /* assuming the defaults.                                               */

    /* We will loop through the event_parameters AVL until we reach the end */
    do
    {
        /* get the next "thing" out of the event parameters avl */
        status = moss_avl_point (event_parameters, &oid, &modifier, &tag, &os,
                                 &last_one);

        /* break out of loop when there is nothing more to extract */
        if (status == MAN_C_NO_ELEMENT)
        {
            status = MAN_C_SUCCESS;
            break;
        }
        else if (status != MAN_C_SUCCESS)
        {
            sprintf (msg, MSG(msg324, 
                     "M324 - moss_avl_point FAILED on event parameters avl"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
            break;
        }

        /* once inside the AVL, there should NEVER be a SEQUENCE */
        if (tag == ASN1_C_SEQUENCE) 
        {
            sprintf (msg, 
                MSG(msg310, "M310 - SEQUENCE occured in event parameters avl"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
            break;
        }
        else if (tag == ASN1_C_EOC)
        {
            if (last_one != 1)
            {
                sprintf (msg, 
      MSG(msg311, "M311 - Mal-formed event parameters avl has more after EOC"));
                SYSLOG (LOG_ERR, msg);
                status = MAN_C_BAD_PARAMETER;
            }
            break;
        }

        /* if we get this far, there better be a valid OID */
        if (oid == NULL)
        {
            sprintf (msg, 
              MSG(msg312, "M312 - Mal-formed event parameters avl (NULL oid)"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
            break;
        }
        else if ( (oid->count == 0) || (oid->value == NULL) )
        {
            sprintf (msg, 
             MSG(msg313, "M313 - Mal-formed event parameters avl (Empty oid)"));
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_BAD_PARAMETER;
            break;
        }
                            
        /* if we get this far, the OID is well-formed; check against list of */
        /* well-known OIDs to determine which event argument this one is...  */
        /* (duplicate arguments (OIDs) in event_parameters avl are IGNORED)  */

        /* is it the Event Type OID? */
        if ( (moss_compare_partial_oid (oid, &prefix_event_eventtype_oid,
                                        0, 0, prefix_event_eventtype_oid.count)
                                        == MAN_C_EQUAL) &&
             (got_eventtype_oid == 0) )
        {
            /* store the value of the (specific) Event Type in return arg */
            if ( (os->length == sizeof(int)) && (os->string != NULL) )
            {
                /* make note of the fact that we got the Event Type OID */
                got_eventtype_oid = 1;
                *event_typeP = *(int *)(os->string);
            }
        }

        /* is it the Enterprise OID? */
        else if ( (moss_compare_partial_oid (oid, &prefix_event_enterprise_oid,
                                        0, 0, prefix_event_enterprise_oid.count)
                                        == MAN_C_EQUAL) &&
             (got_enterprise_oid == 0) )
        {
            /* store the value of the Enterprise OID in return arg */
            if ( (os->length > 0) && (os->length <= MAX_ENTERPRISE_OID_SIZE) &&
                 (os->string != NULL) )
            {
                /* make note of the fact that we got the Enterprise OID */
                got_enterprise_oid = 1;
                enterprise_oidP->count = os->length/sizeof(int);

                /* this value "over-rides" initial Object Class value */
                for (i = 0; i < enterprise_oidP->count; i++)
                    memcpy(&enterprise_oidP->value[i],
                           &os->string[ i*sizeof(int) ],
                           sizeof(unsigned int));
            }
        }

        /* is it the Agent Address OID? */
        else if ( (moss_compare_partial_oid (oid, &prefix_event_agentaddr_oid,
                                        0, 0, prefix_event_agentaddr_oid.count)
                                        == MAN_C_EQUAL) &&
             (got_agentaddr == 0) )
        {
            /* store the value of the Agent Address in return arg */
            if ( (os->length == sizeof(unsigned int)) && (os->string != NULL))
            {
                /* make note of the fact that we got the Agent Address OID */
                got_agentaddr = 1;
                *agent_addrP = *(unsigned int *)(os->string);
            }
        }

        /* is it the Generic Trap OID? */
        else if ((moss_compare_partial_oid (oid, &prefix_event_generictrap_oid,
                                       0, 0, prefix_event_generictrap_oid.count)
                                       == MAN_C_EQUAL) &&
             (got_generictrap_oid == 0) )
        {
            /* store the value of the generic trap in return arg */
            if ( (os->length == sizeof(int)) && (os->string != NULL) )
            {
                /* make note of the fact that we got the Generic Trap OID */
                got_generictrap_oid = 1;
                *generic_trapP = *(int *)(os->string);
            }
        }

        /* is it the Specific Trap OID? */
        else if ((moss_compare_partial_oid(oid, &prefix_event_specifictrap_oid,
                                      0, 0, prefix_event_specifictrap_oid.count)
                                      == MAN_C_EQUAL) &&
             (got_specifictrap_oid == 0) )
        {
            /* store the value of the specific trap in return arg */
            if ( (os->length == sizeof(int)) && (os->string != NULL) )
            {
                /* make note of the fact that we got the Specific Trap OID */
                got_specifictrap_oid = 1;
                *specific_trapP = *(int *)(os->string);
            }
        }

        /* is it the Event Time OID? */
        else if ( (moss_compare_partial_oid (oid, &prefix_event_eventtime_oid,
                                         0, 0, prefix_event_eventtime_oid.count)
                                         == MAN_C_EQUAL) &&
             (got_eventtime == 0) )
        {
            /* store the value of the event time in return arg */
            if ( (os->length == sizeof(unsigned int)) && (os->string != NULL) )
            {
                /* make note of the fact that we got the Event Time OID */
                got_eventtime = 1;
                *event_timeP = *(unsigned int *)(os->string);
            }
        }

        /* is it a Varbind List event argument? */
        else if ( (moss_compare_partial_oid (oid, &prefix_event_varbindarg_oid,
                                      0, 0, prefix_event_varbindarg_oid.count)
                                      == MAN_C_EQUAL) )
        {
            /* copy the avl element into the varbind_avl; if this is the  */
            /* first varbind list entry, just create the avl; next time   */
            /* around we'll convert varbindarg_avl into ASN.1, then we'll */
            /* re-use varbindarg_avl for *this* varbindarg.               */
            /* NOTE that we must keep track of whether we have already    */
            /* started the outermost SEQUENCE for the varbind args.       */
            if (varbindarg_count == 0)
            {
                /* initialize the avl that will hold this varbind arg */
                status = moss_avl_init (&varbindarg_avl);
                if (status != MAN_C_SUCCESS)
                {
                    sprintf (msg, 
              MSG(msg314, "M314 - Unable to create temporary varbind arg avl"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                    break;
                }

                /* strip off the "well-known" OID prefix, Enterprise OID,  */
                /* and Specific Trap integer from the argument's OID;      */

                start_of_real_oid = PREFIX_VARBINDARG_LENGTH   + 
                                    enterprise_oidP->count     + 
                                    1;  /* 1 for trap integer */
                if (start_of_real_oid >= oid->count)
                {
                    sprintf (msg, MSG(msg315, 
           "M315 - Mal-formed enterprise OID in event parameters varbind arg"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_BAD_PARAMETER;
                    break;
                }

                new_oid.count = oid->count - start_of_real_oid;
                new_oid.value = &oid->value[start_of_real_oid];
                modifier = MAN_C_SUCCESS;

                status = moss_avl_add (varbindarg_avl, &new_oid, modifier, 
                                       tag, os);
                if (status != MAN_C_SUCCESS)
                {
                    sprintf (msg, MSG(msg316, 
                    "M316 - Unable to moss_avl_add temporary varbind arg avl"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                    break;
                }
            }

            else /* convert previous varbind arg into ASN.1, then re-use  */
                 /* the varbindarg_avl for this event parameter           */
            {
                /* if this is the first varbind list element to be */
                /* converted, set 'build_start' flag to TRUE.      */
                if (varbindarg_count == 1)
                    build_start = TRUE;
                else
                    build_start = FALSE;

                build_end = FALSE;
                asn_status = avl_to_asn1 (varbindarg_avl, varbind_args_descP,
                                          &moss_error, build_start, build_end);
                                          
                if (asn_status != MCC_S_NORMAL)
                {
                    /* log TRAP PDU-encode failure */
                    sprintf (msg, MSG(msg321, 
"M321 - ASN.1 varbindlist encode failed on TRAP PDU; asn_status(%d), moss(%d)"),
                             asn_status, (int) moss_error);
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                    break;
                }

                /* now re-use the varbind_arg avl for "this" argument */
                moss_error = moss_avl_free (&varbindarg_avl, (int) FALSE);
                if (moss_error != MAN_C_SUCCESS)
                {
                    sprintf (msg, 
       MSG(msg317, "M317 - Unable to moss_avl_free temporary varbind arg avl"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                    break;
                }

                /* strip off the "well-known" OID prefix, Enterprise OID,  */
                /* and Specific Trap integer from the argument's OID;      */

                start_of_real_oid = PREFIX_VARBINDARG_LENGTH   + 
                                    enterprise_oidP->count     + 
                                    1;  /* 1 for trap integer */
                if (start_of_real_oid >= oid->count)
                {
                    sprintf (msg, 
MSG(msg320, "M320 - Mal-formed enterprise OID in event parameters varbind arg"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_BAD_PARAMETER;
                    break;
                }

                new_oid.count = oid->count - start_of_real_oid;
                new_oid.value = &oid->value[start_of_real_oid];
                modifier = MAN_C_SUCCESS;

                status = moss_avl_add (varbindarg_avl, &new_oid, modifier, 
                                       tag, os);
                if (status != MAN_C_SUCCESS)
                {
                    sprintf (msg, MSG(msg322,
                    "M322 - Unable to moss_avl_add temporary varbind arg avl"));
                    SYSLOG (LOG_ERR, msg);
                    status = MAN_C_PROCESSING_FAILURE;
                    break;
                }
            }

            /* count the varbind list argument */
            varbindarg_count += 1;
        }

        else /* the OID is for an event parameter which we can ignore */
        {
            sprintf (msg, MSG(msg318, 
                     "M318 - Ignoring Unknown OID in event parameters avl"));
            SYSLOG (LOG_INFO, msg);
        }

    } while ( (status == MAN_C_SUCCESS) && (last_one != TRUE) );


    if ( (varbindarg_count > 0) && (status == MAN_C_SUCCESS) )
    {
        /* if there is only one varbind list element to be */
        /* converted, set 'build_start' flag to TRUE.      */
        if (varbindarg_count == 1)
            build_start = TRUE;
        else
            build_start = FALSE;

        build_end = TRUE;  /* This IS the last varbindarg element */

        asn_status = avl_to_asn1 (varbindarg_avl, varbind_args_descP,
                                  &moss_error, build_start, build_end);

        if (asn_status != MCC_S_NORMAL)
        {
            sprintf (msg, MSG(msg323,
"M323 - ASN.1 varbindlist encode failed on TRAP PDU; asn_status(%d), moss(%d)"),
                     asn_status, (int) moss_error);
            SYSLOG (LOG_ERR, msg);
            status = MAN_C_PROCESSING_FAILURE;
        }

        /* free the varbindarg_avl ALWAYS (even if NOT success status) */
        if (varbindarg_avl != NULL)
            moss_error = moss_avl_free (&varbindarg_avl, (int) TRUE);
    }

    /* check to be sure we received ALL required event parameters */
    if ( (status == MAN_C_SUCCESS) && 
         ((got_eventtype_oid != 1) || 
          (got_specifictrap_oid != 1) ||
          (*event_typeP != *specific_trapP)) )
    {
        sprintf (msg, MSG(msg319, 
"M319 - Missing or mismatching event_type/specific_trap in event parameters avl"));
        SYSLOG (LOG_ERR, msg);
        status = MAN_C_BAD_PARAMETER;
    }

    return (status);

} /* end of carecv_extract_event_parms() */


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
**    Purpose:
**      This function assigns a unique UID to an event that has been posted,
**      and resets the UID counter for future use.
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

 big_picture_t  *bp;        /*-> Big Picture Structure pointer */
 uid            *event_uid; /*-> Event UID to be set           */
{
    char msg[LINEBUFSIZE];  /* Message build buffer */


#ifdef MTHREADS
    /* if (attempt to acquire the Event UID mutex failed) */
    if (pthread_mutex_lock (&bp->event_uid_m) != 0) 
    {
        sprintf (msg, MSG(msg329, 
                 "M329 - acquisition of Event UID mutex failed, errno = %d"),
                 errno);
        CRASH(msg);
    }
#endif

    /* Set event UID to current UID value, then increment UID value. */
    /* We are treating a uid is an integer here (first clear it out) */
    memset ( (void *) event_uid, '\0', (size_t) sizeof(uid) );
    *(int *)event_uid = bp->event_uid;
    if (bp->event_uid == EVENT_UID_MAX)
        bp->event_uid = 0;
    else
        bp->event_uid++;

#ifdef MTHREADS
    /* if (attempt to release the Event UID mutex failed) */
    if (pthread_mutex_unlock (&bp->event_uid_m) != 0) 
    { 
        sprintf (msg, MSG(msg330,
                 "M330 - release of Event UID mutex failed, errno = %d"),
                 errno);
        CRASH(msg);
    }
#endif

} /* end of evd_set_event_uid() */
