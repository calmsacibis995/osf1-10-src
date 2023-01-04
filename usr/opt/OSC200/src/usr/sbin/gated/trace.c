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
static char	*sccsid = "@(#)$RCSfile: trace.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:59:44 $";
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
 * COMPONENT_NAME: TCPIP trace.c
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
#define MSGSTR(n,s) NLcatgets(catd,MS_TRACE,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/* trace_egp.c
 *
 * Tracing functions for route changes, received packets and EGP messages.
 *
 * Functions: traceaction (modified from routed/trace.c), tracercv, traceegp
 */


#include "include.h"
#ifndef vax11c
#include <sys/stat.h>
#endif  vax11c
#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)
#include <snmp.h>
#endif  defined(AGENT_SNMP) || defined(AGENT_SGMP)

extern FILE *conf_open();

/*
 * traceaction() traces changes to the routing tables
 */
traceaction(fd, action, rt)
	FILE *fd;			/* trace logfile */
	char *action;
	struct rt_entry *rt;
{
  struct sockaddr_in *dst, *gate;
  register struct bits *p;
  register int first;
  char *cp;

  if (fd == NULL)
    return;
  fprintf(fd, "%-8s ", action);
  dst = (struct sockaddr_in *)&rt->rt_dst;
  gate = (struct sockaddr_in *)&rt->rt_router;
  fprintf(fd, "%4s %-15s ", (rt->rt_state & RTS_HOSTROUTE ? "host" : "net" ), inet_ntoa(dst->sin_addr));
  fprintf(fd, MSGSTR(TRACE_5,"gw %-15s metric %5d  proto "), inet_ntoa(gate->sin_addr), rt->rt_metric);
  cp = "%s";
  for (first = 1, p = protobits; p->t_bits; p++) {
    if ((rt->rt_proto & p->t_bits) == 0)
      continue;
    fprintf(fd, cp, p->t_name);
    if (first) {
      cp = "|%s";
      first = 0;
    }
  }
  fprintf(fd, MSGSTR(TRACE_8,"  flags"));
  cp = " %s";
  for (first = 1, p = flagbits; p->t_bits; p++) {
    if ((rt->rt_flags & p->t_bits) == 0)
      continue;
    fprintf(fd, cp, p->t_name);
    if (first) {
      cp = "|%s";
      first = 0;
    }
  }
  fprintf(fd, MSGSTR(TRACE_11,"  state"));
  cp = " %s";
  for (first = 1, p = statebits; p->t_bits; p++) {
    if ((rt->rt_state & p->t_bits) == 0)
      continue;
    fprintf(fd, cp, p->t_name);
    if (first) {
      cp = "|%s";
      first = 0;
    }
  }
  if (rt->rt_proto & RTPROTO_HELLO) {
    printf(MSGSTR(TRACE_14,"  hwin_min %d "), rt->rt_hwindow.h_min);
  }
  if ( rt->rt_fromproto != rt->rt_proto) {
    printf(MSGSTR(TRACE_15,"  fromproto "));
    cp = "%s";
    for (first = 1, p = protobits; p->t_bits; p++) {
      if ((rt->rt_fromproto & p->t_bits) == 0)
        continue;
      fprintf(fd, cp, p->t_name);
      if (first) {
        cp = "|%s";
        first = 0;
      }
    }
  }
  if ( rt->rt_state & RTS_EXTERIOR ) {
    printf(MSGSTR(TRACE_18,"  extmetric %d"), rt->rt_metric_exterior);
  }
  if ( rt->rt_as != mysystem ) {
    printf(MSGSTR(TRACE_19,"  as %d"), rt->rt_as);
  }
  (void) fprintf(fd, "\n");

#ifdef	notdef
  if ((rt->rt_announcelist != NULL) && (tracing & TR_INT)) {
    dst = (struct sockaddr_in *)&rt->rt_announcelist->rdst;
    fprintf(fd, "ANNOUNCE net %s, ", inet_ntoa(dst->sin_addr));
    fprintf(fd, "metric %d, proto %d\n",rt->rt_announcelist->regpmetric, rt->rt_announcelist->rproto);
  }
  if ((rt->rt_noannouncelist != NULL) && (tracing & TR_INT)) {
    dst = (struct sockaddr_in *)&rt->rt_noannouncelist->rdst;
    fprintf(fd, "NOANNOUNCE net %s, ", inet_ntoa(dst->sin_addr));
    fprintf(fd, "metric %d, proto %d\n",rt->rt_noannouncelist->regpmetric, rt->rt_noannouncelist->rproto);
  }
#endif	notdef
}


/*
 * Turn off tracing.
 */
traceoff(tracetype)
	int tracetype;
{
  int tracedel = 0;


#ifndef NSS
  if (tracetype & RIP_TRACE) {
    tracedel |= (TR_UPDATE|TR_RIP);
  }
  if (tracetype & HELLO_TRACE) {
    tracedel |= (TR_UPDATE|TR_HELLO);
  }
#endif NSS
  if (!tracing) {
    return;
  }
  tracing &= ~tracedel;  /* unflag added tracing options */
  if ((ftrace != NULL) && (tracetype & GEN_TRACE)) {
    TRACE_TRC(MSGSTR(TRACE_25,"Tracing to %s suspended at %s"), logfile, strtime);
    tracing = 0;
#ifdef	notdef
    (void) fflush(ftrace);
    (void) fflush(stdout);
    (void) fflush(stderr);
#endif	notdef
    (void) fclose(ftrace);
    (void) fclose(stdout);
    (void) fclose(stderr);
    ftrace = NULL;
    syslog(LOG_ERR, MSGSTR(TRACE_26,"tracing to %s suspended"), logfile);
  }
  return;
}

/*
 * Turn on tracing.
 */
