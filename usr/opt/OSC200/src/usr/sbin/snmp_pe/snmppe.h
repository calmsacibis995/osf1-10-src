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
 * @(#)$RCSfile: snmppe.h,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/08/02 18:06
:38 $
 */
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
 * Module SNMPPE.H
 *      Contains data structure definitions required by the SNMP Protocol
 *      Engine.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   June 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *          This module is included into the compilations of modules that
 *          comprise the SNMP Protocol Engine for the Common Agent.
 *
 *    Purpose:
 *       This module contains the internal data structure definitions
 *       required by the SNMP Protocol Engine code modules.
 *
 * History
 *      V0.0    June 1991               D. D. Burns
 *      V1.1    April 1992              D. D. Burns - Changes to support
 *                                       fix to MIR lookup functions to allow
 *                                       proper roll (GET-NEXT) from original
 *                                       SNMP OID instead of Class OID.
 *      V1.2    May 1992                D. D. Burns - More changes to allow
 *                                       proper rolling (GET-NEXT)
 *      V1.3    July 1992               D. D. Burns - Add support for
 *                                       TRACE debug message class
 *      V1.4    Sept 1992               D. D. Burns - Expand roll code from
 *                                       "rollababy" to "rollclass" &
 *                                       "rollattrib".
 *      V1.5    Sept 1992               D. McKenzie - Add include of
 *                                       <dce/idlbase.h> to make 'c89 -std'
 *                                       compatible (specifically the typedef
 *                                       of object_id).
 *      V1.6    Oct 1992                D. McKenzie - Added externs for all
 *                                       translatable text functions in
 *                                       snmppe_text.c.
 */

/* Module Overview: 
|
|   This file contains definitions for the C-structures and datatypes used
|   internally by the SNMP Protocol Engine.
|
|   It contains include files:
|       -  "iso_defs.h" for shared memory use w/Internet MOM
|       -  "snmp_mib.h" for shared memory use w/Internet MOM
|       -  "<pthread.h>" for IPC (threaded) operation
|
*/

/*
|==============================================================================
|   MAJOR, MINOR
|
|   These symbols are used to form the "Vx.y" Version Identifier in the
|   initialization message logged by SNMP PE into the system log when it
|   starts up.  MAJOR is "x" and MINOR is "y".  (Done in "init_main()").
|
|==============================================================================
*/
#define MAJOR "1"
#define MINOR "10"


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
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif


/* Define MACRO to be used for displaying translatable text */
#ifdef NL
#define MSG(msg_name, string)  msg_name()
#else
#define MSG(msg_name, string)  string
#endif /* NL */

/*
|==============================================================================
|   If we doing "IPC" (rpc), then this implies we're using the threads
|   package.  The SNMP PE code is sensitive to the symbol NOIPC (as the
|   original SNMP PE code was) and also to the symbol THREADS, which
|   we define here according to the value of NOIPC.
|
|   SNMP PE code for rpc depends on "NOIPC", while the thread-dependent
|   code follows "THREADS".
|
|   To handle more than one inbound PDU at once, MTHREADS should be set
|   to the number of ADDITIONAL PDUS (beyond 1) that we desire to support
|   handling AT ONCE.  (2 PDUS at once requires MTHREAD = 1, see documentation
|   in module snmppe_main.c)
|
|==============================================================================
*/
#ifndef NOIPC
# define THREADS 0
/* define MTHREADS 1   --define this after adding any necessary code */
# include <pthread.h>
#endif /* NOIPC */

#include "evd_defs.h"

/*
|==============================================================================
| MIR_STATS_TO_STDERR()
|
|   This macro conditionally invokes the MIR Tier 0 function to dump
|   mandle/class statistics to stderr.  Normally this macro expands to
|   nothing, while in a debug situation the following symbol should be
|   set explicitly if desired to get the dump.
|
|   The macro does not mutex-lock "stderr", so we never allow this macro
|   to expand to something in a threaded-environment (even though the
|   "mir_debug_statistics()" function can operate properly in a threaded
|   environment).
|==============================================================================
*/
/* #define DO_MIR_STATISTICS */

#ifndef THREADS

#ifdef DO_MIR_STATISTICS
/*
| NOTE: To avoid dragging in "mir.h" we include the prototype for the MIR
|       function invoked by this macro right here.
*/
/* mir_debug_statistics - Dump In-Use Statistics for Mandles/Classes */
void mir_debug_statistics PROTOTYPE((
int     *,              /* Address of int to rcv Mandle-in-use count      */
int     *,              /* Address of int to rcv Mandleclass-in-use count */
int     *               /* Address of int to rcv Free Mandle/class  count */
));

#define MIR_STATS_TO_STDERR(user_msg)                                   \
   { int m_count;       /* Mandles-In-Use Count       */                \
     int c_count;       /* MandleClasses-In-Use Count */                \
     int f_count;       /* Free Mandle/Class cells    */                \
                                                                        \
     mir_debug_statistics( &m_count, &c_count, &f_count );              \
     fprintf(stderr,                                                    \
      "DEBUG       %s : MIR Mandles : %d   Classes : %d   Free : %d\n", \
             user_msg, m_count, c_count, f_count                        \
             );                                                         \
   }
#else
#define MIR_STATS_TO_STDERR(msg)
#endif

#else
#define MIR_STATS_TO_STDERR(msg)
#endif /* THREADS */



/*
|
|   NULL (if we need it)
|
*/
#ifndef NULL
#define NULL ((void *)0L)
#endif

#ifndef BOOL_DEF
/*
|   Primitive BOOLEAN type
|
*/
typedef
    int
        BOOL;
#define BOOL_DEF
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



/*
|==============================================================================
|   LINEBUFSIZE
|
|   This symbol is used to declare the size of character arrays into which
|   messages (usually error messages) are built.  It is also used to declare
|   the size of the buffer used to receive lines from the configuration file.
|
|==============================================================================
*/
#define LINEBUFSIZE 250


/*
|==============================================================================
|   PREFIXSIZE
|
|   This symbol is used to declare the size of the name of a "message class"
|   that is prefixed to the beginning of all diagnostic output.  It should
|   never be shorter than the names given the message classes in function
|   init_cmdline_args() in snmppe_init.c, and it should include a padding
|   space.  Current design minimum is 12 characters.
|
|==============================================================================
*/
#define PREFIXSIZE 12


/*
|==============================================================================
|   MAXPDUSIZE
|
|   This symbol is used to declare the size of the buffer that inbound PDU's
|   are received into.
|
|==============================================================================
*/
#define MAXPDUSIZE 484        /* RFC-specified maximum PDU size IN BYTES */

/*
|==============================================================================
|   MAXERRMSG
|
|   This symbol is used to declare the size of the buffer that error messages
|   are built into by the pe_log() function.
|
|==============================================================================
*/
#define MAXERRMSG 300



/*
|==============================================================================
|   VAR_BIND_SIZE
|
|   Size of the buffer into which the ASN.1 representation of the varbind list
|   is encoded (or copied to in the case of a decode).
|
|==============================================================================
*/
#define VAR_BIND_SIZE 2000



/*
|==============================================================================
|   MAX_COMM_NAME_SIZE
|
|   Maximum size of the buffer into which the ASN.1 representation of the 
|   community name is decoded.
|
|==============================================================================
*/
#define MAX_COMM_NAME_SIZE 50



/*
|==============================================================================
|   Memory setting and zeroing functions.
|   This bcmp() (since it uses memcmp()) performs a superset of the basic
|   ULTRIX bcmp()... the superset characteristic of returning a signed 
|   indication as to which string is "greater".
|==============================================================================
*/
#define bzero(x,y)    memset((x), 0, (y))
#define bcmp(x,y,z)   memcmp((x),(y),(z))
#define bcopy(x,y,z)  memcpy((y),(x),(z))

/*
|==============================================================================
|  LOG, SYSLOG, IFLOGGING
|
|   These macros simplify coding logic surrounding the issuing of a debug or
|   error message via a call to function "pe_log()".
|
|   All macros use a convention that the function containing the invocation
|   must have in scope at the point of invocation a variable named "bp" that
|   points to the "big_picture" structure containing the "log_state"
|   structure.
|
|==============================================================================
*/
#define LOG( class, msg )   pe_log(&bp->log_state, class, msg, 0)
#define SYSLOG( code, msg ) pe_log(&bp->log_state, L_SYSLOG, msg, code)
#define IFLOGGING( class )  if ( (bp->log_state.log_classes & class) )



/*
|==============================================================================
|  CRASH
|
|   This macro simplifies coding logic surrounding the issuing of an
|   error message and error exit via a call to function "crash_exit()".
|
|   This macro usees a convention that the function containing the invocation
|   must have in scope at the point of invocation a variable named "bp" that
|   points to the "big_picture" structure.
|
|   This macro "wraps" all calls to "crash_exit" so we can spiff up things
|   at a later date more easily if need be.
|
|==============================================================================
*/
#define CRASH( msg )   crash_exit( bp, msg )



#ifndef OID_DEF
/*
|==============================================================================
|  ISO Object ID
|
|   This structure is of the same format as the "object_id" structure in
|   the CA "man_data.idl" file.  Presumably this definition should be
|   removed when the MIR is integrated with the CA code.
|
|==============================================================================
*/
#include "pe.h"  /* this file includes ALL 'idl' structures we need */

/* typedef
    struct {
        int                 count;
        unsigned int       *value;
        } object_id;
*/
#define OID_DEF
#endif

/*
|==============================================================================
|  SNMP Statistics Structure
|
|   This structure is used by SNMP PE to record SNMP agent statistics.
|   The MOM references this structure (kept in shared memory) to report
|   the statistics on behalf of SNMP PE.
|
|==============================================================================
*/
#include "iso_defs.h"
#include "snmp_mib.h"


/*
|==============================================================================
|  Logging/Debugging Selection
|
|   This data structure contains all the information required for the
|   "log()" SNMP PE function.
|
|   Any function that needs to log something (for normal operation
|   or debugging operation) must pass the address of this structure to "log()".
|
|   Bit flags define Message Classes. If the bit is SET in "log_classes" below
|   at runtime, then messages in that class will be logged.
|==============================================================================
*/

/* Basic Classes */
#define L_SYSLOG        0x00001  /* Log to SYSLOG                       */

#define L_PDUGETIN      0x00002  /* Log Inbound GET PDUs                */
#define L_PDUGETOUT     0x00004  /* Log Outbound GET (Response) PDUs    */
#define L_PDUGETERR     0x00008  /* Log Error responses to GETs         */
#define L_PDUSETIN      0x00010  /* Log Inbound SET PDUs                */
#define L_PDUSETOUT     0x00020  /* Log Outbound GET (Response) PDUs    */
#define L_PDUSETERR     0x00040  /* Log Error responses to SETs         */
#define L_PDUNXTIN      0x00080  /* Log Inbound GET-Next PDUs           */
#define L_PDUNXTOUT     0x00100  /* Log Outbound GET (Response) PDUs    */
#define L_PDUNXTERR     0x00200  /* Log Error responses to GET-Nexts    */
#define L_OIDANOMLY     0x00400  /* Log OID Anomaly Error Messages      */
#define L_ASN1IN        0x01000  /* Log ASN1 Dump of Inbound PDU        */
#define L_ASN1OUT       0x02000  /* Log ASN1 Dump of Outbound PDU       */
#define L_ASN1TRAP      0x04000  /* Log ASN1 Dump of Trap PDUs issued   */
#define L_TRACE         0x10000  /* Log TRACE messages of PE processing */
/* -- Add new individual classes here, bump LOG_CLASS_COUNT below --*/

/* Aggregate Classes for "PDU" */
#define L_PDUSUMIN      (L_PDUGETIN  | L_PDUNXTIN  | L_PDUSETIN )
#define L_PDURSPOUT     (L_PDUGETOUT | L_PDUNXTOUT | L_PDUSETOUT)
#define L_PDUERROUT     (L_PDUGETERR | L_PDUNXTERR | L_PDUSETERR)
#define L_PDUSUMOUT     (L_PDURSPOUT | L_PDUERROUT)
#define L_PDU           (L_PDUSUMIN  | L_PDUSUMOUT)

/* Aggregate Class for "ASN1" */
#define L_ASN1          (L_ASN1IN | L_ASN1OUT)
/* --- Add new aggregates here, bump LOG_CLASS_COUNT below ---*/

#define LOG_CLASS_COUNT 21
/* NOTE: Change "LOG_CLASS_COUNT" if you add to this list.  Then        */
/*       add code in "snmppe_init.c" routine "init_cmdarg()" to support */
/*       parsing from command line.                                     */

typedef
    struct {
        BOOL    debugging_mode;              /* TRUE: Debugging, no syslog() */
        int     log_classes;                 /* Bit flags defined below      */
        char    *log_file_name;              /* NULL: use stderr             */
        FILE    *log_file;                   /* Debug output goes here       */
        char    *dbg_name[LOG_CLASS_COUNT];  /* Debugging Class Names        */
        int     dbg_flag[LOG_CLASS_COUNT];   /* Corresponding Bit Flags      */
#ifdef MTHREADS
        pthread_mutex_t log_file_m;          /* Protects "log_file"          */
#endif
        } log_state_t;

/*
|==============================================================================
|  Community
|
|   This data structure contains all the information required to describe
|   a "community".
|
|   The community may be one that the PE "belongs to" according to the config
|   file or one that it should send traps to.  Fields in this structure
|   support BOTH requirements, so some fields may be superfluous in a context.
|
|==============================================================================
*/
typedef
    struct community {
        char            *comm_name;     /* Community Name                 */
                                        /*  (null terminated, in the heap)*/
                        /* NOTE: We can't handle true arbitrarily named   */
                        /* community names because they come out of the   */
                        /* config file which perforce must be ASCII       */
        unsigned int    comm_addr;      /* Community Address              */
                                        /*  (in network byte order)       */
        int             access_mode;    /* (One of codes defined below)   */
  /*----char            *view_name;---*//* View Name                      */
                                        /*  (null terminated, in the heap)*/
        struct community *next;         /* Pointer to next in the list    */
        } community_t;

/* These values are legal in field "access_mode" above */
#define mode_none      0
#define mode_readOnly  1
#define mode_writeOnly 2
#define mode_readWrite 3



/*
|==============================================================================
|  PDU Type
|
|   The kinds of PDUs we may have to deal with in SNMP-land.
|
|   NOTE WELL:  The values of GET, GETNEXT, GETRESP, SET and TRAP have been
|               explicitly chosen to match the corresponding values in header
|               file "snmp_def.h" used by the ASN.1 encoding routines. They must
|               match!  Don't change 'em!  Why ask for trouble?  (Also, they match the
|               SNMP spec. . .)
|==============================================================================
*/
typedef
    enum {
      GET =     0,      /* Get Request          */
      GETNEXT = 1,      /* Get-Next Request     */
      GETRESP = 2,      /* Get-Response         */
      SET =     3,      /* Set Request          */
      TRAP,             /* Trap                 */
      UNKNOWN           /* Unknown              */
      } pdu_type;

/*
|==============================================================================
|  Trap PDU Type
|
|   The kinds of Traps we may have to generate.
|
|   NOTE WELL:  These values are taken from a recommended RFC. Don't change.
|
|==============================================================================
*/
typedef
    enum {
      coldStart =             0,
      warmStart =             1,
      linkDown =              2,
      linkUp =                3,
      authenticationFailure = 4,
      egpNeighborLoss =       5,
      enterpriseSpecific =    6
      } trap_pdu_type;



/*
|==============================================================================
|  Class/Instance Status Codes
|
|   These are status codes returned by "mir_derive_CI()" in snmppe_mir.c,
|   and also used by functions local to that module in support of _CI().
|
|   NOTE: If you add/delete codes, you must change the definition of
|         module-wide static definition of "CI_status_string[]" in file
|         snmppe_casend.c.
|==============================================================================
*/
typedef
    enum {
      CI_ATTRIBUTE,        /* MIR Object was an Attribute                    */
      CI_CLASS,            /* MIR Object was a Class                         */
      CI_ROLLED_CLASS,     /* Rolled to valid class given MIR Object OID     */
      CI_ROLLED_ATTRIBUTE, /* Rolled to valid attribute given MIR Object OID */
      CI_NOSUCH,           /* No class can be derived from MIR Object        */
      CI_PROC_FAILURE,     /* MIR Processing Failure encountered (generr)    */
      CI_NOT_SET_YET       /* NULL Value, No Value Yet: used in snmppe_mir.c */
      } mir_derive_CI_status;

/*
|==============================================================================
|  Error Status Values
|
|   The valid values for the "error_status" field in a PDU destined for a
|   manager.  Note that these values are specified by the "recommended" RFC,
|   so don't change them!
|
|   Note two added 'bogus' values "rollattrib" and "rollclass".  These should
|   NOT appear in any PDU, but are defined here so that the receiving thread
|   can easily signal the sending thread thru the error_status cell in the
|   varbind list entry block to roll to the next 'thing' for GET-NEXT
|   requirements.
|
|   These are SNMP PE Internal Only, and must be replaced with a valid
|   code from the specified group before the response PDU is built from the
|   varbind entry block.
|
|   The values given "rollclass"/"rollattrib" below are arbitrary, specified
|   only to be different from the legal values.
|
|   If you change this list, you must change static array "e_status_string[]"
|   in module "snmppe_casend.c".  If you add more "PDU-LEGAL" codes (because
|   the SNMP definition gets expanded (!), best to just bump the internal
|   codes up and keep the list monotonic.
|==============================================================================
*/
typedef
    enum {
      /* PDU-"LEGAL" */
      noError =    0, /* No Error being reported                             */
      tooBig =     1, /* Generated resp-PDU is tooBig to construct and return*/
      noSuch =     2, /* OID in incoming request specifies no valid object   */
      badValue =   3, /* Value in SET request was incorrect                  */
      readOnly =   4, /* Attempt to SET a readOnly object's value            */
      genErr =     5, /* An error that wasn't one of the above               */

      /* PDU-"BOGUS" */
      /* SNMP PE INTERNAL: Roll to the next 'class' for GET-NEXT */
      rollclass  = 6,
      /* SNMP PE INTERNAL: Roll to the next 'attribute' for GET-NEXT */
      rollattrib = 7
      } e_status_t;


/*
|==============================================================================
|  Instance Information Structure
|
|   This structure is used to hold just the essential information about the
|   instance information for a class as extracted from the MIR.
|
|   This definition is in essence identical to Mary Walker's original PE
|   "RDNstruct" structure, although the field definitions differ slightly, and
|   there is provision for multiple classes as well as multiple instances per
|   class.
|==============================================================================
*/
typedef
    struct snmp_instance {
      object_id         inst_oid;      /* Object ID of the Instance Attr */
      int               AVLtag;        /* Tag value of datatype for Inst */
      int               class_code;    /* Extracted ID Code for class    */
      struct snmp_instance *next;      /* Next instance block (if any)   */
                                       /*               FOR THIS *CLASS* */
      struct snmp_instance *next_class;/* Next class ("down") header     */
                                       /*      instance block (if any)   */
      } snmp_instance_t;

/*
|==============================================================================
|  Varbind Entry Block
|
|   This data structure contains all the information found in one varbind
|   ENTRY in an incoming PDU message.  Multiple instances of this block
|   strung on a list constitute the contents of the entire "varbindlist"
|   in an inbound PDU.
|
|   These blocks are strung off the "varbind_list" in the Service block
|   structure for the PDU in which they were found.
|
|   Additional fields in this entry block provide for holding info for the
|   PDU when it is returned to the SNMP manager in the event of a successful
|   GET/GET-NEXT.
|
|   When all processing is finished for an inbound pdu associated with a
|   service block, the status of the varbind entries on the varbind_list
|   for that PDU are made "inactive" by setting the "vb_entry_id" value to
|   "zero" (="not in use").
|
|   When a subsequent PDU arrives and the service block is re-used, these
|   existing varbind entry blocks may also be re-used.
|
|   When created from allocated storage, proper initialization consists
|   of zero-filling the entire structure.
|
| Details about "vb_entry_id"
| --------------------------
|   Cell "vb_entry_id" when non-zero (for an active entry in an active
|   Service Block) carries the value that was placed in the "invoke id"
|   of the CA request that was issued to service this varbind entry.
|
|   This "invoke id" is constructed by taking the low-order 3 bytes of the
|   value of "svc_count" in the Service Block for this varbind list, shifting
|   them left one-byte's worth of bits and then ORing in a "object class
|   group count".  When the varbind list is examined, the entries are
|   grouped by class.  All sequential entries that have the same OID for
|   the object class form a single "object class group"  Each group is
|   dispatched to the CA in one request.  Each request is assigned a number
|   (starting with "1") as it is dispatched to the CA.  This number is the
|   "object class group count" mentioned above.
|
|   Therefore, all vb_entry blocks for one CA request have the SAME value
|   in "vb_entry_id".
|
|   With this scheme we're in trouble (by duplicating invoke_id's) if:
|
|       * More than 256 entries occur in any one PDU (ie. the "object group
|         class count" wraps) OR
|       * A request gets hung in the CA and more than 16 million some-odd
|         PDUs (24 bits of unsigned integer) are subsequently successfully
|         processed and "svc_count" wraps and we wind up assigning the same
|         "svc_count" value that the hung request has to a new request.
|
|   Neither is impossible, but either is fairly improbable.
|
|==============================================================================
*/

typedef
    struct vb_entry {
      int               vb_entry_id;    /* "0" = Not In Use, otherwise it  */
                                        /* contains a CA 'invoke_id' value */
      struct vb_entry   *next;          /* -> Next Varbind entry on list   */

      object_id         class_oid;      /* Class OID of "snmp_oid" below,  */
                                        /* or current "class" if GET-NEXT  */
                                        /* (Arc storage must be reclaimed) */

      snmp_instance_t   *inst_list;     /* MIR Instance Information List   */
                                        /* associated w/"class_oid"        */
                                        /* (Storage must be managed and    */
                                        /*  reclaimed).                    */

      object_id         snmp_oid;       /* OID of "current" SNMP Object    */
                        /* Initially, "snmp_oid" is always a copy of the OID
                        |  in the "in_entry" AVL.  For GET/SET, that is as far
                        |  as it ever goes.  For GET-NEXT, whenever a roll
                        |  occurs, the 'next' object rolled-to becomes the
                        |  new "current object", and "snmp_oid" is set to the
                        |  OID of that new "current object".  Arc storage must
                        |  be managed and reclaimed.
                        */

      object_id         *orig_oid;      /* OID of "Original" SNMP Object    */
                        /* "orig_oid" is always the OID in the "in_entry" AVL.
                        |  This cell exists so that casend_bld_instance_AVL()
                        |  can see how many arcs there were in the original OID
                        |  and process them (for GET/SET and the FIRST
                        |  GET-NEXT) request.
                        |  It's storage is actually that of "in_entry" and is
                        |  recovered when the AVL is released.
                        */

      avl               *in_entry;      /* As parsed from Inbound PDU      */
      avl               *out_entry;     /* As returned from CA for "GET"   */
      e_status_t        reply_error;    /* Requested Reply Error Status    */
      } varbind_t;

/*
|==============================================================================
|  Service Block
|
|   This data structure contains all the information required to describe
|   the context associated with one received inbound PDU (request) from an
|   SNMP manager.
|
|   One of these blocks is present for each inbound PDU that the SNMP PE
|   is in the process of handling (servicing).
|
|   These blocks are strung off the "service_list" in the Big Picture
|   structure.  Once created, blocks are never removed from this list.
|
|   When all processing is finished for an inbound pdu associated with a
|   service block, the status of the block moves to "inactive" and storage
|   used by data elements in the service block is left "as-is".
|
|   When a subsequent PDU arrives, if an inactive block is present, it is
|   re-used (right after old heap storage pointed to by cells in the block is
|   released) otherwise new storage is allocated.
|
|   See function "casend_get_service()" in module snmppe_casend.c which
|   manages these blocks on the service list.
|==============================================================================
*/
typedef
    struct service {
        BOOL            svc_active;   /* TRUE: This block is In-Use      */
                        /* (See doc for Big Picture: mutex protected)    */

        struct service  *next;        /* Pointer to next service block   */
                                      /* in the "service_list"           */
                        /* (See doc for Big Picture: mutex protected)    */

        community_t     *ok_community;/* -> Validated Community or NULL  */
                                      /* (Points to entry in bp->        */
                                      /*  community_list when auth=OK)   */

        struct sockaddr_in inbound;   /* Inbound Socket address          */

        int             svc_count;    /* Value of "service_counter" when */
                                      /* PDU was received                */
        /* NOTE:  This serves to uniquely identify a good PDU w/in SNMP  */
        /* PE so we don't get confused if two copies of the same PDU     */
        /* arrive.                                                       */

        int             replies_rcvd; /* Count of replies received from  */
                                      /* the Common Agent for this PDU   */

        /* ======================= PDU Contents ======================== */
        unsigned int    version;      /* SNMP Version received           */

        char            *comm_name;   /* Community Name (in heap storage)*/
        int             comm_namelen; /* Amount of heap storage for above*/

        pdu_type        pdu;          /* GET/GETNEXT/SET REQUEST         */
        unsigned int    request_id;   /* Manager's request id value      */
        unsigned int    error_status; /* (for use in response)           */
        unsigned int    error_index;  /* (for use in response)           */
        varbind_t       *varbind_list;/* Deserialized "varbind" list     */
        /* ======================= PDU Contents ======================== */

        } service_t;

/*
|==============================================================================
|  Big Picture Structure
|
|   This data structure contains all the global data items
|   that modules in the SNMP PE may reference.  These data items are
|   purposefully collected into this data structure so as to avoid the use
|   of ANY globally referenced data in the SNMP PE.  This makes reading
|   the code much easier.  Additionally, debugging is easier.
|
|   Most of the items in this structure are set once at initialization time
|   and are then "read-only" by the threads that comprise the SNMP PE
|   execution context.
|
|   Those items which are subject to change by more than just one thread
|   are protected by a mutex that is also part of this structure.
|
|   Special note on the mutex protecting "service_list":
|       Acquisition of the associated mutex for this list is required to:
|               * Add another service block to the list (by allocation and
|                 change of the "next" cell of the last block on the list)
|               * Change the status of any service block on the list
|                 to "ACTIVE" (by setting the value of field "svc_active"
|                 to "TRUE").
|       Acquisition of the mutex is required because more than one thread
|       may be attempting to do the same thing (get a block).
|
|       Acquisition of the mutex is NOT REQUIRED to change the status of
|       a currently active block to INACTIVE (svc_active = FALSE) because
|       only the "owning thread" will attempt to do this, and it is an
|       atomic operation.
|
|==============================================================================
*/

typedef
    struct {
      /* Inbound Port Information */
      int            in_socket;         /* Inbound Socket                  */
      int            in_port;           /* Inbound Port to use from cmdline*/
      /* ---------- Inbound protected by mutex "in _m" IF MULTIPLE SENDING */
      /* ---------- THREADS ("MTHREADS" defined) is implemented.  See      */
      /* ---------- comment in main().                                     */

      /* Outbound (Trap) Port Information -- all fields protected by mutex */
      struct sockaddr_in outbound;      /* Outbound Socket Address info    */
      int            out_socket;        /* Outbound Socket                 */
      int            out_port;          /* Outbound Port to use: cmdline   */
      struct sockaddr_in our_addr;      /* Host Machine Address            */
      int            our_addr_length;   /* Length of the Host Address      */
      /* ---------- Outbound protected by mutex "out_m" ------------------ */

      char           *config_file_name; /* Name of Config File used        */
      int            service_counter;   /* Incremented w/each OK in-pdu    */
      log_state_t    log_state;         /* Current Logging state info      */
      community_t    *community_list;   /* List of known Communities       */

      community_t    *trap_list;        /* List of known Trap Communities  */
      service_t      *service_list;     /* List of Inactive/Active Service */
                                        /* Blocks (Mutex Protected)        */
      struct snmp_stats *statistics;    /* -> Shared Mem. Statistics Block */
                                        /* (Mutex Protected)               */
#ifdef NOIPC
      struct snmp_stats s_stats;        /* Statistics go right here for a  */
#endif /* NOIPC */                      /* single image execution          */

#ifdef MTHREADS
 /*-- pthread_mutex_t in_m; --- */      /* Protects inbound port fields    */
 /*-- IF MULTIPLE SENDING THREADS gets implemented ----------------------- */

      pthread_mutex_t out_m;            /* Protects outbound port fields   */
      pthread_mutex_t statistics_m;     /* Protects Statistics block       */
      pthread_mutex_t service_list_m;   /* Protects "service_list"         */
#endif

#ifdef THREADS
      pthread_t       recv_thread;      /* Receiving Thread                */
#endif

      /* This is present in the No-IPC version of SNMP PE in order to      */
      /* satisfy msi_* call interface, but is never initialized nor "used" */
      /* unless symbel NOIPC is NOT defined (ie, we're really using RPC)   */
      management_handle rpc_callback;   /* RPC Interface info for MOM RPC  */
                                        /*  call-back to PEI entry pts     */

      int  authTrapFlag;                /* temp place for value of the     */
                                        /* DEC-only "authtrap" tconfig     */
                                        /* file directive; 1 = enabled;    */
                                        /* 2=disabled; 0 = not in tconfig. */

      int    snmp_traps_disabled;       /* 1=SNMP traps are disabled       */

      evd_queue_handle  
            *event_queue_handle;        /* Event Queue Handle ptr for MOMs */

      int    event_uid;                 /* Holds latest atomic event_uid   */

      } big_picture_t;

/*
|
|   Define Prototypes for SNMP PE Functions
|
*/


/* --------------
|   snmppe_main.c
*/
/* pe_log - SNMP Protocol Engine All-Purpose Logging Routine */
void
pe_log PROTOTYPE((
log_state_t     *,              /*-> info needed to actually log the msg   */
int             ,               /* Indicates which class "msg" belongs to  */
char            *,              /* The message to be written to log        */
int                             /* The code needed for syslog call         */
));

/* crash_exit - SNMP Protocol Engine Crash-Shutdown Routine */
void
crash_exit PROTOTYPE((
big_picture_t   *,              /*-> The Big Picture                       */
char            *               /* The error msg to be written to syslog   */
));

/* log_service - Log an interpretation of a Service Block to Log File */
void
log_service PROTOTYPE((
big_picture_t   *,          /*-> The Big Picture                       */
int             ,           /* Indicates class dump "msg" belongs to   */
service_t       *           /*-> Service Block to be dumped            */
));

/* dump_instance_list - Release all storage for an Instance Info List */
void
dump_instance_list PROTOTYPE((
snmp_instance_t     **          /* ->> Instance List to be released */
));

/* build_line_prefix - Builds ASCII prefix into Output Line Buffer */
void
build_line_prefix PROTOTYPE((
log_state_t     *,              /*-> log info needed for logging class      */
int             ,               /* Bit Pattern flag for message class       */
char            *,              /* Output Line buffer for prefix            */
int             *               /* Start String position (index into dmpbuf */
));

/* pe_oid_text - Convert binary OID to text string */
char *
pe_oid_text PROTOTYPE((
object_id       *   /*--> The Object Identifier to be printed */
));

/* Gets the value of a man_status */
char *
get_man_status PROTOTYPE((
int  
));

/* Prints the contents of an AVL */
void
snmppe_print_avl PROTOTYPE(( 
avl *,
FILE *,                 /* Output File                       */
char  *,                /* Error Message build buffer        */
int                     /* Error Msg "String Start" in buf[] */
));


/* --------------
|   snmppe_init.c
*/
/* init_main - Initialization Function for SNMP Protocol Engine */
void
init_main PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure to be initialized     */
int             ,       /* Count of inbound command line arguments       */
char *[]                /* Array of pointers to strings of cmd line args */
));


