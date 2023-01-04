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
/* $Copyright: |
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 * $ */
/* $Header: /usr/sde/osf1/rcs/os/src/usr/include/alpha/obj.h,v 1.1.9.2 1993/12/15 22:12:52 Thomas_Peterson Exp $ */

#ifndef _OBJ_H

#define _OBJ_H

#include <a.out.h>
#include <elf_abi.h>
#include <elf_mips.h>
#include <sex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "obj_type.h"
#include "obj_list.h"
#include "obj_ext.h"

#define global
#define OBJ_CONTINUE	-1
#define OBJ_FAIL	-1

#define OT_NONE		1
#define OT_MIPS_COFF	2
#define OT_MIPS_ELF	3

#define OM_READ		1
#define OM_EXECUTE	2
#define OM_WRITE	3

typedef struct {
	unsigned long	ipd;
	unsigned long	adr;
} dbx_proctbl;

typedef struct obj {
	int		o_target_swapped:1;
	char            o_arch;
	struct stat	o_statb;
	unsigned long	o_type;		/* COFF, whatever */
	FILHDR 		*o_pfilehdr;    /* COFF headers */
	AOUTHDR 	*o_paouthdr;
	SCNHDR		*o_pscnhdr;
	pHDRR		o_phdrr;	/* symbol table header */
	pFDR		o_pfdr;		/* file descriptors */
	pPDR		o_ppdr;		/* proc descriptors */
	pSYMR		o_psymr;	/* local symbols */
	pEXTR		o_pextr;	/* external symbols */
	char		*o_pssext; 	/* external string table */
	char		*o_pss;		/* local string table */
	char		*o_pline;	/* compress line numbers */
	pRFDT 		o_prfd;		/* relative file descriptors */
	pAUXU		o_pauxu;	/* auxiliaries */
	char		*o_praw;	/* raw data */
	unsigned long	o_type_base; 	/* objlist symbol counts */
	unsigned long	o_symbol_base;
	unsigned long	o_file_base;
	unsigned long	o_procedure_base;
	unsigned long	o_external_base;
	char		*o_name; 	/* name of object (from open or liblist) */
	Elf32_Ehdr	*o_pelfhdr; 	/* ELF headers */
	Elf32_Phdr	*o_pproghdr;
	Elf32_Shdr	*o_psecthdr;
	/* Fields for rld */
	char		*o_path;	/* full path to object */
	char		*o_soname;	/* name of object (from dynamic) */
	int		o_fd;		/* fd from EXECFD */
	unsigned long	o_base_address; /* start of first segment */
	unsigned long	o_text_start;	/* text start */
	unsigned long	o_text_size;    /* size of text in bytes */
	unsigned long	o_data_start;	/* data start */
	unsigned long	o_bss_start;	/* bss start */
	unsigned long	o_bss_size;	/* size of bss in bytes */
	Elf32_Addr	o_entry;	/* entry point */

	Elf32_Addr	*o_base;	/* pointer to base of text */
	Elf32_Word	*o_hash;	/* pointer to hash table */
	char		*o_str;	        /* pointer to the string table */
	Elf32_Sym	*o_dsym;	/* pointer to the dynsym table */
	Elf32_Msym	*o_msym;	/* pointer to the msym table */
	Elf32_Got	**o_gots;	/* pointer to multiple local gots */
	Elf32_Got	**o_extgots;	/* pointer to multiple external gots */
	Elf32_Rel	*o_rel; 	/* pointer to the reloc table */
	Elf32_Lib	*o_libl;	/* pointer to the liblist table */
	Elf32_Conflict	*o_conf;	/* pointer to the conflict table */
	Elf32_Word	o_dyflags;	/* flags from dynamic table */
	Elf32_Word	o_ngots;	/* number of got tables */
	Elf32_Word	o_mgotsym;	/* first dynsym that needs multigot */
	char		*o_rpath;	/* directory path */

	Elf32_Word 	o_rldver;	/* rld version */
	Elf32_Word	o_tstamp;	/* time stamp */
	Elf32_Word	o_ichksum;	/* interface checksum */
	Elf32_Word	o_iversion;	/* interface version */

	Elf32_Word	o_symcount;	/* symbol count */
	Elf32_Word	o_syent;	/* symbol table entry size */
	Elf32_Word	o_stsize;	/* string table size */
	Elf32_Word	o_rlsize;	/* relocation table size */
	Elf32_Word	o_rlent;	/* relocation table entry size */
	Elf32_Word	o_llcount;	/* liblist count */
	Elf32_Word	o_htsize;	/* hash table size */
	Elf32_Word	o_cfcount;	/* conflict table size */

	Elf32_Word	o_rldflag;	/* flags for rld */
	Elf32_Word	o_flag; 	/* flags for libmld */
	Elf32_Word	o_mode;		/* see OM_ */
	Elf32_Addr      o_init;	        /* address of .init section */
        Elf32_Addr      o_fini;         /* address of .fini section */
	Elf32_Word	o_unrefextno;	/* index of first unreferenced ext sym */
	Elf32_Word	o_gotsym;	/* index of first sym that has a GOT entry */
	unsigned long   o_rld_map;      /* contains the address where
                                         * pObj_Head from RLD should be
                                         * written into
                                         */
#ifdef  _USE_MMAP
	caddr_t         o_mapaddr;
	size_t          o_maplen;
#endif
        dbx_proctbl 	*o_dbx_ptbl;	/* side proc table sorted for b-search*/

} OBJ, *pOBJ;
#define cbOBJ sizeof(OBJ)

