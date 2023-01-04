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
static char *rcsid = "@(#)$RCSfile: build_directive.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/10/01 14:24:08 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	DIRECTIVE.C
**	
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**
**	It provides the GENERIC DIRECTIVE function used by GET, 
**	SET, DELETE and ACTION.
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

#ifndef VMS
#include "extern_nil.h"
#endif


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      send_reply
**
**	This routine calls the appropriate reply routine (action, set, show or
**	delete).
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
**	reply_routine   - Address of reply routine
**	called_by_invoke_action - flag indicating which reply routine to use.
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
		      	      more_replies,
		      	      reply_routine,
		      	      type )

man_binding_handle	pe_handle;
int			invoke_identifier;
reply_type		reply;
object_id		*object_class;
avl			*object_instance;
uid			*object_uid;
mo_time			*operation_time;
object_id		*action_type;
object_id		*action_response_type_oid;
avl			*return_avl;
int			more_replies;
man_status		(*reply_routine)();
int			type;

{
#ifndef VMS
    /* check for null arguments...replace with special nils for rpc */
    if (object_uid == NULL) object_uid = &nil_uid;
    if (return_avl == NULL) return_avl = nil_avl;
#endif

if (type == ACTION)
    return( moss_send_action_reply( pe_handle,
				     invoke_identifier,		
				     reply, 		
				     object_class,	
				     object_instance,	
				     object_uid,	
				     *operation_time,	
				     action_type,	
				     action_response_type_oid, 
				     return_avl,	
				     more_replies));	
else 
    return ((reply_routine)(pe_handle,
			     invoke_identifier,
			     reply,
			     object_class,
			     object_instance,
			     object_uid,
			     *operation_time,
			     return_avl,
			     more_replies));
} /* End of send_reply() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      mom_apply_filter
**
**	This routine calls the class apply filter routine.
**
**	NOTE: This routines is currently not supported by the MOM Generator.
**
**  FORMAL PARAMETERS:
**
**	object_instance	   - AVL containing instance to retrieve attributes on.
**	avl_in		   - AVL containing list of attributes to retrieve values for.
**	avl_out		   - AVL containing list of values for those attributes.
**	specific_instance  - Pointer to specific instance.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status mom_apply_filter( object_instance,
				    avl_in,
				    avl_out,
				    specific_instance )

avl *object_instance;
avl *avl_in;
avl *avl_out;
char *specific_instance;

{
    return MAN_C_SUCCESS;
} /* End of mom_apply_filter() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_directive
**
**	This routine performs common instance verfication and argument 
**	validation for the MOI set, get, action, and delete routines. 
**
**  FORMAL PARAMETERS:
**
**	mom_handle	   - Management handle.
**	object_class       - AVL containing OID of class to create.
**	object_instance    - AVL containing instance name to create. 
**	iso_scope	   - Not currently used.
**      filter		   - AVL containing filter information.
**	access_control     - Access control information.
**	directive_oid	   - Object ID containing which directive to perform.
**	directive_avl      - AVL containing attribute information.
**	directive_routine  - Address of appropriate perform routine.
**	reply_routine	   - Address of appropriate moss reply routine.
**	invoke_identifier  - Invocation ID.
**	handle        	   - Handle for internal use.
**	action_type	   - Object ID of action type.
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_PE_TIMEOUT
**
**  COMMON RETURN REPLIES:
**
**      MAN_C_SUCCESS
**	MAN_C_ACCESS_DENIED
**	MAN_C_DIRECTIVE_NOT_SUPPORTED
**	MAN_C_DUPLICATE_ARGUMENT
**	MAN_C_INSUFFICIENT_RESOURCES
**	MAN_C_INVALID_ARGUMENT_VALUE
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
man_status moi_directive( mom_handle,
			  object_class,
			  object_instance,
			  iso_scope,
			  filter,
			  access_control,
			  directive_oid,
			  directive_avl,
			  directive_routine,
			  reply_routine,
			  invoke_identifier,
			  handle,
			  action_type )

man_binding_handle mom_handle;
object_id	*object_class;
avl		*object_instance;
scope		iso_scope;
avl		*filter;
avl		*access_control;
object_id	*directive_oid;
avl		*directive_avl;
man_status	(*directive_routine)();
man_status	(*reply_routine)();
int	        invoke_identifier;
management_handle  *handle;
object_id	*action_type;

{
    man_binding_handle pe_handle;
    struct object_id *action_response_type = NULL;
    man_status	    send_status;
    man_status	    status;
    comparison	    *comparison_table[4];
    int		    more_instances = FALSE;
    int		    instance_found = FALSE;
    GET_CONTEXT_STRUCT *get_context = NULL;
    void	    *specific_instance;
    avl		    *specific_object_instance = NULL;
    uid		    *object_uid = NULL;
    avl		    *return_avl = NULL;
    reply_type	    reply_status = (reply_type)NULL;
    mo_time	    *operation_time = NULL;
    int		    class_code = (int)NULL;
    int		    type;

#ifndef VMS
    int		    access_check = 0;
#endif

    if (reply_routine == moss_send_action_reply) 
       type = ACTION;
    else if (reply_routine == moss_send_set_reply) 
       type = SET;
    else if (reply_routine == moss_send_get_reply) 
       type = GET;
    else if (reply_routine == moss_send_delete_reply) 
        type = DELETE;

    /*
     * Allocate a timestamp.
     */
    status = moss_get_time(&operation_time);
    if ERROR_CONDITION( status ) 
        return MAN_C_INSUFFICIENT_RESOURCES;

    /*
     * Bind once to the Reply Receiver.
     */ 
    status = moss_alloc_pe_handle( handle, &pe_handle );
    if ERROR_CONDITION(status)
	{
        moss_free_time( operation_time );
      	return MAN_C_INSUFFICIENT_RESOURCES;
	}