/* ---------------
|   snmppe_netio.c
*/
/* netio_get_pdu - Get Inbound PDU */
void
netio_get_pdu PROTOTYPE((
big_picture_t    *,     /*-> Big Picture Structure for SNMP PE                */
service_t        *,     /*-> Service Blk (on Service List) to be receive pdu  */
MCC_T_Descriptor *      /*-> Descriptor for buffer to receive the PDU image   */
));

/* netio_deserialize_pdu - Deserialize ASN.1-encoded PDU */
BOOL
netio_deserialize_pdu PROTOTYPE((
big_picture_t    *,     /*-> Big Picture Structure for SNMP PE              */
service_t        *,     /*-> Service Blk (on Service List) to rcv PDU info  */
MCC_T_Descriptor *      /*-> Descriptor for buffer to holding the PDU image */
));

/* netio_put_pdu - Put Outbound PDU */
void
netio_put_pdu PROTOTYPE((
big_picture_t    *,    /*-> Big Picture Structure for SNMP PE                */
service_t        *,    /*-> Service Blk (on Service List) to be receive pdu  */
MCC_T_Descriptor *     /*-> Descriptor for buffer to receive the PDU image   */
));

/* netio_serialize_pdu - Serialize Service Block info into ASN.1-encoded PDU */
BOOL
netio_serialize_pdu PROTOTYPE((
big_picture_t    *,         /*-> Big Picture Structure for SNMP PE           */
service_t        *,         /*-> Service Blk containing PDU info             */
MCC_T_Descriptor *,         /*-> Descriptor for buf to rcv encoded PDU image */
BOOL             *          /*-> FALSE on return if PDU was too big          */
));


