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
static char	*sccsid = "@(#)$RCSfile: snmp.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/10 15:48:07 $";
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

#if	defined(AGENT_SNMP) || defined(AGENT_SGMP)
#include "include.h"
#include "snmp.h"
/* #include <snmp.h> */

static int snmp_next;
#endif	defined(AGENT_SNMP) || defined(AGENT_SGMP)

#ifdef	AGENT_SNMP

int sysDescr();
int ipRouteMetric();
int ipRouteNextHop();
int ipRouteType();
int ipRouteProto();
int ipRouteAge();
int egpInMsgs();
int egpInErrors();
int egpOutMsgs();
int egpOutErrors();
int egpNeighState();
int egpNeighAddr();
int egpNeighAs();
int egpNeighInMsgs();
int egpNeighInErrs();
int egpNeighOutMsgs();
int egpNeighOutErrs();
int egpNeighInErrMsgs();
int egpNeighOutErrMsgs();
int egpNeighStateUps();
int egpNeighStateDowns();
int egpNeighIntervalHello();
int egpNeighIntervalPoll();
int egpNeighMode();
int egpNeighEventTrigger();
int egpAs();
struct egpngh *snmp_lookup_ngp();
static u_short agentport;

static struct mibtbl snmptbl[] = {
	0, { 1, 3, 6, 1, 2, 1, 4, 21, 1, 3 }, ipRouteMetric,
	0, { 1, 3, 6, 1, 2, 1, 4, 21, 1, 7 }, ipRouteNextHop, 
	0, { 1, 3, 6, 1, 2, 1, 4, 21, 1, 8 }, ipRouteType, 
	0, { 1, 3, 6, 1, 2, 1, 4, 21, 1, 9 }, ipRouteProto,
	0, { 1, 3, 6, 1, 2, 1, 4, 21, 1, 10 }, ipRouteAge,
	0, { 1, 3, 6, 1, 2, 1, 8, 1 }, egpInMsgs,
	0, { 1, 3, 6, 1, 2, 1, 8, 2 }, egpInErrors,
	0, { 1, 3, 6, 1, 2, 1, 8, 3 }, egpOutMsgs,
	0, { 1, 3, 6, 1, 2, 1, 8, 4 }, egpOutErrors,
	0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 1 }, egpNeighState, 
	0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 2 }, egpNeighAddr, 
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 3 }, egpNeighAs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 4 }, egpNeighInMsgs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 5 }, egpNeighInErrs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 6 }, egpNeighOutMsgs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 7 }, egpNeighOutErrs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 8 }, egpNeighInErrMsgs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 9 }, egpNeighOutErrMsgs,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 10 }, egpNeighStateUps,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 11 }, egpNeighStateDowns,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 12 }, egpNeighIntervalHello,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 13 }, egpNeighIntervalPoll,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 14 }, egpNeighMode,
        0, { 1, 3, 6, 1, 2, 1, 8, 5, 1, 15 }, egpNeighEventTrigger,
        0, { 1, 3, 6, 1, 2, 1, 8, 6 }, egpAs,
	0, { 0 } , 0
};

/*
 *	Process an incoming request from SNMPD.
 */
snmpin(from, size, pkt)
        struct sockaddr *from;
        int size;
        char *pkt;
{
        struct sockaddr_in *sin_from = (struct sockaddr_in *)from;
	
	snmp_in_pkt(sin_from, snmp_socket, size, pkt, snmptbl);
        return;
}


/*
 *  Register all of our supported variables with SNMPD.
 */
register_snmp_vars()
{
	
	snmp_register(snmp_socket, agentport, snmptbl);
}

#define	LOOPBACKHOST	LOOPBACKNET | (1)

