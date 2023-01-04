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
static char *rcsid = "@(#)$RCSfile: build_perform_nontable_getnext.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:23:00 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_NONTABLE_GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**
**	It provides the routines to perform the get-next function for the
**	[[class_name]] class.
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
#include "import_oids.h"
#include "extern_common.h"



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]next_oid
**
**	This routine returns the oid for next lexicographic attribute
**	based on the oid supplied.
**
**  FORMAL PARAMETERS:
**
**	attr_oid    - oid of attribute specified in request
**	next_oid    - oid of next lexicographic attribute
**	
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
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
**      [[class_prefix]]construct_instance
**
**	This routine constructs an instance avl when none is provided
**	in the getnext request.
**
**  FORMAL PARAMETERS:
**
**	instance   	- pointer to avl to hold constructed instance name
**	attr_oid	- attribute oid for the instance to be constructed
**	
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
[[class_prefix]]construct_instance(
				   instance,
				   attr_oid
				  )

avl	    	*instance;
object_id	*attr_oid;

{
man_status	status = MAN_C_SUCCESS;
octet_string	null_instance_octet;
int             null_instance_val = 0;      /* Use for encoding a "0" value */

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

         /* create a datatype octet for the non-table instance "0" */

    null_instance_octet.length = 4;	/* create a NULL datatype octet */
    null_instance_octet.data_type = ASN1_C_NULL;
    null_instance_octet.string = (char *) &null_instance_val;

    status = moss_avl_add(  instance,
	                    attr_oid, 
                            (int)MAN_C_SUCCESS,
                            ASN1_C_NULL,
                            &null_instance_octet);
 
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    return  status;

} /* End of [[class_prefix]]construct_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]perform_getnext
**
**	This routine determines the instance and attribute value to return
**	to the caller.  If there is no attribute that fulfills the requirements,
**	MAN_C_NO_SUCH_ATTRIBUTE_ID is returned.
**
**    The way GetNext works for this class is as follows.
**
**    1.  If there is no attribute specified, the first attribute from
**        the class is used.
**    2.  Else if instance value is not specified (i.e., instance avl is 
**	  merely a "template" with the corresponding octet strings being
**	  blank), the first instance of the class is used.  In addition, 
**	  the MOM returns the value of the attribute specified.
**    3.  Else if both the instance and attribute are specified, then the
**        value of the next lexicographic attribute is returned:
**            a - the attribute following the one requested is returned;
**            b - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.  This
**	          causes the PE to "roll-over" to next class.
**
**  FORMAL PARAMETERS:
**
**	class		  - object class
**	attribute	  - attribute specified in request
**	instance          - instance name
**	dummy_scope	  - scope (not used)
**	attribute_value	  - attribute value to be returned
**	reply_status	  - pointer to integer reply status to be returned
**	reply_oid	  - pointer to return any class-specific responses 
**				(if needed).
**	
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_ATTRIBUTE
**	MAN_C_PROCESSING_FAILURE
**	any status from [[class_prefix]]next_oid
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
man_status	    status;
man_status	    attr_status;
man_status	    inloop_status;
void		    *specific_instance;
avl		    *specific_object_instance = NULL;
int		    class_code = (int)NULL;
int		    more = FALSE;
uid		    *object_uid = NULL;
GET_CONTEXT_STRUCT  *get_context = NULL;
object_id	    *in_oid;
object_id	    *ret_oid;
object_id	    *pad_oid = NULL;
object_id           *new_oid = NULL;
octet_string        *instance_octet = NULL;
unsigned int        itag, imod;
int                 *instance_value;


/** add function call to assure class structure is up-to-date (if necessary) **/

	/* see if an attribute was specified...if not, set ret_oid to first attribute in class */

    attr_status = moss_avl_reset( attribute );
    if ERROR_CONDITION (attr_status) return MAN_C_PROCESSING_FAILURE;
    attr_status = moss_avl_point( attribute, &in_oid, NULL, NULL, NULL, NULL );

    if (attr_status == MAN_C_NO_ELEMENT) {

	/* set attribute to first attribute for class */

	    /*-insert-code-getnext-first-attribute-*/

    }
    else {
	    /* create a copy of the oid in the avl in case we free the avl */
	status = moss_create_oid(in_oid->count, in_oid->value, &new_oid);
	if ERROR_CONDITION (status) return MAN_C_PROCESSING_FAILURE;
	ret_oid = new_oid;
    }


    /* was an instance specified? */

    status = moss_avl_reset( *instance );
    if ERROR_CONDITION (status) {
	if (new_oid != NULL) moss_free_oid( new_oid );
	return MAN_C_PROCESSING_FAILURE;
    }

    status = moss_avl_point( *instance, NULL, NULL, NULL, NULL, NULL );

    if (status != MAN_C_SUCCESS) {
	if (new_oid != NULL) moss_free_oid( new_oid );
	return MAN_C_PROCESSING_FAILURE;
    }

    /* position to first instance in AVL */

    status = find_instance_avl( *instance,
                                ret_oid,
                                &imod,
                                &itag,
                                &instance_octet );
    if (status != MAN_C_SUCCESS) {
        if (new_oid != NULL) moss_free_oid( new_oid );
        return MAN_C_NO_SUCH_ATTRIBUTE_ID;
    }

    /*
     | If attribute AVL is blank, construct the instance AVL from the
     | first attribute as set up in ret_oid
     */
    if (attr_status == MAN_C_NO_ELEMENT) {
        status = [[class_prefix]]construct_instance ( 
				*instance ,
				ret_oid     );
        if (ERROR_CONDITION(status)) {
            if (new_oid != NULL) moss_free_oid( new_oid );
            return MAN_C_NO_SUCH_ATTRIBUTE_ID;
        }
    }

    /*
     | Otherwise, attribute AVL is specified!  Check the instance octet 
     | string value. If the value is NULL, AVL is a "template".  Get the 
     | first instance. Else instance value is specified in the AVL.  Get 
     | the next instance. 
     */

    else {
	instance_value = (int *) instance_octet->string;
	if ( ( instance_octet->length == 0 ) && ( instance_value == NULL) ) {
	    status = [[class_prefix]]construct_instance ( 
				*instance ,
				ret_oid    );
            if (ERROR_CONDITION(status)) {
            if (new_oid != NULL) moss_free_oid( new_oid );
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }
        }
        else {
	    status = [[class_prefix]]next_oid( in_oid, &ret_oid );	/* use next attribute */
	    if (status != MAN_C_SUCCESS) {
	        if (new_oid != NULL) moss_free_oid( new_oid );
	        return status;
	    }
	}
    }

    /* 
     | We now have the oid of the form Class-Instance-Attribute (C-I-A),
     | information about which is to be returned to the requestor.  Get 
     | the <class>_DEF structure (specific_instance) in the linked list 
     | that matches this instance.
     */
    status = get_instance (class,
			    *instance,
			    dummy_scope,
			    &specific_instance,
			    &specific_object_instance,
			    &object_uid,
			    &more,
			    &get_context,
			    FALSE,
			    /*-insert-code-class-code-*/

    if (status != MAN_C_SUCCESS) {	/* instance doesn't exist? */
	if (new_oid != NULL) moss_free_oid( new_oid );
	return MAN_C_NO_SUCH_ATTRIBUTE_ID;    /* force a roll to next class */
    }

    do {

	/*
	 | Now that we have the specific instance of interest, call 
         | [[class_prefix]]perform_get routine to obtain the requested
         | attribute within that instance. 
         */

        status = moss_avl_free( &attribute, FALSE );
	if ERROR_CONDITION (status) {
	    if (new_oid != NULL) moss_free_oid( new_oid );
	    return MAN_C_PROCESSING_FAILURE;
	}
	status = moss_avl_init( &attribute );		/* put appropriate attribute oid in the request */
	if ERROR_CONDITION (status) {
	    if (new_oid != NULL) moss_free_oid( new_oid );
	    return MAN_C_PROCESSING_FAILURE;
	}
	status = moss_avl_add( attribute, ret_oid, 0, 0, NULL );
	if ERROR_CONDITION (status) {
	    if (new_oid != NULL) moss_free_oid( new_oid );
	    return MAN_C_PROCESSING_FAILURE;
	}

	status = [[class_prefix]]perform_get (
					pad_oid,
					attribute,
					specific_instance,
					attribute_value,
					reply_status,
					reply_oid
					);

	if (ERROR_CONDITION(status)) {
	    in_oid = ret_oid;
	    inloop_status = [[class_prefix]]next_oid( in_oid, &ret_oid );
	    if (ERROR_CONDITION(inloop_status)) {
	        if (new_oid != NULL) moss_free_oid( new_oid );
		return inloop_status;
	    }
	    inloop_status = moss_avl_backup( *attribute_value );	    /* remove the error avl and try again with next attribute id */
	    if (ERROR_CONDITION(inloop_status)) {
		if (new_oid != NULL) moss_free_oid( new_oid );
		return MAN_C_PROCESSING_FAILURE;
	    }
	    inloop_status = moss_avl_remove( *attribute_value );
	    if (ERROR_CONDITION(inloop_status)) {
		if (new_oid != NULL) moss_free_oid( new_oid );
		return MAN_C_PROCESSING_FAILURE;
	    }
	}
    }
    while (status != MAN_C_SUCCESS);

    if (new_oid != NULL) moss_free_oid( new_oid );	/* make sure we don't leak memory */

    return status;
} /* End of [[class_prefix]]perform_getnext() */

/* End of [[class_prefix]]perform_getnext.c */
