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
static char *rcsid = "@(#)$RCSfile: standard.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/17 20:22:46 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1988-1991  Mentat Inc.
 **/

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>

static int	drv_open(queue_t *, dev_t *, int, int, cred_t *);
static int	echo_wput(queue_t *, mblk_t *);
static int	null_lrput(queue_t *, mblk_t *);
static int	null_rput(queue_t *, mblk_t *);
static int	null_wput(queue_t *, mblk_t *);
static int	pass_put(queue_t *, mblk_t *);
static int	spass_put(queue_t *, mblk_t *);
static int	spass_srv(queue_t *);
static int	pmod_put(queue_t *, mblk_t *);

/*
 * Echo is a simple echo driver.
 */
static struct module_info echominfo =  {
	5000, "echo", 0, INFPSZ, 2048, 128
};
static struct qinit echorinit = {
	NULL, NULL, drv_open, streams_close_comm, NULL, &echominfo
};
static struct qinit echowinit = {
	echo_wput, spass_srv, NULL, NULL, NULL, &echominfo
};
struct streamtab echoinfo = { &echorinit, &echowinit };

/*
 * Null is a discard driver, and also functions as a mux or module.
 */
static struct module_info nullinfo =  {
	5001, "nuls", 0, INFPSZ, 512, 128
};
static struct qinit nulllrinit = {	/* lower read */
	null_lrput, NULL, NULL, NULL, NULL, &nullinfo
};
static struct qinit nulllwinit = {	/* lower write */
	NULL, NULL, NULL, NULL, NULL, &nullinfo
};
static struct qinit nullrinit = {
	NULL, NULL, drv_open, streams_close_comm, NULL, &nullinfo
};
static struct qinit nullwinit = {
	null_wput, NULL, NULL, NULL, NULL, &nullinfo
};
struct streamtab nulsinfo ={ &nullrinit, &nullwinit, &nulllrinit, &nulllwinit };

static struct module_info nulinfo =  {
	5002, "null", 0, INFPSZ, 512, 128
};
static struct qinit nulmrinit = {
	null_rput, NULL, drv_open, streams_close_comm, NULL, &nulinfo
};
static struct qinit nulmwinit = {
	null_wput, NULL, NULL, NULL, NULL, &nulinfo
};
struct streamtab nulminfo ={ &nulmrinit, &nulmwinit };

/*
 * Pass is a passthrough module with an "errm" and "ptem" alterego.
 * Since the Pseudo-tty emulation functionality is 
 * provided by the slave pty driver (pts)
 * the "ptem" alterego  is provided for SVR4 compatibility.
 */
static struct module_info passminfo =  {
	5003, "pass", 0, INFPSZ, 2048, 128
};
static struct qinit passrinit = {
	pass_put, NULL, drv_open, streams_close_comm, NULL, &passminfo
};
static struct qinit passwinit = {
	pass_put, NULL, NULL, NULL, NULL, &passminfo
};
static struct qinit passeinit = {
	NULL, NULL, NULL, NULL, NULL, &passminfo
};
struct streamtab passinfo = { &passrinit, &passwinit };
struct streamtab modeinfo = { &passrinit, &passeinit };
struct streamtab pteminfo = { &passrinit, &passwinit };

/*
 * Spass is a passthrough via a service procedure.
 */
static struct module_info spassminfo =  {
	5007, "spass", 0, INFPSZ, 2048, 128
};
static struct qinit spassrinit = {
	spass_put, spass_srv, drv_open, streams_close_comm, NULL, &spassminfo
};
static struct qinit spasswinit = {
	spass_put, spass_srv, NULL, NULL, NULL, &spassminfo
};
struct streamtab spassinfo = { &spassrinit, &spasswinit };

/*
 * Rspass is a read side half-spass handy for pushing on
 * drivers which call putnext in interrupt context.
 */
static struct module_info rspassminfo =  {
	5008, "rspass", 0, INFPSZ, 2048, 128
};
static struct qinit rspassrinit = {
	spass_put, spass_srv, drv_open, streams_close_comm, NULL, &rspassminfo
};
static struct qinit rspasswinit = {
	pass_put, NULL, NULL, NULL, NULL, &rspassminfo
};
struct streamtab rspassinfo = { &rspassrinit, &rspasswinit };

