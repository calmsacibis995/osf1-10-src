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
static char *rcsid = "@(#)$RCSfile: ip_addrentry.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1994/02/21 21:04:47 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ip_addrentry.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1994/02/21 21:04:47 $";
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


static
unsigned int
   if_basic_id[2] = {0,0} ;

static
object_id
   if_basic_id_oid = {2, if_basic_id } ;

man_status refresh_ipAddrEntry_list(
			    ipAddrEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the if_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	header       	Input/Output : A pointer to the if structure
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
    if_blk ifs;
    ipAddrEntry_DEF *new_instance;
    struct strioctl str;
    char ifname[12];
    int broadcast, netmask;

    bzero(&ifs, sizeof(if_blk));
    str.ic_cmd = KINFO_GET_INTERFACES;
    str.ic_timout = 15;
    str.ic_len = sizeof(if_blk);
    str.ic_dp = (char *)&ifs;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->ipAdEntAddr);
	    free(new_instance->prev->ipAdEntNetMask);
	    free(new_instance->prev->instance_name);
            free(new_instance->prev);
      }
      free(new_instance->prev->ipAdEntAddr);
      free(new_instance->prev->ipAdEntNetMask);
      free(new_instance->prev->instance_name);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->ipAdEntAddr);
      free(header->next->ipAdEntNetMask);
      free(header->next->instance_name);
      free(header->next);
    }

    header->next = header->prev = header;

    do {
	int i, j;
/*
	char *ch;
	int ins_name[5];  
 */

        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }

        for(j=0; j <ifs.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (ipAddrEntry_DEF *)malloc(sizeof(ipAddrEntry_DEF));
           if (new_instance == NULL) {
	      free(new_instance);
	      return(MAN_C_INSUFFICIENT_RESOURCES);
           }

	  /* get the required information */
	
	    sprintf(ifname,"%s%hd",ifs.info[j].if_name,
		    ifs.info[j].if_unit);
	    get_req_info(ifname,&broadcast,&netmask);
 
          /* fill in the instance info */
 
	   new_instance->ipAdEntIfIndex = (j + 1) ;
	   new_instance->ipAdEntAddr = (char *)malloc(sizeof(int));
	   bcopy((char *)&ifs.info[j].ip_addr,
		 new_instance->ipAdEntAddr,
		 sizeof(int));
	   new_instance->ipAdEntAddr_len = sizeof(int);
	   new_instance->ipAdEntNetMask = (char *)malloc(sizeof(int));
	   bcopy((char *)&netmask,new_instance->ipAdEntNetMask,sizeof(int));
	   new_instance->ipAdEntNetMask_len =  sizeof(int); 
	   new_instance->ipAdEntBcastAddr =  broadcast ;
	   new_instance->ipAdEntReasmMaxSize = 32768;
	
	  /* set up the instance name */
          new_instance->instance_name_length = sizeof(int) ;
          if((new_instance->instance_name =
		(char *)malloc(sizeof(int))) == NULL) {
		 perror();
	  }
/*
          ch = (char *)&ifs.info[j].ip_addr.s_addr;
          for (i=0; i < 4; i++){
              ins_name[i] = *ch & 0xff;
              ch++;
          }
 */
          bcopy(new_instance->ipAdEntAddr, new_instance->instance_name, 4);
          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
	  new_instance->object_instance = (avl *)NULL;

	   /* insert new struct at the end of the list */

	   ipAddrEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(ifs.more);
    return(MAN_C_SUCCESS);
}

get_req_info(ifname,
	     broadcast,
        	netmask)

char *ifname;
int *broadcast;
int *netmask;
{
	
    int lpbck = FALSE ;
    struct ifreq ifr_netmask;
    struct ifreq ifr_brdcast;
    int s;

    /*
     * Check if this the loop back interface.
     * If it is flag it to be a loop back interface.
     */
    if (strncmp(ifname, "lo", 2) == 0) {
        lpbck = TRUE;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("TCPIP_MOM: ip_addrentry.c");
        return(MAN_C_PROCESSING_FAILURE);
    }

    strncpy(ifr_netmask.ifr_name, ifname, (strlen(ifname) + 1));

    if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifr_netmask) < 0) {
        close(s);
	return(MAN_C_PROCESSING_FAILURE);
    }
       
    *netmask = (((struct sockaddr_in *)&ifr_netmask.ifr_addr)->sin_addr.s_addr);
    if (lpbck == FALSE) {
        strncpy(ifr_brdcast.ifr_name, ifname, (strlen(ifname) + 1));
        if (ioctl(s, SIOCGIFBRDADDR, (caddr_t)&ifr_brdcast) < 0) {
            perror("TCPIP_MOM: ip_addrentry.c: SIOCGIFBRDADDR");
            close(s);
            return(MAN_C_PROCESSING_FAILURE);
        }

        *broadcast = 0x00000001 & ntohl((int)ifr_brdcast.ifr_broadaddr.sa_data);
    } else {
        /*
         * NOTE:
         * A loop back interface is not a Multiple Access
         * medium and thus does not have a broadcast address.
         * The broadcast value returned is zero.
         */

        *broadcast = 0;
    }
    close(s);

    return(MAN_C_SUCCESS);

}
