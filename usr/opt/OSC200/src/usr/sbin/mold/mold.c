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
static char *rcsid = "@(#)$RCSfile: mold.c,v $ $Revision: 1.1.2.8 $ (DEC) $Date: 1993/11/29 16:40:12 $";
#endif
#ifndef lint
static char *sccsid = "%W%	DECwest	%G%" ;
#endif

/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    The following routines contain the main entry point for mold,
 *    support routines, and general RPC interaction routines.
 *
 *  Routines:
 *
 *    main()
 *    initialize_mold()
 *    mold_signal_handler()
 *    find_object()
 *    create_object()
 *    insert_containment()
 *    remove_object()
 *    remove_containment()
 *    find_scoped_objects()
 *    compare_oid()
 *    allocate_object()
 *    copy_oid()
 *    hash()
 *    hash_table_hit()
 *    convert_oid()
 *    insert_lexi()
 *    remove_lexi()
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    October 25, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, March 1st, 1990
 *
 *    Put in checks after RPC library calls.
 *
 *    Miriam Amos Nihart, April 16th, 1990
 *
 *    Put in NOIPC ifdefs for the single image CA kit.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change include file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, June 26th, 1990.
 *
 *    Change the lb_$register call to register mold with only the local location broker.
 *
 *    Miriam Amos Nihart, July 4th, 1990.
 *
 *    Put in the #ifdef for using UNIX Domain RPC.
 *
 *    Miriam Amos Nihart, July 30th, 1990.
 *
 *    Put in more debug statements and cleanup around the RPC code and fix pfm_$ calls.
 *
 *    Miriam Amos Nihart, August 15th, 1990.
 *
 *    Put in the ULTRIX fork daemon code.  Ifdef with ULTRIX_KIT so it is only compiled
 *    for the ULTRIX OS distribution.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change the bzero to the ansi equivalent memset.
 *
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.
 *
 *    Miriam Amos Nihart, May 30th, 1991.
 *
 *    correct function declaration of hash() for OSF/1.
 * 
 *    Wim Colgate, August, 28th, 1991.
 *
 *    Added lexigraphic support for objects: This includes a new global variable
 *    lexi_head, and the following support routines: insert_lexi(), remove_lexi().
 *
 *    Kathy Faust, Sept 6th, 1991
 *
 *    Modified mold_v1_0_epv to specify mold_register_mom, mold_deregister_mom,
 *    mold_find_mom, and mold_find_next_mom.  Specified MAN_C_MOLD_MAX_CALLS
 *    for use in rpc_server_listen and rpc_server_use_protseq.
 *
 *    Miriam Amos Nihart, October 18th, 1991.
 *
 *    Perform lexi queue initialization at compile time.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    Removed "* DEBUG *" from after an #endif
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Update for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *	Mike Densmore, April 1992
 *
 *	Added cma.h to includes.
 *
 *      Adam Peller, July 20th, 1992
 *
 *      Ifndef'd out fork() for DEBUG
 *
 */

/*
 *  Support header files
 */

#include "man_data.h"

#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#include <sys/file.h>
#include <sys/ioctl.h>
#endif 

/*
 *  Header files
 */

#include "man.h"

/*
 *  MOLD Specific header files
 */

#include "mold_private.h"
#include "moss.h"
#include "mold_msg_text.h"

/*
 *  We need to include mir.h from MIR build area, but need to define macro
 *  OID_DEF in order not to redefine object_id defined in mir.h.
 */

#ifndef OID_DEF
#define OID_DEF
#endif

#include "mir.h"

#ifndef NOIPC
#include <cma.h>
#include "mold.h"
#include "mold_uuid.h"
#include "exc_handling.h"
#include "ca_config.h"
#endif /* NOIPC */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 *
 *   - ca_check_base_license() is in ca_lmf.c for ultrix
 *
 *   - getopt() is not defined anywhere in the system
 *
 *   - mir_free_mandle() is in mir_t0.c for MIR compiler
 *
 *   - mir_t0_init() is in mir_t0.c for MIR compiler
 *
 *   - mir_search_rel_table() is in mir_t0.c for MIR compiler
 *
 *   - mir_mandle_to_oid() is in mir_t0.c for MIR compiler
 *
 *   - mir_oid_to_mandle() is in mir_t0.c for MIR compiler
 */

extern int ca_check_base_license() ;
extern int getopt() ;
extern mir_status mir_free_mandle() ;
extern mir_status mir_t0_init() ;
extern mir_status mir_search_rel_table() ;
extern mir_status mir_mandle_to_oid() ;
extern mir_status mir_oid_to_mandle() ;

#ifndef NOIPC
extern int rpc_server_inq_bindings(),
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
#ifndef DEBUG

#include <unistd.h>
extern int cma_fork() ;
extern int cma_close() ;
extern int cma_open() ;
extern int cma_dup2() ;
extern int ioctl() ;

#endif /* DEBUG */
#endif 

/*
 * Include files for Internationalization on ultrix, OSF/1 and SunOS only.
 */

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifdef NL

#include <langinfo.h>
#include <locale.h>
#include <nl_types.h>

#endif /* NL */
#endif /* ultrix || __ultrix || __osf__ || sun || sparc */

/*
 *  Global Data
 */

family_relation_t *containment_head = 0 ; 
object_t *( hash_table[ MAN_C_HASH_VALUE ] ) ;
queue_t lexi_head = { &lexi_head, &lexi_head } ;

char *init_mir_filename = NULL ;

/*
 *  External data
 */

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifdef NL

extern nl_catd _m_catd;

#endif /* NL */
#endif /* ultrix || __ultrix || __osf__ || sun || sparc */

/*
 *  Data for RPC
 */

#ifndef NOIPC
globaldef mold_v1_0_epv_t mold_v1_0_m_epv = {
    mold_register_mom,
    mold_deregister_mom,
    mold_find_mom,
    mold_find_next_mom
} ;

error_status_t dummy_status ;

extern char *error_text();
extern void print_exception();

#endif /* NOIPC */

/*
 * Define a simple comparison macro.
 */

#define COMPARE( x, y ) \
        ( ((x) > (y)) ? MAN_C_GREATER_THAN : ((x) < (y)) ? MAN_C_LESS_THAN : MAN_C_EQUAL_TO )

man_status mold_populate_from_mir() ;
man_status mold_init_mir() ;
man_status preregister_objects() ;
man_status mold_preregister_object() ;
man_status get_class_mandle() ;
man_status mold_dump_init_mold() ;
man_status mold_dump_single_object() ;
void mold_signal_handler() ;


void mold_log(p_msg, syslog_code)
char *p_msg;
int   syslog_code;
/*
 * inputs:   p_msg is a pointer to the message to be written to the log.
 *
 *           syslog_code is the code needed for syslog call.
 *
 * outputs:  None.
 *
 * description:  Write a message to syslog().
 */
{
char    bigbuf[MAXERRMSG+1];/* Where we copy a message to add a newline*/
                            /* (+1 is for the NULL byte) */
char    *outmsg;            /* Pointer to final message to send */
int     in_msg_len;         /* Computed length of inbound message */

#ifdef __ultrix
    extern void syslog();
#endif

  /* if (it doesn't end in a newline and there is room) */

  if ( ( (in_msg_len = strlen(p_msg)) <  MAXERRMSG ) &&
         ( *(p_msg + in_msg_len - 1)    != '\n'     )     )
    {
    /* add a newline */

    strcpy(bigbuf, p_msg);            /* copy inbound message */
    bigbuf[in_msg_len] = '\n';      /* add '\n' over null byte */
    bigbuf[in_msg_len+1] = '\0';    /* add the '\0' */
    outmsg = bigbuf;                /* New outbound message is here */
    }
  else
    {
    outmsg = p_msg;   /* Just use the original message */
    }

  /* write message to syslog() */

  syslog(syslog_code, outmsg);

return;
}  /* end of mold_log() */



#ifndef NOIPC

void 
main(
     argc ,
     argv
    )
int argc ;
char *argv[] ;

/*
 *
 * Function Description:
 *
 *    Initialize everything. This includes the pfm signal handler to clean
 *    up after ourselves incase of death by fire....
 *
 * Parameters:
 *
 *    argc         argument count
 *    argv         argument vector
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    None
 *
 */

