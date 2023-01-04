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
static char *rcsid = "@(#)$RCSfile: mir_symdump.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:16:38 $";
#endif
/*
===============================================================================
===============================================================================
========================== WATCH OUT EDITING THIS FILE ========================
===============================================================================
===============================================================================
There are large chunks of duplicate code for the two different Binary File
Formats that this module can interpret.  If you search for a string,
(to change something) make sure you search AGAIN... you may have to make two
changes for the two different dump versions!
===============================================================================
*/

/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991.
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
 * Module MIR_SYMDUMP.C
 *      Contains functions that will perform a symbolic dump of a
 *      MIR binary database file.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   February 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines have need of network management
 *       architecture information in order to properly process incoming
 *       management request messages.  This information must reside in a
 *       Management Information Repository, accessible by ISO Object ID.
 *       The information is stored (for loading into the MIR) in a binary
 *       form.
 *
 *    Purpose:
 *       This module contains the functions that interpret the binary form
 *       of the MIR database file and present an ASCII dump representation
 *       of it.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *   Version    Date            Who             What
 *      V1.0    February 1991   D. D. Burns     Original Version
 *
 *      V1.1    March 1991      D. D. Burns     Add comments and force
 *                                               partition boundaries to new
 *                                               page.
 *
 *      V1.95   July 1992       D. D. Burns     Add support for Binary Output
 *                                               File Format 2, while
 *                                               maintaining support for
 *                                               Format 1
 *                                       
 *      V1.98   Oct 1992        D. D. Burns     Internationalize for Binary
 *                                               Output File Format 2
 *                                       
 *      V2.00   Jan 1993        D. D. Burns     Alpha-port changes per
 *                                               Adam Peller
Module Overview:

This module contains the functions required to give a human-readable
dump of a binary MIR database file of the sort produced by the MIR compiler.


MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name           Synopsis
mirc_symdump            Dumps a Symbolic ASCII Dump of a MIR database file to
                        a specified file.

MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis
mirci_index_dump        Create a Symbolic Dump of the Index
get_oid_index           Employed here to recursively walk the index
                        to discover the full OID for an object in the index.
                 NOTE: The source for this function is #include-d into this
                       module from file "mir_goi.cinc".  This allows us to
                       distribute ONE object module that corresponds to
                       the Tier 0 functions (rather than two).  This function
                       source is shared with "mir_symdump.c" which also
                       includes it.

*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef VMS
#include <malloc.h>
#endif

/* Request definitions for compiler modules from "mir.h" */
#define MIR_T0
#define MIR_COMPILER
#include "mir.h"


/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
# include <nl_types.h>
extern nl_catd _m_catd_mir;
extern void mir_tier_init();
extern int mir_tier_init_status;
# ifdef MTHREADS
static pthread_once_t mir_tier_init_block = pthread_once_init;
#  define CATOPEN_ONCE if (!(mir_tier_init_status)) pthread_once (&mir_tier_init_block, mir_tier_init)
# else
#  define CATOPEN_ONCE if (!(mir_tier_init_status)) mir_tier_init()
# endif
extern char *mp300();
extern char *mp301();
extern char *mp302();
extern char *mp303();
extern char *mp304();
extern char *mp305();
extern char *mp306();
extern char *mp307();
extern char *mp308();
extern char *mp309();
extern char *mp310();
extern char *mp311();
extern char *mp312();
extern char *mp313();
extern char *mp314();
extern char *mp315();
extern char *mp316();
extern char *mp317();
extern char *mp318();
extern char *mp319();
extern char *mp320();
extern char *mp321();
extern char *mp322();
extern char *mp323();
extern char *mp324();
extern char *mp325();
extern char *mp326();
extern char *mp327();
extern char *mp328();
extern char *mp329();
extern char *mp330();
extern char *mp331();
extern char *mp332();
extern char *mp333();
extern char *mp334();
extern char *mp335();
extern char *mp336();
extern char *mp337();
extern char *mp338();
extern char *mp339();
extern char *mp340();
extern char *mp341();
extern char *mp342();
extern char *mp343();
extern char *mp344();
extern char *mp345();
extern char *mp346();
extern char *mp347();
extern char *mp348();
extern char *mp349();
extern char *mp350();
extern char *mp351();
extern char *mp352();
extern char *mp353();
extern char *mp354();
extern char *mp355();
extern char *mp356();
extern char *mp357();
extern char *mp358();
extern char *mp359();
extern char *mp360();
extern char *mp361();
extern char *mp362();
extern char *mp363();
extern char *mp364();
extern char *mp365();
extern char *mp366();
extern char *mp367();
extern char *mp368();
extern char *mp369();
extern char *mp370();
extern char *mp371();
# else
#  define CATOPEN_ONCE
#endif


/*
|
|   Define Prototypes for all Functions internal to module
|
*/

/* mirci_index_dump_BV1 - Create Symbolic Dump of the Index/Binary Version 1 */
static void
mirci_index_dump_BV1 PROTOTYPE((
int     *,       /* The data to dump                                 */
int      ,       /* Cell address (index into array) of slice to dump */
int     *,       /* Arcs encountered so far as indexed by...         */
int      ,       /* Index to arcs in oidarc                          */
FILE    *        /* File pointer to do I/O on                        */
));

/* mirci_index_dump_BV2 - Create Symbolic Dump of the Index/Binary Version 2 */
static void
mirci_index_dump_BV2 PROTOTYPE((
int     *,       /* The data to dump                                 */
int      ,       /* Cell address (index into array) of slice to dump */
int     *,       /* Arcs encountered so far as indexed by...         */
int      ,       /* Index to arcs in oidarc                          */
FILE    *        /* File pointer to do I/O on                        */
));

/* do_BV1 - Dump to (screen/file) the V1 Linear External Representation */
static void
do_BV1 PROTOTYPE((
FILE    *,     /* --> Output file name string or NULL = stdout */
FILE    *,     /* --> Input file name to open                  */
BOOL     ,     /* TRUE: Give Raw Dump (+ symbolic) of index    */
char    *      /* --> Input filename                           */
));

/* do_BV2 - Dump to (screen/file) the V2 Linear External Representation */
static void
do_BV2 PROTOTYPE((
FILE    *,     /* --> Output file name string or NULL = stdout */
FILE    *,     /* --> Input file name to open                  */
BOOL     ,     /* TRUE: Give Raw Dump (+ symbolic) of index    */
char    *      /* --> Input filename                           */
));

/* dump_BV2_NT - Dump Binary Format Version 2 Non-Terminal MIR Object */
void dump_BV2_NT PROTOTYPE((
int     *,        /* --> Cell Address to start at                 */
int     *,        /* Array of MIR Address Space                   */
FILE    *,        /* --> Output File Pointer                      */
char    *[],      /* Array of MIR Relationship Names by "synonym" */
int      ,        /* Synonym for MIR_Text_Name                    */
int      ,        /* Synonym for MIR_DC_SMI_Name                  */
int               /* Maximum Number of OID Arcs                   */
));


/* mirci_index_dump_BV1 - Create Symbolic Dump of the Index/Binary Version 1 */
/* mirci_index_dump_BV1 - Create Symbolic Dump of the Index/Binary Version 1 */
/* mirci_index_dump_BV1 - Create Symbolic Dump of the Index/Binary Version 1 */

static void
mirci_index_dump_BV1( array, ca, oidarcs, level, ofptr)

