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
static char *rcsid = "@(#)$RCSfile: snmp_grp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:22:32 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: snmp_grp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 19:22:32 $";
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
#include "iso_defs.h"
#include "snmp_mib.h"
#include "inet_mom_specific.h"

struct snmp_stats *s_stat ;

man_status refresh_snmp_list(
			     snmp_DEF *new_snmp_header
			     )

/*    Function Descrsnmption:
 *    	This function seeks the snmp_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the snmp structure
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
    snmp_DEF *snmp_instance = NULL;
    man_status status;

    if (new_snmp_header->next == new_snmp_header){
	snmp_instance = (snmp_DEF *)malloc(sizeof(snmp_DEF));
        if (snmp_instance == NULL)
	   return(MAN_C_INSUFFICIENT_RESOURCES);
	status = snmp_add_new_instance(new_snmp_header,
			               snmp_instance);
	if (ERROR_CONDITION(status))
	   return(status);
      }else
	  snmp_instance = new_snmp_header->next;
	
         snmp_instance->snmpInPkts = s_stat->inpkts;
         snmp_instance->snmpOutPkts = s_stat->outpkts;
         snmp_instance->snmpInBadVersions = s_stat->badvers; 
         snmp_instance->snmpInBadCommunityNames = s_stat->badcommname;
         snmp_instance->snmpInBadCommunityUses = s_stat->badcommuse;
         snmp_instance->snmpInASNParseErrs = s_stat->badASN1;
         snmp_instance->snmpInTooBigs = s_stat->intoobig;
         snmp_instance->snmpInNoSuchNames = s_stat->innosuch;
         snmp_instance->snmpInBadValues = s_stat->inbadvalue;
         snmp_instance->snmpInReadOnlys = s_stat->inreadonly;
         snmp_instance->snmpInGenErrs = s_stat->ingenerr;
         snmp_instance->snmpInTotalReqVars = s_stat->totreqvars;
         snmp_instance->snmpInTotalSetVars = s_stat->totsetvars;
         snmp_instance->snmpInGetRequests = s_stat->ingetreq;
         snmp_instance->snmpInGetNexts = s_stat->ingetnxtreq;
         snmp_instance->snmpInSetRequests = s_stat->insetreq;
         snmp_instance->snmpInGetResponses = s_stat->ingetresp;
         snmp_instance->snmpInTraps = s_stat->intraps;
         snmp_instance->snmpOutTooBigs = s_stat->outtoobig;
         snmp_instance->snmpOutNoSuchNames = s_stat->outnosuch;
         snmp_instance->snmpOutBadValues = s_stat->outbadvalue;
         snmp_instance->snmpOutGenErrs = s_stat->outgenerr;
         snmp_instance->snmpOutGetRequests = s_stat->outgetreq;
         snmp_instance->snmpOutGetNexts = s_stat->outgetnxtreq;
         snmp_instance->snmpOutSetRequests = s_stat->outsetreq;
         snmp_instance->snmpOutGetResponses = s_stat->outgetresp;
         snmp_instance->snmpOutTraps = s_stat->outtraps;
         snmp_instance->snmpEnableAuthenTraps = s_stat->enableauthtrap;
         snmp_instance->instance_name_length = 0;
         snmp_instance->instance_name = (char *)NULL;
         memset((void *)&snmp_instance->instance_uid,'\0',sizeof(uid));
         snmp_instance->object_instance = (avl *)NULL;
   
    return(MAN_C_SUCCESS);
}
