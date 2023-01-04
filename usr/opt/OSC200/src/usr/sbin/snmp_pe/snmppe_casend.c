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
static char *rcsid = "@(#)$RCSfile: snmppe_casend.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/08/02 18:07:11 $";
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
 * Module SNMPPE_CASEND.C
 *      Contains "Common Agent - Send Functions" for the SNMP Protocol Engine
 *      for the Common Agent.  These are functions "sending TO" the Common
 *      Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   July 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accept requests over a network,
 *       convert incoming information into standard Common Agent format
 *       and call the Common Agent for processing of that information.
 *       The main function has been entered, initialization has been
 *       performed.  Now processing must begin.
 *
 *    Purpose:
 *       This module contains the functions which accept an incoming
 *       SNMP PDU over a network connection and convert it into
 *       requests to the Common Agent.  The handling of the replies
 *       generated from the Common Agent is done in "snmppe_carecv.c",
 *       while the construction and sending of all response PDUs is
 *       (once the replies have been received) is also handled by functions
 *       in this module.
 *
 * History
 *      V1.0    July 1991              D. D. Burns
 *      V1.1    April 1992             D. D. Burns - Change calling
 *                                       arguments for some MIR functions
 *                                       for fix to correct GET-NEXT 1st roll
 *                                       bug.
 *      V1.2    May 1992               D. D. Burns - Change use of
 *                                       vbe->snmp_oid to be a copy of
 *                                       "in_entry" OID, rather than same
 *                                       (plus more GET-NEXT roll-fix work)
 *      V1.3    July 1992              D. D. Burns - Add support for new
 *                                       message class TRACE.
 *
 *              August 14, 1992        Densmore/Peller - changed modifier
 *                                     parameter to moss_avl_add() calls
 *                                     to be MAN_C_SUCCESS! (again)
 *                                     The moss compare routine will FAIL
 *                                     if the avl modifier field is not
 *                                     MAN_C_SUCCESS.
 *
 *              September 21, 1992     Set modifier according to the
 *                                     PDU/operation type for moss_avl_add()'s.
 *
 *              December 09, 1992      D. McKenzie - Changed magic_getnext_oid
 *                                     from 1.3.6.1.4.1.36.2.99 (length 9)
 *                                     to 1.3.12.2.1011.2.3.1.126.5 (length 10).
 *

Module Overview:
---------------

This module contains the SNMP PE function(s) that processing requests
coming into SNMP PE that are to be transformed and then Sent-To the
Common Agent (hence the module name "_casend.c")


Thread Overview:
---------------

All the functions in this module are executed exclusively by the main
("sending") thread.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------

casend_get_msg          Waits for an inbound PDU to arrive, causes it to
                        be ASN.1 "deserialized" into a "service block"
                        allocated to receive its contents.  The service
                        block is entered onto the list of service blocks
                        (list header in the Big Picture) describing all
                        the requests currently being handled by SNMP PE.
                        Errors in reception and deserialization result in
                        appropriate logging of these errors.

casend_process_msg      Processing of a specific request in a specific
                        service block occurs.  The SNMP request in converted
                        into one or more Common Agent requests with
                        accompanying conversion of Service Block contents
                        to a form for transmission via MSI interface
                        functions to the Common Agent.  Errors result in
                        appropriate logging (local and via traps).


MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------
casend_get_service      Returns an available-for-use Service Block to its
                        caller ("casend_get_msg()")

casend_authenticate     Performs authentication process on newly arrived PDU

casend_process_SET      Process a SET Request outbound to the Common Agent
                        at the request of "casend_process_msg()"

casend_process_GET      Process a GET Request outbound to the Common Agent
                        at the request of "casend_process_msg()"

casend_process_GETNEXT  Process a GET-Next Request outbound to the Common Agent
                        at the request of "casend_process_msg()"

casend_do_invoke_action Issues the actual msi "invoke-action" request on behalf
                        of casend_process_GETNEXT() (and
                        casend_get_noroll_status) to handle logging of
                        weird conditions and map the myriad CA RETURN codes
                        into a local SNMP code (generr, nosuch, success, roll).

casend_get_noroll_status
                        Contains the loop that repeatedly does a class roll
                        and then a msi invoke-action call until a CA RETURN
                        code comes back that does NOT imply a class roll.

casend_bld_instance_AVL Munge the class OID (discovered from MIR), instance
                        information (also from MIR) together with the
                        original SNMP OID and build the "object_instance"
                        AVL to be passed to the Common Agent via msi_* call.

casend_bld_IA_element   Builds one class's naming attribute "element" into the
                        instance AVL ("IA"). (Supports
                        "casend_bld_instance_AVL()").

casend_bld_IA_subelement Builds one element's "subelement" into the AVL that
                        is ultimately appended into the instance AVL.
                        (Supports "casend_bld_IA_element()").

casend_bld_attribute_AVL Munge the class OID (discovered from MIR), and the
                        original SNMP OID to create the attribute OID and
                        store whatever data were passed into attribute AVL.
*/

/* Module-wide Stuff */
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>

#if defined(__osf__) && !defined(_OSF_SOURCE)
/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
   typedefs unless you've got _OSF_SOURCE turned on. */
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE

#else
#include <sys/types.h>
#endif

#include <arpa/inet.h>

/* includes required for "snmppe.h" */
#include "port_cmip.h"
#include <stdio.h>
#include "moss_private.h"     /* for manipulating Instance AVL's modifier */
#include "moss.h"
#include <netinet/in.h>
#define OID_DEF
#include "snmppe.h"

/* Common Agent */
#include "extern_nil.h"
#include "moss_inet.h"
#include "agent.h"
#include "agent_access_config.h"


/*
|==============================================================================
|  bld_IA_status - Build Instance AVL Status
|
|    This status is returned by casend_bld_instance_avl() and it's minions
|    casend_bld_IA_element() and casend_bld_IA_subelement().  These codes are
|    used only within this module, consequently they are defined here instead
|    of in snmppe.h
|==============================================================================
*/
typedef
    enum {
      PARTIAL_PROC_DONE,        /* Partial Processing of instance info OK    */
      FULL_EXACT,               /* All arcs converted, nothing extra/missing */
      FULL_AND_EXTRA_ARCS,      /* Conversion w/extra arcs, AVL complete     */
      FULL_BUT_EXTRA_ARCS,      /* Conversion w/extra arcs, AVL empty        */
      ONE_CONVERTED_THEN_JUNK,  /* Conversion fails after 1st, AVL partial   */
      NONE_CONVERTED_JUNK,      /* Conversion fails on 1st, AVL empty        */
      NONE_CONVERTED_OK,        /* AVL empty, no conversion should be done   */
      NONE_CONVERTED_NONE_PRESENT, /* No arcs, no conversion, AVL empty      */
      SUBELEMENT_OK_CONVERT,    /* Subelement for AVL converted OK           */
      SUBELEMENT_FAIL_CONVERT,  /* Subelement for AVL failed to convert      */
      DONE_IGNORE_INSTANCE      /* Conversion is done; instance info dropped */
      } bld_IA_status;


/*
|==============================================================================
|  bld_IA_string - Build Instance AVL Status String Interpretation
|
|    This array gives an ASCII string interpretation to the codes listed
|    above.
|==============================================================================
*/
char    *bld_IA_string[11] = {
      "PARTIAL_PROC_DONE",      /* Partial Processing of instance info OK    */
      "FULL_EXACT",             /* All arcs converted, nothing extra/missing */
      "FULL_AND_EXTRA_ARCS",    /* Conversion w/extra arcs, AVL complete     */
      "FULL_BUT_EXTRA_ARCS",    /* Conversion w/extra arcs, AVL empty        */
      "ONE_CONVERTED_THEN_JUNK",/* Conversion fails after 1st, AVL partial   */
      "NONE_CONVERTED_JUNK",    /* Conversion fails on 1st, AVL empty        */
      "NONE_CONVERTED_OK",      /* AVL empty, no conversion should be done   */
      "NONE_CONVERTED_NONE_PRESENT", /* No arcs, no conversion, AVL empty    */
      "SUBELEMENT_OK_CONVERT",  /* Subelement for AVL converted OK           */
      "SUBELEMENT_FAIL_CONVERT",/* Subelement for AVL failed to convert      */
      "DONE_IGNORE_INSTANCE"    /* Conversion is done; instance info dropped */
      };


/*
|==============================================================================
|  mir_derive_CI_status - ASCII Interpretation
|
|    This array gives an ASCII string interpretation to the codes defined
|    by typedef "mir_derive_CI_status" in snmppe.h for use in Trace statements
|    in this module and elsewhere (snmppe_mir.c).
|==============================================================================
*/
char    *CI_status_string[7] = {
        "CI_ATTRIBUTE",
        "CI_CLASS",
        "CI_ROLLED_CLASS",
        "CI_ROLLED_ATTRIBUTE",
        "CI_NOSUCH",
        "CI_PROC_FAILURE",
        "CI_NOT_SET_YET"};


/*
|==============================================================================
|  e_status_t - ASCII Interpretation
|
|    This array gives an ASCII string interpretation to the codes defined
|    by typedef "e_status_t" in snmppe.h for use in TRACE debug statements
|    in this module and snmppe_carecv.c
|==============================================================================
*/
char    *e_status_string[8] = {
        /* PDU-"LEGAL" */
        "noError",
        "tooBig",
        "noSuch",
        "badValue",
        "readOnly",
        "genErr",               

        /* PDU-"BOGUS" */
        /* SNMP PE INTERNAL: Roll to the next 'class' for GET-NEXT */
        "rollclass",

        /* SNMP PE INTERNAL: Roll to the next 'attribute' for GET-NEXT */
        "rollattrib"
        };


/*
|==============================================================================
| This macro builds an error message using argument "oid" as the
| source of an OID is to appear in the message.  The message is built
| in buffer named "msg".
|
| This macro serves to make the code a little easier and hide the details
| of the conversion of the oid to text.
|==============================================================================
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

/* casend_authenticate - Perform Authentication checks on PDU */
static BOOL
casend_authenticate PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       *       /*->> Service Blk (on Service List) for PDU          */
));

/* casend_get_service - Get an Available Service Block */
static void
casend_get_service PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE    */
service_t       **      /*-> Free Service Block (on Service List) */
));

/* casend_process_GET - Process a GET Request to the Common Agent */
static void
casend_process_GET PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       *       /*-> Service Block (on Service List) to be processed */
));

/* casend_process_SET - Process a SET Request to the Common Agent */
static void
casend_process_SET PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       *       /*-> Service Block (on Service List) to be processed */
));

/* casend_process_GETNEXT - Process a GET-NEXT Request to the Common Agent */
static void
casend_process_GETNEXT PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       *       /*-> Service Block (on Service List) to be processed */
));

/* casend_do_invoke_action - Issue an Invoke-Action Request to Common Agent */
static e_status_t
casend_do_invoke_action PROTOTYPE((
big_picture_t   *,         /*-> Big Picture Structure for SNMP PE            */
varbind_t       *,         /*-> The current Varbind Entry list element       */
avl             *,         /* -> Instance AVL being built for CA call        */
avl             *,         /* -> Attribute List AVL being built for CA call  */
avl             *          /* -> Access Control AVL being built for CA call  */
));

/* casend_get_noroll_status - Roll/Invoke-Action until no Roll status rtned  */
static e_status_t
casend_get_noroll_status PROTOTYPE((
big_picture_t   *,         /*-> Big Picture Structure for SNMP PE            */
e_status_t      ,          /* Incoming Get-Next status for evaluation        */
varbind_t       *,         /*-> The current Varbind Entry list element       */
avl             *          /* -> Access Control AVL being built for CA call  */
));

/* casend_bld_instance_avl - Build the Instance AVL */
static bld_IA_status
casend_bld_instance_avl PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE  */
mir_derive_CI_status ,          /* MIR Class/Instance derivation status */
varbind_t       *,              /*-> Varbind Entry Block for SNMP OID   */
avl             **,             /*->> Pointer to where we build AVL     */
int             *,              /*-> Where to return i_arc for emsgs    */
int             *,              /*-> Where to return i_arc_left-> emsgs */
pdu_type                        /* PDU type (GET, SET, GETNEXT)         */
));

/* casend_bld_IA_element - Build one Element (for one class) in Instance AVL */
static bld_IA_status
casend_bld_IA_element PROTOTYPE((
big_picture_t         *,             /*-> Big Picture Structure for SNMP PE  */
object_id             *,             /*-> Full SNMP OID we're working on     */
int                   *,             /*-> Index in SNMPOID value[] to nextarc*/
int                   *,             /*-> Count of remaining arcs in value[] */
snmp_instance_t       *,             /*-> 1st MIR Instance Block for a class */
int                   *,             /*-> Count of sub-identifiers proc'ed ok*/
avl                   *,             /*-> Instance AVL for class element     */
pdu_type              ,              /* PDU type (GET, SET, GETNEXT)         */
varbind_t             *,             /*-> Varbind Entry Block for SNMP OID   */
int                   *              /*-> Flag (1+ if Inst arcs converted)   */
));

/* casend_bld_IA_subelement - Build one subelement for a class element */
static bld_IA_status
casend_bld_IA_subelement PROTOTYPE((
big_picture_t         *,             /*-> Big Picture Structure for SNMP PE  */
object_id             *,             /*-> Full SNMP OID we're working on     */
int                   *,             /*-> Index in SNMPOID value[] to nextarc*/
int                   *,             /*-> Count of remaining arcs in value[] */
snmp_instance_t       **,            /*->> MIR Instance blk for a subelement */
avl                   *,             /*->> Pointer to instance element AVL   */
BOOL                  ,              /* TRUE: We're hacking the build of a   */
                                     /*       subelement of higher class     */
pdu_type              ,              /* PDU type (GET, SET, GETNEXT)         */
varbind_t             *,             /*-> Varbind Entry Block for SNMP OID   */
int                   *              /*-> Flag (1+ if Inst arcs converted)   */
));

/* casend_bld_attribute_avl - Build the Attribute List AVL */
static man_status
casend_bld_attribute_avl PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE  */
object_id       *,              /*-> Full SNMP OID we're working on     */
object_id       *,              /*-> OID of the class we're working on  */
mir_derive_CI_status ,          /* MIR Class/Instance derivation status */
int              ,              /* ASN.1 Tag read from varbind list     */
octet_string    *,              /*->AVL octet_string from varbind list  */
avl             **,             /*->> Pointer to where we build AVL     */
pdu_type                        /* PDU type (GET, SET, GETNEXT)         */
));

/* casend_bld_access_control_avl - Build the Access Control AVL */
static man_status
casend_bld_access_control_avl PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE  */
service_t       *,              /*-> Service Block (on Service List)    */
                                /* to be processed.                     */
avl             **              /*->> Pointer to where we build AVL     */
));

/* casend_update_avl_modifier - Update Instance AVL Modifier Value */
static void
casend_update_avl_modifier PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE  */
avl             *,              /*-> Instance AVL                       */
unsigned int                    /* New modifier value for Instance AVL  */
));


/* casend_get_msg - Get Inbound PDU deserialized into Service Block */
/* casend_get_msg - Get Inbound PDU deserialized into Service Block */
/* casend_get_msg - Get Inbound PDU deserialized into Service Block */

void
casend_get_msg( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       **svc;  /*->> Service Blk (on Service List) to be processed  */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a pointer (to a Service Block) that is to
    be set upon return to point to the Service Block containing the newly
    received PDU.


OUTPUTS:

    The function returns only when a PDU has been successfully received and
    deserialized into a Service Block, at which time a pointer to the
    Service Block is set into the passed pointer "svc".  The Service Block
    has been properly inserted into the Service List (maintained off the
    Big Picture) upon return so that the receiving thread can find it.

    In a multiply-threaded/RPC environment access to the Service List is
    protected by a mutex in the Big Picture.


BIRD'S EYE VIEW:
    Context:
        The caller is the main() function of SNMP PE.  Initialization of
        SNMP PE has been successfully performed and the first (next) inbound
        SNMP message is desired.

    Purpose:
        This function performs network I/O to receive the SNMP PDU and
        fully converts it into an SNMP PE internal representation in
        a Service Block in preparation for further processing.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the descriptor for the PDU buffer>
    <acquire a service block for use in receiving the next PDU>

    do
        <perform network I/O to receive the next PDU into local buffer>
        <record current service counter value in service block for this pdu>
        <perform deserialization of PDU into service block>
        while (status is not SUCCESS)

    <return service block pointer>


OTHER THINGS TO KNOW:

    Logging of errors is performed as close to where the errors are
    detected as possible, consequently this function simply repeatedly
    tries to obtain a filled-in service block for return to its caller.

*/

{
service_t         *local_svc;           /* Local Pointer to allocated service block */
BOOL              status;               /* Status returned from netio call          */
MCC_T_Descriptor  pdu_info;             /* Descriptor for "pdu_buf[]"               */
char              pdu_buf[MAXPDUSIZE];  /* Receive the SNMP Request PDU to this buf */

/* initialize the descriptor for the PDU buffer */
pdu_info.mcc_b_class = DSC_K_CLASS_S;
pdu_info.mcc_b_dtype = DSC_K_DTYPE_T;
pdu_info.mcc_b_flags = 0;
pdu_info.mcc_b_ver = 1;
pdu_info.mcc_w_maxstrlen = MAXPDUSIZE;
pdu_info.mcc_w_curlen = 0;
pdu_info.mcc_a_pointer = (unsigned char *) pdu_buf;
pdu_info.mcc_l_id = 0;
pdu_info.mcc_l_dt = 0;

/* acquire a service block for use in receiving the next PDU. (an acquired */
/* service block is returned strung on service list in the Big Picture)    */
casend_get_service( bp, &local_svc );


do {
    /* perform network I/O to receive the next PDU into local buffer.       */
    /* The sending address is stored in the associated service block        */
    /* (local_svc).                                                         */

    /* (This doesn't return until it succeeds) */
    netio_get_pdu( bp, local_svc, &pdu_info );

    MIR_STATS_TO_STDERR("PDU just received OK");

    /* record current service counter value in service block for this pdu */
    local_svc->svc_count = bp->service_counter;
    bp->service_counter += 1;                     /* Bump it for next one */

    /* perform deserialization of PDU into service block */
    /* (This returns success/failure)                    */
    status = netio_deserialize_pdu( bp, local_svc, &pdu_info );
    }
    while (status == FALSE);                 /* status is not SUCCESS */


/* return service block pointer */
*svc = local_svc;

}

/* casend_authenticate - Perform Authentication checks on PDU */
/* casend_authenticate - Perform Authentication checks on PDU */
/* casend_authenticate - Perform Authentication checks on PDU */

static BOOL
casend_authenticate( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       *svc;   /*->> Service Blk (on Service List) for PDU          */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.  Specifically the
    community list in the Big Picture is of interest here.

    "svc" is the address of a pointer (to a Service Block) that corresponds
    to the received PDU to be checked for authentication.


OUTPUTS:

    The function returns TRUE if:

      * the PDU has a community name we can recognize and
      * the manager's address matches the community and
      * it is not trying to write into a "readonly" community and
      * the community is not of access mode "none"
    and stores a pointer to the community block into the service block that
    describes the PDU, thereby linking the PDU to it's community.

    otherwise it returns FALSE.

    If a PDU is "get" and the community mode is "writeonly", we allow the
    "get" to occur.  The MOM will disallow the "get" if the specific variable
    in not readable.  Note that "readOnly" status can never be returned in the
    error status field of a PDU, as this is illegal.  Instead, a "genErr" 
    status is returned.

BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_msg() function in this module.
        Before it does anything to the newly received PDU it needs to
        authenticate it.

    Purpose:
        This function performs the necessary authentication chores.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (we're in debugging mode)
        if (PDU is a SET and community name is "PESHUTDOWN")
            <CRASH "Mxxx - Controlled Exit Requested from address "a.b.c.d">

    <assume FALSE>
    for (each community block on the community list)
        if (community block name matches PDU community)
            if (community block address is zero or matches PDU received-from)
                if (community mode is "none" OR 
                    PDU is "set" and community mode is "readonly")
                    <increment count of bad community use>
                    <return FALSE>
                else
                    <show TRUE>
                    <store community block pointer into service block>
                    <break>

    if (return code is FALSE)
        <increment count of bad community name>

    <return code>


OTHER THINGS TO KNOW:

    This is the function to spiff up for fancier authentication.
    
*/

{
BOOL            status;         /* Status returned from netio call    */
community_t     *current;       /* Community Block we're working on   */
char            buf[LINEBUFSIZE];       /* Error Message build buffer */
extern int      strncasecmp();


/* if (we're in debugging mode) */
if (bp->log_state.debugging_mode == TRUE) {

    /* if (PDU is a SET and community name is "PESHUTDOWN") */
    if (   svc->pdu == SET
        && strncasecmp("PESHUTDOWN", svc->comm_name, svc->comm_namelen) == 0) {
        sprintf(buf,
                MSG(msg221, "M221 - Controlled Exit Requested from (%s)"),
                inet_ntoa(svc->inbound.sin_addr)
                );
        CRASH( buf );
        }
    }

status = FALSE;         /* assume FALSE */

/* for (each community block on the community list) */
for (current = bp->community_list; current != NULL; current = current->next) {

    /* if (community block name matches PDU community) */
    if ((strncasecmp(current->comm_name, svc->comm_name, svc->comm_namelen)==0)
         && (strlen (current->comm_name) == svc->comm_namelen)) {

        /* if (community block address is zero or matches PDU received-from) */
        if ( (current->comm_addr == 0) ||
             (bcmp(&current->comm_addr, &svc->inbound.sin_addr, 4) == 0)) {

            /* if (community mode is "none" OR                   */
            /*     PDU is "set" and community is "readonly")     */
            if ((current->access_mode == mode_none) ||
                (svc->pdu == SET && current->access_mode == mode_readOnly))
                {
                /* increment count of bad community use */
                bp->statistics->badcommuse += 1;
                return(FALSE);
                }

            else {
                status = TRUE;      /* show TRUE */
                /* store community block pointer into service block */
                svc->ok_community = current;
                break;
                }
            }
        }
    }

/* if (return code is FALSE) */
if (status == FALSE) {
    /* increment count of bad community name */
    bp->statistics->badcommname += 1;
    }

return(status);
}

/* casend_get_service - Get an Available Service Block */
/* casend_get_service - Get an Available Service Block */
/* casend_get_service - Get an Available Service Block */

static void
casend_get_service( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE    */
service_t       **svc;  /*-> Free Service Block (on Service List) */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a pointer (to a Service Block) that is to
    be set upon return to point to the available Service Block.


OUTPUTS:

    The function returns only when it has made available to it's caller
    a free Service Block, properly strung on the Service List in the
    Big Picture.

    In a multiply-threaded/RPC environment access to the Service List is
    protected by a mutex in the Big Picture.


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_get_msg() function of SNMP PE.  Before
        it can receive a PDU it must have a Service Block into which it
        can be deserialized.

    Purpose:
        This function performs all required chores to make a Service
        Block available to its caller.


ACTION SYNOPSIS OR PSEUDOCODE:

ifdef MTHREADS
    if (attempt to acquire the Service List mutex failed)
        <CRASH "Mxxx - acquisition of Service List mutex failed, errno=%d">
endif

    <initialize selected service block pointer NULL>
    while (Service Blocks remain to be examined on the Service List)
        if (service block is inactive)
            <set selected service block pointer to this block>
            <mark block "active">
            <reset any fields that ought to be initialized>
            <free any allocated heap storage used by cells in this block>
            <break>
        <load pointer to next service block>

    if (we found no inactive blocks)
        if (attempt to allocate storage for new block failed)
            <CRASH "Mxxx - No memory for Service Block allocation">
        <initialize all cells in new block appropriately>
        <insert new block at top of Service List>

    <set selected block pointer into passed Service Block pointer

ifdef MTHREADS
    if (attempt to release the Service List mutex failed)
        <CRASH "Mxxx - release of Service List mutex failed, errno=%d">
endif


OTHER THINGS TO KNOW:

<TC> In function casend_get_service(). . .
<TC> This function attempts first to locate an available (inactive) Service
<TC> Block already strung onto the Service List in the Big Picture.  If found,
<TC> it simply returns it marked as "active".  In a NOIPC enviroment, after
<TC> processing the first request, there will always be one waiting for re-use.
<TC> 
<TC> In a multiply-threaded environment it must lock the Service List in the
<TC> Big Picture using the mutex located there, before searching the list for
<TC> an inactive Service Block.  The list must remain locked if none was
<TC> found, while a new block is allocated and strung onto the list.

    By convention, when a service block is marked "inactive", all resources
    in it (storage for AVL's, strings and the like) are left "as-is" to
    be recovered by the code present here when the block is "re-used".

    By this strategem, all storage recovery code for this block is
    concentrated right here in this function.

*/

{
service_t       *new_svc;               /* -> New service block       */
service_t       *next_svc;              /* -> Next svc blk on list    */

#ifdef MTHREADS
char            buf[LINEBUFSIZE];       /* Error Message build buffer */

/* if (attempt to acquire the Service List mutex failed) */
if (pthread_mutex_lock(&bp->service_list_m) != 0) {
    sprintf( buf,
            MSG(msg040, "M040 - acquisition of Service List mutex failed, errno = %d"),
            errno);
    CRASH( buf );
    }
#endif


new_svc = NULL;                 /* init selected service block pointer NULL */
                                /* (Ie: "Haven't found anything yet")       */
next_svc = bp->service_list;    /* -> 1st on list, if any                   */


/* while (Service Blocks remain to be examined on the Service List) */
while (next_svc != NULL) {

    /* if (service block is inactive) */
    if (next_svc->svc_active == FALSE) {

        /* set selected service block pointer to this block */
        new_svc = next_svc;

        new_svc->svc_active = TRUE;  /* mark block "active" */

        /* reset any fields that ought to be initialized */
        new_svc->ok_community = NULL;
        bzero(&new_svc->inbound, sizeof(new_svc->inbound));
        new_svc->pdu = UNKNOWN;

        /* free any allocated heap storage used by cells in this block */
        if (new_svc->comm_name != NULL) {
            free(new_svc->comm_name);
            }
        new_svc->comm_namelen = 0;
        new_svc->replies_rcvd = 0;

        /*
        |  NOTE: varbind_list remains!  It will be re-used also!
        |        Don't add code here to free it, as netio_serialize_pdu()
        |        has the necessary smarts in it to re-use these varbind_list
        |        blocks.
        */

        break;
        }

    next_svc = next_svc->next;    /* load pointer to next service block */
    }                             /* on the service list (if any): LOOP */


/* if (we found no inactive blocks) */
if (new_svc == NULL) {

    /* if (attempt to allocate storage for new block failed) */
    if ( (new_svc = (service_t *) malloc(sizeof(service_t))) == NULL) {
        CRASH(MSG(msg042, "M042 - No memory for Service Block allocation"));
        }

    /* initialize all cells in new block appropriately */
    new_svc->svc_active = TRUE;
    new_svc->ok_community = NULL;
    bzero(&new_svc->inbound, sizeof(new_svc->inbound));
    new_svc->replies_rcvd = 0;
    new_svc->svc_count = 0L;
    new_svc->version = 0;
    new_svc->comm_name = NULL;
    new_svc->comm_namelen = 0;
    new_svc->pdu = UNKNOWN;
    new_svc->request_id = 0;
    new_svc->error_status = 0;
    new_svc->error_index = 0;
    new_svc->varbind_list = NULL;
    /*
    |  NOTE: netio_serialize_pdu() knows how to create these varbind blocks and
    |        add them to this list as needed as the PDU is parsed.
    */

    /* insert new block at top of Service List */
    new_svc->next = bp->service_list;
    bp->service_list = new_svc;
    }


*svc = new_svc;  /* set selected block ptr into passed Service Block pointer */


#ifdef MTHREADS
/* if (attempt to release the Service List mutex failed) */
if (pthread_mutex_unlock(&bp->service_list_m) != 0) { 
    sprintf( buf,
            MSG(msg041, "M041 - release of Service List mutex failed, errno = %d"),
            errno);
    CRASH( buf );
    }
#endif

}

/* casend_process_msg - Process the received msg into CA requests */
/* casend_process_msg - Process the received msg into CA requests */
/* casend_process_msg - Process the received msg into CA requests */

void
casend_process_msg( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       *svc;   /*-> Service Block (on Service List) to be processed */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that contains the newly
    received and deserialized inbound PDU.


OUTPUTS:

    The function returns only when a deserialized PDU in the service block
    has been processed, either successfully (Common Agent requests have
    been issued) or unsuccessfully (error processing has been performed).

    Note that even in a threads environment, this function returns only after
    the receiving thread has received the Common Agent replies and after
    a response has been sent to the user.


BIRD'S EYE VIEW:
    Context:
        The caller is the main() function of SNMP PE.  Initialization of
        SNMP PE has been successfully performed and the first (next) inbound
        SNMP message has been received and successfully deserialized.

    Purpose:
        This function performs the processing necessary to convert
        the inbound PDU message (passed in via the service block) into
        Common Agent request(s).


ACTION SYNOPSIS OR PSEUDOCODE:

    if (authenticate failed)
        <issue trap>
        <mark service block as INACTIVE>
        <return>

    <show PDU error status as "noError">
    <show PDU error index as 0>

    IFLOGGING (TRACE)
        <Show PDU Type, Inbound-from address, request id>

    switch (kind of PDU request)
        case GET:
            IFLOGGING (PDUGETIN)
                <log GET PDU>

            <perform GET-specific processing-setting PDU error status/index>
            <count another GET request processed>
            <mark PDU as of type "GET-RESPONSE">

            if (status is "noError")
                IFLOGGING (PDUGETOUT))
                    <log GET response PDU>
            else
                IFLOGGING (PDUGETERR))
                    <log error GET response PDU>

            break;

        case GETNEXT:
            IFLOGGING (PDUNXTIN)
                <log NXT PDU>

            <perform GETNEXT-specific processing-setting PDU error status/index>
            <count another GETNEXT request processed>
            <mark PDU as of type "GET-RESPONSE">

            if (status is "noError")
                IFLOGGING (PDUNXTOUT))
                    <log GET response PDU>
            else
                IFLOGGING (PDUNXTERR))
                    <log error GET-Next response PDU>

            break;

        case SET:
            IFLOGGING (PDUSETIN)
                <log SET PDU>

            <perform SET-specific processing-setting PDU error status/index>
            <count another SET request processed>
            <mark PDU as of type "GET-RESPONSE">

            if (status is "noError")
                IFLOGGING (PDUSETOUT))
                    <log SET response PDU>
            else
                IFLOGGING (PDUSETERR))
                    <log error SET response PDU>
            break;

        default:
            <inrement count of bad-typed PDUs>
            <show error status of genErr>
            break

    <increment count of "get-response" PDUs generated>

    switch (error status)
        case nosuch:
            <increment count of "nosuch" replies>
            <break>
        case badvalue:
            <increment count of "badvalue" replies>
            <break>
        case readOnly:
            <increment count of "readonly" replies>
            <break>
        case genErr:
            <increment count of "generr" replies>
            <break>

    <build serialization buffer descriptor>

    if (attempt to serialize PDU succeeded)
        <endeavor to send the pdu>
    else
        if (attempt failed due to "too big")
            <set PDU error status to "toobig">
            <increment count of "toobig" replies>
            <set PDU error index to 0>
            if (attempt to serialize PDU succeeded)
                <endeavor to send the pdu>
            else
                <SYSLOG "Mxxx - Unable to serialize PDU: dropped">
        else
            <SYSLOG "Mxxx - Unable to serialize PDU: dropped">

    <mark service block as INACTIVE>

    
