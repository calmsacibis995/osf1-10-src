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
static char *rcsid = "@(#)$RCSfile: evd_main.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/08 16:13:38 $";
#endif
/*
 **  Copyright (c) Digital Equipment Corporation, 1993.
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
 *    Common Agent Event Dispatcher (evd process)
 *
 * Module EVD_MAIN.C
 *    Contains main functions for the Common Agent Event Dispatcher process.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks Engineering
 *    D. McKenzie  February 1993    For Common Agent V1.1.
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engine(s) accept requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.  Events
 *       that need to be brought to a management station's attention are 
 *       routed through the event dispatcher to the appropriate protocol
 *       engine (only SNMP-PE for V1.0, V1.1) where they are then forwarded to
 *       the TRAP communities (i.e., IP addresses) defined in SNMP_PE.CONF.
 *
 *    Purpose:
 *       This module contains the main functions for the Event Dispatcher
 *       that is designed to handle event requests from the Common Agent
 *       Managed Object Modules (MOMs), and is responsible for shipping the
 *       event requests to the appropriate Common Agent protocol engine for
 *       eventual shipment to all interested management stations.
 *
 * History
 *      V1.1    Feb 1993    D. McKenzie (stolen from snmppe_main.c).
 *                          Original Version is 1.1 (not 1.0) to coincide with 
 *                          the Common Agent Release 1.1 for which this was 
 *                          new process was created.
 *
 * NOTES:

Module Overview:
---------------

This module contains the main functions for the Common Agent Event Dispatcher
(EVD) and a couple of support functions useful throughout EVD.

File EVD_TEXT.C contains all printable text output for the 'Exxx' messages
used throughout the event dispatcher for diagnostic and debugging use.  If you
modify the text of any message, or add new messages, or delete old messages,
you must:

   (a) Add the new message/modify the existing message/delete the old message
       in/in/from EVD_TEXT.C.  Be sure to follow the format used in
       that module when creating a new text message 'function'.

   (b) Use the 'MSG' macro defined in EVD.H; this macro invokes the
       appropriate emsgXXX() function which you added to EVD_TEXT.C
       in steb (a) above.  Be sure to add/delete the 'extern' for the 
       new/deleted message 'function' at the end of EVD.H.  Perform
       this step ONLY if you are adding a new message, or if you are
       deleting an existing message.  Modifications to existing messages
       require changes to the appropriate function in EVD_TEXT.C only.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (i.e., Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------

main                    Main function for the EVD, executed by the main
                        event dispatcher thread.

evd_log                 Handles logging of diagnoistic and normal operational
                        conditions to a debugging log file or the system log
                        (Used by both threads, log file is mutex protected).
                        (This function usually invoked by macro LOG() or
                        SYSLOG() defined in "evd.h", and is primarily
                        for one-line messages.

evd_log_queue_handle    Handles diagnostic output to a debugging log file
                        (not the system log) of a dump of a event queue
                        handle block.

evd_crash_exit          Handles the details of a crash-shutdown when an
                        unrecoverable error occurs.  (This function usually
                        invoked by macro "CRASH()" defined in "evd.h").

evd_build_line_prefix   Builds the proper ASCII prefix into a output line
                        buffer to be used in a diagnostic dump.


MODULE INTERNAL FUNCTIONS:

Function Name               Synopsis
-------------               --------
evd_print_avl               Interprets an AVL for logging
evd_print_constructed_avl   Interprets a constructed AVL for logging
evd_print_value             Interprets octet string value for logging
evd_get_man_status          Interprets status for logging
evd_get_type                Interprets AVL tag type for logging


Thread Overview:
---------------

EVD can be linked for use with IPC/RPC or it can be linked into the single 
image produced for the Common Agent.

The symbol "NOIPC" controls the conditional compilation as required.

==============================================================================
--EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1--
==============================================================================

When "NOIPC" is NOT defined (i.e., we're using RPC/IPC), EVD V1.1
operates as a client (of the SNMP PE) and as a server to the MOMs (by
offering it's "evd_*" API entry-points as an interface).  In this situation,
EVD becomes (during initialization) a listening thread which performs the
"rpc_server_listen" function to allow EVD to act as a server.  EVD 1.1 
operates as a single threaded process; only one incoming event request is
processed at one time (all other incoming calls are queued by the operating
system for later processing).  SOME code has beed put into place that will
facilitate the addition of multiple threads (#ifdef'd out with the "MTHREADS"
symbol).

Note that more code is required to be added before true "multiple-threaded"
operation can actually be supported.  Search for the symbol "MTHREADS" to
find the places where more code should be added.  Design work should be done
to review C-library functions which may not be thread-safe, including in
at least "inet_ntoa()" which seems to use a static area.

This additional code is activated by defining symbol "MTHREADS".  The value
of this symbol is the number of EXTRA "event-request" threads that
may be started to handle additional inbound simultaneous event requests. 
In other words, if MTHREADS is 1, then EVD can handle 2 inbound event requests
simultaneously.  If MTHREADS is 2, then EVD can handle 3 inbound event requests
simultaneously.  The value of MTHREADS can be thought of as the number of 
inbound event requests beyond the initial inbound event request that can be 
handled simultaneously.

If "MTHREADS" is not defined and "THREADS" is defined, then only the basic
"logically-single-threaded" operation (one event request at a time) is compiled.
(The major difference that occurs when MTHREADS is defined and given a value
in this circumstance is that mutexes are defined and code that initializes
them and references them comes into play.  These code additions are NOT
COMPLETE.)

FOR EVD V1.1, THIS IS THE ONLY CONFIGURATION THAT HAS BEEN TESTED!

The main thread is thought of as the "receiving" thread, as it receives
the Common Agent event requests in the form of an IPC/RPC invocation of a 
function in "evd_recv.c".  The main thread is responsible for initializing 
the EVD process at process start-up time, then the process is "put to sleep" 
to await and subsequently process (to completion) an incoming event request
one at a time.  The "putting to sleep" is performed by the main thread by 
doing an rpc_server_listen() call as the last thing after initialization of the
EVD process has successfully completed.

When synchronization is required between threads (whenever more than one
thread is enabled) for access to shared data-structures or functions, a thread
"mutex" must be used (the required code to perform mutex protection is in 
place in only SOME parts of the code -- NOT ALL).

Aside from this synchronization, the threads should operate independently but 
synchronously.

==============================================================================
--EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1---EVD V1.1--
==============================================================================

Comments in the code that address thread issues are prefaced with the string
"<TC>" for "Thread Comment" (for ease of grepping).

Coding Conventions:
-------------------

* All functions have prototypes.  Static functions have prototypes
  coded into the front of the module in which the function is defined.
  Externally (globally) referenced functions have prototypes in "evd.h".

* Comments in the code that address thread issues are prefaced with the string
  "<TC>" for "Thread Comment" (for ease of grepping).

* Error, Debug and Informational messages formatted according to the
  convention outlined in the documentation for function "evd_log()" are found
  in this module.

* NO (static) GLOBALLY ACCESSIBLE DATA!  All data of this ilk goes into
  the "EVD global data" structure which is passed explicitly to any function
  requiring it.  This permits re-entrancy, recursion (and thread stuff),
  but above all it permits ease of maintenance. (No grepping to find
  where something gets defined/allocated).

*/

