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
static char *rcsid = "@(#)$RCSfile: agent_inter.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 15:58:54 $";
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
 *    The following routines are the Protocol Engine/Agent interface entry
 *    points.  They are:
 *        msi_get_attributes()
 *        msi_set_attributes()
 *        msi_create_instance()
 *        msi_delete_instance()
 *        msi_invoke_action()
 *
 * Author:
 *
 *    Miriam Nihart
 *
 * Date:
 *
 *    November 1, 1989
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14, 1990.
 *
 *    Rename file to comply with 14 character restriction.
 *
 *    Miriam Amos Nihart, June 14th, 1991.
 *
 *    Put in check for return routine on confirmed management
 *    requests - get, create, and delete.
 *
 *    Miriam Amos Nihart, August 13th, 1991.
 *
 *    Changes for the nil avl and filter.
 *
 */

/*
 *  Support header files
 */  

/*
 *  Management header files
 */

#include "agent.h"
#include "moss.h"
#include "extern_nil.h"
#include "agent_priv.h"

/*
 *  External
 */



man_status
msi_get_attributes(
                   object_class ,
                   object_instance ,
                   iso_scope ,
                   filter ,
                   access_control ,
                   synchronization ,
                   attribute_identifier_list ,
                   invoke_identifier ,
                   return_routine
                  )
/*
 *
 * Function description:
 *
 *    This routine is the entry point for the management operation
 *    get.
 *
 * Arguments:
 *
 *    object_class                the address of the object class id for the target mo
 *    object_instance             the address of the instance name for the target mo
 *    iso_scope                   ISO scope information
 *    filter                      the address of the filter for management operation
 *    access_control              the address of the avl containing the access information
 *    synchronization             whether management operation is atomic or best effort
 *    attribute_identifier_list   the address of the an attribute id/value pair list or an attribute id list
 *    invoke_identifier           unique id assigned by the protocol engine
 *    return_routine              the address of the opaque data for locating the return routine for the request
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    replies successfully collected
 *    MAN_C_BAD_PARAMETER              Return routine argument was NULL
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_INVALID_SCOPE              Invalid scope value
 *    MAN_C_MO_TIMEOUT                 RPC time out to MO
 *    MAN_C_MOLD_TIMEOUT               RPC time out to MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC time out to Protocol Engine Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

object_id *object_class ;
avl *object_instance ;
scope iso_scope ;
avl *filter ;
avl *access_control ;
int synchronization ;
avl *attribute_identifier_list ;
int invoke_identifier ;
management_handle *return_routine ;
{
    man_status stat ;

    /*
     *  This is a confirmed management operation.  Check
     *  for return_routine - must be supplied.
     */

    if ( return_routine == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    stat = ( man_status )man_mgmt_request( MAN_C_GET,
                                           object_class ,
                                           object_instance ,
                                           iso_scope ,
                                           filter ,
                                           access_control ,
                                           synchronization ,
                                           attribute_identifier_list ,
                                           invoke_identifier ,
                                           return_routine ,
                                           &nil_object_id ,
                                           nil_avl ,
                                           nil_avl ,
                                           nil_avl ) ;
    return( stat ) ;
} /* end msi_get_attributes() */


man_status
msi_set_attributes(
                   object_class ,
                   object_instance ,
                   iso_scope ,
                   filter ,
                   access_control ,
                   synchronization ,
                   modification_list ,
                   invoke_identifier ,
                   return_routine
                  )
