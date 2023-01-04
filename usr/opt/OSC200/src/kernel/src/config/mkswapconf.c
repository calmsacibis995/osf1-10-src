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
static char	*sccsid = "@(#)$RCSfile: mkswapconf.c,v $ $Revision: 4.4.3.6 $ (DEC) $Date: 1992/09/11 17:31:29 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */


/************************************************************************
 *									*
 *			Copyright (c) 1990 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*		Change History						*
 *									*
 * 24-Apr-91	afd
 *		Add Alpha support.
 *									*
 */


/*
 * Build a swap configuration file.
 */
#include "config.h"

#include <stdio.h>
#include <ctype.h>

swapconf()
{
	register struct file_list *fl;
	struct file_list *do_swap();

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		fl = do_swap(fl);
	}
}

struct file_list *
do_swap(fl)
	register struct file_list *fl;
{
	FILE *fp;
	char  swapname[80], *cp;
	register struct file_list *swap;
/* 
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
	uint_t dev;

	if (eq(fl->f_fn, "generic") || eq(fl->f_fn, "boot")) {
		fl = fl->f_next;
		return (fl->f_next);
	}
	(void) sprintf(swapname, "swap%s.c", fl->f_fn);
	fp = fopen(path(swapname), "w");
	if (fp == 0) {
		perror(path(swapname));
		Exit(1);
	}
	fprintf(fp, "#include <sys/param.h>\n");
	fprintf(fp, "#include <sys/conf.h>\n");
	fprintf(fp, "\n");
	fprintf(fp, "#define BOOTDEVLEN 80\n");
	fprintf(fp, "char bootdevice[BOOTDEVLEN] = {0};\n");
	fprintf(fp, "long askme;\n");
	/*
	 * If there aren't any swap devices
	 * specified, just return, the error
	 * has already been noted.
	 */
	swap = fl->f_next;
	if (swap == 0 || swap->f_type != SWAPSPEC) {
		(void) unlink(path(swapname));
		fclose(fp);
		return (swap);
	}
	fprintf(fp, "dev_t\trootdev = makedev(%d, %d);\n",
		major(fl->f_rootdev), minor(fl->f_rootdev));
	fprintf(fp, "dev_t\targdev  = makedev(%d, %d);\n",
		major(fl->f_argdev), minor(fl->f_argdev));
	fprintf(fp, "dev_t\tdumpdev = makedev(%d, %d);\n",
		major(fl->f_dumpdev), minor(fl->f_dumpdev));
	fprintf(fp, "\n");

	if ( machine == MACHINE_DEC_RISC || machine == MACHINE_ALPHA) {
		fprintf(fp, "\nsetconf()\n");
		fprintf(fp, "{\n");
		fprintf(fp, "\t/* resolve reference for non-generic kernels */\n");
		fprintf(fp, "}\n");
	}
	fclose(fp);
	return (swap);
}

static	int devtablenotread = 1;
static	struct devdescription {
	char	*dev_name;
	int	dev_major;
	struct	devdescription *dev_next;
} *devtable;

/*
 * Given a device name specification figure out:
 *	major device number
 *	partition
 *	device name
 *	unit number
 * This is a hack, but the system still thinks in
 * terms of major/minor instead of string names.
 */

