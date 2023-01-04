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
 *	@(#)$RCSfile: hal_sysinfo.h,v $ $Revision: 1.1.8.2 $ (DEC) $Date: 1993/09/03 14:29:19 $
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
 *	This file contains constants used with the hal_getsysinfo() and
 *	hal_setsysinfo() system calls.
 *
 *	Both of these calls are operation driven; particular
 *	flavors of operation may use arguments, identifiers, flags, etc.
 *	to define the actual result.
 *
 */

/***************************************************************************
 ***************************************************************************
 **  WARNING, HAZARD, WATCH OUT, NOTICE!!!!!!!!!!!!!!
 **
 ** Whenever adding a new GSI or SSI number look in both <hal/hal_sysinfo.h>
 ** and <sys/sysinfo.h> to make sure that the function numbers remain unique!
 ** The defines for GSI/SSI appear in these two header files and can not
 ** overlap, otherwise the hal variant would never be called!
 **
 ***************************************************************************
 ***************************************************************************/

#ifndef _HAL_SYSINFO_H_
#define _HAL_SYSINFO_H_

/*
 *	hal_getsysinfo() operation types
 */

#define GSI_NETBLK	4	/* Return the entire netblk structure */
				/* which is used for network install */
#if ULT_BIN_COMPAT
#define GSI_WSD_TYPE    10      /* Workstation Display Type Info */

#define GSI_WSD_UNITS   11      /* Workstation Display Units Info */

#endif /* ULT_BIN_COMPAT */

#define	GSI_BOOTCTLR	21	/* Logical Controller # for TURBOchannel slot */

#define	GSI_CONSTYPE	22	/* MIPS console type identifier */


/* All values from 1 - 22 are reserved. This is for compatibility */
/* with ULTRIX							  */

#define GSI_BUS_STRUCT  23     /* get bus structure */
#define GSI_BUS_NAME    24     /* get name of bus */

#define GSI_CTLR_STRUCT 25     /* get controller structure */
#define GSI_CTLR_NAME   26     /* get name of controller */

#define GSI_DEV_STRUCT  27     /* get device structure */
#define GSI_DEV_NAME    28     /* get name of device */

#define	GSI_CPU		29     /* cpu type (from cpu global variable) */
#define GSI_MAX_CPU     30     /* max # cpu's on this machine */

#define GSI_PRESTO      34     /* size of non-volatile ram if installed */

#if     ULT_BIN_COMPAT

#define GSI_GRAPHICTYPE 36     /* Graphics module names */

#endif  /* ULT_BIN_COMPAT */

#define GSI_SCS_SYSID   38     /* scs_sysid for ci */
#define GSI_BUS_PNAME   39     /* get port name of bus */
#define GSI_CTLR_PNAME  40     /* get port name of controller */

#define GSI_WSD_CONS	43     /* get console device type (0=gfx,1=alt) */
#define GSI_BOOTEDFILE	44     /* get booted kernel filename */
#define GSI_IEEE_FP_CONTROL 45 /* gateway to ieee_get_fp_control */
#define GSI_IEEE_STATE_AT_SIGNAL 46 /* gateway to ieee_get_state_st_signal */
#define GSI_PROM_ENV	47

#define GSI_GRAPHIC_RES	50     /* get graphics card resolution */

#define GSI_LURT	51     /* get LURT table entry for licensing use */

#define GSI_KEYBOARD	52     /* get keyboard name */
#define GSI_POINTER	53     /* get mouse/tablet name */

#define GSI_DEV_MOD	100    /* return dev_mod struct - loadable drivers */

#define SSI_IEEE_FP_CONTROL 14 /* gateway to ieee_set_fp_control */
#define SSI_IEEE_STATE_AT_SIGNAL 15 /* gateway to ieee_eet_state_st_signal */
#define SSI_IEEE_IGNORE_STATE_AT_SIGNAL 16 /* to ieee_ignore_state_st_signal */
#define SSI_PROM_ENV	17

/*
 *      setsysinfo() SSI_NVPAIRS variable names
 */

/* All values from 1 - 8 are reserved for compatibility with ULTRIX */
#define SSIN_LOAD_CONFIG 100	/* Add a new loadable driver config struct */
#define SSIN_DEL_CONFIG 101	/* Delete all driver config structs        */
#define SSIN_LOAD_DEVSTATE 102	/* Add device method state	           */
#define SSIN_UNLOAD_DEVSTATE 103 /* Delete device method state	           */

#endif /* _HAL_SYSINFO_H_ */







