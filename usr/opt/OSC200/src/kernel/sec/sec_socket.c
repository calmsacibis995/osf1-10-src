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
static char *rcsid = "@(#)$RCSfile: sec_socket.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1992/06/03 14:51:02 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989-1990 SecureWare, Inc.  All rights reserved.
 *
 * Support functions for extensions to socket rights passing mechanism.
 */
/*

*/
/*
 * Based on:

 */

#include <sec/include_sec>

#if SEC_SOCKET /*{*/
#if SEC_ARCH

#include "sys/mbuf.h"
#include "sys/socket.h"

#define MBUF_HAS_RIGHTS(m) ( \
	m->m_type == MT_CONTROL && m->m_len >= sizeof (struct cmsghdr) && \
	mtod(m, struct cmsghdr *)->cmsg_level == SOL_SOCKET && \
	mtod(m, struct cmsghdr *)->cmsg_type  == SCM_RIGHTS && \
	m->m_len >= mtod(m, struct cmsghdr *)->cmsg_len /*validity*/ \
)


/*
 * Convert access rights as passed in from user space by a sendmsg sys call
 * into internal format.  This routine checks appropriate privileges and
 * supplies default values for those attributes that either are unspecified
 * or cannot be specified by the process for lack of privilege.
 */
sec_internalize_rights(mp)
	struct mbuf **mp;
{
	register struct mbuf *mfrom = *mp;
	register struct mbuf *mto;
	register ushort *rp, *wp;
	register struct cmsghdr *cmto, *cmfrom = 0;
	register ushort rightstype, rightslen;
	register int error, i;
	int haveprivs = 0, didone = 0;

	if (!security_is_on)
		return 0;

	if ((mto = m_get(M_WAIT, MT_CONTROL)) == NULL)
		return ENOBUFS;
	
	cmto = mtod(mto, struct cmsghdr *);
	mto->m_len = cmto->cmsg_len = sizeof (*cmto);
	cmto->cmsg_level = SOL_SOCKET;
	cmto->cmsg_type = SCM_RIGHTS;
	wp = (ushort *)(cmto + 1);

	if (mfrom == NULL)
		goto setdefaults;
	if (!MBUF_HAS_RIGHTS(mfrom))
		goto inval;
	cmfrom = mtod(mfrom, struct cmsghdr *);
	mfrom->m_data += sizeof (*cmfrom);
	mfrom->m_len  -= sizeof (*cmfrom);
	cmfrom->cmsg_len -= sizeof (*cmfrom);
	while (cmfrom->cmsg_len > 0) {
		rp = mtod(mfrom, ushort *);
		rightstype = *rp++;	/* get the type indicator */
		rightslen  = *rp++;	/* and length */

		switch (rightstype) {
		    default:	/* assume old style rights */
			if (didone)
				goto inval;
			rightslen = cmfrom->cmsg_len;
			rp -= 2;	/* backup to start of array */
			/* fall through */

		    case SEC_RIGHTS_FDS:	/* file descriptors */
			/* Check for sufficient space in destination mbuf,
			 * and verify that an integral number of file
			 * descriptors has been specified.
			 */
			i = 2 * sizeof(ushort) + rightslen;
			cmto->cmsg_len += i;
			mto->m_len += i;
			if (mto->m_len > MLEN || (rightslen % sizeof(int)) != 0)
				goto inval;
			/* Passing file descriptors circumvents the access
			 * checks that would normally be done by an open
			 * in the receiving process.  We therefore require
			 * that the passing process be privileged to
			 * override these access checks.
			 */
			if (
#if SEC_MAC
			    !privileged(SEC_ALLOWMACACCESS, EPERM) ||
#endif
			    !privileged(SEC_ALLOWDACACCESS, EPERM)) {
				error = EPERM;
				goto err;
			}
			*wp++ = SEC_RIGHTS_FDS;
			*wp++ = rightslen;
			bcopy(rp, wp, rightslen);
			wp += rightslen / sizeof(ushort);
			break;

		    case SEC_RIGHTS_PRIVS:
			i = 2 * sizeof(ushort) + sizeof(privvec_t);
			cmto->cmsg_len += i;
			mto->m_len += i;
			if (mto->m_len > MLEN)
				goto inval;

			/* Only those privileges that the process can enable
			 * can be passed with the message (base + potential).
                         * MP Note:  Locking here is somewhat pointless,
                         * but at least we'll get a consistent set of
                         * privileges as of some instant in time.
			 */
			*wp++ = SEC_RIGHTS_PRIVS;
			*wp++ = sizeof(privvec_t);
			SIP_ATTR_LOCK();
			for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
				((mask_t *) wp)[i] = ((mask_t *) rp)[i] &
					(SIP->si_bpriv[i] | SIP->si_spriv[i]);
			SIP_ATTR_UNLOCK();
			wp += sizeof(privvec_t) / sizeof(ushort);
			haveprivs = 1;
			break;
		}

		/* make sure that each item starts at an even byte */
		rightslen = (rightslen + 1) & ~1;
		didone++;
		i = rightslen + 2 * sizeof(ushort);
		mfrom->m_data += i;
		mfrom->m_len  -= i;
		cmfrom->cmsg_len -= i;
	}
setdefaults:
	/* supply default attributes for those not already specified */

