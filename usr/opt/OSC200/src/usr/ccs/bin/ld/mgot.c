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
static char *rcsid = "@(#)$RCSfile: mgot.c,v $ $Revision: 1.1.5.8 $ (DEC) $Date: 1993/10/13 16:10:06 $";
#endif

#include "stamp.h"
#include "sex.h"
#include "ar.h"
#include "filehdr.h"
#include "aouthdr.h"
#include "sym.h"
#include "symconst.h"
#include "scnhdr.h"
#include "reloc.h"
#include "ext_tbl.h"
#include "got.h"
#include "obj_file.h"
#include "file_tbl.h"
#include "ld.h"
#include "pass1.h"
#include "layout.h"
#include "read.h"
#include "extlink.h"
#include "dn_tbl.h"
#include "doup.h"
#include "gp_tbl.h"
#include "literal.h"
#include "list.h"
#include "write.h"
#include "elf_abi.h"
#include "elf_mips.h"
#include "segment.h"
#include "assert.h"
#include "syms.h"
#include "sys/param.h"

#include "mgot.h"           /* Get prototypes for multiple got routines */


/* All these can be made static, are global for now for debugging.. */
#ifdef  DEBUG
#define STATIC
#else
#define STATIC static
#endif     


extern long vflag;                          /* verbose flag from ld.c (ld -v ) */

STATIC struct got_desc
{
    unsigned long           num_globals;

    unsigned long           num_locals;             /* The 0th entry here is for the loader */
    unsigned long           * locals;               /* The local got */


    unsigned long           num_p1_local_gots;      /* Number of entries in pass1 LOCAL_GOTs:  */
    struct st_p1_gots {
        unsigned long       num_p1_locals;          /* # of entries in p1_got array */
        pLOCAL_GOT          p1_got;                 /* temp local got area; is 2x local got size;
                                                     * See mgot_fixup_local_gots()
                                                     */
        OBJ_FILER           * cur_obj;              /* Ptr to the curobj that contributed these locals */
    } * p1_gots;                                    /* An array of structs  */
    unsigned long           num_cur_objs;           /* Num of objs that are in this got */
    pOBJ_FILER              * cur_objs;             /* an array of pointers to the cur_objs */
    int                     pimmed_offset;          /* index into cur_obj->pimmed for this got. */
                                                    /* Normally 0, but for ld -r files we may have multiple
                                                     *  gots per object, and thus need to idx into pimmed in
                                                     *  relocate()
                                                     */
    
    
} * got_tables;

STATIC unsigned long     num_got_tables = 0;
STATIC unsigned long     cur_got = -1;      /* No got yet allocated */

STATIC int               max_got_count = GP_SIZE/sizeof(Elf32_Addr); /* default is 65512/8 = 8189 */
                                            /* This is the max # of entries in a got table;
                                             * This count includes the loader entry; so there are
                                             *  0..8188 entries max per got.
                                             * Set this way so I could test with small gots..
                                             */
STATIC int               max_got_threshold = GP_SIZE/sizeof(Elf32_Addr);
                                            /* Typically set to same as max_got_count;
                                             * This value is used by mgot_check() to decide
                                             * if the object we are attempting to add should be
                                             * put into the next GOT; Used mostly in debugging
                                             * to generate lots of GOT tables.
                                             */
                                            /* This is the max # of entries in a got table;
                                             * This count includes the loader entry; so there are
                                             *  0..8188 entries max per got.
                                             * Set this way so I could test with small gots..
                                             */
STATIC int              max_got_size = GP_SIZE; /* value ld now uses in place of GP_SIZE;
                                                 * is max # of bytes in got table;
                                                 * is max_got_count * 8 !
                                                 * (really is  (max_got_count+1)*8 - 1 );
                                                 */




/*
 * Routine:     mgot_max_got_count()
 *    This routine returns the max # of entries in a got table.
 *      Used to be GP_SIZE/8, now is max_got_size/8
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    # of entries!
 *   
 */
int mgot_max_got_count()
{
    return( max_got_count );
}




/*
 * Routine:     mgot_max_got_size()
 *    This routine returns the max # of bytes one can have in a got. 
 *    ld used to use GP_SIZE (a constant) for this; But for testing multi
 *      got objects I wanted to be able to vary this.
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    the number of bytes!
 *   
 */
int mgot_max_got_size()
{
    return( max_got_size );
}



#if defined(__mips64)
/*
 * Routine:     bdrealloc()
 *    This routine is a version of realloc we use in the cross environment; 
 *      The problem is that the osf realloc enjoys doing the first malloc
 *      for a block (see the man page), but the ultrix version objects...
 *      So if compiling cross, avoid having ultrix die with a bus error.
 * Inputs:
 *    same as realloc
 * Outputs:
 *    -
 * Returns:
 *    ptr to the block
 *   
 */
void * bdrealloc( void * ptr, size_t size )
{
    char * retptr;

    if (ptr == 0) {
        retptr = (char *)malloc( size );
    }
    else retptr= (char *)realloc( ptr, size );

    return( retptr );
}
#define realloc bdrealloc
#endif




/*
 * Routine:     mgot_print()
 *    This routine dumps my got chain
 * Inputs:
 *    local vars in this module
 * Outputs:
 *    No side effects
 * Returns:
 *    Nothing
 *   
 */
mgot_print()
{
    long         gidx;
    long         di=0;                                      /* displayed got index */
    long         i, mexti;
    pOBJ_FILER   pobj;
    int          process_this_one;
    long         goti;
    pMEXTR       pmext;

    printf("-------------------\n");
    printf("\n%ld got table%s max_got_count=%ld, cur_got=%ld, max_got_threshold=%d\n",
           num_got_tables, num_got_tables > 1 ? "s;" : ";", max_got_count, cur_got, max_got_threshold);


    for (gidx = 0; gidx < num_got_tables; gidx++) {
        printf("GOT[%ld]: %ld entries; %ld locals, %ld globals, %ld object files\n",
               gidx,
               got_tables[ gidx ].num_locals + got_tables[ gidx ].num_globals,
               got_tables[ gidx ].num_locals-1,             /* dont count loader entry */
               got_tables[ gidx ].num_globals,
               got_tables[ gidx ].num_cur_objs );
        for (i = 0; i < got_tables[ gidx ].num_cur_objs; i++) {
            pobj = got_tables[ gidx ].cur_objs[i];
            printf("    %s\n", pobj ? pobj->name : "--no obj file--");
        }

        printf("\n   local    value\n");
        printf("    %2d[%4d]  %s\n", gidx, di++, "-- loader -- ");
        /* Entry 0 of locals is for loader: */
        for (i = 1; i < got_tables[ gidx ].num_locals; i++) {
            printf("    %2d[%4d]  %10lx\n",
                   gidx,
                   di++,
                   got_tables[ gidx ].locals[i] );
        }

        printf("\n   global         name                       #gots/ got#[idx]\n");

        /* Now walk thru the merged external mapped array (ie, its already sorted for us),
         * and output the globals that are in the current got
         */
        for (mexti = 0; mexti < imextMax; mexti++) {
            if (pextmap)
                pmext = map_xmext( mexti );
            else
                pmext = xmext( mexti );

            for (goti = 0; goti < pmext->isin_num_got_tables; goti++) {
                if (gidx == pmext->got_tbls[ goti ]) {
                    printf("    %2d[%4d]  %-30s  %4d/",
                           gidx,
                           di++,                            /* Show a pseudo got index */
                           pmext->name,
                           pmext->isin_num_got_tables);

                    for (goti = 0; goti < pmext->isin_num_got_tables; goti++) {
                        printf("%2d[%2d]  ",
                               pmext->got_tbls[ goti ],
                               pmext->got_tbl_idx[ goti ] );
                    }
                    printf("\n");
                    break;
                }
            }
        }
        printf("-------------------\n");
    }
}




