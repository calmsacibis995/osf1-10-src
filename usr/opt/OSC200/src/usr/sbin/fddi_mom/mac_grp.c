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
static char *rcsid = "@(#)$RCSfile: mac_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/13 21:59:19 $";
#endif
/*
 */
/* Facility:
 *
 *
 * Abstract:
 *
 *	This file contain routines to extract information about the MAC
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



man_status refresh_mac_list(
			     snmpFddiMAC_DEF *new_snmpFddiMAC_header
			    )

/*    Function Description:
 *    	This function seeks the macstat structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	       new_snmpFddiMAC_header	Input/Output : A pointer to the "mac" structure
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
    snmpFddiMAC_DEF *mac_instance = NULL;
    unsigned int status;

 
    if (new_snmpFddiMAC_header->next == new_snmpFddiMAC_header){
       mac_instance = (snmpFddiMAC_DEF *)malloc(sizeof(snmpFddiMAC_DEF));
       if (mac_instance == NULL)
	  return(MAN_C_INSUFFICIENT_RESOURCES);
 
       /* Insert new struct at end of list */
       status = snmpFddiMAC_add_new_instance(new_snmpFddiMAC_header,
				      mac_instance );
       if (ERROR_CONDITION(status))
	   return(status);
    }else
        mac_instance = new_snmpFddiMAC_header->next;


    mac_instance->snmpFddiMACNumber = 1;  
    mac_instance->instance_name_length = 0;
    mac_instance->instance_name = (char *)NULL;
    memset((void *)&mac_instance->instance_uid, '\0', sizeof(uid));
    mac_instance->object_instance = (avl *)NULL;

    return(MAN_C_SUCCESS);
} 

/* end of mac_grp.c */
