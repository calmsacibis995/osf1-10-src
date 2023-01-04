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
static char *rcsid = "@(#)$RCSfile: mir_external.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:59:34 $";
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
 * Module MIR_EXTERNAL.C
 *      Contains functions required by the MIR Compiler to generate the
 *      "externalized" representation of the "interemediate" form of the
 *      MIR data.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   Sept 1990
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
 *       compiler to generate the final "external" binary file containing
 *       information to be loaded into a MIR.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *   Version    Date            Who             What
 *      V0.0    December 1990   D. D. Burns     Original Version
 *      V1.0    February 1991   D. D. Burns     Version 1.0 of Compiler
 *      V2.0    July     1992   D. D. Burns     Version 2.0 of Compiler
                                                      & Binary Format 2
Module Overview:

This module contains all the functions used by the MIR Compiler to generate the
external file containing the "externalized" representation of the
"intermediate" representation of the MIR data.

All these functions' names begin with the letter "E".


MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
e_gen_external       Opens the output file and copies the in-heap intermediate
                     representation of the MIR data to the output file in
                     binary.

INTERNAL FUNCTIONS:
e_conv_ids_type     When passed an intermediate data structures (IDS)
                    of any flavor, this function copies the structures
                    to the output file doing the necessary conversions.
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef VMS
#include <malloc.h>
#endif

/* Request definitions for compiler modules from "mir.h" */
#define MIR_COMPILER
#include "mir.h"


/*
|  This corresponds directly with the "-i" and "-ib" compiler flags
|  defined in mir_frontend.c
*/
extern BOOL supplementary_info;
extern BOOL supplementary_time;


/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp600();
extern char *mp601();
extern char *mp602();
extern char *mp603();
extern char *mp604();
extern char *mp605();
extern char *mp606();
extern char *mp607();
extern char *mp608();
extern char *mp609();
#endif


/*
|  Defined in mir_yacc.y -- needed here too see if an error occurred
*/
extern int eflag;


/*
|
|  Prototypes for module-local functions
|
*/

/* e_conv_ids_type - Convert Intermediate IDS to External Format */
static void e_conv_ids_type PROTOTYPE((
IDS         *,      /* -> Next IDS structure to convert               */
int         *,      /* -> integer containing cell address to start at */
int         *,      /* -> array of cells (ints) where data is written */
int         *       /* -> Counter for this type of IDS                */
));


/* e_gen_external - Generate External Representation of MIR Data */
/* e_gen_external - Generate External Representation of MIR Data */
/* e_gen_external - Generate External Representation of MIR Data */

BOOL e_gen_external (icb, fname)

inter_context  *icb;       /* Pointer to Intermediate Context Block */
char           *fname;     /* Filename to use in writing the file    */