traceon(file, tracetype)
	char *file;
	int tracetype;
{
  struct stat stbuf;
  int traceadd = 0;

  switch (tracetype) {
    case RIP_TRACE:
      traceadd |= (TR_UPDATE|TR_RIP);
      break;
#ifndef NSS
    case HELLO_TRACE:
      traceadd |= (TR_UPDATE|TR_HELLO);
      break;
    case GEN_TRACE:
      traceadd |= savetrace;
      break;
#endif NSS
  }

  if ((ftrace == NULL) && (file != NULL)) {
    if ((stat(file, &stbuf) >= 0) && ((stbuf.st_mode & S_IFMT) != S_IFREG)) {
      return;
    }
    if ((ftrace = fopen(file, "a")) == NULL) {
      syslog(LOG_ERR, MSGSTR(TRACE_28,"error opening %s: %m"), file);
      return;
    }
    setlinebuf(ftrace);
    if ( freopen(file, "a", stdout) == NULL ) {
      syslog(LOG_ERR, MSGSTR(TRACE_28,"error opening %s: %m"), file);
      return;
    }
    setlinebuf(stdout);
    if ( freopen(file, "a", stderr) == NULL ) {
      syslog(LOG_ERR, MSGSTR(TRACE_28,"error opening %s: %m"), file);
      return;
    }
    setlinebuf(stderr);
    syslog(LOG_ERR, MSGSTR(TRACE_33,"tracing to %s started"), file);
    printf(MSGSTR(TRACE_34,"\fTracing to %s started at %s"), file, strtime);
  }
  tracing |= traceadd;
  tracedisplay(tracing);
}


/*
 * Read tracing information from conf file
 */
traceflags()
{
	FILE	*fd;
	int	more_flags, new_flags, trace_flags = 0;
	char	c, keyword[MAXHOSTNAMELENGTH+1];
	struct	bits *flag_ptr;
	char line[256], *lp;

	if ( (fd = conf_open()) == NULL) {
		return(trace_flags);
	}
	while (fgets(line, sizeof(line), fd) != NULL) {
		if ( (*line == '\n') || (*line == '#') ) {
			continue;
		}
		lp = line;
		(void) sscanf(lp, "%s", keyword);
		lp += strlen(keyword);
		if (strcasecmp(keyword, "traceflags") != 0) {
			continue;
		}
		more_flags = TRUE;
		while (more_flags) {
			switch (c= *lp++) {
			case '\n':
				*--lp = c;
			case '\0':
			case '#':
				more_flags = FALSE;
				break;
			case ' ':
			case '\t':
				break;
			default:
				*--lp = c;
				(void) sscanf(lp, "%s", keyword);
				lp += strlen(keyword);
				new_flags = 0;
				for ( flag_ptr = trace_types; flag_ptr->t_name; flag_ptr++ ) {
					if ( strcasecmp(keyword, flag_ptr->t_name) == 0 ) {
						new_flags = flag_ptr->t_bits;
						break;
					}
				}
				if ( new_flags == 0 ) {
					TRACE_INT(MSGSTR(TRACE_38,"traceflags: invalid trace flag '%s' ignored\n"), keyword);
					syslog(LOG_ERR, MSGSTR(TRACE_38,"traceflags: invalid trace flag '%s' ignored\n"), keyword);
				}
				trace_flags |= new_flags;
			}
		}
	}
	(void) fclose(fd);

	return(trace_flags);
}


/*
 *	Display trace options enabled
 */
tracedisplay(trace_flags)
int	trace_flags;
{
  struct bits *flag_ptr, *last_ptr;

  printf(MSGSTR(TRACE_40,"\nTracing flags enabled: "));
  if ( trace_flags ) {	
    last_ptr = &trace_types[1];
    for ( flag_ptr = trace_types; flag_ptr->t_name; last_ptr = flag_ptr++ ) {
      if ( ((trace_flags & flag_ptr->t_bits) == flag_ptr->t_bits) &&
           (flag_ptr->t_bits != last_ptr->t_bits) ) {
        printf("%s ", flag_ptr->t_name);
      }
    }
    printf("\n\n");
  } else {
    printf(MSGSTR(TRACE_43,"none\n\n"));
  }
}


/*
 * trace packets received
 *
 * Only trace data part - EGP packets include IP header but ICMP packets do
 * not (but on 4.3BSD they DO, well it is passed to the RAW ICMP socket.
 */
tracercv(sock, protocol, packet, length, frompt)
	int sock, protocol, length;
	struct sockaddr_in *frompt;
	char *packet;
{
  struct sockaddr_in  sockname;
  int  socklen = sizeof (sockname), i;
  struct ip *pkt;
  int hlen;

  pkt = (struct ip *)packet;
  switch (protocol) {
    case IPPROTO_EGP: 
      hlen = pkt->ip_hl;
      hlen <<= 2;			/* IP header length in octets */
      TRACE_EGPPKT(EGP RECV, pkt->ip_src, pkt->ip_dst,
			(struct egppkt *)(packet+hlen), pkt->ip_len);
      break;
#ifdef IMPLINK_IP
    case IMPLINK_IP: 	/* not implemented */
#endif
    case IPPROTO_ICMP: 	/* IP header not passed with ICMP messages */
      if (getsockname (sock, (char *)&sockname, &socklen))
        p_error ("tracercv: getsockname");
      printf(MSGSTR(TRACE_45,"\nPacket received at time %s"), strtime);
      printf(MSGSTR(TRACE_46,"protocol %d, from addr %s"),protocol,inet_ntoa(frompt->sin_addr));
      printf(MSGSTR(TRACE_47,", on interface addr %s\n"), inet_ntoa(sockname.sin_addr));
      printf(MSGSTR(TRACE_48,"Data octets (decimal):\n"));
      for (i = 0; i < length; i++)
        printf ("%d ", packet[i]);
      printf ("\n");
      break;
  }
  return;
}

/*
 * Trace EGP packet
 */
