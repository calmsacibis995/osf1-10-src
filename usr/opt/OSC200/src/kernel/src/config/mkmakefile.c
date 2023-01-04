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
static char	*sccsid = "@(#)$RCSfile: mkmakefile.c,v $ $Revision: 4.4.24.7 $ (DEC) $Date: 1993/10/29 21:11:59 $";
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

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */



/*
 * Build the makefile for the system, from
 * the information in the files files and the
 * additional files for the machine being compiled to.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "y.tab.h"
#include "config.h"

#define OLDSALUTATION "# DO NOT DELETE THIS LINE"

int	source = 0;
int	kdebug = 0;
static	struct file_list *fcur;
char 	*tail();
struct dynlist *dynamic_targets;
struct dynlist *audit_targets;
char	*binary_dir;

#define GIG	1024	/* units of 1 meg */
#define HAL_LIB

#define next_word(fp, wd) \
	{ register char *word = get_word(fp); \
	  if (word == (char *)EOF) \
		return; \
	  else \
		wd = word; \
	}

/*
 * Lookup a file, by name.
 * This is done by checking the argument "name" against each entry in the 
 * file_list - a link list.
 */
struct file_list *
fl_lookup(file)
	register char *file;
{
	register struct file_list *fp;

	for (fp = ftab ; fp != 0; fp = fp->f_next) {
		if (eq(fp->f_fn, file))
			return (fp);
	}
	return (0);
}

/*
 * Lookup a file, by final component name.
 * This is done by checking the argument "file" reduced to only the
 * file name portion against each entry in the file_list
 * link list.
 */
struct file_list *
fltail_lookup(file)
	register char *file;
{
	return(fl_lookup(tail(file)));
}

/*
 * Make a new file list entry.  This is done by allocating a structure, setting
 * up the basic default values and linking it onto the file_list.
 */
struct file_list *
new_fent()
{
	register struct file_list *fp;
	int i;

	fp = (struct file_list *) malloc(sizeof *fp);
	for (i = 0; i < NNEEDS; i++) {
		fp->f_needs[i] = 0;
	}
	fp->f_directory = 0;
	fp->f_next = 0;
	fp->f_flags = 0;
	fp->f_type = 0;
	fp->f_extra = (char *) 0;
	
	/* If its the first one set up the head pointers otherwise just 
	 * link it on the queue.
	 */
	if (fcur == 0)
		fcur = ftab = fp;
	else
		fcur->f_next = fp;
	fcur = fp;
	return (fp);
}


char	*COPTS;
static	struct users {
	int	u_default;
	int	u_min;
	int	u_max;
} users[] = {
	{ 32, 8, 1024 },		/* MACHINE_DEC_RISC*/
	{ 32, 8, 1024 },		/* MACHINE_ALPHA */
};
#define NUSERS	(sizeof (users) / sizeof (users[0]))

extern char *search_path;

/*
 * Build the makefile from the skeleton
 */
makefile()
{
	FILE *ifp, *ofp;
	FILE *dfp;
	char line[BUFSIZ];
	struct opt *op;
	struct file_sys *fs;
	int found = 0;
	int found_ufs = 0;
	int found_rt = 0;
	struct users *up;
	extern int build_cputypes();
	extern int build_confdep();
	extern char *machinename;

	/*
	 * Flag if real-time is configured.
	 * If so, link to objects in BINARY.rt.
	 */
	for (op = opt; op; op = op->op_next) {
		if (strncmp(op->op_name, "RT_", 3) == 0) {
			found_rt = 1;
			break;
		}
	}

	read_files();
	read_filesystems();

	/*
	 * The next two loops will traverse the option and file
	 * system linked lists, to verify at least one file system
	 * that is listed in the filesystems file is configured.
	 */
	for (op = opt; op; op = op->op_next) {
		for (fs = file_sys; fs; fs = fs->fs_next){
			if (!strcmp(op->op_name, fs->fs_name)) {
				if (!strcmp(fs->fs_name, "UFS"))
					found_ufs = 1;
				found = 1;
			}
		}
	}

	/*
	 * If no match was found, report fatal error and return
	 */
	if (!found) {
		fprintf(stderr,"config: One or more entries listed in filesystems file must be specified\n");
		Exit(1);
	}

	/*
	 * If UFS was not configured, report warning only
	 */
	if (!found_ufs)
		fprintf(stderr, "Warning: UFS not configured\n");

	/*
	 * Set up the name of the binary directory based on whether
	 * real-time is configured.
	 */
	if (found_rt)
		binary_dir = "../BINARY.rt";
	else
		binary_dir = "../BINARY";

	/* If we build from source the config file is much changed
	 * from the makefile used by a user to build from source.  One
	 * way to get the changes is to read in a different template.
	 */
	if( source )
		(void) sprintf(line, "%s/template.mk", config_directory);
	else
		(void) sprintf(line, "%s/template.std.mk", config_directory);
	ifp = VPATHopen(line, "r");
	if (ifp == 0) {
		perror(line);
		Exit(1);
	}
	dfp = fopen(path("Makefile"), "r");
	rename(path("Makefile"), path("Makefile.old"));
	ofp = fopen(path("Makefile"), "w");
	if (ofp == 0) {
		perror(path("Makefile"));
		Exit(1);
	}
	fprintf(ofp, "IDENT=-DIDENT=%s", raise_to_upper(ident));
	if (found_rt)
		fprintf(ofp, " -DREALTIME");
	if (kdebug)
		fprintf(ofp, " -DKDEBUG");

	if (cputype == 0) {
		printf("cpu type must be specified\n");
		Exit(1);
	}

	{ struct cputype *cp, *prevp;
	  for (cp = cputype; cp; cp = cp->cpu_next) {
		for (prevp = cputype; prevp != cp; prevp = prevp->cpu_next) {
			if (strcmp(cp->cpu_name, prevp->cpu_name) == 0) {
				break;
			}
		}
		if (prevp == cp) {
			fprintf(ofp, " -D%s", cp->cpu_name);
		}
	  }
        }
	
	fprintf(ofp, " -DSWAPTYPE=%d",swaptype());

	if (source)
		fprintf(ofp," -DRELEASE='\"'%3.1f'\"' -DVERSION='\"'%d'\"'",
				release,version);

	do_build("cputypes.h", build_cputypes);
	for (op = opt; op; op = op->op_next)
		if (op->op_value)
			fprintf(ofp, " -D%s=\"%s\"", op->op_name, op->op_value);
		else {
			if (eq(op->op_name,"GFLOAT"))
				fprintf(ofp, " -Mg");
			fprintf(ofp, " -D%s", op->op_name);
		}

	fprintf(ofp, "\n");

	if (hadtz == 0)
		printf("timezone not specified; gmt assumed\n");

	up = &users[0];

	if (maxusers == 0) {
		printf("maxusers not specified; %d assumed\n", up->u_default);
		maxusers = up->u_default;
	} else if (maxusers < up->u_min) {
		printf("minimum of %d maxusers assumed\n", up->u_min);
		maxusers = up->u_min;
	} else if (maxusers > up->u_max)
		printf("warning: maxusers > %d (%d)\n", up->u_max, maxusers);

	if (processors == 0 ) {
		printf("processors not specified; 1 assumed\n");
		processors = 1;
	}

	do_build("confdep.h", build_confdep);
	while (fgets(line, BUFSIZ, ifp) != 0) {
		if (*line == '%')
			goto percent;
		if (profiling && strncmp(line, "COPTS=", 6) == 0) {
			register char *cp;
                        fprintf(ofp,
				"GPROF.EX=%s/gmon.ex\n", machinename);
			cp = index(line, '\n');
			if (cp)
				*cp = 0;
			cp = line + 6;
			while (*cp && (*cp == ' ' || *cp == '\t'))
				cp++;
			COPTS = malloc((unsigned)(strlen(cp) + 1));
			if (COPTS == 0) {
				printf("config: out of memory\n");
				Exit(1);
			}
			strcpy(COPTS, cp);
			fprintf(ofp, "%s ${CCPROFOPT}\n", line);
			fprintf(ofp, "PCOPTS=%s\n", cp);
                       	continue;
		}
		if (profiling && strncmp(line, "ALLOPTS=", 8) == 0) {
			register char *cp;
			cp = index(line, '\n');
			if (cp)
				*cp = 0;
                        fprintf(ofp, "%s ${PROFOPTS}\n", line);
			continue;
		}
		fprintf(ofp, "%s", line);
		continue;
	percent:
		if (eq(line, "%OBJS\n"))
			do_objs(ofp, "OBJS=", -1, 0);
		else if (eq(line, "%CFILES\n")) {
			do_files(ofp, "CFILES1=", 'c', 0);
			do_objs(ofp, "COBJS=", 'c', 0);
		} else if (eq(line, "%SFILES\n")) {
			do_files(ofp, "SFILES=", 's', 1);
			do_objs(ofp, "SOBJS=", 's', 1);
		} else if (eq(line, "%BFILES\n"))
			do_files(ofp, "BFILES=", 'b', 0);
		else if (eq(line, "%MACHDEP\n")) {
			do_machdep(ofp);
			for (op = mkopt; op; op = op->op_next)
				fprintf(ofp, "%s=%s\n", op->op_name, op->op_value);
		}
		else if (eq(line, "%ORDERED\n"))
			do_ordered(ofp);
		else if (eq(line, "%DYNAMIC\n"))
			do_dynamic(ofp);
		else if (eq(line, "%RULES\n"))
			do_rules(ofp);
		else if (eq(line, "%LOAD\n"))
			do_load(ofp);
		else if (eq(line, "%MACHINE\n"))
			{
			if (eq(machinename, "PMAX") || (eq(machinename, "MIPS")))
				do_machine(ofp, "mips");
			else
				do_machine(ofp, machinename);
		        }
		else if (eq(line, "%TEXTBASE\n"))
			do_base_addr(ofp);
		else if (eq(line, "%KERNEL_LIBS\n"))
			do_set_libs(ofp);
		else if (eq(line, "%BINARY_DIR\n"))
			fprintf(ofp, "BINARY_DIR=%s\n", binary_dir);
		else
			fprintf(stderr,
			    "Unknown %% construct in generic makefile: %s",
			    line);
	}
	fprintf(ofp,"# DO NOT DELETE THIS LINE\n\n\n");
	(void) fclose(dfp);
	(void) fclose(ifp);
	(void) fclose(ofp);
}

