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
/*
 * $oldlog: xtiso.c,v $
 *
 * 1993/03/02	Sandeep Shah
 *	Don't ignore the sosend errors.  Propagate them back to the user
 *
 * 1993/03/02	Sandeep Shah
 *	Added support for socket level manipulation through streams_ioctl().
 *	Pass-through to so{getopt,setopt}.
 *
 * 1993/01/28	Sandeep Shah
 *	Don't change the current state of the connection in case of
 *	TOUTSTATE errors.  Also, make sure that you don't go to TS_BAD_STATE
 *	for any event.
 *
 * 1993/01/25   Sandeep Shah
 *	free the mbufs in xti_info_req, in case of sogetopt() error.
 *
 * 1993/01/14   Sandeep Shah
 *	turned off unconditional printf for xti_optmgmt_req; made conditional.
 *
 * 1992/12/16   Carol Frampton
 *      Add sogetopt(TOPT_XTIINFO) to xti_info_req() to retrieve the
 *	info from osi transport which can change depending on the state
 *	of the connection.
 *
 * 1992/12/11   Ajay Kachrani
 *	Add soget
 *      Move xti_prec_insq default case for TPI message switch.
 *
 * 1992/11/17	Ajay Kachrani
 *	Call xport for disconnect reason, In order to support unlimited tsdus
 *	stop buffering data in xtiso for record-oriented xport (aka osi) and
 *	set sosend flag to be MSG_EOR when T_MORE is unset!
 *
 * 1992/10/19	Ajay Kachrani
 *	Use SB_ASYNC, not SB_SEL in sockbuf - fixes from OSF1.1b30 pool
 *
 *	This code has been heavily modified.  The OSF1.1BL29 code has been 
 *	back-ported into this DEC Silver kernel.  Code has also been added 
 *	to be tied into the DECNET/OSI code.
 *
 *	The XTI11 define, if defined, means that we are running in an OSF 1.1 
 *	kernel.  Note that throughout this file, all *devp have been changed 
 *	to dev.  Therefore, if you want XTI11 to work, you'll need to change 
 *	the dev's back to *devp's (make sure you check every occurence, and 
 *	don't just do a global replace).
 * $EndLog$
 * $EndLog$
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: xtiso.c,v $ $Revision: 4.4.17.14 $ (DEC) $Date: 1994/01/24 21:13:06 $";
#endif 
/**
 ** XTI-over-SOcket Pseudo-Device Driver
 **  (xtiso)
 **/
/* Socket include files */
#include <sys/param.h>
#include <net/net_globals.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/time.h>
#include <sys/protosw.h>
#include <sys/ioctl.h>

/* Configuration definitions */
#include <sys/sysconfig.h>

#define XTI11
#ifndef XTI11
#include <sys/user.h>		/* XXX before stream.h */
#endif
#include <sys/uio.h>
#ifdef 1.1ALLOC
#include <sys/malloc.h>
#endif
#include <kern/lock.h>
#include <sys/fcntl.h>

/* STREAMS include files */
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/strlog.h>
#include <streams/memdebug.h>


#define XTI_CONFIGURABLE_QLEN

/* TLI include files */
#define XTI_XPG4 0 /* untested */
#include <streamsm/tihdr.h>
#include <streamsm/xti.h>
#include <streamsm/xtiso.h>
#include <streamsm/xtiext.h>

/* stream data structure definitions */
#ifdef XTI11
int xtiopen  (queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp);
int xticlose (queue_t *q, int flag, cred_t *credp);
#else
int xtiopen  (queue_t *q, dev_t dev, int flag, int sflag);
int xticlose (queue_t *q, int flag);
#endif
int xtiwput  (queue_t *q, mblk_t *mp);
int xtirsrv  (queue_t *q);
int xtiwsrv  (queue_t *q);

/* callback from sockets */
void xti_rqenable  (caddr_t q, int state);
void xti_wqenable  (caddr_t q, int state);

/* Socket glue */
#ifdef	COMPAT_43
#define SOCKNAME_FORMAT	1
#else
#define SOCKNAME_FORMAT	0
#endif

extern xtiso_inadm_t	xti_proto_info[];	/* configurable protocol info */
extern int		xti_nprotos;		/* How many protocols supp. */
extern int              clonedev;

static int xtisoConfigure;
static int xtisoInit;
decl_simple_lock_data(static, xtisoLock)

/*
 * Head of configured "invocations".
 */
struct xtisocfg *xtisocfg;

#ifndef XTI11
extern int nfile;	/* to get the file table size */
#endif

static struct module_info xti_info =
{
	XTI_INFO_ID, 	/* module ID number */
	"xtiso",	/* module name 	*/
	0,		/* min pkt size accepted */
	INFPSZ, 	/* max pkt size accepted */
	XTI_INIT_HI,	/* hi-water mark, flow control */
	XTI_INIT_LO	/* lo-water mark, flow control */
};

/* Read queue init structure */
static struct qinit xtirinit =
{
	NULL,	 			/* put procedure */
	xtirsrv, 			/* service procedure */
	xtiopen, 			/* called every open or push */
	xticlose, 			/* called every close or pop */
	NULL,	 			/* admin - reserved */
	&xti_info,			/* information structure */
	(struct module_stat *)0		/* stats structure - unused */
};

/* Write queue init structure */
static struct qinit xtiwinit =
{
	xtiwput,   			/* put procedure */
	xtiwsrv, 			/* service procedure */
	NULL,				/* open procedure - unused */
	NULL, 				/* close procedure - unused */
	NULL, 				/* admin - reserved */
	&xti_info,			/* info structure */
	(struct module_stat *)0		/* stats structure - unused */
};

/* Basic STREAMS data structure */
static struct streamtab xtisoinfo =
{
	&xtirinit, 		/* read queue init */
	&xtiwinit,		/* write queue init */
	(struct qinit *)0,	/* read queue init - mux driver (unused) */
	(struct qinit *)0	/* write queue init - mux driver (unused) */
};

typedef struct iocblk *IOCP;
int xti_bind_req(), xti_snd_ok_ack(), xti_conn_req(), xti_conn_res(), 
    xti_discon_req(), xti_cleanup();

/*
 * TPI Finite State Machine - State Transition Table
 *
 * Beware: states and events match symbolic definitions in tihdr.h.
 * Table is rooted at 0, states, events at 1.
 * Is state 18 (TS_WACK_ORDREL) useful?
 */

#if	(TE_NOEVENTS != 29) || (TS_NOSTATES != 19)
#error	XTI state table out of sync - TE_NOEVENTS,TS_NOSTATES != 29,19
#endif

#define TS_BAD_STATE	(-1)

#if	XTIDEBUG

int xtiDEBUG = 0;

#define strlog \
	if (xtiDEBUG & XTIF_STRLOG) xti_strlog /* (args follow) */

#define TNEXTSTATE(x,e) \
	((((x)->xti_state>0) && (unsigned)((x)->xti_state) < TS_NOSTATES) ? \
		xti_fsm[e][(x)->xti_state] : TS_BAD_STATE)

#define NEXTSTATE(x,e,s) \
	((((x)->xti_state>0) && (unsigned)((x)->xti_state) < TS_NOSTATES) ? \
		((x)->xti_state = xti_fsm[e][(x)->xti_state]) : \
		(xti_panic(s), ((x)->xti_state = TS_BAD_STATE)))

#define allocb xti_allocb
mblk_t *xti_allocb(int size, int pri);

#else	/* !XTIDEBUG */

#define TNEXTSTATE(x,e)		xti_fsm[e][(x)->xti_state]
#define NEXTSTATE(x,e,s)	(x)->xti_state = xti_fsm[e][(x)->xti_state]
#define xti_panic(s)		panic(s)

#endif

#define xx TS_BAD_STATE
static short xti_fsm[TE_NOEVENTS][TS_NOSTATES] = {
/* TE_...   | TS_...  N/A 1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18*/
/* 0  N/A         */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 1  BIND_REQ    */ {xx, 2,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 2  UNBIND_REQ  */ {xx,xx,xx,xx, 3,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 3  OPTMGMT_REQ */ {xx,xx,xx,xx, 5,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 4  BIND_ACK    */ {xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 5  OPTMGMT_ACK */ {xx,xx,xx,xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 6  CONN_REQ    */ {xx,xx,xx,xx, 6,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 7  CONN_RES    */ {xx,xx,xx,xx,xx,xx,xx,xx, 9,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 8  DISCON_REQ  */ {xx,xx,xx,xx,xx,xx,xx,13,14,xx,15,16,17,xx,xx,xx,xx,xx,xx},
/* 9  DATA_REQ    */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,10,xx,12,xx,xx,xx,xx,xx,xx},
/* 10 EXDATA_REQ  */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,10,xx,12,xx,xx,xx,xx,xx,xx},
/* 11 ORDREL_REQ  */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,11,xx, 4,xx,xx,xx,xx,xx,xx},
/* 12 CONN_IND    */ {xx,xx,xx,xx, 8,xx,xx,xx, 8,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 13 CONN_CON    */ {xx,xx,xx,xx,xx,xx,xx,10,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 14 DATA_IND    */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,10,11,xx,xx,xx,xx,xx,xx,xx},
/* 15 EXDATA_IND  */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,10,11,xx,xx,xx,xx,xx,xx,xx},
/* 16 ORDREL_IND  */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,12, 4,xx,xx,xx,xx,xx,xx,xx},
/* 17 DISCON_IND1 */ {xx,xx,xx,xx,xx,xx,xx, 4,xx,xx, 4, 4, 4,xx,xx,xx,xx,xx,xx},
/* 18 DISCON_IND2 */ {xx,xx,xx,xx,xx,xx,xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 19 DISCON_IND3 */ {xx,xx,xx,xx,xx,xx,xx,xx, 8,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 20 ERROR_ACK   */ {xx,xx,xx, 4,xx, 4, 4,xx,xx, 8,xx,xx,xx, 7, 8,10,11,12,xx},
/* 21 OK_ACK1     */ {xx,xx,xx, 1,xx,xx, 7,xx,xx,xx,xx,xx,xx, 4,xx, 4, 4, 4,xx},
/* 22 OK_ACK2     */ {xx,xx,xx,xx,xx,xx,xx,xx,xx,10,xx,xx,xx,xx, 4,xx,xx,xx,xx},
/* 23 OK_ACK3     */ {xx,xx,xx,xx,xx,xx,xx,xx,xx, 4,xx,xx,xx,xx, 4,xx,xx,xx,xx},
/* 24 OK_ACK4     */ {xx,xx,xx,xx,xx,xx,xx,xx,xx, 8,xx,xx,xx,xx, 8,xx,xx,xx,xx},
/* 25 PASS_CONN   */ {xx,xx,xx,xx,10,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 26 UNITDATA_REQ*/ {xx,xx,xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 27 UNITDATA_IND*/ {xx,xx,xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx},
/* 28 UDERROR_IND */ {xx,xx,xx,xx, 4,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx,xx}
};
#undef xx


/*
 * =========================
 * XTI Driver configuration.
 * =========================
 */

xtiso_configure(op, indata, indatalen, outdata, outdatalen)
        sysconfig_op_t  op;
        xtiso_inadm_t * indata;
        size_t          indatalen;
        xtiso_outadm_t *outdata;
        size_t          outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;
	struct xtisocfg		*xc;
#ifndef XTI11
	int			xcsize;
#endif
	int			error;

	CHECKPOINT("xtiso_configure");

	XTITRACE(XTIF_CONFIGURE,
		printf(" xtiso: xtiso_configure op=%d in=%x[%d], out=%x[%d]\n",
			op, indata, indatalen, outdata, outdatalen););

	switch (op) {
	case SYSCONFIG_CONFIGURE:
		if (xtisoInit == 0) {	/* Unsafe, but reasonable */
			++xtisoInit;
			simple_lock_init(&xtisoLock);
		}
		if (indata == NULL || (indatalen == sizeof(str_config_t) &&
		    (indata->sc.sc_version == OSF_STREAMS_CONFIG_11 ||
		     indata->sc.sc_version == OSF_STREAMS_CONFIG_10))) {
			/* For calling from str_config.c */
		        if (xtisoConfigure >= xti_nprotos)
				return EADDRINUSE;
			XTITRACE(XTIF_CONFIGURE,
				 printf("Configure name: %s\n", /* DEBUG */
					xti_proto_info[xtisoConfigure].sc.sc_sa_name););
			if (indata == NULL) {
				indata = &xti_proto_info[xtisoConfigure++];
				indatalen = sizeof *indata;
				devno = indata->sc.sc_devnum; /* Likely NODEV */
			} else {
				devno = indata->sc.sc_devnum;
				indata = &xti_proto_info[xtisoConfigure++];
				indatalen = sizeof *indata;
			}
		} else if (indata != NULL && indatalen == sizeof(xtiso_inadm_t)
			   && indata->sc.sc_version == OSF_XTISO_CONFIG_10) {
			devno = indata->sc.sc_devnum;
		} else {
		        XTITRACE(XTIF_CONFIGURE, printf(" indata combinations invalid\n"););
			return EINVAL;
		}

		sa.sa_version		= OSF_STREAMS_11;
#ifdef XTI11
		sa.sa_flags		= STR_IS_DEVICE|STR_SYSV4_OPEN;
#else
		sa.sa_flags		= STR_IS_DEVICE;
#endif
		sa.sa_ttys		= 0;
		sa.sa_sync_level 	= SQLVL_QUEUEPAIR;
		sa.sa_sync_info		= 0;
		strcpy(sa.sa_name, indata->sc.sc_sa_name);

		if ((devno = strmod_add(devno, &xtisoinfo, &sa)) == NODEV)
		{
		        XTITRACE(XTIF_CONFIGURE,
				 printf(" strmod_add returned %d - invalid\n", devno););
			return ENODEV;
		}

		if (outdata != NULL && outdatalen >= sizeof(str_config_t)) {
			bzero(outdata, outdatalen);
			outdata->sc.sc_sa_flags = indata->sc.sc_sa_flags;
			outdata->sc.sc_version = indata->sc.sc_version;
			outdata->sc.sc_devnum =
				makedev(major(clonedev), major(devno));
			strcpy(outdata->sc.sc_sa_name, sa.sa_name);
		}

#ifdef XTI11
#define xcsize	sizeof(struct xtisocfg)
#else
		/* 
		 * calculate what's the size required 
		 * For each minor dev number we need a bit, max possible
		 * opens are 'nfile' (system wide), so it is fair to assume
		 * some derivative thereof.  Currently lets approximate
		 * that number for each protocol open.
		 *
		 */
		xcsize = sizeof(struct xtisocfg) + nfile >> 3;
#endif	/* ifdef-else XTI11 */
#ifdef 1.1ALLOC
		MALLOC(xc, struct xtisocfg *, xcsize, M_STREAMS, M_WAITOK);
#else
		xc = (struct xtisocfg *)kalloc(xcsize);
#endif
		bzero((caddr_t)xc, xcsize);
		xc->xti_cfgmajor = major(devno);
		simple_lock_init(&xc->xti_cfglock);
		bcopy((caddr_t)&indata->proto,	/* XXX No sanity check... */
			(caddr_t)&(xc->xti_cfgproto), sizeof xc->xti_cfgproto);

		simple_lock(&xtisoLock);
		xc->xti_cfgnext = xtisocfg;
		xtisocfg = xc;
		simple_unlock(&xtisoLock);
		XTITRACE(XTIF_CONFIGURE, printf("dom=%d type=%d proto=%d serv=%d size=%d\tdone\n",
						xc->xti_cfgproto.xp_dom,
						xc->xti_cfgproto.xp_type,
						xc->xti_cfgproto.xp_proto,
						xc->xti_cfgproto.xp_servtype,
						xcsize););
		break;

#ifdef NEW_CONFIGURE
	case SYSCONFIG_QUERYSIZE:
		*(int *)outdata = indatalen;
		break;

	case SYSCONFIG_QUERY:
		if (outdatalen >= indatalen) {
			bcopy(indata,outdata,indatalen);
			break;
		}
		return ENOMEM;

	case SYSCONFIG_RECONFIGURE:
		return EINVAL;

	case SYSCONFIG_UNCONFIGURE:
		return EINVAL;
		break;
#endif
	default:
		return EINVAL;
	}

	return 0;
}

xtiso_unconfigure(dev, sa)
	dev_t			dev;
	struct streamadm	sa;
{
	CHECKPOINT("xtiso_unconfigure");

	/*
	 * NOTE: tbd
	 *
	 * Lock xtisocfg list.
	 * Find xtisocfg associated with this unconfig.
	 * If cfgnopen != 0, unlock, return in-use.
	 * Remove from xtisocfg list.
	 * Unlock xtisocfg list.
	 * Call strmod_del().
	 * Error, optionally restore to list.
	 * Free xtisocfg struct.
	 * Return.
	 */

	return EINVAL;
}


/* streams.h is redefining mfree to be streams_mfree.  This seems to
   be causing us no end of grief. */
#undef mfree

/*
 * Some basic assumptions about mbufs being passed down to the transport
 * layer include:
 *
 *  sosetopt: xtiso allocates
 *            tp owns
 *            pass mbuf *
 *
 *  sogetopt: tp allocates
 *            xtiso owns
 *	     pass mbuf **
 */
struct mbuf
*xti_get_mbuf(int size)
{
    struct mbuf *m;

    m = m_get(M_WAIT, MT_SOOPTS);
    if (m == NULL) {
#if XTIDEBUG
	printf("xti_get_mbuf: Couldn't get mbuf");
#endif
	goto get_mbuf_out;
    }
    if (size > MLEN)
    {
	MCLGET(m, M_WAIT);
	if ((m->m_flags & M_EXT) == NULL)
	{
	    (void) m_free (m);
	    m = (struct mbuf *)0;
#if XTIDEBUG
	    printf("xti_get_mbuf: Couldn't get cluster");
#endif
	}
    }
get_mbuf_out:
    return(m);
}


/*
 * ==========================
 * XTI Open/close procedures.
 * ==========================
 */

/*
 * xtiopen - Driver open procedure.
 *
 * Inputs:
 *	q	= read queue pointer
 *	devp	= major/minor device number
 *	flag	= file open flag
 *	sflag	= STREAMS open flag (ie. CLONEOPEN)
 *	credp	= STREAMS open credentials
 *
 * Outputs:
 *	None
 *
 * Return:
 *	If success, 0 and stores minor number in dev if cloning.
 *	Else, error.
 */
#ifdef XTI11
xtiopen(q, devp, flag, sflag, credp)
        queue_t *q;		/* read queue pointer */
        dev_t   *devp;
        int     flag;
        int     sflag;
	cred_t	*credp;
#else
xtiopen(q, dev, flag, sflag)
        queue_t *q;		/* read queue pointer */
        dev_t   dev;
        int     flag;
        int     sflag;
#endif
{
	struct xticb    *xtip;
	struct xtisocfg *xc;
	int error, device;

	CHECKPOINT("xtiopen");

	/* xxx
	 * NOTE - this routine had a bug in that it was returning a 
	 *        'errno' value instead of OPENFAIL on failure. This 
	 *        has been fixed; however, we do not assign the error 
	 *        code to u.u_error to minimize changes. In any case 
	 *        it currently gets overwritten by the calling code. 
	 *        In some future release, the assignment to u.u_error 
	 *        in the ERROR_RETURN macro should be uncommented if the 
	 *        overwriting of the error value by caller is fixed.
	 *        
	 * Also, I assume XTI11 does in fact expect the error code as 
	 * return value. The macro is defined accordingly and might 
	 * need to be changed if that is not the case.
	 */
#ifdef XTI11
#define	ERROR_RETURN(errorcode)	return errorcode
#else
#define	ERROR_RETURN(errorcode) \
    do { \
	 /* u.u_error = errorcode; */ \
	 return OPENFAIL; \
    } while (0)
#endif
	XTITRACE(XTIF_OPEN,
		printf(" xtiso: xtiopen() - q=%x OTHERQ(q)=%x, *devp=%d flag=%x sflag=%x\n",
			q, OTHERQ(q), *devp, flag, sflag););

#ifdef XTI11
	if (xtisoInit == 0 || *devp == 0)
#else
	if (xtisoInit == 0)
#endif
		ERROR_RETURN(ENXIO);
	
	if (sflag & CLONEOPEN) {
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: this is a CLONEOPEN of %d\n",
				major(*devp)););
#ifndef XTI11
		device = NODEV;
		dev = makedev(major(dev), minor(NODEV));
#endif
	} else {
		XTITRACE(XTIF_OPEN, printf(" xtiso: this is an OPEN of %d/%d\n",
			major(*devp), minor(*devp)););
		xtip = (struct xticb *)q->q_ptr;
		if (xtip) {			/* Already open. */
			XTITRACE(XTIF_OPEN,
				printf(" xtiso: found xticb %x %d/%d\n",
					xtip, xtip->xti_cfg->xti_cfgmajor,
					xtip->xti_minor););
			if (xtip->xti_cfg->xti_cfgmajor != major(*devp))
				xti_panic("xtiopen major botch");
			if (xtip->xti_minor != minor(*devp))
				xti_panic("xtiopen minor botch");
#ifdef XTI11
			return 0;
#else
			return xtip->xti_minor;
#endif
		}
#ifndef XTI11
		device = minor(dev);
#endif
	}

	/* Find config list head. */
	simple_lock(&xtisoLock);
	for (xc = xtisocfg; xc; xc = xc->xti_cfgnext)
		if (major(*devp) == xc->xti_cfgmajor) {
			xc->xti_cfgnopen++;	/* Take reference */
			break;
		}
	simple_unlock(&xtisoLock);

	if (xc == 0)
	{
	    XTITRACE(XTIF_OPEN,
		     printf("xtiopen: xc == 0\n"););
	    ERROR_RETURN(ENOPROTOOPT);
	}

#ifdef XTI11
	if (error = streams_open_comm(sizeof *xtip, q, devp, flag, sflag, credp)) {
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: xtiopen() cdevsw_open_comm() failed %d\n", error););
		simple_lock(&xtisoLock);
		--xc->xti_cfgnopen;
		simple_unlock(&xtisoLock);
		ERROR_RETURN(error);
	}
	xtip = (struct xticb *)q->q_ptr;
#else
	if (!(xtip = (struct xticb *)kalloc(sizeof *xtip)))
	{
	    XTITRACE(XTIF_OPEN,
		     printf("xtiopen() kalloc for xtip failed!\n"););
	    simple_lock(&xtisoLock);
	    --xc->xti_cfgnopen;
	    simple_unlock(&xtisoLock);
	    ERROR_RETURN(ENOMEM);
	}
	bzero((caddr_t)xtip, sizeof *xtip);
#endif
	XTITRACE(XTIF_OPEN,
		printf(" xtiso: xtiopen() xtip 0x%x minor %d\n",
			xtip, minor(*devp)););

	xtip->xti_cfg = xc;
#ifdef XTI11
	xtip->xti_minor = minor(*devp);
#else
	xtip->xti_minor = device;

	if ((device = xti_allocate_minor(xtip)) == NODEV)
	{
#ifdef XTI_CONFIGURABLE_QLEN
	    kfree((caddr_t)xtip->xti_seq, (sizeof *xtip->xti_seq) * xtip->xti_maxqlen);
#endif
	    kfree((caddr_t)xtip, sizeof *xtip);
	    simple_lock(&xtisoLock);
	    --xc->xti_cfgnopen;
	    simple_unlock(&xtisoLock);
	    ERROR_RETURN(NODEV);
	}

	dev = makedev(major(dev), minor(device));
#endif

	xtip->xti_maxqlen	= somaxconn;
#ifdef XTI_CONFIGURABLE_QLEN
	if (!(xtip->xti_seq = (struct xtiseq *)kalloc((sizeof *xtip->xti_seq) * xtip->xti_maxqlen)))
	{
	    XTITRACE(XTIF_OPEN | XTIF_CLOSE,
		     printf(" xtiso: xtiopen() - Couldn't kalloc for xti_seq struct\n"););
	    simple_lock(&xtisoLock);
	    --xc->xti_cfgnopen;
	    simple_unlock(&xtisoLock);
	    ERROR_RETURN(ENOMEM);
	}

	bzero((caddr_t)xtip->xti_seq, (sizeof *xtip->xti_seq) * xtip->xti_maxqlen);
#endif

	if (error = xti_entry_init(xtip, q, XTI_NEWSOCK)) {
#ifdef XTI11
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: xtiopen() failed to create socket\n"););
		simple_lock(&xtisoLock);
		--xc->xti_cfgnopen;
		simple_unlock(&xtisoLock);
		(void) streams_close_comm(q, flag, credp);
		ERROR_RETURN(error);
#else
		xti_deallocate_minor(xtip);
		XTITRACE(XTIF_OPEN,
			printf(" xtiso: xtiopen() failed to create socket\n"););
#ifdef XTI_CONFIGURABLE_QLEN
		kfree((caddr_t)xtip->xti_seq, (sizeof *xtip->xti_seq) * xtip->xti_maxqlen);
#endif
		kfree((caddr_t)xtip, sizeof *xtip);
		simple_lock(&xtisoLock);
		--xc->xti_cfgnopen;
		simple_unlock(&xtisoLock);
		ERROR_RETURN(error);
#endif
	}

	/*
	 * Global sequence # initialized only once per STREAM/Socket open
	 */
	xtip->xti_seqcnt = 1;

	/*
	 * Set SS_PRIV if suser.
	 * It will be turned off after the first bind.
	 */
#ifdef XTI11
	if (drv_priv(credp) == 0) {
		xtip->xti_flags |= XTI_PRIV;
		xtip->xti_so->so_state |= SS_PRIV;
	}
	return 0;
#else
	xtip->xti_flags |= XTI_PRIV;
	xtip->xti_so->so_state |= SS_PRIV;

	/*
	 * Set both Read/Write queue private structure to current
	 * xtiControlBlock
	 */
	q->q_ptr = OTHERQ(q)->q_ptr = (caddr_t)xtip;
	return device;
#endif
#undef ERROR_RETURN
}

/*
 * xticlose - Driver close procedure.
 */
#ifdef XTI11
xticlose(q, flag, credp)
	register queue_t *q;
	int flag;
	cred_t *credp;
#else
xticlose(q, flag)
	register queue_t *q;
	int flag;
#endif
{
	register struct xticb *xtip;
	int error = 0, error2;

	CHECKPOINT("xticlose");

	/*
	 * If xti context block pointer is NULL, then
	 * no more cleanup
	 */
	if (xtip = (struct xticb *)q->q_ptr) {
		if (! (flag & (O_NDELAY | O_NONBLOCK))) {
			xtip->xti_flags |= XTI_ISCLOSING;
			while ( OTHERQ(q)->q_first || xtip->xti_pendcall) { 
  				if (error = tsleep((caddr_t)xtip, 
				    (PZERO + 1) | PCATCH, "xtiso", 0))
					break;
			}
		}
	/*
	 * Flush data/etc out of our queues
	 */
		flushq(OTHERQ(q), FLUSHALL);
		/*
		 * Initiate general cleanup
		 */
		(void) xti_finished(xtip, XTI_CLOSESOCK);

		/*
		 * Unlink and free structure.
		 */
		simple_lock(&xtisoLock);
		--xtip->xti_cfg->xti_cfgnopen;
		simple_unlock(&xtisoLock);
	}

#ifdef XTI11
#ifdef XTI_CONFIGURABLE_QLEN
	/* Support for configurable somaxconn */
	if (xtip->xti_seq && xtip->xti_maxqlen)
	    kfree((caddr_t)xtip->xti_seq,
			    (sizeof *xtip->xti_seq) * xtip->xti_maxqlen);
#endif
	error2 = streams_close_comm(q, flag, credp);
	if (error)
		return error;

	return error2;
#else
	/*
	 * Clear RD(q) and WR(q) private data pointers
	 */
	q->q_ptr = OTHERQ(q)->q_ptr = nil(caddr_t);

	xti_deallocate_minor(xtip);

#ifdef XTI_CONFIGURABLE_QLEN
	/* Support for configurable somaxconn */
	if (xtip->xti_seq && xtip->xti_maxqlen)
	    kfree((caddr_t)xtip->xti_seq,
			    (sizeof *xtip->xti_seq) * xtip->xti_maxqlen);
#endif

	kfree((caddr_t)xtip, sizeof *xtip);
	return(0);
#endif
}



/*
 * ============================
 * XTI Write-side put routines.
 * ============================
 */

/*
 * xtiwput - Driver write-side put procedure.
 *           It validates the message type, perform flush operations,
 *           handles ioctl commands, and puts the message into the
 *           the write-side queue of this stream.
 */
xtiwput(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xtiproto *xp;

	register unsigned int cmd;
	register caddr_t data;
	register struct socket *so;
	int error = 0;
	register mblk_t *mp1;


	CHECKPOINT("xtiwput");

	XTITRACE
	(
		XTIF_WPUT,
		printf(" xtiso: xtiwput() entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
			q, OTHERQ(q), mp);
	);

	pp   = (union T_primitives *)mp->b_rptr;
	xtip = (struct xticb *)q->q_ptr;
	xp   = &(xtip->xti_proto);

	XTITRACE(XTIF_WPUT,
		printf(" xtiso: xtiwput() - M_message type=%d\n",
			(int)mp->b_datap->db_type););

	switch((int)mp->b_datap->db_type) {
	case M_DATA:

		XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - M_DATA\n"););
		xti_prec_insq(q, mp);
		return;

	case M_PROTO:
	case M_PCPROTO:

		/* use smallest primitive for comparison */
		if (mp->b_wptr - mp->b_rptr < sizeof(struct T_info_req)) {
			if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				    SL_TRACE|SL_ERROR,
				    "xti_wput: incorrect message size\n");
				return;
		}

		XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - %s\n",
				(int)mp->b_datap->db_type == M_PROTO ?
					"M_PROTO" : "M_PCPROTO"););

		switch (pp->type) {
		case T_UNITDATA_REQ:

			XTITRACE(XTIF_WPUT,
			    printf(" xtiso: xtiwput() - T_UNITDATA_REQ\n"););
			if (xp->xp_servtype != T_CLTS) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
					"xtiwput: unitdata_req on non-CLTS provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				xti_prec_insq(q, mp);
			return;

		case T_ORDREL_REQ:

			XTITRACE(XTIF_WPUT,
			    printf(" xtiso: xtiwput() - T_ORDREL_REQ\n"););
			if (xp->xp_servtype != T_COTS_ORD) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
					"xtiwput: ordrel_req on non-COTS_ORD provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				xti_prec_insq(q, mp);
			return;

		case T_DATA_REQ:
		case T_EXDATA_REQ:

			XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - %s\n",
			    pp->type == T_DATA_REQ ?
				"T_DATA_REQ" : "T_EXDATA_REQ"););

			if ((xp->xp_servtype != T_COTS_ORD) &&
			    (xp->xp_servtype != T_COTS)) {
				strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
					"xtiwput: data_req on non-COTS provider\n");
				xti_cleanup(q, mp, EPROTO);
			} else
				xti_prec_insq(q, mp);
			return;

		case T_CONN_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_CONN_REQ\n");
				goto next1;);
		case T_CONN_RES:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_CONN_RES\n");
				goto next1;);
		case T_DISCON_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_DISCON_REQ\n");
				goto next1;);
		next1:
			/* Provider will test for COTS/COTS_ORD */
			xti_prec_insq(q, mp);
			return;

		case T_BIND_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_BIND_REQ\n");
				goto next2;);
