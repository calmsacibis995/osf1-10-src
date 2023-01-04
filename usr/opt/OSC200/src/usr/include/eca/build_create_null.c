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
static char *rcsid = "@(#)$RCSfile: build_create_null.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:20:43 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	CREATE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the CREATE function.
**
**	Note: This MOM does not support a CREATE function!
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
**      create_instance
**
**	This routine calls returns the directive_not_supported error reply.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	   - Management handle.
**	object_class       - AVL containing OID of class to create.
**	object_instance    - AVL containing instance name to create. If supplied,
**			     then superior instance is not supplied. 
**	superior_instance  - AVL containing superior instance name. If supplied
**			     then object instance is not supplied.
**	access_control     - AVL containing access control information.
**	reference_instance - AVL containing template instance for setting attributes.
**	attribute_list     - AVL of attribute value pairs for the management operation.
**	invoke_id          - Identifier associated with the request.
**	handle        	   - Address of information for returning data to the PE.
**	called_by_invoke_action - Flag indicating whether caller was create or action.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  CREATE RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_ACCESS_DENIED
**	MAN_C_NO_SUCH_ACTION
**	MAN_C_PROCESSING_FAILURE
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
**	MAN_C_INVALID_ATTRIBUTE_VALUE
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INVALID_USE_OF_WILDCARD
**	MAN_C_REQUIRED_ARGUMENT_OMITTED
**	MAN_C_INSUFFICIENT_RESOURCES
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status create_instance( mom_handle,
			    object_class, 
	    		    object_instance,
			    superior_instance,
			    access_control,
			    reference_instance,
			    attribute_list,
			    invoke_id,
			    return_routine,
			    action_type,
			    called_by_invoke_action )

man_binding_handle	mom_handle;
object_id	    	*object_class;
avl		    	*object_instance;
avl		   	*superior_instance;
avl		   	*access_control;
avl		    	*reference_instance;
avl		   	*attribute_list;
int			invoke_id;
management_handle	*return_routine;
object_id	    	*action_type;
int			called_by_invoke_action;

{
    man_binding_handle pe_handle;
    man_status	    status;

#ifdef MOMGENDEBUG
    printf("In the CREATE routine\n");
    printf("Instance AVL:\n");
    dbg_print_avl (object_instance);
    printf("Argument List:\n");
    dbg_print_avl (attribute_list);
#endif

    if (((man_status) _reply_required( return_routine )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( return_routine , &pe_handle );
        if ERROR_CONDITION(status)
          return(MAN_C_INSUFFICIENT_RESOURCES);

        if (called_by_invoke_action)
	   status = (man_status) 
		    _send_error( pe_handle,
				moss_send_action_reply,
				invoke_id,		/* invoke_identifier */
			   	MAN_C_DIRECTIVE_NOT_SUPPORTED, /* reply */
			   	object_class,		/* object_class */
			   	object_instance,	/* object_instance */
			   	NULL,			/* [object_uid] */
			   	NULL, 			/* [attribute_list] */
			   	action_type);		/* action_type */
        else    
	    status = (man_status)
		    _send_error( pe_handle,
				moss_send_create_reply,
				invoke_id,		/* invoke_identifier */
			   	MAN_C_DIRECTIVE_NOT_SUPPORTED, /* reply */
			   	object_class,		/* object_class */
			   	object_instance,	/* object_instance */
			   	NULL,			/* [object_uid] */
			   	NULL, 			/* [attribute_list] */
			   	NULL);			/* NO action_type */

	status = moss_free_pe_handle( pe_handle );
	}
    
    return MAN_C_SUCCESS;
}
                          
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_create_instance
**
**	This routine calls create_instance to return back a directive not
**	supported error.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	   - Management handle.
**	object_class       - AVL containing OID of class to create.
**	object_instance    - AVL containing instance name to create. If supplied,
**			     then superior instance is not supplied. 
**	superior_instance  - AVL containing superior instance name. If supplied
**			     then object instance is not supplied.
**	access_control     - AVL containing access control information.
**	reference_instance - AVL containing template instance for setting attributes.
**	attribute_list     - AVL of attribute value pairs for the management operation.
**	invoke_id          - Identifier associated with the request.
**	handle        	   - Address of information for returning data to the PE.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  CREATE RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_ACCESS_DENIED
**	MAN_C_CLASS_INSTANCE_CONFLICT
**	MAN_C_COMPLEXITY_LIMITATION
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_DUPLICATE_ARGUMENT
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_INVALID_ARGUMENT_VALUE
**	MAN_C_INVALID_FILTER
**	MAN_C_INVALID_USE_OF_WILDCARD
**	MAN_C_NO_SUCH_ACTION
**	MAN_C_NO_SUCH_ARGUMENT
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_PROCESSING_FAILURE
**	MAN_C_REQUIRED_ARGUMENT_OMITTED
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status moi_create_instance ( mom_handle,
				 object_class, 
	    			 object_instance,
				 superior_instance,
				 access_control,
				 reference_instance,
				 attribute_list,
				 invoke_id,
				 return_routine )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
avl		    *superior_instance;
avl		    *access_control;
avl		    *reference_instance;
avl		    *attribute_list;
int	            invoke_id;
management_handle   *return_routine;

{
man_status status;

status = create_instance( mom_handle,
			object_class, 
			object_instance, 
			superior_instance,
			access_control,
			reference_instance,
			attribute_list,
			invoke_id,
			return_routine,
			NULL,
			CREATE);
return status;
}