OTHER THINGS TO KNOW:

    Logging of errors is performed as close to where the errors are
    detected as possible, consequently this function simply returns
    after successfully or unsuccessfully processing the inbound PDU
    message.

      - Version OK - statistics
      - Request Type OK - statistics
      - Community name
*/

#define LOGIT() fprintf(bp->log_state.log_file, buf)
{
char        buf[LINEBUFSIZE];   /* Error Message build buffer        */
int         ss;                 /* Error Msg "String Start" in buf[] */
MCC_T_Descriptor  pdu_info;     /*-> Desc for buf to receive serialized PDU  */
char              pdu_buf[MAXPDUSIZE];  /* Bld the SNMP Resp PDU in this buf */
BOOL              toobig;               /* TRUE: PDU encoded TOO big         */


/* if (authenticate failed) */
if (casend_authenticate(bp, svc) == FALSE) {

    /* issue trap */
    /* Pass in NULLs to use system default for enterprise oid, agent-addr, */
    /* time-stamp and no varbind list respectively, since                  */
    /* authenticationFailure trap should only be generated by PE.          */

    netio_send_trap(bp, NULL, NULL, authenticationFailure, 0, NULL, NULL);

    /* mark service block as INACTIVE */
    svc->svc_active = FALSE;

    return;
    }

svc->error_status = noError;    /* show PDU error status as "noError" */
svc->error_index = 0;           /* show PDU error index as 0 */

IFLOGGING ( L_TRACE ) {
    char        *pdu_type_name;         /* "GET", "SET" etc                  */

    /* Show PDU Type, Inbound-from address, request id */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    /* Issue a blank line to set new PDU apart from last */
    buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */

    pdu_type_name = "Unknown";
    if (svc->pdu == GET) pdu_type_name = "GET";
    else if (svc->pdu == SET) pdu_type_name = "SET";
    else if (svc->pdu == GETNEXT) pdu_type_name = "GET-NEXT";
    else if (svc->pdu == GETRESP) pdu_type_name = "GET-RESPONSE";
    else if (svc->pdu == TRAP) pdu_type_name = "TRAP";

    sprintf(&buf[ss],
            "(%s) PDU RECEIVED from %s, Request ID: (%d/%x)\n",
            pdu_type_name,
            inet_ntoa(svc->inbound.sin_addr),
            svc->request_id,
            svc->request_id);

    LOGIT();
    }

 switch (svc->pdu)  {      /* kind of PDU request */

    case GET:

        IFLOGGING( L_PDUGETIN ) {                         /* log GET PDU */
            log_service(bp, L_PDUGETIN, svc);
            }

        /* do GET-specific processing-setting PDU error status/index     */
        casend_process_GET(bp, svc);

        /* count another GET request processed */
        bp->statistics->ingetreq += 1;

        svc->pdu = GETRESP;        /* mark PDU as of type "GET-RESPONSE" */

        /* if (status is "noError") */
        if (svc->error_status == noError) {
            IFLOGGING( L_PDUGETOUT ) {           /* log GET response PDU */
                log_service(bp, L_PDUGETOUT, svc);
                }
            }
        else {
            IFLOGGING( L_PDUGETERR ) {     /* log error GET response PDU */
                log_service(bp, L_PDUGETERR, svc);
                }
            }
        break;

    case GETNEXT:
        IFLOGGING( L_PDUNXTIN ) {                    /* log GET-Next PDU */
            log_service(bp, L_PDUNXTIN, svc);
            }

        /* do GETNEXT-specific processing-setting PDU error status/index */
        casend_process_GETNEXT(bp, svc);

        /* count another GET-NEXT request processed */
        bp->statistics->ingetnxtreq += 1;

        svc->pdu = GETRESP;         /* mark PDU as of type "GET-RESPONSE" */

        /* if (status is "noError") */
        if (svc->error_status == noError) {
            IFLOGGING( L_PDUNXTOUT ) {       /* log GET-Next response PDU */
                log_service(bp, L_PDUNXTOUT, svc);
                }
            }
        else {
            IFLOGGING( L_PDUNXTERR ) { /* log error GET-Next response PDU */
                log_service(bp, L_PDUNXTERR, svc);
                }
            }
        break;

    case SET:

        IFLOGGING( L_PDUSETIN ) {                          /* log SET PDU */
            log_service(bp, L_PDUSETIN, svc);
            }

        /* do SET-specific processing-setting PDU error status/index      */
        casend_process_SET(bp, svc);

        /* count another SET request processed */
        bp->statistics->insetreq += 1;

        svc->pdu = GETRESP;         /* mark PDU as of type "GET-RESPONSE" */

        /* if (status is "noError") */
        if (svc->error_status == noError) {
            IFLOGGING( L_PDUSETOUT ) {            /* log SET response PDU */
                log_service(bp, L_PDUSETOUT, svc);
                }
            }
        else {
            IFLOGGING( L_PDUSETERR ) {       /* log error SET response PDU */
                log_service(bp, L_PDUSETERR, svc);
                }
            }
        break;

    default:
        /* inrement count of bad-typed PDUs */
        bp->statistics->badtype += 1;

        /* show error status of genErr */
        svc->error_status = genErr;
        break;
    }

/* increment count of "outgetresp" replies */
bp->statistics->outgetresp += 1;

/* Do statistics on the kind of replies we're generating */
switch (svc->error_status) {
   case noSuch:
       /* increment count of "nosuch" replies */
       bp->statistics->outnosuch += 1;
       break;
   case badValue:
       /* increment count of "badvalue" replies */
       bp->statistics->outbadvalue += 1;
       break;
   case readOnly:
       /* increment count of "readonly" replies */
       bp->statistics->outreadonly += 1;
       break;
   case genErr:
       /* increment count of "generr" replies */
       bp->statistics->outgenerr += 1;
       break;

   case noError:
   default:
       break;
   }

IFLOGGING ( L_TRACE ) {
    sprintf(&buf[ss],
            "PDU PROCESSING COMPLETE from %s,  Request ID: (%d/%x)  Status=%s\n",
            inet_ntoa(svc->inbound.sin_addr),
            svc->request_id,
            svc->request_id,
            e_status_string[svc->error_status]
            );
    LOGIT();
    }

/* initialize the descriptor for the PDU serialization buffer */
pdu_info.mcc_b_class = DSC_K_CLASS_S;
pdu_info.mcc_b_dtype = DSC_K_DTYPE_T;
pdu_info.mcc_b_flags = 0;
pdu_info.mcc_b_ver = 1;
pdu_info.mcc_w_maxstrlen = MAXPDUSIZE;
pdu_info.mcc_w_curlen = 0;
pdu_info.mcc_a_pointer = (unsigned char *) pdu_buf;
pdu_info.mcc_l_id = 0;
pdu_info.mcc_l_dt = 0;

/* if (attempt to serialize PDU succeeded) */
if (netio_serialize_pdu(bp, svc, &pdu_info, &toobig) == TRUE) {
    /* endeavor to send the pdu */
    netio_put_pdu(bp, svc, &pdu_info);
    }
else {
    /* if (attempt failed due to "too big") */
    if (toobig == TRUE) {
        svc->error_status = tooBig;     /* set PDU error status to "toobig" */
        svc->error_index = 0;           /* set PDU error index to 0         */
        bp->statistics->outtoobig += 1; /* count another "toobig" reply     */

        /* if (attempt to serialize PDU succeeded) */
        if (netio_serialize_pdu(bp, svc, &pdu_info, &toobig) == TRUE) {
            netio_put_pdu(bp, svc, &pdu_info);  /* endeavor to send the pdu */
            }
        else {
            SYSLOG( LOG_ERR, MSG(msg056, "M056 - Unable to serialize PDU: dropped") );
            }
        }
    else {
        SYSLOG( LOG_ERR, MSG(msg057, "M057 - Unable to serialize PDU: dropped") );
        }
    }

svc->svc_active = FALSE;        /* mark service block as INACTIVE */

}

/* casend_process_GET - Process a GET Request to the Common Agent */
/* casend_process_GET - Process a GET Request to the Common Agent */
/* casend_process_GET - Process a GET Request to the Common Agent */

static void
casend_process_GET( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       *svc;   /*-> Service Block (on Service List) to be processed */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that contains the newly
    received and deserialized inbound PDU of type "GET".  The error_status
    for the entire PDU is set to "noError" in the service block upon entry.


OUTPUTS:

    This function performs the conversion of an SNMP "GET" request into
    a series of Common Agent requests, one for each varbind entry in the
    PDU.

    On return, all requests have been issued and all replies received.
    Each varbind entry block (strung off the service block) has been
    loaded with new data returned from the Common Agent, or the error
    status and index in the service block have been set according to
    errors detected.

    The service block and varbind entry blocks contain all the information
    needed to formulate either an error response (the original PDU w/status)
    or the desired valid response (a GET-RESPONSE containing new data).


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_msg() function of SNMP PE. 
        SNMP PE has been successfully received the first (next) inbound
        SNMP message and deserialized it, authenticated it and discovered
        that it is a GET.

    Purpose:
        This function performs the processing necessary to convert
        the inbound GET PDU message into one or more Common Agent requests.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set count of replies_rcvd to 0>

    if (attempt to construct the access control AVL failed)
	<set error status to generr>
	<set error index to varbind block count>
	<return>

    for (every valid varbind entry block on the service block's list)

        <reset reply_error to "noError">

        if (attempt to reset the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to reset AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to point the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to point AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to copy input OID failed)
            <CRASH "Mxxx - Attempt to allocate memory failed: %d">

        if (attempt to obtain class information for an ATTRIBUTE failed)
            <set error status to noSuch>
            <set error index to varbind block count>
            <return>

        if (oid length and ".0" check fails)
            <set error status to noSuch>
            <set error index to varbind block count>
            <return>

        if (attempt to construct the instance AVL failed)
            <set error status to generr>
            <set error index to varbind block count>
            <return>
    
        if (attempt to construct the attribute_list AVL failed)
            <set error status to generr>
            <set error index to varbind block count>
            <return>

        <perform msi_get_attributes call>

        <save status on release of instance AVL>
        <save status on release of attribute_list AVL>
        if (release status from instance AVL or attribute AVL is not SUCCESS)
            <SYSLOG "Mxxx - Error from AVL free, instance %d, attribute %d">

        switch (msi return status)

            case MAN_C_SUCCESS:
                break;

            case MAN_C_INSUFFICIENT_RESOURCES:
            case MAN_C_PROCESSING_FAILURE:
            case MAN_C_BAD_PARAMETER:
            case MAN_C_INVALID_SCOPE:
            case MAN_C_MO_TIMEOUT:
            case MAN_C_MOLD_TIMEOUT:
            case MAN_C_NO_MOLD:
            case MAN_C_PE_TIMEOUT:
            case MAN_C_SYNC_NOT_SUPPORTED:
            default:
                <SYSLOG "Mxxx - anomalous Rtn ("%s") from msi_get_attributes">
                <set error status to generr>
                <set error index to varbind block count>
                <return>

            case MAN_C_NO_SUCH_CLASS:
                <set error status to noSuch>
                <set error index to varbind block count>
                <return>

        (*If we reached here, it was a successful call to msi_get_attributes*)

        (* End of Loop *)

    <release access control AVL>

    (* If we reached here, all varbind entries were converted into *)
    (* successful Common Agent calls, we've gotten replies.        *)

    (* As the replies are received, errors encountered while manipulating   *)
    (* the AVL's or erroneous reply codes (that may require logging) are    *)
    (* logged by the receiving thread.                                      *)

    (* By the time we reach here, it has all been done.                     *)

    (* If the receiving thread encountered any reply requiring a "PUNT",    *)
    (* the error status that should be returned (by this, the sending       *)
    (* thread) is stored in "reply_error" (in the varbind block) by the     *)
    (* receiving thread.                                                    *)

    (* All successful replies have resulted in the "out_entry" AVL being set*)
    (* up with a value to be used in the GET-RESPONSE pdu.                  *)

    (* Swing down the varbind list looking for the first non-"noError" value*)
    (* for "reply_code" and return on it.                                   *)

    for (every valid varbind entry block on the service block's list)
        if (reply_error is not "noError")
            <set error status to reply_error>
            <set error index to varbind block count>
            <return>
        else
            <count another variable requested>
    <return>                                                                   

            
OTHER THINGS TO KNOW:

    Logging of errors is performed as close to where the errors are
    detected as possible, consequently this function simply returns
    after successfully or unsuccessfully processing the varbind list,
    leaving error status in the service block's pdu status cells.

    As mentioned in OUTPUTS section, all replies must have been received
    before this function returns.  This is because it's caller is going
    to release the service block, allowing it's immediate re-use.  This
    could result in problems for the receiving thread when it attempts
    to monkey with this service block.

    The "invoke_id" for the msi call is the value of the vb_entry_id cell
    in the varbind entry block.  The receiving thread scans the service
    list to find a matching service block based on the value of that id
    and then scans the service block's varbind entry list for a matching
    varbind entry based on this invoke_id value.  In V1.0 version of SNMPPE,
    the value of vb_entry_id for each varbind block is set by code in
    "netio_deserialize_pdu()" in "snmppe_netio.c".

    If more than one varbind-block entry were grouped into a Common Agent call,
    then the vb_entry_id would have to be changed in this function before
    the CA call was issued, and the receiving thread logic that "finds" and
    processes entry(s) would need modification too.
*/

/*
| PUNT means "kick back a GET-RESPONSE PDU with an error status and error
| index cells set, using the *original* PDU contents for the varbind list."
| The original contents for each element in the varbind list is kept in
| each varbind entry block's "in_entry" AVL.
|
| (This macro defines shorthand for a fast "error exit" FROM WITHIN THE LOOP
|  that scans the varbind list below.  If used outside of the loop, "index"
|  must be correctly set).
|
| (Maybe if estatus == genErr we should SYSLOG something)
*/
#define PUNT_GET(estatus)                               \
    svc->error_status = estatus;                        \
    svc->error_index = index;                           \
    return;


{
int     index;            /* Index for varbind entry list                    */
avl     *inst_AVL;        /* -> Instance AVL being built for CA call         */
avl     *attr_AVL;        /* -> Attribute List AVL being built for CA call   */
avl     *acc_control_AVL; /* -> Access Control AVL being built for CA call   */
char    msg[LINEBUFSIZE]; /* Error Message build buffer                      */
char    buf[LINEBUFSIZE]; /* Trace Message build buffer                      */
int     ss;               /* "Start String" index for Trace messages         */
int     last_element;     /* AVL Last-Element boolean                        */
int     i_arc;            /* Index to "current" instance arc                 */
int     i_arcs_left;      /* Count of instance arcs remaining                */
int     i;                /* Loop index for copying OID                      */

unsigned int    tag;       /* Tag from AVL                                   */
unsigned int    modifier;  /* Modifier value from AVL                        */
octet_string    *data;     /*-> AVL Octet String structure                   */
varbind_t       *vbe;      /*-> The current Varbind Entry list element       */
man_status      status;    /*   General purpose status holder                */
man_status      inst_st;   /* Status from release of instance AVL            */
man_status      attr_st;   /* Status from release of attribute AVL           */
man_status      acc_st;    /* Status from release of access control AVL      */
bld_IA_status   bstatus;   /* Build Instance AVL status                      */
mir_derive_CI_status
                ci_status; /* MIR Lookup-Class/Instance return code          */


svc->replies_rcvd = 0;  /* set count of replies_rcvd to 0     */

/* if (attempt to construct the access control AVL failed) */
if ((status = casend_bld_access_control_avl(bp,
                                            svc,
                               &acc_control_AVL)) != MAN_C_SUCCESS) {
    PUNT_GET(genErr);
    }

/* for (every valid varbind entry block on the service block's list) */
for (vbe = svc->varbind_list, index = 1;
     vbe != NULL && vbe->vb_entry_id != 0;
     vbe = vbe->next, index++) {

    /* reset reply_error to "noError" */
    vbe->reply_error = noError;

    /* if (reset attempt on "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_reset(vbe->in_entry)) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg058, "M058 - Attempt to reset AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg251, "M251 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        /* set error status to noSuch */
        /* set error index to varbind block count */
        /* return */

        PUNT_GET(genErr);
        }


    /* if (point attempt for "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_point(vbe->in_entry,
                                 &(vbe->orig_oid),/* Varbind entry Object ID */
                                 &modifier,       /* AVL Modifier for entry  */
                                 &tag,            /* ASN.1 Tag for entry     */
                                 &data,           /* Data in AVL octetstring */
                                 &last_element    /* Last Element Flag       */
                                 )) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg059, "M059 - Attempt to point AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg252, "M252 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        PUNT_GET(genErr);
        }

    /* if (attempt to copy input OID failed) */
    vbe->snmp_oid.count = vbe->orig_oid->count;
    vbe->snmp_oid.value = (unsigned int *)
                   malloc(sizeof(unsigned int) * vbe->orig_oid->count);
    if (vbe->snmp_oid.value == NULL) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg253, "M253 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        CRASH(MSG(msg240, "M240 - Attempt to allocate memory failed"));
        }
    for (i=0; i < vbe->orig_oid->count; i++) {     /* Copy OID arcs */
        vbe->snmp_oid.value[i] = vbe->orig_oid->value[i];
        }

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;;    /* Points to text version of Orig OID */

        /* Get "TRACE" in the margin. . . */
        build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

        sprintf(&buf[ss],
                "Begin GET Proc. for Varbind Entry #%d\n", index);
        LOGIT();
        sprintf(&buf[ss],
                "    Original & SNMP OID (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid))
                );
        LOGIT();
        if (oid_string != NULL)
            free(oid_string);
        }

    /*
    | Now we've got our hands on all the data that we need: the SNMP OID and
    | the actual data that we must pass to the Common Agent in the
    | attribute list AVL.  The instance arcs on the tail end of the SNMP
    | OID tell us what to put in the instance AVL, but we need to know
    | where they start in the OID.  For that, we go to the MIR to obtain
    | the OID for the class, and then, given the length of the class OID,
    | we'll know where the instance arcs start in the full SNMP OID.
    | Specifically, they start just after the attribute arc:
    |
    |                   -----class--- A -Instance Arcs
    |                   C.C.C.C.C.C.C.A.I.i.i.i.-->
    |
    | For GET/SET, there MUST be at least one instance arc.
    */
    ci_status = mir_class_inst_GETSET(bp, vbe);

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;  /* Points to text version of Orig OID */

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "MIR GET Processing Ends with status = %s\n",
                CI_status_string[ci_status]);                     LOGIT();

        sprintf(&buf[ss],
                "   Post-MIR processing Varbind Entry OIDs:\n");  LOGIT();
        sprintf(&buf[ss], "   class(%s)\n",
                (oid_string = pe_oid_text(&vbe->class_oid)));     LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   snmp (%s)\n",
                (oid_string = pe_oid_text(&vbe->snmp_oid)));      LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   orig (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid)));       LOGIT();
        free(oid_string);
        }

    /* A lookup for GET *must* find an attribute: "CI_ATTRIBUTE" */
    if (   ci_status == CI_NOSUCH
        || ci_status == CI_CLASS
        || ci_status == CI_ROLLED_CLASS /* <--This should never occur on GET */
        || ci_status == CI_ROLLED_ATTRIBUTE /* <--(likewise never for GET)   */
       ) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg254, "M254 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        PUNT_GET(noSuch);
        }
    else if (ci_status == CI_PROC_FAILURE) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg255, "M255 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        PUNT_GET(genErr);
        }

    /*
    |  OK, we seem to be legal up to the instance arcs (which are present,
    |  but we don't know whether they convert correctly or not.
    |  Build the two AVLs we need to make the msi_get_attributes() call.
    */

    bstatus = casend_bld_instance_avl(bp,
                                      ci_status, /* CI_ATTRIBUTE-Always!*/
                                      vbe,
                                      &inst_AVL,
                                      &i_arc,
                                      &i_arcs_left,
                                      GET);
    IFLOGGING ( L_TRACE ) {

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "Build of Instance AVL Ends with status = %s\n",
                bld_IA_string[ (int) bstatus]);                     LOGIT();
        }
        
    /* if (attempt to construct the instance AVL failed) */
    if (bstatus != FULL_EXACT) {
        /*
        | This conversion must always be successful for GET: no missing or
        | extra arcs, and all arcs used during conversion (ie FULL_EXACT).
        | Otherwise we must dump the instance AVL that gets built and kick
        | back "NoSuch" to the manager.
        */
        if ((inst_st = moss_avl_free(&inst_AVL, TRUE)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg061, "M061 - Error from AVL free: %d"), inst_st);
            SYSLOG( LOG_ERR, msg);
            }

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg256, "M256 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        /*
        | If we're logging OID Anomalies, build a fancy message and give 'em
        | the big scoop on what went wrong.
        */
        IFLOGGING( L_OIDANOMLY ) {
            char        xmsg[LINEBUFSIZE];      /* Build 1st pass msg here */
            sprintf(xmsg,
                MSG(msg174, "M174 - GET(%%s), AVL-bld(%s), /i_arc=%d, i_arcs_left=%d/"),
                    bld_IA_string[ (int) bstatus],
                    i_arc,
                    i_arcs_left
                    );
            /* Stick in "orig_oid" value */
            BLD_OID_ERRMSG( xmsg, vbe->orig_oid );
            LOG(L_OIDANOMLY, msg);
            }
        PUNT_GET(noSuch);
        }


    /* if (attempt to construct the attribute_list AVL failed) */
    if ((status = casend_bld_attribute_avl(bp,
                                           &(vbe->snmp_oid),
                                           &(vbe->class_oid),
                                           ci_status,
                                           tag,
                                           data,
                                           &attr_AVL,
                                           GET)) != MAN_C_SUCCESS) {
        if ((inst_st = moss_avl_free(&inst_AVL, TRUE)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg068, "M068 - Error from AVL free: %d"), inst_st);
            SYSLOG( LOG_ERR, msg);
            }

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg257, "M257 - Error from AVL free: %d"), acc_st);
            SYSLOG( LOG_ERR, msg);
            }

        PUNT_GET(genErr);
        }

    /*
    | NOTE: You can't use the PUNT macro without releasing both AVLs
    |       from this point onward.
    */

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "Calling Common Agent via 'msi_get_attributes()'.\n");
        LOGIT();
        sprintf(&buf[ss],
                "--------------------------------------------\n");
        LOGIT();

        sprintf(&buf[ss],"   INSTANCE AVL:\n");
        LOGIT();
        snmppe_print_avl(inst_AVL, bp->log_state.log_file, buf, (ss+3));
        sprintf(&buf[ss],"   ATTRIBUTE AVL:\n");
        LOGIT();
        snmppe_print_avl(attr_AVL, bp->log_state.log_file, buf, (ss+3));
        /* The "+3" is the # of spaces preceding "INSTANCE" & "ATTRIBUTE" */
        }

    /* perform msi_get_attributes call */
    status = msi_get_attributes(&(vbe->class_oid),   /* Class OID            */
                                inst_AVL,            /*-> Instance AVL       */
                                0,                   /* No scoping info      */
                                nil_filter,          /* no filtering         */
                                acc_control_AVL,     /* access control AVL   */
                                0,                   /* no synch             */
                                attr_AVL,            /*-> Attribute List AVL */
                                vbe->vb_entry_id,    /* CA Invoke ID (PEs)   */
                                &(bp->rpc_callback)  /* RPC callback info    */
                                );

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "--------------------------------------------\n");    LOGIT();
        sprintf(&buf[ss],
                "Return from 'msi_get_attributes()', status = %s\n",
                get_man_status(status));                              LOGIT();
        }

    /* save status on release of instance AVL and attribute_list AVL */
    inst_st = moss_avl_free(&inst_AVL, TRUE);
    attr_st = moss_avl_free(&attr_AVL, TRUE);

    /* if (release status from instance AVL or attribute AVL is not SUCCESS) */
    if (inst_st != MAN_C_SUCCESS || attr_st != MAN_C_SUCCESS) {
        /* SYSLOG "Mxxx - Error from AVL free, instance %d, attribute %d" */
        sprintf(msg,
                MSG(msg060, "M060 - Error from AVL free, instance (%s), attribute (%s)"),
                get_man_status(inst_st), get_man_status(attr_st));
        SYSLOG( LOG_ERR, msg);
        }

    /*
    | NOTE: It's OK to "PUNT" again, the AVL's have been released
    */

    switch (status)  {   /* msi return status */

        case MAN_C_SUCCESS:
            break;

        case MAN_C_INSUFFICIENT_RESOURCES:
        case MAN_C_PROCESSING_FAILURE:
        case MAN_C_BAD_PARAMETER:
        case MAN_C_INVALID_SCOPE:
        case MAN_C_MO_TIMEOUT:
        case MAN_C_MOLD_TIMEOUT:
        case MAN_C_NO_MOLD:
        case MAN_C_PE_TIMEOUT:
        case MAN_C_SYNC_NOT_SUPPORTED:
        default:
            sprintf(msg, MSG(msg062, "M062 - anomalous Rtn (%s) from msi_get_attributes"),
                    get_man_status(status));
            SYSLOG( LOG_ERR, msg);

            if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
		sprintf(msg, MSG(msg258, "M258 - Error from AVL free: %d"),
			acc_st);
		SYSLOG( LOG_ERR, msg);
		}

            PUNT_GET(genErr);


        case MAN_C_NO_SUCH_CLASS:

	    if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
		sprintf(msg, MSG(msg259, "M259 - Error from AVL free: %d"),
			acc_st);
		SYSLOG( LOG_ERR, msg);
		}

            PUNT_GET(noSuch);

        }

    /*
    | If we reached here, it was a successful call to msi_get_attributes.
    | We just processed one varbind entry for the PDU.  Now we walk to
    | the next entry on the varbind list and do it again.
    */

    /* End of For Loop */
    }