/*
 * Pipe and pipemod implement pipes and fifo's.
 */
static struct module_info pipminfo =  {
	5303, "pipemod", 0, INFPSZ, PIPSIZ, PIPSIZ-1
};
static struct qinit pipmrinit = {
	pmod_put, NULL, drv_open, streams_close_comm, NULL, &pipminfo
};
static struct qinit pipmwinit = {
	pmod_put, NULL, NULL, NULL, NULL, &pipminfo
};
struct streamtab pmodinfo = { &pipmrinit, &pipmwinit };

static struct module_info pipinfo =  {
	5304, "pipe", 0, INFPSZ, PIPSIZ, PIPSIZ-1
};
static struct qinit piperinit = {
	pmod_put, NULL, drv_open, streams_close_comm, NULL, &pipinfo
};
static struct qinit pipewinit = {
	pmod_put, NULL, NULL, NULL, NULL, &pipinfo
};
struct streamtab pipeinfo = { &piperinit, &pipewinit };


extern dev_t clonedev;
/* Following in log.c and sad.c */
extern struct streamtab loginfo, sadinfo;
extern void strlog_init(void), sad_init(void);

static struct {
	char	name[FMNAMESZ+1];
	struct	streamtab *info;
	int	flags;
	int	level;
	major_t maj;
	void	(*init)(void);
} std_entries[] = {
	{ "log",	&loginfo,
		STR_IS_DEVICE|STR_SYSV4_OPEN, SQLVL_MODULE, 0, strlog_init },
	{ "nuls",	&nulsinfo,
		STR_IS_DEVICE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0, NULL },
	{ "echo",	&echoinfo,
		STR_IS_DEVICE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0, NULL },
	{ "sad",	&sadinfo,
		STR_IS_DEVICE|STR_SYSV4_OPEN, SQLVL_MODULE, 0, sad_init },
	{ "pipe",	&pipeinfo,
		STR_IS_DEVICE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0, NULL },

	{ "null",	&nulminfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0, NULL },
	{ "pass",	&passinfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL },
	{ "errm",	&modeinfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL },
	{ "ptem",	&pteminfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL },
	{ "spass",	&spassinfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL },
	{ "rspass",	&rspassinfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL },
	{ "pipemod",	&pmodinfo,
		STR_IS_MODULE|STR_SYSV4_OPEN, SQLVL_QUEUE,  0,  NULL }
};
#define NSTD   (sizeof std_entries / sizeof std_entries[0])

int
strstd_configure(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t	op;
	str_config_t *	indata;
	size_t		indatalen;
	str_config_t *	outdata;
	size_t		outdatalen;
{
	struct streamadm	sa;
	dev_t			devno;
        int			index;
	static int		next_std;

	switch (op) {
	case SYSCONFIG_CONFIGURE:
	if (next_std == NSTD)
		return ENOENT;

	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= std_entries[next_std].flags;
	sa.sa_ttys		= 0;
	sa.sa_sync_level	= std_entries[next_std].level;
	sa.sa_sync_info		= 0;
	strcpy(sa.sa_name,	  std_entries[next_std].name);
	devno			= NODEV; /* will find availible device slot */
	index			= next_std;
	if (std_entries[next_std].init)
		(*std_entries[next_std].init)();

	devno = strmod_add(devno, std_entries[next_std++].info, &sa);
	if (devno == NODEV)
		return ENODEV;
	std_entries[index].maj = major(devno);

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = makedev(major(clonedev), major(devno));
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy(outdata->sc_sa_name, sa.sa_name);
	}
	break;

	case SYSCONFIG_QUERY:
        /*
	 * Return query information within outdata based upon what
	 * indata->sc_sa_name is.
	 */
	
	for (index = 0; index < NSTD; index++) {
	    if (strcmp(indata->sc_sa_name, std_entries[index].name) == 0) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = makedev(major(clonedev), std_entries[index].maj);
		outdata->sc_sa_flags = std_entries[index].flags;
		strcpy(outdata->sc_sa_name, std_entries[index].name);
		bcopy(outdata,indata, min(outdatalen,indatalen));
	    }
	}
	break;

	default:
        return(EINVAL);
	}
	return 0;
}

/*
 * Common open routine for these drivers and modules.
 */
