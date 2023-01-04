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
static char	*sccsid = "@(#)$RCSfile: ms_comm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:19 $";
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
 */

#include "mmax_debug.h"

/*

 *
 */
#ifdef	SCCS

#endif

/*1
 *  Routines common to DISK and TAPE drivers for MULTIMAX/EMC.
 *
 *  This code comprises the routines common to the top level generic
 *  disk and tape drivers.
 */

/*
 * "GOLDEN": released to Steve Knight on 14-Aug-90
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
#include <mmax/isr_env.h>
#include <mmax/cpu.h>
#include <sys/lock_types.h>

#include <emc.h>
#include <msd.h>
#include <mst.h>

#include <kern/assert.h>
#include <kern/thread.h>


extern	ms_struct_t	msd_struct[];
extern	ms_layout_t	msd_layout[];

extern	ms_struct_t	mst_struct[];

extern struct devaddr emc_devaddr[];
extern struct subdevaddr msd_subdevaddr[];
extern struct subdevaddr mst_subdevaddr[];

#if	MMAX_DEBUG
#define	NMS_CMD		(800)
#else	MMAX_DEBUG
#define	NMS_CMD		(120*NMSD+120*NMST)
#endif	MMAX_DEBUG

crq_ms_xfer_msg_t 	buf_cmds[NMS_CMD];
mpqueue_head_t		ms_cmd_free_que;

extern	long		lbolt;

int		ms_physio();
struct  buf	sccbuf;			/* only one scc op at a time */

crq_ms_xfer_msg_t *ms_getcmd();

struct buf *bbstrategy();

/* ms_bootdev
 *  Looks up a lamp, slot, dev, unit in config tables and returns a
 *  dev_t type logical device name, or -1.
 */
dev_t
ms_bootdev(makeunit, partn)
long makeunit;	/* in form from MAKEUNIT(lamp,slot,dev,lun)  */
int  partn;     /* partition  */
{
	int emc, subunit, lemc, lunit, class;
	struct subdevaddr *sd, *sdbeg, *sdend;
	extern	int	msd_bmajor;

	emc	= GETSLOT(makeunit);
	subunit	= GETLUN(makeunit);
	for (lemc=0; lemc < NEMC; lemc++) 
		if (emc_devaddr[lemc].v_chan == emc) break;
	if (lemc >= NEMC)  return (-1);
	switch (emc_devaddr[lemc].v_valid) {
	    case DEV_INVALID:     return (-1);
	    case DEV_VALID:	  return (-1); /* crq not init*/
	    case DEV_INITIALIZED: break;	    /* A OK */
	}
	/* Now decide if we should look up as a disk or tape.
	 * Somewhat dependent on EMC device numbering.
	 */
	if ((BASE_DISKLUN <= subunit ) && (subunit <= MAX_DISKLUN)) {
	    class = msd_bmajor;
	    sdbeg = &(msd_subdevaddr[0]);
	    sdend = sdbeg + NMSD;
	} else {
	    return(-1);
	}
	/* Now See if we can find the requested device */
	for (lunit=0, sd=sdbeg; sd < sdend; sd++, lunit++) {
		if ((sd->s_valid != DEV_INVALID) && (sd->s_slotunit == lemc) && 
		    (sd->s_subunit == subunit))  {
			return(makedev(class,MS_MAKEUNIT(lunit,partn)));
		}
	}

	/* We Lost */
	return(-1);
}

ms_init_slot(logemc, logdev, class)
	int	logemc;
	int	logdev;
	int	class;
	{
	ms_struct_t	*ms;

	/*
	 * All fields not initialized here are initially zero.
	 */
	if (class == CLASS_DISK) {
		ms = &msd_struct[logdev];
		ms->ms_layout = &msd_layout[logdev];
		msd_null_layout(ms->ms_layout);
		lock_init(&ms->ms_openlock, TRUE);
		ms->ms_dkn = logdev;
	} else {
		ms = &mst_struct[logdev];
		ms->ms_dkn = -1;
	}
	ms->ms_state	= MST_VALID;	/* but still no crq */
	ms->ms_class	= class;
	ms->ms_multidev	= MAKEUNITID(0, emc_devaddr[logemc].v_dev, 0,
		((class == CLASS_DISK) ?
			msd_subdevaddr[logdev].s_subunit :
			mst_subdevaddr[logdev].s_subunit));
	ms->ms_dev = MS_MAKEUNIT(logdev, 0);
	init_bdl();
	return;	/* A_OK */
	}

ms_open(dev, mode, class, form)
	dev_t	dev;
	int 	mode;
	int	class;		/* CLASS_DISK or CLASS_TAPE */
	int	form;		/* BLOCK_DEV  or CHAR_DEV */
{
	int			sts, max;
	int			subunit;
	ms_struct_t		*ms;
	struct subdevaddr	*subadr;

	subunit = MS_SUBUNIT(dev);
	if (class == CLASS_DISK) {
		if (NMSD <= subunit)
			return (ENODEV);
		ms = &msd_struct[subunit];
		subadr = &msd_subdevaddr[subunit];
	}
	else {
		if (NMST <= subunit) {
			return (ENODEV);
		}
		ms = &mst_struct[subunit];
		subadr = &mst_subdevaddr[subunit];
	}
	/*
	 * If subunit hasn't been validated by configure
	 * function, go no farther.  This case can arise only for
	 * dynamically-configured drivers (i.e., here, tape).
	 * In this case, nothing in ms is valid till configure
	 * time.
	 */
	if (subadr->s_valid != DEV_VALID) {
		return (ENODEV);
	}
	lock_write(&ms->ms_openlock);
	sts = ESUCCESS;
	switch(ms->ms_state) {
	case MST_ACTIVE:
		sts = ESUCCESS;
		break;
	case MST_QACTIVE:
		if (MS_PART(dev) == HEADER_PART) {
			/* Allow multiple opens on non-
			 * partitioned drives only for
			 * header partitions.
			 */
			sts = ESUCCESS;
		} else {
			sts = EBUSY;	/* or EACCESS ?? */
		}
		break;
	case MST_INVALID:
		sts = ENODEV;
		break;
	case MST_VALID:
		if (class == CLASS_DISK) {
			if ((sts = ms_open_crq(ms, TRUE)) != ESUCCESS) {
				break;
			}
		}
		else { /* CLASS_TAPE */
#if 0
			/*
			 * Done at configure time.
			 */
			if ((sts = ms_open_crq(ms, FALSE)) != ESUCCESS) {
				break;
			}
#endif
		} /* fall thru */
	case MST_CLOSED:
		if (class == CLASS_DISK)
			sts = ms_init_disk(ms,mode,dev,form);
		else  /* CLASS_TAPE */
			sts = ms_init_tape(ms,mode,dev,form);
		if (sts)
			break;
		ms->ms_class = class;
		ms->ms_subclass	= 0;
		break;
	default:
		sts = ENODEV;
		break;
	}
	/*
	 * Flag new disk partitions as being open
	 */
	if ((sts == ESUCCESS) && (class == CLASS_DISK)) {
		if (form = BLOCK_DEV) 
			ms->ms_block_parts |= (1 << MS_PART(dev));
		else
			ms->ms_char_parts  |= (1 << MS_PART(dev));
	}
	lock_write_done(&ms->ms_openlock);
	return(sts);
}

