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
static char *rcsid = "@(#)$RCSfile: ip_netentry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:43:21 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ip_netentry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:43:21 $";
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



man_status refresh_ipNetToMediaEntry_list(
			    ipNetToMediaEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the udp_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the udp structure
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
    arp_blk ats;
    ipNetToMediaEntry_DEF *new_instance;
    struct strioctl str;


    bzero(&ats, sizeof(arp_blk));
    str.ic_cmd = KINFO_GET_AT;
    str.ic_timout = 15;
    str.ic_len = sizeof(arp_blk);
    str.ic_dp = (char *)&ats;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->ipNetToMediaNetAddress);
	    free(new_instance->prev->ipNetToMediaPhysAddress);
	    free(new_instance->prev->instance_name);
            free(new_instance->prev);
      }
      free(new_instance->prev->ipNetToMediaNetAddress);
      free(new_instance->prev->ipNetToMediaPhysAddress);
      free(new_instance->prev->instance_name);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->ipNetToMediaNetAddress);
      free(header->next->ipNetToMediaPhysAddress);
      free(header->next->instance_name);
      free(header->next);
    }

    header->next = header->prev = header;

    do {
	int i, j;
	char *ch;
	char *ins_name;  

        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }

        for(j=0; j <ats.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (ipNetToMediaEntry_DEF *)malloc(sizeof(ipNetToMediaEntry_DEF));
           if (new_instance == NULL) {
	      free(new_instance);
	      return(MAN_C_INSUFFICIENT_RESOURCES);
           }
 
           /* fill in the instance info */
 
	   new_instance->ipNetToMediaIfIndex = ats.info[j].if_index;

	   new_instance->ipNetToMediaNetAddress = (char *)malloc(sizeof(int));
	   bcopy((char *)&ats.info[j].inet_addr,
                 new_instance->ipNetToMediaNetAddress,
               sizeof(int)); 
	   new_instance->ipNetToMediaNetAddress_len = sizeof(int);

	   new_instance->ipNetToMediaPhysAddress = (char *)malloc(6);
	   bcopy((char *)&ats.info[j].hw_addr[0],
		  new_instance->ipNetToMediaPhysAddress,
		  6); 
	   new_instance->ipNetToMediaPhysAddress_len = 6;

	   if (ats.info[j].arp_flags & ATF_PERM){
	       new_instance->ipNetToMediaType = MOM_C_IP_NETENTRY_STATIC;
	   } else {
	       new_instance->ipNetToMediaType = MOM_C_IP_NETENTRY_DYNAMIC;
	   }
	
	  /* set up the instance name */
          new_instance->instance_name_length = sizeof(int) + 4;
	  if((new_instance->instance_name =
                 (char *)malloc(sizeof(int) + 4)) == NULL) {
                  perror();
          }
	  ins_name = new_instance->instance_name;
	  bcopy(&ats.info[j].if_index,ins_name,4);
	  bcopy(&ats.info[j].inet_addr.s_addr,&ins_name[4],4);
          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
	  new_instance->object_instance = (avl *)NULL;

	   /* insert new struct at the end of the list */

	   ipNetToMediaEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(ats.more);
    return(MAN_C_SUCCESS);
}
