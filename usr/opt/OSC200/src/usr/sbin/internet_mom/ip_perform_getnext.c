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
static char *rcsid = "@(#)$RCSfile: ip_perform_getnext.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/07 21:14:44 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      ip_PERFORM_NONTABLE_GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**
**	It provides the routines to perform the get-next function for the
**	ip class.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  23-Mar-1993
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
**      ip_next_oid
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
ip_next_oid(
			 attr_oid,
			 ret_oid
			)

object_id   *attr_oid;
object_id   **ret_oid;

{

man_status  status;

        status = MAN_C_SUCCESS;

    if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipForwarding_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipDefaultTTL_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipDefaultTTL_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInReceives_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInReceives_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInHdrErrors_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInHdrErrors_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInAddrErrors_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInAddrErrors_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipForwDatagrams_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipForwDatagrams_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInUnknownProtos_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInUnknownProtos_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInDiscards_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInDiscards_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipInDelivers_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipInDelivers_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipOutRequests_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipOutRequests_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipOutDiscards_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipOutDiscards_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipOutNoRoutes_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipOutNoRoutes_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipReasmTimeout_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipReasmTimeout_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipReasmReqds_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipReasmReqds_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipReasmOKs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipReasmOKs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipReasmFails_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipReasmFails_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipFragOKs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipFragOKs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipFragFails_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipFragFails_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &ip_ATTR_ipFragCreates_SNP;
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipFragCreates_SNP)) == MAN_C_EQUAL) 
        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;
/*        *ret_oid = &ip_ATTR_ipRoutingDiscards_SNP; NO_ATTR_ID ***  MIA */
    else if ((status = moss_compare_oid( attr_oid, &ip_ATTR_ipRoutingDiscards_SNP)) == MAN_C_EQUAL) 
        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;
    else status = MAN_C_NO_SUCH_ATTRIBUTE_ID;


    if (status == MAN_C_EQUAL) return MAN_C_SUCCESS;
    return status;

} /* End of ip_next_oid() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ip_construct_instance
**
**	This routine constructs an instance avl when none is provided
**	in the getnext request.
**
**  FORMAL PARAMETERS:
**
**	instance          - pointer to avl to hold constructed instance name
**      attr_oid	  - attribute oid for building the desired instance
**	
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**      MAN_C_PROCESSING_FAILURE
**      MAN_C_SUCCESS			( from moss_avl_add() ) 
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
ip_construct_instance(
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

} /* End of ip_construct_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      ip_perform_getnext
**
**	This routine determines the instance and attribute value to return
**	to the caller.  If there is no attribute that fulfills the requirements,
**	MAN_C_NO_SUCH_ATTRIBUTE is returned.
**
**    The way GetNext works for this class is as follows.
**
**    1.  If there is no attribute specified, the first attribute from
**        the class is used.
**    2.  If there is no instance specified (a "template" avl for the
**        instance name), the first instance of the class is used.  A
**        template avl has all the elements, but no values for the
**        identifier(s).  The MOM then returns the value of the
**        attribute specified.
**    3.  If there is an instance and attribute specified, then the
**        value of the next lexicographic attribute is returned:
**            a - the attribute following the one requested is returned;
**            b - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.
**
**  FORMAL PARAMETERS:
**
**	class		  - object class
**	attribute	  - attribute specified in request
**	object_instance   - instance name
**	scope		  - scope (not used)
**	attribute_value	  - attribute value to be returned
**	reply_status	  - pointer to integer reply status to be returned
**	
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
**      MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
ip_perform_getnext(
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


status = (man_status) refresh_ip_list(new_ip_header); if ERROR_CONDITION(status) return status;

	/* see if an attribute was specified...if not, set ret_oid to first attribute in class */

    attr_status = moss_avl_reset( attribute );
    if ERROR_CONDITION (status) return MAN_C_PROCESSING_FAILURE;
    attr_status = moss_avl_point( attribute, &in_oid, NULL, NULL, NULL, NULL );

    if (attr_status == MAN_C_NO_ELEMENT) {

			/* set attribute to first attribute for class */

       ret_oid = &ip_ATTR_ipForwarding_SNP;

	    in_oid = ret_oid;	/* make sure in_oid points to something */

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

    if (attr_status == MAN_C_NO_ELEMENT) {
        status = ip_construct_instance ( 
				*instance ,
				ret_oid     );
        if (ERROR_CONDITION(status)) {
            if (new_oid != NULL) moss_free_oid( new_oid );
            return MAN_C_NO_SUCH_ATTRIBUTE_ID;
        }
    }

    else {
	instance_value = (int *) instance_octet->string;
	if ( ( instance_octet->length == 0 ) && ( instance_value == NULL) ) {
	    status = ip_construct_instance ( 
				*instance ,
				ret_oid    );
            if (ERROR_CONDITION(status)) {
            if (new_oid != NULL) moss_free_oid( new_oid );
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }
        }
        else {
	    status = ip_next_oid( in_oid, &ret_oid );	/* use next attribute */
	    if (status != MAN_C_SUCCESS) {
	        if (new_oid != NULL) moss_free_oid( new_oid );
	        return status;
	    }
	}
    }

    status = get_instance (class,
			    *instance,
			    dummy_scope,
			    &specific_instance,
			    &specific_object_instance,
			    &object_uid,
			    &more,
			    &get_context,
			    FALSE,
					ip_ip_ID);

    if (status != MAN_C_SUCCESS) {	/* instance doesn't exist? */
	if (new_oid != NULL) moss_free_oid( new_oid );
	return MAN_C_NO_SUCH_ATTRIBUTE_ID;    /* force a roll to next class */
    }

    do {

	/* call perform get routine to actually obtain the attribute */

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

        /*
         | By now, the class-instance-attribute to be returned has been
         | identified, or constructed.  Get its value next.
         */

	status = ip_perform_get (
					pad_oid,
					attribute,
					specific_instance,
					attribute_value,
					reply_status,
					reply_oid
					);

	if (ERROR_CONDITION(status)) {
	    in_oid = ret_oid;
	    inloop_status = ip_next_oid( in_oid, &ret_oid );
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
} /* End of ip_perform_getnext() */

/* End of ip_perform_getnext.c */



