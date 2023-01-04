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
static char *rcsid = "@(#)$RCSfile: build_getnext.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:21:47 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**
**	It provides the entry point for the get-next function which
**	is handled through the moi_invoke_action entry point.
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
#include "extern_nil.h"




/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      send_reply
**
**	This routine calls the action reply routine for getnext.
**
**  FORMAL PARAMETERS:
**
**	pe_handle	- Handle used to bind to PE.
**	invoke_id	- Invokation ID.
**	reply		- reply to send back.
**	object_class    - OID of class to create.
**	object_instance - Instance name to create 
**	object_uid	- UID of instance to send back.
**	operation_time	- Time of created instance.
**	action_type	- Object ID of action.
**	action_response_type_oid - Object ID of response.
**	attribute_list  - AVL used for the management operation.
**	more_replies	- Flag indicating more replies or not.
**
**  RETURN VALUES:
**
**      any status from the reply routines
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status send_reply( pe_handle,
			      invoke_identifier, 
                       	      reply,
		              object_class, 
	    	              object_instance,
		       	      object_uid,
		       	      operation_time,
		      	      action_type,
		      	      action_response_type_oid,
		      	      return_avl,
		      	      more_replies
			      )

man_binding_handle pe_handle;
int	   invoke_identifier;
reply_type   reply;
object_id    *object_class;
avl	   *object_instance;
uid	   *object_uid;
mo_time	   operation_time;
object_id    *action_type;
object_id    *action_response_type_oid;
avl	   *return_avl;
int	   more_replies;

{
#ifndef VMS
                /* check for null arguments...need to use special nil values for rpc */

if (object_uid == NULL) object_uid = &nil_uid;
if (return_avl == NULL) return_avl = nil_avl;
if (action_response_type_oid == NULL) action_response_type_oid = &nil_object_id;
if (action_type == NULL) action_type = &nil_object_id;
#endif


    return( moss_send_action_reply( pe_handle,
				     invoke_identifier,		
				     reply, 		
				     object_class,	
				     object_instance,	
				     object_uid,	
				     operation_time,	
				     action_type,	
				     action_response_type_oid, 
				     return_avl,	
				     more_replies));	
} /* End of send_reply() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_invoke_action
**
**	This routine is the MOI entry point for all action directives into
**	the [[mom_name]] MOM.  There is only one operation supported here:
**	the SNMP GetNext.
**
**    Only one argument is expected to be supplied for the SNMP GetNext:
**    an optional attribute.
**
**    The way GetNext works for the MOM is as follows.
**
**    1.  If there is no attribute specified, the first attribute from
**        the class is used.
**    2.  If there is no instance specified (an empty avl for the
**        instance name), the first instance of the class is used.
**        In addition, the MOM returns the value of the attribute specified.
**    3.  If there is an instance and attribute specified, then the
**        value of the next lexicographic attribute is returned.  For
**        non-table classes, this means that 
**            a - the attribute following the one requested is returned;
**            b - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.
**        For table classes, this means that
**            a - the requested attribute for the next entry in the
**                the table is returned;
**            b - if the instance specified the last entry in the table,
**                the next attribute for the first instance is returned;
**            c - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.
**
**  FORMAL PARAMETERS:
**
**	mom_handle        - Man binding handle.
**	object_class      - OID of class.
**	object_instance   - Instance name.
**	iso_scope         - Scoping value. Not used in SNMP.  Must be NULL.
**	filter            - AVL determining which instance(s) the operation should be
**	    		    performed on.  Not used in SNMP.  Must be NULL.
**      access_control    - Access control information.
**	synchronization   - Not currently used.
**	action_type       - OID of operation to be performed.  Must be getnext.
**	action_information- AVL containing class-specific arguments.
**	invoke_id         - Invocation ID.
**	return_routine    - Handle for reply processing.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
**	MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
moi_invoke_action(
		   mom_handle,
		   object_class,
		   object_instance,
		   iso_scope,
		   filter,
		   access_control,
		   synchronization,
		   action_type,
		   action_information,
		   invoke_identifier,
		   return_routine
                 )

man_binding_handle  mom_handle;
object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
avl		    *filter;
avl		    *access_control;
int		    synchronization;
object_id	    *action_type;
avl		    *action_information;
int	            invoke_identifier;
management_handle   *return_routine;

{

/*-insert-code-external-routines-*/

