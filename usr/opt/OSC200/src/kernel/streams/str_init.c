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
static char *rcsid = "@(#)$RCSfile: str_init.c,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/09/30 18:33:55 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/** Copyright (c) 1989-1991  Mentat Inc.  **/

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

staticf	dev_t	add_device(dev_t, struct streamtab *, struct streamadm *);
staticf	dev_t	add_module(struct streamtab *, struct streamadm *);
staticf	int	del_device(dev_t, struct streamtab *, struct streamadm *);
staticf	int	del_module(struct streamtab *, struct streamadm *);

extern int nodev(), nulldev();

dev_t
strmod_add(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	switch (streamadm->sa_version) {
	case OSF_STREAMS_10:
	case OSF_STREAMS_11:
		switch (streamadm->sa_flags & STR_TYPE_MASK) {
		default:
			return NODEV;
		case STR_IS_DEVICE:
			return add_device(devno, streamtab, streamadm);
		case STR_IS_MODULE:
			return add_module(streamtab, streamadm);
		}
	}
	return NODEV;
}

int
strmod_del(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	switch (streamadm->sa_version) {
	case OSF_STREAMS_10:
	case OSF_STREAMS_11:
		switch (streamadm->sa_flags & STR_TYPE_MASK) {
		default:
			return EINVAL;
		case STR_IS_DEVICE:
			return del_device(devno, streamtab, streamadm);
		case STR_IS_MODULE:
			return del_module(streamtab, streamadm);
		}
	}
	return EINVAL;
}

static int	idnum;

staticf dev_t
add_device(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	struct cdevsw		str_entry;

	bzero((caddr_t)&str_entry, sizeof(str_entry));
	str_entry.d_open   = pse_open;
	str_entry.d_close  = pse_close;
	str_entry.d_read   = pse_read;
	str_entry.d_write  = pse_write;
	str_entry.d_ioctl  = pse_ioctl;
	str_entry.d_stop   = nodev;
	str_entry.d_reset  = nulldev;
	str_entry.d_ttys   = (struct tty *)streamadm->sa_ttys;
	str_entry.d_select = pse_select;
	str_entry.d_mmap   = nodev;
	str_entry.d_segmap = NULL;
	str_entry.d_funnel = NULL;
	str_entry.d_flags  = NULL;

	if ( (devno = cdevsw_add(devno, &str_entry)) != NODEV) {
		if (dmodsw_install(streamadm, streamtab, major(devno)))
			(void) cdevsw_del(devno);
		else {
			if (!streamtab->st_rdinit->qi_minfo->mi_idnum)
				streamtab->st_rdinit->qi_minfo->mi_idnum = ++idnum;
			return devno;
		}
	}
	return NODEV;
}

staticf int
del_device(devno, streamtab, streamadm)
	dev_t			devno;
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	int error;

	error = dmodsw_remove(streamadm->sa_name);
	if (error == 0)
		error = cdevsw_del(devno);
	return error;
}

staticf dev_t
add_module(streamtab, streamadm)
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	if ( fmodsw_install( streamadm, streamtab ) )
		return NODEV;
	return 0;
}

staticf int
del_module(streamtab, streamadm)
	struct streamtab *	streamtab;
	struct streamadm *	streamadm;
{
	return fmodsw_remove(streamadm->sa_name);
}

/*
 * pse_init - global initialization routine for streams module.
 */

int
pse_init ()
{
	sth_muxid_init();
	sqh_init(&sq_sqh);
	sqh_init(&mult_sqh);
	mult_sqh.sqh_parent = &mult_sqh;
	bufcall_init();
	str_to_init();
	act_q_init();
	sth_iocblk_init();
#if	STREAMS_DEBUG
	DB_init();
#endif
	runq_init();
	(void) weldq_init();
	str_open_init();
	str_modsw_init();
	str_config();		/* configure statically bound modules */

	return 0;
}