/*
INPUTS:

    "icb" is simply the pointer to the Intermediate Context Block
    containing the heads of the lists that make up the intermediate
    representation of the compiled MSL files.

    "fname" is the filename to use in creating the external form of the
    data.

OUTPUTS:

    The function returns one of:

    TRUE - A successful copy was made to the file and the file was closed
    without error.

    FALSE - An error occurred, the file is closed (if possible) and an
    error message has been printed.

BIRD'S EYE VIEW:
    Context:
        The caller has completed the construction of the Intermediate
        representation of the MIR data in the heap.

    Purpose:
        This function passes down the linear lists of each different
        Intermediate Data Structure and copies each to the output file
        after creating and writing a preamble section to the file.
        

ACTION SYNOPSIS OR PSEUDOCODE:

    <set-up synonyms in Non-Terminals representing MIR Relationships>

    <set current write-cell address to after end of partition table>

    (* FIRST PASS *)
    for (each kind of IDS type)

        <record cell address of start of this-type>
        while (list pointer of this-type is non-null)
            <bind the entry on this list to external address (masa)
             via e_conv_ids_type>
            <step to the next pointer in current entry>

    <allocate enough storage from the heap to hold it all>
    <write the size in bytes of the "MIR address space" into cell zero of
     the "MIR address space">
    
    <reset the write-cell address to one>
    <write the partition table into the array>

    (* SECOND PASS *)
    for (each kind of IDS type)

        while (list pointer of this-type is non-null)
            <copy the top of the list to external via e_conv_ids_type>
            if (object was a MIR-Relationship Non-Terminal Object)
                <count it>
            <step to the next pointer in current entry>

    if (attempt to open the specified file FAILED)
        <print error message>
        <return FALSE>

    if (no error occurred)
        <write a 4-byte "endian-ness" indicator "1" to output file>
    else
        <write a bogus 4-byte "endian-ness" indicator "0" to output file>
    <write the 4-byte binary output-file format version number>
    <write the Preamble size>
    <write the 4-byte compiler major version number>
    <write the 4-byte compiler minor version number>
    <write the 4-byte arc-count number (longest OID in index)>

    if (write the first cell to file FAILED)
        <return FALSE>

    if (write the rest of the array to the file FAILED)
        <return FALSE>

    if (attempt to close the file failed)
        <print error message>
        <return FALSE>

    <return TRUE>

CALLS:
    Function        Module      Reason/Purpose
    e_conv_ids_type <here>      Performs the conversion of each of the
                                different IDS structures.

OTHER THINGS TO KNOW:

The format of the MIR Compiler Binary Output File is entirely determined by
this function.

By convention, this documentation section describes the format of the Binary
Output File, for *ALL* versions of the compiler.  Of course, the compiler
only actually generates the latest version.  The symbolic dump functions in
module "mir_symdump.c" must *ALWAYS* be able to dump any Binary Output file,
regardless of Binary Format Version.  Consequently, the description of *ALL*
versions is maintained here.

By documentation convention, the internal format of all of the file is
regarded as simply a series of 4-byte words.

By convention, the Binary Output File is divided into "The Header" and
"Sections".

By convention, the structure of The Header is "Binary Version" invariant
(ie, it never changes, regardless of the Binary File Output Format Version).

The Header is structured as follows:

4-Byte Offset
  Into File     Name                    Use
      0         Endian-ness Indicator   Compiler writes a binary "1" into
                                        this word.  Any runtime access function
                                        written in C can fread() this word
                                        and determine what "endian-ness"
                                        conversions may be required to read
                                        the rest of the file.

      1         Binary File Output      Compiler writes the binary value
                Format Version Number   of symbol "BINARY_FORMAT" (defined
                                        in "mir_compiler.h") into this word.

Following The Header come one or more "Sections".  For both Binary File Format
Versions 1 and 2 the output file is divided into two sections:

    * "PREAMBLE"
      Here the compiler records information about the compilation that may
      be useful, but not absolutely necessary for the run-time interpretation
      of the MAS section, or information that should appear in a symbolic dump.

    * "MAS" (MIR Address Space)
      This is where all the compiled MIR data reside.

The internal format of both of these sections is differs between Version 1 and
2.

By convention, only NEW "sections" may be added to subsequent new Binary
Formats (preferably AFTER these two sections, but possibly after the PREAMBLE
and before the MAS).

===============================================================================

The Binary File Format Version 1 PREAMBLE is structured into 4-byte word
fields as follows:

4-Byte Offset
  Into File     Name                    Use
      2         Compiler Major Version  Compiler writes the binary value
                                        of symbol "MAJOR_VERSION" (defined
                                        in "mir_compiler.h") into this word.

      3         Compiler Minor Version  Compiler writes the binary value
                                        of symbol "MINOR_VERSION" (defined
                                        in "mir_compiler.h") into this word.

      4         Maximum Arc-Count       Compiler writes the number of arcs
                                        in the longest OID in the index into
                                        this word.

      5         MIR Address Space Size  Compiler writes the size *IN BYTES*
                                        of the following "MIR Address Space"
                                        section into this word.

      6 (--> Start of MAS Section)

By convention, Version 1 PREAMBLE is FIXED in length as shown above.

===============================================================================

The Binary File Format Version 2 PREAMBLE is structured into 4-byte word
fields as follows:

4-Byte Offset
  Into File     Name                    Use
      2         Preamble Size in Bytes  Indicates the size in bytes of the
                                        remainder of the Preamble in the file.
                                        Since the Preamble is structured in
                                        4-byte words, this number is always
                                        a multiple of 4.  For Version 2, the
                                        value of this word is always at least
                                        20 (decimal) to cover the next 5
                                        words described below.

      3         Compiler Major Version  Compiler writes the binary value
                                        of symbol "MAJOR_VERSION" (defined
                                        in "mir_compiler.h") into this word.

      4         Compiler Minor Version  Compiler writes the binary value
                                        of symbol "MINOR_VERSION" (defined
                                        in "mir_compiler.h") into this word.

      5         Maximum Arc-Count       Compiler writes the number of arcs
                                        in the longest OID in the index into
                                        this word.

      6         MIR Address Space Size  Compiler writes the size *IN BYTES*
                                        of the following "MIR Address Space"
                                        section into this word.

      (The following  values simplifies coding in the compiler that loads a
        binary file as part of a compilation).

      7         Index Slice  Count      Indicates the number of MIR Objects
                                        that represent OID Index "Slices"
                                        contained in the Slice
                                        Partition within the MAS section.

      8         Index Subreg.  Count    Indicates the number of MIR Objects
                                        that represent OID Index "Subregisters"
                                        contained in the Subregister
                                        Partition within the MAS section.

      9         Signed-Number Count     Indicates the number of MIR Objects
                                        that represent Terminal "Signed
                                        Numbers"contained in the Signed-Number
                                        Partition within the MAS section.

     10         Unsigned-Number Count   Indicates the number of MIR Objects
                                        that represent Terminal "Unsigned
                                        Numbers"contained in the Unigned-Number
                                        Partition within the MAS section.

     11         String Count            Indicates the number of MIR Objects
                                        that represent Terminal "Strings"
                                        contained in the String
                                        Partition within the MAS section.

     12         NTs for DataConstruct   Indicates the number of MIR Objects
                     Count              that represent Non-Terminal Data-
                                        Constructs contained in the NT
                                        DataConstruct Partition within the MAS.

     13         General Non-terminal    Indicates the number of MIR Objects
                     Count              that represent General Non-Terminals
                                        contained in the General Non
                                        Terminal Partition within the MAS.

     14         NTs for MIR             Indicates the number of MIR Objects
                Relationship Count      that represent MIR Relationships
                                        contained in the Non-Terminal MIR
                                        Relationship Partition within the
       .                                MAS section.

       .

    (other PREAMBLE information not yet defined).

By convention, Version 2 PREAMBLE is VARIABLE in length.  By convention,
the MIR Tier 0 function and the Symbolic Dump function that loads a Binary
Version 2 file must be able to handle this variable length PREAMBLE.  The
Symbolic dump is coded is such a fashion that it interprets as much as it
can of the PREAMBLE, and dumps in hex whatever it can't understand.

By convention, the Binary Format Version number is *NOT* changed if new
information is added into the PREAMBLE (in subsequent versions of the compiler)
in such a way that the MIR Tier 0 function and Symbolic Dump function can
still operate properly.

As experience is gained in a paged environment, it may turn out that the
compiler could record useful information about the structure of the MAS
that could help the paging mechanism work more efficiently.  Since the goal
is to maintain versions of MIR Tier 0 functions that are both paged and
non-paged, then any extra information needed for the paged version is
superfluous for the non-paged version.  Consequently the Binary Version 2
PREAMBLE is made variable in length so that information of this ilk may be
recorded without the need to constantly change the Binary Format Version
number (signifying a different internal file structure).

The convention is that information NEEDED for proper run-time use of the
MAS should be recorded IN THE MAS, everything else goes in the PREAMBLE.

===============================================================================
===============================================================================

The PREAMBLE section is followed by the MAS section in both Binary Version
1 & 2.

===============================================================================
===============================================================================
MAS For Binary File Format Version 1:

Begins at fixed 4-Byte Offset Into File "6".

The compiler writes the binary representation of the compilation into the file
starting at this point, and going on for it's full size as specified in
file offset "5" of the PREAMBLE.

The MIR Address Space (MAS) is consists of six partitions that appear in the
following order:

Partition-Table Partition - This is merely a series of (4-byte) cells that
       contain the MIR-Address-Space Addresses ("masa"s) for the remaining
       partitions in the MIR Address Space:

       masa     contents
         1      masa of the "Slice" Partition - (top of ISO Object ID Index)
         2      masa of the "Subregister" Partition - (part of Index)
         3      masa of the "Terminal Numbers" Partition (compiled data)
         4      masa of the "Terminal Strings" Partition (compiled data)
         5      masa of the "Non-Terminal Objects" Partition (bulk of
                     compiled data).

         6 -->  (The Slice Partition begins here and the top-most slice
                 in the index is first in the partition.  Therefore, MAS
                 cell with masa of "1" (in the Partition Table) always
                 has a "6" in it . . . in this binary output format version).

       Note that cell with masa of 0 in the MIR Address Space is undefined and
       a masa of zero is considered illegal to reference when the MIR Address
       Space is loaded for reference by Tier 0.  We're one-origined.

Slice Partition - Contains MIR Objects that form the bulk of the OID Index.
       While the compiler regards these objects as bona fide "MIR Objects",
       the MIR Tier 0 lookup functions regard both this partition and the
       following Subregister Partition as a single intertwined "lump" which
       is traversed by a single subroutine.  These index MIR 'objects' cannot
       be 'fetched' by any Tier 0 function in the way that true compiled
       MIR objects (derived from an MSL file and stored in the Non-Terminal
       partition) can be found, fetched and manipulated.

Subregister Partition - Contains supporting MIR Objects that are needed for
       a complete OID Index.  Note the considerations mentioned above.

Terminal Numbers - All numbers (signed and unsigned) derived from the input
       MSL files are recorded ONCE here in this partition, in ascending order.

Terminal Strings - All strings derived from the input MSL files are recorded
       ONCE here in this partition.  They are in lexicographical order and
       preceded with a length word (4 bytes) and the strings are null
       terminated (null byte not counted in the length).  The strings are
       typically valid ASCII characters.

Non-Terminal Objects - The bulk of the compiled MSL information is recorded
       in this partition as MIR Objects.  These are the objects which can
       be specified by OID and then searched for relationship matches by
       MIR Tier 0 and Tier 1 functions.

===============================================================================

MAS For Binary File Format Version 2:

Begins at a variable 4-Byte Offset Into File depending on the actual size of
the PREAMBLE.

The compiler writes the binary representation of the compilation into the file
starting at this point, and going on for it's full size as specified in the
PREAMBLE.

The MIR Address Space (MAS) is consists of nine partitions that appear in the
following order:

Partition-Table Partition - This is merely a series of (4-byte) cells that
       contain the MIR-Address-Space Addresses ("masa"s) for the remaining
       partitions in the MIR Address Space plus masks/counts needed for
       interpreting "compressed" (as compared with V1) Non-Terminal objects:

       masa     contents
         1      masa of the "Slice" Partition - (top of ISO Object ID Index)
         2      masa of the "Subregister" Partition - (part of Index)
         3      masa of the "Terminal Signed Numbers" Partition (compiled data)
         4      masa of the "Terminal Unsigned Numbers" Partition (comp. data)
         5      masa of the "Terminal Strings" Partition (compiled data)
         6      masa of the "Non-Terminal Data-Construct Objects" Partition
                     (derived from contents of "builtin_types.dat" file)
         7      masa of the "Non-Terminal Objects" Partition (bulk of
                     compiled data).
         8      masa of the "Non-Terminal MIR Relationship Objects" Partition
                     (objects defined by the compiler for use in compilation).

         9      Entry Count AND-Mask    The contents of this cell, when
                                        ANDed with the first cell in a V2
                Non-Terminal Object produces a value that can be taken as the
                unsigned count of the number of Entries in the relationship
                table of the Object.

        10      OID Count RSHIFT-Count  By taking the first cell of a V2
                                        Non-Terminal Object and Right-Shifting
                it by the number of times specified by the value specified
                here gives an unsigned count of the number of optional OID
                Backpointer cells that precede the relationship table in the
                Object.

        11      OID Backpointer MASA    The contents of this cell, when
                     AND-Mask           ANDed with an OID Backpointer cell
                                        in a V2 Non-Terminal Object produces a
                value that can be taken as the masa of an Index Slice that
                contains an entry for the right-most arc in the associated OID
                for the object.

        12      OID SMI RSHIFT-Count    By taking an OID Backpointer cell of a
                                        V2 Non-Terminal Object and
                Right-Shifting it by the number of times specified by the
                value specified here gives a value that can be taken as a
                value of the enumerated-type "mir_oid_smi", which indicates
                the SMI of the associated OID.

        13      Synonym AND-Mask        The contents of this cell, when
                                        ANDed with Synonym/Target Relationship
                Table Entry in a V2 Non-Terminal Object produces a
                value that can be taken as the "synonym" of a MIR Relationship
                object for this Relationship-table entry.

        14      Target RSHIFT-Count     By taking a Synonym/Target Relationship
                                        Table Entry in a V2 Non-Terminal Object
                and Right-Shifting it by the number of times specified by the
                value specified here gives a value that can be taken as a
                masa of the Target Object for this Relationship-table entry.

        15 -->  (The Slice Partition begins here and the top-most slice
                 in the index is first in the partition.  Therefore, MAS
                 cell with masa of "1" (in the Partition Table) always
                 has a "15" in it . . . in this binary output format version).

       Compiler #define symbols for these cell's addresses are defined in
       "mir.h" for use by the compiler and MIR Tier 0 functions.

       Note that cell with masa of 0 in the MIR Address Space is undefined and
       a masa of zero is considered illegal to reference when the MIR Address
       Space is loaded for reference by Tier 0.  We're one-origined.

Slice Partition - Contains MIR Objects that form the bulk of the OID Index.
       While the compiler regards these objects as bona fide "MIR Objects",
       the MIR Tier 0 lookup functions regard both this partition and the
       following Subregister Partition as a single intertwined "lump" which
       is traversed by a single subroutine.  These index MIR 'objects' cannot
       be 'fetched' by any Tier 0 function in the way that true compiled
       MIR objects (derived from an MSL file and stored in the Non-Terminal
       partition) can be found, fetched and manipulated.

Subregister Partition - Contains supporting MIR Objects that are needed for
       a complete OID Index.  Note the considerations mentioned above.

Terminal Signed Numbers Partition - All numbers (small enough to be
       represented as a signed number) derived from the input MSL files are
       recorded ONCE here in this partition, in ascending order.

Terminal Unsigned Numbers Partition - All numbers (too big to be represented
       as a signed number) derived from the input MSL files are recorded ONCE
       here in this partition, in ascending order.

Terminal Strings Partition - All strings derived from the input MSL files are
       recorded ONCE here in this partition.  They are in lexicographical
       order and preceded with a length word (4 bytes) and the strings are
       null terminated (null byte not counted in the length).  The strings are
       typically valid ASCII characters.

Non-Terminal Data-Construct Objects Partition - The contents of the compiler's
       "builtin_types.dat" file is recorded in this partition.  These are the
       objects the compiler uses to describe "datatypes" that are built-in
       to all the SMI's it can compile.  These MIR Objects are partitioned
       out separately from the general Non-Terminal Objects in order that this
       partition may be examined specially during a load by the compiler of
       a binary file in preparation for a compilation involving it.

Non-Terminal Objects Partition - The bulk of the compiled MSL information is
       recorded in this partition as MIR Objects.  These are the objects which
       can be specified by OID and then searched for relationship matches by
       MIR Tier 0 and Tier 1 functions.

Non-Terminal MIR Relationship Objects Partition - The MIR Objects needed by
       the compiler to represent "relationships" (used to tie together all the
       compiled MIR objects in the Non-Terminal Objects partition) are stored
       here.  These objects are structured identically to those in the
       Non-Terminal Objects partition, but stored separately to enable easy
       loading of an existing binary file by the compiler.

===============================================================================

In "C", the MAS is simply an array of integers, and a masa is a
valid index into that array, but it is quite useful to think of it more as
an address space that is partitioned much as a machine's might be.

MAS Differences Between Binary Format Version 1 and Version 2:

* In V2, the MAS does not start at a fixed offset from the beginning of the
  binary output file as it does in V1.

* V2 MAS has three more partitions than V1.
    - The Terminal Numbers partition is split into two, one signed and the
      other unsigned, for an increase of one partition.

    - V2 MAS separates MIR Objects for Relationships out of the general
      Non-Terminal Objects partition into their own partition, for an increase
      of one partition.

    - V2 MAS separates MIR Objects for DataConstructs out of the general
      Non-Terminal Objects partition into their own partition for an increase
      of one partition.

* All Non-Terminal Objects in all three Non-Terminal partitions 
  are structured differently in V2 than in V1 (for a decrease in storage
  requirements as well as permitting a MIR object to identify more than one OID
  associated with it in the OID index. . . V1 was restricted to one OID being
  identifiable as associated with a MIR object from the point-of-view of the
  object).

* V1 MAS has no conventions regarding the order in which Non-Terminal Objects
  are written into the Non-Terminal Object partition.  V2 MAS establishes
  several conventions that dictate exactly how all Non-Terminal partitions
  have objects laid out in them.  This permits the compiler to readily
  "load" an existing binary file for incorporation into a new compilation.
  The MIR Tier 0 functions requirements do not dictate any of these changes.

  The V2 conventions are:

    - The Non-Terminal Partitions are grouped together at the end of the MAS.
      (This allows for simpler macros in the MIR Tier 0 code that "detects"
       what kind of object a pointer (masa) points to.  Since they are used
       at run-time, it is important that they be simple and fast).

    - The order of Non-Terminal partitions is:
      - DataConstruct Non-Terminal Object Partition
      - General Non-Terminal Object partition
      - MIR Relationship Non-Terminal Object Partition

    - The first object in the Non-Terminal MIR Relationship Partition is
      the object for "MIR_Relationship_Name", and the order of appearance of
      these objects is exactly the same as the corresponding enumerated type
      definition "mir_relationship" in "mir.h" (for use by the compiler).
      (Note that this list may *ONLY* be expanded *at the end*, see "mir.h".)

    - The objects in the DataConstruct Non-Terminal Object Partition are
      in the order of their definition in the "builtin_types.dat" file.

    - The first object in the general Non-Terminal Object partition is the
      object for "The World".  (In V1, it turned out that this was the last
      object written to the Non-Terminal Object partition).

===============================================================================

In order to generate BINARY FORMAT 2, we need to know the "synonym" number for
each Non-Terminal that stands for a MIR Relationship Object.  The "synonym" is
simply the ordinal position number of the MIR Relationship Object inside the
Non-Terminal Relationship partition.  The best place to store a Relationship
Object's synonym during compilation is right in the IDS for that MIR
Relationship Object (of flavor I_NT_OBJECT_Rel).

Unfortunately, it is getting late, and I don't have time to reorganize the
definition of the Non-Terminal Relationship IDS data structure to admit of
placing the synonym ONLY in IDS of this particular type.  (It will bloat
up things in all Non-Terminals:  for a big MCC compile, there's 54000
Non-Terminals, times 4 bytes thats 1/4 Megabyte of nothingness.  There are
only 60 or so Non-Terminals that need synonyms.

It's in there for now, if the effort warrants it, perhaps things should be
re-arranged, either by doing a little redefinition of the IDS (NT* flavors)
or by recoding the code in this module to use a binary search on a linear
list created and maintained only for this final pass (that maps the heap
address for a NT Relationship object into its synonym).  This remains as
an exercise to the reader.

Lastly note that the value of "arc_count" in the Intermediate Context that
is written into the output file as the number of arcs in the longest OID in
the index may actually be 'high' if there were any "removes" (-r) done, as
the remove logic doesn't attempt to discover if it has deleted the longest
OID from the index (too costly).  This value can be regarded as a "high water
mark" in terms of the size of OIDs in the index, and is still guaranteed to
reflect at least enough storage to hold the longest OID in the index, with
perhaps some "waste" at the end.  No big deal.

*/