traceegp(comment, src, dst, egp, length)
	char  *comment;
	struct in_addr	src, dst;
	struct	egppkt	*egp;
	int	length;
{
  struct egppkt	*ep;
  int reason;
  char *type, *code, *status;
  static char *no_codes[1] = { "0" };
  static struct  {
	char *et_type;
	short et_ncodes;
	char **et_codes;
	char et_nstatus;
	char **et_status;
	} egp_types[9] = {
	"Invalid",	-1,	(char **) 0,	-1,	(char **)0,	/* 0 - Error */
	"UPDATE",	0,	no_codes,	3,	egp_nr_status,	/* 1 - Nets Reachable */
	"POLL",		0,	no_codes,	3,	egp_nr_status,	/* 2 - Poll */
	"ACQUIRE",	5,	egp_acq_codes,	7,	egp_acq_status,	/* 3 - Neighbor Aquisition */
	"Invalid",	-1,	(char **) 0,	-1,	(char **)0,	/* 4 - Error */
	"NEIGHBOR",	2,	egp_reach_codes, 3,	egp_nr_status,	/* 5 - Neighbor Reachability */
	"Invalid",	-1,	(char **) 0,	-1,	(char **)0,	/* 6 - Error */
	"Invalid",	-1,	(char **) 0,	-1,	(char **)0,	/* 7 - Error */
	"ERROR",	-1,	(char **) 0,	3,	egp_nr_status};	/* 8 - Error packet */

  printf("%s", comment);
  if (src.s_addr != INADDR_ANY)
    printf(" %s", inet_ntoa(src));
  printf(MSGSTR(TRACE_63," -> %s length %d at %s"), inet_ntoa(dst), length, strtime);

  if ( egp->egp_type <= EGPERR ) {
    type = egp_types[egp->egp_type].et_type;
    if ( (short)(egp->egp_code % UNSOLICITED) <= egp_types[egp->egp_type].et_ncodes ) {
      code = egp_types[egp->egp_type].et_codes[(egp->egp_code % UNSOLICITED)];
    } else {
      if ( (egp->egp_code % UNSOLICITED) == 0 ) {
	code = "";
      } else {
        code = MSGSTR(TRACE_52,"Invalid");
      }
    }
    if ( egp->egp_status <= egp_types[egp->egp_type].et_nstatus ) {
      status = egp_types[egp->egp_type].et_status[egp->egp_status];
    } else {
      status = MSGSTR(TRACE_52,"Invalid");
    }
  } else {
    type = MSGSTR(TRACE_52,"Invalid");
  }
  printf(MSGSTR(TRACE_68,"%s vers %d, type %s(%d), code %s(%d), status %s(%d)%s, AS# %d, id %d"),
		comment,
		egp->egp_ver,
		type, egp->egp_type,
		code, egp->egp_code,
		status, egp->egp_status,
		egp->egp_code & UNSOLICITED ? MSGSTR(TRACE_69," UNSOLICITED") : "",
		ntohs(egp->egp_system), ntohs(egp->egp_id));
  if (length > sizeof( struct egppkt))
    switch (egp->egp_type) {
      case EGPACQ:
        printf(MSGSTR(TRACE_71,", hello int %d, poll int %d"), 
                       ntohs(((struct egpacq *)egp)->ea_hint),
                       ntohs(((struct egpacq *)egp)->ea_pint));
        break;
      case EGPPOLL:
        printf(MSGSTR(TRACE_72,", src net %s"), inet_ntoa(((struct egppoll *)egp)->ep_net));
        break;
      case EGPNR:
        printf(MSGSTR(TRACE_73,", src net %s, #int %d, #ext %d\n"),
                       inet_ntoa(((struct egpnr *)egp)->en_net),
                       ((struct egpnr *)egp)->en_igw,
                       ((struct egpnr *)egp)->en_egw);
        if (length > sizeof(struct egpnr) && ( tracing & TR_UPDATE) ) {
#ifdef	EGP_UPDATE_FORMAT
          NR_format((struct egpnr *)egp, length);
#else	EGP_UPDATE_FORMAT
          register u_char *data, *dataf;

          data = (u_char *)egp + sizeof(struct egpnr);
          dataf = (u_char *)egp + length;
          for ( ; data < dataf; data++) 
             printf( "%d ", *data);
          printf("\n");
#endif	EGP_UPDATE_FORMAT
        }
        break;
      case EGPERR:
        reason = ntohs(((struct egperr *)egp)->ee_rsn);
	if ( reason > 7 ) {
	        printf(MSGSTR(TRACE_76,", error %d (invalid)\nreturned header: "),
			reason);
	} else {
		printf(MSGSTR(TRACE_77,", error: %s\nreturned header: "),
			egp_reasons[reason]);
	}
        ep = (struct egppkt *)((struct egperr*)egp)->ee_egphd;
        printf(MSGSTR(TRACE_78,"vers %d, typ %d, code %d, status %d, AS# %d,"),
                       ep->egp_ver, ep->egp_type, ep->egp_code,
                       ep->egp_status, ntohs(ep->egp_system));
        printf(MSGSTR(TRACE_79," id %d, %d, %d"), ntohs(ep->egp_id),
                       ((struct egperr *)egp)->ee_egphd[10],
                       ((struct egperr *)egp)->ee_egphd[11]);
        break;
    }
  printf("\n\n");
  return;
}

