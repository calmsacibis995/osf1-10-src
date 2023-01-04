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
static char *rcsid = "@(#)$RCSfile: ip_routeentry.c,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1993/09/07 21:18:38 $";
#endif
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

/*
 ** Access info for Routing table daemon
 ** Because of the throughput bottleneck that occur when
 ** the program always sends a message to rtd and waits for a response else times-out,
 ** when we discover that the rtd is not available, we don't retry until RTD_RETRY_TIMER
 ** has elapsed.
 */
#define RTD_RETRY_TIMER 2*60	/* Number of seconds */

static is_rtd_available = 1;		/* Is RTD Available? (boolean) */
static struct timeval tval_start;	/* Time when RTD last discovered to be unavailable */


static 
unsigned int
    ip_route_id[2] = {0,0};

static
object_id
    ip_route_id_oid = {2, ip_route_id };

static unsigned int ip_route_metric1_id[10] = {1,3,6,1,2,1,4,21,1,3};
static object_id ip_route_metric1_oid =  {10, ip_route_metric1_id};
static unsigned int ip_route_type_id[10] = {1,3,6,1,2,1,4,21,1,8};
static object_id ip_route_type_oid = {10, ip_route_type_id};
static unsigned int ip_route_proto_id[10] = {1,3,6,1,2,1,4,21,1,9};
static object_id ip_route_proto_oid = {10, ip_route_proto_id};
static unsigned int ip_route_age_id[10] = {1,3,6,1,2,1,4,21,1,10};
static object_id ip_route_age_oid = {10, ip_route_age_id};


man_status get_from_router( int *entry_value, struct in_addr dest_addr, object_id *var_oid, int next);


