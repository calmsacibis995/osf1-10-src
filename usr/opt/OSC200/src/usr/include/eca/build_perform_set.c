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
static char *rcsid = "@(#)$RCSfile: build_perform_set.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:06 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_SET.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the routines to actually perform the SET
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
#include "iso_defs.h"
#include "common.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]validate_instance
**
**	This routine verifies that the OID for setting attributes is a valid
**	attribute OID and it matches all of the settable attributes.
**
**	This routine is called from the perform_set and perform_action routines.
**
**  FORMAL PARAMETERS:
**
**	oid	 - Pointer to OID to validate.
**	instance - Pointer to instance.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS  - if OID is valid.
**	MAN_C_FALSE    - if not valid.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  [[class_prefix]]validate_instance( oid,
					       instance )

object_id	    *oid;
void		    *instance;

{
    unsigned int    attribute;
    man_status	    status;

#ifdef DNA_CMIP_INT
    status = moss_compare_partial_oid(oid, &[[class_prefix]]attr_prefix_DNA, 0, 0, ATTRIBUTES_LENGTH);
    if (status == MAN_C_NOT_EQUAL)
	return MAN_C_FALSE;
#endif /* DNA_CMIP_INT */

    status = [[class_prefix]]get_attr_oid( oid, &attribute );
    if ERROR_CONDITION(status)
	return status;

    switch (attribute)
    {
	/*-insert-code-list-attr-*/
	            return MAN_C_SUCCESS;
    }

    return MAN_C_FALSE;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]attr_writeable
**
**	This routine verifies that the OID being set is writeable. Only
**	characteristic attributes are writeable (or settable in MSL).
**	
**	This routine is called from the perform_set and perform_action routines.
**
**  FORMAL PARAMETERS:
**
**	oid	 - Pointer to OID to validate.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS  - if OID is writeable.
**	MAN_C_FALSE    - if not writeable.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  [[class_prefix]]attr_writeable( oid )

object_id	*oid;

{
    man_status	    status;
    unsigned int    attribute;

    status = [[class_prefix]]get_attr_oid( oid, &attribute );
    if ERROR_CONDITION(status)
	return status;

    /*
     * Only characteristic attributes are writeable.
     */

    switch (attribute)
    {
	/*-insert-code-set-char-*/

	default: 
	    return MAN_C_READ_ONLY_ATTRIBUTE;
    }
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]check_attr_value
**
**	This routine verifies that value of the OID being set is valid.
**	It can be called from either the CREATE or SET routines.
**
**	NOTE: This routine requires modifications to be completed.
**
**  FORMAL PARAMETERS:
**
**	oid	 - Pointer to OID of value to check.
**	modifier - Modifier of value to check.
**	tag	 - Tag of value to check.
**	octet    - Pointer to octet of value to check.
**	modification_list - Pointer to AVL containing attributes to verify.
**			    This is necessary for constructed data types.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS  		      - if OID is valid.
**	MAN_C_INVALID_ATTRIBUTE_VALUE - if not valid.
**	MAN_C_NO_SUCH_ATTRIBUTE_ID    - if no attribute exists.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  [[class_prefix]]check_attr_value( oid,
				  	      modifier,
				  	      tag,
				  	      octet,
				  	      modification_list )

object_id		  *oid;
[[sp]]unsigned int	  modifier;
[[sp]]unsigned int	  tag;
[[sp]]octet_string	  *octet;
[[sp]]avl		  *modification_list;

{
    man_status	    status;
    unsigned int    attribute;

    status = [[class_prefix]]get_attr_oid( oid, &attribute );
    if ERROR_CONDITION(status)
	return status;

    /*
     * Check if datatype and size are valid.
     */

    switch (attribute)
    {
	/*-insert-code-check-datatypes-*/

	default: 
	    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
    }

    switch (attribute)
    {
	/*-insert-code-check-values-*/

	default: 
	    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
    }

    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]set_attribute
**
**	This routine actually sets the class-specific data structure 
**	to the value found in the octet on the set directive.
**
**  FORMAL PARAMETERS:
**
**	attribute_list - AVL of attribute list of all attributes set (or ones that failed).
**	attr_oid       - Pointer to OID to validate.
**	octet	       - Pointer to octet containing value to set.
**	modifier       - Modifier of attribute to set.
**	instance       - Pointer to instance.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS  - if OID is valid.
**	MAN_C_FALSE    - if not valid.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status [[class_prefix]]set_attribute( 
                                                attribute_list,
                                                attr_oid,
			                        octet,
			                        modifier,
			                        instance )

avl			*attribute_list;
object_id		*attr_oid;
[[sp]]octet_string	*octet;
unsigned int 		modifier;
[[class_void_name]]	*instance;