	if (!haveprivs) {
		i =  2 * sizeof(ushort) + sizeof(privvec_t);
		cmto->cmsg_len += i;
		mto->m_len += i;
		if (mto->m_len > MLEN)
			goto inval;
		*wp++ = SEC_RIGHTS_PRIVS;
		*wp++ = sizeof(privvec_t);
		SIP_ATTR_LOCK();
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			((mask_t *) wp)[i] = SIP->si_epriv[i];
		SIP_ATTR_UNLOCK();
		wp += sizeof(privvec_t) / sizeof(ushort);
	}

	if (mfrom && mfrom->m_len <= 0)
		mto->m_next = m_free(mfrom);
	else
		mto->m_next = mfrom;
	*mp = mto;
	return 0;

inval:
	error = EINVAL;
err:
	/* Trimming off the previous rights isn't necessary given the
	 * current callers (who just free them), but it's safer. */
	if (mfrom && cmfrom) {
		mfrom->m_data += cmfrom->cmsg_len;
		mfrom->m_len  -= cmfrom->cmsg_len;
		if (mfrom->m_len <= 0)
			mfrom = m_free(mfrom);
	}
	*mp = mfrom;
	(void)m_free(mto);
	return error;
}

/*
 * Convert the internal form of access rights to external form as should
 * be returned by the recvmsg sys call.  If the socket option enabling
 * expanded rights is not turned on, discard all but file descriptors
 * and put them in a backward compatible format.
 */
sec_externalize_rights(mp, so)
	struct mbuf **mp;
	struct socket *so;
{
	register struct mbuf *mfrom = *mp;
	register struct mbuf *mto;
	register ushort *rp, *wp;
	register struct cmsghdr *cmto, *cmfrom = 0;
	register ushort rightstype, rightslen;
	register int expanded_rights = (so->so_options & SO_EXPANDED_RIGHTS);
	int ir_size;

	if (!security_is_on)
		return 0;

	if ((mto = m_get(M_WAIT, MT_CONTROL)) == NULL) {
		if (mfrom && MBUF_HAS_RIGHTS(mfrom)) {
			/* See code at err: */
			cmfrom = mtod(mfrom, struct cmsghdr *);
			mfrom->m_data += sizeof (*cmfrom);
			mfrom->m_len  -= sizeof (*cmfrom);
			cmfrom->cmsg_len -= sizeof (*cmfrom);
		}
		goto err;
	}
	cmto = mtod(mto, struct cmsghdr *);
	mto->m_len = cmto->cmsg_len = sizeof (*cmto);
	cmto->cmsg_level = SOL_SOCKET;
	cmto->cmsg_type = SCM_RIGHTS;
	wp = (ushort *)(cmto + 1);

