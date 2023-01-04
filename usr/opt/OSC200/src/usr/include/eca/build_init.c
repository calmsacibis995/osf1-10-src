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
static char *rcsid = "@(#)$RCSfile: build_init.c,v $ $Revision: 1.1.2.5 $ (DEC)$Date: 1993/09/07 20:16:27 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	INIT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**      It performs required initialization and enrolls the
**	object class(es).
**
**  AUTHORS:
**
**      [[author]]
**
**      This code was initially created with the 
**	[[system]] MOM Generator - version [[version]]
**
**  CREATION DATE:  [[creation_date]]
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"
#ifdef SNMP
#include "moss_i_table.h"
#endif

#include "export_oids.h"
#include "extern_common.h"
#include "syslog.h"

/** TRAP polling and generation in a separate thread **/

/*
 * Define SNMP_TRAPS in the Makefile if using the trap polling routine
 * poll_for_trap_conditions() found in this module.
 */

/*
 * Include file for thread support.
 */

#ifdef SNMP_TRAPS

#include <pthread.h>
static pthread_startroutine_t poll_for_trap_conditions ();

#endif

/** END TRAP polling and generation in a separate thread **/


/** MOM specific #includes go here. **/

/* Size of error message for system logging */
#define MAXERRMSG 300

/*
 * Global data to be used in mom_shutdown().
 */

static management_handle *mom_handle;

#ifndef VMS
/*
 * Global data to be used for matching instances.
 */
extern  comparison  universal_table[] ;
#ifdef SNMP
extern  comparison  inet_application_table[] ;
	comparison  *table_array [] = {
                                      universal_table,          /* universal */
                                      inet_application_table,   /* snmp application */
                                      NULL,                     /* context specific */
                                      NULL,                     /* private */
                                      NULL
                                      };
#else
extern	comparison  dna_application_table[] ;
        comparison  *table_array [] = {
                                      universal_table,          /* universal */
                                      dna_application_table,    /* dna application */
                                      NULL,                     /* context specific */
                                      NULL,                     /* private */
                                      NULL
                                      };
#endif /* SNMP */
#endif /* VMS */

char *global_mom_name = "[[mom_name]]";		/* MOM name string */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      deregister_all
**
**	This routine deregisters all of the classes supported by this MOM
**	starting with all of child classes. 
**
**  FORMAL PARAMETERS:
**
**	mold_handle 	- mold handle to pass to deregister routine.
**	mom_handle	- mom handle to pass to deregister routine.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS or any failure from moss_deregister other than
**	MAN_C_NO_SUCH_CLASS.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status deregister_all( mold_handle,
		       	 	  mom_handle )

man_binding_handle mold_handle;
management_handle *mom_handle;

{
man_status status;

/*-insert-code-dereg-all-*/

return MAN_C_SUCCESS;
} /* End of deregister_all() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      mom_shutdown
**
**	This routine deregisters the [[mom_name]] MOM and exits.
**
**  FORMAL PARAMETERS:
**
**      None
**
**  RETURN VALUE:
**
**      None
**
**  SIDE EFFECTS:
**
**
**--
*/

/* +++
**     Add any MOM-specific shutdown code here.
*/

