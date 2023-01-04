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
static char	*sccsid = "@(#)$RCSfile: cmu_syscalls.c,v $ $Revision: 4.4.18.12 $ (DEC) $Date: 1993/12/07 22:18:46 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/kernel.h>
#include <sys/table.h>
#include <sys/dk.h>
#include <sys/syscall.h>

#include <vm/vm_user.h>
#include <vm/vm_map.h>
#include <vm/vm_swap.h>
#include <vm/vm_kern.h>
#include <mach/vm_param.h>
#include <kern/task.h>
#include <sys/vmmac.h>		/* only way to find size of u./kernel stack */
#include <machine/vmparam.h>	/* only way to find user stack (argblock) */

#include <sys/version.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <sys/malloc.h>

/*
 * Include and externs for SAR.
 */
#include <mach/vm_statistics.h>

long svr4_fte_used();
long svr4_shmem_cnt();
extern long svr4_freeswap_space();

/*
 * Static  SAR counters. These are here because they are most closely
 * associate with the table() functionality and just clutter up
 * init_main.c.
 */

/* for sar -a support */
long tf_iget;
long tf_namei;
long tf_dirblk;

/* for sar -b support */
long ts_bread;
long ts_bwrite;
long ts_lread;
long ts_lwrite;
long ts_phread;
long ts_phwrite;

/* for sar -c support */
long ts_sysread;
long ts_syswrite;
long ts_sysexec;
long ts_readch;
long ts_writech;

/* for sar -q support */
long sar_kmem_fail;

/* for sar -q support */
long sar_runocc;
long sar_runqueue;

/* for sar -v support */
long tbl_proc_ov;
long tbl_inod_ov;
long tbl_file_ov;

long pg_v_pgpgin;
long pg_v_sftlock;
long pg_v_pgout;
long pg_v_dfree;
long pg_v_scan;
long pg_v_s5ifp;


/*
 *  table - get/set element(s) from system table
 *
 *  This call is intended as a general purpose mechanism for retrieving or
 *  updating individual or sequential elements of various system tables and
 *  data structures.
 *
 *  One potential future use might be to make most of the standard system
 *  tables available via this mechanism so as to permit non-privileged programs
 *  to access these common SYSTAT types of data.
 *
 *  Parameters:
 *
 *  id		= an identifer indicating the table in question
 *  index	= an index into this table specifying the starting
 *		  position at which to begin the data copy
 *  addr	= address in user space to receive/supply the data
 *  nel		= number of table elements to retrieve/update
 *  lel		= expected size of a single element of the table.  If this
 *		  is smaller than the actual size, extra data will be
 *		  truncated from the end.  If it is larger, holes will be
 *		  left between elements copied to/from the user address space.
 *
 *		  The intent of separately distinguishing these final two
 *		  arguments is to insulate user programs as much as possible
 *		  from the common change in the size of system data structures
 *		  when a new field is added.  This works so long as new fields
 *		  are added only to the end, none are removed, and all fields
 *		  remain a fixed size.
 *
 *  Returns:
 *
 *  val1	= number of elements retrieved/updated (this may be fewer than
 *		  requested if more elements are requested than exist in
 *		  the table from the specified index).
 *
 *  Note:
 *
 *  A call with lel == 0 and nel == MAXSHORT can be used to determine the
 *  length of a table (in elements) before actually requesting any of the
 *  data.
 */

#define MAXLEL	(sizeof(long))	/* maximum element length (for set) */

