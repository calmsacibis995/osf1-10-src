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
static char *rcsid = "@(#)$RCSfile: nfs_vnodeops.c,v $ $Revision: 1.1.36.8 $ (DEC) $Date: 1993/09/22 18:29:21 $";
#endif
/*	@(#)nfs_vnodeops.c	2.14 90/07/12 NFSSRC4.1 from 2.202 90/02/01 SMI 	*/
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/buf.h>
#include <sys/vp_swap.h>
#include <vm/vm_page.h>
#include <vm/vm_mmap.h>
#include <nfs/pathname.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/vfs_ubc.h>
#include <mach/vm_prot.h>
#include <rpc/rpc.h>
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>
#include <nfs/nfssys.h>
#include <klm/lockmgr.h>

#include <mach_nbc.h>
#include <mach_xp.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/assert.h>
#include <kern/parallel.h>

struct vnode *makenfsnode();
char *newname();
int setdirgid();
u_int setdirmode();
int read_ahead, not_read_ahead = 0;
int dbg_dirty_write,dbg_dirty_read,dbg_dirty_pwrite;
int nfs_link_cache = 1;		/* Non-zero to enable */
int nfs_rlhits;			/* readlink cache hits */

/*
 * Mask some of the security differences.
 */
#if	SEC_BASE
#define	PRIV_SUSER(cr,acflag,priv,code,retcode,error)			\
	error = !privileged(priv,code);   				\
	if (error)							\
		return (retcode);
#else
#define	PRIV_SUSER(cr,acflag,priv,code,retcode,error)			\
	if (error = suser((cr),(acflag)))				\
		return (error);
#define	SEC_REMOTE	0
#endif
	
/*
 * Maximum allowable number of asyncdaemons (patchable).
 */
#define	NFS_MAXASYNCDAEMON 20	/* Max. number async_daemons runnable */
int	max_asyncdaemons = NFS_MAXASYNCDAEMON;

/*
 * Do close to open consistency checking on all filesystems.
 * If this boolean is false, CTO checking can be selectively
 * turned off by setting actimeo to -1 at mount time.
 */
int nfs_cto = 1;

/*
 * Error flags used to pass information about certain special errors
 * back from do_bio() to nfs_getapage() (yuck).
 */
#define ISVDEV(t) ((t == VBLK) || (t == VCHR) || (t == VFIFO))

#define	NFS_EOF		-98

/*
 * Kludge to backup a uio struct so that uio_resid is accurate for the
 * I/O actually done.  Also, backup uio_offset.  We don't have enough
 * information to backup the iovs, so don't try calling uiomove after
 * calling this!
 */
#define uiorewind(uio, len) (uio)->uio_resid += len, (uio)->uio_offset -= len

/*
 * These are the vnode ops routines which implement the vnode interface to
 * the networked file system.  These routines just take their parameters,
 * make them look networkish by putting the right info into interface structs,
 * and then calling the appropriate remote routine(s) to do the work.
 *
 * Note on directory name lookup cacheing:  we desire that all operations
 * on a given client machine come out the same with or without the cache.
 * To correctly do this, we serialize all operations on a given directory,
 * by using RLOCK and RUNLOCK around rfscalls to the server.  This way,
 * we cannot get into races with ourself that would cause invalid information
 * in the cache.  Other clients (or the server itself) can cause our
 * cached information to become invalid, the same as with data pages.
 * Also, if we do detect a stale fhandle, we purge the directory cache
 * relative to that vnode.  This way, the user won't get burned by the
 * cache repeatedly.
 */

extern int vn_maxprivate;	/* From vfs_subr.c, space between vnodes */

/*
 * NFS vnode operations
 */
static int nfs_open(), nfs_close(), nfs_rdwr(), nfs_mknod(),
	   nfs_getattr(), nfs_setattr(), nfs_access(), nfs_lookup(), nfs_create(),
	   nfs_remove(), nfs_link(), nfs_rename(), nfs_mkdir(), nfs_rmdir(), nfs_readdir(),
	   nfs_symlink(), nfs_readlink(), nfs_fsync(), nfs_inactive(), nfs_lockctl(),
	   nfs_bmap(), nfs_strategy(), nfs_seek(), nfs_reclaim(), nfs_print(),
	   nfs_page_read(), nfs_page_write(), nfs_mmap(), nfs_getpage(),
	   nfs_putpage(), nfs_swap(), nfs_bread(), nfs_brelse(), nfs_syncdata();
int nfs_noop();
extern int vfs_noop(), nfs_badop(), nfs_dump(), seltrue();
struct vnodeops nfs_vnodeops = {
	nfs_lookup,
	nfs_create,
	nfs_mknod,
	nfs_open,
	nfs_close,
	nfs_access,
	nfs_getattr,
	nfs_setattr,
	nfs_rdwr,	/* read */
	nfs_rdwr,	/* write */
	vfs_noop,	/* ioctl */
	seltrue,	/* select */
	nfs_mmap,	/* mmap */
	nfs_fsync,
	nfs_seek,
	nfs_remove,
	nfs_link,
	nfs_rename,
	nfs_mkdir,
	nfs_rmdir,
	nfs_symlink,
	nfs_readdir,
	nfs_readlink,
	nfs_noop,	/* abortop */
	nfs_inactive,
	nfs_reclaim,
	nfs_bmap,
	nfs_strategy,
	nfs_print,
	nfs_page_read,
	nfs_page_write,
	nfs_getpage,
	nfs_putpage,
	nfs_swap,
	nfs_bread,
	nfs_brelse,
	nfs_lockctl,	/* Need for KLM support */
	nfs_syncdata,	/* fsync byte range */
#ifdef notdef
	nfs_dump,	/* Need for diskless support */
#endif
};

/* 
 * Special device vnode ops 
 */
extern int spec_lookup(), spec_open(), spec_read(), spec_write(), spec_strategy(), 
	   spec_bmap(), spec_ioctl(), spec_select(), spec_seek(), spec_close(),
	   spec_badop(), spec_nullop(), spec_lockctl();
static int nfsspec_reclaim();
struct vnodeops spec_nfsv2nodeops = {
        spec_lookup,
        spec_badop,             /* create */
        spec_badop,             /* mknod */
        spec_open, 
        spec_close,
        nfs_access,
        nfs_getattr,
        nfs_setattr,
        spec_read, 
        spec_write,
        spec_ioctl,
        spec_select,
        spec_badop,             /* mmap */
        spec_nullop,            /* fsync */
        spec_seek, 
        spec_badop,             /* remove */
        spec_badop,             /* link */
        spec_badop,             /* rename */
        spec_badop,             /* mkdir */
        spec_badop,             /* rmdir */
        spec_badop,             /* symlink */
        spec_badop,             /* readdir */
        spec_badop,             /* readlink */
        spec_badop,             /* abortop */
        nfs_inactive,
        nfsspec_reclaim,
        spec_bmap,
        spec_strategy,
        nfs_print,
        spec_badop,             /* page in */
        spec_badop,             /* page out */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	nfs_swap,		/* swap */
	nfs_bread,
	nfs_brelse,
	spec_lockctl,		/* file locking */
};

/*
 * FIFO vnode operations
 */
extern int fifo_open(), fifo_close(), fifo_read(), fifo_write(), fifo_ioctl(),
	   fifo_select();
static int nfsfifo_getattr();
struct vnodeops fifo_nfsv2nodeops = {
        spec_lookup,            /* lookup */
        spec_badop,
        spec_badop,
        fifo_open,
        fifo_close,
        nfs_access,
        nfsfifo_getattr,
        nfs_setattr,
        fifo_read,
        fifo_write,
        fifo_ioctl,
        fifo_select,
        spec_badop,             /* mmap */
        spec_nullop,            /* fsync */
        spec_seek,
        spec_badop,             /* remove */
        spec_badop,             /* link */
        spec_badop,             /* rename */
        spec_badop,             /* mkdir */
        spec_badop,             /* rmdir */
        spec_badop,             /* symlink */
        spec_badop,             /* readdir */
        spec_badop,             /* readlink */
        spec_badop,             /* abortop */
        nfs_inactive,
        nfs_reclaim,
        spec_bmap,
        spec_badop,             /* strategy */
        nfs_print,
        spec_badop,             /* page in */
        spec_badop,             /* page out */
        spec_badop,             /* getpage */
        spec_badop,             /* putpage */
        spec_badop,             /* swap */
	nfs_bread,
	nfs_brelse,
        spec_lockctl,           /* file locking */
};

/*ARGSUSED*/
static int
nfs_open(vpp, flag, cred)
	register struct vnode **vpp;
	int flag;
	struct ucred *cred;
{
	int error = 0;
	struct vattr va;

	ASSERT(vp->v_type == VREG || vp->v_type == VDIR || vp->v_type == VLNK);

	/*
	 * if close-to-open consistency checking is turned off
	 * we can avoid the over the wire getattr.
	 */
	if (nfs_cto || !vtomi(*vpp)->mi_nocto) {
		/*
		 * Force a call to the server to get fresh attributes
		 * so we can check caches. This is required for close-to-open      
		 * consistency.
		 */
		error = nfs_getattr_otw(*vpp, &va, cred);
		if (error == 0) {
			nfs_cache_check(*vpp, va.va_mtime, (u_int)va.va_size);
			nfs_attrcache_va(*vpp, &va);
		}
	}
	return (error);
}

