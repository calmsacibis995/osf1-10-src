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
static char	*sccsid = "@(#)$RCSfile: method_mst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:31:08 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysconfig.h>
#include "cm.h"

typedef struct mmax_devconf	mst_adm_t;

#define	MAXMSTNAME	64
typedef struct {
	uint		mst_configured;
	char 		mst_name[MAXMSTNAME +1];
	mst_adm_t	mst_curadm;
} mst_mod_t;

/*
 *      Local BSS
 */
uint			MST_loaded;
kmod_id_t		MST_id;
char			namebuf[64 +1];

#define	MST_MAXMSTS	10
mst_mod_t 		mst_mod_info[MST_MAXMSTS];


/*
 *
 *	Name:		MST_method()
 *	Description:	MST Configuration Method
 *	Returns:	Zero 		On success.
 *			-1		On failure.
 *
 */
int
MST_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop )
{
	int		rc;
	mst_mod_t *	mst_mod;

	if (rc=MST_lookup_mst_mod(entry, &mst_mod)) {
		METHOD_LOG(LOG_ERR, rc);
		return(-1);
	}

	rc = 0;
	if (op & CM_OP_LOAD) {
		rc = MST_method_load(logp, entry, mst_mod);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_INFO, MSG_LOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = MST_method_configure(logp, entry, mst_mod);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_CONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = MST_method_unconfigure(logp, entry, mst_mod);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}

	if (op & CM_OP_UNLOAD) {
		rc = MST_method_unload(logp, entry, mst_mod);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_INFO, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_ERR, rc);
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 *
 */
MST_method_load( cm_log_t * logp, ENT_t entry, mst_mod_t * mst_mod )
{
	int	rc;

	if (MST_loaded)
		return(KMOD_LOAD_L_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_load(entry, &MST_id)) != 0)
		return(rc);
#endif
	MST_loaded = 1;
	return(0);
}


/*
 *
 */
MST_method_unload( cm_log_t * logp, ENT_t entry, mst_mod_t * mst_mod )
{
	int	rc;

	if (!MST_loaded)
		return(KMOD_UNLOAD_L_EEXIST);
	if (MST_inuse())
		return(KMOD_UNLOAD_C_EBUSY);
#ifndef TEST
	if ((rc=cm_kls_unload(MST_id)) != 0)
		return(rc);
#endif
	MST_id = LDR_NULL_MODULE;
	MST_loaded = 0;
	return(0);
}


/*
 *
 */
int
MST_method_configure( cm_log_t * logp, ENT_t entry, mst_mod_t * mst_mod)
{
	int		rc;
	mst_adm_t	mst_inadm;

	if (!MST_loaded)
		return(KMOD_CONFIG_L_EEXIST);
	if (mst_mod->mst_configured > 0)
		return(KMOD_CONFIG_C_EBUSY);

	mst_inadm.mdc_level = 0;
	mst_inadm.mdc_bmajnum = dbattr_num(entry, DEVICE_BLKMAJOR, -1);
	if (mst_inadm.mdc_bmajnum <= 0)
		return(KMOD_ENOENT);
	mst_inadm.mdc_cmajnum = dbattr_num(entry, DEVICE_CHRMAJOR, -1);
	if (mst_inadm.mdc_cmajnum <= 0)
		return(KMOD_ENOENT);
	mst_inadm.mdc_minnum = 0;
	mst_inadm.mdc_flags = IH_VEC_DYNAMIC_OK;
	mst_inadm.mdc_errcode = 0;

#ifndef TEST
       	rc = cm_kls_call(MST_id, SYSCONFIG_CONFIGURE, 
			&(mst_inadm), sizeof(mst_adm_t),
			&(mst_mod->mst_curadm), sizeof(mst_adm_t));
	if (rc != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", 
				AFentname(entry), strerror(errno));
		return(rc);
	}
#else
	bcopy(&(mst_inadm), &(mst_mod->mst_curadm), sizeof(mst_adm_t));
#endif
	mst_mod->mst_configured++;
	cm_log(logp, LOG_ERR, "%s: configured MST tape unit %s (%d/%d)\n", 
			AFentname(entry), 
			mst_mod->mst_name,
			mst_mod->mst_curadm.mdc_bmajnum,
			mst_mod->mst_curadm.mdc_cmajnum);

	MST_mknods(logp, entry, mst_mod, 1);

	return(0);
}


/*
 *
 */
int
MST_method_unconfigure( cm_log_t * logp, ENT_t entry, mst_mod_t * mst_mod)
{
	int	rc;

	if (!MST_loaded)
		return(KMOD_UNCONFIG_L_EEXIST);
	if (mst_mod->mst_configured <= 0)
		return(KMOD_UNCONFIG_C_EEXIST);

#ifndef TEST
       	rc = cm_kls_call(MST_id, SYSCONFIG_UNCONFIGURE, 
		&(mst_mod->mst_curadm), sizeof(mst_adm_t), 
		&(mst_mod->mst_curadm), sizeof(mst_adm_t)); 
	if (rc != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", 
				AFentname(entry), strerror(errno));
		return(rc);
	}