/*
 * Routine:     mgot_add_curobj_to_got()
 *    Add the cur_obj struct pointer to the list of curobjs
 *      that are using this got
 *
 * Called from:
 *              mgot_alloc()
 *              mgot_check()
 * Inputs:
 *    cur_obj
 * Outputs:
 *   No side effects
 * Returns:
 *   Nothing
 *   
 */
STATIC void mgot_add_curobj_to_got( pOBJ_FILER pobjfile )
{
    long    i;

    i = got_tables[ cur_got ].num_cur_objs++;

    got_tables[ cur_got ].cur_objs = (pOBJ_FILER *)realloc(
                                      got_tables[ cur_got ].cur_objs,
                                      MAX( (i+10), 30) * sizeof(pOBJ_FILER *));

    if (got_tables[ cur_got ].cur_objs  == (pOBJ_FILER *)0) {
        error( ER_FATAL, "Out of memory allocating GOT curobjs\n" );
    }
    got_tables[ cur_got ].cur_objs[ i ] = pobjfile;
    if (pobjfile != (pOBJ_FILER) 0) {     /* dont crash on initial entries */
        if (pobjfile->first_got_num == -1) { /* if not yet init'd */
            pobjfile->first_got_num = cur_got; 
        }
    }
}



/*
 * Routine:     mgot_collect_got()
 *    This routine records whether a merged symbol is used in the current got
 *      and updates the merged sym to reflect the gots the sym is in.
 *      called from ld.c after pass1() and before layout(), for each object file.
 *
 * Called from:
 *              mgot_alloc()
 *
 * Inputs:
 *    static locals
 * Outputs:
 *    Updates the current got entry with globals..
 * Returns:
 *    Nothing
 *   
 */
void    mgot_collect_got()
{
    long        i;
    unsigned long   tot_entries_in_got;
    MEXTR       * pmext, * new_mext;

    if (num_got_tables > 0) {
        /* Extract all merged symbols into current got: */
        for (i = 0; i != imextMax; i++) {
            pmext = xmext( i );
            pmext->mgot_used |= pmext->used;                /* Save used bit */
            if (pmext->used) {
                pmext->isin_num_got_tables++;
                pmext->got_tbls = (void *)realloc( pmext->got_tbls,
                                                  pmext->isin_num_got_tables * sizeof(unsigned long) );
                if (pmext->got_tbls == (void *)0)
                    error( ER_FATAL, "Out of memory allocating got table idx\n" );


                pmext->got_tbl_idx = (void *)realloc( pmext->got_tbl_idx,
                                                     pmext->isin_num_got_tables * sizeof(unsigned long) );
                if (pmext->got_tbl_idx == (void *)0)
                    error( ER_FATAL, "Out of memory allocating got table idx\n" );

                /* Now while we know that this merged external is a member
                 * of the current got, we dont know what the real got index
                 * for this sym is; We dont know that til after sorting the
                 * merged array.. So we just note the fact the data isnt avail..
                 */
                pmext->got_tbls[ pmext->isin_num_got_tables-1 ] = cur_got;
                pmext->got_tbl_idx[ pmext->isin_num_got_tables-1 ] = -1;

                got_tables[ cur_got ].num_globals++;

                pmext->used = 0;                            /* Note used field says it should go in GOT */
                --tot_num_used_mexts;
                ASSERT(tot_num_used_mexts >= 0);            /* just in case .. */
            }                                               /* /if (pmext->used) */
        }                                                   /* for (i = 0; i != imextMax... */
        /* Now lets just check that we havent overflowed the current got;
         * If we are given bad input objects with too many locals or globals this can
         * happen;
         */
        tot_entries_in_got = got_tables[ cur_got ].num_locals + got_tables[ cur_got ].num_globals;
#ifdef DEBUG
        if (vflag) {
            printf("GOT[%ld] %ld entries; %ld globals + %ld locals\n",
                   cur_got,
                   got_tables[ cur_got ].num_globals + got_tables[ cur_got ].num_locals,
                   got_tables[ cur_got ].num_globals,
                   got_tables[ cur_got ].num_locals );
        }
#endif
        if (tot_entries_in_got > max_got_count) {           /* note num_locals includes loader entry already */
            char * nameptr;
            int cur_obj = got_tables[ cur_got ].num_cur_objs - 1;
            if (cur_obj >= 0) 
                nameptr = mgot_curobj_name( got_tables[cur_got].cur_objs[ cur_obj ] );
            else
                nameptr = "?";

            error( ER_FATAL,
                  "Too many GOT entries in object file '%s';\n    Found %ld (%ld locals + %ld globals) but max is %d\n",
                  nameptr,
                  tot_entries_in_got,
                  got_tables[ cur_got ].num_locals,
                  got_tables[ cur_got ].num_globals,
                  max_got_count );
        }
        
    }                                                       /* if (num_got_tables > 0) */
}





/*
 * Routine:     mgot_got_offset()
 *    Tells you the offset (ie # of got entries) from the first got entry
 *      in got 0 to the start of the current got 
 *
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    long offset (# of got entries, not a byte offest)
 *   
 */
unsigned long mgot_got_offset( unsigned long gotnum )
{
    long    i;
    long    offset = 0;
    
    switch (sharable) {
      case CONST_NON_SHARED:    return( 0 );

      case CONST_MAKE_SHARABLE: 
      case CONST_CALL_SHARED:   
      default:
        LD_ASSERT( gotnum != -1 );                          /* Lets hope we established this already! */
    }

    for (i = 0; i < gotnum; i++) {
        offset += got_tables[ i ].num_globals + got_tables[ i ].num_locals;
    }
    return( offset );
}