static int
nfs_close(vp, flag, cred)
	struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	register struct rnode *rp = vtor(vp);
	int error = 0;
	struct vattr va;

	/*
	 * If the file is an unlinked file, then flush the lookup
	 * cache so that nfs_inactive will be called if this is
	 * the last reference.  Otherwise, if close-to-open
	 * consistency is turned on and the file was open
	 * for writing, we force the "sync on close" semantic by 
	 * calling sync_vp.
	 */
	RLOCK(rp);
	if (rp->r_ndp || rp->r_error) {
		if (!rp->r_error) {
			ubc_invalidate(vp, 0, 0, 0);
			waitforio(vp);
		} else
			ubc_invalidate(vp, 0, 0, B_INVAL);
		cache_purge(vp);
		rp->r_flags &= ~RDIRTY;
		/* 
		 * Want correct file size on close (we know client's 
		 * view is wrong) so force a call to the server.
		 */
		if (rp->r_error) {
			error = nfs_getattr_otw(vp, &va, cred);
			if (error == 0) {
				nfs_attrcache_va(vp, &va);
			}
		}
		error = rp->r_error;
		rp->r_error = 0;
	} else if ((nfs_cto || !vtomi(vp)->mi_nocto) &&
			(flag & FWRITE)) {
		sync_vp(vp);
		if (rp->r_error) {
			ubc_invalidate(vp, 0, 0, B_INVAL);
                	cache_purge(vp);
			/* 
		 	 * Want correct file size on close (we know client's 
		 	 * view is wrong).
		 	 */
			error = nfs_getattr_otw(vp, &va, cred);
			if (error == 0) {
				nfs_attrcache_va(vp, &va);
			}
			error = rp->r_error;
			rp->r_error = 0;
		}
	}
	RUNLOCK(rp);
	return (flag & FWRITE? error : 0);
}

static int
nfs_rdwr(vp, uiop, ioflag, cred)
	register struct vnode *vp;
        struct uio *uiop;
	int ioflag;
	struct ucred *cred;
{
	register struct rnode *rp = vtor(vp);
	enum uio_rw rw = uiop->uio_rw;
	int error = 0;
	struct ucred *tcred;

	if (vp->v_type == VDIR)
		return(EISDIR);

	ASSERT(vp->v_type == VREG);

	/* rnode cred setup code moved to nfs_getpage */

	/*
	 * A Sun change to get attributes on every I/O if RNOCACHE is 
	 * set is inappropriate, as all they wanted to do is update 
	 * r_size before a read that exceeds end-of-file!
	 */
	if ((ioflag & IO_APPEND) && rw == UIO_WRITE) {
		struct vattr va;

		RLOCK(rp);
		error = nfs_getattr(vp, &va, cred);
		if (error == 0) {
			uiop->uio_offset = va.va_size;
		}
	}

	if (error == 0) {
		error = rwvp(vp, uiop, ioflag, cred);
	}

	if ((ioflag & IO_APPEND) && rw == UIO_WRITE) {
		RUNLOCK(rp);
	}
	return (error);
}

rwvp(vp, uio, ioflag, cred)
	register struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	enum uio_rw uio_rw = uio->uio_rw;
	int uio_resid = uio->uio_resid;
	u_int flags;
	int error = 0;
	int eof = 0;
	int size;
        register struct rnode *rp;
        register vm_offset_t off;
        register vm_size_t len;
	caddr_t buf = (caddr_t) NULL;

	if (uio->uio_resid == 0) {
		return (0);
	}
	if (uio->uio_offset + uio->uio_resid > 0x7fffffff) {
		return (EINVAL);
	}
	if (uio_rw == UIO_WRITE && vp->v_type == VREG &&
	    uio->uio_offset+uio->uio_resid >
	    u.u_rlimit[RLIMIT_FSIZE].rlim_cur) {
		unix_master();
		psignal(u.u_procp, SIGXFSZ);
		unix_release();
		return (EFBIG);
	}
	rp = vtor(vp);
	RLOCK(rp);
	size = vtoblksz(vp);
	size &= ~(DEV_BSIZE - 1);

	if (size <= 0) {
		panic("rwvp: zero size");
	}
	do { /* for entire user request or eof for read */
		/*
		 * If asynchronous I/O completion via cache flush or
		 * async daemon finds an error, we'll see it here.  Once
		 * we are certain the user will see it, clear the cell
		 * to let future I/O work, although recovery is problematic.
		 * write() will discard the error if any bytes made it out,
		 * so we can't clear it until we have no output to report.
		 * Weirdness: This can also be set by writes when all the
		 * async daemons are busy, in which case we've already seen
		 * the error as a return value from rwvp_cache....
		 */
		if (error = rp->r_error) { /* Old error from async daemon */
			if (uio_resid == uio->uio_resid)
				rp->r_error = 0;
			goto done;
		}
		off = uio->uio_offset % size;
		len = MIN(uio->uio_resid, (unsigned)(size - off));

		if (rp->r_flags & RNOCACHE) {
			if (!buf) {
				buf = (caddr_t) kalloc(size);
				if (!buf) {
					error = ENOBUFS;
					break;
				}
			}
			error = rwvp_nocache(vp, uio, cred, len, &eof, buf);
		} else
			error = rwvp_cache(vp, uio, cred, len, &eof, size, ioflag);
	} while (error == 0 && uio->uio_resid > 0 && !eof);

done:
	RUNLOCK(rp);
	if (buf)
		kfree(buf, size);
	return (error);
}

rwvp_cache(vp, uio, cred, len, eof, size, ioflag)
	register struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
        register vm_size_t len;
	int *eof;
	int size;
	int ioflag;
{
        register struct rnode *rp = vtor(vp);
        vm_page_t pl[VP_PAGELIST+1];
        register vm_page_t *pp;
	register int n;
        vm_offset_t addr;
        register vm_offset_t off;
	int error = 0;
	int wflag = (ioflag & IO_SYNC)?
		B_WRITE | B_AGE:
		B_WRITE | B_AGE | B_ASYNC;

	off = uio->uio_offset & page_mask;
	error = nfs_getpage(vp, uio->uio_offset, len,
			    (vm_prot_t *) 0, pl, atop(round_page(len+off)),
			    (vm_map_entry_t) 0, (vm_offset_t) 0,
			    uio->uio_rw == UIO_READ, cred);
	if (error) return error;
	pp = pl;

	do {			/* for each page returned by getpage */
		vm_page_wait(*pp);
		n = MIN((PAGE_SIZE - off), uio->uio_resid);
		if(uio->uio_rw == UIO_READ) {
			int diff;
			diff = rp->r_size - uio->uio_offset;
			if (diff <= 0) {
				*eof = 1;
				goto done;
			}
			if (diff <= n) {
				n = diff;
				*eof = 1;
			}
		}
		addr = ubc_load(*pp, off, n);
		error = uiomove(addr + off, n, uio);
		ubc_unload(*pp, off, n);
		if (error) break;
		if (uio->uio_rw == UIO_READ) {
			ubc_page_release(*pp, 0);
		} else {
			/*
			 * r_size is the maximum number of bytes known
			 * to be in the file.
			 * Make sure it is at least as high as the last
			 * byte we just wrote into the buffer.  Used by
			 * nfswrite when writing the last block of the file!
			 */
			if (rp->r_size < uio->uio_offset)
				rp->r_size = uio->uio_offset;
			rp->r_flags |= RDIRTY;
			if (uio->uio_offset % size && !(ioflag & IO_SYNC)) {
				ubc_page_release(*pp, B_DIRTY);
			} else {
				vm_page_t kpl;
				int kpcnt;

				/*
				 * Fetch page list of busy pages
				 * within the block.
				 */

				kpl = ubc_dirty_kluster(vp, *pp,
							(*pp)->pg_offset & 
							~(size -1),
							size, B_WANTED, FALSE, 
							&kpcnt);


				/*
				 * Write these pages. At I/O done
				 * time age asynchronous writes.
				 * Note the held pl[0] page's
				 * hold cnt has been decremented 
				 * because no hold, "FALSE", was 
				 * specified.
				 */
				if (error = nfs_rwblk(vp, kpl, kpcnt, wflag,
						  (daddr_t) 0,
						  uio->uio_offset-n, cred))
					uiorewind(uio, n);
			} 
		}
		*pp = VM_PAGE_NULL;
		pp++;
		off = 0;
	} while (error == 0 && *pp != VM_PAGE_NULL && *eof == 0);

done:
	while (*pp != VM_PAGE_NULL) {
		ubc_page_release(*pp, 0);
		pp++;
	}

	return error;
}

/*
 * Routine used for doing I/O to locked files.  We completely bypass the
 * buffer cache (no ubc_ calls) and I/O daemons (we don't even need a struct
 * buf).  I/O is done directly to the buffer passed in.  If the rnode's
 * idea of the file size is old and the file has shrunk, we must be
 * responsible for detecting EOF.
 */
rwvp_nocache(vp, uio, cred, len, eof, buf)
	register struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
        register vm_size_t len;
	int *eof;
	caddr_t buf;
{
        register struct rnode *rp = vtor(vp);
	int resid;
	int error;

	if (uio->uio_rw == UIO_READ) {
		if (error = nfsread(vp, buf, uio->uio_offset, len, &resid, cred))
			return error;
		if (resid) {	/* implies an earlier EOF than expected */
			len -= resid;
			*eof = 1;
			if (len == 0) return(0);
		}
	}
	if (error = uiomove(buf, len, uio))
		return error;
	if (uio->uio_rw == UIO_WRITE) {
		/*
		 * r_size is the maximum number of bytes known
		 * to be in the file.
		 * Make sure it is at least as high as the last
		 * byte we just wrote into the buffer.
		 */
		if (rp->r_size < uio->uio_offset)
			rp->r_size = uio->uio_offset;
		if (error = nfswrite(vp, buf, uio->uio_offset - len, len,
				     0, cred))
			uiorewind(uio, len);
	}
	return error;
}

