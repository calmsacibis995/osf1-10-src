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
static char *rcsid = "@(#)$RCSfile: build_access.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:54:04 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**                                                                      
**	ACCESS.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides routines to perform access checking.
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
**--
*/

#include "moss.h"
#include "common.h"


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      check_access
**
**	    This routine checks for access to create an instance.  It 
**	    is called from create_instance to verify that the instance can
**	    be created.
**
**  FORMAL PARAMETERS:
**
**	object_class        - Class of the object to be created.
**	object_instance     - Instance of the object to be created.
**	superior_instance   - Instance of the object's superior.  This is optional.
**	reference_instance  - Pointer to an AVL containing the instance name of an entity
**	    		      of the same class, to be used as a template for filling in
**	    		      default attribute values.
**
**	ACCESS_CONTROL
**	    Access control information
**
**  RETURN VALUE:
**
**      MAN_C_SUCCESS - instance can be created.
**	MAN_C_ACCESS_DENIED - instance cannot be created.
**
**--
*/
man_status check_access(object_class,
			object_instance,
			superior_instance,
			reference_instance,
			access_control)

object_id   *object_class;
avl	    *object_instance;
avl	    *superior_instance;
avl	    *reference_instance;
avl	    *access_control;

{

    /** Verify access to create instance **/

    return MAN_C_SUCCESS;
} 

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      check_access_instance 
**
**	This routine checks access to a specific instance.
**
**  FORMAL PARAMETERS:
**
**	class_code        - Instance class to check
**	specific_instance - Instance to check access for
**	access_control    - Access control information
**
**  RETURN VALUE:
**
**      MAN_C_SUCCESS - instance can be accessed
**	MAN_C_ACCESS_DENIED - instance cannot be accessed
**
**--
*/
man_status check_access_instance( class_code,
				  specific_instance,
				  access_control)

int 	    class_code;
void	    *specific_instance;
avl	    *access_control;

/* +++

  Optional change.  Add security access check here as required by
  your application.

*/
{
    /** Verify access to this instance **/

    return MAN_C_SUCCESS;
}
