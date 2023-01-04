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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */


typedef struct {
#if defined(__mips64) || defined(__alpha)
    unsigned long cm_tag;
#else
    unsigned cm_tag;
#endif
    union {
	unsigned long cm_val;
	unsigned long cm_ptr;
    } cm_un;
} CM;

#if defined(__mips64) || defined(__alpha)
#define CM_NULL            0L
#define CM_RELOC_NO        1L
#define CM_RELOC_PTR       2L
#else
#define CM_NULL            0x00000000
#define CM_RELOC_NO        0x00000001
#define CM_RELOC_PTR       0x00000002  
#endif

struct full_rlc {
    int type;
    unsigned long konst;
    unsigned long vaddr;
    unsigned long dist2lo;
};

#if defined(__mips64) || defined(__alpha)
#define CM_R_TYPE_NULL      0
#define CM_R_TYPE_ABS       1
#define CM_R_TYPE_GPREL32   2
#define CM_R_TYPE_QUAD      3
#define CM_R_TYPE_GPHI_LO   4  
#define CM_R_TYPE_WORD      5  /* 32-bit */
/* Don't use type 6 until pixie is fixed */
#define CM_R_TYPE_INITHI_LO 7
#define CM_R_TYPE_SREL16    8
#define CM_R_TYPE_SREL32    9
#define CM_R_TYPE_SREL64   10
#define CM_R_TYPE_PUSHPSUB 11
#define CM_R_TYPE_SHIFTSTORE 12
#define CM_R_TYPE_GPVALUE  13
#define CM_R_TYPE_EXTENDED_TYPE 15
#else
#define CM_R_TYPE_NULL      0
#define CM_R_TYPE_ABS       1
#define CM_R_TYPE_REL32     2
#define CM_R_TYPE_WORD      3
#define CM_R_TYPE_GPHI_LO   4
#define CM_R_TYPE_JMPADDR   5
#define CM_R_TYPE_GPHI_LO2  6
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
#if defined(__mips64) || defined(__alpha)
#define        ADDEND_NOCONST  0
#define        ADDEND_CONST    1   /* if an addend included, e.g. sym + k */
#define        ADDEND_BASE     2   /* base of address to be relocated */
#define        DEL_VADDR_MASK   0xFFFFF00000000000

struct COMPACT_RELOC {
    unsigned long addend:  4;
    unsigned long type:    4;      /* relocation type */
    unsigned long del_lo:  8;      /* delta to ref_lo from ref_hi, shifted 2 */
    signed   long del_vaddr: 48;   /* delta addr, from previous entry */
};

#else /* defined(__mips64) || defined(__alpha) */

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

#endif /* defined(__mips64) || defined(__alpha) */

struct COMPACT_RELOC_C {
    struct COMPACT_RELOC _rlc;
    unsigned long addend_const;         /* k of addend */
};

struct COMPACT_RELOC_C_BASE {
    struct COMPACT_RELOC _rlc;
    unsigned long addend_const;         /* k of addend */
    unsigned long base;                 /* base for next delta */
};

struct COMPACT_RELOC_BASE {
    struct COMPACT_RELOC _rlc;
    unsigned long base;                 /* base for next delta */
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
