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
static char *rcsid = "@(#)$RCSfile: build_action_null.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:20:20 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	ACTION.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the ACTION function.
**
**	Note: This MOM does not support a ACTION function!
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
**--
*/

#include "moss.h"
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_invoke_action
**
**	This routine is the MOI entry point for all action directives into
**	the [[mom_name]] MOM.  DNA CMIP create directives use this entry
**	point.  This routine determines which class specific action routine
**	to call. 
**
**	This version of moi_invoke_action returns directive_not_supported
**	as a reply.
**
**  FORMAL PARAMETERS:
**
**	mom_handle        - Man binding handle.
**	object_class      - OID of class to create.
**	object_instance   - Instance name to create 
**	iso_scope         - Scoping value. Not currently used.
**	filter            - AVL determining which instance(s) the operation should be
**	    		    performed on.
**      access_control    - Access control information.
**	synchronization   - Not currently used.
**	action_type       - OID of operation to be performed.	
**	action_information- AVL containing class-specific arguments.
**	invoke_id         - Invocation ID.
**	return_routine    - Handle for reply processing.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  ACTION RETURN REPLIES:
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
man_status moi_invoke_action(   mom_handle,
				object_class, 
	    			object_instance,
				iso_scope,
				filter,
				access_control,
				synchronization,
				action_type,
				action_information,
				invoke_id,
				return_routine)

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
object_id	    *action_type;
avl		    *action_information;
int		    invoke_id;
management_handle   *return_routine;

{
    man_binding_handle pe_handle;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("In the ACTION routine!\n");
    printf("Instance AVL:\n");
    dbg_print_avl (object_instance);
    printf("Argument List:\n");
    dbg_print_avl (action_information);
#endif

    if (((man_status) _reply_required( return_routine )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( return_routine , &pe_handle );
        if ERROR_CONDITION(status)
          return(MAN_C_INSUFFICIENT_RESOURCES);

        status = (man_status) 
		  _send_error(pe_handle,
			    moss_send_action_reply,
			    invoke_id,
			    MAN_C_DIRECTIVE_NOT_SUPPORTED,
			    object_class,
			    object_instance, 
			    NULL, NULL, action_type);

	status = moss_free_pe_handle( pe_handle );
	}
    
    return MAN_C_SUCCESS;
}
