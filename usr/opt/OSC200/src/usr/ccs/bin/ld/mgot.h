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
 * @(#)$RCSfile: mgot.h,v $ $Revision: 1.1.5.7 $ (DEC) $Date: 1993/10/13 16:10:10 $
 */
#ifndef _MGOT_H_
#define _MGOT_H_

/* These are routines visible from mgot.c */

extern int  mgot_max_got_size( void );            /* max # of bytes in a got */

extern int  mgot_max_got_count( void );               /* # of entries in a got (is size/8) */

extern void mgot_set_max_got_count( int );

extern void mgot_set_got_threshold( int );

extern int mgot_print(void);

extern long mgot_get_num_got_tables(void);

extern long mgot_num_used_mexts(void);

extern void mgot_check(pOBJ_FILER  , unsigned long );

extern void mgot_restore_mext_used_bits( void );

extern void mgot_note_local(pOBJ_FILER, pIMMED );

extern long mgot_local_literal_lookup(pOBJ_FILER  , int );

extern unsigned long mgot_get_num_locals( void );

extern unsigned long mgot_get_num_globals( pOBJ_FILER );

extern unsigned long mgot_got_offset( unsigned long );

extern void mgot_fillin_got_idxs( void );

extern long mgot_get_got_idx_for_pmext( pOBJ_FILER, pMEXTR );

extern unsigned long mgot_get_total_num_got_entries( void );

extern unsigned long mgot_get_num_merged( void );

extern unsigned long mgot_get_num_dynsyms( void );

extern void mgot_collect_got(void);

extern void mgot_write_gots_and_refd_globals(void);

extern void mgot_layout_dynsym_tbl( void );

extern void make_dynsym( pMEXTR, int, int );     /* actually in dynutil.c */

extern long mgot_get_gpvalue_for_obj( pOBJ_FILER );

extern unsigned long mgot_get_cur_got( void );

extern void mgot_set_cur_got( unsigned long );

extern void mgot_alloc_p1_local_got( pOBJ_FILER, LOCAL_GOT *, unsigned long);

extern void mgot_fix_local_gots( void );

extern int mgot_pimmed_offset(void);

extern void mgot_set_pimmed_offset(int);

extern void mgot_alloc( pOBJ_FILER, unsigned long );

extern void mgot_pmext_in_use( MEXTR *);

extern char * mgot_curobj_name( pOBJ_FILER );



extern  Elf32_Dyn   dynamic;           /* dynamic table entry */
extern  int         ndyn;             /* dynamic count of written .dynamic entries */
extern  long        tot_num_used_mexts;     /* # of mexts in use! */

#define WRITE_DYNAMIC_VAL(TAG, VAL) { \
    dynamic.d_tag = TAG; dynamic.d_un.d_val = (VAL); \
    write_outbuf(dso_outbuf, (char *) &dynamic, sizeof(Elf32_Dyn)); ndyn++; \
    }

#define WRITE_DYNAMIC_PTR(TAG, PTR) { \
    dynamic.d_tag = TAG; dynamic.d_un.d_ptr = (PTR); \
    write_outbuf(dso_outbuf, (char *) &dynamic, sizeof(Elf32_Dyn)); ndyn++; \
    }




/* Common routine definitions which were scattered in .c files: */
/* Put them here cause I was touching them and wanted to be sure we DO use
 *   define and use the correct prototypes...
 */

extern void alloc_got(pOBJ_FILER, IMMED **);
extern void build_dyn_objlist( pIMMED );



#if defined(__alpha) || defined(__mips64)
#define FUNNY_IMMED 0xa5a55a5aa5a55a5a  /* 2 32-bit longwords should be identical */
#else
#define FUNNY_IMMED 0xa5a55a5a    /* this never fit in a 16 bit quantity */
#endif

#endif  /* _MGOT_H_ */


