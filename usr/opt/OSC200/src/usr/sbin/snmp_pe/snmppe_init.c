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
static char *rcsid = "@(#)$RCSfile: snmppe_init.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:34:02 $";
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
 * Module SNMPPE_INIT.C
 *      Contains initialization functions for the SNMP Protocol Engine for the
 *      Comman Agent.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   June 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines accept network connects,
 *       convert incoming information into standard Common Agent format
 *       and call the Common Agent for processing of that information.
 *       The main function has been entered and needs to set up the
 *       "big picture" data structure which constitutes the SNMP PE
 *       execution context.
 *
 *    Purpose:
 *       This module contains the initialization functions for SNMP PE.
 *       When "init_main()" returns, the SNMP PE is ready to accept
 *       inbound SNMP PDUs.
 *
 * History
 *      V1.0    June 1991              D. D. Burns
 *      V1.1    July 1992              D. D. Burns
 *                                      Add support for TRACE message class
Module Overview:
---------------

This module contains the initialization function(s) for the SNMP Protocol
Engine.


Thread Overview:
---------------

All the functions in this module including "init_main()" are executed
exclusively by the main ("sending") thread at start-up time.  The receiving
thread doesn't even come into existence until just before "init_main()"
returns where code there creates it.


MODULE CONTENTS:

USER-LEVEL INTERFACE    (ie Functions defined in this module for reference
                         elsewhere):

Function Name           Synopsis
-------------           --------

init_main               The "main" initialization function, called by the
                        SNMP PE main() function, "init_main()" dispatches
                        to functions internal to this module to accomplish
                        the work of initializing SNMP PE.  This consists
                        largely of filling in cells in a central data
                        structure called the "Big Picture" which is passed
                        to all important routines in SNMP PE.


MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
-------------           --------
init_cmdline_args       Parse Command Line Args into Big Picture structure

init_next_cmdarg        Fetch next command line argument as a string
                        (for use exclusively by init_cmdline_args())

init_open_log           Initialize our logging sink according to cmdline
                        arguments now in Big Picture.

init_config             Open and Parse the Configuration File, initializing
                        Big Picture accordingly

init_shared_memory      Setup Shared Memory Data Structure (for communication
                        with MOM)

init_init_rpc           Initialize RPC for server entry points

init_start_rpc          Start threads doing an RPC listen for server interface

init_netio              Initialize Inbound and Outbound Network I/O and
                        record info required to do Net I/O in the Big Picture
*/

/* Module-wide Stuff */
#include <stdlib.h>
#include <syslog.h>
#include <strings.h>
#include <math.h>
#include <netdb.h>

#if defined(__osf__) && !defined(_OSF_SOURCE)
/* KLUDGE. <sys/socket.h> needs <sys/types.h>, which doesn't define "u_"
   typedefs unless you've got _OSF_SOURCE turned on. */
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE

#else
#include <sys/types.h>
#endif

#include <sys/socket.h>
#include <sys/utsname.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>


/* includes required for "snmppe.h" */
#include "port_cmip.h"
#include <stdio.h>
#include "moss.h"
#include <netinet/in.h>
#define OID_DEF
#include "snmppe.h"     /* directly includes ISO_DEFS.H and SNMP_MIB.H */

#include "mir.h"        /* So we can initialize MIR Tier 0 Interface */

#include "ca_config.h"  /* default file locations */

#ifdef THREADS
#include <rpc.h>
#include <exc_handling.h>
#include <dce_error.h>
#include "pe.h"

/*  Protocol Engine RPC V1.0 Interface entry point vector */
globaldef  pe_v1_0_epv_t  pe_v1_0_m_epv = {
    pei_send_get_reply,
    pei_send_set_reply,
    pei_send_create_reply,
    pei_send_delete_reply,
    pei_send_action_reply
} ;

/* Shared Memory */
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#include <sys/file.h>
#include <sys/ioctl.h>
#endif

/*
 * Global
 */
char *global_pe_name = "snmp_pe";    /* PE name string */

extern int rpc_server_use_protseq(),
    rpc_server_register_if(),
    rpc_server_inq_bindings(),
    rpc_binding_to_string_binding(),
    rpc_binding_vector_free(),
    rpc_string_free(),
    rpc_server_listen();

/***** Definitions required for initializing the Shared Memory Segment *****/

/*
 *  The following definitions and declarations were taken directly from the
 *  Internet MOM's module SYSTEM.C.  Both the SNMP PE and the IMOM 
 *  are responsible for initializing the shared memory segment that they share.
 *  As a result, whichever process creates the segment will initialize it
 *  (thus the same logic).  This approach will accomodate any potential race
 *  conditions (i.e., if the PE and the IMOM start at the same time).
 *
 *  See the init_shared_memory() function below.
 *
 *  The system_object_id is the value returned by the attribute
 *  sysobjectid in the class System.  This value has been verified
 *  with the DSRG.  Currently using: 1.3.6.1.4.1.36.2.18.25 for CAU V1.0
 *  (mips).  See ISO_DEFS.H for its definition.
 */


static
unsigned int
    system_object_id_array[ ULTRIX_LENGTH ] = { ULTRIX_SEQ };

static
object_id
    system_object_id_oid = { ULTRIX_LENGTH, system_object_id_array };

static
octet_string sysid_octet;  /* converted to octet str in init_shared_memory() */

/*
|
|   Define Prototypes for Module-Local Functions
|
*/

/* init_cmdline_args - Parse Command Line Args into Big Picture structure */
static void
init_cmdline_args PROTOTYPE((
big_picture_t   *,      /*-> Big Picture Structure to be initialized       */
int              ,      /* Count of inbound command line arguments         */
char            *[]     /* Array of pointers to strings of cmd line args   */
));

/* init_next_cmdarg - Fetch next command line argument as a string */
static char *
init_next_cmdarg PROTOTYPE((
int     ,              /* Count of original inbound command line arguments */
char    *[],           /* Array of pointers to strings of cmd line args    */
BOOL    *              /* On return TRUE: if next arg starts with "-"      */
));

/* init_open_log - Initialize our logging sink according to cmdline args */
static void
init_open_log PROTOTYPE((
big_picture_t   *      /*-> Big Picture Structure to be initialized        */
));

/* init_config - Open and Parse the Configuration File */
static void
init_config PROTOTYPE((
big_picture_t   *      /*-> Big Picture Structure to be initialized     */
));

/* init_shared_memory - Setup Shared Memory Data Structure */
static void
init_shared_memory PROTOTYPE((
big_picture_t   *      /*-> Big Picture Structure to be initialized     */
));

/* init_netio - Initialize Inbound and Outbound Network I/O */
static void
init_netio PROTOTYPE((
big_picture_t   *      /*-> Big Picture Structure to be initialized     */
));

#ifdef THREADS
/* init_init_rpc - Initialize the RPC environment */
static void
init_init_rpc PROTOTYPE((
big_picture_t   *      /*-> Big Picture Structure to be initialized     */
));

/* init_start_rpc - Start the RPC server interface */
static void
init_start_rpc PROTOTYPE((
));
#endif


/* init_main - Initialization Function for SNMP Protocol Engine */
/* init_main - Initialization Function for SNMP Protocol Engine */
/* init_main - Initialization Function for SNMP Protocol Engine */

void
init_main(bp, argc, argv)

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */
int argc;               /* Count of inbound command line arguments       */
char *argv[];           /* Array of pointers to strings of cmd line args */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.

    "argc" and "argv" constitute the description of the command line
    used to start the SNMP PE.  See the "INPUTS" section for function "main()"
    (in snmppe_main.c) for a complete description of these arguments.


OUTPUTS:

    The function returns only on satisfactory initialization, at which time
    the Big Picture structure has been initialized, logging files opened and
    network I/O ports established.

    In a threaded/RPC environment the necessary RPC initialization has been
    performed and the "receiving" thread has started execution upon successful
    return.


BIRD'S EYE VIEW:
    Context:
        The caller is the main() function of SNMP PE.  It is just starting and
        needs to initialize the entire SNMP PE to begin operations.

    Purpose:
        This function causes full initialization to occur.  See synopsis for
        processing performed.


