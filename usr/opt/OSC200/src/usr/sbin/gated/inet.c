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
static char	*sccsid = "@(#)$RCSfile: inet.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:57:14 $";
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
 * COMPONENT_NAME: TCPIP inet.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_INET,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/*
 * these routines were modified from the 4.3BSD routed source.
 *
 * Temporarily, copy these routines from the kernel,
 * as we need to know about subnets.
 */
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#ifndef vax11c
#include <sys/uio.h>
#endif  vax11c
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef vax11c
#include "config.h"
#include <errno.h>
#endif vax11c
#include "if.h"
#include "defs.h"

extern struct interface *ifnet;
u_int gd_inet_wholenetof();

/*
 * Formulate an Internet address from network + host.
 */

struct in_addr gd_inet_makeaddr(net, host, subnetsAllowed)
	u_int net;
	int host, subnetsAllowed;
{
  register struct interface *ifp;
  register u_int mask;
  struct in_addr addr;

  addr.s_addr = NULL;
  if (IN_CLASSA(net)) {
    mask = IN_CLASSA_HOST;
  } else if (IN_CLASSB(net)) {
    mask = IN_CLASSB_HOST;
  } else if (IN_CLASSC(net)) {
    mask = IN_CLASSC_HOST;
  } else {
    return(addr);
  }

  if (subnetsAllowed) {
    for (ifp = ifnet; ifp; ifp = ifp->int_next) {
      if ((ifp->int_netmask & net) == ifp->int_net) {
        mask = ~ifp->int_subnetmask;
        break;
      }
    }
  }

  addr.s_addr = net | (host & mask);
  addr.s_addr = htonl(addr.s_addr);
  return(addr);
}

/*
 * Return the network number from an internet address.
 */

u_int gd_inet_netof(in)
	struct in_addr in;
{
  register u_int i = ntohl(in.s_addr);
  register u_int net;
  register struct interface *ifp;

  net = gd_inet_wholenetof(in);
  /*
   * Check whether network is a subnet;
   * if so, return subnet number.
   */
  for (ifp = ifnet; ifp; ifp = ifp->int_next)
    if ((ifp->int_netmask & net) == ifp->int_net)
      return (i & ifp->int_subnetmask);
  return (net);
}

/*
 * Return the network number from an internet address.
 * unsubnetted version.
 */

u_int gd_inet_wholenetof(in)
	struct in_addr in;
{
  register u_int i = ntohl(in.s_addr);
  register u_int net;

  if (IN_CLASSA(i)) {
    net = i & IN_CLASSA_NET;
  } else if (IN_CLASSB(i)) {
    net = i & IN_CLASSB_NET;
  } else if (IN_CLASSC(i)) {
    net = i & IN_CLASSC_NET;
  } else {
    return(NULL);
  }
  return (net);
}

/*
 * Return the host portion of an internet address.
 */

u_int gd_inet_lnaof(in)
	struct in_addr in;
{
  register u_int i = ntohl(in.s_addr);
  register u_int net, host;
  register struct interface *ifp;

  if (IN_CLASSA(i)) {
    net = i & IN_CLASSA_NET;
    host = i & IN_CLASSA_HOST;
  } else if (IN_CLASSB(i)) {
      net = i & IN_CLASSB_NET;
      host = i & IN_CLASSB_HOST;
  } else if (IN_CLASSC(i)) {
      net = i & IN_CLASSC_NET;
      host = i & IN_CLASSC_HOST;
  } else {
    return(NULL);
  }
  /*
   * Check whether network is a subnet;
   * if so, use the modified interpretation of `host'.
   */
  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    if ((ifp->int_netmask & net) == ifp->int_net) {
      return(host &~ ifp->int_subnetmask);
    }
  }

  return (host);
}

/*
 * Internet network address interpretation routine.
 * The library routines call this routine to interpret
 * network numbers.
 */

u_int gd_inet_isnetwork(cp)
	register char *cp;
{
  register u_int val, base, n;
  register char c;
  u_int parts[4], *pp = parts;
  register int i;

  bzero((char *)parts, sizeof(parts));
again:
  val = 0; base = 10;
  if (*cp == '0')
    base = 8, cp++;
  if (*cp == 'x' || *cp == 'X')
    base = 16, cp++;
  while (c = *cp) {
    if (isdigit(c)) {
      val = (val * base) + (c - '0');
      cp++;
      continue;
    }
    if (base == 16 && isxdigit(c)) {
      val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
      cp++;
      continue;
    }
    break;
  }
  if (*cp == '.') {
    if (pp >= parts + 4)
      return (-1);
    *pp++ = val, cp++;
    goto again;
  }
  if (*cp && !isspace(*cp))
    return (-1);
  *pp++ = val;
  n = pp - parts;
  if (n > 4)
    return (-1);
  for (val = 0, i = 0; i < n; i++) {
#ifdef	notdef
    if ((parts[i] & 0xff) == 0)
      continue;
#endif	notdef
    val <<= 8;
    val |= parts[i] & 0xff;
  }
  return (val);
}


/*
 *	Return the class of the network or zero in not valid
 */

gd_inet_class(net)
u_char *net;
{
	if ( in_isa(*net) ) {
		return CLAA;
	} else if ( in_isb(*net) ) {
		return CLAB;
	} else if ( in_isc(*net) ) {
		return CLAC;
	} else {
		return 0;
	}
}


char *gd_inet_ntoa(addr)
u_int	addr;
{
	struct in_addr in;

	in.s_addr = addr;

	return(inet_ntoa(in));
}