nfs_rwblk(struct vnode *vp,
        vm_page_t pp,
        int pcnt,
        int flags,
        daddr_t rablkno,
	vm_offset_t offset,
	struct ucred *cred)
{
        struct buf *rbp;
        register struct buf *bp;
	register vm_page_t lp;
	int error, size;
	daddr_t blkno;

	lp = pp;

	size = ptoa(pcnt);

        ubc_bufalloc(pp, pcnt, &rbp, size, 1, flags);
        bp = rbp;
	bp->b_vp = vp;
	/*nfs_bmap(vp, lp->pg_offset / vtoblksz(vp), &blkno); */
	/* Could get 1 page requests here if only one page of 
	 * filesystem block is dirty.
	 */
	blkno = (lp->pg_offset & ~(PAGE_SIZE - 1)) / DEV_BSIZE;
	bp->b_blkno = blkno;

	if (flags & B_READ) {
		bp->b_rcred = cred;
		crhold(cred);
	}

        /*
         * Call nfs bmap and setup up the rest of the buf
         * structure for the nfs strategy routine.
         */

        error = nfs_strategy(bp);


        if (error && ((flags & B_ASYNC) == 0)) {
                ubc_sync_iodone(bp);
                return error;
        }

        /*
         * If not async I/O return.
         */
        else if ((flags & B_ASYNC) == 0) {
                error = ubc_sync_iodone(bp);
		return error; 
        }

        else return 0;
}
/*
nfs_rablk(register struct vnode *vp,
        register daddr_t rablkno)
{
        register vm_page_t pp, lp;
        register vm_size_t size;
        int error, kpcnt;
        boolean_t alloc;
        daddr_t dblkno;
        struct buf *rbp;

        pp = ubc_kluster(vp, (vm_page_t) 0,
                (rablkno * vtoblksz(vp)),
                vtoblksz(vp),
                B_CACHE|B_WANTFREE|B_BUSY, UBC_HNONE, &kpcnt);

	if (pp) {
		error = nfs_bmap(vp, rablkno, &dblkno);
                if (error) do {
                        lp = pp->pg_pnext;
                        ubc_page_release(pp, B_DONE|B_ERROR);
                } while (pp = lp);
	



*/


/*
 * Write to file.  Writes to remote server in largest size
 * chunks that the server can handle.  Write is synchronous. 
 *	
 * Attributes are only cached on successful synchronous writes.
 */
static int
nfswrite(vp, base, offset, count, sync, cred)
	struct vnode *vp;
	caddr_t base;
	u_int offset;
	int count;
	int sync;		/* Not used */
	struct ucred *cred;
{
	int error;
	struct nfswriteargs wa;
	struct nfsattrstat *ns;
	int tsize;

	ns = (struct nfsattrstat *)kmem_alloc(sizeof (*ns));
	do {
		tsize = MIN(vtomi(vp)->mi_curwrite, count);
		wa.wa_data = base;
		wa.wa_fhandle = *vtofh(vp);
		wa.wa_offset = wa.wa_begoff = offset;
		wa.wa_count = wa.wa_totcount = tsize;
		error = rfscall(vtomi(vp), RFS_WRITE, xdr_writeargs,
		    (caddr_t)&wa, xdr_attrstat, (caddr_t)ns, cred);
		if (error == ENFS_TRYAGAIN) {
			error = 0;
			continue;
		}
		if (!error) {
			error = geterrno(ns->ns_status);
			PURGE_STALE_FH(error, vp); 
		}
		count -= tsize;
		base += tsize;
		offset += tsize;
	} while (!error && count);
	
	switch (error) {
	case 0:
		nfs_attrcache(vp, &ns->ns_attr);
		break;
	case EDQUOT:
	case EINTR:
		break;
	case ENOSPC:
		printf("NFS write error: on host %s remote file system full\n",
		   vtomi(vp)->mi_hostname);
		break;
	default:
		printf("NFS write error %d on host %s fh ",
		    error, vtomi(vp)->mi_hostname);
		printfhandle((caddr_t)vtofh(vp));
		printf("\n");
		break;
	}

	kmem_free((caddr_t)ns, sizeof (*ns));
	return (error);
}

/*
 * Read from a file.  Reads data in largest chunks our interface can handle.
 *	
 * Attributes are only cached on successful reads.
 */
static int
nfsread(vp, base, offset, count, residp, cred)
	struct vnode *vp;
	caddr_t base;
	u_int offset;
	int count;
	int *residp;
	struct ucred *cred;
{
	int error;
	struct nfsreadargs ra;
	struct nfsrdresult rr;
	register int tsize;

	do {
		tsize = MIN(vtomi(vp)->mi_curread, count);
		rr.rr_data = base;
		ra.ra_fhandle = *vtofh(vp);
		ra.ra_offset = offset;
		ra.ra_count = ra.ra_totcount = tsize;
		do {
			error = rfscall(vtomi(vp), RFS_READ, xdr_readargs, (caddr_t)&ra,
				xdr_rdresult, (caddr_t)&rr, cred);
		} while (error == ENFS_TRYAGAIN);

		if (!error) {
			error = geterrno(rr.rr_status);
			PURGE_STALE_FH(error,vp);
		}
		if (!error) {
			count -= rr.rr_count;
			base += rr.rr_count;
			offset += rr.rr_count;
		}
	} while (!error && count && rr.rr_count == tsize);

	*residp = count;

	if (!error) {
		/*
		 * Can't call nfs_cache_check() because it will try to
		 * invalidate all pages, including our busy page.  So we
		 * do almost everything nfs_cache_check does, except we
		 * pass B_BUSY to ubc_invalidate to leave our page and other
		 * in-transit pages alone.
		 */
		struct rnode *rp = vtor(vp);

		if (!CACHE_VALID(rp, rr.rr_attr.na_mtime, rr.rr_attr.na_size)) {
			INVAL_ATTRCACHE(vp);
			cache_purge(vp);
			ubc_invalidate(vp, 0, 0, B_BUSY);
			rp->r_flags &= ~REOF;
		}
		nfs_attrcache(vp, &rr.rr_attr);
	}
	return (error);
}

static int
nfs_getattr(vp, vap, cred)
	struct vnode *vp;
	struct vattr *vap;
	struct ucred *cred;
{
	struct rnode *rp = vtor(vp);

	RLOCK(rp);
	(void) sync_vp(vp);
	RUNLOCK(rp);
	return(nfsgetattr(vp, vap, cred));
}

static int
nfs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;
	struct nfssaargs args;
	struct nfsattrstat *ns;

	ns = (struct nfsattrstat *)kmem_alloc(sizeof (*ns));
	if ((vap->va_nlink != -1) || (vap->va_blocksize != -1) ||
	    (vap->va_rdev != -1) || (vap->va_bytes != -1) ||
	    (vap->va_ctime.tv_sec != -1) || (vap->va_ctime.tv_usec != -1)) {
		error = EINVAL;
	} else {
		RLOCK(vtor(vp));
		sync_vp(vp);
		RUNLOCK(vtor(vp));
		if (vap->va_size != -1) {
			if (vap->va_size > 0x7fffffff) {
				error = EINVAL;
			}
			(vtor(vp))->r_size = vap->va_size;
		}

		/*
		 * Allow SysV-compatible option to set access and
		 * modified times if root, owner, or write access.
		 *
		 * XXX - For now, va_mtime.tv_usec == -1 flags this.
		 *
		 * XXX - Until an NFS Protocol Revision, this may be
		 *       simulated by setting the client time in the
		 *       tv_sec field of the access and modified times
		 *       and setting the tv_usec field of the modified
		 *       time to an invalid value (1,000,000).  This
		 *       may be detected by servers modified to do the
		 *       right thing, but will not be disastrous on
		 *       unmodified servers.
		 */
		if ((vap->va_mtime.tv_sec != -1) &&
		    (vap->va_mtime.tv_usec == -1)) {
			vap->va_atime = time;
			vap->va_mtime.tv_sec = time.tv_sec;
			vap->va_mtime.tv_usec = 1000000;
		}

		vattr_to_sattr(vap, &args.saa_sa);
		args.saa_fh = *vtofh(vp);
		error = rfscall(vtomi(vp), RFS_SETATTR, xdr_saargs,
		    (caddr_t)&args, xdr_attrstat, (caddr_t)ns, cred);
		if (error == 0) {
			error = geterrno(ns->ns_status);
			if (error == 0) {
				nfs_cache_check(vp, ns->ns_attr.na_mtime, ns->ns_attr.na_size);
				nfs_attrcache(vp, &ns->ns_attr);
			} else {
				PURGE_ATTRCACHE(vp);
				PURGE_STALE_FH(error, vp);
			}
		} else
			PURGE_ATTRCACHE(vp);
	}
	kmem_free((caddr_t)ns, sizeof (*ns));
	return (error);
}

static int
nfs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	struct vattr va;
	gid_t *gp;
	int error = 0;

	error = nfsgetattr(vp, &va, cred);
	if (error) 
		return(error);
	/*
	 * If you're the super-user,
	 * you always get access.
	 */
	if (cred->cr_uid == 0)
		return (0);
	/*
	 * Access check is based on only
	 * one of owner, group, public.
	 * If not owner, then check group.
	 * If not a member of the group,
	 * then check public access.
	 */
	if (cred->cr_uid != va.va_uid) {
		register int i;

		mode >>= 3;
		if (cred->cr_gid == va.va_gid)
			goto found;
		gp = cred->cr_groups;
		for (i=0; i < cred->cr_ngroups; i++, gp++)
			if (va.va_gid == *gp)
				goto found;
		mode >>= 3;
	}
