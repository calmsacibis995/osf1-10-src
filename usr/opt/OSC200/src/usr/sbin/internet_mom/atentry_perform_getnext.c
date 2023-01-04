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
static char *rcsid = "@(#)$RCSfile: atentry_perform_getnext.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/22 15:53:57 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      atEntry_PERFORM_TABLE_GETNEXT.C
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**
**      It provides the routines to perform the get-next function for the
**      atEntry class.
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
#include "moss_inet.h"
#include "common.h"
#include "extern_common.h"
#include "inet_mom_specific.h"



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_next_oid
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
atEntry_next_oid(
                         attr_oid,
                         ret_oid
                        )

object_id   *attr_oid;
object_id   **ret_oid;

{

man_status  status;

        status = MAN_C_SUCCESS;

    if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atIfIndex_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &atEntry_ATTR_atPhysAddress_SNP;
    else if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atPhysAddress_SNP)) == MAN_C_EQUAL) 
        *ret_oid = &atEntry_ATTR_atNetAddress_SNP;
    else if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atNetAddress_SNP)) == MAN_C_EQUAL) 
        status = MAN_C_NO_SUCH_ATTRIBUTE_ID;
    else status = MAN_C_NO_SUCH_ATTRIBUTE_ID;


    if (status == MAN_C_EQUAL) return MAN_C_SUCCESS;
    return status;

} /* End of atEntry_next_oid() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_construct_table_instance
**
**      This routine constructs an instance avl when none is provided
**      in the getnext request.
**
**  FORMAL PARAMETERS:
**
**      instance          - pointer to avl to hold constructed instance name
**	instance_elem     - pointer to class structure with required instance
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
atEntry_construct_table_instance(
                                         instance,
				         instance_elem
                                        )

avl         *instance;
atEntry_DEF *instance_elem;

{
man_status      status = MAN_C_SUCCESS;
unsigned int    itag, imod;
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
 
            status = find_instance_avl( instance,
#ifdef DNA_CMIP_OID
                               &atEntry_ATTR_atIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &atEntry_ATTR_atIfIndex_SNP,
#endif /* SNMP_OID */
                                        &imod,
                                        &itag,
                                        &instance_octet );
            if (status != MAN_C_SUCCESS) 
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;

         /*
          | Back up to first index (atIfIndex), and then remove the three, 
          | viz., atIfIndex , atDummy , atNetAddress
          */
         status = moss_avl_backup (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

         status = moss_avl_remove (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;
         status = moss_avl_remove (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;
         status = moss_avl_remove (  instance );
         if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

	    /* create a datatype octet for the table instance */
	    /** note to developers: this code assumes an integer table index.
	        change the ASN1_C_INTEGER to the appropriate datatype if the
		table is indexed by a different datatype **/

    instance_octet.length = sizeof (int); 
    instance_octet.data_type = ASN1_C_INTEGER;
    instance_octet.string = (char *)&(instance_elem->atIfIndex);

    status = moss_avl_add(  instance,
#ifdef DNA_CMIP_OID 
                               &atEntry_ATTR_atIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &atEntry_ATTR_atIfIndex_SNP,
#endif /* SNMP_OID */
/** Multiple identifier OIDs found. Verify the correct OID **/
                            (int)MAN_C_SUCCESS,
                            ASN1_C_INTEGER,
                            &instance_octet);
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    instance_octet.length = sizeof (int); 
    instance_octet.data_type = ASN1_C_INTEGER;
    instance_octet.string = (char *)&(instance_elem->atDummy);

    status = moss_avl_add(  instance,
#ifdef DNA_CMIP_OID 
                               &atEntry_ATTR_atDummy_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &atEntry_ATTR_atDummy_SNP,
#endif /* SNMP_OID */
                            (int)MAN_C_SUCCESS,
                            ASN1_C_INTEGER,
                            &instance_octet);
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    instance_octet.length = sizeof (int);
    instance_octet.data_type = INET_C_SMI_IP_ADDRESS;
    instance_octet.string = (char *)(instance_elem->atNetAddress);

    status = moss_avl_add(  instance,
#ifdef DNA_CMIP_OID
                               &atEntry_ATTR_atNetAddress_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &atEntry_ATTR_atNetAddress_SNP,
#endif /* SNMP_OID */
/** Multiple identifier OIDs found. Verify the correct OID **/
                            (int)MAN_C_SUCCESS,
                            INET_C_SMI_IP_ADDRESS,
                            &instance_octet);
    if ERROR_CONDITION(status) return MAN_C_PROCESSING_FAILURE;

    return status;
} /* End of atEntry_construct_table_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_find_next_instance
**
**      This routine gets the next instance based on the instance supplied.
**      It does so by first building input_oid from the instance AVL
**	elements relating to the instance identifier, viz. atIfIndex, 
**	atDummy, and atNetAddress.  It then compares this with the tmp_oid
**	built the same way from each of the available instances 
**	doubly-linked in the new_atEntry_header structure.
**
**  FORMAL PARAMETERS:
**
**	inst_avl    - pointer to instance avl passed in the getnext request 
**
**	inst_octet  - pointer to octet string from instance avl pointing
**		      to the first instance element in the avl
**
**	instance    - pointer to class structure containing next instance
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
atEntry_find_next_instance(
                                instance_avl,
				inst_octet,
			        instance
				  )

avl                 *instance_avl;
octet_string	    *inst_octet;
atEntry_DEF **instance;

{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

    man_status          status;
    atEntry_DEF         *return_instance = NULL;
    octet_string	candidate_octet;
    atEntry_DEF         *header_p = NULL;
    int                 inst_index ;
    int                 i,j;
    char                *instance_string, *ch;

    object_id 		tmpoid;
    u_int 		tmp_oid[32];
    object_id 		curroid;
    u_int 		curr_oid[32];
    object_id 		inputoid;
    u_int 		input_oid[32];

    /* First identifier is atIfIndex (passed to the routine by default) */
    if ((inst_octet->length == 0) || (*inst_octet->string) <= 0)
        return MAN_C_MISSING_ATTRIBUTE_VALUE;

    input_oid[0] = *inst_octet->string;         /* Move atIfIndex to array[0] */

    /* Second identifier is atDummy */
    status = moss_avl_point(instance_avl,NULL,NULL,NULL,&inst_octet,NULL);
    if (status != MAN_C_SUCCESS)
        return(MAN_C_NO_SUCH_OBJECT_INSTANCE);
    if ((inst_octet->length == 0) || (*inst_octet->string) <= 0)
        return(MAN_C_MISSING_ATTRIBUTE_VALUE);

    input_oid[1] = *inst_octet->string;         /* Move atDummy to array[1] */
    bcopy(inst_octet->string,&input_oid[4],4);
    inputoid.value = &input_oid[0];


    /* Third identifier is atNetAddress */
    status = moss_avl_point(instance_avl,NULL,NULL,NULL,&inst_octet,NULL);
    if (status != MAN_C_SUCCESS)
        return(MAN_C_NO_SUCH_OBJECT_INSTANCE);
    if (inst_octet->length != sizeof(struct in_addr))
         return(MAN_C_NO_SUCH_OBJECT_INSTANCE);

    /*
     *
    */
    inputoid.count = atEntry_INDEX_SIZE;
    ch = (char *)inst_octet->string;
    for (i = 2; i < inputoid.count; i++){
         input_oid[i] = *ch & 0xff;		/* Move atNetAddress starting
at array[2], 4 bytes */
         ch++;
    }
    inputoid.value = &input_oid[0];

    curroid.value = 0;
    curroid.value = &curr_oid[0];

    /* get the instance name from the each link list, split
     * and put into tmpoid and compare
    */

    header_p = new_atEntry_header;

    while(header_p->next != new_atEntry_header) {

        tmpoid.count = atEntry_INDEX_SIZE;
	ch = (char *)header_p->next->instance_name;
	bcopy(ch,&tmp_oid[0],2*sizeof(int)); /* Move atIfIndex and atDummy */
	for (i=0; i < 2*sizeof(int); i++)
	  ch++;				     /* Increment pointer accordingly */ 
	for(i = 2; i < tmpoid.count; i++){   /* Move atNetAddress */
	    tmp_oid[i] = *ch & 0xff;
	    ch++;
	 }
         tmpoid.value = &tmp_oid[0];

         if ((compare_oid(&tmpoid,&inputoid) == MOM_C_GREATER_THAN) &&
            ((curroid.count == 0) ||
            (compare_oid(&tmpoid,&curroid) == MOM_C_LESS_THAN))){
           /*
            * Save the current instance information.
           */
           curroid.count = tmpoid.count;
           for (j = 0; j < tmpoid.count; j++)
                curroid.value[j] = tmpoid.value[j];
           return_instance = header_p->next;
          }
         header_p = header_p->next ;
    }/* while */

    if (return_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    *instance = return_instance;
    return MAN_C_SUCCESS;

} /* End of atEntry_find_next_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_find_first_instance
**
**      This routine returns the first lexicographic instance of this class.
**      It does so by first building temp_oid and curr_oid from the 
**	available instances doubly-linked in the new_atEntry_header 
**	structure, and then cross-comparing them to arrive at the
**	instance which has the first lexicographic ranking in the list.
**
**  FORMAL PARAMETERS:
**
**	instance    - pointer to class structure containing next instance
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
atEntry_find_first_instance(
				    instance
				   )

atEntry_DEF **instance;

{
/** note to developers: this routine assumes that instance is an integer
    table index.  modify the ordering algorithm appropriately for other
    datatypes **/

    atEntry_DEF *current_instance = NULL;
    int                 i,j;
    char                *instance_string, *ch;
    atEntry_DEF         *header_p = NULL;

    object_id           tmpoid;
    u_int               tmp_oid[32];
    object_id           curroid;
    u_int               curr_oid[32];

    header_p = new_atEntry_header;
    while(header_p->next != new_atEntry_header) {
        if (current_instance == NULL) {
            current_instance = header_p->next;
            header_p = header_p->next ;
        }
        else
        {
         tmpoid.count = (atEntry_INDEX_SIZE * 2);
         bcopy(header_p->next->instance_name,tmp_oid,tmpoid.count);
         tmpoid.value = tmp_oid;
         curroid.count = (atEntry_INDEX_SIZE * 2);
         bcopy(current_instance->instance_name,curr_oid,curroid.count);
         curroid.value = curr_oid;

         if (compare_oid(&tmpoid,&curroid) == MOM_C_LESS_THAN) {
                current_instance = header_p->next;
                header_p = header_p->next;
            }
            else
                header_p = header_p->next;
        }
    }

    if (current_instance == NULL)
	return MAN_C_NO_SUCH_OBJECT_INSTANCE;

    *instance = current_instance;
    return MAN_C_SUCCESS;

} /* End of atEntry_find_first_instance() */


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_perform_getnext
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
**	  template avl has all the elements, but no values for the
**	  identifier(s).  The MOM then returns the value of the 
**	  attribute specified.
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
**	MAN_C_PROCESSING_FAILURE
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status
atEntry_perform_getnext(
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
atEntry_DEF		*specific_instance;
octet_string	*instance_octet = NULL;
unsigned int    itag, imod;
avl		*return_attribute = (avl *)NULL;

status = (man_status) refresh_atEntry_list(new_atEntry_header); if ERROR_CONDITION(status) return status;

    status = moss_avl_init( &return_attribute );
    if ERROR_CONDITION(status)
        return status;

    attr_status = moss_avl_point( attribute, &in_oid, NULL, NULL, NULL, NULL );
    if (attr_status == MAN_C_NO_ELEMENT) {
			    /* if no attribute specified use first attribute for class */
       ret_oid = &atEntry_ATTR_atIfIndex_SNP;

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
                               &atEntry_ATTR_atIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                               &atEntry_ATTR_atIfIndex_SNP,
#endif /* SNMP_OID */
/** Multiple identifier OIDs found. Verify the correct OID **/
					&imod,
					&itag,
					&instance_octet );
	    if (status != MAN_C_SUCCESS) {
                moss_avl_free( &return_attribute, TRUE );
                return MAN_C_NO_SUCH_ATTRIBUTE_ID;
            }
            if (attr_status == MAN_C_NO_ELEMENT) {
                status = atEntry_find_first_instance(&specific_instance);
                if (ERROR_CONDITION(status)) {
                    moss_avl_free( &return_attribute, TRUE );
                    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                }
            }

	    /* find next instance */

            else
            {
                status = atEntry_find_next_instance( *instance, instance_octet, &specific_instance);
	        if (status != MAN_C_SUCCESS)			     /* if no next, return to first */
                {
                   if (status == MAN_C_NO_SUCH_OBJECT_INSTANCE)
                   {
		      status = atEntry_next_oid( in_oid, &ret_oid );	/* but make sure there are more attributes */
		      if (status != MAN_C_SUCCESS)
                      {
                        moss_avl_free( &return_attribute, TRUE );
                        return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                      }
                      status = atEntry_find_first_instance(&specific_instance);
                   }
                   else if (status == MAN_C_MISSING_ATTRIBUTE_VALUE)
                   {
                      status = atEntry_find_first_instance(&specific_instance);
                   }
                   if (status != MAN_C_SUCCESS)
                   {
                      moss_avl_free( &return_attribute, TRUE );
                      return MAN_C_NO_SUCH_ATTRIBUTE_ID;
                   }
  
                }
            }
	    status = atEntry_construct_table_instance(*instance,specific_instance);
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
        
	status = atEntry_perform_get(
					    pad_oid,
					    return_attribute,
					    specific_instance,
					    attribute_value,
					    reply_status,
					    reply_oid
					    );

	if (ERROR_CONDITION(status)) {	/* if failure, try next instance (or next attribute/first instance) */
	    /*  If atEntry attribute is atDummy, return NoSuch, as a special case */
            if ((status = moss_compare_oid( ret_oid, &atEntry_ATTR_atDummy_SNP))
                                       == MAN_C_EQUAL)
                 moss_avl_free( &return_attribute, TRUE );
                 return MAN_C_NO_SUCH_ATTRIBUTE_ID;

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

} /* End of atEntry_perform_getnext() */

/* End of atEntry_perform_getnext.c */