save_dynamic(this, target)
char *this, *target;
{
	struct dynlist *d;
	int len;
	char *ptr, *sp, *cp;
	char och;
	extern char *realloc();

	for (d = dynamic_targets; d != NULL; d = d->next) {
		if (!eq(d->target, target))
			continue;
		sp = tail(this);
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		len = strlen(d->objects) + 1 + len + 1;
		ptr = realloc(d->objects, len);
		if (ptr == NULL) {
			fprintf(stderr, "objects realloc failed\n");
			Exit(1);
		}
		strcat(ptr, " ");
		strcat(ptr, sp);
		d->objects = ptr;
		*cp = och;
		return;
	}
	d = (struct dynlist *)malloc(sizeof(struct dynlist));
	if (d == NULL) {
		fprintf(stderr, "dynlist malloc failed\n");
		Exit(1);
	}
	d->next = dynamic_targets;
	dynamic_targets = d;
	d->target = ns(target);
	if (d->target == NULL) {
		fprintf(stderr, "target ns failed\n");
		Exit(1);
	}
	sp = tail(this);
	cp = sp + strlen(sp) - 1;
	och = *cp;
	*cp = 'o';
	d->objects = ns(sp);
	if (d->objects == NULL) {
		fprintf(stderr, "objects ns failed\n");
		Exit(1);
	}
	*cp = och;
}


save_audit(this, target)
char *this, *target;
{
	struct dynlist *d;
	int len;
	char *ptr, *sp, *cp;
	char och;
	extern char *realloc();

	for (d = audit_targets; d != NULL; d = d->next) {
		if (!eq(d->target, target))
			continue;
		sp = tail(this);
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		len = strlen(d->objects) + 1 + len + 1;
		ptr = realloc(d->objects, len);
		if (ptr == NULL) {
			fprintf(stderr, "objects realloc failed\n");
			Exit(1);
		}
		strcat(ptr, " ");
		strcat(ptr, sp);
		d->objects = ptr;
		*cp = och;
		return;
	}
	d = (struct dynlist *)malloc(sizeof(struct dynlist));
	if (d == NULL) {
		fprintf(stderr, "dynlist malloc failed\n");
		Exit(1);
	}
	d->next = audit_targets;
	audit_targets = d;
	d->target = ns(target);
	if (d->target == NULL) {
		fprintf(stderr, "target ns failed\n");
		Exit(1);
	}
	sp = tail(this);
	cp = sp + strlen(sp) - 1;
	och = *cp;
	*cp = 'o';
	d->objects = ns(sp);
	if (d->objects == NULL) {
		fprintf(stderr, "objects ns failed\n");
		Exit(1);
	}
	*cp = och;
}


/*
 * Read in information about existing file systems.
 */
read_filesystems()
{
	FILE *fp;
	char fsname[256], *wd, *name;
	struct file_sys *fs;

	(void) sprintf(fsname, "%s/filesystems", config_directory);
	fp = VPATHopen(fsname, "r");
	if (fp == NULL) {
		perror(fsname);
		Exit(1);
	}

	/* process each line */

	while ((wd = get_word(fp)) != (char *)EOF) {
		if (wd == NULL)
			continue;

		/* record each file system name */

		name = ns(wd);
		fs = (struct file_sys *)malloc(sizeof (struct file_sys));

		fs->fs_name = name;
		fs->fs_next = file_sys;
		file_sys = fs;
	}
	(void) fclose(fp);
}


/*
 * Read in the information about files used in making the system.
 * Store it in the ftab linked list.
 */
read_files()
{
#define	NAMESIZE	1024

	char fnamebuf[NAMESIZE], dnamebuf[NAMESIZE];
	char *entry, *rtn, *copy, *index;
	struct file_state fs;
	struct file_state dir_list;

	dynamic_targets = NULL;
	ftab = 0;
	fs.fs_fname = fnamebuf;
	fs.fs_directory = 0;
	dir_list.fs_fname = dnamebuf;
	dir_list.fs_directory = 0;

	/* Open the regular non-hardware dependant files file.  This files file
	 * should contain all the files that are common to all hardware platforms.
	 */
	(void) sprintf(fs.fs_fname, "%s/files", config_directory);
	fs.fs_fp = VPATHopen(fs.fs_fname, "r");
	if (fs.fs_fp == 0) {
		perror(fs.fs_fname);
		Exit(1);
	}
	for (;;) {
		if (read_file(&fs, 1))
			break;
	}
	(void) fclose(fs.fs_fp);

	/*
	 * Open the files file that is hardware platform specific.
	 */
	(void) sprintf(fs.fs_fname, "%s/%s/files", config_directory, machinename);
	fs.fs_fp = VPATHopen(fs.fs_fname, "r");
	if (fs.fs_fp == 0) {
		perror(fs.fs_fname);
		Exit(1);
	}
	for (;;) {
		if (read_file(&fs, 2))
			break;
	}
	(void) fclose(fs.fs_fp);

	/* Open a files file with an ident extention.  Ident is set by an entry in the 
	 * config file.  In the regular OSF case the config file has:
	 *	ident osf
	 * These files are files that would only get loaded for this system Identifier.
	 */
	(void) sprintf(fs.fs_fname, "files.%s", raise_to_upper(ident));
	fs.fs_fp = VPATHopen(fs.fs_fname, "r");
	if (fs.fs_fp != 0){
		for (;;) {
			if (read_file(&fs, 3))
				break;
		}
	}
	(void) fclose(fs.fs_fp);

	/* Open a files file used by 3ed party kernel code. 
	 * The path to these layered products 
	 * can be found in a registery file for all accessable layered 
	 * products. These files are files that would only get loaded 
	 *for this system layered product.
	 */
	(void) sprintf(dir_list.fs_fname, "./%s.list", PREFIX);
	dir_list.fs_fp = VPATHopen(dir_list.fs_fname, "r");
	if (dir_list.fs_fp == 0)
		return;
	for ( entry = malloc(sizeof(char) * 1024),
	     copy = malloc(sizeof(char) * 1024),
	     rtn = fgets(entry,1024,dir_list.fs_fp);
	     rtn != (char *) 0;
	     rtn = fgets(entry,1024,dir_list.fs_fp)) {
		/* OK now we have a line from the PREFIX.list file 
		 * which should point us to an
		 * area that MAY contain a files file and config file entries 
		 * for use when building THIS kernel.
		 */
		if(*entry != '#'){			/* Skip # comments */

			/* get first entry from colon/<CR> seperated string
			 */
			(void) strcpy(copy,entry);
			index = strtok(copy,":\n");

			/* Skip Blank Lines
			 */
			if( index == NULL )
				continue;
			/* clear the fname, since previous use may have left
			 * non-null values in the char array.
			 */
			bzero (fs.fs_fname, (sizeof(char) * NAMESIZE));
			(void) strncpy(fs.fs_fname, index, strlen(index)); 
			fs.fs_directory = ns(fs.fs_fname);
			(void) strcat(fs.fs_fname,"/files");
			fs.fs_fp = 0;
			fs.fs_fp = VPATHopen(fs.fs_fname, "r");
			if (fs.fs_fp != 0){
				for (;;) {
					if (read_file(&fs, 4)) 
						break;
				}
				(void) fclose(fs.fs_fp);
			}
		}
		bzero(entry,(sizeof(char) * 1024));
		bzero(copy,(sizeof(char) * 1024));
	}

}

