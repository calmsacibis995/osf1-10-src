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
 *	@(#)$RCSfile: mda.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/14 18:19:18 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#ifndef _MDA_H_
#define _MDA_H_

#define MDA_MAJOR_VERSION 1
#define MDA_MINOR_VERSION 0

#define SUCCESS 1
#define FAILED  0
#define NO_SUCH_SYMBOL -1

#define	MAXHISTORY	100
#define	MAXHISTLEN	128

int		file_mapped;		/* True if a file is mapped */
unsigned long	dump_file_address;	/* Memory adr of dump data  */
int		current_cpu;	/* cpu-related commands operate on this */
int		master_cpu;	/* default current cpu if not specified	*/
int		panic_cpu;	/* the one that called panic !!		*/
int		ptb0;

#define MAP(x) (*(int *)((vm_offset_t)(x) + dump_file_address))
#define MAPPED(x) ((int)((vm_offset_t)(x) + dump_file_address))

#define PHYS(p,v) { if (phys(v, &p, ptb0) != SUCCESS) return(FAILED);}

#define IMAGE(x) (((unsigned int)(x) + (unsigned int)kernel_address))

#endif
