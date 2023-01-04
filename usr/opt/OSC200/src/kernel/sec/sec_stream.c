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
static char *rcsid = "@(#)$RCSfile: sec_stream.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1992/06/03 14:53:10 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */
/*
#ident "%W% %U% %G% SecureWare"
*/
#include <sec/include_sec>

#if SEC_BASE

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sec_stream.h>

#if !SEC_ARCH
#define SEC_EXTENDED_RIGHTS 0xabcd
#endif

#define	ROUND_UP(v,t) (((unsigned long)(v) + sizeof(t) - 1) & ~(sizeof(t) - 1))
#define	ROUND_DOWN(v,t) ((unsigned long)(v) & ~(sizeof(t) - 1))

int
str_copyout_attrs(attr, uattr)
	mblk_t		*attr;
	struct strbuf	*uattr;
{
	register struct tnet_sec_attr	*uap;
	register struct str_sec_attr	*sap;
	register mblk_t			*ubp;
	register int			error;
	
	sap = (struct str_sec_attr *) attr->b_rptr;

	/*
	 * If there is no user buffer we have nothing to do
	 */
	if (uattr == NULL || uattr->maxlen <= 0)
		return 0;

	/*
	 * Allocate a temporary buffer to hold converted attributes
	 */
	if ((ubp = allocb(uattr->maxlen, BPRI_LO)) == NULL)
		return EAGAIN;
	uap = (struct tnet_sec_attr *) ubp->b_rptr;

	/*
	 * Convert attributes into external format
	 */
	error = sec_externalize_attr(sap, uap, uattr->maxlen);
	if (error) {
		freeb(ubp);
		return error;
	}

	/*
	 * Copy converted attributes into user's buffer
	 */
	uattr->len = uap->length;
	error = copyout(uap, uattr->buf, uattr->len);
	freeb(ubp);

	return error;
}


/* sec_externalize_attr()
 *
 * This routine converts a set of security attributes from their local
 * representation format into internal representation for presentation to
 * a user level process.  
 *
 * Upon input, src_attr points to a str_sec_attr structure containing a full
 * set of security attributes.  dst_attr points to a buffer in which to store
 * the converted attributes.  max_length is the length of the the destination
 * buffer.
 *
 * The data stored in the tnet_sec_attr structure is position dependent, so
 * the order of the tests is important.
 */ 
