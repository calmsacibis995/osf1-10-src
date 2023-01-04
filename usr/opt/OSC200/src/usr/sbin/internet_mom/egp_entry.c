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
static char *rcsid = "@(#)$RCSfile: egp_entry.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/11/10 21:33:36 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: egp_entry.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/11/10 21:33:36 $";
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

static unsigned int egp_neighaddr_id[10] = {1,3,6,1,2,1,8,5,1,2};
static object_id egp_neighaddr_oid =  {10, egp_neighaddr_id };
static unsigned int egp_neighstate_id[10] = {1,3,6,1,2,1,8,5,1,1};
static object_id egp_neighstate_oid =  {10, egp_neighstate_id };
static unsigned int egp_neighas_id[10] = {1,3,6,1,2,1,8,5,1,3};
static object_id egp_neighas_oid =  {10, egp_neighas_id };
static unsigned int egp_neighihello_id[10] = {1,3,6,1,2,1,8,5,1,12};
static object_id egp_neighihello_oid =  {10, egp_neighihello_id };
static unsigned int egp_neighipoll_id[10] = {1,3,6,1,2,1,8,5,1,13};
static object_id egp_neighipoll_oid =  {10, egp_neighipoll_id };
static unsigned int egp_neighmode_id[10] = {1,3,6,1,2,1,8,5,1,14};
static object_id egp_neighmode_oid =  {10, egp_neighmode_id };
static unsigned int egp_neighetrigger_id[10] = {1,3,6,1,2,1,8,5,1,15};
static object_id egp_neighetrigger_oid =  {10, egp_neighetrigger_id };


