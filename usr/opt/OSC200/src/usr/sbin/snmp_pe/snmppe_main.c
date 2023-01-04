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
static char *rcsid = "@(#)$RCSfile: snmppe_main.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/08 16:14:02 $";
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
 * Module SNMPPE_MAIN.C
 *      Contains main functions for the SNMP Protocol Engine for the
 *      Common Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   June 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accepts requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.
 *
 *    Purpose:
 *       This module contains the main function for the protocol engine
 *       that is designed to handle requests to the Common Agent using the
 *       Simple Network Management Protocol (SNMP).
 *
 * History
 *      V1.0    June 1991    D. D. Burns
 *      V1.1    Aug 1992     D. D. Burns
 *                            Correct erroneous debugging dump that was
 *                            trapping... (dumps of OCTET_STRINGS et.al).
 *
 * NOTES:

Module Overview:
---------------

This module contains the main function for the SNMP Protocol Engine (SNMP PE)
and a couple of support functions useful throughout the SNMP PE.

File SNMPPE_TEXT.C contains all printable text output for the 'Mxxx' messages
used throughout the protocol engine for diagnostic and debugging use.  If you
modify the text of any message, or add new messages, or delete old messages,
you must:

   (a) Add the new message/modify the existing message/delete the old message
       in/in/from SNMPPE_TEXT.C.  Be sure to follow the format used in
       that module when creating a new text message 'function'.

   (b) Use the 'MSG' macro defined in SNMPPE.H; this macro invokes the
       appropriate msgXXX() function which you added to SNMPPE_TEXT.C
       in steb (a) above.  Be sure to add/delete the 'extern' for the 
       new/deleted message 'function' at the end of SNMPPE.H.  Perform
       this step ONLY if you are adding a new message, or if you are
       deleting an existing message.  Modifications to existing messages
       require changes to the appropriate function in SNMPPE_TEXT.C only.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------

main                    Main function for the SNMP PE, executed by the main
                        "sending" thread.

