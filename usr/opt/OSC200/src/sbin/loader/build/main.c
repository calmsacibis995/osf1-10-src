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
static char	sccsid[] = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/04/23 10:51:37 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * main.c
 *
 * loader main(), receives control from exec_with_loader(2) and crt0
 *
 * OSF/1 Release 1.0.1
 */

/* #define HELLO_GOODBYE 1 */
/* #define STANDALONE 1 */
/* #define DEBUG 1 */

#include <sys/types.h>
#include <sys/auxv.h>
#include <sys/ldr_exec.h>
#include <loader.h>

#include <loader/ldr_main_types.h>
#include <loader/ldr_main.h>

#include "ldr_types.h"
#include "ldr_errno.h"
#include "ldr_auxv.h"
#include "ldr_lock.h"

#ifdef	HELLO_GOODBYE
#include <string.h>
#include <fcntl.h>

static void main_hello();
static void main_goodbye();
#endif

/* structure to save locking functions used by loader */

lib_lock_functions_t	ldr_lock_funcs;

/* This is the only loader lock for this process */

ldr_lock_t ldr_global_lock;

/* This process' loader context */

ldr_context_t	ldr_process_context;

/* Definition of standard loader data file */

#ifdef DEBUG
const char *ldr_global_data_file = "/tmp/ldr_global.dat";
const char *ldr_dyn_database = "/tmp/ldr_dyn_mgr.conf";
#else
const char *ldr_global_data_file = LDR_GLOBAL_DATA_FILE;
const char *ldr_dyn_database = LDR_DYN_DATABASE;
#endif

extern int _ldr_crt0_request;
static int *_lcrp = &_ldr_crt0_request;
int _ldr_present = 1;

#ifdef i386
extern void _ldr_jump();
void (*_ldr_jump_func)() = _ldr_jump;
#endif

static void main_set_program_attributes(ldr_module_t);
extern void ldr_abort(void);
extern void ldr_bpt(void);
extern int errno;
static int ldr_initialized = 0;

ldr_entry_pt_t
main()
{
	ldr_entry_pt_t		entry_pt;
	ldr_module_t		module;
	int			rc;
	char *			loader_file;
	char *			file;
	int			flags;
	ldr_file_t		file_fd;

	if (ldr_initialized++) {

		/* re-entered -- bomb out now */

		ldr_msg("loader main: loader main re-entered, exiting\n");
		ldr_abort();
	}

#ifdef STANDALONE
	setup_auxv();
#endif

	ldr_auxv_init();
#ifdef	HELLO_GOODBYE
	main_hello();
#endif
	if ((rc = ldr_auxv_get_exec_loader_filename(&loader_file)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_auxv_get_exec_loader_filename(), status: %E\n%B\n",
			rc);
		ldr_abort();
	}
	if ((rc = ldr_bootstrap(loader_file, &ldr_process_context)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_bootstrap() failed, status: %E\n%B\n",
			rc);
		ldr_abort();
	}
	if ((rc = ldr_auxv_get_exec_filename(&file)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_auxv_get_exec_filename() failed, status: %E\n%B\n",
			rc);
		ldr_abort();
	}

	/* This fix has been picked up from osf1.1 baselevel 24.  It relies
	 * on changes to kern_exec, but will work with older kernels as well.
	 * Added code to use the file descriptor provided by exec rather
	 * than trying to open the file by hand.  This allows us to execute
	 * programs which have been protected as execute only.  If exec
	 * doesn't provide a file descriptor ldr_context_load_fd() will
         * attempt to open the file by hand.  3/11/92 lowell@krisis.
	 */

	ldr_auxv_get_execfd(&file_fd);
	if ((rc = ldr_context_load_fd(ldr_process_context, file, LDR_MAIN,
				      &module, file_fd)) != LDR_SUCCESS) {
		ldr_msg("loader main: load(\"%s\", LDR_MAIN) failed, status: %E\n%B\n",
			file, rc);
		ldr_abort();
	}
	main_set_program_attributes(module);
	if ((rc = ldr_context_get_entry_pt(ldr_process_context, module,
					   &entry_pt)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_entry() failed, status: %E\n%B\n",
			rc);
		ldr_abort();
	}
#ifdef	HELLO_GOODBYE
	main_goodbye();
#endif
	if ((rc = ldr_auxv_get_exec_loader_flags(&flags)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_auxv_get_exec_loader_flags() failed, status: %E\n%B\n",
			rc);
		ldr_abort();
	}
	if (flags & LDR_EXEC_PTRACE_F)
		ldr_bpt();
	return(entry_pt);
}