struct rt_entry *
snmp_lookup_rt(src, dst)
char *src, *dst;
{
	struct sockaddr_in dest;
	static struct rt_entry *loopback_rt = (struct rt_entry *) 0;
	struct rt_entry *rt = 0;	

	bzero((char *)&dest, sizeof(dest));
	dest.sin_family = AF_INET;
	bcopy(src, (char *)&dest.sin_addr, sizeof(dest.sin_addr));

	if ((dest.sin_addr.s_addr == htonl(LOOPBACKNET)) || (dest.sin_addr.s_addr == htonl(LOOPBACKHOST))) {
		if (!loopback_rt) {
			loopback_rt = (struct rt_entry *) calloc(1, sizeof(*loopback_rt));
			if (!loopback_rt) {
				syslog(LOG_ERR,"snmp_lookup_rt: calloc");
				quit();
			}
			loopback_rt->rt_metric = 0;
			loopback_rt->rt_proto = RTPROTO_DIRECT;
			loopback_rt->rt_timer = 0;
			sock_inaddr(&loopback_rt->rt_router).s_addr = htonl(LOOPBACKHOST);
		}
		rt = loopback_rt;
	} else if (!(rt = rt_lookup((int)INTERIOR|EXTERIOR, &dest)) ) {
/*syslog(LOG_NOTICE,"rt_lookup: sin_addr = %s", inet_ntoa(dest.sin_addr));*/
		rt = rt_lookup((int)HOSTTABLE, &dest);
	}

	return(rt);
}



int ipRouteMetric(src, dst)
char *src, *dst;
{
	int metric;
	struct sockaddr_in reqdst;
	struct rt_entry *grte;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));
	if (!(grte = snmp_lookup_rt(src, dst)) ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = INT;
	*dst++ = sizeof(int);
	switch (grte->rt_proto) {
		case RTPROTO_RIP:
		case RTPROTO_DIRECT:
		case RTPROTO_DEFAULT:
		case RTPROTO_REDIRECT:
			metric = mapmetric(HELLO_TO_RIP, grte->rt_metric);
			break;
		case RTPROTO_EGP:
		case RTPROTO_KERNEL:
			metric = mapmetric(HELLO_TO_EGP, grte->rt_metric);
			break;
		case RTPROTO_HELLO:
			metric = grte->rt_metric;
			break;
		default:
			metric = mapmetric(HELLO_TO_RIP, grte->rt_metric);
			break;
	}
	bcopy((char *)&metric, dst, sizeof(int));
	return(sizeof(int) + 2);
}


int ipRouteNextHop(src, dst)
char *src, *dst;
{
	struct sockaddr_in reqdst;
	struct rt_entry *grte;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));
	if (!(grte = snmp_lookup_rt(src, dst)) ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = IPADD;
	*dst++ = sizeof (u_int);
	bcopy((char *)&sock_inaddr(&grte->rt_router).s_addr, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}


int ipRouteType(src, dst)
char *src, *dst;
{
	int type;
	struct sockaddr_in reqdst;
	struct rt_entry *grte;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));
	if (!(grte = snmp_lookup_rt(src, dst)) ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	if (grte->rt_metric >= DELAY_INFINITY) {
		type = 2;
	} else {
		switch (grte->rt_proto) {
			case RTPROTO_DIRECT:
				type = 3;
				break;
			default:
				type = 4;
		}
	}

	*dst++ = INT;
	*dst++ = sizeof(int);
	bcopy((char *)&type, dst, sizeof(int));
	return(sizeof(int) + 2);
}


int ipRouteProto(src, dst)
char *src, *dst;
{
	int proto;
	struct sockaddr_in reqdst;
	struct rt_entry *grte;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));
	if (!(grte = snmp_lookup_rt(src, dst)) ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	if (grte->rt_state & RTS_STATIC) {
		proto = 2;
	} else {
		switch (grte->rt_proto) {
			case RTPROTO_RIP:
				proto = 8;
				break;
			case RTPROTO_HELLO:
				proto = 7;
				break;
			case RTPROTO_EGP:
				proto = 5;
				break;
			case RTPROTO_DIRECT:
				proto = 1;
				break;
			case RTPROTO_REDIRECT:
				proto = 4;
				break;
			case RTPROTO_DEFAULT:
				proto = 1;
				break;
			default:
				proto = 1;
		}
	}

	*dst++ = INT;
	*dst++ = sizeof(int);
	bcopy((char *)&proto, dst, sizeof(int));
	return(sizeof(int) + 2);
}


