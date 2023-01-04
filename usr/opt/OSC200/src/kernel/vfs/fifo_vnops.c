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
static char	*sccsid = "@(#)$RCSfile: fifo_vnops.c,v $ $Revision: 4.3.18.3 $ (DEC) $Date: 1993/09/09 22:05:13 $";
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
 * FIFO (named pipe) vnode operations
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/unpcb.h>
#include <sys/poll.h>
#include <kern/macro_help.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>

/*
 * We synchronize fifo_open and fifo_close to avoid several races.
 * Some races may only occur in a multiprocessor envirnoment.
 * But there are also races in these two functions that can also
 * occur in the uniprocessor case.  We must avoid races which occur when
 * several threads are racing to open and close FIFOs.  We do not have
 * a convenient data structure to lock and we don't want to add a
 * blocking lock to the vnode to be used just by FIFOs.  So we use
 * two flags in the vnode (VFLOCK and VFWAIT) for synchronization.
 * The vnode v_rdcnt, v_wrcnt, and v_socket fields for FIFOs are only
 * changed in fifo_open and fifo_close under VFLOCK.
 */ 

#define FIFO_LOCK(vp)						\
MACRO_BEGIN							\
	VN_LOCK(vp);						\
	while (vp->v_flag & VFLOCK) {				\
		vp->v_flag |= VFWAIT;				\
		assert_wait((vm_offset_t)&FIFO_WRITE_SOCKET(vp), FALSE);\
		VN_UNLOCK(vp);					\
		(void)tsleep((caddr_t)0, PZERO, "fifo_lock", 0);\
		VN_LOCK(vp);					\
	}							\
	vp->v_flag |= VFLOCK;					\
	VN_UNLOCK(vp);						\
MACRO_END

#define FIFO_UNLOCK(vp)						\
MACRO_BEGIN							\
	VN_LOCK(vp);						\
	if (vp->v_flag & VFWAIT) {				\
		vp->v_flag &= ~(VFWAIT|VFLOCK);			\
		VN_UNLOCK(vp);					\
		wakeup((caddr_t)&FIFO_WRITE_SOCKET(vp));	\
	} else {						\
		vp->v_flag &= ~VFLOCK;				\
		VN_UNLOCK(vp);					\
	}							\
MACRO_END

/*
 * The FIFO_LOCK and open reference protect connections.
 */
#define FIFO_WRITE_SOCKET(vp) ((vp)->v_socket)
#define FIFO_READ_SOCKET(vp)  (sotounpcb((vp)->v_socket)->unp_conn->unp_socket)

/*
 * In order to lock a socket, we must go through the socket funnel first.
 * This looks like a lot, but it's usually just an spl or a no-op.
 */
#define FIFO_LOCK_SOCKET(so)			\
	DOMAIN_FUNNEL_DECL(f)			\
	DOMAIN_FUNNEL(sodomain(so), f);		\
	SOCKET_LOCK(so)				\

#define FIFO_UNLOCK_SOCKET(so)			\
	SOCKET_UNLOCK(so);			\
	DOMAIN_UNFUNNEL(f)			\

#define FIFO_CANSEND(so)			\
MACRO_BEGIN					\
	FIFO_LOCK_SOCKET(so);			\
	(so)->so_state &= ~SS_CANTSENDMORE;	\
	FIFO_UNLOCK_SOCKET(so);			\
MACRO_END

#define FIFO_CANTSEND(so)			\
MACRO_BEGIN					\
	FIFO_LOCK_SOCKET(so);			\
	socantsendmore(so);			\
	FIFO_UNLOCK_SOCKET(so);			\
MACRO_END

#define FIFO_CANRECV(so)			\
MACRO_BEGIN					\
	FIFO_LOCK_SOCKET(so);			\
	(so)->so_state &= ~SS_CANTRCVMORE;	\
	FIFO_UNLOCK_SOCKET(so);			\
MACRO_END

