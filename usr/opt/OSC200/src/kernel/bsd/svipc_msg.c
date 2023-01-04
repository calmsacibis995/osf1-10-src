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
static char *rcsid = "@(#)$RCSfile: svipc_msg.c,v $ $Revision: 4.3.12.7 $ (DEC) $Date: 1993/12/07 21:59:35 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * OSF/1 Release 1.0
 */
/*
 * Modification History
 *
 * 27-Oct-91	Fred Canter
 *	Make System V IPC definitions configurable.
 *
 */


/*
 *	Inter-Process Communication Message Facility. 
 */

#include <sys/secdefines.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/uio.h>
#include <kern/kalloc.h>
#include <kern/zalloc.h>
#include <kern/macro_help.h>
#include <kern/lock.h>
#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <sys/secpolicy.h>
#endif

int			msghdr_wait;	/* wait for headers event */
struct zone		*msghdr_zone;	/* zone for message headers */
struct zone		*msgwait_zone;  /* zone for message wait structs */
extern struct msqid_ds	msgque[];	/* msg queue headers */
extern struct msginfo	msginfo;	/* message parameters */

#if	SEC_ARCH
/*
 * Allocate space for the message queue tag pools. On systems that
 * allocate the message queue structures dynamically, the tag pools
 * should also be dynamically allocated at the same time as the
 * queues.
 */
extern tag_t		msgtag[];
#endif

lock_data_t		msg_lock;	/* protect queues on snd, rcv, ctl */
int			cur_msghdrs;	/* current number of message headers */
extern struct timeval	time;		/* system idea of date */


/*
 * free the message header, wakeup waiting procs, and decrement hdr count 
 */
#define FREE_MSGHDR(mp)				\
MACRO_BEGIN					\
    zfree(msghdr_zone, (vm_offset_t) mp);	\
    if (msghdr_wait) {				\
	msghdr_wait = 0;			\
	wakeup(&msghdr_wait);			\
    }						\
    cur_msghdrs--;				\
MACRO_END

/*
 *	msgconv - Convert a user supplied message queue id into a ptr to a
 *		  msqid_ds structure.
 */

struct msqid_ds *
msgconv(id)
register int    id;	/* message queue id */
{
	register struct msqid_ds 	*qp;	/* ptr to associated q slot */

	if (id < 0)
		return (NULL);

	qp = &msgque[id % msginfo.msgmni];

	if ((qp->msg_perm.mode & IPC_ALLOC) == 0 ||
	    id / msginfo.msgmni != qp->msg_perm.seq)
		return (NULL);
	return (qp);
}

/*
 *	msgctl - msgctl system call: perform  message control operations
 */

/* ARGSUSED */
msgctl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long             msqid, 
		                cmd;
		struct msqid_ds *buf;
	}	*uap = (struct args *)args;
	register struct msqid_ds 	*qp;	/* ptr to associated q */
	struct msqid_ds 		ds;	/* queue work area */
	int				error;
#if	SEC_ARCH
	dac_t	dac;
	int	ret;
