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
#ifndef	lint
static char *sccsid = "@(#)$RCSfile: dli_mopctrs.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/22 22:20:53 $";
#endif

/*
 *   COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1991.  ALL
 *   RIGHTS RESERVED.
 *
 *   THIS  SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE
 *   USED  AND  COPIED ONLY IN ACCORDANCE WITH THE TERMS OF
 *   SUCH  LICENSE  AND  WITH  THE  INCLUSION  OF THE ABOVE
 *   COPYRIGHT  NOTICE.   THIS SOFTWARE OR ANY OTHER COPIES
 *   THEREOF   MAY   NOT  BE  PROVIDED  OR  OTHERWISE  MADE
 *   AVAILABLE  TO  ANY  OTHER  PERSON.   NO  TITLE  TO AND
 *   OWNERSHIP OF THE SOFTWARE IS HEREBY TRANSFERRED.
 *
 *   THE  INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE
 *   WITHOUT  NOTICE  AND  SHOULD  NOT  BE  CONSTRUED  AS A
 *   COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 *
 *   DIGITAL  ASSUMES  NO  RESPONSIBILITY  FOR  THE  USE OR
 *   RELIABILITY  OF  ITS SOFTWARE ON EQUIPMENT THAT IS NOT
 *   SUPPLIED BY DIGITAL.
 *
 *
 *   Telecommunications & Networks
 *
 * IDENT HISTORY:
 *
 * 1.00 22-May-1986
 *      DECnet-ULTRIX   V2.0
 *
 * 2.01 18-Mar-1988
 *      DECnet-ULTRIX   V2.4
 *		- Allowed use of reserved bit in individual and group SAPs
 *
 *      DECtrade-ULTRIX V1.0
 *		- Added mcast buffer array size (in MCAST_SIZE units) in
 *		  match_mcast routine. Comparison is done based on this
 *		  size and *NOT* on MCAST_ASIZE ( as was done previously )
 */

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <kern/assert.h>
#include <net/if.h>
#include <dli/dli_var.h>

#if defined(__alpha)
#define LE_PUT64A(p, v)	  (*(p)++ = (v))
#define LE_PUT128A(p, v)  (*(p)++ = (v), *(p)++ = 0)
#elif (defined(__mips__) && defined(__MIPSEL__)) || defined(vax)
#define LE_PUT64A(p, v)	  (*(p)++ = (v), *(p)++ = 0)
#define LE_PUT128A(p, v)  (*(p)++ = (v), *(p)++ = 0, *(p)++ = 0, *(p)++ = 0)
#endif

struct mbuf *
dli_get_ether_ctrs(
    struct dli_ifnet *dlif,
    u_int flags)
{
    struct mbuf *m;
    struct ctrreq ctrs;
    int blklen = (flags & DLI_FILTER_ETYPE) ? 54 : 200;

    if (dli_getmopctrs(dlif, &ctrs) != 0)
	return NULL;

    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL)
	return;
    if (blklen + dlif->dlif_hdrlen + 8 > MHLEN) {
	MCLGET(m,M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
	    m_freem(m);
	    return NULL;
	}
	m->m_data += MCLBYTES & ~(sizeof(u_long) - 1);
    } else {
	m->m_data += MHLEN & ~(sizeof(u_long) - 1);
    }

    m->m_pkthdr.len = m->m_len = blklen;
    m->m_data -= m->m_len;

    if (flags & DLI_FILTER_ETYPE) {
	u_char *dp = mtod(m, u_char *);

	/* ethernet device  - format counters */
	LE_PUT16A(dp, ctrs.ctr_ether.est_seconds);
	LE_PUT32A(dp, ctrs.ctr_ether.est_bytercvd);

	LE_PUT32A(dp, ctrs.ctr_ether.est_bytesent);
	LE_PUT32A(dp, ctrs.ctr_ether.est_blokrcvd);
	LE_PUT32A(dp, ctrs.ctr_ether.est_bloksent);
	LE_PUT32A(dp, ctrs.ctr_ether.est_mbytercvd);

	LE_PUT32A(dp, ctrs.ctr_ether.est_mblokrcvd);
	LE_PUT32A(dp, ctrs.ctr_ether.est_deferred);
	LE_PUT32A(dp, ctrs.ctr_ether.est_single);
	LE_PUT32A(dp, ctrs.ctr_ether.est_multiple);

	LE_PUT16A(dp, ctrs.ctr_ether.est_sendfail);
	LE_PUT16A(dp, ctrs.ctr_ether.est_sendfail_bm);
	LE_PUT16A(dp, ctrs.ctr_ether.est_recvfail);
	LE_PUT16A(dp, ctrs.ctr_ether.est_recvfail_bm);

	LE_PUT16A(dp, ctrs.ctr_ether.est_unrecog);
	LE_PUT16A(dp, ctrs.ctr_ether.est_overrun);
	LE_PUT16A(dp, ctrs.ctr_ether.est_sysbuf);
	LE_PUT16A(dp, ctrs.ctr_ether.est_userbuf);
    } else {
	u_long *dp = mtod(m, u_long *);

	LE_PUT128A(dp, 0);
	LE_PUT64A(dp, ctrs.ctr_ether.est_bytercvd);
	LE_PUT64A(dp, ctrs.ctr_ether.est_bytesent);

	LE_PUT64A(dp, ctrs.ctr_ether.est_blokrcvd);
	LE_PUT64A(dp, ctrs.ctr_ether.est_bloksent);
	LE_PUT64A(dp, ctrs.ctr_ether.est_mbytercvd);
	LE_PUT64A(dp, ctrs.ctr_ether.est_mblokrcvd);

	LE_PUT64A(dp, ctrs.ctr_ether.est_deferred);
	LE_PUT64A(dp, ctrs.ctr_ether.est_single);
	LE_PUT64A(dp, ctrs.ctr_ether.est_multiple);
	LE_PUT64A(dp, 0);	/* Send failure, excessive collisions */

	LE_PUT64A(dp, 0);	/* Send failure, carrier check failed */
	LE_PUT64A(dp, 0);	/* Send failure, short circuit */
	LE_PUT64A(dp, 0);	/* Send failure, open circuit */
	LE_PUT64A(dp, 0);	/* Send failure, frame too long */

	LE_PUT64A(dp, 0);	/* Send failure, remote failure to defer */
	LE_PUT64A(dp, 0);	/* Receive failure, block check error */
	LE_PUT64A(dp, 0);	/* Receive failure, framing error */
	LE_PUT64A(dp, 0);	/* Receive failure, frame too long */

	LE_PUT64A(dp, ctrs.ctr_ether.est_unrecog);
	LE_PUT64A(dp, ctrs.ctr_ether.est_overrun);
	LE_PUT64A(dp, ctrs.ctr_ether.est_sysbuf);
	LE_PUT64A(dp, ctrs.ctr_ether.est_userbuf);

	LE_PUT64A(dp, 0);	/* Collision detect check failure */
    }
    return m;
}

