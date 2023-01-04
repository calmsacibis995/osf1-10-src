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
static char *rcsid = "@(#)$RCSfile: smt_grp.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/11/10 21:43:14 $";
#endif
/*
 *
 * Facility:
 *
 *
 * Abstract:
 *
 *	This file contain routines to extract information about the SMT
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
#include "fddi_mom_specific.h"
#include <sys/stream.h>
#include <sys/strkinfo.h>
#include <stropts.h>



man_status refresh_smt_list(
			     snmpFddiSMT_DEF *new_snmpFddiSMT_header
			    )

/*    Function Description:
 *    	This function seeks the smtstat structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	       new_snmpFddiSMT_header	Input/Output : A pointer to the "smt" structure
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
    snmpFddiSMT_DEF *smt_instance = NULL;
    unsigned int status;

 
    if (new_snmpFddiSMT_header->next == new_snmpFddiSMT_header){
       smt_instance = (snmpFddiSMT_DEF *)malloc(sizeof(snmpFddiSMT_DEF));
       if (smt_instance == NULL)
	  return(MAN_C_INSUFFICIENT_RESOURCES);
 
       /* Insert new struct at end of list */
       status = snmpFddiSMT_add_new_instance(new_snmpFddiSMT_header,
				      smt_instance );
       if (ERROR_CONDITION(status))
	   return(status);
    }else
        smt_instance = new_snmpFddiSMT_header->next;


    create_fddi_index_info();
    smt_instance->snmpFddiSMTNumber = fddi_index_info.total_smt;  
    smt_instance->instance_name_length = 0;
    smt_instance->instance_name = (char *)NULL;
    memset((void *)&smt_instance->instance_uid, '\0', sizeof(uid));
    smt_instance->object_instance = (avl *)NULL;

    return(MAN_C_SUCCESS);
} 

/* end of smt_grp.c */