/*      -------- This Function is RECURSIVE ---------                       */

int     *array;         /* The data to dump                                 */
int     ca;             /* Cell address (index into array) of slice to dump */
int     *oidarcs;       /* Arcs encountered so far as indexed by...         */
int     level;          /* Index to arcs in oidarc                          */
FILE    *ofptr;         /* File pointer to do I/O on                        */

/*
INPUTS:

    "array" is the array of data being dumped
    "ca" is the index to the slice being processed on this call
    "oidarc" is a list of all the arcs up to where we are
    "level" is the index that points to the last arc in "oidarc".

OUTPUTS:

    This function outputs a one line dump of an ISO-Object ID and the
    MIR index address of any Non-Terminal that has that Object ID for
    each Non-Terminal registered at "level" of the index.

    For each entry in the index which does not point to a Non-Terminal,
    (ie it points to a another level down in the index) this function calls
    itself to evaluate that next level down.

OTHER THINGS TO KNOW:

    This function also processes "Subregisters" (elements in the index that
    stand for an object id that both has a value and points to another level
    down in the index.

*/

/*
| These are Binary Output File Format 1 applicable macros.
| For support of Binary Output File Format 2, the appropriate macros
| are included from "mir.h".
*/

/* This macro allows us to determine whether or not a given cell address is */
/* that of a Non-Terminal or not                                            */
#define IS_A_BV1_NT(ca)  (array[5] <= ca)

/* This macro allows us to determine whether or not a given cell address is */
/* that of a Subregister or not                                     */
#define IS_A_BV1_SR(ca)  ((array[2] <= ca) && (array[3] > ca))

/* This macro allows us to determine whether or not a given cell address is */
/* that of a Slice  or not                                          */
#define IS_A_BV1_SLICE(ca)  ((array[1] <= ca) && (array[2] > ca))
{
static int      oid_storage[100];       /* Store arcs here               */
int             i,j;                    /* General indices               */
int             slice_limit;            /* End of range of slice entries */


/* If we're being called the first time, there'll be no oidarc array, so */
/* we set up our pointer to static local storage.                        */
if (oidarcs == NULL)
  oidarcs = oid_storage;

/* "ca" is an index into "array" to the start of an index "slice".  We want */
/* to scan down the arcs in this slice and either print the address of any  */
/* non-terminal encountered or recur to a lower level to do so if we point  */
/* at a slice.                                                              */
ca += 1;                        /* Skip the backpointer         */
slice_limit = array[ca++];      /* Grab Entry Count             */

/* For each entry in this slice... */
for (i = 0; i < slice_limit; i++) {

  /* If this entry points to a Non-Terminal, print the Object ID */
  if IS_A_BV1_NT((array[(ca+1)])) {

    /* Insert this arc number into the array for printing */
    oidarcs[level] = array[ca];

    /* Now dump out all the arcs as the Object ID of this object */
    for (j=0; j <= level; j++) {
      if (j != 0) fprintf (ofptr,".");
      fprintf(ofptr, "%d", oidarcs[j]);
      }

    /* Now print the address of the non-terminal */
    fprintf(ofptr, " -- [%d]\n",array[ca+1]);
    ca += 2;
    continue;
    }

  /* If this entry points to a Slice, simply recur on it to print it out */
  if IS_A_BV1_SLICE((array[(ca+1)])) {

    /* Put the current arc number into oidarc in prep for the call */
    oidarcs[level] = array[ca];
    mirci_index_dump_BV1(array, array[ca+1], oidarcs, (level + 1), ofptr);
    ca += 2;
    continue;
    }

  /* If this entry points to a Subregister, print the OID for the object */
  /* and then recur to print the lower levels.                           */
  if IS_A_BV1_SR((array[(ca+1)])) {
    
    /* Insert this arc number into the array for printing */
    oidarcs[level] = array[ca];

    /* Now dump out all the arcs as the Object ID of this object */
    for (j=0; j <= level; j++) {
      if (j != 0) fprintf (ofptr,".");
      fprintf(ofptr, "%d", oidarcs[j]);
      }

    /* Now print the address of the non-terminal from subregister */
    fprintf(ofptr, " -- [%d]\n",array[array[ca+1]+1]);

    /* Now recur on the lower-level pointer out of the subregister */
    mirci_index_dump_BV1(array, array[array[ca+1]+2], oidarcs, (level + 1), ofptr);
    ca += 2;
    continue;
    }

  /* If we arrive here, the index points at something that it shouldn't */
  fprintf(ofptr, "MIR Dump: Invalid reference from index cell %d\n",i);
  ca += 2;
  }
}

/* mirci_index_dump_BV2 - Create Symbolic Dump of the Index/Binary Version 2 */
/* mirci_index_dump_BV2 - Create Symbolic Dump of the Index/Binary Version 2 */
/* mirci_index_dump_BV2 - Create Symbolic Dump of the Index/Binary Version 2 */

static void
mirci_index_dump_BV2( mas, ca, oidarcs, level, ofptr)

/*      -------- This Function is RECURSIVE ---------                       */

int     *mas;           /* The data to dump                                 */
int     ca;             /* Cell address (index into array) of slice to dump */
int     *oidarcs;       /* Arcs encountered so far as indexed by...         */
int     level;          /* Index to arcs in oidarc                          */
FILE    *ofptr;         /* File pointer to do I/O on                        */

/*
INPUTS:

    "mas" is the array of data being dumped
    "ca" is the index to the slice being processed on this call
    "oidarc" is a list of all the arcs up to where we are
    "level" is the index that points to the last arc in "oidarc".
    "ofptr" indicates the file to print into.

OUTPUTS:

    This function outputs a one line dump of an ISO-Object ID and the
    MIR index address of any Non-Terminal that has that Object ID for
    each Non-Terminal registered at "level" of the index.

    For each entry in the index which does not point to a Non-Terminal,
    (ie it points to a another level down in the index) this function calls
    itself to evaluate that next level down.

OTHER THINGS TO KNOW:

    This function also processes "Subregisters" (elements in the index that
    stand for an object id that both has a value and points to another level
    down in the index.
*/

/*
| For support of Binary Output File Format 2, the appropriate macros
| are included from "mir.h".  The MAS must be held in an array called "mas[]"
| in order for these macros to perform correctly.
*/

