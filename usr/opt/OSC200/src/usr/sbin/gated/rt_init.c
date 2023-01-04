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
static char	*sccsid = "@(#)$RCSfile: rt_init.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:58:48 $";
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
 * COMPONENT_NAME: TCPIP rt_init.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_RT_INIT,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/* rt_init.c
 *
 * Initialization functions for routing tables - interface routes, reading 
 * kernel routes, non-routing gateways, and routes to be advised in EGP 
 * NR Update messages.
 *
 * Functions: rt_init, rt_readkernel, rt_ifinit, rt_dumbinit, rt_NRadvise_init
 */

#ifdef  vax11c
#include "config.h"
#endif  vax11c
#include <sys/param.h>
#include <sys/mbuf.h>                   /* needed by <net/route.h> */
#include <sys/time.h>
#ifdef  vax11c
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif  vax11c
#include <sys/ioctl.h>
#ifndef vax11c
#include <sys/uio.h>
#endif  vax11c
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <nlist.h>
#include <syslog.h>

#ifdef vax11c
#define DONT_INCLUDE_IF_ARP
#endif vax11c
#include <net/if.h>
#define _KERNEL 1	/* to ensure all <net/route.h> compiled */
#include <net/route.h>

#include "routed.h"
#include "defs.h"
#include "egp.h"
#include "egp_param.h"
#include "if.h"
#include "rt_table.h"
#include "rt_control.h"
#include "af.h"
#include "trace.h"
#ifndef NSS
#include "rip.h"
#include "hello.h"
#endif NSS

/*
 * rt_init() initializes exterior and interior routing tables doubly linked
 * lists. 
 */

extern off_t lseek();

rt_init()
{
  register struct rthash *rh;
  register struct restricthash *ac;

#ifdef lint
  bzero((char *)&rtstat, sizeof(rtstat));   /* make lint happy */
#endif lint

  for (rh = nethash; rh < &nethash[ROUTEHASHSIZ]; rh++)	
  rh->rt_forw = rh->rt_back = (struct rt_entry *)rh;
  for (rh = hosthash; rh < &hosthash[ROUTEHASHSIZ]; rh++)	
  rh->rt_forw = rh->rt_back = (struct rt_entry *)rh;
  for (ac = rt_restrict; ac < &rt_restrict[ROUTEHASHSIZ]; ac++)	
  ac->rt_forw = ac->rt_back = (struct restrictlist *)ac; 
}

#ifndef NSS

/*
 * rt_readkernel() initializes the GATED routing tables from the current kernel
 * routing tables through /dev/kmem.
 * This is necessary to ensure consistency when the GATED process is terminated
 * and restarted while the supporting host continues to run. This may be done
 * if configuration information needs to be changed.
 */

#define	NELEM_NLIST 3			
struct nlist nl[] = {
#define	N_RTHOST		0 
	{ "_rthost" },
#define N_RTNET			1
	{ "_rtnet"},		
#define N_RTHASHSIZE		2
	{ "_rthashsize"},
#define	N_VERSION		3
	{ "_version" },
	{ "" },
};


