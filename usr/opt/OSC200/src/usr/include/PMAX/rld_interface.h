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
 * @(#)$RCSfile: rld_interface.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/09/29 15:11:30 $
 */
/*
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
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/PMAX/rld_interface.h,v 1.1.3.2 1992/09/29 15:11:30 Al_Delorey Exp $ */

/* _RLD_OP_MODIFY_LIST's op codes */
#define _RLD_OP_NONE		0 	/* nop */
#define _RLD_OP_INSERT		1	/* insert new object 'name' before element */
#define _RLD_OP_ADD		2	/* add new object 'name' after element */
#define _RLD_OP_DELETE		3	/* delete element */
#define _RLD_OP_REPLACE		4	/* replace element with new object 'name' */

/* _rld_new_interface's op codes */
#define _SHUT_DOWN		0	/* execute all .fini sections */
#define _RLD_FIRST_PATHNAME	1	/* get to the first obj on the list */
#define _RLD_NEXT_PATHNAME	2	/* get to the next obj on the list */
#define _RLD_MODIFY_LIST	3	/* modify the current obj list */
#define _RLD_ADDR_TO_NAME	4	/* get pathname from addr of obj */
#define _RLD_NAME_TO_ADDR	5	/* get address from name of obj */

#ifdef __osf__
/*
 * Additional _rld_new_interface op codes
 * Negative numbers chosen to avoid future conflicts
 */

#define _RLD_LDR_SBRK				-1	/* sbrk() call */
#define _RLD_LDR_BRK				-2	/* brk() call */
#define _RLD_LDR_CONTEXT_ATEXIT			-3	/* run .finis */
#define _RLD_LDR_CONTEXT_REMOVE			-4	/* rmlib */
#define _RLD_LDR_CONTEXT_INSTALL		-5	/* inlib */
#define _RLD_LDR_CONTEXT_INQ_REGION		-6	/* inquire region */
#define _RLD_LDR_CONTEXT_INQ_MODULE		-7	/* inquire module */
#define _RLD_LDR_CONTEXT_NEXT_MODULE		-8	/* next mod number */
#define _RLD_LDR_CONTEXT_LOOKUP_PACKAGE		-9	/* lookup func name */
#define _RLD_LDR_CONTEXT_LOOKUP			-10	/* not implemented */
#define _RLD_LDR_CONTEXT_UNLOAD			-11	/* unload library */
#define _RLD_LDR_CONTEXT_GET_ENTRY_PT		-12	/* get entry addr */
#define _RLD_LDR_CONTEXT_LOAD			-13	/* load library */
#define _RLD_LDR_DLOPEN				-14	/* dlopen() */
#define _RLD_LDR_DLSYM				-15	/* dlsym() */
#define _RLD_LDR_DLERROR			-16	/* dlerror() */
#define _RLD_LDR_DLCLOSE			-17	/* dlclose() */

#endif /* __osf__ */
	
#if defined(__LANGUAGE_C__)

#include <elf_abi.h>

extern	char *
_rld_first_pathname();

extern char *
_rld_next_pathname();

extern char *
_rld_modify_list(Elf32_Word	operation,
		 char *original_pathname,
		 char *name);

extern char *
_rld_address_to_name(Elf32_Addr address);

extern Elf32_Addr 
_rld_name_to_address(char *name);

extern int
_rld_interface(Elf32_Word operation);

extern void *
_rld_new_interface(Elf32_Word operation, ...);
#endif /* __LANGUAGE_C__ */
