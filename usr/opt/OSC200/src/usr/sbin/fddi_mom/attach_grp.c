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
static char *rcsid = "@(#)$RCSfile: attach_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/13 21:58:41 $";
#endif
/*
 */
/* Facility:
 *
 *
 * Abstract:
 *
 *	This file contain routines to extract information about the ATTACHMENT
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
#include <sys/stream.h>
#include <sys/strkinfo.h>
#include <stropts.h>



man_status refresh_attachment_list(
			     snmpFddiATTACHMENT_DEF *new_snmpFddiATTACHMENT_header
			    )

/*    Function Description:
 *    	This function seeks the attachmentstat structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	       new_snmpFddiATTACHMENT_header	Input/Output : A pointer to the "attachment" structure
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
    snmpFddiATTACHMENT_DEF *attachment_instance = NULL;
    unsigned int status;

 
    if (new_snmpFddiATTACHMENT_header->next == new_snmpFddiATTACHMENT_header){
       attachment_instance = (snmpFddiATTACHMENT_DEF *)malloc(sizeof(snmpFddiATTACHMENT_DEF));
       if (attachment_instance == NULL)
	  return(MAN_C_INSUFFICIENT_RESOURCES);
 
       /* Insert new struct at end of list */
       status = snmpFddiATTACHMENT_add_new_instance(new_snmpFddiATTACHMENT_header,
				      attachment_instance );
       if (ERROR_CONDITION(status))
	   return(status);
    }else
        attachment_instance = new_snmpFddiATTACHMENT_header->next;


    attachment_instance->snmpFddiATTACHMENTNumber = 1;  
    attachment_instance->instance_name_length = 0;
    attachment_instance->instance_name = (char *)NULL;
    memset((void *)&attachment_instance->instance_uid, '\0', sizeof(uid));
    attachment_instance->object_instance = (avl *)NULL;

    return(MAN_C_SUCCESS);
} 

/* end of attachment_grp.c */