#ifdef _OBJ_USE_MACRO
#define obj_otype(obj) ((obj)->o_type)
#define obj_pfilehdr(obj) ((obj)->o_pfilehdr)
#define obj_paouthdr(obj) ((obj)->o_paouthdr)
#define obj_pscnhdr(obj) ((obj)->o_pscnhdr)

#define obj_phdrr(obj) ((obj)->o_phdrr)
#define obj_psymr(obj) ((obj)->o_psymr)
#define obj_pextr(obj) ((obj)->o_pextr)

/* Dbx set/get macros */
#define obj_symbol_base(obj)	((obj)->o_symbol_base)
#define obj_file_base(obj)	((obj)->o_file_base)
#define obj_procedure_base(obj)	((obj)->o_file_base)
#define obj_type_base(obj)	((obj)->o_type_base)

/* Rld set/get macros */
#define obj_base_address(obj) ((obj)->o_base_address)
#define obj_set_base_address(obj,x) (obj_base_address(obj) = (unsigned long)(x))
#define obj_map_address(obj) ((unsigned long)(obj)->o_praw)
#define obj_set_map_address(obj,x) ((obj)->o_praw = (char *)(x))
#define obj_map_diff(obj) ((unsigned long)(obj_base_address(obj) - obj_map_address(obj)))
#define obj_map_diff_dbx(obj) (obj->o_mode != OM_EXECUTE ? 0 : (unsigned long)(obj_base_address(obj) - obj_map_address(obj)))
#define obj_hash(obj) ((obj)->o_hash)
#define obj_set_hash_address(obj,x) (obj_hash(obj) = (Elf32_Addr *)(x))
#define obj_dynstr(obj) ((obj)->o_str)
#define obj_set_dynstr_address(obj,x) (obj_dynstr(obj) = (char *)(x))
#define obj_dynsym(obj) ((obj)->o_dsym)
#define obj_set_dynsym_address(obj,x) (obj_dynsym(obj) = (Elf32_Sym *)(x))
#define obj_msym(obj) ((obj)->o_msym)
#define obj_set_msym_address(obj,x) (obj_msym(obj) = (Elf32_Msym *)(x))
/* #define obj_got(obj)  ((obj)->o_got) */
/* #define obj_set_got_address(obj,x) (obj_got(obj) = (Elf32_Got *)(x)) */
#define obj_dynrel(obj) ((obj)->o_rel)
#define obj_set_dynrel_address(obj,x) ((obj_dynrel(obj)) = (Elf32_Rel *)(x))
#define obj_liblist(obj) ((obj)->o_libl)
#define obj_set_liblist_address(obj,x) (obj_liblist(obj) = (Elf32_Lib *)(x))
#define obj_conflict(obj) ((obj)->o_conf)
#define obj_set_conflict_address(obj,x) (obj_conflict(obj) = (Elf32_Conflict *)(x))
/* #define obj_locgotcount(obj) ((obj)->o_locgotno) */
/* #define obj_set_locgotcount(obj,x) (obj_locgotcount(obj) = (x)) */
#define obj_unrefextno(obj) ((obj)->o_unrefextno)
#define obj_set_unrefextno(obj,x) (obj_unrefextno(obj) = (x))
#define obj_gotsym(obj) ((obj)->o_gotsym)
#define obj_set_gotsym(obj,x) (obj_gotsym(obj) = (x))
#define obj_timestamp(obj) ((obj)->o_tstamp)
#define obj_set_timestamp(obj,x) (obj_timestamp(obj) = (Elf32_Word)(x))
#define obj_ichecksum(obj) ((obj)->o_ichksum)
#define obj_set_ichecksum(obj,x) (obj_ichecksum(obj) = (Elf32_Word)(x))
#define obj_iversion(obj) ((obj)->o_iversion)
#define obj_set_iversion(obj,x) (obj_iversion(obj) = (Elf32_Word)(x))
#define obj_dynflags(obj) ((obj)->o_dyflags)
#define obj_set_dynflags(obj,x) (obj_dynflags(obj) = (Elf32_Word)(x))
#define obj_dynrelsz(obj) ((obj)->o_rlsize)
#define obj_set_dynrelsz(obj,x) (obj_dynrelsz(obj) = (Elf32_Word)(x))
#define obj_dynrelent(obj) ((obj)->o_rlent)
#define obj_set_dynrelent(obj,x) (obj_dynrelent(obj) = (Elf32_Word)(x))
#define obj_dynsymcount(obj) ((obj)->o_symcount)
#define obj_set_dynsymcount(obj,x) (obj_dynsymcount(obj) = (Elf32_Word)(x))
#define obj_dynsyment(obj) ((obj)->o_syent)
#define obj_set_dynsyment(obj,x) (obj_dynsyment(obj) = (Elf32_Word)(x))
#define obj_dynstrsz(obj) ((obj)->o_stsize)
#define obj_set_dynstrsz(obj,x) (obj_dynstrsz(obj) = (Elf32_Word)(x))
#define obj_liblistcount(obj) ((obj)->o_llcount)
#define obj_set_liblistcount(obj,x) (obj_liblistcount(obj) = (Elf32_Word)(x))
#define obj_conflictcount(obj) ((obj)->o_cfcount)
#define obj_set_conflictcount(obj,x) (obj_conflictcount(obj) = (Elf32_Word)(x))
/* #define obj_extgot(obj) ((obj)->o_extgot) */
/* #define obj_set_extgot(obj,x) (obj_extgot(obj) = (Elf32_Got *)(x)) */
#define obj_rpath(obj) ((obj)->o_rpath)
#define obj_set_rpath(obj,x) (obj_rpath(obj) = (char *)(x))
#define obj_soname(obj) ((obj)->o_soname)
#define obj_set_soname(obj,x) (obj_soname(obj) = (char *)(x))
#define obj_rldversion(obj) ((obj)->o_rldver)
#define obj_set_rldversion(obj,x) (obj_rldversion(obj) = (x))

