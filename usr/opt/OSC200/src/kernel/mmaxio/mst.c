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
static char	*sccsid = "@(#)$RCSfile: mst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:34 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989,1990 by Encore Computer Corporation.
 * All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#include "mmax_debug.h"

/*

 *
 */
#ifdef	SCCS

#endif

/*1
 *  TAPE driver for MULTIMAX/EMC.
 *
 *  This code comprises the top level generic tape driver.
 *  It communicates via 'crq' mechanisms to an EMC board on a multimax.
 *
 *  This file was created from ms.c and ms_subr.c in splitting the
 *  disk and tape drivers as part of the OSF loadable device driver
 *  project.
 */

#include <sys/param.h>
#include <sys/user.h>

#include <sys/proc.h>
#include <mach/boolean.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/dk.h>
#include <kern/queue.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <sys/syslog.h>
#include <kern/sched_prim.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmaxio/msdefs.h>
#include <mmaxio/msioctl.h>
#include <mmaxio/elog.h>
#include <mmaxio/ms_dev.h>
#include <mmaxio/crqdefs.h>
#include <mmax/cpu.h>
#include <sys/lock_types.h>
#include <mmax/isr_env.h>
#include <mmax/handler.h>
#include <sys/conf.h>
#include <sys/unix_defs.h>
#include <sys/sysconfig.h>

#include <emc.h>
#include <msd.h>
#include <mst.h>

#include <kern/assert.h>
#include <kern/thread.h>

extern struct devaddr emc_devaddr[];
extern struct subdevaddr mst_subdevaddr[];

ms_struct_t	mst_struct[NMST];

int		ms_physio();

crq_ms_xfer_msg_t *ms_getcmd();

/*
 * Error message for logging
 */
static char tape_rdy_err[] =
	"ms_init_tape: test_ready err: dev 0x%x, sts 0x%x\n";

/*
 * Forward function references
 */
int mststrategy();
int mstopen(), mstclose(), mstioctl();
int mstblkopen(), mstblkclose(), mstread(), mstwrite();

/*
 * External function references
 */
int nodev(), nulldev(), seltrue();

/*
 * Device switch structures for dynamic configuration
 */
struct bdevsw mst_bentry = {
	mstblkopen,
	mstblkclose,
	(int (*)())mststrategy,
	nulldev,
	nulldev,
	1,
	nodev,
#if SER_COMPAT
	FUNNEL_NULL
#endif
};
struct cdevsw mst_centry = {
	mstopen,
	mstclose,
	mstread,
	mstwrite,
	mstioctl,
	nodev,
	nodev,
	0,
	seltrue,
	nodev,
#if SER_COMPAT
	FUNNEL_NULL
#endif
};

dev_t mst_blkdev[NMST<<MS_PART_BITS];
dev_t mst_chardev[NMST<<MS_PART_BITS];
ihandler_id_t *mst_ids[NMST<<MS_PART_BITS];
int mst_blkdevcnt = 0;
int mst_chardevcnt = 0;
int mst_blkquerydev = 0;
int mst_charquerydev = 0;
int mst_firstcfg = TRUE;

static lock_data_t	mst_configure_lock;
#define MST_CONF_LOCK() do { \
	static int firstcall = 1; \
\
	if (firstcall) { \
		lock_init(&mst_configure_lock, TRUE); \
		firstcall = 0; \
	} \
	lock_write(&mst_configure_lock); \
} while (0)
#define	MST_CONF_UNLOCK()	lock_write_done(&mst_configure_lock)

struct mmax_devconf mst_devconf_in = {
	0,			/* intr level */
	NODEV,			/* preferred char major */
	NODEV,			/* preferred block major */
	0,			/* first minor */
	IH_VEC_DYNAMIC_OK,	/* config flags */
	0,			/* error return info */
};

struct mmax_devconf mst_devconf_out;

mst_callconfigure()
{
	long minor, status;

	mst_devconf_in.mdc_bmajnum = NODEV;
	mst_devconf_in.mdc_cmajnum = NODEV;
	mst_callconfigure_once(&mst_devconf_in, &mst_devconf_out, &status);
	return (status);
}

mst_callunconfigure()
{
	long minor, status;

	mst_callunconfigure_once(&status);
	return (status);
}

static
mst_callconfigure_once(inbuf, outbuf, pstatus)
struct mmax_devconf *inbuf, *outbuf;
long *pstatus;
{
	*pstatus = mst_configure(SYSCONFIG_CONFIGURE, inbuf, sizeof (*inbuf), 
			outbuf, sizeof (*outbuf));
	if (*pstatus == ESUCCESS) {
		printf("mst: installed char driver as major device %d\n", 
			outbuf->mdc_cmajnum);
		printf("mst: installed block driver as major device %d\n",
			outbuf->mdc_bmajnum);
	}
	else
		printf("mst: failed to install drivers, code = %d\n",
			*pstatus);
}

