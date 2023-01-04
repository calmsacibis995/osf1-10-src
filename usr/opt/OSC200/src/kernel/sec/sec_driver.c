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
 *	Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 *	Security Policy Message Driver
 *
 *	This driver provides a inter-process communication facility
 *	between client user processes (window manager) and user contexts
 *	calling from within kernel security modules. The messages that
 *	are received from these processes are requests that are directed
 *	to the specified security policy daemon. The daemon will later
 *	deliver a response message to the driver which is then returned
 *	to the user process. The driver manages buffer allocation, message
 *	copying, and process synchronization.
 *
 *	There are two minor devices associated with each security policy
 *	configured on the system. The first device of the pair is used
 *	exclusively by the security policy daemon to receive requests and
 *	deliver responses. The other minor device is used by as many client
 *	user processes that require the services of the daemon. Other users
 *	of the daemon include user contexts within kernel security modules
 *	but these do not interface to the devices using system calls.
 */

/* #ident "@(#)sec_driver.c	2.4 00:37:10 5/3/90 SecureWare"*/
/*
 * Based on:
 *   "@(#)sec_driver.c	2.11.3.2 11:16:46 12/27/89 SecureWare, Inc."
 */

#include <sec/include_sec>

#if SEC_ARCH /*{*/

/* Commonly Used Macro Definitions  */

#define DEVICE(dev)	((minor(dev)) & 1)
#define POLICY(dev)	((minor(dev)) >> 1)

#define	NULL_MCB	((struct message_cblk *) 0)

#define DAEMON		0
#define CLIENT		1

#include <kern/sched_prim.h>
#define	RETERR(e)	return(e);

/* 
 * This is just to reduce the clutter of nested conditionals.
 * Similar code exists in audit_dev.c.
 */

#define	SEC_UIOMOVE(buf, size, rw, uio) uiomove((buf), (size), (uio))


/* Security Policy Device Control Block-one per minor device pair */

static struct device_control_block {
	ulong	spd_sync_id;		/* synchronization id */
	decl_simple_lock_data(,lock)	/* lock for dcb and its mcb queues */
	short flags;		/* driver specific flags */
	pid_t pid;		/* daemon process id */
	struct proc *procp; 	/* daemon process pointer */
	struct message_cblk *request; 	/* daemon request queue */
	struct message_cblk *response; 	/* waiting for response queue */
	struct message_cblk *waiting; 	/* waiting on event queue */
	struct stats {
		int daemon_reads; 	/* daemon device reads */
		int daemon_writes; 	/* daemon device writes */
		int client_reads; 	/* client device reads */
		int client_writes; 	/* client device writes */
		int send_message;	/* send_message() requests */
		int rcv_message;	/* send_message() responses */
	} stats;
} *dcb;

/*
 * 		dcb flag bit definitions
 * SPD_DEVICE_OPEN is set by SPIOC_EXCL ioctl, cleared by spdclose().
 * SPD_DAEMON_ASLEEP is set by spdread() and tested by
 * submit_req_to_daemon()
 */

#define SPD_DEVICE_OPEN		0x01
#define SPD_DAEMON_ASLEEP	0x02	/* sleeping in spdread() */

#define spd_device_open(policy) (dcb[(policy)].flags & SPD_DEVICE_OPEN)

/* Control Block structure for each active request-response pair */

struct message_cblk {
	struct message_cblk *flink;
	struct message_cblk *blink;
	time_t	time;		/* request time */
	ulong	sync_id;	/* sync id for crossmatch */
	caddr_t	buffer;		/* client buffer address (mblk) */
	caddr_t	mbuf;		/* message buffer if arrived */
	thread_t threadp;	/* thread pointer for wakeup */
	pid_t	pid;		/* process id of sender */
	short	size;		/* client buffer size */
	short	flags;		/* state field */
	unsigned policy;	/* just for debugging */
};

/* Flags */

#define SPD_CLIENT_PROCESS	0x01
#define SPD_NO_RESPONSE		0x10
#define SPD_PROCESS_ASLEEP	0x20
#define SPD_DAEMON_TERMED	0x40	/* daemon terminated */

/* Pool of Message Control Blocks that will be dynamically allocated */

static struct message_cblk *mcb;
static struct message_cblk mcb_freelist;

struct message_cblk *mcb_alloc(), *find_mcb();
char *spd_send_message();
char *spd_allocate();

/*
 * Global Message Driver Statistics not particular to a Policy
 */