#define obj_text_start(obj) ((obj)->o_text_start)
#define obj_set_text_start(obj,x) (obj_text_start(obj) = (Elf32_Addr)(x))
#define	obj_text_size(o)	((o)->o_text_size)
#define obj_set_text_size(o,x)	(obj_text_size(o) = (Elf32_Word) (x))
#define obj_data_start(obj) ((obj)->o_data_start)
#define obj_set_data_start(obj,x) (obj_data_start(obj) = (Elf32_Addr)(x))
#define	obj_data_size(obj) (obj_bss_start(obj) - obj_data_start(obj))
#define obj_bss_start(obj) ((obj)->o_bss_start)
#define obj_set_bss_start(obj,x) (obj_bss_start(obj) = (Elf32_Addr)(x))
#define	obj_bss_size(o)	((o)->o_bss_size)
#define	obj_set_bss_size(o,x)	(obj_bss_size(o) = (x))
#define obj_name(obj) ((obj)->o_name)
#define obj_set_name(obj,x) (obj_name(obj) = (char *)(x))
#define obj_path(obj) ((obj)->o_path)
#define obj_set_path(obj,x) (obj_path(obj) = (char *)(x))
#define obj_fd(obj) ((obj)->o_fd)
#define obj_set_fd(obj,x) (obj_fd(obj) = (x))

