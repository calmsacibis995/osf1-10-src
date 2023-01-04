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
static char *rcsid = "@(#)$RCSfile: ipnettomediaentry_get_instance.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:21:37 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**	ipNetToMediaEntry_GET_INSTANCE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**	It provides the entry point for the GET_INSTANCE function
**	for the ipNetToMediaEntry class.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  02-Apr-1993       
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"

#ifdef SNMP
#include "moss_inet.h"
#include "inet_mom_specific.h"
#else
#include "moss_dna.h"
#endif

#include "common.h"
#include "extern_common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ipNetToMediaEntry_build_instance_avl 
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
static man_status ipNetToMediaEntry_build_instance_avl( object_instance, 
						      specific_object_instance,
			        		      specific_instance )

avl *object_instance;
avl **specific_object_instance;
ipNetToMediaEntry_DEF *specific_instance;

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
   * Move down the avl to the ipNetToMediaEntry's name, backup one octet,
   * remove it and replace it with the instance name. Note, we store the
   * instance name as a simple zero-terminated string, but we must return it as
   * a SimpleName which as two extra bytes.
   *
   * NOTE: DNA CMIP requires the number of the identifier attribute ID to be 1.
   */
  status = moss_avl_find_item( *specific_object_instance,
#ifdef DNA_CMIP_OID 
                               &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_SNP,
#endif /* SNMP_OID */
/** Multiple identifier OIDs found. Verify the correct OID **/
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

  octet.data_type = ASN1_C_INTEGER; /** Verify datatype **/
  octet.string = (char *) specific_instance->instance_name;
  octet.length = specific_instance->instance_name_length;
  
  status =(man_status) moss_avl_add(*specific_object_instance,		/* avl_handle */
#ifdef DNA_CMIP_OID
                             &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                             &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_SNP,
#endif /* SNMP_OID */
				    (unsigned int) MAN_C_SUCCESS,			/* modifier */
                             (unsigned int) ASN1_C_INTEGER, /** Verify data type **/ 
				    &octet);				/* octet */
/** Multiple identifier OIDs found. Verify the correct OID **/

  if ERROR_CONDITION( status )
    return status;
  
#ifdef MOMGENDEBUG
  printf("------- Object instance from BUILD_INSTANCE_AVL --------- \n\n");
  dbg_print_avl(*specific_object_instance);
  printf("------- end of instance from BUILD_INSTANCE_AVL --------- \n\n");
#endif /* MOMGENDEBUG */

return MAN_C_SUCCESS;
} /* End of ipNetToMediaEntry_build_instance_avl */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ipNetToMediaEntry_get_instance
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
**	      are needed in the ipNetToMediaEntry_get_next_match routine.
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

man_status ipNetToMediaEntry_get_instance( object_class,
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
ipNetToMediaEntry_DEF	**specific_instance;
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
    
	status = (man_status) refresh_ipNetToMediaEntry_list(new_ipNetToMediaEntry_header); if ERROR_CONDITION(status) return status;
        context->ipNetToMediaEntry_PTR = new_ipNetToMediaEntry_header;

        status = MAN_C_TRUE;
    }
    else
    {
	/*
	 * This isn't the first name we've been looking for.
	 * Restore old context; determine if we are at "end-of-list"
	 */
	context = *return_context;
	if (context->ipNetToMediaEntry_PTR == new_ipNetToMediaEntry_header)
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
     *  For GETNEXT, we are done!  Return the specific instance
     */

    if (build_avl != TRUE)
    {

        *specific_instance = context->ipNetToMediaEntry_PTR->next;
        *object_uid = &(*specific_instance)->instance_uid;
        if (context->search_name != NULL)
            free(context->search_name);
        if (context != NULL)
            free(context);

        return MAN_C_SUCCESS;
    }

    /*
     * For GET, continue on.  Find a match.  Return a pointer to the
     * specific instance.
     */

    else
    {
        status = (man_status) ipNetToMediaEntry_find_matching_instance ( 
		                object_instance, context);
        if (status == MAN_C_FALSE)
        {
           *return_context = NULL;
           *more_instances = FALSE;

           if (context->search_name != NULL)
              free(context->search_name);
              free(context);

           return MAN_C_FALSE;
        }

        *specific_instance = context->ipNetToMediaEntry_PTR;

    /*
     * Set the object UID from the instance.
     */

        *object_uid = &(*specific_instance)->instance_uid;

        status = moss_avl_init (specific_object_instance);
        if ERROR_CONDITION (status)
           return status;
        status = moss_avl_reset (object_instance);
        if ERROR_CONDITION (status)
           return status;

        status = moss_avl_copy (*specific_object_instance, /* avl_handle_dest */
                               object_instance,
                               FALSE,
                               NULL,
                               NULL,
                               NULL,
                               NULL);
        if ERROR_CONDITION (status)
           return status;
    }

    return MAN_C_SUCCESS;
} /* End of ipNetToMediaEntry_get_instance */


