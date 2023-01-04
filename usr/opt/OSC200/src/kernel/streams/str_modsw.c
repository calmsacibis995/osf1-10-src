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
static char *rcsid = "@(#)$RCSfile: str_modsw.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/05/18 16:07:27 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc.
 ** str_modsw.c 1.7, last change 5/12/90
 **/

#include <sys/stat.h>
#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

struct modsw *	dmodsw;
struct modsw *	fmodsw;
decl_simple_lock_data(staticf,modsw_lock)

SQH		sq_sqh;		/* Global Synch Queue */

/* Allocated when SQLVL_ELSEWHERE requested */
struct sq_elsewhere {
	struct	sq_elsewhere *sqe_next;
	struct	sq_elsewhere *sqe_prev;
	char	sqe_info[FMNAMESZ+1];
	int	sqe_refcnt;
	SQH	sqe_sqh;
} *sqe_list;

staticf SQHP sqe_add(caddr_t);
staticf void sqe_del(SQHP);

void
str_modsw_init ()
{
	simple_lock_init(&modsw_lock);
}

/** Allocate and/or reference sq_elsewhere by name */
staticf SQHP
sqe_add (info)
	caddr_t info;
{
	struct sq_elsewhere *sqe, *new_sqe;;

	if (info == 0)
		return 0;
	STR_MALLOC(new_sqe, struct sq_elsewhere *, sizeof *sqe,
					M_STRSQ, M_WAITOK);
	simple_lock(&modsw_lock);
	sqe = sqe_list;
	while (sqe) {
		if (bcmp(info, sqe->sqe_info, FMNAMESZ) == 0)
			break;
		if ((sqe = sqe->sqe_next) == sqe_list) {
			sqe = 0;
			break;
		}
	}
	if (sqe == 0 && (sqe = new_sqe)) {
		new_sqe = 0;
		bzero((caddr_t)sqe, sizeof *sqe);
		bcopy(info, sqe->sqe_info, FMNAMESZ);
		sqh_init(&sqe->sqe_sqh);
		if (sqe_list)
			insque(sqe, sqe_list->sqe_prev);
		else
			sqe_list = sqe->sqe_next = sqe->sqe_prev = sqe;
	}
	if (sqe)
		++sqe->sqe_refcnt;
	simple_unlock(&modsw_lock);
	if (new_sqe)
		STR_FREE(new_sqe, M_STRSQ);
	return (sqe ? &sqe->sqe_sqh : 0);
}

/** Dereference and/or deallocate sq_elsewhere by SQH */
staticf void
sqe_del (sqh)
	SQHP sqh;
{
	struct sq_elsewhere *sqe;

	simple_lock(&modsw_lock);
	sqe = sqe_list;
	while (sqe) {
		if (sqh == &sqe->sqe_sqh)
			break;
		if ((sqe = sqe->sqe_next) == sqe_list) {
			sqe = 0;
			break;
		}
	}
	if (sqe && --sqe->sqe_refcnt <= 0) {
		if (sqe == sqe_list) {
			sqe_list = sqe->sqe_next;
			if (sqe == sqe_list)
				sqe_list = 0;
		}
		remque(sqe);
	}
	simple_unlock(&modsw_lock);
	if (sqe)
		STR_FREE(sqe, M_STRSQ);
}

/** Take or release reference on driver or module */
int
modsw_ref (qi, count)
	struct qinit *	qi;
	int		count;
{
reg	struct modsw	* dmp;
	int	ret = 0;

	simple_lock(&modsw_lock);
	/*
	 * Qi may well be shared. Take or remove all references.
	 */
	dmp = fmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi) {
			dmp->d_refcnt += count;
			++ret;
		}
		if ((dmp = dmp->d_next) == fmodsw)
			break;
	}
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi) {
			dmp->d_refcnt += count;
			++ret;
		}
		if ((dmp = dmp->d_next) == dmodsw)
			break;
	}
	simple_unlock(&modsw_lock);
	return ret;
}