/*
 *
 * Function description:
 *
 *    This routine is the entry point for the management operation
 *    set.
 *
 * Arguments:
 *
 *    object_class       the address of the object class id for the target mo
 *    object_instance    the address of the instance name for the target mo
 *    iso_scope          ISO scope information
 *    filter             the address of the filter for management operation
 *    access_control     the address of the avl containing the access information
 *    synchronization    whether management operation is atomic or best effort
 *    modification_list  the address of an attribute id/value pair list or an attribute id list
 *    invoke_identifier  unique id assigned by the protocol engine
 *    return_routine     the address of the opaque data for locating the return routine for the request
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    replies successfully collected
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_INVALID_SCOPE              Invalid scope value
 *    MAN_C_MO_TIMEOUT                 RPC time out to MO
 *    MAN_C_MOLD_TIMEOUT               RPC time out to MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC time out to Protocol Engine Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

object_id *object_class ;
avl *object_instance ;
scope iso_scope ;
avl *filter ;
avl *access_control ;
int synchronization ;
avl *modification_list ;
int invoke_identifier ;
management_handle *return_routine ;
{
    man_status stat ;

    stat = ( man_status )man_mgmt_request( MAN_C_SET ,
                                           object_class ,
                                           object_instance ,
                                           iso_scope ,
                                           filter ,
                                           access_control ,
                                           synchronization ,
                                           modification_list ,
                                           invoke_identifier ,
                                           return_routine ,
                                           &nil_object_id ,
                                           nil_avl ,
                                           nil_avl ,
                                           nil_avl ) ;
    return( stat ) ;
} /* end msi_set_attributes() */


man_status
msi_create_instance(
                    object_class ,
                    object_instance ,
                    superior_instance ,
                    access_control ,
                    reference_instance ,
                    attribute_list ,
                    invoke_identifier ,
                    return_routine
                   )
/*
 *
 * Function description:
 *
 *    This routine is the entry point for the management operation
 *    create.
 *
 * Arguments:
 *
 *    object_class       the address of the object class id for the target mo
 *    object_instance    the address of the instance name for the target mo
 *    superior_instance  the address of the name of the superior instance for the create management operation
 *    access_control     the address of the avl containing the access information
 *    reference_instance the address of the name of reference instance for the create management operation
 *    attribute_list     the address of the an attribute id/value pair list or an attribute id list
 *    invoke_identifier  unique id assigned by the protocol engine
 *    return_routine     the address of the opaque data for locating the return routine for the request
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    replies successfully collected
 *    MAN_C_BAD_PARAMETER              Return routine argument was NULL
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_MO_TIMEOUT                 RPC time out to MO
 *    MAN_C_MOLD_TIMEOUT               RPC time out to MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC time out to Protocol Engine Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

object_id *object_class ;
avl *object_instance ;
avl *superior_instance ;
avl *access_control ;
avl *reference_instance ;
avl *attribute_list ;
int invoke_identifier ;
management_handle *return_routine ;
{
    man_status stat ;

    /*
     *  This is a confirmed management operation.  Check
     *  for return_routine - must be supplied.
     */

    if ( return_routine == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    stat = ( man_status )man_mgmt_request( MAN_C_CREATE ,
                                           object_class ,
                                           object_instance ,
                                           nil_scope ,
                                           nil_filter ,
                                           access_control ,
                                           nil_synchronization ,
                                           attribute_list ,
                                           invoke_identifier ,
                                           return_routine ,
                                           &nil_object_id ,
                                           nil_avl ,
                                           reference_instance ,
                                           superior_instance ) ;
    return( stat ) ;
} /* end msi_create_instance() */


man_status
msi_delete_instance(
                    object_class ,
                    object_instance ,
                    iso_scope ,
                    filter ,
                    access_control ,
                    synchronization ,
                    invoke_identifier ,
                    return_routine
                   )