man_status refresh_ipRouteEntry_list(
			    ipRouteEntry_DEF *header
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
    ip_routing_blk irs;
    ipRouteEntry_DEF *new_instance;
    struct strioctl str;
    int r_value;
    struct object_id *oid;

    bzero(&irs, sizeof(ip_routing_blk));
    str.ic_cmd = KINFO_GET_IP_ROUTING;
    str.ic_timout = 15;
    str.ic_len = sizeof(ip_routing_blk);
    str.ic_dp = (char *)&irs;

    /* free the old list */

    if (header->next->next != header) /* length is at least 2 structs */
    {
      for (new_instance=header->next->next; new_instance != header;
        new_instance = new_instance->next)
      {
	    free(new_instance->prev->ipRouteDest);
	    free(new_instance->prev->ipRouteNextHop);
            free(new_instance->prev->ipRouteMask);
	    free(new_instance->prev->instance_name);
	    free(new_instance->prev);
      }
      free(new_instance->prev->ipRouteDest);
      free(new_instance->prev->ipRouteNextHop);
      free(new_instance->prev->ipRouteMask);
      free(new_instance->prev->instance_name);
      free (new_instance->prev);
    }
    else if (header->next != header) /* length is 1 struct. Free it. */
    {
      free(header->next->ipRouteDest);
      free(header->next->ipRouteNextHop);
      free(header->next->ipRouteMask);
      free(header->next->instance_name);
      free(header->next);
    }

    header->next = header->prev = header;

    do {
	int i, j;
	char *ch;
/*	int ins_name[5];   */

        if (ioctl(inetfd, I_STR, &str) < 0) {
           perror(" ioctl");
           return(MAN_C_PROCESSING_FAILURE);
        }

        for(j=0; j <irs.curr_cnt; j++) {

           /* allocate space for new instance */

           new_instance = (ipRouteEntry_DEF *)malloc(sizeof(ipRouteEntry_DEF));
           if (new_instance == NULL) {
	      free(new_instance);
	      return(MAN_C_INSUFFICIENT_RESOURCES);
           }
 
           /* fill in the instance info */
 
	   new_instance->ipRouteDest = (char *)malloc(sizeof(int));
	   bcopy((char *)&irs.info[j].rt_dst,
		 new_instance->ipRouteDest,
		 sizeof(int));
	   new_instance->ipRouteDest_len = sizeof(int);
	   new_instance->ipRouteIfIndex = irs.info[j].if_index;
	   oid = &ip_route_metric1_oid ;
           if(get_from_router(&r_value,irs.info[j].rt_dst,oid,0) != MAN_C_SUCCESS)
	       new_instance->ipRouteMetric1  =  -1;
	   else
	       new_instance->ipRouteMetric1  = r_value ;
	   new_instance->ipRouteMetric2 = -1;
  	   new_instance->ipRouteMetric3 = -1;
	   new_instance->ipRouteMetric4 = -1;
	   new_instance->ipRouteNextHop = (char *)malloc(sizeof(int)); 
	   bcopy((char *)&irs.info[j].rt_next_hop,
		 new_instance->ipRouteNextHop,
		 sizeof(int));
	   new_instance->ipRouteNextHop_len = sizeof(int);
	   oid = &ip_route_age_oid;
	   if(get_from_router(&r_value,irs.info[j].rt_dst,oid,0) != MAN_C_SUCCESS)
		new_instance->ipRouteType = MOM_C_ROUTE_INVALID;
	   else
           {
                if ((r_value < 1) || (r_value > 4))
                    new_instance->ipRouteType = MOM_C_ROUTE_INVALID;
                else
                    new_instance->ipRouteType = r_value;
           }

	   oid = &ip_route_proto_oid ;
	   if(get_from_router(&r_value,irs.info[j].rt_dst,oid,0) != MAN_C_SUCCESS)
	       new_instance->ipRouteProto = 2;  /* "local" */
	   else
           {
                if ((r_value < 1) || (r_value > 14))
                    new_instance->ipRouteProto = 2;  /* "local" */
                else
                    new_instance->ipRouteProto = r_value;
           }

	   oid = &ip_route_age_oid ;
	   if(get_from_router(&r_value,irs.info[j].rt_dst,oid,0) != MAN_C_SUCCESS)
	       new_instance->ipRouteAge = 0;
	   else
	       new_instance->ipRouteAge = r_value;
	   new_instance->ipRouteMask  = (char *)malloc(sizeof(int));
	   bcopy((char *)&irs.info[j].rt_maskx,
	         new_instance->ipRouteMask,
		 sizeof(int));
	   new_instance->ipRouteMask_len = sizeof(int);
	   new_instance->ipRouteMetric5 = -1;
	   new_instance->ipRouteInfo = &ip_route_id_oid;

	  /* set up the instance name */
          new_instance->instance_name_length = sizeof(int);
	  if((new_instance->instance_name =
		(char *)malloc(sizeof(int))) == NULL) {
		  perror();
	  }
/*
          ch = (char *)&irs.info[j].rt_dst.s_addr;
          for (i=0; i < 4; i++){
              ins_name[i] = *ch & 0xff;
              ch++;
          }
	  bcopy(ins_name,new_instance->instance_name,4);
 */
	  bcopy(new_instance->ipRouteDest,new_instance->instance_name,4);
          memset((void *)&new_instance->instance_uid, '\0', sizeof(uid));
          
	  new_instance->object_instance = (avl *)NULL;

	   /* insert new struct at the end of the list */

	   ipRouteEntry_add_new_instance(header->prev, new_instance);

        }/*for loop */
     } while(irs.more);
    return(MAN_C_SUCCESS);
}

