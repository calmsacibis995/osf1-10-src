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
static char *rcsid = "@(#)$RCSfile: iprouteentry_get_instance.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:21:50 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**	ipRouteEntry_GET_INSTANCE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**	It provides the entry point for the GET_INSTANCE function
**	for the ipRouteEntry class.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  01-Apr-1993       
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"

#ifdef SNMP
#include "moss_inet.h"
#else
#include "moss_dna.h"
#endif

#include "common.h"
#include "extern_common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ipRouteEntry_build_instance_avl 
**
**	This routine builds an avl containing the entire object instance
**      name given the short hand instance name as a zero terminated string. 
**
**	Example: If object_instance contains "HELENA <null> **", and
**		 specific_instance pointed to the "test" instance. the
**		 specific_object_instance returned would be "HELENA <null> test" 
**		 
**  FORMAL PARAMETERS:
**
**	object_instance - AVL containing the object instance. The assumption is
**			  that the last octet of the object_instance avl is a 
**			  wild card and can be removed and replaced with the
**			  instance name forming the full instance name.
**	 
**	specific_object_instance - Address of pointer to the returned object_instance avl. 
**	 
**	specific_instance - The pointer to the instance data.
**
**  RETURN VALUE:
**
**      MAN_C_SUCCESS 		 - if success
**	MAN_C_PROCESSING_FAILURE - if error.
**--
*/
static man_status ipRouteEntry_build_instance_avl( object_instance, 
						      specific_object_instance,
			        		      specific_instance )

avl *object_instance;
avl **specific_object_instance;
ipRouteEntry_DEF *specific_instance;