{
static int      oid_storage[100];       /* Store arcs here               */
int             i,j;                    /* General indices               */
int             slice_limit;            /* End of range of slice entries */


/* If we're being called the first time, there'll be no oidarc array, so */
/* we set up our pointer to static local storage.                        */
if (oidarcs == NULL)
  oidarcs = oid_storage;

/* "ca" is an index into "mas" to the start of an index "slice".  We want   */
/* to scan down the arcs in this slice and either print the address of any  */
/* non-terminal encountered or recur to a lower level to do so if we point  */
/* at a slice.                                                              */
ca += 1;                        /* Skip the backpointer         */
slice_limit = mas[ca++];      /* Grab Entry Count             */

/* For each entry in this slice... */
for (i = 0; i < slice_limit; i++) {

  /* If this entry points to a Non-Terminal, print the Object ID */
  if (IS_A_NONTERM((mas[(ca+1)]))) {

    /* Insert this arc number into the array for printing */
    oidarcs[level] = mas[ca];

    /* Now dump out all the arcs as the Object ID of this object */
    for (j=0; j <= level; j++) {
        if (j != 0)
            fprintf (ofptr,".");
        fprintf(ofptr, "%d", oidarcs[j]);
        }

    /* Now print the address of the non-terminal */
    fprintf(ofptr, " -- [%d]\n",mas[ca+1]);

    ca += 2;
    continue;
    }

  /* If this entry points to a Slice, simply recur on it to print it out */
  if (IS_A_SLICE((mas[(ca+1)]))) {

    /* Put the current arc number into oidarc in prep for the call */
    oidarcs[level] = mas[ca];
    mirci_index_dump_BV2(mas, mas[ca+1], oidarcs, (level + 1), ofptr);
    ca += 2;
    continue;
    }

  /* If this entry points to a Subregister, print the OID for the object */
  /* and then recur to print the lower levels.                           */
  if (IS_A_SUBREG((mas[(ca+1)]))) {
    
    /* Insert this arc number into the array for printing */
    oidarcs[level] = mas[ca];

    /* Now dump out all the arcs as the Object ID of this object */
    for (j=0; j <= level; j++) {
        if (j != 0)
            fprintf (ofptr,".");
        fprintf(ofptr, "%d", oidarcs[j]);
        }

    /* Now print the address of the non-terminal from subregister */
    fprintf(ofptr, " -- [%d]\n",mas[mas[ca+1]+1]);

    /* Now recur on the lower-level pointer out of the subregister */
    mirci_index_dump_BV2(mas, mas[mas[ca+1]+2], oidarcs, (level + 1), ofptr);
    ca += 2;
    continue;
    }

  /* If we arrive here, the index points at something that it shouldn't */
  if (ofptr != NULL)
      fprintf(ofptr,
              MP(mp300,"MIR Dump: Invalid reference from index cell %d\n"),i);
  ca += 2;
  }

}

/* mirc_symdump - Dump to (screen/file) the Linear External Representation */
/* mirc_symdump - Dump to (screen/file) the Linear External Representation */
/* mirc_symdump - Dump to (screen/file) the Linear External Representation */

void mirc_symdump(ifname, ofname, raw_index)

char    *ifname;    /* --> Input file name to open                  */
char    *ofname;    /* --> Output file name string or NULL = stdout */
BOOL    raw_index;  /* TRUE: Give Raw Dump (+ symbolic) of index    */

/*
INPUTS:

    "ifname" points to the input file to be opened and dumped

    "ofname" points to a string to be opened to receive the representation
             of the input file (or NULL = dump to stdout).

    "raw_index" gives a full dump of the index in gory detail.

OUTPUTS:

    The dump is made in ascii to a file opened to receive it.  The
    dump has minimal interpretation provided.


OTHER THINGS TO KNOW:

    The "raw index" option was the way the dump was first presented. Once
    assured that the index functions were performing properly, the more
    symbolic representation was developed.  Occasionally it becomes important
    to examine the index structures in the original detail, hence this is
    still an option.

    With V1.95 of the compiler, we support two different Binary Output File
    Format Versions: 1 & 2.

*/

#define PERCENT(a,b,c)  \
{double x1,x2,x3; x1 = a; x2 = b; x3 = c; percent = ((x2 - x1) / x3) *100.0;}

{
FILE    *ofptr;     /* Output file pointer                      */
FILE    *ifptr;     /* Input file pointer                       */
int     i;          /* General index                            */


/* Print to the screen if we weren't given a file name */
if (ofname == NULL)
    ofptr = stdout;
else {
    if ( (ofptr = fopen(ofname, "w")) == NULL) {
        fprintf(stderr,
                MP(mp301,"MIR Dump: Attempt to open output file failed. %d\n"),
                errno);
        return;
        }
    }

/* Attempt to open the input file */
if ( (ifptr = fopen(ifname, "rb")) == NULL) {
    fprintf(stderr,
            MP(mp302,"MIR Dump: Attempt to open input file failed. %d\n")
            ,errno);
    return;
    }

/* Grab "endian-ness" indicator */
if (fread(&i, sizeof(i), 1, ifptr) != 1) {
    fprintf (stderr,
             MP(mp303,"MIR Dump: I/O Error on Read.\n"));
    fclose(ifptr);
    return;
    }
if (i != 1 && i != 0) {
    fprintf (stderr,
             MP(mp304,"MIR Dump: Invalid big/little \"endian\" indicator.\n"));
    fclose(ifptr);
    return;
    }

/*
| An endian-ness of ZERO is the compiler's way of signalling "forced"
| binary output: CORRUPT.
|
| We should issue a message:
*/
if (i == 0) {
    fprintf(ofptr,
            MP(mp305,">>>>>>>>>>>>>>>>>>>>>>>>>> WARNING <<<<<<<<<<<<<<<<<<<<<<<<<<\n"));
    fprintf(ofptr,
            MP(mp306,"        COMPILER BINARY OUTPUT FORCED WITH \"-X\" SWITCH\n"));
    fprintf(ofptr,
            MP(mp307,"               BINARY SHOULD BE CONSIDERED CORRUPTED\n"));
    fprintf(ofptr,
            MP(mp308,"                   BINARY IS NOT LOADABLE\n"));
    fprintf(ofptr,
            MP(mp305,">>>>>>>>>>>>>>>>>>>>>>>>>> WARNING <<<<<<<<<<<<<<<<<<<<<<<<<<\n"));
    }

/* Grab "Binary Format Version" indicator */
if (fread(&i, sizeof(i), 1, ifptr) != 1) {
    fprintf (stderr, MP(mp303,"MIR Dump: I/O Error on Read.\n"));
    fclose(ifptr);
    return;
    }

switch (i) {
    case 1:     /* Binary Output Format File Version 1 */
        do_BV1(ofptr, ifptr, raw_index, ifname);
        break;

    case 2:     /* Binary Output Format File Version 2 */
        do_BV2(ofptr, ifptr, raw_index, ifname);
        break;

    default:
        fprintf(stderr, "Unrecognized Binary Version indicator = %d.\n",i);
    } 

/* Close up shop */
fprintf(ofptr,MP(mp309,"\n\n End of Dump Reached.\n"));

if (ofname != NULL) {
    if ( fclose(ofptr) != 0) {
        fprintf(stderr,
                MP(mp310,"MIR Dump: Attempt to close output file failed.\n"));
        return;
        }
    }

if ( fclose(ifptr) != 0) {
    fprintf(stderr,
            MP(mp311,"MIR Dump: Attempt to close input file failed.\n"));
    return;
    }

}

/* do_BV1 - Dump to (screen/file) the V1 Linear External Representation */
/* do_BV1 - Dump to (screen/file) the V1 Linear External Representation */
/* do_BV1 - Dump to (screen/file) the V1 Linear External Representation */

static void
do_BV1(ofptr, ifptr, raw_index, ifname)

FILE    *ofptr;     /* --> Output file name string or NULL = stdout */
FILE    *ifptr;     /* --> Input file name to open                  */
BOOL    raw_index;  /* TRUE: Give Raw Dump (+ symbolic) of index    */
char    *ifname;    /* --> Input filename                           */

/*
INPUTS:

    "ifptr" points to the input file to be opened and dumped

    "ofptr" points to the open output file

    "raw_index" gives a full dump of the index in gory detail.

    "ifname" -> input file name (being dumped).

OUTPUTS:

    The dump is made in ascii to a file opened to receive it.  The
    dump has minimal interpretation provided.


OTHER THINGS TO KNOW:

    With V1.95 of the compiler, we support two different Binary Output File
    Format Versions: 1 & 2.  This function does the V1 support.

*/

