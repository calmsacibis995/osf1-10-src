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
static char	*sccsid = "@(#)$RCSfile: method_streams.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:03:58 $";
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
#include <sys/stream.h>

#include "cm.h"

/*
 *      Local BSS
 */
typedef struct {
	char 		str_name[FMNAMESZ +1];
	uint		str_loaded;
	uint		str_configured;
	kmod_id_t	str_id;
	str_config_t	str_outadm;
} str_mod_t;

#define	MAXSTREAMS	20
str_mod_t	str_mod_list[MAXSTREAMS];


/*
 *
 *	Name:		STREAMS_method()
 *	Description:	Generic OSF/1 STREAMS Configuration Method
 *	Returns:	Zero 		On success.
 *			Non-zero	On failure.
 *
 */
int
STREAMS_method( cm_log_t * logp, ENT_t entry, cm_op_t op, cm_op_t * rop,
	char *opts )
{
	int		rc;
	str_mod_t *	str_mod;

	if (rc=STREAMS_lookup_str_mod(entry, &str_mod)) {
		METHOD_LOG(LOG_ERR, rc);
		return(-1);
	}

	rc = 0;
	if (op & CM_OP_LOAD) {
		rc = STREAMS_method_load(logp, entry, str_mod);
		if (rc == 0) {
			*rop = CM_OP_LOAD;
			METHOD_LOG(LOG_INFO, MSG_LOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_CONFIGURE) {
		rc = STREAMS_method_configure(logp, entry, str_mod);
		if (rc == 0) {
			*rop = CM_OP_CONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_CONFIGURED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_UNCONFIGURE) {
		rc = STREAMS_method_unconfigure(logp, entry, str_mod);
		if (rc == 0) {
			*rop = CM_OP_UNCONFIGURE;
			METHOD_LOG(LOG_INFO, MSG_UNCONFIGURED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}

	if (op & CM_OP_UNLOAD) {
		rc = STREAMS_method_unload(logp, entry, str_mod);
		if (rc == 0) {
			*rop = CM_OP_UNLOAD;
			METHOD_LOG(LOG_INFO, MSG_UNLOADED);
		} else {
			METHOD_LOG(LOG_INFO, rc);
		}
	}
	return(rc == 0 ? 0 : -1);
}


/*
 *
 */
int
STREAMS_method_load( cm_log_t * logp, ENT_t entry, str_mod_t * str_mod )
{
	int	rc;

	if (str_mod->str_loaded)
		return(KMOD_LOAD_L_EBUSY);
	if ((rc=cm_kls_load(entry, &str_mod->str_id)) != 0)
		return(rc);
	str_mod->str_loaded = 1;
	return(0);
}


/*
 *
 */
int
STREAMS_method_unload( cm_log_t * logp, ENT_t entry, str_mod_t * str_mod )
{
	int	rc;

	if (!str_mod->str_loaded)
		return(KMOD_UNLOAD_L_EEXIST);
	if (str_mod->str_configured)
		return(KMOD_UNLOAD_C_EBUSY);
	if ((rc=cm_kls_unload(str_mod->str_id)) != 0)
		return(rc);
	str_mod->str_id = LDR_NULL_MODULE;
	str_mod->str_loaded = 0;
	return(0);
}


/*
 *
 */
int
STREAMS_method_configure( cm_log_t * logp, ENT_t entry, str_mod_t * str_mod )
{
	int	rc;

	if (!str_mod->str_loaded)
		return(KMOD_CONFIG_L_EEXIST);
	if (str_mod->str_configured)
		return(KMOD_CONFIG_C_EBUSY);
	if ((rc=cm_kls_call(str_mod->str_id, SYSCONFIG_CONFIGURE,
		NULL, 0, &(str_mod->str_outadm), sizeof(str_config_t))) != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		return(rc);
	}

	STREAMS_prtcfg(logp, entry, str_mod, "configured");
	STREAMS_mknods(logp, entry, str_mod, 1);
	str_mod->str_configured = 1;
	return(0);
}


/*
 *
 */
int
STREAMS_method_unconfigure( cm_log_t * logp, ENT_t entry, str_mod_t * str_mod )
{
	int	rc;

	if (!str_mod->str_loaded)
		return(KMOD_UNCONFIG_L_EEXIST);
	if (!str_mod->str_configured)
		return(KMOD_UNCONFIG_C_EEXIST);
	if ((rc=cm_kls_call(str_mod->str_id, SYSCONFIG_UNCONFIGURE,
		NULL, 0, NULL, 0)) != 0) {
		if (rc == KLDR_EFAIL)
			cm_log(logp, LOG_ERR, "%s: %s\n", AFentname(entry),
				strerror(errno));
		return(rc);
	}

	STREAMS_prtcfg(logp, entry, str_mod, "deconfigured");
	STREAMS_mknods(logp, entry, str_mod, 0);
	str_mod->str_configured = 0;
	return(0);
}


int
STREAMS_prtcfg(cm_log_t * logp, ENT_t entry, str_mod_t * str_mod, char * string)
{
	if (str_mod->str_outadm.sc_version == OSF_STREAMS_CONFIG_10) {
		switch (str_mod->str_outadm.sc_sa_flags) {
		case STR_IS_DEVICE:
			cm_log(logp, LOG_ERR,
				"%s: %s STREAMS \"%s\" device (%d/%d)\n",
				AFentname(entry),
				string,
				str_mod->str_outadm.sc_sa_name,
				major(str_mod->str_outadm.sc_devnum),
				minor(str_mod->str_outadm.sc_devnum));
			break;
		case STR_IS_MODULE:
			cm_log(logp, LOG_ERR,
				"%s: %s STREAMS \"%s\" module\n",
				AFentname(entry),
				string,
				str_mod->str_outadm.sc_sa_name);
			break;
		}
	} else
		cm_log(logp, LOG_ERR, "%s: %s STREAMS \"%s\" module\n",
			AFentname(entry), string, str_mod->str_name);
}


int
STREAMS_lookup_str_mod( ENT_t entry, str_mod_t ** p)
{
	char *	entname;
	int 	rc;
	int	i;

	if ((entname=AFentname(entry)) == NULL
		|| (i=strlen(entname)) > FMNAMESZ || i < 1)
		return(KMOD_ENOENT);


	for(i=0; i < MAXSTREAMS; i++) {
		if (str_mod_list[i].str_name[0] == '\0')
			continue;
		if (!strcmp(str_mod_list[i].str_name, entname)) {
			*p = &str_mod_list[i];
			return(0);
		}
	}
	for(i=0; i < MAXSTREAMS; i++) {
		if (str_mod_list[i].str_name[0] == '\0') {
			strncpy(str_mod_list[i].str_name, entname, FMNAMESZ);
			str_mod_list[i].str_loaded = 0;
			str_mod_list[i].str_configured = 0;
			str_mod_list[i].str_id = LDR_NULL_MODULE;
			bzero((char *)&str_mod_list[i].str_outadm,
				(sizeof(str_config_t)));
			*p = &str_mod_list[i];
			return(0);
		}

	}
	return(KMOD_ENOMEM);
}


int
STREAMS_mknods( cm_log_t * logp, ENT_t entry, str_mod_t * str_mod, int make)
{
	cm_devices_t 	devices;
	struct ATTR	files_attr;
	struct ATTR	minors_attr;
	char		files_buf[32];
	char		minors_buf[32];
	char *		unit_name;
	int		str_major;
	int		str_minor;
	int		clone_major;
	int		clone_minor;
	int		unit_major;
	int		unit_num;
	int		rc;
	int		mknod_op;

	if (str_mod->str_outadm.sc_version != OSF_STREAMS_CONFIG_10)
		return(-1);

	if ( !(str_mod->str_outadm.sc_sa_flags & STR_IS_DEVICE)
		|| str_mod->str_outadm.sc_devnum == NODEV)
		return(-2);

	unit_name = dbattr_string(entry,STREAMS_NAME, NULL);
	if (unit_name == NULL)
		return(-1);
	devices.dir = dbattr_string(entry, DEVICE_DIR, "/dev");
	devices.subdir = dbattr_string(entry, DEVICE_SUBDIR, NULL);
	devices.mode = dbattr_mode(entry, DEVICE_MODE, DEVMODE_DFLT);
	devices.type = DEVTYPE_CHR;
	devices.uid = dbattr_user(entry, DEVICE_USER, 0);
	devices.gid = dbattr_group(entry, DEVICE_GROUP, 0);
	unit_num = dbattr_num(entry,STREAMS_UNITS,0);

	str_major = major(str_mod->str_outadm.sc_devnum);
	str_minor = minor(str_mod->str_outadm.sc_devnum);
	if (str_minor > 0) {
				/* Driver supports BOTH Clone & Unit devices */
		clone_major = str_major;
		clone_minor = str_minor;
		if (unit_num > 0)
			unit_major = str_minor;
		else
			unit_major = 0;
	} else {
				 /* Driver supports ONLY Unit devices */
		clone_major = 0;
		clone_minor = 0;
		if (unit_num > 0)
			unit_major = str_major;
		else
			unit_major = 0;
	}

	if (clone_major > 0) {
		sprintf(files_buf,"%s", unit_name);
		sprintf(minors_buf,"%d", clone_minor);
		STREAMS_mkatr(&files_attr, DEVICE_CHRFILES, files_buf);
		STREAMS_mkatr(&minors_attr, DEVICE_CHRMINOR, minors_buf);
		devices.majno = clone_major;
		devices.devfiles = &files_attr;
		devices.devminors = &minors_attr;
		if (make)
			mknod_op = CM_RMNOD_FILE | CM_MKNOD_FILE | CM_RPT_MKNOD;
		else
			mknod_op = CM_RMNOD_FILE | CM_RPT_RMNOD;
		mknod_op |= CM_RPT_HEADER;
		rc = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}

	if (unit_num > 0) {
		if (unit_num == 1) {
			sprintf(files_buf,"%s0", unit_name);
			sprintf(minors_buf,"0");
		} else {
			sprintf(files_buf,"%s[0-%d]", unit_name, unit_num-1);
			sprintf(minors_buf,"[0-%d]", unit_num-1);
		}
		STREAMS_mkatr(&files_attr, DEVICE_CHRFILES, files_buf);
		STREAMS_mkatr(&minors_attr, DEVICE_CHRMINOR, minors_buf);
		devices.majno = unit_major;
		devices.devfiles = &files_attr;
		devices.devminors = &minors_attr;
		if (make)
			mknod_op = CM_RMNOD_MAJR | CM_RMNOD_FILE 
					| CM_MKNOD_FILE | CM_RPT_MKNOD;
		else
			mknod_op = CM_RMNOD_MAJR | CM_RPT_RMNOD;
		if (clone_major == 0)
			mknod_op |= CM_RPT_HEADER;
		rc = cm_mknods(logp, AFentname(entry), mknod_op, &devices);
	}

	return(0);
}


int
STREAMS_mkatr( ATTR_t attr, char * name, char * value )
{
	if (attr == NULL || name == NULL || value == NULL)
		return(-1);

	attr->AT_nvalue = NULL;
	attr->AT_name = name;
	attr->AT_value = value;
	while (*value++ != '\0') 		/* Doubly terminate value */
		;
	*value = '\0';
	return(0);
}
