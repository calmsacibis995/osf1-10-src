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
static char *rcsid = "@(#)$RCSfile: build_perform_delete.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:58:16 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**      [[class_prefix]]PERFORM_DELETE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the routines to actually perform the DELETE
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
#include "common.h"
[[extern_common]]

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      [[class_prefix]]perform_delete
**
**	This routine performs the class-specific delete function by removing
**	the instance from the instance queue and deallocating any memory
**	associated with that instance.	
**
**	NOTE: The MOM may need to deallocate any resources stored outside of
**	this process.
**
**  FORMAL PARAMETERS:
**
**	unneeded_oid 	     - OID not currently used.
**	modification_list    - Pointer to AVL containing any delete arguments.
**	instance	     - Pointer to class-specific instance structure.
**	reply_attribute_list - Address of pointer to return reply AVL.
**	reply_status	     - Address of integer to return reply status.
**	action_response_type_oid - Address of pointer to return response OID.
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
man_status  [[class_prefix]]perform_delete( unneeded_oid,
					    modification_list,
					    instance,
					    reply_attribute_list,
					    reply_status,
					    action_response_type_oid )

object_id		*unneeded_oid;
[[sp]]avl		*modification_list;
[[class_void_name]]	*instance;
[[sp]]avl		**reply_attribute_list;
[[sp]]reply_type	*reply_status;
object_id		**action_response_type_oid;

{
    object_id		*oid;
    unsigned int	modifier;
    unsigned int	tag;
    octet_string	*octet;
    int			last_one;
    man_status		status;
    
    status = moss_avl_init( reply_attribute_list );
    if ERROR_CONDITION( status )
        return status;

    /** 
     ** This section may not be needed if the MOM is storing instance data
     ** outside of this process 
     **/
    /*-insert-code-delete-instance-*/

    *reply_status = (reply_type) MAN_C_SUCCESS;
    /*-insert-code-delete-success-oid-*/    

    return MAN_C_SUCCESS;
}			 
