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
 *	@(#)$RCSfile: ldr_global_file.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:39:37 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* ldr_global_file.h
 * Definitions for loader global data file
 *
 * OSF/1 Release 1.0
 */


#ifndef _H_LDR_GLOBAL_FILE
#define _H_LDR_GLOBAL_FILE


/* External representation of a loader private file header */

typedef void *ldr_private_file_hdr;

/* External representation of a loader global file header */

typedef	void *ldr_global_file_hdr;

/* Initialize the specified file as a loader global data file.
 * This involves creating and initializing a global data file
 * header at the beginning of the file and filling it in, and
 * creating a heap from which the data structures that are to
 * be stored in the file can be allocated.  Note that the
 * file will always be mapped shared, read/write, not keep-on-exec.
 * Fd is an open file descriptor on the file to be initialized, open
 * for writing.  Returns pointer to the global file header in
 * *hdrp.  Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_global_file_init __((ldr_file_t fd, ldr_global_file_hdr *hdrp));

/* Compute and set the map size of the specified global file header.
 * The map size includes the global file header and the entire heap
 * including its header.  It is computed assuming that the file is
 * mapped into contiguous virtual address space.  Returns LDR_SUCCESS
 * on success, LDR_ERANGE on error.
 */

extern int
ldr_global_file_set_size __((ldr_global_file_hdr glob_hdr));

/* Copy any preloaded libraries from the specified loader context
 * into the specified loader global data file.  The data file
 * must have been initialized ldr_global_file_init above.
 * Also, the context should have been created with the correct
 * preload region-allocation procedures.
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_global_file_preload __((ldr_global_file_hdr glob_hdr, ldr_context *ctxt));

/* Inherit the specified open file as a loader global file.  This
 * basically involves error-checking.  We inherit the contents of
 * the file (the heap and KPT header).  Returns pointer to the
 * inherited global file header.  Returns LDR_SUCCESS on success
 * or negative error status on error.
 */

extern int
ldr_global_file_inherit __((ldr_file_t fd, ldr_global_file_hdr *hdrp));

/* Return the global KPT header from the specified loader global file.
 * Can't fail.
 */

extern ldr_kpt_header *
ldr_global_file_kpt __((ldr_global_file_hdr glob_hdr));

/* Remove the loader global file from the address space.  All uses of
 * the loader global file should already have been removed.  This is
 * intended to be used only from the global library installation
 * program prior to initializing a new loader global file.
 * Returns LDR_SUCCESS on success or negative error status on error.
 */

extern int
ldr_global_file_remove __((void));

/* Initialize a loader private file for this process.
 * This involves creating an anonymous memory region to hold
 * the private file data, initializing the heap contained in the
 * private file for future allocations, and initializing the
 * constituent data structures (currently a KPT header).
 * Note that the loader private file is always
 * located at a fixed address in the process' address space,
 * found through the address configuration record.
 *
 * This routine will only be called in a process which has not
 * inherited a private file from its parent (because of the address
 * conflict that would otherwise result).
 *
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_private_file_init __((ldr_private_file_hdr *priv_hdr));

/* Check to see whether a private file has been inherited from our parent
 * process.  If so, inherit it.  This basically involves error-checking.
 * Returns LDR_SUCCESS on success, negative error status on error.
 */

extern int
ldr_private_file_inherit __((ldr_private_file_hdr *priv_hdr));

/* Return the private KPT header from the specified loader private file.
 * Return LDR_SUCCESS on success, negative error status on error.
 */

extern ldr_kpt_header *
ldr_private_file_kpt __((ldr_private_file_hdr priv_hdr));

#endif /* _H_LDR_GLOBAL_FILE */