static struct spd_stats {
	decl_simple_lock_data(, lock)
	int	bufwaits;	/* bufcall wait requests */
	int	allocates;	/* number of spd_allocate() requests */
	int	deallocates;	/* number of spd_deallocate() requests */
} spd_stats;

/* Global Driver flags-Common to all minor devices */

static int spd_init = 0;

/*
 * Various lock definitions follow. Note that these locks
 * are sometimes needed, even in the uniprocessor case.
 */
 
lock_data_t	spioc_init_lock;

/*
	spdinit()-perform initialization functions
	This is called only at boot time.
*/

spdinit()
{
	register int	policy, sindex, oindex, bytes;

	/*
	 * Initialize locks used by the policy daemon.
	 * The second argument indicates whether the caller is
	 * willing to sleep in order to get the lock.
	 */

	lock_init(&spioc_init_lock, TRUE);

	simple_lock_init(&spd_stats.lock);


	/*
	 * Allocate space for device, message and buffer control blocks
	 * in one large chunk, then carve it up.  The various pointers
	 * are first initialized to contain byte offsets from the base
	 * of the allocated block. They are then incremented (as byte
	 * pointers) by the starting address of the allocated block.
	 * XXX MP note: This should really be a per-thread allocation.
	 * Similarly, mcb_init() should be changed to do per-thread init.
	 */

#define	SPDALIGN(p)	(((ulong)(p) + 3) & -4L)

	mcb = (struct message_cblk *) 0;
	dcb = (struct device_control_block *) SPDALIGN(&mcb[nproc]);
	bytes = (int) &dcb[spolicy_cnt];

#undef SPDALIGN

	mcb = (struct message_cblk *) kalloc(bytes);
	if (mcb == (struct message_cblk *) 0)
		panic("spdinit: unable to allocate space");

	/*
	 * Clear the space allocated for mcbs and dcbs
	 */
	bzero(mcb, bytes);

	dcb = (struct device_control_block *) ((char *) dcb + (ulong) mcb);

	/*
	 * Initialize the mcb freelist
	 */
	mcb_init();

	/*
	 * Walk through the policy switch to compute the indices
	 * of the first subject and object tag for each policy,
	 * and to call the policy init functions.
	 */
	sindex = 0;
	oindex = 0;
	for(policy = 0; policy < spolicy_cnt; policy++) {
		sp_switch[policy].sw_first_subj_tag = sindex;
		sp_switch[policy].sw_first_obj_tag = oindex;
		sindex += SUBJECT_TAG_COUNT(policy);
		oindex += OBJECT_TAG_COUNT(policy);
		SP_INIT(policy);
	}

	spd_init = 1;
}

/*
 * Macros to lock/unlock the dcb for the specified security policy.
 */

#define dcb_lock(policy)	simple_lock(&dcb[(policy)].lock);
#define dcb_unlock(policy)	simple_unlock(&dcb[policy].lock);

/*
 *	spdopen()-general open routine for all policy minor devices
 *	invoked only through the cdevsw switch
 */

spdopen(dev, flags)
{
	register int policy = POLICY(dev);
	long needed_priv;

	/* Insure that the message control structures have been init'd */

	if (spd_init == 0)
		RETERR(EIO)

	/* Allow the control device to be opened for GETCONF */

	if (minor(dev) == SPD_CONTROL_MINOR)
		return(0);
	
	/* Make sure the device corresponds to a configured policy */

	if (policy >= spolicy_cnt)
		RETERR(ENXIO)

	/* See if we have the required privilege to access the device */

	if (!privileged(sp_switch[policy].sw_priv_required,EPERM))
		RETERR(EPERM)

	/*
	 * The daemon device is an exclusive access device.
	 * The client device can't be opened unless the daemon
	 * device is open.
	 */

	if (DEVICE(dev) == DAEMON) {	/* opening daemon device */
		if (spd_device_open(policy))
			RETERR(EEXIST)
	} else {			/* opening client device */
		if (!spd_device_open(policy))
			RETERR(EIO)
	}

	return(0);
}

/*
 *	spdclose()-close the minor device
 *	invoked only through the cdevsw switch
 */

