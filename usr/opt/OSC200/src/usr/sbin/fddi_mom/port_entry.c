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
static char *rcsid = "@(#)$RCSfile: port_entry.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/18 16:09:35 $";
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
#include "fddi_mom_specific.h"

extern int indx ;

static
unsigned int
	port_chipset_id[2] = {0,0};
static
object_id
	port_chipset_id_oid = {2, port_chipset_id };

man_status refresh_portEntry_list(
			    snmpFddiPORTEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the port_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the port structure
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
   
    snmpFddiPORTEntry_DEF *new_instance;
    struct index_info *fddi_index_ptr;   
    int status;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
            free(new_instance->prev);
      }
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next);
    }

    header->next = header->prev = header;

    create_fddi_index_info();

    indx = 0;
    /*
     * Check if any FDDI Interfaces have been configured
     */

    if (fddi_index_info.index == NULL) {
	return(MAN_C_PROCESSING_FAILURE);
    }

    fddi_index_ptr = fddi_index_info.index ;

    while(fddi_index_ptr != NULL){

    indx = 0;
    if ((status =
        find_info_by_fddi_index(fddi_index_ptr->port_index,FDDIMIB_PORT))
         != MAN_C_SUCCESS)
       return(status);

    new_instance = 
             (snmpFddiPORTEntry_DEF *)malloc(sizeof(snmpFddiPORTEntry_DEF));
    if (new_instance == NULL) {
       free(new_instance);
       return(MAN_C_INSUFFICIENT_RESOURCES);
     }
 
     /* fill in the instance info */
 

        new_instance->snmpFddiPORTSMTIndex =
	 ctr_info.ctr_ctrs.port_fddi.port_smt_index;
        new_instance->snmpFddiPORTIndex =
	 ctr_info.ctr_ctrs.port_fddi.port_index;
        new_instance->snmpFddiPORTCEState =
	 ctr_info.ctr_ctrs.port_fddi.port_CEstate;
        new_instance->snmpFddiPORTConnectState =
       	 ctr_info.ctr_ctrs.port_fddi.port_connectstate;
	new_instance->snmpFddiPORTPCMState =
       	 ctr_info.ctr_ctrs.port_fddi.port_PCMstate;
	new_instance->snmpFddiPORTLCTFailCts =
       	 ctr_info.ctr_ctrs.port_fddi.port_LCTfail;
	new_instance->snmpFddiPORTLemRejectCts =
       	 ctr_info.ctr_ctrs.port_fddi.port_Lemreject;
	new_instance->snmpFddiPORTLemCts =
       	 ctr_info.ctr_ctrs.port_fddi.port_Lem;
	new_instance->snmpFddiPORTPCType =
         ctr_info.ctr_ctrs.port_fddi.port_pctype;
	new_instance->snmpFddiPORTPCNeighbor =
         ctr_info.ctr_ctrs.port_fddi.port_pcneighbor;
	new_instance->snmpFddiPORTConnectionPolicies =
         ctr_info.ctr_ctrs.port_fddi.port_connpolicy;
	new_instance->snmpFddiPORTRemoteMACIndicated =
      	 ctr_info.ctr_ctrs.port_fddi.port_remoteind; 
	new_instance->snmpFddiPORTPathsRequested =
       	 ctr_info.ctr_ctrs.port_fddi.port_pathreq;
	new_instance->snmpFddiPORTMACPlacement =
       	 ctr_info.ctr_ctrs.port_fddi.port_placement;
	new_instance->snmpFddiPORTAvailablePaths =
       	 ctr_info.ctr_ctrs.port_fddi.port_availpaths;
	new_instance->snmpFddiPORTMACLoopTime =
       	 ctr_info.ctr_ctrs.port_fddi.port_looptime;
	new_instance->snmpFddiPORTTBMax =
       	 ctr_info.ctr_ctrs.port_fddi.port_TBmax;
	new_instance->snmpFddiPORTBSFlag =
       	 ctr_info.ctr_ctrs.port_fddi.port_BSflag;
	new_instance->snmpFddiPORTLerEstimate =
       	 ctr_info.ctr_ctrs.port_fddi.port_LerrEst;
	new_instance->snmpFddiPORTLerCutoff =
       	 ctr_info.ctr_ctrs.port_fddi.port_Lercutoff;
	new_instance->snmpFddiPORTLerAlarm =
       	 ctr_info.ctr_ctrs.port_fddi.port_alarm;
	new_instance->snmpFddiPORTPCWithhold =
       	 ctr_info.ctr_ctrs.port_fddi.port_PCwithhold;
	new_instance->snmpFddiPORTLerCondition =
       	  ctr_info.ctr_ctrs.port_fddi.port_Lercondition;
	new_instance->snmpFddiPORTChipSet = &port_chipset_id_oid;
	new_instance->snmpFddiPORTAction =
       	 ctr_info.ctr_ctrs.port_fddi.port_action;
       new_instance->instance_name = (char *)&indx;
       new_instance->instance_name_length = sizeof(int);
       memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
       new_instance->object_instance = (avl *)NULL;

     /* insert new struct at the end of the list */

     snmpFddiPORTEntry_add_new_instance(header->prev, new_instance);
     fddi_index_ptr = fddi_index_ptr->next;
    }

    return(MAN_C_SUCCESS);
}