#define obj_init_address(obj) ((obj)->o_init)
#define obj_set_init_address(obj,x) (obj_init_address(obj) = (Elf32_Addr)(x))

#define obj_fini_address(obj) ((obj)->o_fini)
#define obj_set_fini_address(obj,x) (obj_fini_address(obj) = (Elf32_Addr)(x))

#define obj_entry_address(obj) ((obj)->o_entry)
#define obj_set_entry_address(obj,x) (obj_entry_address(obj) = (Elf32_Addr) (x))

#define obj_rld_map(obj) ((obj)->o_rld_map)
#define obj_set_rld_map(obj,x) (obj_rld_map(obj) = (unsigned long) (x))

#endif
/* o_rldflag field values, they are powers of two */
#define	OF_NONE		0x0000		/* object is in the the list */
#define	OF_MAPPED	0x0001		/* object is mapped flag */
#define	OF_MODIFIED	0x0002		/* object has been modified */
#define OF_TSTMPCHG	0x0004		/* object's timestamp has been modified */
#define OF_CHKSUMCHG	0x0008		/* object's checksum has been modified */
#define OF_MOVED	0x0010		/* object has been moved */
#define OF_POSTTST	0x0020		/* object follows a timestamp changed obj */
#define OF_POSTCSUM	0x0040		/* object follows a checksum changed obj */
#define OF_POSTMOVED	0x0080		/* object follows a moved obj */

#define FOREIGN_OBJ	-1

#ifdef _OBJ_USE_MACRO
#define obj_rldflags(o) ((o)->o_rldflag)
#define obj_set_rldflag(o,x) (obj_rldflags(o) |= (x))
#define obj_unset_rldflag(o,x) (obj_rldflags(o) &= ~(x))
#define obj_is_mapped(o) (obj_rldflags(o)&OF_MAPPED)

#define obj_was_modified(o) (obj_rldflags(o)&OF_MODIFIED)

#define obj_chksum_changed(o) (obj_rldflags(o)&OF_CHKSUMCHG)

#define obj_was_moved(o) (obj_rldflags(o)&OF_MOVED)

#define obj_followed_csc(o) (obj_rldflags(o)&OF_POSTCSUM)

/* Dynamic string indices -> char * */
#define obj_dynstrtab(o)   ((o)->o_str)
#define obj_dynstring(o,i) ((char *)(obj_dynstrtab(o)+i))

/* Dynamic symbol manipulation */
/* caps leftover from rld */
#define obj_conflict_foreign(o)	(obj_conflictcount(o) == FOREIGN_OBJ)
#define obj_liblist_foreign(o)	(obj_liblistcount(o) == FOREIGN_OBJ)

#define	obj_dynsym_value(o,i)	((o)->o_dsym[(i)].st_value)
#define	obj_dynsym_size(o,i)	((o)->o_dsym[(i)].st_size)
#define	obj_sym_shndx(o,i)	((o)->o_dsym[(i)].st_shndx)
#define	obj_dynsym_name(o,i)	(obj_dynstring(o,(o)->o_dsym[(i)].st_name))
#define	obj_sym_info(o,i)	((o)->o_dsym[(i)].st_info)
#endif


#define NOMSYM	((Elf32_Msym *)0)


