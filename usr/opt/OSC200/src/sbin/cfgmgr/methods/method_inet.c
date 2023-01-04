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
static char	*sccsid = "@(#)$RCSfile: method_inet.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/11/06 15:13:04 $";
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

#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>
#include <netinet/inet_config.h>
#include "cm.h"

#define	INET_IPGATEWAY		"Inet_IP_Gateway"
#define	INET_IPFORWARD		"Inet_IP_Forward"
#define	INET_IPSENDICMP		"Inet_IP_SendICMP"
#define	INET_IPBROADCAST	"Inet_IP_Broadcast"
#define	INET_IPSRCROUTING	"Inet_IP_Routing"
#define	INET_IPLOCALSUBNETS	"Inet_IP_Local"
#define	INET_IPMAXQLEN		"Inet_IP_Qlength"

/*
 *	Local BSS
 */
int		INET_configured;
int		INET_loaded;
kmod_id_t	INET_id;

inet_config_t	outadm;
inet_config_t	inadm;

/*
 *
 *	Name:		INET_method()
 *	Description:	Inet Configuration Method
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 */
int
INET_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop,
	char *opts )
{
	int	rc;

	rc = 0;
	if (op & CM_OP_LOAD) {
		rc = INET_method_load(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_ERR, MSG_LOADED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = INET_method_configure(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_ERR, MSG_CONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = INET_method_unconfigure(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_ERR, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNLOAD) {
		rc = INET_method_unload(logp, entry);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_ERR, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 *
 */
int
INET_method_load( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (INET_loaded)
		return(KMOD_LOAD_L_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_load(entry, &INET_id)) != 0)
		return(rc);
#endif
	INET_loaded = 1;
	return(0);
}


/*
 *
 */
int
INET_method_unload( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!INET_loaded)
		return(KMOD_UNLOAD_L_EEXIST);
	if (INET_configured)
		return(KMOD_UNLOAD_C_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_unload(INET_id)) != 0)
		return(rc);
#endif
	INET_id = LDR_NULL_MODULE;
	INET_loaded = 0;
	return(0);
}


/*
 *
 */
int
INET_method_configure( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!INET_loaded)
		return(KMOD_CONFIG_L_EEXIST);
	if (INET_configured)
		return(KMOD_CONFIG_C_EBUSY);
	if (INET_inadm(logp, entry))
		return(KMOD_EINVAL);
#ifndef TEST
        if ((rc=cm_kls_call(INET_id, SYSCONFIG_CONFIGURE, 
			&inadm, sizeof(inet_config_t),
			&outadm, sizeof(inet_config_t))) != 0)
		return(rc);
#else
	bcopy(&inadm, &outadm, sizeof(inet_config_t));
#endif
	INET_prtcfg(logp, entry);
	INET_configured = 1;
	return(0);
}


/*
 *
 */
int
INET_method_unconfigure( cm_log_t * logp, ENT_t entry )
{
	int	rc;

	if (!INET_loaded)
		return(KMOD_UNCONFIG_L_EEXIST);
	if (!INET_configured)
		return(KMOD_UNCONFIG_C_EEXIST);
#ifndef TEST
	if ((rc=cm_kls_call(INET_id, SYSCONFIG_UNCONFIGURE, NULL, 0, 
			NULL, 0)) != 0)
		return(rc);
#endif
	INET_configured = 0;
	return(0);
}

/*
 *
 */
int
INET_inadm( cm_log_t * logp, ENT_t entry )
{
	int	i;

	inadm.version = INET_CONFIG_VERSION_1;
	inadm.flags = IN_USEVALUE;	/* IN_USEVALUE || IN_USEDEFAULTS */

	inadm.inetprintfs = 0;		/* If configured, enable printfs (0) */
	inadm.useloopback = 1;		/* Use loopback for own packets (1) */

	if (dbattr_flag(entry, INET_IPGATEWAY, FALSE) == FALSE)
		inadm.ipgateway = 0;		/* Configure as gateway (0) */
	else
		inadm.ipgateway = 1;
	if (dbattr_flag(entry, INET_IPFORWARD, FALSE) == FALSE)
		inadm.ipforwarding = 0;		/* Act as gateway (0) */
	else
		inadm.ipforwarding = 1;
	if (dbattr_flag(entry, INET_IPSENDICMP, TRUE) == TRUE)
		inadm.ipsendredirects = 1;	/* Send ICMP redirects (1) */
	else
		inadm.ipsendredirects = 0;
	if (dbattr_flag(entry, INET_IPBROADCAST, FALSE) == FALSE)
		inadm.ipdirected_broadcast = 0;	/* Accept unique B'casts  (0) */
	else
		inadm.ipdirected_broadcast = 1;
	if (dbattr_flag(entry, INET_IPSRCROUTING, TRUE) == TRUE)
		inadm.ipsrcroute = 1;		/* Enable host src routing(1) */
	else
		inadm.ipsrcroute = 0;
	if (dbattr_flag(entry, INET_IPLOCALSUBNETS, TRUE) == TRUE)
		inadm.subnetsarelocal = 1;	/* Sub appear connected(1) */
	else
		inadm.subnetsarelocal = 0;
	if ((i=dbattr_num(entry, INET_IPMAXQLEN, 50)) < 50)
		inadm.ipqmaxlen = 50;		/* IP input queue length(50) */
	else
		inadm.ipqmaxlen = i;

	inadm.tcpttl = 60;		/* Default time to live (60) */
	inadm.tcpmssdflt = 536;		/* Default max segsize (536) */
	inadm.tcprttdflt = 3;		/* Default initial rtt (3) */
	inadm.tcpkeepidle = 7200;	/* Keepalive idle timer (7200) */
	inadm.tcpkeepintvl = 75;	/* Keepalive interval (75) */
	inadm.tcpcompat_42 = 1;		/* BSD4.2 compat keepalive/urg (1) */
	inadm.tcprexmtthresh = 3;	/* Retransmit threshold (3) */
	inadm.tcpconsdebug = 0;		/* If configured, debug printfs (0) */
	inadm.tcp_sendspace = 4096;	/* Default send queue (4096) */
	inadm.tcp_recvspace = 4096;	/* Default receive queue (4096) */
	
	inadm.udpttl = 60;		/* Default time to live (60) */
	inadm.udpcksum = 1;		/* Enable checksumming (1) */
	inadm.udp_sendspace = 4096;	/* Default send queue (4096) */
	inadm.udp_recvspace = 4096;	/* Default receive queue (4096) */
	
	inadm.arpkillc = 1200;		/* Time to remove completed (1200) */
	inadm.arpkilli = 180;		/* Time to remove incomplete (180) */
	inadm.arprefresh = 120;		/* Time to refresh entry (120) */
	inadm.arphold = 5;		/* Time to hold packet (5) */
	inadm.arplost = 3;		/* Count to broadcast refresh (3) */
	inadm.arpdead = 6;		/* Count to assume dead (6) */
	inadm.arpqmaxlen = 50;		/* Length of ARP input queue (50) */
	inadm.arptabbsiz = 9;		/* Table bucket size (16/9 gw/!gw) */
	inadm.arptabnb = 19;		/* Number of buckets (37/19 gw/!gw) */

	return(0);
}


/*
 *
 */
int
INET_prtcfg( cm_log_t * logp, ENT_t entry )
{
	char  *	name = AFentname(entry);

	if (outadm.version != INET_CONFIG_VERSION_1)
		return(-1);

	if (outadm.ipgateway == 1)
		cm_log(logp, LOG_INFO, "%s: Configure as gateway\n", name);
	if (outadm.ipforwarding== 1)
		cm_log(logp, LOG_INFO, "%s: Acting as gateway\n", name);
	if (outadm.ipsendredirects == 1)
		cm_log(logp, LOG_INFO, "%s: Sending ICMP redirects\n", name);
	if (outadm.ipdirected_broadcast == 0)
		cm_log(logp, LOG_INFO, "%s: Accept unique B'casts\n", name);
	if (outadm.ipsrcroute == 1)
		cm_log(logp, LOG_INFO, "%s: Enable host src routing\n", name);
	if (outadm.subnetsarelocal == 1)
		cm_log(logp, LOG_INFO, "%s: Subnets appear connected\n", name);
	if (outadm.ipqmaxlen != 50)
		cm_log(logp, LOG_INFO, "%s: IP input queue length = %d\n", name,
			outadm.ipqmaxlen);

	return(0);
}