void
msintr(pih)
register ihandler_t *pih;
{
	register ms_struct_t *ms = (ms_struct_t *)(pih->ih_hparam[0].intparam);
	emc_msg_t	*cmd = (emc_msg_t *)1;
	emc_msg_t	*pend_cmd;
	crq_msg_t	*attn = (crq_msg_t *)1;

	while ((cmd != NULL) || (attn != NULL)) {
		if((attn = (crq_msg_t *) rec_attn(&ms->ms_crq)) != NULL) {
		ms_attn(attn, GETSLOT(((crq_msg_t *)attn)->crq_msg_unitid),
			&ms->ms_crq);
		put_free(attn, &ms->ms_crq);
		}
	/*
	 *	Do a receive and, if we get a message, get a pointer to the context
	 *	data.
	 */
		if((cmd = (emc_msg_t *)rec_rsp(&ms->ms_crq)) != NULL) {
#if	MMAX_DEBUG
			ASSERT(chk_bdl( cmd, 2 ) == 1);
#endif	MMAX_DEBUG
			if(cmd->em_msg_hdr.crq_msg_code == CRQOP_MS_READ_RESET_STAT) {
				printf("msintr: unexpected statistics response\n ");
				/*
				 * Update stats. Then resched another stats packet.
				 *   stat_msg = (ms_rdstat_msg_t *)cmd;
				 *   if(stats != NULL) timeout(ms_stat_timer, ms, 30*hz);
				 */
			} else {
				struct	buf *bp;

				/*
				 *	Regular data xfer or ioctl commands
				 */
				bp = (struct buf *) cmd->em_msg_hdr.crq_msg_refnum;
				/*
				 *  Not all commands we've sent to the disk/tape return
				 *  completion counts.  These puppies are all the non-
				 *  masstore commands that don't return counts (cf crqdefs.h).
				 */
				switch (cmd->em_msg_hdr.crq_msg_code) {
				case CRQOP_CREATE_CHANNEL:
				case CRQOP_DELETE_CHANNEL:
				case CRQOP_ABORT_REQ:
				case CRQOP_ABORT_ALL:
				case CRQOP_LOAD:
				case CRQOP_DUMP:
				case CRQOP_EXEC_PROG:
				case CRQOP_WARM:
				case CRQOP_COLD:
				case CRQOP_NOP:
#if	MMAX_DEBUG
					if (bp->b_resid != 0) {
						printf("msintr:  resid %d on non-ms cmd code %d\n",
						   bp->b_resid, cmd->em_msg_hdr.crq_msg_code);
						panic("msintr:  non-zero resid on non-ms cmd");
					}
#endif	MMAX_DEBUG
					break;
				default:
					bp->b_resid -= cmd->em_compltn_cnt;
					break;
				}
				if (bp->b_resid < 0) {
					printf("b_resid = %d, msg_code = %d, completion count = %d\n",
					   bp->b_resid, cmd->em_msg_hdr.crq_msg_code,
					   cmd->em_compltn_cnt);
					panic("msintr: negative b_resid");
				}
# define MSCMD  ((crq_ms_xfer_msg_t *) cmd)
#ifdef	systemv_statistics
				/*
				 * Compute amount of time that device had i/o queued up.
				 * (Assumes lock held, but who cares)
				 */
# define CRQCMD ms->ms_crq.crq_cmd.dbl_fwd
				if (ms->ms_iostart && (CRQCMD == &CRQCMD)) { 
					ms->ms_devinfo.io_act  += lbolt - ms->ms_iostart;
					ms->ms_iostart = 0;
				}
				if ((cmd->em_msg_hdr.crq_msg_code == CRQOP_MS_READ) || 
				(cmd->em_msg_hdr.crq_msg_code == CRQOP_MS_WRITE)) {
					ms->ms_devinfo.ios.io_ops++;
					ms->ms_devinfo.io_bcnt += (MSCMD->ms_xfer_bcnt >> 9);
					if (ms->ms_class == CLASS_DISK) {
						/* update circular block stats records for sadp.c */
						int indx = ms->ms_index;

						ms->ms_blknum(indx) = MSCMD->ms_xfer_lbn;
						ms->ms_blksiz(indx) = MSCMD->ms_xfer_bcnt;
						ms->ms_index = (indx+1) % NUMOUTQ;
					}
				} else { /* its not a read or write */
					ms->ms_devinfo.ios.io_misc++;
				}
#endif	systemv_statistics
				if (((cmd->em_msg_hdr.crq_msg_code == CRQOP_MS_READ) ||
				(cmd->em_msg_hdr.crq_msg_code == CRQOP_MS_WRITE)) &&
				ms->ms_dkn >= 0) {
					dk_busy &= ~(1 << ms->ms_dkn);
					dk_wds[ms->ms_dkn] += MSCMD->ms_xfer_bcnt >> 6;
				}

				/*
				 * Check for errors:
				 *  certain tape parameters updated in ms_error
				 */
				if (cmd->em_msg_hdr.crq_msg_status != 0) {
					if (ms_error (ms, bp, cmd, &bp->b_error)) {
					bp->b_flags |= B_ERROR;
					}
				}
# if	MMAX_DEBUG
				ASSERT(chk_bdl(cmd, 22));
# endif	MMAX_DEBUG
				ms_freecmd(cmd);	/* free the cmd block */
				iodone(bp);
			}	/* End of statistics command conditional */
		}		/* End of command-present conditional */
	}		/* End of loop */
}			/* End of msintr */

ms_open_crq(ms, initvec)
ms_struct_t *ms;
int initvec;
{
	register crq_t *crq = &ms->ms_crq;
	int i, error = 0;
	crq_msg_t *free;

	/*
	 * Try to init a crq
	 */
	init_crq(crq, CRQMODE_INTR, CRQOPT_NULL, ms->ms_multidev, NULL);
	put_free(&ms->ms_attn_msg[0], crq);
	put_free(&ms->ms_attn_msg[1], crq);
	if (initvec == TRUE) {
		/*
		 * On error, punt forever
		 */
		if ((error = alloc_vector(&crq->crq_master_vect,
				msintr, ms, INTR_DEVICE)) == ESUCCESS) {
			if ((error = create_chan(crq)) == ESUCCESS)
				goto noerrs;
			dealloc_vector(&crq->crq_master_vect);
		}
		ms->ms_state = MST_INVALID;
	}
noerrs:
	return (error);
}

ms_deverr(str,dev,physdev,sts,opt)
	char	*str;
	dev_t	dev;
	int	physdev, sts, opt;
{
#if	MMAX_DEBUG
	printf (str, dev, sts, opt);
#endif	MMAX_DEBUG
	return;
}

bbread(dev, uio)
dev_t dev;
struct uio *uio;
{

	int ret;

	BUF_LOCK(&sccbuf);
	event_clear(&sccbuf.b_iocomplete);
	ret = physio(bbstrategy, &sccbuf, dev, B_READ, minphys, uio);
	BUF_UNLOCK(&sccbuf);
	return(ret);
}

