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
static char	*sccsid = "@(#)$RCSfile: mkheaders.c,v $ $Revision: 4.3.3.4 $ (DEC) $Date: 1992/12/08 18:13:08 $";
#endif 
/*
 */
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


/*	Change History							*
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures.	*
 *									*
 * 6-14-91	Brian Stevens						*
 *		Removed lines which overwrote a previously created	*
 *		vba.h.							*
 *									*
 */

/*
 * Make all the .h files for the optional entries
 */

#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "y.tab.h"


extern char	*toheader();

headers()
{
	register FILE *fp;
	register struct file_list *fl;
	int needndx;
	

	/* Check all the entries in the file list (fl) and count up how
	 * many are asked for.  This count is loaded into the "h" file
	 * as a define which the drivers use to size the 'number' of things.
	 * i.e. if 4 ra devices are in the config file then the define NRA will
	 * be set to 4.
	 */
	for (fl = ftab; fl != 0; fl = fl->f_next)
		for (needndx = 0; needndx < NNEEDS; needndx++) {
			if (fl->f_needs[needndx] == NULL) {
				break;
			}
			do_count(fl->f_needs[needndx], fl->f_needs[needndx],
						1);
		}

	/* check to see if the SCS subsytem is configured in :
	 * currently if "uq", "bvpssp", "ci", or "msi" are configured
	 * then the scs subsystem is also included.
	 */
 
	if (isconfigured("uq") || isconfigured("bvpssp") 
			       || isconfigured("ci") || isconfigured("msi")) {
		if (scs_system_id.lol==0 && scs_system_id.hos==0) {
			printf("scs_sysid must be specified\n");
			Exit(1);
		}
	}

#ifdef __alpha
/* ahvfix.  The Brian Stevens comment at the top indicates that vba.h
 *	doesn't get created here. But then where does it get created? And,
 *	don't you WANT to overwrite previously created vba.h's?
 *	We'll put the code Brian yanked out back in until we figure out
 *	what's going on.  If we don't, vba.h never gets created.
 */
        fp=fopen(path("vba.h"),"w");
        fprintf(fp,"#define NVBA %d \n",vba_bus.max_bus_num);
        fprintf(fp,"#define CVBA %d \n",vba_bus.cnt);
        (void) fclose(fp);
#endif /* alpha */

	/* Load the 'scs_data.h' file.
	 * It gets a node name which is an array of 8 char's that hold the IDENT
	 * nad a sysid which is a char array size 2.
	 */
	{
	register int i;
	fp=fopen(path("scs_data.h"),"w");
	fprintf(fp,"#define SCS_NODE_NAME { ");
	for (i=0; ident[i] != '\0' && i < 8; i++)
		fprintf(fp, "'%c',", ident[i]);
	while(i++<8) {
		fprintf(fp, "' '");
		if(i < 8)
			fprintf(fp, ",");
	}
	fprintf(fp,"}\n");
	fprintf(fp,"#define SCS_SYSID { %d,%d }\n", scs_system_id.lol, 
		scs_system_id.hos);
	(void) fclose(fp);
	}

	fp=fopen(path("vaxbi.h"),"w");
	fprintf(fp,"#define NVAXBI %d \n",vaxbi_bus.max_bus_num);
	fprintf(fp,"#define CVAXBI %d \n",vaxbi_bus.cnt);
	(void) fclose(fp);

	/* Find all the pseudo-device entries in the config file and
	 * make "h" files for them.
	 */
	pseudo_dev_hfile();

}

/*
 * count all the devices of a certain type and recurse to count
 * whatever the device is connected to
 */
do_count(dev, hname, search)
	register char *dev, *hname;
	int search;
{
	register struct device_entry *dp, *mp;
	register int count;

	for (count = 0,dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_unit != -1 && eq(dp->d_name, dev)) {
			if (dp->d_type == PSEUDO_DEVICE) {
				count =
				    dp->d_slave != UNKNOWN ? dp->d_slave : 1;
				if (dp->d_flags) /* was this an OPTIONS line */
					dev = NULL;
				break;
			}
			/*
			 * Allow holes in unit numbering,
			 * assumption is unit numbering starts
			 * at zero.
			 */
			if (dp->d_unit + 1 > count)
				count = dp->d_unit + 1;
			if (search) {
				mp = dp->d_conn;
                                if (mp != 0 && mp != TO_NEXUS &&
				    mp->d_conn != TO_NEXUS) {
                                        /*
					 * Check for the case of the
					 * controller that the device
					 * is attached to is in a separate
					 * file (e.g. "sd" and "sc").
					 * In this case, do NOT define
					 * the number of controllers
					 * in the hname .h file.
					 */
					if (!file_needed(mp->d_name))
					do_count(mp->d_name, hname, count);
					search = 0;
				}
			}
		}
	}
	do_header(dev, hname, count, 0);
	/* OPTION/ line specifies "hname"_DYNAMIC define */
	if (dp && dp->d_type == PSEUDO_DEVICE && dp->d_flags == 2)
		do_header(NULL, hname, dp->d_dynamic == NULL ? 0 : 1, 1);
}