#ifndef NSS
tracerip(dir, who, cp, size)
	struct sockaddr_in *who;		/* should be sockaddr */
	char *dir, *cp;
	register int size;
{
  register struct rip *rpmsg = (struct rip *)cp;
  register struct netinfo *n;
  register char *cmd = MSGSTR(TRACE_52,"Invalid");

  if (rpmsg->rip_cmd && rpmsg->rip_cmd < RIPCMD_MAX) {
    cmd = ripcmds[rpmsg->rip_cmd];
  }
  printf(MSGSTR(TRACE_82,"RIP %s %s.%d vers %d, cmd %s, length %d"),
                   dir,
                   inet_ntoa(who->sin_addr),
                   ntohs(who->sin_port),
                   rpmsg->rip_vers,
                   cmd, size);
  switch (rpmsg->rip_cmd) {
#ifdef	RIPCMD_POLL
    case RIPCMD_POLL:
#endif	RIPCMD_POLL
    case RIPCMD_REQUEST:
    case RIPCMD_RESPONSE:
      printf(MSGSTR(TRACE_83," at %s"), strtime);
      if (tracing & TR_UPDATE) {
        size -= 4 * sizeof (char);
        n = rpmsg->rip_nets;
        for (; size > 0; n++, size -= sizeof (struct netinfo)) {
          if (size < sizeof (struct netinfo)) {
            break;
          }
          printf(MSGSTR(TRACE_84,"\tnet %-15s  metric %2d  size %d\n"), inet_ntoa(sock_inaddr(&n->rip_dst)),
                      ntohl((u_int)n->rip_metric), size);
         }
      }
      break;
    case RIPCMD_TRACEON:
      printf(MSGSTR(TRACE_85,", file %*s at %s"), size, rpmsg->rip_tracefile, strtime);
      break;
#ifdef	RIPCMD_POLLENTRY
    case RIPCMD_POLLENTRY:
      n = rpmsg->rip_nets;
      printf(MSGSTR(TRACE_86,", net %s at %s"),
                  inet_ntoa(sock_inaddr(&n->rip_dst)),
                  strtime);
      break;
#endif	RIPCMD_POLLENTRY
    default:
      printf(MSGSTR(TRACE_83," at %s"), strtime);
      break;
  }
  printf("\n");
}

/* ARGSUSED */
tracehello(comment, src, dst, hello, length, nets)
	char  *comment;
	struct in_addr	src, dst;
	char	*hello;
	int	length, nets;
{
  printf("%s %s -> ", comment, inet_ntoa(src));
  printf(MSGSTR(TRACE_90,"%s length %d"), inet_ntoa(dst), length);
  if (nets < 0) {
    printf(MSGSTR(TRACE_91,", %d nets"), nets);
  }
  printf(MSGSTR(TRACE_92," at %s\n"), strtime);
}

#endif NSS

#if     defined(AGENT_SNMP) || defined(AGENT_SGMP)
tracesnmp(comment, dst, packet, length)
  char *comment;
  struct sockaddr_in *dst;
  char *packet;
  int length;
{
  int size, integer;
  char *cp;
  char *pkt = packet;
  register struct bits *p;
#ifdef  AGENT_SNMP
  struct in_addr ipaddr;
#endif  AGENT_SNMP

  cp = MSGSTR(TRACE_93,"Unknown");
  for (p = snmp_types; p->t_bits > (u_int)0; p++) {
    if ( p->t_bits == *pkt ) {
      cp = p->t_name;
      break;
    }
  }

  printf(MSGSTR(TRACE_94,"%s %s.%d type %s(%d) length %d at %s"),
  	comment,
  	inet_ntoa(dst->sin_addr),
  	ntohs(dst->sin_port),
  	cp, *pkt++, length,
  	strtime);
 
  if (tracing & TR_UPDATE) {
    if ((pkt-packet) <= length) {
      switch (*packet) {
      case AGENT_RSP:
        while ((pkt-packet) <= length-1) {
          switch (*pkt++) {
            case INT:
              size = *pkt++;
              bcopy(pkt, (char *) &integer, size);
              pkt += size;
              printf(MSGSTR(TRACE_95,"%s\tinteger length %d value: %d 0x%x\n"), comment, size, integer, integer);
              break;
            case STR:
              size = *pkt++; 	
              printf(MSGSTR(TRACE_96,"%s\tstring length %d value: "), comment, size);
              for (; size; size--) {
                printf("%c", *pkt++);
              }
              printf("\n");
              break;
#ifdef  AGENT_SNMP
            case IPADD:
              size = *pkt++;
              bcopy(pkt, (char *) &ipaddr.s_addr, size);
              pkt += size;
              printf(MSGSTR(TRACE_99,"%s\tIP address length %d value: %s 0x%x\n"), comment, size, inet_ntoa(ipaddr), ipaddr.s_addr);
              break;
            case CNTR:
              size = *pkt++;
              bcopy(pkt, (char *) &integer, size);
              pkt += size;
              printf(MSGSTR(TRACE_100,"%s\tcounter length %d value: %u 0x%x\n"), comment, size, integer, integer);
              break;
            case GAUGE:
              size = *pkt++;
              bcopy(pkt, (char *) &integer, size);
              pkt += size;
              printf(MSGSTR(TRACE_101,"%s\tgauge length %d value: %u 0x%x\n"), comment, size, integer, integer);
              break;
            case TIME:
              size = *pkt++;
              bcopy(pkt, (char *) &integer, size);
              pkt += size;
              printf(MSGSTR(TRACE_102,"%s\ttimer length %d value: %u 0x%x\n"), comment, size, integer, integer);
              break;
#endif AGENT_SNMP
          }
        }
        break;
      case AGENT_ERR:
        break;
      case AGENT_REQ:
      case AGENT_REG:
        while ((pkt-packet) <= length-1) {
	  size = *pkt++; 	
          printf(MSGSTR(TRACE_103,"%s\tlength %d variable: "), comment, size);
          for (; size; size--) {
            printf("%02x", *pkt++ & 0xff);
          }
          printf("\n");
        }
        break;
      }
    }
  } else {
    printf("\n");
  }
}
#endif  defined(AGENT_SNMP) || defined(AGENT_SGMP)