found:
	if ((va.va_mode & mode) == mode)
		return (0);
	return (EACCES);
}

/*
 * Notes on symbolic link caching:
 * Symlinks are cached in the free space between the end of the rnode and
 * the start of the vnode.  The layout of the vnode table is a vector that
 * replicates something like:
 *	struct {
 *		struct vnode;		[minus v_data!]
 *		union {
 *			struct inode;
 *			struct rnode;
 *		}
 *	}
 *
 * The inodes are about 100 bytes bigger than rnodes, so we just use wasted
 * space that follows the rnode.  It's not part of the rnode because when
 * this was implemented, we were supposed to avoid changing sizes of data
 * structures!  The symlink stored here is valid if the cached attributes
 * are valid, and we update attributes that have expired if we cache them at
 * all.  When the rnode is modified, the first byte of the symlink is zeroed
 * to invalidate it.  (This means we do not cache zero length symlinks.  Big
 * deal!)
 *
 * When we have to ask the server for the symlink, we'll save the symlink
 * if it is short enough.
 *
 * This code could break automount, but as automount doesn't let attributes
 * be cached on its intercept points, we don't impact it.
 */
#define MAX_LINK_LENGTH (vn_maxprivate - sizeof(struct rnode) - 1)

static int
nfs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	register struct rnode *rp = vtor(vp);
	char *lp = rtol(rp);	/* Pointer to symlink cache entry */
	int error;
	struct nfsrdlnres rl;

	ASSERT(vp->v_type == VLNK);

	if (nfs_link_cache) {
		if (vtomi(vp)->mi_acregmax) /* Do we cache attributes? */
			nfs_validate_caches(vp, cred);
		if (lp[0] && time.tv_sec < rp->r_attrtime.tv_sec) {
			error = uiomove(lp, strlen(lp), uiop);
			nfs_rlhits++; /* add to nfs_clstat? */
			return (error);
		}
		if (vtomi(vp)->mi_acregmax)
			nfs_validate_caches(vp, cred);
	}
	rl.rl_data = (char *)kmem_alloc(NFS_MAXPATHLEN);
	error = rfscall(vtomi(vp), RFS_READLINK, xdr_fhandle,
	    (caddr_t)vtofh(vp), xdr_rdlnres, (caddr_t)&rl, cred);
	if (!error) {
		error = geterrno(rl.rl_status);
		if (!error) {
			ASSERT(uiop->uio_rw == UIO_READ);
			error = uiomove(rl.rl_data, (int)rl.rl_count, uiop);
			if ((int)rl.rl_count < MAX_LINK_LENGTH) {
				bcopy(rl.rl_data, lp, (int)rl.rl_count);
				lp[(int)rl.rl_count] = '\0';
			}
		} else {
			PURGE_STALE_FH(error, vp);
		}
	}
	kmem_free((caddr_t)rl.rl_data, NFS_MAXPATHLEN);
	return (error);
}

/*ARGSUSED*/
static int
nfs_fsync(vp, flags, cred, waitfor)
	register struct vnode *vp;
	int flags;
	struct ucred *cred;
	int waitfor;
{
	register struct rnode *rp = vtor(vp);

	RLOCK(rp);
	if (rp->r_flags & RDIRTY) {
		/* MPC XXX Why would nfs_fsync not flush sync 
		   vflushbuf(vp, waitfor == MNT_WAIT ? B_SYNC : 0); 
		 */
		ubc_flush_dirty(vp, 0);
		waitforio(vp);
		rp->r_flags &= ~RDIRTY;
	}
	RUNLOCK(rp);
	return (rp->r_error);
}

/*
 * Synch a range of an open file.
 */
/* ARGSUSED */
nfs_syncdata(vp, fflags, start, length, cred)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
{
	return(nfs_fsync(vp, fflags, cred, MNT_WAIT));
}

/*
 * Weirdness: if the file was removed while it was open it got renamed
 * (by nfs_remove) instead.  Here we remove the renamed file.
 */
/*ARGSUSED*/
static int
nfs_inactive(vp)
	register struct vnode *vp;
{
	register struct rnode *rp;
	register struct nameidata *ndp;
	struct nfsdiropargs da;
	enum nfsstat status;
	int error = 0;
	struct rnode *unlrp;	/* Directory rnode */

	rp = vtor(vp);

	if (vp->v_usecount != 1) {
		return (0);
	}

redo:
	if ((ndp = rp->r_ndp) != NULL) {
		/*
		 * Lock down directory where unlinked-open file got renamed.
		 * This keeps a lookup from finding this rnode.
		 * Fix bug 1034328 - corbin
		 * Lock rnode down until we are finished doing the remove
		 * to prevent a race condition.
		 * The locking sequence is important here to prevent deadlock.
		 * Note that the RLOCK on the directory might have slept while
		 * someone else was doing a lookup on the file we're about to
		 * remove.  By locking the file itself, we can ensure that
		 * whoever it it is either done with it (v_usecount == 1),
		 * or has is still using it (v_usecount > 1).  In that case,
		 * someone else will deal with the cleanup later.
		 */
		unlrp = vtor(ndp->ni_dvp);
		RLOCK(unlrp);
		RLOCK(rp);

		if (vp->v_usecount != 1) {
			RUNLOCK(rp);
			RUNLOCK(unlrp);
			return(0);
		}

		if (rp->r_ndp == NULL) { /* I doubt this can happen */
			RUNLOCK(rp);
			RUNLOCK(unlrp);
			goto redo;
		}

		rp->r_flags &= ~RDIRTY;

		/*
		 * Do the remove operation on the renamed file
		 */
		setdiropargs(&da, ndp);
		error = rfscall(vtomi(ndp->ni_dvp), RFS_REMOVE,
		    xdr_diropargs, (caddr_t)&da,
		    xdr_enum, (caddr_t)&status, ndp->ni_cred);
		if (error == 0)
			error = geterrno(status);

		/*
		 * Release stuff held for the remove
		 */
		vrele(ndp->ni_dvp);
		rp->r_ndp = NULL;
		crfree(ndp->ni_cred);
		RUNLOCK(rp); /* Fix bug 1034328 */
		RUNLOCK(unlrp);
		kmem_free((caddr_t)ndp, sizeof(*ndp));
	}
	return (error);
}

int nfs_dnlc = 1;	/* use dnlc */

/*
 * Remote file system operations having to do with directory manipulation.
 */
/* ARGSUSED */
static int
nfs_lookup(vp, ndp)
	register struct vnode *vp;
	register struct nameidata *ndp;
{
	int error = 0;
	struct nfsdiropargs da;
	struct  nfsdiropres *dr;

	ASSERT(vp->v_type == VDIR);

	/*
	 * Before checking name cache, validate caches
	 */
	ndp->ni_vp = (struct vnode *)0;
	if (error = nfs_validate_caches(vp, ndp->ni_cred))
		return (error);

	RLOCK(vtor(vp));
	ndp->ni_dvp = vp;
	if ((error = cache_lookup(ndp)) && error != ENOENT) {
		struct vnode *vdp;
		int vpid;

		ASSERT(!(vp == ndp->ni_rdir && ndp->ni_isdotdot));
		vdp = ndp->ni_vp;
#if	UNIX_LOCKS
		vpid = ndp->ni_vpid;
#else
		vpid = vdp->v_id;
#endif	/* UNIX_LOCKS */
		if (vp == vdp) {
			VREF(vdp);
			error = 0;
		} else 
			error = vget_nowait(vdp);

		if (error == 0) {
			if (vpid == vdp->v_id) {
				VOP_ACCESS(vp, VEXEC, u.u_cred, error);
				if (error) {
					vrele(vdp);
					ndp->ni_vp = NULL;
				}
				RUNLOCK(vtor(vp));
				return(error);
			} else
				vrele(vdp);
		}
		ndp->ni_vp = (struct vnode *)0;
	}
	dr = (struct  nfsdiropres *)kmem_alloc(sizeof (*dr));
	setdiropargs(&da, ndp);
	error = rfscall(vtomi(vp), RFS_LOOKUP, xdr_diropargs,
	    (caddr_t)&da, xdr_diropres, (caddr_t)dr, ndp->ni_cred);
	if (error == 0) {
		error = geterrno(dr->dr_status);
		PURGE_STALE_FH(error, vp);
		if (error == 0) {
			int flag = ndp->ni_nameiop & OPFLAG;
			int wantparent = ndp->ni_nameiop & WANTPARENT;

			if (flag == DELETE &&
			    *ndp->ni_next == 0 &&
			    !bcmp(&vtor(vp)->r_fh, &dr->dr_fhandle, NFS_FHSIZE)) {
				VREF(vp);
				ndp->ni_vp = vp;
			} else if (flag == RENAME && 
				   wantparent && 
				   *ndp->ni_next == 0 &&
				   !bcmp(&vtor(vp)->r_fh, &dr->dr_fhandle, NFS_FHSIZE)) {
					error = EISDIR;
			} else if (!bcmp(&vtor(vp)->r_fh, &dr->dr_fhandle, NFS_FHSIZE)) {
				VREF(vp);
				ndp->ni_vp = vp;
			}
			if (error == 0) {
				if (ndp->ni_vp == (struct vnode *)0) {
					ndp->ni_vp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, vp->v_mount, (struct vattr *)0);
					if (ndp->ni_vp == (struct vnode *)0)
						error = ENFILE;
				}
				if (error == 0 && nfs_dnlc && ndp->ni_makeentry &&
			    	    (vtomi(ndp->ni_vp)->mi_noac == 0))
					cache_enter(ndp);
			}
		}
	}
	kmem_free((caddr_t)dr, sizeof (*dr));
	RUNLOCK(vtor(vp));
	return (error);
}

