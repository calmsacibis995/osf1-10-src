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
 * @(#)$RCSfile: pty.h,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/06/17 20:40:08 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * Linked list header and initialization macro
 */
struct que_hd {
	struct que_hd	 	*next;
	struct que_hd	 	*prev;
};
#define ptque_init(q)	((q)->next = (q)->prev = q)


/*
 *  pt_ioctl structure used for BSD compatibilty.  Protected by pty_lock
 *	in pty_s structure.
 */
struct pt_ioctl {
	int 			pi_flags;	/* BSD mode (eg. PACKET)*/ 
	u_char			pi_send;	/* for proc Packet mode */
	u_char			pi_ucntl;	/* for proc User Ctl mode*/
	struct que_hd 		pi_selqhd;	/* select queue head  */
};

/*
 * pty queue used for BSD reads.  Protected by pty_lock in 
 * pty_s structure.
 */

struct ptque {
	mblk_t 			*ptq_head;
	mblk_t 			*ptq_tail;
	int	 		ptq_count;	
	int	 		ptq_hiwater;
	int	 		ptq_lowater;
};

struct pty_s {
	/* Fields protected by PTY_READ_LOCK/PTY_WRITE_LOCK (blocking lock).
	 */
	int 			pt_alloc;	/* allocated state 	*/
	dev_t 			pt_dev;		/* maj+min (master maj) */
	caddr_t 		pt_mrq;		/* master read queue	*/
	caddr_t 		pt_mwq;		/* master write queue	*/
	int 			(*pt_xfer)();	/* slave write vector 	*/
	void 			(*pt_enable)();	/* flow enable vector 	*/
	int 			(*pt_mdrained)();/* master drained vector */
	queue_t 		*pt_srq;	/* slave read queue	*/
#if SEC_ARCH
	tag_t   		pt_mtag[SEC_TAG_COUNT];
	tag_t   		pt_stag[SEC_TAG_COUNT];
#endif /* SEC_ARCH */
	/*
	 * Field protected by streams synch (only used on slave side)
	 */
	mblk_t			*pt_qioctl;	/* queued ioctl 	*/
	/*
	 * Fields protected by pty_lock (simple lock).
	 */
	int 			pt_flags;	/* pty state flags	*/
	struct termios 		pt_termios;	/* termios structure	*/
	struct winsize 		pt_wnsz;	/* window size struct.	*/
	struct pt_ioctl 	pt_ioctl;	/* for BSD compatibility */
	struct ptque 		pt_ptque;	/* for BSD reads */
	tcflag_t		pt_compatflags; /* for BSD compatibility */
	int			pt_readers;	/* read lock ref. count */
#if	MACH_ASSERT
	char			*pt_mutex_holder;	/* thread address */
#endif
	decl_simple_lock_data(,pty_lock)
};   	

#define	pt_iflag		pt_termios.c_iflag
#define	pt_oflag		pt_termios.c_oflag
#define	pt_cflag		pt_termios.c_cflag
#define	pt_lflag		pt_termios.c_lflag
#define	pt_cc			pt_termios.c_cc
#define pt_ispeed		pt_termios.c_ispeed
#define pt_ospeed		pt_termios.c_ospeed

#define pt_bsdflags		pt_ioctl.pi_flags
#define pt_send			pt_ioctl.pi_send
#define pt_ucntl		pt_ioctl.pi_ucntl
#define pt_selqhd		pt_ioctl.pi_selqhd


/*
 * State bits for pt_flags field
 */
#define PF_MOPEN	0x00000001	/* master is open */
#define PF_SOPEN	0x00000002	/* slave is open  */
#define PF_XCLUDE	0x00000004	/* exclusive owner */
#define PF_TTSTOP	0x00000008	/* output stopped  */
#define PF_TTINSTOP	0x00000010    	/* input stopped   */
#define PF_MFLOW	0x00000020	/* master resource flow controlled */
#define PF_SFLOW	0x00000040	/* slave resource flow controlled */
#define PF_SLOCK	0x00000080	/* Slave locked - SYS V only */
#define PF_SWOPEN	0x00000100   	/* Slave waiting for master open */
#define PF_CARR_ON	0x00000200	/* Slave and Master in sync */
#define PF_REMOTE	0x00000400	/* REMOTE or noncanonical mode */
#define PF_WDRAIN	0x00000800	/* Slave closing; waiting for drain */
#define PF_WANT_LOCK	0x00001000	/* Asleep until lock available */
#define PF_NBIO		0x00002000	/* don't block (on writes) */
#define PF_SV		0x10000000	/* System V style master */

/*
 * State bits for pt_alloc field.
 */

#define PFA_SLAVE_ALLOC		0x1	/* At least one slave is active */
#define PFA_MASTER_ALLOC	0x2	/* A master is active */
#define PFA_ALLOC		(PFA_SLAVE_ALLOC | PFA_MASTER_ALLOC)

/*
 * State bits for pt_bsdflags field.
 */

#define PFB_PKT		0x08  /* in PACKET Mode		*/
#define PFB_STOPPED	0x10  /* output stopped		*/
#define PFB_NOSTOP	0x40
#define PFB_UCNTL	0x80  /* USER CONTROL Mode */

/*
 * High water and low water marks for flow control in the 3 "sub-drivers".
 */

#define SLAVE_HIWATER		512
#define SLAVE_LOWATER		256

#define BSD_MAST_HIWATER	2048
#define BSD_MAST_LOWATER	1024

#define SYSV_MAST_HIWATER	512
#define SYSV_MAST_LOWATER	128

/*
 *
 * Locks
 *
 */

#define PTY_OPCL_LOCK_INIT()	lock_init2(&PTY_OPCL_lock, 1, LTYPE_PTY)
#define PTY_OPCL_LOCK()		lock_write(&PTY_OPCL_lock)
#define PTY_OPCL_UNLOCK()	lock_done(&PTY_OPCL_lock)

#define PTY_LOCK_INIT(ptp)
#define PTY_WRITE_LOCK(ptp)						    \
	while (1) {							    \
		simple_lock(&(ptp)->pty_lock);				    \
		if ((ptp)->pt_readers != 0) {				    \
			(ptp)->pt_flags |= PF_WANT_LOCK;		    \
			assert_wait((int) &(ptp)->pt_readers,	    	    \
							FALSE);             \
			simple_unlock(&(ptp)->pty_lock);		    \
			(void) tsleep((caddr_t)0, PTYPRI, ptyclck, 0);   \
		} else {						    \
			(ptp)->pt_readers = -1;				    \
			ASSERT((ptp)->pt_mutex_holder =			    \
					(char *) current_thread()); 	    \
			simple_unlock(&(ptp)->pty_lock);		    \
			break;						    \
		}							    \
	}

#define PTY_WRITE_UNLOCK(ptp)						    \
	do {								    \
		simple_lock(&(ptp)->pty_lock);				    \
		ASSERT((ptp)->pt_readers == -1);			    \
		ASSERT((ptp)->pt_mutex_holder == (char *) current_thread());\
		(ptp)->pt_readers = 0;					    \
		ASSERT(!((ptp)->pt_mutex_holder = (char *) 0));		    \
		if ((ptp)->pt_flags & PF_WANT_LOCK) {			    \
			(ptp)->pt_flags &= ~PF_WANT_LOCK;		    \
			simple_unlock(&(ptp)->pty_lock);		    \
			pty_wakeup((caddr_t) &(ptp)->pt_readers);	    \
		} else							    \
			simple_unlock(&(ptp)->pty_lock);		    \
	} while (0)

#define PTY_WR2RD_LOCK(ptp)						    \
	do {								    \
		simple_lock(&(ptp)->pty_lock);				    \
		ASSERT((ptp)->pt_readers == -1);			    \
		ASSERT((ptp)->pt_mutex_holder == (char *) current_thread());\
		(ptp)->pt_readers = 1;					    \
		ASSERT(!((ptp)->pt_mutex_holder = (char *) 0));		    \
		if ((ptp)->pt_flags & PF_WANT_LOCK) {			    \
			(ptp)->pt_flags &= ~PF_WANT_LOCK;		    \
			simple_unlock(&(ptp)->pty_lock);		    \
			pty_wakeup((caddr_t) &(ptp)->pt_readers);	    \
		} else							    \
			simple_unlock(&(ptp)->pty_lock);		    \
	} while (0)

#define PTY_READ_LOCK(ptp)						    \
	while (1) {						    	    \
		simple_lock(&(ptp)->pty_lock);				    \
		if ((ptp)->pt_readers == -1) {				    \
			(ptp)->pt_flags |= PF_WANT_LOCK;		    \
			assert_wait((int) &(ptp)->pt_readers,	            \
							FALSE);             \
			simple_unlock(&(ptp)->pty_lock);		    \
			thread_block();					    \
		} else {						    \
			(ptp)->pt_readers++;				    \
			simple_unlock(&(ptp)->pty_lock);		    \
			break;						    \
		}							    \
	}

#define PTY_READ_UNLOCK(ptp)						    \
	do {								    \
		simple_lock(&(ptp)->pty_lock);				    \
		ASSERT((ptp)->pt_readers > 0);				    \
		if ((--(ptp)->pt_readers == 0)				    \
		&& ((ptp)->pt_flags & PF_WANT_LOCK)) {			    \
			(ptp)->pt_flags &= ~PF_WANT_LOCK;		    \
			simple_unlock(&(ptp)->pty_lock);		    \
			pty_wakeup((caddr_t) &(ptp)->pt_readers);	    \
		} else							    \
			simple_unlock(&(ptp)->pty_lock);		    \
	} while (0)

#define PTY_RD2WR_LOCK(ptp)						    \
	do {								    \
		PTY_READ_UNLOCK((ptp));					    \
		PTY_WRITE_LOCK((ptp));					    \
	} while (0)

#if MACH_ASSERT
#define PTY_READ_LOCK_ASSERT(ptp)					    \
	do {								    \
		simple_lock(&(ptp)->pty_lock);				    \
		ASSERT((ptp)->pt_readers > 0);				    \
		simple_unlock(&(ptp)->pty_lock);			    \
	} while (0)
#else
#define PTY_READ_LOCK_ASSERT(ptp)
#endif

#if MACH_ASSERT
#define PTY_ANY_LOCK_ASSERT(ptp)					    \
	do {								    \
		simple_lock(&(ptp)->pty_lock);				    \
		ASSERT((ptp)->pt_readers != 0);				    \
		simple_unlock(&(ptp)->pty_lock);			    \
	} while (0)
#else
#define PTY_ANY_LOCK_ASSERT(ptp)
#endif

/*
 * Sleep priority.
 */

#define PTYPRI 28			/* sleep priority		*/

/****						       ****
 ****	Prototypes from here to the end of the file.   ****
 ****						       ****/

PRIVATE_STATIC int
ptm_close(queue_t *, int, cred_t *);

PRIVATE_STATIC int
ptm_mdrained(struct pty_s *);

int
ptm_configure(sysconfig_op_t, str_config_t *, size_t, str_config_t *, size_t);

PRIVATE_STATIC void
ptm_enable(struct pty_s *, int);

PRIVATE_STATIC int
ptm_open(queue_t *, dev_t *, int, int, cred_t *);

PRIVATE_STATIC int
ptm_rput(queue_t *, mblk_t *);

PRIVATE_STATIC int
ptm_rsrv(queue_t *);

PRIVATE_STATIC int
ptm_wput(queue_t *, mblk_t *);

PRIVATE_STATIC int
ptm_write_data(struct pty_s *, mblk_t *);

PRIVATE_STATIC int
ptm_wsrv(queue_t *);

PRIVATE_STATIC int
ptm_xfer(struct pty_s *, mblk_t *);

int
ptrclose(dev_t, int, int, struct ucred *, void *);

PRIVATE_STATIC int
ptrmdrained(struct pty_s *);

PRIVATE_STATIC void
ptrenable(struct pty_s *, int);

int
ptrioctl(dev_t, unsigned int, caddr_t, int, struct ucred *, void *, int *);

int
ptropen(dev_t, int, int, dev_t *, struct ucred *, void **);

int
ptrread(dev_t, struct uio *, int, void *);

int
ptrselect(dev_t, short *, short *, int, void *);

int
ptrwrite(dev_t, struct uio *, int, void *);

PRIVATE_STATIC int
ptrxfer(struct pty_s *, mblk_t *);

PRIVATE_STATIC int
pts_close(queue_t *, int, cred_t *);

int
pts_configure(sysconfig_op_t, str_config_t *, size_t, str_config_t *, size_t);

PRIVATE_STATIC int
pts_mctl(queue_t *, mblk_t *);

PRIVATE_STATIC int
pts_open(queue_t *, dev_t *, int, int, cred_t *);

PRIVATE_STATIC int
pts_rput(queue_t *, mblk_t *);

PRIVATE_STATIC int
pts_rsrv(queue_t *);

PRIVATE_STATIC int
pts_start(queue_t *, int);

PRIVATE_STATIC int
pts_wput(queue_t *, mblk_t *);

PRIVATE_STATIC int
pts_writedata(struct pty_s *, mblk_t *);

PRIVATE_STATIC int
pts_wsrv(queue_t *);

PRIVATE_STATIC int
pty_bsd43_ioctl(int, caddr_t, struct termios *, tcflag_t *, int);

PRIVATE_STATIC mblk_t *
pty_ctl(int, int);

PRIVATE_STATIC void
pty_init(struct pty_s *);

PRIVATE_STATIC int
pty_ioctl_comm(int *, int, caddr_t, struct pty_s *, int);

#if SEC_ARCH
PRIVATE_STATIC void
pty_sec_init(struct pty_s *);
#endif /* SEC_ARCH */

PRIVATE_STATIC void
pty_strioctl_comm(mblk_t *, struct pty_s *, int );

PRIVATE_STATIC void
pty_wakeup(caddr_t);