static
mst_callunconfigure_once(pstatus)
long *pstatus;
{

	mst_devconf_in.mdc_bmajnum = major(mst_blkdev[0]);
	mst_devconf_in.mdc_cmajnum = major(mst_chardev[0]);
	*pstatus = mst_configure(SYSCONFIG_UNCONFIGURE,
		&mst_devconf_in, sizeof (struct mmax_devconf),
		&mst_devconf_out, sizeof (struct mmax_devconf));
	if (*pstatus == ESUCCESS)
		printf("mst: deinstalled drivers (former device  b %d, c %d)\n",
			mst_devconf_in.mdc_bmajnum, mst_devconf_in.mdc_cmajnum);
	else
		printf("mst: failed to deinstall drivers, code = %d\n",
			*pstatus);
}

/*#endif*/

/*
 * Return number of devices already configured at major number
 * indicated by inbuf.  If major number in inbuf is NODEV, then
 * return value will be the number of devices so far configured
 * (i.e., NODEV matches any previous major number used).
 */
mst_chkmajor(devlst, devcnt, flags, majnum)
dev_t *devlst;
int devcnt, flags;
dev_t majnum;
{
	int i, rv;

	rv = 0;
	if (!(flags & IH_DRV_NEWMAJOR)) {
		if (majnum == NODEV)
			rv = devcnt;
		else
			for (i = 0; i < devcnt; ++i)
				if (major(devlst[i]) == majnum) {
					++rv;
				}
	} 
	return (rv);
}

/*
 * Indicate whether device number indicated by inbuf has
 * already been seen.  NODEV as the major number in inbuf
 * will match any previous major number.
 */
mst_chkminor(devlst, devcnt, majnum, minnum)
dev_t *devlst;
int devcnt;
dev_t majnum, minnum;
{
	int i, rv;

	rv = 0;
	for (i = 0; i < devcnt; ++i)
		if ((majnum == NODEV || majnum == major(devlst[i])) &&
			minor(devlst[i]) == minnum) {
			++rv;
		}
	return (rv);
}

/*
 * Indicate whether device described by inbuf has already been
 * initialized.
 */
mst_chksubunit(devlst, devcnt, minnum)
dev_t *devlst;
int devcnt;
dev_t minnum;
{
	int i, rv;

	rv = 0;
	for (i = 0; i < devcnt; ++i)
		if (MS_SUBUNIT(minor(devlst[i])) == MS_SUBUNIT(minnum)) {
			++rv;
		}
	return (rv);
}

/*
 * Delete device number in inbuf from devlst, if present.
 */
void
mst_deldev(devlst, pdevcnt, majnum, minnum)
dev_t *devlst;
int *pdevcnt;
dev_t majnum, minnum;
{
	int i, j;
	int oldcnt = *pdevcnt;
	dev_t devno = makedev(majnum, minnum);

	for (i = 0; i < oldcnt; ++i)
		if (devlst[i] == devno) 
			break;
	if (i < oldcnt) {
		--oldcnt;
		for (j = i; j < oldcnt; ++j)
			devlst[j] = devlst[j + 1];
	}
	*pdevcnt = oldcnt;
}

/*
 * Dynamic configuration function for tape driver.  This function
 * expects to be called once for each interface (block or char)
 * to be configured for each physcial device (or subunit, in
 * Multimax terms).  Thus it is called separately to configure 
 * the char and block interface for each device.
 *
 * The function distinguishes between operations done on a per-
 * major number basis (accessing the device switches), and operations
 * done on a per-subunit basis (interrupt handling).  Some support
 * is present for multiple major numbers (and hence multiple
 * switch entries) per device.  This support is activated using
 * the major number passed in via inbuf, or using the IH_DRV_NEWMAJOR
 * mdc_flags value.
 */