{
  unsigned int  	modifier;
  unsigned int  	tag;
  octet_string 		*junk_octet;
  octet_string 		octet;
  man_status		status;
  int			last = FALSE;

  status = moss_avl_init(specific_object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  status = moss_avl_reset(object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  /* 
   *  Copy the entire object_instance avl.
   */ 
  do {
    status = moss_avl_copy(*specific_object_instance,	/* avl_handle_dest */
			   object_instance,		/* avl_handle_source */
			   FALSE,			/* reset_dest */
			   NULL,			/* dest_oid */
			   NULL,			/* dest_modifier */
			   &last);			/* source_last_one */
    
  } while ( (status == MAN_C_SUCCESS) && (last == FALSE));
    
  /*
   * Move down the avl to the ipRouteEntry's name, backup one octet,
   * remove it and replace it with the instance name. Note, we store the
   * instance name as a simple zero-terminated string, but we must return it as
   * a SimpleName which as two extra bytes.
   *
   * NOTE: DNA CMIP requires the number of the identifier attribute ID to be 1.
   */
  status = moss_avl_find_item( *specific_object_instance,
#ifdef DNA_CMIP_OID 
                               &ipRouteEntry_ATTR_ipRouteDest_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &ipRouteEntry_ATTR_ipRouteDest_SNP,
#endif /* SNMP_OID */
				/** Need to supply identifier attribute OID **/
                                          /** &ATTR_NAME_DNA **/
			       &modifier,
			       &tag,
			       &junk_octet);
  if ERROR_CONDITION( status )
    {
#ifdef MOMGENDEBUG
    printf("\n*** Identifier attribute not found in object instance. For DNA_CMIP, identifier attr must be 1. *** \n");
#endif /* MOMGENDEBUG */
    return status;
    }  

  status = moss_avl_backup(*specific_object_instance);
  if ERROR_CONDITION( status )
    return status;
  
  status = moss_avl_remove(*specific_object_instance);
  if ERROR_CONDITION( status )
    return status;

  octet.data_type = INET_C_SMI_IP_ADDRESS;
  octet.string = specific_instance->instance_name;
  octet.length = specific_instance->instance_name_length;
  
  status =(man_status) moss_avl_add(*specific_object_instance,		/* avl_handle */
#ifdef DNA_CMIP_OID
                             &ipRouteEntry_ATTR_ipRouteDest_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                             &ipRouteEntry_ATTR_ipRouteDest_SNP,
#endif /* SNMP_OID */
				    (unsigned int) MAN_C_SUCCESS,			/* modifier */
                             (unsigned int) INET_C_SMI_IP_ADDRESS,
				    &octet);				/* octet */

  if ERROR_CONDITION( status )
    return status;
  
#ifdef MOMGENDEBUG
  printf("------- Object instance from BUILD_INSTANCE_AVL --------- \n\n");
  dbg_print_avl(*specific_object_instance);
  printf("------- end of instance from BUILD_INSTANCE_AVL --------- \n\n");
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* End of ipRouteEntry_build_instance_avl */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ipRouteEntry_get_instance
**
**	This routine performs all of the work of returning a matching instance
**	structure, if any. A context structure is allocated per request and
**	deallocated in the caller of this routine.
**
**	The more_instances flag must be accurately as each reply is returned.
**	The MOM may need to maintain two instance structures in its context
**	block so that this flag on each reply.
**
**	NOTE: This routine may need minor modifications if the data resides
**	      outside of this MOM. If this is the case, most of the changes
**	      are needed in the ipRouteEntry_get_next_match routine.
**
**  FORMAL PARAMETERS:
**
**	object_class	  - OID of class to match.
**	object_instance   - AVL containing instance name to match.
**	scope		  - Matching information for instance qualification. (Not currently used).
**	specific_instance - Address of pointer to class specific structure for instance match.
**	specific_object_instance - Address of pointer to AVL containing new object
**			    instance name for this matched instance (done in build_instance_avl).
**	object_uid	  - Address of pointer to UID for this instance match.
**	more_instances	  - Address of flag indicating more instances or not.
**	return_context    - Address of pointer to context structure.
**
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 		 - if match found
**	MAN_C_PROCESSING_FAILURE - if no match found
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/

man_status ipRouteEntry_get_instance( object_class,
				  	 object_instance,
				  	 iso_scope,
				  	 specific_instance,
				  	 specific_object_instance,
				  	 object_uid,
				  	 more_instances,
				  	 return_context,
					 build_avl	 )

object_id		*object_class;
avl			*object_instance;
scope			iso_scope;
ipRouteEntry_DEF	**specific_instance;
avl			**specific_object_instance;
uid			**object_uid;
int			*more_instances;
GET_CONTEXT_STRUCT	**return_context;
int			build_avl;
{
    int 	    length;
    char 	    *cptr;
    int		    match_found = FALSE;
    unsigned int    tag;
    octet_string    *octet;
    man_status	    status;
    GET_CONTEXT_STRUCT	    *context;

    /*
     * Check to see if this is the first time being called.
     */

    if (*return_context == NULL)
    {
	/*
	 * Allocate space for context block.
	 */
	context = (GET_CONTEXT_STRUCT *) malloc(sizeof(GET_CONTEXT_STRUCT));
	if (context == NULL)
	    return MAN_C_INSUFFICIENT_RESOURCES;
	memset( (void *)context, '\0', sizeof(GET_CONTEXT_STRUCT) );

	context->first_time = TRUE;

	*return_context = context;
    
	status = (man_status) refresh_ipRouteEntry_list(new_ipRouteEntry_header); if ERROR_CONDITION(status) return status;
        context->ipRouteEntry_PTR = new_ipRouteEntry_header;

	/*
	 * Find the first matching name, if any.
	 */
        if ( build_avl == TRUE )
	   status = (man_status) ipRouteEntry_get_next_match(object_instance,
							     iso_scope,
							     context);
        else
          status = MAN_C_TRUE;
    }
    else
    {
	/*
	 * This isn't the first name we've been looking for.
	 * Restore old context; determine if we are at "end-of-list"
	 */
	context = *return_context;
	if (context->ipRouteEntry_PTR == new_ipRouteEntry_header)
	    status = MAN_C_FALSE;
	else
	    status = MAN_C_TRUE;
    }

    /*
     * If we have no match, return MAN_C_FALSE and free context block.
     */
    if (status == MAN_C_FALSE)
    {
	*return_context = NULL;
	*more_instances = FALSE;

	if (context->search_name != NULL)
	    free(context->search_name);
	free(context);

	return MAN_C_FALSE;
    }

    /*
     * We have a match.  Set the pointer to the specific instance.
     * Set the object UID from the instance.
     */

    if (build_avl != TRUE)
        *specific_instance = context->ipRouteEntry_PTR->next; 
    else
        *specific_instance = context->ipRouteEntry_PTR; 

    *object_uid = &(*specific_instance)->instance_uid;

    /*
     *  For GETNEXT, we are done!  Return the specific instance
     */
    if ( build_avl != TRUE)
    {
      if (context->search_name != NULL)
         free(context->search_name);
      if (context != NULL)
         free(context);

      return MAN_C_SUCCESS;
    }
 
    /*
     * For GET, continue on.  Find a match.
     * Build the complete instance name AVL, since build_avl is TRUE!
     */

    if ((*specific_instance)->object_instance != (avl *) NULL)
	{
	if (*specific_object_instance == NULL)
	    moss_avl_init( specific_object_instance );
	moss_avl_reset((*specific_instance)->object_instance );
	status = moss_avl_copy( *specific_object_instance,
			        (*specific_instance)->object_instance,
			   	TRUE,			/* reset_dest */
			   	NULL,			/* dest_oid */
			   	NULL,			/* dest_modifier */
			   	NULL);			/* source_last_one */
	}
    else
	{
        status = ipRouteEntry_build_instance_avl( object_instance, 
						     specific_object_instance, 
						     *specific_instance );
        if ERROR_CONDITION( status )
            return status;
	}


    /*
     * Read ahead to determine if there is another match since more_instances
     * must be set accordingly when the current is returned.
     */

    status = (man_status) ipRouteEntry_get_next_match(object_instance,
							 iso_scope,
							 context);
    if (status == MAN_C_TRUE)
	*more_instances = TRUE;
    else
	*more_instances = FALSE;

    return MAN_C_SUCCESS;
} /* End of ipRouteEntry_get_instance */

/* End of ipRouteEntry_get_instance.c */
