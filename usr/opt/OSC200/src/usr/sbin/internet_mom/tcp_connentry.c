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
static char *rcsid = "@(#)$RCSfile: tcp_connentry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:45:46 $";
#endif
/*
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: tcp_connentry.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:45:46 $";
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


man_status refresh_tcpConnEntry_list(
			    tcpConnEntry_DEF *header
			    )

/*    Function Description:
 *    	This function seeks the udp_blk structure out of the kernel and
 *	reads in the required data into the provided structure.
 *    
 *    Arguments:
 *	buf       	Input/Output : A pointer to the udp structure
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
    tcp_blk tcps;
    tcpConnEntry_DEF *new_instance;
    struct strioctl str;

    bzero(&tcps, sizeof(tcp_blk));
    str.ic_cmd = KINFO_GET_TCP;
    str.ic_timout = 15;
    str.ic_len = sizeof(tcp_blk);
    str.ic_dp = (char *)&tcps;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
            free(new_instance->prev->tcpConnLocalAddress);
	    free(new_instance->prev->tcpConnRemAddress);
	    free(new_instance->prev->instance_name);
            free(new_instance->prev);
      }
      free(new_instance->prev->tcpConnLocalAddress);
      free(new_instance->prev->tcpConnRemAddress);
      free(new_instance->prev->instance_name);
      free(new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->tcpConnLocalAddress);
      free(header->next->tcpConnRemAddress);
      free(header->next->instance_name);
      free(header->next);
    }

    /*  FREE INSTANCE NAME TOO ??*/
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
        for(j=0; j <tcps.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (tcpConnEntry_DEF *)malloc(sizeof(tcpConnEntry_DEF));
           if (new_instance == NULL) {
              free(new_instance);
              return(MAN_C_INSUFFICIENT_RESOURCES);
           }

           /* fill in the instance info */
	
	  new_instance->tcpConnLocalAddress_len = sizeof(int);
	  new_instance->tcpConnLocalAddress = (char *)malloc(sizeof(int));
          bcopy((char *)&tcps.info[j].local_addr,
		new_instance->tcpConnLocalAddress,
		sizeof(int));
          new_instance->tcpConnLocalPort = ntohs(tcps.info[j].local_port);
	  new_instance->tcpConnRemAddress_len = sizeof(int); 
	  new_instance->tcpConnRemAddress = (char *)malloc(sizeof(int));
          bcopy((char *)&tcps.info[j].rem_addr,
                new_instance->tcpConnRemAddress,
                sizeof(int));
	  new_instance->tcpConnRemPort = ntohs(tcps.info[j].rem_port);
	  get_state_info(tcps.info[j].state, &new_instance->tcpConnState);

	 /* set up the instance name */
	 /* Instance name is a string of local ip address */
	 /* local port, remote ip address and remote port */

	  new_instance->instance_name_length = 16;
          if((new_instance->instance_name =
		 (char *)malloc(sizeof(int) * 4)) == NULL){
                perror();
          }
	  ins_name = new_instance->instance_name;
	  bcopy(&tcps.info[j].local_addr.s_addr,ins_name,4);
          tmp_int = ntohs(tcps.info[j].local_port);
	  bcopy(&tmp_int,&ins_name[4],4);
          bcopy(&tcps.info[j].rem_addr.s_addr,&ins_name[8],4);
	  tmp_int = ntohs(tcps.info[j].rem_port);
	  bcopy(&tmp_int,&ins_name[12],4);

          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));

          new_instance->object_instance = (avl *)NULL;

           /* insert new struct at the end of the list */

           tcpConnEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(tcps.more);
  
    return(MAN_C_SUCCESS);
}



get_state_info(state_info,
               state)
/*
 *    Function Description:
 *      This routine is just a switch table. Given a value state_info,
 *      it translates it into the appropriate MIB value and fills in the
 *      variable state.
 *
 *
 *    Arguments:
 *        state_info    Input : The state of the connection as the
 *                              kernel knows it.
 *        state         Output: The corresponding MIB value for that state.
 *
 *    Return Value:
 *
*
 *         MAN_C_SUCCESS
 *
 *    Side Effects:
 *
 */

int state_info;
int *state;

{

    switch(state_info){

    case TCPS_CLOSED:
        *state = CLOSED;
        break;

    case TCPS_LISTEN:
        *state = LISTEN;
        break;

    case TCPS_SYN_SENT:
        *state = SYN_SENT;
        break;

    case TCPS_SYN_RECEIVED:
        *state = SYN_RECEIVED;
        break;

    case TCPS_ESTABLISHED:
        *state = ESTABLISHED;
        break;

    case TCPS_CLOSE_WAIT:
        *state = CLOSE_WAIT;
        break;

    case TCPS_FIN_WAIT_1:
        *state = FIN_WAIT_1;
        break;

    case TCPS_FIN_WAIT_2:
        *state = FIN_WAIT_2;
        break;

    case TCPS_LAST_ACK:
        *state = LAST_ACK;
        break;

    case TCPS_CLOSING:
        *state = CLOSING;
        break;

    case TCPS_TIME_WAIT:
        *state = TIME_WAIT;
        break;

    }
}