man_status get_from_router( int *entry_value, struct in_addr dest_addr, object_id *var_oid, int next)
/*
 *    Function Description:
 *      This function goes sends a packet to the router requesting a
 *      value of the variable. The protocol used is simple and in 
 *      DEC OSF/1, the gated daemon has been modified to recognize this
 *      protocol.
 *        [MSG TYPE] [VARIABLE LENGTH] [VARIABLE REQUESTED]
 *          byte 1        byte 2          byte 3 - MAX
 *      Where:
 *      MSG TYPE : Can be a 1) MOM_C_AGENT_REQ
 *                          2) MOM_C_AGENT_RESP
 *                          3) MOM_C_AGENT_ERR
 *                          4) MOM_C_AGENT_REG
 *
 *      The last one is used by the daemon to register the variables it
 *      has, when it comes up. The message is sent to port 167. In this
 *      implementation, since we are using RPC and are waiting on a select
 *      it is not possible to receive that message and thus ignore it.
 *
 *      VAR LENGTH : The length of the following bytes.
 *      VARIABLE REQUESTED : The object id of the variable requested.
 *
 *    Arguments:
 *        entry_value     Output: The value of the attribute.
 *
 *    Return Value:
 *
 *         MAN_C_SUCCESS
 *         MAN_C_FAILURE
 *
 *
 *    Side Effects:
 *      None.
 *
 *
 */
{

  char reqpkt[MOM_C_MAX_BUFLEN];
  char agntpkt[MOM_C_MAX_BUFLEN];
  char *p;
  int size, pktsnd, pktrec, remotelen, rspsize, agntvartype;
  int cnt = 0;
  struct sockaddr_in remote;
  struct rtentry rte;
  u_int *ch;
  struct timeval time_out;
  fd_set readfds;
  int n;
  struct timeval tval_current ;
  struct timezone tzone ;

/*
 ** If RTD was discovered to be unavailable, then
 **  If the retry timer has not yet expired, then return with error status
 */
  if (! is_rtd_available) {
    if ((gettimeofday( &tval_current, &tzone)) != 0)  return MAN_C_FAILURE;
    if ((tval_current.tv_sec - tval_start.tv_sec) < RTD_RETRY_TIMER) return MAN_C_FAILURE;
    is_rtd_available = 1;
  }

  /*
   *  Fill in the request packet to be sent to the
   *  agent/daemon.  The format is as follows:
   *
   *  [msg_type] [var. length] [variable requested]
   *   byte 1      byte 2       byte 3 - MAX
   */
  p = reqpkt;
  ch = var_oid->value;
  if (next)
    *p++ = MOM_C_AGENT_REQN;
  else
    *p++ = MOM_C_AGENT_REQ;
  *p++ = var_oid->count;
  while (cnt++ < var_oid->count)
    *p++ = *ch++ & 0xff;
  
  size = var_oid->count + 2;
  
  bcopy((char *)&dest_addr,
	p,
	sizeof(struct in_addr));
  
  size += sizeof(struct in_addr);
  reqpkt[1] += sizeof(struct in_addr);
  
  pktsnd = sendto( rtsock, reqpkt, size, 0, ( struct sockaddr * )&to_router, sizeof( struct sockaddr_in ) ) ;
  if ( pktsnd < 0 )
    return( MAN_C_FAILURE ) ;
  
  FD_ZERO(&readfds);
  FD_SET(rtsock, &readfds);
  
  time_out.tv_sec = 1;
  time_out.tv_usec = 0;
  
  if ((n = select (FD_SETSIZE, &readfds, 0, 0, &time_out)) < 0) {
    return( MAN_C_FAILURE ) ;
  }
  else if (n == 0) {	/* Time-out */
    is_rtd_available = 0;				/* Mark rtd status = unavailable */
    if ((gettimeofday( &tval_start, &tzone)) != 0)	/* Save time when failure occurred */
	tval_start.tv_sec = -1;
    return( MAN_C_FAILURE ) ;				/* Return with error */
  }
  
  remotelen = sizeof( remote ) ;
  pktrec = recvfrom( rtsock, agntpkt, MOM_C_MAX_BUFLEN, 0, ( struct sockaddr * )&remote, &remotelen ) ;
  
  /*
   *  Process response from routing table daemon
   */
  switch ( agntpkt[ 0 ] ) {
  case MOM_C_AGENT_RSP : 
    p = agntpkt ;
    p++ ;
    agntvartype = *p++ ;
    rspsize = *p++ ;
    
    /*
     * The size of the value returned better not be larger
     * that the size of an integer.
     */
    if ( rspsize > sizeof( int ) )
      return( MAN_C_FAILURE ) ;
    
    memcpy( ( char * )entry_value, p, rspsize ) ;
    return( MAN_C_SUCCESS ) ;
    break ;
    
  default:
    return( MAN_C_FAILURE ) ;
  }
  return( MAN_C_SUCCESS ) ;
}