spdclose(dev)
int dev;
{
	register struct message_cblk *mcbp;
	register int policy = POLICY(dev);

	/*
	 * Lock the dcb while walking the mcb queues, and to 
	 * prevent races with SPIOC_EXCL, etc.
	 */

	if ( DEVICE(dev) == DAEMON ) {	/* closing daemon device */
		dcb_lock(policy);
		dcb[policy].pid = 0;
		dcb[policy].procp = (struct proc *) 0;
		dcb[policy].flags &= ~SPD_DEVICE_OPEN;

	/*
	 * Walk the message control block chains and mark any MCBs
	 * as daemon terminated so that when they awaken they can
	 * properly handle the termination case.
	 */

		daemon_term_wakeup(dcb[policy].request);
		daemon_term_wakeup(dcb[policy].response);
		daemon_term_wakeup(dcb[policy].waiting);
		dcb_unlock(policy);

	/*
	 * Revoke the daemon's privilege of immortality.
	 */
		PROC_LOCK(u.u_procp);
		u.u_procp->p_flag &= ~SSYS;
		PROC_UNLOCK(u.u_procp);
	}
	return(0);
}

/*
 * Traverse a daemon queue, informing all sleepers that the
 * policy daemon on which they're waiting is deceased.
 * This is called only by spdclose(), which holds the dcb lock.
 */

daemon_term_wakeup(mcbp)
register struct message_cblk *mcbp;
{
	while(mcbp != NULL_MCB) {
		mcbp->flags |= SPD_DAEMON_TERMED;
		thread_wakeup((vm_offset_t) mcbp);
		mcbp = mcbp->flink;
	}
}

/*
 * Return a Boolean to indicate whether the daemon has died.
 * If it has, make sure this mcb has been dequeued from the
 * daemon's queues, using an ugly brute force search. This is
 * typically called upon awakening from a sleep after sending
 * the daemon a message.
 */

daemon_termed(policy, mcbp)
register unsigned policy;
register struct message_cblk *mcbp;
{
	dcb_lock(policy);
	if ((mcbp -> flags & SPD_DAEMON_TERMED) == 0) {
	    dcb_unlock(policy);
	    return(0);
	}

	/*
	 * The daemon may have died while our request was still outstanding.
	 * Do a brute force search, and dequeue it if we find it, since
	 * we know the dead daemon will never do it for us.
	 */

	mcb_dequeue(&dcb[policy].request,mcbp,1);
	mcb_dequeue(&dcb[policy].response,mcbp,1);
	mcb_dequeue(&dcb[policy].waiting,mcbp,1);
	dcb_unlock(policy);
	mcb_free(mcbp);
	return(1);
}

/*
 * Assign a unique sequence number for mcb and mhdr so that
 * the daemon's response will get matched up with the proper mcb.
 * Add the request to the daemon's request queue.
 * Awaken the daemon if it's sleeping in spdread().
 */

submit_req_to_daemon(policy, mcbp)
register unsigned policy;
register struct message_cblk *mcbp;
{
	mhdr_t	*mhdr = (mhdr_t *) mcbp -> buffer;
	mcbp -> sync_id = dcb[policy].spd_sync_id++;
	mhdr -> sync_id = mcbp -> sync_id;
	mcb_enqueue(&dcb[policy].request,mcbp);
	if (dcb[policy].flags & SPD_DAEMON_ASLEEP)
		thread_wakeup((vm_offset_t) &dcb[policy]);
}


/*
 *	spdread()-Reads on the daemon minor device are done by the daemon
 *	to pick up request messages. These will be serviced and the response
 *	returned via a write to the same device. Reads by client user processes
 *	on the other minor device of the pair are attempts by the process
 *	to retrieve responses from the daemon.
 *	Invoked only through the cdevsw switch
 *
 * 	POSIX ACLS  -- removed ifdef osf_source
 */

spdread(dev, uio, flag)
	dev_t		dev;
	struct uio	*uio;
{
	register struct message_cblk *mcbp;
	register int policy = POLICY(dev);
	int result = 0;		/* assume success */

