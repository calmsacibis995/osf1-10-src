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
static char *rcsid = "@(#)$RCSfile: relocate.c,v $ $Revision: 1.1.4.4 $ (DEC) $Date: 1993/10/14 20:16:14 $";
#endif

#ifdef _BSD
#if    defined(__alpha) || defined(__mips64)
#include "machine/inst.h"
#else
#include "mips/inst.h"
#endif
#endif

#ifdef _SYSV
#include "sys/inst.h"
#endif

#include <stdio.h>
#if defined(__alpha) || defined(__mips64)
#include <stdlib.h>
#include <string.h>
#include "unistd.h"
#endif
#include "sex.h"
#include "cmplrs/usys.h"
#include "cmplrs/ucode.h"

#include "ar.h"
#include "filehdr.h"
#include "aouthdr.h"
#include "sym.h"
#include "symconst.h"
#include "syms.h"   /* For GP_SIZE */
#include "scnhdr.h"
#include "reloc.h"
#include "cmplrs/stsupport.h"
#if defined(__osf__)
#include "excepthdr.h"
#else
#include "exception.h"
#endif
#include "excpt.h"

#undef exception_info

#include "ext_tbl.h"
#include "file_tbl.h"
#include "dn_tbl.h"
#include "ld.h"
#include "got.h"
#include "obj_file.h"
#include "layout.h"
#include "extmap.h"
#include "fdmap.h"
#include "dnmap.h"
#include "write.h"
#include "read.h"
#include "pass2.h"
#include "doup.h"
#include "seg.h"
#include "gp_tbl.h"
#include "list.h"
#include "pic_const.h"
#if defined(__alpha) || defined(__mips64)
#include "literal.h"
#include "scncomment.h"
#endif
#include "assert.h"


#include "mgot.h"           /* Get prototypes for multiple got routines */

#ifdef __osf__
/*
 * dependency file generation flag
 */
extern int dependflag;
#endif /* __osf__ */

/*
 * dynamic linking variables
 */

extern int prologsz();

extern long ELFflag;
extern HDRR hdrr;
extern unsigned long linking_main;
extern unsigned long init_size;
extern unsigned long fini_size;
extern char *pinitraw;
extern char *pfiniraw;
#if defined(__alpha) || defined(__mips64)
extern unsigned long got_base;
extern void *obj_cm_rlc_ptr();
extern unsigned long obj_cm_size();
extern unsigned long cm_rel_size_1;
#endif
extern SCNHDR cm_scnhdr;
extern unsigned compact_rlc;

extern long availgpsize;
extern int  autognum;
extern int  l8_count;
extern int  l4_count;
extern void assign_gp(void);
extern void sortlsym(void);
extern void sortlsym_mdn(void);
extern void ucode_count_ref(char *praw, long size);
extern int  sym_in_gp(pDNR mdn, int blkno);
extern void hide_externs(void);
extern void add_relscn(RELOC *, unsigned long, int, unsigned long);


extern unsigned long    need_excinfo;   /* flag set if someone references
                     * the size of pdata or is a .so
                     */

/* target_swapped is set if the targetsex is not the same as the hostsex */
extern long target_swapped;




#if defined(__alpha) || defined(__mips64)
extern void alpha_pick_nop(union alpha_instruction *, unsigned long);


/*
 *  Bit-vector used to tag .text targets as destinations for relocations.
 *  This is used to determine if there is a label at a particular spot.
 *  The macros and code should allow this to be null, and then not make
 *  use of any target information (assume everything is a target).
 */

static unsigned char *target_bits;


/*
 *  Masks for .text addresses target information.
 */


#define M_LABEL     1   /* Address has a label                      */
#define M_JSR_OR_BSR    2   /* Address has a jsr/bsr relocation (i.e. jsr/bsr instruction)  */


/*
 *  MAX_JSR_LDGP_DISTANCE is the maximum distance in bytes between a
 *  jsr/bsr and the corresponding ldgp.  If the ldgp after a jsr is
 *  not within that distance, it will not be a candidate for removal.
 *  This is to prevent relocations in "random" order to not hang ld seeing
 *  if there are any intermediate jsr/bsr's.
 */

#define MAX_JSR_LDGP_DISTANCE   (16*sizeof(union alpha_instruction))


/*
 *  Macro for determining if vaddr is a target.  It is presumed that vaddr
 *  is known to be within .text and not within a shared object.  Return destinations
 *  don't count.
 */

#define HAS_LABEL(vaddr)                        \
    (!target_bits ||                        \
    ((target_bits[((vaddr) - t_base)/sizeof(union alpha_instruction)]) & M_LABEL))


/*
 *  Macro for determining if vaddr is a non-ret control flow.
 */
 
#define HAS_CONTROL_FLOW(vaddr)                     \
    (!target_bits ||                        \
    ((target_bits[((vaddr) - t_base)/sizeof(union alpha_instruction)]) & M_JSR_OR_BSR))

/*
 *  Macro for adding vaddr as a target.  It is not presumed that vaddr
 *  is known to be within .text, but vaddr should not point into a shared object.
 */

#define ADD_LABEL(vaddr)                                        \
    { const unsigned long _addr = vaddr;                                \
      if (target_bits && _addr >= t_base && _addr < t_base+t_size)                      \
         target_bits[(_addr - t_base)/sizeof(union alpha_instruction)] |= M_LABEL; }

#define ADD_CONTROL_FLOW(vaddr)                                     \
    { const unsigned long _addr = vaddr;                                \
      if (target_bits && _addr >= t_base && _addr < t_base+t_size)                      \
         target_bits[(_addr - t_base)/sizeof(union alpha_instruction)] |= M_JSR_OR_BSR; }


/*
 *  Allocates the target bit information.  (Allowed to return NULL)
 */

#define ALLOC_TARGET_BITS(t_size) calloc(((t_size)/sizeof(union alpha_instruction)+1)/sizeof(*target_bits)*8, sizeof(*target_bits))


static void opt_pass(char *, RELOC *, long, long, long, long, char *, long, RELOC **, int);


#endif
void relocate(char *, RELOC *, long, long, long, long, char *, long, int);
void mk_local(RELOC *, pMEXTR);
void output_gp_info(register unsigned long addr, unsigned long gp_value);

/*
 * relocate() does the relocation on the "raw data" for the section
 */
void
relocate(p_raw, p_rlc, s_size, nreloc, s_vaddr,f_vaddr,s_name,s_flags,s_type)
char *p_raw;    /* pointer to the section's "raw data" to be relocated */
RELOC *p_rlc;   /* pointer to the sections relocation entries */
long s_size;    /* size of the section's "raw data" to be relocated */
long nreloc;    /* number of section's relocation entries */
long s_vaddr,   /* original starting address of the section */
     f_vaddr;   /* starting address of the section in the final object */
