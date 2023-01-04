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
static char *rcsid = "@(#)$RCSfile: build_perform_table_getnext.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:23 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_TABLE_GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**
**      It provides the routines to perform the get-next function for the
**      [[class_name]] class.
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
#include "extern_common.h"



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]next_oid
**
**      This routine returns the oid for next lexicographic attribute
**      based on the oid supplied.
**
**  FORMAL PARAMETERS:
**
**      attr_oid    - oid of attribute specified in request
**      next_oid    - oid of next lexicographic attribute
**
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_NO_SUCH_ATTRIBUTE_ID
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]next_oid(
                         attr_oid,
                         ret_oid
                        )

object_id   *attr_oid;
object_id   **ret_oid;

{

man_status  status;

            /*-insert-code-getnext-attribute-oid-*/

    if (status == MAN_C_EQUAL) return MAN_C_SUCCESS;
    return status;

} /* End of [[class_prefix]]next_oid() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]construct_table_instance
**
**      This routine constructs an instance avl when none is provided
**      in the getnext request.
**
**  FORMAL PARAMETERS:
**
**      instance          - pointer to avl to hold constructed instance name
**	instance_elem     - pointer to class structure with required instance
**
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]construct_table_instance(
                                         instance,
				         instance_elem
                                        )

avl         *instance;
[[class_void_name]] *instance_elem;

{
man_status      status = MAN_C_SUCCESS;
octet_string    instance_octet;

/** Note to developers: This routine may be replaced if the data resides
    outside the MOM.  Also, this routine only deals with one level in the
    containment hierarchy.  If the hierarchy is more complex you will need
    to add the appropriate levels in the avl construction
 **/