#ifndef VMS
    /*
       verify access for requested operation...currently get and set are
       the only operations supported for UNIX/SNMP (getnext is handled
       in the getnext.c module)
     */

    switch (type) {
       case GET:

	if (authenticate_client(MAN_C_GET,access_control) == FALSE)
	    access_check = 1;
	break;

       case SET:

	if (authenticate_client(MAN_C_SET,access_control) == FALSE)
	    access_check = 1;
	break;

    }

    if (access_check) {

	reply_status = (reply_type) MAN_C_ACCESS_DENIED;
	return_avl = NULL;
        send_status= send_reply( 
		   pe_handle,		  
		   invoke_identifier,	          
		   (reply_type) reply_status, 	  
		   object_class,	          
		   object_instance,         
		   NULL,		          
		   operation_time,         
		   action_type,	          
		   NULL, 		          
		   return_avl,		          
		   FALSE,		          
		   reply_routine,
		   type);
	moss_avl_free(&return_avl, FALSE);
        moss_free_time( operation_time );
        moss_free_pe_handle( pe_handle );
        if ERROR_CONDITION( send_status )
	    return MAN_C_PE_TIMEOUT;	
        return MAN_C_SUCCESS;
    }
#endif

    /*
     * Validate the filter argument and construct the comparison table 
     */
    status = moss_validate_filter(filter, comparison_table);
    if ERROR_CONDITION( status )
        { 
#ifdef OSI_CMIP_OID
	unsigned int modifier = (int) MAN_C_FILTER_TOO_COMPLEX;

	status = moss_avl_reset( filter );
    	if ERROR_CONDITION( status )
            return MAN_C_INSUFFICIENT_RESOURCES;
	
        status = moss_avl_init( &return_avl );
    	if ERROR_CONDITION( status )
            return MAN_C_INSUFFICIENT_RESOURCES;

	status = moss_avl_copy( return_avl, 
				filter, 
				FALSE, 
				NULL,
				&modifier,
				NULL);
    	if ERROR_CONDITION( status )
            return MAN_C_INSUFFICIENT_RESOURCES;

	reply_status = MAN_C_COMPLEXITY_LIMITATION;
#endif /* OSI_CMIP_OID */	

#ifdef DNA_CMIP_OID 
	reply_status = (reply_type) MAN_C_PROCESSING_FAILURE;
	return_avl = NULL;
#endif /* DNA_CMIP_OID */

        send_status= send_reply( 
		   pe_handle,		  
		   invoke_identifier,	          
		   (reply_type) reply_status, 	  
		   object_class,	          
		   object_instance,         
		   NULL,		          
		   operation_time,         
		   action_type,	          
		   NULL, 		          
		   return_avl,		          
		   FALSE,		          
		   reply_routine,
		   type);
	moss_avl_free(&return_avl, FALSE);
        moss_free_time( operation_time );
        moss_free_pe_handle( pe_handle );
        if ERROR_CONDITION( send_status )
	    return MAN_C_PE_TIMEOUT;	
        return MAN_C_SUCCESS;
	}

    /*
     * Determine the class on which the operation is to be performed.
     */
    status = _get_class_code(object_class, &class_code);
    if ERROR_CONDITION(status)
        {
	moss_free_time( operation_time );
	moss_avl_free(&return_avl, TRUE);
        moss_free_pe_handle( pe_handle );
        return (_error_exit("\n*** Error in moi_directive, class not supported\n", status));
        }

    /*
     * Using the object_class, object_instance, and iso_scope arguments,
     * find possible instances for applying the directive.
     */
