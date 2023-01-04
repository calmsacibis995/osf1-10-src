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
static char	*sccsid = "@(#)$RCSfile: icmp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:07:01 $";
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
 * COMPONENT_NAME: TCPIP icmp.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_ICMP,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/*
 *  Routines for handling ICMP messages
 */
 
#include "include.h"

#ifndef NSS

/*
 * icmpin() handles ICMP redirect messages.
 */

icmpin(icmp_buf, from)
	char	*icmp_buf;
	struct sockaddr_in *from;
{
  struct	icmp	*icmppkt;
  struct	sockaddr_in	gateway,	/* gateway address */
                                dest;		/* destination addr */
#ifdef	ICMP_IP_HEADER
  int hlen;
  struct sockaddr_in who_from;
  struct	ip *ip;

  /*
   * Strip off the IP header on the ICMP message.
   */
  ip = (struct ip *)icmp_buf;
  hlen = ip->ip_hl << 2;
  icmppkt = (struct icmp *)(icmp_buf + hlen);

  bzero( (char *)&who_from, sizeof( who_from));
  who_from.sin_family = AF_INET;
  who_from.sin_addr = ip->ip_src;
#else	ICMP_IP_HEADER
  struct sockaddr_in who_from;

  bzero( (char *)&who_from, sizeof( who_from));
  who_from = *from;  
  icmppkt = (struct icmp *)icmp_buf;
#endif	ICMP_IP_HEADER
  /*
   * filter ICMP network redirects
   */
  if (icmppkt->icmp_type != ICMP_REDIRECT) {
    return;
  } else {
    if ((icmppkt->icmp_code != ICMP_REDIRECT_NET) &&
        (icmppkt->icmp_code != ICMP_REDIRECT_HOST)) {
      return;
    } else {
      bzero((char *)&gateway, sizeof(gateway));
      gateway.sin_family = AF_INET;
      gateway.sin_addr = icmppkt->icmp_gwaddr;
      bzero((char *)&dest, sizeof(dest));
      dest.sin_family = AF_INET;
      dest.sin_addr = icmppkt->icmp_ip.ip_dst;
      if (icmppkt->icmp_code == ICMP_REDIRECT_NET) {
        dest.sin_addr = gd_inet_makeaddr(gd_inet_netof(dest.sin_addr),0,TRUE);
      }
/*#ifdef	ICMP_IP_HEADER*/
      rt_redirect( &dest, &gateway, &who_from, icmppkt->icmp_code);
/*#else	ICMP_IP_HEADER
      rt_redirect( &dest, &gateway, NULL, icmppkt->icmp_code);
#endif	ICMP_IP_HEADER*/
    }
  }
  return;
}


/*
 * rt_redirect() changes the routing tables in response to an ICMP redirect 
 * message
 */

