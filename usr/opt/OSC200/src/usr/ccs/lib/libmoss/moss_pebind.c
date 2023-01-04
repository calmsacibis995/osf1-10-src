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
static char *rcsid = "@(#)$RCSfile: moss_pebind.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:33:04 $";
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
 *    Aid the MOM in allocating and releasing binding handles for the 
 *    Protocol Engines' reply-receiver.
 *    These routines are:
 *        moss_alloc_pe_handle()
 *        moss_free_pe_handle()
 *
 * Author:
 *
 *    Wim Colgate
 *
 * Date:
 *
 *    January 9th, 1990.
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, March 1st, 1990
 *
 *    Put in checks after RPC library calls.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Miriam Amos Nihart, June 20th, 1990.
 *
 *    Correct rpc call in moss_unbind_from_pe.
 *
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.
 *
 *    Kathy Faust, October 28th, 1991.
 *
 *    Changed moss_bind_to_pe to moss_alloc_pe_handle; changed
 *    moss_unbind_from_pe to moss_free_pe_handle.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Updated for dce-starter-kit; changed cma_exception.h to exc_handling.h
 */

#ifndef NOIPC
#include <rpc.h>
#endif /* NOIPC */

/*
 *  Support header files
 */  

#include "man_data.h"
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
extern int rpc_binding_from_string_binding(), rpc_binding_free();
extern char * error_text();
#endif /* NOIPC */


man_status
moss_alloc_pe_handle(
		     man_handle ,
		     pe_handle
		     )

management_handle *man_handle ;
man_binding_handle *pe_handle ;

/*
 *
 * Function Description:
 *
 *	Allocate binding handle to the Protocol Engine.
 *
 * Parameters:
 *
 *	man_handle      management_handle that contains binding information
 *      pe_handle       address in which to return the PE binding handle
 *
 * Return value:
 *
 *	MAN_C_SUCCESS    Successfully allocated
 *	MAN_C_NO_REPLY   No reply was requested (another form of success)
 *      MAN_C_FAILURE    Unable to return binding handle to PE
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
     * If the man_handle for the PE is null (no length),
     * then it is not allowed to send replies back to it.
     */

    if ( man_handle->length == 0 )
        return( MAN_C_NO_REPLY ) ;

    /*
     * Now instantiate a binding to PE.
     */

    rpc_binding_from_string_binding ( man_handle->socket_address, 
				     (rpc_binding_handle_t *)pe_handle, 
				     &rpc_status );

    if ( rpc_status != error_status_ok)
        return( MAN_C_FAILURE ) ;

#endif /* NOIPC */

    return( MAN_C_SUCCESS ) ;
}  /* end of moss_alloc_pe_handle() */


man_status
moss_free_pe_handle(
		    pe_handle
		    )

man_binding_handle pe_handle ;

/*
 *
 * Function Description:
 *
 *	Free up RPC handle.
 *
 * Parameters:
 *
 *	pe_handle      man_binding_handle to be released
 *
 * Return value:
 *
 *	MAN_C_SUCCESS  Successfully unbound
 *      MAN_C_FAILURE  Unable to clear binding
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

    if (pe_handle == (man_binding_handle)0)
	return(MAN_C_SUCCESS);

    rpc_binding_free( &pe_handle, &rpc_status ) ;

    if ( rpc_status != error_status_ok)
        return( MAN_C_FAILURE ) ;

#endif /* NOIPC */
    return( MAN_C_SUCCESS ) ;
}  /* end of moss_free_pe_handle() */

/* end of moss_pebind.c */