/*
 * filename [ standard | optional ]
 *	[ config-dependent | ordered | sedit ]
 *	[ if_dynamic <module> | only_dynamic <module> ]
 *	[ dev* | profiling-routine ] [ device-driver]
 *	[Binary | Notbinary] [ floating-point] [ Unsupported ] 
 */
read_file(fs, pass)
struct file_state *fs;
int pass;
{
	struct file_list *tp, *pf;
	char *rest;
	int needndx;
	char *needs[NNEEDS];
	int isdup;

	for (needndx=0;needndx < NNEEDS;needndx++)
		needs[needndx]='\0';

	fs->fs_wd = get_word(fs->fs_fp);
	/* OK its the END so lets get out-a here
	 */
	if (fs->fs_wd == (char *)EOF)
		return(1);
	if (fs->fs_wd == 0)
		return(0);
	/* Ignore comments
	 */
	if (*fs->fs_wd == '#') {
		while (fs->fs_wd = get_word(fs->fs_fp))
			if (fs->fs_wd == (char *)EOF)
				break;
		return(0);
	}
	fs->fs_this = ns(fs->fs_wd);

	/* Is this file already in the list and not an optional kernel lib?
	 */
	if (fl_lookup(fs->fs_this) && strncmp(fs->fs_this, "LIB/", 4)) {
		printf("<Warning> %s: Duplicate file %s.\n",fs->fs_fname,fs->fs_this);
		while (fs->fs_wd = get_word(fs->fs_fp))
			if (fs->fs_wd == (char *)EOF)
				break;
		return(0);
	}

	next_word(fs->fs_fp, fs->fs_wd);
	if (fs->fs_wd == 0) {
		printf("%s: No type for %s.\n", fs->fs_fname, fs->fs_this);
		Exit(1);
	}
	if ((pf = fl_lookup(fs->fs_this)) &&
	    (pf->f_type != INVISIBLE || pf->f_flags))
		isdup = 1;
	else
		isdup = 0;
	tp = 0;
	if ((pass == 3 || pass == 4) && (tp = fltail_lookup(fs->fs_this)) != 0)
		printf("%s: Local file %s overrides %s.\n",
		    fs->fs_fname, fs->fs_this, tp->f_fn);
	fs->fs_dynamic = (char *)0;
	fs->fs_dev = (struct device_entry *)0;
	fs->fs_type = NORMAL;
	fs->fs_flags = SEDIT;
	fs->fs_define_dynamic = 0;
	rest = (char *)0;

	if (eq(fs->fs_wd, "standard")) {
		parse_standard(fs, &rest);
		save(fs, tp, pf, rest, needs);
		return(0);
	}

	if (!eq(fs->fs_wd, "optional")) {
		printf("%s: %s must be optional or standard\n",
		       fs->fs_fname, fs->fs_this);
		Exit(1);
	}

	if (parse_optional(fs, &rest, needs, isdup)) {
		/* save this file entry */
		save(fs, tp, pf, rest, needs);
		return(0);
	}

	/* mark the file invisible */
	while (fs->fs_wd  != 0){
		fs->fs_wd = get_word(fs->fs_fp);
		if(fs->fs_wd == (char *)EOF)
			break;
        }
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = fs->fs_this;
	tp->f_type = INVISIBLE;
	for (needndx=0;needndx < NNEEDS;needndx++)
		tp->f_needs[needndx] = needs[needndx];
	tp->f_flags = isdup;
	return(0);
}

save(fs, tp, pf, rest, needs)
struct file_state *fs;
struct file_list *tp, *pf;
char *rest, *needs[];
{
	int needndx = 0;

	if (fs->fs_type == SYSCALL)
		save_audit(fs->fs_this, fs->fs_syscall);
	if (fs->fs_type == PROFILING && !profiling)
		return;
	if (fs->fs_define_dynamic && fs->fs_dev)
		fs->fs_dev->d_flags++;
	if (fs->fs_dynamic && !want_dynamic(fs))
		return;
	if (tp == 0)
		tp = new_fent();
	tp->f_fn = fs->fs_this;
	tp->f_directory = fs->fs_directory;
	tp->f_extra = rest;
	tp->f_type = fs->fs_type;
	tp->f_flags = fs->fs_flags;
	tp->f_syscall = fs->fs_syscall;
	
	while(needs[needndx] != NULL){
		if( !load_needs(tp->f_needs,needs[needndx])){
			fprintf(stderr,"%s has to many dependancies\n",fs->fs_this);
			Exit(1);
		}
		needndx++;
        }
	if (pf && pf->f_type == INVISIBLE)
		pf->f_flags = 1;		/* mark as duplicate */
	return;
}

want_dynamic(fs)
struct file_state *fs;
{
	register struct device_entry *dp;

	for (dp = dtab; dp != 0; dp = dp->d_next) {
		if (dp->d_dynamic == 0)
			continue;
		if (dp->d_type == PSEUDO_DEVICE && dp->d_slave == 0)
			continue;
		if (eq(dp->d_dynamic, fs->fs_dynamic)) {
			save_dynamic(fs->fs_this, fs->fs_dynamic);
			fs->fs_flags |= (DYNAMICF | ORDERED);
			return(1);
		}
	}
	if (fs->fs_flags & DYNAMICF)	/* only_dynamic */
		return(0);
	return(1);
}

parse_standard(fs, restp)
struct file_state *fs;
char **restp;
{
	if (fs->fs_wd == 0)
		return;
	if (*fs->fs_wd == '|') {
		*restp = ns(get_rest(fs->fs_fp));
		return;
	}
	for (;;) {
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd == 0)
			return;
		if (!parse_opt(fs,restp))
			break;
	}
	*restp = ns(get_rest(fs->fs_fp));
	return;
}


parse_optional(fs, restp, needsp, isdup)
struct file_state *fs;
char **restp;
char *needsp[];
int isdup;
{
	int options;
	int libs = 0;
	int pass = 0;
	int logical_last = 0;
	char *logic[NNEEDS * 3];
	int i;

	for (i = 0; i < NNEEDS; i++) {
		logic[i] = NULL;
	}

	options = (strncmp(fs->fs_this, "OPTIONS/", 8) == 0);
	if (options)
		fs->fs_type = INVISIBLE;
	else {
		libs = (strncmp(fs->fs_this, "LIB/", 4) == 0);
		if (libs) {
			fs->fs_type = INVISIBLE;
		}
	}
	for (;;) {
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd == 0){
			if ( pass == 0 ) {
				printf("%s: what is %s optional on?\n",
				       fs->fs_fname, fs->fs_this);
				Exit(1);
			} else {
				/* All the needs have been 
				 * found by now so see if we want this file in the 
				 * Makefile.
				 */
				if ( !want_request(fs, logic) )
					return(0);
				else
					return(1);
		        }
		}else{
			pass++;
			if (parse_opt(fs, restp))
				continue;
			/* If there are more needs go get them.
			 */
			if ((eq(fs->fs_wd, "and")) || (eq(fs->fs_wd, "AND"))) {
				load_needs(logic,ns("and"));
				logical_last = 1;
				continue;
			}
			if ((eq(fs->fs_wd, "or")) || (eq(fs->fs_wd, "OR"))) {
				load_needs(logic,ns("or"));
				logical_last = 1;
				continue;
			}
			if (pass && !logical_last) {
				/*
				 * add the implied 'or'
				 */
				load_needs(logic,ns("or"));
			}
			if ((eq(fs->fs_wd, "not")) || (eq(fs->fs_wd, "NOT"))) {
				load_needs(logic,ns("not"));
				logical_last = 1;
				continue;
			}
			load_needs(needsp,ns(fs->fs_wd));
			load_needs(logic,ns(fs->fs_wd));
			if (logical_last) {
				logical_last = 0;
				continue;
			}
			if(libs)
				if(add_lib(fs, needsp) < 0)
					Exit(1);
			if (isdup)
				return(0);
			if (options)
				add_option(fs);
        	}
	}
}

match_need(fs, need)
    struct file_state *fs;
    char *need;
{
    struct device_entry *dp;
    struct opt *op;
    struct cputype *cp;

    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if (eq(dp->d_name, need) &&
	    (dp->d_type != PSEUDO_DEVICE || dp->d_slave))
	    return(1);
    }

    for (dp = dtab; dp != 0; dp = dp->d_next) {
	if (eq(dp->d_port_name, need) &&
	    (dp->d_type != PSEUDO_DEVICE || dp->d_slave))
	    return(1);
    }

    /*
     * The opt points to a link list of "option" entries found in the config
     * file by yacc in config.y
     */
    for (op = opt; op != 0; op = op->op_next) {
	if (op->op_value == 0 && opteq(op->op_name, need)) 
	    return(1);
    }

    /*
     * Walk through the cpu type structue looking for a match on the name
     * passed in.
     */
    for (cp = cputype; cp; cp = cp->cpu_next) {
	if (opteq(cp->cpu_name, need)) 
	    return(1);
    }

    return(0);
}