mst_configure(op, inbuf, siz_in, outbuf, siz_out)
sysconfig_op_t			op;
register struct mmax_devconf	*inbuf, *outbuf;
size_t				siz_in, siz_out;
{
	extern int			sys_resolve();
	extern void			msintr();
	register ihandler_t		*ihp;
	register struct devaddr		*devadr;
	register struct subdevaddr	*subadr;
	register crq_t			*crqp;
	register ihandler_id_t		*id;
	register dev_t			bdevno, cdevno, v_maj, n;
	int				i, size, rv, minnum;
	ms_struct_t			*ms;
	struct mmax_devconf		tmpbuffer;
	struct mmax_devconf		*tmpbuf = &tmpbuffer;
	int				error;

	/*
	 *  Lock the routine.
	 */
	MST_CONF_LOCK();
	error = ESUCCESS;
	minnum = inbuf->mdc_minnum;
	/*
	 * Make a scratch copy of input parameters.
	 */
	bcopy((caddr_t)inbuf, (caddr_t)tmpbuf, siz_in);
	/*
	 * Initialize all per-subunit locks now, if we've
	 * never been here before.
	 */
	if (mst_firstcfg == TRUE) {
		for (i = 0; i < NMST; ++i)
			lock_init(&mst_struct[i].ms_openlock, TRUE);
		mst_firstcfg = FALSE;
	}
	if (op == SYSCONFIG_CONFIGURE) {
		/*
		 * Verify that the required controller is present
		 * for the given device number.
		 */
		ms = &mst_struct[MS_SUBUNIT(minnum)];
		subadr = &mst_subdevaddr[MS_SUBUNIT(minnum)];
		v_maj = subadr->s_slotunit;
		if (v_maj >= NEMC) {	/* Board not present via autoconf() */
			error = ENXIO;
			goto err_lst;
		}
		devadr = &emc_devaddr[v_maj];
		if(devadr->v_valid != DEV_INITIALIZED ||
			devadr->v_chan == -1) {	/* Board not configured */
			error = ENXIO;
			goto err_lst;
		}

		/*
		 * We have a configured board for this driver.
		 * Continue driver installation by checking the
		 * range of the major number and whether the number
		 * is currently in use.
		 */

		/*
		 * If the major number indicated in inbuf is not
		 * already in device switch, call the appropriate
		 * add function to register the driver proper at
		 * that number.  Set tmpbuf->mdc_majnum with
		 * actual devno used.  If the driver add failed,
		 * return the error.
		 *
		 * If the major number indicated is already configured,
		 * then check the full device number (major + minor).
		 * Calls for new minor numbers are permitted, but do
		 * nothing save adding the full device number to the list
		 * of those encountered.  A second call for a given minor
		 * number results in an error.
		 *
		 * NB:  if (needless) per-minor configure calls made,
		 * then matching per-minor unconfigure calls must be
		 * made, or our bookkeeping will be messed up.
		 */
		bdevno = inbuf->mdc_bmajnum;
		cdevno = inbuf->mdc_cmajnum;
		if (bdevno != NODEV)
			bdevno = makedev(bdevno, minnum);
		if (cdevno != NODEV)
			cdevno = makedev(cdevno, minnum);
		if (!mst_chkmajor(mst_blkdev,
			mst_blkdevcnt, inbuf->mdc_flags, inbuf->mdc_bmajnum)) {
			bdevno = bdevsw_add(bdevno, &mst_bentry);
			if (bdevno == NODEV) {
				error = ENODEV;
				goto err_lst;
			}
		}
		if (!mst_chkmajor(mst_chardev,
			mst_chardevcnt, inbuf->mdc_flags, inbuf->mdc_cmajnum)) {
			cdevno = cdevsw_add(cdevno, &mst_centry);
			if (cdevno == NODEV) {
				error = ENODEV;
				goto err_lst;
			}
		}
		if (mst_chkminor(mst_blkdev,
			mst_blkdevcnt, inbuf->mdc_bmajnum, minnum) ||
			mst_chkminor(mst_chardev, mst_chardevcnt,
			inbuf->mdc_cmajnum, minnum)) {
			error = EBUSY;
			goto err_lst;
		}

		/*
		 * If the physical device (subunit) named by inbuf hasn't
		 * been initialized, do so.  Otherwise, skip to post-
		 * initialization bookkeeping code.
		 */
		if (bdevno != NODEV)
			tmpbuf->mdc_bmajnum = major(bdevno);	
		else {
			ASSERT(mst_blkdevcnt != 0);
			tmpbuf->mdc_bmajnum = major(mst_blkdev[mst_blkdevcnt - 1]);
		}
		if (cdevno != NODEV)
			tmpbuf->mdc_cmajnum = major(cdevno);	
		else {
			ASSERT(mst_chardevcnt != 0);
			tmpbuf->mdc_cmajnum = major(mst_chardev[mst_chardevcnt - 1]);
		}
		bdevno = makedev(tmpbuf->mdc_bmajnum, minnum);
		cdevno = makedev(tmpbuf->mdc_cmajnum, minnum);
		if (mst_chksubunit(mst_blkdev, mst_blkdevcnt, minnum) ||
			mst_chksubunit(mst_chardev, mst_chardevcnt, minnum)) {
			ASSERT(mst_chardevcnt != 0);
			id = mst_ids[mst_chardevcnt - 1];
			goto out;
		}
		subadr->s_valid = DEV_VALID;
		lock_write(&ms->ms_openlock);
		ms_init_slot(subadr->s_slotunit,
			MS_SUBUNIT(minnum), CLASS_TAPE);

		/*
		 * Everything checks out, build the interrupt structure.
		 *
		 * NOTE: The following NULL test is
		 * an unimplemented error return from
		 * kalloc() and SHOULD NOT HAPPEN!
		 * It is here for the time when a kalloc() failure
		 * does not result in a panic().
		 */
		if(((ihandler_t *)ihp = kalloc(sizeof(ihandler_t))) == NULL) {
			printf("mst_configure: cannot allocate an ihandler_t\n");
			error = ENOMEM;
			goto err_2;
		}
		bzero((caddr_t)ihp, sizeof(ihandler_t));
		ihp->ih_level = inbuf->mdc_level;
		ihp->ih_handler = msintr;
		ihp->ih_resolver = sys_resolve;
		ihp->ih_next = NULL;
#if	SER_COMPAT
		if (inbuf->mdc_flags & IH_DRV_NONPARALLEL) {
			ihp->ih_funnel = !FUNNEL_NULL;
		}
		else
			ihp->ih_funnel = FUNNEL_NULL;
#endif
		ihp->ih_rparam[0].intparam = inbuf->mdc_flags;
		ihp->ih_stats.ihs_type = INTR_DEVICE;

		/*
		 * Initialize the crq and itable entry for this device.
		 * Then call handler_add() to register the interrupt
		 * handler, recording the returned id.  Fill in
		 * inbuf->mdc_level with level actually used.
		 */
		ihp->ih_hparam[0].intparam = (int)ms;
		(void)ms_open_crq(ms, FALSE);
		crqp = &ms->ms_crq;
		if ((id = handler_add(ihp)) == NULL) {
			if (!(error = ihp->ih_rparam[0].intparam))
				error = -1;
			goto err_3;
		}
		tmpbuf->mdc_level = ihp->ih_level;

		/*
		 * Initialize the crq header with the interrupt info.
		 */
		MAKEVECTOR(&crqp->crq_master_vect, MASTER_CLASS, ihp->ih_level);

		/*
		 * Open up communications with the device.
		 */
		if ((n = create_chan(crqp))) {
			error = n;
			goto err_all;
		}


		/*
		 * Setup flow control word if the DRV_NONPARALLEL
		 * flag is set.  Do the serialization prior to calling
		 * any initialization functions.
		 * Not needed for this driver.
		 */

		/*
		 * Finally, call handler_enable() to enable interrupt handling.
		 */
		if (handler_enable(id)) {
			error = EINTR;
			goto err_all;
		}
		lock_write_done(&ms->ms_openlock);

out:
		mst_ids[mst_chardevcnt] = id;
		mst_blkdev[mst_blkdevcnt++] = bdevno;
		mst_chardev[mst_chardevcnt++] = cdevno;
	} else if (op == SYSCONFIG_UNCONFIGURE) {
		tmpbuf->mdc_flags &= ~IH_DRV_NEWMAJOR;
		ms = &mst_struct[MS_SUBUNIT(minnum)];
		/*
		 * If subunit isn't currently closed, it can't be
		 * unconfigured.
		 */
		lock_write(&ms->ms_openlock);
		if (ms->ms_state != MST_CLOSED && ms->ms_state != MST_VALID) {
			error = EBUSY;
			goto err_1;
		}
		/*
		 * Get the major number given (must be explicit), and check
		 * for its presence in our tables.  If not there, return an
		 * error.  If there more than once, then this call just
		 * removes the full device number (major + minor) from the
		 * table, reducing the count for this major number by one.
		 * Otherwise, this major occurs only once, and it's time to
		 * to remove it from the device switch.
		 */
		bdevno = tmpbuf->mdc_bmajnum;
		cdevno = tmpbuf->mdc_cmajnum;
		if (bdevno == NODEV || cdevno == NODEV) {
			error = EINVAL;
			goto err_1;
		}
		bdevno = makedev(bdevno, 0);
		cdevno = makedev(cdevno, 0);
		rv = 0;
		if (mst_chkmajor(mst_blkdev,
			mst_blkdevcnt, inbuf->mdc_flags, tmpbuf->mdc_bmajnum) == 0 ||
			mst_chkmajor(mst_chardev,
				mst_chardevcnt, inbuf->mdc_flags, tmpbuf->mdc_cmajnum) == 0) {
			error = ENODEV;
			goto err_1;
		}
		if (mst_chkmajor(mst_blkdev,
			mst_blkdevcnt, inbuf->mdc_flags, tmpbuf->mdc_bmajnum) == 1)
			rv = bdevsw_del(bdevno);
		if (!rv &&
			mst_chkmajor(mst_chardev,
			mst_chardevcnt, inbuf->mdc_flags, tmpbuf->mdc_cmajnum) == 1)
			rv = cdevsw_del(cdevno);
		if (rv) {
			error = ESRCH;
			goto err_1;
		}
		/*
		 * If the current device number is the last one referring
		 * to this subunit, then deallocate the subunit's interrupt
		 * handling interface.  Otherwise just remove the table
		 * entry for the device number.
		 */
		if (mst_chksubunit(mst_blkdev, mst_blkdevcnt, minnum) +
			mst_chksubunit(mst_chardev,
			mst_chardevcnt, minnum) != (1+1))
			goto deldev;
		subadr = &mst_subdevaddr[MS_SUBUNIT(minnum)];
		v_maj = subadr->s_slotunit;
		id = 0;
		for (i = 0; i < mst_chardevcnt; ++i)
			if (cdevno == mst_chardev[i]) {
				id = mst_ids[i];
				break;
			}
		ASSERT(id != 0);
		if (handler_disable(id)) {
			error = EINTR;
			goto err_1;
		}

		crqp = &ms->ms_crq;
		if ((n = delete_chan(crqp))) {
			error = n;
			goto err_1;
		}
		if (handler_del(id)) {
			error = ENOENT;
			goto err_1;
		}
		ms->ms_state = MST_INVALID;
deldev:
		lock_write_done(&ms->ms_openlock);
		mst_deldev(mst_blkdev,
			&mst_blkdevcnt, tmpbuf->mdc_bmajnum, minnum);
		mst_deldev(mst_chardev,
			&mst_chardevcnt, tmpbuf->mdc_cmajnum, minnum);
	} else if (op == SYSCONFIG_QUERY) {
		/*
		 * Return the major and minor device numbers for the
		 * "next" device, as indicated by the appropriate
		 * query index.
		 */
		if (mst_blkdevcnt + mst_chardevcnt == 0) {
			error = ENODEV;
			goto err_lst;
		}
		else {
			if (mst_blkdevcnt <= mst_blkquerydev)
				mst_blkquerydev = 0;
			if (mst_chardevcnt <= mst_charquerydev)
				mst_charquerydev = 0;
			tmpbuf->mdc_bmajnum = major(mst_blkdev[mst_blkquerydev]);
			tmpbuf->mdc_cmajnum = major(mst_chardev[mst_charquerydev]);
			tmpbuf->mdc_minnum = minor(mst_blkdev[mst_blkquerydev]);
			++mst_blkquerydev;
			++mst_charquerydev;
		}
	}
	else {
		error = EPERM;
		goto err_lst;
	}
	if (outbuf) {
		size = siz_in;
		if (siz_out < size)
			size = siz_out;
		bcopy((caddr_t)tmpbuf, (caddr_t)outbuf, size);
	}
	MST_CONF_UNLOCK();
	return(0);

	/*
	 * The following are goto labels used to back out
	 * any registrations made in case of an error.
	 */
err_all:
	(void)handler_del(id);
err_3:
	(void)kfree(ihp, sizeof(ihandler_t));
err_2:
	(void)bdevsw_del(bdevno);
	(void)cdevsw_del(cdevno);
err_1:
	lock_write_done(&ms->ms_openlock);
err_lst:
	MST_CONF_UNLOCK();
	return(error);
}