table(cp, args, retval)
	struct proc *cp;
	void *args;
	long *retval;
{
	register struct args {
		long id;
		long index;
		caddr_t addr;
		long nel;	/* >0 ==> get, <0 ==> set */
		u_long lel;
	} *uap = (struct args *) args;
	struct proc *p;
	unsigned size;
	int error = 0;
	int set;
	long indx = uap->index;
	long nel = uap->nel;
	vm_offset_t	arg_addr;
	vm_size_t	arg_size;
	vm_offset_t	copy_start;
	vm_offset_t	copy_end;
	vm_size_t	copy_size;
	vm_map_copy_t	copy_result;
	vm_map_t	proc_map;
	vm_offset_t	dealloc_start;	/* area to remove from kernel map */
	vm_size_t	dealloc_end;
	PROC_INTR_VAR(s);

	/*
	 *  Verify that any set request is appropriate.
	 */
	set = 0;
	if (nel < 0) {
		switch (uap->id) {
#if	!SEC_BASE
		case TBL_AID:
#endif
		case TBL_MAXUPRC:
#if	SEC_BASE
			if (!privileged(SEC_LIMIT, EPERM))
				return (EPERM);
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				return (error);
#endif
			/* fall through */
#if	!SEC_BASE
		case TBL_MODES:
#endif
		        break;
		default:
			return (EINVAL);
		}
		set++;
		nel = -(nel);
	}

	*retval = 0;

	/*
	 *  Verify common case of only current process index.
	 */
	switch (uap->id) {
#if	!SEC_BASE
	case TBL_AID:
	case TBL_MODES:
#endif
	case TBL_U_TTYD:
	case TBL_MAXUPRC:
		if ((indx != cp->p_pid && indx != 0) ||
				(nel != 1))
			goto bad;
		break;
	}
	/*
	 *  Main loop for each element.
	 */

	while (nel > 0) {
		union {
			dev_t			_ttyd;
			int			_iv;
			struct tbl_loadavg	_tl;
			struct tbl_procinfo	_tp;
			struct tbl_sysinfo	_ts;
			struct tbl_intr		_ti;
			struct tbl_dkinfo	_td;
			struct tbl_ttyinfo	_tt;
		        struct tbl_file		_fi;
			struct tbl_buffer	_bu;
			struct tbl_scalls	_sc;
			struct tbl_runq		_rq;
			struct tbl_tblstats	_tb;
			struct tbl_kmem		_km;
			struct tbl_paging	_pg;
			struct tbl_mapinfo	_tm;
		} table_entry;
#define ttyd table_entry._ttyd
#define iv table_entry._iv
#define tl table_entry._tl
#define tp table_entry._tp
#define ts table_entry._ts
#define ti table_entry._ti
#define td table_entry._td
#define tt table_entry._tt
#define bu table_entry._bu
#define fi table_entry._fi
#define sc table_entry._sc
#define rq table_entry._rq
#define tb table_entry._tb
#define km table_entry._km
#define pg table_entry._pg
#define tm table_entry._tm
		caddr_t data = (caddr_t)&table_entry;
		struct vnode *vnp;

		dealloc_start = (vm_offset_t) 0;
		switch (uap->id) {
		case TBL_U_TTYD:
			PROC_LOCK(cp);
			SESS_LOCK(cp->p_session);
			if((cp->p_flag & SCTTY) &&
			   (vnp = cp->p_session->s_ttyvp) != NULL &&
			   (vnp->v_flag & VXLOCK) == 0 &&
			   vnp->v_type != VBAD)
				ttyd = vnp->v_rdev;
			else {
				ttyd = NODEV;
			}
			SESS_UNLOCK(cp->p_session);
			PROC_UNLOCK(cp);
			size = sizeof (dev_t);
			break;
		case TBL_LOADAVG:
			if (indx != 0 || nel != 1)
				goto bad;
			bcopy((caddr_t)&avenrun[0], (caddr_t)&tl.tl_avenrun,
					sizeof(tl.tl_avenrun));
			tl.tl_lscale = LSCALE;
			bcopy((caddr_t)mach_factor, (caddr_t)tl.tl_mach_factor,
					sizeof(tl.tl_mach_factor));
			size = sizeof (tl);
			break;
		case TBL_INTR:
		    {
			int	i;
			extern int	scmax;
			extern int	sccount[];
			extern long	swtch_tsk_ctxt_cnt;
			extern long	swtch_thrd_ctxt_cnt;
			extern long	handler_stats();

			if (indx != 0 || nel != 1)
				goto bad;
			
			bzero((caddr_t)&ti, sizeof(ti));

			ti.in_devintr = handler_stats(INTR_NOTCLOCK);

                        ti.in_context = swtch_tsk_ctxt_cnt +swtch_thrd_ctxt_cnt;

			ti.in_syscalls = 0;
			for(i=0; i < scmax; i++)
				ti.in_syscalls += sccount[i];
                        ti.in_forks = sccount[SYS_fork];
                        ti.in_vforks = sccount[SYS_vfork];

                        size = sizeof(ti);
			break;
		    }
		case TBL_INCLUDE_VERSION:
			if (indx != 0 || nel != 1)
				goto bad;
			iv = INCLUDE_VERSION;
			size = sizeof(iv);
			break;
		case TBL_MAXUPRC:
			data = (caddr_t)&u.u_maxuprc;
		        size = sizeof(u.u_maxuprc);
			break;
		case TBL_UAREA:
		   {
			struct user 	*fake;
			task_t		task;
			thread_t	thread;

			/*
			 *	Lookup process by pid
			 */
			if (error = pid_to_proc(indx, &p))
				return error;
			/*
			 *	Before we can block (any VM code), make
			 *	another reference to the task to keep it
			 *	alive.
			 */

			task = p->task;

			task_reference(task);

			fake = (struct user *)
				kmem_alloc_pageable(kernel_copy_map,
					round_page(sizeof(struct user)));

			if (fake == (struct user *) 0) {
				task_deallocate(task);
				return (ENOMEM);
			}

			task_lock(task);
			if (task->thread_count > 0) {
				thread = (thread_t) task->thread_list.next;
				thread_reference(thread);
				task_unlock(task);

				fake_u(fake, thread);
				task_deallocate(task);
				thread_deallocate(thread);

				data = (caddr_t) fake;
				size = (vm_size_t) sizeof(struct user);
				dealloc_start = (vm_offset_t) fake;
				dealloc_end = dealloc_start +
						round_page(sizeof(struct user));
			} else {
				task_unlock(task);
				task_deallocate(task);
				return (ESRCH);
			}
			break;
		   }

		case TBL_ARGUMENTS:
                   {
			register struct utask	*utaskp;
			/*
			 *	Returns the first N bytes of the user args,
			 *	Odd data structure is for compatibility.
			 */
			/*
			 *	Lookup process by pid
			 */
			if (error = pid_to_proc(indx, &p))
				return error;
                        /*
                         *      Get task struct
                         */
                        utaskp = p->task->u_address;

			/*
			 *	Get map for process
			 */
			proc_map = p->task->map;

			/*
			 *	If the user expects no more than N bytes of
			 *	argument list, use that as a guess for the
			 *	size.
			 */
			if ((arg_size = uap->lel) == 0) {
				error = EINVAL;
				goto bad;
			}
			arg_addr = (vm_offset_t)utaskp->uu_argp;
                        copy_size = MIN(utaskp->uu_arg_size, arg_size);

                        /*
                         * If there are no arguments then return empty
                         * buffer
                         */
                        if (copy_size == 0 || utaskp->uu_argp == NULL) {
                                return(0);
                        }
                        
			/*
			 *	Make a reference to the map to keep it alive,
			 *	then make the copy.
			 */
			vm_map_reference(proc_map);
			if (vm_map_copyin(proc_map, arg_addr, copy_size, FALSE, &copy_result)
			    != KERN_SUCCESS) {
			    	vm_map_deallocate(proc_map);
				goto bad;
			}
			vm_map_deallocate(proc_map);

			/*
			 *	Drop the copy here.
			 */

			if (vm_map_copyout(kernel_copy_map, &copy_start, copy_result)
			    != KERN_SUCCESS) {
				vm_map_copy_discard(copy_result);
				goto bad;
			}

                        /*
                         * Return arguments
                         */
                        data = (caddr_t) copy_start;
                        size = (vm_size_t) copy_size;
                        
			copy_end = copy_start + copy_size;

			dealloc_start = copy_start;
			dealloc_end = copy_end;
                        break;
                    }
		case TBL_ENVIRONMENT:
                   {
			register struct utask	*utaskp;
			/*
			 *	Returns the first N bytes of the environment,
			 *	Odd data structure is for compatibility.
			 */
			/*
			 *	Lookup process by pid
			 */
			if (error = pid_to_proc(indx, &p))
				return error;
                        /*
                         *      Get task struct
                         */
                        utaskp = p->task->u_address;

			/*
			 *	Get map for process
			 */
			proc_map = p->task->map;

			/*
			 *	If the user expects no more than N bytes of
			 *	argument list, use that as a guess for the
			 *	size.
			 */
			if ((arg_size = uap->lel) == 0) {
				error = EINVAL;
				goto bad;
			}
			arg_addr = (vm_offset_t)utaskp->uu_envp;
                        copy_size = MIN(utaskp->uu_env_size, arg_size);

                        /*
                         * If there is no environment then return empty
                         * buffer
                         */
                        if (copy_size == 0 || utaskp->uu_envp == NULL ) {
                                return (0);
                        }
                        
			/*
			 *	Make a reference to the map to keep it alive,
			 *	then make the copy.
			 */
			vm_map_reference(proc_map);
			if (vm_map_copyin(proc_map, arg_addr, copy_size, FALSE, &copy_result)
			    != KERN_SUCCESS) {
			    	vm_map_deallocate(proc_map);
				goto bad;
			}
			vm_map_deallocate(proc_map);

			/*
			 *	Drop the copy here.
			 */

			if (vm_map_copyout(kernel_copy_map, &copy_start, copy_result)
			    != KERN_SUCCESS) {
				vm_map_copy_discard(copy_result);
				goto bad;
			}

                        /*
                         * Return arguments
                         */
                        data = (caddr_t) copy_start;
                        size = (vm_size_t) copy_size;
                        
			copy_end = copy_start + copy_size;

			dealloc_start = copy_start;
			dealloc_end = copy_end;
                        break;
                    }
		case TBL_PROCINFO:
		    {
			register struct utask	*utaskp;
			struct session 		*sess;
                        int                     referenced=0, locked=0;
			char    		stat = 0;

			/*
			 *	Index is entry number in proc table.
			 */
			if (indx >= nproc || indx < 0)
			    goto bad;

			p = &proc[indx];
			PROC_LOCK(p);
			stat = p->p_stat;
			if (!PROC_ACTIVE(p) || p->p_flag == 0) {
				/*
				 * Keep proc locked!
				 */
				locked = 1;
			} else {
				PROC_UNLOCK(p);
				P_REF(p);
				referenced = 1;
			}


#if	SEC_BASE
			/*
			 * To retrieve information about another process
			 * requires privilege.
			 */
			if (p != u.u_procp && !privileged(SEC_DEBUG, EPERM))
				return (EPERM);
#else
			if (p != u.u_procp &&
			    (error = suser(u.u_cred, &u.u_acflag)))
				return (error);
#endif
			bzero((caddr_t)&tp, sizeof(tp));
			if (p->p_stat == NULL || p->p_stat == SIDL) {
			    tp.pi_status = PI_EMPTY;
			}
			else {
                           if (!locked)
                                 PROC_INTR_LOCK(p, s);
                            tp.pi_flag  = p->p_flag;
			    tp.pi_uid	= p->p_rcred->cr_uid;
                            tp.pi_ruid  = p->p_ruid;
                            tp.pi_svuid = p->p_svuid;
                            tp.pi_rgid  = p->p_rgid;
                            tp.pi_svgid = p->p_svgid;
			    tp.pi_pid	= p->p_pid;
			    tp.pi_ppid	= p->p_ppid;
                            tp.pi_session   = 0;
                            tp.pi_ttyd      = NODEV;
                            tp.pi_pgrp      = 0;
                            tp.pi_tpgrp     = 0;
                            tp.pi_jobc      = 0;
                            if (p->p_pgrp) {
                                    tp.pi_pgrp  = p->p_pgrp->pg_id;
                                    tp.pi_jobc  = p->p_pgrp->pg_jobc;
				    if (sess = p->p_session) {
					SESS_LOCK(sess);
					/*
					 * save it now, since we
					 * already have the pgrp.
					 */
					tp.pi_session = sess->s_id;
					if (sess->s_fpgrpp) {
					  struct pgrp *pgrp;
					  pgrp = *sess->s_fpgrpp;
					  if (pgrp)
					    	tp.pi_tpgrp = pgrp->pg_id;
				        }
				        vnp = sess->s_ttyvp;
				        if (vnp != NULL &&
                  (vnp->v_flag & VXLOCK) == 0 &&
                  vnp->v_type != VBAD) {
						VREF(vnp);
						tp.pi_ttyd = vnp->v_rdev;
						SESS_UNLOCK(sess);
						VUNREF(vnp);
				        } else {
						SESS_UNLOCK(sess);
				        }
				   }
                            }
                            PROC_SDATAUP_LOCK(p);
                            tp.pi_sigignore = p->p_sigignore;
                            tp.pi_sigcatch = p->p_sigcatch;
                            PROC_SDATAUP_UNLOCK(p);
                            tp.pi_sig   = p->p_sig;
                            tp.pi_sigmask = p->p_sigmask;
			    tp.pi_cursig = 0;
			    if (!(p->p_flag & SCTTY)) {
				    if (!locked)
					PROC_INTR_UNLOCK(p,s);
				    tp.pi_ttyd = NODEV;
                                    tp.pi_tsession = 0;
                                    tp.pi_tpgrp = 0;
                            }
			    else  {
				    if (!locked)
					PROC_INTR_UNLOCK(p,s);
                                    tp.pi_tsession = tp.pi_session;
			    }
			    if (p->p_stat == SZOMB) {
				tp.pi_status = PI_ZOMBIE;
			    }
			    else {
				utaskp = p->task->u_address;
				bcopy(utaskp->uu_comm, tp.pi_comm,
				      MAXCOMLEN);
				tp.pi_comm[MAXCOMLEN] = '\0';

				if (p->p_flag & SWEXIT)
				    tp.pi_status = PI_EXITING;
				else
				    tp.pi_status = PI_ACTIVE;
			    }
			}
                        if (locked)
                                PROC_UNLOCK(p);
                        if (referenced)
                                P_UNREF(p);

			size = sizeof(tp);
			break;
		    }
                case TBL_SYSINFO:
			if (indx != 0 || nel != 1)
				goto bad;
                        ts.si_user = cp_time[CP_USER];
                        ts.si_nice = cp_time[CP_NICE];
                        ts.si_sys = cp_time[CP_SYS];
                        ts.si_idle = cp_time[CP_IDLE];
        		ts.wait = cp_time[CP_WAIT];
                        ts.si_hz = hz;
                        ts.si_phz = phz;
			ts.si_boottime = boottime.tv_sec;
                        size = sizeof(ts);
                        break;
                case TBL_DKINFO:
			if (indx >= DK_NDRIVE || nel != 1)
				goto bad;
                        td.di_ndrive = dk_ndrive;
                        td.di_busy = dk_busy;
                        td.di_time = dk_time[indx];
                        td.di_seek = dk_seek[indx];
                        td.di_xfer = dk_xfer[indx];
                        td.di_wds = dk_wds[indx];
                        td.di_wpms = dk_wpms[indx];
			/*
			 * would require driver change, so they're unsupported 
			 */
        		td.di_avque = 0;
        		td.di_avwait = 0;
        		td.di_avserv = 0;
#if defined(mips) || defined (__alpha)
#define __DKNAME
                        {
                                extern char *szinfo();
                                extern char *mscp_info();

                                /*
                                 * We call both SCSI and MSCP stats to see who
                                 * owns this unit.  The way we obtain stats
                                 * needs to be more dynamic (obviously) in the
                                 * future (this is a hack; but a well meaning
                                 * hack!).
                                 */
                                data = szinfo(indx, &td.di_unit);
                                if (data)
                                        bcopy(data, td.di_name, DI_NAMESZ);
                                else {
                                    data = mscp_info(indx, &td.di_unit);
                                    if (data)
                                        bcopy(data, td.di_name, DI_NAMESZ);
#ifdef __alpha
                                    else {
					td.di_unit = indx;
					bcopy("dk", td.di_name, 3);
				    }
#else
                                    else
                                        goto bad;
#endif
                                }
                                td.di_name[DI_NAMESZ] = '\0';
                        }
#endif
#ifdef multimax
#define __DKNAME
                        {
                                extern char *msdinfo();
                                data = msdinfo(indx, &td.di_unit);
                                if (data)
                                        bcopy(data, td.di_name, DI_NAMESZ);
                                else
                                        goto bad;
                                td.di_name[DI_NAMESZ] = '\0';
                        }
#endif
#ifndef __DKNAME
                        td.di_unit = indx;
                        bcopy("dk", td.di_name, 3);
#endif                        
#undef __DKNAME
			data = (caddr_t)&td;
                        size = sizeof(td);
                        break;
                case TBL_TTYINFO:
			if (indx != 0 || nel != 1)
				goto bad;
                        tt.ti_nin = tk_nin;
                        tt.ti_nout = tk_nout;
                        tt.ti_cancc = tk_cancc;
                        tt.ti_rawcc = tk_rawcc;
			/*
			 * These would requre tty driver changes,
			 * so left unsupported.
			 */
			tt.rcvin = 0;
			tt.xmtin = 0;
			tt.mdmin = 0; /* not implemented in SVR4 */

                        size = sizeof(tt);
                        break;
 		case TBL_MSGDS:
			if (indx > msginfo.msgmni || indx < 0)
			    goto bad;
	
			data = (caddr_t) &msgque[indx];
			size = sizeof(struct msqid_ds);

			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}

			break;
		case TBL_SEMDS:
			if (indx > seminfo.semmni || indx < 0)
			    goto bad;

			data = (caddr_t) &sema[indx];
			size = sizeof(struct semid_ds);

			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}

			break;
		case TBL_SHMDS:
			if (indx > shminfo.shmmni || indx < 0)
			    goto bad;

			data = (caddr_t) &(shmem[indx].s);
			size = sizeof(struct shmid_ds);

			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}

			break;
		case TBL_MSGINFO:
			switch (indx) {
			  case MSGINFO_MAX:
			    data = (caddr_t)&msginfo.msgmax;
			    size = sizeof(msginfo.msgmax);
			    break;
			  case MSGINFO_MNB:
			    data = (caddr_t)&msginfo.msgmnb;
			    size = sizeof(msginfo.msgmnb);
			    break;
			  case MSGINFO_MNI:
			    data = (caddr_t)&msginfo.msgmni;
			    size = sizeof(msginfo.msgmni);
			    break;
			  case MSGINFO_TQL:
			    data = (caddr_t)&msginfo.msgtql;
			    size = sizeof(msginfo.msgtql);
			    break;
			case MSGINFO_SAR:
                		data = (caddr_t)&msginfo.msg;
	                	size = sizeof(msginfo.msg);
				break;
			  default:
			    goto bad;
			}	

			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}
			
			break;
		case TBL_SEMINFO:
			switch (indx) {
			  case SEMINFO_MNI:
			    data = (caddr_t)&seminfo.semmni;
			    size = sizeof(seminfo.semmni);	
			    break;
			  case SEMINFO_MSL:
			    data = (caddr_t)&seminfo.semmsl;
			    size = sizeof(seminfo.semmsl);
			    break;
			  case SEMINFO_OPM:
			    data = (caddr_t)&seminfo.semopm;
			    size = sizeof(seminfo.semopm);
			    break;				
			  case SEMINFO_UME:
			    data = (caddr_t)&seminfo.semume;
			    size = sizeof(seminfo.semume);
			    break;
			  case SEMINFO_VMX:
			    data = (caddr_t)&seminfo.semvmx;
			    size = sizeof(seminfo.semvmx);
			    break;
			  case SEMINFO_AEM:
			    data = (caddr_t)&seminfo.semaem;
			    size = sizeof(seminfo.semaem);
			    break;
			case SEMINFO_OPT:
                		data = (caddr_t)&seminfo.sema;
                		size = sizeof(seminfo.sema);
                		break;
			  default:
			    goto bad;
			}	
			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}	
			
			break;
		case TBL_SHMINFO:
			switch (indx) {
			  case SHMINFO_MAX:
			    data = (caddr_t)&shminfo.shmmax;
			    size = sizeof(shminfo.shmmax);
			    break;
			  case SHMINFO_MIN:
			    data = (caddr_t)&shminfo.shmmin;
			    size = sizeof(shminfo.shmmin);
			    break;
			  case SHMINFO_MNI:
			    data = (caddr_t)&shminfo.shmmni;
			    size = sizeof(shminfo.shmmni);
			    break;
			  case SHMINFO_SEG:
			    data = (caddr_t)&shminfo.shmseg;
			    size = sizeof(shminfo.shmseg);
			    break;
			  default:
			    goto bad;
			}		

			if (size != uap->lel) {
			    error = EINVAL;
			    goto bad;
			}	

			break;
		case TBL_SWAPINFO:
		    {
			extern lock_data_t vm_swap_modify_lock; /* vm/vm_swap.c */
			struct tbl_swapinfo swapinfo;

			/*
			** Lock down the swap device list.
			** Process the request depending
			** upon the index value.
			*/
			lock_read(&vm_swap_modify_lock);
			if (indx < 0) {
			  /*
			  ** Return default swap and VM's notion of
			  ** swap availability when the index is
			  ** negative.
			  */
			  swapinfo.flags = vm_swap_eager;
			  swapinfo.size  = vm_total_swap_space;
			  swapinfo.free  = vm_swap_space;
			  if (vm_swap_lazy)
			    swapinfo.dev = vm_swap_lazy->vs_vinfo.vps_dev;
			  else
			    swapinfo.dev = 0;
			  swapinfo.ino = 0;	/* Should swap files ever exist */
			  }
			else {
			  /*
			  ** Return information about this positive
			  ** (>= 0) partition in the swap list.
			  */
			  int i;
			  struct vm_swap *swap;
			
			  if (!(swap = vm_swap_head)) {
			      lock_done(&vm_swap_modify_lock);
			      goto bad;
			  }
			      
			  for (i = 0; i <= indx; i++) {
			    /*
			    ** If we linked completely around the list, and
			    ** the index is not 0 (i.e. first entry), then
			    ** we've seen this entry before and should return
			    ** an error.
			    */
			    if (i > 0 && swap == vm_swap_head) {
			      lock_done(&vm_swap_modify_lock);
			      goto bad;
			      }

			    /*
			    ** If this is the one, build the tbl_swapinfo.
			    */
			    if (i == indx) {
			      swapinfo.flags = swap->vs_flags;
			      swapinfo.size  = swap->vs_swapsize;
			      swapinfo.free  = swap->vs_freespace;
			      swapinfo.dev   = swap->vs_vinfo.vps_dev;
			      swapinfo.ino   = 0; /* Should swap files ever exist */
			      }

			    /*
			    ** Advance to the next entry in the list.
			    */
			    swap = swap->vs_fl;
			    } /* for */
			  } /* else */

			/*
			** We can only be here if we were successful.
			** Attempt to return the swapinfo we just compiled.
			*/
			size = MIN(sizeof(swapinfo), uap->lel);
			if (size)
			  error = copyout(&swapinfo, uap->addr, size);
			size = 0;
			lock_done(&vm_swap_modify_lock);
			break;
		    }

		case TBL_FILEINFO:
			/*
			 * file access information table
			 */
		        if (uap->index != 0 || uap->nel != 1)
				goto bad;
		        fi.iget = tf_iget;
		        fi.namei = tf_namei;
		        fi.dirblk = tf_dirblk;
		        size = sizeof(fi);
		        break;
		case TBL_BUFFER:
			/*
			 * buffer activity
			 */

			/*
			 * no indexing and examine only
			 */
			if (uap->index != 0 || uap->nel != 1)
				goto bad;

			bu.bread = ts_bread;
			bu.bwrite = ts_bwrite;
			bu.lread = ts_lread;
			bu.lwrite = ts_lwrite;
			bu.phread = ts_phread;
			bu.phwrite = ts_phwrite;
			size = sizeof(bu);
			break;

		case TBL_SCALLS:
			/*
			 * system call activity
			 */
			{
			int i;
			extern int scmax; /* global - max # of system calls */
			extern int sccount[]; /* global - system call counts */
	
			/*
			 * no indexing and examine only
			 */
			if (uap->index != 0 || uap->nel != 1)
				goto bad;
	
			sc.syscall = 0;
	
			/* add up all system calls */
			for(i=0; i < scmax; i++)
				sc.syscall += sccount[i];

			sc.sysfork = sccount[SYS_fork];
			sc.sysread = ts_sysread;
			sc.syswrite = ts_syswrite;
			sc.sysexec = ts_sysexec;
			sc.readch = ts_readch;
			sc.writech = ts_writech;
			size = sizeof(sc);
			break;
			}

		case TBL_RUNQ:
			{
			extern long sar_runqeueue;

			/*
			 * run queue activity
			 */
	
			/*
			 * no indexing and examine only
			 */
			if (uap->index != 0 || uap->nel != 1)
				goto bad;
	
			rq.runque = sar_runqueue;
			rq.runocc = sar_runocc;

			size = sizeof(rq);
			break;
			}

		case TBL_TBLSTATS:
			/*
			 * system tables status
			 */
			{
			struct proc *procp;	/* pointer to beginning of process table */
			register a;		/* process count holder */
	
			/*
			 * no indexing and examine only
			 */
			if (uap->index != 0 || uap->nel != 1)
				goto bad;
	
			a=0;
			for (procp = allproc; procp; procp = procp->p_nxt)
				a++;
			for (procp = zombproc; procp; procp = procp->p_nxt)
				a++;
	
			tb.tb_proca = nproc; /* # of allocated process table entries */
			tb.tb_procu = a; /* # of used process table entries */
			/* # of allocated inode (vnodes) table entries */
			tb.tb_inoda = nvnode;

			/* The variable "numvnodes" is actually the # of FREE 
			 * inode (vnode) table entries -- NOT # of used inode
			 * table entries.  Therefore, tb.tb_inode should be
			 * nvnode - numvnodes (# of allocated inode - # of 
			 * free inode = # of used inode).
			 */
			/* # of used inode (vnodes) table entries */
			tb.tb_inodu = nvnode - numvnodes;

			 /* file table entries allocated dynamically */
			tb.tb_filea = 0;

			/*
			 * # of used file table entries,
			 * svr4_fte_used() defined locally
			 */
			tb.tb_fileu = svr4_fte_used();

			/* shared memory allocated dynamically */
			tb.tb_locka = 0;

			/*
			 * # of used shared memory table entries
			 * svr4_shmem_cnt is defined in bsd/svipc_shm.c
			 */
			tb.tb_locku = svr4_shmem_cnt();

			/* # of overflows for proc table */
			tb.tb_procov = tbl_proc_ov;
			/* # of overflows for inode table */
			tb.tb_inodov = tbl_inod_ov;
			/* # of overflows for file table */
			tb.tb_fileov = tbl_file_ov;

			size = sizeof(tb);
			break;
			}

		case TBL_KMEM:
			/*
	 		* support for sar -k - kernel memory stats
	 		*/
			{
			vm_map_entry_t	entry;
			vm_offset_t	entrysize;
			extern vm_map_t kernel_map;

			register int	i;

			/* global kernel mem failure counter */
			km.kmem_fail = sar_kmem_fail;

			vm_map_lock(kernel_map);
			if (kernel_map == VM_MAP_NULL) {
				vm_map_unlock(kernel_map);
				break;
			}
			km.kmem_avail = vm_map_max(kernel_map) -
					vm_map_min(kernel_map);
			entrysize = 0;
			entry = kernel_map->vm_links.next;

			for (i = 1; i <= kernel_map->vm_nentries; i++) { 
				if (entry == VM_MAP_ENTRY_NULL) {
					entrysize = 0;
					break;
				}
				entrysize += (entry->vme_end - entry->vme_start);
				entry = entry->vme_next;
			}
			km.kmem_alloc = entrysize;
			vm_map_unlock(kernel_map);

			size = sizeof(km);
			break;
			}

		case TBL_PAGING:
			/*
	 		* support for sar -p, sar -g and sar -r
	 		*/
			
			/* # of pages paged-in */
			pg.v_pgpgin = pg_v_pgpgin;
			/* # of software lock faults */
			pg.v_sftlock = pg_v_sftlock;
			/* # of page-out requests */
			pg.v_pgout = pg_v_pgout;
			/* # of pages freed */
			pg.v_dfree = pg_v_dfree;
			/* # of pages scanned */
			pg.v_scan = pg_v_scan;
			/* # of s5 inodes taken of freelist */
			pg.v_s5ifp = pg_v_s5ifp;

			/*
	 		 * support for sar -f flag - # of free disk blocks
			 * available for page swapping.
			 * svr4_freeswap_space defined in vm/vm_swap.c
			 */
			pg.freeswap = svr4_freeswap_space();
			
			size = sizeof(pg);
			break;

		case TBL_MAPINFO:
			{
			vm_offset_t next_addr;
			if (*retval == 0
			    && (error = pid_to_proc(MAPINFO_PID(indx), &p)))
				return error;
			vm_map_reference(proc_map = p->task->map);
			next_addr = vm_mapinfo(proc_map,
						   ptoa(MAPINFO_VPN(indx)),
						   &tm);
			vm_map_deallocate(proc_map);
			if (next_addr)
				MAPINFO_VPN(indx) = atop(next_addr) - 1;
			else
				goto bad;
			size = sizeof(tm);
			break;
			}

		case TBL_MALLOCBUCKETS:
			if (indx >= MINBUCKET + 16 || indx < 0)
				goto bad;

			data = (caddr_t) &(bucket[indx]);
			size = sizeof(struct kmembuckets);

			break;

		case TBL_MALLOCTYPES:
			if (indx >= M_LAST || indx < 0)
				goto bad;

			data = (caddr_t) &(kmemtypes[indx]);
			size = sizeof(struct kmemtypes);

			break;
		case TBL_MALLOCNAMES:
			if (indx >= M_LAST || indx < 0)
				goto bad;

			data = (caddr_t) &kmemnames[indx][0];
			size = KMEMNAMSZ;

			break;
		default:
		bad:
			/*
			 *	Return error only if all indices
			 *	are invalid.
			 */
			if (*retval == 0)
				error = EINVAL;
			else
				error = 0;
			return (error);
		}
		/*
		 * This code should be generalized if/when other tables
		 * are added to handle single element copies where the
		 * actual and expected sizes differ or the table entries
		 * are not contiguous in kernel memory (as with TTYLOC)
		 * and also efficiently copy multiple element
		 * tables when contiguous and the sizes match.
		 */
		size = MIN(size, uap->lel);
		if (size) {
			if (set) {
				char buff[MAXLEL];

			        error = copyin(uap->addr, buff, size);
				if (error == 0)
					bcopy(buff, data, size);
			}
			else {
				error = copyout(data, uap->addr, size);
			}
		}
		if (dealloc_start != (vm_offset_t) 0) {
			kmem_free(kernel_copy_map, dealloc_start,
				dealloc_end - dealloc_start);
		}
		if (error)
			return (error);
		uap->addr += uap->lel;
		nel -= 1;
		indx += 1;
		*retval += 1;