want_request(fs, needsp)
    struct file_state *fs;
    char *needsp[];
{
    int not_option = 0;
    int i;
    int want_it = 0;
    int bad_and = 0;

    for (i = 0; (needsp[i] != NULL) && (i < NNEEDS); i++) {
	if ((eq(needsp[i], "and")) || (eq(needsp[i], "AND"))) {
	    /*
	     * flag if the already processed part of this 'and'
	     * clause has failed
	     */
	    bad_and = !want_it;
	    continue;
	}
	if ((eq(needsp[i], "or")) || (eq(needsp[i], "OR"))) {
	    bad_and = 0;
	    if (want_it)
		break;
	    else
		continue;
	}
	if ((eq(needsp[i], "not")) || (eq(needsp[i], "NOT"))) {
	    not_option = 1;
	    continue;
	}

	if (match_need(fs, needsp[i]))
	    want_it = bad_and ? 0 : !not_option;
	else
	    want_it = bad_and ? 0 : not_option;

	not_option = 0;
    }

    return(want_it);
}

add_option(fs)
struct file_state *fs;
{
	register struct opt *op;
	struct opt *lop = 0;
	struct device_entry tdev;
	char *od;
	extern struct  device_entry * curp;


	/*
	 *  Allocate a pseudo-device entry which we will insert into
	 *  the device list below.  The flags field is set non-zero to
	 *  indicate an internal entry rather than one generated from
	 *  the configuration file.  The slave field is set to define
	 *  the corresponding symbol as 0 should we fail to find the
	 *  option in the option list.
	 */
	init_dev(&tdev);
	tdev.d_name = ns(fs->fs_wd);
	tdev.d_type = PSEUDO_DEVICE;
	tdev.d_flags++;
	tdev.d_slave = 0;
	od = raise_to_upper(ns(fs->fs_wd));

	for (op=opt; op; lop=op, op=op->op_next) {
		/*
		 *  Found an option which matches the current device
		 *  dependency identifier.  Set the slave field to
		 *  define the option in the header file.
		 */
		if (strcmp(op->op_name, od) == 0) {
			if (op->op_value)
				tdev.d_slave = atoi(op->op_value);
			else
				tdev.d_slave = 1;
			tdev.d_dynamic = op->op_dynamic;
			if (lop == 0)
				opt = op->op_next;
			else
				lop->op_next = op->op_next;
			free(op);
			op = 0;
		}
		if (op == 0)
			break;
	}
	free(od);
	newdev(&tdev);
	fs->fs_dev = curp;
}

parse_opt(fs,restp)
struct file_state *fs;
char **restp;
{
	if ((eq(fs->fs_wd, "config-dependent")) || (eq(fs->fs_wd, "CONFIG-DEPENDENT"))) {
		fs->fs_flags |= CONFIGDEP;
		return(1);
	}
	if ((eq(fs->fs_wd, "ordered")) || (eq(fs->fs_wd, "ORDERED"))) {
		fs->fs_flags |= ORDERED;
		return(1);
	}
	if ((eq(fs->fs_wd, "sedit")) || (eq(fs->fs_wd, "SEDIT"))) {
		fs->fs_flags |= SEDIT;
		return(1);
	}
	if ((eq(fs->fs_wd, "define_dynamic")) || (eq(fs->fs_wd, "DEFINE_DYNAMIC"))) {
		fs->fs_define_dynamic++;
		return(1);
	}
	if ((eq(fs->fs_wd, "if_dynamic")) || (eq(fs->fs_wd, "IF_DYNAMIC"))) {
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd == 0) {
			printf("%s: missing if_dynamic module for %s\n",
			       fs->fs_fname, fs->fs_this);
			Exit(1);
		}
		fs->fs_dynamic = ns(fs->fs_wd);
		return(1);
	}
	if ((eq(fs->fs_wd, "only_dynamic")) || (eq(fs->fs_wd, "ONLY_DYNAMIC"))) {
		fs->fs_flags |= DYNAMICF;
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd == 0) {
			printf("%s: missing only_dynamic module for %s\n",
			       fs->fs_fname, fs->fs_this);
			Exit(1);
		}
		fs->fs_dynamic = ns(fs->fs_wd);
		return(1);
	}
	/* syntax is "syscall prefix" where prefix is "std" for
	 * the standard syscall vector or otherwise the prefix name. 
	 * e.g., "ult_" */
	if (eq(fs->fs_wd, "syscall")) {
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd == 0) {
			printf("%s: missing syscall name for module  %s\n",
			       fs->fs_fname, fs->fs_this);
			Exit(1);
		}
		fs->fs_syscall = ns(fs->fs_wd);
		fs->fs_type = SYSCALL;
		return(1);
	}
	if ((eq(fs->fs_wd, "floating-point")) || (eq(fs->fs_wd, "FLOATING-POINT"))){
		fs->fs_type = FLOAT;
		return(1);
	}
	if ((eq(fs->fs_wd, "device-driver")) || (eq(fs->fs_wd, "DEVICE-DRIVER"))){
		fs->fs_type = DRIVER;
		return(1);
	}
	if ((eq(fs->fs_wd, "profiling-routine")) || (eq(fs->fs_wd, "PROFILING-ROUTINE"))){
		fs->fs_type = PROFILING;
		return(1);
	}
	if ((eq(fs->fs_wd, "Binary")) || (eq(fs->fs_wd, "BINARY")) ) {
		fs->fs_flags |= ISBINARY;
		return(1);
	}
	if ((eq(fs->fs_wd, "Notbinary"))|| (eq(fs->fs_wd, "NOTBINARY"))) {
		fs->fs_flags |= NOTBINARY;
		return(1);
	} 
	if ((eq(fs->fs_wd, "cpu")) || (eq(fs->fs_wd, "CPU")) ){
		return(1);
	}
	if ((eq(fs->fs_wd, "no_global_ptr")) || (eq(fs->fs_wd, "NO_GLOBAL_PTR")) ) {
		fs->fs_flags |= NO_GLOBAL_PTR;
		return(1);
	}
	if ((eq(fs->fs_wd, "Unsupported")) || (eq(fs->fs_wd, "UNSUPPORTED"))) {
		fs->fs_flags |= UNSUPPORTED;
		return(1);
	}
	if ( *fs->fs_wd == '|') {
		next_word(fs->fs_fp, fs->fs_wd);
		if (fs->fs_wd)
			*restp = ns(get_rest(fs->fs_fp));
		return(1);
	}

	return(0);
}

opteq(cp, dp)
	char *cp, *dp;
{
	char c, d;

	for (; ; cp++, dp++) {
		if (*cp != *dp) {
			c = isupper(*cp) ? tolower(*cp) : *cp;
			d = isupper(*dp) ? tolower(*dp) : *dp;
			if (c != d)
				return (0);
		}
		if (*cp == 0)
			return (1);
	}
}

do_objs(fp, msg, ext, anycase)
	FILE	*fp;
	char	*msg;
	int	ext;
	int	anycase;
{
	register struct file_list *tp, *fl;
	register int lpos, len;
	register char *cp, och, *sp;
	char swapname[32];

	fprintf(fp, "\n");
	fprintf(fp, msg);
	lpos = strlen(msg);
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE || tp->f_type == SYSCALL)
			continue;
		/*
		 *	Check for '.o' file in list
		 */
		cp = tp->f_fn + (len = strlen(tp->f_fn)) - 1;
		if ((ext == -1 && (tp->f_flags & ORDERED)) || /* not in objs */
		    (ext != -1 && *cp != ext &&
		     (!anycase || *cp != toupper(ext))))
			continue;
		else if (*cp == 'o') {
			if (len + lpos > 72) {
				lpos = 8;
				fprintf(fp, "\\\n\t");
			}
			fprintf(fp, "%s ", tp->f_fn);
			fprintf(fp, " ");
			lpos += len + 1;
			continue;
		}
		sp = tail(tp->f_fn);
		for (fl = conf_list; fl; fl = fl->f_next) {
			if (fl->f_type != SWAPSPEC)
				continue;
			if (eq(fl->f_fn, "generic") || eq(fl->f_fn, "boot"))
				sprintf(swapname, "swap.c");
			else
				sprintf(swapname, "swap%s,c", fl->f_fn);
			if (eq(sp, swapname))
				goto cont;
		}
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		if (tp->f_flags & UNSUPPORTED)
			printf("Warning this device is not supported by DIGITAL %s\n",sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}


/* not presently used and probably broken,  use ORDERED instead */
do_ordered(fp)
	FILE *fp;
{
	register struct file_list *tp;
	register int lpos, len;
	register char *cp, och, *sp;
	char swapname[32];

	fprintf(fp, "ORDERED=");
	lpos = 10;
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if ((tp->f_flags & ORDERED) != ORDERED)
			continue;
		if ((tp->f_flags & DYNAMICF) == DYNAMICF)
			continue;
		sp = tail(tp->f_fn);
		cp = sp + (len = strlen(sp)) - 1;
		och = *cp;
		*cp = 'o';
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(fp, "\\\n\t");
		}
		fprintf(fp, "%s ", sp);
		lpos += len + 1;
		*cp = och;
