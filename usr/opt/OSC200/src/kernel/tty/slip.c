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
static char *rcsid = "@(#)$RCSfile: slip.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:06:31 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <slip.h>
#include <sl.h>

#if	!(SLIP_DYNAMIC || (SLIP && NSL > 0))

/* SLIP needs NSL in order to link. */
#error Statically built SLIP requires NSL > 0 (pseudo-device sl).

#else

#include <sys/secdefines.h>
#if     SEC_BASE
#include <sys/security.h>
#endif

#include "sys/param.h"
#include "sys/mbuf.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"
#include "sys/ioctl.h"

#include "net/if.h"
#include "net/if_types.h"
#include "net/netisr.h"

#include "netinet/in.h"
#include "netinet/in_var.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"

#include "sys/stropts.h"
#include "sys/stream.h"
#include "net/slcompress.h"
#include "net/if_slvar.h"  

int sl_rput();
int sl_wput();
int sl_rsrv();
int sl_wsrv();
int sl_open();
int sl_close();

static struct module_info minfo = { SLIP_MODID, "slip", 0, INFPSZ, 
				SLIP_HIWAT, SLIP_LOWAT};

static struct qinit rinit = {
	sl_rput, sl_rsrv, sl_open, sl_close, NULL, &minfo, NULL
	};
static struct qinit winit = {
	sl_wput, sl_wsrv, NULL, NULL, NULL, &minfo, NULL
	};

struct streamtab slipinfo = { &rinit, &winit, NULL, NULL };

extern struct sl_softc sl_softc[];
static int if_str_output();

#define STRVERS		OSF_STREAMS_11
#define STRFLAGS	STR_IS_MODULE|STR_SYSV4_OPEN
#define STRSYNCL	SQLVL_QUEUE
#define STRNAME		"slip"
#define STRCONFIG	slip_configure
#define STRINFO		slipinfo

#include "streamsm/template.c"

int
sl_open(q, dev, flag, sflag, credp)
	queue_t *q;
	dev_t *dev;
	int flag;
	int sflag;
	cred_t *credp;
{
	int error;

	PRIV_SUSER(u.u_cred, &u.u_acflag, SEC_REMOTE, EPERM, EPERM, error);
	if (error)
		return error;
	/* No already-open check (for baudrate, etc) */
	return 0;
}

int 
sl_close(q,flag, credp)
	queue_t *q;
	int flag;
	cred_t *credp;
{
	struct sl_softc *sc = (struct sl_softc *)q->q_ptr;

	if (sc != NULL)
		sldinit(sc);
	q->q_ptr = NULL;
	return 0;
}

