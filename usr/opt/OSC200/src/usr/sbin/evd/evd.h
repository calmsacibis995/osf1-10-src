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
 * @(#)$RCSfile: evd.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:30:37 $
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
 * Module EVD.H
 *      Contains data structure definitions required by the Common Agent
 *      Event Logger/Dispatcher process.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture Engineering
 *    D. McKenzie   February 1993
 *
 * BIRD'S EYE VIEW:
 *    Purpose:
 *       This module contains the internal data structure definitions
 *       required by the Common Agent Event Logger/Dispatcher code modules.
 *
 * History
 *      V1.1    Feb 1993     D. McKenzie - Original version (no V1.0 version).
 */

#ifndef EVD_H_ALREADY_INCLUDED
#define EVD_H_ALREADY_INCLUDED

/*
|==============================================================================
|   MAJOR, MINOR
|
|   These symbols are used to form the "Vx.y" Version Identifier in the
|   initialization message logged by EVD into the system log when it
|   starts up.  MAJOR is "x" and MINOR is "y".  (Done in "evd_init_main()").
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

#if defined(mips) || defined(__mips)
#  define PROTOTYPE_ALLOWED
#endif

#ifdef PROTOTYPE_ALLOWED
#  define PROTOTYPE(args) args
#else
#  define PROTOTYPE(args) ()
#endif


/* Module Overview: 
|
|   This file contains definitions for the C-structures and datatypes used
|   internally by the Common Agent Event Logger/Dispatcher process.
|
|   We need to include the definitions used by the "users" of EVD, namely 
|   "evd_defs.h".
|
*/

#include "evd_defs.h"


/* Define MACRO to be used for displaying translatable text */
#ifdef NL
# define MSG(msg_name, string)  msg_name()
#else
# define MSG(msg_name, string)  string
#endif /* NL */


/*
|==============================================================================
|   If we're doing IPC/RPC, implies we're using some form of a threads
|   package.  The EVD code is sensitive to the symbol NOIPC and also to the 
|   symbol THREADS, which we define here according to the value of NOIPC.
|
|   EVD code for IPC/RPC depends on "NOIPC", while the thread-dependent
|   code follows "THREADS".
|
|   To handle more than one inbound event request at once, MTHREADS should be 
|   set to the number of ADDITIONAL EVENT REQUESTS (beyond 1) that we desire 
|   to support handling AT ONCE.  (2 event requests at once requires MTHREAD=1;
|   see the documentation in module evd_main.c).
|
|==============================================================================
*/
#ifndef NOIPC
#define THREADS 0
/*** define MTHREADS 1   --define this after adding any necessary code ***/

extern void *cma_lib_free();     /* to resolve 'c89 -std' warnings */
extern void *cma_lib_malloc();   /* to resolve 'c89 -std' warnings */

#endif /* NOIPC */


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
|   evd_init_cmdline_args() in evd_init.c, and it should include a padding
|   space.  Current design minimum is 12 characters.
|
|==============================================================================
*/
#define PREFIXSIZE 12


/*
|==============================================================================
|   MAXERRMSG
|
|   This symbol is used to declare the size of the buffer that error messages
|   are built into by the evd_log() function.
|
|==============================================================================
*/
#define MAXERRMSG 300



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
|   error message via a call to function "evd_log()".
|
|   All macros use a convention that the function containing the invocation
|   must have in scope at the point of invocation a variable named "bp" that
|   points to the "big_picture" (i.e., evd_global_data) structure containing 
|   the "log_state" structure.
|
|==============================================================================
*/
#define LOG(class, msg)     evd_log (&bp->log_state, class, msg, 0)
#define SYSLOG(code, msg)   evd_log (&bp->log_state, L_SYSLOG, msg, code)
#define IFLOGGING(class)    if ( (bp->log_state.log_classes & class) )



/*
|==============================================================================
|  CRASH
|
|   This macro simplifies coding logic surrounding the issuing of an
|   error message and error exit via a call to function "evd_crash_exit()".
|
|   This macro usees a convention that the function containing the invocation
|   must have in scope at the point of invocation a variable named "bp" that
|   points to the "big_picture" (i.e., evd_global_data) structure.
|
|   This macro "wraps" all calls to "evd_crash_exit" so we can spiff up things
|   at a later date more easily if need be.
|
|==============================================================================
*/
#define CRASH(msg)   evd_crash_exit (bp, msg)

/*
|==============================================================================
|  Logging/Debugging Selection
|
|   This data structure contains all the information required for the
|   "evd_log()" EVD function.
|
|   Any function that needs to log something (for normal operation
|   or debugging operation) must pass the address of this structure to 
|   "evd_log()".
|
|   Bit flags define Message Classes. If the bit is SET in "log_classes" below
|   at runtime, then messages in that class will be logged.
|==============================================================================
*/

/* Basic Classes */
#define L_SYSLOG        0x00001  /* Log to SYSLOG                        */
#define L_TRACE         0x00002  /* Log TRACE messages of EVD processing */
/* -- Add new individual classes here, bump LOG_CLASS_COUNT below --*/


#define LOG_CLASS_COUNT 2

/* NOTE: Change "LOG_CLASS_COUNT" if you add to this list.  Then         */
/*       add code in "evd_init.c" routine "evd_init_cmdline_args()" to   */
/*       support parsing from the command line.                          */

typedef struct evd_log_state 
{
    int      debugging_mode;             /* TRUE: Debugging, no syslog() */
    int      log_classes;                /* Bit flags defined below      */
    char    *log_file_name;              /* NULL: use stderr             */
    FILE    *log_file;                   /* Debug output goes here       */
    char    *dbg_name[LOG_CLASS_COUNT];  /* Debugging Class Names        */
    int      dbg_flag[LOG_CLASS_COUNT];  /* Corresponding Bit Flags      */

#ifdef MTHREADS
    pthread_mutex_t log_file_m;          /* Protects "log_file"          */
#endif

} evd_log_state_t;



/*
|==============================================================================
|  Trap PDU Type:
|
|   The kinds of Specific Traps we have to handle.
|
|   NOTE WELL:  These values are taken from a recommended RFC. Don't change.
|
|==============================================================================
*/

typedef enum 
{
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
|  Event Queue Handle Block
|
|   This data structure contains all the information required to describe
|   the context associated between EVD and the MOM that has requested
|   this association with EVD.
|
|   One of these blocks is present for each queue handle requested by a
|   MOM. 
|
|   These blocks are strung off the "queue_handle_list" in the Big Picture
|   structure.
|
|==============================================================================
*/

typedef enum 
{
    INVALID_EVD_HANDLE = 0, 
    VALID_EVD_HANDLE   = 0x0F00BEEF

} evd_handle_check_t;


typedef struct event_queue_handle
{
   struct event_queue_handle *next;        /* Pointer to next queue handle    */
                                           /* block in the queue_handle_list  */
   evd_handle_check_t        check_value;  /* Set to VALID_EVD_HANDLE if this */
                                           /*   handle is still valid/active  */
   evd_queue_access_mode     access_mode;  /* Access mode of the handle       */
   evd_queue_handle          *self_handleP;/* Queue handle of this queue      */
                                           /* handle block                    */
#ifdef MTHREADS
   pthread_mutex_t           lock_m;       /* lock on access to this data     */
#endif

} event_queue_handle_t;


/*
|==============================================================================
|  EVD Global Data ("Big Picture") Structure
|
|   This data structure contains all the global data items
|   that modules in the EVD may reference.  These data items are
|   purposefully collected into this data structure so as to avoid the use
|   of ANY globally referenced data in EVD.  This makes reading
|   the code much easier.  Additionally, debugging is easier.
|
|   Most of the items in this structure are set once at initialization time
|   and are then "read-only" by the threads that comprise the EVD
|   execution context.
|
|   Those items which are subject to change by more than just one thread
|   are protected by a mutex that is also part of this structure.
|
|   Special note on the mutex protecting "queue_handle_list":
|       Acquisition of the associated mutex for this list is required to:
|               * Add another service block to the list (by allocation and
|                 change of the "next" cell of the last block on the list)
|               * Change the status of any service block on the list
|                 to "ACTIVE" (by setting the value of field "svc_active"
|                 to "TRUE").  <<<<**** NOT DONE FOR EVD V1.1 ****>>>>
|
|       Acquisition of the mutex is required because more than one thread
|       may be attempting to do the same thing (get a block).
|
|       Acquisition of the mutex is NOT REQUIRED to change the status of
|       a currently active block to INACTIVE (svc_active = FALSE) because
|       only the "owning thread" will attempt to do this, and it is an
|       atomic operation. (????)
|
|==============================================================================
*/

typedef struct evd_global_data 
{
    evd_log_state_t      log_state;          /* Current Logging state info  */
    event_queue_handle_t *queue_handle_list; /* List of Inactive/Active     */
                                             /* event queue handle blocks   */
    char                 *config_file_name;  /* Name of Config File used    */
    int                  snmp_trap_listeners;/* Count of SNMP trap listeners*/
    int                  snmp_traps_disabled;/* TRUE: SNMP traps disabled   */
    evd_queue_handle    *snmp_queue_handle;  /* Queue Handle into SNMP PE   */
    int                  event_uid;          /* For generating Event UIDs   */

/*
#ifdef THREADS
    pthread_t            recv_thread;        (* Receiving Thread            *)
#endif
*/
    management_handle    rpc_callback;       /* RPC Interface info for EVD  */
                                             /*  entry points for MOMs      */

#ifdef MTHREADS
    pthread_mutex_t      event_uid_m;        /* lock on access to event UIDs*/
    pthread_mutex_t      queue_handle_list_m;/* lock on access to event     */
                                             /*  queue handle list          */
#endif

} evd_global_data_t;


/*
|
|   Define Prototypes for the EVD functions globally accessible to EVD.
|
*/


/*----------------------------------------------------------------------------
|   evd_main.c
*/

/* evd_log - Event Dispatcher All-Purpose Logging Routine */
void
evd_log PROTOTYPE((
 evd_log_state_t *,              /*-> info needed to actually log the msg    */
 int             ,               /* Indicates which class "msg" belongs to   */
 char            *,              /* The message to be written to log         */
 int                             /* The code needed for syslog call          */
));


/* evd_crash_exit - Event Dispatcher Crash-Shutdown Routine */
void
evd_crash_exit PROTOTYPE((
 evd_global_data_t *,            /*-> The Big Picture                        */
 char              *             /* The error msg to be written to syslog    */
));


/* evd_log_queue_handle - Log interpretation of an Event Queue Handle Block to Log File */
void
evd_log_queue_handle PROTOTYPE((
 evd_global_data_t    *,         /*-> The Big Picture                        */
 int                  ,          /* Indicates class dump "msg" belongs to    */
 event_queue_handle_t *          /*-> Event Queue Handle Block to be dumped  */
));


/* evd_build_line_prefix - Builds ASCII prefix into Output Line Buffer */
void
evd_build_line_prefix PROTOTYPE((
 evd_log_state_t *,              /*-> log info needed for logging class      */
 int             ,               /* Bit Pattern flag for message class       */
 char            *,              /* Output Line buffer for prefix            */
 int             *               /* Start String position (index into dmpbuf)*/
));


/* evd_oid_text - Convert binary OID to text string */
char *
evd_oid_text PROTOTYPE((
 object_id       *               /*--> The Object Identifier to be printed   */
));


/* evd_get_man_status - Get the value of a man_status */
char *
evd_get_man_status PROTOTYPE((
 int  
));


/* evd_print_avl - Prints the contents of an AVL */
void
evd_print_avl PROTOTYPE((
 avl   *,
 FILE  *,                        /* Output File                         */
 char  *,                        /* Error Message build buffer          */
 int                             /* Error Msg "String Start" in buf[]   */
));



/*----------------------------------------------------------------------------
|   evd_init.c  
*/

/* evd_init_main - Initialization Function for the Event Dispatcher */
void
evd_init_main PROTOTYPE((
 evd_global_data_t *,    /*-> Big Picture Structure to be initialized     */
 int               ,     /* Count of inbound command line arguments       */
 char *[]                /* Array of pointers to strings of cmd line args */
));

/* evd_init_start_rpc - Start the IPC/RPC server interface */
void
evd_init_start_rpc PROTOTYPE((
 evd_global_data_t   *  /*-> Big Picture Structure to be initialized */
));


#ifdef NL           /* Only include in IPC images (where NL is included) */
/*--------------------------------------------------------------------------
|  evd_text.c - Requires externs of all functions that print translatable
|               text.  No prototypes needed.  Add/remove externs here as new
|               messages are created/old messages are deleted.  All messages
|               are kept in EVD_TEXT.C, and are translated by 'make'.
*/
extern char *emsg001();
extern char *emsg002();
extern char *emsg003();
extern char *emsg004();
extern char *emsg005();
extern char *emsg006();
extern char *emsg007();
extern char *emsg008();
extern char *emsg009();
extern char *emsg010();
extern char *emsg011();
extern char *emsg012();
extern char *emsg013();
extern char *emsg014();
extern char *emsg015();
extern char *emsg016();
extern char *emsg017();
extern char *emsg018();
extern char *emsg019();
extern char *emsg020();
extern char *emsg021();
extern char *emsg022();
extern char *emsg023();
extern char *emsg025();
extern char *emsg026();
extern char *emsg027();
extern char *emsg028();
extern char *emsg029();
extern char *emsg030();
extern char *emsg031();
extern char *emsg032();
extern char *emsg033();
extern char *emsg034();
extern char *emsg035();
extern char *emsg036();
extern char *emsg037();
extern char *emsg038();
extern char *emsg039();
extern char *emsg040();
extern char *emsg041();
extern char *emsg042();
extern char *emsg043();
extern char *emsg044();
extern char *emsg045();
extern char *emsg046();
extern char *emsg047();

#endif /* NL */


#endif /* EVD_H_ALREADY_INCLUDED */
