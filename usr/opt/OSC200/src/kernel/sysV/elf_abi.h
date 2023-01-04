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
 *	@(#)elf_abi.h	9.2	(ULTRIX/OSF)	10/18/91
 */ 
#ifndef _ELF_ABI_H_
#define _ELF_ABI_H_
#if defined(__LANGUAGE_C__) || defined(LANGUAGE_ASSEMBLY)
		/* Right now this includes the entire file */
/* Need to deal with LANGUAGE_PASCAL & LANGUAGE_ASSEMBLY */

/*
 * ELF data types
 */
#if defined(__LANGUAGE_C__)
typedef unsigned long	Elf32_Addr;
typedef unsigned short	Elf32_Half;
typedef unsigned long	Elf32_Off;
typedef signed long	Elf32_Sword;
typedef unsigned long	Elf32_Word;
#endif
/*
 * ELF header
 */
#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7
#define EI_NIDENT	16

#define ELFMAG0		0x7f
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

#define ELFCLASSNONE	0
#define ELFCLASS32	1	
#define ELFCLASS64	2	
#define ELFDATANONE	0	
#define ELFDATA2LSB	1	
#define ELFDATA2MSB	2

/*
 * e_ident
 * e_ident[EI_CLASS] should be ELFCLASS32
 * e_ident[EI_DATA]  should be either ELFDATA2LSB or ELFDATA2MSB
 */
#if defined(__LANGUAGE_C__)
#define IS_ELF(ehdr) ((ehdr).e_ident[EI_MAG0] == ELFMAG0 && \
		      (ehdr).e_ident[EI_MAG1] == ELFMAG1 && \
		      (ehdr).e_ident[EI_MAG2] == ELFMAG2 && \
		      (ehdr).e_ident[EI_MAG3] == ELFMAG3)

typedef struct
{
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} Elf32_Ehdr;
#endif

/*
 * e_type
 */
#define ET_NONE		0
#define ET_REL		1
#define ET_EXEC		2
#define ET_DYN		3
#define ET_CORE		4
#define ET_LOPROC	0xff00
#define ET_HIPROC	0xffff

/*
 * e_machine
 */
#define EM_NONE		0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
#define EM_88K		5
#define EM_860		6
#define EM_MIPS		8

/*
 * e_version
 */
#define EV_NONE		0
#define EV_CURRENT	1

/*
 * special section indexes
 */
#define SHN_UNDEF	0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC	0xff00
#define SHN_HIPROC	0xff1f
#define SHN_ABS		0xfff1
#define SHN_COMMON	0xfff2
#define SHN_HIRESERVE	0xffff

/*
 * section header
 */
#if defined(__LANGUAGE_C__)
typedef struct
{
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} Elf32_Shdr;
#endif
/*
 * sh_type
 */
#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL	9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

/*
 * sh_flags
 */
#define SHF_WRITE	0x1
#define SHF_ALLOC	0x2
#define SHF_EXECINSTR	0x4
#define SHF_MASKPROC	0xf0000000

/*
 * special section names
 */

#define ELF_BSS		".bss"
#define ELF_DATA	".data"
#define ELF_DEBUG	".debug"
#define ELF_DYNAMIC	".dynamic"
#define ELF_DYNSTR	".dynstr"
#define ELF_DYNSYM	".dynsym"
#define ELF_FINI	".fini"
#define ELF_GOT		".got"
#define ELF_HASH	".hash"
#define ELF_INIT	".init"
#define ELF_REL_DATA	".rel.data"
#define ELF_REL_FINI	".rel.fini"
#define ELF_REL_INIT	".rel.init"
#define ELF_REL_DYN	".rel.dyn"
#define ELF_REL_RODATA	".rel.rodata"
#define ELF_REL_TEXT	".rel.text"
#define ELF_RODATA	".rodata"
#define ELF_SHSTRTAB	".shstrtab"
#define ELF_STRTAB	".strtab"
#define ELF_SYMTAB	".symtab"
#define ELF_TEXT	".text"

/*
 * symbol table entry
 */
#if defined(__LANGUAGE_C__)
typedef struct
{
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} Elf32_Sym;
#endif



/*
 * st_info
 */
#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i) & 0x0f)
#define ELF32_ST_INFO(b,t)	(((b) << 4) + ((t) & 0x0f))

/*
 * symbol binding
 */
#define STB_LOCAL	0
#define STB_GLOBAL	1
#define STB_WEAK	2
#define STB_LOPROC	13
#define STB_HIPROC	15

/*
 * symbol type
 */
#define STT_NOTYPE	0
#define STT_OBJECT	1
#define STT_FUNC	2
#define STT_SECTION	3
#define STT_FILE	4
#define STT_LOPROC	13
#define STT_HIPROC	15

#if defined(__LANGUAGE_C__)
typedef struct {
    Elf32_Sword         d_tag;
    union {
	Elf32_Word      d_val;
	Elf32_Addr      d_ptr;
    } d_un;
} Elf32_Dyn;
#endif

#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH	15
#define DT_SYMBOLOC	16
#define DT_REL		17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23
#define DT_LOPROC	0x70000000
#define DT_HIPROC	0x7fffffff




/*
 * relocation entry
 */
#if defined(__LANGUAGE_C__)
typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
} Elf32_Rel;


/*
 * relocation entry with addend
 */
typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
} Elf32_Rela;
#endif

/*
 * r_info
 */
#define ELF32_R_SYM(i)		((i) >> 8)
#define ELF32_R_TYPE(i)		((unsigned char)(i))
#define ELF32_R_INFO(s,t)	(((s) << 8) + (unsigned char)(t))

/*
 * program header
 */
#if defined(__LANGUAGE_C__)
typedef struct
{
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr;
#endif

/*
 * p_type
 */
#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE		4
#define PT_SHLIB	5
#define PT_PHDR		6
#define PT_LOPROC	0x70000000
#define PT_HIPROC	0x7fffffff

/*
 * p_flags
 */
#define PF_X		0x1
#define PF_W		0x2
#define PF_R		0x4
#define PF_MASKPROC	0xf0000000

/* Archive macros */
#define ELF_AR_SYMTAB_NAME	"/"
#define ELF_AR_SYMTAB_NAME_LEN	1
#define ELF_AR_STRTAB_NAME	"//"
#define ELF_AR_STRTAB_NAME_LEN	2
#if defined(__LANGUAGE_C__)
#define IS_ELF_AR_SYMTAB(s) \
  ((s[0] == '/') && ((s[1] == ' ') || (s[1] == '\0')))

#define IS_ELF_AR_STRTAB(s) \
  (((s[0] == '/') && (s[1] == '/')) && ((s[2] == ' ') || (s[2] == '\0')))
#endif

#endif /* __LANGUAGE_C__ || LANGUAGE_ASSEMBLY */
#endif /* _ELF_ABI_H_ */