int
sl_wput(q,mp)
	queue_t *q;
	mblk_t *mp;
{
	struct iocblk *iop;
	struct sl_softc *sc;
	int unit, error;

	switch (mp->b_datap->db_type) {
	   case M_FLUSH:
		if (q->q_ptr) {
			freemsg(mp);
			return;
		}
		if (*mp->b_rptr & FLUSHW)
 			flushq(q, FLUSHDATA);
		break;
	   case M_IOCTL:
		iop = (struct iocblk *)mp->b_rptr;
		switch(iop->ioc_cmd) {
		   case SLIOGUNIT:
		   case SLIOCGFLAGS:
			if((sc = (struct sl_softc *)q->q_ptr) == NULL) {
				error = ENETDOWN;
				goto err;
			}
		    	if(!mp->b_cont &&
			   !(mp->b_cont = allocb(sizeof(int), BPRI_MED))) {
				error = ENOSR;
				goto err;
			}
			mp->b_datap->db_type = M_IOCACK;
			*((int *)(mp->b_cont->b_rptr)) =
				(iop->ioc_cmd == SLIOGUNIT) ?
					sc->sc_if.if_unit :
					sc->sc_flags;
			mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
			iop->ioc_count = sizeof(int);
			iop->ioc_error = 0;
			break;
		   case SLIOCSFLAGS:
			if (!mp->b_cont ||
			    mp->b_cont->b_wptr - mp->b_cont->b_rptr != sizeof (int)) {
				error = EINVAL;
				goto err;
			}
			if((sc = (struct sl_softc *)q->q_ptr) == NULL) {
				error = ENETDOWN;
				goto err;
			}
			SLIP_LOCK(sc);
			sc->sc_flags = (*(int *)mp->b_cont->b_rptr & SC_CANSET)|
				(sc->sc_flags & ~SC_CANSET);
			SLIP_UNLOCK(sc);
			mp->b_datap->db_type = M_IOCACK;
			iop->ioc_rval = iop->ioc_count = iop->ioc_error = 0;
			break;
		   case SLIOCSATTACH:
			if (!mp->b_cont ||
			    mp->b_cont->b_wptr - mp->b_cont->b_rptr != sizeof (int)) {
				error = EINVAL;
				goto err;
			}
			unit = *(int *)mp->b_cont->b_rptr;
			if ((error = funit( &unit)) == SC_FOUND) {
				sc = &sl_softc[unit];
				if (error = slinit(sc)) {
					SLIP_LOCK(sc);
					sc->sc_flags &= ~SC_MASK;
					SLIP_UNLOCK(sc);
					goto err;
				}
				SLIP_LOCK(sc);
				sc->sc_qptr = (void *)q;
				RD(q)->q_ptr = q->q_ptr = (caddr_t)sc;
				sc->sc_output = if_str_output;
				sc->sc_flags |= SC_COMPRESS;	/* default(s) */
				SLIP_UNLOCK(sc);
				mp->b_datap->db_type = M_IOCACK;
				*(int *)mp->b_cont->b_rptr = unit;
				iop->ioc_rval = iop->ioc_error = 0;
				iop->ioc_count = sizeof(int);
			} else {
err:
				iop->ioc_count = 0;
				iop->ioc_error = error;
				mp->b_datap->db_type = M_IOCNAK;
			}
			break;

		   default:
			putnext(q, mp);
			return;
		}
		qreply(q, mp);
		return;
	   case M_DATA:
		freemsg(mp);
		return;
	   default:
		break;
	}
	putnext(q, mp);
}

int
sl_wsrv(q)
	register queue_t *q;
{
	register mblk_t *mp;

	while ((mp = getq(q)) != NULL) {
		if (!bcanput(q->q_next, mp->b_band)) {
			putbq(q, mp);
			return;
		}
		putnext(q, mp);
	}
}

int
sl_rput(q,mp)
	queue_t *q;
	mblk_t *mp;
{
	switch (mp->b_datap->db_type) {
		case M_DATA:
			if (canput(q))
				putq(q, mp);
			else
				freemsg(mp);
			return;
		case M_FLUSH:
			if (q->q_ptr) {
				freemsg(mp);
				return;
			}
			if (*mp->b_rptr & FLUSHR)
				flushq(q, FLUSHDATA);
			break;
	}
	putnext(q, mp);
}

sl_rsrv(q)
	register queue_t *q;
{
	register mblk_t *mp,*bp;
	register u_char c, *cp;
	struct sl_softc *sc;
	struct mbuf *m;
	int len;

	if((sc = (struct sl_softc *)q->q_ptr) == 0) {
		flushq(q, FLUSHALL);
		return;
	}
	while ((mp = getq(q)) != NULL) {
	    for (bp = mp; bp != 0; bp = bp->b_cont) {
               	cp = bp->b_rptr;
               	while (cp < bp->b_wptr) {
                       	c = *cp++;
	 		switch (c) {

		   	case TRANS_FRAME_ESCAPE:
				if (sc->sc_escape)
					c = FRAME_ESCAPE;
				break;

		   	case TRANS_FRAME_END:
				if (sc->sc_escape)
					c = FRAME_END;
				break;

		   	case FRAME_ESCAPE:
				sc->sc_escape = 1;
				continue;

		   	case FRAME_END:
				if (sc->sc_mp >= sc->sc_ep)	/* overrun */
					goto error1;
				len = sc->sc_mp - sc->sc_buf;
				if (len < 3)
					goto newpack;
				SLIP_LOCK(sc);
				if ((c = (*sc->sc_buf & 0xf0)) != (IPVERSION << 4)) {
					if (c & 0x80)
						c = TYPE_COMPRESSED_TCP;
					else if (c == TYPE_UNCOMPRESSED_TCP)
						*sc->sc_buf &= 0x4f; /* XXX */
			/*
 			 * We've got something that's not an IP packet.
 			 * If compression is enabled, try to decompress it.
 			 * Otherwise, if `auto-enable' compression is on and
 			 * it's a reasonable packet, decompress it and then
 			 * enable compression.  Otherwise, drop it.
 			 */
					if (sc->sc_flags & SC_COMPRESS) {
						len = sl_uncompress_tcp(
							&sc->sc_buf,
							len, (int)c,
							&sc->sc_comp);
						if (len <= 0)
							goto error;
					} else if ((sc->sc_flags & SC_AUTOCOMP) &&
					    c == TYPE_UNCOMPRESSED_TCP && 
					    len >= 40) {
						len = sl_uncompress_tcp(
							&sc->sc_buf,
							len, (int)c,
							&sc->sc_comp);
						if (len <= 0)
							goto error;
						sc->sc_flags |= SC_COMPRESS;
					} else
						goto error;
				}
				if ((m = sl_btom(sc, len)) == NULL)
					goto error;
				SLIP_UNLOCK(sc);
				sc->sc_if.if_ipackets++;
				microtime(&sc->sc_if.if_lastchange);
				if (netisr_input(NETISR_IP, m, (caddr_t)0, 0))
					goto error1;
				goto newpack;
			}
			if (sc->sc_mp < sc->sc_ep)
				*sc->sc_mp++ = c;
			sc->sc_escape = 0;
			continue;
error:
			SLIP_UNLOCK(sc);
error1:
			sc->sc_if.if_ierrors++;
newpack:
			sc->sc_mp = sc->sc_buf = sc->sc_ep - SLMAX;
			sc->sc_escape = 0;
		}
	    }
	    freemsg(mp);
	}
}