{
int     size;       /* Size of file in bytes                    */
int     *array;     /* --> allocated storage for MIR data       */
int     ca;         /* Current Cell address                     */
int     i;          /* General index                            */
int     limit;      /* Limit for dump loops                     */
double  percent;    /* For calculating percentage of each type  */
int     major;      /* Compiler Major Version Number            */
int     minor;      /* Compiler Minor Version Number            */
int     max_oid_size; /* Maximum number of arcs in OID index    */


/* Grab Compiler Major Version indicator */
if (fread(&major, sizeof( int ), 1, ifptr) != 1) {
    fprintf (stderr, "MIR Dump: I/O Error on Read.\n");
    fclose(ifptr);
    return;
    }

/* Grab Compiler Minor Version indicator */
if (fread(&minor, sizeof( int ), 1, ifptr) != 1) {
    fprintf (stderr, "MIR Dump: I/O Error on Read.\n");
    fclose(ifptr);
    return;
    }

/* Grab Maximum Number of arcs in any oid in index */
if (fread(&max_oid_size, sizeof( int ), 1, ifptr) != 1) {
    fprintf (stderr, "MIR Dump: I/O Error on Read.\n");
    fclose(ifptr);
    return;
    }

/* Grab Size */
if (fread(&size, sizeof(size), 1, ifptr) != 1) {
    printf ("MIR Dump: I/O Error on Read.\n");
    fclose(ifptr);
    return;
    }

fprintf(ofptr,
        "Symbolic Dump of MIR Binary Database file \"%s\"\n",
        ifname);
fprintf(ofptr,
        "   Database file produced by MIR Compiler V%d.%2d\n",
        major,
        minor);
fprintf(ofptr,
        "   in Binary Output File Format Version 1\n");
fprintf(ofptr,
        "   Dump file produced by Dump Facility for MIR Compiler V%d.%d\n\n",
        VERSION_MAJOR,
        VERSION_MINOR);

fprintf(ofptr,"Total Number of 4-byte Cells: %d\n",(size/4));
fprintf(ofptr,"Longest Object Identifier: %d arcs.\n\n",max_oid_size);

/* Allocate storage big enough to receive all of the MIR Address Space
|  (Note: cell "0" in the MAS array is "not legal", so we read the MAS into
|   "array" starting at "1", so we allocate one more cell. . .
|   "sizeof(array[0])")
*/
if ( (array = (int *)malloc(size+sizeof(array[0]))) == NULL) {
    printf ("MIR Dump: Malloc failure, size = %d.\n",size);
    return;
  }

/* Now read in the rest of the MIR data */
if ((i = fread(&array[1], size, 1, ifptr)) != 1) {
    printf ("MIR Dump: fread() of input file failed. Code: %d (%s)\n",
            i, strerror(errno));
    }

/* Dump out the Partition Table */
fprintf(ofptr,
        "\n------------------------Partition Table------------------------\n");
fprintf(ofptr,"Structure\t\tStart Cell Address\tPercentage\n");

PERCENT(array[1], array[2], (size/4))
fprintf(ofptr,"Slices\t\t\t\t%d\t\t%6.2f%%\n",array[1], percent);

PERCENT(array[2], array[3], (size/4))
fprintf(ofptr,"Subregisters\t\t\t%d\t\t%6.2f%%\n",array[2], percent);

PERCENT(array[3], array[4], (size/4))
fprintf(ofptr,"Terminal Numbers\t\t%d\t\t%6.2f%%\n", array[3], percent);

PERCENT(array[4], array[5], (size/4))
fprintf(ofptr,"Terminal Strings\t\t%d\t\t%6.2f%%\n",array[4], percent);

PERCENT(array[5], (size/4), (size/4))
fprintf(ofptr,"Non-Terminals\t\t\t%d\t\t%6.2f%%\n\n",array[5], percent);

/* The goods start at 6 */
ca = 6;

/* Create a Symbolic  Dump of the Index portion of the MIR database */
fprintf(ofptr,"\nSYMBOLIC SLICE & SUBREGISTER PARTITION\n");
mirci_index_dump_BV1(array, ca, NULL, 0, ofptr);

if (raw_index == TRUE) {
  /* Create a Raw Dump of the Index portion of the MIR database */
  /* Dump the Slices out */
  /* (while the ca is less than start of terminal numbers) */
  fprintf(ofptr,"\fSLICE PARTITION\n");

  while (ca < array[2]) {

    fprintf(ofptr,"\n:[%d]\tBackptr: %d\n",ca,array[ca]);  ca += 1;
    fprintf(ofptr,":[%d]\tEntry Count: %d\n",ca,array[ca]);


    limit = array[ca++];
    for (i=0; i < limit; i++) {
/*      fprintf(ofptr,":[%d]\tArc: %d  Next: [%d]\n",ca,array[ca++],array[ca++]); */
/* MIPS compiler miss-compiles... */
        {
          int c,a,n;
          c = ca;
          a = array[ca++]; n = array[ca++];
          fprintf(ofptr,":[%d]\tArc: %d  Next: [%d]\n",c,a,n);
        }
      }
    }

  /* Dump the Subregs out */
  /* (while the ca is less than start of subregs) */
  fprintf(ofptr,"\fSUBREGISTER PARTITION\n");

  while (ca < array[3]) {
    fprintf(ofptr,"\n:[%d]\tBackptr: %d\n",ca,array[ca]);  ca += 1;
    fprintf(ofptr,":[%d]\tObject: [%d]\n",ca,array[ca]);  ca += 1;
    fprintf(ofptr,":[%d]\tLLevel: [%d]\n",ca,array[ca]);  ca += 1;
    }
  }    /* End "if raw == TRUE" */

ca = array[3];

/* Dump the Terminal Numbers out */
/* (while the ca is less than start of terminal strings) */
fprintf(ofptr,"\fTERMINAL-NUMBERS PARTITION\n");

while (ca < array[4]) {
    fprintf(ofptr,":[%d]\tNUMBER: %d\n",ca,array[ca]); ca += 1;
    }

/* Dump the Terminal Strings out */
/* (while the ca is less than start of non-terminals) */
fprintf(ofptr,"\fTERMINAL-STRINGS PARTITION\n");

while (ca < array[5]) {

    fprintf(ofptr,"\n:[%d]\tLEN: %d\n",ca,array[ca]); ca += 1;
    fprintf(ofptr,":[%d]\tVALUE: \"%s\"\n",ca,(char *) &array[ca]);

    ca += (array[ca-1] + 1) / sizeof(array[0]) +
          ((((array[ca-1] + 1) % sizeof(array[0])) == 0) ? 0 : 1);
    }


/* Dump the Non-Terminals out */
/* (while the ca is less than end of the array) */
fprintf(ofptr,"\fNON-TERMINAL PARTITION\n");

while (ca < size/sizeof(array[0])) {

    fprintf(ofptr,"\n:[%d]\tBackptr: %d\n",ca,array[ca]); ca += 1;
    fprintf(ofptr,":[%d]\tEntry Count: %d\n",ca,array[ca]);

    limit = array[ca++];

    for (i=0; i < limit; i++) {

/* This gives a "rawer" dump, spiffier code follows */
#if 0
/*      fprintf(ofptr,":[%d]\tRel: [%d]  Tar: [%d]",ca,array[ca++],array[ca]);*/
/* MIPS compiler miss-compiles... */
        { int add, reladd, taradd;
          add = ca; reladd=array[ca++]; taradd=array[ca];
          fprintf(ofptr,":[%d]\tRel: [%d]  Tar: [%d]",add,reladd,taradd);
        }
#endif
        /* Spiffier Dump Code */
        { int add, reladd, taradd;
          char  *rel_name;  /* Relationship name pointer (fancy dump)   */
          add = ca;                     /* Cell Addresss */
          reladd=array[ca++];           /* Relationship cell address */

          /* Fetch Rel String */
          /* (Note: This is super-crude.. if the compiler ever builds */
          /*  the relationship non-terminals with "name" as anything  */
          /*  other than the first entry in the relationship table    */
          /*  this code heads south.)                                 */
          rel_name =(char *) &array[array[reladd+3]+1];
          taradd=array[ca];             /* Target Cell Address       */
          fprintf(ofptr,":[%d]\tRel: %-20.20s  Tar: [%d]",add,rel_name,taradd);
        }

        /* If the target is a number or a string, show it */
        if (array[ca] >= array[3] && array[ca] < array[4]) { /* Number */
          fprintf(ofptr,"\t\'%d\'\n",array[array[ca++]]);
          }
        else if (array[ca] >= array[4] && array[ca] < array[5]) { /* String */
          fprintf(ofptr,"\t\"%s\"\n",(char *) &array[array[ca++] + 1]);
          }
        else {
          fprintf(ofptr,"\n");
          ca += 1;
          }
        }
    }
}

