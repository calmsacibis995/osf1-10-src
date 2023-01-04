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
static char	*sccsid = "@(#)$RCSfile: rt_table.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:59:02 $";
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
 * COMPONENT_NAME: TCPIP rt_table.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_RT_TABLE,n,s) 
#else
#define MSGSTR(n,s) s
#endif

 /*
 * Some modified from Routing Table Management Daemon routed/tables.c.
 *
 * Functions: rt_lookup, rt_add, rt_change, rt_delete,
 *	      rt_default, rt_time, rt_gwunreach, rt_redirect,
 *	      rt_check_default, rt_find, addrouteforif,
 *            init_hwin, add_win
 */

#include "include.h"

/*
 * rt_lookup() looks up a destination network for an exact match.
 */

struct rt_entry *
rt_lookup(table, dst)
	int table;
	struct sockaddr_in *dst;
{
  register struct rt_entry *rt;
  register struct rthash *rh;
  register unsigned hash;
  struct afhash h;


  if (dst->sin_family != AF_INET)
    return(0);
		
  (*afswitch[dst->sin_family].af_hash)(dst, &h);
  if (table == HOSTTABLE) {
    hash = h.afh_hosthash;
    rh = &hosthash[hash & ROUTEHASHMASK];
  } else {
    hash = h.afh_nethash;
    rh = &nethash[hash & ROUTEHASHMASK];
  }
  for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
    if (rt->rt_hash != hash)
      continue;
    if (equal(&rt->rt_dst, dst) && (rt->rt_state & table))
      return (rt);
  }
  return (0);
}

/*
 * rt_add() adds a route to either the interior or exterior routing tables.
 */