bbwrite(dev, uio)
dev_t dev;
struct uio *uio;
{

	int ret;

	BUF_LOCK(&sccbuf);
	event_clear(&sccbuf.b_iocomplete);	/* post is done in biodone */
	ret = physio(bbstrategy, &sccbuf, dev, B_WRITE, minphys, uio);
	BUF_UNLOCK(&sccbuf);
	return(ret);
}

struct buf *
bbstrategy(bp)
struct buf *bp;
{
	
	int avail, size;

	avail = get_bbram_avail(bp);
	if (avail <= 0) {
		bp->b_resid = bp->b_bcount;
		bp->b_flags |= B_ERROR;
		iodone(bp);
		return(bp);
	}

	if (bp->b_flags & B_READ)
		size = get_current_dct(bp, min(avail, bp->b_bcount));
	else
		size = set_current_dct(bp, min(avail, bp->b_bcount));
	
	iodone(bp);
	return(bp);

}


/*
 * Standard routine for loggin attention messages.  Called bot
 * from ms_isr (just below here), and from scsi_host_isr shich 
 * is a sort of fake 'emc_isr' routine for DBSYS/R5 scsi systems.
 */
void
ms_attn(attn,slot,crq)
emc_atn_msg_t *attn;
int	slot;
crq_t	*crq;
{
	printf("emc/scsi attn unit 0x%x, status 0x%x, xtnd_stat 0x%x\n",
		((crq_msg_t *)attn)->crq_msg_unitid, attn->emc_atn_status,
			attn->emc_atn_xtnd_stat);
	return;
}


/* 
 *	PANIC DUMP BLOCK DEVSW ROUTINE
 */

#define BOGUS -1
#define PHYSBLK 9
int		  panic_dump_unit = BOGUS;
crq_t		  panic_dump_crq;
bd_t		  panic_dump_bdl[3];
char		  panic_dump_laybuf[DEV_BSIZE];
layout_t	  *panic_dump_layout;
crq_ms_xfer_msg_t panic_dump_msg;

#define pcrqmsg panic_dump_msg.ms_xfer_hdr.em_msg_hdr
#define pmsg	panic_dump_msg

msdumpintr(arg)
     int arg;
{
  printf("msdumpintr: shouldn't get here on a polled dump crq\n");
}

msddump (dev, blkno, begin_phys, numblks, rwflag)
     dev_t	dev;
     u_long	blkno;
     char	*begin_phys;
     u_long	numblks;
     int	rwflag;		/* B_READ or B_WRITE */
{
	bd_t	*bdl;
	int	sts, part, physdev;
	unsigned long	io_bytes, comp_cnt, last_block, rem_block;

	/* ******  get physical device somehow	******* */
	if (msd_struct[MS_SUBUNIT(dev)].ms_state == MST_INVALID)
		return(ENODEV);
	physdev = msd_struct[MS_SUBUNIT(dev)].ms_multidev;

	/* setup the BDL, round up to 8 byte boundary */
	bdl = (bd_t *) ((((unsigned long) &(panic_dump_bdl[0])) + 7) & ~7);
	bdl++; BD_TAIL(bdl); bdl--;
	pmsg.ms_xfer_desc = bdl;
	
	/* See if we must start up a crq */
	if(panic_dump_unit != physdev) {
		/* Delete old dump channel */
		if (panic_dump_unit != BOGUS) 
			polled_delete_chan(&panic_dump_crq);

		/* INIT and CREATE a new CHANNEL */
		init_crq(&panic_dump_crq, CRQMODE_POLL, CRQOPT_NULL, 
				physdev, NULL);
		sts = polled_create_chan(&panic_dump_crq, msdumpintr, NULL);
		if(sts != ESUCCESS) {
			printf("\npolled_create_chan failed, sts = 0x%x\n", sts);
			return(sts);
		}
		panic_dump_unit		= physdev;

		/* Set Up to read in on disk LAYOUT structure */
		/* Should read in partial pieces and copy to in core struct */
		pcrqmsg.crq_msg_unitid  = physdev;
		panic_dump_crq.crq_unitid = physdev;
		pcrqmsg.crq_msg_crq     = &panic_dump_crq;
		pcrqmsg.crq_msg_code    = CRQOP_MS_READ;
		pmsg.ms_xfer_lbn 	= 0;
		pmsg.ms_xfer_bcnt 	= DEV_BSIZE;
		/* only reads first 1k of layout, does no cksum */
		panic_dump_layout = (layout_t *) &(panic_dump_laybuf[0]);
		BD_NONALIGN(bdl, panic_dump_layout, DEV_BSIZE);
		if (sts = ms_panic_cmd()) {
		  printf("msddump: Unable to read layout on logdev %x, physdev %x\n", dev, panic_dump_unit);
		  polled_delete_chan(&panic_dump_crq);
		  panic_dump_unit = BOGUS;
		  return(sts);
		}
	}

	/* Check block numbers to partition layout */
	part = MS_PART(dev);
	if ( panic_dump_layout->partitions[part].part_type != DUMP_TYPE )
	    return(EINVAL);
	last_block = panic_dump_layout->partitions[part].part_size;
	if ((blkno < 0) || (blkno >= last_block)) return(EINVAL); 
	/* could attempt to do partial write */
	if ((blkno + numblks) > last_block) return(EINVAL);

	pcrqmsg.crq_msg_code = (rwflag == B_READ ? CRQOP_MS_READ 
						 : CRQOP_MS_WRITE);
	pmsg.ms_xfer_lbn = blkno+panic_dump_layout->partitions[part].part_off;
	rem_block   = numblks;

#define MAX_BYTES 0x800000

	bdl++; BD_TAIL(bdl); bdl--;
	while (rem_block > 0) {
		io_bytes = rem_block << PHYSBLK;
		if (io_bytes > MAX_BYTES) io_bytes = MAX_BYTES;
		pmsg.ms_xfer_bcnt = io_bytes;
		BD_NONALIGN(bdl, begin_phys, io_bytes);
#ifndef SMALL_DUMPS
		printf(" .");
#endif
		if (sts = ms_panic_cmd()) return(sts);
		comp_cnt = panic_dump_msg.ms_xfer_hdr.em_compltn_cnt;
		if (comp_cnt != io_bytes) {
		  printf("msddump: warning io_bytes =%x, comp_cnt = %x\n",
			 io_bytes, comp_cnt);
		}
		begin_phys	 += comp_cnt;
		rem_block	 -= comp_cnt >> PHYSBLK;
		pmsg.ms_xfer_lbn += comp_cnt >> PHYSBLK;
	}

	return(ESUCCESS);
}