/*
 * Routine:     mgot_fillin_got_idxs()
 *    This routine fills into each merged external the got index for the symbol in the got.
 *
 * Called from:
 *          layout()
 *
 * Inputs:
 *    -
 * Outputs:
 *    all merged externals which are in gots are setup.
 * Returns:
 *    Zip
 *   
 */
void mgot_fillin_got_idxs()
{
    long        mnum;                                       /* index into merged externals array */
    long        gotnum;                                     /* # of got currently processing */
    MEXTR       * pmext;
    long        * got_idxs;
    

    if (num_got_tables == (long)0) return;                /* qar 13415 */

    got_idxs = (long *)malloc( num_got_tables * sizeof(long) );
    if (got_idxs == (void *)0)
        error( ER_FATAL, "Out of memory allocating got table idxs\n" );

    bzero( got_idxs, num_got_tables * sizeof(long) );
    for (gotnum = 0; gotnum < num_got_tables; gotnum++) {
        got_idxs[ gotnum ] = mgot_got_offset( gotnum ) + got_tables[ gotnum ].num_locals;
    }

    for (mnum = 0; mnum != imextMax; mnum++) {
        pmext = map_xmext( mnum );
        for (gotnum = 0; gotnum < pmext->isin_num_got_tables; gotnum++ ) {
            LD_ASSERT( pmext->got_tbl_idx[ gotnum ] == -1 ); /* shldnt have init'd this already..  */
            pmext->got_tbl_idx[ gotnum ] = got_idxs[ pmext->got_tbls[gotnum] ]++;
        }
    }

    
}



/*
 * Routine:     mgot_get_got_idx_for_pmext()
 *    Returns the got index for this merged external in the current got.
 *    The passed in symbol must be in the current got or we will assert.
 *    The current got is ascertained from the current object.
 *
 * Called from:
 *          relocate()
 *
 * Inputs:
 *    the merged external sym ptr (pmext)
 * Outputs:
 *    -
 * Returns:
 *    The got index relative to the overall got base;
 *    That is if the first got has 10 symbols (counting the loader and locals and globals)
 *    and the 2nd got has 4 locals (3 real locals + loader), then the first global
 *    in the 2nd got will be at offset 10 + 4 = 14
 *   
 */
long mgot_get_got_idx_for_pmext( pOBJ_FILER pobj,  pMEXTR pmext )
{
    long        i;
    long        this_got;
    

    for (i = 0; i < pmext->isin_num_got_tables; i++) {
        if (pmext->got_tbls[ i ] != cur_got) continue;

        /* Found the sym claims to be in the current got.. */
        LD_ASSERT( pmext->got_tbl_idx[ i ] != -1 );

        return( pmext->got_tbl_idx[ i ] );
    }
#ifdef DEBUG_JPM
    mgot_print();
#endif
    error( ER_FATAL, "mgot_get_got_idx_for_pmext: Found symbol(%s) isnt in current got(%ld)\n",
          pmext->name, cur_got );
    return( 0 );
}


/*
 * Routine:     mgot_restore_mext_used_bits()
 *    This routine sets the used bits in merged external symtbl
 *      if for any of the multiple GOTs the bits were set;
 *      Called from layout when setting up to build dynsym tbl.
 *
 * Called from:
 *              layout()
 *
 * Inputs:
 *    
 * Outputs:
 *    
 * Returns:
 *    Nothing
 *   
 */
void mgot_restore_mext_used_bits()
{
    long        i;
    MEXTR       * pmext;

    
    tot_num_used_mexts = 0;
    for (i = 0; i != imextMax; i++) {
        pmext = xmext( i );
        pmext->used |= pmext->mgot_used;
        if (pmext->used) {
            ++tot_num_used_mexts;
        }
    }
}





/*
 * Routine:     mgot_num_used_mexts()
 *    Find out how many mexts are in use in ld's master table; 
 *      Tells us how many global got entries are needed so far
 *      This count will be the got_tables[].num_globals once we have
 *      run mgot_collect_got().
 * Inputs:
 *    
 * Outputs:
 *    
 * Returns:
 *    # of mexts with used bit set. (meaning they are to go into the GOT)
 *   
 */
long    tot_num_used_mexts = 0;

long    mgot_num_used_mexts()
{
#ifdef DEBUG_MEXTS
    long        i;
    long        num_used = 0;
    MEXTR       * pmext, * new_mext;

    for (i = 0; i != imextMax; i++) {
        pmext = xmext( i );
        if (pmext->used){
            num_used++;
        }
    }
    if (num_used != tot_num_used_mexts) {
        error(ER_FATAL,"mgot_num_used_mexts: expected: %ld, but found %ld\n",
              tot_num_used_mexts, num_used );
    }
        
    return( num_used );
#endif
    return( tot_num_used_mexts );
}     




/*
 * Routine:     mgot_pmext_in_use()
 *    Sets the used bit in the pmext and records the total count
 *      in a global; We do this as a rtn so we can maintain a sanity check;
 *      Turns out that in large mgot applications we spent lots of time
 *      in mgot_num_used_mexts() when we used to rescan the mext chain!
 *
 * Called from:
 *          lots of places
 *
 * Inputs:
 *    an mext
 * Outputs:
 *    Sets the used bit, updates global
 * Returns:
 *    Nothing
 *   
 */
void mgot_pmext_in_use( MEXTR * pmext )
{     
    if (pmext->used) return;
    pmext->used = 1;
    ++tot_num_used_mexts;
}




/*
 * Routine:     mgot_alloc()
 *    allocate a new got & update cur_got
 *
 * Called from:
 *          mgot_check()
 *
 * Inputs:
 *    num_relocs   is max num relocs we think this files is adding;
 *                      not needed for this routine, since we WILL
 *                      alloc a new got here; Is used for debugging.
 *    static locals
 * Outputs:
 *    cur_got is set to be the index of the new got.
 * Returns:
 *    Nothing
 *   
 */