int
sec_externalize_attr(src_attr, dest_attr, max_length)
	register struct str_sec_attr	*src_attr;
	register struct tnet_sec_attr	*dest_attr;
	int    max_length;
{
	register ulong	*out_ptr;
	int	size, i;
	int	error = 0;

	max_length = (int) ROUND_DOWN(max_length, long);
	if (max_length < sizeof(struct tnet_sec_attr))
		return EMSGSIZE;

	dest_attr->attr_type = SEC_EXTENDED_RIGHTS;
	dest_attr->length = sizeof(struct tnet_sec_attr);
	dest_attr->mask = 0;
	out_ptr = (ulong *) &dest_attr[1];

	/* 
	 * the order of the tests is important ! 
	 */

#if SEC_MAC
	if (src_attr->mask & TNET_SW_SEN_LABEL) {
		dest_attr->length += sizeof *out_ptr;
		size = max_length - dest_attr->length;
		if (error = macilb_tagtoir(&src_attr->tags[macobjtag], 
						&out_ptr[1], &size)) 
			return error;
		size = (int) ROUND_UP(size, long);
		dest_attr->length += size;
		*out_ptr++ = size;
		out_ptr = (ulong *) ((caddr_t) out_ptr + size);
		dest_attr->mask |= TNET_SW_SEN_LABEL;
	}
#endif

#if SEC_NCAV
	/*
	 * CAVEATS STUBBED OUT!!!!
	 *
	 * In order to get this to work, you will need to write the 
 	 * ncav equivalents for macilb_tagtoir() and macilb_irtotag()
	 * and add them to /kernel/sec/spd_ncav.c
	 */
	if (src_attr->mask & TNET_SW_NAT_CAVEATS) {
		dest_attr->length += sizeof *out_ptr;
		size = max_length - dest_attr->length;
		if (error = ncav_tagtoir(&src_attr->tags[ncavobjtag], 
						&out_ptr[1], &size)) 
			return error;
		size = (int) ROUND_UP(size, long);
		dest_attr->length += size;
		*out_ptr++ = size;
		out_ptr = (ulong *) ((caddr_t) out_ptr + size);
		dest_attr->mask |= TNET_SW_NAT_CAVEATS;
	}
#endif

#if SEC_ILB
	if (src_attr->mask & TNET_SW_INFO_LABEL) {
		dest_attr->length += sizeof *out_ptr;
		size = max_length - dest_attr->length;
		if (error = macilb_tagtoir(&src_attr->tags[ilbobjtag], 
						&out_ptr[1], &size)) 
			return error;
		size = (int) ROUND_UP(size, long);
		dest_attr->length += size;
		*out_ptr++ = size;
		out_ptr = (ulong *) ((caddr_t) out_ptr + size);
		dest_attr->mask |= TNET_SW_INFO_LABEL;
	}
#endif

	if (src_attr->mask & TNET_SW_PRIVILEGES) {
		dest_attr->length += sizeof(src_attr->privs);
		if (dest_attr->length > max_length)
			return EMSGSIZE;
		for (i=0; i<SEC_SPRIVVEC_SIZE; ++i)
			*out_ptr++ = src_attr->privs[i];
		dest_attr->mask |= TNET_SW_PRIVILEGES;
	}

	if (src_attr->mask & TNET_SW_LUID) {
		dest_attr->length += sizeof(ulong);
		if (dest_attr->length > max_length)
			return EMSGSIZE;
		*(uid_t *) out_ptr = src_attr->luid;
		out_ptr++;
		dest_attr->mask |= TNET_SW_LUID;
	}

	if (src_attr->mask & TNET_SW_IDS) {
		size = sizeof(uid_t) + sizeof(gid_t);
#if SEC_GROUPS
		size += sizeof(ulong) + src_attr->g_len * sizeof(gid_t);
#endif
		dest_attr->length += ROUND_UP(size, long);
		if (dest_attr->length > max_length)
			return EMSGSIZE;
		bcopy(&src_attr->uid, out_ptr, size);
		out_ptr = (ulong *) ((caddr_t) out_ptr + ROUND_UP(size, long));
		dest_attr->mask |= TNET_SW_IDS;
	}

	if (src_attr->mask & TNET_SW_PID) {
		dest_attr->length += sizeof(ulong);
		if (dest_attr->length > max_length)
			return EMSGSIZE;
		*(pid_t *)out_ptr = src_attr->pid;
		++out_ptr;
		dest_attr->mask |= TNET_SW_PID;
	}

	return 0;
}


/*
 * str_copyin_attrs
 *
 * Copy security attributes from the user-supplied buffer into their
 * internal format as a struct str_sec_attr.
 */
mblk_t *
str_copyin_attrs(uattr, errp)
	struct strbuf	*uattr;
	int		*errp;
{
	register struct str_sec_attr	*sap;
	register struct tnet_sec_attr	*uap;
	register mblk_t			*sbp, *ubp;
	register int			error;

	/*
	 * Copy in the user-specified attributes if specified
	 */
	if (uattr && uattr->len > 0) {
		if ((ubp = allocb(uattr->len, BPRI_LO)) == NULL) {
			*errp = EAGAIN;
			return NULL;
		}
		error = copyin(uattr->buf, ubp->b_wptr, uattr->len);
		if (error) {
			freeb(ubp);
			*errp = error;
			return NULL;
		}
		ubp->b_wptr += uattr->len;
		uap = (struct tnet_sec_attr *) ubp->b_rptr;
		if (uap->attr_type != SEC_EXTENDED_RIGHTS ||
		    uap->length != uattr->len) {
			freeb(ubp);
			*errp = EINVAL;
			return NULL;
		}
	} else {
		uap = NULL;
		ubp = NULL;
	}

	/*
	 * Allocate a buffer for the converted attributes
	 */
	if (!(sbp = allocb(sizeof(struct str_sec_attr), BPRI_LO))) {
		if (ubp)
			freeb(ubp);
		*errp = EAGAIN;
		return NULL;
	}
	sbp->b_wptr += sizeof(struct str_sec_attr);
	sap = (struct str_sec_attr *) sbp->b_rptr;

	/*
	 * Convert the user-supplied attributes and fill in unspecified
	 * attributes from the current process.
	 */
	error = sec_internalize_attr(uap, sap);
	if (ubp)
		freeb(ubp);
	if (error) {
		freeb(sbp);
		sbp = NULL;
		*errp = error;
	}
		
	return sbp;
}


