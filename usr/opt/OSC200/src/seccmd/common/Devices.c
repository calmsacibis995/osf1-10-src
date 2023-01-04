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
static char	*sccsid = "@(#)$RCSfile: Devices.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:20 $";
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
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/* support routines for the Devices screens */


#include <sys/secdefines.h>

#if SEC_BASE

#include "IfDevices.h"

/* error messages */

static char 
	**msg_devices_error_1,
	**msg_devices_error_2,
	**msg_devices_error_3,
	**msg_devices_error_4,
	*msg_devices_error_1_text,
	*msg_devices_error_2_text,
	*msg_devices_error_3_text,
	*msg_devices_error_4_text;

/* static routines in this file */

static int IsDeviceAlreadyListed();
static int IsSpecialDevice();
extern char **alloc_cw_table();

/*
 * retrieve device fillin struct
 * returns 0 on success or 1 on failure
 */

int
DevGetFillin(device, df)
	char *device;
	DeviceFillin *df;
{
	if (XGetDeviceInfo(device, &df->dev) != SUCCESS)
		return 1;

	/* device name */

	strcpy(df->device_name, df->dev.dev.ufld.fd_name);

	/* device type */

	if (!df->dev.dev.uflg.fg_type)
		df->device_type = UnknownDevice;
	else if (ISBITSET(df->dev.dev.ufld.fd_type, AUTH_DEV_PRINTER))
		df->device_type = PrinterDevice;
	else if (ISBITSET(df->dev.dev.ufld.fd_type, AUTH_DEV_TERMINAL))
		df->device_type = TerminalDevice;
	else if (ISBITSET(df->dev.dev.ufld.fd_type, AUTH_DEV_TAPE))
		df->device_type = RemovableDevice;
#ifdef SEC_NET_TTY
	else if (ISBITSET(df->dev.dev.ufld.fd_type, AUTH_DEV_REMOTE))
		df->device_type = HostDevice;
#endif

	/* device list */

	if (!df->dev.dev.uflg.fg_devs)
		df->devices = (char **) 0;
	else {
		int i;

		for (i = 0; df->dev.dev.ufld.fd_devs[i]; i++)
			;
		df->ndevices = i;
		df->devices = df->dev.dev.ufld.fd_devs;
	}

	/* Authorized user list */

	if (!df->dev.dev.uflg.fg_users)
		df->auth_users = (char **) 0;
	else {
		int i;

		for (i = 0; df->dev.dev.ufld.fd_users[i]; i++)
			;
		df->nauth_users = i;
		df->auth_users = df->dev.dev.ufld.fd_users;
	}

	/* enabled for import */

	if (df->dev.dev.uflg.fg_assign &&
	    ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_IMPORT))
			df->import_enable = 1;
	else
			df->import_enable = 0;

	/* enabled for export */

	if (df->dev.dev.uflg.fg_assign &&
	    ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_EXPORT))
		df->export_enable = 1;
	else
		df->export_enable = 0;


	/* following fields are for terminals only */

	if (df->device_type == TerminalDevice
#if SEC_NET_TTY
	 || df->device_type == HostDevice
#endif
	   ) {

		/* administrative lock status */

		if (df->dev.tm.uflg.fg_lock && df->dev.tm.ufld.fd_lock)
			df->locked = 1;
		else
			df->locked = 0;

		/* max user logins */

		if (df->dev.tm.sflg.fg_max_tries)
			df->def_max_ulogins_value =
				df->dev.tm.sfld.fd_max_tries;
		else
			df->def_max_ulogins_value = 0;

		if (!df->dev.tm.uflg.fg_max_tries) {
			df->def_max_ulogins = 1;
			df->max_ulogins = df->def_max_ulogins_value;
		} else {
			df->def_max_ulogins = 0;
			df->max_ulogins = df->dev.tm.ufld.fd_max_tries;
		}

		/* login timeout */

		if (df->dev.tm.sflg.fg_login_timeout)
			df->def_login_timeout_value =
			  df->dev.tm.sfld.fd_login_timeout;
		else
			df->def_login_timeout_value = 0;

		if (!df->dev.tm.uflg.fg_login_timeout) {
			df->def_login_timeout = 1;
			df->login_timeout = df->def_login_timeout_value;
		} else {
			df->def_login_timeout = 0;
			df->login_timeout = df->dev.tm.ufld.fd_login_timeout;
		}

		/* login delay */

		if (df->dev.tm.uflg.fg_logdelay)
			df->def_login_delay_value =
			  df->dev.tm.sfld.fd_logdelay;
		else
			df->def_login_delay_value = 0;

		if (!df->dev.tm.uflg.fg_logdelay) {
			df->def_login_delay = 1;
			df->login_delay = df->def_login_delay_value;
		} else {
			df->def_login_delay = 0;
			df->login_delay = df->dev.tm.ufld.fd_logdelay;
		}
	}