struct mbuf *
dli_get_fddi_ctrs(
    struct dli_ifnet *dlif,
    u_int flags)
{
    struct mbuf *m;
    struct ctrreq ctrs;
    int blklen = 264;
    u_long *dp;

    if (flags & DLI_FILTER_ETYPE)
        return NULL;

    if (dli_getmopctrs(dlif, &ctrs) != 0)
	return NULL;

    MGETHDR(m, M_DONTWAIT, MT_DATA);
    if (m == NULL)
	return;
    if (blklen + dlif->dlif_hdrlen + 8 > MHLEN) {
	MCLGET(m,M_DONTWAIT);
	if ((m->m_flags & M_EXT) == 0) {
	    m_freem(m);
	    return NULL;
	}
	m->m_data += MCLBYTES & ~(sizeof(u_long) - 1);
    } else {
	m->m_data += MHLEN & ~(sizeof(u_long) - 1);
    }

    m->m_pkthdr.len = m->m_len = blklen;
    m->m_data -= m->m_len;
    dp = mtod(m, u_long *);

    LE_PUT128A(dp, 0);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_bytercvd);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_bytesent);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_pdurcvd);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_pdusent);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_mbytercvd);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_mbytesent);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_mpdurcvd);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_mpdusent);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_underrun);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_sendfail);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_fcserror);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_fseerror);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_pdualig);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_pdulen);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_pduunrecog);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_mpduunrecog);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_overrun);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_sysbuf);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_userbuf);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_frame);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_error);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_lost);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_ringinit);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_ringinitrcv);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_dupaddfail);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_duptoken);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_ringpurge);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_bridgestrip);
    LE_PUT64A(dp, ctrs.ctr_fddi.fst_traceinit);
    LE_PUT64A(dp, 0);	/* Datalink selftest errors */

    return m;
}

/*
 *		d l i _ p r o c _ r e q  c t r s
 *
 * This routine is called to process requests for data link counters 
 *
 * Outputs:		None.
 *
 * Inputs:		pointer to mbuf chain from driver
 *              	pointer to dli_recv structure.
 *
 * Version History:
 * 1.0	JA
 *
 */
void
dli_snd_mopctrs(
    struct dli_ifnet *dlif,
    struct dli_filter *filter,
    u_int receipt,
    const union dli_addr *dst)
{
    struct mbuf *m = NULL;
    register u_char *msg, *bmsg;

    if (dlimgtsw != NULL) {
	m = (*dlimgtsw->mgt_get_dlctrs)(dlif, filter->fltr_flags);
    } else if (dlif->dlif_iftype == IFT_ETHER) {
	m = dli_get_ether_ctrs(dlif, filter->fltr_flags);
    } else if (dlif->dlif_iftype == IFT_FDDI) {
	m = dli_get_fddi_ctrs(dlif, filter->fltr_flags);
    }
    if (m == NULL)
	return;

    bmsg = msg = mtod(m, u_char *);
    bmsg -= sizeof(u_short);
    LE_INS16(bmsg, receipt);
    if (dlif->dlif_iftype == IFT_ETHER || dlif->dlif_iftype == IFT_ISO88023)
	*--bmsg = DLI_CTRS;
    else if (dlif->dlif_iftype == IFT_FDDI) {
	*--bmsg = DLI_DLFDDI;
	*--bmsg = DLI_CTRS2;
    } else {
	m_freem(m);
	return;
    }

    m->m_data -= (msg - bmsg);
    m->m_len += (msg - bmsg);
    m->m_pkthdr.len += (msg - bmsg);

    if (dli_add_header(dlif, &m, &filter->fltr_llcdata, filter->fltr_flags) != 0)
	return;
    dli_output(dlif, filter, m, dst);
}
