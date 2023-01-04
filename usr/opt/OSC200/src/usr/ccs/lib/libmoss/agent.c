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
static char *rcsid = "@(#)$RCSfile: agent.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:58:33 $";
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
 *    The following routines are the main body of the Agent.
 *        man_mgmt_request()
 *        man_dispatch()
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    October 25, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, March 1st, 1990
 *
 *    Put in checks after an RPC library call.  If the call fails return
 *    MAN_C_PROCESSING_FAILURE unless within DEBUG, which only prints a
 *    message.
 *
 *    Miriam Amos Nihart, April 16th, 1990
 *
 *    Put in NOIPC ifdefs for the single image CA kit.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Correct file names to reflect 14 character restriction.
 *
 *    Miriam Amos Nihart, June 26th, 1990.
 *
 *    Changes return code from a failed moss_bind_to_mold to MAN_C_NO_MOLD.
 *
 *    Miriam Amos Nihart, June 27th, 1990.
 *
 *    Use rpc_$free_handle() rather than rpc_$free() for releasing the mo_handle.
 *
 *    Miriam Amos Nihart, August 1st, 1990.
 *
 *    Fix the pfm_$ calls.
 *
 *    Miriam Amos Nihart, October 8th, 1990.
 *
 *    Fix memory accumulation in man_mgmt_request.
 *
 *    Miriam Amos Nihart, October 29th, 1990.
 *
 *    Change the bzero() to memset().
 *
 *    Miriam Amos Nihart, November 8th, 1990.
 *
 *    Changes for new format of moss_avl_init() and moss_avl_free().
 *
 *    Miriam Amos Nihart, November 11th, 1990.
 *
 *    Changes access control from char * to avl *.
 *
 *    Jim Teague, March 4th, 1991.
 *
 *    Port to DCE RPC.
 *
 *    Miriam Amos Nihart, May 29th, 1991.
 *
 *    Rest the avls prior to mio calls in the NOIPC model.
 *
 *    Wim Colgate, August, 28th, 1991.
 *
 *    Changed mold_find_object call to mold_find_mom.
 *
 *    Miriam Amos Nihart, October 16th, 1991.
 *
 *    Fixes for prototyping.
 *
 *    Kathy Faust, October 25th, 1991.
 *
 *    Modified for explicit rpc binding.
 * 
 *    Wim Colgate, November, 18th, 1991.
 *
 *    Added casting for rpc_binding_from_string_binding(), (takes unsigned
 *    char * instead of char *)
 *
 *    Kathy Faust, December 9th, 1991
 *
 *    Update for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *    Mike Densmore, 17-May-1992
 *
 *    Fixed bug in rpc_binding_from_string_binding call in man_dispatch.  Old
 *    was passing an string which was not properly terminated.  (Note: cma.h
 *    was added to include files for EFT/SSB DEC Starter Kit compatibility
 *    as well.)
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 *
 *   - mold_find_mom() is prototyped in mold.h but has a mismatched type
 *     free_avl, so we prototype it here
 */

extern int mold_find_mom() ;

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
extern int rpc_binding_from_string_binding(), rpc_binding_free();
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#endif /* NOIPC */

/*
 *  Support header files
 */  

#ifndef NOIPC
#include <cma.h>
#include "rpc.h"
#include "rpcexc.h"
#include "exc_handling.h"
#endif /* NOIPC */

/*
 *  Management header files
 */

#include "moss.h"
#include "agent.h"
#include "agent_priv.h"

/*
 *  Data
 */

/*
 *  External
 */



man_status
man_mgmt_request(
                 operation_type ,
                 object_class_p ,
                 object_instance_p ,
                 iso_scope ,
                 filter_p ,
                 access_control_p ,
                 synchronization ,
                 attribute_list_p ,
                 invoke_identifier ,
                 return_routine_p ,
                 action_type_p ,
                 action_argument_p ,
                 reference_instance_p ,
                 superior_instance_p
                )