char *s_name;   /* name of section relocating (for error messages) */
long s_flags;   /* section flags */
int  s_type;
{
    register RELOC *preloc, *preloc_lo;
    register signed long immed;             /* Explicitly signed! */
    register long j, word, value, offset, word_lo, offset_lo,
    lit_offset, target_addr, my_new_addr;
    register unsigned long mask;
    register int i;
    register short half;
    register pMEXTR pmext;
    int     gpvalue_i= 0;                      /* value of i at last R_GPVALUE record */

#define STACK_SIZE 20
    unsigned long   stack[STACK_SIZE];      /* relocation op stack */
    int         tos=-1;                     /* top of relocation stack */

    long        src_gp_value = cur_obj->aouthdr->gp_value;

#if defined(__alpha) || defined(__mips64)
    /*  Change the scope of "gp_value" for Alpha    */
    long    gp_value = cur_obj->new_gp_value;

    /*  Buffer combined relocations for compacts */
    unsigned long   combined_value = 0;

    /*  Set the optimization level.
     *  (No optimization possible if multiple gp's in the input object
     *  since the actual GP values aren't known.
     */

    int optlevel = (cur_obj->la_scnhdr && cur_obj->la_scnhdr->s_size <= mgot_max_got_size() ? Olevel : 0);

    /* Instruction pointed to by R_LITERAL  (NULL means not in effect) */
    union alpha_instruction *r_literal_instr = NULL;
    long r_literal_addr = 0;                /* Address loaded from R_LITERAL instruction */
    RELOC *r_literal_reloc;
    int  r_lita_offset;

    union alpha_instruction *r_lituse_instr;
    int  r_lituse_cnt;                      /* Count or R_LITUSE after the R_LITERAL  */

    /* True if linking main image (note not 'linking_main') */

    int final_link = (sharable == CONST_CALL_SHARED ||
                      sharable == CONST_NON_SHARED && !relocatable);

    /* Setup mgot's notion of the current got: */
    mgot_set_cur_got( cur_obj->first_got_num );

#endif

    if (s_type == R_SN_TEXT)
        output_gp_info(f_vaddr, gp_value);


    /*
     * For each relocation entry do the specified relocation.
     */
    /*
     * For section which have overflowed s_nreloc the first entry contained
     * the true value.  So skip this entry since it is not a real relocation
     * entry and zero it out so it will not cause problems in case it the
     * relocation entries are saved.
     */
    if(s_flags & S_NRELOC_OVFL){
        j = 1;
        preloc = &(p_rlc[0]);
        preloc->r_vaddr = 0;
        preloc->r_symndx = 0;
        preloc->r_reserved = 0;
        preloc->r_type = R_ABS;
        preloc->r_extern = 0;
    }
    else
        j = 0;

    for(i = j ; i < nreloc ; i++){     
        /* This for loop goes almost to the end of relocate(); about 1940 lines.. */
        preloc = &(p_rlc[i]);               /* get the relocation entry */

            
        /*
         * This is required here in case previously used for the value
         * of s_nreloc is run into.
         */
        if ((sharable == CONST_NON_SHARED) && preloc->r_type == R_ABS)
            continue;

#if defined(__alpha) || defined(__mips64)
        /*
         * Determine the offset into the "raw data" that this entry
         * refers to and check for out-of-range errors.
         */
        if (preloc->r_type != R_OP_PUSH && preloc->r_type != R_OP_PSUB &&
            preloc->r_type != R_OP_PRSHIFT) 
#endif
            {
                offset = preloc->r_vaddr - s_vaddr;
                if (sharable == CONST_NON_SHARED)
                    if(offset < 0 || offset > s_size)
                        error(ER_OBJ, "bad relocation entry (r_vaddr)\n");
#if !defined(__alpha) && !defined(__mips64)
                if(preloc->r_type == R_REFHALF){
                    if(offset & 0x1)
                        error(ER_OBJ, "bad relocation entry (r_vaddr unaligned)\n");
                }
#endif
                else
                    if(offset & 0x3)
                        error(ER_OBJ, "bad relocation entry (r_vaddr unaligned)\n");
            }                               /* if */

        /*
         * Determine the value for the relocation.
         */
        if(preloc->r_type != R_GPDISP &&
           preloc->r_type != R_GPVALUE &&
           preloc->r_extern){
            /*
             * If r_extern is set then the value for relocation is the
             * value of the external symbol which is at the absolute
             * r_symndex in the object file's original external table.
             * To get a pointer to this symbol the external map for this
             * object file is used which maps absolute external symbol
             * table indexes into pointers to their merge external symbol
             * table entry.
             */
            if(preloc->r_symndx > cur_obj->hdrr.iextMax)
                error(ER_OBJ, "bad relocation entry (r_symndx [index])\n");
            pmext = get_extmap(cur_obj->iextmap + preloc->r_symndx);
            /*
             * If the map entry for this external symbol is a Nil pointer
             * it means that symbol was not a relocatable symbol (not going
             * to be in the output external symbol table) and is an error.
             */
            if(pmext == mextNil)
                error(ER_OBJ, "relocation entry for non-relocatable symbol in %s section for entry %d\n", s_name, i);
            if (!need_excinfo && 
                (pmext->aext.asym.sc == scPData ||
                 pmext->aext.asym.sc == scXData))
                error(ER_OBJ,
                      "exception relocation entry (0x%x) not expected\n", preloc->r_vaddr);
#if !(defined(__osf__) && defined(__mips)) && !(defined(__alpha) || defined(__mips64))
            /* (Mips Ultrix Only) */
            if (relocatable) {
                value = 0;
                preloc->r_symndx = mextx(pmext);
            }
            else
#endif                                      /* __osf__ */
                value = pmext->aext.asym.value;
        

            /*
             * If the symbol this entry refers to is undefined or common
             * then update the entry's r_symndx and r_vaddr so it can be
             * relocated on another load (having an undefined or common
             * symbol implies the final object will be relocatable).
             */
            if(pmext->aext.asym.sc == scUndefined ||
               pmext->aext.asym.sc == scSUndefined ||
               pmext->aext.asym.sc == scCommon ||         
               pmext->aext.asym.sc == scSCommon) {
                if (sharable == CONST_NON_SHARED) {
                    preloc->r_symndx = mextx(pmext);
                    switch (preloc->r_type) {
                      case R_OP_PUSH:
                      case R_OP_PSUB:
                      case R_OP_PRSHIFT:
                      case R_OP_STORE:
                        break;              /* must do all of these or none */
                      default:
                        preloc->r_vaddr += f_vaddr - s_vaddr;
                        continue;
                
                    }                       /* switch */
                }                           /* if (sharable == CONST_NON_SHARED) */
                else {
                    /* Ok now if we have an unresolvable symbol in the unref global
                     * area we want to use a value of 0, not its stub addr; this is an
                     * optmiziation for the loader; So we know the sym is a ref'd glob if
                     * its in a got;...
                     */
                    switch (s_type) {
                      case R_SN_RDATA:
                      case R_SN_DATA:
                      case R_SN_SDATA:
                        if (pmext->resolved != NOT_RESOLVED) {
                            value = pmext->dsogotvalue; /* sym is resolved, use its value */
                        } else if (pmext->isin_num_got_tables == 0) {
                            value = 0;
                        } else {
                            value = pmext->dsogotvalue; /* sym isnt what i want to special case.. */
                        }
                        break;
                      default:
                        value = pmext->dsogotvalue;
                    }
#if defined(__alpha) || defined(__mips64)
                    switch (preloc->r_type) {
                      case R_SREL16:
                      case R_SREL32:
                      case R_SREL64:
                      case R_OP_PUSH:
                      case R_OP_PSUB:
                      case R_OP_PRSHIFT:
                      case R_OP_STORE:
                        error(ER_OBJ,
                              "relocation illegaly points to undefined symbol\n", preloc->r_vaddr);
                
                    }                       /* switch(preloc->r_type) */
#endif
                }
            }
#if defined(__alpha) || defined(__mips64)
            else if (relocatable) {
                /* Relocatable objects need symndx updated to match
                 * the merged symbol table.  The code above only handles
                 * undefined externals.
                 */
                preloc->r_symndx = mextx(pmext);
            }
#endif

        }
#if defined(__alpha) || defined(__mips64)
        else if (preloc->r_type == R_GPDISP)
            value = gp_value - src_gp_value +
                s_vaddr - f_vaddr;
        else if (preloc->r_type == R_GPVALUE)
            value = preloc->r_symndx;
#endif
        else{
            /*
             * If r_extern is not set then the value for the relocation is
             * the final object's base address minus the original object
             * file's base address.  These base addresses are for the
             * section refered to by the section number in the r_symndx
             * field.
             */
            switch(preloc->r_symndx){
              case R_SN_TEXT:
                value = t_base + cur_obj->t_start -
                    cur_obj->t_scnhdr->s_vaddr;
                break;
              case R_SN_INIT:
                value = i_base + cur_obj->i_start -
                    cur_obj->i_scnhdr->s_vaddr;
                break;
              case R_SN_FINI:
                value = f_base + cur_obj->f_start -
                    cur_obj->f_scnhdr->s_vaddr;
                break;
              case R_SN_RDATA:
                value = rd_base + cur_obj->rd_start -
                    cur_obj->rd_scnhdr->s_vaddr;
                break;
              case R_SN_NULL:
              case R_SN_ABS:
                value = 0;
                break;
              case R_SN_XDATA:
                if (!need_excinfo) {
                    error(ER_OBJ,
                          "exception relocation entry (0x%x) not expected\n", 
                          preloc->r_vaddr);
                }                           /* if */
                value = seg_base[LD_XDATA] +
                    cur_obj->scninfo[LD_XDATA].start -
                        cur_obj->scninfo[LD_XDATA].scnhdr->s_vaddr;
                break;
              case R_SN_PDATA:
                if (!need_excinfo) {
                    error(ER_OBJ,
                          "exception relocation entry (0x%x) not expected\n", 
                          preloc->r_vaddr);
                }                           /* if */
                value = seg_base[LD_PDATA] +
                    cur_obj->scninfo[LD_PDATA].start -
                        cur_obj->scninfo[LD_PDATA].scnhdr->s_vaddr;
                break;
              case R_SN_DATA:
                value = d_base + cur_obj->d_start -
                    cur_obj->d_scnhdr->s_vaddr;
                break;
              case R_SN_SDATA:
                value = sd_base + cur_obj->sd_start -
                    cur_obj->sd_scnhdr->s_vaddr;
                break;
              case R_SN_SBSS:
                value = sb_base + cur_obj->sb_start -
                    cur_obj->sb_scnhdr->s_vaddr;
                break;
              case R_SN_BSS:
                value = b_base + cur_obj->b_start -
                    cur_obj->b_scnhdr->s_vaddr;
                break;
                /*
                 * Relocation of literal pool items are handled differently
                 * see the case of the R_LITERAL relocation type below.
                 */
#if defined(__alpha) || defined(__mips64)
              case R_SN_LIT8:
                value = l8_base + cur_obj->l8_start -
                    cur_obj->l8_scnhdr->s_vaddr;
                break;

              case R_SN_LIT4:
                value = l4_base + cur_obj->l4_start -
                    cur_obj->l4_scnhdr->s_vaddr;
                break;

              case R_SN_LITA:
                break;

#else                                       /* mips */
              case R_SN_LIT8:
              case R_SN_LIT4:
#endif                                      /* __alpha */
                break;
              default:
                if (sharable == CONST_NON_SHARED)
                    error(ER_OBJ,
                          "bad relocation entry (0x%x)\n", preloc->r_vaddr);
                else {
                    continue;               /* turned off due to pic */
                }
            }
        }
        DPRINTF_4("fixup offset %x, inst %x, type %d\n",offset,offset >= 0 && offset < s_size ? *(int *)(p_raw + offset) : 0xb00b00,preloc->r_type );


        /*
         * Now do the relocation at the offset in the "raw data" for the
         * r_type of relocation.  Note that all immediate values are
         * signed and must be sign extended before the value for the
         * relocation is added.
         */

        /*****************************************************************************
         *                                                                           *
         *                                                                           *
         * start of swtich (preloc->r_type), about 1620 lines                AAA *****
         *                                                                           *
         *                                                                           *
         *****************************************************************************/
        switch(preloc->r_type) {            /* About 1620 lines... AAA */

          case R_ABS:
#if defined(__alpha) || defined(__mips64)
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
            break;
#endif
            if (sharable == CONST_NON_SHARED) 
                error(ER_FATAL, "Unexpected relocation type in %s\n",
                      cur_obj->name);
            else {
                static int count;
#if 0
                error(ER_INFO_NOFILE,"Fixing R_ABS for %s=0x%x, count=%d\n",
                      pmext->name,value,count);
#endif
                word = *(long *)(p_raw + offset);
                if ((count&1) == 0) {
                    word_lo = (short)value;
                    immed = (short)((value - word_lo) >> 16);
#if defined(__alpha) || defined(__mips64)
                    word = (word & 0xffffffffffff0000) | (immed & 0x000000000000ffff);
#else
                    word = (word & 0xffff0000) | (immed & 0x0000ffff);
#endif
                }
                else if ((count&1) == 1) {
#if defined(__alpha) || defined(__mips64)
                    word = (word & 0xffffffffffff0000) | (value & 0x000000000000ffff);
#else
                    word = (word & 0xffff0000) | (value & 0x0000ffff);
#endif
                }
                else {
                    ASSERT(FALSE);
                }
                *(long *)(p_raw + offset) = word;
                count++;
            }
            break;

#if !defined(__alpha) && !defined(__mips64)
          case R_REFHALF:
            half = *(short *)(p_raw + offset);
            if(target_swapped)
                half = swap_half(half);
            word = half + value;
            /*
             * The value after relocation for a R_REFHALF must fit in to
             * a half word if not it is an error.
             */
            if(word < 0xffff8000 || word > 0x00007fff)
                error(ER_WARNING, "half word relocation out-of-range\n");
            half = word;
            if(target_swapped)
                half = swap_half(half);
            *(short *)(p_raw + offset) = half;
            break;
#endif                                      /* !__alpha */

#if defined(__alpha) || defined(__mips64)
          case R_REFLONG:
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
#else
          case R_REFWORD:
#endif
            {
                unsigned long origword;

                word = *(signed int *)(p_raw + offset); /* Both ALPHA && MIPS */

                if(target_swapped)
                    word = swap_word(word);
#if !defined(__alpha) && !defined(__mips64)
                if ((sharable > CONST_NON_SHARED) && !preloc->r_extern) {
                    pSECTION psec;
                    if (preloc->r_symndx == R_SN_LIT8) 
                        half = SEG_LIT8;
                    if (preloc->r_symndx == R_SN_LIT4) 
                        half = SEG_LIT4;
                    ASSERT(preloc->r_symndx != R_SN_LIT8 && preloc->r_symndx != R_SN_LIT4);
                    if (preloc->r_symndx == R_SN_TEXT) 
                        half = SEG_TEXT;
                    if (preloc->r_symndx == R_SN_INIT)
                        half = SEG_INIT;
                    if (preloc->r_symndx == R_SN_FINI)
                        half = SEG_FINI;
                    if (preloc->r_symndx == R_SN_DATA)
                        half = SEG_DATA;
                    if (preloc->r_symndx == R_SN_SDATA)
                        half = SEG_SDATA;
                    if (preloc->r_symndx == R_SN_RDATA)
                        half = SEG_RDATA;
                    if (preloc->r_symndx == R_SN_SBSS)
                        half = SEG_SBSS;
                    if (preloc->r_symndx == R_SN_BSS)
                        half = SEG_BSS;
                    if (preloc->r_symndx == R_SN_XDATA)
                        half = SEG_XDATA;
                    if (preloc->r_symndx == R_SN_PDATA)
                        half = SEG_PDATA;
                    if (preloc->r_symndx == R_SN_ABS)
                        error(ER_OBJ, "unexpected R_SN_ABS index in reloaction\n"); 
                    DPRINTF_2("from %x", word);
                    if (preloc->r_symndx == R_SN_TEXT || 
                        preloc->r_symndx == R_SN_INIT ||
                        preloc->r_symndx == R_SN_FINI) {
                        psec = (pSECTION)(cur_obj->section_list[half]->data);
                        word = map_address(word, psec, FALSE) + value - 
                            psec->cum_extra;
                    } 
                    else 
                        word += value;
                    origword = word;
                    DPRINTF_3(" to %x (%x)\n",word, word - gp_value);
                    if (!linking_main && (s_type == R_SN_RDATA)) {
                        if ((preloc->r_symndx == R_SN_TEXT || 
                             preloc->r_symndx == R_SN_INIT ||
                             preloc->r_symndx == R_SN_FINI))
                            word = word - gp_value;
                    }
                }                           /* if ((sharable > CONST_NON_SHARED... */
                else {
                    DPRINTF_3("from %x to %x\n",word, word+value);
                    word += value;
                    origword = word;
                }
#else                                       /* __alpha */
                DPRINTF_3("from %x to %x\n",word, word+value);
                word += value;
                origword = word;
#endif
#if defined(__alpha) || defined(__mips64)
                /*
                 * The value after relocation for a R_REFLONG must fit in to
                 * a longword if not it is an error.
                 */

                if ((signed long) word > 0x7fffffffL ||
                    (signed long) word < -0x80000000L)
                    error(ER_WARNING, "longword relocation out-of-range (offset at %lx)\n", preloc->r_vaddr);

                /*
                 *  Add as label if -non_shared, or -call_shared and local, or -call_shared and
                 *  from a current object.
                 */

                if (optlevel > 0)
                    if (sharable == CONST_NON_SHARED ||
                        sharable == CONST_CALL_SHARED && !preloc->r_extern ||
                        sharable == CONST_CALL_SHARED && pmext->resolved == WITH_OBJ)
                        ADD_LABEL(word);

                if (s_type == R_SN_LITA) {
                    long litar_idx = offset / sizeof(long);

                    /* Now we are looking thru the lita raw area; we need to
                     *  create an array to map from the lita to relocs so that
                     *  when we process a ref* we can get the correct reloc
                     *  entry  (remember it has padding...)
                     */
                    if (cur_obj->lita_to_reloc == NULL) { /* The array should have been allocated for us already... */
                        error( ER_FATAL,
                              "Someone didnt allocate lita_to_reloc\n");
                    }
                    cur_obj->lita_to_reloc[ litar_idx ] = i;
                }
#endif
                if(target_swapped)
                    word = swap_word(word);
                *(unsigned int *)(p_raw + offset) = word; /* Both Alpha & MIPS */

                if (sharable > CONST_NON_SHARED && s_type != R_SN_LITA)
                    add_relscn(preloc, f_vaddr - s_vaddr, s_type, origword);
                break;
            }

#if defined(__alpha) || defined(__mips64)
          case R_SREL16:
#if TODO
            cross bytesex stuff
#endif
                if (preloc->r_extern == 0) {
                    /* reconstruct what we want to be offset from and
                     *  relocate it.
                     */
                    word = *(short *)(p_raw + offset);
                    word += preloc->r_vaddr;
                    target_addr = word + value;
                } else {
                    /* new extern address is in value */
                    target_addr = value;
                }                           /* if */

            /* calculate what this locations new address will be */
            my_new_addr = preloc->r_vaddr - s_vaddr + f_vaddr;

            /* calc new offset so my_new_addr + contents == target_addr */
            word = target_addr - my_new_addr;
            if (word < -(1 << 16) || word >= (1 << 16)) {
                error(ER_OBJ, "SREL relocation overflow 0x%x\n", 
                      preloc->r_vaddr);
            }                               /* if */
            *(short *)(p_raw + offset) = word;
            /* Add compact relocation */
            if (sharable > CONST_NON_SHARED && compact_rlc) {
                obj_cm_rlc_make(preloc, word, my_new_addr);
            }
            break;


          case R_SREL32:
#if TODO
            cross bytesex stuff
#endif
                if (preloc->r_extern == 0) {
                    /* reconstruct what we want to be offset from and
                     *  relocate it.
                     */
                    word = *(int *)(p_raw + offset);
                    word += preloc->r_vaddr;
                    target_addr = word + value;
                } else {
                    /* new extern address is in value */
                    target_addr = value;
                }                           /* if */

            /* calculate what this locations new address will be */
            my_new_addr = preloc->r_vaddr - s_vaddr + f_vaddr;

            /* calc new offset so my_new_addr + contents == target_addr */
            word = target_addr - my_new_addr;
            if (word < (signed long)(0xffffffff80000000) || word > (signed long)(0x000000007fffffff)) {
                error(ER_OBJ, "SREL relocation overflow vaddr=0x%lx (0x%lx - 0x%lx = 0x%lx)\n", 
                      preloc->r_vaddr, target_addr, my_new_addr, word);
            }                               /* if */
            *(int *)(p_raw + offset) = word;
            /* Add compact relocation */
            if (sharable > CONST_NON_SHARED && compact_rlc) {
                obj_cm_rlc_make(preloc, word, my_new_addr);
            }
            break;

          case R_SREL64:
#if TODO
            cross bytesex stuff
#endif
                if (preloc->r_extern == 0) {
                    /* reconstruct what we want to be offset from and
                     *  relocate it.
                     */
                    word = *(long *)(p_raw + offset);
                    word += preloc->r_vaddr;
                    target_addr = word + value;
                } else {
                    /* new extern address is in value */
                    target_addr = value;
                }                           /* if */

            /* calculate what this locations new address will be */
            my_new_addr = preloc->r_vaddr - s_vaddr + f_vaddr;

            /* calc new offset so my_new_addr + contents == target_addr */
            word = target_addr - my_new_addr;
            *(long *)(p_raw + offset) = word;
            /* Add compact relocation */
            if (sharable > CONST_NON_SHARED && compact_rlc) {
                obj_cm_rlc_make(preloc, word, my_new_addr);
            }
            break;

          case R_OP_PUSH:
            {
                /* need to set preloc->r_vaddr so we can use it */
                if (preloc->r_extern) {
                    preloc->r_vaddr = value;
                } else {
                    /* local so it's a relative address */
                    preloc->r_vaddr += value;
                }                           /* if */
                if (tos + 1 >= STACK_SIZE) {
                    error(ER_OBJ, "R_OP_PUSH: stack overflow\n", 
                          preloc->r_vaddr);
                }                           /* if */
                stack[++tos] = preloc->r_vaddr;
                /* Add compact relocation */
                if (sharable > CONST_NON_SHARED) {
                    combined_value = preloc->r_vaddr;
                }
            }
            break;

          case R_OP_PRSHIFT:
          case R_OP_PSUB:
            {
                /* need to set preloc->r_vaddr so we can use it */
                if (preloc->r_extern) {
                    preloc->r_vaddr = value;
                } else {
                    /* local so it's a relative address */
                    preloc->r_vaddr += value;
                }                           /* if */
                if (tos < 0) {
                    error(ER_OBJ, "relocation stack op: stack empty\n", 
                          preloc->r_vaddr);
                }                           /* if */
                switch(preloc->r_type){
                  case R_OP_PRSHIFT:
                    /* sort of a push sright shift */
                    stack[tos] = stack[tos] >> (unsigned long)preloc->r_vaddr;
                    combined_value = preloc->r_vaddr;
                    break;
                  case R_OP_PSUB:
                    /* sort of a push subtract */
                    stack[tos] -= preloc->r_vaddr;
                    if (sharable > CONST_NON_SHARED && compact_rlc) {
                        obj_cm_rlc_make(preloc, preloc->r_vaddr, 
                                        combined_value);
                        combined_value = 0;
                    }
                    break;
                }                           /* switch */
            }
            break;

          case R_OP_STORE:
#ifdef TODO
            call shared
#endif
                /* this will take the value from the top of the
                 *  relocation stack and store selected bits
                 *  into a field in a word in memory
                 */
                if (tos < 0) {
                    error(ER_OBJ, "R_OP_STORE: stack empty\n", 
                          preloc->r_vaddr);
                }                           /* if */

            word = *(long *)(p_raw + offset);

            if(target_swapped)
                word = swap_word(word);

            /* remove target bits from destination word */
            mask = 0xffffffffffffffffUL;
            if (preloc->r_size != 64)
                mask &= ((1UL << (unsigned long)preloc->r_size) - 1UL);
            mask <<= (unsigned long)preloc->r_offset;
            word &= ~(mask);

            /* select target bits from source and position them */
            if (preloc->r_size != 64)
                value = stack[tos--] & ((1UL << (unsigned long)preloc->r_size) - 1UL);
            value <<= (unsigned long)preloc->r_offset;

            /* or source target bits into destination */
            word |= value;

            if(target_swapped)
                word = swap_word(word);

            *(long *)(p_raw + offset) = word;

            if (sharable > CONST_NON_SHARED && compact_rlc) {
                unsigned long offsize;
                offsize = combined_value;
                offsize <<= 16;
                offsize |= (preloc->r_offset & 0xFFFF);
                offsize <<= 16;
                offsize |= (preloc->r_size & 0xFFFF);
                obj_cm_rlc_make(preloc, 
                                offsize,
                                preloc->r_vaddr + f_vaddr - s_vaddr);
                combined_value = 0;
            }
            break;

          case R_REFQUAD:
            {
                unsigned long origword;

                r_literal_instr = NULL;     /* Any R_LITUSE not in effect */

                word = *(long *)(p_raw + offset);

                if(target_swapped)
                    word = swap_word(word);
                DPRINTF_3("from %lx to %lx\n",word, word+value);
                word += value;
                origword = word;
                if(target_swapped)
                    word = swap_word(word);

                /*
                 *  Add as label if -non_shared, or -call_shared and local, or -call_shared and
                 *  from a current object.
                 */

                if (optlevel > 0)
                    if (sharable == CONST_NON_SHARED ||
                        sharable == CONST_CALL_SHARED && !preloc->r_extern ||
                        sharable == CONST_CALL_SHARED && pmext->resolved == WITH_OBJ)
                        ADD_LABEL(word);

                *(long *)(p_raw + offset) = word;

#if defined(__alpha) || defined(__mips64)   /* jpm */
                if (s_type == R_SN_LITA) {
                    long litar_idx = offset / sizeof(long);

                    LD_ASSERT( offset < cur_obj->la_scnhdr->s_size); /* insure our ref is into the allocated area */

                    /* Now we are looking thru the lita raw area; we need to
                     *  create an array to map from the lita to relocs so that
                     *  when we process a ref* we can get the correct reloc
                     *  entry  (remember it has padding...)
                     */
                    if (cur_obj->lita_to_reloc != NULL) /* No array? must be opt level 0 or no ram... oh well */
                        cur_obj->lita_to_reloc[ litar_idx ] = i;
                }
#endif

                if (sharable > CONST_NON_SHARED && s_type != R_SN_LITA)
                    add_relscn(preloc, f_vaddr - s_vaddr, s_type, origword);
                break;
            }                               /* end case refquad: */
#endif                                      /* __alpha */


#ifdef _DLI_DLA
          case R_REFWORD_64:
            word = *(long *)(p_raw + offset);
            if(target_swapped)
                word = swap_word(word);
            word += Thi64_BASE;
            if(target_swapped)
                word = swap_word(word);
            *(long *)(p_raw + offset) = word;
            break;
#endif

#if !defined(__mips64) && !defined(__alpha)
          case R_REFHI_ADDEND:
            /* prototype not handle shared and -r link yet    */
            /* for -r linkage, we may need to create a hi/lo  */
            /* relocation pair in case the addend grew beyond */
            /* 16 bits                                        */
            assert(sharable < CONST_CALL_SHARED && !relocatable);
        
            word = *(long *)(p_raw + offset);
            if (target_swapped)
                word = swap_word(word);
            immed = (short)word;            /* low 16 bits are embedded in inst */
            if (preloc->r_extern == 0) {
                /* in case of local, relative start is always from beginning */
                /* of section. Unlike other relocations. Such way, I can get */
                /* better chance of immed falls within range */
                switch (preloc->r_symndx) {
                  case R_SN_TEXT:
                    value += cur_obj->t_scnhdr->s_vaddr;
                    break;
                  case R_SN_INIT:
                    value += cur_obj->i_scnhdr->s_vaddr;
                    break;
                  case R_SN_FINI:
                    value += cur_obj->f_scnhdr->s_vaddr;
                    break;
                  case R_SN_RDATA:
                    value += cur_obj->rd_scnhdr->s_vaddr;
                    break;
                  case R_SN_ABS:
                    /* no change needed */
                    break;
                  case R_SN_XDATA:
                    if (need_excinfo)
                        value += cur_obj->scninfo[LD_XDATA].scnhdr->s_vaddr;
                    break;
                  case R_SN_PDATA:
                    if (need_excinfo)
                        value += cur_obj->scninfo[LD_PDATA].scnhdr->s_vaddr;
                    break;
                  case R_SN_DATA:
                    value += cur_obj->d_scnhdr->s_vaddr;
                    break;
                  case R_SN_SDATA:
                    value += cur_obj->sd_scnhdr->s_vaddr;
                    break;
                  case R_SN_SBSS:
                    value += cur_obj->sb_scnhdr->s_vaddr;
                    break;
                  case R_SN_BSS:
                    value += cur_obj->b_scnhdr->s_vaddr;
                    break;
                  default:
                    assert(0);
                }                           /* switch preloc->r_symndx */
            }                               /* if preloc->r_extern */
            immed += value;
            word = (word & 0xffff0000) | 
                (((immed + 0x00008000) >> 16) & 0x0000ffff);
            if (target_swapped)
                word = swap_word(word);
            *(long *)(p_raw + offset) = word;
            break;
#endif


#if defined(__alpha) || defined(__mips64)
          case R_GPREL32:
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
#else
          case R_GPREL:
            /*
             * If there are R_GPREL entries and the final object is not to
             * be relocatable then there should be a reference to the gp
             * symbol so the value will get loaded by some instruction.
             */
            if((sharable == CONST_NON_SHARED) && 
               relocatable == FALSE && p_gp == mextNil)
                error(ER_WARNING,
                      "R_GPREL relocation entry with no global pointer reference\n");
#endif                                      /* __alpha */

            word = *(signed int *)(p_raw + offset); /* BOTH MIPS & ALPHA */
            if(target_swapped)
                word = swap_word(word);

#if defined(__alpha) || defined(__mips64)
            immed = word;
#else
            immed = (short)word;
#endif
            if(preloc->r_extern) {
                /*
                 * For R_GPREF relocation types that are external references
                 * the constant at the entry is just an offset from the
                 * symbol.
                 */
#if defined(__alpha) || defined(__mips64)
                /*
                 * On Alpha, must have a single gp value when generating -r objects
                 * if the GP_REL32 is external.  This is because the actual gp value is
                 * not known at the point the reference is made on the final link.
                 */

                if (relocatable && multiple_got)
                    error(ER_WARNING, "GP value for external GP_REL32 relocation will be lossed on final link, bad object file produced\n");
#endif
                immed += value - gp_value;
            }
            else
                /*
                 * For R_GPREF relocation types that are local references
                 * the constant at the entry is just an offset from the
                 * gp_value for that object file.
                 */
                immed += value + src_gp_value - gp_value;
            /*
             *  The value after relocation for a R_GPREL32 must fit into
             * a long word.  If not, it is an error.
             */

            if(immed > (signed long) 0x000000007fffffff || immed < (signed long) 0xffffffff80000000) {

                /* Value is out of range. Error processing: */

                dump_gpwarn();
                if(preloc->r_extern){
                    if(pmext->aext.asym.sc != scSData && 
                       pmext->aext.asym.sc != scSBss){
                        if (pmext->aext.asym.sc == scUndefined) {
                            error(ER_ERROR, "Error: symbol %s has a GPREL32 relocation and is unresolved.\n", pmext->name);
                        }
                        else {
                            error(ER_ERROR, "symbol %s has incompatible declarations in object files, use ld -v option for more info.\n", pmext->name);
                            error(ER_ERROR, "gp relocation out-of-range in %s section for relocation entry %d for symbol: %s\n", s_name, i, pmext->name);
                            error(ER_ERROR, "above gp relocation entry for non .sdata or .sbss symbol\n");
                        }
                    }
                    else if(vflag){
                        error(ER_ERROR, "gp relocation out-of-range in %s section for relocation entry %d for symbol: %s\n", s_name, i, pmext->name);
                    }
                    else if(!gp_range_errors){
                        error(ER_ERROR, "gp relocation out-of-range errors have occured and bad object file produced (corrective action must be taken)\n");
                    }
                }
                else{
                    if(preloc->r_symndx != R_SN_SDATA &&
                       preloc->r_symndx != R_SN_SBSS){
                        error(ER_ERROR, "gp relocation out-of-range in %s section for relocation entry %d\n", s_name, i);
                        error(ER_ERROR, "above gp relocation entry for non .sdata or .sbss section\n");
                    }
                    else if(vflag){
                        error(ER_ERROR, "gp relocation out-of-range in %s section for relocation entry %d\n", s_name, i);
                    }
                    else if(!gp_range_errors){
                        error(ER_ERROR, "gp relocation out-of-range errors have occured and bad object file produced (corrective action must be taken)\n");
                    }
                }
                gp_range_errors = TRUE;
            }
            word = immed;

            /*
             *  Add as label if -non_shared, or -call_shared and local, or -call_shared and
             *  from a current object.
             */

            if (optlevel > 0)               /* cur_obj's la_size has been included in optlevel */
                if (sharable == CONST_NON_SHARED ||
                    sharable == CONST_CALL_SHARED && !preloc->r_extern ||
                    sharable == CONST_CALL_SHARED && pmext->resolved == WITH_OBJ)
                    ADD_LABEL(immed+gp_value);


            /* Add compact relocation for R_GPREL32 relocations */
            if (sharable > CONST_NON_SHARED && compact_rlc) {
                obj_cm_rlc_make(preloc, (immed & 0xffffffff),
                                preloc->r_vaddr + f_vaddr - s_vaddr);
            }
            if(target_swapped)
                word = swap_word(word);
            *(unsigned int *)(p_raw + offset) = word; /* Both ALPHA and MIPS */

            if (relocatable && preloc->r_extern)
                mk_local(preloc, pmext);
            break;

        
          case R_LITERAL:
            {
#if !defined(__alpha) && !defined(__mips64)
                /*
                 * If there are R_LITERAL entries and the final object is not to
                 * be relocatable then there should be a reference to the gp
                 * symbol so the value will get loaded by some instruction.
                 */
                if((sharable == CONST_NON_SHARED) && 
                   relocatable == FALSE && p_gp == mextNil)
                    error(ER_WARNING, "R_LITERAL relocation entry with no global pointer reference\n");
#else
                r_literal_instr = NULL;     /* Any R_LITUSE not in effect */
                r_lituse_cnt = 0;
                r_literal_reloc = preloc;
#endif
                word = *(unsigned int *)(p_raw + offset); /* Both ALPHA and MIPS */

                if(target_swapped)
                    word = swap_word(word);
                immed = (signed short) word;
                if(preloc->r_extern)
                    error(ER_OBJ,
                          "bad LITERAL relocation entry (r_extern != 0)\n");

                /*
                 * Relocate the offset to the literal by determining the old
                 * offset into the orignal literal pool and then getting the
                 * new offset for it (mapped by what is now in p_raw_lit[A84])
                 * and then changing that back into a displacement off the gp.
                 */
                switch(preloc->r_symndx) {
#if defined(__alpha) || defined(__mips64)
                  case R_SN_LITA:
                    lit_offset = immed + src_gp_value -
                        cur_obj->la_scnhdr->s_vaddr;
                    if (lit_offset > cur_obj->la_scnhdr->s_size ||
                        lit_offset % sizeof(long) != 0)
                        error(ER_OBJ, "bad LITERAL address relocation entry (offset at %lx)\n",preloc->r_vaddr);
                    if (sharable > CONST_NON_SHARED)
                        immed = got_base + *(cur_obj->p_raw_lita + lit_offset / sizeof(long)) - gp_value;
                    else
                        immed = la_base +
                            (staticflag ? *(cur_obj->p_raw_lita + lit_offset / sizeof(long))
                             : cur_obj->la_start + lit_offset) - gp_value;

                    break;
#else                                       /* __mips */
                  case R_SN_LIT8:
                    lit_offset = immed + src_gp_value -
                        cur_obj->l8_scnhdr->s_vaddr;
                    if(lit_offset > cur_obj->l8_scnhdr->s_size ||
                       lit_offset % sizeof(long) != 0 )
                        error(ER_OBJ, "bad LITERAL relocation entry (offset at %lx)\n",preloc->r_vaddr);
                    immed = l8_base + *(cur_obj->p_raw_lit8 +
                                        lit_offset / sizeof(long)) - gp_value;
                    break;

                  case R_SN_LIT4:
                    lit_offset = immed + src_gp_value -
                        cur_obj->l4_scnhdr->s_vaddr;
                    if(lit_offset > cur_obj->l4_scnhdr->s_size ||
                       lit_offset % sizeof(int) != 0 )
                        error(ER_OBJ, "bad LITERAL relocation entry (offset at %x)\n",preloc->r_vaddr);

                    immed = l4_base + *(cur_obj->p_raw_lit4 +
                                        lit_offset / sizeof(int)) - gp_value;
                    break;
#endif                                      /* __alpha */
                  default:
                    error(ER_OBJ,
                          "bad LITERAL relocation entry (r_symndx)\n");
                }                           /* switch( preloc->r_symndx) */

                /*
                 * The value after relocation for a R_LITERAL must fit in to
                 * a half word if not it is an error.
                 */

                if (immed < -32768L || immed > 32767L) { /* Both ALPHA && MIPS */
                    dump_gpwarn();
                    if(vflag){
                        error(ER_ERROR, "gp relocation out-of-range in %s section for relocation entry %d\n", s_name, i);
                    }
                    else if(!gp_range_errors){
                        error(ER_ERROR, "gp relocation out-of-range errors have occured and bad object file produced (corrective action must be taken)\n");
                    }
                    gp_range_errors = TRUE;
                }
#if defined(__alpha) || defined(__mips64)
                word = (word & 0xffffffffffff0000) | (immed & 0x000000000000ffff);
#else
                word = (word & 0xffff0000) | (immed & 0x0000ffff);
#endif
                if(target_swapped)
                    word = swap_word(word);

                *(unsigned int *)(p_raw + offset) = word; /* Both ALPHA && MIPS */

#if defined(__alpha) || defined(__mips64)
                r_literal_instr = (union alpha_instruction *) (p_raw + offset);
                r_lita_offset = lit_offset;

                /*
                 *  Save lita relocation for opt_pass() .text relocations
                 *  (note that array might not have been allocated).
                 */

                if (cur_obj->pp_rlc_lita)
                    cur_obj->pp_rlc_lita[i] = &cur_obj->p_rlc_lita[ cur_obj->lita_to_reloc[ r_lita_offset /sizeof(long) ] ];

                if (optlevel < 1 || relocatable || staticflag) {
                    r_literal_instr = NULL; /* No opt desired */
                    break;
                }

                /*
                 *  THE REMAINDER R_LITERAL CODE IS FOR POTENTIAL OPTIMIZATION.
                 *
                 *  Need to determine the address loaded by the r_literal
                 *  instruction.
                 */

                switch (sharable) {

                  case CONST_NON_SHARED:
                    /*
                     *  -non_shared
                     *  This means the relocated address in the .lita entry
                     *  is desired (regardless of local/external setting).
                     */

                    r_literal_addr = *(cur_obj->p_raw_lita + r_lita_offset / sizeof(long));
                    break;


                  case CONST_MAKE_SHARABLE:
                    /*
                     *  -shared
                     *  This means that locals are in the local got,
                     *      and external addresses are used only if hidden, and
                     *  defined by an object file (not .so).
                     */

                    {
                        long     litar_idx;         /* Index into lita_to_reloc[]*/
                        long     lita_idx;          /* Index into p_rlc_lita[] */
                        RELOC    *r_lita_relocation;

                        /*  Look at .lita relocation to see where symbol came from.  */
                        litar_idx = r_lita_offset/sizeof(long);
                        lita_idx = cur_obj->lita_to_reloc[ litar_idx ];

                        if (lita_idx == -1) {
                            error( ER_OBJ, "LITUSE(%ld) has bad mapping to reloc record", litar_idx );
                        }
                        r_lita_relocation = &cur_obj->p_rlc_lita[ lita_idx ];


                        if (r_lita_relocation->r_extern) {
                            /* external -- lookup symbol */
                            pMEXTR  pmext = get_extmap(cur_obj->iextmap +r_lita_relocation->r_symndx);

                            if (pmext->mext_flags & MF_SOHIDDEN && pmext->resolved == WITH_OBJ)
                                r_literal_addr = pmext->aext.asym.value;
                            else {
                                r_literal_addr = 0;
                                r_literal_instr = NULL;
                            }
                        } else {
                            /* local -- lookup in .got */
                            r_literal_addr = mgot_local_literal_lookup( cur_obj, r_lita_offset );
                        }
                    }
                    break;

                  case CONST_CALL_SHARED:
                    /*
                     *  -call_shared
                     *  This means that locals are in the local got,
                     *      and external addresses are used only if not common (commons
                     *  might be changed from .so's) and defined from by
                     *  an object file.
                     */

                    {
                        long     litar_idx;         /* Index into lita_to_reloc[]*/
                        long     lita_idx;          /* Index into p_rlc_lita[] */
                        RELOC    *r_lita_relocation;

                        /*  Look at .lita relocation to see where symbol came from.  */
                        litar_idx = r_lita_offset/sizeof(long);
                        lita_idx = cur_obj->lita_to_reloc[ litar_idx ];

                        if (lita_idx == -1) {
                            error( ER_OBJ, "LITUSE(%ld) has bad mapping to reloc record", litar_idx );
                        }
                        r_lita_relocation = &cur_obj->p_rlc_lita[ lita_idx ];


                        if (r_lita_relocation->r_extern) {
                            /* external -- lookup symbol */
                            pMEXTR  pmext = get_extmap(cur_obj->iextmap + r_lita_relocation->r_symndx);

                            if (pmext->aext.asym.sc != scCommon &&
                                pmext->aext.asym.sc != scSCommon &&
                                pmext->resolved == WITH_OBJ) /* There is no -hidden check! */
                                r_literal_addr = pmext->aext.asym.value;
                            else {
                                r_literal_addr = 0;
                                r_literal_instr = NULL;
                            }
                        } else {
                            /* local -- lookup in got */
                            r_literal_addr = mgot_local_literal_lookup( cur_obj, r_lita_offset );
                        }
                    }
                    break;
                }

                if (!r_literal_instr) break;


                /*
                 * OPTIMIZATION POTENTIAL
                 *
                 *  This will check if
                 *
                 *      ldq  rx, disp(gp)
                 *
                 *      can be replaced with
                 *
                 *      lda  rx, disp2(gp)
                 *
                 *      based upon the value that would be loaded from got.
                 */


                if (r_literal_addr - gp_value < 32768L &&
                    r_literal_addr - gp_value >= -32768L)
                    {

                        r_literal_instr->m_format.opcode = op_lda;
                        r_literal_instr->m_format.memory_displacement = r_literal_addr - gp_value;
                    }
#endif
            }                               /* case R_LITERAL: */
            break;

#if defined(__alpha) || defined(__mips64)
          case R_BRADDR:                    /* 21-bit branch displacement */
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
            {
                union alpha_instruction *instr = (union alpha_instruction *) (p_raw + offset);

                immed = 4 * instr->b_format.branch_displacement; /* 21-bit sign extended */
                if (!preloc->r_extern) {
                    /*
                     * For R_BRADDR relocation types that are local
                     * references, the constant at the entry is the offset
                     * to the branch target.
                     */

                    immed += preloc->r_vaddr + 4;
                }

                /*
                 *  Do the relocation and check to see if the target of the
                 *  branch can be reached from the location of the branch.
                 */

                immed += value;             /* immed is now address of destination */

                /*
                 *  Add as label if -non_shared, or -call_shared and local, or -call_shared and
                 *  from a current object.
                 */

                if (optlevel > 0) {
                    if (sharable == CONST_NON_SHARED ||
                        sharable == CONST_CALL_SHARED && !preloc->r_extern ||
                        sharable == CONST_CALL_SHARED && pmext->resolved == WITH_OBJ)
                        ADD_LABEL(immed);

                    ADD_CONTROL_FLOW(preloc->r_vaddr - s_vaddr + f_vaddr)
                    }

                if (immed & 3) {
                    error(ER_WARNING, "branch at 0x%lx not to a longword destination\n",
                          preloc->r_vaddr - s_vaddr + f_vaddr);
                    immed &= ~ 3UL;
                }

                /*  Make pc relative  */
                immed -= preloc->r_vaddr - s_vaddr + f_vaddr + 4;

                /*  Store in instruction  */
                instr->b_format.branch_displacement = immed/4;

                /*  See if we're branching across shared library boundaries.  */
                if (preloc->r_extern &&
                    sharable > CONST_NON_SHARED &&
                    pmext->resolved == WITH_SO)
                    error(ER_WARNING, "branch to shared library symbol '%s' will cause unpredictable behavior.\n",
                          pmext->name);


                /*  See if we could store displacement in 21-bits  */
                if (instr->b_format.branch_displacement != immed/4)
                    error(ER_ERROR, "branch relocation out-of-range, bad object file produced, can't branch from 0x%lx to 0x%lx\n",
                          preloc->r_vaddr - s_vaddr + f_vaddr, immed + preloc->r_vaddr - s_vaddr + f_vaddr );

            }
            break;


          case R_HINT:                      /* 14 bits of hint for JSR  */
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
            {
                union alpha_instruction *instr = (union alpha_instruction *) (p_raw + offset);

                if (instr->common.opcode == op_bsr)
                    break;                  /* jsr was optimized to bsr--so no hint bits */

                immed = 4 * instr->j_format.hint; /* 14-bit sign extension */

                if (!preloc->r_extern) {
                    /*
                     * For R_HINT relocation types that are local references
                     * the constant at the entry is the offset to the jump target.
                     */
                    immed += preloc->r_vaddr + 4;
                }

                if (optlevel > 0)
                    ADD_CONTROL_FLOW(preloc->r_vaddr - s_vaddr + f_vaddr);

                /*
                 *  Do the relocation and check to see if the target of the
                 *  branch can be reached from the location of the branch.
                 */

                immed += value;

                /*
                 *  immed is now the absolute address, convert to pc-relative.
                 */

                immed -= preloc->r_vaddr - s_vaddr + f_vaddr + 4;

                if (immed & 3) {
                    error(ER_WARNING, "jump hint at 0x%lx not to a longword destination\n",
                          preloc->r_vaddr - s_vaddr + f_vaddr);
                    immed &= ~ 3UL;
                }

                /* IGNORE TESTING FOR RANGE */

                instr->j_format.hint = immed/4;
            }
            break;


          case R_GPVALUE:                   /* gp value relocation record */
            /*
             *  This relocation signifies a new gp value range is
             *  in effect.
             *
             *  'value' is offset within the new gp_value.
             */

            if (offset & 3)
                error(ER_OBJ, "bad R_GPVALUE relocation entry (r_vaddr unaligned)\n");

            if (preloc->r_extern) {
                error(ER_OBJ, "bad R_GPVALUE relocation entry (should not be external)\n");
                preloc->r_extern = 0;
            }

            /* (Emit exception stuff somewhere about here--BEFORE changing
               gp values -mjr)                       */

            mgot_set_cur_got( mgot_get_cur_got()+1 ); /* Advance to next GOT! */

            src_gp_value = cur_obj->aouthdr->gp_value + preloc->r_symndx;
            /* If generating a -r file we generate gp_value the old way.
             * else we are reading in a -r file so use the real gpvalue..
             */
            if (sharable == CONST_NON_SHARED) {
                gp_value = cur_obj->new_gp_value + value;
            } else {
                gp_value = cur_obj->new_gp_value - cur_obj->offset_from_output_gp
                    + mgot_got_offset( mgot_get_cur_got() ) * sizeof(Elf32_Addr);
            }
            gpvalue_i = i;

            if (s_type == R_SN_TEXT)
                output_gp_info(((preloc->r_vaddr-s_vaddr)+f_vaddr), gp_value);


            preloc->r_symndx = cur_obj->offset_from_output_gp + value;
            /* preloc->r_vaddr updated below */
            break;


          case R_GPDISP:                    /* lda/ldah instruction pair */
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
            {
                union alpha_instruction *lda_instr, *ldah_instr;


                lda_instr = (union alpha_instruction *) (p_raw + offset);
                ldah_instr = (union alpha_instruction *) (p_raw + offset + preloc->r_symndx);

                if (lda_instr->m_format.opcode == op_ldah) {
                    /*
                     *  Got them swapped.
                     */

                    lda_instr = (union alpha_instruction *) (p_raw + offset + preloc->r_symndx);
                    ldah_instr = (union alpha_instruction *) (p_raw + offset);
                }


                /*
                 *  Check for not finding the instruction pair.
                 */

                if (lda_instr->m_format.opcode != op_lda && ldah_instr->m_format.opcode != op_ldah)
                    error(ER_OBJ, "unable to find lda/ldah pair for GPDISP relocation (offset at %lx)\n",
                          preloc->r_vaddr);


                /*
                 *  Get offset from GP.
                 *  Note that memory_displacement is a signed bit-field, and it
                 *  is appropriately sign extended.
                 */

                immed = 65536 * ldah_instr->m_format.memory_displacement +
                    lda_instr->m_format.memory_displacement;
            
                /*
                 *  Do relocation.
                 */

                immed += value;

                /*
                 *  Check to make sure it can be put back into the
                 *  instruction pair.
                 */

                if ((unsigned long) immed >> 32 != 0 && (unsigned long) immed >> 32 != 0xffffffff)
                    error(ER_OBJ, "GPDISP relocation out of range (offset at %lx)\n",
                          preloc->r_vaddr);


                lda_instr->m_format.memory_displacement = immed;
                ldah_instr->m_format.memory_displacement = (immed + 32768) / 65536;

                /* Add compact relocations for R_GPDISP */
                /* Make sure dist2lo is less than or equal 3FF.  Otherwise ignore it */
                if (sharable > CONST_NON_SHARED && 
                    (preloc->r_symndx & 0xFFFFFC00) == 0 &&
                    compact_rlc) {
                    obj_cm_rlc_make(preloc, 
                                    (unsigned long)ldah_instr->m_format.memory_displacement,
                                    preloc->r_vaddr + f_vaddr - s_vaddr);
                    obj_cm_rlc_make(preloc,
                                    (unsigned long)lda_instr->m_format.memory_displacement,
                                    preloc->r_vaddr + f_vaddr - s_vaddr + preloc->r_symndx);
                }

                break;
            }


          case R_LITUSE:
            if (optlevel < 1 ||
                !final_link && preloc->r_extern ||
                !r_literal_instr)
                break;

            r_lituse_cnt++;

            if (preloc->r_vaddr & 0x3) {
                error(ER_OBJ, "bad R_LITUSE relocation alignment (offset at %lx)--entry not optimized.\n",
                      preloc->r_vaddr);
                break;
            }


            /*
             *  Determine the type of R_LITUSE, and check for appropriate optimizations.
             */

            r_lituse_instr = (union alpha_instruction *) (p_raw + offset);

            switch (preloc->r_symndx) {
                long net_gp_displacement;   /* Net displacement of object from GP */


              case R_LU_BASE:

                net_gp_displacement = r_literal_addr +
                    r_lituse_instr->m_format.memory_displacement - gp_value;

                /*
                 *  OPTIMIZATION POTENTIAL
                 *
                 *  ldq     rx, disp(gp)    R_LITERAL
                 *  ldq/stq ry, disp2(rx)   R_LITUSE(R_LU_BASE)
                 *  -->
                 *  ldq rx, disp(gp)
                 *  ldq/stq ry, disp3(gp)
                 */

                if (net_gp_displacement < 32768L &&
                    net_gp_displacement >= -32768L) {
                    r_lituse_instr->m_format.memory_displacement = net_gp_displacement;
                    r_lituse_instr->m_format.rb = GP;

                    /*
                     * Can nop the R_LITERAL if only 1 LITUSE exists.
                     */

                    if ((i+1 == nreloc || p_rlc[i+1].r_type != R_LITUSE)
                        && r_lituse_cnt == 1) {
                        alpha_pick_nop(r_literal_instr, r_literal_reloc->r_vaddr);
                    }
                    break;
                }



                /*
                 *  OPTIMIZATION POTENTIAL
                 *
                 *  ldq     rx, disp(gp)    R_LITERAL
                 *  ldq/stq ry, disp2(rx)   R_LITUSE(R_LU_BASE)
                 *  -->
                 *  ldah    rx, disp3(gp)
                 *  ldq/stq ry, disp4(rx)
                 *
                 *  This basically is testing if within signed 32-bit distance of gp.
                 *
                 *  This can currently only be done if exactly 1 R_LITUSE exists
                 *  for the R_LITERAL.
                 */

                if ((i+1 == nreloc          /* Very last relocation */ ||
                     p_rlc[i+1].r_type != R_LITUSE)
                    && r_lituse_cnt == 1    /* 1 and only 1 */
                    && net_gp_displacement < 0x7fff8000L &&
                    net_gp_displacement > -0x7fff8000L) {

                    r_literal_instr->m_format.opcode = op_ldah;
                    r_literal_instr->m_format.memory_displacement = (net_gp_displacement + 0x8000)>>16;

                    r_lituse_instr->m_format.memory_displacement = net_gp_displacement & 0xffff;
                }


                /*
                 *  OPTIMIZATION POTENTIAL
                 *
                 *  ldq     rx, disp(gp)    R_LITERAL
                 *  ldq/stq ry, disp2(rx)   R_LITUSE(R_LU_BASE)
                 *    (more than one)
                 *  -->
                 *  ldah    rx, disp3(gp)
                 *  ldq/stq ry, disp4(rx)
                 *    (more than one)
                 *
                 */

                if ((i+1 == nreloc          /* Very last relocation */ ||
                     (p_rlc[i+1].r_type != R_LITUSE)) /* more than one */
                    && r_lituse_cnt > 1 &&
                    optlevel > 1 &&         /* Must be >= 2 because it's "harder" */
                    net_gp_displacement < 0x7fff8000L &&
                    net_gp_displacement > -0x7fff8000L) {

                    RELOC  *p;
                    const signed int ldah_displacement = (signed short) ((net_gp_displacement + 0x8000)>>16);

                    /*
                     *  Pass through looking for any that fall outside of same ldah range.
                     */

                    for (p = r_literal_reloc+1; p < preloc; p++) {
                        union alpha_instruction *instr;

                        if (p->r_symndx != R_LU_BASE)
                            goto do_nothing;

                        instr = (union alpha_instruction *) (p_raw + p->r_vaddr - s_vaddr );
                        if ((signed short) ((r_literal_addr + instr->m_format.memory_displacement
                                             - gp_value + 0x8000)>>16) != ldah_displacement)
                            goto do_nothing;
                    }

                    /*
                     *  Now we know that everything will fit with the same ldah value
                     *  and is within 16 bits.  So change all of the offsets.
                     */

                    r_literal_instr->m_format.opcode = op_ldah;
                    r_literal_instr->m_format.memory_displacement = (net_gp_displacement + 0x8000)>>16;

                    for (p = r_literal_reloc+1; p <= preloc; p++) {
                        union alpha_instruction *instr;
                        instr = (union alpha_instruction *) (p_raw + p->r_vaddr - s_vaddr);

                        instr->m_format.memory_displacement = (r_literal_addr + instr->m_format.memory_displacement 
                                                               - gp_value) & 0xffff;
                    }

                  do_nothing: ;
                }


                break;

              case R_LU_BYTOFF:
                break;

              case R_LU_JSR:
                {
                    RELOC   *r_lita_relocation;
                    long    branch_displacement = 0;
                    int     gp_prologue_skipped = FALSE;
                    long    lita_idx;         /* Index into p_rlc_lita[] */
                    long    litar_idx;     /* Index into lita_to_reloc[]*/

                    /*  Look at .lita relocation to see where symbol came from.  */

                    litar_idx = r_lita_offset/sizeof(long);
                    lita_idx = cur_obj->lita_to_reloc[ litar_idx ];
                    if (lita_idx == -1) {
                        error( ER_OBJ, "LITUSE(%ld) has bad mapping to reloc record\n", litar_idx );
                    }
                    r_lita_relocation = &cur_obj->p_rlc_lita[ lita_idx ];

                    if (r_lita_relocation->r_extern) {
                        pMEXTR  pmext = get_extmap(cur_obj->iextmap + r_lita_relocation->r_symndx);

                        if ( ( !(pmext->mext_flags & MF_SOHIDDEN)
                              && sharable == CONST_MAKE_SHARABLE)
                            || pmext->resolved != WITH_OBJ)
                            break;          /* No transformation possible */


                        /*
                         *  See if there is a gp-prologue that can be skipped.
                         *  This is done if the destination uses gp, and the
                         *  gp values are the same.
                         */

                        if (pmext->pdr &&   /* Has a procedure descriptor (and -O2) */
                            pmext->pdr->gp_used && /* Procedure uses gp */
                            pmext->pobj->new_gp_value == gp_value) /* Same gp value (also fails if dest has multple gp) */
                            branch_displacement += pmext->pdr->gp_prologue;

                        gp_prologue_skipped = (branch_displacement != 0);
                    }

                    branch_displacement += r_literal_addr -
                        (preloc->r_vaddr - s_vaddr + f_vaddr + 4);
                    if (branch_displacement < -(0xfffffL << 2)
                        || branch_displacement > (0xfffffL << 2))
                        break;              /* too far */

                    if (branch_displacement & 3)
                        break;              /* an unaligned destination...something else happening */

                    r_lituse_instr->b_format.opcode = op_bsr;
                    /* b_format.b_format.ra is already correct */
                    r_lituse_instr->b_format.branch_displacement = branch_displacement/4;

                    if ((i+1 == nreloc      /* Very last relocation */ ||
                         (p_rlc[i+1].r_type != R_LITUSE)) /* more than one */
                        && r_lituse_cnt == 1 &&
                        gp_prologue_skipped) {

                        /*  Can remove load into pv  */
                        alpha_pick_nop(r_literal_instr, r_literal_reloc->r_vaddr);
                    }
                }                           /* case R_LU_JSR: */
                break;

                default :
                error(ER_OBJ, "bad R_LITUSE relocation subtype (offset at %lx)--entry not optimized.\n",
                      preloc->r_vaddr);
            }

            break;
#endif                                      /* __alpha */


          default:
#if defined(__alpha) || defined(__mips64)
            r_literal_instr = NULL;         /* Any R_LITUSE not in effect */
#endif
            error(ER_OBJ, "bad relocation entry (r_type)");
        }

        /*****************************************************************************
         *                                                                           *
         *                                                                           *
         * ************ end of swtich (preloc->r_type), about 1620 lines ago AAA *****
         *                                                                           *
         *                                                                           *
         *****************************************************************************/






#if defined(__alpha) || defined(__mips64)
        /*
         *  .lita entries get merged after relocation.
         */

        if (s_flags & STYP_LITA) {
            if (staticflag && !relocatable && sharable == CONST_NON_SHARED) {
                long new_offset = lookup_mlita((long *) (p_raw + offset));

                if (new_offset != -1L)
                    *(long *)(p_raw + offset) = new_offset;
                else
                    *(long *)(p_raw + offset) = enter_mlita(p_raw + offset);

            }
            else if (sharable > CONST_NON_SHARED) {
                /*
                 *  Making a .got
                 */
#ifdef DEBUG_JPM
                if (s_type != R_SN_LITA) {
                    printf("relocate: ********** s_flags has STYP_LITA set, but s_type is %d.\n", s_type);
                }
#endif            
                if (preloc->r_extern) {
                    *(long *)(p_raw + offset) = mgot_get_got_idx_for_pmext( cur_obj, pmext )
                        * sizeof( Elf32_Addr );
#ifdef DEBUG
                    printf("relocate: raw[%d]=%ld EXTERN '%s'\n", 
                           offset/8, *(long *)(p_raw+offset), pmext->name );
#endif
                } else {
                    if (preloc->r_type != R_GPVALUE)  {
                        int pimmed_idx;
                        pimmed_idx = i + mgot_pimmed_offset() - gpvalue_i;
                        LD_ASSERT( cur_obj->pimmed[ pimmed_idx ].got_index != (FUNNY_IMMED & 0xFFFFFFFF) );
                        *(long *)(p_raw + offset) = cur_obj->pimmed[ pimmed_idx ].got_index
                            + mgot_got_offset( mgot_get_cur_got() ) * sizeof( Elf32_Addr );
#ifdef DEBUG
                        printf("relocate: raw[%d]=%ld, LOCAL, i=%d, pimmed_idx=%d, gotidx=%ld\n", 
                               offset/8, *(long *)(p_raw+offset), i, pimmed_idx,
                               cur_obj->pimmed[ pimmed_idx ].got_index );
#endif
                    }
                }                           /* end else */
            }                               /* else if (sharable > CONST_NON_SHARED) */
        }                                   /* if (s_flags & STYP_LITA) */
#endif


        /*
         * If the final object is to be relocatable and the relocation
         * entry is a external reference convert the entry to a local
         * entry so it can be relocated on another load.
         * Stack ops usually update their own.
         */
        if (preloc->r_type != R_OP_PUSH && preloc->r_type != R_OP_PSUB &&
            preloc->r_type != R_OP_PRSHIFT && relocatable) {

            if (preloc->r_extern)
                mk_local(preloc, pmext);
            preloc->r_vaddr += f_vaddr - s_vaddr;
        }
    }                                       /* for (i=j; i < nreloc; i++) */
    /************ END OF FOR LOOP PROCESSING preloc=&(p_rlc[i]);   *****************************/
    if (tos != -1) {
        error(ER_PROG, "relocation stack not empty on exit from relocate\n");
    }                                       /* if */

    if (s_type == R_SN_TEXT)
        output_gp_info(f_vaddr+s_size, gp_value);

}







