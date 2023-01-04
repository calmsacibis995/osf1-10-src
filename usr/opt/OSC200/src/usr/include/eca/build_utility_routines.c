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
static char *rcsid = "@(#)$RCSfile: build_utility_routines.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:24:06 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	UTILITY_ROUTINES.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].  It supplies common functions used in
**	the MOM.
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
#include "syslog.h"
#ifdef MCC
#include "mcc_interface_def.h"
#endif /* MCC */

#ifdef VMS
#include "dnsdef.h"
#include "mossvms_auth.h"
#else
extern	comparison  *table_array[];
#endif

/*
 *  Definitions
 */

#define COMPARE( x, y ) \
    ( ((x) > (y)) ? MAN_C_GREATER               \
                  : ((x) < (y)) ? MAN_C_LESS    \
                                : MAN_C_EQUAL )

/*  Maximum size of error msg to syslog */
#define MAXERRMSG 300

/*
 *  Function Prototypes to satisfy compiler warnings.
 */

man_status 	_error_exit(),
  		_get_class_code(),
  		_list_error(),
  		_send_error(),
  		_reply_required();


/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	setup_reply
 **
 ** 	This routine sets up the appropriate reply (and response_type_oid for
 **	actions) based on the status.  Only certain errors may be returned for
 **	DNA CMIP GET,SET, and ACTIONs. These errors are listed in this routine.
 ** 	All other errors must be returned via some user-defined exception for
 **	actions or handled as a fatal error for SETs and GETs.
 **
 ** 	If a status can be mapped (and consequently be returned as a valid reply
 **     for this directive), MAN_C_SUCCESS *must* be returned as the return
 **	status for this routine. Otherwise, call _error_exit to handle the
 ** 	appropriate error (possibly by exiting) or return MAN_C_INSUFFICIENT_RESOURCES
 **     or MAN_C_PE_TIMEOUT and the caller of this routine will return that
 **	status as the return status from the appropriate MOI routine.
 **	
 **  FORMAL PARAMETERS:
 **
 **	status	    status
 **	reply	    address of reply to return.
 **	oid	    pointer to address of action OID.
 **	attr	    address of AVL (used in passing back parameters).
 **	class_code  which class (in multi_class).
 **	type	    type of directive (action,create,delete,set,get).
 **	action_type OID of action directive.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS 	 - Successfully setup the error reply
 **	MAN_C_PE_TIMEOUT - If not.
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status setup_reply( status, 
		        reply, 
		      	oid, 
		      	attr,
		      	class_code,
		      	type,
			action_type )

man_status 	status;
reply_type 	*reply;
object_id  	**oid;
avl	 	*attr;
int		class_code;
int		type;
object_id	*action_type;