{
    int i ;
    int s ;
    man_status status ;
    char *mir_filename = DEFAULT_DATABASE_PATH ;
    char *user_mir_filename = NULL ;
    int c ;
    extern int optind, opterr ;
    extern char *optarg ;
    char *optstring = "d:--" ;
    int dump_mold_now = FALSE ;
    char msg[LINEBUFSIZE];
    int openlog();


#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifdef NL

    /*
     * For Internationalization, open the catalog file and init the runtime
     * locale.
     */

    _m_catd = catopen ( "mold.cat", NL_CAT_LOCALE ) ;
    setlocale ( LC_ALL, "" ) ;

#endif /* NL */
#endif /* ultrix || __ultrix || __osf__ || sun || sparc */

    /*
     * Check for CA Base Kit license
     */
    if ((ca_check_base_license()) != 0) {
	fprintf (stderr, MSG(mold_msg001, "\n%s:  License for Common Agent Base Kit (COM-AGNT-BAS) is not loaded, or \nLicense for Common Agent Developer's Toolkit (COM-AGNT-DEV) is not loaded\n"), argv[0]) ;
        fflush( stderr ) ;
	exit ( 1 ) ;
    }

#if defined(__osf__) || defined(sun) || defined(sparc)
    openlog("mold", LOG_PID, LOG_DAEMON);
#else
    openlog("mold", LOG_PID);
#endif

    /*
     * Check for superuser.
     */

    if ( getuid() )
    {
        fprintf( stderr, MSG(mold_msg002, 
                 "%s: not super user.\n"), argv[ 0 ] ) ;
        exit( 1 ) ;
    } 


#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifndef DEBUG

    if ( fork() )
        exit ( 0 ) ;

    for ( s = 0 ; s < 10 ; s++ )
        ( void )close( s ) ;

    ( void )open( "/", O_RDONLY ) ;
    ( void )dup2( 0, 1 ) ;
    ( void )dup2( 0, 2 ) ;

    s = open( "/dev/tty", O_RDWR ) ;
    if ( s > 0 )
    {
        ( void )ioctl( s, TIOCNOTTY, ( char * )NULL ) ;
        ( void )close( s ) ;
    }

#endif /* DEBUG */
#endif 

    /*
     * User can specify -d dumpfile option to dump the MOLD to a file
     * at startup time.
     */

    while ( ( c = getopt( argc, argv, optstring ) ) != EOF )
    {
#ifdef DEBUG
	fprintf ( stderr, "Option = %c, Argument = %s, Argv_index = %d\n",
		  c, optarg, optind ) ;
#endif
	switch ( c )
	{
	    case 'd' :
			/*
			 * We only support -d option with argument.
			 */

			dump_mold_now = TRUE ;
			break ;

	    case '?' :
			fprintf( stderr, 
				MSG(mold_msg003, "\nUsage: [-d dumpfile]\n") ) ;
			exit( 1 ) ;
			break ;
	}
    }

    if ( dump_mold_now == TRUE )
    {
	/*
	 * User specified -d with argument but we might have more
	 * arguments than we need.
	 */

	switch ( argc )
	{
	    case 2 :
			/*
			 * User specified the form -da.dmp, ok.
			 */
			break ;

	    case 3 :
			/*
			 * User must specify -d a.dmp to pass.
			 */
			if ( strcmp( argv[1], "-d" ) != 0 )
			{
			    fprintf( stderr,
				MSG(mold_msg004, "\nUsage: [-d dumpfile]\n") ) ;
			    exit( 1 ) ;
			}
			break ;

	    default :
			/*
			 * User specified -d a.dmp and more arguments.
			 */
			fprintf( stderr,
				MSG(mold_msg005, "\nUsage: [-d dumpfile]\n") ) ;
			exit( 1 ) ;
	}
    }
    else if ( argc > 1 )
    {
	/*
	 * If user does not specify -d, but we have argument(s),
	 * return error.
	 */

	fprintf( stderr, MSG(mold_msg006, "\nUsage: [-d dumpfile]\n") ) ;
	exit( 1 ) ;
    }

   /*
    * Populate the MOLD containment tree from the MIR.
    */

    status = mold_populate_from_mir() ;

    if ( status != MAN_C_SUCCESS )
    {
	sprintf( msg, MSG(mold_msg007, "%s - Error populating mold from mir...\n"),
		"MOLD: mold_populate_from_mir: " );
	mold_log(msg, LOG_ERR);
	exit( 1 ) ;
    }

    /*
     * We want to get the filename of the MIR so that whenever we are
     * asked to dump the MOLD, we can put this filename in the dump file.
     */

    /*
     * The default MIR file is defined in DEFAULT_DATABASE_PATH. The
     * user can override this with environment variable ECA_MIR.
     */

    user_mir_filename = ( char * ) getenv( "ECA_MIR" ) ;

    if ( user_mir_filename != NULL )
    {
	mir_filename = user_mir_filename ;
    }

    /*
     * Store the MIR filename in global variable init_mir_filename.
     */

    MALLOC( init_mir_filename, char *, strlen( mir_filename ) + 1 ) ;

    if ( init_mir_filename == NULL )
    {
	sprintf( msg, MSG(mold_msg008, "MOLD: main: memory allocation error\n\n") ) ;
	mold_log(msg, LOG_ERR);
	exit( 1 ) ;
    }

    strcpy( init_mir_filename, mir_filename ) ;

    /*
     * Dump the initial MOLD to a user file specified by -d option.
     */

    if ( dump_mold_now == TRUE )
    {
	status = mold_dump_init_mold( optarg ) ;
    }

    if ( status != MAN_C_SUCCESS )
    {
	sprintf( msg, MSG(mold_msg009, "%s - Error dumping the MOLD.\n"),
		"MOLD: mold_dump_init_mold: " ) ;
	mold_log(msg, LOG_ERR);
	exit( 1 ) ;
    }

   /*
    *  Set up MOLD.
    */

    sprintf( msg, MSG(mold_msg026, "mold (V%s.%s) initialization complete\n"),
             "1", "10");
    mold_log(msg, LOG_INFO);

    initialize_mold() ;

}  /* end of main() */


void
mold_signal_handler( )

/*
 *
 * Function Description:
 *
 *    This is our signal handler. If any of our specified signals
 *    go off, unregister mold from the location broker, and then exit.
 *
 * Parameters:
 *
 *    None
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    None
 *
 */


{
    error_status_t status = error_status_ok;
    char *prefix = "MOLD: mold_signal_handler: " ;
    uuid_vector_t obj_uuid_vec;
    rpc_binding_vector_t *binding_vec;
    char msg[LINEBUFSIZE];

    /*
     *  Clean up Handler for RPC call.
     */

    TRY

	rpc_server_inq_bindings ( &binding_vec, &status );

	if (status != error_status_ok)
	    {
	    sprintf(msg, MSG(mold_msg010, "%s - Error inquiring about bindings...\n"),prefix);
	    mold_log(msg, LOG_ERR);
	    exit(1);
	    }

        obj_uuid_vec.count = 1;
	obj_uuid_vec.uuid[0] = (uuid_t *) &mold_obj; 

	rpc_ep_unregister ( mold_v1_0_s_ifspec,
			    binding_vec,
			    &obj_uuid_vec,
			    &status);


	if (status != error_status_ok)
	    {
	    sprintf(msg, MSG(mold_msg011, "%s - Error unregistering with ep mapper...\n"),prefix);
	    mold_log(msg, LOG_ERR);
	    exit (1);
	    }

	rpc_binding_vector_free ( &binding_vec, &status );

	if (status != error_status_ok)
	    {
	    sprintf(msg, MSG(mold_msg012, "%s - Error freeing binding vector...\n"),prefix);
	    mold_log(msg, LOG_ERR);
	    exit (1);
	    }

  	rpc_server_unregister_if ( mold_v1_0_s_ifspec,
				   NULL,
				   &status );

	if (status != error_status_ok)
	    {
	    sprintf(msg, MSG(mold_msg013, "%s - Error unregistering with runtime...\n"),prefix);
	    mold_log(msg, LOG_ERR);
	    exit (1);
	    }

    CATCH_ALL

#ifdef DEBUG
        sprintf( msg, "%s RPC signal trying to unregister...\n", prefix);
	mold_log(msg, LOG_ERR);
	print_exception( THIS_CATCH ) ;
#endif

	rpc_binding_vector_free ( &binding_vec, &status );
       exit( 1 ) ;  

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY  

#ifdef DEBUG
    if ( status != error_status_ok )
    {
        sprintf( msg, "%s RPC error trying to unregister, %s\n", prefix,
		error_text (status) ) ;
	mold_log(msg, LOG_ERR);
    }
	
#endif /* DEBUG */

    exit( 1 ) ;

}  /* end of mold_signal_handler() */

#endif /* NOIPC */



man_status
mold_populate_from_mir()

/*
 *
 * Function description:
 *
 *    This routine populates the MOLD containment tree from the MIR starting
 *    from the SNMP with oid ( 1 3 6 1 2 1 ) under the ROOT of MOLD. All the
 *    entries in the containment tree are marked as preregistered.
 *
 * Arguments:
 *
 *    None
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful in populating MOLD from MIR
 *    MAN_C_FAILURE		Failure in populating MOLD from MIR
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status status = MAN_C_SUCCESS ;
    object_id root_oid ;
    object_id snmp_oid ;
    object_id rel_oid ;
    unsigned int snmp_oid_array[6] ;
    unsigned int rel_oid_array[9] ;
    mandle *snmp_mandle = NULL ;
    mandle *rel_mandle = NULL ;

    /*
     * Define the oid for the ROOT of the containment tree.
     */

    root_oid.count = 0 ;
    root_oid.value = NULL ;

    /*
     * Define the oid for the SNMP with oid ( 1 3 6 1 2 1 ).
     */

    snmp_oid_array[0] = 1 ;
    snmp_oid_array[1] = 3 ;
    snmp_oid_array[2] = 6 ;
    snmp_oid_array[3] = 1 ;
    snmp_oid_array[4] = 2 ;
    snmp_oid_array[5] = 1 ;

    snmp_oid.count = 6 ;
    snmp_oid.value = snmp_oid_array ;

    /*
     * Define the oid ( 1 3 12 2 1011 2 17 1 43 ) for the MIR relationship
     * name "MIR_Cont_entityClass". This need to be fixed so that it is not
     * hard-coded.
     */

    rel_oid_array[0] = 1 ;
    rel_oid_array[1] = 3 ;
    rel_oid_array[2] = 12 ;
    rel_oid_array[3] = 2 ;
    rel_oid_array[4] = 1011 ;
    rel_oid_array[5] = 2 ;
    rel_oid_array[6] = 17 ;
    rel_oid_array[7] = 1 ;
    rel_oid_array[8] = 43 ;

    rel_oid.count = 9 ;
    rel_oid.value = rel_oid_array ;

#ifndef NOIPC

    /*
     * Initialize for MIR lookup only for multiple image. For single image,
     * mold_populate_from_mir() is called from the PE but since PE has
     * already initialized the mir, we should not do it again.
     */

    status = mold_init_mir() ;

#endif

    /*
     * Get the mandle for the SNMP from the MIR.
     */

    if ( status == MAN_C_SUCCESS )
    {
	status = get_class_mandle( &snmp_oid, &snmp_mandle ) ;
    }

    /*
     * Get the mandle for the relationship name "MIR_Cont_entityClass"
     * from the MIR.
     */

    if ( status == MAN_C_SUCCESS )
    {
	status = get_class_mandle( &rel_oid, &rel_mandle ) ;
    }

    /*
     * Call the recursive routine preregister_objects() to recursively
     * register all the objects in the MIR under SNMP.
     */

    if ( status == MAN_C_SUCCESS )
    {
	status = preregister_objects( &root_oid,
				      &snmp_oid,
				      snmp_mandle,
				      rel_mandle ) ;
    }

    /*
     * Free resources
     */

    if ( snmp_mandle != NULL )
    {
	mir_free_mandle( &snmp_mandle ) ;
    }

    if ( rel_mandle != NULL )
    {
	mir_free_mandle( &rel_mandle ) ;
    }

    return( status ) ;

}  /* end of mold_populate_from_mir() */


#ifndef NOIPC


man_status
mold_init_mir()

