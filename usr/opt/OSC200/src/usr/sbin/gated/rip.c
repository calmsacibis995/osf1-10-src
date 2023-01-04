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
static char	*sccsid = "@(#)$RCSfile: rip.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:57:49 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

*/
/*
 * COMPONENT_NAME: TCPIP rip.c
 *
 * FUNCTIONS: procname1
 *
 * ORIGINS: 10 26 27 39 36
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   CENTER FOR THEORY AND SIMULATION IN SCIENCE AND ENGINEERING
 *			CORNELL UNIVERSITY
 *
 *      Portions of this software may fall under the following
 *      copyrights: 
 *
 *	Copyright (c) 1983 Regents of the University of California.
 *	All rights reserved.  The Berkeley software License Agreement
 *	specifies the terms and conditions for redistribution.
 *
 *  GATED - based on Kirton's EGP, UC Berkeley's routing daemon (routed),
 *	    and DCN's HELLO routing Protocol.
 */

/*
#ifndef	lint
#endif	not lint
*/

#ifdef MSG
#include "gated_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_RIP,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#include "include.h"
u_short ripport;

/*
 * 	Check out a newly received RIP packet.
 */

ripin(from, size, pkt)
	struct sockaddr *from;
	int size;
	char *pkt;
{
  register struct rt_entry *rt, *exrt;
  register struct netinfo *n;
  register struct afswitch *afp;
  register struct interface *ifp, *ifpc;
  register int OK = 0;
  struct rip *inripmsg = (struct rip *)pkt;
  int change = FALSE, check_zero = FALSE;
  int newsize, rte_table, fromproto;
  struct sockaddr_in *sin_from = (struct sockaddr_in *)from;
  u_short src_port = sin_from->sin_port;
  char *reject_msg = (char *)0;
  char type[MAXHOSTNAMELENGTH];
  int answer = FALSE;
  int split_horizon = TRUE;

  if (from->sa_family != AF_INET) {
    reject_msg = MSGSTR(RIP_1,"protocol not INET");
    goto Reject;
  }

  switch (inripmsg->rip_vers) {
    case 0:
       reject_msg = MSGSTR(RIP_2,"ignoring version 0 packets");
       goto Reject;
    case 1:
       check_zero++;
       break;
  }

  /* ignore if not from a trusted RIPer */
  if (trustedripperlist != (struct advlist *)NULL) {
    register struct advlist *ad;   /* list of trusted RIPpers */

    for (ad = trustedripperlist; ad; ad = ad->next)
      if (sin_from->sin_addr.s_addr == ad->destnet.s_addr) {
        OK++;
	break;
      }
  } else {
    OK++;
  }

  afp = &afswitch[from->sa_family];

  TRACE_RIPINPUT(ifp, (struct sockaddr_in *)from, size, (char *)inripmsg);

  switch (inripmsg->rip_cmd) {
#ifdef	RIPCMD_POLL
    case RIPCMD_POLL:
       answer = TRUE;
       split_horizon = FALSE;
#endif	RIPCMD_POLL
    case RIPCMD_REQUEST:
      (*afp->af_canon)(from);

      if ((src_port != ripport) || answer) {
        sin_from->sin_port = src_port;
        if ((ifp = if_withdst((struct sockaddr_in *)from)) <= (struct interface *)0) {
          struct sockaddr_in dst;

          dst = *sin_from;
          dst.sin_addr.s_addr = htonl(gd_inet_netof(dst.sin_addr));
	  bzero(dst.sin_zero, sizeof(dst.sin_zero));

          if ((rt = rt_lookup((int) (INTERIOR+EXTERIOR), &dst)) == (struct rt_entry *)0) {
            if ((rt = rt_lookup((int) (INTERIOR+EXTERIOR), &hello_dfltnet)) == (struct rt_entry *)0) {
              reject_msg = MSGSTR(RIP_3,"can not find interface for route");
              goto Reject;
            }
          }
          ifp = rt->rt_ifp;
        }
      } else {
        if (ifpc = if_ifwithaddr(from)) {
          return;
        }
        if (!OK) {
          reject_msg = MSGSTR(RIP_4,"not on trustedripgateways list");
          goto Reject;
        } 
        if ((ifp = if_withdst((struct sockaddr_in *)from)) <= (struct interface *)0) {
          reject_msg = MSGSTR(RIP_5,"not on same net");
          goto Reject;
        } 
        if ( ifp->int_flags & (IFF_NORIPIN|IFF_NORIPOUT) ) {
          reject_msg = MSGSTR(RIP_6,"interface marked for no RIP in/out");
          goto Reject;
        }
        if (rip_supplier <= 0) {
          reject_msg = MSGSTR(RIP_7,"not supplying RIP");
          goto Reject;
        } 
        (void) if_updateactivegw(ifp, sin_from->sin_addr.s_addr, RTPROTO_RIP);
      }

      newsize = 0;
      size -= 4 * sizeof(char);
      n = inripmsg->rip_nets;
      while (size > 0) {
        if (size < sizeof(struct netinfo)) {
          break;
        }
        size -= sizeof(struct netinfo);
        n->rip_dst.sa_family = ntohs(n->rip_dst.sa_family);
        n->rip_metric = ntohl((u_int)n->rip_metric);
        if (n->rip_dst.sa_family == AF_UNSPEC &&
            n->rip_metric == RIPHOPCNT_INFINITY &&
            size == 0) {
          supply(from, 0, ifp, split_horizon);
          return;
        }
        rt = rt_lookup((int)INTERIOR, (struct sockaddr_in *)&n->rip_dst);
        n->rip_metric = (rt == 0) ? RIPHOPCNT_INFINITY :
                        min(mapmetric(HELLO_TO_RIP, (u_short)rt->rt_metric) + 1 +
                            ifp->int_metric, (u_short)RIPHOPCNT_INFINITY);
        n++;
        newsize += sizeof(struct netinfo);
      }
      if (newsize > 0) {
        inripmsg->rip_cmd = RIPCMD_RESPONSE;
        newsize += sizeof(int);
        bcopy((char *) inripmsg, (char *)ripmsg, newsize);
        TRACE_RIPOUTPUT(ifp, (struct sockaddr_in *)from, newsize);
        (*afp->af_output)(rip_socket, 0, from, newsize);
      }
      return;
    case RIPCMD_TRACEON:
    case RIPCMD_TRACEOFF:
      if (!OK) {
        reject_msg = MSGSTR(RIP_4,"not on trustedripgateways list");
        goto Reject;
      }
      if ((*afp->af_portcheck)(from) == 0) {
        reject_msg = MSGSTR(RIP_9,"not from a trusted port");
        goto Reject;
      }
      if ((ifp = if_withdst((struct sockaddr_in *)from)) <= (struct interface *)0) {
        reject_msg = MSGSTR(RIP_5,"not on same net");
        goto Reject;
      }
      if (ifp->int_flags & IFF_NORIPIN) {
        reject_msg = MSGSTR(RIP_11,"not listening to RIP on this interface");
        goto Reject;
      }
      *(pkt + size) = '\0';
      reject_msg = MSGSTR(RIP_12,"TRACE packets not supported");
      goto Reject;
#ifdef	RIPCMD_POLLENTRY
    case RIPCMD_POLLENTRY:
      n = inripmsg->rip_nets;
      newsize = sizeof (struct entryinfo);
      n->rip_dst.sa_family = ntohs(n->rip_dst.sa_family);
      if (n->rip_dst.sa_family == AF_INET && afswitch[n->rip_dst.sa_family].af_hash) {
        rt = rt_lookup((int)INTERIOR, (struct sockaddr_in *)&n->rip_dst);
      } else {
        rt = 0;
      }
      if (rt) {       /* don't bother to check rip_vers */
        struct entryinfo *e = (struct entryinfo *) n;
        e->rtu_dst = rt->rt_dst;
        e->rtu_dst.sa_family = ntohs(e->rtu_dst.sa_family);
        e->rtu_router = rt->rt_router;
        e->rtu_router.sa_family = ntohs(e->rtu_router.sa_family);
        e->rtu_flags = ntohs((unsigned short) rt->rt_flags);
        e->rtu_state = ntohs((unsigned short) rt->rt_state);
        e->rtu_timer = ntohl((unsigned int) rt->rt_timer);
        e->rtu_metric = ntohl((unsigned int) mapmetric(HELLO_TO_RIP, (unsigned short) rt->rt_metric));
        if (ifp = rt->rt_ifp) {
          e->int_flags = ntohl((unsigned int) ifp->int_flags);
          (void) strncpy(e->int_name, rt->rt_ifp->int_name, sizeof(e->int_name));
        } else {
          e->int_flags = 0;
          (void) strcpy(e->int_name, MSGSTR(RIP_13,"(none)"));
        }
      }	else {
        bzero((char *)n, newsize);
      }
      bcopy((char *) inripmsg, (char *)ripmsg, newsize);
      TRACE_RIPOUTPUT(ifp, (struct sockaddr_in *)from, newsize);
      (*afp->af_output)(rip_socket, 0, from, newsize);
      return;
#endif	RIPCMD_POLLENTRY
    case RIPCMD_RESPONSE:
      /*
       *  Are we talking to ourselves???
       *
       *  if_ifwithaddr() handles PTP's also.  If from a
       *  dst of a PTP link, let it through for further processing.
       *  you shouldn't receive your own RIPs on a PTP.
       */

      if (ifpc = if_ifwithaddr(from)) {
        rt_ifupdate(ifpc);
        if ((ifpc->int_flags & IFF_POINTOPOINT) == 0) {
          return;
        }
      }
      if (!OK) {
#ifdef	notdef
        reject_msg = MSGSTR(RIP_4,"not on trustedripgateways list");
        goto Reject;
#else	notdef
	return;
#endif	notdef
      }
      if ((*afp->af_portmatch)(from) == 0) {
        reject_msg = MSGSTR(RIP_9,"not from a trusted port");
        goto Reject;
      }
      if ((ifp = if_withdst((struct sockaddr_in *)from)) <= (struct interface *)0) {
        reject_msg = MSGSTR(RIP_5,"not on same net");
        goto Reject;
      }
      if (ifp->int_flags & IFF_NORIPIN) {
        reject_msg = MSGSTR(RIP_17,"interface marked for no RIP in");
        goto Reject;
      }

      (*afp->af_canon)(from);

      /*
       * update interface timer on interface that packet came in on.
       */
      rt_ifupdate(ifp);

      fromproto = if_updateactivegw(ifp,sin_from->sin_addr.s_addr,RTPROTO_RIP);
      size -= 4 * sizeof (char);
      n = inripmsg->rip_nets;
      for (; size > 0; size -= sizeof (struct netinfo), n++) {
        if (size < sizeof (struct netinfo))
          break;
        n->rip_dst.sa_family = ntohs(n->rip_dst.sa_family);
        /*
         *  Convert metric to host byte order.  If metric is zero, set to one to avoid interface routes
         */
        if ( (n->rip_metric = ntohl((u_int)n->rip_metric)) == 0) {
          n->rip_metric = 1;
        }
        /*
         * Now map rip metric to Time delay in millisec's.
         */
        n->rip_metric = mapmetric(RIP_TO_HELLO,(u_short)(n->rip_metric+ifp->int_metric));

        if (n->rip_dst.sa_family != AF_INET)
          continue;
        afp = &afswitch[n->rip_dst.sa_family];
        if (((*afp->af_checkhost)(&n->rip_dst)) == 0)
          continue;
        if ((*afp->af_ishost)(&n->rip_dst)) {
          rte_table = HOSTTABLE;
        } else {
          rte_table = INTERIOR;
        }
        rt = rt_lookup(rte_table, (struct sockaddr_in *)&n->rip_dst);
        if (rt == NULL) {  /* new route */
          struct sockaddr_in *tmp = (struct sockaddr_in *)&n->rip_dst;
          struct rt_entry rttmp;

          bzero((char *)&rttmp, sizeof(rttmp));
          rttmp.rt_listenlist = control_lookup(RT_NOLISTEN, (struct sockaddr_in *)&n->rip_dst);
          rttmp.rt_srclisten = control_lookup(RT_SRCLISTEN, (struct sockaddr_in *)&n->rip_dst);
          if (is_valid_in(&rttmp, RTPROTO_RIP, ifp, (struct sockaddr_in *)from) == 0)
            continue;
          /*
           *	Check for and ignore martain nets
           */
          if (is_martian(tmp->sin_addr)) {
            char badgate[16];
            (void) strcpy(badgate, inet_ntoa(sin_from->sin_addr));
            TRACE_EXT(MSGSTR(RIP_18,"ripin: ignoring invalid net %s from %s at %s"),
              inet_ntoa(tmp->sin_addr), badgate, strtime);
            continue;
          }
          if (n->rip_metric >= DELAY_INFINITY) {
            continue;
          }
          if ((rte_table == INTERIOR) &&
              (exrt = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)&n->rip_dst))) {
            do {
              rt_delete(exrt, KERNEL_INTR);
            } while (exrt = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)&n->rip_dst));
          }
          (void) rt_add(rte_table, &n->rip_dst, from, n->rip_metric, 0, RTPROTO_RIP, fromproto, 0, 0);
          change = TRUE;
        } else {
          if ((rt->rt_flags & RTF_GATEWAY) == 0) {
            continue;
          }
          if (rt->rt_state & (RTS_INTERFACE | RTS_STATIC) ) {
            continue;
          }
          if (is_valid_in(rt, RTPROTO_RIP, ifp, (struct sockaddr_in *)from) == 0) {
            continue;
          }
          if ((rte_table == INTERIOR) &&
              (exrt = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)&n->rip_dst))) {
            do {
              rt_delete(exrt, KERNEL_INTR);
            } while (exrt = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)&n->rip_dst));
          }
          if (equal(&rt->rt_router, from)) {
            if (n->rip_metric >= DELAY_INFINITY) {
              if (rt->rt_metric < DELAY_INFINITY) {
                (void) rt_unreach(rt);
                change = TRUE;
              }
              continue;
            }
            if (n->rip_metric != rt->rt_metric)
              if (rt_change(rt, from, n->rip_metric, RTPROTO_RIP, fromproto, 0, 0))
                change = TRUE;
            rt->rt_timer = 0;
          } else {
            /* if a metric is INFINITY at this point
             * we don't care about the new router.
             * The only way it would accept this
             * route anyway would be if the metric
             * was already 16 and the route was old.
             * we will stick with old gateway and let
             * it time out in 2 minutes if it wants to.
             *
             * also, if the current metric is INFINITY,
             * we will only listen to our current
             * gateway.  Yes, a terrible hold down!
             * if our current gateway says nothing then
             * this route will expire in 120 seconds.
             */
            if ((n->rip_metric >= DELAY_INFINITY) ||
                (rt->rt_metric >= DELAY_INFINITY))
              continue;
            if ((n->rip_metric < rt->rt_metric) ||
                ((rt->rt_timer > (EXPIRE_TIME/2)) &&
                (rt->rt_metric == n->rip_metric))) {
              if ((rt->rt_proto & RTPROTO_HELLO) &&
                  (n->rip_metric >= rt->rt_hwindow.h_min))
                continue;
              if (rt_change(rt, from, n->rip_metric, RTPROTO_RIP, fromproto, 0, 0))
                change = TRUE;
              rt->rt_timer = 0;
            }
          }
        }
      }  /*  for each net */
      break;
    default:
      reject_msg = MSGSTR(RIP_19,"invalid or not implemented command");
      goto Reject;
  }
  if ( change && (tracing & TR_RT) ) {
    printf(MSGSTR(RIP_20,"rip_update: above routes supplied from %s updates %s\n"),
                inet_ntoa(sin_from->sin_addr), strtime);
  }
  return;

