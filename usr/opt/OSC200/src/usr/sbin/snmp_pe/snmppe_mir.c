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
static char *rcsid = "@(#)$RCSfile: snmppe_mir.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:25:21 $";
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
 * Module SNMPPE_MIR.C
 *      Contains "Common Agent - MIR Inquiry" for the SNMP Protocol Engine
 *      for the Comman Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   October 1991
 *      with a big Tip 'o the Hat to Mary Walker/UEG after whose MIR routines
 *      some of these MIR inquiry routines are patterned.
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accept requests over a network,
 *       convert incoming information into standard Common Agent format
 *       and call the Common Agent for processing of that information.
 * 
 *       - When the PDU is parsed, certain MIR-based information may be
 *         required to correctly handle it.
 *
 *       - When a request is translated to/from 'Common Agent format',
 *         MIR-based information is required to obtain instance encoding or
 *         decoding information.
 *
 *    Purpose:
 *       This module contains the functions which acquire the necessary
 *       information from the MIR to support these SNMP PE requirements.
 *
 *       These are special purpose Tier 1 and Tier 2 functions for use
 *       only be SNMP PE.
 *
 * History
 *      V1.0    October 1991    D. D. Burns
 *      V1.1    April 1992      D. D. Burns
 *                              Fix design flaw that allowed for
 *                                 roll from last class in all cases when roll
 *                                 should occur from original OID the first
 *                                 time (not from the Class OID in that OID!)
 *                                          
 *      V1.2    May 1992        D. D. Burns - another attempt to fix
 *                                 the problem described above, since we now
 *                                 realize that rolls must always be done from
 *                                 the last 'object' regardless of whether it
 *                                 was a class or attribute
 *      V1.3    July 1992       D. D. Burns - Changes to support V1.96 of MIR
 *                                 subsystem (ability to specify OID SMIs)
 *                                 plus TRACE support.
 *                                          
 *      V1.4    August 1992     D. D. Burns - Changes to support proper rolling
 *                                 in multiple SMI (OID) environment
 *
 *      V1.5    Sept. 1992      D. D. Burns - Addition of logic to allow for
 *                                 a fast roll to the next CLASS to eliminate
 *                                 toxic overhead from rolling attribute-by-
 *                                 attribute to the next CLASS (this can occur
 *                                 if a MOM wasn't registered in MOLD but was
 *                                 represented in the MIR).

Module Overview:
---------------

This module contains the SNMP PE function(s) that query the MIR.


Thread Overview:
---------------

With V1.0 configuration of the SNMP PE, all the functions in this module
happen to be executed exclusively by the main ("sending") thread.

Nonetheless, the construction of any function in this module must
not preclude that a function may be called 'simultaneously' by multiple
sending threads.

(This requirement is met pretty simply by observing the SNMPPE coding
 convention precluding the use of static data).



MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------
mir_class_inst_GETSET   Performs a MIR lookup to discover the object identifier
                        for the SNMP object class corresponding to a supplied
                        full SNMP object identifier.  Instance information is
                        also returned.  (This function corresponds very closely
                        to Mary Walker's "find_class_info()" function).
                        For use when request is GET or SET.

mir_class_inst_GETNXT   Same as _GETSET above but for use when request is
                        GET-NEXT.

mir_class_inst_ROLLNXT  Performs a "roll" to the next object (attribute or
                        class) given an Object Id for the last 'thing'.  This
                        supports GET-NEXT SNMP requirements.  (This function
                        corresponds roughly to Mary Walker's
                        "find_next_class_info()", although it is not called
                        in all the situations where her's was).

mir_get_asn1_tag        Performs a MIR lookup to support ASN.1 decoding for
                        functions in "snmp_lib.c". (Since it uses existing
                        MIR Tier 1 functions, it is a Tier 2 function).


MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------
load_local_mandles      Loads mandles to module-local static areas for
                        use and re-use by functions in this module.  These
                        are thread-safe.

mir_derive_CI           Derive Class/Instance information by working from a
                        MIR object to decide whether it is a SNMP "class"
                        object or "attribute", then records the "class OID"
                        for its caller.  It then finds the class MIR object
                        (if it wasn't passed in explicitly) and extracts
                        instance information and returns it in a linked list of
                        "snmp_instance_t" structures.

mir_derive_CLASS        Supports "mir_derive_CI()" above by performing the
                        Class analysis.

mir_derive_INSTANCE     Supports "mir_derive_CI()" above by performing the
                        Instance extraction from the MIR.

mir_build_one_instance  Supports "mir_derive_INSTANCE()" by processing one
                        occurrence of an instance of an identifying attribute
                        in the MIR.

mir_build_null_instance Supports "mir_derive_INSTANCE()" by creating a 
                        "standard" instance block for classes not actually
                        having identifying attribute information in the MIR.

mir_null_inst_oid_patchup  This function supports "mir_derive_INSTANCE()" by
                        running down the linked list created by it and
                        stuffing in valid OIDs in instance blocks created
                        by "mir_build_null_instance()" (which didn't have
                        enough information at the time to do it).
*/

/* Module-wide Stuff */

#if defined(__osf__) && !defined(_OSF_SOURCE)
/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
   typedefs unless you've got _OSF_SOURCE turned on. */
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE

#else
#include <sys/types.h>
#endif

#include <stdio.h>
#include <syslog.h>
#include <malloc.h>
#include "moss.h"

/* Suppress the inclusion of definition of "object_id" in "mir.h" */
#define OID_DEF
#include "mir.h"

/* includes required for "snmppe.h" */
#include "port_cmip.h"
#include <stdio.h>
#include <netinet/in.h>
#include "snmppe.h"


/*
|==============================================================================
|  Derive Class Style
|
|    This enumerated type serves to define an argument to mir_derive_CI() and
|    it minion mir_derive_CLASS().  These codes are used only within this
|    module, consequently they are defined here instead of in snmppe.h
|
|    If you modify this list, change the corresponding ASCII interpretation
|    array below!
|==============================================================================
*/
typedef
    enum {
      ROLL_FOR_ATTRIB,  /* Roll on to next ATTRIBUTE from current MIR Object */
      ROLL_FOR_CLASS,   /* Roll on to next CLASS from current MIR Object     */
      ROLL_ALLOWED,     /* MIR Object unknown, roll is ALLOWED to get next   */
                        /*   class or attribute IF needed                    */
      NO_ROLL_AT_ALL    /* MIR Object should be an attribute and so          */
      } derive_style;   /*            rolling is NOT ALLOWED                 */


/*
|==============================================================================
|  Derive Class Style - String
|
|    This array provides an ASCII interpretation for the strings above for
|    use in TRACE debug statements.
|==============================================================================
*/
static
char    *derive_style_string[4] = {
        "ROLL_FOR_ATTRIB",
        "ROLL_FOR_CLASS",
        "ROLL_ALLOWED",
        "NO_ROLL_AT_ALL"};

/*
|==============================================================================
| OID SMI names
|
|  For use in TRACE messages
|==============================================================================
*/
static char * oid_smi_names[6] = {
    "OID_ANY",         /* (Request Code, not an SMI name...see "mir.h" */
    "OID_MCC",
    "OID_OID",
    "OID_DNA",
    "OID_OSI",
    "OID_SNMP" };


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
|==============================================================================
| CI_status_string - Defined externally to this module in snmppe_casend.c
|==============================================================================
*/
extern char *CI_status_string[];

/*
|
|   Define Prototypes for Module-Local Functions
|
*/

/* call_load_local_mandles - Load Modules Local Mandle Pointers */
static void
call_load_local_mandles PROTOTYPE((
));

/* load_local_mandles - Load Modules Local Mandle Pointers */
static mir_status
load_local_mandles PROTOTYPE((
));

/* mir_derive_CI - Derive Class and Instance Information from MIR Object */
static mir_derive_CI_status
mir_derive_CI PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
object_id       *,              /* -> to where to store returned class oid   */
snmp_instance_t **,             /* ->> where to store the instance list      */
object_id       *,              /* -> to OID of "current object"             */
mir_oid_smi      ,              /* SMI in which current obj. OID resides     */
mandle          *,              /* Mandle describing MIR Object to be worked */
mandle_class    **,             /*-> Mandle Class containing "mir_object"    */
derive_style    ,               /* Class-Derivation Style                    */
BOOL                            /* TRUE: One roll already done               */
));

/* mir_derive_CLASS - Derive Class from MIR Object supplied */
static mir_derive_CI_status
mir_derive_CLASS PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
object_id       *,              /* -> to where to store returned class oid   */
object_id       *,              /* -> to OID of "current object"             */
mir_oid_smi      ,              /* SMI of where current obj OID resides      */
mandle          **,             /* Mandle describing MIR Object to be worked */
mandle_class    **,             /*-> Mandle Class containing "mir_object"    */
derive_style    ,               /* Class-Derivation Style                    */
BOOL                            /* TRUE: One roll already done               */
));

/* mir_derive_INSTANCE - Derive Instance Information from Class MIR Object */
static void
mir_derive_INSTANCE PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **,             /* ->> where to store the instance list      */
mandle          *,              /* Mandle describing MIR Object to be worked */
mandle_class    **              /*-> Mandle Class containing "mir_object"    */
));

/* mir_build_one_instance - Extract One Instance's worth of info to block */
static void
mir_build_one_instance PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **,             /* ->> to store the addr of next inst blk    */
mandle          *               /* Mandle describing MIR Object to be worked */
));

/* mir_build_null_instance - Build the Standard NULL Instance Info Block */
static void
mir_build_null_instance PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **              /* ->> to store the addr of next inst blk    */
));

/* mir_null_inst_oid_patchup - Patchup OID in Standard NULL Instance Block */
static void
mir_null_inst_oid_patchup PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t *               /* -> Top of Instance List                   */
));


/*         MODULE LOCAL MANDLES          */
/* (For use by functions in this module) */

/*
|  This "once" block assures "load_local_mandles()" is only called once
|  by the first thread that calls "mir_derive_CLASS()".
*/
#ifdef THREADS
static pthread_once_t  mir_init_once=pthread_once_init;
#endif

/*
| Module-local Relationship Mandles
|
| This array is loaded by "load_local_mandles()" at initialization time
| to contain mandles that other functions in this module need for their
| lookup chores.
|
| Mnemonic Indices for each entry in this array that 'point' to the
| corresponding Relationship mandle:
|
*/
#define M_STRUCT_AS 0   /* "MIR_Structured_As"          */
#define M_CONT_BY   1   /* "MIR_Contained_By"           */
#define M_INDEX_BY  2   /* "MIR_Indexed_By"             */
#define M_ID_CODE   3   /* "MIR_ID_Code"                */
#define M_TEXT_NAME 4   /* "MIR_Text_Name" (for trace)  */
#define LOCAL_MANDLE_COUNT 5
rel_pair        m_list[LOCAL_MANDLE_COUNT];

/*
| Pointer to the mandle class for all mandles in mandle list "m_list[]" above
*/
static mandle_class *local_class=NULL;

/*
| Handy-Dandy macro used in sending "TRACE" debug messages
*/
#define LOGIT() fprintf(bp->log_state.log_file, buf)

/* call_load_local_mandles - Load Modules Local Mandle Pointers */
/* call_load_local_mandles - Load Modules Local Mandle Pointers */
/* call_load_local_mandles - Load Modules Local Mandle Pointers */

static void
call_load_local_mandles()

/*
INPUTS:

    The pointer to the Big Picture structure stored in "snmppe_main.c"
    is implicit input to this routine, as we need this address to
    issue a CRASH if MIR initialization fails in a threaded environment.

OUTPUTS:

    The function returns or crashes depending on the return status from
    the "load_local_mandles()" call.

BIRD'S EYE VIEW:
    Context:
        The caller is "mir_instance_extract()" and it needs to access the MIR
        using "local mandles" to relationships.  These mandles must be
        initialized.

    Purpose:
        This function calls the function that performs the initialization
        and CRASHes if the initialization is not successful.
        This function can be used in a threaded environment.

ACTION SYNOPSIS OR PSEUDOCODE:

    <acquire Big Picture pointer>
    if (status from load_local_mandles is NOT success)
        <CRASH "M126 - MIR Tier 2 Initialization failed">

OTHER THINGS TO KNOW:

    This function serves mainly to "wrap" a non-thread safe function
    that is also unaware of the "big picture" in SNMPPE.
    
*/

{
big_picture_t   *bp;               /* Pointer for the Big Picture        */
char            msg[LINEBUFSIZE];  /* Error Message build buffer         */
mir_status      mstatus;           /* Return status from MIR init        */


/* Yep, External.  In "snmppe_main.c".                                   */
extern big_picture_t *big_picture;

/* acquire Big Picture pointer */
bp = big_picture;

/* if (status from load_local_mandles is NOT success) */
if ((mstatus = load_local_mandles()) != MS_SUCCESS) {
    sprintf(msg, MSG(msg126, "M126 - MIR Tier 2 Initialization failed %d"), mstatus);
    CRASH( msg );
    }
}

/* load_local_mandles - Load Modules Local Mandle Pointers */
/* load_local_mandles - Load Modules Local Mandle Pointers */
/* load_local_mandles - Load Modules Local Mandle Pointers */

static mir_status
load_local_mandles()

/*
INPUTS:

    The implicit inputs are the static local pointers to mandles listed
    at the beginning of this module.

OUTPUTS:

The function returns one of:

MS_SUCCESS - The local mandle pointers have been setup.

Additionally, this function can return all the error codes that the following
function may return:

    mir_get_rel_mandles() - Performs the actual lookups

BIRD'S EYE VIEW:
    Context:
        The caller is any function in this module, to set up relationship
        mandle pointers that may be needed.

    Purpose:
        This function accesses the MIR via the special "mir_get_rel_mandles()"
        to take care of the details of getting mandle pointers for
        relationship object needed by any function in this module.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set up mandle/name pair array for all desired mandles>
    if (status was MS_FAILURE)
        <return MS1_INIT_FAIL>
    <return status>

OTHER THINGS TO KNOW:

    This function uses the standard Tier 0 function to actually do the
    fetching of the mandles, and then stores them locally.

*/

{
mir_status status;      /* Status from mir_get_rel_mandles */

/* set up mandle/name pair array for all desired mandles */
m_list[M_STRUCT_AS].m = NULL;
m_list[M_STRUCT_AS].name = "MIR_Structured_As";

m_list[M_CONT_BY].m = NULL;
m_list[M_CONT_BY].name = "MIR_Contained_By";

m_list[M_INDEX_BY].m = NULL;
m_list[M_INDEX_BY].name = "MIR_Indexed_By";

m_list[M_ID_CODE].m = NULL;
m_list[M_ID_CODE].name = "MIR_ID_Code";

m_list[M_TEXT_NAME].m = NULL;
m_list[M_TEXT_NAME].name = "MIR_Text_Name";

/* if (attempt to get mandles succeeded) */
status = mir_get_rel_mandles(LOCAL_MANDLE_COUNT, m_list, &local_class);
if ( status == MS_FAILURE) {
    return(MS1_INIT_FAIL);
    }

return(status);
}

/* mir_class_inst_GETSET - Find SNMP Obj Class & Instance Info for GET/SET */
/* mir_class_inst_GETSET - Find SNMP Obj Class & Instance Info for GET/SET */
/* mir_class_inst_GETSET - Find SNMP Obj Class & Instance Info for GET/SET */

mir_derive_CI_status
mir_class_inst_GETSET (bp, vbe)

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
varbind_t       *vbe;           /*-> Varbind Entry Block for SNMP OID        */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "vbe" points to a varbind entry block.  Inside the vbe:

        "snmp_oid" is a pointer to an object-id structure containing the SNMP
        OID that the caller wants to have converted into class oid + instance
        information.  This field is input to this function.

        "class_oid" is the object-id structure to be set to the object id of
        the class (discovered by this function).  The "value" field of this
        object_id structure is overwritten on return, either by a NULL or by
        a pointer to dynamically allocated storage, depending on success
        The "count" field is also set before return (see OUTPUTS)

        "inst_list" is a pointer that is to be set to point to an "instance
        list" carrying the instance information for this class if there is
        any.  If there is none, this pointer is set to NULL.


OUTPUTS:

    On a successful return ( CI_ATTRIBUTE ), the vbe entries . . .

    * "class_oid->value" is set to point to heap storage containing
      the arcs of the OID that form the object class object identifier.
      "class_oid->count" is set to indicate the number of arcs in the oid.
      The caller is responsible for reclaiming the storage allocated to
      the "value" list.

    * "inst_list" is set to point to a linked list of "snmp_instance_t" structures
      containing the instance information for this object class.  If there
      is no instance information for the class, this cell is set to NULL.
      The caller is responsible for reclaiming the storage allocated to
      this list AND to the object-id arc-list ("value") contained in each
      snmp_instance_t structure on the list.

    CI_CLASS may be returned, but it's going to be rejected as an error.

    On an unsuccessful return, ( CI_NOSUCH or CI_PROC_FAILURE ) (no object
    class found or processing error):

    * "class_oid->value" is set NULL and
      "class_oid->count" is set to 0.

    * "inst_list" is set NULL.