#if	XTI_XPG4
		case T_GETADDR_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_GETADDR_REQ\n");
				goto next2;);
#endif
		case T_OPTMGMT_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_OPTMGMT_REQ\n");
				goto next2;);
		case T_UNBIND_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_UNBIND_REQ\n");
				goto next2;);
		case T_INFO_REQ:
			XTITRACE(XTIF_WPUT,
				printf(" xtiso: xtiwput() - T_INFO_REQ\n");
				goto next2;);
		next2:
			xti_prec_insq(q, mp);
			return;

		default:
			strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
				"xtiwput: bad TPI message type %d\n", pp->type);
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	case M_IOCTL:

		XTITRACE(XTIF_WPUT, printf(" xtiso: xtiwput() - M_IOCTL\n"););
		XTITRACE(XTIF_WPUT, DumpIOCBLK((IOCP)mp->b_rptr););
		/*
		 *  ioctl's handled only when so endpoint is available
		 */
		so = xtip->xti_so;
		if (!so) {
			XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput()-no so end point\n"););
			goto ioctlnak;
		}
		cmd = ((IOCP)mp->b_rptr)->ioc_cmd;

		/* Now process the command based on it's group */
		switch (XTIOCGROUP(cmd)) {
		    case ('X' & XTIOCGROUP_MASK): /* Generic XTI/socket cmds */
			XTITRACE(XTIF_WPUT,
				 printf("xtiso: Generic XTI cmd = %x\n", cmd);
				);
			{
			struct xtisoioctl	*xsip;
			struct mbuf		*mbufp = (struct mbuf *)0;

			if ( ((IOCP)mp->b_rptr)->ioc_count != 
					sizeof(struct xtisoioctl) ||
			     !(mp1 = mp->b_cont) ) {
				goto ioctlnak;
			}
			xsip = (struct xtisoioctl *)mp1->b_rptr;
#ifdef XTIDEBUG
			/* print the xtisoioctl as we see it */
			XTITRACE(XTIF_OPTMGMT,
				printf("so=%lx, level=%d, optname=%d, optval=%lx\n"
					, so
					, xsip->level
					, xsip->optname
					, xsip->opt.buf););

#endif
			if (cmd == XTI_SOGETOPT) {
				if (error=sogetopt(so
						   , xsip->level
						   , xsip->optname
						   , &mbufp)) {
#if XTIDEBUG
					printf("ioctl(XTI_SOGETOPT): "
						"received error %d\n",
						error);

#endif
					if (mbufp)
						m_free(mbufp);
					goto ioctlnak;
				}
				/* copy the results back */
				if (mbufp->m_len <= xsip->opt.maxlen) {
					if ((xsip->opt.len = mbufp->m_len) == sizeof(int)) {
						*((int *)xsip->opt.buf) = *mtod(mbufp, int *);
					} else {
						 bcopy(mtod(mbufp, caddr_t)
						       , xsip->opt.buf
						       , mbufp->m_len);
					}
					
				} else {
					goto ioctlnak;
				}
				/* return the same buffer */
				((IOCP)mp->b_rptr)->ioc_count = xsip->opt.len;
			} else {
				if (!(mbufp = xti_get_mbuf(xsip->opt.len))) {
					error = ENOBUFS;
					goto ioctlnak;
				}
				if ((mbufp->m_len = xsip->opt.len) == sizeof(int)) {
					*mtod(mbufp, int *) = *((int *)xsip->opt.buf);
				} else {
					 bcopy(xsip->opt.buf
					       , mtod(mbufp, caddr_t)
					       , mbufp->m_len);
				}

				if (error=sosetopt(so
						   , xsip->level
						   , xsip->optname
						   , mbufp)) {
#if XTIDEBUG
					printf("ioctl(XTI_SOSETOPT): "
						"received error %d\n",
						error);
					goto ioctlnak;
#endif
				}
				((IOCP)mp->b_rptr)->ioc_count = 0;
			}
			mp->b_datap->db_type = M_IOCACK;

			qreply(q, mp);

			return;
			}

		    /*
		     * The following is a "jacket" for supporting the standard
		     * ioctl's through XTI.  The trick for making them work
		     * is in encoding the XTI ioctl command (look for example
		     * in xtiext.h).  Although only one command group would 
		     * have been sufficient, three are created so that 
		     * ioctl commands from each transport protocol family
		     * are not required to be unique.  To add a generic
		     * ioctl command that works, just encode a corresponding
		     * XTI ioctl command as explained.
		     *
		     */
		    case ('O' & XTIOCGROUP_MASK):
		    case ('T' & XTIOCGROUP_MASK):
		    case ('I' & XTIOCGROUP_MASK):
			XTITRACE(XTIF_WPUT,
				 printf("xtiso: Generic ioctl cmd = 0x%x, "
					"base=%d, inlen = %d, outlen = %d\n", 
					cmd,
					XTIOCBASECMD(cmd),
					XTIOCPARM_INLEN(cmd),
					XTIOCPARM_OUTLEN(cmd));
				);
			/*
			 * If the count is different than what the command
			 * expects, or if the actual data is unavailable
			 * then return
			 *
			 */
			if ( (((IOCP)mp->b_rptr)->ioc_count != 
			                              XTIOCPARM_INLEN(cmd)) ||
			     (XTIOCPARM_INLEN(cmd) && !(mp1 = mp->b_cont)) )
				goto ioctlnak;
			
			data = (caddr_t)mp1->b_rptr;

			XTITRACE(XTIF_WPUT,
				 printf("Calling PRU_CONTROL(%d, %lx, %d\n",
					XTIOCBASECMD(cmd),
					data,
					(*(int *)data));
				);
			/* Perform the actual work */
			if (error = ((*so->so_proto->pr_usrreq)(so,
					 PRU_CONTROL, 
					 (struct mbuf *)XTIOCBASECMD(cmd), 
					 (struct mbuf *)data, 
					 (struct mbuf *)0)) ) {
#ifdef XTI_DEBUG
			      printf(" xtiso: ioctl cmd err = %d, data=%s", 
				     error, data);
#endif
			      goto ioctlnak;
			}

			mp->b_datap->db_type = M_IOCACK;
			/* Now determine if anything is to be returned */
			((IOCP)mp->b_rptr)->ioc_count = XTIOCPARM_OUTLEN(cmd);

			qreply(q, mp);
			return;

		    default:
			/* fall through in the ioctlnak error handling */
			break;
		} /* switch (cmd) */
ioctlnak:
		XTITRACE(XTIF_WPUT,
		     printf(" xtiso: xtiwput() -M_IOCNAK for cmd=%d\n",
		     ((IOCP)mp->b_rptr)->ioc_cmd););
		mp->b_datap->db_type = M_IOCNAK;
	        ((IOCP)mp->b_rptr)->ioc_error = error ? error : EINVAL;
		((IOCP)mp->b_rptr)->ioc_count = 0;
		qreply(q, mp);
		return;

 	case M_FLUSH:

		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE,
			"xtiwput: M_FLUSH type %d received mp = %x\n",
	       		mp->b_datap->db_type, mp);

		XTITRACE(XTIF_WPUT,
			printf(" xtiso: xtiwput() - M_FLUSH\n"););

		if (*mp->b_rptr & FLUSHW) {
			if (xtip->xti_wndata || xtip->xti_wexpdata) {
				if (xtip->xti_wndata)
					freemsg(xtip->xti_wndata);
				xtip->xti_wndata = 0;

				if (xtip->xti_wexpdata)
					freemsg(xtip->xti_wexpdata);
				xtip->xti_wexpdata = 0;

				xtip->xti_tsdu = xtip->xti_etsdu = 0;

				if (xtip->xti_wnam) {
					(void) m_free(xtip->xti_wnam);
					xtip->xti_wnam = 0;
				}

				xtip->xti_flags &=
					~(XTI_FLOW|XTI_MORENDATA|XTI_MOREEXPDATA);
			}
			flushq(q, FLUSHALL);
		}

		if (*mp->b_rptr & FLUSHR) {
			if (xtip->xti_rdata) {
				m_freem(xtip->xti_rdata);
				xtip->xti_rdata = 0;
				if (xtip->xti_rnam) {
					(void) m_free(xtip->xti_rnam);
					xtip->xti_rnam = 0;
				}
				xtip->xti_rflags = 0;
			}
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
		} else
			freemsg(mp);

		return;

	default:
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xtiwput: Non understood STR msg type %d received mp = %x\n",
			mp->b_datap->db_type, mp);
		XTITRACE(XTIF_WPUT, printf(" xtiso: xtiwput() - M_? (%d)\n",
			(int)mp->b_datap->db_type););
		freemsg(mp);
		return;
	}
}

/*
 * xti_prec_insq - 	insert a message to the write-side queue based on the
 *		the precedence orders defined in the TPI Specification
 */
xti_prec_insq(q, mp)
	register queue_t *q;
	register mblk_t  *mp;
{
	register union  T_primitives *pp, *pp1;
	register mblk_t *mp1, *mp2;
	register struct xticb *xtip;

	CHECKPOINT("xti_prec_insq");

	XTITRACE(XTIF_MISC,
	    printf(" xtiso: xti_prec_insq() - entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
			q, OTHERQ(q), mp););

	xtip = (struct xticb *)q->q_ptr;
	pp   = (union T_primitives *)mp->b_rptr;

	if ((mp->b_datap->db_type != M_PROTO) &&
	    (mp->b_datap->db_type != M_PCPROTO) ||
	    (pp->type != T_EXDATA_REQ) &&
 	    (pp->type != T_DISCON_REQ)) {
		putq(q, mp);
		return;
	}
	for (mp1 = q->q_first; mp1 != NULL; mp1 = mp2) {
	    long type;
	    pp1 = (union T_primitives *)mp1->b_rptr;
	    mp2 = mp1->b_next;
	    type = (mp1->b_datap->db_type == M_DATA ? T_DATA_REQ : pp1->type);
	    switch (type) {
		case T_DATA_REQ:
			if (pp->type == T_EXDATA_REQ) {
				insq(q, mp1, mp);
				return;
			}
			/* FALL THROUGH */
                case T_EXDATA_REQ:
		case T_CONN_RES:
			if (pp->type == T_DISCON_REQ) {
				rmvq(q, mp1);
				freemsg(mp1);
			}
			break;
		case T_CONN_REQ:
			if (pp->type == T_DISCON_REQ) {
				/*
				 * Remove previously sent connection
				 * request and the current discon request.
				 */
				rmvq(q, mp1);
				freemsg(mp1);
				XTITRACE(XTIF_CONNECT,
				    printf(" xtiso: xti_prec_insq() - xti_snd_ok_ack(q=%x mp=%x)\n",
					q, mp););
				if (! xti_snd_ok_ack(q, mp))
					xtip->xti_pendcall = xti_snd_ok_ack;
				return;
		 	}
			break;
		default:
			strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			       "xti_prec_insq: bad TPI message type %d\n", 
			       pp->type);
			xti_cleanup(q, mp, EPROTO);
			return;
	    }
	}
	if (pp->type == T_DISCON_REQ) {
	    if (xtip->xti_wexpdata != NULL) {
		freemsg(xtip->xti_wexpdata);
		xtip->xti_wexpdata = NULL;
	    }
	    if (xtip->xti_wndata != NULL) {
		freemsg(xtip->xti_wndata);
		xtip->xti_wndata = NULL;
	    }
	    xtip->xti_flags &= ~(XTI_FLOW|XTI_MORENDATA|XTI_MOREEXPDATA);
	}
	putq(q, mp);
}

/*
 * ================================
 * XTI Write-side service routines.
 * ================================
 */

/*
 * xtiwsrv - xti write-side service procedure
 */
xtiwsrv(q)
	register queue_t *q;
{
	register mblk_t *mp;
	register struct xticb *xtip;
	int id, error;

	CHECKPOINT("xtiwsrv");

	XTITRACE(XTIF_WSRV,
		printf(" xtiso: xtiwsrv() - entry...q=%x, OTHERQ(q)=%x\n",
			q, OTHERQ(q)););

	xtip = (struct xticb *)q->q_ptr;

	if (id = xtip->xti_bufcallid)
		xti_unbufcall(q);	

	if (xtip->xti_pendcall) {
		if( (*xtip->xti_pendcall)(q, (mblk_t *)0)) {
			xtip->xti_pendcall = 0;
			xtip->xti_errtype = xtip->xti_tlierr = 0;
			xtip->xti_unixerr = 0;
		} else
			return;
	}

	for (;;) {
		/*
		 * The immediate next send is built in wdata,
		 * and marked complete with the MORE flag.
		 * If there, send it in case awakened via socket
		 * upcall, then go look at next message. If canput
		 * fails, XTI_FLOW will be set TRUE and sends remain
		 * queued, if send conversion fails, XTI_OUTBUF will
		 * be set true and bufcall will awaken us.
		 */
		/* First send expedited data if any. Then normal data. */
		/*
		 * Note for expedited data, we MUST check the MORE bit 
		 * since underlying OSI transport cannot handle 
		 * expedited data in multiple pieces.
		 */
		if (xtip->xti_wexpdata &&
		    !(xtip->xti_flags & XTI_MOREEXPDATA)) {
			XTITRACE(XTIF_WSRV,
				 printf(" xtiso: xtiwsrv() sending exdata (len=%d)\n", xtip->xti_etsdu););
			/*
			 * Let the transport decide if it can accept the 
			 * expedited data.
			 */
			if ((error = xti_send(xtip, T_EXDATA_REQ)) != 0) {
			    if (error == EWOULDBLOCK)
			    {
				XTITRACE(XTIF_SEND_FLOW,
					 printf(" xtiso: xtiwsrv() - setting FLOW CONTROL\n"););
				xtip->xti_flags |= XTI_FLOW;
			    }
			    break;
			}
		}
		/* Now send normal data if any is pending. */
		/* 
		 * TBD - instead of checking the MORE flag, maybe for normal
		 *       data we should send the partial chunk as is ?
		 */
		if (xtip->xti_wndata &&
		    !(xtip->xti_flags & XTI_MORENDATA)) {
			XTITRACE(XTIF_WSRV,
			    printf(" xtiso: xtiwsrv() sending data\n"););
			if ((xti_canput(xtip,
					msgdsize(xtip->xti_wndata)) < 0) ||
			    (xti_send(xtip, T_DATA_REQ) != 0)) {
				break;
			}
		}

		/*
		 * Get more data from stream. Somewhat suboptimal if currently
		 * flow controlled and next message is data send, but...
		 */
		mp = getq(q);
		XTITRACE(XTIF_WSRV,
		   printf(" xtiso: xtiwsrv() - getq() returned: mp=%x\n", mp););
		if (mp == 0)
			break;

		if (xtip->xti_flags & XTI_FATAL) {
			freemsg(mp);
			continue;
		}

		/*
		 * Try to move the message downstream...
		 */
		xti_output(q, mp);

		/*
		 * . If buffer shortage occurs, "xti_bufcall" has been issued
		 *   to callback sometime soon.
		 * . If flow-controlled, then get out of service routine
		 *   and wait to get rescheduled...
		 */
		if (((xtip->xti_flags & XTI_FLOW)) || xtip->xti_bufcallid) {
			XTITRACE(XTIF_WSRV|XTIF_SEND_FLOW,
			    printf(" xtiso: xtiwsrv() - early return flow=%d\n",
				(xtip->xti_flags & XTI_FLOW) != 0););
			break;
		}
		if (id && !xtip->xti_bufcallid) {
			id = 0;
			qenable(OTHERQ(q));
		}
		if ((XTI_ISCLOSING & xtip->xti_flags) && !(q->q_first) &&
			!xtip->xti_bufcallid)
			wakeup(xtip);
	}
}


/*
 * xti_output - process TPI messages with proper calls
 */