Reject:
  if (inripmsg->rip_cmd < RIPCMD_MAX) {
    (void) strcpy(type, ripcmds[inripmsg->rip_cmd]);
  } else {
    (void) sprintf(type, "#%d", inripmsg->rip_cmd);
  }
  TRACE_RIP(MSGSTR(RIP_22,"ripin: ignoring RIP %s packet from %s - %s\n"),
    type, inet_ntoa(sin_from->sin_addr), reject_msg);
#ifdef	notdef
  syslog(LOG_INFO, MSGSTR(RIP_22,"ripin: ignoring RIP %s packet from %s - %s\n"),
    type, inet_ntoa(sin_from->sin_addr), reject_msg);
#endif	notdef
  return;
}

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */

toall(f)
	int (*f)();
{
  register struct interface *ifp;
  register struct advlist *ad;
  register struct sockaddr *dst;
  register int flags;
  extern struct interface *ifnet;

  if (!(rip_pointopoint)) {
    for (ifp = ifnet; ifp; ifp = ifp->int_next) {
      TRACE_JOB(MSGSTR(RIP_24,"toall: Checking interface %s\n"), ifp->int_name);
      if (ifp->int_flags & IFF_PASSIVE) {
        TRACE_JOB(MSGSTR(RIP_25,"toall: No RIP - %s is passive\n"), ifp->int_name);
        continue;
      }
      if ( !(ifp->int_flags & IFF_UP) ) {
        TRACE_JOB(MSGSTR(RIP_26,"toall: No RIP - %s is down\n"), ifp->int_name);
        continue;
      }
      if (ifp->int_flags & IFF_NORIPOUT) {
        TRACE_JOB(MSGSTR(RIP_27,"toall: RIP not allowed out %s\n"), ifp->int_name);
        continue;
      }
      dst = (ifp->int_flags & IFF_BROADCAST) ? &ifp->int_broadaddr :
               (ifp->int_flags & IFF_POINTOPOINT) ? &ifp->int_dstaddr :
                   &ifp->int_addr;
      dst->sa_family = AF_INET;   /*  what else???? */
      flags = ((ifp->int_flags & IFF_INTERFACE) &&
               !(ifp->int_flags & IFF_POINTOPOINT)) ? MSG_DONTROUTE : 0;
      TRACE_JOB(MSGSTR(RIP_28,"toall: Sending RIP packet to %s, flags %d, interface %s\n"),
         inet_ntoa(sock_inaddr(dst)), flags, ifp->int_name);
      (*f)(dst, flags, ifp, TRUE);
    }
  }
  for (ad = srcriplist; ad; ad = ad->next) {
    struct sockaddr_in tmpdst;

    bzero((char *)&tmpdst, sizeof(tmpdst));
    tmpdst.sin_family = AF_INET;
    tmpdst.sin_addr = ad->destnet;
    if ((ifp = if_withdst(&tmpdst)) <= (struct interface *)0) {
      syslog(LOG_ERR, MSGSTR(RIP_29,"toall: Source RIP gateway %s not on same net"),
                    inet_ntoa(tmpdst.sin_addr));
      TRACE_TRC(MSGSTR(RIP_30,"toall: Source RIP gateway %s not on same net\n"),
                    inet_ntoa(tmpdst.sin_addr));
      continue;
    }
    if (((ifp->int_flags & IFF_UP) == 0) || (ifp->int_flags & IFF_NORIPOUT)) {
      continue;
    }
    flags = ((ifp->int_flags & IFF_INTERFACE) &&
             !(ifp->int_flags & IFF_POINTOPOINT)) ? MSG_DONTROUTE : 0;
    TRACE_JOB(MSGSTR(RIP_31,"toall: Sending RIP packet to %s, flags %d, interface %s\n"),
              inet_ntoa(sock_inaddr(&tmpdst)), flags, ifp->int_name);
    (*f)(&tmpdst, flags, ifp, TRUE);
  }
}

