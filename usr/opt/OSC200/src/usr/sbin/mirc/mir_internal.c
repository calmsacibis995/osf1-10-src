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
static char *rcsid = "@(#)$RCSfile: mir_internal.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/09/07 19:56:54 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1989-1992, 1993.
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
 * Module MIR_INTERNAL.C
 *      Contains functions required by the MIR Compiler to load into (the
 *      heap) an "internal" representation of a MIR binary database file.
 *      (These function perform the inverse operations of those in module
 *      "mir_external.c").
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/Common Agent
 *    D. D. Burns   Sept 1992
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines have need of network management
 *       architecture information in order to properly process incoming
 *       management request messages.  This information must reside in a
 *       Management Information Repository, accessible by ISO Object ID.
 *
 *    Purpose:
 *       This module contains the functions required by the Common Agent MIR
 *       compiler to generate from an input "external" binary file an
 *       internal ("Intermediate") representation of that information for use
 *       in another compilation by the compiler.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History      When            Who             What
 *      V1.0    September 1992  D. D. Burns     Original Version
 *      V1.99   October 1992    D. D. Burns     Add support for Merge & Augment

Module Overview:

This module contains all the functions used by the MIR Compiler to generate an
internal heap representation of the contents of a binary MIR database file.
Once loaded by these functions, a compilation can begin on another (ASCII)
input MSL file.



MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
mirc_binary_load     Opens the input binary MIR file and copies the contents
                     to an in-heap intermediate representation, builds internal
                     compiler lists and readies the compiler for an ASCII MSL
                     compilation.

INTERNAL FUNCTIONS:
mirci_internal_init  Performs the file system open on the binary input file,
                     reads in the preamble information and Partition Table
                     Partition, sets up some local data structures in
                     preparation for reading the MIR Address Space ("MAS"--the
                     actual compiled binary info).

mirci_MAS_load       Reads sequentially (via mirci_fetch_next()) the contents
                     of each partition (Slice, Subregister, Signed/Unsigned
                     Numbers, Strings and the three Non-Terminal Partitions),
                     allocates heap storage for each instance of every kind
                     of partition MIR Object, copies the contents of each
                     Object read from the file into each in-heap 
                     representation of same WITHOUT performing pointer
                     address-translation.

mirci_fetch_next     Reads the next sequential MIR Object  from the binary MIR
                     file into a buffer and makes it available to it's caller
                     (mirci_MAS_load()).  This function takes care of doing
                     the actual file I/O to be certain that all of the next
                     MIR Object to be processed is actually in-memory.

mirci_addr_xlate     Once the in-heap representation of all MAS partitions
                     has been created, this function sweeps through all
                     in-heap MIR Objects that contain pointers (Slices,
                     Subregisters and all Non-Terminals) and converts the
                     external MAS addresses to in-heap addresses using a
                     cross-reference data-structure created by
                     "mirci_MAS_load()".

mirci_NT_addr_xlate  Performs processing particular to address translation
                     required for Non-Terminals (called by mirci_addr_xlate()).

mirci_compiler_lists Constructs the four internal lists (that the front-end of
                     the compiler needs to perform compilations) by sweeping
                     down the Non-Terminal partitions that contain the
                     MIR Relationship Objects and the Built-In SMI Datatype
                     objects.
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef VMS
#include <malloc.h>
#endif

/* Request definitions for compiler modules & Tier 0 from "mir.h" */
#define MIR_COMPILER
#define MIR_T0
#include "mir.h"


/*
|       This corresponds directly with the "-i" and "-ib" compiler flags
|       defined in mir_frontend.c
*/
extern BOOL supplementary_info;
extern BOOL supplementary_time;

/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp500();
extern char *mp501();
extern char *mp502();
extern char *mp503();
extern char *mp504();
extern char *mp505();
extern char *mp506();
extern char *mp507();
extern char *mp508();
extern char *mp509();
extern char *mp510();
extern char *mp511();
extern char *mp512();
extern char *mp513();
extern char *mp514();
extern char *mp515();
extern char *mp516();
extern char *mp517();
extern char *mp518();
extern char *mp519();
extern char *mp520();
extern char *mp521();
extern char *mp522();
extern char *mp523();
extern char *mp524();
extern char *mp525();
extern char *mp526();
extern char *mp527();
extern char *mp528();
extern char *mp529();
extern char *mp530();
extern char *mp531();
extern char *mp532();
extern char *mp533();
extern char *mp534();
extern char *mp535();
extern char *mp536();
#endif


/*
| MIR Database file "Read Context"
|
|  This structure carries context required for "mirci_fetch_next()" to 
|  return a "local MASA" that points to (the next) MIR Object residing
|  entirely in an in-memory buffer.
|
|  "magic_fseek" is a number (created by "mirci_interal_init()"), which
|  when added to any legal "masa" produces a sum, which when submitted to
|  "fseek()" will position the file so that the next read will fetch the
|  cell containing the masa.  This allows for re-reading the file to
|  refresh the buffer with an entire MIR object (that might be hanging off the
|  end of the current buffer, "broken" as it were).
|
|  "eop_masa[]" is initialized (by mirc_internal_init()) to contain the
|  "End-Of-Partition" masas for each partition.  (These values are actually
|  the *starting* addresses of the succeeding partitions in each case except
|  for the last, which is simply the first masa 'beyond' the end of the file.
|  By using the IDS_TYPE corresponding to the flavor of the partition as
|  an index, you get the first illegal masa beyond the end of that partition.
*/
typedef
    struct {
        FILE     *fileptr;    /* Binary MIR File we're doing I/O from        */
        long     magic_fseek; /* Magic Fseek Number (see above)              */
        masa     local;       /* Index to next MIR Object in "local_mas"     */
        masa     local_max;   /* Index to first cell following "local_mas"   */
        masa     external;    /* External masa for obj. indicated by "local" */
        masa     *local_mas;  /* --> Buffer containing page of MAS from file */
        masa     base_extrnl; /* External masa of 1st cell in "local_mas"    */
        unsigned mo_size;     /* Size in (masa-cells) of MIR Object @ "local"*/
        masa     sop_masa[MAX_IDS_TYPE];  /* Address of 1st cell in partition*/
                              /* specified by an index of type "IDS_TYPE".   */
        masa     eop_masa[MAX_IDS_TYPE];  /* Address of 1st cell following   */
                              /* the end of the partition specified by an    */
                              /* index of type "IDS_TYPE".                   */
        } read_context;


/*
| LOCAL_MAS_SIZE
|
|  This symbol defines how many MIR Address "cells" are contained in the
|  buffer pointed to by the "local_mas" entry in "read_context" above.
|
|  Whenever "mirci_fetch_next()" needs to read more MIR Address "space" into
|  a buffer, it will attempt to read this many cells (integers) into the local
|  mas buffer.
|
|  Note that symbol *must* be big enough to create a buffer large enough to
|  read in the largest MIR Object that the compiler placed in the MIR file.
|
|  It should be a reasonable buffer size; not so big that we blow space
|  needlessly, not so small that we do too many fread()s or we can't read
|  in the largest MIR Object; it should be a number that could conceivably
|  be an even multiple of a disk sector size.
|
|  Units of MIR Address cells: (of type "masa").
*/
#define LOCAL_MAS_SIZE 1024


/*
| "The World" - Parent of Everything (root object) 
|
|  We need access to this here because once "the World" has been copied
|  to internal form (ie into an IDS), we need to set this external cell
|  to point to it so the front-end parser (yacc grammar) knows "where to
|  start".
|
|  Defined in the parser grammer (mir_yacc.y)
*/
extern IDS     *the_world_IDS;


/*
|
|       Prototypes for module-local functions
|
*/

/* mirci_internal_init - Init for binary MIR Database file load */
static MC_STATUS
mirci_internal_init PROTOTYPE((
read_context  *,         /*--> Read-Context Block for further initialization */
int           **,        /*-->> Preamble List returned from MIR file         */
masa          *[],       /*-->> to mini-MIR Address Space (Partition Table)  */
inter_context *          /* --> Intermediate Context Block                   */
));

/* mirci_MAS_load - Load MIR Address Space Element-by-Element */
static MC_STATUS
mirci_MAS_load PROTOTYPE((
read_context      *,     /*--> Read-Context Block for further initialization */
masa              *,     /*-->> to mini-MIR Address Space (Partition Table)  */
back_end_context  *,     /* Pointer to back-end context block                */
IDS             **[]     /* Address of pointer Synonym-to-(IDS *)xlate array */
));

/* mirci_fetch_next - Fetch next MIR Object from file */
static MC_STATUS
mirci_fetch_next PROTOTYPE((
IDS_TYPE        ,        /* Type of next MIR Object we wanna fetch          */
read_context    *,       /* Read Context block.. for I/O on MIR binary file */
masa            *        /* --> the "Mini-MAS" containing Partition-Table   */
));

/* mirci_addr_xlate - Translate External Addresses to Internal */
static MC_STATUS
mirci_addr_xlate PROTOTYPE((
masa          *,         /*--> to mini-MIR Address Space (Partition Table)  */
inter_context *,         /* --> Intermediate Context Block                  */
IDS           *[]        /* Synonym-to-(IDS *) xlate array                  */
));

/* mirci_NT_addr_xlate - Translate NT External Addresses to Internal */
static MC_STATUS
mirci_NT_addr_xlate PROTOTYPE((
IDS_TYPE      ,          /* Type of Non-Terminal: DC, General, MIR-Rel       */
masa          *,         /*--> to mini-MIR Address Space (Partition Table)   */
inter_context *,         /* --> Intermediate Context Block                   */
IDS           *[]        /* Synonym-to-(IDS *) xlate array                   */
));

/* mirci_compiler_lists - Constructs Internal Lists for Compiler Front-end */
static MC_STATUS
mirci_compiler_lists PROTOTYPE((
back_end_context  *      /* Pointer to back-end context block    */
));


/* mirc_binary_load - Load binary MIR Database file into MIR Compiler */
/* mirc_binary_load - Load binary MIR Database file into MIR Compiler */
/* mirc_binary_load - Load binary MIR Database file into MIR Compiler */

MC_STATUS
mirc_binary_load (filename, bec)

char              *filename;  /* Binary MIR database filename to load */
back_end_context  *bec;       /* Pointer to back-end context block    */

/*
INPUTS:

    "filename" points to the name of the file to be opened and loaded.

    "bec" is the address of the "back-end" context block, some of whose
    data-structures must be initialized by this function.


OUTPUTS:

    On success, the function returns MC_SUCCESS:

      * The specified binary MIR database file has been read into the compiler
      * All back-end context block data structures needed to perform a
        compilation have been initialized

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.

    Purpose:
        This function takes care of "compiling" the one binary file the
        user is allowed to specify as input.  Once processing by this
        function is completed, the compiler is ready to compiler one or more
        ASCII (non-binary) MSL input files.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (attempt to open input file failed)
        <issue error msg>
        <return MC_FAILURE>

    if (attempt to perform initialization chores FAILED)
        <return status>

    if (attempt to read-in MAS to heap FAILED)
        <return status>

    if (attempt to perform address translation on in-heap structures FAILED)
        <return status>

    if (attempt to build compiler internal lists FAILED)
        <return status>

    (* Perform cleanup *)
    <close input file>
    <release "local_mas">
    <release "synonym_to_IDS">
    for (all IDS types)
        if (type is NOT "I_T_OBJECT_string")
            <release xref array for IDS type>

    <set the parser to be pointed to the IDS for "The World">
    <mark "The World" as created>
    <set top of index to first slice in index partition>

OTHER THINGS TO KNOW:

    Upon return, we're all set to do a 'normal' ASCII compile.  This implies
    that the information equivalent to that found in the "builtin-types" file
    has been extracted from the binary MIR database file.... the builtin_types
    file doesn't need to be parsed.  There could be a discrepancy.
*/      