BIRD'S EYE VIEW:
    Context:
        The caller is sending thread code in "casend_process_GET()" or
        "casend_process_SET()" functions that perform the specific
        translations from SNMP to Common Agent format for GET & SET requests.

    Purpose:
        This function performs the MIR inquiries necessary to get the
        information needed to parse SNMP Object ID portions that correspond
        to "instance information" SPECIFIC to GET and SET.


ACTION SYNOPSIS OR PSEUDOCODE:

    <show no class to caller>
    <show no instance to caller>

    <endeavor a mir_oid_to_mandle GET_EXACT lookup for the SNMP oid>

    switch (lookup return code)

        case MS0_DB_LOAD_FAILED:
            <CRASH "Mxxx - MIR Database load failed">
            break;

        case MS0_FIND_NONE:
             <show return status CI_NOSUCH>
             break;

        case MS0_FIND_EXACT:
             <show return status CI_NOSUCH>
             if (release of mandle failed)
                 <CRASH "Mxxx - MIR storage release error>
             break;

        case MS0_FIND_SHORT:
             <show return status CI_NOSUCH>
             if (release of mandle failed)
                 <CRASH "Mxxx - MIR storage release error>
             break;

        case MS0_FIND_LONG:
             <perform instance-info-extraction processing>
             break;

        case MS0_INVALID_MANDLE:
             (* A passed mandle address or pointer was invalid. *)
             <CRASH "Mxxx - MIR invalid mandle">

        case MS0_INVALID_OID:
             (* A passed address of an object id was invalid (null) *)
             <CRASH "Mxxx - MIR oid pointer">

        default:
             <CRASH "Mxxx - Unrecognized MIR subsystem return code">

    <record return status in vbe>
    <return>                                                                  


OTHER THINGS TO KNOW:

    This function has to do a lot more work that wasn't necessary under
    the earlier PE code because the MIR for the earlier PE stored
    only "things" that were "classes".  In the real MIR, attributes are
    mixed in with classes along with other MIR objects, and it requires
    maneuvering to discover what exactly a 'random' SNMP OID specifies in
    the MIR.  This 'sorting out' is done here and in the instance-derivation
    code in mir_derive_CI() (and its children).

    V1.4 We fetch the SMI in which the OID resides so that when Class/Instance
    derivation is done, the OIDs fetched there are forced to be fetched from
    the SMI of the original OID (whose lookup is done here).
*/

{
char            buf[LINEBUFSIZE];  /* Trace Message build buffer           */
int             ss;                /* "Start String" index for Trace msgs  */
mandle          *mir_object=NULL;  /* The main mandle for the object       */
mandle_class    *mclass=NULL;      /* The mandle class used for request    */
mir_status      mir_st;            /* MIR return status                    */
mir_oid_smi     oid_smi;           /* SMI in which OID for the object sits */
mir_derive_CI_status
                ret_status;        /* Local copy of status to return       */


/* show no class to caller */
vbe->class_oid.count = 0;
vbe->class_oid.value = NULL;

/* show no instance to caller */
vbe->inst_list = NULL;

/* endeavor a mir_oid_to_mandle lookup for the SNMP oid */
mir_st = mir_oid_to_mandle(GET_EXACT,           /* Exact lookup desired     */
                           &(vbe->snmp_oid),    /* Lookup by this OID       */
                           &mir_object,         /* Return to this mandle    */
                           &mclass,             /* put mandle in this class */
                           &oid_smi);           /* Get the SMI for OID here */

IFLOGGING( L_TRACE ) {
    char    *oid_string=NULL;    /* Points to text version of SNMP OID */

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "Using (%s) in MIR GET_EXACT request\n",
            (oid_string = pe_oid_text(&(vbe->snmp_oid))));
    LOGIT();
    if (oid_string != NULL)
        free(oid_string);

    sprintf(&buf[ss], "    returns status %s\n", mir_error(mir_st));
    LOGIT();

    /* If a non-terminal was returned . . . */
    if (   mir_st == MS0_FIND_SHORT
        || mir_st == MS0_FIND_ROLL
        || mir_st == MS0_FIND_EXACT
        || mir_st == MS0_FIND_LONG ) {

        mir_status      mstatus; /* MIR inquiry return status            */
        mir_value       m_value; /* Where terminal value can be returned */
        mandle          *m_nonterm=NULL; /* Where NT mandle is returned  */
        mandle_class    *mclass=NULL;    /*-> Mandle Class for any NT    */
        BOOL            release=FALSE;   /* TRUE: Release string storage */
        char            *string_name;    /* Name of object (if any)      */

        mstatus =
            mir_search_rel_table(SEARCH_FROMTOP,      /* Search style    */
                                 mir_object,          /* Search this. .  */
                                 m_list[M_TEXT_NAME].m,/* . . for this   */
                                 &mclass,             /* Use this mclass */
                                 &m_nonterm,          /* Return nonterm  */
                                 &m_value             /* Return term     */
                                 );

        switch (mstatus) {              /* search return status */
            case MS0_FIND_NONE:
                string_name = "<No-Name-Found>";
                break;
            case MS0_FIND_NONTERM:
                string_name = "<No-Name-Found>";
                mir_free_mandle_class(&mclass); /* Blow off storage */
                break;
            case MS0_FIND_TERM:
                if (IS_MV_STRING(&m_value)) {
                    string_name = MV_STRING(&m_value);
                    release = TRUE;
                    }
                else {
                    string_name = "<No-Name-Found>";
                    }
                break;
            default:
                string_name = "<No-Name-Found>";
                break;
            }

        sprintf(&buf[ss],
                "    Non-Terminal Object MASA: %d  \n",
                mir_object->m.m.ex_add);
        LOGIT();

        sprintf(&buf[ss],
                "    with name '%s'\n",
                string_name);
        LOGIT();

        /* Make sure next search starts from the top also */
        mir_reset_search(mir_object);
        }
    }

switch (mir_st)  {              /* lookup return code */

    case MS0_DB_NOT_LOADED:
        CRASH(MSG(msg121, "M121 - MIR Database has not been loaded"));
        break;

    case MS0_FIND_NONE:
        /* No Object by the specified Object ID was found in the MIR.
        |  This also implies that there were no Objects in the MIR
        |  whose complete Object ID matched any shorter portion of
        |  the specified Object ID.  (In such a case, MS0_FIND_SHORT
        |  is returned).  No mandle is created or re-used and no
        |  mandle-class is created or used.
        |
        |  SNMP:  This means absolutely no SNMP class exists in MIR,
        |         and no instance info can be had either.
        */
        ret_status = CI_NOSUCH;
        break;


     case MS0_FIND_EXACT:
         /* An Object by the specified Object ID was found in the MIR.
         |  There may be Objects in the MIR whose complete Object ID
         |  matches a shorter portion of the specified Object ID and
         |  there may also be Objects in the MIR for which the specified
         |  Object ID is a shorter portion of their complete Object ID,
         |  however the only exactly matching MIR Object is returned.
         |  A mandle is created or re-used and mandle-class is created
         |  or used.
         |
         |  SNMP: We found something EXACTLY.  This implies that the
         |        SNMP OID has no instance info "arcs" tacked on the
         |        end of it (because the MIR only has objects that
         |        correspond to class and attributes).  On a GET or SET,
         |        there MUST be at least ".0" as an instance arc on the
         |        SNMP OID (for the simplest cases: "scalar" non-table
         |        entries), so this is a "no class - no instance
         |        situation" also.
         |
         |  NOTE: We gotta dump what we found
         */
         ret_status = CI_NOSUCH;

         /* if (release of mandle failed) */
         if (mir_free_mandle_class(&mclass) != MS_SUCCESS) {
              CRASH(MSG(msg122, "M122 - MIR storage release error"));
              }
          break;


     case MS0_FIND_SHORT:
         /* An Object was found in the MIR, but the "object_id" supplied
         |  was too short to fully specify the Object that was found.
         |  The MIR returns the first occurrence (as it scans the internal
         |  index of objects) that fully matches the supplied (short)
         |  "object_id".
         |
         |  SNMP: The SNMP OID was 'way to short to contain instance
         |        info, and wasn't even long enough to exactly specify
         |        an SNMP class (or we'd have gotten MS0_FIND_EXACT).
         |        This is OK for GET-NEXT, but not for GET or SET.
         |        This is a "no class - no instance" situation.
         |
         |  NOTE: We gotta dump what we found
         */
         ret_status = CI_NOSUCH;

         /* if (release of mandle failed) */
         if (mir_free_mandle_class(&mclass) != MS_SUCCESS) {
              CRASH(MSG(msg123, "M123 - MIR storage release error"));
              }
         break;


     case MS0_FIND_LONG:
         /* An Object was found in the MIR, but the "object_id" supplied
         | was too long with respect to the Object that was found.
         |   The Object in the MIR whose full Object Id matches as much
         |   as possible of the supplied "object_id" is returned.
         |
         |   SNMP: OK! This looks like the bonafide goods.  On a GET or
         |         SET, the SNMP OID should properly be longer than
         |         anything in the MIR.  The SNMP manager should be
         |         including the attribute arc as well as instance arcs,
         |         and the MIR *should* find the object that corresponds
         |         to the attribute.  This must be checked, tho, as
         |         what we take to be the attribute arc may be bogus, and
         |         the MIR may then find a class directly (here) instead of the
         |         attribute.  This possibility must be considered during
         |         instance info extraction. In this case we let the MOM
         |         discover that the attribute arc is bogus, and for
         |         GET/SET let it kick it back w/NOSUCH.  (This presumes
         |         that the instance arcs look OK to the PE and manage
         |         to be interpreted into AVL's w/o errors).
         |
         |   NOTE: The MIR returned a mandle (& mandleclass) which must
         |         ultimately be disposed of.
         */

         /*
         | Strictly speaking, we should do the same thing here that we do
         | in mir_class_inst_GETNXT(), to wit: replace vbe->snmp_oid with
         | the exact OID that was found by the MIR call above, so that it
         | matches exactly the "mir_object" we're passing to mir_derive_CI().
         | We don't bother with the overhead though, because mir_derive_CLASS()
         | only really depends on the OID matching the object for 'rolling'
         | situations, and for GET/SET we never roll.  So we break the rules,
         | document the breaking, and save the overhead.
         */

         /* perform no-roll instance-info-extraction processing */
         ret_status =
             mir_derive_CI(bp,                 /* Big Picture                */
                           &vbe->class_oid,    /* return Class OID here      */
                           &vbe->inst_list,    /* return instance list here  */
                           &(vbe->snmp_oid),   /* "current object" OID       */
                           oid_smi,            /* SMI of "current object" OID*/
                           mir_object,         /* Inbound object to work on  */
                           &mclass,            /* Mandle-class of object     */
                           NO_ROLL_AT_ALL,     /* Rolling not allowed        */
                           FALSE               /* MIR has not rolled         */
                           );

         /* The mandle class has been released on success or failure */
         break;


     case MS0_INVALID_MANDLE:
         /* A passed mandle address or pointer was invalid. */
         CRASH(MSG(msg124, "M124 - MIR invalid mandle"));

     case MS0_INVALID_OID:
         /* A passed address of an object id was invalid (null) */
         CRASH(MSG(msg125, "M125 - MIR oid pointer invalid"));

     default:
         CRASH(MSG(msg233, "M233 - Unrecognized Return Status from MIR Subsystem"));
     }

return(ret_status);
}

/* mir_class_inst_GETNXT - Find SNMP Obj Class & Instance Info for GET-NEXT */
/* mir_class_inst_GETNXT - Find SNMP Obj Class & Instance Info for GET-NEXT */
/* mir_class_inst_GETNXT - Find SNMP Obj Class & Instance Info for GET-NEXT */

mir_derive_CI_status
mir_class_inst_GETNXT (bp, vbe)

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
varbind_t       *vbe;           /*-> Varbind Entry Block for SNMP OID        */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "vbe" points to a varbind entry block.  Inside the vbe:

        "snmp_oid" is a pointer to an object-id structure containing the SNMP
        OID that the caller wants to have converted into class oid + instance
        information.  This field is input to this function, and it is always
        a copy of the OID of vbe's "in_entry" AVL.  As we roll, (either here
        or in "mir_derive_CLASS()"), it changes.

        "class_oid" is the object-id structure to be set to the object id of
        the class (discovered by this function).  The "value" field of this
        object_id structure is overwritten on return, either by a NULL or by
        a pointer to dynamically allocated storage, depending on success
        The "count" field is also set before return (see OUTPUTS)

        "inst_list" is a pointer that is to be set to point to an "instance
        list" carrying the instance information for this class if there is
        any.  If there is none, this pointer is set to NULL.


OUTPUTS:

  Returned out of this function we may have:

  CI_ATTRIBUTE    - SNMP OID was an attribute, with or without instance arcs
                    present.

  CI_CLASS        - SNMP OID contained a valid class, but there may
                    be extra arcs on the end which we don't know what to do
                    with yet but we do know that the 1st (of any such extra
                    arcs) is *not* a valid attribute arc

  CI_ROLLED_CLASS - SNMP OID provided a starting place from which we rolled
                    to what appears to be a valid class.  Any 'extra' arcs in
                    SNMP OID beyond those needed to figure out where
                    to-roll-to-the-valid-class-being-returned are to be
                    considered junk (ie, instance arcs are dumped).

  CI_ROLLED_ATTRIBUTE - vbe's "snmp_oid" provided a starting place from which
                    we rolled to what appears to be another new valid
                    attribute object.

  CI_NOSUCH       - SNMP OID couldn't be rolled to any valid MIR object
                    that could be recognized as a class, much less an
                    attribute.

  CI_PROC_FAILURE - Processing failure (bad MIR lookup or somesuch).

    On a successful return ("CI_ATTRIBUTE", "CI_CLASS" or "CI_ROLLED_CLASS")
    in the vbe:

    * "class_oid->value" is set to point to heap storage containing
      the arcs of the OID that form the object class object identifier.
      "class_oid->count" is set to indicate the number of arcs in the oid.
      The caller is responsible for reclaiming the storage allocated to
      the "value" list.

    * "instance" is set to point to a linked list of "snmp_instance_t" structures
      containing the instance information for this object class.  (Note: There
      is always instance information of some sort for all classes, but the
      information may specify a "Null" Identifying Attribute (one whose
      datatype is "ASN.1 NULL"), indicating (in the SNMP sense) that there is
      'no' identifying instance info (".0")).

      The caller is responsible for reclaiming the storage allocated to
      this list AND to the object-id arc-list ("value") contained in each
      snmp_instance_t structure on the list.

    * "last_CI_status" is set to indicate how things went.

    On an unsuccessful return, ( CI_NOSUCH or CI_PROC_FAILURE ) (no object
    class found or processing error):

    * "class_oid->value" is set NULL and
      "class_oid->count" is set to 0.


BIRD'S EYE VIEW:
    Context:
        The caller is sending thread code in "casend_process_GETNEXT()"
        function that performs the specific translation from SNMP to
        Common Agent format for GET-NEXT requests.

    Purpose:
        This function performs the MIR inquiries necessary to get the
        information needed to parse SNMP Object ID portions that correspond
        to "instance information" SPECIFIC to the FIRST GET-NEXT request
        to the Common Agent for a given varbind entry block.