rt_redirect(dst, gateway, src, icmpcode)
	struct sockaddr_in *dst, *gateway, *src;
	u_char icmpcode;
{
  int	saveinstall;
  struct	rt_entry  *rt, dup_rt;
  int	interior = 0;
  register struct interface *ifp;

  /* check gateway directly reachable */
  if (if_withdst((struct sockaddr_in *)gateway) == NULL) {
    return;
  }
  if (src != NULL && if_ifwithaddr((struct sockaddr *)src)) {
    return;
  }
  if (if_ifwithaddr((struct sockaddr *)gateway) || if_ifwithaddr((struct sockaddr *)dst)) { /* a routing loop? */
    if (((rt = rt_lookup((int)INTERIOR, (struct sockaddr_in *)dst)) ||
        (rt = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)dst)) ||
        (rt = rt_lookup((int)HOSTTABLE, (struct sockaddr_in *)dst))) &&
        ((rt->rt_state & RTS_INTERFACE) == 0))
      rt_delete(rt, KERNEL_INTR);
    return;			/* talking to ourselves */
  }
  saveinstall = install;
  install = FALSE;       /* route already in kernel */

  TRACE_ICMP(MSGSTR(ICMP_1,"ICMP: %s redirect"), icmpcode == ICMP_REDIRECT_NET ? "net" : "host");
  if (src != NULL) {
    TRACE_ICMP(MSGSTR(ICMP_4," from %s"), inet_ntoa(sock_inaddr(src)));
  }
  TRACE_ICMP(": %s", inet_ntoa(sock_inaddr(dst)));
  TRACE_ICMP(MSGSTR(ICMP_6," via %s: "), inet_ntoa(sock_inaddr(gateway)));
	
  if ( (ifp = if_ifwithaddr((struct sockaddr *)&gateway)) > (struct interface *) 0) {
    TRACE_ICMP(MSGSTR(ICMP_7,"cannot redirect to myself"));
    goto delete;
  }

  if (icmpcode == ICMP_REDIRECT_HOST) {
    rt = rt_lookup((int)HOSTTABLE, (struct sockaddr_in *)dst);
    if (src && rt && !equal(src, &rt->rt_router)) {
      TRACE_ICMP(MSGSTR(ICMP_8,"not from router in use\n"));
      if (!equal(dst, &rt->rt_dst)) {
        goto delete;
      } else {
        goto invalid;
      }
    }
    if (rt) {
      if (!(rt->rt_state & IFF_INTERFACE)) {
        if (rt_change(rt, (struct sockaddr *)gateway, rt->rt_metric, RTPROTO_REDIRECT, RTPROTO_REDIRECT, 0, 0) == 0) {
          TRACE_ICMP(MSGSTR(ICMP_9,"tables not changed\n"));
          goto invalid;
        } else {
          TRACE_ICMP("\n");
        }
      }
    } else {
      TRACE_ICMP("\n");
      (void) rt_add((int)HOSTTABLE, (struct sockaddr *)dst, (struct sockaddr *)gateway,
                        mapmetric(RIP_TO_HELLO,RIPHOPCNT_INFINITY-1), 0,
                        RTPROTO_REDIRECT, RTPROTO_REDIRECT, 0, 0);
    }
  } else {  /* is this an interior route? */
    for (ifp = ifnet; ifp; ifp = ifp->int_next)
      if (gd_inet_wholenetof(sock_inaddr(dst)) == gd_inet_wholenetof(in_addr_ofs(&ifp->int_addr))) {
        interior++;
        break;
      }
    if (rt = rt_lookup((int)INTERIOR|(int)EXTERIOR, dst)) {
      if (src && !equal(src, &rt->rt_router)) {
      	TRACE_ICMP(MSGSTR(ICMP_8,"not from router in use\n"));
      	if (!equal(dst, &rt->rt_dst)) {
          goto delete;
        } else {
          goto invalid;
        }
      }
      if (!(rt->rt_state & IFF_INTERFACE)) {
        if (rt_change(rt, (struct sockaddr *)gateway, rt->rt_metric, RTPROTO_REDIRECT, RTPROTO_REDIRECT, 0, 0) == 0) {
          TRACE_ICMP(MSGSTR(ICMP_9,"tables not changed\n"));
          goto invalid;
        } else {
          TRACE_ICMP("\n");
        }
      }
    } else {
      /*
       *	Check for and ignore martian nets
       */
      if ( is_martian(sock_inaddr(dst)) ) {
        TRACE_ICMP(MSGSTR(ICMP_15,"ignoring martian net\n"));
        TRACE_EXT(MSGSTR(ICMP_16,"rt_redirect: ignoring invalid net %s at %s"), inet_ntoa(sock_inaddr(dst)), strtime);
        goto delete;
      }
      TRACE_ICMP("\n");
      if (interior) {
        (void) rt_add((int)INTERIOR, (struct sockaddr *)dst, (struct sockaddr *)gateway,
                       mapmetric(RIP_TO_HELLO, RIPHOPCNT_INFINITY - 1),
                       0, RTPROTO_REDIRECT, RTPROTO_REDIRECT, 0, 0);
      } else {
        (void) rt_add((int)EXTERIOR, (struct sockaddr *)dst, (struct sockaddr *)gateway,
                       mapmetric(EGP_TO_HELLO, HOPCNT_INFINITY - 1),
                       0, RTPROTO_REDIRECT, RTPROTO_REDIRECT, 0, 0);
      }
    }
  }
  goto do_log;
  
delete:
  bzero((char *)&dup_rt, sizeof(struct rt_entry));
  bcopy((char *)dst, (char *)&(dup_rt.rt_dst), sizeof(struct sockaddr));
  bcopy((char *)gateway, (char *)&(dup_rt.rt_router), sizeof(struct sockaddr));
  dup_rt.rt_flags = RTF_UP | RTF_GATEWAY;
  install = TRUE;
  rt_delete(&dup_rt, KERNEL_ONLY);
  
do_log:
  TRACE_RT(MSGSTR(ICMP_18,"rt_redirect: above %s redirect from %s updated %s\n"),
          (icmpcode == ICMP_REDIRECT_NET ? "net" : "host"),
          (src ? inet_ntoa(sock_inaddr(src)) : "ICMP"),
          strtime);

invalid:
  install = saveinstall;
  return;
}
#endif NSS
