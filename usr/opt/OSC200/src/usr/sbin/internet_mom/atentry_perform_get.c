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
static char *rcsid = "@(#)$RCSfile: atentry_perform_get.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:19:31 $";
#endif
/*
**++
**  FACILITY:  ZKO3-3
**
**  Copyright (c) 1993  Digital Equipment Corporation
**
**  MODULE DESCRIPTION:
**
**      This module is part of the Managed Object Module (MOM)
**	for rfc1213.
**	It provides the routines to actually perform the GET
**	function for the atEntry class.
**
**  AUTHORS:
**
**      Geetha M. Brown
**
**      This code was initially created with the 
**	Ultrix MOM Generator - version X1.1.0
**
**  CREATION DATE:  27-Mar-1993
**
**  MODIFICATION HISTORY:
**
**
**--
*/

#include "moss.h"
#include "moss_inet.h"
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_get_attr_oid
**
**	This routine compares the incoming attribute OID against all attribute
**	OIDs. If a match is found, it returns a unique number (from COMMON.H).
**
**  FORMAL PARAMETERS:
**
**	attr_oid - pointer to attribute object id to compare
**	attr     - address of unique attribute number
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
man_status atEntry_get_attr_oid( attr_oid, 
					 attr )

object_id *attr_oid;
unsigned int *attr;

{
  man_status status;

  *attr = 0;

  if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atIfIndex_DNA)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atIfIndex;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atIfIndex_SNP)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atIfIndex;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atNetAddress_DNA)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atNetAddress;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atNetAddress_SNP)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atNetAddress;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atPhysAddress_DNA)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atPhysAddress;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atPhysAddress_SNP)) == MAN_C_EQUAL)
     *attr = atEntry_ATTR_atPhysAddress;
   else   if ((status = moss_compare_oid( attr_oid, &atEntry_ATTR_atDummy_SNP)) == MAN_C_EQUAL)
     return MAN_C_NO_SUCH_ATTRIBUTE_ID;
  else 
      {
#ifdef MOMGENDEBUG
      printf("*** Attribute OID not found ***\n");
#endif /* MOMGENDEBUG */
      return MAN_C_PROCESSING_FAILURE;
      }
  
  return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_get_list_item
**
**	This routine adds the attribute value from the class-specific 
**	instance structure to the attribute reply AVL.
**
**  FORMAL PARAMETERS:
**
**	attribute 	     - Integer containing unique attribute number.  
**	instance   	     - Pointer to class-specific instance structure.
**	reply_attribute_list - Pointer to AVL containing attribute values to return.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 	     - if attribute was added to reply AVL successfully.
**	Any error status from moss_avl_add.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status atEntry_get_list_item( attribute,
						 instance,
						 reply_attribute_list )

int		     attribute;
atEntry_DEF  *instance;
avl		     *reply_attribute_list;