void mgot_alloc( pOBJ_FILER pobjfile, unsigned long num_relocs)
{
    long    i;
    long    incr_size;                      /* Num bytes we are growing got_tables by */

    mgot_collect_got();
    cur_got = num_got_tables++;             /* This will point to the got we are about to add */
    
    incr_size = sizeof(struct got_desc) * (num_got_tables + 10);

    got_tables = (struct got_desc *) realloc( got_tables, incr_size );
    if (got_tables == (struct got_desc *)0) {
        error( ER_FATAL, "Out of memory allocating got table descriptors\n" );
    }

    memset( &got_tables[ cur_got ], 0x00, sizeof( struct got_desc ) );

    mgot_add_curobj_to_got( pobjfile );

    /* Allocate the 0th local for the loader: */
    i = got_tables[ cur_got ].num_locals++;
    got_tables[ cur_got ].locals = (void *) realloc(
                                                    got_tables[ cur_got ].locals,
                                                    MAX( (i+500) * sizeof(unsigned long), 8192) );
    
    if (got_tables[ cur_got ].locals  == (void *)0) {
        error( ER_FATAL, "Out of memory allocating GOT locals\n" );
    }
    got_tables[ cur_got ].locals[ 0 ] = 0;

#ifdef DEBUG
    if (vflag) {
        printf("GOT[%ld] allocated; adding obj %s with %ld relocs, [%ld] has %ld entries; %ld globals + %ld locals\n",
               cur_got, mgot_curobj_name(pobjfile), num_relocs,
               cur_got,
               got_tables[ cur_got ].num_globals + got_tables[ cur_got ].num_locals,
               got_tables[ cur_got ].num_globals,
               got_tables[ cur_got ].num_locals );
    }
#endif
}








/* 
 * Routine:     mgot_curobj_name()
 *    This routine returns a pointer to a string which is the name of the object
 *      module in question. We may be procesing an archive so it might be qualified
 *      to be archive.a(module.o)
 *
 * Inputs:
 *    pobjfile      is cur_obj
 * Outputs:
 *    -
 * Returns:
 *    Pointer to a string;
 *   
 */
char * mgot_curobj_name( pOBJ_FILER pobjfile )
{
    static char cur_name[ MAXPATHLEN ]; 
    
    if (pobjfile->ftype != FTYPE_AR) return( pobjfile->name );
    /* Looks like an archive; lets build the name...*/
    sprintf(cur_name, "%s(%s)", pobjfile->name, pobjfile->member_name );
    return( cur_name );
}







/*
 * Routine:     mgot_check()
 *    This routine sees if there is enough room in the current got
 *      to add an addtional N relocs;
 *    If there isnt we will create a new GOT.
 *
 * Called from:
 *              alloc_got() from merge() from pass1()
 *
 * Inputs:
 *    pobjfile      is cur_obj
 *    num_relocs    number of relocs we want to add to the got
 * Outputs:
 *    May create a new got
 * Returns:
 *    Nothing
 *   
 */
void mgot_check(pOBJ_FILER pobjfile,  unsigned long num_relocs )
{
    unsigned long    gotsz = 0;
    unsigned long    i;

    gotsz = mgot_num_used_mexts();

    if (got_tables != (struct got_desc *)0) { /* if a got exists */
        gotsz += got_tables[ cur_got ].num_locals;
    }
    
    if ( (gotsz + num_relocs) > max_got_threshold) {
        if (pobjfile->first_got_num == -1) {         /* if not yet init'd */
            pobjfile->first_got_num = cur_got+1;     /* we are going to alloc a new got for this obj */
        }
        mgot_alloc( pobjfile, num_relocs );          /* Only allocate the 1 got we are requesting */
                                                     /* The obj may be a -r which needs multiple but
                                                      * we will get to them later (in alloc_got())
                                                      */
        cur_got = pobjfile->first_got_num;
    }
    else {
        if (got_tables == (struct got_desc *)0) {
            mgot_alloc( pobjfile, num_relocs );
        } else {
            mgot_add_curobj_to_got( pobjfile );
        }
        if (pobjfile->first_got_num == -1) {         /* if not yet init'd */
            pobjfile->first_got_num = cur_got;
        }
#ifdef DEBUG
        if (vflag) {
            printf("GOT[%ld] adding obj %s with %ld relocs, [%ld] has %ld entries; %ld globals + %ld locals\n",
                   cur_got, mgot_curobj_name(pobjfile), num_relocs,
                   cur_got,
                   got_tables[ cur_got ].num_globals + got_tables[ cur_got ].num_locals,
                   got_tables[ cur_got ].num_globals,
                   got_tables[ cur_got ].num_locals );
        }
#endif
    }
}





/*
 * Routine:     mgot_note_local()
 *    This routine will note that there will be a local GOT entry created for p
 *      We set the got_index for p, and reserve space for the entry.
 *      The real value of the entry is filled in later via fix_got();
 *
 * Called from:
 *          get_got_tbl() from alloc_got() from merge() from pass1()
 *
 * Inputs:
 *    p     the IMMEDiate
 * Outputs:
 *    p->got_index is set to the index of the new local in the current got * sizeof(Elf32_Addr)
 * Returns:
 *    Nothing
 *   
 */
void mgot_note_local( pOBJ_FILER pobj, pIMMED  p )
{
    long    i;

    i = got_tables[ cur_got ].num_locals++;
    got_tables[ cur_got ].locals = (void *) realloc(
                                      got_tables[ cur_got ].locals,
                                      MAX((i+500) * sizeof(unsigned long), 8192));

    if (got_tables[ cur_got ].locals  == (void *)0) {
        error( ER_FATAL, "Out of memory allocating GOT locals\n" );
    }
    got_tables[ cur_got ].locals[ i ] = -1;
    p->got_index = i * sizeof(Elf32_Addr);
}








/*
 * Routine:     mgot_get_cur_got()
 *    This routine returns the value of cur_got the current got number.
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    Value of cur_got()
 *   
 */
unsigned long   mgot_get_cur_got( void )
{
    return( cur_got );
}



/*
 * Routine:     mgot_set_cur_got()
 *    This routine returns sets the value of cur_got to the value passed in.
 * Inputs:
 *    New value for cur_got
 * Outputs:
 *    -
 * Returns:
 *    -
 *   
 */
void    mgot_set_cur_got( unsigned long new_value )
{
    if (sharable == CONST_NON_SHARED) {
        /* If non-shared we might get a value of 0 or 1 in as new_value; Thats cause relocate()
         *   when it sees a GPVALUE reloc will try to inc cur_got. So lets just stay at got 0
         *   in the non-shared case...
         */
        cur_got = 0;  
    }
    else {
        LD_ASSERT( new_value < num_got_tables ); /* make sure value is valid */
        cur_got = new_value;
    }
}







/* 
 *  mgot_local_literal_lookup( cur_obj, r_lita_offset )
 * 
 *  This routine is given a lita offset and cur_obj and returns the literal addr
 *      from the local got.
 *
 * Called from:
 *          relocate()
 *
 * Inputs:
 *      cur_obj
 *      r_lita_offset
 *      We know that cur_got reflects the got we are currently processing.
 * Outputs:
 *      No side effects
 * Returns:
 *   long r_literal_addr
 *   
 *   
 */
