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
static char *rcsid = "@(#)$RCSfile: build_perform_create.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:58:00 $";
#endif
/****************************************************************************** *
****
****		This module is not used anymore...
****		Use build_perform_create_instance.c.
****
*******************************************************************************/

/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
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
#include "descrip.h"
#include "common.h"
[[extern_common]]

/*-insert-code-add-new-instance-*/

/*-insert-code-map-oid-*/

/*-insert-code-add-check-attributes-*/

man_status [[class_prefix]]locate_instance( instance_name,
					    return_instance_ptr )

char				*instance_name;
[[class_void_name]]		**return_instance_ptr;

{

/** This routine returns the instance pointer if a match is found **/

return MAN_C_FALSE;

}

man_status  [[class_prefix]]perform_create( instance,
					    instance_length,
					    ref_instance_ptr,
					    attribute_list )

char			*instance;
int			instance_length;
[[class_void_name]]	**ref_instance_ptr;
avl			*attribute_list;

{
    object_id		*oid;
    unsigned int	modifier;
    unsigned int	tag;
    octet_string	*octet;
    int			last_one;
    unsigned int 	argument;
    man_status		status;
    
  /** Create the new instance, set up ref_instance_ptr. **/

  /**
   * Create the UID
   **/

  /*
   * Check to see if the attribute list exists.
   */
  status = (man_status) _moss_avl_supplied(attribute_list);
  if (status == MAN_C_TRUE) {
    status = moss_avl_reset(attribute_list);
    if (ERROR_CONDITION(status))
      return status;

   /* 
    * Get the characteristic attribute values
    */

    do {
      status = moss_avl_point(attribute_list, &oid, &modifier, &tag, &octet, NULL);
      if (!ERROR_CONDITION(status)) {
	[[class_prefix]]map_oid_to_arg(oid, &argument);
	switch (argument)
	  {
	  /*-insert-code-map-oid-list-*/
	  default:
	    return MAN_C_NO_SUCH_ATTRIBUTE_ID;
          }
       }  
    } while (!ERROR_CONDITION(status));
  }

  /*-insert-code-add-new-instance-call-*/

  return MAN_C_SUCCESS;
}			 
