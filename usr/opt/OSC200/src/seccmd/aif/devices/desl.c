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
static char	*sccsid = "@(#)$RCSfile: desl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:05 $";
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



/*
 * Sensitivity Label Support Routines for Device Assignment database
 */

#include <sys/secdefines.h>

#if SEC_MAC

#include "gl_defs.h"
#include "IfDevices.h"
#include "userif.h"

/* field titles (defined in de_scrns.c) */

extern uchar dedmnsl_title[];	/* default multilevel min SL */
extern uchar dedmxsl_title[];	/* default multilevel max SL */
extern uchar dedssl_title[];	/* default single-level SL */
extern uchar demnsl_title[];	/* device multilevel min SL */
extern uchar demxsl_title[];	/* device multilevel max SL */
extern uchar dessl_title[];	/* device single-level SL */

/*
 * Default multilevel device minimum sensitivity label
 */

int
dedmnsl(argv)
	char **argv;
{
	DefDeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*min_sl;
	int		ret;

	if (DevGetDefFillin(defill))
		return 1;

	min_sl = mand_alloc_ir();

	if (min_sl == (mand_ir_t *) 0)
		MemoryError();

	mand_copy_ir(defill->ml_min_sl, min_sl);

	ret = aif_label(dedmnsl_title, min_sl,
			NULL, NULL, NULL, NULL);

	if (ret == 0 && CompareSL(min_sl, defill->ml_max_sl)) {
		pop_msg("Default multilevel minimum sensitivity label must be",
		  "dominated by multilevel maximum sensitivity label.");
		ret = 1;
	}

	if (ret == 0) {
		mand_copy_ir(min_sl, defill->ml_min_sl);
		ret = DevChangeDefMLMinSL(defill);
	}

	mand_free_ir(min_sl);
	DevFreeDefFillin(defill);

	return ret;
}

/*
 * Default multilevel device maximum sensitivity label
 */

int
dedmxsl(argv)
	char **argv;
{
	DefDeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*max_sl;
	int		ret;

	if (DevGetDefFillin(defill))
		return 1;

	max_sl = mand_alloc_ir();

	if (max_sl == (mand_ir_t *) 0)
		MemoryError();

	mand_copy_ir(defill->ml_max_sl, max_sl);

	ret = aif_label(dedmxsl_title, max_sl,
			NULL, NULL, NULL, NULL);

	if (ret == 0 && CompareSL(defill->ml_min_sl, max_sl)) {
		pop_msg("Default multilevel minimum sensitivity label must be",
		  "dominated by multilevel maximum sensitivity label.");
		ret = 1;
	}

	if (ret == 0) {
		mand_copy_ir(max_sl, defill->ml_max_sl);
		ret = DevChangeDefMLMaxSL(defill);
	}

	mand_free_ir(max_sl);
	DevFreeDefFillin(defill);

	return ret;
}

/*
 * Default single-level device sensitivity label
 */

int
dedssl(argv)
	char **argv;
{
	DefDeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*cur_sl;
	int		ret;

	if (DevGetDefFillin(defill))
		return 1;

	cur_sl = mand_alloc_ir();

	if (cur_sl == (mand_ir_t *) 0)
		MemoryError();

	mand_copy_ir(defill->sl_sl, cur_sl);

	ret = aif_label(dedssl_title, cur_sl,
			NULL, NULL, NULL, NULL);

	if (ret == 0) {
		mand_copy_ir(cur_sl, defill->sl_sl);
		ret = DevChangeDefSLSL(defill);
	}

	mand_free_ir(cur_sl);
	DevFreeDefFillin(defill);

	return ret;
}

/*
 * Device multilevel minimum sensitivity label
 */

int
demnsl(argv)
	char **argv;
{
	DeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*min_sl;
	mand_ir_t	*max_sl;
	mand_ir_t	*def_sl;
	int		ret;
	char		use_default;

	if (!IsDeviceSelected) {
		pop_msg("You must select a device before setting its",
		  "multilevel minimum sensitivity label.");
		return 1;
	}
	if (DevGetFillin(DevSelected, defill))
		return 1;

	min_sl = mand_alloc_ir();

	if (min_sl == (mand_ir_t *) 0)
		MemoryError();

	/* if the system default min sl is not set, use min SL/syslo as dflt */

	if (defill->dev.dev.sflg.fg_min_sl)
		def_sl = defill->dev.dev.sfld.fd_min_sl;
	else
#if SEC_ENCODINGS
		def_sl = mand_minsl;
#else
		def_sl = mand_syslo;
#endif

	/* use the device specific or system default max SL for comparison */

	if (!defill->ml_max_sl_set) {
		if (defill->dev.dev.sflg.fg_max_sl)
			max_sl = defill->dev.dev.sfld.fd_max_sl;
		else
			max_sl = mand_syshi;
	} else {
		max_sl = defill->ml_max_sl;
	}

	if (defill->ml_min_sl_set) {
		use_default = 0;
		mand_copy_ir(defill->ml_min_sl, min_sl);
	} else {
		use_default = 1;
		mand_copy_ir(def_sl, min_sl);
	}

	ret = aif_label(demnsl_title, min_sl,
			NULL, NULL, &use_default, def_sl);

	if (ret == 0 && CompareSL(min_sl, max_sl)) {
		pop_msg("Device multilevel minimum sensitivity label must be",
		  "dominated by multilevel maximum sensitivity label.");
		ret = 1;
	}

	if (ret == 0) {
		if (use_default)
			defill->ml_min_sl_set = 0;
		else {
			defill->ml_min_sl_set = 1;
			mand_copy_ir(min_sl, defill->ml_min_sl);
		}
		ret = DevChangeMLMinSL(defill);
	}

	mand_free_ir(min_sl);
	DevFreeFillin(defill);

	return ret;
}