dumpinfo()
{
  FILE *fd;
  register struct interface *ifp;

#define HELLO_REPORT	8	/* how far back we will report */

  register struct bits *p;
  register int first;
  char *cp, *ch;
  struct rthash	*rh;
  struct active_gw *agp;
  struct rthash	*base = hosthash;
  struct rt_entry *rt;
  int doinghost = 1, cnt, ind, cntr;
  struct in_addr tmpos;
  struct advlist *mptr;
  struct egpngh *ngp;

  if ((fd = fopen(DUMPFILE, "a")) <= (FILE *)0) {
    p_error("dumpinfo: ");
    return;
  }
  setlinebuf(fd);
  syslog(LOG_NOTICE, MSGSTR(TRACE_109,"dumpinfo: processing dump to %s"), DUMPFILE);
  TRACE_TRC(MSGSTR(TRACE_110,"dumpinfo: processing dump to %s at %s"), DUMPFILE, strtime);
  fprintf(fd, MSGSTR(TRACE_111,"\f\n\t%s[%d] version %s memory dump on %s at %s\n"), my_name, my_pid, Version, my_hostname, strtime);
  if (version_kernel) {
    fprintf(fd, "\t\t%s\n\n", version_kernel);
  }
  /*
   * Print out all of the interface stuff.
   */
  fprintf(fd, MSGSTR(TRACE_113,"Interface Stats:\n\n"));
  for (ifp = ifnet; ifp; ifp = ifp->int_next) {
    fprintf(fd, MSGSTR(TRACE_114,"\tInterface: %s\t\t\tMetric: %d\n"), ifp->int_name, ifp->int_metric);
    fprintf(fd, MSGSTR(TRACE_115,"\tAddress: %s\n"), inet_ntoa(sock_inaddr(&ifp->int_addr)));
    fprintf(fd, MSGSTR(TRACE_116,"\tUp-down transitions: %u\n"), ifp->int_transitions);
    if (ifp->int_flags & IFF_BROADCAST) {
      fprintf(fd, MSGSTR(TRACE_117,"\tBroadcast Address:   %s\n"), inet_ntoa(sock_inaddr(&ifp->int_broadaddr)));
    }
    if (ifp->int_flags & IFF_POINTOPOINT) {
      fprintf(fd, MSGSTR(TRACE_118,"\tDestination Address: %s\n"), inet_ntoa(sock_inaddr(&ifp->int_dstaddr)));
    }
    tmpos.s_addr = ntohl(ifp->int_net);
    fprintf(fd, MSGSTR(TRACE_119,"\tNet Number:    %s\t\tNet Mask:    %lx\n"),
                       inet_ntoa(tmpos),
                       ifp->int_netmask);
    tmpos.s_addr = ntohl(ifp->int_subnet);
    fprintf(fd, MSGSTR(TRACE_120,"\tSubnet Number: %s\t\tSubnet Mask: %lx\n"),
                       inet_ntoa(tmpos),
                       ifp->int_subnetmask);
    fprintf(fd, MSGSTR(TRACE_121,"\tFlags: <"));
    cp = "%s";
    for (first = 1, p = intfbits; p->t_bits > (u_int)0; p++) {
      if ((ifp->int_flags & p->t_bits) == 0)
        continue;
      fprintf(fd, cp, p->t_name);
      if (first) {
        cp = "|%s";
        first = 0;
      }
    }
    fprintf(fd, ">\n");
#ifndef NSS
    fprintf(fd, "\tFixed Metrics:\n");
    fprintf(fd, "\t\t\tRIP:\t");
    if (ifp->int_flags & IFF_RIPFIXEDMETRIC) {
      fprintf(fd, "%d\n", ifp->int_ripfixedmetric);
    } else {
      fprintf(fd, MSGSTR(TRACE_128,"NONE\n"));
    }
    fprintf(fd, "\t\t\tHELLO:\t");
    if (ifp->int_flags & IFF_HELLOFIXEDMETRIC) {
      fprintf(fd, "%d\n", ifp->int_hellofixedmetric);
    } else {
      fprintf(fd, MSGSTR(TRACE_128,"NONE\n"));
    }
#endif NSS
    fprintf(fd, MSGSTR(TRACE_132,"\tActive gateways on interface:"));
    if (ifp->int_active_gw == NULL) {
      fprintf(fd, MSGSTR(TRACE_128,"\tNONE\n"));
    } else {
      fprintf(fd, "\n");
      for (agp = ifp->int_active_gw; agp; agp = agp->next) {
         tmpos.s_addr = agp->addr;
         fprintf(fd, "\t\t\t%s\t<", inet_ntoa(tmpos));
         cp = "%s";
         for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
           if ((agp->proto & p->t_bits) == 0)
             continue;
           fprintf(fd, cp, p->t_name);
           if (first) {
             cp = "|%s";
             first = 0;
           }
         }
         fprintf(fd, ">\t%d\n", agp->timer);
      }
    }
    fprintf(fd, "\n");
  }
  /*
   *	EGP neighbor status
   */
  if ( doing_egp ) {
    struct tm *tmp;
    fprintf(fd, MSGSTR(TRACE_140,"EGP status:\n\tN_remote_nets: %d\t\tdefaultegpmetric: %d\n"), n_remote_nets, conv_factor);
    fprintf(fd, MSGSTR(TRACE_141,"\tAutonomous System: %d\t\tacquired: %d/%d\n"), mysystem, n_acquired, maxacq);
    fprintf(fd, "\tMaxage: %d\t\t\tMaxpollint: %d\n", rt_maxage, maxpollint);
    fprintf(fd, MSGSTR(TRACE_143,"\tPackets In: %d\t\t\tErrors In: %d\n"), egp_stats.inmsgs, egp_stats.inerrors);
    fprintf(fd, MSGSTR(TRACE_144,"\tPackets Out: %d\t\t\tErrors Out: %d\n"), egp_stats.outmsgs, egp_stats.outerrors);
    fprintf(fd, MSGSTR(TRACE_145,"\t\t\t\t\tTotal Errors: %d\n"), egp_stats.outerrors+egp_stats.inerrors);
    for (ngp = egpngh; ngp != NULL; ngp = ngp->ng_next) {
      fprintf(fd, MSGSTR(TRACE_146,"\n\tNeighbor: %s"),inet_ntoa(ngp->ng_addr));
      fprintf(fd, MSGSTR(TRACE_147,"\t\tInterface: %s\n"), inet_ntoa(ngp->ng_myaddr));
      tmp = localtime(&ngp->ng_htime);
      fprintf(fd, MSGSTR(TRACE_148,"\tHello time: %2d:%02d:%02d\t\tHello interval: %d\n"),
                  tmp->tm_hour, tmp->tm_min, tmp->tm_sec, ngp->ng_hint);
      tmp = localtime(&ngp->ng_stime);
      fprintf(fd, MSGSTR(TRACE_149,"\tPoll time: %2d:%02d:%02d\t\tPoll interval: %d\n"),
                  tmp->tm_hour, tmp->tm_min, tmp->tm_sec, ngp->ng_spint);
      fprintf(fd, MSGSTR(TRACE_150,"\tState: <"));
      for (p = egp_states; p->t_bits != (u_int) -1; p++) {
        if ( p->t_bits == ngp->ng_state ) {
          fprintf(fd, p->t_name);
          break;
        }
      }
      fprintf(fd, MSGSTR(TRACE_151,">\t\tReachability: <%s|%s>\n"),
        (ngp->ng_reach & NG_DOWN) ? "NG_DOWN" : "NG_UP",
        (ngp->ng_reach & ME_DOWN) ? "ME_DOWN" : "ME_UP");
     fprintf(fd, MSGSTR(TRACE_156,"\tLast poll received: %s"), inet_ntoa(ngp->ng_paddr));
     fprintf(fd, MSGSTR(TRACE_157,"\tNet to poll: %s\n"), inet_ntoa(ngp->ng_saddr));
     fprintf(fd, MSGSTR(TRACE_158,"\tFlags: %X <"), ngp->ng_flags);
      cp = "%s";
      for (first = 1, p = egp_flags; p->t_bits > (u_int)0; p++) {
        if ((ngp->ng_flags & p->t_bits)) {
          fprintf(fd, cp, p->t_name);
          if (first) {
            cp = "|%s";
            first = 0;
          }
        }
      }
      fprintf(fd, ">\n");
      fprintf(fd, "\tMetricOut: ");
      if ( (ngp->ng_flags & NG_METRICOUT) ) {
        fprintf(fd, "%d", ngp->ng_metricout);
      } else {
        fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\t\t\tMetricIn: ");
      if ( (ngp->ng_flags & NG_METRICIN) ) {
        fprintf(fd, "%d", ngp->ng_metricin);
      } else {
        fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\n\tASout: ");
      if ( (ngp->ng_flags & NG_ASOUT) ) {
        fprintf(fd, "%d", ngp->ng_asout);
      } else {
        fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\t\t\tASin: ");
      if ((ngp->ng_flags & NG_ASIN) || ngp->ng_asin) {
        fprintf(fd, "%d", ngp->ng_asin);
      } else {
        fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\n\tAS: ");
      if ((ngp->ng_flags & NG_AS) || ngp->ng_as) {
      	fprintf(fd, "%d", ngp->ng_as);
      }
      fprintf(fd, "\n\tDefaultMetric: ");
      if ( (ngp->ng_flags & NG_DEFAULTOUT) ) {
        fprintf(fd, "%d", ngp->ng_defaultmetric);
      } else {
        fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\t\tGateway: ");
      if ( (ngp->ng_flags & NG_GATEWAY) ) {
        fprintf(fd, "%s", inet_ntoa(ngp->ng_gateway));
      } else {
	fprintf(fd, MSGSTR(TRACE_164,"N/A"));
      }
      fprintf(fd, "\n");
    }
    fprintf(fd, "\n");
  }
  /*
   *	List all of the {no,}annoucetoAS clauses
   */
  if (sendas) {
    struct as_list *tmp_list;
    struct as_entry *tmp_entry;

    fprintf(fd, MSGSTR(TRACE_184,"AS restriction lists:\n"));
    for (tmp_list = sendas; tmp_list; tmp_list = tmp_list->next) {
      fprintf(fd, "\tAS %d %s AS(s):",
                  tmp_list->as,
                  tmp_list->flags & AS_DONOTSEND ? MSGSTR(TRACE_186,"Does not receive") : MSGSTR(TRACE_187,"receives"));
      for (tmp_entry = tmp_list->as_ptr; tmp_entry; tmp_entry = tmp_entry->next) {
        fprintf(fd, " %d", tmp_entry->as);
      }
      if (tmp_list == my_aslist) {
        fprintf(fd, "*\n");
      } else {
        fprintf(fd, "\n");
      }
    }
    fprintf(fd, "\n");
  }
  /*
   *	Dump the validAS list
   */
  if (validas) {
    struct as_valid *tmp_valid;

    fprintf(fd, MSGSTR(TRACE_192,"AS validation:\n"));
    for (tmp_valid = validas; tmp_valid; tmp_valid = tmp_valid->next) {
      fprintf(fd, "\tvalidAS %s AS %d metric %d\n",
        inet_ntoa(tmp_valid->dst), tmp_valid->as, tmp_valid->metric);
    }
    fprintf(fd, "\n");
  }
  /*
   *	Dump the list of martian networks
   */
  fprintf(fd, MSGSTR(TRACE_195,"Martian networks:\n\t"));
  cnt = 0;
  for (mptr = martians; mptr; mptr = mptr->next) {
    cp = inet_ntoa(mptr->destnet);
    if ( (cnt+strlen(cp)+1) > 64 ) {
      fprintf(fd, "\n\t");
      cnt = 0;
    }
    cnt += strlen(cp) + 1;
    fprintf(fd, "%s ", cp);
  }
  fprintf(fd, "\n");
#ifdef	notdef
  fprintf(fd, "\n-----------------------------------------------------------\n");
#endif	notdef
  /*
   * Dump all the routing information
   */
  fprintf(fd, MSGSTR(TRACE_200,"\n\nRouting Tables:\nEntries:\t%d\n"), n_routes);
  fprintf(fd, MSGSTR(TRACE_201,"\tUnits:\n\t\tInternal metric:\tmilliseconds\n\t\tAge:\t\t\tseconds\n\n"));
onemoretime:
  for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
    for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
      fprintf(fd,MSGSTR(TRACE_202,"\tDestination:       %s\n"), inet_ntoa(sock_inaddr(&rt->rt_dst)));
      if (rt->rt_as != mysystem) {
        fprintf(fd,MSGSTR(TRACE_203,"\tAutonomous System: %d\n"), rt->rt_as);
      }
      fprintf(fd,MSGSTR(TRACE_204,"\tGateway:           %s\n"), inet_ntoa(sock_inaddr(&rt->rt_router)));
      fprintf(fd,MSGSTR(TRACE_205,"\tInterface:         %s\n"), rt->rt_ifp->int_name);
      fprintf(fd,MSGSTR(TRACE_206,"\tAge:               %d\n"), rt->rt_timer);
      ch = MSGSTR(TRACE_207,"Unknown");
      for (p = protobits; p->t_bits > (u_int)0; p++) {
        if ( (rt->rt_proto == p->t_bits) ) {
          ch = p->t_name;
          break;
        }
      }
      fprintf(fd, MSGSTR(TRACE_208,"\tProtocol:          %s\n"), ch);
      fprintf(fd, MSGSTR(TRACE_209,"\tMetric:            %d\n"), rt->rt_metric);
      if ( rt->rt_state & RTS_EXTERIOR ) {
        fprintf(fd, MSGSTR(TRACE_210,"\tExterior Metric:   %d\n"), rt->rt_metric_exterior);
      }
      fprintf(fd, MSGSTR(TRACE_211,"\tFlags:             <"));
      cp = "%s";
      for (first = 1, p = flagbits; p->t_bits > (u_int)0; p++) {
        if ((rt->rt_flags & p->t_bits) == 0)
          continue;
        fprintf(fd, cp, p->t_name);
        if (first) {
          cp = "|%s";
          first = 0;
        }
      }
      fprintf(fd, ">\n");
      fprintf(fd, MSGSTR(TRACE_215,"\tState:             <"));
      cp = "%s";
      for (first = 1, p = statebits; p->t_bits > (u_int)0; p++) {
        if ((rt->rt_state & p->t_bits) == 0)
          continue;
        fprintf(fd, cp, p->t_name);
        if (first) {
          cp = "|%s";
          first = 0;
        }
      }
      fprintf(fd, ">\n");
      fprintf(fd, MSGSTR(TRACE_219,"\tFrom Protocol:     <"));
      cp = "%s";
      for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
        if ((rt->rt_fromproto & p->t_bits) == 0)
          continue;
        fprintf(fd, cp, p->t_name);
        if (first) {
          cp = "|%s";
          first = 0;
        }
      }
      fprintf(fd, ">\n");
      if (rt->rt_proto & RTPROTO_HELLO) {
        fprintf(fd, MSGSTR(TRACE_223,"\tMinimum HELLO time delay in last %d updates: %d\n"),
                       HWINSIZE, rt->rt_hwindow.h_min);
        fprintf(fd, MSGSTR(TRACE_224,"\tLast %d HELLO time delays:\n\t\t"), HELLO_REPORT);
        cnt = 0;
        ind = rt->rt_hwindow.h_index;
        while (cnt < HELLO_REPORT) {
          fprintf(fd, "%d ", rt->rt_hwindow.h_win[ind]);
          cnt++;
          ind++;
          if (ind >= HWINSIZE)
            ind = 0;
        }
        fprintf(fd, "\n");
      }
      if (rt->rt_announcelist || rt->rt_noannouncelist || rt->rt_listenlist || rt->rt_srclisten ) {
        fprintf(fd, MSGSTR(TRACE_227,"\tRestrictions:\n"));
        if (rt->rt_announcelist != NULL) {
          fprintf(fd, MSGSTR(TRACE_228,"\t\tANNOUNCED via <"));
          cp = "%s";
          for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
             if ((rt->rt_announcelist->rproto & p->t_bits) == 0) {
               continue;
             }
             fprintf(fd, cp, p->t_name);
             if (first) {
               cp = "|%s";
               first = 0;
             }
           }
           fprintf(fd, "> ");
           if (rt->rt_announcelist->rproto & RTPROTO_EGP) {
             fprintf(fd, "EGP metric %d ", rt->rt_announcelist->regpmetric);
           }
           cntr = 0;
           if (rt->rt_announcelist->rintf[cntr] == 0) {
             fprintf(fd, "OUT ALL INTERFACES\n");
           } else {
             fprintf(fd, MSGSTR(TRACE_234,"OUT INTERFACES:\n"));
             while ((rt->rt_announcelist->rintf[cntr] != 0) || (cntr >= MAXINTERFACE)) {
               fprintf(fd, "\t\t\t\t");
               tmpos.s_addr = rt->rt_announcelist->rintf[cntr];
               fprintf(fd, "%s\n", inet_ntoa(tmpos));
               cntr++;
             }
           }
        }
        if (rt->rt_noannouncelist != NULL) {
          fprintf(fd, MSGSTR(TRACE_237,"\t\tNOT ANNOUNCED via <"));
          cp = "%s";
          for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
            if ((rt->rt_noannouncelist->rproto & p->t_bits) == 0) {
               continue;
            }
             fprintf(fd, cp, p->t_name);
             if (first) {
               cp = "|%s";
               first = 0;
             }
           }
           fprintf(fd, "> ");
           if (rt->rt_noannouncelist->rproto & RTPROTO_EGP) {
             fprintf(fd, "EGP metric %d ", rt->rt_noannouncelist->regpmetric);
           }
           cntr = 0;
           if (rt->rt_noannouncelist->rintf[cntr] == 0) {
             fprintf(fd, MSGSTR(TRACE_233,"OUT ALL INTERFACES\n"));
           } else {
             fprintf(fd, MSGSTR(TRACE_234,"OUT INTERFACES:\n"));
             while ((rt->rt_noannouncelist->rintf[cntr] != 0) || (cntr >= MAXINTERFACE)) {
               fprintf(fd, "\t\t\t\t");
               tmpos.s_addr = rt->rt_noannouncelist->rintf[cntr];
               fprintf(fd, "%s\n", inet_ntoa(tmpos));
               cntr++;
             }
           }
        }
        if (rt->rt_listenlist != NULL) {
          fprintf(fd, MSGSTR(TRACE_246,"\t\tNOTLISTENEDTO via <"));
          cp = "%s";
          for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
            if ((rt->rt_listenlist->rproto & p->t_bits) == 0) {
              continue;
            }
            fprintf(fd, cp, p->t_name);
            if (first) {
              cp = "|%s";
              first = 0;
            }
          }
          fprintf(fd, "> ");
          if (rt->rt_listenlist->rproto & RTPROTO_EGP) {
            fprintf(fd, "EGP metric %d ", rt->rt_listenlist->regpmetric);
          }
          cntr = 0;
          if (rt->rt_listenlist->rintf[cntr] == 0) {
            fprintf(fd, MSGSTR(TRACE_251,"FROM ALL INTERFACES\n"));
          } else {
            fprintf(fd, MSGSTR(TRACE_252,"FROM INTERFACES:\n"));
            while ((rt->rt_listenlist->rintf[cntr] != 0) || (cntr >= MAXINTERFACE)) {
              fprintf(fd, "\t\t\t\t");
              tmpos.s_addr = rt->rt_listenlist->rintf[cntr];
              fprintf(fd, "%s\n", inet_ntoa(tmpos));
              cntr++;
            }
          }
        }
        if (rt->rt_srclisten != NULL) {
          fprintf(fd, MSGSTR(TRACE_255,"\t\tLISTENEDTO via <"));
          cp = "%s";
          for (first = 1, p = protobits; p->t_bits > (u_int)0; p++) {
            if ((rt->rt_srclisten->rproto & p->t_bits) == 0) {
               continue;
            }
            fprintf(fd, cp, p->t_name);
            if (first) {
              cp = "|%s";
              first = 0;
            }
          }
          fprintf(fd, "> ");
          if (rt->rt_srclisten->rproto & RTPROTO_EGP) {
            fprintf(fd, "EGP metric %d ", rt->rt_srclisten->regpmetric);
          }
          cntr = 0;
          if (rt->rt_srclisten->rintf[cntr] == 0) {
            fprintf(fd, MSGSTR(TRACE_260,"FROM ALL GATEWAYS\n"));
          } else {
            fprintf(fd, MSGSTR(TRACE_261,"FROM GATEWAYS:\n"));
            while ((rt->rt_srclisten->rintf[cntr] != 0) || (cntr >= MAXINTERFACE)) {
              fprintf(fd, "\t\t\t\t");
              tmpos.s_addr = rt->rt_srclisten->rintf[cntr];
              fprintf(fd, "%s\n", inet_ntoa(tmpos));
              cntr++;
            }
          }
        }
      }
      fprintf(fd, "\n");
    }
  }
  if (doinghost) {
    base = nethash;
    doinghost = 0;
    goto onemoretime;
  }
  (void) fflush(fd);
  (void) fclose(fd);
  syslog(LOG_NOTICE, MSGSTR(TRACE_265,"dumpinfo: dump completed to %s"), DUMPFILE);
  TRACE_TRC(MSGSTR(TRACE_266,"dumpinfo: dump completed to %s at %s"), DUMPFILE, strtime);
}

