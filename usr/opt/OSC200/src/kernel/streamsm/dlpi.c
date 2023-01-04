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
static char *rcsid = "@(#)$RCSfile: dlpi.c,v $ $Revision: 1.1.11.8 $ (DEC) $Date: 1994/01/22 01:29:52 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1991  Mentat Inc.
 ** dlosf.c 0.1, last change 6/1/91
 **/

/*
static	char	sccsid[] = "@(#)dlosf.c\t\t0.1";
*/

/* STREAMS DLPI (subset) to network interface module for OSF/1 */

#include <dlpi.h>              /* XXX dynamic dependency */
#include <sys/param.h>

#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include <dli/dli_var.h>

#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/enet.h>
#include <sys/dlpihdr.h>
#include <streams/nd.h>
#include <sys/sysconfig.h>

/* #define DL_DEBUG	1 */

#ifndef	staticf
#define staticf	static
#endif

#define	DLB_MIN_DL_IND_HDR_SIZE		(DL_UNITDATA_IND_SIZE + 16)

typedef u_char	u8;
typedef u_short	u16;
typedef struct dli_ifnet DLIF, *DLIFP;
typedef struct dli_filter FLTR, *FLTRP;
typedef struct dli_recv RECV, *RECVP;
typedef	struct ifnet	IF, * IFP;
typedef struct mbuf	MBUF, * MBUFP;
typedef struct msgb	MBLK, * MBLKP;

#define	DLB_MAX_FILTERS		8

typedef struct dlb_s {
	struct dlb_s	* dlb_link;
	queue_t	* dlb_rq;
	int	dlb_addr_length;
	char    dlb_addr[16];
	FLTRP	dlb_filters[DLB_MAX_FILTERS];
	IFP 	dlb_ifp;
	uint	dlb_mac_type;
	uint	dlb_sap;
} DLB, * DLBP, ** DLBPP;