rt_readkernel()
{
  /*
   *  ..routed/table.h redefines some of the elements of 
   * struct rtentry defined in <net/route.h>. In order for the code from routed
   * to be able to be utilized part of the kernel structure has been redefined 
   * with a preceding k_.  
   */
  struct k_rtentry {
    u_int	k_rt_hash;		/* to speed lookups */
    struct	sockaddr k_rt_dst;	/* key */
    struct	sockaddr k_rt_gateway;	/* value */
    short	k_rt_flags;		/* up/down?, host/net */
  } *rt;

  int 	i, hashsize, kmem, doinghost, rtbufsize;
#if     defined(ULTRIX3_X)
  struct        rtentry *next, m_buf, **base;
#else   defined(ULTRIX3_X)
  struct        mbuf    *next, m_buf, **base;
#endif  defined(ULTRIX3_X)


   (void) nlist("/vmunix", nl);

  if ((nl[N_RTHOST].n_value == 0) || (nl[N_RTNET].n_value == 0)) {
    syslog(LOG_ERR, MSGSTR(RT_INIT_6,"rt_readkernel: rthost and rtnet not in namelist\n"));
    quit();
  }
  kmem = open("/dev/kmem", 0);
  if (kmem < 0) {
    p_error("rt_readkernel: /dev/kmem");
    quit();
  }
  if ( nl[N_VERSION].n_value ) {
    char *p;
    
    if ( (version_kernel = calloc(1, RIPPACKETSIZE)) == 0 ) {
      p_error("rt_readkernel: calloc:");
      quit();
    }
    (void) lseek(kmem, (off_t) nl[N_VERSION].n_value, 0);
    (void) read(kmem, version_kernel, RIPPACKETSIZE-1);
    if ( p = index(version_kernel, '\n') ) {
    	*p = NULL;
    }
    if ( (p = malloc((unsigned int)strlen(version_kernel))) == 0 ) {
      p_error("rt_readkernel: malloc:");
      quit();
    }
    (void) strcpy(p, version_kernel);
    free(version_kernel);
    version_kernel = p;
    TRACE_INT("\nrt_readkernel: _version = %s\n", version_kernel);
  } else {
    version_kernel = NULL;
  }
  if (nl[N_RTHASHSIZE].n_value != 0) {
    (void) lseek(kmem, (off_t) nl[N_RTHASHSIZE].n_value, 0);
    (void) read(kmem, (char *)&hashsize, sizeof(hashsize));
  } else {
    syslog(LOG_ERR, MSGSTR(RT_INIT_12,"rt_readkernel: rthashsize not in namelist\n"));
    quit();
  }
	
  /* set up to read table of net hash chains */

    rtbufsize = hashsize * sizeof(struct mbuf *);
    base = (struct mbuf **)malloc((unsigned int)rtbufsize);
  if (base == NULL) {
      syslog(LOG_ERR, MSGSTR(RT_INIT_13,"rt_readkernel: can't malloc %d bytes"), rtbufsize);
      quit();
  }

  if ((lseek(kmem, (off_t)nl[N_RTNET].n_value, 0) == -1) ||
       (read(kmem, (char *)base, rtbufsize) != rtbufsize)) {
    syslog(LOG_ERR, MSGSTR(RT_INIT_14,"rt_readkernel: error reading kmem\n"));
    quit();
  }
  TRACE_RT(MSGSTR(RT_INIT_15,"\nrt_readkernel: Initial routes read from kernel (if any):\n"));
  doinghost = 0;

doitagain:
  for (i = 0; i < hashsize; i++) {
    for (next = base[i]; next != NULL; next = m_buf.m_next) {
      if ((lseek(kmem, (off_t)next, 0) == -1) ||
/*** In OSF/1 mbuf structure contains the mbuf header ***/
          (read(kmem, (char *)&m_buf,  sizeof(struct mbuf)) != (sizeof (struct mbuf)))) {
        quit();
      }
      rt = mtod(&m_buf, struct k_rtentry *);
#ifdef	why_do_it_this_way
      if (!(rt->k_rt_flags & RTF_GATEWAY))
        continue;
#else	why_do_it_this_way
      if ( ((struct sockaddr_in *)&rt->k_rt_dst)->sin_addr.s_addr == htonl(LOOPBACKNET) ) {
      	continue;
      }
#endif	why_do_it_this_way
      if (rt->k_rt_gateway.sa_family != AF_INET)
        continue;
      install = FALSE;  /* don't install routes in kernel */
      {
        register struct interface *ifp;
        int interior =  0, saveinstall, addgood = 0;
        struct rt_entry dup_rt;
        struct rt_entry *foundit;
			
        if (doinghost != 0) {
          if (!(foundit = rt_lookup((int)HOSTTABLE, (struct sockaddr_in *)&rt->k_rt_dst))) {
            addgood = (int) rt_add((int)HOSTTABLE, &rt->k_rt_dst, &rt->k_rt_gateway,
                      mapmetric(RIP_TO_HELLO, (u_short)(RIPHOPCNT_INFINITY - 1)), 0, RTPROTO_KERNEL,
                      RTPROTO_KERNEL, 0, 0);
          }
          if (foundit || !(addgood)) {
            bzero((char *)&dup_rt, sizeof(struct rt_entry));
            dup_rt.rt_dst = rt->k_rt_dst;
            dup_rt.rt_router = rt->k_rt_gateway;
            dup_rt.rt_flags = rt->k_rt_flags;
            saveinstall = install;
            install = TRUE;
            rt_delete(&dup_rt, KERNEL_ONLY);
            install = saveinstall;
          }
        } else {
          /*
           *	Check for and ignore martian nets
           */
          if ( is_martian(((struct sockaddr_in *)&rt->k_rt_dst)->sin_addr) ) {
            TRACE_EXT(MSGSTR(RT_INIT_16,"rt_readkernel: ignoring invalid net %s at %s"),
              inet_ntoa(((struct sockaddr_in *)&rt->k_rt_dst)->sin_addr), strtime);
            bzero((char *)&dup_rt, sizeof(struct rt_entry));
            dup_rt.rt_dst = rt->k_rt_dst;
            dup_rt.rt_router = rt->k_rt_gateway;
            dup_rt.rt_flags = rt->k_rt_flags;
            dup_rt.rt_proto = RTPROTO_KERNEL;
            dup_rt.rt_fromproto = RTPROTO_KERNEL;
            dup_rt.rt_state = RTS_EXTERIOR;
            saveinstall = install;
            install = TRUE;
            rt_delete(&dup_rt, KERNEL_ONLY);
            install = saveinstall;
            continue;
          }
          for (ifp = ifnet; ifp; ifp = ifp->int_next)
            if ( gd_inet_wholenetof(((struct sockaddr_in *)&rt->k_rt_dst)->sin_addr)
                == gd_inet_wholenetof(in_addr_ofs(&ifp->int_addr)) ) {
              interior++;
              break;
            }
          if (interior) {
            if (!(foundit = rt_lookup((int)INTERIOR, (struct sockaddr_in *)&rt->k_rt_dst))) {
              addgood = (int) rt_add((int)INTERIOR,
                                &rt->k_rt_dst,
                                &rt->k_rt_gateway,
                                mapmetric(RIP_TO_HELLO, (u_short)(RIPHOPCNT_INFINITY - 1)),
                                0,
                                RTPROTO_KERNEL, RTPROTO_KERNEL, 0, 0);
            }
            if (foundit || !(addgood)) {
              bzero((char *)&dup_rt, sizeof(struct rt_entry));
              dup_rt.rt_dst = rt->k_rt_dst;
	      dup_rt.rt_router = rt->k_rt_gateway;
              dup_rt.rt_flags = rt->k_rt_flags;
              saveinstall = install;
              install = TRUE;
              rt_delete(&dup_rt, KERNEL_ONLY);
              install = saveinstall;
            }
          } else {
            if (!(foundit = rt_lookup((int)EXTERIOR, (struct sockaddr_in *)&rt->k_rt_dst))) {
              addgood = (int) rt_add((int)EXTERIOR, &rt->k_rt_dst, &rt->k_rt_gateway,
                         mapmetric(EGP_TO_HELLO, (u_short)(HOPCNT_INFINITY - 1)), 0, RTPROTO_KERNEL,
                         RTPROTO_KERNEL, 0, 0);
            }
            if (foundit || !(addgood)) {
              bzero((char *)&dup_rt, sizeof(struct rt_entry));
              dup_rt.rt_dst = rt->k_rt_dst;
              dup_rt.rt_router = rt->k_rt_gateway;
              dup_rt.rt_flags = rt->k_rt_flags;
              saveinstall = install;
              install = TRUE;
              rt_delete(&dup_rt, KERNEL_ONLY);
              install = saveinstall;
            }
          }
        }
      }
    }
  }
  /* set up to read the host hash chain. */
  if (doinghost == 0) {
    (void) lseek(kmem, (off_t)0 ,0);
    if ((lseek(kmem, (off_t)nl[N_RTHOST].n_value, 0) == -1) ||
        (read(kmem, (char *)base, rtbufsize) != rtbufsize)) {
      syslog(LOG_ERR, MSGSTR(RT_INIT_17,"rt_readkernel: error reading kmem\n"));
      quit();
    }
    doinghost++;
    goto doitagain;
  }
  (void) close(kmem);
  free((char *)base);
  return;
}

