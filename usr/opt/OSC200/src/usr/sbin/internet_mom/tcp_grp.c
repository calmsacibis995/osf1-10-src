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
static char *rcsid = "@(#)$RCSfile: tcp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:45:54 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: tcp_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:45:54 $";
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


man_status refresh_tcp_list(
			    tcp_DEF *new_tcp_header
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
    tcp_DEF *tcp_instance = NULL;
    tcp_blk *tcps;
    struct strioctl str;
    man_status status;

    tcps = (tcp_blk *) malloc(sizeof(tcp_blk));
    if (tcps == NULL)
        return(MAN_C_INSUFFICIENT_RESOURCES);

    if (new_tcp_header->next == new_tcp_header){
        tcp_instance = (tcp_DEF *)malloc(sizeof(tcp_DEF));
        if (tcp_instance == NULL)
           return(MAN_C_INSUFFICIENT_RESOURCES); 
        status = tcp_add_new_instance(new_tcp_header,
				      tcp_instance) ;
	if (ERROR_CONDITION(status))
           return(status);
      }else
	  tcp_instance = new_tcp_header->next;
	   

    bzero(tcps, sizeof(tcp_blk));
    str.ic_cmd = KINFO_GET_TCP;
    str.ic_timout = 15;
    str.ic_len = sizeof(tcp_blk);
    str.ic_dp = (char *)tcps;
    if (ioctl(inetfd, I_STR, &str) < 0) {
        perror(" ioctl");
	free(tcps);
        return(MAN_C_PROCESSING_FAILURE);
    }
    tcp_instance->tcpActiveOpens = tcps->active_opens; 
    tcp_instance->tcpPassiveOpens = tcps->passive_opens;
    tcp_instance->tcpAttemptFails = tcps->attempt_fails;
    tcp_instance->tcpEstabResets = tcps->estab_resets;
    tcp_instance->tcpCurrEstab = tcps->curr_estab;
    tcp_instance->tcpInSegs = tcps->in_segs;
    tcp_instance->tcpOutSegs = tcps->out_segs;
    tcp_instance->tcpRetransSegs = tcps->retrans_segs;
    tcp_instance->tcpInErrs = tcps->in_errors;
    tcp_instance->tcpOutRsts = tcps->out_rsts;
    tcp_instance->tcpRtoAlgorithm = MOM_C_VAN_JACOBSONS;
    tcp_instance->tcpRtoMin = TCPTV_MIN * 1000; 
    tcp_instance->tcpRtoMax = TCPTV_REXMTMAX * 1000;
    tcp_instance->tcpMaxConn = MOM_C_TCP_MAXCONN_VALUE;
    tcp_instance->instance_name_length = 0;
    tcp_instance->instance_name = (char *)NULL;
    memset( (void *)&tcp_instance->instance_uid, '\0', sizeof(uid));
    tcp_instance->object_instance = (avl *)NULL ;

    free(tcps);
    return(MAN_C_SUCCESS);
}
