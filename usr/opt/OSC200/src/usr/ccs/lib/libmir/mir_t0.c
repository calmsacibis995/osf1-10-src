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
static char *rcsid = "@(#)$RCSfile: mir_t0.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/07 19:48:23 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991, 1992, 1993
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent Management Information Repository
 *
 * Module MIR_T0.C
 *      Contains Tier 0 Interface Functions required by a user of
 *      the MIR to obtain information from the MIR.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   November 1990
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines have need of network management
 *       architecture information in order to properly process incoming
 *       management request messages.  This information must reside in a
 *       Management Information Repository, accessible by ISO Object ID.
 *
 *    Purpose:
 *       This module contains the functions required by the Common Agent
 *       to query the MIR for network management information.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *      V0.0    December 1990           D. D. Burns
 *      V1.0    February 1991           1st use w/Compiler (DDB)
 *      V1.1    March 1991              Initial Release (DDB)
 *      V1.2    June 1991               Add mir_copy_mandle() function
 *                                       to meet OSI PE requirement
 *      V1.7    October 1991            Remove "_LONGEST" as a return code
 *                                       for mir_oid_to_mandle(), implement
 *                                       GET_EXACT_ROLL opcode for SNMP
 *                                       GET-NEXT mir_oid_to_mandle() reqs.
 *      V1.8    December 1991           Add mutex protection for mandle/class
 *                                        lists for threaded environment and
 *                                        mir_debug_statistics().
 *      V1.9    January 1992            Provide mir_t0_init() call, removes
 *                                        the need for a once_block for threads
 *      V1.90   June 1992               Pre-V2.00 release, incorporates
 *                                        calling sequence changes for OID
 *                                        reporting but w/o the underlying
 *                                        support for it, also
 *                                        mir_free_mv_string().
 *      V1.95   July 1992               Support for Binary File Format Version
 *                                        2 and multiple OID support, plus
 *                                        new mir_get_rel_mandles() function
 *      V1.96   August 1992             Fix reporting of OID SMI to work for
 *                                        all cases, not just MS0_FIND_EXACT
 *      V1.97   August 1992             Add support for OID-SMI disambiguation
 *                                        in mir_mandle_to_oid().
 *      V1.98   October 1992            Internationalization
 *      V2.00   November 1992           Support for software-paging of
 *                                        Non-Terminals, removed function
 *                                        stub for "mir_free_mv_string()".
 *      V2.01   Feb 1993                Avoid bugcheck in VAX C macro-size
 *                                        limitation (J. Plouffe).
 */                     

/*
Module Overview:

This module contains all the basic functions available to a user of the MIR.
These basic functions form the "Tier 0" interface to the MIR. 

The "Tier 1" functions for the MIR are formed from calls made t0 these Tier 0
functions.  Tier 1 function sources are found in another source module.

MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
mir_t0_init          Performs MIR Database File loading and other
                     initialization chores.

mir_get_rel_mandles  Gets a list of mandles "by name" for a Tier 0 user.

mir_oid_to_mandle    Converts a supplied Object ID to a MIR Handle (mandle)

mir_mandle_to_oid    Converts a mandle into the Object ID represented by the
                        mandle

mir_search_rel_table Searches the Relationship Table of a MIR Non-Terminal
                     object given a mandle to the object and a mandle to a
                     relationship object.

mir_copy_mandle      Creates a copy of an existing mandle (in the same
                     mandleclass as the original)

mir_free_mandle      Free a previously created mandle

mir_free_mandle_class
                     Free all previously created mandles in a given class.

mir_reset_search     Resets the starting search point in a Non-Terminal's
                     relationship table "to the top" for a subsequent call
                     to mir_search_rel_table().  (Implemented as a macro for
                     use by Tier 1 functions in "mir.h").

mir_error            Returns string interpretation of an Tier 0/1 Interface
                     error status code (for debugging)

mir_get_mas          Returns address of MIR Address Space (for debugging ONLY)

mir_debug_statistics Returns in-use statistics for mandles and mandleclasses
                     to the caller.


INTERNAL FUNCTIONS:
NT_page_in       Makes available in-memory the page containing a specified
                 Non-Terminal

set_mandle       Sets up a passed pointer to a mandle for proper value return

setup_next       Sets up ret_mandle position to return NEXT object in index

setup_prev       Sets up ret_mandle position to return PREVIOUS object in index

ret_mandle       Sets up passed ptr to mandle and returns proper object from
                 MIR index

get_oid_index    Employed by mir_mandle_to_oid() to recursively walk the index
                 to discover the full OID for an object in the index.

                 NOTE: The source for this function is #include-d into this
                       module from file "mir_goi.c".  This allows us to
                       distribute ONE object module that corresponds to
                       the Tier 0 functions (rather than two).  This function
                       source is shared with "mir_symdump.cinc" which also
                       includes it.
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>


#define MIR_T0          /* Get special definitions for MIR Tier 0 */
#include "mir.h"

/*
| MTHREADS
|
| While most of the code needed to support Multiple Threads is present, some
| known changes must be made before it can be made operational:
| 
|       1) The initialization of the mutex "mandle_lists_m" that protects
|          the module-wide cells listed below must itself be protected by
|          a "once" block.  The initialization is done by code in
|          "mir_t0_init()".
|
|       2) Search the code for symbol MTHREADS and read the comments for
|          mention of other changes needed (all changes needed may not all
|          be documented... hey!  Its not just a job, its an adventure!)
|
|       3) Everything must be tested!
|
*/
#ifdef MTHREADS
#include <pthread.h>
#endif


/*
| Debugging Aid
|
| To track the operation of the paging mechanism, define the following symbol.
|
| Paging-mechanism trace dumps are fprint-ed to the file specified by
| the associated file symbol below.  There is no logic to open the file,
| the tracing dumps presume the file is already open.
*/
/* #define PAGE_TRACE 1 */
/* #define PT_FILE stdout */


/*  Module-Wide Data Areas */

/*
| === The Data Structures used when a 'paging' OR 'non-paging' Tier 0 open is
| === performed follow. . . 
*/

/*
|=============================================================================
| Pointer to MIR Address Space ("mas") containing the MIR database loaded
| from disk.
|
| For non-paging open, its 'The Works'. . . everything from
| the MIR database file.
|
| For the paging open, its everything *up to* (but not inluding) the
| Non-Terminal partitions.
|=============================================================================
*/
static unsigned int *mas=NULL;   /* --> Array of Address-Space "Cells" */


/*
|=============================================================================
| Number of arcs in the longest OID in the index
|
| (Well, it's *at least* the number of longest OID.  The compiler doesn't
|  update this number when deletes (using "-r" on MIRC command line) are
|  performed, so this number may be too big by a tad, but it'll never be
|  too small).
|=============================================================================
*/
static int maxoid_arcs;


/*
|=============================================================================
| Size of the MIR Relationship Masa-to-Synonym array (in entries, see below).
|=============================================================================
*/
static int synonym_count;


/*
|=============================================================================
| MIR Relationship masa-to-synonym Array
|
|  This dynamically allocated array (by mir_t0_init()) is used by
|  "mir_search_rel_table()" to convert the masa of a Relationship Object
|  to its "synonym".
|
|  The synonym is what actually appears in the Relationship Table of an object
|  being searched.  The synonym is simply the ordinal position of the MIR
|  Relationship Object in the MIR Relationship Object partition.  The first
|  time "mir_search_rel_table()" receives a mandle for an object that should
|  be a MIR Relationship Object, it translates the masa to the synonym and
|  stores the synonym in the mandle for future use.
|
|  The array below contains the masa's of each MIR Relationship Object.  The
|  index of each entry (+1) is the synonym of that entry (synonyms start at
|  "1"). (These correspond to what the compiler considers "External" synonyms).
|=============================================================================
*/
static int *masa_to_synonym=NULL;


/*
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
*/

/*
|=============================================================================
| SET_MAS_PAGE
|
| This macro converts a 'real' MIR Address-Space Address ("masa") of a
| Non-Terminal MIR Object into a 'virtual' masa + a pointer to a buffer
| guaranteed to contain the Non-Terminal 'at' the 'virtual' masa.  (In other
| words, the virtual masa is an index into the returned buffer which contains
| the Non-Terminal).
|
| IN:
|       "m" - pointer to a Mandle (containing the ('real') masa of the
|       Non-Terminal MIR Object to be made available).
|
| OUT:
|       New 'Virtual' masa value "v".
|
|       Implicitly returned into "Mas" is a pointer to type "masa".  This
|       is (essentially) the buffer containing the Non-Terminal.
|       "Mas[v]" is the first cell of the Non-Terminal upon return, whether
|       paged-in or not.
|=============================================================================
*/
#ifdef PAGE_TRACE
#define SMP_TRACE(va, mn)                                                     \
    fprintf(PT_FILE,                                                          \
            "\nPT: Referencing NT at virtual masa %d, page %x, refs = %d\n",  \
            va, (mn)->m.m.page, (mn)->m.m.page->ref_count);
#else
#define SMP_TRACE(va, mn)
#endif

#define SET_MAS_PAGE( mndl, v )                                               \
/* if we're not doing paging */                                               \
if (pbuffer_maxcount == 0) {                                                  \
    Mas = mas;          /* Set 'local' page buffer to be same as full "mas" */\
    v = (mndl)->m.m.ex_add; /* Rtn the full external addr as virtual masa   */\
    }                                                                         \
else {  /* We're paging. . . get the local pointer set correctly */           \
    mir_status  st;             /* Status from NT_page_in() call */           \
                                                                              \
    /*                                                                        \
    | If no page buffer is already setup for this mandle                      \
    */                                                                        \
    if ((mndl)->m.m.page == NULL) {                                           \
        if ((st = NT_page_in(mndl)) != MS_SUCCESS) /* Get Page Buffer setup */\
            return(st);                                                       \
        }                                                                     \
                                                                              \
    v = (mndl)->m.m.vir_add;       /* Return virtual masa in . . . */         \
    Mas = (mndl)->m.m.page->page;  /* the associated page buffer   */         \
    SMP_TRACE(v, mndl);                                                       \
    }

        
/*
|=============================================================================
| RELEASE_ANY_PAGE_BUFFER
|
| This macro releases any page buffer associated with the mandle (by pointer)
| specified as the argument.
|
| As can be seen, the macro references module-local variables that may be
| manipulated only while the LOCK_MANDLES() macro is in force.
|
|=============================================================================
*/
#ifdef PAGE_TRACE
#define RAPB_TRACE(mn)                                                        \
    fprintf(PT_FILE,                                                          \
            "\nPT: Releasing Page Buffer (ref_count=0) w/starting masa %d\n", \
            (mn)->m.m.page->first);
#else
#define RAPB_TRACE(mn)
#endif

/* If (we're open-paging and a page-buf is associated)
 *   Decrement page reference count
 *   If (reference count is zero)
 *     Hook the page buffer into the bottom of free list
 *     Hook to-be-newly-freed to bottom of any existing
 *     Make new bottom point back at any previous entry
 *     Make list bottom prt point to new list bottom entry
 *     If it's the only one on the list, adjust the top ptr
 *     Show no-next on the Free List
 *     Increment the free list buffer count
 *   Show no page buffer
 */
#define RELEASE_ANY_PAGE_BUFFER(mndl)                                         \
if (pbuffer_maxcount != 0 && (mndl)->m.m.page != NULL) {                      \
                                                                              \
    (mndl)->m.m.page->ref_count -= 1;                                         \
                                                                              \
    if ((mndl)->m.m.page->ref_count == 0) {                                   \
                                                                              \
        RAPB_TRACE(mndl);                                                     \
                                                                              \
        if (bot_free != NULL)                                                 \
            bot_free->next = (mndl)->m.m.page;                                \
                                                                              \
        (mndl)->m.m.page->prev = bot_free;                                    \
                                                                              \
        bot_free = (mndl)->m.m.page;                                          \
                                                                              \
        if (top_free == NULL)                                                 \
            top_free = (mndl)->m.m.page;                                      \
                                                                              \
        (mndl)->m.m.page->next = NULL;                                        \
                                                                              \
        pbuffer_freecount += 1;                                               \
        }                                                                     \
                                                                              \
    (mndl)->m.m.page = NULL;                                                  \
    (mndl)->m.m.vir_add = 0;                                                  \
    }


/*
|=============================================================================
| "Non-Terminal Maximum Page-buffer Count"
|
| When Tier 0 is called to open "paged", this is the number of a page buffers
| allocated to receive a slugs of Non-Terminals when some need to be read in.
|
| The count is the total number of buffers that would be required to hold all
| the MIR Non-Terminals (if every buffer started exactly where the previous
| buffer ended) PLUS 50% of that number.  This 50% slack is provided because
| it is quite likely that the buffers will NOT be allocated and used so that
| there is no overlap.
|
| A non-zero value in this cell implies to the rest of the code in this module
| that the last call to "mir_t0_init()" performed a 'paged' open.  If zero,
| this means that it was a non-paged open.
|=============================================================================
*/
static int pbuffer_maxcount = 0;


/*
|=============================================================================
| "Non-Terminal Page-buffer Size"
|
| When Tier 0 is called to open "paged", this is the size of a page buffer
| (in bytes) that is allocated to receive a slug of Non-Terminals when some
| need to be read in.
|
| This buffer must be large enough to hold the largest Non-Terminal.
|
| At mir_t0_init() time, this cell is loaded with the size in bytes of the
| largest Non-Terminal in the file being opened (a value recorded in the
| binary MIR database file preamble by the compiler) OR some minimum that
| resembles a disk sector size.
|=============================================================================
*/
static unsigned int pbuffer_size = 0;


/*
|=============================================================================
| "Magic MASA Fseek"
|
| When Tier 0 is called to open "paged", the code in other sections of this
| module expect to be able to take this "magic-fseek" number:
|
|   * add it to (sizeof(int)*(the masa they seek to read-in next)),
|   * do an "fseek" using this sum, and then
|   * do an fread() to obtain a slug of MIR Address Space starting at a
|     specified masa.
|=============================================================================
*/
static long magic_fseek = 0;


/*
|=============================================================================
|=============================================================================
| In a threaded enviroment (ie "MTHREADS" is defined), the following list
| headers (and by implication the lists themselves) and the following cells
| are protected by the associated mutex defined below.
|
| Before adding or deleting entries from any of these lists or cells, the
| mutex must be acquired using macros "LOCK_MANDLES()" and "UNLOCK_MANDLES()"
| defined in "mir.h".  (By locking the mandles, we implicitly acquire access
| to any underlying data-structures (mentioned below) used for page-buffers
| for the mandles if we're open for 'paging' of the Non-Terminals).
| 
|=============================================================================
*/
#ifdef MTHREADS
pthread_mutex_t mandle_lists_m;
#endif

/*
| These two lists are used to manage mandles, and are consequently in use
| regardless of the kind of 'open' performed on the MIR database file
| (that is, regardless of paged vs non-paged open).
*/

/* Pointer to MIR Mandle Class List */
static mandle_class *mcl=NULL;      /* --> Linked list of structures */

/* Pointer to the Free Mandle List (used mandles now available for re-use) */
static mandle *fml=NULL;


/*
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
| === Data Structures used when a 'paging' Tier 0 open active. . .  ===
*/

/*
|=============================================================================
| "MIR database file"
|
| This file pointer points to the open binary MIR database file when a
| 'paged' Tier 0 open has been performed.
|
| This cell may only be modified/referenced when the mutex above is held.
|=============================================================================
*/
FILE    *mir_file=NULL;


/*
|=============================================================================
| "Non-Terminal Current Page-buffer Count"
|
| When Tier 0 is called to open "paged", this is the number of a page buffers
| currently allocated holding slugs of read-in Non-Terminals (with reference
| counts of zero or more.... ie including the buffers on the Free List).
|
| This cell may only be modified/referenced when the mutex above is held.
|=============================================================================
*/
static int pbuffer_curcount = 0;


/*
|=============================================================================
| "masa vector"
|
| When Tier 0 is called to open "paged", a block of heap is allocated to
| contain the "masa vector": an array of "masa"s (ordered in ascending value).
|
| Each entry is the "masa" of the first MIR object in the buffer whose
| corresponding "page_buf" heap address is in the same slot in "buffer vector".
|
| The size of this array is the total number of buffers that
| would be required to hold all the MIR Non-Terminals (if every buffer
| started exactly where the previous buffer ended) *PLUS 50%* of that number.
|
| This count is stored in "pbuffer_maxcount".  The number of active entries
| in this array is given by "pbuffer_curcount".
|
| This cell and the corresponding heap-array are not used when the Tier 0
| 'open' is Non-paged.
|
| This array points to page buffers that are referenced (ref_count > 0) and to
| page buffers that are unreferenced (ref_count == 0) (and are hence on the
| Free List).
|
| This array may only be modified/referenced when the mutex above is held.
|=============================================================================
*/
static unsigned int *masa_vector=NULL;


/*
|=============================================================================
| "buffer vector"
|
| When Tier 0 is called to open "paged", a block of heap is allocated to
| contain the "buffer vector": an array of pointers to "page_buf" structures
|  in heap.
|
| Each entry in the "buffer vector" contains a pointer to the page buffer that
| starts at the MIR address space address that is contained in the
| corresponding entry in "masa vector".  See above.
|
| This cell and the corresponding heap-array are not used when the Tier 0
| initialization is Non-paged.
|
| This array may only be modified/referenced when the mutex above is held.
|=============================================================================
*/
static page_buf **buffer_vector=NULL;


/*
|=============================================================================
| "Free List"
|
| The free list contains pagebufs whose reference count is zero (see
| discussion above at definition of "pagebuf").
|
| A pagebuf can be removed from the Free List for two reasons:
|
|       1) A search of "masa_vector" reveals that an entry on it describes
|          a pagebuf containing a MIR Non-Terminal that needs to be referenced.
|          If the entry points to a pagebuf on the Free List, then that means
|          the pagebuf's ref_count must be incremented to one, meaning it is
|          no longer free (and should be unlinked from the Free List).
|
|       2) A search of "masa_vector" revals NO entries (either 'active' or
|          'inactive') that contain the desired MIR Non-Terminal.  In this
|          case, provided the Free List exceeds FREE_LIST_DEPTH, the oldest
|          ('topmost') entry on the list is removed from the Free List and
|          new information is read into the buffer (the buffer is re-used).
|          The entry in "masa_vector" (corresponding to the removed pagebuf)
|          must also be removed.
|
| The value of cell "fl_depth" determines the minimum size of the Free List
| when viewed as a cache.  In other words, buffers are not re-used from the
| Free List as long as the Free List has not yet reached the number of entries
| specified by this symbol (new buffers are simply allocated from the heap).
|
| Page Buffers on the Free List are still referred to by "masa_vector" and
| "buffer_vector", so they participate in searches made by code trying to
| find an existing-loaded buffer with the right stuff in it.  In this way,
| the Free List is essentially a cache of available (but loaded) buffers.
|
| NOTE:  If you play with the value of fl_depth in an attempt to 'tune'
| the Tier 0 paging system, be sure to always run your tests with a binary
| MIR database file that truly approximates the 'real' version of this file.
| This is because the *size* of the pagebuf buffers is determined by the
| *largest* Non-Terminal (above a certain minimum) that the compiler produced
| **in the given MIR file**! Obviously if you tune fl_depth using a 'fake' MIR
| binary file that uses the minimum 'pea-sized' buffers, the paging system
| won't work the same way with a 'real' MIR binary file that uses
| 'watermelon-sized' buffers!  The value of "fl_depth" can be set via an
| environment variable named in the call to "mir_t0_init()".  It is re-read
| each time mir_t0_init() is invoked.
|
| When a pagebuf is put on the free list, it is put on 'at the bottom', and
| "next" is null.  When another pagebuf is put on the free list, it's "next"
| becomes null and the 'old-end' entry's "next" points to the new "end" entry.
| Removals from the free list (when re-using a buffer) are done 'at the top'
| (first in, last out) in order to give us a chance of re-using the contents
| of each buffer.  The 'top-most' entry on the free list has a "prev" cell
| value of NULL.
|
| These cells may only be modified/referenced when the mutex above is held.
|==============================================================================
*/
#define FREE_LIST_DEPTH 25
static int fl_depth = FREE_LIST_DEPTH;
static page_buf *top_free = NULL; /* Remove from list here (to re-use buffer)*/
static page_buf *bot_free = NULL; /* Insert here on list (to cache buffer)   */


/*
|=============================================================================
| "Free List Page-buffer Count"
|
| When Tier 0 is called to open "paged", this is the number of a page buffers
| currently on the Free List (defined above).
|
| This cell may only be modified/referenced when the mutex above is held.
|=============================================================================
*/
static int pbuffer_freecount = 0;


/*
|=======================End of Mutex-Protected Stuff  =========================
|==============================================================================
*/

/*
| Forward Reference definition
*/

/*
| NT_page_in - Non-Terminal Page-In
|
| NOTE: This function invoked via macro "SET_MAS_PAGE()"
*/
static mir_status
NT_page_in PROTOTYPE((
mandle          *     /* Mandle of Non-Terminal to be paged in    */
));



/* mir_t0_init - Init Tier 0 by Loading the MIR Database File into memory */
/* mir_t0_init - Init Tier 0 by Loading the MIR Database File into memory */
/* mir_t0_init - Init Tier 0 by Loading the MIR Database File into memory */

mir_status
mir_t0_init(
            /* MIR Database File parameters. . . */
            /* IN       IN         OUT           */
            datafile, ev_file, actual_dfile,

            /* Non-Terminal Paging parameters . . . . . . . . . */
            /*IN     IN        IN         OUT          IN/OUT   */
            page, ev_open, ev_fl_depth, max_buffs, free_list_size,

            /* Database File preamble */
            /*  OUT                   */
            preamble_list
           )

char    *datafile;      /* If non-NULL, string containing filename to use   */
char    *ev_file;       /* If non-NULL, -> env variable name "File Name"    */
char    **actual_dfile; /* If non-NULL, on successfile file-open, name-used */
                        /*              is returned to caller.              */
BOOL    page;           /* TRUE: Page NTs if "ev_open" (see also "ev_open") */
char    *ev_open;       /* If non-NULL, -> env variable name for "pageOpen" */
char    *ev_fl_depth;   /* If non-NULL, -> env variable name for "fl_depth" */ 
int     *max_buffs;     /* If non-NULL, -> where to rtn max # of page buffs */
int     *free_list_size;/* If non-NULL, sets/returns free list size         */

int     **preamble_list;/* Set on return to point to preamble array         */