static void mom_shutdown()
{
    man_binding_handle mold_handle;
    man_status	status;

    status = moss_alloc_mold_handle( &mold_handle );
    if ERROR_CONDITION( status )
        return;

    status = deregister_all( mold_handle, mom_handle );
    if ERROR_CONDITION( status )
        return;
    
    status = moss_free_mold_handle( mold_handle );
    if ERROR_CONDITION( status )
        return;
} /* End of mom_shutdown() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**     mo_sig_handler
**
**     This is the MOM signal handler.  Once we return from moss_start_server
**     we can assume a signal has been received and we should clean up with
**     mold and then exit.
**
**  FORMAL PARAMETERS:
**
**	None
**
**  RETURN VALUE:
**
**	None
**
**--
*/
#ifndef NOIPC
void mo_sig_handler()
{
#ifdef MOMGENDEBUG
  printf("\n*** Unexpected signal encountered: In sig handler ***\n");
#endif 
  
mom_shutdown();
} /* End of mo_sig_handler() */
#endif /* NOIPC */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      mom_init
**
**	This routine registers the classes supported by the 
**      [[mom_name]] MOM. It then calls moss_start_server to
**	receive management requests from the Common Agent and
**	waits.
**
**  FORMAL PARAMETERS:
**
**      None
**
**  RETURN VALUE:
**
**      Any error status.
**
**  SIDE EFFECTS:
**
**      The class is registered.  This routine only exits upon an
**	condition during the registration process.
**
**--
*/
/**+++
**   Use this entry point if you want to use a
**   startup daemon.
**/
man_status mom_init()
{
    man_binding_handle mold_handle;
    int num_threads;
    man_status	status;
    char msg[MAXERRMSG];	/* buffer for logging messages to syslog */

    /*  Open the system log file */
#if defined(__osf__)
    openlog("[[mom_target]]", LOG_PID, LOG_DAEMON);
#else
    openlog("[[mom_target]]", LOG_PID);
#endif

#ifdef MOMGENDEBUG
    printf("In [[mom_name]] INIT \n");
#endif

/** MOM-specific initialization here. **/

    /*-insert-code-perform-inits-*/

    /*-insert-code-init-trap-*/

    status = moss_create_management_handle( &mom_handle );
    if ERROR_CONDITION( status )
        return status;

    status = moss_alloc_mold_handle( &mold_handle );
    if ERROR_CONDITION( status )
        return status;

    /*-insert-code-register-*/

    status = moss_free_mold_handle( mold_handle );
    if ERROR_CONDITION( status )
        return status;

    num_threads = [[num_threads]];

/** Place call to any general initialization function here. **/


#ifdef MOMGENDEBUG
    printf("Done INIT \n");
#endif

#ifdef SNMP_TRAPS

/** TRAP polling and generation in a separate thread **/

/*
 * Define SNMP_TRAPS in the Makefile if using the trap polling routine
 * poll_for_trap_conditions() found in this module.
 */

    {
      pthread_t trap_polling_thread;
      int trap_polling_args = 1;
      int thread_status;

      thread_status = pthread_create( &trap_polling_thread,
                                    pthread_attr_default,
                                    ( pthread_startroutine_t )
                                      poll_for_trap_conditions,
                                    ( pthread_addr_t )
                                      &trap_polling_args );

      if ( thread_status != 0 )
      {
        fprintf( stderr, "pthread_create() error\n\n" );
        exit(1);
      }

    }
/** END TRAP polling and generation in a separate thread **/

#endif


    sprintf( msg, "%s - Initialization complete...\n", "[[mom_target]]" );
    mom_log( msg, LOG_ERR);

#ifndef NOIPC
    status = moss_start_server( mo_sig_handler, num_threads, mom_handle );
    if ERROR_CONDITION( status )
        return status;
#endif /* NOIPC */

    return MAN_C_SUCCESS;
} /* End of mom_init() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      poll_for_trap_conditions
**
**      This is a template routine for the [[mom_name]] MOM for polling the
**      trap conditions and send the appropriate trap.  This routine
**      is generated by MOM Generator but controlled by the define symbol
**	SNMP_TRAPS.
**
**	Developer needs to determine how they would like to detect traps and
**	when to send the traps. Use this routine as a template. Make
**	modification if necessary.
**
**  FORMAL PARAMETERS:
**
**      arg
**
**  RETURN VALUES:
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/


/** TRAP polling and generation in a separate thread **/

/*
 * Define SNMP_TRAPS in the Makefile if using the trap polling routine
 * poll_for_trap_conditions() found in this module.
 */

/*
 * The polling interval is set by #define polling_interval_seconds
 * in this routine, make any change as appropriate.
 */

#ifdef SNMP_TRAPS

static pthread_startroutine_t poll_for_trap_conditions ( arg )

pthread_addr_t arg;

{
#define polling_interval_seconds 5

    int thread_status;
    struct timespec poll_interval;

/*-insert-code-trap-cond-*/

    poll_interval.tv_sec = polling_interval_seconds;
    poll_interval.tv_nsec = 0;

    while ( 1 )
    {

/*-insert-code-trap-polling-*/

	/*
	 * Delay for a while.
	 */

        thread_status = pthread_delay_np( &poll_interval );
        if ( thread_status != 0 )
        {
            fprintf( stderr, "pthread_delay_np() error\n\n" );
            pthread_exit(0);
        }
    }

/*
 * Remove the following comment around pthread_exit() if you ever exit
 * the above forever while loop, so as to cleanly exit this polling thread.
 */

/*

pthread_exit(0);

 */

} /* End of poll_for_trap_conditions() */

#endif

/** END TRAP polling and generation in a separate thread **/



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      main
**
**	This is the main routine for the [[mom_name]] MOM.  Its only function
**	is to call mom_init. This routine never returns on success.
**
**  FORMAL PARAMETERS:
**
**	None
**
**  RETURN VALUES:
**
**      Any error status from mom_init
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
#ifndef NOIPC
int main(
	 int  argc,
	 char *argv[]
	 )
{
    man_status status;

    /*
     * Check for superuser.
     */
    if ( getuid() ) {
	fprintf( stderr, "%s: not super user.\n", argv[ 0 ] ) ;
	exit( 1 ) ;
    } 
      
#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
# ifndef DEBUG
    {
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

      extern int dup2 (int, int);
      extern pid_t fork (void);
      extern int ioctl();
      int file_desc ;

      if ( fork() )
	exit ( 0 ) ;
      
      for ( file_desc= 0 ; file_desc< 10 ; file_desc++ )
	( void )close( file_desc) ;
      
      ( void )open( "/dev/null", O_RDONLY ) ;
      ( void )dup2( 0, 1 ) ;
      ( void )dup2( 0, 2 ) ;
      
      file_desc = open( "/dev/tty", O_RDWR ) ;
      if ( file_desc > 0 ) {
	( void )ioctl( file_desc, TIOCNOTTY, ( char * )NULL ) ;
	( void )close( file_desc ) ;
      }
    }
# endif
#endif


    status = mom_init();

    return (int) status;
} /* End of main() */
#endif /* NOIPC */

/* End of init.c */
