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
static char *rcsid = "@(#)$RCSfile: EXP_dot5_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/13 21:12:56 $";
#endif
/*
 *
 * Facility:
 *
 *    Management 
 *
 * Abstract:
 *
 *	
 * Author:
 *
 * Date:
 *
 *
 * Revision History:
 *
 */
#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "dot5_mom_specific.h"


man_status refresh_EXP_dot5_list(
			    EXP_dot5_DEF *new_EXP_dot5_header
			    )

{
   
    EXP_dot5_DEF *EXP_dot5_instance = NULL;
    struct dot5_info *dot5_index_ptr;
    man_status status;
   

    /* check if first time */

    if (new_EXP_dot5_header->next == new_EXP_dot5_header) 
    {
	EXP_dot5_instance = (EXP_dot5_DEF *)malloc(sizeof(EXP_dot5_DEF));
	if (EXP_dot5_instance == NULL)
		return(MAN_C_INSUFFICIENT_RESOURCES);

	/* Insert new struct at end of list */
	status = EXP_dot5_add_new_instance(new_EXP_dot5_header,
						EXP_dot5_instance);
	if (ERROR_CONDITION(status))
		return(status);
    }
    else
	EXP_dot5_instance = new_EXP_dot5_header->next;



     /* fill in the instance info */
 
     EXP_dot5_instance->instance_name_length = 0;
     EXP_dot5_instance->instance_name = (char *)NULL;
     memset((void *)&EXP_dot5_instance->instance_uid, '\0', sizeof(uid));
     EXP_dot5_instance->object_instance = (avl *)NULL;

     return(MAN_C_SUCCESS);
}