/*
 *
 * Function description:
 *
 *    This is the routine for processing the management operations :
 *    get, set, create, delete, and action.
 *
 * Arguments:
 *
 *    operation_type       the specific management operation
 *    object_class_p       the address of the object class id for the target mo
 *    object_instance_p    the address of the instance name for the target mo
 *    iso_scope            ISO scope information
 *    filter_p             the address of the filter for management operation
 *    access_control_p     the address of the avl containing the access information
 *    synchronization      whether management operation is atomic or best effort
 *    attribute_list_p     the address of an attribute id/value pair list or an attribute id list
 *    invoke_identifier    unique id assigned by the protocol engine
 *    return_routine_p     the address of the opaque data for the mo to locate the pe's reply reciever
 *    action_type_p        the address of the action type for the action mangement operation
 *    action_argument_p    the address of the action arguments for the action management operation
 *    reference_instance_p the address of the name of reference instance for the create management operation
 *    superior_instance_p  the address of the name of the superior instance for the create management operation
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Management operation received and processed successfully
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_INVALID_SCOPE              Invalid scope value
 *    MAN_C_MO_TIMEOUT                 RPC timeout to the MO
 *    MAN_C_MOLD_TIMEOUT               RPC timeout to the MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC timeout to the Protocol Engine's Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

int operation_type ;
object_id *object_class_p ;
avl *object_instance_p ;
scope iso_scope ;
avl *filter_p ;
avl *access_control_p ;
int synchronization ;
avl *attribute_list_p ;
int invoke_identifier ;
management_handle *return_routine_p ;
object_id *action_type_p ;
avl *action_argument_p ;
avl *reference_instance_p ;
avl *superior_instance_p ;

{
    avl *return_info = ( avl * )NULL ;
    object_id *oid_p ;
    unsigned int type ;
    unsigned int tag ;
    int last_one = FALSE ;
    octet_string *mo_rpc_info_p ;
    man_binding_handle mold_handle ;
    man_status return_status ;
    man_status mold_status ;
    man_status moss_status ;

#ifndef NOIPC
      int got_an_exception = 0; 
#endif /* NOIPC */

    /*
     *  Check for unsupported combinations of parameters.
     */

    if ( synchronization == MAN_C_ISO_ATOMIC ) 
        return ( MAN_C_SYNC_NOT_SUPPORTED ) ;

    /*
     *  Set up for call to MOLD.  Zero the avl to guarantee no random
     *  garbage off the stack.
     */

    moss_status = ( man_status )moss_alloc_mold_handle( &mold_handle ) ;
    if ( moss_status != MAN_C_SUCCESS )
        return( MAN_C_NO_MOLD ) ;

    moss_status = moss_avl_init( &return_info ) ;
    if ( moss_status != MAN_C_SUCCESS )
        return( MAN_C_PROCESSING_FAILURE ) ;

#ifndef NOIPC

    /*
     *  TRY clause for RPC call to MOLD
     */
    
    TRY

#endif /* NOIPC */

       mold_status = ( man_status )mold_find_mom( mold_handle, /* RPC call */ 
						 object_class_p ,
						 iso_scope ,
						 return_info ) ;

#ifndef NOIPC

    /*
     *  Cleanup handler 
     */
    CATCH_ALL
	moss_free_mold_handle( mold_handle );
	moss_avl_free( &return_info, TRUE );
	got_an_exception = TRUE ;
	moss_status = MAN_C_NO_MOLD;
	if (exc_matches(THIS_CATCH,&rpc_x_comm_failure))
	    moss_status = MAN_C_MOLD_TIMEOUT;

    ENDTRY

    if (got_an_exception)
	return ( moss_status ); 
 	
    moss_free_mold_handle( mold_handle );