/*
 * Output a preformed RIP packet.
 */

/*ARGSUSED*/
sendripmsg(dst, flags, ifp, do_split_horizon)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int do_split_horizon;
{
  register u_int tmp = ntohl((u_int)ripmsg->rip_nets[0].rip_metric);
  struct rt_entry *rt;
  struct netinfo *n = ripmsg->rip_nets;

  if (dst->sa_family != AF_INET) {
    return;
  }
  /*
   * Check to see if we are sending the initial RIP request to other
   * gateways.  That request has no restrictions other than whether RIP
   * is allowed on that interface or not.  This restriction is handled
   * in toall().
   */
  if (!((ntohs(n->rip_dst.sa_family) == AF_UNSPEC) &&
       (ntohl((u_int)n->rip_metric) == RIPHOPCNT_INFINITY))) {
    n->rip_dst.sa_family = ntohs(n->rip_dst.sa_family);
    rt = rt_lookup((int)INTERIOR, (struct sockaddr_in *)&n->rip_dst);
    if (rt == NULL) {
      rt = rt_lookup((int)HOSTTABLE, (struct sockaddr_in *)&n->rip_dst);
      if (rt == NULL) {
        syslog(LOG_ERR, MSGSTR(RIP_32,"sendripmsg: bad route %s"),
               inet_ntoa(sock_inaddr(&n->rip_dst)));
        TRACE_TRC(MSGSTR(RIP_33,"sendripmsg: bad route %s\n"),
               inet_ntoa(sock_inaddr(&n->rip_dst)));
        return;
      }
    }
    n->rip_dst.sa_family = htons(n->rip_dst.sa_family);
    /*
     * make sure this route can be announced via this interface/proto.
     */
    if (!is_valid(rt, RTPROTO_RIP, ifp)) {
      return;
    }
    /*
     * since we are only sending out this one packet, we can add the
     * interface metric here.  Don't forget Split Horizon.
     */
    if ((rt->rt_ifp == ifp) &&
        do_split_horizon &&
        (rt->rt_fromproto & (RTPROTO_RIP|RTPROTO_DIRECT|RTPROTO_KERNEL)) &&
        ((rt->rt_state & RTS_STATIC) == 0)) {
      tmp = ntohl((u_int)n->rip_metric);
      n->rip_metric = htonl((u_int)RIPHOPCNT_INFINITY);
    } else if ((tmp = ntohl((u_int) n->rip_metric)) != RIPHOPCNT_INFINITY) {
      if ((tmp + ifp->int_metric) >= RIPHOPCNT_INFINITY) {
        n->rip_metric = htonl((u_int)RIPHOPCNT_INFINITY);
      } else {
        n->rip_metric = htonl((u_int)(tmp + ifp->int_metric));
      }
    }
  }
  (*afswitch[dst->sa_family].af_output)(rip_socket, flags,
                                           dst, sizeof (struct rip));
  TRACE_RIPOUTPUT(ifp, (struct sockaddr_in *)dst, sizeof (struct rip));
  n->rip_metric = htonl(tmp);
}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 */
supply(dst, flags, ifp, do_split_horizion)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int do_split_horizion;
{
  register struct rt_entry *rt;
  struct netinfo *n;
  register struct rthash *rh;
  struct rthash *base;
  int doinghost, size, iff_subnets, same_net;
  int (*output)() = afswitch[AF_INET].af_output;
  u_short metric, split_horizon;

  ripmsg->rip_cmd = RIPCMD_RESPONSE;
  ripmsg->rip_vers = RIPVERSION;
  n = ripmsg->rip_nets;
  
  iff_subnets = (ifp->int_flags & IFF_SUBNET) == IFF_SUBNET;

  for (base = hosthash, doinghost = 1; doinghost >= 0; base = nethash, doinghost--) {
    for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
      for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
        /*
         * don't broadcast subnets where they shouldn't be announced.
         */
	if (!doinghost) {
	  same_net = (gd_inet_wholenetof(sock_inaddr(&rt->rt_dst)) == gd_inet_wholenetof(sock_inaddr(dst)));
	  if (rt->rt_state & RTS_SUBNET) {
	    if ( !iff_subnets ) {
	      continue;		/* Subnets not allowed out this interface */
	    }
	    if (!same_net) {
	      continue;		/* Not a subnet of this interface */
	    }
	  } else {
	    if (iff_subnets && same_net) {
	      continue;		/* Don't sent whole net via a subneted interface */
	    }
	  }
        }
        if ((ifp == rt->rt_ifp) &&
	    do_split_horizion &&
            (rt->rt_fromproto & (RTPROTO_RIP|RTPROTO_DIRECT|RTPROTO_KERNEL)) &&
            !(rt->rt_state & RTS_STATIC) ) {
          split_horizon = RIPHOPCNT_INFINITY;
        } else {
          split_horizon = 0;
        }
        switch (rt->rt_proto) {
          case RTPROTO_RIP:
          case RTPROTO_HELLO:
          case RTPROTO_DIRECT:
          case RTPROTO_KERNEL:
            if (!(doinghost) && !(rt->rt_state & RTS_INTERIOR)) {
              continue;
            }
            if ( !is_valid(rt, RTPROTO_RIP, ifp) ) {
              continue;
            }
            metric = mapmetric(HELLO_TO_RIP, (u_short)rt->rt_metric) + ifp->int_metric + 1;
            break;
          case RTPROTO_REDIRECT:
            continue;
          case RTPROTO_DEFAULT:
            if (!rip_gateway) {
              continue;
            }
            if ( !is_valid(rt, RTPROTO_RIP, ifp) ) {
              continue;
            }
            metric = rip_default + ifp->int_metric;
            split_horizon = 0;
            break;
          case RTPROTO_EGP:
            if (rt->rt_as == mysystem) {
              continue;
            }
            switch (sendAS(my_aslist, rt->rt_as)) {
              case 0:	/* not valid to send */
                continue;
              case 2:	/* valid to send - announce clauses apply */
                if ( !is_valid(rt, RTPROTO_RIP, ifp) ) {
                  continue;
                }
                break;
              case 1:	/* valid to send - announce clauses do not apply */
                break;
              case -1:	/* no AS restrictions */
                continue;
            }
            metric = mapmetric(HELLO_TO_RIP, (u_short)rt->rt_metric) + ifp->int_metric;
            split_horizon = 0;
            break;
          default:
            syslog(LOG_ERR, MSGSTR(RIP_34,"supply: Unknown protocol %d for net %s"),
                   rt->rt_proto, inet_ntoa(sock_inaddr(&rt->rt_dst)));
            TRACE_TRC(MSGSTR(RIP_35,"supply: Unknown protocol %d for net %s\n"),
                   rt->rt_proto, inet_ntoa(sock_inaddr(&rt->rt_dst)));
        }
        if (split_horizon) {
          metric = split_horizon;
        }
        if (!(rt->rt_ifp->int_flags & IFF_UP)) {
          metric = RIPHOPCNT_INFINITY;
        }
        if ((ifp->int_flags & IFF_RIPFIXEDMETRIC) &&
            (rt->rt_proto != RTPROTO_DEFAULT) &&
            (metric < RIPHOPCNT_INFINITY)) {
          metric = ifp->int_ripfixedmetric;
        }
        size = (char *)n - rip_packet;
        if (size > (RIPPACKETSIZE - sizeof (struct netinfo))) {
          (*output)(rip_socket, flags, dst, size);
          TRACE_RIPOUTPUT(ifp, (struct sockaddr_in *)dst, size);
          n = ripmsg->rip_nets;
        }
        n->rip_dst = rt->rt_dst;
        n->rip_dst.sa_family = htons(n->rip_dst.sa_family);
        if (metric > RIPHOPCNT_INFINITY) {
          metric = RIPHOPCNT_INFINITY;
        }
        n->rip_metric = htonl((u_int)metric);
        n++;
      }
    }
  }
  if (n != ripmsg->rip_nets) {
    size = (char *)n - rip_packet;
    (*output)(rip_socket, flags, dst, size);
    TRACE_RIPOUTPUT(ifp, (struct sockaddr_in *)dst, size);
  }
}