staticf	MBLKP	dlb_attach(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_bind(   DLBP dlb, MBLKP mp   );
staticf MBLKP	dlb_subs_bind(   DLBP dlb, MBLKP mp   );
staticf	int	dlb_close(   queue_t * q, int flag, cred_t * credp   );
staticf	MBLKP	dlb_detach(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_error_ack(   MBLKP mp, ulong prim, int unix_error, int dl_error   );
staticf int	dlb_get_ifnames(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf	int	dlb_get_toggle(   queue_t * q, mblk_t * mp, caddr_t data   );
staticf int	dlb_input( RECVP rcv, MBUFP m, FLTRP filter );
staticf	MBLKP	dlb_ok_ack(   MBLKP mp, ulong prim   );
staticf	int	dlb_open(  queue_t * q, dev_t * devp, int flag, int sflag, cred_t * credp   );
staticf	int	dlb_rsrv(   queue_t * q   );
staticf	int	dlb_set_toggle(   queue_t * q, mblk_t * mp, unsigned char * value, caddr_t data   );
staticf	MBLKP	dlb_unbind(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_subs_unbind(   DLBP dlb, MBLKP mp   );
staticf	int	dlb_wput(   queue_t * q, MBLKP mp   );
staticf	MBLKP	dlb_promiscon(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_promiscoff(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_phys_addr(   DLBP dlb, MBLKP mp   );
staticf	MBLKP	dlb_set_phys_multi_addr(   DLBP dlb, MBLKP mp   );

extern	MBUFP	mblk_to_mbuf(   MBLKP mblk, int canwait   );
extern	MBLKP	mbuf_to_mblk(   MBUFP m, int pri   );

static struct module_info minfo =  {
	5010, "dl", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	NULL, dlb_rsrv, dlb_open, dlb_close, NULL, &minfo
};

static struct qinit winit = {
	dlb_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab dlbinfo = { &rinit, &winit };

static	dl_unitdata_ind_t	dlb_udi = {
	DL_UNITDATA_IND,
	6,
	DL_UNITDATA_IND_SIZE,
	6,
	DL_UNITDATA_IND_SIZE + 8,
	0
};

static	int	dlb_g_bind_count = 0;
static	DLBP	dlb_list = 0;
static	caddr_t	dlb_g_nd;

/* OSF configuration routine for the dlb device. */
int
dl_configure(sysconfig_op_t op, char *indata, size_t indatalen,
		 char *outdata, size_t outdatalen)
{
	static struct streamadm	sa;
	static dev_t		devno;
	static int              dlb_major;
        int                     configured;
        int                     size;
        int                     ret = 0;
#ifdef NEW_CONFIG
        struct subsystem_info   info;
        static config_attr_t dlb_attr[] = {
        { SUBSYS_NAME, STRTYPE, (caddr_t)"dl" , CONSTANT, {0, NULL} },
        { "sa_flags", INTTYPE, (caddr_t)&sa.sa_flags, CONSTANT, {0} },
        { "sa_name", STRTYPE, (caddr_t)sa.sa_name, CONSTANT, {0, NULL} },
        { "devno", INTTYPE, (caddr_t)&devno, CONFIG, {(int)NODEV} },
        { "major", INTTYPE, (caddr_t)&dlb_major, CONSTANT, {0} }
        };
#define NUM_STRDLB_ATTRS sizeof(dlb_attr)/sizeof(config_attr_t)

        strcpy(info.subsystem_name,"dl");
        configured = ((!subsys_reg(SUBSYS_GET_INFO,&info)) && info.config_flag);
        if ((configured && (op == SYSCONFIG_CONFIGURE)) ||
            (!configured && (op != SYSCONFIG_CONFIGURE)))
                return EALREADY;
#endif NEW_CONFIG

        switch (op) {
        case SYSCONFIG_CONFIGURE:
	    if (indata != NULL && indatalen == sizeof(str_config_t)
		&& ((str_config_t *)indata)->sc_version == OSF_STREAMS_CONFIG_10)
		  devno = ((str_config_t *)indata)->sc_devnum;
	    else
		  devno = NODEV;

	    sa.sa_version	= OSF_STREAMS_11;
	    sa.sa_flags		= STR_IS_DEVICE|STR_SYSV4_OPEN;
	    sa.sa_ttys		= 0;
	    sa.sa_sync_level	= SQLVL_MODULE;
	    sa.sa_sync_info		= 0;
	    strcpy(sa.sa_name, 	"dl");

	    if ((devno = strmod_add(devno, &dlbinfo, &sa)) == NODEV) {
		return ENODEV;
	    } 
	    dlb_major = major(devno);

            if (outdata && outdatalen>=0)
                bcopy(indata,outdata,min(indatalen,outdatalen));
            break;

#ifdef NEW_CONFIG
        case SYSCONFIG_UNCONFIGURE:
                if (configured && info.module_id != 0)
                    ret = strmod_del(devno, &dlbinfo, &sa);
                else
                    ret = EINVAL;
                break;

        case SYSCONFIG_QUERYSIZE:
            if (outdatalen >= sizeof(int)) {
                *(int *)outdata = do_querysize(dlb_attr,NUM_STRDLB_ATTRS);
            } else {
                ret = ENOMEM;
            }
            break;

        case SYSCONFIG_QUERY:
            size = do_querysize(dlb_attr,NUM_STRDLB_ATTRS);
            if (outdatalen < size) {
                ret = ENOMEM;
                break;
            }
            ret = do_query(dlb_attr,NUM_STRDLB_ATTRS,outdata,outdatalen);
            break;

        case SYSCONFIG_RECONFIGURE:
#endif NEW_CONFIG
        default:
	    ret = EINVAL;
	    break;
	}

	return(ret);
}

/*
 * TODO:  Right now this code supports ethernet only.
 */

staticf MBLKP
dlb_attach (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_attach_req_t	* dlar;
	IFP	ifp;

	if ( dlb->dlb_ifp )
		return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_OUTSTATE);
	dlar = (dl_attach_req_t *)mp->b_rptr;
	dlb->dlb_addr_length = 0;
	for ( ifp = ifnet; ifp; ifp = ifp->if_next ) {
		if (dlar->dl_ppa == ifp->if_index) {
		        switch (ifp->if_type) {
			    case IFT_ETHER:	dlb->dlb_mac_type = DL_ETHER; break;
			    case IFT_ISO88023:	dlb->dlb_mac_type = DL_CSMACD; break;
			    case IFT_FDDI:	dlb->dlb_mac_type = DL_FDDI; break;
			    case IFT_ISO88025:	dlb->dlb_mac_type = DL_TPR; break;
			    default: {
				return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_BADPPA);
			    }
			}
			bcopy((caddr_t)((struct arpcom *)ifp)->ac_hwaddr,
				(caddr_t)dlb->dlb_addr, 6);
			dlb->dlb_addr_length = 6;
			dlb->dlb_ifp = ifp;
#if	DL_DEBUG
{char sbuf[128];
arp_sprintf(sbuf, (uchar *)dlb->dlb_addr, 6);
printf("dlb_attach: ppa %d ifp 0x%x (%s%d), phys addr %s\n",
dlar->dl_ppa, ifp, ifp->if_name, ifp->if_unit, sbuf);
}
#endif
			return dlb_ok_ack(mp, DL_ATTACH_REQ);
		}
	}
	return dlb_error_ack(mp, DL_ATTACH_REQ, 0, DL_BADPPA);
}

staticf MBLKP
dlb_bind (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_bind_ack_t	* dlba;
	DLBP		dlb1;
	dl_bind_req_t	* dlbr;
	int		err;
	int		i1;
	uint		sap;
	struct sockaddr_dl sdl;

	dlbr = (dl_bind_req_t *)mp->b_rptr;
	if ( !dlb->dlb_ifp || dlb->dlb_sap != 0 )
	 	return dlb_error_ack(mp, DL_BIND_REQ, 0, DL_OUTSTATE);
	sap = dlbr->dl_sap;

	bzero(&sdl, sizeof(sdl));
	sdl.dli_family = AF_DLI;
	if (sap < 256) {
	    sdl.dli_substructype = DLI_802;
	    sdl.choose_addr.dli_802addr.svc = USER;
	    sdl.choose_addr.dli_802addr.ioctl = DLI_EXCLUSIVE;
	    sdl.choose_addr.dli_802addr.eh_802.dsap = sap;
	    sdl.choose_addr.dli_802addr.eh_802.ssap = sap;
	} else if (sap > ETHERMTU && sap < 65536) {
	    sdl.dli_substructype = DLI_ETHERNET;
	    sdl.choose_addr.dli_eaddr.dli_protype = sap;
	    sdl.choose_addr.dli_eaddr.dli_ioctlflg = DLI_EXCLUSIVE;
	} else {
	    return dlb_error_ack(mp, DL_BIND_REQ, 0, DL_BADSAP);
	}

	err = dli_filter_add(dli_ifp2dlif(dlb->dlb_ifp), &sdl,
			     &dlb->dlb_filters[0]);
	if (err) {
	    int dlerror;
	    switch (err) {
		case EADDRNOTAVAIL:	err = 0; dlerror = DL_BUSY; break;
		case EINVAL:		err = 0; dlerror = DL_BADSAP; break;
		case EACCES:		err = 0; dlerror = DL_ACCESS; break;
		default:		dlerror = DL_SYSERR; break;
	    }
	    return dlb_error_ack(mp, DL_BIND_REQ, err, dlerror);
	}

	i1 = DL_BIND_ACK_SIZE + dlb->dlb_addr_length + 2;
	if ( (mp->b_datap->db_lim - mp->b_datap->db_base) < i1 ) {
		freemsg(mp);
		if (!(mp = allocb(i1, BPRI_HI))) {
		        dli_filter_destroy(dli_ifp2dlif(dlb->dlb_ifp),
					   &dlb->dlb_filters[0]);
			return dlb_error_ack(mp, DL_BIND_REQ, ENOMEM, DL_SYSERR);
		}
	} else
		mp->b_rptr = mp->b_datap->db_base;
	dlb->dlb_filters[0]->fltr_ctx = (caddr_t) dlb;
	dlb->dlb_filters[0]->fltr_match = dlb_input;
	dlb->dlb_sap = sap;
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + i1;
	dlba = (dl_bind_ack_t *)mp->b_rptr;
	dlba->dl_primitive = DL_BIND_ACK;
	dlba->dl_sap = dlb->dlb_sap;
	dlba->dl_addr_offset = DL_BIND_ACK_SIZE;
	dlba->dl_max_conind = 0;
	dlba->dl_xidtest_flg = 0;
	bcopy((char *)&dlb->dlb_addr[0], (char *)&dlba[1], (int)dlb->dlb_addr_length);
	if (sap < 256) {
		mp->b_rptr[dlb->dlb_addr_length + DL_BIND_ACK_SIZE] = (uchar)sap;
  		dlba->dl_addr_length = dlb->dlb_addr_length + 1;
		mp->b_wptr--;
	} else {
		mp->b_rptr[dlb->dlb_addr_length + DL_BIND_ACK_SIZE] = 
			(uchar)(sap >> 8);
		mp->b_rptr[dlb->dlb_addr_length + DL_BIND_ACK_SIZE +1] = (uchar)sap;
  		dlba->dl_addr_length = dlb->dlb_addr_length + 2;
	}
	return mp;
}

staticf MBLKP
dlb_subs_bind (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_subs_bind_ack_t	* dlsba;
	DLBP		dlb1;
	dl_subs_bind_req_t	* dlsbr;
	int		err;
	int		i1, fltr;
	uint		sap;
	struct sockaddr_dl sdl;

	dlsbr = (dl_subs_bind_req_t *)mp->b_rptr;
	if ( !dlb->dlb_ifp || dlb->dlb_sap == 0 )
	 	return dlb_error_ack(mp, DL_SUBS_BIND_REQ, 0, DL_OUTSTATE);
	if (dlsbr->dl_subs_bind_class != DL_HIERARCHICAL_BIND || dlsbr->dl_subs_sap_len != 1)
	 	return dlb_error_ack(mp, DL_SUBS_BIND_REQ, 0, DL_UNSUPPORTED);
	
	sap = mp->b_rptr[dlsbr->dl_subs_sap_offset];

	bzero(&sdl, sizeof(sdl));
	sdl.dli_family = AF_DLI;
	sdl.dli_substructype = DLI_802;
	sdl.choose_addr.dli_802addr.svc = USER;
	sdl.choose_addr.dli_802addr.ioctl = DLI_EXCLUSIVE;
	sdl.choose_addr.dli_802addr.eh_802.dsap = sap;
	sdl.choose_addr.dli_802addr.eh_802.ssap = sap;

	for (fltr = 1; fltr < DLB_MAX_FILTERS; fltr++) {
	    if (dlb->dlb_filters[fltr] == NULL)
		break;
	}
	if (fltr == DLB_MAX_FILTERS)
	    return dlb_error_ack(mp, DL_SUBS_BIND_REQ, 0, DL_TOOMANY);

	err = dli_filter_add(dli_ifp2dlif(dlb->dlb_ifp), &sdl,
			     &dlb->dlb_filters[fltr]);
	if (err) {
	    int dlerror;
	    switch (err) {
		case EADDRNOTAVAIL:	err = 0; dlerror = DL_BUSY; break;
		case EINVAL:		err = 0; dlerror = DL_BADSAP; break;
		case EACCES:		err = 0; dlerror = DL_ACCESS; break;
		default:		dlerror = DL_SYSERR; break;
	    }
	    return dlb_error_ack(mp, DL_SUBS_BIND_REQ, err, dlerror);
	}

	i1 = DL_SUBS_BIND_ACK_SIZE + dlsbr->dl_subs_sap_len;
	if ( (mp->b_datap->db_lim - mp->b_datap->db_base) < i1 ) {
		freemsg(mp);
		if (!(mp = allocb(i1, BPRI_HI))) {
		        dli_filter_destroy(dli_ifp2dlif(dlb->dlb_ifp),
					   &dlb->dlb_filters[fltr]);
			return dlb_error_ack(mp, DL_SUBS_BIND_REQ, ENOMEM, DL_SYSERR);
		}
	} else
		mp->b_rptr = mp->b_datap->db_base;
	dlb->dlb_filters[fltr]->fltr_ctx = (caddr_t) dlb;
	dlb->dlb_filters[fltr]->fltr_match = dlb_input;
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + i1;
	dlsba = (dl_subs_bind_ack_t *)mp->b_rptr;
	dlsba->dl_primitive = DL_SUBS_BIND_ACK;
  	dlsba->dl_subs_sap_len = dlsbr->dl_subs_sap_len;
	dlsba->dl_subs_sap_offset = DL_SUBS_BIND_ACK_SIZE;
	mp->b_rptr[dlsba->dl_subs_sap_offset] = sap;
	return mp;
}

staticf int
dlb_close (q, flag, credp)
	queue_t	* q;
	int	flag;
	cred_t	* credp;
{
	DLBP	dlb;
	DLBPP	dlbp;

	dlb = (DLBP)q->q_ptr;
	(void)dlb_unbind(dlb, (MBLKP)0);
	(void)dlb_detach(dlb, (MBLKP)0);
	for (dlbp = &dlb_list; *dlbp; dlbp = &(*dlbp)->dlb_link) {
		if (*dlbp == dlb) {
			*dlbp = dlb->dlb_link;
			break;
		}
	}
	return streams_close_comm(q, flag, credp);
}

staticf MBLKP
dlb_detach (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	/* The board may only be detached if the stream is attached but unbound */
	if ( !dlb->dlb_ifp  ||  dlb->dlb_sap )
		return mp ? dlb_error_ack(mp, DL_DETACH_REQ, 0, DL_OUTSTATE) : (MBLKP)0;
	dlb->dlb_ifp = 0;
	return mp ? dlb_ok_ack(mp, DL_DETACH_REQ) : (MBLKP)0;
}

staticf MBLKP
dlb_error_ack (mp, prim, unix_error, dl_error)
	MBLKP	mp;
	ulong	prim;
	int	unix_error;
	int	dl_error;
{
	dl_error_ack_t	* dlea;

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < DL_ERROR_ACK_SIZE) {
		freemsg(mp);
		mp = allocb(DL_ERROR_ACK_SIZE, BPRI_HI);
		if (!mp)
			return mp;
	}
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + DL_ERROR_ACK_SIZE;
	mp->b_datap->db_type = M_PCPROTO;
	dlea = (dl_error_ack_t *)mp->b_rptr;
	dlea->dl_primitive = DL_ERROR_ACK;
	dlea->dl_error_primitive = prim;
	dlea->dl_errno = dl_error;
	dlea->dl_unix_errno = unix_error;
	return mp;
}

staticf MBLKP
dlb_ok_ack (mp, prim)
	MBLKP	mp;
	ulong	prim;
{
	dl_ok_ack_t	* dloa;

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < DL_OK_ACK_SIZE) {
		freemsg(mp);
		mp = allocb(DL_OK_ACK_SIZE, BPRI_HI);
		if (!mp)
			return mp;
	}
	mp->b_rptr = mp->b_datap->db_base;
	mp->b_wptr = mp->b_rptr + DL_OK_ACK_SIZE;
	mp->b_datap->db_type = M_PCPROTO;
	dloa = (dl_ok_ack_t *)mp->b_rptr;
	dloa->dl_primitive = DL_OK_ACK;
	dloa->dl_correct_primitive = prim;
	return mp;
}

staticf int
dlb_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	DLBP	dlb;
	int	err;

	if (err = drv_priv(credp))
		return err;
	if (!dlb_g_nd
	    && (!nd_load(&dlb_g_nd, "dl_ifnames", dlb_get_ifnames, (int (*)())NULL,
			 (caddr_t)NULL)))
		return EAGAIN;
	err = streams_open_comm(sizeof(DLB), q, devp, flag, sflag, credp);
	if (err == 0) {
		dlb = (DLBP)q->q_ptr;
		dlb->dlb_rq = q;
		dlb->dlb_link = dlb_list;
		dlb_list = dlb;
	}
	return err;
}

staticf MBLKP
dlb_info (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_info_ack_t	* dlia;
	int ack_size;
	

	/*
	 * if unbound no DLSAP is returned otherwise
	 * need to allocate enuff room for DLSAP
	 */
	if (!dlb->dlb_sap)
		ack_size = DL_INFO_ACK_SIZE; 
	else 
		ack_size = DL_INFO_ACK_SIZE + dlb->dlb_addr_length + 2; 
	if ( mp->b_datap->db_lim - mp->b_datap->db_base < ack_size ) {
		freemsg(mp);
		if (!(mp = allocb(ack_size, BPRI_HI)))
			return dlb_error_ack(mp, DL_INFO_REQ, ENOMEM, DL_SYSERR);
	} else {
		mp->b_rptr = mp->b_datap->db_base;
		mp->b_wptr = mp->b_rptr;
	}
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr += ack_size;
	dlia = (dl_info_ack_t *)mp->b_rptr;
	dlia->dl_primitive = DL_INFO_ACK;

	if ( dlb->dlb_ifp ) {
		/* Attached */
	        if (dlb->dlb_ifp->if_mediamtu)
			dlia->dl_max_sdu = dlb->dlb_ifp->if_mediamtu;
		else
			dlia->dl_max_sdu = dlb->dlb_ifp->if_mtu;
		dlia->dl_min_sdu = 1;
		dlia->dl_mac_type = dlb->dlb_mac_type;
		dlia->dl_addr_length = dlb->dlb_addr_length;
		if ( dlb->dlb_sap ) {
			dlia->dl_current_state = DL_IDLE;
			dlia->dl_addr_offset = DL_INFO_ACK_SIZE;
			bcopy((char *)&dlb->dlb_addr[0], (char *)&dlia[1], 
				(int)dlb->dlb_addr_length);
			if (dlb->dlb_sap < 256) {
				dlia->dl_sap_length = 1;
				dlia->dl_addr_length++; 
				mp->b_rptr[dlb->dlb_addr_length + DL_INFO_ACK_SIZE] = (uchar)dlb->dlb_sap;
				mp->b_wptr--;
			} else {
				dlia->dl_sap_length = 2;
				dlia->dl_addr_length += 2; 
				mp->b_rptr[dlb->dlb_addr_length + DL_INFO_ACK_SIZE] = 
					(uchar)(dlb->dlb_sap >> 8);
				mp->b_rptr[dlb->dlb_addr_length + DL_INFO_ACK_SIZE +1] = (uchar)dlb->dlb_sap;
			}
		} else {
			dlia->dl_addr_offset = 0;
			dlia->dl_addr_length = 0; 
			dlia->dl_sap_length = 0;
			dlia->dl_current_state = DL_UNBOUND;
		}
	} else {
		/* Not Attached */
		dlia->dl_max_sdu = 1500;	/* ??dlb->dlb_ifp->if_mtu; */
		dlia->dl_min_sdu = 1;
		dlia->dl_mac_type = DL_ETHER;
		dlia->dl_addr_length = dlb->dlb_addr_length;
		dlia->dl_current_state = DL_UNATTACHED;
	}

	dlia->dl_reserved = 0;
	dlia->dl_service_mode = DL_CLDLS;
	dlia->dl_qos_length = 0;
	dlia->dl_qos_offset = 0;
	dlia->dl_qos_range_length = 0;
	dlia->dl_qos_range_offset = 0;
	dlia->dl_provider_style = DL_STYLE2;
	dlia->dl_growth = 0;
	return mp;
}

staticf int
dlb_get_ifnames (q, mp, data)
	queue_t	* q;
	mblk_t	* mp;
	caddr_t	data;
{
	struct ifnet	* ifp;

	for ( ifp = ifnet; ifp; ifp = ifp->if_next )
		mpsprintf(mp, "%s%d (PPA %d)",
				ifp->if_name, ifp->if_unit, ifp->if_index);
	return 0;
}

staticf int
dlb_input (rcv, m, filter)
	RECVP	rcv;
	MBUFP	m;
	FLTRP	filter;
{
	DLBP	dlb = (DLBP) filter->fltr_ctx;
	MBLKP	mp1;
	ushort	*ap;
	int	hdrlen = rcv->rcv_hdrlen;

	mp1 = allocb(DLB_MIN_DL_IND_HDR_SIZE, BPRI_MED);
	if (mp1 == NULL) {
	    m_freem(m);
	    return 1;
	}

	mp1->b_wptr = mp1->b_rptr + DLB_MIN_DL_IND_HDR_SIZE;
	mp1->b_datap->db_type = M_PROTO;

	*((dl_unitdata_ind_t *)mp1->b_rptr) = dlb_udi;

	DLI_ADDR_EXTRACT(&rcv->rcv_dst, &mp1->b_rptr[dlb_udi.dl_dest_addr_offset]);
	DLI_ADDR_EXTRACT(&rcv->rcv_src, &mp1->b_rptr[dlb_udi.dl_src_addr_offset]);

	if (filter->fltr_flags & DLI_FILTER_SNAPSAP)
	    hdrlen += LLC_SNAP_LEN;
	m_adj(m, hdrlen);

	if (m->m_len == 0)
		m = m_free(m);
	mp1->b_cont = mbuf_to_mblk(m, BPRI_MED);
	if (!mp1->b_cont) {
		(void) freeb(mp1);
		m_freem(m);
		return 1;
	}
	putq(dlb->dlb_rq, mp1);
	return 1;
}

staticf int
dlb_rsrv (q)
	queue_t	* q;
{
	MBLKP	mp1;

	while (mp1 = getq(q)) {
		if ( !canput(q->q_next) ) {
			putbq(q, mp1);
			return;
		}
		putnext(q, mp1);
	}
	return 0;
}

staticf MBLKP
dlb_unbind (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	int	fltr;

	if ( !dlb->dlb_sap )
		return mp ? dlb_error_ack(mp, DL_UNBIND_REQ, 0, DL_OUTSTATE) : (MBLKP)0;

	for (fltr = 0; fltr < DLB_MAX_FILTERS; fltr++) {
	    if (dlb->dlb_filters[fltr]) {
		dli_filter_destroy(dli_ifp2dlif(dlb->dlb_ifp),
				   &dlb->dlb_filters[fltr]);
	    }
	}
	dlb->dlb_sap = 0;
	
	return mp ? dlb_ok_ack(mp, DL_UNBIND_REQ) : (MBLKP)0;
}

staticf MBLKP
dlb_subs_unbind (dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
        dl_subs_unbind_req_t *dlsur = (dl_subs_unbind_req_t *) mp->b_rptr;
	int	fltr;
	uint	sap;

	if ( !dlb->dlb_sap )
		return dlb_error_ack(mp, DL_SUBS_UNBIND_REQ, 0, DL_OUTSTATE);
	if (dlsur->dl_subs_sap_len != 1)
		return dlb_error_ack(mp, DL_SUBS_UNBIND_REQ, 0, DL_BADSAP);
	sap = mp->b_rptr[dlsur->dl_subs_sap_offset];
	for (fltr = 1; fltr < DLB_MAX_FILTERS; fltr++) {
	    if (dlb->dlb_filters[fltr]
			&& dlb->dlb_filters[fltr]->fltr_llc.llc_dsap == sap)
		break;
	}
	if (fltr == DLB_MAX_FILTERS)
		return dlb_error_ack(mp, DL_SUBS_UNBIND_REQ, 0, DL_BADSAP);
	dli_filter_destroy(dli_ifp2dlif(dlb->dlb_ifp), &dlb->dlb_filters[fltr]);
	return dlb_ok_ack(mp, DL_SUBS_UNBIND_REQ);
}

/*
 *	DL_PROMISCON_REQ
 *	This primitive requests DLS provider to enable promiscuos mode on
 *	a per Stream basis, either at the physical or at the SAP level.
 *
 *	For now, this routine only supports (DL_PROMISC_PHYS and
 *	DL_PROMISC_MULTI) physical level. Therefore, this primitive is only 
 *	valid, if the interface is attached.
 */
staticf MBLKP
dlb_promiscon(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_promiscon_req_t * dlpr;
	IFP ifp;
#define	STK_PARMS	128
	char stkbuf [STK_PARMS];
	caddr_t data = stkbuf;
	int err;

	ifp = dlb->dlb_ifp;
	if ( !ifp )  /* Unattached */
		return dlb_error_ack (mp, DL_PROMISCON_REQ, 0, DL_OUTSTATE);
	
	dlpr = (dl_promiscon_req_t *) mp->b_rptr;

	switch (dlpr->dl_level) {
	case DL_PROMISC_PHYS:
	case DL_PROMISC_MULTI:
		/*
		 * If the interface's ioctl undefined than fall thru
		 */
		if (ifp->if_ioctl) {
			/*
			 * Set appropriate n/w interface flag and call ioctl
			 */
			if (dlpr->dl_level == DL_PROMISC_PHYS)
				ifp->if_flags |= IFF_PROMISC;
			else
				ifp->if_flags |= IFF_ALLMULTI;
	   
			*(int *)data = 0;
			err = (*ifp->if_ioctl) (ifp, SIOCSIFFLAGS, data);
#if	DLPI_DEBUG
if (err) printf ("dlb_promiscon: interface returned 0x%x\n", err);
#endif
			if (err)
				return dlb_error_ack(mp, DL_PROMISCON_REQ, err, DL_SYSERR);
			else
				return dlb_ok_ack(mp, DL_PROMISCON_REQ);
		}
		break;
	case DL_PROMISC_SAP:
		break;
	default:
		return dlb_error_ack(mp, DL_PROMISCON_REQ, 0, DL_UNSUPPORTED);
		break;
	}

	return dlb_error_ack(mp, DL_PROMISCON_REQ, 0, DL_NOTSUPPORTED);
	
}

/*
 *	DL_PROMISCOFF_REQ
 *	This primitive requests DLS provider to disable promiscuos mode on
 *	a per Stream basis, either at the physical or at the SAP level.
 *
 *	For now, this routine only supports (DL_PROMISC_PHYS and
 *	DL_PROMISC_MULTI) physical level. Therefore, this primitive is only 
 *	valid, if the interface is attached.
 */
staticf MBLKP
dlb_promiscoff(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_promiscoff_req_t * dlpfr;
	IFP ifp;
#define	STK_PARMS	128
	char stkbuf [STK_PARMS];
	caddr_t data = stkbuf;
	int err;

	ifp = dlb->dlb_ifp;
	if ( !ifp )  /* Unattached */
		return dlb_error_ack (mp, DL_PROMISCOFF_REQ, 0, DL_OUTSTATE);
	
	dlpfr = (dl_promiscoff_req_t *) mp->b_rptr;

	switch (dlpfr->dl_level) {
	case DL_PROMISC_PHYS:
	case DL_PROMISC_MULTI:
		/*
		 * If the interface's ioctl undefined than fall thru
		 */
		if (ifp->if_ioctl) {
			/*
			 * Set appropriate n/w interface flag and call ioctl
			 */
			if (dlpfr->dl_level == DL_PROMISC_PHYS)
				ifp->if_flags &= ~IFF_PROMISC;
			else
				ifp->if_flags &= ~IFF_ALLMULTI;
	   
			*(int *)data = 0;
			err = (*ifp->if_ioctl) (ifp, SIOCSIFFLAGS, data);
#if	DLPI_DEBUG
if (err) printf ("dlb_promiscoff: interface returned 0x%x\n", err);
#endif
			if (err)
				return dlb_error_ack(mp, DL_PROMISCOFF_REQ, err, DL_SYSERR);
			
			return dlb_ok_ack(mp, DL_PROMISCOFF_REQ);
		}
		break;
	case DL_PROMISC_SAP:
		break;
	default:
		return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, DL_UNSUPPORTED);
		break;
	}

	return dlb_error_ack(mp, DL_PROMISCOFF_REQ, 0, DL_NOTSUPPORTED);
	
}

/*
 *	DL_PHYS_ADDR_REQ
 *	This primitive requests DLS provider to either default(factory) or the
 *	current value of the physical address.
 *
 */
staticf MBLKP
dlb_phys_addr(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_phys_addr_req_t * dlphr;
	dl_phys_addr_ack_t * dlpha;
	IFP ifp;
#define	STK_PARMS	128
	char stkbuf [STK_PARMS];
	caddr_t data = stkbuf;
        struct ifdevea *ifd = (struct ifdevea *) data;
	int err, ack_size;

	ifp = dlb->dlb_ifp;
	if ( !ifp ) /* Unattached */
		return dlb_error_ack (mp, DL_PHYS_ADDR_REQ, 0, DL_OUTSTATE);
	
	dlphr = (dl_phys_addr_req_t *) mp->b_rptr;

	switch (dlphr->dl_addr_type) {
	case DL_FACT_PHYS_ADDR:
	case DL_CURR_PHYS_ADDR:
		/*
		 * If the interface's ioctl undefined than fall thru
		 */
		if (ifp->if_ioctl) {
			/*
			 * First read the phys_address using ioctl
			 */
			err = (*ifp->if_ioctl) (ifp, SIOCRPHYSADDR, data);
#if	DLPI_DEBUG
if (err) printf ("dlb_phys_addr: interface returned 0x%x\n", err);
#endif
			if (err)
				return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, err, DL_SYSERR);
			/*
			 * Get ack buffer if current mp is insufficient
			 */
			ack_size = DL_PHYS_ADDR_ACK_SIZE + dlb->dlb_addr_length;

			if ( mp->b_datap->db_lim - mp->b_datap->db_base < ack_size ) {
				freemsg(mp);
				if (!(mp = allocb(ack_size, BPRI_HI)))
					return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, ENOMEM, DL_SYSERR);
			} else 
				mp->b_rptr = mp->b_datap->db_base;
			/*
			 * Ack with appropriate address (DEFAULT or CURRENT
			 * both supplied by ioctl.
			 */
			mp->b_datap->db_type = M_PCPROTO;
			mp->b_wptr = mp->b_rptr + ack_size;

			dlpha = (dl_phys_addr_ack_t *)mp->b_rptr;
			dlpha->dl_primitive = DL_PHYS_ADDR_ACK;
			dlpha->dl_addr_length = dlb->dlb_addr_length;
			dlpha->dl_addr_offset = DL_PHYS_ADDR_ACK_SIZE;

			if (dlphr->dl_addr_type == DL_FACT_PHYS_ADDR)
				bcopy((char *)&ifd->default_pa[0], 
				      (char *)&dlpha[1],
				      (int)dlb->dlb_addr_length);
			else
				bcopy((char *)&ifd->current_pa[0], 
				      (char *)&dlpha[1],
				      (int)dlb->dlb_addr_length);
			return mp;
		}
		break;
	default:
		break;
	}
	return dlb_error_ack(mp, DL_PHYS_ADDR_REQ, 0, DL_NOTSUPPORTED);
}