long mgot_local_literal_lookup( pOBJ_FILER cur_obj, int r_lita_offset )
{
    long    r_literal_addr;
    long    lita_idx;
    long    local_real_idx;                                 /* index into cur got locals */

                        
    lita_idx = r_lita_offset/sizeof(Elf32_Addr);            /* idx into .lita */

    /* now we take the relative index and adjust it to where it really is taking into account
     * the local GOT we have built thus far.
     * Take the current relative index and account for current got size
     */
    local_real_idx = cur_obj->p_raw_lita[ lita_idx ] / sizeof(Elf32_Addr)
        - mgot_got_offset( cur_got );
    
    r_literal_addr = got_tables[ cur_got ].locals[ local_real_idx ];
    return( r_literal_addr );
}




/*
 * Routine:     mgot_get_num_locals()
 *    This routine returns the number of locals in the current got
 *     INCLUDES the one used for rld lazy handler.
 * Inputs:
 *    -
 * Outputs:
 *    No side effects
 * Returns:
 *    # of locals!
 *   
 */
unsigned long    mgot_get_num_locals()
{     
    if ( got_tables == (void *)0 ) return( 0 );             /* if no gots have been allocated then 0 locals ! */

    return( got_tables[ cur_got ].num_locals );
}







/*
 * Routine:     mgot_get_total_num_got_entries()
 *    This routine returns the number of got entries which is:
 *      + # locals in each got (includes the reserved entry for rld)
 *      + # globlals in each got
 * Inputs:
 *    None
 * Outputs:
 *    No side effects
 * Returns:
 *    Total number of got entries
 *   
 */
unsigned long    mgot_get_total_num_got_entries()
{
    long                i;
    unsigned long       gotsz;

    gotsz = 0;
    for (i = 0; i < num_got_tables; i++) {
        gotsz += got_tables[ i ].num_globals + got_tables[ i ].num_locals;
    }
    return( gotsz );
}     







/*
 * Routine:     mgot_get_num_globals()
 *    This routine returns the number of globals in the current got
 * Inputs:
 *    pobj tells us which got we are interested in; we will use the got the
 *          current objects locals are stored in.
 * Outputs:
 *    No side effects
 * Returns:
 *    # of globals!
 *   
 */
unsigned long    mgot_get_num_globals( pOBJ_FILER pobj )
{     
    LD_ASSERT( pobj->first_got_num != -1 );

    if (pobj->num_gots_in_obj > 1)  error(ER_FATAL, "num gots > 1 in obj %s\n", pobj->name ); /* jpm fixup */
        
    return( got_tables[ pobj->first_got_num ].num_globals );
}





/*
 * Routine:     mgot_get_num_merged()
 *    This routine returns the number of merged syms in the current got
 * Inputs:
 *    None
 * Outputs:
 *    No side effects
 * Returns:
 *    # of merged syms
 *   
 */
unsigned long    mgot_get_num_merged()
{     
    return( imextMax );
}




/*
 * Routine:     mgot_get_num_dynsyms()
 *    This routine returns the number of dynamic symbol entries that there will be;
 *      This routine is called from make_dsoscn() during layout().
 * Inputs:
 *    The merged externals array; All the got 
 * Outputs:
 *    No side effects
 * Returns:
 *    # of merged syms
 *   
 */
unsigned long    mgot_get_num_dynsyms()
{
    long    numsyms;
    long    i;
    pMEXTR  pmext;

    /* if there are no duplicated ref'd globals the # is the number of merged syms..
     * So now lets just run thru the merged syms and add in the # of dup got entries
     */
    numsyms = imextMax+1;                   /* account for entry 0 */
    for (i = 0; i < imextMax; i++) {
        pmext = xmext( i );
        if (pmext->isin_num_got_tables > 1 )
            numsyms += pmext->isin_num_got_tables - 1;
    }
    return( numsyms );
}







/*
 * Routine:     mgot_get_num_got_tables()
 *    Returns number of got tables that have been created.
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    number of gots!
 *   
 */
long    mgot_get_num_got_tables()
{
    return( num_got_tables );
}





/*
 * Routine:     mgot_write_got_entry()
 *    Write the current merged external to the got;
 *    We know at this point this entry IS a member of the current GOT
 *
 * Called from:
 *          mgot_write_gots_and_refd_globals()
 *
 * Inputs:
 *    None
 * Outputs:
 *    No side effects
 * Returns:
 *    Nothing
 *   
 */
STATIC void mgot_write_got_entry( pMEXTR pmext )
{    
    extern long lazy_eval;

    if (pmext->aext.asym.sc == scUndefined || 
        pmext->aext.asym.sc == scSUndefined) {
        /* for procs, mext_flags!=0 --> address was taken */
        if (pmext->aext.asym.st != stProc || 
            (pmext->mext_flags&MF_ADDRESS_TAKEN) != 0) {
            write_outbuf(got_outbuf, (char *)&(pmext->dsogotvalue), 
                         sizeof(unsigned long));
        }
        else {
            if (!lazy_eval && pmext->resolved != NOT_RESOLVED) {
                write_outbuf(got_outbuf, (char *)&(pmext->dsogotvalue), 
                             sizeof(unsigned long));
            }
            else {
                write_outbuf(got_outbuf,  
                             (char *)(&(pmext->aext.asym.value)),
                             sizeof(unsigned long));
            }
        }
    }
    else {                                  /* ->used is 0 */
        if (pmext->max_common != 0
            && (pmext->aext.asym.sc == scCommon || 
                pmext->aext.asym.sc == scSCommon))
            write_outbuf(got_outbuf, (char *)&(pmext->dsogotvalue), 
                         sizeof(unsigned long));
        else
            write_outbuf(got_outbuf, (char *)(&(pmext->aext.asym.value)), 
                         sizeof(unsigned long));
    }
}





/*
 * Routine:     mgot_write_gots_and_refd_globals()
 *    This routine writes the local and global GOTs
 *      and the ref'd globals in the dynsym
 *
 * Called from:
 *              write_dsoscn() from pass2()
 *
 * Inputs:
 *    None
 * Outputs:
 *    No side effects
 * Returns:
 *    Nothing
 *   
 */
