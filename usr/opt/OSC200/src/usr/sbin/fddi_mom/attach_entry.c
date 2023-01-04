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
static char *rcsid = "@(#)$RCSfile: attach_entry.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/18 16:08:13 $";
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

extern int indx ;

man_status refresh_attachmentEntry_list(
			    snmpFddiATTACHMENTEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the attachment_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the attachment structure
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
   
    snmpFddiATTACHMENTEntry_DEF *new_instance;
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

    /*
     * Check if any FDDI Interfaces have been configured
     */

    indx = 0;

    if (fddi_index_info.index == NULL) {
       return(MAN_C_PROCESSING_FAILURE);
    }

    fddi_index_ptr = fddi_index_info.index ;

    while(fddi_index_ptr != NULL){

    indx++;
    if ((status =
        find_info_by_fddi_index(fddi_index_ptr->attach_index,FDDIMIB_ATTA))
         != MAN_C_SUCCESS)
       return(status);

    new_instance = 
             (snmpFddiATTACHMENTEntry_DEF *)malloc(sizeof(snmpFddiATTACHMENTEntry_DEF));
    if (new_instance == NULL) {
       free(new_instance);
       return(MAN_C_INSUFFICIENT_RESOURCES);
     }
 
     find_info_by_fddi_index();
     /* fill in the instance info */
 
     new_instance->snmpFddiATTACHMENTSMTIndex = 
	ctr_info.ctr_ctrs.atta_fddi.atta_smt_index;
     new_instance->snmpFddiATTACHMENTIndex =
	ctr_info.ctr_ctrs.atta_fddi.atta_index;
     new_instance->snmpFddiATTACHMENTInsertedStatus =
	ctr_info.ctr_ctrs.atta_fddi.atta_InsertedStatus;
     new_instance->snmpFddiATTACHMENTClass =
	ctr_info.ctr_ctrs.atta_fddi.atta_class;
     new_instance->snmpFddiATTACHMENTOpticalBypassPresent =
	ctr_info.ctr_ctrs.atta_fddi.atta_bypass;
     new_instance->snmpFddiATTACHMENTIMaxExpiration =
	ctr_info.ctr_ctrs.atta_fddi.atta_IMaxExpiration;
     new_instance->snmpFddiATTACHMENTInsertPolicy =
	 ctr_info.ctr_ctrs.atta_fddi.atta_InsertPolicy;
     new_instance->instance_name = (char *)&indx;
     new_instance->instance_name_length = sizeof(int);
     memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
     new_instance->object_instance = (avl *)NULL;

     /* insert new struct at the end of the list */

     snmpFddiATTACHMENTEntry_add_new_instance(header->prev, new_instance);
     fddi_index_ptr = fddi_index_ptr->next;
    }

    return(MAN_C_SUCCESS);
}