xti_output(q, mp)
	queue_t  *q;
	register mblk_t *mp;
{
	register union  T_primitives *pp;
	register struct xticb *xtip;

	CHECKPOINT("xti_output");

	XTITRACE(XTIF_OUTPUT,
	    printf(" xtiso: xti_output() entry...q=%x, OTHERQ(q)=%x, mp=%x\n",
		q, OTHERQ(q), mp););

	xtip = (struct xticb *)q->q_ptr;
	pp   = (union T_primitives *)mp->b_rptr;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (xtip->xti_flags & XTI_FLOW)
			putbq(q, mp);
		else
			xti_data_req(q, mp);
		break;

	case M_PROTO:
	case M_PCPROTO:
		switch (pp->type) {
		case T_UNITDATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_unitdata_req(q, mp);
			break;

		case T_ORDREL_REQ:
			xti_ordrel_req(q, mp);
			break;

		case T_DATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_data_req(q, mp);
			break;

		case T_EXDATA_REQ:
			if (xtip->xti_flags & XTI_FLOW)
				putbq(q, mp);
			else
				xti_exdata_req(q, mp);
			break;

		case T_CONN_REQ:
			xti_conn_req(q, mp);
			break;

		case T_CONN_RES:
			xti_conn_res(q, mp);
			break;

		case T_DISCON_REQ:
			xti_discon_req(q, mp);
			break;

		case T_INFO_REQ:
			xti_info_req(q, mp);
			break;

		case T_BIND_REQ:
			xti_bind_req(q, mp);
			break;

		case T_OPTMGMT_REQ:
			xti_optmgmt_req(q, mp);
			break;

		case T_UNBIND_REQ:
			xti_unbind_req(q, mp);
			break;

#if	XTI_XPG4
		case T_GETADDR_REQ:
			xti_getaddr_req(q, mp);
			break;
#endif

		default:
			xti_panic("xti_output 1");
			break;
		}

		break;

	case M_IOCTL:

		XTITRACE(XTIF_OUTPUT,
			printf(" xtiso: xti_output() - M_IOCTL\n");
			DumpIOCBLK((IOCP)mp->b_rptr););

		/* Huh? */
		switch (((IOCP)mp->b_rptr)->ioc_cmd) {
		default:
			xti_panic("xti_output 2");
			break;
		}
		break;

	default:
		xti_panic("xti_output 3");
		break;
	}
}


/*
 * ===============================
 * XTI Read-side service routines.
 * ===============================
 */

/*
 * xtirsrv - xti read-side service procedure
 */
xtirsrv(q)
	register queue_t *q;
{
	register struct xticb *xtip;
	int id = 0;

	CHECKPOINT("xtirsrv");

	xtip = (struct xticb *)q->q_ptr;
	
	if (id = xtip->xti_bufcallid)
		xti_unbufcall(q);

	for (;;) {
		if (xtip->xti_flags & XTI_FATAL)
			break;
		if (!canput(q->q_next))
			break;
		if (!xti_input(xtip))
			break;
		if (xtip->xti_bufcallid)
			break;
		if (id != 0 && !xtip->xti_bufcallid) {
			id = 0;
			qenable(OTHERQ(q));
		}
	}
}

/*
 *  xti_input - input data from the socket and convert them to TPI msgs
 *
 *  return values:
 *	 non-0 		- receives data/information from the socket.
 *	 0		- receives nothing from the socket.
 */
xti_input(xtip)
	register struct xticb *xtip;
{
	int indication, status = 1;

	CHECKPOINT("xti_input");

	if (xtip->xti_pendind) {
		indication = xtip->xti_pendind;
		xtip->xti_pendind = 0;
	} else
		indication = xti_new_tp_ind(xtip);

	switch (indication) {
	case T_CONN_CON:
		xti_conn_con (xtip);
		break;

	case T_CONN_IND:
		xti_conn_ind (xtip);
		break;

	case T_DISCON_IND:
		if (xtip->xti_flags & XTI_DISCONMAIN)
			while (xtip->xti_cindno > 0)
				xti_discon_ind (xtip);
		xti_discon_ind (xtip);
		break;

	case T_DATA_IND:
	case T_EXDATA_IND:
		xti_data_ind (indication, xtip);
		break;

	case T_ORDREL_IND:
		xti_ordrel_ind (xtip);
		break;

	case T_UNITDATA_IND:
		xti_unitdata_ind (xtip);
		break;

	case T_ERROR_ACK:
		if (xti_snd_error_ack(xtip->xti_rq, (mblk_t *)0, (int)0, (int)0)) {
			xtip->xti_tlierr    = 0;
			xtip->xti_unixerr   = 0;
			xtip->xti_errtype   = 0;
		}
		break;
	
	default:
		XTITRACE(XTIF_INPUT,
		  printf(" xtiso: xti_input() - indication=%d\n", indication););
	case 0:
		status = 0;
		break;
	}

	/* Buffer alloc failed - bufcall issued */
	if (xtip->xti_bufcallid)
		status = 0;

	return (status);
}


/*
 * xti_new_tp_ind - return indication from socket layer.
 */
xti_new_tp_ind(xtip)
	register struct xticb *xtip;
{
	register struct  socket *so;
	register struct  xtiseq *seq;
		 int     state;
		 int     lastState;
		 int 	 indication = 0;
		 int	 error = 0;
	/*
	 * The following bit mask indicates that a socket is connected.
	 */
#	define connected (SE_CONNOUT | SE_SENDCONN | SE_RECVCONN)

	CHECKPOINT("xti_new_tp_ind");

 	so = xtip->xti_so;
	if (!so)
		return 0;
	/*
	 * Save asynch socket state bits
	 */
	(void) sbpoll(so, &(so->so_rcv));
	lastState = xtip->xti_sostate;
	xtip->xti_sostate = 0;			/* Clear for new bits */

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_new_tp_ind() - lastState=%x\n", lastState););

	/*
	 * Update socket state bits and synch socket resources.
	 * Need to fold data bits into state bits.
	 */
	state  = sbpoll(so, &(so->so_rcv));
	state |= xtip->xti_sostate;		/* After sbpoll */

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_new_tp_ind() - xti_sostate=%x, servtype=%d, xti_state=%d\n", state, xtip->xti_proto.xp_servtype, xtip->xti_state););

	switch (xtip->xti_proto.xp_servtype) {
	case T_CLTS:

		/*
		 * look for any error that may have triggered this indication
		 */
		if (state & SE_ERROR) {
			xtip->xti_unixerr = xtip->xti_so->so_error;
#ifdef XTIDEBUG
			printf("Warning: Error from transport provider [%d]\n",
					xtip->xti_unixerr);
#endif
			xti_cleanup(xtip->xti_rq, (mblk_t *)0, 0);

			return(0);
		}
		/*
		 * look for a unitdata indication
		 */
		if (xtip->xti_state == TS_IDLE) {
			if (state & SE_HAVEDATA) {
				XTITRACE(XTIF_EVENTS,  printf(" xtiso: xti_new_tp_ind() - SE_HAVEDATA, T_UNITDATA_IND\n"););
				return(T_UNITDATA_IND);
			}
		}
		break;

	case T_COTS_ORD:
	case T_COTS:

		/*
		 * look for a discon/ordrel indication
		 */
		switch (xtip->xti_state) {
		case TS_WCON_CREQ:
                        /*
                         * If in this state,
                         * either a completely connected state (SE_CONNOUT)
                         * or the transient, SO_ISCONNECTING state
                         * is valid before we consider any pessimistic action
                         * This is a temporary kludge, because checking the
                         * so_state here defeats the purpose of sbpoll().
                         */

                        if (!(state & (SE_CONNOUT )) &&
                            !(xtip->xti_so->so_state & SS_ISCONNECTING)) {
				indication = T_DISCON_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=TS_WCON_CREQ, T_DISCON_IND\n", xtip););
			}
                	break;

		case TS_WREQ_ORDREL:
			if ((state & connected) != connected) {
				if ((state & SE_ERROR) &&
				    (xtip->xti_soerror == ECONNRESET)) {
					indication = T_DISCON_IND;
					XTITRACE(XTIF_EVENTS,
						printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=TS_WREQ_ORDREL, T_DISCON_IND\n", xtip););
				}
			}
                	break;

		case TS_DATA_XFER:	/* When data consumed */
                        /*
                         * If T_COTS it's not an orderly release protocol so
                         * send up the disconnect indication even if there is
                         * more data to be received.
                         */
                        if (xtip->xti_proto.xp_servtype == T_COTS ||
                                !((state & (SE_HAVEDATA|SE_HAVEOOB)) ||
                                	xtip->xti_rdata)) {
                                if (!(state & connected)) {
                                        indication = T_DISCON_IND;
                                } else if (((state & connected) ==
                                                (SE_SENDCONN|SE_CONNOUT)) &&
                                             (xtip->xti_proto.xp_servtype ==
                                                T_COTS_ORD)) {

                                        indication = T_ORDREL_IND;
                                }
                                XTITRACE(XTIF_EVENTS,
                                        if (indication)
					printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=TS_DATA_XFER, %s\n",
						xtip, (indication == T_DISCON_IND ? "T_DISCON_IND" : "T_ORDREL_IND")););
			}
			break;

		case TS_WIND_ORDREL:
			if (!((state & (SE_HAVEDATA|SE_HAVEOOB)) ||
				xtip->xti_rdata) &&
			    (state & connected) != connected) {
				indication = T_ORDREL_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=TS_WIND_ORDREL, T_ORDREL_IND\n", xtip););
			}
                	break;

		case TS_WRES_CIND:
			if (((state & connected) != connected) &&
			    ((lastState & connected) == connected)) {
				/*
				 * The main connection for the Server
			 	 * is being disconnected.  We need
			 	 * to disconnect all of the pending
			 	 * connection indications first.  This
			 	 * is why we set the flag here to ensure
			 	 * proper indications will be passed up
				 * later from xtirsrv().
			 	 */
				xtip->xti_flags |= XTI_DISCONMAIN;
				indication = T_DISCON_IND;
				XTITRACE(XTIF_EVENTS,
					printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=TS_WRES_CIND, T_DISCON_IND\n", xtip););
			}
			/* 
			 * Check for lost unaccepted connections.
			 */ 
			seq = xtip->xti_seq;
			do {
			    if (seq->seq_used == XTIS_AWAITING && seq->seq_so) {
				int unaccState =
				    sbpoll(seq->seq_so, &seq->seq_so->so_rcv);
				if ((unaccState & connected) != connected) {
					(void) soclose(seq->seq_so);
					seq->seq_so = 0;
					seq->seq_used = XTIS_LOST;
					indication = T_DISCON_IND;
				}
			    }
			} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);
			break;

                case TS_IDLE:
                        if (state & SE_ERROR) {
                                xtip->xti_unixerr = xtip->xti_so->so_error;
                                xti_cleanup(xtip->xti_rq, (mblk_t *)0, 0);
                                return(0);
                        }
                        break;


		default:
			indication = 0;
			XTITRACE(XTIF_EVENTS,
				printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=%d indication=%d\n",
				xtip, xtip->xti_state, indication););
			break;

		} /* switch */

		/*
		 * Return indication if any.  Inidcations wipe out any errors.
		 */
		if (indication) {
                        xtip->xti_so->so_error = 0;
			return (indication);
		}

		/*
		 * look for a conn indication. Obey restriction that 
		 * at most xti_qlen slots are can be used.
		 */
		if ((state & SE_CONNIN) &&
		    ((xtip->xti_state == TS_IDLE) ||
		     (xtip->xti_state == TS_WRES_CIND))) {
			if (!xtip->xti_lso)
				xti_panic("xti_new_tp_ind not listening");
			seq = xtip->xti_seq;
			do {
				if (seq->seq_used != XTIS_AVAILABLE)
					continue;
				seq->seq_used = XTIS_AWAITING;
				error = sodequeue(xtip->xti_lso, &seq->seq_so,
					  (struct mbuf **)0, SOCKNAME_FORMAT);
				XTITRACE(XTIF_EVENTS, if (error)
					printf(" xtiso: xti_new_tp_ind(xtip=%x) - xti_state=%s, SE_CONNIN, sodequeue error=%d\n",
					xtip, xtip->xti_state == TS_IDLE ?
					    "TS_IDLE" : "TS_WRES_CIND",
					error););

				/*
				 * Tally an outstanding connect indication,
				 * even if it failed. Pass result later.
				 */
				xtip->xti_cindno++;
				return (T_CONN_IND);

			} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);
			/* We will be qenabled when a slot opens. */
		}

		/*
		 * look for a conn confirmation
		 */
		if ((state & SE_CONNOUT) && xtip->xti_state == TS_WCON_CREQ)
			return (T_CONN_CON);

		/*
		 * look for a data/exdata indication
		 *
		 * NOTE:
		 * This mecahnism is slightly hacky and doesn't work in
		 * the same manner for different Transport Protocols.
		 * For OSI, This would never be set, because SE_HAVEOOB is not
		 * set due to the fact that there is none of the so_oobmark,
		 * and SS_RCVATMARK used at the socket level.
		 * Look at the comment in xti_rcv() for how things will work
		 * for OSI and "similar" protocols.  -sandeep
		 *
		 */
		if ((xtip->xti_state == TS_DATA_XFER) ||
		    (xtip->xti_state == TS_WIND_ORDREL)) {
			if ((state & (SE_HAVEDATA|SE_HAVEOOB)) ||
					xtip->xti_rdata) {
				XTITRACE(XTIF_EVENTS, printf(" xtiso: xti_new_tp_ind(), T_DATA_IND\n"););
				/* mark so we know what to pass in soreceive()*/
				if (state&SE_HAVEOOB) {
					xtip->xti_rflags |= MSG_OOB;
				}
				return (T_DATA_IND);
			}
		}
		break;

	default:
		break;

	} /* switch */

	return (0);
#undef connected
}


/*
 * ==========================================
 * XTI service routines for TP user requests.
 * ==========================================
 *
 * q 	= Pointer to write queue
 * mp 	= Pointer to message block
 */

xti_info_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xtiproto *xp;
	register mblk_t *mpp;
	struct	 mbuf	*m = 0;

	CHECKPOINT("xti_info_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);

	XTITRACE(XTIF_INFO,
		printf(" xtiso: xti_info_req() - dump of xti_proto...\n");
		DumpPROTO(xp););
	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_info_req)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_info_req: incorrect message size\n");
		return;
	}
		
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < 
	     sizeof(struct T_info_ack)) {
		if ((mpp = allocb(sizeof(struct T_info_ack), BPRI_HI)) == 
		    (mblk_t *)0) {
			xti_bufcall(q, sizeof(struct T_info_ack), BPRI_HI, 0);
			putbq(q, mp);
			return;
		}
		freemsg(mp);
		mp = mpp;
	} else
		mp->b_rptr = mp->b_datap->db_base;
	
	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr           = mp->b_rptr + sizeof(struct T_info_ack);
	pp                   = (union T_primitives *)mp->b_rptr;

	pp->info_ack.PRIM_type          = T_INFO_ACK;
	pp->info_ack.CURRENT_state      = xtip->xti_state;
	pp->info_ack.TSDU_size          = xp->xp_tsdulen;
	pp->info_ack.ADDR_size          = xp->xp_addrlen;
	pp->info_ack.OPT_size           = xp->xp_optlen;
	pp->info_ack.TIDU_size          = xp->xp_tidulen;
	pp->info_ack.SERV_type          = xp->xp_servtype;

	/*
	 * The info returned depends on the state of the transport.
	 */
	if (xtip->xti_proto.xp_options) {
	    int error;
	    struct topt_xtiinfo *tp;
	    if (error = sogetopt(xtip->xti_so, xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_getinfo, &m))
	    {
#if XTIDEBUG
		printf("xti_info_req: sogetopt gave us an error (%d %d)\n",
		       xtip->xti_proto.xp_opt_level, error);
#endif
		if (m)
			m_freem(m);
		if (xti_snd_error_ack(q, mp, TSYSERR, error))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_info_req: sogetopt error\n");
		return;
	    }
	    if (m->m_len != sizeof(struct topt_xtiinfo))
	    {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_info_req: sogetopt len error\n");
		m_freem(m);
		return;
	    }
	    tp = mtod(m, struct topt_xtiinfo *);
	    pp->info_ack.ETSDU_size         = tp->etsdulen;
	    pp->info_ack.CDATA_size         = tp->connectlen;
	    pp->info_ack.DDATA_size         = tp->disconlen;
	    m_freem(m);

	}
	else
	{
	    pp->info_ack.ETSDU_size         = xp->xp_etsdulen;
	    pp->info_ack.CDATA_size         = xp->xp_connectlen;
	    pp->info_ack.DDATA_size         = xp->xp_disconlen;
	}

	qreply(q, mp);
}

xti_bind_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	struct	mbuf *nam = 0;
	int	newState;
	int	error = 0;

	CHECKPOINT("xti_bind_req");

	xtip = (struct xticb *)q->q_ptr;

	if (xtip->xti_wnam) {
		nam = xtip->xti_wnam;
		goto pendcall;
	}
	pp  = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_BIND_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0, 
			SL_TRACE|SL_ERROR,
			"xti_bind_req: would place interface out of state\n");
		return;
	}

	if (((mp->b_wptr - mp->b_rptr) < (sizeof(struct T_bind_req))) ||
	    ((mp->b_wptr - mp->b_rptr) < (pp->bind_req.ADDR_offset +
					 pp->bind_req.ADDR_length))) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_bind_req: incorrect message size\n");
		return;
	}
	if (pp->bind_req.ADDR_length != 0) {
		if (pp->bind_req.ADDR_offset < sizeof(struct T_bind_req)) {
			if (xti_snd_error_ack(q, mp, TBADADDR, 0))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				SL_TRACE|SL_ERROR,
				"xti_bind_req: incorrect address size\n");
			return;
		}
		if (pp->bind_req.ADDR_length <= 0 ||
	    	   (xtip->xti_proto.xp_addrlen == XTI_NS) ||
	    	   (xtip->xti_proto.xp_addrlen >= 0 &&
	     	    pp->bind_req.ADDR_length > xtip->xti_proto.xp_addrlen)) {
			if (xti_snd_error_ack(q, mp, TBADADDR, 0))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				SL_TRACE|SL_ERROR,
				"xti_bind_req: invalid address size\n");
			return;
		}

		XTITRACE(XTIF_BINDING,
			printf(" xtiso: pp->bind_req.ADDR_length=%d\n",
			pp->bind_req.ADDR_length););

		error = xti_copy_to_mbuf(mp->b_rptr + pp->bind_req.ADDR_offset,
	             pp->bind_req.ADDR_length, MT_SONAME, &nam);
		switch (error) {
		case ENOBUFS:
			xti_bufcall(q, pp->bind_req.ADDR_length, BPRI_HI,
			    XTI_TIMEOUT);
			putbq(q, mp);
			return;
		case EINVAL:
			if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
				strlog(XTI_INFO_ID, xtip->xti_minor,0,
				SL_TRACE|SL_ERROR,
				"xti_bind_req: xti_copy_to_mbuf failed\n");
			return;
		}
	}

	XTITRACE(XTIF_BINDING,
		printf(" xtiso: sobind(%x, %x)\n", xtip->xti_so, nam);
		DumpSO(xtip->xti_so););

	error = sobind(xtip->xti_so, nam);

	XTITRACE(XTIF_BINDING,
		if (error) printf(" xtiso: sobind error=%d\n", error););

	if (nam) {
		(void) m_free(nam);
		nam = 0;
	}

	switch (error) {
	case 0:
		/*
		 * "getsockname"
		 */
		if (error = sogetaddr(xtip->xti_so, &nam, 0, SOCKNAME_FORMAT)){
			if (nam)
				(void)m_free(nam);
			xti_finished(xtip, XTI_NOSOCK);
			xti_snd_error_ack(q, mp, TBADADDR, 0);
			return;
		}
		xtip->xti_state = newState;
		xtip->xti_so->so_state &= ~SS_PRIV;
		break;

	case EADDRNOTAVAIL:
		xti_snd_error_ack(q, mp, TBADADDR, 0);
		return;
	case EACCES:
		xti_snd_error_ack(q, mp, TACCES, 0);
		return;
	case EADDRINUSE:
		xti_snd_error_ack(q, mp, TADDRBUSY, 0);
		return;
	default:
	        if (pp->bind_req.ADDR_length == 0)
			xti_snd_error_ack(q, mp, TNOADDR, 0);
		else
			xti_snd_error_ack(q, mp, TSYSERR, error);
		return;
	}

	xtip->xti_qlen  = (pp->bind_req.CONIND_number > xtip->xti_maxqlen) ?
			 xtip->xti_maxqlen : pp->bind_req.CONIND_number;
pendcall:
	if (!mp || (mp->b_datap->db_lim - mp->b_datap->db_base) <
	     sizeof(struct T_bind_ack) + nam->m_len) {
		mblk_t *mpp;
		mpp = allocb(sizeof(struct T_bind_ack) + nam->m_len, BPRI_HI);
		if (mpp == (mblk_t *)0) {
			xti_bufcall(q, sizeof(struct T_bind_ack) + nam->m_len, BPRI_HI, 0);
			xtip->xti_wnam = nam;
			putbq(q, mp);
			return;
		}
		if (mp)
			freemsg(mp);
		mp = mpp;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	/*
	 * If qlen is greater than zero, enable listens on this socket
	 */
	if (xtip->xti_qlen > 0 && xti_listen_req(q, mp) != 0) {
		(void) m_free(nam);
		xtip->xti_wnam = 0;
		return;
	}

	mp->b_datap->db_type  = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_bind_ack) + nam->m_len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->bind_ack.PRIM_type     = T_BIND_ACK;
	pp->bind_ack.ADDR_length   = nam->m_len;
	pp->bind_ack.ADDR_offset   = sizeof (struct T_bind_ack);
	pp->bind_ack.CONIND_number = xtip->xti_qlen;
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->bind_ack.ADDR_offset),
		(unsigned)nam->m_len);
	(void) m_free(nam);
	xtip->xti_wnam = 0;

	NEXTSTATE(xtip, TE_BIND_ACK, "xti_bind_req");
	qreply(q, mp);
}