#endif /* NOIPC */

    moss_status = moss_avl_reset( return_info ) ;

    /*
     *  If a valid mo was found then call it.
     */

    if ( mold_status == MAN_C_SUCCESS )
    {
        /*
         *  Extract info from avl
         */

        while ( last_one != TRUE )
        {
            moss_status = moss_avl_point( return_info ,
                                          &oid_p ,
                                          &type ,
                                          &tag ,
                                          &mo_rpc_info_p ,
                                          &last_one ) ;

            if ( moss_status == MAN_C_NO_ELEMENT )
            {
                moss_avl_free( &return_info, TRUE ) ;
                return( MAN_C_NO_SUCH_CLASS ) ;
            }

            /*
             *  Call the MO via dispatch routine.
             */

            return_status = man_dispatch( mo_rpc_info_p ,
                                          operation_type ,
                                          oid_p ,
                                          object_instance_p ,
                                          iso_scope ,
                                          filter_p ,
                                          access_control_p ,
                                          synchronization ,
                                          attribute_list_p ,
                                          invoke_identifier ,
                                          return_routine_p ,
                                          action_type_p ,
                                          action_argument_p ,
                                          reference_instance_p ,
                                          superior_instance_p )  ;

            /*
             *  If the MO was unable to return the reply to the Protocol Engine's
             *  Reply Reciever, then quit processing.
             */

            if ( return_status == MAN_C_PE_TIMEOUT )
                break ;
        }

        /*
         *  If this is a scoped call and one of the MOs did not response - forget it.
         */
        if ( ( return_status == MAN_C_MO_TIMEOUT ) &&
             ( iso_scope != MAN_C_ZERO_LEVEL_SCOPE ) )
            return_status = MAN_C_SUCCESS ;
    }
    else
        /*
         *  Either no mo found or an invalid scope - whatever, it is in the mold status.
         */

        return_status = mold_status ;

    moss_avl_free( &return_info, TRUE ) ;
    return ( return_status ) ;
} /* end man_mgmt_request */


man_status
man_dispatch(
             rpc_handle_p ,
             operation_type ,
             object_class_p ,
             object_instance_p ,
             iso_scope ,
             filter_p ,
             access_control_p ,
             synchronization ,
             attribute_list_p ,
             invoke_identifier ,
             return_routine_p ,
             action_type_p ,
             action_argument_p ,
             reference_instance_p ,
             superior_instance_p
            )
/*
 *
 * Function description:
 *
 *    This routine is the point at which the Manageable Object is called with
 *    the management operation.
 *
 * Arguments:
 *
 *    rpc_handle_p         the address of the rpc object uuid for finding the target mo
 *    operation_type       the specific managemnt operation
 *    object_class_p       the address of the object class id for the target mo
 *    object_instance_p    the address of the instance name for the target mo
 *    iso_scope            ISO scope information
 *    filter_p             the address of the filter for management operation
 *    access_control_p     the address of the avl containing the access information
 *    synchronization      whether management operation is atomic or best effort
 *    attribute_list_p     the address of an attribute id/value pair list or an attribute id list
 *    invoke_identifier    the unique id assigned to the management operation by the PE
 *    return_routine_p     the address of the opaque data for the mo to find the PE Reply Reciever
 *    action_type_p        the address of the action type for the action mangement operation
 *    action_argument_p    the address of the action arguments for the action management operation
 *    reference_instance_p the address of the name of reference instance for the create management operation
 *    superior_instance_p  the address of the name of the superior instance for the create management operation
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Management operation received and processed successfully
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processin the operation
 *    MAN_C_MO_TIMEOUT                 RPC timeout to the MO
 *    MAN_C_PE_TIMEOUT                 RPC timeout to the Protocol Engine's Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *
 * Side effects:
 *
 *    None
 *
 * Notes:
 *
 *    The dispatch table makes an RPC call to the appropriate moi interface.
 *    The cleanup handler surrounding the call prevents RPC from terminating
 *    the process (ie the protocol engine).  If the RPC does not complete,
 *    the cleanup handle traps the error and returns an appropriate value.
 *    If all goes well, the return value from the moi call is passed back.
 */

octet_string *rpc_handle_p ;
int operation_type ;
object_id *object_class_p ;
avl *object_instance_p ;
scope iso_scope ;
avl *filter_p ;
avl *access_control_p ;
int synchronization ;
avl *attribute_list_p ;
int invoke_identifier ;
management_handle *return_routine_p ;
object_id *action_type_p ;
avl *action_argument_p ;
avl *reference_instance_p ;
avl *superior_instance_p ;