ACTION SYNOPSIS OR PSEUDOCODE:

    <show no class to caller>
    <show no instance to caller>
    <show no MIR roll>

    <endeavor a mir_oid_to_mandle lookup GET_EXACT_ROLL for the SNMP oid>

    switch (lookup return code)

        case MS0_DB_LOAD_FAILED:
            <CRASH "Mxxx - MIR Database load failed">
            break;

        case MS0_FIND_NONE:
             <show return status CI_NOSUCH>
             break;

        case MS0_FIND_SHORT:
        case MS0_FIND_ROLL:
             <show "mir_has_rolled">
             (* FALL THRU *)
        case MS0_FIND_EXACT:
        case MS0_FIND_LONG:
             <free arc storage for OID for "current object">
             <perform mandle_to_oid processing to get exact OID>
             if (processing wasn't successful)
                 <CRASH "Mxxx - Mandle to OID conversion failed">
             <perform instance-info-extraction processing>
             break;

        case MS0_INVALID_MANDLE:
             (* A passed mandle address or pointer was invalid. *)
             <CRASH "Mxxx - MIR invalid mandle">

        case MS0_INVALID_OID:
             (* A passed address of an object id was invalid (null) *)
             <CRASH "Mxxx - MIR oid pointer">

        default:
             <CRASH "Mxxx - Unrecognized MIR subsystem return code">

    <record return status in vbe>
    <return>                                                                  


OTHER THINGS TO KNOW:

    This function is used by its caller to construct the FIRST "invoke-action"
    call to the Common Agent for a GET-NEXT request.  If a roll is subsequently
    required for the same GET-NEXT varbind entry, then function
    mir_class_inst_ROLLNXT() is invoked instead of this one to build the
    second "invoke-action" request.

    This function has to do a lot more work that wasn't necessary under
    the earlier PE code because the MIR for the earlier PE stored
    only "things" that were "classes".  In the real MIR, attributes are
    mixed in with classes along with other MIR objects, and it requires
    maneuvering to discover what exactly a 'random' SNMP OID specifies in
    the MIR.  This maneuvering is done to a large extent here and in the
    instance-derivation code.

    This code (and the code in the functions it calls) is also complicated
    by the fact that the 'rolling' operation of moving to 'the next' OID
    is actually split into two different places in the processing for GET-NEXT.
    This is because of the way the MIR lookup logic (in MIR function
    mir_oid_to_mandle()) works.

    The actual MIR lookup performed by this function through a call to
    mir_oid_to_mandle() may do a 'roll' on its own as it does the lookup.
    This can happen in two cases, when either MS0_FIND_ROLL or MS0_FIND_SHORT
    is returned.  (See the comments in the code below for more discussion).
    This is the first place in GET-NEXT processing where a roll can occur.

    The second place where a roll can occur is in mir_derive_CI() (called by
    this function).  It examines the MIR object (found by the MIR lookup) to
    see if it is a valid attribute or a valid class object, and it may decide
    to "roll" depending on what it finds.

    Note:  In all cases the FIRST roll occurs "away from" the original snmp
           oid, (OID from "in_entry" AVL) SNMP OID.  On second and subsequent
           rolls (usually performed by mir_class_inst_ROLLNXT()), the roll
           always starts with the "current object": the last object we used
           (class or attribute).

    The importance of a roll occurring centers around the fact that when it
    happens (wherever it does), whatever attribute & instance arcs there may
    be in the original SNMP OID become invalid!  So we must communicate to the
    levels above this one the fact that a roll was performed so that no attempt
    is made to extract attribute or instance arcs from the original inbound
    SNMP OID.

    The two places in this module where rolling occurs (here in this function
    as a consequence of the MIR mir_oid_to_mandle() call) and in mir_derive_CI
    must be linked together logically so that if either code has rolled, that
    fact is returned in the final status code (as CI_ROLLED_CLASS).  The code
    in this function can detect when a rolling-action has occurred (based on 
    the return code from the MIR lookup mir_oid_to_mandle() call), but it
    doesn't really 'know' what it has rolled 'over' until mir_derive_CI() does
    its work.  Consequently the occurrence of a MIR return code indicating
    an implicit 'roll' is signalled through to mir_derive_CI() in a boolean
    argument to it: "mir_has_rolled", and mir_derive_CI() takes care to return
    the proper return code (if not CI_NOSUCH, then CI_ROLLED_CLASS).

    V1.4 - We fetch the SMI of the OID we use for the initial lookup in this
    function and then pass it downward so that other mandle-to-oid conversions
    are performed in the same SMI as the original OID.
*/

{
char            buf[LINEBUFSIZE];  /* Trace Message build buffer           */
int             ss;                /* "Start String" index for Trace msgs  */
char            msg[LINEBUFSIZE];  /* Error Message build buffer           */
mandle          *mir_object=NULL;  /* The main mandle for the object       */
mandle_class    *mclass=NULL;      /* The mandle class used for request    */
BOOL            mir_has_rolled;    /* TRUE: MIR lookup performed a roll    */
mir_status      mir_st;            /* MIR return status                    */
mir_oid_smi     oid_smi;           /* SMI indicator for mir_mandle_to_oid()*/
mir_derive_CI_status
                ret_status;        /* Local copy of status to return       */


/* show no class to caller */
vbe->class_oid.count = 0;
vbe->class_oid.value = NULL;

/* show no instance to caller */
vbe->inst_list = NULL;

/* show no MIR roll as yet */
mir_has_rolled = FALSE;

/* endeavor a mir_oid_to_mandle lookup for the SNMP oid */
mir_st = mir_oid_to_mandle(GET_EXACT_ROLL,     /* IN  */
                           &(vbe->snmp_oid),   /* IN  */
                           &mir_object,        /* OUT */
                           &mclass,            /* IN  */
                           &oid_smi);          /* OUT */

IFLOGGING( L_TRACE ) {
    char    *oid_string=NULL;    /* Points to text version of SNMP OID */

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "Using (%s) in MIR GET_EXACT_ROLL request\n",
            (oid_string = pe_oid_text(&(vbe->snmp_oid))));
    LOGIT();
    if (oid_string != NULL)
        free(oid_string);

    sprintf(&buf[ss], "    returns status %s\n", mir_error(mir_st));
    LOGIT();

    /* If a non-terminal was returned . . . */
    if (   mir_st == MS0_FIND_SHORT
        || mir_st == MS0_FIND_ROLL
        || mir_st == MS0_FIND_EXACT
        || mir_st == MS0_FIND_LONG ) {

        mir_status      mstatus; /* MIR inquiry return status            */
        mir_value       m_value; /* Where terminal value can be returned */
        mandle          *m_nonterm=NULL; /* Where NT mandle is returned  */
        mandle_class    *mclass=NULL;    /*-> Mandle Class for any NT    */
        BOOL            release=FALSE;   /* TRUE: Release string storage */
        char            *string_name;    /* Name of object (if any)      */

        mstatus =
            mir_search_rel_table(SEARCH_FROMTOP,      /* Search style    */
                                 mir_object,          /* Search this. .  */
                                 m_list[M_TEXT_NAME].m,/* . . for this   */
                                 &mclass,             /* Use this mclass */
                                 &m_nonterm,          /* Return nonterm  */
                                 &m_value             /* Return term     */
                                 );

        switch (mstatus) {              /* search return status */
            case MS0_FIND_NONE:
                string_name = "<No-Name-Found>";
                break;
            case MS0_FIND_NONTERM:
                string_name = "<No-Name-Found>";
                mir_free_mandle_class(&mclass); /* Blow off storage */
                break;
            case MS0_FIND_TERM:
                if (IS_MV_STRING(&m_value)) {
                    string_name = MV_STRING(&m_value);
                    release = TRUE;
                    }
                else {
                    string_name = "<No-Name-Found>";
                    }
                break;
            default:
                string_name = "<No-Name-Found>";
                break;
            }

        sprintf(&buf[ss],
                "    Non-Terminal Object MASA: %d  with name '%s'\n",
                mir_object->m.m.ex_add,
                string_name);
        LOGIT();

        /* Make sure next search starts from the top also */
        mir_reset_search(mir_object);
        }
    }

switch (mir_st)  {              /* lookup return code */

    case MS0_DB_NOT_LOADED:
        CRASH(MSG(msg157, "M157 - MIR Database has not been loaded"));
        break;

    case MS0_FIND_NONE:
        /* No Object by the specified Object ID was found in the MIR.
        |  With a GET_EXACT_ROLL search style, this implies that we rolled
        |  up against the end of the MIR.  This is really "The End".
        |
        |  SNMP:  This means absolutely no SNMP class exists in MIR,
        |         and no instance info can be had either.
        */
        ret_status = CI_NOSUCH;
        break;


    case MS0_FIND_SHORT:
        /* An Object was found in the MIR, but the "object_id" supplied
        |  was too short to fully specify the Object that was found.
        |  The MIR returns the first occurrence (as it scans the internal
        |  index of objects) that fully matches the supplied (short)
        |  "object_id".
        |
        |  SNMP: The SNMP OID was 'way to short to contain instance
        |        info, and wasn't even long enough to exactly specify
        |        an SNMP class (or we'd have gotten MS0_FIND_EXACT).
        |        In essence, the MIR lookup logic that supports returning
        |        this "SHORT" match has essentially 'rolled' (from an
        |        SNMP GET-NEXT viewpoint) to the next object in the MIR,
        |        which should be a class.  When we go for the instance
        |        info we'll find out for sure, it could be Not-a-Class,
        |        but it shouldn't be an attribute, but we can handle it if
        |        by some magic it turns out to be.
        |
        |  SHOW 'ROLL' PERFORMED BY
        |  FALLING THRU
        */


    case MS0_FIND_ROLL:
        /* The normal GET_EXACT processing failed to find any MIR object whose
        |  OID shared any portion of the supplied OID.  Since this was a
        |  GET_EXACT_ROLL call, the MIR lookup logic goes on to roll internally
        |  to the next MIR Object that would immediately follow a MIR object
        |  that had the supplied oid (if one were present in the MIR).
        |
        |  SNMP: The supplied SNMP OID must come before any object registered
        |        in the MIR and share no arc values with any object in the
        |        MIR.  (If it had come after all the MIR objects, we'd have
        |        gotten "MS0_FIND_NONE").  This is another instance where
        |        the MIR lookup logic has 'rolled' (from an SNMP GET-NEXT
        |        viewpoint) inside mir_oid_to_mandle() on our behalf.
        |
        |  SHOW 'ROLL' PERFORMED 
        */
        mir_has_rolled = TRUE;

        /* FALL THRU */


    case MS0_FIND_EXACT:
        /* An Object by the specified Object ID was found in the MIR.
        |  There may be Objects in the MIR whose complete Object ID
        |  matches a shorter portion of the specified Object ID and
        |  there may also be Objects in the MIR for which the specified
        |  Object ID is a shorter portion of their complete Object ID,
        |  however the only exactly matching MIR Object is returned.
        |  A mandle is created or re-used and mandle-class is created
        |  or used.
        |
        |  SNMP: We found something EXACTLY.  This implies that the
        |        SNMP OID has no instance info "arcs" tacked on the
        |        end of it (because the MIR only has objects that
        |        correspond to class and attributes).  On a GET-NEXT
        |        this is fine.  The possibility exists however that
        |        we've been pointed at something that is neither a class
        |        nor an attribute.  When we go for the instance info, we'll
        |        discover exactly whether it is a class, attribute or neither.
        |
        |        -> We're covering the possibility here that almost *anything*
        |        can be placed in the MIR and given an OID.  With V1.0 of
        |        SNMP PE & MIR, the only things currently known to be in the
        |        MIR w/OID that are not a class nor attribute are MIR
        |        relationship objects (used by the MIR compiler) and "The
        |        World" (under which all compiled things are inserted).
        |
        | FALL THRU === NO ROLL DONE
        */

    /*
    | We go to the MIR and extract class info in all circumstances because
    | the receiving thread is going to need it, even if the sending thread
    | doesn't need it to convert instance arcs (also, we have to 'fake-up'
    | the higher containing class 'distinguished names').
    */

    case MS0_FIND_LONG:
        /*  An Object was found in the MIR, but the "object_id" supplied
        |   was too long with respect to the Object that was found.
        |   The Object in the MIR whose full Object Id matches as much
        |   as possible of the supplied "object_id" is returned.
        |
        |   SNMP: This is probably what happens most frequently.  The SNMP
        |         manager is probably including the attribute arc as well as
        |         instance arcs, and the MIR *should* find the object that
        |         corresponds to the attribute.  This must be checked, tho, as
        |         what we take to be the attribute arc may be bogus, and
        |         the MIR may then find a class directly (here w/"_LONG")
        |         instead of the attribute.  No implicit rolling occurs on a
        |         _LONG return from the MIR lookup ("mir_has_rolled" is FALSE
        |         by default).
        |
        |   NOTE: The MIR returned a mandle (& mandleclass) which must
        |         ultimately be disposed of (not only for _LONG, but _SHORT
        |         and _ROLL too).
        */

        /*
        | The rule is that "snmp_oid" in the vbe always reflects the OID
        | of the "current object".  As a conesquence of the mir_oid_to_mandle()
        | call above, for the cases when a roll occurs, this makes "snmp_oid"
        | obsolete, and mir_derive_CI() depends on "snmp_oid" matching the
        | mir_object it is given.  Consequently we update "snmp_oid" right
        | here uncoditionally.  It's serious bogusness if this update fails.
        | Now, the OID we get may not be a true "SNMP OID", but that's OK,
        | cause MOLD will ultimately inform us as to whether it is supported
        | by a MOM someplace, (and whatever the object is will undergo the
        | scrutiny of a "mir_derive_CI()" call. . .if it passes that we can
        | go on to have MOLD and CA examine it).  Note that even tho this is
        | an "SNMP" PE, we have to be able to handle an OID in another SMI.
        | We already know which SMI: the one returned on the initial call to
        | mir_oid_to_mandle() above.
        */
        /* free arc storage for OID for "current object" */
        free(vbe->snmp_oid.value);

        /* perform mandle_to_oid processing to get exact OID */
        /* (Insist on the same SMI as we got on the mir_oid_to_mandle() call)*/
        mir_st = mir_mandle_to_oid(mir_object, &(vbe->snmp_oid), &oid_smi);

        /* if (processing wasn't successful) */
        if (mir_st != MS_SUCCESS) {
            sprintf(msg,
                    MSG(msg239, "M239 - mandle-to-oid conversion failure %s %d"),
                    mir_error(mir_st),
                    mir_object->m.m.ex_add
                    );
            CRASH( msg );
            }

        IFLOGGING( L_TRACE ) {
            /* Points to text version of new SNMP OID */
            char    *oid_string=NULL;

            sprintf(&buf[ss],
                    "%s for new object from GET_EXACT_ROLL: (%s)\n",
                    oid_smi_names[oid_smi],
                    (oid_string = pe_oid_text(&(vbe->snmp_oid)))); LOGIT();
            if (oid_string != NULL)
                free(oid_string);
            }

        /* perform instance-info extraction with rolling enabled */
        ret_status =
            mir_derive_CI(bp,                 /* The Big Picture            */
                          &(vbe->class_oid),  /* return Class OID here      */
                          &(vbe->inst_list),  /* return instance list here  */
                          &(vbe->snmp_oid),   /* "current object" OID       */
                          oid_smi,            /* SMI of "current obj" OID   */
                          mir_object,         /* work on this MIR obj.      */
                          &mclass,            /* ..in this mandleclass      */
                          ROLL_ALLOWED,       /* ROLL as needed             */
                          mir_has_rolled      /* if TRUE: 1 roll done       */
                          );                  /*          already           */

        /* The mandle class has been released on success or failure */
        break;


    case MS0_INVALID_MANDLE:
        /* A passed mandle address or pointer was invalid. */
        CRASH(MSG(msg158, "M158 - MIR invalid mandle"));

    case MS0_INVALID_OID:
        /* A passed address of an object id was invalid (null) */
        CRASH(MSG(msg159, "M159 - MIR oid pointer invalid"));

     default:
         CRASH(MSG(msg234, "M234 - Unrecognized Return Status from MIR Subsystem"));
    }


return(ret_status);
}

/* mir_class_inst_ROLLNXT - Roll to get next SNMP Obj Class & Instance Info */
/* mir_class_inst_ROLLNXT - Roll to get next SNMP Obj Class & Instance Info */
/* mir_class_inst_ROLLNXT - Roll to get next SNMP Obj Class & Instance Info */

mir_derive_CI_status
mir_class_inst_ROLLNXT (bp, sstatus, vbe)

big_picture_t   *bp;        /*-> Big Picture Structure for SNMP PE           */
e_status_t      sstatus;    /* "signalled" status from msi_invoke_action req */
varbind_t       *vbe;       /*-> Varbind Entry Block for SNMP OID            */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "sstatus" - "signalled" status indicating what kind of roll we're being
    asked to do: to next attribute ("rollattr") or to next class ("rollclass").

    "vbe" points to a varbind entry block.  Inside the vbe:

        "snmp_oid" is a pointer to an object-id structure containing the
        OID of the "current object" (initially the Original SNMP OID received
        by SNMP PE).

        "class_oid" is an object-id structure already set to the object
        id of a class (of "snmp_oid" on input).  We throw this value away
        and after rolling to 'the next' object after "snmp_oid", this
        object-id structure is set to the OID of the class of the next
        object rolled-to.

        The "value" field of this object_id structure is overwritten on return,
        either by a NULL or by a pointer to dynamically allocated storage,
        depending on success. The "count" field is also set before return
        (see OUTPUTS).

        "inst_list" is a pointer that points to an "instance list" carrying
        the instance information for the "class_oid" class.  Once the roll
        occurs successfully , this list is discarded and replaced with a new
        one (if there is any).  If there is none, this pointer is set to NULL.

OUTPUTS:

  Returned out of this function we may have:

  CI_ROLLED_CLASS - vbe's "snmp_oid" provided a starting place from which
                    we rolled to what appears to be another new valid class
                    object.

  CI_ROLLED_ATTRIBUTE - vbe's "snmp_oid" provided a starting place from which
                    we rolled to what appears to be another new valid
                    attribute object.

  CI_NOSUCH       - Couldn't roll to any new valid MIR object that could be
                    recognized as a class or attribute.  This is basically
                    "End-of-MIR".

  CI_PROC_FAILURE - An error occured during processing, a message is logged.

    On a successful return (CI_ROLLED_CLASS):

    * vbe's "snmp_oid->value" is set to point to heap storage containing
      the arcs of the OID for the new 'next' object (either attribute or
      class).  Caller is responsible for reclaiming the storage.  Any old
      storage that was there is released before the new arcs are put in place.

    * vbe's "class_oid->value" is set to point to heap storage containing
      the arcs of the OID that form the object class object identifier.
      vbe's "class_oid->count" is set to indicate the number of arcs in the
      oid.  The caller is responsible for reclaiming the storage allocated to
      the "arc value" list.  Any old storage that was there is released before
      the new arcs are put in place.

    * "instance" is set to point to a linked list of "snmp_instance_t" structures
      containing the instance information for this object class.

      The caller is responsible for reclaiming the storage allocated to
      this list AND to the object-id arc-list ("value") contained in each
      snmp_instance_t structure on the list.

    * "last_CI_status" is set to indicate how things went.

    On an unsuccessful return, (CI_NOSUCH & CI_PROC_FAILURE) (no object class
    found or processing error):

    * "class_oid->value" is set NULL (any old storage is released) and
      "class_oid->count" is set to 0.

    * Any existing "instance" list is released and NULL placed in
      vbe->inst_list.


