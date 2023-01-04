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
static char *rcsid = "@(#)$RCSfile: port_grp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/07/13 21:59:36 $";
#endif
/*
 * Facility:
 *
 *
 * Abstract:
 *
 *	This file contain routines to extract information about the PORT
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



man_status refresh_port_list(
			     snmpFddiPORT_DEF *new_snmpFddiPORT_header
			    )

/*    Function Description:
 *    	This function seeks the portstat structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	       new_snmpFddiPORT_header	Input/Output : A pointer to the "port" structure
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
    snmpFddiPORT_DEF *port_instance = NULL;
    unsigned int status;

 
    if (new_snmpFddiPORT_header->next == new_snmpFddiPORT_header){
       port_instance = (snmpFddiPORT_DEF *)malloc(sizeof(snmpFddiPORT_DEF));
       if (port_instance == NULL)
	  return(MAN_C_INSUFFICIENT_RESOURCES);
 
       /* Insert new struct at end of list */
       status = snmpFddiPORT_add_new_instance(new_snmpFddiPORT_header,
				      port_instance );
       if (ERROR_CONDITION(status))
	   return(status);
    }else
        port_instance = new_snmpFddiPORT_header->next;


    port_instance->snmpFddiPORTNumber = 1;  
    port_instance->instance_name_length = 0;
    port_instance->instance_name = (char *)NULL;
    memset((void *)&port_instance->instance_uid, '\0', sizeof(uid));
    port_instance->object_instance = (avl *)NULL;

    return(MAN_C_SUCCESS);
} 

/* end of port_grp.c */
