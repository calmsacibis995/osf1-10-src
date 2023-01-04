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
static char *rcsid = "@(#)$RCSfile: moss_handle.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:31 $";
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
 *    The following support MO initialization routines.  The routines are:
 *        moss_create_management_handle()
 *        moss_delete_management_handle()
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    December 5th, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, March 1st, 1990
 *
 *    Put in checks after RPC library calls.  If the call fails then
 *    it returns MAN_C_FAILURE.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, June 15th, 1990.
 *
 *    Correct parameter in bzero call.
 *
 *    Miriam Amos Nihart, July 4th, 1990.
 *
 *    Put in the #ifdef for using UNIX Domain RPC.
 *
 *    Miriam Amos Nihart, July 31st, 1990.
 *
 *    Check the status after all RPC calls and fix the pfm_$ calls.
 *
 *    Miriam Amos Nihart, Octoer 29th, 1990.
 *
 *    Change bzero and bcopy to the ansi equivalents memset and memcpy.
 *
 *    Miriam Amos Nihart, November 8th, 1990.
 *
 *    Change moss_create_management_handle to take the address of a pointer
 *    to a management_handle.
 *
 *    Jim Teague, March 4th, 1990.
 *
 *    Port to DCE RPC.
 *
 *    Kathy Faust, October 25th, 1991.
 *
 *    Modified arg to rpc_server_use_protseq -- remove MAX_CALLS.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    Modified casting from signed char to unsigned char for RPC routines.
 *
 *    Miriam Amos Nihart, December 3rd, 1991.
 *
 *    Put in macros for malloc and free.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Update for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *	Mike Densmore, 1-Apr-1992
 *
 *	Added cma.h to includes for RPC variant
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 */

#ifndef NOIPC
extern int rpc_server_use_protseq(), rpc_server_register_if(),
  rpc_server_inq_bindings(),
  rpc_binding_to_string_binding(),
  rpc_binding_vector_free(),
  rpc_string_free();
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

/*
 * Header files
 */

#include "man_data.h"
#include <string.h>
#include <stdio.h>
#ifndef NOIPC
#include <cma.h>
#include "exc_handling.h"
#endif /* NOIPC */

/*
 *  Support header files
 */  

#ifndef NOIPC
#include "mold_uuid.h"
#include "mo.h"
#endif /* NOIPC */

#include "man.h"

/*
 *  MOSS Specific header files
 */

#include "moss_private.h"
#include "extern_nil.h"

/*
 *   moi RPC routines
 */

#ifndef NOIPC
globaldef mo_v1_0_epv_t mo_v1_0_m_epv = {
    moi_get_attributes ,
    moi_set_attributes ,
    moi_create_instance ,
    moi_delete_instance ,
    moi_invoke_action 
} ;

/*
 *  External
 */

globalref rpc_binding_handle_t mold_handle ;
extern char *error_text();
extern void print_exception();

#endif /* NOIPC */



man_status
moss_create_management_handle( 
                              man_handle
                             )
management_handle **man_handle ;

/*
 *
 * Function Description:
 *
 *	Initialize a management_handle. To do this we must create an RPC uuid.
 *
 * Parameters:
 *
 *	man_handle		The address of a pointer to a management_handle.
 *
 * Return value:
 *
 *	MAN_C_SUCCESS             Management handle successfully gotten
 *      MAN_C_BAD_PARAMETER       Bad Input parameter
 *	MAN_C_FAILURE             Unable to create management handle
 *      MAN_C_PROCESSING_FAILURE  Error on memory allocation
 *
 * Side effects:
 *
 *	None
 *
 */