#ifdef _OBJ_USE_MACRO
#define obj_msym_exists(o)	(((o)->o_msym != NOMSYM))
#define obj_msym_not_exists(o)	(((o)->o_msym == NOMSYM))
#define	obj_dynsym_hash_value(o,i)   ((obj_msym_exists(o) && ((o)->o_msym[i].ms_hash_value)) ? (o)->o_msym[i].ms_hash_value : get_dynsym_hash_value((o),i))
#define	obj_nbucket(o)	((o)->o_hash[0])
#define	obj_nchain(o)	((o)->o_hash[1])
#define	obj_hash_bucket(o,i)	((o)->o_hash[(i+2)])
#define	obj_hash_chain(o,i)	((o)->o_hash[(i)+obj_nbucket(o)+2])

/* #define	obj_dynsym_got(o,i)	((o)->o_extgot[(i-obj_gotsym(o))].g_index) */
/* #define obj_set_dynsym_got(o,i,x) (obj_dynsym_got(o,i) = (x)) */
/* #define	obj_locgot(o,i)	((o)->o_got[(i)].g_index) */

/* #define obj_set_local_got(o,i,x) (obj_locgot(o,i) = (x)) */
#define	obj_dynsym_rel_index(o,i)	ELF32_MS_REL_INDEX((o)->o_msym[(i)].ms_info)
#define	obj_msym_ms_info(o,i)	((o)->o_msym[(i)].ms_info)
#define	obj_set_msym_ms_info(o,i,x)	((o)->o_msym[(i)].ms_info = (x))
#define	obj_msym_ms_hash_value(o,i)	((o)->o_msym[(i)].ms_hash_value)
#define	obj_set_msym_ms_hash_value(o,i,x)	((o)->o_msym[(i)].ms_hash_value = (x))
#endif

#ifdef _OBJ_USE_MACRO
#define obj_rel_off(o,i)	((o)->o_rel[(i)].r_offset)
#define	obj_rel_info(o,i)	((o)->o_rel[(i)].r_info)
#define	obj_conflict_symbol(o,i)	((o)->o_conf[(i)].c_index)
#define obj_liblist_name(o,i)   (obj_dynstring(o,obj_liblist(o)[(i)].l_name))
#define	obj_liblist_tstamp(o,i)	((o)->o_libl[(i)].l_time_stamp)
#define	obj_liblist_csum(o,i)	((o)->o_libl[(i)].l_checksum)
#define	obj_liblist_version_str(o,i)	((o)->o_str+(o)->o_libl[(i)].l_version)
#define	obj_liblist_version(o,i)	((o)->o_libl[(i)].l_version)
#define	obj_liblist_flags(o,i)	((o)->o_libl[(i)].l_flags)
#define	obj_interface_version(o)	((o)->o_str+(o)->o_iversion)

#define obj_interface_not_match(comp,obj,i) \
                      (strcmp(obj_liblist_version_str(obj,i), \
			      obj_interface_version(comp)))

#define obj_checksum_not_match(comp,obj,i) \
                      (obj_liblist_csum(obj,i) != comp->o_ichksum)
#define obj_name_not_match(comp,obj,i) \
                      (strcmp(obj_liblist_name(obj,i), comp->o_soname))

#define obj_tstamp_not_match(comp,obj,i) \
                      (obj_liblist_tstamp(obj,i) != comp->o_tstamp)


#define obj_different_name(oa,ob)	(strcmp(oa->o_soname, ob->o_soname))

/* Elf fields */
#define obj_pelfhdr(obj) ((obj)->o_pelfhdr)
#define obj_pproghdr(obj) ((obj)->o_pproghdr)
#define obj_psecthdr(obj) ((obj)->o_psecthdr)

#define obj_section(obj,x) (obj_psecthdr(obj)[x])
#define obj_shstrndx(obj)  (obj_pelfhdr(obj)->e_shstrndx)
#define obj_section_index_name(obj,x) \
                   ((char *) (obj_shstrndx(obj) ? \
			      (obj)->o_praw + \
			      obj_section(obj,obj_shstrndx(obj)).sh_offset + \
			      obj_section(obj,x).sh_name : "N/A"))
#define obj_section_name(obj,section) \
                   ((char *)((obj)->o_praw + \
			     obj_section(obj,obj_shstrndx(obj)).sh_offset + \
			     (section)->sh_name))
#define obj_section_bits(obj,section) \
    ((char *)(&((obj)->o_praw[(section)->sh_offset])))