{
    man_status	    status;
    unsigned int    attribute;

    status = [[class_prefix]]get_attr_oid( attr_oid, &attribute );
    if ERROR_CONDITION(status)
	return status;

    switch (attribute)
    {
	/*-insert-code-set-attributes-*/

	default: 
	    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
    }

    return MAN_C_SUCCESS;
}    


man_status  [[class_prefix]]perform_set( unneeded_oid,
					 modification_list,
					 instance,
					 reply_attribute_list,
					 reply_status,
					 action_response_type_oid )

object_id		*unneeded_oid;
[[sp]]avl		*modification_list;
[[sp]]void		*instance;
[[sp]]avl		**reply_attribute_list;
[[sp]]reply_type	*reply_status;
[[sp]]object_id		**action_response_type_oid;

{
    object_id		*oid;
    unsigned int	modifier;
    unsigned int	tag;
    octet_string	*octet;
    int			last_one;
    man_status		status, moss_status;
    unsigned int        success_status = (unsigned int)MAN_C_SUCCESS;
    
    status = moss_avl_init(reply_attribute_list);
    if ERROR_CONDITION(status)
        return status;

    status = moss_avl_reset(modification_list);
    if ERROR_CONDITION(status)
	return status;

    *reply_status = (reply_type) MAN_C_SUCCESS;

    do
    {
	/*
	 * Point to each element.  We are interested in the
	 * object identifier, the modifier value, and the
	 * new value(s) of the attribute.
	 */
	status = moss_avl_point(modification_list,
				&oid,
				&modifier,
				&tag,
				&octet,
				&last_one);
				
	if (status == MAN_C_SUCCESS)
	{
	    /*
	     * Validate oid against instance.
	     */
	    status = [[class_prefix]]validate_instance(oid, instance);
	    if (status != MAN_C_SUCCESS)
		/*
		 * OID is invalid for this instance.  Return the error for this
		 * attribute, and set overall status to MAN_C_SET_LIST_ERROR
		 * because at least one of the requested attributes was not able
		 * to be modified.
		 */
       		status = (man_status) _list_error(oid, 
				    	*reply_attribute_list, 
				    	(unsigned int *) reply_status, 
				    	(reply_type) MAN_C_NO_SUCH_ATTRIBUTE_ID, 
				    	(reply_type) MAN_C_SET_LIST_ERROR);
	    else
            {
		status = [[class_prefix]]attr_writeable(oid);
		if (status != MAN_C_SUCCESS)
		{
		    /*
		     * OID is not writeable.  Return the error for this
		     * attribute, and set overall status to MAN_C_SET_LIST_ERROR
		     * because at least one of the requested attributes was not able
		     * to be modified.
		     */
		    status = (man_status) _list_error(oid, 
						*reply_attribute_list, 
						(unsigned int *) reply_status, 
						(reply_type) MAN_C_READ_ONLY_ATTRIBUTE, 
				        	(reply_type) MAN_C_SET_LIST_ERROR);
		}
		else
		{
		    status = [[class_prefix]]check_attr_value( oid,
						   [[sp]]modifier,
						   [[sp]]tag,
						   [[sp]]octet,
						   [[sp]]modification_list );
		    if (status == MAN_C_SUCCESS)
		    {
		        /*
			 * Attribute and value is valid, go ahead set the value.
			 */
			status = [[class_prefix]]set_attribute( modification_list, oid, octet, modifier, instance );
			if ERROR_CONDITION(status)
                        {
                          moss_status = moss_avl_add( 
                                                 *reply_attribute_list,
                                                 oid,
                                                 MAN_C_INVALID_ATTRIBUTE_VALUE,
                                                 tag,
                                                 octet );
			  return status;
                        }
#ifdef SNMP
    /* always return attribute value for SNMP */

			status = moss_avl_backup( modification_list );
			if ERROR_CONDITION(status) return status;
			status = moss_avl_copy( *reply_attribute_list,
						modification_list,
						FALSE,
						oid,
						&success_status,
						&last_one);
                        if ERROR_CONDITION(status) return status;
#endif
		    }
		    else  
			status = (man_status) _list_error(oid, 
						*reply_attribute_list, 
						(unsigned int *) reply_status, 
						(reply_type) MAN_C_INVALID_ATTRIBUTE_VALUE,
				        	(reply_type) MAN_C_SET_LIST_ERROR);
		}
            }	
        }
        else if (status == MAN_C_NO_ELEMENT)
      	    return MAN_C_SUCCESS;
    } while (!last_one);

    return MAN_C_SUCCESS;
}			 
