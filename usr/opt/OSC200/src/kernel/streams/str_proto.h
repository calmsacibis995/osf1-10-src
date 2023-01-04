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
 * @(#)$RCSfile: str_proto.h,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1994/01/20 18:43:55 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * str_modsw.c
 */
extern	void		str_modsw_init(void);
extern	int		modsw_ref(struct qinit *, int);
extern	int		dcookie_to_dindex(int);
extern	struct streamtab * dindex_to_str(int);
extern	int		dname_to_dcookie(char *);
extern	int		dname_to_dindex(char *);
extern	struct streamtab * dname_to_str(char *);
extern	struct streamtab * dqinfo_to_str(struct qinit *);
extern	struct streamtab * fname_to_str(char *);
extern	int		dmodsw_install(struct streamadm *, struct streamtab *, int);
extern	int		dmodsw_remove(char *);
extern	int		fmodsw_install(struct streamadm *, struct streamtab *);
extern	int		fmodsw_remove(char *);
extern	char *		qinfo_to_name(struct qinit *);
extern	struct streamtab * mid_to_str(ushort);
extern	SQHP		sqh_set_parent(queue_t *, struct streamtab *);

/*
 * str_config.c
 */
extern	void		str_config(void);

/*
 * str_tty.c
 */
extern	void		sth_ttyopen(STHP, int);
extern	void		sth_ttyclose(STHP);
extern	int		sth_tiocsctty(STHP, struct proc *);
extern	void		sth_pgsignal(STHP, int);
extern	int		sth_ttyioctl(struct tty *, dev_t, int, caddr_t, int);

/*
 * str_env.c
 */
extern	void		streams_lbolt_init(void);
extern	void		str_to_init(void);

/*
 * str_filesys.c
 */
extern	int		fd_to_cookie(int, struct file_cookie *);
extern	int		fd_alloc(struct file_cookie *, int *);
extern	void		cookie_destroy(struct file_cookie *);
extern	int		sth_fd_to_sth(int, STHPP);
extern	void		sth_update_times(STHP, int, struct stat *);
extern	int		sth_fattach(STHP, int, void *);

/*
 * str_init.c
 */
extern	int		pse_init(void);

/*
 * str_memory.c
 */
extern	caddr_t		he_alloc(long, int);
extern	void		he_free(caddr_t);
extern	caddr_t		he_realloc(caddr_t, long, long);
extern	int		bufcall_configure(/* TODO */);
extern	int		bufcall_init(void);
extern	int		bufcall_rsrv(queue_t *);

/*
 * str_osr.c and str_tty.c
 */
extern	void		discard_passfp(MBLKP);
extern	int		osr_atmark(OSRP);
extern	int		osr_canput(OSRP);
extern	int		osr_ckband(OSRP);
extern	int		osr_fattach(OSRP);
extern	int		osr_fdinsert(OSRP);
extern	int		osr_fifo(OSRP);
extern	int		osr_find(OSRP);
extern	int		osr_fionread(OSRP);
extern	int		osr_flush(OSRP);
extern	int		osr_flushband(OSRP);
extern	int		osr_getband(OSRP);
extern	int		osr_getcltime(OSRP);
extern	int		osr_getmsg(OSRP);
extern	int		osr_getpmsg(OSRP);
extern	int		osr_grdopt(OSRP);
extern	int		osr_gwropt(OSRP);
extern	int		osr_getsig(OSRP);
extern	int		osr_isastream(OSRP);
extern	int		osr_link(OSRP);
extern	int		osr_list(OSRP);
extern	int		osr_look(OSRP);
extern	int		osr_nread(OSRP);
extern	int		osr_peek(OSRP);
extern	int		osr_pipe(OSRP);
extern	int		osr_pipestat(OSRP);
extern	int		osr_pop(OSRP);
extern	int		osr_pop_subr(OSRP, queue_t *);
extern	int		osr_push(OSRP);
extern	int		osr_putmsg(OSRP);
extern	int		osr_putpmsg(OSRP);
extern	int		osr_read(OSRP);
extern	int		osr_recvfd(OSRP);
extern	int		osr_sendfd(OSRP);
extern	int		osr_setcltime(OSRP);
extern	int		osr_tiocspgrp(OSRP);
extern	int		osr_setsig(OSRP);
extern	int		osr_srdopt(OSRP);
extern	int		osr_swropt(OSRP);
extern	int		osr_str(OSRP);
extern	int		osr_tioccons(OSRP);
extern	int		osr_tiocgpgrp(OSRP);
extern	int		osr_tiocgsid(OSRP);
extern	int		osr_tiocsctty(OSRP);
extern	int		osr_unlink(OSRP);
extern	int		osr_unlink_subr(OSRP, int, int, OSRQP);
extern	int		osr_write(OSRP);

/*
 * str_runq.c
 */
extern	void		runq_init(void);
extern	void		runq_run(void);
extern	void		runq_sq_init(queue_t *);
extern	void		runq_remove(queue_t *);

/*
 * str_scalls.c
 */
extern	void		str_open_init(void);
extern	int		pse_open(dev_t, int, int, dev_t *,
					struct ucred *, void **);
extern	int		osr_open(OSRP);
extern	int		osr_reopen(OSRP);
extern  int             clone_open(dev_t, int, int, dev_t *, 
				   struct ucred *, void **);
extern	int		pse_close(dev_t, int, int, struct ucred *, void *);
extern	int		osr_close_subr(OSRQP);
extern	int		pse_read(dev_t, struct uio *, int, void *);
extern	int		pse_write(dev_t, struct uio *, int, void *);
extern	int		pse_ioctl(dev_t, int, caddr_t, int,
					struct ucred *, void *, int *);
extern	int		pse_select(dev_t, short *, short *, int, void *);

/*
 * str_shead.c
 */
extern	int		sth_rput(queue_t *, MBLKP);
extern	void		sth_iocdata(STHP, MBLKP, int);
extern	MBLKP		sth_getq(STHP);
extern	int		sth_canput(STHP, int);

/*
 * str_subr.c
 */
extern	STHP		sth_alloc(void);
extern	void		sth_free(STHP);
extern	queue_t *	q_alloc(void);
extern	int		q_free(queue_t *);
extern	OSRP		osr_alloc(STHP, int, int);
#ifdef  __GNUC__
__inline__
#endif
extern	void		osr_free(OSRP);
extern	STHPP		sth_muxid_lookup(STHP, int, int);
extern	void		sth_iocblk_init(void);
extern	int		sth_iocblk(void);
extern	MBLKP		sth_link_alloc(OSRP, int, int, queue_t *, queue_t *);
extern	int		sth_read_reset(OSRP);
extern	int		sth_read_seek(OSRP, int, long);
extern	int		open_wrapper(struct open_args *);
extern	int		close_wrapper(struct open_args *);
extern	int		sth_uiomove(caddr_t, int, OSRP);
extern	void		sth_uiodone(OSRP);

/*
 * str_synch.c
 */
extern	int		osr_run(OSRP);
extern	void		osrq_init(OSRQP);
extern	void		osrq_insert(OSRQP, OSRP);
extern	OSRP		osrq_remove(OSRQP);
extern	int		osrq_cancel(OSRQP);
extern	int		osr_sleep(OSRP, int, int);
extern	int		osr_bufcall(OSRP, int, int, int, int);
extern	void		osrq_wakeup(OSRQP);

extern	void		act_q_init(void);
extern	void		csq_run(SQP);
typedef	void *		csq_protect_arg_t;
typedef	int		(*csq_protect_fcn_t)(csq_protect_arg_t);
extern	int		csq_protect(queue_t *, queue_t *,
				csq_protect_fcn_t, csq_protect_arg_t, SQP, int);
extern	queue_t *	csq_which_q(void);
extern	void		_csq_acquire(SQHP, SQP); /* macro in str_stream.h */
extern	void		_csq_release(SQHP);	 /* macro in str_stream.h */
extern	void		csq_turnover(SQHP);
extern	void		csq_lateral(SQHP, SQP);
extern	void		mult_sqh_acquire(OSRP);
extern	void		mult_sqh_release(OSRP);
extern	void		csq_newparent(OSRP, queue_t *, struct streamtab *);
extern	void		csq_cleanup(SQHP);

extern	void		sqh_insert(SQHP, SQP);
extern	SQP		sqh_remove(SQP);

/*
 * str_util.c
 *
 * Implements the STREAMS utility routines. Some routines which
 * are closely related to certain modules are found in other
 * files. The prototypes for this file come from the standard
 * include file stream.h. We list here only non-standard
 * extensions, which are for the use by the stream head, but
 * functionally closely related to standard utilities.
 */
extern	int		putq_owned(queue_t *, mblk_t *);


/*
 *	str_weld.c
 */
extern	int		weldq_init(void);
extern	void		weldq_exec(queue_t *, queue_t *, SQP);
extern	void		unweldq_exec(queue_t *, queue_t *, SQP);