{
object_id *arg_oid;
man_status stat;
octet_string octet;
int	    reason;

/*
 * MAN_C_INVALID_USE_OF_WILDCARD is common between GET, SET, and ACTION.
 * The wildcard reason is a required argument.
 */

if (status == MAN_C_INVALID_USE_OF_WILDCARD)
    {
    *reply = (reply_type) status;
	
    /* 
    * Set up wildcard reason.
    */

    reason = 1;	/** Need to determine to correct wildcard reason **/

    octet.data_type = ASN1_C_INTEGER;
    octet.length = 4;
    octet.string = (char *) &reason;
    status = moss_avl_add( attr, 
                           NULL,
                           (unsigned int) MAN_C_SUCCESS,
                           ASN1_C_INTEGER,
                           &octet);
    *oid = NULL;           
    return MAN_C_SUCCESS;
    }

if ((type == GET) || (type == SET))
  switch (status)
    {
    /*
     * These are the only valid errors that can be returned for DNA CMIP GET 
     * and SET. Anything else cannot be returned. In this case, an error
     * message is displayed and PE_TIMEOUT is returned as a status.
     */
    case MAN_C_SUCCESS 		    	:
    case MAN_C_ACCESS_DENIED 	    	:
    case MAN_C_NO_SUCH_OBJECT_INSTANCE 	:
    case MAN_C_DIRECTIVE_NOT_SUPPORTED 	: 
    case MAN_C_INSUFFICIENT_RESOURCES  	:
    case MAN_C_GET_LIST_ERROR	    	:
    case MAN_C_SET_LIST_ERROR	    	:
    case MAN_C_ENTITY_CLASS_NOT_SUPPORTED:    
    case MAN_C_CONSTRAINT_VIOLATION	 :
        *reply = (reply_type) status;
	*oid = NULL;
    	break;
	
    default:
	return( _error_exit( "\n**** Invalid error received in setup_reply ****\n", status));
    }

else if (type == ACTION)
    switch (status)
    {
    case MAN_C_SUCCESS :
	/*-insert-code-reply-select-routine-*/

	*reply = (reply_type) MAN_C_SUCCESS;
	break;

    case MAN_C_ACCESS_DENIED 		:
    case MAN_C_NO_SUCH_OBJECT_INSTANCE  :
    case MAN_C_DIRECTIVE_NOT_SUPPORTED  : 
    case MAN_C_NO_SUCH_ACTION 		:
    case MAN_C_INSUFFICIENT_RESOURCES 	:
    case MAN_C_NO_SUCH_ARGUMENT 	:
    case MAN_C_REQUIRED_ARGUMENT_OMITTED:
    case MAN_C_PROCESSING_FAILURE	:
    case MAN_C_INVALID_ATTRIBUTE_VALUE:
    case MAN_C_NO_SUCH_ATTRIBUTE_ID:
    case MAN_C_MISSING_ATTRIBUTE_VALUE:
	*reply = (reply_type) status;
	*oid = NULL;
    	break;

/*+++
**   This section is intended to trap random errors.
**   If you have specific errors, you can add
**   them here.
**/

    default:	
	{
	*reply = (reply_type) MAN_C_PROCESSING_FAILURE;
	/*-insert-code-default-error-reply-*/
	break;
	}
    }    

return MAN_C_SUCCESS;
} /* end of setup_reply() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	_error_exit
 **
 ** 	This routine handles fatal programming errors that cannot be returned
 **	back to the user.  If the MOM cannot exit and must return some 
 **	type of error, modify this routine to return MAN_C_PE_TIMEOUT
 **	as a status (i.e. not an error reply). This will indicate the 
 ** 	MOM was not able to return an error reply.
 **
 **  FORMAL PARAMETERS:
 **
 **	error_string	Address of error string to print out.
 **	status 		Integer containing fatal error status.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_PE_TIMEOUT  - Possible error status to return.
 **
 **  SIDE EFFECTS:
 **
 **     MOM image will exit.
 **
 **--
 */
man_status _error_exit( error_string, 
		        status )

char *error_string;
man_status status;

{
    printf("%s\n",error_string);

    /** If the MOM cannot exit, remove this call and return PE_TIMEOUT **/
    exit( (int) status );
    
    return MAN_C_PE_TIMEOUT;
} /* end of _error_exit() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	copy_avl_attribute
 **
 ** 	This routine initializes a new AVL and copies the source AVL into
 **	the destination AVL.
 **
 **  FORMAL PARAMETERS:
 **
 **	object_id   *new_oid
 **	avl 	    *attr_list
 **	avl 	    **new_attr
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status copy_avl_attribute( new_oid, attr_list, new_attr )

object_id *new_oid;
avl *attr_list;
avl **new_attr;

{
object_id 	*oid;
octet_string 	*octet;
man_status 	status;
int 		last_one = FALSE;
unsigned int    modifier, tag;

/*
 * Set up the new AVL
 */

if (*new_attr != NULL)
    {
    status = moss_avl_free( new_attr, FALSE );
    if ERROR_CONDITION(status)
	return status;
    }

status = moss_avl_init( new_attr );
if ERROR_CONDITION(status)
    return status;

/* 
 * The attr_list AVL currently points to the element *after* the start
 * construction. Backup to the start construction, then copy the entire 
 * construction to a temporary avl.
 */

status = moss_avl_backup( attr_list );
if ERROR_CONDITION(status)
    return status;

status = moss_avl_copy( *new_attr,  /* avl_handle_dest */
			attr_list,  /* avl_handle_src  */
			FALSE,      /* reset_dest      */
			new_oid,    /* dest_oid        */
			NULL,       /* dest_modifier   */
			&last_one); /* source_last_one */
if ERROR_CONDITION(status)
    return status;

#ifdef MOMGENDEBUG
dbg_print_avl ( *new_attr );
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* end of copy_avl_attribute() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	add_avl_attribute
 **
 ** 	This routine adds an element to the AVL.
 **
 **  FORMAL PARAMETERS:
 **
 **	object_id   *new_oid
 **	avl 	    *attr_list
 **	avl 	    **attr
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status add_avl_attribute( new_oid, attr_list, attr )

object_id *new_oid;
avl *attr_list;
avl **attr;