/*
 * MODE to open:  screwy stuff.  Seems like 1=(FWRITE-FOPEN) means writeaccess.
 *		Can actually get called with FTRUNC+FWRITE-FOPEN which also
 *		means writeaccess.  For now, we assume nonzero is writeaccess
 *		and zero is readonly.  Protection at readonly level may not
 *		work for disk partitions unless we amend ms_struct.  Most 
 *		protection is done at higher level in fio access and alloc
 *		routines.
 *
 */

mstblkopen(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	int sts;
	return(ms_open (dev, mode, CLASS_TAPE, BLOCK_DEV));
	}

mstopen(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	int sts;
	return(ms_open (dev, mode, CLASS_TAPE, CHAR_DEV));
	}

mstblkclose(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	int sts;
	return(mst_close (dev, mode, BLOCK_DEV));
	}

mstclose(dev,mode)
	dev_t	dev;
	int 	mode;
	{
	int sts;
	return(mst_close (dev, mode, CHAR_DEV));
	}

ms_init_tape(ms,mode,dev,form)
ms_struct_t *ms;
int mode;
dev_t dev;
int form;	/* BLOCK_DEV or CHAR_DEV */
{
  bp_env_t          bp_env;
  struct buf        *bp;
  int               sts;
  crq_ms_xfer_msg_t *mscmd;

    ms->ms_state = MST_QACTIVE;
    bp = ms_getblk (&bp_env, dev);
    mscmd = ms_getcmd(bp,&sts,TRUE);
    if (sts != ESUCCESS) goto badtape1;
    sts =ms_syncio (ms, mscmd, bp, CRQOP_MS_TEST_UNIT_READY);
    if (sts != ESUCCESS) goto badtape;
	ms->ms_flags	&= ~MSF_SAWFMK;		/* clear flags */
	ms->ms_currec	 = 0;
	ms->ms_eofrec	 = 0x7fffff;
	ms->ms_owner_pid = u.u_procp->p_pid;
	ms->ms_state = MST_QACTIVE;	/* only one open allowed */
#ifdef	PARANOID
	if (mode & FWRITE) {
		/* one would hope the TEST_UNIT_READY command would 
		 * return status including presence/absence of write ring
		 */
		printf("mt_open: WRITE: hope the write wring is on\n");
	}
	/* TAPE DENSITY:
	 *   Tapes only exist on MULTMAX and the only supported tape
	 * unit does auto_select on read, and obeys the buttons on
	 * the tape cabinet for writes.  No software density selection
	 * commands exist, nor are there any status reports available,
	 * so for now, we remind the user to check the button
	 */

	if (mode & FWRITE) {
	    if (MT_HIGHDENS(dev)) {
		printf("mt_open: HIGH DENS (check buttons on tape drive?) \n");
	    } else {
		printf("mt_open: LOW DENS (check buttons on tape drive?) \n");
	    }
	}
#endif

    ms_putblk (bp, &bp_env);
    return(ESUCCESS);

badtape:
    ms_putblk (bp, &bp_env);
 badtape1:
    ms->ms_state = MST_CLOSED;
    ms_deverr(tape_rdy_err, dev, ms->ms_multidev, sts, 0);
    return(ENXIO);
}