struct rt_entry*
rt_add(table, dst, gate, metric, state, proto, fromproto, as, metric_exterior)
	int table;
	struct sockaddr *dst, *gate;
	int metric, state, proto, fromproto;
	u_short as;
	u_int metric_exterior;
{
  register struct rt_entry *rt;
  struct rthash *rh;
  unsigned hash;
  int af = AF_INET;
  struct afhash h;

  /*
   * take care of routes pointing back to the localhost interface.
   * We don't care about them and will ignore them in the kernel forever.
   */
  if ((unsigned)gd_inet_netof(sock_inaddr(gate)) == (unsigned)LOOPBACKNET) {
    return((struct rt_entry *) 1);	/* Need to fix this! */
  }

  (*afswitch[af].af_hash)((struct sockaddr_in *)dst, &h);
  if (table == HOSTTABLE) {
    hash = h.afh_hosthash;
    rh = &hosthash[hash & ROUTEHASHMASK];
  } else {
    hash = h.afh_nethash;
    rh = &nethash[hash & ROUTEHASHMASK];
  }
  rt = (struct rt_entry *)malloc(sizeof(*rt));
  if (rt == 0) {
    syslog(LOG_ERR, MSGSTR(RT_TABLE_1,"rt_add: malloc: out of memory\n"));
    return(rt);
  }
  rt->rt_hash = hash;
  rt->rt_dst = *dst;
  /*
   * set local part of dest addr zero for nets
   */
  if ((gd_inet_lnaof(in_addr_ofs(&rt->rt_dst)) != 0) && (table != HOSTTABLE)) {
    in_addr_ofs(&rt->rt_dst) = gd_inet_makeaddr(gd_inet_netof(in_addr_ofs(&rt->rt_dst)),0,TRUE);
  }
  rt->rt_router = *gate;
  rt->rt_metric = metric;
  rt->rt_metric_exterior = metric_exterior;
  rt->rt_timer = 0;
  rt->rt_flags = RTF_UP;
  if (table == HOSTTABLE) {
    rt->rt_flags |= RTF_HOST;
  }
  rt->rt_state = state | RTS_CHANGED | table;
  if ((htonl(gd_inet_wholenetof(sock_inaddr(&rt->rt_dst))) !=
      sock_inaddr(&rt->rt_dst).s_addr) && ((rt->rt_flags & RTF_HOST) == 0)) {
    rt->rt_state |= RTS_SUBNET;
  }
  rt->rt_proto = proto;
  rt->rt_fromproto = fromproto;
  if (as) {
    rt->rt_as = as;
  } else {
    rt->rt_as = mysystem;
  }
  rt->rt_announcelist = (struct restrictlist *)NULL;
  rt->rt_noannouncelist = (struct restrictlist *)NULL;
  rt->rt_listenlist = (struct restrictlist *)NULL;
  rt->rt_srclisten = (struct restrictlist *)NULL;
  rt->rt_ifp = if_withdst((struct sockaddr_in *)&rt->rt_router);
#ifndef NSS
  if (rt->rt_ifp == (struct interface *)NULL) {
#else   NSS
  if ((rt->rt_ifp == (struct interface *)NULL) && (rt->rt_proto != RTPROTO_IGP))
 {
#endif  NSS
    char badgate[16];

    (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
    TRACE_TRC(MSGSTR(RT_TABLE_2,"rt_add: interface not found for net %-15s gateway %s\n"), inet_ntoa(sock_inaddr(&rt->rt_dst)), badgate);
    free((char *)rt);
    return((struct rt_entry *) 0);
  }
#ifndef NSS
  if (rt->rt_proto == RTPROTO_HELLO) {
    /* initialize hello history window. */
    init_hwindow(&rt->rt_hwindow, metric);
  }
  if (donotlisten != 0) {
    if ((rt->rt_listenlist = control_lookup(RT_NOLISTEN, (struct sockaddr_in *)dst)) <= (struct restrictlist *)0) {
      rt->rt_listenlist = (struct restrictlist *)NULL;
    } else {
      /* see if there is a restriction before adding */
      if ( (rt->rt_proto != RTPROTO_EGP) && !(rt->rt_state & (RTS_PASSIVE|RTS_INTERFACE|RTS_STATIC)) ) {
        /* no restrictions in EGP */
        if (is_valid_in(rt, rt->rt_proto, rt->rt_ifp, (struct sockaddr_in *)gate) == 0) {
          free((char *) rt);
          return((struct rt_entry *) 0);
        }
      }
    }
  }
  if (islisten != 0) {
    if ((rt->rt_srclisten = control_lookup(RT_SRCLISTEN, (struct sockaddr_in *)dst)) <= (struct restrictlist *)0) {
      rt->rt_srclisten = (struct restrictlist *)NULL;
    } else {
      /* see if there is a restriction before adding */
      /*
       * No restrictions in EGP.  Also, don't prevent
       * rt_default(ADD) from adding default route.
       */
      if ( (rt->rt_proto != RTPROTO_EGP) && (rt->rt_proto != RTPROTO_DEFAULT)
           && !(rt->rt_state & (RTS_PASSIVE|RTS_INTERFACE|RTS_STATIC)) ) {
        if (is_valid_in(rt, rt->rt_proto, rt->rt_ifp, (struct sockaddr_in *)gate) == 0) {
          free((char *) rt);
          return((struct rt_entry *) 0);
        }
      }
    }
  }
#endif NSS
  if ((glob_announcethesenets != 0) || (announcethesenets != 0)) {
    if ((rt->rt_announcelist = control_lookup(RT_ANNOUNCE, (struct sockaddr_in *)dst)) <= (struct restrictlist *)0) {
      rt->rt_announcelist = (struct restrictlist *)NULL;
#ifdef	notdef
      if ((table == INTERIOR) || (table == HOSTTABLE)) {
        if (glob_announcethesenets != 0) {
          syslog(LOG_NOTICE,MSGSTR(RT_TABLE_3,"invalid %s route %s not announced"),
            (table == INTERIOR) ? "net" : "host", inet_ntoa(sock_inaddr(&rt->rt_dst)));
        } else {
          syslog(LOG_NOTICE, MSGSTR(RT_TABLE_6,"no announcelist for %s route %s, may be restricted "),
            (table == INTERIOR) ? "net" : "host", inet_ntoa(sock_inaddr(&rt->rt_dst)));
        }
      }
#endif	notdef
    }
  }
  if ((glob_donotannounce != 0) || (donotannounce != 0)) {
    if ((rt->rt_noannouncelist = control_lookup(RT_NOANNOUNCE, (struct sockaddr_in *)dst)) <= (struct restrictlist *)0) {
      rt->rt_noannouncelist = (struct restrictlist *)NULL;
    } else {
#ifdef	notdef
      if ((table == INTERIOR) || (table == HOSTTABLE)) {
        if (glob_announcethesenets != 0) {
          syslog(LOG_NOTICE,MSGSTR(RT_TABLE_3,"invalid %s route %s not announced"),
            (table == INTERIOR) ? "net" : "host", inet_ntoa(sock_inaddr(&rt->rt_dst)));
        } else {
          syslog(LOG_NOTICE,MSGSTR(RT_TABLE_12,"noannouncelist for %s route %s"),
            (table == INTERIOR) ? "net" : "host", inet_ntoa(sock_inaddr(&rt->rt_dst)));
        }
      }
#endif	notdef
    }
  }
  if (table == EXTERIOR) {
    rt->rt_flags |= RTF_GATEWAY;
    TRACE_ACTION(ADD, rt)
  } else {
    switch (rt->rt_proto) {
      case RTPROTO_DIRECT:
        break;
#ifndef NSS
      case RTPROTO_RIP:
        if (metric) {
          rt->rt_flags |= RTF_GATEWAY;
        }
        break;
#endif NSS
      default:
        rt->rt_flags |= RTF_GATEWAY;
        break;
    }
    TRACE_ACTION(ADD, rt);
  }
  insque(rt, rh);
#ifndef NSS
  /*
   * If the ioctl fails because the gateway is unreachable
   * from this host, discard the entry.  This should only
   * occur because of an incorrect entry in /etc/gated.conf.
   */
  if (install && !(rt->rt_state & RTS_NOTINSTALL)) {
    if (ioctl(s, SIOCADDRT, (char *)&rt->rt_rt) < 0) {
      char badgate[16];

      (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
      (void) sprintf(err_message, "rt_add: SIOCADDRT dst %s gw %s flags %x",
                    inet_ntoa(((struct sockaddr_in *)&rt->rt_dst)->sin_addr),
                    badgate, rt->rt_flags);
      p_error(err_message);
      if (errno == ENETUNREACH) {
        TRACE_ACTION(DELETE, rt);
        remque(rt);
        free((char *)rt);
      }
    }
  }
#endif NSS
  n_routes++;
#ifndef NSS
  rttable_changed++;
#endif NSS
  return(rt);
}

/*
 * rt_change() changes a route &/or notes that an update was received.
 * returns 1 if change made
 */

rt_change(rt, gate, metric, proto, fromproto, as, metric_exterior)
	struct rt_entry *rt;
	struct sockaddr *gate;
	short metric;
	int proto, fromproto;
	u_short as;
	u_int metric_exterior;
{
  int doioctl = 0, metricchanged = 0, protochanged = 0;
  int ifchanged = 0, fromprotochanged = 0, aschanged = 0;
  struct interface *t_ifp;

  if (rt->rt_ifp != (t_ifp = if_withdst((struct sockaddr_in *)gate))) {
#ifndef NSS
    if (rt->rt_ifp == (struct interface *)NULL) {
#else   NSS
    if ((rt->rt_ifp == (struct interface *)NULL) && (rt->rt_proto != RTPROTO_IGP
)) {
#endif  NSS
      char badgate[16];

      (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
      TRACE_TRC(MSGSTR(RT_TABLE_2,"rt_add: interface not found for net %-15s gateway %s\n"), inet_ntoa(sock_inaddr(&rt->rt_dst)), badgate);
      return(0);
    }
    rt->rt_ifp = t_ifp;
    ifchanged++;
  }
  rt->rt_state |= RTS_CHANGED;		/* ensures route age reset */
  rt->rt_state &= ~RTS_PASSIVE;         /* no longer passive if changed */
  if (!equal(&rt->rt_router, gate)) {
    doioctl++;
  }
  if (metric != rt->rt_metric) {
    metricchanged++;
    rt->rt_metric = metric;
  }
  if (metric_exterior != rt->rt_metric_exterior) {
    metricchanged++;
    rt->rt_metric_exterior = metric_exterior;
  }
  if (fromproto != rt->rt_fromproto) {
    fromprotochanged++;
    rt->rt_fromproto = fromproto;
  }
  if (as == 0) {
    as = mysystem;
  }
  if (as != rt->rt_as) {
    aschanged++;
    rt->rt_as = as;
  }
  if (proto != rt->rt_proto) {
    protochanged++;
#ifndef NSS
    rt->rt_proto = proto;
    if (proto & RTPROTO_HELLO) {
      init_hwindow(&rt->rt_hwindow, rt->rt_metric);
    }
#endif NSS
  }
  if (doioctl) {
    struct rt_entry oldroute;

    oldroute = *rt;
    rt->rt_router = *gate;
#ifndef NSS
    if (install && !(rt->rt_state & RTS_NOTINSTALL)) {
#ifndef RTM_ADD

      if (ioctl(s, SIOCADDRT, (char *)&rt->rt_rt) < 0) {
        char badgate[16];

        (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
        (void) sprintf(err_message, "rt_change: SIOCADDRT dst %s gw %s flags %x",
                      inet_ntoa(((struct sockaddr_in *)&rt->rt_dst)->sin_addr),
                      badgate, rt->rt_flags);
        p_error(err_message);
      }

       if (ioctl(s, SIOCDELRT, (char *)&oldroute.rt_rt) < 0) {
        char badgate[16];

        (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&oldroute.rt_router)->sin_addr));
        (void) sprintf(err_message, "rt_change: SIOCDELRT dst %s gw %s flags %x",
                      inet_ntoa(((struct sockaddr_in *)&oldroute.rt_dst)->sin_addr),
                      badgate, oldroute.rt_flags);
        p_error(err_message);
      }

#else RTM_ADD

       if (ioctl(s, SIOCDELRT, (char *)&oldroute.rt_rt) < 0) {
        char badgate[16];

        (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&oldroute.rt_router)->sin_addr));
        (void) sprintf(err_message, "rt_change: SIOCDELRT dst %s gw %s flags %x",
                      inet_ntoa(((struct sockaddr_in *)&oldroute.rt_dst)->sin_addr),
                      badgate, oldroute.rt_flags);
        p_error(err_message);
      }

      if (ioctl(s, SIOCADDRT, (char *)&rt->rt_rt) < 0) {
        char badgate[16];

        (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
        (void) sprintf(err_message, "rt_change: SIOCADDRT dst %s gw %s flags %x",
                      inet_ntoa(((struct sockaddr_in *)&rt->rt_dst)->sin_addr),
                      badgate, rt->rt_flags);
        p_error(err_message);
      }

#endif RTM_ADD

    }
#endif NSS

  }
  if (doioctl || metricchanged || protochanged || ifchanged || fromprotochanged || aschanged) {
    TRACE_ACTION(CHANGE, rt);
    return(1);
#ifdef NSS
    rttable_changed++;
#endif NSS
  }
  return(0);
}

/*
 * rt_delete deletes a route from the routing table.
 */

rt_delete(rt, just_kernel)
	struct rt_entry *rt;
	int just_kernel;
{
#ifdef  NSS
  struct rt_entry       *rt_igp;
#endif  NSS

  TRACE_ACTION(DELETE, rt);
#ifndef NSS
  if (install && !(rt->rt_state & RTS_NOTINSTALL)) {
    if (ioctl(s, SIOCDELRT, (char *)&rt->rt_rt) < 0) {
      char badgate[16];

      (void) strcpy(badgate, inet_ntoa(((struct sockaddr_in *)&rt->rt_router)->sin_addr));
      (void) sprintf(err_message,"rt_delete: SIOCDELRT dst %s gw %s flags %x",
                     inet_ntoa(((struct sockaddr_in *)&rt->rt_dst)->sin_addr),
                     badgate, rt->rt_flags);
      p_error(err_message);
    }
  }
#else   NSS
  if (rt->rt_proto == RTPROTO_EGP) {
    es_rtdel((struct sockaddr_in *)&rt->rt_dst);
    psp_egp_rtdel((struct sockaddr_in *)&rt->rt_dst, (struct sockaddr_in *)&rt->
rt_router);
    rt_igp = rt_locate((int) EXTERIOR, (struct sockaddr_in *) &rt->rt_dst, RTPRO
TO_IGP);
    if (rt_igp != NULL) {
      es_rtadd((struct sockaddr_in *) &rt_igp->rt_dst, rt_igp->rt_as);
    }
  }
#endif  NSS
  if (!(just_kernel)) {
    remque(rt);
    free((char *)rt);
    n_routes--;
#ifdef NSS
    rttable_changed++;
#endif NSS
  }
}

/*
 * rt_default() adds or deletes default route in kernel.
 * also adds a default in it's own tables only for the RIP/HELLO
 * gateway.
 */

rt_default(cmd)
	char *cmd;
{
  struct rt_entry *rt;
  struct sockaddr_in	defaultdst;
  int keepinstall, changed = FALSE;

  if (!rip_gateway && !hello_gateway) {
    return(changed);
  }

  bzero((char *)&defaultdst, sizeof(defaultdst));
  defaultdst.sin_family = AF_INET;
  defaultdst.sin_addr.s_addr = DEFAULTNET;

  if (strcasecmp( cmd, "ADD") == 0) {
    if (rt_default_active == FALSE) {
      keepinstall = install;
      install = TRUE;
      rt = rt_lookup((int)EXTERIOR, &defaultdst);
      if (rt) {
        rt_delete(rt, KERNEL_INTR);	/* delete old default route */
      }
      rt = rt_lookup((int)INTERIOR, &defaultdst);
      if (rt) {
        rt_delete(rt, KERNEL_INTR);	/* delete old default route */
      }
      /*
       * this is only a false route, so just pick the first interface address
       * in the ifnet list to satisfy the gateway.
       */
      install = FALSE;
      (void) rt_add((int)INTERIOR, (struct sockaddr *)&defaultdst,
              &ifnet->int_addr, 0, RTS_PASSIVE|RTS_NOTINSTALL|RTS_STATIC, RTPROTO_DEFAULT, RTPROTO_DEFAULT, 0, 0);
      install = keepinstall;
      rt_default_active = TRUE;
      changed = TRUE;
    }
  } else {
    if (rt_default_active == TRUE) {
      rt = rt_lookup((int)INTERIOR, &defaultdst);
      if (rt != NULL) {
        rt_delete(rt, KERNEL_INTR);
        rt_default_active = FALSE;
      } else {
        syslog(LOG_NOTICE, MSGSTR(RT_TABLE_21,"rt_default: no active default route\n"));
      }
      if (default_gateway) {
        (void) rt_add(default_gateway->rt_state & RTS_INTERIOR ? (int)INTERIOR : (int)EXTERIOR,
                     &default_gateway->rt_dst, &default_gateway->rt_router,
                     default_gateway->rt_metric, default_gateway->rt_state,
                     default_gateway->rt_proto, default_gateway->rt_fromproto,
                     default_gateway->rt_as, default_gateway->rt_metric_exterior);
      }
      changed = TRUE;
    }
  }
  return (changed);
}

/*
 * rt_time() increments the age of all routes in the routing table
 * If any EGP routes are older than rt_maxage (set in egpstime()) the routes are
 * deleted. If RIP/HELLO routes are greater that GARBAGE_TIME, delete them.
 */

rt_time()
{
  struct rthash	*rh;
  struct rthash	*base;
  struct rt_entry *rt;
  int old_routes = 0, hold_routes = 0;
  int doinghost;
#ifndef NSS
  int timetobroadcast, hellotimetobroad;
  int do_flash_update = FALSE;
  int do_helloflash_update = FALSE;
  char *s;
  struct bits *p;

  timetobroadcast = rip_supplier && ((gatedtime % RIP_INTERVAL) == 0);
  hellotimetobroad = hello_supplier && ((gatedtime % HELLO_TIMERRATE) == 0);
#else   NSS
  int igp_insane_rt = 0;
#endif  NSS

  for (doinghost = 1, base = hosthash; doinghost >= 0; doinghost--, base=nethash) {
    for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
      for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
        if (rt->rt_state & RTS_CHANGED) {    /* recently updated */
          rt->rt_state &= ~RTS_CHANGED;
          rt->rt_timer = 0;
          if (((rt->rt_state & RTS_PASSIVE) == 0) &&
              (rt->rt_state & RTS_INTERIOR)) {
#ifndef NSS
            if (doing_rip) {
              if (rip_supplier && !timetobroadcast)
                do_flash_update = TRUE;
            }
            if (doing_hello) {
              if (hello_supplier && !hellotimetobroad)
                do_helloflash_update = TRUE;
            }
#endif NSS
          }
        } else {
          if (!(rt->rt_state & RTS_PASSIVE)) {
            rt->rt_timer += gatedtime - last_time;
          }
        }
        /*
         *  is route too old?
         */
        if (rt->rt_state & RTS_EXTERIOR) {
          /*
           *	Exterior Routes
           */
          switch (rt->rt_proto) {
#ifdef  NSS
            case RTPROTO_IGP:
              igp_insane_rt += igp_sanity_chk(rt);
              break;
#endif  NSS
            case RTPROTO_EGP:
            case RTPROTO_KERNEL:
            case RTPROTO_DEFAULT:
            case RTPROTO_REDIRECT:
              switch (rt_time_exterior(rt)) {
                case 1:
                  old_routes++;
                  break;
                case 2:
                  hold_routes++;
                  break;
              }
              break;
            default:
              if (sock_inaddr(&rt->rt_dst).s_addr != DEFAULTNET) {
                for (p = protobits; p->t_bits; p++) {
                  if ((rt->rt_proto & p->t_bits) == 0)
                    continue;
                    s = p->t_name;
                }
                TRACE_TRC(MSGSTR(RT_TABLE_22,"rt_time: Invalid exterior route to net %s via %s at %s"),
                          inet_ntoa(sock_inaddr(&rt->rt_dst)), s, strtime);
              }
              break;
          }
        } else {
          /*
           *	Interior Routes
           */
          switch (rt->rt_proto) {
#ifndef NSS
            case RTPROTO_RIP:
            case RTPROTO_HELLO:
#endif NSS
            case RTPROTO_DIRECT:
            case RTPROTO_KERNEL:
            case RTPROTO_REDIRECT:
            case RTPROTO_DEFAULT:
              switch (rt_time_interior(rt)) {
                case 1:
                  old_routes++;
                  break;
                case 2:
                  hold_routes++;
                  break;
              }
              break;
            default:
              if (sock_inaddr(&rt->rt_dst).s_addr != DEFAULTNET) {
		for (p = protobits; p->t_bits; p++) {
                  if ((rt->rt_proto & p->t_bits) == 0)
                    continue;
                    s = p->t_name;
                }
                TRACE_TRC(MSGSTR(RT_TABLE_23,"rt_time: Invalid interior route to net %s via protocol %s at %s"),
                        inet_ntoa(sock_inaddr(&rt->rt_dst)), s, strtime);
              }
              break;
          }
        }
      }
    }
  }
#ifndef NSS
  if (do_flash_update) {
    /* there was one or more changes, update RIP */
    toall(supply);
  }
  if (do_helloflash_update) {
    /* there was one or more changes, update HELLO */
    hellojob();
  }
#else   NSS
  if (old_routes) {
        egp2esrec();
  }
  if (igp_insane_rt > 0) {
    TRACE_RT("rt_time: above %d IGP routes are insane %s\n", igp_insane_rt, strt
ime);
  }
#endif  NSS


  if (old_routes+hold_routes) {
    (void) rt_check_default();
    TRACE_RT(MSGSTR(RT_TABLE_24,"rt_time: above %d routes deleted and %d routes helddown %s\n"), old_routes, hold_routes, strtime);
  }
  last_time = gatedtime;
  return;
}

/*
 * rt_time_exterior() checks the age, expires and deletes exterior
 * routes.
 */
rt_time_exterior(rt)
  struct rt_entry *rt;
{
  struct rt_entry *old_rt;
  int old_routes = 0;

  if ( (rt->rt_timer >= rt_maxage - HOLD_DOWN) && (rt->rt_metric < TDHOPCNT_INFINITY) ) {
    old_routes = rt_unreach(rt)*2;
  }
  if (rt->rt_timer >= rt_maxage) {
    old_rt = rt;
    rt = rt->rt_back;
    rt_delete(old_rt, KERNEL_INTR);
    old_routes = 1;
  }
  return(old_routes);
}


/*
 * rt_time_interior() checks the age, expires and deletes interior
 * routes.
 */
rt_time_interior(rt)
  struct rt_entry *rt;
{
  struct rt_entry *old_rt;
  u_int old_sin = sock_inaddr(&rt->rt_dst).s_addr;
  int old_routes = 0;

  if ( (rt->rt_timer >= EXPIRE_TIME) && (rt->rt_metric < DELAY_INFINITY) ) {
    old_routes = rt_unreach(rt)*2;
#ifndef NSS
    if (rt->rt_proto == RTPROTO_HELLO) {
      add_win(&rt->rt_hwindow, rt->rt_metric);
    }
#endif NSS
  }
  if (rt->rt_timer >= FINAL_DELETE) {
    if (rt->rt_state & (RTS_INTERFACE|RTS_REMOTE)) {
      TRACE_INT(MSGSTR(RT_TABLE_25,"interface timeout - deleting route to %s\n"),
           inet_ntoa(sock_inaddr(&rt->rt_dst)));
      syslog(LOG_ERR, MSGSTR(RT_TABLE_25,"interface timeout - deleting route to %s\n"),
           inet_ntoa(sock_inaddr(&rt->rt_dst)));
    }
    old_rt = rt;
    rt = rt->rt_back;
    rt_delete(old_rt, KERNEL_INTR);
    if ((old_sin == DEFAULTNET) && (default_gateway)) {
      (void) rt_add(default_gateway->rt_state & RTS_INTERIOR ? (int)INTERIOR : (int)EXTERIOR,
                   &default_gateway->rt_dst, &default_gateway->rt_router,
                   default_gateway->rt_metric, default_gateway->rt_state,
                   default_gateway->rt_proto, default_gateway->rt_fromproto,
                   default_gateway->rt_as, default_gateway->rt_metric_exterior);
    }
    old_routes = 1;
  }
  return(old_routes);
}


/*
 *
 * rt_unreach() does processing on a route that has been 
 * indicated as unreachable in a routing update.  The metric
 * is set to infinity and the timer is set so the route will expire
 * within HOLD_DOWN seconds.  A fudge factor is subtracted from the
 * time to make sure that the hold down lasts at least HOLD_DOWN.
 */
int rt_unreach(rt)
	struct rt_entry *rt;
{
#ifdef	NO_HOLD_DOWN
  rt->rt_timer = rt_maxage;
  return(0);
#else	NO_HOLD_DOWN
  TRACE_ACTION(HOLDDOWN, rt);
  if ( rt->rt_state & (RTS_INTERIOR|RTS_HOSTROUTE) ) {
    rt->rt_timer = EXPIRE_TIME;
    rt->rt_metric = DELAY_INFINITY;
  } else {
    rt->rt_metric = TDHOPCNT_INFINITY;
    rt->rt_timer = rt_maxage - HOLD_DOWN;
  }
  rt->rt_state &= ~RTS_CHANGED;
  rt->rt_timer -= gatedtime - last_time;
  return(1);
#endif	NO_HOLD_DOWN
}	


/*
 * rt_check_default() checks whether there are currently any EGP neighbors
 * that are both acquired and reachable and if not, stops the announcement
 * of default, if the gateway is a rip_gateway or hello_gateway.
 */

rt_check_default()
{
  int neigh_up = FALSE;
  struct egpngh	*ngp;
  static int save_neigh_up = -1;

  if (rt_default_active == FALSE) {
    return(0);
  }

  if ( !doing_egp || (!(rip_gateway) && !(hello_gateway)) ) {
    return(0);
  }
  for (ngp = egpngh; ngp != NULL; ngp = ngp->ng_next) {
    if ((ngp->ng_state == NGS_UP) && !(ngp->ng_reach & NG_DOWN) && !(ngp->ng_flags & NG_NOGENDEFAULT)) {
      neigh_up = TRUE;
      break;
    }
  }
  if (neigh_up != save_neigh_up) {
    if (neigh_up == FALSE) {
      (void) rt_default("DELETE");
    }
    save_neigh_up = neigh_up;
    return(1);
  } else {
    return(0);
  }
}

/*
 * rt_gwunreach() deletes all exterior routes from the routing table for a
 * specified gateway
 */

rt_gwunreach(gateway)
	struct in_addr	gateway;	/* internet address of gateway */
{
  int changes = 0;
  struct rthash	*rh;
  struct rt_entry *rt, *unreach_rt;

  for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++) {
    for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
      if ((sock_inaddr(&rt->rt_router).s_addr == gateway.s_addr) &&
          !(rt->rt_state & RTS_PASSIVE) && (rt->rt_state & RTS_EXTERIOR)) {
        unreach_rt = rt;
        rt = rt->rt_back;
        TRACE_RT(MSGSTR(RT_TABLE_28,"GW UNREACH: "));
        rt_delete(unreach_rt, KERNEL_INTR);
        changes++;
      }
    }
  }
  return(changes);
}