/* do_BV2 - Dump to (screen/file) the V2 Linear External Representation */
/* do_BV2 - Dump to (screen/file) the V2 Linear External Representation */
/* do_BV2 - Dump to (screen/file) the V2 Linear External Representation */

static void
do_BV2(ofptr, ifptr, raw_index, ifname)

FILE    *ofptr;     /* --> Output file name string or NULL = stdout */
FILE    *ifptr;     /* --> Input file name to open                  */
BOOL    raw_index;  /* TRUE: Give Raw Dump (+ symbolic) of index    */
char    *ifname;    /* Input filename                               */

/*
INPUTS:

    "ifptr" points to the input file to be opened and dumped

    "ofptr" points to the open output file

    "raw_index" gives a full dump of the index in gory detail.

    "ifname" points to the input filename (being dumped)

OUTPUTS:

    The dump is made in ascii to a file opened to receive it.  The
    dump has minimal interpretation provided.


OTHER THINGS TO KNOW:

    With V1.95 of the compiler, we support two different Binary Output File
    Format Versions: 1 & 2.  This function does the V2 support.

    With V1.98, the dump for V2 is internationalized.
*/

{
int     size;           /* Size of file in bytes                         */
int     pre_size;       /* Size of Preamble in bytes                     */
int     *preamble;      /* Preamble loaded here                          */
int     *array;         /* --> allocated storage for MIR data            */
int     ca;             /* Current Cell address                          */
int     i;              /* General index                                 */
int     limit;          /* Limit for dump loops                          */
double  percent;        /* For calculating percentage of each type       */
int     major;          /* Compiler Major Version Number                 */
int     minor;          /* Compiler Minor Version Number                 */
int     max_oid_size;   /* Maximum number of arcs in OID index           */
int     rel_count;      /* Count of number of MIR Relationship Objects   */
char   **syn_names;     /* --> List of MIR Relationship Names by synonym */
int     ca_rel;         /* Used to address MIR Relationship NT partition */
int     reladd;         /* Relationship Address                          */

int     mir_text_name_syn;  /* Records Synonym for this MIR Relationship */
int     mir_dc_smi_name_syn;/* Records Synonym for this MIR Relationship */


/* Grab Preamble Size */
if (fread(&pre_size, sizeof( int ), 1, ifptr) != 1) {
    fprintf (stderr, MP(mp303,"MIR Dump: I/O Error on Read.\n"));
    fclose(ifptr);
    return;
    }

/* Allocate a preamble array big enough to hold it all */
if ( (preamble = (int *) malloc(pre_size)) == NULL) {
    fprintf(stderr, MP(mp312,"MIR Dump: Unable to malloc memory\n"));
    return;
    }

/* Read in the Preamble */
if (fread(preamble, pre_size, 1, ifptr) != 1) {
    fprintf (stderr, MP(mp303,"MIR Dump: I/O Error on Read.\n"));
    fclose(ifptr);
    return;
    }

/* Unload the Preamble to local variables */
major = preamble[PRE_MAJOR_VER];        /* Compiler Major Version Number */
minor = preamble[PRE_MINOR_VER];        /* Compiler Minor Version Number */
max_oid_size = preamble[PRE_MAX_ARCS];  /* Maximum OID arc size          */
size = preamble[PRE_MAS_SIZE];          /* Size of MAS in bytes          */
rel_count = preamble[PRE_NT_REL_CNT];   /* Count of MIR Relationships    */


fprintf(ofptr,
        MP(mp313,"Symbolic Dump of MIR Binary Database file \"%s\"\n"),
        ifname);
fprintf(ofptr,
        MP(mp314,"   Database file produced by MIR Compiler V%d.%02d\n"),
        major,
        minor);
fprintf(ofptr,
        MP(mp315,"     in Binary Output File Format Version 2\n\n"));
fprintf(ofptr,
        MP(mp316,"This dump produced by Dump Facility for MIR Compiler V%d.%d\n\n"),
        VERSION_MAJOR,
        VERSION_MINOR);

fprintf(ofptr,MP(mp317,"Total Number of 4-byte Cells........................ %d\n"), (size/4));
fprintf(ofptr,MP(mp318,"Longest Object Identifier........................... %d arcs\n"), max_oid_size);
fprintf(ofptr,MP(mp370,"Largest General Non-Terminal........................ %d bytes\n"), preamble[PRE_NT_MAX_SIZE]);
fprintf(ofptr,MP(mp371,"Largest String Terminal............................. %d bytes\n\n"), preamble[PRE_STR_MAX_SIZE]);
fprintf(ofptr,MP(mp319,"Index Slice Count................................... %d\n"), preamble[PRE_SLICE_CNT]);
fprintf(ofptr,MP(mp320,"Index Subregister Count............................. %d\n"), preamble[PRE_SUBREG_CNT]);
fprintf(ofptr,MP(mp321,"Terminal Signed-Number Count........................ %d\n"), preamble[PRE_SIGNED_CNT]);
fprintf(ofptr,MP(mp322,"Terminal Unsigned-Number Count...................... %d\n"), preamble[PRE_UNSIGNED_CNT]);
fprintf(ofptr,MP(mp323,"Terminal String Count............................... %d\n"), preamble[PRE_STRING_CNT]);
fprintf(ofptr,MP(mp324,"DataConstruct Non-Terminal Count.................... %d\n"), preamble[PRE_NT_DC_CNT]);
fprintf(ofptr,MP(mp325,"General Non-Terminal Count.......................... %d\n"), preamble[PRE_NT_GEN_CNT]);
fprintf(ofptr,MP(mp326,"MIR Relationship Object NT Count.................... %d\n"), rel_count);

/*
|   If the compiler has been changed to make the Preamble bigger (a legal
|   thing to do without changing the Binary Format Version number), then
|   we endeavor to dump (in hex) the rest of the stuff that this code
|   doesn't know how to interpret.
*/
#define KNOWN_PREAMBLE_SIZE 56
if (pre_size > KNOWN_PREAMBLE_SIZE) {
     fprintf(ofptr,
             MP(mp327,"Remaining Preamble (%d bytes) Uninterpreted Dump:\n "),
             (pre_size - KNOWN_PREAMBLE_SIZE));
     fprintf(ofptr, MP(mp328,"Preamble Cell      Contents\n"));
     for (i=(KNOWN_PREAMBLE_SIZE/sizeof(int));
          i < pre_size/sizeof(int);
          i++) {
         fprintf(ofptr, MP(mp329,"    [%d]:                %#x\n"), i, preamble[i]);
         }
     }

/* Allocate storage big enough to receive all of the MIR Address Space
|  (Note: cell "0" in the MAS array is "not legal", so we read the MAS into
|   "array" starting at "1", so we allocate one more cell. . .
|   "sizeof(array[0])")
*/
if ( (array = (int *)malloc(size+sizeof(array[0]))) == NULL) {
    fprintf (ofptr, MP(mp330,"MIR Dump: Malloc failure, size = %d.\n"),size);
    return;
  }

/* Now read in the rest of the MIR data */
if ((i = fread(&array[1], size, 1, ifptr)) != 1) {
    fprintf (ofptr,
             MP(mp331,"MIR Dump: fread of input file failed. Code: %d (%s)\n"),
             i, strerror(errno));
    }

/* Dump out the Partition Table */
fprintf(ofptr,
        MP(mp332,"\n--------------------------Partition-Table----------------------\n"));
fprintf(ofptr,
        MP(mp333,"Structure\t\tStart Cell Address\tPercentage\n"));
fprintf(ofptr,
        MP(mp334,".........\t\t..................\t..........\n"));

PERCENT(array[ROOT_INDEX_MASA], array[START_SUBREG_MASA], (size/4))
fprintf(ofptr,
        MP(mp335,"Index Slices\t\t\t%d\t\t%6.2f%%\n"),
        array[ROOT_INDEX_MASA], percent);

PERCENT(array[START_SUBREG_MASA], array[START_SNUMBER_MASA], (size/4))
fprintf(ofptr,
        MP(mp336,"Index Subregisters\t\t%d\t\t%6.2f%%\n"),
        array[START_SUBREG_MASA], percent);

PERCENT(array[START_SNUMBER_MASA], array[START_UNUMBER_MASA], (size/4))
fprintf(ofptr,
        MP(mp337,"Terminal SIGNED Numbers\t\t%d\t\t%6.2f%%\n"),
        array[START_SNUMBER_MASA], percent);

PERCENT(array[START_UNUMBER_MASA], array[START_STRING_MASA], (size/4))
fprintf(ofptr,
        MP(mp338,"Terminal UNSIGNED Numbers\t%d\t\t%6.2f%%\n"),
        array[START_UNUMBER_MASA], percent);

PERCENT(array[START_STRING_MASA], array[START_NONTERM_DC_MASA], (size/4))
fprintf(ofptr,
        MP(mp339,"Terminal Strings\t\t%d\t\t%6.2f%%\n"),
        array[START_STRING_MASA], percent);

PERCENT(array[START_NONTERM_DC_MASA], array[START_NONTERM_MASA], (size/4))
fprintf(ofptr,
        MP(mp340,"Non-Terminal DATA-CONSTRUCTS\t%d\t\t%6.2f%%\n"),
        array[START_NONTERM_DC_MASA], percent);

PERCENT(array[START_NONTERM_MASA], array[START_NONTERM_REL_MASA], (size/4))
fprintf(ofptr,
        MP(mp341,"Non-Terminal (GENERAL)\t\t%d\t\t%6.2f%%\n"),
        array[START_NONTERM_MASA], percent);

PERCENT(array[START_NONTERM_REL_MASA], (size/4), (size/4))
fprintf(ofptr,
        MP(mp342,"Non-Terminal MIR RELATIONSHIPS\t%d\t\t%6.2f%%\n\n"),
        array[START_NONTERM_REL_MASA], percent);


fprintf(ofptr,
        MP(mp343,"Non-Terminal Relationship-Table Entry Count AND-Mask.. %#X\n"),
        array[ENTRY_COUNT_AMASK_MASA]);

fprintf(ofptr,
        MP(mp344,"Non-Terminal OID-Backpointer Count Right-SHIFT Count.. %d\n"),
        array[OID_COUNT_RSHIFT_MASA]);

fprintf(ofptr,
        MP(mp345,"Non-Terminal OID-Backpointer AND-Mask................. %#X\n"),
        array[OID_BPTR_AMASK_MASA]);

fprintf(ofptr,
        MP(mp346,"Non-Terminal OID-SMI Indicator Right-SHIFT Count...... %d\n"),
        array[OID_SMI_RSHIFT_MASA]);

fprintf(ofptr,
        MP(mp347,"Non-Terminal Synonym AND-Mask......................... %#X\n"),
        array[SYNONYM_AMASK_MASA]);

fprintf(ofptr,
        MP(mp348,"Non-Terminal Target Right-SHIFT Count................. %d\n"),
        array[TARGET_RSHIFT_MASA]);


fprintf(ofptr,
        MP(mp349,"\nTargets of Relationships (in a Non-Terminal) are displayed as follows:\n"));
fprintf(ofptr,
        MP(mp350,"      Non-Terminal with a name -> NT(<name>)  (e.g. NT(Node)  )\n"));
fprintf(ofptr,
        MP(mp351,"     Signed or Unsigned Number -> '<number>'  (e.g. '1'       )\n"));
fprintf(ofptr,
        MP(mp352,"                        String -> \"<string>\"  (e.g. \"Hi There\")\n"));


/* The goods start here */
ca = FIRST_MASA_AFTER_P_TABLE;

/* --------------------------------------------------------------------------*/
/* Create a Symbolic  Dump of the Index portion of the MIR database */
fprintf(ofptr,"\n\nINDEX DISPLAY: Slice & Subregister Partitions\n");
mirci_index_dump_BV2(array, ca, NULL, 0, ofptr);

if (raw_index == TRUE) {
  /* Create a Raw Dump of the Index portion of the MIR database */
  /* Dump the Slices out */
  /* (while the ca is less than start of terminal numbers) */
  fprintf(ofptr, MP(mp353,"\fSLICE PARTITION\n"));

  while (ca < array[START_SUBREG_MASA]) {

    fprintf(ofptr, MP(mp354,"\n%d:\tBackptr: %d\n"),ca,array[ca]);   ca += 1;
    fprintf(ofptr, MP(mp355,"%d:\tEntry Count: %d\n"),ca,array[ca]);


    limit = array[ca++];
    for (i=0; i < limit; i++) {
/*      fprintf(ofptr,":[%d]\tArc: %d  Next: [%d]\n",ca,array[ca++],array[ca++]); */
/* MIPS compiler miss-compiles... */
        {
          int c,a,n;
          c = ca;
          a = array[ca++]; n = array[ca++];
          fprintf(ofptr,MP(mp356,"%d:\tArc: %d  Next: [%d]\n"),c,a,n);
        }
      }
    }

  /* Dump the Subregs out */
  /* (while the ca is less than start of subregs) */
  fprintf(ofptr, MP(mp357,"\fSUBREGISTER PARTITION\n"));

  while (ca < array[START_SNUMBER_MASA]) {
    fprintf(ofptr, MP(mp358,"\n%d:\tBackptr: %d\n"),ca,array[ca]);  ca += 1;
    fprintf(ofptr, MP(mp359,"%d:\tObject: [%d]\n"),ca,array[ca]);  ca += 1;
    fprintf(ofptr, MP(mp360,"%d;\tLLevel: [%d]\n"),ca,array[ca]);  ca += 1;
    }
  }    /* End "if raw == TRUE" */

ca = array[START_SNUMBER_MASA];


/* --------------------------------------------------------------------------*/
/* Dump the Terminal SIGNED Numbers out */
/* (while the ca is less than start of UNSIGNED terminal numbers) */
fprintf(ofptr,MP(mp361,"\fTERMINAL SIGNED-NUMBERS PARTITION\n"));

while (ca < array[START_UNUMBER_MASA]) {
    fprintf(ofptr, MP(mp362,"%d:\tNUMBER: %d\n"),ca,array[ca]); ca += 1;
    }



/* --------------------------------------------------------------------------*/
/* Dump the Terminal UNSIGNED Numbers out */
/* (while the ca is less than start of UNSIGNED terminal numbers) */
fprintf(ofptr, MP(mp363,"\fTERMINAL UNSIGNED-NUMBERS PARTITION\n"));

while (ca < array[START_STRING_MASA]) {
    fprintf(ofptr,"%d:\tNUMBER: %u\n",ca,array[ca]); ca += 1;
    }



/* --------------------------------------------------------------------------*/
/* Dump the Terminal Strings out */
/* (while the ca is less than start of non-terminals) */
fprintf(ofptr, MP(mp364,"\fTERMINAL-STRINGS PARTITION\n"));

while (ca < array[START_NONTERM_DC_MASA]) {

    fprintf(ofptr,MP(mp365,"\n%d:\tLEN: %d\n"),ca,array[ca]); ca += 1;
    fprintf(ofptr,MP(mp366,"%d:\tVALUE: \"%s\"\n"),ca,(char *) &array[ca]);

    ca += (array[ca-1] + 1) / sizeof(array[0]) +
          ((((array[ca-1] + 1) % sizeof(array[0])) == 0) ? 0 : 1);
    }


/*
| In order to give an interpreted dump of the NTs, we need to have an array
| of MIR Relationship names arranged by "synonym".
|
| We build this here, using a few assumptions about the way the compiler
| builds the MIR Objects that stand for MIR Relationships.  This code assumes:
|
|       * The compiler assigns only one OID to a MIR Relationship Object
|       * There is only one entry in the Relationship Table for each MIR
|         Relationship Object
|       * The one entry is for relationship "MIR_Relationship_name" and the
|         target is the string-name of the relationship.
|
| Using these assumptions allows us to efficiently "grab" the string name
| of the Relationship without having to go thru a full-fledged "parse" of the
| NT itself.
|
| The NT Dump function is Very Interested in the synonym numbers for the two
| MIR Relationships that serve to "name" things most often:
|       "MIR_Text_Name" and "MIR_DC_SMI_Name".  This code watches out for
| these synonyms and records them especially for use in calls to the dump
| function.
|
| NOTE:  This array is "one-origined" (because synonym assignments begin with
|        "1", consequently we make this array one entry larger than the actual
|        number of relationship synonyms we need names for ([0] is empty)
*/
if ((syn_names = (char **) malloc((rel_count+1)*sizeof(char *))) == NULL) {
    fprintf (ofptr, MP(mp330,"MIR Dump: Malloc failure, size = %d.\n"),
             rel_count*sizeof(char *));
    return;
    }

/* Load pointers to names for each NT in the MIR Relationship NT Partition */
ca_rel = array[START_NONTERM_REL_MASA];   /* Point to start of partition   */
i = 1;                                    /* First synonym is "1"          */

while (rel_count > 0) {

     /*
     |  Bump to 1st Relationship Table Entry
     |  (over packed count and oid back pointer
     */
     ca_rel += 2;

     /*
     |  Obtain Target address which should be address of a string for the
     |  Relationship name.
     */
     reladd = ((unsigned int) array[ca_rel++]) >> array[TARGET_RSHIFT_MASA];
     syn_names[i] = (char *) &array[reladd+1];/* "+1"=Skip the Length word */

     /* Check for "MIR_Text_Name" and "MIR_DC_SMI_Name" */
     if ( strcmp(syn_names[i], "MIR_Text_Name") == 0)
         mir_text_name_syn = i;
     else if (strcmp(syn_names[i], "MIR_DC_SMI_Name") == 0)
         mir_dc_smi_name_syn = i;

     i += 1;
     rel_count -= 1;
     }


/* --------------------------------------------------------------------------*/
/* Dump the DATACONSTRUCT Non-Terminals out */
/* (while the ca is less than end of the array) */
fprintf(ofptr,"\fDATACONSTRUCT NON-TERMINAL PARTITION\n");

while (ca < array[START_NONTERM_MASA]) {
    dump_BV2_NT(&ca, array, ofptr, syn_names,
                mir_text_name_syn, mir_dc_smi_name_syn, max_oid_size);
    }


/* --------------------------------------------------------------------------*/
/* Dump the GENERAL  Non-Terminals out */
/* (while the ca is less than end of the array) */
fprintf(ofptr,"\fGENERAL NON-TERMINAL PARTITION\n");

while (ca < array[START_NONTERM_REL_MASA]) {
    dump_BV2_NT(&ca, array, ofptr, syn_names,
                mir_text_name_syn, mir_dc_smi_name_syn, max_oid_size);
    }


/* --------------------------------------------------------------------------*/
/* Dump the MIR-RELATIONSHIP Non-Terminals out */
/* (while the ca is less than end of the array) */
fprintf(ofptr,"\fMIR-RELATIONSHIP NON-TERMINAL PARTITION\n");

while (ca < size/sizeof(array[0])) {
    dump_BV2_NT(&ca, array, ofptr, syn_names,
                mir_text_name_syn, mir_dc_smi_name_syn, max_oid_size);
    }

}

