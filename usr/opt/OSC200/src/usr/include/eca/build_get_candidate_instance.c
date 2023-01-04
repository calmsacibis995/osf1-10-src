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
static char *rcsid = "@(#)$RCSfile: build_get_candidate_instance.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:21:26 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	[[class_prefix]]GET_INSTANCE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the GET_INSTANCE function
**	for the [[class_name]] class.
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

#ifdef SNMP
#include "moss_inet.h"
#else
#include "moss_dna.h"
#endif

#include "common.h"
[[extern_common]]

/*-insert-code-instance-avl-*/

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]get_instance
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
**	      are needed in the [[class_prefix]]get_next_match routine.
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

man_status [[class_prefix]]get_instance( object_class,
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
[[class_void_name]]	**specific_instance;
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
    
	/** Remove this if not using the queue instance header. **/
        context->[[class_name_ptr]] = new_[[class_name]]_header;

	/*
	 * Find the first matching name, if any.
	 */
        if ( build_avl == TRUE )
	   status = (man_status) [[class_prefix]]get_next_match(object_instance,
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
	if (context->[[class_name_ptr]] == new_[[class_name]]_header)
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
      *specific_instance = context->[[class_name_ptr]]->next; 
    else
      *specific_instance = context->[[class_name_ptr]]; 

    *object_uid = &(*specific_instance)->instance_uid;

    /*
     *  For GETNEXT, we are done!  Return the specific instance
     */

    if (build_avl != TRUE)
    {
      if (context->search_name != NULL)
         free(context->search_name);
      if (context != NULL)
         free(context);

      return MAN_C_SUCCESS;
    }

    /*
     *  Continue on for GET operation
     */

    /*
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
        status = [[class_prefix]]build_instance_avl( object_instance, 
						     specific_object_instance, 
						     *specific_instance );
        if ERROR_CONDITION( status )
            return status;
	}


    /*
     * Read ahead to determine if there is another match since more_instances
     * must be set accordingly when the current is returned.
     */

    status = (man_status) [[class_prefix]]get_next_match(object_instance,
							 iso_scope,
							 context);
    if (status == MAN_C_TRUE)
	*more_instances = TRUE;
    else
	*more_instances = FALSE;

    return MAN_C_SUCCESS;
} /* End of [[class_prefix]]get_instance() */

/* End of [[class_prefix]]get_instance.c */