/*
 * rt_find() looks up a destination as the kernel would
 */

struct rt_entry *
rt_find(dst)
	struct sockaddr_in *dst;
{
  register struct rt_entry *rt;
  register struct rthash *rh;
  register unsigned hash;
  struct afhash h;
  int af = AF_INET;
  int doinghost = 1;
  int (*match)();

  (*afswitch[af].af_hash)((struct sockaddr_in *)dst, &h);
  hash = h.afh_hosthash;
  rh = &hosthash[hash & ROUTEHASHMASK];
  match = afswitch[af].af_netmatch;

nexttable:
  for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
    if (rt->rt_hash != hash)
      continue;
    if (doinghost) {
      if (equal(&rt->rt_dst, dst))
        return (rt);
    }
    else {
      if ((*match)(&rt->rt_dst, dst))
        return(rt);
    }
  }
  if (doinghost) {
    doinghost = 0;
    hash = h.afh_nethash;
    rh = &nethash[hash & ROUTEHASHMASK];
    goto nexttable;
  }
  return (0);
}


/*
 * Looks up a destination network route with a specific protocol mask. 
 * Specifying a protocol of zero will match all protocols.
 */
 
struct rt_entry *rt_locate(table, dst, proto)
int table;
struct sockaddr_in *dst;
int	proto;
{
  struct rt_entry *rt;
  struct rthash *rh;
  unsigned hash;
  struct afhash h;

  if (dst->sin_family != AF_INET) {
    return(NULL);
  }
		
  (*afswitch[dst->sin_family].af_hash)(dst, &h);
  if (table == HOSTTABLE) {
    hash = h.afh_hosthash;
    rh = &hosthash[hash & ROUTEHASHMASK];
  } else {
    hash = h.afh_nethash;
    rh = &nethash[hash & ROUTEHASHMASK];
  }
  for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
    if (rt->rt_hash != hash) {
      continue;
    }
    if ( equal(&rt->rt_dst, dst) && (rt->rt_state & table) && (!proto || (rt->rt_proto & proto)) ) {
      return (rt);
    }
  }
  return (NULL);
}

