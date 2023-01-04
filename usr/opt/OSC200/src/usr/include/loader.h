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
 *	@(#)$RCSfile: loader.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:16:30 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* loader.h
 *
 * Loader application program interfaces
 * Depends on : 
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_LOADER
#define _H_LOADER

#include <standards.h>
#include <limits.h>

#ifdef	_OSF_SOURCE

#define LDR_REGION_NAME_MAX	64	/* max length of region name */

typedef int		ldr_process_t;	/* process descriptor for xp routines  */

typedef	unsigned	ldr_load_flags_t; /* load flags */
#define LDR_NOFLAGS	((ldr_load_flags_t)0)
#define	LDR_NOINIT	((ldr_load_flags_t)0x1)	/* don't run init routines */
#define LDR_WIRE	((ldr_load_flags_t)0x2)	/* wire loaded module against paging */
#define LDR_NOUNREFS	((ldr_load_flags_t)0x4)	/* no unresolved refs allowed */
#define LDR_NOPREXIST	((ldr_load_flags_t)0x8)	/* module must not pre-exist */
#define LDR_EXPORTONLY	((ldr_load_flags_t)0x10) /* just build module export table (internal only) */
#define LDR_NOUNLOAD	((ldr_load_flags_t)0x20) /* unloading module disallowed */
#define LDR_PREXIST	((ldr_load_flags_t)0x40) /* module must already be loaded */
#define LDR_MAIN	((ldr_load_flags_t)0x80) /* main module (internal only) */

/* The following load flags are propagated to dependencies */

#define	LDR_FLAGS_PROPAGATE	(LDR_NOINIT|LDR_WIRE|LDR_NOUNREFS)

typedef unsigned long	ldr_module_t;	/* a module ID */
typedef long		ldr_region_t;	/* a region ID */
typedef int		(*ldr_entry_pt_t)(); /* an entry point */

#define LDR_NULL_MODULE	((ldr_module_t)0) /* null module ID */

typedef unsigned ldr_prot_t;		/* protections for loader regions */
#define	LDR_R	((ldr_prot_t)0x1)	/* read permission */
#define	LDR_W	((ldr_prot_t)0x2)	/* write permission */
#define	LDR_X	((ldr_prot_t)0x4)	/* execute permission */

typedef struct ldr_module_info_t {
	ldr_module_t	lmi_modid;	/* module ID */
	int		lmi_nregion;	/* count of no. of regions in module */
	ldr_load_flags_t lmi_flags;	/* flags for module */
	char		lmi_name[PATH_MAX]; /* pathname of module */
} ldr_module_info_t;

typedef struct ldr_region_info_t {
	ldr_region_t	lri_region_no;	/* region number */
	ldr_prot_t	lri_prot;	/* protection flags */
	void		*lri_vaddr;	/* virtual address of region */
	void		*lri_mapaddr;	/* address region is mapped at */
	size_t		lri_size;	/* region size */
	char		lri_name[LDR_REGION_NAME_MAX]; /* region name */
} ldr_region_info_t;

#ifndef _NO_PROTO

/* 
 * loader operations on the current process 
 */

extern ldr_module_t
load(char *file_pathname, ldr_load_flags_t load_flags);

extern int
unload(ldr_module_t mod_id);

extern ldr_entry_pt_t
ldr_entry(ldr_module_t mod_id);

extern void *
ldr_lookup (ldr_module_t mod_id, char* symbol_name);

extern void *
ldr_lookup_package(char *package, char *symbol_name);

extern int
ldr_install(const char *module_name);

extern int
ldr_remove(const char *module_name);

/*
 * loader operations across process boundaries 
 */

extern ldr_process_t
ldr_my_process(void);

extern ldr_process_t
ldr_kernel_process(void);

extern int
ldr_xattach(ldr_process_t process);

extern int
ldr_xdetach(ldr_process_t process);

extern int
ldr_xload(ldr_process_t process, char *file_pathname, 
	  ldr_load_flags_t load_flags, ldr_module_t *mod_id_ptr);

extern int
ldr_xunload(ldr_process_t process, ldr_module_t mod_id);

extern int
ldr_xentry(ldr_process_t process, ldr_module_t mod_id,
	   ldr_entry_pt_t *entry_ptr);

extern int
ldr_xlookup(ldr_process_t process, ldr_module_t mod_id,
	    char *symbol_name, void **symbol_addr_ptr);

extern int
ldr_xlookup_package(ldr_process_t process, char *package_name,
		    char *symbol_name, void **symbol_addr_ptr);


/*
 * Debug functions
 */

extern int
ldr_inq_module(ldr_process_t process, ldr_module_t mod_id,
	       ldr_module_info_t *info, size_t info_size, size_t *ret_size);

extern int
ldr_inq_region(ldr_process_t process, ldr_module_t mod_id, ldr_region_t region,
	       ldr_region_info_t *info, size_t info_size, size_t *ret_size);

extern int
ldr_next_module(ldr_process_t process, ldr_module_t *mod_id_ptr);

#else	/* _NO_PROTO : no function prototypes */

/* 
 * loader operations on the current process 
 */

extern ldr_module_t load();
extern int unload();
extern ldr_entry_pt_t ldr_entry();
extern void *ldr_lookup ();
extern void *ldr_lookup_package();
extern int ldr_install();
extern int ldr_remove();

/*
 * loader operations across process boundaries 
 */

extern ldr_process_t ldr_my_process();
extern ldr_process_t ldr_kernel_process();
extern int ldr_xload();
extern int ldr_xunload();
extern int ldr_xentry();
extern int ldr_xlookup();
extern int ldr_xlookup_package();

/*
 * Debug functions
 */

extern int ldr_inq_module();
extern int ldr_inq_region();
extern int ldr_next_module();

#endif	/* _NO_PROTO */

#endif /* _OSF_SOURCE */

#endif	/* _H_LOADER */