#define obj_section_index_bits(obj,x) \
    ((char *)(&((obj)->o_praw[obj_section(obj,x).sh_offset])))
#endif

/*
 * New macros and functions for multiple GOT handling.
 * Arguments use these conventions:
 *
 *	o   object manipulated (pObj)
 *	n   got number (in the range 0 <= n < obj_mgot_count(o))
 *	x   new value of element
 *	i   index in table
 *
 */

_BEGIN_CPLUSPLUS
extern Elf32_Addr obj_set_dynsym_got_fcn(struct obj *o, int i, Elf32_Addr x);
extern Elf32_Addr obj_dynsym_got_fcn(struct obj *o, int i);
_END_CPLUSPLUS

#ifdef _OBJ_USE_MACRO

/* Number of GOTs in this object.  Older objects will have just one GOT */
#define obj_mgot_count(o) 		((o)->o_ngots)

/* Set the value used by obj_mgot_count */
#define obj_set_mgot_count(o,x)		((o)->o_ngots = (x))

/* Set the table accessed by obj_mgot_local, obj_set_mgot_local */
#define obj_set_mgot_local_table(o,x)	((o)->o_gots = (Elf32_Got **)(x))

/* Set the table accessed by obj_mgot_external, obj_set_mgot_external */
#define obj_set_mgot_external_table(o,x) ((o)->o_extgots = (Elf32_Got **)(x))

/* Get the index of the local entries in the N'th GOT */
#define obj_mgot_local(o,n) 		((o)->o_gots[(n)])

/* Set the index of the local entries in the N'th GOT */
#define obj_set_mgot_local(o,n,x) 	((o)->o_gots[(n)] = (Elf32_Got *)(x))

/* Get the index of the external entries in the N'th GOT */
#define obj_mgot_external(o,n) 		((o)->o_extgots[(n)])

/* Set the index of the external entries in the N'th GOT */
#define obj_set_mgot_external(o,n,x) 	\
	((o)->o_extgots[(n)] = (Elf32_Got *)(x))

/* Get the number of local entries in the N'th GOT */
#define obj_mgot_local_count(o,n)	\
	((o)->o_extgots[(n)] - (o)->o_gots[(n)])

/* Get the number of external entries in the N'th GOT */
#define obj_mgot_external_count(o,n)	\
	((o)->o_gots[(n)+1] - (o)->o_extgots[(n)])

/* Get the GOT entry for the I'th local symbol in the N'th GOT */
#define	obj_mgot_local_entry(o,n,i) 	((o)->o_gots[(n)][(i)].g_index)

/* Set the GOT entry for the I'th local symbol in the N'th GOT */
#define obj_set_mgot_local_entry(o,n,i,x) \
	((o)->o_gots[(n)][(i)].g_index = (x))

/* Get the GOT entry for the I'th external symbol in the N'th GOT */
#define	obj_mgot_external_entry(o,n,i) 	((o)->o_extgots[(n)][(i)].g_index)

/* Set the GOT entry for the I'th external symbol in the N'th GOT */
#define obj_set_mgot_external_entry(o,n,i,x) \
	((o)->o_extgots[(n)][(i)].g_index = (x))

/* Get the sym index of the first symbol beyond the first GOT.  Having this
 * separate 'cached' field makes obj_dynsym_got() fast for the typical case. */
#define obj_mgot_first_multisym(o)	((o)->o_mgotsym)

/* Set the sym index of the first symbol beyond the first GOT */
#define obj_set_mgot_first_multisym(o,x) ((o)->o_mgotsym = (x))

/* Get the GOT entry for the symbol index I */
#define	obj_dynsym_got(o,i)		\
	(((i)<obj_mgot_first_multisym(o)) ? \
	 (o)->o_extgots[0][(i)-obj_gotsym(o)].g_index: obj_dynsym_got_fcn(o,i))

/* Set the GOT entry for the symbol index I */
#define obj_set_dynsym_got(o,i,x)	\
	(((i)<obj_mgot_first_multisym(o)) ? \
	 ((o)->o_extgots[0][(i)-obj_gotsym(o)].g_index = (x)) : \
	 obj_set_dynsym_got_fcn(o,i,x))