#if	XTI_XPG4
xti_getaddr_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register mblk_t *mpp;
	register struct xtiproto *xp;
	struct   mbuf *peernam = 0;
	struct   mbuf *bndnam = 0;
	int	len;
	int	err;

	CHECKPOINT("xti_getaddr_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);

	XTITRACE(XTIF_INFO,
		printf(" xtiso: xti_getaddr_req() ...\n"););

	err = 0;
	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_getaddr_req)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_getaddr_req: incorrect message size\n");
		return;
	}

	/*
	 * getsockaddr
	 */
	if (sogetaddr(xtip->xti_so, &bndnam, 0, SOCKNAME_FORMAT)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_getaddr_req: socket not bound\n");
		err = 1;
		goto bad;
	}

	/*
	 * getpeeraddr
	 */
	if (sogetaddr(xtip->xti_so, &peernam, 1, SOCKNAME_FORMAT))
		peernam = 0;

	pp = (union T_primitives *)mp->b_rptr;
	if ((pp->getaddr_req.BNDADDR_maxlen < bndnam->m_len) ||
	     (pp->getaddr_req.PEERADDR_maxlen) < (peernam ? peernam->m_len:0)) {
		if (xti_snd_error_ack(q, mp, TBUFOVFLW, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_getaddr_req: address buffer overflow\n");
		err = 1;
		goto bad;
	}

	if ((mp->b_datap->db_lim - mp->b_datap->db_base) <
	     (len = sizeof(struct T_getaddr_ack) +
	     bndnam->m_len + (peernam ? peernam->m_len : 0))) {
		if ((mpp = allocb(len, BPRI_HI)) == (mblk_t *)0) {
			xti_bufcall(q, len, BPRI_HI, 0);
			putbq(q, mp);
			err = 1;
			goto bad;
		}
		freemsg(mp);
		mp = mpp;
	} else
		mp->b_rptr = mp->b_datap->db_base;

	mp->b_datap->db_type = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + len;
	pp = (union T_primitives *)mp->b_rptr;
	pp->getaddr_ack.PRIM_type = T_GETADDR_ACK;
	pp->getaddr_ack.BNDADDR_length = bndnam->m_len;
	pp->getaddr_ack.BNDADDR_offset = sizeof(struct T_getaddr_ack);
	bcopy(mtod(bndnam, caddr_t),
	     (caddr_t)(mp->b_rptr + pp->getaddr_ack.BNDADDR_offset),
	     pp->getaddr_ack.BNDADDR_length);

	if (peernam) {
		pp->getaddr_ack.PEERADDR_length = peernam->m_len;
		pp->getaddr_ack.PEERADDR_offset = sizeof(struct T_getaddr_ack)+
		     bndnam->m_len;
		bcopy(mtod(peernam, caddr_t),
		     (caddr_t)(mp->b_rptr + pp->getaddr_ack.PEERADDR_offset),
		     pp->getaddr_ack.PEERADDR_length);
	} else {
		pp->getaddr_ack.PEERADDR_length = 0;
		pp->getaddr_ack.PEERADDR_offset = 0;
	}
bad:
	if (bndnam)
		m_free(bndnam);
	if (peernam)
		m_free(peernam);
	if (err == 0)
		qreply(q, mp);
}
#endif


xti_unbind_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	int newState;
	register struct xtiseq *seq;
	struct socket *so_s, *lso_s;

	CHECKPOINT("xti_unbind_req");

	xtip = (struct xticb *)q->q_ptr;
	if (!mp)
		goto pendcall;

	if ((newState = TNEXTSTATE(xtip, TE_UNBIND_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_unbind_req: would place interface out of state\n");
		return;
	}
	xtip->xti_state = newState;

	if (! xti_snd_flushrw(q)) {
		xti_snd_error_ack(q, mp, TSYSERR, ENOMEM);
		return;
	}

	/* fake the close followed by a new open */
	xti_finished(xtip, XTI_NEWSOCK);
        xtip->xti_state = newState;

pendcall:
	XTITRACE(XTIF_BINDING,
	    printf(" xtiso: xti_unbind_req() - xti_snd_ok_ack(q=%x mp=%x)\n",
		q, mp););

	if (! xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_unbind_req;
		return 0;
	}
	NEXTSTATE(xtip, TE_OK_ACK1, "xti_unbind_req");
}


xti_optmgmt_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct mbuf *m = 0;
	int	 newState;
	int      optlen, flags, error;
#if	XTI_XPG4
	int	error;
	struct	t_opthdr	*opt;
	char	*stopt;
#endif

	CHECKPOINT("xti_optmgmt_req");

	pp  = (union T_primitives *)mp->b_rptr;
	xtip = (struct xticb *)q->q_ptr;

	if ((newState = TNEXTSTATE(xtip, TE_OPTMGMT_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: would place interface out of state\n");
		return;
	}

	if (((mp->b_wptr - mp->b_rptr) < (sizeof(struct T_optmgmt_req))) ||
	    ((mp->b_wptr - mp->b_rptr) < (pp->optmgmt_req.OPT_offset +
					 pp->optmgmt_req.OPT_length))) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: incorrect message size\n");
		return;
	}
	if (xtip->xti_proto.xp_optlen == XTI_NS) {
		if (xti_snd_error_ack(q, mp, TNOTSUPPORT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: options not supported\n");
		return;
	}

	if ((xtip->xti_proto.xp_optlen >= 0 &&
	     pp->optmgmt_req.OPT_length > xtip->xti_proto.xp_optlen) ||
	     pp->optmgmt_req.OPT_offset < sizeof(struct T_optmgmt_req)) {
		if (xti_snd_error_ack(q, mp, TBADOPT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			       SL_TRACE|SL_ERROR,
			       "xti_optmgmt_req: invalid options size\n");
		return;
	}
	/*
	 * The following rule is applied according to the TPI spec.
	 */
	if ((flags = pp->optmgmt_req.MGMT_flags) == T_NEGOTIATE &&
	     pp->optmgmt_req.OPT_length == 0)
		flags = T_DEFAULT;
	optlen = xtip->xti_proto.xp_optlen > 0 ? xtip->xti_proto.xp_optlen : 0;
	/*
	 * if optlen == 0 do we support no options ??
	 */

	if ((flags != T_DEFAULT) &&
	    (optlen == 0 || optlen > pp->optmgmt_req.OPT_length))
		optlen = pp->optmgmt_req.OPT_length;

	/*
	 * Copy the requested options to be  T_CHECKed or to be  T_NEGOTIATEd
	 * (before the buffer would get trashed below!)
	 */
	 if ((xtip->xti_proto.xp_options) &&
		 (flags == T_CHECK || flags == T_NEGOTIATE)) {
		 if (!(m = xti_get_mbuf(pp->optmgmt_req.OPT_length)))
		 {
			 xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
			 return;
		 }
		 bcopy((caddr_t)pp + pp->optmgmt_req.OPT_offset,
		       mtod(m, caddr_t), 
		       pp->optmgmt_req.OPT_length);

		 m->m_len = pp->optmgmt_req.OPT_length;
	 }
	/*
	 * Use the same (req) mblk for ack if there is sufficient 
	 *	space and retarget pp
	 */
	if ((mp->b_datap->db_lim - mp->b_datap->db_base) < 
	     (sizeof(struct T_optmgmt_ack) + optlen)) { 
		mblk_t *mpp;
		if ((mpp = allocb(sizeof(struct T_optmgmt_ack) + optlen,
		    BPRI_HI)) == 0) {
			xti_bufcall(q, sizeof(struct T_optmgmt_ack) + optlen, BPRI_HI, 0);
			putbq(q, mp);
			if (m)
			    m_freem(m);
			return;
		}
		freemsg(mp);
		mp = mpp;
	} else
		mp->b_rptr = mp->b_datap->db_base;

        pp  = (union T_primitives *)mp->b_rptr;

#if	XTI_XPG4
	mp->b_datap->db_type = M_PCPROTO;

	stopt = (char *)mp->b_rptr + sizeof(struct T_optmgmt_req);
	mp->b_wptr = mp->b_rptr + sizeof(struct T_optmgmt_ack) + optlen;
	pp->optmgmt_ack.PRIM_type  = T_OPTMGMT_ACK;
	pp->optmgmt_ack.OPT_length = optlen;
	pp->optmgmt_ack.OPT_offset = sizeof(struct t_opthdr);

	for (opt = (struct t_opthdr *)stopt; opt != NULL;
		opt = OPT_NEXTHDR(stopt, optlen, opt)) {

		error = xti_chk_protocol(opt, xtip, optlen, flags);
		if (error < 0)
			break;
		if (error > 0)
			goto badopt;
	}
	pp->optmgmt_ack.MGMT_flags = T_SUCCESS;
#else
	switch (flags) {
	case T_DEFAULT:
		mp->b_datap->db_type = M_PCPROTO;
		mp->b_wptr = mp->b_rptr + sizeof(struct T_optmgmt_ack) + optlen;
		pp->optmgmt_ack.PRIM_type  = T_OPTMGMT_ACK;
		pp->optmgmt_ack.OPT_offset = sizeof(struct T_optmgmt_ack);
		pp->optmgmt_ack.MGMT_flags = flags;
		/*
		 * Default options should be assigned here
		 */
		if (xtip->xti_proto.xp_options) {
#if XTIDEBUG
		    XTITRACE(XTIF_OPTMGMT,
		    	printf ("xti_optmgmt_req: T_DEFAULT calling sogetopt:\n"););
#endif
		    if (error = sogetopt(xtip->xti_so, 
					 xtip->xti_proto.xp_opt_level,
					 xtip->xti_proto.xp_mgmtdef,
					 &m) ) {
#if XTIDEBUG
			printf("xti_optmgt_req:sogetopt(%d,mgmtdef=%d), err = %d\n",
				xtip->xti_proto.xp_opt_level, 
				xtip->xti_proto.xp_mgmtdef,
				error);
#endif
			if (m)
			    m_freem(m);

			if (error == EINVAL)
			    xti_snd_error_ack(q, mp, TBADOPT, 0);
			else
			    xti_snd_error_ack(q, mp, TSYSERR, error);
			return;
		    }
#if XTIDEBUG
		    if (!m)
			printf("xti_optmgt_req: sogetopt returned NULL m\n");
#endif
		    /*
		     * TP should not send option len > what is specified by its AF_ above****APK***
		     */
		    m->m_len = m->m_len > optlen ? optlen : m->m_len;

		    bcopy(mtod(m, caddr_t),
			  (caddr_t)pp + pp->optmgmt_ack.OPT_offset,
			  m->m_len);

		    pp->optmgmt_ack.OPT_length = m->m_len;
		    m_freem(m);
		}
		break;

	case T_CHECK:
		mp->b_datap->db_type = M_PCPROTO;
		pp->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		pp->optmgmt_ack.OPT_offset = sizeof(struct T_optmgmt_ack);
		pp->optmgmt_ack.OPT_length = 0;
		pp->optmgmt_ack.MGMT_flags = flags;
		/*
		 * TP Options should be validated or negotiated here
		 */
		if (xtip->xti_proto.xp_options) {
		    if (error = sosetopt(xtip->xti_so, 
					 xtip->xti_proto.xp_opt_level,
					 xtip->xti_proto.xp_mgmtchk,
					 m) ) {
#if XTIDEBUG
			printf("xti_optmgt_req: T_CHECK sosetopt gave us an error (%d %d)\n",
			       xtip->xti_proto.xp_opt_level, error);
#endif
			if (error == EFAULT) 
			{
				pp->optmgmt_ack.MGMT_flags |= T_FAILURE;
				break;
			} else if (error == EINVAL) {
				xti_snd_error_ack(q, mp, TBADOPT, 0);
				return;
			} else {
				xti_snd_error_ack(q, mp, TSYSERR, error);
				return;
			}
		    }
		}
		pp->optmgmt_ack.MGMT_flags |= T_SUCCESS;
		break;
	case T_NEGOTIATE:
		mp->b_datap->db_type = M_PCPROTO;
		pp->optmgmt_ack.PRIM_type = T_OPTMGMT_ACK;
		pp->optmgmt_ack.OPT_offset = sizeof(struct T_optmgmt_ack);
		pp->optmgmt_ack.MGMT_flags = flags;
		/*
		 * TP Options should be validated or negotiated here
		 */
		if (xtip->xti_proto.xp_options) {
		    if (error = sosetopt(xtip->xti_so, 
					 xtip->xti_proto.xp_opt_level,
					 xtip->xti_proto.xp_mgmtneg,
					 m) ) {
#if XTIDEBUG
			printf("xti_optmgt_req: T_NEGOTIATE (1) sosetopt gave us an error (%d %d)\n",
			       xtip->xti_proto.xp_opt_level, error);
#endif
			if (error == EINVAL)
			    xti_snd_error_ack(q, mp, TBADOPT, 0);
			else
			    xti_snd_error_ack(q, mp, TSYSERR, error);
			return;
		    }

		    m = 0;	/* to make sure if sogetopt returned anything */
		    if (error = sogetopt(xtip->xti_so, 
					 xtip->xti_proto.xp_opt_level,
					 xtip->xti_proto.xp_mgmtneg,
					 &m) ) {
#if XTIDEBUG
			printf("xti_optmgt_req: T_NEGOTIATE (2) sogetopt gave us an error (%d %d)\n",
			       xtip->xti_proto.xp_opt_level, error);
#endif
			if (m)
			    m_freem(m);

			if (error == EINVAL)
			    xti_snd_error_ack(q, mp, TBADOPT, 0);
			else
			    xti_snd_error_ack(q, mp, TSYSERR, error);
			return;
		    }
		    /*
		     * TP should not send option len > what is specified by its AF_ above
		     */
		    m->m_len = m->m_len > optlen ? optlen : m->m_len;

		    bcopy(mtod(m, caddr_t), 
			  (caddr_t)pp + pp->optmgmt_ack.OPT_offset, m->m_len);

		    pp->optmgmt_ack.OPT_length = m->m_len;
		    m_freem(m);
		}
		break;
	default:
		if (xti_snd_error_ack(q, mp, TBADFLAG, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0, 
			SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: invalid flags\n");
		return;
	}
#endif
	xtip->xti_state = newState;
	NEXTSTATE(xtip, TE_OPTMGMT_ACK, "xti_optmgmt_req");
	qreply(q, mp);
#if	XTI_XPG4
	return;

badopt:
	if (xti_snd_error_ack(q, mp, TBADOPT, 0))
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_optmgmt_req: invalid flags\n");
#endif
}


xti_listen_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{

	register struct xticb *xtip;
	int error = 0;

	CHECKPOINT("xti_listen_req");

	xtip = (struct xticb *)q->q_ptr;
	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS))
		/*
		 * Not a connection-oriented TP so ignore listen request
		 */
		return(0);

	if (xtip->xti_state != TS_WACK_BREQ) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_listen_req: would place interface out of state\n");
		return (TOUTSTATE);
	}

	if (xtip->xti_qlen <= 0) {
		if (xti_snd_error_ack(q, mp, TBADQLEN, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_listen_req: qlen must be greater than zero\n");
		return(TBADQLEN);
	}

	XTITRACE(XTIF_BINDING,
		printf(" xtiso: xti_listen_req() - solisten(so=%x qlen=%d)\n",
			xtip->xti_so, xtip->xti_qlen););

	/* Use the xti qlen, solisten will fix if too big. */
	if (error = solisten(xtip->xti_so, xtip->xti_qlen)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, error))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_listen_req: solisten error status %d\n", error);
		return(error);
	}

	/*
	 * Preserve listening socket
	 */
	xtip->xti_lso = xtip->xti_so;
	return(0);
}


xti_conn_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct   mbuf *nam = 0;
	int size, dsize, error = 0;
	int newState, origState;
	struct mbuf *m;

	CHECKPOINT("xti_conn_req");

	xtip    = (struct xticb *)q->q_ptr;
	if (!mp)
		goto pendcall;
	pp     = (union T_primitives *)mp->b_rptr;

	/*
	 * XXX
	 * Cannot do conn_req if listening. Semantic mismatch between
	 * sockets and XTI, where a socket can only listen or connect, not
	 * both, In fact, if we call soconnect on a listener or solisten
	 * on a connector, we'll botch it badly!
	 */
	if (xtip->xti_lso != 0 ||
	    (newState = TNEXTSTATE(xtip, TE_CONN_REQ)) == TS_BAD_STATE) {
		if (xti_snd_error_ack(q, mp, TOUTSTATE, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: would place interface out of state\n");
		return;
	}
	origState = xtip->xti_state;
	xtip->xti_state = newState;

	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EOPNOTSUPP))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: primitive not supported by provider\n");
		return;
	}

	if ((size = mp->b_wptr - mp->b_rptr) < sizeof(struct T_conn_req) ||
	    size < (pp->conn_req.DEST_length + pp->conn_req.DEST_offset) ||
	    size < (pp->conn_req.OPT_length + pp->conn_req.OPT_offset)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: incorrect message size\n");
		return;
	}
	if (pp->conn_req.DEST_length < 0 ||
	    (pp->conn_req.DEST_length && xtip->xti_proto.xp_addrlen == XTI_NS) ||
	    (xtip->xti_proto.xp_addrlen >= 0 &&
	     pp->conn_req.DEST_length > xtip->xti_proto.xp_addrlen)) {
		if (xti_snd_error_ack(q, mp, TBADADDR, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid dest address\n");
		return;
	}
	if (pp->conn_req.OPT_length < 0 ||
	    (pp->conn_req.OPT_length && xtip->xti_proto.xp_optlen == XTI_NS) ||
	    (xtip->xti_proto.xp_optlen >= 0 &&
	     pp->conn_req.OPT_length > xtip->xti_proto.xp_optlen)) {
		if (xti_snd_error_ack(q, mp, TBADOPT, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,	
			SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid option size\n");
		return;
	}
	dsize = msgdsize(mp);
	/* cannot receive/accept ANY user data */
	if ((dsize && xtip->xti_proto.xp_connectlen == XTI_NS) ||
	    (xtip->xti_proto.xp_connectlen >= 0 &&
	     dsize > xtip->xti_proto.xp_connectlen)) {
		if (xti_snd_error_ack(q, mp, TBADDATA, 0))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: invalid connection data dsize\n");
		return;
	}

	/* 
	 * Send options if supported by transport and user has specified them.
	 */
	if ((xtip->xti_proto.xp_options) && pp->conn_req.OPT_length) {
	    if (!(m = xti_get_mbuf(pp->conn_req.OPT_length))) {
		xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
		return;
	    }

	    bcopy((caddr_t)pp + pp->conn_req.OPT_offset,
		  mtod(m, caddr_t), pp->conn_req.OPT_length);
	    m->m_len = pp->conn_req.OPT_length;

	    XTITRACE(XTIF_CONNECT,
		    printf("calling sosetopt(CONOPTS, %d, %d)\n",
			xtip->xti_proto.xp_opt_level,
			xtip->xti_proto.xp_conopts
			););
	    if (error = sosetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_conopts,
				 m) ) {
#if XTIDEBUG
	        XTITRACE(XTIF_CONNECT,
		  printf("xti_conn_req: sosetopt(%d,conopts=%d) error =%d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_conopts, 
			       error););
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		return;
	    }
	}

	/* 
	 * If connect data is supported, then send it.  Even then
	 * error may be returned depending upon negotiated options.
	 *
	 * for example, In OSI, Connect user data is valid for 
	 * TP4 and TP2 and not for TP0.  xp_connectlen
	 * is set to the max length assuming connect data is valid.
	 * If TP0 was negotiated for the connection, connect data is
	 * invalid and an error will be returned below.  Note that this
	 * must follow option negotation above so that the negotiated class
	 * is known by the transport.
	 */
	if (dsize && xtip->xti_proto.xp_options && xtip->xti_proto.xp_condata) {
	    struct mbuf *m;
	    if (!(m = xti_get_mbuf(dsize))) {
		xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
		return;
	    }

	    bcopy((caddr_t)mp->b_cont->b_rptr, mtod(m, caddr_t), dsize);
	    m->m_len = dsize;

	    XTITRACE(XTIF_CONNECT,
		    printf("calling sosetopt(CONDATA, %d, %d)\n",
			xtip->xti_proto.xp_opt_level,
			xtip->xti_proto.xp_condata
			););
	    if (error = sosetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_condata,
				 m) ) {
#if XTIDEBUG
		    XTITRACE(XTIF_CONNECT,
			printf("xti_conn_req: sosetopt(%d, condata=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_condata, 
			       error););
#endif
		xti_snd_error_ack(q, mp, TBADDATA, 0);
		return;
	    }
	}

	error = xti_copy_to_mbuf(mp->b_rptr + pp->conn_req.DEST_offset,
			     pp->conn_req.DEST_length, MT_SONAME, &nam);
	switch (error) {
	case ENOBUFS:
		xti_bufcall(q, pp->conn_req.DEST_length, BPRI_HI,
			XTI_TIMEOUT);
		putbq(q, mp);
		return;
	case EINVAL:
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_conn_req: xti_copy_to_mbuf failed\n");
		return;
	}

	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - about to soconnect(), so=%x, nam=%x\n",
			xtip->xti_so, nam););

	error = soconnect(xtip->xti_so, nam);

	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - soconnect error=%d. (%x)\n",
			error, error););

	(void) m_free(nam);
	if (error == 0)
		xtip->xti_so->so_state &= ~SS_PRIV;
	else {
		xtip->xti_state = origState;
		switch (error) {
		case EADDRNOTAVAIL:
			XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TBADADDR 0)\n", q, mp););
			xti_snd_error_ack(q, mp, TBADADDR, 0);
			return;

		case EACCES:
			XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TACCES 0)\n", q, mp););
			xti_snd_error_ack(q, mp, TACCES, 0);
			return;

		case EISCONN:
		case ENOTCONN:
		case ESHUTDOWN:
			XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TOUTSTATE 0)\n", q, mp););
			xti_snd_error_ack(q, mp, TOUTSTATE, 0);
			return;

		default:
			XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_conn_req() - xti_snd_error_ack(q=%x mp=%x TSYSERR error=%d)\n", q, mp, error););
			xti_snd_error_ack(q, mp, TSYSERR, error);
			return;
		}
	}

pendcall:
	XTITRACE(XTIF_CONNECT,
		printf(" xtiso: xti_conn_req() - xti_snd_ok_ack(q=%x mp=%x)\n",
			q, mp););
	if (! xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_conn_req;
		return 0;
	}
	NEXTSTATE(xtip, TE_OK_ACK1, "xti_conn_req");
}


