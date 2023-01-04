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
static char *rcsid = "@(#)$RCSfile: moss_mobind.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:39 $";
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
 *    Support routines for allocating the binding and unbinding 
 *    handle to/from MOLD.  These routines are:
 *        moss_alloc_mold_handle()
 *        moss_free_mold_handle()
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
 *    Put in status checks after RPC library calls.  If the call fails
 *    return MAN_C_FAILURE.
 *
 *    Miriam Amos Nihart, March 13th, 1990
 *
 *    Put in status check after RPC lb_lookup_object_local() call.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, May 20th, 1990.
 *
 *    Correct moss_unbind_from_mold() to use rpc_$free_handle rather than rpc_$free.
 *
 *    Miriam Amos Nihart, July 5th, 1990.
 *
 *    Put in the #ifdef for using UNIX Domain RPC.
 *
 *    Miriam Amos Nihart, August 1st, 1990.
 *
 *    Fix the pfm_$ calls.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change bzero to ansi equivalent memset.
 *
 *    Jim Teague, March 4th, 1991.
 *  
 *    Port to DCE RPC.
 *
 *    Kathy Faust, October 25, 1991.
 *
 *    Changed moss_bind_to_mold to moss_alloc_mold_handle; changed
 *    moss_unbind_from_mold to moss_free_mold_handle.
 *
 *    Wim Colgate, Novemeber 18th, 1991.
 *
 *    Added casting from char * to unsigned char * for RPC string 
 *    binding compose.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Update for dce-starter-kit; changed cma_exception.h to exc_handling.h
 */

#include "man_data.h"
#include <stdio.h>

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifndef DEBUG

#include <unistd.h>
#include <stdlib.h>

#endif /* DEBUG */
#endif

#include <string.h>

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
 *  External
 */

#ifndef NOIPC
extern int  rpc_string_binding_compose(),
  rpc_binding_from_string_binding(),
  rpc_string_free(),
  rpc_binding_set_object(),
  rpc_binding_free();
extern void print_exception();
extern char * error_text();
#endif /* NOIPC */


man_status
moss_alloc_mold_handle(
		       mold_handle
		       )

man_binding_handle *mold_handle ;

/*
 *
 * Function Description:
 *
 *	Create the MOLD binding using MOLD's uuid.
 *      The MOLD's uuid is static (from the include file mold_uuid.h).
 *
 * Parameters:
 *
 *      mold_handle:  Address for return of the RPC handle to MOLD
 *
 * Return value:
 *
 *	MAN_C_SUCCESS    MOLD binding handle successfully allocated
 *	MAN_C_FAILURE    Failure to allocate MOLD binding handle
 *
 * Side effects:
 *
 *	None
 *
 */

{

#ifndef NOIPC
    error_status_t rpc_status ;
    unsigned char *string_binding;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

#if defined(ultrix) || defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#ifndef DEBUG

    /*
     * Check for superuser.
     */

    if ( getuid() )
    {
	fprintf( stderr, "Not super user.\n" ) ;
	exit( 1 ) ;
    }

#endif /* DEBUG */
#endif

    /*
     *  Compose a string binding using the mold object uuid, a
     *    desired transport, the localhost address, and no endpoint.  
     *    Convert to a binding handle, and use this as the mold handle: 
     *    when used in a call, the endpoint will be filled in by the 
     *    local endpoint mapper. 
     */

    rpc_string_binding_compose ( NULL, 
				 (unsigned char *)"ncadg_ip_udp",
				 (unsigned char *)"127.0.0.1",
				 (unsigned char *)"0",
				 NULL,
				 &string_binding,
				 &rpc_status );
    
    if (rpc_status != error_status_ok )
	{
#ifdef DEBUG
	fprintf(stderr, "Error in binding compose\n\n");
	fprintf(stderr, "%s\n\n",error_text(rpc_status));
#endif /* DEBUG */
	return ( MAN_C_FAILURE );
	}
    
    /* 
     *  Now convert from the string representation to a real binding...
     */

    rpc_binding_from_string_binding ( string_binding,
				      (rpc_binding_handle_t *)mold_handle,
				      &rpc_status );

    if (rpc_status != error_status_ok )
	{
#ifdef DEBUG
	fprintf(stderr, "Error in binding from string binding\n\n");
	fprintf(stderr, "%s\n\n",error_text(rpc_status));
#endif /* DEBUG */
	return ( MAN_C_FAILURE );
	}

    rpc_string_free( &string_binding,&rpc_status);

    if (rpc_status != error_status_ok)
	{
#ifdef DEBUG 
	fprintf(stderr, "Error freeing string_binding\n");
#endif /* DEBUG */
	return (MAN_C_FAILURE);
	}

    rpc_binding_set_object ( (rpc_binding_handle_t)*mold_handle,
			     &mold_obj,
			     &rpc_status );

    if (rpc_status != error_status_ok)
	{
#ifdef DEBUG
	fprintf(stderr, "Error in rpc_binding_set_object\n");
	fprintf(stderr, "%s\n\n",error_text(rpc_status));
#endif /* DEBUG */
	return (MAN_C_FAILURE);
	}
    /*
     * We now have a binding to mold.
     */

#endif /* NOIPC */

    return( MAN_C_SUCCESS ) ;
}  /* end of moss_alloc_mold_handle() */


man_status
moss_free_mold_handle(
		      mold_handle
		      )

man_binding_handle mold_handle ;

/*
 *
 * Function Description:
 *
 *	Free up RPC handle.
 *
 * Parameters:
 *
 *	None.
 *
 * Return value:
 *
 *	MAN_C_SUCCESS    Successfully unbound
 *      MAN_C_FAILURE    Unable to release the RPC resources
 *
 * Side effects:
 *
 *	None
 *
 */

{

#ifndef NOIPC

    error_status_t rpc_status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     * Simply free the RPC handle! RPC removes all the 
     * memory and associated gronk.
     */
    
    if (mold_handle == (man_binding_handle) NULL)
	return (MAN_C_SUCCESS);
    else
	{
        rpc_binding_free ( (rpc_binding_handle_t *)&mold_handle, 
			  &rpc_status ) ;

        if ( rpc_status != error_status_ok )
	    {
#ifdef DEBUG 
	    fprintf(stderr, "Error on rpc_binding_free: %s\n",error_text(rpc_status));
#endif /* DEBUG */
            return( MAN_C_FAILURE ) ;
	    }
	}
#endif /*NOIPC */
	
    return (MAN_C_SUCCESS);

}  /* end of moss_free_mold_handle() */

/* end of moss_mobind.c */