cont:
		;
	}
	if (lpos != 8)
		putc('\n', fp);
}


do_dynamic(fp)
	FILE *fp;
{
	register struct dynlist *dp;

	for (dp = dynamic_targets; dp != 0; dp = dp->next) {
		fprintf(fp, "%s: %s_kmod\n\n", dp->target, dp->target);
		fprintf(fp, "%s_kmod: %s\n", dp->target, dp->objects);
		fprintf(fp, "\t${DLD} ${DLD_FLAGS} -e %s_configure -o %s_kmod %s ${VMUNIX_LIB}\n\n", dp->target, dp->target, dp->objects);
	}
}

do_files(fp, msg, ext, anycase)
	FILE	*fp;
	char	*msg;
	char	ext;
	int	anycase;
{
	register struct file_list *tp;
	register int lpos;
	int count = 0;
	int total = 0;
	int partition;

	/*
	 * count the number of files
	 */
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;
		if ( (!source) && (tp->f_flags & ISBINARY) )
			continue;
		if (tp->f_fn[strlen(tp->f_fn)-1] != ext &&
		    (!anycase || tp->f_fn[strlen(tp->f_fn)-1] != toupper(ext)))
			continue;

		++total;
	}
	partition = total / 4;

	fprintf(fp, msg);
	lpos = 8;
	for (tp = ftab; tp != 0; tp = tp->f_next) {
		if (tp->f_type == INVISIBLE)
			continue;
		if ( (!source) && (tp->f_flags & ISBINARY) )
			continue;

		if (tp->f_fn[strlen(tp->f_fn)-1] != ext &&
		    (!anycase || tp->f_fn[strlen(tp->f_fn)-1] != toupper(ext)))
			continue;
		/*
		 * Always generate a newline.
		 * Our Makefile's aren't readable anyway.
		 */

		lpos = 8;
		if(tp->f_directory != 0)
			fprintf(fp, "\\\n\t%s/%s ",tp->f_directory,  tp->f_fn);
		else
			fprintf(fp, "\\\n\t%s ", tp->f_fn);
		lpos += 1;

		++count;

		/*
		 * temporary fix for ode: split the CFILES list into chunks
		 */
		if (ext == 'c') {
		    if (count == partition)
		        fprintf(fp, "\n\nCFILES2=");
		    if (count == 2 * partition)
		        fprintf(fp, "\n\nCFILES3=");
		    if (count == 3 * partition)
		        fprintf(fp, "\n\nCFILES4=");
		}
	}
	if (lpos != 8)
		putc('\n', fp);
}

/*
 *  Include machine dependent makefile in output
 */

do_machdep(ofp)
	FILE *ofp;
{
	FILE *ifp;
	char line[BUFSIZ];

	(void) sprintf(line, "%s/%s/template.mk", config_directory, machinename);
	ifp = VPATHopen(line, "r");
	if (ifp == 0) {
		perror(line);
		Exit(1);
	}
	while (fgets(line, BUFSIZ, ifp) != 0) {
		fputs(line, ofp);
	}
	fclose(ifp);
}



/*
 *  Format configuration dependent parameter file.
 */

build_confdep(fp)
	FILE *fp;
{
	fprintf(fp,
	    "#define TIMEZONE %d\n#define MAXUSERS %d\n#define DST %d\n",
	    timezone, maxusers, dst);
	fprintf(fp, "#define SYS_V_MODE %d\n", sys_v_mode);
	if (maxdsiz)
	    fprintf(fp, "#define MAXDSIZ %d\n", maxdsiz);
	if (maxssiz)
	    fprintf(fp, "#define MAXSSIZ %d\n", maxssiz);
	if (dfldsiz)
	    fprintf(fp, "#define DFLDSIZ %d\n", dfldsiz);
	if (dflssiz)
	    fprintf(fp, "#define DFLSSIZ %d\n", dflssiz);

	if (msgmax)
		fprintf(fp, "#define MSGMAX %d\n", msgmax);
	if (msgmnb)
		fprintf(fp, "#define MSGMNB %d\n", msgmnb);
	if (msgmni)
		fprintf(fp, "#define MSGMNI %d\n", msgmni);
	if (msgtql)
		fprintf(fp, "#define MSGTQL %d\n", msgtql);
	if (semmni)
		fprintf(fp, "#define SEMMNI %d\n", semmni);
	if (semmns)
		fprintf(fp, "#define SEMMNS %d\n", semmns);
	if (semmsl)
		fprintf(fp, "#define SEMMSL %d\n", semmsl);
	if (semopm)
		fprintf(fp, "#define SEMOPM %d\n", semopm);
	if (semume)
		fprintf(fp, "#define SEMUME %d\n", semume);
	if (semvmx)
		fprintf(fp, "#define SEMVMX %d\n", semvmx);
	if (semaem)
		fprintf(fp, "#define SEMAEM %d\n", semaem);
	if (shmmin)
		fprintf(fp, "#define SHMMIN %d\n", shmmin);
	if (shmmax)
		fprintf(fp, "#define SHMMAX %d\n", shmmax);
	if (shmmni)
		fprintf(fp, "#define SHMMNI %d\n", shmmni);
	if (shmseg)
		fprintf(fp, "#define SHMSEG %d\n", shmseg);

	/*
	 * Don't put a default MAXUPRC into condep.h if it is not 
	 * specified in the config file.  This will allow it to get 
	 * picked up from sys/limits.h (POSIX specifies this).
	 */
	if (maxuprc != 0) {
	    fprintf(fp, "#define MAXUPRC %d\n", maxuprc);
        }
	if (maxproc != 0) {
	    fprintf(fp, "#define MAXPROC %d\n", maxproc);
        }
	if (maxthreads != 0) {
	    fprintf(fp, "#define MAXTHREADS %d\n", maxthreads);
        }
	if (threadmax != 0) {
	    fprintf(fp, "#define THREADMAX %d\n", threadmax);
        }
	if (nclist != 0) {
	    fprintf(fp, "#define NCLIST %d\n", nclist);
        }
	if (taskmax != 0) {
	    fprintf(fp, "#define TASKMAX %d\n", taskmax);
        }
	if (maxcallouts != 0) {
	    fprintf(fp, "#define MAXCALLOUTS %d\n", maxcallouts);
        }
	if (bufcache != 0) {
	    fprintf(fp, "#define BUFCACHE %d\n", bufcache);
        }
	if (ubcminpercent != 0) {
	    fprintf(fp, "#define UBCMINPERCENT %d\n", ubcminpercent);
	}
	if (ubcmaxpercent != 0) {
	    fprintf(fp, "#define UBCMAXPERCENT %d\n", ubcmaxpercent);
	}
	if (writeio_kluster != 0) {
	    fprintf(fp, "#define WRITEIO_KLUSTER %d\n", writeio_kluster);
	}
	if (readio_kluster != 0) {
	    fprintf(fp, "#define READIO_KLUSTER %d\n", readio_kluster);
        }
	if (cowfaults != 0) {
	    fprintf(fp, "#define COWFAULTS %d\n", cowfaults);
	}
	if (mapentries != 0) {
	    fprintf(fp, "#define MAPENTRIES %d\n", mapentries);
	}
	if (maxvas != 0) {
	    fprintf(fp, "#define MAXVAS %ld\n", maxvas);
	}
	if (maxwire != 0) {
	    fprintf(fp, "#define MAXWIRE %ld\n", maxwire);
	}
	if (heappercent != 0) {
	    fprintf(fp, "#define HEAPPERCENT %d\n", heappercent);
	}
	if (anonklshift != 0) {
	    fprintf(fp, "#define ANONKLSHIFT %d\n", anonklshift);
	}
	if (anonklpages != 0) {
	    fprintf(fp, "#define ANONKLPAGES %d\n", anonklpages);
	}
	if (vpagemax != 0) {
	    fprintf(fp, "#define VPAGEMAX %d\n", vpagemax);
	}
	if (segmentation != 0) {
	    fprintf(fp, "#define SEGMENTATION %d\n", segmentation - 1);
	}
	if (ubcpagesteal != 0) {
	    fprintf(fp, "#define UBCPAGESTEAL %d\n", ubcpagesteal);
	}
	if (ubcdirtypercent != 0) {
	    fprintf(fp, "#define UBCDIRTYPERCENT %d\n", ubcdirtypercent);
	}
	if (ubcseqstartpercent != 0) {
	    fprintf(fp, "#define UBCSEQSTARTPERCENT %d\n", ubcseqstartpercent);
	}
	if (ubcseqpercent != 0) {
	    fprintf(fp, "#define UBCSEQPERCENT %d\n", ubcseqpercent);
	}
	if (csubmapsize != 0) {
	    fprintf(fp, "#define CSUBMAPSIZE %d\n", csubmapsize);
	}
	if (ubcbuffers != 0) {
	    fprintf(fp, "#define UBCBUFFERS %d\n", ubcbuffers);
	}
	if (swapbuffers != 0) {
	    fprintf(fp, "#define SWAPBUFFERS %d\n", swapbuffers);
	}
	if (clustermap != 0) {
	    fprintf(fp, "#define CLUSTERMAP %d\n", clustermap);
	}
	if (clustersize != 0) {
	    fprintf(fp, "#define CLUSTERSIZE %d\n", clustersize);
	}
        if (zone_size != 0) {
            fprintf(fp, "#define ZONE_SIZE %d\n", zone_size);
        }
        if (kentry_zone_size != 0) {
            fprintf(fp, "#define KENTRY_ZONE_SIZE %d\n", kentry_zone_size);
        }
        if (syswiredpercent != 0) {
            fprintf(fp, "#define SYSWIREDPERCENT %d\n", syswiredpercent);
	  }

}