#if SEC_MAC
	/* allocate sensitivity label field memory */

	df->ml_min_sl = mand_alloc_ir();
	df->ml_max_sl = mand_alloc_ir();
	df->sl_sl = mand_alloc_ir();
	if (df->ml_min_sl == (mand_ir_t *) 0 ||
	    df->ml_max_sl == (mand_ir_t *) 0 ||
	    df->sl_sl == (mand_ir_t *) 0)
		MemoryError();
#if SEC_ILB
	df->sl_il = ilb_alloc_ir();
	if (df->sl_il == (ilb_ir_t *) 0)
		MemoryError();
#endif

	/* multilevel min sl */

	if (!df->dev.dev.uflg.fg_min_sl)
		df->ml_min_sl_set = 0;
	else {
		df->ml_min_sl_set = 1;
		mand_copy_ir(df->dev.dev.ufld.fd_min_sl, df->ml_min_sl);
	}

	/* multilevel max sl */

	if (!df->dev.dev.uflg.fg_max_sl)
		df->ml_max_sl_set = 0;
	else {
		df->ml_max_sl_set = 1;
		mand_copy_ir(df->dev.dev.ufld.fd_max_sl, df->ml_max_sl);
	}

	/* single-level sl */

	if (!df->dev.dev.uflg.fg_cur_sl)
		df->sl_sl_set = 0;
	else {
		df->sl_sl_set = 1;
		mand_copy_ir(df->dev.dev.ufld.fd_cur_sl, df->sl_sl);
	}

	/* sensitivity label assignment */

	if (ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_SINGLE))
		df->asg_sl_sl = 1;
	else
		df->asg_sl_sl = 0;

	if (ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_MULTI))
		df->asg_ml_sl = 1;
	else
		df->asg_ml_sl = 0;

#if SEC_ILB

	/* single-level IL */

	if (!df->dev.dev.uflg.fg_cur_il)
		df->sl_il_set = 0;
	else {
		df->sl_il_set = 1;
		ilb_copy_ir(df->dev.dev.ufld.fd_cur_il, df->sl_il);
	}

	if (ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILSINGLE)
		df->asg_sl_il = 1;
	else
		df->asg_sl_il = 0;

	if (ISBITSET(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILMULTI)
		df->asg_ml_il = 1;
	else
		df->asg_ml_il = 0;

#endif /* SEC_ILB */
#endif /* SEC_MAC */

	return 0;
}

/*
 * initialize device fillin struct for a new device
 * returns 0 on success or 1 on failure
 */