#undef ttyd
#undef iv
#undef tl
#undef tp
#undef ts
#undef ti
#undef td
#undef tt
#undef bu
#undef fi
#undef sc
#undef rq
#undef tb
#undef km
#undef pg
#undef tm
	}
	return(0);
}

/*
 * Walk the process list and count the number of open
 * file table entries per-process
 */
long
svr4_fte_used()
{
	struct proc *procp;	/* process table pointer */
	struct ufile_state *ofp;      /* per-process open file table pointer */
	struct file *fp;
	long used_fte_count;
	int i;

	used_fte_count=0;
	for (procp = allproc; procp; procp = procp->p_nxt) {
		if (procp->utask == NULL)
			continue;
		U_FDTABLE_LOCK(&procp->u_file_state);
		for (i=0; i < procp->u_file_state.uf_lastfile; i++) {

			if ((fp = U_OFILE(i, &procp->u_file_state)) == NULL)
				continue;
			if (fp == U_FD_RESERVED)
				continue;

			/*
			 *I believe we want all table entries which
			 * includes communications endpoints (sockets).
			 * if you just want files, uncomment the following:

			if ((fp->f_type != DTYPE_VNODE))
				continue;

			 */

			/*
			 * open file entry found
			 */
			used_fte_count++;
		}
		U_FDTABLE_UNLOCK(&u.u_file_state);
	}
	return(used_fte_count);
}

