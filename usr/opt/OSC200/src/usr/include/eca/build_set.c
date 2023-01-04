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
static char *rcsid = "@(#)$RCSfile: build_set.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:34 $";
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
**      moi_set_attributes
**
**	This routine calls the moi_directive routine with the class specific
**	perform_set and moss_send_set_reply routines as arguments.
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
    /*-insert-code-external-routines-*/

    object_id		*action=NULL;
    man_status		status;

#ifdef MOMGENDEBUG
    printf("\n***********  Calling MOM via moi_set_attributes() ***********\n");
    printf(  "-------------------------------------------------------------\n");
    dbg_print_moi_set_attributes (
                        object_class ,
                        object_instance ,
                        iso_scope ,
                        filter ,
                        access_control ,
                        synchronization ,
                        modification_list,
                        invoke_id ,
                        handle
                        ) ;
#endif

    status = _get_class_code( object_class, &class_code );
    if (status == MAN_C_SUCCESS)
        switch( class_code )
            {
    	    /*-insert-code-select-routine-*/
	    }
    else 
        return (_error_exit("\n*** Error in set, class not supported\n", status));

    /*
     * This directive is handled by the generic "directive" routine,
     * which will apply the filter and scoping rules to all instances
     * of the proper class, and dispatch to the directive-specific
     * routine for matching entities.
     */
    status = moi_directive(mom_handle,
			   object_class,
			   object_instance,
			   iso_scope,
			   filter,
			   access_control,
			   NULL,		/* Directive OID */
			   modification_list,	/* Directive AVL */
			   perform_set,		/* Directive routine */
			   moss_send_set_reply,	/* Reply routine */
			   invoke_id,
			   handle,
			   action);
   
    return status;
}