#define FIFO_CANTRECV(so)			\
MACRO_BEGIN					\
	FIFO_LOCK_SOCKET(so);			\
	socantrcvmore(so);			\
	FIFO_UNLOCK_SOCKET(so);			\
MACRO_END

int fifo_open();
int fifo_close();
int fifo_read();
int fifo_write();
int fifo_ioctl();
int fifo_getattr();
int fifo_select();
int fifo_print();
int fifo_bread();
int fifo_brelse();

extern int spec_lookup();
extern int spec_badop();
extern int spec_ebadf();
extern int spec_nullop();
extern int spec_seek();
extern int spec_lockctl();

struct vnodeops fifo_vnodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	fifo_open,		/* open */
	fifo_close,		/* close */
	spec_ebadf,		/* access */
	fifo_getattr,		/* getattr */
	spec_ebadf,		/* setattr */
	fifo_read,		/* read */
	fifo_write,		/* write */
	fifo_ioctl,		/* ioctl */
	fifo_select,		/* select */
	spec_badop,		/* mmap */
	spec_nullop,		/* fsync */
	spec_seek,		/* seek */
	spec_badop,		/* remove */
	spec_badop,		/* link */
	spec_badop,		/* rename */
	spec_badop,		/* mkdir */
	spec_badop,		/* rmdir */
	spec_badop,		/* symlink */
	spec_badop,		/* readdir */
	spec_badop,		/* readlink */
	spec_badop,		/* abortop */
	spec_nullop,		/* inactive */
	spec_nullop,		/* reclaim */
	spec_ebadf,		/* bmap */
	spec_ebadf,		/* strategy */
	fifo_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_badop,		/* swap */
	fifo_bread,		/* buffer read */
	fifo_brelse,		/* buffer release */
	spec_lockctl,		/* lockctl */
	spec_nullop,		/* fsync byte range */
};


/*
 * Open a fifo.
 * Check whether a socket pair already exists for this fifo.  If
 * so, use it, otherwise create one.
 */