#ifndef SNMP
    while 
#else
    if
#endif
          (MAN_C_SUCCESS == (status = get_instance( object_class,
						    object_instance,
						    iso_scope,
						    &specific_instance,
						    &specific_object_instance,
						    &object_uid,
						    &more_instances,
						    &get_context,
						    TRUE,
						    class_code)))
    {
	/*
	 * There is a candidate instance.  Check access to the instance.
	 */
	 status = check_access_instance( class_code,
					 specific_instance,
					 access_control );
         if ERROR_CONDITION( status ) 
	    {
	    /*
	     * The access check failed.  See if we should return here,
	     * or continue to check more candidates.  
	     * 
	     * If the iso_scope is a non-zero value, the access check is 
	     * used to refine the set of possible candidates.
	     */
            if (iso_scope == 0)
		{
       	   	send_status = send_reply( 
			    pe_handle,           
		       	    invoke_identifier,   
		            (reply_type) MAN_C_ACCESS_DENIED,  
		            object_class,	    
		            object_instance,     
		            NULL,		    
			    operation_time,     
		            action_type,	    
		       	    NULL, 		    
		            NULL,		    
		            FALSE,		    
		            reply_routine,	    
		            type);                          
		if ERROR_CONDITION( send_status )			
		    {
		    if (return_avl != NULL)
	                moss_avl_free(&return_avl, TRUE);
    	    	    moss_free_time( operation_time );
    	    	    moss_free_pe_handle( pe_handle );
		    return MAN_C_PE_TIMEOUT;
                    }
                }
	    }
	 else
	 {
	    /*
	     * The access check succeeded.  See if this candidate passes
	     * the filter check.
	     */
	    status = moss_apply_smi_filter(filter,
					   object_instance,
					   mom_apply_filter,
					   (char *) specific_instance,
					   comparison_table);
            if (status == MAN_C_TRUE)
	    {
		/*
		 * This instance passed the filter, so go ahead and
		 * perform directive. 
		 */

		if (return_avl != NULL)
	            moss_avl_free(&return_avl, FALSE);

		instance_found = TRUE;
		/*
		 * Call the directive routine to perform the directive (get, set,
		 * action) on this instance. The perform directive routine 
		 * should set up the reply status.
	 	 */
		status = (directive_routine)(directive_oid,
					     directive_avl,
			    		     specific_instance,
					     &return_avl,
					     &reply_status,
					     &action_response_type);
		/* 
		 * If an error occurred or the reply_status
		 * was not set, setup the appropriate reply.
		 */ 
	        if (ERROR_CONDITION( status ) || 
		   (reply_status == (reply_type) NULL))
		    {
                    status = setup_reply( status, 
		       	         	  &reply_status, 
		       	         	  &action_response_type, 
			         	  return_avl,
		       	         	  class_code,
			         	  type,
			         	  action_type );
		    /*
		     * Setup reply may fail if it cannot map the error status
		     * into a valid error to be returned by this directive. If it
		     * cannot map that error, it may return PE_TIMEOUT. 
		     */
		    if ERROR_CONDITION( status )
		        {
    	    	        if (return_avl != NULL)
			    moss_avl_free( &return_avl, TRUE );
   	    	        moss_free_time( operation_time );
    	    	        moss_free_pe_handle( pe_handle );
		        return status;
		        }
		    }
		/*
		 * Check to see if reply needs to be send.
		 */
		if (_reply_required(handle) == MAN_C_TRUE)
		{

#ifdef MOMGENDEBUG
		    printf("\n================>  Reply from MOM <===============\n\n");
		    printf("----------- Object instance -----------\n");
		    dbg_print_avl( specific_object_instance );
		    printf("\n-------------- Reply AVL --------------\n");
		    dbg_print_avl( return_avl );
		    printf(  "================>  End of Reply <=================\n\n");
#endif /* MOMGENDEBUG */
		    /*
		     * Send response appropriate to directive
		     */
		     send_status = send_reply( 
					  pe_handle,
					  invoke_identifier,	
					  reply_status,	
					  object_class,    	
					  specific_object_instance, 
					  object_uid,		
					  operation_time,	
					  action_type, 
					  action_response_type, 
					  return_avl,		
					  more_instances,	
					  reply_routine,
					  type );	

                     if ( ( specific_object_instance != NULL ) &&
                          ( specific_object_instance != object_instance ) )
                         moss_avl_free( &specific_object_instance, TRUE);

		     /* 
		      * Check the status from the send_reply routine. If it	
		      * failed, then no reply can be sent. Return an error
		      * status instead.
		      */
		     if ERROR_CONDITION( send_status )
		         {
    	    		 if (return_avl != NULL)
			    moss_avl_free( &return_avl, TRUE );
    	    		 moss_free_time( operation_time );
    	    		 moss_free_pe_handle( pe_handle );
			 return MAN_C_PE_TIMEOUT;
                         } /* Error condition */
                } /* Reply required 	   */
            } /* Instance passed filter */
	} /* Access check passed */
    } /* No more instances */

    /*
     * Send error status back if an error other than false occurred. We should 
     * always end up with FALSE unless an error occurred. FALSE indictates that
     * no more instances were found.
     *
     * Setup_reply maps status into reply_status as a valid reply to be returned
     * back for this directive.
     */