ACTION SYNOPSIS OR PSEUDOCODE:

    <Read in and verify command line arguments>
    <Open either debug or system log according to command line args>

    <Open, parse and check the configuration file>
    <Set up the in/outbound network i/o ports for requests/traps>
    <Set up shared memory statistics block w/IMOM>
    <Initialize MIR T0>

if THREADS
    if (Start receiving thread failed)
        <signal failure and exit>
    else
        <signal success: thread running>
endif

ifdef NOIPC
    <initialize the IMOM>
    <initialize the EVD>
endif

    <Log snmp_pe as "up" in system log>


OTHER THINGS TO KNOW:

    If we're running in a threaded/RPC environment, the initialization
    includes starting the "receiving" thread that does an RPC listen for
    replies from the Common Agent.  See the <TC> below.

    Any errors that occur before we return from this function are logged
    and are fatal.
*/

{
char            msg[LINEBUFSIZE];  /* Message build buffer               */
char            *mir_err_msg;      /* --> MIR-generated error string     */
mir_status      mstatus;           /* Return status from MIR init        */
int             *preamble = NULL;  /* Preamble info from mir database file */
extern int      ca_check_base_license();
extern uid_t    getuid();
extern pid_t    cma_fork();
extern int      cma_close();
extern int      cma_dup2();
extern int      ioctl();


#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
int             s;
#endif

#ifdef NOIPC
extern man_status mold_populate_from_mir();
extern void mom_init();
extern void evd_init();
#endif

/*
| Read in and verify command line arguments.  Record the info in the Big 
| Picture structure.  (Log any errors to stderr & exit)
*/
init_cmdline_args( bp, argc, argv);

/*
| Open either debug or system log according to whether the command line
| args specify debugging mode or normal operation.
| (Log errors to stderr and exit during this process).
*/
init_open_log( bp );

#ifndef NOIPC
if ((ca_check_base_license()) != 0) {
  fprintf (stderr, MSG(msg286, "\nM286 %s: License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or\nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded.\n"), argv[0]);
  fflush( stderr ) ;
  exit (1);
}
#endif /* NOIPC */

    /*
    | Check for superuser (which we need to be to bind to SNMP port)
    */

    if ( getuid() )
    {
        fprintf( stderr, "%s: not super user.\n", argv[ 0 ] ) ;
        exit( 1 ) ;
    } 

/* Here's the fork to start the daemon.  Ifdef'd for u*x, and make */
/* sure DEBUG is not defined -- dbx doesn't like fork()'s */

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifndef DEBUG
    /* We don't fork if they turned on debugging mode via cmdline switch */
    if (bp->log_state.debugging_mode == FALSE) {

        if ( fork() )
            exit ( 0 ) ;

        for ( s = 0 ; s < 10 ; s++ )
            ( void )close( s ) ;

        ( void )open( "/", O_RDONLY ) ;
        ( void )dup2( 0, 1 ) ;
        ( void )dup2( 0, 2 ) ;

        s = open( "/dev/tty", O_RDWR ) ;
        if ( s > 0 ) {
            ( void )ioctl( s, TIOCNOTTY, ( char * )NULL ) ;
            ( void )close( s ) ;
            }
        }
    else {
        sprintf(msg, MSG(msg244, "M244 - snmp_pe Debug Mode enabled: Fork suppressed"));
        SYSLOG( LOG_INFO, msg);
        }

#endif /* DEBUG */
#endif 

/* ========================================================================= */
/* From here on out, fatal errors get logged through a call to macro CRASH() */
/* ========================================================================= */

init_config( bp );         /* Config File Open, Parse and load to big picture*/

init_netio( bp );          /* Set up the in/outbound network i/o ports       */

init_shared_memory( bp );  /* Set up shared memory statistics block w/MOM    */


/* initialize the Tier 0 MIR Functions */
mstatus = mir_t0_init(DEFAULT_DATABASE_PATH,
		      "ECA_MIR",
		      NULL,
		      TRUE,
		      NULL,
		      NULL,
		      NULL,
		      NULL,
		      &preamble);

if ( mstatus != MS_SUCCESS) {
    sprintf(msg, MSG(msg232, "M232 - MIR Init Fail: %s"), mir_err_msg);
    CRASH( msg );
    }

#ifdef THREADS
/* <TC> In function "init_main()" . . .                                      */
/* <TC> Initialize RPC, then start the receiving thread(s), waiting for RPC  */
/* <TC> invocation of our reply functions.                                   */

init_init_rpc( bp );       /* Initialize RPC environment                     */

/* Start receiving thread if needed */
if (pthread_create(&(bp->recv_thread),
                   pthread_attr_default,
   (pthread_startroutine_t) init_start_rpc,
   (pthread_addr_t) bp) != 0) {        /* a "pthread_addr_t" is a "(void *)" */
    CRASH(MSG(msg003, "M003 - RPC Receive Thread(s) creation failed"));
    }
#endif

#ifdef NOIPC
/* To build a single image MOM, we need to populate the MOLD from mir.       */
if (mold_populate_from_mir() != MAN_C_SUCCESS) {
    sprintf(msg, MSG(msg287, "M287 - mold_populate_from_mir() fails"));
    CRASH( msg );
    }

/* initialize the Internet MOM and EVD */
mom_init();
evd_init(argc, argv);
#endif /* NOIPC */

/* Log snmp_pe as "up" in system log */
sprintf(msg, MSG(msg004, "M004 - snmp_pe (V%s.%s) initialization complete"), MAJOR, MINOR );
SYSLOG( LOG_INFO, msg);

MIR_STATS_TO_STDERR("Immediately after PE initialization");
}

#ifdef THREADS
/* init_init_rpc - Initialize the RPC environment */
/* init_init_rpc - Initialize the RPC environment */
/* init_init_rpc - Initialize the RPC environment */