/*ARGSUSED*/
static int
nfs_create(ndp, va)
	register struct nameidata *ndp;
	register struct vattr *va;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

	ndp->ni_vp = (struct vnode *)0;

	dr = (struct  nfsdiropres *)kmem_alloc(sizeof (*dr));
	setdiropargs(&args.ca_da, ndp);

	/*
	 * Decide what the group-id of the created file should be.
	 * Set it in attribute list as advisory...then do a setattr
	 * if the server didn't get it right the first time.
	 */
	va->va_gid = (short) setdirgid(ndp->ni_dvp);

	/*
	 * This is a completely gross hack to make mknod
	 * work over the wire until we can wack the protocol
	 */
#define	IFCHR		0020000		/* character special */
#define	IFBLK		0060000		/* block special */
#define	IFSOCK		0140000		/* socket */
	if (va->va_type == VCHR) {
		va->va_mode |= IFCHR;
		va->va_size = (u_long)va->va_rdev;
	} else if (va->va_type == VBLK) {
		va->va_mode |= IFBLK;
		va->va_size = (u_long)va->va_rdev;
	} else if (va->va_type == VFIFO) {
		va->va_mode |= IFCHR;		/* xtra kludge for namedpipe */
		va->va_size = (u_long)NFS_FIFO_DEV;	/* blech */
	} else if (va->va_type == VSOCK) {
		va->va_mode |= IFSOCK;
	}

	vattr_to_sattr(va, &args.ca_sa);
	RLOCK(vtor(ndp->ni_dvp));
	cache_purge(ndp->ni_dvp);
	error = rfscall(vtomi(ndp->ni_dvp), RFS_CREATE, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, ndp->ni_cred);
	PURGE_ATTRCACHE(ndp->ni_dvp);	/* mod time changed */
	if (!error) {
		error = geterrno(dr->dr_status);
		if (!error) {
			gid_t gid;

			ndp->ni_vp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr,
			    ndp->ni_dvp->v_mount, va);
			if (ndp->ni_vp == (struct vnode *)0) {
				error = ENFILE;
				PURGE_STALE_FH(error, ndp->ni_dvp);
			} else {
				if (va->va_size == 0) {
					(vtor(ndp->ni_vp))->r_size = 0;
				}
				if (nfs_dnlc && ndp->ni_makeentry && 
				    (vtomi(ndp->ni_vp)->mi_noac == 0)) {
					/*
					 * ni_ptr needed in cache_enter;
					 * was freed in namei
					 */
					ndp->ni_ptr = &ndp->ni_dent.d_name[0];
					cache_enter(ndp);
				}
				/*
				 * Make sure the gid was set correctly.
				 * If not, try to set it (but don't lose
				 * any sleep over it).
				 */
				gid = va->va_gid;
				nattr_to_vattr(ndp->ni_vp, &dr->dr_attr, va);
				if (gid != va->va_gid) {
					struct vattr vattr;
	
					vattr_null(&vattr);
					vattr.va_gid = gid;
					(void) nfs_setattr(ndp->ni_vp, &vattr, ndp->ni_cred);
					va->va_gid = gid;
				}
			}
		} else {
			PURGE_STALE_FH(error, ndp->ni_dvp);
		}
	}
	RUNLOCK(vtor(ndp->ni_dvp));
	kmem_free((caddr_t)dr, sizeof (*dr));
	vrele(ndp->ni_dvp);
	return (error);
}

/*
 * This winds up being exactly like create, except that vfs expects the
 * vnode released.  While it looks like we can also do some of the special
 * device stuff here too, unp_bind() calls VOP_CREATE() to create a socket
 * and leave it open.
 */
static int
nfs_mknod(ndp, va)
	register struct nameidata *ndp;
	register struct vattr *va;
{
	int error;

	if (error = nfs_create(ndp, va))
		return error;
	vrele(ndp->ni_vp);
	return error;
}

/*
 * Weirdness: if the vnode to be removed is open
 * we rename it instead of removing it and nfs_inactive
 * will remove the new name.
 */
static int
nfs_remove(ndp)
	register struct nameidata *ndp;
{
	register struct vnode *vp = ndp->ni_vp;
	register struct vnode *dvp = ndp->ni_dvp;
	enum nfsstat status = NFS_OK;
	int error;

	RLOCK(vtor(dvp));
	cache_purge(vp);
	if ((vp->v_usecount > 1) && vtor(vp)->r_ndp == NULL) {
		struct nameidata *np = (struct nameidata *)kmem_alloc(sizeof(*np));
		struct nfsrnmargs args;

		bzero((caddr_t)np, sizeof(*np));
		np->ni_dvp = dvp;
		newname((caddr_t)np->ni_dent.d_name);
		np->ni_dent.d_namlen = strlen((caddr_t)np->ni_dent.d_name);

		cache_purge(dvp);
		setdiropargs(&args.rna_from, ndp);
		setdiropargs(&args.rna_to, np);
		error = rfscall(vtomi(dvp), RFS_RENAME, xdr_rnmargs, 
				(caddr_t)&args, xdr_enum, (caddr_t)&status, ndp->ni_cred);
		PURGE_ATTRCACHE(dvp);
		PURGE_ATTRCACHE(vp); /*XXX*/
		PURGE_STALE_FH(error ? error : geterrno(status), dvp); /*XXX*/
		if (error) {
			kmem_free((caddr_t)np, sizeof(*np));
		} else {
			VREF(dvp);
			np->ni_cred = crdup(ndp->ni_cred);
			vtor(vp)->r_ndp = np; /*DWS sync with nfs_inactive */
		}
	} else {
		struct nfsdiropargs da;

		ubc_invalidate(vp, 0, 0, 0);
		waitforio(vp);
		vtor(vp)->r_flags &= ~RDIRTY;
		setdiropargs(&da, ndp);
		error = rfscall(vtomi(dvp), RFS_REMOVE, xdr_diropargs,
			    (caddr_t)&da, xdr_enum, (caddr_t)&status, ndp->ni_cred);
		PURGE_ATTRCACHE(dvp);	/* mod time changed */
		PURGE_ATTRCACHE(vp);	/* link count changed */
		PURGE_STALE_FH(error ? error : geterrno(status), dvp);
	}

	RUNLOCK(vtor(dvp));
	vrele(vp);
	vrele(dvp);
	return (error ? error : geterrno(status));
}

static int
nfs_link(vp, ndp)
	struct vnode *vp;
	struct nameidata *ndp;
{
	int error;
	struct nfslinkargs args;
	enum nfsstat status;

	args.la_from = *vtofh(vp);
	setdiropargs(&args.la_to, ndp);
	RLOCK(vtor(ndp->ni_dvp));
	error = rfscall(vtomi(vp), RFS_LINK, xdr_linkargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, ndp->ni_cred);
	PURGE_ATTRCACHE(ndp->ni_dvp);	/* mod time changed */
	PURGE_ATTRCACHE(vp);		/* link count changed */
	RUNLOCK(vtor(ndp->ni_dvp));
	if (!error) {
		error = geterrno(status);
		PURGE_STALE_FH(error, vp);
		PURGE_STALE_FH(error, ndp->ni_dvp);
	}
	vrele(ndp->ni_dvp);
	return (error);
}

static int
nfs_rename(ondp, nndp)
	register struct nameidata *ondp, *nndp;
{
	int error;
	enum nfsstat status;
	struct nfsrnmargs args;

	RLOCK(vtor(ondp->ni_dvp));
	cache_purge(ondp->ni_dvp);
	cache_purge(nndp->ni_dvp);
	if (nndp->ni_dvp != ondp->ni_dvp) {
		RLOCK(vtor(nndp->ni_dvp));
	}
	setdiropargs(&args.rna_from, ondp);
	setdiropargs(&args.rna_to, nndp);
	error = rfscall(vtomi(ondp->ni_dvp), RFS_RENAME, xdr_rnmargs,
	    (caddr_t)&args, xdr_enum, (caddr_t)&status, ondp->ni_cred);
	PURGE_ATTRCACHE(ondp->ni_dvp);	/* mod time changed */
	PURGE_ATTRCACHE(nndp->ni_dvp);	/* mod time changed */
	RUNLOCK(vtor(ondp->ni_dvp));
	if (nndp->ni_dvp != ondp->ni_dvp) {
		RUNLOCK(vtor(nndp->ni_dvp));
	}
	if (!error) {
		error = geterrno(status);
		PURGE_STALE_FH(error, ondp->ni_dvp);
		PURGE_STALE_FH(error, nndp->ni_dvp);
	}
        vrele(nndp->ni_dvp);
        if (nndp->ni_vp)
                vrele(nndp->ni_vp);
        vrele(ondp->ni_dvp);
        vrele(ondp->ni_vp);
	return (error);
}