{
int     cell;      /* The cell address we're writing to (int array index) */
int     i;         /* General index                                       */
FILE    *ex_file;  /* File pointer for externalized output file.          */

/* In each of these arrays, the ith entry corresponds to the ith list of
|  IDS structures processed, as defined by the initialization at run-time 
|  of "top_of_list[]".
*/
IDS     *top_of_list[MAX_IDS_TYPE];         /*-> Top of each list          */
int     start_cell_address[MAX_IDS_TYPE];   /*-> Starting cell address for */
                                            /*   each externalized         */
                                            /*   list.                     */
char    *ids_names[MAX_IDS_TYPE];           /* For Info messages           */
int     ids_count[MAX_IDS_TYPE];            /* For Preamble: Counts of IDSs*/

IDS     *l_e;   /* "list-entry"-Set in turn to each entry of top_of_list[] */
int     *array; /* --> allocated storage for second pass (the MAS)         */
int     one=1;  /* Our "endian-ness" indicator, then one-word for I/O      */
int     zero=0; /* Our BOGUS "endian-ness" indicator                       */
int     cell_value;     /* General purpose cell we can load with things    */
                        /* that need writing out.                          */
time_t  pass_start;     /* Time that the pass begins                       */
time_t  ids_start;      /* Time that the pass begin for this IDS list      */
time_t  now;            /* The current time (whenever)                     */
char    i_elapsed[50];  /* Buffer for IDS elapsed time                     */
char    p_elapsed[50];  /* Buffer for Pass elapsed time                    */
int     max_nt_size=0;  /* Largest General NT we wrote out (in bytes)      */
int     max_str_size=0; /* Largest Terminal String we wrote out (in bytes) */
int     cell_save;      /* (Used in computing largest above)               */


/*
| Define an array into which we build the contents of the Preamble.
|
| V2 Preamble:
|
|       * 4 bytes - Major Compiler Version Number
|       * 4 bytes - Minor Compiler Version Number
|       * 4 bytes - Arc Count
|       * 4 bytes - MIR Address Space Size
|       * 32 bytes- 4 bytes EACH for the Count of Objects in each of
|                   8 Partitions
|       * 4 bytes - Maximum General NT size *in bytes*
|       * 4 bytes - Maximum Terminal String size *in bytes*
|         -----
|         56 Bytes, for a word count as defined below.
*/
#define PREAMBLE_WORD_COUNT 14
        /* NOTE:  "+1" below Leaves preamble[0] for preamble size */
int     preamble[PREAMBLE_WORD_COUNT+1];


/*
|  set-up synonyms in Non-Terminals representing MIR Relationships
|
|  NOTE: External Synonym values start at "1"... "0" is an illegal
|        external synonym value.
*/
for (l_e = icb->flavors[I_NT_OBJECT_Rel], i = 1;
     l_e != NULL;
     l_e = l_e->next, i++) {

    /* Store Synonym inside the object to which it applies */
    l_e->idsu.nt.ntu.synonym = i;
    }

/*
|  Set current write-cell address to just after end of Partition-Table
|  Partition.  We write the Partition Table separately from the writes that
|  create all the other things in the final MAS.
|
*/
cell = FIRST_MASA_AFTER_P_TABLE;

/*
|  Build an array of list-headers that we can swing down in a loop to
|  bind-or-write each IDS entry to the MAS.
|
|  It is these assignments that determine the ordering of the
|  partitions in the final output file.
|
| NOTE:
|       The order assigned here MATTERS, and might not match the order
|       in which IDS_TYPES were defined!
|
*/

ids_names[0] = "Index Slices.......";
ids_names[1] = "Index Subregisters.";
ids_names[2] = "Signed Numbers.....";
ids_names[3] = "Unsigned Numbers...";
ids_names[4] = "Strings............";
ids_names[5] = "NT: Data-Construct.";
ids_names[6] = "NT: General........";
ids_names[7] = "NT: MIR Rel........";

top_of_list[0] = icb->flavors[I_SLICE];
top_of_list[1] = icb->flavors[I_SUBREGISTER];
top_of_list[2] = icb->flavors[I_T_OBJ_snumber];
top_of_list[3] = icb->flavors[I_T_OBJ_unumber];         
top_of_list[4] = icb->flavors[I_T_OBJ_string];          
top_of_list[5] = icb->flavors[I_NT_OBJECT_DC];
top_of_list[6] = icb->flavors[I_NT_OBJECT];
top_of_list[7] = icb->flavors[I_NT_OBJECT_Rel];

/* (...For a total of 8 partitions beyond the Partition-Table Partition) */

if (supplementary_time == TRUE) {
    time(&pass_start);
    fprintf(stderr, "mirc - Info: Beginning Pass 1 Binary Generation\n");
    }

/* --------------------FIRST PASS-------------------- */
/* --------------------FIRST PASS-------------------- */
/* --------------------FIRST PASS-------------------- */
/* for (each kind of IDS type) */
for (i=0; i < MAX_IDS_TYPE; i++) {

    /*
    | record cell address of start of this-type.  This, so we can write this
    | array into the MAS as the Partition-Table Partition during the second
    | pass.
    */
    start_cell_address[i] = cell;

    if (supplementary_time == TRUE) {
        time(&ids_start);
        }

    /* while (list pointer of this-type is non-null) */
    for (l_e = top_of_list[i];  l_e != NULL; l_e=l_e->next) {

        /* Record starting point for use in finding largest */
        cell_save = cell;

        /*
        | bind the entry on this list to external address (masa)
        | via e_conv_ids_type() call.  The "NULL" indicates that we're
        | doing address binding, (the second NULL is for the counter, used
        | only during the second pass).
        */
        e_conv_ids_type(l_e, &cell, NULL, NULL);

        /* Discover largest General NT and Terminal String */
        if (l_e->flavor == I_NT_OBJECT) {
            if (((cell-cell_save)*sizeof(int)) > max_nt_size) {
                /* Record a larger NT */
                max_nt_size = ((cell-cell_save)*sizeof(int));
                }
            }
        else if (l_e->flavor == I_T_OBJ_string) {
            if (((cell-cell_save)*sizeof(int)) > max_str_size) {
                /* Record a larger String*/
                max_str_size = ((cell-cell_save)*sizeof(int));
                }
            }

        /* step to the next pointer in current entry */
        }

    if (supplementary_time == TRUE) {
        time(&now);
        mirf_elapsed((now-ids_start), i_elapsed);
        mirf_elapsed((now-pass_start), p_elapsed);
        fprintf(stderr, "%s Elapsed, IDS: %s  Pass 1: %s\n",
                ids_names[i], i_elapsed, p_elapsed);
        }

    }

/* allocate enough storage from the heap to hold it all (whew!) */
if ( (array = (int *)malloc(sizeof(array[0])*cell))==NULL) {
    fprintf (stderr,
             MP(mp600,"mirc - Internal Error: Malloc failure, entry count = %ld.\n"),cell);
    return (FALSE);
    }

/* write the size in bytes (of the MAS: which starts at "1") into cell zero
|
| At this point, "cell" points at the next cell we'd be binding, so it is
| the address of the next "free" cell.  Knock off 1 cell to get the masa of
| the last used cell, which is the number of cells used, since cell addresses
| (masas) start at 1.
*/
array[0] = (cell-1)*sizeof(array[0]);

/* write the partition table into the array */
for (i = 0; i < MAX_IDS_TYPE ; i++) {
    array[i+1] = start_cell_address[i];
    }

/*
| Load the Masks and Counts needed to interpret the packed cells in
| Non-Terminals into the tail end of the Partition-Table Partition.
*/
array[ENTRY_COUNT_AMASK_MASA] = ENTRY_COUNT_AMASK;
array[OID_COUNT_RSHIFT_MASA] = OID_COUNT_RSHIFT;
array[OID_BPTR_AMASK_MASA] = OID_BPTR_AMASK;
array[OID_SMI_RSHIFT_MASA] = OID_SMI_RSHIFT;
array[SYNONYM_AMASK_MASA] = SYNONYM_AMASK;
array[TARGET_RSHIFT_MASA] = TARGET_RSHIFT;


/* reset the write-cell address */
cell = FIRST_MASA_AFTER_P_TABLE;


if (supplementary_time == TRUE) {
    time(&pass_start);
    fprintf(stderr, "mirc - Info: Beginning Pass 2 Binary Generation\n");
    }

/* --------------------SECOND PASS-------------------- */
/* --------------------SECOND PASS-------------------- */
/* --------------------SECOND PASS-------------------- */
/* for (each kind of IDS type) */
for (i=0; i < MAX_IDS_TYPE; i++) {

    if (supplementary_time == TRUE) {
        time(&ids_start);
        }

    /* Initialize for incrementation during this pass */
    ids_count[i] = 0;

    /* while (list pointer of this-type is non-null) */
    for (l_e = top_of_list[i];  l_e != NULL; l_e=l_e->next) {
        /*
        | copy the entry on this list to the MAS at it's bound MAS address
        | via e_conv_ids_type() call.  The "array" indicates that we're
        | doing real writes.
        */
        e_conv_ids_type(l_e, &cell, array, &ids_count[i]);

        /* step to the next pointer in current entry */
        }

    if (supplementary_time == TRUE) {
        time(&now);
        mirf_elapsed((now-ids_start), i_elapsed);
        mirf_elapsed((now-pass_start), p_elapsed);
        fprintf(stderr, "%s Elapsed, IDS: %s  Pass 2: %s  Count:%d  %d/sec\n",
                ids_names[i],
                i_elapsed,
                p_elapsed,
                ids_count[i],
                (((now-ids_start) == 0) ? 0 : ids_count[i]/(now-ids_start)));
        }

    }

/* if (attempt to open the specified file FAILED) */
if ((ex_file = fopen(fname,"wb")) == NULL) {
    fprintf (stderr,
             MP(mp601,"mirc - Internal Error: Attempt to open file %s failed.\n"),
             fname);
    return (FALSE);
    }

/*
| If there was a severe error, we can only get here if the user specified
| a "-X" switch to force output.  We mark such output with a bogus
| endian-ness indicator in this case
|
| On "OK" output: write a 4-byte "endian-ness" indicator "1" to output file
*/
if (eflag == FALSE) {
    if (fwrite(&one, sizeof(one), 1, ex_file) != 1) {
        fprintf (stderr,
                 MP(mp602,"mirc - Internal Error: fwrite of endian-ness indicator to '%s' failed.\n"),
                 fname);
        return (FALSE);
        }
    }
else {  /* Write a BOGUS zero */
    if (fwrite(&zero, sizeof(zero), 1, ex_file) != 1) {
        fprintf (stderr,
                 MP(mp602,"mirc - Internal Error: fwrite of endian-ness indicator to '%s' failed.\n"),
                 fname);
        return (FALSE);
        }
    }

/* write the 4-byte binary output-file format version number */
cell_value = BINARY_FORMAT;
if (fwrite(&cell_value, sizeof(cell_value), 1, ex_file) != 1) {
    fprintf (stderr,
             MP(mp603,"mirc - Internal Error: fwrite of binary format indicator to '%s' failed.\n"),
             fname);
    return (FALSE);
    }

/* 
| Build the V2 Preamble
*/
preamble[0] = PREAMBLE_WORD_COUNT*sizeof(preamble[0]);  /* size in *BYTES* */
preamble[1] = VERSION_MAJOR;
preamble[2] = VERSION_MINOR;
preamble[3] = icb->arc_count;   /* Longest OID in arcs               */
preamble[4] = array[0];         /* Size of MAS in *BYTES*            */
preamble[5] = ids_count[0];     /* Count of Index Slices             */
preamble[6] = ids_count[1];     /* Count of Index Subregisters       */
preamble[7] = ids_count[2];     /* Count of Signed Numbers           */
preamble[8] = ids_count[3];     /* Count of Unsigned Numbers         */
preamble[9] = ids_count[4];     /* Count of Strings                  */
preamble[10]= ids_count[5];     /* Count of NTs for Dataconstructs   */
preamble[11]= ids_count[6];     /* Count of General NTS              */
preamble[12]= ids_count[7];     /* Count of NTs for MIR Relationships*/
preamble[13]= max_nt_size;      /* Largest General NT size in bytes  */
preamble[14]= max_str_size;     /* Largest Terminal String in bytes  */

/* 
| Write the SIZE of the V2 Preamble and the PREAMBLE as a lump
*/
if (fwrite(preamble, (PREAMBLE_WORD_COUNT+1)*sizeof(int), 1, ex_file) != 1) {
    fprintf (stderr,
             MP(mp604,"mirc - Internal Error: fwrite of output PREAMBLE to '%s'failed.\n"),
             fname);
    return (FALSE);
    }

/*
| NOTE: MAS starts at array[1], and array[0] contains MAS size in *BYTES*
|
| if (write the rest of the array (the MAS) to the file FAILED)
*/
if (fwrite(&array[1], array[0], 1, ex_file) != 1) {
    fprintf (stderr,
             MP(mp605,"mirc - Internal Error: fwrite of MAS to '%s' failed.\n"),
             fname);
    return (FALSE);
    }

/* if (attempt to close the file failed) */
if (fclose(ex_file) != 0) {
    /* print error message */
    fprintf (stderr,
             MP(mp606,"mirc - Internal Error: fclose of file '%s'failed.\n"),
             fname);
    return (FALSE);
    }

return (TRUE);
}