ms_panic_cmd()
{
	int sts;
	crq_ms_xfer_msg_t *response;

	panic_dump_msg.ms_xfer_hdr.em_status_code = 0;
	send_cmd (&panic_dump_msg, &panic_dump_crq);
	send_vector (&panic_dump_crq.crq_slave_vect);
	response = (crq_ms_xfer_msg_t *) rec_polled_rsp(&panic_dump_crq);
	if(response != &panic_dump_msg) {
		printf("msddump: response %x != panic_msg\n",response);
		return(EIO);
	}
	if ((pcrqmsg.crq_msg_status != STS_SUCCESS) &&
	    (pcrqmsg.crq_msg_status != STS_WARNING)) {
	     printf("msddump ERROR: STS %x, CODE %x, XSTAT %x, DEVCODE %x.\n",
			pcrqmsg.crq_msg_status,
			pmsg.ms_xfer_hdr.em_status_code,
			pmsg.ms_xfer_hdr.em_xtnd_status,
			pmsg.ms_xfer_hdr.em_dev_errcode);
		return(EIO);
	}
	return(ESUCCESS);
}

int
ms_physio(strat, dev, rw, minphys, uio)
int		(*strat)();
dev_t		dev;
int		rw;
unsigned	(*minphys)();
struct uio	*uio;
{
	struct buf	*bp;
	bp_env_t	bp_env;
	int		return_value;
	
	bp = ms_getblk (&bp_env, dev);
	LASSERT(BUF_LOCK_HOLDER(bp));
	ASSERT((bp->b_flags&B_BUSY) == 0);
	return_value = physio(strat, bp, dev, rw, minphys, uio);
	ms_putblk (bp, &bp_env);
	return return_value;
}

ms_valid (dev, class, msret)
	dev_t		dev;
	int		class;		/* CLASS_DISK or CLASS_TAPE */
	ms_struct_t	**msret;
{
	ms_struct_t	*ms;
	int		subunit;
	int		sts;

	sts	= ESUCCESS;
	subunit = MS_SUBUNIT(dev);

	/* if (MS_PART(dev) > MAX_V_PARTITIONS) goto baddev; */
	if (subunit >= (class==CLASS_DISK ? NMSD : NMST))
		return (ENODEV);

	ms = (class == CLASS_DISK) ? &msd_struct[subunit] : &mst_struct[subunit];

	switch (ms->ms_state) {
	case MST_QACTIVE:
		if (class == CLASS_DISK) {
#if	0
#if	MMAX_DEBUG
		printf("MSG_QACTIVE state \n");
#endif	MMAX_DEBUG
#endif	0
		}
		break;
	case MST_ACTIVE:	/* It's OPEN and able to open again */
		break;
				
	case MST_VALID:	/* Can't read/write/ioctl unless open */
	case MST_CLOSED:	/* Can't read/write/ioctl unless open */
	case MST_INVALID:	/* No such slot, or worse */
	default:	
		sts = ENODEV;
		break;
	}

	*msret = ( (sts == ENODEV) ? NULL : ms );
	return(sts);
}

struct	buf *
ms_getblk(bp_env, dev)
	bp_env_t	*bp_env;
	dev_t		dev;
{
	struct	buf	*bp;

	bp = geteblk(MAXBSIZE);
	bp_env->b_addr	    = bp->b_un.b_addr;
	bp_env->b_bcount    = bp->b_bcount;
	bp_env->b_flags	    = bp->b_flags;
	bp->b_dev	    = dev;
	bp->b_error	    = 0;
	bp->b_flags	    = 0;
	LASSERT(BUF_LOCK_HOLDER(bp));
	return(bp);
}

ms_putblk(bp,bp_env)
	struct	buf	*bp;
	bp_env_t	*bp_env;
{
	LASSERT(BUF_LOCK_HOLDER(bp));
	bp->b_flags	= bp_env->b_flags;
	bp->b_un.b_addr = bp_env->b_addr;
	bp->b_bcount    = bp_env->b_bcount;
	bp->b_flags |= (B_ERROR | B_AGE);
	brelse(bp);
	return;
}

ms_syncio (ms, mscmd, bp, opcode)
	ms_struct_t	*ms;
	crq_msg_t *mscmd;
	struct	buf	*bp;
	{
	int sts;
	mscmd->crq_msg_code = opcode;
	mscmd->crq_msg_unitid = ms->ms_crq.crq_unitid;
	mscmd->crq_msg_refnum = (long) bp;

	if (bp->b_flags & B_ASYNC) {
		/* B_ASYNC might be useful for eg rewinding tapes,
		 * but would require alternative means of ms_putblk
		 */
		bp->b_flags |= (B_ERROR | B_AGE);
	}
	ms_start (ms, bp, mscmd);
	if (bp->b_flags & B_ASYNC) {
		return(ESUCCESS);
	} else {
		iowait(bp);
		event_clear(&bp->b_iocomplete);
		return(bp->b_error);
	}
}

/*****************************************************************************
 *
 * NAME:
 *	ms_error
 *
 * DESCRIPTION:
 *
 *	'ms_error' is called from the interrupt service routine when some
 *	sort of error has happened.  'ms_error' must analyze the 'emc style'
 *	error codes, and determine an applicable 'unix style' error code. It
 *	must also log any errors which are serious enough to warrant it. Some
 *	error messages are only warnings such as corrected ECC, which should
 *	be logged, but still allow a successful return to the unix user.
 *
 * ARGUMENTS:
 *	ms		- ptr to a disk/tape structure.
 *	cmd		- ptr to an emc command block.
 *	error		- ptr to an integer error value (such as &bp->b_error)
 *
 * RETURN VALUE: 
 *	TRUE		- if error was serious enough to mark the i/o in error.
 *	FALSE		- if error was more of a warning nature, or no error.
 *
 */

/* Each error can set these bits in 'error' for final common processing at
 * the end of the routine.  Variables set by each error case include:
 *	error	= some of the MSE_xxx bits below
 *	unerror	= returned unix style error.
 *	err_lbn = extra logical or physical block in error for some cmds.
 *	str	= string to appear in message when either MSE_PRINT is enabled,
 *			(or when MMAX_DEBUG is enabled, see MSE_STR macro below).
 *
 *	MSE_ERROR determines if the caller should return error to user. In
 *	general most errors set unerror to a unix error, and turn on this bit.
 *	Some errors may set unerror without barfing the user.  This bit 
 *	corresponds to turning on bp->b_flags | B_ERROR; whereas 'unerror' is
 *	generally put in bp->b_error.
 *
 *	MSE_LOGIT logs the error via logberr.
 *
 *	MSE_PRINT attempts to print an error message so hopefully the user
 *	will notice it on system console near the tape drive.
 *
 *	MSE_TAPE is for errors which should only occure on tapes.  On tape
 *	drives they indicate eg FILE_MARKS; whereas on a disk, something
 *	must have went bonkers.
 *
 *	MSE_SHUT_UNIT and MSE_SHUT_CTRL are for errors where perhaps the
 *	disk or controller should be disabled before they do something 
 *	horrible.  Such disablement could consist of deleteing the i/o 
 *	channel, marking it as invalid for future accesses, sending msgs
 *	to the emc?, and shutting dowm the current ms->ms_state.
 */

#define MSE_ERROR	0x01
#define MSE_LOGIT	0x02
#define MSE_PRINT	0x04
#define MSE_TAPE	0x08
#define MSE_SHUT_UNIT	0x10
#define MSE_SHUT_CTRL	0x20
#define MSE_NOWAY_JOSE	0x40

#define MSE_STR(STR)	str = STR

