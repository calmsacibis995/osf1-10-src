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
static char *rcsid = "@(#)$RCSfile: if_grp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:20:22 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: if_grp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:20:22 $";
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

man_status refresh_if_list(
			   interfaces_DEF *new_interfaces_header
			   )

/*    Function Description:
 *    	This function seeks the if_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the if structure
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
    interfaces_DEF *interfaces_instance = NULL;
    if_blk ifs;
    struct strioctl str;
    man_status status;

    if (new_interfaces_header->next == new_interfaces_header){
        interfaces_instance = (interfaces_DEF *)malloc(sizeof(interfaces_DEF));
	if (interfaces_instance == NULL)
	   return(MAN_C_INSUFFICIENT_RESOURCES);

        status = interfaces_add_new_instance(new_interfaces_header,
					     interfaces_instance);
	if (ERROR_CONDITION(status))
           return(status);
     }else
         interfaces_instance = new_interfaces_header->next;

    bzero(&ifs, sizeof(if_blk));
    str.ic_cmd = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len = sizeof(if_blk);
    str.ic_dp = (char *)&ifs;
    if (ioctl(inetfd, I_STR, &str) < 0) {
        perror(" ioctl");
        return(MAN_C_PROCESSING_FAILURE);
    }
    interfaces_instance->ifNumber = ifs.curr_cnt;  
    interfaces_instance->instance_name_length = 0;
    interfaces_instance->instance_name = (char *)NULL;
    memset( (void *)&interfaces_instance->instance_uid, '\0', sizeof(uid));
    interfaces_instance->object_instance = (avl *)NULL ;

    return(MAN_C_SUCCESS);
}