mst_close (dev, fp_flag, form)
	dev_t dev;
	int fp_flag;
	int form;		/* BLOCK_DEV or CHAR_DEV */
	{
	int 	    sts;
	int	    subunit;
	ms_struct_t *ms;
	struct	buf	    *bp;
	bp_env_t    bp_env;
	crq_ms_xfer_msg_t   *mscmd;

	subunit = MS_SUBUNIT(dev);
	if (subunit > NMST) return (ENODEV);
	ms = &(mst_struct[subunit]);

	/* in case device somehow isn't configured, stop now */
	if (ms->ms_state == MST_INVALID)
		return (ENODEV);

	lock_write(&ms->ms_openlock);
	switch (ms->ms_state) {
	    case MST_ACTIVE:    sts = ESUCCESS;	break;
	    case MST_QACTIVE:   sts = ESUCCESS;	break;

	    case MST_CLOSED:	sts = ENXIO;	break;
	    case MST_VALID:	sts = ENXIO;	break;
	    default:		sts = ENODEV;	break;
	}
	if (sts != ESUCCESS) {
		lock_write_done(&ms->ms_openlock);
		return (sts);
	}
	bp = ms_getblk(&bp_env, dev);
	if (fp_flag & FWRITE) {
		/* write 2 filemarks */
		if (sts = ms_filemark (ms, bp, 2)) goto cleanup;
		if (!MT_NOREWIND(dev)) {
			sts = ms_rewind (ms, bp);
                } else { /* position inbetween the two tape marks */
                        sts = ms_space (ms, bp, -1, CRQOP_MT_SPACE_FMARK);
                }
	} else { /* Open for READ only */
		if (MT_NOREWIND(dev)) {
			/* Do a NOP to insure queue is drained */
			mscmd = ms_getcmd(bp,&sts,TRUE);
			if (sts != ESUCCESS) goto cleanup1;
			ms_syncio (ms, mscmd, bp, CRQOP_NOP);
		} else { 
			sts = ms_rewind (ms, bp);
		}
	}

cleanup:
	ms_putblk(bp,&bp_env);
cleanup1:
	if (!MT_NOREWIND(dev)) {
		ms->ms_flags &= ~MSF_SAWEOT;
		ms->ms_eotcnt = 0;
	}
	ms->ms_owner_pid = 0;
	ms->ms_state = MST_CLOSED;
	/* might do delete_chan on crq, and change open logic */
	lock_write_done(&ms->ms_openlock);
	return(ESUCCESS);
	}