/*
 *  Format cpu types file.
 */

build_cputypes(fp)
	FILE *fp;
{
	struct cputype *cp;

	for (cp = cputype; cp; cp = cp->cpu_next)
		fprintf(fp, "#define\t%s\t1\n", cp->cpu_name);
}



/*
 *  Build a define parameter file.  Create it first in a temporary location and
 *  determine if this new contents differs from the old before actually
 *  replacing the original (so as not to introduce avoidable extraneous
 *  compilations).
 */

do_build(name, format)
	char *name;
	int (*format)();
{
	static char temp[]="#config.tmp";
	FILE *tfp, *ofp;
	int c;

	tfp = fopen(path(temp), "w+");
	if (tfp == 0) {
		perror(path(temp));
		Exit(1);
	}
	unlink(path(temp));
	(*format)(tfp);
	ofp = fopen(path(name), "r");
	if (ofp != 0)
	{
		fseek(tfp, 0, 0);
		while ((c = fgetc(tfp)) != EOF)
			if (fgetc(ofp) != c)
				goto copy;
		if (fgetc(ofp) == EOF)
			goto same;
		
	}
copy:
	if (ofp)
		fclose(ofp);
	ofp = fopen(path(name), "w");
	if (ofp == 0) {
		perror(path(name));
		Exit(1);
	}
	fseek(tfp, 0, 0);
	while ((c = fgetc(tfp)) != EOF)
		fputc(c, ofp);
same:
	fclose(ofp);
	fclose(tfp);
}

/* return the file name portion of the full path name passed in.
 */
char *
tail(fn)
	char *fn;
{
	register char *cp;

	cp = rindex(fn, '/');
	if (cp == 0)
		return (fn);
	return (cp+1);
}


/*
 * Create the makerules for each file
 * which is part of the system.
 * Devices are processed with the special c2 option -i
 * which avoids any problem areas with i/o addressing
 * (e.g. for the VAX); assembler files are processed by as.
 */
do_rules(f)
	FILE *f;
{
	char *cp, *np, och, *tp;
	struct file_list *ftp;
	char *extras;
	char *kcc;
	char *kcc_nflags;
	char *kcc_dflags;
	char *kcc_pflags;
	char *kcc_fflags;
	char *nl = "; \\";
	

for (ftp = ftab; ftp != 0; ftp = ftp->f_next) {
	if (ftp->f_type == INVISIBLE)
		continue;
	if (( source) && (ftp->f_flags & NOTBINARY) )
		continue;
	cp = (np = ftp->f_fn) + strlen(ftp->f_fn) - 1;
	och = *cp;
	/*
	 *	Don't compile '.o' files
	 */
	if (och == 'o')
		continue;
	/*
	 *	Determine where sources should come from
	 */
	if ((np[0] == '.') && ( np[1] == '/'))
		np += 2;
	*cp = '\0';
	tp = tail(np);
	if( ftp->f_type != SYSCALL){
	    if ( (source) || (ftp->f_flags & NOTBINARY ) ) {
		if(ftp->f_directory != 0)
			fprintf(f, "%so: %s/%s%c\n", tp, ftp->f_directory, np, och);
		else
			fprintf(f, "%so: %s%c\n", tp, np, och);
	    } else
		fprintf(f, "%so:\n", tp);
	    if (och == 's') {
		if ( (source) || (ftp->f_flags & NOTBINARY ))
		{
	 		fprintf(f, "\t@${RM} %so\n", tp);
	 		if(ftp->f_directory != 0){
	 			fprintf(f, "\t${KCC} ${CCASFLAGS} %s %s/%ss\n\n",
 				    (ftp->f_extra?ftp->f_extra:""),  
 				    ftp->f_directory, np);
	 		} else if (ftp->f_type == FLOAT) {
	  			fprintf(f, "\t${KCC} ${CCASFFLAGS} %s %ss\n\n",
 			 	    (ftp->f_extra?ftp->f_extra:""),  np);
	 		} else {
	  			fprintf(f, "\t${KCC} ${CCASFLAGS} %s %ss\n\n",
 			 	    (ftp->f_extra?ftp->f_extra:""),  np);
			}
			continue;
		}
	    }
	    if (och == 'b') {
		fprintf(f, "\t${B_RULE_1A}%.*s${B_RULE_1B}\n\n", 
			tp-np, np);
		continue;
	    }
	    if (ftp->f_flags & CONFIGDEP) {
	      fprintf(stderr,
	     "config: %s%c: \"config-dependent\" obsolete; include \"confdep.h\" instead\n", np, och);
	    }
	    else
		extras = "";
	    if (ftp->f_flags & DYNAMICF) {
		kcc = "${KCC}";
		kcc_nflags = "${DCC_NFLAGS}";
		kcc_dflags = "${DCC_DFLAGS}";
		kcc_pflags = "${DCC_PFLAGS}";
		kcc_fflags = "${DCC_FFLAGS}";
	    } else {
		kcc = "${KCC}";
		kcc_nflags = "${CCNFLAGS}";
		kcc_dflags = "${CCDFLAGS}";
		kcc_pflags = "${CCPFLAGS}";
		kcc_fflags = "${CCFFLAGS}";
	    }
	}

	switch (ftp->f_type) {

	case FLOAT:
		switch (machine) {

		case MACHINE_DEC_RISC:
		case MACHINE_ALPHA:
			fprintf(f, "\t@${RM} %so\n", tp);
			if(( source ) || ( ftp->f_flags & NOTBINARY )) {
			       if(ftp->f_directory != 0){
				       fprintf(f, "\t%s %s%s%s -I %s %s%s%s/%sc\n\n", 
					       kcc, kcc_fflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					       ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
					       ftp->f_directory,
					       (ftp->f_extra?ftp->f_extra:""), extras, ftp->f_directory, np);
			       }else{
				       fprintf(f, "\t%s %s%s%s %s%s%sc\n\n", 
					       kcc, kcc_fflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					       ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
					       (ftp->f_extra?ftp->f_extra:""), extras, np);
			       }
				       
			}
			else
				if(ftp->f_directory != 0){
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
						ftp->f_directory, tp, tp);
				}else{
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
						binary_dir, tp, tp);
				}


			continue;
		default:
			goto common;
		}
		break;

	case NORMAL:
		switch (machine) {

		case MACHINE_DEC_RISC:
		case MACHINE_ALPHA:
			fprintf(f, "\t@${RM} %so\n", tp);
			if(( source ) || ( ftp->f_flags & NOTBINARY )) {
			       if(ftp->f_directory != 0){
				       fprintf(f, "\t%s %s%s%s -I %s %s%s%s/%sc\n\n", 
					       kcc, kcc_nflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					       ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
					       ftp->f_directory,
					       (ftp->f_extra?ftp->f_extra:""), extras, ftp->f_directory, np);
			       }else{
				       fprintf(f, "\t%s %s%s%s %s%s%sc\n\n", 
					       kcc, kcc_nflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					       ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
					       (ftp->f_extra?ftp->f_extra:""), extras, np);
			       }
				       
			}
			else
				if(ftp->f_directory != 0){
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
						ftp->f_directory, tp, tp);
				}else{
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
						binary_dir, tp, tp);
				}


			continue;
		default:
			goto common;
		}
		break;
	case SYSCALL:
		switch (machine) {

		case MACHINE_DEC_RISC:
		case MACHINE_ALPHA:
			if( (( source ) || ( ftp->f_flags & NOTBINARY)))
				make_loadable_syscall(f,ftp);
			break;
		default:
			goto common;
			break;
		}
		break;

	case DRIVER:
		switch (machine) {

		case MACHINE_DEC_RISC:
		case MACHINE_ALPHA:
			fprintf(f, "\t@${RM} %so\n", tp);
			if( source  || ( ftp->f_flags & NOTBINARY )) {
				if(ftp->f_directory != 0){
					fprintf(f, "\t%s -I %s %s%s%s%s %s%s/%sc\n\n", 
						kcc, ftp->f_directory, kcc_dflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					        ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
						(ftp->f_extra?ftp->f_extra:""), extras, ftp->f_directory, np);
				}else{
					fprintf(f, "\t%s %s%s%s%s %s%sc\n\n", 
						kcc, kcc_dflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
					        ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
						(ftp->f_extra?ftp->f_extra:""), extras, np);
				}
			}
			else
				if( ftp->f_directory != 0 ){
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
						ftp->f_directory, tp, tp);
				}else{
					fprintf(f, "\t@${LN} -s %s/%so %so\n\n", 
					        binary_dir, tp, tp);
				}
			continue;
		default:
			extras = "_D";
			goto common;
		}
		break;

	case PROFILING:
		if (!profiling)
			continue;
		if (COPTS == 0) {
			fprintf(stderr,
			    "config: COPTS undefined in generic makefile");
			COPTS = "";
		}
		switch (machine) {
		        case MACHINE_DEC_RISC:
		        case MACHINE_ALPHA:
				fprintf(f, "\t@${RM} %so\n", tp);
				fprintf(f, "\t%s %s%s%s %s%s%sc\n\n", 
				    kcc, kcc_pflags, ((ftp->f_flags & ISBINARY)?" -DBINARY":""),
				    ((ftp->f_flags & NO_GLOBAL_PTR)?" -G 0":""),
				    (ftp->f_extra?ftp->f_extra:""), extras, np);
				continue;
				break;
			default:
			fprintf(stderr,
			    "config: don't know how to profile kernel on this cpu\n");
			break;
		}

	common:
		if (ftp->f_flags & DYNAMICF)	/* dynamic module rule extra */
			extras = "_DYN";
		fprintf(f, "\t${C_RULE_1A%s}", extras);
		if (ftp->f_flags & OBJS_ONLY)
			fprintf(f, " -DBINARY");
		if (ftp->f_extra)
			fprintf(f, "%s", ftp->f_extra);
		fprintf(f, " %.*s${C_RULE_1B%s}%s\n", (tp-np), np, extras, nl);
		fprintf(f, "\t${C_RULE_2%s}%s\n", extras, nl);
		fprintf(f, "\t${C_RULE_3%s}%s\n", extras, nl);
		fprintf(f, "\t${C_RULE_4%s}\n\n", extras);
		break;

	default:
		printf("Don't know rules for %s\n", np);
		break;
	}
	*cp = och;
}
}