/*
INPUTS:

    "datafile", if non-NULL, is presumed to point to a string containing
    the full filename to use for the MIR Database file.  If this argument is
    NULL, "mir.dat" in the current directory is assumed, pending analysis of
    the "ev_file" argument.

    "ev_file", if non-NULL, points to a string which is the name of an
    environment variable, which if defined, its value is used instead of
    one specified by "datafile" (or the default "mir.dat" if "datafile" was
    NULL).

    If a successful open occurs on a MIR database file and "actual_dfile" is
    non-NULL, then a pointer to the actual filename used in the open is
    returned to "actual_dfile".
        NOTE:  The actual pointer returned may point to a variety of places
    including (potentially) the same value passed in as "datafile".
    If the value of "datafile" is returned, then the caller is responsible for
    making reference to "*actual_dfile" before the value passed as "datafile"
    becomes invalid (if it does)!
    
    The value of "page" in conjunction with any specified for "ev_open"
    determines whether or not Non-Terminals will be 'paged' into memory
    on-demand (by software paging logic in this module) or whether they
    will not be software-paged (ie, they are all simply copied into the heap
    at initialization time and referenced directly...presumably allowing for
    hardware paging to occur).

    If the application *does not* want to allow the user to specify at run-time
    (via an environment variable) whether or not to software-page the
    Non-Terminals, then the application specifies TRUE (1) or FALSE (0) for
    "page" and supplies NULL as the value of "ev_open".  This leaves the
    run-time user with no environment-variable option, and the application's
    boolean value for "page" indicates whether paging occurs or not.

    If the application *does* want to allow the user to specify at run-time
    whether or not to software-page the Non-Terminals, then the application
    specifies TRUE or FALSE for "page" (as the default to be used if the user
    chooses not to specify at run-time) and then also specifies an environment
    variable name via "ev_open".  At runtime, if the environment variable name
    is not defined, then the argument value for "page" determines whether
    software-paging occurs.  If the environment variable name is defined, then
    a value of "PAGE_NT" causes Non-Terminals to be software-paged (thereby
    overriding any "page" value of FALSE).  A value of "NOPAGE_NT" causes
    Non-Terminals to be read into the heap at initialization time (thereby
    overriding any "page" value of TRUE).  Any other value for the environment
    variable causes an error to be returned (MS0_BAD_ENV_VAR_VAL).
    
    If a "paging-Non-Terminals" open is actually done:

      - "ev_fl_depth", if non-NULL, specifies an environment variable name
      which (if defined) the value is taken to be the number of page-buffers
      cached on the Free List of page buffers.  If the value cannot be parsed
      to a non-negative number, or if the environment variable is not defined,
      then EITHER the compile-time value of symbol FREE_LIST_DEPTH is taken as
      the cache size (ie compile-time value of module-local cell named
      "fl_depth") OR the positive value of "free_list_size" is taken as the
      cache size (see below).

      - "max_buffs", if non-NULL, specifies where the maximum number of paging
      buffers will be returned to the caller.

      - "free_list_size", on INPUT, if non-NULL, any positive integer specifies
      the free list cache size if no valid value for this parameter could be
      extracted from the "ev_fl_depth" environment variable value.  If this
      parameter is NULL or its value is negative or there is no legal 
      "ev_fl_depth" environment variable value, then the compile-time value
      of symbol "FREE_LIST_DEPTH" is taken as the cache size.  On OUTPUT, if
      non-NULL, the actual value used for the free-list cache size is
      *RETURNED* into the specified integer.

    
    "preamble_list" is the cell which receives the address of a heap-storage
    array created to hold the preamble information read from the file.  The
    caller is expected to dispense with this storage when done with it via
    a call to "free()".  Note that "*preamble_list" is checked to be sure
    it is NULL, and if not, the storage at that address is freed via a call
    to "free()".  This allows "mir_t0_init()" to be called a 2nd and subsequent
    time without requiring the caller to free this preamble storage explicitly
    (mir_t0_init() will do it).
    
OUTPUTS:
    
    The function returns one of:
    
    MS_SUCCESS - The MIR data base was successfully loaded, and:
    * a pointer to it is set into "mas" (the module-wide location available
    to other functions in this module)
    * the preamble information is returned to the caller via array
    * the number of arcs in the longest OID in the index is stored in
    module-wide symbol "maxoid_arcs".
    
    MS0_BAD_ENV_VAR_VAL - The value of a specified environment string was not
    "NOPAGE_NT" or "PAGE_NT" (the only valid values).
    
    MS_MUTEX_INIT_FAIL - If symbol "THREADS" is defined and initialization of
    the mutex required to protect the mandle lists fails.
    
    MS0_DBFILE_OPENFAIL - If the filename (ultimately derived) could not be opened.
    (ERRNO is valid on return).
    
    MS0_DBFILE_READFAIL - If an "fread()" call failed (ERRNO is valid on return).
    
    MS0_BAD_ENDIAN - The binary MIR database file did not have a binary "1" in 
    the first four bytes, (this means it's not a valid MIR file or it was
    generated by MIRC on a machine that had a different byte ordering than
    the machine attempting to load it).
    
    MS0_BAD_VERSION - The binary file Format Version number was not supported
    by this version of the Tier 0 functions.
    
    
BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0 User that desires to perform MIR lookups using
        other functions in this module.  Initialization chores for these other
        functions have not yet been performed, or they need to be done on
        another (new?) MIR database file.
    
    Purpose:
        This function opens the MIR Database file, allocates enough space
        to hold it and then loads in the data.  Any other initialization
        chores required by other functions in this module are performed.
    
    
ACTION SYNOPSIS OR PSEUDOCODE:
    
    ifdef MTHREADS
    if (mutex init on Mandle List heads failed)
        <return error>
    endif
    
    if (MIR address space is already allocated)
        <free it>
    
    if (masa vector is allocated)
        <free it>
        <show it free>
    
    if (buffer vector is allocated)
        for (every entry in buffer vectory)
            <free the page buffer>
            <free buffer vector>
        <show it free>
    
    if (preamble space already exists)
        <free it>
        <show it free>
    
    if (masa-to-synonym array is already allocated)
        <free it>
        <show it free>
    
    <assume default name for input file>

    if (user passed in an input filename)
        <use it's value instead>
    
    if (environment variable string name for filename was passed)
        if (environment variable exists)
            <use it's value instead> -- environment variable overrides all!
    
    if (module-level file pointer for Tier 0 paged-open is non-NULL)
        <close the currently open MIR file>
        <indicate the file as closed>
    
    if (attempt to open input file failed)
        <return error>
    
    if (return-filename pointer is non-null)
        <return pointer to filename used>
    
    <read "endian-ness" indicator>
    if (indicator is not "1")
        <return error>
    
    <read in Binary File Format Version number>
    if (version is not "2")
        <return error>
    
    <read in preamble size>
    if (attempt to allocate preamble array storage failed)
        <return error>
    
    <read in preamble>
    
    <return pointer to preamble array data>
    
    <copy passed default page value to local storage page>
    if (paging environment variable name string is present)
        if (attempt to fetch value of environment variable succeeds)
            if (value is "NOPAGE_NT")
                <set local storage page to FALSE>
            else if (value is "PAGE_NT")
                <set local storage page to TRUE>
            else
                <return MS0_BAD_ENV_VAR_VAL>
    
    if (paging desired)
    
        <(re)-initialize module-level 'paging' cells>
    
        <compute fseek magic number for fread to get *any* masa read-in>

        if (attempt to allocate space to receive Partition-Table failed)
            <return MS_NO_MEMORY>
        if (attempt to read-in the Partition-Table Partition failed)
            <return MS0_DBFILE_READFAIL>

        <compute size of fore-shortened MAS: everything except Non-Terminals>
        <free the Partition-Table>

        if (attempt to allocate space to receive fore-shortened MAS failed)
            <return MS_NO_MEMORY>        
        if (attempt to read-in the fore-shortened MAS failed)
            <return MS0_DBFILE_READFAIL>

        <compute space required to hold Relationship subpartition>

        if (attempt to allocate space to receive Rel. subpartition failed)
            <return MS_NO_MEMORY>
        if (attempt to read-in the Rel. subpartition failed)
            <return MS0_DBFILE_READFAIL>

        if (attempt to allocate synonym-masa array to local module failed)
            <return error>
        for (each MIR Relationship non-terminal)
            <compute it's add. and store it in the next slot of synonym array>
        <store MIR Relationship count in local module>

        <release the space holding the Relationship subpartiton>

        <store size of largest Non-Terminal (in bytes) local to module>
        <round up to a respectable, probable disk-sector size>

        <compute number of buffers req'd given space for all Non-Terminals>
        if (attempt to allocate space for masa vector failed)
            <return MS_NO_MEMORY>
        if (attempt to allocate space for buffer vector failed)
            <return MS_NO_MEMORY>

        <record file pointer for use by other functions in this module>

        if (environment variable string name for Free List Depth is passed)
                    AND
            environment variable exists)
            <attempt to convert value to binary>
            if (value is negative)
                if (argument "free_list_size" is non-NULL)
                    if (value is positive)
                        <set depth to "free_list_size" value)
                    else
                        <set depth to compile-time symbol>
                ELSE
                    <set depth to compile-time symbol>
            else
                <set depth to environment variable value>
        else
            if (argument "free_list_size" is non-NULL)
                if (value is positive)
                    <set depth to "free_list_size" value)
                else
                    <set depth to compile-time symbol>
            else
                <set depth to compile-time symbol>

        if (argument "free_list_size" is non-NULL)
            <return free-list depth through this argument>

        if (argument "max_buffs" is non-NULL)
            <return maximum number of buffers through this argument>

    else (* No Paging Desired *)
        if (attempt to allocate MIR Address Space array storage failed)
            <return error>
        <read in MAS data to big array>
        <store MAS address local to module>    
        <store MIR Relationship count in local module>
        if (attempt to allocate synonym-masa array to local module failed)
            <return error>
        for (each MIR Relationship non-terminal)
            <compute it's address and store it in the next slot of synonym
             array>

        <close the input file>
    
    <store longest OID in arcs local to module>
    
    <return MS_SUCCESS>
    
    
OTHER THINGS TO KNOW:


    V1.00 - Currently precious little ancillary information other than the
    compiled MIR data is present in the MIR binary database file; only the
    compiler major and minor version numbers are present.  In the future this
    function should probably return a structure containing all this stuff
    (and whatever else might be present, like date/time of compilation, node
    name, user comment etc).

    For a one-sending-thread SNMP PE, this is fine.  For two or more sending
    threads or MCC environment, the invocation of this function must be blocked
    by a "once" block to be assured it executes only once.  (No horrible
    harm if more than once, we'll just waste heap in a big way).

    V1.95 - Binary File Format Version 2 now in use: preamble contains added
    information needed when compiler loads a binary for a compilation.  New
    premable info is now returned in an array, rather than separate arguments
    to this function.

    V2.00 - This function expanded to support software 'paging' of
    Non-Terminals.  This function may be repeatedly invoked to open different
    MIR files, however only the last-opened is currently active.  Notice that
    any mandles that are active at the time this function is invoked for the
    second or subsequent time become logically invalid (and should be freed
    via a call to mir_free_mandle() or mir_free_mandle_class()).  This
    function takes no steps to actively mark any currently active mandles as
    invalid!  The caller must explicitly free all mandles before calling this
    function for a 2nd or subsequent time.  Note that any Tier 1 function
    packages may need to re-init their list of frequently-used mandles.

    When paging Non-Terminals, the size of the buffers used is dependent on
    the largest Non-Terminal compiled into the MIR database file that is in
    use.  If the largest Non-Terminal is less than 512 BYTES (not MIR cells)
    then the buffers are 512 bytes large (128 MIR Address Space cells).  If
    the largest Non-Terminal is > 512 bytes, then the buffers are all sized
    to be large enough to hold the largest Non-Terminal.

    The size of the buffers used in-turn affects the maximum number of
    buffers that Tier 0 will allow to be allocated.  After computing the size
    of the buffers according to the scheme outlined above, the maximum number
    of buffers allowed is computed as follows:

       * The number of buffers needed to hold all the Non-Terminals
         simultaneously is computed (assuming that the buffers never hold
         any duplicates of the MIR address space and that no 'holes' are
         allowed).

       * The number of buffers is then increased by 50% (to try to account
         for the inevitable overlap that will occur).

    The number of buffers cached on the free list ("free list depth"), (while
    setable as an explicit number via environment variable by the run-time
    user) is clearly dependent on the number of buffers that *might* be
    allocated as a maximum, which (as can be seen from the foregoing
    discussion) is dependent on the buffer size.  Consequently, any twiddling
    of the free-list depth must be done *carefully*, being aware that an
    optimum number for one MIR database file may *NOT BE* for another file.

    (Perhaps the scheme should allow the free list depth to be set as
     percentage of the total number of buffers that may maximally be allocated,
     but that ain't the way it works with V2.0).

    - - - - - - - - - -

    NOTE:  If you change this function to read in a larger Preamble, you must
    change code in "mir_symdump.c" that dumps it, and "mir_internal.c"
    (function "mirci_internal_init()") that reads it.

*/

{
FILE         *ifptr;        /* Input Data File Pointer                       */
unsigned int *array;        /* --> allocated storage for MIR data            */
char         *ifname;       /* Name to be used to open the MIR Database file */
char         *alt_ifname;   /* Alternate MIR Database input filename         */
int          i,j;           /* general purpose indices                       */
int          bump_amount;   /* Amt to bump indices by when walking Rel.Part. */
int          pre_size;      /* Size of Preamble in bytes                     */
int          *preamble;     /* Preamble loaded here                          */
masa         NT_rel;        /* masa for (next) MIR Relationship Non-Terminal */
unsigned int cell;          /* First cell in  MIR Relationship Non-Terminal  */
char         *page_type_str;/* Value retrieved for "ev_open" env. variable   */
char         *fl_depth_str; /* Value retrieved for "ev_fl_depth" env. var.   */
BOOL         page_type;     /* TRUE: Page Non-Terminals, FALSE: Don't page   */
int          short_mas_size;/* Size of the "fore-shortened" MAS for paging   */
int          NT_rel_size;   /* Size *in bytes* of the Rel. NT partition      */
    
    
#ifdef MTHREADS
/*
  | Initialize the mutex associated with the mandle class lists and paging
  | data-structures.
  |
  | NOTE: Code needed here to cause this initialization to occur only once
  |       if this function is invoked for a 2nd or subsequent time!
  */
if (pthread_mutex_init(&mandle_lists_m, pthread_mutexattr_default)
    != 0) {
    return(MS_MUTEX_INIT_FAIL);
    }
#endif
    
/* if (MIR address space is already allocated) */
if (mas != NULL) {
    /* free it */
    free(mas);
    }

/* if (masa vector is allocated) */
if (masa_vector != NULL) {
    free(masa_vector);          /* free it      */
    masa_vector = NULL;         /* show it free */
    pbuffer_maxcount = 0;       /* "no longer 'paged' open */
    }

/* if (buffer vector is allocated) */
if (buffer_vector != NULL) {

    /* for (every entry in buffer vector) */
    for (i=0; i < pbuffer_curcount; i++) {
        free((buffer_vector[i])->page); /* free the buffer itself      */
        free(buffer_vector[i]);         /* free the page_buf structure */
        }

    free(buffer_vector);        /* free it            */
    buffer_vector = NULL;       /* show it free       */
    pbuffer_curcount = 0;       /* no current buffers */
    }

/* if (preamble space already exists) */
if (*preamble_list != NULL) {
    free(*preamble_list);       /* free it      */
    *preamble_list = NULL;      /* show it free */
    }

/* if (masa-to-synonym array is already allocated) */
if (masa_to_synonym != NULL) {
    free(masa_to_synonym);      /* free it      */
    masa_to_synonym = NULL;     /* show it free */
    }

/* assume default name for input file */
ifname = "mir.dat";

/*
  | If the user passed in a file name to use, let that override the internal
  | default.
  */
if (datafile != NULL) {
    ifname = datafile;
    }

/* if (environment variable string name for filename was passed) */
if (ev_file != NULL) {

    /* NOTE: Environment variable overrides *ALL* the default value(s)!
       |
       |  Find out if they've established an environment variable with the MIR
       |  database filename in it... use that instead if it is there.
       */
    if ( (alt_ifname= (char *) getenv(ev_file)) != NULL) {
        ifname = alt_ifname;
        }
    }

/* if (module-level file pointer for Tier 0 paged-open is non-NULL) */
if (mir_file != NULL) {
    /* close the currently open MIR file */
    fclose(mir_file);

    /* indicate the file as closed */
    mir_file = NULL;
    }

/* Attempt to open the input file */
if ( (ifptr = fopen(ifname, "rb")) == NULL) {
    return(MS0_DBFILE_OPENFAIL);
    }
    
/* if (return-filename pointer is non-null) */
if (actual_dfile != NULL) {
    /* return pointer to filename used */
    *actual_dfile = ifname;
    }

/* read "endian-ness" indicator */
if (fread(&i, sizeof(i), 1, ifptr) != 1) {
    fclose(ifptr);
    return(MS0_DBFILE_READFAIL);
    }
if (i != 1) {
    fclose(ifptr);
    return(MS0_BAD_ENDIAN);
    }

/* read "Binary Format Version" indicator */
if (fread(&i, sizeof(i), 1, ifptr) != 1) {
    fclose(ifptr);
    return(MS0_DBFILE_READFAIL);
    }
if (i != BINARY_FORMAT) {
    fclose(ifptr);
    return(MS0_BAD_VERSION);
    }

/* read Preamble Size */
if (fread(&pre_size, sizeof( int ), 1, ifptr) != 1) {
    fclose(ifptr);
    return(MS0_DBFILE_READFAIL);
    }

/* Allocate a preamble array big enough to hold it all */
if ( (preamble = (int *) malloc(pre_size)) == NULL) {
    return(MS_NO_MEMORY);
    }

/* Read in the Preamble */
if (fread(preamble, pre_size, 1, ifptr) != 1) {
    fclose(ifptr);
    return(MS0_DBFILE_READFAIL);
    }

/* return pointer to preamble to caller */
*preamble_list = preamble;


/* copy passed default page value (TRUE/FALSE) to local storage page */
page_type = page;

/* if (paging environment variable name string is present) */
if (ev_open != NULL) {

    /* if (attempt to fetch value of environment variable succeeds) */
    if ((page_type_str = getenv(ev_open)) != NULL) {

        /* if (value is "NOPAGE_NT") */
        if (strcmp(page_type_str, "NOPAGE_NT") == 0) {
            page_type = FALSE;  /* set local storage page to FALSE */
            }
        else if (strcmp(page_type_str, "PAGE_NT") == 0) {
            page_type = TRUE;  /* set local storage page to TRUE */
            }
        else {
            return (MS0_BAD_ENV_VAR_VAL);
            }
        }
    }

/* if (paging desired) */
if (page_type == TRUE) {

    /* (re)-initialize module-level 'paging' cells
    |
    | The buffers in the free list are blown away thru the loop above
    | that dumps the buffer_vector, here we're just resetting the Free List
    | cells appropriately.
    */
    top_free = NULL;
    bot_free = NULL;
    pbuffer_freecount = 0;

    /* compute fseek magic number for fread to get *any* masa read-in
    |
    | The "read-point" for the input file is now positioned to the
    | first cell of the MAS (masa "1").  The first cell of MAS contains the 
    | first cell of the Partition-Table partition.  Before we do a read, we
    | compute the "magic-fseek" number.  The code in other sections of this
    | module expects to be able to take the "magic-fseek" number,,
    | add it to (sizeof(int)*(the masa they seek to read-in next)),
    | do an "fseek" using this sum, and then do the read.
    |
    | The "magic_fseek" number is whatever "ftell" returns MINUS 
    | (sizeof(int)*ONE).
    */
    if ((magic_fseek = ftell(ifptr)) == -1) {
        return(MS0_DBFILE_READFAIL);
        }
    magic_fseek -= sizeof(int);

    /* Partition Table space
    |
    | Note:  Here we want to allocate enough space to hold all the entries in
    | the first part of the MAS *up to but not including* the first cell of
    | the first partition following the Partition-Table Partition (this would
    | be the first cell of the SLICE Partition).  The 1st cell of the SLICE
    | partition starts at the MASA given by the symbol FIRST_MASA_AFTER_P_TABLE.
    | While we are not interested in reading in THAT CELL, (we want to read the
    | number of cells given by the value of "FIRST_MASA_AFTER_P_TABLE-1"), we
    | need to allocate ONE MORE (4-byte) CELL in the array, because MASA of "0"
    | is not legal (and the entry in the array is not used).  Consequently the
    | number of cells we want to ALLOCATE is "FIRST_MASA_AFTER_P_TABLE-1+1",
    | and we'll READ from the file into the second cell (whose masa is "1": 
    | mas[1]) ONE LESS CELL than we allocated space for.
    |
    | if (attempt to allocate space to receive Partition-Table failed) 
    */
    if ((mas = (unsigned int *) malloc((FIRST_MASA_AFTER_P_TABLE-1+1)*sizeof(int)))
        == NULL){
        return(MS_NO_MEMORY);
        }

    /* if (attempt to read-in the Partition-Table Partition failed) */
    if (fread((mas+1),
                ((FIRST_MASA_AFTER_P_TABLE-1)*sizeof(int)),
                 1,
                 ifptr) != 1) {
        return(MS0_DBFILE_READFAIL);
        }

    /* compute size of fore-shortened MAS: everything except Non-Terminals
    |
    | We've just read in enough of the MAS (specifically the Partition-Table
    | Partition) to be able to tell where all the other partitions in the MAS
    | begin and how big they are.
    |
    | During an open "for paging", we only want to permanently read-in
    | everything in the MAS *up to* the first Non-Terminal (sub) partition.
    | Consequently the last partition to be read-in is that for Strings,
    | and the masa of the first Non-Terminal partition can be found in
    | the Partition Table cell whose address is "START_NONTERM_DC_MASA".
    | We want to allocate space-for, and read-in, everything *up to* but
    | not including that address.  (For a picture & more insight, see "mir.h"
    | near the string "START_NONTERM_DC_MASA").
    */
    short_mas_size = mas[START_NONTERM_DC_MASA]*sizeof(int);/* Size in bytes */

    /* free the Partition-Table */
    free(mas);

    /*
    | We only needed the Partition-Table to compute the fore-shortened size.
    | Now we throw it away and read it in again, but this time include
    | everything up to but not including the Non-terminals.
    |
    | if (attempt to allocate space to receive fore-shortened MAS failed)
    |
    |  (Note: cell "0" in the MAS array is "not legal", so we read the MAS into
    |   "array" starting at "1", so we  allocate one more cell with
    |   "sizeof(mas[0])")
    */
    if ((mas = (unsigned int *) malloc((short_mas_size+sizeof(mas[0]))))
        == NULL){
        return(MS_NO_MEMORY);
        }

    /* if attempt to reposition to start of MAS within the MIR file failed) */
    if (fseek(ifptr,
              ((1*sizeof(unsigned int)) + magic_fseek),
              SEEK_SET
              ) != 0) {
        return(MS0_DBFILE_READFAIL);
        }

    /* if (attempt to read-in the fore-shortened MAS failed) */
    if (fread((mas+1),
              (short_mas_size),           /* Size in bytes */
              1,
              ifptr) != 1) {
        return(MS0_DBFILE_READFAIL);
        }

    /*
    | We need to build a synonym-to-masa array.  To do that, we need to
    | scan over every Relationship Non-Terminal.  To do that, we've got to
    | allocate enough space to read them all in temporarily to do the scan.
    |
    | The starting masa of the Relationship Non-terminal partition is stored
    | in the Partition Table at masa "START_NONTERM_REL_MASA".  This partition
    | is the last partition in the MIR address space, so consequently the
    | masa of the last cell in that partition is equal to the size of the
    | MIR address space (in bytes) divided by the number of bytes per cell.
    |
    | Subtracting the masa of the first cell from the masa of the last cell
    | and adding 1 (for the 'fencepost') gives the total # of cells in the
    | MIR relationship non-terminal subpartition, multiply by cell-size to get
    | bytes.
    |
    |  compute space (*in bytes *) required to hold Relationship subpartition
    */
                   /* ------masa of last valid cell---- */
    NT_rel_size = ((preamble[PRE_MAS_SIZE]/sizeof(mas[0])) - 

                  /* masa of first valid cell */
                   mas[START_NONTERM_REL_MASA]   + 1) * sizeof (unsigned int);


    /* if (attempt to allocate space to receive Rel. subpartition failed) */
    if ((array = (unsigned int *) malloc(NT_rel_size)) == NULL){
        return(MS_NO_MEMORY);
        }

    /* if (attempt to read-in the Rel. subpartition failed) */
    if (fseek(ifptr,
              (long) ((mas[START_NONTERM_REL_MASA]*sizeof(unsigned int)) + magic_fseek),
              SEEK_SET
              ) != 0) {
        return(MS0_DBFILE_READFAIL);
        }
                    /* Size in bytes */
    if (fread(array,    NT_rel_size,    1, ifptr) != 1) {
        return(MS0_DBFILE_READFAIL);
        }

    /*
    | The MIR Relationship Partition has been read-in temporarily.  Now
    | set up space for the synonym-masa array.
    |
    | if (attempt to allocate synonym-masa array to local module failed)
    */
    /* store MIR Relationship count in local module */
    synonym_count = preamble[PRE_NT_REL_CNT];

    /* if (attempt to allocate synonym-masa array to local module failed)
    |  NOTE: The synonym array is ONE origined, so we can directly map synonyms
    |        which start at 1.
    */
    if ((masa_to_synonym = (int *)
         malloc((synonym_count+1)*sizeof(masa))) == NULL ) {
        return(MS_NO_MEMORY);
        }

    /* for (each MIR Relationship non-terminal) */
    for (i=1, NT_rel=mas[START_NONTERM_REL_MASA], j=0;
         i <= synonym_count;
         i++) {

        /* compute it's address and store it in the next slot of synonym array */
        masa_to_synonym[i] = NT_rel;

        /* Compute start of next NT (if any) */
        cell = array[j];/* Grab packed Rel-Table Entry Count & OID Bptr count*/
        bump_amount = 1                         /* Step over Count cell..    */
         + (cell & mas[ENTRY_COUNT_AMASK_MASA]) /* ..Over Rel. Table Entries */
         + (cell >> mas[OID_COUNT_RSHIFT_MASA]);/* ..Over OID Backpointers   */
        j += bump_amount;       /* Step local index */
        NT_rel += bump_amount;  /* Step MASA        */
        }

    /* release the space holding the Relationship subpartiton */
    free(array);

    /* store size of largest Non-Terminal (in bytes) local to module */
    pbuffer_size = preamble[PRE_NT_MAX_SIZE];

    /*
    | Round up to a respectable, probable disk-sector size in case the largest
    | Non-Terminal turns out to be smaller than your-average disk sector.
    */
    if (pbuffer_size < 512)
        pbuffer_size = 512;

    /* compute number of buffers req'd given space for all Non-Terminals
    |
    | The number of entries in this array is the total number of buffers that
    | would be required to hold all the MIR Non-Terminals (if every buffer
    | started exactly where the previous buffer ended) PLUS 50% of that number.
    |
    | Space for all Non-Terminals is computed (to within a cell or two) by
    | taking the address of the last valid cell (ie total size / cellsize)
    | less the starting Non-Terminal masa (mas[START_NONTERM_DC_MASA]),
    | multiply by sizeof(unsigned int) to get size in bytes, divide
    | by largest Non-Terminal Size (pbuffer_size), then add 50%.
    |
    | (See the MIR Address Space layout in "mir.h" near the definition of
    |  symbol "START_NONTERM_DC_MASA").
    */
    i = pbuffer_maxcount =
        /* masa of last valid cell */
        ((((preamble[PRE_MAS_SIZE]/sizeof(mas[0])) - 

                  /* masa of first valid Non-Terminal cell */
                   mas[START_NONTERM_DC_MASA]   + 1) * sizeof (unsigned int))

            / pbuffer_size);

    /* Now add 50% */
    pbuffer_maxcount += (i/2);

    /*
    | NOTE: In the allocations below, we allocate space for one more slot
    |       in each vector (masa & buffer) because in function NT_page_in()
    |       the code that maintains these vectors needs to have space for
    |       one more entry during re-organizations:
    */
    /* if (attempt to allocate space for masa vector failed) */
    if ((masa_vector =
         (unsigned int *) malloc((pbuffer_maxcount+1)*sizeof(masa)))
        == NULL ) {         
        return(MS_NO_MEMORY);
        }

    /* if (attempt to allocate space for buffer vector failed) */
    if ((buffer_vector =
         (page_buf **) malloc((pbuffer_maxcount+1)*sizeof(page_buf *)))
        == NULL ) {
        return(MS_NO_MEMORY);
        }

    /* record file pointer for use by other functions in this module */
    mir_file = ifptr;

    /*
    | if (environment variable string name for Free List Depth is passed and
    |     it exists)
    */
    if ( (ev_fl_depth != NULL)
        && ((fl_depth_str = getenv(ev_fl_depth)) != NULL)) {

        /* attempt to convert value to binary */
        errno = 0;
        fl_depth = atoi(fl_depth_str);

        /* if (value is negative or invalid) */
        if (/* fl_depth == INT_MIN  || */
            errno == ERANGE
            || (fl_depth < 0)) {

                /* if (argument "free_list_size" is non-NULL) */
                if (free_list_size != NULL) {

                    /* if (value is positive) */
                    if (*free_list_size > 0) {
                        /* set depth to "free_list_size" value */
                        fl_depth = *free_list_size;
                        }
                    else { /* Compile-time caller passed invalid value */
                        /* set depth to compile-time symbol */
                        fl_depth = FREE_LIST_DEPTH;
                        }
                    }
                else {      /* Compile-time caller didn't offer value */
                    /* set depth to compile-time symbol */
                    fl_depth = FREE_LIST_DEPTH;
                    }
                }
        /* ELSE
            set depth to environment variable value (done above) */
        }
    else { /* No valid environment variable value */
        /* if (argument "free_list_size" is non-NULL) */
        if (free_list_size != NULL) {

            /* if (value is positive) */
            if (*free_list_size > 0) {
                /* set depth to "free_list_size" value */
                fl_depth = *free_list_size;
                }
            else { /* Compile-time caller passed invalid value */
                /* set depth to compile-time symbol */
                fl_depth = FREE_LIST_DEPTH;
                }
            }
        else {      /* Compile-time caller didn't offer value */
            /* set depth to compile-time symbol */
            fl_depth = FREE_LIST_DEPTH;
            }
        }

    /* if (argument "free_list_size" is non-NULL) */
    if (free_list_size != NULL) {
        /* return free-list depth through this argument */
        *free_list_size = fl_depth;
        }

    /* if (argument "max_buffs" is non-NULL) */
    if (max_buffs != NULL) {
        /* return maximum number of buffers through this argument */
        *max_buffs = pbuffer_maxcount;
        }

#ifdef PAGE_TRACE
    fprintf(PT_FILE, "\nPT: Paged-Open requested:\n");
    fprintf(PT_FILE, "PT: pbuffer_maxcount = %d\n", pbuffer_maxcount);
    fprintf(PT_FILE, "PT: pbuffer_curcount = %d\n", pbuffer_curcount);
    fprintf(PT_FILE, "PT: pbuffer_size     = %d\n", pbuffer_size);
    fprintf(PT_FILE, "PT: pbuffer_freecount= %d\n", pbuffer_freecount);
    fprintf(PT_FILE, "PT: magic_fseek      = %d\n", magic_fseek);
    fprintf(PT_FILE, "PT: fl_depth         = %d\n", fl_depth);
#endif
    }

else {  /* No Paging Desired */

    /* Allocate storage big enough to receive all of the MIR Address Space
    |
    |  (Note: cell "0" in the MAS array is "not legal", so we read the MAS into
    |   "array" starting at "1", so we allocate one more cell with
    |   "sizeof(array[0])")
    */
    if ((array = (unsigned int *)malloc(preamble[PRE_MAS_SIZE]+sizeof(array[0])))
        == NULL) {
        fclose(ifptr);
        return(MS_NO_MEMORY);
        }

    /* Now read in the rest of the MIR data */
    if ((i = fread(&array[1], preamble[PRE_MAS_SIZE], 1, ifptr)) != 1) {
        fclose(ifptr);
        return(MS0_DBFILE_READFAIL);
        }

    /* Store pointer to the newly allocated MAS storage. */
    mas = array; /* (Stored locally in this module, not returned to user!) */

    /* store MIR Relationship count in local module */
    synonym_count = preamble[PRE_NT_REL_CNT];

    /* if (attempt to allocate synonym-masa array to local module failed)
    |  NOTE: The synonym array is ONE origined, so we can directly map synonyms
    |        which start at 1.
    */
    if ((masa_to_synonym = (int *)
         malloc((synonym_count+1)*sizeof(masa))) == NULL ) {
        return(MS_NO_MEMORY);
        }

    /* for (each MIR Relationship non-terminal) */
    for (i=1, NT_rel=mas[START_NONTERM_REL_MASA]; i <= synonym_count; i++) {
        /* compute NT address and store it in the next slot of synonym array */
        masa_to_synonym[i] = NT_rel;

        /* Compute start of next NT (if any) */
        cell = mas[NT_rel];/* Grab packed Rel-Table Entry Cnt & OID Bptr Cnt */
        NT_rel += 1                             /* Step over Count cell..    */
         + (cell & mas[ENTRY_COUNT_AMASK_MASA]) /* ..Over Rel. Table Entries */
         + (cell >> mas[OID_COUNT_RSHIFT_MASA]);/* ..Over OID Backpointers   */
        }

    /* Close up the file, we don't need it anymore */
    fclose(ifptr);
    }

/* Store longest OID in arcs local to module */
maxoid_arcs = preamble[PRE_MAX_ARCS];

return (MS_SUCCESS);
}

