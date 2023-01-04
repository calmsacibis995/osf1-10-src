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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: cma_vmprot.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/09/11 22:38:05 $";
#endif

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	System-dependent services for changing memory protection: Ultrix
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	21 June 1990
 *
 *  MODIFICATION HISTORY:
 *	001	Paul Curtin	05 July 1990
 *		Added 1 to guard page size calculation.
 *	002	Paul Curtin	06 July 1990
 *		Added check on returned size in set_writable
 *	003	Dave Butenhof	03 April 1991
 *		Add support for OSF/1 (mprotect() returns 0 on success, not
 *		size of altered region).
 *	003A1	Webb Scales	21 January 1991
 *		Add typecasts to address arithmetic.
 *	006	Dave Butenhof	19 August 1992
 *		Add information to bugchecks to help diagnosis.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_vmprot.h>
#include <cma_int_errno.h>
#include <sys/mman.h>
#include <sys/types.h>

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Make a region of memory inaccessible (mark a range of pages as "no
 * 	access")
 *
 *	The mprotect system call changes the protection of portions
 *	of an application program's data memory.  Protection is per-
 *	formed on page cluster boundaries.  The default protection
 *	for data memory on process invocation is user READ/WRITE.
 *	The addr argument is the beginning address of the data
 *	block, and must fall on a page cluster boundary.
 *
 *	Chunk, stack, yellow zone, and guard page sizes must be rounded so
 *	that this granularity is acceptable (this is why the architecture
 *	defines the stack and guard sizes to be minimums).
 *
 *  FORMAL PARAMETERS:
 *
 *      low_address     Lowest address to modify
 *
 *      high_address    Highest address to modify
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__set_noaccess
#ifdef _CMA_PROTO_
	(
        cma_t_address   low_address,    /* Lowest address to change */
        cma_t_address   high_address)   /* Highest address to change */
#else	/* no prototypes */
	(low_address, high_address)
        cma_t_address   low_address;    /* Lowest address to change */
        cma_t_address   high_address;   /* Highest address to change */
#endif	/* prototype */
    {
    cma_t_integer	status;
    cma_t_integer	length;
    cma_t_integer	protection;

    length = (cma_t_integer)
	((unsigned long)high_address - (unsigned long)low_address + 1);
    protection = 0;

    status =  mprotect ( low_address, length, protection );

    if (status == -1)
	cma__bugcheck (
		"set_noaccess: %ld protecting (0x%lx,%ld bytes)",
		cma__get_errno (),
		low_address, length);

#if _CMA_OSIMPL_ != _CMA__OS_OSF
    if (status != length)
	cma__bugcheck (
		"set_noaccess: %ld protecting (0x%lx,%ld bytes)",
		status,
		low_address, length);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Make a region of memory accessible (mark a range of pages as "user
 * 	write")
 *
 *	The mprotect system call changes the protection of portions
 *	of an application program's data memory.  Protection is per-
 *	formed on page cluster boundaries.  The default protection
 *	for data memory on process invocation is user READ/WRITE.
 *	The addr argument is the beginning address of the data
 *	block, and must fall on a page cluster boundary.
 *
 *	Chunk, stack, yellow zone, and guard page sizes must be rounded so
 *	that this granularity is acceptable (this is why the architecture
 *	defines the stack and guard sizes to be minimums).
 *
 *  FORMAL PARAMETERS:
 *
 *	begin_address	Beginning address of area to modify
 *
 *	length		Length of area to be modified
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern void
cma__set_writable
#ifdef _CMA_PROTO_
	(
        cma_t_address   low_address,    /* Lowest address to change */
        cma_t_address   high_address)   /* Highest address to change */
#else	/* no prototypes */
	(low_address, high_address)
        cma_t_address   low_address;    /* Lowest address to change */
        cma_t_address   high_address;   /* Highest address to change */
#endif	/* prototype */
    {
    cma_t_integer	status;
    cma_t_integer	length;
    cma_t_integer	protection;


    length = (cma_t_integer)
	((unsigned long)high_address - (unsigned long)low_address + 1);
    protection = 0x7;

    status =  mprotect ( low_address, length, protection );

    if (status == -1)
	cma__bugcheck (
		"set_writeable: %ld unprotecting (0x%lx,%ld bytes)",
		cma__get_errno (),
		low_address, length);

#if _CMA_OSIMPL_ != _CMA__OS_OSF
    if (status != length)
	cma__bugcheck (
		"set_writeable: %ld protecting (0x%lx,%ld bytes)",
		status,
		low_address, length);
#endif
    }
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_VMPROT.C */
/*  *8    21-AUG-1992 13:41:02 BUTENHOF "Add info to bugchecks" */
/*   5A1  22-JAN-1992 13:25:11 SCALES "Fix address arithmetic" */
/*   7    29-OCT-1991 12:13:19 BUTENHOF "Add some casts" */
/*   6    14-OCT-1991 13:34:16 BUTENHOF "Fix compilation errors" */
/*  *5    10-JUN-1991 18:09:22 SCALES "Add sccs headers for Ultrix" */
/*  *4     2-MAY-1991 14:01:44 BUTENHOF "Adjust OSF conditionals for Tin BL3" */
/*  *3    12-APR-1991 23:33:58 BUTENHOF "Modify to support OSF/1" */
/*  *2    12-DEC-1990 20:33:42 BUTENHOF "Fix assem include, and clean up CMS history" */
/*  *1    12-DEC-1990 19:07:24 BUTENHOF "ULTRIX vmprot code" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_VMPROT.C */
