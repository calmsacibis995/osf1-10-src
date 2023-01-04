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
static char *rcsid = "@(#)$RCSfile: at_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:40:01 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: at_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:40:01 $";
#endif
/*
 */
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
#include "inet_mom_specific.h"

man_status refresh_at_list(
			    at_DEF *new_at_header
			    )

/*    Function Description:
 *    	This function seeks the at_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the at structure
 *				       that needs to be filled in.
 *    Return Value:
 *   
 *         MAN_C_SUCCESS
 *	   MAN_C_FAILURE
 *    
 *    Side Effects:
 *	None.
 */
{
    at_DEF *at_instance = NULL;
    man_status status;

    /* check if first time */

	if (new_at_header->next == new_at_header)
	{
	 at_instance = (at_DEF *)malloc(sizeof(at_DEF));
	 if (at_instance == NULL)
	    return(MAN_C_INSUFFICIENT_RESOURCES);
	
         status = at_add_new_instance(new_at_header,
				      at_instance);
	 if (ERROR_CONDITION(status))
           return(status);
	 } else
	      at_instance = new_at_header->next;

    at_instance->instance_name_length = 0;
    at_instance->instance_name = (char *)NULL;
    memset( (void *)&at_instance->instance_uid, '\0', sizeof(uid));
    at_instance->object_instance = (avl *)NULL ;

    return(MAN_C_SUCCESS);
}
