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
 *	@(#)$RCSfile: IfDevices.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:59:49 $
 */ 
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

/*

 *
 * Include file for X and terminfo interfaces for devices
 */

#ifndef __IfDevices__
#define __IfDevices__

#include <sys/secdefines.h>

#if SEC_BASE

/* Common C include files */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#include <sys/errno.h>
#include <ctype.h>
#include <signal.h>

/* Security include files */
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "XAccounts.h"

/* Bring in .h files which are needed */
#ifndef NAME_MAX
#include <limits.h>
#endif

#ifndef O_RDONLY
#include <fcntl.h>
#endif

enum device_type {	UnknownDevice,
			PrinterDevice,
			TerminalDevice,
			RemovableDevice
#ifdef SEC_NET_TTY
		      , HostDevice
#endif
};

#define DEVNAMELEN	14 /* size of terminal name in term ctl db */
#define DEVICEWIDTH	16 /* size of device synonyms on screen */

/* externals used by other routines */

extern char DevSelected[];
extern char IsDeviceSelected;
extern enum device_type DeviceType;

enum device_scrn_mode {	DeviceCreate,
			DeviceUpdate,
			DeviceDisplay,
			DeviceRemove};

/* Fillin structure for a specific device */

typedef struct device_fillin {
	char		device_name[DEVNAMELEN + 1];
	enum device_type device_type;	/* device type */
	char		**devices;	/* device list */
	int		ndevices;	/* length of devices */
	char 		**auth_users;	/* authorized user list */
	int		nauth_users;	/* length of auth_users */
	char		import_enable;	/* enabled for import */
	char		export_enable;	/* enabled for export */
	char		locked;		/* administrative lock set? */
	char		def_max_ulogins; /* default max u logins? */
	long		def_max_ulogins_value;
	int		max_ulogins;	/* max unsuccessful logins */
	char		def_login_timeout; /* default login timeout? */
	long		def_login_timeout_value;
	int		login_timeout;	/* login timeout */
	char		def_login_delay; /* default login delay? */
	int		def_login_delay_value;
	int		login_delay;	/* login delay */
#if SEC_MAC
	char		ml_min_sl_set;	/* is multilevel min SL set? */
	mand_ir_t	*ml_min_sl;	/* multilevel min SL */
	char		ml_max_sl_set;	/* is multilevel max SL set? */
	mand_ir_t	*ml_max_sl;	/* multilevel max SL */
	char		sl_sl_set;	/* is single-level SL set? */
	mand_ir_t	*sl_sl;		/* single-level SL */
	char		asg_sl_sl;	/* assigned for single-level SLs */
	char		asg_ml_sl;	/* assigned for multilevel SLs */
#if SEC_ILB
	char		sl_il_set;	/* is single-level IL set? */
	ilb_ir_t	*sl_il;		/* single-level IL */
	char		asg_sl_il;	/* assigned for single-level ILs */
	char		asg_ml_il;	/* assigned for multilevel ILs */
#endif /* SEC_ILB */
#endif /* SEC_MAC */
	struct		dev_if	dev;	/* dev asg & term ctl db entry */
} DeviceFillin;

/* structure definition for system default device parameters */

typedef struct def_device_fillin {
	char 		**auth_users;	/* authorized user list */
	int		nauth_users;	/* length of auth_users */
	char		locked;		/* administrative lock set? */
	int		max_ulogins;	/* max unsuccessful logins */
	int		login_timeout;	/* login timeout */
	int		login_delay;	/* login delay */
#if SEC_MAC
	mand_ir_t	*ml_min_sl;	/* multilevel min SL */
	mand_ir_t	*ml_max_sl;	/* multilevel max SL */
	mand_ir_t	*sl_sl;		/* single-level SL */
#if SEC_ILB
	ilb_ir_t	*sl_il;		/* single-level IL */
#endif /* SEC_ILB */
#endif /* SEC_MAC */
	struct	sdef_if	sd;		/* System default database */
} DefDeviceFillin;

extern int DevGetFillin();		/* retrieve device fillin struct */
extern int DevCreateFillin();		/* new device fillin struct */
extern void DevFreeFillin();		/* return memory used by DevGetFillin */
extern int DevGetDefFillin();		/* retrieve sys default fillin */
extern void DevFreeDefFillin();		/* return DevGetDefFillin's memory */
extern int DevChangeUserList();		/* update authorized user list */
extern int DevChangeControlParams();	/* update control parameters */
extern int DevValidateDefControlParams();	/* control parm validation */
extern int DevChangeDefControlParams();	/* change default control parameters */
extern int DevChangeDefUserList();	/* update default user list */
extern int DevLock();			/* lock device */
extern int DevUnlock();			/* unlock device */
#if SEC_MAC
extern int DevChangeMLMinSL();		/* change multilevel min SL */
extern int DevChangeMLMaxSL();		/* change multilevel max SL */
extern int DevChangeSLSL();		/* change single-level SL */
extern int DevChangeDefMLMinSL();	/* change default multilevel min SL */
extern int DevChangeDefMLMaxSL();	/* change default multilevel max SL */
extern int DevChangeDefSLSL();		/* change default single-level SL */
#endif /* SEC_MAC */

#endif /* SEC_BASE */
#endif /* __IfDevices__ */