/*
 * rt_dumbinit() reads the initialization file EGPINITFILE to
 * initialize: 
 * - the interior route table with routes to known non-routing gateways on a
 * shared net, these routes are static and not updated;
 * - a default gateway prior to another default being heard (if active default);
 * It installs them in the kernel if they were not previously read from the
 * kernel.
 */

rt_dumbinit(fp)
	FILE *fp;
{
  char keyword[MAXHOSTNAMELENGTH+1];
  char netname[MAXHOSTNAMELENGTH+1];
  char gname[MAXHOSTNAMELENGTH+1];
  char proto_type[MAXHOSTNAMELENGTH+1];
  char deftype[MAXHOSTNAMELENGTH+1];
  char buf[BUFSIZ];
  struct 	sockaddr_in	netaddr, gateway, defaultdst;
  struct	rt_entry *rt;
  int	metric, error = FALSE, old_install, line, ptype = 0;
  int   tableadd;

  bzero((char *)&netaddr, sizeof(netaddr));
  bzero((char *)&gateway, sizeof(gateway));
  bzero((char *)&defaultdst, sizeof(defaultdst));

  TRACE_RT(MSGSTR(RT_INIT_18,"rt_dumbinit: non-routing gateway routes (if any):\n"));

  rewind(fp);
  line = 0;

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    line++;
    if ((buf[0] == '#') || (buf[0] == '\n')) {
      continue;
    }
    if (sscanf(buf, "%s", keyword) != 1) {
      continue;
    }
    if ((strcasecmp(keyword, "net") == 0) ||
        (strcasecmp(keyword, "host") == 0)) {
      if (sscanf(buf, "%*s %s gateway %s metric %d %s", netname, gname, &metric, proto_type) != 4) {
        syslog(LOG_WARNING, MSGSTR(RT_INIT_23,"rt_dumbinit: syntax error, line %d\n"), line);
        TRACE_TRC(MSGSTR(RT_INIT_23,"rt_dumbinit: syntax error, line %d\n"), line);
        error = TRUE;
      } else if (!getnetorhostname(keyword, netname, &netaddr)) {
        syslog(LOG_WARNING, MSGSTR(RT_INIT_25,"rt_dumbinit: invalid %s name or address %s\n"), keyword, netname);
        TRACE_TRC(MSGSTR(RT_INIT_25,"rt_dumbinit: invalid %s name or address %s\n"), keyword, netname);
        error = TRUE;
      } else if (!getnetorhostname("host", gname, &gateway)) {
        syslog(LOG_WARNING, MSGSTR(RT_INIT_28,"rt_dumbinit: invalid gateway name or address %s\n"), gname);
        TRACE_TRC(MSGSTR(RT_INIT_28,"rt_dumbinit: invalid gateway name or address %s\n"), gname);
        error = TRUE;
      } else {
      	/* Initialize routings tables with non-routing gateway */
        /*
         * first delete any old route in exterior 
         * route table read from kernel if not a host route.
         */
        old_install = install;
        if (strcasecmp(keyword, "host") == 0) {
          tableadd = HOSTTABLE;
        } else {
          tableadd = INTERIOR;
          rt = rt_lookup((int)EXTERIOR, &netaddr);
          if (rt != NULL) {
            if (equal(&rt->rt_router, &gateway)) {
              install = FALSE;	/* already in kernel */
            }
            rt_delete(rt, KERNEL_INTR);
          }
        }
        rt = rt_lookup(tableadd, &netaddr);
        if (rt != NULL) {
          if (equal(&rt->rt_router, &gateway)) {
            install = FALSE;	/* already in kernel */
          }
          rt_delete(rt, KERNEL_INTR);
        }
        if (strcasecmp("rip", proto_type) == 0) {
          ptype = RTPROTO_RIP;
          metric = mapmetric(RIP_TO_HELLO, (u_short)metric);
        } else if (strcasecmp("hello", proto_type) == 0) {
          ptype = RTPROTO_HELLO;
        } else if (strcasecmp("egp", proto_type) == 0) {
          ptype = RTPROTO_EGP;
          metric = mapmetric(EGP_TO_HELLO, (u_short)metric);
          tableadd = EXTERIOR;
        } else {
          syslog(LOG_WARNING, MSGSTR(RT_INIT_34,"unsupported protocol %s line %d\n"), proto_type, line);
          TRACE_TRC(MSGSTR(RT_INIT_34,"unsupported protocol %s line %d\n"), proto_type, line);
          error = TRUE;
          continue;
        }
        (void) rt_add(tableadd, (struct sockaddr *)&netaddr, (struct sockaddr *)&gateway, metric,
				 RTS_STATIC|RTS_PASSIVE, ptype, ptype, 0, 0);
        install = old_install;
      }
    } /* end "net" || "host" entry */
    else if (strcasecmp(keyword, "defaultgateway") == 0) {
      if (sscanf(buf, "%*s %s %s %s", gname, proto_type, deftype) != 3) {
        syslog(LOG_WARNING, MSGSTR(RT_INIT_38,"rt_dumbinit: syntax error, line %d"), line);
        TRACE_TRC(MSGSTR(RT_INIT_23,"rt_dumbinit: syntax error, line %d\n"), line);
        error = TRUE;
      }
      if (!strcasecmp(deftype, "metric")) {
        if (sscanf(buf, "%*s %*s %*s %*s %d %s", &metric, deftype) != 2) {
          syslog(LOG_WARNING, MSGSTR(RT_INIT_38,"rt_dumbinit: syntax error, line %d"), line);
          TRACE_TRC(MSGSTR(RT_INIT_23,"rt_dumbinit: syntax error, line %d\n"), line);
          error = TRUE;
        }
        if (metric < 0 || metric > DELAY_INFINITY) {
          syslog(LOG_WARNING, MSGSTR(RT_INIT_44,"rt_dumbinit: invalid metric: %d, line %d"),
                              metric, line);
          TRACE_TRC(MSGSTR(RT_INIT_45,"rt_dumbinit: invalid metric: %d, line %d\n"),
                              metric, line);
          error = TRUE;
        }
      } else {
        metric = -1;
      }
      if (!getnetorhostname("host", gname, &gateway)) {
        syslog(LOG_WARNING, MSGSTR(RT_INIT_47,"rt_dumbinit: invalid gateway name or address %s"),
                  gname);
        TRACE_TRC(MSGSTR(RT_INIT_48,"rt_dumbinit: invalid gateway name or address %s\n"),
                  gname);
        error = TRUE;
      } else {		/* Initialize default gateway */
        /*
         * check if a kernel entry exists for default
         * route, if not add new default else change 
         * to new default
         */
        defaultdst.sin_family = AF_INET;
        defaultdst.sin_addr.s_addr = DEFAULTNET;
        if (strcasecmp("rip", proto_type) == 0) {
          ptype = RTPROTO_RIP;
          if ( metric < 0 ) {
            metric = RIPHOPCNT_INFINITY - 1;
          }
          metric = mapmetric(RIP_TO_HELLO, (u_short) metric);
        } else if (strcasecmp("hello", proto_type) == 0) {
          ptype = RTPROTO_HELLO;
          if ( metric < 0 ) {
            metric = DELAY_INFINITY - 100;
          }
        } else if (strcasecmp("egp", proto_type) == 0) {
          ptype = RTPROTO_EGP;
          if ( metric < 0 ) {
            metric = HOPCNT_INFINITY - 1;
          }
          metric = mapmetric(EGP_TO_HELLO, (u_short) metric);
        } else {
          syslog(LOG_WARNING, MSGSTR(RT_INIT_34,"unsupported protocol %s line %d"),
                    proto_type, line);
          TRACE_TRC(MSGSTR(RT_INIT_34,"unsupported protocol %s line %d\n"),
                    proto_type, line);
          error = TRUE;
          continue;
        }
        if (strcasecmp("passive", deftype) == 0) {
          /*
           * get rid of all left over defaults from initialization.
           */
          while (rt = rt_lookup((int)EXTERIOR, &defaultdst)) {
            rt_delete(rt, KERNEL_INTR);
          }
          rt = rt_lookup((int)INTERIOR, &defaultdst);
          if (rt == NULL) {
            if( !(rt = rt_add((int)INTERIOR, (struct sockaddr *)&defaultdst, (struct sockaddr *)&gateway, metric,
                          RTS_PASSIVE, ptype, ptype, 0, 0)) ) {
              syslog(LOG_ERR, MSGSTR(RT_INIT_55,"rt_dumbinit: error adding default route\n"));
              TRACE_INT(MSGSTR(RT_INIT_55,"rt_dumbinit: error adding default route\n"));
              error = TRUE;
              continue;
            }
          } else {
            (void) rt_change(rt, (struct sockaddr *)&gateway, metric, ptype, ptype, 0, 0);
            rt->rt_state |= RTS_PASSIVE;
          }
          if ((default_gateway = (struct rt_entry *)malloc(sizeof(struct rt_entry))) == NULL) {
            syslog(LOG_ERR, MSGSTR(RT_INIT_57,"rt_dumbinit: out of memory"));
            quit();
          }
          bcopy((char *)rt, (char *)default_gateway, sizeof(struct rt_entry));
        } else if (strcasecmp("active", deftype) == 0) {
          /*
           * get rid of all left over defaults from initialization.
           */
          while (rt = rt_lookup((int)INTERIOR, &defaultdst)) {
            rt_delete(rt, KERNEL_INTR);
          }
          rt = rt_lookup((int)EXTERIOR, &defaultdst);
          if (rt == NULL) {
            if ( !(rt = rt_add((int)EXTERIOR, (struct sockaddr *)&defaultdst, (struct sockaddr *)&gateway, metric,
                          RTS_PASSIVE, ptype, ptype, 0, 0)) ) {
              syslog(LOG_ERR, MSGSTR(RT_INIT_55,"rt_dumbinit: error adding default route\n"));
              TRACE_INT(MSGSTR(RT_INIT_55,"rt_dumbinit: error adding default route\n"));
              error = TRUE;
              continue;
            }
          } else {
            (void) rt_change(rt, (struct sockaddr *)&gateway, metric, ptype, ptype, 0, 0);
            rt->rt_state |= RTS_PASSIVE;
          }
          if ((default_gateway = (struct rt_entry *)malloc(sizeof(struct rt_entry))) == NULL) {
            syslog(LOG_ERR, MSGSTR(RT_INIT_57,"rt_dumbinit: out of memory"));
            quit();
          }
          bcopy((char *)rt, (char *)default_gateway, sizeof(struct rt_entry));
        } else {
          syslog(LOG_WARNING, MSGSTR(RT_INIT_62,"unsupported default type %s line %d"),
                    deftype, line);
          TRACE_TRC(MSGSTR(RT_INIT_63,"unsupported default type %s line %d\n"),
                    deftype, line);
          error = TRUE;
          continue;
        }
      }
    }	/* end "defaultgateway" entry */
  } /* end while */
  if (error) {
    syslog(LOG_EMERG, MSGSTR(RT_INIT_64,"rt_dumbinit: %s: initialization error\n"), EGPINITFILE);
    quit();
  }
}

