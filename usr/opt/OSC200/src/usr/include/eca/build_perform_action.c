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
static char *rcsid = "@(#)$RCSfile: build_perform_action.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:57:50 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	[[class_prefix]]PERFORM_[[action_directive]].C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the ACTION function.
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
**--
*/

#include "moss.h"
#include "common.h"

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]perform_[[action_directive]]
**
**	This routine is generated as a template and/or guideline for 
**	verifying and [[action_directive]] attributes.  
**
**	This routine requires modifications for it to be complete.
**
**  FORMAL PARAMETERS:
**
**	unneeded_oid	  - Pointer to object ID (used for compatibility for other perform  routines.
**	modification_list - Pointer to AVL containing attributes for this action.
**	instance	  - Address of class specific instance structure.
**	reply_attribute_list - Address of pointer to reply AVL.
**	reply_status	  - Address of reply to return back.
**	action_response_type_oid - Address of pointer to success or failure OID
**				    This can be a response for success or
**				    exception for failure.
**  RETURN VALUES:
**
**      MAN_C_SUCCESS 	  	 - if action was performed.
**	MAN_C_PROCESSING_FAILURE - if action was not successfully performed.
**
**  SIDE EFFECTS:
**
**      None
**
**--
*/
man_status  [[class_prefix]]perform_[[action_directive]]( unneeded_oid,
							  modification_list,
							  instance,
							  reply_attribute_list,
							  reply_status,
							  action_response_type_oid )

object_id	*unneeded_oid;
avl		*modification_list;
void		*instance;
avl		**reply_attribute_list;
reply_type	*reply_status;
object_id	**action_response_type_oid;

{
  object_id		*oid;
  unsigned int  	modifier;
  unsigned int  	tag;
  octet_string	*octet;
  int			last_one;
  man_status		status;
  
  status = moss_avl_init(reply_attribute_list);
  if ERROR_CONDITION(status)
    return status;
  
  status = moss_avl_reset(modification_list);
  if ERROR_CONDITION(status)
    return status;
  
  *reply_status = (reply_type)MAN_C_SUCCESS;
  
  do
      {
	/*
	 * Point to each element.  We are interested in the
	 * object identifier, the modifier value, and the
	 * new value(s) of the argument.
	 */
	status = moss_avl_point(modification_list,
				&oid,
				&modifier,
				&tag,
				&octet,
				&last_one);
	
	if (status == MAN_C_SUCCESS)
	  {
	  /**
	   ** Validate the action arguments and values.  If successful,
	   ** perform the action on this argument.
	   **/
	  }
	  else if (status == MAN_C_NO_ELEMENT) /* Empty AVL */
	        status = MAN_C_SUCCESS; 
      } while (!last_one && (status == MAN_C_SUCCESS));
  

  if (status != MAN_C_SUCCESS)
    {
    /**
     ** Map the error into the appropriate exception. Any exception arguments 
     ** need to be added to the reply_attribute_list AVL.
     **/

    *reply_status = (reply_type) MAN_C_PROCESSING_FAILURE;

    /*-insert-code-action-exception-*/

    return status;
    }    

  /** 
   ** Map the success into the appropriate response (if there are multiple
   ** responses). Any response arguments need be added to the 
   ** reply_attribute_list AVL.
   **/

  /*-insert-code-action-success-oid-*/

  return status;
}			 