man_status refresh_egpNeighEntry_list(
			    egpNeighEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the egpNeighEntry_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the egpNeighEntry structure
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
    egpNeighEntry_DEF *new_instance;
    struct in_addr dest_addr;
    struct object_id *oid;
    int egp_value;
    int i, j;
    char *ch;


    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->egpNeighAddr);
	    free(new_instance->prev->instance_name);
            free(new_instance->prev);
      }
      free(new_instance->prev->egpNeighAddr);
      free(new_instance->prev->instance_name);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->egpNeighAddr);
      free(header->next->instance_name);
      free(header->next);
    }

    header->next = header->prev = header;

    /* Get ng_addr in the first entry. */ 
 
    new_instance = (egpNeighEntry_DEF *)malloc(sizeof(egpNeighEntry_DEF));
    if (new_instance == NULL) {
              return(MAN_C_INSUFFICIENT_RESOURCES);
    }
    bzero(&dest_addr, sizeof(struct in_addr));
    oid = &egp_neighaddr_oid;  
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
              free(new_instance);
              return(MAN_C_NO_SUCH_OBJECT_INSTANCE);
       }
    new_instance->egpNeighAddr_len = sizeof(int);
    new_instance->egpNeighAddr = (char *)malloc(sizeof(int));
    bcopy((char *)&egp_value,new_instance->egpNeighAddr,sizeof(int));
    bcopy((char *)&egp_value,&dest_addr,sizeof(struct in_addr));

    /* use the address to get the rest of the info */
    oid = &egp_neighstate_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
		egp_value = 3; /* "down" */
    }
    if ((egp_value < 1) || (egp_value > 5))
        egp_value = 3; /* "down" */
    new_instance->egpNeighState = egp_value;

    oid = &egp_neighas_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 0; /* "not yet known" */
    }
    new_instance->egpNeighAs = egp_value;

    oid = &egp_neighihello_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = -1;
    }
    new_instance->egpNeighIntervalHello = egp_value;

    oid = &egp_neighipoll_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = -1;
    }
    new_instance->egpNeighIntervalPoll = egp_value;

    oid = &egp_neighmode_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 2; /* "passive" */
    }
    if ((egp_value != 1) && (egp_value != 2))
        egp_value = 2; /* "passive" */
    new_instance->egpNeighMode = egp_value;

    oid = &egp_neighetrigger_oid;
    if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 2; /* "stop" */
    }
    if ((egp_value != 1) && (egp_value != 2))
        egp_value = 2; /* "stop" */
    new_instance->egpNeighEventTrigger = egp_value;

    new_instance->egpNeighStateUps   = 0;
    new_instance->egpNeighStateDowns = 0;
    new_instance->egpNeighInMsgs     = 0;
    new_instance->egpNeighInErrs     = 0;
    new_instance->egpNeighOutMsgs    = 0;
    new_instance->egpNeighOutErrs    = 0;
    new_instance->egpNeighInErrMsgs  = 0;
    new_instance->egpNeighOutErrMsgs = 0;

    new_instance->instance_name_length = sizeof(int);
    if((new_instance->instance_name = 
	 (char *)malloc(sizeof(int))) == NULL) {
	   perror();
    }

    bcopy((char *)&dest_addr,new_instance->instance_name,sizeof(int));
    memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
    new_instance->object_instance = (avl *)NULL;
    egpNeighEntry_add_new_instance(header->prev, new_instance);


    while(dest_addr.s_addr != NULL){

	  oid = &egp_neighaddr_oid;
          if (get_from_router(&egp_value,dest_addr,oid,1) != MAN_C_SUCCESS){
              /* This is the last instance.  So return  */
	      break;
          }

	  else{

             new_instance = 
		(egpNeighEntry_DEF *)malloc(sizeof(egpNeighEntry_DEF));
             if (new_instance == NULL) {
              return(MAN_C_INSUFFICIENT_RESOURCES);
             }
             new_instance->egpNeighAddr_len = sizeof(int);
             new_instance->egpNeighAddr = (char *)malloc(sizeof(int));
             bcopy((char *)&egp_value,new_instance->egpNeighAddr,sizeof(int));
             bcopy((char *)&egp_value,&dest_addr,sizeof(int));
  
             oid = &egp_neighstate_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
		egp_value = 3; /* "down" */
             }
             if ((egp_value < 1) || (egp_value > 5))
              egp_value = 3; /* "down" */
             new_instance->egpNeighState = egp_value;

             oid = &egp_neighas_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 0; /* "not yet known" */
             }
             new_instance->egpNeighAs = egp_value;

             oid = &egp_neighihello_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = -1;
             }
             new_instance->egpNeighIntervalHello = egp_value;

             oid = &egp_neighipoll_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = -1;
             }
             new_instance->egpNeighIntervalPoll = egp_value;

             oid = &egp_neighmode_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 2; /* "passive" */
             }
             if ((egp_value != 1) && (egp_value != 2))
              egp_value = 2; /* "passive" */
             new_instance->egpNeighMode = egp_value;

             oid = &egp_neighetrigger_oid;
             if (get_from_router(&egp_value,dest_addr,oid,0) != MAN_C_SUCCESS){
                egp_value = 2; /* "stop" */
             }
             if ((egp_value != 1) && (egp_value != 2))
              egp_value = 2; /* "stop" */
             new_instance->egpNeighEventTrigger = egp_value;

             new_instance->egpNeighStateUps   = 0;
    	     new_instance->egpNeighStateDowns = 0;
    	     new_instance->egpNeighInMsgs     = 0;
    	     new_instance->egpNeighInErrs     = 0;
             new_instance->egpNeighOutMsgs    = 0;
             new_instance->egpNeighOutErrs    = 0;
    	     new_instance->egpNeighInErrMsgs  = 0;
    	     new_instance->egpNeighOutErrMsgs = 0;

	     new_instance->instance_name_length = sizeof(int);
             new_instance->instance_name = (char *)malloc(sizeof(int));
             bcopy((char *)&dest_addr,new_instance->instance_name,sizeof(int));
             memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
             new_instance->object_instance = (avl *)NULL;
             egpNeighEntry_add_new_instance(header->prev, new_instance);
	  }/*else*/
    }/*while*/
    return(MAN_C_SUCCESS);
}
