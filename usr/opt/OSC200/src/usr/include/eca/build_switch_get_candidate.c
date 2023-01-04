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
static char *rcsid = "@(#)$RCSfile: build_switch_get_candidate.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:59:45 $";
#endif
/*
**++
**  FACILITY:  [[facility]]
**
**  Copyright (c) [[copyright_date]]  [[copyright_owner]]
**
**  MODULE DESCRIPTION:
**
**	GET_INSTANCE.C
**
**      This module is part of the Managed Object Module (MOM)
**	for [[mom_name]].
**	It provides the entry point for the GET_INSTANCE function
**	which will then dispatch for the code for the appropriate
**	class.
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
**  TEMPLATE HISTORY:
**
**--
*/

#include "man_data.h"
#include "man.h"
#include "moss.h"
#include "common.h"
[[extern_common]]


man_status get_instance( object_class,
			 object_instance,
			 iso_scope,
			 specific_instance,
			 specific_object_instance,
			 object_uid,
  			 more_instances,
			 return_context,
			 build_avl,
			 class_code )

object_id	    *object_class;
avl		    *object_instance;
scope		    iso_scope;
void		    **specific_instance;
avl		    **specific_object_instance;
uid		    **object_uid;
int		    *more_instances;
GET_CONTEXT_STRUCT  **return_context;
int		    build_avl;
int		    class_code;

{
    man_status	    status;

    /*
     * Switch to the "get_instance" code which is appropriate for this
     * entity class.
     */

/*+++
**     The following routine supports multiple
**     classes.  You can simplify it if your
**     MOM supports only one class.
**/

    switch (class_code)
    {
    /*-insert-code-switch-get-candidate-*/
	default:
	    status = MAN_C_PROCESSING_FAILURE;
    }
    return status;
}