{

    management_handle *tmp_handle ;
#ifndef NOIPC
    error_status_t status = error_status_ok;
    rpc_binding_vector_t *bvec;
    unsigned char *string_binding;
    unsigned int string_length;
    int exception = FALSE;

#endif /* NOIPC */

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( man_handle == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    MOSS_MALLOC( tmp_handle, management_handle, sizeof( management_handle ) )
    if ( tmp_handle == NULL )
        return( MAN_C_PROCESSING_FAILURE ) ;

    memset( ( void * )tmp_handle, '\0', sizeof( management_handle ) ) ;

    *man_handle = ( management_handle * )NULL ;

#ifndef NOIPC

    /*
     *  Create a socket for the RPC use.
     */

    rpc_server_use_protseq ( (unsigned char *) "ncadg_ip_udp",
			     rpc_c_protseq_max_calls_default ,
			     &status );

    if ( status != error_status_ok )
    {
        MOSS_FREE( tmp_handle ) ;
#ifdef DEBUG
	fprintf(stderr, "MOSS: moss_handle: Error (rpc_server_use_protseq)...\n");
	fprintf(stderr, "%s\n",error_text(status));
#endif /* DEBUG */
        return( MAN_C_FAILURE ) ;
    }


    /*
     *  Register the interface.
     */

    TRY

	/*
         * Make the RPC call to register the Interface.
         */

        rpc_server_register_if( mo_v1_0_s_ifspec,
                                NULL ,
                                ( rpc_mgr_epv_t ) &mo_v1_0_m_epv ,
                                &status ) ;
    CATCH_ALL

#ifdef DEBUG
	fprintf(stderr, "MOSS: moss_handle: EXCEPTION (rpc_server_register_if)\n");
	print_exception (THIS_CATCH);
#endif /* DEBUG */
	MOSS_FREE(tmp_handle);
	exception = TRUE;

    ENDTRY

    if ( status != error_status_ok )
    {
#ifdef DEBUG
        fprintf( stderr, "MOSS: moss_handle: RPC Interface Register error.\n" ) ;
        fprintf( stderr, "status = %s\n", error_text(status) ) ;
#endif /* DEBUG */
        MOSS_FREE( tmp_handle ) ;
        return( MAN_C_FAILURE ) ;
    }
    else
	if ( exception )
	    return ( MAN_C_FAILURE );

    TRY

        rpc_server_inq_bindings ( &bvec, &status);

    CATCH_ALL

#ifdef DEBUG
	fprintf(stderr, "MOSS: moss_handle: EXCEPTION (rpc_server_inq_bindings)\n");
	print_exception(THIS_CATCH);
#endif /* DEBUG */
	exception = TRUE;

    ENDTRY

    if (status != error_status_ok)
	{
#ifdef DEBUG
	fprintf(stderr, "MOSS: moss_handle: Error (rpc_server_inq_bindings)\n");
	fprintf(stderr, "%s\n",error_text(status));
#endif /* DEBUG */
	return ( MAN_C_FAILURE );
	}
    else
	if ( exception )
	    return ( MAN_C_FAILURE );

    rpc_binding_to_string_binding ( bvec->binding_h[0],
				    &string_binding,
				    &status );
    if (status != error_status_ok)
	{
#ifdef DEBUG
	fprintf(stderr, "MOSS: moss_handle: Error (rpc_bndg_to_string_bndg)\n");
	fprintf(stderr, "%s\n",error_text(status));
#endif /* DEBUG */
	return ( MAN_C_FAILURE );
	}

    rpc_binding_vector_free ( &bvec, &status ) ;

    if (status != error_status_ok)
	{	
#ifdef DEBUG
	fprintf(stderr, "Error freeing binding vector...\n\n");
#endif /* DEBUG */
	return (MAN_C_FAILURE);
	}

    /* 
     * Place the socket and length of the socket into
     * the management handle. Tricky aren't we?
     */

    string_length = (unsigned int) strlen ( (char *)string_binding );
    tmp_handle->length = string_length ;
    memcpy( tmp_handle->socket_address, string_binding, string_length ) ;
    rpc_string_free (&string_binding, &status);

    if (status != error_status_ok)
	{
#ifdef DEBUG
	fprintf(stderr, "Error freeing string_binding...\n\n");
#endif
	return (MAN_C_FAILURE);
	}

#endif /* NOIPC */

    *man_handle = tmp_handle ;

    return( MAN_C_SUCCESS ) ;
}  /* end of moss_create_management_handle() */




man_status
moss_delete_management_handle( 
                              man_handle
                             )
management_handle *man_handle ;

/*
 *
 * Function Description:
 *
 *	This routine is used to deallocate the memory allocated for a
 *	management_handle by the moss_create_management_handle routine.
 *
 * Parameters:
 *
 *	man_handle		A pointer to a management_handle.
 *
 * Return value:
 *
 *	MAN_C_SUCCESS             Management handle successfully freed.
 *      MAN_C_BAD_PARAMETER       Bad Input parameter
 *
 * Side effects:
 *
 *	None
 *
 */

{
    if ( man_handle == NULL )
	return( MAN_C_BAD_PARAMETER ) ;

    MOSS_FREE( man_handle ) ;

    return( MAN_C_SUCCESS ) ;

}  /* end of moss_delete_management_handle() */

/* end of moss_handle.c */