#endif NSS
/*
 * rt_NRadvise_init() reads the initialization file EGPINITFILE to
 * determine user specification of nets allowed to be advised in Network 
 * Reachability messages. If any such nets are specified, only those specified
 * are allowed to be advised.
 * These nets must still meet the normal rules. i.e. they should be either
 * direct nets or nets reached via non-routing gateways of the same autonomous
 * sytem reported in EGPINITFILE. If the net is not one of these it is
 * ignored.
 *
 * EGPINITFILE relevant format is:
 * egpnetsreachable name name .... name
 */

rt_NRadvise_init(fp)
	FILE *fp;
{
  char keyword[MAXHOSTNAMELENGTH+1];
  char netname[MAXHOSTNAMELENGTH+1];
  char line[256], *lp;
  struct sockaddr_in netaddr;
  struct advlist *templist;
  struct advlist *tailof;
  int	error = FALSE;
  int	net_count = 0;

  adlist = (struct advlist *) NULL;
  srcriplist = (struct advlist *) NULL;
  srchellolist = (struct advlist *) NULL;
  trustedhelloerlist = (struct advlist *) NULL;
  trustedripperlist = (struct advlist *) NULL;
  templist = (struct advlist *) NULL;
  tailof = (struct advlist *) NULL;
  bzero((char *)&netaddr, sizeof(netaddr));
  netaddr.sin_family = AF_INET;

  rewind(fp);
  /*
   * read first word of line and compare to key words
   */
  while (fgets(line, sizeof(line), fp) != NULL) {
    lp = line;
    if (gettoken(&lp, keyword, sizeof(keyword)) == 0)
      continue;
    if ((strcasecmp(keyword, "egpnetsreachable") == 0) ||
        (strcasecmp(keyword, "noripoutinterface") == 0) ||
        (strcasecmp(keyword, "noripfrominterface") == 0) ||
        (strcasecmp(keyword, "nohellooutinterface") == 0) ||
        (strcasecmp(keyword, "nohellofrominterface") == 0) ||
        (strcasecmp(keyword, "trustedripgateways") == 0) ||
        (strcasecmp(keyword, "trustedhellogateways") == 0) ||
        (strcasecmp(keyword, "passiveinterfaces") == 0) ||
        (strcasecmp(keyword, "sourcehellogateways") == 0) ||
        (strcasecmp(keyword, "martiannets") == 0) ||
        (strcasecmp(keyword, "sourceripgateways") == 0)) {

      TRACE_INT("%s ", keyword);
      /*
       *  Don't allow egpnetsreachable if we have restriction
       *  lists.  If we have no restriction lists we will
       *  allow it as a "soft" level of restriction.
       */
      if ((strcasecmp(keyword, "egpnetsreachable") == 0) &&
          ((announcethesenets != 0) || (donotannounce != 0))) {
        printf(MSGSTR(RT_INIT_78,"ignored with announce or noannounce restrictions.\n"));
        goto nextone;
      }
      /*
       * read successive net strings into destination until end of line
       */
      while (gettoken(&lp, netname, sizeof(netname)) > 0) {
          TRACE_INT("%s ", netname);
          if (!getnetorhostname(
                  ((strcasecmp(keyword,"egpnetsreachable") == 0) || (strcasecmp(keyword,"martiannets") == 0))
                                     ? "net" : "host", netname, &netaddr)) {
              syslog(LOG_WARNING, MSGSTR(RT_INIT_84,"NRadvise_init: invalid name or address %s"),
                        netname);
              error = TRUE;
          } else {
              struct advlist *al;
              int rprotoflag = 0;
              struct interface *ifp;

              if (strcasecmp(keyword, "noripoutinterface") == 0) {
                rprotoflag = IFF_NORIPOUT;
	      }
              if (strcasecmp(keyword, "noripfrominterface") == 0) {
                rprotoflag = IFF_NORIPIN;
	      }
              if (strcasecmp(keyword, "nohellooutinterface") == 0) {
                rprotoflag = IFF_NOHELLOOUT;
	      }
              if (strcasecmp(keyword,"nohellofrominterface") == 0) {
                rprotoflag = IFF_NOHELLOIN;
	      }
              if (strcasecmp(keyword,"passiveinterfaces") == 0) {
                rprotoflag = IFF_NOAGE;
	      }
              if (rprotoflag != 0) {
                if ((ifp = if_ifwithaddr((struct sockaddr *)&netaddr)) <= (struct interface *)0) {
                  syslog(LOG_WARNING, MSGSTR(RT_INIT_90,"bad interface in control list.\n"));
                  error = TRUE;
                } else {
                  ifp->int_flags |= rprotoflag;
                }
              } else {
                if ((al = (struct advlist *)malloc(sizeof
                              (struct advlist))) == NULL) {
                  syslog(LOG_EMERG, MSGSTR(RT_INIT_91,"out of memory\n"));
                  quit();
                }
                al->next = templist;
                if (templist == NULL) {
                  tailof = al;
                }
                templist = al;
                al->destnet = netaddr.sin_addr;
                net_count++;
              }
            }
      }
      TRACE_INT("\n");
      if (strcasecmp(keyword, "egpnetsreachable") == 0) {
        n_remote_nets += net_count;
        if (adlist) {
          tailof->next = adlist;
        }
        adlist = templist;
      }
      if (strcasecmp(keyword, "martiannets") == 0) {
        if (martians) {
          tailof->next = martians;
        }
        martians = templist;
      }
      else if (strcasecmp(keyword, "sourceripgateways") == 0) {
        if (srcriplist) {
          tailof->next = srcriplist;
        }
        srcriplist = templist;
      }
      else if (strcasecmp(keyword, "sourcehellogateways") == 0) {
        if (srchellolist) {
          tailof->next = srchellolist;
        }
        srchellolist = templist;
      }
      else if (strcasecmp(keyword, "trustedhellogateways") == 0) {
        if (trustedhelloerlist) {
          tailof->next = trustedhelloerlist;
        }
        trustedhelloerlist = templist;
      }
      else if (strcasecmp(keyword, "trustedripgateways") == 0) {
        if (trustedripperlist) {
          tailof->next = trustedripperlist;
        }
        trustedripperlist = templist;
      }
      templist = (struct advlist *) NULL;
    }				/* end if "THE WORLD" */
nextone:    ;
  } 				/*end while*/
  if (error) {
    syslog(LOG_EMERG, MSGSTR(RT_INIT_99,"rt_NRadvise_init: %s: initialization error\n"),
              EGPINITFILE);
    quit();
  }
}