/* e_conv_ids_type - Convert Intermediate IDS to External Format */
/* e_conv_ids_type - Convert Intermediate IDS to External Format */
/* e_conv_ids_type - Convert Intermediate IDS to External Format */

static void e_conv_ids_type (nextIDS, cell_add, array, count)

IDS         *nextIDS;   /* -> Next IDS structure to convert               */
masa        *cell_add;  /* -> integer containing cell address to start at */
int         array[];    /* -> array of cells (ints) where data is written */
int         *count;     /* -> counter for these kinds of IDS              */


/*
INPUTS:

    "next" points to the IDS structure to be converted and copied to the
    output.

    "cell" is the index into array "array" (or, alternatively it is viewed as
    the "MIR Address-Space Address") where the writing of this converted
    structure is to be done IF "array" is non-null.

    "count" is incremented if the IDS is actually processed.


OUTPUTS:

    The function returns the value of "cell" updated to point to the next
    free cell following the last used to hold the converted structure
    (regardless of whether it was actually converted and copied).

    If "array" is NULL on entry, then
    this function merely computes the amount of storage required for that
    structure.  Additionally it stores into the original IDS structure the
    cell address (external address) at which this structure will be stored
    when it is finally copied into the output array.  (IDS with a zero
    reference count are processed by skipping them).

    If the value of "array" was non-null on entry, then
    the routine actually writes the converted structure into this array
    starting at the input value of "cell" AFTER checking to be sure that
    the value of "cell" is in fact the same value that is stored in the
    IDS structure as the external address for the structure.

BIRD'S EYE VIEW:
    Context:
        The caller has completed the construction of the Intermediate
        representation of the MIR data in the heap.  By passing TWICE
        down the lists of the various intermediate data structures, we
        compute (FIRST pass) the addresses to which all the structures are
        bound and the total size required and on the SECOND pass the
        structures are actually copied.

    Purpose:
        This function computes the size and starting external address for each
        Intermediate Data Structure passed it, and optionally copies the
        converted structure to the output array.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (the IDS is a general Non-Terminal or string Terminal)
        if (reference count is zero)
            <return>
    else if (the IDS is a non-string Terminal)
        if (reference count is zero)
            <error "Terminal With non-zero reference count">

    if ("array" is NULL [1st pass])
        <perform sanity check on string ordering>

    if ("array" is NULL [1st pass])
        if (structure's externalized address is 0)
            <store input "cell" value as structure's externalized address>
        else
            <error "Externalized address not zero: <value>">
            <return>
    else
        <count one-of-these processed>
        if (structure's externalized address is not EQUAL TO "cell")
            <error "Phase Error for structure">
            <return>

    switch (flavor of structure)

        case I_SLICE:
            <process backpointer, entry count and entries>
            break;

        case I_SUBREGISTER:
            <process backpointer, cell addr of thing & lower slice>
            break:

        case I_NT_OBJECT_DC:
        case I_NT_OBJECT:
        case I_NT_OBJECT_rel:
            <compute number of OID backpointers needed>
            <create the packed initial count cell value>
            <write the cell value>

            if (array is NON-NULL (ie, we're for real))
                while (OID backpointers remain to be written)
                    <find the next (from the top) unwritten backpointer
                        entry: "candidate">
                    for (the rest of the list)
                        if (backpointer match w/candidate)
                            if (arc number is < candidate's arc number)
                                <make just found entry be new candidate>
                    <create the packed cell value of
                        OID backpointer + SMI indicator>
                    <write the packed cell value>
                    <mark the candidate as "written" by zeroing backpointer>
                    <count a backpointer written>
            else
                <add count of OID backpointers needed to "cell">

            if (array is NON-NULL (ie, we're for real))
                for (each relationship table entry)
                    <compute compressed synonym/target value>
                    <write the value to MAS>
            else
                <add count of Relationship table entries to "cell">

            break;

        case I_T_OBJ_snumber:
            <store signed number>
            break;

        case I_T_OBJ_unumber:
            <store unsigned number>
            break;

        case I_T_OBJ_string:
            <store size of string>
            <store string itself w/null byte>
            break;

    <return the value of "cell" to the caller>

OTHER THINGS TO KNOW:

    This function does "triple-duty" in the two pass scheme we use to
convert and bind each different data structure into the externalized form.
If the IDS is not referenced, this function "weeds it out" by not
allocating space for them during the first pass.  During the first pass
all other referenced IDS structures are bound to an externalized address
(a "masa").  During the second pass, the contents of the IDS is copied
into "the MIR address space" .. "MAS".

                           >>> START OF *NOTE* <<<<<
    This function does not do a perfect job of trimming the output of
    unreferenced MIR objects.  It will only clip the top unreferenced
    MIR object in what might be a branch of unreferenced objects. 

    The only objects that *should* be unreferenced (and clipped) are
    Non-Terminal objects representing the contents of an MSL "TYPE" statement
    like:

        TYPE SpecialInteger = 4 : Integer;

    If "SpecialInteger" is not referenced in the MSL in which it appears,
    then it's (General) Non-Terminal will have a reference count of 0 and
    the check at the beginning of this function will cause it to be skipped
    and not written to the output file.

    However, should *two* or more TYPE statements appear:

        TYPE SpecialInteger = 4 : Integer;
        TYPE VerySpecialInteger = 5 : SpecialInteger

    where neither is referenced elsewhere in the MSL file,
    then only the Non-Terminal representing "VerySpecialInteger" will be
    skipped by the logic in this function.  This is because "SpecialInteger"
    will have a non-zero reference count (it is referenced by
    "VerySpecialInteger").  In order to fully trim everything of this nature,
    logic that implemented multiple passes through the list of General
    Non-Terminals would have to be implemented:  Keep running down the list
    "reclaiming" IDS's that have a zero as a reference count until a pass
    occurs in which none are reclaimed.  For large compilations, *this is
    very expensive* and we don't do it because the returns aren't worth the
    time spent.
    
    The "ref_count" field in the IDS has varying importance depending on the
    flavor of the IDS:

    I_SLICE, I_SUBREGISTER - A reference count is entirely superfluous for
    these flavors, as we can correctly infer when to delete it by examining
    it while walking the index (and consequently there is no reference count
    field for these flavors).

    I_T_OBJ_snumber, _unumber - For these Terminals, the final pass may
    encounter one with a zero reference count if a binary MIR database file was
    supplied as input.  In this instance if a Non-Terminal representing a TYPE
    statement was clipped during the production of that binary MIR database
    file originally, then when we go to write out the product of the current
    compilation, any unique numbers referenced by the clipped TYPE statement
    Non-Terminal will have a zero reference count.  Most numbers will be
    reclaimed automatically when their reference count goes to zero (by
    function "mir_remove_OBJECT()" in "mir_remove.c").

    I_T_OBJ_string - For these string Terminals, the final pass may legally
    encounter string IDSs with zero reference counts either following the
    scenario described above for numbers or for the Seed Strings created
    (by mirc_init_backend() in "mir_backend.c") when no binary MIR database
    file is loaded at the start of a compilation.  See discussion in file
    "mir_intermediate.c" for function "I_Create_IDS_STRING()".

    I_NT_OBJECT_DC - While a reference count could be accurately maintained,
    we never delete even an unreferenced instance of one of these Non-Terminals
    because it represents information extracted from the "builtin_types.dat"
    file for the compiler, and so must always be present in any MIR binary
    database file.    The reference count is forced to 1 at creation time.

    I_NT_OBJECT_Rel - Similarly, we never delete an unreferenced MIR
    Relationship Non-Terminal, as all must always be present in any MIR binary
    database file.  The reference count is forced to 1 at creation time.

    I_NT_OBJECT - A reference count is maintained for these Non-Terminals.
    Typically, the only times the reference count is used include:

     * In this function, as described above to attempt to weed out unreferenced
       Non-Terminals representing unused TYPE statements.
     * Similarly in "mir_remove.c", the reference count is used to determine
       whena Non-Terminal describing a TYPE statement should be reclaimed.

    
Each different flavor of Intermediate Data Structure is copied, and
any "heap pointers" within the structure (flavors I_SLICE, I_SUBREGISTER and
I_NT_OBJECT_* only) are converted to externalized addresses (on the second
pass).

Each IDS structure is mapped as follows (where one line describes the contents
of one MAS (4-byte) cell:

TYPE            CELL CONTENTS (4 bytes)
-------------------------------------------------
I_SLICE         <backpointer cell address>
                <entry count>
                            .
                            .
                <arc number>                    |
                <cell addr of thing>            |repeats for each entry



I_SUBREGISTER   <backpointer cell address>
                            .
                            .
                <cell addr of thing>
                <cell addr of next lower slice>


I_T_OBJ_string  <size of null terminated string in bytes (excluding null)>
                <...string value starts in next cell and uses as many as
                 needed and includes a null byte>


--------------------------------
For Binary File Format Version 1:
I_T_OBJ_number  <number value>

--------------------------------
For Binary File Format Version 2:
I_T_OBJ_snumber <signed number value>

I_T_OBJ_snumber <unsigned number value>
--------------------------------


--------------------------------
For Binary File Format Version 1:
I_NT_OBJECT     <backpointer cell address>
                <entry count>
                            .
                            .
                <cell addr of relationship>     |
                <cell addr of target>           |repeats for each entry


--------------------------------
For Binary File Format Version 2:

(NOTE: This documentation extracted to "mir.h" for definition of masks
 and counts!  Change it here: you gotta copy it over.)

I_NT_OBJECT_*   <OID-backpointer count/Entry count>    --See Note 1
                <1st optional OID Backpointer>         --See Note 2
                            .
                            .
                <last optional OID Backpointer>        --See Note 2
                            .
                            .
                <Synonym/Target Rel. Table Entryr>|    --See Note 3
                                                  |_repeats for each
                                                     relationship table entry

1) This 4-byte cell contains two data:

    - Entry Count: The number of entries in the relationship table
    - OID Backpointer Count: The number of optional OID Backpointers
      that precede the relationship table.

    This cell is "packed" in that it contains more than one datum.
    - To obtain the Entry Count, apply the "Entry Count AND-Mask"
      (obtained from the Partition-Table Partition) in an "AND" operation
      to this cell.  This produces a 4-byte unsigned count.
    - To obtain the OID-backpointer count, apply the "OID Count RSHIFT-Count"
      (obtained from the Partition-Table Partition) as a Right-Shift operation
      to this cell.  This produces a 4-byte unsigned count.

    You can infer that the "packing order" of these fields is:
              Hi                                Lo
                [<OID bptr Count> <Entry Count>]

2) This optional 4-byte cell contains two data.  The number of these cells
   that appear in any given V2 Non-Terminal Object is specified by the value
   of the OID-backpointer count (as extracted from the cell described in 1)
   above.

    This cell is "packed" in that it contains more than one datum.
    - To obtain the MASA of the Index Slice that describes the OID, apply
      the "OID Backpointer MASA AND-Mask" (obtained from the Partition-Table
      Partition) in an AND operation to this cell.
    - To obtain the SMI indicator for the OID, apply the "OID SMI RSHIFT-Count"
      (obtained from the Partition-Table Partition) in a Right-Shift
      operation on this cell.  The result is a 4-byte value that can be
      taken as a valid enumerated-type "mir_oid_smi" value.

    You can infer that the "packing order" of these fields is:
              Hi                                              Lo
                [<OID SMI Indicator> <OID Backpointer MASA>]

3) This 4-byte cell represents an entry in the Non-Terminal Object's
   Relationship Table.  It contains a "synonym" for a MIR Relationship Object
   and a (left-zero truncated) masa of the target of the relationship for this
   entry.  The count of the number of these entries is extracted as "Entry
   Count" in 1) above.

   The synonym for a MIR Relationship is nothing more than it's ordinal
   position in the MIR Relationship Non-Terminal Object Partition.  The MIR
   Tier 0 functions must scan the MIR Relationship Non-Terminal Object
   Partition to establish the mapping from "synonym" to MAS address for each
   MIR Relationship stored in that partition.

   The truncated masa is just that: truncated on the "left".  Non-significant
   left zero bits are dropped when the masa is packed into this cell.

   If 8 bits is used to represent a synonym, this gives 255 possible MIR
   Relationship objects (V2.0 of the compiler uses about 60).  The remaining 
   24 bits are available to represent the target MASA, (always a 4-byte word
   address) giving an available address space of 64 MB.

    - To obtain the synonym for the MIR relationship object for this
      relationship entry, apply the "Synonym AND-Mask" (taken from the
      Partition-Table Partition) in an AND operation on this cell.  The result
      can be taken as a 4-byte value that represents the "synonym" for the
      MIR Relationship.
      
    - To obtain the MASA of the target for this relationship entry,
      apply the "Target RSHIFT-Count" (taken from the Partition-Table
      Partition) in a Right-Shift operation to this cell.  This results in
      a value that can be taken as a 4-byte masa of the target MIR object.

    You can infer that the "packing order" of these fields is:
              Hi                         Lo
                [<Target MASA> <Synonym>]

>>> By storing compiler-generated AND-masks and counts into the MAS, the
>>> packing arrangement can be controlled by changing the compiler without
>>> affecting MIR Tier 0 functions, nor the Binary Format Version number.
    (That's why it's done this way).


The packing schemes detailed in 1) thru 3) result in OVER a 50% reduction
in the size of all the Non-Terminal Partitions.  Since Non-Terminals take
60% to 75% of MAS under Binary Format Version 1, this results in a drastic
reduction in binary output file size with a cost of a few AND operations
when doing searches of the Relationship Tables.  A non-trivial enhancement.

Note as well that the MIR Tier 0 functions need only compute the synonym
for a MIR Relationship Object ONCE, and store it in the mandle for that
object when it is initially returned to the caller.  Thereafter the MIR Tier 0
function (that actually performs the search of a relationship table) really
just looks for a "match" in the table using the synonym value obtained from
the mandle it receives.  It never needs to "back-translate" a synonym into
the masa for a MIR Relationship object.  A side effect of this scheme is
that the MIR Tier 0 search function can immediately tell when it has been
passed a mandle for an object that the caller thinks is a MIR Relationship
object when it is not:  there will be no synonym stored in the mandle.

--------------------------------

Note that when processing Non-Terminals, the "backpointers" that point back
to the Index Slices (thereby indicating one or more OIDs assigned to this
object) must be recorded in the Non-Terminal in a special way according to
the convention described below.

Here's why.  In the original design of this subsystem, an object could only
have one OID assigned to it.  This OID was indicated in the Non-Terminal
by placing a "back-pointer" inside the Non-Terminal that pointed to a slice
somewhere in the Index partition (or a subregister, but it's the same thing).
The Tier 0 function that extracts the OID (given the mandle for a Non-Terminal)
discovers it's path back through the index (and thereby the OID) by
scanning the Slice specified by the backpointer.  Somewhere in that Slice
should be an entry with a "forward pointer" that points to the Non-Terminal.
That entry contains the right-most arc of the OID for the Non-Terminal object.
By recursively "climbing" the Index back pointers, it can elucidate the entire
OID.

With the advent of the ability to assign multiple OIDs to an object the
possibility arises that an object may have EXACTLY the same OID *EXCEPT* for
the LAST ARC!  This introduces an ambiguity in the algorithm described
above, as there will be two entries in the same Index Slice that point
directly at such a Non-Terminal: the algorithm doesn't know which is the
correct arc.

We tackle and solve this problem at the expense of no extra space in the
MAS by establishing the following convention:

    * In the event an object is registered "ambiguously" as described above,
      then the order of the ambiguous backpointers in the Non-Terminal
      MATCHES the order of appearance of their right-most arcs in the
      Index Slice (which is ordered in ascending arc value).

In other words, if the object has two OIDs:

              OID_MCC - ending in ".5"
              OID_DNA - ending in ".3"

then the compiler writes the OID backpointer for OID_DNA into the Non-Terminal
FIRST, because it's right-most arc is less than that of OID_MCC, and therefore
appears first in the Index Slice.

This comes at the expense of run-time execution logic in MIR Tier 0 to
check to see if ambiguity applies and scan the Index accordingly.

The loops in this function that discovers the right OID "candidate" to write
next is implementing this convention within the compiler.

If I can get the architectural OK, we'll simply drop the convention and
have the compiler enforce the rule that this must never be allowed to happen,
since it seems it should NEVER happen that OIDs from different SMI's match
EXCEPT for the last arc.

*/      