/* netio_send_trap - Issue a Trap to all proper trap community managers */
void
netio_send_trap PROTOTYPE((
big_picture_t    *,         /*-> Big Picture Structure for SNMP PE           */
object_id        *,         /*-> Object id for enterprise sending the trap   */
MCC_T_Descriptor *,         /*-> Descriptor for agent-addr                   */
trap_pdu_type     ,         /* Type of trap to send                          */
int               ,         /* Specific trap id for enterprise specific trap */
unsigned int     *,         /*-> Time-stamp of trap                          */
MCC_T_Descriptor *          /*-> Descriptor for varbind list                 */
));

/* ----------------
|   snmppe_casend.c
*/
/* casend_get_msg - Get Inbound PDU deserialized into Service Block */
void
casend_get_msg PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       **      /*->> Service Blk (on Service List) to be processed  */
));

/* casend_process_msg - Process the received msg into CA requests */
void
casend_process_msg PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure for SNMP PE               */
service_t       *       /*-> Service Block (on Service List) to be processed */
));


/* -------------
|   snmppe_mir.c
*/
/* mir_class_inst_GETSET - Find SNMP Obj Class & Instance Info for GET/SET */
mir_derive_CI_status
mir_class_inst_GETSET PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
varbind_t       *               /*-> Varbind Entry Block for SNMP OID        */
));

