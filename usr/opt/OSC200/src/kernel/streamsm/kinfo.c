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
static char	*sccsid = "@(#)$RCSfile: kinfo.c,v $ $Revision: 4.4.9.6 $ (DEC) $Date: 1993/12/21 20:47:24 $";
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

/*
 * 06-Jun-91 wca
 *	Added STREAMS kernel support
 * 07-Jun-91 wca
 *	. Fixed error cases in function switch statement
 *	  which did not break out of switch whenever 
 *	  requestor's buffer size was insufficient for
 *	  function request data size.
 *	. Got rid of 'buf' variable and just passed
 *	  cast pointer to mblk data block.
 * 24-Jun-91 walker
 *     Added ID support
 * 04-Nov-91 heather
 *     Corrected UDP InDatagrams and InErrors per MIB II (rfc1213)
 */

#include <sys/errno.h>
#include <sys/param.h>
#include <sys/user.h>           /* XXX before stream.h */
#include <sys/uio.h>
#include <streams/str_stream.h>
#include <streams/mi.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/sysconfig.h>
#include <sys/strkinfo.h>
#include <sys/secdefines.h>
#ifdef SEC_BASE
#include <sys/security.h>
#endif

extern struct arptab *arptab;
extern int arptab_size;
dev_t kinfodev;

#ifndef staticf
#define staticf static
#endif

#ifdef _NO_PROTO

staticf int	kinfo_close();
staticf int	kinfo_open();
staticf int	kinfo_rsrv();
staticf int	kinfo_wsrv();
staticf int	kinfo_wput();

#else

staticf int	kinfo_close(queue_t *, int, cred_t *);
staticf int	kinfo_open(queue_t *, int, int, int, cred_t *);
staticf int	kinfo_rsrv(queue_t * q);
staticf int	kinfo_wsrv(queue_t * q);
staticf int	kinfo_wput(queue_t * q, mblk_t *);

#endif

static struct module_info kinfo =  {
        KINFO_ID,         /* module ID number */
	"kinfo",          /* module name */
	0,                /* min pkt size accepted */
	INFPSZ,           /* max pkt size accepted */
	KINFO_HI_MARK,    /* hi-water mark, flow control */
	KINFO_LO_MARK     /* lo-water mark, flow contorl */
};

static struct qinit rinit = {
	nil(pfi_t), 			/* put procedure */
	nil(pfi_t), 			/* service procedure */
	kinfo_open, 			/* called every open or push */
	kinfo_close, 			/* called every close or pop */
	nil(pfi_t), 			/* admin - reserved */
	&kinfo,	               		/* information structure */
	(struct module_stat *)0		/* stats structure - unused */
};

static struct qinit winit = {
	kinfo_wput,  			/* put procedure */
	nil(pfi_t), 			/* service procedure */
	nil(pfi_t),			/* open procedure - unused */
	nil(pfi_t), 			/* close procedure - unused */
	nil(pfi_t), 			/* admin - reserved */
	&kinfo, 			/* info structure */
	(struct module_stat *)0		/* stats structure - unused */
};

struct streamtab kinfo_info = {
	&rinit, 		/* read queue init */
	&winit,		        /* write queue init */
	(struct qinit *)0,	/* read queue init - mux driver (unused) */
	(struct qinit *)0	/* write queue init - mux driver (unused) */
};

typedef struct iocblk *IOCP;

struct context {   /* per stream context structure */
    int inuse;     /* true if this instance is in use */
    int is_priv;   /* true if stream was opened by su */
};
static struct context kinfo_tbl[MAX_KINFO_STREAMS];

int
kinfo_configure(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t	op;
	str_config_t *	indata;
	size_t 		indatalen;
	str_config_t *	outdata;
	size_t  	outdatalen;
{
	struct streamadm sa;
	extern dev_t clonedev;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	if (indata != NULL && indatalen == sizeof(str_config_t)
			&& indata->sc_version == OSF_STREAMS_CONFIG_10)
		kinfodev = indata->sc_devnum;
	else
		kinfodev = NODEV;

	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= STR_IS_DEVICE | STR_SYSV4_OPEN;
	sa.sa_ttys		= nil(caddr_t);
	sa.sa_sync_level 	= SQLVL_QUEUE;
	sa.sa_sync_info		= nil(caddr_t);
	strcpy(sa.sa_name, 	"kinfo");

	if ((kinfodev = strmod_add(kinfodev, &kinfo_info, &sa)) == NODEV) {
		return ENODEV;
	}

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = makedev(major(clonedev), major(kinfodev));
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy(outdata->sc_sa_name, sa.sa_name);
	}

	return 0;
}



