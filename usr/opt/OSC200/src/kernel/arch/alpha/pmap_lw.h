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
 * @(#)$RCSfile: pmap_lw.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/10/19 19:45:32 $
 */
#include <vm/vm_map.h>
#include <machine/pmap.h>

#define set_lww_bit(pte)        ((pte)->quadword |= 0x20000L)
#define clear_lww_bit(pte)        ((pte)->quadword &= ~0x20000L)
#define	LWW_MASK 0x20000L
#define VALID_MASK 0x1L
#define SEG_MASK 0x40000L
#define VM_FULL_WIRE 0x8000000000000000
#define LW_TRANS_EL 20

#define LW_TRANS_SIZE 4
struct vm_lw_trans {
     struct vm_lw_trans * next;
     vm_map_t             map;
     vm_offset_t          va;
     u_long               n_pages;
};
typedef struct vm_lw_trans * vm_lw_trans_t;
decl_simple_lock_data(extern,vm_lw_trans_queue_lock)

extern int pmap_lw_wire();
extern kern_return_t pmap_lw_wire_partial();
extern int pmap_lw_unwire();
extern int pmap_lw_unwire_aud();
extern void pmap_get_pfns();
extern void lw_init();
extern zone_t  vm_lw_trans_zone;
extern zone_t  vm_lw_buf_zone;
extern vm_lw_trans_t  lw_trans_queue;
extern vm_lw_trans_t  lw_free_queue;
extern long lw_waiters;
extern vm_map_t pmap_submap;

#define VM_LW_BUF_DEF    (2 * (ALPHA_PGBYTES/8))

struct vm_lw_buf {
      u_long     buf[VM_LW_BUF_DEF];
};

typedef struct vm_lw_buf  * vm_lw_buf_t;


extern void get_buffer();
extern void free_buffer();
