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
 *	@(#)$RCSfile: ldr_package.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:36:56 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_package.h
 * Definitions for loader package tables
 * 
 * This file contains the definitions for the import and export
 * package table records shared between the format-dependent and
 * format-independent managers.  The import and export package
 * tables use the same package table record format.  The package
 * table record just lists the package table name type (package
 * name or module name) and name.
 *
 * OSF/1 Release 1.0
 */

#ifndef	_H_LDR_PACKAGE
#define	_H_LDR_PACKAGE


#define	LDR_PACKAGE_VERSION	1	/* current structure version */

/* The package kind field is the "hook" needed by format-dependent managers
 * (such as ELF) that need to have full control over the symbol resolution
 * policy.  A package record of kind "ldr_package_module" is interpreted
 * by the symbol resolution routines as containing the pathname of the
 * module exporting the symbol, rather than a package name.  This allows
 * the format-dependent manager to specify exactly which module the symbol
 * is to be resolved from, bypassing the whole package mechansim, if required.
 */

typedef enum ldr_package_kind {		/* kind of package record */
	ldr_package,			/* real package record */
	ldr_package_module		/* "fake" package record giving module name */
	} ldr_package_kind;

typedef struct ldr_package_rec {
	int		lp_version;	/* version number of structure */
	ldr_package_kind lp_kind;	/* kind of package record */
	char		*lp_name;	/* package name */
	char		*lp_reserved[20]; /* reserved space */
} ldr_package_rec;


/* Create a list of package records large enough to hold
 * count records, initialize it, and return it in *pkgs.
 * Version is the structure version number.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int ldr_packages_create __((int count, int version,
				   ldr_package_rec **retval));

/* Free a list of package records containing count records.  Returns
 * LDR_SUCCESS on success, negative error status on error.
 */

extern int ldr_packages_free __((int count, ldr_package_rec *val));

#endif	/* _H_LDR_PACKAGE */