extern int dynsymi;                         /* index into pdynsym */
void mgot_write_gots_and_refd_globals()
{
    long        tblnum;                     /* number of the got we are currently processing */
    pMEXTR      pmext;
    long        i;
    long        goti;
    int         process_this_one;   /* Flag if we should process current pmext */

    static long    local_gotno = 0;     /* Previous value; so we can keep a running total */

#ifdef DEBUG
    /* Default got size is odd, so only print got if even gotmax value..*/
    if (vflag && (max_got_count & 0x01)== 0)
        mgot_print();       /* If an odd/even got size print the got */

#endif
    for( tblnum = 0; tblnum < num_got_tables; tblnum++ ) {
        /* Note that locals include the 0th entry for rld already */
        write_outbuf( got_outbuf, got_tables[ tblnum ].locals,
                      got_tables[ tblnum ].num_locals * sizeof(unsigned long));

        /* LOCAL_GOTNO is the index to the first global for GOT[n]: */
        local_gotno += got_tables[ tblnum ].num_locals; 
        WRITE_DYNAMIC_VAL(DT_MIPS_LOCAL_GOTNO, local_gotno );
        local_gotno += got_tables[ tblnum ].num_globals;

        /* Now walk thru the merged external mapped array (ie, its already sorted for us),
         * and output the globals that are in the current got
         */

        WRITE_DYNAMIC_VAL(DT_MIPS_GOTSYM, dynsymi );
        for (i = 0; i < imextMax; i++) {
            pmext = map_xmext( i );
            LD_ASSERT( pmext->used == pmext->mgot_used ); /* just in case... */
            if (pmext->used == 0) continue;

            process_this_one = 0;
            for (goti = 0; goti < pmext->isin_num_got_tables; goti++) {
                if (tblnum == pmext->got_tbls[ goti ]) {
                    process_this_one = 1;
                    break;
                }
            }
            if (process_this_one == 0) continue; /* this sym isnt in the current got */
 
            mgot_write_got_entry( pmext );
            make_dynsym( pmext, tblnum, 2 ); /* We know we are always in ref'd globals at this point */
        }
    }
}


/*
 * Routine:     mgot_layout_dynsym_tbl()
 *    This routine sets the pmext->dynsym_idx field to be where the sym will be in the dynamic
 *      symbol table; This is used to fixup the stubs; We only set the idx to be the first
 *      occurance of the symbol in the dynamic symtbl.
 *
 * Called from:
 *          layout()
 *
 * Inputs:
 *    None
 * Outputs:
 *    modifies pmext->dynsym_idx
 * Returns:
 *    -
 *   
 */
void mgot_layout_dynsym_tbl()
{
    int         process_this_one;
    long        goti;
    long        tblnum;                     /* # of the got we are now processing */
    long        exp_dynsymi = 1;            /* expected index of this sym in dyamic symtbl */
    pMEXTR      pmext;
    int         imext;

    for (imext = 0; imext < imextMax; imext++) {
        pmext = map_xmext( imext );

        pmext->dynsym_idx = exp_dynsymi++;
#if 0
            DPRINTF_4("pmext[%d]: %s  @ dynsym[%d]\n",
                      imext, pmext->name, pmext->dynsym_idx );
#endif
        if (pmext->isin_num_got_tables == 0) continue; /* skip to ref'd globals */
        break;
    }

    /* Handle the case of having no ref'd globals; eg:
     *  char * fooie[] = { "foo"};
     */
    if ( (imext == imextMax) && (pmext->isin_num_got_tables == 0) ) return;

    pmext->dynsym_idx = 0;                  /* cause we bashed the last sym when we shouldnt have */
    exp_dynsymi--;
    for( tblnum = 0; tblnum < num_got_tables; tblnum++ ) {
        for (imext = 0; imext < imextMax; imext++) {
            pmext = map_xmext( imext );

            if (pmext->isin_num_got_tables == 0) continue;                   /* skip to ref'd globals */

            process_this_one = 0;
            for (goti = 0; goti < pmext->isin_num_got_tables; goti++) {
                if (tblnum == pmext->got_tbls[ goti ]) {
                    process_this_one = 1;
                    break;
                }
            }
            if (process_this_one == 0) continue; /* this sym isnt in the current got */
            if (pmext->dynsym_idx == 0) {
                pmext->dynsym_idx = exp_dynsymi;
            };
            exp_dynsymi++;

#if 0
            DPRINTF_4("pmext[%d]: %s  @ dynsym[%d]\n",
                      imext, pmext->name, pmext->dynsym_idx );
#endif

        }
    }
}


/*
 * Routine:     mgot_get_gpvalue_for_obj()
 *    This routine returns the value for offset_from_output_gp
 *
 * Called from:
 *          calc_gp_value() from dso_layout() from do_layout() from layout()
 *
 * Inputs:
 *    None
 * Outputs:
 *    No side effects
 * Returns:
 *    gpvalue
 *   
 */
long mgot_get_gpvalue_for_obj( pOBJ_FILER pobj )
{
    long    i;
    long    offset = 0;


    /* Now knowing the got this object starts with lets calc the offset
     * to return; It will be the offset to the locals for this got...
     */
    offset = mgot_got_offset( pobj->first_got_num );
    offset *= sizeof( Elf32_Addr );
    return( offset );
}





/*
 * Routine:     mgot_set_max_got_count()
 *    This routine is called when we want to explicitly set the max got count
 *      and max got size;
 *    Called from ld.c when we see the flag -Gotmax.
 * Inputs:
 *    value we want to set max_got_count to
 * Outputs:
 *    Sets max_got_count and max_got_size
 * Returns:
 *    -
 *   
 */
void mgot_set_max_got_count( int newvalue )
{
    if (newvalue < 0) {
        error(ER_ARG, "-Gotmax: value must be positive; not %d\n", newvalue );
    }
    if (newvalue > max_got_count) { /* value too large ! */
        error(ER_ARG, "-Gotmax: value must < %d\n", max_got_count );
    }
    max_got_count = newvalue;
    max_got_size = max_got_count * sizeof( Elf32_Addr ); /* compute this ahead of time */
}





/*
 * Routine:     mgot_set_got_threshold()
 *    This routine is called when we want to explicitly set the threshold
 *      value used in creating GOTs;
 *    When we are adding a new object to a GOT if the current got size
 *      exceeds max_got_threshold, then we put the obj into the
 *      next GOT; (see mgot_check())
 *    Called from ld.c when we see the flag -Gotthresh
 * Inputs:
 *    value we want to set max_got_threshold to
 * Outputs:
 *    Sets the global;
 * Returns:
 *    -
 *   
 */
void mgot_set_got_threshold( int newvalue )
{
    if (newvalue < 0) {
        error(ER_ARG, "-Gotthresh: value must be positive; not %d\n", newvalue );
    }
    if (newvalue > max_got_count) { /* value too large ! */
        error(ER_ARG, "-Gotthresh: value must < %d\n", max_got_count );
    }
    max_got_threshold = newvalue;
}