/** Convert cookie to device index - used to verify in table. */
int
dcookie_to_dindex (cookie)
	int	cookie;
{
reg	struct modsw	* dmp;

	if (cookie < 0)
		return -1;
	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( cookie == dmp->d_major )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	return (dmp ? cookie : -1);
}

/** Convert device index to streamtab */
struct streamtab *
dindex_to_str (index)
	int	index;
{
reg	struct modsw	* dmp;

	if (index < 0)
		return 0;
	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( index == dmp->d_major )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	return (dmp ? dmp->d_str : 0);
}

/** Convert device name to device cookie */
int
dname_to_dcookie (name)
	char	* name;
{
reg	struct modsw	* dmp;

	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	return (dmp ? dmp->d_major : -1);
}

/** Convert device name to device index */
int
dname_to_dindex (name)
	char	* name;
{
	return dname_to_dcookie(name);
}

/** Convert device name to streamtab */
struct streamtab *
dname_to_str (name)
	char	* name;
{
reg	struct modsw	* dmp;

	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 )
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	/* If no match, try for an unambiguous idname */
	if (dmp == 0) {
		struct modsw *maybe = 0;
		dmp = dmodsw;
		while (dmp) {
			if (dmp->d_str->st_rdinit
			&&  dmp->d_str->st_rdinit->qi_minfo
			&&  dmp->d_str->st_rdinit->qi_minfo->mi_idname
			&&  strcmp(name, dmp->d_str->st_rdinit->qi_minfo->mi_idname) == 0) {
				if (maybe) {
					dmp = 0;
					break;
				}
				maybe = dmp;
			}
			if ((dmp = dmp->d_next) == dmodsw) {
				dmp = maybe;
				break;
			}
		}
	}
	simple_unlock(&modsw_lock);
	return (dmp ? dmp->d_str : 0);
}

/** Convert rqinit pointer to streamtab from dmodsw */
struct streamtab *
dqinfo_to_str (qi)
	struct qinit	* qi;
{
reg	struct modsw	* dmp;

	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit == qi)
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	return (dmp ? dmp->d_str : 0);
}