/*
 *	DL_SET_PHYS_ADDR_REQ
 *	Sets the physical address value for all streams for a particular PPA.
 *
 * 	DL_ENABMULTI_REQ	
 * 	Request to enable specific multicast addresses on a per streams basis.
 *
 * 	DL_DISABMULTI_REQ	
 * 	Request to disable specific multicast addresses on a per streams basis.
 * 
 */
staticf MBLKP
dlb_set_phys_multi_addr(dlb, mp)
	DLBP	dlb;
	MBLKP	mp;
{
	dl_set_phys_addr_req_t * dlspr;
	IFP ifp;
	DLIFP dlif;
	int err = 0, fltr;
	caddr_t src;
	union dli_addr addr;

	dlspr = (dl_set_phys_addr_req_t *) mp->b_rptr;
	ifp = dlb->dlb_ifp;

	if (ifp == NULL || dlb->dlb_filters[0] == NULL)  /* Unattached */
		return dlb_error_ack (mp, dlspr->dl_primitive, 0, DL_OUTSTATE);
	dlif = dli_ifp2dlif(ifp);

	if (dlspr->dl_addr_length != dlb->dlb_addr_length)
	    return dlb_error_ack (mp, dlspr->dl_primitive, 0, DL_BADADDR);
	/*
	 * Get the address supplied to be set and call ioctl
	 */
	src = (char *) dlspr + dlspr->dl_addr_offset;

	addr.un_addr[0] = src[0];
	addr.un_addr[1] = src[1];
	addr.un_addr[2] = src[2];
	addr.un_addr[3] = src[3];
	addr.un_addr[4] = src[4];
	addr.un_addr[5] = src[5];
	addr.un_addr[6] = 0;
	addr.un_addr[7] = 0;

	switch (dlspr->dl_primitive) {
	    case DL_SET_PHYS_ADDR_REQ:
	    case DL_ENABMULTI_REQ: {
		int errs, fltrs;
		for (fltr = 0, fltrs = 0, errs = 0; fltr < DLB_MAX_FILTERS; fltr++) {
		    if (dlb->dlb_filters[fltr]) {
			int err2;
			fltrs++;
			err2 = dli_addrset_add(dlif, dlb->dlb_filters[fltr],
					      &dlb->dlb_filters[fltr]->fltr_addrset, &addr);
			if (err == 0 && err2 != 0) {
			    err = err2;
			    errs++;
			}
		    }
		}
		if (fltrs > errs)
		    err = 0;
		break;
	    }
	    case DL_DISABMULTI_REQ: {
		for (fltr = 0; fltr < DLB_MAX_FILTERS; fltr++) {
		    if (dlb->dlb_filters[fltr]) {
			(void) dli_addrset_remove(dlif, dlb->dlb_filters[fltr],
					      &dlb->dlb_filters[fltr]->fltr_addrset, &addr);
		    }
		}
		break;
	    }
	}
#if	DL_DEBUG
	if (err) printf ("dlb_set_phys_multi_addr: interface returned 0x%x\n", err);
#endif
	if (err)
	    return dlb_error_ack(mp, dlspr->dl_primitive, err, DL_SYSERR);
	return dlb_ok_ack(mp, dlspr->dl_primitive);
}

