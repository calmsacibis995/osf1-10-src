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
static char	sccsid[] = "@(#)$RCSfile: lib_admin.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:35:21 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * lib_admin.c
 *
 * Library administration tool.
 * Manages installing and pre-loading libraries into global library
 * data file.
 *
 * Syntax:
 *  lib_admin [-p] [-i] [-o <global_file>] <database>
 *
 *  -p			Don't pre-load any libraries
 *  -i			Don't install any libraries
 *  -o <global_file>	Output result to specified file instead
 *			of default (/etc/ldr_global.dat)
 *  -v			Verbose
 *
 * OSF/1 Release 1.0.1
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <string.h>
#include <nl_types.h>
#include <errno.h>
#include <loader.h>
#include <AFdefs.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#include "ldr_types.h"
#include "ldr_errno.h"
#include "squeue.h"

#ifdef LDR_STATIC_LINK
#include "ldr_lock.h"
#endif

#include "lib_admin_msg.h"

#define MSGSTR(num,str) catgets(catd,MS_LIB_ADMIN,num,str)

/* #define DEBUG 1 */

/* Shouldn't be here, but there is no include file for them... */

extern int getopt(int argc, char **argv, char *opts);
extern int optind, opterr;
extern char *optarg;


/* NOTE: following four definitions only needed if the loader is being
 * statically linked.
 */

#ifdef LDR_STATIC_LINK

/* structure to save locking functions used by loader */

lib_lock_functions_t	ldr_lock_funcs;

/* This is the only loader lock for this process */

ldr_lock_t ldr_global_lock;

/* This process' loader context */

ldr_context_t	ldr_process_context;

/* Definition of standard loader data file.
 */

#ifdef DEBUG
const char *ldr_global_data_file = "/tmp/ldr_global.dat";
const char *ldr_dyn_database = "/tmp/ldr_dyn_mgr.conf";
#else
const char *ldr_global_data_file = LDR_GLOBAL_DATA_FILE;
const char *ldr_dyn_database = LDR_DYN_DATABASE;
#endif

#endif /* LDR_STATIC_LINK */

typedef	unsigned	lib_flags_t;	/* install/preload flags */

#define LIB_FLAGS_NONE	0		/* no flags */
#define	LIB_INSTALL	0x01		/* do library install */
#define	LIB_PRELOAD	0x02		/* do library preload */

/* Structure describing a library to be installed and/or preloaded.
 * One such entry is allocated for each entry in the database.
 */

typedef struct library_desc {
	/* Note: the next field must be first, for the squeue routines */
	struct library_desc	*ld_next; /* next in linked list */
	char			*ld_name; /* library name */
	lib_flags_t		ld_flags; /* flags */
} library_desc;

/* List header.  Must match struct squeue2 in squeue.h */

typedef struct	library_list {
	struct library_desc	*ll_head;
	struct library_desc	*ll_tail;
} library_list;


/* The following macro iterates through all the library descriptors on
 * the specified list.
 */

#define	for_all_libs(list, lib) \
	for ((lib) = sq2_head(list, library_desc *); \
	     (lib) != NULL; \
	     (lib) = (lib)->ld_next)


/* Attributes and attribute parsing routines */

typedef int (*parse_p)(const char *attr, library_desc *desc);

static int parse_install(const char *value, library_desc *desc);
static int parse_preload(const char *value, library_desc *desc);
static int lib_admin_maketemp(const char *loc, int mode, ldr_file_t *pfd, char **pfname);

const struct attribute {
	char		*at_name;
	parse_p		at_proc;
} attributes[] = {
	{ "install", parse_install },
	{ "preload", parse_preload },
	{ NULL, NULL }
};

/* The following macro iterates through all the attributes */

#define for_all_attributes(atp) \
	for ((atp) = &attributes[0]; \
	     (atp)->at_name != NULL; \
	     (atp)++)



/* Globals */

