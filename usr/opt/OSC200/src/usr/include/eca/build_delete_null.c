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
static char *rcsid = "@(#)$RCSfile: build_delete_null.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:20:51 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	DELETE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the DELETE function.
**
**	Note: This MOM does not support a DELETE function!
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
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      delete_instance
**
**	This routine performs all of the verification and validation of
**	the instance to be delete. This routine can be called from both 
**	moi_invoke_action (for DNA CMIP) and moi_delete_instance.
**
**  FORMAL PARAMETERS:
**
**      mom_handle         - Management handle.
**	object_class       - OID of class to create.
**	object_instance    - AVL containing instance name to create.
**	iso_scope          - Matching information for instance qualification (not currently used).
**	filter             - AVL determining which instance(s) the operation 
**	       		     should be performed on.
**	access_control     - AVL containing access control information.
**	synchronization    - Request to be best effort or atomic (not currently used).
**	invoke_id          - Identifier associated with the request.
**	return_routine	   - Address of information for returning data to the PE.
**      action_type        - Object ID of action to be performed (NULL if delete)
**      action_information - AVL containing action specific info (NULL if delete)
**      type 		   - Flag indicating delete or action.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  CREATE/ACTION RETURN REPLIES:
**
**	 (common)
**      MAN_C_SUCCESS
**	MAN_C_ACCESS_DENIED
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_INVALID_USE_OF_WILDCARD
**      MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_PROCESSING_FAILURE
**
**	 (action specific)
**	MAN_C_DUPLICATE_ARGUMENT
**	MAN_C_INVALID_ARGUMENT_VALUE
**	MAN_C_NO_SUCH_ACTION
**	MAN_C_NO_SUCH_ARGUMENT
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_REQUIRED_ARGUMENT_OMITTED
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status delete_instance( mom_handle,
			    object_class, 
	    		    object_instance,
			    iso_scope,
			    filter,
			    access_control,
			    synchronization,
			    invoke_id,
			    return_routine,
			    action_type,
			    action_information,
			    type )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
int	            invoke_id;
management_handle   *return_routine;
object_id	    *action_type;
avl		    *action_information;
int		    type;

{
    man_binding_handle pe_handle;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("In the DELETE routine!\n");
    printf("Instance AVL:\n");
    dbg_print_avl (object_instance);
#endif

    if (((man_status) _reply_required( return_routine )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( return_routine , &pe_handle );
        if ERROR_CONDITION(status)
          return(MAN_C_INSUFFICIENT_RESOURCES);

        status = (man_status) 
		 _send_error(pe_handle,
			    moss_send_delete_reply,
			    invoke_id,
			    MAN_C_DIRECTIVE_NOT_SUPPORTED,
			    object_class,
			    object_instance, 
			    NULL, NULL, NULL );

	status = moss_free_pe_handle( pe_handle );
	}
    
    return MAN_C_SUCCESS;
}


/*
**  FUNCTIONAL DESCRIPTION:
**
**      moi_delete_instance
**
**	This routine returns the directive_not_supported error reply.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	   - Management binding handle.
**	object_class       - OID of class to create.
**	object_instance    - AVL of instance name to create 
**	iso_scope          - Scoping value. Not currently used.
**	filter             - AVL determining which instance(s) the operation 
**			     should be performed on.
**	access_control     - AVL containing access control information.
**	synchronization    - Not currently used.
**	invoke_id          - Invocation ID.
**	handle        	   - Handle for internal use.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  DELETE RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_ACCESS_DENIED
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_INVALID_USE_OF_WILDCARD
**      MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status moi_delete_instance( mom_handle,
				object_class, 
	    			object_instance,
				iso_scope,
				filter,
				access_control,
				synchronization,
				invoke_id,
				return_routine )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
int	            invoke_id;
management_handle   *return_routine;

{
    man_status		status;

    status = delete_instance(mom_handle,
			     object_class,
			     object_instance,
			     iso_scope,
			     filter,
			     access_control,
			     synchronization,
			     invoke_id,
			     return_routine,
			     (object_id *)NULL,
			     (avl *)NULL,	/* Action information */
 			     (int)NULL);
   
    return status;
}