/*
 * Scan the file list to see if name is needed to bring in a file.
 */
file_needed(name)
	char *name;
{
	register struct file_list *fl;
	int i;

	for (fl = ftab; fl != 0; fl = fl->f_next) {
		for (i = 0; i < NNEEDS; i++) {
			if(fl->f_needs[i])
				if ( strcmp(fl->f_needs[i], name) == 0)
					return (1);
		}
	}
	return (0);
}

/* Open up a XXXXX.h file and load it with #define's that will reflect
 * the number of items found by config
 */
do_header(dev, hname, count, dynamic_define)
	char *dev, *hname;
	int count, dynamic_define;
{
	char *file, *name, *inw, *toheader(), *tomacro();
	char dyndefine[64];
	struct file_list *fl, *fl_head;
	FILE *inf, *outf;
	int inc, oldcount;

	/* Stick a ".h" on the end of the name to form a header file name */
	file = toheader(hname);
	if (dynamic_define) {
		strcpy(dyndefine, hname);
		strcat(dyndefine, "_DYNAMIC");
		name = tomacro(dyndefine, 0);	/* x_DYNAMIC define */
	} else if (dev != NULL) {
		name = tomacro(dev, 1);		/* PSUEDO_DEVICE define */
	} else {
		name = tomacro(hname, 0);	/* OPTION/ define */
	}
	inf = fopen(file, "r");
	oldcount = -1;
	if (inf == 0) {
		outf = fopen(file, "w");
		if (outf == 0) {
			perror(file);
			Exit(1);
		}
		fprintf(outf, "#define %s %d\n", name, count);
		(void) fclose(outf);
		return;
	}
	fl_head = 0;
	for (;;) {
		char *cp;
		if ((inw = get_word(inf)) == 0 || inw == (char *)EOF)
			break;
		if ((inw = get_word(inf)) == 0 || inw == (char *)EOF)
			break;
		inw = ns(inw);
		cp = get_word(inf);
		if (cp == 0 || cp == (char *)EOF)
			break;
		inc = atoi(cp);
		if (eq(inw, name)) {
			oldcount = inc;
			inc = count;
		}
		cp = get_word(inf);
		if (cp == (char *)EOF)
			break;
		fl = (struct file_list *) malloc(sizeof *fl);
		fl->f_fn = inw;
		fl->f_type = inc;
		fl->f_next = fl_head;
		fl_head = fl;
	}
	(void) fclose(inf);
	if (count == oldcount) {
		for (fl = fl_head; fl != 0; fl = fl->f_next)
			free((char *)fl);
		return;
	}
	if (oldcount == -1) {
		fl = (struct file_list *) malloc(sizeof *fl);
		fl->f_fn = name;
		fl->f_type = count;
		fl->f_next = fl_head;
		fl_head = fl;
	}
	unlink(file);
	outf = fopen(file, "w");
	if (outf == 0) {
		perror(file);
		Exit(1);
	}
	for (fl = fl_head; fl != 0; fl = fl->f_next) {
		fprintf(outf, "#define %s %d\n",
		    fl->f_fn, count ? fl->f_type : 0);
		free((char *)fl);
	}
	(void) fclose(outf);
}


/*
 * convert a dev name to a .h file name bu appending a ".h" on the end of the name.
 */
char *
toheader(dev)
	char *dev;
{
	static char hbuf[80];

	(void) strcpy(hbuf, path(dev));
	(void) strcat(hbuf, ".h");
	return (hbuf);
}

/*
 * convert a dev name to a macro name by putting a 'N' in the front.
 */
char *tomacro(dev, isdev)
	register char *dev;
	register int isdev;
{
	static char mbuf[64];
	register char *cp;

	cp = mbuf;
	if (isdev)
		*cp++ = 'N';
	while (*dev)
		if (!islower(*dev))
			*cp++ = *dev++;
		else
			*cp++ = toupper(*dev++);
	*cp++ = 0;
	return (mbuf);
}

pseudo_dev_hfile()
{
	char *toheader(), *tomacro();
	FILE *inf, *outf;
	register struct device_entry *dp;
/* OK OK Here is where the "h" files are made for any pseudo-device requested.
 * We will walk the device table (dtab) looking for PSEUDO_DEVICE types with a
 * flag field of zero (non-zero is for internal generated pseudo entries --
 * whatever they are??).  The slave field is used to hold any arguments that 
 * are with the device .... ie. pty 80.
 *
 */ 

 	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if((dp->d_type == PSEUDO_DEVICE ) && ( dp->d_slave >= 0) && (dp->d_flags == 0)){
			inf = fopen(toheader(dp->d_name), "r");
			if (inf != 0) {
				unlink(toheader(dp->d_name));
				fclose(inf);
			}
			
			outf = fopen(toheader(dp->d_name), "w");
			if (outf == 0) {
				perror(toheader(dp->d_name));
				Exit(1);
			}
			fprintf(outf, "#define %s %d\n",
				tomacro(dp->d_name, 1), dp->d_slave);
			(void) fclose(outf);

		}
	}
}
