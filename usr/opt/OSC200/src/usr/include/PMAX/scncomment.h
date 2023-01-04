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
 *       @(#)$RCSfile: scncomment.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/01/28 20:01:03 $
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
/************************************************************************
 *									*
 *			Copyright (c) 1986,1988 by			*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/* $Copyright: |
 * |-----------------------------------------------------------|
 * | Copyright (c) 1990       MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restrictive Rights Legend                        |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         928 Arques Avenue                                 |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 *  $ */
#ifndef _SCNCOMMENT_H_
#define _SCNCOMMENT_H_

typedef struct {
    unsigned cm_tag;
    union {
	unsigned long cm_val;
	unsigned long cm_ptr;
    } cm_un;
} CM;

#define CM_NULL            0x00000000
#define CM_RELOC_NO        0x00000001
#define CM_RELOC_PTR       0x00000002  

struct full_rlc {
    int type;
    unsigned long konst;
    unsigned long vaddr;
    unsigned long dist2lo;
};

#define CM_R_TYPE_NULL      0
#define CM_R_TYPE_ABS       1
#define CM_R_TYPE_REL32     2
#define CM_R_TYPE_WORD      3
#define CM_R_TYPE_GPHI_LO   4
#define CM_R_TYPE_JMPADDR   5
#define CM_R_TYPE_GPHI_LO2  6

/* we don't consider 64 bits rightnow, when we do, we have to extend the */
/* type field                                                            */
#if 0
#define CM_R_TYPE_HI_LO64   7
#define CM_R_TYPE_WORD64    8
#define CM_R_TYPE_REL64     9
#endif


typedef struct {
    char *scn_praw;
    char *rlc_ptr;
    char *cur_rlc_ptr;
    int  rlc_no;
    int  cur_rlc_no;
    unsigned long last_base;
    struct full_rlc rlc_entry;
} cm_struct ;


/* this struct must be the same as next, except for the last item, */
/* which is the constant for the addend */
struct COMPACT_RELOC {
    unsigned addend:  2;
#define        ADDEND_NOCONST  0
#define        ADDEND_CONST    1   /* if an addend included, e.g. sym + k */
#define        ADDEND_BASE     2   /* base of address to be relocated */
    unsigned type:    3;           /* relocation type */
    unsigned del_lo:  8;           /* delta to ref_lo from ref_hi, shifted 2 */
    
    signed del_vaddr: 19;          /* delta addr, from previous entry */
                                   /* to be relocated */
#define        DEL_VADDR_MASK   0xFFFF8000
};


struct COMPACT_RELOC_C {
    struct COMPACT_RELOC _rlc;
    unsigned addend_const;         /* k of addend */
};

struct COMPACT_RELOC_C_BASE {
    struct COMPACT_RELOC _rlc;
    unsigned addend_const;         /* k of addend */
    unsigned base;                 /* base for next delta */
};

struct COMPACT_RELOC_BASE {
    struct COMPACT_RELOC _rlc;
    unsigned base;                 /* base for next delta */
};


union cm_rlc {
    struct COMPACT_RELOC            r;
    struct COMPACT_RELOC_C_BASE    cb;
    struct COMPACT_RELOC_C          c;
    struct COMPACT_RELOC_BASE       b;
};


#define VADDR_OVFL(delta)  (((delta & DEL_VADDR_MASK) != 0) && \
			    ((delta & DEL_VADDR_MASK) != DEL_VADDR_MASK))

#define VADDR_DELTA(delta) (delta & ~DEL_VADDR_MASK)

#endif /* _SCNCOMMENT_H_ */