mstread(dev, uio)
dev_t dev;
struct	uio	*uio;
{
	dev_t	subdev = MS_SUBUNIT(dev);
	int error;
	
	if (error = ms_valid(dev,CLASS_TAPE,NULL))
		return(error);

	return ms_physio(mststrategy, dev, B_READ, minphys, uio);
}

mstwrite(dev, uio)
dev_t dev;
{
	dev_t	subdev = MS_SUBUNIT(dev);
	int error;

	if (error = ms_valid(dev,CLASS_TAPE,NULL))
		return(error);

	return ms_physio(mststrategy, dev, B_WRITE, minphys, uio);
}

mstioctl(dev, cmd, arg, fp_flag)
     dev_t dev;
     int cmd, arg, fp_flag;
{
  int 	  	sts, cnt;
  struct  mtop	*mtop;
  struct  mtget *mtget;
  struct	buf  	*bp;
  bp_env_t  	bp_env;
  ms_struct_t	*ms;
  crq_ms_xfer_msg_t *mscmd;

  if (sts = ms_valid (dev, CLASS_TAPE,  &ms)) return(sts);

  sts = 0;
  bp = ms_getblk(&bp_env, dev);

  switch(cmd) {

    case MTIOCTOP:

      mtop = (struct mtop *)arg;
      cnt  =  mtop->mt_count;
      switch (mtop->mt_op) {

        case  MTWEOF:	/* Write 'cnt' end-of-file records */
	  sts = ms_filemark (ms, bp, cnt);
	  break;

	case  MTFSF:	/* 'cnt' Forward Space File:
			 * ?? should we clear SAWFMK flag 
			 */
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_FMARK);
	  break;

	case  MTBSF:	/* 'cnt' Backward Space File:
			 * ?? should we clear SAWFMK flag
			 */
	  if (cnt > 0) cnt = -cnt;
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_FMARK);
	  break;

	case  MTFSR:	/* 'cnt' Forward Space Record */
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_BLOCK);
	  break;

	case  MTBSR:	/* 'cnt' Backward Space Record */
	  if (cnt > 0) cnt = -cnt;
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_BLOCK);
	  break;

	case  MTREW:	/* Rewind */
	  sts = ms_rewind (ms, bp);
	  break;

	case  MTOFFL:	/* Rewind and put the drive offline */
	  sts = ms_rewind (ms, bp);
	  break;

	case  MTNOP:	/* No Operation: since this operation is 
			 * synchronous, it effectively flushes out
			 * buffered tape commands. Also if sequence
			 * (open, nop, close) on readonly tape is
			 * done, it effectively sequences forwards
			 * one file.
			 */
	  mscmd = ms_getcmd(bp,&sts,TRUE);
	  if (sts != ESUCCESS) break;
	  sts = ms_syncio(ms, mscmd, bp, CRQOP_NOP);
	  break;

	case  MTFSSF:	/* 'cnt' Forward Space Sequential File:
			 * searches for series on contiguous file
			 * marks 
			 */
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_SQN_FMARK);
	  break;

	case  MTBSSF:	/* 'cnt' Back Space Sequential File */
	  if (cnt > 0) cnt = -cnt;
	  sts = ms_space (ms, bp, cnt, CRQOP_MT_SPACE_SQN_FMARK);
	  break;

	case  MTEOT:	/* streamers only: not implemented */
	case  MTRETEN:	/* streamers only: not implemented */
	case  MTERASE:	/* streamers only: not implemented */
	default:
	  sts = EINVAL;
	  break;
	}
      break;  /* out of switch(cmd)  */

    case MTIOCGET:

      mtget = (struct mtget *)arg;
      mtget->mt_erreg = 0;
      mtget->mt_resid = bp->b_resid;
      mtget->mt_dsreg = 0;
      mtget->mt_type = MT_ISCSI;
      sts = ESUCCESS;
      break;

    default:
      sts = EINVAL;
      break;  /* out of switch(cmd)  */

    }

  ms_putblk(bp,&bp_env);
  return(sts);

}