BIRD'S EYE VIEW:
    Context:
        The caller is sending thread code in "casend_process_GETNEXT()"
        function that performs the specific translation from SNMP to
        Common Agent format for GET-NEXT requests.

    Purpose:
        This function performs the MIR inquiries necessary to get the
        information needed (class OID + instance info) to make a
        SECOND GET-NEXT request to the Common Agent for a given varbind
        entry block.


ACTION SYNOPSIS OR PSEUDOCODE:

    <release any existing instance list in the vbe>
    <release current "class OID" storage>
    <endeavor a GET_EXACT mir_oid_to_mandle lookup for the SNMP oid for
     the 'current object'>

    switch (lookup return code)

        case MS0_DB_LOAD_FAILED:
            <CRASH "Mxxx - MIR Database load failed">

        case MS0_FIND_NONE:
        case MS0_FIND_SHORT:
        case MS0_FIND_ROLL:
        case MS0_FIND_LONG:
             <SYSLOG "Mxxx - Anomalous MIR lookup "%s">
             <show return status CI_PROC_FAILURE>
             break;

        case MS0_FIND_EXACT:
             <perform instance-info-extraction processing>
             break;

        case MS0_INVALID_MANDLE:
             (* A passed mandle address or pointer was invalid. *)
             <CRASH "Mxxx - MIR invalid mandle">

        case MS0_INVALID_OID:
             (* A passed address of an object id was invalid (null) *)
             <CRASH "Mxxx - MIR oid pointer">

        default:
             <CRASH "Mxxx - Unrecognized MIR subsystem return code">

    <return>                                                                  


OTHER THINGS TO KNOW:

    This function is used by its caller to construct the SECOND or subsequent
    "invoke-action" call to the Common Agent for a GET-NEXT request.  The
    first is serviced by mir_class_inst_GETNXT().

    We expect the MIR lookup in this function to succeed because theoretically
    it has already been done once before.  Consequently it is a bit bizarre
    if it fails to exactly find the "current object".  Also note that with
    V1.4 of this module we are fetching the SMI of the OID an passing it
    downward for use by mir_derive_CI().

*/

{
char            buf[LINEBUFSIZE];  /* Trace Message build buffer           */
int             ss;                /* "Start String" index for Trace msgs  */
char            msg[LINEBUFSIZE];  /* Error Message build buffer           */
mandle          *mir_object=NULL;  /* The main mandle for the object       */
mandle_class    *mclass=NULL;      /* The mandle class used for request    */
mir_status      mir_st;            /* MIR return status                    */
mir_oid_smi     oid_smi;           /* Recvs SMI of OID we're working on    */
mir_derive_CI_status
                ret_status;        /* Local copy of status to return       */


/* release any existing instance list in the vbe */
dump_instance_list(&vbe->inst_list);

/* release current "class OID" storage */
free(vbe->class_oid.value);
vbe->class_oid.value = NULL;

IFLOGGING( L_TRACE ) {
    char    *oid_string=NULL;    /* Points to text version of SNMP OID */

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "Begin ROLLNXT with (%s) in MIR GET_EXACT request\n",
            (oid_string = pe_oid_text(&(vbe->snmp_oid)))); LOGIT();
    if (oid_string != NULL)
        free(oid_string);
    }

/* endeavor a mir_oid_to_mandle lookup for the "current object" */
mir_st = mir_oid_to_mandle(GET_EXACT,         /* IN  */
                           &(vbe->snmp_oid),  /* IN  */
                           &mir_object,       /* OUT */
                           &mclass,           /* IN  */
                           &oid_smi);         /* OUT */

switch (mir_st)  {              /* lookup return code */

    case MS0_DB_NOT_LOADED:
        CRASH(MSG(msg175, "M175 - MIR Database has not been loaded"));

    case MS0_FIND_NONE:
    case MS0_FIND_SHORT:
    case MS0_FIND_ROLL:
    case MS0_FIND_LONG:
        /*
        | In each of these cases, we failed to find a class or attribute
        | EXACTLY that had previously been successfully looked up (at least,
        | while we may not have looked it up  *directly*, at least we
        | converted a mandle (for what we took to be a MIR object representing
        | a object) into an OID).  Consequently, we should be able to EXACTLY
        | GET the "current object".
        |
        | These codes above indicate that for some reason, we failed.
        */
        {
            char    xmsg[LINEBUFSIZE];  /* Error Message build buffer        */
            sprintf(xmsg, MSG(msg178, "M178 - Anomalous return (%s) from MIR for (%%s)"),
                    mir_error(mir_st));
            BLD_OID_ERRMSG(xmsg, &(vbe->snmp_oid));
            SYSLOG(LOG_ERR, msg);

            ret_status = CI_PROC_FAILURE;
        }
        break;


    case MS0_FIND_EXACT:
        /* An Object by the specified Object ID was found in the MIR.
        |  There may be Objects in the MIR whose complete Object ID
        |  matches a shorter portion of the specified Object ID and
        |  there may also be Objects in the MIR for which the specified
        |  Object ID is a shorter portion of their complete Object ID,
        |  however the only exactly matching MIR Object is returned.
        |  A mandle is created or re-used and mandle-class is created
        |  or used.
        |
        |  SNMP: We've successfully re-looked-up a class or attribute object.
        |        Now we'll ask mir_derive_CI() to endeavor to 'roll away FROM'
        |        this object  to find the next one.
        */

        IFLOGGING( L_TRACE ) {

            mir_status      mstatus; /* MIR inquiry return status            */
            mir_value       m_value; /* Where terminal value can be returned */
            mandle          *m_nonterm=NULL; /* Where NT mandle is returned  */
            mandle_class    *mclass=NULL;    /*-> Mandle Class for any NT    */
            BOOL            release=FALSE;   /* TRUE: Release string storage */
            char            *string_name;    /* Name of object (if any)      */

            mstatus =
                mir_search_rel_table(SEARCH_FROMTOP,      /* Search style    */
                                     mir_object,          /* Search this. .  */
                                     m_list[M_TEXT_NAME].m,/* . . for this   */
                                     &mclass,             /* Use this mclass */
                                     &m_nonterm,          /* Return nonterm  */
                                     &m_value             /* Return term     */
                                     );

            switch (mstatus) {              /* search return status */
                case MS0_FIND_NONE:
                    string_name = "<No-Name-Found>";
                    break;
                case MS0_FIND_NONTERM:
                    string_name = "<No-Name-Found>";
                    mir_free_mandle_class(&mclass); /* Blow off storage */
                    break;
                case MS0_FIND_TERM:
                    if (IS_MV_STRING(&m_value)) {
                        string_name = MV_STRING(&m_value);
                        release = TRUE;
                        }
                    else {
                        string_name = "<No-Name-Found>";
                        }
                    break;
                default:
                    string_name = "<No-Name-Found>";
                    break;
                }

            sprintf(&buf[ss],
             "MIR GET_EXACT request returns correct status MS0_FIND_EXACT\n");
            LOGIT();

            sprintf(&buf[ss],
                    "    for Non-Terminal Object MASA: %d  with name '%s'\n",
                    mir_object->m.m.ex_add,
                    string_name);
            LOGIT();

            /* Make sure next search starts from the top also */
            mir_reset_search(mir_object);
            }
            

        /*
        | Perform ROLL_FOR_ATTRIB/ROLL_FOR_CLASS, then do class-instance
        | information extraction.
        */
        ret_status =
            mir_derive_CI(bp,                   /* The Big Picture           */
                          &(vbe->class_oid),    /* return Class OID here     */
                          &(vbe->inst_list),    /* return instance here      */
                          &(vbe->snmp_oid),     /* "current object" OID      */
                          oid_smi,              /* SMI of "curr. Obj" OID    */
                          mir_object,           /* work on this MIR object   */
                          &mclass,              /* ..in this mandleclass     */
                          ((sstatus == rollattrib) ? /* ROLL away FROM to ...*/
                                  (ROLL_FOR_ATTRIB): /* next ATTRIBUTE       */
                                  (ROLL_FOR_CLASS)), /* next CLASS           */
                          TRUE                  /* (Bogus: we roll anyway)   */
                          );

        /* The mandle class has been released on success or failure */
        break;


    case MS0_INVALID_MANDLE:
        /* A passed mandle address or pointer was invalid. */
        CRASH(MSG(msg176, "M176 - MIR invalid mandle"));

    case MS0_INVALID_OID:
        /* A passed address of an object id was invalid (null) */
        CRASH(MSG(msg177, "M177 - MIR oid pointer invalid"));

    default:
        CRASH(MSG(msg235, "M235 - Unrecognized Return Status from MIR Subsystem"));
    }

return(ret_status);
}

/* mir_derive_CI - Derive Class and Instance Information from MIR Object */
/* mir_derive_CI - Derive Class and Instance Information from MIR Object */
/* mir_derive_CI - Derive Class and Instance Information from MIR Object */

static mir_derive_CI_status
mir_derive_CI (bp,
               class_oid,           /* OUT    */
               instance,            /* OUT    */
               mir_object_oid,      /* IN/OUT */
               oid_smi,             /* IN     */
               mir_object,          /* IN     */
               mclass,              /* IN     */
               dstyle,              /* IN     */
               mir_has_rolled       /* IN     */
               )

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
object_id       *class_oid;     /* -> to where to store returned class oid   */
snmp_instance_t **instance;     /* ->> where to store the instance list      */
object_id       *mir_object_oid;/* SNMP OID that corresponds to 'mir_object' */
mir_oid_smi     oid_smi;        /* SMI of OID in mir_object_oid              */
mandle          *mir_object;    /* Mandle describing MIR Object to be worked */
mandle_class    **mclass;       /*-> Mandle Class containing "mir_object"    */
derive_style    dstyle;         /* Class-Derivation Style                    */
BOOL            mir_has_rolled; /* TRUE: One roll has already been done      */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "class_oid", "mir_object_oid", "oid_smi", "mir_object", "mclass", "dstyle"
    and "mir_has_rolled" are all "pass-thru" arguments to function
    mir_derive_CLASS() which is called by this function.  See the INPUTs
    section of that function for a description of these arguments.

    "instance" is the address of a pointer that is to be set to point
    to an "instance list" carrying the instance information for this class
    if there is any.  If there is none, this pointer is set to NULL.
    This argument is "passed thru" to function mir_derive_INSTANCE() that
    is called by this function.

OUTPUTS:

    On a successful return, the status is one of those returned by
    function "mir_derive_CLASS()".  See the OUTPUTS section of that function's
    documentation for a description of the successful return codes and
    their meanings.

    On a successful return, the "mir_object" mandle and all the mandles in
    it's class have been reclaimed.

      NOTE:  That means the mandle *PASSED IN* is GONE along with any
             other mandles that had been allocated in the same class!

    On an unsuccessful return, the status is:

    * CI_NOSUCH:
      The MIR object was neither a valid attribute nor a valid object class
      for SNMP.  If dstyle was ROLL_ALLOWED or ROLL_FOR_SURE  upon entry,
      this function "rolled forward" through the index until the end of the
      MIR was reached without finding a MIR object that was a valid object
      class.

    * CI_PROC_FAILURE
      A processing failure occurred, it's a generr situation.

    On an unsuccessful return:

    * class oid "count" is zero
    * class oid "value" is NULL
    * instance pointer points to a NULL (which is set there by this code)
    * the mandle and mandleclass storage have been reclaimed
      NOTE:  That means the mandle *PASSED IN* is GONE along with any
             other mandles that had been allocated in the same class!


BIRD'S EYE VIEW:
    Context:
        The caller is mir_class_inst_GETSET(), mir_class_inst_GETNXT() or
        mir_class_inst_ROLLNXT() in this module.

    Purpose:
        This function performs the MIR inquiries necessary to discover
        whether the MIR object is a "class" or an "attribute" and then
        return the Class OID and instance information.  If we are called
        "rollable", then in the event the passed OID doesn't specifiy
        a valid attribute or class, we'll "roll" through the MIR until
        we find a valid class or attribute.


ACTION SYNOPSIS OR PSEUDOCODE:

    <derive class object and OID, get derivation-status>
    if (status was CI_NOSUCH or CI_PROC_FAILURE)
        if (release of mandleclass failed)
            <CRASH "Mxxx - Mandle class release failed %d">
        <return status>

    (*
    At this point, we've found the MIR Object corresponding to the Object
    Class we're going to process, rolling (if allowed) as necessary to find it.
    Now we extract all the instance information from the MIR for this class
    and all the classes that contain it.
    *)

    <derive instance information for class and containing classes>

    if (mandleclass release failed)
        <CRASH - "Mandleclass release failed">

    if (instance derivation failed)
        <return CI_PROC_FAILURE>

    <return derivation status>


OTHER THINGS TO KNOW:

*/

{
char            buf[LINEBUFSIZE];   /* Trace Message build buffer            */
int             ss;                 /* "Start String" index for Trace msgs   */
mir_derive_CI_status    dstatus;    /* Status for "derivation" calls         */


IFLOGGING( L_TRACE ) {

    char    *oid_string=NULL;  /* Points to text version of Orig OID */

    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);

    sprintf(&buf[ss],
            "Deriving Class & Instance for 'current' MIR Object w/MASA %d\n",
            mir_object->m.m.ex_add);                            LOGIT();

    sprintf(&buf[ss], "    'Current' MIR Object's OID(%s)\n",
            (oid_string = pe_oid_text(mir_object_oid)));        LOGIT();
    free(oid_string);

    sprintf(&buf[ss],
            "    Specified Derive-Style: %s\n",
            derive_style_string[dstyle]);                       LOGIT();

    sprintf(&buf[ss],
            "    'MIR_has_rolled' input flag value: %s\n",
            ((mir_has_rolled == TRUE) ? ("TRUE") : ("FALSE"))); LOGIT();
    }

/* derive class object and OID, get derivation-status */
dstatus =
    mir_derive_CLASS(bp,            /* The Big Picture                       */
                     class_oid,     /* Return derived Class OID here         */
                     mir_object_oid,/* --> OID for starting & ending Object  */
                     oid_smi,       /* SMI for "mir_object_oid"              */
                     &mir_object,   /* Derive from this MIR object           */
                     mclass,        /* The MIR object is in this mandleclass */
                     dstyle,        /* Return derivation status here         */
                     mir_has_rolled /* Indicator of rolling done already     */
                     );             /* (in getting "mir_object")             */

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "Class derivation status = %s\n",
            CI_status_string[dstatus]);                     LOGIT();
    }

if (dstatus == CI_NOSUCH || dstatus == CI_PROC_FAILURE) {

    /* if (release of mandleclass failed) */
    if (mir_free_mandle_class(mclass) != MS_SUCCESS) {
        CRASH(MSG(msg133, "M133 - Mandle class release failed"));
        }

    return(dstatus);
    }

IFLOGGING( L_TRACE ) {

    char    *oid_string=NULL;  /* Points to text version of Orig OID */

    sprintf(&buf[ss],
            "    After Class derivation, 'current' MIR Object OID(%s)\n",
            (oid_string = pe_oid_text(mir_object_oid)));        LOGIT();
    if (oid_string != NULL) free(oid_string);

    sprintf(&buf[ss],
            "    CLASS MIR Object OID(%s)\n",
            (oid_string = pe_oid_text(class_oid)));             LOGIT();
    if (oid_string != NULL) free(oid_string);

    sprintf(&buf[ss],
            "    CLASS MIR Object MASA %d\n",
            mir_object->m.m.ex_add);                            LOGIT();
    }

/*
|   At this point, we've found the MIR Object corresponding to the Object
|   Class we're going to process, rolling (if allowed) as necessary to find it.
|   Now we extract all the instance information from the MIR for this class
|   and all the classes that contain it.
*/

/* derive instance information for class and containing classes */
mir_derive_INSTANCE(bp, instance, mir_object, mclass);

if (mir_free_mandle_class(mclass) != MS_SUCCESS) {
    CRASH(MSG(msg137, "M137 - Mandle class release failed"));
    }

/* if (instance derivation failed) */
if (instance == NULL) {
    return(CI_PROC_FAILURE);
    }

/* return derivation status */
return (dstatus);
}

/* mir_derive_CLASS - Derive Class from MIR Object supplied */
/* mir_derive_CLASS - Derive Class from MIR Object supplied */
/* mir_derive_CLASS - Derive Class from MIR Object supplied */