/* Release the access control AVL */
if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg260, "M260 - Error from AVL free: %d"), acc_st);
    SYSLOG( LOG_ERR, msg);
    }

/*
| If we reached here, all varbind entries were converted into
| successful Common Agent calls, we've gotten replies.
|
| As the replies are received, errors encountered while manipulating
| the AVL's or erroneous reply codes (that may require logging) are
| logged by the receiving thread.
|
| By the time we reach here, it has all been done, (threaded or not).
|
| All successful replies have resulted in the "out_entry" AVL being set
| up in the varbind entry block with a value to be used in the GET-RESPONSE
| pdu.
|
| If the receiving thread encountered any Common Agent reply requiring a
| "PUNT", the error status that should be returned (by this, the sending
| thread) is stored (by the receiving thread) in "reply_error" (in the varbind
| block).  It is already there!
|
| Swing down the varbind list looking for the first non-"noError" value
| for "reply_code" and return on it, setting error and error index in
| the service block for the PDU.
|
*/

IFLOGGING ( L_TRACE ) {

    /* The Prefix has already been built into the line buffer */
    sprintf(&buf[ss], "Beginning Scan for error return codes. . \n");
    LOGIT();
    }

/* for (every valid varbind entry block on the service block's list) */
for (vbe = svc->varbind_list, index = 1;
     vbe != NULL && vbe->vb_entry_id != 0;
     vbe = vbe->next, index++) {

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "Varbind Entry %d  status = %s\n", index,
                e_status_string[vbe->reply_error]);
        LOGIT();                    
        }

    /* if (reply_error is not "noError") */
    if (vbe->reply_error != noError) {
        PUNT_GET(vbe->reply_error);     /* This PUNT won't wait */
        }
    else {
        /* count another variable requested */
        bp->statistics->totreqvars += 1;
        }
    }
/*
| This return occurs leaving "noError" as the return status in the service
| block for the PDU: this means when the PDU gets sent, we'll use "out_entry"
| AVL in the varbind list as the PDU gets built for return, thereby returning
| any new data left there by the receiving thread.
*/
return;

}

/* casend_process_SET - Process a SET Request to the Common Agent */
/* casend_process_SET - Process a SET Request to the Common Agent */
/* casend_process_SET - Process a SET Request to the Common Agent */

static void
casend_process_SET( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       *svc;   /*-> Service Block (on Service List) to be processed */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that contains the newly
    received and deserialized inbound PDU of type "SET".  The error_status
    for the entire PDU is set to "noError" in the service block upon entry.


OUTPUTS:

    This function performs the conversion of an SNMP "SET" request into
    a series of Common Agent requests, one for each varbind entry in the
    PDU.

    On return, all requests have been issued and all replies received.
    Each varbind entry block (strung off the service block) has been
    loaded with new data returned from the Common Agent, or the error
    status and index in the service block have been set according to
    errors detected.

    The service block and varbind entry blocks contain all the information
    needed to formulate either an error response (the original PDU w/status)
    or the desired valid response (a GET-RESPONSE containing new data).


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_msg() function of SNMP PE. 
        SNMP PE has been successfully received the first (next) inbound
        SNMP message and deserialized it, authenticated it and discovered
        that it is a SET.

    Purpose:
        This function performs the processing necessary to convert
        the inbound SET PDU message into one or more Common Agent requests.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set count of replies_rcvd to 0>

    if (attempt to construct the access control AVL failed)
        <set error status to generr>
        <set error index to varbind block count>
        <return>

    for (every valid varbind entry block on the service block's list)

        <reset reply_error to "noError">

        if (attempt to reset the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to reset AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to point the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to point AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to copy input OID failed)
            <CRASH "Mxxx - Attempt to allocate memory failed: %d">

        if (attempt to obtain class information for an ATTRIBUTE failed)
            <set error status to noSuch>
            <set error index to varbind block count>
            <return>

        if (oid length and ".0" check fails)
            <set error status to noSuch>
            <set error index to varbind block count>
            <return>

        if (attempt to construct the instance AVL failed)
            <set error status to generr>
            <set error index to varbind block count>
            <return>
    
        if (attempt to construct the attribute_list AVL failed)
            <set error status to generr>
            <set error index to varbind block count>
            <return>

        <perform msi_set_attributes call>

        <save status on release of instance AVL>
        <save status on release of attribute_list AVL>
        if (release status from instance AVL or attribute AVL is not SUCCESS)
            <SYSLOG "Mxxx - Error from AVL free, instance %d, attribute %d">

        switch (msi return status)

            case MAN_C_SUCCESS:
                break;

            case MAN_C_BAD_PARAMETER:
            case MAN_C_INVALID_SCOPE:
            case MAN_C_MO_TIMEOUT:
            case MAN_C_MOLD_TIMEOUT:
            case MAN_C_NO_MOLD:
            case MAN_C_PE_TIMEOUT:
            case MAN_C_SYNC_NOT_SUPPORTED:
            case MAN_C_INSUFFICIENT_RESOURCES:
            case MAN_C_PROCESSING_FAILURE:
            default:
                <SYSLOG "Mxxx - anomalous Rtn ("%s") from msi_set_attributes">
                <set error status to generr>
                <set error index to varbind block count>
                <return>

            case MAN_C_NO_SUCH_CLASS:
                <set error status to noSuch>
                <set error index to varbind block count>
                <return>

        (*
         If we reached here, it was a successful call to msi_set_attributes
         and a reply has come in.

         If the receiving thread encountered any reply requiring a "PUNT",
         the error status that should be returned (by this, the sending
         thread) is stored in "reply_error" (in the varbind block) by the
         receiving thread.
        *)
        if (reply_error is not "noError")
            <set error status to reply_error>
            <set error index to varbind block count>
            <return>
        else
            <count another variable requested>

        (* End of Loop *)

    <release access control AVL>

    (*
     If we reached here, all varbind entries were converted into successful
     Common Agent calls and successful replies.  There is nothing more to do.
    *)

    <return>                                                                   

            
OTHER THINGS TO KNOW:

    Processing for SET resembles but is not identical with that of GET.

    For SET, rather than issuing all the msi_set_attribute() calls in a row
    and then checking the reply status, we check after each successful
    SET request issued.

    This allows is to "give-up" as soon as possible in the event of an
    error, since the RFC calls for all variable "setting" to be done as
    simultaneously as possible.  Quitting after the first error is as close
    as we can come to this requirement.

    Logging of errors is performed as close to where the errors are
    detected as possible, consequently this function simply returns
    after successfully or unsuccessfully processing the varbind list,
    leaving error status in the service block's pdu status cells.

    As mentioned in OUTPUTS section, all replies must have been received
    before this function returns.  This is because it's caller is going
    to release the service block, allowing it's immediate re-use.  This
    could result in problems for the receiving thread when it attempts
    to monkey with this service block.

    The "invoke_id" for the msi call is the value of the vb_entry_id cell
    in the varbind entry block.  The receiving thread scans the service
    list to find a matching service block based on the value of that id
    and then scans the service block's varbind entry list for a matching
    varbind entry based on this invoke_id value.  In V1.0 version of SNMPPE,
    the value of vb_entry_id for each varbind block is set by code in
    "netio_deserialize_pdu()" in "snmppe_netio.c".

    If more than one varbind-block entry were grouped into a Common Agent call,
    then the vb_entry_id would have to be changed in this function before
    the CA call was issued, and the receiving thread logic that "finds" and
    processes entry(s) would need modification too.
*/

/*
| PUNT means "kick back a GET-RESPONSE PDU with an error status and error
| index cells set, using the *original* PDU contents for the varbind list."
| The original contents for each element in the varbind list is kept in
| each varbind entry block's "in_entry" AVL.
|
| (This macro defines shorthand for a fast "error exit" FROM WITHIN THE LOOP
|  that scans the varbind list below.  If used outside of the loop, "index"
|  must be correctly set).
|
| (Maybe if estatus == genErr we should SYSLOG something)
*/
#define PUNT_SET(estatus)                               \
    svc->error_status = estatus;                        \
    svc->error_index = index;                           \
    return;

{
int     index;            /* Index for varbind entry list                    */
avl     *inst_AVL;        /* -> Instance AVL being built for CA call         */
avl     *attr_AVL;        /* -> Attribute List AVL being built for CA call   */
avl     *acc_control_AVL; /* -> Access Control AVL being built for CA call   */
char    msg[LINEBUFSIZE]; /* Error Message build buffer                      */
char    buf[LINEBUFSIZE]; /* Trace Message build buffer                      */
int     ss;               /* "Start String" index for Trace messages         */
int     last_element;     /* AVL Last-Element boolean                        */
int     i_arc;            /* Index to "current" instance arc                 */
int     i_arcs_left;      /* Count of instance arcs remaining                */
int     i;                /* Loop index for copying OID                      */

unsigned int    tag;       /* Tag from AVL                                   */
unsigned int    modifier;  /* Modifier value from AVL                        */
octet_string    *data;     /*-> AVL Octet String structure                   */
varbind_t       *vbe;      /*-> The current Varbind Entry list element       */
man_status      status;    /*   General purpose status holder                */
man_status      inst_st;   /* Status from release of instance AVL            */
man_status      attr_st;   /* Status from release of attribute AVL           */
man_status      acc_st;    /* Status from release of access control AVL      */
bld_IA_status   bstatus;   /* Build Instance AVL status                      */
mir_derive_CI_status
                ci_status; /* MIR Lookup-Class/Instance return code          */


svc->replies_rcvd = 0;  /* set count of replies_rcvd to 0     */

/* if (attempt to construct the access control AVL failed) */
if ((status = casend_bld_access_control_avl(bp,
                                            svc,
                               &acc_control_AVL)) != MAN_C_SUCCESS) {
    PUNT_SET(genErr);
    }

/* for (every valid varbind entry block on the service block's list) */
for (vbe = svc->varbind_list, index = 1;
     vbe != NULL && vbe->vb_entry_id != 0;
     vbe = vbe->next, index++) {

    /* reset reply_error to "noError" */
    vbe->reply_error = noError;

    /* if (reset attempt on "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_reset(vbe->in_entry)) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg200, "M200 - Attempt to reset AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg261, "M261 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        /* set error status to noSuch */
        /* set error index to varbind block count */
        /* return */

        PUNT_SET(genErr);
        }


    /* if (point attempt for "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_point(vbe->in_entry,
                                 &(vbe->orig_oid),/* Varbind entry Object ID */
                                 &modifier,       /* AVL Modifier for entry  */
                                 &tag,            /* ASN.1 Tag for entry     */
                                 &data,           /* Data in AVL octetstring */
                                 &last_element    /* Last Element Flag       */
                                 )) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg201, "M201 - Attempt to point AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg262, "M262 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_SET(genErr);
        }

    /* if (attempt to copy input OID failed) */
    vbe->snmp_oid.count = vbe->orig_oid->count;
    vbe->snmp_oid.value = (unsigned int *)
                   malloc(sizeof(unsigned int) * vbe->orig_oid->count);
    if (vbe->snmp_oid.value == NULL) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg263, "M263 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        CRASH(MSG(msg241, "M241 - Attempt to allocate memory failed"));
        }
    for (i=0; i < vbe->orig_oid->count; i++) {     /* Copy OID arcs */
        vbe->snmp_oid.value[i] = vbe->orig_oid->value[i];
        }

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;;    /* Points to text version of Orig OID */

        /* Get "TRACE" in the margin. . . */
        build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

        sprintf(&buf[ss],
                "Begin SET Proc. for Varbind Entry #%d\n", index);
        LOGIT();
        sprintf(&buf[ss],
                "    Original & SNMP OID (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid))
                );
        LOGIT();
        if (oid_string != NULL)
            free(oid_string);
        }

    /*
    | Now we've got our hands on all the data that we need: the SNMP OID and
    | the actual data that we must pass to the Common Agent in the
    | attribute list AVL.  The instance arcs on the tail end of the SNMP
    | OID tell us what to put in the instance AVL, but we need to know
    | where they start in the OID.  For that, we go to the MIR to obtain
    | the OID for the class, and then, given the length of the class OID,
    | we'll know where the instance arcs start in the full SNMP OID.
    | Specifically, they start just after the attribute arc:
    |
    |                   -----class--- A -Instance Arcs
    |                   C.C.C.C.C.C.C.A.I.i.i.i.-->
    |
    | For GET/SET, there MUST be at least one instance arc.
    */
    ci_status = mir_class_inst_GETSET(bp, vbe); 

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;  /* Points to text version of Orig OID */

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "MIR SET Processing Ends with status = %s\n",
                CI_status_string[ci_status]);                     LOGIT();

        sprintf(&buf[ss],
                "   Post-MIR processing Varbind Entry OIDs:\n");  LOGIT();
        sprintf(&buf[ss], "   class(%s)\n",
                (oid_string = pe_oid_text(&vbe->class_oid)));     LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   snmp (%s)\n",
                (oid_string = pe_oid_text(&vbe->snmp_oid)));      LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   orig (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid)));       LOGIT();
        free(oid_string);
        }

    /* A lookup for SET *must* find an attribute: "CI_ATTRIBUTE" */
    if (   ci_status == CI_NOSUCH
        || ci_status == CI_CLASS
        || ci_status == CI_ROLLED_CLASS /* <--This should never occur on SET */
        || ci_status == CI_ROLLED_ATTRIBUTE /* <--(likewise never for SET)   */
       ) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg264, "M264 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_SET(noSuch);
        }
    else if (ci_status == CI_PROC_FAILURE) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg265, "M265 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_SET(genErr);
        }

    /*
    |  OK, we seem to be legal up to the instance arcs (which are present,
    |  but we don't know whether they convert correctly or not.
    |  Build the two AVLs we need to make the msi_set_attributes() call.
    */

    bstatus = casend_bld_instance_avl(bp,
                                      ci_status, /* CI_ATTRIBUTE-Always!*/
                                      vbe,
                                      &inst_AVL,
                                      &i_arc,
                                      &i_arcs_left,
                                      SET);
    IFLOGGING ( L_TRACE ) {

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "Build of Instance AVL Ends with status = %s\n",
                bld_IA_string[ (int) bstatus]);                     LOGIT();
        }
        
    /* if (attempt to construct the instance AVL failed) */
    if (bstatus != FULL_EXACT) {
        /*
        | This conversion must always be successful for SET: no missing or
        | extra arcs, and all arcs used during conversion (ie FULL_EXACT).
        | Otherwise we must dump the instance AVL that gets built and kick
        | back "NoSuch" to the manager.
        */
        if ((inst_st = moss_avl_free(&inst_AVL, TRUE)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg202, "M202 - Error from AVL free: %d"), inst_st);
            SYSLOG( LOG_ERR, msg);
            }

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg266, "M266 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        /*
        | If we're logging OID Anomalies, build a fancy message and give 'em
        | the big scoop on what went wrong.
        */
        IFLOGGING( L_OIDANOMLY ) {
            char        xmsg[LINEBUFSIZE];      /* Build 1st pass msg here */
            sprintf(xmsg,
                MSG(msg203, "M203 - SET(%%s), AVL-bld(%s), /i_arc=%d, i_arcs_left=%d/"),
                    bld_IA_string[ (int) bstatus],
                    i_arc,
                    i_arcs_left
                    );
            /* Stick in "orig_oid" value */
            BLD_OID_ERRMSG( xmsg, vbe->orig_oid );
            LOG(L_OIDANOMLY, msg);
            }
        PUNT_SET(noSuch);
        }


    /* if (attempt to construct the attribute_list AVL failed) */
    if ((status = casend_bld_attribute_avl(bp,
                                           &(vbe->snmp_oid),
                                           &vbe->class_oid,
                                           ci_status,
                                           tag,
                                           data,
                                           &attr_AVL,
                                           SET)) != MAN_C_SUCCESS) {
        if ((inst_st = moss_avl_free(&inst_AVL, TRUE)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg204, "M204 - Error from AVL free: %d"), inst_st);
            SYSLOG( LOG_ERR, msg);
            }

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg267, "M267 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_SET(genErr);
        }

    /*
    | NOTE: You can't use the PUNT macro without releasing both AVLs
    |       from this point onward.
    */

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "Calling Common Agent via 'msi_set_attributes()'.\n");
        LOGIT();
        sprintf(&buf[ss],
                "--------------------------------------------\n");
        LOGIT();

        sprintf(&buf[ss],"   INSTANCE AVL:\n");
        LOGIT();
        snmppe_print_avl(inst_AVL, bp->log_state.log_file, buf, (ss+3));
        sprintf(&buf[ss],"   ATTRIBUTE AVL:\n");
        LOGIT();
        snmppe_print_avl(attr_AVL, bp->log_state.log_file, buf, (ss+3));
        /* The "+3" is the # of spaces preceding "INSTANCE" & "ATTRIBUTE" */
        }

    /* perform msi_set_attributes call */
    status = msi_set_attributes(&vbe->class_oid,     /* Class OID            */
                                inst_AVL,            /*-> Instance AVL       */
                                0,                   /* No scoping info      */
                                nil_filter,          /* no filtering         */
                                acc_control_AVL,     /* access control AVL   */
                                0,                   /* no synch             */
                                attr_AVL,            /*-> Attribute List AVL */
                                vbe->vb_entry_id,    /* CA Invoke ID (PEs)   */
                                &(bp->rpc_callback)  /* RPC callback info    */
                                );

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "--------------------------------------------\n");    LOGIT();
        sprintf(&buf[ss],
                "Return from 'msi_set_attributes()', status = %s\n",
                get_man_status(status));                              LOGIT();
        }

    /* save status on release of instance AVL and attribute_list AVL */
    inst_st = moss_avl_free(&inst_AVL, TRUE);
    attr_st = moss_avl_free(&attr_AVL, TRUE);

    /* if (release status from instance AVL or attribute AVL is not SUCCESS) */
    if (inst_st != MAN_C_SUCCESS || attr_st != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg205, "M205 - Error from AVL free, instance (%s), attribute (%s)"),
                get_man_status(inst_st), get_man_status(attr_st));
        SYSLOG( LOG_ERR, msg);
        }

    /*
    | NOTE: It's OK to "PUNT" again, the AVL's have been released
    */

    switch (status)  {   /* msi return status */

        case MAN_C_SUCCESS:
            break;

        case MAN_C_INVALID_SCOPE:
        case MAN_C_MO_TIMEOUT:
        case MAN_C_MOLD_TIMEOUT:
        case MAN_C_NO_MOLD:
        case MAN_C_PE_TIMEOUT:
        case MAN_C_SYNC_NOT_SUPPORTED:
        case MAN_C_INSUFFICIENT_RESOURCES:
        case MAN_C_PROCESSING_FAILURE:
        default:
            sprintf(msg, MSG(msg206, "M206 - anomalous Rtn (%s) from msi_SET_attributes"),
                    get_man_status(status));
            SYSLOG( LOG_ERR, msg);

            if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
		sprintf(msg, MSG(msg268, "M268 - Error from AVL free: %d"),
			acc_st);
		SYSLOG( LOG_ERR, msg);
		}

            PUNT_SET(genErr);

        case MAN_C_NO_SUCH_CLASS:

	    if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
		sprintf(msg, MSG(msg269, "M269 - Error from AVL free: %d"),
			acc_st);
		SYSLOG( LOG_ERR, msg);
		}

            PUNT_SET(noSuch);
        }

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "Varbind Entry %d  status = %s\n", index,
                e_status_string[vbe->reply_error]);
        LOGIT();                    
        }

    /*
    | If we reached here, it was a successful call to msi_set_attributes
    | so we check it for success before continuing.
    |
    | If the receiving thread encountered any Common Agent reply requiring a
    | "PUNT", the error status that should be returned (by this, the sending
    | thread) is stored (by the receiving thread) in "reply_error" (in the
    | varbind block).  It is already there!
    */
    /* if (reply_error is not "noError") */
    if (vbe->reply_error != noError) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg270, "M270 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_SET(vbe->reply_error);     /* This PUNT won't wait */
        }
    else {
        /* count another variable requested */
        bp->statistics->totsetvars += 1;
        }

    /* End of For Loop */
    }