/*
 * Create the load strings
 */
do_load(f)
	register FILE *f;
{
	register struct file_list *fl;
	int first = 1;
	register struct dynlist *dp;
	register int lpos, len;
	register char *sp;
	struct file_list *do_systemspec();

	fl = conf_list;
	while (fl) {
		if (fl->f_type != SYSTEMSPEC) {
			fl = fl->f_next;
			continue;
		}
		fl = do_systemspec(f, fl, first);
		if (first)
			first = 0;
	}
	fprintf(f, "LOAD=");
	for (fl = conf_list; fl != 0; fl = fl->f_next)
		if (fl->f_type == SYSTEMSPEC)
			fprintf(f, " %s", fl->f_needs[0]);
	fprintf(f, "\n\nDYNAMIC=");
	lpos = 10;
	for (dp = dynamic_targets; dp != 0; dp = dp->next) {
		sp = dp->target;
		len = strlen(sp);
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(f, "\\\n\t");
		}
		fprintf(f, "%s ", sp);
		lpos += len + 1;
	}
	if (lpos != 8)
		putc('\n', f);
	fprintf(f, "\n\nSYSENT=");
	lpos = 10;
	for (dp = audit_targets; dp != 0; dp = dp->next) {
		sp = dp->target;
		len = strlen(sp)+13/*init_sysent.c*/;
		if(strcmp(sp,"std")==0)
			*sp = 0;
		if (len + lpos > 72) {
			lpos = 8;
			fprintf(f, "\\\n\t");
		}
		fprintf(f, "%sinit_sysent.c ", sp);
		lpos += len + 1;
	}
	if (lpos != 8)
		putc('\n', f);
	fprintf(f, "\nall: ${LOAD} ${DYNAMIC}\n");
	fprintf(f, "\n");
}

do_macros(f)
	register FILE *f;
{
	register struct opt *op;

	for (op = mkopt; op; op = op->op_next)
		fprintf(f, "%s=%s\n", op->op_name, op->op_value);
}

struct file_list *
do_systemspec(f, fl, first)
	FILE *f;
	register struct file_list *fl;
	int first;
{
	if (!source) {
	    fprintf(f, "%s .ORDER: %s.sys ${SYSDEPS}\n",
		fl->f_needs[0], fl->f_needs[0]);
	    fprintf(f, "\t${SYS_RULE_1}\n");
	    fprintf(f, "\t${SYS_RULE_2}\n");
	    fprintf(f, "\t${SYS_RULE_3}\n");
	    fprintf(f, "\t${SYS_RULE_4}\n\n");
	} else {
	    fprintf(f, "%s .ORDER: migfiles migdebugfiles %s.sys ${SYSDEPS}\n",
		fl->f_needs[0], fl->f_needs[0]);
	}
	do_swapspec(f, fl->f_fn, fl->f_needs[0]);
	for (fl = fl->f_next; fl != NULL && fl->f_type == SWAPSPEC; fl = fl->f_next)
		continue;
	return (fl);
}

do_swapspec(f, name, system)
 	char *system;
 	FILE *f;
 	char *name;
{
	extern char* swapgeneric_dir;
	char swapname[256];
	char *gdir= (eq(name, "generic") || eq(name,"boot")) ? "${cpu}/" : "";
	
	/* If we are not building from source we need to look for the swap file
	 * in one of two places.  For swap on generic the swapgenric.c file is
	 * in the /usr/sys/... tree as reflected by the real source tree.  If
	 * its a non-generic swap device ie. rz0b then we build a swap.c file
	 * in the builds directory.
	 */
	if( ( eq(name, "generic")) || (eq(name, "boot")) )
		sprintf(swapname,"%s","generic");
	else
		sprintf(swapname,"%s",name);
	if( (!source) && (( eq(name, "generic")) || (eq(name, "boot")) )){
		gdir= swapgeneric_dir;
	}else{
		if( !source )
			*gdir='\0';
	}

	fprintf(f, "%s.sys:${P} ${PRELDDEPS} ${LDOBJS} ${LOBJS} ${LDDEPS}\n\n", system);
	if (!source) {
	    fprintf(f, "%s.swap: swap%s.o\n", system, swapname);
	    fprintf(f, "\t@cp swap%s.o $@\n\n", swapname);
	    fprintf(f, "swap%s.o: %sswap%s.c ${SWAPDEPS}\n", swapname, gdir, swapname);
	    fprintf(f, "\t@${RM} swap%s.o\n", swapname);
	    fprintf(f, "\t${KCC} ${CCSFLAGS} %sswap%s.c\n\n", gdir, swapname);
	}
}

char *
raise_to_upper(str)
	register char *str;
{
	register char *cp = str;

	while (*str) {
		if (islower(*str))
			*str = toupper(*str);
		str++;
	}
	return (cp);
}

#define LINESIZE 1024
static char makbuf[LINESIZE];		/* one line buffer for makefile */

FILE *
VPATHopen(file, mode)
register char *file, *mode;
{
	register char *nextpath,*nextchar,*fname,*lastchar;
	char fullname[BUFSIZ];
	FILE *fp;

	nextpath = ((*file == '/') ? "" : search_path);
	do {
		fname = fullname;
		nextchar = nextpath;
		while (*nextchar && (*nextchar != ':'))
			*fname++ = *nextchar++;
		if (nextchar != nextpath && *file) *fname++ = '/';
		lastchar = nextchar;
		nextpath = ((*nextchar) ? nextchar + 1 : nextchar);
		nextchar = file;	/* append file */
		while (*nextchar)  *fname++ = *nextchar++;
		*fname = '\0';
		fp = fopen (fullname, mode);
	} while (fp == NULL && (*lastchar));
	return (fp);
}

/* Check through the device list looking for the device name passed in. 
 * return TRUE if its found.
 */
isconfigured(device)
	register char *device;
{
	register struct device_entry *dp;

	for (dp = dtab; dp != 0; dp = dp->d_next)
		if (dp->d_unit != -1 && eq(dp->d_name, device)) 
			return(1);
	return(0);
}

/* Load the needs array in the file list.  If its full return 0 if we can load
 * it return 1.
 * 
 * needs and string are arrays of strings oboth the same size.
 */
load_needs(needs, string)
char 	*string;
char	*needs[];

{
	int index;

	for(index = 0; index < NNEEDS; index++) {

		if ( needs[index] == NULL) {
			needs[index] = string;
			return(1);
		}
	}
	return(0);
}