/*
 * Routine:     mgot_alloc_p1_local_got()
 *    This routine allocates space for a temporary pass1 duplex local got
 *      (a local got 2 times the required size so it holds immediates and section
 *      info; aka was cur_obj->pgot );
 *
 * Called from:
 *              alloc_got() from merge() during pass1().
 * Inputs:
 *    see below.
 * Outputs:
 *    Records the pseudo local_got info from pass1.
 * Returns:
 *    Nothing
 *   
 */
void mgot_alloc_p1_local_got(
     pOBJ_FILER     pobjfile,               /* the object structure we are processing */
     LOCAL_GOT      * local_got,            /* the pseudo local_got info we save from pass1 */
     unsigned long  ngot )                  /* number of entries in the above struct */
{
    unsigned long   i;

    if (ngot == 0) return;                  /* If no locals then no work to do.... */

    i = got_tables[ cur_got ].num_p1_local_gots++;

    got_tables[ cur_got ].p1_gots = (void *)realloc(
                                      got_tables[ cur_got ].p1_gots,
                                      MAX( (i+100) * sizeof( struct st_p1_gots ), 8192) );

    if (got_tables[ cur_got ].p1_gots  == (void *)0) {
        error( ER_FATAL, "Out of memory allocating p1 local gots; needed %ld entries\n", i+1 );
    }

    got_tables[ cur_got ].p1_gots[ i ].p1_got = local_got;          /* save the duplexed table for layout() */
    got_tables[ cur_got ].p1_gots[ i ].num_p1_locals = ngot;
    got_tables[ cur_got ].p1_gots[ i ].cur_obj = pobjfile;

    if (got_tables[ cur_got ].num_locals >  max_got_count) {            /* num_locals counts loader entry */
        error( ER_ERROR, "Too many locals in object file; Found %d but max is %d\n",
              ngot, max_got_count );        /* Show user the object file we are processing */
        error(ER_FATAL, "");                /* Lets stop now; er_error would keep going; */
    }
}







/*
 * Routine:     mgot_fix_local_gots()
 *  This routine was the old dynutil.c fix_got;
 *  What we are doing here is to resolve all the values for the
 *      local got.
 *  We are using the fact that the local got in pass1 had 2 parts,
 *      The first half of the local GOT has the 
 *      immediates. The 2nd half has the section indices for the
 *      corresponding immediates. Thus in this routine we can walk
 *      thru pobj->pgot[x] and pobj->pgot[x + pobj->ngot ]  
 *
 * Called from:
 *              layout()
 * Inputs:
 *    -
 * Outputs:
 *    Fixes up all the local gots.
 * Returns:
 *    Nothing
 *   
 */
void mgot_fix_local_gots()
{
    pOBJ_FILER      pobj;
    pLOCAL_GOT      pgot;
    int             i;
    pSECTION        psec;
    unsigned long   tmp;
    long            ngot;                   /* number of local got entries */
    unsigned long   p1;
    unsigned long   x;
    unsigned long   lidx;
    unsigned long   expected_num_locals;


    for (cur_got = 0; cur_got < num_got_tables; cur_got++) { /* Walk thru all the GOTs */
        ngot = got_tables[ cur_got ].num_locals;


        /* Now lets just do a sanity check on our gots...
         * We want to be sure that the number of locals we were told about
         * during pass1 (ie merge()) agrees with the number of local values
         * we see here now..
         */
        expected_num_locals = 1;            /* Loader is a freebie */
        for (p1 = 0; p1 < got_tables[ cur_got ].num_p1_local_gots; p1++) { /* Walk thru all pass1 "local_gots" */
            expected_num_locals += got_tables[ cur_got ].p1_gots[ p1 ].num_p1_locals;
        }
#ifdef DEBUG
        if ( got_tables[ cur_got ].num_locals != expected_num_locals ) {
            printf("****** mgot: got[%ld].num_locals (%ld) != expected_num(%ld)********* \n",
                   cur_got,
                   got_tables[ cur_got ].num_locals,
                   expected_num_locals );
        }
#endif
        LD_ASSERT( got_tables[ cur_got ].num_locals == expected_num_locals );
        got_tables[ cur_got ].num_locals = 1; /* reset count; we will inc it agn below. */
        
        for (p1 = 0; p1 < got_tables[ cur_got ].num_p1_local_gots; p1++) { /* Walk thru all pass1 "local_gots" */
            pobj = got_tables[ cur_got ].p1_gots[ p1 ].cur_obj;
            ngot = got_tables[ cur_got ].p1_gots[ p1 ].num_p1_locals;

            /* Allocate enough room for the additional locals: */

            x = got_tables[ cur_got ].num_locals + ngot;
            got_tables[ cur_got ].locals = (void *)realloc(
                                                           got_tables[ cur_got ].locals,
                                                           MAX(x * sizeof(unsigned long), 8192) );

            if (got_tables[ cur_got ].locals == (void *)0) {
                error(ER_FATAL, "Out of memory allocating GOT locals\n");
            }
            
            pgot = got_tables[ cur_got ].p1_gots[ p1 ].p1_got;

#ifdef DEBUG_FIX_LOCALS
            DPRINTF_5("mgot_fix_local_gots: cur_got=%ld, %ld partial gots, processing partial %ld with %ld locals\n",
                      cur_got, got_tables[ cur_got ].num_p1_local_gots, p1, ngot );
#endif

            for (i = 0; i < ngot; i++, pgot++) {
                switch((pgot + ngot)->immed) {
                  case R_SN_TEXT:    
                    psec = (pSECTION)(pobj->section_list[SEG_TEXT]->data);
                    tmp = map_address(pgot->immed, psec, FALSE) - 
                        psec->cum_extra;
#if defined(__alpha) || defined(__mips64)
                    tmp -= psec->extra;     /* Already in s_vaddr */
#endif
                    pgot->immed = t_base + tmp +
                        pobj->t_start - pobj->t_scnhdr->s_vaddr;
                    break;
                  case R_SN_INIT:    
                    psec = (pSECTION)(pobj->section_list[SEG_INIT]->data);
                    tmp = map_address(pgot->immed, psec, FALSE) - 
                        psec->cum_extra;
                    pgot->immed = i_base + tmp +
                        pobj->i_start - pobj->i_scnhdr->s_vaddr;
                    break;
                  case R_SN_FINI:    
                    psec = (pSECTION)(pobj->section_list[SEG_FINI]->data);
                    tmp = map_address(pgot->immed, psec, FALSE) - 
                        psec->cum_extra;
                    pgot->immed = f_base + tmp +
                        pobj->f_start - pobj->f_scnhdr->s_vaddr;
                    break;
                  case R_SN_RDATA:    
                    pgot->immed += rd_base + 
                        pobj->rd_start - pobj->rd_scnhdr->s_vaddr;
                    break;
                  case R_SN_DATA:    
                    pgot->immed += d_base + 
                        pobj->d_start - pobj->d_scnhdr->s_vaddr;
                    break;
                  case R_SN_SDATA:    
                    pgot->immed += sd_base + 
                        pobj->sd_start - pobj->sd_scnhdr->s_vaddr;
                    break;
                  case R_SN_SBSS:    
                    pgot->immed += sb_base +
                        pobj->sb_start - pobj->sb_scnhdr->s_vaddr;
                    break;
                  case R_SN_BSS:    
                    pgot->immed += b_base + 
                        pobj->b_start - pobj->b_scnhdr->s_vaddr;
                    break;
                  case R_SN_LIT8:
                    pgot->immed += l8_base +
                        pobj->l8_start - pobj->l8_scnhdr->s_vaddr;
                    break;
                  case R_SN_LIT4:
                    pgot->immed += l4_base +
                        pobj->l4_start - pobj->l4_scnhdr->s_vaddr;
                    break;
                  case R_SN_LITA:
                    assert(0);              /* Should never reach here */    
                    pgot->immed += la_base + 
                        pobj->la_start - pobj->la_scnhdr->s_vaddr;
                    break;
                }
                lidx = got_tables[ cur_got ].num_locals++;
                got_tables[ cur_got ].locals[ lidx ] = pgot->immed;
            }                               /* for (i = 0; i < ngot...  */
            /* Now we have processed all the locals in that table. Lets free up that memory */
            free( got_tables[ cur_got ].p1_gots[ p1 ].p1_got );
            got_tables[ cur_got ].p1_gots[ p1 ].p1_got = (void *)0;
            got_tables[ cur_got ].p1_gots[ p1 ].num_p1_locals = 0;
        }                                   /* for (p1 = 0; p1 < got_tables[ cur_got ].num_p1_local... */
        /* Now we have processed all the pass1 local_got's to be a real got;
         * Free up the memory from the pass1 localgots:
         */
        free( got_tables[ cur_got ].p1_gots );
        got_tables[ cur_got ].p1_gots = (void *)0;
        got_tables[ cur_got ].num_p1_local_gots = 0;
    }                                       /* for (cur_got = 0; cur_got < num_got_tables ... */
}