static int
nfs_mkdir(ndp, va)
	register struct nameidata *ndp;
	struct vattr *va;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

	dr = (struct  nfsdiropres *)
			kmem_alloc(sizeof (*dr));
	setdiropargs(&args.ca_da, ndp);

	/*
	 * Decide what the group-id and set-gid bit of the created directory
	 * should be.  May have to do a setattr to get the gid right.
	 */
	va->va_gid = (short) setdirgid(ndp->ni_dvp);
	va->va_mode = (u_short) setdirmode(ndp->ni_dvp, va->va_mode);

	vattr_to_sattr(va, &args.ca_sa);
	RLOCK(vtor(ndp->ni_dvp));
	cache_purge(ndp->ni_dvp);
	error = rfscall(vtomi(ndp->ni_dvp), RFS_MKDIR, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, ndp->ni_cred);
	PURGE_ATTRCACHE(ndp->ni_dvp);	/* mod time changed */
	RUNLOCK(vtor(ndp->ni_dvp));
	if (!error) {
		error = geterrno(dr->dr_status);
		PURGE_STALE_FH(error, ndp->ni_dvp);
	}
	if (!error) {
		gid_t gid;

		/*
		 * Due to a pre-4.0 server bug the attributes that come back
		 * on mkdir are not correct. Use them only to set the vnode
		 * type in makenfsnode.
		 */
		ndp->ni_vp = makenfsnode(&dr->dr_fhandle, &dr->dr_attr, ndp->ni_dvp->v_mount, (struct vattr *)0);
		if (ndp->ni_vp == (struct vnode *)0)
			error = ENFILE;
		else {
			PURGE_ATTRCACHE(ndp->ni_vp);

			if (nfs_dnlc && ndp->ni_makeentry && 
			    (vtomi(ndp->ni_vp)->mi_noac == 0)) {
				/*
				 * ni_ptr needed in cache_enter;
				 * was freed in namei
				 */
				ndp->ni_ptr = &ndp->ni_dent.d_name[0];
				cache_enter(ndp);
			}

			/*
			 * Make sure the gid was set correctly.
			 * If not, try to set it (but don't lose
			 * any sleep over it).
			 */
			gid = va->va_gid;
			nattr_to_vattr(ndp->ni_vp, &dr->dr_attr, va);
			if (gid != va->va_gid) {
				vattr_null(va);
				va->va_gid = gid;
				(void) nfs_setattr(ndp->ni_vp, va, ndp->ni_cred);
			}
		}
	} else {
		ndp->ni_vp = (struct vnode *)0;
	}
	kmem_free((caddr_t)dr, sizeof (*dr));
	vrele(ndp->ni_dvp);
	return (error);
}

static int
nfs_rmdir(ndp)
	register struct nameidata *ndp;
{
	int error;
	enum nfsstat status;
	struct nfsdiropargs da;

	if (ndp->ni_dvp == ndp->ni_vp) {
		vrele(ndp->ni_vp);
		vrele(ndp->ni_vp);
		return(EINVAL);
	}
	setdiropargs(&da, ndp);
	RLOCK(vtor(ndp->ni_dvp));
	cache_purge(ndp->ni_dvp);
	error = rfscall(vtomi(ndp->ni_dvp), RFS_RMDIR, xdr_diropargs, (caddr_t)&da,
	    xdr_enum, (caddr_t)&status, ndp->ni_cred);
	PURGE_ATTRCACHE(ndp->ni_dvp);	/* mod time changed */
	RUNLOCK(vtor(ndp->ni_dvp));
	if (!error) {
		error = geterrno(status);
		PURGE_STALE_FH(error, ndp->ni_dvp);
	}
	vrele(ndp->ni_dvp);
	vrele(ndp->ni_vp);
	return (error);
}

static int
nfs_symlink(ndp, tva, tnm)
        struct nameidata *ndp;
        struct vattr *tva;
        char *tnm;
{
	int error;
	struct nfsslargs args;
	enum nfsstat status;

	setdiropargs(&args.sla_from, ndp);
	vattr_to_sattr(tva, &args.sla_sa);
	args.sla_tnm = tnm;
	error = rfscall(vtomi(ndp->ni_dvp), RFS_SYMLINK, xdr_slargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, ndp->ni_cred);
	PURGE_ATTRCACHE(ndp->ni_dvp);	/* mod time changed */
	if (!error) {
		error = geterrno(status);
		PURGE_STALE_FH(error, ndp->ni_dvp);
	}
	vrele(ndp->ni_dvp);
	return (error);
}

/*
 * Read directory entries.
 * There are some weird things to look out for here.  The uio_offset
 * field is either 0 or it is the offset returned from a previous
 * readdir.  It is an opaque value used by the server to find the
 * correct directory block to read.  The byte count must be at least
 * vtoblksz(vp) bytes.  The count field is the number of blocks to
 * read on the server.  This is advisory only, the server may return
 * only one block's worth of entries.  Entries may be compressed on
 * the server.
 */
static int
nfs_readdir(vp, uiop, cred, eofflagp)
	struct vnode *vp;
	register struct uio *uiop;
	struct ucred *cred;
	int *eofflagp;
{
	int error = 0;
	struct iovec *iovp;
	unsigned alloc_count, count;
	struct nfsrddirargs rda;
	struct nfsrddirres  rd;
	struct rnode *rp = vtor(vp);

	ASSERT(vp->v_type == VDIR);

	*eofflagp = 0;
        if ((rp->r_lastr == (daddr_t)uiop->uio_offset) &&
	    (rp->r_flags & REOF) && 
	    (time.tv_sec < rp->r_attrtime.tv_sec)) {
		*eofflagp = 1;
		return (0);
	}
	iovp = uiop->uio_iov;
	alloc_count = count = MIN(iovp->iov_len, vtomi(vp)->mi_tsize);
	if (uiop->uio_iovcnt != 1) {
		return (EINVAL);
	}
	rda.rda_offset = (u_int)uiop->uio_offset;
	rd.rd_entries = (struct dirent *)kmem_alloc((u_int)alloc_count);
	rda.rda_fh = *vtofh(vp);
	do {
		count = MIN(count, vtomi(vp)->mi_curread);
		rda.rda_count = count;
		rd.rd_size = count;
		error = rfscall(vtomi(vp), RFS_READDIR, xdr_rddirargs,
				(caddr_t)&rda, xdr_getrddirres, (caddr_t)&rd,
				cred);
	} while (error == ENFS_TRYAGAIN);

	if (!error) {
		error = geterrno(rd.rd_status);
		PURGE_STALE_FH(error, vp);
	}
	if (!error) {
		/*
		 * move dir entries to user land
		 */
		if (rd.rd_size) {
			ASSERT(uiop->uio_rw == UIO_READ);
			error = uiomove((caddr_t)rd.rd_entries, (long)rd.rd_size, uiop);
			rda.rda_offset = rd.rd_offset;
			uiop->uio_offset = rd.rd_offset;
		}
		if (rd.rd_eof) {
			rp->r_flags |= REOF;
			rp->r_lastr = (daddr_t)uiop->uio_offset;
			*eofflagp = 1;
		}
	}
	kmem_free((caddr_t)rd.rd_entries, alloc_count);
	return (error);
}


struct buf async_bufhead;
int asyncdaemon_requests = 0;
int nfs_asyncdaemons = 0;

static int
nfs_strategy(bp)
	register struct buf *bp;
{
	register struct rnode *rp = vtor(bp->b_vp);
	int error = 0;

	ASSERT(((bp->b_flags & B_PHYS) == 0));
	/*
	 * If there was an asynchronous write error on this rnode
	 * then we just return the old error code. This continues
	 * until the error is returned to some user process.  This is
	 * a little problematic because there can be many procs
	 * writing this rnode.
	 */
	if (rp->r_error) {
		error = bp->b_error = rp->r_error;
		bp->b_flags |= B_ERROR;
		if (bp->b_flags & B_READ) {
			crfree(bp->b_rcred);
			bp->b_rcred = NOCRED;
		}
		biodone(bp);
		return(error);
	}
	/*
	 * Handle synchronous IO.
	 */
	if ((bp->b_flags & B_ASYNC) == 0)
		return(do_bio(bp));
	/*
	 * Handle asynchronous IO.
	 *	if the async_daemon is busy, handle as synchronous
	 */
	ASYNCD_LOCK();
	if (asyncdaemon_requests < nfs_asyncdaemons) {
		struct buf *dp = &async_bufhead;

		if (dp->b_actf == (struct buf *)0) {
			dp->b_actl = bp;
			bp->b_actf = dp;
		} else {
			dp->b_actf->b_actl = bp;
			bp->b_actf = dp->b_actf;
		}
		dp->b_actf = bp;
		bp->b_actl = dp;
		++asyncdaemon_requests;
		/* vref(bp->b_vp); */
		ASYNCD_UNLOCK();
		thread_wakeup_one((vm_offset_t)&nfs_asyncdaemons);
	} else {
		ASYNCD_UNLOCK();
		bp->b_flags &= ~B_ASYNC; /* Flag sync to return one error */
		return(do_bio(bp));
	}
	return(error);
}