const char	*database_file;
const char	*output_file;
/* For now, preloading MIPS COFF/ELF files doesn't make sense... */
/* lib_flags_t	def_mask = (LIB_INSTALL | LIB_PRELOAD);		 */
lib_flags_t	def_mask = LIB_INSTALL;
nl_catd catd;   /* message catalog descriptor */
int		verbose = 0;

/* Defines */

#define	OPEN_MODE (S_IRUSR|S_IRGRP|S_IROTH)
#define MAXREC	(2 * PATH_MAX)
#define MAXATTR	(2 * NAME_MAX)



int
main(int argc, char **argv)
{
	library_list	lib_list;
	ldr_context_t	ctxt;

#ifdef	LDR_STATIC_LINK

/* NOTE: only needed while statically linked with loader */

	{
		int	rc;

		if ((rc = ldr_bootstrap(argv[0], &ldr_process_context)) != LDR_SUCCESS) {
			errno = ldr_status_to_errno(rc);
			perror(MSGSTR(ERR_BOOT, "ldr_bootstrap failed"));
			exit(1);
		}
	}

#endif	/* LDR_STATIC_LINK */

	(void)setlocale(LC_ALL, "");
	catd = catopen(MF_LIB_ADMIN, 0);

	if (parse_options(argc, argv) != 0) {
		usage(argc, argv);
		exit(1);
	}

	sq2_init(&lib_list);

	if (read_database(database_file, &lib_list) != 0)
		exit(1);

	if (get_temp_context(&ctxt) != 0)
		exit(1);

	if (install_libraries(ctxt, &lib_list) != 0)
		exit(1);

	if (preload_libraries(ctxt, &lib_list) != 0)
		exit(1);

	if (copyout_context(ctxt, output_file) != 0)
		exit(1);

	exit(0);
}


int
usage(int argc, char **argv, int rc)
{
	fprintf(stderr, MSGSTR(ERR_USAGE,
			       "usage: %s [-i] [-p] [-o <output file>] [-v] <database file>\n"),
			       argv[0]);
}


int
parse_options(int argc, char **argv)
{
	int optc;

	/* Initialize */

	output_file = ldr_global_data_file;

	while ((optc = getopt(argc, argv, "ipo:v")) != EOF) {
		switch(optc) {

		      case 'i':		/* don't install */
			def_mask &= ~LIB_INSTALL;
			break;

		      case 'p':		/* don't preload */
			def_mask &= ~LIB_PRELOAD;
			break;

		      case 'o':		/* output file */
			output_file = optarg;
			break;

		      case 'v':		/* verbose */
			verbose = 1;
			break;

		      default:
			return(-1);
		}
	}

	if (optind != (argc - 1))
		return(-1);
	database_file = argv[optind];

	return(0);
}


int
read_database(const char *name, library_list *listp)

/* Read the specified database file.  For each object, construct a
 * library descriptor and add it to tail of the list.
 */
{
	library_desc		*desc;
	AFILE_t			db;
	ENT_t			ent;
	ATTR_t			attr;
	const struct attribute	*atp;

	if ((db = AFopen((char *)name, MAXREC, MAXATTR)) == NULL) {
		perror(MSGSTR(ERR_AFOPEN, "opening database file"));
		return(-1);
	}

	while ((ent = AFnxtent(db)) != NULL) {

                /*
                 * Syntax errors return null: error checking done below.
                 */

		/* Have a library object; allocate and fill in
		 * descriptor for it.
		 */

		if ((desc = (library_desc *)malloc(sizeof(library_desc))) == NULL) {
			perror(MSGSTR(ERR_MALLOC, "malloc error"));
			return(-1);
		}

		/* Check for absolute pathname of library here? */

		desc->ld_next = NULL;
		if ((desc->ld_name = strdup(AFentname(ent))) == NULL) {
			perror(MSGSTR(ERR_MALLOC, "malloc error"));
			return(-1);
		}
		desc->ld_flags = LIB_FLAGS_NONE;

		/* Now get all library attributes */

		while ((attr = AFnxtatr(ent)) != NULL) {

			for_all_attributes(atp) {

				if (strcmp(AFatrname(attr), atp->at_name) == 0) {

					if ((*(atp->at_proc))(attr->AT_value, desc) == 0)
						goto found;
					fprintf(stderr, MSGSTR(ERR_BAD_VALUE,
							       "bad value %s for attribute %s, lib %s\n"),
						attr->AT_value, AFatrname(attr), AFentname(ent));
					return(-1);
				}
			}

			fprintf(stderr, MSGSTR(ERR_ATTR_UNKNOWN,
					       "unknown attribute %s in library %s\n"),
					       AFatrname(attr), AFentname(ent));
			return(-1);

found:			;		/* here on successful parse (null stmnt) */
		}

		/* Mask requested operations against prohibited ones */

		desc->ld_flags &= def_mask;

		/* Chain onto library descriptor list */

		sq2_ins_tail(listp, desc);
	}

        /* Do basic error checking after reading entry */

        if (db->AF_errs && (AF_SYNTAX || AF_ERRCBUF || AF_ERRCATR)) {
                fprintf(stderr, MSGSTR(ERR_DB_SYNTAX,
                                       "syntax error in database file\n"));
                return(-1);
        }

	/* Finished; clean up */

	AFclose(db);
	return(0);
}