int
DevCreateFillin(device_type, df)
	enum device_type device_type;
	DeviceFillin *df;
{
	/* device name */

	memset(df->device_name, '\0', sizeof(df->device_name));

	/* device type */

	switch(device_type) {
	case PrinterDevice:
		df->device_type = PrinterDevice;
		ADDBIT(df->dev.dev.ufld.fd_type, AUTH_DEV_PRINTER);
		break;
	case TerminalDevice:
		df->device_type = TerminalDevice;
		ADDBIT(df->dev.dev.ufld.fd_type, AUTH_DEV_TERMINAL);
		break;
	case RemovableDevice:
		df->device_type = RemovableDevice;
		ADDBIT(df->dev.dev.ufld.fd_type, AUTH_DEV_TAPE);
		break;
#ifdef SEC_NET_TTY
	case HostDevice:
		df->device_type = HostDevice;
		ADDBIT(df->dev.dev.ufld.fd_type, AUTH_DEV_REMOTE);
		break;
#endif
	}

	/* device list */

	df->devices = (char **) 0;
	df->ndevices = 0;

	/* Authorized user list */

	df->auth_users = (char **) 0;
	df->nauth_users = 0;

	/* enabled for import */

	df->import_enable = 0;

	/* enabled for export */

	df->export_enable = 0;

	/* following fields are for terminals only */

	if (df->device_type == TerminalDevice
#if SEC_NET_TTY
	 || df->device_type == HostDevice
#endif
	   ) {

		struct pr_default *pr = getprdfnam("default");

		/* administrative lock status */

		df->locked = 0;

		/* max user logins */

		df->def_max_ulogins = 1;
		if (pr == (struct pr_default *) 0 || !pr->tcg.fg_max_tries)
			df->def_max_ulogins_value = 0;
		else
			df->def_max_ulogins_value = pr->tcd.fd_max_tries;
		df->max_ulogins = df->def_max_ulogins_value;

		/* login timeout */

		df->def_login_timeout = 1;
		if (pr == (struct pr_default *) 0 || !pr->tcg.fg_login_timeout)
			df->def_login_timeout_value = 0;
		else
			df->def_login_timeout_value = pr->tcd.fd_login_timeout;
		df->login_timeout = df->def_login_timeout_value;

		/* login delay */

		df->def_login_delay = 1;
		if (pr == (struct pr_default *) 0 || !pr->tcg.fg_logdelay)
			df->def_login_delay_value = 0;
		else
			df->def_login_delay_value = pr->tcd.fd_logdelay;
		df->login_delay = df->def_login_delay_value;
	}

#if SEC_MAC
	/* allocate sensitivity label field memory */

	df->ml_min_sl = mand_alloc_ir();
	df->ml_max_sl = mand_alloc_ir();
	df->sl_sl = mand_alloc_ir();
	if (df->ml_min_sl == (mand_ir_t *) 0 ||
	    df->ml_max_sl == (mand_ir_t *) 0 ||
	    df->sl_sl == (mand_ir_t *) 0)
		MemoryError();
#if SEC_ILB
	df->sl_il = ilb_alloc_ir();
	if (df->sl_il == (ilb_ir_t *) 0)
		MemoryError();
#endif

	/* multilevel min sl */

	df->ml_min_sl_set = 0;

	/* multilevel max sl */

	df->ml_max_sl_set = 0;

	/* single-level sl */

	df->sl_sl_set = 0;

	/* sensitivity label assignment */

	df->asg_sl_sl = 1;

	df->asg_ml_sl = 0;

#if SEC_ILB

	/* single-level IL */

	df->sl_il_set = 0;

	df->asg_sl_il = 1;

	df->asg_ml_il = 0;

#endif /* SEC_ILB */
#endif /* SEC_MAC */

	return 0;
}

/*
 * free memory allocated by DevGetFillin
 */

void
DevFreeFillin(df)
	DeviceFillin *df;
{
#if SEC_MAC
	if (df->ml_min_sl) {
		mand_free_ir(df->ml_min_sl);
		df->ml_min_sl = (mand_ir_t *) 0;
	}
	if (df->ml_max_sl) {
		mand_free_ir(df->ml_max_sl);
		df->ml_max_sl = (mand_ir_t *) 0;
	}
	if (df->sl_sl) {
		mand_free_ir(df->sl_sl);
		df->sl_sl = (mand_ir_t *) 0;
	}
#if SEC_ILB
	if (df->sl_il) {
		ilb_free_ir(df->sl_il);
		df->sl_il = (ilb_ir_t *) 0;
	}
#endif /* SEC_ILB */
#endif /* SEC_MAC */
}

/* retrieve sys default fillin */