{
object_id 	*oid;
octet_string 	*octet;
man_status 	status;
int 		last_one = FALSE;
unsigned int    modifier, tag;
avl 		*temp_avl = NULL;

if (*attr == NULL)
    {
    status = copy_avl_attribute( new_oid, attr_list, attr );
    return status;
    }

/*
 * Initialize the temp AVL.
 */

status = moss_avl_init( &temp_avl );
if ERROR_CONDITION(status)
    return status;

/* 
 * The attr_list AVL currently points to the element *after* the start
 * construction. Backup to the start construction, then copy the entire 
 * construction to a temporary avl.
 */

status = moss_avl_backup( attr_list );
if ERROR_CONDITION(status)
    return status;

status = moss_avl_copy( temp_avl,   /* avl_handle_dest */
			attr_list,  /* avl_handle_src  */
			FALSE,      /* reset_dest      */
			NULL,       /* dest_oid        */
			NULL,       /* dest_modifier   */
			&last_one); /* source_last_one */
if ERROR_CONDITION(status)
    return status;

status = moss_avl_reset( temp_avl );
if ERROR_CONDITION(status)
    return status;

status = moss_avl_reset( *attr );
if ERROR_CONDITION(status)
    return status;

/*
 * Get the past the start construction from the temp avl and existing attr avl.
 */

status = moss_avl_point( temp_avl, &oid, &modifier, &tag, &octet, &last_one);
if ERROR_CONDITION(status)
    return status;

status = moss_avl_point( *attr, &oid, &modifier, &tag, &octet, &last_one);
if ERROR_CONDITION(status)
    return status;

/*
 * Copy each element of the construction stopping at the end of construction.
 */

do {
    status = moss_avl_point( temp_avl, &oid, &modifier, &tag, &octet, &last_one);
    if ERROR_CONDITION(status)
        return status;

    if (tag != ASN1_C_EOC)
	{
	status = moss_avl_add_cons_field( *attr, (unsigned int) NULL, tag, octet );
	if ERROR_CONDITION(status)
	    return status;
	}
   }
while (!last_one);

status = moss_avl_free( &temp_avl, TRUE );
if ERROR_CONDITION(status)
    return status;

#ifdef MOMGENDEBUG
dbg_print_avl ( *attr );
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* end of add_avl_attribute() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	remove_avl_attribute
 **
 ** 	This routine removes an element from the AVL.
 **
 **  FORMAL PARAMETERS:
 **
 **	object_id   *new_oid
 **	avl 	    *attr_list
 **	avl 	    **attr
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status remove_avl_attribute( new_oid, attr_list, attr )

object_id *new_oid;
avl *attr_list;
avl **attr;

{
object_id 	*oid,*oid1;
octet_string 	*octet,*octet1;
man_status 	status,status1;
int 		last_one = FALSE, 
		last_one1 = FALSE;
unsigned int    modifier, tag;
unsigned int    modifier1, tag1;
avl 		*temp_avl = NULL;

/*
 * Initialize the temp AVL
 */

status = moss_avl_init( &temp_avl );
if ERROR_CONDITION(status)
    return status;

/* 
 * The attr_list AVL currently points to the element *after* the start
 * construction. Backup to the start construction, then copy the entire 
 * construction to a temporary avl.
 */

status = moss_avl_backup( attr_list );
if ERROR_CONDITION(status)
    return status;

status = moss_avl_copy( temp_avl,   /* avl_handle_dest */
			attr_list,  /* avl_handle_src  */
			FALSE,      /* reset_dest      */
			NULL,       /* dest_oid        */
			NULL,       /* dest_modifier   */
			&last_one); /* source_last_one */
if ERROR_CONDITION(status)
    return status;

status = moss_avl_reset( temp_avl );
if ERROR_CONDITION(status)
    return status;

status = moss_avl_reset( *attr );
if ERROR_CONDITION(status)
    return status;

/*
 * Get the past the start construction from the temp avl and existing attr avl.
 */

status = moss_avl_point( temp_avl, &oid, &modifier, &tag, &octet, &last_one);
if ERROR_CONDITION(status)
    return status;

status = moss_avl_point( *attr, &oid, &modifier, &tag, &octet, &last_one);
if ERROR_CONDITION(status)
    return status;

/*
 * Copy each element of the construction stopping at the end of construction.
 */

do {
    status = moss_avl_point( temp_avl, &oid, &modifier, &tag, &octet, &last_one);
    if ERROR_CONDITION(status)
        return status;

    if (tag != ASN1_C_EOC)
	{
	status1 = moss_avl_reset( *attr );
	if ERROR_CONDITION(status1)
	    return status1;

	status1 = moss_avl_point( *attr, &oid1, &modifier1, &tag1, &octet1, &last_one1);
	if ERROR_CONDITION(status)
	    return status;

	do 
	    {
	    status1 = moss_avl_point( *attr, &oid1, &modifier1, &tag1, &octet1, &last_one1);
    	    if ERROR_CONDITION(status1)
	        return status;
	
	    if (tag1 != ASN1_C_EOC)
		{
		avl *orig=NULL,*cand=NULL;
		moss_avl_init(&orig);moss_avl_init(&cand);
		status = moss_avl_add( orig, 
				       oid1,
                                       (unsigned int) MAN_C_SUCCESS,
				       tag1,
				       octet1 );
		status = moss_avl_add( cand, 
				       oid,
                                       (unsigned int) MAN_C_SUCCESS,
				       tag,
				       octet );
#ifdef MOMGENDEBUG
		printf("Original ******\n");
		dbg_print_avl ( orig );
		printf("Candidate -----\n");
		dbg_print_avl ( cand );
#endif /* MOMGENDEBUG */
                status = moss_match_instance_name( orig, cand, 0, NULL );
		if (status == MAN_C_TRUE )
		    {
		    status = moss_avl_backup( *attr );
		    if ERROR_CONDITION(status)
			return status;
    		    status = moss_avl_remove( *attr );			
		    }
		moss_avl_free(&orig,TRUE);moss_avl_free(&cand,TRUE);
		}
            }
	while (!last_one1);

	printf("Removing an AVL item\n");	
	}
   }
while (!last_one);

status = moss_avl_free( &temp_avl, TRUE );
if ERROR_CONDITION(status)
    return status;

#ifdef MOMGENDEBUG
dbg_print_avl ( *attr );
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* end of remote_avl_attribute() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	_get_class_code
 **
 ** 	Determines which class the object_class belongs to. It returns the id
 **     of the class(es) supported by this MOM.
 **
 **  FORMAL PARAMETERS:
 **
 **	object_id   *object_class   - The OID to compare
 ** 	int 	    *class_code	    - the returned class
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **	MAN_C_ENTITY_CLASS_NOT_SUPPORTED
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status _get_class_code(object_class,
			   class_code)

object_id 	*object_class;
int 		*class_code;

{
  man_status status;
  
  /*-insert-code-class-compare-*/

  return MAN_C_SUCCESS;
} /* end of _get_class_code() */

#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	 simple_name_to_char
**
**       This routine modifiers the octet to make it point to what looks like a
**       plain old character string.
**
**  FORMAL PARAMETERS:
**
**       octet  - Pointer to octet.
**       ptr    - Address of pointer to return string.
**	 length - Address of return length.
**
**  RETURNS VALUES:
**
**       MAN_C_SUCCESS
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status simple_name_to_char( octet, 
				ptr, 
				length)

octet_string  *octet;
char          **ptr;
int 	      *length;

{
    char	buffer[dns$k_simplestrmax];
    short int	buffer_len = sizeof(buffer);
    int		status;
    short int   o_length;

    o_length = (short int) octet->length;
    status = moss_dns_simplename_to_opaque( octet->string, 
					   &o_length,
					   buffer, 
					   &buffer_len );
  *length = buffer_len;
  *ptr = (char *) malloc( buffer_len );
  memcpy( *ptr, buffer, buffer_len );
  return (man_status) status;  
} /* end of simple_name_to_char() */
#endif /* VMS */
                
#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	char_to_simple_name
**
**      This routine modifies the octet to make it point to what looks like a
**      plain old character string.
**
**   FORMAL PARAMETERS:
**
**	ptr    - Address of string.
**	length - Address of length.
**      octet  - Pointer to octet.
**
**   RETURN VALUES:
**
**      MAN_C_SUCCESS
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status char_to_simple_name( ptr, 
				length, 
				octet )

char         *ptr;
int           length;
octet_string *octet;

{
    char	sn[dns$k_simplenamemax];
    short int 	sn_len = sizeof(sn);
    int		status;

    status = moss_dns_opaque_to_simplename( ptr, 
					    (short int *)&length, 
					    sn,
					    &sn_len );
    octet->string = (char *) malloc(sn_len);
    octet->length = sn_len;
    memcpy( octet->string, sn, sn_len );

    return (man_status) status;
} /* end of char_to_simple_name() */
#endif /* VMS */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      send_error_reply
**
**	This routine allocates an pe_handle and sends the reply.
**
**  FORMAL PARAMETERS:
**
**	return_routine  - Addresss of return information.
**	send_routine    - Address of appropriate moss send routine.
**	invoke_id	- Identifier associated with the request.
**	error_reply	- Error reply
**	object_class	- OID of the class to manipulate.
**	object_instance	- AVL containing instance that is being returned.
**	object_uid	- UID of object instance.
**	attribute_list  - AVL containing return attribute information.
**	action_type     - OID of action.
**
**  RETURN VALUES:
**
** 	Any status from _send_error.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status send_error_reply( return_routine,
		             send_routine,
		       	     invoke_id,
		      	     error_reply,
		             object_class,
		      	     object_instance,
		      	     object_uid,
		      	     attribute_list,
		      	     action_type
		    	   )

management_handle *return_routine;
man_status        (*send_routine)();
int               invoke_id;
man_status        error_reply;
object_id	  *object_class;
avl	          *object_instance;
uid	          *object_uid;
avl	          *attribute_list;
object_id	  *action_type;

{
    man_status status;
    man_binding_handle  pe_handle;

    if (((man_status) _reply_required( return_routine )) == MAN_C_TRUE)
        {
        status = moss_alloc_pe_handle( return_routine , &pe_handle );
        if (status != MAN_C_SUCCESS)
            return(MAN_C_INSUFFICIENT_RESOURCES);

        status = (man_status) 
		_send_error(pe_handle,
			   send_routine,
			   invoke_id,
			   error_reply,
			   object_class,
			   object_instance, 
			   NULL, NULL, action_type);
                               
	moss_free_pe_handle( pe_handle );
 	}

return status;
} /* end of send_error_reply() */

#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      validate_acl
**
**	This routine validates an ACL by calling the mossvms authorize
**	services to look up the actual identifier to see if it exists. If
** 	it does exist, it is converted back into an AVL.  THis routine is
**	typically called on create or set.
**
**  FORMAL PARAMETERS:
**
**	in_avl		Pointer to avl containing ACL.
**	in_oid		Address of OID of in_avl.
**	out_avl		Address of pointer to contain validated ACL.
**	out_oid		Address of OID of out_avl.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 		 
**	any error status from mossvms authorization services.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status validate_acl( in_avl,
			 in_oid,
			 out_avl,
			 out_oid )

avl *in_avl;
object_id *in_oid;
avl **out_avl;
object_id *out_oid;

{
    man_status status;
    VMSacl *idacl = NULL;

    if (*out_avl != NULL)
	moss_avl_free( out_avl, TRUE );

    moss_avl_init( out_avl );
    mossvms_auth_acl_init( &idacl );
    moss_avl_backup( in_avl ); 
    status = (man_status) mossvms_auth_avl_to_idacl( 
				in_avl,
				in_oid,
				idacl, 
				NULL );
    if ERROR_CONDITION( status )
	return status;

    status = (man_status) mossvms_auth_idacl_to_avl( 
				 *out_avl,
				 out_oid,
				 idacl, 
				 NULL );

    mossvms_auth_free_acl( &idacl);
   
    return (man_status) status;
} /* end of validate_acl() */
#endif /* VMS */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	_list_error
 **
 ** 	This routine adds an error status to the reply_attribute_list.
 **
 **  FORMAL PARAMETERS:
 **
 ** 	attr_oid    	    	pointer to the object_id
 **	reply_attribute_list 	pointer to reply avl to add error_type to.
 ** 	reply_status	    	pointer to unsigned int
 ** 	error_type  	    	reply_type
 ** 	return_error 	    	reply_type	
 **
 **  RETURN VALUE:
 **
 **	Anything moss_avl_add can return. Typically MAN_C_SUCCESS. 
 **
 **  SIDE EFFECTS:
 **
 **     Adds an octet to the reply_attribute_list avl.
 **
 **--
 */
man_status _list_error( attr_oid,
		        reply_attribute_list,
		        reply_status,
		        error_type,
		        return_error )

object_id	*attr_oid;
avl		*reply_attribute_list;
unsigned int	*reply_status;
reply_type	error_type;
reply_type	return_error;

{
    man_status	status;

    /*
     * The overall status for the get request will be 
     * MAN_C__xxx_LIST_ERROR because at least one of
     * the requested attributes was not able to be
     * retrieved or set.
     */
    *reply_status = return_error;

    status = moss_avl_add(reply_attribute_list,
			  attr_oid,
			  error_type,
			  ASN1_C_NULL,
			  NULL);
	
    return status;
} /* end of _list_error() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	_send_error
 **
 ** 	This routine calls the appropriate moss_send_XXX_reply depending on the
 ** 	type of action the directive is.
 **
 **  FORMAL PARAMETERS:
 **
 **
 **  pe_handle	    	    	man_binding_handle
 **  send_routine   	    	address of a function returning man_status
 **  int	    	    	invoke_identifier
 **  error  	    	    	status
 **  object_class   	    	pointer to the object_id
 **  object_instance	    	pointer to the object_instance avl
 **  object_uid	    	    	pointer to the instance's uid
 **  attribute_list 	    	pointer to the instance's attribute avl
 **  action_type    	    	pointer to the action type
 **
 ** 	See the documentation for the moss_send_XXX_reply.
 **
 **  RETURN VALUE:
 **
 **	Anything moss_send_XXX_reply can return. Typically MAN_C_SUCCESS.  
 **
 **  SIDE EFFECTS:
 **
 **     Sends a reply back to the PE.
 **
 **--
 */
man_status _send_error( pe_handle,
		        send_routine,
		        invoke_identifier,
		        error,
		        object_class,
		        object_instance,
		        object_uid,
		        attribute_list,
		        action_type
		      )

man_binding_handle  pe_handle;
man_status   	    (*send_routine)();
int	    	    invoke_identifier;
man_status    	    error;
object_id	    *object_class;
avl	    	    *object_instance;
uid  		    *object_uid;
avl	    	    *attribute_list;
object_id	    *action_type;

{
  reply_type	reply;
  mo_time	*current_time = NULL;
  man_status	status;
  
  moss_get_time(&current_time);
  
  if (send_routine == moss_send_action_reply)
    status = moss_send_action_reply(pe_handle,
				    invoke_identifier, 		
				    (reply_type) error,
				    object_class,
				    object_instance,
				    (uid *) NULL,           /* UID */
				    *current_time,
				    action_type,
				    (object_id *)NULL,		/* Action response type */
				    (avl *)NULL,		/* Attribute list */
				    (int) FALSE);		/* More replies */					
  else if ((send_routine == moss_send_delete_reply) ||
	   (send_routine == moss_send_set_reply) || 
	   (send_routine == moss_send_get_reply))
    status = (send_routine)(pe_handle,
			    invoke_identifier,
			    (reply_type) error,
			    object_class,
			    object_instance,
			    (uid *)NULL,
			    *current_time,
			    (avl *)NULL,		/* Attribute list */
			    FALSE);
  else if ((send_routine) == moss_send_create_reply)	
    status = moss_send_create_reply(pe_handle,
				    invoke_identifier,
				    (reply_type) error,
				    object_class,
				    object_instance,
				    (uid *)NULL,
				    *current_time,
				    (avl *)NULL);		/* Attribute list */
  
  moss_free_time(current_time);
  
  return status;
} /* end of send_error() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	_reply_required
 **
 ** 	This routine determines if a reply is required.
 **
 **  FORMAL PARAMETERS:
 **
 **  	man_handle   	pointer to a management_handle.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_TRUE - if a reply is required, 
 **  	otherwise, MAN_C_FALSE.
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status _reply_required(man_handle)

management_handle   *man_handle;

{
    if (man_handle != NULL)
	return MAN_C_TRUE;
    else
	return MAN_C_FALSE;
} /* end of _reply_required() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	copy_octet_to_unsigned_int 
 **
 ** 	This routine transfers the ASN1_C_INTEGER from the octet to an unsigned
 ** 	int.  
 **
 **  FORMAL PARAMETERS:
 **
 **  	unsigned int     	integer to receive the value in the octet.
 **  	octet_string	    	octet that contains an ASN1_C_INTEGER.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS if successful
 **  	otherwise, MAN_C_BAD_PARAMETER
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status copy_octet_to_unsigned_int (dest,
				       src )

unsigned int        *dest;
octet_string        *src;

{
  int i;
  int len;
  unsigned char *buf;
  
  /*
   * If the destination or source is NULL, we have bad input.
   */
  if ((dest == NULL) ||
      (src == NULL))
    return MAN_C_BAD_PARAMETER;    
  
  if ((src->string == NULL) || (src->data_type != ASN1_C_INTEGER))
    return MAN_C_BAD_PARAMETER;

  len = src->length;
  buf = (unsigned char *)src->string;

  /*
   ** Only unsigned integers that are 32bits or less are supported.
   */
  if ((len == 5) &&
      (buf[4] != (char)NULL))
    return MAN_C_BAD_PARAMETER;

  if (len == 0)
      return MAN_C_BAD_PARAMETER;

  if (len == 5)
    len = 4;


  /* Initialized to all zeros. */
  *dest = 0;

  for (i = len - 1; i >= 0; i--) {
    /* Shift one byte and add one byte */
    *dest = (*dest  << 0x008) + buf[i];
  }

  return MAN_C_SUCCESS;
} /* end of copy_octet_to_unsigned_int() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	copy_octet_to_signed_int 
 **
 ** 	This routine transfers the ASN1_C_INTEGER from the octet to an signed
 ** 	int.  
 **
 **  FORMAL PARAMETERS:
 **
 **  	int			signed integer to receive the value in the octet.
 **  	octet_string	    	octet that contains an ASN1_C_INTEGER.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS if successful
 **  	otherwise, MAN_C_BAD_PARAMETER
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status copy_octet_to_signed_int (dest,
				     src)

int                 *dest;
octet_string        *src;

{
  int i;
  int len;
  unsigned char *buf;
  
  /*
   * If the destination or source is NULL, we have bad input.
   */
  if ((dest == NULL) ||
      (src == NULL))
    return MAN_C_BAD_PARAMETER;    
  
  if ((src->string == NULL) || (src->data_type != ASN1_C_INTEGER))
    return MAN_C_BAD_PARAMETER;

  len = src->length;
  buf = (unsigned char *)src->string;

  /*
   ** Only signed integers that are 32bits or less are supported.
   */
  if (len == 5)
    return MAN_C_BAD_PARAMETER;
  
  *dest = 0;

  if ((buf[len-1] & 0X80) >> 007) {	
    /* If negative number... */
    for (i = len - 1; i >= 0; i--) {
      *dest = (*dest << 0x008) + (0x000000FF - buf[i]);
    }
/*    *dest = ~(*dest + 1); */
    *dest = ~(*dest);
    
  }
  else {
    for (i = len - 1; i >= 0; i--) {
      /* Shift one byte and add one byte */
      *dest = (*dest << 0x008) + buf[i];
    }
  }
  
  return MAN_C_SUCCESS;
} /* end of copy_octet_to_signed_int() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	copy_unsigned_int_to_octet
 **
 ** 	This routine builds an octet for an unsigned int.   
 **
 **  FORMAL PARAMETERS:
 **
 **  	octet_string	    	octet to contain an ASN1_C_INTEGER.
 **  	unsigned int     	pointer to the unsigned integer.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS if successful
 **  	otherwise, MAN_C_BAD_PARAMETER
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status copy_unsigned_int_to_octet (dest,
				       src)

octet_string 	     *dest;
unsigned int         *src;

{
  /*
   * If the destination or source is NULL, we have bad input.
   */
  if ((dest == NULL) ||
      (src == NULL))
    return MAN_C_BAD_PARAMETER;    

  if ((*src >= 0X00000000) && (*src <= 0X0000007F)) {
    dest->length = 1;
  }
  else if ((*src >= 0X00000080) && (*src <= 0X00007FFF)) {
    dest->length = 2;
  }
  else if ((*src >= 0X00008000) && (*src <= 0X007FFFFF )) {
    dest->length = 3;
  }
  else if ((*src >= 0X00800000) && (*src <= 0X7FFFFFFF)) {
    dest->length = 4;
  }
  else if ((*src >= 0X80000000) && (*src <= 0XFFFFFFFF)) {
    dest->length = 5;
  }
  else
    return MAN_C_BAD_PARAMETER;

  dest->data_type = ASN1_C_INTEGER;
  dest->string = (char *)src;

  return MAN_C_SUCCESS;
} /* end of copy_unsigned_int_to_octet() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	copy_signed_int_to_octet
 **
 ** 	This routine builds an octet for an signed int.   
 **
 **  FORMAL PARAMETERS:
 **
 **  	octet_string	    	octet to contain an ASN1_C_INTEGER.
 **  	int			pointer to the signed integer.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS if successful
 **  	otherwise, MAN_C_BAD_PARAMETER
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status copy_signed_int_to_octet (dest,
				     src)

octet_string 	*dest;
int		*src;

{

  /*
   * If the destination or source is NULL, we have bad input.
   */
  if ((dest == NULL) ||
      (src == NULL))
    return MAN_C_BAD_PARAMETER;    

  dest->length = 4;
  
  dest->data_type = ASN1_C_INTEGER;

  dest->string = (char *)src;

  return MAN_C_SUCCESS;
} /* end of copy_signed_int_to_octet() */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	mom_compare_array
 **
 ** 	This routine compares the first array to the second array. This
 **     is kinda like the MOSS routine.  But we need more accuracy than
 **     equal to or not equal to.  Stolen from mold.c by Ken Chapman and 
 **     modified for the CA project. 
 **
 **  FORMAL PARAMETERS:
 **
 **  	first_array    - Pointer to the first array
 **  	last_array     - Pointer to the last array
 **
 **  RETURN VALUE:
 **
 **	MAN_C_LESS
 **	MAN_C_SUCCESS
 **  	MAN_C_GREATER
 **  	MAN_C_BAD_PARAMETER   A null array was passed - no comparison done
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */

man_status
mom_compare_array(
                  length ,
                  first_array ,
                  last_array
                 )
unsigned int length ;
unsigned int *first_array ;
unsigned int *last_array ;

{
    int index ;
    int ret_value ;

    if ( first_array == NULL || last_array == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    /*
     * Now compare each element.
     */

    for ( index = 0 ; index < length ; index++ )
    {

        ret_value = COMPARE( *first_array, *last_array ) ;

        first_array++ ;
        last_array++ ;

        if ( ret_value != MAN_C_EQUAL )
            return( ret_value ) ;

    }

    return( ret_value ) ;

}  /* end of mom_compare_array() */


/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	find_instance_avl
 **
 ** 	This routine searches an avl for the instance information for a
 **	specified instance oid.
 **
 **  FORMAL PARAMETERS:
 **
 **  	avl	    - pointer avl containing the instance information
 **	oid	    - oid of instance desired
 **	modifier    - modifier field of specified instance
 **	tag	    - tag	"    "      "        "
 **	octet	    - octet     "    "      "        "
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **  	MAN_C_NO_ELEMENT
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
man_status  find_instance_avl(
			      instance_avl,
			      instance_oid,
			      instance_mod,
			      instance_tag,
			      instance_octet
			     )

avl		    *instance_avl;
object_id	    *instance_oid;
unsigned int        *instance_mod;
unsigned int	    *instance_tag;
octet_string	    **instance_octet;

{
object_id   *temp_oid;
int	    last_one;
man_status  status;

    /*
     * Find the element specified by instance_oid.
     */
    status = moss_avl_reset( instance_avl );

    status = moss_avl_point( instance_avl,	    /* start of instance constructore */
			    &temp_oid,
			    instance_mod,
			    instance_tag,
			    instance_octet,
			    &last_one);

    if ((status != MAN_C_SUCCESS) || (*instance_tag != ASN1_C_SEQUENCE))
	return MAN_C_PROCESSING_FAILURE;

    do	{

	status = moss_avl_point( instance_avl,
				&temp_oid,
				instance_mod,
				instance_tag,
				instance_octet,
				&last_one);

	if ERROR_CONDITION( status ) return status;
	if (*instance_tag == ASN1_C_EOC) return MAN_C_NO_ELEMENT;
    }
    while (moss_compare_oid( temp_oid, instance_oid ) != MAN_C_EQUAL);

    return MAN_C_SUCCESS;
}  /* end of find_instance_avl() */


/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **	mom_log
 **
 **     This routine logs an error message in the system log file.  The
 **     routine is available to MOM developer for logging any message from
 **     the MOM that needs to be written to the system log file.
 **
 **  FORMAL PARAMETERS:
 **
 **	p_msg       - pointer to the message to be written to the log.
 **	syslog_code - code needed for syslog call.
 **
 **  RETURN VALUE:
 **
 **	MAN_C_SUCCESS
 **  	MAN_C_NO_ELEMENT
 **
 **  SIDE EFFECTS:
 **
 **     None
 **
 **--
 */
void mom_log(char *p_msg, int syslog_code)
{
char    bigbuf[MAXERRMSG+1];/* Where we copy a message to add a newline*/
                            /* (+1 is for the NULL byte) */
char    *outmsg;            /* Pointer to final message to send */
int     in_msg_len;         /* Computed length of inbound message */

  /* if (it doesn't end in a newline and there is room) */

  if ( ( (in_msg_len = strlen(p_msg)) <  MAXERRMSG ) &&
         ( *(p_msg + in_msg_len - 1)    != '\n'     )     )
    {
    /* add a newline */

    strcpy(bigbuf, p_msg);            /* copy inbound message */
    bigbuf[in_msg_len] = '\n';      /* add '\n' over null byte */
    bigbuf[in_msg_len+1] = '\0';    /* add the '\0' */
    outmsg = bigbuf;                /* New outbound message is here */
    }
  else
    {
    outmsg = p_msg;   /* Just use the original message */
    }

  /* write message to syslog() */

  syslog(syslog_code, outmsg);

  return;
}  /* end of mom_log() */

/* end of utility routines.c */