/*
 *
 * Function description:
 *
 *    This routine initializes to read MIR.
 *
 * Arguments:
 *
 *    None
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful in initializing to read MIR
 *    MAN_C_FAILURE		Failure in initializing to read MIR
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{

    mir_status m_status ;
    char *mir_err_string ;
    int *mir_preamble_list = NULL ;
    char msg[LINEBUFSIZE];

#ifdef DEBUG
    fprintf( stderr, "Default mir file is %s\n", DEFAULT_DATABASE_PATH ) ;
#endif

    /*
     * Call mir_t0_init() with default MIR filename to initialize.
     * Also pass ECA_MIR environment variable for override if user defines it.
     * No paging of Non-terminals needed.
     */

    m_status = mir_t0_init( DEFAULT_DATABASE_PATH,
			    "ECA_MIR",
			    NULL,
			    FALSE,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    &mir_preamble_list ) ;

    if ( m_status != MS_SUCCESS )
    {
	sprintf( msg, "%s\n\n", mir_err_string ) ;
	mold_log(msg, LOG_ERR);
	return( MAN_C_FAILURE ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_init_mir() */

#endif /* NOIPC */



man_status
preregister_objects(
                    parent_class,
                    child_class,
                    child_class_mandle,
                    relationship_mandle
                   )

object_id *parent_class ;
object_id *child_class ;
mandle *child_class_mandle ;
mandle *relationship_mandle ;

/*
 *
 * Function description:
 *
 *    This is a routine which recursively calls itself to register
 *    all the objects under SNMP.
 *
 * Arguments:
 *
 *    parent_class		pointer to the parent oid
 *    child_class		pointer to the child oid
 *    child_class_mandle	pointer to the mandle for the child oid
 *    relationship_mandle	pointer to the mandle for the relationship
 *				"MIR_Cont_entityClass"
 *
 * Return value:
 *
 *    MAN_C_SUCCESS
 *    MAN_C_FAILURE
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status status ;
    mir_status s_status ;
    mir_status conv_status ;
    mandle_class *local_class_mandle = NULL ;
    mandle *grandchild_mandle = NULL ;
    mir_value m_value ;
    object_id grandchild_class ;
    search_style search ;
    mir_oid_smi oid_smi ;

    oid_smi = OID_SNMP ;
    search = SEARCH_FROMTOP ;

    /*
     * Register the child object under the parent object and mark it
     * as preregistered.
     */

    status = mold_preregister_object( parent_class, child_class ) ;

    if ( status != MAN_C_SUCCESS )
	return( status ) ;

    /*
     * We must set this to NULL for it is freed every time in the loop.
     */

    grandchild_class.value = NULL ;

    do
    {
	/*
	 * Search the MIR for the next grandchild mandle of child.
	 */

	s_status = mir_search_rel_table( search,
					 child_class_mandle,
					 relationship_mandle,
					 &local_class_mandle,
					 &grandchild_mandle,
					 &m_value ) ;

	/*
	 * If grandchild found, recursively call itself.
	 */

	if ( s_status == MS0_FIND_NONTERM )
	{
	    /*
	     * Get the oid of the grandchild from the mandle. We
	     * want the SNMP OID.
	     */

	    conv_status = mir_mandle_to_oid( grandchild_mandle,
					     &grandchild_class,
					     &oid_smi ) ;

	    if ( conv_status != MS_SUCCESS )
		status = MAN_C_FAILURE ;


	    if ( status == MAN_C_SUCCESS )
	    {
		/*
		 * After the first search above, we want to search
		 * from the last one found.
		 */

		search = SEARCH_FROMLAST ;

		/*
		 * Call itself recursively.
		 */

		status = preregister_objects( child_class,
					      &grandchild_class,
					      grandchild_mandle,
					      relationship_mandle ) ;
	    }

	    /*
	     * Free the oid and set to NULL for next allocation.
	     */

	    if ( grandchild_class.value != NULL )
	    {
		free( grandchild_class.value ) ;
		grandchild_class.value = NULL ;
	    }
	}

    } while ( ( s_status == MS0_FIND_NONTERM ) &&
	      ( status == MAN_C_SUCCESS ) ) ;

    if ( status == MAN_C_SUCCESS )
    {
	/*
	 * It is ok if we found none.
	 */

	if ( s_status == MS0_FIND_NONE )
	    status = MAN_C_SUCCESS ;
	else
	    status = MAN_C_FAILURE ;
    }

    /*
     * Free resources
     */

    if ( grandchild_mandle != NULL )
    {
	mir_free_mandle( &grandchild_mandle ) ;
    }

    return( status ) ;

}  /* end of preregister_objects() */



man_status
mold_preregister_object(
                        parent_class,
                        child_class
                       )

object_id *parent_class ;
object_id *child_class ;

/*
 *
 * Function description:
 *
 *    This routine calls register_containment to register child under
 *    parent and marks it as preregistered.
 *
 * Arguments:
 *
 *    parent_class		pointer to the parent oid
 *    child_class		pointer to the child oid
 *
 * Return value:
 *
 *    MAN_C_SUCCESS
 *    MAN_C_FAILURE
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status status ;
    object_t *object ;
    int i ;

#ifdef DEBUG
    fprintf( stderr, "Parent oid :   " ) ;
    for ( i = 0 ; i < parent_class->count ; i++ )
    {
	fprintf( stderr, "%u", parent_class->value[i] ) ;
	if ( ( i + 1 ) < parent_class->count )
	    fprintf( stderr, " . " ) ;
	else
	    fprintf( stderr, "\n" ) ;
    }

    fprintf( stderr, "Child oid :   " ) ;
    for ( i = 0 ; i < child_class->count ; i++ )
    {
	fprintf( stderr, "%u", child_class->value[i] ) ;
	if ( ( i + 1 ) < child_class->count )
	    fprintf( stderr, " . " ) ;
	else
	    fprintf( stderr, "\n\n" ) ;
    }
#endif

    /*
     * Register child under parent and give no management handle.
     */

    status = register_containment( NULL, parent_class, child_class,
				   &object ) ;

    /*
     * Mark it as preregistered.
     */

    if ( status == MAN_C_SUCCESS )
	object->state_flag = MAN_C_PREREGISTERED ;

    /*
     * It is ok if it is already registered.
     */

    if ( status == MAN_C_OBJECT_ALREADY_EXISTS )
	status = MAN_C_SUCCESS ;

    return( status ) ;

}  /* end of mold_preregister_object() */



man_status
get_class_mandle(
                 oid,
                 return_mandle
                )

object_id *oid ;
mandle **return_mandle ;

/*
 *
 * Function description:
 *
 *
 * Arguments:
 *
 *    oid		pointer of oid to get the mandle
 *    return_mandle	address of pointer to a mandle
 *
 * Return value:
 *
 *    MAN_C_SUCCESS
 *    MAN_C_FAILURE
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    mir_status m_status ;
    mandle_class *local_class_mandle = NULL ;

    m_status = mir_oid_to_mandle( GET_EXACT,
				  oid,
				  return_mandle,
				  &local_class_mandle,
				  NULL ) ;

    if ( m_status != MS0_FIND_EXACT )
	return( MAN_C_FAILURE ) ;

    return( MAN_C_SUCCESS ) ;

}  /* end of get_class_mandle() */


#ifndef NOIPC


man_status
mold_dump_oid(
              dmp_file,
              oid
             )

FILE *dmp_file ;
object_id *oid ;

/*
 *
 * Function description:
 *
 *    This routine dump an oid to the dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to the FILE pointer
 *    oid		pointer of oid
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    int i ;

    if ( oid->count == 0 )
    {
	fprintf( dmp_file, " ( ROOT )\n" ) ;

	return( MAN_C_SUCCESS ) ;
    }

    for ( i = 0 ; i < oid->count ; i++ )
    {
	fprintf( dmp_file, "%u", oid->value[i] ) ;

	if ( ( i + 1 ) < oid->count )
	    fprintf( dmp_file, " . " ) ;
	else
	    fprintf( dmp_file, "\n" ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_oid() */



man_status
mold_dump_object(
                 dmp_file,
                 object
                )

FILE *dmp_file ;
object_t *object ;

