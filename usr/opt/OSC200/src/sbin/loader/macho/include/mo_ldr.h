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
 *	@(#)$RCSfile: mo_ldr.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:41:46 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *
 * mo_ldr.h
 *
 * Declarations and data structures used by the mach_o
 * format dependent loader.
 * Depends on "open_hash.h"
 *
 * OSF/1 Release 1.0
 */

#ifndef _MO_LDR

#define _MO_LDR

enum lc_type {			/* LC type */
	MO_UNDEFINED,
	MO_CMD_MAP,
	MO_INTERPRETER,
	MO_STRINGS,
	MO_REGION,
	MO_RELOC,
	MO_PACKAGE,
	MO_SYMBOLS,
	MO_ENTRY,
	MO_FUNC_TABLE,
	MO_GEN_INFO
};


/* load command table entry */
struct mo_lc_entry_t {
	void *LC;				/* VM addr of Load Command */
	enum lc_type type;			/* type of load cmd */
	short flags;
	union	{
		void *section_data_u;		/* ptr to section contents */
		void *LC_data_u;		/* ptr to LC immediate info */
	} data;

#define	section_data	data.section_data_u
#define LC_data		data.LC_data_u

	union	{
		ldr_window_t *section_window_u;	/* ptr to window for section */
		int region_id_u;		/* region no, for regions only */
	} optional;

#define section_window optional.section_window_u
#define region_id optional.region_id_u

};

typedef struct mo_lc_entry_t mo_lc_entry_t;

/*
 * get a ptr to the load command given a ptr to the LC table entry
 */
#define LOAD_CMD(lc_entry_p) (lc_entry_p->LC)

/*
 * get a ptr to the nth load command given the number n and a 
 * macho_ldr_module_handle_t (read handle).
 */
#define LOAD_CMD_N(n, handle) \
	((((mo_lc_entry_t *)handle->load_cmd_table) + n)->LC)

/*
 * give the region number corresponding to region load cmd table
 * entry.  Returns a -1 if LC is not a region load cmd.
 */
#define LC_TO_REGNO(lc_entry_p) \
	((lc_entry_p->type == MO_REGION) ? lc_entry_p->region_id : -1)

/* format dependent (FD) region table entry */
struct mo_region_entry_t {
	mo_lc_entry_t *LC_entry;	/* pointer to LC table entry */
	unsigned long region_size;	/* size of region in VM addr space */
};

typedef struct mo_region_entry_t mo_region_entry_t;

/* 
 * get a ptr to the load command table entry *LC_entry given a
 * ptr to the region table and the region number.
 */

#define REGION_LCTABLE_ENTRY(rt_p, regno) \
	(((mo_region_entry_t *)rt_p + regno)->LC_entry)


/* mach format dependent (FD) handle (for use by macho FD manager) */
struct macho_module_handle_t {
	long no_load_cmds;		/* count of load commands in object */
	mo_lc_entry_t *load_cmd_table;	/* table - keeps LC information */
	long no_regions;		/* no of region table entrys */
	mo_region_entry_t 
		*region_table;		/* Format Dependent region table */

	mo_lc_entry_t *def_symb_cmd;	/* def symbol LC table entry */
	mo_lc_entry_t *import_cmd;	/* import LC table entry */
	mo_lc_entry_t 
		*import_package_cmd; 	/* import pkg LC table entry */
	mo_lc_entry_t	
		*export_package_cmd;	/* export pkg LC table entry */
	mo_lc_entry_t *string_cmd;	/* string LC table entry  */
	mo_lc_entry_t *entry_pt_cmd;	/* entry point LC table entry */
	ldr_entry_pt_t entry_pt;	/* entry point for module if any */

	ldr_file_t fd;			/* handle to object file */
	ldr_window_t *load_cmd_w;	/* all LCs as single window */

	long no_exports;		/* no of export symbols in module */
					/* export open hash table */
	open_hashtab_t export_list;	/* needed here for cleanup use */

	mo_addr_t *term_funcs;		/* array of termination functions */
	long term_funcs_nentries;	/* number of termination functions */
};

typedef struct macho_module_handle_t *macho_module_handle_ptr_t;



/*
 * given a mo_addr_t and the macho handle, return a ptr to
 * the data in memory. This assumes that the data immediately
 * follows the LC and does not need to be brough in from disk.
 */
#define MO_GET_LDR_ADDR_IMED(moaddr, handle) \
	do { \
		(void *)(((char*)((handle + moaddr.adr_lcid)->ldc_cmd_size) + \
		 moaddr.addr_sctoff)) \
	} while(0)

/*
 * check for a non-NULL loader module handle.
 */
#define	CHECK_MACHO_HANDLE(h) \
	do { \
		     if (h == NULL) return LDR_ENOEXEC; \
	} while (0)

/*
 * check for a valid load command id.
 */
#define CHECK_LC_ID(h, n) \
	do { \
		if ((n == MO_INVALID_LCID) || (n >= ((macho_module_handle_ptr_t)h)->no_load_cmds)) \
			return LDR_ENOEXEC; \
	} while(0)

/*
 * check region number within valid range
 */
#define CHECK_REG_NO(regno, max) \
	do { \
		if ((regno < 0) || (regno >= max)) \
			return LDR_ENOEXEC; \
	} while(0)


#define MO_ASSERT(i, j, str, err) \
	do { \
		if (i == j) { \
			ldr_msg("MO_ASSERT: %s\n", str); \
			return err; \
		} \
	} while(0)

#define MO_NEG_ASSERT(i, j, str, err) \
	do { \
		if (i != j) { \
			ldr_msg("MO_NEG_ASSERT: %s\n", str); \
			return err; \
		} \
	} while (0)

/* unwindow a section of load command that was previously windowed
   by calling window_in_section() */
#define UNWINDOW_SECTION(lc_entry_p) \
        do { \
		if ((lc_entry_p->section_data != NULL) && \
		    (lc_entry_p->section_window != NULL)) { \
			ldr_unwindow(lc_entry_p->section_window); \
			lc_entry_p->section_data = NULL; \
			lc_entry_p->section_window = NULL; \
		} \
	} while (0)
			

/* Machine dependent relocation routine */
extern int
mach_dep_relocate_region(macho_module_handle_ptr_t handle, 
			 ldr_region_rec *regions, int nregions,
			 int npackages, ldr_package_rec *import_pkgs, 
			 int nimports, ldr_symbol_rec *imports,
			 int reg_no);

/* Machine dependent routine to extract a ldr_symval from a Mach-O file */
extern int
sivalue_to_ldrsymval(macho_module_handle_ptr_t handle, symbol_info_t *sym_p, 
		     ldr_symval *sym_value);

/* Machine independent routines needed by machine dependent code */

extern void *
get_mo_ldr_addr(mo_addr_t *fileaddr, macho_module_handle_ptr_t handle);

extern int
window_in_section(macho_module_handle_ptr_t handle, mo_lc_entry_t *p);


#endif	/* _MO_LDR */

