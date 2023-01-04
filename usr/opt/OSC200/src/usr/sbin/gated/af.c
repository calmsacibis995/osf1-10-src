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
static char	*sccsid = "@(#)$RCSfile: af.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:55:00 $";
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
 * COMPONENT_NAME: TCPIP af.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_AF,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/*
 *  modified and extracted from 4.3BSD routed sources
 */

#include "include.h"

/*
 * Address family support routines
 */

int	inet_hash(), inet_netmatch(), inet_output(),
	inet_portmatch(), inet_portcheck(),
	inet_checkhost(), inet_ishost(), inet_canon();
char	*inet_format();
#define NIL	{ 0 }
#define	INET \
	{ inet_hash,		inet_netmatch,		inet_output, \
	  inet_portmatch,	inet_portcheck,		inet_checkhost, \
	  inet_ishost,		inet_canon, 		inet_format }

struct afswitch afswitch[AF_MAX] =
	{ NIL, NIL, INET, };

/*
 * hash routine for the route table.
 */

inet_hash(sin, hp)
	register struct sockaddr_in *sin;
	struct afhash *hp;
{
  register u_int n;

  n = gd_inet_netof(sin->sin_addr);
  if (n)
    while ((n & 0xff) == 0)
      n >>= 8;
  hp->afh_nethash = n;
  hp->afh_hosthash = ntohl(sin->sin_addr.s_addr);
  hp->afh_hosthash &= 0x7fffffff;
}

inet_netmatch(sin1, sin2)
	struct sockaddr_in *sin1, *sin2;
{
  return(gd_inet_netof(sin1->sin_addr) == gd_inet_netof(sin2->sin_addr));
}

/*
 * Verify the message is from the right port.
 */

inet_portmatch(sin)
	register struct sockaddr_in *sin;
{
#ifndef NSS
  return(sin->sin_port == ripport);
#endif /* NSS */
}

/*
 * Verify the message is from a "trusted" port.
 */
inet_portcheck(sin)
	struct sockaddr_in *sin;
{
  return (ntohs(sin->sin_port) <= IPPORT_RESERVED);
}

/*
 * Internet output routine.
 */

inet_output(sr, flags, sin, size)
	int sr, flags;
	struct sockaddr_in *sin;
	int size;
{
#ifndef NSS
  struct sockaddr_in dst;

  dst = *sin;
  sin = &dst;
  if (sin->sin_port == 0)
    sin->sin_port = ripport;
  if (sendto(sr, rip_packet, size, flags, (struct sockaddr *)sin, sizeof (*sin)) < 0) {
    (void) sprintf(err_message, MSGSTR(AF_1,"inet_output: sendto() error sending %d bytes to %s"),
      size, inet_ntoa(dst.sin_addr));
    p_error(err_message);
  }
#else
  TRACE_TRC("inet_output called at %s\n", strtime);
  abort();
#endif  /* NSS */
}

/*
 * Return 1 if the address is believed
 * for an Internet host -- THIS IS A KLUDGE.
 */

inet_checkhost(sin)
	struct sockaddr_in *sin;
{
  u_int i = ntohl(sin->sin_addr.s_addr);

#ifndef IN_BADCLASS
#define	IN_BADCLASS(i)	(((u_int) (i) & 0xe0000000) == 0xe0000000)
#endif	IN_BADCLASS

  if (IN_BADCLASS(i) || sin->sin_port != 0)
    return (0);
  if (i != 0 && (i & 0xff000000) == 0)
    return (0);
  for (i = 0; i < sizeof(sin->sin_zero)/sizeof(sin->sin_zero[0]); i++)
    if (sin->sin_zero[i])
      return (0);
  return (1);
}

/*
 * Return 1 if the address is
 * for an Internet host, 0 for a network.
 */

inet_ishost(sin)
	struct sockaddr_in *sin;
{
  return(gd_inet_lnaof(sin->sin_addr) != 0);
}

inet_canon(sin)
	struct sockaddr_in *sin;
{
  sin->sin_port = 0;
}

char *
inet_format(sin)
	struct sockaddr_in *sin;
{
  char *inet_ntoa();

  return(inet_ntoa(sin->sin_addr));
}
