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
static char *rcsid = "@(#)$RCSfile: str_filesys.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/08/02 20:41:20 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/*
 *	File: str_filesys.c - interface between STREAMS and file system
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/time.h>	/* vnode.h needs this ... */
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/user.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>

/*
 * List of routines in this file
 *
 * fd_to_cookie   - translate a fd to a file cookie
 * fd_alloc       - allocate a file desciptor for given file cookie
 * cookie_destroy - deallocate a file cookie
 * sth_fd_to_sth  - find and return fd associated with fd
 *
 *
 * fd_to_cookie, fd_alloc, cookie_destroy
 *
 * are needed to implement the passing of file descriptors between
 * processes via STREAMS pipes using I_SENDFD / I_RECVFD operations.
 * For each call to fd_to_cookie(), there will be exactly one call
 * to fd_alloc() or cookie_destroy(). Should the fd_alloc() fail,
 * then there will be another call to cookie_destroy().
 *
 * However, whether and when the second call happens, depends on
 * the behavior (and correctness) of the application programs.
 * The STREAMS framework can only ensure that cookie_destroy() gets
 * called when the associated message gets discarded.
 */

/*
 *	fd_to_cookie
 *
 *	Given a fd in the current process context (unchecked user
 *	parameter!), translate it into a file cookie. Regard this
 *	as an additional reference to this file, so that it does not
 *	get closed while we hold this reference.
 *
 *	Parameters:
 *	fd		(in)  - file descriptor to translate
 *	cookie		(out) - translated file descriptor
 *
 *	Return value          - error condition
 *				EBADF - invalid file descriptor
 */

int
fd_to_cookie (fd, cookie)
	int			fd;
	struct file_cookie *	cookie;
{
        struct ufile_state *ufp = &u.u_file_state;

        U_FDTABLE_LOCK(ufp);
        if (fd < 0 || fd > ufp->uf_lastfile) {
                U_FDTABLE_UNLOCK(ufp);
                return EBADF;
        }

        if ( (cookie->fc_fp = U_OFILE(fd, ufp)) == NULL
           || cookie->fc_fp == U_FD_RESERVED ) {
                U_FDTABLE_UNLOCK(ufp);
                return EBADF;
        }
        FP_REF(cookie->fc_fp);
        cookie->fc_flags = U_POFILE(fd, ufp);
        cookie->fc_magic = FILE_COOKIE_MAGIC;   /* validate cookie */
        U_FDTABLE_UNLOCK(ufp);

        return 0;
}

/*
 *	fd_alloc
 *
 *	Given a cookie obtained from a call to fd_to_cookie (possibly
 *	in another process context) allocate a file descriptor and associate
 *	it with the cookie. The cookie does already represent a reference
 *	to that file.
 *
 *	If the allocation of the file descriptor is impossible (ENFILE),
 *	then DON'T destroy the cookie, but leave this to the dispostion
 *	of the caller. This restriction is necessary, since the caller
 *	might not be prepared to close a stream which is currently
 *	manipulating.
 *
 *	Parameters:
 *	cookie		(in)	- pointer to file cookie
 *	fdp		(out)	- pointer to file descriptor
 *
 *	Return value		- error condition
 *				  ENFILE - no file descriptor available
 */

int
fd_alloc (cookie, fdp)
	struct file_cookie *	cookie;
	int *			fdp;
{
        int     error;
        struct ufile_state *ufp = &u.u_file_state;

        if (cookie->fc_magic != FILE_COOKIE_MAGIC) {
                STR_DEBUG(printf("fd_alloc: invalid cookie %x\n", cookie->fc_mag
ic));
                return EBADF;
        }
        if (error = ufalloc(0, fdp, ufp))
                return error;

        /*
         *      from the U_FD_SET macro from file.h, plus setting u_pofile
         */
        U_FDTABLE_LOCK(ufp);
        ASSERT(U_OFILE(*fdp, ufp) == U_FD_RESERVED);
        U_OFILE_SET(*fdp, cookie->fc_fp, ufp);
        U_POFILE_SET(*fdp, cookie->fc_flags & ~UF_EXCLOSE, ufp);
        U_FDTABLE_UNLOCK(ufp);
        return 0;
}

/*
 *	cookie_destroy
 *
 *	Given a cookie obtained from a call to fd_to_cookie (possible
 *	in another process context), destroy it. (That is, decrement the
 *	reference count on the file structure.)
 *
 *	As a result of this, a close operation might be necessary. It is
 *	the caller's responsibility to provide an appropriate context
 *	for this. The actual action is taken by cookie_destroy. (The second
 *	is guaranteed not to happen for the thread which "just" allocated
 *	the cookie, and finds that it can't use it.)
 *
 *	Parameters:
 *	cookie		(in)	- pointer to file cookie
 *
 *	Return value:		- none.
 */