staticf int
dlb_wput (q, mp)
	queue_t	* q;
	MBLKP	mp;
{
	DLBP	dlb;
	dl_unitdata_req_t * dlur;
	int		err;
	int		len;
	MBUFP		m;
	IFP		ifp;
	struct iocblk	* iocp;
	char		* src;

	switch (mp->b_datap->db_type) {
	case M_PROTO:
	case M_PCPROTO:
		break;
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case ND_GET:
		case ND_SET:
			if ( nd_getset(q, dlb_g_nd, mp) ) {
				qreply(q, mp);
				return 0;
			}
			/* fallthrough */
		default:
			mp->b_datap->db_type = M_IOCNAK;
			iocp->ioc_error = ENOENT;
			iocp->ioc_count = 0;
			qreply(q, mp);
		}
		return 0;
	case M_FLUSH:
		/* TODO: do it */
	default:
		freemsg(mp);
		return 0;
	}
	dlb = (DLBP)q->q_ptr;
	dlur = (dl_unitdata_req_t *)mp->b_rptr;
	if (dlur->dl_primitive == DL_UNITDATA_REQ) {
		union dli_addr addr;
		if (!dlb->dlb_ifp
		|| dlur->dl_dest_addr_length < dlb->dlb_addr_length
		|| dlur->dl_dest_addr_offset < DL_UNITDATA_REQ_SIZE
		|| (mp->b_wptr - mp->b_rptr) < dlur->dl_dest_addr_offset + dlur->dl_dest_addr_length
		|| !mp->b_cont) {
			freemsg(mp);
			return 0;
		}
		m = mblk_to_mbuf(mp->b_cont, M_DONTWAIT);
		if ( !m ) {
			freemsg(mp);
			return 0;
		}
		src = (char *) dlur + dlur->dl_dest_addr_offset;
		addr.un_l_addr[1] = 0;
		addr.un_addr[0] = src[0];
		addr.un_addr[1] = src[1];
		addr.un_addr[2] = src[2];
		addr.un_addr[3] = src[3];
		addr.un_addr[4] = src[4];
		addr.un_addr[5] = src[5];
		(void) freeb(mp);
		err = dli_output(dli_ifp2dlif(dlb->dlb_ifp), dlb->dlb_filters[0], m, &addr);
		/* Interface frees mbuf chain if there is a problem. */
#if	DL_DEBUG
if (err) printf("dlb_wput: interface returned 0x%x\n", err);
#endif
		return 0;
	}
	switch (dlur->dl_primitive) {
	case DL_ATTACH_REQ:
		mp = dlb_attach(dlb, mp);
		break;
	case DL_DETACH_REQ:
		mp = dlb_detach(dlb, mp);
		break;
	case DL_BIND_REQ:
		mp = dlb_bind(dlb, mp);
		break;
	case DL_SUBS_BIND_REQ:
		mp = dlb_subs_bind(dlb, mp);
		break;
	case DL_INFO_REQ:
		mp = dlb_info(dlb, mp);
		break;
	case DL_UNBIND_REQ:
		mp = dlb_unbind(dlb, mp);
		break;
	case DL_SUBS_UNBIND_REQ:
		mp = dlb_subs_unbind(dlb, mp);
		break;
	case DL_PROMISCON_REQ:
		mp = dlb_promiscon(dlb, mp);
		break;
	case DL_PROMISCOFF_REQ:
		mp = dlb_promiscoff(dlb, mp);
		break;
	case DL_PHYS_ADDR_REQ:
		mp = dlb_phys_addr(dlb, mp);
		break;
	case DL_SET_PHYS_ADDR_REQ:
	case DL_ENABMULTI_REQ:
	case DL_DISABMULTI_REQ:
		mp = dlb_set_phys_multi_addr(dlb, mp);
		break;
	default:
		mp = dlb_error_ack(mp, dlur->dl_primitive, 0, DL_BADPRIM);
		break;
	}
	qreply(q, mp);
	return 0;
}