xti_conn_res(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct socket *so;
	struct xtiseq *seq;
	struct mbuf *m;
	queue_t *connq;
	int size, dsize, error = 0;
	int newState;

	CHECKPOINT("xti_conn_res");

	pp     = (union T_primitives *)mp->b_rptr;
	xtip   = (struct xticb *)q->q_ptr;
	
	if ((newState = TNEXTSTATE(xtip, TE_CONN_RES)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,xtip->xti_state, SL_TRACE|SL_ERROR,
			"xti_conn_res: would place interface out of state\n");
		xti_snd_error_ack(q, mp, TOUTSTATE, 0);
		return;
	}

	if ((xtip->xti_proto.xp_servtype != T_COTS_ORD) &&
	    (xtip->xti_proto.xp_servtype != T_COTS)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_conn_res: primitive not supported by provider\n");
		xti_snd_error_ack(q, mp, TSYSERR, EOPNOTSUPP);
		return;
	}

	if ((size = mp->b_wptr - mp->b_rptr) < sizeof(struct T_conn_res) ||
	    size < (pp->conn_res.OPT_length + pp->conn_res.OPT_offset)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_conn_res: incorrect message size\n");
		xti_snd_error_ack(q, mp, TSYSERR, EINVAL);
		return;
	}
	if (pp->conn_res.OPT_length < 0 ||
	    (pp->conn_res.OPT_length && xtip->xti_proto.xp_optlen == XTI_NS) ||
	    (xtip->xti_proto.xp_optlen >= 0 &&
	     pp->conn_res.OPT_length > xtip->xti_proto.xp_optlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_conn_res: incorrect option size\n");
		xti_snd_error_ack(q, mp, TBADOPT, 0);
		return;
	}
	dsize = msgdsize(mp);
	/* cannot receive/accept ANY user data */
	if ((dsize && xtip->xti_proto.xp_connectlen == XTI_NS) ||
	    (xtip->xti_proto.xp_connectlen >= 0 &&
	     dsize > xtip->xti_proto.xp_connectlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_conn_res: invalid connect data size\n");
		xti_snd_error_ack(q, mp, TBADDATA, 0);
		return;
	}

	seq = xtip->xti_seq;
	do {
		if (seq->seq_used == XTIS_ACTIVE &&
		    pp->conn_res.SEQ_number == seq->seq_no)
			    break;
	} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);

	if (seq >= &xtip->xti_seq[xtip->xti_qlen]) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_conn_res: invalid sequence number\n");
		xti_snd_error_ack(q, mp, TBADSEQ, 0);
		return;
	}

#if	XTIDEBUG
	if (seq->seq_so == 0)
		xti_panic("xti_conn_res seq_so");
	if (xtip->xti_cindno <= 0)
		xti_panic("xti_conn_res cindno");
#endif
	xtip->xti_state = newState;

	connq = pp->conn_res.QUEUE_ptr;
	if (connq == RD(q)) {
		if (xtip->xti_cindno > 1) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: must respond to other rcvd conn indications first\n");
			xti_snd_error_ack(q, mp, TBADF, 0);
			return;
		}
		/*
		 * Connection response on same stream
		 * as connection request
		 *	. done with listening socket
		 *	. make socket carrying connect request the
		 *	  active socket for now
		 */
#if	XTIDEBUG
		if (xtip->xti_lso != xtip->xti_so)
			xti_panic("xti_conn_res: lso != so");
#endif

		(void) soclose(xtip->xti_lso);
		xtip->xti_lso = 0;
		xtip->xti_so  = seq->seq_so;
		NEXTSTATE(xtip, TE_OK_ACK2, "xti_conn_res 1");

		/*
		 * XXX
		 * Setup socket to make things work between sockets/xtiso
		 */
		xti_init_socket(xtip->xti_so, connq);
		so = xtip->xti_so;
	} else {
		register struct xticb *tmp = (struct xticb *)connq->q_ptr;
		/*
		 * Grab pointer to "other queue's" xti context block
		 */
		if (tmp == 0 || OTHERQ(connq)->q_qinfo->qi_putp != xtiwput) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: invalid resfd\n");
			xti_snd_error_ack(q, mp, TBADF, 0);
			return;
		}

		if (TNEXTSTATE(tmp, TE_PASS_CONN) == TS_BAD_STATE) {
			xti_snd_error_ack(q, mp, TOUTSTATE, 0);
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_conn_res: would place resfd out of state\n");
			return;
		}
		/*
		 * Connection response on new stream;
		 * ie. different stream from that on which
		 * connection request arrived
		 */
		(void) soclose (tmp->xti_so);
		tmp->xti_so = seq->seq_so;
		NEXTSTATE(tmp, TE_PASS_CONN, "xti_conn_res 2");

		if (xtip->xti_cindno == 1)
			NEXTSTATE(xtip, TE_OK_ACK3, "xti_conn_res 3");
		else
			NEXTSTATE(xtip, TE_OK_ACK4, "xti_conn_res 4");

		/*
		 * XXX
		 * Setup socket to make things work between sockets/xtiso
		 */
		xti_init_socket(tmp->xti_so, connq);
		so = tmp->xti_so;
	}
	/* 
	 * Replaced xti_maxqlen with xti_qlen since that is now the 
	 * max number of entries that can be used. - apn
	 */
	if (xtip->xti_cindno-- == xtip->xti_qlen)
		qenable(xtip->xti_rq);
	seq->seq_used  = XTIS_AVAILABLE;
	seq->seq_so    = 0;

	if (xtip->xti_proto.xp_options) {
	    if (pp->conn_res.OPT_length) {
		if (!(m = xti_get_mbuf(pp->conn_res.OPT_length))) {
		    xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
		    return;
		}

		bcopy((caddr_t)pp + pp->conn_res.OPT_offset,
		      mtod(m, caddr_t), pp->conn_res.OPT_length);
		m->m_len = pp->conn_res.OPT_length;

		if (error = sosetopt(so, 
				     xtip->xti_proto.xp_opt_level,
				     xtip->xti_proto.xp_conopts,
				     m) ) {
#if XTIDEBUG
		    printf("xti_conn_res: sosetopt(%d,conopts=%d) error = %d\n",
				   xtip->xti_proto.xp_opt_level, 
				   xtip->xti_proto.xp_conopts, 
				   error);
#endif
		    xti_snd_error_ack(q, mp, TSYSERR, error);
		    return;
		}
	    }

	    /* 
	     * If connect data is supported, then send it.  Even then
	     * error may be returned depending upon negotiated options.
	     *
	     * for example, In OSI, Connect user data is valid for 
	     * TP4 and TP2 and not for TP0.  xp_connectlen
	     * is set to the max length assuming connect data is valid.
	     * If TP0 was negotiated for the connection, connect data is
	     * invalid and an error will be returned below.  Note that this
	     * must follow option negotation above so that the negotiated class
	     * is known by the transport.
	     *
	     */
	    if (dsize && xtip->xti_proto.xp_condata) {
		struct mbuf *m;
		if (!(m = xti_get_mbuf(dsize)))
		{
		    xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
		    return;
		}

		bcopy((caddr_t)mp->b_cont->b_rptr, mtod(m, caddr_t), dsize);
		m->m_len = dsize;

		if (error = sosetopt(so, 
				     xtip->xti_proto.xp_opt_level,
				     xtip->xti_proto.xp_condata,
				     m) ) {
#if XTIDEBUG
		    printf("xti_conn_res: sosetopt(%d,condata=%d) error = %d\n",
				   xtip->xti_proto.xp_opt_level, 
				   xtip->xti_proto.xp_condata, 
				   error);
#endif
		    xti_snd_error_ack(q, mp, TBADDATA, 0);
		    return;
		}
	    }

	    if (error = sosetopt(so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_accept,
				 (struct mbuf *)0) ) {
#if XTIDEBUG
		printf("xti_conn_res: sosetopt(%d, accpt=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_accept, 
			       error);
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		return;
	    }
	}

	XTITRACE(XTIF_CONNECT,
	  printf(" xtiso: xti_conn_res() - xti_snd_ok_ack(q=%x mp=%x)\n", q, mp););
	if (! xti_snd_ok_ack(q, mp)) {
		xtip->xti_pendcall = xti_snd_ok_ack;
		return 0;
	}
}


xti_discon_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	struct   xtiseq *seq = 0;
	int      size, error = 0;
	int 	 savedState;
	int 	 serviceType;
	int	 newState;
	struct   mbuf *nam = 0;

	CHECKPOINT("xti_discon_req");

	xtip = (struct xticb *)q->q_ptr;

	pp = (union T_primitives *)mp->b_rptr;
	savedState = xtip->xti_state;
	serviceType = xtip->xti_proto.xp_servtype;
	if ((newState = TNEXTSTATE(xtip, TE_DISCON_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: would place interface out of state\n");
		xti_snd_error_ack(q, mp, TOUTSTATE, 0);
		return;
	}

	if ((serviceType != T_COTS_ORD) && (serviceType != T_COTS)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: primitive not supported by provider\n");
		xti_snd_error_ack(q, mp, TSYSERR, EOPNOTSUPP);
		return;
	}

	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_discon_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: incorrect message size\n");
		xti_snd_error_ack(q, mp, TSYSERR, EINVAL);
		return;
	}


	if (xtip->xti_cindno) {
		seq = xtip->xti_seq;
		do {
			if (seq->seq_used == XTIS_ACTIVE &&
			    pp->discon_req.SEQ_number == seq->seq_no)
				break;
		} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);

		if (seq >= &xtip->xti_seq[xtip->xti_qlen]) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
				"xti_discon_req: invalid sequence number\n");
			xti_snd_error_ack(q, mp, TBADSEQ, 0);
			return;
		}
#if 1
	}
#else	
	} else if (pp->discon_req.SEQ_number != -1) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: invalid sequence number: no conn ind\n");
		xti_snd_error_ack(q, mp, TBADSEQ, 0);
		return;
	}
#endif

	size = msgdsize(mp);
	if ((size && xtip->xti_proto.xp_disconlen == XTI_NS) ||
	    (xtip->xti_proto.xp_disconlen >= 0 &&
	     size > xtip->xti_proto.xp_disconlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_req: invalid discon data size\n");
		xti_snd_error_ack(q, mp, TBADDATA, 0);
		return;
	}

	/* 
	 * If disconnect data is supported, then send it.  Even then
	 * error may be returned depending upon negotiated options.
	 *
	 * for example, In OSI, Disconnect user data is valid for 
	 * TP4 and TP2 and not for TP0.  xp_disconlen
	 * is set to the max length assuming connect data is valid.
	 * If TP0 was negotiated for the connection, disconnect data is
	 * invalid and an error will be returned below.
	 *
	 */
	if (size && xtip->xti_proto.xp_options && xtip->xti_proto.xp_discondata)
	{
	    struct mbuf *m;
	    if (!(m = xti_get_mbuf(size)))
	    {
		xti_snd_error_ack(q, mp, TSYSERR, ENOBUFS);
		return;
	    }

	    bcopy((caddr_t)mp->b_cont->b_rptr, mtod(m, caddr_t), size);
	    m->m_len = size;

	    if (error = sosetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_discondata,
				 m) ) {
#if XTIDEBUG
		printf("xti_disconn_req: sosetopt(%d, discon=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_discondata, 
			       error);
#endif
		xti_snd_error_ack(q, mp, TBADDATA, 0);
		return;
	    }
	}

	xtip->xti_state = newState;

	/*
	 * "getsockname"
	 */
	if (error = sogetaddr(xtip->xti_so, &nam, 0, SOCKNAME_FORMAT))
		goto bad;

	if (pp->discon_req.SEQ_number != -1 &&
	    seq) {
		/*
		 * "reject" pending connect request
		 */
		(void) soabort (seq->seq_so);
		(void) soclose (seq->seq_so);
		seq->seq_so = 0;
		seq->seq_used = XTIS_AVAILABLE;
	} else {
		/*
		 * disconnect established connection
		 */
		error = sodisconnect(xtip->xti_so);
		if (error)
			goto bad;
	}

	if (xtip->xti_cindno == 0)
		NEXTSTATE(xtip, TE_OK_ACK1, "xti_discon_req 1");
	else {
		if (xtip->xti_cindno == 1)
			NEXTSTATE(xtip, TE_OK_ACK2, "xti_discon_req 2");
		else
			NEXTSTATE(xtip, TE_OK_ACK4, "xti_discon_req 3");
		if (xtip->xti_cindno-- == xtip->xti_qlen)
			qenable(xtip->xti_rq);
	}

	if (nam) {
		(void) m_free(nam);
		nam = 0;
	}

	if ((savedState == TS_DATA_XFER || savedState == TS_WIND_ORDREL)  &&
	    (serviceType == T_COTS_ORD  || serviceType == T_COTS)) {
		if (!xti_snd_flushrw(q)) {
			error = EPROTO;
			goto bad;
		}
	}

	XTITRACE(XTIF_CONNECT,
	 printf(" xtiso: xti_discon_req() - xti_snd_ok_ack(q=%x mp=%x)\n", q, mp););
	if (! xti_snd_ok_ack(q, mp))
		xtip->xti_pendcall = xti_snd_ok_ack;
	return;

bad:	if (nam)
		(void) m_free(nam);
	xti_snd_error_ack(q, mp, TSYSERR, error);
}


xti_data_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xtiproto *xp;
	mblk_t   *mp1;
	int      more;
	int	 newState;

	CHECKPOINT("xti_data_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);
	pp  = (union T_primitives *)mp->b_rptr;

	/* 
	 * TBD - is the second test below a bug ? If xti is blocked by 
	 *       the socket space limit, then there could be data 
	 *       sitting on xti_wndata. Is it ok for this to cause a 
	 *       EPROTO error ?
	 */
	if ((newState = TNEXTSTATE(xtip, TE_DATA_REQ)) == TS_BAD_STATE ||
	    (xtip->xti_wndata && !(xtip->xti_flags & XTI_MORENDATA))) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_data_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if (mp->b_datap->db_type != M_DATA) {
		if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_data_req)) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_data_req: incorrect message size\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	}
	if (xp->xp_tsdulen == XTI_NS) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_data_req: cannot send normal data\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	/*
	 * If this TE supports TSDU, then check the "MORE data"
	 * user flag and either link data block or pass it
	 * on, accordingly.
	 */
	xtip->xti_tsdu += msgdsize(mp);
	switch (xp->xp_tsdulen) {
	      case 0: /* streams-oriented transport */
		more = 0;
		break;
	      case -1: /* record oriented transport with unlimited tsdu */
		if (!pp->data_req.MORE_flag)
			xtip->xti_flags |= XTI_SETEOR;
		more = 0;
		break;
	      default: /* other -- buffer data in xtiso if T_MORE is set! */
		more = pp->data_req.MORE_flag;
		if (xp->xp_tsdulen > 0 && xtip->xti_tsdu > xp->xp_tsdulen) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_data_req: TSDU size exceeded\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
		break;
	}
	xtip->xti_state = newState;

	if (mp->b_datap->db_type != M_DATA) {
		mp1 = unlinkb(mp);
		freemsg(mp);
	} else
		mp1 = mp;

	if (xtip->xti_wndata)
		linkb(xtip->xti_wndata, mp1);
	else
		xtip->xti_wndata = mp1;

	/*
	 * If user specified more data to come for
	 * this tsdu, then act as follows:
	 *  	if tsdu_len is set to
	 * 		-0: means stream transport allow to send
	 *		-1: means record-oriented transport allow to send
	 *	   		(see XTI_SETEOR flag)
	 *		-anything-else: don't send it out yet, link
	 * 			it onto the end of any queued data blocks
	 * 			for this TE.
	 */
	if (!more) {
		xtip->xti_flags &= ~XTI_MORENDATA;
		if (xti_canput(xtip, xtip->xti_tsdu) >= 0) {
			if (xtip->xti_unixerr = xti_send(xtip, T_DATA_REQ)) {
				xti_cleanup(q, 0, xtip->xti_unixerr);
				return;
			}
		}
		xtip->xti_tsdu = 0;
	} else {
		xtip->xti_flags |= XTI_MORENDATA;
	}
}


xti_exdata_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xtiproto *xp;
	mblk_t   *mp1;
	int      more;
	int	 newState;

	CHECKPOINT("xti_exdata_req");

	xtip = (struct xticb *)q->q_ptr;
	xp  = &(xtip->xti_proto);
	pp  = (union T_primitives *)mp->b_rptr;

	/* TBD - following behaves as the old code - if there is 
	 *       already a complete packet on the xti q, generate an 
	 *       error. This seems wrong. Same bug as noted in xti_data_req
	 */
	if ((newState = TNEXTSTATE(xtip, TE_EXDATA_REQ)) == TS_BAD_STATE ||
	    (xtip->xti_wexpdata && !(xtip->xti_flags & XTI_MOREEXPDATA))) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
		       "xti_exdata_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	
	if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_exdata_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_exdata_req: incorrect message size\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	if (xp->xp_etsdulen == XTI_NS) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
			"xti_exdata_req: cannot send expedited data\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	/* Check tsdu as for xti_data_req */
	xtip->xti_etsdu += msgdsize(mp);
	if (xp->xp_etsdulen != 0) {
		more = pp->exdata_req.MORE_flag;
		if (xp->xp_etsdulen > 0 && xtip->xti_etsdu > xp->xp_etsdulen) {
			strlog(XTI_INFO_ID, xtip->xti_minor,0,SL_TRACE|SL_ERROR,
				"xti_exdata_req: ETSDU size exceeded\n");
			xti_cleanup(q, mp, EPROTO);
			return;
		}
	} else
		more = 0;

	xtip->xti_state = newState;

	if (xtip->xti_wexpdata)
	    linkb(xtip->xti_wexpdata, unlinkb(mp));
	else 
	    xtip->xti_wexpdata = unlinkb(mp);
	freeb(mp);

	if (!more) {
		xtip->xti_flags &= ~XTI_MOREEXPDATA;
		/*
		 * Let the transport decide if it can accept the expedited
		 * data.
		 */
		if (xtip->xti_unixerr = xti_send(xtip, T_EXDATA_REQ)) {
		    if (xtip->xti_unixerr != EWOULDBLOCK)
			xti_cleanup(q, 0, xtip->xti_unixerr);
		    else
		    {
			XTITRACE(XTIF_SEND_FLOW,
				 printf(" xtiso: xti_exdata_req() - setting FLOW CONTROL\n"););
			xtip->xti_flags |= XTI_FLOW;
		    }
		    return;
		} else {
		    xtip->xti_etsdu = 0;
		}
	} else {
		xtip->xti_flags |= XTI_MOREEXPDATA;
	}
}


xti_ordrel_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	int newState;

	CHECKPOINT("xti_ordrel_req");

	xtip = (struct xticb *)q->q_ptr;
	pp  = (union T_primitives *)mp->b_rptr;

	if ((newState = TNEXTSTATE(xtip, TE_ORDREL_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_req: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	if ((mp->b_wptr - mp->b_rptr) < sizeof(struct T_ordrel_req)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_req: incorrect message size\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}

	xtip->xti_state = newState;

	if (soshutdown(xtip->xti_so, 1))
		xti_cleanup(q, mp, EPROTO);
}


xti_unitdata_req(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register union T_primitives *pp;
	register struct xticb *xtip;
	register struct xtiproto *xp;
	register mblk_t *mp1;
	int size, error = 0;
	int newState;
	struct mbuf *nam = 0;

/* TBD - Temporary #define for location of udata. So we don't fill this
   function with unnecessary #ifdef's. To be removed once the bugfix is tested
   and accepted */
#define UDATA_(x) (x)->xti_wndata

	CHECKPOINT("xti_unitdata_req");

	xtip    = (struct xticb *)q->q_ptr;
	xp     = &(xtip->xti_proto);
	pp     = (union T_primitives *)mp->b_rptr;

	/*
	 * For TPI allowable fatal errors, call xti_cleanup
	 * otherwise, call xti_uderror_ind
	 */
	if ((newState = TNEXTSTATE(xtip, TE_UNITDATA_REQ)) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: would place interface out of state\n");
		xti_cleanup(q, mp, TOUTSTATE);
		return;
	}

	if (xp->xp_tsdulen == XTI_NS) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: TSDU not supported\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	if (xp->xp_tsdulen > 0 && msgdsize(mp) > xp->xp_tsdulen) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: TSDU size exceeded\n");
		xti_cleanup(q, mp, EPROTO);
		return;
	}
	if (mp->b_wptr - mp->b_rptr < sizeof(struct T_unitdata_req)) {
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			    SL_TRACE|SL_ERROR,
			    "xti_unitdata_req: incorrect message size\n");
		return;
	}

	error = xti_copy_to_mbuf(mp->b_rptr + pp->unitdata_req.DEST_offset,
			 pp->unitdata_req.DEST_length, MT_SONAME, &nam);
	switch (error) {
	case ENOBUFS:
		xti_bufcall(q, pp->unitdata_req.DEST_length, BPRI_HI,
		    XTI_TIMEOUT);	
		putbq(q, mp);
		return;
	case EINVAL:
		if (xti_snd_error_ack(q, mp, TSYSERR, EINVAL))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_unitdata_req: xti_copy_to_mbuf failed\n");
		return;
	}

	size = (mp->b_wptr - mp->b_rptr);

	if ((size < sizeof(struct T_unitdata_req)) ||
	    (size < (pp->unitdata_req.DEST_length +
			pp->unitdata_req.DEST_offset)) ||
	    (size < (pp->unitdata_req.OPT_length +
			pp->unitdata_req.OPT_offset))){
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: incorrect message size\n");
		xti_uderror_ind(q, mp, nam, XTIU_BADMSGSZ);
		return;
	}

	if (pp->unitdata_req.OPT_length < 0 ||
	    (pp->unitdata_req.OPT_length && xtip->xti_proto.xp_optlen == XTI_NS) ||
	    (xtip->xti_proto.xp_optlen >= 0 &&
	     pp->unitdata_req.OPT_length > xtip->xti_proto.xp_optlen)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_req: incorrect option size\n");
		xti_uderror_ind(q, mp, nam, XTIU_BADOPTSZ);
		return;
	}
	xtip->xti_state = newState;

	if (UDATA_(xtip))
		xti_panic("xti_unidata_req wdata");

	if (UDATA_(xtip) = unlinkb(mp)) {
		xtip->xti_wnam = nam;
		xtip->xti_flags &= ~XTI_MORENDATA;
		XTITRACE(XTIF_DATA,
		   printf(" xtiso: xti_unitdata_req() - msgdsize(mp=%x) = %d\n",
			  UDATA_(xtip), msgdsize(UDATA_(xtip))););
		if (xti_canput(xtip, msgdsize(UDATA_(xtip))) >= 0) {
			if (error = xti_send(xtip, T_UNITDATA_REQ)) {
				xti_uderror_ind(q, mp, nam, error);
				return;
			}
		} else {
			freemsg(UDATA_(xtip));
			UDATA_(xtip) = 0;
			(void) m_free(nam);
			xtip->xti_wnam = 0;
		}
	} else
		(void) m_free(nam);

	if (mp)
		freemsg(mp);

#undef UDATA_
}


/*
 * =======================================
 * XTI service routines for socket events.
 * =======================================
 */


xti_send(xtip, type)
	register struct xticb *xtip;
	int type;
{
	struct mbuf *data = 0, *nam = 0;
	int error, flag = 0;
	mblk_t **pmblk, *mp;
	CHECKPOINT("xti_send");

	switch (type) {
	case T_UNITDATA_REQ:
		nam = xtip->xti_wnam;
		/* fall through */
	case T_DATA_REQ:
		pmblk = &xtip->xti_wndata;
		break;
	case T_EXDATA_REQ:
		if (! (mp = dupmsg(xtip->xti_wexpdata))) {
		    xti_bufcall(xtip->xti_wq,
			msgdsize(xtip->xti_wexpdata), 0, XTI_TIMEOUT);
		    return (ENOBUFS);
		}
		pmblk = &mp;
		flag |= MSG_OOB;
		break;
	}
	if (xtip->xti_flags & XTI_SETEOR)
		flag |= MSG_EOR;

	XTITRACE(XTIF_SEND,
		printf(" xtiso: xti_send(xtip=%x type=%d) msgdsize=%d nam=%x\n",
			xtip, type, msgdsize(*pmblk), nam););

	data = mblk_to_mbuf(*pmblk, M_DONTWAIT);

	if (data == 0) {
		if (type != T_UNITDATA_REQ)
			xti_bufcall(xtip->xti_wq,
				    msgdsize(*pmblk), 
				    0, XTI_TIMEOUT);
		else {
			/* If DATA_REQ free wdata, if EXDATA_REQ free dup. */
			freemsg(*pmblk);
			*pmblk = 0;
			/* If expedited data, free original. */
			if (type == T_EXDATA_REQ) {
			    freemsg(xtip->xti_wexpdata);
			    xtip->xti_wexpdata = 0;
			}
			/* preserve nam for uderror_ind */
			xtip->xti_wnam = 0;
		}
		return (ENOBUFS);
	}

	xtip->xti_wnam = 0;
	xtip->xti_flags &= ~XTI_FLOW;
	xtip->xti_flags &= ~XTI_SETEOR;

	XTITRACE(XTIF_SEND,
		printf(" xtiso: xti_send() - sosend(%x)\n", xtip->xti_so););

	error = sosend(xtip->xti_so, nam, (struct uio *)0, data,
			(struct mbuf *)0, flag);

        /* The socket layer/transport frees the data mbuf in all cases.  For
         * expedited data, if the data was accepted by the transport, free the
         * original - the dupliate was already freed by the transport.  If
         * the transport couldn't accept the data, the mblk is requeued so
         * it can be resent later.
         */
	if (type == T_EXDATA_REQ) {
	    if (error == 0) {
		freemsg(xtip->xti_wexpdata);
		xtip->xti_wexpdata = 0;
		xtip->xti_etsdu = 0;
	    }
	} else {
	    *pmblk = 0;
	}

#if XTIDEBUG
	/*
	 * There are a bunch of reasons why we will get errors,
	 *   o Datagram sockets will have transient errors,
	 *     at least it needs to be passed up to the application code.
	 *   o Flow restrictions will return EWOULDBLOCK and needs to
	 *     be handled properly. callers of xti_send take care of this.
	 *     Although xti_canput filters most such situations, it can't
	 *     guarantee a complete avoidance of transport "clogging".
	 *
	 */
	if (error)
		printf(" xtiso: xticb %x sosend error %d: data lost\n",
			xtip, error);
#endif

	if (type != T_UNITDATA_REQ) {
		if (nam)
			(void) m_free(nam);
		return(error);
	}
	/* If error, preserve nam for uderror_ind */
	if (error == 0 && nam)
		(void) m_free(nam);
	return(error);
}