	switch( DEVICE(dev) ) {

	   case DAEMON:
		/*
		 * While there's nothing on the request queue, sleep
		 * until someone calls submit_req_to_daemon().
		 */
		dcb_lock(policy);
		dcb[policy].stats.daemon_reads++;
		/*
		 * POSIX ACLS -- set mcbp 
                 */
		while((mcbp = dcb[policy].request) == NULL_MCB) {
			dcb[policy].flags |= SPD_DAEMON_ASLEEP;
			thread_sleep((vm_offset_t) &dcb[policy],
				simple_lock_addr(dcb[policy].lock), FALSE);
			dcb_lock(policy);
		}


		/*
                 * POSIX ACLS --
                 * Remove the mcb from the request queue while we still
                 * hold the dcb lock and if a reply is expected p ut it
                 * on the response queue so the daemon will be ab le to
                 * find it when it writes the response.
		 */

                mcb_dequeue(&dcb[policy].request, mcbp, 0);
                if ((mcbp->flags & SPD_NO_RESPONSE) == 0)
                        mcb_enqueue(&dcb[policy].response, mcbp);
		dcb_unlock(policy);

		/* 
		 * POSIX ACLS  -- Now transfer the message buffer to the 
		 * daemon and release it.
		 */
		if (mcbp->size > uio->uio_resid)
			result = E2BIG;
		else if (SEC_UIOMOVE(mcbp->buffer, mcbp->size, UIO_READ, uio))
			result = EFAULT;
		spd_deallocate(mcbp->buffer);
                mcbp->buffer = NULL;
                mcbp->size = 0;


		/*
		 * POSIX ACLS  
                 * If the sender doesn't expect a response, just release
                 * the mcb.  Otherwise, if there was an error in copying
                 * the message out to the daemon, remove the mcb from the
                 * response queue, note the error in the flags fi eld, and
                 * notify the client, queueing the mcb on the wai ting
                 * queue as necessary.
		 */

		if (mcbp->flags & SPD_NO_RESPONSE)
			mcb_free(mcbp);
                else if (result != 0) {
                        dcb_lock(policy);
                        mcb_dequeue(&dcb[policy].response, mcbp, 0);
			if (mcbp->flags & SPD_CLIENT_PROCESS)
                                mcb_enqueue(&dcb[policy].waiting, mcbp);
                        thread_wakeup((vm_offset_t) mcbp);
			dcb_unlock(policy);
		}
		return(result);

	case CLIENT:

		dcb_lock(policy);
		/* If the daemon died, return an error to caller */

		if (!spd_device_open(policy)) {
			dcb_unlock(policy);
			RETERR(ENXIO)
		}

		dcb[policy].stats.client_reads++;

		/*
		 * If there is a response for this caller on the
		 * waiting queue, return it now.
		 */

check_waitq:
		if ((mcbp = find_mcb(dcb[policy].waiting)) != NULL_MCB) {
			mcb_dequeue(&dcb[policy].waiting,mcbp,0);
			dcb_unlock(policy);
			/* 
			 * POSIX ACLS  -- removed ifdef osf_source
			 */
			if (mcbp->size > uio->uio_resid) 
				result = E2BIG;
			else if (SEC_UIOMOVE(mcbp->mbuf, mcbp->size,
				UIO_READ, uio)) 
				result = EFAULT;
			/* 
			 * POSIX ACLS   
			 */
			if (mcbp->mbuf)
				spd_deallocate(mcbp->mbuf);
			mcb_free(mcbp);
			RETERR(result);
		}

		/*
		 * Nothing on the waiting queue now.  If there's a
		 * request for the caller on the response or request
		 * queues, it will eventually be put on the waiting queue.
		 * Sleep until the daemon notifies us of the completion,
		 * then check the waiting queue again.
		 */

		if (((mcbp = find_mcb(dcb[policy].response)) != NULL_MCB) ||
		        ((mcbp = find_mcb(dcb[policy].request)) != NULL_MCB)) {

			mcbp->flags |= SPD_PROCESS_ASLEEP;
			thread_sleep((vm_offset_t) mcbp,
				simple_lock_addr(dcb[policy].lock), FALSE);

			if (daemon_termed(policy, mcbp)) /* quit if daemon  */
				RETERR(ENXIO)    /* terminated during sleep */

			dcb_lock(policy);
			goto check_waitq;	 /* take it from the top    */
		}

		/*
		 * Trying to do a client read when there's no outstanding
		 * requests for this process.  This can presumably only
		 * happen if the caller does a read without a prior write.
		 * Rather than hanging forever sleeping uninterruptibly, just
		 * complain and give up.
		 */

		dcb_unlock(policy);
		printf("spd: mcb not found in read (%s)\n", u.u_comm);
		RETERR(EIO); /* XXX What's right here ? */
	}
}

/*
 *	spdwrite()-For the daemon device, a write is a response to a
 *	request message read earlier. For the client process, a write
 *	is a message request that is to be buffered and delivered to
 *	the daemon process.  Invoked only through the cdevsw switch
 *
 * 	POSIX ACLS  -- removed ifdef osf_source
 */

spdwrite(dev, uio, flag)
	dev_t		dev;
	struct uio	*uio;
{
	register struct message_cblk *mcbp;
	register int policy = POLICY(dev);
	register struct spd_message_header *mhdrp;
	register char *buffer;
	ulong sync_id;
	int	count;