char qmsg[] = " MASS_STORE: NULL PACKET RETURNED on dev 0x%x\n";
char devmsg[] = " MASS_STORE %s (%s): logdev 0x%x, physdev 0x%x,\n\t\topcode %d, stat 0x%x, xstat 0x%x, devstat 0x%x\n";

ms_error (ms, bp, cmd, ret_error)
	ms_struct_t *ms;
	struct	buf *bp;
	emc_msg_t   *cmd;
	int	    *ret_error;
{
	long	err_lbn;
	int	unerror = ESUCCESS;
	int	error = 0;
	char	*str	= "??";

#if	MMAX_DEBUG
	ASSERT(chk_bdl ( cmd, 4 ));
#endif	MMAX_DEBUG

	if(cmd == NULL) {
		unerror	= EFAULT;
		error	= MSE_ERROR | MSE_LOGIT  /* | MSE_SHUT_CTRL */;

	} else if (cmd->em_msg_hdr.crq_msg_status == STS_TIMEOUT) { 
		unerror = EINTR;
		error	= MSE_ERROR | MSE_LOGIT  /* | MSE_SHUT_CTRL */;

	} else if (cmd->em_msg_hdr.crq_msg_status == STS_ABORTED) { 
		unerror = EIO;
		error	= MSE_ERROR | MSE_LOGIT;

	} else if (cmd->em_msg_hdr.crq_msg_status == STS_BADARG) { 
		unerror = EINVAL;
		error	= MSE_ERROR | MSE_LOGIT;

	} else if (cmd->em_msg_hdr.crq_msg_status == STS_INVOPCODE) { 
		if ((ms->ms_class == CLASS_TAPE) &&
		    (cmd->em_msg_hdr.crq_msg_code == CRQOP_NOP)) { 
			/* Allow NOP's to tape without error */
			error	= 0;
		} else { 
			unerror = EINVAL;
			error	= MSE_ERROR | MSE_LOGIT;
		}

	} else {	/* STS_WARNING or STS_ERROR */

	    switch(cmd->em_status_code) {
		case EMCER_NO_STATUS:
			MSE_STR("No Status Code");
			if (cmd->em_msg_hdr.crq_msg_status == STS_WARNING) {
				error	= MSE_LOGIT;
			} else { /* STS_ERROR */
				unerror = EIO;
				error	= MSE_ERROR | MSE_LOGIT;
			}
			break;
		case EMCER_ILL_OPCODE:
			MSE_STR("illegal message/command opcode");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_BAD_UNITID:
			MSE_STR("bad unitid in CRQ or message");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_ILL_LBN:
			MSE_STR("illegal disk logical block address");
			unerror = EFAULT;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_FRAME_TOOLG:
			MSE_STR("Ethernet Frame in disk channel??");
			unerror	= EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_NOWAY_JOSE;
			break;
		case EMCER_CHAN_EXIST:
			MSE_STR("channel already exists");
			unerror = EEXIST;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_NO_MORE_CHAN:
			MSE_STR("no more channels available");
			unerror = ENOSPC;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_UNIT_OFFLINE:
			MSE_STR("unit offline or inoperative");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_CMD_PENDING:
			MSE_STR("command still pending on channel");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_RECOVERED_ERROR:
			MSE_STR("recovered error (type unknown)");
			/* unerror = some code */
			if (cmd->em_msg_hdr.crq_msg_code ==
			      CRQOP_MT_WRITE_FILE_MARK) { 
				/* EMC erroneously returns STS_ERROR */
				error	= MSE_TAPE;
			} else { 
				error	= MSE_LOGIT;
			}
			break;
		case EMCER_DATA_ECCC:
			MSE_STR("corrected ECC error");
			ms->ms_ecc_soft++;
			/* unerror = some code */
			error	= MSE_LOGIT;
			break;
		case EMCER_DATA_ECCU:
			MSE_STR("uncorrected ECC error");
			/* retries=1; ?? */
			ms->ms_ecc_hard++;
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_INVLD_BUFF_DESC:
			MSE_STR("invalid buffer descriptor");
			unerror = EFAULT;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_BAD_BUFF_ADDR:
			MSE_STR("accessing non-existent memory");
			unerror = EFAULT;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_HOST_MEM_PE:
			MSE_STR("main memory parity error");
			unerror = EFAULT;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_IOBUS_PE:
			MSE_STR("I/O bus parity error");
			unerror = EFAULT;
			error	= MSE_ERROR | MSE_LOGIT /* | MSE_SHUT_CTRL */;
			break;
		case EMCER_CTLR_NOT_SELECT:
			MSE_STR("controller not selected");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_CTLR_TIMEOUT:
			MSE_STR("controller timed out");
			unerror = EINTR;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_CTLR_HW_ERROR:
			MSE_STR("controller hardware error");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_ILLEGAL_REQUEST:
			MSE_STR("illegal request to controller");
			unerror = EINVAL;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_UNIT_ATTN:
			str= "EMCER_UNIT_ATTN: (media change)";
			if ((ms->ms_class == CLASS_TAPE) &&
			    (cmd->em_msg_hdr.crq_msg_code ==
			      CRQOP_MS_TEST_UNIT_READY)) { 
				/* Used by 'open' code to flush media change */
				error	= 0;
			} else { 
				unerror = EIO;
				error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT;
			}
			break;
		case EMCER_CMD_ABORTED:
			MSE_STR("command aborted by controller");
			unerror = ENXIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_VOLUME_OVFL:	
			str = "End of Medium (with data remaining in buffer)";
			goto end_medium;
		case EMCER_DRIVE_NOT_READY:
#if	MMAX_DEBUG
			MSE_STR("drive not ready");
			unerror = ENXIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT;
			break;
#else	MMAX_DEBUG
			if(ms->ms_class == CLASS_TAPE) {
				error	= MSE_ERROR;
				goto out;
			} else {
				MSE_STR("drive not ready");
				unerror = ENXIO;
				error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT;
				break;
			}
#endif	MMAX_DEBUG
		case EMCER_DATA_PROTECT:
#if	MMAX_DEBUG
			str ="Data Protected from operation";
			unerror = EROFS;
			error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT;
			break;
#else	MMAX_DEBUG
			/*
			 * No need to write an error to the console
			 *  for write protect problems.
			 */
			if(ms->ms_class == CLASS_TAPE) {
				error	= MSE_ERROR;
				goto out;
			} else {
				str ="Data Protected from operation";
				unerror = EROFS;
				error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT;
				break;
			}
#endif	MMAX_DEBUG
		case EMCER_NO_INDEX_SIGNAL:
			MSE_STR("no index signal during disk format");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_DRIVE_FAULT:
			MSE_STR("permanent drive fault detected");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_DRIVE_NOT_SELECT:
			MSE_STR("drive not selected");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT  /* | MSE_SHUT_UNIT */;
			break;
		case EMCER_MULTI_DR_SELECT:
			MSE_STR("> 1 drive selected");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT  /* | MSE_SHUT_UNIT */;
			break;
		case EMCER_FILE_MARK:
			MSE_STR("File Mark");
			error	= MSE_TAPE;
			if (ms->ms_class == CLASS_TAPE) {
				ms->ms_flags |= MSF_SAWFMK;
			}
			break;
		case EMCER_END_MEDIUM:
			str = "end of medium encountered";
end_medium:
			error = MSE_TAPE | MSE_PRINT;
			if (ms->ms_class == CLASS_TAPE) {
				if (((ms->ms_eotcnt++ > 8) || 
					!(ms->ms_flags & MSF_SAWEOT))
				  && (cmd->em_msg_hdr.crq_msg_code == 
					CRQOP_MS_WRITE)) {
					ms->ms_flags |= MSF_SAWFMK;
					unerror = ENOSPC;
					error  |= MSE_ERROR;
				}
				ms->ms_flags |= MSF_SAWFMK;
			}
			break;
		case EMCER_INCORRECT_LENGTH:
			MSE_STR("Incorrect tape block length");
			/* If this is a short RAW read, A_OK, else barf out */
			if ((bp->b_flags & B_PHYS) && (bp->b_flags & B_READ) &&
			    (cmd->em_compltn_cnt < bp->b_bcount)) {
				error	= MSE_TAPE;
				cmd->em_msg_hdr.crq_msg_status = STS_SUCCESS;
			} else {
				unerror = EIO;
				error	= MSE_ERROR | MSE_LOGIT | MSE_TAPE;
			}
			break;
		case EMCER_NO_DATA:
			str = "No data detected";
			goto end_medium;
		case EMCER_BOT_NOT_INDICATED:
			MSE_STR("Begin Of Tape (BOT) not indicated");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_TAPE | MSE_SHUT_UNIT;
			break;
		case EMCER_SCSI_SENSE_KEY:
			MSE_STR("SCSI sense key code");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_DTC801C_ERRCODE:
			MSE_STR("DTC-801c specific error code");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_NCR_CTLR_ERRCODE:
			MSE_STR("NCR (tape) controller error code");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_BLANK_CHECK:
			str = "encountered blank block reading tape";
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_PRINT | MSE_TAPE;
			break;
		case EMCER_TIMEOUT:
			MSE_STR("Timed out by EMC");
			unerror = EINTR;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
		case EMCER_SEEK_INCOMPLETE:
			MSE_STR("seek complete signal missing");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_WRITE_FAULT:
			MSE_STR("Drive failure disallows writes");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_ID_CRC_ERROR:
			MSE_STR("ID field not recovered by retry");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_ID_AM_NOT_FOUND:
			MSE_STR("Missing Sector Pulse");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_RECORD_NOT_FOUND:
			MSE_STR("could not locate desired sector");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_SEEK_ERROR:
			MSE_STR("could not seek to track with correct id");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_BAD_FORMAT:
			MSE_STR("Format failed, or disk not formatted");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_ADP48_ERRCODE:
			MSE_STR("NCR ADP-48 specific error");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_CTLR_MALFUNC:
			MSE_STR("Controller malfunctions");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_8031_MALFUNC:
			MSE_STR("8031 malfunctions");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_SCSI_CHIP_MALFUNC:
			MSE_STR("SCSI controller chip malfunctions");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT;
			break;
		case EMCER_DATA_REWRITTEN:
			MSE_STR("Correctable error; block rewritten");
			error	= MSE_LOGIT;
			break;
		case EMCER_BLOCK_REMAPPED:
			MSE_STR("Correctable error; block remapped");
			error	= MSE_LOGIT;
			break;
		default:
			MSE_STR("Unknown Error Code");
			unerror = EIO;
			error	= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
			break;
	    }	/* End of SWITCH on em_status_code */
	}	/* End of ELSE */

	switch(cmd->em_msg_hdr.crq_msg_code) {
		case CRQOP_MS_READ:
		case CRQOP_MS_WRITE:
		case CRQOP_MD_READ_LONG:
		case CRQOP_MD_WRITE_LONG:
			err_lbn = ((crq_ms_xfer_msg_t *)cmd)->ms_xfer_lbn;
			break;
		case CRQOP_MD_REPLACE_TRACK:
			err_lbn = ((crq_md_rpltrk_msg_t *)cmd)->md_rpltrk_lbn;
			break;
		case CRQOP_MD_SEEK:
			err_lbn = ((crq_md_seek_msg_t *)cmd)->md_seek_pbn;
			break;
		case CRQOP_MD_CHECK_TRACK:
			err_lbn = ((crq_md_chktrk_msg_t *)cmd)->md_chktrk_pbn;
			break;
		case CRQOP_MD_READ_SECT_ID:
			err_lbn = ((crq_md_rdsid_msg_t *)cmd)->md_rdsid_pbn;
			break;
		default:	err_lbn = 0;	break;
	}

	if ((error & MSE_TAPE) && (ms->ms_class != CLASS_TAPE)) {
		unerror	= EIO;
		error  |= MSE_ERROR | MSE_LOGIT | MSE_SHUT_UNIT;
	}


#if	0
#if	MMAX_DEBUG
	if (error & (MSE_ERROR | MSE_LOGIT | MSE_PRINT)) {
#else
#if	defined(MSDEBUG)
	if (error & (MSE_LOGIT | MSE_PRINT)) {
#else
	if (error & (MSE_PRINT))  {
#endif  MSDEBUG
#endif	MMAX_DEBUG
#endif	0
	if (error & (MSE_ERROR | MSE_LOGIT | MSE_PRINT)) {
		if (cmd == NULL) {
			printf(qmsg, ms->ms_dev);
		} else {
			char *sts_str;
			switch(cmd->em_msg_hdr.crq_msg_status) {
				case STS_SUCCESS:
					sts_str = "SUCCESS";
					break;
				case STS_WARNING:
					sts_str = "WARNING";
					break;
				case STS_PENDING:
					sts_str = "PENDING";
					break;
				case STS_QUEUED:
					sts_str = "QUEUED";
					break;
				case STS_FREE:
					sts_str = "FREE";
					break;
				case STS_ABORTED:
					sts_str = "ABORTED";
					break;
				case STS_BADARG:
					sts_str = "BADARG";
					break;
				case STS_INVOPCODE:
					sts_str = "INVALID OPCODE";
					break;
				case STS_ERROR:
					sts_str = "ERROR";
					break;
				case STS_TIMEOUT:
					sts_str = "TIMEOUT";
					break;
				default:	   
					sts_str = "UNKNOWN STATUS";
					break;
			}
			printf(devmsg, sts_str, str,
				ms->ms_dev, ms->ms_multidev, 
				cmd->em_msg_hdr.crq_msg_code, 
				cmd->em_status_code,
				cmd->em_xtnd_status,
				cmd->em_dev_errcode);
		}
	}

	if (error & (MSE_ERROR | MSE_LOGIT))
		ms->ms_errors++;

	/* Be easy on guy for the first open of a possibly  unformatted disk */
	if ((ms->ms_state == MST_QACTIVE) && (ms->ms_class == CLASS_DISK))
		error &= ~(MSE_SHUT_UNIT | MSE_SHUT_CTRL);

	if (error & (MSE_SHUT_UNIT | MSE_SHUT_CTRL)) {
		printf("Serious ms_error on dev 0x%x: should shutdown the %s\n",
		   ms->ms_dev, (error & MSE_SHUT_UNIT) ? "unit" : "controller");
		/* ms->ms_state = MST_CLOSE or MST_INVALID; */
	}
	if (ms->ms_class == CLASS_DISK && ms->ms_dkn >= 0) {
		dk_busy &= ~(1 << ms->ms_dkn);
	}
out:
	*ret_error = unerror;
	return( error & MSE_ERROR);
}

/*****************************************************************************
 *
 * NAME:
 *	ms_start
 *
 * DESCRIPTION:
 *	Called for DISK or TAPE queue requests to EMC.  Important commands
 *	are generally non massstore specific (< CRQOP_MS_BASE) and are put on
 *	the immediate queue.  Disk transfers and remaining tape commands go on
 *	the command queues.  Funny disk commands (which??) are queued on the
 *	immediate queue and are immune from the sorting algorithm.
 *
 *	DISK data transfers are sorted into the command queue by block number.
 *	When it is necessary to actually sort, the existing chain is removed
 *	from the crq and sorted without the spin lock held.  In this case,
 *	a softlock is taken for duration of sort to keep other processes out.
 *	(only necessary for multiprocessor systems)
 *	
 *
 * ARGUMENTS:
 *	ms	- pointer to the masstore structure
 *	bp	- pointer to a buffer pointer which points to command.
 *
 */

ms_start (ms, bp, cmd)
	ms_struct_t	*ms;

	struct	buf	*bp;
	crq_msg_t	*cmd;
{
	crq_t		*crq;
	crq_msg_t	*link_cmd;
	int	 	interrupt;
	int		s;

#if	MMAX_DEBUG
	ASSERT(chk_bdl ( cmd, 3 ));
#endif	MMAX_DEBUG

	crq = &ms->ms_crq;


	if ((ms->ms_class == CLASS_DISK) &&
		((cmd->crq_msg_code == CRQOP_MS_READ) ||
		 (cmd->crq_msg_code == CRQOP_MS_WRITE) ||
		 (cmd->crq_msg_code == CRQOP_MD_READ_LONG) ||
		 (cmd->crq_msg_code == CRQOP_MD_WRITE_LONG) )) {

/*
 *	Data transfer commands need to take the mutex lock to block other
 *	processes since ms_disksort may release the crq spin-lock while
 *	sorting	a chain of requests.  Take both mutex and spin locks, then do
 *	the sort.
 */
		interrupt = 0;
		s = spl7();
		crq_lock(&crq->crq_slock);
		interrupt = ms_disksort((crq_ms_xfer_msg_t *) cmd, crq, s);
				/* sort the request queue */

		/*
		 * We have the world put back together ie - the request queue is
		 * sorted and ready to go.  Set the message status and update
		 * statistics before releasing the crq locks.
		 */

		cmd->crq_msg_crq = crq;
		cmd->crq_msg_status = STS_QUEUED;
		crq->crq_totcmds++;
		if (interrupt) interrupt = (crq->crq_mode == CRQMODE_INTR);
		if (interrupt) {
			crq->crq_totvects++;
		}
		crq_unlock(&crq->crq_slock);
		splx(s);

		if(interrupt) {
			send_vector(&crq->crq_slave_vect);
		}
        /* Statistics update */
		if (ms->ms_dkn >= 0) {
			dk_busy |= 1 << ms->ms_dkn;
			dk_xfer[ms->ms_dkn]++;
		}
        return;
	}
	else {
		/* IF its a funny DISK request, 
		 * OR something non masstore specific (abort, warm restart)
		 * THEN put it on the immediate queue
		 */
		if((cmd->crq_msg_code < CRQOP_MS_BASE) || 
		   (ms->ms_class == CLASS_DISK)) {
			send_immedcmd(cmd, crq);
		
		} else { /* Must be a TAPE command */	
			send_cmd(cmd, crq);
		}
	}

	return;
}

/*
**  The old algorithm set ms_need_wakeup=1 when ms_getcmd() couldn't
**  find a free cmd block.  ms_freecmd checked ms_need_wakeup and posted
**  a wakeup.  There could be a race between ms_freecmd setting that
**  variable to 0 and ms_getcmd setting the variable to 1 in a low-memory
**  (and mp) environment.  The new way is a bit more reliable:  always
**  post a wakeup when we're adding to an empty queue.  We check for the
**  queue being empty under lock and update the queue under lock to avoid
**  the following possibility.
**
**  Say we looked at the queue, then took the lock and did the update.
**  We could see a queue with one element on it while someone else has
**  the lock and is removing that element.  We then lose the race on
**  the queue lock to a third party seeking to acquire a cmd block.
**  Having acquired the lock, the third party sees that the queue is
**  empty and so it releases the lock and sleeps.  Now we get the
**  queue lock, add the newly freed element to the queue, release the
**  lock and exit without posting the wakeup because we originally
**  saw a non-empty queue.  The sleeping party will only be awakened
**  if the queue empties again and the race works out so a wakeup is posted.
*/

ms_freecmd(cmd)
crq_ms_xfer_msg_t *cmd;
{
	if (cmd->ms_xfer_hdr.em_msg_hdr.crq_msg_hdr.hdr_bdl_alloc) {
		dealloc_bdl(cmd->ms_xfer_desc);
		cmd->ms_xfer_hdr.em_msg_hdr.crq_msg_hdr.hdr_bdl_alloc = 0;
	}
	mpenqueue1 (&ms_cmd_free_que, cmd);
}

#define	ms_xfer_hdr_bdl_alloc ms_xfer_hdr.em_msg_hdr.crq_msg_hdr.hdr_bdl_alloc

crq_ms_xfer_msg_t *
ms_getcmd(bp, result, need_bdl)
struct	buf	*bp;		/* Buffer pointer                         */
int		*result;	/* Pointer to Success/Failure indication  */
boolean_t	need_bdl;       /* true if bdl ptr needed in cmd header   */
{
	crq_ms_xfer_msg_t	*cmd;

	LASSERT(BUF_IS_LOCKED(bp));
	ASSERT(bp->b_bcount > 0);
	mpdequeue1 (&ms_cmd_free_que, &cmd, QWAIT);
	cmd->ms_xfer_hdr_bdl_alloc = 0;
	if (need_bdl) {
		*result = create_bdl(bp, &cmd->ms_xfer_desc);
		if (*result == ESUCCESS) {
			cmd->ms_xfer_hdr_bdl_alloc = 1;
		}
		else {
			mpenqueue1 (&ms_cmd_free_que, cmd);
			cmd = (crq_ms_xfer_msg_t *) 0;
		}
	}
	else
		*result = ESUCCESS;
	return (cmd);
}


void
ms_cmdinit()
{
	crq_ms_xfer_msg_t	*cmd;
	int			s;

	s = splhigh();
	mpqueue_init(&ms_cmd_free_que);

	for (cmd = buf_cmds; cmd < &buf_cmds[NMS_CMD]; cmd++) {
		mpenqueue1 (&ms_cmd_free_que, cmd);
	}
	splx(s);
}

#define	NUM_BDL_BUF	20	/* This allows up to 19 noncontiguous areas */
#define	TOTAL_BDL_BUF	NMS_CMD*NUM_BDL_BUF
#define	BDL_SLOP	6

mpqueue_head_t		bdl_free;
bd_t			bdl_buf[TOTAL_BDL_BUF+BDL_SLOP];
bd_t			*bdl_free_base;
#if	MMAX_DEBUG
bd_t			*bdl_free_end;
int         bdl_count = 0;
simple_lock_data_t  bdl_count_lock;
#endif	MMAX_DEBUG

/*
 *	init_bdl - Initialize the bdl allocator
 */

lock_data_t	init_bdl_lock;

init_bdl()
{
	static int	bdl_inited = 0;
	int		i;
	bd_t		*bdlp;

	simple_lock(&init_bdl_lock);
	if (bdl_inited) {
		simple_unlock(&init_bdl_lock);
		return;
	}
	bdl_inited++;

	mpqueue_init(&bdl_free);

	/* set bdlp to address in bdl_buf with 8 byte alignment */
	bdl_free_base = (bd_t *)((long)&bdl_buf[1] & ~(sizeof(bd_t) - 1));
	bdlp = bdl_free_base;

	for (i=0; i < NMS_CMD; i++, bdlp += NUM_BDL_BUF) {
		mpenqueue1 (&bdl_free, bdlp);
	}
#if	MMAX_DEBUG
	bdl_free_end = bdlp;
    simple_lock_init(&bdl_count_lock);
#endif	MMAX_DEBUG
	simple_unlock(&init_bdl_lock);
}

/*
 *	alloc_bdl - Allocate and format a buffer descriptor list with 4 bdl's.
 */

bd_t *
alloc_bdl()
{
	bd_t	*bdl_entry;
#if	MMAX_DEBUG
	int	s;
#endif	MMAX_DEBUG

	mpdequeue1 (&bdl_free, &bdl_entry, QNOWAIT);
	if (bdl_entry == (bd_t *) 0) {
#if MMAX_DEBUG
        printf("alloc_bdl: Free bdls exhausted, count=%d\n",bdl_count);
        panic("alloc_bdl:  Free bdls exhausted");
#else   MMAX_DEBUG
        printf("alloc_bdl: Free bdls exhausted\n");
#endif  MMAX_DEBUG
		/*
		**  This is not a real, CMU-style fs resource pause.
		**  It is not a good idea at this point to do such a
		**  pause, potentially allowing the user to suspend
		**  this thread.  Who knows what locks we hold at
		**  this point and what other threads may (now or soon)
		**  need them; this is a crisis situation.  We really
		**  shouldn't even go to sleep at this point, just
		**  spin on the queue for a while and/or return an error.
		*/
		mpdequeue1 (&bdl_free, &bdl_entry, QWAIT);
		printf ("alloc_bdl:  continuing\n");
	}
#if MMAX_DEBUG
    s = splhigh();
    simple_lock(&bdl_count_lock);
    ++bdl_count;
    simple_unlock(&bdl_count_lock);
    splx(s);
#endif  MMAX_DEBUG
	return (bdl_entry);
}


/*
 *	dealloc_bdl - Deallocate a buffer descriptor list with 4 bdl's.
 */

dealloc_bdl(bdl)
bd_t	*bdl;
{
#if MMAX_DEBUG
    int s;
#endif  MMAX_DEBUG

#if MMAX_DEBUG
    s = splhigh();
    simple_lock(&bdl_count_lock);
    --bdl_count;
    simple_unlock(&bdl_count_lock);
    splx(s);
#endif  MMAX_DEBUG
	mpenqueue1(&bdl_free, bdl);
}


#if	MMAX_DEBUG
/*
 *      check_bdl - Test emc message for valid bdl pointer
 */

chk_bdl( cmd, id )
crq_ms_xfer_msg_t	*cmd;
int			id;
{
	int i;
	bd_t *bdl;
	
	if ( ! cmd->ms_xfer_hdr_bdl_alloc ) {
		return 1;
	}

	bdl = cmd->ms_xfer_desc;

	if ( bdl == 0 ) {
		printf ("chk_bdl: No bdl pointer in cmd at 0x%x, id = %d\n",
			cmd, id);
		panic ("chk_bdl: No bdl pointer in cmd\n");
	}

	if (bdl < bdl_free_base || bdl >= bdl_free_end)	{
		printf ("chk_bdl: Bad bdl pointer (bdl=0x%x, i=%d) in cmd at 0x%x, id = %d\n",
			bdl, i, cmd, id);
		panic ("chk_bdl: Bad bdl pointer in cmd\n");
	}
	
	return 1;
}
#endif	MMAX_DEBUG


/*
 *	create_bdl - Create a buffer descriptor list that describes the
 *		location of all the physical pages of the buffer.
 *
 *	bp	- pointer to buffer
 *	ret_bdl	- Pointer to returned buffer descriptor list pointer (or NULL)
 */

create_bdl(bp, ret_bdl)
struct	buf	*bp;
bd_t	**ret_bdl;
{
	caddr_t	addr = bp->b_un.b_addr;
	long	bytes = bp->b_bcount;
	bd_t	*bdl;		/* pointer to buffer descript. list */
	int	pb;		/* temp byte count */
	caddr_t	pa;		/* temp physical address */
	int	sts = ESUCCESS;
	struct  proc *mproc;	/* tell vatopa where to look */
	int     i;              /* counter                          */

	LASSERT(BUF_IS_LOCKED(bp));
	ASSERT(bytes > 0);
	if (bp->b_flags&B_PHYS) {
		mproc = bp->b_proc;
		ASSERT(mproc != (struct proc *) 0);  /* this process */
	}
	else
		mproc = (struct proc *) 0;  /* system space */

	if (bytes > 0) {
		if ((*ret_bdl = alloc_bdl()) == 0)
			panic ("create_bdl:  alloc_bdl failed");
		bdl = *ret_bdl;
	}

        for ( i=1;  bytes != 0 && i < NUM_BDL_BUF; i++ ) {
	  if (bytes > 0) {
	    if((sts = vatopa(addr, bytes, &pa, &pb, mproc)) != ESUCCESS)
	    { printf ("create_bdl: vatopa call failed\n");
	      printf ("  addr: 0x%x, bytes: %d, pa: 0x%x, pb: %d, proc: %d\n",
		      addr, bytes, pa, pb, mproc);
	      goto error;
	    }
	    bytes -= pb;
	    addr += pb;
	    BD_NONALIGN(bdl, pa, pb);
	    BD_NEXT(bdl);
	  }
	}

	if(bytes > 0)	/* error cases */
	  {     printf("create_bdl: Bytes remaining after bdl's allocated\n");
		sts = EIO;
	  }
	else
		BD_TAIL(bdl);
/*
 * If we got an error, release the BDLs
 * we allocated and set the returned BDL pointer to NULL.
 */
 error:
	if (sts != ESUCCESS) {
		dealloc_bdl(*ret_bdl);
		*ret_bdl = NULL;
		printf("create_bdl failure\n");
/*		panic("create_bdl failure\n"); */
	}
	return (sts);
}
/*	---------------  end of ms_comm.c	---------------	*/