xti_conn_ind(xtip)
	register struct xticb *xtip;
{
	struct  xtiseq *seq;
	struct   mbuf *nam = NULL;
	mblk_t  *mp = NULL;
	union   T_primitives *pp;
	queue_t *q;
	struct mbuf *m = NULL, *data_m = NULL;
	int error, mlen = 0, data_mlen = 0, size;

	CHECKPOINT("xti_conn_ind");

	q = xtip->xti_rq;
	seq = xtip->xti_seq;
	do {
		if (seq->seq_used == XTIS_AWAITING)
			break;
	} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);

	if (seq >= &xtip->xti_seq[xtip->xti_qlen] || seq->seq_so == 0 ||
	    TNEXTSTATE(xtip, TE_CONN_IND) == TS_BAD_STATE)
	{
        	xti_cleanup(q, mp, EPROTO);
		goto bad;
	}

	/*
	 * "getpeername"
	 */
	if (sogetaddr(seq->seq_so, &nam, 1, SOCKNAME_FORMAT))
	{
	    xti_cleanup(q, mp, EPROTO);
	    goto bad;
	}

	/*
	 * Check for connect data if the Transport supports it
	 */
	if (xtip->xti_proto.xp_options && xtip->xti_proto.xp_condata) {
	    XTITRACE(XTIF_CONNECT,
	          printf("calling sogetopt(CONDATA, %d, %d)\n",
			 xtip->xti_proto.xp_opt_level,
			 xtip->xti_proto.xp_condata
		        ););
	    if (error = sogetopt(seq->seq_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_condata,
				 &data_m) ) {
#if XTIDEBUG
	        XTITRACE(XTIF_CONNECT,
			printf("xti_conn_ind: sogetopt(%d, condata=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_condata, 
			       error););
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		goto bad;
	    }
	    data_mlen = data_m->m_len;
	}

	/*
	 * Get options if supported by the transport
	 */
	if (xtip->xti_proto.xp_options) {
	    XTITRACE(XTIF_CONNECT,
		printf("calling sogetopt(CONOPTS, %d, %d)\n",
			xtip->xti_proto.xp_opt_level,
			xtip->xti_proto.xp_conopts
		      ););
	    if (error = sogetopt(seq->seq_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_conopts,
				 &m) ) {
#if XTIDEBUG
	        XTITRACE(XTIF_CONNECT,
			printf("xti_conn_ind: sogetopt(%d, conopts=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_conopts, 
			       error);
			);
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		goto bad;
	    }
	    mlen = m->m_len;
	}

	
	size = sizeof(struct T_conn_ind) + nam->m_len + mlen;
	if ((mp = allocb(size, BPRI_MED)) == (mblk_t *)0) {
		xti_bufcall(q, size, BPRI_MED, 0);
		xtip->xti_pendind = T_CONN_IND;
		goto bad;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_conn_ind) + nam->m_len 
		+ mlen;
	pp = (union T_primitives *)mp->b_rptr;
	pp->type = T_CONN_IND;
	seq->seq_no = xtip->xti_seqcnt++;
	seq->seq_used = XTIS_ACTIVE;
	pp->conn_ind.SRC_offset = sizeof(struct T_conn_ind);
	pp->conn_ind.SRC_length = nam->m_len;
	pp->conn_ind.SEQ_number = seq->seq_no;
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->conn_ind.SRC_offset),
		(unsigned)nam->m_len);

	/*
	 * Copy options if supported by the transport
	 */
	if (mlen)
	{
	    pp->conn_ind.OPT_offset = pp->conn_ind.SRC_offset + 
		    pp->conn_ind.SRC_length;
	    bcopy(mtod(m, caddr_t), (caddr_t)pp + pp->conn_ind.OPT_offset,
		  m->m_len);
	    pp->conn_ind.OPT_length = m->m_len;
         } else {
	    pp->conn_ind.OPT_offset = 0;
	    pp->conn_ind.OPT_length = 0;
	 }

	/*
	 * Copy connect data if supported by the transport.
	 */
	if (data_mlen)
	{
	    if ((mp->b_cont = allocb(data_mlen, BPRI_MED)) == (mblk_t *)0) {
		    xti_bufcall(q, data_mlen, BPRI_MED, 0);
		    xtip->xti_pendind = T_CONN_IND;
		    goto bad;
	    }
	    mp->b_cont->b_wptr = mp->b_cont->b_rptr + data_mlen;
	    bcopy(mtod(data_m, caddr_t), mp->b_cont->b_rptr, 
		  (unsigned)data_m->m_len);
	}

	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_conn_ind() -> putnext(q=%x mp=%x)\n", q, mp););
	NEXTSTATE(xtip, TE_CONN_IND, "xti_conn_ind");
	PUTNEXT (q, mp);

bad:	if (nam)
	    (void) m_free(nam);
	if (data_m)
	    (void) m_freem(data_m);
	if (m)
	    (void) m_freem(m);
	return;
}


xti_conn_con(xtip)
	register struct xticb *xtip;
{
	struct  xtiseq *seq;
	struct   mbuf *nam = NULL, *m = NULL, *data_m = NULL;
	mblk_t  *mp = NULL;
	union   T_primitives *pp;
	queue_t *q;
	int	error, mlen = 0, data_mlen = 0, size;

	CHECKPOINT("xti_conn_con");

	q = xtip->xti_rq;
	if (TNEXTSTATE(xtip, TE_CONN_CON) == TS_BAD_STATE)
	{
	    xti_cleanup(q, mp, EPROTO);
	    goto bad;
	}

	/*
	 * "getpeername"
	 */
	if (sogetaddr(xtip->xti_so, &nam, 1, SOCKNAME_FORMAT))
	{
	    xti_cleanup(q, mp, EPROTO);
	    goto bad;
	}

	/*
	 * Check for connect data if the Transport supports it
	 */
	if (xtip->xti_proto.xp_options && xtip->xti_proto.xp_condata) {
	    if (error = sogetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_condata,
				 &data_m) ) {
#if XTIDEBUG
		printf("xti_conn_con: sogetopt(%d, condata=%d) error = %d\n",
		       xtip->xti_proto.xp_opt_level, 
		       xtip->xti_proto.xp_condata, 
		       error);
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		goto bad;
	    }
	    data_mlen = data_m->m_len;
	}

	/*
	 *  Get conn_con options from the xport if options are supported
	 */
	if (xtip->xti_proto.xp_options)
	{
	    if (error = sogetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_conopts,
				 &m) ) {
#if XTIDEBUG
		printf("xti_conn_con: sogetopt(%d, conopts=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_conopts, 
			       error);
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		goto bad;
	    }
	    mlen = m->m_len;
        }
		
	size = sizeof(struct T_conn_con) + nam->m_len + mlen;
	if ((mp = allocb(size, BPRI_MED)) == (mblk_t *)0) {
		xti_bufcall(q, size, BPRI_MED, 0);
		xtip->xti_pendind = T_CONN_CON;
		goto bad;
	}

	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_conn_con) + nam->m_len 
		+ mlen;
	pp = (union T_primitives *)mp->b_rptr;
	pp->type = T_CONN_CON;
	pp->conn_con.RES_offset = sizeof(struct T_conn_con);
	pp->conn_con.RES_length = nam->m_len;
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr + pp->conn_con.RES_offset),
		(unsigned)nam->m_len);

	/*
	 *  Copy options for user if xport is supporting
	 */
	if ((xtip->xti_proto.xp_options) && mlen) {
	    pp->conn_con.OPT_offset = pp->conn_con.RES_offset + 
		    pp->conn_con.RES_length;
	    pp->conn_con.OPT_length = m->m_len;
	    bcopy(mtod(m, caddr_t), (caddr_t)pp + pp->conn_con.OPT_offset,
		  m->m_len);
	    pp->conn_con.OPT_length = m->m_len;
	} else {
	    pp->conn_con.OPT_offset = 0;
	    pp->conn_con.OPT_length = 0;
	}
	
	/*
	 * Copy connect data if supported by the transport.
	 */
	if (data_mlen)
	{
	    if ((mp->b_cont = allocb(data_mlen, BPRI_MED)) == (mblk_t *)0) {
		    xti_bufcall(q, data_mlen, BPRI_MED, 0);
		    xtip->xti_pendind = T_CONN_IND;
		    goto bad;
	    }
	    mp->b_cont->b_wptr = mp->b_cont->b_rptr + data_mlen;
	    bcopy(mtod(data_m, caddr_t), mp->b_cont->b_rptr, 
		  (unsigned)data_m->m_len);
	}

	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_conn_con() -> putnext(q=%x mp=%x)\n", q, mp););
	NEXTSTATE(xtip, TE_CONN_CON, "xti_conn_con");
	PUTNEXT (q, mp);

bad:	if (nam)
	    (void) m_free(nam);
	if (data_m)
	    (void) m_freem(data_m);
	if (m)
	    (void) m_freem(m);
	return;
}


xti_discon_ind(xtip)
	register struct xticb *xtip;
{
	struct xtiseq *seq;
	mblk_t *mp = NULL;
	union T_primitives *pp;
	queue_t *q;
	int discon_event, error = 0, discon_reason = 0;
	struct mbuf *m = NULL, *data_m = NULL;

	CHECKPOINT("xti_discon_ind");

	q = xtip->xti_rq;
	seq = xtip->xti_seq;

	switch(xtip->xti_state) {
	case TS_DATA_XFER:
	case TS_WIND_ORDREL:
		if ((xtip->xti_proto.xp_servtype == T_COTS_ORD) ||
		    (xtip->xti_proto.xp_servtype == T_COTS))
			if (! xti_snd_flushrw(q)) {
				xti_snd_error_ack(q, mp, TSYSERR, ENOMEM);
				return;
			}
	}

	if (xtip->xti_cindno == 0) {
		discon_event = TE_DISCON_IND1;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND1\n"););
	} else if (xtip->xti_cindno == 1) {
		discon_event = TE_DISCON_IND2;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND2\n"););
	} else if (xtip->xti_cindno > 1)  {
		discon_event = TE_DISCON_IND3;
		XTITRACE(XTIF_CONNECT,
			printf(" xtiso: xti_discon_ind() - TE_DISCON_IND3\n"););
	} else {
		/* should not occur */
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_ind: invalid connection indication count\n");
		goto bad;
	}

	if (TNEXTSTATE(xtip, discon_event) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_discon_ind: would place interface out of state\n");
		xti_cleanup(q, mp, EPROTO);
		goto bad;
	}

	if ((mp = allocb(sizeof(struct T_discon_ind), BPRI_HI)) == (mblk_t *)0) 	{
		xti_bufcall(q, sizeof(struct T_discon_ind), BPRI_HI, 0);
		xtip->xti_pendind = T_DISCON_IND;
		goto bad;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_ind);
	pp = (union T_primitives *)mp->b_rptr;
	pp->discon_ind.PRIM_type = T_DISCON_IND;

	/*
	 * Get disconnect reason if supported by the transport
	 */
	if (xtip->xti_proto.xp_options) {
	    if (error = sogetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level, 
				 xtip->xti_proto.xp_disconreas, 
				 &m) ) {
#if XTIDEBUG
		printf("xti_discon_ind: sogetopt(%d, disreas=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_disconreas, 
			       error);
#endif
		if (xti_snd_error_ack(q, mp, TSYSERR, error))
			strlog(XTI_INFO_ID, xtip->xti_minor,0,
			SL_TRACE|SL_ERROR,
			"xti_discon_ind: sogetopt error status %d\n", error);
		goto badstate;
	    }
	    discon_reason = *mtod(m, int *);
	}

	/*
	 * Check for disconnect data if the Transport supports it
	 */
	if (xtip->xti_proto.xp_options && xtip->xti_proto.xp_discondata) {
	    if (error = sogetopt(xtip->xti_so, 
				 xtip->xti_proto.xp_opt_level,
				 xtip->xti_proto.xp_discondata,
				 &data_m) ) {
#if XTIDEBUG
		printf("xti_disconn_ind: sogetopt(%d, disdata=%d) error = %d\n",
			       xtip->xti_proto.xp_opt_level, 
			       xtip->xti_proto.xp_discondata, 
			       error);
#endif
		xti_snd_error_ack(q, mp, TSYSERR, error);
		goto badstate;
	    }
	    if ((mp->b_cont = allocb(data_m->m_len, BPRI_MED)) == (mblk_t *)0) {
		    xti_bufcall(q, data_m->m_len, BPRI_MED, 0);
		    xtip->xti_pendind = T_CONN_IND;
		    goto bad;
	    }
	    mp->b_cont->b_wptr = mp->b_cont->b_rptr + data_m->m_len;
	    bcopy(mtod(data_m, caddr_t), mp->b_cont->b_rptr, 
		  (unsigned)data_m->m_len);
	}

	switch (discon_event) {
	case TE_DISCON_IND1:
		if (xtip->xti_state == TS_WCON_CREQ)
			pp->discon_ind.DISCON_reason = discon_reason ? discon_reason
				: XTID_REMREJECT;
		else
			pp->discon_ind.DISCON_reason = discon_reason ? discon_reason
				: XTID_REMINIT;
		pp->discon_ind.SEQ_number = -1;
		break;

	case TE_DISCON_IND2:
	case TE_DISCON_IND3:
		seq = xtip->xti_seq;
		do {
			if (seq->seq_used == XTIS_LOST) {
				seq->seq_used = XTIS_AVAILABLE;
				pp->discon_ind.SEQ_number = seq->seq_no;
				if (xtip->xti_cindno-- == xtip->xti_qlen)
					qenable(xtip->xti_rq);
				break;
			}
		} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);
		pp->discon_ind.DISCON_reason = discon_reason ? discon_reason 
			: XTID_REMWITHDRAW;
		break;
	}
	XTITRACE(XTIF_CONNECT,
	   printf(" xtiso: xti_discon_ind() - putnext(q=%x, mp=%x)\n", q, mp););
	PUTNEXT(q, mp);
badstate:
	NEXTSTATE(xtip, discon_event, "xti_discon_ind");

bad:	if (m)
	    (void) m_freem(m);
	if (data_m)
	    (void) m_freem(data_m);
	return;
}