/* mir_class_inst_GETNXT - Find SNMP Obj Class & Instance Info for GET-NEXT */
mir_derive_CI_status
mir_class_inst_GETNXT PROTOTYPE((
big_picture_t   *,              /*-> Big Picture Structure for SNMP PE       */
varbind_t       *               /*-> Varbind Entry Block for SNMP OID        */
));

/* mir_class_inst_ROLLNXT - Roll to get next SNMP Obj Class & Instance Info */
mir_derive_CI_status
mir_class_inst_ROLLNXT PROTOTYPE((
big_picture_t   *,         /*-> Big Picture Structure for SNMP PE            */
e_status_t       ,         /* "signalled" status for msi_invoke_action req   */
varbind_t       *          /*-> Varbind Entry Block for SNMP OID             */
));


#ifdef NL           /* Only include in IPC images (where NL is included) */
#ifndef MOM_HEADER  /* Don't include in single image; avoids link errors */
/* -------------------------------------------------------------------------
|  snmppe_text.c - Requires externs of all functions that print translatable
|                  text.  No prototypes needed.  Add/remove externs here as new
|                  messages are created/old messages are deleted.  All messages
|                  are kept in SNMPPE_TEXT.C, and are translated by 'make'.
*/
extern char *msg001();
extern char *msg002();
extern char *msg003();
extern char *msg004();
extern char *msg005();
extern char *msg006();
extern char *msg007();
extern char *msg008();
extern char *msg009();
extern char *msg010();
extern char *msg011();
extern char *msg012();
extern char *msg013();
extern char *msg014();
extern char *msg015();
extern char *msg016();
extern char *msg017();
extern char *msg018();
extern char *msg019();
extern char *msg020();
extern char *msg021();
extern char *msg022();
extern char *msg023();
extern char *msg024();
extern char *msg025();
extern char *msg026();
extern char *msg027();
extern char *msg028();
extern char *msg029();
extern char *msg030();
extern char *msg031();
extern char *msg032();
extern char *msg033();
extern char *msg034();
extern char *msg035();
extern char *msg036();
extern char *msg037();
extern char *msg038();
extern char *msg039();
extern char *msg040();
extern char *msg041();
extern char *msg042();
extern char *msg045();
extern char *msg046();
extern char *msg047();
extern char *msg048();
extern char *msg049();
extern char *msg050();
extern char *msg051();
extern char *msg052();
extern char *msg053();
extern char *msg054();
extern char *msg055();
extern char *msg056();
extern char *msg057();
extern char *msg058();
extern char *msg059();
extern char *msg060();
extern char *msg061();
extern char *msg062();
extern char *msg063();
extern char *msg064();
extern char *msg065();
extern char *msg066();
extern char *msg067();
extern char *msg068();
extern char *msg069();
extern char *msg070();
extern char *msg071();
extern char *msg072();
extern char *msg073();
extern char *msg074();
extern char *msg078();
extern char *msg079();
extern char *msg080();
extern char *msg081();
extern char *msg082();
extern char *msg083();
extern char *msg084();
extern char *msg085();
extern char *msg086();
extern char *msg087();
extern char *msg088();
extern char *msg089();
extern char *msg090();
extern char *msg091();
extern char *msg092();
extern char *msg093();
extern char *msg094();
extern char *msg095();
extern char *msg096();
extern char *msg097();
extern char *msg098();
extern char *msg099();
extern char *msg100();
extern char *msg101();
extern char *msg102();
extern char *msg103();
extern char *msg104();
extern char *msg105();
extern char *msg106();
extern char *msg107();
extern char *msg108();
extern char *msg109();
extern char *msg110();
extern char *msg111();
extern char *msg112();
extern char *msg113();
extern char *msg114();
extern char *msg115();
extern char *msg116();
extern char *msg117();
extern char *msg118();
extern char *msg119();
extern char *msg120();
extern char *msg121();
extern char *msg122();
extern char *msg123();
extern char *msg124();
extern char *msg125();
extern char *msg126();
extern char *msg127();
extern char *msg128();
extern char *msg129();
extern char *msg130();
extern char *msg132();
extern char *msg133();
extern char *msg135();
extern char *msg136();
extern char *msg137();
extern char *msg138();
extern char *msg139();
extern char *msg140();
extern char *msg141();
extern char *msg142();
extern char *msg143();
extern char *msg144();
extern char *msg145();
extern char *msg146();
extern char *msg147();
extern char *msg148();
extern char *msg149();
extern char *msg150();
extern char *msg151();
extern char *msg152();
extern char *msg153();
extern char *msg154();
extern char *msg155();
extern char *msg156();
extern char *msg157();
extern char *msg158();
extern char *msg159();
extern char *msg160();
extern char *msg161();
extern char *msg162();
extern char *msg163();
extern char *msg164();
extern char *msg165();
extern char *msg166();
extern char *msg168();
extern char *msg169();
extern char *msg170();
extern char *msg171();
extern char *msg172();
extern char *msg173();
extern char *msg174();
extern char *msg175();
extern char *msg176();
extern char *msg177();
extern char *msg178();
extern char *msg179();
extern char *msg180();
extern char *msg181();
extern char *msg182();
extern char *msg183();
extern char *msg184();
extern char *msg185();
extern char *msg186();
extern char *msg187();
extern char *msg188();
extern char *msg190();
extern char *msg191();
extern char *msg192();
extern char *msg193();
extern char *msg194();
extern char *msg195();
extern char *msg196();
extern char *msg197();
extern char *msg198();
extern char *msg199();
extern char *msg200();
extern char *msg201();
extern char *msg202();
extern char *msg203();
extern char *msg204();
extern char *msg205();
extern char *msg206();
extern char *msg207();
extern char *msg208();
extern char *msg209();
extern char *msg210();
extern char *msg211();
extern char *msg212();
extern char *msg213();
extern char *msg214();
extern char *msg215();
extern char *msg216();
extern char *msg217();
extern char *msg218();
extern char *msg219();
extern char *msg220();
extern char *msg221();
extern char *msg222();
extern char *msg223();
extern char *msg224();
extern char *msg225();
extern char *msg226();
extern char *msg227();
extern char *msg228();
extern char *msg229();
extern char *msg230();
extern char *msg231();
extern char *msg232();
extern char *msg233();
extern char *msg234();
extern char *msg235();
extern char *msg236();
extern char *msg237();
extern char *msg238();
extern char *msg239();
extern char *msg240();
extern char *msg241();
extern char *msg242();
extern char *msg243();
extern char *msg244();
extern char *msg245();
extern char *msg246();
extern char *msg247();
extern char *msg248();
extern char *msg249();
extern char *msg250();
extern char *msg251();
extern char *msg252();
extern char *msg253();
extern char *msg254();
extern char *msg255();
extern char *msg256();
extern char *msg257();
extern char *msg258();
extern char *msg259();
extern char *msg260();
extern char *msg261();
extern char *msg262();
extern char *msg263();
extern char *msg264();
extern char *msg265();
extern char *msg266();
extern char *msg267();
extern char *msg268();
extern char *msg269();
extern char *msg270();
extern char *msg271();
extern char *msg272();
extern char *msg273();
extern char *msg274();
extern char *msg275();
extern char *msg276();
extern char *msg277();
extern char *msg278();
extern char *msg279();
extern char *msg280();
extern char *msg281();
extern char *msg282();
extern char *msg283();
extern char *msg284();
extern char *msg285();
extern char *msg286();
extern char *msg287();
extern char *msg288();
extern char *msg289();
extern char *msg290();
extern char *msg291();
extern char *msg292();
extern char *msg293();
extern char *msg294();
extern char *msg295();
extern char *msg296();
extern char *msg297();
extern char *msg298();
extern char *msg299();
extern char *msg300();
extern char *msg301();
extern char *msg302();
extern char *msg303();
extern char *msg304();
extern char *msg305();
extern char *msg306();
extern char *msg307();
extern char *msg308();
extern char *msg309();
extern char *msg310();
extern char *msg311();
extern char *msg312();
extern char *msg313();
extern char *msg314();
extern char *msg315();
extern char *msg316();
extern char *msg317();
extern char *msg318();
extern char *msg319();
extern char *msg320();
extern char *msg321();
extern char *msg322();
extern char *msg323();
extern char *msg324();
extern char *msg325();
extern char *msg326();
extern char *msg327();
extern char *msg328();
extern char *msg329();
extern char *msg330();

#endif /* MOM_HEADER */
#endif /* NL */