/*
 * ==========================
 * kinfo Open/close procedures.
 * ==========================
 */

/*
 * kinfoopen - Driver open put procedure.
 *
 * Inputs:
 *	q	= read queue pointer
 *	dev	= major/minor device number
 *	flag	= file open flag
 *	sflag	= STREAMS open flag (ie. CLONEOPEN)
 *
 * Outputs:
 *	None
 *
 * Return:
 *	If success, >= 0 minor device number to be used for context
 *	 for subsequent STREAMS I/O for this STREAM,
 *	Else, OPENFAIL - XXX u.u_error here is fake, see stream.h
 */
staticf int
kinfo_open (q, devp, flag, sflag, credp)
	queue_t	*	q;
	dev_t *		devp;
	int		flag;
	int		sflag;
        cred_t *        credp;
{
        int newminor;

	strlog(KINFO_ID, 
		0, 
		0, 
		SL_TRACE, 
		"kinfo: kinfo_open() - major = %d. minor = %d.",
		major(*devp),
		minor(*devp));

	if (sflag == CLONEOPEN) {
		for (newminor = 0; newminor < MAX_KINFO_STREAMS; newminor++) {
			if (kinfo_tbl[newminor].inuse == 0)
				break;
		}
	} else
		newminor = minor(*devp);
	*devp = makedev(major(*devp), newminor);

	if (newminor >= MAX_KINFO_STREAMS) {
		u.u_error = ETOOMANYREFS;
		return OPENFAIL;
	}

	if (kinfo_tbl[newminor].inuse != 0) {
		u.u_error = ETOOMANYREFS;
		return OPENFAIL;
	}

	kinfo_tbl[newminor].inuse = 1;
#if SEC_BASE
	if (!privileged(SEC_SYSATTR, 0)) 
	{
	    kinfo_tbl[newminor].is_priv = FALSE;
	}
#else
	if (suser(u.u_cred, &u.u_acflag))
	{
	    kinfo_tbl[newminor].is_priv = FALSE;
	}
#endif
	else
	{
	    kinfo_tbl[newminor].is_priv = TRUE;
	}

	q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t)&kinfo_tbl[newminor];

	return 0;
}

/*
 * kinfo_close - Driver close put procedure.
 */
staticf int
kinfo_close (q, flag, credp)
	queue_t	*	q;
        int             flag;
        cred_t  *       credp;
{
        /*
	 * Clear RD(q) and WR(q) private data pointers
	 */
        *q->q_ptr = 0;
        q->q_ptr = OTHERQ(q)->q_ptr = nil(caddr_t);

	return 0;
}

/*
 * kinfo_wput - Driver write-side put procedure.
 *           It validates the message type, perform flush operations,
 *           handles ioctl commands, and puts the message into the
 *           the write-side queue of this stream.
 */
