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
static char *rcsid = "@(#)$RCSfile: build_perform_create_instance.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:22:34 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_CREATE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the routines to actually perform the CREATE
**	function for the [[class_name]] class.
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
#include "extern_nil.h"
#include "moss_dna.h"
#include "common.h"
[[extern_common]]

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]map_oid_to_arg
**
**	This routine compares the incoming create argument OID against all 
**	argument OIDs. If a match is found, it returns a unique number
**	(from COMMON.H).
**
**  FORMAL PARAMETERS:
**
**	oid 	 - pointer to argument object id to compare
**	argument - address of unique argument number
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 		   - if match found
**	MAN_C_NO_SUCH_ATTRIBUTE_ID - if no match found
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
/*-insert-code-map-oid-*/

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]check_all_attrs
**
**	This routine checks each incoming attribute in the modification list
**	AVL by calling the common set and create check routine 
**	[[class_prefix]]check_attr_value for each attribute.
**
**  FORMAL PARAMETERS:
**
**	modification_list    - pointer to AVL of attributes to check.
**	reply_attribute_list - pointer to AVL of possible error return replies.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 		 - if all attributes are valid
**	Any error status from [[class_prefix]]check_attr_value 
**      (e.g. MAN_C_INVALID_ATTRIBUTE_VALUE, MAN_C_NO_SUCH_ATTRIBUTE_ID)
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
/*-insert-code-add-check-attributes-*/
/* End of [[class_prefix]]map_oid_to_arg() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]perform_create
**
**	This routine performs the class-specific create instance function.
**	It allocates the class-specific instance structure and UID. It
**	also stores any attributes (create arguments) in the instance
**	structure. It then calls [[class_prefix]]add_new_instance to 
**	store the instance. The object instance is copied in the class_def
**	structure to use for instance matching.
**
**  FORMAL PARAMETERS:
**
**	object_instance  - Pointer to object instance.
**	instance_octet   - Pointer to octet containing instance data.
**	instance 	 - Pointer to string containing instance name.
**	instance_length	 - Integer containing length of instance name.
**	ref_instance_ptr - Address of pointer to return instance structure.
**	attribute_list	 - AVL containing list of create arguments.
**	return_avl	 - Address of pointer to return AVL create responses.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 		 - if successful create.
**	Any error status during create.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  [[class_prefix]]perform_create( 	
					   object_instance,
					   instance_octet,
					   instance,
					   instance_length,
					   ref_instance_ptr,
					   attribute_list,
					   return_avl )

avl			*object_instance;
octet_string		*instance_octet;
char			*instance;
int			instance_length;
[[class_void_name]]	**ref_instance_ptr;
avl			*attribute_list;
avl			**return_avl;

{
    object_id		*oid;
    unsigned int	modifier;
    unsigned int	tag;
    octet_string	*octet;
    uid			*temp_uid;
    int			last_one;
    int			last;
    unsigned int 	argument;
    man_status		add_status, status;
    [[class_name]]_DEF	*new_[[class_name]]_instance;
    
    new_[[class_name]]_instance = ([[class_name]]_DEF *) malloc( sizeof( [[class_name]]_DEF ));
    if (new_[[class_name]]_instance == NULL)
        return MAN_C_INSUFFICIENT_RESOURCES;

    memset( (void *)new_[[class_name]]_instance, '\0', sizeof( [[class_name]]_DEF ));

    *ref_instance_ptr = new_[[class_name]]_instance;

    /*
     * Allocate a new instance name and set up the length.
     */

    /*-insert-code-identifier-attr-*/

  /*
   * Create the UID.
   */
#ifdef VMS
    temp_uid = &new_[[class_name]]_instance->instance_uid;
    status = (man_status) moss_get_uid( &temp_uid );
    if (ERROR_CONDITION(status))
	{
	free( new_[[class_name]]_instance );
	free( new_[[class_name]]_instance->instance_name );
        return status;
	}
#endif /* VMS */

   /*
    * Save a copy of the object instance to use for instance name matching.
    */
	
    moss_avl_init( &new_[[class_name]]_instance->object_instance );
    moss_avl_reset( object_instance );
    status = moss_avl_copy( new_[[class_name]]_instance->object_instance,
		   object_instance,
		   TRUE,
		   NULL,
		   NULL,
		   NULL );		   
    if (ERROR_CONDITION(status))
	{
	free( new_[[class_name]]_instance );
        return status;
	}

  /*
   * Check to see if the attribute list exists.
   */
  status = moss_avl_reset( attribute_list );

  if (status == MAN_C_SUCCESS)
      status = moss_avl_point( attribute_list, NULL, NULL, NULL, NULL, NULL );

  /* 
   * If no AVL elements were found, no need to read through the attribute list.
   */
  if (status == MAN_C_SUCCESS) 
    {
    moss_avl_reset( attribute_list );

    /* 
     * Get the characteristic attribute values
     */
    do {
      status = moss_avl_point( attribute_list, &oid, &modifier, &tag, &octet, NULL );
      if (status == MAN_C_SUCCESS)
        {
	status = [[class_prefix]]map_oid_to_arg( oid, &argument );
        if (status == MAN_C_SUCCESS) 

	 switch( argument )
	  {
	  /*-insert-code-map-oid-list-*/
	  default:
	    status = MAN_C_NO_SUCH_ATTRIBUTE_ID;
	  }
        } 
    } while (!ERROR_CONDITION( status ));
  } 

  /* 
   * No attributes were specified.
   */
  if (status == MAN_C_NO_ELEMENT)
      status = MAN_C_SUCCESS;

  /*-insert-code-add-new-instance-call-*/

  /*
   * If any errors occurred, deallocate any new structures and return the error.
   */
  if ERROR_CONDITION( status )
      {
      free( new_[[class_name]]_instance->instance_name );
      moss_avl_free( &new_[[class_name]]_instance->object_instance, TRUE );
      free( new_[[class_name]]_instance );
      }
      
  return status;
} /* End of [[class_prefix]]perform_create() */
