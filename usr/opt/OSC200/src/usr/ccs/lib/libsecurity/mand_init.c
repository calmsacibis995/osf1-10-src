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
static char	*sccsid = "@(#)$RCSfile: mand_init.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:17:17 $";
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
 *  Copyright (c) 1990 SecureWare, Inc.
 *  All Rights Reserved
 *
 *  Mandatory access control initialization library.
 *
 *  This library of subroutines initializes policy parameters for use
 *  by other routines in the libmand directory for manipulating
 *  and changing sensitivity and information labels.
 */



#include <sys/secdefines.h>
#include "libsecurity.h"

#if SEC_MAC

#include <sys/types.h>
#include <sys/errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/security.h>
#include <sys/secpolicy.h>
#include <sys/secioctl.h>
#include <mandatory.h>

struct mand_config mand_config;	/* configuration parameter structure */

unsigned mand_max_class = 0;	/* maximum numerical classification */
unsigned mand_max_cat = 0;	/* maximum numerical category */
unsigned mand_max_mark = 0;	/* maximum numerical marking (ILB only) */

static	mandinit = 0;		/* set if initialized */
static ilb_configured = 0;	/* set if information labels are configured */
static struct sp_init sp_init;	/* initialization parameters for daemon */

extern	int errno;
extern	mand_ir_t *er_to_ir();

#define	MAC_PARAM_FILE	"/etc/policy/mac/config"
#define	ILB_PARAM_FILE	"/etc/policy/macilb/config"

/*
 * Initialize policy parameters [and label encodings]
 */

int
mand_init()
{
	int ret;

	if (mandinit)		/* once only */
		return 0;

	ret = mand_init_params();
#if SEC_ENCODINGS
	if (ret == 0)
		ret = init_encodings();
#endif
	if (ret == 0)
		mandinit = 1;

	return ret;
}

int
mand_init_params()
{
	FILE	*fp;
	int	fields;
	char	*bp, buffer[100];
	int	rval;
	int	mac_device;
	char	*param_file;

#ifndef SEC_STANDALONE /*{*/
	/* perform the ioctl to get parms */

	if((mac_device = open(SPD_CONTROL_DEVICE,O_RDONLY)) == -1) {
		perror(MSGSTR(MAND_INIT_1, "Error opening policy control device"));
		return(-1);
	}

	/*
	 * Check to see which module is configured into the kernel
	 */

	sp_init.magic = SEC_MACILB_MAGIC;

	rval = ioctl(mac_device, SPIOC_GETCONF, &sp_init);
	if (rval == -1 && errno == ENXIO) {
		sp_init.magic = SEC_MAC_MAGIC;
		rval = ioctl(mac_device, SPIOC_GETCONF, &sp_init);
		param_file = MAC_PARAM_FILE;
	} else {
		ilb_configured = 1;
		param_file = ILB_PARAM_FILE;
	}

	close(mac_device);
	if (rval == -1)
		return rval;

	mand_config.subj_tags = sp_init.subj_tag_count;
	mand_config.obj_tags = sp_init.obj_tag_count;
	mand_config.first_subj_tag = sp_init.first_subj_tag;
	mand_config.first_obj_tag = sp_init.first_obj_tag;
	mand_config.minor_device = sp_init.spminor;
	mand_config.policy = sp_init.policy;

#else /*} SEC_STANDALONE {*/
#if SEC_ENCODINGS
	param_file = ILB_PARAM_FILE;
#else
	param_file = MAC_PARAM_FILE;
#endif
#endif /*} SEC_STANDALONE */

	/* read in the appropriate parameters file */

	fp = fopen (param_file, "r");
	if (fp == (FILE *) 0) {
		fprintf(stderr,
			MSGSTR(MAND_INIT_2, "Cannot open policy config file %s\n"), param_file);
		perror(param_file);
		return -1;
	}

	/* get parameter line */
	do {
		if (fgets (buffer, sizeof buffer, fp) == NULL) {
			fclose (fp);
			fprintf(stderr,
				  MSGSTR(MAND_INIT_3, "Empty policy config file %s\n"), param_file);
			return -1;
		}
		bp = buffer + strspn(buffer, " \t");
	} while (*bp == '\n' || *bp == '#' || *bp == '\0');

	fields = sscanf (buffer,
			"%s %ld %ld %d %d %d",
			mand_config.dbase,
			&mand_config.cache_size, &mand_config.buffers,
			&mand_max_class, &mand_max_cat,
			&mand_max_mark);

	if (fields != (ilb_configured ? 6 : 5)) {	/* format error */
		fprintf(stderr,
		    MSGSTR(MAND_INIT_4, "Format error in policy config file %s\n"), param_file);
		fclose(fp);
		return(-1);
	}

	/* Classifications and categories [and markings] are numbered from 0.
	 * Adjust the counts read from the config file to maximum values.
	 */
	--mand_max_class;
	--mand_max_cat;
	--mand_max_mark;

#if !SEC_ENCODINGS
	if (mand_set_sl_range(fp)) {
		fclose(fp);
		return -1;
	}
#endif

	fclose(fp);
	return(0);
}


/* special entry point for the daemon, who needs to consult the kernel
 * to initialize the policy file when the system is being initialized.
 */

int
mand_init_daemon(sp)
	struct sp_init	*sp;
{
	if (mand_init() != 0)
		return -1;

	/* Copy into caller's structure from cached version */

	*sp = sp_init;

	return 0;
}

#if !SEC_ENCODINGS
/*
 * Initialize parameters for mandatory access control.
 */
int
mand_set_sl_range(fp)
FILE	*fp;
{
	int	ret, fields;
	char	buffer[100];
	long	minor_device;

	if (mandinit)	/* do this only once */
		return 0;

	/*
	 * Must set this before building class and cat tables to prevent
	 * recursion.
	 */
	mandinit = 1;
	
	/* construct the classifications and categories tables so we
	 * can parse the system high and system low labels.
	 */
	ret = mand_make_class_table();
	if (ret == 0)
		ret = mand_make_cat_table();
	if (ret != 0) {
		fclose(fp);
		mand_end();
		mandinit = 0;
		return ret;
	}

	/* get system low value */
	while (fgets (buffer, sizeof buffer, fp) != NULL)
		if (buffer[0] != '\n' && buffer[0] != '#') {
			buffer[strlen(buffer) - 1] = '\0';
			mand_syslo = er_to_ir (buffer, 0);
			break;
		}
	if (mand_syslo == (mand_ir_t *) 0) {
		fprintf(stderr,
			MSGSTR(MAND_INIT_5, "Cannot read System Low label from MAC config file\n"));
		ret = -1;
	}
	
	/* get system high value */
	while (fgets (buffer, sizeof buffer, fp) != NULL)
		if (buffer[0] != '\n' && buffer[0] != '#') {
			buffer[strlen(buffer) - 1] = '\0';
			mand_syshi = er_to_ir (buffer, 0);
			break;
		}
	if (mand_syshi == (mand_ir_t *) 0) {
		fprintf(stderr,
			MSGSTR(MAND_INIT_6, "Cannot read System High label from MAC config file\n"));
		ret = -1;
	}
		
	(void) fclose (fp);

	if (ret != 0) {
		mand_end();
		mandinit = 0;
	}

	return ret;
}
#endif /* !SEC_ENCODINGS */

#endif /* SEC_MAC */