pe_log                  Handles logging of diagnoistic and normal operational
                        conditions to a debugging log file or the system log
                        (Used by both threads, log file is mutex protected).
                        (This function usually invoked by macro LOG() or
                        SYSLOG() defined in "snmppe.h", and is primarily
                        for one-line messages.

log_service             Handles diagnostic output to a debugging log file
                        (not the system log) of a dump of a specified
                        service block.

crash_exit              Handles the details of a crash-shutdown when an
                        unrecoverable error occurs.  (This function usually
                        invoked by macro "CRASH()" defined in "snmppe.h").

dump_instance_list      Releases all the storage in a varbind-entry block
                        "instance information" list.

build_line_prefix       Builds the proper ASCII prefix into a output line
                        buffer to be used in a diagnostic dump.

DEBUG FUNCTIONS:
codecenter_print_oid    Special print function designed exclusively for use
                        with the "CodeCenter" C-tool.  It prints an OID thru
                        the tool when called by the tool.

MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------
snmppe_print_avl        Interprets an AVL for logging
print_constructed_avl   Interprets a constructed AVL for logging
print_value             Interprets octet string value for logging
get_man_status          Interprets status for logging
get_type                Interprets AVL tag type for logging



Files containing modules specific to the SNMP PE begin with the letters
"snmppe" in the general form "snmppe_xxxxx.c", where "xxxxx" is a descriptive
hint as to what is inside.

All functions within a file begin with the letters "xxxxx_" (the only
exceptions being the functions within this main source module).

The other code module files that together with this one comprise the SNMP PE
are:

File                    Synopsis
-------------           --------
snmppe_init.c           Contains functions that perform one-time initialization
                        chores in anticipation of SNMP PE startup.

snmppe_carecv.c         Contain functions that handle receiving the replies
                        from the Common Agent and recording them so that
                        a response can be sent to the SNMP manager.

snmppe_casend.c         Contain functions that handle sending the requests
                        to the Common Agent, and responses to the SNMP manager.

snmppe_mir.c            Contains Tier 1 and Tier 2 MIR lookup functions tha
                        directly support SNMP PE.

snmppe_netio.c          Contains functions that perform ASN 1 "serialization"
                        and "deserialization" of SNMP PDUs "to" and "from" the
                        data structures used in the SNMP PE on behalf of
                        functions in "snmppe_ca_recv.c" and "snmppe_ca_send.c".

                        Actual Network I/O (send and receive) is done by
                        functions in this module.

Supporting code from other sources (in some cases modified for use with
SNMP PE) includes:

File                    Synopsis
-------------           --------
snmp_lib.c              Contains functions that build or parse SNMP messages
                        using functions in MCC_ASN1.C (et. al.) on behalf of
                        functions in "snmppe_netio.c".

mcc_asn1.c              -Main Module of ASN1 support function from DECmcc.
                        Contains functions that perform the nitty-gritty of
                        building/parsing ASN.1 byte streams.
mcc_asn1_extra.c        -TLV support routines
mcc_asn1_objid.c        -Object ID support routines
mcc_asn1_dump.c         -Interpreted-Dump function

Thread Overview:
---------------

SNMP PE can be linked for use with RPC (and implicitly the DCE threads package)
or it can be linked as a single image with the Common Agent.

The symbol "NOIPC" controls the conditional compilation as required.

==============================================================================
---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---
==============================================================================
When "NOIPC" is NOT defined (ie we're using RPC and DCE Threads), SNMP PE V1.0
operates as a client (of MOLD and the MOMS) and as a server to the MOMs (by
offering it's "pei_*" entry-points as an interface).  In this situation,
SNMP PE spawns (during initialization) a second thread which performs the
"rpc_listen" function to allow SNMP PE to act as a server.  This is described
as the "receiving" thread below, while the main "sending" thread then goes
on to wait for an inbound PDU.

Even though there are two threads running, in this mode SNMP PE V1.0 is still
logically "singly-threaded" insofar is shared-data structures is concerned.
This is because the main "sending" thread will always be blocked (doing an
RPC-supported msi_* call) while the secondary "receiving" thread is running.
Similarly the receiving thread is blocked waiting to support an RPC-initiated
call to a pei_* entry point when the sending thread is not doing an msi_* call.
Consequently there can be no contention for shared data areas and no mutexes
are required or employed.  In this mode SNMP PE V1.0 handles exactly one
inbound SNMP PDU request at a time, sending any response back to the caller
before waiting to accept another.

Compilation of all thread-package calls used to set up this "two-threaded,
logically-singly-threaded" arrangement is controlled by the symbol "THREADS"
which in turn is set according to "NOIPC" in header "snmppe.h".

Some additional conditionally compiled code is included in the source that
can support true "multiple-threaded" operation.

(Note that more code is required to be added before true "multiple-threaded"
operation can actually be supported.  Search for the symbol "MTHREADS" to
find the places where more code should be added.  Design work should be done
to review C-library functions which may not be thread-safe, including in
at least "inet_ntoa()" which seems to use a static area).

This additional code is activated by defining symbol "MTHREADS".  The value
of this symbol is the number of EXTRA "sending-receiving" thread PAIRs that
may be started to handle additional inbound PDUs simultaneously.  In other
words, if MTHREADS is 1, then SNMP PE can handle 2 inbound PDUs
simultaneously, and four threads may be active simultaneously, paired
"sending-receiving" for each PDU request active. If MTHREADS is 2, then SNMP
PE can handle 3 inbound PDUs simultaneously using six threads (the basic 2 as
a pair, plus an additional four working in pairs as "sending" and "receiving").
The value of MTHREADS can be thought of as the number of PDUs beyond the
initial PDU that can be handled simultaneously.

If "MTHREADS" is not defined and "THREADS" is defined, then only the basic
"logically-single-threaded" operation (one PDU at a time) is compiled.
(The major difference that occurs when MTHREADS is defined and given a value
 in this circumstance is that mutexes are defined and code that initializes
 them and references them comes into play).
========FOR V1.0, THIS IS THE ONLY CONFIGURATION THAT HAS BEEN TESTED!======

In the thread discussion throughout the sources, only one "sending" and one
"receiving" thread is discussed, even though there may be multiple pairs
operating at once if this MTHREAD code is ever checked out properly.
==============================================================================
---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---SNMP PE V1.0---
==============================================================================

The main thread is thought of as the "sending" thread, as it receives
inbound SNMP messages and makes RPC calls to "send" the information on to
the Common Agent.  It spawns the second thread at initialization time.

The second thread is thought of as the "receiving" thread, as it receives
the Common Agent replies in the form of an RPC invocation of a function in
"snmppe_carecv.c".

The functions executed *solely* by the second "receiving" thread are contained
entirely in file "snmppe_carecv.c".  (Other functions such as pe_log() in this
module may be called by either thread, but never simultaneously.

Synchronization is never required between the sending and receiving thread
of a single pair, as they are never active simultaneously.  Mutex protection
is required in several places two arbitrate between groups of sending threads
or groups of receiving threads that may be attempting access to a shared
resource. (This paragraph true only if MTHREADS is defined).

When synchronization is required between the threads for access to shared
data-structures or functions, a thread "mutex" is used.  Aside from this
synchronization, the threads operate independently but synchronously:
    - the sending thread sends a common agent request for each varbind entry
    - the receiving thread receives each common agent reply, looks up the
      queue entry corresponding to the varbind entry and stores status into it
    - the sending thread, after sending all requests and all replies have
      been received, then
    - constructs an SNMP PDU containing the reply to be sent to the
      director (SNMP manager), and sends it.

Only the sending thread makes use of the functions in "snmppe_netio.c" to
receive and send ASN.1-encoded SNMP PDUs.  The receiving thread merely
sets flags and status codes that are examined by the sending thread
(repeatedly, until all Common Agent requests spawned by a PDU have been
processed).

Comments in the code that address thread issues are prefaced with the string
"<TC>" for "Thread Comment" (for ease of grepping).

Coding Conventions:
-------------------

* All functions are named according to the source module they are found in
  (as described under "Module Overview").

* All functions have prototypes.  Static functions have prototypes
  coded into the front of the module in which the function is defined.
  Externally referenced functions have prototypes in "snmppe.h".

* Comments in the code that address thread issues are prefaced with the string
  "<TC>" for "Thread Comment" (for ease of grepping).

* Error, Debug and Informational messages formatted according to the
  convention outlined in the documentation for function "pe_log()" found
  in this module.

* MINIMIZE STATIC DATA! (If you want to use static data, be VERY sparing,
                         and VERY careful, we didn't dump FORTRAN for nothing)

* NO (static) GLOBALLY ACCESSIBLE DATA!  All data of this ilk goes into
  the "Big Picture" structure which is passed explicitly to any function
  requiring it.  This permits re-entrancy, recursion (and thread stuff)
  but above all it permits ease of maintenance. (No grepping to find
  where something gets defined/allocated).

* You change it, you document it, or I rear up in your dreams and 
  smartly smite you for making MY documentation invalid.

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

#include <ctype.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <arpa/inet.h>

/* includes required for "snmppe.h" */
#include "port_cmip.h"
#include <stdio.h>
#include "moss.h"
#include <netinet/in.h>
#define OID_DEF
#include "snmppe.h"
#include "moss_inet.h"

#define IMOM_TRAP_POLLING_FREQUENCY 20 /* Poll MIPS Internet MOM every 20   */
                                       /* times thru main loop (NOIPC only) */


/*---------------------- For Internationalization ---------------------------*/
/* The following includes are required in the main module of a program ONLY. */

#ifdef NL
#include <langinfo.h>
#include <locale.h>
#include <nl_types.h>
#endif

/*---------------------------------------------------------------------------*/

/*
|==============================================================================
|
|   Define Prototypes for Module-Local Functions
|
|==============================================================================
*/

/* Prints the contents of a constructed AVL */
static int
print_constructed_avl PROTOTYPE((
avl *,
FILE *,
char  *,                /* Error Message build buffer        */
int                     /* Error Msg "String Start" in buf[] */
));

/* Prints the value of an octet_string */
static void
print_value PROTOTYPE((
int          ,
octet_string *,
FILE *,                 /* Output File                       */
char  *,                /* Error Message build buffer        */
int                     /* Error Msg "String Start" in buf[] */
));

/* Gets the value of a ASN1 Tag */
static char *
get_type PROTOTYPE((
unsigned int
));

/*
|==============================================================================
| Big Picture Pointer
|
|   This globally accessible pointer is the exception to the "no global data"
|   rule.  This pointer is loaded by the initilization code thereby making
|   the "big_picture" structure available to the receiving thread.
|
|   I don't know how to avoid using this when in a threaded environment.
|
|   If I did, I would.
|
|   Most of the stuff in this structure is "read-only", things that aren't are
|   protected by mutexes in the structure.
|
|==============================================================================
*/
big_picture_t           *big_picture;



/* main - Main Function for SNMP Protocol Engine */
/* main - Main Function for SNMP Protocol Engine */
/* main - Main Function for SNMP Protocol Engine */

#ifndef PE_EMBEDDED

void
main (argc, argv)              /* When the PE is its own image */
 int argc;
 char *argv[];

#else

void
snmppe_main_init (argc, argv)  /* When the PE is embedded into another image */
 int argc;
 char *argv[];

#endif


/*
INPUTS:

    "argc" and "argv" constitute the description of the command line
    used to start the SNMP PE.

    The optional command line arguments have the form:

    [-p <inbound port number to use>]  . . . .
      [-t <outbound port number to use>]  . . . . 
        [-c <configfilename>]               . . . . 
           [-d <debugfilename>] [<debuginfo1> [<debuginfo2> ...] ...]

    where:

    -p  optionally introduces a decimal port number to use instead of the
        normal port assigned to SNMP service.  Use of this option puts
        SNMP PE into "debugging operation".


    -t  optionally introduces a decimal port number to use instead of the
        normal port assigned to SNMP-TRAP service.  Use of this option puts
        SNMP PE into "debugging operation".


    -c  optionally introduces the name of the configuration file to use 
        instead of a builtin file name.  If debugging operation is requested
        by other options on the command line and this -c argument is not
        present, then SNMP PE attempts to look up the value of environment
        variable "SNMPPE_CONFIG_FILE" and uses that value (if present) as
        the name of the configuration file to be opened instead of the builtin
        name.

        If this option is absent and debugging operation is not requested by
        the use of other arguments, SNMP PE attempts to open the configuration
        file using the builtin name.  See "snmppe.h" for the value of this
        builtin name.


    -d  optionally introduces the name of a file to be used to receive
        error logging and debug information.  If the file exists, it is
        opened for appending, otherwise it is created.  Specifying this
        option constitutes specifying "debugging operation" as mentioned above
        in the description of "-c".

        If this option is not specified but other <debuginfo> options are
        specified on the command line, (signifying "debugging operation")
        then all debug/error information is routed to "stderr" (including
        error messages that otherwise would be routed to the system log).

        If neither this option nor any <debuginfo> class options are specified,
        then SNMP PE is in "normal operation" and error logging is done to
        the system log file.


    <debuginfo> optionally specifies a class of debugging information to
        be routed to the debug/error log file.  The names of these debugging
        information classes correspond to the values of the symbols defined
        near the definition of "log_state_t" in "snmppe.h" WITHOUT the leading
        "L_".  For instance, to enable the issuance of debugging statements
        that dump the inbound GET requests to SNMP PE, supply the string
        "PDUGETIN" on the command line (case insensitive).  Function
        init_cmdline_args() in snmppe_init.c parses & recognizes these
        debugging information message class names.

        Providing one or more of these debug-info class names puts SNMP PE into
        "debugging operation", causing the absence of the "-c" option to
        perform differently (see above) and causes all error logging output
        (that under normal operation is routed to the system log) to be logged
        to the debug/error output file ("stderr" or file specified by -d
        above).

        By providing just "SYSLOG" as a debugging class of information desired,
        SNMP PE operates in debugging mode producing exactly the same messages
        to the debug/error log file as would otherwise be produced to the
        system log file under normal operation.  (SYSLOG is always internally
        automatically enabled as a message class, specifying it explicitly
        simply causes debugging mode to be enabled).

        When SNMP PE is running in debug mode, sending a PDU "SET" request
        with community = "PESHUTDOWN" will cause a controlled crash to occur,
        closing the log file and shutting down operations in an orderly manner.
        Note that SNMP PE does not kickback a response PDU in this situation.

OUTPUTS:

    The function never returns.  A controlled crash may be initiated by
    a call to functions "crash_exit()" (usually via macro CRASH()).


BIRD'S EYE VIEW:
    Context:
        The caller is presumably a shell script that is starting up
        the Common Agent.

    Purpose:
        This function causes normal initialization to occur, creation of
        a receiving thread and then continuous processing of all received
        SNMP messages into requests to the Common Agent.


ACTION SYNOPSIS OR PSEUDOCODE:

     <load the big picture global reference pointer so receive thread can
      "see" the big picture>

     <If internationalizing, open the catalogue file & init the runtime locale>
     <perform all initialization>

     do forever
         <obtain next deserialized inbound message in a service block>
         <process the deserialized msg to generate CA calls>


OTHER THINGS TO KNOW:

    If we're running in a threaded/RPC environment, the initialization
    included starting the "receiving" thread that does an RPC listen for
    replies from the Common Agent.  See the <TC> below.
*/

{
big_picture_t           bp;     /* All the "global" SNMP PE-wide data     */
service_t               *svc;   /* ->Service block for just received PDU  */
int                     i;      /* loop counter for checking Imom traps   */

#ifdef NOIPC
# if defined(__mips) || defined (mips)
    extern void poll_for_link_traps();  /* See ULTRIX Inet MOM's MOM_MAIN.C */
    i = IMOM_TRAP_POLLING_FREQUENCY;    /* Check for link traps right away  */
# endif
#endif

#ifdef NL
extern nl_catd _m_catd;
/*
extern nl_catd catopen();
extern char    *setlocale();
*/
#endif


/* <TC> In function "main()". . .                                         */
/* <TC> load the big picture global reference pointer so receive thread   */
/* <TC> can "see" the big picture structure containing SNMP PE's context  */
big_picture = &bp;

#ifdef NL

/* If internationalizing, open the catalogue file & init the runtime locale */
_m_catd = catopen ("snmp_pe.cat", NL_CAT_LOCALE);
setlocale (LC_ALL, "");

#endif


/* perform all initialization
|
|    This consists of at least the following:
|      * Read in and verify command line arguments
|      * Open either debug or system log
|      * Open, parse and check the configuration file
|      * Set up shared memory statistics block w/MOM
|      * Set up the inbound network i/o port
|      * Set up the outbound network i/o port for traps
|      * Log snmp_pe as "up" in system log
|      * Send coldstart trap if needed
|      * Start receiving thread(s) if needed
|
|    The "big_picture" data structure is initialized and we're ready to roll
|    if we return.
*/
init_main( &bp, argc, argv);  /* If there is an error, we don't come back */

/* <TC> In function "main()". . .                                         */
/* <TC> The "sending" thread (the main thread) returns from init_main()   */
/* <TC> call above, and goes on to loop forever below waiting for and     */
/* <TC> processing into CA requests each inbound PDU received.            */

/* do forever */
for (;;) {

    /* -----------------------------------------------------------
    | obtain next deserialized inbound message in a service block
    |
    | (This consists of acquiring an available service block,
    |  wait for incoming PDU, deserialize the ASN1 info into
    |  service block using AVLs as needed.  Issue error logging
    |  messages and error traps as needed while discarding bad
    |  PDUs until we return successfully with a valid service
    |  block containing a syntactically correct request PDU).
    */
    casend_get_msg( &bp, &svc );


    /* MTHREADS
    |
    |  It is right here where we could spawn another thread and dispatch
    |  it on "casend_process_msg()" to handle the service block returned by
    |  the previous call.  This would allow SNMP PE to handle multiple PDU
    |  requests at once.  Currently we'll not accept another PDU until the
    |  one just received has been fully processed.
    |
    |  If this improvement is done, a new mutex ("in_m") must be established
    |  to protect the inbound socket.  netio_get_pdu() and netio_put_pdu() 
    |  must take turns acquiring the mutex and the associated socket.
    |
    |  Note that if this improvement is implemented, it should probably be
    |  controlled by a switch from the command line to allow for disabling it.
    |  By allowing multiple PDUs to be in-process at once, it'll produce
    |  confusing diagnostic logging output since the output doesn't indicate
    |  which thread is "speaking".  This could be alleviated by a change to
    |  function "build_line_prefix()" in this module to add an indicator to
    |  the prefix (which it adds to each diagnostic output line generated) to
    |  indicate which thread is generating that particular diagnostic output
    |  line.
    |
    |  Note that the logic placed here should check the number of threads
    |  currently active (probably by using variable incremented by each
    |  thread as it becomes active and decremented as it exits, protected
    |  by a condition variable & a mutex) againts the allowed limit given
    |  by the value of MTHREADS, and only start a thread if we're below
    |  the limit.
    */


    /* ------------------------------------------------------------
    | process the deserialized msg to generate CA calls
    |
    | (This consists of authenticating the PDU as coming from a
    |  valid community, looking up class info for all varbind
    |  entries, then building the appropiate request(s) to the
    |  Common Agent and issuing them all.  In the event of errors,
    |  an appropriate Get-Response PDU or trap is created and sent
    |  and error logging occurs internal to SNMP PE).
    */
    casend_process_msg( &bp, svc );


    /*
    | Upon return, either a semantically erroneous request has been error
    | handled or a valid request has generated Common Agent calls which
    | have all been made, and all replies have also been received.
    | The service block has been released for re-use.  Here, the value in
    | "svc" is of no interest (and probably points to an inactive block).
    */


#ifdef NOIPC
# if defined(__mips) || defined (mips)
    /* Every nth time thru the loop, check MIPS Imom for LINK-UP/DOWN traps */
    if (++i > IMOM_TRAP_POLLING_FREQUENCY)
    {
        i = 0;
        poll_for_link_traps();
    }
# endif
#endif

    } /* end forever loop */
}

/* pe_log - SNMP Protocol Engine All-Purpose Logging Routine */
/* pe_log - SNMP Protocol Engine All-Purpose Logging Routine */
/* pe_log - SNMP Protocol Engine All-Purpose Logging Routine */

void
pe_log( log_context, msg_class, msg, syslog_code)

log_state_t     *log_context;   /*-> info needed to actually log the msg   */
int             msg_class;      /* Indicates which class "msg" belongs to  */
char            *msg;           /* The message to be written to log        */
int             syslog_code;    /* The code needed for syslog call         */

/*
INPUTS:

    "log_context" - a structure that is actually part of the "big_picture"
    containing all the information function "log" needs to direct the message
    to the proper sink.

    "msg_class" - The set bit(s) in this integer indicate the debugging
    class-of-messages that the message belongs to.  "pe_log()" is going to
    check this category in "log_context" to be sure that logging of messages
    in this class is enabled before it actually writes the message to
    the log file.

    "msg" - The string to be placed into the log file (it should NOT have
    any format specifiers (like "%d") in it).  If it doesn't end in a
    newline, one is added.

    By coding convention it should start with the letters "Mxxx" where
    "xxx" is a unique decimal number.

    By grepping for "Mxxx" in the sources a person should be able to locate
    where the message is being issued from without cluttering the message
    with function names.

    "syslog_code" - The extra argument for the actual syslog system function
    call.  This code is not referenced unless the "msg" is in "msg_class"
    "L_SYSLOG" (and we're not in a debugging situation).


OUTPUTS:

    The function returns nothing, but baring unrecoverable errors, the
    specified message has been logged to the current log file if it is in
    a message class that is enabled for logging.

BIRD'S EYE VIEW:
    Context:
        The caller is any routine in SNMP PE (after snmppe_init() has
        been called) that wishes to log a debug or operational error message
        to the current log file.

    Purpose:
        This is the central function all other SNMP PE function call to record
        events that need logging (either debug or normal operation).


ACTION SYNOPSIS OR PSEUDOCODE:

    if (message class is enabled for logging)

if MTHREADS
        if (acquire mutex on log file pointer FAILED)
            <write message to syslog>
            <return>
endif

        if (it doesn't end in a newline and there is room)
            <add a newline>

        if (debugging mode enabled)
            <prepend the message class name>
            <write message to log_state's log file>
        else
            <write message to syslog()>

if MTHREADS
        if (release mutex on log file pointer FAILED)
            <write message to syslog>
            <return>
endif


OTHER THINGS TO KNOW:

    This function is not normally invoked using a direct call to it.  Instead
    macros "LOG()" and "SYSLOG()" are to be used.  These macros hide some of
    the parameters making the code simpler to read.  Also macro IFLOGGING() can
    be used to conditionalize the invocation of LOG() and SYSLOG() to avoid
    the overhead of formatting a complex message.  These macros are defined
    in "snmppe.h".

    Note the coding convention on the message string: it should start "Mxxx"
    where "xxx" is a unique decimal number.

    Message class "L_SYSLOG" is typically always enabled because messages in
    this class are important messages that need to be logged during normal
    SNMP PE operation.  This is a convention created by code in snmppe_init()
    that sets L_SYSLOG in the log_context block at init time.

*/

{
char    bigbuf[MAXERRMSG+1];/* Where we copy a message to add a newline */
                            /* (+1 is for the NULL byte)                */
char    *outmsg;            /* Pointer to final message to send         */
int     in_msg_len;         /* Computed length of inbound message       */

#ifdef __ultrix
extern void syslog();
#endif

/* <TC> In function pe_log(). . .                                       */
/* <TC> pe_log() may get called at any moment from any sending thread or*/
/* <TC> any receiving thread.  We take no chances of stepping on our    */
/* <TC> toes by acquiring a mutex to cover whatever the file pointer is.*/
/* <TC> If normal operations are underway, "syslog()" is used and the   */
/* <TC> file pointer in "log_state" is not used, although the mutex IS  */
/* <TC> used anyway.  The only ugliness comes if an attempt to acquire  */
/* <TC> or release the mutex fails.  In this instance a message is      */
/* <TC> unconditionally logged to syslog() and we return, hoping for the*/
/* <TC> the best!                                                       */

/* if (message class is enabled for logging) */
if ( (log_context->log_classes & msg_class) != 0) {


#ifdef MTHREADS
    /* if (acquire mutex on file pointer FAILED) */
    if (pthread_mutex_lock(&log_context->log_file_m) != 0) {
        /* write message to syslog */
        syslog(LOG_ERR, MSG(msg001, "M001-mutex lock failed in pe_log(): %m"));
        return;
        }
#endif


    /* if (it doesn't end in a newline and there is room) */
    if ( ( (in_msg_len = strlen(msg)) <  MAXERRMSG ) &&
         ( *(msg + in_msg_len - 1)    != '\n'     )     ){
        /* add a newline */
        strcpy(bigbuf, msg);            /* copy inbound message            */
        bigbuf[in_msg_len] = '\n';      /* add the '\n' over the null byte */
        bigbuf[in_msg_len+1] = '\0';    /* add the '\0'                    */
        outmsg = bigbuf;                /* New outbound message is here    */
        }
    else {
        outmsg = msg;   /* Just use the original message */
        }

    /* if (debugging mode enabled) */
    if (log_context->debugging_mode == TRUE) {

        char    biggerbuf[MAXERRMSG+1+12]; /* Where we copy msg to add class */
        int     i;                      /* Loop index                        */

        build_line_prefix(log_context, msg_class, biggerbuf, &i);

        /* load msg to message build buffer */
        strcpy(&biggerbuf[i], outmsg);

        /* write message to log_state's log file */
        fprintf(log_context->log_file, biggerbuf);
        }
    else {
        /* write message to syslog() */
        syslog(syslog_code, outmsg);
        }


#ifdef MTHREADS
    /* if (release mutex on file pointer FAILED) */
    if (pthread_mutex_unlock(&log_context->log_file_m) != 0) {
        /* write message to syslog */
        syslog(LOG_ERR, MSG(msg002, "M002-mutex unlock failed in pe_log(): %m"));
        return;
        }
#endif
    }
}

/* log_service - Log an interpretation of a Service Block to Log File */
/* log_service - Log an interpretation of a Service Block to Log File */
/* log_service - Log an interpretation of a Service Block to Log File */

void
log_service( bp, msg_class, svc)

big_picture_t   *bp;        /*-> The Big Picture                       */
int             msg_class;  /* Indicates class dump "msg" belongs to   */
service_t       *svc;       /*-> Service Block to be dumped            */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information needed by SNMP PE to operate.

    "msg_class" - The set bit in this integer indicates the debugging
    class-of-messages that the dump message belongs to.  This determines
    the heading that accompanies the dump of the service block.

    "svc" is the address of a Service Block that contains the newly
    received and deserialized inbound PDU.


OUTPUTS:

    The function returns nothing, but baring unrecoverable errors, an
    interpreted dump of the specified service block is logged to the current
    log file.  Note that unlike "pe_log()", this function ALWAYS writes to
    the log file, (never the system log) and it is only called when SNMP PE
    is running in "debug" mode.

BIRD'S EYE VIEW:
    Context:
        The caller is any routine in SNMP PE (after snmppe_init() has
        been called) that wishes to log an interpretation of a service
        block to the current log file.

    Purpose:
        This primarily supports the "PDU" class of diagnostic dump, providing
        interpreted description of the current values of the fields of
        a service block.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (service block address is NULL)
        <SYSLOG "Mxxx - NULL Service block address presented for log">
        <return>

    <build message class name as output buffer line prefix>

if MTHREADS
    if (acquire mutex on file pointer FAILED)
        <write message to syslog>
        <return>
endif

    <issue "Overhead Section:">
    <interpret "svc_active">
    <interpret "ok_community" to show community name>        
    <interpret "inbound" to show inet address>
    <interpret "svc_count">
    <interpret "replies_rcvd">

    <issue "PDU Section:">
    <interpret "version">
    <interpret "community name">
    <interpret "pdu">
    <interpret "request_id">
    <interpret "error_status">
    <interpret "error_index">

    <issue "PDU Varbind List Section:">
    for (each varbind entry)
        <issue "Varbind Entry %d">
        <interpret "vb_entry_id" in decimal, hex and split decimal>
        <interpret "class_oid">
        <issue "MIR Instance Info List:">
        if (instance list is NULL)
            <issue "NULL List header">
        else
            for (every class list in full MIR Instance List)
                for (every entry in the MIR Instance List>
                    <interpret "inst_oid">
                    <interpret "AVLtag">
        <interpret "in_entry" AVL>
        <interpret "out_entry" AVL>
        <interpret "reply_error">

if MTHREADS
    if (release mutex on file pointer FAILED)
        <write message to syslog>
        <return>
endif


OTHER THINGS TO KNOW:

    This function doesn't use the "Mxxx" form of message.
*/

#define INDENT 4
#define LOGIT() fprintf(bp->log_state.log_file, buf)
{
char            buf[LINEBUFSIZE];       /* Error Message build buffer        */
int             ss;                     /* Error Msg "String Start" in buf[] */
varbind_t       *vbe;                   /* Scanner for varbind list          */
char            *pdu_type_name;         /* "GET", "SET" etc                  */
char            *reply_name;            /* "noError", "tooBig", etc          */
int             i;                      /* Handy index                       */
int             entry_count;            /* Counts Varbind Entries            */
char            *oid_string;            /* -> Converted-to-Text OID          */


/* if (service block address is NULL) */
if (svc == NULL) {
    SYSLOG(LOG_ERR, MSG(msg150, "M150 - NULL Service block address presented for log"));
    return;
    }

/* build message class name as output buffer line prefix */
build_line_prefix(&bp->log_state, msg_class, buf, &ss);

#ifdef MTHREADS
/* if (acquire mutex on file pointer FAILED) */
if (pthread_mutex_lock(&log_context->log_file_m) != 0) {
    /* write message to syslog */
    syslog(LOG_ERR, MSG(msg148, "M148 - mutex lock failed in log_service(): %m"));
    return;
    }
#endif

/* issue "Overhead Section:" */
/* interpret "svc_active" */
/* interpret "ok_community" to show community name & access mode*/
sprintf(&buf[ss],
        "PDU SERVICE BLOCK  active '%s'  community '%s'  access '%d'\n",
        ((svc->svc_active == FALSE) ? ("FALSE") : ("TRUE")),
        svc->ok_community->comm_name,
        svc->ok_community->access_mode
        );
LOGIT();

/* interpret "inbound" to show inet address */
/* interpret "svc_count" */
/* interpret "replies_rcvd" */

sprintf(&buf[ss],
        "inbound '%s'  svc_count '%d'  replies_rcvd '%d'\n",
        inet_ntoa(svc->inbound.sin_addr),
        svc->svc_count,
        svc->replies_rcvd
        );
LOGIT();


/* issue "PDU Section:" */
/* interpret "version" */
/* interpret "community name" */
/* interpret "pdu" */
pdu_type_name = "Unknown";
if (svc->pdu == GET) pdu_type_name = "GET";
else if (svc->pdu == SET) pdu_type_name = "SET";
else if (svc->pdu == GETNEXT) pdu_type_name = "GET-NEXT";
else if (svc->pdu == GETRESP) pdu_type_name = "GET-RESPONSE";
else if (svc->pdu == TRAP) pdu_type_name = "TRAP";

buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */
sprintf(&buf[ss],
        "PDU SECTION  type '%s'  version '%d'  community '%.*s'\n",
        pdu_type_name,
        svc->version,
        svc->comm_namelen,
        svc->comm_name
        );
LOGIT();

/* interpret "request_id" */
/* interpret "error_status" */
/* interpret "error_index" */
sprintf(&buf[ss],
        "             requestID '%d'  error_status '%d'  error_index '%d'\n",
        svc->request_id,
        svc->error_status,
        svc->error_index
        );
LOGIT();

/* issue "PDU Varbind List Section:" */
buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */
sprintf(&buf[ss],
        "PDU VARBIND LIST SECTION\n"
        );
LOGIT();

/* Indent the Varbind list dump */
for (i=ss; i < (ss + INDENT); i++)
    buf[i] = ' ';
ss = i;

entry_count = 1; /* Counts Entries in the loop */

/* for (each varbind entry) */
for (vbe = svc->varbind_list;
     vbe != NULL && vbe->vb_entry_id != 0;
     vbe=vbe->next) {

    /* issue "Varbind Entry %d" */
    sprintf(&buf[ss], "Varbind Entry %d -------------------\n",
            entry_count++);
    LOGIT();

    /* interpret "vb_entry_id" in decimal, hex and split decimal */
    sprintf(&buf[ss],
            "vb_entry_id: decimal(%d)  hex(%x)  split(%d/%d)\n",
            vbe->vb_entry_id,
            vbe->vb_entry_id,
            ((vbe->vb_entry_id >> 8) & 0xFFFFFF),
            (vbe->vb_entry_id & 0xFF)
            );
    LOGIT();

    /* interpret "next" as hex address */
    /* interpret "class_oid" */
    moss_oid_to_text(&vbe->class_oid, NULL, NULL, NULL, &oid_string);
    if (oid_string == NULL) {
        oid_string = "<none>";
        sprintf(&buf[ss],"class OID '%s'\n", oid_string);
        }
    else {
        sprintf(&buf[ss],"class OID '%s'\n", oid_string);
        free(oid_string);
        }
    LOGIT();


    /* Indent the MIR Instance list dump */
    for (i=ss; i < (ss + INDENT); i++)
        buf[i] = ' ';
    ss = i;

    /* issue "MIR Instance Info List:" */
    sprintf(&buf[ss],"MIR Instance Info List (Classes Top-Down):\n");  LOGIT();
    sprintf(&buf[ss],
            "<Instance OID>  <AVLTag(dec/hex)>\n");
    LOGIT();

    /* if (instance list is NULL) */
    if (vbe->inst_list == NULL) {
        /* issue "NULL List header" */
        sprintf(&buf[ss],"List has NULL Header\n");  LOGIT();
    }
    else {
        snmp_instance_t      *inst;
        snmp_instance_t      *first_in_class;

        /* for (every class list in the full MIR instance List) */
        for (first_in_class = vbe->inst_list;
             first_in_class != NULL;
             first_in_class = first_in_class->next_class) {

            /* for (every entry in the class MIR Instance List */
            for (inst = first_in_class; inst != NULL; inst=inst->next) {

                /* interpret "inst_oid" */
                moss_oid_to_text(&inst->inst_oid, NULL, NULL, NULL, &oid_string);
                if (oid_string == NULL) {
                    oid_string = "<none>";
                    /* interpret "AVLtag" */
                    sprintf(&buf[ss],
                        "%s  (%d/%x)\n",
                        oid_string,
                        inst->AVLtag, inst->AVLtag
                        );                
                    }
                else {
                    /* interpret "AVLtag" */
                    sprintf(&buf[ss],
                        "%s  (%d/%x)\n",
                        oid_string,
                        inst->AVLtag, inst->AVLtag
                        );                
                    free(oid_string);
                    }
                LOGIT();
                }

            /* Line as Class separator*/
            sprintf(&buf[ss], "---\n");
            LOGIT();
            }    
        }

    ss = ss - INDENT;
    buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */

    /* interpret "in_entry" AVL */
    sprintf(&buf[ss], "in_entry AVL:\n");  LOGIT();
    snmppe_print_avl(vbe->in_entry, bp->log_state.log_file, buf, ss);
    buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */

    /* interpret "out_entry" AVL */
    sprintf(&buf[ss], "out_entry AVL:\n");  LOGIT();
    snmppe_print_avl(vbe->out_entry, bp->log_state.log_file, buf, ss);
    buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */

    /* interpret "reply_error" */
    if (vbe->reply_error == noError) reply_name = "noError";
    else if (vbe->reply_error == tooBig) reply_name = "tooBig";
    else if (vbe->reply_error == noSuch) reply_name = "noSuch";
    else if (vbe->reply_error == badValue) reply_name = "badValue";
    else if (vbe->reply_error == readOnly) reply_name = "readOnly";
    else if (vbe->reply_error == genErr) reply_name = "genErr";
    else reply_name = "<Unknown>";
    sprintf(&buf[ss], "reply_error '%s'\n", reply_name);  LOGIT();    
    buf[ss] = '\n';  buf[ss+1] = '\0';  LOGIT();        /* Blank Line */
    }

#ifdef MTHREADS
/* if (release mutex on file pointer FAILED) */
if (pthread_mutex_unlock(&log_context->log_file_m) != 0) {
    /* write message to syslog */
    syslog(LOG_ERR, MSG(msg149, "M149 - mutex unlock failed in log_service(): %m"));
    return;
    }
#endif

}

/* crash_exit - SNMP Protocol Engine Crash-Shutdown Routine */
/* crash_exit - SNMP Protocol Engine Crash-Shutdown Routine */
/* crash_exit - SNMP Protocol Engine Crash-Shutdown Routine */

void
crash_exit( bp, msg )

big_picture_t   *bp;            /*-> The Big Picture                       */
char            *msg;           /* The error msg to be written to syslog   */

/*
INPUTS:

    "bp" - the global "big picture" structure

    "msg" - The string to be placed into syslog (it should NOT have
    any format specifiers (like "%d") in it).  By coding convention it should
    start with the letters "Mxxx" where "xxx" is a unique decimal number.
    By grepping for "Mxxx" in the sources a person should be able to locate
    where the message is being issued from.  Thus we avoid cluttering the
    message with function names.

    crash_exit() appends the phrase "--unrecoverable: snmp_pe exiting" to the
    end of the message before logging it.

OUTPUTS:

    The function returns nothing, but causes the complete shutdown
    of all SNMP PE threads after the specified message has been logged to the
    system log.


BIRD'S EYE VIEW:
    Context:
        The caller is any routine in SNMP PE (after snmppe_init() has
        been called) that has encountered a fatal unrecoverable error.

    Purpose:
        This is the central function all other SNMP PE function call to cause
        SNMP PE to exit in an error situation.


ACTION SYNOPSIS OR PSEUDOCODE:

    <if possible, build copy of the message w/termination phrase at the end>
    <log the message to syslog>

if MTHREADS
    <signal other threads to exit?>
    <shutdown RPC?>
endif

    if (debugging mode NOT enabled)
        <close syslog>
    else
        <close the logging file>

    <call exit()>

OTHER THINGS TO KNOW:

    Note the coding convention on the message string: it should start "Mxxx"
    where "xxx" is a UNIQUE decimal number.

    Adherence to this coding convention allows easy discovery via grep
    of the place in the sources where any given error message is issued.
    This is not by accident.

*/

#define TERM_PHRASE "--unrecoverable: snmp_pe exiting\n"
{
char    bigbuf[MAXERRMSG+1]; /* Where msg w/extra phrase is copied and built */
                             /* (+1 is for null byte)                        */
extern  void closelog();


/* if possible, build copy of the message w/termination phrase at the end    */
bigbuf[MAXERRMSG] = '\0';               /* Safety termination  */
strncpy(bigbuf, msg, MAXERRMSG);        /* Copy in the message */

/* If there is room left in the buffer for the entire phrase. . . . . . . . .*/
if ( ( MAXERRMSG - strlen(bigbuf)) > strlen(TERM_PHRASE)) {
    strcpy(&bigbuf[strlen(bigbuf)], TERM_PHRASE);       /*  . . .then Add it */
    }

/* log the message to syslog */
SYSLOG( LOG_ERR, bigbuf);

/*
---if MTHREADS
    <signal other threads to exit>

    NOTE: Some sort of interlock should probably be established to preclude
          more than one thread being in this crash function at once.

          Note that you don't know whether a sending or receiving thread has
          entered this function.

          The "close" done below might be crunching an I/O operation initiated
          on it by another thread.  This has to be precluded somehow.

    <shut down RPC?>  --it could be an RPC-initiated thread that has entered
                      --this function.
---endif
*/

/* if (debugging mode NOT enabled) */
if (bp->log_state.debugging_mode == FALSE) {
    /* close syslog */
    closelog();
    }
else {
    /* close the logging file */
    fclose(bp->log_state.log_file);
    }

/* call exit() */
exit(0);

}
/* dump_instance_list - Release all storage for an Instance Info List */
/* dump_instance_list - Release all storage for an Instance Info List */
/* dump_instance_list - Release all storage for an Instance Info List */

void
dump_instance_list( inst )

snmp_instance_t      **inst;         /* ->> Instance List to be released */

/*
INPUTS:

    "inst" is the address of a pointer to an Instance List (in a varbind
    entry block probably) to be released.


OUTPUTS:

    The function returns nothing, but causes the pointer whose address is
    passed to be set NULL after any list present there is released.


BIRD'S EYE VIEW:
    Context:
        The caller is function netio_deserialize_pdu() or function
        mir_class_inst_ROLLNXT(), each is trying to obtain a varbind entry
        block for re-use.

    Purpose:
        This function "knows" the structure of the instance list in a
        varbind entry block and can properly (we hope!) release all storage
        associated with such a list.


ACTION SYNOPSIS OR PSEUDOCODE:

    while (a class instance list remains)
        <grab pointer to current block>
        if (oid storage is present)
            <release the storage>
        while (other blocks in this class remain)
            <grab pointer to next block>
            if (oid storage is present)
                <release the storage>
            <step pointer to next block if any>
            <release this block>
        <step pointer to next block if any>
        <release this block>

OTHER THINGS TO KNOW:


*/

{
/* release each class in the entire list */
while (*inst != NULL)
    {
    snmp_instance_t  *this;

    this = *inst;    /* Point to current block */

    /* if there is storage for the OID value */
    if (this->inst_oid.value != NULL) {
        free(this->inst_oid.value);
        }

    /* Dump any other blocks in this class */
    while (this->next != NULL)
        {
        snmp_instance_t  *this_cb;           /* This class's block */

        this_cb = this->next;           /* Point to current block */

        /* if there is storage for the OID value */
        if (this_cb->inst_oid.value != NULL) {
            free(this_cb->inst_oid.value);
            }

        this->next = this_cb->next;   /* Step to the next for class */
        free(this_cb);                /* Free current block */
        }

    *inst = this->next_class;  /* Step to next class */
    free(this);                                /* Free current block */
    }
}

/* build_line_prefix - Builds ASCII prefix into Output Line Buffer */
/* build_line_prefix - Builds ASCII prefix into Output Line Buffer */
/* build_line_prefix - Builds ASCII prefix into Output Line Buffer */

void
build_line_prefix( log_context, msg_class, dmpbuf, ss )

log_state_t     *log_context;   /*-> log info needed for logging class      */
int             msg_class;      /* Bit Pattern flag for message class       */
char            dmpbuf[];       /* Output Line buffer for prefix            */
int             *ss;            /* Start String position (index into dmpbuf */

/*
INPUTS:

    "log_context" is the logging context information block from the Big
    Picture that contains all info about logging (including pointers to
    the message class names).

    "msg_class" is an integer containing a the bit(s) set corresponding
    to the message class whose name must be placed into . . .

    "dmpbuf" is a character array being used is an output line buffer in
    a diagnostic dump.

    "ss" is an integer that is set up on return to point to the null byte
    which is placed into the dmpbuf[] array at the end of the prefix.  This
    corresponds to where the caller can continue to build a diagnostic line
    destined for output.


OUTPUTS:

    The function returns nothing, but causes the ASCII string describing
    the message class specified by "msg_class" bit mask" to be loaded into
    the beginnig of "dmpbuf[]".  If "msg_class" is somehow invalid, you'll
    get "MSGCLASS?" as the prefix.


BIRD'S EYE VIEW:
    Context:
        The caller is any code that is busy formatting a diagnostic output
        line.

    Purpose:
        This function takes care of the conversion of a bit-mask specifying
        a message class into the ASCII name of that message class.


ACTION SYNOPSIS OR PSEUDOCODE:

    for (each possible message class)
        if (class matches passed class)
           <select pointer to class name string>
           <break>

    if (no message class selected)
        <use "MSGCLASS?">

    <load name padded to PREFIXSIZE characters into output buffer>
    <null terminate the prefix>
    <return "start-string" index value>


OTHER THINGS TO KNOW:

    Note that the prefix is null-terminated, and "ss" is set to the index
    in dmpbuf[] that corresponds to this null-byte.  "ss" stands for
    "start-string" which is where the caller starts building any new string
    to go into the output buffer.

MTHREADS
    If multiple-threads is implemented, this function should probably be
    enhanced to fetch a thread-id of some sort and build it into the
    prefix that it builds.  In this way it'll be possible to trace which
    threads are issuing messages.
*/

{
char    *msg_class_name=NULL;   /* -> Message Class name in log_state */
int     i;                      /* Handy index                     */

/* for (each possible message class) */
for (i=0; i < LOG_CLASS_COUNT; i++) {
    /* if (class matches passed class) */
    if (log_context->dbg_flag[i] == msg_class) {  /* select class name */
        msg_class_name = log_context->dbg_name[i];
        break;
        }
    }

/* if (no message class selected) */
if (msg_class_name == NULL) {
    msg_class_name = "MSGCLASS?"; /* set selected class name to "Unknown" */
    }

/* load padded name to message build buffer */
strcpy(dmpbuf, msg_class_name);
for (i = strlen(msg_class_name); i < PREFIXSIZE; i++)
    dmpbuf[i] = ' ';

dmpbuf[i] = '\0';

/* return "start-string" index value */
*ss = i;

/*
| As we fall out of here, the buffer to be used by the Dump routine has
| been initialized with a prefatory message class name (indicating what
| kind of dump we're doing) and 'ss' has been set to the entry in the
| dmpbuf[] array where it can start building lines to be dumped.
*/
}

/* pe_oid_text - Convert binary OID to text string */
/* pe_oid_text - Convert binary OID to text string */
/* pe_oid_text - Convert binary OID to text string */

char *
pe_oid_text( oid )

object_id       *oid;   /*--> The Object Identifier to be printed */

/*
INPUTS:

    "oid" is a pointer to an Object-ID structure whose textual representation
    is desired.

OUTPUTS:

    The function calls the standard MOSS routine to obtain a converted
    string representation.  If the conversion fails, a string indicating
    this is still returned.  The caller must free the storage.


BIRD'S EYE VIEW:
    Context:
        The caller is the trace/debugging code typically.

    Purpose:
        This function takes care of returning a string representation
        of an OID in an easy way.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (oid pointer is NULL)
        <set bad string to "<NULL PTR TO OID STRUCT>"
    else if (no arcs in oid)
        <set bad string to "<NO ARCS IN OID STRUCT>"
    else 
        <attempt OID conversion>
        if (conversion failed)
            <set bad string to "<OID Text Convert Failed>">

    if (conversion failed)
        if (attempt to allocate storage for message failed)
            <return NULL>
        <copy message to storage>
        <return storage address>

    <return converted OID text representation>


OTHER THINGS TO KNOW:

    
*/

#define OID_ERR_STRING "<OID Text Convert Failed>"
{
char    *oid_string=NULL;    /* Where we store ptr to string to return */
char    *bad_string=NULL;    /* Error string to use                    */


/* if (oid pointer is NULL) */
if (oid == NULL) {
    /* set bad string to "<NULL PTR TO OID STRUCT>" */
    bad_string = "<NULL PTR TO OID STRUCT>";
    }
else if (oid->count == 0 || oid->value == NULL) {    /* no arcs in oid */
    /* set bad string to "<NO ARCS IN OID STRUCT>" */
    bad_string = "<NO ARCS IN OID STRUCT>";
    }
else { 
    /* attempt OID conversion */
    moss_oid_to_text(oid, NULL, NULL, NULL, &oid_string);

    /* if (conversion failed) */
    if (oid_string == NULL) {
        /* set bad string to "<OID Text Convert Failed>" */
        bad_string = "<OID Text Convert Failed>";
        }
    }

/* If (conversion failed ) */
if (bad_string != NULL) {

    /* if (attempt to allocate storage for message failed) */
    if ((oid_string = (char *) malloc(strlen(bad_string) + 1)) == NULL) 
        return (NULL);

    /* copy standard message to storage */
    strcpy(oid_string, bad_string);

    /* return storage address */
    }

/* return converted OID text representation */
return(oid_string);
}

#ifdef SABER
/* codecenter_print_oid - Prints an OID when called by CodeCenter (Saber-C) */
/* codecenter_print_oid - Prints an OID when called by CodeCenter (Saber-C) */
/* codecenter_print_oid - Prints an OID when called by CodeCenter (Saber-C) */

void
codecenter_print_oid( oid, low, hi ) /*ARGSUSED*/

object_id       *oid;   /*--> The Object Identifier to be printed */
char            *low;   /* Low-memory limit */
char            *hi;    /* Hi-memory limit */

/*
INPUTS:

    "oid" is a pointer to an Object-ID structure.  Note that this function
    can be invoked (from CodeCenter) "on" an OID and also on a *pointer* to
    an object_id structure.  This is because CodeCenter will invoke this
    function with a pointer to a structure or the *value* of a pointer.

    "low" and "hi" are ignored.

OUTPUTS:

    The function calls fprintf on file "saber_stdout" to output a string
    that contains a representation of the object_id.  A null pointer is
    correctly handled.


BIRD'S EYE VIEW:
    Context:
        The caller is the "CodeCenter" tool (Saber-C).

    Purpose:
        This function makes it much easier to see Object IDs floating
        around inside the SNMP PE when debugging.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (oid pointer is NULL)
        <saber-out "(OIDnullptr)">
    else
        if (count of arcs is zero)
            <saber-out "(OIDnoarcs)">
        else
            if (conversion succeeded)
                <saber-out the oid string>
                <free the text string>
            else
                <saber-out "(OIDconvfail)">
    <flush the saber file>


OTHER THINGS TO KNOW:

    This function is enabled in CodeCenter when you submit to the
    workspace shell prompt:

        userprint codecenter_print_oid object_id *  (enables use w/pointer)
        userprint codecenter_print_oid object_id    (enables use on structure)

    (also:

        userprint codecenter_print_oid struct object_id *

     ...because the definitions in mir.h don't exactly match those in man.h)

    I believe both these commands can be placed into the ".ccenterinit" file
    for automatic execution when CodeCenter starts.  If you set the
    "print_custom" option using:

        setopt print_custom

    then this function is called wherever a value of this type is to be
    printed (including printing in the workspace which is otherwise the
    default.

NOTE WELL:
    The Saber-C documentation states that if "print_custom" is enabled, then
    one of the many places where this function is called is from the
    "Data Browser" and that there are special considerations noted in
    a Saber "Technical Note" that have to be followed to make "Data Browser"
    work correctly.  I don't have the note and I didn't follow any
    special considerations, so be careful not to use "print_custom" TRUE
    with Data-Browser.

*/

{
extern FILE *saber_stdout;      /* Where the output goes for saber   */
char    *oid_text=NULL;         /* -> Converted text string returned */

/* if (oid pointer is NULL) */
if (oid == NULL) {
    /* saber-out "(OIDnullptr)" */
    fprintf(saber_stdout, "(OIDnullptr)");
    }
else {
    /* if (count of arcs is zero) */
    if (oid->count <= 0) {
        /* saber-out "(OIDnoarcs)" */
        fprintf(saber_stdout, "(OIDnoarcs)");
        }
    else {
        /* if (conversion succeeded) */
        if (moss_oid_to_text(oid,NULL,NULL,NULL,&oid_text) == MAN_C_SUCCESS) {

            /* saber-out the oid string */
            fprintf(saber_stdout, "(%s)", oid_text);

            /* free the text string */
            free(oid_text);
            }
        else {
            /* saber-out "(OIDconvfail)" */
            fprintf(saber_stdout, "(OIDconvfail)");
            }
        }
    }

/* flush the saber file */
fflush(saber_stdout);

}
#endif

/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1990                                  *
 *                                                                          *
 *      This is an unpublished work which was created in the indicated      *
 *      year, which contains confidential and secret information,  and      *
 *      which is protected under the copyright laws.  The existence of      *
 *      the copyright notice is not to be construed as an admission or      *
 *      presumption that publication has occurred. Reverse engineering      *
 *      and unauthorized copying is strictly prohibited.   All  rights      *
 *      reserved.                                                           *
 *                                                                          *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management 
 *
 * Abstract:
 *
 *    Common diagnostic print routines used:
 *
 *                    snmppe_print_avl()
 *                    print_constructed_avl()
 *                    print_value()
 *                    get_man_status()
 *                    get_type()
 *                    print_get_attributes()
 *                    print_send_get_reply()
 *                    print_set_attributes()
 *                    print_send_set_reply()
 *                    print_invoke_action()
 *                    print_send_action_reply()
 *                    print_create_instance()
 *                    print_send_create_reply()
 *                    print_delete_instance()
 *                    print_send_delete_reply()
 *
 * Author:
 *
 *    Kathy Faust; stolen from DNA test MOM.  Replace dna_application_table
 *                 with inet_application_table.
 *
 * Date:
 *
 *    Aug 8, 1991
 *
 * Revision History :
 *
 *    D. D. Burns - December 1991, stolen from SNMP Test MOM.  Modified to
 *                                 fit into SNMP PE's logging system.
 */


#define IS_UNIVERSAL( value )         ( ( value >> 30 ) == 0 )
#define IS_APPLICATION( value )       ( ( value >> 30 ) == 1 )
#define IS_CONTEXT_SPECIFIC( value )  ( ( value >> 30 ) == 2 )
#define IS_PRIVATE( value )           ( ( value >> 30 ) == 3 )
#define TAG_VALUE( value )            ( value & 0x1fffffff )


/*
 *  Local
 */
static
char *man_status_table_1[] = {
    "MAN_C_NO_SUCH_CLASS" , 
    "MAN_C_NO_SUCH_OBJECT_INSTANCE" , 
    "MAN_C_ACCESS_DENIED" , 
    "MAN_C_SYNC_NOT_SUPPORTED" , 
    "MAN_C_INVALID_FILTER" , 
    "MAN_C_NO_SUCH_ATTRIBUTE_ID" , 
    "MAN_C_INVALID_ATTRIBUTE_VALUE" , 
    "MAN_C_GET_LIST_ERROR" , 
    "MAN_C_SET_LIST_ERROR" , 
    "MAN_C_NO_SUCH_ACTION" , 
    "MAN_C_PROCESSING_FAILURE" , 
    "MAN_C_DUPLICATE_M_O_INSTANCE" , 
    "MAN_C_NO_SUCH_REFERENCE_OBJECT" , 
    "MAN_C_NO_SUCH_EVENT_TYPE" , 
    "MAN_C_NO_SUCH_ARGUMENT" , 
    "MAN_C_INVALID_ARGUMENT_VALUE" , 
    "MAN_C_INVALID_SCOPE" , 
    "MAN_C_INVALID_OBJECT_INSTANCE" , 
    "MAN_C_MISSING_ATTRIBUTE_VALUE" , 
    "MAN_C_CLASS_INSTANCE_CONFLICT" , 
    "MAN_C_COMPLEXITY_LIMITATION" , 
    "MAN_C_MISTYPED_OPERATION" ,
    "MAN_C_NO_SUCH_INVOKE_ID" ,
    "MAN_C_OPERATION_CANCELLED" ,
    "MAN_C_INVALID_OPERATION" ,
    "MAN_C_INVALID_OPERATOR" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "MAN_C_DIRECTIVE_NOT_SUPPORTED" , 
    "MAN_C_ENTITY_CLASS_NOT_SUPPORTED" , 
    "MAN_C_INVALID_USE_OF_WILDCARD" , 
    "UNKNOWN reply code" ,
    "MAN_C_CONSTRAINT_VIOLATION" , 
    "MAN_C_WRITE_ONLY_ATTRIBUTE" , 
    "MAN_C_READ_ONLY_ATTRIBUTE" , 
    "MAN_C_DUPLICATE_ATTRIBUTE" , 
    "MAN_C_DUPLICATE_ARGUMENT" , 
    "UNKNOWN" ,
    "MAN_C_REQUIRED_ARGUMENT_OMITTED" , 
    "MAN_C_FILTER_INVALID_FOR_ACTION" , 
    "MAN_C_INSUFFICIENT_RESOURCES" ,
    "MAN_C_NO_SUCH_ATTRIBUTE_GROUP" , 
    "MAN_C_FILTER_USED_WITH_CREATE" 
} ; 

static
char *man_status_table_2[] = {
    "MAN_C_WILD_NOT_AT_LOWEST_LEVEL" ,
    "MAN_C_WILD_CLASS_WITH_FILTER" ,
    "MAN_C_WILD_INVALID_DIRECTIVE" ,
    "MAN_C_WILD_WITH_CREATE" ,
    "MAN_C_WILD_INVALID_GROUP" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "UNKNOWN" ,
    "MAN_C_SCOPE_TOO_COMPLEX" ,
    "MAN_C_SYNC_TOO_COMPLEX" ,
    "MAN_C_FILTER_TOO_COMPLEX"
} ;

static
char *man_status_table_3[] = {
     "MAN_C_ALREADY_INITIALIZED" ,
     "MAN_C_BAD_PARAMETER" ,
     "MAN_C_FAILURE" ,
     "MAN_C_HANDLE_NOT_BOUND" ,
     "MAN_C_HAS_ACTIVE_CHILDREN" ,
     "MAN_C_MO_TIMEOUT" ,
     "MAN_C_MOLD_TIMEOUT" ,
     "MAN_C_NO_ELEMENT" ,
     "MAN_C_NO_MOLD" ,
     "MAN_C_NO_REPLY" ,
     "MAN_C_NO_SUCH_PARENT_CLASS" ,
     "MAN_C_NOT_CONSTRUCTED" ,
     "MAN_C_NOT_INITIALIZED" ,
     "MAN_C_OBJECT_ALREADY_EXISTS" ,
     "MAN_C_PE_TIMEOUT" ,
     "MAN_C_READ_ONLY"
 } ;

static
char *man_status_table_4[] = {
    "MAN_C_EQUAL" ,
    "MAN_C_TRUE" ,
    "MAN_C_NOT_EQUAL" ,
    "MAN_C_FALSE"
} ;

static
char *man_status_table_5[] = {
    "MAN_C_NOT_SUPPORTED" ,
    "MAN_C_END_OF_MIB"
} ;

static
char *inet_application_tag_table[] = {
    "INET_C_SMI_IP_ADDRESS" ,
    "INET_C_SMI_COUNTER" ,
    "INET_C_SMI_GAUGE" ,
    "INET_C_SMI_TIME_TICKS" ,
    "INET_C_SMI_OPAQUE"
} ;


void
snmppe_print_avl( 
	  avl_handle,
          ofile,
          buf,
          ss
	  )
avl *avl_handle ;
FILE *ofile;                            /* Output File                       */
char            buf[LINEBUFSIZE];       /* Error Message build buffer        */
int             ss;                     /* Error Msg "String Start" in buf[] */
/*
 * Function description:
 *
 *    Prints the contents of an AVL
 *
 * Arguments:
 *
 *    avl_handle            pointer to the AVL handle
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

#define PLOGIT() fprintf(ofile, buf)
{
    char              *string = ( char * )NULL ;
    int               last_one  = FALSE ;
    man_status        stat ;
    object_id         *oid ;
    octet_string      *octet ;
    unsigned int      modifier ;
    unsigned int      tag ;
    int               i ;

    if (avl_handle == NULL) {
        sprintf(&buf[ss], "NULL AVL\n");  PLOGIT();
        return;
        }

    if ( ( stat = moss_avl_reset( avl_handle ) ) != MAN_C_SUCCESS ) {
	sprintf(&buf[ss], "Error on moss_avl_reset() ", stat ) ;  PLOGIT();
        return;
        }

    while ( last_one != TRUE )
    {
	if ( ( stat = moss_avl_point( avl_handle ,
				     &oid , &modifier, &tag,
				     &octet, & last_one ) )
	    != MAN_C_SUCCESS )
	{
	    if ( stat == MAN_C_NO_ELEMENT ) {
		sprintf(&buf[ss],  "----AVL has no elements----\n" ) ;
                PLOGIT();
                }
	    else {
                sprintf(&buf[ss], "Error on moss_avl_point() ", stat ) ;
                PLOGIT();
                }
	    return;
	}
	sprintf(&buf[ss], "---------------------------------------\n");
        PLOGIT();
	if ( oid )
	{
	    stat = moss_oid_to_text( oid, ".", "", "", &string ) ;
	    if ( MAN_C_SUCCESS == stat )
	    {
		sprintf(&buf[ss],  "attribute id =   %s\n", string ) ;
                PLOGIT();
		free( string ) ;
	    }
	    else {
	        sprintf(&buf[ss],  "attribute id =   NULL\n" ) ;  PLOGIT();
                }
	}
	else {
	    sprintf(&buf[ss], "attribute id =   NULL\n" ) ;  PLOGIT();
            }

	sprintf(&buf[ss],
                "Modifier =       %s(%d)\n",
                get_man_status(modifier),
                modifier
                );
        PLOGIT();

	sprintf(&buf[ss],  "Attribute type = %s\n", get_type( tag ) ) ;
        PLOGIT();

	if IS_CONSTRUCTED( tag ) {
            for (i=ss; i < (ss + INDENT); i++)
                buf[i] = ' ';
            ss += INDENT;
	    last_one = print_constructed_avl( avl_handle, ofile, buf, ss ) ;
            ss -= INDENT;
            }
	else
	{
	    sprintf(&buf[ss],  "Attribute value: \n" ) ;  PLOGIT();
            for (i=ss; i < (ss + 17); i++)
                buf[i] = ' ';
	    print_value( tag, octet, ofile, buf, (ss + 17) ) ; 
	}
	sprintf(&buf[ss], "--------------------------------------\n");
        PLOGIT();
    }
} /* end snmppe_print_avl() */


static int
print_constructed_avl( avl_handle, ofile, buf, ss )
avl *avl_handle ;
FILE *ofile;
char            buf[LINEBUFSIZE];       /* Error Message build buffer        */
int             ss;                     /* Error Msg "String Start" in buf[] */

/*
 * Function description:
 *
 *    Prints the contents of a constructed AVL
 *
 * Arguments:
 *
 *    avl_handle            pointer to the AVL handle
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

{
    int               last_one ;
    int               more ;
    object_id         *oid ;
    octet_string      *octet ;
    unsigned int      modifier ;
    unsigned int      tag ;
    int               i;
    char              *string ;
    man_status stat ;

    sprintf(&buf[ss], "IS CONSTRUCTED:\n" );
    PLOGIT();
    more = TRUE ;
    do
    {
	if ( (stat = moss_avl_point(avl_handle ,
				      &oid , &modifier, &tag,
				      &octet, & last_one ) )
	    != MAN_C_SUCCESS )
	{
	    if ( stat == MAN_C_NO_ELEMENT ) {
		sprintf(&buf[ss],  "----AVL has no elements----\n" ) ;
                PLOGIT();
                }
	    else {
	        sprintf(&buf[ss], "Error on moss_avl_point() ", stat ) ;
                PLOGIT();
                }
	    return( TRUE ) ;
	}
	sprintf(&buf[ss],"--------------------------------------\n");
        PLOGIT();
	if ( oid != NULL )
	{
	    stat = moss_oid_to_text( oid, ".", "", "", &string ) ;
	    if ( MAN_C_SUCCESS == stat )
	    {
		sprintf(&buf[ss], "attribute id  =  %s\n", string ) ;
                PLOGIT();
		free( string ) ;
	    }
	    else {
	        sprintf(&buf[ss], "attribute id = NULL\n" ) ;
                PLOGIT();
                }
	}
	else {
	    sprintf(&buf[ss], "attribute id = NULL\n" ) ;
            PLOGIT();
            }
	sprintf(&buf[ss], "Attribute type = %s\n", get_type( tag ) ) ;
        PLOGIT();
	if (tag == ASN1_C_EOC)
	    more = FALSE ;
	else
	if IS_CONSTRUCTED( tag ) {
            for (i=ss; i < (ss+INDENT); i++)
                buf[i] = ' ';
            ss += INDENT;
	    print_constructed_avl( avl_handle, ofile, buf, ss ) ;
            ss -= INDENT;
            }
	else
	{
	    sprintf(&buf[ss], "Attribute value: \n" ) ;
            PLOGIT();
	    print_value( tag, octet, ofile, buf, ss ) ; 
	}
	sprintf(&buf[ss], "--------------------------------------\n");
        PLOGIT();
    } while ( (more == TRUE) && (last_one == FALSE) ) ;
    return( last_one ) ;
} /* print_constructed_avl() */


static void
print_value(
	    tag , 
	    octet,
            ofile,
            buf,
            ss
	    )

int          tag ;
octet_string *octet ;
FILE *ofile;                            /* Output File                       */
char            buf[LINEBUFSIZE];       /* Error Message build buffer        */
int             ss;                     /* Error Msg "String Start" in buf[] */

/*
 * Function description:
 *
 *    Prints the value of an octet_string
 *
 * Arguments:
 *
 *    tag                 the tag value for the octet_string
 *    octet               ptr to the octet_string
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

{
    char      *string = ( char * )NULL ;
    object_id *oid = ( object_id * )NULL ;
    man_status return_status ;

    if ( ( tag == ASN1_C_NULL ) || ( octet == NULL ) )
    {
	if ( octet == NULL ) {
	    sprintf(&buf[ss], "        octet pointer is NULL\n" ) ; PLOGIT();
            }
	else {
	    sprintf(&buf[ss], "        octet is ignored\n" ) ;
            PLOGIT();
            }
        return ;
    }
    sprintf(&buf[ss], "octet.data_type = %s\n", get_type(octet->data_type));
    PLOGIT();
    sprintf(&buf[ss], "octet.length    = %d (decimal)\n", octet->length ) ;
    PLOGIT();
    if (octet->length)
    {
	switch ( octet->data_type ) 
	{
	    case ASN1_C_BOOLEAN :
	        if (*((char *)octet->string) == 0) {
		   sprintf(&buf[ss], "octet.string    = FALSE\n" ) ;  PLOGIT();
                   }
		else {
		   sprintf(&buf[ss], "octet.string    = TRUE\n" ) ; PLOGIT();
                   }
		break ;

	    case ASN1_C_NULL :
	        sprintf(&buf[ss], "octet.string    = NULL\n" ) ; PLOGIT();
		break ;

	    case ASN1_C_INTEGER :
	    case INET_C_SMI_COUNTER :
	    case INET_C_SMI_GAUGE :
	    case INET_C_SMI_TIME_TICKS :
	        switch( octet->length )
		{
		    case 0: 
		        sprintf(&buf[ss], "octet.string    = NULL\n" ) ;
                        PLOGIT();
			break ;

		    case 1: 
			sprintf(&buf[ss],
                                "octet.string    = %d\n",
                                *((char *)octet->string)
                                ) ;
                        PLOGIT();
			break ;

		    case 2: 
			sprintf(&buf[ss],
                                "octet.string    = %d\n",
                                *((unsigned short *)octet->string)
                                ) ;
                        PLOGIT();
			break ;

		    case 4: 
			sprintf(&buf[ss],
                                "octet.string    = %d\n",
                                *((unsigned int *)octet->string)
                                ) ;
                        PLOGIT();
			break ;

		    default:  
			sprintf(&buf[ss], "octet.string    = <?>\n" ) ;
                        PLOGIT();
		}
	        break ;

	    case ASN1_C_PRINTABLE_STRING :
                sprintf(&buf[ss],
                        "octet.string    = %.*s\n",
                        octet->length,
                        octet->string
                        );
                PLOGIT();
                break;

	    case ASN1_C_OCTET_STRING : 
	    case INET_C_SMI_IP_ADDRESS :
	    case INET_C_SMI_OPAQUE :
	    {
		int i,j,k ;

                sprintf(&buf[ss], "octet.string    = ");
                k = strlen(buf);

                /* For all the data . . . */
		for ( i = 0 ; i < octet->length;) {

                    /* If the dump is more than one line . . . */
                    if ( i == 8 ) {

                        /* Blank out "octet.string    = " */
                        memset((char *) &buf[ss], ' ', 18);
                        }

                    /*
                    | Zap the dump area to blanks:
                    |       8 blocks of 3 characters,
                    |       1 blank,
                    |       8 characters
                    */
                    memset((char *) &buf[k], ' ', ((8*3)+1+(8*1)));

                    for (j = 0; j < 8 && i < octet->length; j++, i++) {

                        char sbuf[30];

                        memset ( (char *) sbuf, '\0', 30);

                        /* sprintf weirdness:
                        |  dump it to sbuf (where hi-order bit-set
                        |  overflows) and then copy rightmost three characters.
                        */
                        sprintf(sbuf, "%2.2x ", octet->string[i]);

                        /* Load the dump area w/8 bytes of interpretation */
                        sprintf(&buf[k+(j*3)],
                                "%s",
                                (char *) &sbuf[strlen(sbuf)-3]);

                        buf[k+(j*3)+3] = ' ';   /* Blow off null byte */
                        sprintf(&buf[k+26+j],  "%c\n",
                                ((isprint(octet->string[i])) ?
                                  (octet->string[i]) : ('.') ));
                        }

                    PLOGIT();                    /* Print the line */
                    }

		break ;
	    }

	    case ASN1_C_OBJECT_ID :
		return_status = moss_octet_to_oid( octet, &oid ) ;
		return_status = moss_oid_to_text ( oid, ".", "", "", &string ) ;
                if (return_status == MAN_C_SUCCESS) {
		    sprintf(&buf[ss], "octet.string    = %s\n", string );
                    PLOGIT();
		    free( string ) ;
                    }
                else {
                    sprintf(&buf[ss],
                            "octet.string    = <OID conversion failure>\n"
                            );
                    PLOGIT();
                    }
		moss_free_oid( oid ) ;
		break ;

	    default :
	    {
		int i,j,k ;
	        sprintf(&buf[ss],
                        "***unaccounted for type = %s\n",
                        get_type(octet->data_type)
                        );
                PLOGIT();

                sprintf(&buf[ss], "octet.string    = ");
                k = strlen(buf);

                /* For all the data . . . */
		for ( i = 0 ; i < octet->length;) {

                    /* If the dump is more than one line . . . */
                    if ( i == 8 ) {

                        /* Blank out "octet.string    = " */
                        memset((char *) &buf[ss], ' ', 18);
                        }

                    /*
                    | Zap the dump area to blanks:
                    |       8 blocks of 3 characters,
                    |       1 blank,
                    |       8 characters
                    */
                    memset((char *) &buf[k], ' ', ((8*3)+1+(8*1)));

                    for (j = 0; j < 8 && i < octet->length; j++, i++) {

                        char sbuf[30];

                        memset ( (char *) sbuf, '\0', 30);

                        /* sprintf weirdness:
                        |  dump it to sbuf (where hi-order bit-set
                        |  overflows) and then copy rightmost three characters.
                        */
                        sprintf(sbuf, "%2.2x ", octet->string[i]);

                        /* Load the dump area w/8 bytes of interpretation */
                        sprintf(&buf[k+(j*3)],
                                "%s",
                                (char *) &sbuf[strlen(sbuf)-3]);

                        buf[k+(j*3)+3] = ' ';   /* Blow off null byte */
                        sprintf(&buf[k+26+j],  "%c\n",
                                ((isprint(octet->string[i])) ?
                                  (octet->string[i]) : ('.') ));
                        }

                    PLOGIT();                    /* Print the line */
                    }
            }
	}
    }
    else {
        sprintf(&buf[ss], "\n" ) ;
        PLOGIT();
        }

} /* end of print_value */


char *
get_man_status( code )
int code;

/*
 * Function description:
 *
 *    Gets the ASCII-string representation of a man_status
 *
 * Arguments:
 *
 *    code          man_status to display
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */

{
static  char value[100];

    if (code == (int)MAN_C_SUCCESS)
        return( "MAN_C_SUCCESS" ) ;
    else
    if ( (code > (int)MAN_C_SUCCESS) &&
	(code <= (int)MAN_C_FILTER_USED_WITH_CREATE ) )
        return( man_status_table_1[ code ] ) ;
    else
    if ( (code >= (int)MAN_C_WILD_NOT_AT_LOWEST_LEVEL) &&
	(code <= (int)MAN_C_FILTER_TOO_COMPLEX) )
        return( man_status_table_2[ code - 1000 ] ) ;
    else
    if ( (code >= (int)MAN_C_ALREADY_INITIALIZED) &&
	(code <= (int)MAN_C_READ_ONLY) )
        return( man_status_table_3[ code - 1200 ] ) ;
    else
    if ( (code >= (int)MAN_C_EQUAL) &&
	(code <= (int)MAN_C_FALSE) )
        return( man_status_table_4[ code - 1300 ] ) ;
    else
    if ( (code >= (int)MAN_C_NOT_SUPPORTED) &&
	(code <= (int)MAN_C_END_OF_MIB) )
        return( man_status_table_5[ code - 2000 ] ) ;
    else {
        sprintf(value, "<UKNOWN man_status code %d>", code );
        return( value );
        }

}  /* end get_man_status */


static char *
get_type( 
	 tag
         )
unsigned int tag ;

/*
 * Function description:
 *
 *    Gets the ASCII representation of ASN1 value of a supplied ASN1 tag
 *
 * Arguments:
 *
 *    tag             ASN1 tag
 *
 * Return values:
 *
 *    None.
 *
 * Side effects:
 *
 *    None.
 */
{
    static char value[100];    /* "Unknown" message buffer */
    int tagint ;

    tagint = TAG_VALUE( tag ) ;

    if ( IS_UNIVERSAL( tag ) )
    {
	switch ( tag )
	{
	    case ASN1_C_EOC : 
                return ("ASN1_C_EOC" ) ;

	    case ASN1_C_BOOLEAN :
	        return(  "ASN1_C_BOOLEAN" ) ; 

	    case ASN1_C_INTEGER :
	        return(  "ASN1_C_INTEGER" ) ; 

	    case ASN1_C_BITSTRING :  
	        return(  "ASN1_C_BITSTRING" ) ; 

	    case ASN1_C_OCTET_STRING :
	        return(  "ASN1_C_OCTET_STRING" ) ; 

            case ASN1_C_NULL :  
                return(  "ASN1_C_NULL" ) ; 

            case ASN1_C_OBJECT_ID :
                return(  "ASN1_C_OBJECT_ID" ) ; 

            case ASN1_C_PRINTABLE_STRING : 
	        return(  "ASN1_C_PRINTABLE_STRING" ) ; 

            case ASN1_C_SEQUENCE :        
	        return(  "ASN1_C_SEQUENCE" ) ; 

	    case ASN1_C_SET :
	        return(  "ASN1_C_SET" ) ; 

	    default :
                sprintf(value, "UKNOWN universal %d", tag ) ;
	        return( value );
	}
    }
    else
    if ( IS_APPLICATION( tag ) )
    {
	if ( ( tag < INET_C_SMI_IP_ADDRESS ) || ( tag > INET_C_SMI_OPAQUE ) )
	    return(  "UNKNOWN application" ) ;
	else
            return( inet_application_tag_table[ tagint ] ) ;
    }
    else
    if ( IS_CONTEXT_SPECIFIC( tag ) )
    {
        sprintf(value, "UNKNOWN context-specific 0x%x", tagint ) ;
	return( value );
    }
    else {
        return( "UNKNOWN tag type" ) ;
        }
} /* end of get_type */
