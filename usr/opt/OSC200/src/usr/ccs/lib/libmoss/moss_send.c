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
static char *rcsid = "@(#)$RCSfile: moss_send.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:33:31 $";
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
 *    Support routines for sending replies from the MOM to the PERR.
 *    These routines are:
 *        moss_send_get_reply()
 *        moss_send_set_reply()
 *        moss_send_create_reply()
 *        moss_send_delete_reply()
 *        moss_send_action_reply()
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
 *    Modifications to check pe_handle before call to rpc_$inq_binding.
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
 *    Miriam Amos Nihart, May 29th, 1991.
 *
 *    Reset avls before pei_* calls and add action type and
 *    action response type to moss_send_action_reply.
 *
 *    Miriam Amos Nihart, June 3rd, 1991.
 *
 *    Remove more argument from moss_send_create_reply.
 *
 *    Kathy Faust, October 25, 1991.
 *
 *    Add explicit binding handle arguments.
 *
 *    Kathy Faust, December 9th, 1991.
 *
 *    Updated for dce-starter-kit; changed cma_exception.h to exc_handling.h
 *
 *	Mike Densmore, 11-Apr-1992
 *
 *	Added cma.h to includes for RPC variant
 */

#ifdef NOIPC
extern int pei_send_get_reply() ;
extern int pei_send_set_reply() ;
extern int pei_send_create_reply() ;
extern int pei_send_delete_reply() ;
extern int pei_send_action_reply() ;
#endif

#include <string.h>
#ifndef NOIPC
#include <cma.h>
#include "exc_handling.h"
#endif /* NOIPC */

/*
 *  Support header files
 */  

#ifndef NOIPC
#include "pe.h"
#endif /* NOIPC */

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
globalref rpc_binding_handle_t pe_handle ;
#endif /* NOIPC */



man_status
moss_send_get_reply(
		    pe_handle ,
                    invoke_identifier ,
                    reply ,
                    object_class ,
                    object_instance ,
                    object_uid ,
                    operation_time ,
                    attribute_list ,
                    more_replies
                   )

man_binding_handle pe_handle ;
int invoke_identifier ;
reply_type reply ;
object_id *object_class ;
avl *object_instance ;
uid *object_uid ;
mo_time operation_time ;
avl *attribute_list ;
int more_replies ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call pei_send_get_reply().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    pe_handle              explicit rpc binding handle for PE
 *    invoke_identifier      id associated with the request
 *    reply                  type of reply information returned
 *    object_class           class specification associated with the return data
 *    object_instance        instance name associated with the return data
 *    object_uid             unique id assigned to the instance
 *    operation_time         execution time of the request
 *    attribute_list         return data
 *    more_replies           flag indicating more return data to follow
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Reply sent successfully
 *    MAN_C_HANDLE_NOT_BOUND           Not bound to the PERR
 *    MAN_C_INVALID_INVOKE_IDENTIFIER  No corresponding invoke identifier for MO's reply
 *    MAN_C_PE_TIMEOUT                 Unable to send reply to PERR
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
     *  Check the state of the pe handle.  If it is not bound
     *  return MAN_C_HANDLE_NOT_BOUND.
     */

    if ( pe_handle == ( man_binding_handle )0 )
        return( MAN_C_HANDLE_NOT_BOUND ) ;

    /*
     *  Clean up Handler for RPC call to PERR.
     */

    TRY

#else

    /*
     *  Reset the avl parameters back to the beginning.
     */

    return_status = moss_avl_reset( object_instance ) ;
    return_status = moss_avl_reset( attribute_list ) ;

#endif /* NOIPC */

         return_status = ( man_status )pei_send_get_reply( pe_handle ,
							  invoke_identifier ,
                                                          reply ,
                                                          object_class ,
                                                          object_instance ,
                                                          object_uid ,
                                                          operation_time ,
                                                          attribute_list ,
                                                          more_replies ) ;

#ifndef NOIPC

    CATCH_ALL

	return_status = MAN_C_PE_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY

#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_get_reply() */



man_status
moss_send_set_reply(
		    pe_handle ,
                    invoke_identifier ,
                    reply ,
                    object_class ,
                    object_instance ,
                    object_uid ,
                    operation_time ,
                    attribute_list ,
                    more_replies
                   )