	switch( DEVICE(dev) ) {

	   case DAEMON:

		count = uio->uio_resid;
		buffer = spd_allocate(count);
		if (SEC_UIOMOVE(buffer, count, UIO_WRITE, uio))
		{
			spd_deallocate(buffer);
			RETERR(EFAULT)
		}

		mhdrp = (struct spd_message_header *) buffer;
		sync_id = mhdrp->sync_id;

	/* Search the response queue for mcb of request  */

		dcb_lock(policy);
		dcb[policy].stats.daemon_writes++;

		mcbp = dcb[policy].response;

		while((mcbp != NULL_MCB) && (mcbp->sync_id != sync_id))
			mcbp = mcbp->flink;

		if (mcbp == NULL_MCB)
			panic("spd: error on sync_id match in mcb");

		mcbp->mbuf = buffer;
		mcbp->size = count;

		if (mcbp->flags  & SPD_CLIENT_PROCESS) {
			mcb_dequeue(&dcb[policy].response,mcbp,0);
			mcb_enqueue(&dcb[policy].waiting,mcbp);
			if (mcbp->flags & SPD_PROCESS_ASLEEP)
				thread_wakeup((vm_offset_t) mcbp);
		}
		else {
			thread_wakeup((vm_offset_t) mcbp);
		}
		dcb_unlock(policy);
		return(0);

	   case CLIENT:

		/* If the daemon died, return an error to caller */

		if (!spd_device_open(policy))
			RETERR(ENXIO)


		mcbp = mcb_alloc(policy);
		mcbp->size = uio->uio_resid;
		buffer = spd_allocate(mcbp->size);
		mcbp->buffer = buffer;

		if (SEC_UIOMOVE(buffer, uio->uio_resid, UIO_WRITE, uio))
		{
			spd_deallocate(buffer);
			mcb_free(mcbp);
			RETERR(EFAULT)
		}

		mcbp->flags = SPD_CLIENT_PROCESS;
		dcb_lock(policy);
		dcb[policy].stats.client_writes++;
		submit_req_to_daemon(policy, mcbp);
		dcb_unlock(policy);

		return(0);

	}
}

/*
 * This code does most of the work for simple ioctl requests to
 * a policy daemon.
 */

struct message_cblk *
spd_submit_ioctl(policy, cmd)
register unsigned int policy;
int cmd;
{
	register mhdr_t *mhdr;
	register struct message_cblk *mcbp;

	/* Build the message to send to the daemon */

	mhdr = (mhdr_t *) spd_allocate(sizeof(mhdr_t));
	mhdr->msg_type = cmd;
	mhdr->error_code = 0;
	mhdr->mh_flags = 0;

	/*
	 * Allocate a message control block to wait for the response.
	 * Submit the request to the daemon, and wait for it to complete.
	 */

	mcbp = mcb_alloc(policy);
	mcbp->buffer = (char *) mhdr;
	mcbp->size = sizeof(mhdr_t);
	mcbp->flags = SPD_CLIENT_PROCESS | SPD_PROCESS_ASLEEP;
	dcb_lock(policy);
	submit_req_to_daemon(policy, mcbp);
	thread_sleep((vm_offset_t) mcbp,
		simple_lock_addr(dcb[policy].lock), FALSE);
	return(mcbp);
}

/*
 *	spdioctl()-send an ioctl message to the specified policy daemon.
 *	Ioctl calls are only permitted to the client minor device.
 */