/* "Write Structure's Externalized Address"
|
|   This macro, when provided the address of an Intermediate Data Structure,
|   will write (IF "array" is non-null) the structures' Externalized address
|   into the next cell location at "cell".  "cell" is always updated by 1.
*/
#define WSEA(idsptr)   \
{if (array != NULL) {array[cell] = (idsptr)->ex_add;} cell += 1;}


/* "Write Value"
|   This macro, when provided a value,
|   will write (IF "array" is non-null) the value.
|   into the next cell location at "cell".  "cell" is always updated by 1.
*/
#define WVAL(value)    \
{if (array != NULL) {array[cell] = (int) value;} cell += 1;}

{
int             cell;           /* Local cell_address                       */
int             oidbptr_count;  /* OID Back-pointer count                   */
int             i,j;            /* general indices                          */
int             slen;           /* Length of a terminal string              */
struct SES      *sl;            /* Slice List (as extracted from Slice IDS) */
struct RES      *rt;            /* Relationship Table                       */

static char *last_string = NULL;  /* Used in string-ordering sanity check   */


/* if (the IDS is a general Non-Terminal or a Terminal) */
if ((nextIDS->flavor == I_NT_OBJECT && nextIDS->idsu.nt.ref_count == 0)
    || ( (   nextIDS->flavor == I_T_OBJ_string
          || nextIDS->flavor == I_T_OBJ_snumber
          || nextIDS->flavor == I_T_OBJ_unumber)
        && nextIDS->idsu.t.ref_count == 0)
    ) {
    return;     /* Return to ignore this non-referenced MIR Object */
    }

/* Perform string sanity check on last pass to insure correct order */
if (array != NULL) 
    if (nextIDS->flavor == I_T_OBJ_string) {
        if (last_string != NULL) {
            if (strcmp(last_string, nextIDS->idsu.t.t_v.string.str) >= 0) {
                printf(MP(mp607,"mirc - Internal Error - Out of order: \"%s\" and \"%s\"\n"),
                       last_string, nextIDS->idsu.t.t_v.string.str);
                }
            }
        last_string = nextIDS->idsu.t.t_v.string.str;
        }

cell = *cell_add;   /* Local copy for private mods before return */

/* if ("array" is NULL) */
if (array == NULL) {

    /* if (structure's externalized address is 0) */
    if (nextIDS->ex_add != 0) {
        /* error "Externalized address not zero: <value>" */
        fprintf (stderr,
                 MP(mp608,"mirc - Internal Error: Externalized address not zero: %ld.\n"),
                 nextIDS->ex_add);
        }

    /* store input "cell" value as structure's externalized address */
    nextIDS->ex_add = cell;

    }
else {
    /* if (structure's externalized address is not EQUAL TO "cell") */
    if (nextIDS->ex_add != cell) {
        /* error "Phase Error for structure" */
        fprintf (stderr,
                 MP(mp609,"mirc - Internal Error: Phase Error, ex_add=%ld, cell=%ld.\n"),
                 nextIDS->ex_add,
                 cell);
        return;
        }

    /* count one-of-these processed */
    *count += 1;
    }

switch (nextIDS->flavor) {     /* flavor of structure */

    case I_SLICE:
        /* process backpointer, entry count and entries */
        if (nextIDS->idsu.i.backptr != NULL) {
            WSEA(nextIDS->idsu.i.backptr);
                }
        else {
            WVAL(0);
            }
        WVAL(nextIDS->idsu.i.entry_count);
        sl = nextIDS->idsu.i.slice;
        for (i=0; i < nextIDS->idsu.i.entry_count; i++) {
            WVAL(sl[i].iso_arc);
            WSEA(sl[i].next);
            }
        break;

    case I_SUBREGISTER:
        /* process backpointer, cell addr of thing & lower slice */
        WSEA(nextIDS->idsu.s.backptr);
        WSEA(nextIDS->idsu.s.ntobject);
        WSEA(nextIDS->idsu.s.lower_levels);
        break;

    case I_NT_OBJECT_DC:
    case I_NT_OBJECT:
    case I_NT_OBJECT_Rel:
        /* compute number of OID backpointers needed */
        for (i=0, oidbptr_count=0; i < MAX_OID_SMI;  i++) {
            if (nextIDS->idsu.nt.oid_backptr[i] != NULL) {
                oidbptr_count += 1;
                }
            }

        /* create the packed initial count cell value */
        /* write the cell value */
        if (array == NULL) {
            cell += 1;    /* Don't bother with the overhead on bind pass */
            }
        else {
            WVAL(
                 (    (oidbptr_count << OID_COUNT_RSHIFT)  /* Hi-order */
                    |  nextIDS->idsu.nt.entry_count        /* Lo-order */
                 )
                );
            }

        /* if (array is NON-NULL (ie, we're for real)) */
        if (array != NULL) {

            /* while (OID backpointers remain to be written) */
            while (oidbptr_count > 0) {

                /*
                |  find the next (from the top) unwritten backpointer
                |  entry: "candidate"
                */
                for (i=0;  i < MAX_OID_SMI;  i++) {
                    if (nextIDS->idsu.nt.oid_backptr[i] != NULL)
                        break;
                    }

                if ( i >= MAX_OID_SMI)  /* Preclude disasters */
                    break;

                /*
                | "i" points at the current 'candidate'.  Here we are
                | attempting to ensure disambiguation of OIDs that potentially
                | match except for the right most arc.
                */

                /* for (the rest of the list) */
                for (j=i+1; j < MAX_OID_SMI; j++) {

                    /* if (backpointer match w/candidate) */
                    if (nextIDS->idsu.nt.oid_backptr[j] ==
                        nextIDS->idsu.nt.oid_backptr[i] )

                        /* if (arc number is < candidate's arc number) */
                        if (nextIDS->idsu.nt.oid_backarc[j] <
                            nextIDS->idsu.nt.oid_backarc[i] )

                            /* make just-found entry be new 'candidate' */
                            i = j;
                    }
                /*
                |  As we fall out of this loop, "i" indicates the OID entry
                |  to be converted into a 'packed' backpointer + SMI indicator.
                |  The value of "i" *IS* the value of the SMI Indicator.
                */

                /*
                |  Create the packed cell value of
                |  OID backpointer + SMI indicator
                */
                /* write the packed cell value */
                WVAL(
                     (    /* Hi-order */
                      (i << OID_SMI_RSHIFT)
                      |   /* Lo-order */
                      (nextIDS->idsu.nt.oid_backptr[i]->ex_add)
                     )
                    );

                /* mark the candidate as "written" by zeroing backpointer */
                nextIDS->idsu.nt.oid_backptr[i] = NULL;

                /* count a backpointer written */
                oidbptr_count -= 1;                    
                }
            }
        else {
            /* add count of OID backpointers needed to "cell" */
            cell += oidbptr_count;
            }

        /* if (array is NON-NULL (ie, we're for real)) */
        if (array != NULL) {

            /* for (each relationship table entry) */
            for (i=0; i < nextIDS->idsu.nt.entry_count; i++) {

                /*
                | - compute compressed synonym/target value 
                | - write the value to MAS
                */
                rt = nextIDS->idsu.nt.rel_table;
                j = rt[i].rel_obj->idsu.nt.ntu.synonym;      /* Get synonym */
                WVAL(
                     (  /* Hi-order Target MAS (external) Address*/
                      (rt[i].tar_obj->ex_add << TARGET_RSHIFT)
                      |
                      (j)     /* Lo-order Synonym */
                     )
                    );
                }
            }
        else {
            /* add count of Relationship table entries to "cell" */
            cell += nextIDS->idsu.nt.entry_count;
            }
        break;

    case I_T_OBJ_snumber:
        /* store signed number */
        WVAL(nextIDS->idsu.t.t_v.snumber.value);
        break;

    case I_T_OBJ_unumber:
        /* store unsigned number */
        WVAL(nextIDS->idsu.t.t_v.unumber.value);
        break;

    case I_T_OBJ_string:
        slen = nextIDS->idsu.t.t_v.string.len;
        /* store size of string */
        WVAL(slen);

        /* store string itself w/null byte */
        if (array != NULL) {

            /* No garbage in last cell */
            array[cell + ((slen+1)/sizeof(array[0]))] = 0;

            bcopy (nextIDS->idsu.t.t_v.string.str, &array[cell], (slen+1));
            }

        cell += (slen+1)/sizeof(array[0]) + /* 1 more if not evenly aligned */
                (( ( (slen+1) % sizeof(array[0])) == 0) ? 0 : 1);
        break;
    }

/* return the value of "cell" to the caller */
*cell_add = cell;
}
