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
 *	@(#)$RCSfile: mandatory.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:17:41 $
 */ 
/*
 */
#if SEC_BASE && SEC_MAC
#ifndef __MANDATORY__
#define __MANDATORY__

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*

 * Based on:

 *
 * Copyright (c) 1988-1989 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 *
 * Include file for mandatory access control
 *
 * This file contains definitions for all major mandatory access control
 * data structures and routine declarations.  It must be preceded by a
 * #include of <sys/security.h>.  The data structures are for
 * classifications, categories, synonyms, and mandatory policy parameters.
 */

#if defined(AUX) || defined(BSD)
#include <sys/dir.h>
#else
#include <dirent.h>
#endif

#if SEC_ILB
#include <std_labels.h>
#endif

/* Types for classifications and categories and sensitivity labels */

typedef unsigned long	class_ir_t;	/* 32 bit classification */

typedef struct mand_ir {
	class_ir_t	class;
	mask_t		cat[1];
}  mand_ir_t;

#if SEC_ILB
typedef mand_ir_t	ilb_ir_t;
#endif

#if SEC_MAC_OB

/* structure of the synonym database file:
 *    putw (number of synonyms)
 *    putw (number of category masks)	each of these is CATWORDS words
 *    putw (number of string characters)
 *    syn_file structures
 *    category masks
 *    string table
 *
 * The index of category mask (starting at zero) is stored in syn.cat_set.
 * The index of the synonym name in the string table (starting at zero) is
 * stored in syn.name.
 * syn.type is CLASS_SYN, CATEGORY_SYN, or SENS_LABEL_SYN.
 * syn.class is an integer between 1 and mand_max_class.
 */

struct syn_file {
	int	name;
	int	type;
	int	cat_set;
	int	class;
};

#define	CLASS_SYN	1	/* synonym for a classification */
#define CATEGORY_SYN	2	/* synonym for a category set */
#define SENS_LABEL_SYN	3	/* synonym for a sensitivity label */

#endif /* SEC_MAC_OB */

/* following definition tells how many words required to represent the
 * categories on this system.
 */

#define CATWORDS	(WORD_OF_BIT(mand_max_cat) + 1)
#if SEC_ILB
#define	MARKWORDS	(WORD_OF_BIT(mand_max_mark) + 1)
#endif

#define MAND_INVALID_CLASS	(-1)
#define MAND_INVALID_CAT	(-1)

/* definition of parameters from the mandatory policy file */

struct mand_config {
	char	dbase[50];
	ulong	cache_size;
	ulong	buffers;
	ushort	subj_tags;
	ushort	obj_tags;
	ushort	first_subj_tag;
	ushort	first_obj_tag;
	dev_t	minor_device;
	ushort	policy;
};

extern 	struct mand_config mand_config;	/* configuration parms */

/* Parameters of classifications and categories: */
extern	unsigned	mand_max_class;	/* maximum numerical class */
extern	unsigned	mand_max_cat;	/* maximum numerical category */
#if SEC_ILB
extern	unsigned	mand_max_mark;	/* maximum numerical marking */
extern	ilb_ir_t	*mand_syshi;	/* system high info label */
extern	ilb_ir_t	*mand_syslo;	/* system low  info label */
#else
extern	mand_ir_t	*mand_syshi;	/* system high sens. label */
extern	mand_ir_t	*mand_syslo;	/* system low  sens. label */
#endif

#if SEC_ENCODINGS

extern	mand_ir_t	*mand_clrnce;		/* user's clearance */
extern	mand_ir_t	*mand_minclrnce;	/* system minimum clearance */
extern	mand_ir_t	*mand_minsl;		/* minimum useful SL */

#else

#define	MAND_CLASS_FILE	"/etc/policy/mac/classes"
#define MAND_CAT_FILE	"/etc/policy/mac/categories"
#define MAND_SYN_FILE	"/etc/policy/mac/synonyms"
#define MAND_SYN_DB	"/etc/policy/mac/synonyms.db"
#define MAND_PARAM_FILE	"/etc/policy/mac/config"

#endif

#define MAND_EXTENSION	":t"		/* when re-writing new files */

/* decision values upon comparing the relationship of two labels */

#define	MAND_SDOM	1
#define	MAND_ODOM	2
#define	MAND_EQUAL	4
#define	MAND_INCOMP	8
#if SEC_ILB && !defined(ILB_SDOM)
#define	ILB_SDOM	0x10
#define	ILB_ODOM	0x20
#define	ILB_SAME	0x40
#endif

/* different ways to traverse a multilevel directory */

#define	MAND_MLD_ALLDIRS	0
#define	MAND_MLD_MANDLEVEL	1

/* definition to use for traversal of multilevel directories (see mld(3)) */

struct multdir  {
	char *name;
	char sdirname[NAME_MAX+1];
	DIR *mdir;
	DIR *sdir;
	int technique;
	mand_ir_t *ir;
};
typedef struct multdir MDIR;


/* functions for mandatory access control representations */

#if SEC_MAC_OB
char	*mand_cltoname();		/* return a name given a number */
int	mand_nametocl();		/* return a number given a name */
char	*mand_cattoname();		/* return a name given a number */
int	mand_nametocat();		/* return a number given a name */
void	mand_lookup_and_print_syn();	/* lookup and print a synonym */
#endif /* SEC_MAC_OB */
mand_ir_t	*clearance_er_to_ir();	/* map clearance external to internal */
char	*clearance_ir_to_er();		/* map clearance internal to external */
mand_ir_t	*mand_er_to_ir();	/* map external to internal */
char	*mand_ir_to_er();		/* map internal to external */
mand_ir_t	*mand_alloc_ir();	/* allocate a mandatory structure */
void	mand_free_ir();			/* free memory for structure */
void	mand_end();			/* free storage used by databases */
int	mand_init();			/* init parms but not databases */
int mand_ir_to_tag();			/* convert IR to tag */
int mand_tag_to_ir();			/* convert tag to IR */
int mand_ir_relationship();		/* compare two IRs */
int mand_tag_relationship();		/* compare two tags */
MDIR *openmultdir();			/* open a MLD for traversal */
void closemultdir();			/* close a MLD */
void rewindmultdir();			/* reset MLD to beginning */
void readmultdir();			/* read a name from a MLD */
char *ir_to_subdir();			/* find subdir in a MLD with given IR */
char		*mand_convert();	/* generic ir to er conversion */
#if SEC_ILB
ilb_ir_t	*mand_parse();		/* generic er to ir conversion */
#else
mand_ir_t	*mand_parse();		/* generic er to ir conversion */
#endif

#if SEC_ILB
ilb_ir_t	*ilb_er_to_ir();	/* map external to internal */
char		*ilb_ir_to_er();	/* map internal to external */
ilb_ir_t	*ilb_alloc_ir();	/* allocate ILB structure */
void		ilb_free_ir();		/* deallocate ILB structure */

/* definitions for the irtype argument to mand_convert() */

#define	MAND_IL	0
#endif
#define	MAND_SL	1

/* definitions for the tables to be used by mand_convert() and mand_parse() */

#define MAND_CLEARANCE_TABLE	1
#define MAND_IL_TABLE		2
#define MAND_SL_TABLE		3
#define MAND_BANNER_TABLE	4
#define MAND_CHANNEL_TABLE	5

/* the number of bytes in a sensitivity label */

#define mand_bytes() (sizeof(class_ir_t) + (CATWORDS * sizeof(mask_t)))
#if SEC_ILB
#define	ilb_bytes() (mand_bytes() + (MARKWORDS * sizeof(mask_t)))
#endif

/* copy a sensitivity label given a source and destination pointer */

#define mand_copy_ir(from,to)	memcpy(to,from,mand_bytes())
#if SEC_ILB
#define	ilb_copy_ir(from,to)	memcpy(to,from,ilb_bytes())
#endif

#endif /* __MANDATORY__ */
#endif /* SEC_BASE && SEC_MAC */