static int
if_str_output(inter, m, sc)
	int inter;
	struct	mbuf *m;
	struct	sl_softc *sc;
{
	mblk_t	*mp;
	struct	mbuf	*bp, *n;
	u_char	*cp;
	int	cnt, ch, len, i;
	queue_t	*wrtq;

	if(! (wrtq = (queue_t *)sc->sc_qptr)) {
		m_freem(m);
		return ENETDOWN;
	}
	if(! bcanput(wrtq, inter)) {
		m_freem(m);
		++sc->sc_if.if_collisions;
		return 0;
	}
	len = 2;
	for (n = m; n != 0 ; n = n->m_next ) {
		cp = mtod(n, u_char *);
		cnt = n->m_len;
		len += cnt;
		for(i = 0; i<cnt; i++) {
			switch (*cp++) {
			case FRAME_ESCAPE:
			case FRAME_END:
				len++;
			}
		}
	}
	if ((mp = allocb(len, BPRI_MED)) == NULL) {
		m_freem(m);
 		return ENOSR;
	}
	*mp->b_wptr++ = FRAME_END;
	for (bp = m; bp != NULL; bp = bp->m_next) {
		cp = mtod(bp, u_char *);
		for (cnt = 0; cnt < bp->m_len; cnt++) {
			switch (ch = *cp++) {
			   case FRAME_END:
				*mp->b_wptr++ = FRAME_ESCAPE;
				*mp->b_wptr++ = TRANS_FRAME_END;
				break;
			   case FRAME_ESCAPE:
				*mp->b_wptr++ = FRAME_ESCAPE;
				*mp->b_wptr++ = TRANS_FRAME_ESCAPE;
				break;
			   default:
				*mp->b_wptr++ = ch;
			}
		}
	}
	m_freem(m);
	*mp->b_wptr++ = FRAME_END;
	mp->b_band = inter;
	putq(wrtq, mp);
	return 0;
}

int
funit(i)
	int *i;
{
	struct sl_softc *sc;
	struct ifnet *ifp;
	int mode;
	char s[64];

	mode = 1;
	if (*i < 0)
		*i = mode = 0;
	while (1) {
		sprintf(s, "%s%d", SLIFNAME, *i);
		sc = &sl_softc[*i];
		if ( !(ifp = ifunit(s)) || (ifp != &sc->sc_if)) 
			return mode ? ENXIO : EBUSY;
		SLIP_LOCK(sc);
		if (sc->sc_flags & SC_INUSE) {
			SLIP_UNLOCK(sc);
			if (mode)
				return (EALREADY);
		} else {
			sc->sc_flags = SC_INUSE;
			SLIP_UNLOCK(sc);
			return SC_FOUND;
		}
		*i++;
	}
}
#endif
