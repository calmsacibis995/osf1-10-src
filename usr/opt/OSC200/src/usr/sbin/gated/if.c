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
static char	*sccsid = "@(#)$RCSfile: if.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:56:44 $";
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
 * COMPONENT_NAME: TCPIP if.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_IF,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/*
 * if.c
 *
 * Functions: if_withdst, if_check, if_print
 */

#include "include.h"

/* Find the interface on the network of the specified address.  On a   */
 /* point-to-point interface only the destination address is compared.  */
 /* On all other interfaces, the network/subnet is compared against the */
 /* network/subnet of the interface address.				*/

struct interface *
if_withdst(dstaddr)
struct sockaddr_in *dstaddr;
{
  register struct interface *ifp;

  if (dstaddr->sin_family != AF_INET)
    return (0);

  /* Scan the interface list.  For P2P interfaces look for an exact     */
  /* match of the specified address and the destination of this link.   */
  /* For other types of interfaces search for interfaces with the same  */
  /* (whole)  netmask.  On these interfaces, compare the specified       */
  /* address with the interface address under the subnetmask.           */

  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if (ifp->int_flags & IFF_POINTOPOINT) {
      if (dstaddr->sin_addr.s_addr == in_addr_ofs(&ifp->int_dstaddr).s_addr) {
        break;
      }
    } else if ( !(ntohl(dstaddr->sin_addr.s_addr ^ in_addr_ofs(&ifp->int_addr).s_addr) & ifp->int_subnetmask) ) {
      break;
    }
  }

  return(ifp);
}

#ifdef  notdef
/*
 *      Find the interface on the network of the specified address.  On a
 *      point-to-point interface only the destination address is compared.
 */

struct interface *
if_withdst(dstaddr)
struct sockaddr_in *dstaddr;
{
  register struct interface *ifp;
  register u_int net1, tmp, net2;

  if (dstaddr->sin_family != AF_INET)
    return (0);

  /* get network part of dstaddr */
  tmp = ntohl(dstaddr->sin_addr.s_addr);
  net1 = gd_inet_wholenetof(dstaddr->sin_addr);
  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if (ifp->int_flags & IFF_POINTOPOINT) {
      if (dstaddr->sin_addr.s_addr == in_addr_ofs(&ifp->int_dstaddr).s_addr) {
        return(ifp);
      }
     } else if ((ifp->int_netmask & net1) == ifp->int_net) {
      net1 = tmp & ifp->int_subnetmask;
      break;
    }
  }
  /* search for ifp */
  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if (!(ifp->int_flags & IFF_POINTOPOINT)) {
      tmp = ntohl(in_addr_ofs(&ifp->int_addr).s_addr);
      net2 = gd_inet_wholenetof(in_addr_ofs(&ifp->int_addr));
      if ((ifp->int_netmask & net2) == ifp->int_net) {
        net2 = tmp & ifp->int_subnetmask;
      }
      if (net1 == net2) {
    	break;
      }
   }
 }
 return(ifp);
}

#endif notdef


#ifdef notdef
/* used for DEBUGing */
if_print()
{
  register struct interface *ifp;

  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if_display("if_print", ifp);
  }
}
#endif	notdef

#ifndef NSS

/*
 * if_check() checks the current status of all interfaces
 * If any interface has changed status, then the interface values
 * are re-read from the kernel and re-set.
 */