mststrategy(bp)
	struct	buf *bp;
{
	int sts;
	ms_struct_t *ms;
	crq_ms_xfer_msg_t *mscmd;

	LASSERT(BUF_IS_LOCKED(bp));
	if ((sts = ms_valid(bp->b_dev,CLASS_TAPE,&ms)) != ESUCCESS) {
		goto mterror;
	}

	/* Keep returning EOF after the guy crosses the fmark. */
	if (ms->ms_flags & MSF_SAWFMK /* || bp->b_bcount > maximum */) {
		sts = ENXIO ;
		goto mterror;
	}
	/* On block devices, may need to seek to proper record.  In this
	 * case, records are always 1k; and since block devices don't
	 * support ioctl's, we don't have to account for the various
	 * ioctl seeks
	 */
	if (!(bp->b_flags & B_PHYS)) {
		struct	buf	 *bp2;
		bp_env_t bp_env2;
		long	 rec, nrecs;

		rec = bp->b_blkno / (BLKDEV_IOSIZE/DEV_BSIZE);
		if (rec > ms->ms_eofrec) { 
			sts = ENXIO;
			goto mterror;
		}
		if ((rec == ms->ms_eofrec) && (bp->b_flags & B_READ)) {
			clrbuf(bp);
			goto done;
		}
		if ( (nrecs = rec - ms->ms_currec) != 0) {
			bp2 = ms_getblk (&bp_env2, bp->b_dev);
			/* check status */
			ms_space  (ms, bp2, nrecs, CRQOP_MT_SPACE_BLOCK);
			ms_putblk (bp2, &bp_env2);
			ms->ms_currec = rec;
		}
	} /* end of special block code */
	mscmd = ms_getcmd(bp,&sts,TRUE);
	if (sts != ESUCCESS) goto mterror;
#if	MMAX_DEBUG
	ASSERT(chk_bdl(mscmd, 7));
#endif	MMAX_DEBUG
	mscmd->ms_xfer_bcnt = bp->b_resid = bp->b_bcount; 
	mscmd->ms_xfer_hdr.em_msg_hdr.crq_msg_code = 
		((bp->b_flags & B_READ) ? CRQOP_MS_READ : CRQOP_MS_WRITE);