{
read_context    rc;              /* The "Read-Context" Block               */
int             *preamble=NULL;  /* Where the Preamble array gets returned */
int             *mas=NULL;       /* Where the "mini-MAS" gets returned     */
mir_status      status;          /* Returned status                        */
IDS            **syn_to_IDS;     /* Synonym to IDS array                   */
int             i;               /* Handy-Dandy loop index                 */
char            *the_world_name; /* --> Name of MIR Object believed to be  */
                                 /*     "The World"                        */

/* if (attempt to open input file failed) */
if ( (rc.fileptr = fopen(filename, "rb")) == NULL) {
    fprintf(stderr,
            MP(mp500,"mirc - Error:\n       Attempt to open binary database file \"%s\" failed. status = %d"),
            filename,
            errno);
    return (MC_FAILURE);
    }

/* if (attempt to perform initialization chores FAILED) */
if ((status = mirci_internal_init(&rc, &preamble, &mas, &bec->i_context ))
    != MC_SUCCESS) {
    return (status);
    }

/* if (attempt to read-in MAS to heap FAILED) */
if ((status = mirci_MAS_load(&rc,
                             mas,
                             bec,
                             &syn_to_IDS)) != MC_SUCCESS) {
    return (status);
    }

/* if (attempt to perform address translation on in-heap structures FAILED) */
if ((status = mirci_addr_xlate(mas,
                               &bec->i_context,
                               syn_to_IDS)) != MC_SUCCESS) {
    return (status);
    }

/* if (attempt to build compiler internal lists FAILED) */
if ((status = mirci_compiler_lists(bec)) != MC_SUCCESS) {
    return (status);
    }

/*
| Perform cleanup
*/

/* close input file */
fclose(rc.fileptr);

/* release "local_mas" */
free(mas);

/* release "synonym_to_IDS" */
free(syn_to_IDS);

/* for (all IDS types) */
for (i=0; i < MAX_IDS_TYPE; i++) {
    
    /* if (type is NOT "I_T_OBJECT_string") */
    if ((i != I_T_OBJ_string) && (bec->i_context.xref[i] != NULL))
        {
        /* release xref array for IDS type */
        free(bec->i_context.xref[i]);
        }
    }

/*
| Set the parser to be pointed to the IDS for "The World".
|
| The first object in the General Non-Terminal partition should be
| "The World" MIR Object, and its name should be "The World".
|
*/
the_world_IDS = bec->current_object = bec->i_context.flavors[I_NT_OBJECT];

if ( (the_world_name = mirc_find_obj_NAME(the_world_IDS)) == NULL
    || strcmp(the_world_name, "The World") != 0) {
    fprintf(stderr,
            MP(mp501,"mirc - Error: The World MIR Object missing: (%s) found instead\n"),
            ((the_world_name == NULL) ?
                 ("<No Name Found>") : (the_world_name)));
    return(MC_FAILURE);
    }

/* mark "The World" as created (this is probably superfluous) */
the_world_IDS->idsu.nt.ntu.ds = DS_CREATED;

/* set top of index to first slice in index partition */
bec->i_context.index_top = bec->i_context.flavors[I_SLICE];

return(MC_SUCCESS);
}

/* mirci_internal_init - Init for binary MIR Database file load */
/* mirci_internal_init - Init for binary MIR Database file load */
/* mirci_internal_init - Init for binary MIR Database file load */

static MC_STATUS
mirci_internal_init(rc, preamble, mas, ic )

read_context  *rc;       /*--> Read-Context Block for further initialization */
int           **preamble;/*-->> Preamble List returned from MIR file         */
masa          *mas[];    /*-->> to mini-MIR Address Space (Partition Table)  */
inter_context *ic;       /* --> Intermediate Context Block                   */


/*
INPUTS:

    "rc" points to the Read-Context block whose initialization by this function
    must be completed.

    "preamble" is the address of a pointer to be set upon return to an array
    containing the "preamble" as read from the MIR file.  See the argument
    description for "mir_t0_init()" in "mir_t0.c" for a full discussion of
    this preamble.

    "mas" points to a cell to be loaded with the first portion of a MIR
    Address Space array that is just big enough to contain the Partition-Table
    Partition (as read in from the MIR file).

    "ic" points to the "Intermediate Context Block" where context (list heads)
    is maintained for functions in module "mir_intermediate.c" that create
    and manage Intermediate Data Structures (IDS).  This funtion must set-up
    some of these lists.

OUTPUTS:

    On success, the function returns MC_SUCCESS:

      * The "rc" read-context block is fully initialized
      * The "preamble" is loaded with the preamble read from the file.
      * The "mas" is loaded with the Partition-Table Partition contents
      * The "ic" list-headers for Address Cross-Reference arrays are
        initialized.

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been successfully opened.

    Purpose:
        This function takes care of "reading-in" the preamble portion of
        the MIR database file along with the contents of the Partition-Table
        Partition (the first partition in the file) for subsequent use.


ACTION SYNOPSIS OR PSEUDOCODE:

    <read "endian-ness" indicator>
    if (indicator is not "1")
        <issue error msg>
        <return MC_FAILURE>

    <read in Binary File Format Version number>
    if (version is not "2")
        <issue error msg>
        <return MC_FAILURE>

    <read in preamble size>
    if (attempt to allocate preamble array storage failed)
        <issue error msg>
        <return MC_FAILURE>

    <read in preamble>

    if (attempt to allocate a mini-MIR Address Space array storage failed)
        <issue error msg>
        <return MC_FAILURE>

    <read in the Partition-Table Partition contents to mini-MAS>
    if (read attempt failed)
        <issue error msg>
        <return MC_FAILURE>

    <initialize the rest of the read-context block (magic-number et. al.)>

    <initialize the xref_count arrays with the number of each kind of
     MIR object expected to be read-in from the MIR database file>

    for (each flavor of MIR object to be read-in)
        if (attempt to allocate storage to hold addr-xref for objects failed)
            <issue error msg>
            <return MC_FAILURE>
        <store starting address in inter-context "xref[]" array for this type>

    <copy longest oid arc count to intermediate context storage>
    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    We copy the "longest oid arc count" into the Intermediate Context block
    for reference by the final pass.  If any removes are performed, it is
    possible that this value may no longer be perfectly accurate if the remove
    happens to remove a MIR Object that had the longest OID in the index.

    It costs too much to recompute this value, either as the removes are done
    or just before the final pass occurs (to make it exactly right).  We don't
    do it, but the error is likely to waste only a few arc entries in general.

    The value remains accurate as being 'big enough' to hold at least the
    longest OID in the index.
*/      

