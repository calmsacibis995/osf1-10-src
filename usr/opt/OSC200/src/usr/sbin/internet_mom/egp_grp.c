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
static char *rcsid = "@(#)$RCSfile: egp_grp.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/07 20:36:44 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: egp_grp.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/09/07 20:36:44 $";
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
#include "inet_mom_specific.h"

static unsigned int egp_inmsgs_id[8] = {1,3,6,1,2,1,8,1};
static object_id egp_inmsgs_oid =  {8, egp_inmsgs_id };
static unsigned int egp_inerrors_id[8] = {1,3,6,1,2,1,8,2};
static object_id egp_inerrors_oid =  {8, egp_inerrors_id };
static unsigned int egp_outmsgs_id[8] = {1,3,6,1,2,1,8,3};
static object_id egp_outmsgs_oid =  {8, egp_outmsgs_id };
static unsigned int egp_outerrors_id[8] = {1,3,6,1,2,1,8,4};
static object_id egp_outerrors_oid =  {8, egp_outerrors_id };
static unsigned int egp_as_id[8] = {1,3,6,1,2,1,8,6};
static object_id egp_as_oid =  {8, egp_as_id };



man_status refresh_egp_list(
			    egp_DEF *new_egp_header
			   )

/*    Function Description:
 *    	This function seeks the egp_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the egp structure
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

    egp_DEF *egp_instance = NULL;
    struct in_addr dest_addr ;
    struct object_id *oid;
    int egp_value;
    man_status status;

    /* check if first time */

	if (new_egp_header->next == new_egp_header)
	{
	  egp_instance = (egp_DEF *)malloc(sizeof(egp_DEF));
	  if (egp_instance == NULL)
	     return(MAN_C_INSUFFICIENT_RESOURCES);

	  status = egp_add_new_instance(new_egp_header,
					egp_instance);
	  if (ERROR_CONDITION(status))
           return(status);
        }else
            egp_instance = new_egp_header->next;
        

    	bzero(&dest_addr, sizeof(struct in_addr));


	oid = &egp_inmsgs_oid;
	if (get_from_router(&egp_value, dest_addr, oid,0) != MAN_C_SUCCESS){
	    egp_value = 0;
        }
        egp_instance->egpInMsgs = egp_value;  
        
        oid = &egp_inerrors_oid;
        if (get_from_router(&egp_value, dest_addr, oid,0) != MAN_C_SUCCESS){
            egp_value = 0;
        }
        egp_instance->egpInErrors = egp_value;

        oid = &egp_outerrors_oid;
        if (get_from_router(&egp_value, dest_addr, oid,0) != MAN_C_SUCCESS){
            egp_value = 0;
        }
        egp_instance->egpOutErrors = egp_value;
 
        oid = &egp_outmsgs_oid ;
        if (get_from_router(&egp_value, dest_addr, oid,0) != MAN_C_SUCCESS){
            egp_value = 0;
        }
        egp_instance->egpOutMsgs = egp_value;

        oid = &egp_as_oid;
        if (get_from_router(&egp_value, dest_addr, oid,0) != MAN_C_SUCCESS){
            egp_value = -1;
        }
        egp_instance->egpAs = egp_value ;
 
        egp_instance->instance_name_length = 0;
        egp_instance->instance_name = (char *)NULL;
        memset( (void *)&egp_instance->instance_uid, '\0', sizeof(uid));
        egp_instance->object_instance = (avl *)NULL ;

    return(MAN_C_SUCCESS);
}
