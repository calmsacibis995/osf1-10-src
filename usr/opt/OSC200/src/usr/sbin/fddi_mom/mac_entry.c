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
static char *rcsid = "@(#)$RCSfile: mac_entry.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/18 16:07:13 $";
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
        mac_chipset_id[2] = {0,0};
static
object_id
        mac_chipset_id_oid = {2, mac_chipset_id };

man_status refresh_macEntry_list(
			    snmpFddiMACEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the mac_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the mac structure
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
   
    snmpFddiMACEntry_DEF *new_instance;
    struct index_info *fddi_index_ptr;
    int mac_index;
    int status;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->snmpFddiMACUpstreamNbr);
	    free(new_instance->prev->snmpFddiMACOldUpstreamNbr);
            free(new_instance->prev->snmpFddiMACSMTAddress);
            free(new_instance->prev);
      }
      free(new_instance->prev->snmpFddiMACUpstreamNbr);
      free(new_instance->prev->snmpFddiMACOldUpstreamNbr);
      free(new_instance->prev->snmpFddiMACSMTAddress);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->snmpFddiMACUpstreamNbr);
      free(header->next->snmpFddiMACOldUpstreamNbr);
      free(header->next->snmpFddiMACSMTAddress);
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

    indx++;
    if ((status =
        find_info_by_fddi_index(fddi_index_ptr->smt_index,FDDIMIB_MAC))
         != MAN_C_SUCCESS)
       return(status);

    new_instance = 
             (snmpFddiMACEntry_DEF *)malloc(sizeof(snmpFddiMACEntry_DEF));
    if (new_instance == NULL) {
       free(new_instance);
       return(MAN_C_INSUFFICIENT_RESOURCES);
     }
 
     /* fill in the instance info */
 
     new_instance->snmpFddiMACSMTIndex =
	ctr_info.ctr_ctrs.mac_fddi.mac_smt_index;
	find_mac_index(fddi_index_ptr->mac_index,&mac_index);
     new_instance->snmpFddiMACIndex = mac_index;
     new_instance->snmpFddiMACFrameStatusCapabilities = 
	ctr_info.ctr_ctrs.mac_fddi.mac_fsc;
     new_instance->snmpFddiMACRMTState =
	ctr_info.ctr_ctrs.mac_fddi.mac_rmtstate;
     new_instance->snmpFddiMACFrameCts =
	ctr_info.ctr_ctrs.mac_fddi.mac_counter;
     new_instance->snmpFddiMACErrorCts =
	ctr_info.ctr_ctrs.mac_fddi.mac_error;
     new_instance->snmpFddiMACLostCts =
	ctr_info.ctr_ctrs.mac_fddi.mac_lost;
     new_instance->snmpFddiMACTMaxGreatestLowerBound =
	ctr_info.ctr_ctrs.mac_fddi.mac_gltmax;
     new_instance->snmpFddiMACTVXGreatestLowerBound =
	ctr_info.ctr_ctrs.mac_fddi.mac_gltvx;
     new_instance->snmpFddiMACPathsAvailable =
	ctr_info.ctr_ctrs.mac_fddi.mac_paths;
     new_instance->snmpFddiMACCurrentPath =
	ctr_info.ctr_ctrs.mac_fddi.mac_current;
     new_instance->snmpFddiMACUpstreamNbr = (char *)malloc(6);
	bcopy((char *)ctr_info.ctr_ctrs.mac_fddi.mac_upstream,
		new_instance->snmpFddiMACUpstreamNbr,6);
     new_instance->snmpFddiMACUpstreamNbr_len = 6;
     new_instance->snmpFddiMACOldUpstreamNbr = (char *)malloc(6);
	bcopy((char *)ctr_info.ctr_ctrs.mac_fddi.mac_oldupstream,
	new_instance->snmpFddiMACOldUpstreamNbr,6);
     new_instance->snmpFddiMACOldUpstreamNbr_len  = 6;
     new_instance->snmpFddiMACDupAddrTest =
	ctr_info.ctr_ctrs.mac_fddi.mac_dupaddrtest;
     new_instance->snmpFddiMACPathsRequested =
	ctr_info.ctr_ctrs.mac_fddi.mac_pathsreq;
     new_instance->snmpFddiMACDownstreamPORTType =
	ctr_info.ctr_ctrs.mac_fddi.mac_downstreamtype;
     new_instance->snmpFddiMACSMTAddress = (char *)malloc(6);
	bcopy((char *)ctr_info.ctr_ctrs.mac_fddi.mac_smtaddress,
		new_instance->snmpFddiMACSMTAddress,6);
     new_instance->snmpFddiMACSMTAddress_len = 6;
     new_instance->snmpFddiMACTReq =
	ctr_info.ctr_ctrs.mac_fddi.mac_treq;
     new_instance->snmpFddiMACTNeg =
	ctr_info.ctr_ctrs.mac_fddi.mac_tneg;
     new_instance->snmpFddiMACTMax =
	ctr_info.ctr_ctrs.mac_fddi.mac_tmax;
     new_instance->snmpFddiMACTvxValue =
	ctr_info.ctr_ctrs.mac_fddi.mac_tvx;
     new_instance->snmpFddiMACTMin =
	ctr_info.ctr_ctrs.mac_fddi.mac_tmin;
     new_instance->snmpFddiMACCurrentFrameStatus =
	ctr_info.ctr_ctrs.mac_fddi.mac_framestatus;
     new_instance->snmpFddiMACFrameErrorThreshold = 1;
     new_instance->snmpFddiMACFrameErrorRatio = 1;
     new_instance->snmpFddiMACDaFlag =
	ctr_info.ctr_ctrs.mac_fddi.mac_dupaddr;
     new_instance->snmpFddiMACUnaDaFlag =
	ctr_info.ctr_ctrs.mac_fddi.mac_updupaddr;/* Check this with Uttam */
     new_instance->snmpFddiMACFrameCondition =
	ctr_info.ctr_ctrs.mac_fddi.mac_condition;
     new_instance->snmpFddiMACChipSet = &mac_chipset_id_oid;
     new_instance->snmpFddiMACAction =
	ctr_info.ctr_ctrs.mac_fddi.mac_action;
     new_instance->instance_name = (char *)&indx ;
     new_instance->instance_name_length = sizeof(int);
     memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
     new_instance->object_instance = (avl *)NULL;

     /* insert new struct at the end of the list */

     snmpFddiMACEntry_add_new_instance(header->prev, new_instance);
     fddi_index_ptr = fddi_index_ptr->next;
    }

    return(MAN_C_SUCCESS);
}