/* mir_get_rel_mandles - Get Relationship MIR Object Mandles */
/* mir_get_rel_mandles - Get Relationship MIR Object Mandles */
/* mir_get_rel_mandles - Get Relationship MIR Object Mandles */

mir_status
mir_get_rel_mandles(count, pair_list, mclass)

unsigned     count;        /* Number of entries in following array      */
rel_pair     *pair_list;   /* --> Array of rel_pairs with count entries */
mandle_class **mclass;     /* --> Mandleclass pointer for mandle class  */
                           /* to use.                                   */
/*
INPUTS:

    "count" indicates the number of entries in the array specified by
    "pair_list".

    "pair_list" is an array (of count entries) of "rel_pair" structures.
    The user indicates in each "rel_pair" structure the "name" (on input) 
    of the MIR Relationship Object for which a mandle (to it) is desired.
    (You put a pointer to the string into the entry.  All names in any given
    invocation of this function SHOULD BE UNIQUE! In other words, don't ask
    for the same relationship object twice within the same invocation!)

    "mclass" is a pointer to the mandleclass pointer for the mandle class
    that this function uses when creating mandles that are created for
    each of the Relationship Object mandles.

    NOTE: The "m" cell in each rel_pair entry MUST BE NULL on entry.  If
    the function cannot determine a mandle, it leaves the entry alone (and
    the NULL remains), thereby signalling back to the caller which mandle(s)
    weren't found.  An error is returned if this requirement is not met.


OUTPUTS:

The function returns one of:

MS_SUCCESS - All the MIR Relationship Names in the array were successfully
        mapped to mandles (and their mandles returned).

MS_FAILURE - Not All MIR Relationship Names were successfully mapped to
        mandles.  Any entry with a NULL mandle pointer was not successfully
        mapped (or argument "count" was illegally zero).

MS0_INVALID_MANDLE - One or more "m" cells in the rel_pair array were not
        initialized to NULL (as they should have been) on entry.

MS0_INDEX_CORRUPTED - If the main MIR Relationship Object
        "MIR_Relationship_Name" could not be found in the MIR.

Any invalid return possible for "mir_oid_to_mandle()" or "mir_mandle_to_oid()".


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0 User that desires to perform MIR lookups using
        other functions in this module.  At some point, a mandle for MIR
        Relationship Object(s) used by the compiler will be needed to do
        searches in the Relationship tables of Non-Terminal MIR Objects.

    Purpose:
        This function makes it very easy to obtain the mandles for one
        or more MIR Relationship Objects "by name" (as opposed to OID).


ACTION SYNOPSIS OR PSEUDOCODE:

    if (count is zero)
        <return MS_FAILURE>
    for (each entry in rel_pair array)
        if (mandle pointer is not NULL)
            <return MS0_INVALID_MANDLE>

    <construct OID for MIR Relationship "MIR_Relationship_Name">
    <attempt a mir_oid_to_mandle on the built OID>
    if (return is not MS0_FIND_EXACT)
        <return MS0_INDEX_CORRUPTED>

    <copy MIR_Relationship_Name mandle as "current mandle">

    do
        <free any storage associated with "current oid">
        <perform mandle-to-oid conversion on "current mandle" to get new
         "current oid">
        if (status is not SUCCESS)
            <return status>
        <perform search on "current mandle" for "MIR_Relationship_Name">
        if (search status is MS0_FIND_TERM)
            for (each entry in the rel_pair list)
                if (mandle pointer is NULL)
                    if (entry name matches just-found relationship name)
                        <copy "current mandle" pointer into rel_pair entry>
                        <clear "current mandle" pointer to NULL>
                        <break>
            if (string was returned)
                <release string storage returned>
        else
            continue    (* This'll bag us out *)

        <perform GET_NEXT on "current oid", receive to "current mandle">
        while (status is MS0_FIND_EXACT and mandle is for a Rel. Object and
               search status was MS0_FIND_TERM)

    <free our local mandle for "MIR_Relationship_name">

    for (each entry in rel_pair array)
        if (mandle-pointer is NULL)
            <return MS_FAILURE>

    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

   You should not offer duplicate MIR Relationship names in the input rel_pair
   array, as only the first occurrence will be returned, and you'll get
   MS_FAILURE indicating that not all names were found.

*/

#define REL_NAME_OID_LEN 9
{
unsigned        i;                        /* Local General-purpose Index */
unsigned int    value[REL_NAME_OID_LEN];  /* Storage for OID arcs        */
object_id       rel_name_oid;             /* OID of MIR_Relationship_Name*/
mandle          *current_mandle=NULL;     /* "Current Mandle" for loop   */
object_id       current_oid;              /* "Current OID" for c. mandle */
mandle          *m_mir_rel_name=NULL;     /* Mandle used to get rel name */
mandle_class    *local_class=NULL;        /* Mandle class for local use  */
mandle          *junk=NULL;               /* (Should never be used)      */
mir_status      status;                   /* Local status holder         */
mir_status      sstatus;                  /* Local SEARCH status holder  */
mir_value       mvalue;                   /* MIR Terminal value (string) */


/* Zap storage for local OID clear */
current_oid.count = 0;
current_oid.value = NULL;

/* if (count is zero) */
if (count == 0) 
    return (MS_FAILURE);

/* for (each entry in rel_pair array) */
for (i=0; i < count; i++) {
    /* if (mandle pointer is not NULL) */
    if (pair_list[i].m != NULL) 
        return (MS0_INVALID_MANDLE);
    }

/* construct OID for MIR Relationship "MIR_Relationship_Name" */
rel_name_oid.count = REL_NAME_OID_LEN;
rel_name_oid.value = value;
value[0]=1;     /* ISO                                              */
value[1]=3;     /* Identified Organization                          */
value[2]=12;    /* ICD-European Computer Manufacturer's Assocation  */
value[3]=2;     /* Member Organization                              */
value[4]=1011;  /* Digital Equipment Corp.                          */
value[5]=2;     /* Enterprise Management Architecture               */
value[6]=17;    /* Properties (Per Entity Model)                    */
value[7]=1;     /* 1 = "Properties for MIR Compiler (as of V2.0)"   */
value[8]=0;     /* OID Arc for "MIR_Relationship_Name": The Root    */

/* attempt a mir_oid_to_mandle on the built OID */
status = mir_oid_to_mandle(GET_EXACT,       /* Exact lookup             */
                           &rel_name_oid,   /* OID of Relationship Name */
                           &m_mir_rel_name, /* Mandle of Rel. name      */
                           mclass,          /* (See copy comment below) */
                           NULL);           /* Don't care about OID SMI */

/* if (return is not MS0_FIND_EXACT) */
if (status != MS0_FIND_EXACT)         /* We lose a little mandle storage    */
    return (MS0_INDEX_CORRUPTED);     /*  here. . but it looks fatal anyway */

/* copy MIR_Relationship_Name mandle as "current mandle"
|
|  NOTE: We used the user's passed-in mandle_class ("mclass") to obtain the
|        mandle for "MIR_Relationship_Name" (above).  We did that so that this
|        copy will result in returning a mandle (in the user's mandleclass) IF
|        it turns out that one of the mandles they want is for
|        "MIR_Relationship_Name".  This way we don't have to keep track of
|        whether or not they asked for the first one, we'll just release
|        our local copy below.
*/
if ((status = mir_copy_mandle(m_mir_rel_name, &current_mandle)) != MS_SUCCESS)
    /* We'll lose storage for mandle m_mir_rel_name if we return */
    return (status);

do {
    /* free any storage associated with "current oid" */
    if (current_oid.value != NULL) {
        free(current_oid.value);
        current_oid.value = NULL;
        current_oid.count = 0;
        }

    /*
    | perform mandle-to-oid conversion on "current mandle" to get new
    | "current oid" (from which we can eventually do a GET_NEXT-lookup).
    */
    status = mir_mandle_to_oid(current_mandle, &current_oid, NULL);

    /* if (status is not SUCCESS) */
    if (status != MS_SUCCESS)
        /* We'll lose storage for mandle m_mir_rel_name if we return */
        return (status);

    /* perform search on "current mandle" for "MIR_Relationship_Name" */
    sstatus = mir_search_rel_table(SEARCH_FROMTOP,     /* Start looking here */
                                  current_mandle,      /* Search this thing  */
                                  m_mir_rel_name,      /* ..for this thing   */
                                  &local_class,        /* Should not get used*/
                                  &junk,               /* Should not get used*/
                                  &mvalue);            /* Rel Name comes back*/
                                                       /* in this structure  */
    /* if (sstatus is MS0_FIND_TERM) */
    if (sstatus == MS0_FIND_TERM) {

        /* for (each entry in the rel_pair list) */
        for (i=0; i < count; i++) {

            /* if (mandle pointer is NULL) */
            if (pair_list[i].m == NULL) {

                /* if (entry name matches just-found relationship name) */
                if (strcmp(pair_list[i].name, MV_STRING(&mvalue)) == 0) {

                    /* copy "current mandle" pointer into rel_pair entry */
                    pair_list[i].m = current_mandle;

                    /* clear "current mandle" pointer to NULL */
                    current_mandle = NULL;
                    break;  /* This means we find only the first occurrence */
                    }       /* if duplicate names were inadvertently given  */
                }
            }
        }
    else {
        continue;       /* This'll cause us to bag out early */
        }

    /* perform GET_NEXT on "current oid", receive to "current mandle" */
    status = mir_oid_to_mandle(GET_NEXT,        /* GET_NEXT = Lookup style */
                               &current_oid,    /* from this OID           */
                               &current_mandle, /* return next NT obj here */
                               mclass,          /* use this mandleclass    */
                               NULL);           /* Don't care what SMI     */

     
    }                                   /* WHILE:                           */
    while (   status == MS0_FIND_EXACT  /* Lookup was EXACT                 */
           && sstatus == MS0_FIND_TERM  /* Search found a Terminal (string) */
           && IS_A_NONTERM_REL(current_mandle->m.m.ex_add)  /* Got NT REL   */
          );

/* free our local mandle for "MIR_Relationship_name" */
if ((status = mir_free_mandle(&m_mir_rel_name)) != MS_SUCCESS) {
    return (status);
    }

/* for (each entry in rel_pair array) */
for (i=0; i < count; i++) {
    /* if (mandle-pointer is NULL) */
    if (pair_list[i].m == NULL) {
        return (MS_FAILURE);
        }
    }

return(MS_SUCCESS);

}

/* set_mandle - Allocate and Set-Up Mandle */
/* set_mandle - Allocate and Set-Up Mandle */
/* set_mandle - Allocate and Set-Up Mandle */

static mir_status
set_mandle (obj_mandle, mclass)

mandle      **obj_mandle;   /* ->> Mandle                               */
mandle_class **mclass;      /* ->> Mandle Class                         */

/*
INPUTS:

    "obj_mandle" is the address of a pointer to a mandle.  If this pointer
    is NULL, then a new mandle is created (for use by the caller to describe
    a MIR object).  This originally-NULL pointer is set to point to the new
    mandle on return.  If the pointer is non-null, then it is taken as the
    address of a mandle to be re-used.

    "mclass" is the address of a pointer to a mandle class. If this pointer
    is NULL, and an object is found in the MIR, then when a new mandle is
    created to return the object, this pointer is set to point to that mandle's
    class on return.  If the pointer is non-null, then it is taken as the address
    of a mandle class into which any new mandle that is created is to be put.
    
OUTPUTS:

The function returns one of:

MS_SUCCESS - The MIR mandle and mandle class were successfully setup

MS0_INVALID_MANDLE - Mandle address passed turns out not to point to
    a valid mandle.

MS_NO_MEMORY - Insufficient memory available to allocate a mandle or
    mandle-class.

BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0 function (ret_mandle() for mir_oid_to_mandle()
        or mir_search_rel_table()) that desires to return an object to the
        Tier 0 caller via a mandle.

    Purpose:
        This function acquires a mandle (and possibly a mandle-class)
        and sets up the mandle and mandle class so that the caller
        can fill it in with information that it must return to its caller.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (mandle pointer is NULL)
        LOCK_MANDLES()
        if (free mandle list is not empty)
            <allocate a mandle from the free list>
            <set the passed pointer to point to it>
        else
            if (attempt to allocate storage for mandle succeeded)
                <set the passed pointer to point to it>
            else
                UNLOCK_MANDLES()
                <return MS_NO_MEMORY>

        if (mandle_class pointer is NULL)
            if (free mandle list is not empty)
                <allocate a mandleclass from the free list>
            else
                if (attempt to allocate storage for mandle class failed)
                    UNLOCK_MANDLES()
                    <return MS_NO_MEMORY>
            <put the new mandle class on the mandle class list>
            <set the passed pointer to point to it>

        else
            <Make our local pointer jibe with what we were passed>

        <hook the new mandle into the mandle class we've got>
        <Reset "Last Match" entry to "none">
        <Show no synonym>
        <Show no page buffer>
        UNLOCK_MANDLES()
    else
        if (mandle is not valid)
            <return INVALID_MANDLE>
        else
            <Reset "Last Match" entry to "none">
            <Show no synonym>
            LOCK_MANDLES()
            RELEASE_ANY_PAGE_BUFFER(for new mandle)
            UNLOCK_MANDLES()

    <return MS_SUCCESS>

OTHER THINGS TO KNOW:

   Note that this function merely creates storage for a mandle (if need be)
   and sets up all aspects of the mandle EXCEPT for:

       * the cell pointing to the MIR object that it is to describe ("ex_add"),
       * any synonym ("synonym"),
       * any page buffer ("page") or associated virtual masa ("vir_add")
         to contain the Non-Terminal (if paging).

   It is up to the caller to load these final cells in the mandle.

*/

{
mandle  *local_mandle;  /* Local pointer to mandle being returned   */ 
mandle  *local_mclass;  /* Local pointer to mandleclass being used  */

/* if (mandle pointer is NULL) */
if (*obj_mandle == NULL) {

    LOCK_MANDLES();     /* Acquire mutex in threaded enviroment for lists */

    /* if (free mandle list is not empty) */
    if (fml != NULL) {
        /* allocate a mandle from the free list */
        local_mandle = fml;
        fml = local_mandle->next;
        local_mandle->tthis = local_mandle;  /* Show it legal now */

        /* set the passed pointer to point to it */
        *obj_mandle = local_mandle;
        }

    else {
        /* if (attempt to allocate storage for mandle succeeded) */
        if ( (local_mandle = (mandle *) malloc(sizeof(mandle))) != NULL) {
            /* set the passed pointer to point to it */
            *obj_mandle = local_mandle;
            local_mandle->tthis = local_mandle;  /* Show it legal now */
            }
        else {
            UNLOCK_MANDLES();           /* Release lists' mutex */
            return (MS_NO_MEMORY);
            }
        }

    /* if (mandle_class pointer is NULL) */
    if (*mclass == NULL) {

        /* if (free mandle list is not empty) */
        if (fml != NULL) {
            /* allocate a mandleclass from the free list */
            local_mclass = fml;
            fml = local_mclass->next;
            local_mandle->tthis = local_mandle;  /* Show it legal now */
            }

        else {
            /* if (attempt to allocate storage for mandle class succeeded) */
            if ( (local_mclass = (mandle *) malloc(sizeof(mandle))) == NULL) {
                UNLOCK_MANDLES();           /* Release lists' mutex */
                return (MS_NO_MEMORY);
                }
            }

        /* put the new mandle class on the mandle class list */
        local_mclass->next = mcl;    /* Make new one point properly       */
        local_mclass->prev = NULL;
        if (mcl != NULL) mcl->prev = local_mclass;  /* If prev, adjust    */
        mcl = local_mclass;          /* Fix list header to pt-> new       */
        local_mclass->cclass = NULL;  /* NULL=This is a MandleClass struct */
        local_mclass->tthis = local_mclass;  /* For error checking         */
        local_mclass->m.mc.top = NULL;      /* No mandles in class yet    */

        /* set the passed pointer to point to it */
        *mclass = local_mclass;
        }

    else {
        /* Make our local pointer jibe with what we were passed */
        local_mclass = *mclass;
        }

    /* hook the new mandle into the mandle class we've got */
    local_mandle->cclass = local_mclass;         /* show what class mandle in */
    local_mandle->next = local_mclass->m.mc.top;/* Hook it into class list   */
    local_mandle->prev = NULL;                  /* (Indicates "top")         */

    /* Reset "Last Match" entry to "none" */
    local_mandle->m.m.last_match = -1;          /* "Last-Match was NONE"     */

    /* Show no synonym */
    local_mandle->m.m.synonym = 0;

    /* Show no page buffer */
    local_mandle->m.m.page = NULL;
    local_mandle->m.m.vir_add = 0;

    if (local_mclass->m.mc.top != NULL)         /* Change any 2nd on list    */
        local_mclass->m.mc.top->prev = local_mandle;
    local_mclass->m.mc.top = local_mandle;      /* Make mclass top--> new    */

    UNLOCK_MANDLES();           /* Release lists' mutex */
    }

else {
    /* if (mandle is not valid) */
    if ((*obj_mandle)->tthis != (*obj_mandle)) {
        return (MS0_INVALID_MANDLE);
        }
    else {
        /* Reset "Last Match" entry to "none" */
        (*obj_mandle)->m.m.last_match = -1;

        /* Show no synonym */
        (*obj_mandle)->m.m.synonym = 0;

#ifdef PAGE_TRACE
        fprintf(PT_FILE, "\nPT: (...in set_mandle(): re-using mandle..)\n");
#endif

        LOCK_MANDLES();
        RELEASE_ANY_PAGE_BUFFER(*obj_mandle);
        UNLOCK_MANDLES()
        }
    }

return (MS_SUCCESS);
}

/* setup_next - Step "Next" thru Index to Non-Terminal & Setup to Return it */
/* setup_next - Step "Next" thru Index to Non-Terminal & Setup to Return it */
/* setup_next - Step "Next" thru Index to Non-Terminal & Setup to Return it */

static mir_status
setup_next (callers_slice, callers_slice_entry)

masa    *callers_slice;         /* -> to Slice containing entry */
masa    *callers_slice_entry;   /* -> to Entry in Slice         */

/*
INPUTS:

    "callers_slice" is the address of a masa of a slice containing an entry that
    we must "step" next "from".

    "callers_slice_entry" is the address of a masa of the entry.

    The addresses are passed so new values can be set into these parameters
    on an MS_SUCCESS return.

OUTPUTS:

The function returns one of:

    MS_SUCCESS - The "next" Non-Terminal object registered in the index
        was successfully found and "slice" and "slice_entry" arguments were
        set appropriately to indicate the location in the index of that
        "next" object.

    MS0_FIND_NONE - There is no "next" non-terminal registered in the index.


BIRD'S EYE VIEW:
    Context:
        The caller is "ret_mandle" and it needs to "step" its position
        to the "next" non-terminal object in the index before returning it.

    Purpose:
        This function takes care of the details of "walking" the index to
        find the next entry.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set up local copies of the slice and entry pointer>
    if (the slice entry points to a sub-register)
        <set slice pointer to point to slice in subregister>
        <set entry pointer to point to first entry in new slice>
        while (slice entry points at "slice")
            <set slice pointer to point to slice>
            <set slice entry to point to 1st in slice>
        <return slice & entry pointer values>
        <return MS_SUCCESS>
    else
        FOREVER
            if (entries remain in this slice)
                <increment the slice entry>
                while (slice entry points at "slice")
                    <set slice pointer to point to slice>
                    <set slice entry to point to 1st in slice>
                <return slice & entry pointer values>
                <return MS_SUCCESS>
            else
                if (slice has a non-zero backptr)
                    <record current slice as "old-slice">
                    <set slice ptr to backptr>
                    if (slice points at a subregister)
                        <set "old-slice" to subregister>
                        <set slice ptr to subregister's backptr>
                    if (find entry in slice that points at "old-slice" FAILED)
                        <return MS0_INDEX_CORRUPTED>
                    <set slice entry to point to it>
                    (*continue*)
                else
                    <return MS0_FIND_NONE>
        
OTHER THINGS TO KNOW:

    Awright, here's the scoop.  This function wades around in the raw
    binary representation of the MIR's index.  The index comprises
    the contents of two partitions in the MIR: the index and subregister
    partitions.  Each partition contains data structures of two different
    types, while the overall MIR "index" is made of both of these partitions,
    their data structures logically intermingled.

    A "slice" can be thought of as the list of all the arcs in a particular
    position in an OID **for a given prefix**.  Each entry in the list is
    termed a "slice entry".  Consider the following list of OIDs:

              a b c d
              1.2
              1.2.4.0
              1.2.4.10
              1.2.5

    Looking down slice "a" for prefix <null>,
                               it has one slice entry: "1".

    Looking down slice "b" for prefix "1",
                               it has one slice entry: "2".

    Looking down slice "c" for prefix "1.2",
                               it has two slice entries: one for "4", another
                                  for "5"

    Looking down slice "d" for prefix "1.2.4",
                               it also has two entries: one for "0", another
                                  for "10"

    Each slice entry contains two pieces of information in it:
      1) The arc number
      2) The MAS address of the next "thing" ("over") associated with that
         arc, either:
             - MASA of next slice (if more arcs exist in OID)
             - MASA of a subregister (see below)
             - MASA of a MIR Object (if this arc is the last in an OID)

    The index for a MIR with only those objects in it would have four slices
    and one subregister.

    The subregister takes care of the situation created by OID "1.2".  The
    slice entry carrying the "2" in slice "b" must not only carry a pointer
    for the MIR object represented by "1.2", but it must also point at the
    rest of the index for objects that have "1.2" as a prefix.  The rest
    of the index in this example starts with slice "c".

----

    This function steps "forward" one "object" in the index from the
    position it is given as input and returns that new position if there
    is another object at that position (ie if we're not at the end of the
    index).  We step from one registered object forward to the "next"
    registered object.  ("Object" in this discussion are MIR SMI objects
    that have OIDs in the index).

    This is complicated because we can have a situation where one
    object id "slice-entry" in a slice (that is, the right-most arc of a
    given OID) must serve to point both to an object registered in the index
    and also to a slice containing entries for other objects in the index
    that have the given OID as PART of their OID, as in:

                        t u v  w   --slice
             Object A:  1.2.4.10
                                    and
             Object B:  1.2.4.10.1

    where both objects A and B are registered in the index with the oids shown.

    In this instance, the slice with a slice entry of value "10" (w),
    representing the rightmost arc of 1.2.4.10 contains a masa that MUST point
    to a subregister.

    The "object masa" in the subregister points at object A.  The slice pointer
    in the subregister points to the slice that contains a slice-entry with
    value "1" (and masa pointer to object B).

    In "stepping-next" (what this function does) from object A's oid to
    object B's oid, the code must negotiate the subregister.  Also, if
    object B's oid was instead:

                       t u v  w x y z --slice
            Object A:  1.2.4.10
            Object B:  1.2.4.10.1.2.3

    ... the code must also negotiate "stepping down" through the first slice
    entry of each slice x, first entry of slice y to the first entry of slice z
    before encountering the first entry of slice z which points at an object
    (B) instead of another slice (as x and y do).

    Another source of complexity arises when the index is shaped like this
    and we must step from A's oid to B's:

                      t u v  w x y z --slices
            Object A: 1.2.4.10.1.2.3
            Object B: 1.2.4.11

    In this situation, the code must "walk-up" the back pointers of slices
    z, y, and x (in that order) to arrive at slice w before stepping to
    the next entry in w (from the one holding "10" to the one holding "11").

    All of the complexity of this routine derives from the fact that when
    traversing the index, we must deal with not only the "slice" structure
    (when "the next" oid in the index does not have the same number of arcs
    in it as "the previous" one did) but also the subregisters that appear
    when the next-previous have differing arc counts.

    The claim is made that this code correctly handles all cases.

    Function setup_previous() encounters the same challenges as this
    code does, except that setup_previous must deal with "falling off"
    the "low-end" of the current slice (as we step backwards) while
    "setup_next" must deal with falling off the high end.  This leads
    to a slightly different ordering of checks, but the challenges are
    exactly the same.

------------------------------------------------------------------------------

As you read this code, you'll need these pictures of the way data are laid
out in the two partitions that make up the index, the slice and subregister
partitions.  These layouts are defined by the code that writes the final
"externalized" binary representation of the compiled data in "mir_external.c".

(The fact that the slices and subregisters are in different partitions only
matters in the code in this function because the macros "IS_A_xxxx" use the
fact that the slices and subregisters are in different partitions in order
to be able to return a result indicating what a masa points at (slice,
subregister or non-terminal)).

======
SLICES (are defined as follows)
======

A Slice is a series of contiguous 4-byte integer "cells" in the "MIR Address
Space".  (Think of the "MAS" as an array of integers, the index into the
array being a "masa").  If "i" is the masa of the start of a slice, (and
where "masa offset" means offset from the start OF THE SLICE), a slice
consists of:

i+masa
offset    (contains)
-------  -------------------------
    +0   <backpointer masa>           (masa of "previous slice")
    +1   <entry count>                (count of entries following)

(...start of first entry...see below).
    +2   <arc number of 1st entry>    |
    +3   <masa of associated thing>   | ... repeats for each slice entry...
                                          (a slice entry is also known as a
                                                      "slot")


The "backpointer" is zero (illegal masa) for the slice containing the
arcs in the leftmost position of all the oids in the index.  A zero backpointer
means "top of the index".

For all other slices, backpointer points at EITHER a slice, OR a subregister.
Consider:

    1.3.2.10

The slice containing an entry with an arc-number of "1" (for the leftmost arc
shown above) will contain a backpointer of zero (there is no leftmost arc from
"1"). In:

    1.3.2.10
    1.3.2.10.5

...the slice that contains an entry with arc-number "5" will have a backpointer
that points AT THE SUBREGISTER pointed at by the slice that contains a slice
entry with arc-number "10" (because of the structure of the index).  (The
backpointer of the subregister points at the slice containing the entry
for "10").  (See the example below to ferret out the meaning of this English).


=============
SLICE ENTRIES (part of a slice)
=============

(Repeating a portion shown above)
A slice entry (also known as a "slot") is two mas (MIR-Address Space) cells.

The offset below is from the start of the *ENTRY* (SLOT)... not the slice in
which the entry is found:

  masa
offset       (contains)
------   -------------------------
    +0   <arc number>                 (ie a "2" for the two in an oid "1.3.2")
    +1   <masa of associated thing>   (thing is another slice, subreg or
                                       object)

    case 1:
            1.3.2   <--slice entry corresponding to "2" here points at a
            1.3.2.1          subregister

    case 2:
            1.3.2   <--slice entry correspending to "2" here points at a
            1.3.3            non-terminal object (directly, no subregister)

    case 3:
            1.3.2.4 <--slice entry corresponding to "2" here points at the
                             slice containing an entry for the next arc, 4.


============
SUBREGISTERS (are defined as follows)
============

A subregister consists of three mas cells: (offset shown from beginning of
subregister)

  masa
offset  contents
------  -------------------------
    +0  <backpointer>          (always points at the slice "up" (back))
    +1  <object masa>          (always points an a non-terminal)
    +2  <slice masa>           (always points at next slice "down")


Example:

=============
MIR contains:
=============
Obj   slices
      t u v w
      -------
A:    1.3.2
B:    1.3.2.1
C:    1.3.2.2
D:    1.3.3

===============
Index contains:
===============
[Annotation (*) means "the masa of object *"]
[Annotation "sr(name)" means "subregister of (name)"] 

                t         u           v           w
--------------------------------------------------------------------------
backpointer |   0  |   | (t) |   |   (u)  |   | (of sr(a))|
count       |   1  |   |  1  |   |    2   |   |     2     |
            |      |   |     |   |        |   |           |
arc         |   1  |   |  3  |   |    2   |   |     1     |
masa        |  (u) |   | (v) |   | (sr(a))|   |    (B)    |
            *------*   *-----*   |        |   |           |
arc                              |    3   |   |     2     |
masa                             |   (D)  |   |    (C)    |
                                 *--------*   *-----------*

Subregisters:
                   a
-------------------------
backpointer    |  (v)  |
object pointer |  (A)  |
slice pointer  |  (w)  |
               *-------*

To verify, scan the index for each object's oid by taking first arc of each,
scanning down the first slice for a match, follow pointer and repeat for
each arc until satisfied with masa of the object.  Objects B and C require
traversing subregister a, while obtaining Object A requires walking into
(but not out of) subregister a.  Object D's masa may be acquired without
traversing any subregisters.

The setup_previous and setup_next functions "walk" in their respective
directions given a masa of a slice and a masa of an entry within that slice.

V1.7:
Note that the original design of these routines presumed that the slice entry
specified a bona-fide non-terminal object (directly or via subregister).  With
the advent of the "GET_NEXT_ROLL" opcode for mir_oid_to_mandle(), this is
no longer completely true.  For V1.7, we allow 'setup_next()' to handle the
situation where it is given a bonafide 'slice' masa, but it may accept as
a slice-entry masa what WOULD BE the masa of a slice-entry immediately
following the last truly valid slice entry in the slice.  A single 'if' clause
is added to handle this situation, to allow GET_EXACT_ROLL to invoke this
function to roll to the next object if not even a partial match is possible.

*/

