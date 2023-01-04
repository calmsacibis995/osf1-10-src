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
 *	@(#)$RCSfile: sysconfig.h,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/04/15 14:12:32 $
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

#ifndef _SYS_SYSCONFIG_H_
#define _SYS_SYSCONFIG_H_

#include <sys/types.h>

typedef int (*sysconfig_entrypt_t)();		/* kernel module entry point */
typedef int   sysconfig_op_t;			/* configuration operation */

#define	SYSCONFIG_PARAM_MAX	NBPG		/* max size of param buffer */

/*
 *	kmodcall() sysconfig_op_t types
 */
#define SYSCONFIG_NOSPEC	0x00		
#define SYSCONFIG_CONFIGURE	0x01	
#define SYSCONFIG_UNCONFIGURE	0x02
#define SYSCONFIG_QUERY		0x04
#define SYSCONFIG_OPERATE	0x08

/*
 * 	Device interrupt handler ih_flag defines for adding a interrupt handler.
 * 	Also used by configuration manager methods.
 */
#define IH_VEC_DYNAMIC_OK       0x0001  /* Allow relocation of target vector */
#define IH_VEC_MULTIPLE_OK      0x0002  /* Allow multiple handlers per vector */
#define IH_VEC_PASS_ISP         0x0004  /* Pass ptr to interrupt frame to ISR */

#define IH_DRV_NONPARALLEL      0x8000  /* Driver is non-parallelized */
#define IH_DRV_NEWMAJOR         0x4000  /* Force install at new major number */




/*
 * 	Device interrupt handler defines.
 *	Used by device subsystems when adding an interrupt handler.
 *	Also passed by cfgmgr methods to device subsystem configuration 
 *		entry points (via (via device_admin_t)).
 */
#define IH_VEC_DYNAMIC_OK       0x0001  /* Allow relocation of target vector */
#define IH_VEC_MULTIPLE_OK      0x0002  /* Allow multiple handlers per vector */
#define IH_VEC_PASS_ISP         0x0004  /* Pass ptr to interrupt frame to ISR */

/*
 * 	Device switch defines.
 *	Used by device subsystems when adding an device switch.
 *	Also passed by cfgmgr methods to device subsystem configuration 
 *		entry points (via (via device_admin_t)).
 */
#define IH_DRV_NONPARALLEL      0x8000  /* Driver is non-parallelized */
#define IH_DRV_NEWMAJOR         0x4000  /* Force install at new major number */
#define IH_DRV_USEMAJOR         0x2000  /* Force install at new major number */
#define IH_DRV_SAMEMAJOR        0x1000  /* Use same # for block & char major */
#define IH_DRV_DYNAMIC          0x10000  /* Driver is loaded; not static     */


/*
 * Configuration I/O structure definitions
 */

/*
 * File System configuration entry point in/out data structures
 */
#define	OSF_FILESYS_CONFIG_10	0x04026020 /* OSF/1 filesys_config_t version */

typedef struct filesys_config {
	uint	fc_version;
	uint	fc_type;
	ulong	fc_flags;
} filesys_config_t;


/*
 * Device System configuration entry point in/out data structures
 */
#define	OSF_DEVICE_CONFIG_10	0x04026021 /* OSF/1 device_config_t version */
#define DRIVER_BUILD_LEVEL	OSF_DEVICE_CONFIG_10
#define DEVNAMESZ 80

typedef struct {
	uint	dc_version;
	uint	dc_errcode;		/* Additional error information */
	long	dc_bmajnum;		/* Preferred block major number */
	long	dc_cmajnum;		/* Preferred char major number */
	long	dc_begunit;		/* 1st minor device number in range */
	long	dc_numunit;		/* number of minor device numbers */
	long	dc_dsflags;		/* Device switch config flags */
	long	dc_ihflags;		/* Interrupt switch config flags */
	long	dc_ihlevel;		/* Preferred interrupt level */
	char 	config_name[DEVNAMESZ +1]; /* Driver name for resolver */
} device_config_t;

/*
 * Structure used to maintain driver method state.
 */
typedef struct _dev_mod_t {
	struct _dev_mod_t	*next;
	struct _dev_mod_t	*prev;
	char 		dev_name[DEVNAMESZ +1];
	uint		dev_load_flags;
	long		dev_id;
	device_config_t	dev_outadm;
} dev_mod_t;

#if     MULTIMAX
struct mmax_devconf {
	long	mdc_level;		/* Preferred interrupt level */
	long	mdc_bmajnum;		/* Preferred block major number */
	long	mdc_cmajnum;		/* Preferred char major number */
	long	mdc_minnum;		/* 1st minor device number in range */
	long	mdc_flags;		/* Configuration flags */
	long	mdc_errcode;		/* Additional error information */
};
#endif

#endif /* _SYS_SYSCONFIG_H_ */