void
cookie_destroy (cookie)
	struct file_cookie *	cookie;
{
	if (cookie->fc_magic != FILE_COOKIE_MAGIC) {
		STR_DEBUG(printf("cookie_destroy: invalid cookie %x\n", cookie->fc_magic));
		return;
	}
	FP_UNREF(cookie->fc_fp);
}

/*
 *	sth_fd_to_sth
 *
 *	Given an fd, translate that into a stream head.
 *	Must verify that fd references another stream.
 *	Should, but doesn't, take reference on file.
 *	See usage and comments in str_osr.c...
 */

int
sth_fd_to_sth (fd, sthpp)
	int		fd;
	STHPP		sthpp;
{
	struct file *	fp;
	struct vnode *	vp;
	int		error = 0;
	struct specalias *sa;
        struct ufile_state *ufp = &u.u_file_state;

        U_FDTABLE_LOCK(ufp);
        if (fd < 0 || fd > ufp->uf_lastfile
        ||  (fp = U_OFILE(fd, ufp)) == NULL
        ||  fp == U_FD_RESERVED
        ||  !(vp = (struct vnode *)fp->f_data)) {
                U_FDTABLE_UNLOCK(ufp);
                return EBADF;
        }
        U_FDTABLE_UNLOCK(ufp);
	VN_LOCK(vp);
	if (vp->v_type == VCHR && (vp->v_flag & VXLOCK) == 0 &&
	    (sa = vp->v_specinfo->si_alias) && sa->sa_private &&
	    dcookie_to_dindex(major(vp->v_rdev)) >= 0)
		*sthpp = sa->sa_private;
	else
		error = EINVAL;
	VN_UNLOCK(vp);
	return error;
}

/*
 * Keep fake inode, fattach list, create, access, and modify times for pipes.
 */

#include <sys/unpcb.h>
extern	ino_t	unp_vno;

struct sth_pipestat {
	ino_t	ino;
	time_t	ctime, atime, mtime;
	STHP	sth1, sth2;
	struct sth_fattach {
		struct sth_fattach *next, *prev;
		STHP	sth;
		void *	vnode;
	} *fnext, *fprev;
	decl_simple_lock_data(,lock)
};

static struct vnodeops sfv_ops;
static struct vnodeops *old_v_ops;
static sfvop_inited;

int
sth_fattach (sth, flag, p)
	STHP	sth;
	int	flag;
	void *	p;
{
	struct vnode *vp = (struct vnode *)p;
	struct sth_pipestat *stb;
	struct sth_fattach *stf;
	extern int sth_close_fifo();

	if ((sth->sth_flags & (F_STH_PIPE|F_STH_FIFO)) == 0)
		return 0;
	if ((stb = (struct sth_pipestat *)sth->sth_next) == 0) {
		sth_update_times(sth, FCREAT, (struct stat *)0);
		stb = (struct sth_pipestat *)sth->sth_next;
	}
	simple_lock(&stb->lock);
	if (flag) {	/* fattach() */
		if (!stb->sth1 || !stb->sth2) {
			/* Cannot re-fattach after forcible fdetach */
			simple_unlock(&stb->lock);
			return EINVAL;
		}
		simple_unlock(&stb->lock);
		STR_MALLOC(stf, struct sth_fattach *, sizeof *stf,
			M_STREAMS, M_NOWAIT);
		if (stf == 0)
			return ENOMEM;
		stf->sth = sth;
		stf->vnode = vp;

		/* If a FIFO is being attached then change the vnode's
		 * vnodeops table to intercept closes.  This must be done
		 * by dup'ing the existing table since these tables are
		 * static.  Also, assume that all STREAMS FIFO vnodes
		 * will initially reference the same vnops table.
		 */
		if(sth->sth_flags & F_STH_FIFO) {
		    if(!sfvop_inited)
		    {
			sfv_ops = *vp->v_op;
			old_v_ops = vp->v_op;
			sfv_ops.vn_close = sth_close_fifo;
			sfvop_inited++;
		    }
		    VN_LOCK(vp);
		    vp->v_op = &sfv_ops;
		    VN_UNLOCK(vp);
		}
		simple_lock(&stb->lock);
		insque(stf, stb->fprev);
		simple_unlock(&stb->lock);
	} else {	/* fdetach() */
		for (stf = stb->fnext;
		     stf != (struct sth_fattach *)&stb->fnext;
		     stf = stf->next)
			if (stf->sth == sth && stf->vnode == vp) {
				remque(stf);
				break;
			}
		simple_unlock(&stb->lock);
		if (stf == (struct sth_fattach *)&stb->fnext)
		{
			return EINVAL;
		}
		STR_FREE(stf, M_STREAMS);
	}
	return 0;
}

void
sth_update_times (sth, flag, sb)
	STHP	sth;
	int	flag;
	struct stat *sb;
{
	STHP	sth2;
	queue_t *q;
	struct sth_pipestat *stb;
	struct sth_fattach *stf;
	struct timeval now;