/* Module-wide Stuff */


/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
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
#include <ctype.h>
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include "moss.h"
#include "moss_inet.h"
#include "evd.h"
#include "evd_defs.h"


/*---------------------- For Internationalization ---------------------------*/
/* The following includes are required in the main module of a program ONLY. */

#ifdef NL
# include <langinfo.h>
# include <locale.h>
# include <nl_types.h>
#endif

/*---------------------------------------------------------------------------*/

/*
|==============================================================================
|
|   Define Prototypes for Module-Local Functions
|
|==============================================================================
*/

/* evd_print_constructed_avl - Print the contents of a constructed AVL. */
static int
evd_print_constructed_avl PROTOTYPE((
 avl   *,
 FILE  *,
 char  *,                /* Error Message build buffer        */
 int                     /* Error Msg "String Start" in buf[] */
));


/* evd_print_value - Print the value of an octet_string. */
static void
evd_print_value PROTOTYPE((
 int          ,
 octet_string *,
 FILE         *,         /* Output File                       */
 char         *,         /* Error Message build buffer        */
 int                     /* Error Msg "String Start" in buf[] */
));



/* evd_get_type - Get the value of a ASN1 Tag. */
static char *
evd_get_type PROTOTYPE((
 unsigned int
));


/*
|==============================================================================
| EVD Global Data Pointer
|
|   This globally accessible pointer is the exception to the "no global data"
|   rule.  This pointer is loaded by the initilization code thereby making
|   the "global data" structure available to the receiving thread.
|
|   Most of the stuff in this structure is "read-only", things that aren't are
|   protected by mutexes in the structure.
|
|==============================================================================
*/
evd_global_data_t  global_data;         /* All the "global" EVD-wide data    */
evd_global_data_t  *evd_global_dataP;   /* Global Ptr to global data         */



/*
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    main - This routine is the main function for the Common Agent
**           Event Dispatcher process (evd).
**
**  FORMAL PARAMETERS:
**
**    "argc" and "argv" constitute the description of the command line
**    used to start EVD.
**
**    The optional command line arguments have the form:
**
**      [-c <configfilename>]               . . . . 
**        [-d <debugfilename>] [<debuginfo1> [<debuginfo2> ...] ...]
**
**    where:
**
**    -c  optionally introduces the name of the configuration file to use 
**        instead of a builtin file name.  If debugging operation is requested
**        by other options on the command line and this -c argument is not
**        present, then EVD attempts to look up the value of environment
**        variable "SNMPPE_CONFIG_FILE" and uses that value (if present) as
**        the name of the configuration file to be opened instead of the builtin
**        name.
**
**        If this option is absent and debugging operation is not requested by
**        the use of other arguments, EVD attempts to open the configuration
**        file using the builtin name.  See "ca_config.h" for the value of this
**        builtin name.
**
**
**    -d  optionally introduces the name of a file to be used to receive
**        error logging and debug information.  If the file exists, it is
**        opened for appending, otherwise it is created.  Specifying this
**        option constitutes specifying "debugging operation" as mentioned 
**        above in the description of "-c".
**
**        If this option is not specified but other <debuginfo> options are
**        specified on the command line, (signifying "debugging operation")
**        then all debug/error information is routed to "stderr" (including
**        error messages that otherwise would be routed to the system log).
**
**        If neither this option nor any <debuginfo> class options are 
**        specified, then EVD is in "normal operation" and error logging is 
**        done to the system log file.
**
**
**    <debuginfo> optionally specifies a class of debugging information to
**        be routed to the debug/error log file.  The names of these debugging
**        information classes correspond to the values of the symbols defined
**        near the definition of "evd_log_state_t" in "evd.h" WITHOUT the 
**        leading "L_".  Function evd_init_cmdline_args() in evd_init.c parses
**        and recognizes these debugging information message class names.
**
**        Providing one or more of these debug-info class names puts EVD into
**        "debugging operation", causing the absence of the "-c" option to
**        perform differently (see above) and causes all error logging output
**        (that under normal operation is routed to the system log) to be logged
**        to the debug/error output file ("stderr" or file specified by -d
**        above).
**
**        By providing just "SYSLOG" as a debugging class of information 
**        desired, EVD operates in debugging mode producing exactly the same 
**        messages to the debug/error log file as would otherwise be produced 
**        to the system log file under normal operation.  (SYSLOG is always 
**        internally automatically enabled as a message class, specifying it 
**        explicitly simply causes debugging mode to be enabled).
**
**        If more classes are to be added, add them to evd.h, and add the code
**        to initialize them in evd_init.c in function evd_init_cmdline_args().
**
**  OUTPUTS:
**
**    The function never returns (for IPC version).  A controlled crash may be 
**    initiated by a call to function "evd_crash_exit()" (usually via macro 
**    CRASH() ).
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is presumably a shell script that is starting up
**        the Common Agent (for IPC version), or the SNMP PE main/init
**        module for the NOIPC version.
**
**    Purpose:
**        This function causes normal initialization to occur, then "creation"
**        of (i.e., "becoming") a receiving thread for continuous processing of 
**        all received event requests from the Common Agent MOMs.  
**
**        The main thread of the EVD process "becomes" the sole receiving 
**        thread for this initial (V1.1) release of EVD.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**       <load the global reference pointer so receive thread can "see" the 
**        global data>
**       
**       <If internationalizing, open the catalogue file & init runtime locale>
**       <perform all initialization>
**
**   >>ifdef THREADS
**       <"convert" main thread into a pssive (server) thread to handle
**        incoming event requests from the MOMs>
**   >>endif
**
**  OTHER THINGS TO KNOW:
**
**    If we're running in a threaded/RPC/IPC environment, the initialization
**    includes converting the (single) main thread into a "receiving" thread 
**    that does an RPC listen for inbound event requests from the Common Agent
**    MOMs.  See the "<TC>" thread comments below.
**
**---------------------------------------------------------------------------*/