/*ARGSUSED*/
int
fifo_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	register struct unpcb *unp;
	struct socket *rso;
	struct socket *wso;
	register struct vnode *vp = *vpp;
	thread_t	th = current_thread();
	int error;

	FIFO_LOCK(vp);
	if ((wso = FIFO_WRITE_SOCKET(vp)) == NULL) {
		/* No readers or writers yet.
		 *
		 * Since this is a one-way pipe, it seems wasteful
		 * to allocate to socket structures.  One socket never
		 * uses its receive queue, and the other never uses its
		 * send queue.  I would prefer to allocate one and
		 * have unp_conn refer to itself.
		 * However, I'm not sure whether the socket locking
		 * can handle this, so for now I'll be safe and use
		 * two sockets.
		 *
		 * (Actually, there are other reasons, all of them
		 *  uipc socket internals. Two sockets are best.)
		 */
		if (error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0)) {
			FIFO_UNLOCK(vp);
			return(error);
		}

		if (error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0)) {
			(void)soclose(wso);
			FIFO_UNLOCK(vp);
			return(error);
		}

		/*
		 * We shouldn't need to lock these sockets because we
		 * just created them.
		 */
		wso->so_special |= SP_WATOMIC;	/* Atomic behavior like pipes */
		if (error = soconnect2(wso, rso)) {
			(void)soclose(wso);
			(void)soclose(rso);
			FIFO_UNLOCK(vp);
			return(error);
		}
		wso->so_state |= SS_CANTSENDMORE|SS_CANTRCVMORE;
		rso->so_state |= SS_CANTSENDMORE|SS_CANTRCVMORE;
		FIFO_WRITE_SOCKET(vp) = wso;
	}
	error = 0;
	rso = FIFO_READ_SOCKET(vp);
	if (mode & FREAD) {
		if (vp->v_rdcnt++ == 0) {
			FIFO_CANSEND(wso);
			/* Wake up any waiting writers */
			if (vp->v_wrcnt)
				wakeup((caddr_t) &vp->v_rdcnt);
		}
	}
	if (mode & FWRITE) {
		/* was (mode & FNDELAY) */
		if ((vp->v_rdcnt == 0) && (mode & (FNDELAY|FNONBLOCK))) {
			/*
			 * We need to temporarily increment the vnode
			 * writer count here because fifo_close will
			 * decrement the count.
			 */
			vp->v_wrcnt++;
			error = ENXIO;
			goto out;
		}
		if (vp->v_wrcnt++ == 0) {
			FIFO_CANRECV(rso);
			/* Wake up any waiting readers */
			if (vp->v_rdcnt)
				wakeup((caddr_t) &vp->v_wrcnt);
		}
	}
	if (mode & FREAD) {
		/*
		 * If FNDELAY is set or there is data in the pipe,
		 * return.
		 */
		/* was (mode & FNDELAY) */
		if ((vp->v_wrcnt == 0) && (mode & (FNDELAY|FNONBLOCK)))
			goto out;
		while (vp->v_wrcnt == 0) {
			int rstat;
			FIFO_LOCK_SOCKET(rso);
			SOCKBUF_LOCK(&rso->so_rcv);
			/*
			 * Block if there is no data in the pipe.
			 * Can't use soreadable() here as it will say
			 * readable if no writers.
			 */
			rstat = (rso->so_rcv.sb_cc > 0);
			if (rstat == 0)
				assert_wait((vm_offset_t)&vp->v_wrcnt, TRUE);
			SOCKBUF_UNLOCK(&rso->so_rcv);
			FIFO_UNLOCK_SOCKET(rso);
			FIFO_UNLOCK(vp);
			if (rstat || (error = tsleep((caddr_t)0,
					(PZERO+1)|PCATCH, "fifo_ropen", 0)))
				goto out2;
			FIFO_LOCK(vp);
		}
	}
	if (mode & FWRITE) {
		while (vp->v_rdcnt == 0) {
			assert_wait((vm_offset_t)&vp->v_rdcnt, TRUE);
			FIFO_UNLOCK(vp);
			error = tsleep((caddr_t)0, (PZERO+1)|PCATCH,
					"fifo_wopen", 0);
			if (error)
				goto out2;
			FIFO_LOCK(vp);
		}
	}
out:
	FIFO_UNLOCK(vp);
out2:
	if (error)
		(void) fifo_close(vp, mode & FMASK, cred);
	return (error);
}

/*
 * Close called
 */
/* ARGSUSED */
fifo_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct socket *wso = FIFO_WRITE_SOCKET(vp);
	struct socket *rso;

	if (wso == NULL)
		return (0);
	FIFO_LOCK(vp);
	rso = FIFO_READ_SOCKET(vp);
	if (fflag & FREAD) {
		ASSERT(vp->v_rdcnt > 0);
		if (--vp->v_rdcnt == 0)
			FIFO_CANTSEND(wso);
	}
	if (fflag & FWRITE) {
		ASSERT(vp->v_wrcnt > 0);
		if (--vp->v_wrcnt == 0)
			FIFO_CANTRECV(rso);
	}
	if (vp->v_wrcnt == 0 && vp->v_rdcnt == 0) {
		(void)soclose(wso);
		(void)soclose(rso);
		FIFO_WRITE_SOCKET(vp) = NULL;
	}
	FIFO_UNLOCK(vp);
	return (0);
}

/*
 * Vnode op for reading.
 */
/* ARGSUSED */
fifo_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct socket *wso = FIFO_WRITE_SOCKET(vp);
	register struct unpcb *unp;
	register struct socket *rso;
	int flags = 0;
	
	if (wso == NULL)
		panic("fifo_read: no sock");
	if (uio->uio_resid == 0)
		return (0);
	unp = sotounpcb(wso);
	if (unp->unp_conn == NULL || (rso = unp->unp_conn->unp_socket) == NULL) 
		panic("fifo_read: not connected");
	if (ioflag & (IO_NDELAY|IO_NONBLOCK))
		flags = MSG_NONBLOCK;
	return soreceive(rso, (struct mbuf **)0,
		uio, (struct mbuf **)0, (struct mbuf **)0, &flags);
}