xti_data_ind(indication, xtip)
	register struct xticb *xtip;
{
	union   T_primitives *pp;
	queue_t *q;
	mblk_t  *mp;

	CHECKPOINT("xti_data_ind");

    	mp = 0;
	q = xtip->xti_rq;
       	if (TNEXTSTATE(xtip, TE_DATA_IND) == TS_BAD_STATE ||
       	    TNEXTSTATE(xtip, TE_EXDATA_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_data_ind: would place interface out of state\n");
		goto bad;
	}
	if ((mp = allocb(sizeof(struct T_data_ind), BPRI_MED)) == (mblk_t *)0) {
		xti_bufcall(q, sizeof(struct T_data_ind), BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_data_ind);
	pp = (union T_primitives *)mp->b_rptr;
	pp->data_ind.MORE_flag = NULL;
	if (xti_rcv(xtip, &mp->b_cont, &pp->data_ind.MORE_flag,
		    (mblk_t *)0, 0) == 0) {
		if (pp->data_ind.MORE_flag & T_EXPEDITED) {
			NEXTSTATE(xtip, TE_EXDATA_IND, "xti_exdata_ind");
			pp->type = T_EXDATA_IND;
		} else {
			NEXTSTATE(xtip, TE_DATA_IND, "xti_data_ind");
			pp->type = T_DATA_IND;
		}
		XTITRACE(XTIF_DATA,
		    printf(" xtiso: xti_data_ind() -> putnext(q=%x mp=%x)\n",
			q, mp););
		PUTNEXT (q, mp);
		return;
	}
	if (xtip->xti_bufcallid)
		goto pend;

bad:	xti_cleanup(q, mp, EPROTO);
	return;

pend:   xtip->xti_pendind = indication;
	if (mp)
		freemsg(mp);
}


xti_ordrel_ind(xtip)
	register struct xticb *xtip;
{
	union   T_primitives *pp;
	queue_t *q;
	mblk_t  *mp;

	CHECKPOINT("xti_ordrel_ind");

    	mp = NULL;
	q = xtip->xti_rq;
       	if (TNEXTSTATE(xtip, TE_ORDREL_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_ordrel_ind: would place interface out of state\n");
		goto bad;
	}
	if ((mp = allocb(sizeof(struct T_ordrel_ind), BPRI_MED)) == (mblk_t *)0) {
		xti_bufcall(q, sizeof(struct T_ordrel_ind), BPRI_MED, 0);
		goto pend;
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ordrel_ind);
	pp            = (union T_primitives *)mp->b_rptr;
	pp->type      = T_ORDREL_IND;
	XTITRACE(XTIF_CONNECT,
	    printf(" xtiso: xti_ordrel_ind() -> putnext(q=%x mp=%x)\n",
		q, mp););
	PUTNEXT (q, mp);
	NEXTSTATE(xtip, TE_ORDREL_IND, "xti_ordrel_ind");
	return;

bad:	xti_cleanup(q, mp, EPROTO);
	return;

pend:   xtip->xti_pendind = T_ORDREL_IND;
}


xti_unitdata_ind(xtip)
	register struct xticb *xtip;
{
	int flags = 0;
	union T_primitives *pp;
	queue_t *q;
	mblk_t *namblk = NULL;
	mblk_t *datblk = NULL;
	int err;

	CHECKPOINT("xti_unitdata_ind");

	q = xtip->xti_rq;
       	if (TNEXTSTATE(xtip, TE_UNITDATA_IND) == TS_BAD_STATE) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_unitdata_ind: would place interface out of state\n");
		err = EPROTO;
		goto bad;
	}

	if ((err = xti_rcv(xtip, &datblk, &flags,
			   &namblk, sizeof(struct T_unitdata_ind))) == 0) {
#if XTIDEBUG
		if (namblk == 0)
			xti_panic("xti_unitdata_ind: xti_rcv returned NULL namblk");
#endif
		namblk->b_cont = datblk;
		pp = (union T_primitives *)namblk->b_rptr;
		pp->type = T_UNITDATA_IND;
		pp->unitdata_ind.OPT_offset = 0;
		pp->unitdata_ind.OPT_length = 0;
		pp->unitdata_ind.SRC_offset = sizeof(struct T_unitdata_ind);
		NEXTSTATE(xtip, TE_UNITDATA_IND, "xti_unitdata_ind");
		pp->unitdata_ind.SRC_length =
		    (namblk->b_wptr - namblk->b_rptr) - sizeof(struct T_unitdata_ind);
		XTITRACE(XTIF_DATA,
		   printf(" xtiso: xti_unitdata_ind() -> putnext(q=%x namblk=%x)\n",
			q, namblk););
		PUTNEXT (q, namblk);
		return;
	}
	/* 
	 * Some kind of error. If waiting for a buffer, then fine, 
	 * else abort by calling xti_cleanup
	 */
	if (xtip->xti_bufcallid) {
		xtip->xti_pendind = T_UNITDATA_IND;
		if (namblk)
			(void) freeb(namblk);
		return;
	}
	
bad:
	if (!err)
		err = TSYSERR;
	xti_cleanup(q, namblk, err);
}


xti_uderror_ind(q, mp, nam, error)
	register queue_t *q;
	register mblk_t *mp;
	struct mbuf *nam;
	int error;
{
	register struct xticb *xtip;
	register union T_primitives *pp, *pp1;
	register mblk_t *mp1;

	CHECKPOINT("xti_uderror_ind");

	XTITRACE(XTIF_DATA,
		printf(" xtiso: xti_uderror_ind() error=%d\n", error););

	xtip = (struct xticb *)q->q_ptr;
	pp = (union T_primitives *)mp->b_rptr;

	if (((mp->b_datap->db_lim - mp->b_datap->db_base))  < 
	    (sizeof(struct T_uderror_ind) + pp->unitdata_req.DEST_length)) { 
		freemsg(mp);
		if ((mp = allocb((sizeof(struct T_uderror_ind) + 
		    pp->unitdata_req.DEST_length), BPRI_HI)) == (mblk_t *)0)
			goto bad;
	} else 
		mp->b_rptr = mp->b_datap->db_base;

	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_uderror_ind);
	pp1 = (union T_primitives *)mp->b_rptr;
	pp1->uderror_ind.PRIM_type = T_UDERROR_IND;
	pp1->uderror_ind.DEST_length = pp->unitdata_req.DEST_length;
	pp1->uderror_ind.DEST_offset = pp->unitdata_req.DEST_offset + 
		sizeof(pp->uderror_ind.ERROR_type);
	pp1->uderror_ind.OPT_length = pp->unitdata_req.OPT_length;
	pp1->uderror_ind.OPT_offset = pp->unitdata_req.OPT_offset + 
		sizeof(pp->uderror_ind.ERROR_type);
	bcopy(mtod(nam, caddr_t),
		(caddr_t)(mp->b_rptr  + pp1->uderror_ind.DEST_offset),
		(unsigned)pp1->uderror_ind.DEST_length);
	
	if (nam)
		m_free(nam);
	XTITRACE(XTIF_DATA,
	    printf(" xtiso: xti_uderror_ind() -> putnext(q=%x mp=%x)\n",
		q, mp););
	NEXTSTATE(xtip, TE_UDERROR_IND, "xti_uderror_ind");
	PUTNEXT(xtip->xti_rq, mp);
	return;
bad:	
	if (nam)
		m_free(nam);
	xti_snd_error_ack(q, mp, TSYSERR, 0);
	return;
}


xti_rcv(xtip, dp, moreflagp, addr, addr_offset)
	register struct xticb *xtip;
	mblk_t **dp;		/* Location to hold data mblk. dp must not
				   be null */
	int    *moreflagp;
	mblk_t **addr;		/* Location to hold mblk containing the
				   remote address. May be passed as NULL if
				   remote address is not desired. Otherwise,
				   on return, *addr holds a ptr to the mblk
				   containing the remote address (note - caller
				   assumes this is a single mblk, not a chain).
				   The actual address is stored at offset
				   addr_offset in the mblk (ie. starting at
				   location (*addr)->b_rptr + addr_offset).
				   Its length can be calculated by subtracting
				   this from (*addr)->b_wptr.
				   The mblk's type field is set to M_PROTO.
				   Note that if addr_offset is 0,
				   and there is no address associated with
				   the received packet, no mblk will be
				   allocated and *addr will be null.*/
	int    addr_offset;	/* Offset into the *addr mblk at which to
				   store the address. This is so that the
				   caller can fill in additional info
				   (like an indication) in the same mblk. It
				   is caller's responsibility to make sure
				   this value is such that the stored address
				   will be aligned properly if required.*/
{
	int error, size;
	register struct xtiproto *xp;

	CHECKPOINT("xti_rcv");

	XTITRACE(XTIF_RECV,
		printf(" xtiso: xti_rcv(xtip=%x dp=%x addr=%x addr_offset=%d)\n",
			xtip, dp, addr, addr_offset););

	*moreflagp = NULL;
	*dp = 0;
	if (addr) *addr = NULL;

	xp = &(xtip->xti_proto);
	if (xtip->xti_rdata == 0) {		/* No pending read data */
		struct uio uio;

		size = xp->xp_tidulen;
		/* This assumes that 
		 * 	estdulen <= tidulen && estdulen <= tsdulen */
		if (xp->xp_tsdulen > 0 && size > xp->xp_tsdulen)
			size = xp->xp_tsdulen;
		uio.uio_resid = size;
		/*
		 *
		 * NOTE:
		 * Expedited data "kludge" alert.
		 * For some transports, xtip->xti_rflags will have the MSG_OOB
		 * bit set (if detected via xti_new_tp_ind()), implying that
		 * we are explicitly asking for the expedited data.
		 * However, for some protocols, this would not be known, so
		 * at the time of calling soreceive, xtip->xti_rflags will
		 * not have MSG_OOB set, but still the socket layer and
		 * the protocol layer may return the data with MSG_OOB set in
		 * xtip->xti_rflags, indicating that the data returned was
		 * expedited data.
		 * The code that follows soreceive() is blissfully ignorant
		 * of this "hack" and works just fine in either case.  As of
		 * this writing the example of the 1st case is TCP and of the
		 * 2nd is OSI TP4. - sandeep
		 *
		 */
		error = soreceive(xtip->xti_so,
				addr ? &xtip->xti_rnam : (struct mbuf **)0,
				&uio, &xtip->xti_rdata,
				(struct mbuf **)0, &xtip->xti_rflags);
		size -= uio.uio_resid;
		XTITRACE(XTIF_RECV,
			printf(" xtiso: soreceive (%d) read %d\n",error,size););
		if (size)
			error = 0;
		if (error)
			return error;
	} else {
		struct mbuf *mp;
		for (size = 0, mp = xtip->xti_rdata; mp; mp = mp->m_next)
			size += mp->m_len;
		XTITRACE(XTIF_RECV,
			printf(" xtiso: previous soreceive read %d\n", size););
	}

	/* 
	 * Try for a address block first, then the data. Recovery from allocb
	 * failures is slightly easier if done in this order.
	 *
	 * xxx - like the original code this assumes the address fits in the
	 *  first mbuf
	 */
	if (addr) {
		int addr_len;
		addr_len = addr_offset
			+ (xtip->xti_rnam ? xtip->xti_rnam->m_len : 0);
		/* Only allocate mblk if total length is non-0 */
		if (addr_len) {
			if ((*addr=allocb(addr_len, BPRI_MED)) == (mblk_t *)0){
				xti_bufcall(xtip->xti_rq,addr_len,BPRI_MED,0);
				return ENOBUFS;
			}
			(*addr)->b_datap->db_type = M_PROTO;
			(*addr)->b_wptr = (*addr)->b_rptr + addr_offset;
		}
	}

	if ((*dp = (mblk_t *)mbuf_to_mblk(xtip->xti_rdata, BPRI_MED)) == 0) {
		if (addr && *addr) {
			freeb(*addr);
			*addr = 0;
		}
		xti_bufcall(xtip->xti_rq, size, BPRI_MED, 0);
		return ENOBUFS;
	}

	/* Both allocations successful, so copy the address */
	if (addr && *addr && xtip->xti_rnam) {
		bcopy(mtod(xtip->xti_rnam, caddr_t),
		      (caddr_t)(*addr)->b_wptr,
		      (unsigned)xtip->xti_rnam->m_len);
		(*addr)->b_wptr += xtip->xti_rnam->m_len;
	}
	
	/*
	 * Handle T_MORE, depending on type. Note that the TSDU "option"
	 * is closely tied to the transport behavior here, since EOR will
	 * never be set for non-record oriented transports (e.g. TCP).
	 * However, XTI spec says flag is meaningless in this case.
	 * T_MORE means something additional for exdata.
	 */
	if (xtip->xti_rflags & MSG_OOB) {
		*moreflagp |= T_EXPEDITED;
		if (sbpoll(xtip->xti_so, &xtip->xti_so->so_rcv) & SE_HAVEOOB)
			*moreflagp |= T_MORE;
	} else if ((xp->xp_tsdulen > 0 || xp->xp_tsdulen == -1) &&
		   !(xtip->xti_rflags & MSG_EOR)) {
		*moreflagp |= T_MORE;
	}
	XTITRACE(XTIF_RECV,
		printf(" xtiso: xti_rcv() - size=%d, flags=%x, msgdsize=%d more=%d\n",
		size, xtip->xti_rflags, msgdsize(*dp), *moreflagp););

	if (xtip->xti_rnam)
		(void) m_free(xtip->xti_rnam);
	xtip->xti_rnam = 0;
	xtip->xti_rdata = 0;
	xtip->xti_rflags = 0;

	return 0;
}


/*
 * ==================
 * XTI miscellaneous.
 * ==================
 */

/*
 * xti_cleanup()
 *		Called to clean up the xti connection whenever
 *		an unexpected fatal error occurs.  It will mark
 *		the xticb as "fatal" and reset all fields in the
 *		xticb, send a M_ERROR message to the STREAM head
 *		to inform it the fatal error. From this point on,
 *		all input and output messages will be tossed away.
 *		The only meaningful action is to wait for the
 *		driver xticlose routine being called to close the
 *		current stream.
 */
xti_cleanup(q, mp, error)
	register queue_t *q;
	register mblk_t *mp;
	int error;
{
	register struct xticb *xtip;

	CHECKPOINT("xti_cleanup");

	xtip = (struct xticb *)q->q_ptr;

	XTITRACE(XTIF_CLOSE,
		printf(" xtiso: xti_cleanup(q=%x mp=%x error=%d)\n",
			q, mp, error);
		if (mp) {
			register union T_primitives *pp =
				(union T_primitives *)mp->b_rptr;
			printf(" xtiso: xti_cleanup() - T_primitive type=%x (%d.)\n",
				pp->type, pp->type);
		});

	if (!mp) {
		error = xtip->xti_unixerr;
		goto pendcall;
	}

	/*
	 * Flush data/etc out of our queues
	 */
	flushq(q, FLUSHALL);
	flushq(OTHERQ(q), FLUSHALL);

	xtip->xti_flags |= XTI_FATAL;

	/*
	 * Deinitialize this queue context
	 */
	xti_finished(xtip, XTI_NOSOCK);

	if (mp && (mp->b_datap->db_lim == mp->b_datap->db_base) < 2) { 
		freemsg(mp);
pendcall:
		if ((mp = allocb(sizeof(char), BPRI_HI)) == (mblk_t *)0) {
			xtip->xti_pendcall = xti_cleanup;
			xtip->xti_unixerr = error;
			return 0;
		}
	} else
		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;

	mp->b_datap->db_type = M_ERROR;
	*mp->b_wptr++ = error;		/* send new-style M_ERROR */
	*mp->b_wptr++ = error;

	/*
	 * Send error message up Read queue.
	 * This should cause STREAM head to generate
	 * an M_FLUSH with FLUSHRW.
	 */
	PUTNEXT(xtip->xti_rq, mp);
}


xti_finished(xtip, flags)
	register struct xticb *xtip;
	int flags;
{
	register struct xtiseq *seq;

	CHECKPOINT("xti_finished");

	if (xtip == 0)
		return 0;

	if (xtip->xti_lso)
		(void) soclose(xtip->xti_lso);

	if (xtip->xti_so && xtip->xti_so != xtip->xti_lso) {
		(void) soabort(xtip->xti_so);
		(void) soclose(xtip->xti_so);
	}
	xti_unbufcall(xtip->xti_rq);

	seq = xtip->xti_seq;
	do {
		if (seq->seq_so) {
			(void) soabort(seq->seq_so);
			(void) soclose(seq->seq_so);
		}
	} while (++seq < &xtip->xti_seq[xtip->xti_qlen]);

#if	XTI_XPG4
	if (xtip->xti_def_opts.ip_options)
		(void) m_free(xtip->xti_def_opts.ip_options);
#endif

	if (xtip->xti_wndata)
		freemsg(xtip->xti_wndata);
	if (xtip->xti_wexpdata)
		freemsg(xtip->xti_wexpdata);

	if (xtip->xti_wnam)
		(void) m_free(xtip->xti_wnam);

	if (xtip->xti_rdata)
		m_freem(xtip->xti_rdata);

	if (xti_entry_init(xtip, xtip->xti_rq, flags)) {
		strlog(XTI_INFO_ID, xtip->xti_minor,0, SL_TRACE|SL_ERROR,
			"xti_finished: xti_entry_init failed\n");
		return 0;
	}

	return 1;
}


xti_snd_error_ack(q, mp, tli_error, unix_error)
	queue_t *q;
	register mblk_t *mp;
	int tli_error;
	int unix_error;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	long type;
	int  errState;

	CHECKPOINT("xti_snd_error_ack");

	xtip = (struct xticb *)q->q_ptr;
	if (mp) {
		pp   = (union T_primitives *)mp->b_rptr;
		type = pp->type;
	} else {
		type = xtip->xti_errtype;
		tli_error = xtip->xti_tlierr;
		unix_error = xtip->xti_unixerr;
	}

	XTITRACE(XTIF_ERRORS,
		printf(" xtiso: xti_snd_error_ack(xtip=%x mp=%x tli_error=%d unix_error=%d)\n",
			xtip, mp, tli_error, unix_error););

	if ( !mp || ((mp->b_datap->db_lim - mp->b_datap->db_base) < 
	   (sizeof(struct T_error_ack)))) { 
		if (mp)
			freemsg(mp);
		if ((mp = allocb(sizeof(struct T_error_ack), BPRI_HI)) == 
		    (mblk_t *)0) {
			xtip->xti_tlierr = tli_error;
			xtip->xti_unixerr = unix_error;
			xtip->xti_errtype = type;
			xti_bufcall(xtip->xti_rq, sizeof(struct T_error_ack), BPRI_HI, 0);
			xtip->xti_pendind = T_ERROR_ACK;
			return 0;
		}
	} else {
		/* blow up any other chunks */
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = (mblk_t *)0;
		}
		mp->b_rptr = mp->b_datap->db_base;
	}
	
	mp->b_datap->db_type     = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_error_ack);
	pp = (union T_primitives *)mp->b_rptr;
	pp->error_ack.ERROR_prim = type;
	pp->error_ack.TLI_error  = tli_error;
	pp->error_ack.UNIX_error = unix_error;
	pp->error_ack.PRIM_type  = T_ERROR_ACK;

	/*
	 * Don't change the state if the error is TOUTSTATE.  The xti_fsm
	 * is not geared to handle it.
	 *
	 * Also if any transition takes you to TS_BAD_STATE, either the
	 * table is wrong or the state transition need not be done (In most
	 * cases the latter IS true; and we hope that former cases are cleaned
	 * up, by setting the trace).
	 *
	 */
	if ((tli_error != TOUTSTATE) &&
	    ((errState = TNEXTSTATE(xtip, TE_ERROR_ACK)) != TS_BAD_STATE)) {
		xtip->xti_state = errState;
	}
	XTITRACE(XTIF_ERRORS,
		if ((errState < 0) && (tli_error != TOUTSTATE)) {
			printf("xti_panic: TE_ERROR_ACK transition to illegal state [prev state = %d]\n", xtip->xti_state);
		});

	/*
	 * Send error message back up Read queue
	 */
	PUTNEXT(xtip->xti_rq, mp);
	return 1;
}


int
xti_snd_flushrw(q)
	queue_t *q;
{
	register mblk_t *mp;
	register struct xticb *xtip;

	CHECKPOINT("xti_snd_flushrw");

	/*
	 * Generate a FLUSH queues message type,
	 * pass it up to STREAM head which should
	 * flush read queues and then pass M_FLUSH
	 * downstream to flush write queues
	 */
	xtip = (struct xticb *)q->q_ptr;
	if ((mp = allocb(sizeof(*mp->b_wptr), BPRI_HI)) == (mblk_t *)0)
		return 0;

	mp->b_datap->db_type = M_FLUSH;
	*mp->b_wptr++ = FLUSHRW;
	PUTNEXT(xtip->xti_rq, mp);
	return 1;
}



/*
 * xti_snd_ok_ack - postively acknowledge the current message
 */
xti_snd_ok_ack(q, mp)
	queue_t *q;
	register mblk_t *mp;
{
	register struct xticb *xtip;
	register union T_primitives *pp;
	long type;

	CHECKPOINT("xti_snd_ok_ack");

	xtip = (struct xticb *)q->q_ptr;
	if (mp) {
		pp = (union T_primitives *)mp->b_rptr;
		type = pp->type;
		freemsg(mp);
	}
	else
		type = xtip->xti_errtype;

	/*
	 * Allocate new storage for the 'ok' ack.
	 */

	if ((mp = allocb(sizeof(struct T_ok_ack), BPRI_HI)) == (mblk_t *)0) {
		xtip->xti_errtype = type;
		xti_bufcall(xtip->xti_rq, sizeof(struct T_ok_ack), BPRI_HI, 0);
		return 0;
	}
	mp->b_datap->db_type    = M_PCPROTO;
	mp->b_wptr = mp->b_rptr + sizeof(struct T_ok_ack);
	pp                      = (union T_primitives *)mp->b_rptr;
	pp->ok_ack.CORRECT_prim = type;
	pp->ok_ack.PRIM_type    = T_OK_ACK;

	/*
	 * Send error message back up Read queue
	 */
	PUTNEXT(xtip->xti_rq, mp);
	return 1;
}


/* Buffer Management Routines */
int
xti_copy_to_mbuf(addr, len, type, nam)
	char *addr;
	int len;
	int type;
	struct mbuf **nam;
{
	register struct mbuf *m = NULL;
	int error = 0;

	CHECKPOINT("xti_copy_to_mbuf");

	if ((u_int)len > MLEN)
		return(EINVAL);

	XTITRACE(XTIF_SOCKET,
		printf(" xtiso: addr=%x, len=%d, type=%d, nam=%x\n",
				addr, len, type, nam););

	/*
	 * Check input pointer to location to store mbuf pointer
	 */
	if (nam == (struct mbuf **)0)
		return(EINVAL);

	/*
	 * Attempt to allocate an mbuf
	 */
	*nam = m = m_get(M_DONTWAIT, type);
	if (m == 0)
		return(ENOBUFS);

	XTITRACE(XTIF_SOCKET,
		printf(" xtiso: m_get returns %x\n", m););

	m->m_len = len;
	bcopy((caddr_t)addr, mtod(m, caddr_t), (unsigned)len);

	/*
	 * Setup pointer to sockaddr structure
	 */
	sockaddr_new(m);		/* convert to kernel representation */

	XTITRACE
	(
		XTIF_SOCKET,
		{
			struct sockaddr *sa = mtod(m, struct sockaddr *);
			int i;

			printf(" xtiso:  family: %d\n", sa->sa_family);
			printf("            len: %d\n", sa->sa_len);
			printf("           data: ");
			for (i = 0; i < sizeof(sa->sa_data); i++)
				printf("[%d]=%d ", i, sa->sa_data[i]);
			printf("\n");
		}
	);

	return(error);
} 

xti_bufcall(q, size, pri, type)
	queue_t *q;
	int size, pri, type;
{
	register struct xticb *xtip = (struct xticb *)q->q_ptr;
	
	xtip->xti_flags &= ~XTI_TIMEOUT;
	if (type == XTI_TIMEOUT) {
		xtip->xti_flags |= XTI_TIMEOUT;
		xtip->xti_bufcallid = timeout(qenable, (caddr_t)q, hz/2);
	} else {
		if ((xtip->xti_bufcallid  = 
		    bufcall(size, pri, qenable, (long) q)) == 0) {
			xtip->xti_flags |= XTI_TIMEOUT;
			xtip->xti_bufcallid = timeout(qenable,(caddr_t)q,hz/2);
		}
	}
}

xti_unbufcall(q)
	queue_t *q;
{
	register struct xticb *xtip = (struct xticb *)q->q_ptr;
	register int id;


	id = xtip->xti_bufcallid;
#ifdef XTI11
	if ((xtip->xti_flags & XTI_TIMEOUT) && id)
		unbufcall(id);
	else if (id)
#else
	if (id)
#endif
		untimeout(id);

	xtip->xti_bufcallid = 0;
	qenable(OTHERQ(q));
}


xti_entry_init(xtip, q, flags)
	register struct xticb *xtip;
	queue_t *q;
	int flags;
{
	register struct xtiproto *xp;
	register struct xtiseq *seq;
	int status = 0;

	CHECKPOINT("xti_entry_init");

#if	XTIDEBUG
	if (xtip == 0) {
		xti_panic("xti_entry_init");
		return(1);
	}
#endif

	XTITRACE((XTIF_OPEN | XTIF_CLOSE),
		printf(" xtiso: xti_entry_init() - %d\n", flags););

	/*
	 * (Re-)Init the xti context block
	 */
	xtip->xti_flags 	&= ~(XTI_ACTIVE);
	xtip->xti_state 	= TS_UNBND;
	xtip->xti_so 		= 0;
	xtip->xti_lso 		= 0;
	seq = xtip->xti_seq;
	do {
		seq->seq_used = XTIS_AVAILABLE;
		seq->seq_so   = 0;
		seq->seq_no   = 0;
	} while (++seq < &xtip->xti_seq[xtip->xti_maxqlen]);
	xtip->xti_sostate	= 0;
	xtip->xti_soerror	= 0;
	xtip->xti_sorcount	= 0;
	xtip->xti_sowspace	= 0;
	xtip->xti_sosnap	= 0;
	xtip->xti_qlen		= 0;
	xtip->xti_cindno	= 0;
	xtip->xti_pendind 	= 0;
	xtip->xti_tsdu		= 0;
	xtip->xti_etsdu		= 0;
	xtip->xti_wndata	= 0;
	xtip->xti_wexpdata	= 0;
	xtip->xti_wnam		= 0;
	xtip->xti_rdata		= 0;
	xtip->xti_rnam		= 0;
	xtip->xti_rflags	= 0;
	xtip->xti_bufcallid	= 0;
	xtip->xti_pendcall	= 0;
	xtip->xti_tlierr	= 0;
	xtip->xti_unixerr	= 0;
	xtip->xti_errtype	= 0;
#if	XTI_XPG4
	bzero((caddr_t)&xtip->xti_def_opts, sizeof xtip->xti_def_opts);
#endif

	if (flags == XTI_NEWSOCK) {

		/*
	 	 * For new opens, grab some socket resources and initialize them
	 	 */
		xp = &(xtip->xti_proto);
		status = socreate(xp->xp_dom, &(xtip->xti_so),
				  xp->xp_type, xp->xp_proto);
		if (status == 0) {
			/*
			 * Setup "glue" in socket structure
			 * to make sockets/xtiso work
			 */
			xti_init_socket(xtip->xti_so, q);

			xtip->xti_wq = WR(q);
			xtip->xti_rq = q;
			xtip->xti_flags |= XTI_ACTIVE;
			xtip->xti_flags &= ~XTI_FLOW;

			XTITRACE(XTIF_OPEN,
				printf("\n xtiso: so-> after init...\n");
				DumpSO(xtip->xti_so););

		} else {
			XTITRACE(XTIF_OPEN | XTIF_CLOSE,
				printf(" xtiso: xti_entry_init() - socreate error = %d, dom=%d, type=%d, proto=%d\n",
				       status, xp->xp_dom, xp->xp_type, xp->xp_proto););
		}
	}

	return(status);
}


/*
 * xti_[rw]qenable()
 *
 *	Asynchronous upcalls from the socket layer come here. There are
 *	problems with synchronizing the two, especially regarding locking
 *	hierarchy. The solution is to place an "unsafe" state word in the
 *	xticb which is manipulated by these upcalls, and to use the
 *	sbpoll() mechanism to move it to a local copy, along with the
 *	current space or count. The old state is always or'd together
 *	with new events, and cleared on each poll.
 */

#define DATABITS	(SE_HAVEDATA|SE_HAVEOOB|SE_DATAFULL)

void
xti_wqenable(Q, state)
	caddr_t Q;
	int state;
{
	register queue_t *q = (queue_t *)Q;
	struct xticb *xtip;

	CHECKPOINT("xti_wqenable");

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_wqenable(q=%x, state=%x)\n", q, state););

	if (q == 0)
		return;
	xtip = (struct xticb *)q->q_ptr;
	if (xtip == 0)
		return;
	if (state & SE_STATUS)
		xtip->xti_sosnap |= state;
	if (state & SE_POLL) {
		/*
		 * The sbpoll() call results in this. Synchronously snap
		 * the current state and space, clear the state for later.
		 */
		xtip->xti_sostate |= (xtip->xti_sosnap & ~DATABITS);
		xtip->xti_sowspace = sbspace(&xtip->xti_so->so_snd);
		if (state & SE_ERROR)
			xtip->xti_soerror = xtip->xti_so->so_error;
		xtip->xti_sosnap = 0;
	} else {
		/*
		 * An asynchronous socket event results in this.
		 * If this socket is active, then enable queue,
		 * Otherwise, ignore wakeup call from socket layer
		 */
		if (xtip->xti_flags & XTI_ACTIVE)
			qenable(q);
	}
}

void
xti_rqenable(Q, state)
	caddr_t Q;
	int state;
{
	register queue_t *q = (queue_t *)Q;
	struct xticb *xtip;

	CHECKPOINT("xti_rqenable");

	XTITRACE(XTIF_EVENTS,
		printf(" xtiso: xti_rqenable(q=%x, state=%x)\n", q, state););

	if (q == 0)
		return;
	xtip = (struct xticb *)q->q_ptr;
	if (xtip == 0)
		return;
	if (state & SE_STATUS)
		xtip->xti_sosnap |= state;
	if (state & SE_POLL) {
		/*
		 * The sbpoll() call results in this. Synchronously snap
		 * the current state and count, clear the state for later.
		 */
		xtip->xti_sostate |= (xtip->xti_sosnap & ~DATABITS);
		xtip->xti_sorcount = xtip->xti_so->so_rcv.sb_cc;
		if (state & SE_ERROR)
			xtip->xti_soerror = xtip->xti_so->so_error;
		xtip->xti_sosnap = 0;
	} else {
		/*
		 * An asynchronous socket event results in this.
		 * If this socket is active, then enable queue,
		 * Otherwise, ignore wakeup call from socket layer
		 */
		if (xtip->xti_flags & XTI_ACTIVE)
			qenable(q);
	}
}

/*
 * xti_canput()
 *
 *	Flow control checking for socket sends. Returns additional
 *	space available:
 *		>= 0	data length acceptable
 *		<  0	too big by this much
 *	When returning < 0, sets XTI_FLOW. Note, expedited data
 *	may pass in a negative need, to adjust to socket's special
 *	handling of same.
 *
 *	XXX Problem: if xti_canput request is greater than max
 *	capacity, no send will be attempted and no upcall will
 *	re-awaken it. Caller needs to examine xti_sowspace and
 *	return error, or strictly adhere to tsdulen, which must
 *	then be accurate.
 */
xti_canput(xtip, need)
	register struct xticb *xtip;
	int need;
{
	int space;

	CHECKPOINT("xti_canput");

#if	XTIDEBUG
	if (!xtip->xti_so)
		xti_panic("xti_canput so");
#endif

	/*
	 * Check available space
	 */
	(void) sbpoll(xtip->xti_so, &(xtip->xti_so->so_snd));
	/* Not interested in sostate here (???), so leave it alone */
	space = xtip->xti_sowspace;

	if (space < need) {
		XTITRACE(XTIF_SEND_FLOW, if (!(xtip->xti_flags & XTI_FLOW))
			printf(" xtiso: xti_canput() - setting FLOW CONTROL, space=%d, need=%d\n",
				space, need););
		xtip->xti_flags |= XTI_FLOW;
	} else {
		XTITRACE(XTIF_SEND_FLOW, if (xtip->xti_flags & XTI_FLOW)
			printf(" xtiso: xti_canput() - FLOW CONTROL ready, space=%d, need=%d\n",
				space, need););
	}
	return (space - need);
}

xti_init_socket(so, q)
	register struct socket *so;
	queue_t *q;
{
	CHECKPOINT("xti_init_socket");

	if (!so)
		return 0;
	/*
	 * XXX Unsafe to do this on any attached socket! Fix xti_conn_res...
	 */

	/*
	 * Don't do blocking operations down in Socket layer.
	 * No associated u-area with XTI Sockets.
	 */
	so->so_state   |= SS_NBIO;
	so->so_state   &= ~SS_PRIV;
#ifdef XTI11
/*	so->so_special |= (SP_NOUAREA|SP_EXTPRIV);  mary */
	so->so_special |= SP_NOUAREA;
#else
	so->so_special |= SP_NOUAREA;
#endif

	/*
	 * Current logic dictates that the OOBINLINE is not set, so that
	 * t_rcv can pass such data as soon as it can do so.
	 *
	 */
	/* so->so_options |= SO_OOBINLINE;*/

	/*
	 * Set SB_ASYNC to ensure upcalls.
	 * Make send/recv socket I/O non-interrupted
	 */
	so->so_rcv.sb_flags |= (SB_ASYNC|SB_NOINTR);
	so->so_snd.sb_flags |= (SB_ASYNC|SB_NOINTR);

	/*
	 * Assume the `q' points to read queue
	 */
	so->so_snd.sb_wakeup  = xti_wqenable;
	so->so_snd.sb_wakearg = (caddr_t)WR(q);
	so->so_rcv.sb_wakeup  = xti_rqenable;
	so->so_rcv.sb_wakearg = (caddr_t)q;

	return 1;
}

