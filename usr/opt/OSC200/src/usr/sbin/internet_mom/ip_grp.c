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
static char *rcsid = "@(#)$RCSfile: ip_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:43:13 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ip_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:43:13 $";
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

#define FORWARDING     1  /* kernel sets ipforwarding = 1, if it's a gateway */
#define NOT_FORWARDING 2  /* kernel sets ipforwarding = 0, if it's a host    */

man_status refresh_ip_list(
			   ip_DEF *new_ip_header
			   )

/*    Function Description:
 *    	This function seeks the ip_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the ip structure
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
    ip_DEF *ip_instance = NULL;
    ip_blk *ips;
    struct strioctl str;
    man_status status;

    ips = (ip_blk *) malloc(sizeof(ip_blk));
    if (ips == NULL)
        return(MAN_C_INSUFFICIENT_RESOURCES);

    if (new_ip_header->next == new_ip_header){
	ip_instance = (ip_DEF *)malloc(sizeof(ip_DEF));
        if (ip_instance == NULL)
	   return(MAN_C_INSUFFICIENT_RESOURCES);
        status = ip_add_new_instance(new_ip_header,
				     ip_instance);
	if (ERROR_CONDITION(status))
           return(status);
    }else
        ip_instance = new_ip_header->next;

    bzero(ips, sizeof(ip_blk));
    str.ic_cmd = KINFO_GET_IP;
    str.ic_timout = 15;
    str.ic_len = sizeof(ip_blk);
    str.ic_dp = (char *)ips;
    if (ioctl(inetfd, I_STR, &str) < 0) {
        perror(" ioctl");
	free(ips);
        return(MAN_C_PROCESSING_FAILURE);
    }
    ip_instance->ipInReceives = ips->in_receives; 
    ip_instance->ipInHdrErrors = ips->in_hdr_errors;
    ip_instance->ipInAddrErrors = ips->in_addr_errors;
    ip_instance->ipForwDatagrams = ips->forw_datagrams;

    /* This value is always zero, as ALL unknown protocols
     * go to RAW IP and thus never come to this layer.
    */
    ip_instance->ipInUnknownProtos = 0;

    /* This value is always zero. */  
    ip_instance->ipInDiscards = 0;

    ip_instance->ipInDelivers = ips->in_delivers;
    ip_instance->ipOutRequests = ips->out_requests;
    ip_instance->ipOutDiscards = ips->out_discards;
    ip_instance->ipOutNoRoutes = ips->out_noroutes;
    ip_instance->ipReasmReqds = ips->reasm_reqds;
    ip_instance->ipReasmOKs = ips->reasm_OKs;
    ip_instance->ipReasmFails = ips->reasm_fails;
    ip_instance->ipFragOKs = ips->frag_OKs;
    ip_instance->ipFragFails = ips->frag_fails;
    ip_instance->ipFragCreates = ips->frag_creates;
    ip_instance->ipRoutingDiscards = 0;  /* check this ??? */
    if (ips->forwarding)
        ip_instance->ipForwarding = FORWARDING;
    else
        ip_instance->ipForwarding = NOT_FORWARDING;
    ip_instance->ipDefaultTTL = ips->default_ttl;
    ip_instance->ipReasmTimeout = ips->reasm_tout;
    ip_instance->instance_name_length = 0;
    ip_instance->instance_name = (char *)NULL;
    memset( (void *)&ip_instance->instance_uid, '\0', sizeof(uid));
    ip_instance->object_instance = (avl *)NULL ;

    free(ips);
    return(MAN_C_SUCCESS);
}