free_needs(needs)
char *needs[];
{
	int index;
	for(index = 0; index < NNEEDS; index++) {

		if ( needs[index] != NULL) {
				free(needs[index]);
				needs[index] = NULL;
		}
	}
}

swaptype()
{
	register struct file_list *fl;

	for (fl = conf_list; fl; fl = fl->f_next) {
		if (fl->f_type != SWAPSPEC)
			continue;
		if (eq(fl->f_fn, "generic"))
			return (1);
		if (eq(fl->f_fn, "boot"))
			return (2);
		return (0);
	}
}

/*
 * Set up a kernel library to use.  There are a couple of rules in this area
 * that must be followed.
 *	1) All kernels will work with the lib_${machine}.a library specified
 *	   and the generic hardware abstraction library: lib_hal.a
 *	   This is configured by default:
 *	2) Only one cpu type may be specified in the 'config file' to get
 *	   a kernel library.  More than one cpu specified will default to only
 *	   the generic libraries -- lib_${machine}.a , lib_hal.a being used.
 */

do_set_libs(f)
register FILE *f;
{
	struct cpu_lib *lib_ptr;
	struct lib_req *wk;
	register struct cputype *cp;
	int cpu_index = 0;
	char *layer = "hal";
	char *lib_head = "lib";
	int found = FALSE;

	for (cp = cputype ; cp; cp = cp->cpu_next) {
		cpu_index++;
	}

	/* Build LIBS make(1) variable */

	fprintf(f, "\nLIBS\t\t= \\\n");
	if(cpu_index == SINGLE_CPU) {
		for (lib_ptr = cpu_lib_head; lib_ptr != NULL; lib_ptr = lib_ptr->next){
			for(wk = lib_ptr->lib_req; wk; wk = wk->next) {
	
				/* find match between cpu type and depend list entry */
	
				if(!strcmp(wk->depend, cputype->cpu_name)) {

				/*
				 * leave found, now unused, but may be needed for
				 * future warning messages.
				 */ 
					found = TRUE;
					fprintf(f,"\t%s/%s/%s/%s/lib_%s.O \\\n", 
						binary_dir, lib_head, machinename,
						lib_ptr->lib, lib_ptr->lib);
					break;
				}
			}
		}
	}

	/*
	 * Output to kernel Makefile the default libraries.  We always allow machine
	 * name as a generic default along the with hardware abstraction layer library: 
	 * lib_hal.a
	 */

#ifdef HAL_LIB
	fprintf(f, "\t-l_%s -l_%s -l_%s \n", layer, machinename, layer);
#else
	fprintf(f, "\t-l_%s  \n", machinename);
#endif

	/* Build library search path make(1) variable, LIB_SEARCH */

	fprintf(f, "\nLIB_SEARCH\t= \\\n");
#ifdef PATH_IN
	if(cpu_index == SINGLE_CPU) {
		for (lib_ptr = cpu_lib_head; lib_ptr != NULL; lib_ptr = lib_ptr->next){
			for(wk = lib_ptr->lib_req; wk; wk = wk->next) {
				if(!strcmp(wk->depend, cputype->cpu_name)) {
					fprintf(f,"\t-L%s/%s/%s/%s \\\n", binary_dir, lib_head,
						machinename, lib_ptr->lib);
					break;
				}
			}
		}

	}
#endif

	/*
	 * Output to kernel Makefile the default library search path.
	 */

#ifdef HAL_LIB
	fprintf(f, " \t-L%s/%s/%s/%s \\\n\t-L%s/%s/%s \n", binary_dir, lib_head, machinename, layer,
		binary_dir, lib_head, machinename);
#else
	fprintf(f, " \t-L%s/%s/%s/%s \n", binary_dir, lib_head, machinename);
#endif

	if(cpu_index > SINGLE_CPU) 
		fprintf(stderr, "  <Note> multiple cpus defined-building generic kernel\n");
	
}

/*
 * A line in the files file starts with LIB/ so we are going to
 * add the string following to the list of optional libraries. Each 
 * library list member has its own seperate list of cpu dependancies
 * listed in the files file. 
 */

add_lib(fs, needspl)
struct file_state *fs;
char *needspl[];
{
	char string[LIB_MAX];
	struct cpu_lib *clib_ptr, *wk, *prev;
	int found = FALSE;


	bzero(string, LIB_MAX);

	/* obtain the specific information part of the lib string */

	lib_root((ns(fs->fs_this)), string);

	/* allocate and load the library list entry */

	if((clib_ptr = (struct cpu_lib *)malloc(sizeof(struct cpu_lib))) 
		== (struct cpu_lib *)NULL)
		return LERROR;
	else { 
		strcpy(clib_ptr->lib, string);
		clib_ptr->next = NULL;
		clib_ptr->lib_req = NULL;
	}

	if(cpu_lib_head == (struct cpu_lib *)NULL) {   /* if list of libs is empty */
		cpu_lib_head = cpu_lib_ptr = clib_ptr;
		if(add_lib_req(clib_ptr, needspl) < 0)
			return LERROR;
	} else {   		 /* at least one list entry */
		
		for(wk = prev = cpu_lib_head ; wk; prev = wk, found = FALSE, wk = wk->next) {
			if(!strcmp(wk->lib, string)) {
				if(add_lib_req(wk, needspl) < 0)
					return LERROR;
				found = TRUE;
				break;
			}
		}
		if(found == FALSE) {
			if(add_lib_req(clib_ptr, needspl) < 0)
				return LERROR;
			prev->next = clib_ptr;
		}
	}
	return SUCCESS;

}

/*
 *	Add an optional cpu requirement to a library. Look through
 *	current array of needs and determine if a given need in
 * 	the array has been added to the library need list.
 *	If the need has been not been yet added, add the requirement
 *	to the list-else do not add to list.
 */

add_lib_req(lptr, np)
struct cpu_lib *lptr;
char *np[];
{
	int i = 0;
	struct lib_req *wk, *deptr, *prev;
	int found = FALSE;

	for(i = 0; i < NNEEDS && np[i] != NULL; i++) {

		for (wk = prev = lptr->lib_req; wk; prev = wk, wk = wk->next, found = FALSE)
			if(!strcmp(np[i], wk->depend)) {
				found = TRUE;	
				break;
			}
		if(found == FALSE) {
			if((deptr = (struct lib_req *)malloc(sizeof(struct lib_req)))
				 == (struct lib_req *)NULL)
				return LERROR;
			else {
				deptr->next = NULL;
				strcpy(deptr->depend, np[i]);
				if(lptr->lib_req == NULL)
					lptr->lib_req = deptr;
				else 
					prev->next = deptr;
			}
		}
	}
	return SUCCESS;
}

/*
 * Extract library root name from the library string, ex: alpha from
 * LIB/lib_alpha.a
 *
 */

void
lib_root(st, rptr)
char *st;
char *rptr;
{
	char *lptr;

	lptr = st + strlen(LIB_PREFIX);
	(void)strncpy(rptr, lptr, strlen(lptr) - strlen(LQUALFR));
	return;

}

make_loadable_syscall(fp,ftp)
FILE *fp;
struct file_list *ftp;
{ 
	char *name;
	char *prefix;

	name = ftp->f_fn;
	prefix = ftp->f_syscall;
	if(strcmp(ftp->f_syscall,"std") == 0)
		ftp->f_syscall[0] = 0;

	fprintf(fp,"%sinit_sysent.c %ssyscalls.c %ssyscall.h:",
		prefix, prefix, prefix);
	fprintf(fp," conf/makesyscalls.ksh \\\n");
	fprintf(fp,"\t\tconf/%sr\n", name);

	fprintf(fp,"\tksh conf/makesyscalls.ksh \\\n");
	fprintf(fp,"\t\t-hk . conf/%sr %s\n", name, prefix);
	fprintf(fp,"\trm -f ../include/sys/%ssyscall.h\n", prefix);
	fprintf(fp,"\tmv ./%ssyscall.h ../include/sys/%ssyscall.h\n\n",
		prefix, prefix);
}

/*
 * Create the load address for the kernel.  It is based on the `-k' flag
 * when config is run.  It needs to be a higher address for kernel
 * debugging because the kernel debugger lives in the low address space.
 * (thats dbgmon used by DEC.) This could be put in via makeopts but I don't
 * think thats the kind of power the customer needs and could make our
 * debugging custome dumps harder.
 */
do_base_addr(f)
	register FILE *f;
{
	unsigned int load_addr = 0x80030000;
	if(kdebug)
		load_addr = 0x80040000;
	else
		load_addr = 0x80030000;
	fprintf(f, "TEXTBASE=\t%x\n\n",load_addr);
}

/*
 * Create a Makefile varaible MACHINE that reflects the machine architecture
 * we are on so that we can determin what to do in the makefile based
 * on this value.
 */
do_machine(f,string)
register FILE *f;
char *string;
{
	fprintf(f,"MACHINE=\t%s\n\n",string);
}
