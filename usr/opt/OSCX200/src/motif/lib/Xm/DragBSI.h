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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: DragBSI.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:18:20 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#ifndef _XmDragBSI_h
#define _XmDragBSI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  atoms and targets table structures
 */

typedef struct {
  Atom		atom;
  Time		time;
} xmAtomsTableEntryRec, *xmAtomsTableEntry;

typedef struct {
  Cardinal	numEntries;
  xmAtomsTableEntry entries;
} xmAtomsTableRec, *xmAtomsTable;

typedef struct {
    Cardinal	numTargets;
    Atom	*targets;
}xmTargetsTableEntryRec, *xmTargetsTableEntry;

typedef struct {
    Cardinal	numEntries;
    xmTargetsTableEntry entries;
}xmTargetsTableRec, *xmTargetsTable;

/*
 *  The following are structures for property access.
 *  They must have 64-bit multiple lengths to support 64-bit architectures.
 */

typedef struct {
    CARD32	atom B32;
    CARD16	name_length B16;
    CARD16	pad B16;
}xmMotifAtomPairRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_atom_pairs B16;
    CARD32	heap_offset B32;
    /* xmMotifAtomPairRec 	 atomPairs[];	*/
}xmMotifAtomPairPropertyRec;

typedef struct {
    CARD32	atom B32;
    CARD32	time B32;
}xmMotifAtomsTableRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_atoms B16;
    CARD32	heap_offset B32;
    /* xmMotifAtomsTableRec atoms[]; 	*/
}xmMotifAtomsPropertyRec;

typedef struct {
    BYTE	byte_order;
    BYTE	protocol_version;
    CARD16	num_target_lists B16;
    CARD32	heap_offset B32;
}xmMotifTargetsPropertyRec;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDragBSI_h */
