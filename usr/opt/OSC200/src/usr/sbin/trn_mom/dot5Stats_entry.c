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
static char *rcsid = "@(#)$RCSfile: dot5Stats_entry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/13 21:13:33 $";
#endif

#include "moss.h"
#include "common.h"
#include "extern_common.h"
#include "dot5_mom_specific.h"


man_status refresh_dot5StatsEntry_list(
			    dot5StatsEntry_DEF *header
			    )

/*    Function Description:
 *    	This function reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the dot5Stats structure
 *				       that needs to be filled in.
 *    Return Value:
 *   
 *         MAN_C_SUCCESS
 *	   MAN_C_PROCESSING_FAILURE
 *	   MAN_C_INSUFFICIENT_RESOURCES
 *    
 *    Side Effects:
 *	None.
 */
{
   
    dot5StatsEntry_DEF *new_instance;
    struct dot5_info *dot5_index_ptr;
    man_status status;
   

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

    status = create_dot5_index_info();   
    if (status != MAN_C_SUCCESS)
	return(MAN_C_PROCESSING_FAILURE);
 
    /*
     * Check if any Token Ring Interfaces have been configured
     */

    if (dot5_index_info.index == NULL)
	return(MAN_C_PROCESSING_FAILURE);

    dot5_index_ptr = dot5_index_info.index ;

    while(dot5_index_ptr != NULL)
    {

    if ((status = 
	find_info_by_dot5_index(dot5_index_ptr->dot5_index,TRN_MIB_STAT_ENTRY)) 
	 != MAN_C_SUCCESS)
       return(status); 

    new_instance = 
             (dot5StatsEntry_DEF *)malloc(sizeof(dot5StatsEntry_DEF));
    if (new_instance == NULL) {
       free(new_instance);
       return(MAN_C_INSUFFICIENT_RESOURCES);
    }
 
     /* fill in the instance info */
 
     new_instance->dot5StatsIfIndex = 
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsIfIndex;
     new_instance->dot5StatsLineErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsLineErrors;
     new_instance->dot5StatsBurstErrors = 
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsBurstErrors;
     new_instance->dot5StatsACErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsACErrors;
     new_instance->dot5StatsAbortTransErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsAbortTransErrors;
     new_instance->dot5StatsInternalErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsInternalErrors;
     new_instance->dot5StatsLostFrameErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsLostFrameErrors;
     new_instance->dot5StatsReceiveCongestions =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsReceiveCongestions;
     new_instance->dot5StatsFrameCopiedErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsFrameCopiedErrors;
     new_instance->dot5StatsTokenErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsTokenErrors;
     new_instance->dot5StatsSoftErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsSoftErrors;
     new_instance->dot5StatsHardErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsHardErrors;
     new_instance->dot5StatsSignalLoss =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsSignalLoss;
     new_instance->dot5StatsTransmitBeacons =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsTransmitBeacons;
     new_instance->dot5StatsRecoverys =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsRecoverys;
     new_instance->dot5StatsLobeWires =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsLobeWires;
     new_instance->dot5StatsRemoves =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsRemoves;
     new_instance->dot5StatsSingles =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsSingles;
     new_instance->dot5StatsFreqErrors =
	ctr_info.ctr_ctrs.dot5StatsEntry.dot5StatsFreqErrors;
     new_instance->instance_name_length = sizeof(int);
     new_instance->instance_name = (char *)&dot5_index_ptr->dot5_index;
     memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
     new_instance->object_instance = (avl *)NULL;

     /* insert new struct at the end of the list */

     dot5StatsEntry_add_new_instance(header->prev, new_instance);
     dot5_index_ptr = dot5_index_ptr->next;
    }
    return(MAN_C_SUCCESS);
}