int
DevGetDefFillin(ddf)
	DefDeviceFillin	*ddf;
{
	if (XGetSystemInfo(&ddf->sd) != SUCCESS)
		return 1;

	/* authorized user list */

	if (!ddf->sd.df.devg.fg_users) {
		ddf->auth_users = (char **) 0;
		ddf->nauth_users = 0;
	} else {
		int i;

		for (i = 0; ddf->sd.df.devd.fd_users[i]; i++)
			;
		ddf->nauth_users = i;
		ddf->auth_users = ddf->sd.df.devd.fd_users;
	}

	/* administrative lock */

	if (ddf->sd.df.tcg.fg_lock && ddf->sd.df.tcd.fd_lock)
		ddf->locked = 1;
	else
		ddf->locked = 0;

	/* max unsuccessul logins */

	if (ddf->sd.df.tcg.fg_max_tries)
		ddf->max_ulogins = ddf->sd.df.tcd.fd_max_tries;
	else
		ddf->max_ulogins = 0;

	/* login timeout */

	if (ddf->sd.df.tcg.fg_login_timeout)
		ddf->login_timeout = ddf->sd.df.tcd.fd_login_timeout;
	else
		ddf->login_timeout = 0;

	/* login delay */

	if (ddf->sd.df.tcg.fg_logdelay)
		ddf->login_delay = ddf->sd.df.tcd.fd_logdelay;
	else
		ddf->login_delay = 0;

#ifdef SEC_MAC
	ddf->ml_min_sl = mand_alloc_ir();
	ddf->ml_max_sl = mand_alloc_ir();
	ddf->sl_sl = mand_alloc_ir();
	if (ddf->ml_min_sl == (mand_ir_t *) 0 ||
	    ddf->ml_max_sl == (mand_ir_t *) 0 ||
	    ddf->sl_sl == (mand_ir_t *) 0)
		MemoryError();

	/* multi-level min SL */

	if (ddf->sd.df.devg.fg_min_sl)
		mand_copy_ir(ddf->sd.df.devd.fd_min_sl, ddf->ml_min_sl);
	else
#if SEC_ENCODINGS
		mand_copy_ir(mand_minsl, ddf->ml_min_sl);
#else
		mand_copy_ir(mand_syslo, ddf->ml_min_sl);
#endif

	/* multi-level max SL */

	if (ddf->sd.df.devg.fg_max_sl)
		mand_copy_ir(ddf->sd.df.devd.fd_max_sl, ddf->ml_max_sl);
	else
		mand_copy_ir(mand_syshi, ddf->ml_max_sl);

	/* single-level SL */

	if (ddf->sd.df.devg.fg_cur_sl)
		mand_copy_ir(ddf->sd.df.devd.fd_cur_sl, ddf->sl_sl);
	else
#if SEC_ENCODINGS
		mand_copy_ir(mand_minsl, ddf->sl_sl);
#else
		mand_copy_ir(mand_syslo, ddf->sl_sl);
#endif

#if SEC_ILB
	/* single-level IL */

	if (ddf->sd.df.devg.fg_cur_il)
		ilb_copy_ir(ddf->sd.devd.fd_cur_il, ddf->sl_il);
	else
		ilb_copy_ir(mand_syslo, ddf->sl_il);
#endif /* SEC_ILB */

#endif /* SEC_MAC */

	return 0;
}

/*
 * free memory allocated by DevGetDefFillin
 */

void
DevFreeDefFillin(ddf)
	DefDeviceFillin *ddf;
{
#if SEC_MAC
	if (ddf->ml_min_sl) {
		mand_free_ir(ddf->ml_min_sl);
		ddf->ml_min_sl = (mand_ir_t *) 0;
	}
	if (ddf->ml_max_sl) {
		mand_free_ir(ddf->ml_max_sl);
		ddf->ml_max_sl = (mand_ir_t *) 0;
	}
	if (ddf->sl_sl) {
		mand_free_ir(ddf->sl_sl);
		ddf->sl_sl = (mand_ir_t *) 0;
	}
#if SEC_ILB
	if (ddf->sl_il) {
		ilb_free_ir(ddf->sl_il);
		ddf->sl_il = (ilb_ir_t *) 0;
	}
#endif /* SEC_ILB */
#endif /* SEC_MAC */
}

/* update authorized user list */