/*
 * sec_in_user_attr()
 *
 * The user has passed in a set of security attributes to be associated with
 * outgoing data.  Convert those attributes from internal representation into
 * local representation, and check the user for the appropriate override
 * attributes.
 *
 * The user supplied attributes are in a tnet_sec_attr structure, which is
 * a variable length structure with attribute positions determined by the
 * bit mask.  Output goes into a str_sec_attr structure.
 *
 * Returns 0      - OK
 *         EPERM  - User lacked an override privilege.
 */
static int
sec_in_user_attr(src_attr, dest_attr)
	register struct tnet_sec_attr	*src_attr;
	register struct str_sec_attr	*dest_attr;
{
	register ulong	*in_ptr = (ulong *) &src_attr[1];
	register long	size;
	int	 i;
	int      error = 0;

	dest_attr->mask = 0;

	if (src_attr->mask == 0)
		return(0);

	/* 
 	 * the order of the tests is important ! 
 	 */

#if SEC_MAC
	if (src_attr->mask & TNET_SW_SEN_LABEL) {
		if (!privileged(SEC_ALLOWMACACCESS, 0)) 
			return(EPERM);
		size = *in_ptr++;
		if (error = macilb_irtotag(in_ptr, size,
				&dest_attr->tags[macobjtag]))
			return(error);
		size = ROUND_UP(size, long);
		in_ptr = (ulong *) ((caddr_t) in_ptr + size);
		dest_attr->mask |= TNET_SW_SEN_LABEL;
	}
#endif
	
#if SEC_NCAV
	/*
 	* CAVEATS STUBBED OUT!!!!
 	*
 	* In order to get this to work, you will need to write the 
 	* ncav equivalents for macilb_tagtoir() and macilb_irtotag()
 	* and add them to /kernel/sec/spd_ncav.c
 	*/
	if (src_attr->mask & TNET_SW_NAT_CAVEATS) {
		if (!privileged(SEC_ALLOWNCAVACCESS, 0)) 
			return(EPERM);
		size = *in_ptr++;
		if (error = ncav_irtotag(in_ptr, size,
				&dest_attr->tags[ncavobjtag]))
			return(error);
		size = ROUND_UP(size, long);
		in_ptr = (ulong *) ((caddr_t) in_ptr + size);
		dest_attr->mask |= TNET_SW_NAT_CAVEATS;
	}
#endif
	
#if SEC_ILB
	if (src_attr->mask & TNET_SW_INFO_LABEL) {
		if (!privileged(SEC_ALLOWILBACCESS, 0)) 
			return(EPERM);
		size = *in_ptr++;
		if (error = macilb_irtotag(in_ptr, size,
				&dest_attr->tags[ilbobjtag]))
			return(error);
		size = ROUND_UP(size, long);
		in_ptr = (ulong *) ((caddr_t) in_ptr + size);
		dest_attr->mask |= TNET_SW_INFO_LABEL;
	}
#endif
	
	if (src_attr->mask & TNET_SW_PRIVILEGES) {
		for (i=0; i<SEC_SPRIVVEC_SIZE; ++i)
			dest_attr->privs[i] = *in_ptr++ &
				(SIP->si_bpriv[i] | SIP->si_spriv[i]);
		dest_attr->mask |= TNET_SW_PRIVILEGES;
	}

	if (src_attr->mask & TNET_SW_LUID) {
		if (!privileged(SEC_SETPROCIDENT, 0))
			return(EPERM);
		dest_attr->luid  = *(uid_t *) in_ptr;
		in_ptr++;
		dest_attr->mask |= TNET_SW_LUID;
	}

	if (src_attr->mask & TNET_SW_IDS) {
		register struct ids {
			uid_t	uid;
			gid_t	gid;
#if SEC_GROUPS
			ulong	gcnt;
#endif
		} *idp = (struct ids *) in_ptr;

		if (!privileged(SEC_SETPROCIDENT, 0)) 
			return(EPERM);
		size = sizeof *idp;
#if SEC_GROUPS
		size += idp->gcnt * sizeof(gid_t);
#endif
		bcopy(in_ptr, &dest_attr->uid, size);
		in_ptr = (ulong *) ((caddr_t) in_ptr + ROUND_UP(size, long));
		dest_attr->mask |= TNET_SW_IDS;
	}

	if (src_attr->mask & TNET_SW_PID) {
		if (!privileged(SEC_SETPROCIDENT, 0))
			return(EPERM);
		dest_attr->pid = *(pid_t *) in_ptr;
		in_ptr++;
		dest_attr->mask |= TNET_SW_PID;
	}

	return(0);
}