spdioctl(dev, cmd, arg, flags)
int dev, cmd, flags;
char *arg;
{
	register struct message_cblk *mcbp;
	register int policy = POLICY(dev);
	register mhdr_t *mhdr;
	struct sp_init sp_init;
	struct sp_inv_tag sp_inv;
	struct sp_set_cache_size sp_set;
	int i, size;
	int result;

	if (!security_is_on) {
		RETERR(EACCES)
	}

	/*
	 * ioctl operations must be performed on the proper minor device.
	 * Eject anyone who won't abide by house rules.
	 */

	switch(cmd) {
	  case SPIOC_SHUTDOWN:
	  case SPIOC_IMMUNE:
	  case SPIOC_GET_STATS:
		if (DEVICE(dev) != CLIENT)
			RETERR(EINVAL)
		break;
	  case SPIOC_EXCL:
	  case SPIOC_INIT:
		if (DEVICE(dev) != DAEMON)
			RETERR(EINVAL)
		break;
	  case SPIOC_GETCONF:
		if (minor(dev) != SPD_CONTROL_MINOR)
			RETERR(EINVAL)
		break;
	  default:		/* unknown ioctl */
		RETERR(EINVAL)
	}

	/*
	 * The ioctl is reasonable for this device.  OSF
	 * ioctls that use "arg" need to indirect through it.
	 */

	switch(cmd) {
	   case SPIOC_GETCONF:
	   case SPIOC_INIT:	
	   case SPIOC_GET_STATS:
		arg = *(char **) arg;
	}

	/*
	 * Allow common code for error paths by setting these.
	 */

	result = 0;
	mcbp = (struct message_cblk *) 0;

	switch(cmd) {

	   case SPIOC_GETCONF:

		/* Return policy configuration or error if not present */

		if (copyin(arg, &sp_init, sizeof(struct sp_init))) {
			result = EFAULT;
			break;
		}

		/* Find the security policy based on passed magic */

		for(i=0; i < spolicy_cnt; i++) {
			if (sp_init.magic == sp_switch[i].sw_magic) {
				sp_init.policy = i;
				break;
			}
		}

		if (i >= spolicy_cnt) {	/* if no such policy */
			result = ENXIO;
			break;
		}

		/* Fill in the sp_init structure for the policy */

		sp_init.spminor = sp_init.policy * 2;
		sp_init.subj_tag_count = 
			sp_switch[sp_init.policy].sw_subj_count;
		sp_init.obj_tag_count = 
			sp_switch[sp_init.policy].sw_obj_count;
		sp_init.first_subj_tag = 
			sp_switch[sp_init.policy].sw_first_subj_tag;
		sp_init.first_obj_tag = 
			sp_switch[sp_init.policy].sw_first_obj_tag;

		/* Copy the structure back to the caller */

		if (copyout(&sp_init, arg, sizeof(struct sp_init)))
			result = EFAULT;
		break;

		/*
		 * Make the device exclusive to the daemon.
		 * The dcb lock protects against races related to
		 * SPD_DEVICE_OPEN.
		 */

	   case SPIOC_EXCL:
		dcb_lock(policy);
		if (spd_device_open(policy)) {
			dcb_unlock(policy);
			result = EEXIST;
			break;
		}
		dcb[policy].flags |= SPD_DEVICE_OPEN;
		dcb[policy].pid = u.u_procp->p_pid;
		dcb[policy].procp = u.u_procp;
		dcb_unlock(policy);
		PROC_LOCK(u.u_procp);
		u.u_procp->p_flag |= SSYS; /* mark as unkillable */
		PROC_UNLOCK(u.u_procp);
		break;

	   case SPIOC_INIT:	/* Get the configuration information */

		if (copyin(arg, &sp_init, sizeof(struct sp_init))) {
			result = EFAULT;
			break;
		}

		/*
		 * Verify that the policy specified in the configuration
		 * structure corresponds to this device
		 */

		if (sp_switch[policy].sw_magic != sp_init.magic) {
			result = ENXIO;
			break;
		}

		/*
		 * Initialize the decision cache for this policy daemon.
		 * Only one thread at a time is allowed in dcache_init(),
		 * even in the uniprocessor case, in order to prevent
		 * various races related to pending in kalloc().  In
		 * practice, collisions here should *very* rarely, if ever,
		 * happen.  We lock here instead of in dcache_init() in
		 * order to simplify dcache_init() error paths.
		 */

		lock_write(&spioc_init_lock);
		result = dcache_init(policy, sp_init.cache_size,
		    sp_init.cache_width);
		lock_done(&spioc_init_lock);
		break;

	   case SPIOC_IMMUNE:
		set_shutdown_pgrp(dcb[policy].procp);
			/* fall into */
	   case SPIOC_SHUTDOWN:

		cmd = cmd == SPIOC_IMMUNE ?
			 SPD_PREPARE_SHUTDOWN : SPD_SHUTDOWN;
		mcbp = spd_submit_ioctl(policy, cmd);

		/* Check to see if terminated during sleep */

		if (daemon_termed(policy, mcbp))
			result = ENXIO;
		break;

	   case SPIOC_GET_STATS:
#ifdef notdef	/* daemon needs to be taught to deal with this msg */
		mcbp = spd_submit_ioctl(policy, cmd);

		/* Check to see if terminated during sleep */

		if (daemon_termed(policy, mcbp)) {
			result = ENXIO;
			break;
		}

		/*
		 * Copyout the security policy module portion of the
		 * statistics.  Note that the copyout may fault.  It
		 * would therefore be a good idea to copy the stats to
		 * a temporary buffer, then do the copyout from the
		 * buffer to avoid having to hold locks for a long time,
		 * or to avoid getting inconsistent stats.
		 */

		if (copyout(&sp_mod_stats[policy], arg, SP_MOD_STAT_SIZE)) {
			result = EFAULT;		  
			break;
		}

		/* Copyout the database manager stats returned in message */

		if (copyout(mcbp->mbuf, (int) arg + SP_MOD_STAT_SIZE,
					mcbp->size)) {
			result = EFAULT;
			break;
		}
#endif

		break;

	   default:		/* unknown ioctl */
		result = EINVAL;
		break;
	}

	if (mcbp) {
		dcb_lock(policy);
		mcb_dequeue(&dcb[policy].waiting,mcbp,0);
		dcb_unlock(policy);
		spd_deallocate(mcbp->mbuf);
		mcb_free(mcbp);
	}
	RETERR(result);
}