/* Release the access control AVL */
if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg271, "M271 - Error from AVL free: %d"), acc_st);
    SYSLOG( LOG_ERR, msg);
    }

/*
| If we reached here, all varbind entries were converted into
| successful Common Agent calls and successful replies. There is nothing more
| to do.
|
| This return occurs leaving "noError" as the return status in the service
| block for the PDU: this means when the PDU gets sent, we'll use "out_entry"
| AVL in the varbind list as the PDU gets built for return, thereby returning
| any new data left there by the receiving thread.
*/
return;

}

/* casend_process_GETNEXT - Process a GET-NEXT Request to the Common Agent */
/* casend_process_GETNEXT - Process a GET-NEXT Request to the Common Agent */
/* casend_process_GETNEXT - Process a GET-NEXT Request to the Common Agent */

static void
casend_process_GETNEXT( bp, svc )

big_picture_t   *bp;    /*-> Big Picture Structure for SNMP PE               */
service_t       *svc;   /*-> Service Block (on Service List) to be processed */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a Service Block that contains the newly
    received and deserialized inbound PDU of type "GET-NEXT".  The error_status
    for the entire PDU is set to "noError" in the service block upon entry.


OUTPUTS:

    This function performs the conversion of an SNMP "GET-NEXT" request into
    a series of Common Agent requests, one for each varbind entry in the
    PDU.

    On return, all requests have been issued and all replies received.
    Each varbind entry block (strung off the service block) has been
    loaded with new data returned from the Common Agent, or the error
    status and index in the service block have been set according to
    errors detected.

    The service block and varbind entry blocks contain all the information
    needed to formulate either an error response (the original PDU w/status)
    or the desired valid response (a GET-RESPONSE containing new data).


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_msg() function of SNMP PE. 
        SNMP PE has been successfully received the first (next) inbound
        SNMP message and deserialized it, authenticated it and discovered
        that it is a GET-NEXT.

    Purpose:
        This function performs the processing necessary to convert
        the inbound GET-NEXT PDU message into one or more Common Agent
        requests.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set count of replies_expected to 0>
    <set count of replies_rcvd to 0>

    if (attempt to construct the access control AVL failed)
        <set error status to generr>
        <set error index to varbind block count>
        <return>

    for (every valid varbind entry block on the service block's list)

        <reset reply_error to "noError">

        if (attempt to reset the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to reset AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to point the "in" varbind block AVL was not MAN_C_SUCCESS)
            <SYSLOG "Mxxx - Attempt to point AVL failed: %d">
            <set error status to genErr>
            <set error index to varbind block count>
            <return>

        if (attempt to copy input OID failed)
            <CRASH "Mxxx - Attempt to allocate memory failed: %d">

        if (attempt to obtain class information failed with NOSUCH or GENERR)
            <set error status to noSuch/genErr>
            <set error index to varbind block count>
            <return>

        <attempt to construct the instance AVL>
        iflogging(OIDANOMALY) 

            switch (construction status)

                case FULL_AND_EXTRA_ARCS:
                    <LOG - "Mxxx - OID Anomaly (oid), extra arcs %d %d">
                    <break>

                case FULL_BUT_EXTRA_ARCS:
                    <LOG - "Mxxx - OID Anomaly (oid), Attribute Arc invalid,
                               extra arcs %d %d">
                    <break>

                case ONE_CONVERTED_THEN_JUNK:
                    if (class instance status is CI_CLASS)
                        <LOG - "Mxxx - OID Anomaly (oid), Attribute Arc
                                   invalid, some conversion OK, junk: %d %d">
                    else
                        <LOG - "Mxxx - OID Anomaly (oid), some conversion
                                   OK, junk: %d %d">
                    <break>

                case NONE_CONVERTED_JUNK:
                    if (class instance status is CI_CLASS)
                        <LOG - "Mxxx - OID Anomaly (oid), Attribute Arc
                                   invalid, no conversion OK, junk: %d %d">
                    else
                        <LOG - "Mxxx - OID Anomaly (oid), no conversion
                                   OK, junk: %d %d">
                    <break>

                case DONE_IGNORE_INSTANCE:
                    <LOG - "Mxxx - OID Anomaly (oid), instance arcs ignored %d">
                    <break>

                case NONE_CONVERTED_NONE_PRESENT:
                case NONE_CONVERTED_OK:
                case FULL_EXACT:
                    <break>

    
        if (attempt to construct the attribute_list AVL failed)
            <set error status to generr>
            <set error index to varbind block count>
            <return>

        <perform msi_invoke_action processing, return status>

        <save status on release of instance AVL>
        <save status on release of attribute_list AVL>
        if (release status from instance AVL or attribute AVL is not SUCCESS)
            <SYSLOG "Mxxx - Error from AVL free, instance %d, attribute %d">

        (*
        | This processing consists of issuing however many msi_invoke_action()
        | calls as is necessary to get a status that DOES NOT specify "roll"
        | (ie, we're looking for a no-roll status).
        *)
        <perform processing to get a no-roll status>

        if (signalled status is not "noError")
            <set svc-block error status to signalled status>
            <set error index to varbind block count>
            <return w/error status set>

        (* If we reached here, it was a successful call to msi_invoke_action *)

        (* End of Loop *)

    (*
    | If we reached here, all varbind entries were converted into
    | successful Common Agent calls, we've gotten replies.
    *)

    (*
    |  As the replies are received, errors encountered while manipulating
    |  the AVL's or erroneous reply codes (that may require logging) are
    |  logged by the receiving thread.
    |
    |  By the time we reach here, it has all been done: error logging and/or
    |  getting of the requested values into "out_entry" AVLs in each varbind
    |  entry block.
    |
    |  If the receiving thread encountered any reply requiring a "PUNT",
    |  the error status that should be returned (by this, the sending
    |  thread) is stored in "reply_error" (in the varbind block) by the
    |  receiving thread.
    |
    |  For GET-NEXT, the receiving thread may detect a situation where
    |  a roll-to-the-next-class/attribute operation must be attempted.
    |
    |  In that case, the "reply_error" cell in the varbind block is set to
    |  one of the special SNMP PE internal values "rollattrib" or "rollclass"
    |  by the receiving thread.  This value causes this sending thread code
    |  to re-issue the appropriate msi_invoke_action request with a rolled-to
    |  new class oid.
    |
    |  All successful replies have resulted in the "out_entry" AVL being set
    |  up with a value to be used in the GET-RESPONSE pdu.
    |
    |  Swing down the varbind list looking for the first non-"noError" value
    |  for "reply_code" and process it, either by rolling or by returning it
    |  as the code on which the GET-NEXT request failed.
    *)

    do

        for (every valid varbind entry block on the service block's list OR
             until "roll_again_issued" is TRUE)

            switch (reply_error code)
                case noError:
                    <increment the count of total variables requested>
                    break;

                default:
                    <set error status to reply_error>
                    <set error index to varbind block count>
                    <return>

                case rollclass:
                case rollattrib:
                    <perform processing to get a no-roll status>
                    if (signalled status is "noError")
                        <show roll_again_issued>
                    else
                        <set error status to signalled status>
                        <set error index to varbind block count>
                        <return>
                    break;

        while (no roll-again requested)
                                                                               
    <release access control AVL>

            
OTHER THINGS TO KNOW:

    Logging of errors is performed as close to where the errors are
    detected as possible, consequently this function simply returns
    after successfully or unsuccessfully processing the varbind list,
    leaving error status in the service block's pdu status cells.

    As mentioned in OUTPUTS section, all replies must have been received
    before this function returns.  This is because it's caller is going
    to release the service block, allowing it's immediate re-use.  This
    could result in problems for the receiving thread when it attempts
    to monkey with this service block.

    The "invoke_id" for the msi call is the value of the vb_entry_id cell
    in the varbind entry block.  The receiving thread scans the service
    list to find a matching service block based on the value of that id
    and then scans the service block's varbind entry list for a matching
    varbind entry based on this invoke_id value.  In V1.0 version of SNMPPE,
    the value of vb_entry_id for each varbind block is set by code in
    "netio_deserialize_pdu()" in "snmppe_netio.c".

    If more than one varbind-block entry were grouped into a Common Agent call,
    then the vb_entry_id would have to be changed in this function before
    the CA call was issued, and the receiving thread logic that "finds" and
    processes entry(s) would need modification too.
*/

/*
| PUNT means "kick back a GET-RESPONSE PDU with an error status and error
| index cells set, using the *original* PDU contents for the varbind list."
| The original contents for each element in the varbind list is kept in
| each varbind entry block's "in_entry" AVL.
|
| (This macro defines shorthand for a fast "error exit" FROM WITHIN THE LOOP
|  that scans the varbind list below.  If used outside of the loop, "index"
|  must be correctly set).
|
| (Maybe if estatus == genErr we should SYSLOG something)
*/
#define PUNT_NXT(estatus)                               \
    svc->error_status = estatus;                        \
    svc->error_index = index;                           \
    return;


{
int     index;            /* Index for varbind entry list */
avl     *inst_AVL;        /* -> Instance AVL being built for CA call        */
avl     *attr_AVL;        /* -> Attribute List AVL being built for CA call  */
avl     *acc_control_AVL; /* -> Access Control AVL being built for CA call   */
char    msg[LINEBUFSIZE]; /* Error Message build buffer                     */
char    buf[LINEBUFSIZE]; /* Trace Message build buffer                     */
int     ss;               /* "Start String" index for Trace messages        */
int     last_element;     /* AVL Last-Element boolean                       */
int     i_arc;            /* Index to "current" instance arc                */
int     i_arcs_left;      /* Count of instance arcs remaining               */
BOOL    roll_again_issued;/* TRUE: Re-issued invoke-action, rescan vbe's    */
int     i;                /* Loop index for copying OID                     */

unsigned int    tag;       /* Tag from AVL                                   */
unsigned int    modifier;  /* Modifier value from AVL                        */
octet_string    *data;     /*-> AVL Octet String structure                   */
varbind_t       *vbe;      /*-> The current Varbind Entry list element       */
e_status_t      sstatus;   /* "signalled" status for msi_invoke_action req   */
man_status      status;    /*   General purpose status holder                */
man_status      inst_st;   /* Status from release of instance AVL            */
man_status      attr_st;   /* Status from release of attribute AVL           */
man_status      acc_st;    /* Status from release of access control AVL      */
bld_IA_status   bstatus;   /* Build Instance AVL status                      */
mir_derive_CI_status
                ci_status; /* MIR Lookup-Class/Instance return code          */


svc->replies_rcvd = 0;  /* set count of replies_rcvd to 0     */

/* if (attempt to construct the access control AVL failed) */
if ((status = casend_bld_access_control_avl(bp,
                                            svc,
                               &acc_control_AVL)) != MAN_C_SUCCESS) {
    PUNT_NXT(genErr);
    }

/*
| In this loop we are issuing the FIRST msi_invoke_action() request for
| each varbind entry block off the service block.  If the FIRST request
| fails with a RETURN code that should result in a "roll" to the next class,
| then the second and any required subsequent requests that may be needed
| to get a "good" msi_invoke_action request sent off are ALSO issued within
| this loop.
|
| A second loop (the "do" at the same logical level as this "for") below
| handles the REPLY code generated as a consequence of the request made in
| this first "for" loop.  If that REPLY code specifies that a "roll" to the
| next class is required, then the second loop handles the RETURN code *and*
| the REPLY code for the subsequent msi_invoke_action requests.
|
| for (every valid varbind entry block on the service block's list)
*/
for (vbe = svc->varbind_list, index = 1;
     vbe != NULL && vbe->vb_entry_id != 0;
     vbe = vbe->next, index++) {

    /* reset reply_error to "noError" */
    vbe->reply_error = noError;

    /* if (reset attempt on "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_reset(vbe->in_entry)) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg172, "M172 - Attempt to reset AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg272, "M272 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        /* set error status to noSuch */
        /* set error index to varbind block count */
        /* return */

        PUNT_NXT(genErr);
        }

    /* if (point attempt for "in" varbind block AVL was not MAN_C_SUCCESS) */
    if ((status = moss_avl_point(vbe->in_entry,
                                 &(vbe->orig_oid),/* Varbind entry Object ID */
                                 &modifier,       /* AVL Modifier for entry  */
                                 &tag,            /* ASN.1 Tag for entry     */
                                 &data,           /* Data in AVL octetstring */
                                 &last_element    /* Last Element Flag       */
                                 )) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg173, "M173 - Attempt to point AVL failed (%s)"),
                get_man_status(status));
        SYSLOG( LOG_ERR, msg);

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg273, "M273 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_NXT(genErr);
        }

    /* if (attempt to copy input OID failed) */
    vbe->snmp_oid.count = vbe->orig_oid->count;
    vbe->snmp_oid.value = (unsigned int *)
                   malloc(sizeof(unsigned int) * vbe->orig_oid->count);
    if (vbe->snmp_oid.value == NULL) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg274, "M274 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        CRASH(MSG(msg242, "M242 - Attempt to allocate memory failed"));
        }
    for (i=0; i < vbe->orig_oid->count; i++) {     /* Copy OID arcs */
        vbe->snmp_oid.value[i] = vbe->orig_oid->value[i];
        }

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;;    /* Points to text version of Orig OID */

        /* Get "TRACE" in the margin. . . */
        build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

        sprintf(&buf[ss],
                "Begin GETNEXT Proc. for Varbind Entry #%d\n", index);
        LOGIT();
        sprintf(&buf[ss],
                "    Original & SNMP OID (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid))
                );
        LOGIT();
        if (oid_string != NULL)
            free(oid_string);
        }

    /*
    | Now we've got our hands on all the data that we need: the SNMP OID and
    | the actual data that we must pass to the Common Agent in the
    | attribute list AVL.  The instance arcs on the tail end of the SNMP
    | OID tell us what to put in the instance AVL, but we need to know
    | where they start in the OID.  For that, we go to the MIR to obtain
    | the OID for the class, and then, given the length of the class OID,
    | we'll know where the instance arcs start in the full SNMP OID.
    | Specifically, they start just after the attribute arc:
    |
    |                   -----class--- A -Instance Arcs
    |                   C.C.C.C.C.C.C.A.I.i.i.i.-->
    |
    | For GET-NEXT, there may be no instance arcs, and given the SNMP OID
    | inbound, we've got to 'roll' to the next instance of the attribute
    | specified.  Rolling at the instance level and attribute level is done
    | by the MOM, and rolling at the class level is done by SNMP PE.  Whenever
    | we roll at the class level, any attribute/instance arcs become
    | superfluous.
    */
    ci_status = mir_class_inst_GETNXT(bp, vbe);

    IFLOGGING ( L_TRACE ) {
        char    *oid_string=NULL;  /* Points to text version of Orig OID */

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "MIR GETNEXT Processing Ends with status = %s\n",
                CI_status_string[ci_status]);                     LOGIT();

        sprintf(&buf[ss],
                "   Post-MIR processing Varbind Entry OIDs:\n");  LOGIT();
        sprintf(&buf[ss], "   class(%s)\n",
                (oid_string = pe_oid_text(&vbe->class_oid)));     LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   snmp (%s)\n",
                (oid_string = pe_oid_text(&vbe->snmp_oid)));      LOGIT();
        free(oid_string);
        sprintf(&buf[ss], "   orig (%s)\n",
                (oid_string = pe_oid_text(vbe->orig_oid)));       LOGIT();
        free(oid_string);
        }

    /*
    |  A lookup for GET-NEXT may find an attribute: "CI_ATTRIBUTE"
    |                                      a class: "CI_CLASS"
    |             or it may roll to the next class: "CI_ROLLED_CLASS"
    |         or it may roll to the next attribute: "CI_ROLLED_ATTRIBUTE"
    |                       or it may find nothing: "CI_NOSUCH"
    |
    |  In case of a processing failure, we get "CI_PROC_FAILURE", and catch
    |  that here.
    */
    if (ci_status == CI_NOSUCH) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg275, "M275 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_NXT(noSuch);
        }
    else if (ci_status == CI_PROC_FAILURE) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg276, "M276 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_NXT(genErr);
        }

    /*
    |  OK, we seem to be legal.  Build the two AVLs we need to make the
    |  msi_invoke_action() call.
    */

    /* attempt to construct the instance AVL */
    bstatus = casend_bld_instance_avl(bp,         /* Big Picture             */
                                      ci_status,  /* Class/Inst derive status*/
                                      vbe,        /* has Class/Instance Info */
                                      &inst_AVL,  /* Build AVL here          */
                                      &i_arc,     /* For error logging. . .  */
                                      &i_arcs_left,
                                      GETNEXT);
    IFLOGGING ( L_TRACE ) {

        /* The Prefix has already been built into the line buffer */
        sprintf(&buf[ss],
                "Build of Instance AVL Ends with status = %s\n",
                bld_IA_string[ (int) bstatus]);                     LOGIT();
        }
        
    IFLOGGING( L_OIDANOMLY ) {

        char    xmsg[LINEBUFSIZE]; /* Error Message build buffer            */
        BOOL    do_message=TRUE;   /* TRUE: Message should be issued        */

        switch (bstatus) {
            /* ======================================================= */
            case FULL_AND_EXTRA_ARCS:
                sprintf(xmsg,
                        MSG(msg179, "M179 - OID Anomaly (%%s), extra arcs /i_arc=%d, i_arcs_left=%d/"),
                        i_arc, i_arcs_left);
                break;

            /* ======================================================= */
            case FULL_BUT_EXTRA_ARCS:
                sprintf(xmsg,
                        MSG(msg180, "M180 - OID Anomaly (%%s), Attr Arc invalid, extra arcs /i_arc=%d, i_arcs_left=%d/"),
                        i_arc, i_arcs_left);
                break;

            /* ======================================================= */
            case ONE_CONVERTED_THEN_JUNK:
                /* if (class instance status is CI_CLASS) */
                if (ci_status == CI_CLASS)
                {
                    sprintf(xmsg,
                            MSG(msg181, "M181 - OID Anomaly <oid>, Attr Arc bad, some conv. OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                            i_arc, i_arcs_left);
                }
                else
                {
                    sprintf(xmsg,
                            MSG(msg182, "M182 - OID Anomaly <oid>, some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                            i_arc, i_arcs_left);
                }
                break;

            /* ======================================================= */
            case NONE_CONVERTED_JUNK:
                /* if (class instance status is CI_CLASS) */
                if (ci_status == CI_CLASS)
                    sprintf(xmsg,
                            MSG(msg183, "M183 - OID Anomaly (%%s), Attr Arc invalid, no conv. OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                            i_arc, i_arcs_left);
                else
                    sprintf(xmsg,
                            MSG(msg184, "M184 - OID Anomaly (%%s), no conv OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                            i_arc, i_arcs_left);
                break;

            /* ======================================================= */
            case DONE_IGNORE_INSTANCE:
                sprintf(xmsg,
                        MSG(msg295, "M295 - OID Anomaly (%%s), instance arcs ignored/i_arc=%d, i_arcs_left=%d/"),
                        i_arc, i_arcs_left);
                break;

            /* ======================================================= */
            case NONE_CONVERTED_NONE_PRESENT:
            case NONE_CONVERTED_OK:
            case FULL_EXACT:
                do_message = FALSE;
                break;
            }

        if (do_message == TRUE) {
            BLD_OID_ERRMSG(xmsg, vbe->orig_oid);
            LOG(L_OIDANOMLY, msg);
            }
        }

    /* if (attempt to construct the attribute_list AVL failed) */
    if ((status = casend_bld_attribute_avl(bp,
                                           &(vbe->snmp_oid),
                                           &vbe->class_oid,
                                           ci_status,
                                           tag,
                                           data,
                                           &attr_AVL,
                                           GETNEXT)) != MAN_C_SUCCESS) {
        if ((inst_st = moss_avl_free(&inst_AVL, TRUE)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg185, "M185 - Error from AVL free: %d"), inst_st);
            SYSLOG( LOG_ERR, msg);
            }

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg277, "M277 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_NXT(genErr);
        }

    /*
    | NOTE: You can't use the PUNT macro without releasing both AVLs
    |       from this point onward.
    */

    /* perform msi_invoke_action processing, return status */
    sstatus = casend_do_invoke_action( bp, vbe, inst_AVL, attr_AVL,
				       acc_control_AVL);

    /* save status on release of instance AVL and attribute_list AVL */
    inst_st = moss_avl_free(&inst_AVL, TRUE);
    attr_st = moss_avl_free(&attr_AVL, TRUE);

    /* if (release status from instance AVL or attribute AVL is not SUCCESS) */
    if (inst_st != MAN_C_SUCCESS || attr_st != MAN_C_SUCCESS) {
        /* SYSLOG "Mxxx - Error from AVL free, instance %d, attribute %d" */
        sprintf(msg, MSG(msg186, "M186 - Error from AVL free, instance(%s), attribute(%s)"),
                get_man_status(inst_st), get_man_status(attr_st));
        SYSLOG( LOG_ERR, msg);
        }

    /*
    | NOTE: It's OK to "PUNT" again, the AVL's have been released
    */

    /*
    | This processing consists of issuing however many msi_invoke_action()
    | calls as is necessary to get a status that DOES NOT specify "roll"
    | (ie, we're looking for a no-roll status given the current status).
    |
    | perform processing to get a no-roll status
    */
    sstatus = casend_get_noroll_status(bp, sstatus, vbe, acc_control_AVL);

    /* if (signalled status is not "noError") */
    if (sstatus != noError) {

        if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
	    sprintf(msg, MSG(msg278, "M278 - Error from AVL free: %d"), acc_st);
	    SYSLOG( LOG_ERR, msg);
	    }

        PUNT_NXT(sstatus);
        }

    /*
    | If we reached here, it was a successful call to msi_invoke_action
    | We just processed one varbind entry for the PDU.  Now we walk to
    | the next entry on the varbind list and do it again.
    */

    /* End of For Loop */
    }


/*
| If we reached here, all varbind entries were converted into
| successful Common Agent calls, (that is, the RETURN codes were "noError". .) 
| we've gotten replies.
|
| ============================================================================
|
|  As the replies are received, errors encountered while manipulating
|  the AVL's or erroneous reply codes (that may require logging) are
|  logged by the receiving thread.
|
|  By the time we reach here, it has all been done: error logging and/or
|  getting of the requested values into "out_entry" AVLs in EACH varbind
|  entry block.
|
|  If the receiving thread encountered any reply requiring a "PUNT",
|  the error status that should be returned (by this, the sending
|  thread) is stored in "reply_error" (in the varbind block) by the
|  receiving thread.
|
|  For GET-NEXT, the receiving thread may detect a situation where
|  a roll-to-the-next-class/attribute operation must be attempted.
|
|  In that case, the "reply_error" cell in the varbind block is set to
|  one of the special SNMP PE internal values "rollattrib" or "rollclass"
|  by the receiving thread.  This value causes this sending thread code
|  to re-issue the appropriate msi_invoke_action request with a rolled-to
|  new class oid.
|
|  All successful replies have resulted in the "out_entry" AVL being set
|  up with a value to be used in the GET-RESPONSE pdu.
|
|  Swing down the varbind list looking for the first non-"noError" value
|  for "reply_code" and process it, either by rolling or by returning it
|  as the code on which the GET-NEXT request failed.
*/

IFLOGGING ( L_TRACE ) {

    /* The Prefix has already been built into the line buffer */
    sprintf(&buf[ss], "Beginning Scan for error return codes. . \n");
    LOGIT();
    }

do {    /* Do this as long as there is a vbe unprocessed */
        /*       (As long as we're still "rolling")      */

    /*
    | for (every valid varbind entry block on the service block's list
    |      OR until we hit a REPLY code that forced a roll to the next class
    |         and another invoke-action request to be issued)
    */
    for ( vbe = svc->varbind_list, index = 1, roll_again_issued = FALSE;

         /* (keep looping down the varbind list entries (vbe's) if: */
         roll_again_issued == FALSE &&  /* We've NOT hit a roll-request AND */
         vbe != NULL &&                 /* There is another vbe AND         */
         vbe->vb_entry_id != 0;         /* It's not marked as "Not In Use"  */

         vbe = vbe->next, index++) {

        IFLOGGING( L_TRACE ) {
            sprintf(&buf[ss],
                    "Varbind Entry %d  status = %s\n",
                    index, e_status_string[sstatus]);
            LOGIT();                    
            }

        /*
        |  Examine the REPLY code returned by the Common Agent.
        |  If we have to roll, do it and start the scan
        |     of vbe's all over again.
        |  If something went wrong, give-up the get-next immediately. 
        |  Otherwise, we'll fall out the bottom w/all requests successfully
        |     handled.
        */
        switch (vbe->reply_error) {
            case noError:
                /* increment the count of total variables requested */
                bp->statistics->totreqvars += 1;
                break;

            default:
                PUNT_NXT(vbe->reply_error);

            case rollclass:
            case rollattrib:
                /* perform processing to get a no-roll status */
                sstatus = casend_get_noroll_status (bp, vbe->reply_error, vbe,
						    acc_control_AVL);

                /* if (signalled status is "noError") */
                if (sstatus == noError) {
                    roll_again_issued = TRUE;  /* force a wait & re-scan of */
                    }                          /* all vbe's                 */
                else {
                    vbe->reply_error = sstatus;
                    PUNT_NXT(vbe->reply_error);
                    }
                break;

            }   /* switch */
        }    /* for    */
    }     /* all vbe's not-yet-processed */
    while (roll_again_issued == TRUE); 

/* Release the access control AVL */
if ((acc_st = moss_avl_free(&acc_control_AVL, TRUE)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg279, "M279 - Error from AVL free: %d"), acc_st);
    SYSLOG( LOG_ERR, msg);
    }

/*
| This return occurs leaving "noError" as the return status in the service
| block for the PDU: this means when the PDU gets sent, we'll use "out_entry"
| AVL in the varbind list as the PDU gets built for return, thereby returning
| any new data left there by the receiving thread.
*/
return;

}

/* casend_get_noroll_status - Roll/Invoke-Action until no Roll status rtned  */
/* casend_get_noroll_status - Roll/Invoke-Action until no Roll status rtned  */
/* casend_get_noroll_status - Roll/Invoke-Action until no Roll status rtned  */

static e_status_t
casend_get_noroll_status( bp, in_status, vbe, acc_control_AVL)

big_picture_t   *bp;       /*-> Big Picture Structure for SNMP PE            */
e_status_t      in_status; /* Incoming Get-Next status for evaluation        */
varbind_t       *vbe;      /*-> The current Varbind Entry list element       */
avl             *acc_control_AVL; /*-> Access Control AVL built for CA call  */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "in_status" is the "signalled-status" of the last Get-Next (invoke-action)
    request just issued.

    "vbe" is the address of the varbind entry block that contains the
    object class and invoke id to be used in the invoke action request.


OUTPUTS:

    This function returns the status from the 'last' invoke-action request
    made to the Common Agent that DID NOT return the SNMP PE internal code
    'rollclass' or 'rollattrib'.  (In other words, it will roll-and-issue
    invoke-action requests until it gets a RETURN code that does NOT indicate
    a need to roll again).


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_GETNEXT() function of SNMP PE. 
        SNMP PE has successfully received the first (next) inbound
        SNMP message and deserialized it, authenticated it and discovered
        that it is a GET-NEXT.  _GETNEXT is processing each varbind entry
        in the request AND AT LEAST ONE ATTEMPT to do an invoke-action
        to the Common Agent has already been made.

    Purpose:
        This function performs the looping and repeated invoke-action requests
        to the Common Agent to process the RETURN code from earlier requests
        that specified "roll to the next class".

ACTION SYNOPSIS OR PSEUDOCODE:

    <show keep_rolling to be TRUE>
    <copy input status to "signalled" status>

    do
        switch (last Get-Next invoke-action status)

            case rollclass:
            case rollattrib:
                <perform class-instance derivation on NEXT class>
                switch (class-instance status)

                    case CI_ROLLED_CLASS:
                    case CI_ROLLED_ATTRIBUTE:
                        <create empty instance and attribute AVL>
                        <attempt to construct instance AVL with class hierarchy>
                        if (CI_ROLLED_ATTRIBUTE)
                            <load attribute AVL with just attribute OID>
                        <Do invoke-action, giving new Get-Next status>
                        <release AVLs>
                        <break>

                    case CI_NOSUCH:
                        <signal get-next status as "noSuch">
                        <show keep_rolling as FALSE>
                        <break>

                    case CI_PROC_FAILURE:
                        <signal get-next status as "genErr">
                        <show keep_rolling as FALSE>
                        <break>
                <break>

            case genErr:
            case noSuch:
            case noError:
            default:
                <show keep_rolling as FALSE>
                <break>

        while (keep_rolling is TRUE)

    <return the signalled get-next status code>


OTHER THINGS TO KNOW:

*/

{
char     buf[LINEBUFSIZE]; /* Trace Message build buffer                     */
int      ss;               /* "Start String" index for Trace messages        */
char    msg[LINEBUFSIZE];  /* Error Message build buffer                     */
BOOL        keep_rolling;  /* TRUE: Roll the class and re-issue the request  */
e_status_t  sstatus;       /* "signalled" status for msi_invoke_action req   */
avl        *inst_AVL=NULL; /* -> Instance AVL being built for CA call        */
avl        *attr_AVL=NULL; /* -> Attribute List AVL being built for CA call  */
man_status  status;        /* General purpose status holder                  */
man_status  inst_st;       /* Status from release of instance AVL            */
man_status  attr_st;       /* Status from release of attribute AVL           */
mir_derive_CI_status
                ci_status; /* MIR Lookup-Class/Instance return code          */
bld_IA_status bstatus;     /* Build Instance AVL status                      */
int         i_arc;         /* Index to "current" instance arc                */
int         i_arcs_left;   /* Count of instance arcs remaining               */


IFLOGGING( L_TRACE ) {

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "'casend_get_noroll_status()' has been entered.\n");
    LOGIT();
    }

keep_rolling = TRUE;        /* Loop 'till we come back "not"-roll" */
sstatus = in_status;        /* copy input status to "signalled" status */

do {    /* Roll 'til we don't have to */

    switch (sstatus) {

        /*
        |  In this case, either we've been passed a status requiring a roll
        |  or an earlier iteration of the outer "do" resulted in a return code
        |  requiring (another!) roll.
        */
        case rollclass:
        case rollattrib:
            /* perform class-instance derivation on NEXT class or attribute */
            ci_status = mir_class_inst_ROLLNXT(bp, sstatus, vbe);

            IFLOGGING ( L_TRACE ) {

                /* Points to text version of Orig OID */
                char    *oid_string=NULL;

                /* The Prefix has already been built into the line buffer */
                sprintf(&buf[ss],
                        "MIR ROLLNEXT Processing Ends with status = %s\n",
                        CI_status_string[ci_status]);                 LOGIT();

                sprintf(&buf[ss],
                     "   Post-MIR processing Varbind Entry OIDs:\n"); LOGIT();
                sprintf(&buf[ss], "   class(%s)\n",
                        (oid_string = pe_oid_text(&vbe->class_oid))); LOGIT();
                free(oid_string);
                sprintf(&buf[ss], "   snmp (%s)\n",
                        (oid_string = pe_oid_text(&vbe->snmp_oid)));  LOGIT();
                free(oid_string);
                sprintf(&buf[ss], "   orig (%s)\n",
                        (oid_string = pe_oid_text(vbe->orig_oid)));   LOGIT();
                free(oid_string);
                }

            switch (ci_status) {

                case CI_ROLLED_CLASS:
                case CI_ROLLED_ATTRIBUTE:
                    /* if (attempt to init the Attribute AVL failed) */
                    if ((status = moss_avl_init(&attr_AVL)) != MAN_C_SUCCESS) {
                        sprintf(msg,
                                MSG(msg191, "M191 - Attribute AVL init failed (%s)"),
                                get_man_status(status));
                        CRASH(msg);
                        }

                    /* attempt to construct an instance AVL with just the class hierarchy */
                    bstatus = casend_bld_instance_avl(bp,     /* Big Picture  */
                                      ci_status,  /* Class/Inst derive status */
                                      vbe,        /* has Class/Instance Info  */
                                      &inst_AVL,  /* Build AVL here           */
                                      &i_arc,     /* For error logging. . .   */
                                      &i_arcs_left,
                                      GETNEXT);
                    IFLOGGING ( L_TRACE ) {

                        /* The Prefix has already been built into the line buffer */
                        sprintf(&buf[ss],
                                "Build of Instance AVL Ends with status = %s\n",
                                bld_IA_string[ (int) bstatus]);  LOGIT();
                        }
        
                    IFLOGGING( L_OIDANOMLY ) {

                        char    xmsg[LINEBUFSIZE]; /* Error Message build buffer     */
                        BOOL    do_message=TRUE;   /* TRUE: Message should be issued */

                        switch (bstatus) {
                            /* ======================================================= */
                            case FULL_AND_EXTRA_ARCS:
                                sprintf(xmsg,
                                        MSG(msg294, "M294 - OID Anomaly (%%s), extra arcs /i_arc=%d, i_arcs_left=%d/"),
                                        i_arc, i_arcs_left);
                                break;

                            /* ======================================================= */
                            case FULL_BUT_EXTRA_ARCS:
                                sprintf(xmsg,
                                        MSG(msg288, "M288 - OID Anomaly (%%s), Attr Arc invalid, extra arcs /i_arc=%d, i_arcs_left=%d/"),
                                        i_arc, i_arcs_left);
                                break;

                            /* ======================================================= */
                            case ONE_CONVERTED_THEN_JUNK:
                                /* if (class instance status is CI_CLASS) */
                                if (ci_status == CI_CLASS)
                                {
                                    sprintf(xmsg,
                                            MSG(msg289, "M289 - OID Anomaly <oid>, Attr Arc bad, some conv. OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                                            i_arc, i_arcs_left);
                                }
                                else
                                {
                                    sprintf(xmsg,
                                            MSG(msg290, "M290 - OID Anomaly <oid>, some conversion OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                                            i_arc, i_arcs_left);
                                }
                                break;

                            /* ======================================================= */
                            case NONE_CONVERTED_JUNK:
                                /* if (class instance status is CI_CLASS) */
                                if (ci_status == CI_CLASS)
                                    sprintf(xmsg,
                                            MSG(msg291, "M291 - OID Anomaly (%%s), Attr Arc invalid, no conv. OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                                            i_arc, i_arcs_left);
                                else
                                    sprintf(xmsg,
                                            MSG(msg292, "M292 - OID Anomaly (%%s), no conv OK, junk:/i_arc=%d, i_arcs_left=%d/"),
                                            i_arc, i_arcs_left);
                                break;

                            /* ======================================================= */
                            case DONE_IGNORE_INSTANCE:
                                sprintf(xmsg,
                                        MSG(msg296, "M296 - OID Anomaly (%%s), instance arcs ignored/i_arc=%d, i_arcs_left=%d/"),
                                        i_arc, i_arcs_left);
                                break;

                            /* ======================================================= */
                            case NONE_CONVERTED_NONE_PRESENT:
                            case NONE_CONVERTED_OK:
                            case FULL_EXACT:
                                do_message = FALSE;
                                break;
                            }

                        if (do_message == TRUE) {
                            BLD_OID_ERRMSG(xmsg, vbe->orig_oid);
                            LOG(L_OIDANOMLY, msg);
                            }
                        }

                    /*
                    |  If we actually rolled forward to another attribute,
                    |  it's OID must be put into the Attribute AVL so the MOM
                    |  knows about it.
                    */
                    if (ci_status == CI_ROLLED_ATTRIBUTE) {
                        /* load attribute AVL with just attribute OID */
                        if ((status = moss_avl_add(attr_AVL,
                                                   &(vbe->snmp_oid),
                                                   MAN_C_SUCCESS, NULL, NULL))
                            != MAN_C_SUCCESS) {
                            sprintf(msg, MSG(msg238, "M238 - AVL Add failed (%s)"),
                                    get_man_status(status));
                            CRASH( msg );
                            }                        
                        }

                    sstatus =   /* Issue the next invoke-action w/new class */
                        casend_do_invoke_action( bp, vbe, inst_AVL, attr_AVL,
						 acc_control_AVL);

                    /* Release the AVLs */
                    inst_st = moss_avl_free(&inst_AVL, TRUE); /* Dump AVLs */
                    attr_st = moss_avl_free(&attr_AVL, TRUE);
                    if (inst_st != MAN_C_SUCCESS || attr_st != MAN_C_SUCCESS) {
                        sprintf(msg,
                    MSG(msg192, "M192 - Error from AVL free, instance(%s), attribute(%s)"),
                                get_man_status(inst_st),
                                get_man_status(attr_st));
                        SYSLOG( LOG_ERR, msg);}
                    break;

                case CI_NOSUCH:
                    keep_rolling = FALSE;
                    sstatus = noSuch;
                    break;

                case CI_PROC_FAILURE:
                default:
                    keep_rolling = FALSE;
                    sstatus = genErr;
                    break;
                }
             break;


        /*
        |  In these cases, we just return whatever the last value of the
        |  get-next invoke-action request was, since there's no need to roll
        */
        case genErr:
        case noSuch:
        case noError:
        default:
            keep_rolling = FALSE;
            break;
        }
    }
    while (keep_rolling == TRUE);

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "Returning from 'casend_get_noroll_status()' with status = %s\n",
            e_status_string[sstatus]);                              LOGIT();
    }
    
/* return the signalled get-next status code */
return (sstatus);

}

/* casend_do_invoke_action - Issue an Invoke-Action Request to Common Agent */
/* casend_do_invoke_action - Issue an Invoke-Action Request to Common Agent */
/* casend_do_invoke_action - Issue an Invoke-Action Request to Common Agent */

static e_status_t
casend_do_invoke_action( bp, vbe, inst_AVL, attr_AVL, acc_control_AVL)

big_picture_t   *bp;       /*-> Big Picture Structure for SNMP PE            */
varbind_t       *vbe;      /*-> The current Varbind Entry list element       */
avl             *inst_AVL; /* -> Instance AVL being built for CA call        */
avl             *attr_AVL; /* -> Attribute List AVL being built for CA call  */
avl             *acc_control_AVL; /*-> Access Control AVL being built for CA */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "vbe" is the address of the varbind entry block that contains the
    object class and invoke id to be used in the invoke action request.

    "inst_AVL" points to the AVL to be used as the instance AVL in the
    invoke-action request.

    "attr_AVL" points to the AVL to be used as the attribute AVL in the
    invoke-action request.


OUTPUTS:

    This function issues the Common Agent invoke action request, handles the
    RETURN code from this function call and maps it into an "e_status_t" value
    returned by this function.  Any logging required as a consequence of
    receiving certain RETURN codes from the invoke action request is performed
    before returning.


BIRD'S EYE VIEW:
    Context:
        The caller is the casend_process_GETNEXT() function of SNMP PE. 
        SNMP PE has successfully received the first (next) inbound
        SNMP message and deserialized it, authenticated it and discovered
        that it is a GET-NEXT.  _GETNEXT is processing each varbind entry
        in the request.

    Purpose:
        This function performs the invoke-action request to the Common Agent
        and processes the RETURN code from that request.  The mapping of
        the various RETURN codes to one of "generr", "rollattrib", "rollclass"
        or "success" is the main purpose of this code.


ACTION SYNOPSIS OR PSEUDOCODE:

    <perform msi_invoke_action processing>

    switch (msi return status)

        (* REPLY IS EXPECTED *)
        case MAN_C_SUCCESS:
            <set return code to "noError">
            break;

        (* LOG AND ROLL TO NEXT ATTRIBUTE*)
        case MAN_C_PROCESSING_FAILURE:
            <SYSLOG "Mxxx - (%s) return from (oid) invoke-action: rolling>

        (* ROLL TO NEXT ATTRIBUTE*)
        case MAN_C_NO_SUCH_OBJECT_INSTANCE:
        case MAN_C_NO_SUCH_ATTRIBUTE_ID:
            <set return code to "rollattrib">
            break;

        (* ROLL TO NEXT CLASS*)
        case MAN_C_NO_SUCH_CLASS:
            <set return code to "rollclass">
            break;

        (* LOG AND GENERR *)
        case MAN_C_BAD_PARAMETER:
        case MAN_C_SYNC_NOT_SUPPORTED:
        case MAN_C_INVALID_SCOPE:
        case MAN_C_MO_TIMEOUT:
        case MAN_C_MOLD_TIMEOUT:
        case MAN_C_NO_MOLD:
        case MAN_C_PE_TIMEOUT:
        case MAN_C_INSUFFICIENT_RESOURCES:
        default:
            <set return code to generr>
            <SYSLOG
              "Mxxx - anomalous Rtn (%s) from (oid) invoke-action: generr">
            <break>

    <return the return code>


OTHER THINGS TO KNOW:

*/

{
char     msg[LINEBUFSIZE]; /* Error Message build buffer                     */
char     buf[LINEBUFSIZE]; /* Trace Message build buffer                     */
int      ss;               /* "Start String" index for Trace messages        */
man_status      status;    /*   General purpose status holder                */
e_status_t      sstatus;   /* "signalled" status for msi_invoke_action req   */

/* Magic OID that means "do a get-next" when passed as invoke-action OID */
static unsigned int      
                value_list[10] = { 1, 3, 12, 2, 1011, 2, 3, 1, 126, 5 } ; 

static object_id 
                magic_getnext_oid = { 10, value_list } ;


IFLOGGING( L_TRACE ) {

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "'casend_do_invoke_action()' has been entered. . .calling \n");
    LOGIT();
    sprintf(&buf[ss],
            "Common Agent via 'msi_invoke_action()'.\n");
    LOGIT();
    sprintf(&buf[ss],
            "--------------------------------------------\n");
    LOGIT();

    sprintf(&buf[ss],"   INSTANCE AVL:\n");
    LOGIT();
    snmppe_print_avl(inst_AVL, bp->log_state.log_file, buf, (ss+3));
    sprintf(&buf[ss],"   ATTRIBUTE AVL:\n");
    LOGIT();
    snmppe_print_avl(attr_AVL, bp->log_state.log_file, buf, (ss+3));
    /* The "+3" is the # of spaces preceding "INSTANCE" & "ATTRIBUTE" */
    }

/* perform msi_invoke_action processing */
status = msi_invoke_action(&vbe->class_oid,     /* Class OID                 */
                           inst_AVL,            /* Instance AVL              */
                           0,
                           nil_filter, 
                           acc_control_AVL,     /* Access Control AVL        */
                           0,
                           &magic_getnext_oid,  /* "Get-Next" Invoke Action  */
                           attr_AVL,            /* attribute list AVL        */
                           vbe->vb_entry_id,    /* Invoke ID                 */
                           &(bp->rpc_callback)  /* RPC interface for callback*/
                           );

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "--------------------------------------------\n");    LOGIT();
    sprintf(&buf[ss],
            "Return from 'msi_invoke_action()', status = %s\n",
            get_man_status(status));                              LOGIT();
    }

switch (status) {

    /* REPLY IS EXPECTED */
    case MAN_C_SUCCESS:
        sstatus = noError;      /* set return code to "noError" */
        break;

    /* LOG AND ROLL TO NEXT ATTRIBUTE*/
    case MAN_C_PROCESSING_FAILURE:
        {
        char xmsg[LINEBUFSIZE];         /* Error Message build buffer */

        sprintf(xmsg,
                MSG(msg187, "M187 - (%d)=(%s) return from (%%s) invoke-action: rolling"),
                (int) status, get_man_status(status));
        BLD_OID_ERRMSG(xmsg, &vbe->snmp_oid);
        SYSLOG(LOG_ERR, msg);
        }
        /* ===== FALL THRU TO ROLL ===== */

    /* ROLL TO NEXT ATTRIBUTE*/
    case MAN_C_NO_SUCH_OBJECT_INSTANCE:
    case MAN_C_NO_SUCH_ATTRIBUTE_ID:
        sstatus = rollattrib;    /* set return code to "rollattrib" */
        break;

    /* ROLL TO NEXT CLASS*/
    case MAN_C_NO_SUCH_CLASS:
        sstatus = rollclass;    /* set return code to "rollclass" */
        break;

    /* LOG AND GENERR */
    case MAN_C_BAD_PARAMETER:
    case MAN_C_SYNC_NOT_SUPPORTED:
    case MAN_C_INVALID_SCOPE:
    case MAN_C_MO_TIMEOUT:
    case MAN_C_MOLD_TIMEOUT:
    case MAN_C_NO_MOLD:
    case MAN_C_PE_TIMEOUT:
    case MAN_C_INSUFFICIENT_RESOURCES:
    default:
        /* SYSLOG "Mxxx - (%s) return from (oid) invoke-action: generr" */
        {
        char xmsg[LINEBUFSIZE];         /* Error Message build buffer */

        sprintf(xmsg,
                MSG(msg188, "M188 - (%d)=(%s) return from (%%s) invoke-action: generr"),
                (int) status, get_man_status(status));
        BLD_OID_ERRMSG(xmsg, &vbe->snmp_oid);
        SYSLOG(LOG_ERR, msg);
        }

        sstatus = genErr;       /* set return code to generr */
        break;
    }

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "Returning from 'casend_do_invoke_action()' with status = %s\n",
            e_status_string[sstatus]);                              LOGIT();
    }
    
/* return the return code */
return (sstatus);

}

/* casend_bld_instance_avl - Build the Instance AVL */
/* casend_bld_instance_avl - Build the Instance AVL */
/* casend_bld_instance_avl - Build the Instance AVL */

static bld_IA_status
casend_bld_instance_avl( bp,
                        dstatus,        /* IN     */
                        vbe,            /* IN     */
                        p_inst_AVL,     /* IN/OUT */
                        i_arc,          /* IN/OUT */
                        i_arcs_left,    /* IN/OUT */
                        pdu)            /* IN     */

big_picture_t         *bp;           /*-> Big Picture Structure for SNMP PE  */
mir_derive_CI_status  dstatus;       /* Class/Instance derivation status     */
varbind_t             *vbe;          /*-> Varbind Entry Block for SNMP OID   */
avl                   **p_inst_AVL;  /*->> Pointer to where we build AVL     */
int                   *i_arc;        /* Index to "current" instance arc      */
int                   *i_arcs_left;  /* Count of instance arcs remaining     */
pdu_type              pdu;           /* PDU type (GET, SET, GETNEXT)         */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "vbe->snmp_oid" is the object id of the SNMP variable we're working on
    for this request.

    "dstatus" is the Class/Instance derivation status as returned by the
    MIR lookup logic.  This indicates what the original SNMP "boiled down to"
    when submitted to the MIR.  At this point the only valid values are:

      "CI_ATTRIBUTE" - Original SNMP OID selected a MIR object that was an
                   attribute that was contained within a valid class.  Any
                   instance arcs present need to be converted into the
                   instance AVL.

      "CI_CLASS" - Original SNMP OID selected a MIR object that was a class
                   (but not an attribute within that class) and the "class_oid"
                   in "vbe" is a subset of the original SNMP oid.  Any
                   attribute arc present is bogus (according to the MIR) but
                   any instance arcs present after any attribute arc should be
                   converted into the instance AVL nonetheless.

      "CI_ROLLED_ATTRIBUTE"
      "CI_ROLLED_CLASS" - Original SNMP OID did not exactly select an object
                   that was a class or attribute and a roll occurred.  This
                   implies that any attribute and instance arcs present in
                   the original SNMP oid are now useless.  However, an instance
                   AVL with the class hierarchy is created.

    The other possible values for the MIR Derived status should have been
    already handled by the caller of this function and result in a CRASH if
    present in the call.

    "vbe" is the address of a Varbind Entry Block that corresponds
    to the "SNMP" OID we're processing.  Referenced in this data structure
    are:
        * The Original SNMP OID we're working on
        * The class oid and
        * instance information

    "p_inst_AVL" is the AVL we're being asked to build for transmission
    to the Common Agent.  The pointer whose address is passed for this
    AVL should not be initialized to anything, as it will be set to NULL
    before the AVL is created.

    "i_arc" and "i_arcs_left" are integer values whose addresses are passed
    in so that in the case where an instance arc conversion fails, these
    values are returned to assist maintenance in deciding why a particular
    OID instance arc (or arc group) failed to be converted (for use in
    logged error messages).

    "pdu" is the type of PDU that is being serviced at this time.  The PDU
    type can be one of: GET, SET, or GETNEXT.  The PDU type is required for
    setting up the modifier field in the moss_avl_add() call;  the modifier
    MUST have a valid meaning for SET requests; for GET & GETNEXT requests it
    is always MAN_C_SUCCESS.


OUTPUTS:

    There are many varieties of success for this function:

      FULL_EXACT:
          "inst_AVL" avl has been created with a construction containing
          however many elements are required to represent the "instance arcs"
          at the end of the "snmp_oid" PLUS the containment hierarchy.  There
          were no missing or extra instance arcs.
          Can occur only for "dstatus":
              CI_CLASS
              CI_ATTRIBUTE

      FULL_AND_EXTRA_ARCS:
          "inst_AVL" avl has been created with a construction containing
          however many elements are required to represent the "instance arcs"
          CALLED FOR BY THE MIR INSTANCE INFORMATION at the end of the
          "snmp_oid" PLUS the containment hierarchy.  There were extra instance
          arcs at the end of the SNMP OID once all the arcs called for by
          the MIR definition of the class were used.
          Can occur only for "dstatus":
              CI_ATTRIBUTE

      FULL_BUT_EXTRA_ARCS:
          "inst_AVL" avl has been created with just the class hierarchy.  This 
          occurs when conversion is complete (according to the MIR) for all classes
          but extra arcs remained at the end of SNMP OID *AND* we know that
          the attribute arc is bogus according to the MIR.
          Can occur only for "dstatus":
              CI_CLASS

      ONE_CONVERTED_THEN_JUNK:
          "inst_AVL" avl has been created with a construction containing
          as many elements for as many classes as possible up to and including
          the first conversion for a class whose subsequent conversions could
          not be performed, either because the existing arcs could not be
          converted or arcs required were missing.  In other words, for the
          last class processed, we converted one arc instance group and then
          couldn't convert any more (for some reason).
          (For SNMP PE V1.0, this code actually describes what happened at the
          lowest class level, because no instance arcs participate in V1.0 in
          the conversion of upper-level containment class instance information)
          Can occur only for "dstatus":
              CI_CLASS
              CI_ATTRIBUTE

      NONE_CONVERTED_JUNK:
          "inst_AVL" avl has been created with just the class hierarchy.  This
          occurs when the first attempt to convert instance arcs for a class fails
          due to a conversion error (incorrect arcs for the specific conversion).
          Can occur only for "dstatus":
              CI_CLASS
              CI_ATTRIBUTE

      NONE_CONVERTED_NONE_PRESENT:
          "inst_AVL" avl has been created with just the class hierarchy.  This
          occurs when there are no instance arcs at all.
          Can occur only for "dstatus":
              CI_CLASS
              CI_ATTRIBUTE

      NONE_CONVERTED_OK:
          "inst_AVL" avl has been created with just the class hierarchy.  This
          occurs when the MIR rolled in order to find a valid class, thereby
          invalidating any attribute or instance arcs present in the original
          SNMP OID.  
          Can occur only for "dstatus":
              CI_ROLLED_CLASS
              CI_ROLLED_ATTRIBUTE

      DONE_IGNORE_INSTANCE:
          "inst_AVL" avl has been created with a construction containing
          however many elements are required to represent the "instance arcs"
          at the end of the "snmp_oid" PLUS the containment hierarchy.  There
          may have been missing, incorrect, or extra instance arcs, but they 
          were ignored (this status is possible ONLY for GETNEXT requests, 
          since GETs and SETs don't allow for invalid arc information).
          Can occur only for "dstatus":
              CI_CLASS
              CI_ATTRIBUTE
              CI_ROLLED_CLASS
              CI_ROLLED_ATTRIBUTE


    On failures beyond those described above, we CRASH.


BIRD'S EYE VIEW:
    Context:
        The caller is one of "casend_process_*()" functions in this module.
        All necessary information has been extracted from the MIR for
        a varbind entry element, and the Object Instance AVL for the
        Common Agent msi_* call needs to be built.

    Purpose:
        This function merges the MIR instance information and original SNMP
        OID instance arcs thru a feat of legerdemain to create an
        'instance AVL'!

        Behold this magic per RFC1212.  Conversion of instance arcs is done on
        a "best effort" basis, with detailed results returned to the caller
        for any logging needed.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the AVL pointer NULL>
    if (attempt to init the main AVL failed)
        <CRASH "Mxxx - main Instance AVL initialization failed %d">

    switch (class/instance derivation status)
        case CI_CLASS:
        case CI_ATTRIBUTE:
            <compute index to 1st instance arc>
            if (there are no instance arcs AND pdu is GET or SET)
                <return NONE_CONVERTED_NONE_PRESENT>
            <set pointer to first instance info block in varbind block list>
            break:

        case CI_ROLLED_CLASS:
        case CI_ROLLED_ATTRIBUTE:
            if (pdu is GET or SET)
                <return NONE_CONVERTED_OK>

            <set up arc count for building an instance AVL with class hierarchy>
            <set pointer to first instance info block in varbind block list>
            break;

        default:
            <CRASH "Mxxx - Invalid derived-status inbound argument">

    if (attempt to start a construction failed)
        <CRASH "Mxxx - Start Construct on object instance AVL failed %d">

    <show "PARTIAL_PROC_DONE" yet>

    while (instance info blocks for a class remain
           and status is "PARTIAL_PROC_DONE")

        if (instance arcs do not remain AND pdu is GET or SET)
            <break>

        if (attempt to init the current-class AVL failed)
            <CRASH "Mxxx - Instance AVL initialization failed %d">

        <process current-class MIR-instance list>

        if (SUBELEMENT_FAIL_CONVERT is returned status)
            if (subelements == 0)
                <set status to "NONE_CONVERTED_JUNK">
                if (attempt to free current-class avl failed)
                    <CRASH "Mxxx - Attempt to free current-class AVL failed>
                <continue>
            else
                <set status to "ONE_CONVERTED_THEN_JUNK">
        else if (instance arcs not invalid)
            <show PARTIAL_PROC_DONE as the status for this iteration>

        (* Otherwise we converted something and we should return it *)
        if (subelements-in-class > 1)
            if (attempt to "start constructor" failed)
                <CRASH "Mxxx - Attempt to start constructor failed">

        if (attempt to append current-class AVL failed)
            <CRASH "Mxxx - Attempt to append current-class AVL Failed %d">

        if (subelements-in-class > 1)
            if (attempt to "end constructor" failed)
                <CRASH "Mxxx - Attempt to end constructor failed">

        <step class instance pointer to next class first-instance blk pointer>

    (* On exit from the loop, we need to decide what happened, and set  *)
    (* the return status accordingly.  If the status is already set, we *)
    (* just need to clean up.                                           *)

    <free the class_AVL if it is not NULL>

    if (processing status is PARTIAL_PROC_DONE)
        if (instance arcs remain)
            if (dstatus is CI_CLASS)
                <set processing status to FULL_BUT_EXTRA_ARCS>
            else
                <set processing status to FULL_AND_EXTRA_ARCS>
        else
            <set processing status to FULL_EXACT>
 
     switch (processing status)
         case NONE_CONVERTED_JUNK:
         case FULL_BUT_EXTRA_ARCS:
             <set modifier to MAN_C_NO_SUCH_OBJECT_INSTANCE>
             <break>

         case FULL_AND_EXTRA_ARCS:
         case FULL_EXACT:
             <set modifier to MAN_C_SUCCESS>
             <break>

         case ONE_CONVERTED_THEN_JUNK:
             <set modifier to MAN_C_INVALID_OBJECT_INSTANCE>
             <break>

         case DONE_IGNORE_INSTANCE:
             if (pdu == GETNEXT)
                 if (arcs_converted)
                     <set modifier to MAN_C_INVALID_OBJECT_INSTANCE>
                 else
                     <set modifier to MAN_C_NO_SUCH_OBJECT_INSTANCE>
             else (* for GET and SET *)
                 <set modifier to MAN_C_NO_SUCH_OBJECT_INSTANCE>
             break>

         default:
             <CRASH "Mxxx - Invalid processing status %d>

    if (attempt to end the construction failed)
        <CRASH "Mxxx - End Construct on object instance AVL failed %d">

    <update top-most AVL element's modifier value>

    <return processing status>


OTHER THINGS TO KNOW:

    There is a slick bit of happy GET-NEXT synchronization going on here.
    The following paragraphs describe this, because it shouldn't be obscured,
    as GET-NEXT processing is complicated to begin with.

    This function is called to analyze the instance arcs and build an AVL
    representing their contents ONLY for the Originally Received SNMP OID.

    For GET/SET, the processing must succeed 'exactly' (ie, we have to return
    FULL_EXACT) or GET/SET processing should fail.  This is obvious and simple.

    For GET-NEXT, "snmp_oid" in the vbe is a copy of the originally received
    SNMP OID (with instance arcs truncated off) *only* on the FIRST GET-NEXT
    attempt that SNMP PE makes.  If the Common Agent responds with some form
    of code that makes SNMP PE 'roll', then "snmp_oid" in the vbe will be
    changed (to 'the next object') and will no longer even resemble the 
    original received OID.

    Fortunately, once this happens, there are no 'instance arcs' to be
    analyzed, and all this function has to do to do the right thing is
    to create an instance AVL with just the class hierarchy; there will
    always be an avl element with the attribute oid and, if valid, the
    instance arcs described in the octet.

    (All the above may seem like idle comments, but during development
    the 'meaning' of the "snmp_oid" cell in the vbe was changed from
    "Is Always the Original SNMP OID" to "Is Always the Original SNMP OID"
    **until the first GET-NEXT roll**"!  This had the potential to upset the
    applecart in this function, but fortunately it doesn't!  Just ta let ya
    know. . .)

    On a related point, "vbe->orig_oid" is used here, because it DOES have
    the instance arcs required for analysis and construction of the instance
    AVL.
*/

{
man_status      status;            /* Local general purpose status word    */
bld_IA_status   bstatus;           /* Build status holder                  */
snmp_instance_t *MIR_inst;         /*-> MIR-derived instance info list     */
char            msg[LINEBUFSIZE];  /* Error Message build buffer           */
avl             *inst_AVL;         /* Local pointer to instance AVL        */
int             subs_OK;           /* Count of subelements created OK      */
avl             *class_AVL=NULL;   /* AVL for one (current) class "element"*/
unsigned int    modifier;          /* Final overall Modifier value for AVL */
int             arcs_converted=0;  /* NON-zero == some Inst arcs converted */


/* initialize the AVL pointer NULL */
*p_inst_AVL = NULL;

/* if (attempt to init the AVL failed) */
if ((status = moss_avl_init(p_inst_AVL)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg161, "M161 - Instance AVL initialization failed (%s)"),
            get_man_status(status));
    CRASH(msg);
    }

inst_AVL = *p_inst_AVL;         /* Grab local copy */

/* switch (class/instance derivation status)
|
|  Here we filter out the case where we return an instance AVL with just the
|  class hierarchy without even having to look at any instance arcs.  We also 
|  prepare for handling those cases where we DO look at the instance arcs.
*/
switch (dstatus) {

    case CI_CLASS:
    case CI_ATTRIBUTE:
        /*  compute index to 1st instance arc
        |
        | We reach inside the class oid structure to get it's length here, and
        | carrying in mind that the arrays are 0-origined, the class OID length
        | used as an index points to the attribute arc.  Consequently one more
        | points to the first instance arc. Subtracting the first instance arc
        | index from total number of arcs in the original SNMP OID gives us
        | instance-arcs-remaining (including the one we're pointing at).
        |
        | Note that all this logic only works when the class oid is actually
        | a part of the SNMP OID (which it may not be if we rolled with dstatus
        | CI_ROLLED_CLASS or CI_ROLLED_ATTRIBUTE).
        */
        *i_arc = vbe->class_oid.count + 1;
        *i_arcs_left = vbe->orig_oid->count - *i_arc;

        /* if (there are no instance arcs AND pdu is GET or SET) */
        if ( (*i_arcs_left <= 0) && (pdu == GET || pdu == SET) )
        {
            /* Return with an initialized and empty instance AVL */
            return(NONE_CONVERTED_NONE_PRESENT);
        }

        /* set pointer to first instance info block in varbind block list */
        MIR_inst = vbe->inst_list;
        break;

    case CI_ROLLED_CLASS:
    case CI_ROLLED_ATTRIBUTE:
        /* "rolling" should never occur for GET or SET */
        if (pdu == GET || pdu == SET)
            return(NONE_CONVERTED_OK);

        /* set up to create an instance avl containing the class hierarchy */
        *i_arc = vbe->class_oid.count + 1;
        *i_arcs_left = 0;

        /* set pointer to first instance info block in varbind block list */
        MIR_inst = vbe->inst_list;
        break;

    default:
        sprintf(msg,
                MSG(msg074, "M074 - Invalid derived-status %d inbound argument"),
                (int) dstatus
                );                    
        CRASH( msg );
    }

/*
|  From here on down, we're dealing with an original SNMP OID that has
|  embedded in it at least a class OID (CI_CLASS) or possibly an attribute OID
|  (CI_ATTRIBUTE) and we're going to have to build an instance AVL accordingly.
|  There may or may not be instance arcs and we may yet have to return a
|  an instance AVL with just the class hierarchy depending on how the conversion
|  of instance arcs goes (success/failure) and depending on whether we've got
|  just class (CI_CLASS) or class+attribute (CI_ATTRIBUTE).
*/

/* if (attempt to start a construction failed) */
if ((status = moss_avl_start_construct(inst_AVL,
                                       NULL,
                                       NULL,
                                       ASN1_C_SEQUENCE,
                                       NULL)) != MAN_C_SUCCESS) {
    sprintf(msg,
            MSG(msg162, "M162 - Instance AVL start-construct failed (%s)"),
            get_man_status(status));
    CRASH( msg );
    }

bstatus = PARTIAL_PROC_DONE;    /* show "PARTIAL_PROC_DONE" yet */

/*
|  Each time this loop goes around, we build an "element" in the "sequence
|  of distinguished names" (per Page 4 of RFC1214).  Each element is an element
|  in the instance AVL, and it may be just a single "bare" AVL element or a
|  true AVL SEQUENCE.  (We make it a sequence (by "if(subsOK > 1)" code
|  executed in this loop) if the element has more than one "distinguished
|  name").
|
|  Each such element corresponds to one or more "naming attributes" for a
|  particular class.  All the "naming attributes" values for a class that
|  arrive as instance arcs in the SNMP oid are converted to AVL elements
|  through the call to function casend_bld_IA_element().
|
|  Each time we go around, we process one MIR instance list for a class,
|  starting with the outermost containing class and working down to the
|  innermost.
|
|  NOTE:  For the outermost classes, SNMPPE V1.0 creates just a single AVL
|  element whose datatype is ASN1_C_NULL w/OID extracted from the MIR or
|  generated by code in snmppe_mir.c at the time the MIR instance block is
|  created... no instance arcs from the original SNMP OID are used during
|  the creation of the AVL elements for the outermost containing classes.
|
|  Only the innermost class actually uses SNMP instance arcs.  Each
|  "subelement" within the element for the innermost class uses one or more
|  instance arcs that are converted according to the MIR instance block for
|  that subelement (see code for casend_bld_IA_element() and
|  casend_bld_IA_subelement()).
|
| *********************************************:
| ******************* NOTE ********************:
| *********************************************:
|
|  When the instance arcs for a NON-TABLE are being constructed for a GETNEXT
|  operation and the "bstatus" ends up being DONE_IGNORE_INSTANCE", we perform
|  a little *magic* with the MIR instance list in the varbind entry block.
|
|  If the instance information is valid (e.g., '.0'), then the attribute oid
|  is put into an avl element with the octet set to:
|      tag = ASN1_C_NULL, length = 4, *string_ptr = 0.
|
|  If the instance information is INvalid (e.g., not '.0' or is missing), then 
|  the attribute oid is put into an avl element with the octet set to:
|      tag = ASN1_C_NULL, length = 0, string_ptr = NULL.
|
|  This is done so that the MOM can differentiate "valid" instance information
|  from "invalid" instance information (since the normal encoding for the
|  ASN1_C_NULL type has length 0 and a NULL pointer).  
|
|  The *magic* that is performed to to the MIR instance list is to add a "new"
|  final list element that contains the attribute oid (this is normally NOT
|  derived by the MIR).  We do this so that when the SNMPPE_RECV (i.e., the
|  pei thread) can properly unwind the instance avl.  
|
|  This all had to be done in order to:
|     - convey the class hierarchy to the MOMs
|     - provide a "clean", symmetrical conveyence of instance information for
|       non-tables and tables (to make MOMgen user's lives nicer).
|
| *********************************************
| ***************** END NOTE ******************
| *********************************************
|
|  while (instance info blocks for a class remain
|         and status is "PARTIAL_PROC_DONE")
*/

while (MIR_inst != NULL && bstatus == PARTIAL_PROC_DONE) {

    /* if instance arcs do not remain AND pdu is GET or SET */
    if ( (*i_arcs_left < 1) && (pdu == GET || pdu == SET) )
        break;

    /*
    |  Here we're creating a temporary AVL for use just within one iteration
    |  of this loop to contain the AVL representation of one class "element"
    |  in the SNMP "sequence of distinguished names".  Once created, we'll
    |  know whether the AVL has one or more elements in it thereby indicating
    |  whether or not the AVL should be stuck into the main instance AVL we're
    |  building "bare", or with start/end constructors surrounding it.
    */
    if ((status = moss_avl_init(&class_AVL)) != MAN_C_SUCCESS) {
        sprintf(msg, MSG(msg071, "M071 - Instance AVL initialization failed (%s)"),
                get_man_status(status));
        CRASH(msg);
        }

    /*
    |  process current-class MIR-instance list & corresponding instance arcs
    |  to produce a single "class_AVL" containing "the element" (from SNMP
    |  point-of-view) that describes the "distinguished name" for the class.
    */
    bstatus =
        casend_bld_IA_element(bp,               /* Big Picture             */
                              vbe->orig_oid,    /* SNMPoid w/instance arcs */
                              i_arc,            /* 'next' instance arc     */
                              i_arcs_left,      /* instance arcs left      */
                              MIR_inst,         /* MIR list for class      */
                              &subs_OK,         /* subelements created OK  */
                              class_AVL,        /* Build into this AVL     */
                              pdu,              /* PDU type                */
                              vbe,              /* Varbind Block Entry ptr */
                              &arcs_converted); /* NON-zero==arcs converted*/

    /*
    |  We process the subelements (groups of instance arcs) on a best effort
    |  basis within casend_bld_IA_element().  When a conversion blows, if 
    |  we've managed to convert at least one subelement for the class
    |  successfully, then we'll be returning the entire instance AVL with
    |  whatever we got up to the error.  Otherwise, we'll be dumping the
    |  entire instance AVL (ie if we attempted a conversion for the FIRST
    |  subelement within an element for a class and it blew).
    |
    |  if (SUBELEMENT_FAIL_CONVERT is returned status)
    */
    if (bstatus == SUBELEMENT_FAIL_CONVERT) {/* some conversion failed . . . */

        /*
        |  Set our local status to indicate how far conversion got for this
        |  class according the count of subelements converted OK for this
        |  element.
        */
        if (subs_OK == 0) {     /* No subelements converted OK . . . */

            /* set status to "NONE_CONVERTED_JUNK"
            |
            | This plus the "continue" below forces us out of the loop.  In
            | all such circumstances, we're going to want to drop everything.
            | Since the class AVL has absolutely nothing in it to append to
            | the main instance AVL, we set this status to cause the loop
            | to exit . . . .
            */
            bstatus = NONE_CONVERTED_JUNK;  /* "junk" may also mean "ran out */
                                            /* of instance arcs"             */

            /*
            | We've got to blow off the class AVL here just for this case
            | 'cause we're skipping the 'normal' release of it within this
            | loop. . . 
            |
            |  if (attempt to free current-class avl failed)
            */
            if ((status = moss_avl_free(&class_AVL, TRUE)) != MAN_C_SUCCESS) {
                sprintf(msg,
                        MSG(msg163, "M163 - Error from AVL free (%s)"),
                        get_man_status(status));
                CRASH( msg );
                }

            /*
            | We skip the append and release below to avoid appending nothing.
            */
            continue;
            }

        else {  /* . . . at least one subelement converted OK */
            /* set status to "ONE_CONVERTED_THEN_JUNK"
            |
            | This'll force us out of the loop prematurely due to the error
            | but not before we concatenate the class AVL "element" onto the
            | main instance AVL.
            */
            bstatus = ONE_CONVERTED_THEN_JUNK;
            }
        }

    /* else if (instance arcs not invalid) */
    else if (bstatus != DONE_IGNORE_INSTANCE) {
        /* show PARTIAL_PROC_DONE as the status for this iteration */
        bstatus = PARTIAL_PROC_DONE;
        }

    /* Otherwise we converted something and we should return it */
    /* if (subelements-in-class > 1) */
    if (subs_OK > 1) {
        /* if (attempt to "start constructor" failed) */
        if ((status = moss_avl_start_construct(inst_AVL,
                                               NULL,
                                               NULL,
                                               ASN1_C_SEQUENCE,
                                               NULL)) != MAN_C_SUCCESS) {
            sprintf(msg,
                    MSG(msg164, "M164 - Instance AVL start-construct failed (%s)"),
                    get_man_status(status));
            CRASH( msg );
            }
        }

    /* if (attempt to append current-class AVL failed)
    |  (NOTE: This does not free the class_AVL, it is still initialized
    |         but empty and can be reused.)
    */
    if ((status = moss_avl_append(inst_AVL,
                                  class_AVL,
                                  FALSE)) != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg165, "M165 - Attempt to append current Class AVL Failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }

    /* if (subelements-in-class > 1) */
    if (subs_OK > 1) {
        /* if (attempt to "end constructor" failed) */
        if ((status = moss_avl_end_construct(inst_AVL)) != MAN_C_SUCCESS) {
            sprintf(msg,
                    MSG(msg166, "M166 - Instance AVL end-construct failed (%s)"),
                    get_man_status(status));
            CRASH( msg );
            }
        }

    /* step class instance pointer to next class first-instance blk pointer */
    MIR_inst = MIR_inst->next_class;

    } /* end of while loop */

/* free the class_AVL if it is not NULL */
if (class_AVL != NULL) {
    if ((status = moss_avl_free(&class_AVL, TRUE)) != MAN_C_SUCCESS) {
	sprintf(msg,
		MSG(msg285, "M285 - Error from AVL free (%s)"),
		get_man_status(status));
	CRASH( msg );
	}
    }


/*
|  On exit from the loop, we need to decide what happened, and set
|  the return status accordingly.  If the status is already set, we
|  just need to clean up.  PARTIAL_PROC_DONE means all conversions went OK,
|  and we ran out of MIR list that directed conversions.  DONE_IGNORE_INSTANCE
|  means that the instance arcs were either malformed, deficient, or 
!  out of range; we simply build the class hierarchy for this status.
|
|  if (processing status is PARTIAL_PROC_DONE)
*/
if (bstatus == PARTIAL_PROC_DONE) {

    /* if (instance arcs remain) */
    if (*i_arcs_left > 0) {

        if (dstatus == CI_CLASS) {
            /*
            | A '_LONG' SNMP OID did not get derived to a MIR attribute, only
            | a MIR class.   Consequently we have a situation where an
            | attribute arc was bogus, BUT we converted all the instance arcs
            | that followed the bogus arc without any error, but when we did
            | all the conversions specified by the MIR we wound up with still
            | more unconverted instance arcs.
            */
            bstatus = FULL_BUT_EXTRA_ARCS;
            }
        else {
            /*
            | A '_LONG' SNMP OID did derive a MIR attribute object.
            | Consequently we have a situation where the attribute arc was
            | valid, AND we converted all the instance arcs that followed
            | the valid arc without any error.  When all the conversions
            | specified by the MIR were finished, we had still more
            | unconverted instance arcs.
            */
            bstatus = FULL_AND_EXTRA_ARCS;
            }
        }
    else {  /* Everything came out 'even' */
        /* set processing status to FULL_EXACT */
        bstatus = FULL_EXACT;
        }
    }

/*
| Make the AVL match the return status we've decided on.
*/

switch (bstatus) {

    case NONE_CONVERTED_JUNK:
    case FULL_BUT_EXTRA_ARCS:
        modifier = MAN_C_NO_SUCH_OBJECT_INSTANCE;
        break;

    case FULL_AND_EXTRA_ARCS:
    case FULL_EXACT:
        modifier = MAN_C_SUCCESS;
        break;

    case ONE_CONVERTED_THEN_JUNK:
        modifier = MAN_C_INVALID_OBJECT_INSTANCE;
        break;

    case DONE_IGNORE_INSTANCE:
        if (pdu == GETNEXT)
        {
            if (arcs_converted)
                modifier = MAN_C_INVALID_OBJECT_INSTANCE;    
            else
                modifier = MAN_C_NO_SUCH_OBJECT_INSTANCE;
        }
        else /* for GET and SET */
        {
            modifier = MAN_C_NO_SUCH_OBJECT_INSTANCE;
        }
        break;

    default:
        sprintf(msg, MSG(msg171, 
                "M171 - Invalid processing status %d"),(int) bstatus);
        CRASH( msg );
    }

if ((status = moss_avl_end_construct(inst_AVL)) != MAN_C_SUCCESS) {
    sprintf(msg,
            MSG(msg170, "M170 - Instance AVL end-construct failed (%s)"),
            get_man_status(status));
    CRASH( msg );
}

/* update top-most AVL element's modifier value */
casend_update_avl_modifier (bp, inst_AVL, modifier);

/* return processing status */
return(bstatus);

}

/* casend_bld_IA_element - Build one Element (for one class) in Instance AVL */
/* casend_bld_IA_element - Build one Element (for one class) in Instance AVL */
/* casend_bld_IA_element - Build one Element (for one class) in Instance AVL */

static bld_IA_status
casend_bld_IA_element(bp, snmp_oid, i_arc, i_arcs_left, inst, subs_OK,
                      inst_AVL, pdu, vbe, arcs_converted)

big_picture_t         *bp;           /*-> Big Picture Structure for SNMP PE  */
object_id             *snmp_oid;     /*-> Full SNMP OID we're working on     */
int                   *i_arc;        /*-> Index in SNMPOID value[] to nextarc*/
int                   *i_arcs_left;  /*-> Count of remaining arcs in value[] */
snmp_instance_t       *inst;         /*-> 1st MIR Instance Block for a class */
int                   *subs_OK;      /*-> Count of sub-identifiers proc'ed ok*/
avl                   *inst_AVL;     /*-> Instance AVL for class element     */
pdu_type              pdu;           /* PDU type (GET, SET, GETNEXT)         */
varbind_t             *vbe;          /*-> Varbind Entry Block for SNMP OID   */
int                   *arcs_converted;/*-> Flag (1+ if Inst arcs converted)  */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "snmp_oid" is the object id of the SNMP variable we're working on
    for this request.

    "i_arc" is the index into the value[] array of "snmp_oid" to the next
    instance arc to be used for conversion.  This is modified upon return.

    "i_arcs_left" is the count of remaining arcs in value[] of "snmp_oid"
    starting at "i_arc".  This is modified upon return.

    "inst" is a pointer to the first MIR instance block of
    information for the entire class we're to process.

    "subs_OK" is set on return to the number of subidentifiers for the
    class that were properly converted.

    "inst_AVL" is the AVL to contain the element for the class.  The
    AVL should already be initialized.

    "pdu" is the type of PDU that is being serviced at this time.  The PDU
    type can be one of: GET, SET, or GETNEXT.  The PDU type is required for
    setting up the modifier field in the moss_avl_add() call;  the modifier
    MUST have a valid meaning for SET requests; for GET & GETNEXT requests it
    is always MAN_C_SUCCESS.

    "vbe" is the address of a Varbind Entry Block that corresponds
    to the "SNMP" OID we're processing.  Referenced in this data structure
    are:
        * The Original SNMP OID we're working on
        * The class oid and
        * instance information


OUTPUTS:

    The function returns the last code returned by it's call to
    casend_bld_IA_subelement() which can only be SUBELEMENT_OK_CONVERT or
    SUBELEMENT_FAIL_CONVERT.

    The values of "i_arc", "i_arcs_left", and "inst" are modified on return.

    "subs_OK" is set to the number of times casend_bld_IA_SUBelement() returned
    "SUBELEMENT_OK_CONVERT" which corresponds to the number of
    'sub-identifiers' (groups of instance arcs) successfully converted.


BIRD'S EYE VIEW:
    Context:
        The caller is "casend_bld_instance_avl" functions in this module.
        It needs to have all identifying attributes for a selected class
        converted into AVL format from SNMP OID "instance arc" format.

    Purpose:
        This function merges the MIR instance information for ONE class and
        the original SNMP OID instance arcs that should correspond to that
        class into AVL format.


ACTION SYNOPSIS OR PSEUDOCODE:

    <show status as "SUBELEMENT_OK_CONVERT">
    <show subs_OK as 0>

    while ( (instance info blocks remain within the class) AND
            ((last status was "SUBELEMENT_OK_CONVERT") OR
             (last status was "DONE_IGNORE_INSTANCE AND pdu is GETNEXT)) )

        if (no instance arcs are left AND pdu is GET or SET)
            break; (* we allow zero instance arcs for GETNEXT *)

        <attempt conversion of next instance block using SNMPOID instance arcs>
        if (status is SUBELEMENT_OK_CONVERT)


        if ( (bstatus == SUBELEMENT_OK_CONVERT) OR
             (bstatus == DONE_IGNORE_INSTANCE && pdu == GETNEXT) )
            <increment subs_OK by 1>

    <return last conversion status>

OTHER THINGS TO KNOW:

    Each time this function is invoked, one element in the "sequence of
    distinguished names" (per Page 4 of RFC1214) is being converted from
    SNMP OID instance arc form into an element in the instance AVL.

    Each element in the sequence of distinguished names may itself be a
    sequence of "sub-elements" where each sub-element is an instance of
    one "naming attribute".  Normally, for all the higher containing classes,
    the naming attribute is null and the MIR specifies a datatype of
    ASN1_C_NULL, the element for that class in the sequence of distinguished
    names gets converted into a single AVL element by this function, and no
    real instance arcs in "snmp_oid" get used during this conversion.  This
    higher-class conversion is triggered by variable "snmp_higher_class_hack"
    being set to TRUE.  This allows this function to be expanded later
    (by removing the logic surrounding "snmp_higher_class_hack") to allow
    real SNMP OID arcs to be used to specify higher containing class naming
    attribute values (SNMP has been hacked like this in RFC1214, they'll do it
    again, just watch!)

    The loop in this function processes all "sub-elements" that go to make up
    the "element".  Currently, only the bottom-most class is actually going
    to stand at risk of having more than one sub-element.
   
    For instance, (uhmm ..), consider the "ipNetToMediaEntry" described at the
    bottom of page 11 in RFC1212, which has an "INDEX" clause value of

                        { ipNetToMediatIfIndex,
                          ipNetToMediaNetAddress }.

    Each of these two entries in the INDEX clause corresponds to one MSL
    line of:
                IDENTIFIERS = ( ipNetToMediaIfIndex ipNETtoMediaNetAddress ),

    When "mir_class_inst_GETSET()" is called, all the higher containing
    classes each have a "list" entry consisting of a single element specifying
    "ASN1_C_NULL" as the datatype of the naming attribute, then for the
    bottom-most class (for which the IDENTIFIERS = clause shown above appeared
    in the MSL) the two naming attributes get converted into two "instance
    blocks" (of type "snmp_instance_t") in the MIR instance list:

    MIR Instance
    List--> {        ("ELEMENT" corresponds to an "snmp_instance_t" block)

             ELEMENT=ASN1_C_NULL  (Outer-most containing class)
             |
             |..(via snmp_instance_t cell "next_class")
             |
             V

             ELEMENT=ASN1_C_NULL  (Next class)
             |
             |..(via snmp_instance_t cell "next_class")
             |
             V
                  .
                  .         (as many nested classes as turns out)
                  .
                  .                      ..(via snmp_instance_t cell "next")
                  .                      |
                  .                      V
             ELEMENT=ipNetToMediaIfIndex -->  ELEMENT=ipNetToMediaNetAddress
             |
             V
             NULL ("next_class" == NULL means "bottom-most" class hit)
            }


    If we regard each instance block as describing a "sub-element" mentioned
    above, then the loop in this function rips SNMP OID instance-arcs off
    in groups where each group corresponds to an "instance block".  A conversion
    is performed on each group and the results become a single element in the
    instance AVL. (Sometimes a group may consist of only one arc).  (For
    the higher classes, "snmp_higher_class_hack" is TRUE and no instance
    arcs are actually used).
   
    The construction of this function and it's caller is intended to allow
    the handling of the conversion of a class's naming attribute instance
    value(s) uniformly regardless of whether the class is the bottom-most
    "real" SNMP class (as SNMP is currently defined, with
    snmp_higher_class_hack = FALSE) or the higher 'fake' classes whose naming
    attribute instance values are currently always NULL (and for which no real
    SNMP OID arcs are used in conversion. . . snmp_higher_class_hack = TRUE).
*/

{
bld_IA_status   bstatus;        /* Build Status */

/* show status as "SUBELEMENT_OK_CONVERT" */
bstatus = SUBELEMENT_OK_CONVERT;

/* show subs_OK as 0 */
*subs_OK = 0;

/*
|   While MIR Instance Info Blocks for this CLASS remain 
|                       AND
|       our last subelement conversion went OK . . . . . 
|
|   while ( (instance info blocks remain within the class) and
|           ((last status was "SUBELEMENT_OK_CONVERT") or
|            (last status was "DONE_IGNORE_INSTANCE and pdu is GETNEXT)) )
*/

while ( (inst != NULL) && 
        ((bstatus == SUBELEMENT_OK_CONVERT) ||
         (bstatus == DONE_IGNORE_INSTANCE && pdu == GETNEXT)) ) {

    if ( (*i_arcs_left <= 0) && (pdu == GET || pdu == SET) )
        break; /* we allow 0 instance arcs for GETNEXT */

    /* attempt conversion of next instance block using SNMPOID instance arcs */
    bstatus = casend_bld_IA_subelement(bp,         /* The Big Picture        */
                                       snmp_oid,   /* SNMPOID w/inst.  arcs  */
                                       i_arc,      /* Start w/this inst arcs */
                                       i_arcs_left,/* Instance arcs left     */
                                       &inst,      /* MIR Inst blk for subel */
                                       inst_AVL,   /* AVL to convert into    */
    /* snmp_higher_class_hack ==> */   ((inst->next_class != NULL) ?
    /* ('real' conversion of SNMP */                      TRUE : FALSE),
    /* instance arcs happens when */
    /* this is FALSE              */
                                       pdu,        /* PDU type               */
                                       vbe,        /*-> Varbind Block Entry  */
                                       arcs_converted);

    if ( (bstatus == SUBELEMENT_OK_CONVERT) || 
         (bstatus == DONE_IGNORE_INSTANCE && pdu == GETNEXT) ) {
        /* increment subs_OK by 1 */
        *subs_OK += 1;
        }
    }

/* return last conversion status */
return(bstatus);
}

/* casend_bld_IA_subelement - Build one subelement for a class element */
/* casend_bld_IA_subelement - Build one subelement for a class element */
/* casend_bld_IA_subelement - Build one subelement for a class element */

static bld_IA_status
casend_bld_IA_subelement(bp, snmp_oid, i_arc, i_arcs_left, inst, inst_AVL,
                         snmp_higher_class_hack, pdu, vbe, arcs_converted)

big_picture_t      *bp;             /*-> Big Picture Structure for SNMP PE   */
object_id          *snmp_oid;       /*-> Full SNMP OID we're working on      */
int                *i_arc;          /*-> Index in SNMPOID value[] to nextarc */
int                *i_arcs_left;    /*-> Count of remaining arcs in value[]  */
snmp_instance_t    **inst;          /*->> MIR Instance blk for a subelement  */
avl                *inst_AVL;       /*->> Pointer to instance element AVL    */
BOOL        snmp_higher_class_hack; /* TRUE: We're hacking the build of a    */
                                    /*       subelement of higher class      */
pdu_type           pdu;             /* PDU type (GET, SET, GETNEXT)          */
varbind_t          *vbe;            /*-> Varbind Entry Block for SNMP OID    */
int                *arcs_converted; /*-> Flag (1+ if Inst arcs converted)    */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "snmp_oid" is the object id of the SNMP variable we're working on
    for this request.

    "i_arc" is the index into the value[] array of "snmp_oid" to the next
    instance arc to be used for conversion.  This is modified upon successful
    return.

    "i_arcs_left" is the count of remaining arcs in value[] of "snmp_oid"
    starting at "i_arc".  This is modified upon successful return.

    "inst" is the address of a pointer to the MIR instance block of
    information for the subelement we're to process.  This is modified
    upon successful return.

    "inst_AVL" is the AVL into which the element for the current class
    is being built.  We're adding a "subelement" (from SNMP standpoint) AS
    an "AVL ELEMENT" to this AVL.  The pointer whose address is passed for this
    AVL should already be initialized.

    "snmp_higher_class_hack" is a Boolean flag used to indicate that we're
    building a subelement of an element that is a higher class element.
    When TRUE, it means basically "don't use any real instance arcs and
    check to be sure we're encoding something that has a datatype of
    ASN1_C_NULL".  When SNMP is expanded to allow the transmission of
    instance arcs that describe naming attributes of higher containing
    classes above the lowest class, this argument and the supporting code
    can be removed.

    "pdu" is the type of PDU that is being serviced at this time.  The PDU
    type can be one of: GET, SET, or GETNEXT.  The PDU type is required for
    setting up the modifier field in the moss_avl_add() call;  the modifier
    MUST have a valid meaning for SET requests; for GET & GETNEXT requests it
    is always MAN_C_SUCCESS.
    
    "vbe" is the address of a Varbind Entry Block that corresponds
    to the "SNMP" OID we're processing.  Referenced in this data structure
    are:
        * The Original SNMP OID we're working on
        * The class oid and
        * instance information


OUTPUTS:

    The function returns SUBELEMENT_OK_CONVERT or SUBELEMENT_FAIL_CONVERT
    depending on whether the conversion of the instance arcs succeeded or
    failed.  DONE_IGNORE_INSTANCE is returned if an instance block does
    not convert correctly for GETNEXT operations.  Since we are supplying 
    the class hierarchy in the instance AVL at a minimum, we need to know 
    when the instance arcs are bad, so that the instance AVL can be sent in 
    a proper form.

    On SUBELEMENT_OK_CONVERT:
    The values of "i_arc", "i_arcs_left", and "inst" are modified on
    return, and the AVL pointed to by p_iAVL has another element added
    to it containing the information converted from the instance arcs for this
    subelement.

    On SUBELEMENT_FAIL_CONVERT:
    Nothing is added to the AVL, "i_arc", "i_arcs_left", and "inst" remain
    unchanged.

    On DONE_IGNORE_INSTANCE:
    The values of "i_arc", "i_arcs_left", and "inst" are modified on
    return, and the AVL pointed to by p_iAVL has another element added
    to it containing the information converted from the instance arcs for this
    subelement (although the actual instance information may consist of just
    the attribute oid with an "empty" octet if the object is a non-table and
    the operation is a GETNEXT).

BIRD'S EYE VIEW:
    Context:
        The caller is "casend_bld_IA_class()" functions in this module.
        It needs to have an identifying attribute for a selected class
        converted into AVL format from SNMP OID "instance arc" format.

    Purpose:
        This function merges the MIR instance information for ONE 'subelement'
        (from an 'snmp_instance_t' block) and the original SNMP OID instance arcs
        that should correspond to that sub-element into AVL format.


ACTION SYNOPSIS OR PSEUDOCODE:

    <grab pointer to actual MIR instance block we're going to reference>

    <zap local octet_string datastructure to zero/null>

    if (snmp_higher_class_hack is TRUE)
        if (MIR instance tag is not ASN1_C_NULL)
            <SYSLOG
              "Mxxx - Higher Class tag not ASN1_C_NULL: (oid) %d, defaulting">
              
        if (attempt to add default higher class element to AVL failed)
            <CRASH "Mxxx - Add of Element to element Instance AVL failed %D">

        <advance the pointer to the MIR instance block address>
        <return SUBELEMENT_OK_CONVERT>

    switch (MIR instance info ASN.1 Tag)

        case ASN1_C_NULL:
            (* If we get here and the MIR instance list element is the "fake"
               one just added, we can skip it and return since we've already
               processed it into the instance AVL *)

            if (this MIR instance list element is not part of the class hierarchy)
                <advance MIR_instance pointer to next (empty) class>
                if (no arcs were converted OK)
                    <return DONE_IGNORE_INSTANCE>
                else
                    <return SUBELEMENT_CONVERT_OK>

            <remember this object is a non-TABLE object>

            if (at least one instance arc does NOT remain)
                if (pdu is GET or SET)
                    <return SUBELEMENT_FAIL_CONVERT>
                else
                    <set bstatus to DONE_IGNORE_INSTANCE>

            else if (arc is not 0)
                if (pdu is GET or SET)
                    <return SUBELEMENT_FAIL_CONVERT>
                else
                    <set bstatus to DONE_IGNORE_INSTANCE>

            if (instance arcs are OK)
                <set octet_string length to 0>
                <set octet_string data pointer to null>
                <set octet_string data type to ASN1_C_NULL>
                <show one less instance arc remaining for processing>
                <set "arcs_converted" to TRUE>

            break;

        case INET_C_SMI_COUNTER:
        case INET_C_SMI_GAUGE:
            (* NOTE: These values are UNSIGNED! *)
        case ASN1_C_INTEGER:
            (* NOTE: Integers are SIGNED! *)
            if (at least one instance arc does NOT remain)
                if (pdu is GET or SET)
                    <return SUBELEMENT_FAIL_CONVERT>
                else
                    <set bstatus to DONE_IGNORE_INSTANCE>
                    <break>

            <set octet_string length to size of integer>
            <set octet_string data pointer to next instance arc>
            <set octet_string data type to integer>
            <show one less instance arc remaining for processing>
            <set "arcs_converted" to TRUE>
            break;

        case ASN1_C_PRINTABLE_STRING:
        case ASN1_C_OCTET_STRING:
            if (attempt to create octet string from OID arcs FAILED)
                <LOG OIDANOMLY "Mxxx ----">
                if (pdu is GET or SET)
                    <return SUBELEMENT_FAIL_CONVERT>
                else
                    <set bstatus to DONE_IGNORE_INSTANCE>
                    <break>

            <show used instance arcs not remaining for processing>
            <set "arcs_converted" to TRUE>
            break;

        case INET_C_SMI_IP_ADDRESS:
            if (not enough arcs remain for IP address)
                <LOG OIDANOMLY "Mxxx - Not enough inst arcs for IP Addr (oid)>
                if (pdu is GET or SET)
                    <return SUBELEMENT_FAIL_CONVERT>
                else
                    <set bstatus to DONE_IGNORE_INSTANCE>
                    <break>

            <build host word w/IP address from instance arcs>
            <convert from network to host layout>
            <initialize octet_string to point to conversion w/length>
            <show used instance arcs not remaining for processing>
            <set "arcs_converted" to TRUE>
            break;

        default:
            <LOG OIDANOMLY"Mxxx - Unsupported Instance arc translation for
                   Tag %hex, SNMP (oid)">
            if (pdu is GET or SET)
                <return SUBELEMENT_FAIL_CONVERT>
            else
                <set bstatus to DONE_IGNORE_INSTANCE>
                <break>

    <set up modifier field based on pdu type and the instance arc conversion>

    if (attempt to add an element to AVL failed)
        <CRASH - "Mxxx - Add of sublement to Instance AVL element failed %d">

    if (we should free the octet string value)
        <free the octet string value>

    if ((object is a non-TABLE) AND (no more classes to convert) AND
        (element just converted is of type ASN1_C_NULL))
        <tack on a "fake" instance block for the pei side to use>
        <set up the "new" last MIR instance list block>
        <copy length and dynamically allocate storage for instance object ID>
        <copy SNMP OID arcs and set AVLtag to ASN1_C_NULL>
        <make the "old" last element point to the "new" last element>

        if (overall conversion of instance arc failed)
            <leave octet string "empty" (0 length, NIL ptr) >
        else 
            <stuff a "valid" NULL value into the octet string>

        if (attach attribute oid and the octet to the avl subelement FAILED)
           <CRASH - "Mxxx - Add of sublement to Instance AVL element failed %d">

    <advance the pointer to the MIR instance block address>

    if (build status is DONE_IGNORE_INSTANCE)
        <return DONE_IGNORE_INSTANCE>
    elses
        <return SUBELEMENT_OK_CONVERT>

OTHER THINGS TO KNOW:

    Each time this function is invoked, one subelement that goes towards
    making up an element of a "sequence of distinguished names" (per Page 4
    of RFC1214) is being converted from 'instance-arc' form into AVL form.

    See the Other Things to Know section of casend_bld_IA_element(), this
    function's caller.

    Note that we 'convert' an instance arc whose target datatype is
    ASN1_C_NULL by checking the value of the provided arc to be sure it
    is 0 and then dropping it on the floor.  The conversion fails if the
    arc value is not 0.

    NOTE: Not all of the conversions specified in RFC1214 are supported as
          of V1.0. (Jan. 1992)

    Note that we must always minimally pass the class hierarchy information in 
    in the instance AVL.  For cases where the instance arcs are missing or
    invalid (malformed or out of range), then we simply end the sequence in
    the instance avl, setting the modifier field appropriately in 
    casend_bld_IA_subelement().


*/

/*
| This macro makes sure that we always manipulate "*i_arc" (pointer to the
| index of the "current" arc) and "*i_arcs_left" (pointer to the count of
| remaining arcs) TOGETHER!
*/
#define BUMP_ARC_INDICES()  *i_arc += 1;  *i_arcs_left -= 1;
{
char            msg[LINEBUFSIZE];  /* Error Message build buffer           */
octet_string    os;                /* An AVL-style octet string            */
man_status      status;            /* MOSS status                          */
snmp_instance_t *MIR_inst;         /*-> MIR-derived instance info block    */
BOOL            free_os=FALSE;     /* TRUE: Free the os string value       */
unsigned int    ip_addr = 0;       /* Build IP Addr here                   */
int             i,j;               /* Handy Index                          */
unsigned int    modifier;          /* Modifier value for AVL               */
bld_IA_status   bstatus;           /* Build Instance AVL status            */
int             non_table=0;       /* Set TRUE if instance is NON_TABLE    */
int             null_value=0;      /* Use for encoding a ASN1_C_NULL value */
snmp_instance_t *fake_inst;        /*-> "fake" SNMP Instance block for     */
                                   /*    holding non-table instance info   */
int             os_size;           /* size of printable/octet string       */


/* show "build status" as SUBELEMENT_OK_CONVERT */
bstatus = SUBELEMENT_OK_CONVERT;

/* grab pointer to actual MIR instance block we're going to reference */
MIR_inst = *inst;

/* zap local octet_string datastructure to zero/null */
os.length = 0;  os.string = NULL;  os.data_type = ASN1_C_NULL;

/*
| This is the high class hack.  Basically we're circumventing any logic that
| monkeys with instance arcs.
*/

/*
|  This is the code to remove when expanding the instance AVL code to actually
|  perform 'real' conversions on instance arcs for 'higher' containing classes.
|  As it is now, we simply check to be sure it's NULL and then do the right
|  thing.... otherwise if the datatype is not NULL, we log it and do the right
|  thing.
*/

/* if (snmp_higher_class_hack is TRUE) */
if (snmp_higher_class_hack == TRUE) {

    /* if (MIR instance tag is not ASN1_C_NULL) */
    if (MIR_inst->AVLtag != ASN1_C_NULL) {
        char xmsg[LINEBUFSIZE];  /* Local buffer for first msg build */

        sprintf(xmsg,
                MSG(msg160, "M160 - High Class tag value(%x hex) in (%%s), defaulting to ASN1_C_NULL"),
                MIR_inst->AVLtag
                );
        BLD_OID_ERRMSG( xmsg, snmp_oid); /* Add in actual OID via this call */
        SYSLOG( LOG_ERR, msg );
        }

    /* if (attempt to add default higher class element to AVL failed) */
    if (pdu == SET)
        modifier = MAN_C_SET_MODIFY; /* must be accurate for a SET request */
    else /* modifier is just MAN_C_SUCCESS for GET & GET-NEXT here */
        modifier = MAN_C_SUCCESS;  
    if ((status = moss_avl_add(inst_AVL,
                               &MIR_inst->inst_oid,
                               modifier,
                               ASN1_C_NULL,
                               &os)) != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg063, "M063 - Add of Element to Instance AVL element failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }

    /* advance the pointer to the MIR instance block address */
    *inst = MIR_inst->next;

    return(SUBELEMENT_OK_CONVERT);
    }

switch (MIR_inst->AVLtag) {        /* MIR instance info ASN.1 Tag */

    case ASN1_C_NULL:
        /*  If we get here and the MIR instance list element is the "fake"
         |  one just added, we can skip it and return since we've already
         |  processed it into the instance AVL.
         */

        /* this check will only ever succeed for a GETNEXT of a non-table */
        if (MIR_inst->inst_oid.value[2] != ECMA_CODE)
        {
            /* advance the pointer to the MIR instance block address */
            *inst = MIR_inst->next;  /* will ALWAYS be NULL */
            if (*arcs_converted == 0)
                return (DONE_IGNORE_INSTANCE);
            else
                return(SUBELEMENT_OK_CONVERT);
        }

        /*  Make note of the fact that the object is a scalar (NON-TABLE),
         |  as the ASN1_C_NULL datatype can only ever occur for scalars
         |  (once we've got past the class hierarchy information in the
         |  MIR instance list).  We'll use this flag at the end of this
         |  function to help determine whether to create a "fake" MIR instance
         |  block entry to help the pei entry point handle the non-table
         |  instance arc information that it would normally not ever see.
         |  (See carecv_avl_to_oid() in SNMPPE_CARECV.C; the ASN1_C_NULL
         |  is handled special just for this case.)
         */
        non_table = 1;

        /* if (at least one instance arc does NOT remain) */
        if (*i_arcs_left < 1 ) 
        {
            IFLOGGING( L_OIDANOMLY ) {
                BLD_OID_ERRMSG( MSG(msg072, 
           "M072 - No Arcs left: conversion fails/not done on (%s)"), snmp_oid);
                LOG( L_OIDANOMLY, msg);
                }

            if (pdu == GET || pdu == SET)
                return (SUBELEMENT_FAIL_CONVERT);
            else
                bstatus = DONE_IGNORE_INSTANCE;
        }

        /* else if (arc is not 0) */
        else if (snmp_oid->value[*i_arc] != 0)
        {
            if (pdu == GET || pdu == SET)
                return (SUBELEMENT_FAIL_CONVERT);
            else
                bstatus = DONE_IGNORE_INSTANCE;
        }

        /* if instance arc(s) are OK (well formed) */
        if (bstatus != DONE_IGNORE_INSTANCE)
        {
            os.length = 0;                   /* Signals 'null' data */
            os.string = NULL;                /* (likewise)          */
            os.data_type = MIR_inst->AVLtag;

            /* show one less instance arc remaining for processing */
            BUMP_ARC_INDICES();
            *arcs_converted = 1;
        }

        break;

    case INET_C_SMI_COUNTER:
    case INET_C_SMI_GAUGE:
        /* NOTE: These values are UNSIGNED! */
    case ASN1_C_INTEGER:
        /* NOTE: Integers are SIGNED! */

        /* if (at least one instance arc does NOT remain) */
        if (*i_arcs_left < 1 ) {
            IFLOGGING( L_OIDANOMLY ) {
                BLD_OID_ERRMSG(
                    MSG(msg064, "M064 - No Arcs left: conversion fails on (%s)"), snmp_oid);
                LOG( L_OIDANOMLY, msg);
                }

            if (pdu == GET || pdu == SET) {
                return (SUBELEMENT_FAIL_CONVERT);
	        }
            else {
                bstatus = DONE_IGNORE_INSTANCE;
                break;
                }
            }

        /* set octet_string length to size of integer */
        os.length = sizeof(int );

        /* set octet_string data pointer to next instance arc */
        os.string = (char *) &snmp_oid->value[*i_arc];

        /* set octet_string data type given value */
        os.data_type = MIR_inst->AVLtag;

        /* show one less instance arc remaining for processing */
        BUMP_ARC_INDICES();
        *arcs_converted = 1;
        break;


    case ASN1_C_PRINTABLE_STRING:
    case ASN1_C_OCTET_STRING:
        /*
        | We assume that the next section of instance arcs is of the
        | "counted" type, where the OID arc at "i_arc" contains a number
        | that is the count of arcs *after* the "i_arc"th arc that need
        | to be transferred.
        |
        | We create a dynamic allocated string of the appropriate length,
        | copy the arcs while checking to be sure they aren't bigger in
        | value than what can be represented in a byte.
        |
        | ================================================================
        | Check to be sure that the number of arcs specified by the
        | (next) 'length' arc actually exist in the SNMP OID we've got.
        | (It's a down-and-dirty examination of the object_id structure
        |  fields. . .)
        |
        |   <the length arc value> > (count of remaining arcs less length
        |                                              arcs)
        */

        if (*i_arcs_left > 1)
        {
            os_size = snmp_oid->value[*i_arc]; /* get length of octet string */
            if ( (os_size <= 0) || (os_size > *i_arcs_left-1) )
                os_size = 0;
        }
        else /* there are no useful instance arcs */
            os_size = 0; /* even one instance arc is not useful in this case */

        /* if there are no useful instance arcs, don't try to access the    */
        /* octet string's instance information (prevents bad memory access) */
        if (os_size == 0) 
        {
            IFLOGGING(L_OIDANOMLY) 
            {
                char xmsg[LINEBUFSIZE];  /* Local buffer for msg build */

                sprintf (xmsg, MSG(msg065, 
                  "M065 - Length Arc #%d invalid '%d': convert fails on (%%s)"),
                         *i_arc, snmp_oid->value[*i_arc]);
                BLD_OID_ERRMSG (xmsg, snmp_oid);
                LOG (L_OIDANOMLY, msg);
            }

            /* remaining arcs are useless (if any); set value to 0 */
            *i_arcs_left = 0;

            if (pdu == GET || pdu == SET) 
            {
                return (SUBELEMENT_FAIL_CONVERT);
            }
            else 
            {
                bstatus = DONE_IGNORE_INSTANCE;
                break;
            }
        }

        /* if attempt to allocate storage failed */
        if ((os.string = (char *) malloc(snmp_oid->value[*i_arc])) == NULL){
            CRASH(MSG(msg066, "M066 - Memory exhausted"));
            }

        i = os.length = snmp_oid->value[*i_arc]; /* Copy length to octet */
        BUMP_ARC_INDICES();

        /* Copy and Convert each instance arc */
        {
            char    *target;            /* Where we copy the arcs */

            target = os.string;        /* Where the arc goes */

            while (*i_arcs_left > 0 && i > 0) {
                if (snmp_oid->value[*i_arc] > 0xFF) {
                    IFLOGGING(L_OIDANOMLY) {
                        char xmsg[LINEBUFSIZE];  /* Local buffer for msg bld */

                        sprintf(xmsg,
                            MSG(msg067, "M067 - Instance Arc #%d too large '%d': (%%s)"),
                                *i_arc, snmp_oid->value[*i_arc]);
                        BLD_OID_ERRMSG( xmsg, snmp_oid);
                        LOG( L_OIDANOMLY, msg);
                        }

                    if (pdu == GET || pdu == SET) {
                        return (SUBELEMENT_FAIL_CONVERT);
                        }
                    else {
                        free_os = TRUE;
                        bstatus = DONE_IGNORE_INSTANCE;
                        break;
                        }
                    }

                /* Copy OID arc into octet string storage */
                *target++ = (char) snmp_oid->value[*i_arc];
                BUMP_ARC_INDICES();
                *arcs_converted = 1;
                i -= 1;                 /* Decrement total left in this copy */
                }
        }

        if (bstatus != DONE_IGNORE_INSTANCE) {
            /* Load the Datatype to the octet string structure */
            os.data_type = MIR_inst->AVLtag;
            free_os = TRUE;
	    }
        break;

    case INET_C_SMI_IP_ADDRESS:
        /* "Tip 'o the Hat" to Mary Walker */

        /* if (not enough arcs remain for IP address) */        
        if (*i_arcs_left < 4) {
            IFLOGGING(L_OIDANOMLY) {
                char xmsg[LINEBUFSIZE];  /* Local buffer for msg bld */

                sprintf(xmsg,
               MSG(msg070, "M070 - Not enough instance arcs left (%d) for IP Address(%%s)"),
                        *i_arcs_left);
                BLD_OID_ERRMSG(xmsg,snmp_oid);
                LOG( L_OIDANOMLY, msg);
                }

            /* remaining arcs are useless (if any); set value to 0 */
            *i_arcs_left = 0;

            if (pdu == GET || pdu == SET) {
                return (SUBELEMENT_FAIL_CONVERT);
                }
            else {
                bstatus = DONE_IGNORE_INSTANCE;
                break;
                }
            }

        /* build host word w/IP address from 4 instance arcs */
        for (i = 0; i < sizeof(ip_addr); i++) {

            /* If the arc value is too big. . . */
            if (snmp_oid->value[*i_arc] > 0xFF) {
                IFLOGGING(L_OIDANOMLY) {
                    char xmsg[LINEBUFSIZE];  /* Local buffer for msg bld */

                    sprintf(xmsg,
                        MSG(msg237, "M237 - Instance Arc #%d too large '%d': (%%s)"),
                            *i_arc, snmp_oid->value[*i_arc]);
                    BLD_OID_ERRMSG( xmsg, snmp_oid);
                    LOG( L_OIDANOMLY, msg);
                    }

                if (pdu == GET || pdu == SET) {
                    return (SUBELEMENT_FAIL_CONVERT);
                    }
                else {
                    bstatus = DONE_IGNORE_INSTANCE;
                    break;
                    }
                }

            ip_addr <<= 8;
            ip_addr |= snmp_oid->value[*i_arc];
            BUMP_ARC_INDICES();
            *arcs_converted = 1;
            }

        if (bstatus != DONE_IGNORE_INSTANCE) {
            /* convert from network to host layout */
            ip_addr = ntohl(ip_addr);

            /* initialize octet_string to point to conversion w/length */
            os.length = sizeof(ip_addr);
            os.string = (char *)&ip_addr;
            os.data_type = MIR_inst->AVLtag;
	    }

        break;

    default:
        IFLOGGING(L_OIDANOMLY)
            {
            char xmsg[LINEBUFSIZE]; /* First pass of error msg goes here     */
            sprintf(xmsg,           /* NOTE: "%%s" goes to "%s" on this call */
            MSG(msg073, "M073 - Unsupported Instance Arc #%d translation for Tag %d, (%%s)"),
                    *i_arc, MIR_inst->AVLtag);
            BLD_OID_ERRMSG( xmsg, snmp_oid ); /* %s converts to OID here */
            LOG( L_OIDANOMLY, msg);
            }

        /* remaining arcs are useless (if any); set value to 0 */
        *i_arcs_left = 0;

        if (pdu == GET || pdu == SET) {
            return (SUBELEMENT_FAIL_CONVERT);
            }
        else {
            bstatus = DONE_IGNORE_INSTANCE;
            break;
            }
    }

/* set up modifier field based on pdu type and the instance arc conversion */
if (pdu == SET)
    modifier = MAN_C_SET_MODIFY; /* must ALWAYS be this for a SET request */

else /* modifier is otherwise ignored for GET & GET-NEXT */
    modifier = MAN_C_SUCCESS;  

/* if (attempt to add an element to AVL failed) */
if ((status = moss_avl_add(inst_AVL,
                           &MIR_inst->inst_oid,
                           modifier,
                           MIR_inst->AVLtag,
                           &os)) != MAN_C_SUCCESS) {
    sprintf(msg,
            MSG(msg069, "M069 - Add of subelement to Instance AVL element failed (%s)"),
            get_man_status(status));
    CRASH( msg );
    }

/* if (we should free the octet string value) */
if (free_os == TRUE) {
    free(os.string);
    }

/*  If the object is a NON-TABLE, and we are at the end of the MIR instance
 |  list, and the tag of this last instance list element is ASN1_C_NULL, then
 |  this is a GETNEXT operation (GETs and SETs fail if anything goes wrong
 |  during the arc conversion).  We need to attach a "fake" instance list
 |  entry for the pei entry point to see when it is converting the avl
 |  back into an OID;  this instance list entry is normally not derived by
 |  the MIR, but it is essential for the MOMs to be able to determine "good"
 |  instance information from "bad" instance information for NON-TABLES.
 |
 */
if ((non_table == 1) && (MIR_inst->next == NULL) &&
    (MIR_inst->AVLtag == ASN1_C_NULL))
{
    /* tack on a "fake" instance block for the pei side to use */
    if ( (fake_inst = (snmp_instance_t *) malloc(sizeof(snmp_instance_t))) == NULL) 
    {
        CRASH(MSG(msg297, "M297 - memory exhausted"));
    }

    /* set up the "new" last MIR instance list block */
    fake_inst->next = NULL;              /* initialize "next" field to null    */
    fake_inst->next_class = NULL;        /* initialize "next_class" field null */
    fake_inst->class_code = -1;          /* initialize "class_code" field null */

    /* copy length and dynamically allocate storage for instance object ID */
    fake_inst->inst_oid.count = vbe->snmp_oid.count;
    fake_inst->inst_oid.value = (unsigned int *)
                   malloc(sizeof(unsigned int) * fake_inst->inst_oid.count);
    if (fake_inst->inst_oid.value == NULL) 
    {
        CRASH(MSG(msg300, "M300 - memory exhausted"));
    }

    /* Copy SNMP OID arcs and set AVLtag to ASN1_C_NULL */
    for (j=0; j < fake_inst->inst_oid.count; j++) 
        fake_inst->inst_oid.value[j] = vbe->snmp_oid.value[j];

    fake_inst->AVLtag = ASN1_C_NULL;

    /* make the "old" last element point to the "new" last element */
    MIR_inst->next_class = fake_inst;

    /* if overall conversion of instance arc failed, leave octet string empty */
    if (bstatus == DONE_IGNORE_INSTANCE)
    {
        os.length = 0;
        os.string = NULL;
    }
    else /* stuff a "valid" NULL value into the octet string */
    {
        os.length = 4;                     /* normally 0! */                 
        os.string = (char *) &null_value;  /* normally NULL! */
    }
    os.data_type = ASN1_C_NULL;

    /* attach the attribute oid and the assoc. octet to the avl subelement */
    if ((status = moss_avl_add (inst_AVL,
                                &fake_inst->inst_oid,
                                (unsigned int) MAN_C_SUCCESS,
                                fake_inst->AVLtag,
                                &os)) != MAN_C_SUCCESS)
    {
        sprintf(msg,
                MSG(msg298, "M298 - Add of subelement to Instance AVL element failed (%s)"),
                get_man_status(status));
        CRASH(msg);
    }
}

/*  Advance the pointer to the MIR instance block address.  Note that
 |  we may have just set the pointer to point to the "fake" MIR instance
 |  block entry we just added; this will fake element will get bypassed
 |  under the switch statement above since the attribute oid will NOT
 |  contain the ECMA_CODE that is normally present in MIR instance blocks
 |  with an AVL tag of ASN1_C_NULL.  This may not be pretty, but it is simple
 |  AND it works.  Besides, we wouldn't have to go through all this if...
 |  ...oh, never mind.
 */
*inst = MIR_inst->next;

if (bstatus == DONE_IGNORE_INSTANCE)
    return(DONE_IGNORE_INSTANCE);
else
    return(SUBELEMENT_OK_CONVERT);

}

/* casend_bld_attribute_avl - Build the Attribute List AVL */
/* casend_bld_attribute_avl - Build the Attribute List AVL */
/* casend_bld_attribute_avl - Build the Attribute List AVL */

static man_status
casend_bld_attribute_avl( bp, snmp_oid, class_oid, dstatus, tag, data,
                          p_attr_AVL, pdu)

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE  */
object_id       *snmp_oid;      /*-> Full SNMP OID we're working on     */
object_id       *class_oid;     /*-> OID of the class we're working on  */
mir_derive_CI_status
                dstatus;        /* Class/Instance Derivation status     */
int             tag;            /* ASN.1 Tag read from varbind list     */
octet_string    *data;          /*->AVL octet_string from varbind list  */
avl             **p_attr_AVL;   /*->> Pointer to where we build AVL     */
pdu_type        pdu;            /* PDU type (GET, SET, GETNEXT)         */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "snmp_oid" is the object id of the SNMP variable we're working on
    for this request.

    "class_oid" is the object id of the class of the SNMP  variable 
    we're working on for this request, as obtained from the MIR.

    "dstatus" indicates (essentially) whether or not we rolled before
    settling on the class.  If we did, the attribute AVL is just empty,
    otherwise we'll attempt to build an AVL if the "snmp_oid" was long enough
    to have an attribute arc.

    "tag" is the ASN.1 tag for any data associated with the varbind entry.

    "data" is the data from the varbind entry. (The MOM cares only if SET).

    "p_attr_AVL" is the attribute list AVL we're being asked to build for
    transmission to the Common Agent.  The pointer whose address is passed
    for this AVL should not be initialized to anything, as it will be set
    to NULL before the AVL is created.

    "pdu" is the type of PDU that is being serviced at this time.  The PDU
    type can be one of: GET, SET, or GETNEXT.  The PDU type is required for
    setting up the modifier field in the moss_avl_add() call;  the modifier
    MUST have a valid meaning for SET requests, and for GET and GETNEXT
    requests, it is ignored in the attribute AVL.


OUTPUTS:

    On successful return, "attr_AVL" avl has been created with
    a construction containing one element to represent the data
    from the varbind entry.  MAN_C_SUCCESS is returned.

    On failure, messages may have been logged, the AVL is not initialized
    (it is freed) and MAN_C_FAILURE is returned.


BIRD'S EYE VIEW:
    Context:
        The caller is one of "casend_process_*()" functions in this module.
        All necessary information has been extracted from the MIR for
        a varbind entry element, and the Attribute AVL for the
        Common Agent msi_* call needs to be built.

    Purpose:
        This function merges the MIR info and original SNMP OID to
        create a single-entry on the attribute list AVL.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the AVL pointer NULL>
    if (attempt to init the AVL failed)
        <CRASH "Mxxx - Attribute List AVL initialization failed %d">

    if (dstatus is "CI_ROLLED_CLASS" or "CI_ROLLED_ATTRIBUTE")
        <return MAN_C_SUCCESS>

    <obtain the length of the class object id>
    <obtain the length of the original object id>

    if (length of the class object id is < original object id length)

        if (attempt to create a temporary OID with one arc in it
            containing the attribute arc FAILED)
            <CRASH "Mxxx - Temporary Object Id creation failed %d">

        if (attempt to create the attribute OID by concatenation failed)
            <CRASH "Mxxx - Attribute Object Id creation failed %d">

        if (attempt to free the temporary OID failed)
            <CRASH "Mxxx - Temporary Object Id deletion failed %d">

        if (ASN.1 Tag is valid)
            if (attempt to do an AVL ADD with tag and data FAILED )
                <CRASH "Mxxx - AVL Add failed %d">
        else
            if (attempt to do an AVL ADD with no tag and no data FAILED )
                <CRASH "Mxxx - AVL Add failed %d">

        if (attempt to free the attribute OID failed)
            <CRASH "Mxxx - attribute Object Id deletion failed %d">

    <return MAN_C_SUCCESS>

OTHER THINGS TO KNOW:

    Tip 'o the Hat to Mary Walker on this one too.
*/

{
int     class_len;      /* Length of the Class OID              */
int     snmp_len;       /* Length of the original SNMP OID      */

man_status   status;    /* General purpose status holder        */
object_id    *temp_oid; /* General purpose OID holder           */
object_id    *attr_oid; /* Attribute List AVL OID               */
char  msg[LINEBUFSIZE]; /* Error Message build buffer           */
unsigned int modifier;  /* Modifier value for AVL               */


/* initialize the AVL pointer NULL and the modifier value (based on PDU type) */
*p_attr_AVL = NULL;
if (pdu == SET)
    modifier = MAN_C_SET_MODIFY; /* must be accurate for SET request */
else /* modifier ignored for GET & GET-NEXT here */
    modifier = MAN_C_SUCCESS;  

/* if (attempt to init the AVL failed) */
if ((status = moss_avl_init(p_attr_AVL)) != MAN_C_SUCCESS) {
    sprintf(msg,
            MSG(msg078, "M078 - Attribute list AVL initialization failed (%s)"),
            get_man_status(status));
    CRASH(msg);
    }

/* if (dstatus is "CI_ROLLED_CLASS" or "CI_ROLLED_ATTRIBUTE") */
if (dstatus == CI_ROLLED_CLASS || dstatus == CI_ROLLED_ATTRIBUTE) {
    return(MAN_C_SUCCESS);
    }

class_len = class_oid->count;   /* obtain the length of the class object id */
snmp_len = snmp_oid->count;     /* obtain the length of the original oid    */

/* if (length of the class object id is < original object id length) */
if (class_len < snmp_len) {

    /*
    | if (attempt to create a temporary OID with one arc in it
    |     containing the attribute arc FAILED)
    */
    if ((status = moss_create_oid(1, &(snmp_oid->value[class_len]), &temp_oid))
        != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg079, "M079 - Temporary Object Id creation failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }

    /* 
    | if (attempt to create the attribute OID by concatenation failed)
    */
    if ((status = moss_concat_oids(class_oid, temp_oid, &attr_oid))
        != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg080, "M080 - Attribute Object Id creation failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }

    /* 
    | if (attempt to free the temporary OID failed)
    */
    if ((status = moss_free_oid(temp_oid)) != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg081, "M081 - Temporary Object Id deletion failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }

    /* if (ASN.1 Tag is valid) */
    if (tag != 0) {
        /* if (attempt to do an AVL ADD with tag and data FAILED ) */
        if ((status = moss_avl_add(*p_attr_AVL, attr_oid, modifier,
				   tag, data)) != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg082, "M082 - AVL Add failed (%s)"), get_man_status(status));
            CRASH( msg );
            }
        }
    else {
        /* if (attempt to do an AVL ADD with no tag and no data FAILED ) */
        if ((status = moss_avl_add(*p_attr_AVL, attr_oid, modifier,
				   NULL, NULL))
             != MAN_C_SUCCESS) {
            sprintf(msg, MSG(msg083, "M083 - AVL Add failed (%s)"), get_man_status(status));
            CRASH( msg );
            }
        }

    /* if (attempt to free the attribute OID failed) */
    if ((status = moss_free_oid(attr_oid)) != MAN_C_SUCCESS) {
        sprintf(msg,
                MSG(msg084, "M084 - Attribute Object Id deletion failed (%s)"),
                get_man_status(status));
        CRASH( msg );
        }
    }

return (MAN_C_SUCCESS);
}


/* casend_bld_access_control_avl - Build the Access Control AVL */
/* casend_bld_access_control_avl - Build the Access Control AVL */
/* casend_bld_access_control_avl - Build the Access Control AVL */

static man_status
casend_bld_access_control_avl( bp, svc, p_access_control_AVL )

big_picture_t	*bp;		/*-> Big Picture Structure for SNMP PE  */
service_t	*svc;		/*-> Service Block (on Service List) */
				/* to be processed.                  */
avl		**p_access_control_AVL;	/*->> Pointer to where we build AVL  */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "svc" is the address of a pointer (to a Service Block) that corresponds
    to the received PDU.

    "p_access_control_AVL" is the access control AVL we're asked to build for
    transmission to the Common Agent.  The pointer whose address is passed
    for this AVL should not be initialized to anything, as it will be set
    to NULL before the AVL is created.


OUTPUTS:

    On successful return, "p_access_control_AVL" avl has been created with
    three avl elements for access control check. The first avl element is
    the type of access control. This is SNMP_ACCESS for SNMP PE. The second
    avl element is the community name passed in the PDU. The third avl
    element is the ip address of the requester.  MAN_C_SUCCESS is returned.

    On failure, messages may have been logged, the AVL is not initialized
    (it is freed) and we called CRASH to abort.


BIRD'S EYE VIEW:
    Context:
	The caller is one of "casend_process_GET", "casend_process_SET" and
	"casend_process_GETNEXT" functions in this module. All the
	information for building the access control AVL is in the service
	block passed in.

    Purpose:
	This function creates the access control AVL for call to CA.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the AVL pointer NULL>
    if (attempt to init the AVL failed)
	<CRASH "Mxxx - Access Control AVL initialization failed %d">

    <prepare the type of access control>
    if (attempt to do an AVL ADD failed)
	<CRASH "Mxxx - Access Control AVL add failed %d">

    if (attempt to allocate memory for community name failed)
	<CRASH "Mxxx - No memory fo Access Control AVL community name">

    <prepare the community name>
    if (attempt to do an AVL ADD failed)
	<CRASH "Mxxx - Access Control AVL add failed %d">

    <prepare the ip address>
    if (attempt to do an AVL ADD failed)
	<CRASH "Mxxx - Access Control AVL add failed %d">

    <return MAN_C_SUCCESS>

*/
        
{
man_status   status;    /* General purpose status holder        */
char  msg[LINEBUFSIZE]; /* Error Message build buffer           */
octet_string data;      /* octet_string for moss_avl_add()      */
int      i;
char *p_comm_name = NULL;  /* Pointer to community name created */

/* initialize the AVL pointer NULL */
*p_access_control_AVL = NULL;

/* if (attempt to init the AVL failed) */
if ((status = moss_avl_init(p_access_control_AVL)) != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg280,
	    "M280 - Access Control AVL initialization failed %d"), status);
    CRASH(msg);
    }

i = SNMP_ACCESS;
data.length = sizeof(int     );
data.data_type = ASN1_C_INTEGER;
data.string = (char *) &i;

status = moss_avl_add(*p_access_control_AVL, &nil_object_id, MAN_C_SUCCESS,
		      ASN1_C_INTEGER, &data);

if (status != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg281,
	    "M281 - Access Control AVL add failed %d"), status);
    CRASH(msg);
    }

if ( (p_comm_name = (char *) malloc(svc->comm_namelen + 1)) == NULL) {
    CRASH(MSG(msg282, 
	  "M282 - No memory for Access control AVL community name"));
    }

data.length = svc->comm_namelen + 1;
memset(p_comm_name, '\0', data.length);
memcpy(p_comm_name, svc->comm_name, svc->comm_namelen);

data.data_type = ASN1_C_OCTET_STRING;
data.string = (char *) p_comm_name;

status = moss_avl_add(*p_access_control_AVL, &nil_object_id, MAN_C_SUCCESS,
		      ASN1_C_OCTET_STRING, &data);

free(p_comm_name);

if (status != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg283,
	    "M283 - Access Control AVL add failed %d"), status);
    CRASH(msg);
    }

data.length = sizeof(unsigned int);
data.data_type = ASN1_C_INTEGER;
data.string = (char *) &svc->inbound.sin_addr;

status = moss_avl_add(*p_access_control_AVL, &nil_object_id, MAN_C_SUCCESS,
		      ASN1_C_INTEGER, &data);

if (status != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg284,
	    "M284 - Access Control AVL add failed %d"), status);
    CRASH(msg);
    }