int
get_temp_context(ldr_context_t *ctxtp)

/* Create a loader context in which to do the library installation and
 * preloading.  We must start by removing the current global library
 * file, so that installs and loads into the new temporary context
 * will not use preload information from the old cache.
 *
 * The region allocation routines we use are those for the preloading
 * procedure.
 */
{
	ldr_context_t		ctxt;
	int			rc;
	extern			int preload_alloc_abs(), preload_alloc_rel(),
				preload_dealloc();

	if ((rc = ldr_context_global_file_remove(ldr_process_context)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror(MSGSTR(ERR_REMFILE, "unable to unmap old loader global file"));
		return;
	}

	if ((rc = ldr_context_create(LDR_NMODULES, preload_alloc_abs,
				     preload_alloc_rel, preload_dealloc,
				     &ctxt)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror(MSGSTR(ERR_CONTEXT, "unable to create temporary context"));
		return(-1);
	}

	if ((rc = ldr_context_bootstrap(ctxt, "")) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror(MSGSTR(ERR_CTXBOOT, "unable to bootstrap temporary context"));
		return(-1);
	}

	*ctxtp = ctxt;
	return(0);
}


int
install_libraries(ldr_context_t ctxt, library_list *list)

/* Install all the libraries in the specified list that are marked
 * for installation into the specified context.
 */
{
	library_desc		*lib;
	int			rc;

	for_all_libs(list, lib) {

		if (!(lib->ld_flags & LIB_INSTALL))
			continue;

		if ((rc = ldr_context_install(ctxt, lib->ld_name)) != LDR_SUCCESS) {
			errno = ldr_status_to_errno(rc);
			perror(MSGSTR(ERR_INSTALL, "error installing library"));
			fprintf(stderr, MSGSTR(ERR_INSTALLLIB, "installing library %s\n"), 
				lib->ld_name);
			return(-1);
		}

		if (verbose)
			printf(MSGSTR(MSG_INSTALL,
				      "installed library %s\n"), lib->ld_name);

	}
	return(0);
}


int
preload_libraries(ldr_context_t ctxt, library_list *list)

/* Preload all the libraries in the specified list that are marked
 * for preloading into the specified context.
 */
{
	library_desc		*lib;
	ldr_module_t		mod_id;
	int			rc;

	for_all_libs(list, lib) {
		if (!(lib->ld_flags & LIB_PRELOAD))
			continue;

		if ((rc = ldr_context_load(ctxt, lib->ld_name, LDR_NOUNREFS,
					   &mod_id)) != LDR_SUCCESS) {
			errno = ldr_status_to_errno(rc);
			perror(MSGSTR(ERR_PRELOAD, "error preloading library"));
			fprintf(stderr, MSGSTR(ERR_PRELOADLIB,
					       "preloading library %s\n"),
				lib->ld_name);
			return(-1);
		}

		if (verbose)
			printf(MSGSTR(MSG_PRELOAD,
				      "loaded library %s\n"), lib->ld_name);

	}
	return(0);
}


int
copyout_context(ldr_context_t ctxt, const char *fname)

/* Copy out the specified context into the specified output file.
 * Begin by creating a temp file in the same directory, open for
 * writing and truncated.  Copy the context out to it, and close it.
 * Then, atomically rename it into place as the new output file.
 */
{
	int			fd;
	char			*tmp_name;
	int			rc;

	if ((rc = lib_admin_maketemp(fname, OPEN_MODE, &fd, &tmp_name)) != LDR_SUCCESS) {
		perror(MSGSTR(ERR_OUTFILE, "error creating output file"));
		return(-1);
	}

	/* Now, install our new context into the global file */

	if ((rc = ldr_context_global_file_init(ctxt, fd)) != LDR_SUCCESS) {
		errno = ldr_status_to_errno(rc);
		perror(MSGSTR(ERR_FILEINIT, "error initializing global file"));
		goto err_return;
	}

	/* Close our file, and rename it into place */
	(void)close(fd);
	if (rename(tmp_name, fname) != 0) {
		perror(MSGSTR(ERR_RENAME, "unable to rename temp to global file"));
		goto err_return;
	}

	if (verbose)
		printf(MSGSTR(MSG_DUMPED, "created global data file %s\n"),
		       fname);

	return(0);

err_return:
	(void)close(fd);
	(void)unlink(tmp_name);
	(void)free(tmp_name);
	return(rc);
}


/* Attribute parsers */

static int
parse_install(const char *value, library_desc *desc)
{
	if (strcmp(value, "true") == 0)
		desc->ld_flags |= LIB_INSTALL;
	else if (strcmp(value, "false") == 0)
		desc->ld_flags &= ~LIB_INSTALL;
	else
		return(-1);
	return(0);
}


static int
parse_preload(const char *value, library_desc *desc)
{
	if (strcmp(value, "true") == 0)
		desc->ld_flags |= LIB_PRELOAD;
	else if (strcmp(value, "false") == 0)
		desc->ld_flags &= ~LIB_PRELOAD;
	else
		return(-1);
	return(0);
}


int
lib_admin_maketemp(const char *loc, int mode, ldr_file_t *pfd, char **pfname)

/* Make a temporary file name, and create it.  Arguments are: pathname of
 * a file (or directory, must end in '/') in which to create the temp
 * file (may be NULL), and protection mode for new file.  Returns the
 * pathname of the temp file (in strdup'ed storage), and the open
 * file descriptor on the temp file.
 *
 * Constructed file name is of the form "ldrPPPPPPPP.X", where the
 * PPPPPPPP is the process ID in hex, and the .X is a "uniqueizer".
 */
{
	const char		*defdir = "/tmp/";
	const int		deflen = 5; /* strlen(defdir) */
	const char		*pattern = "ldr%08x.%1x";
	const int		psize = 14; /* size of pattern */
	const int		maxtry = 16;

	const char		*endloc;
	const char		*p;
	char			*newname;
	int			try;
	int			fd;

	if (loc == NULL) {		/* no location specified */
		loc = (char *)defdir;
		endloc = &defdir[deflen];
	} else {
		for (p = loc, endloc = NULL; *p != '\0'; p++)
			if (*p == '/')
				endloc = p + 1;
		if (endloc == NULL)	/* HUH? */
			return(-1); /* invalid location */
	}

	if ((newname = malloc((endloc - loc) + psize + 1)) == NULL)
		return(-1);

	bcopy(loc, newname, (endloc - loc));

	for (try = 0; try < maxtry; try++) {

		sprintf(&newname[(endloc - loc)], pattern, getpid(),
			    try);
		if ((fd = open(newname, O_RDWR|O_CREAT|O_TRUNC|O_EXCL,
				   mode)) >= 0)
			break;
	}

	if (fd < 0) {
		(void)free(newname);
		return(-1);
	}

	*pfd = (ldr_file_t)fd;
	*pfname = newname;
	return(0);
}