static mir_derive_CI_status
mir_derive_CLASS  (bp,
                   class_oid,           /* OUT    */
                   mir_object_oid,      /* IN/OUT */
                   oid_smi,             /* IN     */
                   mir_object,          /* IN/OUT */
                   mclass,              /* IN     */
                   dstyle,              /* IN     */
                   mir_has_rolled       /* IN     */
                   )

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
object_id       *class_oid;     /* -> to where to store returned class oid   */
object_id       *mir_object_oid;/* SNMP OID that corresponds to 'mir_object' */
mir_oid_smi     oid_smi;        /* SMI for "mir_object_oid"                  */
mandle          **mir_object;   /* Mandle describing MIR Object to be worked */
mandle_class    **mclass;       /*-> Mandle Class containing "mir_object"    */
derive_style    dstyle;         /* Class-Derivation Style                    */
BOOL            mir_has_rolled; /* TRUE: One roll has already been done      */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "class_oid" is a pointer to an-already-allocated-object-id structure that
    is to be set to describe the object id of the class derived by this
    function.  The "value" field of this object_id structure is overwritten
    on return, either by a NULL or by a pointer to dynamically allocated heap
    storage, depending on success (see OUTPUTS).  The "count" field is also
    set before return (on success or failure).

    "mir_object_oid" - OID of "mir_object" *on eentry*, on exit it is set to
    "the current object".  This OID is used inside here in situations where
    rolling happens, to roll to the next thing in the MIR.  If rolling is done,
    this input OID value is what we 'roll-from', while on output this is set to
    the new OID of the object rolled-to.  The rule is that whenever this
    function returns (with something other than CI_NOSUCH or CI_PROC_FAILURE),
    this OID must be set to "the current object" SNMP PE is dealing with.

    This includes the situation wherein 'mir_object' turns
    out to be something that is a class, then both "class_oid" and
    "mir_object_oid" will contain the same OID upon return, and "the current
    object" happens to be a class.

    NOTE: On return, the "current object" is NOT NECESSARILY "mir_object"!
          This is because on a successful return (not _NOSUCH nor _FAILURE)
          the "mir_object" is always set to a MIR object that is a "class"
          (so that MIR-based instance information can be extracted from it).
          The "current object" can be either that class OR an attribute within
          that class.

          (We need to keep track of the (OID of the) "current object" only
          for GET-NEXT situations so that we have an OID to 'roll away from'
          in the event a MOM says that there is no 'next' in that particular
          class, (signalled as a return code to us) and therefore we need to
          roll.

    Note also that upon return, any storage used to represent the arcs
    in "mir_object_oid" on input is freed (via free(), so this storage
    MUST be dynamic heap storage) before new storage is allocated to receive
    the arcs of the new returned oid.  The caller is responsible for ultimately
    freeing this storage.

    "oid_smi" is the SMI indicator for the SMI in which "mir_object_oid" was
    found.  We need this because the OID for whatever MIR object is found to
    be the class must have an OID (the one we want) in this particular SMI.

    "mir_object" is the address of the mandle pointer for the MIR object to be
    examined. (it may be a "class", an "attribute" or neither on entry).  On
    successful return, it is set to point to a MIR object that is a class
    (so that instance information can subsequently be extracted).

    "mclass" is the address of the mandleclass pointer for mandle "mir_object".

    "dstyle" is the derivation style indicating exactly what to do with
    "mir_object" in order to come up with the desired class we're trying
    to derive, one of:

        ROLL_FOR_ATTRIB - "mir_object" is known to be something, but we don't
            care.  To derive the (next) ATTRIBUTE, we roll through the MIR
            (using "mir_object_oid" as the starting point) looking for the
            NEXT MIR object that is a bonafide attribute inside a bonafide
            class.  This supports rolling on the SECOND and subsequent
            requests to the Common Agent for a single GET-NEXT SNMP varbind
            entry request.

        ROLL_FOR_CLASS - "mir_object" is known to be something, but we don't
            care.  To derive the (next) CLASS, we roll through the MIR
            (using "mir_object_oid" as the starting point) looking for the
            NEXT MIR object that is a bonafide class.    This code occurs when
            the MOLD signals the CA library that a class doesn't exist
            (MAN_C_NO_SUCH_CLASS) and we need to quickly roll over any
            attributes in this none-existent class to the next class.
            This supports rolling on the SECOND and subsequent requests to
            the Common Agent for a single GET-NEXT SNMP varbind entry request.

        ROLL_ALLOWED - "mir_object" is of unknown type (could be attribute,
            class or neither).  MIR inquiries are performed.

              If it is an attribute, and the MIR object that "contains" it is
              shown to be a valid class, then this is the extent of the
              derivation, and the containing MIR object's OID is returned as
              the class OID.  No rolling need be done, and none is.

              If it is a class, then the OID of it is returned as the class
              OID, no rolling is done.

              If the mir_object is not an attribute or its containing MIR
              object is not a valid class or it is not a class itself, then
              derivation consists of rolling thru the MIR as described above
              for ROLL_FOR_SURE looking for the next MIR Object that is a
              bonafide class or attribute.
              This supports rolling on the FIRST request to the Common Agent
              for a GET-NEXT SNMP varbind entry request.  

        NO_ROLL_AT_ALL - "mir_object" is expected to be an attribute.
            MIR inquiries are performed.  If it is an attribute and the MIR
            object that "contains" it is a class, then this is the extent of
            the derivations and the containing MIR object's OID is returned as
            the class OID.  If the mir_object is not an attribute or its
            containing MIR object is not a valid class, then the derivation
            fails, as no rolling is permitted.  This supports class extraction
            for GET and SET SNMP requests to the Common Agent.

    "mir_has_rolled" - indicates whether the MIR lookup done by the caller
    of this function resulted in an implicit roll being done.  If TRUE, that
    means this function must always return "CI_ROLLED_CLASS" or
    "CI_ROLLED_ATTRIBUTE" inlieu of "CI_CLASS" or "CI_ATTRIBUTE".


OUTPUTS:

    On a successful return, the status is one of:

    * CI_ATTRIBUTE:
      The MIR object "mir_object" (on input) turned out to be a valid MIR
      object for an attribute.

      "mir_object_oid" remains unchanged.

      "mir_object" is changed to point to the MIR object that is the "class"
                   of the original "mir_object".

      "class_oid" is changed to the OID of "mir_object".

                   
    * CI_CLASS:
      The MIR object "mir_object" (on input) turned out to be a valid MIR
      object for a class.

      "mir_object_oid" remains unchanged.

      "mir_object" points to the same object (the class) it did on input.

      "class_oid" is changed to the OID of "mir_object".


    * CI_ROLLED_CLASS:
      Either: 
        (1) The MIR object "mir_object" (on input) was a valid class,
            and "mir_has_rolled" was TRUE
             OR
        (2)     "mir_has_rolled" was FALSE  AND
            the MIR object "mir_object" was neither a valid attribute nor a
            valid object class for SNMP, so this function "rolled forward"
            through the MIR index until what turned out to be a valid class
            object was found.

      "mir_object_oid" remains unchanged (1) or is set to the OID of the
                       new MIR object 'rolled-to'.

      "mir_object" points to the same object (the class) it did on input (1)
                   or to the new MIR object 'rolled-to'(2).

      "class_oid" is changed to the OID of "mir_object".

      This code is returned only if dstyle was ROLL_FOR_SURE
      or ROLL_ALLOWED upon entry AND the function did indeed have to roll.
      

    * CI_ROLLED_ATTRIBUTE:
      Either: 
        (1) The MIR object "mir_object" (on input) was a valid attribute,
            and "mir_has_rolled" was TRUE
             OR
        (2)     "mir_has_rolled" was FALSE  AND
            the MIR object "mir_object" was neither a valid attribute nor a
            valid object class for SNMP, so this function "rolled forward"
            through the MIR index until what turned out to be a valid attribute
            object was found.

      "mir_object_oid" remains unchanged (1) or is set to the OID of the
                       new MIR object 'rolled-to'.

      "mir_object" points to the MIR Object for the class of:
                   (1) the original "mir_object" attribute supplied as input
                   or
                   (2) the new MIR object attribute that was 'rolled-to'.

      "class_oid" is changed to the OID of "mir_object".

      This code is returned only if dstyle was ROLL_FOR_SURE
      or ROLL_ALLOWED upon entry AND the function did indeed have to roll.

      
    - and no mandleclass storage is released.

    In general for a successful return you get:
      1) The OID of the "current object" (in mir_object_oid)
      2) The OID of the class of the "current object" (in "class_oid")
      3) The mandle for the MIR object representing the class of the
         "current object" (in "mir_object").
      4) An indication as to what the "current object" is and how you got
         it (CI_CLASS, CI_ATTRIBUTE, CI_ROLLED_CLASS or CI_ROLLED_ATTRIBUTE).

    On an unsuccessful return, the status is:

    * CI_NOSUCH:
      The MIR object was neither a valid attribute nor a valid object class
      for SNMP.  If ROLL_ALLOWED upon entry, this function "rolled forward"
      through the index until the end of the MIR was reached without finding
      a MIR object that was a valid object class or valid attribute.

    * CI_PROC_FAILURE
      This code is returned if there was a processing failure of some
      sort.  If this code is returned a message will have already been logged.

      - class oid "count" is zero
      - class oid "value" is NULL
      - the mandle and mandleclass storage have NOT been reclaimed

    NOTE: During processing, this function may create mandles within the
          passed mandleclass.  This function relies on the caller to
          release all mandles in the class, thereby assuring that any
          mandles created here will also be released eventually.