void
main_set_program_attributes(module)
	ldr_module_t		module;
{
	ldr_module_info_t	ldr_module_info, *lmip;
	ldr_region_info_t	ldr_region_info, *lrip;
	ldr_region_t		region;
	int			text_set = 0;
	int			text_start = 0;
	int			text_end = 0;
	int			data_set = 0;
	int			data_start = 0;
	int			data_end = 0;
	int			text_size, data_size;
	int			start, end;
	size_t			ret_size;
	int			rc;

	lmip = &ldr_module_info;
	if ((rc = ldr_context_inq_module(ldr_process_context, module,
					 lmip, sizeof(ldr_module_info),
					 &ret_size)) != LDR_SUCCESS) {
		ldr_msg("loader main: ldr_inq_module() failed, status: %E\n%B\n",
			rc);
		ldr_abort();
	}
	lrip = &ldr_region_info;
	for (region = 0; region < ldr_module_info.lmi_nregion; region++) {
		if ((rc = ldr_context_inq_region(ldr_process_context,
						 module, region, lrip,
						 sizeof(ldr_region_info),
						 &ret_size)) != LDR_SUCCESS) {
			ldr_msg("loader main: ldr_inq_region() failed, status: %E\n%B\n",
				rc);
			ldr_abort();
		}

		start = (int)lrip->lri_vaddr;
		end = start + (int)lrip->lri_size;

		if (lrip->lri_prot & LDR_W) {
			/* data or bss region */
			if (data_set) {
				if (start < data_start)
					data_start = start;
				if (end > data_end)
					data_end = end;
			} else {
				data_set = 1;
				data_start = start;
				data_end = end;
			}
		} else {
			/* text region */
			if (text_set) {
				if (start < text_start)
					text_start = start;
				if (end > text_end)
					text_end = end;
			} else {
				text_set = 1;
				text_start = start;
				text_end = end;
			}
		}
	}

	text_size = text_end - text_start;
	data_size = data_end - data_start;

	if (set_program_attributes(text_start, text_size, data_start,
	    data_size) == -1) {
		ldr_msg("loader main: set_program_attributes() failed with errno = %d\n",
			errno);
		ldr_abort();
	}

	/* Set the loader's idea of the current break */

	if ((rc = ldr_brk((univ_t)data_end)) != LDR_SUCCESS) {
		ldr_msg("loader main: unable to initialize break, %E\n", rc);
		ldr_abort();
	}
}

#ifdef	HELLO_GOODBYE

#define	STRING_SIZE	(10*1024)

static char *loader;

static void
main_hello()
{
	char hello[STRING_SIZE];
	char *file;

	hello[0] = '\0';
	if (ldr_auxv_get_exec_loader_filename(&loader) != LDR_SUCCESS)
		return;
	if (loader == (char *)-1)
		(void)strcat(hello, "loading");
	else {
		(void)strcat(hello, loader);
		(void)strcat(hello, ": loading");
	}
	if (ldr_auxv_get_exec_filename(&file) != LDR_SUCCESS)
		return;
	if (file == (char *)-1)
		(void)strcat(hello, " ...\n");
	else {
		(void)strcat(hello, " ");
		(void)strcat(hello, file);
		(void)strcat(hello, " ...\n");
	}
	ldr_puts(hello);
}

static void
main_goodbye()
{
	char goodbye[STRING_SIZE];

	goodbye[0] = '\0';
	if (loader == (char *)-1)
		strcpy(goodbye, "done\n");
	else {
		strcpy(goodbye, loader);
		strcat(goodbye, ": done\n");
	}
	ldr_puts(goodbye);
}

#endif	/* HELLO_GOODBYE */

#ifdef STANDALONE
setup_auxv()
{
	static auxv_t auxv[4];
	extern auxv_t *_auxv;

	auxv[0].a_type = AT_EXEC_FILENAME;
	auxv[0].a_un.a_ptr = "foo";
	auxv[1].a_type = AT_EXEC_LOADER_FILENAME;
	auxv[1].a_un.a_ptr = "loader";
	auxv[2].a_type = AT_EXEC_LOADER_FLAGS;
	auxv[2].a_un.a_val = 0;
	auxv[3].a_type = AT_NULL;

	_auxv = auxv;
}
#endif

void
exit(int status)
{
	_exit(status);
}