int
DevChangeUserList(df)
	DeviceFillin *df;
{
	char **user_list;
	int i;
	int ret;

	if (df->nauth_users == 0) {
		user_list = (char **) 0;
		df->dev.dev.uflg.fg_users = 0;
	} else {
		/* make null-terminated list */

		user_list = (char **) Calloc(df->nauth_users+1, sizeof(char *));
		if (user_list == (char **) 0)
			MemoryError();
		for (i = 0; i < df->nauth_users; i++)
			user_list[i] = df->auth_users[i];
		df->dev.dev.uflg.fg_users = 1;
		df->dev.dev.ufld.fd_users = user_list;
	}
	if (XWriteDeviceInfo(&df->dev) != SUCCESS) {
		audit_subsystem(df->device_name, 
			"Unsuccessful update of authorized user list",
			ET_SYS_ADMIN);
		ret = 1;
	} else {
		audit_subsystem(df->device_name, 
			"Successful update of authorized user list",
			ET_SYS_ADMIN);
		ret = 0;
	}

	if (user_list != (char **) 0)	
		free((char *) user_list);
		
	return ret;
}

/* update control parameters */
int
DevChangeControlParams(df)
	DeviceFillin *df;
{
	char **device_list;
	int i;
	int retterm = 0, retdev = 0;

	/* device list */

	device_list = (char **) Calloc(df->ndevices+1, sizeof(char *));
	if (device_list == (char **) 0)
		MemoryError();

	for (i = 0; i < df->ndevices; i++)
		device_list[i] = df->devices[i];

	df->dev.dev.uflg.fg_devs = 1;
	df->dev.dev.ufld.fd_devs = device_list;

	/* login parameters -- terminals only */

	if (df->device_type == TerminalDevice
#if SEC_NET_TTY
	 || df->device_type == HostDevice
#endif
	   ) {
		/* maximum user logins */

		if (df->def_max_ulogins) {
			df->dev.tm.uflg.fg_max_tries = 0;
		} else {
			df->dev.tm.uflg.fg_max_tries = 1;
			df->dev.tm.ufld.fd_max_tries = df->max_ulogins;
		}

		/* login timeout */

		if (df->def_login_timeout)
			df->dev.tm.uflg.fg_login_timeout = 0;
		else {
			df->dev.tm.uflg.fg_login_timeout = 1;
			df->dev.tm.ufld.fd_login_timeout = df->login_timeout;
		}

		/* login delay */

		if (df->def_login_delay)
			df->dev.tm.uflg.fg_logdelay = 0;
		else {
			df->dev.tm.uflg.fg_logdelay = 1;
			df->dev.tm.ufld.fd_logdelay = df->login_delay;
		}
		if (XWriteTerminalInfo(&df->dev) != SUCCESS) {
			audit_subsystem(df->device_name, 
"Unsuccessful update of control parameters in Terminal Control Database",
			  ET_SYS_ADMIN);
			retterm = 1;
		} else {
			audit_subsystem(df->device_name, 
			  "Successful update of control parameters",
			  ET_SYS_ADMIN);
			retterm = 0;
		}
	}

	/* enable for import/export -- removables only */

	if (df->device_type == RemovableDevice) {
		df->dev.dev.uflg.fg_assign = 1;
		if (df->import_enable)
			ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_IMPORT);
		else
			RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_IMPORT);
		if (df->export_enable)
			ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_EXPORT);
		else
			RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_EXPORT);
	}
#if SEC_MAC

	/* single-level, multilevel assignment, Sensitivity Labels */

	df->dev.dev.uflg.fg_assign = 1;
	if (df->asg_sl_sl) {
		ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_SINGLE);
		RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_MULTI);
	} else {
		RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_SINGLE);
		ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_MULTI);
	}

#if SEC_ILB

	/* single-level, multilevel assignment, Information Labels */

	if (df->asg_sl_il) {
		ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILSINGLE);
		RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILMULTI);
	} else {
		RMBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILSINGLE);
		ADDBIT(df->dev.dev.ufld.fd_assign, AUTH_DEV_ILMULTI);
	}

#endif /* SEC_ILB */
#endif /* SEC_MAC */

	if (XWriteDeviceInfo(&df->dev) != SUCCESS) {
		audit_subsystem(df->device_name, 
			"Unsuccessful update of control parameters",
			ET_SYS_ADMIN);
		retdev = 1;
	} else {
		audit_subsystem(df->device_name, 
			"Successful update of control parameters",
			ET_SYS_ADMIN);
		retdev = 0;
	}

	if (device_list != (char **) 0)
		free((char *) device_list);

	return retterm + retdev;
}

/* validate default control parameters */