man_binding_handle  pe_handle;
mo_time		    *op_time = NULL;
man_status	    status;
uid		    *object_uid = NULL;
avl		    *reply_avl = NULL;
reply_type	    reply_status = (reply_type)NULL;
struct object_id    *response_type = NULL;
unsigned int        modifier;
#ifdef MOMGENDEBUG
    printf("\n***********  Calling MOM via moi_invoke_action() ***********\n");
    printf( "-------------------------------------------------------------\n");

    dbg_print_moi_invoke_action(
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
			) ;
#endif

    /*
     *  Reset all incoming avls
     */

    status = moss_avl_reset( object_instance ) ;
    if ERROR_CONDITION(status) return status;
    status = moss_avl_reset( filter ) ;
    if ERROR_CONDITION(status) return status;
    status = moss_avl_reset( access_control ) ;
    if ERROR_CONDITION(status) return status;
    status = moss_avl_reset( action_information ) ;
    if ERROR_CONDITION(status) return status;

    status = moss_get_time (&op_time);	    /* get operation timestamp */
    if ERROR_CONDITION (status) return MAN_C_INSUFFICIENT_RESOURCES;

    status = moss_alloc_pe_handle( return_routine, &pe_handle ); /* bind to pe */
    if ERROR_CONDITION (status) {
	moss_free_time (op_time);
	return MAN_C_INSUFFICIENT_RESOURCES;
    }

    /*
     *  Initialize AVL for reply
     */

    status = moss_avl_init( &reply_avl ) ;
    if ( status != MAN_C_SUCCESS ) {
        moss_free_pe_handle( pe_handle ) ;
        moss_free_time( op_time ) ;
        return( MAN_C_INSUFFICIENT_RESOURCES ) ;
    }

#ifndef VMS
/*
    make sure requestor has access to perform getnext
 */

    if (authenticate_client(MAN_C_GET,access_control) == FALSE) {
	status = send_reply( pe_handle,
		    invoke_identifier,
		    MAN_C_ACCESS_DENIED,
		    object_class,
		    object_instance,
		    object_uid,
		    *op_time,
		    action_type,
		    &nil_object_id,
		    reply_avl,
		    FALSE);
        moss_avl_free(&reply_avl, TRUE);
        moss_free_time( op_time );
        moss_free_pe_handle( pe_handle );
        if ERROR_CONDITION( status )
            return MAN_C_PE_TIMEOUT;
        return MAN_C_SUCCESS;
    }