static int
drv_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err, (*putp)() = WR(q)->q_qinfo->qi_putp;

	if (putp == NULL)	/* errm */
		return ENXIO;
	if (putp == pmod_put && sflag != MODOPEN && devp)
		*devp = NODEV;	/* pipes must be non-aliased */
	err = streams_open_comm(0, q, devp, flag, sflag, credp);
	if (putp == echo_wput && err == 0
	&&  sflag != MODOPEN && !WR(q)->q_next) {
		extern void wakeup();
		err = weldq(WR(q), q, (queue_t *)0, (queue_t *)0, wakeup, q, q);
		if (err == 0)
			sleep((caddr_t)q, PZERO);
	}
	return err;
}

static int
echo_wput (q, mp)
	queue_t	* 	q;
	mblk_t *	mp;
{
	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & (FLUSHR|FLUSHW)) {
			if ((*mp->b_rptr & FLUSHBAND)
			&&  mp->b_wptr - mp->b_rptr == 2)
				flushband(q, mp->b_rptr[1], FLUSHALL);
			else
				flushq(q, FLUSHALL);
		}
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 1;
		}
		break;
	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		return 1;
	default:
		if (mp->b_datap->db_type >= QPCTL
		||  bcanput(RD(q)->q_next, mp->b_band)) {
			qreply(q, mp);
			return 1;
		}
		if (putq(q, mp))
			return 1;
		break;
	}
	freemsg(mp);
	return 0;
}

static int
null_lrput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	if (mp->b_datap->db_type == M_FLUSH) {
		/* Tests are reversed because we are the acting "stream head"*/
		if (*mp->b_rptr & FLUSHW) {
			*mp->b_rptr &= ~FLUSHR;
			qreply(q, mp);
			return 1;
		}
	}
	freemsg(mp);
	return 1;
}

/* For use when configured as a module (as opposed to a device) */
static int
null_rput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		break;
	case M_DATA:
		if (msgdsize(mp) == 0)
			break;
		/* fall through */
	default:
		freemsg(mp);
		return 1;
	}
	putnext(q, mp);
	return 1;
}

static int
null_wput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		switch (((struct iocblk *)mp->b_rptr)->ioc_cmd) {
		case I_PLINK:
		case I_PUNLINK:
		case I_LINK:
		case I_UNLINK:
			mp->b_datap->db_type = M_IOCACK;
			((struct iocblk *)mp->b_rptr)->ioc_count = 0;
			((struct iocblk *)mp->b_rptr)->ioc_error = 0;
			break;
		default:
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		return 1;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 1;
		}
		break;
	}
	freemsg(mp);
	return 1;
}

static int
pass_put (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	putnext(q, mp);
	return 1;
}

static int
spass_put (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	if (mp->b_datap->db_type == M_FLUSH) {
		if (*mp->b_rptr & ((q->q_flag & QREADR) ? FLUSHR : FLUSHW)) {
			if ((*mp->b_rptr & FLUSHBAND)
			&&  mp->b_wptr - mp->b_rptr == 2)
				flushband(q, mp->b_rptr[1], FLUSHALL);
			else
				flushq(q, FLUSHALL);
		}
		putnext(q, mp);
		return 1;
	}
	if (putq(q, mp))
		return 1;
	freemsg(mp);
	return 0;
}

static int
spass_srv (q)
	queue_t	* q;
{
	mblk_t * mp;

	while (mp = getq(q)) {
		if (mp->b_datap->db_type < QPCTL
		&&  !bcanput(q->q_next, mp->b_band)) {
			(void) putbq(q, mp);
			break;
		}
		putnext(q, mp);
	}
	return 0;
}

static int
pmod_put (q, mp)
	queue_t	* q;
	mblk_t	* mp;
{
	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		mp->b_datap->db_type = M_IOCNAK;
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = 0;
		}
		qreply(q, mp);
		break;
	case M_FLUSH:
		if (q->q_flag & QREADR) {
			if (mp->b_rptr[0] & FLUSHR)
				mp->b_rptr[0] &= ~FLUSHR;
				mp->b_rptr[0] |= FLUSHW;
		} else {
			if (mp->b_rptr[0] & FLUSHW)
				mp->b_rptr[0] &= ~FLUSHW;
				mp->b_rptr[0] |= FLUSHR;
		}
		/* fall through */
	default:
		putnext(q, mp);
		break;
	}
	return 1;
}