int
DevValidateDefControlParams(ddf)
	DefDeviceFillin *ddf;
{
	int nerrs = 0;
	static char **msg_error, *msg_error_text;

	if (!(ddf->max_ulogins >= 0 && ddf->max_ulogins <= 999))
		nerrs++;

	if (!(ddf->login_timeout >= 0 && ddf->login_timeout <= 999))
		nerrs++;

	if (!(ddf->login_delay >= 0 && ddf->login_delay <= 999))
		nerrs++;

	if (nerrs) {
		if (! msg_error) 
			LoadMessage ("msg_devices_default_login_error",
				&msg_error, &msg_error_text);
		ErrorMessageOpen(-1, msg_error, 1, NULL);
		return 1;
	}
	return 0;
}

/* change default control parameters */

int
DevChangeDefControlParams(ddf)
	DefDeviceFillin	*ddf;
{
	int ret;

	ddf->sd.df.tcg.fg_max_tries = 1;
	ddf->sd.df.tcd.fd_max_tries = ddf->max_ulogins;
	ddf->sd.df.tcg.fg_login_timeout = 1;
	ddf->sd.df.tcd.fd_login_timeout = ddf->login_timeout;
	ddf->sd.df.tcg.fg_logdelay = 1;
	ddf->sd.df.tcd.fd_logdelay = ddf->login_delay;
	ddf->sd.df.tcg.fg_lock = 1;
	ddf->sd.df.tcd.fd_lock = ddf->locked;

	if (XWriteSystemInfo(&ddf->sd) == SUCCESS) {
		audit_subsystem("default database",
		  "Successful update of default device control parameters",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem("default database",
		  "Unsuccessful update of default device control parameters",
		  ET_SYS_ADMIN);
		ret = 1;
	}
	return ret;
}

/* update default user list */

int
DevChangeDefUserList(ddf)
	DefDeviceFillin	*ddf;
{
	char **user_list;
	int i;
	int ret;

	user_list = (char **) Calloc(ddf->nauth_users+1, sizeof(char *));
	if (user_list == (char **) 0)
		MemoryError();

	for (i = 0; i < ddf->nauth_users; i++)
		strcpy(user_list[i], ddf->auth_users[i]);

	ddf->sd.df.devg.fg_users = 1;
	ddf->sd.df.devd.fd_users = user_list;

	if (XWriteSystemInfo(&ddf->sd) == SUCCESS) {
		audit_subsystem("default database",
			"Successful update of default authorized user list",
			ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem("default database",
			"Unsuccessful update of default authorized user list",
			ET_SYS_ADMIN);
		ret = 1;
	}

	free((char *) user_list);

	return ret;
}

/* lock device */

int
DevLock(df)
	DeviceFillin *df;
{
	int ret;

	df->dev.tm.uflg.fg_lock = 1;
	df->dev.tm.ufld.fd_lock = 1;

	if (XWriteTerminalInfo(&df->dev) == SUCCESS) {
		sa_audit_lock(ES_SET_TERM_LOCK, df->device_name);
		ret = 0;
	} else {
		audit_subsystem(df->device_name,
		  "Unsuccessful lock of terminal in Terminal Control database",
		  ET_SYS_ADMIN);
		ret = 1;
	}
	return ret;
}

/* unlock device */

int
DevUnlock(df)
	DeviceFillin *df;
{
	int ret;
	int max_tries;

	df->dev.tm.uflg.fg_lock = 1;
	df->dev.tm.ufld.fd_lock = 0;

	/* if locked for too many unsuccessful logins, reset that value */

	if (df->dev.tm.uflg.fg_max_tries)
		max_tries = df->dev.tm.ufld.fd_max_tries;
	else if (df->dev.tm.sflg.fg_max_tries)
		max_tries = df->dev.tm.sfld.fd_max_tries;
	else
		max_tries = 0;

	if (df->dev.tm.uflg.fg_nlogins)
		if (df->dev.tm.ufld.fd_nlogins > max_tries) {
			df->dev.tm.uflg.fg_nlogins = 1;
			df->dev.tm.ufld.fd_nlogins = 0;
		}

	if (XWriteTerminalInfo(&df->dev) == SUCCESS) {
		sa_audit_lock(ES_SET_TERM_UNLOCK, df->device_name);
		ret = 0;
	} else {
		audit_subsystem(df->device_name,
		 "Unsuccessful unlock of terminal in Terminal Control database",
		  ET_SYS_ADMIN);
		ret = 1;
	}
	return ret;
}

#if SEC_MAC

/* change multilevel min SL */

int
DevChangeMLMinSL(df)
	DeviceFillin *df;
{
	int ret;

	if (df->ml_min_sl_set) {
		df->dev.dev.uflg.fg_min_sl = 1;
		df->dev.dev.ufld.fd_min_sl = df->ml_min_sl;
	} else
		df->dev.dev.uflg.fg_min_sl = 0;

	if (XWriteDeviceInfo(&df->dev) == SUCCESS) {
		audit_subsystem(df->device_name,
	"Successful update of multilevel minimum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem(df->device_name,
	"Unuccessful update of multilevel minimum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

/* change multilevel max SL */

int
DevChangeMLMaxSL(df)
	DeviceFillin *df;
{
	int ret;

	if (df->ml_max_sl_set) {
		df->dev.dev.uflg.fg_max_sl = 1;
		df->dev.dev.ufld.fd_max_sl = df->ml_max_sl;
	} else
		df->dev.dev.uflg.fg_max_sl = 0;

	if (XWriteDeviceInfo(&df->dev) == SUCCESS) {
		audit_subsystem(df->device_name,
	"Successful update of multilevel maximum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem(df->device_name,
	"Unuccessful update of multilevel maximum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

/* change single-level SL */

int
DevChangeSLSL(df)
	DeviceFillin *df;
{
	int ret;

	if (df->sl_sl_set) {
		df->dev.dev.uflg.fg_cur_sl = 1;
		df->dev.dev.ufld.fd_cur_sl = df->sl_sl;
	} else
		df->dev.dev.uflg.fg_cur_sl = 0;

	if (XWriteDeviceInfo(&df->dev) == SUCCESS) {
		audit_subsystem(df->device_name,
	"Successful update of single-level device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem(df->device_name,
	"Unuccessful update of single-level device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

/* change default multilevel min SL */

int
DevChangeDefMLMinSL(ddf)
	DefDeviceFillin	*ddf;
{
	int ret;

	ddf->sd.df.devg.fg_min_sl = 1;
	ddf->sd.df.devd.fd_min_sl = ddf->ml_min_sl;

	if (XWriteSystemInfo(&ddf->sd) == SUCCESS) {
		audit_subsystem("Default database",
"Successful update of default multilevel minimum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem("Default database",
"Unsuccessful update of default multilevel minimum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

/* change default multilevel max SL */

int
DevChangeDefMLMaxSL(ddf)
	DefDeviceFillin	*ddf;
{
	int ret;

	ddf->sd.df.devg.fg_max_sl = 1;
	ddf->sd.df.devd.fd_max_sl = ddf->ml_max_sl;

	if (XWriteSystemInfo(&ddf->sd) == SUCCESS) {
		audit_subsystem("Default database",
"Successful update of default multilevel maximum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem("Default database",
"Unsuccessful update of default multilevel maximum device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

/* change default single-level SL */
int
DevChangeDefSLSL(ddf)
	DefDeviceFillin	*ddf;
{
	int ret;

	ddf->sd.df.devg.fg_cur_sl = 1;
	ddf->sd.df.devd.fd_cur_sl = ddf->sl_sl;

	if (XWriteSystemInfo(&ddf->sd) == SUCCESS) {
		audit_subsystem("Default database",
"Successful update of default single-level device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem("Default database",
"Unsuccessful update of default single_level device sensitivity label",
		  ET_SYS_ADMIN);
		ret = 0;
	}
	return ret;
}

#endif /* SEC_MAC */

/* remove a device */

RemoveDevice(selected_name)
	char *selected_name;
{
	struct dev_if dv;
	int ret;

	/* Read the device assignment and terminal control database entries */

	ret = XGetDeviceInfo(selected_name, &dv);

	if (ret != SUCCESS)
		return 1;

	/* Remove the device */
	dv.dev.uflg.fg_name   = 0;
	dv.tm.uflg.fg_devname = 0;

	/* If it is a terminal or host then remove also */
	/* Write the information back */
	if (dv.dev.uflg.fg_type && 
	      (ISBITSET(dv.dev.ufld.fd_type, AUTH_DEV_TERMINAL)
#if SEC_NET_TTY
	    || ISBITSET(dv.dev.ufld.fd_type, AUTH_DEV_REMOTE)
#endif
	    ) )
		ret = XWriteDeviceAndTerminalInfo(&dv);
	else
		ret = XWriteDeviceInfo(&dv);

	if (ret == SUCCESS) {
		audit_subsystem(selected_name, 
		  "Device has been removed", ET_SYS_ADMIN);
		ret = 0;
	} else {
		audit_subsystem(selected_name,
		  "Unsuccessful removal of device", ET_SYS_ADMIN);
		ret = 1;
	}

	return ret;
}

/* validation routine for create/modify/update/delete screen */

DevValidateDeviceParams(defill, device_mode)
	DeviceFillin *defill;
	enum device_scrn_mode device_mode;
{
	int i;
	int ret = 0;

	for (i = 0; i < defill->ndevices; i++)
		if (!IsSpecialDevice(defill->devices[i])) {
			ret = 1;
			break;
		}

	if (ret == 0 && defill->ndevices > 0)
		if (IsDeviceAlreadyListed(defill->device_name,
					defill->devices, defill->ndevices))
			ret = 1;
	return ret;
}

/* Returns true if device is a special device */

static int
IsSpecialDevice(device_name)
	char *device_name;
{
	struct stat sb;

#ifdef DEBUG
	printf ("IsDeviceCharSpecial: name= %s\n", device_name);
#endif
	/* Check for null name */
	if (device_name[0] == '\0') {
		if (! msg_devices_error_1)
			LoadMessage ("msg_devices_add_error_null_device",
			       &msg_devices_error_1, &msg_devices_error_1_text);
		ErrorMessageOpen (-1, msg_devices_error_1, 0, NULL);
		return (0);
	}

	/* Check we can stat the file */
	if (stat(device_name, &sb) < 0) {
		if (! msg_devices_error_2)
			LoadMessage ("msg_devices_add_error_cant_stat",
			       &msg_devices_error_2, &msg_devices_error_2_text);
		ErrorMessageOpen (-1, msg_devices_error_2, 0, device_name);
		return (0);
	}
	
	/* Check it is a special character or block device */
	if ( !(S_ISCHR(sb.st_mode)) && !(S_ISBLK(sb.st_mode)) ) {
		if (! msg_devices_error_3)
			LoadMessage ("msg_devices_add_error_not_special",
			       &msg_devices_error_3, &msg_devices_error_3_text);
		ErrorMessageOpen (-1, msg_devices_error_3, 0, device_name);
		return (0);
	}
	return(1);
}

/*
 * returns true if the device is listed in another Device Assignment entry
 */

static int
IsDeviceAlreadyListed(dev_name, dev_list, dev_list_count)
	char	*dev_name;
	char **dev_list;
	int	dev_list_count;
{
	struct dev_asg	*dv;
	int i,j;
	char	*cp;

#ifdef DEBUG
	printf ("XIsDeviceAlreadyListed\nDevice name %s count %d\n",
	  dev_name, dev_list_count);
#endif
	setdvagent();
	while ( (dv=getdvagent()) != (struct dev_asg *) 0) {

	    /* only check if there is a device list and it's not our entry */

	    if (dv->uflg.fg_devs && strcmp(dv->ufld.fd_name,dev_name) ) {

		/* walk our list */

		for (j=0; j<dev_list_count; j++) {

		    /* walk their list, if there is one */

		    if (dv->uflg.fg_devs)
		      for (i=0; cp = dv->ufld.fd_devs[i]; i++) {

			/* if one of ours is on theirs, it's not unique */

			if (strcmp(dev_list[j], cp) == 0) {
			    if (! msg_devices_error_4)
			       LoadMessage ("msg_devices_add_error_is_listed",
      			       &msg_devices_error_4, &msg_devices_error_4_text);
			    ErrorMessageOpen (-1, msg_devices_error_4, 0, 
					dev_list[j]);
			    return(1);
			}
		      }
		}
	    }
	}
	return(0);
}

#endif /* SEC_BASE */