#if	XTI_XPG4
/*
 *    XTI OPTION MANAGEMENT
 *
 *	Note: most all of the options below default to off or 0,
 *	except for the buffer sizes, which in the case of TCP may
 *	change after connection. We fetch them all anyway, but only
 *	when first asked for one.
 */

xti_init_default_options(xtip)
	struct xticb *xtip;
{
	struct socket *so = xtip->xti_so;
	struct mbuf   *m = 0;

	/*
	 * XTI options.
	 */

	if (!sogetopt(so, SOL_SOCKET, SO_DEBUG, &m)) {
		xtip->xti_def_opts.xti_debug = *mtod(m, int *) ? T_YES : T_NO;
		m_free(m);
		m = 0;
	}

	if (!sogetopt(so, SOL_SOCKET, SO_LINGER, &m)) {
		xtip->xti_def_opts.xti_linger.l_onoff =
		     mtod(m, struct linger *)->l_onoff ? T_YES : T_NO;
		xtip->xti_def_opts.xti_linger.l_linger =
		     mtod(m, struct linger *)->l_linger;
		m_free(m);
		m = 0;
	}

	if (!sogetopt(so, SOL_SOCKET, SO_SNDBUF, &m)) {
		xtip->xti_def_opts.xti_sndbuf = *mtod(m, int *);
		m_free(m);
		m = 0;
	}

	if (!sogetopt(so, SOL_SOCKET, SO_RCVBUF, &m)) {
		xtip->xti_def_opts.xti_rcvbuf = *mtod(m, int *);
		m_free(m);
		m = 0;
	}

	if (!sogetopt(so, SOL_SOCKET, SO_SNDLOWAT, &m)) {
		xtip->xti_def_opts.xti_sndlowat = *mtod(m, int *);
		m_free(m);
		m = 0;
	}

	if (!sogetopt(so, SOL_SOCKET, SO_RCVLOWAT, &m)) {
		xtip->xti_def_opts.xti_rcvlowat = *mtod(m, int *);
		m_free(m);
		m = 0;
	}

	/*
	 * IP options.
	 */
	if (sodomain(so)->dom_family == AF_INET) {

		if (!sogetopt(so, SOL_SOCKET, SO_BROADCAST, &m)) {
			xtip->xti_def_opts.ip_broadcast =
				*mtod(m, int *)? T_YES : T_NO;
			m_free(m);
			m = 0;
		}

		if (!sogetopt(so, SOL_SOCKET, SO_DONTROUTE, &m)) {
			xtip->xti_def_opts.ip_dontroute =
				*mtod(m, int *) ? T_YES : T_NO;
			m_free(m);
			m = 0;
		}

		if (!sogetopt(so, IPPROTO_IP, IP_OPTIONS, &m)) {
			if (m->m_len)
				xtip->xti_def_opts.ip_options = m;
			else
				(void) m_free(m);
			m = 0;
		}

		if (!sogetopt(so, SOL_SOCKET, SO_REUSEADDR, &m)) {
			xtip->xti_def_opts.ip_reuseaddr =
				*mtod(m, int *) ? T_YES : T_NO;
			m_free(m);
			m = 0;
		}

		if (!sogetopt(so, IPPROTO_IP, IP_TOS, &m)) {
			xtip->xti_def_opts.ip_tos = *mtod(m, int *);
			m_free(m);
			m = 0;
		}

		if (!sogetopt(so, IPPROTO_IP, IP_TTL, &m)) {
			xtip->xti_def_opts.ip_ttl = *mtod(m, int *);
			m_free(m);
			m = 0;
		}

		/*
		 * TCP options.
		 */
		if (so->so_type == SOCK_STREAM) {

			if (!sogetopt(so, SOL_SOCKET, SO_KEEPALIVE, &m)) {
				xtip->xti_def_opts.tcp_keepalive.kp_onoff =
					*mtod(m, int *) ? T_YES : T_NO;
				xtip->xti_def_opts.tcp_keepalive.kp_timeout =
					T_NOTSUPPORT;
				m_free(m);
				m = 0;
			} else {
				xtip->xti_def_opts.tcp_keepalive.kp_onoff = 0;
				xtip->xti_def_opts.tcp_keepalive.kp_timeout = 0;
			}

			if (!sogetopt(so, IPPROTO_TCP, TCP_MAXSEG, &m)) {
				xtip->xti_def_opts.tcp_maxseg = *mtod(m, int *);
				m_free(m);
				m = 0;
			}

			if (!sogetopt(so, IPPROTO_TCP, TCP_NODELAY, &m)) {
				xtip->xti_def_opts.tcp_nodelay =
					*mtod(m, int *)? T_YES : T_NO;
				m_free(m);
				m = 0;
			}
		/*
		 * UDP options.
		 * none at present can not be modified...
		 */
		} else
			xtip->xti_def_opts.udp_checksum = T_YES;
	}
	xtip->xti_flags |= XTI_OPTINIT;
}

xti_chk_protocol(opt, xtip, optlen, flags)
	struct t_opthdr *opt;
	struct xticb *xtip;
	int optlen, flags;
{
	if (!(xtip->xti_flags & XTI_OPTINIT))
		xti_init_default_options(xtip);

	switch (opt->level) {

	case XTI_GENERIC:
		return xti_xti_optmgmt(opt, xtip, flags, optlen);

	case INET_IP:
		if (sodomain(xtip->xti_so)->dom_family != AF_INET)
			break;
		return xti_ip_optmgmt(opt, xtip, flags, optlen);

	case INET_TCP:
		if (sodomain(xtip->xti_so)->dom_family != AF_INET)
			break;
		if (xtip->xti_so->so_type != SOCK_STREAM)
			break;
		return xti_tcp_optmgmt(opt, xtip, flags, optlen);

	case INET_UDP:
		if (sodomain(xtip->xti_so)->dom_family != AF_INET)
			break;
		if (xtip->xti_so->so_type != SOCK_DGRAM)
			break;
		return xti_udp_optmgmt(opt, xtip, flags, optlen);

	/* Support more protocols here */

	default:
		break;
	}
	return opt->status = T_FAILURE;
}

xti_xti_optmgmt(opt, xtip, flag, len)
	struct t_opthdr *opt;
	struct xticb  *xtip;
	int flag, len;
{
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opt + 1);
	char *startopt;
	struct mbuf *m = 0;
	struct t_linger *ling;
	int all = 0;

	switch (opt->name) {
	case T_ALLOPT:
		startopt = (char *)opt;
		all = 1;

	case XTI_DEBUG:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_debug;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.xti_debug;
			else if (!sogetopt(so, SOL_SOCKET, SO_DEBUG, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_DEBUG, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case XTI_LINGER:
		ling = (struct t_linger *)mpbuf;
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			ling->l_onoff = xtip->xti_def_opts.xti_linger.l_onoff;
			ling->l_linger = xtip->xti_def_opts.xti_linger.l_linger;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY)) {
				ling->l_onoff = xtip->xti_def_opts.xti_linger.l_onoff;
				ling->l_linger = xtip->xti_def_opts.xti_linger.l_linger;
			} else if (!sogetopt(so, SOL_SOCKET, SO_LINGER, &m)) {
				ling->l_onoff =
				mtod(m, struct linger *)->l_onoff ? T_YES:T_NO;
				ling->l_linger =
				mtod(m, struct linger *)->l_linger;
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (struct linger);
			mtod(m, struct linger *)->l_onoff = ling->l_onoff;
			mtod(m, struct linger *)->l_linger = ling->l_linger;
			if (sosetopt(so, SOL_SOCKET, SO_LINGER, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(struct t_linger) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case XTI_RCVBUF:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_rcvbuf;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.xti_rcvbuf;
			else if (!sogetopt(so, SOL_SOCKET, SO_RCVBUF, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_RCVBUF, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case XTI_SNDBUF:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_sndbuf;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.xti_sndbuf;
			else if (!sogetopt(so, SOL_SOCKET, SO_SNDBUF, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_SNDBUF, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case XTI_RCVLOWAT:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_rcvlowat;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.xti_rcvlowat;
			else if (!sogetopt(so, SOL_SOCKET, SO_RCVLOWAT, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_RCVLOWAT, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case XTI_SNDLOWAT:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.xti_sndlowat;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.xti_sndlowat;
			else if (!sogetopt(so, SOL_SOCKET, SO_SNDLOWAT, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_SNDLOWAT, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(long) + sizeof(struct t_opthdr);
		break;

	default:
		opt->status = T_FAILURE;
		return TBADOPT;
	}
	return 0;
}

xti_ip_optmgmt(opt, xtip, flag, len)
	struct t_opthdr *opt;
	struct xticb *xtip;
	int flag, len;
{
	int all = 0;
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opt + 1);
	char *startopt;
	struct mbuf *m = 0;

	opt->len = 0;

	switch (opt->name) {
	case T_ALLOPT:
		startopt = (char *)opt;
		all = 1;

	case IP_REUSEADDR:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_reuseaddr;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.ip_reuseaddr;
			else if (!sogetopt(so, SOL_SOCKET, SO_REUSEADDR, &m)) {
				*mpbuf = *mtod(m, int *) ? T_YES : T_NO;
				m_free(m);
			}
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_REUSEADDR, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case IP_DONTROUTE:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_dontroute;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.ip_dontroute;
			else if (!sogetopt(so, SOL_SOCKET, SO_DONTROUTE, &m)) {
				*mpbuf = *mtod(m, int *) ? T_YES : T_NO;
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_DONTROUTE, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case IP_BROADCAST:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_broadcast;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.ip_broadcast;
			else if (!sogetopt(so, SOL_SOCKET, SO_BROADCAST, &m)) {
				*mpbuf = *mtod(m, int *) ? T_YES : T_NO;
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, SOL_SOCKET, SO_BROADCAST, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case IP_TOS:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_tos;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.ip_tos;
			else if (!sogetopt(so, IPPROTO_IP, IP_TOS, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, IPPROTO_IP, IP_TOS, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case IP_TTL:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.ip_ttl;
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.ip_ttl;
			else if (!sogetopt(so, IPPROTO_IP, IP_TTL, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, unsigned int *) = *mpbuf;
			if (sosetopt(so, IPPROTO_IP, IP_TTL, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(int) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case IP_OPTIONS:
		mlen = opt->len - sizeof (struct t_opthdr);
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
		t_def:
			bzero((caddr_t)mpbuf, mlen);
			if (xtip->xti_def_opts.ip_options) {
				if (xtip->xti_def_opts.ip_options->m_len > mlen)
					opt->status = T_FAILURE;
				else
					bcopy(mtod(xtip->xti_def_opts.ip_options, caddr_t),
					     (caddr_t)mpbuf, mlen);
			}
			break;
		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				goto t_def;
			else if (!sogetopt(so, IPPROTO_IP, IP_OPTIONS, &m)) {
				if (m->m_len > mlen)
					opt->status = T_FAILURE;
				else {
					if (mlen > m->m_len)
						bzero((caddr_t)mpbuf, mlen);
					bcopy(mtod(m, caddr_t), mpbuf, m->m_len);
				}
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			if (mlen > MLEN)
				opt->status = T_FAILURE;
			else {
				if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
					return (ENOBUFS);
				xtip->xti_flags |= XTI_OPTMODIFY;
				m->m_len = mlen;
				bcopy((caddr_t)mpbuf, mtod(m, caddr_t), m->m_len);
				if (sosetopt(so, IPPROTO_IP, IP_OPTIONS, m))
					opt->status = T_FAILURE;
			}
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = mlen + sizeof(struct t_opthdr);
		break;

	default:
		opt->status = T_FAILURE;
		return TBADOPT;
	}
	return 0;
}

xti_udp_optmgmt(opt, xtip, flag, len)
	struct t_opthdr *opt;
	struct xticb  *xtip;
	int flag, len;
{
	long *mpbuf = (long *)(opt + 1);

	switch (opt->name) {
	case UDP_CHECKSUM:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
		case T_CURRENT:
			*mpbuf = T_YES;
			break;
		case T_CHECK:
			break;
		case T_NEGOTIATE:
			opt->status = T_FAILURE | T_NOTSUPPORT;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
	default:
		opt->status = T_FAILURE;
		return TBADOPT;
	}
	return 0;
}

xti_tcp_optmgmt(opt, xtip, flag, len)
	struct t_opthdr *opt;
	struct xticb *xtip;
	int flag, len;
{
	struct mbuf *m = 0;
	struct socket *so = xtip->xti_so;
	long *mpbuf = (long *)(opt + 1);
	int all = 0;
	struct t_kpalive *kp;
	int mlen;
	char *startopt;

	opt->len = 0;

	switch (opt->name) {
	case T_ALLOPT:
		startopt = (char *)opt;
		all = 1;

	case TCP_NODELAY:
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.tcp_nodelay;
			break;

		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.tcp_nodelay;
			else if (!sogetopt(so, IPPROTO_TCP, TCP_NODELAY, &m)) {
				*mpbuf = *mtod(m, int *) ? T_YES : T_NO;
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;

		case T_CHECK:
			break;

		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = *mpbuf;
			if (sosetopt(so, IPPROTO_TCP, TCP_NODELAY, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
			break;
		}
		opt->len = sizeof(unsigned long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case TCP_MAXSEG:
		opt->status = T_SUCCESS | T_READONLY;
		switch (flag) {
		case T_DEFAULT:
			*mpbuf = xtip->xti_def_opts.tcp_maxseg;
			break;

		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.tcp_maxseg;
			else if (!sogetopt(so, IPPROTO_TCP, TCP_MAXSEG, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;

		case T_CHECK:
			break;

		case T_NEGOTIATE:
			opt->status = T_FAILURE | T_READONLY;
			break;
		default:
			opt->status = T_FAILURE;
		}
		opt->len = sizeof(unsigned long) + sizeof(struct t_opthdr);
		if (!all)
			return 0;
		opt = OPT_NEXTHDR(startopt, len, opt);
		if (!opt)
			return -1;
		mpbuf = (long *)((char *)opt + opt->len);
		/* fall through */

	case TCP_KEEPALIVE:
		kp = (struct t_kpalive *)mpbuf;
		opt->status = T_SUCCESS;
		switch (flag) {
		case T_DEFAULT:
			mpbuf = (long *)((char *)opt + sizeof(struct t_opthdr));
			kp->kp_onoff = xtip->xti_def_opts.tcp_keepalive.kp_onoff;
			kp->kp_timeout = xtip->xti_def_opts.tcp_keepalive.kp_timeout;
			break;

		case T_CURRENT:
			if (!(xtip->xti_flags & XTI_OPTMODIFY))
				*mpbuf = xtip->xti_def_opts.tcp_maxseg;
			else if (!sogetopt(so, SOL_SOCKET, SO_KEEPALIVE, &m)) {
				*mpbuf = *mtod(m, int *);
				m_free(m);
			} else
				opt->status = T_FAILURE;
			break;
		case T_CHECK:
			break;

		case T_NEGOTIATE:
			if (!(m = m_get(M_DONTWAIT, MT_SOOPTS)))
				return (ENOBUFS);
			xtip->xti_flags |= XTI_OPTMODIFY;
			m->m_len = sizeof (int);
			*mtod(m, int *) = kp->kp_onoff;
			if (sosetopt(so, SOL_SOCKET, SO_KEEPALIVE, m))
				opt->status = T_FAILURE;
			break;
		default:
			opt->status = T_FAILURE;
		}
		opt->len = sizeof(struct t_kpalive) + sizeof(struct t_opthdr);
		break;

	default:
		opt->status = T_FAILURE;
		return TBADOPT;
	}
	return 0;
}
#endif	/* XTI_XPG4 */


#if	XTIDEBUG

/*
 * Routines for debugging.
 */

xti_putnext(q, mp)
	queue_t *q;
	mblk_t *mp;
{
	CHECKPOINT("xti_putnext");

	XTITRACE(XTIF_MISC,
		printf(" xtiso: xti_putnext(q=%x mp=%x)\n", q, mp);
		DumpMBLK(mp);
	);

	putnext(q, mp);
}

mblk_t *
xti_allocb(size, pri)
	int size, pri;
{
#undef allocb
	mblk_t *ptr;

	if (!(ptr = allocb(size, pri)))
	    printf("allocb FAILED!!!!!"); /* DEBUG */

	return(ptr);
}
xti_panic(s)
	char *s;
{
	if (xtiDEBUG & XTIF_PANIC)
		panic(s);
	printf(" xtiso: xti_panic \"%s\"\n", s);
	if (xtiDEBUG & XTIF_BREAK)
		Debugger();
}


xti_strlog(  a, b, c, d, fmt, e, f, g)
int a,b,c,d,e,f,g;
char *fmt;
{
	printf(" xtiso: xti_strlog() - (%d %d %d %d) ", a, b, c, d);
	printf(fmt, e, f, g);
}

DumpIOCBLK(p)
	register struct iocblk *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: iocblk = %x...\n", p);
		printf("	  cmd: %d (%x)\n", p->ioc_cmd, p->ioc_cmd);
		printf("	  uid: %d\n", p->ioc_uid);
		printf("	  gid: %d\n", p->ioc_gid);
		printf("	   id: %d\n", p->ioc_id);
		printf("	count: %d\n", p->ioc_count);
		printf("	error: %d\n", p->ioc_error);
		printf("	 rval: %d\n", p->ioc_rval);
		printf("\n");

		status = 0;
	}

	return(status);
}

DumpPROTO(p)
	register struct xtiproto *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: xtiproto = %x...\n", p);
		printf("               dom: %d\n", p->xp_dom);
		printf("              type: %d\n", p->xp_type);
		printf("             proto: %d\n", p->xp_proto);
		printf("          servtype: %d\n", p->xp_servtype);
		printf("	   tsdulen: %d\n", p->xp_tsdulen);
		printf("	  etsdulen: %d\n", p->xp_etsdulen);
		printf("	connectlen: %d\n", p->xp_connectlen);
		printf("  	 disconlen: %d\n", p->xp_disconlen);
		printf("	   addrlen: %d\n", p->xp_addrlen);
		printf("	    optlen: %d\n", p->xp_optlen);
		printf(" 	   tidulen: %d\n", p->xp_tidulen);
		printf("	  servtype: %d\n", p->xp_servtype);
		printf("\n");

		status = 0;
	}

	return(status);
}

DumpSO(p)
	register struct socket *p;
{
	register status = 1;

	if (p) {
		printf("\n");
		printf(" xtiso: socket = %x...\n", p);
		printf("           type: %d\n", p->so_type);
		printf("        options: %x\n", p->so_options);
		printf("         linger: %d\n", p->so_linger);
		printf("          state: %x\n", p->so_state);
		printf("            pcb: %x\n", p->so_pcb);
		printf("          proto: %x\n", p->so_proto);
		printf("         wakeup: %x\n", p->so_rcv.sb_wakeup);
		printf("        wakearg: %x\n", p->so_rcv.sb_wakearg);
		printf("\n");

		status = 0;
	}

	return(status);
}

mbufdsize(m)
	register struct mbuf *m;
{
	int count = 0;

	while (m) {
		if (m->m_type == MT_DATA || m->m_type == MT_HEADER)
			count += m->m_len;
		m = m->m_next;
       }

       return(count);
}


DumpMBLK(p)
	mblk_t *p;
{
	mblk_t *mp = p;
	int status = 0;

	if (mp) {
		int cnt = 0;

		do {
			cnt++;
			printf("\n");
			printf(" %d -> mblk_t = %x ", cnt, mp);
			if (cnt == 1)
				printf(" (msgdsize = %d)", msgdsize(mp));
			printf(" ...\n");
			printf("  msgb:\n");
			printf("      b_next: %x\n", mp->b_next);
			printf("      b_prev: %x\n", mp->b_prev);
			printf("      b_cont: %x\n", mp->b_cont);
			printf("      b_rptr: %x\n", mp->b_rptr);
			printf("      b_wptr: %x\n", mp->b_wptr);
			printf("     b_datap: %x\n", mp->b_datap);
			printf("  datab:\n");
			printf("          db_freep: %x\n", mp->b_datap->db_freep);
			printf("           db_base: %x\n", mp->b_datap->db_base);
			printf("            db_lim: %x\n", mp->b_datap->db_lim);
			printf("            db_ref: %d\n", mp->b_datap->db_ref);
			printf("           db_type: %d\n", mp->b_datap->db_type);
			printf("\n");

		} while (mp = mp->b_cont);

		status = 0;
	}

	return(status);
}
#endif	/* XTIDEBUG */

/*
 * TODO:
 *
 * . TS_UNINIT or T_UNINIT state - should these be defined to be truly
 *   XTI conformant?  The XTI documentation references the user space
 *   T_UNINITed state
 * . If an M_IOCTL request to NDELAY on STREAM, seems as though
 *   we should ACK it and then send a M_SETOPTS / SO_NDELOFF request
 *   back up to STREAM head to handle O_NDELAY processing up there...
 *
 * . Also see comments at "XXX" above.
 */

#ifndef XTI11
/*
 * Allocate/deallocate minor device.
 * I'm sure I can think of a better way to manage this,
 * but I'm not particularly inspired at the moment.
 */
xti_allocate_minor(xtip)
	struct xticb *xtip;
{
	struct xtisocfg *xc = xtip->xti_cfg;
	int i, device = xtip->xti_minor;
	int xc_devicearraysize = nfile >> 3; /* 8 devices per byte */

	simple_lock(&xc->xti_cfglock);
	if (device == NODEV) {
	        /* Loop should only go up to MAXOPENS/8 since each
		   byte represents eight devices. */

		for (device = 0; device < xc_devicearraysize; device++)
			if (xc->xti_cfgminor[device] != 0xff)	/* 8 bits */
				break;
		if (device == xc_devicearraysize) {
			simple_unlock(&xc->xti_cfglock);
			simple_lock(&xtisoLock);
			xc->xti_cfgnopen--;	/* Deref xtisocfg on error */
			simple_unlock(&xtisoLock);
			return NODEV;
		} else
			i = ffs(~xc->xti_cfgminor[device]) - 1;
	} else {
		i = device % 8;
		device /= 8;
	}
	xc->xti_cfgminor[device] |= (1 << i);
	device = device * 8 + i;
	xtip->xti_minor = device;
	simple_unlock(&xc->xti_cfglock);
	return (device);
}

xti_deallocate_minor(xtip)
	struct xticb *xtip;
{
	struct xtisocfg *xc;
	int m1, m2;

	if (!xtip)
		panic("xti_deallocate_minor: xtip NULL");

	m1 = xtip->xti_minor / 8;
	m2 = xtip->xti_minor % 8;

	if (m1 >= (nfile >> 3))
		panic("xti_deallocate_minor: bogus Minor #");

	xc = xtip->xti_cfg;
	simple_lock(&xc->xti_cfglock);
	if ((xc->xti_cfgminor[m1] & (1 << m2)) == 0)
		xti_panic("xti_deallocate_minor");
	xc->xti_cfgminor[m1] &= ~(1 << m2);
	simple_unlock(&xc->xti_cfglock);
	simple_lock(&xtisoLock);
	xc->xti_cfgnopen--;
	simple_unlock(&xtisoLock);
}
#endif