#ifndef NOIPC
   void main (argc, argv)
    int   argc;
    char *argv[];
#else
   void evd_init(argc,argv) /* EVD Init entry point for single image Agent */
    int   argc;
    char *argv[];
#endif /* NOIPC */

{
#ifdef NL
    extern nl_catd _m_catd;
#endif


    /* <TC> In function "main()". . .                                     */
    /* <TC> load the global data reference pointer so the receive thread  */
    /* <TC> can "see" the global data structure containing EVD's context. */

    evd_global_dataP = &global_data; /* also referred to as "Big Picture" */

#ifdef NL

    /* If internationalizing, open the catalogue file & init runtime locale */
    _m_catd = catopen ("evd.cat", NL_CAT_LOCALE);
    setlocale (LC_ALL, "");

#endif

    /* Perform all initialization for the evd process:
    |
    |      * Read in and verify command line arguments
    |      * Open either debug or system log
    |      * Open, parse and check the configuration file
    |      * Log EVD as "up" in system log
    |
    |    The "evd_global_data" structure is initialized and we're ready to 
    |    roll if we return.  If there is an error, we don't come back.
    */
    evd_init_main (&global_data, argc, argv); 

    /* <TC> In function "main()". . .                                       */
    /* <TC> The "main" thread returns from the evd_init_main()              */
    /* <TC> call above, and goes on to loop forever below waiting for and   */
    /* <TC> processing into CA requests each inbound PDU received.  It does */
    /* <TC> this by converting itself into a passive "listening" thread.    */

#ifdef THREADS

    /* EVD V1.1: "Become" a single-threaded server to the Common Agent  */
    /*           MOMs; await and process incoming MOM event requests... */
    evd_init_start_rpc (&global_data);  /* this call never returns...   */

#endif /* THREADS */

} /* end of main(), OR end evd_init() if NOIPC */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_log - Event Dispatcher All-Purpose Logging Routine.
**
**  FORMAL PARAMETERS:
**
**    "log_context" - a structure that is actually part of the "big_picture"
**    containing all the information function "evd_log" needs to direct the 
**    message to the proper sink.
**
**    "msg_class" - The set bit(s) in this integer indicate the debugging
**    class-of-messages that the message belongs to.  "evd_log()" is going to
**    check this category in "log_context" to be sure that logging of messages
**    in this class is enabled before it actually writes the message to
**    the log file.
**
**    "msg" - The string to be placed into the log file (it should NOT have
**    any format specifiers (like "%d") in it).  If it doesn't end in a
**    newline, one is added.
**
**    By coding convention it should start with the letters "Exxx" where
**    "xxx" is a unique decimal number.
**
**    By grepping for "Exxx" in the sources a person should be able to locate
**    where the message is being issued from without cluttering the message
**    with function names.
**
**    "syslog_code" - The extra argument for the actual syslog system function
**    call.  This code is not referenced unless the "msg" is in "msg_class"
**    "L_SYSLOG" (and we're not in a debugging situation).
**
**
**  OUTPUTS:
**
**    The function returns nothing, but baring unrecoverable errors, the
**    specified message has been logged to the current log file if it is in
**    a message class that is enabled for logging.
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is any routine in EVD (after evd_init_main() has been 
**        called) that wishes to log a debug or operational error message to 
**        the current log file.
**
**    Purpose:
**        This is the central function all other EVD functions call to record
**        events that need logging (either debug or normal operation).  
**        New logging classes may be added, and the infrastructure to 
**        accomodate them is already in place; all you need to do is add them 
**        to evd.h (See "Basic Classes", and to add the necessary processing 
**        in evd_init.c to set them up.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      if (message class is enabled for logging)
**
**    >>if MTHREADS
**        if (acquire mutex on log file pointer FAILED)
**            <write message to syslog>
**            <return>
**    >>endif
**
**        if (it doesn't end in a newline and there is room)
**            <add a newline>
**
**        if (debugging mode enabled)
**            <prepend the message class name>
**            <write message to log_state's log file>
**        else
**            <write message to syslog()>
**
**    >>if MTHREADS
**        if (release mutex on log file pointer FAILED)
**            <write message to syslog>
**            <return>
**    >>endif
**
**
**  OTHER THINGS TO KNOW:
**
**    This function is not normally invoked using a direct call to it.  Instead
**    macros "LOG()" and "SYSLOG()" are to be used.  These macros hide some of
**    the parameters making the code simpler to read.  Also macro IFLOGGING() 
**    can be used to conditionalize the invocation of LOG() and SYSLOG() to 
**    avoid the overhead of formatting a complex message.  These macros are 
**    defined in "evd.h".
**
**    Note the coding convention on the message string: it should start "Exxx"
**    where "xxx" is a unique decimal number.
**
**    Message class "L_SYSLOG" is typically always enabled because messages in
**    this class are important messages that need to be logged during normal
**    EVD operation.  This is a convention created by code in evd_init_main()
**    that sets L_SYSLOG in the log_context block at init time.
**
**---------------------------------------------------------------------------*/

void
evd_log (log_context, msg_class, msg, syslog_code)

 evd_log_state_t *log_context;   /*-> info needed to actually log the msg   */
 int              msg_class;     /* Indicates which class "msg" belongs to  */
 char            *msg;           /* The message to be written to log        */
 int              syslog_code;   /* The code needed for syslog call         */
{
    char   bigbuf[MAXERRMSG+1]; /* Where we copy a message to add a newline */
                                /* (+1 is for the NULL byte)                */
    char  *outmsg;              /* Pointer to final message to send         */
    int    in_msg_len;          /* Computed length of inbound message       */

#ifdef __ultrix
    extern void syslog();
#endif

    /* <TC> In function evd_log()...                                        */
    /* <TC> evd_log() may get called at any moment from any of the event    */
    /* <TC> receive threads.  We take no chances of stepping on our         */
    /* <TC> toes by acquiring a mutex to cover whatever the file pointer is.*/
    /* <TC> If normal operations are underway, "syslog()" is used and the   */
    /* <TC> file pointer in "log_state" is not used, although the mutex IS  */
    /* <TC> used anyway.  The only ugliness comes if an attempt to acquire  */
    /* <TC> or release the mutex fails.  In this instance a message is      */
    /* <TC> unconditionally logged to syslog() and we return, hoping for    */
    /* <TC> the best!                                                       */

    /* if (message class is enabled for logging) */
    if ( (log_context->log_classes & msg_class) != 0) 
    {

#ifdef MTHREADS
        /* if (acquire mutex on file pointer FAILED) */
        if (pthread_mutex_lock (&log_context->log_file_m) != 0) 
        {
            /* write message to syslog */
            syslog (LOG_ERR, MSG(emsg025, "E025 - mutex lock failed in evd_log(): %m"));
            return;
        }
#endif

        /* if (it doesn't end in a newline and there is room) */
        if ( ( (in_msg_len = strlen(msg)) <  MAXERRMSG ) &&
             ( *(msg + in_msg_len - 1)    != '\n'     )     )
        {
            /* add a newline */
            strcpy(bigbuf, msg);          /* copy inbound message            */
            bigbuf[in_msg_len] = '\n';    /* add the '\n' over the null byte */
            bigbuf[in_msg_len+1] = '\0';  /* add the '\0'                    */
            outmsg = bigbuf;              /* New outbound message is here    */
        }
        else 
        {
            outmsg = msg;   /* Just use the original message */
        }

        /* if (debugging mode enabled) */
        if (log_context->debugging_mode == TRUE) 
        {
            char biggerbuf[MAXERRMSG+1+12]; /* Where we copy msg to add class */
            int  i;                         /* Loop index */

            evd_build_line_prefix (log_context, msg_class, biggerbuf, &i);

            /* load msg to message build buffer */
            strcpy (&biggerbuf[i], outmsg);

            /* write message to log_state's log file */
            fprintf (log_context->log_file, biggerbuf);
        }
        else 
        {
            /* write message to syslog() */
            syslog (syslog_code, outmsg);
        }

#ifdef MTHREADS
        /* if (release mutex on file pointer FAILED) */
        if (pthread_mutex_unlock (&log_context->log_file_m) != 0) 
        {
            /* write message to syslog */
            syslog (LOG_ERR, MSG(emsg026, "E026 - mutex unlock failed in evd_log(): %m"));
            return;
        }
#endif
    }

} /* end of evd_log() */


/* 
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_log_queue_handle - Log an interpretation of an Event Queue Handle 
**                           to Log File.
**
**  INPUTS:
**
**    bp 
**      Points to the Big Picture structure containing all the "global"
**      context information needed by EVD to operate.
**
**    msg_class
**      The set bit in this integer indicates the debugging
**      class-of-messages that the dump message belongs to.  This determines
**      the heading that accompanies the dump of the event queue handle.
**
**    handle 
**      The address of an Event Queue Handle that contains the 
**      information associated with a particular event queue.
**
**
**  OUTPUTS:
**
**    The function returns nothing, but baring unrecoverable errors, an
**    interpreted dump of the specified event queue handle is logged to the 
**    current log file.  Note that unlike "evd_log()", this function ALWAYS 
**    writes to the log file, (never the system log) and it is only called when 
**    EVD is running in "debug" mode.
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is any routine in EVD (after evd_init_main() has
**        been called) that wishes to log an interpretation of a service
**        block to the current log file.
**
**    Purpose:
**        This primarily supports the one and only class of diagnostic
**        dump, providing interpreted description of the current values of the 
**        fields of an event queue handle block.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      if (event queue handle block address is NULL)
**          <SYSLOG "Exxx - NULL Event Queue Handle block addr given for log">
**          <return>
**
**      <build message class name as output buffer line prefix>
**
**    >>if MTHREADS
**      if (acquire mutex on file pointer FAILED)
**          <write message to syslog>
**          <return>
**    >>endif
**
**      <interpret "check value">
**      <interpret "access mode">
**      <interpret "self_handle">
**
**    >>if MTHREADS
**      if (release mutex on file pointer FAILED)
**          <write message to syslog>
**          <return>
**    >>endif
**
**
**  OTHER THINGS TO KNOW:
**
**    This function doesn't use the "Exxx" form of message.
**
**--------------------------------------------------------------------------*/

void
evd_log_queue_handle (bp, msg_class, handle)

 evd_global_data_t    *bp;        /*-> The Big Picture                       */
 int                   msg_class; /* Indicates class dump "msg" belongs to   */
 event_queue_handle_t *handle;    /*-> Event queue handle Block to be dumped */

#define INDENT 4
#define LOGIT() fprintf(bp->log_state.log_file, buf)

{
    char            buf[LINEBUFSIZE];   /* Error Message build buffer        */
    int             ss;                 /* Error Msg "String Start" in buf[] */
    int             i;                  /* Handy index                       */
    char            *oid_string;        /* -> Converted-to-Text OID          */


    /* if (service block address is NULL) */
    if (handle == NULL) 
    {
        SYSLOG(LOG_ERR, MSG(emsg027, "E027 - NULL Event Queue Handle block address given for log"));
        return;
    }

    /* build message class name as output buffer line prefix */
    evd_build_line_prefix (&bp->log_state, msg_class, buf, &ss);

#ifdef MTHREADS
    /* if (acquire mutex on file pointer FAILED) */
    if (pthread_mutex_lock (&log_context->log_file_m) != 0) {
        /* write message to syslog */
        syslog (LOG_ERR, MSG(emsg028, "E028 - mutex lock failed in log_queue_service(): %m"));
        return;
    }
#endif

    /* interpret "check_value" */
    /* interpret "access_mode" */
    /* interpret "self_handle" */

    sprintf (&buf[ss],
             "EVENT QUEUE HANDLE %lx: check_value '0x%x'  access_mode '%d'  self_handle '0x%lx'\n",
             handle, handle->check_value, handle->access_mode,
             handle->self_handleP );
    LOGIT();

#ifdef MTHREADS
    /* if (release mutex on file pointer FAILED) */
    if (pthread_mutex_unlock(&log_context->log_file_m) != 0) 
    {
        /* write message to syslog */
        syslog(LOG_ERR, MSG(emsg029, "E029 - mutex unlock failed in log_queue_service(): %m"));
        return;
    }
#endif

} /* end of evd_log_queue_handle() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_crash_exit - Event Dispatcher Crash-Shutdown Routine.
**
**  FORMAL PARAMETERS:
**
**    bp
**      Pointer to the global "big picture" structure.
**
**    msg 
**      The string to be placed into syslog (it should NOT have any format 
**      specifiers (like "%d") in it).  By coding convention it should
**      start with the letters "Exxx" where "xxx" is a unique decimal number.
**      By grepping for "Exxx" in the sources a person should be able to locate
**      where the message is being issued from.  Thus we avoid cluttering the
**      message with function names.
**
**      evd_crash_exit() appends the phrase "--unrecoverable: evd exiting" 
**      to the end of the message before logging it.
**
**  OUTPUTS:
**
**    The function returns nothing, but causes the complete shutdown
**    of all EVD threads after the specified message has been logged to the
**    system log.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is any routine in EVD (after evd_init_main() has
**        been called) that has encountered a fatal unrecoverable error.
**
**    Purpose:
**        This is the central function all other EVD functions call to cause
**        EVD to exit in an error situation.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**        <if possible, build copy of the message w/termination phrase at end>
**        <log the message to syslog>
**
**    >>if MTHREADS
**        <signal other threads to exit?>
**        <shutdown RPC/IPC>
**    >>endif
**
**        if (debugging mode NOT enabled)
**            <close syslog>
**        else
**            <close the logging file>
**        
**        <call exit()>
**
**  OTHER THINGS TO KNOW:
**
**    Note the coding convention on the message string: it should start "Exxx"
**    where "xxx" is a UNIQUE decimal number.
**
**    Adherence to this coding convention allows easy discovery via grep
**    of the place in the sources where any given error message is issued.
**    This is not by accident.
**
**--------------------------------------------------------------------------*/

void
evd_crash_exit (bp, msg)

 evd_global_data_t *bp;   /*-> The Big Picture                       */
 char              *msg;  /* The error msg to be written to syslog   */

#define TERM_PHRASE "--unrecoverable: evd exiting\n"

{
    char   bigbuf[MAXERRMSG+1]; /* Where msg w/extra phrase is copied and blt */
                                /* (+1 is for null byte)                      */
    extern void closelog();


    /* if possible, build copy of message w/termination phrase at the end */
    bigbuf[MAXERRMSG] = '\0';               /* Safety termination  */
    strncpy (bigbuf, msg, MAXERRMSG);        /* Copy in the message */
    
    /* If there is room left in the buffer for the entire phrase... */
    if ( ( MAXERRMSG - strlen(bigbuf)) > strlen(TERM_PHRASE)) 
    {
        strcpy (&bigbuf[strlen(bigbuf)], TERM_PHRASE); /*  ...then Add it */
    }

    /* log the message to syslog */
    SYSLOG(LOG_ERR, bigbuf);

/*
---if MTHREADS
    <signal other threads to exit>

    NOTE: Some sort of interlock should probably be established to preclude
          more than one thread being in this crash function at once.

          Note that you don't know whether a sending or receiving thread has
          entered this function.

          The "close" done below might be crunching an I/O operation initiated
          on it by another thread.  This has to be precluded somehow.

    <shut down RPC/IPC>--it could be an RPC/IPC-initiated thread that has 
                       --entered this function.
---endif
*/

    /* if (debugging mode NOT enabled) */
    if (bp->log_state.debugging_mode == FALSE)
        closelog ();
    else /* close the logging file */
        fclose (bp->log_state.log_file);

    exit(0);

} /* end of evd_crash_exit() */

/*
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_build_line_prefix - Builds ASCII prefix into Output Line Buffer.
**
**  FORMAL PARAMETERS:
**
**    log_context 
**      This is the logging context information block from the Big
**      Picture that contains all info about logging (including pointers to
**      the message class names).
**
**    msg_class
**      This is an integer containing the bit(s) set corresponding
**      to the message class whose name must be placed into...
**
**    dmpbuf 
**      This is a character array being used is an output line buffer in
**      a diagnostic dump.
**
**    ss 
**      This is an integer that is set up on return to point to the null byte
**      which is placed into the dmpbuf[] array at the end of the prefix.  This
**      corresponds to where the caller can continue to build a diagnostic line
**      destined for output.
**
**
**  OUTPUTS:
**
**    The function returns nothing, but causes the ASCII string describing
**    the message class specified by "msg_class" bit mask" to be loaded into
**    the beginnig of "dmpbuf[]".  If "msg_class" is somehow invalid, you'll
**    get "MSGCLASS?" as the prefix.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is any code that is busy formatting a diagnostic output
**        line.
**
**    Purpose:
**        This function takes care of the conversion of a bit-mask specifying
**        a message class into the ASCII name of that message class.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      for (each possible message class)
**          if (class matches passed class)
**             <select pointer to class name string>
**             <break>
**
**      if (no message class selected)
**          <use "MSGCLASS?">
**
**      <load name padded to PREFIXSIZE characters into output buffer>
**      <null terminate the prefix>
**      <return "start-string" index value>
**
**  
**  OTHER THINGS TO KNOW:
**
**    Note that the prefix is null-terminated, and "ss" is set to the index
**    in dmpbuf[] that corresponds to this null-byte.  "ss" stands for
**    "start-string" which is where the caller starts building any new string
**    to go into the output buffer.
**
**  MTHREADS
**    If multiple-threads is implemented, this function should probably be
**    enhanced to fetch a thread-id of some sort and build it into the
**    prefix that it builds.  In this way it'll be possible to trace which
**    threads are issuing messages.
**
**--------------------------------------------------------------------------*/

void
evd_build_line_prefix (log_context, msg_class, dmpbuf, ss)

 evd_log_state_t *log_context;   /*-> log info needed for logging class      */
 int              msg_class;     /* Bit Pattern flag for message class       */
 char             dmpbuf[];      /* Output Line buffer for prefix            */
 int             *ss;            /* Start String position (index into dmpbuf */
{
    char    *msg_class_name=NULL;   /* -> Message Class name in log_state */
    int     i;                      /* Handy index                        */

    /* for (each possible message class) */
    for (i=0; i < LOG_CLASS_COUNT; i++) 
    {
        /* if (class matches passed class) */
        if (log_context->dbg_flag[i] == msg_class) 
        {
            /* select class name */
            msg_class_name = log_context->dbg_name[i];
            break;
        }
    }

    /* if (no message class selected) */
    if (msg_class_name == NULL) 
        msg_class_name = "MSGCLASS?"; /* set selected class name to "Unknown" */

    /* load padded name to message build buffer */
    strcpy (dmpbuf, msg_class_name);
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

} /* end of evd_build_line_prefix() */


/*
**+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_oid_text - Convert binary OID to text string.
**
**  FORMAL PARAMETERS:
**
**
**    oid 
**      This is a pointer to an Object-ID structure whose textual 
**      representation is desired.
**
**  OUTPUTS:
**
**    The function calls the standard MOSS routine to obtain a converted
**    string representation.  If the conversion fails, a string indicating
**    this is still returned.  The caller must free the storage.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the trace/debugging code typically.
**
**    Purpose:
**        This function takes care of returning a string representation
**        of an OID in an easy way.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**        if (oid pointer is NULL)
**            <set bad string to "<NULL PTR TO OID STRUCT>"
**        else if (no arcs in oid)
**            <set bad string to "<NO ARCS IN OID STRUCT>"
**        else 
**            <attempt OID conversion>
**            if (conversion failed)
**                <set bad string to "<OID Text Convert Failed>">
**
**        if (conversion failed)
**            if (attempt to allocate storage for message failed)
**                <return NULL>
**            <copy message to storage>
**            <return storage address>
**
**        <return converted OID text representation>
**
** 
**  OTHER THINGS TO KNOW:
**
**    
**--------------------------------------------------------------------------*/

char *
evd_oid_text (oid)

 object_id    *oid;   /*--> The Object Identifier to be printed */

#define OID_ERR_STRING "<OID Text Convert Failed>"

{
    char    *oid_string=NULL;    /* Where we store ptr to string to return */
    char    *bad_string=NULL;    /* Error string to use                    */


    /* if (oid pointer is NULL) */
    if (oid == NULL) 
    {
        /* set bad string to "<NULL PTR TO OID STRUCT>" */
        bad_string = "<NULL PTR TO OID STRUCT>";
    }
    else if (oid->count == 0 || oid->value == NULL)
    {
        /* no arcs in oid */
        /* set bad string to "<NO ARCS IN OID STRUCT>" */
        bad_string = "<NO ARCS IN OID STRUCT>";
    }
    else 
    {
        /* attempt OID conversion */
        moss_oid_to_text (oid, NULL, NULL, NULL, &oid_string);

        /* if (conversion failed) */
        if (oid_string == NULL) 
        {
            /* set bad string to "<OID Text Convert Failed>" */
            bad_string = "<OID Text Convert Failed>";
        }
    }

    /* If (conversion failed ) */
    if (bad_string != NULL) 
    {
        /* if (attempt to allocate storage for message failed) */
        if ((oid_string = (char *) malloc (strlen(bad_string) + 1)) == NULL) 
            return (NULL);

        /* copy standard message to storage */
        strcpy (oid_string, bad_string);
    
        /* return storage address */
    }

    /* return converted OID text representation */
    return (oid_string);

} /* end of evd_oid_text() */

/****************************************************************************
 *                                                                          *
 *  (C) DIGITAL EQUIPMENT CORPORATION 1990,1993                             *
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
 *                    evd_print_avl()
 *                    evd_print_constructed_avl()
 *                    evd_print_value()
 *                    evd_get_man_status()
 *                    evd_get_type()
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
 *                                 fit into EVD's logging system.
 *    D. McKenzie - February 1993, stolen from SNMP Protocol Engine.  Modified
 *                                 ONLY the function names so that there
 *                                 isn't a linking collision for the single
 *                                 image (e.g., NOIPC) version.
 *
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
char *evd_man_status_table_1[] = {
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
char *evd_man_status_table_2[] = {
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
char *evd_man_status_table_3[] = {
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
char *evd_man_status_table_4[] = {
    "MAN_C_EQUAL" ,
    "MAN_C_TRUE" ,
    "MAN_C_NOT_EQUAL" ,
    "MAN_C_FALSE"
} ;

static
char *evd_man_status_table_5[] = {
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


/*
 * Function description:
 *
 *    evd_print_avl - Prints the contents of an AVL.
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

void
evd_print_avl (avl_handle, ofile, buf, ss)
 avl *avl_handle;
 FILE *ofile;                        /* Output File                       */
 char            buf[LINEBUFSIZE];   /* Error Message build buffer        */
 int             ss;                 /* Error Msg "String Start" in buf[] */

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

    if (avl_handle == NULL) 
    {
        sprintf (&buf[ss], "NULL AVL\n");
        PLOGIT();
        return;
    }

    if ((stat = moss_avl_reset (avl_handle)) != MAN_C_SUCCESS) 
    {           
	sprintf (&buf[ss], "Error on moss_avl_reset() ", stat);
        PLOGIT();
        return;
    }

    while ( last_one != TRUE )
    {
     	if ((stat = moss_avl_point (avl_handle,
	                            &oid, &modifier, &tag,
				    &octet, &last_one)) != MAN_C_SUCCESS)
	{
	    if (stat == MAN_C_NO_ELEMENT)
            {
		sprintf (&buf[ss],  "----AVL has no elements----\n");
                PLOGIT();
            }
	    else 
            {
                sprintf (&buf[ss], "Error on moss_avl_point() ", stat);
                PLOGIT();
            }
	    return;
	}

	sprintf (&buf[ss], "---------------------------------------\n");
        PLOGIT();
	if ( oid )
	{
	    stat = moss_oid_to_text( oid, ".", "", "", &string);
	    if ( MAN_C_SUCCESS == stat )
	    {
		sprintf (&buf[ss],  "attribute id =   %s\n", string);
                PLOGIT();
		free (string);
	    }
	    else
            {
	        sprintf (&buf[ss],  "attribute id =   NULL\n");
                PLOGIT();
            }
	}
	else 
        {
	    sprintf (&buf[ss], "attribute id =   NULL\n");
            PLOGIT();
        }

	sprintf (&buf[ss],
                 "Modifier =       %s(%d)\n",
                 evd_get_man_status(modifier),
                 modifier);
        PLOGIT();

	sprintf (&buf[ss],  "Attribute type = %s\n", evd_get_type(tag) );
        PLOGIT();

	if IS_CONSTRUCTED( tag ) 
        {
            for (i=ss; i < (ss + INDENT); i++)
                buf[i] = ' ';
            ss += INDENT;
	    last_one = evd_print_constructed_avl (avl_handle, ofile, buf, ss);
            ss -= INDENT;
            }
	else
	{
	    sprintf (&buf[ss],  "Attribute value: \n");
            PLOGIT();
            for (i=ss; i < (ss + 17); i++)
                buf[i] = ' ';
	    evd_print_value (tag, octet, ofile, buf, (ss + 17)); 
	}

	sprintf (&buf[ss], "--------------------------------------\n");
        PLOGIT();
    }
} /* end evd_print_avl() */


/*
 * Function description:
 *
 *    evd_print_constructed_avl - Prints the contents of a constructed AVL.
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

static int
evd_print_constructed_avl (avl_handle, ofile, buf, ss)
 avl  *avl_handle;
 FILE *ofile;
 char  buf[LINEBUFSIZE];       /* Error Message build buffer        */
 int   ss;                     /* Error Msg "String Start" in buf[] */
{
    int               last_one;
    int               more;
    object_id        *oid;
    octet_string     *octet;
    unsigned int      modifier;
    unsigned int      tag;
    int               i;
    char             *string;
    man_status        stat;

    sprintf (&buf[ss], "IS CONSTRUCTED:\n");
    PLOGIT();
    more = TRUE;

    do
    {
	if ( (stat = moss_avl_point (avl_handle,
    	                             &oid, &modifier, &tag,
				     &octet, &last_one)) != MAN_C_SUCCESS)
	{
	    if (stat == MAN_C_NO_ELEMENT) 
            {
		sprintf (&buf[ss], "----AVL has no elements----\n");
                PLOGIT();
            }
	    else 
            {
	        sprintf (&buf[ss], "Error on moss_avl_point() ", stat);
                PLOGIT();
            }
	    return (TRUE);
	}

	sprintf (&buf[ss],"--------------------------------------\n");
        PLOGIT();

	if ( oid != NULL )
	{
	    stat = moss_oid_to_text ( oid, ".", "", "", &string);
	    if ( MAN_C_SUCCESS == stat )
	    {
		sprintf (&buf[ss], "attribute id  =  %s\n", string);
                PLOGIT();
		free (string);
	    }
	    else 
            {
	        sprintf (&buf[ss], "attribute id = NULL\n");
                PLOGIT();
            }
	}
	else 
        {
	    sprintf (&buf[ss], "attribute id = NULL\n");
            PLOGIT();
        }

	sprintf (&buf[ss], "Attribute type = %s\n", evd_get_type(tag) );
        PLOGIT();

	if (tag == ASN1_C_EOC)
	    more = FALSE ;

	else if IS_CONSTRUCTED( tag ) 
        {
            for (i=ss; i < (ss+INDENT); i++)
                buf[i] = ' ';
            ss += INDENT;
	    evd_print_constructed_avl (avl_handle, ofile, buf, ss);
            ss -= INDENT;
        }
	else
	{
	    sprintf (&buf[ss], "Attribute value: \n");
            PLOGIT();
	    evd_print_value (tag, octet, ofile, buf, ss ); 
	}

	sprintf (&buf[ss], "--------------------------------------\n");
        PLOGIT();

    } while ( (more == TRUE) && (last_one == FALSE) );

    return (last_one);

} /* end evd_print_constructed_avl() */


/*
 * Function description:
 *
 *    evd_print_value - Prints the value of an octet_string.
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

static void
evd_print_value (tag, octet, ofile, buf, ss)
 int           tag;
 octet_string *octet;
 FILE         *ofile;               /* Output File                       */
 char          buf[LINEBUFSIZE];    /* Error Message build buffer        */
 int           ss;                  /* Error Msg "String Start" in buf[] */
{
    char      *string = (char *) NULL;
    object_id *oid = (object_id *) NULL;
    man_status return_status ;

    if (( tag == ASN1_C_NULL ) || ( octet == NULL ))
    {
	if (octet == NULL) 
        {
	    sprintf (&buf[ss], "octet pointer is NULL\n"); 
            PLOGIT();
        }
	else 
        {
	    sprintf (&buf[ss], "octet is ignored (tag=ASN1_C_NULL) \n");
            PLOGIT();
        }
        return;
    }

    sprintf (&buf[ss], "octet.data_type = %s\n", 
             evd_get_type(octet->data_type));
    PLOGIT();
    sprintf (&buf[ss], "octet.length    = %d (decimal)\n", octet->length);
    PLOGIT();

    if (octet->length)
    {
	switch (octet->data_type) 
	{
	    case ASN1_C_BOOLEAN:
	        if (*((char *)octet->string) == 0) 
                {
		   sprintf (&buf[ss], "octet.string    = FALSE\n" );
                   PLOGIT();
                }
		else
                {
		   sprintf (&buf[ss], "octet.string    = TRUE\n" );
                   PLOGIT();
                }
		break;

	    case ASN1_C_NULL:
	        sprintf (&buf[ss], "octet.string    = NULL\n" );
                PLOGIT();
		break;

	    case ASN1_C_INTEGER:
	    case INET_C_SMI_COUNTER:
	    case INET_C_SMI_GAUGE:
	    case INET_C_SMI_TIME_TICKS:
	        switch (octet->length)
		{
		    case 0: 
		        sprintf (&buf[ss], "octet.string    = NULL\n");
                        PLOGIT();
			break;

		    case 1: 
			sprintf (&buf[ss], "octet.string    = %d\n",
                                 *((char *)octet->string));
                        PLOGIT();
			break;

		    case 2: 
			sprintf (&buf[ss], "octet.string    = %d\n",
                                 *((unsigned short *)octet->string));
                        PLOGIT();
			break;

		    case 4: 
			sprintf (&buf[ss], "octet.string    = %d\n",
                                 *((unsigned int *)octet->string));
                        PLOGIT();
			break;

		    default:  
			sprintf (&buf[ss], "octet.string    = <?>\n");
                        PLOGIT();
		}
	        break ;

	    case ASN1_C_PRINTABLE_STRING:
                sprintf (&buf[ss], "octet.string    = %.*s\n", octet->length,
                         octet->string);
                PLOGIT();
                break;

	    case ASN1_C_OCTET_STRING: 
	    case INET_C_SMI_IP_ADDRESS:
	    case INET_C_SMI_OPAQUE:
	    {
		int i,j,k;

                sprintf (&buf[ss], "octet.string    = ");
                k = strlen (buf);

                /* For all the data . . . */
		for (i = 0 ; i < octet->length;) 
                {
                    /* If the dump is more than one line . . . */
                    if (i == 8) 
                    {
                        /* Blank out "octet.string    = " */
                        memset ((char *) &buf[ss], ' ', 18);
                    }

                    /*
                    | Zap the dump area to blanks:
                    |       8 blocks of 3 characters,
                    |       1 blank,
                    |       8 characters
                    */
                    memset ((char *) &buf[k], ' ', ((8*3)+1+(8*1)));

                    for (j = 0; j < 8 && i < octet->length; j++, i++) 
                    {
                        char sbuf[30];

                        memset ((char *) sbuf, '\0', 30);

                        /* sprintf weirdness:
                        |  dump it to sbuf (where hi-order bit-set
                        |  overflows) and then copy rightmost three characters.
                        */
                        sprintf (sbuf, "%2.2x ", octet->string[i]);

                        /* Load the dump area w/8 bytes of interpretation */
                        sprintf (&buf[k+(j*3)], "%s",
                                 (char *) &sbuf[strlen(sbuf)-3]);

                        buf[k+(j*3)+3] = ' ';   /* Blow off null byte */
                        sprintf (&buf[k+26+j],  "%c\n",
                                 ((isprint(octet->string[i])) ?
                                  (octet->string[i]) : ('.') ));
                    }

                    PLOGIT();                    /* Print the line */
                }
		break ;
	    }

	    case ASN1_C_OBJECT_ID:
		return_status = moss_octet_to_oid (octet, &oid);
		return_status = moss_oid_to_text (oid, ".", "", "", &string);

                if (return_status == MAN_C_SUCCESS) 
                {
		    sprintf (&buf[ss], "octet.string    = %s\n", string);
                    PLOGIT();
		    free (string);
                }
                else 
                {
                    sprintf (&buf[ss],
                             "octet.string    = <OID conversion failure>\n");
                    PLOGIT();
                }

		moss_free_oid (oid);
		break;

	    default:
	    {
		int i,j,k;

	        sprintf (&buf[ss], "***unaccounted for type = %s\n",
                         evd_get_type(octet->data_type));
                PLOGIT();

                sprintf (&buf[ss], "octet.string    = ");
                k = strlen(buf);

                /* For all the data . . . */
		for (i = 0 ; i < octet->length;) 
                {
                    /* If the dump is more than one line... */
                    if ( i == 8 ) 
                    {
                        /* Blank out "octet.string    = " */
                        memset ((char *) &buf[ss], ' ', 18);
                    }

                    /*
                    | Zap the dump area to blanks:
                    |       8 blocks of 3 characters,
                    |       1 blank,
                    |       8 characters
                    */
                    memset ((char *) &buf[k], ' ', ((8*3)+1+(8*1)));

                    for (j = 0; j < 8 && i < octet->length; j++, i++) 
                    {
                        char sbuf[30];

                        memset ((char *) sbuf, '\0', 30);

                        /* sprintf weirdness:
                        |  dump it to sbuf (where hi-order bit-set
                        |  overflows) and then copy rightmost three characters.
                        */
                        sprintf (sbuf, "%2.2x ", octet->string[i]);

                        /* Load the dump area w/8 bytes of interpretation */
                        sprintf (&buf[k+(j*3)], "%s",
                                 (char *) &sbuf[strlen(sbuf)-3]);

                        buf[k+(j*3)+3] = ' ';   /* Blow off null byte */
                        sprintf (&buf[k+26+j],  "%c\n",
                                 ((isprint(octet->string[i])) ?
                                  (octet->string[i]) : ('.') ));
                    }

                    PLOGIT();                    /* Print the line */
                }
            }
	}
    }
    else 
    {
        sprintf (&buf[ss], "\n" ) ;
        PLOGIT();
    }
} /* end of evd_print_value */


/*
 * Function description:
 *
 *   evd_get_man_status - Gets the ASCII-string representation of a man_status.
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

char *
evd_get_man_status (code)
 int code;
{
    static  char value[100];

    if (code == (int) MAN_C_SUCCESS)
        return ("MAN_C_SUCCESS");

    else if ( (code >  (int) MAN_C_SUCCESS) &&
              (code <= (int) MAN_C_FILTER_USED_WITH_CREATE ))
            return (evd_man_status_table_1[ code ] );
    else if ( (code >= (int) MAN_C_WILD_NOT_AT_LOWEST_LEVEL) &&
              (code <= (int) MAN_C_FILTER_TOO_COMPLEX) )
            return (evd_man_status_table_2[ code - 1000 ] );
    else if ( (code >= (int) MAN_C_ALREADY_INITIALIZED) &&
              (code <= (int) MAN_C_READ_ONLY) )
            return (evd_man_status_table_3[ code - 1200 ] );
    else if ( (code >= (int) MAN_C_EQUAL) &&
              (code <= (int) MAN_C_FALSE) )
            return (evd_man_status_table_4[ code - 1300 ] );
    else if ( (code >= (int) MAN_C_NOT_SUPPORTED) &&
              (code <= (int) MAN_C_END_OF_MIB) )
            return (evd_man_status_table_5[ code - 2000 ] );
    else 
    {
        sprintf(value, "<UKNOWN man_status code %d>", code );
        return( value );
    }

}  /* end evd_get_man_status */


/*
 * Function description:
 *
 *    evd_get_type - Gets the ASCII representation of ASN1 value of a 
 *                   supplied ASN1 tag.
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

static char *
evd_get_type (tag)
 unsigned int tag ;
{
    static char value[100];    /* "Unknown" message buffer */
    int tagint;

    tagint = TAG_VALUE( tag );

    if ( IS_UNIVERSAL( tag ))
    {
	switch (tag)
	{
	    case ASN1_C_EOC:
                return ("ASN1_C_EOC");

	    case ASN1_C_BOOLEAN :
	        return ("ASN1_C_BOOLEAN"); 

	    case ASN1_C_INTEGER :
	        return ("ASN1_C_INTEGER"); 

	    case ASN1_C_BITSTRING :  
	        return ("ASN1_C_BITSTRING"); 

	    case ASN1_C_OCTET_STRING :
	        return ("ASN1_C_OCTET_STRING"); 

            case ASN1_C_NULL :  
                return ("ASN1_C_NULL"); 

            case ASN1_C_OBJECT_ID :
                return ("ASN1_C_OBJECT_ID"); 

            case ASN1_C_PRINTABLE_STRING : 
	        return ("ASN1_C_PRINTABLE_STRING"); 

            case ASN1_C_SEQUENCE :        
	        return ("ASN1_C_SEQUENCE"); 

	    case ASN1_C_SET:
	        return ("ASN1_C_SET"); 

	    default:
                sprintf (value, "UKNOWN universal %d", tag);
	        return (value);
	}
    }

    else if ( IS_APPLICATION( tag ))
    {
	if ( (tag < INET_C_SMI_IP_ADDRESS) || (tag > INET_C_SMI_OPAQUE) )
	    return ("UNKNOWN application");
	else
            return (inet_application_tag_table[tagint] );
    }

    else if ( IS_CONTEXT_SPECIFIC( tag ))
    {
        sprintf (value, "UNKNOWN context-specific 0x%x", tagint);
	return (value);
    }

    else 
    {
        return ("UNKNOWN tag type");
    }
} /* end of evd_get_type */