#endif

    /*
     *  If a filter is supplied then reject the request -- filter specification
     *  is inconsistent with GetNext action.
     */

    status = moss_avl_point( filter, NULL, NULL, NULL, NULL, NULL ) ;
    if ( status != MAN_C_NO_ELEMENT ) 
    {
	status = moss_avl_reset( filter ) ;
	if ERROR_CONDITION(status) {
            moss_free_time( op_time ) ;
            moss_avl_free(&reply_avl, TRUE);
            moss_free_pe_handle( pe_handle );
            return status;
        }
	modifier = MAN_C_FILTER_TOO_COMPLEX ;
	status = moss_avl_copy( reply_avl, filter, FALSE, NULL, &modifier, NULL ) ;
	if ERROR_CONDITION(status) {
            moss_free_time( op_time ) ;
            moss_avl_free(&reply_avl, TRUE);
            moss_free_pe_handle( pe_handle );
            return status;
        }

	status = send_reply( pe_handle,
		    invoke_identifier,
		    MAN_C_COMPLEXITY_LIMITATION,
		    object_class,
		    object_instance,
		    object_uid,
		    *op_time,
		    action_type,
		    &nil_object_id,
		    reply_avl,
		    FALSE);

	moss_free_time( op_time );
        moss_avl_free(&reply_avl, TRUE);
        moss_free_pe_handle( pe_handle );
        if ERROR_CONDITION( status )
            return MAN_C_PE_TIMEOUT;
	return MAN_C_SUCCESS;
    }

    /*
     *  A non-zero iso_scope value is inconsistent with GetNext.  Reject
     *  request if it is supplied.
     */

    else if ( iso_scope != 0 )
    {
	octet_string octet ;
	octet.data_type = ASN1_C_INTEGER ;
	octet.length = sizeof( iso_scope ) ;
	octet.string = (char *)&iso_scope ;
	status = moss_avl_add( reply_avl, NULL, MAN_C_SCOPE_TOO_COMPLEX,
			      ASN1_C_INTEGER, &octet ) ;
	if ERROR_CONDITION(status) {
            moss_free_time ( op_time );
            moss_avl_free(&reply_avl, TRUE);
            moss_free_pe_handle( pe_handle );
            return status;
        }
	status = send_reply( pe_handle,
		    invoke_identifier,
		    MAN_C_COMPLEXITY_LIMITATION,
		    object_class,
		    object_instance,
		    object_uid,
		    *op_time,
		    action_type,
		    &nil_object_id,
		    reply_avl,
		    FALSE);

	moss_free_time( op_time );
        moss_avl_free(&reply_avl, TRUE);
        moss_free_pe_handle( pe_handle );
        if ERROR_CONDITION( status )
            return MAN_C_PE_TIMEOUT;
	return MAN_C_SUCCESS;
    }

    /*
     *  get the class and call the object specific routine to determine
     *	the instance and attribute needed to fulfill the request
     */

    status = _get_class_code( object_class, &class_code );
    if (ERROR_CONDITION(status)) {
        moss_free_time ( op_time );
        moss_avl_free(&reply_avl, TRUE);
        moss_free_pe_handle( pe_handle );
        return status;
    }

    switch( class_code )
	{
	/*-insert-code-select-routine-*/
	}

    status = (perform_getnext) (
				object_class,
				action_information,
				&object_instance,
				iso_scope,
				&reply_avl,
				&reply_status,
				&response_type
				);

    if (status != MAN_C_NO_SUCH_ATTRIBUTE_ID) {	/* don't reply if end of class */
	if (reply_status == (reply_type)NULL)
	    setup_reply( status, &reply_status, &response_type, reply_avl,
		    class_code, ACTION, action_type);

#ifdef MOMGENDEBUG
                    printf("\n================>  Reply from MOM <===============\n\n");
                    printf("----------- Object instance -----------\n");
                    dbg_print_avl( object_instance );
                    printf("\n-------------- Reply AVL --------------\n");
                    dbg_print_avl( reply_avl );
                    printf(  "================>  End of Reply <=================\n\n");
#endif /* MOMGENDEBUG */

	status = send_reply( pe_handle,
		    invoke_identifier,
		    reply_status,
		    object_class,
		    object_instance,
		    object_uid,
		    *op_time,
		    action_type,
		    response_type,
		    reply_avl,
		    FALSE);

	if ( status != MAN_C_SUCCESS )
	    status = MAN_C_PE_TIMEOUT;
    }

#ifdef MOMGENDEBUG
    if (status == MAN_C_NO_SUCH_ATTRIBUTE_ID)
    {
         printf("\n================>  Reply from MOM <===============\n\n");
         printf("status = MAN_C_NO_SUCH_ATTRIBUTE_ID\n\n");
         printf(  "================>  End of Reply <=================\n\n");
    }
#endif /* MOMGENDEBUG */

    moss_free_time( op_time ) ;
    moss_avl_free(&reply_avl, TRUE);
    moss_free_pe_handle( pe_handle );

    return( status ) ;
}  /* End of moi_invoke_action() */

/* End of getnext.c */