/*
 * Routine:     mgot_pimmed_offset()
 *    Returns the offset into the pimmed table of cur_obj, for the current got.
 * 
 * Called from:
 *              relocate()
 * Inputs:
 *    -
 * Outputs:
 *    -
 * Returns:
 *    The offset
 *   
 */
int mgot_pimmed_offset()
{
    return( got_tables[ cur_got ].pimmed_offset );
}





/*
 * Routine:     mgot_set_pimmed_offset()
 *    Sets the offset into the cur_obj->pimmed table for the current got;
 *     If the offset is zero we leave it. Else we must back off by one.
 *
 * Called from:
 *              alloc_got()
 * Inputs:
 *    The offset
 *
 * Outputs:
 *    -
 * Returns:
 *    -
 *   
 */
void mgot_set_pimmed_offset( int offset )
{
    if (cur_got == -1) return;              /* If no got allocated yet.
                                             * This happens in processing a mentor .a file where we try to
                                             * set the immed offset but no got is going to be gen'd
                                             */
    if (offset != 0) offset--;              /* if offset is zero is ok, otherwise is off by one. */

    if (got_tables[ cur_got ].pimmed_offset != 0) { 
        /* Uh oh... kalxon alert; We should not be here! */
           error(ER_FATAL, "mgot_set_pimmed_offset: Attempt to bash offset (%d) to %d for got %ld\n", 
           got_tables[ cur_got ].pimmed_offset, offset, cur_got);
    } else {
        got_tables[ cur_got ].pimmed_offset = offset;
    }
}





/*
 * Routine:     ldstats()
 *    Outputs getrusage() stats on the current process.
 *
 * Called from:
 *              lots of places
 * Inputs:
 *    -
 *
 * Outputs:
 *    getrusage() data.
 *              REGRETTABLY getrusage() doesnt appear to work
 *              correctly on  alpha; the utime (and problaby stime)
 *              fields get reset at random, and the min* max*
 *              and nswap fields all come back with zeros all the time..
 *
 * Returns:
 *    -
 *   
 */
#include <sys/time.h>
#include <sys/resource.h>
extern void * sbrk(ssize_t);
static short ldstat_first_time = 1;
void ldstats()
{
    static char *           base_addr;
    static char *           last_addr;
    static struct rusage    base_ru;
    static float            base_utime, base_stime;
    
    char *          sbrk_addr;
    struct rusage   rstruct;
    float           utime, stime, cur;
    float           brkamt;
    int             err;

    if (ldstat_first_time == 1) {
        ldstat_first_time = 0;
        base_addr = (char *)sbrk((ssize_t)0 );
        last_addr = base_addr;
        getrusage( RUSAGE_SELF, &base_ru );
        base_utime = base_ru.ru_utime.tv_sec + base_ru.ru_utime.tv_usec * 1e-6;
        base_stime = base_ru.ru_stime.tv_sec + base_ru.ru_stime.tv_usec * 1e-6;
        return;
    }
    sbrk_addr = sbrk( (ssize_t)0 );
    brkamt = (float)(sbrk_addr - base_addr)/(1024. * 1024. );
    printf("ldstats: sbrk amount %ld., tot sbrk: %.2fM",   sbrk_addr - last_addr,  brkamt);
    last_addr = sbrk_addr;
    
    err = getrusage( RUSAGE_SELF, &rstruct);
    if (err != 0) {
        printf("**** GETRUSAGE returns nonzero\n");
        exit(999);
    }
    /* computed this way so I could validate getrusage() was busted versus a compiler math bug...*/
    cur = rstruct.ru_utime.tv_sec + (rstruct.ru_utime.tv_usec * 1e-6);
    utime = cur - base_utime;

    cur = rstruct.ru_stime.tv_sec + rstruct.ru_stime.tv_usec * 1e-6 ;
    stime = cur -  base_stime;
#ifdef BUSTED_RUSAGE
    printf("  utime=%.2f, stime=%.2f, maxrss:%ld, minflt=%ld, majflt=%ld, nswap=%ld, inblock=%ld, outblock=%ld\n",
           utime, stime,
           rstruct.ru_maxrss - base_ru.ru_maxrss,
           rstruct.ru_minflt - base_ru.ru_minflt,
           rstruct.ru_majflt - base_ru.ru_majflt,
           rstruct.ru_nswap  - base_ru.ru_nswap,
           rstruct.ru_inblock - base_ru.ru_inblock,
           rstruct.ru_oublock - base_ru.ru_oublock );
#else
    printf("  utime=%.2f, stime=%.2f\n",
           utime, stime);
#endif
}
