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
static char *rcsid = "@(#)$RCSfile: evd_init.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:30:52 $";
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
 *
 *    Common Agent Event Dispatcher (evd).
 *
 * Module EVD_INIT.C
 *    Contains initialization functions for the Common Agent Event Dispatcher
 *    process.
 *
 * WRITTEN BY:
 *    Enterprise Management Frameworks Engineering
 *    D. McKenzie   February 1993, initialliy for Common Agent V1.1
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engine(s) accept requests over a network,
 *       converts incoming information into standard Common Agent format
 *       and calls the Common Agent for processing of that request.  Events
 *       that need to be brought to a management station's attention are 
 *       routed through the event dispatcher to the appropriate protocol
 *       engine (only SNMP-PE for V1.1) where they are then forwarded to
 *       the TRAP communities (i.e., IP addresses) defined in SNMP_PE.CONF.
 *
 *    Purpose:
 *       This module contains the initialization functions for the EVD.
 *       When "evd_init_main()" returns, EVD is ready to become a "server"
 *       thread that can accept inbound event report function calls from MOMs.
 *
 * History
 *      V1.1    February 1993           D. McKenzie
 *
Module Overview:
---------------

This module contains the initialization function(s) for the Event Dispatcher.


Thread Overview:
---------------

All the functions in this module including "evd_init_main()" are executed
exclusively by the single main thread at start-up time.  upon successful
initialization, the main thread "converts" itself into the receiving (server)
thread; this "conversion" is performed in main() (evd_main.c) as its last task.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (i.e., Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------

evd_init_main           The "main" initialization function, called by the
                        EVD main() function, "evd_init_main()" dispatches
                        to functions internal to this module to accomplish
                        the work of initializing EVD.  This consists
                        largely of filling in cells in the global data
                        structure which is passed to all important routines 
                        in EVD (called from within EVD_MAIN.C).

evd_init_start_rpc      Start threads doing an RPC listen for server interface
                        (called from within EVD_MAIN.C).


MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------
evd_init_open_log       Initialize our logging sink according to cmdline
                        arguments now in Big Picture.

evd_init_config         Open and Parse the SNMP-PE Configuration File, 
                        initializing the EVD global data structure accordingly.

evd_init_cmdline_args   Parse command line arguments into the EVD Global
                        Data Structure ("Big Picture").

evd_init_next_cmdarg    Fetch next command line argument as a string (for use
                        exclusively by evd_init_cmdline_args() ). 

evd_init_init_rpc       Initialize RPC for server entry points (only if THREADS)

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
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

/* includes required for "evd.h" */
#include "moss.h"
#include "moss_inet.h"
#include "ca_config.h"  /* default file locations */
/* #include <netinet/in.h> */
#include "evd_defs.h"

#include "evd.h"


#ifdef THREADS
#include <cma.h>
#include "ev.h"

/*  Interface entry point vector */
globaldef evd_v11_0_epv_t evd_v11_0_m_epv = {
    evd_create_queue_handle,
    evd_delete_queue_handle,
    evd_post_event
} ;
 
#endif /* THREADS */

#ifndef NOIPC
extern int rpc_server_inq_bindings(),
  rpc_binding_to_string_binding(),
  rpc_string_free(),
  rpc_ep_unregister(),
  rpc_binding_vector_free(),
  rpc_server_unregister_if(),
  rpc_binding_vector_free(),
  rpc_server_register_if(),
  rpc_server_use_protseq(),
  rpc_server_inq_bindings(),
  rpc_ep_register(),
  rpc_binding_vector_free(),
  rpc_server_listen();

extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#endif /* NOIPC */

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
# include <sys/file.h>
# include <sys/ioctl.h>
#endif


/*
|
|   Define Prototypes for Module-Local Functions
|
*/

/* evd_init_open_log - Initialize our logging sink according to cmdline args */
static void
evd_init_open_log PROTOTYPE((
 evd_global_data_t   *      /*-> Big Picture Structure to be initialized     */
));


/* evd_init_config - Open and Parse the SNMP-PE Configuration File */
static void
evd_init_config PROTOTYPE((
 evd_global_data_t   *      /*-> Big Picture Structure to be initialized     */
));


/* evd_init_cmdline_args - Parse Cmd Line Args into Big Picture Structure    */
static void
evd_init_cmdline_args PROTOTYPE((
 evd_global_data_t   *,     /*-> Big Picture Structure to be initialized     */
 int                 ,      /* Count of inbound command line arguments       */
 char               *[]     /* Array of pointers to strings of cmd line args */
));


/* evd_init_next_cmdarg - Fetch next command line argument as a string       */
static char *
evd_init_next_cmdarg PROTOTYPE((
 int                ,       /* Count of original inbound cmd line arguments  */
 char               *[],    /* Array of pointers to strings of cmd line args */
 int                *       /* On return TRUE: if next args starts with "-"  */
));


#ifdef THREADS

/* evd_init_init_rpc - Initialize the IPC/RPC environment */
static void
evd_init_init_rpc PROTOTYPE((
 evd_global_data_t   *      /*-> Big Picture Structure to be initialized     */
));


#endif /* THREADS */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_main - Main Initialization Function for the Event Dispatcher.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.
**
**    "argc" and "argv" constitute the description of the command line
**    used to start the EVD.  See the "INPUTS" section for function "main()"
**    (in evd_main.c) for a complete description of these arguments.
**
**
**  OUTPUTS:
**
**    The function returns only on satisfactory initialization, at which time
**    the global data structure has been initialized, logging files opened and,
**    in a threaded/RPC/IPC environment, the necessary IPC/RPC initialization 
**    has been performed.  Upon return to main(), the main thread becomes the 
**    "receiving" thread (only one thread for EVD V1.1).
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the main() function of EVD.  It is just starting and
**        needs to initialize the entire EVD process to begin operations.
**
**    Purpose:
**        This function causes full initialization to occur.  See synopsis for
**        processing performed.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**      <Read in and verify command line arguments>
**      <Open either debug or system log according to command line args>
**
**    >>ifnot NOIPC
**      <check base Common Agent license>
**    >>endif
**
**      if (not superuser)
**          <exit>
**
**    >>if U*X
**    >>ifnot DEBUG
**      <fork into background>
**    >>endif
**    >>endif
**
**      <Init big picture values from command line arguments>
**      <Open, parse and check the configuration file>
**      <Initialize Event UID and queue handle variables in big picture>
**
**    >>if THREADS
**      <Init RPC/IPC for later conversion into a "receiving" thread>
**    >>endif
**
**      <Log EVD as "up" in system log>
**
**
**  OTHER THINGS TO KNOW:
**
**    If we're running in a threaded/RPC/IPC environment, the initialization
**    includes preparing the main thread for conversion into the "receiving" 
**    thread that listens for inbound event requests from the Common Agent 
**    MOMs.  See the <TC> comment below.
**
**    Any errors that occur before we return from this function are logged
**    and are fatal.
**
**--------------------------------------------------------------------------*/

void
evd_init_main (bp, argc, argv)

 evd_global_data_t *bp;     /*-> Big Picture Structure to be initialized     */
 int                argc;   /* Count of inbound command line arguments       */
 char              *argv[]; /* Array of pointers to strings of cmd line args */
{
    char            msg[LINEBUFSIZE];  /* Message build buffer               */
    extern int      ca_check_base_license();
    extern uid_t    getuid();
    extern int      ioctl();

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
    int s;
#endif

    /*
    | Read in and verify command line arguments.  Record the info in the Big 
    | Picture structure.  (Log any errors to stderr & exit)
    */
    evd_init_cmdline_args (bp, argc, argv);

    /*
    | Open either debug or system log according to whether the command line
    | args specify debugging mode or normal operation.
    | (Log errors to stderr and exit during this process).
    */
    evd_init_open_log (bp);

#ifndef NOIPC
    if ((ca_check_base_license()) != 0) 
    {
        fprintf (stderr, MSG(emsg001, "\nE001 %s: License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or\nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n"), argv[0]);
        fflush (stderr);
        exit (1);
    }
#endif /* NOIPC */

    /* Here's the fork to start the daemon.  Ifdef'd for U*X, and */
    /* make sure DEBUG is not defined -- dbx doesn't like fork()'s */

    /* Check for superuser */
    if ( getuid() )
    {
        fprintf (stderr, "%s: not super user.\n", argv[0] );
        exit (1);
    } 

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifndef DEBUG

    /* We don't fork if they turned on debugging mode via cmdline switch */
    if (bp->log_state.debugging_mode == FALSE) 
    {
        if ( fork() )
            exit (0);

        for ( s = 0 ; s < 10 ; s++ )
            (void ) close (s);

        (void ) open ("/", O_RDONLY);
        (void ) dup2 (0, 1);
        (void ) dup2 (0, 2);

        s = open ("/dev/tty", O_RDWR);
        if (s > 0) 
        {
            (void ) ioctl (s, TIOCNOTTY, (char *) NULL);
            (void ) close (s);
        }
    }
    else 
    {
        sprintf (msg, MSG(emsg002, "E002 - EVD Debug Mode enabled: Fork suppressed"));
        SYSLOG (LOG_INFO, msg);
    }

#endif /* DEBUG */
#endif 

/* ========================================================================= */
/* From here on out, fatal errors get logged through a call to macro CRASH() */
/* ========================================================================= */

    /* Open, parse and check the configuration file */
    evd_init_config (bp);

    /* Initialize Event UID and queue handle variables in big picture */
    bp->event_uid = 1;
    bp->queue_handle_list = NULL;
    bp->snmp_queue_handle = (evd_queue_handle *) NULL;

#ifdef THREADS

    /* <TC> In function "evd_init_main()"...                                 */
    /* <TC> Initialize IPC/RPC.  The main thread becomes a receiving thread, */
    /* <TC> waiting for inbound event requests from MOMs.                    */

    /* Initialize IPC/RPC environment */
    evd_init_init_rpc (bp);

#endif

    /* Log EVD as "up" in system log */
    sprintf (msg, MSG(emsg003, "E003 - evd (V%s.%s) initialization complete"),
             MAJOR, MINOR);
    SYSLOG (LOG_INFO, msg);

} /* end of evd_init_main() */

#ifdef THREADS

/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_init_rpc - Initialize the IPC/RPC environment.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.
**
**
**  OUTPUTS:
**
**    The function returns only after correctly initializing the RPC server
**    interface that EVD presents to the MOMs.  It doesn't return anything
**    and any errors result in a CRSAH.
**
**    On success, the "rpc_callback" cell in the global data structure has been
**    initialized.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_main() function of EVD.  It
**        needs to perform IPC/RPC initialization.
**
**    Purpose:
**        This function establishes the protocol, registers the interface,
**        and initializes the "rpc_callback" cell in the global data structure.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    if (attempt to establish protocol failed)
**        <CRASH "Exxx - Protocol setup failure:">
**
**    if (attempt to register the interface failed)
**        <CRASH "Exxx - Interface registration failure:">
**
**    if (attempt to acquire binding vector failed)
**        <CRASH "Exxx - Binding vector acqusition failed:">
**
**    if (attempt to convert binding vector to string failed)
**        <CRASH "Exxx - Binding vector string conversion failed:">
**
**    if (attempt to free the binding vector failed)
**        <CRASH "Exxx - Binding vector deallocation failed:">
**
**    <copy binding string representation length to rpc_callback cell>
**    <copy binding string to rpc_callback cell>
**
**    if (attempt to free the binding string failed)
**        <CRASH "Exxx - Binding string deallocation failed:">
**
**
**  OTHER THINGS TO KNOW:
**
**    This function is only used in the RPC/IPC incarnation of EVD.
**
**--------------------------------------------------------------------------*/

static void
evd_init_init_rpc (bp)

 evd_global_data_t  *bp;    /*-> Big Picture Structure to be initialized */
{
    char                    msg[LINEBUFSIZE];   /* Message build buffer    */
    error_status_t          status=rpc_s_ok;    /* RPC Status indicator    */
    dce_error_string_t      ebuf;               /* DCE Error string buffer */
    rpc_binding_vector_t    *bvec;              /* Binding Vector returned */
    unsigned char           *string_binding;    /* ASCII binding info      */
    int                     str_length;         /* ASCII binding info len  */
    int                     exception=FALSE;    /* Flag for excep. handler */
    int                     estat;              /* Something we need for   */
                                                /*   dce_error_inq_text()  */


    /* ================================================== */
    rpc_server_use_protseq ((unsigned char *) "ncadg_ip_udp",
    /*                       rpc_c_protseq_max_calls_default,    */
                             2,
                             &status);

    /* if (attempt to establish protocol failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg004, "E004 - Protocol setup failure '%s', %d, %d"), ebuf,status,estat);
        CRASH(msg);
    }


    /* ================================================== */
    TRY
        rpc_server_register_if (evd_v11_0_s_ifspec,
                                NULL,
                                (rpc_mgr_epv_t) &evd_v11_0_m_epv,
                                &status);
    CATCH_ALL
        exception = TRUE;
    ENDTRY

    /* if (attempt to register the interface failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg005, "E005 - Interface registration failure '%s',%d, %d"),
                 ebuf, status, estat);
        CRASH(msg);
    }
    else if (exception == TRUE) 
    {
        CRASH(MSG(emsg006, "E006 - Exception TRUE from 'rpc_server_register_if'"));
    }

    /* ================================================== */
    TRY
        rpc_server_inq_bindings (&bvec, &status);

    CATCH_ALL
        exception = TRUE;
    ENDTRY

    /* if (attempt to acquire binding vector failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg007, "E007 - Binding vector acqusition failed '%s',%d, %d"),
                 ebuf, status, estat);
        CRASH(msg);
    }
    else if (exception == TRUE) {
        CRASH(MSG(emsg008, "E008 - Exception TRUE from 'rpc_server_inq_bindings'"));
    }

    /* ================================================== */
    rpc_binding_to_string_binding (bvec->binding_h[0],
                                   &string_binding,
                                   &status);

    /* if (attempt to convert binding vector to string failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg009, "E009 - Binding vector string conversion failure '%s',%d, %d"),
                 ebuf, status, estat);
        CRASH(msg);
    }

    /* ================================================== */
    rpc_binding_vector_free (&bvec, &status);

    /* if (attempt to free the binding vector failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg010, "E010 - Binding vector deallocation failure '%s',%d, %d"),
                 ebuf, status, estat);
        CRASH(msg);
    }

    /* copy binding string representation length to rpc_callback cell */
    bp->rpc_callback.length = str_length = (unsigned int ) strlen((char *)string_binding);

    /* copy binding string to rpc_callback cell */
    memcpy (bp->rpc_callback.socket_address, string_binding, str_length);


    /* ================================================== */
    rpc_string_free (&string_binding, &status);

    /* if (attempt to free the binding string failed) */
    if (status != rpc_s_ok) 
    {
        dce_error_inq_text (status, ebuf, &estat);  /* Fetch DCE error string */
        sprintf (msg,
                 MSG(emsg011, "E011 - Binding string deallocation failure '%s',%d, %d"),
                 ebuf, status, estat);
        CRASH(msg);
    }

} /* end of evd_init_init_rpc() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_start_rpc - Start the RPC server interface.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.
**
**
**  OUTPUTS:
**
**    The function returns only in the event of an error in the listen call.
**    The function should not normally return.
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_main() function of EVD.  It
**        needs to start a thread listening on the EVD RPC/IPC server
**        interface.
**
**    Purpose:
**        This function, started as a separate thread, issues the
**        appropriate RPC listen call.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    <issue RPC listen call>
**    if (status is not OK)
**        <SYSLOG "Exxx - RPC Listen has returned, status = x">
**
**  OTHER THINGS TO KNOW:
**
**    This function is only used in the RPC incarnation of EVD.
**
**
**--------------------------------------------------------------------------*/

void
evd_init_start_rpc (bp)

 evd_global_data_t   *bp;    /*-> Big Picture Structure to be initialized */
{
    char                    msg[LINEBUFSIZE];   /* Message build buffer    */
    error_status_t          status=rpc_s_ok;    /* RPC Status indicator    */
    int                     exception=FALSE;    /* Flag for excep handler  */
    
    /* issue the RPC listen
    |
    | MTHREADS - If multiple PDUs are to be processed, the "1" below must be
    |            replaced with "(MTHREADS + 1)".
    */
    
    TRY
        rpc_server_listen(1,         /* Number of threads to have listening */
                          &status);  /* returned status                     */

    CATCH_ALL
        exception = TRUE;
    ENDTRY
    
    /* If listen status is not OK */
    if (status != rpc_s_ok) 
    {
        sprintf (msg, MSG(emsg012, "E012 - RPC listen returned w/code %d"), 
                 status);
        SYSLOG(LOG_ERR, msg);
    }
    else if (exception == TRUE)
    {
        CRASH(MSG(emsg013, "E013 - Exception TRUE from 'rpc_server_listen'"));
    }

} /* end of evd_init_start_rpc() */

#endif /* THREADS */

 
/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_cmdline_args - Parse Command Line Args into Big Picture 
**                            structure.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.
**
**    "argc" and "argv" constitute the description of the command line
**    used to start EVD.  See the "INPUTS" section for function "main()"
**    (in evd_main.c) for a complete description of these arguments.
**
**
**  OUTPUTS:
**
**    The function returns only on satisfactory parsing of the command line
**    arguments.  The Big Picture structure has been initialized to contain the
**    essence of the command line arguments supplied supplemented w/internal
**    defaults:
**
**        * bp->log_state:
**                * debugging_mode
**                * log_classes
**                * log_file_name
**
**        * bp->config_file_name
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_main() function of EVD.  It
**        needs to parse the command line arguments for correctness and to
**        store the info in them in the big picture structure.
**
**    Purpose:
**        This function parses the command line arguments, verifying them
**        for correctness and setting portions of "bp->log_state" accordingly.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    <initialize valid debug class info names/codes>
**
**    <establish default log_state: No debug, L_SYSLOG class, no logfile name
**     no log file>
**    <establish default for config file name: NULL>
**
**    while (next argument is returned)
**
**        if (argument is hyphenated)
**
**            <step over the hyphen>
**
**            switch (argument value)
**
**                case 'd':       (* Debug Filename *)
**                    if (next argument is returned)
**                        if (log file name already present or arg is hyphenated)
**                            <stderr log: "invalid -d usage">
**                            <exit>
**                        <set debugging mode TRUE>
**                        <set argument value as log file name>
**                    else
**                        <stderr log: "Missing debug logfile name">
**                        <exit>
**                    <break>
**
**                case 'c':       (* Configuration Filename *)
**                    if (next argument is returned)
**                        if (config name already present or arg is hyphenated)
**                            <stderr log: "invalid -c usage">
**                            <exit>
**                        <set argument value as log file name>
**                    else
**                        <stderr log: "Missing configuration file name">
**                        <exit>
**                    <break>
**
**                default:
**                    <stderr log: "invalid command line argument: x">
**                    <exit>
**
**        else  (* argument is not hyphenated *)
**            <signal "debugging mode" TRUE>
**            for (every legal debug class)
**                if (argument caseless matches legal debug class)
**                    <set corresponding bit in log_class>
**                    <set pointer to argument to NULL: success>
**                    <break>
**            if (argument ptr is NON-NULL)
**                <stderr log: "invalid debuginfo class: xxx">
**                <exit>
**
**    (* All command line arguments have been processed: apply config file name
**       logic *)
**
**    if (we're debugging and no -c value was given)
**        if ( environment variable "ECA_SNMPPE_CONFIG_FILE" has a value)
**            <use environment variable as config file name>
**
**    if (no config file name has been specified)
**        <use builtin name>
**
**    >>if MTHREADS
**        <initialize mutex associated w/log file>
**    >>endif
**
**
**  OTHER THINGS TO KNOW:
**
**    Any errors that occur before we return from this function are logged
**    to "stderr" and are fatal.
**
**    No way of seeing the options from the command line as of V1.0.
**
**--------------------------------------------------------------------------*/

static void
evd_init_cmdline_args (bp, argc, argv)

 evd_global_data_t *bp;     /*-> Big Picture Structure to be initialized     */
 int                argc;   /* Count of inbound command line arguments       */
 char              *argv[]; /* Array of pointers to strings of cmd line args */
{
    char    *next_arg;              /*-> String which is next argument      */
    int     hyphenated;             /* TRUE: next arg string starts w/"-"   */
    char    *environ_value;         /* Possible environment string value    */
    int     t_port;                 /* Temporary port number (host order)   */
    char    *port_cnv_char;         /* --> character terminationg conversion*/
    int     i;                      /* Index for scanning debug arrays      */
    extern int strcasecmp();


    /* initialize valid debug class info names/codes */
    bp->log_state.dbg_name[0] = "SYSLOG"; bp->log_state.dbg_flag[0] = L_SYSLOG;
    bp->log_state.dbg_name[1] = "TRACE";  bp->log_state.dbg_flag[0] = L_TRACE;

    /* establish default log_state: No debug, L_SYSLOG class, no logfile name */
    /* no log file:                                                           */
    bp->log_state.debugging_mode = FALSE;    /* No Debugging Mode yet         */
    bp->log_state.log_classes    = L_SYSLOG; /* Logging to SYSLOG always on   */
    bp->log_state.log_file_name  = NULL;     /* No log file name specified yet*/
    bp->log_state.log_file       = NULL;     /* No log file at all yet        */

    /* establish default for config file name: NULL */
    bp->config_file_name = NULL;

    /* while (next argument is returned) */
    while ((next_arg = evd_init_next_cmdarg (argc, argv, &hyphenated)) != NULL)
    {
        /* if argument is hyphenated (e.g.: "-d") */
        if (hyphenated == TRUE) 
        {
            /* step over the hyphen */
            next_arg += 1;

            /* This switch should be dispatching on all valid arguments that */
            /* are preceded with a "-".  Code here is responsible for        */
            /* checking for duplicates.                                      */

            switch (*next_arg) 
            {
                case 'd':   /* "-d <debug log file name>" */

                    /* if (next argument is returned) */
                    if ( (next_arg = evd_init_next_cmdarg (argc, argv,
                                                        &hyphenated)) != NULL)
                    {
                        /* if (log file name already present or arg hyphenated) */
                        if (   (bp->log_state.log_file_name != NULL)
                            || (hyphenated == TRUE)) 
                        {

                            fprintf (stderr, MSG(emsg014, "E014 - invalid -d usage\n"));
                            exit(0);
                        }

                        /* set debugging mode TRUE */
                        bp->log_state.debugging_mode = TRUE;
    
                        /* set argument value as log file name */
                        bp->log_state.log_file_name = next_arg;
                    }
                    else 
                    {
                        fprintf (stderr, MSG(emsg015, "E015 - Missing debug logfile name\n"));
                        exit(0);
                    }
                    break;


                case 'c':   /* "-c <configuration file name>" */

                    /* if (next argument is returned) */
                    if ( (next_arg = evd_init_next_cmdarg (argc, argv, &hyphenated))
                        != NULL ) 
                    {
    
                        /* if (config name already present or arg is hyphenated) */
                        if (   (bp->config_file_name != NULL)
                            || (hyphenated == TRUE)) 
                        {
                            fprintf (stderr, MSG(emsg016, "E016 - invalid -c usage\n"));
                            exit(0);
                        }
    
                        /* set argument value as log file name */
                        bp->config_file_name = next_arg;
                    }
                    else 
                    {
                        fprintf (stderr,
                                 MSG(emsg017, "E017 - Missing configuration file name\n"));
                        exit(0);
                    }
                    break;
    
    
                default:    /* a bogus "-" */
                    fprintf (stderr,
                             MSG(emsg018, "E018 - invalid command line argument: '%s'\n"),
                             next_arg);
                    exit(0);
            }
        }

        else 
        {
            /* argument is not hyphenated: this implies that it is the name */
            /* of a "debug info class" of messages we want to see in debug  */
            /* logging file.                                                */

            /* signal "debugging mode" */
            bp->log_state.debugging_mode = TRUE;

            /* for (every legal debug class) */
            for (i=0; i < LOG_CLASS_COUNT; i++) 
            {
                /* if (argument caseless matches legal debug class) */
                if (strcasecmp (bp->log_state.dbg_name[i], next_arg) == 0) 
                {
                    /* set corresponding bit in log_class */
                    bp->log_state.log_classes |= bp->log_state.dbg_flag[i];

                    /* set pointer to argument to NULL: success */
                    next_arg = NULL;
                    break;
                }
            }

            /* if (argument ptr is NON-NULL) */
            if (next_arg != NULL) 
            {
                fprintf (stderr, MSG(emsg019, "E019 - invalid debug class: '%s'\n"),
                         next_arg);
                exit(0);
            }
        }
    }

    /* All command line arguments have been processed: apply config file name */
    /* logic (see evd_main.c).                                                */
    
    /* if (we're debugging and no -c value was given) */
    if (bp->log_state.debugging_mode == TRUE && bp->config_file_name == NULL)
    {
        /* if (environment variable "ECA_SNMPPE_CONFIG_FILE" has a value) */
        if ( (environ_value = getenv ("ECA_SNMPPE_CONFIG_FILE")) != NULL) 
        {
            /* use environment variable as config file name */
            bp->config_file_name = environ_value;
        }
    }
    
    /* if (no config file name has been specified) */
    if (bp->config_file_name == NULL) 
    {
        /* use builtin name */
        bp->config_file_name = DEFAULT_SNMPPE_CONFIG_PATH;
    }
    
#ifdef MTHREADS
    /* <TC> In function evd_init_cmdline_args()...                          */
    /* <TC> Initialize the mutex associated with the log file regardless of */
    /* <TC> whether we'll be writing to a log file or just using syslog()   */

    /* initialize mutex for log file */
    if (pthread_mutex_init (&bp->log_state.log_file_m, 
                            pthread_mutexattr_default)) != 0) 
    {
        fprintf (stderr, MSG(emsg020, "E020 - pthread mutex initialization failed: %s\n"),
                 strerror(errno) );
        exit(0);
    }
#endif

} /* end of evd_init_cmdline_args() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_next_cmdarg - Fetch next command line argument as a string.
**
**  FORMAL PARAMETERS:
**
**    "argc" and "argv" constitute the description of the command line
**    used to start EVD.  See the "INPUTS" or "FORMAL PARAMETERS" section of
**    function "main()" (in evd_main.c) for a complete description of these 
**    arguments.
**
**    "hyphenated" is a boolean used to indicate on return that the string
**    being returned originally started with "-".
**
**
**  OUTPUTS:
**
**    The function returns the next command line argument as a string 
**    or NULL if there are no more.
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_cmdline_args() function of EVD.  It
**        needs to get the next argument from the command line.
**
**    Purpose:
**        This function sequentially returns each command line argument
**        while signalling whether or not it is preceded by a "-".
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    if (next index >= argc )
**        <return NULL>
**
**    <set hyphenated according to next argument>
**    <return pointer to next argument while incrementing counter>
**
**
**  OTHER THINGS TO KNOW:
**
**    A local static variable keeps track of where we are in the array
**    of command strings.  (Yeah, this breaks the "no static data" rule,
**    but how much trouble are we in here by doing this anyway?)
**
**--------------------------------------------------------------------------*/

static char *
evd_init_next_cmdarg (argc, argv, hyphenated)

 int     argc;           /* Count of original inbound command line arguments */
 char    *argv[];        /* Array of pointers to strings of cmd line args    */
 int     *hyphenated;    /* On return TRUE: if next arg starts with "-"      */
{
    static int next=1;        /* Index of next arg to be returned */

    /* if (next index >= argc ) */
    if (next >= argc)
        return (NULL);

    /* set hyphenated according to next argument */
    *hyphenated = (*argv[next] == '-') ? (TRUE) : (FALSE);

    /* return pointer to next argument while incrementing counter */
    return (argv[next++]);

} /* end of evd_init_next_cmd_arg() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_open_log - Initialize EVD's logging sink according to cmdline
**                        arguments.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.  This function is concerned with finishing the
**    initialization of the log_state structure in the big picture.
**
**
**  OUTPUTS:
**
**    The function returns only on satisfactory opening of the logging sink,
**    either the system log or a user-specified log file.
**
**        * bp->log_state:
**                * log_file -- open occurs if debug mode.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_main() function of EVD.  It
**        needs to get the logging sink open so all other initialization
**        functions can log errors into the logging sink.
**
**    Purpose:
**        This function examines the current state of the "log_state"
**        portion of the "big picture" to open the proper logging sink
**        according to the command line arguments already parsed.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    if (logging mode is "debugging")
**
**        if (no logging file supplied)
**            <initialize to use stderr>
**        else
**            if (attempt to open for create/append the specified file failed)
**                <stderr log: "Exxx - attempt to open file %s" failed>
**                <exit>
**
**    else (* we're not debugging: normal operations to system log *)
**        open system log
**
**
**  OTHER THINGS TO KNOW:
**
**    Any errors that occur before we return from this function are logged
**    to "stderr" and are fatal.
**
**
**--------------------------------------------------------------------------*/

static void
evd_init_open_log (bp)
           
 evd_global_data_t   *bp;    /*-> Big Picture Structure to be initialized */
{
    int     syslog_code;            /* Value returned by openlog() call */
    int     openlog();

    /* if (logging mode is "debugging") */
    if (bp->log_state.debugging_mode == TRUE) 
    {
        /* We're in debugging mode.  If there is no file name, we use stderr */
        /* if (no logging file supplied) */
        if (bp->log_state.log_file_name == NULL) 
        {
            bp->log_state.log_file = stderr;     /* initialize to use stderr */
        }
        else /* There is a filename, try to open it */
        {
            /* if(attempt to open for create/append the specified file failed)*/
            if ( (bp->log_state.log_file = fopen (bp->log_state.log_file_name, 
                                                  "a")) == NULL) 
            {
                fprintf (stderr, MSG(emsg021, "E021 - attempt to open file '%s' failed: %s\n"),
                         bp->log_state.log_file_name,
                         strerror(errno) );
                exit(0);
            }
        }
    }

    else /* we're not debugging: normal operations to system log */
    {
        /* open system log */
#if defined(__osf__) || defined(sun) || defined(sparc)
        syslog_code = openlog ("evd", LOG_PID, LOG_DAEMON);
#else
        syslog_code = openlog ("evd", LOG_PID);
#endif
    }

} /* end of evd_init_open_log() */


/*
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
**  FUNCTIONAL DESCRIPTION:
**
**    evd_init_config - Open and Parse the SNMP-PE Configuration File.
**
**  FORMAL PARAMETERS:
**
**    "bp" points to the Big Picture structure containing all the "global"
**    context information that needs to be initialized before EVD can
**    begin operation.  This function is concerned with initializing:
**
**    bp->
**          * snmp_trap_listeners
**          * snmp_traps_disabled
**
**    using:
**
**    bp->
**          * config_file_name
**
**  OUTPUTS:
**
**    The function returns the big picture structure initialized with info
**    extracted from the configuration file.  Specifically a count of the trap
**    communities recognized by SNMP PE (and EVD), and whether or not the
**    sending of traps to all trap communities is disabled.
**
**
**  BIRD'S EYE VIEW:
**    Context:
**        The caller is the evd_init_main() function of EVD.  It
**        needs to suck in all the info in the configuration file.
**
**    Purpose:
**        This function opens and parses all the information contained
**        in the configuration file.  We ignore all "verbs" except the
**        "trap" verb and the "disable_traps" verb.
**
**
**  ACTION SYNOPSIS OR PSEUDOCODE:
**
**    <initialize the verb list>
**    <initialize the list headers in the bp>
**
**    if (attempt to open configuration file failed)
**       <crash exit "open failed on config file: %s">
**
**    while (lines remain to be read)
**
**        <count another line>
**        <span pointer into line over any whitespace at start of line>
**
**        if (current character is '0' or '#' or '\n')
**            <continue>
**
**        if (parse of initial token fails)
**            <continue>
**
**        <assume no verb match>
**        for (all verbs in verb list)
**            if (current verb caseless matches current first token)
**                <set verb index>
**                <break>
**
**        switch (verb index)
**
**            case VERB_DISABLETRAPS:
**                <set flag in global data structure indicating SNMP traps are
**                 disabled>
**                <break>
**
**            case VERB_TRAP:
**                <increment count of SNMP TRAP communities in global data 
**                 structure>
**                <break>
**
**            case VERB_COMMUNITY:
**            case VERB_NOAUTHTRAPS:
**            default: (* Unknown_Verb *)
**                <break>
**
**    <close the configuration file>
**
**
**  OTHER THINGS TO KNOW:
**
**    The list of "verbs" (first non-blank string in each line of the
**    configuration file) is found in the array "verbs[]" below.
**
**    Any line with first non-whitespace character of "#" is a comment.
**
**    Empty lines and lines containing whitespace only are ignored.
**
**
**--------------------------------------------------------------------------*/

static void
evd_init_config (bp)

 evd_global_data_t   *bp;    /*-> Big Picture Structure to be initialized */

#define VERB_COUNT         2  /* Number of Verbs in config file WE recognize */
#define VERB_DISABLETRAPS  0  /* "disable_traps" */
#define VERB_TRAP          1  /* "trap"          */
#define UNKNOWN_VERB VERB_COUNT

{
    FILE    *config;              /* Configuration File Descriptor pointer    */
    char    *verbs[VERB_COUNT];   /* List of "verbs" we recognize from config */
    char    linebuf[LINEBUFSIZE]; /* Input line buffer                        */
    char    msgbuf[LINEBUFSIZE];  /* Crash Error messages built here          */
                                  /* are constructed here                     */
    char    *next_char;           /* -> Next character in input line          */
    int     lineno=0;             /* Line # in configuration file we're on    */
    int     verb_index;           /* Index of recognized verb                 */
    int     i;                    /* General purpose loop index               */
    char    *temp_str;            /* Temporary string pointer                 */
    extern  int strcasecmp();


    /* Initialize the Verb list
    |
    |  (The index into this array to the string is the "verb index" of that verb
    |  and serves to indicate the selected verb when it is parsed from an
    |  input line in the configuration file, we define the corresponding symbols
    |  to make the code easier to read).
    */

    verbs[VERB_DISABLETRAPS]   = "disable_traps";
    verbs[VERB_TRAP]           = "trap";

    /* initialize list counter(s) in the bp */
    bp->snmp_trap_listeners = 0;  /* assume no SNMP trap listeners */
    bp->snmp_traps_disabled = 0;  /* assume SNMP traps are enabled */

    /* if (attempt to open configuration file failed) */
    if ( (config = fopen (bp->config_file_name, "r")) == NULL) 
    {
        sprintf (msgbuf, MSG(emsg023, "E023 - open failed on config file: '%s'"),
                 bp->config_file_name);
        CRASH(msgbuf);
    }

    /* while (lines remain to be read) */
    while ( (next_char = fgets (linebuf, LINEBUFSIZE, config)) != NULL) 
    {
        lineno += 1;    /* count another line */

        /* span pointer into line over any whitespace at start of line */
        next_char += strspn (next_char, " \t");
    
        /* if (current character is '0' or '#' or '\n') */
        if (*next_char == '\0' || *next_char == '#' || *next_char == '\n')
            continue;

        /* if (parse of initial token fails) */
        if ( (next_char = (char *) strtok (next_char, " \t\n")) == NULL)
            continue;

        verb_index = UNKNOWN_VERB;    /* assume no verb match */

        /* for (all verbs in verb list) */
        for (i=0; i < VERB_COUNT; i++) 
        {
            /* if (current verb caseless matches current first token) */
            if ( strcasecmp (verbs[i], next_char) == 0) 
            {
                verb_index = i;     /* set verb index */
                break;
            }
        }

        switch (verb_index) 
        {
            case VERB_TRAP:
                bp->snmp_trap_listeners += 1;
                break;

            case VERB_DISABLETRAPS:
                bp->snmp_traps_disabled = 1;
                break;

            default:    /* Unknown verb; just skip it */
                break;
        }

    } /* end_while */

    /* close the configuration file */
    fclose (config);

} /* end of evd_init_config() */