	if ((sth->sth_flags & F_STH_PIPE) == 0)
		return;
	stb = (struct sth_pipestat *)sth->sth_next;
#if	MACH_ASSERT
	if ((stb && flag == FCREAT) || (!stb && flag != FCREAT))
		panic("sth_update_times");
#endif
	switch (flag) {
	default:
		simple_lock(&stb->lock);
		if (stb->ino == 0) {
			/* Use the same phony ino_t for streams and sockets */
			UNPMISC_LOCK();
			stb->ino = unp_vno++;
			UNPMISC_UNLOCK();
		}
		sb->st_ino = stb->ino;
		sb->st_atime = stb->atime;
		sb->st_mtime = stb->mtime;
		sb->st_ctime = stb->ctime;
		simple_unlock(&stb->lock);
		break;
	case FREAD:
		microtime(&now);
		simple_lock(&stb->lock);
		stb->atime = now.tv_sec;
		simple_unlock(&stb->lock);
		break;
	case FWRITE:
		microtime(&now);
		simple_lock(&stb->lock);
		stb->mtime = stb->ctime = now.tv_sec;
		simple_unlock(&stb->lock);
		break;
	case FCREAT:
		for (q = sth->sth_wq; q->q_next; q = q->q_next)
			;
		sth2 = (STHP)q->q_ptr;
#if	MACH_ASSERT
		if (!sth2 || sth2->sth_next
		||  sth2->sth_rq->q_qinfo->qi_putp != sth_rput )
			panic("sth_update_times fattach");
#endif
		STR_MALLOC(stb, struct sth_pipestat *, sizeof *stb,
				M_STREAMS, M_WAITOK);
		microtime(&now);
		simple_lock_init(&stb->lock);
		stb->atime = stb->mtime = stb->ctime = now.tv_sec;
		(stb->sth1 = sth)->sth_next = (STHP)stb;
		(stb->sth2 = (STHP)q->q_ptr)->sth_next = (STHP)stb;
		stb->fnext = stb->fprev = (struct sth_fattach *)&stb->fnext;
		break;
	case FSYNC:
		simple_lock(&stb->lock);
		/*
		 * SVID requires far end of pipe to be detached when near end
		 * closes (e.g server exits). We fdetach all instances on both
		 * sides however, because any (unlikely) fattaches on this side 
		 * need to be nuked - near side because it's being closed, far
		 * side because it's the SVID.
		 */
		while ((stf = stb->fnext) != (struct sth_fattach *)&stb->fnext){
			struct mount *mp;
			remque(stf);
			simple_unlock(&stb->lock);
			mp = ((struct vnode *) (stf->vnode))->v_mount;
			STR_FREE(stf, M_STREAMS);
			dounmount(NULLVP, mp, 0);
			simple_lock(&stb->lock);
		}
		if (stb->sth1 == sth)
			stb->sth1 = 0;
		if (stb->sth2 == sth)
			stb->sth2 = 0;
		sth->sth_next = 0;
		if (!stb->sth1 && !stb->sth2) {
			simple_unlock(&stb->lock);
			STR_FREE(stb, M_STREAMS);
		} else
			simple_unlock(&stb->lock);
		break;
	}
}

#include <sys/specdev.h>

/* intercept closes of vnodes which belong to FIFO's.  This must be done
 * so that the write reference count going to 0 can be detected to wakeup
 * any readers awaiting data.  This is required for SVID compliance.
 */

sth_close_fifo(vp,flags,cred)
struct vnode *vp;
int flags;
struct ucred *cred;
{
	register STHP sth;
	register struct mount *mp = vp->v_mount;
	register int error = 0;

	/* get the stream head */
	sth = (STHP)vp->v_specinfo->si_alias->sa_private;
	if(!sth)
		panic("no stream head on fifo close\n");
	/* call the real close routine.  Note, if the VOP_CLOSE funnels
	 * this call to spec_close will be funneled also.
	 */
	error = (*old_v_ops->vn_close)(vp,flags,cred);

	/* if the close went ok */
	if(error == 0) {
		/* update the stream head FIFO references.  Use the vnode
		 * counts because the attachment of the FIFO will generate
		 * an close with no corresponding open (because it wasn't
		 * a FIFO when originally created).
		 */
		simple_lock(&sth->sth_ext_flags_lock);
		if(flags & FREAD)
			sth->sth_rdcnt = (int)vp->v_rdcnt - 1;
		if(flags & FWRITE) {
			sth->sth_wrcnt = vp->v_wrcnt - 1;
			if(sth->sth_wrcnt == 0) {
				/* wakeup all readers */
				osrq_wakeup(&sth->sth_read_osrq);
			}
		}
		simple_unlock(&sth->sth_ext_flags_lock);
	}
	return error;
}
