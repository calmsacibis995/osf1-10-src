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
static char *rcsid = "@(#)$RCSfile: nfs_xdr.c,v $ $Revision: 1.1.14.2 $ (DEC) $Date: 1993/08/31 17:25:35 $";
#endif
/*
 * Replacement for OSF/NFS server code built from ONC/NFS + ULTRIX/NFS code.
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <dirent.h>
#include <sys/mbuf.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <nfs/nfssys.h>
#include <ufs/inode.h>
#include <ufs/dir.h>
#include <sys/presto.h>
#include <sys/prestoioctl.h>
#include <sys/fs_types.h>	

#if	MACH
#include <kern/zalloc.h>
#include <kern/mfs.h>
#else
#include <sys/malloc.h>
#endif

#include <sys/uio.h>
#include <sys/ucred.h>
#include <sys/namei.h>
#include <sys/errno.h>

#ifdef	DEBUG
extern int nfssvcdebug;
#define DPRINTF(c, args)	if(c) printf args
#else
#define DPRINTF(c, args)
#endif
int rrokdebug = 0;

/*
 * SMP lock for sleep/wakeup mechanism around releasing of
 * type 2 mbuf data by networking code.
 */
/*struct	lock_t	lk_nfsrrok;*/
/*
 * These are the XDR routines used to serialize and deserialize
 * the various structures passed as parameters accross the network
 * between NFS clients and servers.
 */

/*
 * File access handle
 * The fhandle struct is treated a opaque data on the wire
 */