/*
 * Device multilevel maximum sensitivity label
 */

int
demxsl(argv)
	char **argv;
{
	DeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*min_sl;
	mand_ir_t	*max_sl;
	mand_ir_t	*def_sl;
	int		ret;
	char		use_default;

	if (!IsDeviceSelected) {
		pop_msg("You must select a device before setting its",
		  "multilevel maximum sensitivity label.");
		return 1;
	}
	if (DevGetFillin(DevSelected, defill))
		return 1;

	max_sl = mand_alloc_ir();

	if (max_sl == (mand_ir_t *) 0)
		MemoryError();

	/* if the system default max sl is not set, use syshi as dflt */

	if (defill->dev.dev.sflg.fg_max_sl)
		def_sl = defill->dev.dev.sfld.fd_max_sl;
	else
		def_sl = mand_syshi;

	/* use the device specific or system default min SL for comparison */

	if (!defill->ml_min_sl_set) {
		if (defill->dev.dev.sflg.fg_min_sl)
			min_sl = defill->dev.dev.sfld.fd_min_sl;
		else
#if SEC_ENCODINGS
			min_sl = mand_minsl;
#else
			min_sl = mand_syslo;
#endif
	} else {
		min_sl = defill->ml_min_sl;
	}

	if (defill->ml_max_sl_set) {
		use_default = 0;
		mand_copy_ir(defill->ml_max_sl, max_sl);
	} else {
		use_default = 1;
		mand_copy_ir(def_sl, max_sl);
	}

	ret = aif_label(demxsl_title, max_sl,
			NULL, NULL, &use_default, def_sl);

	if (ret == 0 && CompareSL(min_sl, max_sl)) {
		pop_msg("Device multilevel minimum sensitivity label must be",
		  "dominated by multilevel maximum sensitivity label.");
		ret = 1;
	}

	if (ret == 0) {
		if (use_default)
			defill->ml_max_sl_set = 0;
		else {
			defill->ml_max_sl_set = 1;
			mand_copy_ir(max_sl, defill->ml_max_sl);
		}
		ret = DevChangeMLMaxSL(defill);
	}

	mand_free_ir(max_sl);
	DevFreeFillin(defill);

	return ret;
}

/*
 * Device single-level sensitivity label
 */

int
dessl(argv)
	char **argv;
{
	DeviceFillin debuf, *defill = &debuf;
	mand_ir_t	*cur_sl;
	mand_ir_t	*def_sl;
	int		ret;
	char 		use_default;

	if (!IsDeviceSelected) {
		pop_msg("You must select a device before setting its",
		  "single-level sensitivity label.");
		return 1;
	}
	if (DevGetFillin(DevSelected, defill))
		return 1;

	cur_sl = mand_alloc_ir();

	if (cur_sl == (mand_ir_t *) 0)
		MemoryError();

	if (defill->dev.dev.sflg.fg_cur_sl)
		def_sl = defill->dev.dev.sfld.fd_cur_sl;
	else
#if SEC_ENCODINGS
		def_sl = mand_minsl;
#else
		def_sl = mand_syslo;
#endif

	if (defill->sl_sl_set) {
		use_default = 0;
		mand_copy_ir(defill->sl_sl, cur_sl);
	}
	else {
		use_default = 1;
		mand_copy_ir(def_sl, cur_sl);
	}

	ret = aif_label(dessl_title, cur_sl,
			NULL, NULL, &use_default, def_sl);

	if (ret == 0) {
		if (use_default)
			defill->sl_sl_set = 0;
		else {
			defill->sl_sl_set = 1;
			mand_copy_ir(cur_sl, defill->sl_sl);
		}
		ret = DevChangeSLSL(defill);
	}

	mand_free_ir(cur_sl);
	DevFreeFillin(defill);

	return ret;
}
#endif /* SEC_MAC */