/** Convert module name to streamtab */
struct streamtab *
fname_to_str (name)
	char	* name;
{
	struct modsw	* fmp;

	if (name == 0)
		return 0;
	simple_lock(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (strcmp(fmp->d_name, name) == 0)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	/* If no match, try for an unambiguous idname */
	if (fmp == 0) {
		struct modsw *maybe = 0;
		fmp = fmodsw;
		while (fmp) {
			if (fmp->d_str->st_rdinit
			&&  fmp->d_str->st_rdinit->qi_minfo
			&&  fmp->d_str->st_rdinit->qi_minfo->mi_idname
			&&  strcmp(name, fmp->d_str->st_rdinit->qi_minfo->mi_idname) == 0) {
				if (maybe) {
					fmp = 0;
					break;
				}
				maybe = fmp;
			}
			if ((fmp = fmp->d_next) == fmodsw) {
				fmp = maybe;
				break;
			}
		}
	}
	simple_unlock(&modsw_lock);
	return (fmp ? fmp->d_str : 0);
}

int
dmodsw_install (adm, str, cookie)
	struct streamadm	* adm;
	struct streamtab	* str;
	int	cookie;
{
reg	struct modsw	* dmp, * new_dmp;
	SQHP	sqh;
	int	error = 0;

	if ( !str || !adm ) {
		STR_DEBUG(printf("dmodsw_install: nil streamadm/streamtab pointer for driver '%s'\n", adm ? adm->sa_name : "???"));
		return EINVAL;
	}
	if (strlen(adm->sa_name) > FMNAMESZ) {
		STR_DEBUG(printf("dmodsw_install: name too long\n"));
		return EINVAL;
	}

	STR_MALLOC(new_dmp, struct modsw *, sizeof *dmp, M_STRMODSW, M_WAITOK);
	switch (adm->sa_sync_level) {
	case SQLVL_ELSEWHERE:
		sqh = sqe_add(adm->sa_sync_info);
		break;
	case SQLVL_MODULE:
		STR_MALLOC(sqh, SQHP, sizeof *sqh, M_STRSQ, M_WAITOK);
		break;
	default:
		sqh = 0;
		break;
	}
	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (strcmp(adm->sa_name, dmp->d_name) == 0)
			break;
		if ((dmp = dmp->d_next) == dmodsw) {
			dmp = 0;
			break;
		}
	}
	if (dmp) {
		error = EEXIST;
		goto out;
	}
	if ((dmp = new_dmp) == 0) {
		error = ENOMEM;
		goto out;
	}
	new_dmp = 0;
	bzero((caddr_t)dmp, sizeof *dmp);
	bcopy(adm->sa_name, dmp->d_name, FMNAMESZ);
	dmp->d_str = str;
	dmp->d_major = cookie;
	if ((adm->sa_flags & STR_SYSV4_OPEN) == 0)
		dmp->d_flags |= F_MODSW_OLD_OPEN;
	switch (dmp->d_sq_level = adm->sa_sync_level) {
	case SQLVL_MODULE:
		if (sqh) sqh_init(sqh);
		/* fall through */
	case SQLVL_ELSEWHERE:
		if (sqh == 0) {
			error = ENOMEM;
			goto out;
		}
		dmp->d_sqh = sqh;
		sqh = 0;
		break;
	}
	if (dmodsw)
		insque(dmp, dmodsw->d_prev);
	else
		dmodsw = dmp->d_next = dmp->d_prev = dmp;
out:
	simple_unlock(&modsw_lock);
	if (sqh) {
		switch (adm->sa_sync_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(sqh);
			break;
		case SQLVL_MODULE:
			STR_FREE(sqh, M_STRSQ);
			break;
		}
	}
	if (new_dmp)
		STR_FREE(new_dmp, M_STRMODSW);
	return error;
}