/*
 *	spd_send_message()-called by a kernel security module to send a
 *	request message to the daemon and wait for the response to return.
 */

char *
spd_send_message(buffer, size, policy)
char *buffer;
int *size, policy;
{
	register struct message_cblk *mcbp;
	register char *buffer_temp;

	dcb[policy].stats.send_message++;

	/* Insure that the daemon is operating for the policy before
	 * allowing any messages to be queued else the process will hang.
	 */

	if ((spd_init == 0) || (!spd_device_open(policy))) {

	/* Must free the message since the daemon will never pick it up */

		spd_deallocate(buffer);
		return((char *) 0);
	}

	/*
	 * Build the message to request action from daemon
	 * MP NOTE: Possible race conditions here with daemon termination
	 * between the time we checked above and the time we send the message.
	 * For now, these are ignored, since daemon termination is considered
	 * fatal to the system.
	 */

	mcbp = mcb_alloc(policy);
	mcbp->buffer = buffer;
	mcbp->size = *size;
	dcb_lock(policy);
	submit_req_to_daemon(policy, mcbp);
	thread_sleep((vm_offset_t) mcbp,
		simple_lock_addr(dcb[policy].lock), FALSE);

	/* Check to see if terminated during sleep */

	if (daemon_termed(policy, mcbp)) {
		return((char *) 0);
	}

	buffer_temp = mcbp->mbuf;
	*size = mcbp->size;
	dcb_lock(policy);
	mcb_dequeue(&dcb[policy].response,mcbp,0);
	dcb[policy].stats.rcv_message++;
	dcb_unlock(policy);
	mcb_free(mcbp);
	return(buffer_temp);
}

/*
 * Increment one of the spd statistics counters.  This is really
 * overkill, since the stats don't have to be all that precise.
 */

spd_stats_incr(p)
int *p;
{
	int i;

	simple_lock(&spd_stats.lock);
	i = *p;
	*p = ++i;
	simple_unlock(&spd_stats.lock);
}

/*
 *	spd_allocate()-allocate the requested size buffer for a message
 */

char *
spd_allocate(size)
int size;
{
	register char *p;

	/* Insure that the driver is open for business */

	if (!spd_init)
		panic("spd_allocate: called prematurely");

	/*
	 * Each version of UNIX seems to have its own fancy way of
	 * dealing with memory exhaustion.  In practice, it should very
	 * rarely happen here, and doesn't justify a lot of machine
	 * specific code.  If you change this, note that many callers
	 * aren't prepared to deal with allocation failures.
	 */

	size += sizeof(int);	/* account for size word */
	while ((p = (caddr_t) kalloc(size)) == (caddr_t) 0) {
		spd_stats_incr(&spd_stats.bufwaits);		
		sleep(&lbolt, PZERO-1);
	}		
	spd_stats_incr(&spd_stats.allocates);

	/*
	 * Stuff the size of the allocated region into the first word.
	 * This allows spd_deallocate() to free the memory later without
	 * having the caller have to know more than the address of the
	 * buffer to be freed.
	 */
	
	*(int *) p = size;
	p += sizeof(int);
	return(p);
}

/*
 *	spd_deallocate()-deallocate the message block that is associated
 *	with the buffer address passed as an argument.
 *	spd_allocate() saved the size of the region in the word before
 *	what our caller thinks is the start of the buffer being freed.
 */

spd_deallocate(buffer)
char *buffer;
{
	register int size;
	buffer -= sizeof(int);		/* point to real start of buffer */
	size = *(int *) buffer;		/* get the size of the buffer    */
	if (*(int *) buffer == -1)	/* panic if buffer is free       */
		panic("spd_deallocate: freeing free region");
	*(int *) buffer = -1;		/* mark it free			 */
	spd_stats_incr(&spd_stats.deallocates);
	kfree(buffer, size);		/* send it back to the free lists*/
}

/*
 *	mcb_init()-initialize the linked list of mcb's to prepare for
 *	dynamic allocation and deallocation by the driver.
 */