if_check()
{
  register struct interface *ifp;
  struct ifreq ifrequest;
  int  if_change = FALSE;
  struct sockaddr_in *sin;
  u_int a;
  int info_sock;

  if ((info_sock = getsocket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    p_error(MSGSTR(IF_2,"if_check: no info socket "));
    return;
  }
  for (ifp = ifnet; ifp != NULL; ifp = ifp->int_next) {
    /* get interface status flags */
    (void) strcpy(ifrequest.ifr_name, ifp->int_name);
    if (ioctl(info_sock, SIOCGIFFLAGS, (char *)&ifrequest)) {
      (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFFLAGS:", ifp->int_name);
      p_error(err_message);
    } else {
      if ((ifrequest.ifr_flags & IFF_UP) != (ifp->int_flags & IFF_UP)) {
        if_change = TRUE;
        if (ifrequest.ifr_flags & IFF_UP) {
          ifp->int_flags = IFF_INTERFACE |
                           (ifrequest.ifr_flags & IFF_MASK) |
                           (ifp->int_flags & IFF_KEEPMASK);
#if     defined(SIOCGIFMETRIC)
          (void) strcpy(ifrequest.ifr_name, ifp->int_name);
          if (ioctl(info_sock, SIOCGIFMETRIC, (char *)&ifrequest) < 0) {
            (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFMETRIC:", ifp->int_name);
            p_error(err_message);
          } else {
            ifp->int_metric = (ifrequest.ifr_metric >= 0) ?
                                 ifrequest.ifr_metric : 0;
          }
#else   defined(SIOCGIFMETRIC)
          ifp->int_metric =  0;
#endif  defined(SIOCGIFMETRIC)
          if (ifp->int_flags & IFF_POINTOPOINT) {
            (void) strcpy(ifrequest.ifr_name, ifp->int_name);
            if (ioctl(info_sock, SIOCGIFDSTADDR, (char *)&ifrequest) < 0) {
              (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFDSTADDR:", ifp->int_name);
              p_error(err_message);
            } else {
              ifp->int_dstaddr = ifrequest.ifr_dstaddr;
            }
          }
          (void) strcpy(ifrequest.ifr_name, ifp->int_name);
          if (ioctl(info_sock, SIOCGIFADDR, (char *)&ifrequest) < 0) {
            (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFADDR:",ifp->int_name);
            p_error(err_message);
          } else {
            ifp->int_addr = ifrequest.ifr_addr;
          }
          if (ifp->int_flags & IFF_BROADCAST) {
#ifdef SIOCGIFBRDADDR
            (void) strcpy(ifrequest.ifr_name, ifp->int_name);
            if (ioctl(info_sock, SIOCGIFBRDADDR, (char *)&ifrequest) < 0) {
              (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFBRDADDR:", ifp->int_name);
              p_error(err_message);
            } else {
              ifp->int_broadaddr = ifrequest.ifr_broadaddr;
            }
#else !SIOCGIFBRDADDR
            ifp->int_broadaddr = ifp->int_addr;
            sin = (struct sockaddr_in *)&ifp->int_addr;
            a = ntohl(sin->sin_addr.s_addr);
            sin = (struct sockaddr_in *)&ifp->int_broadaddr;
            if (IN_CLASSA(a))
		 sin->sin_addr.s_addr = htonl(a & IN_CLASSA_NET);
            else if (IN_CLASSB(a))
              sin->sin_addr.s_addr = htonl(a & IN_CLASSB_NET);
            else
              sin->sin_addr.s_addr = htonl(a & IN_CLASSC_NET);
#endif SIOCGIFBRDADDR
          }
#ifdef  SIOCGIFNETMASK
          (void) strcpy(ifrequest.ifr_name, ifp->int_name);
          if (ioctl(info_sock, SIOCGIFNETMASK, (char *)&ifrequest) < 0) {
            (void) sprintf(err_message,"if_check: %s: ioctl SIOCGIFNETMASK:", ifp->int_name);
            p_error(err_message);
            ifp->int_subnetmask = (u_int) 0;
          } else {
            sin = (struct sockaddr_in *)&ifrequest.ifr_addr;
            ifp->int_subnetmask = ntohl(sin->sin_addr.s_addr);
          }
#else   SIOCGIFNETMASK
          sin = (struct sockaddr_in *)&ifp->int_addr;
          a = ntohl(sin->sin_addr.s_addr);
          if (IN_CLASSA(a)) {

		ifp->int_subnetmask = IN_CLASSB_NET;
         } else {
            ifp->int_subnetmask = IN_CLASSC_NET;
         }
#endif  SIOCGIFNETMASK
	  sin = (struct sockaddr_in *)&ifp->int_addr;
          a = ntohl(sin->sin_addr.s_addr);
          if (IN_CLASSA(a)) {
            ifp->int_netmask = IN_CLASSA_NET;
          } else if (IN_CLASSB(a)) {
            ifp->int_netmask = IN_CLASSB_NET;
          } else {
            ifp->int_netmask = IN_CLASSC_NET;
          }
          if (ifp->int_subnetmask == 0) {
            ifp->int_subnetmask = ifp->int_netmask;
          } else if (ifp->int_subnetmask != ifp->int_netmask) {
            ifp->int_flags |= IFF_SUBNET;
          }
          ifp->int_net = a & ifp->int_netmask;
          ifp->int_subnet = a & ifp->int_subnetmask;
          syslog(LOG_NOTICE, MSGSTR(IF_9,"if_check: %s, address %s up"),
                             ifp->int_name, inet_ntoa(sock_inaddr(&ifp->int_addr)));
          TRACE_INT(MSGSTR(IF_10,"if_check: %s, address %s up at %s"),
                             ifp->int_name, inet_ntoa(sock_inaddr(&ifp->int_addr)), strtime);
          if_display("if_check", ifp);
          rt_ifup(ifp);
        } else {
          syslog(LOG_NOTICE, MSGSTR(IF_12,"if_check: %s, address %s down"),
                             ifp->int_name, inet_ntoa(sock_inaddr(&ifp->int_addr)));
          TRACE_INT(MSGSTR(IF_13,"if_check: %s, address %s down at %s"),
                             ifp->int_name, inet_ntoa(sock_inaddr(&ifp->int_addr)), strtime);
	  /* Note: IFF_MASK has been extended to include IFF_NOECHO */
          ifp->int_flags = IFF_INTERFACE |
                           (ifrequest.ifr_flags & IFF_MASK) |
                           (ifp->int_flags & IFF_KEEPMASK);
          rt_ifdown(ifp, FALSE);
        }
      }
    }
  }
  if (if_change) {
    register struct rt_entry *rt;
    register struct rthash *rh;

    for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++) 
      for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
        if ((rt->rt_state & RTS_INTERIOR) == 0)
          continue;
        if (rt->rt_ifp->int_flags & IFF_UP)
          rt->rt_flags |= RTF_UP;
        else
          rt->rt_flags &= ~RTF_UP;
      }
  }
  (void) close(info_sock);
}


/*
 *	if_display():
 *		Log the configuration of the interface
 */
if_display(name, ifp)
  char *name;
  struct interface *ifp;
{
  TRACE_INT(MSGSTR(IF_14,"%s: interface %s: %s, addr %s, metric %d"),
    name, ifp->int_name, (ifp->int_flags & IFF_UP) ? MSGSTR(IF_15,"up") : MSGSTR(IF_16,"down"), inet_ntoa(sock_inaddr(&ifp->int_addr)), ifp->int_metric);
  if (ifp->int_flags & IFF_BROADCAST) {
    TRACE_INT(MSGSTR(IF_17,", broadaddr %s, "), inet_ntoa(sock_inaddr(&ifp->int_broadaddr)));
  }
  if (ifp->int_flags & IFF_POINTOPOINT) {
    TRACE_INT(MSGSTR(IF_18,", dstaddr %s, "), inet_ntoa(sock_inaddr(&ifp->int_dstaddr)));
  }
  TRACE_INT(MSGSTR(IF_19,"\n%s: interface %s: "), name, ifp->int_name);
  TRACE_INT(MSGSTR(IF_20,"net %s, "), gd_inet_ntoa(htonl(ifp->int_net)));
  TRACE_INT(MSGSTR(IF_21,"netmask %s, "), gd_inet_ntoa(htonl(ifp->int_netmask)));
  TRACE_INT(MSGSTR(IF_22,"\n%s: interface %s: "), name, ifp->int_name);
  TRACE_INT(MSGSTR(IF_23,"subnet %s, "), gd_inet_ntoa(htonl(ifp->int_subnet)));
  TRACE_INT(MSGSTR(IF_24,"subnetmask %s\n"), gd_inet_ntoa(htonl(ifp->int_subnetmask)));
}

#endif NSS

/*
 * Find the interface with address addr.
 */

struct interface *
if_ifwithaddr(withaddraddr)
	struct sockaddr *withaddraddr;
{
  register struct interface *ifp;
  struct sockaddr_in *addr = (struct sockaddr_in *) withaddraddr;
  struct sockaddr_in *intf_addr;

  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if (ifp->int_flags & IFF_REMOTE) {
      continue;
    }
    if (ifp->int_addr.sa_family != withaddraddr->sa_family) {
      continue;
    }
    if (ifp->int_flags & IFF_POINTOPOINT) {
      intf_addr = (struct sockaddr_in *)&ifp->int_dstaddr;
      if (!bcmp((char *)&intf_addr->sin_addr, (char *)&addr->sin_addr, sizeof(struct in_addr))) {
        break;
      } else {
        continue;
      }
    }
    intf_addr = (struct sockaddr_in *)&ifp->int_addr;
    if (!bcmp((char *)&intf_addr->sin_addr, (char *)&addr->sin_addr, sizeof(struct in_addr))) {
      break;
    }
    intf_addr = (struct sockaddr_in *)&ifp->int_broadaddr;
    if (ifp->int_flags & IFF_BROADCAST) {
    }
    if ((ifp->int_flags & IFF_BROADCAST) &&
      !bcmp((char *)&intf_addr->sin_addr, (char *)&addr->sin_addr, sizeof(struct in_addr)))
      break;
  }
  return (ifp);
}

#ifndef NSS
/*
 * update the active gw list on argument interface.
 */

if_updateactivegw(ifptr, actgw_addr, gw_proto)
	struct interface *ifptr;
	u_int actgw_addr;
	int gw_proto;
{
  struct active_gw *agp, *tmpactgw;
  int found_gw = 0;

  for (agp = ifptr->int_active_gw; agp; agp = agp->next) {
    if (actgw_addr == agp->addr) {
      found_gw++;
      break;
    }
    if (agp->next == NULL)
      break;
  }
  if (found_gw != 0) {
    agp->timer = 0;
    agp->proto |= gw_proto;
    return(agp->proto);
  }
  /*
   * this active gateway wasn't recorded yet!  Add it.
   * we have agp pointing to the last element, so no need to
   * traverse again.
   */
  tmpactgw = (struct active_gw *)malloc((unsigned)sizeof(struct active_gw));
  if (tmpactgw <= (struct active_gw *)0) {
    syslog(LOG_WARNING, MSGSTR(IF_25,"if_updateactivegw: out of memory"));
    return(0);
  }
  tmpactgw->proto = gw_proto;
  tmpactgw->addr = actgw_addr;
  tmpactgw->timer = 0;
  if (agp == NULL) {		/* first one */
    ifptr->int_active_gw = tmpactgw;
    tmpactgw->back = ifptr->int_active_gw;
  }
  else {
    agp->next = tmpactgw;
    tmpactgw->back = agp;
  }
  tmpactgw->next = NULL;
  tmpactgw = NULL;		/* just to be safe */
  return (gw_proto);
}
#endif NSS