/*
 * mk_local() converts a relocation entry for an external relocation into
 * an entry for a local relocation.
 */
static
void
mk_local(preloc, pmext)
RELOC *preloc;  /* a pointer to the relocation entry to be converted */
pMEXTR pmext;   /* a pointer to the symbol this entry refers to */
{
    /*
     * Leave relocation entries refering to symbols in the base file
     * of an incremental load as external entries.
     */
    if(pmext->Aflag || pmext->aext.asym.sc == scUndefined ||
       pmext->aext.asym.sc == scSUndefined)
        return;

    switch(pmext->aext.asym.sc){
      case scText:
        preloc->r_symndx = R_SN_TEXT;
        break;
      case scInit:
        preloc->r_symndx = R_SN_INIT;
        break;
      case scFini:
        preloc->r_symndx = R_SN_FINI;
        break;
      case scRData:
        preloc->r_symndx = R_SN_RDATA;
        break;
      case scData:
        preloc->r_symndx = R_SN_DATA;
        break;
      case scSData:
        preloc->r_symndx = R_SN_SDATA;
        break;
      case scSBss:
        preloc->r_symndx = R_SN_SBSS;
        break;
      case scBss:
        preloc->r_symndx = R_SN_BSS;
        break;
      case scCommon:
      case scSCommon:
        /*
         * Relocation entries for common symbols have to remain external.
         */
        return;
      case scAbs:
        /*
         * Relocation entries for absolute symbols have to have their
         * relocation types changed to R_ABS so they don't get relocated
         * again.
         */
        preloc->r_type = R_ABS;
        return;
      case scXData:
        if (need_excinfo) {
            preloc->r_symndx = R_SN_XDATA;
            break;
        }
      case scPData:
        if (need_excinfo) {
            preloc->r_symndx = R_SN_PDATA;
            break;
        }
      default:
        error(ER_OBJ, "bad symbol for relocation entry");
    }
    preloc->r_extern = 0;
}