static void
init_init_rpc( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.


OUTPUTS:

    The function returns only after correctly initializing the RPC server
    interface that SNMP PE presents to the MOMs.  It doesn't return anything
    and any errors result in a CRSAH.

    On success, the "rpc_callback" cell in the Big Picture has been
    initialized.


BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to perform RPC initialization.

    Purpose:
        This function establishes the protocol, registers the interface,
        and initializes the "rpc_callback" cell in the Big Picture.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (attempt to establish protocol failed)
        <CRASH "Mxxx - Protocol setup failure:">

    if (attempt to register the interface failed)
        <CRASH "Mxxx - Interface registration failure:">

    if (attempt to acquire binding vector failed)
        <CRASH "Mxxx - Binding vector acqusition failed:">

    if (attempt to convert binding vector to string failed)
        <CRASH "Mxxx - Binding vector string conversion failed:">

    if (attempt to free the binding vector failed)
        <CRASH "Mxxx - Binding vector deallocation failed:">

    <copy binding string representation length to rpc_callback cell>
    <copy binding string to rpc_callback cell>

    if (attempt to free the binding string failed)
        <CRASH "Mxxx - Binding string deallocation failed:">


OTHER THINGS TO KNOW:

    This function is only used in the RPC incarnation of SNMP PE.
*/

{
char                    msg[LINEBUFSIZE];       /* Message build buffer    */
error_status_t          status=rpc_s_ok;        /* RPC Status indicator    */
dce_error_string_t      ebuf;                   /* DCE Error string buffer */
rpc_binding_vector_t    *bvec;                  /* Binding Vector returned */
unsigned char           *string_binding;        /* ASCII binding info      */
int                     str_length;             /* ASCII binding info len  */
BOOL                    exception=FALSE;        /* Flag for excep. handler */
int                     estat;                  /* Something we need for   */
                                                /*   dce_error_inq_text()  */


/* ================================================== */
rpc_server_use_protseq((unsigned char *) "ncadg_ip_udp",
/*                       rpc_c_protseq_max_calls_default,    */
                       2,
                       &status);

/* if (attempt to establish protocol failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg222, "M222 - Protocol setup failure '%s', %d, %d"), ebuf,status,estat);
    CRASH( msg );
    }


/* ================================================== */
TRY
    rpc_server_register_if(pe_v1_0_s_ifspec,
                           NULL,
                           (rpc_mgr_epv_t) &pe_v1_0_m_epv,
                           &status);
CATCH_ALL
    exception = TRUE;
ENDTRY

/* if (attempt to register the interface failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg223, "M223 - Interface registration failure '%s',%d, %d"),
            ebuf, status, estat);
    CRASH( msg );
    }
else if (exception == TRUE) {
    CRASH(MSG(msg229, "M229 - Exception TRUE from 'rpc_server_register_if'"));
    }

/* ================================================== */
TRY
    rpc_server_inq_bindings( &bvec, &status );

CATCH_ALL
    exception = TRUE;
ENDTRY

/* if (attempt to acquire binding vector failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg224, "M224 - Binding vector acqusition failed '%s',%d, %d"),
            ebuf, status, estat);
    CRASH( msg );
    }
else if (exception == TRUE) {
    CRASH(MSG(msg230, "M230 - Exception TRUE from 'rpc_server_inq_bindings'"));
    }

/* ================================================== */
rpc_binding_to_string_binding(bvec->binding_h[0],
                              &string_binding,
                              &status);

/* if (attempt to convert binding vector to string failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg225, "M225 - Binding vector string conversion failure '%s',%d, %d"),
            ebuf, status, estat);
    CRASH( msg );
    }

/* ================================================== */
rpc_binding_vector_free( &bvec, &status);

/* if (attempt to free the binding vector failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg226, "M226 - Binding vector deallocation failure '%s',%d, %d"),
            ebuf, status, estat);
    CRASH( msg );
    }

/* copy binding string representation length to rpc_callback cell */
bp->rpc_callback.length = str_length = (unsigned int ) strlen((char *)string_binding);

/* copy binding string to rpc_callback cell */

memset((void *)bp->rpc_callback.socket_address, '\0', 
       sizeof(bp->rpc_callback.socket_address));

memcpy( bp->rpc_callback.socket_address, string_binding, str_length);


/* ================================================== */
rpc_string_free(&string_binding, &status);

/* if (attempt to free the binding string failed) */
if (status != rpc_s_ok) {
    dce_error_inq_text(status, ebuf, &estat);   /* Fetch DCE error string */
    sprintf(msg,
            MSG(msg227, "M227 - Binding string deallocation failure '%s',%d, %d"),
            ebuf, status, estat);
    CRASH( msg );
    }

}

/* init_start_rpc - Start the RPC server interface */
/* init_start_rpc - Start the RPC server interface */
/* init_start_rpc - Start the RPC server interface */

static void
init_start_rpc( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.


OUTPUTS:

    The function returns only in the event of an error in the listen call.
    The function should not normally return.

BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to start a thread listening on the SNMP PE RPC server
        interface.

    Purpose:
        This function, started as a separate thread, issues the
        appropriate RPC listen call.


ACTION SYNOPSIS OR PSEUDOCODE:

    <issue RPC listen call>
    if (status is not OK)
        <SYSLOG "Mxxx - RPC Listen has returned, status = x">

OTHER THINGS TO KNOW:

    This function is only used in the RPC incarnation of SNMP PE.

*/

{
char                    msg[LINEBUFSIZE];       /* Message build buffer    */
error_status_t          status=rpc_s_ok;        /* RPC Status indicator    */
BOOL                    exception=FALSE;        /* Flag for excep handler  */

/* issue the RPC listen
|
| MTHREADS - If multiple PDUs are to be processed, the "1" below must be
|            replaced with "(MTHREADS + 1)".
*/

TRY
    rpc_server_listen(1,            /* Number of threads to have listening */
                      &status       /* returned status                     */
                      );
CATCH_ALL
    exception = TRUE;
ENDTRY

/* If listen status is not OK */
if (status != rpc_s_ok) {
    sprintf(msg, MSG(msg228, "M228 - RPC listen returned w/code %d"), status);
    SYSLOG(LOG_ERR, msg );
    }
else if (exception == TRUE) {
    CRASH(MSG(msg231, "M231 - Exception TRUE from 'rpc_server_listen'"));
    }
}
#endif
 
/* init_cmdline_args - Parse Command Line Args into Big Picture structure */
/* init_cmdline_args - Parse Command Line Args into Big Picture structure */
/* init_cmdline_args - Parse Command Line Args into Big Picture structure */

static void
init_cmdline_args(bp, argc, argv)

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */
int argc;               /* Count of inbound command line arguments       */
char *argv[];           /* Array of pointers to strings of cmd line args */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.

    "argc" and "argv" constitute the description of the command line
    used to start the SNMP PE.  See the "INPUTS" section for function "main()"
    (in snmppe_main.c) for a complete description of these arguments.


OUTPUTS:

    The function returns only on satisfactory parsing of the command line
    arguments.  The Big Picture structure has been initialized to contain the
    essence of the command line arguments supplied supplemented w/internal
    defaults:

        * bp->log_state:
                * debugging_mode
                * log_classes
                * log_file_name

        * bp->config_file_name


BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to parse the command line arguments for correctness and to
        store the info in them in the big picture structure.

    Purpose:
        This function parses the command line arguments, verifying them
        for correctness and setting portions of "bp->log_state" accordingly.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize valid debug class info names/codes>

    <establish default log_state: No debug, L_SYSLOG class, no logfile name
     no log file>
    <establish default for config file name: NULL>

    while (next argument is returned)

        if (argument is hyphenated)

            <step over the hyphen>

            switch (argument value)

                case 'p':       (* Inbound-Request Port Number *)
                    if (next argument is returned)
                        if (string fails to convert to binary number correctly)
                            <stderr log: "invalid inbound "-p" port value">
                            <exit>
                        <convert number to network byte order and store in bp>
                    else
                        <stderr log: "Missing inbound port number">
                        <exit>
                    <break>

                case 't':       (* Outbound Trap Port Nmuber *)
                    if (next argument is returned)
                        if (string fails to convert to binary number correctly)
                            <stderr log: "invalid outbound "-t" port value">
                            <exit>
                        <convert number to network byte order and store in bp>
                    else
                        <stderr log: "Missing outbound trap port number">
                        <exit>
                    <break>

                case 'd':       (* Debug Filename *)
                    if (next argument is returned)
                        if (log file name already present or arg is hyphenated)
                            <stderr log: "invalid -d usage">
                            <exit>
                        <set debugging mode TRUE>
                        <set argument value as log file name>
                    else
                        <stderr log: "Missing debug logfile name">
                        <exit>
                    <break>

                case 'c':       (* Configuration Filename *)
                    if (next argument is returned)
                        if (config name already present or arg is hyphenated)
                            <stderr log: "invalid -c usage">
                            <exit>
                        <set argument value as log file name>
                    else
                        <stderr log: "Missing configuration file name">
                        <exit>
                    <break>

                default:
                    <stderr log: "invalid command line argument: x">
                    <exit>

        else  (* argument is not hyphenated *)
            <signal "debugging mode" TRUE>
            for (every legal debug class)
                if (argument caseless matches legal debug class)
                    <set corresponding bit in log_class>
                    <set pointer to argument to NULL: success>
                    <break>
            if (argument ptr is NON-NULL)
                <stderr log: "invalid debuginfo class: xxx">
                <exit>

    (* All command line arguments have been processed: apply config file name
       logic *)

    if (we're debugging and no -c value was given)
        if ( environment variable "ECA_SNMPPE_CONFIG_FILE" has a value)
            <use environment variable as config file name>

    if (no config file name has been specified)
        <use builtin name>

if MTHREADS
    <initialize mutex associated w/log file>
endif


OTHER THINGS TO KNOW:

    Any errors that occur before we return from this function are logged
    to "stderr" and are fatal.

    No way of seeing the options from the command line as of V1.0.
*/

{
char    *next_arg;              /*-> String which is next argument      */
BOOL    hyphenated;             /* TRUE: next arg string starts w/"-"   */
char    *environ_value;         /* Possible environment string value    */
int     t_port;                 /* Temporary port number (host order)   */
char    *port_cnv_char;         /* --> character terminationg conversion*/
int     i;                      /* Index for scanning debug arrays      */
extern int strcasecmp();

/* initialize valid debug class info names/codes */
bp->log_state.dbg_name[ 0] = "SYSLOG";      bp->log_state.dbg_flag[ 0] = L_SYSLOG;
bp->log_state.dbg_name[ 1] = "PDUGETIN";    bp->log_state.dbg_flag[ 1] = L_PDUGETIN;
bp->log_state.dbg_name[ 2] = "PDUGETOUT";   bp->log_state.dbg_flag[ 2] = L_PDUGETOUT;
bp->log_state.dbg_name[ 3] = "PDUGETERR";   bp->log_state.dbg_flag[ 3] = L_PDUGETERR;
bp->log_state.dbg_name[ 4] = "PDUSETIN";    bp->log_state.dbg_flag[ 4] = L_PDUSETIN;
bp->log_state.dbg_name[ 5] = "PDUSETOUT";   bp->log_state.dbg_flag[ 5] = L_PDUSETOUT;
bp->log_state.dbg_name[ 6] = "PDUSETERR";   bp->log_state.dbg_flag[ 6] = L_PDUSETERR;
bp->log_state.dbg_name[ 7] = "PDUNXTIN";    bp->log_state.dbg_flag[ 7] = L_PDUNXTIN;
bp->log_state.dbg_name[ 8] = "PDUNXTOUT";   bp->log_state.dbg_flag[ 8] = L_PDUNXTOUT;
bp->log_state.dbg_name[ 9] = "PDUNXTERR";   bp->log_state.dbg_flag[ 9] = L_PDUNXTERR;

bp->log_state.dbg_name[10] = "PDUSUMIN";    bp->log_state.dbg_flag[10] = L_PDUSUMIN;
bp->log_state.dbg_name[11] = "PDURSPOUT";   bp->log_state.dbg_flag[11] = L_PDURSPOUT;
bp->log_state.dbg_name[12] = "PDUERROUT";   bp->log_state.dbg_flag[12] = L_PDUERROUT;
bp->log_state.dbg_name[13] = "PDUSUMOUT";   bp->log_state.dbg_flag[13] = L_PDUSUMOUT;
bp->log_state.dbg_name[14] = "PDU";         bp->log_state.dbg_flag[14] = L_PDU;

bp->log_state.dbg_name[15] = "OIDANOMLY";   bp->log_state.dbg_flag[15] = L_OIDANOMLY;
bp->log_state.dbg_name[16] = "ASN1IN";      bp->log_state.dbg_flag[16] = L_ASN1IN;
bp->log_state.dbg_name[17] = "ASN1OUT";     bp->log_state.dbg_flag[17] = L_ASN1OUT;
bp->log_state.dbg_name[18] = "ASN1TRAP";    bp->log_state.dbg_flag[18] = L_ASN1TRAP;
bp->log_state.dbg_name[19] = "ASN1";        bp->log_state.dbg_flag[19] = L_ASN1;
bp->log_state.dbg_name[20] = "TRACE";       bp->log_state.dbg_flag[20] = L_TRACE;

/* establish default log_state: No debug, L_SYSLOG class, no logfile name  */
/* no log file:                                                            */
bp->log_state.debugging_mode = FALSE;   /* No Debugging Mode yet           */
bp->log_state.log_classes = L_SYSLOG;   /* Logging to SYSLOG always on     */
bp->log_state.log_file_name = NULL;     /* No log file name specified yet  */
bp->log_state.log_file = NULL;          /* No log file at all yet          */
bp->in_port = 0;                        /* No port specified from cmdline  */
bp->out_port = 0;                       /* No port specified from cmdline  */

/* establish default for config file name: NULL */
bp->config_file_name = NULL;

/* while (next argument is returned) */
while ( (next_arg = init_next_cmdarg(argc, argv, &hyphenated)) != NULL ) {

    /* if argument is hyphenated (e.g.: "-d") */
    if (hyphenated == TRUE) {

        /* step over the hyphen */
        next_arg += 1;

        /* This switch should be dispatching on all valid arguments that  */
        /* are preceded with a "-".  Code here is responsible for checking*/
        /* for duplicates.                                                */

        switch ( *next_arg ) {

            case 'p':   /* "-p <inbound request port>" */

                /* if (next argument is returned) */
                if ( (next_arg = init_next_cmdarg(argc, argv, &hyphenated))
                    != NULL ) {

                    /* if (string fails to convert to binary correctly) */
                    t_port = strtol(next_arg, &port_cnv_char, 10);
                    if (   hyphenated == TRUE
                        || port_cnv_char == next_arg
                        || t_port == 0
                        || *next_arg == '\0'
                       ) {
                        fprintf(stderr, MSG(msg030, "M030 - invalid -p port value\n"));
                        exit(0);
                        }

                    /* convert to network byte order and store in bp */
                    bp->in_port = htonl( t_port );
                    }
                else {
                    fprintf(stderr, MSG(msg031, "M031 - Missing inbound port number\n"));
                    exit(0);
                    }
                break;


            case 't':   /* "-t <outbound trap port>" */

                /* if (next argument is returned) */
                if ( (next_arg = init_next_cmdarg(argc, argv, &hyphenated))
                    != NULL ) {

                    /* if (string fails to convert to binary correctly) */
                    t_port = strtol(next_arg, &port_cnv_char, 10);
                    if (   hyphenated == TRUE
                        || port_cnv_char == next_arg
                        || t_port == 0
                        || *next_arg == '\0'
                       ) {
                        fprintf(stderr,
                                MSG(msg032, "M032 - invalid outbound -t port value\n"));
                        exit(0);
                        }

                    /* convert number to network byte order and store in bp */
                    bp->out_port = htonl( t_port );
                    }
                else {
                    fprintf(stderr, MSG(msg033, "M033 - Missing outbound port number\n"));
                    exit(0);
                    }
                break;



            case 'd':   /* "-d <debug log file name>" */

                /* if (next argument is returned) */
                if ( (next_arg = init_next_cmdarg(argc, argv, &hyphenated))
                    != NULL ) {

                    /* if (log file name already present or arg hyphenated) */
                    if (   (bp->log_state.log_file_name != NULL)
                        || (hyphenated == TRUE)) {

                        fprintf(stderr, MSG(msg005, "M005 - invalid -d usage\n"));
                        exit(0);
                        }

                    /* set debugging mode TRUE */
                    bp->log_state.debugging_mode = TRUE;

                    /* set argument value as log file name */
                    bp->log_state.log_file_name = next_arg;
                    }

                else {
                    fprintf(stderr, MSG(msg006, "M006 - Missing debug logfile name\n"));
                    exit(0);
                    }
                break;


            case 'c':   /* "-c <configuration file name>" */

                /* if (next argument is returned) */
                if ( (next_arg = init_next_cmdarg(argc, argv, &hyphenated))
                    != NULL ) {

                    /* if (config name already present or arg is hyphenated) */
                    if (   (bp->config_file_name != NULL)
                        || (hyphenated == TRUE)) {
                        fprintf(stderr, MSG(msg007, "M007 - invalid -c usage\n"));
                        exit(0);
                        }

                    /* set argument value as log file name */
                    bp->config_file_name = next_arg;
                    }

                else {
                    fprintf(stderr,
                            MSG(msg008, "M008 - Missing configuration file name\n"));
                    exit(0);
                    }
                break;


            default:    /* a bogus "-" */
                fprintf(stderr,
                        MSG(msg009, "M009 - invalid command line argument: '%s'\n"),
                        next_arg);
                exit(0);
            }
        }

    else {

       /* argument is not hyphenated: this implies that it is the name */
       /* of a "debug info class" of messages we want to see in debug  */
       /* logging file.                                                */

        /* signal "debugging mode" */
        bp->log_state.debugging_mode = TRUE;

        /* for (every legal debug class) */
        for (i=0; i < LOG_CLASS_COUNT; i++) {

            /* if (argument caseless matches legal debug class) */
            if (strcasecmp(bp->log_state.dbg_name[i], next_arg) == 0) {

                /* set corresponding bit in log_class */
                bp->log_state.log_classes |= bp->log_state.dbg_flag[i];

                /* set pointer to argument to NULL: success */
                next_arg = NULL;
                break;
                }
            }

        /* if (argument ptr is NON-NULL) */
        if (next_arg != NULL) {
            fprintf(stderr, MSG(msg010, "M010 - invalid debug class: '%s'\n"),
                    next_arg);
            exit(0);
            }
        }
    }

/* All command line arguments have been processed: apply config file name */
/* logic (see snmppe_main.c).                                             */

/* if (we're debugging and no -c value was given) */
if (bp->log_state.debugging_mode == TRUE  &&  bp->config_file_name == NULL) {

    /* if ( environment variable "ECA_SNMPPE_CONFIG_FILE" has a value) */
    if ( (environ_value = getenv("ECA_SNMPPE_CONFIG_FILE")) != NULL) {
        /* use environment variable as config file name */
        bp->config_file_name = environ_value;
        }
    }

/* if (no config file name has been specified) */
if (bp->config_file_name == NULL) {
    /* use builtin name */
    bp->config_file_name = DEFAULT_SNMPPE_CONFIG_PATH;
    }

#ifdef MTHREADS
/* <TC> In function init_cmdline_args() . . .                           */
/* <TC> Initialize the mutex associated with the log file regardless of */
/* <TC> whether we'll be writing to a log file or just using syslog()   */
/* initialize mutex for log file */
if (pthread_mutex_init(&bp->log_state.log_file_m, pthread_mutexattr_default))
    != 0) {
    fprintf(stderr, MSG(msg011, "M011 - pthread mutex initialization failed: %s\n"),
            strerror(errno));
    exit(0);
    }
#endif

}

/* init_next_cmdarg - Fetch next command line argument as a string */
/* init_next_cmdarg - Fetch next command line argument as a string */
/* init_next_cmdarg - Fetch next command line argument as a string */

static char *
init_next_cmdarg( argc, argv, hyphenated)

int     argc;           /* Count of original inbound command line arguments */
char    *argv[];        /* Array of pointers to strings of cmd line args    */
BOOL    *hyphenated;    /* On return TRUE: if next arg starts with "-"      */

/*
INPUTS:

    "argc" and "argv" constitute the description of the command line
    used to start the SNMP PE.  See the "INPUTS" section for function "main()"
    (in snmppe_main.c) for a complete description of these arguments.

    "hyphenated" is a boolean used to indicate on return that the string
    being returned originally started with "-".


OUTPUTS:

    The function returns the next command line argument as a string 
    or NULL if there are no more.

BIRD'S EYE VIEW:
    Context:
        The caller is the init_cmdline_arg() function of SNMP PE.  It
        needs to get the next argument from the command line.

    Purpose:
        This function sequentially returns each command line argument
        while signalling whether or not it is preceded by a "-".


ACTION SYNOPSIS OR PSEUDOCODE:

    if (next index >= argc )
        <return NULL>

    <set hyphenated according to next argument>
    <return pointer to next argument while incrementing counter>


OTHER THINGS TO KNOW:

    A local static variable keeps track of where we are in the array
    of command strings.  (Yeah, this breaks the "no static data" rule,
    but how much trouble are we in here by doing this anyway?)
*/

{
static int next=1;        /* Index of next arg to be returned */

/* if (next index >= argc ) */
if (next >= argc)
    return (NULL);

/* set hyphenated according to next argument */
*hyphenated = (*argv[next] == '-') ? (TRUE) : (FALSE);

/* return pointer to next argument while incrementing counter */
return (argv[next++]);

}

/* init_open_log - Initialize our logging sink according to cmdline args */
/* init_open_log - Initialize our logging sink according to cmdline args */
/* init_open_log - Initialize our logging sink according to cmdline args */

static void
init_open_log( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.  This function is concerned with finishing the
    initialization of the log_state structure in the big picture.


OUTPUTS:

    The function returns only on satisfactory opening of the logging sink,
    either the system log or a user-specified log file.

        * bp->log_state:
                * log_file -- open occurs if debug mode.


BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to get the logging sink open so all other initialization
        functions can log errors into the logging sink.

    Purpose:
        This function examines the current state of the "log_state"
        portion of the "big picture" to open the proper logging sink
        according to the command line arguments already parsed.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (logging mode is "debugging")

        if (no logging file supplied)
            <initialize to use stderr>
        else
            if (attempt to open for create/append the specified file failed)
                <stderr log: "Mxxx - attempt to open file %s" failed>
                <exit>

    else (* we're not debugging: normal operations to system log *)
        open system log


OTHER THINGS TO KNOW:

    Any errors that occur before we return from this function are logged
    to "stderr" and are fatal.

*/

{
int     syslog_code;            /* Value returned by openlog() call */
int     openlog();

/* if (logging mode is "debugging") */
if (bp->log_state.debugging_mode == TRUE) {

    /* We're in debugging mode.  If there is no file name, we use stderr */
    /* if (no logging file supplied) */
    if (bp->log_state.log_file_name == NULL) {
        bp->log_state.log_file = stderr;        /* initialize to use stderr */
        }

    else {      /* There is a filename, try to open it */

        /* if (attempt to open for create/append the specified file failed) */
        if ( (bp->log_state.log_file = fopen(bp->log_state.log_file_name, "a"))
            == NULL) {

            fprintf(stderr, MSG(msg012, "M012 - attempt to open file '%s' failed: %s\n"),
                    bp->log_state.log_file_name,
                    strerror(errno));
            exit(0);
            }
        }
    }

else {      /* we're not debugging: normal operations to system log */

    /* open system log */
#if defined(__osf__) || defined(sun) || defined(sparc)
    syslog_code = openlog("snmp_pe", LOG_PID, LOG_DAEMON);
#else
    syslog_code = openlog("snmp_pe", LOG_PID);
#endif
    }
}

/* init_config - Open and Parse the Configuration File */
/* init_config - Open and Parse the Configuration File */
/* init_config - Open and Parse the Configuration File */

static void
init_config( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE can
    begin operation.  This function is concerned with initializing:

    bp->
          * community_list
          * trap_list

    using:

    bp->
          * config_file_name

OUTPUTS:

    The function returns the big picture structure initialized with info
    extracted from the configuration file.  Specifically the list of
    communities recognized by SNMP PE is created as is the list of
    trap communities where SNMP PE should send traps is created.


BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to suck in all the info in the configuration file.

    Purpose:
        This function opens and parses all the information contained
        in the configuration file.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the verb list>
    <initialize the list headers in the bp>
    <initialize big picture "enable authentication traps" flag (enabled)>


    if (attempt to open configuration file failed)
       <crash exit "open failed on config file: %s">

    while (lines remain to be read)

        <count another line>

        <span pointer into line over any whitespace at start of line>
        if (current character is '0' or '#' or '\n')
            <continue>

        if (parse of initial token fails)
            <continue>

        <assume no verb match>
        for (all verbs in verb list)
            if (current verb caseless matches current first token)
                <set verb index>
                <break>

        switch (verb index)

            case VERB_COMMUNITY:

                <allocate a community list block and insert it at the
                 top of the community list in the big picture>
                <assume "read-only" and "0.0.0.0">

                if (parse of community name failed)
                    <crash exit
                     "Missing Community name, config file line %d">
                <duplicate the string to the community name field in the
                 community block>

                if (parse of the IP address failed)
                    <break>
                <convert IP address to binary and store in community block>

                if (parse of the access mode failed)
                    <break>
                <set access mode in community block according to parsed
                 value, crash exit if no match>
                <break>


            case VERB_TRAP:

                <allocate a community list block and insert it at the
                 top of the TRAP list in the big picture>

                if (parse of community name failed)
                    <crash exit
                     "Missing Trap Community name, config file line %d">
                <duplicate the string to the community name field in the
                 community block>

                if (parse of the IP address failed)
                    <crash exit
                    "Missing ID address, config file line %d">
                <convert IP address to binary and store in community block>
                <break>


            case VERB_NOAUTHTRAPS:

                if (parse of 'noauthtraps' directive failed)
                    <crash exit 
                 "Extraneous no_auth_traps value, config file line %d">
                <set big picture's "enable authentication traps" flag for later
                 use in setting 'enableauthentraps' in the snmp stats struct>
                <break>


            default: (* Unknown_Verb *)
                <crash exit "Unrecognized entry, config file line %d">

    <close the configuration file>


OTHER THINGS TO KNOW:

    The list of "verbs" (first non-blank string in each line of the
    configuration file) is found in the array "verbs[]" below.

    Any line with first non-whitespace character of "#" is a comment.

    Empty lines and lines containing whitespace only are ignored.

*/

#define VERB_COUNT 3            /* Number of Verbs in config file recognized */
{
FILE    *config;                /* Configuration File Descriptor pointer     */
char    *verbs[VERB_COUNT];     /* List of "verbs" we recognize from config  */
char    linebuf[LINEBUFSIZE];   /* Input line buffer                         */
char    msgbuf[LINEBUFSIZE];    /* Crash Error messages built here           */
                                /* are constructed here                      */
char    *next_char;             /* -> Next character in input line           */
int     lineno=0;               /* Line number in configuration file we're on*/
int     verb_index;             /* Index of recognized verb                  */
int     i;                      /* General purpose loop index                */
char    *temp_str;              /* Temporary string pointer                  */

community_t    *temp_community; /* Temporary pointer to a community block    */

extern int strcasecmp();


/* Initialize the Verb list
|
|  (The index into this array to the string is the "verb index" of that verb
|  and serves to indicate the selected verb when it is parsed from an
|  input line in the configuration file, we define the corresponding symbols
|  to make the code easier to read).
*/
#define VERB_COMMUNITY     0
verbs[VERB_COMMUNITY]      = "community";

#define VERB_TRAP          1
verbs[VERB_TRAP]           = "trap";
                   
#define VERB_NOAUTHTRAPS   2
verbs[VERB_NOAUTHTRAPS]    = "no_auth_traps";

#define UNKNOWN_VERB VERB_COUNT


/* initialize the list headers in the bp */
bp->community_list = NULL;
bp->trap_list = NULL;
bp->service_list = NULL;
bp->service_counter = 1;

/* initialize the event queue-related variables */
bp->event_queue_handle  = NULL;
bp->snmp_traps_disabled = 0;
bp->event_uid           = 1;

/* initialize big picture "enable authentication traps" flag (enabled) */
bp->authTrapFlag = 1;  /* 1 = enabled; 2 = disabled [per RFC 1213, pg. 66] */

/* if (attempt to open configuration file failed) */
if ( (config = fopen( bp->config_file_name, "r")) == NULL) {
    sprintf( msgbuf, MSG(msg014, "M014 - open failed on config file: '%s'"),
             bp->config_file_name);
    CRASH(msgbuf);
    }

/* while (lines remain to be read) */
while ( (next_char = fgets( linebuf, LINEBUFSIZE, config)) != NULL) {

    lineno += 1;    /* count another line */

    /* span pointer into line over any whitespace at start of line */
    next_char += strspn( next_char, " \t");

    /* if (current character is '0' or '#' or '\n') */
    if (*next_char == '\0' || *next_char == '#' || *next_char == '\n')
        continue;

    /* if (parse of initial token fails) */
    if ( (next_char = (char *) strtok( next_char, " \t\n")) == NULL)
        continue;

    verb_index = UNKNOWN_VERB;    /* assume no verb match */

    /* for (all verbs in verb list) */
    for (i=0; i < VERB_COUNT; i++) {

        /* if (current verb caseless matches current first token) */
        if ( strcasecmp( verbs[i], next_char ) == 0 ) {
            verb_index = i;     /* set verb index */
            break;
            }
        }

    switch (verb_index) {

        case VERB_COMMUNITY:
            /*
            |  We recognize a "Community" line accordingly:
            |
            |                  (Reqd)             (Reqd)             (Reqd)
            |    Verb           Arg1               Arg2               Arg3
            |    ----           ----               ----               ----
            |  community  <community name> <community inet-addr>  <accessmode>
            |
            |eg:
            |  community     PUBLIC                0.0.0.0         readonly
            |  community     OBSERVE               1.2.3.4         readonly
            |  community     MANAGE                1.2.3.5         readwrite
            |
            |  The "spec" for this is pages 212 and 213 of Rose's
            |  "The Simple Book" (SNMP), except we make all args mandatory.
            */

            /* allocate a community list block and insert it at the     */
            /* top of the community list in the big picture             */
            if ( (temp_community = (community_t *) malloc(sizeof(community_t)))
                == NULL) {
                CRASH(MSG(msg019, "M019 - Malloc fail in init_config()"));
                }

            /* Insert the block into the Community list in the Big Picture */
            temp_community->next = bp->community_list;
            bp->community_list = temp_community;

            /* assume "read-only" and "0.0.0.0" (don't really need to do */
            /* now that all community directive args are required)       */
            temp_community->access_mode = mode_readOnly;
            temp_community->comm_addr = 0;

            /* if (parse of community name failed) */
            if ( (temp_str = (char *) strtok(NULL, " \t\n")) == NULL) {
                sprintf( msgbuf,
                         MSG(msg015, "M015 - Missing Community name, config file line %d"),
                         lineno);
                CRASH(msgbuf);
                }

            /* duplicate the string to the community name field in the  */
            /* community block.                                         */
            if ( (temp_community->comm_name =
                     (char *) malloc( strlen(temp_str)+1 ) ) == NULL) {
                CRASH(MSG(msg020, "M020 - Malloc fail in init_config()"));
                }
            strcpy(temp_community->comm_name, temp_str);

            /* if (parse of the IP address failed) */
            if ( (temp_str = (char *) strtok(NULL, " \t\n")) == NULL) {
                sprintf( msgbuf,
                    MSG(msg246, "M246 - Missing inet address, config file line %d"), lineno);
                CRASH(msgbuf);
                }

            /* convert IP address to binary and store in community block */
            if ( (temp_community->comm_addr = inet_addr(temp_str)) == -1) {
                sprintf( msgbuf,
                  MSG(msg021, "M021 - Malformed inet address '%s', config file line %d"),
                         temp_str,
                         lineno);
                CRASH(msgbuf);
                }

            /* if (parse of the access mode failed) */
            if ( (temp_str = (char *) strtok(NULL, " \t\n")) == NULL) {
                sprintf( msgbuf,
                    MSG(msg250, "M250 - Missing access mode, config file line %d"), lineno);
                CRASH(msgbuf);
                }

            /* set access mode in community block according to parsed   */
            /* value, crash exit if no match                            */
            if ( strcasecmp( temp_str, "none" ) == 0) {
                temp_community->access_mode = mode_none;
                break;
                }
            else if( strcasecmp( temp_str, "readonly" ) == 0) {
                temp_community->access_mode = mode_readOnly;
                break;
                }
            else if ( strcasecmp( temp_str, "writeonly" ) == 0) {
                temp_community->access_mode = mode_writeOnly;
                break;
                }
            else if( strcasecmp( temp_str, "readwrite" ) == 0) {
                temp_community->access_mode = mode_readWrite;
                break;
                }
            else {
                sprintf( msgbuf,
                   MSG(msg022, "M022 - invalid access mode '%s', config file line %d"),
                         temp_str,
                         lineno);
                CRASH(msgbuf);
                }
            break;


        case VERB_TRAP:
            /*
            |  We recognize a "Trap" line accordingly:
            |
            |                 (Reqd)             (Reqd)
            |   Verb           Arg1               Arg2
            |   ----           ----               ----
            |   trap    <trap community name> <trap inet-addr>
            |eg:
            |   trap         PUBLIC             16.0.10.151
            |
            |  The "spec" for this is pages 212 and 213 of Rose's
            |  "The Simple Book" (SNMP).
            */

            /* allocate a community list block and insert it at the     */
            /* top of the TRAP community list in the big picture        */
            if ( (temp_community = (community_t *) malloc(sizeof(community_t)))
                == NULL) {
                CRASH(MSG(msg023, "M023 - Malloc fail in init_config()"));
                }

            /* Insert the block into the trap list in the Big Picture */
            temp_community->next = bp->trap_list;
            bp->trap_list = temp_community;

            /* if (parse of community name failed) */
            if ( (temp_str = (char *) strtok(NULL, " \t\n")) == NULL) {
                sprintf( msgbuf,
                     MSG(msg016, "M016 - Missing Trap Community name, config file line %d"),
                         lineno);
                CRASH(msgbuf);
                }

            if ( (temp_community->comm_name =
                     (char *) malloc( strlen(temp_str)+1 ) ) == NULL) {
                CRASH(MSG(msg024, "M024 - Malloc fail in init_config()"));
                }
            strcpy(temp_community->comm_name, temp_str);

            /* if (parse of the IP address failed) */
            if ( (temp_str = (char *) strtok(NULL, " \t\n")) == NULL) {
                sprintf( msgbuf,
                      MSG(msg220, "M220 - Missing Trap INET Address, config file line %d"),
                         lineno);
                CRASH(msgbuf);
                }

            if ( (temp_community->comm_addr = inet_addr(temp_str)) == -1) {
                sprintf( msgbuf,
                         MSG(msg025, "M025 - Malformed inet address, config file line %d"),
                         lineno);
                CRASH(msgbuf);
                }
            break;


        case VERB_NOAUTHTRAPS:
            /*
            |  We recognize a proprietary "authtrap" directive accordingly:  
            |
            |   Verb         
            |   ----         
            |   no_auth_traps
            |eg:
            |   no_auth_traps
            |
            |  There is no "spec" for this verb; this is a Digital-only
            |  way to make the enabling & disabling of authentication traps
            |  "configurable".  If specified, the authTrapFlag in the "big
            |  picture" is set to 2 (disabled), else 1 (enabled).  The
            |  authTrapFlag value in the "big picture" is used later to init
            |  the shared memory SNMP variable 'enableauthentrap'.
            */

            /* if (parse of 'no_auth_traps' directive failed) */
            if ((temp_str = (char *) strtok(NULL, " \t\n")) != NULL) 
            {
                sprintf( msgbuf,
                   MSG(msg245, "M245 - Extraneous no_auth_traps value, config file line %d"),
                         lineno);
                CRASH(msgbuf);
            }
            else /* it parsed OK; remember to disable authentication traps */
                bp->authTrapFlag = 2;
            break;


        default:    /* Unknown_Verb */
             sprintf( msgbuf,
                     MSG(msg026, "M026 - Unrecognized entry, config file line %d"),
                     lineno);
             CRASH(msgbuf);
        }
    }

/* close the configuration file */
fclose(config);

}

/* init_shared_memory - Setup Shared Memory Data Structure */
/* init_shared_memory - Setup Shared Memory Data Structure */
/* init_shared_memory - Setup Shared Memory Data Structure */

static void
init_shared_memory( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.


OUTPUTS:

    The function returns only on satisfactory creation of the
    shared memory structure and the initialization of the bp cell:

        * bp->statistics

    (and if running with threads:

        * bp->statistics_m ... mutex for "statistics")


BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to set up the shared memory area that it shares with the
        MOM so that during normal operation it can increment the necessary
        cells.

    Purpose:
        This function performs all chores associated with the creation
        and initialization of this shared memory structure.


ACTION SYNOPSIS OR PSEUDOCODE:

ifnot NOIPC
    if (attempt to get shared memory segment failed)
        if (attempt to create shared memory segment failed)
            <crash exit "Mxxx - unable to create snmp variables shared memory
             errno = %d>

    if (attempt to attach to shared memory segment failed)
        <crash exit "Mxxx - unable to attach to snmp variables shared memory
         errno = %d>

else
    <init big picture cell to point to local storage>
endif

    if (shared memory is not initialized)
        <zero the entire shared memory storage>
        <set the "sysuptime" and "sysobjid" values in shared memory>
        <issue coldStart TRAP>
        <set the "coldStartTrapSent" flag in the shared memory>
        <set the "initialized" flag in the shared memory>

    else (* shared memory is already initialized *)
        if (a coldStart TRAP has not yet been sent)
            <issue coldStart TRAP>
            <set the "coldStartTrapSent" flag in the shared memory>
        else
            <issue warmStart TRAP>  (* the PE has been re-started *)

    if ("enableauthtrap" is not legal (1 or 2))
        <set according to the following:
            a) the value specified for the "authtrap" verb in the configuration 
               file, or, if the "authtrap" directive was not specified,
            b) set to 1 (enabled) if we have a trap list, else 2 (disabled)>

if MTHREADS
    <initialize the mutex associated with the statistics block>
endif


OTHER THINGS TO KNOW:

    The shared memory structure is defined by include file "snmp_mib.h"
    (and at first writing, is included by an INCLUDE statement in "snmppe.h").

    The shared memory "key" value is defined in "snmp_mib.h".

    There is a lot of hanky-panky going on here with the 
    "enableauthtrap" variable in the shared-memory section:

        * If SNMP PE creates the shared-memory region, then we set this value
          according to:
            a) the setting of "authtrap" directive in the configuration file, 
               or, if the authtrap verb was not specified,
            b) whether or not a list of trap communities was provided
               in the configuration file.

        * If SNMP PE discovers this shared-memory region is already present,
          it determines whether or not to send a cold start trap (if the IMOM
          initialized the memory, it leaves the "coldStartTrapSent flag
          zeroed), or whether to send a warm start trap (the shared memory
          was previously initialized by either a previous SNMP-PE or by the
          IMOM) AND a cold start trap has already been sent.

    
    We crash-exit if any of this initialization fails.
*/

{
key_t           key;    
int             size, flag, i;
int             release;
man_status      status;
char            msgbuf[LINEBUFSIZE]; /* Construct crash-exit messages here    */
int             shared_mem_id;    /* Id used in system calls to establish mem */


#ifndef NOIPC   /* Multiple Images */
/* ========================================================================= */


/* if (attempt to 'get' [existing] shared memory segment failed) */
key  = SHARED_MEM_KEY;
size = sizeof (struct snmp_stats);
flag = 0;  /* initially the PE tries to 'get' an existing shared mem segment */

if ((shared_mem_id = shmget (key, size, flag)) < 0) {

    /* It isn't already there, try to create it. . . (for Read/Write access) */
    flag = IPC_CREAT | SHM_R | SHM_W;  /* Create | Read | Write = 01600 */

    if ((shared_mem_id = shmget (key, size, flag)) < 0) {

        sprintf(msgbuf,
        MSG(msg027, "M027 - unable to create snmp variables shared mem, '%s', errno = %d"),
                strerror(errno), errno);
        CRASH(msgbuf);
        }
    }

/* if (attempt to attach to shared memory segment failed) */
if ((bp->statistics = (struct snmp_stats *) shmat(shared_mem_id, 0, 0)) == 0) {
    sprintf(msgbuf,
        MSG(msg028, "M028 - unable to attach to snmp variables shared memory, errno = %d"),
        errno);
    CRASH(msgbuf);
    }

#else   /* Single Image */

/* init big picture cell to point to local storage */
bp->statistics = &bp->s_stats;
bp->statistics->initialized = 0;

#endif /* NOIPC */

/* initialize shared memory segment if it is not initialized */
if (bp->statistics->initialized != SNMP_IMOM_SHDMEM_INIT) 
{
    bzero((char *) bp->statistics, sizeof(struct snmp_stats));
    
    /*
     *  Set up the octet containing the system object identifier.
     *
     *  *** NOTE ***:
     *
     *  THE FOLLOWING CODE WAS LIFTED DIRECTLY OUT OF SYSTEM.C OF THE
     *  Inet_MOM CODE, with the return('failure_code') statements turned
     *  into CRASH's instead.  IF YOU CHANGE THIS BASIC LOGIC OF SETTING UP 
     *  SYSTEM OID, YOU NEED TO MAKE THE EQUIVALENT CHANGES IN SNMP_GROUP.C 
     *  (or VICE-VERSA) !!!!!!!!!
     */ 

    status = moss_oid_to_octet (&system_object_id_oid, &sysid_octet);
    if (status != MAN_C_SUCCESS)
    {
        sprintf(msgbuf,
        MSG(msg249, "M249 - moss_oid_to_octet for sysid_octet failed, '%s', errno = %d"),
                strerror(errno), errno);
        CRASH(msgbuf);
    }

    /* END OF STOLEN CODE FROM Inet_MOM's SYSTEM.C */


    /* Store system OID (sysobjid) and agent start-up time (sysuptime) in */
    /* snmp_stats structure for use by the SNMP-PE in building TRAP PDUs. */
    /* NOTE: The octet string is counted in bytes; the sysobjid value is  */
    /*       counted and stored as ints.                                  */
    bp->statistics->sysobjid[0] = (int ) (sysid_octet.length / sizeof(int ));
    for (i = 0; i < bp->statistics->sysobjid[0]; i++)
        bp->statistics->sysobjid[i+1] = sysid_octet.string[i*sizeof(int )];

    time ((time_t *) &bp->statistics->sysuptime);
    
    netio_send_trap (bp, NULL, NULL, coldStart, 0, NULL, NULL);  /* issue coldStart TRAP */
    bp->statistics->coldStartSent = 1;
    bp->statistics->initialized   = SNMP_IMOM_SHDMEM_INIT;
}
else /* shared memory is already initialized; determine which TRAP to send */
{
    /* send a coldStart TRAP if one has not yet been sent */
    if (bp->statistics->coldStartSent == 0)
    {
        netio_send_trap (bp, NULL, NULL, coldStart, 0, NULL, NULL);
        bp->statistics->coldStartSent = 1;
    }
    else /* send warmStart TRAP since a coldStart TRAP has already been sent */
    {
        netio_send_trap (bp, NULL, NULL, warmStart, 0, NULL, NULL);
    }
}

/* if ("enableauthtrap" is not legal (1=enabled or 2=disabled)) */
if (bp->statistics->enableauthtrap != 1 && bp->statistics->enableauthtrap != 2)
{
    /* the value of bp->authTrapFlag should be 1 or 2... */
    if (bp->authTrapFlag == 1 || bp->authTrapFlag == 2)
    {
        bp->statistics->enableauthtrap = bp->authTrapFlag;
    }
    else /* ...if not, set according to whether or not we have a trap list */

    {
        bp->statistics->enableauthtrap = (bp->trap_list == NULL) ? 2 : 1;
    }
}

#ifdef MTHREADS
/* initialize the mutex associated with the statistics block */
/* <TC> In function init_shared_memory() . . .                            */
/* <TC> Initialize the mutex associated with the shared memory statistics */
/* <TC> structure.                                                        */

/* <TC> Here it is being initialized, but the code that increments counters */
/* <TC> in the statistics block doesn't currently acquire this mutex before */
/* <TC> incrementing any counters.  I'm not sure a mutex is even really     */
/* <TC> worth the overhead!                                                 */

if (pthread_mutex_init(&bp->statistics_m, pthread_mutexattr_default)) != 0) {
    sprintf(msgbuf,
        MSG(msg029, "M029 - unable to init mutex for statistics block, errno = %d\n"),
        errno);
    CRASH(msgbuf);
    }
#endif

}

/* init_netio - Initialize Inbound and Outbound Network I/O */
/* init_netio - Initialize Inbound and Outbound Network I/O */
/* init_netio - Initialize Inbound and Outbound Network I/O */

static void
init_netio( bp )

big_picture_t   *bp;    /*-> Big Picture Structure to be initialized     */

/*
INPUTS:

    "bp" points to the Big Picture structure containing all the "global"
    context information that needs to be initialized before SNMP PE con
    begin operation.

    Cells "inbound" and "outbound" may have already been initialized with
    the 

OUTPUTS:

    The function returns only on satisfactory creation of the
    shared memory structure and the initialization of the bp cells:

        * bp->inbound
        * bp->in_socket
        * bp->outbound
        * bp->out_socket

    and if MTHREADS:

        * bp->in_m (mutex for inbound port)
        * bp->out_m (mutex for outbound port)



BIRD'S EYE VIEW:
    Context:
        The caller is the init_main() function of SNMP PE.  It
        needs to initialize network I/O in order to receive an inbound
        SNMP pdus via a connectionless UDP socket and to send traps outbound.

    Purpose:
        This function performs all chores associated with the 
        initialization of inbound and outbound-trop network I/O.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (no inbound port was supplied on the command line)
        if (attempt to find "snmp" service failed)
            <crash exit "Mxxx - service for snmp not found by getservbyname()">

    if (attempt to acquire a socket failed)
        <crash exit "Mxxx - unable to acquire internet socket. errno=%d">

    <build sockaddr - internet style>

    if (attempt to bind inbound socket in internet domain failed)
        <crash exit "Mxxx - unable to bind socket in internet domain. errno=%d">

    if (no trap port was supplied on the command line)
        if (attempt to find "snmp-trap" service failed)
            <crash exit "Mxxx - service for snmp not found by getservbyname()">

    if (attempt to acquire a socket failed)
        <crash exit "Mxxx - unable to acquire internet socket. errno=%d">

    <build sockaddr - internet style>

    if (attempt to get host name failed)
        <crash exit "Mxxx - Unable to acquire host name>

    if (attempt to get host address failed)
        <crash exit "Mxxx - Unable to acquire host address>

    <Copy host addressing info into the Big Picture for use by trap builder>

OTHER THINGS TO KNOW:

    We crash-exit if any of this initialization fails.

*/

#define H_NAMESIZE 32

{
char            h_name[H_NAMESIZE];     /* Host Name w/null byte             */
struct hostent  *host_info;             /* ->Host Info from system database  */
struct servent  *dap;                   /* Service Description structure     */
char            buf[LINEBUFSIZE];       /* Message buffer                    */
struct sockaddr_in inbound;             /* Inbound Socket address            */
extern int      socket();
extern int      bind();
extern int      gethostname();

/* if (no inbound port was supplied on the command line) */
if (bp->in_port == 0) {
    /* if (attempt to find "snmp" service failed) */
    if ( (dap = getservbyname("snmp", "udp")) == NULL) {
        CRASH(
          MSG(msg034, "M034 - service for 'snmp' 'udp' not found by getservbyname()"));
        }
    bp->in_port = (u_short) dap->s_port;   /* Copy to bp, network byte order */
    }

/* if (attempt to acquire a socket failed) */
if ( (bp->in_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sprintf(buf, MSG(msg035, "M035 - unable to acquire internet socket. '%s' errno=%d"),
            strerror(errno), errno);
    CRASH(buf);
    }

/* build sockaddr - internet style */
bzero(&inbound, sizeof(inbound));
inbound.sin_family = AF_INET;
inbound.sin_port = (u_short) bp->in_port;

/* if (attempt to bind inbound socket in internet domain failed) */
if ( bind( bp->in_socket, &inbound, sizeof(inbound)) < 0) {
    sprintf(buf,
            MSG(msg036, "M036 - unable to bind socket in internet domain. '%s' errno=%d"),
            strerror(errno), errno);
    CRASH(buf);
    }

/* ========================================================================= */
/*                      Now do the outbound trap port                        */
/* ========================================================================= */

/* if (no outbound port was supplied on the command line) */
if (bp->out_port == 0) {
    /* if (attempt to find "snmp-trap" service failed) */
    if ( (dap = getservbyname("snmp-trap", "udp")) == NULL) {
        CRASH(MSG(msg037, "M037 - service for 'snmp-trap' 'udp' not found by getservbyname()"));
        }
    bp->out_port = dap->s_port;        /* Copy to bp, network byte order */
    }

/* if (attempt to acquire a socket failed) */
if ( (bp->out_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    sprintf(buf, MSG(msg038, "M038 - unable to acquire internet socket. '%s' errno=%d"),
            strerror(errno), errno);
    CRASH(buf);
    }

/* build sockaddr - internet style */
bzero(&bp->outbound, sizeof(bp->outbound));
bp->outbound.sin_family = AF_INET;
bp->outbound.sin_port = (u_short) bp->out_port;

/* if (attempt to bind outbound socket in internet domain failed) */
/*********************************************************************** 

11-30-92, V1.0: Do NOT bind the outbound socket to port 162!!!!!!

if ( bind( bp->out_socket, &bp->outbound, sizeof(bp->outbound)) < 0) {
    sprintf(buf,
            MSG(msg039, "M039 - unable to bind socket in internet domain. '%s' errno=%d"),
            strerror(errno), errno);
    CRASH(buf);
    }
***********************************************************************/

/* if (attempt to acquire our name failed) */
if (gethostname(h_name, H_NAMESIZE) < 0 ) {
    sprintf(buf,
            MSG(msg214, "M214 - Unable to acquire host name '%s', errno = %d"),
            strerror(errno),
            errno
            );
    CRASH(buf);
    }

/* if (attempt to acquire our host info failed) */
if ((host_info = gethostbyname(h_name)) == NULL ) {
    sprintf(buf,
            MSG(msg215, "M215 - Unable to acquire host information '%s', errno = %d"),
            strerror(errno),
            errno
            );
    CRASH(buf);
    }

/* Copy host addressing info into the Big Picture for use by trap builder */
bzero(&bp->our_addr, sizeof(struct sockaddr_in));       /* Zap storage */
bcopy(host_info->h_addr,                /* From here . . . */
      &bp->our_addr.sin_addr,           /* To here . . .   */
      host_info->h_length               /* For this much . */
      );
bp->our_addr_length = host_info->h_length;

}
