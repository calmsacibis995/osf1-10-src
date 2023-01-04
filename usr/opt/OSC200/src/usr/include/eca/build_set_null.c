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
static char *rcsid = "@(#)$RCSfile: build_set_null.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:46 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the SET function.
**
**	Note: This MOM does not support a SET function!
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
**  TEMPLATE HISTORY:
**
**--
*/

#include "moss.h"
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_set_attributes
**
**	This routine returns the directive_not_supported error reply.
**
**  FORMAL PARAMETERS:
**
**	mom_handle	- Unused binding handle.
**	object_class	- OID of the class to manipulate.
**	object_instance	- AVL containing instance to manipulate.
**	iso_scope	- Matching information for instance qualification. (Not currently used).
**	filter		- AVL determining which instance(s) the operation should be performed on.
**	access_control	- AVL containing access control information.
**	synchronization	- Request to be best effort or atomic (not currently used).
**	modification_list - AVL listing the attribute(s) which should be set.
**	invoke_id	- Identifier associated with the request.
**	handle		- Address of information for returning data to the PE. 
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  SET RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**	MAN_C_ACCESS_DENIED
**	MAN_C_SET_LIST_ERROR
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_INVALID_USE_OF_WILDCARD
**	MAN_C_CONSTRAINT_VIOLATION
**	MAN_C_INSUFFICIENT_RESOURCES
**	
**--
*/
man_status moi_set_attributes(  mom_handle,
				object_class, 
	    			object_instance,
				iso_scope,
				filter,
				access_control,
				synchronization,
				modification_list,
				invoke_id,
				handle )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
avl		    *modification_list;
int		    invoke_id;
management_handle   *handle;

{
    man_binding_handle pe_handle;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("In the SET routine!\n");
    printf("Instance AVL:\n");
    dbg_print_avl (object_instance);
    printf("Attribute List:\n");
    dbg_print_avl (modification_list);
#endif

    if (((man_status) _reply_required( handle )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( handle, &pe_handle );
        if ERROR_CONDITION(status)
          return(MAN_C_INSUFFICIENT_RESOURCES);

        status = (man_status)
	    	 _send_error(pe_handle,
			    moss_send_set_reply,
			    invoke_id,
			    MAN_C_DIRECTIVE_NOT_SUPPORTED,
			    object_class,
			    object_instance, 
			    NULL, NULL, NULL );

	status = moss_free_pe_handle( pe_handle );
	}
    
    return MAN_C_SUCCESS;
}
