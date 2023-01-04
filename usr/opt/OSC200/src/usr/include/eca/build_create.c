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
static char *rcsid = "@(#)$RCSfile: build_create.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:20:37 $";
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
#include "moss_dna.h"
#include "extern_nil.h"
#include "common.h"

#define SUCCESS TRUE


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**    check_existence
**    
**    This routine builds the instance name string from the AVL and calls
**    perform_locate to see if it exists.
**
**  FORMAL PARAMETERS:
**
**    object_instance   - AVL containing instance name.
**    instance_octet    - Address of pointer to contain instance octet.
**    instance_name	- Address of pointer to contain instance name.
**    instance_length   - Address of integer to contain instance name length.
**    instance_ptr	- Address of pointer to contain class-specific structure 
**    class_code	- Integer containing class code.
**
**  RETURN VALUES:
**
**     MAN_C_SUCCESS or any error status from locate routine.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status check_existence( object_instance,
				   instance_octet,
			   	   instance_name,
			   	   instance_length,
			   	   instance_ptr,
			    	   class_code )

avl		     *object_instance;
octet_string	    **instance_octet;
char		    **instance_name;
int		     *instance_length;
void		    **instance_ptr;
int		      class_code;

{
    unsigned int    tag;
    man_status	    status;
    int		    last_one;
    man_status	    (*perform_locate)();
    object_id	    *instance_oid;

    /*-insert-code-create-select-locate-*/

    moss_avl_reset( object_instance );

    /*
     * Find last element in list.  This is our "name"
     */

    status = moss_avl_find_item( object_instance, 
				 instance_oid,
				 NULL, 
				 &tag, 
				 instance_octet );
    if (ERROR_CONDITION( status ))
        return status;

    *instance_name = (char *) malloc( (*instance_octet)->length + sizeof(char) );
    *instance_length = (*instance_octet)->length;
    memcpy(*instance_name, (*instance_octet)->string, (*instance_octet)->length);
    (*instance_name)[*instance_length] = '\0';

    if (status == MAN_C_SUCCESS)
        status = perform_locate( object_instance, 
				 *instance_name, 
				 *instance_length,
				 instance_ptr );
    else if (*instance_name != NULL)
        free( *instance_name );

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**    mom_check_attributes
**    
**    This routine calls class specific check attribute routine.
**
**  FORMAL PARAMETERS:
**
**    attribute_list	- AVL containing attributes to check.
**    reply_avl		- Return AVL containing reply.
**    class_code	- Integer containing class.
**
**  RETURN VALUES:
**
**      Any status from the class specific check routines.
**
**	Valid statuses for action are:
**	    MAN_C_INVALID_ARGUMENT_VALUE
**	    MAN_C_PROCESSING_FAILURE
**
**	Valid statuses for create are:
**          MAN_C_NO_SUCH_ATTRIBUTE_ID
**          MAN_C_MISSING_ATTRIBUTE_VALUE
**	    MAN_C_INVALID_ATTRIBUTE_VALUE
**	    MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
/*-insert-code-mom-check-attributes-*/

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      send_reply
**
**	This routine calls the appropriate reply routine (either action or
**	create. DNA CMIP uses the action entry point to create instances.
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
			      invoke_id, 
		              reply,
			      object_class, 
			      object_instance,
			      object_uid,
			      operation_time,
			      action_type,
			      action_response_type_oid,
			      attribute_list,
			      more_replies,
			      called_by_invoke_action )

man_binding_handle	pe_handle;
int			invoke_id;
reply_type		reply;
object_id		*object_class;
avl			*object_instance;
uid			*object_uid;
mo_time			*operation_time;
object_id		*action_type;
object_id		*action_response_type_oid;
avl			*attribute_list;
int			more_replies;
int			called_by_invoke_action;

{
man_status status;

#ifndef VMS
	/* check for null arguments...need to use special nil values for rpc */

if (object_uid == NULL) object_uid = &nil_uid;
if (attribute_list == NULL) attribute_list = nil_avl;
#endif

if (called_by_invoke_action == ACTION)
    status = moss_send_action_reply( pe_handle,
				     invoke_id,		
				     reply, 		
				     object_class,	
				     object_instance,	
				     object_uid,	
				     *operation_time,	
				     action_type,	
				     action_response_type_oid, 
				     attribute_list,	
				     more_replies);	
else
    status = moss_send_create_reply( pe_handle,
				     invoke_id,	 	
			             reply, 	       	
			       	     object_class,   	
			             object_instance,	
			             object_uid,	
				     *operation_time,	
			             attribute_list);	
return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      create_instance
**
**	This routine performs all of the verification and validation of
**	the instance to be created. Security access routines are also
**	called from this routine. This routine can be called from both 
**	moi_invoke_action (for DNA CMIP) and moi_create_instance.
**
**  FORMAL PARAMETERS:
**
**	object_class       - OID of class to create.
**	object_instance    - AVL containing instance name to create. 
**      superior_instance  - AVL containing superior instance name. 
**	access_control     - AVL containing access control information.
**	reference_instance - AVL containing template instance for setting attributes.
**	attribute_list     - AVL of attribute value pairs for the management operation.
**	invoke_id          - Invocation ID.
**      return_routine     - Management handle.
**      action_type        - object ID of action to be performed (NULL if create)
**      called_by_invoke_action - flag to indicate create or action
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
man_status create_instance ( mom_handle,
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
object_id		*object_class;
avl			*object_instance;
avl			*superior_instance;
avl			*access_control;
avl			*reference_instance;
avl			*attribute_list;
int			invoke_id;
management_handle	*return_routine;
object_id		*action_type;
int			called_by_invoke_action;

{
    man_binding_handle  pe_handle;
    avl			*reply_avl = NULL;
    reply_type		reply;
    struct object_id 	*action_response_type_oid = NULL;
    man_status 		(*perform_routine)();
    int 		class_code;
    octet_string	*instance_octet;
    mo_time	    	*operation_time = NULL;
    man_status 		status;
    man_status 		send_status = MAN_C_PE_TIMEOUT;
    char		*instance_name;
    int 		instance_length = 0;
    [[class_void_name]]	*ref_instance_ptr;

#ifdef MOMGENDEBUG
    printf("In the CREATE routine\n");
    printf("Argument List:\n");
    dbg_print_avl( attribute_list );
#endif
   
    /* 
     * Reset all incoming AVLs. 
     */
    moss_avl_reset( object_instance );
    moss_avl_reset( superior_instance );
    moss_avl_reset( reference_instance );
    moss_avl_reset( attribute_list );
  
    /*
     * Allocate a timestamp.
     */
    status = moss_get_time( &operation_time );
    if ERROR_CONDITION( status ) 
        return( MAN_C_INSUFFICIENT_RESOURCES );

    /* 
     * Allocate an AVL to be used to send a reply. 
     */
    status = moss_avl_init( &reply_avl );
    if ERROR_CONDITION( status ) 
        { 
        moss_free_time( operation_time );
        return( MAN_C_INSUFFICIENT_RESOURCES );
        }
  
    /*
     * Bind once to the Reply Receiver.
     */ 
    status = moss_alloc_pe_handle( return_routine , &pe_handle );
    if ERROR_CONDITION(status)
      return(MAN_C_INSUFFICIENT_RESOURCES);

    /*-insert-code-create-select-routine-*/

    if ERROR_CONDITION(status)
        {
	moss_free_time( operation_time );
        moss_free_pe_handle( pe_handle );
        return (_error_exit("\n*** Error in create, class not supported\n", status));
        }

    /*
     * Check the access of the requestor against supplied instances.
     * Send error reply, MAN_C_ACCESS_DENIED, if access check fails.
     */
    status = check_access( object_class, 
			   object_instance, 
			   superior_instance, 
			   reference_instance, 
			   access_control);

    if (status != MAN_C_SUCCESS)
        {
        /*
         * Requestor does not have the right to create a new instance
         */
        send_status = send_reply( 
		   pe_handle,		  
		   invoke_id,	          
		   (reply_type) MAN_C_ACCESS_DENIED, 	  
		   object_class,	          
		   object_instance,         
		   NULL,		          
		   operation_time,         
		   action_type,	          
		   NULL, 		          
		   NULL,		          
		   FALSE,		          
		   called_by_invoke_action);
        if ERROR_CONDITION( send_status )
	    {
	    moss_free_time( operation_time );
            moss_free_pe_handle( pe_handle );
	    return MAN_C_PE_TIMEOUT;
	    }        
	}
      else {   	/* Check access succeeded */
    	/*
	 * Check to see whether new instance names comes from object_instance or
     	 * superior_instance.  
     	 */
        if (moss_avl_point(object_instance,
		       	   NULL,
		       	   NULL,
		       	   NULL,
		       	   NULL,
		       	   NULL) == MAN_C_NO_ELEMENT)  /* object instance */
	    {
            if (moss_avl_point(superior_instance,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL) == MAN_C_NO_ELEMENT)  /* superior_instance */
	       {
	       /*
	        * Neither object_instance or superior_instance was supplied.
	        * Since one of these is needed to construct a new instance name,
	        * we can't create a new instance.  Since DNA_CMIP does not
	        * support MAN_C_INVALID_OBJECT_INSTANCE,  return NO_SUCH_OBJECT_INSTANCE
	        * instead.
	        */
	
                send_status = send_reply( 
			  pe_handle,		  
		   	  invoke_id,	          
		   	  (reply_type) MAN_C_NO_SUCH_OBJECT_INSTANCE,
		   	  object_class,	          
		   	  object_instance,         
		   	  NULL,		          
			  operation_time,         
		   	  action_type,	          
		   	  NULL, 		          
		   	  NULL,		          
		   	  FALSE,		          
		          called_by_invoke_action);
        	if ERROR_CONDITION( send_status )
		    {
		    moss_free_time( operation_time );
                    moss_free_pe_handle( pe_handle );
	            return MAN_C_PE_TIMEOUT;
		    }
		}
	    else {
	 	/*
		 * The superior_instance name was supplied. Check to see if the
		 * superior_instance exists.
		 */
		status = moss_avl_reset(superior_instance);
		
 		/** 					  **
		 ** Determine if superior instance exists **
		 ** 					  **
		 ** Also, check superior instance access. **
		 **                                       **/
		
		if (status != MAN_C_SUCCESS)
		    {
	  	    /*
	   	     * The specified superior_instance does not exist.
	   	     */
              	    send_status = send_reply( 
			        pe_handle,		  
		   	  	invoke_id,	          
		   	  	(reply_type) MAN_C_NO_SUCH_OBJECT_INSTANCE,
		   	  	object_class,	          
		   	  	object_instance,         
		   	  	NULL,		          
			        operation_time,         
		   	  	action_type,	          
		   	  	NULL, 		          
		   	  	NULL,		          
		   	  	FALSE,		          
		          	called_by_invoke_action);
        	    if ERROR_CONDITION( send_status )
		        {
		        moss_free_time( operation_time );
                        moss_free_pe_handle( pe_handle );
	                return MAN_C_PE_TIMEOUT;
		        }
		    }
            }	
     	}	
    else {  /* Object_instance */
	
    	status = moss_avl_reset(object_instance);

    	/*
     	 * Check to see if specified object instance exists.
     	 */

#ifdef MOMGENDEBUG
    	printf("****************Print out the object_instance**************\n");
    	dbg_print_avl(object_instance);
    	printf("****************done***************************************\n");
#endif

    	status = check_existence(object_instance, 
			        &instance_octet,
			     	&instance_name, 
			     	&instance_length, 
			     	NULL,
			     	class_code);

    	if (status == MAN_C_TRUE)
	    {
	    /*	    
	     * Object instance already exists.  Return duplicate error reply.
	     */
	    if (called_by_invoke_action == ACTION) 
	      {
	      /*-insert-code-duplicate-error-reply-*/

              send_status = send_reply( pe_handle,		  
		   	  		invoke_id,	          
		   	  		(reply_type) MAN_C_PROCESSING_FAILURE,
		   	  		object_class,	          
		   	  		object_instance,         
		   	  		NULL,		          
				        operation_time,         
		   	  		action_type,	          
		   	  		action_response_type_oid, 
		   	  		reply_avl,
		   	  		FALSE,		          
		          		called_by_invoke_action);
	      } 		
	    else 
              send_status = send_reply( pe_handle,		  
		   	  		invoke_id,	          
		   	  		(reply_type) MAN_C_DUPLICATE_M_O_INSTANCE,
		   	  		object_class,	          
		   	  		object_instance,         
				        NULL,		          
		   	  		operation_time,         
		   	  		action_type,	          
		   	  		NULL, 		          
		   	  		NULL,		          
		   	  		FALSE,		          
		          		called_by_invoke_action);

          if ERROR_CONDITION( send_status )
	      {
	      moss_free_time( operation_time );
              moss_free_pe_handle( pe_handle );
	      return MAN_C_PE_TIMEOUT;
	      }
	  }
        else {

        /*
         * We can create the instance if all attributes for the new instance can
         * be accounted for. First check the attributes supplied to see if they
         * are all valid
         */

        status = mom_check_attributes( attribute_list, 
				       reply_avl, 
				       class_code );
        
        if (status != MAN_C_SUCCESS) {
	    /*
	     * Error from check attributes, send back status as reply. It will
	     * be some type of valid create reply. If called from action, map
	     * create reply into action and allocate a temporary UID to use.
	     */
	    {							  
	    uid *t_uid = NULL;

            if (called_by_invoke_action == ACTION)
		{
	    	moss_get_uid(&t_uid);              		  
		/*
		 * Map the error status into a valid error reply. If the mapping
		 * cannot be done, return a valid status (i.e. PE_TIMEOUT or
		 * INSUFFICIENT_RESOURCES) back to the user.
		 */
		status = setup_reply( status, 
		             	      &reply, 
		             	      &action_response_type_oid, 
			              reply_avl,
		             	      class_code,
		                      called_by_invoke_action,
			     	      action_type );
                if ERROR_CONDITION( status )
	    	    {
		    moss_free_time( operation_time );
                    moss_free_pe_handle( pe_handle );
		    moss_free_uid(&t_uid);			  
		    return status;
		    }		
		}
#ifdef MOMGENDEBUG
    	    printf("**************** Error Reply AVL **************\n");
            dbg_print_avl( reply_avl );
    	    printf("***********************************************\n");
#endif /* MOMGENDEBUG */

            send_status = send_reply( 
			pe_handle,		  
			invoke_id,	          
			reply,
			object_class,	          
			object_instance,         
			t_uid,		          
		        operation_time,         
			action_type,	          
			action_response_type_oid, 		          
			reply_avl,		          
			FALSE,		          
			called_by_invoke_action);

            if (called_by_invoke_action == ACTION)
	    	moss_free_uid(&t_uid);			  

            if ERROR_CONDITION( send_status )
	        {
	        moss_free_time( operation_time );
                moss_free_pe_handle( pe_handle );
	        return MAN_C_PE_TIMEOUT;
	        }
	    }
	}
	else {
	   /*
	    * Instance does not exist and attributes are valid, call the class 
	    * specific instance routine to create it. 
	    */                                                
    	   status = ( perform_routine )( object_instance,
				         instance_octet,
					 instance_name, 
				   	 instance_length, 
				   	 &ref_instance_ptr, 
				   	 attribute_list,
					 &reply_avl );
	   /* 
	    * The only error conditions that can be returned are:	
	    * 	MAN_C_INSUFFICIENT_RESOURCES
	    * 	MAN_C_PE_TIMEOUT     
	    * All others need to be returned by a reply.  Set up the 
	    * appropriate reply based on the status.
	    */

#ifdef MOMGENDEBUG
    	    printf("**************** Reply AVL **************\n");
            dbg_print_avl( reply_avl );
    	    printf("*****************************************\n");
#endif /* MOMGENDEBUG */
	    /*
	     * Map the error status into a valid error reply. If the mapping
	     * cannot be done, return a valid status (i.e. PE_TIMEOUT or
	     * INSUFFICIENT_RESOURCES) back to the user.
	     *
             * NOTE: The perform create module does not setup the reply_status.
	     */
	    status = setup_reply( status, 
			          &reply, 
			          &action_response_type_oid, 
				  reply_avl,
			          class_code,
			          called_by_invoke_action,
				  action_type );
	    if ERROR_CONDITION( status )
		{	
	        moss_free_time( operation_time );
                moss_free_pe_handle( pe_handle );
		return status;
		}      
            send_status = send_reply( pe_handle, 
			    	      invoke_id, 		
				      reply,		
				      object_class,	
				      object_instance,	
				      NULL,		
				      operation_time,	
				      action_type,	
				      action_response_type_oid, 
				      reply_avl,
				      FALSE,              
				      called_by_invoke_action);		
	       }
            }
        }
    }
    moss_free_time(operation_time);
    moss_avl_free( &reply_avl, TRUE );

    moss_free_pe_handle( pe_handle );

    if ERROR_CONDITION( send_status )
        return MAN_C_PE_TIMEOUT;
    else
        return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      moi_create_instance
**
**	This routine calls create_instance with the CREATE flag set.
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
			  CREATE );
return status;
}