int ipRouteAge(src, dst)
char *src, *dst;
{
	int age;
	struct sockaddr_in reqdst;
	struct rt_entry *grte;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));
	if (!(grte = snmp_lookup_rt(src, dst)) ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	age = grte->rt_timer;
	*dst++ = INT;
	*dst++ = sizeof(int);
	bcopy((char *)&age, dst, sizeof(int));
	return(sizeof(int) + 2);
}


int egpInMsgs(src, dst)
char *src, *dst;
{
	u_int msgs;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = CNTR;
	*dst++ = sizeof(u_int);
	msgs = egp_stats.inmsgs;
	bcopy((char *)&msgs, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}


int egpInErrors(src, dst)
char *src, *dst;
{
	u_int errors;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = CNTR;
	*dst++ = sizeof(u_int);
	errors = egp_stats.inerrors;
	bcopy((char *)&errors, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}


int egpOutMsgs(src, dst)
char *src, *dst;
{
	u_int msgs;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = CNTR;
	*dst++ = sizeof(u_int);
	msgs = egp_stats.outmsgs;
	bcopy((char *)&msgs, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}


int egpOutErrors(src, dst)
char *src, *dst;
{
	u_int errors;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = CNTR;
	*dst++ = sizeof(u_int);
	errors = egp_stats.outerrors;
	bcopy((char *)&errors, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}



int egpNeighState(src, dst)
char *src, *dst;
{
	int state;
	struct egpngh *ngp;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = INT;
	*dst++ = sizeof(int);
	state = ngp->ng_state + 1;
	bcopy((char *)&state, dst, sizeof(int));
	return(sizeof(int) + 2);
}


int egpNeighAddr(src, dst)
char *src, *dst;
{
	int addr;
	struct egpngh *ngp;
	
	if (!(doing_egp)) {
		*--dst = AGENT_ERR;
		return(1);
	}
	if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
		*--dst = AGENT_ERR;
		return(1);
	}
	*dst++ = IPADD;
	*dst++ = sizeof(u_int);
	addr = ngp->ng_addr.s_addr;
	bcopy((char *)&addr, dst, sizeof(u_int));
	return(sizeof(u_int) + 2);
}


struct egpngh *
snmp_lookup_ngp(src)
	char *src;
{
	struct sockaddr_in reqdst;
	struct egpngh *ngp = egpngh;
	
	bzero((char *)&reqdst, sizeof(reqdst));
	reqdst.sin_family = AF_INET;
	bcopy(src, (char *)&reqdst.sin_addr.s_addr, sizeof(u_int));

	if (reqdst.sin_addr.s_addr != (u_int)DEFAULTNET) {
		for (; ngp; ngp = ngp->ng_next) {
			if (ngp->ng_addr.s_addr == reqdst.sin_addr.s_addr) {
				break;
			}
		}
	} 
	if (ngp && snmp_next) {
		ngp = ngp->ng_next;
	}
	return(ngp);
}
#endif	AGENT_SNMP


/*
 *	Routines common to both SGMP and SNMP
 */
 
#if	defined(AGENT_SGMP) || defined(AGENT_SNMP)

/*
 *  Process an incoming request from SNMPD.  Speed is of the essence here.
 *  Not elegance.
 */
int snmp_in_pkt(sin_from, socket, size, pkt, objtbl)
	struct sockaddr_in *sin_from;
        int socket, size;
        char *pkt;
        struct mibtbl objtbl[];
{
        char agntreqpkt[SNMPMAXPKT];
	char *ptr = pkt;
	char *ptr1 = agntreqpkt;
	int rspsize;
	struct mibtbl *mib_ptr;

        TRACE_SNMPPKT(SNMP RECV, sin_from, pkt, size);
        snmp_next = 0;
	switch (*ptr) {
		case AGENT_REG:
		case AGENT_RSP:
		case AGENT_ERR:
			syslog(LOG_ERR, "snmp_in_pkt: unexpected AGENT pkt type");
			TRACE_TRC("snmp_in_pkt: unexpected AGENT pkt type\n");
			return;
		case AGENT_REQN:
			snmp_next = 1;
		case AGENT_REQ:
			ptr += 2;
			*ptr1++ = AGENT_RSP;
			rspsize = 1;
			for (mib_ptr = objtbl; mib_ptr->function; mib_ptr++) {
				if (bcmp(ptr, mib_ptr->object, mib_ptr->length) == 0) {
					break;
				}
			}
			if (mib_ptr->function) {
				ptr += mib_ptr->length;
				rspsize += (mib_ptr->function)(ptr, ptr1);
			} else {
				TRACE_SNMP("error\n");
				syslog(LOG_NOTICE, "snmp_in_pkt: unknown object id");
				agntreqpkt[0] = AGENT_ERR;
				rspsize = 1;
			}
			break;
		default:
			syslog(LOG_ERR, "snmp_in_pkt: invalid AGENT pkt type");
			TRACE_SNMP("snmp_in_pkt: invalid AGENT pkt type\n");
			agntreqpkt[0] = AGENT_ERR;
			rspsize = 1;
			break;
	} /* switch */

	if (rspsize) {	
	        TRACE_SNMPPKT(SNMP SEND, sin_from, agntreqpkt, rspsize);
        	if (sendto(socket, agntreqpkt, rspsize, 0, (struct sockaddr *)sin_from, sizeof(struct sockaddr_in)) < 0) {
        		p_error("snmp_in: sendto() error");
			syslog(LOG_NOTICE, "snmp_in_pkt: sendto() error");
	        }
	}
	return;
}


/*
 *  Register all of our supported variables with SNMPD.
 */
snmp_register(socket, port, objtbl)
	int socket;
	u_short port;
	struct mibtbl objtbl[];
{
        struct sockaddr_in dst;
        char agntpkt[MAXPACKETSIZE];
	char *p = agntpkt;
	int asize, len;
	struct mibtbl *mib_ptr;

	*p++ = 0x01;
	asize = 1;

	for (mib_ptr = objtbl; mib_ptr->function; mib_ptr++) {
		if (mib_ptr->length == 0) {
			mib_ptr->length = strlen(mib_ptr->object);
		}
/*
 * comment out the following piece of code for use with the common agent.
 * It does not need to have objects registered with it.
 *
		*p++ = mib_ptr->length;
		bcopy(mib_ptr->object, p, mib_ptr->length);
		p += mib_ptr->length;
		asize += mib_ptr->length + 1;
 * end of commented out code
 */
	}

/* 
 * comment out the following piece of code for use with the common agent.
 * It does not need to have objects registered with it.
 *
        bzero((char *)&dst, sizeof(struct sockaddr_in));
        dst.sin_family = AF_INET;
        dst.sin_port = port;
        dst.sin_addr.s_addr = inet_addr("127.0.0.1");

        TRACE_SNMPPKT(SNMP SEND, &dst, agntpkt, asize);
        if (sendto(socket, agntpkt, asize, 0, (struct sockaddr *)&dst, sizeof(struct sockaddr_in)) < 0) {
        	p_error("snmp_register: sendto() error");
        }
 * end of commented out code
 */
}


struct sockaddr_in snmpaddr;
int
snmp_init()
{
        int snmpinits;
	struct servent *dap;
	
	dap = getservbyname("snmp-rt", "udp");
	if (dap == NULL) {
	    syslog(LOG_NOTICE, "snmp_init: snmp-rt service not defined");
	    return(ERROR);
	}
        snmpaddr.sin_family = AF_INET;
        snmpaddr.sin_port = agentport = (u_short)dap->s_port;

        snmpinits = get_snmp_socket(AF_INET, SOCK_DGRAM, &snmpaddr);
        if (snmpinits < 0)
                return(ERROR);
        return(snmpinits);
}


/*
 *  Open up the SNMP socket.
 */
int
get_snmp_socket(domain, type, sin)
        int domain, type;
        struct sockaddr_in *sin;
{
        int snmpsocks, on = 1;

        if ((snmpsocks = socket(domain, type, 0)) < 0) {
                p_error("get_snmp_socket: socket");
                return (ERROR);
        }
#ifdef SO_RCVBUF
        on = 48*1024;
        if (setsockopt(snmpsocks,SOL_SOCKET,SO_RCVBUF,(char *)&on,sizeof(on))<0) {
                p_error("setsockopt SO_RCVBUF");
        }
#endif SO_RCVBUF
        if (bind(snmpsocks, sin, sizeof (*sin)) < 0) {
                p_error("get_snmp_socket: bind");
                (void) close(snmpsocks);
                return (ERROR);
        }
        return (snmpsocks);
}

int egpNeighAs(src, dst)
char *src, *dst;
{
        int as;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
	if (ngp->ng_flags & NG_ASIN)
           as = ngp->ng_asin;
	else 
	   if (ngp->ng_flags & NG_AS)
	        as = ngp->ng_as ;
	   else
		as = 0 ;
        bcopy((char *)&as, dst, sizeof(int));
        return(sizeof(int) + 2);

}

int egpNeighInMsgs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr =  -1;		/* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighInErrs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr = -1;              /* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);
}

int egpNeighOutMsgs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr = -1;              /* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighOutErrs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr = -1;              /* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighInErrMsgs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr = -1;              /* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighOutErrMsgs(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr = -1;              /* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighStateUps(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(int);
        addr = -1;			/* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighStateDowns(src, dst)
char *src, *dst;
{
        u_int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = CNTR;
        *dst++ = sizeof(u_int);
        addr =  -1;		/* undefined */
        bcopy((char *)&addr, dst, sizeof(u_int));
        return(sizeof(u_int) + 2);

}

int egpNeighIntervalHello(src, dst)
char *src, *dst;
{
        time_t addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
	
        addr = ngp->ng_hint;
        bcopy((char *)&addr, dst, sizeof(int));
        return(sizeof(int) + 2);

}

int egpNeighIntervalPoll(src, dst)
char *src, *dst;
{
        time_t addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
        addr = ngp->ng_spint;
        bcopy((char *)&addr, dst, sizeof(int));
        return(sizeof(int) + 2);

}

int egpNeighMode(src, dst)
char *src, *dst;
{
        int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
	if (ngp->ng_rcmd)
	    addr = 1 ;
	else
	    addr = 2 ;
        bcopy((char *)&addr, dst, sizeof(int));
        return(sizeof(int) + 2);

}

int egpNeighEventTrigger(src, dst)
char *src, *dst;
{
        int addr;
        struct egpngh *ngp;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        if ( (ngp = snmp_lookup_ngp(src)) == 0 ) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
        addr = -1;		/* undefined */
        bcopy((char *)&addr, dst, sizeof(int));
        return(sizeof(int) + 2);

}

int egpAs(src, dst)
char *src, *dst;
{
        int eas;

        if (!(doing_egp)) {
                *--dst = AGENT_ERR;
                return(1);
        }
        *dst++ = INT;
        *dst++ = sizeof(int);
        eas = mysystem;
        bcopy((char *)&eas, dst, sizeof(int));
        return(sizeof(int) + 2);

}      

#endif defined(AGENT_SNMP) || defined(AGENT_SGMP)
