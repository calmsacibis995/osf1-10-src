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
 *	@(#)$RCSfile: mach_o_format.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:15 $
 */ 
/*
 */
/*
 * Copyright (c) 1990
 * Open Software Foundation, Inc.
 * 
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/* mach_o_format.h
 * This is the header file for the OSF version of the Mach-O object
 * file format.
 * 
 * NOTE:  THIS IS PRELIMINARY AND WILL CHANGE
 *
 * OSF/1 Release 1.0
 */

#ifndef _H_MACH_O_FORMAT
#define _H_MACH_O_FORMAT

/*
 * A Mach-O object file consists of 
 * - a fixed-length header  
 * - a variable-length load command map, containing only the offsets
 *     of the load commands
 * - the (variable-length) load commands themselves, in no
 *     specified order
 * - the object file's sections, in no specified order
 */


#include <mach_o_types.h>		/* machine-dependent type declarations */

#ifndef _KERNEL
#include <time.h>
#endif

/* The following typedefs are used in the format declarations. */

typedef mo_long_t	mo_lcid_t;	/* index in load command map */
typedef struct mo_addr_t {		/* addr of location in section */
	mo_lcid_t		adr_lcid;
	mo_offset_t		adr_sctoff;
} mo_addr_t;

typedef struct mo_index_t {	    /* elem index in array-type section */
	mo_lcid_t		adx_lcid;
	mo_long_t		adx_index;
} mo_index_t;

typedef struct mo_rel_addr_t {		/* addr of location rel to section */
	mo_lcid_t		adrl_lcid;
	mo_offset_t		adrl_reloff;
} mo_rel_addr_t;



/* LOAD COMMANDS
 * For now, the load commands must all be together and must immediately
 * follow the object header.  The offset of a load command is equal
 * to the preceding command's (offset + ldc_header.ldci_cmd_size),
 * so moh_size_of_cmds is the sum of all the load command sizes.
 */

/* LOAD COMMAND HEADER
 * All the load commands should be together and should immediately follow
 * mo_header.  All load commands begin with the following header even
 * if they don't correspond to sections (i.e., ldci_section_off and
 * ldci_section_len are zero).
 */

typedef struct ldc_header_t {
	mo_long_t		ldci_cmd_type;
	mo_long_t		ldci_cmd_size;
	mo_offset_t		ldci_section_off;  /* offset from BOF */
	mo_long_t		ldci_section_len;  /* length in the file */
} ldc_header_t;

/* The following macros should be used to access the fields in the
 * headers of load commands.
 */

#define ldc_cmd_type		ldc_header.ldci_cmd_type
#define ldc_cmd_size		ldc_header.ldci_cmd_size
#define ldc_section_off		ldc_header.ldci_section_off
#define ldc_section_len		ldc_header.ldci_section_len

#define VALID_LDC_HEADER_PTR(p)  VALID_MO_LONG_PTR(p)

/*
 * Definitions of machine/vendor-independent load command types --
 * the values from 0x00000001 to 0x0fffffff are reserved for this category.
 * The values from 0x10000000 to 0xffffffff are reserved for machine
 * or vendor specific load commands.  The name space for the latter
 * is determined by the object file header's moh_cpu_type field.
 */

#define LDC_UNDEFINED		0
#define LDC_CMD_MAP		1
#define LDC_INTERPRETER		2
#define LDC_STRINGS		3
#define LDC_REGION		4
#define LDC_RELOC		5
#define LDC_PACKAGE		6
#define LDC_SYMBOLS		7
#define LDC_ENTRY		8
#define LDC_FUNC_TABLE		9
#define LDC_GEN_INFO		10


/* LOAD COMMAND MAP LOAD COMMAND
 * What it is:
 * The load command map is an array whose entries are 
 * the file offsets of the load commands (type mo_offset_t).
 * When adding, deleting or moving load commands, DO NOT change the
 * correspondence between a load command map index and its load command
 * (unless you are purposely substituting one load command for another).
 * Where it is:
 * The map is in a load command and so is with the other load commands.
 * Why it is:
 * It is used to facilitate the interpretation of references from
 * one section to another and to provide flexibility for adding,
 * deleting and changing load commands.	 It is used to locate load
 * commands after they have been read in, but is not used to find
 * load commands in order to read them in.  A load command that is
 * not in the map is ignored by the loader and may be subject to
 * garbage collection.
 *
 * This load command does not have an associated section.  It ends in
 * a variable-length array whose entries are of type mo_offset_t.
 * It also identifies the strings section used by the load commands.
 */

typedef struct load_cmd_map_command_t {		/* LDC_CMD_MAP */
	ldc_header_t		ldc_header;
	mo_lcid_t		lcm_ld_cmd_strings;
	mo_long_t		lcm_nentries;
	mo_offset_t		lcm_map [1];
} load_cmd_map_command_t;

/* definition of the value for an invalid (unused) load command map entry */
#define LCM_INVALID_ENTRY	0

/* INTERPRETER LOAD COMMAND
 * The interpreter command, if present, must correspond to the first entry
 * in the load command map.   It is designed to not depend on other
 * load commands.
 *
 * This load command does not have an associated section.
 * It ends in a null-terminated string.
 */

typedef struct interpreter_command_t {		/* LDC_INTERPRETER */
	ldc_header_t		ldc_header;
	char			intc_interpreter_path [1];
} interpreter_command_t;


/* STRINGS LOAD COMMAND
 * Each object file has one or more strings sections that contain the
 * strings referenced by the load commands and the object "meta data".
 * The strings used by the load commands should be in a separate strings
 * section from the strings section(s) used by the rest of the object.
 *
 * This load command has an associated section which contains
 * null-terminated strings.
 */

typedef struct strings_command_t {		/* LDC_STRINGS */
	ldc_header_t		ldc_header;
	mo_long_t		strc_flags;
} strings_command_t;


/* REGION LOAD COMMAND
 * This is used for sections that are to be part of the running
 * program and thus are known to the format-independent loader.
 * This is equivalent to a section header.
 * In executable files, it indicates that the section is to be mapped
 * into the task's address space.
 * 
 * This load command has an associated section (called a region) 
 * which is a piece of the program.  When the ldc_section_off field
 * is zero, the associated section occupies no space in the file, 
 * and the ldc_section_len field is also zero.
 */

typedef struct region_command_t {		/* LDC_REGION */
	ldc_header_t		ldc_header;
	mo_addr_t		regc_region_name;
	union {
		mo_vm_addr_t	vm_addr;
		mo_rel_addr_t	rel_addr;
	} regc_addr;
	mo_long_t		regc_vm_size;
	mo_long_t		regc_flags;
	mo_lcid_t		regc_reloc_addr; /* id of relocation info */
						 /* command, if any */
	mo_long_t		regc_addralign;
	mo_short_t		regc_usage_type; /* text, data, bss, etc. */
	mo_short_t		regc_initprot;
} region_command_t;

/* definitions for referencing fields that are in unions */
#define regc_vm_addr		regc_addr.vm_addr
#define regc_rel_addr		regc_addr.rel_addr

/* definitions for the regc_flags field */
#define REG_ABS_ADDR_F		0x1    /* use absolute address vm_addr */
#define REG_REL_ADDR_F		0x2	/* region is to be loaded at a loc */
                                        /* relative to specified region */

/* definitions for region protection fields
 * these are generic flags; the loader must translate these into
 * the values used by the system
 */

#define MO_PROT_NONE		0x0
#define MO_PROT_READ		0x1
#define MO_PROT_WRITE		0x2
#define MO_PROT_EXECUTE		0x4

/* the default protection for newly created virtual memory */
#define MO_PROT_DEFAULT	 \
(MO_PROT_READ | MO_PROT_WRITE| MO_PROT_EXECUTE)

/* maximum privileges possible, for parameter checking */
#define MO_PROT_ALL  \
(MO_PROT_READ | MO_PROT_WRITE| MO_PROT_EXECUTE)


/* RELOC LOAD COMMAND
 * This is the "header" for a relocation section.  There is at most
 * one for every region.  Some regions don't need to be relocated.
 *
 * This load command has an associated section which is an array whose
 * entries are of type reloc_info_t.
 */

typedef struct reloc_command_t {		/* LDC_RELOC */
	ldc_header_t	ldc_header;
	mo_long_t	relc_nentries;
	mo_lcid_t	relc_owner_section;
	mo_long_t	relc_reserved;
} reloc_command_t;

typedef struct reloc_info_t {			/* relocation table entry */
	mo_offset_t		ri_roffset;	/* first byte to relocate */
	mo_short_t		ri_flags;	/* relate other fields */
	mo_short_t		ri_size_type;	/* how to fill in info */
	union { mo_index_t	symbol_index;	/* symbol being referenced */
		mo_addr_t	loc_addr;	/* internal loc referenced */
	} ri_value;
} reloc_info_t;

/* definitions for the ri_flags field */
#define RI_SYMBOL_F	0x1			/* use symbol_addr */
#define RI_LOC_F	0x2			/* use loc_addr */
#define RI_PC_REL_F	0x4			/* fill in diff from roffset */
#define RI_INDIRECT_F	0x8			/* roffset references table */
                                                /* of address constants */
/* RI_RELOC_VADDR_F and RO_RELOC_OFFSET_F are probably temporary, to
 * help in porting to the MIPS. */
#define RI_RELOC_VADDR_F 0x100	       /* roffset contains a virtual address */
#define RI_RELOC_OFFSET_F 0x200	  /* roffset contains an offset into section */


/* definitions for referencing fields that are in unions */
#define ri_symbol_index		ri_value.symbol_index
#define ri_loc_addr		ri_value.loc_addr


/* PACKAGE LOAD COMMAND
 * There are separate package lists for imports and exports, although
 * the structure is the same.
 *
 * This load command does not have an associated section.  It ends
 * in a variable-length array whose entries are of type pkg_entry_t.
 */

typedef struct pkg_entry_t {
	mo_offset_t	pe_pkg_name;
	mo_addr_t	pe_version_addr;	/* package version info */
} pkg_entry_t;

typedef struct package_command_t {		/* LDC_PACKAGE */
	ldc_header_t	ldc_header;
	mo_short_t	pkgc_flags;
	mo_short_t	pkgc_nentries;
	mo_lcid_t	pkgc_strings_id;	/* assoc. strings section */
	pkg_entry_t	pkgc_pkg_list [1];	/* list of pkg info */
} package_command_t;

/* definitions of flags for the pkgc_flags field */
#define PKG_EXPORT_F	0x1
#define PKG_IMPORT_F	0x2


/* SYMBOL INFORMATION ENTRY
 * This is used for defined symbols, imports, and stabs.  The type (kind)
 * of the associated symbols load command determines which.
 */

typedef struct symbol_info_t {
	union {mo_offset_t	symbol_name;
	       mo_ptr_t		symbol_nameP;
       } si_name;
	mo_short_t		si_package_index;
	mo_short_t		si_type;
	mo_short_t		si_flags;
	mo_byte_t		si_reserved_byte;
	mo_byte_t		si_sc_type;
	union { mo_addr_t	def_val;	/* defined section,offset */
		mo_long_t	imp_val;	/* index in import list */
		mo_long_t	lit_val;	/* literal value */
		mo_vm_addr_t	abs_val;	/* absolute value */
	} si_value;
} symbol_info_t;

/* definitions for the si_flags field */
#define SI_EXPORT_F		0x1
#define SI_IMPORT_F		0x2
#define SI_LOCAL_F		0x4
#define SI_CODE_F		0x8
#define SI_DATA_F		0x10
#define SI_LITERAL_F		0x20
#define SI_FORWARD_F		0x40			/* to another symbol */
#define SI_COMMON_F     	0x80
#define SI_LOCAL_LABEL_F	0x100
#define SI_ABSOLUTE_VALUE_F	0x200

/* definitions for referencing the fields that are in unions */
#define si_symbol_name		si_name.symbol_name
#define si_symbol_nameP		si_name.symbol_nameP
#define si_def_val		si_value.def_val
#define si_imp_val		si_value.imp_val
#define si_lit_val		si_value.lit_val
#define si_abs_val		si_value.abs_val

/* SYMBOLS LOAD COMMAND
 * This specifies (some of the) symbols for the object file.
 * There are different kinds of symbols sections; all have the same
 * format but are used for different purposes, such as imports, exports, etc.
 * Thus many object files will have several symbols sections.
 *
 * This load command has an associated section which is an array whose
 * entries are of type symbol_info_t.
 */


typedef struct symbols_command_t {		/* LDC_SYMBOLS */
	ldc_header_t	ldc_header;
	mo_short_t	symc_kind;
	mo_short_t	symc_short_reserved;
	mo_long_t	symc_nentries;
	mo_lcid_t	symc_pkg_list;
	mo_lcid_t	symc_strings_section;
	mo_lcid_t	symc_reloc_addr;
	union { mo_short_t	n_exported_symb;
		mo_long_t	long_reserved;
	} symc_other;
} symbols_command_t;

/* definitions for the symc_kind field */
#define	SYMC_IMPORTS		1
#define SYMC_DEFINED_SYMBOLS	2
#define SYMC_STABS		3

/* definitions for referencing the fields that are in unions */
#define symc_n_exported_symb	symc_other.n_exported_symb


/* ENTRY LOAD COMMAND
 * This specifies the entrypoint in a program.
 * (There may be only one such command.)
 * The absaddr field is optional, but is required when loading via kern_exec.
 *
 * This load command does not have an associated section.
 */

typedef struct entry_command_t {		/* LDC_MAIN_ENTRY */
	ldc_header_t	ldc_header;
	mo_short_t	entc_flags;
	mo_short_t	entc_short_reserved;
	mo_vm_addr_t	entc_absaddr;		/* kern_exec uses type long */
	mo_addr_t	entc_entry_pt;
} entry_command_t;

/* definitions for the entc_flags field */
#define ENT_VALID_ABSADDR_F	0x1


/* GEN_INFO LOAD COMMAND
 * This provides information about when and how the object file
 * was created.
 *
 * This load command does not have an associated section, but
 * does have strings in the (load commands') string section.
 */

typedef struct gen_info_command_t {		/* LDC_GEN_INFO */
	ldc_header_t	ldc_header;
	time_t		genc_obj_create_time;
	mo_addr_t	genc_creator_name;
	mo_addr_t	genc_creator_version;
	time_t		genc_creator_time;
	mo_addr_t	genc_options_to_creator;
} gen_info_command_t;


/* FUNC_TABLE LOAD COMMAND
 * This specifies a table of function entrypoints.  The type field
 * indicates when they are to be invoked, e.g. when the object file
 * is loaded or unloaded.
 *
 * This load command does not have an associated section since the
 * location of the code for the functions is specified by
 * region load commands and does not need to be in separate sections.
 * It ends in a variable-length array whose entries are of type mo_addr_t.
 */

typedef struct func_table_command_t {		/* LDC_FUNC_TABLE */
	ldc_header_t	ldc_header;
	mo_short_t	fntc_type;
	mo_short_t	fntc_nentries;
	mo_addr_t	fntc_table_name;
	mo_long_t	fntc_reserved;
	mo_addr_t	fntc_entry_loc [1];
} func_table_command_t;

/* definitions for the fntc_type field */
#define FNTC_INITIALIZATION	1
#define FNTC_TERMINATION	2

#endif /* _H_MACH_O_FORMAT */