	((crq_msg_t *) mscmd)->crq_msg_unitid = ms->ms_crq.crq_unitid;
	((crq_msg_t *) mscmd)->crq_msg_refnum = (long) bp;
	mscmd->ms_xfer_hdr.em_status_code = 0; /* EMC bug */

#if	MMAX_DEBUG
	ASSERT(chk_bdl(mscmd, 8));
#endif	MMAX_DEBUG
	if (!(bp->b_flags & B_READ)) 
		ms->ms_eofrec = ms->ms_currec + 1;

	ms_start (ms, bp, mscmd);

	ms->ms_currec++;
	goto out;
	
mterror:
	bp->b_error  = sts;
	bp->b_flags |= B_ERROR 	/* | B_STALE */;
	bp->b_resid  = bp->b_bcount;
done:
	iodone(bp);
out:
	sts = 0;
	if (bp->b_flags & B_ERROR)
		sts = (bp->b_error) ? bp->b_flags : EIO;
	return(sts);
}

/*
 * TAPE IOCTL support 
 */

ms_filemark (ms, bp, count)
	ms_struct_t	*ms;
	struct	buf	*bp;
	int		count;
{
	crq_mt_wrtfmk_msg_t	*mscmd;
	int sts;

	mscmd = (crq_mt_wrtfmk_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	/* Be nice to users who think this command writes one filemark */
	if (count <= 0) count=1;
	mscmd->mt_wrtfmk_cnt = count;
	bp->b_resid = count;
	return( ms_syncio (ms, mscmd, bp, CRQOP_MT_WRITE_FILE_MARK));
}

ms_space (ms, bp, count, opcode)
	ms_struct_t	*ms;
	struct	buf	*bp;
	int		count;
	int		opcode;		/* One of:
					 * 	CRQOP_MT_SPACE_BLOCK
				 	 *	CRQOP_MT_SPACE_FMARK
				 	 *	CRQOP_MT_SPACE_SQN_FMARK
					 */
{
	crq_mt_spc_msg_t	*mscmd;
	int sts;

	mscmd = (crq_mt_spc_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	/* Be nice to users who think this command skips one quantum */
	if (count == 0) count = 1;
	mscmd->mt_spc_cnt = count; 
	bp->b_resid = count;
	return( ms_syncio (ms, mscmd, bp, opcode));
}

ms_rewind (ms, bp)
	ms_struct_t	*ms;
	struct	buf	*bp;
	{
	crq_mt_rwnd_msg_t	*mscmd;
	int sts;

	mscmd = (crq_mt_rwnd_msg_t *)ms_getcmd(bp,&sts,FALSE);
	if (sts != ESUCCESS ) return(sts);
	return( ms_syncio (ms, mscmd, bp, CRQOP_MT_REWIND));
	}
/*	---------------  end of mst.c	---------------	*/