#endif	

	if ((qp = msgconv(uap->msqid)) == NULL)
		return(EINVAL);

	*retval = 0;

	switch (uap->cmd) {
	case IPC_RMID:

#if	SEC_BASE
		if (!sec_owner(qp->msg_perm.uid, qp->msg_perm.cuid))
			return(EPERM);
#else
		if (u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif

#if	SEC_ARCH
		if (sec_ipcaccess(&qp->msg_perm, MSGTAG(qp, 0), SP_DELETEACC))
			return(EACCES);
#endif
		/*
	 	 * Don't have to take the lock until here, since this
		 * system call is not parallel. Makes sure we don't 
		 * destroy a queue while a snd or rcv is in progress.
		 */
		lock_write(&msg_lock);

                /*
		 * make sure the queue has not been removed.  This can
		 * happen if another msgctl() got in here first while
		 * we were blocked on the lock.
		 */
                if ((qp = msgconv(uap->msqid)) == NULL) {
                        lock_done(&msg_lock);
                        return(EIDRM);
                }

		while (qp->msg_first)
			msgfree(qp, NULL, qp->msg_first);

		qp->msg_cbytes = 0;

		if (uap->msqid + msginfo.msgmni < 0)
			qp->msg_perm.seq = 0;
		else
			qp->msg_perm.seq++;

		if (qp->msg_wait_list) {
			struct msg_wait *next_mwp=qp->msg_wait_list;
			struct msg_wait *mwp;

			while(mwp = next_mwp) {
				wakeup(mwp);
				next_mwp = mwp->msg_wait_next;
				zfree(msgwait_zone,(vm_offset_t) mwp);
			}
			qp->msg_wait_list = NULL;
		}
 
		if (qp->msg_perm.mode & MSG_RWAIT)
			wakeup(&qp->msg_qnum);

		if (qp->msg_perm.mode & MSG_WWAIT)
			wakeup(qp);

		qp->msg_perm.mode = 0;

		lock_done(&msg_lock);
		return(0);

	case IPC_SET:

#if	SEC_BASE
		if (!sec_owner(qp->msg_perm.uid, qp->msg_perm.cuid))
			return(EPERM);
#else
		if (u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif

#if	SEC_ARCH
		if (sec_ipcaccess(&qp->msg_perm, MSGTAG(qp, 0), SP_SETATTRACC))
			return(EACCES);
#endif

		/* get the queue id data struct into a work area */
		if (copyin(uap->buf, &ds, sizeof(ds)))
			return(EFAULT);

#if	SEC_BASE
		if (!sec_owner_change_permitted(qp->msg_perm.uid,
		    qp->msg_perm.gid, ds.msg_perm.uid, ds.msg_perm.gid))
			return(EPERM);
		if (ds.msg_qbytes > qp->msg_qbytes &&
		    !privileged(SEC_LIMIT, EPERM))
			return(EPERM);
#else
		if (ds.msg_qbytes > qp->msg_qbytes
		    && (error = suser(u.u_cred, &u.u_acflag)))
			return(error);
#endif	

		qp->msg_perm.uid = ds.msg_perm.uid;
		qp->msg_perm.gid = ds.msg_perm.gid;
		qp->msg_perm.mode = (qp->msg_perm.mode & ~0777) |
				    (ds.msg_perm.mode & 0777);
		qp->msg_qbytes = ds.msg_qbytes;
#if	SEC_ARCH
		dac.uid = qp->msg_perm.uid;
		dac.gid = qp->msg_perm.gid;
		dac.mode = qp->msg_perm.mode;
		ret = SP_CHANGE_OBJECT(MSGTAG(qp, 0), &dac,
				SEC_NEW_UID|SEC_NEW_GID|SEC_NEW_MODE);
		if (ret) {
			if (ret & SEC_NEW_UID)
				qp->msg_perm.uid = dac.uid;
			if (ret & SEC_NEW_GID)
				qp->msg_perm.gid = dac.gid;
			if (ret & SEC_NEW_MODE)
				qp->msg_perm.mode = (qp->msg_perm.mode & ~0777)
						  | (dac.mode & 0777);
		}
#endif
		qp->msg_ctime = time.tv_sec;

		return(0);

	case IPC_STAT:

#if	SEC_ARCH
		if (sec_ipcaccess(&qp->msg_perm, MSGTAG(qp, 0), SP_READACC))
#else
		if (ipcaccess(&qp->msg_perm, MSG_R))
#endif	
			return(EACCES);

		if (copyout(qp, uap->buf, sizeof(*qp)))
			return(EFAULT);

		return(0);

	default:
		return(EINVAL);
	}
	return(0);
}

/*
 *	msgfree - Free up space and message header, relink pointers on q,
 *	and wakeup anyone waiting for resources.
 */

msgfree(qp, pre_mp, mp)
register struct msqid_ds 	*qp; 		/* ptr to q of mesg being freed */
register struct msg 		*mp; 		/* ptr to msg being freed */
register struct msg 		*pre_mp;	/* ptr to mp's predecessor */
{
	/* unlink message from the queue */
	if (pre_mp == NULL)
		qp->msg_first = mp->msg_next;
	else
		pre_mp->msg_next = mp->msg_next;

	if (mp->msg_next == NULL)
		qp->msg_last = pre_mp;

	qp->msg_qnum--;

	/* wakeup processes waiting for space on the queue */
	if (qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~MSG_WWAIT;
		wakeup(qp);
	}

	/* free message text */
	if (mp->msg_ts)
		kfree(mp->msg_addr, mp->msg_ts);

	FREE_MSGHDR(mp);
}

/*
 *	msgget - msgget system call to get a message queue.
 */

/* ARGSUSED */
msgget(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
	        long		key;
		long            msgflg;
	}	*uap = (struct args *) args;
	struct msqid_ds *qp;	/* ptr to associated q */
	int             s;	/* ipcget status return */
	int		error;

	if (error = ipcget((key_t)uap->key, uap->msgflg, msgque, msginfo.msgmni,
			   (long)sizeof(*qp), &s, (struct ipc_perm **)&qp))
		return(error);


	if (s) {
		/* initialize the new queue */
		qp->msg_first = qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_qbytes = msginfo.msgmnb;
		qp->msg_lspid = qp->msg_lrpid = 0;
		qp->msg_stime = qp->msg_rtime = 0;
		qp->msg_ctime = time.tv_sec;
#if	SEC_ARCH
		sec_svipc_object_create(MSGTAG(qp, 0));
#endif
#if	SEC_ILB
		sp_init_obj_bits(MSGTAG(qp, 0));
#endif
	}

	*retval = qp->msg_perm.seq * msginfo.msgmni + (qp - msgque);
	return(0);
}