man_binding_handle pe_handle ;
int invoke_identifier ;
reply_type reply ;
object_id *object_class ;
avl *object_instance ;
uid *object_uid ;
mo_time operation_time ;
avl *attribute_list ;
int more_replies ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call pei_send_set_reply().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    pe_handle              explicit rpc binding handle to PE
 *    invoke_identifier      id associated with the request
 *    reply                  type of reply information returned
 *    object_class           class specification associated with the return data
 *    object_instance        instance name associated with the return data
 *    object_uid             unique id assigned to the instance
 *    operation_time         execution time of the request
 *    attribute_list         return data
 *    more_replies           flag indicating more return data to follow
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Reply sent successfully
 *    MAN_C_HANDLE_NOT_BOUND          Not bound to the PERR
 *    MAN_C_INVALID_INVOKE_IDENTIFIER  No corresponding invoke identifier for MO's reply
 *    MAN_C_PE_TIMEOUT                 Unable to send reply to PERR
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;

#ifndef NOIPC

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Check the state of the pe handle.  If it is not bound
     *  return MAN_C_HANDLE_NOT_BOUND.
     */

    if ( pe_handle == (man_binding_handle)0  )
        return( MAN_C_HANDLE_NOT_BOUND ) ;

    /*
     *  Clean up Handler for RPC call to PERR.
     */

    TRY

#else

    /*
     *  Reset the avl parameters back to the beginning.
     */

    return_status = moss_avl_reset( object_instance ) ;
    return_status = moss_avl_reset( attribute_list ) ;

#endif /* NOIPC */

         return_status = ( man_status )pei_send_set_reply( pe_handle ,
							  invoke_identifier ,
                                                          reply ,
                                                          object_class ,
                                                          object_instance ,
                                                          object_uid ,
                                                          operation_time ,
                                                          attribute_list ,
                                                          more_replies ) ;

#ifndef NOIPC

    CATCH_ALL
	
	return_status = MAN_C_PE_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY

#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_set_reply() */



man_status
moss_send_create_reply(
		       pe_handle ,
                       invoke_identifier ,
                       reply ,
                       object_class ,
                       object_instance ,
                       object_uid ,
                       operation_time ,
                       attribute_list
                      )

man_binding_handle pe_handle ;
int invoke_identifier ;
reply_type reply ;
object_id *object_class ;
avl *object_instance ;
uid *object_uid ;
mo_time operation_time ;
avl *attribute_list ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call pei_send_create_reply().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    pe_handle              explicit rpc binding handle to PE
 *    invoke_identifier      id associated with the request
 *    reply                  type of reply information returned
 *    object_class           class specification associated with the return data
 *    object_instance        instance name associated with the return data
 *    object_uid             unique id assigned to the instance
 *    operation_time         execution time of the request
 *    attribute_list         return data
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Reply sent successfully
 *    MAN_C_HANDLE_NOT_BOUND          Not bound to the PERR
 *    MAN_C_INVALID_INVOKE_IDENTIFIER  No corresponding invoke identifier for MO's reply
 *    MAN_C_PE_TIMEOUT                 Unable to send reply to PERR
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;

#ifndef NOIPC

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Check the state of the pe handle.  If it is not bound
     *  return MAN_C_HANDLE_NOT_BOUND.
     */

    if ( pe_handle == (man_binding_handle)0 )
        return( MAN_C_HANDLE_NOT_BOUND ) ;

    /*
     *  Clean up Handler for RPC call to PERR.
     */

    TRY

#else

    /*
     *  Reset the avl parameters back to the beginning.
     */

    return_status = moss_avl_reset( object_instance ) ;
    return_status = moss_avl_reset( attribute_list ) ;

#endif /* NOIPC */

         return_status = ( man_status )pei_send_create_reply( pe_handle ,
							     invoke_identifier ,
                                                             reply ,
                                                             object_class ,
                                                             object_instance ,
                                                             object_uid ,
                                                             operation_time ,
                                                             attribute_list ) ;

#ifndef NOIPC
    CATCH_ALL

	return_status = MAN_C_PE_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY

#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_create_reply() */



man_status
moss_send_delete_reply(
		       pe_handle ,
                       invoke_identifier ,
                       reply ,
                       object_class ,
                       object_instance ,
                       object_uid ,
                       operation_time ,
                       attribute_list ,
                       more_replies
                      )