nfs_async_daemon(p, args, retval)
        struct proc *p;
        void *args;
        int *retval;
{
	struct buf *bp, *dp = &async_bufhead;
	struct vnode *vp;
	int dying = 0;
	int error;
	thread_t th;
	char exitsig;

	/*
	 * Must be super user
	 */
	PRIV_SUSER(u.u_cred, &u.u_acflag, SEC_REMOTE, 0, EPERM, error);

	th = current_thread();
	ASYNCD_LOCK();
	if (nfs_asyncdaemons >= max_asyncdaemons) {
		ASYNCD_UNLOCK();
		NFSD_UNLOCK();
		return (EBUSY);
	}
	++nfs_asyncdaemons;
	ASYNCD_UNLOCK();	/* Need to drop because pmap_collect() can sleep */
	NFSD_UNLOCK();

	/*
	 * Try to discard pte's, mark not swappable.
	 */
	pmap_collect(vm_map_pmap(th->task->map));
	thread_swappable(th, FALSE);

	ASYNCD_LOCK();
	for (;;) {
		while (dp->b_actf == (struct buf *)0) {
			if (dying)
				goto out;
			assert_wait((vm_offset_t)&nfs_asyncdaemons, FALSE);
			ASYNCD_UNLOCK();
			thread_block();
			switch (th->wait_result) {
			      case THREAD_TIMED_OUT:
				panic("async_daemon:  thread timed out");
				/* NOTREACHED */
			      case THREAD_INTERRUPTED:
			      case THREAD_SHOULD_TERMINATE:
				/*
				 * We've been interrupted!
				 * Make ourselves unavailable but
				 * handle requests until the queue empties.
				 * Then exit, above.  (NB:  may want to change
				 * this behavior to handle no more than one
				 * additional request to prevent indefinite
				 * postponement problems.)
				 */
				if (!dying++) {
					NFSD_LOCK();
					ASYNCD_LOCK();
					--nfs_asyncdaemons;
					ASYNCD_UNLOCK();
					NFSD_UNLOCK();
				}
				break;
			      default:
				break;
			}
			ASYNCD_LOCK();
		}
		/*
		 * Other daemons could have emptied the list, so need
		 * another empty queue test.
		 */
		if ((bp = dp->b_actl) == (struct buf *)0)
			continue;
		if (bp->b_actl == dp) {
			dp->b_actf = dp->b_actl = (struct buf *)0;
		} else {
			dp->b_actl = bp->b_actl;
			bp->b_actl->b_actf = dp;
		}
		ASYNCD_UNLOCK();

		vp = bp->b_vp;
		(void) do_bio(bp);
		--asyncdaemon_requests;
		/* vrele(vp); */
		ASYNCD_LOCK();
	}
	
      out:
	exitsig = u.u_procp->p_cursig;
	ASYNCD_UNLOCK();
	thread_swappable(th, TRUE);
	return(0);
}

static int
do_bio(bp)
	register struct buf *bp;
{
	register struct rnode *rp = vtor(bp->b_vp);
	int error;

	if (bp->b_flags & B_READ) {
		error = bp->b_error = nfsread(bp->b_vp, bp->b_un.b_addr,
						(u_int)dbtob(bp->b_blkno), bp->b_bcount,
						&bp->b_resid, bp->b_rcred);
		crfree(bp->b_rcred);
		bp->b_rcred = NOCRED;
		if (!error) {
			if (bp->b_resid) {
				/*
				 * Didn't get it all because we hit EOF,
				 * zero all the memory beyond the EOF.
				 */
				bzero(bp->b_un.b_addr +
				    (bp->b_bcount - bp->b_resid),
				    (u_int)bp->b_resid);
			}
                        /*if (bp->b_resid == bp->b_bcount &&
                            dbtob(bp->b_blkno) >= rp->r_size) {
				/*
				 * We didn't read anything at all as we are
				 * past EOF.  Return an error indicator back
				 * but don't destroy the pages (yet).
				 */
				/*error = NFS_EOF;
			} */
		}
	} else {
		/*
		 * If the write fails and it was asynchronous
		 * a future write will return an error.
		 */
		if (rp->r_error == 0) {
			int count = MIN(bp->b_bcount, rp->r_size - dbtob(bp->b_blkno));
			if (count < 0) {
				panic("do_bio: write count < 0");
			}
			error = bp->b_error = nfswrite(bp->b_vp, 
			    bp->b_un.b_addr, (u_int)dbtob(bp->b_blkno),
			    count, !(bp->b_flags & B_ASYNC), rp->r_cred);
			if (error && bp->b_flags & B_ASYNC)
				rp->r_error = error;
		} else
			error = bp->b_error = rp->r_error;
	}

	if (error != 0 && error != NFS_EOF)
		bp->b_flags |= B_ERROR;

	/*
	 * Call biodone() to free the bp and pages.
	 */
	biodone(bp);
	return (error);
}

/*
 * Record-locking requests are passed to the local Lock-Manager daemon.
 */
static int
nfs_lockctl(vp, eld, flag, cred, clid, offset)
	struct vnode *vp;
	struct eflock *eld;
	int flag;
	struct ucred *cred;
	pid_t clid;
	off_t offset;
{
	lockhandle_t lh;
	int error;
	int cmd;
	struct eflock eld2;
	
	/*
	 * NFS does not support mandatory locking
	 */
	if (flag & VNOFLCK)
		return(0);

	lh.lh_vp = vp;
	lh.lh_servername = vtomi(vp)->mi_hostname;
	bcopy((caddr_t)vtofh(vp), (caddr_t)&lh.lh_id, NFS_FHSIZE);

	if (flag & CLNFLCK) {
		eld2.l_type = F_UNLCK;    /* set to unlock entire file */
		eld2.l_whence = 0;  /* unlock from start of file */
		eld2.l_start = 0L;
		eld2.l_len = 0L;           /* do entire file */
		eld2.l_rpid= 0;
		eld2.l_rsys= 0;
		error = klm_lockctl(&lh, &eld2, F_SETLK, cred, clid, 0);
		return(error);
	}

	if ((flag & (GETFLCK|RGETFLCK|SETFLCK|RSETFLCK)) != 0) {
		if (eld->l_len == MAXEND)  /* use 32 bit (NFS) MAXEND */
			eld->l_len = 0x7fffffff;
					/* NLM and KLM protocols are 32 bit */
		if ((eld->l_len > 0L) &&
		    ((eld->l_start + eld->l_len - 1L) > 0x7fffffff))
			return(EINVAL);
	} else
		return(EINVAL);

        /*
         * If we are setting a lock mark the rnode RNOCACHE so the page
         * cache does not give inconsistent results on locked files shared
         * between clients.  The RNOCACHE flag is never turned off as long
         * as the vnode is active because it is hard to figure out when the
         * last lock is gone.
         * XXX - what if someone already has the vnode mapped in?
         */

        if (((vtor(vp)->r_flags & RNOCACHE) == 0) &&
            (eld->l_type != F_UNLCK) && ((flag & GETFLCK) == 0)) {
                vtor(vp)->r_flags |= RNOCACHE;
		ubc_invalidate(vp, 0, 0, 0);
                PURGE_ATTRCACHE(vp);
        }

	if (flag & GETFLCK)
		cmd = F_GETLK;

	else if (flag & RGETFLCK)
		cmd = F_RGETLK;

	else if (flag & SETFLCK) {
		if (flag & SLPFLCK)
			cmd = F_SETLKW;
		else
			cmd = F_SETLK;
	}

	else if (flag & RSETFLCK) { 
		if (flag & SLPFLCK)
			cmd = F_RSETLKW;
		else
			cmd = F_RSETLK;
	}

	error = klm_lockctl(&lh, eld, cmd, cred, clid, 0);

	if (!error && flag & SETFLCK && eld->l_type != F_UNLCK) {
		VN_LOCK(vp);
		vp->v_flag |= VLOCKS; /* Tell closef to ensure locks are freed */
		VN_UNLOCK(vp);
	}

        if (eld->l_len == 0x7fffffff)	/* use 64 bit (Alpha) MAXEND */
                eld->l_len = MAXEND;

	return (error);
}

sync_vp(vp)
	struct vnode *vp;
{
	if (vtor(vp)->r_flags & RDIRTY)
		flush_vp(vp);
}

flush_vp(vp)
	struct vnode *vp;
{
	ubc_flush_dirty(vp, 0);
	waitforio(vp);
	vtor(vp)->r_flags &= ~RDIRTY;
}

waitforio(vp)
	struct vnode *vp;
{
	int s;

	s = splbio();
	VN_OUTPUT_LOCK(vp);
	while (vp->v_numoutput) {
		vp->v_outflag |= VOUTWAIT;
		assert_wait((vm_offset_t)&vp->v_numoutput, FALSE);
		VN_OUTPUT_UNLOCK(vp);
		thread_block();
		VN_OUTPUT_LOCK(vp);
	}
	VN_OUTPUT_UNLOCK(vp);
	(void) splx(s);
}


/*
 * Convert from file system blocks to device blocks
 */
static int
nfs_bmap(vp, bn, bnp)
	struct vnode *vp;	/* file's vnode */
	daddr_t bn;		/* fs block number */
	daddr_t *bnp;		/* RETURN device block number */
{
	int bsize;		/* server's block size in bytes */

	if (bnp) {
		bsize = vtoblksz(vp);
		*bnp = bn * (bsize / DEV_BSIZE);
	}
	return (0);
}

static int
nfs_print(vp)
        struct vnode *vp;
{
	printf("rnode %x\n", vtor(vp));
	return(0);
}

static int
nfs_page_read(vp, uio, cred)
        struct vnode *vp;
        struct uio *uio;
        struct ucred *cred;
{
        int error;

        error = nfs_rdwr(vp, uio, 0, cred);
        if (error) {
                printf("error %d on pagein (nfs_rdwr)\n", error);
                error = EIO;
        }
        return(error);
}

static int
nfs_page_write(vp, uio, cred, pager, offset)
        struct vnode *vp;
        struct uio *uio;
        struct ucred *cred;
        memory_object_t pager;
        vm_offset_t offset;
{
        int error;

        error = nfs_rdwr(vp, uio, 0, cred);
        if (error) {
                printf("error %d on pageout (nfs_rdwr)\n", error);
                error = EIO;
        }
        return(error);
}


static int
nfs_reclaim(vp)
        register struct vnode *vp;
{
	register struct rnode *rp = vtor(vp);
	ASSERT(vp->v_usecount == 0);

	cache_purge(vp);
	rp_rmhash(rp);
	rfree(rp);
	return(0);
}

/* ARGSUSED */
static int
nfs_seek(vp, oldoff, newoff, cred)
        struct vnode *vp;
        off_t oldoff, newoff;
        struct ucred *cred;
{
	if (newoff > 0x7fffffff)	/* 2 GB NFS otw limit */
		return(EINVAL);
	return(((int)newoff < 0) ? EINVAL : 0);
}