/*
 *	msginit - Called by main(main.c) to initialize message queues.
 */

msginit()
{
	/* initialize a zone for message headers */
	msghdr_zone = zinit(sizeof(struct msg),
			    msginfo.msgtql * sizeof(struct msg),
			    10 * sizeof(struct msg), "msghdr zone");

	msgwait_zone = zinit(sizeof(struct msg_wait),
			     msginfo.msgtql * sizeof(struct msg_wait),
			     100*sizeof(struct msg_wait), "msgwait_zone");

	lock_init(&msg_lock, TRUE);
}

/*
 *	msgrcv - msgrcv system call to receive a message.
 */

/* ARGSUSED */
msgrcv(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		msqid;
		void		*msgp;
		long		msgsz;
		long		msgtyp;
		long		msgflg;
	}	*uap = (struct args *) args;
	register struct msg 		*best_mp;	/* ptr to best msg on q */
	register struct msg 		*mp;		/* ptr to msg on q */
	register struct msg 		*pre_mp;	/* ptr to mp's predecessor */
	register struct msg 		*pre_best_mp;	/* ptr to best_mp's pred. */
	register struct msqid_ds 	*qp;		/* ptr to associated q */
	register int    		size;		/* transfer byte count */
	struct uio      		suio;		/* I/O info */
	struct iovec    		siov;		/* I/O vectors */
	int				error;
	int				firstpass;
#if	SEC_ILB
	tag_t				ntag;