#endif
	mst_mod->mst_configured--;
	cm_log(logp, LOG_ERR, "%s: deconfigured MST tape unit %s (%d/%d)\n", 
			AFentname(entry), 
			mst_mod->mst_name,
			mst_mod->mst_curadm.mdc_bmajnum,
			mst_mod->mst_curadm.mdc_cmajnum);

	MST_mknods(logp, entry, mst_mod, 0);

	return(0);
}



int
MST_lookup_mst_mod( ENT_t entry, mst_mod_t ** p)
{
	char *	entname;
	int 	rc;
	int	i;
	int	type;

	if ((entname=AFentname(entry)) == NULL || 
			(i=strlen(entname)) > MAXMSTNAME || i < 1)
		return(KMOD_ENOENT);

	for(i=0; i < MST_MAXMSTS; i++) {
		if (mst_mod_info[i].mst_name[0] == '\0')
			continue;
		if (!strcmp(mst_mod_info[i].mst_name, entname)) {
			*p = &mst_mod_info[i];
			return(0);
		}
	}
	for(i=0; i < MST_MAXMSTS; i++) {
		if (mst_mod_info[i].mst_name[0] == '\0') {
			strncpy(mst_mod_info[i].mst_name, entname, MAXMSTNAME);
			mst_mod_info[i].mst_configured = 0;
			bzero(&(mst_mod_info[i].mst_curadm),
				(sizeof(struct mmax_devconf)));
			*p = &mst_mod_info[i];
			return(0);
		}
	}
	return(KMOD_ENOENT);
	}

MST_inuse()
{
	int	i;
	int	cnt = 0;

	for(i=0; i < MST_MAXMSTS; i++) {
		if (mst_mod_info[i].mst_name[0] != '\0'
		&&  mst_mod_info[i].mst_configured)
			cnt++;
	}
	return(cnt);
}


int
MST_mknods( cm_log_t * logp, ENT_t entry, mst_mod_t * mst_mod, int make)
{
	cm_devices_t 	devices;
	int		mknod_op;

	/*
	 *	Block devices
	 */
	devices.devfiles  = AFgetatr(entry, DEVICE_BLKFILES);
	devices.devminors = AFgetatr(entry, DEVICE_BLKMINOR);
	devices.dir 	= dbattr_string(entry, DEVICE_DIR, "/dev");
	devices.subdir	= dbattr_string(entry, DEVICE_BLKSUBDIR, NULL);
	devices.majno	= mst_mod->mst_curadm.mdc_bmajnum;
	devices.type 	= DEVTYPE_BLK;
	devices.mode 	= dbattr_mode(entry, DEVICE_MODE, DEVMODE_DFLT);
	devices.uid 	= dbattr_user(entry, DEVICE_USER, 0);
	devices.gid 	= dbattr_group(entry, DEVICE_GROUP, 0);
	if (make)
		mknod_op = 	CM_RMNOD_MAJR | CM_RMNOD_FILE |
				CM_MKNOD_FILE | CM_RPT_MKNOD | 
				CM_RPT_HEADER;
	else
		mknod_op = 	CM_RMNOD_MAJR | CM_RPT_RMNOD | 
				CM_RPT_HEADER;
	(void) cm_mknods(logp, AFentname(entry), mknod_op, &devices);

	/*
	 *	Character  devices
	 */
	devices.devfiles  = AFgetatr(entry, DEVICE_CHRFILES);
	devices.devminors = AFgetatr(entry, DEVICE_CHRMINOR);
	devices.dir 	= dbattr_string(entry, DEVICE_DIR, "/dev");
	devices.subdir	= dbattr_string(entry, DEVICE_CHRSUBDIR, NULL);
	devices.majno	= mst_mod->mst_curadm.mdc_cmajnum;
	devices.type 	= DEVTYPE_CHR;
	devices.mode 	= dbattr_mode(entry, DEVICE_MODE, DEVMODE_DFLT);
	devices.uid 	= dbattr_user(entry, DEVICE_USER, 0);
	devices.gid 	= dbattr_group(entry, DEVICE_GROUP, 0);
	if (make)
		mknod_op = 	CM_RMNOD_MAJR | CM_RMNOD_FILE |
				CM_MKNOD_FILE | CM_RPT_MKNOD;
	else
		mknod_op = 	CM_RMNOD_MAJR | CM_RPT_RMNOD;
	(void) cm_mknods(logp, AFentname(entry), mknod_op, &devices);

	return(0);
}