return(MAN_C_SUCCESS);

}  /* end of casend_bld_access_control_avl() */


/* casend_update_avl_modifier - Update Instance AVL Top Level Modifier Value */
/* casend_update_avl_modifier - Update Instance AVL Top Level Modifier Value */
/* casend_update_avl_modifier - Update Instance AVL Top Level Modifier Value */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
     context information needed by SNMP PE to operate.

    "inst_AVL" points to the Instance AVL that is to be updated.

    "modifier" is the value of the modifier to be stored into the top-most
     AVL element of the Instance AVL.


OUTPUTS:

    On return, the Instance AVL has been updated with the
    given modifier value.

    On failure, a message has been logged and the AVL has NOT been updated.
    The process is terminated.


BIRD'S EYE VIEW:
    Context:
	The caller is the "casend_build_instance_avl" function in this module. 

    Purpose:
	This function updates the modifier value of the top-most AVL element
        of the Instance AVL.  If the AVL is messed up, we log a message & CRASH.
*/

static void
casend_update_avl_modifier (bp, inst_AVL, modifier)
 big_picture_t  *bp;           /*-> Big Picture Structure for SNMP PE  */
 avl            *inst_AVL;      
 unsigned int    modifier;
{
    iavl         *i_avl_ptr;
    avl_element  *element_ptr;
    char          msg[LINEBUFSIZE];   /* Error Message build buffer    */


    /* get to the first AVL element; this should be the Start-Of-Sequence */
    i_avl_ptr   = (iavl *) inst_AVL;
    element_ptr = i_avl_ptr->first_avl;

    if (element_ptr->tag != ASN1_C_SEQUENCE)
    {
        sprintf(msg, MSG(msg327,
             "M327 - Instance AVL modifier cannot be updated; AVL is botched"));
        CRASH( msg );
    }

    element_ptr->modifier = modifier;
    
} /* end of casend_update_avl_modifier() */