static int
pid_to_proc(pid, proc_p)
	int			pid;
	struct proc		**proc_p;
{
	register struct proc	*p;

	p = pfind(pid);
	if (p == (struct proc *)0) {
		/*
		 *	Proc 0 isn't in the hash table
		 */
		if (pid == 0)
			p = &proc[0];
		else {
			/*
			 *	No such process
			 */
			return (ESRCH);
		}
	}

	if (p->p_stat == SIDL || p->p_stat == SZOMB)
		return (ESRCH);

	*proc_p = p;
#if	SEC_BASE
	/*
	 * To retrieve information about another process
	 * requires privilege.
	 */
	if (p != u.u_procp && !privileged(SEC_DEBUG, EPERM))
		return (EPERM);
#else
	if (p != u.u_procp)
		return suser(u.u_cred, &u.u_acflag);
#endif
	return 0;
}

/*
 * sar_runq_update(): called once per second by sched_thread to snapshot
 * the state of the system's run qeueues. These global values are
 * involved:
 * 	sar_runqueue: cummulative count of threads in the runq.
 *	sar_runocc: cummulative count of the number of times the run queue
 *		has been occupied at the 1-second check.
 * So, we count the number of runnable processes. If there are any,
 * we update the running runqueue tally and increment the runocc counter.
 */
void
sar_runq_update()
{
	extern long sar_runqeueue;
	register processor_t myprocessor = cpu_to_processor(cpu_number());
	register unsigned long count;

	count =  myprocessor->processor_set->runq.count;
	count += myprocessor->runq.count;
	if (count) {
		sar_runqueue += count;
		sar_runocc++;
	}
}