#endif	
	/* over ride message type passed in */
	if (uap->msgflg & MSG_TAG) {
		uap->msgtyp = (long) p->p_pid;
	}

	lock_write(&msg_lock);
	firstpass = 1;
	error = 0;

	for (;;) {
		/* make sure the queue has not been removed */
		if ((qp = msgconv(uap->msqid)) == NULL) {
			error = firstpass?EINVAL:EIDRM;
			goto out;
		}
		firstpass = 0;

#if	SEC_ARCH
		if (sec_ipcaccess(&qp->msg_perm, MSGTAG(qp, 0), SP_READACC))
#else
		if (ipcaccess(&qp->msg_perm, MSG_R))
#endif
		{
			error = EACCES;
			goto out;
		}

		if (uap->msgsz < 0) {
			error = EINVAL;
			goto out;
		}

		best_mp = pre_best_mp = NULL;

		mp = qp->msg_first;
		pre_mp = NULL;

		if (uap->msgtyp == 0)
			/* get the first message on the list */
			best_mp = mp;
		else {
		        /* 
			 * prevent a message being removed while another
			 * proc is searching the list
			 */

			for (; mp; pre_mp = mp, mp = mp->msg_next) {
				if (uap->msgtyp > 0) {
					/* return first message of this type */
					if (uap->msgtyp != mp->msg_type)
						continue;

					best_mp = mp;
					pre_best_mp = pre_mp;

					break;
				}

				if (mp->msg_type <= -uap->msgtyp) {
					/*
					 * return message with lowest type of
					 * all on the queue 
					 */
					if (best_mp &&
					    best_mp->msg_type <= mp->msg_type)
						continue;

					best_mp = mp;
					pre_best_mp = pre_mp;
				}
			}
		}

		if (best_mp) {

			/* found the requested message */
			if (uap->msgsz < best_mp->msg_ts)
				if (!((int)uap->msgflg & MSG_NOERROR)) {
					error = E2BIG;
					goto out;
				} else
					size = uap->msgsz;
			else
				size = best_mp->msg_ts;

#if	SEC_ILB
			if (SP_CHECK_FLOAT(UIO_READ, MSGTAG(qp, 0), &ntag)) {
				error = u.u_error;
				goto out;
			}
#endif	
			/* store the message type */
			if (error = copyout(&best_mp->msg_type, uap->msgp,
					    sizeof(best_mp->msg_type))) {
				goto out;
			}

			if (size) {
				suio.uio_iov = &siov;
				suio.uio_iovcnt = 1;
				suio.uio_offset = 0;
				suio.uio_segflg = UIO_USERSPACE;
				suio.uio_resid = siov.iov_len = size;
				siov.iov_base = (caddr_t) uap->msgp +
						sizeof(best_mp->msg_type);

				/* read message text */
				suio.uio_rw = UIO_READ;
				if (error = uiomove(best_mp->msg_addr, size, 
						    &suio)) {
					goto out;
				}
			}

			*retval = size;

			qp->msg_cbytes -= best_mp->msg_ts;
			qp->msg_lrpid = u.u_procp->p_pid;
			qp->msg_rtime = time.tv_sec;
#if	SEC_ILB
			SP_DO_FLOAT(UIO_READ, MSGTAG(qp, 0), &ntag);
			if (qp->msg_qnum == 0)
				SP_EMPTY_OBJECT(MSGTAG(qp, 0));
#endif	

			/*
			 * Sys V raises the priority of this process to
			 * assure that it isn't preempted by the process
			 * doing the sleep. 
			 * curpri = PMSG; 
			 */

			/* free message from the linked list */
			msgfree(qp, pre_best_mp, best_mp);
        		/* global table() system call counter (see msg.h) */
        		msginfo.msg++;
			goto out;
		}  /* if (best_mp) */

		/* no message on the queue matches the receive request */
		if ((int)uap->msgflg & IPC_NOWAIT) {
			error = ENOMSG;
			goto out;
		}

		if (uap->msgtyp > 0) {
			struct msg_wait *qp_wait = qp->msg_wait_list;
			struct msg_wait *qp_prev;

			/* wait for a specific message type */
			while(qp_wait) {
				if (qp_wait->msg_wait_type == uap->msgtyp)
					break;
				qp_wait = qp_wait->msg_wait_next;
			}

			if (!qp_wait) {
				qp_wait = (struct msg_wait *)zalloc(msgwait_zone);
				qp_wait->msg_wait_next = qp->msg_wait_list;
				qp->msg_wait_list = qp_wait;
				qp_wait->msg_wait_type = uap->msgtyp;
				qp_wait->msg_wait_cnt = 0;
			}
			qp_wait->msg_wait_cnt++;

			if (mpsleep(qp_wait, PMSG | PCATCH, "sv_msg_rcv",
				    0, &msg_lock, MS_LOCK_WRITE)) {

				lock_write(&msg_lock);
				if ((qp = msgconv(uap->msqid)) == NULL) {
					lock_done(&msg_lock);
					return (EIDRM);
				}

				for (qp_wait = qp->msg_wait_list, qp_prev=NULL;
				     qp_wait; qp_prev = qp_wait,
				     qp_wait = qp_wait->msg_wait_next) {
					if (qp_wait->msg_wait_type ==
					    uap->msgtyp) {
					    if (! (--qp_wait->msg_wait_cnt)) {
						if (qp_prev)
							qp_prev->msg_wait_next=
							 qp_wait->msg_wait_next;
						else
				          		qp->msg_wait_list =
							 qp_wait->msg_wait_next;
						zfree(msgwait_zone, 
						      (vm_offset_t)qp_wait);
					    }
					    break;
				        }
				}

				lock_done(&msg_lock);
				return(EINTR);
			}

		} else {
			/* wait for ANY messages on the queue */
			qp->msg_perm.mode |= MSG_RWAIT;

			if (mpsleep(&qp->msg_qnum, PMSG | PCATCH, "sv_msg_rcv",
				    0, &msg_lock, MS_LOCK_WRITE))
				return(EINTR);
		}
	}  /* for (;;) */

out:
	lock_done(&msg_lock);
	return(error);
}

/*
 *	msgsnd - msgsnd system call to send a message. 
 */

/* ARGSUSED */
msgsnd(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long		msqid;
		void		*msgp;
		long		msgsz;
		long		msgflg;
	}	*uap = (struct args *) args;
	register struct msqid_ds 	*qp;	/* ptr to associated q */
	register struct msg 		*mp;	/* ptr to allocated msg hdr */
	register int    		cnt;	/* byte count */
	register caddr_t 		msgaddr;/* msg allocation address */
	struct uio      		suio;	/* I/O info */
	struct iovec    		siov;	/* I/O vectors */
	long            		type;	/* msg type */
	int				error;
	int				firstpass;
#if	SEC_ILB
	tag_t   			ntag;