/*
| Here we include the source of local static function "get_oid_index()" that
| serves the functions in both "mir_t0.c" and "mir_symdump.c" (ie. here).
| This allows the object module produced from "mir_t0.c" to be wholely
| "self-sufficient", requiring no other object module to be linked with it.
|
| In essence, we're doing a compile-time link for the purposes of link-time
| (and kit generation) simplicity.
*/
#include "mir_goi.cinc"


/* dump_BV2_NT - Dump Binary Format Version 2 Non-Terminal MIR Object */
/* dump_BV2_NT - Dump Binary Format Version 2 Non-Terminal MIR Object */
/* dump_BV2_NT - Dump Binary Format Version 2 Non-Terminal MIR Object */

void dump_BV2_NT(ca_ptr, mas, ofptr, syn_names, mir_text_name_syn,
                 mir_dc_smi_name_syn, max_oid_arcs)

int     *ca_ptr;             /* --> Cell Address to start at                 */
int     *mas;                /* Array of MIR Address Space                   */
FILE    *ofptr;              /* --> Output File Pointer                      */
char    *syn_names[];        /* Array of MIR Relationship Names by "synonym" */
int     mir_text_name_syn;   /* Synonym for MIR_Text_Name                    */
int     mir_dc_smi_name_syn; /* Synonym for MIR_Text_Name                    */
int     max_oid_arcs;        /* Count of arcs in largest OID                 */

