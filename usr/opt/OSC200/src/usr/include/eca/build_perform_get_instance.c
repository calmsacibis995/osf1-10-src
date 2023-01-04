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
static char *rcsid = "@(#)$RCSfile: build_perform_get_instance.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:58:33 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_GET
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the routines to actually perform the GET
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
**  TEMPLATE HISTORY:
**
**--
*/

#include "man_data.h"
#include "man.h"
#include "moss.h"
#include "iso_defs.h"
#include "common.h"


void [[class_prefix]]get_list_item( attribute,
			            instance,
			            reply_attribute_list )

int		    attribute;
void		    *instance;
avl		    *reply_attribute_list;

{
    octet_string    octet;
    man_status	    status;

    switch (attribute)
    {
	/*-insert-code-get-items-instance-*/
    }
}


man_status  [[class_prefix]]get_all_attributes( instance,
			                        reply_attribute_list )

void	    *instance;
avl	    *reply_attribute_list;

{

    /*-insert-code-get-all-attributes-*/

    return MAN_C_SUCCESS;
}

man_status  [[class_prefix]]get_attribute_value( attr_oid,
				                 instance,
				                 reply_attribute_list,
				                 reply_status )

object_id	*attr_oid;
void		*instance;
avl		*reply_attribute_list;
unsigned int	*reply_status;

{
    man_status	status;
    int		group;
    int		attribute;

    /*
     * Do we want all attributes in a group? 
     */

    status = moss_compare_partial_oid( attr_oid, &univ_attr_oid, 0, 0, ATTRIBUTES_LENGTH);
    if (status == MAN_C_EQUAL)
        {
	/*
	 * Yes, we want an entire group.
	 */
	status = moss_compare_oid( attr_oid, &univ_all_status_attr_oid );
    	if (status == MAN_C_EQUAL) 
        {
	    /*-insert-code-list-status-*/
	} else {
	status = moss_compare_oid( attr_oid, &univ_all_counter_attr_oid );
        if (status == MAN_C_EQUAL)
    	{
    	    /*-insert-code-list-counters-*/
    	} else {
	status = moss_compare_oid( attr_oid, &univ_all_id_attr_oid );
	if (status == MAN_C_EQUAL)
	{
            /*-insert-code-list-id-*/
	} else {
	status = moss_compare_oid( attr_oid, &univ_all_char_attr_oid );
        if (status == MAN_C_EQUAL)
	{
	    /*-insert-code-list-char-*/
	} else {
	status = moss_compare_oid( attr_oid, &univ_all_attr_oid );
        if (status == MAN_C_EQUAL)
	{
	    [[class_prefix]]get_all_attributes(instance, reply_attribute_list);
	} else {
		_list_error( attr_oid, 	
			    reply_attribute_list, 
			    (unsigned int *) reply_status, 
			    (reply_type) MAN_C_NO_SUCH_ATTRIBUTE_ID, 
			    (reply_type) MAN_C_GET_LIST_ERROR);
	       }}}}}  /* end of all elses */
	}
    /*
     * Do we want a specific attribute?
     */
    else
    {
        status = moss_compare_partial_oid( attr_oid, &univ_attr_oid, 0, 0, ATTRIBUTES_LENGTH);

        if (status == MAN_C_EQUAL)
        {
	    status = _moss_get_oid_last(attr_oid, &attribute);
	    if ERROR_CONDITION(status)
	        return status;

	    switch(attribute)
	    {
		/*-insert-code-list-attr-*/
		default:
		    _list_error(attr_oid, 
			       reply_attribute_list, 
    		               (unsigned int *) reply_status, 
			       (reply_type) MAN_C_NO_SUCH_ATTRIBUTE_ID, 
			       (reply_type) MAN_C_GET_LIST_ERROR);
	    }
        }
	else
	    status = _list_error(attr_oid, 
				reply_attribute_list, 
				(unsigned int *) reply_status, 
				(reply_type) MAN_C_NO_SUCH_ATTRIBUTE_ID, 
				(reply_type) MAN_C_GET_LIST_ERROR);
    }
    return status;
}

man_status  [[class_prefix]]perform_get( unneeded_oid,
					 attribute_identifier_list,
					 instance,
					 reply_attribute_list,
					 reply_status )

object_id	*unneeded_oid;
avl		*attribute_identifier_list;
void		*instance;
avl		**reply_attribute_list;
reply_type	*reply_status;

{
    object_id	    *oid;
    int		    last_one;
    man_status	    status;

    status = moss_avl_init(reply_attribute_list);
    if ERROR_CONDITION(status)
        return status;

    status = moss_avl_reset(attribute_identifier_list);
    if ERROR_CONDITION(status)
	return status;

    *reply_status = MAN_C_SUCCESS;
    do
    {
	/*
	 * Point to each element.  We are only interested in the
	 * object identifier of the attribute.
	 */
	status = moss_avl_point(attribute_identifier_list, &oid, NULL, NULL, NULL, &last_one);
	if ((ERROR_CONDITION(status)) && (status != MAN_C_NO_ELEMENT))
	    return status;

	if (status == MAN_C_NO_ELEMENT)
	    /*
	     * Get all the attributes if no specific ones are requested
	     */
	    [[class_prefix]]get_all_attributes(instance, *reply_attribute_list);
	else
	    [[class_prefix]]get_attribute_value(oid, instance, *reply_attribute_list, reply_status);
    } while (!last_one);
 
    return MAN_C_SUCCESS;
}