{
masa    slice;          /* Local copy of "callers_slice"        */
masa    slice_entry;    /* Local copy of "callers_slice_entry"  */
masa    i_masa;         /* Temporary masa for limit checks      */
masa    old_slice;      /* For keeping track of where we are    */

/* set up local copies of the slice and entry pointer */
/*
| These values are masas, direct addresses into the MIR address space
|  (mas).  "masa"s are exactly indexes into the "mas[]" array.
*/
slice = *callers_slice;
slice_entry = *callers_slice_entry;

/*
|  Here we deal with the special case where the caller has passed us a slice
|  (and a slice entry in it) whose "<masa of associated thing>" turns out to
|  be a pointer to a subregister.  The subregister itself contains two
|  forward pointers (described above).  Here we ignore the pointer to the
|  non-terminal ("<object masa>" because the caller actually wants us to
|  step *from* this non-terminal to the next) and we grab the <slice masa> and
|  "step down" into this next lower-level slice.
*/

/*
|  (Here we compute what the slice entry masa is of the last slice
|   entry in the slice so we can compare it to the 'current' entry.
|   V1.7: skip the subregister check if slice_entry is > last valid
|   (ie invalid)
*/
i_masa = slice + (mas[slice+1] * 2);   /* Address of last valid entry*/

/* if (slice entry is valid AND the slice entry points to a sub-register) */
if (slice_entry <= i_masa && IS_A_SUBREG(mas[slice_entry+1])) {

    /* set slice pointer to point to slice in subregister */
    slice = mas[mas[slice_entry+1]+2];

    /* set entry pointer to point to first entry in new slice */
    slice_entry = slice + 2;    /* (+2 skips backpointer & count) */

    /*
    | We've "stepped down" to the next slice and positioned ourselves to the
    | first slice entry in it.  We now (repeatedly) examine this first entry
    | to see whether it points to a slice.  We (repeatedly) "step down" on
    | this first entry, in essence descending at each first opportunity until
    | we encounter a slice whose first entry *does not* point at a lower slice.
    | Since slice entries may point at only one of three things (another lower
    | slice, a subregister or directly at a non-terminal), once we've reached
    | a slice entry that *does not* point at another lower slice, then we've
    | finished: we've hit the "next" MIR object in the index (in the form of
    | a subregister whose object it is we're after) or the direct "next" MIR 
    | non-terminal object.
    */

    /* while (slice entry points at "slice") */
    while (IS_A_SLICE(mas[slice_entry+1])) {
        /* set slice pointer to point to slice */
        slice = mas[slice_entry+1];

        /* set slice entry to point to 1st in slice */
        slice_entry = slice + 2;    /* skip backpointer and count */
        }

    /*
    | Note here we are returning the "coordinates" of a slice ENTRY that
    | points directly at the next non-terminal or indirectly at the next
    | non-terminal (through a subregister).  The "coordinates" are a masa of
    | slice and masa of slice-entry within that slice.
    */

    /* return slice & entry pointer values */
    *callers_slice = slice;
    *callers_slice_entry = slice_entry;

    return (MS_SUCCESS);
    }

else {
    /*
    |  The slice and entry passed are assumed to point directly to the
    |  non-terminal object that the caller wants "the next" one after.
    |  (At this point, we know the entry doesn't point to a subregister).
    |
    |  (With the advent of 'GET_EXACT_ROLL' though, this is not necessarily
    |  true, the input entry is simply the entry from which we want to 'roll'
    |  to the next valid MIR object registered.  The processing remains the
    |  almost the same).
    |
    |  -- If there is another valid entry beyond the specified one in the
    |  slice, then the logic is exactly that above for a subregister:
    |  step to it, walk "down" as far as there are slices, return first
    |  entry that describes a "non-slice".
    |
    |  -- If there are no other valid entries beyond "this one" in this slice,
    |  do the ELSE logic.
    */

    /* Each time around this loop, we're considering a new slice and a
    |  *particular* entry in that slice.  On entry the first time, we're
    |  considering the slice and slice-entry passed in by the caller.
    */
    /* FOREVER */
    for (;;) {

        /*
        |  (Here we compute what the slice entry masa is of the last slice
        |   entry in the slice so we can compare it to the 'current' entry.
        */
        i_masa = slice + (mas[slice+1] * 2);   /* Address of last valid entry*/

        /* if (entries remain in this slice) */
        if (slice_entry < i_masa) {

            /* increment the slice entry */
            slice_entry += 2;   /* (Each entry 2 cells wide) */

            /* while (slice entry points at "slice") */
            while (IS_A_SLICE(mas[slice_entry+1])) {

                /* set slice pointer to point to slice */
                slice = mas[slice_entry+1];

                /* set slice entry to point to 1st in slice */
                slice_entry = slice + 2;
                }

            /* return slice & entry pointer values */
            *callers_slice = slice;
            *callers_slice_entry = slice_entry;

            return (MS_SUCCESS);
            }

        else {
            /*
            |  No entries in the current slice beyond the current entry remain
            |  to be examined.  We undertake to "back-up" to the previous
            |  slice in order to consider the entry in that previous slice
            |  *immediately after* the entry that points to the current slice
            |  we're abandoning.
            |
            |  NOTE: offset from slice-start to a slice's backptr is 0,
            |        hence 'mas[slice+0]' is the backpointer masa.
            */
            /* if (slice has a non-zero backptr) */
            if (mas[slice] != 0) {

                /* record current slice as "old-slice" */
                old_slice = slice;

                /* set slice ptr to backptr */
                slice = mas[slice];

                /*
                | If the slice we're abandoning on our 'upward journey' had
                | been reached via a subregister, we 'step over' the
                | subregister here while re-recording the masa of the 'old
                | slice' needed for later processing.  In this case, 'old
                | slice' becomes the address of the subregister.
                */
                /* if (slice points at a subregister) */
                if (IS_A_SUBREG(slice)) {

                    /* set "old-slice" to subregister */
                    old_slice = slice;

                    /* set slice ptr to subregister's backptr */
                    slice = mas[slice];
                    }

                /*
                | We've reached the previous slice.  We must scan it to find
                | the entry that points down to the ('old') slice we just 
                | 'stepped-up out-of' (in order to consider the entry
                | immediately following it (if any!)).
                */
                /* Scan the entry for a match */
                for (slice_entry = slice + 2;
                     slice_entry <= (slice + (mas[slice+1] * 2));
                     slice_entry += 2)
                    {
                    if (mas[slice_entry+1] == old_slice)  break;
                    }

                /* if (find entry in slice that points at "old-slice" FAILED) */
                if (slice_entry > (slice + (mas[slice+1] * 2))) {
                    return (MS0_INDEX_CORRUPTED);
                    }

                /*
                | OK! 'slice' and 'slice_entry' now indicate the next entry
                | to be considered "from the top". . . fall thru and loop back
                | to perform the checks needed to cover all possible situations
                | again.
                */
                /* set "slice entry" to point to it (done by for loop above) */
                /*continue*/
                }
            else { /* There is no previous slice, nothing left to look at. */
                return (MS0_FIND_NONE);
                }
            }
        }
    }
}

/* setup_prev - Step "Previous" thru Index to Non-Terminal & Setup to Return  */
/* setup_prev - Step "Previous" thru Index to Non-Terminal & Setup to Return  */
/* setup_prev - Step "Previous" thru Index to Non-Terminal & Setup to Return  */

static mir_status
setup_prev(callers_slice, callers_slice_entry)

masa    *callers_slice;         /* -> to Slice containing entry */
masa    *callers_slice_entry;   /* -> to Entry in Slice         */

/*
INPUTS:

    "callers_slice" is the address of a masa of a slice containing an entry that
    we must "step" to the "previous".

    "callers_slice_entry" is the address of a masa of the entry.

    The addresses are passed so new values can be set into these parameters
    on an MS_SUCCESS return.

OUTPUTS:

The function returns one of:

    MS_SUCCESS - The "previous" Non-Terminal object registered in the index
        was successfully found and "slice" and "slice_entry" arguments were
        set appropriately to indicate the location in the index of that
        "previous" object.

    MS0_FIND_NONE - There is no "previous" non-terminal registered in the index.


BIRD'S EYE VIEW:
    Context:
        The caller is "ret_mandle" and it needs to "step" its position
        to the "previous" non-terminal object in the index before returning it.

    Purpose:
        This function takes care of the details of "walking" the index to
        find the prev entry.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set up local copies of the slice and entry pointer>
    FOREVER
        if (entry exists prior to current one)
            <decrement the slice entry>
            while (slice entry points at "slice" or "subregister")
                if (points at "slice")
                    <set slice pointer to point to slice>
                else
                    <set slice pointer to point to slice in Subreg>
                <set slice entry to point to LAST in slice>

            <return slice & entry pointer values>
            <return MS_SUCCESS>
        else
            if (slice has a non-zero backptr)
                <record current slice as "old-slice">
                <set slice ptr to backptr>
                if (slice points at a subregister)
                    <set "old-slice" to slice (the subregister)>
                    <set slice ptr to subregister's backptr>
                    <set "THRU S-R" TRUE to indicate "via subregister">
                else
                    <set "THRU S-R" FALSE to indicate "NOT via subregister">
                if (find entry in slice that points at "old-slice" FAILED)
                    <return MS0_INDEX_CORRUPTED>
                <set slice entry to point to it>
                if (we went "THRU S-R")
                    <return slice & entry pointer values>
                    <return MS_SUCCESS>
                (*continue*)
            else
                <return MS0_FIND_NONE>

OTHER THINGS TO KNOW:

    See the documentation associated with "setup_next()" in order to have
    a ghost of a chance of understanding the code in this function.

*/

{
masa    slice;          /* Local copy of "callers_slice"        */
masa    slice_entry;    /* Local copy of "callers_slice_entry"  */
masa    old_slice;      /* For keeping track of where we are    */
BOOL    thru_subreg;    /* TRUE: Backtracked via subregister    */

/* set up local copies of the slice and entry pointer */
slice = *callers_slice;
slice_entry = *callers_slice_entry;

/* Each time around this loop, we're considering a new slice and a
|  *particular* entry in that slice.  On entry the first time, we're
|  considering the slice and slice-entry passed in by the caller.
*/
/* FOREVER */
for (;;) {

    /* if (entry exists prior to current one) */
    if ( (slice_entry-2) >= (slice + 2) ) {

        /* decrement the slice entry */
        slice_entry -= 2;

        /*
        |  If this newly selected entry points to a slice or subregister,
        |  must do the right thing in each case in order to 'step down' to
        |  the last entry in the slice OR the last entry in the slice
        |  pointed-to by the subregister.  We do this repeatedly until we
        |  encounter a "last" entry that doesn't point to a subregister or
        |  slice. . . it must therefore point at a non-terminal: the 'previous'
        |  one.
        */
        /* while (slice entry points at "slice" or "subregister") */
        while (IS_A_SLICE(mas[slice_entry+1]) ||
               IS_A_SUBREG(mas[slice_entry+1])   ) {

            /* if (points at "slice") */
            if (IS_A_SLICE(mas[slice_entry+1])) {
                /* set slice pointer to point to slice */
                slice = mas[slice_entry+1];
                }
            else {
                /* set slice pointer to point to slice shown in Subreg */
                slice = mas[(mas[slice_entry+1]+2)];
                }

            /* set slice entry to point to LAST in slice */
            slice_entry = slice + (mas[slice+1]*2);
            }

        /*
        | Note here we are returning the "coordinates" of a slice ENTRY that
        | points directly at the previous non-terminal or indirectly at the
        | previous non-terminal (through a subregister).  The "coordinates"
        | are a masa of slice and masa of slice-entry within that slice.
        */

        /* return slice & entry pointer values */
        *callers_slice = slice;
        *callers_slice_entry = slice_entry;

        return (MS_SUCCESS);
        }

    else {
        /*
        | At this point, we've 'bumped-up' against the *beginning* of the
        | current slice.  We've got to 'back-up' to the previous slice in
        | order to consider the entry *immediately before* the entry that
        | points to the current slice we're abandoning.
        */
        /* if (slice has a non-zero backptr) */
        if (mas[slice] != 0) {

            /* record current slice as "old-slice" */
            old_slice = slice;

            /* set slice ptr to backptr */
            slice = mas[slice];

            /*
            | If the slice we're abandoning on our 'upward journey' had
            | been reached via a subregister, we 'step over' the
            | subregister here while re-recording the masa of the 'old
            | slice' needed for later processing.  In this case, 'old
            | slice' becomes the address of the subregister.  We also record
            | the fact that we passed thru it, since it contains the
            | next "prior" non-terminal we must return.
            */
            /* if (slice points at a subregister) */
            if (IS_A_SUBREG(slice)) {

                /* set "old-slice" to slice (the subregister) */
                old_slice = slice;

                /* set slice ptr to subregister's backptr */
                slice = mas[slice];

                /* set "THRU S-R" TRUE to indicate "via subregister" */
                thru_subreg = TRUE;
                }
            else {
                /* set "THRU S-R" FALSE to indicate "NOT via subregister" */
                thru_subreg = FALSE;
                }

            /*
            | We've reached the previous slice.  We must scan it to find
            | the entry that points down to the ('old') slice we just 
            | 'stepped-up out-of' (in order to consider the entry immediately
            | preceding it (which we'll do if we didn't step thru a
            | subregister)).
            */
            /* Scan the entry for a match */
            for (slice_entry = slice + 2;
                 slice_entry <= (slice + (mas[slice+1] * 2));
                 slice_entry += 2)
                {
                if (mas[slice_entry+1] == old_slice)  break;
                }

            /* if (find entry in slice that points at "old-slice" FAILED) */
            if (slice_entry > (slice + (mas[slice+1] * 2))) {
                return (MS0_INDEX_CORRUPTED);
                }
            /* set slice entry to point to it (done by the 'for' loop above) */

            /* 
            | If we stepped 'backward' thru a subregister, then the 'old slice'
            | was actually pointed-to by the subregister, and consequently, the
            | non-terminal object in the subregister is logically the
            | "previous" non-terminal sought.  So we return it by returning
            | the slice and entry that points *to the subregister*.
            */
            /* if (we went "THRU S-R") */
            if (thru_subreg == TRUE) {
                /* return slice & entry pointer values */
                *callers_slice = slice;
                *callers_slice_entry = slice_entry;
                return (MS_SUCCESS);
                }
            /*continue*/
            }
        else { /* There is no previous slice 'above', end-of-the-line */
            return (MS0_FIND_NONE);
            }
        }
    }
}

/* ret_mandle - Allocate and Return Mandle for proper object */
/* ret_mandle - Allocate and Return Mandle for proper object */
/* ret_mandle - Allocate and Return Mandle for proper object */

static mir_status
ret_mandle(style, this_slice, s_e_masa, obj_mandle, mclass, oid_smi)

get_style    style;         /* Style of the Lookup to be done           */
masa         this_slice;    /* External Address of next slice to search */
masa         s_e_masa;      /* masa Address of an entry in the slice    */
mandle       **obj_mandle;  /* ->> Mandle                               */
mandle_class **mclass;      /* ->> Mandle Class                         */
mir_oid_smi  *oid_smi;      /* -> Where to return OID SMI indicator     */

/*
INPUTS:

    "style" is the lookup strategy to be employed in searching the index,
    one of "GET_EXACT", "GET_NEXT", or "GET_PREVIOUS".  This argument
    determines whether the object specified by the "this_slice" and
    "s_e_masa" arguments is returned (GET_EXACT) or whether the object
    before (GET_PREVIOUS) or immediately after (GET_NEXT) is returned.

    "this_slice" is a masa for the start of the slice containing the entry
    whose Non-Terminal Object is to be returned via mandle.

    "s_e_masa" (slice-entry masa) is the masa of the entry in "this_slice"
    that points (either directly or via a subregister) to a Non-Terminal
    object to be returned...(it is the SMI of the OID for this object that is
    optionally returned in "oid_smi" below... the slice entry specifies the
    last arc in the OID for the object).

    "obj_mandle" is the address of a pointer to a mandle.  If this pointer
    is NULL, and an object is found in the MIR, then a new mandle is created
    to return the object.  This pointer is set to point to that mandle on
    return.  If the pointer is non-null, then it is taken as the address
    of a mandle to be re-used.

    "mclass" is the address of a pointer to a mandle class. If this pointer
    is NULL, and an object is found in the MIR, then when a new mandle is
    created to return the object, this pointer is set to point to that mandle's
    class on return.  If the pointer is non-null, then it is taken as the
    address of a mandle class into which any new mandle that is created is to
    be put.

    "oid_smi", if NON-NULL, points to a cell to receive the SMI indicator
    for the OID for the object being returned.  For MS0_FIND_EXACT, this
    will be the SMI of the OID used in the call to "mir_oid_to_mandle()".
    For all other return codes, it will be the SMI of the OID that was
    found in the index (for instance, for MS0_FIND_LONG, the OID in the
    index is shorter than the LONG input OID, but the SMI is that of the
    shorter OID found in the index).  Additionally note that if the "style"
    was GET_NEXT or GET_PREVIOUS, it is the SMI of the OID of the object
    actually returned that is returned.  (Ie, on a GET_NEXT, if the input
    OID was in SMI "SNMP", but the next OID in the index following is in
    SMI "DNA" then "DNA" is the SMI returned.

OUTPUTS:

The function returns one of:

MS_SUCCESS - The MIR object was successfully returned.

MS_NO_MEMORY - Insufficient memory available to allocate a mandle or
    mandle-class.

BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0 function that desires to return an object
        to the Tier 0 caller via a mandle.

    Purpose:
        This function acquires a mandle (and possibly a mandle-class)
        and returns an object through it to the Tier 0 functions' caller.


ACTION SYNOPSIS OR PSEUDOCODE:

    switch (style)
        case GET_EXACT:
        case GET_EXACT_ROLL:
            <break>
        case GET_NEXT:
            if (setup_next() did not return MS_SUCCESS)
                <return setup_next() status>
            <break>
        case GET_PREVIOUS:
            if (setup_prev() did not return MS_SUCCESS)
                <return setup_prev() status>
            <break>
        default:
            <return MS_NOT_YET_IMPLEMENTED>

    if (setup of mandle/mandle class failed)
        <return setup status>

    if (the slice entry object points at a Subregister)
        <obtain masa of object in subregister and put into  mandle>
        <substitute the subregister address (masa) as the "slice" address 
         sought in the SMI scan below>
        <substitute the subregister's object's address as the NT to search for
         the OID backpointer>
        <signal "skip count must be zero" TRUE>
    else
        <copy masa of object to mandle>
        <signal "skip count must be zero" FALSE>

    if (oid_smi indicator pointer is NOT NULL)
        <establish code-to-return as "OID_ANY": some kind of bug>
        <perform page-in for paged-Non-Terminals>
        <extract the number of OID backpointers from the object>
        <set skip count to 0>
        if (backpointer count is greater than 1 and skip count may NOT be zero)
            <compute MASA of first entry in slice>
            for (every prior slice entry in slice)
                if (slot points to current object)
                    <increment skip count>
        <establish address of first potential OID backpointer>
        while (backpointers remain to be examined)
            <extract the backpointer slice address>
            if (backpointer matches sought address)
                if (skip count is ZERO or less)
                    <extract the SMI indicator and store as code-to-return>
                    <break>
                else
                    <decrement skip count>
            <count another processed>
            <step to next possible OID backpointer>
        <return the code-to-return>

    <return MS_SUCCESS>

OTHER THINGS TO KNOW:

    See setup_next() documentation as an aid to understanding this code.

    V1.7 GET_EXACT_ROLL: This opcode is supported by special code in
    mir_oid_to_mandle(), and requires no special handling at this point.

    Note that for evaluating the SMI, this function does contain
    disambiguation logic that needed to take care of the
    "Match-Except-for-Last-Arc" case that the compiler specifically supports.
    See "mir_external.c"/"mir_t0.c" for discussion of OID-SMI disambiguation.
*/

{
mandle      *local_mandle;  /* Local pointer to mandle being returned   */ 
mir_status  s_u_status;     /* Setup status                             */
mir_oid_smi ctr;            /* Code-To-Return                           */
int         bptr_cnt;       /* Backpointer count                        */
masa        bptr_masa;      /* Extracted Backpointer for OID            */
int         skip_count;     /* Number of Backpointer entries to skip    */
BOOL        sc_must_be_0;   /* TRUE: Skip Count must be zero            */
masa        first_se;       /* MASA of first Slice Entry in the slice   */
masa        back_scan;      /* Used to scan backward to "first_se"      */
unsigned int *Mas;          /* Set to the Non-Terminal 'page' in memory */
                            /*   by SET_MAS_PAGE() macro call           */
masa        r_object_masa;  /* "Real" (unpaged) masa of object          */
masa        v_object_masa;  /* "Virtual" (paged) masa of object         */


/* Decide exactly which object in the index we'll return */
switch (style) {

    case GET_EXACT:
    case GET_EXACT_ROLL:
        break;

    case GET_NEXT:
        /* if (setup_next() did not return MS_SUCCESS) */
        if ( (s_u_status = setup_next(&this_slice, &s_e_masa)) != MS_SUCCESS) {
            return (s_u_status);
          }
        break;

    case GET_PREVIOUS:
        /* if (setup_prev() did not return MS_SUCCESS) */
        if ( (s_u_status = setup_prev(&this_slice, &s_e_masa)) != MS_SUCCESS) {
            return (s_u_status);
          }
        break;

    default:
        return (MS_NOT_IMPLEMENTED);
    }

/* if (setup of mandle/mandle class failed) */
if ((s_u_status = set_mandle (obj_mandle, mclass)) != MS_SUCCESS)
    return (s_u_status);

local_mandle = *obj_mandle;     /* Get local copy */

/*
| Grab Non-Terminal object or Subregister masa out of the 2nd cell in the
|  slice entry (+1)
*/
r_object_masa = mas[s_e_masa + 1];

/*
| We're killing two birds with one stone here:
|
| * Loading the mandle with the masa of the Non-Terminal object to be returned
| * Setting up to scan for the SMI of the OID if it needs to be returned
|   Extra setup is required here only for case whene the index slice entry
|   points a Subregister rather than directly at a Non-Terminal.
|
| if (the slice_entry points to a Subregister)
*/
if ( IS_A_SUBREG(r_object_masa) ) {

    /* obtain masa of object in subregister and put into mandle */
    local_mandle->m.m.ex_add = mas[r_object_masa+1];
    
    /*
    | Substitute the subregister's address (masa) as the "slice" address sought
    | in the SMI scan below (because OID backpointers in Non-Terminal objects
    | always point to index slices *unless* the OID is structured in such a
    | way that the last arc of it must be represented by a Subregister, in
    | which case the OID backpointer in the NT object points *at the
    | Subregister* rather than the index slice that contains the entry that
    | points at the Subregister).
    */
    this_slice = mas[s_e_masa+1];

    /*
    | Replace the Subregister's masa with the masa of the actual Non-Terminal
    | object.
    */
    r_object_masa = local_mandle->m.m.ex_add;

    /*
    | If the slice entry we are passed points at an object that is a 
    | Subregister, then there is no need to disambiguate.  (An object whose
    | last OID arc must be represented by a Subregister cannot possibly be
    | identical to another OID except for the last arc because we'd need a
    | slice, not a subregister to represent this situation).  Consequently we
    | can tell in advance that no OID backpointer entries need to be skipped.
    |
    | signal "skip count must be zero" TRUE
    */
    sc_must_be_0 = TRUE;
    }
else {
    /* copy masa of object to mandle */
    local_mandle->m.m.ex_add = r_object_masa;

    /*
    | We don't know yet whether the skip count will be zero, we'll have to
    | actually compute it if there is more than one OID backpointer in the
    | actual NT object.
    */
    sc_must_be_0 = FALSE;
    }

/*
| At this point, "this_slice" contains the address expected as an OID
| "backpointer" inside the NT object pointed to by "r_object_masa", while
| "s_e_masa" still points to the the slice entry in that slice that corresponds
| to the last arc in the OID for the object.
|
| NOTE: "this_slice" may point at
|       * an index slice (if sc_must_be_0 = FALSE) or
|       * a subregister (sc_must_be_0 = TRUE)
|
| but it should simply be regarded as a "backpointer" into the index somewhere.
*/
if (oid_smi != NULL) {

    /*
    | establish code-to-return as "OID_ANY":
    |       if this is returned, it's some kind of bug
    */
    ctr = OID_ANY;

    /* Perform 'page-in' for Non-Terminal
    |
    | We're about to examine the 'innards' of the Non-Terminal.  To cover the
    | situation where we might be open with 'paging', we force the page in and
    | do all references to it via "object_masa" and "Mas", both of which are
    | set (or reset) thru the call to "SET_MAS_PAGE()".  If Non-Paged, "Mas"
    | is set to "mas" and "object_masa" is set to the external address (that
    | would be used in mas[*]).
    |
    | NOTE WELL:  This "SET_MAS_PAGE()" invocation can execute a 'return'
    |             with any status that can be generated by a call to function
    |             "NT_page_in()" if paging is active and there is an I/O
    |             or other failure!
    */
    SET_MAS_PAGE(local_mandle, v_object_masa);


    /*================================= NOTE ==================================
    | From here on down through this code, we no longer reference "mas[]"
    | directly when referencing the Non-Terminal in question.  Instead we
    | reference "Mas[]" which has been set-up (along with "v_object_masa") to
    | contain what is needed.  At this point, "Mas[v_object_masa]" is the first
    | cell of the Non-Terminal specified by mandle local_mandle, (the Packed
    | Count Cell at the start of the Non-Terminal).
    |
    | PAGED:
    | "Mas[..]" contains a slug of a Non-Terminal partition starting with the
    |           local_mandle's Non-Terminal (ie "Mas[v_object_masa]" is the
    |           first cell of the Non-Terminal)
    | "mas[..]" contains the entire MIR Address Space **except** for the
    |           Non-Terminal (sub)partitions
    |
    | NON-PAGED:
    |  "Mas" has the same value in it as "mas", and everything in the MIR
    |   address space can be referenced via "Mas[]", in other words
    |   "Mas[v_object_masa]" references the same cell as "mas[r_object_masa]",
    |   (and v_object_masa == r_object_masa).
    */

    /* extract the number of OID backpointers from the object */
    bptr_cnt = ((unsigned int) Mas[v_object_masa]) >> 
                                                   mas[OID_COUNT_RSHIFT_MASA];

    /* set skip count to 0 */
    skip_count = 0;

    /* if (backpointer count is > than 1 and skip count may NOT be zero) */
    if (bptr_cnt > 0 && sc_must_be_0 == FALSE) {

        /*
        | At this point we are setting "skip_count" to the proper value.
        | All of the code in this if clause is devoted to doing this.
        |
        | By scanning the slice BACKWARD starting at the slice-entry that
        | specified the last arc in the OID, we count the number of slice
        | entries we need to skip (ie the number of entries that ALSO point
        | at the same Non-Terminal that the s_e_masa entry pointed to).
        |
        | Whatever number we come up with as a skip count is the number of
        | OID backpointers *in the Non-Terminal* (that points to the index
        | slice) that we must *skip* (in the while loop below) before settling
        | on the next OID backpointer (that points at the index slice) as the
        | OID whose SMI must be returned.
        | 
        | C'est si facile, n'est-ce pas?
        */

        /* compute MASA of first entry in slice
        |
        | In this case, we know "this_slice" really points at a Slice
        | (because sc_must_be_0 is FALSE), so the slice looks like:
        |
        | +0:  Backpointer
        | +1:  Slice Entry Count
        | +2:  <Start of 1st Slice Entry>
        */
        first_se = this_slice + 2;      /* Masa of first slice-entry        */

        /* for (every prior slice entry in slice) */
        for (back_scan = s_e_masa - 2;  /* Start w/one immediately prior    */
             back_scan >= first_se;     /* Don't go back beyond first entry */
             back_scan -= 2) {          /* Step back 1 slice-entry's width  */
            
            /*
            | Each slice-entry is:
            | +0: Arc number
            | +1: Masa of thing pointed to by this last arc
            |
            | if (slot points to current object)
            */
            if (mas[back_scan+1] == r_object_masa) {
                skip_count += 1;    /* increment skip count */
                }
            }
        }

    /* establish address of first potential OID backpointer */
    v_object_masa += 1;

    /* while (backpointers remain to be examined) */
    while (bptr_cnt-- > 0) {

        /* extract the backpointer slice address */
        bptr_masa = Mas[v_object_masa] & mas[OID_BPTR_AMASK_MASA];

        /* if (backptr matches sought address (index slice or index subreg) */
        if (bptr_masa == this_slice) {

            /* if (skip count is ZERO or less) */
            if (skip_count <= 0) {

                /* extract the SMI indicator and store as code-to-return */
                ctr = ((unsigned int) Mas[v_object_masa])
                            >> mas[OID_SMI_RSHIFT_MASA];

                /* NOTE: This "breaks" the "while", not the "if" ! */
                break;
                }

            else {
                skip_count -= 1;        /* decrement skip count */
                }
            }

        /* count another processed (done above) */

        /* step to next possible OID backpointer */
        v_object_masa += 1;
        }

    /* return the code-to-return */
    *oid_smi = ctr;
    }

/* Reset "Last Match" entry to "none" */
local_mandle->m.m.last_match = -1;

return (MS_SUCCESS);
}