man_binding_handle pe_handle ;
int invoke_identifier ;
reply_type reply ;
object_id *object_class ;
avl *object_instance ;
uid *object_uid ;
mo_time operation_time ;
avl *attribute_list ;
int more_replies ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call pei_send_delete_reply().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    pe_handle              explicit rpc binding handle for PE
 *    invoke_identifier      id associated with the request
 *    reply                  type of reply information returned
 *    object_class           class specification associated with the return data
 *    object_instance        instance name associated with the return data
 *    object_uid             unique id assigned to the instance
 *    operation_time         execution time of the request
 *    attribute_list         return data
 *    more_replies           flag indicating more return data to follow
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Reply sent successfully
 *    MAN_C_HANDLE_NOT_BOUND          Not bound to the PERR
 *    MAN_C_INVALID_INVOKE_IDENTIFIER  No corresponding invoke identifier for MO's reply
 *    MAN_C_PE_TIMEOUT                 Unable to send reply to PERR
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;

#ifndef NOIPC

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Check the state of the pe handle.  If it is not bound
     *  return MAN_C_HANDLE_NOT_BOUND.
     */

    if ( pe_handle == (man_binding_handle)0 )
        return( MAN_C_HANDLE_NOT_BOUND ) ;

    /*
     *  Clean up Handler for RPC call to PERR.
     */

    TRY

#else

    /*
     *  Reset the avl parameters back to the beginning.
     */

    return_status = moss_avl_reset( object_instance ) ;
    return_status = moss_avl_reset( attribute_list ) ;

#endif /* NOIPC */

         return_status = ( man_status )pei_send_delete_reply( pe_handle ,
							     invoke_identifier ,
                                                             reply ,
                                                             object_class ,
                                                             object_instance ,
                                                             object_uid ,
                                                             operation_time ,
                                                             attribute_list ,
                                                             more_replies ) ;

#ifndef NOIPC

    CATCH_ALL

	return_status = MAN_C_PE_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY

#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_delete_reply() */



man_status
moss_send_action_reply(
		       pe_handle ,
                       invoke_identifier ,
                       reply ,
                       object_class ,
                       object_instance ,
                       object_uid ,
                       operation_time ,
                       action_type ,
                       action_response_type ,
                       attribute_list ,
                       more_replies
                      )

man_binding_handle pe_handle ;
int invoke_identifier ;
reply_type reply ;
object_id *object_class ;
avl *object_instance ;
uid *object_uid ;
mo_time operation_time ;
object_id *action_type ;
object_id *action_response_type ;
avl *attribute_list ;
int more_replies ;

/*
 *
 * Function Description:
 *
 *    This routine jackets the RPC call pei_send_action_reply().  It
 *    traps the RPC signals and returns an appropriate error code.
 *
 * Parameters:
 *
 *    pe_handle              explicit rpc binding handle for PE
 *    invoke_identifier      id associated with the request
 *    reply                  type of reply information returned
 *    object_class           class specification associated with the return data
 *    object_instance        instance name associated with the return data
 *    object_uid             unique id assigned to the instance
 *    operation_time         execution time of the request
 *    action_type            object id for action type performed
 *    action_response_type   object id for action specific response type 
 *    attribute_list         return data
 *    more_replies           flag indicating more return data to follow
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    Reply sent successfully
 *    MAN_C_HANDLE_NOT_BOUND          Not bound to the PERR
 *    MAN_C_INVALID_INVOKE_IDENTIFIER  No corresponding invoke identifier for MO's reply
 *    MAN_C_PE_TIMEOUT                 Unable to send reply to PERR
 *
 * Side effects:
 *
 *    None
 *
 */

{
    man_status return_status = MAN_C_SUCCESS ;

#ifndef NOIPC

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Check the state of the pe handle.  If it is not bound
     *  return MAN_C_HANDLE_NOT_BOUND.
     */

    if ( pe_handle == (man_binding_handle)0 )
        return( MAN_C_HANDLE_NOT_BOUND ) ;

    /*
     *  Clean up Handler for RPC call to PERR.
     */

    TRY

#else

    /*
     *  Reset the avl parameters back to the beginning.
     */

    return_status = moss_avl_reset( object_instance ) ;
    return_status = moss_avl_reset( attribute_list ) ;

#endif /* NOIPC */

         return_status = ( man_status )pei_send_action_reply( pe_handle ,
							     invoke_identifier ,
                                                             reply ,
                                                             object_class ,
                                                             object_instance ,
                                                             object_uid ,
                                                             operation_time ,
                                                             action_type ,
                                                             action_response_type ,
                                                             attribute_list ,
                                                             more_replies ) ;

#ifndef NOIPC
    CATCH_ALL

	return_status = MAN_C_PE_TIMEOUT;

    /*
     *  Release cleanup handler and unbind.
     */

    ENDTRY
#endif /* NOIPC */

    return ( return_status ) ;

} /* end of moss_send_action_reply() */

/* end of moss_send.c */
