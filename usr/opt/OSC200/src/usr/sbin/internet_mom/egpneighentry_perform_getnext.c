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
static char *rcsid = "@(#)$RCSfile: egpneighentry_perform_getnext.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/11/10 21:37:15 $";
#endif
/*
**++
**  FACILITY:  LKG2-1
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      egpNeighEntry_PERFORM_TABLE_GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**
**      It provides the routines to perform the get-next function for the
**      egpNeighEntry class.
**
**  AUTHORS:
**
**      Muhammad I. Ashraf
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  22-Feb-1993
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"
#include "moss_inet.h"
#include "common.h"
#include "extern_common.h"



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      egpNeighEntry_next_oid
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
egpNeighEntry_next_oid(
                        object_id   *attr_oid,
                        object_id   **ret_oid
                        )
{

man_status  status;

        status = MAN_C_SUCCESS;

    if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighState_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighAddr_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighAddr_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighAs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighAs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighInMsgs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighInMsgs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighInErrs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighInErrs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighOutMsgs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighOutMsgs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighOutErrs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighOutErrs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighInErrMsgs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighInErrMsgs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighOutErrMsgs_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighOutErrMsgs_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighStateUps_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighStateUps_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighStateDowns_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighStateDowns_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighIntervalHello_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighIntervalHello_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighIntervalPoll_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighIntervalPoll_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighMode_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighMode_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &egpNeighEntry_ATTR_egpNeighEventTrigger_SNP;
    else if ((status = moss_compare_oid( attr_oid, &egpNeighEntry_ATTR_egpNeighEventTrigger_SNP)) == MAN_C_EQUAL) 
        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;
    else status = MAN_C_NO_SUCH_ATTRIBUTE_ID;


    if (status == MAN_C_EQUAL) return MAN_C_SUCCESS;
    return status;

} /* End egpNeighEntry_next_oid() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      egpNeighEntry_construct_table_instance
**
**      This routine constructs an instance avl when none is provided
**      in the getnext request.
**
**  FORMAL PARAMETERS:
**
**      object_instance   - pointer to avl to hold constructed instance name
**	instance -	    pointer to class structure with required instance
**
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_NO_SUCH_ATTRIBUTE_ID
**      MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
egpNeighEntry_construct_table_instance(
                                avl         *instance,
				egpNeighEntry_DEF *instance_elem
                                )
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
            /** note to developers: this code assumes an integer table
index.
                change the ASN1_C_INTEGER to the appropriate datatype if
the
                table is indexed by a different datatype **/

    instance_octet.length = 4;
    instance_octet.data_type = INET_C_SMI_IP_ADDRESS;
    instance_octet.string = instance_elem->instance_name;

    status = moss_avl_add(  instance,
#ifdef DNA_CMIP_OID 
                               &egpNeighEntry_ATTR_egpNeighAddr_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &egpNeighEntry_ATTR_egpNeighAddr_SNP,
#endif /* SNMP_OID */
				/** Need to supply identifier attribute OID **/
                                          /** &ATTR_NAME_DNA **/
                            (int)MAN_C_SUCCESS,
                            INET_C_SMI_IP_ADDRESS,
                            &instance_octet);
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    return  status;

} /* End of egpNeighEntry_construct_table_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      egpNeighEntry_find_next_instance
**
**      This routine gets the next instance based on the instance supplied.
**      It does so by first building input_oid from the instance AVL
**      elements relating to the instance identifier, viz. egpNeighAddr
**      It then compares this with the tmp_oid
**      built the same way from each of the available instances
**      doubly-linked in the new_egpNeighEntry_header structure.
**
**  FORMAL PARAMETERS:
**
**      inst_avl    - pointer to instance avl passed in the getnext request
**
**	inst_octet  - pointer to octet from instance avl of instance supplied
**		      in request
**	instance    - pointer to pointer to class structure containing
**		      next instance
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_NOSUCH_OBJECT_INSTANCE
**      MAN_C_MISSING_ATTRIBUTE_VALUE
**      MAN_C_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
egpNeighEntry_find_next_instance(
				octet_string	    *inst_octet,
				egpNeighEntry_DEF **instance
				    )
{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

unsigned int        ret_instance;
egpNeighEntry_DEF   *return_instance = NULL;
man_status          status;
int                 index ;
char                *ch ;
man_status          cmp_status ;
unsigned int        curr_array[ 4 ] ;
unsigned int        cand_array[ 4 ] ;
unsigned int        ret_array[ 4 ] ;

    /* get requested instance */
    if ((inst_octet->string == NULL) ||
        (inst_octet->data_type != INET_C_SMI_IP_ADDRESS))
        return MAN_C_MISSING_ATTRIBUTE_VALUE;

    ch = (char *)inst_octet->string;
    for ( index = 0; index < 4; index++ )
    {
       curr_array[ index ] = *ch & 0xff;
       ch++;
    }

    *instance = new_egpNeighEntry_header;/*point to top of class structure lst*/

    while ((*instance)->next != new_egpNeighEntry_header) {
        ch = (char *)(*instance)->next->instance_name;
        for ( index = 0; index < 4; index++ )
        {
           cand_array[ index ] = *ch & 0xff;
           ch++;
        }
        cmp_status = mom_compare_array( 4, cand_array , curr_array );
        if ( ( cmp_status == MAN_C_LESS) || ( cmp_status == MAN_C_EQUAL))
            *instance = (*instance)->next;
        else if (return_instance == NULL) {
            for ( index = 0; index < 4; index++ )
                ret_array[ index ] = cand_array[ index ];
            return_instance = (*instance)->next;
            *instance = (*instance)->next;
        }
        else if (mom_compare_array( 4, cand_array , ret_array) == MAN_C_LESS)
        {
            for ( index = 0; index < 4; index++ )
                ret_array[ index ] = cand_array[ index ];
            return_instance = (*instance)->next;
            *instance = (*instance)->next;
        }
        else
            *instance = (*instance)->next;
    }

    if (return_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    *instance = return_instance;
    return MAN_C_SUCCESS;

} /* End of egpNeighEntry_find_next_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      egpNeighEntry_find_first_instance
**
**      This routine returns the first lexicographic instance of this class.
**      It does so by first building temp_oid and curr_oid from the
**      available instances doubly-linked in the new_atEntry_header
**      structure, and then cross-comparing them to arrive at the
**      instance which has the first lexicographic ranking in the list.
**
**  FORMAL PARAMETERS:
**
**      instance    - pointer to class structure containing next instance
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS
**      MAN_C_NO_SUCH_OBJECT_INSTANCE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
egpNeighEntry_find_first_instance(
				egpNeighEntry_DEF **instance
				    )
{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

egpNeighEntry_DEF *current_instance = NULL;
unsigned int        inst, curr_inst;
octet_string	    inst_octet;
octet_string        curr_inst_octet;
int                 index ;
char                *ch ;
man_status          cmp_status ;
unsigned int        curr_array[ 4 ] ;
unsigned int        cand_array[ 4 ] ;

    inst_octet.data_type = INET_C_SMI_IP_ADDRESS;
    curr_inst_octet.data_type = INET_C_SMI_IP_ADDRESS;

    *instance = new_egpNeighEntry_header;	/* point to top of class structure list */

    while ((*instance)->next != new_egpNeighEntry_header) {
	if (current_instance == NULL) {
	    current_instance = (*instance)->next;
	    *instance = (*instance)->next;
	}
	else {
            ch = (char *)(*instance)->next->instance_name;
            for ( index = 0; index < 4; index++ )
            {
               cand_array[ index ] = *ch & 0xff;
               ch++;
            }
            ch = (char *)current_instance->instance_name;
            for ( index = 0; index < 4; index++ )
            {
               curr_array[ index ] = *ch & 0xff;
               ch++;
            }
            cmp_status = mom_compare_array( 4, cand_array , curr_array);

            if (cmp_status == MAN_C_LESS) {
                current_instance = (*instance)->next;
                *instance = (*instance)->next;
            }
            else
                *instance = (*instance)->next;

	}
    }
    if (current_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    *instance = current_instance;
    return MAN_C_SUCCESS;
} /* End of egpNeighEntry_find_first_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      egpNeighEntry_perform_getnext
**
**      This routine determines the instance and attribute value to return
**      to the caller.  If there is no attribute that fulfills the requirements,
**      MAN_C_NO_SUCH_ATTRIBUTE is returned.
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
**        value of the next lexicographic attribute is returned.
**            a - the requested attribute for the next entry in the
**                the table is returned;
**            b - if the instance specified the last entry in the table,
**                the next attribute for the first instance is returned;
**            c - if there are no more attributes for the class, the
**                MOM returns MAN_C_NO_SUCH_ATTRIBUTE_ID directly.
**
**  FORMAL PARAMETERS:
**
**	object_class	- class
**	attribute	- attribute specified in request
**	object_instance	- instance name
**	iso_scope	- scope (not used)
**	attribute_value	- attribute value to be returned
**	reply_status	- pointer to integer reply status to be returned
**
**  RETURN VALUES:       
** 
**      MAN_C_SUCCESS
**	MAN_C_NO_SUCH_ATTRIBUTE_ID
**
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
egpNeighEntry_perform_getnext(
				object_id   *class,
				avl	    *attribute,
				avl	    **instance,
				scope	    dummy_scope,
				avl	    **attribute_value,
				reply_type  *reply_status,
				object_id   **reply_oid
				)
{

man_status	status;
man_status      attr_status;
man_status      inloop_status;
object_id	*in_oid;
object_id       *ret_oid;
object_id       *pad_oid = NULL;
egpNeighEntry_DEF		*specific_instance;
octet_string	*instance_octet = NULL;
unsigned int    itag, imod;
avl		*return_attribute = (avl *)NULL;

/** add function call to assure class structure is up-to-date (if necessary) **/

    status=(man_status)refresh_egpNeighEntry_list(new_egpNeighEntry_header);
        if ERROR_CONDITION(status)
           return(status);

    status = moss_avl_init( &return_attribute );
    if ERROR_CONDITION(status)
        return status;

    attr_status = moss_avl_point( attribute, &in_oid, NULL, NULL, NULL, NULL );
    if (attr_status == MAN_C_NO_ELEMENT) {
			    /* if no attribute specified use first attribute for class */
       ret_oid = &egpNeighEntry_ATTR_egpNeighState_SNP;

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
#ifdef DNA_CMIP_OID 
                               &egpNeighEntry_ATTR_egpNeighAddr_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &egpNeighEntry_ATTR_egpNeighAddr_SNP,
#endif /* SNMP_OID */
				/** Need to supply identifier attribute OID **/
                                          /** &ATTR_NAME_DNA **/
					&imod,
					&itag,
					&instance_octet );
	    if (status != MAN_C_SUCCESS) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }
            if (attr_status == MAN_C_NO_ELEMENT) {
                status = egpNeighEntry_find_first_instance(&specific_instance);
                if (ERROR_CONDITION(status)) {
                    moss_avl_free( &return_attribute, TRUE );
                    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                }
            }

            /* find next instance */

            else
            {
                status = egpNeighEntry_find_next_instance(instance_octet, &specific_instance);
                if (status != MAN_C_SUCCESS)                         /* if no next, return to first */
                {
                   if (status == MAN_C_NO_SUCH_OBJECT_INSTANCE)
                   {
                      status = egpNeighEntry_next_oid( in_oid, &ret_oid);
/* but make sure there are more attributes */
                      if (status != MAN_C_SUCCESS)
                      {
                        moss_avl_free( &return_attribute, TRUE );
                        return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                      }
                      status = egpNeighEntry_find_first_instance(&specific_instance);
                   }
                   else if (status == MAN_C_MISSING_ATTRIBUTE_VALUE)
                   {
                      status = egpNeighEntry_find_first_instance(&specific_instance);
                   }
                   if (status != MAN_C_SUCCESS)
                   {
                      moss_avl_free( &return_attribute, TRUE );
                      return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                   }

                }
            }
            status = egpNeighEntry_construct_table_instance(*instance,specific_instance);
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
         | By now, the class-instance-attribute to be returned has been
         | identified, or constructed.  Get its value next.
         */

	status = egpNeighEntry_perform_get(
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

} /* End of egpNeighEntry_perform_getnext() */

/* End of egpNeighEntry_perform_getnext.c */