{
    octet_string octet;
    man_status	 status = MAN_C_SUCCESS;
    int 	 last = FALSE;

    switch( attribute )
    {
        case atEntry_ATTR_atIfIndex:
            status = copy_signed_int_to_octet( &octet, &instance->atIfIndex  );
            octet.data_type = ASN1_C_INTEGER;
            status = moss_avl_add(reply_attribute_list,
#ifdef DNA_CMIP_OID
                                  &atEntry_ATTR_atIfIndex_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                                  &atEntry_ATTR_atIfIndex_SNP,
#endif /* SNMP_OID */
                                  (unsigned int) MAN_C_SUCCESS,
                                  ASN1_C_INTEGER,
                                  &octet);
            break;
        case atEntry_ATTR_atNetAddress:
            octet.string = instance->atNetAddress;
            octet.length = instance->atNetAddress_len;
            octet.data_type = INET_C_SMI_IP_ADDRESS;
            status = moss_avl_add(reply_attribute_list,
#ifdef DNA_CMIP_OID
                                  &atEntry_ATTR_atNetAddress_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                                  &atEntry_ATTR_atNetAddress_SNP,
#endif /* SNMP_OID */
                                  (unsigned int) MAN_C_SUCCESS,
                                  INET_C_SMI_IP_ADDRESS,
                                  &octet);
            break;
        case atEntry_ATTR_atPhysAddress:
            octet.string = instance->atPhysAddress;
            octet.length = instance->atPhysAddress_len;
            octet.data_type = ASN1_C_OCTET_STRING;
            status = moss_avl_add(reply_attribute_list,
#ifdef DNA_CMIP_OID
                                  &atEntry_ATTR_atPhysAddress_DNA,
#endif /* DNA_CMIP_OID */
#ifdef SNMP_OID
                                  &atEntry_ATTR_atPhysAddress_SNP,
#endif /* SNMP_OID */
                                  (unsigned int) MAN_C_SUCCESS,
                                  ASN1_C_OCTET_STRING,
                                  &octet);
            break;
    }

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_get_all_attributes
**
**	This routine calls atEntry_get_list_item for each attribute.
**
**  FORMAL PARAMETERS:
**
**	instance 	     - Pointer to class-specific instance structure.
**	reply_attribute_list - Pointer to AVL containing attribute values to return.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 
**	Any error from atEntry_get_list_item.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status atEntry_get_all_attributes( instance,
			           		      reply_attribute_list )

void	    *instance;
avl	    *reply_attribute_list;

{
    man_status status;

    status = atEntry_get_list_item(atEntry_ATTR_atIfIndex          , instance, reply_attribute_list);
    if ERROR_CONDITION( status )
        return status;

    status = atEntry_get_list_item(atEntry_ATTR_atNetAddress       , instance, reply_attribute_list);
    if ERROR_CONDITION( status )
        return status;

    status = atEntry_get_list_item(atEntry_ATTR_atPhysAddress      , instance, reply_attribute_list);
    if ERROR_CONDITION( status )
        return status;


    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_get_attribute_value
**
**	This routine calls loops through the attribute OID looking to 
**	see which attributes need to be returned.  If any errors occur,
**	a AVL containing the attribute and error is is returned.
**
**  FORMAL PARAMETERS:
**
**	attr_oid	     - Pointer to attribute OID containing specific attribute (or attr group).
**	instance 	     - Pointer to class-specific instance structure.
**	reply_attribute_list - Pointer to AVL containing attribute values to return.
**	reply_status	     - Address of integer containing reply to return.
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 
**	Any error from atEntry_get_list_item.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
static man_status atEntry_get_attribute_value( attr_oid,
						       instance,
						       reply_attribute_list,
						       reply_status )

object_id	*attr_oid;
void		*instance;
avl		*reply_attribute_list;
reply_type	*reply_status;

{
    man_status	status;
    unsigned int attribute;

    status = atEntry_get_attr_oid( attr_oid, &attribute );
    if ERROR_CONDITION(status)
        return status;

    switch(attribute)
    {
                case atEntry_ATTR_atIfIndex:
                case atEntry_ATTR_atNetAddress:
                case atEntry_ATTR_atPhysAddress:
	    status = atEntry_get_list_item( attribute, 
						    instance, 
						    reply_attribute_list);
	    break;
	default:
	    status = (man_status) _list_error(attr_oid, 
					reply_attribute_list, 
					(unsigned int *) reply_status, 
					(reply_type) MAN_C_NO_SUCH_ATTRIBUTE_ID, 
			 		(reply_type) MAN_C_GET_LIST_ERROR);
    }

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      atEntry_perform_get
**
**	This routine performs the class-specific get function by determining
**	if all attributes are requested or if a specific attribute is
**	requested.
**
**  FORMAL PARAMETERS:
**
**	unneeded_oid	     _ Pointer to unused OID.
**	attribute_identifier_list - Pointer to AVL containing which attribute (or attribute group) to get.
**	instance 	     - Pointer to class-specific instance structure.
**	reply_attribute_list - Address of pointer to AVL containing attribute values to return.
**	reply_status	     - Address of integer containing reply to return.
**	get_response_oid     - Address of pointer to return any class-specific responses (if needed).
**
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 
**	Any error from the get attribute routines.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  atEntry_perform_get( unneeded_oid,
					 attribute_identifier_list,
					 instance,
					 reply_attribute_list,
					 reply_status,
					 get_response_oid )

object_id	*unneeded_oid;
avl		*attribute_identifier_list;
void		*instance;
avl		**reply_attribute_list;
reply_type	*reply_status;
object_id	**get_response_oid;

{
    object_id	    *oid;
    int		    last_one;
    man_status	    status;

    status = moss_avl_init( reply_attribute_list );
    if ERROR_CONDITION( status )
        return status;

    status = moss_avl_reset( attribute_identifier_list );
    if ERROR_CONDITION( status )
	return status;

    *reply_status = (reply_type) MAN_C_SUCCESS;
    do
    {
	/*
	 * Point to each element.  We are only interested in the
	 * object identifier of the attribute.
	 */
	status = moss_avl_point(attribute_identifier_list, &oid, NULL, NULL, NULL, &last_one);
	if ((ERROR_CONDITION( status )) && (status != MAN_C_NO_ELEMENT))
	    return status;

	if (status == MAN_C_NO_ELEMENT)
	    /*
	     * Get all the attributes if no specific ones are requested 
	     * (i.e. NO_ELEMENT was returned from moss_avl_point).
	     */
	    status = atEntry_get_all_attributes( instance, 
                                                         *reply_attribute_list);
	else
	    status = atEntry_get_attribute_value( oid, 
                                                          instance, 
                                                          *reply_attribute_list, 
                                                          reply_status);
    } while (!last_one && (status == MAN_C_SUCCESS));
 
    return status;
}