#ifdef  NSS
gated_trace(sock, pkt, size, src, dst)
int     sock;
char    *pkt;
int     size;
struct sockaddr_in      src, dst;
{
  bzero((char *)&src, sizeof(src));
  bzero((char *)&dst, sizeof(dst));

  if (tracing & TR_EGP) {
    tracercv(sock, IPPROTO_EGP, pkt, size, &src);
  }
}
#endif  NSS

#ifdef	EGP_UPDATE_FORMAT
/*
 *	Format an EGP network update packet
 */

NR_format(nr, nr_length)
struct egpnr *nr;
int	nr_length;
{
	int gateways, distances, networks;
	int distance, t_gateways;
	int shared_net_class, net_class;
	char class;
	u_char *nr_ptr, *nr_end;
	struct in_addr gateway, network;

	shared_net_class = gd_inet_class((u_char *)&nr->en_net);
	bcopy((char *) &nr->en_net, (char *)&gateway, shared_net_class);
	nr_ptr = (u_char *)nr + sizeof(struct egpnr);
	nr_end = (u_char *)nr + nr_length;
	printf(MSGSTR(TRACE_267,"net %s (%c) - %d interior gateways, %d exterior gateways\n"),
		inet_ntoa(nr->en_net), 'A' - 1 + shared_net_class,
		nr->en_igw, nr->en_egw);
	t_gateways = nr->en_igw + nr->en_egw;
	for ( gateways = 0; gateways < t_gateways; gateways++) {
		bcopy((char *)nr_ptr, (char *)&gateway + shared_net_class, 4 - shared_net_class);
		nr_ptr += 4 - shared_net_class;
		distances = (u_char) *nr_ptr;
		nr_ptr++;
		printf(MSGSTR(TRACE_268,"\t%s gateway %s, %d distances\n"),
			gateways < nr->en_igw ? MSGSTR(TRACE_269,"interior") : MSGSTR(TRACE_270,"exterior"), 
			inet_ntoa(gateway), distances);
		for ( ; distances; distances-- ) {
			distance = (u_char) *nr_ptr;
			nr_ptr++;
			networks = (u_char) *nr_ptr;
			nr_ptr++;
			printf(MSGSTR(TRACE_271,"\t\tdistance %d, %d networks\n"),
				distance, networks);
			for ( ; networks; networks-- ) {
				bzero((char *)&network, sizeof(struct in_addr));
				if ( (net_class = gd_inet_class(nr_ptr)) == 0 ) {
					net_class = CLAC;
					class = '?';
				} else {
					class = 'A' - 1 + net_class;
				}
				bcopy((char *)nr_ptr, (char *)&network, net_class);
				nr_ptr += net_class;
				printf("\t\t\t(%c) %s\n",
					class, inet_ntoa(network));
				if ( nr_ptr > nr_end ) {
					printf(MSGSTR(TRACE_273,"\tpacket smashed\n"));
					return;
				}
			}
		}
	}
	return;
}
#endif	EGP_UPDATE_FORMAT
