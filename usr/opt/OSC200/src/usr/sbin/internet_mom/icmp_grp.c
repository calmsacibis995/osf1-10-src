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
static char *rcsid = "@(#)$RCSfile: icmp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:41:57 $";
#endif
/*
 */
/*
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: icmp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:41:57 $";
#endif
/*
 */
/* Facility:
 *
 *
 * Abstract:
 *
 *	This file contain routines to extract information about the ICMP
 *	class. 
 *	
 * Author:
 *
 *
 * Revision History:
 *
 */
#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "inet_mom_specific.h"


man_status refresh_icmp_list(
			     icmp_DEF *new_icmp_header
			    )

/*    Function Description:
 *    	This function seeks the icmpstat structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	       new_icmp_header	Input/Output : A pointer to the "icmp" structure
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
    icmp_DEF *icmp_instance = NULL;
    icmp_blk *icmp_stats;
    struct strioctl str;
    man_status status;


    icmp_stats = (icmp_blk *) malloc(sizeof(icmp_blk));
    if (icmp_stats == NULL)
	return(MAN_C_INSUFFICIENT_RESOURCES);
 
    if (new_icmp_header->next == new_icmp_header){
       icmp_instance = (icmp_DEF *)malloc(sizeof(icmp_DEF));
       if (icmp_instance == NULL)
	  return(MAN_C_INSUFFICIENT_RESOURCES);
 
       /* Insert new struct at end of list */
       status = icmp_add_new_instance(new_icmp_header,
				      icmp_instance );
       if (ERROR_CONDITION(status))
	   return(status);
    }else
        icmp_instance = new_icmp_header->next;


    bzero(icmp_stats, sizeof(icmp_blk));
    str.ic_cmd = KINFO_GET_ICMP;
    str.ic_timout = 15;
    str.ic_len = sizeof(icmp_blk);
    str.ic_dp = (char *)icmp_stats;
    if (ioctl(inetfd, I_STR, &str) < 0) {
        perror(" ioctl");
	free(icmp_stats);
	return(MAN_C_PROCESSING_FAILURE);
    }
    icmp_instance->icmpInMsgs = icmp_stats->in_msgs ;
    icmp_instance->icmpInErrors = icmp_stats->in_icmp_errors;
    icmp_instance->icmpInDestUnreachs = icmp_stats->in_unreach;
    icmp_instance->icmpInTimeExcds = icmp_stats->in_timeexcds;
    icmp_instance->icmpInParmProbs = icmp_stats->in_parmprobs;
    icmp_instance->icmpInSrcQuenchs = icmp_stats->in_srcquenchs;
    icmp_instance->icmpInRedirects = icmp_stats->in_redirects;
    icmp_instance->icmpInEchos = icmp_stats->in_echos;
    icmp_instance->icmpInEchoReps = icmp_stats->in_echoreps;
    icmp_instance->icmpInTimestamps = icmp_stats->in_tstamps;
    icmp_instance->icmpInTimestampReps = icmp_stats->in_tstampreply;
    icmp_instance->icmpInAddrMasks = icmp_stats->in_maskreqs;
    icmp_instance->icmpInAddrMaskReps = icmp_stats->in_maskreps;
    icmp_instance->icmpOutMsgs = icmp_stats->out_msgs;
    icmp_instance->icmpOutErrors = icmp_stats->out_errors;
    icmp_instance->icmpOutDestUnreachs = icmp_stats->out_unreach;
    icmp_instance->icmpOutTimeExcds = icmp_stats->out_timeexcds;
    icmp_instance->icmpOutParmProbs = icmp_stats->out_parmprobs;
    icmp_instance->icmpOutSrcQuenchs = icmp_stats->out_srcquenchs;
    icmp_instance->icmpOutRedirects = icmp_stats->out_redirects;
    icmp_instance->icmpOutEchos = icmp_stats->out_echos;
    icmp_instance->icmpOutEchoReps = icmp_stats->out_echoreps;
    icmp_instance->icmpOutTimestamps = icmp_stats->out_tstamps;
    icmp_instance->icmpOutTimestampReps = icmp_stats->out_tstampreply;
    icmp_instance->icmpOutAddrMasks = icmp_stats->out_maskreqs;
    icmp_instance->icmpOutAddrMaskReps = icmp_stats->out_maskreps;
    icmp_instance->instance_name_length = 0;
    icmp_instance->instance_name = (char *)NULL;
    memset((void *)&icmp_instance->instance_uid, '\0', sizeof(uid));
    icmp_instance->object_instance = (avl *)NULL;

    free(icmp_stats);
    return(MAN_C_SUCCESS);
} 

/* end of icmp_grp.c */