#else  /* _OBJ_USE_MACRO */

#define	obj_dynsym_got(o,i)		obj_dynsym_got_fcn(o,i)
#define obj_set_dynsym_got(o,i,x)	obj_set_dynsym_got_fcn(o,i,x)

#endif /* _OBJ_USE_MACRO */

#ifndef _OBJ_NO_MGOT
/*
 * Force compile errors for use of old macros or functions, since the
 * caller must be changed to be aware of the possibility of multiple GOTs.
 * The error message will give an indication of the cure.
 * Calls to obj_set_locgotcount() can be removed - obj_locgotcount()
 * no longer addresses a separate field, but is derived from the got
 * locations.
 *
 */
#define obj_got(obj)			ERROR___use___obj_mgot_local
#define obj_set_got_address(obj,x)	ERROR___use___obj_set_mgot_local
#define obj_locgotcount(obj)		ERROR___use___obj_mgot_local_count
#define obj_set_locgotcount(obj,x)	ERROR___NA____obj_set_locgotcount
#define obj_extgot(obj)			ERROR___use___obj_mgot_external
#define obj_set_extgot(obj,x)		ERROR___use___obj_set_mgot_external
#define obj_locgot(o,i)			ERROR___use___obj_mgot_local_entry
#define obj_set_local_got(o,i,x)	ERROR___use___obj_set_mgot_local_entry

#else  /* _OBJ_NO_MGOT */
/*
 * When _OBJ_NO_MGOT is defined:
 * These macros are available for backward compatibility and tool
 * conversion, but will ONLY work correctly if the object does not
 * have multiple GOTs.  To guarantee they are used safely, a check
 * is done on each call.  If the check fails, _MGOT_ERROR is called
 * with the obj pointer as an argument.  This is assumed to be a macro
 * or function defined by the caller that will print an error message
 * that is appropriate for the application.  An example:
 *
 *    void _MGOT_ERROR(pOBJ o) {
 *	printf("Error: obj %s has multiple gots\n", obj_name(o)); exit(1); }
 *
 */
#define _MGOT_CHK(obj)			\
	((obj_mgot_count(obj)>1) ? _MGOT_ERROR(obj):0)

#define obj_got(obj)			\
	(_MGOT_CHK(obj),obj_mgot_local(obj, 0))

#define obj_set_got_address(obj,x)	\
	(_MGOT_CHK(obj),obj_set_mgot_local(obj, 0, x))

#define obj_locgotcount(obj)		\
	(_MGOT_CHK(obj), obj_mgot_local_count(obj, 0))

#define obj_extgot(obj)			\
	(_MGOT_CHK(obj), obj_mgot_external(obj, 0))

#define obj_set_extgot(obj,x)		\
	(_MGOT_CHK(obj), obj_set_mgot_external(obj, 0, x))

#define obj_locgot(o,i)			\
	(_MGOT_CHK(o), obj_mgot_local_entry(o, 0, i))

#define obj_set_local_got(o,i,x)	\
	(_MGOT_CHK(o), obj_set_mgot_local_entry(o, 0, i, x))

#define obj_set_locgotcount(o,i)	(_MGOT_CHK(o),0)  /* No-op */
#endif /* _OBJ_NO_MGOT */

#ifndef _ELF
#define _RHEADER	".rheader"


#ifdef _OBJ_USE_MACRO
#define hdr_symptr(pobj) (pobj->o_pfilehdr->f_symptr)

#define obj_section_vaddr(obj, psection) (psection->s_vaddr)
#define obj_section_size(obj, psection) (psection->s_size)
/* obj_section_name also defined above for ELF */
#undef obj_section_name
#define obj_section_name(obj, psection) (psection->s_name)
#define procedure_lnlow(obj, procedure) (obj->o_ppdr[procedure-obj->o_procedure_base].lnLow)
#define procedure_lnhigh(obj, procedure) (obj->o_ppdr[procedure-obj->o_procedure_base].lnHigh)

#endif
#endif /* _ELF */




#endif 