decl_simple_lock_data(,free_mcb_lock)	/* lock for mcb free lists */

mcb_init()
{
	register struct message_cblk *mcbp;
	register int i, nmcb;

	nmcb = nproc - 1;

	simple_lock_init(&free_mcb_lock);
	mcb_freelist.flink = &mcb[0];
	mcb[nmcb].flink = NULL_MCB;

	for(i=0, mcbp = mcb; i < nmcb; i++, mcbp++)
		mcbp->flink = &mcb[i + 1];
}

/*
 *	mcb_alloc()-allocate an mcb entry returning its pointer to the
 *	caller.
 *	XXX This shouldn't panic, but it's better than returning
 *	a NULL pointer until our callers are taught to deal with the error.
 */

struct message_cblk *
mcb_alloc(policy)
unsigned policy;
{
	register struct message_cblk *mcbp;

	simple_lock(&free_mcb_lock);
	if ((mcbp = mcb_freelist.flink) == NULL_MCB) {
		simple_unlock(&free_mcb_lock);
		panic("spd: out of Message Control Blocks");
		return(NULL_MCB);
	}

	mcb_freelist.flink = mcbp->flink;	/* link to next */
	simple_unlock(&free_mcb_lock);
	mcbp->flags = 0;
	mcbp->time = SYSTEM_TIME;
	mcbp->threadp = current_thread();
	mcbp->pid = u.u_procp->p_pid;
	mcbp->policy = policy;
	return(mcbp);
}

/*
 *	mcb_free()-free the mcb pointed to by mcbp
 */

mcb_free(mcbp)
register struct message_cblk *mcbp;
{
	simple_lock(&free_mcb_lock);
	mcbp->flink = mcb_freelist.flink;
	mcb_freelist.flink = mcbp;
	simple_unlock(&free_mcb_lock);
}

/*
 *	find_mcb()-locate the message control block for the process and
 *	the buffer address.  Callers must hold the dcb lock.
 *	This is called only by spdread(), to tell when a response from
 *	the daemon is ready.
 */

struct message_cblk *
find_mcb(mcbp)
struct message_cblk *mcbp;

{
	thread_t mythread = current_thread();

	while((mcbp != NULL_MCB) &&
	      (mcbp->threadp != mythread) &&
	      (mcbp->pid != u.u_procp->p_pid))
		mcbp = mcbp->flink;
	return(mcbp);	/* may be NULL_MCB */
}

/*
 *	mcb_enqueue()-enqueue the mcb whose address is passed to the
 *	sepcified message block queue.  Callers include:
 *	submit_req_to_daemon()  - adds to the request queue
 *	spdread(DAEMON)		- adds to the response queue
 *	spdwrite(DAEMON)	- adds to the waiting queue
 */

mcb_enqueue(q, mcbp)
register struct message_cblk **q, *mcbp;
{
	register struct message_cblk *work_mcbp;

	if (*q == NULL_MCB) {
		*q = mcbp;
		mcbp->flink = NULL_MCB;
	}
	else {
		work_mcbp = *q;
		while(work_mcbp->flink != NULL_MCB)
			work_mcbp = work_mcbp->flink;

		work_mcbp->flink = mcbp;
		mcbp->flink = NULL_MCB;
		mcbp->blink = work_mcbp;
	}
}

/*
 *	mcb_dequeue()-remove the specified mcb form the queue
 *	Callers must hold the appropriate dcb lock.
 */

mcb_dequeue(q,mcbp,flag)
register struct message_cblk **q, *mcbp;
int flag;
{
	register struct message_cblk *work_mcbp;

	/* Check to see if mcb is at the head of the queue */

	if (*q == mcbp) {
		if (mcbp->flink != NULL_MCB)
			mcbp->flink->blink = mcbp->blink;
		*q = mcbp->flink;
	}

	/* Search the queue to locate the mcb */

	else {
		work_mcbp = *q;

		while((work_mcbp != NULL_MCB) &&
		      (work_mcbp != mcbp))
			work_mcbp = work_mcbp->flink;

		/*
		 * This printf can only happen when we're called from
		 * daemon_termed().  It seems a bit silly...
		 */

		if (work_mcbp != mcbp) {
			if (flag)
				printf("spd: cannot locate mcb to dequeue\n");
			return;
		}

	/* Delete the entry from the queue */

		if (work_mcbp->flink != NULL_MCB)
			work_mcbp->flink->blink = work_mcbp->blink;
		work_mcbp->blink->flink = work_mcbp->flink;
	}
}

#endif /*}*/