/*
 *
 * Function description:
 *
 *    This routine is the entry point for the management operation
 *    delete.
 *
 * Arguments:
 *
 *    object_class       the address of the object class id for the target mo
 *    object_instance    the address of the instance name for the target mo
 *    iso_scope          ISO scope information
 *    filter             the address of the filter for management operation
 *    access_control     the address of the avl containing the access information
 *    synchronization    whether management operation is atomic or best effort
 *    invoke_identifier  unique id assigned by the protocol engine
 *    return_routine     the address of the opaque data for locating the return routine for the request
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    replies successfully collected
 *    MAN_C_BAD_PARAMETER              Return routine argument was NULL
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_INVALID_SCOPE              Invalid scope value
 *    MAN_C_MO_TIMEOUT                 RPC time out to MO
 *    MAN_C_MOLD_TIMEOUT               RPC time out to MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC time out to Protocol Engine Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

object_id *object_class ;
avl *object_instance ;
scope iso_scope ;
avl *filter ;
avl *access_control ;
int synchronization;
int invoke_identifier ;
management_handle *return_routine ;
{
    man_status stat ;

    /*
     *  This is a confirmed management operation.  Check
     *  for return_routine - must be supplied.
     */

    if ( return_routine == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    stat = ( man_status )man_mgmt_request( MAN_C_DELETE ,
                                           object_class ,
                                           object_instance ,
                                           iso_scope ,
                                           filter ,
                                           access_control ,
                                           synchronization ,
                                           nil_avl ,
                                           invoke_identifier ,
                                           return_routine ,
                                           &nil_object_id ,
                                           nil_avl ,
                                           nil_avl ,
                                           nil_avl ) ;
    return( stat ) ;
} /* end msi_delete_instance() */


man_status
msi_invoke_action(
                  object_class ,
                  object_instance ,
                  iso_scope ,
                  filter ,
                  access_control ,
                  synchronization ,
                  action_type ,
                  action_information ,
                  invoke_identifier ,
                  return_routine
                 )
/*
 *
 * Function description:
 *
 *    This routine is the entry point for the management operation
 *    action.
 *
 * Arguments:
 *
 *    object_class       the address of the object class id for the target mo
 *    object_instance    the address of the instance name for the target mo
 *    iso_scope          ISO scope information
 *    filter             the address of the filter for management operation
 *    access_control     the address of the avl containing the access information
 *    synchronization    whether management operation is atomic or best effort
 *    action_type        the address of the action type for the action mangement operation
 *    action_information the address of the action arguments for the action management operation
 *    invoke_identifier  unique id assigned by the protocol engine
 *    return_routine     the address of the opaque data for locating the return routine for the request
 *
 * Return value:
 *
 *    MAN_C_SUCCESS                    replies successfully collected
 *    MAN_C_INSUFFICIENT_RESOURCES     Resource failure in processing the operation
 *    MAN_C_INVALID_SCOPE              Invalid scope value
 *    MAN_C_MO_TIMEOUT                 RPC time out to MO
 *    MAN_C_MOLD_TIMEOUT               RPC time out to MOLD
 *    MAN_C_NO_MOLD                    No Managed Object Location Directory available
 *    MAN_C_NO_SUCH_CLASS              Target class not found
 *    MAN_C_PE_TIMEOUT                 RPC time out to Protocol Engine Reply Reciever
 *    MAN_C_PROCESSING_FAILURE         General failure in processing the operation
 *    MAN_C_SYNC_NOT_SUPPORTED         Synchronization value not supported
 *
 * Side effects:
 *
 */

object_id *object_class ;
avl *object_instance ;
scope iso_scope ;
avl *filter ;
avl *access_control ;
int synchronization ;
object_id *action_type ;
avl *action_information ;
int invoke_identifier ;
management_handle *return_routine ;
{
    man_status stat ;

    stat = ( man_status )man_mgmt_request( MAN_C_ACTION ,
                                           object_class ,
                                           object_instance ,
                                           iso_scope ,
                                           filter ,
                                           access_control ,
                                           synchronization ,
                                           nil_avl ,
                                           invoke_identifier ,
                                           return_routine ,
                                           action_type ,
                                           action_information ,
                                           nil_avl ,
                                           nil_avl ) ;
    return( stat ) ;
} /* end msi_invoke_action() */

/* end agent_interface.c */