/* OSF configuration routine for the dlb device. */
int
strdlb_configure(op, indata, indatalen, outdata, outdatalen)
        sysconfig_op_t  op;
        str_config_t *  indata;
        size_t          indatalen;
        str_config_t *  outdata;
        size_t          outdatalen;
{
        struct streamadm        sa;
        dev_t                   devno;
	extern dev_t clonedev;

        if (op != SYSCONFIG_CONFIGURE)
                return EINVAL;

        if (indata != NULL && indatalen == sizeof(str_config_t)
                        && indata->sc_version == OSF_STREAMS_CONFIG_10)
                devno = indata->sc_devnum;
        else
                devno = NODEV;

        sa.sa_version           = OSF_STREAMS_10;
        sa.sa_flags		= STR_IS_DEVICE|STR_SYSV4_OPEN;
        sa.sa_ttys              = 0;
        sa.sa_sync_level        = SQLVL_MODULE;
        sa.sa_sync_info         = 0;
        strcpy(sa.sa_name,      "dlb");

        if ((devno = strmod_add(devno, &dlbinfo, &sa)) == NODEV) {
                return ENODEV;
        }

        if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
                outdata->sc_version = OSF_STREAMS_CONFIG_10;
                outdata->sc_devnum = makedev(major(clonedev), major(devno));
                outdata->sc_sa_flags = sa.sa_flags;
                strcpy(outdata->sc_sa_name, sa.sa_name);
        }

        return 0;
}