{
int     i;                /* General purpose index & integer      */
int     pre_size;         /* Size of Preamble in bytes            */
int     total_array_size; /* Computed size of a single xref array */


/* read "endian-ness" indicator */
if (fread(&i, sizeof(i), 1, rc->fileptr) != 1) {
    fprintf (stderr,
             MP(mp502,"mirc - Error: I/O Error %d on Read\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }
/* if (indicator is not "1") */
if (i != 1) {
    fprintf (stderr,
             MP(mp503,"mirc - Error: Invalid \"Endian\" Indicator\n       Should have been \"1\": was \"%d\"\n"),
             i);
    return(MC_FAILURE);
    }

/* read in Binary File Format Version number */
if (fread(&i, sizeof(i), 1, rc->fileptr) != 1) {
    fprintf (stderr,
             MP(mp502,"mirc - Error: I/O Error %d on Read\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }
/* if (version is not "2") */
if (i != 2) {
    fprintf (stderr,
             MP(mp504,"mirc - Error: Invalid Binary File Format Version Indicator\n       Should have been \"2\": was \"%d\"\n"),
             i);
    return(MC_FAILURE);
    }

/* read in preamble size */
if (fread(&pre_size, sizeof( int ), 1, rc->fileptr) != 1) {
    fprintf (stderr,
             MP(mp502,"mirc - Error: I/O Error %d on Read\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }

/* if (attempt to allocate preamble array storage failed) */
if ( (*preamble = (int *) malloc(pre_size)) == NULL) {
    fprintf(stderr,
            MP(mp505,"mirc - Error: Out of Memory\n       while allocating MAS Preamble of size %d"),
            pre_size);
    return(MC_FAILURE);
    }

/* read in preamble */
if (fread((*preamble), pre_size, 1, rc->fileptr) != 1) {
    fprintf (stderr,
             MP(mp502,"mirc - Error: I/O Error %d on Read\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }

/*
| Here we allocate space just large enough to read-in the beginning of the
| "MIR Address Space" (MAS). . . that begining portion that includes the
| Partition-Table partition (which describes where everything else is
| located).  The "read-point" for the input file is now positioned to the
| first cell of the MAS (masa "1").  The first cell of MAS contains the first
| cell of the Partition-Table partition.  While we're here, let us compute
| the "magic-fseek" number.  The code in other sections of this module expect
| to be able to take the "magic-fseek" number from the rc (read-context) block,
| add it to (sizeof(int)*(the masa they seek to read-in next)), do an "fseek"
| using this sum, and do the read.  Compute this "magic-fseek" number now.
| It is whatever "ftell" returns MINUS (sizeof(int)*ONE).
|
*/
if ((rc->magic_fseek = ftell(rc->fileptr)) == -1) {
    fprintf (stderr,
             MP(mp506,"mirc - Error: \"ftell()\" Error %d\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }
rc->magic_fseek -= sizeof(int);


/*
| Note:  Here we want to allocate enough space to hold all the entries in
| the first part of the MAS *up to but not including* the first cell of
| the first partition following the Partition-Table Partition (this is the
| first cell of the SLICE Partition).  The 1st cell of the SLICE partition
| starts at the MASA given by the symbol FIRST_MASA_AFTER_P_TABLE.  While we
| are not interested in reading in THAT CELL, (we want to read the number of
| cells given by the value of "FIRST_MASA_AFTER_P_TABLE-1"), we need to
| allocate ONE MORE (4-byte) CELL in the array, because MASA of "0" is not
| legal (and the entry in the array is not used).  Consequently the number of
| cells we want to ALLOCATE is "FIRST_MASA_AFTER_P_TABLE-1+1", and
| we'll READ from the file into the second cell (whose masa is "1": mas[1])
| ONE LESS CELL than we allocated space for.
|
| if (attempt to allocate a mini-MIR Address Space array storage failed)
*/
if ((*mas = (int *) malloc((FIRST_MASA_AFTER_P_TABLE-1+1)*sizeof(int)))
    == NULL){
    fprintf(stderr,
            MP(mp507,"mirc - Error: Out of Memory\n       while allocating Mini-MAS  of size %d"),
            ((FIRST_MASA_AFTER_P_TABLE-1)*sizeof(int)));
    return(MC_FAILURE);
    }

/* read in the Partition-Table Partition contents to mini-MAS */
/* if (read attempt failed) */
if (fread((*mas+1),
            ((FIRST_MASA_AFTER_P_TABLE-1)*sizeof(int)),
             1,
             rc->fileptr) != 1) {
    fprintf(stderr,
             MP(mp502,"mirc - Error: I/O Error %d on Read\n       on binary MIR database file"),
             errno);
    return(MC_FAILURE);
    }

/*
| initialize the rest of the read-context block (magic-number et. al.)
*/
rc->local = 1;                                /* Bogus: Just to give a value */
rc->local_max = rc->local + LOCAL_MAS_SIZE;   /* Bogus: Likewise             */
rc->external = 1;                             /* Bogus: Likewise             */

/*  (Still setting up the read-context block. . . )
|
| Allocate a buffer to read the rest of the MAS into a 'page' at a time.  Note
| that this array is used as a "1"-origined array, so we allocate an extra
| cell we never read.
*/
if ( (rc->local_mas = (int *) malloc((LOCAL_MAS_SIZE+1)*sizeof(int))) == NULL){
    fprintf(stderr,
            MP(mp508,"mirc - Error: Out of Memory\n       while allocating local MAS buffer of size %d"),
            (LOCAL_MAS_SIZE+1)*sizeof(int));
    return(MC_FAILURE);
    }

/*  (Still setting up the read-context block. . . )                  */
rc->base_extrnl = 0;          /* Signnal "Nothing In Buffer"         */
rc->mo_size = 0;              /* Likewise no MIR Object detected yet */


/*  (Still setting up the read-context block. . . )
|
| Initialize the Start-Of-Partition MASA array:
|
|    *  For each partition store the address of the first cell in that
|       partition.
|       
*/
rc->sop_masa[I_SLICE]            = (*mas)[ROOT_INDEX_MASA];
rc->sop_masa[I_SUBREGISTER]      = (*mas)[START_SUBREG_MASA];
rc->sop_masa[I_T_OBJ_snumber]    = (*mas)[START_SNUMBER_MASA];
rc->sop_masa[I_T_OBJ_unumber]    = (*mas)[START_UNUMBER_MASA];
rc->sop_masa[I_T_OBJ_string]     = (*mas)[START_STRING_MASA];
rc->sop_masa[I_NT_OBJECT_DC]     = (*mas)[START_NONTERM_DC_MASA];
rc->sop_masa[I_NT_OBJECT]        = (*mas)[START_NONTERM_MASA];
rc->sop_masa[I_NT_OBJECT_Rel]    = (*mas)[START_NONTERM_REL_MASA];


/*  (Still setting up the read-context block. . . )
|
| Initialize the End-Of-Partition MASA array:
|
|    *  For each partition store the first address
|       FOLLOWING the LAST LEGAL cell in that partition.
|       
*/
rc->eop_masa[I_SLICE]            = (*mas)[START_SUBREG_MASA];
rc->eop_masa[I_SUBREGISTER]      = (*mas)[START_SNUMBER_MASA];
rc->eop_masa[I_T_OBJ_snumber]    = (*mas)[START_UNUMBER_MASA];
rc->eop_masa[I_T_OBJ_unumber]    = (*mas)[START_STRING_MASA];
rc->eop_masa[I_T_OBJ_string]     = (*mas)[START_NONTERM_DC_MASA];
rc->eop_masa[I_NT_OBJECT_DC]     = (*mas)[START_NONTERM_MASA];
rc->eop_masa[I_NT_OBJECT]        = (*mas)[START_NONTERM_REL_MASA];
rc->eop_masa[I_NT_OBJECT_Rel]    = (((*preamble)[PRE_MAS_SIZE])/4)
                                   +1; /* +1 = 1 more after last legal cell */

/* - - - - - - - -Read Context Block initialization Complete - - - - - - - - */


/*
|  Initialize the xref_count arrays (in the back-end context) with the number
|  of each kind of MIR object expected to be read-in from the MIR database
|  file.  We get the info from the preamble we just read in.
*/
ic->xref_count[I_SLICE]            = (*preamble)[PRE_SLICE_CNT];
ic->xref_count[I_SUBREGISTER]      = (*preamble)[PRE_SUBREG_CNT];
ic->xref_count[I_T_OBJ_snumber]    = (*preamble)[PRE_SIGNED_CNT];
ic->xref_count[I_T_OBJ_unumber]    = (*preamble)[PRE_UNSIGNED_CNT];
ic->xref_count[I_T_OBJ_string]     = (*preamble)[PRE_STRING_CNT];
ic->xref_count[I_NT_OBJECT_DC]     = (*preamble)[PRE_NT_DC_CNT];
ic->xref_count[I_NT_OBJECT]        = (*preamble)[PRE_NT_GEN_CNT];
ic->xref_count[I_NT_OBJECT_Rel]    = (*preamble)[PRE_NT_REL_CNT];


/* for (each flavor of MIR object to be read-in) */
for (i=0; i < MAX_IDS_TYPE; i++) {

    ic->xref[i] = NULL;
    total_array_size = sizeof(ext_int_xref) * ic->xref_count[i];

    /* if (attempt to allocate storage to hold addr-xref for objects failed) */
    /* store starting address in inter-context "xref[]" array for this type */
  if (total_array_size != 0)
    if ((ic->xref[i] = (ext_int_xref *) malloc(total_array_size)) == NULL) {
        fprintf(stderr,
                MP(mp509,"mirc - Error: Out of Memory\n       while allocating xref list of size %d"),
                total_array_size);
        return(MC_FAILURE);
        }
    }

/* copy longest oid arc count to intermediate context storage */
ic->arc_count = (*preamble)[PRE_MAX_ARCS];

return (MC_SUCCESS);
}

/* mirci_MAS_load - Load MIR Address Space Element-by-Element */
/* mirci_MAS_load - Load MIR Address Space Element-by-Element */
/* mirci_MAS_load - Load MIR Address Space Element-by-Element */

static MC_STATUS
mirci_MAS_load(rc, mas, bec, syn_to_IDS)

read_context      *rc;   /*--> Read-Context Block for further initialization */
masa              *mas;  /*-->> to mini-MIR Address Space (Partition Table)  */
back_end_context  *bec;  /* Pointer to back-end context block                */
IDS             ***syn_to_IDS; /* Synonym-to-IDS xlate array                 */

/*
INPUTS:

    "rc" points to the Read-Context block whose initialization by this function
    must be completed.

    "mas" points to a cell loaded with the first portion of a MIR
    Address Space array that is just big enough to contain the Partition-Table
    Partition (as read in from the MIR file).

    "bec" points to the back-end context where cross-reference arrays are
    maintained along with:

        "ic" points to the "Intermediate Context Block" where context
        (list heads) is maintained for functions in module
        "mir_intermediate.c" that create and manage Intermediate Data
        Structures (IDS).  This funtion reads some of these lists.

    "syn_to_IDS" - points to a cell to be initialized on successful return
    to an array of pointers to IDS structures for each MIR Relationship object
    arranged by synonym.

OUTPUTS:

    On success, the function returns MC_SUCCESS:

      * Each MIR Object in every partition within the MIR database file
        has been read into a corresponding in-heap structure.  All pointers
        within these structures remain "externalized" (as copied from the
        file).

      * The "syn_to_IDS" array has been initialized and filled with pointers.

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been opened, the preamble and Partition-Table partition
        have been read in, and we're ready to read in the rest (bulk) of
        the MIR database file.

    Purpose:
        This function takes care of copying the "external" representation
        of each MIR object into an in-heap "intermediate" representation.
        All data are copied, but pointers are not translated into the
        in-heap equivalent (they remain "externalized").


ACTION SYNOPSIS OR PSEUDOCODE:

    <set synonym generator to 1>
    if (attempt to allocate storage for synonym-to-IDS array failed)
        <issue message>
        <return MC_FAILURE>

    for (each type of MIR Object (ie for every partition))

        <set counter-read to 0 (for indexing into xref arrays)>
        <obtain a copy of number of objects expected for this type>
        <obtain address of xref array to use>

        while ( next object is not End-Of-Partition)

            if (attempt to read next failed)
                <return MC_FAILURE>

            if (object count is zero)
                <issue Count-Mismatch message>
                <return MC_FAILURE>

            switch (type of MIR Object)

                case I_SLICE:
                    <obtain the number of entries required>
                    if (I_Create_IDS - attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>

                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>

                    <copy back pointer to new IDS>

                    for (each entry in the slice)
                        <copy the arc number>
                        <copy the external address>
                    <load the entry count to be the size of the slice>
                    <break>


                case I_SUBREGISTER:
                    if (I_Create_IDS - attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>

                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>

                    <copy back pointer>
                    <copy the object address>
                    <copy the slice address>
                    <break>


                case I_T_OBJ_snumber:
                    if (I_AddNew_IDS_SNUMBER-attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>
                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>
                    <break>


                case I_T_OBJ_unumber:
                    if (I_AddNew_IDS_UNUMBER-attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>
                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>
                    <break>


                case I_T_OBJ_string:
                    if (I_AddNew_IDS_STRING attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>
                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>
                    <break>


                case I_NT_OBJECT_Rel:
                case I_NT_OBJECT_DC:
                case I_NT_OBJECT:
                    <obtain the number of entries required>
                    if (I_Create_IDS--attempt to allocate space failed)
                        <issue message>
                        <return MC_FAILURE>

                    <store in-heap IDS address & external address in
                     "counter-read" entry of xref[] array>

                    if (it is a I_NT_OBJECT_Rel)
                        <load synonym generator into IDS>
                        <insert IDS address into synonym_to_IDS array>
                        <increment synonym generator>

                    for (each OID backpointer in external object)
                        <extract the SMI and raw backpointer value>
                        <record the backpointer in next slot of local array>
                        <record the backpointer in external form in IDS in
                         backpointer array under the SMI>
                        (* SRs handled correctly w/o special handling *)
                        for (each previously locally-stored backpointer)
                            if (it matches current backpointer)
                                <increment IDS backarc entry for SMI>

                    for (each entry in the relationship table)
                        <copy the synonym to relationship table entry>
                        <copy the external address to target entry>
                    <load the entry count to be the size of the Rel Table>

                    <break>
                (* End of SWITCH *)

            <increment the "counter-read" index for xref array>
            <decrement object count>
            (* End of WHILE *)

        if (object count is NOT zero)
            <issue Count-Mismatch message>
            <return MC_FAILURE>

        (* End of FOR *)

    <return pointer to synonym-to-IDS array>
    <return MC_SUCCESS>

OTHER THINGS TO KNOW:

    There is a bogosity in this code:  The code presumes the cells in
    structures "SES" and "RES" that are used to point to in-heap structures
    are big enough to stuff an "integer" into.  Big trouble in a 16-bit Intel
    environment. . . but probably nothing we'll ever have to worry about.
    Look for the comment "Unconverted".
*/      

/* #define DEBUG_EXTERNAL */
{
unsigned        syn_gen;        /* Synonym Generator (for Rel. NTs)         */
int             mo_type;        /* MIR Object type (currently working on    */
int             xref_ctr;       /* Index into xref array being used         */
int             type_count;     /* Count for number of objs of current type */
ext_int_xref   *xr;             /* Cross-Reference Array for current type   */
mir_status      status;         /* Fetch-Next status returned               */
int             entry_count;    /* Number of Slots in Slice/Entries in NT   */
masa           *object;         /* Local name for the buffer holding object */
masa            cell;           /* Local name for index to object           */
int             size;           /* Storage size to alloacte                 */
int             bptr_cnt;       /* Backpointer count                        */
masa            bptr_masa;      /* Extracted Backpointer for OID            */
int             rel_cnt;        /* Relationship Table Entry  count          */
int             i;              /* Handy-Dandy index & integer              */
int             j;              /* Handy-Dandy index & integer              */
int             k;              /* Handy-Dandy index & integer              */
int             local_bptr[MAX_OID_SMI]; /* Store backptrs in local 'stac'  */
mir_oid_smi     oid_smi;        /* SMI Code for OID backpointer             */
IDS            *in_heap;        /* Pointer to in-heap representation        */
IDS           **synonym_to_IDS; /* Using "synonym" as index gives IDS of    */
                                /* MIR Relationship object for that synonym */


/* set synonym generator to 1 */
syn_gen = 1;

/*
|  Get some space for the synonym_to_IDS array.
|  This array is 1-origined, so we allocate an extra cell.
*/
size = (bec->i_context.xref_count[I_NT_OBJECT_Rel]+1) * sizeof(IDS *);
if ( (synonym_to_IDS = (IDS **) malloc(size)) == NULL) {
    fprintf(stderr,
            MP(mp510,"mirc - Error: Out of Memory\n       while allocating syn-to-IDS list of size %d"),
            size);
    return(MC_FAILURE);
    }

/* for (each type of MIR Object (ie for every partition)) */
for (mo_type = 0; mo_type < MAX_IDS_TYPE; mo_type++) {

    /* set counter-read to 0 (for indexing into xref arrays) */
    xref_ctr = 0;

    /* obtain a copy of number of objects expected for this type */
    type_count = bec->i_context.xref_count[mo_type];

    /* obtain address of xref array to use */
    xr = bec->i_context.xref[mo_type];

    /* while ( next object is not End-Of-Partition) */
    while ((status = mirci_fetch_next(mo_type, rc, mas))
           != MC_END_OF_PARTITION) {

        /* if (attempt to read next failed) */
        if (status == MC_FAILURE)
            return (MC_FAILURE);

        /* if (object count is zero) */
        if (type_count == 0) {
            /* issue Count-Mismatch message */
            fprintf(stderr,
                    MP(mp511,"mirc - Error: MIR Input file Object/Count Mismatch\n       while processing Type %d"),
                    mo_type);
            return (MC_FAILURE);
            }

        object = rc->local_mas;         /* Grab local name for buffer      */
        cell = rc->local;               /* Grab local index to this object */

        switch (mo_type) {  /* type of MIR Object */

            /* SLICE
            | A slice (@ "cell") is shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            MASA Backpointer to Previous Slice
            |     +1            Count of Number of Entries (Slots) in Slice
            |     +2            Start of first Entry (Slot)
            |     Within Slot:
            |         +0        Arc Number
            |         +1        Associated MASA of "thing": either Slice,
            |                              subregister or Non-Terminal
            */
            case I_SLICE:
                /* obtain the number of entries required */
                entry_count = object[cell+1];

                /* if (I_Create_IDS - attempt to allocate space failed) */
                if ( (in_heap = I_Create_IDS(I_SLICE,
                                             entry_count)) == NULL) {

                    fprintf(stderr,
                            MP(mp512,"mirc - Error: Out of Memory\n       while allocating SLICE w/size %d"),
                            entry_count);
                    return (MC_FAILURE);
                    }

#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;

                /* copy back pointer to new IDS: Unconverted */
                in_heap->idsu.i.backptr = (IDS *) object[cell];

                /* for (each entry in the slice (2 cells/entry)) */
                for (i=0, j=0; i < entry_count; i++, j+=2) {

                    /* copy the arc number */
                    in_heap->idsu.i.slice[i].iso_arc =
                        (unsigned int) object[cell+2+0+j];

                    /* copy the external address: Unconverted */
                    in_heap->idsu.i.slice[i].next =
                        (IDS *) object[cell+2+1+j];
                    }

                /* load the entry count to be the size of the slice */
                in_heap->idsu.i.entry_count = entry_count;
                break;


            /* SUBREGISTER
            | A Subregister (@ "cell") is shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            MASA Backpointer to 'Owning' Slice
            |     +1            MASA of associated Non-Terminal
            |     +2            MASA of next-lower Slice
            */
            case I_SUBREGISTER:
                /* if (I_Create_IDS - attempt to allocate space failed) */
                if ( (in_heap = I_Create_IDS(I_SUBREGISTER,0)) == NULL) {
                    fprintf(stderr,
                            MP(mp513,"mirc - Error: Out of Memory\n       while allocating SUBREGISTER")
                            );
                    return (MC_FAILURE);
                    }
#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;

                /* copy back pointer to new IDS: Unconverted */
                in_heap->idsu.s.backptr = (IDS *) object[cell];

                /* copy the object address: Unconverted */
                in_heap->idsu.s.ntobject = (IDS *) object[cell+1];

                /* copy the slice address: Unconverted */
                in_heap->idsu.s.lower_levels = (IDS *) object[cell+2];
                break;


            /* SIGNED NUMBER
            | A Signed Number (@ "cell") is shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            Signed Number
            */
            case I_T_OBJ_snumber:
                /* if (I_AddNew_IDS_SNUMBER-attempt to allocate space failed)*/
                if ( (in_heap = I_AddNew_IDS_SNUMBER(object[cell])) == NULL) {
                    fprintf(stderr,
                            MP(mp514,"mirc - Error: Out of Memory\n       while allocating SIGNED NUMBER")
                            );
                    return (MC_FAILURE);
                    }
#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;
                break;


            /* UNSIGNED NUMBER
            | A Unsigned Number (@ "cell") is shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            Signed Number
            */
            case I_T_OBJ_unumber:
                /* if (I_AddNew_IDS_UNUMBER-attempt to allocate space failed)*/
                if ( (in_heap = I_AddNew_IDS_UNUMBER(
                                                     (unsigned int)
                                                          object[cell]
                                                     )
                      ) == NULL) {
                    fprintf(stderr,
                            MP(mp515,"mirc - Error: Out of Memory\n       while allocating UNSIGNED NUMBER")
                            );
                    return (MC_FAILURE);
                    }
#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;
                break;


            /* STRING
            | A String (@ "cell") is shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            Length
            |     +1            Start of String
            |      .             (goes on for however long
            |      .              and is terminated w/null byte)
            */
            case I_T_OBJ_string:
                /* if (I_AddNew_IDS_STRING attempt to allocate space failed) */
                if ( (in_heap = I_AddNew_IDS_STRING(
                                                     (char *) &object[cell+1]
                                                    )
                      ) == NULL) {
                    fprintf(stderr,
                            MP(mp516,"mirc - Error: Out of Memory\n       while allocating STRING")
                            );
                    return (MC_FAILURE);
                    }
#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;
                break;


            /* NON-TERMINALS
            | All varieties of NON-TERMINALS (@ "cell") are shaped like this:
            |  
            |  Masa Offset      Contents
            |     +0            Packed Entry Count & OID Backpointer Count
            |     +1            Start of However-Many OID Backpointers
            |                     (possibly 0)
            |     +n            Start of first Relationship Table Entry
            |                     (where "n" is number of OID Backpointers
            |                      present)
            |     Within Entry:
            |         +0        Packed MIR Rel. Synonym & Packed Target MASA
            |         .
            |         .
            |         . . .for however many Relationship Table Entries
            |              specified (one Entry per MIR Address Space cell)
            */
            case I_NT_OBJECT_Rel:
            case I_NT_OBJECT_DC:
            case I_NT_OBJECT:
                /* obtain the number of entries required */
                rel_cnt = object[cell] & mas[ENTRY_COUNT_AMASK_MASA];

                /* Get the count of the number of OID backpointers */
                bptr_cnt = ((unsigned int) object[cell])
                                        >> mas[OID_COUNT_RSHIFT_MASA];

                /* if (I_Create_IDS - attempt to allocate space failed) */
                if ( (in_heap = I_Create_IDS(mo_type, rel_cnt)) == NULL) {
                    fprintf(stderr,
                            MP(mp517,"mirc - Error: Out of Memory\n       while allocating NT w/size %d"),
                            rel_cnt);
                    return (MC_FAILURE);
                    }

#ifdef DEBUG_EXTERNAL
                /* Store External Address to aide in debugging */
                in_heap->ex_add = rc->external;
#endif
                /*
                |  store in-heap IDS address & external address in
                |  "counter-read" entry of xref[] array
                */
                xr[xref_ctr].int_add = in_heap;
                xr[xref_ctr].ext_add = rc->external;

                /* if (it is a I_NT_OBJECT_Rel) */
                if (mo_type == I_NT_OBJECT_Rel) {

                    /* load (external) synonym generator into IDS */
                    in_heap->idsu.nt.ntu.synonym = syn_gen;

                    /* insert IDS address into synonym_to_IDS array */
                    synonym_to_IDS[syn_gen] = in_heap;

                    syn_gen += 1;  /* increment synonym generator */
                    }

                cell += 1;      /* Step up to next cell in object */

                /* What is going on in this loop. . .
                |
                |  The order of the backpointers in the original NT in the
                |  file may reflect the order of the corresponding pointers
                |  to this NT from an index slice IF it turns out that more
                |  than one OID (in more than one SMI) is identical except
                |  for the last arc.  Here we are temporarily using the
                |  "backarc" array in the IDS to hold THE ORDINAL POSITION
                |  in the slice for the ith backpointer in the NT.  Later
                |  on during conversion in "mirci_NT_addr_xlate()" processing
                |  the backarc entry will be converted into a true arc number.
                |
                |  Note that for the usual case, the backarc value only goes
                |  to "1".
                |
                |  for (each OID backpointer in external object)
                */
                for (i=0, j=0; i < bptr_cnt; i++) {

                    /* extract the SMI and raw backpointer value */
                    bptr_masa = object[cell+i] & mas[OID_BPTR_AMASK_MASA];
                    oid_smi = ((unsigned int) object[cell+i])
                               >> mas[OID_SMI_RSHIFT_MASA];

                    /* record the backpointer in next slot of local array */
                    local_bptr[j] = bptr_masa;

                    /*
                    |  record the backpointer in external form in IDS in
                    |  backpointer array under the SMI: Unconverted
                    |
                    |  NOTE: Backpointers to Subregisters are handled
                    |        correctly w/o special handling.
                    */
                    in_heap->idsu.nt.oid_backptr[oid_smi] =
                        (IDS *) bptr_masa;

                    /* for (each previously locally-stored backpointer) */
                    for (k=j; k >= 0; k--) {

                        /* if (it matches current backpointer) */
                        if (local_bptr[k] == bptr_masa) {
                            /* increment IDS backarc entry for SMI */
                            in_heap->idsu.nt.oid_backarc[oid_smi] += 1;
                            }
                        }

                    j += 1;     /* Show another recorded locally */

                    }

                /* for (each entry in the relationship table) */
                for (i=0; i < rel_cnt; i++) {
                    /*
                    | copy the synonym to relationship table entry: 
                    | Unconverted.
                    */
                    in_heap->idsu.nt.rel_table[i].rel_obj =
                     (IDS *)
                         (
                          object[cell+bptr_cnt+i] & mas[SYNONYM_AMASK_MASA]
                         );

                    /*
                    |  copy the external address to target entry:
                    |  Unconverted.
                    */
                    in_heap->idsu.nt.rel_table[i].tar_obj =
                        (IDS *) 
                            (
                                  ((unsigned int) object[cell+bptr_cnt+i])
                               >> mas[TARGET_RSHIFT_MASA]
                            );
                    }

                /* load the entry count to be the size of the Rel Table */
                in_heap->idsu.nt.entry_count = rel_cnt;
                break;
            }             /* End of SWITCH */

        /* increment the "counter-read" index for xref array */
        xref_ctr += 1;

        /* decrement object count */
        type_count -= 1;

        } /* End of WHILE */

    /* if (object count is NOT zero) */
    if (type_count != 0) {
        /* issue Count-Mismatch message */
        fprintf(stderr,
                MP(mp518,"mirc - Internal Error: Type Count %d not zero for type %d"),
                type_count, mo_type);

        return(MC_FAILURE);
        }
    }    /* End of FOR */

/* return pointer to synonym-to-IDS array */
*syn_to_IDS = synonym_to_IDS;

return(MC_SUCCESS);
}

/* mirci_fetch_next - Fetch next MIR Object from file */
/* mirci_fetch_next - Fetch next MIR Object from file */
/* mirci_fetch_next - Fetch next MIR Object from file */

static MC_STATUS
mirci_fetch_next(mo_type, rc, mas)

IDS_TYPE        mo_type; /* Type of next MIR Object we wanna fetch          */
read_context    *rc;     /* Read Context block.. for I/O on MIR binary file */
masa            *mas;    /* --> the "Mini-MAS" containing Partition-Table   */

/*
INPUTS:

    "mo_type" - indicates the kind of MIR Object the caller expects to get
    next.  We return failure if we're "out" of that kind of MIR Object (ie
    we've exhausted the partition).

    "rc" - the "read context" block needed to do I/O on the MIR database file
    and to keep track of "where we are".  Some Fields in this block are
    returned to the caller on success (see "OUTPUTS").  On input:

        "base_extrnl" - if ZERO on entry, this function presumes that the
        next MIR object to be returned is the first MIR Object of the
        "mo_type" (ie the first MIR Object in that partition).  The internal
        buffer will be completely refreshed starting with the first MIR Object.

        The other fields are for use exclusively by this function.

    "mas" is the address of the "mini"-MAS containing the Partition-Table.
    Macros referenced by "mirci_fetch_next()" reference the Partition-Table
    via this name.


OUTPUTS:

    On success, the function returns MC_SUCCESS:

      * If the next MIR Object of the specified type is available in
        the "local_mas" buffer, then the following fields in the
        read-context block are set:

        "local" - This function sets this to a value which can be used to
        point into "local_mas" (e.g. "local_mas[local]") to fetch the first
        cell of the next MIR Object.  mirci_fetch_next() guarantees that the
        next entire MIR object will reside in "local_mas".  The caller may not
        change this value, although the caller is expected to reference it.

        "external" - This function sets this to the external MAS address
        (masa) of the MIR object that "local" points to.  The caller may
        reference this, but may not change it.  

        "local_mas" - Points to the buffer containing the MIR Object being
        returned.

    If there are no more MIR objects of the specified type to be returned,
    then MC_END_OF_PARTITION is returned.  (The caller should change to the
    next MIR Object type desired and call again).

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been opened, the preamble read, and the read_context
        block needed by this function has been setup.  The caller of this
        function (mirci_MAS_load()) needs to read in each MIR Object in the
        binary file.

    Purpose:
        This function takes care of the details of buffering-in just enough
        of the MIR database binary file to make the next MIR object available
        in an in-memory buffer.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (base external address is zero)
        <set local to size of buffer to trigger re-read>
        <set external to base of "mo_type"'s partition>
        <set mo_size to zero (so external & local don't change)>
    else
        (* Check to see if another MIR Object exists in this partition *)
        if (external + mo_size > end of mo_type's partition)
            <issue misalignment error message>
            <return MC_FAILURE>
        else if (external + mo_size == end of mo_type's partition)
            <return MC_END_OF_PARTITION>

    (*
      Step "forward" in the local buffer (and also in the external address
      space) to the next MIR Object
    *) 
    <add "mo_size" to "local">
    <add "mo_size" to "external": new read address>

    (* If the START of the next object is outside the "local_mas" buffer,
           THEN Perform a (re-)read to put start of next object at beginning
                of buffer
    *)
    if (local is greater than or equal to "local_max")
        <reset local to "1">
        <fseek on magic_fseek + (new external value * sizeof(int)>
        <read LOCAL_MAS_SIZE integers to "local_mas">
        <set base_external to external>
        if (read returned something other than LOCAL_MAS_SIZE)
            if (read returned ZERO)
                <issue misalignment error message>
                <return MC_FAILURE>

        <set local_max to value returned PLUS ONE>

     (*
        For variable-length MIR objects, we only need to see the first cell to
        tell how long it is.
     *)
     <compute size of next MIR Object at "local", store into mo_size>

     (*
       If the END of the next object is not within the "local_mas" buffer
            THEN Perform a (re-)read to put start of next object at beginning
                 of buffer
     *)
     if (local + mo_size is greater than or equal to "local_max")
        <reset local to "1">
        <fseek on magic_fseek + (new external value * sizeof(int)>
        <read LOCAL_MAS_SIZE integers to "local_mas">
        <set base_external to external>
        if (read returned something other than LOCAL_MAS_SIZE)
            if (read returned ZERO)
                <issue misalignment error message>
                <return MC_FAILURE>

        <set local_max to value returned PLUS ONE>

    (*
       If the END of the next object is STILL not within the "local_mas" buffer
           THEN <issue error msg and return>
    *)
    if (local + mo_size is greater than or equal to "local_max")
        <issue buffer-too small error msg>
        <return MC_FAILURE>

     <return MC_SUCCESS>

OTHER THINGS TO KNOW:

   The code above has to be able to handle an "empty" partition.

   Note that the normal mode of operation is to call initially with
   "base_extrnl" set to zero w/mo_type = I_SLICE.  Repeated calls are made
   until "MC_END_OF_PARTITION" is returned.  Then, to switch to the next
   partition, simply call in again with the NEXT flavor of MIR Object type
   expected in the file (e.g. I_SUBREGISTER).  This is expected to continue
   until the last MIR Object in the last partition is read.

   So, in general, it is expected that each partition be read *in the order*
   that they appear in the MIR database file.  You *should* be able (however)
   to read starting (in general) with the first MIR object in *any* partition,
   by simply zeroing "base_extrnl" (signalling an empty buffer) and then
   calling in with the MIR Object type of the partition desired.  This has
   not been tested though!

   Note that the "local_mas" buffer is actually allocated one-word bigger
   than the number of words expected to be read into it.  This is in keeping
   with the philosophy that "masa"s never have a value of "0".  So we read
   into this buffer starting in position 1.

*/      

{
long    new_fseek;      /* Newly Computed fseek value */
int     status;         /* Status from fread et.al.   */


/*
| If the base external address is zero, then it is a "reset" situation.
|
| if (base external address is zero)
*/
if (rc->base_extrnl == 0) {

    /* set local to size of buffer + 1 to trigger re-read */
    rc->local = LOCAL_MAS_SIZE+1;

    /* set external to base of "mo_type"'s partition */
    rc->external = rc->sop_masa[mo_type];

    /* set mo_size to zero (so external & local don't change) */
    rc->mo_size = 0;
    }
else {
    /* Check to see if another MIR Object exists in this partition
    |
    | if (external + mo_size > end of mo_type's partition)
    */
    if ( (rc->external + rc->mo_size) > rc->eop_masa[mo_type]) {
        fprintf(stderr,
        MP(mp519,"mirc - Error: Binary MIR File internal alignment #0 error\n       mo_size = %u  external = %d  mo_type code %d"),
                rc->mo_size, rc->external, mo_type);
        return(MC_FAILURE);
        }
    else if ((rc->external + rc->mo_size) == rc->eop_masa[mo_type]) {
        return(MC_END_OF_PARTITION);
        }
    }

/*
|  Step "forward" in the local buffer (and also in the external address
|  space) to the next MIR Object
*/
rc->local += rc->mo_size;    /* add "mo_size" to "local"                     */
rc->external += rc->mo_size; /* add "mo_size" to "external": new read address*/

/*
| If the START of the next object is outside the "local_mas" buffer,
|    THEN Perform a (re-)read to put start of next object at beginning
|          of buffer
|
| if (local is greater than or equal to "local_max")
*/
if (rc->local >= rc->local_max) {
    rc->local = 1;       /* reset local to "1" */

    new_fseek = rc->magic_fseek + (rc->external * sizeof(int));

   /* fseek on magic_fseek + (new external value * sizeof(int) */
    if (fseek(rc->fileptr, new_fseek, SEEK_SET) == -1) {
        fprintf(stderr,
                MP(mp520,"mirc - Error: Binary MIR File fseek error\n       fseek value %ld"),
                new_fseek);
        return(MC_FAILURE);
        }

    /* read LOCAL_MAS_SIZE integers to "local_mas" */
    if ( (status = fread(&(rc->local_mas[1]),
                         sizeof(int),
                         LOCAL_MAS_SIZE,
                         rc->fileptr)) < 0) {
        fprintf(stderr,
                MP(mp521,"mirc - Error: Binary MIR File fread error\n       errno  %d"),
                errno);
        return (MC_FAILURE);
        }

    rc->base_extrnl = rc->external;   /* set base_external to external */

    /* if (read returned something other than LOCAL_MAS_SIZE) */
    if (status != LOCAL_MAS_SIZE) {

        /* if (read returned ZERO) */
        if ( status == 0) {
            fprintf(stderr,
                    MP(mp522,"mirc - Error: Binary MIR File internal alignment #1 error\n       mo_size = %u  external = %d  mo_type code %d status = %d"),
                rc->mo_size, rc->external, mo_type, status);
            return(MC_FAILURE);
            }
        }

    /* set local_max to value returned PLUS ONE */
    rc->local_max = status + 1;
    }

/*
|   For variable-length MIR objects, we only need to see the first cell to
|   tell how long it is.
*/

/* compute size of next MIR Object at "local", store into mo_size */
switch (mo_type) {
    case I_SLICE:
        /*
        | The length of a slice in MIR Address Space cells is 1 for backptr +
        | 1 for the entry count slot + (number of entries times 2).
        */
        rc->mo_size = 2 + (rc->local_mas[rc->local+1]*2);
        break;

    case I_SUBREGISTER:
        /*
        | The length of a subregister in MIR Address Space cells is always 3.
        */
        rc->mo_size = 3;
        break;

    case I_T_OBJ_snumber:
    case I_T_OBJ_unumber:
        /*
        | The length of a number in MIR Address Space cells is always 1.
        */
        rc->mo_size = 1;
        break;
        
    case I_T_OBJ_string:
        /*
        | The length of a string in MIR Address Space cells is 1 (for length
        | word) + (plus number of bytes + null byte)/4
        | (+ 1 if modulo 4 is not 0).
        */
        rc->mo_size = 1 + ((rc->local_mas[rc->local]+1)/4);
        if ( ((rc->local_mas[rc->local]+1) % 4) != 0)
            rc->mo_size += 1;
        break;

    case I_NT_OBJECT_DC:
    case I_NT_OBJECT:
    case I_NT_OBJECT_Rel:
        /*
        | The length of any kind of Non-Terminal in MIR Address Space cells
        | is 1 (for OID/Entry count cell) + number of Relationship Table 
        | entries + number OID Backpointers.
        */
        rc->mo_size = 1 + (rc->local_mas[rc->local]
                           & mas[ENTRY_COUNT_AMASK_MASA])
                    + (((unsigned int) rc->local_mas[rc->local])
                                        >> mas[OID_COUNT_RSHIFT_MASA]);
        break;
    }

        
/*
|  If the END of the next object is not within the "local_mas" buffer
|       THEN Perform a (re-)read to put start of next object at beginning
|           of buffer
*/
/* if (local + mo_size-1) is greater than or equal to "local_max") */
if ((rc->local + (rc->mo_size-1)) >= rc->local_max) {
    rc->local = 1;       /* reset local to "1" */

    new_fseek = rc->magic_fseek + (rc->external * sizeof(int));

   /* fseek on magic_fseek + (new external value * sizeof(int) */
    if (fseek(rc->fileptr, new_fseek, SEEK_SET) == -1) {
        fprintf(stderr,
                MP(mp523,"mirc - Error: Binary MIR File fseek error\n       fseek value %ld"),
                new_fseek);
        return(MC_FAILURE);
        }

    /* read LOCAL_MAS_SIZE integers to "local_mas" */
    if ( (status = fread(&(rc->local_mas[1]),
                         sizeof(int),
                         LOCAL_MAS_SIZE,
                         rc->fileptr)) < 0) {
        fprintf(stderr,
                MP(mp524,"mirc - Error: Binary MIR File fread error\n       errno  %d"),
                errno);
        return (MC_FAILURE);
        }

    rc->base_extrnl = rc->external;   /* set base_external to external */

    /* if (read returned something other than LOCAL_MAS_SIZE) */
    if (status != LOCAL_MAS_SIZE) {

        /* if (read returned ZERO) */
        if ( status == 0) {
            fprintf(stderr,
                    MP(mp525,"mirc - Error: Binary MIR File internal alignment #2 error\n       mo_size = %u  external = %d  mo_type code %d status = %d"),
                    rc->mo_size, rc->external, mo_type, status);
            return(MC_FAILURE);
            }
        }

    /* set local_max to value returned PLUS ONE */
    rc->local_max = status + 1;
    }

/*
|  If the END of the next object is STILL not within the "local_mas" buffer
|      THEN <issue error msg and return>
|
| if (local + mo_size is greater than or equal to "local_max")
|                 (The End of Obj)                            */
if ( (rc->local + (rc->mo_size -1)) >= rc->local_max) {
    fprintf(stderr,
            MP(mp526,"mirc - Error: Binary MIR File internal buffer too small\n       mo_size = %u  external = %d  mo_type code %d "),
            rc->mo_size, rc->external, mo_type);
    return(MC_FAILURE);
    }

#if 0
if (mo_type == I_NT_OBJECT_Rel) {
    printf("mirci_fetch_next returns:\n");
    printf("    rc->base_extrnl = %d\n", rc->base_extrnl);
    printf("    rc->external    = %d\n", rc->external);
    printf("    rc->local_max   = %d\n", rc->local_max);
    printf("    rc->local       = %d\n", rc->local);
    printf("    rc->mo_size     = %d\n\n", rc->mo_size);
    printf("   @rc->local       = %d\n\n", rc->local_mas[rc->local]);
    }
#endif

return(MC_SUCCESS);
}

/* Comparison Function - Used in "bsearch()" call in "mirci_addr_xlate" */
/* Comparison Function - Used in "bsearch()" call in "mirci_addr_xlate" */
/* Comparison Function - Used in "bsearch()" call in "mirci_addr_xlate" */

int
xref_compare( a, b)

const void *a;        /* First argument  */
const void *b;        /* Second argument */
{
return ((*(int*)a) - (*(int *)b) );
}


/* mirci_addr_xlate - Translate External Addresses to Internal */
/* mirci_addr_xlate - Translate External Addresses to Internal */
/* mirci_addr_xlate - Translate External Addresses to Internal */

static MC_STATUS
mirci_addr_xlate(mas, ic, syn_to_IDS)

masa          *mas;      /*--> to mini-MIR Address Space (Partition Table)  */
inter_context *ic;       /* --> Intermediate Context Block                  */
IDS         **syn_to_IDS;/* Synonym-to-IDS *xlate array                     */

/*
INPUTS:

    "mas" is the address of the "mini"-MAS containing the Partition-Table.
    Macros referenced by "mirci_fetch_next()" reference the Partition-Table
    via this name.

    "ic" points to the "Intermediate Context Block" where context (list heads)
    is maintained for functions in module "mir_intermediate.c" that create
    and manage Intermediate Data Structures (IDS).  This funtion reads
    some of these lists.

    "syn_to_IDS" - points to an array of pointers (to IDS structures for each
    MIR Relationship object) arranged by synonym.


OUTPUTS:

    On success, the function returns MC_SUCCESS:

       * All internal heap IDS structures that contain pointers have been
         converted from external addresses into internal heap addresses.

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been opened and all the MIR Objects in it have been
        "read-in" to the heap.  Some MIR Object flavors have pointers in them.
        During the read-in process these pointers remain unchanged.

    Purpose:
        This function passes through all the MIR objects containing unchanged
        pointers and converts them to point to the proper in-heap MIR object
        using cross-reference data-structures built during the "read-in"
        process.


ACTION SYNOPSIS OR PSEUDOCODE:

    (* Slices *)
    for (every slice on the slice list)
        if (backpointer is not zero)
            <determine backpointer object: I_SLICE or I_SUBREGISTER>
            <lookup & translate (appropriately) backpointer in-heap address>
        for (every slice entry in the slice)
            <discover kind of address the slice entry "next" points to>
            <lookup & translate slice entry "next" in-heap address>


    (* Subregisters *)
    for (every subregister on the subregister list)
        <lookup & translate (as an I_SLICE) backptr in-heap address>
        <lookup & translate (as an I_SLICE) "lower_levels" in-heap address>
        <discover what kind of NON-TERMINAL "ntobject" points at>
        <lookup & translate SR "ntobject" (as Non-Terminal) in-heap address>


    (* Data-Construct Non-Terminals *)
    <select DC-Non-Terminal list and associated xref list(s)>
    <perform non-terminal xref>
    if (an error occurred)
        <return MC_FAILURE>


    (* General Non-Terminals *)
    <select General Non-Terminal list and associated xref list(s)>
    <perform non-terminal xref>
    if (an error occurred)
        <return MC_FAILURE>


    (* MIR Relationship Non-Terminals *)
    <select MIR Relationship Non-Terminal list and associated xref list(s)>
    <perform non-terminal xref>
    if (an error occurred)
        <return MC_FAILURE>

    <return MC_SUCCESS>

OTHER THINGS TO KNOW:

    Color Television was first demonstrated in 1929.
*/      

/*
|  XLATE - Translate External Address to Internal (in-heap) address
|
|  This macro returns the "ext_int_xref" element that contains the in-heap IDS
|  address of a MIR Object of type "mo_type" whose external address is stored
|  at (keyptr).
*/
#define XLATE(keyptr, mo_type, x)                                             \
if (  (x = (ext_int_xref *) bsearch((keyptr),          /* Ptr to Ext. Addr  */\
                                    ic->xref[mo_type], /* Ptr xref array    */\
                                    ic->xref_count[mo_type], /* Size of xref*/\
                                    sizeof(ext_int_xref), /*Size of elements*/\
                                    xref_compare       /* Compare function  */\
                                    )) == NULL) {                             \
    fprintf(stderr,                                                           \
            MP(mp527,"mirc - Error: Binary xref lookup failed\n       external address = %d, MIR object type code %d\n"), \
            *(keyptr), mo_type);                                              \
    return(MC_FAILURE);                                                       \
    }

{
IDS             *inheap;        /* Pointer to thing we're working on now   */
ext_int_xref    *xr;            /* Pointer into Cross Reference array      */
int             i;              /* Handy-Dandy index and integer           */
IDS_TYPE        ids_type;       /* Type of something at a selected address */
masa            ext_addr;       /* An external address (to be converted)   */


/* Slices
|
| for (every slice on the slice list)
*/
for (inheap = ic->flavors[I_SLICE];
     inheap != NULL;
     inheap = inheap->next) {

    /* if (backpointer is not zero) */
    if (inheap->idsu.i.backptr != 0) {
        /* determine backpointer object: I_SLICE or I_SUBREGISTER */
        if (IS_A_SLICE(( int )inheap->idsu.i.backptr))
            ids_type = I_SLICE;
        else
            ids_type = I_SUBREGISTER;

        /* lookup & translate (as an I_SLICE) backpointer in-heap address */
#if 0
        printf("\nBacktranslating backpointer of %d\n",
               (int)inheap->idsu.i.backptr);
#endif
        XLATE(&(inheap->idsu.i.backptr), ids_type, xr);
        inheap->idsu.i.backptr = xr->int_add;
        }

    /* for (every slice entry in the slice) */
    for (i = 0; i < inheap->idsu.i.entry_count; i++) {
        
        /* discover kind of address the slice entry "next" points to */
        ext_addr = (masa) inheap->idsu.i.slice[i].next;
        if (IS_A_SLICE(ext_addr))
            ids_type = I_SLICE;
        else if (IS_A_SUBREG(ext_addr))
            ids_type = I_SUBREGISTER;
        else if (IS_A_NONTERM_DC(ext_addr))
            ids_type = I_NT_OBJECT_DC;
        else if (IS_A_NONTERM_REL(ext_addr))
            ids_type = I_NT_OBJECT_Rel;
        else ids_type = I_NT_OBJECT;

        /* lookup & translate slice entry "next" in-heap address */
#if 0
        printf("Backtranslating %dth 'next' of %d\n", i, ext_addr);
#endif
        XLATE(&ext_addr, ids_type, xr);
        inheap->idsu.i.slice[i].next = xr->int_add;
        }
    }

/* Subregisters
|
| for (every subregister on the list)
*/
for (inheap = ic->flavors[I_SUBREGISTER];
     inheap != NULL;
     inheap = inheap->next) {

    /* lookup & translate (as an I_SLICE) backptr in-heap address */
    XLATE(&(inheap->idsu.s.backptr), I_SLICE, xr);
    inheap->idsu.s.backptr = xr->int_add;

    /* lookup & translate (as an I_SLICE) "lower_levels" in-heap address */
    XLATE(&(inheap->idsu.s.lower_levels), I_SLICE, xr);
    inheap->idsu.s.lower_levels = xr->int_add;

    /* discover what kind of NON-TERMINAL "ntobject" points at */
    ext_addr = (masa) inheap->idsu.s.ntobject;
    if (IS_A_NONTERM_DC(ext_addr))
        ids_type = I_NT_OBJECT_DC;
    else if (IS_A_NONTERM_REL(ext_addr))
        ids_type = I_NT_OBJECT_Rel;
    else ids_type = I_NT_OBJECT;

    /* lookup & translate SR "ntobject" (as Non-Terminal) in-heap address */
    XLATE(&(ext_addr), ids_type, xr);
    inheap->idsu.s.ntobject = xr->int_add;
    }

/*
| Data-Construct Non-Terminals 
*/
if (mirci_NT_addr_xlate(I_NT_OBJECT_DC, mas, ic, syn_to_IDS) != MC_SUCCESS) {
    return(MC_FAILURE);
    }

/*
| General Non-Terminals 
*/
if (mirci_NT_addr_xlate(I_NT_OBJECT, mas, ic, syn_to_IDS) != MC_SUCCESS) {
    return(MC_FAILURE);
    }

/*
| MIR Relationship Non-Terminals 
*/
if (mirci_NT_addr_xlate(I_NT_OBJECT_Rel, mas, ic, syn_to_IDS) != MC_SUCCESS) {
    return(MC_FAILURE);
    }

return(MC_SUCCESS);
}

/* mirci_NT_addr_xlate - Translate NT External Addresses to Internal */
/* mirci_NT_addr_xlate - Translate NT External Addresses to Internal */
/* mirci_NT_addr_xlate - Translate NT External Addresses to Internal */

static MC_STATUS
mirci_NT_addr_xlate(NT_type, mas, ic, syn_to_IDS)

IDS_TYPE      NT_type;   /* Type of Non-Terminal: DC, General, MIR-Rel       */
masa          *mas;      /*--> to mini-MIR Address Space (Partition Table)   */
inter_context *ic;       /* --> Intermediate Context Block                   */
IDS           *syn_to_IDS[]; /* Synonym-to-IDS xlate array                     */


/*
INPUTS:

    "NT_type" - indicates which one of the "sub-species" of Non-Terminal
    that we're being called to process.

    "mas" is the address of the "mini"-MAS containing the Partition-Table.
    Macros referenced by "mirci_fetch_next()" reference the Partition-Table
    via this name.

    "ic" points to the "Intermediate Context Block" where context (list heads)
    is maintained for functions in module "mir_intermediate.c" that create
    and manage Intermediate Data Structures (IDS).  This funtion reads
    some of these lists to do it's processing.

    "syn_to_IDS" - points to an array of pointers (to IDS structures for each
    MIR Relationship object) arranged by synonym.


OUTPUTS:

    On success, the function returns MC_SUCCESS:

       * All the internal heap IDS structures for the specified type of
         Non-Terminal have been scanned and their pointers have been
         converted from external addresses into internal heap addresses.

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been opened and all the MIR Objects in it have been
        "read-in" to the heap.  Some MIR Object flavors have pointers in them.
        During the read-in process these pointers remain unchanged.

    Purpose:
        This function passes through a specified list of Non-Terminals
        (of one sub-type) containing pointers and converts them to point
        to the proper in-heap MIR object using cross-reference data-structures
        built during the "read-in" process.


ACTION SYNOPSIS OR PSEUDOCODE:

    for (each MIR Object remaining on the list)
        for (all currently active entries in the Relationship table)
            <lookup & translate via (synonym_to_IDS) "rel_obj" to in-heap>
            <discover kind of address the entry "tar_obj" points to>
            <lookup & translate "tar_obj" to in-heap>
            if ("tar_obj" was a Terminal or general Non-Terminal)
                <increment reference count>

        (* Fixup the OID Backpointer array entries *)
        for (each OID-SMI entry)

            <discover kind of address the oid-backpointer points to>
            if (it points to nothing)
                continue;

            if (I_SLICE)
                <lookup & translate (as an I_SLICE) the oid_backptr in-heap>
                if (count stored in oid_backarc is < 0 or > MAX_SMI)
                    <issue error message>
                    <return MC_FAILURE>
                <set count-down count to value stored in oid_backarc>
                <set slice-to-scan to slice (the specified backpointer)>
                <set thing-to-look-for to current inheap NT>
            else (I_SUBREGISTER)
                <lookup & translate (as an I_SUBREGISTER) the oid_backptr>
                if (count stored in oid_backarc is NOT 1)
                    <issue error message>
                    <return MC_FAILURE>
                <set count-down count to 1>
                <set slice-to-scan to subregister's backpointer>
                <set thing-to-look-for to subregister>

            <signal "arc-not-found">
            while ( count-down count)
                for (each slot in slice)
                    if (slot-entry pointer matches current object)
                        <decrement count>
                        if (count is zero)
                            <store slot-entry arc number into IDS array>
                            <signal "arc found">
                            <break>

            if ("arc-not-found")
                <issue error message>
                <MC_FAILURE>

    <return MC_SUCCES>


OTHER THINGS TO KNOW:

    What's going on here??

    Converting the Relationship Table "external addresses" to "internal"
    (i.e. to in-heap addresses of IDS structures) is fairly straightforward.

    However, the machinations on the OID Backpointers in the Non-Terminal
    may need a little explanation.  90% of this logic is to take care of
    the disambiguation needed when two or more OIDs (in different SMIs, of
    course) share exactly the same prefix except for the last, rightmost arc.

    When disambiguation is needed, what this means is that in each Non-terminal
    (in-heap) we have to store the ARC NUMBER from the index slice (that
    contains multiple pointers to this in-heap Non-Terminal) that specifies 
    the entry for a particular SMI for this Non-Terminal.

    Suppose a Non-Terminal has three OIDs (in need of disambiguation) assigned
    to it for SMIs "DNA", "SNMP" and "MCC".  This means that the three OID
    backpointers into the index from this Non-Terminal will have the SAME
    slice specified (the same value).  On entry into this routine, the
    corresponding "backarc" values for DNA, SNMP and MCC will specify which
    slots in the slice correspond to which SMI.  Assume the MCC OID rightmost
    arc turns out to be the highest arc number ("67") of the three arcs.
    Because the slice entries go in ascending order, the last slot in the slice
    with the same 'forward' pointer to this Non-Terminal will be the one for
    MCC (ie, it'll be the third entry in the slice WITH THE SAME forward
    pointer to this Non-Terminal).  Consequently, the "mirci_MAS_load()" code
    will put a "3" in the backarc for SMI=MCC.  The disambiguation logic
    in this function scans the slice for the "third" matching entry (variable
    "count_down" will be set to 3 initially) and the arc number 67 that appears
    in the slice will be loaded on top of the "3" in the "backarc" entry for
    the non-terminal, thereby converting the ordinal position (3) into it's
    true ARC number (67).

*/      

{
IDS             *inheap;        /* Pointer to thing we're working on now   */
ext_int_xref    *xr;            /* Pointer into Cross Reference array      */
int             i;              /* Handy-Dandy index and integer           */
int             j;              /* Handy-Dandy index and integer           */
IDS_TYPE        ids_type;       /* Type of something at a selected address */
masa            ext_addr;       /* An external address (to be converted)   */
int             count_down;     /* Counts down occurrence of a fwd ptr from*/  
IDS             *slice_to_scan; /* ..this slice while re-orging OID backptrs*/
IDS             *thing_to_look_for;  /* During slice-scan, we want this    */
BOOL            arc_found;      /* TRUE: Correct arc was found in slice    */


/* for (each MIR Object remaining on the list) */
for (inheap = ic->flavors[NT_type];
     inheap != NULL;
     inheap = inheap->next) {
     
    /* for (all currently active entries in the Relationship table) */
    for (i = 0; i < inheap->idsu.nt.entry_count; i++) {

        /* lookup & translate via (synonym_to_IDS) "rel_obj" to in-heap */
        inheap->idsu.nt.rel_table[i].rel_obj = 
            syn_to_IDS[( int ) inheap->idsu.nt.rel_table[i].rel_obj];

        /* discover kind of address the entry "tar_obj" points to */
        ext_addr = (masa) inheap->idsu.nt.rel_table[i].tar_obj;
        if (IS_A_SNUMBER(ext_addr))
            ids_type = I_T_OBJ_snumber;
        else if (IS_A_UNUMBER(ext_addr))
            ids_type = I_T_OBJ_unumber;
        else if (IS_A_STRING(ext_addr))
            ids_type = I_T_OBJ_string;
        else if (IS_A_NONTERM_DC(ext_addr))
            ids_type = I_NT_OBJECT_DC;
        else if (IS_A_NONTERM_REL(ext_addr))
            ids_type = I_NT_OBJECT_Rel;
        else ids_type = I_NT_OBJECT;

        /* lookup & translate "tar_obj" to in-heap */
        XLATE(&(ext_addr), ids_type, xr);
        inheap->idsu.nt.rel_table[i].tar_obj = xr->int_add;

        /* if ("tar_obj" was a general Non-Terminal) */
        if (xr->int_add->flavor == I_NT_OBJECT) {
            xr->int_add->idsu.nt.ref_count += 1;
            }
        else if (xr->int_add->flavor >= I_T_OBJ_snumber  /* or a Terminal */
            && xr->int_add->flavor <= I_T_OBJ_string) {

            /* increment Terminal's reference count */
            xr->int_add->idsu.t.ref_count += 1;
            }
        }

    /*
    | Fixup the OID Backpointer array entries
    |  for (each OID-SMI entry)
    */
    for (i=0; i < MAX_OID_SMI; i++) {

        /*
        |  Discover kind of address the oid-backpointer points to (it is
        |  currently unconverted).
        */
        ext_addr = ( int )inheap->idsu.nt.oid_backptr[i];
        if (ext_addr == 0) continue;    /* If Nothing to adjust, skip it */

        if (IS_A_SLICE(ext_addr)) {

            /* lookup & translate (as an I_SLICE) the oid_backptr in-heap */
            XLATE(&(ext_addr), I_SLICE, xr);
            inheap->idsu.nt.oid_backptr[i] = xr->int_add;
            
            /* if (count stored in oid_backarc is < 0 or > MAX_SMI) */
            if (   inheap->idsu.nt.oid_backarc[i] < 0
                || inheap->idsu.nt.oid_backarc[i] > MAX_OID_SMI) {
                fprintf(stderr,
                        MP(mp528,"mirc - Error: Internal Error - 'backarc' invalid\n       arc = %d, smi code = %d"),
                        inheap->idsu.nt.oid_backarc[i], i);
                return(MC_FAILURE);
                }

            /* set count-down count to value stored in oid_backarc */
            count_down = inheap->idsu.nt.oid_backarc[i];

            /* set slice-to-scan to slice (the specified backpointer) */
            slice_to_scan = inheap->idsu.nt.oid_backptr[i];

            /* set thing-to-look-for to current inheap NT */
            thing_to_look_for = inheap;
            }

        else { /* it must be a I_SUBREGISTER */

            /* lookup & translate (as an I_SUBREGISTER) the oid_backptr */
            XLATE(&(ext_addr), I_SUBREGISTER, xr);
            inheap->idsu.nt.oid_backptr[i] = xr->int_add;

            /* if (count stored in oid_backarc is NOT 1) */
            if (inheap->idsu.nt.oid_backarc[i] != 1) {
                fprintf(stderr,
                        MP(mp529,"mirc - Error: Internal Error - Subregister 'backarc' invalid\n       arc = %d, smi code = %d"),
                        inheap->idsu.nt.oid_backarc[i], i);
                return(MC_FAILURE);
                }

            count_down = 1;     /* set count-down count to 1 */

            /* set slice-to-scan to subregister's backpointer */
            slice_to_scan = xr->int_add->idsu.s.backptr;

            /* set thing-to-look-for to subregister */
            thing_to_look_for = xr->int_add;
            }

        arc_found = FALSE;      /* signal "arc-not-found" */
        while ( count_down > 0) {

            /* for (each slot in slice) */
            for (j=0; j < slice_to_scan->idsu.i.entry_count; j++) {

                /* if (slot-entry pointer matches current object) */
                if (slice_to_scan->idsu.i.slice[j].next == thing_to_look_for) {

                    count_down -= 1;    /* decrement count */

                    if (count_down <= 0) {
                        /* store slot-entry arc number into IDS array */
                        inheap->idsu.nt.oid_backarc[i] =
                            slice_to_scan->idsu.i.slice[j].iso_arc;

                        arc_found = TRUE;  /* signal "arc found" */
                        break;
                        }
                    }
                }   /* for each slot */
            }   /* while count still non-zero */

        /* if ("arc-not-found") */
        if (arc_found == FALSE) {
            fprintf(stderr,
                    MP(mp530,"mirc - Error: Internal Error - Slice fwd ptr missing\n       Obj @ External Addr %d, SMI code %d "),
                    ext_addr, i);
            return(MC_FAILURE);
            }

        } /* For each SMI's OID */
    } /* For each flavor of NT */

return(MC_SUCCESS);
}

/* mirci_compiler_lists - Constructs Internal Lists for Compiler Front-end */
/* mirci_compiler_lists - Constructs Internal Lists for Compiler Front-end */
/* mirci_compiler_lists - Constructs Internal Lists for Compiler Front-end */

static MC_STATUS
mirci_compiler_lists(bec)

back_end_context  *bec;       /* Pointer to back-end context block    */


/*
INPUTS:

    "bec" is the address of the "back-end" context block.  In this structure
    is stored the list-heads of the internal lists this function must
    construct, as well as the other lists from which they are derived.


OUTPUTS:

    On success, the function returns MC_SUCCESS and the following lists in the
    "back-end context" are setup:

       * The lists named "map_rel_to_ids[]" and "map_relstr_to_ids[]" are
         initialized to point to the proper IDS structures for all the MIR
         Relationship Objects.  

       * The list "map_smicode_to_smistr[]" is set to indicate the string 
         names of the SMI's the compiler can recognize (when indexed by the
         SMI numeric code) as they were found in the input binary file.

       * The list "built_in[]" is set up with a list of "IDX" structures for
         each SMI the compiler discovered in the input binary file.

    On failure, MC_FAILURE is returned and a message has been printed on
    stderr informing the user of the problem.  The compiler should exit.


BIRD'S EYE VIEW:
    Context:
        The front-end of the compiler is just beginning operations and
        has discovered from the command-line arguments that the user has
        specified a binary MIR database file as part of the compilation.
        The file has been opened and all the MIR Objects in it have been
        "read-in" to the heap.  All pointers within the loaded MIR objects
        have been translated to their proper "in-heap" representation.
        In order to perform a compilation, the compiler needs to build
        some temporary lists.

    Purpose:
        This function passes through all necessary in-heap IDS structures
        (for MIR objects) and builds the necessary compile-time lists in
        the "back-end context".


ACTION SYNOPSIS OR PSEUDOCODE:

    (* Build the map_rel*[] lists *)
    <set index to 0 (corresponds to MIR Relationship "MIR_Relationship_name")>
    for (each MIR Object in the MIR Relationship Partition)
        <store pointer to MIR object in index position in "map_rel_to_ids[]">
        <store internal synonym into the IDU>
        <fetch name of IDS and store it in index position of
         "map_relstr_to_ids[]">

    (* Read "The World":
    |   - Build the "map_smicode_to_smistr[]" list
    |   - Read and increment the "Rebuild Count"
    |   - Construct the internal list of valid Keywords
    *)
    <obtain the IDS of the first object in the General Non-Terminal partition:
     The World>
    <set Relationship Table scanning index to 0>
    while (scanning index does not match "MIR_Special"
           and is less than the table size)
        <increment scanning index>
    <extract pointer to Built-in Type Version string and store in bec>
    <set local SMI code to MIR_SMI_UNKNOWN+1>
    <increment scanning index>
    while (scanning index matches "MIR_Special" and target is of type STRING
           and is less than the table size)
        <obtain the target of MIR_Special as a string>
        <store string in map_smicode_to_smistr[local SMI Code]>
        <increment scanning index>
        <increment local SMI code>

    (* Build the "built_in[]" list *)
    <set current SMI code to 1>
    <set scanner pointer to address of "built_in[current-code]">
    for (each MIR object in the Data-Construct Partition)
        <obtain by convention the code for the SMI in which the object is
         defined>
        if (the code does not match the current SMI code)
            if (the code is < MAX_SMI)
                <change the current SMI code to just-obtained code>
                <set scanner ptr to addr of "built_in[current-code]">
            else
                <issue "Invalid SMI Code read from MIR database file>
                <return MC_FAILURE>

        if (attempt to allocate IDX failed (into current scanner))
            <issue error msg>
            <return MC_FAILURE>

        <set IDX "next" cell to NULL>
        <set current scanner to address of IDX "next" cell>
        <obtain by convention the name for the SMI in which the object is
         defined>
        <set IDX "name" cell to string for name>
        <set IDX "len" cell to length of string for name>
        <set IDX "nt" cell to IDS for current dataconstruct>
    <return MC_SUCCESS>

OTHER THINGS TO KNOW:

    In V2.0 of the compiler, "map_relstr_to_ids[]" is only used by function
    "mirc_find_rel_IDS()" which is only called by the parser code in
    "mirc_yacc.y" when it parses the builtin-type file.  The builtin-type file
    is *not* parsed fully enough to cause this function to be called when 
    the compiler is starting from a binary MIR database file.  But this code
    loads the array anyway for completeness sake, so as to leave no holes to
    fall into later.

    Also note that this function takes advantage of the conventions established
    for what is contained in "The World" MIR Object: See "mirc_init_backend()"
    in "mir_backend.c" for the details.
*/      

{
int           index;    /* Index for building map_rel_to_ids[] array        */
IDS           *inheap;  /* Points to succession of IDS's we're dealing with */
mir_smi_code  smi;      /* Used for loading names of SMI into list of them  */
IDX          **scanner; /* Used to build up a list of IDX structures        */
IDX           *idx;     /* Points to the current IDX we're working on       */
mir_smi_code  ext_code; /* Code extracted from a Data-Construct NT for SMI  */
char         *ext_name; /* Name extracted from a Data-Construct NT          */
int           ext_len;  /* Length of name extracted.                        */
int           rebuild;  /* Rebuild count extracted from "The World"         */
IDS          *new_rbc;  /* IDS for the new value of the Rebuild-Count       */


/*==========================
| Build the map_rel*[] lists
*/

/*
| set index to 0 (corresponds to MIR Relationship "MIR_Relationship_name")
| for (each MIR Object in the MIR Relationship Partition)
*/
for (index = 0, inheap = bec->i_context.flavors[I_NT_OBJECT_Rel];
     inheap != NULL && index < REL_COUNT;
     inheap = inheap->next, index += 1) {

    /* store pointer to MIR object in index position in "map_rel_to_ids[]" */
    bec->map_rel_to_ids[index] = inheap;

    /*
    |  Load the "Internal" (zero-origined) Synonym into the IDS for use by
    |  "mir_remove.c" et.al.
    */
    inheap->idsu.nt.ntu.synonym = index;

    /*
    | fetch name of IDS and store it in index position of "map_relstr_to_ids[]"
    */
    if ((bec->map_relstr_to_ids[index] = mirc_find_obj_NAME(inheap)) == NULL) {
        fprintf(stderr,
                MP(mp531,"mirc - Error: MIR Relationship %d has no name\n"),
                index);
        return(MC_FAILURE);
        }
    }
if (inheap != NULL) {
    fprintf(stderr,
            MP(mp532,"mirc - Error: Internal Limit %d exceeded for MIR Relationships\n"),
            index);
    return(MC_FAILURE);
    }


/*========================================
| Read "The World":
|   - Build the "map_smicode_to_smistr[]" list
|   - Read and increment the "Rebuild Count"
|   - Construct the internal list of valid Keywords
|
| Build the "map_smicode_to_smistr[]" list
*/

/*
| obtain the IDS of the first object in the General Non-Terminal partition:
| "The World"
*/
inheap = bec->i_context.flavors[I_NT_OBJECT];

index = 0;      /* set Relationship Table scanning index to 0 */

/*
| (This "walks" us down The World to the first "MIR_Special" entry).
|
| while (scanning index does not match "MIR_Special"
|       and is less than the table size)
*/
while ((index < inheap->idsu.nt.entry_count)
       &&
       (inheap->idsu.nt.rel_table[index].rel_obj
         != bec->map_rel_to_ids[MIR_Special]) )
    {
    index += 1;         /* increment scanning index */
    }
/* We pretty much presume there is one, disaster may result if not! */

/* extract pointer to Built-in Type Version string and store in bec */
bec->binary_bidt_version =
    inheap->idsu.nt.rel_table[index].tar_obj->idsu.t.t_v.string.str;

/*
| (We want the first code *after* "unknown")
|
| set local SMI code to MIR_SMI_UNKNOWN+1
*/
smi = MIR_SMI_UNKNOWN + 1;

index += 1;     /* increment scanning index */

/*
| (This "walks" us down The World thru *EACH* "MIR_Special" entry until
|  we either run out of them (an error we don't report) or we hit one whose
|  type is not "String" (it must be "Signed Number": the Rebuild Count).
|
| while (scanning index *does*  match "MIR_Special" and Target is a String
|       and is less than the table size)
*/
while ( (index < inheap->idsu.nt.entry_count)
             &&
        (inheap->idsu.nt.rel_table[index].tar_obj->flavor == I_T_OBJ_string)
             &&
      (inheap->idsu.nt.rel_table[index].rel_obj
       == bec->map_rel_to_ids[MIR_Special]))
    {
    /*
    |  obtain the target of MIR_Special as a string and store string in
    |  map_smicode_to_smistr[local SMI Code].
    */
    bec->map_smicode_to_smistr[smi] =
        inheap->idsu.nt.rel_table[index].tar_obj->idsu.t.t_v.string.str;

    index += 1; /* increment scanning index */
    smi += 1;   /* increment local SMI code */
    }

/*
| We presume we stopped on the Rebuild Count
|
| Reach in and grab the actual rebuild count value
*/
rebuild = inheap->idsu.nt.rel_table[index].tar_obj->idsu.t.t_v.snumber.value;

/* Blow off the IDS for the old number */
mirc_remove_OBJECT(inheap->idsu.nt.rel_table[index].tar_obj, bec);

/* Create a new IDS for Rebuild-Count + 1 */
if ((new_rbc = I_Create_IDS_SNUMBER((rebuild+1))) == NULL) {
    fprintf(stderr,
            MP(mp533,"mirc - Error: Create for Rebuild Count failed\n"));
    }

new_rbc->idsu.t.ref_count += 1; /* Bump the reference count */

/* Substitute the new IDS for the new Rebuild Count in for the old r-b IDS */
inheap->idsu.nt.rel_table[index].tar_obj = new_rbc;

/* Show us "on to the next entry". . .*/
index += 1;

/*
| Now we have to create the compiler's internal list of valid keywords as
| found opposite the remaining "MIR_Specal" relationships.
*/
for (scanner = &bec->legal_keyword_list;        /* Build list here        */

     index < inheap->idsu.nt.entry_count        /* As long as MIR_Special */
     && (inheap->idsu.nt.rel_table[index].rel_obj /* entries exist        */
         == bec->map_rel_to_ids[MIR_Special]);

     scanner = &(*scanner)->next, index += 1) {

    /* Try to create the IDX for the next keyword */
    if ( ((*scanner) = (IDX *) malloc(sizeof(IDX))) == NULL) {
         fprintf(stderr,
            MP(mp534,"mirc - Error: Create for Keyword IDX failed\n"));
         }

    /* Load the IDX w/pointer to string for the Keyword value
    |
    |  NOTE: We can do this without copying the actual string value because
    |        The World can never be deleted, so these target IDS's will always
    |        remain valid.
    */
    (*scanner)->name =
        inheap->idsu.nt.rel_table[index].tar_obj->idsu.t.t_v.string.str;
    (*scanner)->next = NULL;
    }


/*===========================
| Build the "built_in[]" list
*/

/* set current SMI code to 1 (the first code we should see for an SMI) */
smi = MIR_SMI_UNKNOWN + 1;

/* set scanner pointer to address of "built_in[current-code]" */
scanner = &bec->built_in[smi];

/* for (each MIR object in the Data-Construct Partition) */
for (inheap = bec->i_context.flavors[I_NT_OBJECT_DC];
     inheap != NULL;
     inheap = inheap->next) {

    /*
    | Obtain by convention the code for the SMI in which the object is
    | defined.
    |
    | (The convention is this is FIRST in the relationship table. See
    |  code in "mir_backend.c" function "mirc_create_dataconstruct()").
    */
    ext_code = inheap->idsu.nt.rel_table[0].tar_obj->idsu.t.t_v.snumber.value;

    /* if (the code does not match the current SMI code) */
    if (ext_code != smi) {

        /* If it is a valid new code . . .*/
        if (ext_code < MAX_SMI) {
            /* change the current SMI code to just-obtained code */
            smi = ext_code;

            /* set scanner ptr to addr of "built_in[current-code]" */
            scanner = &bec->built_in[smi];
            }
        else {
            /* issue "Invalid SMI Code read from MIR database file" */
            fprintf(stderr,
            MP(mp535,"mirc - Error: Invalid SMI Code %d read from MIR database file \n"),
                    ext_code);
            return(MC_FAILURE);
            }
        }

    /* if (attempt to allocate IDX failed (into current scanner)) */
    if ( (*scanner = (IDX *) malloc(sizeof(IDX))) == NULL) {
        fprintf(stderr,
            MP(mp536,"mirc - Error: Out of Memory for IDX structure\n"));
        return(MC_FAILURE);
        }

    idx = *scanner;

    /* set IDX "next" cell to NULL */
    (*scanner)->next = NULL;

    /* set current scanner to address of IDX "next" cell */
    scanner = &((*scanner)->next);

    /*
    | Obtain by convention the name for the SMI in which the object is
    | defined.
    |
    | (The convention is this is THIRD in the relationship table. See
    |  code in "mir_backend.c" function "mirc_create_dataconstruct()").
    */
    ext_name = inheap->idsu.nt.rel_table[2].tar_obj->idsu.t.t_v.string.str;
    ext_len  = inheap->idsu.nt.rel_table[2].tar_obj->idsu.t.t_v.string.len;

    /* set IDX "name" cell to string for name */
    idx->name = ext_name;
    
    /* set IDX "len" cell to length of string for name */
    idx->len = ext_len;

    /* set IDX "nt" cell to IDS for current dataconstruct */
    idx->nt = inheap;
    }

return(MC_SUCCESS);

}