{
    man_status return_status ;
    man_binding_handle mom_handle = (man_binding_handle)0 ;

    char *bindingstring = NULL;	    /* string_binding arg for rpc call */

#ifndef NOIPC
    error_status_t rpc_status ;

    /*
     *  Set up communications (this is where man_handle is used).
     *  Man_handle contains the socket address and length.  No need to
     *  go through the Local Location Broker.
     */

    /*
     *  Copy the string from the octet_string structure into a char array
     *  to assure that it is null terminated for the rpc routine.
     */

    bindingstring = (char *) malloc (rpc_handle_p->length + 1);	    /* allocate space for string plus one additional byte for...*/
    memset( bindingstring, '\0', rpc_handle_p->length + 1 ) ;	    /* ...term.  fill with \0 to assure null terminator */
    strncpy (bindingstring, rpc_handle_p->string, rpc_handle_p->length);   /* copy binding string */

    rpc_binding_from_string_binding ( (unsigned char *)bindingstring, 
				      (handle_t *)&mom_handle,
				      &rpc_status );
    if (bindingstring != NULL)
    {
	free(bindingstring);
    }
	
    if ( rpc_status != error_status_ok )
	return ( MAN_C_PROCESSING_FAILURE );
    /*
     *  Set up cleanup handler around RPC call to MO.
     */

    TRY 

#else

    /*
     *  Reset the avl parametes prior to moi calls.
     */

    return_status = moss_avl_reset( object_instance_p ) ;  
    return_status = moss_avl_reset( filter_p ) ;  
    return_status = moss_avl_reset( access_control_p ) ;  
    return_status = moss_avl_reset( attribute_list_p ) ;  
    return_status = moss_avl_reset( action_argument_p ) ;  
    return_status = moss_avl_reset( reference_instance_p ) ;  
    return_status = moss_avl_reset( superior_instance_p ) ;  

#endif /* NOIPC */

        switch ( operation_type )
        {
            case MAN_C_GET :
                return_status = ( man_status )moi_get_attributes( mom_handle , /* RPC call */
								  object_class_p ,
                                                                  object_instance_p ,
                                                                  iso_scope ,
                                                                  filter_p ,
                                                                  access_control_p ,
                                                                  synchronization ,
                                                                  attribute_list_p ,
                                                                  invoke_identifier ,
                                                                  return_routine_p ) ;
                break ;

            case MAN_C_SET :
                return_status = ( man_status )moi_set_attributes( mom_handle , /* RPC call */
								  object_class_p ,
                                                                  object_instance_p ,
                                                                  iso_scope ,
                                                                  filter_p ,
                                                                  access_control_p ,
                                                                  synchronization ,
                                                                  attribute_list_p ,
                                                                  invoke_identifier ,
                                                                  return_routine_p ) ;
                break ;

            case MAN_C_CREATE:
                return_status = ( man_status )moi_create_instance( mom_handle , /* RPC call */
								   object_class_p ,
                                                                   object_instance_p ,
                                                                   superior_instance_p ,
                                                                   access_control_p ,
                                                                   reference_instance_p ,
                                                                   attribute_list_p ,
                                                                   invoke_identifier ,
                                                                   return_routine_p ) ;
                break ;

            case MAN_C_DELETE :
                return_status = ( man_status )moi_delete_instance( mom_handle , /* RPC call */
								   object_class_p ,
                                                                   object_instance_p ,
                                                                   iso_scope ,
                                                                   filter_p ,
                                                                   access_control_p ,
                                                                   synchronization ,
                                                                   invoke_identifier ,
                                                                   return_routine_p ) ;
                break ;

            case MAN_C_ACTION :
                return_status = ( man_status )moi_invoke_action( mom_handle , /* RPC call */
								 object_class_p ,
                                                                 object_instance_p ,
                                                                 iso_scope ,
                                                                 filter_p ,
                                                                 access_control_p ,
                                                                 synchronization ,
                                                                 action_type_p ,
                                                                 action_argument_p ,
                                                                 invoke_identifier ,
                                                                 return_routine_p ) ;
                break ;
        }

#ifndef NOIPC
    CATCH_ALL

	return_status = MAN_C_MO_TIMEOUT;

    /*
     *  Release cleanup handler.
     */

    ENDTRY


    /*
     *  Clear the RPC binding in preparation for the next scoped call.
     */
    rpc_binding_free( &mom_handle, &rpc_status ) ;

#ifdef DEBUG
    if ( rpc_status != error_status_ok )
        fprintf( stderr, "Error freeing mom_handle.\n" ) ;
#endif /* DEBUG */
#endif /* NOIPC */

    return ( return_status ) ;
} /* end man_dispatch */

/* end agent.c */