/*
 *
 * Function description:
 *
 *    This routine dumps the oids of the object, its parent, its siblings
 *    and its children, the registration state and supported interface to
 *    a dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to FILE pointer
 *    object		pointer to object
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status m_status ;
    family_relation_t *sibling ;
    family_relation_t *child ;
    object_t *local_object ;
    int count ;

    /*
     * Dump the object oid.
     */

    fprintf( dmp_file, "Object oid   : " ) ;

    m_status = mold_dump_oid( dmp_file, &(object->oid) ) ;

    if ( m_status != MAN_C_SUCCESS )
	return( m_status ) ;

    fprintf( dmp_file, "\n" ) ;

    /*
     * Dump the parent oid.
     */

    fprintf( dmp_file, "Parent oid   : " ) ;

    m_status = mold_dump_oid( dmp_file, &(object->parent_oid) ) ;

    if ( m_status != MAN_C_SUCCESS )
	return( m_status ) ;

    fprintf( dmp_file, "\n" ) ;

    sibling = object->relation.fsibling ;

    fprintf( dmp_file, "Sibling oid  : " ) ;

    /*
     * Dump the oids of the siblings.
     */

    if ( sibling == &(object->relation) )
    {
	fprintf( dmp_file, "  ( No siblings )\n\n" ) ;
    }
    else
    {
	count = 0 ;

	do
	{
	    if ( count > 0 )
	    {
		fprintf( dmp_file, "               " ) ;
	    }

	    local_object = CONTAINING_RECORD_BY_NAME( sibling,
						      object_t, relation ) ;

	    m_status = mold_dump_oid( dmp_file, &(local_object->oid) ) ;

	    sibling = sibling->fsibling ;
	    count++ ;

	} while ( sibling != &(object->relation) ) ;

	fprintf( dmp_file, "\n" ) ;
    }

    /*
     * Dump the oids of the children.
     */

    child = object->relation.children ;

    fprintf( dmp_file, "Child oid    : " ) ;

    if ( child == NULL )
    {
	fprintf( dmp_file, "  ( No children )\n\n" ) ;
    }
    else
    {
	count = 0 ;

	do
	{
	    if ( count > 0 )
	    {
		fprintf( dmp_file, "               " ) ;
	    }

	    local_object = CONTAINING_RECORD_BY_NAME( child,
						      object_t, relation ) ;

	    m_status = mold_dump_oid( dmp_file, &(local_object->oid) ) ;

	    child = child->fsibling ;
	    count++ ;

	} while ( child != object->relation.children ) ;

	fprintf( dmp_file, "\n" ) ;
    }

    /*
     * Dump the registration state.
     */

    fprintf( dmp_file, "Registration   \n" ) ;
    fprintf( dmp_file, "State        :   " ) ;

    switch ( object->state_flag )
    {
	case MAN_C_PREREGISTERED :

		fprintf( dmp_file, "Pre-registered\n\n" ) ;
		break ;

	case MAN_C_DEREGISTERED :

		fprintf( dmp_file, "Deregistered\n\n" ) ;
		break ;

	case MAN_C_REGISTERED :

		fprintf( dmp_file, "Registered\n\n" ) ;
		break ;

	default :

		fprintf( dmp_file, "( Invalid State %d )\n\n",
			 object->state_flag ) ;
		break ;
    }

    /*
     * Dump the supported interface.
     */

    fprintf( dmp_file, "Supported      \n" ) ;
    fprintf( dmp_file, "Interface    :   " ) ;

    switch ( object->supported_interface )
    {
	case MAN_M_MSI :

		fprintf( dmp_file, "MAN_M_MSI\n\n" ) ;
		break ;

	case MAN_M_SNMP :

		fprintf( dmp_file, "MAN_M_SNMP\n\n" ) ;
		break ;

	case ( MAN_M_MSI | MAN_M_SNMP ) :

		fprintf( dmp_file, "MAN_M_MSI\n" ) ;
		fprintf( dmp_file, "                 " ) ;
		fprintf( dmp_file, "MAN_M_SNMP\n\n" ) ;
		break ;

	default :

		fprintf( dmp_file, "( Invalid Interface %u )\n\n",
			 object->supported_interface ) ;
		break ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_object() */



man_status
mold_dump_hash(
               dmp_file
              )

FILE *dmp_file ;

/*
 *
 * Function description:
 *
 *    This routine dumps the whole hash table to a dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to FILE pointer
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status m_status ;
    int i ;
    int count = 0 ;
    int entry ;
    object_t *object ;

    fprintf( dmp_file, "***************************\n" ) ;
    fprintf( dmp_file, " Dump of Hash Table\n" ) ;
    fprintf( dmp_file, "***************************\n\n" ) ;

    fprintf( dmp_file, "Hash Table Size = %d\n\n", MAN_C_HASH_VALUE ) ;

    fprintf( dmp_file, "Hash Table Hashing Prime = %d\n\n",
	     MAN_C_HASH_PRIME ) ;

    for ( i = 0 ; i < MAN_C_HASH_VALUE ; i++ )
    {
	if ( hash_table[i] != NULL )
	{
	    fprintf( dmp_file, "***************************\n" ) ;
	    fprintf( dmp_file, " Start of Hash Bucket %d\n", i ) ;
	    fprintf( dmp_file, "***************************\n\n" ) ;

	    object = hash_table[i] ;
	    entry = 1 ;

	    while ( object != NULL )
	    {
		fprintf( dmp_file, "***************************\n" ) ;
		fprintf( dmp_file, " Hash Bucket %d Entry %d\n",
			 i, entry ) ;
		fprintf( dmp_file, "***************************\n\n" ) ;

		m_status = mold_dump_object( dmp_file, object ) ;

		if ( m_status != MAN_C_SUCCESS )
		    return( m_status ) ;

		object = object->next_hash ;
		entry++ ;
	    }

	    fprintf( dmp_file, "***************************\n" ) ;
	    fprintf( dmp_file, " End of Hash Bucket %d\n", i ) ;
	    fprintf( dmp_file, "***************************\n\n" ) ;
	    
	    count++ ;
	}
    }

    if ( count == 0 )
    {
	fprintf( dmp_file, "***************************\n" ) ;
	fprintf( dmp_file, " Hash Table is Empty\n" ) ;
	fprintf( dmp_file, "***************************\n\n" ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_hash() */



man_status
mold_dump_lexi(
               dmp_file
              )

FILE *dmp_file ;

/*
 *
 * Function description:
 *
 *    This routine dumps the lexigraphic queue to a dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to FILE pointer
 *
 * Return value:
 *
 *    MAN_C_SUCCESS		Successful
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status m_status ;
    queue_t *queue ;
    object_t *object ;
    int count ;

    fprintf( dmp_file, "***************************\n" ) ;
    fprintf( dmp_file, " Dump of Lexigraphic Queue\n" ) ;
    fprintf( dmp_file, "***************************\n\n" ) ;

    if ( lexi_head.flink == &lexi_head )
    {
	fprintf( dmp_file, "***************************\n" ) ;
	fprintf( dmp_file, " Lexigraphic Queue is Empty\n" ) ;
	fprintf( dmp_file, "***************************\n\n" ) ;
    }
    else
    {
	queue = lexi_head.flink ;
	count = 1 ;

	fprintf( dmp_file, "***************************\n" ) ;
	fprintf( dmp_file, " Start of Lexigraphic Queue\n" ) ;
	fprintf( dmp_file, "***************************\n\n" ) ;

	while ( queue != &lexi_head )
	{
	    fprintf( dmp_file, "***************************\n" ) ;
	    fprintf( dmp_file, " Lexigraphic Entry %d\n", count ) ;
	    fprintf( dmp_file, "***************************\n\n" ) ;

	    object = CONTAINING_RECORD_BY_NAME( queue, object_t, lexi_q ) ;

	    m_status = mold_dump_object( dmp_file, object ) ;

	    if ( m_status != MAN_C_SUCCESS )
		return( m_status ) ;

	    queue = queue->flink ;
	    count++ ;
	}

	fprintf( dmp_file, "***************************\n" ) ;
	fprintf( dmp_file, " End of Lexigraphic Queue\n" ) ;
	fprintf( dmp_file, "***************************\n\n" ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_lexi() */



man_status
mold_dump_generation(
                     dmp_file,
                     first_child
                    )

FILE *dmp_file ;
family_relation_t *first_child ;

/*
 *
 * Function description:
 *
 *    This routine dumps the containment tree starting from the current
 *    generation by recursively calling itself, given the first child
 *    of the generation to start, to a dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to FILE pointer
 *    first_child	pointer to the family relation structure of the
 *			first child in the current generation
 *
 * Return value:
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status m_status ;
    family_relation_t *curr ;
    object_t *object ;

    curr = first_child ;

    do
    {
	object = CONTAINING_RECORD_BY_NAME( curr, object_t, relation ) ;

	fprintf( dmp_file, "***************************\n\n" ) ;

	m_status = mold_dump_object( dmp_file, object ) ;

	if ( m_status != MAN_C_SUCCESS )
	    return( m_status ) ;

	if ( curr->children != NULL )
	{
	    m_status = mold_dump_generation( dmp_file, curr->children ) ;

	    if ( m_status != MAN_C_SUCCESS )
		return( m_status ) ;
	}

	curr = curr->fsibling ;

    } while ( curr != first_child ) ;

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_generation() */



man_status
mold_dump_containment(
                      dmp_file
                     )

FILE *dmp_file ;

/*
 *
 * Function description:
 *
 *    This routine dumps the whole containment tree starting from the
 *    ROOT to a dump file.
 *
 * Arguments:
 *
 *    dmp_file		pointer to FILE pointer
 *
 * Return value:
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status m_status ;

    fprintf( dmp_file, "***************************\n" ) ;
    fprintf( dmp_file, " Dump of Containment Tree\n" ) ;
    fprintf( dmp_file, "***************************\n\n" ) ;

    if ( containment_head == NULL )
    {
	fprintf( dmp_file, "***************************\n" ) ;
	fprintf( dmp_file, " Containment Tree is Empty\n" ) ;
	fprintf( dmp_file, "***************************\n\n" ) ;
    }
    else
    {
	m_status = mold_dump_generation( dmp_file, containment_head ) ;

	if ( m_status != MAN_C_SUCCESS )
	    return( m_status ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_dump_containment() */



man_status
mold_concat_oid(
                buff,
                oid
               )

char *buff ;
object_id *oid ;

/*
 * Function description:
 *
 *    This routine takes an object id and convert it into an oid in the string
 *    form.
 *
 * Arguments:
 *
 *    buff		pointer to buffer to put text string
 *    oid		Address of object id
 *
 */

{
    int i ;
    char num[16] ;

    if ( oid->count == 0 )
    {
	strcat( buff, " ( ROOT )\n" ) ;

	return( MAN_C_SUCCESS ) ;
    }

    for ( i = 0 ; i < oid->count ; i++ )
    {
	sprintf( num, "%u", oid->value[i] ) ;
	strcat( buff, num ) ;

	if ( ( i + 1 ) < oid->count )
	    strcat( buff, " . " ) ;
	else
	    strcat( buff, "\n" ) ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* end of mold_concat_oid() */



man_status
mold_dump_single_object(
                        oid,
                        return_text
                       )

object_id *oid ;
char **return_text ;

/*
 * Function description:
 *
 *    This routine returns information of an oid in the MOLD.
 *
 * Arguments:
 *
 *    oid		Address of object id
 *    return_text	Address of a pointer to a char
 *
 * Return value:
 *
 *    MAN_C_SUCCESS			Successful
 *    MAN_C_PROCESSING_FAILURE		Memory allocation failure
 */

{
    int count ;
    time_t curr_time ;
    char *time_text ;
    object_id *parent_oid ;
    object_t *current ;
    family_relation_t *sibling ;
    object_t *local_object ;
    family_relation_t *child ;
    unsigned int hash_value ;
    int i ;
    int j ;
    char num[16] ;

    /*
     * Get the time in text form.
     */

    time( &curr_time ) ;
    time_text = ctime( &curr_time ) ;

    /*
     * Get the current object and its parent oid.
     */

    current = find_object( oid ) ;
    parent_oid = &(current->parent_oid) ;

    /*
     * Count the total bytes we need.
     */

    count = 1 ;
    count = count + strlen( time_text ) + 1 ;
    count = count + 1 ;
    count = count + strlen( "MIR file :  " ) + strlen( init_mir_filename ) + 1 ;
    count = count + 1 ;
    count = count + 28 ;
    count = count + 41 ;
    count = count + 28 ;
    count = count + 1 ;
    count = count + 16 + ( ( oid->count ) * 13 ) ;
    count = count + 1 ;
    count = count + 16 ;

    if ( parent_oid->count == 0 )
    {
	count = count + 9 ;
    }
    else
    {
	count = count + ( ( parent_oid->count ) * 13 ) ;
    }

    count = count + 1 ;

    sibling = current->relation.fsibling ;

    if ( sibling == &(current->relation) )
    {
	count = count + 33 ;
	count = count + 1 ;
    }
    else
    {
	do
	{
	    local_object = CONTAINING_RECORD_BY_NAME( sibling,
						      object_t, relation ) ;

	    count = count + 16 + ( ( local_object->oid.count ) * 13 ) ;

	    sibling = sibling->fsibling ;

	} while ( sibling != &(current->relation) ) ;

	count = count + 1 ;
    }

    child = current->relation.children ;

    if ( child == NULL )
    {
	count = count + 33 ;
	count = count + 1 ;
    }
    else
    {
	do
	{
	    local_object = CONTAINING_RECORD_BY_NAME( child,
						      object_t, relation ) ;

	    count = count + 16 + ( ( local_object->oid.count ) * 13 ) ;

	    child = child->fsibling ;

	} while ( child != current->relation.children ) ;

	count = count + 1 ;
    }

    count = count + 13 ;
    count = count + 47 ;
    count = count + 10 ;
    count = count + 51 ;
    count = count + 1 ;

    MALLOC( *return_text, char *, count ) ;

    if ( *return_text == NULL )
	return( MAN_C_PROCESSING_FAILURE ) ;

    hash_value = hash( oid ) ;
    i = 1 ;
    local_object = hash_table[hash_value] ;

    while ( local_object != NULL )
    {
	if ( compare_oid( oid, &(local_object->oid) ) == MAN_C_EQUAL_TO )
	    break ;
	local_object = local_object->next_hash ;
	i++ ;
    }

    strcpy( *return_text, "\n" ) ;
    strcat( *return_text, time_text ) ;
    strcat( *return_text, "\n\n" ) ;
    strcat( *return_text, "MIR file :  " ) ;
    strcat( *return_text, init_mir_filename ) ;
    strcat( *return_text, "\n\n" ) ;
    strcat( *return_text, "***************************\n" ) ;
    sprintf( num, "%u", hash_value ) ;
    strcat( *return_text, " Hash Bucket " ) ;
    strcat( *return_text, num ) ;
    strcat( *return_text, " Entry " ) ;
    sprintf( num, "%d", i ) ;
    strcat( *return_text, num ) ;
    strcat( *return_text, "\n" ) ;
    strcat( *return_text, "***************************\n\n" ) ;
    strcat( *return_text, "Object oid   : " ) ;
    mold_concat_oid( *return_text, oid ) ;
    strcat( *return_text, "\n" ) ;
    strcat( *return_text, "Parent oid   : " ) ;
    mold_concat_oid( *return_text, parent_oid ) ;
    strcat( *return_text, "\n" ) ;

    sibling = current->relation.fsibling ;

    if ( sibling == &(current->relation) )
    {
	strcat( *return_text, "Sibling oid  :   ( No siblings )\n\n" ) ;
    }
    else
    {
	j = 1 ;

	do
	{
	    if ( j == 1 )
	    {
		strcat( *return_text, "Sibling oid  : " ) ;
	    }
	    else
	    {
		strcat( *return_text, "               " ) ;
	    }

	    local_object = CONTAINING_RECORD_BY_NAME( sibling,
						      object_t, relation ) ;

	    mold_concat_oid( *return_text, &(local_object->oid) ) ;
	    sibling = sibling->fsibling ;
	    j++ ;

	} while ( sibling != &(current->relation) ) ;

	strcat( *return_text, "\n" ) ;
    }

    child = current->relation.children ;

    if ( child == NULL )
    {
	strcat( *return_text, "Child oid    :   ( No children )\n\n" ) ;
    }
    else
    {
	j = 1 ;

	do
	{
	    if ( j == 1 )
	    {
		strcat( *return_text, "Child oid    : " ) ;
	    }
	    else
	    {
		strcat( *return_text, "               " ) ;
	    }

	    local_object = CONTAINING_RECORD_BY_NAME( child,
						      object_t, relation ) ;


	    mold_concat_oid( *return_text, &(local_object->oid) ) ;
	    child = child->fsibling ;
	    j++ ;

	} while ( child != current->relation.children ) ;

	strcat( *return_text, "\n" ) ;
    }

    strcat( *return_text, "Registration\n" ) ;
    strcat( *return_text, "State        :   " ) ;

    switch ( current->state_flag )
    {
	case MAN_C_PREREGISTERED :

		strcat( *return_text, "Pre-registered\n\n" ) ;
		break ;

	case MAN_C_DEREGISTERED :

		strcat( *return_text, "Deregistered\n\n" ) ;
		break ;

	case MAN_C_REGISTERED :

		strcat( *return_text, "Registered\n\n" ) ;
		break ;

	default :

		strcat( *return_text, "( Invalid State " ) ;
		sprintf( num, "%u", current->state_flag ) ;
		strcat( *return_text, num ) ;
		strcat( *return_text, " )\n\n" ) ;
		break ;
    }

    strcat( *return_text, "Supported\n" ) ;
    strcat( *return_text, "Interface    :   " ) ;

    switch ( current->supported_interface )
    {
	case MAN_M_MSI :

		strcat( *return_text, "MAN_M_MSI\n\n" ) ;
		break ;

	case MAN_M_SNMP :

		strcat( *return_text, "MAN_M_SNMP\n\n" ) ;
		break ;

	case ( MAN_M_MSI | MAN_M_SNMP ) :

		strcat( *return_text, "MAN_M_MSI\n" ) ;
		strcat( *return_text, "                 " ) ;
		strcat( *return_text, "MAN_M_SNMP\n\n" ) ;
		break ;

	default :

		strcat( *return_text, "( Invalid Interface " ) ;
		sprintf( num, "%u", current->supported_interface ) ;
		strcat( *return_text, num ) ;
		strcat( *return_text, " )\n\n" ) ;
		break ;
    }

    return( MAN_C_SUCCESS ) ;

}  /* mold_dump_single_object() */



man_status
mold_dump_init_mold(
                    mold_dump_file
                   )

char *mold_dump_file ;

/*
 *
 * Function description:
 *
 *    This routine dumps the hash table, lexigraphic queue and
 *    containment tree of the MOLD to a dump file.
 *
 * Arguments:
 *
 *    mold_dump_file		character string of a dump file name
 *
 * Return value:
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 */

{
    man_status status ;
    FILE *dmp_file ;
    time_t curr_time ;
    char *time_text ;
    char msg[LINEBUFSIZE];

#ifdef DEBUG
    fprintf( stderr, "MOLD: Dumping mold to %s\n\n", mold_dump_file ) ;
#endif

    dmp_file = fopen( mold_dump_file, "w" ) ;

    if ( dmp_file == NULL )
    {
	sprintf( msg, MSG(mold_msg014, "MOLD: mold_dump_init_mold: error opening file %s\n\n"),
		mold_dump_file ) ;
	mold_log(msg, LOG_ERR);
	return( MAN_C_PROCESSING_FAILURE ) ;
    }

    fprintf( dmp_file, "***********************************************\n" ) ;
    fprintf( dmp_file, "*                                             *\n" ) ;
    fprintf( dmp_file, "*          Dump of Common Agent MOLD          *\n" ) ;
    fprintf( dmp_file, "*             built from MIR file             *\n" ) ;
    fprintf( dmp_file, "*                                             *\n" ) ;
    fprintf( dmp_file, "***********************************************\n" ) ;

    time( &curr_time ) ;
    time_text = ctime( &curr_time ) ;

    fprintf( dmp_file, "\n\n\nFile dumped on %s", time_text ) ;

    fprintf( dmp_file, "\n\n\nMIR file :  %s\n\n\n", init_mir_filename ) ;

    /*
     * Dump the hash table.
     */

    status = mold_dump_hash( dmp_file ) ;

    if ( status != MAN_C_SUCCESS )
    {
	sprintf( msg, "MOLD: mold_dump_hash: " ) ;
	mold_log(msg, LOG_ERR);
	sprintf( msg, MSG(mold_msg015, "error dumping hash table\n\n") ) ;
	mold_log(msg, LOG_ERR);
	fclose( dmp_file ) ;
	return( status ) ;
    }

    /*
     * Dump the lexigraphic queue.
     */

    status = mold_dump_lexi( dmp_file ) ;

    if ( status != MAN_C_SUCCESS )
    {
	sprintf( msg, "MOLD: mold_dump_lexi: " ) ;
	mold_log(msg, LOG_ERR);
	sprintf( msg, MSG(mold_msg016, "error dumping lexigraphic queue\n\n") ) ;
	mold_log(msg, LOG_ERR);
	fclose( dmp_file ) ;
	return( status ) ;
    }

    /*
     * Dump the containment tree.
     */

    status = mold_dump_containment( dmp_file ) ;

    if ( status != MAN_C_SUCCESS )
    {
	sprintf( msg, "MOLD: mold_dump_containment: " ) ;
	mold_log(msg, LOG_ERR);
	sprintf( msg, MSG(mold_msg017, "error dumping containment tree\n\n") ) ;
	mold_log(msg, LOG_ERR);
	fclose( dmp_file ) ;
	return( status ) ;
    }

    fclose( dmp_file ) ;

    return( status ) ;

}  /* end of mold_dump_init_mold() */



static
void
initialize_mold()

/*
 *
 * Function description:
 *
 *    Initialize ourselves as an RPC server.
 *
 * Arguments:
 *
 *    None
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    We never voluntarily leave this routine (We block
 *    in rpc listen mode).
 *
 */

{
    error_status_t st = error_status_ok; 
    char *prefix = "MOLD: initialize_mold: " ;
    char *prefix2 = "MOLD: mold_signal_handler: ";
    char *string_bind;
    rpc_binding_vector_t *binding_vec;
    uuid_vector_t obj_uuid_vec;
    int exception = FALSE;
    char msg[LINEBUFSIZE];


    /*
     *  Create a socket for RPC use.
     */

    TRY

    rpc_server_register_if ( mold_v1_0_s_ifspec,
			     NULL,
			     (rpc_mgr_epv_t) &mold_v1_0_m_epv,
			     &st );

    CATCH_ALL

#ifdef DEBUG
	sprintf(msg, "Exception from rpc_server_register_if...\n");
	mold_log(msg, LOG_ERR);
	print_exception(THIS_CATCH);
#endif
	st = MAN_C_FAILURE;
	exception = TRUE;

    ENDTRY

    if ( st != error_status_ok )
        {
#ifdef DEBUG
	if (!exception)
	    {
            sprintf( msg, "%s Error with rpc_server_register_if call... exiting.\n", 
		prefix ) ;
	    mold_log(msg, LOG_ERR);
            sprintf( msg, "%s Status = %s\n", prefix, error_text(st) ) ;
	    mold_log(msg, LOG_ERR);
	    }
#endif /* DEBUG */
	return ;
	}


    /*
     *  Register the interface with the runtime
     */

    TRY

        rpc_server_use_protseq ( (unsigned char *) "ncadg_ip_udp",
			         MAN_C_MOLD_MAX_CALLS,
			         &st);
    CATCH_ALL

#ifdef DEBUG
	sprintf(msg, "%s EXCEPTION calling use_protseq...\n",
		prefix);
	mold_log(msg, LOG_ERR);
	print_exception(THIS_CATCH);
#endif /* DEBUG */
	st = MAN_C_FAILURE;
        exception = TRUE;

    ENDTRY

    if ( st != error_status_ok )
        {
#ifdef DEBUG
	if (!exception)
	    {
            sprintf( msg, "%s Error calling use_protseq...exiting.\n", prefix ) ;
	    mold_log(msg, LOG_ERR);
            sprintf( msg, "%s Status = %s\n", prefix, error_text(st) ) ;
	    mold_log(msg, LOG_ERR);
            }
#endif /* DEBUG */
	return ;
	}


    /*
     * Get a list of this server's bindings...
     */

    TRY

        rpc_server_inq_bindings ( &binding_vec,
			          &st );
    CATCH_ALL

#ifdef DEBUG
	sprintf(msg, "%s EXCEPTION inquiring about server bindings...\n",prefix);
	mold_log(msg, LOG_ERR);
	print_exception ( THIS_CATCH );
#endif
	st = MAN_C_FAILURE;
        exception = TRUE;

    ENDTRY

    if ( st != error_status_ok )
	{
#ifdef DEBUG
	if (!exception)
	    {
	    sprintf(msg, "%s Error inquiring about server bindings...exiting.\n",
		prefix);
	    mold_log(msg, LOG_ERR);
            sprintf(msg, "%s Status = %s\n",error_text(st) );
	    mold_log(msg, LOG_ERR);
	    }
#endif /* DEBUG */
	return;
	}


    obj_uuid_vec.count    = 1;
    obj_uuid_vec.uuid[0]  = (uuid_t *) &mold_obj;

    /* 
     *  Do a local registration with the endpoint mapper
     */

    TRY

        rpc_ep_register ( mold_v1_0_s_ifspec,
		      binding_vec,
		      &obj_uuid_vec,
		      (unsigned char *) "Managed Object Location Directory",
		      &st );

    CATCH_ALL

#ifdef DEBUG
	sprintf(msg, "%s EXCEPTION registering locally....\n",prefix);
	mold_log(msg, LOG_ERR);
	print_exception(THIS_CATCH);
#endif /* DEBUG */
	st = MAN_C_FAILURE;
  	exception = TRUE;

    ENDTRY

    if ( st != error_status_ok )
        {
#ifdef DEBUG
	if (!exception)
	    {
            sprintf( msg, "%s Error registering locally...exiting.\n", prefix ) ;
	    mold_log(msg, LOG_ERR);
            sprintf( msg, "%s Status = %s\n", prefix, error_text(st) ) ;
	    mold_log(msg, LOG_ERR);
	    }
#endif /* DEBUG */
	return ;
	}


    rpc_binding_vector_free ( &binding_vec, &st );

    if (st != error_status_ok)
	{
#ifdef DEBUG
	sprintf(msg, "Error freeing binding vec, %s\n",error_text(st));
	mold_log(msg, LOG_ERR);
#endif /* DEBUG */
	return;
	}

    /*
     * If a signal occurs call mold_signal_handler to deregister
     * and cleanup mold.
     */

    TRY

        rpc_server_listen( MAN_C_MOLD_MAX_CALLS, &st );

    CATCH_ALL

	sprintf (msg, MSG(mold_msg018, "%s Received a signal...\n"),prefix2);
	mold_log(msg, LOG_ERR);
	sprintf (msg, MSG(mold_msg019, "%s Unregistering locally and exiting...\n"),prefix2);
	mold_log(msg, LOG_ERR);

    ENDTRY

    mold_signal_handler();

    /*
     * To satisfy the compiler...
     */

    return;

}  /* end of initialize_mold() */

#endif /* NOIPC */


object_t *
find_object(
            oid
           )
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Find an object in our hash table. 
 *
 * Arguments:
 *
 *    None
 *
 * Return value:
 *
 *    The address of the found object.
 *    If one is not found, NULL is returned.
 *
 * Side effects:
 *
 *    None
 *
 */

{
    object_t *object ;
    unsigned int address ;

    /*
     * Hash the oid.
     */

    address = hash( oid ) ;

    /*
     * Look for it in the hash table.
     */

    object = hash_table_hit( address, oid, 0 ) ;

    return( object ) ;
    
}  /* end of find_object() */


object_t *
create_object(
              oid
             )
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Create an object in our hash table.
 *
 * Arguments:
 *
 *    None
 *
 * Return value:
 *
 *    If the object already exists, a NULL is returned as an error.  
 *    Otherwise the address of the object is returned.
 *
 * Side effects:
 *
 *    None
 */

{

    object_t *old_object ;
    object_t *object ;
    unsigned int address ;

    /*
     * See if we detect a pre-existing object. An existsing object
     * causes us noe to create a new one.
     */

    address = hash( oid ) ;
    old_object = hash_table_hit( address, oid, 0 ) ;

    if (old_object != NULL)
        return( NULL ) ;

    /*
     * Allocate space for the new one.
     */

    object = allocate_object( ) ;

    /*
     * Attach the object last found at this hash address (if there
     * isn't one, a null is still OK).
     */

    object->next_hash = hash_table[address] ;

    /*
     * Place it at the head of the hash table bucket.
     */

    hash_table[ address ] = object ;

    return( object ) ;

}  /* end of create_object() */


void
insert_containment(
                   family_head, 
                   parent, 
                   curr, 
                   oid 
                  )
family_relation_t **family_head ;
family_relation_t *parent ;
family_relation_t *curr ;
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Insert a new member into the containment hierarchy structure.
 *
 * Arguments:
 *
 *    family_head    The pointer to the beginning of the containment hierarchy
 *    parent         The family substructure of the parent containing data structure.
 *    curr           The family substructure of the element to insert containing data structure.
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    It is assumed that curr does not already exists and can be added directly.
 *    And, of course, that the parent (if non-NULL) exists.
 *
 */

{

    family_relation_t *child ;
    family_relation_t *fchild;
    family_relation_t *bchild;
    object_t *object ;
    int relationship ;
    int done = FALSE ;

    curr->parent = parent ;

    /*
     * If parent is NULL, then we are adding a top-level object (i.e. it is 
     * not embedded down deep within the containment hierarchy, so the 'parent'
     * pointer is really the global root of the containment tree).
     */

    if ( parent == NULL ) 
    {
        child = *family_head ;

        /*
         * If there are no children of the root data structure,
         * set the root to point to the curr, set curr to point to 
         * itself (as far as siblings go), and note that we are done.
         */

        if ( child == NULL )
        {
            *family_head = curr ;
            curr->fsibling = curr ;
            curr->bsibling = curr ;
            done = TRUE ;
        }
    }
    else
    {

        /*
         * If there are no children of the parent,
         * set curr to be that child, and point to 
         * itself (as far as siblings go), and note that we are done.
         */

        child = parent->children ;
        if ( child == NULL )
        {
            parent->children = curr ;
            curr->fsibling = curr ;
            curr->bsibling = curr ;
            done = TRUE ;
        }
    }

    /*
     * Place the new object in the correct collating sequence of sibling objects.
     */

    fchild = child ;
    while( done == FALSE )
    {
        object = CONTAINING_RECORD_BY_NAME( fchild, object_t, relation ) ;

        /*
         * Compare an established node's oid with the one we want to insert.
         * As soon as the established node's oid is greater_than the one we
         * want to insert, then it is in the correct order.
         */

        relationship = compare_oid( &( object->oid ), oid ) ;

        /*
         * If the established one is larger than we insert it here.
         */

        if ( relationship == MAN_C_GREATER_THAN )
        {
            bchild = fchild->bsibling ;

            fchild->bsibling = curr ;
            bchild->fsibling = curr ;

            curr->bsibling = bchild ;
            curr->fsibling = fchild ;
            done = TRUE ;
        }
        else

        /*
         * If the established one is smaller, but the last one on the list,
         * (fchild->fsibling points back to the 'original' child),
         * we insert it here.
         */

        if ( fchild->fsibling == child )
        {
            bchild = fchild->fsibling ;

            fchild->fsibling = curr ;
            bchild->bsibling = curr ;

            curr->bsibling = fchild ;
            curr->fsibling = bchild ;
            done = TRUE ;
        }

        /*
         * If it has been inserted in the right place, 
         * make sure to set the parent pointer to the right
         * 'least most' child
         */

        if ( done == TRUE )
        {
            if ( relationship == MAN_C_GREATER_THAN )
            {
                if ( curr->fsibling == child )
                {
                    if ( parent == NULL ) 
                        *family_head = curr ;
                    else
                        parent->children = curr ;
                } 
                /* else means the parent already points to the smallest one */
            }
            /* else means the parent already points to the smallest one */
        }
        else
            fchild = fchild->fsibling ;
    }

}  /* end of insert_containment() */


man_status 
remove_object(
              curr
             )
object_t *curr ;

/*
 *
 * Function description:
 *
 *    Remove the object from the lexical and hierarchical tree.
 *
 * Arguments:
 *
 *    curr            the object to remove
 *
 * Return value:
 *
 *    MAN_C_HAS_ACTIVE_CHILDREN   Cannot remove because of existing children
 *    MAN_C_SUCCESS               Object successfully removed
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    The memory is NOT freed. This must be done by the caller.
 */

{

    man_status stat ; 
    object_t *object ;
    object_t *prev_object ;
    unsigned int address ;
    char msg[LINEBUFSIZE];

    /*
     * Remove curr from the tree.
     */

    stat = remove_containment( &containment_head, curr ) ;

    if ( stat == MAN_C_SUCCESS )
    {

        /*
         * remove the object from our lexigraphic order too. Note that
         * the only bad status generated (MAN_C_HAS_ACTIVE_CHILDREN) would
         * already have been caught by the remove_containment call.
         */

        if ( curr->lexi_q.flink != NULL )
            stat = remove_lexi( &lexi_head, curr ) ;

        /*
         * Make sure we also can find it in our hash table.
         */

        address = hash( &( curr->oid ) ) ;
        object = hash_table_hit( address, &( curr->oid ), &prev_object ) ;
        
        if ( object == NULL )
        {
            sprintf( msg, MSG(mold_msg020, "MOLD: remove_object: Fatal error. Object not found in hash table. Exiting.\n") ) ;
	    mold_log(msg, LOG_ERR);
            exit( 1 ) ;
        }

        /*
         * Pick the object out of the hash table, making sure to keep it the bucket
         * splits consistent.
         */

        else
        {
            if ( prev_object == NULL )
                hash_table[ address ] = object->next_hash ;
            else
                prev_object->next_hash = object->next_hash ; 
            object->next_hash = NULL ;
        }
        return( MAN_C_SUCCESS ) ;
    }

    return( stat ) ;

}  /* end of remove_object() */


static
man_status 
remove_containment(
                   family_head , 
                   object
                  ) 
family_relation_t **family_head ;
object_t *object;

/*
 *
 * Function description:
 *
 *    Remove a member from the family structure.
 *
 * Arguments:
 *
 *    family_head    The pointer to the beginning of the containment hierarchy
 *    parent         The family substructure of the parent containing data structure.
 *    curr           The object to remove.
 *
 * Return value:
 *
 *    MAN_C_SUCCESS              if we can remove the object
 *    MAN_HAS_ACTIVE_CHILDREN    children exist
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    It is assumed that curr already exists and can be removed directly.
 *    It also assumes, of course, that the parent exists.
 *    The memory allocated is NOT freed. The caller must perform that operation.
 *
 *      ! The entry in question CANNOT have active children (children pointer must == 0).
 */

{

    family_relation_t *fsibling ;
    family_relation_t *bsibling ;
    family_relation_t *curr ;

    curr = &( object->relation ) ;

    /*
     * If there are no siblings, parent or children, then this object was part of
     * the lexigraphic ordering, and not part of the containment hierarchy.
     */

    if ( curr->fsibling == NULL &&
         curr->bsibling == NULL &&
         curr->parent == NULL &&
         curr->children == NULL )
        return( MAN_C_SUCCESS ) ;

    /*
     * If there are children, disallow removal!
     */

    if ( curr->children != NULL )
        return( MAN_C_HAS_ACTIVE_CHILDREN ) ; 

    /*
     * Adjust all the pointers so that the removal is clean.
     */

    if ( curr->parent != NULL )
    {

        /*
         * This particular object is the 'least most' child of all its parents'
         * children, so make the 'next' sibling the least most. Unless, of course,
         * it is the only child...
         */

        if ( curr->parent->children == curr )
        {
            if ( curr->fsibling == curr )
                curr->parent->children = NULL ;
            else
                curr->parent->children = curr->fsibling ;
        }
    }

    /*
     * This is a child of the root....
     */

    else
    {

        /*
         * If it is the only child, mark it has NULL, otherwise,
         * let the 'next' child be least most.
         */

        if ( curr->fsibling == curr )
            *family_head = NULL ;
        else
            *family_head = curr->fsibling ;
    }

    /*
     * Now simply remove it from the doubly linked list, and
     * zero out the pointer so we don't use any references by accident.
     */

    fsibling = curr->fsibling ;
    bsibling = curr->bsibling ;

    bsibling->fsibling = fsibling ;
    fsibling->bsibling = bsibling ;

    curr->parent = NULL ;
    curr->bsibling = NULL ;
    curr->fsibling = NULL ;

    return( MAN_C_SUCCESS ) ;

}  /* end of remove_containment() */


man_status 
find_scoped_objects(
                    base_object , 
                    containment_level , 
                    return_info
                   ) 
object_t *base_object ;
int containment_level ;
avl *return_info ;

/*
 *
 * Function description:
 *
 *    Find all the obejcts in the scope specified from the object specified. 
 *
 * Arguments:
 *
 *    base_object        A pointer to the base object 
 *    containment_level  The level specified for scoping
 *    return_info        A pointer to the AVL handle we will construct with the object id's.
 *
 *
 * Return value:
 *
 *    Currently always MAN_C_SUCCESS
 *
 * Side effects:
 *
 *    None
 *
 * NOTE:
 *
 *    The scope (containment_level) is passed in the following manner:
 *
 *                                              4 byte representation:
 *                                                         3 2 1 0
 *
 *        base object alone:                               0 0 0 0
 *        first level subordinates:                        0 0 0 1
 *        base object and all subordinates:                0 0 0 2
 *        the nth level below the base object:             0 0 1 n
 *        base object and all subordinates to level n:     0 0 2 n
 *
 *    This means that the object hierarchy is limited to 256 levels.
 *
 * NOTE:
 *
 *    This routine could return an empty set of managed objects.
 *
 */

{
    family_relation_t *child ;
    family_relation_t *curr_family ;
    family_relation_t *next_family ;
    object_t *object ; 
    object_t *object2 ; 
    int level ;
    int direction ; 
    int relationship ;
    int operation ;
    int length ;
    octet_string man_handle_octet ;
    unsigned char *split_containment = (unsigned char *)&containment_level ;

    /* 
     * Build the AVL, tack base_object->oid to it if the scope references the base object.
     */

    if (
       (       /* any byte 1 */               split_containment[ 0 ] == 0 )   ||
       ( ( split_containment[ 1 ] == 0 ) && ( split_containment[ 0 ] == 2 ) ) ||
       ( split_containment[ 1 ] == 2                 /* any byte 0 */       )
       )
    {
        man_handle_octet.length = base_object->man_handle.length ;
        man_handle_octet.string = ( char * )( base_object->man_handle.socket_address ) ;
        moss_avl_add( return_info , 
                      &( base_object->oid ) , 
                      0 ,
                      ASN1_C_NULL ,
                      &( man_handle_octet ) ) ;
    }

    child = base_object->relation.children ;

    /*
     * If there are no children, or we wanted ONLY the base object, (byte 0 == 0 
     * implies from the above comment that either the base object alone, or
     * the nth level (level 0) below the base object are to be gotten. And
     * since we start counting at 1...)
     */

    if ( ( child == NULL ) || ( split_containment[ 0 ] == 0 ) )
    {
        return( MAN_C_SUCCESS ) ;
    }

    /*
     * Start going down the containment hierarchy. 
     * We are going to keep track of how far away (what level) from the base object.
     * Note that we can also be smart about our traversal; we can prune off portions 
     * of the containment tree that we know we will not go down.
     */

    level = -1 ;
    direction = MAN_C_DOWN ;
    curr_family = child ;

    while ( TRUE )
    {

        /*
         * Once we come back to the base object, we are finished.
         */

        if ( level == 0 )
        {
            return( MAN_C_SUCCESS ) ;
        }

        next_family = curr_family->fsibling ;

        object = CONTAINING_RECORD_BY_NAME( curr_family, object_t, relation ) ;
        object2 = CONTAINING_RECORD_BY_NAME( next_family, object_t, relation ) ;
    
        relationship = compare_oid( &( object->oid ), &( object2->oid ) ) ;

        /*
         * We base our operation of our tree traversal from the following table:
         *
         *                 LESS_THAN          EQUAL_TO          GREATER_THAN
         *
         *     UP          SKIP               SKIP              SKIP
         *                 NEXT               UP                UP
         *
         *    DOWN         ADD                ADD               ADD
         *                 DOWN/NEXT          DOWN/UP           DOWN/UP
         *
         * To understand the table, we figure out which direction in the tree we were
         * going versus the result of the oid comparison. We will either ADD or SKIP
         * the oid for inclusion into the return info. In the case of moving DOWN the
         * tree, we may reach a leaf node; depending on the reult of the oid comparison,
         * we will either move NEXT or UP in the containment tree.
         */

        switch ( direction )
        {
            case MAN_C_UP :
                 operation = MAN_C_SKIP ;
                 if ( relationship == MAN_C_LESS_THAN )
                     direction = MAN_C_NEXT ;
                 break ;

            case MAN_C_DOWN :

		 /*
		  * If this object is not registered, skip it and don't
		  * go down to the next level.
		  */

		 if ( object->state_flag != MAN_C_REGISTERED )
		     operation = MAN_C_SKIP ;
		 else
		     operation = MAN_C_ADD ;

                 if ( ( curr_family->children == NULL ) ||
		      ( object->state_flag != MAN_C_REGISTERED ) )
                 {
                     if ( relationship == MAN_C_LESS_THAN )
                         direction = MAN_C_NEXT ;
                     else
                         direction = MAN_C_UP ;
                 }
                 break ;
        }

        /*
         * If we were told to add it, add it (object->oid).... 
         * but, hee hee, remember to check the containment scope!
         */

        if ( operation == MAN_C_ADD )
        {
            if (
               ( ( split_containment[ 1 ] == 0 ) && ( split_containment[ 0 ] == 1 ) && ( level == -1 ) ) ||
               ( ( split_containment[ 1 ] == 0 ) && ( split_containment[ 0 ] == 2 ) ) ||
               ( ( split_containment[ 1 ] == 1 ) && ( split_containment[ 0 ] == -level ) ) ||
               ( split_containment[ 1 ] == 2         /* any byte 0 */       )
 
               )
            {
                man_handle_octet.length = object->man_handle.length ;
                man_handle_octet.string = ( char * )( object->man_handle.socket_address ) ;
                moss_avl_add( return_info , 
                              &( object->oid ) , 
                              0 ,
                              ASN1_C_NULL ,
                              &( man_handle_octet ) ) ;
            }
        }
    
        /*
         * If we are looking for an exact level or down to a specific level and we
         * are at that level, there is no need to go any farther down... 
         */

        if (
           ( ( split_containment[ 1 ] == 0 ) && ( split_containment[ 0 ] == 1) && ( level == -1 ) ) ||
           ( ( split_containment[ 1 ] != 0 ) && ( split_containment[ 0 ] == -level ) )
           )
        {
           if ( direction == MAN_C_DOWN )
           {

               /*
                * Make sure not to continue to wrap around in our circular list!
                */

               if ( ( relationship == MAN_C_GREATER_THAN ) ||
                    ( relationship == MAN_C_EQUAL_TO ) )
                   direction = MAN_C_UP ;
               else
                   direction = MAN_C_NEXT ;
           }
        }
    
        /* 
         * Move along the tree in the specified order.
         */

        switch( direction )
        {
            case MAN_C_UP :
                 curr_family = curr_family->parent ;
                 level++ ;
                 break ;

            case MAN_C_NEXT : 
                 curr_family = next_family ;
                 direction = MAN_C_DOWN ;
                 break ;

            case MAN_C_DOWN : 
                 curr_family = curr_family->children ;
                 level-- ;
                 break ;
        }
    }

}  /* end of find_scoped_obects() */


static int 
compare_oid(
            first_oid ,
            last_oid
           )
object_id *first_oid ;
object_id *last_oid ;

/*
 *
 * Function description:
 *
 *    Compare the first oid to the second oid. This is kinda like the MOSS routine,
 *    But we need more accuracy than equal to or not equal to.
 *
 * Arguments:
 *
 *    first_oid       Pointer to the first oid
 *    last_oid        Pointer to the last oid
 *
 * Return value:
 *
 *    MAN_C_LESS_THAN 
 *    MAN_C_EQUAL_TO 
 *    MAN_C_GREATER_THAN
 *    NULL                  Null oid as input - no comparison done
 *
 * Side effects:
 *
 *    None
 *
 */

{

    int longer_depth ;
    int i ;
    int ret_value ;
    unsigned int *first ; 
    unsigned int *last  ;

    if ( first_oid == NULL || last_oid == NULL ||
         first_oid->value == NULL || last_oid->value == NULL )
        return( NULL ) ;

    first = first_oid->value ;
    last  = last_oid->value ;

    /*
     * Get the longer length between the two oid's.
     */

    if ( first_oid->count > last_oid->count )
        longer_depth = first_oid->count ;
    else
        longer_depth = last_oid->count ;

    /*
     * Now compare each element.
     */

    for ( i = 0 ; i < longer_depth ; i++ )
    {

        /*
         * If the two oid's are not the name length, then one will 
         * be greater or less than the other based on who is shorter
         */

        if ( i >= first_oid->count )
            return( MAN_C_LESS_THAN ) ;
        if ( i >= last_oid->count )
            return( MAN_C_GREATER_THAN ) ;

        ret_value = COMPARE( *first, *last ) ;

        first++ ; 
        last++ ;

        if ( ret_value != MAN_C_EQUAL_TO )
            return( ret_value ) ;
    }

    return( ret_value ) ;

}  /* end of compare_oid() */


static
object_t *
allocate_object( ) 

/*
 *
 * Function description:
 *
 *    Allocate an object and make sure we clear out all memory.
 *
 * Arguments:
 *
 *    None.
 *
 * Return value:
 *
 *    The address of the object.
 *
 * Side effects:
 *
 *    None
 *
 */

{
    object_t *object ;

    MALLOC( object, object_t *, sizeof( object_t ) ) ;

    memset( ( void * )object, '\0', sizeof( object_t ) ) ;

    return( object ) ;

}  /* end of allocate_object() */


void
copy_oid(
         doid ,
         soid
        ) 
object_id *doid ;
object_id *soid ;

/*
 *
 * Function description:
 *
 *    Copy an existing oid to a new one (allocate some memory for it too).
 *    This is like the MOSS routines, but we do not want to allocate the 
 *    the header portion....
 *
 * Arguments:
 *
 *    doid            Destination oid
 *    soid            Source oid
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    None
 *
 */

{

    unsigned int *sarray ;
    unsigned int *darray ;
    int i ;

    MALLOC( darray, unsigned int *, soid->count * sizeof( unsigned int ) ) ;

    doid->value = darray ;
    doid->count = soid->count ;
    sarray = soid->value ;

    for( i = 0 ; i < soid->count ; i++ ) 
        *darray++ = *sarray++ ;

}  /* end of copy_oid() */


static
unsigned int
hash(
     oid
    )
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Take an oid and hash it to an integer.
 *
 * Arguments:
 *
 *     oid            oid to hash
 *
 * Return value:
 *
 *    The resulting hash from the object id.
 *
 * Side effects:
 *
 *    None
 *
 */

{

    unsigned int address ;
    unsigned int temp ;

    /*
     * Convert oid to an integer.
     */

    temp = convert_oid( oid ) ;

    /*
     * Take the modulus of the hash prime... (knuth says it is a good one :-))
     */

    temp = temp % MAN_C_HASH_PRIME ;

    /*
     * Wrap the result around by the size of our hash table.
     */

    address = temp % MAN_C_HASH_VALUE ;

    return( address ) ;

}  /* end of hash() */


static
object_t *
hash_table_hit(
               address , 
               oid , 
               prev_object 
              ) 
int address ;
object_id *oid ;
object_t **prev_object ;

/*
 *
 * Function description:
 *
 *    Find the object in the hash table at a certain bucket.
 *
 * Arguments:
 *
 *    address            The bucket to search
 *    oid                The object to look for
 *    prev_object        The object prior to the one we find
 *                       (used because our list is singly linked ).
 *
 * Return value:
 *
 *    The address of the object that matches the specified oid at
 *    The specified hash table bucket.
 *    If none is found, a NULL is returned.
 *
 * Side effects:
 *
 *    None
 *
 */

{

    object_t *object ;
    object_t *pobj = NULL ;

    object = hash_table[address] ;
 
    /*
     * There may be more than one objct in this particular bucket.
     * scan through them to find the right one.
     */
   
    while ( object != NULL )
    {
        if ( compare_oid( oid, &( object->oid ) ) == MAN_C_EQUAL_TO )
            break ;
        pobj = object ;
        object = object->next_hash ;
    }

    /*
     * If the address for the previous object is valid, deposit the
     * address of the previous object into it (it is used for unlinking
     * from a singly-linked list
     */

    if ( prev_object != NULL )
        *prev_object = pobj ;

    return( object ) ;

}  /* end of hash_table_hit() */


static 
unsigned int
convert_oid(
            oid
           )
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Convert an oid to an integer.
 *
 * Arguments:
 *
 *    oid            the oid to convert
 *
 * Return value:
 *
 *    An integer that represents a collapsing of the object id 
 *    elements into a single long integer.
 *
 * Side effects:
 *
 *    None
 *
 */

{

    int i ;
    unsigned int *array ;
    unsigned int value = 0 ;

    array = ( unsigned int * )oid->value ;

    for (i = 0 ; i < oid->count ; i++ ) 
        value = value * 10 + *array++ ;

    return( value ) ;

}  /* end of convert_oid() */


void
insert_lexi(
             lexi_head_ptr, 
             lexi_q, 
             oid 
            )
queue_t *lexi_head_ptr ;
queue_t *lexi_q;
object_id *oid ;

/*
 *
 * Function description:
 *
 *    Insert a new member into the lexigraphic ordering structure.
 *
 * Arguments:
 *
 *    lexi_head_ptr  The pointer to the head of the lexigraphic structure (begin from here)
 *    lexi_q         The pointer to the queue structure (doubly linked) that resides inside the implied object
 *    oid            The pointer to the object id we are inserting.
 *
 * Return value:
 *
 *    None
 *
 * Side effects:
 *
 *    None
 *
 */

{
    int relationship ;
    queue_t *next_q ;
    int placed = FALSE ;
    object_t *object ;

    /*
     * if the queue is empty, then add directly here
     */

    next_q = lexi_head_ptr->flink ;

    if ( next_q == lexi_head_ptr ) 
    {
        queue_insert( lexi_head_ptr, lexi_q ) ;
        return ;
    }

    while( next_q != lexi_head_ptr ) 
    {
        object = CONTAINING_RECORD_BY_NAME( next_q, object_t, lexi_q ) ;
        relationship = compare_oid( oid, &( object->oid ) ) ;
                                   
        /*
         * If the oid passed in is less than the object we are on, then lexigraphically
         * speaking, it goes here and now.
         */

        if ( relationship == MAN_C_LESS_THAN )
        {
            queue_insert( next_q, lexi_q ) ;
            placed = TRUE ;
            break ;
        }

        /*
         * go to the next one
         */

        next_q = next_q->flink ;

    }

    /*
     * if we didn't place the object, then it must go at the end
     */

    if ( placed == FALSE )
        queue_insert( lexi_head_ptr->blink, lexi_q ) ;

}  /* end of insert_lexi() */


man_status
remove_lexi(
             lexi_head_ptr , 
             object
            )
queue_t *lexi_head_ptr ;
object_t *object ;

/*
 *
 * Function description:
 *
 *    Insert a new member into the lexigraphic ordering structure.
 *
 * Arguments:
 *
 *    lexi_head_ptr  The address of the pointer to the head of the lexigraphic structure (begin from here)
 *    object         The pointer to the object we wish to remove from the queue
 *
 * Return value:
 *
 *    MAN_C_HAS_ACTIVE_CHILDREN
 *    MAN_C_SUCCESS 
 *
 * Side effects:
 *
 *    None
 *
 */

{
    family_relation_t *curr ;

    curr = &( object->relation ) ;

    /*
     * If there are children, disallow removal!
     */

    if ( curr->children != NULL )
        return( MAN_C_HAS_ACTIVE_CHILDREN ) ; 

    /*
     * otherwise simply remove it from the queue
     */

    queue_remove( lexi_head_ptr, &( object->lexi_q ) ) ;

    return( MAN_C_SUCCESS ) ;

}  /* end of insert_lexi() */


void
queue_insert( 
#ifdef __STDC__
              queue_t *back_one ,
              queue_t *element
            )
#else
              back_one ,
              element 
            )
    queue_t *back_one ;
    queue_t *element ;
#endif

/*
 *
 * Function description:
 *
 *    This function adds the specified element to the queue. The insertion
 *    can happen anywhere in the queue. because everything is linked in, 
 *    we only need the address of the 'back_one' element in the queue.
 *
 * Arguments:
 *
 *    back_one       in        A pointer to the previous queue element in the list
 *    element        in        A pointer to the queue structure in the larger
 *                             data structure.
 *
 * Return value:
 *
 *    None.
 *
 * Side effects:
 *
 */

{
    queue_t *next_one ;

    next_one = back_one->flink ;

    back_one->flink = element ;
    element->blink = back_one ;
    element->flink = next_one ;
    next_one->blink = element ;

} /* end of queue_add */


queue_t *
queue_remove( 
#ifdef __STDC__
                  queue_t *queue ,
                  queue_t *element
                 )
#else
                  queue ,
                  element
                 )
    queue_t *queue ;
    queue_t *element ;
#endif

/*
 *
 * Function description:
 *
 *    This function removes the specified element from anywhere in the queue.
 *    Note that if we are asked to remove the head from the head (!)
 *    we will not comply, and return a NULL. 
 *
 * Arguments:
 *
 *    queue          in        A pointer to the queue head.
 *    element        in        A pointer to the queue structure in the larger
 *                             data structure.
 *
 * Return value:
 *
 *    The address of the queue element if one exists. If not, 
 *    a NULL (zero (0)) is returned. 
 *
 * Side effects:
 *
 */

{
    queue_t *next ;
    queue_t *prev ;

    /*
     * This is the case that either the queue was empty and someone 
     * tried to pop the queue, or someone was daft enough to try
     * to remove the queue head from the queue.
     */

    if ( element == queue )
        return( NULL ) ;

    /*
     * get the elements infront and behind the element
     * in question 
     */

    next = element->flink ;
    prev = element->blink ;

    /*
     * skip over the element we are removing
     */

    next->blink = prev ;
    prev->flink = next ;

    /*
     * get rid of any stale pointers
     */

    element->flink = 0 ;
    element->blink = 0 ;

    return( element ) ;

} /* end of queue_remove */

/*  end of mold.c */
