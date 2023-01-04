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
/* @(#)$RCSfile: coff_ldr.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/19 09:14:49 $ */ 
/*
 *
 * 01-Mar-1991, Ken Lesniak
 *	Modified for COFF/ELF shared libraries
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * coff_ldr.h
 *
 * Declarations and data structures used by the COFF format dependent
 * loader.
 *
 * OSF/1 Release 1.0
 */

#ifndef __H_COFF_LDR
#define __H_COFF_LDR

/* Region indicies */
#define TEXT_REGNO		0
#define	DATA_REGNO		1
#define	BSS_REGNO		2
#define MAX_COFF_REGIONS        3       /* no of regions in a coff file */

/*
 * COFF format dependent (FD) handle (for use by COFF FD manager).
 */
struct coff_module_handle {
	ldr_file_t	fd;		/* file descriptor for module */
	ldr_window_t	*wp;		/* object file window */
	univ_t		p_raw;		/* pointer to raw object file data */
	ldr_entry_pt_t	entry_pt;	/* entry point, if any */
	univ_t		base_vaddress;	/* loaded virtual base addr of module */
	univ_t		base_paddress;	/* loaded physical base addr of module */
	open_hashtab_t	export_list;	/* export open hash table */
	univ_t		export_svp;	/* memory allocated to export_list */
	char		**expkgnam;	/* export package name list */
	int		is_PIC;		/* true if object contains PIC */
	int		is_dynamic;	/* true if dynamic object */
	Elf32_Rel	*d_rel;		/* address of .rel.dyn section */
	Elf32_Sym	*d_symtab;	/* address of .dynsym section */
	Elf32_Got	*d_got;		/* address of .got section */
	Elf32_Package	*d_package;	/* address of .package section */
	Elf32_Word	*d_packsym;	/* address of .packsym section */	
	char		*d_strtab;	/* address of .dynstr section */
	Elf32_Addr	d_init;		/* address of .init section */
	Elf32_Addr	d_fini;		/* address of .fini section */
	Elf32_Word	d_strsz;	/* string table size */
	Elf32_Word	d_locgotno;	/* number of local GOT entries */
	Elf32_Word	d_relsz;	/* relocation table size */
	Elf32_Word	d_unrefextno;	/* index of 1st unreferenced ext sym */
	Elf32_Word	d_gotsym;	/* index of 1st sym with a GOT entry */
	Elf32_Word	d_symtabno;	/* dynamic symbol count */
	Elf32_Word	d_packageno;	/* package table count */
	Elf32_Word	d_impackno;	/* import package count */
	Elf32_Word	d_expackno;	/* export package count */
	Elf32_Word	d_impsymno;	/* import symbol count */
	Elf32_Word	d_expsymno;	/* export symbol count */
	unsigned long	d_base_address;	/* linked base address of so */
	unsigned long	text_start;	/* virtual address of text segment */
	unsigned long	text_size;	/* size of text segment */
	unsigned long	data_start;	/* virtual address of data segment */
	unsigned long	data_size;	/* size of data segment */
	unsigned long	bss_start;	/* virtual address of bss segment */
	unsigned long	bss_size;	/* size of bss segment */
	unsigned long	rdata_start;	/* start of rdata trunc to page */
	unsigned long	rdata_size;     /* size of rdata rounded to top */
#ifdef	PARANOID
	char		*filename;	/* Name of file being loaded */
#endif
};

typedef struct coff_module_handle  *coff_module_handle_t;

/*
 * Check for a non-NULL loader module handle.
 */
#define CHECK_COFF_HANDLE(h) \
	do { \
		if (h == NULL) return LDR_ENOEXEC; \
	} while (0)

/* Dynamic relocation routine */
extern int
coff_dynamic_relocate(coff_module_handle_t handle,
		      int nimports, ldr_symbol_rec *imports,
		      int nregions, ldr_region_rec *regions);

#if 0
/* don't need this as long as dynamic loader is built as a PIC shared lib */
/* Bridge to PIC code */
extern void
coff_PIC_bridge(void *func);
#endif

#endif /* __H_COFF_LDR */
