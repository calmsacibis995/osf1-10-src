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
static char *rcsid = "@(#)$RCSfile: udp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:46:48 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: udp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:46:48 $";
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

man_status refresh_udp_list(
			    udp_DEF *new_udp_header
			   )

/*    Function Description:
 *    	This function seeks the udp_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the udp structure
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
    udp_DEF *udp_instance = NULL;
    udp_blk *udps;
    struct strioctl str;
    man_status status;

    udps = (udp_blk *) malloc(sizeof( udp_blk));
    if (udps == NULL)
        return(MAN_C_INSUFFICIENT_RESOURCES);

    /* check if first time */

    if (new_udp_header->next == new_udp_header)
    {
	 udp_instance = (udp_DEF *)malloc(sizeof(udp_DEF));
	 if (udp_instance == NULL)
	    return(MAN_C_INSUFFICIENT_RESOURCES);
	 
	 status = udp_add_new_instance(new_udp_header,
				       udp_instance);
	 if (ERROR_CONDITION(status))
           return(status);
    }else
         udp_instance = new_udp_header->next;



    bzero(udps, sizeof(udp_blk));
    str.ic_cmd = KINFO_GET_UDP;
    str.ic_timout = 15;
    str.ic_len = sizeof(udp_blk);
    str.ic_dp = (char *)udps;
    if (ioctl(inetfd, I_STR, &str) < 0) {
        perror(" ioctl");
	free(udps);
        return(MAN_C_PROCESSING_FAILURE);
    }
    udp_instance->udpInDatagrams = udps->in_datagrams; 
    udp_instance->udpNoPorts = udps->no_ports;
    udp_instance->udpInErrors = udps->in_errors;
    udp_instance->udpOutDatagrams = udps->out_datagrams;
    udp_instance->instance_name_length = 0;
    udp_instance->instance_name = (char *)NULL;
    memset( (void *)&udp_instance->instance_uid, '\0', sizeof(uid));
    udp_instance->object_instance = (avl *)NULL ;

    free(udps);
    return(MAN_C_SUCCESS);
}