/*
 * sec_internalize_attr()
 *
 * This routine collects the security attributes to be associated with the
 * outgoing data.
 *
 * If src_attr is non-NULL, it contains a set of attributes specified by the
 * user process.  They are checked for validity, and for the appropriate
 * privilege to override the attribute.  If all checks out, they are applied
 * to the outgoing data.  Any missing attributes are filled in from current
 * attributes of the user process.
 *
 * The collected attributes are loaded into dest_attr.
 *
 * The data stored in the tnet_sec_attr structure is position dependent.
 */ 
int
sec_internalize_attr(src_attr, dest_attr)
	register struct tnet_sec_attr	*src_attr;
	register struct str_sec_attr	*dest_attr;
{
	int	error = 0;
	int	i;

	dest_attr->mask = 0;

	/*
	 * First translate any attributes specified by the user process,
	 * and check the user process for the appropriate override privileges.
	 */
	if (src_attr) 
		if (error = sec_in_user_attr(src_attr, dest_attr))
			return(error);

	/* 
	 * If any attributes remain unspecified, fill them in from
	 * current process attributes.
	 */

#if SEC_MAC
	if (!(dest_attr->mask & TNET_SW_SEN_LABEL)) {
		dest_attr->tags[macsubjtag] = SIP->si_tag[macsubjtag];
		dest_attr->mask |= TNET_SW_SEN_LABEL;
	}
#endif

#if SEC_NCAV
	if (!(dest_attr->mask & TNET_SW_NAT_CAVEATS)) {
		dest_attr->tags[ncavsubjtag] = SIP->si_tag[ncavsubjtag];
		dest_attr->mask |= TNET_SW_NAT_CAVEATS;
	}
#endif

#if SEC_ILB
	if (!(dest_attr->mask & TNET_SW_INFO_LABEL)) {
		dest_attr->tags[ilbsubjtag] = SIP->si_tag[ilbsubjtag];
		dest_attr->mask |= TNET_SW_INFO_LABEL;
	}
#endif

	if (!(dest_attr->mask & TNET_SW_PRIVILEGES)) {
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) 
			dest_attr->privs[i] = SIP->si_epriv[i];
		dest_attr->mask |= TNET_SW_PRIVILEGES;
	}

	if (!(dest_attr->mask & TNET_SW_LUID)) {
		dest_attr->luid = SIP->si_luid_set ? SIP->si_luid : -1;
		dest_attr->mask |= TNET_SW_LUID;
	}

	if (!(dest_attr->mask & TNET_SW_PID)) {
		dest_attr->pid = u.u_procp->p_pid;
		dest_attr->mask |= TNET_SW_PID;
	}

	if (!(dest_attr->mask & TNET_SW_IDS)) {
		dest_attr->uid = u.u_uid;
		dest_attr->gid = u.u_gid;
#if SEC_GROUPS
		if (dest_attr->g_len = u.u_ngroups - 1)
			bcopy(u.u_groups, dest_attr->groups,
				dest_attr->g_len * sizeof(gid_t));
#endif
		dest_attr->mask |= TNET_SW_IDS;
	}

	return(error);
}
#endif /* SEC_BASE */
