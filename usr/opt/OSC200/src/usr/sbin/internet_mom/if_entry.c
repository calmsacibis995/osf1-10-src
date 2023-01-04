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
static char *rcsid = "@(#)$RCSfile: if_entry.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/07 20:48:33 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: if_entry.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/07 20:48:33 $";
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

man_status refresh_ifEntry_list(
			    ifEntry_DEF *header
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
    ifEntry_DEF *new_instance;
    struct strioctl str;
    struct ifdevea ifd;			/* to get the ethernet address */
    extern unsigned int sysStartTime;   /* in system_load_structures.c */


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
	    free(new_instance->prev->ifDescr);
	    if (*(new_instance->prev->ifPhysAddress) != NULL)
	      free(new_instance->prev->ifPhysAddress);
            free(new_instance->prev);
      }
      free(new_instance->prev->ifDescr);
      if (*(new_instance->prev->ifPhysAddress) != NULL)
        free(new_instance->prev->ifPhysAddress);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->ifDescr);
      if (*(new_instance->prev->ifPhysAddress) != NULL)
        free(header->next->ifPhysAddress);
      free(header->next);
    }

    header->next = header->prev = header;

    do {
	int i, j;
	char *ch;
	int *ins_name;  

        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }

        for(j=0; j <ifs.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (ifEntry_DEF *)malloc(sizeof(ifEntry_DEF));
           if (new_instance == NULL) {
	      free(new_instance);
	      return(MAN_C_INSUFFICIENT_RESOURCES);
           }
 
          /* fill in the instance info */
 
	   new_instance->ifIndex = j + 1 ;
           new_instance->ifOperStatus = ifs.info[j].if_operstatus;
	   new_instance->ifSpeed = ifs.info[j].if_speed;
           new_instance->ifInOctets = ifs.info[j].if_inoctets;
           new_instance->ifInUcastPkts = ifs.info[j].if_inucasts;
           new_instance->ifInNUcastPkts = ifs.info[j].if_inmcasts; 
           new_instance->ifInDiscards = ifs.info[j].if_indiscards;
           new_instance->ifInErrors = ifs.info[j].if_inerrors;
           new_instance->ifInUnknownProtos = ifs.info[j].if_noproto;
           new_instance->ifOutOctets = ifs.info[j].if_outoctets;
           new_instance->ifOutUcastPkts = ifs.info[j].if_outucasts;
           new_instance->ifOutNUcastPkts = ifs.info[j].if_outmcasts;
           new_instance->ifOutDiscards = ifs.info[j].if_outdiscards;
           new_instance->ifOutErrors = ifs.info[j].if_outerrors;
	   new_instance->ifOutQLen = ifs.info[j].if_outqlen;
           new_instance->ifType = (int)ifs.info[j].if_type;
           new_instance->ifMtu = ifs.info[j].if_mtu;

           /* set ifLastChange to 0 if older than when agent was started */
           new_instance->ifLastChange = ifs.info[j].if_lastchange.tv_sec * 100;
           if (new_instance->ifLastChange > sysStartTime)
               new_instance->ifLastChange = 0;

           if (ifs.info[j].if_operstatus & IFF_UP) 
           {
               new_instance->ifOperStatus  =
               new_instance->ifAdminStatus = MOM_C_IF_UP;
           } 
           else if (ifs.info[j].if_operstatus & IFF_LOOPBACK) 
           {
               new_instance->ifOperStatus  =
               new_instance->ifAdminStatus = MOM_C_IF_TESTING;
           } 
           else if (ifs.info[j].if_operstatus & ~IFF_UP) 
           {
               new_instance->ifOperStatus  =
               new_instance->ifAdminStatus = MOM_C_IF_DOWN;
           }

	   if (ifs.info[j].if_version) {
	      new_instance->ifDescr =
                (char *)malloc(strlen(ifs.info[j].if_version) + 1) ;
	      strcpy(new_instance->ifDescr,ifs.info[j].if_version);
	      new_instance->ifDescr_len = (strlen(ifs.info[j].if_version) + 1);
	   } 
	    else {
	      new_instance->ifDescr =
        (char *)malloc(255);
	      sprintf(new_instance->ifDescr,"%s%hd",ifs.info[j].if_name,ifs.info[j].if_unit);
	      new_instance->ifDescr_len = strlen(new_instance->ifDescr);
	   }

           new_instance->ifSpecific = &if_basic_id_oid ;

	   /* Need to check if it is a Loop back interface */

	   if (strncmp("lo", ifs.info[j].if_name, 2) == 0) {
	      /* what is the right thing to do refer if_entry.c */
	      new_instance->ifPhysAddress = "";
	      new_instance->ifPhysAddress_len = 0;

	   } else { /* It is NOT a Loop back interface */
 
	   int sock;
            sprintf(ifd.ifr_name, "%s%hd", ifs.info[j].if_name,ifs.info[j].if_unit);
            sock = socket(AF_INET, SOCK_DGRAM, 0);
            if (ioctl(sock, SIOCRPHYSADDR, &ifd) < 0) {
	      new_instance->ifPhysAddress = "";
              new_instance->ifPhysAddress_len = 0;
            } else {
               new_instance->ifPhysAddress = (char *)malloc(6);
	       bcopy(ifd.default_pa,new_instance->ifPhysAddress,6);
	       new_instance->ifPhysAddress_len = 6 ; 
	     }
           close(sock);
	   }
	
	  /* set up the instance name */
          new_instance->instance_name_length = sizeof(int) ;
          new_instance->instance_name =(char *)&new_instance->ifIndex ;

          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
	  new_instance->object_instance = (avl *)NULL;

	   /* insert new struct at the end of the list */

	   ifEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(ifs.more);
    return(MAN_C_SUCCESS);
}