int
dmodsw_remove (name)
	char	* name;
{
reg	struct modsw	* dmp;
	int	error = 0;

	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if ( strcmp(name, dmp->d_name) == 0 ) {
			if (dmp->d_refcnt > 0) {
				error = EBUSY;
				break;
			}
			if (dmp == dmodsw) {
				dmodsw = dmp->d_next;
				if (dmp == dmodsw)
					dmodsw = 0;
			}
			remque(dmp);
			break;
		}
		if ((dmp = dmp->d_next) == dmodsw) {
			error = ENOENT;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	if (error == 0) {
		switch (dmp->d_sq_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(dmp->d_sqh);
			break;
		case SQLVL_MODULE:
			STR_FREE(dmp->d_sqh, M_STRSQ);
			break;
		}
		STR_FREE(dmp, M_STRMODSW);
	}
	return error;
}

int
fmodsw_install (adm, str)
	struct streamadm	* adm;
	struct streamtab	* str;
{
reg	struct modsw	* fmp, * new_fmp;
	SQHP	sqh;
	int	error = 0;

	if ( !str || !adm ) {
		STR_DEBUG(printf("fmodsw_install: nil streamadm/streamtab pointer for module '%s'\n", adm ? adm->sa_name : "???"));
		return EINVAL;
	}
	if (strlen(adm->sa_name) > FMNAMESZ) {
		STR_DEBUG(printf("fmodsw_install: name too long\n"));
		return EINVAL;
	}

	STR_MALLOC(new_fmp, struct modsw *, sizeof *fmp, M_STRMODSW, M_WAITOK);
	switch (adm->sa_sync_level) {
	case SQLVL_ELSEWHERE:
		sqh = sqe_add(adm->sa_sync_info);
		break;
	case SQLVL_MODULE:
		STR_MALLOC(sqh, SQHP, sizeof *sqh, M_STRSQ, M_WAITOK);
		break;
	default:
		sqh = 0;
		break;
	}
	simple_lock(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (strcmp(adm->sa_name, fmp->d_name) == 0)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	if (fmp) {
		error = EEXIST;
		goto out;
	}
	if ((fmp = new_fmp) == 0) {
		error = ENOMEM;
		goto out;
	}
	new_fmp = 0;
	bzero((caddr_t)fmp, sizeof *fmp);
	bcopy(adm->sa_name, fmp->d_name, FMNAMESZ);
	fmp->d_str = str;
	fmp->d_major = NODEV;
	if ((adm->sa_flags & STR_SYSV4_OPEN) == 0)
		fmp->d_flags |= F_MODSW_OLD_OPEN;
	switch (fmp->d_sq_level = adm->sa_sync_level) {
	case SQLVL_MODULE:
		if (sqh) sqh_init(sqh);
		/* fall through */
	case SQLVL_ELSEWHERE:
		if (sqh == 0) {
			error = ENOMEM;
			goto out;
		}
		fmp->d_sqh = sqh;
		sqh = 0;
		break;
	}
	if (fmodsw)
		insque(fmp, fmodsw->d_prev);
	else
		fmodsw = fmp->d_next = fmp->d_prev = fmp;
out:
	simple_unlock(&modsw_lock);
	if (sqh) {
		switch (adm->sa_sync_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(sqh);
			break;
		case SQLVL_MODULE:
			STR_FREE(sqh, M_STRSQ);
			break;
		}
	}
	if (new_fmp)
		STR_FREE(new_fmp, M_STRMODSW);
	return error;
}

int
fmodsw_remove (name)
	char	* name;
{
	struct modsw	* fmp;
	int	error = 0;

	simple_lock(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if ( strcmp(name, fmp->d_name) == 0 ) {
			if (fmp->d_refcnt > 0) {
				error = EBUSY;
				break;
			}
			if (fmp == fmodsw) {
				fmodsw = fmp->d_next;
				if (fmp == fmodsw)
					fmodsw = 0;
			}
			remque(fmp);
			break;
		}
		if ((fmp = fmp->d_next) == fmodsw) {
			error = ENOENT;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	if (error == 0) {
		switch (fmp->d_sq_level) {
		case SQLVL_ELSEWHERE:
			sqe_del(fmp->d_sqh);
			break;
		case SQLVL_MODULE:
			STR_FREE(fmp->d_sqh, M_STRSQ);
			break;
		}
		STR_FREE(fmp, M_STRMODSW);
	}
	return error;
}

/** Convert rqinit pointer to fmodsw name */
char *
qinfo_to_name (qi)
	struct qinit	* qi;
{
reg	struct modsw	* fmp;

	simple_lock(&modsw_lock);
	fmp = fmodsw;
	while (fmp) {
		if (fmp->d_str->st_rdinit == qi)
			break;
		if ((fmp = fmp->d_next) == fmodsw) {
			fmp = 0;
			break;
		}
	}
	simple_unlock(&modsw_lock);
	return (fmp ? fmp->d_name : 0);
}

struct streamtab *
mid_to_str (ushort mid)
{
reg	struct modsw	* dmp;
	struct streamtab	* str = 0;

	simple_lock(&modsw_lock);
	dmp = dmodsw;
	while (dmp) {
		if (dmp->d_str->st_rdinit
		&&  dmp->d_str->st_rdinit->qi_minfo->mi_idnum == mid) {
			str = dmp->d_str;
			break;
		}
		if ((dmp = dmp->d_next) == dmodsw)
			break;
	}
	if (str == 0) {
		dmp = fmodsw;
		while (dmp) {
			if (dmp->d_str->st_rdinit
			&&  dmp->d_str->st_rdinit->qi_minfo->mi_idnum == mid) {
				str = dmp->d_str;
				break;
			}
			if ((dmp = dmp->d_next) == fmodsw)
				break;
		}
	}
	simple_unlock(&modsw_lock);
	return str;
}

/*
 * Reset the synch queue subordination for a queue based on the
 * synchronization level corresponding to the specified
 * streamtab pointer.  A pointer to the new parent is returned.
 */
SQHP
sqh_set_parent (q, str)
	queue_t	*		q;
	struct streamtab *	str;
{
reg	struct modsw *		dmp;
	SQHP			sqhp;
	int			sq_lvl = 0;
	SIMPLE_LOCK_DECL

	if ( str == &sthinfo ) {
		sq_lvl = SQLVL_QUEUEPAIR;
		SIMPLE_LOCK(&q->q_qlock);
		q->q_flag &= ~(QOLD|QSAFE);
		SIMPLE_UNLOCK(&q->q_qlock);
	} else {
		/* Check for entry in fmodsw first... */
		simple_lock(&modsw_lock);
		dmp = fmodsw;
		while (dmp) {
			if (dmp->d_str == str)
				break;
			if ((dmp = dmp->d_next) == fmodsw) {
				dmp = 0;
				break;
			}
		}
		if (dmp) {
			sq_lvl = dmp->d_sq_level;
			sqhp = dmp->d_sqh;
		}
		if ( sq_lvl == 0 ) {
			/* Now try to find it in dmodsw... */
			dmp = dmodsw;
			while (dmp) {
				if (dmp->d_str == str)
					break;
				if ((dmp = dmp->d_next) == dmodsw) {
					dmp = 0;
					break;
				}
			}
			if (dmp) {
				sq_lvl = dmp->d_sq_level;
				sqhp = dmp->d_sqh;
			}
		}
		/*
		 * Set qflag(s) per modsw.
		 */
		if (dmp) {
			SIMPLE_LOCK(&q->q_qlock);
			if (dmp->d_flags & F_MODSW_OLD_OPEN)
				q->q_flag |= QOLD;
			if (dmp->d_flags & F_MODSW_QSAFETY)
				q->q_flag |= QSAFE;
			SIMPLE_UNLOCK(&q->q_qlock);
		}
		simple_unlock(&modsw_lock);
	}

	switch (sq_lvl) {
	case SQLVL_QUEUE:
		sqhp = &q->q_sqh;
		break;
	case SQLVL_QUEUEPAIR:
		sqhp = (q->q_flag & QREADR) ? &q->q_sqh : &RD(q)->q_sqh;
		break;
	case SQLVL_ELSEWHERE:
	case SQLVL_MODULE:
		break;
	case SQLVL_DEFAULT:
	case SQLVL_GLOBAL:
	default:
		sqhp = &sq_sqh;
		break;
	}
	q->q_sqh.sqh_parent = sqhp;
	return sqhp;
}

/*
 * Following lookups used by "sc" to find all modules/devices.
 */

staticf int
modsw_next (modsw, nextp, namep)
	struct modsw **modsw;
	void **	nextp;
	char **	namep;
{
	struct modsw *dmp;

	simple_lock(&modsw_lock);
	dmp = *modsw;
	/* Get back to previous "next". */
	if (*nextp) while (dmp) {
		if (dmp == (struct modsw *)(*nextp))
			break;
		if ((dmp = dmp->d_next) == *modsw) {
			dmp = 0;
			break;
		}
	}
	if (dmp) {
		*namep = dmp->d_name;
		if ((dmp = dmp->d_next) == *modsw)
			*nextp = (void *)-1;
		else
			*nextp = (void *)dmp;
	}
	simple_unlock(&modsw_lock);
	return (dmp != 0);
}

int
dmodsw_next (nextp, namep)
	void **	nextp;
	char **	namep;
{
	return modsw_next(&dmodsw, nextp, namep);
}

int
fmodsw_next (nextp, namep)
	void **	nextp;
	char **	namep;
{
	return modsw_next(&fmodsw, nextp, namep);
}