#ifndef SNMP
    if (status != MAN_C_FALSE)
#else
    /*
     * Since get_instance() may not have freed get_context, we free
     * it here if necessary.
     */
    if (get_context != NULL)
        {
        free(get_context);
        get_context = NULL;
        }
    if ((status != MAN_C_FALSE) && (status != MAN_C_SUCCESS))
#endif
	{
	status = setup_reply( status, 
		     	      &reply_status, 
		     	      &action_response_type, 
		     	      return_avl,
		     	      class_code,
		     	      type,
		     	      action_type );
        if ERROR_CONDITION( status )
	    {
    	    if (return_avl != NULL)
		moss_avl_free( &return_avl, TRUE );
    	    moss_free_time( operation_time );
    	    moss_free_pe_handle( pe_handle );
	    return status;
	    }
	
       	send_status = send_reply( 
			     pe_handle,           
		    	     invoke_identifier,   
		    	     reply_status,  
		    	     object_class,	    
		    	     object_instance,     
		    	     NULL,		    
		    	     operation_time,     
		    	     action_type,	    
		    	     action_response_type, 		    
		    	     NULL,
		    	     FALSE,		    
		    	     reply_routine,	    
		    	     type);                          
        if ERROR_CONDITION( send_status )
	    {
    	    if (return_avl != NULL)
		moss_avl_free( &return_avl, TRUE );
    	    moss_free_time( operation_time );
    	    moss_free_pe_handle( pe_handle );
	    return MAN_C_PE_TIMEOUT;
	    }
	
	}

    /*
     * Free the return_avl, if it was allocated.
     */
    if (return_avl != NULL)
	moss_avl_free( &return_avl, TRUE );

    /*
     * Check to see if we found any instances.  If no instances were
     * found, return NO_SUCH_OBJECT_INSTANCE.
     */
    if (!instance_found)
	{
        send_status = send_reply( 
			     pe_handle,           
		    	     invoke_identifier,   
		    	     (reply_type) MAN_C_NO_SUCH_OBJECT_INSTANCE,  
		    	     object_class,	    
		    	     object_instance,     
		    	     NULL,		    
		             operation_time,     
		    	     action_type,	    
			     NULL, 		    
			     NULL,		    
			     FALSE,		    
			     reply_routine,	    
			     type);                          
        if ERROR_CONDITION( send_status )
	    {
    	    moss_free_time( operation_time );
    	    moss_free_pe_handle( pe_handle );
	    return MAN_C_PE_TIMEOUT;
	    }
	}

    moss_free_time( operation_time );

    /*
     * Free up the protocol engine handle.
     */
    moss_free_pe_handle( pe_handle );

    return MAN_C_SUCCESS;
} /* End of moi_directive() */

/* End of directive.c */