/*
 * Vnode op for writing.
 */
/* ARGSUSED */
fifo_write(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct socket *wso = FIFO_WRITE_SOCKET(vp);
	int flags = 0;
	
	if (wso == NULL)
		panic("fifo_write: no sock");
	if (ioflag & (IO_NDELAY|IO_NONBLOCK))
		flags = MSG_NONBLOCK;
	return sosend(wso, (struct mbuf *)0,
		uio, (struct mbuf *)0, (struct mbuf *)0, flags);
}

/*
 * Vnode op for ioctl.
 */
/* ARGSUSED */
fifo_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	unsigned int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{
	register struct socket *wso = FIFO_WRITE_SOCKET(vp);
	register struct unpcb *unp;
	struct file ftemp;

	if (com == FIONBIO)	/* Handled each r/w */
		return (0);
	if (fflag & FREAD) {
                unp = sotounpcb(wso);
                if (unp->unp_conn == NULL ||
		    (ftemp.f_data = (caddr_t)unp->unp_conn->unp_socket) == NULL)
                        panic("fifo_ioctl: not connected");
	} else
		ftemp.f_data = (caddr_t)wso;
	return (soo_ioctl(&ftemp, com, data));
}

/*
 * Vnode getattr op.  Just fill in the fields that we know about.
 */
/* ARGSUSED */
fifo_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	register struct socket *wso = FIFO_WRITE_SOCKET(vp);
	struct stat sb;
	int error;

	if (wso == NULL) {
		vap->va_size = vap->va_bytes = 0;
		vap->va_blocksize = 0;
	} else {
		if (error = soo_stat(wso, &sb))
			return (error);
		vap->va_size = vap->va_bytes = sb.st_size;
		vap->va_blocksize = sb.st_blksize;
	}
#if !__alpha
	vap->va_size_rsv = 0;
#endif
	return(0);
}

/*
 * Vnode select op.  Just use soo_select to do the work.
 * Be sure to select the right side for the right thing(s).
 */
/* ARGSUSED */
fifo_select(vp, events, revents, scanning, cred)
        struct vnode *vp;
        short *events, *revents;
        int scanning;
        struct ucred *cred;     /* unused */
{
        struct socket *rso, *wso = FIFO_WRITE_SOCKET(vp);
        struct file ftemp;
        short rsev, wsev, rsrev, wsrev;
        int error;

        if (wso == NULL)
                return 0;
        rso = FIFO_READ_SOCKET(vp);
        /* POLLPRI and POLLHUP events don't occur on these fifo's */
        rsev = *events & (POLLNORM);
        wsev = *events & (POLLOUT);
        if (!scanning) {
	   if (wsev) {
                ftemp.f_data = (caddr_t)wso;
                (void) soo_select(&ftemp, &wsev, (short *)0, 0);
	   }
	   if (rsev) {
                ftemp.f_data = (caddr_t)rso;
                (void) soo_select(&ftemp, &rsev, (short *)0, 0);
	   }
        } else {
           rsrev = wsrev = 0;
	   if (wsev) {
                ftemp.f_data = (caddr_t)wso;
                if (error = soo_select(&ftemp, &wsev, &wsrev, scanning))
                       return error;
	   }
	   if (rsev) {
                ftemp.f_data = (caddr_t)rso;
                if (error = soo_select(&ftemp, &rsev, &rsrev, scanning))
                       return error;
	   }
           *revents = rsrev|wsrev;
        }
        return (0);
}

/*
 * Print out the contents of a fifo.
 */
fifo_print(vp)
	struct vnode *vp;
{
	/*printf("Nothing to say about this fifo");*/
	return(0);
}

fifo_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

fifo_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}