#endif	

	lock_write(&msg_lock);
	firstpass = 1;
	error = 0;

	/* wait until all resources are available to allocate this message */
	for (;;) {
		/* make sure the queue has not been removed */
		if ((qp = msgconv(uap->msqid)) == NULL) {
			error = firstpass?EINVAL:EIDRM;
			goto out;
		}
		firstpass = 0;

#if	SEC_ARCH
		if (sec_ipcaccess(&qp->msg_perm, MSGTAG(qp, 0), SP_WRITEACC))
#else	
		if (ipcaccess(&qp->msg_perm, MSG_W))
#endif
		{
			error = EACCES;
			goto out;
		}

		if ((cnt = uap->msgsz) < 0 || cnt > msginfo.msgmax) {
			error = EINVAL;
			goto out;
		}

		/* get the message type */
		if (uap->msgflg & MSG_TAG) {
			type = (long) p->p_pid;
		} else if (error = copyin(uap->msgp, &type, sizeof(type)))
			goto out;

		if (type < 1) {
			error = EINVAL;
			goto out;
		}

		/*
		 * if bytes for message exceed maximum number of bytes on 
		 * the queue, wait for more space on the queue 
		 */
		if (cnt + qp->msg_cbytes > qp->msg_qbytes) {
			if (uap->msgflg & IPC_NOWAIT) {
				error = EAGAIN;
				goto out;
			}

			qp->msg_perm.mode |= MSG_WWAIT;

			if (mpsleep(qp, PMSG | PCATCH, "sv_msg_snd", 0,
				    &msg_lock, MS_LOCK_WRITE)) {
				qp->msg_perm.mode &= ~MSG_WWAIT;
				wakeup(qp);
				return(EINTR);
			}
			continue;
		}

		/*
		 * if the max number of message headers have been allocated, 
		 * wait for some to be freed 
		 */
		if (cur_msghdrs >= msginfo.msgtql) {
			if (uap->msgflg & IPC_NOWAIT) {
				error = EAGAIN;
				goto out;
			}

			msghdr_wait++;
			if (mpsleep(&msghdr_wait, PMSG | PCATCH, "sv_msg_snd",
				    0, &msg_lock, MS_LOCK_WRITE))
				return(EINTR);

			continue;
		}

		break;
	}  /* for (;;) */

	/* everything is available; allocate a message header */
	mp = (struct msg *) zalloc(msghdr_zone);
	cur_msghdrs++;

	if (cnt) {
		/* allocate the message text and fill it in */
		msgaddr = kalloc(cnt);

		suio.uio_iov = &siov;
		suio.uio_iovcnt = 1;
		suio.uio_offset = 0;
		suio.uio_segflg = UIO_USERSPACE;
		suio.uio_resid = siov.iov_len = cnt;
		siov.iov_base = (caddr_t) uap->msgp + sizeof(type);

		suio.uio_rw = UIO_WRITE;
		if (error = uiomove(msgaddr, cnt, &suio)) {
			kfree(msgaddr, cnt);
			FREE_MSGHDR(mp);
			goto out;
		}
#if	SEC_ILB
		if (SP_CHECK_FLOAT(UIO_WRITE, MSGTAG(qp, 0), &ntag)) {
			kfree(msgaddr, cnt);
			FREE_MSGHDR(mp);
			goto out;
		}
		SP_DO_FLOAT(UIO_WRITE, MSGTAG(qp, 0), &ntag);
#endif	
	}

	/* put the message on the queue */
	qp->msg_qnum++;
	qp->msg_cbytes += cnt;
	qp->msg_lspid = u.u_procp->p_pid;
	qp->msg_stime = time.tv_sec;

	mp->msg_next = NULL;
	mp->msg_type = type;
	mp->msg_ts = cnt;
	mp->msg_addr = msgaddr;

	if (qp->msg_last == NULL)
		qp->msg_first = qp->msg_last = mp;
	else {
		qp->msg_last->msg_next = mp;
		qp->msg_last = mp;
	}

	if (qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;

		/*
		 * Sys V raises the priority of this process to assure that
		 * it isn't preempted by the process doing the sleep.      
		 * curpri = PMSG; 
		 */
		wakeup(&qp->msg_qnum);
	}
	
	if (qp->msg_wait_list) {
		struct msg_wait *prev_mwp=(struct msg_wait *)&qp->msg_wait_list;
		struct msg_wait *mwp;

		while(mwp = prev_mwp->msg_wait_next) {
			if (mwp->msg_wait_type == type) {
				wakeup(mwp);
				prev_mwp->msg_wait_next = mwp->msg_wait_next;
				zfree(msgwait_zone,(vm_offset_t) mwp);
			} else {
				prev_mwp = mwp;
			}
		}
	}

	/* global table() system call counter (see msg.h) */
	msginfo.msg++;

	*retval = 0;
out:
	lock_done(&msg_lock);
	return(error);
}
