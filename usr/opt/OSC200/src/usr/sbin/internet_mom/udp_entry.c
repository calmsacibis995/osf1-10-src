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
static char *rcsid = "@(#)$RCSfile: udp_entry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:46:41 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: udp_entry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:46:41 $";
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

man_status refresh_udpEntry_list(
			    udpEntry_DEF *header
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
    udp_blk udps;
    udpEntry_DEF *new_instance;
    struct strioctl str;


    bzero(&udps, sizeof(udp_blk));
    str.ic_cmd = KINFO_GET_UDP;
    str.ic_timout = 15;
    str.ic_len = sizeof(udp_blk);
    str.ic_dp = (char *)&udps;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->udpLocalAddress);
	    free(new_instance->prev->instance_name);
            free(new_instance->prev);
      }
      free(new_instance->prev->udpLocalAddress);
      free(new_instance->prev->instance_name);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->udpLocalAddress);
      free(header->next->instance_name);
      free(header->next);
    }

    header->next = header->prev = header;

    do {
	int i, j;
	char *ch;
        int tmp_int;
	char *ins_name;  

        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }

        for(j=0; j <udps.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (udpEntry_DEF *)malloc(sizeof(udpEntry_DEF));
           if (new_instance == NULL) {
	      free(new_instance);
	      return(MAN_C_INSUFFICIENT_RESOURCES);
           }
 
           /* fill in the instance info */
 
	   new_instance->udpLocalAddress_len = sizeof(int);
           new_instance->udpLocalAddress = (char *)malloc(sizeof(int));
           bcopy((char *)&udps.info[j].local_addr,
                 new_instance->udpLocalAddress,
               sizeof(int)); 
           new_instance->udpLocalPort = ntohs(udps.info[j].local_port);
	  /* set up the instance name -check if this is ok?*/
          new_instance->instance_name_length = 8;
       	  if((new_instance->instance_name = (char *)malloc(sizeof(int) * 2)) 
== NULL){
		perror();
	  }
	  ins_name = new_instance->instance_name;
          bcopy(&udps.info[j].local_addr.s_addr,ins_name,4);
          tmp_int = ntohs(udps.info[j].local_port);
          bcopy(&tmp_int,&ins_name[4],4);
          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
	  new_instance->object_instance = (avl *)NULL;

	   /* insert new struct at the end of the list */

	   udpEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(udps.more);
    return(MAN_C_SUCCESS);
}