/*  Roll up the avl element having the "entered" instance value to the
    next one.  Do this by backing up to the "entered" instance value,
    remove it, and add the next instance value, passed as a parameter
 */
 
         status = moss_avl_backup (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

         status = moss_avl_remove (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

	    /* create a datatype octet for the table instance */
	    /** note to developers: this code assumes an integer table index.
	        change the ASN1_C_INTEGER to the appropriate datatype if the
		table is indexed by a different datatype **/

    instance_octet.length = instance_elem->instance_name_length;
    instance_octet.data_type = ASN1_C_INTEGER;
    instance_octet.string = instance_elem->instance_name;

    status = moss_avl_add(  instance,
                      /*-insert-code-name-oid-*/
                            (int)MAN_C_SUCCESS,
                            ASN1_C_INTEGER,
                            &instance_octet);
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    return  status;

} /* End of [[class_prefix]]construct_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]find_next_instance
**
**      This routine gets the next instance based on the instance supplied.
**
**  FORMAL PARAMETERS:
**
**	inst_octet  - pointer to octet from instance avl matching the
**		      attribute of interest that is supplied in the request
**
**	instance    - pointer to next lexicographical instance is returned 
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_FAILURE
**	MAN_C_MISSING_ATTRIBUTE_VALUE
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]find_next_instance(
				   inst_octet,
				   instance
				  )

octet_string	    *inst_octet;
[[class_void_name]] **instance;

{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

man_status          status;
unsigned int        current_instance, candidate_instance, ret_instance;
[[class_void_name]] *return_instance = NULL;
octet_string	    candidate_octet;

    /* get requested instance */
    status = copy_octet_to_unsigned_int( &current_instance, inst_octet );
    if (status == MAN_C_BAD_PARAMETER)  
        return MAN_C_MISSING_ATTRIBUTE_VALUE;

    *instance = new_[[class_name]]_header;	/* point to top of class structure list */

    candidate_octet.data_type = ASN1_C_INTEGER;

    /* Search for the next lexicographic occurence of the particular
     | instance provided in the instance AVL
     */
    while ((*instance)->next != new_[[class_name]]_header) {
	candidate_octet.length = (*instance)->next->instance_name_length;
	candidate_octet.string = (*instance)->next->instance_name;
	copy_octet_to_unsigned_int( &candidate_instance, &candidate_octet );
	if (candidate_instance <= current_instance)
	    *instance = (*instance)->next;
	else if (return_instance == NULL) {
	    ret_instance = candidate_instance;
	    return_instance = (*instance)->next;
	    *instance = (*instance)->next;
	}
	else if (candidate_instance < ret_instance) {
	    ret_instance = candidate_instance;
	    return_instance = (*instance)->next;
	    *instance = (*instance)->next;
	}
	else
	    *instance = (*instance)->next;
    }
    if (return_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    /* 
     | Found the next lexicographic instance.  Return its pointer
     */
    *instance = return_instance;
    return MAN_C_SUCCESS;

} /* End of [[class_prefix]]find_next_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]find_first_instance
**
**      This routine returns the first lexicographic instance of this class.
**
**  FORMAL PARAMETERS:
**
**	instance    - pointer to next lexicographical instance is returned 
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_FAILURE
**	MAN_C_NO_SUCH_OBJECT_INSTANCE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]find_first_instance(
				    instance
				   )

[[class_void_name]] **instance;

{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

[[class_void_name]] *current_instance = NULL;
unsigned int        inst, curr_inst;
octet_string	    inst_octet;
octet_string        curr_inst_octet;

    inst_octet.data_type = ASN1_C_INTEGER;
    curr_inst_octet.data_type = ASN1_C_INTEGER;

    *instance = new_[[class_name]]_header;	/* point to top of class structure list */

    /* Search for the next lexicographic occurence from within the
     | linked list of class structures 
     */
    while ((*instance)->next != new_[[class_name]]_header) {
	if (current_instance == NULL) {
	    current_instance = (*instance)->next;
	    *instance = (*instance)->next;
	}
	else {
	    inst_octet.length = (*instance)->next->instance_name_length;
	    inst_octet.string = (*instance)->next->instance_name;
	    curr_inst_octet.length = current_instance->instance_name_length;
	    curr_inst_octet.string = current_instance->instance_name;

	    copy_octet_to_unsigned_int( &inst, &inst_octet );
	    copy_octet_to_unsigned_int( &curr_inst, &curr_inst_octet );

	    if (inst < curr_inst) {
		current_instance = (*instance)->next;
		*instance = (*instance)->next;
	    }
	    else
		*instance = (*instance)->next;
	}
    }
    if (current_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    /* 
     | Found the next lexicographic instance.  Return its pointer
     */
    *instance = current_instance;
    return MAN_C_SUCCESS;

} /* End of [[class_prefix]]find_first_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]perform_getnext
**
**      This routine determines the instance and attribute value to return
**      to the caller.  If there is no attribute that fulfills the requirements,
**      MAN_C_NO_SUCH_ATTRIBUTE is returned.
**
**    The way GetNext works for this class is as follows.
**
**    1.  If there is no attribute specified, the first attribute from
**        the class is used.
**    2.  Else if instance value is not specified (i.e., instance avl is
**        merely a "template" with the corresponding octet strings being
**        blank), the first instance of the class is used.  In addition,
**        the MOM returns the value of the attribute specified.
**    3.  Else if both the instance and attribute are specified, then
the
**        value of the next lexicographic attribute is returned:
**            a - the attribute following the one requested is returned;
**            b - if the instance specified the last entry in the table,
**                the next attribute for the first instance is returned;
**            c - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.  This
**                causes the PE to "roll-over" to next class.
**
**  FORMAL PARAMETERS:
**
**	class	        - class
**	attribute	- attribute specified in request
**	instance	- instance name
**	dummy_scope	- scope (not used)
**	attribute_value	- attribute value to be returned
**	reply_status	- pointer to integer reply status to be returned
**      reply_oid         - pointer to return any class-specific responses
**                              (if needed).
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
**      MAN_C_PROCESSING_FAILURE
**      any status from [[class_prefix]]next_oid
**
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]perform_getnext(
				class,
				attribute,
				instance,
				dummy_scope,
				attribute_value,
				reply_status,
				reply_oid
			       )

object_id   *class;
avl	    *attribute;
avl	    **instance;
scope	    dummy_scope;
avl	    **attribute_value;
reply_type  *reply_status;
object_id   **reply_oid;

{

man_status	status;
man_status      attr_status;
man_status      inloop_status;
object_id	*in_oid;
object_id       *ret_oid;
object_id       *pad_oid = NULL;
[[class_void_name]]		*specific_instance;
octet_string	*instance_octet = NULL;
unsigned int    itag, imod;
avl		*return_attribute = (avl *)NULL;

/** add function call to assure class structure is up-to-date (if necessary) **/

    status = moss_avl_init( &return_attribute );
    if ERROR_CONDITION(status)
        return status;

    attr_status = moss_avl_point( attribute, &in_oid, NULL, NULL, NULL, NULL );
    if (attr_status == MAN_C_NO_ELEMENT) {
			    /* if no attribute specified use first attribute for class */
	/*-insert-code-getnext-first-attribute-*/
    }
    else if (attr_status == MAN_C_SUCCESS) {
        ret_oid = in_oid;
    }
    else {
        moss_avl_free( &return_attribute, TRUE );
        return attr_status;
    }
/*
 * get the instance avl and set the internal loop status to seed the loop (return
 * if status is not success or no element)
 */
    inloop_status = moss_avl_point( *instance, NULL, NULL, NULL, NULL, NULL );
    if (inloop_status != MAN_C_SUCCESS) {
        moss_avl_free( &return_attribute, TRUE );
        return MAN_C_PROCESSING_FAILURE;
    }

    do {
			/* if no instance, get first instance */

	    status = find_instance_avl( *instance,
					/*-insert-code-name-oid-*/
					&imod,
					&itag,
					&instance_octet );
	    if (status != MAN_C_SUCCESS) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }
            /*
            | If attribute AVL is blank, construct the instance AVL from the
            | first attribute 
            */
            if (attr_status == MAN_C_NO_ELEMENT) {
                status = [[class_prefix]]find_first_instance(&specific_instance);
                if (ERROR_CONDITION(status)) {
                    moss_avl_free( &return_attribute, TRUE );
                    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                }
            }

            /*
             | Otherwise, attribute AVL is specified!  Find the next
             | lexicographic occurence.  If no class found, (NO_SUCH_OBJ_INS)
             | roll-over to the first attribute of the next class.  If the
             | value is NULL, AVL is a "template".  Get the first instance. 
	     | Else instance value is specified in the AVL.  Get the next 
	     | instance.
             */

            else
            {
                status = [[class_prefix]]find_next_instance(instance_octet, &specific_instance);
	        if (status != MAN_C_SUCCESS)			     /* if no next, return to first */
                {
                   if (status == MAN_C_NO_SUCH_OBJECT_INSTANCE)
                   {
		      status = [[class_prefix]]next_oid( in_oid, &ret_oid );	/* but make sure there are more attributes */
		      if (status != MAN_C_SUCCESS)
                      {
                        moss_avl_free( &return_attribute, TRUE );
                        return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                      }
                      status = [[class_prefix]]find_first_instance(&specific_instance);
                   }
                   else if (status == MAN_C_MISSING_ATTRIBUTE_VALUE)
                   {
                      status = [[class_prefix]]find_first_instance(&specific_instance);
                   }
                   if (status != MAN_C_SUCCESS)
                   {
                      moss_avl_free( &return_attribute, TRUE );
                      return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                   }
  
                }
            }
	    status = [[class_prefix]]construct_table_instance(*instance,specific_instance);
            if ERROR_CONDITION (status) {
                moss_avl_free( &return_attribute, TRUE );
                return status;
            }

                   /* put appropriate attribute oid in the request */

        status = moss_avl_add( return_attribute, ret_oid, 0, 0, NULL );
        if ERROR_CONDITION (status) 
        {
            moss_avl_free( &return_attribute, TRUE );
            return MAN_C_PROCESSING_FAILURE;
        }

        /*
         | Now that we have the specific instance of interest, call
         | [[class_prefix]]perform_get routine to obtain the requested
         | attribute within that instance.
         */

	status = [[class_prefix]]perform_get(
					    pad_oid,
					    return_attribute,
					    specific_instance,
					    attribute_value,
					    reply_status,
					    reply_oid
					    );

	if (ERROR_CONDITION(status)) {	/* if failure, try next instance (or next attribute/first instance) */
            in_oid = ret_oid;
	    inloop_status = moss_avl_backup( *attribute_value );		/* get rid of error reply */
	    if (ERROR_CONDITION(inloop_status)) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_PROCESSING_FAILURE;
            }
	    inloop_status = moss_avl_remove( *attribute_value );
	    if (ERROR_CONDITION(inloop_status)) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_PROCESSING_FAILURE;
            }
	    inloop_status = moss_avl_reset( *instance );				/* get instance avl again */
	    if (ERROR_CONDITION(inloop_status)) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_PROCESSING_FAILURE;
            }
	    inloop_status = moss_avl_point( *instance, NULL, NULL, NULL, NULL, NULL );
	}

    }
    while (status != MAN_C_SUCCESS);

    moss_avl_free( &return_attribute, TRUE );

    return MAN_C_SUCCESS;
} /* End of [[class_prefix]]perform_getnext() */

/* End of [[class_prefix]]perform_getnext.c */