init_options(fp)
	FILE *fp;
{
  char keyword[MAXHOSTNAMELENGTH+1];
  char option[MAXHOSTNAMELENGTH+1];
  char line[256], *lp;
  int	route_proto = FALSE,
	error = FALSE,
	egp_conv = 0;

  /*
   *  standard initializations - might as well be done here
   */
#ifndef NSS
  doing_egp = FALSE;
  doing_rip = TRUE;
  doing_hello = FALSE;
  rip_supplier = hello_supplier = -1;
  rip_gateway = hello_gateway = FALSE;
  rip_default = hello_default = 0;
  rip_pointopoint = FALSE;
  hello_pointopoint = FALSE;
#else   NSS
  doing_egp = TRUE;
#endif  NSS
  conv_factor = HOPCNT_INFINITY;
  rt_maxage = RT_MINAGE;
  egpsleep = MINHELLOINT + HELLOMARGIN;
  n_remote_nets = 0;
#ifndef NSS
  /*
   * initialize the hello_default net.
   */
  bzero((char *)&hello_dfltnet, sizeof(hello_dfltnet));
  hello_dfltnet.sin_family = AF_INET;
  hello_dfltnet.sin_addr.s_addr = htonl((u_int)HELLO_DEFAULT);
#endif NSS

  rewind(fp);

  TRACE_INT(MSGSTR(RT_INIT_100,"init_options: Reading configuration protocol options:\n"));

  while (fgets(line, sizeof(line), fp) != NULL) {
    lp = line;
    if (gettoken(&lp, keyword, sizeof(keyword)) == 0)
      continue;
    route_proto = 0;
    if (strcasecmp(keyword, "EGP") == 0) {
      route_proto = IPPROTO_EGP;
#ifndef NSS
    } else if (strcasecmp(keyword, "RIP") == 0) {
      route_proto = IPPROTO_RIP;
    } else if (strcasecmp(keyword, "HELLO") == 0) {
      route_proto = IPPROTO_HELLO;
#endif NSS
    } else if (strcasecmp(keyword, "defaultegpmetric") == 0) {
      route_proto++;
      egp_conv++;
    }

    if (route_proto != 0) {
      TRACE_INT(MSGSTR(RT_INIT_105,"%s options: "), keyword);
      while (gettoken(&lp, option, sizeof(option))) {
            TRACE_INT("%s ", option);
            if (egp_conv) {
              if ((conv_factor = atoi(option)) < 0 ||
                  conv_factor > HOPCNT_INFINITY - 1) {
                error = TRUE;
                syslog(LOG_WARNING, MSGSTR(RT_INIT_113,"EGP conversion factor error - %d\n"),
                          conv_factor);
              }
              break;
            }
            if (strcasecmp(option, "yes") == 0) {
              switch (route_proto) {
#ifndef NSS
                case IPPROTO_RIP:
                  doing_rip = TRUE;
                  break;
                case IPPROTO_HELLO:
                  doing_hello = TRUE;
                  break;
#endif NSS
                case IPPROTO_EGP:
                  doing_egp = TRUE;
                  break;
              }
              break;
            }
            if (strcasecmp(option, "no") == 0) {
              switch (route_proto) {
#ifndef NSS
                case IPPROTO_RIP:
                  doing_rip = FALSE;
                  rip_supplier = FALSE;
                  break;
                case IPPROTO_HELLO:
                  doing_hello = FALSE;
                  hello_supplier = FALSE;
                  break;
#endif NSS
                case IPPROTO_EGP:
                  doing_egp = FALSE;
                  break;
              }
              break;
            }
#ifndef NSS
            if (strcasecmp(option, "quiet") == 0) {
              switch (route_proto) {
                case IPPROTO_RIP:
                  doing_rip = TRUE;
                  rip_supplier = FALSE;
                  break;
                case IPPROTO_HELLO:
                  doing_hello = TRUE;
                  hello_supplier = FALSE;
                  break;
              }
              break;
            }
            if (strcasecmp(option, "supplier") == 0) {
              switch (route_proto) {
                case IPPROTO_RIP:
                  doing_rip = TRUE;
                  rip_supplier = TRUE;
                  break;
                case IPPROTO_HELLO:
                  doing_hello = TRUE;
                  hello_supplier = TRUE;
                  break;
              }
              break;
            }
            if (!strcasecmp(option, "pointopoint") ||
                !strcasecmp(option, "pointtopoint")) {
              switch (route_proto) {
                case IPPROTO_RIP:
                  doing_rip = TRUE;
                  rip_supplier = TRUE;
                  rip_pointopoint = TRUE;
                  break;
                case IPPROTO_HELLO:
                  doing_hello = TRUE;
                  hello_supplier = TRUE;
                  hello_pointopoint = TRUE;
                  break;
              }
              break;
            }
            if (strcasecmp(option, "gateway") == 0) {
	      (void) gettoken(&lp, option, sizeof(option));
              switch (route_proto) {
                case IPPROTO_RIP:
                  doing_rip = TRUE;
                  rip_supplier = TRUE;
                  rip_gateway = TRUE;
                  rip_default = atoi(option);
                  if (rip_default < 0 || rip_default >= RIPHOPCNT_INFINITY) {
                    error = TRUE;
                    syslog(LOG_WARNING, MSGSTR(RT_INIT_121,"RIP default metric error - %d\n"),
                              rip_default);
                  }
                  if (!(error))
                    TRACE_INT("%d ", rip_default);
                  break;
                case IPPROTO_HELLO:
                  doing_hello = TRUE;
                  hello_supplier = TRUE;
                  hello_gateway = TRUE;
                  hello_default = atoi(option);
                  if (hello_default < 0 || hello_default >= DELAY_INFINITY) {
                    error = TRUE;
                    syslog(LOG_WARNING, MSGSTR(RT_INIT_123,"HELLO default metric error - %d\n"),
                              hello_default);
                  }
                  if (!(error))
                    TRACE_INT("%d ",hello_default);
                  break;
                }
                break;
            }
#endif NSS
      }				/* end while gettoken */
      TRACE_INT("\n");
    }				/* end if "route_proto" */
  } 				/*end while*/
  if (error) {
    syslog(LOG_EMERG, MSGSTR(RT_INIT_126,"init_options: %s: initialization error\n"), EGPINITFILE);
    quit();
  }
  init_display_config("init_options: RIP", doing_rip, rip_supplier, rip_gateway, rip_pointopoint, rip_default);
  init_display_config("init_options: HELLO", doing_hello, hello_supplier, hello_gateway, hello_pointopoint, hello_default);
  init_display_config("init_options: EGP", doing_egp, -1, 0, 0, 0);
}

init_display_config(s, doing, supplier, gateway, p2p, metric)
	char *s;
	int doing, supplier, gateway, p2p, metric;
{

  TRACE_INT("%s ", s);
  if (doing) {
    if ( p2p ) {
      TRACE_INT("pointopoint");
    } else if ( gateway ) {
      TRACE_INT("gateway %d", metric);
    } else if ( supplier == TRUE ) {
      TRACE_INT("supplier");
    } else if ( supplier == FALSE ) {
      TRACE_INT("quiet");
    } else {
      TRACE_INT("yes");
    }
  } else {
    TRACE_INT("no");
  }
  TRACE_INT("\n");
}