/* mir_oid_to_mandle - ISO Object ID to Mandle Lookup */
/* mir_oid_to_mandle - ISO Object ID to Mandle Lookup */
/* mir_oid_to_mandle - ISO Object ID to Mandle Lookup */

mir_status
mir_oid_to_mandle(style, oid, obj_mandle, mclass, oid_smi)

get_style   style;          /* Style of the Lookup to be done       */
object_id   *oid;           /* -> Object Id structure to be found   */
mandle      **obj_mandle;   /* ->> Mandle                           */
mandle_class **mclass;      /* ->> Mandle Class                     */
mir_oid_smi  *oid_smi;      /* --> Where to return SMI of 'oid'     */

/*
INPUTS:

    "style" is the lookup strategy to be employed in searching the index,
    one of "GET_EXACT", "GET_EXACT_ROLL", "GET_NEXT", or "GET_PREVIOUS".

        GET_EXACT:      mir_oid_to_handle attempts to find an exact match
                        for the supplied "oid", searching for the longest
                        match if no exact match, or accepting the first
                        complete match if the supplied oid is too short.

        GET_EXACT_ROLL: performs the same processing as GET_EXACT, except
                        in the instance where no portion of the supplied oid
                        matches any portion of any MIR object oid, normally
                        GET_EXACT returns MS0_FIND_NONE (see below).  Under
                        the same circumstances, GET_EXACT_ROLL returns
                        MS0_FIND_ROLL and the MIR object whose oid immediately
                        follows the supplied oid in lexicographical order.
                        If there is no 'next' object it returns MS0_FIND_NONE.

        GET_NEXT:       performs the same processing as GET_EXACT, but once
                        the MIR object has been selected for return, GET_NEXT
                        actually returns the next MIR object that follows
                        the earlier selected one in lexicographical order or
                        MS0_FIND_NONE if there is no next object.

        GET_PREVIOUS:   performs the same processing as GET_EXACT, but once
                        the MIR object has been selected for return, 
                        GET_PREVIOUS actually returns the MIR object that
                        precedes the earlier selected one in lexicographical
                        order or MS0_FIND_NONE if there is no prior object.

    "oid" is the address of an object id structure containing the object
    id to be looked up in the MIR Index.

    "obj_mandle" is the address of a pointer to a mandle.  If this pointer
    is NULL, and an object is found in the MIR, then a new mandle is created
    to return the object.  This pointer is set to point to that mandle on
    return.  If the pointer is non-null, then it is taken as the address
    of a mandle to be re-used.

    "mclass" is the address of a pointer to a mandle class. If this pointer
    is NULL, and an object is found in the MIR, then when a new mandle is
    created to return the object, this pointer is set to point to that mandle's
    class on return.  If the pointer is non-null, then it is taken as the 
    address of a mandle class into which any new mandle that is created is
    to be put.

    "oid_smi" if non-NULL, is the address of where to return the SMI
    indicator for "oid" (that is, which SMI "oid" is defined in) WHEN
    the return status is such that something was found and returned
    (if, say, MS0_FIND_NONE is returned, this value is not returned regardless
    of whether it is supplied non-NULL).  See the discussion of this argument
    as passed to "ret_mandle()".

    Note that in the mir_t0 'paged' version, supplying a non-NULL
    argument here will result in page activity that otherwise is avoided,
    so supply this argument only if you really need the information returned.

OUTPUTS:

The function returns one of:

MS0_DB_NOT_LOADED - The MIR Database File has not been loaded into memory
    by a call to mir_t0_init().


MS0_FIND_NONE - No Object by the specified Object ID was found in the MIR.

    This also implies that there were no Objects in the MIR whose complete
    Object ID matched any shorter portion of the specified Object ID.  (In
    such a case, MS0_FIND_SHORT is returned).

    No mandle is created or re-used and no mandle-class is created or used.


MS0_FIND_ROLL - On a call with style = GET_EXACT_ROLL, this code may be
    returned inlieu of MS0_FIND_NONE.  See discussion under INPUTS.


MS0_FIND_EXACT - An Object by the specified Object ID was found in the MIR.

    There may be Objects in the MIR whose complete Object ID matches a
    shorter portion of the specified Object ID and there may also be Objects
    in the MIR for which the specified Object ID is a shorter portion of
    their complete Object ID, however the only exactly matching MIR Object
    is returned.

    A mandle is created or re-used and mandle-class is created or used, if
    "oid_smi" was supplied non-NULL, the SMI indicator for the OID is
    returned.


MS0_FIND_SHORT - An Object was found in the MIR, but the "object_id" supplied
    was too short to fully specify the Object that was found.

    The MIR returns the first instance as it scans the internal index of
    objects that fully matches the supplied (short) "object_id".

    If "oid_smi" was supplied non-NULL, the SMI indicator for the OID is
    returned.


MS0_FIND_LONG - An Object was found in the MIR, but the "object_id" supplied
    was too long with respect to the Object that was found.  The Object in
    the MIR whose full Object Id matches as much as possible of the supplied
    "object_id" is returned.

    If "oid_smi" was supplied non-NULL, the SMI indicator for the OID is
    returned.


MS0_INVALID_MANDLE - A passed mandle address or pointer was invalid.

MS0_INVALID_OID - A passed address of an object id was invalid (null).

BIRD'S EYE VIEW:
    Context:
        The caller has an ISO Object ID and needs to look it up in the MIR.

    Purpose:
        This function performs the search of MIR index looking for an entry
        that meets the calling protocols's requirement for a match (EXACT,
        NEXT or PREVIOUS) and returns a handle to any found object via a
        "mandle".

ACTION SYNOPSIS OR PSEUDOCODE:

    <show "no previous object found">

    if (pointer to object ID is NULL)
        <return MS0_INVALID_OID>

    if (the mandle or mandle-class passed "address of pointer" is NULL)
        <return MS0_INVALID_MANDLE>

    if (pointer to MIR Address Space is NULL)
        <return MS0_DB_NOT_LOADED>

    <set up to scan the first slice of the index>

    if (oid has no arcs in it)
        if (request is GET_EXACT_ROLL)
            if (current entry is NON-TERMINAL or SUBREGISTER)
                <perform return through mandle on object>
                <return MS0_FIND_ROLL>
            else
                <walk the minimum path to first SR/NT object>
                <perform return through mandle on found object>
                <return MS0_FIND_ROLL>
        <return MS0_FIND_NONE>

    for (every number in the input ISO Object ID)

        for (each entry in the slice)
            if (entry arc < current input oid arc)
                <continue>

            else if (entry arc matches current input oid arc)

                if (Kind of Object was SLICE)
                    if (no more arcs in input OID)
                        <walk the minimum path to first SR/NT object>
                        <perform return through mandle on found object>
                        <return MS0_FIND_SHORT>
                    else
                        <set next object (slice) as next slice to scan>
                        <break>

                else if (kind of Object was NON-TERMINAL)
                    <perform return through mandle on NT object>
                    if (no more arcs in input OID)
                        if (they want OID SMI)
                            <attempt to return OID SMI>
                        <return MS0_FIND_EXACT>
      |             else
      |A                <return MS0_FIND_LONG>

                else if (kind of Object was SUBREGISTER)
                    if (no more arcs in input OID)
                        <perform return through mandle on SR object>
                        if (they want OID SMI)
                            <attempt to return OID SMI>
                        <return MS0_FIND_EXACT>
                    else
      |                 <record position of subregister's object for
      |                  last-found-during-descent exit>
      |B                <select next-lower-level as slice to scan next>
                        <break>
                else
                    <issue "Ill structured Index" error code>

            else    (NO MATCH)
      |         if (object was recorded during index descent)
      |C            <return last recorded object during descent, "_LONG">

                if (request is GET_EXACT_ROLL)
                    if (current entry is NON-TERMINAL or SUBREGISTER)
                        <perform return through mandle on object>
                        <return MS0_FIND_ROLL>
                    else
                        <walk the minimum path to first SR/NT object>
                        <perform return through mandle on found object>
                        <return MS0_FIND_ROLL>

                <return MS0_FIND_NONE>

            (* End of inner slice-scanning For loop *)

        if (no new slice was selected)

            if (object was recorded during index descent)
     |D         <return last recorded object during descent, "_LONG">

            else if (request is GET_EXACT_ROLL)
                if ('setup_next' on the 'object' beyond slice-end SUCCEEDED)
                    <perform return through mandle on new slice/entry>
                    <return MS0_FIND_ROLL>
                else
                    <return setup_next status>
            else
                <return MS0_FIND_NONE>


OTHER THINGS TO KNOW:

  The lookup strategy "GET_NEXT" and "GET_PREVIOUS" is not necessarily
  related to SNMP "GET-NEXT".  Both of these options function like this:

  * The code performs the same lookup action as "GET_EXACT"

  * If some MIR object gets found (either "exactly" or with a "short" or
    "long" match), then whatever object that was, the NEXT (GET_NEXT) or
    PREVIOUS (GET_PREVIOUS) MIR object in the index is the one actually
    returned!  The code returned (one of MS0_FIND_SHORT, MS0_FIND_LONG or
    MS0_FIND_EXACT) is the SAME one that would have been returned had the
    operation been simply "GET_EXACT", unless . . .

  * There is no NEXT or PREVIOUS, in which case MS0_FIND_NONE is returned.

  The names of the return codes were chosen to illuminate the relationship
  that the "search-key" OID stands in with respect to the OID of the object
  returned for *GET_EXACT*.  Clearly if you use GET_NEXT, then in this
  situation:

  1.3.1         MIR Object
  1.3.2.4       MIR Object
  1.3.2.4.1     (Supplied "search-key" OID)
  1.4.1.1.1.1   MIR Object

  . . .the object returned is the last one (because the "get-exact" search
  function would "find" 1.3.2.4 on a "long" match, then GET_NEXT returns
  the next MIR object in the index.  The search key OID in this case is
  actually SHORTER than the OID of the object actually returned, although it
  was longer than the one "found" by the "get-exact" processing.

  This little anomaly can twist your head unless you understand it.  The best
  way of thinking about this is that the return code name always reflects
  how the "search-key" OID stands in relationship to that "substring" portion
  of the search-key OID that exactly matches the OID of the object returned.
  On second thought, maybe that is not useful; however it is true.

                                 - - - - - - - 

  The complexity of this function derives largely from the fact that we've
  got to handle these noxious subregisters and what they imply: that short
  portions of an OID can specify an object in the index AND a path to other
  objects in the index.

  There are complications when searching the index with an OID that does
  not exactly match anything in the index.  The following discussion presumes
  processing for a GET_EXACT request.

  If the supplied OID is too short to FULLY specify any object, this is simple,
  and MS0_FIND_SHORT or MS0_FIND_NONE is returned.

  Otherwise, an MS0_FIND_LONG is returned, but there is a distinction to be
  made between several circumstances when it is returned.

  Section B above records a possible object to return when a short part of the
  supplied OID happens to specify an object, but the index contains other
  entries that might match more of it.  The code records the each instance
  of such a match ("forgetting" the last match, there may be more than one
  such situation) before sections C or D return this recorded value.

  The returns from sections C and D pass back the last match recorded
  by section B and a "_LONG" indication.  In these two cases, "_LONG" means
  we "found something", kept on searching "rightward" through the index,
  failed to find an exact match and returned the last object we partly
  matched EVEN THOUGH THERE MAY HAVE BEEN other "shorter partial" matches.  

  Section A returns "_LONG" and the object found in the situation where the
  OID was too long and we "ran out" of index to search.  In this instance, we
  are assured that even though there is more to the supplied OID, there are no
  more slices in the index that we can search for a further match on it
  (because it would require a subregister in this position for this to occur).

  The motivation for this long discussion is simply to point out that
  it might turn out that we want to refine the "_LONG" return status a bit more
  and return different versions that indicate whether or not it is profitable
  to "chop and resubmit" if the caller is desperate to find all objects that
  share any part of a long OID as their registered OID.  As the situation
  stands now, this information is available to the code of this function, but
  not returned.

                                 - - - - - - - 

  With V1.7, support for GET_EXACT_ROLL is provided to allow proper support
  for SNMP requirements.  GET_EXACT_ROLL functions exactly as does GET_EXACT,
  unless the GET_EXACT processing results in a situation in which MS0_FIND_NONE
  would be returned.  (In this case, no portion of the supplied oid has
  completely matched any existing MIR object oid ie MS0_FIND_LONG cannot be
  returned nor can MS0_FIND_EXACT.  The supplied oid is long enough to have
  matched existing MIR oids had the arc values been different).

  GET_EXACT_ROLL 'rolls' to the next lexicographically greater MIR object and
  returns it's oid and MS0_FIND_ROLL.  In this situation no statement can be
  made about what portion of the oid is shared by the supplied oid and the oid
  of the returned MIR object.  If there is no lexicographically greater MIR
  object, MS0_FIND_NONE is returned.

*/


#define RETURN_MANDLE(s_e_a)                                                \
if ((m_p_status = ret_mandle(style, this_slice, s_e_a, obj_mandle,          \
                             mclass, oid_smi)) != MS_SUCCESS)               \
                                {                                           \
                                return (m_p_status);                        \
                                }

