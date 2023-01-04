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
static char *rcsid = "@(#)$RCSfile: moss_server.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:33:40 $";
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
 *    Support routine for starting and stopping the MOM server.
 *    These routines are:
 *        moss_start_server()
 *        moss_stop_server()
 *
 * Author:
 *
 *    Miriam Amos Nihart
 *
 * Date:
 *
 *    June 28th, 1990
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, July 4th, 1990.
 *
 *    Put in the #ifdef for using UNIX Domain RPC.
 *
 *    Miriam Amos Nihart, August 3rd, 1990.
 *
 *    Put in check for NULL arguement to moss_start_server().
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change bzero to the ansi equivalent memset.
 *
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.
 *
 *    Kathy Faust, October 25, 1991
 *
 *    Added two new args to moss_start_server:  num_threads and man_handle;
 *    Added new moss_stop_server call.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Updated for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *	Mike Densmore, 1-Apr-1992
 *
 *	Added cma.h to includes for RPC variant.
 */

#include <string.h>
#ifndef NOIPC
#include <cma.h>
#include "rpc.h"
#endif /* NOIPC */

/*
 *  Support header files
 */  

#include "man.h"
#include "man_data.h"

/*
 *  MOSS Specific header files
 */

#include "moss_private.h"
#include "extern_nil.h"

/*
 *  External
 */
#ifndef NOIPC
extern int rpc_server_listen(), rpc_mgmt_stop_server_listening();
#endif


man_status
moss_start_server(
                  mo_sig_handler ,
		  num_threads ,
		  man_handle
                 )

PFV *mo_sig_handler ;
int num_threads ;
management_handle *man_handle ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC calls need to setup up as a MOM server.
 *    Control is relinquished to the rpc library and returned only at the
 *    moi_*() entry points.
 *
 * Parameters:
 *
 *    mo_sig_handler    the address of the signal handler for the MOM
 *    num_threads       the number of concurrent rpc calls supported
 *    man_handle        not used by U*x implementation; needed by VMS
 *
 * Return value:
 *
 *    If successful it will not return.
 *
 *    MAN_C_PROCESSING_FAILURE    A NCS RPC call failed
 *
 * Side effects:
 *
 *    Control is only returned at the moi_*() entry points.
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;
#ifndef NOIPC
    error_status_t st ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( mo_sig_handler == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    /*
     *  Ready to listen on socket.  This should not return from
     *  the rpc_server_listen except when a signal is received.
     */

    TRY

         rpc_server_listen( num_threads, &st ) ;

    CATCH_ALL

	(*mo_sig_handler)();
    
        return_status = MAN_C_FAILURE ;

    ENDTRY


    if ( st != error_status_ok )
        return_status = MAN_C_FAILURE ;

#endif /* NOIPC */

    return( return_status ) ;

}  /* end of moss_start_server() */


man_status
moss_stop_server(
		  man_handle
                 )

management_handle *man_handle ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC calls need to stop the MOM as an RPC
 *    server.
 *
 * Parameters:
 *
 *
 * Return value:
 *
 *    If successful it will not return.
 *
 *
 * Side effects:
 *
 *    Control is only returned at the moi_*() entry points.
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;
#ifndef NOIPC
    error_status_t st ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Stop MOM from acting as a server.
     */

    TRY

         rpc_mgmt_stop_server_listening( NULL, &st ) ;

    CATCH_ALL

         return_status = MAN_C_FAILURE ;

    ENDTRY

    if ( st != error_status_ok )
        return_status = MAN_C_FAILURE ;

#endif /* NOIPC */

    return( return_status ) ;
}  /* end of moss_stop_server() */
/* end of moss_server.c */