/* 
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
uint_t
nametodev(name, defunit, defpartition)
	char *name;
	int defunit;
	char defpartition;
{
	char *cp, partition;
	register unit_t unit;
	register major_t maj_no;
	register minor_t min_no;
	register int maxdisk = NONMSCP_MAXDISK;
	register struct devdescription *dp;

	cp = name;
	if (cp == 0) {
		fprintf(stderr, "config: internal error, nametodev\n");
		Exit(1);
	}
	while (*cp && !isdigit(*cp))
		cp++;
	unit = *cp ? atoi(cp) : defunit;
	if (name) {
		if (strncmp(name,"ra",2) == 0) {
			maxdisk = MSCP_MAXDISK;
		}
	}
	if (unit < 0 || unit > maxdisk) {		/* 001, 002 */
		fprintf(stderr,
"config: %s: invalid device specification, unit out of range\n", name);
		unit = defunit;			/* carry on more checking */
	}
	if (*cp) {
		*cp++ = '\0';
		while (*cp && isdigit(*cp))
			cp++;
	}
	partition = *cp ? *cp : defpartition;
	if (partition < 'a' || partition > 'h') {
		fprintf(stderr,
"config: %c: invalid device specification, bad partition\n", *cp);
		partition = defpartition;	/* carry on */
	}
	if (devtablenotread)
		initdevtable();
	for (dp = devtable; dp->dev_next; dp = dp->dev_next)
		if (eq(name, dp->dev_name))
			break;
	if (dp == 0) {
		fprintf(stderr, "config: %s: unknown device\n", name);
		return (NODEV);
	}
	maj_no = dp->dev_major;


	/*
	 * We differentiate between SCSI and DSA because each has
	 * a different underlying minor number format.
	 */ 
	switch (maj_no) {
		case SCSI_MAJ:
			min_no = MAKECAMMINOR(unit, (partition - 'a'));
			break;

		case MSCP_MAJ:
			min_no = MAKEMINOR(unit, (partition - 'a'));
			break;

		default:
			min_no = MAKEMINOR(unit, (partition - 'a'));
	}

	return (makedev(maj_no, min_no));
}

char *
devtoname(dev)
/* 
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
	uint_t dev;
{
	char buf[80]; 
	register struct devdescription *dp;
	register unit_t unit_no;
	register minor_t minor_no;

	if (devtablenotread)
		initdevtable();

	for (dp = devtable; dp->dev_next; dp = dp->dev_next) {
		if (major(dev) == dp->dev_major)
			break;
	}
	if (dp == 0)
		dp = devtable;
	/*
	 * We differentiate between SCSI and DSA because each has
	 * a different underlying minor number format.
	 */ 
	switch (major(dev)) {
		case SCSI_MAJ:
			unit_no = GETCAMUNIT(dev);
			minor_no = MAKECAMMINOR(unit_no, 0);
			break;

		case MSCP_MAJ:
			unit_no = GETUNIT(dev);
			minor_no = MAKEMINOR(unit_no, 0);
			break;

		default:
			unit_no = GETUNIT(dev);
			minor_no = MAKEMINOR(unit_no, 0);
	}
 
	sprintf(buf, "%s%d%c", dp->dev_name,
		unit_no, (minor_no) + 'a');
	return (ns(buf));
}

initdevtable()
{
	char buf[BUFSIZ];
	char line[BUFSIZ];
	int maj;
	register struct devdescription **dp = &devtable;
	FILE *fp;
	extern char* source_confdir;

#ifdef __alpha
/* alpha jestabro - make "config" ALWAYS look into the machine subdir */
/* The non-alpha version was changed in Silver, and may be ok now for alpha? */
	sprintf(buf, "%s/%s/devices", config_directory, machinename);
#else
	if( source )
		sprintf(buf, "%s/%s/devices", config_directory, machinename);
	else
		sprintf(buf, "%s/devices", machinename);
#endif /* __alpha */
	fp = VPATHopen(buf, "r");
	if (fp == NULL) {
		fprintf(stderr, "config: can't open %s\n", buf);
		Exit(1);
	}
	while (fgets(line, BUFSIZ, fp) != 0) {
		if (*line == '#' || *line == '\n')
			continue;
		if (sscanf(line, "%s\t%d\n", buf, &maj) != 2)
			break;
		*dp = (struct devdescription *)malloc(sizeof (**dp));
		(*dp)->dev_name = ns(buf);
		(*dp)->dev_major = maj;
		dp = &(*dp)->dev_next;
	}
	*dp = 0;
	fclose(fp);
	devtablenotread = 0;
}