{
masa          this_slice;       /* External Address of next slice to search */
masa          this_slice_start; /* Saved starting value of "this_slice"     */
masa          s_e_masa;         /* masa Address of an entry in a slice      */
int           i;                /* General purpose index                    */
masa          j;                /* General purpose masa                     */
unsigned int  i_arc;            /* An input arc, extracted from input oid   */
mir_status    m_p_status;       /* Mandle Processing Status                 */
mir_status    s_u_status;       /* Setup status                             */
masa          saved_this_slice; /* Storage for recovery exit                */
masa          saved_j;          /* (likewise)                               */


/* show "no previous object found" */
saved_this_slice = saved_j = 0;

/* if (pointer to object ID is NULL) */
if (oid == NULL) {
    return (MS0_INVALID_OID);
    }

/* if (the mandle or mandle-class passed "address of pointer" is NULL) */
if (obj_mandle == NULL || mclass == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* if (pointer to MIR Address Space is NULL) */
if (mas == NULL) {
    return (MS0_DB_NOT_LOADED);
    }

/* set up to scan the first slice of the index */
this_slice = mas[ROOT_INDEX_MASA];

/*
| If the OID has no arcs in it at all, we handle that here as a special
| case.  For:
|
|       GET_EXACT -             Return MS0_FIND_NONE
|       GET_EXACT_ROLL -        Roll to the first object in the MIR
|                               and return MS0_FIND_ROLL
*/
if (oid->count == 0) {

    /* if (request is GET_EXACT_ROLL) */
    if (style == GET_EXACT_ROLL) {

        /*
        | Point ourselves to the first entry in the slice, just as would
        | be the case if we were inside the 2nd for loop below
        */
        j=this_slice+2;

        /*
        | If the entry that stopped us was a 'non-slice', then it
        | is the object we want to return (in one of two flavors).
        */
        /* if (current entry is NON-TERMINAL or SUBREGISTER) */
        if ( IS_A_SUBREG(mas[j+1]) || IS_A_NONTERM(mas[j+1])) {
            /* perform return through mandle on object */
            RETURN_MANDLE(j);

            return (MS0_FIND_ROLL);
            }
        else {
            /*
            | The slice entry that we're at is pointed to a slice,
            | so we 'walk the index' in the same style we use when
            | handle a _SHORT situation: walk down first-entries in
            | successively lower slices until we hit a first-entry
            | that doesn't point at a slice: its the object we want to
            | 'roll' to and return.
            */
            this_slice = mas[j+1];  /* Step down to next slice      */
            s_e_masa = 0;           /* Indicate we're walking down  */

            while (s_e_masa == 0) {
                /* Set j = masa of masa of next thing down */
                j = this_slice + 3;  
                if ( IS_A_SLICE(mas[j]) ) {   /* Slice ? */
                    this_slice = mas[j];
                    continue;
                    }
                /* 
                | If we fall thru, mas[j] ("this_slice") is masa of
                | either a Subregister or a Non-Terminal object.
                */
                s_e_masa = this_slice + 2; /* Select 1st entry */
                }

            /* perform return through mandle on found object */
            RETURN_MANDLE(s_e_masa);

            return (MS0_FIND_ROLL);
            }
        }

    /*
    | It wasn't a 'roll' situation (GET_EXACT_ROLL) and we hit no
    | partial matches on the oid before reaching this point, (obviously)
    | so there is absolutely nothing to return for GET_EXACT, GET_NEXT
    | or GET_PREVIOUS.
    */
    return(MS0_FIND_NONE);
    }

        
/* for (every number in the input ISO Object ID) */
for (i=0; i < oid->count; i++) {

    i_arc = oid->value[i];              /* Grab the next input oid arc      */

    this_slice_start = this_slice;      /* Record the slice we're scanning  */

    /* The slice in the MIR Address Space is composed of the following
    |
    |  (where the start of the slice (its masa) is added to):
    |  +0: Back Pointer to Previous Slice
    |  +1: Count of number of entries
    |      .... (Start of Entry)
    |  +2: Arc Value
    |  +3: External Address of Next (thing) "Down" (thing: slice, subreg or NT)
    |      ....  where there are "count" of these two-celled entries
    |
    |  in the loop below:
    |      "j" points at "slice entries"
    |      mas[j] is the arc value for that entry
    |      mas[j+1] is the masa of the thing attached to the arc in this
    |         entry
    |
    |      "this_slice" points at the top of the slice, to the
    |                  "Back Pointer" in the slice (as shown above)
    |
    |  We break from the loop below when we hit a match within the slice
    |  for the input arc we are currently considering ("i_arc").  Just
    |  before the break occurs, we select a new index slice to sweep down.
    */

    /* for (each entry [pointed to by "j"] in the slice) */
    for (j=this_slice+2;        /* "+2" skips backpointer & count of entries */
         j < (mas[this_slice+1]*2)+this_slice+2;
         j += 2) {              /* Each entry (slot) is 2 cells wide */

        /* if (entry arc < current input oid arc) */
        if ( mas[j] < i_arc) {
            continue;
          }

        /* else if (entry arc matches current input oid arc) */
        else if ( mas[j] == i_arc) {

            /* if (Kind of Object was SLICE) */
            if ( IS_A_SLICE(mas[j+1]) ) {

                /* if (no more arcs in input OID) */
                if ( (i+1) == oid->count) {

                    /* walk the minimum path to first SR/NT object          */
                    this_slice = mas[j+1];  /* Step down to next slice      */
                    s_e_masa = 0;           /* Indicate we're walking down  */

                    while (s_e_masa == 0) {
                        /* Set j = masa of masa of next thing down */
                        j = this_slice + 3;  
                        if ( IS_A_SLICE(mas[j]) ) {   /* Slice ? */
                            this_slice = mas[j];
                            continue;
                            }
                        /* 
                        |  If we fall thru, mas[j] ("this_slice") is masa
                        |  of either a Subregister or a Non-Terminal object.
                        */
                        s_e_masa = this_slice + 2; /* Select 1st entry */
                        }

                    /* perform return through mandle on found object */
                    RETURN_MANDLE(s_e_masa);
                    return (MS0_FIND_SHORT);
                    }
                else {
                    /* set next object (slice) as next slice to scan */
                    this_slice = mas[j+1];
                    break;
                    }
                }

            /* else if (kind of Object was NON-TERMINAL) */
            else if ( IS_A_NONTERM(mas[j+1]) ) {
                /* perform return through mandle on NT object */
                RETURN_MANDLE(j);

                /* if (no more arcs in input OID) */
                if ( (i+1) == oid->count ) {
                    return (MS0_FIND_EXACT);
                    }
                else
                    return (MS0_FIND_LONG);
                }

            /* else if (kind of Object was SUBREGISTER) */
            else if ( IS_A_SUBREG(mas[j+1]) ) {

                /* if (no more arcs in input OID) */
                if ( (i+1) >= oid->count) {
                    /* perform return through mandle on SR object */
                    RETURN_MANDLE(j);

                    return (MS0_FIND_EXACT);
                    }
                else {
                    /* record position of subregister's object for */
                    /* last-found-during-descent exit              */
                    saved_this_slice = this_slice;
                    saved_j = j;

                    /* select next-lower-level as slice to scan next
                    |  Here, mas[j+1]'s value is a masa of a SUBREGISTER
                    |  The SUBREGISTER looks like this:
                    |  +0: Backpointer
                    |  +1: masa of Subregister's Non-Terminal Object
                    |  +2: masa of Next Lower Level Slice
                    */
                    this_slice = mas[mas[j+1]+2];
                    break;
                    }
                }

            else {
                /* issue "Ill structured Index" error code */
                return (MS0_INDEX_CORRUPTED);
                }

            }  /* end of IF ARC MATCHES */

        else { /*   (NO EXACT MATCH)  */
            /*
            | We've hit an entry whose arc is greater than the arc currently
            | under consideration: no match situation.  If we hit something
            | enroute here, return it, otherwise on a GET_EXACT_ROLL, do the
            | roll.  We know that the entry that stopped us is the entry we
            | want to return in a roll situation.
            */
            /* if (object was recorded during index descent) */
            if (saved_this_slice != 0 ) {
                this_slice = saved_this_slice;
                RETURN_MANDLE(saved_j);

                /* return last recorded object during descent, "_LONG" */
                return(MS0_FIND_LONG);
                }

            /* if (request is GET_EXACT_ROLL) */
            if (style == GET_EXACT_ROLL) {

                /*
                | If the entry that stopped us was a 'non-slice', then it
                | is the object we want to return (in one of two flavors).
                */
                /* if (current entry is NON-TERMINAL or SUBREGISTER) */
                if ( IS_A_SUBREG(mas[j+1]) || IS_A_NONTERM(mas[j+1])) {
                    /* perform return through mandle on object */
                    RETURN_MANDLE(j);

                    return (MS0_FIND_ROLL);
                    }
                else {
                    /*
                    | The slice entry that stopped is pointed to a slice,
                    | so we 'walk the index' in the same style we use when
                    | handle a _SHORT situation: walk down first-entries in
                    | successively lower slices until we hit a first-entry
                    | that doesn't point at a slice: its the object we want to
                    | 'roll' to and return.
                    */
                    this_slice = mas[j+1];  /* Step down to next slice      */
                    s_e_masa = 0;           /* Indicate we're walking down  */

                    while (s_e_masa == 0) {
                        /* Set j = masa of masa of next thing down */
                        j = this_slice + 3;  
                        if ( IS_A_SLICE(mas[j]) ) {   /* Slice ? */
                            this_slice = mas[j];
                            continue;
                            }
                        /* 
                        | If we fall thru, mas[j] ("this_slice") is masa of
                        | either a Subregister or a Non-Terminal object.
                        */
                        s_e_masa = this_slice + 2; /* Select 1st entry */
                        }

                    /* perform return through mandle on found object */
                    RETURN_MANDLE(s_e_masa);

                    return (MS0_FIND_ROLL);
                    }
                }

            /*
            | It wasn't a 'roll' situation (GET_EXACT_ROLL) and we hit no
            | partial matches on the oid before reaching this point, so 
            | there is absolutely nothing to return for GET_EXACT, GET_NEXT
            | or GET_PREVIOUS.
            */
            /* return MS0_FIND_NONE */
            return(MS0_FIND_NONE);
            }

        } /* for each entry in the slice */

    /*
    | If we fall out of the innermost for loop, it is either because
    | we scanned the entire slice and found no match, or we "break"
    | with a new slice selected.  If no new slice was selected,
    | (ie "this_slice == this_slice_start") we're done looking through
    | the index.
    */
    if (this_slice == this_slice_start) {

        /* if (object was recorded during index descent) */
        if (saved_this_slice != 0) {

            /* return last recorded object during descent, "_LONG" */
            this_slice = saved_this_slice;
            RETURN_MANDLE(saved_j);

            /* return last recorded object during descent, "_LONG" */
            return(MS0_FIND_LONG);
            }
             /* if (request is GET_EXACT_ROLL) */
        else if (style == GET_EXACT_ROLL) { 

            /* if ('setup_next' on the 'object' beyond slice-end SUCCEEDED) */
            if ( (s_u_status = setup_next(&this_slice, &j)) == MS_SUCCESS) {

                /* perform return through mandle on new slice/entry */
                RETURN_MANDLE(j);

                return (MS0_FIND_ROLL);
                }
            else {
                return (s_u_status);
                }
            }
        /* ELSE */
        return (MS0_FIND_NONE);
        }
    }

/*
|  If we fall out of the loop that examines each arc in the OID, and we reach
|  here, something is seriously wrong, because each arc should be processed
|  into either a RETURN from inside the innermost 'for' loop, or the RETURN
|  from code following a drop from the innermost for loop.  Something is
|  gonzo if we manage to reach here.
*/
return(MS0_INDEX_CORRUPTED);
}

/*
| Here we include the source of local static function "get_oid_index()" that
| serves the functions in both "mir_t0.c" (ie. here) and "mir_symdump.c".
| This allows the object module produced from "mir_t0.c" to be wholely
| "self-sufficient", requiring no other object module to be linked with it.
|
| In essence, we're doing a compile-time link for the purposes of link-time
| (and kit generation) simplicity.
*/
#include "mir_goi.cinc"



/* mir_mandle_to_oid - MIR mandle to ISO Object ID Conversion */
/* mir_mandle_to_oid - MIR mandle to ISO Object ID Conversion */
/* mir_mandle_to_oid - MIR mandle to ISO Object ID Conversion */

mir_status
mir_mandle_to_oid(obj_mandle, oid, oid_smi)

mandle      *obj_mandle;    /* --> Mandle whose ISO Object ID is desired */
object_id   *oid;           /* Object ID structure to receive OID value  */
mir_oid_smi *oid_smi;       /* --> SMI's OID to be returned (or OID_ANY  */
                            /*     to receive first-available) or NULL   */
/*
INPUTS:

    "obj_mandle" is the address of the mandle whose ISO Object ID is
    requested.

    "oid" is the address of a object id storage area to receive the
    object id.  On successful return, the field named 'count' is set to
    the number of arcs returned and field named 'value' is set to point
    to heap storage containing the arc values themselves.  The caller
     is responsible for ultimately reclaiming this storage.

    "oid_smi" is the address of cell which on:

        INPUT:  If NULL, this means "give me an OID for the object,
                         I don't care which SMI it belongs to"

                If non-NULL, the value in the cell is taken to indicate the
                         SMI whose OID for this MIR Object is to be returned.

                         If the object has no OID assigned under the specified
                         SMI, MS0_FIND_NONE is returned.

                         If the input value is "OID_ANY", then the first
                         OID under any SMI for the object is randomly
                         chosen by this function and returned, and this
                         field is SET to the indicator that corresponds
                         to the SMI for the OID.  (In other words, we
                         choose the SMI and tell you what it is).

        OUTPUT:  If NULL, nothing is returned.

                 If non-NULL, the value is set (if MS_SUCCESS is returned)
                         to indicate the SMI of the OID returned as described
                         above.

OUTPUTS:

The function returns one of:

    MS_SUCCESS - The ISO object ID was successfully returned.  Storage
                 was allocated to hold the object ID arcs and a pointer to it
                 was placed in the "value" portion of the object ID structure.
                 The caller is responsible for releasing it.

                 If the "oid_smi" indicator was supplied and non-NULL, then
                 an indicator for the SMI of the OID is retured via "oid_smi".

    MS0_FIND_NONE - There is no OID assigned to the object (for the indicated
                    SMI, if "oid_smi" had a valid non-NULL value on input).

    MS0_INVALID_MANDLE - The pointer passed did not point to a valid mandle.

    MS_NO_MEMORY - Insufficient storage to return the ISO Object ID.

    MS0_INDEX_CORRUPTED -  MIR Database File Index is corrupted.

    MS0_INVALID_SMI - Input value for non-NULL "oid_smi" argument was bad.


BIRD'S EYE VIEW:
    Context:
        The caller wants to translate a mandle into the ISO Object ID for
        the object that that mandle represents in the MIR.

    Purpose:
        This function walks backpointers in the MIR database to infer
        the ISO Object ID that identifies the object.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (mandle pointer is NULL)
        <return MS0_INVALID_MANDLE>

    if (mandle pointer doesn't point at a mandle)
        <return MS0_INVALID_MANDLE>

    if (mandle pointer doesn't point at a Non-Terminal)
        <return MS0_INVALID_MANDLE>

    <perform 'page-in' operation if we're paging>

    <extract OID Backpointer count>
    if (Non-Terminal has no OID registered for it)
        <return MS0_FIND_NONE>

    if (attempt to allocate storage for largest OID in index fails)
        <return MS_NO_MEMORY>

    <set OID value pointer to point to storage allocated>

    if (SMI indicator is NULL)
        <set local requested OID SMI to "OID_ANY">
    else
        if (passed OID SMI value is legal)
            <set local requested OID SMI to passed OID SMI value>
        else
            <return MS0_INVALID_SMI>

    <establish pointer to next OID backpointer to be analyzed>
    <show no OID slice backpointer extracted yet>
    while (backpointers remain to be scanned)
        <extract backpointer SMI indicator>
        if (requested is "OID_ANY" or requested matches extracted)
            <extract the OID slice backpointer>
            <break>
        <count another analyzed, step to next>

    if (no OID slice backpointer was extracted)
        <return MS0_FIND_NONE>

    <assume disambiguation "skip count" is zero>
    for (all previous backpointers)
        if (next previous backpointer matches selected one)
            <increment backpointer skip count by one>

    if (walk the index and set oid arcs via get_oid_index FAILED)
        <return get_oid_index status>
    <assign the count of arcs loaded by get_oid_index>

    if (passed OID SMI value is non-NULL)
        <return the extracted SMI code>

    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

    The caller is responsible for reclaiming the heap storage allocated
    on a successful (MS_SUCCESS) return that contains the arcs.

    Note that this function contains disambiguation logic that
    is needed to take care of the "Match-Except-for-Last-Arc" case
    that the compiler specifically supports.  See compiler source file
    "mir_external.c" for a discussion of this disambiguation issue.
*/

{
mir_status      walk_status;      /* Return status from walk attempt    */
unsigned int    index;            /* Index storage                      */
int             bptr_cnt;         /* Count of Backpointers in NT        */
int             bptrs_processed;  /* Count of Processed Backpointers    */
mir_oid_smi     local_req;        /* Local copy of OID SMI requested    */
mir_oid_smi     extracted_smi;    /* SMI extracted from OID Backpointer */
masa            bptr_masa;        /* masa of next backpointer to get    */
masa            slice_masa;       /* masa of Index slice for OID        */
int             skip_count;       /* count of back pointers to-skip     */
unsigned int *Mas;                /* Set to the Non-Terminal 'page' in  */
                                  /* memory by SET_MAS_PAGE() macro call*/
masa            r_object_masa;    /* "Real" (unpaged) masa of object    */
masa            v_object_masa;    /* "Virtual" (paged) masa of object   */


/* if (mandle pointer is NULL) */
if (obj_mandle == NULL)
    return (MS0_INVALID_MANDLE);

/* if (mandle pointer doesn't point at a mandle) */
if (obj_mandle->tthis != obj_mandle)
    return (MS0_INVALID_MANDLE);

/* Grab the Real MASA */
r_object_masa = obj_mandle->m.m.ex_add; 

/* if (mandle pointer doesn't point at a Non-Terminal) */
if (!IS_A_NONTERM(obj_mandle->m.m.ex_add))
    return (MS0_INVALID_MANDLE);

/* Perform 'page-in' for Non-Terminal
|
| We're about to examine the 'innards' of the Non-Terminal.  To cover the
| situation where we might be open with 'paging', we force the page in and
| do all references to it via "object_masa" and "Mas", both of which are
| set (or reset) thru the call to "SET_MAS_PAGE()".  If Non-Paged, "Mas"
| is set to "mas" and "object_masa" is set to the external address (that
| would be used in mas[*]).
|
| NOTE WELL:  This "SET_MAS_PAGE()" invocation can execute a 'return'
|             with any status that can be generated by a call to function
|             "NT_page_in()" if paging is active and there is an I/O
|             or other failure!
*/
SET_MAS_PAGE(obj_mandle, v_object_masa);


/*================================= NOTE ==================================
| From here on down through this code, we no longer reference "mas[]"
| directly when referencing the Non-Terminal in question.  Instead we
| reference "Mas[]" which has been set-up (along with "v_object_masa") to
| contain what is needed.  At this point, "Mas[v_object_masa]" is the first
| cell of the Non-Terminal specified by mandle local_mandle, (the Packed
| Count Cell at the start of the Non-Terminal).
|
| PAGED:
| "Mas[..]" contains a slug of a Non-Terminal partition starting with the
|           local_mandle's Non-Terminal (ie "Mas[v_object_masa]" is the
|           first cell of the Non-Terminal)
| "mas[..]" contains the entire MIR Address Space **except** for the
|           Non-Terminal (sub)partitions
|
| NON-PAGED:
|  "Mas" has the same value in it as "mas", and everything in the MIR
|   address space can be referenced via "Mas[]", in other words
|   "Mas[v_object_masa]" references the same cell as "mas[r_object_masa]",
|   (and v_object_masa == r_object_masa).
*/

/* extract OID Backpointer count */
bptr_cnt = ((unsigned int) Mas[v_object_masa])
                         >> mas[OID_COUNT_RSHIFT_MASA];

/* if (Non-Terminal has no OID registered for it) */
if (bptr_cnt == 0)
    return (MS0_FIND_NONE);

/* if (attempt to allocate storage for largest OID in index fails) */
if ((oid->value =
        (unsigned int *) malloc(maxoid_arcs * sizeof(unsigned int))) == NULL)
    return (MS_NO_MEMORY);

/* set OID value pointer to point to storage allocated --- done above */
oid->count = 0;

/* if (SMI indicator is NULL) */
if (oid_smi == NULL) {
    /* set local requested OID SMI to "OID_ANY" */
    local_req = OID_ANY;
    }
else {
    /* if (passed OID SMI value is legal) */
    if (*oid_smi == OID_ANY || *oid_smi < MAX_OID_SMI) {
        /* set local requested OID SMI to passed OID SMI value */
        local_req = *oid_smi;
        }
    else {
        return (MS0_INVALID_SMI);
        }
    }

/* establish pointer to next OID backpointer to be analyzed */
bptr_masa = v_object_masa + 1;

/* show no OID slice backpointer extracted yet */
slice_masa = 0;

/* while (backpointers remain to be scanned) */
for (bptrs_processed = 0;
     bptrs_processed < bptr_cnt;
     bptrs_processed += 1) {

    /* extract backpointer SMI indicator */
    extracted_smi = ((unsigned int) Mas[bptr_masa])
                        >> mas[OID_SMI_RSHIFT_MASA];

    /* if (requested is "OID_ANY" or requested matches extracted) */
    if (local_req == OID_ANY || local_req == extracted_smi) {
        /* extract the OID slice backpointer */
        slice_masa = Mas[bptr_masa] & mas[OID_BPTR_AMASK_MASA];
        break;
        }
    /* count another analyzed, step to next */
    bptr_masa += 1;
    }

/* if (no OID slice backpointer was extracted) */
if (slice_masa == 0)
    return (MS0_FIND_NONE);

/*
|  This is the disambiguation logic required to take care of the case where
|  two OIDs are assigned to the same object and only differ in the final
|  OID arc value.  By convention the compiler writes the OID backpointers
|  into the NT object *in the order of their last arcs* when the OIDs differ
|  only by their last arcs.  So here, after we find the backpointer that
|  applies to the SMI the caller specified, we have to 'scan backward' over
|  any previous backpointers in the NT to count any backpointers that MATCH
|  the backpointer for the selected SMI.  The number of such matches is the
|  "skip-count", which will be passed to "get_oid_index()".  That function
|  skips however many "matches" it is told to skip (in the actual index slice)
|  before finding the match that corresponds to the last arc of the OID for
|  the desired SMI.
|
|  assume disambiguation "skip count" is zero
*/
skip_count = 0;

/* for (all previous backpointers) */
for (bptr_masa -= 1;
     bptrs_processed > 0;
     bptrs_processed -= 1, bptr_masa -= 1) {

    /* if (next previous backpointer matches selected one) */
    if ( (Mas[bptr_masa] & mas[OID_BPTR_AMASK_MASA]) == slice_masa) {
        
        /* increment backpointer skip count by one */
        skip_count += 1;
        }
    }

/* if (walk the index and set oid arcs via get_oid_index FAILED) */
if ( (walk_status =
        get_oid_index(mas,                          /* MIR Address space */
                      slice_masa,                   /* Slice (Subreg)    */
                      skip_count,                   /* Matches to skip   */
                      r_object_masa,                /* Object MASA       */
                      &index,                       /* index storage     */
                      oid->value)) != MS_SUCCESS) { /* oid storage       */
    return (walk_status);
    }

/* Assign the count of arcs loaded by get_oid_index */
oid->count = index;

/* if (passed OID SMI value is non-NULL) */
if (oid_smi != NULL) {
    /* return the extracted SMI code */
    *oid_smi = extracted_smi;
    }

return (MS_SUCCESS);

}

/* Comparison Function - Used in "bsearch()" call in "mir_search_rel_table" */
/* Comparison Function - Used in "bsearch()" call in "mir_search_rel_table" */
/* Comparison Function - Used in "bsearch()" call in "mir_search_rel_table" */
int
compar( a, b)

const void *a;        /* First argument  */
const void *b;        /* Second argument */
{
return ( (*(int *)a) - (*(int *)b) );
}



/* mir_search_rel_table - Search Non-Terminal Object Relationship Table */
/* mir_search_rel_table - Search Non-Terminal Object Relationship Table */
/* mir_search_rel_table - Search Non-Terminal Object Relationship Table */

mir_status
mir_search_rel_table(search, s_o_mandle, r_o_mandle, mclass, nonterm, term)

search_style    search;         /* Search Style to be used on rel. table    */
mandle          *s_o_mandle;    /* Mandle for Searched-Object               */
mandle          *r_o_mandle;    /* Mandle for Relationship-Object           */
mandle_class   **mclass;        /* Addr of ptr to mandle-class              */
mandle         **nonterm;       /* Addr of ptr to mandle                    */
mir_value       *term;          /* Addr of ptr to mir_value structure       */

/*
INPUTS:

    "search" indicates the style of search to be performed, one of
    SEARCH_FROMTOP or SEARCH_FROMLAST.

    "s_o_mandle" is the mandle describing the non-terminal whose relationship
    table is to be searched.

    "r_o_mandle" is the mandle describing the relationship sought in the
    s_o_mandle's relationship table.

    "mclass" and "nonterm" are used to return a mandle for a Non-Terminal
    that may be found as the target of the r_o_mandle relationship.

    "term" is the address of a user supplied "mir_value" structure used to
    return the Terminal object value that may be found as the target of the
    r_o_mandle relationship.


OUTPUTS:

The function returns one of:

    MS0_FIND_NONE - No match between the passed relationship object and the
        specified object to be searched was found in the relationship table.
        No mandle nor mandle-class is created or re-used and nothing is
        returned into the "term" mir_value structure.

    MS0_FIND_NONTERM - A match in the searched-object's relationship table was
        found, and a mandle describing the Target Object was returned.  An
        annotation is made in the searched-object's mandle indicating that
        this particularly relationship was the last successful match.  A
        subsequent search call may begin at this spot or start again from the
        top of the table depending on the value of the search_style argument
        supplied during that call.

    MS0_FIND_TERM - A match in the searched-object's relationship table was
        found, and the value of the Terminal Object was returned via the
        "term" mir_value structure.  An annotation is made in the
        searched-object's mandle indicating that this particularly
        relationship was the last successful match.  A subsequent search call
        may begin at this spot or start again from the top of the table
        depending on the value of the search_style argument supplied during
        that call.

    MS0_INVALID_MANDLE - One of the passed mandle pointers  was invalid.

    MS_NOT_YET_IMPLEMENTED - Search style argument is invalid.

    MS0_NT_CORRUPTED - Non-Terminal section of database file is corrupted.

    MS0_NOT_REL_MANDLE - Mandle for relationship object is not for a MIR
        Object that is a valid MIR Relationship


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 1 function or direct user call that desires to
        search a particular Non-Terminal Object's relationship table for
        a match, starting from a previous success-match in the table or from
        the top.

    Purpose:
        This function scans the table for the desired relationship match.
        and returns the target if there was one.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (search mandle address, relationship mandle address, terminal or
            nonterminal address is NULL)
        <return MS0_INVALID_MANDLE>

    if (search mandle or relationship mandle are invalid (this != mandle))
        <return MS0_INVALID_MANDLE>

    if (mandle-class pointer is valid and mandle is invalid or not a mandle
            class)
        <return MS0_INVALID_MANDLE>

    if (relationship mandle has no synonym for it)
        if (relationship mandle is not for a MIR Relationship Object)
            <return MS0_NOT_REL_MANDLE>
        else
            <compute synonym for object and store in mandle>

    switch (search)
        case SEARCH_FROMTOP:
            <set starting entry to 0>
            <break>
        case SEARCH_FROMLAST:
            <set starting entry to value in search mandle + 1>
            <break>
        default:
            <return MS_NOT_YET_IMPLEMENTED>
            <break>

    <perform 'page-in' for Non-Terminal>
    <compute number of entries left to check>
    <compute starting masa of 1st entry to check>
    while (entries remain to be checked)
         <extract synonym from entry>
         if (synonym of r_o matches entry synonym)
            <extract target masa from entry>
            if (masa of target is NONTERM)
                <return mandle thru set_mandle() call>
                if (ret_mandle() call succeeded)
                    <record successful match in search mandle>
                    <return MS0_FIND_NONTERM>
                else
                    <return ret_mandle() call status>
            else (it must be a terminal)
                if (masa of target is STRING)
                    <record successful match in search mandle>
                    <copy the length to the mir_value structure>
                    <copy the address of the string to the mir_value structure>
                    <return MS0_FIND_TERM>
                else if (masa of target is NUMBER)
                    <record successful match in search mandle>
                    <copy the number to the mir_value structure>
                    <return MS0_FIND_TERM>
                else
                    <return MS0_NT_CORRUPTED>

        else
            <step to the next entry>

    <return MS0_FIND_NONE>


OTHER THINGS TO KNOW:

    With V2.00 comes support for paging the Non-Terminals.  This is the only
    Tier 0 function (with the exception of the mir_oid_to_mandle() code that
    returns the OID SMI indicator) that actually examines Non-Terminals.

    Consequently a small change is made to this function to handle the
    paging requirements.  The basic code in this function remains unchanged,
    only a call to SET_MAS_PAGE plus some additional definitions are added.

*/

{
int         index;          /* Starting index where 1st compare should occur */
masa        i;              /* General Index masa                            */
int         full_count;     /* Count of number of entries in the rel. table  */
mir_status  s_u_status;     /* Setup-Mandle call status                      */
int         *syn_ptr;       /* For call to bsearch                           */
int         bptr_count;     /* Count of number of OID backpointers           */
int         entry_synonym;  /* Synonym extracted from (next) entry           */
masa        entry_target;   /* Target masa extracted from (next) entry       */
unsigned int *Mas;          /* Set to the Non-Terminal 'page' in memory by   */
                            /*   SET_MAS_PAGE                                */

/*
|  if (search mandle address, relationship mandle address or
|    terminal/nonterminal ptr is NULL)
*/
if (s_o_mandle == NULL || r_o_mandle == NULL || nonterm == NULL || term == NULL)
    return (MS0_INVALID_MANDLE);

/* if (search mandle or relationship mandle are invalid (this != mandle)) */
if ( (s_o_mandle->tthis != s_o_mandle) || (r_o_mandle->tthis != r_o_mandle))
    return (MS0_INVALID_MANDLE);

/*
|  if (mandle-class pointer is valid and mandle is invalid or not a mandle
|             class)
*/
if (mclass != NULL && *mclass != NULL &&
        ((*mclass)->cclass != NULL || (*mclass)->tthis != *mclass) )
    return (MS0_INVALID_MANDLE);

/* if (relationship mandle has no synonym for it) */
if (r_o_mandle->m.m.synonym == 0) {

    /* if (relationship mandle is not for a MIR Relationship Object) */
    if (!IS_A_NONTERM_REL(r_o_mandle->m.m.ex_add)) {
        return (MS0_NOT_REL_MANDLE);
        }
    else {
        /* compute synonym for object and store in mandle */
        if ((syn_ptr = (int *) bsearch(&(r_o_mandle->m.m.ex_add),  /* Sought */
                                      &masa_to_synonym[1],         /* Search */
                                      synonym_count,               /* Count  */
                                      sizeof(masa),                /* Keysize*/
                                      compar)) == NULL) {
            return (MS0_NOT_REL_MANDLE);
            }

        /*
        |  The math here is a little intense.  "syn_ptr" points to the "found"
        |  synonym entry.  We really want the ordinal position of that entry
        |  in the masa_to_synonym[] array.  So we subtract the first possible
        |  entry's address from the pointer and add "one" to get the ordinal
        |  position (ie the "FORTRAN array index") of the entry.
        */
        r_o_mandle->m.m.synonym =
            ( (int) (syn_ptr - &masa_to_synonym[1]) + 1);
        }
    }

switch (search) {
    case SEARCH_FROMTOP:
        /* set starting entry to 0 */
        index = 0;
        break;
    case SEARCH_FROMLAST:
        /* set starting entry to value in search mandle + 1 */
        index = s_o_mandle->m.m.last_match + 1;
        break;
    default:
        return (MS_NOT_IMPLEMENTED);
    }

/* Perform 'page-in' for Non-Terminal
|
| We're about to examine the 'innards' of the Non-Terminal.  To cover the
| situation where we might be open with 'paging', we force the page in and
| do all references to it via "i" and "Mas", both of which are set thru the
| call to "SET_MAS_PAGE()".  If Non-Paged, "Mas" is set to "mas" and "i"
| is set to the external address (that would be used in mas[*]).
|
| NOTE WELL:  This "SET_MAS_PAGE()" invocation can execute a 'return'
|             with any status that can be generated by a call to function
|             "NT_page_in()" if paging is active and there is an I/O
|             or other failure!
*/
SET_MAS_PAGE(s_o_mandle, i);


/*=================================== NOTE ====================================
| From here on down through this code, we no longer reference "mas[]" directly
| when referencing the Non-Terminal in question.  Instead we reference "Mas[]"
| which has been set-up (along with "i") to contain what is needed.  At this
| point, "Mas[i]" is the first cell of the Non-Terminal specified by mandle
| s_o_mandle, (the Packed Count Cell at the start of the Non-Terminal).
|
| PAGED:
| "Mas[..]" contains a slug of a Non-Terminal partition starting with the
|           s_o_mandle's Non-Terminal
| "mas[..]" contains the entire MIR Address Space **except** for the
|           Non-Terminal partitions
|
| NON-PAGED:
|  "Mas" has the same value in it as "mas", and everything in the MIR address
|  space can be referenced via "Mas[]".
*/

/* The Non-Terminal in the MIR Address Space is composed of the following
|  (where the start of the Non-Terminal (its 'real' or 'virtual' masa) is
|   added to):
|
|  +0: Count of OID Backpointers & Count of Rel. Table Entries (Packed)
|  +1: <Start of (packed) OID Backpointers & SMI Indicators, **IF ANY**>
|
|   *--("+2" below assumes 1 OID Backpointer @ "+1" above)
|   |
|   V   .... (Start of Entry)
|  +2: Relationship Object masa Synonym + Target Object masa (Packed)
|  +3: Relationship Object masa Synonym + Target Object masa (Packed)
|       .
|       .
|       .
|      ....  where there are "count" of these entries
|
| An example from a real dump with one OID backpointer and 9 entries is:
|
| Off
| Set Masa
| +0  3057: (Contents 0x10009)   OID Count...1   Rel-Entry Count...9
| +1  3058: (Contents 0x2000363) OID_DNA      (1.3.12.2.1011.2.1)-[867]
| +2  3059: 04 MIR_Text_Name          [2274]  "The World"
| +3  3060: 05 MIR_Special            [2388]  "V1.99   V1.01  10/30/92  (ddb)"
| +4  3061: 05 MIR_Special            [1232]  "DNA"
| +5  3062: 05 MIR_Special            [2193]  "SNMP"
| +6  3063: 05 MIR_Special            [870]   '0'
| +7  3064: 05 MIR_Special            [2130]  "PA"
| +8  3065: 05 MIR_Special            [1475]  "HISTORIAN"
| +9  3066: 05 MIR_Special            [1386]  "EXPORTER"
|+10  3067: 43 MIR_Cont_entityClass   [3319]  NT(Node)
|
| compute number of entries left to check
*/
full_count = Mas[i] & mas[ENTRY_COUNT_AMASK_MASA];
bptr_count = ((unsigned int) Mas[i]) >> mas[OID_COUNT_RSHIFT_MASA];

/* compute starting masa of 1st entry to check
| set i pointing to cell to start checking with:
| (start) + index + 1 (over count cell) + (number of OID backpointers)
|       where "index" is either 0 (ie SEARCH_FROMTOP) or last place we stopped
|             plus 1 (ie SEARCH_FROMLAST).
*/
i += index + 1 + bptr_count;

/* while (entries remain to be checked) */
while (index < full_count) {

    /* extract synonym from entry */
    entry_synonym = Mas[i] & mas[SYNONYM_AMASK_MASA];

    /* if (synonym of r_o matches entry synonym) */
    if (r_o_mandle->m.m.synonym == entry_synonym) {

        /* extract target masa from entry */
        entry_target = ((unsigned int) Mas[i]) >> mas[TARGET_RSHIFT_MASA];

        /* if (masa of target is NONTERM) */
        if (IS_A_NONTERM(entry_target)) {

            /* return mandle thru set_mandle() call */
            /* if (ret_mandle() call succeeded) */
            if ( (s_u_status = set_mandle(nonterm, mclass)) == MS_SUCCESS) {
                /* record successful match in search mandle */
                s_o_mandle->m.m.last_match = index;
                (*nonterm)->m.m.ex_add = entry_target;
                return (MS0_FIND_NONTERM);
                }
            else {
                /* return ret_mandle() call status */
                return (s_u_status);
                }
            }

        else  {/* (it must be a terminal) */

            /* if (masa of target is STRING) */
            if (IS_A_STRING(entry_target)) {

                /* record successful match in search mandle */
                s_o_mandle->m.m.last_match = index;

                /* copy the length to the mir_value structure */
                i = entry_target;           /* i is masa of string */
                term->mv.s.len = mas[i++];  /* Copy string length  */

                /* copy the address of the string to the mir_value structure */
                term->mv.s.str = (char *) &mas[i];    /* Set pointer to str  */
                term->mv_type = MIR_STRING;

                return (MS0_FIND_TERM);
                }

                 /* masa of target is Signed NUMBER */
            else if (IS_A_SNUMBER(entry_target)) {

                /* record successful match in search mandle */
                s_o_mandle->m.m.last_match = index;

                /* copy the number to the mir_value structure */
                i = entry_target;            /* i is masa of number      */
                term->mv.sn.number = mas[i]; /* Set number to mir_value  */
                term->mv_type = MIR_SNUMBER;

                return (MS0_FIND_TERM);
                }

                 /* masa of target is Unsigned NUMBER */
            else if (IS_A_UNUMBER(entry_target)) {

                /* record successful match in search mandle */
                s_o_mandle->m.m.last_match = index;

                /* copy the number to the mir_value structure */
                i = entry_target;            /* i is masa of number      */
                term->mv.un.number = mas[i]; /* Set number to mir_value  */
                term->mv_type = MIR_UNUMBER;

                return (MS0_FIND_TERM);
                }
            else {
                return (MS0_NT_CORRUPTED);
                }
            }
        }
    else {
        /* step to the next entry */
        index += 1;     /* Count another entry          */
        i += 1;         /* Step masa to start of next   */
        }
    }

return (MS0_FIND_NONE);

}

/* mir_free_mandle - Free Mandle for Re-Use */
/* mir_free_mandle - Free Mandle for Re-Use */
/* mir_free_mandle - Free Mandle for Re-Use */

mir_status
mir_free_mandle(a_f_mandle)

mandle          **a_f_mandle;    /* Address of ptr to Mandle to be freed */

/*
INPUTS:

    "a_f_mandle" - address of pointer to mandle to be freed.


OUTPUTS:

The function returns one of:

    MS_SUCCESS - The mandle was successfully freed.  The ptr (whose address
    is passed as an argument) is set to NULL;

    MS0_INVALID_MANDLE - The passed mandle pointer was invalid.


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 1 function or direct user call that desires to
        free a mandle created by a Tier 0 function.

    Purpose:
        This function frees the mandle by placing it on a free mandle list
        for later re-use.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (address of mandle pointer is NULL)
        <return MS0_INVALID_MANDLE>

    <copy mandle pointer locally>

    if (mandle pointer is NULL)
        <return MS0_INVALID_MANDLE>

    if (mandle is not a valid mandle)
        <return MS0_INVALID_MANDLE>

    LOCK_MANDLES()
    RELEASE_ANY_PAGE_BUFFER(for mandle)

    if (there is a "next" mandle)
        <make "next's" "previous" be set to freed "previous">

    if (mandle is at the top of the mandle list)
        <make mandle-class top point to next from freed mandle>        
    else
        <make previous mandle's "next" point at freed mandle's "next">

    <show freed mandle as "invalid">
    <insert freed mandle at top of the free mandle list>
    <show mandle pointer as invalid>

    if (the mandleclass is now empty)
        <blow off the mandle class itself>

    UNLOCK_MANDLES();
    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

    Note that this function acquires the mutex for the mandle lists before
    fooling with them.

MTHREADS:
    Double-note: when we try to blow the mandleclass in a threaded environment,
    an attempt will be made to acquire the mutex for a second time by the same
    thread (when mir_free_mandle_class() gets called from here).  If this
    croaks, then things will have to be re-arranged a bit.
*/

{
mandle          *f_mandle;      /* Local copy of pointer to mandle          */
mir_status      mir_st;         /* Status from mir_free_mandle_class() call */


/* if (address of mandle pointer is NULL) */
if (a_f_mandle == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* copy mandle pointer locally */
f_mandle = *a_f_mandle;

/* if (mandle pointer is NULL) */
if (f_mandle == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* if (mandle is not a valid mandle) */
if (f_mandle->tthis != f_mandle) {
    return (MS0_INVALID_MANDLE);
    }

LOCK_MANDLES();

#ifdef PAGE_TRACE
        fprintf(PT_FILE, "\nPT: (...in mir_free_mandle(): freeing mandle..)\n");
#endif

RELEASE_ANY_PAGE_BUFFER(f_mandle);

/* if (there is a "next" mandle) */
if (f_mandle->next != NULL) {
    /* make "next's" "prev" be set to freed "prev" */
    f_mandle->next->prev = f_mandle->prev;
    }

/* if (mandle is at the top of the mandle list) */
if (f_mandle->prev == NULL) {
    /* make mandle-class top point to "next" from freed mandle */
    f_mandle->cclass->m.mc.top = f_mandle->next;
    }
else {
    /* make previous mandle's "next" point at freed mandle's "next" */
    f_mandle->prev->next = f_mandle->next;
    }

/* show freed mandle as "invalid" */
f_mandle->tthis = NULL;

/* insert freed mandle at top of the free mandle list */
f_mandle->next = fml;
fml = f_mandle;

/* show mandle pointer as invalid */
*a_f_mandle = NULL;

/* if (the mandleclass is now empty) */
if (f_mandle->cclass->m.mc.top == NULL) {

    /* blow off the mandle class itself */
    if ((mir_st = mir_free_mandle_class(&(f_mandle->cclass))) != MS_SUCCESS) {
        UNLOCK_MANDLES();
        return (mir_st);
        }
    }

UNLOCK_MANDLES();

return (MS_SUCCESS);
}

/* mir_copy_mandle - Copy Mandle */
/* mir_copy_mandle - Copy Mandle */
/* mir_copy_mandle - Copy Mandle */

mir_status
mir_copy_mandle(a_mandle, new_mandle)

mandle          *a_mandle;      /* Address of Mandle to be copied  */
mandle          **new_mandle;   /* Address of pointer for new copy */

/*
INPUTS:

    "a_mandle" - address of mandle to be copied.
    "new_mandle" - address of pointer that will point to new mandle
                   on a successful return.

OUTPUTS:

The function returns one of:

    MS_SUCCESS - The mandle was successfully freed.

    MS_NO_MEMORY - Insufficent memory to complete the request.

    MS0_INVALID_MANDLE - The passed mandle pointer was invalid.


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 1 function or direct user call that desires to
        obtain a new mandle that points to the same MIR object as an existing
        mandle.

    Purpose:
        This function creates a new copy of the mandle passed in.



ACTION SYNOPSIS OR PSEUDOCODE:

    if (mandle pointer is NULL)
        <return MS0_INVALID_MANDLE>

    if (mandle is not a valid mandle)
        <return MS0_INVALID_MANDLE>

    if (new_mandle is NULL)
        <return MS0_INVALID_MANDLE>

    if (pointer at new_mandle is NOT NULL)
        <return MS0_INVALID_MANDLE>

    if (set_mandle(new_mandle, addr of current mandle's mandleclass) is
          not SUCCESS)
        <return Status>

    <copy masa out of old mandle into new mandle>
    <copy synonym out of old and into new>
    <copy any page buffer + virtual address out of old into new>
    if (we're open for paging and page buffer was "live")
        <increment the page buffer reference count>

    <return SUCCESS>


OTHER THINGS TO KNOW:

    The pointer at new_mandle must be NULL (ie *new_mandle == NULL)
    or this function returns MS0_INVALID_MANDLE.
*/

{
mir_status      set_status;     /* Status returned from set_mandle */


/* if (mandle pointer is NULL) */
if (a_mandle == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* if (mandle is not a valid mandle) */
if (a_mandle->tthis != a_mandle) {
    return (MS0_INVALID_MANDLE);
    }

/* if (new_mandle is NULL) */
if ( new_mandle == NULL ) {
    return (MS0_INVALID_MANDLE);
    }

/* if (pointer at new_mandle is NOT NULL) */
if ( *new_mandle != NULL ) {
    return (MS0_INVALID_MANDLE);
    }

/* if (set_mandle(new_mandle, addr of current mandle's mandleclass) is  */
/*          not SUCCESS)                                                */
if ( (set_status = set_mandle(new_mandle, &a_mandle->cclass)) != MS_SUCCESS) {
    return (set_status);
    }

/* copy masa out of old mandle into new mandle */
(*new_mandle)->m.m.ex_add = a_mandle->m.m.ex_add;

/* copy synonym out of old and into new */
(*new_mandle)->m.m.synonym = a_mandle->m.m.synonym;

/* copy any page buffer + virtual address out of old into new */
(*new_mandle)->m.m.vir_add = a_mandle->m.m.vir_add;
(*new_mandle)->m.m.page = a_mandle->m.m.page;

/* if (we're open for paging and page buffer was "live") */
if ((*new_mandle)->m.m.page != NULL && pbuffer_maxcount != 0) {

    /* increment the page buffer reference count */
    (*new_mandle)->m.m.page->ref_count += 1;

#ifdef PAGE_TRACE
        fprintf(PT_FILE, "\nPT: In mir_copy_mandle()....\n");
        fprintf(PT_FILE,
                "PT: Incrementing ref count to %d in page starting @ %d\n",
                (*new_mandle)->m.m.page->ref_count,
                (*new_mandle)->m.m.page->first);
#endif

    }

return (MS_SUCCESS);        
}

/* mir_free_mandle_class - Free all Mandles in a Mandle Class for Re-Use */
/* mir_free_mandle_class - Free all Mandles in a Mandle Class for Re-Use */
/* mir_free_mandle_class - Free all Mandles in a Mandle Class for Re-Use */

mir_status
mir_free_mandle_class(a_f_mandleclass)

mandle_class  **a_f_mandleclass;  /* Addr of ptr to Mandle class to be freed */

/*
INPUTS:

    "f_mandleclass" - address of pointer to mandleclass whose mandles are to
        be freed.


OUTPUTS:

The function returns one of:

    MS_SUCCESS - The mandle was successfully freed.

    MS0_INVALID_MANDLE - The passed mandle pointer was invalid.


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 1 function or direct user call that desires to
        free all mandles in a class created by a Tier 0 function.

    Purpose:
        This function frees the mandles by placing all on a free mandle list
        for later re-use.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (address of mandleclass pointer is NULL)
        <return MS0_INVALID_MANDLE>

    <copy mandleclass pointer locally>

    if (f_mandleclass pointer is NULL)
        <return MS0_INVALID_MANDLE>

    if (f_mandleclass is not a valid mandleclass)
        <return MS0_INVALID_MANDLE>

    LOCK_MANDLES()
    if (there is a "next")
         <make it's previous be freed's previous>

    if (f_mandleclass is at top of mandle class list)
        <make header of mandleclass list point to freed mc "next">
    else
        <make previous mandleclass's "next" point at freed "next">


    <copy top of free mandle list to local variable>
    <make free mandle list top point at freed mandleclass>
    <make freed mandleclass "next" be it's old "top" value>
    <set movable index to point to freed mandleclass>
    while (mandles in list remain)
        <mark as invalid>
        <step to next mandle if there is one>
        if (there is one)
            RELEASE_ANY_PAGE_BUFFER(for current mandle);

    <make last mandle "next" have old value of fml "top">

    <show mandleclass pointer as invalid>

    UNLOCK_MANDLES()
    <return MS_SUCCESS>

        
OTHER THINGS TO KNOW:

    When we delete a mandle class, not only does the mandleclass structure
    go onto the free mandle list, but all the mandles in the class.  So first
    we put the mandleclass onto the fml, then we jive to get all the mandles
    that were in that class onto the list too.    

    Note the mutex protection used in a threaded environment.
*/

{
mandle_class *f_mandleclass;      /* Local copy of pointer to mandleclass  */
mandle       *local_top;          /* Temporary storage for top of fml      */
mandle       *i,*j;               /* Local pointers to scan a mandle list  */


/* if (address of mandleclass pointer is NULL) */
if (a_f_mandleclass == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* copy mandleclass pointer locally */
f_mandleclass = *a_f_mandleclass;

/* if (mandle-class pointer is NULL) */
if (f_mandleclass == NULL) {
    return (MS0_INVALID_MANDLE);
    }

/* if (mandle is not a valid mandle) */
if (f_mandleclass->tthis != f_mandleclass) {
    return (MS0_INVALID_MANDLE);
    }

/*
|==============================================================================
| Start the process of detaching the mandleclass structure...
|==============================================================================
*/

LOCK_MANDLES();

/* if (there is a "next") */
if (f_mandleclass->next != NULL) {
    /* make it's previous be freed's previous */
    f_mandleclass->next->prev = f_mandleclass->prev;
    }

/* if (f_mandleclass is at top of mandle class list) */
if (f_mandleclass->prev == NULL) {
    /* make header of mandleclass list point to freed mc "next" */
    mcl = f_mandleclass->next;
    }
else {
    /* make previous mandleclass's "next" point at freed "next" */
    f_mandleclass->prev->next = f_mandleclass->next;
    }

/* copy top of free mandle list to local variable */
local_top = fml;

/* make free mandle list top point at freed mandleclass */
fml = f_mandleclass;


/*
|==============================================================================
| Ok, it has been detached and placed on the fml.  Now we put it's mandle
| list on the free list also.
|==============================================================================
*/

/* make freed mandleclass "next" be it's old "top" value */
f_mandleclass->next = f_mandleclass->m.mc.top;

/* set movable index to point to freed mandleclass */
i = j = f_mandleclass;

#ifdef PAGE_TRACE
    fprintf(PT_FILE,
            "\nPT: (...in mir_free_mandle_class(): freeing mandle(s)..)\n");
#endif

/*
| NOTE: Upon entry into this loop, "i" points to a mandleclass structure which
|       has no page-buffer associated with it.  Consequently the internal
|       logic of the loop is arranged to skip the release of any page buffer
|       for the first iteration of the loop.
|
| while (mandles in list remain) */
while (i != NULL) {

    /* mark as invalid */
    i->tthis = NULL;

    /* step to next mandle if there is one */
    j = i;
    i = i->next;

    if (i != NULL) {
        RELEASE_ANY_PAGE_BUFFER(i);
        }
    }

/*
|==============================================================================
| Now we hook what used to be the entire fml onto the end of the mandle list
| just taken from the freed mandleclass.  Now everything is on the fml.
|==============================================================================
*/

/* make last mandle "next" have old value of fml "top" */
j->next = local_top;

/* show mandleclass pointer as invalid */
*a_f_mandleclass = NULL;

UNLOCK_MANDLES();

return (MS_SUCCESS);        
}

/* mir_error - MIR Status Interpretation Function */
/* mir_error - MIR Status Interpretation Function */
/* mir_error - MIR Status Interpretation Function */

char *
mir_error(status)

mir_status status;      /* Inbound status */

/*
INPUTS:

    "status" is the binary (enumerated) status code to be interpreted to
    ASCII.


OUTPUTS:

The function returns a pointer to a string containing the ASCII representation
of the C-symbol used in the code that corresponds to the value of "status".


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0/1 function that needs to print out an
        error message.

    Purpose:
        This function performs a simple translation to obtain a printable
        string representing a Tier 0/1 error CODE.


ACTION SYNOPSIS OR PSEUDOCODE:

See code.

OTHER THINGS TO KNOW:

    This function is anticipated to serve debugging primarily.
*/

{
static char    *s[39];


s[0]="MS_NOT_IMPLEMENTED";  /* Function not implemented yet                  */
s[1]="MS_NO_MEMORY";        /* Insufficient Memory available                 */
s[2]="MS_SUCCESS";          /* The operation succeeded                       */
s[3]="MS_FAILURE";          /* The operation failed                          */
s[4]="MS_MUTEX_INIT_FAIL";  /* Failed to initialize a mutex                  */
s[5]="MS_MUTEX_ACQ_FAIL";   /* Acquisition of a Mutex failed                 */
s[6]="MS_MUTEX_RLS_FAIL";   /* Release of a Mutex failed                     */

        /* Tier 0 Status */
s[7]="MS0_DBFILE_OPENFAIL", /* Unable to open MIR binary database file       */
s[8]="MS0_DBFILE_READFAIL", /* Unable to read MIR binary database file       */
s[9]="MS0_BAD_ENDIAN",      /* Invalid Endian-ness indicator                 */
s[10]="MS0_BAD_VERSION",    /* Invalid Format Version indicator              */
s[11]="MS0_BAD_ENV_VAR_VAL";/* Bad Environment Variable Value (Paging)       */
s[12]="MS0_DB_NOT_LOADED";  /* MIR Database not loaded: no Init done         */
s[13]="MS0_INDEX_CORRUPTED";/* MIR Database File Index is corrupted          */
s[14]="MS0_NT_CORRUPTED";   /* MIR Database File Non-Terminals are corrupted */
s[15]="MS0_INVALID_OID";    /* A pointer to a required obj. id was null      */
s[16]="MS0_INVALID_MANDLE"; /* A pointer to a Mandle passed on the call was  */
                            /*   invalid or the mandle was invalid           */
s[17]="MS0_FIND_NONE";      /* No object was found                           */
s[18]="MS0_FIND_EXACT";     /* Found an exact match                          */
s[19]="MS0_FIND_SHORT";     /* The supplied OID was too short to fully match */
                            /*   the entry found in the MIR                  */
s[20]="MS0_FIND_LONG";      /* The supplied OID was too long to fully match  */
                            /*   the longest non-terminal object index entry */
s[21]="MS0_FIND_ROLL";      /* No long nor short match, rolled to next object*/
s[22]="MS0_FIND_NONTERM";   /* A match was found and NonTerminal mandle rtned*/
s[23]="MS0_FIND_TERM";      /* A match was found and a Terminal was returned */
s[24]="MS0_INVALID_SMI";    /* OID SMI indicator code was not valid          */
s[25]="MS0_NOT_REL_MANDLE"; /* Mandle is not a valid Relationship Mandle     */
s[26]="MS0_NO_PAGE_SLOTS";  /* Out of Page Slots to record a new page buffer */
s[27]="MS0_PAGE_LOGIC_ERR", /* Error in Paging Logic                         */

        /* Tier 1 Status */
s[28]="MS1_NOT_A_VALUED_OBJECT";   /* Obj. Found"; not defined w/value       */
s[29]="MS1_EXACT_OBJ_NOT_FND";     /* No Object by that exact Obj. ID        */
s[30]="MS1_MISSING_SMI";    /* SMI Relationship was inexplicably missing     */
s[31]="MS1_INIT_FAIL";      /* Initialization of Relationship Mandles failed */
s[32]="MS1_DC_NT_CORRUPT";  /* Non-Terms describing Data-Constructs corrupted*/
s[33]="MS1_DC_BUILTIN";     /* Built-In Data Construct found and returned    */
s[34]="MS1_DC_BUILTUP";     /* Built-Up Data Construct found and returned    */
s[35]="MS1_DC_BUILTIN_TMPLT";/* Built-In Data Construct Template fnd & ret.  */
s[36]="MS1_TMPLT_NUMBER";   /* Built-In Template "Number Value"              */
s[37]="MS1_TMPLT_STRING";   /* Built-In Template "String Value"              */
s[38]="MS1_INVALID_ARG";    /* Invalid Context or Ident Block pointer arg    */

return(s[(int) status]);

}

/* mir_get_mas - Get MIR Address Space */
/* mir_get_mas - Get MIR Address Space */
/* mir_get_mas - Get MIR Address Space */

void
mir_get_mas(mas_addr, masa_to_syn, rel_count)

unsigned int **mas_addr;     /* Where to return address of MIR Address Space */
masa         **masa_to_syn;  /* Where to return address of tranlate table    */
int          *rel_count;     /* Where to return size of translate table      */

/*
INPUTS:

    "mas_addr" is the address of where to return the address of the MIR
    Address Space.

    "masa_to_syn" is the address of where to return the address of the
    masa - to - synonym translation table.

    "rel_count" is where to return the number of entries in "masa_to_syn".


OUTPUTS:

The function returns each of the data described above.


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0/1 function that needs to "cheat" and
        obtain direct access to the MIR database binary representation.

    Purpose:
        This function returns addresses of data structures that are
        normally local to this module.


ACTION SYNOPSIS OR PSEUDOCODE:

See code.

OTHER THINGS TO KNOW:

    This function is anticipated to serve use in debugging tools.

    IT SHOULD NOT BE USED BY A TIER 1 INTERFACE or ANY PRODUCTION CODE.
*/

{

*mas_addr = mas;
*masa_to_syn = masa_to_synonym;
*rel_count = synonym_count;

return;

}

/* mir_debug_statistics - Dump In-Use Statistics for Mandles/Classes */
/* mir_debug_statistics - Dump In-Use Statistics for Mandles/Classes */
/* mir_debug_statistics - Dump In-Use Statistics for Mandles/Classes */

void
mir_debug_statistics(m_ptr, c_ptr, f_ptr)

int     *m_ptr;      /* --> Count of Mandles in-use               */
int     *c_ptr;      /* --> Count of Mandleclasses in-use         */
int     *f_ptr;      /* --> Count of free mandles/classes         */

/*
INPUTS:

    "m_count" is the address of an integer to receive the count of mandles
    currently in-use.

    "c_count" is the address of an integer to receive the count of mandle
    classes currently in-use.

    "f_count" is the address of an integer to receive the count of the
    number of mandles/class structures currently free for re-use.


OUTPUTS:

    The function returns the counts respectively of the number mandles
    in-use, (in) the number of mandleclasses in-use and the number of
    mandles or mandleclass structures available for re-use (free).


BIRD'S EYE VIEW:
    Context:
        The caller is a Tier 0/1 Level user.

    Purpose:
        This function provides in-use statistics for data-structures used
        by the Tier 0 functions that are dynamically allocated in the heap.
        This should allow the Tier 0 function user to be sure that all
        mandles and mandleclasses being properly released.

        This is primarily for use in debugging.


ACTION SYNOPSIS OR PSEUDOCODE:

    <LOCK_MANDLES()>

    <compute count of mandle-classes and in-use mandles>
    <compute count of empty mandle/classes available for re-use>
    <return the counts to the caller>

    <UNLOCK_MANDLES()>


OTHER THINGS TO KNOW:

    This function supports being called in a threaded-environment.
*/

{
mandle          *m;             /* Pointer to instance of a mandle       */
mandle_class    *c;             /* Pointer to instance of a mandle-class */
int             m_count=0;      /* Count of Mandles in-use               */
int             c_count=0;      /* Count of Mandles in-use               */
int             f_count=0;      /* Count of Mandles in-use               */


LOCK_MANDLES();

/* compute count of mandle-classes and in-use mandles */
for (c = mcl; c != NULL; c = c->next) {         /* for each mandle-class. . .*/
    c_count += 1;                               /* count it                  */

#ifdef SABER
    /* Do a validity check on class while we're here */
    if (c != c->tthis)
        fprintf(stderr,
                "DEBUG     <><><> MIR Mandle Class Corrupt %x <><><>\n", c);
#endif

    for (m = c->m.mc.top;
         m != NULL;
         m = m->next){ /* for each mandle in class  */

#ifdef SABER
        /* Do a validity check on mandle while we're here */
        if (m != m->tthis)
            fprintf(stderr,
                    "DEBUG     <><><> MIR Mandle Corrupt %x <><><>\n", m);
#endif

        m_count += 1;                           /* count it                  */
        }
    }      

/* compute count of empty mandle/class structures available for re-use */
for (c = fml; c != NULL; c = c->next)     /* for each free block. . .  */
    f_count += 1;                         /* count it                  */

/* return the counts to the caller */
*m_ptr = m_count;
*c_ptr = c_count;
*f_ptr = f_count;

UNLOCK_MANDLES();

}

/* NT_page_in - Non-Terminal Page-In */
/* NT_page_in - Non-Terminal Page-In */
/* NT_page_in - Non-Terminal Page-In */

          /* NOTE: This function invoked via macro "SET_MAS_PAGE()" */

static mir_status
NT_page_in(nt_mandle)

mandle          *nt_mandle;     /* Mandle of Non-Terminal to be paged in    */


/*
INPUTS:

    "nt_mandle" is a pointer to a mandle for the MIR Non-Terminal Object that
    must be made available.  The "page" cell within the mandle is zero
    on entry.  For paging operations, this must be filled in as well as
    "vir_add".


OUTPUTS:

    The function returns MS_SUCCESS when a buffer has been found (or loaded)
    with that portion of the MIR address space that completely encompasses the
    requested MIR Non-Terminal.  Additionally, mandle cells are set to point
    to that buffer and "vir_masa" (in the mandle) is set to the 'index' into
    the buffer to reference the MIR Non-Terminal.

    If there is an error reading the binary MIR database file, the function
    returns "MS0_DBFILE_READFAIL".

    If a certain page-logic error occurs, the function returns
    "MS0_PAGE_LOGIC_ERR".

    If I've made a misjudgment and we run out of 'slots' to represent
    page buffers, the function returns "MS0_NO_PAGE_SLOTS".

    If anything other than MS_SUCCESS gets returned, things are in a state
    of disarray (storage may have been leaked) and things should be shut down.


BIRD'S EYE VIEW:
    Context:
        After a 'paged-open' on the binary MIR database file, a Tier 0
        function has reached a point where it needs to reference a MIR
        Non-Terminal.  These are paged-on-demand into buffers maintained
        by Tier 0 (in this function).  This function is only invoked by
        the macro that calls it (SET_MAS_PAGE) if the mandle for the
        Non-Terminal that is about to be referenced has no page-buffer
        (containing the NT) associated with it.

    Purpose:
        This function acquires a buffer containing the needed MIR Non-Terminal
        Object plus the needed information required to reference that
        Non-Terminal Object in the buffer.  (As needed, an existing buffer
        is found or an entirely new buffer is (re-used from the Free List or)
        created and loaded from the external MIR database file).


ACTION SYNOPSIS OR PSEUDOCODE:

    <Lock Mandles and Lists>

    <signal "No Highest-Closest Buffer Found (yet)">
    <perform binary search on list of active buffers for Highest-Closest>
    if (Highest-Closest Buffer was found)
        if (no last-available MIR Non-Terminal starting address has been computed)
            <compute the addr of last fully-available MIR Non-Terminal in buff>
        if (NT is contained within the buffer)
            <store virtual masa and buffer page in mandle>
            if (the buffer is on the Free List)
                <remove it from the Free List>
                <decrement Free List count>
            <increment the buffer reference count>
            <Unlock Mandles and Lists>
            <return MS_SUCCESS>

    (* No active Buffer holds desired NT, but binary search has located the
       slot where such a buffer's masa should be stored in "masa_vector" *)

    if (Free List contains more than full depth)
       <remove the oldest buffer from the Free List>
       <decrement count of buffers on Free List>
       <perform bin. search on list of active buffers for this buffer's entry>
       <record slot as entry-to-be-deleted>
    else
       <show no slot as entry-to-be-deleted>
       <increment count of active page buffers>
       if (we have no more slots for another buffer)
           <unlock Mandles and Lists>
           <return MS0_NO_PAGE_SLOTS>
       if (attempt to allocate storage for new page_buf failed)
           <unlock Mandles and Lists>
           <return MS_NO_MEMORY>
       if (attempt to allocate storage for page buffer failed)
           <free the page_buf>
           <unlock Mandles and Lists>
           <return MS_NO_MEMORY>
    <(re)-initialize all cells in page_buf>
    <compute fseek value given desired masa and magic_fseek>
    if (fseek failed)
        <unlock Mandles and Lists>
        <return MS0_DBFILE_READFAIL>
    if (fread for buffer size failed)
        <unlock Mandles and Lists>
        <return MS0_DBFILE_READFAIL>

    <store virtual masa and buffer page in mandle>
    <set the buffer reference count to 1>

    <select the lowest slot we're to start with: any "entry-to-be-deleted" or
     slot where new buffer insertion must be done>
    for (every slot from lowest to end in "masa_vector")
        if (current slot is "entry-to-be-deleted")
            <skip it>
        if (current slot is where new entry should go)
            <insert new masa>

    for (every slot from lowest to end in "buffer_vector")
        if (current slot is "entry-to-be-deleted")
            <skip it>
        if (current slot is where new entry should go)
            <insert new page-buffer pointer>

    <Unlock Mandles and Lists>
    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

    This function attempts to avoid unneeded I/O by searching the list
    of loaded buffers ("masa_vector") for a buffer that might contain the
    requested Non-Terminal.

    If it finds such a buffer, the buffer's reference count is incremented
    and it is returned as the needed buffer.

    If no current buffer contains the complete MIR Non-Terminal, then

        If the Free List of 'used-buffers' contains more than "FREE_LIST_DEPTH"
        buffers, then the oldest buffer on the Free List is re-used by loading
        it with that portion of the MIR address space that encompasses the
        requested Non-Terminal.

        If the Free List of 'used-buffers' does not contain "FREE_LIST_DEPTH"
        buffers, then a new buffer is allocated from the heap and loaded with
        that portion of the MIR address space that encompasses the requested
        Non-Terminal.

    Note that the size of each buffer is set to be big enough to contain the
    largest MIR Non-Terminal that is contained in the currently open binary
    MIR database file.  It might seem that this would be a source of waste
    when a huge buffer is used to contain a small Non-Terminal.

    This is not necessarily the case, as the lookup scheme will successfully
    find a Non-Terminal residing toward the end of a buffer that was originally
    loaded for a small Non-Terminal that precedes it.  This is by conscious
    design, because the compiler's output has been carefully arranged to
    have 'children' Non-Terminals *follow* their parents.  Consequently, when
    loading a Non-Terminal that is considered a 'parent', some of its
    children will typically be loaded with it.  When descending the hierarchy,
    sometimes I/O will not be needed, as a child will have been loaded with
    it's parent (in the cases where the parent is comparatively small).
*/

{
int             candidate_buffer_index; /* Index resulting from Bin Search  */
int             selected_slot_index;    /* Index resulting from Bin Search  */
int             deleted_slot_index;     /* Index resulting from Bin Search  */
page_buf        *spage;                 /* Selected Page Buffer datablock   */
int             cmp_code;   /* Result of string comparison                  */
int             i;          /* Handy-Dandy General purpose index            */
int             hi,lo;      /* Boundary posts for binary search             */
int             cell_count; /* Count of cells to move in masa/buffer_vector */


#ifdef PAGE_TRACE
page_buf        *scanner;       /* Used for scanning Free List */
fprintf(PT_FILE, "\nPT: >>>>> Entering NT_page_in() . . . \n");
fprintf(PT_FILE, "PT: Mandle:\n");
fprintf(PT_FILE, "PT: ex_add: %d\n", nt_mandle->m.m.ex_add);
fprintf(PT_FILE, "PT: vir_add: %d\n", nt_mandle->m.m.vir_add);
fprintf(PT_FILE, "PT: synonym: %d\n", nt_mandle->m.m.synonym);
fprintf(PT_FILE,
        "PT: Page Buf: starting at %d\n\n",
        ((nt_mandle->m.m.page == NULL) ? (0) : nt_mandle->m.m.page->first));
fprintf(PT_FILE, "Page-Buffer Summary On-Entry:\n");
fprintf(PT_FILE, "PT:    Entry\tmasa_r\tfirst\tlast\tv_last\trefs\ta_size\tpage_buf\n");
for (i = 0; i < pbuffer_curcount; i++) {
    spage = buffer_vector[i];
    fprintf(PT_FILE, "PT:\t%4d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04x\n",
            i, masa_vector[i], spage->first, spage->last, spage->v_last,
            spage->ref_count, spage->a_size, spage);
    }
fprintf(PT_FILE, "PT:    End of Page-Buffer Summary\n");
fprintf(PT_FILE,
        "\nPT: Free List Dump  Top of Free = %x,  Bot of Free = %x\n",
        top_free, bot_free);
fprintf(PT_FILE, "PT: Page_buf Addr\tRef_count\tPrev\t\tNext\n");
for (scanner = top_free; scanner != NULL; scanner = scanner->next) {
    fprintf(PT_FILE, "PT:\t%x\t%04d\t\t%8x\t%8x\n",
            scanner, scanner->ref_count, scanner->prev, scanner->next);
    if (scanner->next == NULL && bot_free != scanner) {
        fprintf(PT_FILE,
                "PT: Free List bottom pointer is incorrect: %x\n",
                bot_free);
        }
    }
fprintf(PT_FILE, "PT:    End of Free List Dump\n");
#endif

/* Lock Mandles and Lists */
LOCK_MANDLES();

/* signal "No Highest-Closest Buffer Found (yet)" */
candidate_buffer_index = -1;
selected_slot_index = 0;

/*
| ====================== START BINARY SEARCH ========================
| Perform binary search on list of active buffers for Highest-Closest
|
| We're looking to find the highest *index* of an entry in "masa_vector"
| whose (entry) value DOES NOT EXCEED the masa of the Non-Terminal we're
| seeking to 'page-in'.  The page buffer corresponding to such a found-entry
| stands the best chance of actually containing the Non-Terminal we're after.
|
| This index selects a buffer via "buffer_vector" that contains a "page"
| starting at the masa we found in "masa_vector".  With luck, it will
| also contain the Non-Terminal we're looking for.
*/
hi = pbuffer_curcount - 1;  /* Set Hi boundary post             */
lo = 0;                     /* Set Lo boundary post             */
i = -1;                     /* Set so we can infer "no buffers" */

while (lo <= hi) {
    i = (lo+hi)/2;     /* Grab the (new) comparison point */

    /* Compare: --NEXT ENTRY-- to SOUGHT Non-Terminal's external address */
    cmp_code =  masa_vector[i] - nt_mandle->m.m.ex_add;

    if (cmp_code < 0)
        lo = i + 1;       /* Tested Entry is Too Low: Raise lower bound */
    else if (cmp_code > 0)
        hi = i - 1;       /* Tested Entry is Too High: Lower upper bound */
    else
        break;
    }
/*
| ============================= END BINARY SEARCH ========================
|
| We're interested in developing at least one, and possibly two pieces of
| information from the output of this binary search:
|
|  1) "candidate_buffer_index":
|
|     The index of a "slot" in the "masa_vector" (and implicitly in the
|     "buffer_vector" as well) where an entry resides that is the best
|     possible candidate for a buffer holding the Non-Terminal we are
|     interested in.  The masa of the first cell in the buffer referred to
|     by the entry in this selected "slot" is supposed to be the highest
|     starting-masa (of all the possible buffers referred to by "masa_vector")
|     that DOES NOT EXCEED the masa of the Non-Terminal we're after.  We've
|     got to do further checks to see if the buffer really does encompass the
|     Non-Terminal we're after.
|
| *IF* it turns out that:
|       * there are no buffers currently in use at all OR
|       * all buffers in use have starting-masa's that EXCEED the masa of
|         the desired Non-Terminal OR
|       * the candidate "slot" selected in 1) above specified a buffer
|         that did not encompass the desired Non-Terminal
| *THEN*
|     we need a second piece of information:
|
|  2) "selected_slot_index":
|
|     The index of a "slot" in the "masa_vector" (and implicitly in the
|     "buffer_vector" as well) where a NEW entry must be inserted to describe
|     a NEW buffer (to be allocated) which will be set-up to contain the
|     Non-Terminal we're interested in.  It may be that this slot is already
|     occupied (and we'll have to 'slide' existing entries 'upward' to insert
|     the new entry) or it may be that this slot is 'beyond the end' of the
|     current entries (in which case no 'sliding' will be needed to insert
|     it, so long as room remains in "masa_vector" and "buffer_vector").
| 
| At this point in the code, we know all we need to know to generate both
| pieces of information, but we don't know whether the second piece will be
| needed (because we don't know yet whether any apparently valid buffer
| selected by the binary search actually encompasses the desired NT).
|
| Note: Both pieces of information are "slot" indexes, but only the
|       "candidate_buffer_index" slot actually contains a buffer we're
|       interested in looking at.
|
| We develop "candidate_buffer_index" and "selected_slot_index", according
| to the following possible situations:
|
| 1) No Buffers at all are active:
|
|       ..
|
! 2) Active Candidate buffer "B" is found amongst other active buffers "X":
|
|    a)   .BXX.
|    b)   .XBX.
|    c)   .XXB.
|
| 3) All active buffers "X" have 'starting-masas' that EXCEED the masa
|    of the desired Non-Terminal, which must eventually be loaded into a new
|    buffer "b":
|
|       .bXX.
|
| Note that if the desired NT masa falls within the last active buffer, OR
| it falls beyond (and higher) than the last active buffer, then case 2c)
| describes both these situations.
|
| -----------------------------------------------------------------------------
|
| We have two pieces of information coming out of the binary search code:
|
|       * "i"        (the index of the last slot a comparison was made against,
|                      or "-1" if no comparisons were made)
|
|       * "cmp_code" (the result of the last comparison at slot "i" if there
|                     was a comparison)
|
|
| "i" and "cmp_code" must be examined to determine which of the possible
| cases obtains, and then set "candidate_buffer_index" (where we hope to find
| the Non-Terminal we're after) and "selected_slot_index" (where we'll have
| to stick-in another reference to a new buffer if the NT *wasn't* in the
| "candidate_buffer_index" buffer) accordingly:
|
| 1) No Buffers at all are active:
|
|       ..
|
|    candidate_buffer_index = NONE (-1)
|    selected_slot_index = 0 (the first)
|
|
! 2) Candidate buffer "B" is found amongst active buffers "X":
|
|    a)   .BXX.
|    b)   .XBX.
|    c)   .XXB.
|
|    candidate_buffer_index = (slot for "B")
|    selected_slot_index = (slot for "B" + 1)
|
|
| 3) All active buffers "X" have 'starting-masas' that EXCEED the masa
|    of the desired Non-Terminal, which must eventually be loaded into a new
|    buffer "b":
|
|       .bXX.
|
|    candidate_buffer_index = NONE (-1)
|    selected_slot_index = (slot for "b", ie "0")
|
|
| * Case 1) is detected when no comparisons have been performed (i == -1).
|
|       "candidate_buffer_index" is left as NONE (-1).
|       "selected_slot_index" is left as 0.
|
| * Case 2) is handled as follows:
|
|       "i" specifies the last slot compared
|       "cmp_code" specifies how the comparison went
|
|   2a) cmp_code == 0:
|       "i" specifies the "candidate_buffer_index".  We know that
|       "selected_slot_index" will not be needed, but it is set to "i+1"
|       just for symmetry.
|
|   2b) cmp_code < 0:
|       "i" specifies the "candidate_buffer_index" because it's masa DOES
|       NOT EXCEED the desired NT's masa.  "selected_slot_index" will be
|       "i+1" (if it is needed).
|
|   2c) cmp_code > 0:
|       "i" specifies an entry that EXCEEDS the desired NT's masa.
|       Consequently, "candidate_buffer_index" is set to "i-1",
|       "selected_slot_index" is set to "i".  (See also Case 3)
|
| * Case 3) is handled as a subset of Case 2c), when "i" is
|       zero, then there is no "i-1"th buffer that can be
|       "candidate_buffer_index", consequently:
|
|       "candidate_buffer_index" = NONE (-1)
|       "selected_slot_index" is set to "i" (zero).
|
|   NOTE: The code to handle Case 2c) generates the right answers for
|         Case 3) by happenstance (well, maybe not entirely coincidence!).
*/

if (i != -1) {        /* "NOT Case 1)" ... implies Cases 2) & 3) */

    if (cmp_code <= 0 ) {                  /* Cases 2a) & 2b) */
        candidate_buffer_index = i;
        selected_slot_index = i + 1;
        }
    else { /* . . . implies (cmp_code > 0):   Case 2c) and 3) */
        candidate_buffer_index = i - 1;
        selected_slot_index = i;
        }
    }


/* if (Highest-Closest Buffer was found) */
if (candidate_buffer_index != -1) {

    /* if (no last-available MIR Non-Terminal starting addr is computed yet) */
    if (buffer_vector[candidate_buffer_index]->last == 0) {

        masa    v_masa;        /* Used for tracking the virtual masa         */
        masa    r_masa;        /* Used for tracking the real masa            */
        int     nt_size;       /* Non-Terminal size *in MIR addr space cells**/
        masa    pv_masa;       /* Next potential 'v_masa'                    */
        masa    pr_masa;       /* Next potential 'r_masa'                    */
        unsigned int *l_page;  /* Local copy of page buffer                  */

        l_page = buffer_vector[candidate_buffer_index]->page;

        /*
        | compute the addr of last fully-available MIR Non-Terminal in buff
        |
        | Note that the first Non-Terminal is guaranteed to be wholly-contained
        | because the buffers are always largest enough to hold the largest
        | Non-Terminal.  Beyond the first one though, things get tricky!
        */

        /* Initial 'virtual' masa */
        v_masa = 0;

        /* Initial 'real' masa */
        r_masa = buffer_vector[candidate_buffer_index]->first;

        /* Compute size of NT that "v_masa" points to */
        nt_size = 1
            + (l_page[v_masa] & mas[ENTRY_COUNT_AMASK_MASA])
            + (((unsigned int) l_page[v_masa]) >> mas[OID_COUNT_RSHIFT_MASA]);

        /* If the first NT does not completely fill the buffer. . . then loop
        |
        | Note that we *might* come up with the fact that the NT exactly
        | fills the buffer, but it should not discover that it's too big.
        */
        if ( (v_masa + nt_size)       /* Total if this NT fits in buffer */
            < buffer_vector[candidate_buffer_index]->a_size)   /* Actual */

            for (;;) {
                /*
                | Compute next potential 'v_masa/r_masa' address.  We know
                | that at least this one cell of the next NT exists in the
                | buffer if we arrive here.
                */               
                pv_masa = (v_masa + nt_size);
                pr_masa = (r_masa + nt_size);

                nt_size = 1 + (l_page[pv_masa] & mas[ENTRY_COUNT_AMASK_MASA])
                    + (((unsigned int) l_page[pv_masa])
                       >> mas[OID_COUNT_RSHIFT_MASA]);

                /* if (size of next potential NT can't fit in buffer) */
                if ( (pv_masa + nt_size)
                    > buffer_vector[candidate_buffer_index]->a_size)
                    break;      /* Bust out: v_masa & r_masa are set */

                /* else if (potential NT fits EXACTLY in remaining space) */
                else if ( (pv_masa + nt_size)
                    == buffer_vector[candidate_buffer_index]->a_size) {

                    /* Boost v_masa & r_masa to reflect last NT */
                    v_masa = pv_masa;
                    r_masa = pr_masa;
                    break;
                    }

                /*
                | If we get to here, we know that *at least* one more cell
                | legally exists in the buffer beyond the NT @ pv_masa.
                */
                v_masa = pv_masa;
                r_masa = pr_masa;
                }

        /*
        | As we fall out of the loop above, v_masa and r_masa are
        | (respectively) the 'virtual' and 'real' masas for the last NT that
        | is wholly contained within the page buffer.
        |
        | Record this stuff with the page buffer so we don't have to do this
        | again.
        */
        buffer_vector[candidate_buffer_index]->v_last = v_masa;
        buffer_vector[candidate_buffer_index]->last = r_masa;
        }
          
    /*
    | The requested NT resides in the buffer if it's NT is less than or
    | equal to the real masa of the last wholly contained NT: "last".
    |
    | if (NT is contained within the buffer)
    */
    if (buffer_vector[candidate_buffer_index]->last >= nt_mandle->m.m.ex_add) {

        /* Grab a shorter synonym for our selected page buffer datablock */
        spage = buffer_vector[candidate_buffer_index];

        /* store virtual masa and buffer page in mandle */
        nt_mandle->m.m.vir_add = nt_mandle->m.m.ex_add - spage->first;
        nt_mandle->m.m.page = spage;

        /* if (the buffer is on the Free List) */
        if (spage->ref_count == 0) {

            /* ===========================
            | remove it from the Free List
            |
            | If it was at the top of the list, fix the top-of-list pointer
            */
            if (top_free == spage)
                top_free = spage->next;

            /*
            | If it was at the bottom of the list, fix the bot-of-list pointer
            */
            if (bot_free == spage)
                bot_free = spage->prev;

            /* Now unhook it */
            if (spage->prev != NULL)
                spage->prev->next = spage->next; /* Fix The Entry Above */
            if (spage->next != NULL)
                spage->next->prev = spage->prev; /* Fix The Entry Below */
            /* =========================== */

            /* decrement Free List count */
            pbuffer_freecount -= 1;
            }

        /* increment the buffer reference count */
        spage->ref_count += 1;

#ifdef PAGE_TRACE
fprintf(PT_FILE, "\nPage-Buffer Summary after check for availability:\n");
fprintf(PT_FILE, "PT:    Entry\tmasa_r\tfirst\tlast\tv_last\trefs\ta_size\tpage_buf\n");
for (i = 0; i < pbuffer_curcount; i++) {
    spage = buffer_vector[i];
    fprintf(PT_FILE, "PT:\t%4d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04x\n",
            i, masa_vector[i], spage->first, spage->last, spage->v_last,
            spage->ref_count, spage->a_size, spage);
    }
fprintf(PT_FILE, "PT:    End of Page-Buffer Summary\n");
fprintf(PT_FILE,
        "\nPT: Free List Dump  Top of Free = %x,  Bot of Free = %x\n",
        top_free, bot_free);
fprintf(PT_FILE, "PT: Page_buf Addr\tRef_count\tPrev\t\tNext\n");
for (scanner = top_free; scanner != NULL; scanner = scanner->next) {
    fprintf(PT_FILE, "PT:\t%x\t%04d\t\t%8x\t%8x\n",
            scanner, scanner->ref_count, scanner->prev, scanner->next);
    if (scanner->next == NULL && bot_free != scanner) {
        fprintf(PT_FILE,
                "PT: Free List bottom pointer is incorrect: %x\n",
                bot_free);
        }
    }
fprintf(PT_FILE, "PT:    End of Free List Dump\n");
fprintf(PT_FILE, "\nPT: <<<<<<< Leaving NT_page_in() after using existing page\n");
#endif

        /* Unlock Mandles and Lists */
        UNLOCK_MANDLES();
        return (MS_SUCCESS);
        }
    }

/*
| No active Buffer holds desired NT, but the binary search has located the
| slot where such a buffer's masa should be stored in "masa_vector".  The
| slot is specified by "selected_slot_index". Ta dum.
*/

/* if (Free List contains more than full depth) */
if (pbuffer_freecount > fl_depth) {

    /* remove the oldest buffer from the Free List
    |
    | NOTE:  No assumptions can be made about the count of entries on the free
    |        list, as "fl_depth" could be set to zero.  Consequently full
    |        logic must be provided here to keep "top_free" and "bot_free"
    |        correctly updated!.
    */
    spage = top_free;           /* Grab the top of the list  */
    top_free = spage->next;     /* Reset top-of-list pointer */
    if (top_free != NULL)       /* Show new "top of list"    */
        top_free->prev = NULL;  /*   if there is one         */
    if (bot_free == spage)      /* If it was also the last ..*/
        bot_free = NULL;        /*  . . .show nothing left   */

    /* decrement count of buffers on Free List */
    pbuffer_freecount -= 1;

    /* perform bin. search on list of active buffers for this buffer's entry
    |
    | We just grabbed a page-buffer off the free list.  It contained some
    | part of the MIR address space (that was still available for re-use by
    | being looked up via "masa_vector").  We now need to remove the entry for
    | this to-be-reused page buffer from "masa_vector" and "buffer_vector"
    | because we're going to put a different part of the MIR address space in
    | it.
    |
    | To look up this old entry, we use the masa of the first Non-Terminal
    | in the page buffer.  This binary search *must* succeed or something
    | is seriously amiss.
    */
    hi = pbuffer_curcount - 1;  /* Set Hi boundary post             */
    lo = 0;                     /* Set Lo boundary post             */
    i = -1;                     /* Set so we can infer "no buffers" */

    while (lo <= hi) {
        i = (lo+hi)/2;     /* Grab the (new) comparison point */

        /* Compare: --NEXT ENTRY-- to SOUGHT entry-to-be-deleted */
        cmp_code =  masa_vector[i] - spage->first;

        if (cmp_code < 0)
            lo = i + 1;       /* Tested Entry is Too Low: Raise lower bound */
        else if (cmp_code > 0)
            hi = i - 1;       /* Tested Entry is Too High: Lower upper bound */
        else
            break;
        }

    /* If (we didn't get an EXACT match) */
    if (cmp_code != 0 || i == -1) {
        UNLOCK_MANDLES();               /* unlock Mandles and Lists */
        return (MS0_PAGE_LOGIC_ERR);
        }

    /* record slot as entry-to-be-deleted */
    deleted_slot_index = i;
    }

else {  /* Nothing to re-use from the Free List, need to malloc more */

    /* show "no slot" as entry-to-be-deleted */
    deleted_slot_index = -1;

    /* increment count of active page buffers */
    pbuffer_curcount += 1;

    /* if (we have no more slots for another buffer) */
    if (pbuffer_curcount > pbuffer_maxcount) {
        UNLOCK_MANDLES();               /* unlock Mandles and Lists */
        return(MS0_NO_PAGE_SLOTS);
        }

    /* if (attempt to allocate storage for new page_buf failed) */
    if ((spage = (page_buf *) malloc(sizeof(page_buf))) == NULL) {
        UNLOCK_MANDLES();               /* unlock Mandles and Lists */
        return(MS_NO_MEMORY);
        }

    /* if (attempt to allocate storage for page buffer failed) */
    if ((spage->page = (unsigned int *) malloc(pbuffer_size)) == NULL){
        free(spage);    /* free the page_buf */
        UNLOCK_MANDLES();               /* unlock Mandles and Lists */
        return(MS_NO_MEMORY);
        }
    }

/*
| Ok, at this point we have a page buffer that is unencumbered (except for
| a reference to it from "masa_vector" and "buffer_vector" which we'll take
| care of in the loops below).
|
| (re)-initialize all cells in page_buf
*/
spage->prev = spage->next = NULL;
spage->first = nt_mandle->m.m.ex_add;
spage->last = spage->v_last = 0;

/* compute fseek value given desired masa and magic_fseek */
/* if (fseek failed) */
if (fseek(mir_file,
          (magic_fseek + (sizeof(unsigned int)*nt_mandle->m.m.ex_add)),
          SEEK_SET) == -1) {
    UNLOCK_MANDLES();               /* unlock Mandles and Lists */
    return (MS0_DBFILE_READFAIL);
    }

/* if (fread for buffer size failed) */
if ((spage->a_size =
     fread(spage->page,                         /* Addr of Buffer     */
           sizeof(unsigned int),                /* Items of this size */
           pbuffer_size/sizeof(unsigned int),   /* These many items   */
           mir_file                             /* From this File     */
           )) == 0) {
    UNLOCK_MANDLES();               /* unlock Mandles and Lists */
    return(MS0_DBFILE_READFAIL);
    }

/* store virtual masa and buffer page in mandle */
nt_mandle->m.m.vir_add = 0;     /* "0" means the NT starts @ beg of buffer */
nt_mandle->m.m.page = spage;    /* there is an active page buffer          */

/* Set the buffer reference count to 1*/
spage->ref_count = 1;

/*
| Now we've got to bring "masa_vector" and "buffer_vector" into line with
| the new state of affairs w/respect to the page buffers.
|
| We *may* have an entry to-be-deleted ("deleted_slot_index").
|
| We *have* a new entry (spage) to be added (at "selected_slot_index").
|
| They might be the same slot!  Either slot may precede the other!
|
| We have to do the same thing to both masa_vector and buffer_vector.
|
| At this point "pbuffer_curcount" reflects the size of both vectors after the
| new entry has been added.  A deletion is always accompanied by an insertion,
| so pbuffer_curcount is unaffected (and still correct).
*/

/* ============================================================================
| Handle creation of new "slot" as needed in "masa_vector" & "buffer_vector"
|
| Slide the contents of the slot to be used for the new insertion (plus
| anything that follows it) "up" one entry to 'empty out' the slot receiving
| the new insertion.  Note that "pbuffer_curcount" *INCLUDES*
| the new slot, so that is taken into account in computing the actual number
| of live entries to be slid.
|
| Note that once this slide has occurred, we may be temporarily using one more
| slot in the vectors than is actually active (until the delete code does its
| thing below).
*/
cell_count = (pbuffer_curcount - selected_slot_index);

/* Slide it up */
if (cell_count > 0) {

    memmove(&masa_vector[selected_slot_index + 1],  /* ... to here         */
            &masa_vector[selected_slot_index],      /* ... from here       */
            (cell_count * sizeof(masa_vector[0]))   /* ... this many bytes */
            );

    memmove(&buffer_vector[selected_slot_index + 1],  /* ... to here         */
            &buffer_vector[selected_slot_index],      /* ... from here       */
            (cell_count * sizeof(buffer_vector[0]))   /* ... this many bytes */
            );
    }

/* Record the masa of the first NT in the page buffer in "masa_vector" */
masa_vector[selected_slot_index] = spage->first;

/* Record the address of the page-buffer block in "buffer_vector" */
buffer_vector[selected_slot_index] = spage;

/*
| If the deleted_slot_index is greater than or equal to "selected_slot_index",
| then the slot-to-be-deleted has been slid up by 1, so we compensate by
| adding 1 to the deleted_slot_index:
*/
if (deleted_slot_index >= selected_slot_index)
    deleted_slot_index += 1;


/* ============================================================================
| Handle deletion of any slot to-be-deleted in masa_vector & buffer_vector
*/
if (deleted_slot_index != -1) {

    /* Compute size of stuff following deleted slot that needs to be moved down
    |
    | Note that we are computing the size of the stuff *following* but not
    | including the slot, unlike the "slide up" above.
    |
    | "deleted_slot_index + 1" is the start of the stuff to be moved downward.
    */
    cell_count = pbuffer_curcount - deleted_slot_index;

    /* Slide it down */
    if (cell_count > 0) {

        memmove(&masa_vector[deleted_slot_index],     /* ... to here         */
                &masa_vector[deleted_slot_index+ 1],  /* ... from here       */
                (cell_count * sizeof(masa_vector[0])) /* ... this many bytes */
                );

        memmove(&buffer_vector[deleted_slot_index],   /* ... to here         */
                &buffer_vector[deleted_slot_index+ 1],/* ... from here       */
                (cell_count * sizeof(buffer_vector[0]))/* ...this many bytes */
                );
        }
    }

#ifdef PAGE_TRACE
fprintf(PT_FILE, "\nPage-Buffer Summary after re-use, reorg:\n");
fprintf(PT_FILE, "PT:    Entry\tmasa_r\tfirst\tlast\tv_last\trefs\ta_size\tpage_buf\n");
for (i = 0; i < pbuffer_curcount; i++) {
    spage = buffer_vector[i];
    fprintf(PT_FILE, "PT:\t%4d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04x\n",
            i, masa_vector[i], spage->first, spage->last, spage->v_last,
            spage->ref_count, spage->a_size, spage);

    }
fprintf(PT_FILE, "PT:    End of Page-Buffer Summary\n");
fprintf(PT_FILE,
        "\nPT: Free List Dump  Top of Free = %x,  Bot of Free = %x\n",
        top_free, bot_free);
fprintf(PT_FILE, "PT: Page_buf Addr\tRef_count\tPrev\t\tNext\n");
for (scanner = top_free; scanner != NULL; scanner = scanner->next) {
    fprintf(PT_FILE, "PT:\t%x\t%04d\t\t%8x\t%8x\n",
            scanner, scanner->ref_count, scanner->prev, scanner->next);
    if (scanner->next == NULL && bot_free != scanner) {
        fprintf(PT_FILE,
                "PT: Free List bottom pointer is incorrect: %x\n",
                bot_free);
        }
    }
fprintf(PT_FILE, "PT:    End of Free List Dump\n");
fprintf(PT_FILE, "\nPT: <<<<<<< Leaving NT_page_in() after creating page\n");
#endif

/* Unlock Mandles and Lists */
UNLOCK_MANDLES();
return(MS_SUCCESS);

}
