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
static char *rcsid = "@(#)$RCSfile: smt_entry.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/18 16:12:12 $";
#endif
/*
 *
 *
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

int indx = 0;

man_status refresh_smtEntry_list(
			    snmpFddiSMTEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the smt_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the smt structure
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
   
    snmpFddiSMTEntry_DEF *new_instance;
    struct index_info *fddi_index_ptr;
    int status;
   

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free( new_instance->prev->snmpFddiSMTStationId);
            free(new_instance->prev);
      }
      free( new_instance->prev->snmpFddiSMTStationId);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->snmpFddiSMTStationId);
      free(header->next);
    }

    header->next = header->prev = header;

    status = create_fddi_index_info();   
    if (status != MAN_C_SUCCESS){
       return(MAN_C_PROCESSING_FAILURE);
    }
 
    /*
     * Check if any FDDI Interfaces have been configured
     */

    if (fddi_index_info.index == NULL) {
        return(MAN_C_PROCESSING_FAILURE);
    }

    fddi_index_ptr = fddi_index_info.index ;
    indx = 0;

    while(fddi_index_ptr != NULL){

    indx++;
    if ((status = 
	find_info_by_fddi_index(fddi_index_ptr->smt_index,FDDIMIB_SMT)) 
	 != MAN_C_SUCCESS)
       return(status); 

    new_instance = 
             (snmpFddiSMTEntry_DEF *)malloc(sizeof(snmpFddiSMTEntry_DEF));
    if (new_instance == NULL) {
       free(new_instance);
       return(MAN_C_INSUFFICIENT_RESOURCES);
     }
 
     /* fill in the instance info */
 
     new_instance->snmpFddiSMTIndex = 
	ctr_info.ctr_ctrs.smt_fddi.smt_index;
     new_instance->snmpFddiSMTStatusReporting =
	ctr_info.ctr_ctrs.smt_fddi.smt_statusreport;
     new_instance->snmpFddiSMTECMState = 
	ctr_info.ctr_ctrs.smt_fddi.smt_ecmstate;
     new_instance->snmpFddiSMTCFState =
	ctr_info.ctr_ctrs.smt_fddi.smt_cfstate;
     new_instance->snmpFddiSMTHoldState =
	ctr_info.ctr_ctrs.smt_fddi.smt_holdstate;
     new_instance->snmpFddiSMTStationId = (char *)malloc(8);
	bcopy((char *)ctr_info.ctr_ctrs.smt_fddi.smt_stationid,
		new_instance->snmpFddiSMTStationId,8);
     new_instance->snmpFddiSMTStationId_len = 8;
     new_instance->snmpFddiSMTOpVersionId =
	ctr_info.ctr_ctrs.smt_fddi.smt_opversionid;
     new_instance->snmpFddiSMTHiVersionId =
	ctr_info.ctr_ctrs.smt_fddi.smt_hiversionid;
     new_instance->snmpFddiSMTLoVersionId =
	ctr_info.ctr_ctrs.smt_fddi.smt_loversionid;
     new_instance->snmpFddiSMTMACCt =
	ctr_info.ctr_ctrs.smt_fddi.smt_macct;
     new_instance->snmpFddiSMTNonMasterCt =
	ctr_info.ctr_ctrs.smt_fddi.smt_nonmasterct;
     new_instance->snmpFddiSMTMasterCt =
	ctr_info.ctr_ctrs.smt_fddi.smt_masterct;  
     new_instance->snmpFddiSMTPathsAvailable =
	ctr_info.ctr_ctrs.smt_fddi.smt_pathsavail;
     new_instance->snmpFddiSMTConfigCapabilities =
        ctr_info.ctr_ctrs.smt_fddi.smt_configcap;
     new_instance->snmpFddiSMTConfigPolicy =
	ctr_info.ctr_ctrs.smt_fddi.smt_configpolicy;
     new_instance->snmpFddiSMTConnectionPolicy =
	ctr_info.ctr_ctrs.smt_fddi.smt_connectpolicy;
     new_instance->snmpFddiSMTTNotify =
	ctr_info.ctr_ctrs.smt_fddi.smt_timenotify;
     new_instance->snmpFddiSMTRemoteDisconnectFlag =
	ctr_info.ctr_ctrs.smt_fddi.smt_remotedisconn;
     new_instance->snmpFddiSMTStationAction =
	ctr_info.ctr_ctrs.smt_fddi.smt_action;
     new_instance->instance_name = (char *)&indx;
     new_instance->instance_name_length = sizeof(int);
     memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
     new_instance->object_instance = (avl *)NULL;

     /* insert new struct at the end of the list */

     snmpFddiSMTEntry_add_new_instance(header->prev, new_instance);
     fddi_index_ptr = fddi_index_ptr->next;
    }
    return(MAN_C_SUCCESS);
}