#ifndef NSS

/*
 * initialize the sliding HELLO history window.
 */

init_hwindow(hwp, tdelay)
	struct hello_win *hwp;
	int tdelay;
{
  int msf = 0;

  while (msf < HWINSIZE)
    hwp->h_win[msf++] = DELAY_INFINITY;
  hwp->h_index = 0;
  hwp->h_min = tdelay;
  hwp->h_min_ttl = 0;
  hwp->h_win[0] = tdelay;
}

/*
 * add a HELLO derived time delay to the route entries HELLO window.
 */

add_win(hwp, tdelay)
	struct hello_win *hwp;
	int tdelay;
{
  int msf, t_index = 0;

  hwp->h_index++;
  if (hwp->h_index >= HWINSIZE)
    hwp->h_index = 0;
  hwp->h_win[hwp->h_index] = tdelay;
  if (tdelay > hwp->h_min)
    hwp->h_min_ttl++;
  else {
    hwp->h_min = tdelay;
    hwp->h_min_ttl = 0;
  }
  if (hwp->h_min_ttl >= HWINSIZE) {
    hwp->h_min = DELAY_INFINITY;
    for (msf = 0; msf < HWINSIZE; msf++)
      if (hwp->h_win[msf] <= hwp->h_min) {
        hwp->h_min = hwp->h_win[msf];
        t_index = msf;
      }
    hwp->h_min_ttl = 0;
    if (t_index < hwp->h_index)
      hwp->h_min_ttl = hwp->h_index - t_index;
    else
      if (t_index > hwp->h_index)
        hwp->h_min_ttl = HWINSIZE - (t_index - hwp->h_index);
  }
}

/*
 * map HELLO time delay to RIP metric  OR
 * RIP metric to HELLO time delay (in miliseconds)
 */

mapmetric(proto, cost)
	int proto;
	u_short cost;
{
  static unsigned short metric_delay[RIPHOPCNT_INFINITY+1] = HOP_TO_DELAY;
  u_short metric;

  switch (proto) {
    case HELLO_TO_RIP:
      for (metric = 0; metric < RIPHOPCNT_INFINITY; metric++) {
        if (cost <= metric_delay[metric]) {
          break;
        }
      }
      return(metric);
    case RIP_TO_HELLO:
      if (cost > RIPHOPCNT_INFINITY) {
        cost = RIPHOPCNT_INFINITY;
      }
      return(metric_delay[cost]);
    case EGP_TO_HELLO:
      if (cost > HOPCNT_INFINITY) {
        cost = HOPCNT_INFINITY;
      }
      return(cost * ETOTD_CONV);
    case HELLO_TO_EGP:
      if (cost > TDHOPCNT_INFINITY) {
        cost = TDHOPCNT_INFINITY;
      }
      return(cost / ETOTD_CONV);
    default:
      return(DELAY_INFINITY);   /* just in case some screwup, better to return INFINITY */
  }
}

#endif NSS