bool_t
xdr_fhandle(xdrs, fh)
	XDR *xdrs;
	nfsv2fh_t *fh;
{

	if (xdr_opaque(xdrs, (caddr_t)fh, NFS_FHSIZE)) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Arguments to remote write and writecache
 */
bool_t
xdr_writeargs(xdrs, wa)
	XDR *xdrs;
	struct nfswriteargs *wa;
{
	extern struct xdr_ops xdrmbuf_ops;	/* XXX */
	extern int nfs_write_mbufs;

	if (xdr_fhandle(xdrs, &wa->wa_fhandle) &&
	    xdr_int(xdrs, (int *)&wa->wa_begoff) &&
	    xdr_int(xdrs, (int *)&wa->wa_offset) &&
	    xdr_int(xdrs, (int *)&wa->wa_totcount) &&
	    ((xdrs->x_ops == &xdrmbuf_ops &&
	      xdrs->x_op == XDR_DECODE &&
	      nfs_write_mbufs) ?
	    xdrmbuf_getmbuf(xdrs, (struct mbuf **) &wa->wa_mbuf,
		(u_int *)&wa->wa_count) :
	    xdr_bytes(xdrs, &wa->wa_data, (u_int *)&wa->wa_count,
		NFS_MAXDATA))) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * File attributes
 */
bool_t
xdr_fattr(xdrs, na)
	XDR *xdrs;
	struct nfsfattr *na;
{
	int *ptr;

	if (xdrs->x_op == XDR_ENCODE) {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			IXDR_PUT_ENUM(ptr, na->na_type);
			IXDR_PUT_LONG(ptr, na->na_mode);
			IXDR_PUT_LONG(ptr, na->na_nlink);
			IXDR_PUT_LONG(ptr, na->na_uid);
			IXDR_PUT_LONG(ptr, na->na_gid);
			IXDR_PUT_LONG(ptr, na->na_size);
			IXDR_PUT_LONG(ptr, na->na_blocksize);
			IXDR_PUT_LONG(ptr, na->na_rdev);
			IXDR_PUT_LONG(ptr, na->na_blocks);
			IXDR_PUT_LONG(ptr, na->na_fsid);
			IXDR_PUT_LONG(ptr, na->na_nodeid);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_usec);
			return (TRUE);
		}
	} else {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			na->na_type = IXDR_GET_ENUM(ptr, enum nfsftype);
			na->na_mode = IXDR_GET_LONG(ptr);
			na->na_nlink = IXDR_GET_LONG(ptr);
			na->na_uid = IXDR_GET_LONG(ptr);
			/* otw we get unsigned int -2, and hence we have
			   to convert it to 65534 - standard nobody */
			if (na->na_uid == 0xfffffffe)
				na->na_uid = 65534;
			na->na_gid = IXDR_GET_LONG(ptr);
			if (na->na_gid == 0xfffffffe)
				na->na_gid = 65534;
			na->na_size = IXDR_GET_LONG(ptr);
			na->na_blocksize = IXDR_GET_LONG(ptr);
			na->na_rdev = IXDR_GET_LONG(ptr);
			na->na_blocks = IXDR_GET_LONG(ptr);
			na->na_fsid = IXDR_GET_LONG(ptr);
			na->na_nodeid = IXDR_GET_LONG(ptr);
			na->na_atime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_atime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_usec = IXDR_GET_LONG(ptr);
			return (TRUE);
		}
	}
	if (xdr_enum(xdrs, (enum_t *)&na->na_type) &&
	    xdr_u_int(xdrs, &na->na_mode) &&
	    xdr_u_int(xdrs, &na->na_nlink) &&
	    xdr_u_int(xdrs, &na->na_uid) &&
	    xdr_u_int(xdrs, &na->na_gid) &&
	    xdr_u_int(xdrs, &na->na_size) &&
	    xdr_u_int(xdrs, &na->na_blocksize) &&
	    xdr_u_int(xdrs, &na->na_rdev) &&
	    xdr_u_int(xdrs, &na->na_blocks) &&
	    xdr_u_int(xdrs, &na->na_fsid) &&
	    xdr_u_int(xdrs, &na->na_nodeid) &&
	    xdr_timeval(xdrs, &na->na_atime) &&
	    xdr_timeval(xdrs, &na->na_mtime) &&
	    xdr_timeval(xdrs, &na->na_ctime) ) {
		if (xdrs->x_op == XDR_DECODE) {
			if (na->na_uid == 0xfffffffe)
				na->na_uid = 65534;
			if (na->na_gid == 0xfffffffe)
				na->na_gid = 65534;
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Arguments to remote read
 */
bool_t
xdr_readargs(xdrs, ra)
	XDR *xdrs;
	struct nfsreadargs *ra;
{

	if (xdr_fhandle(xdrs, &ra->ra_fhandle) &&
	    xdr_int(xdrs, (int *)&ra->ra_offset) &&
	    xdr_int(xdrs, (int *)&ra->ra_count) &&
	    xdr_int(xdrs, (int *)&ra->ra_totcount) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Info necessary to free the mapping which is also an mbuf
 */
struct rrokinfo {
	int	(*func)();
	int	done;
	struct vnode *vp;
	struct buf *bp;
	char	*data;
	u_int	count;
};
	
static
rrokfree(rip)
	struct rrokinfo *rip;
{
	int s;
	int error;
	struct mbuf *n;

#ifdef	KTRACE
	kern_trace(321,rip, rip->vp, rip->bp);
#endif  /* KTRACE */
	s = splimp();
	/*smp_lock(&lk_nfsrrok, LK_RETRY);*/
	while (rip->done == 0) {
		/*sleep_unlock((caddr_t)rip, PZERO-1, &lk_nfsrrok);*/
		/*smp_lock(&lk_nfsrrok, LK_RETRY);*/
		DPRINTF(rrokdebug, ("rrokfree: 0x%x zzz ...\n", (caddr_t)rip));
		(void) sleep((caddr_t)rip, PZERO-1);
	}
	DPRINTF(rrokdebug, ("rrokfree: wakeup\n"));

	/*smp_unlock(&lk_nfsrrok);*/
	(void) splx(s);
        DPRINTF(rrokdebug, ("rrokfree: MFREE 0x%x\n", dtom(rip)));

	/*VOP_BRELSE(rip->vp, rip->bp);*/
	DPRINTF(rrokdebug, ("rrokfree: call vop_brelse vp 0x%x bp 0x%x\n",
			rip->vp, rip->bp));
/*		ufs_brelse(rip->vp, rip->bp); */
	VOP_BRELSE(rip->vp, rip->bp, error);
	DPRINTF(rrokdebug, ("rrokfree: vrele vp 0x%x\n", rip->vp));
		vrele(rip->vp);
	DPRINTF(rrokdebug, ("rrokfree: MFREE 0x%x\n", dtom(rip)));

	MFREE(dtom(rip), n);
#ifdef lint
	n = n;
#endif lint
	DPRINTF(rrokdebug, ("rrokfree: done\n"));
}

/*
 * Wake up user process to free mapping and vp (rrokfree)
 */
static
rrokwakeup(addr, size, rip)
	caddr_t addr;
	u_int	size;
	struct rrokinfo *rip;
{

	int s;

#ifdef	KTRACE
	kern_trace(321, rip, 0, 0);
#endif  /* KTRACE */
	s = splimp();
	/*smp_lock(&lk_nfsrrok, LK_RETRY);*/
	rip->done = 1;
	/*smp_unlock(&lk_nfsrrok);*/
	(void) splx(s);
	DPRINTF(rrokdebug, ("rrokwakeup: 0x%x\n", (caddr_t)rip));
	wakeup((caddr_t)rip);
}

int rrok_putbuf = 1;
/*
 * Status OK portion of remote read reply
 */
bool_t
xdr_rrok(xdrs, rrok)
	XDR *xdrs;
	struct nfsrrok *rrok;
{
	int rc;
	int error;

	if (xdr_fattr(xdrs, &rrok->rrok_attr)) {
		DPRINTF(rrokdebug, ("rrok: attr done\n"));
		
		if ( xdrs->x_op == XDR_ENCODE && rrok->rrok_bp) {
			
			struct mbuf *m = (struct mbuf *)NULL;
			struct rrokinfo *rip;

			if (rrok_putbuf) {
				
				DPRINTF(rrokdebug, ("rrok: call MGET\n"));
				MGET(m, M_WAIT, MT_DATA);
				DPRINTF(rrokdebug, ("rrok: MGET 0x%x\n", m));
				if (m == NULL) {
					printf("xdr_rrok: FAILED, can't get mbuf\n");
					/*
					 *  Release resources since rrokfree
					 *  will not be called
					 */
					
					VOP_BRELSE(rrok->rrok_vp,
						   rrok->rrok_bp, error);
/*					ufs_brelse(rrok->rrok_vp,
					           rrok->rrok_bp); */
					vrele(rrok->rrok_vp);
					return (FALSE);
				}
				rip = mtod(m, struct rrokinfo *);
				rip->func = rrokfree;
				rip->done = 0;
				rip->vp = rrok->rrok_vp;
				rip->bp = rrok->rrok_bp;
				rip->data = rrok->rrok_data; /* CJXXX */
				rip->count = rrok->rrok_count; /* CJXXX */
				xdrs->x_public = (caddr_t)rip;
				DPRINTF(rrokdebug,
					("rrok: call putbuf rrokwakeup 0x%x rip 0x%x\n",
					rrokwakeup, rip));
#ifdef	DEBUG
				if (rrokdebug) {
					struct mbuf *xm = (struct mbuf *)xdrs->x_base;
					
					printf("rrok b4 putbuf: x_base 0x%x %d next 0x%x ",
					       xm, xm->m_len, xm->m_next);
					while (xm->m_next != NULL) {
						xm = xm->m_next;
						printf("%d next 0x%x ",
						       xm->m_len, xm->m_next);
					}
					printf("\n");
				}
#endif
				if (xdrmbuf_putbuf(xdrs, rrok->rrok_data,
						   (u_int)rrok->rrok_count, rrokwakeup, (int)rip)) {
#ifdef	DEBUG
					if (rrokdebug) {
						struct mbuf *xm = (struct mbuf *)xdrs->x_base;
						
						printf("rrok after putbuf: x_base 0x%x %d next 0x%x ",
						       xm, xm->m_len, xm->m_next);
						while (xm->m_next != NULL) {
							xm = xm->m_next;
							printf("%d next 0x%x ",
							       xm->m_len,
							       xm->m_next);
						}
						printf("\n");
					}
#endif
					DPRINTF(rrokdebug, ("rrok: OK1\n"));
					return (TRUE);
					DPRINTF(rrokdebug, ("rrok: OK2\n"));
				} else {
					rip->done = 1;
					/*
					 * Try it the old, slow way.
					 */
					DPRINTF(rrokdebug,
						("rrok: call bytes\n"));
					if (xdr_bytes(xdrs, &rrok->rrok_data,
						      (u_int *)&rrok->rrok_count, NFS_MAXDATA)) {
						return (TRUE);
						DPRINTF(rrokdebug,
							("rrok: OK2\n"));
					}
				}
			} else {
#ifdef	DEBUG
				if (rrokdebug) {
					struct mbuf *xm = (struct mbuf *)xdrs->x_base;
					
					printf("rrok b4 bytes: x_base 0x%x %d next 0x%x ",
					       xm, xm->m_len, xm->m_next);
					while (xm->m_next != NULL) {
						xm = xm->m_next;
						printf("%d next 0x%x ",
						       xm->m_len, xm->m_next);
					}
					printf("\n");
				}
#endif
				rc = xdr_bytes(xdrs, &rrok->rrok_data,
					       (u_int *)&rrok->rrok_count,
					       NFS_MAXDATA);
#ifdef	DEBUG
				if (rrokdebug) {
					struct mbuf *xm = (struct mbuf *)xdrs->x_base;
					
					printf("rrok after bytes: x_base 0x%x %d next 0x%x ",
					       xm, xm->m_len);
					while (xm->m_next != NULL) {
						xm = xm->m_next;
						printf("%d next 0x%x ",
						       xm->m_len, xm->m_next);
					}
					printf("\n");
				}
#endif
				DPRINTF(rrokdebug,
				("rrok: call vop_brelse vp 0x%x bp 0x%x\n",
				       rrok->rrok_vp, rrok->rrok_bp));
/*				ufs_brelse(rrok->rrok_vp, rrok->rrok_bp); */
				VOP_BRELSE(rrok->rrok_vp, rrok->rrok_bp,
					   error);
				DPRINTF(rrokdebug,
					("rrok: vrele vp 0x%x\n",
					rrok->rrok_vp));
				vrele(rrok->rrok_vp);
				DPRINTF(!rc && rrokdebug, ("rrok1: NOT OK\n"));
				return (rc);
			}
			
			
		} else {
#ifdef	DEBUG
			if (rrokdebug) {
				struct mbuf *xm = (struct mbuf *)xdrs->x_base;
				
				printf("rrok b4 bytes2: x_base 0x%x %d next 0x%x ",
				       xm, xm->m_len, xm->m_next);
				while (xm->m_next != NULL) {
					xm = xm->m_next;
					printf("%d next 0x%x ", xm->m_len, xm->m_next);
				}
				printf("\n");
			}
#endif
			rc = xdr_bytes(xdrs, &rrok->rrok_data,
				       (u_int *)&rrok->rrok_count, NFS_MAXDATA);
#ifdef	DEBUG
			if (rrokdebug) {
				struct mbuf *xm = (struct mbuf *)xdrs->x_base;
				
				printf("rrok after bytes: x_base 0x%x %d next 0x%x ",
				       xm, xm->m_len, xm->m_next);
				while (xm->m_next != NULL) {
					xm = xm->m_next;
					printf("%d next 0x%x ",
					       xm->m_len, xm->m_next);
				}
				printf("\n");
			}
#endif
			DPRINTF(!rc && rrokdebug, ("rrok2: NOT OK\n"));
			return (rc);
		}
	}
	DPRINTF(rrokdebug,
		("rrok3: NOT OK\n"));
	return (FALSE);
}

struct xdr_discrim rdres_discrim[2] = {
	{ (int)NFS_OK, xdr_rrok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply from remote read
 */
bool_t
xdr_rdresult(xdrs, rr)
	XDR *xdrs;
	struct nfsrdresult *rr;
{

	if (xdr_union(xdrs, (enum_t *)&(rr->rr_status),
	      (caddr_t)&(rr->rr_ok), rdres_discrim, xdr_void) ) {

		DPRINTF(nfssvcdebug, ("xdr_rdresult: return TRUE\n"));
		return (TRUE);
	}
	DPRINTF(nfssvcdebug, ("xdr_rdresult: return FALSE\n"));
	return (FALSE);
}

/*
 * File attributes which can be set
 *
 * Since ufs (and other parts of the kernel) don't really support
 * greater than 16 bit uid's we need a way of representing user
 * nobody (-2) in a useable manner.  So to be consistent with
 * xdr_fattr perform the kludge of translating a 32-bit version of
 * -2 into a 16 bit version thereof.
 */
bool_t
xdr_sattr(xdrs, sa)
	XDR *xdrs;
	struct nfssattr *sa;
{

	if (xdr_u_int(xdrs, &sa->sa_mode) &&
	    xdr_u_int(xdrs, &sa->sa_uid) &&
	    xdr_u_int(xdrs, &sa->sa_gid) &&
	    xdr_u_int(xdrs, &sa->sa_size) &&
	    xdr_timeval(xdrs, &sa->sa_atime) &&
	    xdr_timeval(xdrs, &sa->sa_mtime) ) {
		if (xdrs->x_op == XDR_DECODE) {
			if (sa->sa_uid == 0xfffffffe)
				sa->sa_uid = 65534;
			if (sa->sa_gid == 0xfffffffe)
				sa->sa_gid = 65534;
		}
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim attrstat_discrim[2] = {
	{ (int)NFS_OK, xdr_fattr },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply status with file attributes
 */
bool_t
xdr_attrstat(xdrs, ns)
	XDR *xdrs;
	struct nfsattrstat *ns;
{

	if (xdr_union(xdrs, (enum_t *)&(ns->ns_status),
	      (caddr_t)&(ns->ns_attr), attrstat_discrim, xdr_void) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of read sym link reply union
 */
bool_t
xdr_srok(xdrs, srok)
	XDR *xdrs;
	struct nfssrok *srok;
{

	if (xdr_bytes(xdrs, &srok->srok_data, (u_int *)&srok->srok_count,
	    NFS_MAXPATHLEN) ) {
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim rdlnres_discrim[2] = {
	{ (int)NFS_OK, xdr_srok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Result of reading symbolic link
 */
bool_t
xdr_rdlnres(xdrs, rl)
	XDR *xdrs;
	struct nfsrdlnres *rl;
{

	if (xdr_union(xdrs, (enum_t *)&(rl->rl_status),
	      (caddr_t)&(rl->rl_srok), rdlnres_discrim, xdr_void) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Arguments to readdir
 */
bool_t
xdr_rddirargs(xdrs, rda)
	XDR *xdrs;
	struct nfsrddirargs *rda;
{
	if (xdr_fhandle(xdrs, &rda->rda_fh) &&
	    xdr_u_int(xdrs, &rda->rda_offset) &&
	    xdr_u_int(xdrs, &rda->rda_count) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Directory read reply:
 * union (enum status) {
 *	NFS_OK: entlist;
 *		boolean eof;
 *	default:
 * }
 *
 * Directory entries
 *	struct  direct {
 *		u_int  d_ino;	       	       * inode number of entry *
 *		u_short d_reclen;	       * length of this record *
 *		u_short d_namlen;	       * length of string in d_name *
 *		char    d_name[MAXNAMLEN + 1]; * name no longer than this *
 *	};
 * are on the wire as:
 * union entlist (boolean valid) {
 * 	TRUE: struct dirent;
 *	      u_int nxtoffset;
 * 	      union entlist;
 *	FALSE:
 * }
 * where dirent is:
 * 	struct dirent {
 *		u_int	de_fid;
 *		string	de_name<NFS_MAXNAMELEN>;
 *	}
 */

#define	nextdp(dp)	((struct dirent *)((vm_offset_t)(dp) + (dp)->d_reclen))
#define MINDIRSIZ(dp)	(sizeof(struct dirent) - MAXNAMLEN + (dp)->d_namlen)

/*
 * ENCODE ONLY
 */
bool_t
xdr_putrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	struct dirent *dp;
	off_t offset;
	char *name;
	int size;
	int xdrpos;
	int lastxdrpos;
	u_int namlen;
	bool_t true = TRUE;
	bool_t false = FALSE;

	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}
	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	xdrpos = XDR_GETPOS(xdrs);
	lastxdrpos = xdrpos;
	for (offset = rd->rd_offset, size = rd->rd_size, dp = rd->rd_entries;
	     size > 0;
	     size -= dp->d_reclen, dp = nextdp(dp) ) {
		if (dp->d_reclen == 0 || MINDIRSIZ(dp) > dp->d_reclen) {
			return (FALSE);
		}
		offset += dp->d_reclen;

		if (dp->d_ino == 0) {
			continue;
		}
		name = dp->d_name;
		namlen = dp->d_namlen;
		if (!xdr_bool(xdrs, &true) ||
		    !xdr_u_int(xdrs, &dp->d_ino) ||
		    !xdr_bytes(xdrs, &name, &namlen, NFS_MAXNAMLEN) ||
		    !xdr_u_int(xdrs, (u_int *) &offset) ) {
				return (FALSE);
		}
		if (XDR_GETPOS(xdrs) - xdrpos >= rd->rd_origreqsize -
		    2 * RNDUP(sizeof (bool_t))) {
			XDR_SETPOS(xdrs, lastxdrpos);
			rd->rd_eof = FALSE;
			break;
		} else
			lastxdrpos = XDR_GETPOS(xdrs);
	}
	if (!xdr_bool(xdrs, &false)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	if (XDR_GETPOS(xdrs) - xdrpos >= rd->rd_origreqsize) {
		printf("xdr_putrddirres:  encoding overrun\n");
		return (FALSE);
	} else
		return (TRUE);
}

/*
 * DECODE ONLY
 */
bool_t
xdr_getrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	struct dirent *dp;
	int size;
	bool_t valid;
	bool_t first = TRUE;
	u_int offset = (u_int)-1;

	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	size = rd->rd_size;
	dp = rd->rd_entries;
	for (;;) {
		if (!xdr_bool(xdrs, &valid)) {
			return (FALSE);
		}
		if (valid) {
			unsigned int tmp_fileno;
			unsigned short tmp_namlen;

			if (!xdr_u_int(xdrs, &tmp_fileno) ||
			    !xdr_u_short(xdrs, &tmp_namlen)) {
				return (FALSE);
			} else {
				dp->d_ino = tmp_fileno;
				dp->d_namlen = tmp_namlen;
			}

			if (DIRSIZ(dp) > size) {
				/*
				 *	Entry won't fit.  If this isn't the
				 *	first one, just quit and return what
				 *	we've already XDRed.  Else, it's an
				 *	error.
				 */
				if (first == FALSE) {
					rd->rd_eof = FALSE;
					rd->rd_size = (u_int) ((vm_offset_t)dp -
						(vm_offset_t)rd->rd_entries);
					rd->rd_offset = offset;
					return (TRUE);
				} else {
					return (FALSE);
				}
			}
			if (!xdr_opaque(xdrs, dp->d_name,
					(u_int)dp->d_namlen) ||
			    !xdr_u_int(xdrs, (u_int *) &offset)) {
				return (FALSE);
			}
			dp->d_reclen = DIRSIZ(dp);
			dp->d_name[dp->d_namlen] = '\0';
			first = FALSE;
		} else {
			break;
		}
		size -= DIRSIZ(dp);
		if (size < 0) {
			return (FALSE);
		}
		dp = nextdp(dp);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	rd->rd_size = (u_int) ((vm_offset_t)dp - (vm_offset_t)rd->rd_entries);
	rd->rd_offset = offset;
	return (TRUE);
}

/*
 * Arguments for directory operations
 */
bool_t
xdr_diropargs(xdrs, da)
	XDR *xdrs;
	struct nfsdiropargs *da;
{

	if (xdr_fhandle(xdrs, &da->da_fhandle) &&
#ifdef notdef
	    xdr_string(xdrs, &da->da_name, NFS_MAXNAMLEN) ) {
#else
	    xdr_name(xdrs, &da->da_name, MIN(da->da_len, NFS_MAXNAMLEN)) ) {
#endif
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of directory operation result
 */
bool_t
xdr_drok(xdrs, drok)
	XDR *xdrs;
	struct nfsdrok *drok;
{

	if (xdr_fhandle(xdrs, &drok->drok_fhandle) &&
	    xdr_fattr(xdrs, &drok->drok_attr) ) {
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim diropres_discrim[2] = {
	{ (int)NFS_OK, xdr_drok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results from directory operation 
 */
bool_t
xdr_diropres(xdrs, dr)
	XDR *xdrs;
	struct nfsdiropres *dr;
{

	if (xdr_union(xdrs, (enum_t *)&(dr->dr_status),
	      (caddr_t)&(dr->dr_drok), diropres_discrim, xdr_void) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Time structure
 */
bool_t
xdr_timeval(xdrs, tv)
	XDR *xdrs;
	struct timeval *tv;
{

	if (xdr_int(xdrs, &tv->tv_sec) &&
	    xdr_int(xdrs, &tv->tv_usec) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to setattr
 */
bool_t
xdr_saargs(xdrs, argp)
	XDR *xdrs;
	struct nfssaargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->saa_fh) &&
	    xdr_sattr(xdrs, &argp->saa_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to create and mkdir
 */
bool_t
xdr_creatargs(xdrs, argp)
	XDR *xdrs;
	struct nfscreatargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->ca_da) &&
	    xdr_sattr(xdrs, &argp->ca_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to link
 */
bool_t
xdr_linkargs(xdrs, argp)
	XDR *xdrs;
	struct nfslinkargs *argp;
{

	if (xdr_fhandle(xdrs, &argp->la_from) &&
	    xdr_diropargs(xdrs, &argp->la_to) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to rename
 */
bool_t
xdr_rnmargs(xdrs, argp)
	XDR *xdrs;
	struct nfsrnmargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->rna_from) &&
	    xdr_diropargs(xdrs, &argp->rna_to) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to symlink
 */
bool_t
xdr_slargs(xdrs, argp)
	XDR *xdrs;
	struct nfsslargs *argp;
{

	if (xdr_diropargs(xdrs, &argp->sla_from) &&
	    xdr_string(xdrs, &argp->sla_tnm, (u_int)MAXPATHLEN) &&
	    xdr_sattr(xdrs, &argp->sla_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of statfs operation
 */
xdr_fsok(xdrs, fsok)
	XDR *xdrs;
	struct nfsstatfsok *fsok;
{
	if (xdr_int(xdrs, (int *)&fsok->fsok_tsize) &&
	    xdr_int(xdrs, (int *)&fsok->fsok_bsize) &&
	    xdr_int(xdrs, (int *)&fsok->fsok_blocks) &&
	    xdr_int(xdrs, (int *)&fsok->fsok_bfree) &&
	    xdr_int(xdrs, (int *)&fsok->fsok_bavail) ) {
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim statfs_discrim[2] = {
	{ (int)NFS_OK, xdr_fsok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results of statfs operation
 */
xdr_statfs(xdrs, fs)
	XDR *xdrs;
	struct nfsstatfs *fs;
{
	if (xdr_union(xdrs, (enum_t *)&(fs->fs_status),
	      (caddr_t)&(fs->fs_fsok), statfs_discrim, xdr_void) ) {
		return (TRUE);
	}
	return (FALSE);
}

bool_t
xdr_name(xdrs, cpp, size)
	XDR *xdrs;
	char **cpp;
	u_int size;
{
	char *sp = *cpp;  /* sp is the actual string pointer */
	u_int nodesize;

	if (xdrs->x_op == XDR_FREE && sp == NULL)
		return (TRUE);

	if (! xdr_u_int(xdrs, &size)) {
		return (FALSE);
	}
	nodesize = size + 1;

	switch (xdrs->x_op) {
	case XDR_DECODE:
		if (nodesize == 0) {
			return (TRUE);
		}
		if (sp == NULL)
			*cpp = sp = (char *)mem_alloc(MAXPATHLEN);
		sp[size] = 0;
	case XDR_ENCODE:
		return (xdr_opaque(xdrs, sp, size));
	case XDR_FREE:
		mem_free(sp, MAXPATHLEN);
		*cpp = NULL;
		return (TRUE);
	}
	return (FALSE);
}