	if (mfrom == NULL)
		goto out;		/* ??? with empty rights ??? */
	if (!MBUF_HAS_RIGHTS(mfrom))
		goto err;
	cmfrom = mtod(mfrom, struct cmsghdr *);
	mfrom->m_data += sizeof (*cmfrom);
	mfrom->m_len  -= sizeof (*cmfrom);
	cmfrom->cmsg_len -= sizeof (*cmfrom);
	while (cmfrom->cmsg_len > 0) {
		rp = mtod(mfrom, ushort *);
		rightstype = *rp++;	/* get the type indicator */
		rightslen = *rp++;	/* and length */

		switch (rightstype) {
		    case SEC_RIGHTS_FDS:	/* file descriptors */
			cmto->cmsg_len += rightslen;
			mto->m_len += rightslen;
			if (expanded_rights) {
				cmto->cmsg_len += 2 * sizeof(ushort);
				mto->m_len += 2 * sizeof(ushort);
			}
			if (mto->m_len > MLEN)
				goto err;
			if (expanded_rights) {
				*wp++ = SEC_RIGHTS_FDS;
				*wp++ = rightslen;
			}
			bcopy(rp, wp, rightslen);
			wp += rightslen / sizeof(ushort);
			break;

		    case SEC_RIGHTS_PRIVS:
			if (!expanded_rights)
				break;
			cmto->cmsg_len += 2 * sizeof(ushort) + rightslen;
			mto->m_len += 2 * sizeof(ushort) + rightslen;
			if (mto->m_len > MLEN)
				goto err;
			*wp++ = SEC_RIGHTS_PRIVS;
			*wp++ = rightslen;
			bcopy(rp, wp, rightslen);
			wp += rightslen / sizeof(ushort);
			break;

		}
		mfrom->m_data += rightslen + 2 * sizeof(ushort);
		mfrom->m_len  -= rightslen + 2 * sizeof(ushort);
		cmfrom->cmsg_len -= rightslen + 2 * sizeof(ushort);
	}
out:
	if (mfrom && mfrom->m_len <= 0)
		mto->m_next = m_free(mfrom);
	else
		mto->m_next = mfrom;
	*mp = mto;
	return;

err:
	/* Trim off old rights before returning when fail */
	if (mfrom && cmfrom) {
		mfrom->m_data += cmfrom->cmsg_len;
		mfrom->m_len  -= cmfrom->cmsg_len;
		if (mfrom->m_len <= 0)
			mfrom = m_free(mfrom);
	}
	*mp = mfrom;
	if (mto)
		m_freem(mto);
}

/*
 * Scan a rights buffer for the specified type.  If found, return a pointer
 * to the associated data and return the associated length through the prlen
 * argument.  Return NULL if the buffer doesn't contain the specified type.
 */
caddr_t
findrights(rp, len, type, prlen)
	register ushort *rp;
	register int len;
	ushort type;
	int *prlen;
{
	register int itemlength;

	for (;;) {
		itemlength = 2 * sizeof(ushort) + rp[1];
		if (len < itemlength)
			break;
		if (rp[0] == type) {
			*prlen = (int) rp[1];
			return (caddr_t) &rp[2];
		}
		/* round up to an even number of bytes */
		itemlength = (itemlength + 1) & ~1;
		len -= itemlength;
		rp = (ushort *) ((int) rp + itemlength);
	}
	return NULL;
}

/*
 * Count the readable bytes in the specified socket buffer, ignoring
 * any access rights and address mbufs.
 */

sec_sobufcount(sb, so)
	register struct sockbuf	*sb;
	register struct socket *so;
{
	register int	count = 0;
	register struct mbuf	*m, *m0;

	if (sosblock(sb, so) == 0) {
		for (m0 = sb->sb_mb; m0; m0 = m0->m_nextpkt)
			for (m = m0; m; m = m->m_next)
				if (m->m_type != MT_SONAME)	/* XXX? */
					if (!MBUF_HAS_RIGHTS(m))
						count += m->m_len;
		sbunlock(sb);
	}
	return count;
}


#endif /* SEC_ARCH */
#endif /*}*/
