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
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * rcoff_ldr.h
 *
 * Declarations and data structures used by the relocatable COFF format
 * dependent loader.  Original based on coff_types.h
 *
 * OSF/1 Release 1.0
 */

#ifndef __H_RCOFF_LDR
#define __H_RCOFF_LDR

/* Region indicies */
#define TEXT_REGNO		0
#define	DATA_REGNO		1
#define	BSS_REGNO		2
#define MAX_RCOFF_REGIONS	3       /* no of regions in a coff file */

/*
 * RCOFF format dependent (FD) handle (for use by RCOFF FD manager).
 */
typedef struct rcoff_module_handle  *rcoff_module_handle_t;

struct rcoff_module_handle {
	ldr_file_t	fd;		/* file descriptor for module */
	ldr_window_t	*wp;		/* window file header */
	univ_t		fhd;		/* module file header */
	ldr_window_t	*wpo;		/* window optional header */
	univ_t		ohd;		/* optional file header */
	ldr_window_t	*wps;		/* window section headers */
	univ_t		shd;		/* section headers */
	ldr_entry_pt_t	entry_pt;	/* entry point, if any */
	open_hashtab_t	export_list;	/* export open hash table */
	int		nexport;	/* number of exports */
	int		nimport;	/* number of imports */
	int		*import_map;	/* symbol index to import index map */
	char		*strtab;	/* string table */
	univ_t		symtab;		/* symbol table */
	int		nsym;		/* number of symbols in symtab */
	char		*filename;	/* associated file name */
	rcoff_module_handle_t next;	/* next in linked list */
	rcoff_module_handle_t prev;	/* previous in linked list */
	ldr_package_rec *package;	/* associated package */
};

/*
 * Check for a non-NULL loader module handle.
 */
#define CHECK_RCOFF_HANDLE(h) \
	do { \
		if (h == NULL) return LDR_ENOEXEC; \
	} while (0)

/* Section relocation routine */
extern int
rcoff_relocate_section(rcoff_module_handle_t handle,
			int nregions, ldr_region_rec *regions,
			int nimports, ldr_symbol_rec *imports,
			int sectno);

/* Transfer vector intialization */
extern int
rcoff_init_vectors(vector_t *vec, int nimports, ldr_symbol_rec *imports);

#endif /* __H_RCOFF_LDR */