/*
 * getattr wrapper for fifos.
 */
static int
nfsfifo_getattr(vp, vap, cred)
        struct vnode *vp;
        register struct vattr *vap;
        struct ucred *cred;
{
        int error;

        if (error = nfs_getattr(vp, vap, cred))
                return(error);
        return(fifo_getattr(vp, vap, cred));
}

static int
nfsspec_reclaim(vp)
        register struct vnode *vp;
{
	int error;		/* Winds up always being a 0.... */

	if (!(error = spec_reclaim(vp)))
		error = nfs_reclaim(vp);
	return error;
}

int
nfs_noop()
{
	return(0);
}

static int
nfs_mmap(register struct vnode *vp,
	vm_offset_t offset,
	vm_map_t map,
	vm_offset_t *addrp,
	vm_size_t len,
	vm_prot_t prot,
	vm_prot_t maxprot,
	int flags,
	struct ucred *cred)
{
        struct vp_mmap_args args;
        register struct vp_mmap_args *ap = &args;
        extern kern_return_t u_vp_create();

        ap->a_offset = offset;
        ap->a_vaddr = addrp;
        ap->a_size = len;
        ap->a_prot = prot,
        ap->a_maxprot = maxprot;
        ap->a_flags = flags;
        return u_vp_create(map, vp->v_object, (vm_offset_t) ap);
}

static int
nfs_getpage(struct vnode *vp,
	register vm_offset_t offset,
	vm_size_t len,
	vm_prot_t *protp,
	vm_page_t *pl,
	register int plsz,
	vm_map_entry_t ep,
	vm_offset_t addr,
	int rw,
	struct ucred *cred)
{
	register struct rnode *rp = vtor(vp);
	int error;
	register vm_offset_t end;
	int async = 0;
	int fsbsize = vtoblksz(vp);

        /*
         * protp is an array of protections for each page returned
         * We can disallow write access by marking an array element
         * with VM_PROT_WRITE.  This is a one to one correspondence
         * between the pl array and the protp array.
         */

	if (ep)
		RLOCK(rp);

	if ((!rw && cred != rp->r_cred) || /* Writing with a different cred */
		(rw && rp->r_cred == NULL)) { /* or reading and no better idea */
		crhold(cred);
		if (rp->r_cred) {
			crfree(rp->r_cred);
		}
		rp->r_cred = cred;
	}

	/*
	 * Hate this.  Will add function to simply get 
	 * pages when don't need to read into them.  
	 * This function will be called back in rwvp (getblk)
	 */
	if ((!rw) && (len == fsbsize))
		async = B_ASYNC;

        error = nfs_getapage(vp, offset, len, protp, pl, ep, rw, async, cred);
	if (error) {
		*pl = VM_PAGE_NULL;
		goto done;
	}
	else if ((((offset & page_mask) + len) <= PAGE_SIZE) && plsz == 1)
                goto done;
	else pl++;
	
	if (ep) {
	}
	else {
		len += (offset & page_mask);
		offset = trunc_page(offset);
                for (end = offset + len, offset += PAGE_SIZE, len -= PAGE_SIZE;
                        offset < end;
                        offset += PAGE_SIZE, pl++, len -= PAGE_SIZE) {
                        if (*pl != VM_PAGE_NULL)
				continue;
                        else {
				if (error = nfs_getapage(vp, offset, len,
                                protp, pl, ep, rw, async, cred)) {
                                	*pl = VM_PAGE_NULL;
                                	goto done;
				}
                        }
                }
	}
done:
	if (ep)
		RUNLOCK(rp);

	return(error);
}

nfs_getapage(struct vnode *vp,
        register vm_offset_t offset,
        vm_size_t len,
        vm_prot_t *protp,
        vm_page_t *pl,
        vm_map_entry_t ep,
        int rw, 
	int async,
        struct ucred *cred)
{
        vm_page_t pp, kpl, kpp;
        vm_page_t rapp, rakpl, rakpp;
        int uflags, fsbsize, error;
	struct rnode *rp = vtor(vp);
	daddr_t dblkno, bn;
	int khold, kplcnt;

        fsbsize = vtoblksz(vp);
        fsbsize &= ~(DEV_BSIZE - 1);
	bn = offset / fsbsize;
	/*
	 * Just get me the page
	 */
	if (async) {
		uflags = B_CACHE;
		error = ubc_lookup(vp, trunc_page(offset), (vm_size_t)fsbsize, 
			len+offset, &pp, &uflags);

		if (error)
			goto failed;

		if (uflags & B_DIRTY) {
			dbg_dirty_write++;
		}
		if (uflags & B_NOCACHE) {
			uflags = 0;
			error = ubc_page_alloc(vp, trunc_page(offset), 
				fsbsize, len+offset, &pp, &uflags);
			if (error)
				goto failed;
		}
	}
	/*
	 * reading or preread for partial block write 
	 */
	else {
		/*
		 * If should do read ahead 
		 */
		if ((rp->r_lastr + 1 == bn) &&
			(offset % fsbsize == 0) &&
			((offset+fsbsize) < rp->r_size)) { 
			rapp = ubc_kluster(vp, (vm_page_t) 0, 
				(offset+fsbsize) & ~(fsbsize-1), fsbsize, 
				B_CACHE|B_WANTFREE|B_BUSY, UBC_HNONE, &kplcnt);

			if (rapp) {
				error = nfs_rwblk(vp, rapp, kplcnt, 
					B_READ | B_ASYNC, (daddr_t) 0, 
					offset+fsbsize, cred);
				if (error) {
					*pl = VM_PAGE_NULL;
					return error;
				}
				read_ahead++;
			}
			else
				not_read_ahead++;
		}

		if (ubc_incore(vp, offset, len))
			(void) nfs_validate_caches(vp, cred);

		uflags = 0;
		error = ubc_lookup(vp, trunc_page(offset), (vm_size_t)fsbsize, 
			len+offset, &pp, &uflags);
		if (error)
			goto failed;
		if (uflags & B_DIRTY) {
			if (rw)
				dbg_dirty_read++;
			else
				dbg_dirty_pwrite++;
		}

		if (uflags & B_NOCACHE) {
                        if(ep && current_thread())
                                 current_thread()->thread_events.pageins++;
			if (fsbsize == PAGE_SIZE) {
				error = nfs_rwblk(vp, pp, 1, B_READ, 
					(daddr_t) 0, offset, cred);
				if (error) goto failed;
			}
			else if (fsbsize > PAGE_SIZE) {
				register vm_offset_t kstart;
				register vm_size_t ksize;
	
				kstart = pp->pg_offset & ~(fsbsize - 1);
				ksize = fsbsize;
				khold = UBC_HNONE;

				kpl = ubc_kluster(vp, pp, kstart, 
					ksize, B_CACHE|B_WANTFREE|B_BUSY, 
					khold, &kplcnt);
				kpp = kpl;

                                error = nfs_rwblk(vp, kpl, kplcnt, B_READ, 
					(daddr_t) 0 , offset, cred);
                                if (error)
					goto failed;
                        }
			else panic("nfs_getapage: fs size < PAGE_SIZE");
		}
		rp->r_lastr = bn;
	}
        if (protp != NULL) {
        	/* Disallow write to a memory mapper
         	* until UBC knows the page is dirty.
         	*/
        	if ((uflags & B_DIRTY) == 0)  {
                	if (rw)
				*protp = VM_PROT_WRITE;
                	else {
                        	ubc_page_dirty(pp);
                        	*protp = 0;
                	}
        	}
        	else *protp = 0;
	}

        /*
         * Return the page in the page list.
         * Also mark the end of the list null.
         */

done:
        *pl = pp;
        *(pl + 1) = VM_PAGE_NULL;
        return(0);

failed:
	*pl = VM_PAGE_NULL;
        if (pp != VM_PAGE_NULL)
                ubc_page_release(pp, 0);
        return error;
}




static int
nfs_putpage(register struct vnode *vp,
	register vm_page_t *pl,
	register int pcnt,
	int flags,
	struct ucred *cred)
{
	register struct rnode *rp=vtor(vp);
	register vm_page_t pp, plp;
	int size;
	int error, plcnt;

	if (flags & B_UBC) {
		VN_LOCK(vp);
		if (vp->v_flag & VXLOCK) {
			VN_UNLOCK(vp);
			while (pcnt--) {
				ubc_page_release(*pl, B_DONE|B_DIRTY);
				*pl++ = VM_PAGE_NULL;
			}
			return 0;
		}
		else VN_UNLOCK(vp);
	}
	else if (flags & B_MSYNC) {
		/* 
		 * XXX lebel - Where is the attr cache getting updated
		 * with the new times?
		 */
	}

	size = vtoblksz(vp);
	size &= ~(DEV_BSIZE - 1);
	if (size <= 0) {
		panic("rwvp: zero size");
	}

	if (rp->r_cred == NULL) {
		crhold(cred);
		rp->r_cred = cred;
	}

	/*
	 * Push each page and mark a successful push as a null page.
	 */

	while (pcnt--) {
		pp = *pl;
		plp = ubc_dirty_kluster(vp, pp, 
				pp->pg_offset & ~(size - 1),
				size, flags, FALSE, &plcnt);
		error = nfs_rwblk(vp, plp, plcnt, flags, (daddr_t) 0, 
			pp->pg_offset, cred);
		if (error) return error;
		else *pl++ = VM_PAGE_NULL;
	}
	return 0;
}

static int
nfs_swap(register struct vnode *vp,
        vp_swap_op_t swop,
        vm_offset_t args)
{
        return EIO;
}

nfs_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

nfs_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}
