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
static char *rcsid = "@(#)$RCSfile: moss_reg.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:59:48 $";
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
 *    Support routines for registering and deregistering with MOLD.
 *    These routines are:
 *        moss_register()
 *        moss_extended_register()
 *        moss_deregister()
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
 *    Miriam Amos Nihart, July 12th, 1990.
 *
 *    Modified moss_register and moss_deregister to check mold_handle 
 *    before call to rpc_$inq_binding.
 *
 *    Miriam Amos Nihart, August 1st, 1990.
 *
 *    Fix the pfm_$ calls.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change bzero to the ansi equivalent memset.
 *   
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.
 *
 *    Wim Colgate, AUgust 28th, 1991.
 *
 *    Modified moss_register and moss_deregister to use the new mold routines.
 *    Also added moss_extended_register for new styles of registration.
 *
 *    Kathy Faust, Sept 6th, 1991
 *
 *    Upgrades to DCE RPC; modified moss_extended_register to use DCE RPC
 *
 *    Kathy Faust, October 25, 1991
 *
 *    Modified moss_register, moss_deregister, and moss_extended_register to
 *    include explicit rpc binding handle.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Updated for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *	Mike Densmore, 1-Apr-1992
 *
 *	Added cma.h to includes for RPC variant
 */

#if defined(__osf__) && !defined(_OSF_SOURCE)
# define _OSF_SOURCE
# include <sys/types.h>
# undef _OSF_SOURCE
#else
# include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>

#ifndef NOIPC
#include <cma.h>
#include "exc_handling.h"
#include "rpcexc.h"
#endif /* NOIPC */

/*
 *  Support header files
 */  

#ifndef NOIPC
#include "mold.h"
#endif /* NOIPC */

#include "man_data.h"
#include "man.h"

#ifdef NOIPC
extern man_status mold_register_mom() ;
extern man_status mold_deregister_mom() ;
#endif

/*
 *  MOSS Specific header files
 */

#include "moss_private.h"
#include "extern_nil.h"


/*
 *  External
 */

#ifdef DEBUG
extern void print_exception() ;
#endif



man_status
moss_register(
	      mold_handle ,
              man_handle ,
              parent_class ,
              object_class
             )

man_binding_handle mold_handle ;
management_handle *man_handle ;
object_id *parent_class ;
object_id *object_class ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call mold_register_mom().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    mold_handle            explicit rpc binding handle to MOLD
 *    man_handle             opaque management handle
 *    parent_class           object id of the parent object class
 *    object_class           object id of the object class registering
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                 Clasa successfully registered
 *    MAN_C_NO_SUCH_PARENT_CLASS    There is no registered class that matches the parent class
 *    MAN_C_OBJECT_ALREADY_EXISTS   There already exisist a registered class
 *    MAN_C_NO_MOLD                 No MOLD to register with
 *    MAN_C_MOLD_TIMEOUT            RPC timeout to MOLD
 *
 * Side effects:
 *
 *    None
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
     *  Check the state of the mold handle.  If it is not bound, bind to mold.
     */

    if ( mold_handle == (man_binding_handle)0 )
    {
        return_status = moss_alloc_mold_handle( &mold_handle ) ;
        if ( return_status != MAN_C_SUCCESS )
            return( MAN_C_NO_MOLD ) ;
    }

    /*
     *  Clean up Handler for RPC call to MOLD
     */

    TRY

#endif /* NOIPC */

         return_status = ( man_status )mold_register_mom( mold_handle , 
							  man_handle ,
                                                          parent_class ,
                                                          object_class,
                                                          MAN_M_CONTAINMENT,
                                                          MAN_M_MSI,
                                                          0 ) ;

#ifndef NOIPC
    CATCH_ALL

#ifdef DEBUG
	fprintf(stderr, "Exception from mold_register_mom:\n\n");
	print_exception(THIS_CATCH); 
#endif
	return_status = MAN_C_NO_MOLD;
	if (exc_matches(THIS_CATCH,&rpc_x_comm_failure))
	    return_status = MAN_C_MOLD_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY
#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_register() */



man_status
moss_deregister(
		mold_handle ,
                man_handle ,
                object_class
               )

man_binding_handle mold_handle ;
management_handle *man_handle ;
object_id *object_class ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call mold_deregister_mom().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    mold_handle            explicit rpc binding handle to MOLD
 *    man_handle             opaque management handle
 *    object_class           class specification associated with the return data
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                Reply sent successfully
 *    MAN_C_NO_MOLD                No MOLD to deregister from
 *    MAN_C_NO_SUCH_CLASS          There is no registered class that matches the object id
 *    MAN_C_HAS_ACTIVE_CHILDREN    The object class specified has registered children
 *
 * Side effects:
 *
 *    None
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
     *  Check the state of the mold handle.  If it is not bound, bind to mold.
     */

    if ( mold_handle == (man_binding_handle)0 )
    {
        return_status = moss_alloc_mold_handle( &mold_handle ) ;
        if ( return_status != MAN_C_SUCCESS )
            return( MAN_C_NO_MOLD ) ;
    }

    /*
     *  Clean up Handler for RPC call to MOLD
     */

    TRY
#endif /* NOIPC */

    return_status = ( man_status )mold_deregister_mom( mold_handle , 
						       man_handle ,
                                                       object_class ) ;

#ifndef NOIPC
    CATCH_ALL

	return_status = MAN_C_NO_MOLD;

	if (exc_matches(THIS_CATCH,&rpc_x_comm_failure))
  	    return_status = MAN_C_MOLD_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY
#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_set_reply() */



man_status
moss_extended_register(
		        mold_handle ,
                        man_handle ,
                        parent_class ,
                        object_class ,
                        registration_type ,
                        supported_interface
                      )

man_binding_handle mold_handle ;
management_handle *man_handle ;
object_id *parent_class ;
object_id *object_class ;
unsigned int registration_type ;
unsigned int supported_interface ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call mold_register_mom().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    mold_handle            explicit rpc binding handle to MOLD
 *    man_handle             opaque management handle
 *    parent_class           object id of the parent object class
 *    object_class           object id of the object class registering
 *    registration_type      a bit mask representing how the mom is registered with mold
 *    supported_interface    a bit mask representing the interfaces supported by this mom
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                 Clasa successfully registered
 *    MAN_C_NO_SUCH_PARENT_CLASS    There is no registered class that matches the parent class
 *    MAN_C_OBJECT_ALREADY_EXISTS   There already exisist a registered class
 *    MAN_C_NO_MOLD                 No MOLD to register with
 *    MAN_C_MOLD_TIMEOUT            RPC timeout to MOLD
 *    MAN_C_INVALID_ARGUMENT_VALUE  One of the arguments (registration_type or supported interface) is invalid.
 *
 * Side effects:
 *
 *    None
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
     *  Check the state of the mold handle.  If it is not bound, bind to mold.
     */

    if ( mold_handle == (man_binding_handle)0 )
    {
        return_status = moss_alloc_mold_handle( &mold_handle ) ;
        if ( return_status != MAN_C_SUCCESS )
            return( MAN_C_NO_MOLD ) ;
    }

    /*
     *  Clean up Handler for RPC call to MOLD
     */

    TRY

#endif /* NOIPC */

        return_status = ( man_status )mold_register_mom( mold_handle , 
							man_handle ,
							parent_class ,
							object_class ,
							registration_type ,
							supported_interface ,
							0 ) ;

#ifndef NOIPC

    CATCH_ALL

#ifdef DEBUG
        fprintf(stderr, "Exception from mold_register_mom:\n\n");
        print_exception(THIS_CATCH);
#endif
        return_status = MAN_C_NO_MOLD ;
        if ( exc_matches( THIS_CATCH, &rpc_x_comm_failure ) )
	    return_status = MAN_C_MOLD_TIMEOUT ;

    ENDTRY
#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_extended_register() */


/* end of moss_register.c */