/*
INPUTS:

    "ca_ptr" points to the the cell address to begin dumping with.

    "mas" points to the MIR Address Space array.

    "ofptr" points to the file to which output is done.

    "syn_name" --> array of pointers to MIR Relationship names by synonym.

    "mir_text_name_syn" and "mir_dc_smi_name_syn" are needed to give a nice
    interpreted dump.


OUTPUTS:

    The dump is made in ascii to the specified file.  On return, the
    cell at "ca_ptr" receives the address of the next cell following
    the NT that was dumped.


OTHER THINGS TO KNOW:

    With V1.95 of the compiler, we support two different Binary Output File
    Format Versions: 1 & 2.  This function does the V2 support for the
    new compressed Non-Terminal format.

*/

{
int     ca;                     /* "Cell Address": where we're at       */
int     rel_count;              /* Relationship count extracted         */
int     oid_count;              /* OID Count extracted                  */
int     oid_bptr;               /* OID back-pointer address             */
int     oid_smi;                /* OID SMI indicator                    */
int     synonym;                /* MIR Relationship Synonym             */
int     target;                 /* Relationship Target Address          */
int     string_name_target;     /* Target of String name of NT target   */
int     NT_rel_count;           /* NT Target's relationship count       */
int     NT_masa;                /* Stored NT address for OID dump       */
int     first_oid_bptr;         /* MASA of first (if any) OID backptrs  */
int     skip_count;             /* Skip Count of OID matches            */
int     back_scan;              /* Index MASA for OID backptr scan      */

unsigned int oid_arc_count;     /* Count of arcs in Fetched OID         */
unsigned int j;                 /* General Index                        */
static
unsigned int *oid_arcs=NULL;   /* Once-allocated storage for OID arcs  */

static  char *smi_string[MAX_OID_SMI+1] = {
  "OID_MCC ", "OID_OID ", "OID_DNA ", "OID_OSI ", "OID_SNMP",
  /* Add new SMI names HERE */

  "OID_ANY "};


/* If we've never been called before, allocate storage to receive OID arcs */
if (oid_arcs == NULL) {
    if ((oid_arcs = (unsigned int *) malloc(max_oid_arcs*sizeof(int))) == NULL) {
        fprintf(ofptr, MP(mp367,"MIR Dump: Out of memory\n"));
        fclose(ofptr);
        exit(BAD_EXIT);
        }
    }

ca = *ca_ptr;                     /* Cell Addresss */

/*
| Dump the First cell of the Non-Terminal that contains packed counts for
| the OID Backpointers and Relationship Table:
|              Hi                                Lo
|                [<OID bptr Count> <Entry Count>]
|
| Special AND-Masks and Right-Shift are used to break the stuff out.
*/
rel_count = mas[ca] & mas[ENTRY_COUNT_AMASK_MASA];
oid_count = ((unsigned int) mas[ca]) >> mas[OID_COUNT_RSHIFT_MASA];
fprintf(ofptr,MP(mp368,"\n%d: (Contents %#x)   OID Count...%d   Rel-Entry Count...%d\n"),
        ca, mas[ca], oid_count, rel_count);

NT_masa = ca;
ca += 1;
first_oid_bptr = ca;

/*
| Dump any optional OID backpointers
*/
while (oid_count > 0) {

    oid_bptr = mas[ca] & mas[OID_BPTR_AMASK_MASA];
    oid_smi =  ((unsigned int) mas[ca]) >> mas[OID_SMI_RSHIFT_MASA];

    fprintf(ofptr, MP(mp369,"%d: (Contents %#x) %s     ("),
            ca, mas[ca], smi_string[oid_smi]);

    /*
    | For disambiguation purposes, we need to generate the skip-count:
    | the number of matches "get_oid_index()" should skip before accepting
    | a hit.  To do this, we march "backwards" to the first oid backpointer
    | checking (and counting) all previous matches.
    */
    for (back_scan = ca - 1, skip_count = 0;
         back_scan >= first_oid_bptr;
         back_scan -= 1) {

        if ( (mas[back_scan] & mas[OID_BPTR_AMASK_MASA]) == oid_bptr)
            skip_count += 1;
        }

    /*
    | Extract OID from index using "common" MIR_T0/SYMDUMP function
    | "get_oid_index()".
    */
    oid_arc_count = 0;
    if (get_oid_index(mas,              /* MIR address space array        */
                      oid_bptr,         /* Address of Slice in Index      */
                      skip_count,       /* Skip Count of matches to skip  */
                      NT_masa,          /* Address of this Non-Terminal   */
                      &oid_arc_count,   /* # of arcs in OID returned here */
                      oid_arcs)         /* Arcs for OID arrive here       */
        == MS_SUCCESS) {

        /* We've got the OID, turn it into ASCII */
        for (j=0; j < oid_arc_count; j++) {
            if (j != 0) fprintf(ofptr, ".");
            fprintf(ofptr, "%d", oid_arcs[j]);
            }
        }

    /* Conversion blew, just \n */
    fprintf(ofptr, ")-[%d]\n", oid_bptr);

    ca += 1;
    oid_count -= 1;
    }


/*
| Dump any and all Relationship Table Entries
*/
for (; rel_count > 0; rel_count -= 1) {

    synonym = mas[ca] & mas[SYNONYM_AMASK_MASA];
    target  = ((unsigned int) mas[ca]) >> mas[TARGET_RSHIFT_MASA];

    fprintf(ofptr,
            "%d: %02d %-22.22s [%d]",
            ca,
            synonym,
            syn_names[synonym],
            target);

    /*
    | If the target is a number or a string, show it
    */
    if (IS_A_SNUMBER(target)) {         /* Signed Number */
        fprintf(ofptr,"\t\'%d\'\n",mas[target]);
        }
    else if (IS_A_UNUMBER(target)) {    /* Unsigned Number */
        fprintf(ofptr,"\t\'%u\'\n",mas[target]);
        }
    else if (IS_A_STRING(target)) {     /* String */
        fprintf(ofptr,"\t\"%s\"\n",(char *) &mas[target + 1]);
        }
    else {      /* Else its a NT: Try and get a name (MIR_Text_Name) */
        NT_rel_count = mas[target] & mas[ENTRY_COUNT_AMASK_MASA];
        oid_count = ((unsigned int) mas[target]) >> mas[OID_COUNT_RSHIFT_MASA];

        target += oid_count + 1;        /* Step to Rel. Table */

        /*
        | Scan the Relationship Table for either MIR_Text_Name or 
        | MIR_DC_SMI_Name, and give 'em a hint of the target
        */
        while (NT_rel_count > 0) {
            synonym = mas[target] & mas[SYNONYM_AMASK_MASA];
            string_name_target  = ((unsigned int) mas[target])
                                   >> mas[TARGET_RSHIFT_MASA];

            if (   synonym == mir_text_name_syn
                || synonym == mir_dc_smi_name_syn) {
                fprintf(ofptr,
                        "\tNT(%s)\n",
                        (char *) &mas[string_name_target + 1]);
                break;
                }
            target += 1;        /* Step to next Relationship Table Entry */
            NT_rel_count -= 1;  /* Show a miss on this one               */
            }

        /* If we found no match, just give 'em a CR/LF */
        if (NT_rel_count == 0)
            fprintf(ofptr,"\n");
        }

    ca += 1;
    }

/* Return where "we're at" to the caller */
*ca_ptr = ca;
}