staticf int
kinfo_wput (q, mp)
	queue_t	* 	q;
	mblk_t *	mp;
{
	IOCP		iocp;

	switch (mp->b_datap->db_type) 
	{
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, 0);
		if (*mp->b_rptr & FLUSHR) {
			flushq(RD(q), 0);
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		}
		break;
	case M_IOCTL:
		iocp = (IOCP)mp->b_rptr;

		/*
		 * check priv on all ioctl's BUT getting streams config.
		 */
		/*
		 * Following code is commented out for now.  When write/set
		 * functions are allowed, this need to be put back.
		 */

	       /*if ((iocp->ioc_cmd != KINFO_GET_STR_CFG) && 
		    !((struct context *)q->q_ptr)->is_priv)
		{
		        mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = EACCES;
			qreply(q, mp);
			return(0);
		} */
		
        	mp->b_datap->db_type = M_IOCACK;
		switch (iocp->ioc_cmd) 
		{
		case KINFO_GET_SYSTEM:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(system_blk))
				get_system((system_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_INTERFACES:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(if_blk))
				get_interfaces((if_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_ICMP:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(icmp_blk))
				get_icmp((icmp_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_TCP:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(tcp_blk))
				get_tcp((tcp_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_IP:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(ip_blk))
				get_ip((ip_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_IP_ROUTING:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(ip_routing_blk))
				get_ip_routing((ip_routing_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_AT:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(arp_blk))
				get_arp((arp_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_UDP:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(udp_blk))
				get_udp((udp_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_IF_NAMES:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(if_names_blk))
				get_if_names((if_names_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_ID:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(id_blk))
				get_id((id_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;
		case KINFO_GET_STR_CFG:
		        if (mp->b_cont && iocp->ioc_count >= sizeof(sad_blk))
				get_str_config((sad_blk *)mp->b_cont->b_rptr);
			else
        			mp->b_datap->db_type = M_IOCNAK;
			break;

		default:
        		mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		if (mp->b_datap->db_type == M_IOCNAK)
		{
			strlog(KINFO_ID, 
				0, 
				0, 
				SL_TRACE, 
				"kinfo: kinfo_wput() - IOCNAK");
		}
		qreply(q, mp);
		break;
	default:
		freemsg(mp);
		break;
	}
	return 0;
}

get_system(buf)
     system_blk *buf;
{
#if ( defined(wca) )
	extern char *get_system_type_string();
#endif /* wca */

	strncpy(buf->hostname, hostname, hostnamelen);
	buf->hostnamelen = hostnamelen;
	strncpy(buf->cputype,
		get_system_type_string(),
		MAX_CPUNAME_LEN);
	strncpy(buf->kernel_version, 
		version, 
		KERNEL_VERSION_MAX);
	bcopy(&boottime, &buf->bootime, sizeof(struct timeval));
}

/*
 * collect all the data about the network interfaces 
 */
get_interfaces(buf)
     if_blk *buf;
{
        struct ifnet *ifp;
	struct if_info_blk *info;
	int i;
	int max_if_infos = MAX_IF_INFOS;

        ifp = ifnet;
	buf->more = 0;
	buf->curr_cnt = 0;
	info = &buf->info[0];

	/*
	 * if we are in the middle of processing the table - let's try
	 * to pick up where we left off.
	 */
	if (buf->total_cnt != 0) {
	        for (i = 0; i < buf->total_cnt; i++) {
		      if (ifp->if_next) {
			      ifp = ifp->if_next;
		      } else {                   /* no entries left */
			      ifp = NULL;
			      buf->total_cnt = 0;
		      }
		}
	}

	/*
	 * collect the data for each entry
	 */
	for (; ifp; ifp = ifp->if_next) {
	        char ifbuf[10];
		struct ifaddr *ptr;

	        max_if_infos--;
	        if (max_if_infos < 0) {         /* out of room */
		         buf->more = 1;
			 break;
		}
		buf->total_cnt++;
		buf->curr_cnt++;
		strncpy(info->if_name, ifp->if_name, MAX_IFNAME_LEN-1);
		if (ifp->if_version)
		         strncpy(info->if_version, 
				 ifp->if_version, MAX_IFVER_LEN-1);
		/*
		 * find the internet address,if any, for this interface
		 * XXX  this doesn't work with aliases - only gets first 
		 * ip address - fix me.
		 */
		for (ptr = ifp->if_addrlist; ptr; ptr = ptr->ifa_next) {
		         if (ptr->ifa_addr->sa_family == AF_INET) {
			         bcopy(
			     &((struct sockaddr_in *)ptr->ifa_addr)->sin_addr, 
				       &info->ip_addr, 
				       sizeof(struct in_addr));
				 break;
			 }
		}
		info->if_unit = ifp->if_unit;
		info->if_type = ifp->if_type;
		info->if_mtu = ifp->if_mediamtu;
		info->if_operstatus = ifp->if_flags;
		info->if_speed = ifp->if_baudrate;
		bcopy(&ifp->if_lastchange, &info->if_lastchange,
		      sizeof(struct timeval));
		info->if_inoctets = ifp->if_ibytes;
		info->if_inmcasts = ifp->if_imcasts;
		info->if_inucasts = ifp->if_ipackets - ifp->if_imcasts;
		info->if_outmcasts = ifp->if_omcasts;
		info->if_outucasts = ifp->if_opackets - ifp->if_omcasts;
		info->if_indiscards = ifp->if_iqdrops;
		info->if_inerrors = ifp->if_ierrors;
		info->if_noproto = ifp->if_noproto;
		info->if_outoctets = ifp->if_obytes;
		info->if_outerrors = ifp->if_oerrors;
		info->if_outqlen = ifp->if_snd.ifq_len;
		info->if_outdiscards = ifp->if_snd.ifq_drops;
		info++;
	 }
}

get_icmp(buf)
     icmp_blk *buf;
{
       buf->in_msgs = icmpstat.icps_badcode +
	              icmpstat.icps_checksum +
		      icmpstat.icps_badlen +
		      icmpstat.icps_inhist[ICMP_UNREACH] +
		      icmpstat.icps_inhist[ICMP_TIMXCEED] +
		      icmpstat.icps_inhist[ICMP_PARAMPROB] +
		      icmpstat.icps_inhist[ICMP_SOURCEQUENCH] +
		      icmpstat.icps_inhist[ICMP_REDIRECT] +
		      icmpstat.icps_inhist[ICMP_ECHO] +
		      icmpstat.icps_inhist[ICMP_ECHOREPLY] +
		      icmpstat.icps_inhist[ICMP_TSTAMP] +
		      icmpstat.icps_inhist[ICMP_TSTAMPREPLY] +
		      icmpstat.icps_inhist[ICMP_MASKREQ] +
		      icmpstat.icps_inhist[ICMP_MASKREPLY];
       buf->in_icmp_errors = icmpstat.icps_badcode +
	                icmpstat.icps_checksum +
			icmpstat.icps_badlen;
       buf->in_unreach = icmpstat.icps_inhist[ICMP_UNREACH];
       buf->in_timeexcds = icmpstat.icps_inhist[ICMP_TIMXCEED]; 
       buf->in_parmprobs = icmpstat.icps_inhist[ICMP_PARAMPROB];
       buf->in_srcquenchs = icmpstat.icps_inhist[ICMP_SOURCEQUENCH];
       buf->in_redirects = icmpstat.icps_inhist[ICMP_REDIRECT];
       buf->in_echos = icmpstat.icps_inhist[ICMP_ECHO];
       buf->in_echoreps = icmpstat.icps_inhist[ICMP_ECHOREPLY];
       buf->in_tstamps = icmpstat.icps_inhist[ICMP_TSTAMP];
       buf->in_tstampreply = icmpstat.icps_inhist[ICMP_TSTAMPREPLY];
       buf->in_maskreqs = icmpstat.icps_inhist[ICMP_MASKREQ];
       buf->in_maskreps = icmpstat.icps_inhist[ICMP_MASKREPLY];
       buf->out_msgs = icmpstat.icps_reflect +
	               icmpstat.icps_error;
       buf->out_errors = icmpstat.icps_error;
       buf->out_unreach = icmpstat.icps_inhist[ICMP_UNREACH];
       buf->out_timeexcds = icmpstat.icps_inhist[ICMP_TIMXCEED];
       buf->out_parmprobs = icmpstat.icps_inhist[ICMP_PARAMPROB];
       buf->out_srcquenchs = icmpstat.icps_inhist[ICMP_SOURCEQUENCH];
       buf->out_redirects = icmpstat.icps_inhist[ICMP_REDIRECT];
       buf->out_echos = icmpstat.icps_inhist[ICMP_ECHO];
       buf->out_echoreps = icmpstat.icps_inhist[ICMP_ECHOREPLY];
       buf->out_tstamps = icmpstat.icps_inhist[ICMP_TSTAMP];
       buf->out_tstampreply = icmpstat.icps_inhist[ICMP_TSTAMPREPLY];
       buf->out_maskreqs = icmpstat.icps_inhist[ICMP_MASKREQ];
       buf->out_maskreps = icmpstat.icps_inhist[ICMP_MASKREPLY];
}

get_tcp(buf)
     tcp_blk *buf;
{
     struct tcp_info_blk *info;
     struct inpcb *curr_cb;
     struct tcpcb *tcp_cb;
     int max_tcp_infos = MAX_TCP_INFOS;

     buf->active_opens = tcpstat.tcps_connattempt;
     buf->passive_opens = tcpstat.tcps_accepts;
     buf->attempt_fails = tcpstat.tcps_conndrops;
     buf->estab_resets = tcpstat.tcps_drops;
     buf->in_segs = tcpstat.tcps_rcvtotal;
     buf->out_segs = tcpstat.tcps_sndtotal - tcpstat.tcps_sndrexmitpack;
     buf->retrans_segs = tcpstat.tcps_sndrexmitpack;
     buf->in_errors = tcpstat.tcps_rcvbadoff +
                      tcpstat.tcps_rcvshort +
                      tcpstat.tcps_rcvbadsum;
     buf->out_rsts = tcpstat.tcps_sndtotal;

     buf->curr_estab = 0;
     buf->curr_cnt = 0;
     buf->more = 0;
     info = &buf->info[0];
     for (curr_cb = tcb.inp_next; curr_cb != &tcb; curr_cb = curr_cb->inp_next)
     {
           tcp_cb = (struct tcpcb *)curr_cb->inp_ppcb;
	   if ((tcp_cb->t_state == TCPS_ESTABLISHED) || 
	       (tcp_cb->t_state == TCPS_CLOSE_WAIT)) {
	          buf->curr_estab++;
	   }
	   if (max_tcp_infos > 0) {
	          max_tcp_infos--;
	          buf->total_cnt++;
		  buf->curr_cnt++;
	          info->state = tcp_cb->t_state;
		  bcopy(&curr_cb->inp_laddr, &info->local_addr, 
			sizeof(struct in_addr));
		  info->local_port = curr_cb->inp_lport;
		  bcopy(&curr_cb->inp_faddr, &info->rem_addr, 
			sizeof(struct in_addr));
		  info->rem_port = curr_cb->inp_fport;
	          info++;
	   } else {
	          buf->more = 1;
	   }
     }
}

get_ip(buf)
     ip_blk *buf;
{
    /* 
     * set buf->forwarding only if ipforwarding is on AND we have >1
     * interface using ip
     */
    buf->forwarding = 0;
    if (ipforwarding) {
        struct ifnet *ifp;
        int num_interfaces = 0;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
	    struct ifaddr *ptr;
	    /*
	     * find the internet address for this interface - we are
	     * only collecting interfaces that have an ip address.
	     */
	    for (ptr = ifp->if_addrlist; ptr; ptr = ptr->ifa_next) {
	        if (ptr->ifa_addr->sa_family == AF_INET) {
		    num_interfaces++;
		    break;
		}
	    }
	}
	if (num_interfaces > 1)
	    buf->forwarding = 1;
    }

    buf->default_ttl = MAXTTL;
    buf->in_receives = ipstat.ips_total;
    buf->in_hdr_errors = ipstat.ips_badsum +
                         ipstat.ips_tooshort +
			 ipstat.ips_badlen +
			 ipstat.ips_toosmall +
                         ipstat.ips_badhlen;
    buf->in_addr_errors = ipstat.ips_cantforward;
    buf->forw_datagrams = ipstat.ips_forward;
    buf->in_delivers = ipstat.ips_total -
                       ipstat.ips_tooshort -
                       ipstat.ips_toosmall -
                       ipstat.ips_badhlen -
                       ipstat.ips_badlen;
    buf->out_requests = ipstat.ips_localout;
    buf->out_discards = ipstat.ips_odropped;
    buf->out_noroutes = ipstat.ips_cantforward;
    buf->reasm_tout = IPFRAGTTL;
    buf->reasm_reqds = ipstat.ips_fragments;
    buf->reasm_OKs = ipstat.ips_reassembled;
    buf->reasm_fails = ipstat.ips_fragdropped + ipstat.ips_fragtimeout;
    buf->frag_OKs = ipstat.ips_fragmented;
    buf->frag_fails = ipstat.ips_cantfrag;
    buf->frag_creates = ipstat.ips_ofragments;
}


fillin_routing_info(rt_ptr, buf)
     struct rtentry *rt_ptr;
     ip_routing_blk *buf;
{
     struct ip_routing_info_blk *info;
     struct ifnet *ifp;
     struct ifaddr *ptr;

     if (((struct sockaddr_in *)rt_key(rt_ptr))->sin_family != AF_INET)
         return 0;

     if (buf->curr_cnt >= MAX_IP_ROUTE_INFOS) {
         buf->more = 1;
         return 0;
     }

     info = &buf->info[buf->curr_cnt];
     buf->curr_cnt++;
     buf->total_cnt++;

     /*
      * fill in the values for this routing entry
      */
     bcopy(&((struct sockaddr_in *)rt_key(rt_ptr))->sin_addr,
	   &info->rt_dst,
	   sizeof(struct in_addr));
     if (rt_ptr->rt_gateway) {
         bcopy(&((struct sockaddr_in *)rt_ptr->rt_gateway)->sin_addr,
	       &info->rt_next_hop,
	       sizeof(struct in_addr));
     }
     if (rt_mask(rt_ptr)) {
         bcopy(&((struct sockaddr_in *)rt_mask(rt_ptr))->sin_addr,
#ifdef __alpha
/* alpha jestabro - differentiate field and macro for compiler problem */
	       &info->rt_maskx,
#else
	       &info->rt_mask,
#endif /* __alpha */
	       sizeof(struct in_addr));
     }
     info->rt_flags = rt_ptr->rt_flags;

     /*
      * loop through ifnet to find match on rt_ptr->rt_ifp;
      */
     info->if_index = 0;
     for (ifp = ifnet; ifp; ifp = ifp->if_next) {
         for (ptr = ifp->if_addrlist; ptr; ptr = ptr->ifa_next) {
	     if (ptr->ifa_addr->sa_family == AF_INET) {
	         info->if_index++;
		 break;
	     }
	 }
         if (ifp == rt_ptr->rt_ifp)
	     break;
     }

     /*
      * find the internet address for this interface
      */
     for (ptr = rt_ptr->rt_ifp->if_addrlist; ptr; ptr = ptr->ifa_next) {
         if (ptr->ifa_addr->sa_family == AF_INET) {
	     bcopy(
		   &((struct sockaddr_in *)ptr->ifa_addr)->sin_addr, 
		   &info->interface_ipaddr, 
		   sizeof(struct in_addr));
	     break;
	 }
     }

     return 0;
}


/*
 * this algorithm is taken from netstat
 */
walk_tree(rn, buf)
     struct radix_node *rn;
     ip_routing_blk *buf;
{
again:
     if (rn->rn_b < 0) {
           if (!(rn->rn_flags & RNF_ROOT))
	        fillin_routing_info((struct rtentry *)rn, buf);
	   if (rn = rn->rn_dupedkey)
 	        goto again;
     } else {
           walk_tree(rn->rn_l, buf);
           walk_tree(rn->rn_r, buf);
     }
}

get_ip_routing(buf)
     ip_routing_blk *buf;
{
     struct radix_node_head *rnh;
     struct radix_node *rn;

     /*
      * NOTE: this doesn't handle multiple buffers - need to add
      * code for it to handle more than MAX_IP_ROUTE_INFOS
      */
     buf->more = 0;
     buf->curr_cnt = 0;

     for (rnh = radix_node_head; rnh && (rnh->rnh_af != AF_INET); )
	  rnh = rnh->rnh_next;
     if (rnh == 0) return;
     rn = rnh->rnh_treetop;

     walk_tree(rn, buf);
}


get_arp(buf)
     arp_blk *buf;
{
    struct arptab *at;
    int i;
    struct ifnet *ifp;
    struct ifaddr *ptr;
    struct arp_info_blk *info;
    int max_arp_infos = MAX_ARP_INFOS;
    int start = 0;
    
    buf->more = 0;
    buf->curr_cnt = 0;
    info = &buf->info[0];
    at = &arptab[0];
    /*
     * if we didn't get it all the first time - pick up where
     * we left off
     */
    if (buf->total_cnt) {
        for (i = 0; i < arptab_size; i++, at++) {
	    if ((at->at_iaddr.s_addr == 0) || 
		(at->at_flags & ATF_INUSE == 0) ||
		(at->at_flags & ATF_COM == 0))
	         continue;
	    start++;
	    if (start >= buf->total_cnt)
	          break;
	}
    }
    /*
     * go through the arp table and fill in the entries
     */
    for (i = start; i < arptab_size; i++, at++) {
        if ((at->at_iaddr.s_addr == 0) || 
	    (at->at_flags & ATF_INUSE == 0) ||
	    (at->at_flags & ATF_COM == 0))
	    continue;
	if (--max_arp_infos < 0) {        /* out of room */
	    buf->more = 1;
	    break;
	}
	buf->total_cnt++;
	buf->curr_cnt++;
	bcopy(&at->at_iaddr, &info->inet_addr, 
	      sizeof(struct in_addr));
	bcopy(at->at_hwaddr, info->hw_addr, 14);

	/*
	 * find the index into the interface table.
	 */
	info->if_index = 0;
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
	    for (ptr = ifp->if_addrlist; ptr; ptr = ptr->ifa_next) {
	        if (ptr->ifa_addr->sa_family == AF_INET) {
		    info->if_index++;
		    break;
		}
		if (ifp == at->at_if)
		    break;
	    }     
	}
	info++;
    }
}


get_udp(buf)
     udp_blk *buf;
{
        struct inpcb *curr_cb;
	struct udp_info_blk *info;
	int max_udp_infos = MAX_UDP_INFOS;

	buf->in_datagrams = udpstat.udps_ipackets - 
			    udpstat.udps_hdrops -
			    udpstat.udps_badsum -
			    udpstat.udps_badlen -
			    udpstat.udps_noport -
			    udpstat.udps_fullsock;
	buf->no_ports = udpstat.udps_noport;
	buf->in_errors = udpstat.udps_hdrops +
	                 udpstat.udps_badsum +
			 udpstat.udps_badlen +
			 udpstat.udps_fullsock;
	buf->out_datagrams = udpstat.udps_opackets;

	buf->more = 0;
	buf->curr_cnt = 0;
	info = &buf->info[0];
	for (curr_cb = udb.inp_next; curr_cb != &udb; curr_cb = curr_cb->inp_next) {
	        if(--max_udp_infos < 0) {          /* out of room */
		         buf->more = 1;
			 break;
	        }
		buf->total_cnt++;
		buf->curr_cnt++;
		bcopy(&curr_cb->inp_laddr, &info->local_addr, 
		      sizeof(struct in_addr));
		info->local_port = curr_cb->inp_lport;
		info++;
	}

 }


/*
 * collects the interface names and unit #'s.  Used by netsetup
 * to sniff out the network adapters
 */
get_if_names(buf)
  if_names_blk *buf;
{
    struct ifnet *ifp;
    struct if_names_info_blk *info; 
    int found;

    buf->more = 0;
    buf->curr_cnt = 0;
    info = &buf->info[0];

    for (ifp = ifnet; ifp; ifp = ifp->if_next) {
        found = 0;
	if (strcmp(ifp->if_name, "lo") == 0) {
	    found = 1;
	    break;
	}
	if (!found) {
	    strcpy(info->if_name, ifp->if_name);
	    info->if_unit = ifp->if_unit;
	    buf->curr_cnt++;
	    buf->total_cnt++;
	    info++;
	}
    }
}

get_id(buf)
  id_blk *buf;
{
   buf->version = KINFO_VERSION;
   strcpy(buf->name, KINFO_NAME);
}


int get_str_config( sad )
SAD *sad;
{
	extern struct modsw *dmodsw;
	extern struct modsw *fmodsw;
	extern dev_t clonedev;

	register struct modsw *dp;
	STR_CFG *s_cfg;	

	/* 
	 * Init 
	 */
	sad->sad_blkhdr.sad_cnt = 0;
	s_cfg = &sad->sad_cfg[0]; 

	/* 
	 * Return clone driver info
	 */
	sad->sad_blkhdr.sad_clonedev = clonedev;

	/*
	 * Get configured STREAMS driver info
	 */
	dp = dmodsw;
	while(dp)
	{
		if (dp->d_str)
		{
			sad->sad_blkhdr.sad_cnt++;
			bzero(s_cfg, sizeof(*s_cfg));
			s_cfg->scb_mid = dp->d_str->st_rdinit->qi_minfo->mi_idnum;
			s_cfg->scb_device = makedev(dp->d_major, 0);
			s_cfg->scb_flags |= (SADF_CLONE | SADF_DEVICE);
			bcopy( dp->d_name, s_cfg->scb_name, str_strlen(dp->d_name) );
			s_cfg++;
		}
		if((dp = dp->d_next) == dmodsw)
			break;
	}

	/*
	 * Get configured STREAMS module info
	 */
	dp = fmodsw;
	while(dp)
	{
		if (dp->d_str)
		{
			sad->sad_blkhdr.sad_cnt++;
			bzero(s_cfg, sizeof(*s_cfg));
			s_cfg->scb_mid = dp->d_str->st_rdinit->qi_minfo->mi_idnum;
			s_cfg->scb_flags |= SADF_MODULE;
			bcopy( dp->d_name, s_cfg->scb_name, str_strlen(dp->d_name) );
			s_cfg++;
		}
		if((dp = dp->d_next) == fmodsw)
			break;
	}

	return;
}

int str_strlen( p )
char *p;
{
	register char *p1 = p;

	if (p1)
	{
		while(*p1)
		{
			p1++;
		}
	}

	return(p1 - p);
}

#if ( defined(wca) )
static char system_type[] = "MIPS";

char *get_system_type_string()
{
	return( system_type );
}
#endif /* wca */