BIRD'S EYE VIEW:
    Context:
        The caller is mir_derive_CI in this module.

    Purpose:
        This function performs the MIR inquiries necessary to discover
        whether the MIR object is a "class" or an "attribute" and then
        returns the Class OID.  If we are called "rollable", then in the
        event the passed object isn't a valid attribute or class,
        we'll "roll" through the MIR until we find the next valid attribute
        or class object and return that.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (local mandles haven't been loaded)
        <load local mandles>

    <assume failure for class: count=0, value=NULL>
    <show no local return set yet>

    if (ROLL_FOR_ATTRIB or ROLL_FOR CLASS)
        <select NEED_A_ROLL>
    else
        <select NEED_ATTRIBUTE_CHECK>

    (* Loop to find a class or check to be sure it is a class as needed *)
    while (we're not ALL_FINISHED)

        switch (local dispatch)

            case NEED_ATTRIBUTE_CHECK:

                (*
                |  We may have been passed an SNMP "class" object or an
                |  "attribute" object.  Attributes have datatypes, classes
                |  don't.  We use this fact to determine what we've got.
                *)
                <perform a search on
                           * current mir_object,
                           * relationship "MIR_Structured_As"
                           * from the top>

                switch (return status)
                    case MS0_FIND_TERM:
                    case MS0_INVALID_MANDLE:
                        <SYSLOG "Mxxx - %s returned, MIR corrupt or coding
                                 error">
                        <return CI_PROC_FAILURE>

                    case MS0_FIND_NONTERM:
                        (*
                        |  Ok, datatype returned: it's an attribute, try to
                        |  "back-up" internally (within the MIR) to the
                        |  class, which should contain it.  Use the original
                        |  mir_object mandle to potentially receive the class
                        |  object.
                        *)

                        <perform a search on:
                            * current mir_object,
                            * relationship "MIR_Contained_By"
                            * from the top>

                        switch (return status)
                            case MS0_FIND_NONE:
                                <show local dispatch as "Need a Roll">
                                <break>

                            case MS0_FIND_NONTERM:
                                (*
                                | "mir_object" now points to SNMP "class"
                                | Object
                                *)
                                <show local return status as "CI_ATTRIBUTE">
                                <show local dispatch as "Need a Class Check">
                                break;

                            case MS0_FIND_TERM:
                            case MS0_INVALID_MANDLE:
                            default:
                                <SYSLOG "Mxxx - Missing "MIR_Contained_By"
                                  Rel, (%s)>
                                <return CI_PROC_FAILURE>
                        break;

                    case MS0_FIND_NONE:
                        (* Ok, no datatype: presume it's a class *)
                        <show local dispatch as "Need a Class Check">
                        <show local return as "CI_CLASS">
                        break;

                    default:
                        <CRASH "Mxxx - Unrecognized MIR subsystem return code">


            case NEED_CLASS_CHECK:
                <perform a search on:
                    * current mir_object,
                    * relationship "MIR_Indexed_By"
                    * from the top>

                switch (return code)
                    case MS0_FIND_NONE:
                        (*
                        | If the class-check failed, we've got to do the roll
                        *)
                        <set local dispatch to "NEED_A_ROLL">
                        <break>

                    case MS0_FIND_TERM:
                    case MS0_FIND_NONTERM:
                        if (local return is NOT SET YET)
                            <set local return to CI_CLASS>
                        <set local dispatch to "ALL_FINISHED">
                        <break>
                    default:
                        <CRASH "Mxxx - Coding Error, status=%s>
                <break>


            case NEED_A_ROLL:

                if (rolling is NOT permitted)
                    <return CI_NOSUCH>

                <show we've rolled>

                <perform oid_to_mandle w/GET_NEXT search using mir_object_oid>

                switch (oid_to_mandle return code)
                    case MS0_FIND_EXACT:
                        if (derive status was ROLL_FOR_ATTRIB or ROLL_ALLOWED)
                            <set local dispatch for "NEED_ATTRIBUTE_CHECK">
                        else (* derive status can't be NO_ROLL_AT_ALL here *)
                            <set local dispatch for "NEED_CLASS_CHECK">
                        <break>

                    default:
                        <SYSLOG "Mxxx - Execution Anomaly: MIR GET_NEXT for 
                                 (oid) returned %s>
                        (* Fall Thru *)

                    case MS0_FIND_NONE:
                        <return CI_NOSUCH>

                <free storage on mir_object_oid>
                <perform mandle_to_oid on current mir object>
                if (status is not success)
                    <CRASH "Mandle to OID conversion failure">

                <break>

    <perform a mir_mandle_to_oid() to obtain the "class" OID as the MIR 
     sees it now>

    if (return code is not MS_SUCCESS)
        <SYSLOG "Mxxx - Execution Anomaly: mandle_to_oid returned %s>
        <return CI_PROC_FAILURE>

    if (we've rolled)
        <set local rtn to CI_ROLLED_ATTRIBUTE or CI_ROLLED_CLASS accordingly>

    <return local return code>


OTHER THINGS TO KNOW:

    This is the other function where 'rolling' for GET-NEXT can occur.
    Here it is normally done by 'brute force':  Submit the supplied oid of
    "the current object" (in "mir_object_oid") to a GET_NEXT lookup request
    to get the next MIR object.

    This code consists of three main switch-statement sections within a loop
    that switches amongst the sections as needed.

    The first section of code (case NEED_ATTRIBUTE_CHECK) attempts to do two
    things:

        * Verify the "current object" as a valid attribute
        * Derive the MIR object that corresponds to the attribute's SNMP class

    For GET/SET situations and for FIRST-time GET-NEXT situations, this code
    is the first executed.  We're hoping that the "current object" turns out
    to specify a valid attribute within a valid class.  If we manage to extract
    what seems to be a valid class, under these circumstances, we go on to
    the second section.  If the extraction fails, we hope that the "current
    object" might be a valid class anyway, so we go on to submit the "current
    object" to the second section.

    The second section of code (case NEED_CLASS_CHECK) endeavors to verify
    the current 'thing' as a class MIR object.  If it succeeds, we're all done
    and we return a MIR object mandle for the class, the class OID and the
    OID for "the current object" which also happens to be "the class" we were
    called to derive.  If the class-check fails, this section assumes we should
    'roll' in an attempt to find a valid class or attribute.

    The third section of code (case NEED_A_ROLL) endeavors to roll thru the
    MIR (using "the current object" ---as specified by the current MIR object
    OID) as a starting point) to find the next valid class or attribute
    object.  The section services requirements for GET-NEXT *ONLY* (since we
    don't roll on GET or SET), so consequently if we arrive in this section
    when called to service GET/SET (ie we were called with NO_ROLL_AT_ALL),
    it is a CI_NOSUCH situation (we exit).

    If the third section is indeed allowed to roll, it'll roll, acquire a new
    "current object" that is again examined by the first section in hopes
    it describes an attribute, and the cycle begins anew.

    For SECOND-time GET-NEXT situations, the order of execution is different,
    as we're not interested in the "current object" *except* to 'roll away'
    from it!  Consequently the third section is executed first, in an attempt
    to 'go find' the next valid object.

    THE LONG AND THE SHORT:
        This function may look simple, but it services the requirements of
        GET/SET, GET-NEXT-first, GET-NEXT-second time 'calls', and the
        requirements are wildly different.  I'd advise changing this code
        with a great deal of care, because it took about seven iterations to
        get it to this point: . . . we hope it contains no bugs!

                               - - - - - - - -
    By the way. . .

    The second section always checks what it thinks is the MIR class object
    for its MIR_Indexed_By relationship to preclude returning as a class a MIR
    Object that is simply missing a "MIR_Structured_As" relationship.  Since
    we'll "roll" through many MIR objects, including the MIR objects that
    stand for relationships (and which *don't* have datatypes, but also
    *aren't* classes), we thereby do 'the right thing'.

    With V1.4, we want to "play" in the SMI of the original ("mir_object_oid")
    OID.  If we don't, we could wind up passing a MOM an attribute OID from
    one SMI and a class OID from another!
*/

{
char            buf[LINEBUFSIZE];  /* Trace Message build buffer            */
int             ss;                /* "Start String" index for Trace msgs   */
char            msg[LINEBUFSIZE];  /* Error Message build buffer            */
mir_status      mstatus;           /* MIR inquiry return status             */
mir_value       m_value;           /* Where terminal value can be returned  */
mandle          *m_nonterm=NULL;   /* Where non-terminal mandle is returnd  */
mir_derive_CI_status
                local_return;      /* Tentative local copy of return status */

/*
| Local Dispatching values. . .
| Serves to route processing locally
*/
enum {
    NEED_A_ROLL,                   /* Must Roll to Next object if allowed   */
    NEED_CLASS_CHECK,              /* Check object to be sure it is a class */
    NEED_ATTRIBUTE_CHECK,          /* Check object as attribute, extract    */
                                   /*   a possible class object.            */
    ALL_FINISHED                   /* Processing complete for class object  */
    }
                local_dispatch;

static
char    *local_disp_string[3] = {
    "NEED_A_ROLL",
    "NEED_CLASS_CHECK",
    "NEED_ATTRIBUTE_CHECK"
    };

/* if (local mandles haven't been loaded) */
#ifdef THREADS
pthread_once(&mir_init_once,               /* Good Work, Mir! */
             (pthread_initroutine_t) (call_load_local_mandles));
#else
if (local_class == NULL)
    call_load_local_mandles();
#endif


/* assume failure for class: count=0, value=NULL */
class_oid->count = 0;
class_oid->value = NULL;

/* show no local return set yet */
local_return = CI_NOT_SET_YET;

/* if (ROLL_FOR_ATTRIB or ROLL_FOR CLASS) */
if (dstyle == ROLL_FOR_ATTRIB || dstyle == ROLL_FOR_CLASS) {
    local_dispatch = NEED_A_ROLL;            /* SECOND "GET-NEXT" */
    }
else {
    local_dispatch = NEED_ATTRIBUTE_CHECK;   /* "GET/SET", FIRST "GET-NEXT" */
    }

IFLOGGING( L_TRACE ) {
    /* Get "TRACE" in the margin. . . */
    build_line_prefix(&bp->log_state, L_TRACE, buf, &ss);
    }

/*
| Loop to find a class or attribute and check to be sure whatever found
| really is what we think it is.
*/
/* while (we're not ALL_FINISHED) */
while (local_dispatch != ALL_FINISHED) {

    IFLOGGING( L_TRACE ) {
        sprintf(&buf[ss],
                "--Top of Derive Class/Attribute Loop, dispatch(%s)\n",
                local_disp_string[local_dispatch]);     LOGIT();
        }

    switch (local_dispatch) {

        case NEED_ATTRIBUTE_CHECK:
            /*
            | We may have been passed an SNMP "class" object or an "attribute"
            | object. Attributes have datatypes, classes don't.  We use this
            | fact to determine what we've got.
            |       - Once we've found an attribute, we should be able to step
            |         back one containment level to it's containing class.
            |
            | Perform a search on
            |          * current mir_object,
            |          * relationship "MIR_Structured_As"
            |          * from the top
            |  ... we're looking to see if "mir_object" has a data type. . .
            */
            mstatus =
                mir_search_rel_table(SEARCH_FROMTOP,      /* Search style    */
                                     *mir_object,         /* Search this. .  */
                                     m_list[M_STRUCT_AS].m,/* . . for this   */
                                     mclass,              /* Use this mclass */
                                     &m_nonterm,          /* Return nonterm  */
                                     &m_value             /* Return term     */
                                     );
            IFLOGGING( L_TRACE ) {
                sprintf(&buf[ss],
                        "----Search for MIR_Structured_As (as attrib check)\n"
                        );     LOGIT();
                sprintf(&buf[ss],
                        "---- in MIR Object (masa=%d) returns status of %s\n",
                        (*mir_object)->m.m.ex_add,
                        mir_error(mstatus)
                        );     LOGIT();
                }

            switch (mstatus) {              /* search return status */

                case MS0_FIND_TERM:
                case MS0_INVALID_MANDLE:
                    sprintf(msg,
                    MSG(msg127, "M127 - '%s' returned, MIR corrupt or coding error %d"),
                            mir_error(mstatus),
                            (*mir_object)->m.m.ex_add
                            );
                    SYSLOG( LOG_ERR, msg);
                    return(CI_NOSUCH);


                case MS0_FIND_NONTERM:
                    /* Datatype returned: it's an attribute, try to "back-up"
                    |  internally (within the MIR) to the class which should
                    |  contain it.  Use the original mir_object mandle to
                    |  receive class object.  Note the mandle in m_nonterm is
                    |  junk now, but we don't dump it explicitly, as it is in
                    |  the same class as the MIR object we were passed, and
                    |  it'll be dumped when the entire class is dumped by the
                    |  caller of this function.  (Plus we may re-use it
                    |  momentarily in code below ("Class Check")).
                    |  perform a search on:
                    |    * current mir_object,
                    |    * relationship "MIR_Contained_By"
                    |    * from the top
                    */
                    mstatus =
                    mir_search_rel_table(SEARCH_FROMTOP,  /* Search style    */
                                         *mir_object,     /* Search this. .  */
                                         m_list[M_CONT_BY].m,/* . for this . */
                                         mclass,          /* Use this mclass */
                                         mir_object,      /* Return nonterm  */
                                         &m_value         /* Return term     */
                                         );

                    IFLOGGING( L_TRACE ) {
                        sprintf(&buf[ss],
        "----Search for MIR_Contained_By (to step 'back' to Class MIR Object\n"
                                );     LOGIT();
                        sprintf(&buf[ss],
                                "---- returns status of %s\n",
                                mir_error(mstatus)
                                );     LOGIT();
                        }

                    switch (mstatus)  {  /* status Contained-By */

                        case MS0_FIND_NONE:
                            /*
                            | We've searched what we thought was a MIR object
                            | for an attribute, but it turned out to not have
                            | a containing class.  Consequently if rolling is
                            | enabled, we should roll on to the next MIR
                            | object that might be a class.
                            |
                            | show local dispatch as "Need a Roll" if we can.
                            */
                            local_dispatch = NEED_A_ROLL;

                            IFLOGGING( L_TRACE ) {
                                sprintf(&buf[ss],
                  "----No Containing Class: Need to Roll to next MIR Object\n"
                                        );     LOGIT();
                                }
                            break;


                        case MS0_FIND_NONTERM:
                            /*
                            | "mir_object" now points to SNMP "class" Object,
                            | we presume.  This is because the search was
                            | successful and the mandle that once pointed to
                            | the attribute MIR object has been re-used to
                            | point to what we presume to be it's containing
                            | class MIR object.  But this must be checked.
                            |
                            | show local dispatch as "Need a Class Check"
                            */
                            local_dispatch = NEED_CLASS_CHECK;

                            /*
                            | When we finally return to our caller, we want to
                            | indicate how we actually arrived at the class
                            | being returned. Here we we derived the class
                            | from the ATTRIBUTE object passed in.
                            */
                            local_return = CI_ATTRIBUTE;

                            IFLOGGING( L_TRACE ) {
                                sprintf(&buf[ss],
                       "----Containing Class is found as MIR Object masa=%d\n",
                                        (*mir_object)->m.m.ex_add
                                        );     LOGIT();
                                sprintf(&buf[ss],
                                        "---- need to do a Class Check\n"
                                        );     LOGIT();
                                sprintf(&buf[ss],
                               "---- (Status-for-return set to CI_ATTRIBUTE)\n"
                                        );     LOGIT();
                                }
                            break;


                        case MS0_FIND_TERM:
                        case MS0_INVALID_MANDLE:
                        default:
                             sprintf(msg,
                    MSG(msg129, "M129 - Missing 'MIR_Contained_By' Relationship: %s %d"),
                                     mir_error(mstatus),
                                     (*mir_object)->m.m.ex_add
                                     );
                             SYSLOG(LOG_ERR, msg);
                             return(CI_PROC_FAILURE);
                        }    /* switch Contained-By */
                    break;


                case MS0_FIND_NONE:
                    /*
                    | Ok, no datatype: it's a class, we presume.  We'll double
                    | check this by making sure that it does indeed have a
                    | "MIR_Indexed_By" relationship in it's table (required
                    | for all classes).
                    |
                    | show local dispatch as "Need a Class Check"
                    */
                    local_dispatch = NEED_CLASS_CHECK;

                    /*
                    | When we finally return to our caller, we want to indicate
                    | how we actually arrived at the class being returned.
                    | Here we note that it turned out to be just the current
                    | object, no attribute involved.
                    */
                    local_return = CI_CLASS;
                    IFLOGGING( L_TRACE ) {
                        sprintf(&buf[ss],
                                "----MIR Object is presumed to be a Class\n"
                                );     LOGIT();
                        sprintf(&buf[ss],
                                "---- need to do a Class Check\n"
                                );     LOGIT();
                        sprintf(&buf[ss],
                                "---- (Status-for-return set to CI_CLASS)\n"
                                );     LOGIT();
                        }
                    break;

                default:
                     CRASH(
                      MSG(msg236, "M236 - Unrecognized Return Status from MIR Subsystem"));

                }       /* structured-as */
            break;


        case NEED_CLASS_CHECK:
            /* ----------------------------------------------------------------
            |  Either the code above discovered what it thought was a class or
            |  the roll code below in this switch has rolled to what it thinks
            |  is a MIR object representing a class.  All MIR objects that
            |  represent a class have a "MIR_Indexed_By" relationship.  We look
            |  for that as the "class check".
            |
            | perform a search on:
            |   * current mir_object,
            |   * relationship "MIR_Indexed_By"
            |   * from the top
            |
            | Note we are using (for possibly the second time) mandle
            | "m_nonterm". It'll go into the same class as "mir_object".
            */
            mstatus =
            mir_search_rel_table(SEARCH_FROMTOP,      /* Search style    */
                                 *mir_object,         /* Search this. .  */
                                 m_list[M_INDEX_BY].m,/* . . for this    */
                                 mclass,              /* Use this mclass */
                                 &m_nonterm,          /* Return nonterm  */
                                 &m_value             /* Return term     */
                                 );

            IFLOGGING( L_TRACE ) {
                sprintf(&buf[ss],
                        "----Search for MIR_Indexed_By (as class check)\n"
                        );     LOGIT();
                sprintf(&buf[ss],
                        "---- in MIR Object (masa=%d) returns status of %s\n",
                        (*mir_object)->m.m.ex_add,
                        mir_error(mstatus)
                        );     LOGIT();
                }

            switch (mstatus) {
                case MS0_FIND_NONE:
                    /*
                    | No "MIR_Indexed_By" relationship found, therefore
                    | it must not be a class.  We need to start doing
                    | "GET_NEXT" thru the MIR (rolling) until we find an
                    | object that is a valid class or attribute.
                    */
                    local_dispatch = NEED_A_ROLL;

                    IFLOGGING( L_TRACE ) {
                        sprintf(&buf[ss],
                                "---- Search failed, we must roll\n"
                                );     LOGIT();
                        }
                    break;


                case MS0_FIND_TERM:
                case MS0_FIND_NONTERM:
                    /*
                    | "MIR_Indexed_By" was found, and it's target can legally
                    | be either Terminal or Non Terminal, so we have a class
                    | as far as we can tell.
                    */

                    /* if (local return is NOT SET YET) */
                    if (local_return == CI_NOT_SET_YET) {

                        /* set local return to CI_CLASS */
                        local_return = CI_CLASS;
                        }

                    local_dispatch = ALL_FINISHED;

                    IFLOGGING( L_TRACE ) {
                        sprintf(&buf[ss],
                            "---- Search succeeded: Valid Class, we're done\n"
                                );     LOGIT();
                        }
                    break;


                default:
                    /* CRASH "Mxxx - Coding Error, status=%s */
                    sprintf(msg,
                            MSG(msg128, "M128 - Coding Error, status=%s %d"),
                            mir_error(mstatus), (*mir_object)->m.m.ex_add
                            );
                    CRASH( msg );
                }
            break;


        case NEED_A_ROLL:
            /* ----------------------------------------------------------------
            | Either the 'Class Check' or 'Attribute Check' code in this switch
            | discovered that we need to roll on to another class or attribute
            | object.
            */

            /* Only if rolling is permitted do we actually roll */
            if (dstyle == NO_ROLL_AT_ALL) {
                return(CI_NOSUCH);
                }

            /*
            | When we finally return to our caller, we want to indicate how
            | we actually arrived at the class being returned.  Show we
            | 'rolled' into it.
            */
            mir_has_rolled = TRUE;      /* Override supplied value */

            /*
            | perform oid_to_mandle w/GET_NEXT search
            |
            | This'll step us 'forward' thru the MIR index to the next
            | MIR object whatever it is.  Whatever we get, we must check
            | to be sure it represents class or attribute.
            |
            | Note that with V1.4, this call can change the SMI we're working
            | "in", and hereafter in this function we'll be looking for OIDs
            | in the SMI that the "NEXT" OID falls in.
            */
            mstatus = mir_oid_to_mandle(GET_NEXT,       /* IN-Search Style */
                                        mir_object_oid, /* IN  */
                                        mir_object,     /* OUT */
                                        mclass,         /* IN  */
                                        &oid_smi);      /* OUT */

            IFLOGGING( L_TRACE ) {
                char    *oid_string=NULL;  /* Points to text version of  OID */

                sprintf(&buf[ss],
                        "----GET_NEXT request on OID(%s) returned status \n",
                        (oid_string = pe_oid_text(mir_object_oid))
                        );        LOGIT();                        
                sprintf(&buf[ss],
                        "---- %s, oid SMI=%s\n",
                        mir_error(mstatus),
                        oid_smi_names[oid_smi]
                        );        LOGIT();                        
                if (oid_string != NULL) free(oid_string);
                }

            switch (mstatus) {          /* oid_to_mandle return code */

                case MS0_FIND_EXACT:
                    /* if (derive style is ROLL_FOR_ATTRIB or ROLL_ALLOWED) */
                    if (dstyle == ROLL_FOR_ATTRIB || dstyle == ROLL_ALLOWED) {

                        /* set local dispatch for "NEED_ATTRIBUTE_CHECK" */
                        local_dispatch = NEED_ATTRIBUTE_CHECK;

                        IFLOGGING( L_TRACE ) {
                            sprintf(&buf[ss],
                                "---- Need ATTRIBUTE Check on new object\n");
                            LOGIT();
                            }
                        }

                    else {/* derive status can't be NO_ROLL_AT_ALL here */

                        /* set local dispatch for "NEED_CLASS_CHECK" */
                        local_dispatch = NEED_CLASS_CHECK;

                        IFLOGGING( L_TRACE ) {
                            sprintf(&buf[ss],
                                "---- Need CLASS Check on new object\n");
                            LOGIT();
                            }
                        }
                    break;

                default:
                    {
                    char xmsg[LINEBUFSIZE];  /* Error Message build buffer */

                    sprintf(xmsg,
                    MSG(msg151, "M151 - Execution Anomaly: MIR GET_NEXT on (%%s) rtned %s"),
                            mir_error(mstatus));
                    BLD_OID_ERRMSG(xmsg, mir_object_oid);
                    SYSLOG( LOG_ERR, msg );
                    }
                    /* FALL THRU */

                case MS0_FIND_NONE:
                    IFLOGGING( L_TRACE ) {
                        sprintf(&buf[ss],
                                "---- We're at the end of the MIR\n"); LOGIT();
                        }
                    return(CI_NOSUCH);
                }

            /*
            | perform a mir_mandle_to_oid() to obtain the OID of the
            | "current object" we're on.  Note that we insist that this
            | be an OID in the SMI returned above (ie the actual OID we 
            | 'rolled-to' in the GET_NEXT call above).
            */
            free(mir_object_oid->value);
            mstatus = mir_mandle_to_oid(*mir_object,    /* IN  */
                                        mir_object_oid, /* OUT */
                                        &oid_smi);      /* IN  */

            IFLOGGING( L_TRACE ) {
                sprintf(&buf[ss],
                        "----Request for OID on new current MIR Object\n"
                        ); LOGIT();
                sprintf(&buf[ss],
                        "---- (masa=%d) gives status = %s  oid-SMI=%s\n",
                        (*mir_object)->m.m.ex_add,
                        mir_error(mstatus),
                        oid_smi_names[oid_smi]
                        );
                        LOGIT();
                }                
          
            if (mstatus != MS_SUCCESS) {
                sprintf(msg,
                        MSG(msg130, "M130 - mandle-to-oid conversion failure %s %d"),
                        mir_error(mstatus),
                        (*mir_object)->m.m.ex_add
                        );
                SYSLOG( LOG_ERR,msg );
                return(CI_NOSUCH);   /* Temporary punt until we can fix MIR */
                }

            IFLOGGING( L_TRACE ) {
                char    *oid_string=NULL;  /* Points to text version of  OID */

                sprintf(&buf[ss],
                        "----OID returned was (%s)\n",
                        (oid_string = pe_oid_text(mir_object_oid))
                        ); LOGIT();
                if (oid_string != NULL) free(oid_string);
                }

            break;

        }  /* switch (local_dispatch) */

    }  /* while */

/* 
| Perform mir_mandle_to_oid() to obtain the "class" OID as the MIR sees it.
| (Again we insist it come from the same SMI as the original object we started
|  work on. . )
*/
mstatus = mir_mandle_to_oid(*mir_object, class_oid, &oid_smi);

/* if (return code is not MS_SUCCESS) */
if (mstatus != MS_SUCCESS) {
    sprintf(msg,
            MSG(msg152, "M152 - Execution Anomaly: mandle_to_oid returned %s, %d"),
            mir_error(mstatus),
            (*mir_object)->m.m.ex_add
            );
    SYSLOG(LOG_ERR, msg );
    return (CI_PROC_FAILURE);
    }
/*
|==============================================================================
| NOTE: If you get M152 above with "MS0_FIND_NONE", it probably means that
|       somebody forgot to specify an Object Identifier when they defined
|       the class in the MSL file.  The decimal <number> in the message can
|       be used with the ASCII dump of the MIR database to find the actual
|       object for which there was no OID.   Search the dump for the MIR object
|       with address <number> and that is the MIR object.  By looking at the
|       name of the object (which is the target of relationship
|       "MIR_Text_Name") in the MIR dump, you can discover where to repair
|        the MSL file.
|==============================================================================
*/

IFLOGGING( L_TRACE ) {
    char    *oid_string=NULL;  /* Points to text version of  OID */

    sprintf(&buf[ss],
            "Derive Loop has been exited.\n");
    LOGIT();
    sprintf(&buf[ss],
       "Request for %s of 'current MIR obj' (new Class Object)\n",
       oid_smi_names[oid_smi]);
    LOGIT();
    sprintf(&buf[ss],
            "    returns status %s and OID (%s)\n",
            mir_error(mstatus),
            (oid_string = pe_oid_text(class_oid)));
    LOGIT();
    if (oid_string != NULL) free(oid_string);
    }

/* if (we've rolled) */
if (mir_has_rolled == TRUE) {
    /* set local rtn to CI_ROLLED_ATTRIBUTE or CI_ROLLED_CLASS accordingly */
    local_return = (local_return == CI_CLASS) ? CI_ROLLED_CLASS :
                                                CI_ROLLED_ATTRIBUTE;
    }

IFLOGGING( L_TRACE ) {
    sprintf(&buf[ss],
            "mir_derive_CLASS returning status = %s\n",
            CI_status_string[local_return]);
    LOGIT();
    }
    
/* return local return code */
return (local_return);

}
/* mir_derive_INSTANCE - Derive Instance Information from Class MIR Object */
/* mir_derive_INSTANCE - Derive Instance Information from Class MIR Object */
/* mir_derive_INSTANCE - Derive Instance Information from Class MIR Object */

static void
mir_derive_INSTANCE (bp,
                     instance,
                     mir_object,
                     mclass
                     )
big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **instance;     /* ->> where to store the instance list      */
mandle          *mir_object;    /* Mandle describing MIR Object to be worked */
mandle_class    **mclass;       /*-> Mandle Class containing "mir_object"    */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "instance" is the address of a pointer that is to be set to point
    to an "instance list" carrying the instance information for this class.

    "mir_object" is the the mandle for the MIR object to be examined for
    instance information (it is expected to be a MIR object representing a
    "class").

    "mclass" is the address of the mandleclass pointer for mandle "mir_object".


OUTPUTS:

    On a successful return the following occurs:

    * "instance" is set to point to a linked list of "snmp_instance_t" structures
      containing the instance information for the object class returned and
      all of it's "containing" classes.

      The caller is responsible for reclaiming the storage allocated to
      this list AND to the object-id arc-list ("value") contained in each
      snmp_instance_t structure on the list. (See "Other Things to Know" for
      a discussion of this list).

    On an unsuccessful return,

    * instance pointer points to a NULL (which is set there by this code)


BIRD'S EYE VIEW:
    Context:
        The caller is mir_derive_CI in this module.

    Purpose:
        This function performs the MIR inquiries necessary to discover and
        return the instance information for the class and all containing
        classes above.


ACTION SYNOPSIS OR PSEUDOCODE:

    <load a NULL into the caller's instance list pointer>

    for (* For all classes in containment hierarchy starting at bottom...*)

        (* Extract the instance information by finding every occurrence of   *)
        (* the relationship "MIR_Indexed_By" in the class object and extract *)
        (* into an (allocated) instance block the OID of the target and the  *)
        (* target's datatype's ASN.1 Tag in "Common Agent Format".           *)

        <reset the search pointer in the class object "to the top">
        <set "pointer to first instance block for class" to NULL>

        do (* For all instance info (ie all identifying attribs) in class *)

            <perform a search on:
                * class object
                * relationship "MIR_Indexed_By"
                * FROMLAST>

            <assume extraction fails>
            switch (return code)

                case MS0_FIND_TERM:
                    if (terminal is "")
                        <create instance block w/default NULL info>
                        <show extraction OK>
                        break;
                    (* ELSE FALL THRU *)

                case MS0_INVALID_MANDLE:
                default:
                    <CRASH "Mxxx - %s returned, MSL invalid, MIR corrupt or
                     coding error">

                case MS0_FIND_NONTERM:
                    <perform instance OID + datatype extraction>
                    if (instance block created)
                        <show extraction OK>
                    break;

                case MS0_FIND_NONE:
                    if (there is no "first instance block" for class)
                        <return>
                    break;

            if (extraction went OK)
                if (pointer to first instance block for class is NULL)
                    <record pointer to current block as first>
                    <make caller's instance list pointer point to this block>
                <step instance pointer forward>

            while (extraction status is OK & search status was OK)

        (*
          If we reach here, we just processed at least one and possibly more
          groups of instance info for the current class.  We now want to grab
          the code for the class (in case of null instance info) and step
          backward to the containing MIR object (maybe a class, maybe not)
          and do it again for that level.
        *)

        <perform a search on:
                * class object
                * relationship "MIR_ID_Code"
                * FROMTOP>

        switch (return code)

            case MS0_FIND_TERM:
                <copy code to first instance block>
                <break>

            default:
                <CRASH "Mxxx - %s returned, MSL invalid, MIR corrupt or
                 coding error">

        
        <perform a search on:
                * class object
                * relationship "MIR_Contained_By"
                * FROMTOP>
                * return any non-terminal into current class mandle

        switch (return code)

            case MS0_FIND_TERM:
            case MS0_INVALID_MANDLE:
                <CRASH "Mxxx - %s returned, MSL invalid, MIR corrupt or
                 coding error">

            case MS0_FIND_NONTERM:
                <break>

            case MS0_FIND_NONE:
                <return>
        
        <reset current ptr to point to "next_class" cell in first_for_class>

    <return>


OTHER THINGS TO KNOW:

    This is nominally a Tier 2 function, as it uses Tier 1 functions to extract
    datatype information (ASN.1 Tag and Class), but it is only used internally
    to this module.

    The instance list may actually be a complicated list of lists depending
    on the circumstances.  The list contains instance information not only
    for the "lowest" level of the containment hierarchy (ie the level at
    which SNMP currently (as this is written) provides instance information
    for) but also for the "containing" classes.  Typically the instance
    information for these containing classes is an identifier with datatype
    "NULL".  If the MSL doesn't explicitly specify an identifying attribute
    for a class, this routine provides exactly one with datatype ASN.1 "NULL"
    and a 'magic' OID derived from the codes that make up the class containment
    hierarchy.

    The instance arcs at the end of an SNMP OID may be used in groups of
    one or more arcs to specify an instance of an identifying attribute.
    For the particular instance of the particular attribute named by the
    overall SNMP OID, there may be one or more identifying attributes for
    which instance information must be extracted from the instance arcs at
    the tail end of the overall SNMP OID.

    The instance information extracted from the MIR for any ONE identifying
    attribute goes into one "snmp_instance_t" block strung on the lists shown
    below.  If the MIR indicates that a particular class has two identifying
    attributes, then two snmp_instance_t blocks will appear in the list for that
    class's entry in the list.

    Each class may have just it's "header" snmp_instance_t block or a string
    of other blocks strung off it: as many as needed to describe the instance
    information for that class, one block per identifying attribute.
   
*/

{
char            msg[LINEBUFSIZE];  /* Error Message build buffer           */
mir_status      mstatus;           /* MIR inquiry return status            */
mir_value       m_value;           /* Where terminal value can be returned */
mandle          *m_nonterm=NULL;   /* Where non-terminal mandle is returnd */
BOOL            extraction_OK;     /* TRUE: Last instance extraction OK    */
snmp_instance_t *first_for_class;  /* -> First instance block in current cl*/
snmp_instance_t **current;         /* Local ptr to ptr to current block    */
snmp_instance_t *this=NULL;        /* Something for "current" to point at  */


/*
| When we return, this pointer is going to either be NULL if no instance
| info was extracted (implausible, but possible) or it points to the first
| instance block for the top-most class in the containment hierarchy.
|
| load a NULL into the caller's instance list pointer
*/
*instance = NULL;

/*
| Once the loop below has gone around once, "current" will point at the
| "next" cell WITHIN the (in general) last instance block created.  Since
| there is no "last" instance block before the loop begins, we give it a local
| cell to point it.  "current" actually does have to point to something
| because "mir_build_one_instance()" will load the cell pointed to by "current"
| with the instance block address that it creates, consequently . . . 
*/
current = &this;

/* For all classes in containment hierarchy starting at BOTTOM . . . */
for (;/*ever*/;)  {

    /* reset the search pointer in the class object "to the top" */
    mir_reset_search(mir_object);

    /* set "pointer to first instance block for class" to NULL */
    first_for_class = NULL;

    /*
    | Extract the instance information by finding every occurrence of
    | the relationship "MIR_Indexed_By" in the class object and extract
    | into an (allocated) instance block the OID of the target and the
    | target's datatype's ASN.1 Tag in "Common Agent Format".
    */

    do {   /* For all instance info (ie all identifying attribs) in class */

        /* perform a search on:
        |
        |    * class object         (described by mandle "mir_object")
        |    * relationship "MIR_Indexed_By"
        |    * FROMLAST             (to make us scan down the table)
        |
        */
        mstatus =
        mir_search_rel_table(SEARCH_FROMLAST,         /* Search style    */
                             mir_object,              /* Search this. .  */
                             m_list[M_INDEX_BY].m,    /* . . for this    */
                             mclass,                  /* Use this mclass */
                             &m_nonterm,              /* Return nonterm  */
                             &m_value                 /* Return term     */
                             );

        extraction_OK = FALSE;      /* Assume we can't do it */

        switch (mstatus)  {         /* return code */

            case MS0_FIND_TERM:
                /* ----------------------------------------------------------
                | The only legal value for a target of MIR_Indexed_By when the
                | target is a Terminal is the value "" (zero-length string)
                | which implies "IDENTIFIER=()" in the MSL, ie the class
                | doesn't have a MSL-specified identifying attribute, we
                | default to our special "Null" Identifying Attribute, whose
                | datatype is conjured up right here and put into an instance
                | block.
                */
                if (IS_MV_NUMBER((&m_value)))
                    if (MV_SNUMBER((&m_value)) == 0)
                        /* create instance block w/default NULL info */
                        mir_build_null_instance(bp, current);
                        extraction_OK = TRUE;   /* show extraction OK */
                        break;
                /* ELSE FALL THRU: MIR Compiler error      */
                /* FALL THRU FALL THRU FALL THRU FALL THRU */

                /* FALL THRU FALL THRU FALL THRU FALL THRU */
            case MS0_INVALID_MANDLE:
            default:
                sprintf(msg,
                        MSG(msg135, "M135 - MIR corrupt or coding error, status= %s, masa=%d"),
                        mir_error(mstatus),
                        mir_object->m.m.ex_add);
                CRASH( msg );


            case MS0_FIND_NONTERM:
                /* ----------------------------------------------------------
                | The MSL specified a real attribute as an identifying
                | attribute, and we must take the MIR object corresponding to
                | this identifying attribute and evaluate it's datatype and
                | build an instance block containing it's OID and ASN.1 tag
                | value.
                |
                | perform instance OID + datatype extraction
                */
                mir_build_one_instance(bp, current, m_nonterm);

                /* if (instance block created) show extraction OK */
                extraction_OK = (*current != NULL) ? TRUE : FALSE;
                break;


            case MS0_FIND_NONE:
                /* ------------------------------------------------------------
                | We've failed to find (another) MIR_Indexed_By relationship.
                |
                | This should never happen on the very first time around this
                | loop, but there is no check for it because this function is
                | only called if there is at least one MIR_Indexed_By
                | relationship in the class MIR object passed in.  On second
                | and subsequent times around this loop, this failure simply
                | means there are no more identifying attributes (signalled by
                | MIR_Indexed_By) in the current class object.
                |
                | If we reach here without having extracted at least one
                | identifying attribute for the class, (in other words, there
                | is no "first instance block" for the class) then it means
                | that what we're looking at isn't a "class" in the SNMP sense
                | and we should give up.  (This happens only after having
                | processed the lowest 'real' SNMP class successfully, and the
                | code below that executes the search for the MIR_Contained_By
                | relationship has popped us up to what is believed to be a
                | containing class).
                |
                | if (there is no "first instance block" for class)
                */
                if (first_for_class == NULL) {
                    /*
                    |  Before we return, we've got to patch up any
                    |  null instance blocks w/empty OIDs that might be in
                    |  the list.
                    */
                    mir_null_inst_oid_patchup(bp, *instance);
                    return;
                    }
                break;
            }


        /* if we really did extract something. . . . */
        if (extraction_OK == TRUE) {

            /*
            | We handle processing required to integrate another class
            | above all previously processed classes here.  We're catching
            | things on the first trip around this loop for each class.
            |
            | if (pointer to first instance block for class is NULL)
            */
            if (first_for_class == NULL) {

                /* record pointer to current block as first */
                first_for_class = *current;

                /*
                | make this new first-block point downward to first block for
                | the class below.  The caller's instance list pointer
                | previously pointed to the first-block in the class below
                | the one we're working on now (or to NULL if we're working
                | on the very first class).  We grab that pointer and stuff
                | it into the "next_class" (-below) cell of the first instance
                | block for the new class we're working on.
                */
                first_for_class->next_class = *instance;

                /*
                | make caller's instance list pointer point to this new block.
                | This wipes out the pointer we just copied above and
                | substitutes in a pointer to the just-created first-block
                | for the current class we're working on.  In effect, we've
                | re-pointed the caller to the next layer on the cake we're
                | building.
                */
                *instance = first_for_class;
                }

            /* step instance pointer forward */
            current = &(*current)->next;
            }

        }  /* extraction status is OK and search status was OK */
        while (extraction_OK == TRUE && (   mstatus == MS0_FIND_NONTERM
                                         || mstatus == MS0_FIND_TERM)   );


    /*
    | If we reach here, we just processed at least one and possibly more
    | groups of instance info for the current class.  We now want to
    | grab the ID Code (in case of null instance info) and store it in the
    | first instance block for the class.
    |
    | perform a search on:
    |       * class object
    |       * relationship "MIR_ID_Code"
    |       * FROMTOP
    |       * store code number in the first instance block for the class
    */
    mstatus =
    mir_search_rel_table(SEARCH_FROMTOP,          /* Search style    */
                         mir_object,              /* Search this. .  */
                         m_list[M_ID_CODE].m,     /* . . for this    */
                         mclass,                  /* Use this mclass */
                         &mir_object,             /* Return nonterm  */
                         &m_value                 /* Return term     */
                         );
    switch (mstatus) {


        case MS0_FIND_TERM:
            /* ----------------------------------------------------------------
            | Got it! Store it.  This value may be used by
            | mir_null_inst_oid_patchup() when it is called to fill in OIDs
            | in null instance blocks that use the class code sequence as part
            | of the OID.
            */
            /* copy code to first instance block */
            first_for_class->class_code = MV_SNUMBER((&m_value));
            break;


        /* ---------------------------------------------------------------- */
        default:
            sprintf(msg,
                MSG(msg154, "M154 - %s returned, MSL invalid, MIR corrupt or coding error"),
                    mir_error(mstatus)
                    );
            CRASH( msg );
        }


    /*
    | Now step backward (upward) to the containing MIR object
    | (maybe a class, maybe not) and do it again for the next level:
    |
    | perform a search on:
    |       * class object
    |       * relationship "MIR_Contained_By"
    |       * FROMTOP
    |       * return any non-terminal into current class mandle (mir_object)
    */
    mstatus =
    mir_search_rel_table(SEARCH_FROMTOP,          /* Search style    */
                         mir_object,              /* Search this. .  */
                         m_list[M_CONT_BY].m,     /* . . for this    */
                         mclass,                  /* Use this mclass */
                         &mir_object,             /* Return nonterm  */
                         &m_value                 /* Return term     */
                         );
    switch (mstatus) {

        case MS0_FIND_NONTERM:
            /* ----------------------------------------------------------------
            | Presumably the non-terminal returned by this call corresponds to
            | a containing class.  We'll know for sure once the loop above
            | tries to find the MIR_Indexed_By relationship.  At this point,
            | all looks well.
            */
            break;


        case MS0_FIND_NONE:
            /* ----------------------------------------------------------------
            | There is no containing class, no higher place to go.
            | Patch up OIDs in any null instance blocks created by
            | mir_build_null_instance() and then return.
            */
            mir_null_inst_oid_patchup(bp, *instance);
            return;



        case MS0_FIND_TERM:
        case MS0_INVALID_MANDLE:
            /* ------------------------------------------------------------- */
            sprintf(msg,
                MSG(msg153, "M153 - %s returned, MSL invalid, MIR corrupt or coding error"),
                    mir_error(mstatus)
                    );
            CRASH( msg );
        }

    /* reset current ptr to point to local "this" cell */
    current = &this;

    }   /* for (all classes) */
}

/* mir_build_one_instance - Extract One Instance's worth of info to block */
/* mir_build_one_instance - Extract One Instance's worth of info to block */
/* mir_build_one_instance - Extract One Instance's worth of info to block */

static void
mir_build_one_instance (bp, instance, inst_object )

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **instance;     /* ->> to store the addr of next inst blk    */
mandle          *inst_object;   /* Mandle describing MIR Object to be worked */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "instance" is the address of a pointer that is to be set to point
    to the next (to be created) "instance block" carrying the instance
    information that we're about to extract.  

    "inst_object" is the the mandle for the MIR instance object to be examined
     for it's datatype (ASN.1 tag) and it's OID.


OUTPUTS:

    On a successful return:

    * "instance" is set to point to a newly created instance block
      of type "snmp_instance_t" containing the instance information for the
      instance object.  Instance information is the OID and ASN.1 Tag
      in Common Agent format.

      The caller is responsible for reclaiming the storage allocated to
      this block AND to the object-id arc-list ("value") contained in the
      snmp_instance_t block.

    On an unsuccessful return:

    * No instance block is created.


BIRD'S EYE VIEW:
    Context:
        The caller is mir_derive_INSTANCE in this module.

    Purpose:
        This function performs the MIR inquiries necessary to discover
        the datatype and OID of one occurence of "instance" information
        (ie one "identifying attribute").


ACTION SYNOPSIS OR PSEUDOCODE:

    <show no instance block at current pointer>    
    <endeavor to convert the instance attribute MIR mandle to SNMP OID>
    if (convert return code is not SUCCESS)
        <endeavor to convert the instance attribute MIR mandle to *ANY* OID>

    switch (return code)
        case MS0_FIND_NONE:
            <SYSLOG "Mxxx - Instance Attribute w/no OID: masa(%d)>
            <return>

        case MS_SUCCESS:
            break;

        default:
            <CRASH  "Mxxx - mandle-to-oid conversion failure %s">

    <attempt a "mir_lookup_data_construct()" call to evaluate attribute
        datatype>

    switch (return status)

            (* Immediate Success *)
        case MS1_DC_BUILTIN:
            break;

            (* Not yet Success *)
        case MS1_DC_BUILTUP:
            (* Loop to descend however deeply we need to find datatype *)
            do
                <attempt a "mir_eval_data_construct()" call using context>
                switch (return status)
                         (* Success *)
                    case MS1_DC_BUILTIN:
                        break;

                         (* Not yet Success *)
                    case MS1_DC_BUILTUP:
                        break;

                         (* Hit constructed: Not for SNMP *)
                    case MS1_DC_BUILTIN_TMPLT:
                        <SYSLOG "Mxxx - Instance (oid) has constructed dt">
                        if (dissolve context block failed)
                            <CRASH "Mxxx - MIR Dissolve Failed (oid)">
                    default:
                        <CRASH "Mxxx - Fault from eval data construct">
                while (status is not MS1_DC_BUILTIN)
            break;

             (* Failure of some sort *)
        case MS1_NOT_A_VALUED_OBJECT:
            <SYSLOG "Mxxx - Instance (oid) has no value">
            <return>

        case MS1_EXACT_OBJ_NOT_FND:
            <SYSLOG "Mxxx - Instance (oid) is not exact">
            <return>

        case MS1_INIT_FAIL:
            <CRASH "Mxxx - MIR initialization failed">

        default:
            <CRASH "Mxxx- (%d) Unrecognized mir_lookup_data_construct() error">

    if (dissolve Tier 1 context block failed)
        <CRASH "Mxxx - MIR Dissolve Failed (oid)">

    if (attempt to allocate storage for instance block failed)
        <CRASH "Mxxx - memory exhausted">

    <store pointer to new instance block at caller's pointer>
    <initialize "next", "next_class" and "class_code" fields to null>
    <copy length and dynamic storage pointer to caller to return object ID>
    <convert MIR ASN.1 Tag and Class numbers to Common Agent format>
    <store Common Agent Format tag in instance block>

    <return>


OTHER THINGS TO KNOW:

    This is a Tier 2 function, as it uses Tier 1 functions to extract
    datatype information (ASN.1 Tag and Class), but it is only used internally
    to this module.

    We try to use the SNMP OID for the object we're interested in, but if
    that fails, we'll take any OID.
*/

{
char            msg[LINEBUFSIZE];       /* Error Message build buffer    */
object_id       inst_oid;               /* Temp storage for Instance OID */
mir_status      mstatus;                /* MIR Return code status        */
dc_context      *context=NULL;  /* Addr of ptr to be set to context blk  */
dc_ident        ident;     /* Data-Construct identification block        */
snmp_instance_t *inst;     /* Local pointer to instance block            */
mir_oid_smi     oid_smi;   /* SMI indicator for mir_mandle_to_oid()      */


/* show no instance block at current pointer */
*instance = NULL;

/* endeavor to convert the instance attribute MIR mandle to SNMP OID */
oid_smi = OID_SNMP;
mstatus = mir_mandle_to_oid(inst_object, &inst_oid, &oid_smi);

/* if (convert return code is not SUCCESS) */
if (mstatus != MS_SUCCESS) {
    /* endeavor to convert the instance attribute MIR mandle to *ANY* OID */
    oid_smi = OID_ANY;
    mstatus = mir_mandle_to_oid(inst_object, &inst_oid, &oid_smi);
    }

switch (mstatus) {      /* return code */
    case MS0_FIND_NONE:
        sprintf(msg,
                MSG(msg138, "M138 - Instance Attribute w/no SNMP OID: masa(%d)"),
                (int) inst_object->m.m.ex_add
                );
        SYSLOG( LOG_ERR, msg );
        return;

    case MS_SUCCESS:
        break;

    default:
        CRASH(MSG(msg139, "M139 - mandle-to-oid conversion failure"));
    }

/*
|  Attempt a "mir_lookup_data_construct()" call to evaluate attribute
|  datatype.  This is a Tier 1 function.
*/
mstatus = mir_lookup_data_construct(&inst_oid, &context, &ident);

switch (mstatus)   {    /* return status */

    /* Immediate Success */
    case MS1_DC_BUILTIN:
        break;

    /* Not yet Success */
    case MS1_DC_BUILTUP:

        /* Loop to descend however deeply we need to find datatype */
        do {
            /* attempt a "mir_eval_data_construct()" call using context */
            mstatus = mir_eval_data_construct(&context, &ident);

            switch (mstatus)  {  /* return status */

                /* Success */
                case MS1_DC_BUILTIN:
                    break;

                /* Not yet Success */
                case MS1_DC_BUILTUP:
                    break;

                /* Hit constructed: Not for SNMP */
                case MS1_DC_BUILTIN_TMPLT:
                    BLD_OID_ERRMSG(
                           MSG(msg140, "M140 - Instance (%s) has constructed datatype"),
                           &inst_oid
                           );
                    SYSLOG( LOG_ERR, msg );

                    /* if (dissolve context block failed) */
                    if (mir_dissolve_context(&context) != MS_SUCCESS) {
                        BLD_OID_ERRMSG(MSG(msg141, "M141 - MIR Dissolve Failed (%s)"),
                                       &inst_oid
                                       );
                        CRASH(msg);
                        }
                    break;

                default:
                    CRASH(MSG(msg142, "M142 - Fault from eval data construct"));

                }
            }   /* status is not MS1_DC_BUILTIN */
            while (mstatus != MS1_DC_BUILTIN);
        break;

         /* Failure of some sort */
    case MS1_NOT_A_VALUED_OBJECT:
        BLD_OID_ERRMSG(MSG(msg136, "M136 - Instance (%s) has no value"), &inst_oid);
        SYSLOG(LOG_ERR, msg);
        return;

    case MS1_EXACT_OBJ_NOT_FND:
        BLD_OID_ERRMSG(MSG(msg143, "M143 - Instance (%s) is not exact"), &inst_oid);
        SYSLOG(LOG_ERR, msg);
        return;

    case MS1_INIT_FAIL:
        CRASH(MSG(msg144, "M144 - MIR initialization failed"));

    default:
        sprintf(msg,
                MSG(msg145, "M145 - (%d) Unrecognized mir_lookup_data_construct() error"),
                mstatus);
        CRASH( msg );
      }

/*
| We fall out of this switch complex with "ident" loaded with what we need.
*/

/* if (dissolve Tier 1 context block failed) */
if (mir_dissolve_context(&context) != MS_SUCCESS) {
    BLD_OID_ERRMSG(MSG(msg146, "M146 - MIR Dissolve Failed (%s)"), &inst_oid)
    CRASH(msg);
    }

/* if (attempt to allocate storage for instance block failed) */
if ( (inst = (snmp_instance_t *) malloc(sizeof(snmp_instance_t))) == NULL) {
    CRASH(MSG(msg147, "M147 - memory exhausted"));
    }

/* store pointer to new instance block at caller's pointer */
*instance = inst;

inst->next = NULL;              /* initialize "next" field to null    */
inst->next_class = NULL;        /* initialize "next_class" field null */
inst->class_code = -1;          /* initialize "class_code" field null */

/* copy length and dynamic storage pointer to caller to return object ID */
inst->inst_oid.count = inst_oid.count;
inst->inst_oid.value = inst_oid.value;

/* convert MIR ASN.1 Tag and Class numbers to Common Agent format */
/* store Common Agent Format tag in instance block                */
inst->AVLtag = (ident.asn1_class << 30) | ident.asn1_tag;

/* return */
}

/* mir_build_null_instance - Build the Standard NULL Instance Info Block */
/* mir_build_null_instance - Build the Standard NULL Instance Info Block */
/* mir_build_null_instance - Build the Standard NULL Instance Info Block */

static void
mir_build_null_instance (bp, instance)

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t **instance;     /* ->> to store the addr of next inst blk    */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "instance" is the address of a pointer that is to be set to point
    to the null "instance block" (carrying the standard NULL instance
    information) that we're about to create.


OUTPUTS:

    On a successful return:

    * "instance" is set to point to a newly created instance block
      of type "snmp_instance_t" containing the standard NULL instance information

      The caller is responsible for reclaiming the storage allocated to
      this block AND to the object-id arc-list ("value") contained in the
      snmp_instance_t block.

    On an unsuccessful return:

    * No instance block is created.


BIRD'S EYE VIEW:
    Context:
        The caller is mir_derive_INSTANCE in this module.

    Purpose:
        This function generates an instance block that SNMP PE uses
        in lieu of instance information that is not present in the MIR
        for classes that have no instance information created for them in
        the MIR (ie have "IDENTIFIER=()" in their MSL definition).


ACTION SYNOPSIS OR PSEUDOCODE:

    <show no instance block at current pointer>

    if (attempt to allocate storage for instance block failed)
        <CRASH "Mxxx - memory exhausted">

    <store pointer to new instance block at caller's pointer>
    <initialize "next" field to null>
    <initialize "next_class" field to null>

    <store show "no OID" yet>
    <convert ASN.1 "NULL" Tag and Class numbers to Common Agent format>
    <store Common Agent Format tag in instance block>

    <return>


OTHER THINGS TO KNOW:

    We use the DNA Magic OID for "Null Identifying Attribute":

           1.3.12.2.1011.2.13.<entity class sequence>.126

    However, at the time this function is called, we don't know the
    entire entity class sequence!  Consequently, we can't construct the
    proper OID.

    This problem is handled by mir_derive_INSTANCE() (the caller) by
    calling "mir_null_inst_oid_patchup()" as it's final act.  This
    function can examine the class code stored at all the levels and
    create the proper OID in any instance block w/no instance OID
    (ie the instance blocks created by this function).
*/

{
snmp_instance_t      *inst;     /* Local pointer to instance block            */

/* show no instance block at current pointer */
*instance = NULL;

/* if (attempt to allocate storage for instance block failed) */
if ( (inst = (snmp_instance_t *) malloc(sizeof(snmp_instance_t))) == NULL) {
    CRASH(MSG(msg132, "M132 - memory exhausted"));
    }

/* store pointer to new instance block at caller's pointer */
*instance = inst;

/* initialize "next" field to null */
inst->next = NULL;

/* initialize "next_class" field to null */
inst->next_class = NULL;

/* store show "no OID" yet */
inst->inst_oid.count = 0;
inst->inst_oid.value = NULL;

inst->class_code = -1;

/* convert ASN.1 "NULL" Tag and Class numbers to Common Agent format */
/* store Common Agent Format tag in instance block */
inst->AVLtag = ASN1_C_NULL;

}

/* mir_null_inst_oid_patchup - Patchup OID in Standard NULL Instance Block */
/* mir_null_inst_oid_patchup - Patchup OID in Standard NULL Instance Block */
/* mir_null_inst_oid_patchup - Patchup OID in Standard NULL Instance Block */

static void
mir_null_inst_oid_patchup (bp, instance)

big_picture_t   *bp;            /*-> Big Picture Structure for SNMP PE       */
snmp_instance_t *instance;      /* -> Top of Instance List                   */


/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "instance" is the address the "first" instance block for the top-most
    class.  We scan all the classes downward from this block.


OUTPUTS:

    On return:

    All empty OIDs in all Standard Null Instance blocks have been filled in.

    The only reason we don't complete successfully is lack of heap storage,
    in which case we CRASH.


BIRD'S EYE VIEW:
    Context:
        The caller is mir_derive_INSTANCE in this module.

    Purpose:
        This function scans the hierarchy of class instance-info lists
        and extracts the class codes stored in the first instance block
        of each class list while simultaneously filling any missing OIDs
        in the Standard Null Instance blocks it encounters.


ACTION SYNOPSIS OR PSEUDOCODE:

    <construct DNA Magic OID prefix in local storage>
    for (each class from top down)
        <copy class code from first instance block into local OID>
        if (first instance block has empty OID and AVL tag "ASN1_C_NULL")
            if (attempt to allocate storage for OID arcs fails)
                <CRASH - "Mxxx Memory exhausted">
            <copy current OID arcs to allocated storage>
            <copy ending ".126">
            <copy arc count into oid>


OTHER THINGS TO KNOW:

    See the Other Things to Know section of mir_build_null_instance() for
    a complete discussion of why this function is necessary.

    Note that a Standard Null Instance block is typically the first and ONLY
    instance block in a list for a class, (because it got generated as a
    consequence of an empty IDENTIFIER=() clause).  Consequently this function
    only examines the first instance block for each class to see if it needs
    it's OID filled in (with datatype = ASN1_C_NULL).

    We build the "entity class sequence" as we descend the class hierarchy.
    At any given moment, the currently accumulated entity class sequence is
    the correct sequence for OID construction purposes.

    We use the DNA Magic OID for "Null Identifying Attribute":

           1.3.12.2.1011.2.13.<entity class sequence>.126

*/

#define OID_DEPTH 50
{
int             arcs[OID_DEPTH];        /* Where we build OID arcs      */
int             arc_index;              /* Points into "arcs[]"         */
int             i;                      /* Handy loop index             */
snmp_instance_t *inst;                  /* -> current Instance block    */


/* construct DNA Magic OID prefix in local storage */
arc_index = 0;
arcs[arc_index++] = 1;
arcs[arc_index++] = 3;
arcs[arc_index++] = 12;
arcs[arc_index++] = 2;
arcs[arc_index++] = 1011;
arcs[arc_index++] = 2;
arcs[arc_index++] = 13;

/* for (each class from top down) */
for (inst = instance; inst != NULL; inst = inst->next_class) {

    /* copy class code from first instance block into local OID */
    if (arc_index >= OID_DEPTH) {
        CRASH(MSG(msg155, "M155 - OID Arc limit exceeded"));
        }
    arcs[arc_index++] = inst->class_code;

    /* if (first instance block has empty OID and AVL tag "ASN1_C_NULL") */
    if (   inst->inst_oid.count == 0
        && inst->inst_oid.value == NULL
        && inst->AVLtag == ASN1_C_NULL
       ) {

        /* if (attempt to allocate storage for OID arcs fails) */
        if ((inst->inst_oid.value = (unsigned int *)
                   malloc( (arc_index + 1) * sizeof(int) )) == NULL) {
            CRASH(MSG(msg156, "M156 -  Memory exhausted"));
            }

        /* copy current OID arcs to allocated storage */
        for (i=0; i < arc_index; i++)
            inst->inst_oid.value[i] = arcs[i];

        /* copy ending ".126" */
        inst->inst_oid.value[arc_index] = 126;

        /* copy arc count w/126 into oid */
        inst->inst_oid.count = arc_index + 1;
        }
    }
}