man_status ipNetToMediaEntry_find_matching_instance (
                                   instance_avl,
                                   context
                                  )

avl                 *instance_avl;
GET_CONTEXT_STRUCT  *context;

{
/** note to developers: this routine assumes that instance is an integer
table index.  modify the ordering algorithm appropriately for other
    datatypes **/

    man_status              status;
    octet_string            *instance_octet = NULL;
    ipNetToMediaEntry_DEF             *header_p = NULL;
    int                     inst_index ;
    unsigned int            itag, imod;
    int                     i,j;
    char *instance_string, *ch;

    object_id tmpoid;
    u_int tmp_oid[32];
    object_id curroid;
    u_int curr_oid[32];
    object_id inputoid;
    u_int input_oid[32];

    status = find_instance_avl( instance_avl,
#ifdef DNA_CMIP_OID
                                        &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                                        &ipNetToMediaEntry_ATTR_ipNetToMediaIfIndex_SNP,
#endif /* SNMP_OID */
/** Multiple identifier OIDs found. Verify the correct OID **/
                                        &imod,
                                        &itag,
                                        &instance_octet );
            if (status != MAN_C_SUCCESS) {
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }

    /* First identifier is ipNetToMediaIfIndex */

    inputoid.count = 5;
    bcopy(instance_octet->string,&input_oid[0],4);

    /* Next identifier is ipNetToMediaNetAddress  */

    status = moss_avl_point(instance_avl,NULL,NULL,NULL,&instance_octet,NULL);
    if (status != MAN_C_SUCCESS)
        return(MAN_C_NO_SUCH_OBJECT_INSTANCE);
    if ((instance_octet->length == 0) || (instance_octet->string) == NULL)
        return(MAN_C_NO_SUCH_OBJECT_INSTANCE);

    ch = (char *)instance_octet->string;
    for (i = 1; i < 5; i++){
         input_oid[i] = *ch & 0xff;
         ch++;
    }

    inputoid.value = &input_oid[0];

    /* get the instance name from the each link list, split
     * and put into tmpoid and compare
    */

    context->ipNetToMediaEntry_PTR = context->ipNetToMediaEntry_PTR->next;

    tmpoid.count = 5;
    while (context->ipNetToMediaEntry_PTR != new_ipNetToMediaEntry_header) {

         ch = (char *)context->ipNetToMediaEntry_PTR->instance_name;
         bcopy(ch, &tmp_oid[0], 4);
         for (i = 1; i < 5; i++)
              ch++;
         for (i = 1; i < 5; i++){
         tmp_oid[i] = *ch & 0xff;
         ch++;
         }
         tmpoid.value = &tmp_oid[0];


         if ((compare_oid(&tmpoid,&inputoid) == MOM_C_EQUAL_TO))
            {
             return MAN_C_TRUE;
            }
         context->ipNetToMediaEntry_PTR = context->ipNetToMediaEntry_PTR->next;
    }/* while */

    return MAN_C_FALSE;

} /* End of ipNetToMediaEntry_find_matching_instance */

/* End of ipNetToMediaEntry_get_instance.c */
