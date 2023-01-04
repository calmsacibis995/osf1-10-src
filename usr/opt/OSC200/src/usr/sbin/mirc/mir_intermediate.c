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
static char *rcsid = "@(#)$RCSfile: mir_intermediate.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:00:03 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990-1992, 1993.
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
 * Module MIR_INTERMEDIATE.C
 *      Contains Intermediate-representation management functions required by
 *      the MIR to manage it's Intermediate representation of the MIR information.
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
 *       to manage the "index" of information required to locate a piece of
 *       information in the MIR by ISO Object ID.  The index is the directory
 *       within the MIR and these functions manage the creation of the
 *       Intermediate form of the index.  Other functions in this module
 *       permit the creation of the "Internal Data Structures" (IDS) needed
 *       to represent Terminal and Non-Terminal Objects in the MIR.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *   Version    Date            Who             What
 *      V0.0    December 1990   D. D. Burns     Original code
 *
 *      V0.1    January 1991    D. D. Burns     Modified to support the
 *                                               compiler rather than the
 *                                               "bridge" code used to get us
 *                                               off the ground.
 *
 *      V1.90   March 1992      D. D. Burns     Modified to support new
 *                                               compiler binary output file
 *                                                format: multiple OID lookup
 *
 *      V1.95   Sept 1992       D. D. Burns     Interim version of this module:
 *                                               I_Create_IDS() accepts an
 *                                               argument indicating size of
 *                                               table to create.
 *
 *      V1.98   Sep 1992        D. D. Burns     Make "I_Find_OID()" a globally
 *                                               accessible function so that
 *                                               the code in "mir_remove.c"
 *                                               can use it to look up the OID
 *                                               for an Entity-class to be
 *                                               deleted, add function
 *                                               "I_Reclaim_IDS()" and modify
 *                                               other 'create' IDS functions
 *                                               to use reclaimed-free lists
 *                                               created by I_Reclaim_IDS();
 *                                               internationalize
 */
/*
Module Overview:

This module contains all the functions used by the MSL Compiler to create the
"Intermediate" representation of the MIR data.

All these functions' names begin with the letter "I", and those that return
status codes do so using values from the MIR Compiler Status Code list
defined in "mir_compiler.h".  These codes begin with the letters "MC_".


MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
I_Register_Context   Registers an Intermediate Context Block with this module
                     so that the code in this module has a "context" within
                     which to work.

I_Register_OID       Registers a new Object ID in Intermediate MIR Index on
                     behalf of it's caller, or indicates why it can't.

I_Find_OID           Finds Object Id Entry in Intermediate MIR Index, and
                     returns enough information to allow insertion of one or
                     more new arcs needed to register a new OID.  This code
                     supports the operation of "I_Register_OID()" and is also
                     used by the remove code to find an Entity-Class by OID.

I_Insert_Rel_Entry   Inserts a new Relationship entry into a given Non-Terminal

I_Create_IDS_SNUMBER Creates a Terminal Object that is a Signed Number and
                     records it in the compiler's back-end list of this kind
                     of object.

I_Create_IDS_UNUMBER Creates a Terminal Object that is an Unsigned Number and
                     records it in the compiler's back-end list of this kind
                     of object.

I_Create_IDS_STRING  Creates a Terminal Object that is a String and
                     records it in the compiler's back-end list of this kind
                     of object.

I_AddNew_IDS_SNUMBER Creates a Terminal Object that is a Signed Number and
                     adds it to the end of the compiler's back-end list of
                     this kind of object, *as the next* in an ordered series 
                     of these objects.

I_AddNew_IDS_UNUMBER Creates a Terminal Object that is a Unsigned Number and
                     adds it to the end of the compiler's back-end list of this
                     kind of object, *as the next* in an ordered series of
                     these objects.

I_AddNew_IDS_STRING  Creates a Terminal Object that is a String and
                     adds to the end of the compiler's back-end list of this
                     kind of object, *as the next* in an ordered series of
                     these objects.

I_Create_IDS         Creates a "Not Terminal" ("Non-Terminal) Intermediate
                     Data Structure of the specified flavor and records it
                     in the compiler's back-end list of this kind of object.

I_Reclaim_IDS        Reclaims a now-unneeded IDS (of any flavor) and makes it
                     available to the I_Create_IDS*() functions for use when
                     another allocation is needed.  IDS's are re-used on a
                     FIFO basis.

INTERNAL FUNCTIONS:
I_Insert_Slice_Entry Inserts a new entry into a given Index Slice (on behalf
                     of "I_Register_OID()".

I_Add_Table_Entry    Guarantees Size of an Object's Table (either a Slice's
                     Table or a Non-Terminal's Table) can hold 1 More Entry
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef VMS
#include <malloc.h>
#endif

#ifndef NULL
#define NULL (void *)0
#endif

/* Request definitions for compiler modules from "mir.h" */
#define MIR_COMPILER
#include "mir.h"


/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp200();
extern char *mp201();
#endif


/*
|  Intermediate Context Block
|
|  This pointer points to the context that all the functions in this module
|  use.  By registering different contexts with this module, the intermediate
|  representation functions (those in this module) can be made to work on
|  more than one compiled "intermediate" representation of one or more MSL
|  files.  This capability is introduced with an eye to supporting a "merge"
|  capability for the compiler at a future date.
|
|  This pointer must be set through a call to "I_Register_Context()" before
|  any other functions in the module may be invoked.
*/
static inter_context  *ic=NULL;


/*
|
|       Prototypes for module-local functions
|
*/
/* I_Insert_Slice_Entry - Inserts a new entry into a given Index Slice */
static MC_STATUS I_Insert_Slice_Entry PROTOTYPE((
IDS                 *,      /* Address of slice being inserted to       */
int                 ,       /* Index to entry where insert should occur */
unsigned int                /* Value to be placed into the new slot     */
));

/* I_Add_Table_Entry-Guarantees Size of Object's Table can hold 1 More Entry */
static BOOL I_Add_Table_Entry PROTOTYPE((
IDS_TYPE          ,   /* Type of object whose table is being expanded */ 
VOID            **,   /* Address of pointer to table being expanded   */
unsigned short   *,   /* Address of entry counter for table           */
unsigned short   *    /* Address of maximum count for table           */
));


/* I_Register_Context - Register an Intermediate Context Block */
/* I_Register_Context - Register an Intermediate Context Block */
/* I_Register_Context - Register an Intermediate Context Block */

MC_STATUS
I_Register_Context (context)

inter_context  *context;   /* The Intermediate Context block ptr */

/*
INPUTS:

   "context" is a pointer to the intermediate context block to be
   registered for use by the functions in this module.

OUTPUTS:

    The function returns one of:

    MC_SUCCESS - The specified context block was successfully registered.

BIRD'S EYE VIEW:
    Context:
        The caller has allocated space for an intermediate context block and
        wishes to register (or re-register) the block in this module.

    Purpose:
        This function does the "registration".


ACTION SYNOPSIS OR PSEUDOCODE:

   <record address of intermediate context block local to the module>
   <return>

OTHER THINGS TO KNOW:

   This function serves to

      * eliminate the need for any references to global symbols outside of
        this module

      * eliminate the need for any global data symbols within this module
        for reference by code outside of this module.
*/
{
ic = context;
return(MC_SUCCESS);
}

/* I_Register_OID - Register new Object ID in Intermediate MIR Index */
/* I_Register_OID - Register new Object ID in Intermediate MIR Index */
/* I_Register_OID - Register new Object ID in Intermediate MIR Index */

MC_STATUS
I_Register_OID (oid, oid_smi, object)

object_id   *oid;       /* -> Object Id structure to be inserted      */
mir_oid_smi oid_smi;    /* SMI whose rules used to construct OID */
IDS         *object;    /* -> Object in dictionary to be registered   */

/*
INPUTS:

    "oid" is the address of an object id structure containing the object
    id to be inserted into the MIR Intermediate Index.

    "oid_smi" is an indicator for the SMI whose rules governed the
    construction of the OID.

    "object" is the address of the object.  The address (heap address) is
    to be inserted into the MIR Intermediate Index structure element
    allocated to record the "oid" object id.

OUTPUTS:

    The function returns one of:

    MC_REG_Already_OID - The specified Object ID has already been registered
                         (The existing instance remains intact, the index is
                          not changed, you know that the OID has been used
                          to register an object DIFFERENT from the object
                          passed as an argument).

    MC_REG_Already_OBJ - The specified Object has already been registered
                         using the specified Object ID.
                         (The existing instance remains intact, the index is
                          not changed, you know that the OID has already been
                          used to register the very SAME object passed as the
                          "object" argument).

    MC_REG_SMI_Already - The specified Object has already been registered in
                         this SMI by another OID.

    MC_OUT_OF_MEMORY - Insufficient heap memory was available to add the new
                    instance to the MIR Intermediate Index structures.  The
                    index may be corrupted.

    MC_FAILURE - Internal logic error, an already-registered object was
                 missing a valid OID backpointer. . . discovered as we were
                 rearranging the index to accomodate the new object

    MC_SUCCESS - The specified Object Id was successfully entered into the
                    MIR Intermediate Index, and the object's reference count
                    is incremented by one.

BIRD'S EYE VIEW:
    Context:
        The caller has allocated space for a Non-Terminal Object and wants
        to register the Object Id of the object in the MIR Intermediate Index
        so that it can be retrieved later by Object ID.

    Purpose:
        This function modifies and/or extends the MIR Intermediate Index
        to accomodate the new Object ID.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (I_Find_OID returns MC_FIND_Exact)
        if (Full Entry points to Index Slice)  (* convert existing to Subreg *)
            <create Subregister IDS>
            <load pointer from Full Entry (to Index Slice) into Subregister>
            <change Full entry pointer to point to Subregister IDS>
            <insert Object IDS address into Subregister>
            if (object has no OID for this SMI)
                <make the object point back at the index in a way that
                  indicates OID SMI>
            else
                <return MC_REG_SMI_Already>
            <record the last arc in the object for disambiguation>
            <change pted-at Index Slice backptr to point to new subregister>
            <return MC_SUCCESS>

        if (Full Entry points to Subregister)
            <set local pointer to object from Subregister>
        else
            <set local pointer to object from Slice>

        if (Object from Index is same as inbound object)
            <return MC_REG_Already_OBJ>
        else
            <return MC_REG_Already_OID>

    do
        if (I_Find_OID returned "MC_FIND_Exact_Short")
            <create Subregister IDS>
            <load pointer from Full Entry (to object) into Subregister>
            <make Object's OID backpointer point now to the new Subregister>
            if (Object's OID backpointer not found)
                <return MC_FAILURE>
            <change Full entry pointer to point to Subregister IDS>
            <allocate fresh slice and insert address into Subregister IDS>
            <continue>

        if (I_Find_OID returned "MC_FIND_Exact_Long")
            if (slot entry points to subregister)
                if (Object from Index is same as inbound object)
                    <return MC_REG_Already_OBJ>
                else
                    <return MC_REG_Already_OID>
            <create Subregister IDS>
            <load pointer from Full Entry (to slice) into Subregister>
            <change Full entry pointer to point to Subregister IDS>
            <insert Object IDS address into Subregister>
            if (object has no OID for this SMI)
                <make the object point back at the index in a way that
                  indicates OID SMI>
            else
                <return MC_REG_SMI_Already>
             OID SMI>
            <update longest-arc recorder>
            <return MC_SUCCESS>

        (* Must have returned MC_FIND_Partial *)
        if (perform I_Insert_Entry into Slice returned by I_Find_OID FAILED)
            <return MC_OUT_OF_MEMORY>
        if (insertion was for last arc in object id)
            <break>
        <allocate fresh slice and insert address into Slice Entry Structure
         of last inserted entry>
        while (I_Find_OID returns MC_FIND_Partial or MC_FIND_Exact_Short or
                     _Long)

    <insert object address into Slice Entry Structure of last inserted arc>
    if (object has no OID for this SMI)
        <make the object point back at the index in a way that
          indicates OID SMI>
    else
        <return MC_REG_SMI_Already>
    <update longest-arc recorder>
    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

   In general, any data structure that points someplace else (ie
everything except numbers and string) including index slices, index
subregisters, and non-terminals ALL "point back" at whatever points DIRECTLY
at them.  So this means that if:

   * an entry in a slice points at a non-terminal, then the non-terminal
     points back at the slice (thru the appropriate OID backpointer)

   * an entry in a slice that points at a subregister, then the subregister
     points back at the slice, but the object and the slice in the subregister
     both point back AT THE SUBREGISTER.

vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv SUPERSEDED vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
With V1.5 of the compiler, we allow an object to be registered under multiple
object identifiers.  This causes the compiler no problem, except that the Tier
0 function which returns the OID can only return one OID, and we want it to be
the first OID an object was registered under.  We do this by only setting an
object's backpointer (that points into the index) ONCE, (the first time an
object is registered).

With this change, the Tier 0 function that returns the OID for an object
will indeed return the OID it was first registered under UNLESS the first
and any subsequent OIDs are:

  * EXACTLY the same length AND
  * Match EXACTLY except for the last arc AND
  * the first-registered OID's last arc is GREATER than a subsequently
    registered OID's last arc (in which case the subsequently-registered
    OID is the one returned as the object's OID).

It is difficult and noxious to fix this complication, and a fix is probably
unneeded. (It entails a change to the external format of the
MIR database binary file to allow multiple backpointers in the case of a
multiply-indexed MIR object... this implies changes in MIR_EXTERNAL.C
and MIR_SYMDUMP.C as well as the Tier 0 code).

This problem arises because the original data-structures were designed under
the assumption that an object is registered only once, and the OID derivation
algorithm could simply scan the index slice specified by the backpointer of
an object to find in that slice the address of the object (for that OID) and
thereby infer the (last) arc number (the same scheme is applied to derive
all the other arc numbers too).

If the conditions described above occur, the address of the object will occur
twice (once for each OID) in the particular index slice, but the Tier 0
function will return only the first occurrence in the slice, which may NOT be
the first-registered OID in time-sequence.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ SUPERSEDED ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With V2.0 the restriction described above no longer applies, and it is
possible to determine which SMI an OID was constructed under and it is also
possible to retrieve an OID for a specific object "by SMI".  This changes
has indeed resulted in a changed binary output file format.

*/

{
IDS     *ids_slice;     /* ->IDS of type "I_SLICE" somewhere in MIR index */
int     slice_index;    /* Pointer to slot in ids_slice.                  */
int     match_count;    /* Count of arcs that were matched on I_Find_OID  */
IDS     *new_subreg;    /* --> Newly created subregister                  */
IDS     *new_slice;     /* --> Newly created slice                        */
IDS     *index_object;  /* --> A MIR Non-Terminal Object found in Index   */
struct SES
        *sl;            /* Slice List (as extracted from Slice IDS)       */

MC_STATUS
        ret_code;       /* Returned from call to I_Find_OID               */

/* if (I_Find_OID returns MC_FIND_Exact) */
if ( (ret_code = I_Find_OID (oid, &ids_slice, &slice_index, &match_count))
                                                         == MC_FIND_Exact) {

    /* if (Full Entry points to Index Slice)  ---convert existing to Subreg
    |      This implies that there is already something in the index that
    |      has "oid" as a PREFIX.  We have to arrange for the index to
    |      not only point to the things that have "oid" as a PREFIX but to ALSO
    |      point at "object".  We need a "subregister" IDS to do this.
    */
    sl = ids_slice->idsu.i.slice;
    index_object = sl[slice_index].next;        /* Grab ptr to Object */
    if (index_object->flavor == I_SLICE) {

        /* create Subregister IDS */
        if ( (new_subreg = I_Create_IDS(I_SUBREGISTER,
                                        IDS_DFT_TABLESIZE)) == NULL) {
            return (MC_OUT_OF_MEMORY);
            }

        /* load pointer from Full Entry (to Index Slice) into Subregister */
        new_subreg->idsu.s.lower_levels = sl[slice_index].next;

        /* change Full entry pointer to point to Subregister IDS */
        sl[slice_index].next = new_subreg;
        new_subreg->idsu.s.backptr = ids_slice; /* Set up subreg's backptr */

        /* insert Object IDS address into Subregister */
        new_subreg->idsu.s.ntobject = object;

        /*
        |  Make the object point back at the index in a way that
        |  indicates the OID and the SMI from which it came.
        |
        |  We check to be sure that this object has no other OID registered
        |  in this SMI.
        */
        if (object->idsu.nt.oid_backptr[oid_smi] == NULL)
            object->idsu.nt.oid_backptr[oid_smi] = new_subreg;
        else
            return (MC_REG_SMI_Already);

        /*
        | Record the actual last-arc (far right-most) in order to disambiguate
        | OIDs that happen to match exactly except for the last arc.
        |
        | This arc value is not needed for situations such as this one,
        | because there is no way two OIDs can be involved in a subregister
        | in such a fashion that the subregister 'points' to one object (as
        | can happen with an index slice containing two slot entries that
        | point at the same object).  However we record this anyway, as
        | when things can be deleted, this arc may become important in
        | reconfiguring the index after something has been deleted).
        |
        | (Disambiguation is done by the final compiler pass).
        */
        object->idsu.nt.oid_backarc[oid_smi] = sl[slice_index].iso_arc;

        /* change pted-at Index Slice backptr to point to new subregister */
        new_subreg->idsu.s.lower_levels->idsu.i.backptr = new_subreg;

        /*
        |  (We don't update the longest OID count here because we know that
        |   there are other things longer in the index)
        */

        return (MC_SUCCESS);
        }

    /*
    |  At this point, we know the input OID has selected some MIR Non-Terminal
    |  that has already been registered in the index.  This is because the
    |  slice_index entry must perforce point at either a Subregister (that
    |  in turn points at a Non-Terminal IDS) or the entry points directly
    |  at a Non-Terminal IDS.  We have a pointer to that object
    |  ("index_object") and return a status indicating whether the caller
    |  is trying to "re-register" an OID that has already been used for
    |  some OTHER object, or that they are trying to "re-register" an OID
    |  for the SAME object (again).
    */
    /* if (Full Entry points to Subregister) */
    if (index_object->flavor == I_SUBREGISTER) {
        /* set local pointer to object from Subregister */
        index_object = index_object->idsu.s.ntobject;
        }
    /* else index_object already points at the Non-Terminal object */

    /* if (Object from Index is same as inbound object) */
    if (index_object == object ) 
        return (MC_REG_Already_OBJ);    /* Object already registered w/OID   */
    else
        return (MC_REG_Already_OID);    /* OID already used for other object */
    }

/* If we reach here, we must be in the process of actually inserting
|  something that needs either a new subregister or new slice.
|
|  The call to I_Find_OID() above took us as far as we can go in one fell
|  swoop.  Now each time around this loop below, we typically modify the
|  existing index to accomodate one more arc, repeating until all the arcs
|  in the incoming OID have been recorded in some manner in the index, or
|  we encounter a duplicate or some other error.
*/
do  {
    /* if (I_Find_OID returned "MC_FIND_Exact_Short")
    |   This implies we've reached the bottom of the existing index and
    |   we've got to create more index slices to carry the remaining arcs
    |   in the input OID.  Also, since we're at the bottom, we'll need a
    |   subregister (in order to carry on) in addition to new index slices.
    */
    if (ret_code == MC_FIND_Exact_Short) {

        IDS     *i_object;      /* (Already) Indexed Obj (other than input) */
        int     i;              /* Handy-Dandy loop index                   */

        /* create Subregister IDS */
        if ( (new_subreg = I_Create_IDS(I_SUBREGISTER,
                                        IDS_DFT_TABLESIZE)) == NULL) {
            return (MC_OUT_OF_MEMORY);
            }

        /* load pointer from Full Entry (to object) into Subregister */
        sl = ids_slice->idsu.i.slice;
        i_object = new_subreg->idsu.s.ntobject = sl[slice_index].next;

        /* make Object's OID backpointer point now to the new Subregister
        |
        |  Do this by scanning down the Object's Back (pointer & arc)
        |  arrays until we find an exact match on both the back ptr and the arc
        */
        for (i=0; i < MAX_OID_SMI; i++)
            if (i_object->idsu.nt.oid_backptr[i] == ids_slice &&
                i_object->idsu.nt.oid_backarc[i] == sl[slice_index].iso_arc) {
                i_object->idsu.nt.oid_backptr[i] = new_subreg;
                break;
                }
        /* sanity check: we'd better have found a valid backptr! */
        if (i >= MAX_OID_SMI)
            return (MC_FAILURE);

        /* change Full entry pointer to point to Subregister IDS */
        sl[slice_index].next = new_subreg;
        new_subreg->idsu.s.backptr = ids_slice; /* Set up subreg's backptr */

        /* allocate fresh slice and insert address into Subregister IDS */
        if ( (new_slice = I_Create_IDS(I_SLICE, IDS_DFT_TABLESIZE)) == NULL) {
            return (MC_OUT_OF_MEMORY);
            }
        new_subreg->idsu.s.lower_levels = new_slice;    /* Insertion    */
        new_slice->idsu.i.backptr = new_subreg;         /* Back pointer */

        ret_code =
            I_Find_OID(oid, &ids_slice, &slice_index, &match_count);
        continue;
        }

    /* if (I_Find_OID returned "MC_FIND_Exact_Long")
    |      This implies that there is more index to examine, but we ran
    |      out of input OID arcs to compare with.  The only way the short
    |      input OID could already point at a Non-Terminal object is thru
    |      a Subregister pointed to by the slice_indexth entry in the current
    |      slice.  (If it does, we have to return a status indicating whether
    |      it is the same object the caller is trying to register or not).
    */
    if (ret_code == MC_FIND_Exact_Long) {

        /* if (slot entry points to subregister) */
        sl = ids_slice->idsu.i.slice;
        if (sl[slice_index].next->flavor == I_SUBREGISTER) {
            if (sl[slice_index].next->idsu.s.ntobject == object) {
                return (MC_REG_Already_OBJ);
                }
            else {
                return (MC_REG_Already_OID);
                }
            }

        /* create Subregister IDS to allow recording a ptr to the new obj */
        if ( (new_subreg = I_Create_IDS(I_SUBREGISTER,
                                        IDS_DFT_TABLESIZE)) == NULL) {
            return (MC_OUT_OF_MEMORY);
            }

        /* load pointer from Full Entry (to slice) into Subregister */
        new_subreg->idsu.s.lower_levels =
                sl[slice_index].next;

        /* change Full entry pointer to point to Subregister IDS */
        sl[slice_index].next = new_subreg;

        /* insert Object IDS address into Subregister */
        new_subreg->idsu.s.ntobject = object;

        /*
        |  Make the object point back at the index in a way that
        |  indicates the OID and the SMI from which it came.
        |
        |  We check to be sure that this object has no other OID registered
        |  in this SMI.
        */
        if (object->idsu.nt.oid_backptr[oid_smi] == NULL)
            object->idsu.nt.oid_backptr[oid_smi] = new_subreg;
        else
            return (MC_REG_SMI_Already);

        /*
        | Record the actual last-arc (far right-most) in order to disambiguate
        | OIDs that happen to match exactly except for the last arc!
        | (Disambiguation is done by the final compiler pass).
        */
        object->idsu.nt.oid_backarc[oid_smi] = sl[slice_index].iso_arc;

        /* update longest-arc recorder */
        if (ic->arc_count < oid->count) {
            ic->arc_count = oid->count;
            }

        return (MC_SUCCESS);
        }

    /* Must have returned MC_FIND_Partial
    |   This implies that we've found a slice in which an arc from the
    |   input OID should be found.  We insert it, and if more arcs remain
    |   in the input OID, we carry on.
    */
    /* if (perform I_Insert_Entry into Slice returned by I_Find_OID FAILED) */
    if (I_Insert_Slice_Entry(ids_slice,
                             slice_index,
                             (unsigned int) oid->value[match_count])
        != MC_SUCCESS) {
        return (MC_OUT_OF_MEMORY);
        }

    /* if (insertion was for last arc in object id) */
    if ( (match_count+1) == oid->count)
        break;

    /* allocate fresh slice and insert address into Slice Entry Structure */
    /* of last inserted entry */
    if ( (new_slice = I_Create_IDS(I_SLICE, IDS_DFT_TABLESIZE)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }
    sl = ids_slice->idsu.i.slice;
    sl[slice_index].next = new_slice;
    new_slice->idsu.i.backptr = ids_slice;

    ret_code =
        I_Find_OID(oid, &ids_slice, &slice_index, &match_count);
    }

    /* while (I_Find_OID returns:
       MC_FIND_Partial or MC_FIND_Exact_Short or _Long) */
    while (ret_code == MC_FIND_Partial ||
           ret_code == MC_FIND_Exact_Short ||
           ret_code == MC_FIND_Exact_Long);


/* insert object address into Slice Entry Structure of last inserted arc */
sl = ids_slice->idsu.i.slice;
sl[slice_index].next = object;

/*
|  Make the object point back at the index in a way that
|  indicates the OID and the SMI from which it came.
|
|  We check to be sure that this object has no other OID registered
|  in this SMI.
*/
if (object->idsu.nt.oid_backptr[oid_smi] == NULL)
    object->idsu.nt.oid_backptr[oid_smi] = ids_slice;
else
    return (MC_REG_SMI_Already);

/*
| Record the actual last-arc (far right-most) in order to disambiguate OIDs
| that happen to match exactly except for the last arc!  (Disambiguation is
| done by the final compiler pass).
*/
object->idsu.nt.oid_backarc[oid_smi] = sl[slice_index].iso_arc;


/* update longest-arc recorder */
if (ic->arc_count < oid->count) {
    ic->arc_count = oid->count;
    }

return (MC_SUCCESS);

}

/* I_Find_OID - Find Object Id Entry in Intermediate MIR Index */
/* I_Find_OID - Find Object Id Entry in Intermediate MIR Index */
/* I_Find_OID - Find Object Id Entry in Intermediate MIR Index */

MC_STATUS
I_Find_OID (oid, ids_slice, slice_index, match_count)

object_id  *oid;         /*  IN: -> Object Id structure to be inserted       */
IDS        **ids_slice;  /* OUT: Address of pointer to slice being returned  */
int        *slice_index; /* OUT: Address of integer to return slice index to */
int        *match_count; /* OUT: Address of integer to return match count to */

/*
INPUTS:

    "oid" is the address of an object id structure whose closest match in
    the MIR Intermediate Index is sought.

    "ids_slice" is the address of a pointer to the last "slice" of
    the MIR Intermediate Index structure that I_Find_OID searched.  The
    address of this slice is returned to the caller.

    "slice_index" is the address of an integer into which is returned
    the index to the "slot" in "ids_slice" that I_Find_OID is specifying
    as the slot into which the "match_count"th arc of the OID is found
    OR should be placed into ("match_count+1", when there was no match).
    See OUTPUTS for details.

OUTPUTS:

    The function returns one of:

    MC_FIND_Exact:
    The specified Object ID was fully found in the MIR Intermediate Index.
    "match_count" exactly matches the total number of arcs in the incoming
    OID while "ids_slice" points to the slice containing the "slice_index"th
    slot whose value corresponds to the last arc in the OID.

    It is up to the caller to determine whether this value points to a Slice
    IDS, a Sub-Register IDS or a Non-Terminal IDS.

    Exactly the same number of slices contained in the MIR Index as were
    required for arcs in the specified Object Id were used to resolve to a MIR
    object (ie, there may be more slices in the index, but we're out of arcs
    in the incoming OID, but all arcs in the incoming OID had entries in the
    index) :
                        Supplied OID: 1.2.3.4
                   Closest Index OID: 1.2.3.4 <-slice_index points to slot
                                            ^   containing arc of "4"
                                            |
                                            *--slice_ptr is set to point here
                                            match_count = 4 (number of arcs
                                                             matched)

    >>>-----------------------------------------------------------------------
    >>> If we're trying to register the specified OID in the MIR, this code
    >>> means that
    >>>
    >>>  if the MIR slice entry points at a subregister or non-terminal 
    >>>  object, then the specified OID already has been registered.
    >>>
    >>>  if the MIR slice points to a slice, then a subregister must be created
    >>>  to contain the object in order to register the specified OID.
    >>>-----------------------------------------------------------------------

    MC_FIND_Partial:
    The specified Object ID was partially found in the MIR Intermediate Index.
    The number of arcs in the incoming OID that WERE FOUND is returned in
    "match_count", while a pointer to the slice in which the (match_count+1)th
    arc should be placed is returned in "ids_slice" while the index to the
    slot where the "match_count+1"th arc should go is returned in
    "slice_index".

    In other words a slice was found with no arc to match an arc in the
    inbound specified Object ID:
                            Supplied OID: 1.2.4[.*...perhaps more, not known]
                               Index OID: 1.2.3[.*...perhaps more, not known]
                                          1.2.5[.* <-slice_index points to slot
                                              ^      containing the 5
                                              |
                                              *--slice_ptr is set to point here
                                                 match_count = 2
    >>>-----------------------------------------------------------------------
    >>> If we're trying to register the specified OID in the MIR, this code
    >>> means that another slot in the specified slice must be used to
    >>> represent the arc that was not found in that slice.
    >>>
    >>> If the arc is
    >>> NOT the last in the specified OID, then the new slot must be made to
    >>> point at a new slice (to contain the next arc).  
    >>>
    >>> If the arc IS
    >>> the last in the specified OID, then the new slot must be made to the
    >>> actual object that the specified OID represents.
    >>>-----------------------------------------------------------------------


    MC_FIND_Exact_Short:
    The part of specified Object ID matched a full entry in the MIR index such
    that the full entry specified an object in the MIR. There were more arcs
    in the specified Object ID than slices with matching entries in the MIR
    index:
                        Supplied OID: 1.2.3.*
                   Closest Index OID: 1.2.3 <-slice_index points to slot
                                          ^   containing the 3
                                          |
                                          *--slice_ptr is set to point here
                                          match_count = 3

    >>>-----------------------------------------------------------------------
    >>> If we're trying to register the specified OID in the MIR, this code
    >>> means that the specified slot must be changed from pointing to an
    >>> object in the MIR to pointing to a subregister object (which must be
    >>> created).  The new subregister object must be made to point at the old 
    >>> object that the original slot entry used to point at directly, and 
    >>> then additionally the subregister must be made to point at a new slice
    >>> (which must also be created) which will hold the next arc in the 
    >>> specified OID that could not be compared against the MIR index on this
    >>> call because there were no more slices at this level.  (Got that?)
    >>>
    >>> This code implies that the bottom-most slice in the MIR index for the
    >>> given search path dictated by the specified OID has been reached.  This
    >>> further implies that the slot MUST point at a non-terminal object in
    >>> the MIR (and not a subregister or slice ... which would imply "not
    >>> the bottom" of the MIR index).
    >>>-----------------------------------------------------------------------

    MC_FIND_Exact_Long:
    The specified Object ID fully matched a portion of the MIR Index:

                        Supplied OID: 1.2.3
                   Closest Index OID: 1.2.3.* <-slice_index points to slot
                                          ^   containing the 3
                                          |
                                          *--slice_ptr is set to point here
                                          match_count = 3

    >>>-----------------------------------------------------------------------
    >>> If we're trying to register the specified OID in the MIR, this code
    >>> means that the slot found in the slice must be examined to see what
    >>> it points at.  If it points at:
    >>> --a Subregister:  This implies that the OID is already registered in
    >>>                   the index for an object
    >>>
    >>> --a Slice: This implies that a subregister must be created in order
    >>>            to properly register the specified object in the index
    >>>
    >>>-----------------------------------------------------------------------

BIRD'S EYE VIEW:
    Context:
        The caller wants to search the MIR Intermediate Index to determine
        EITHER where the next arc in an OID should go (when inserting)
        OR what the heap address of an Intermediate Data Structure is of
        the IDS for a particular OID (when doing a bona fide lookup).

    Purpose:
        This function searches however much of the Intermediate Index there
        may be in existence for the specified Object ID, returning enough
        information to either obtain an object that was already registered
        or enough information to expand the Intermediate Index to include
        a new OID.

ACTION SYNOPSIS OR PSEUDOCODE:

    <obtain the address of the top of the Intermediate Index->slice ptr>
    <set local slice index to 0>
    <set local match_count to 0>

    do
        <scan the current slice for entry that matches, set local slice index>

        if (scan found EXACT MATCH)
            <increment local match_count>

            if (match_count is same as arc-count in input OID)
                <return match_count to caller>
                <return slice pointer to caller>
                <return slice index to caller>
                <return MC_FIND_Exact>

            if (found entry points to a Sub-Register)
                <set local slice pointer to lower-levels subreg value>
                <continue>
            else if ("next" value of entry is a Slice)
                <set local slice pointer to "next" value of entry>
                <continue>
            else  (* we're out of index slices to check *)
                <return match_count to caller>
                <return slice pointer to caller>
                <return slice index to caller>
                <return MC_FIND_Exact_Short>

        <return match_count to caller>
        <return slice pointer to caller>
        <return slice index to caller>
        <return MC_FIND_Partial>

        while (match_count < count of arcs in oid)

    (* There are more index slices to check, but we're out of arcs in *)
    (* the inbound Object ID *)
    <return match_count to the caller>
    <return slice pointer to caller>
    <return slice index to caller>
    <return MC_FIND_Exact_Long>


OTHER THINGS TO KNOW:

    This function employs a slow linear scan of each slice.  No big deal.
    The equivalent function for the External form of the index can use
    a binary search.

    This function is also kinda tricky, as it compares variable length
    ISO Object IDs in essence, and returns enough info to allow the insertion
    (into a slice of the MIR index) of an arc that failed to match.  However,
    it should be thought of as actually examining a single slice in the MIR
    Index (or arc of the supplied ISO object ID) at a time (since it's real
    duty is to examine just enough of the MIR index given a supplied ISO Object
    ID to allow deciding how to modify *just the last slice examined* in order
    to include *just the last arc examined in the ISO Object ID* in the index.

    You really really want to be careful if you believe you've found a bug
    here, not because the liklihood of a bug is low, just that the code is
    rather tricky.

*/

{
IDS     *slice_ptr;    /* Points to each slice in turn as we walk down index */
int     index;         /* Points into slice_ptr's slice to entry being used  */
int     m_c;           /* Count of matched arcs in inbound ISO Object Id     */
IDS     *entry;        /* Entry from selected slot in MIR Slice              */
struct SES
        *sl;           /* -> Slice List                                      */

/*
| obtain the address of the top of the Intermediate Index->slice ptr
| (create the topmost empty slice if it doesn't exist yet)
*/
if ((slice_ptr = ic->index_top) == NULL) {
    slice_ptr = ic->index_top = I_Create_IDS(I_SLICE, IDS_DFT_TABLESIZE);
    };

m_c= 0;    /* set local match_count to 0 */

do  {

    /*
    |  Scan the current slice for entry that matches, set local slice index
    |  (We're looking for first entry that matches or is greater than the
    |  supplied arc from the input OID).
    */
    entry = NULL;
    sl = slice_ptr->idsu.i.slice;
    for (index = 0; index < slice_ptr->idsu.i.entry_count; index++) {
        if ( sl[index].iso_arc >= oid->value[m_c] ) {
            entry = sl[index].next; /* Grab Entry */
            break;
            }
        }

    /* if (scan found EXACT MATCH) */
    if ( (entry != NULL) &&
         (sl[index].iso_arc == oid->value[m_c])
       ) {
        m_c += 1;   /* increment local match_count */

        /* if (match_count is same as arc-count in input OID) */
        if (m_c == oid->count) {
            *match_count = m_c;     /* return match_count to caller */
            *ids_slice = slice_ptr; /* return slice pointer to caller */
            *slice_index = index;   /* return slice index to caller */
            return (MC_FIND_Exact);
            }

        /* if (found entry points to a Sub-Register) */
        if (entry->flavor == I_SUBREGISTER) {
            /* set local slice pointer to lower-levels subreg value */
            slice_ptr = entry->idsu.s.lower_levels;
            continue;
            }

             /* "next" value of entry is Slice */
        else if (entry->flavor == I_SLICE) { 
            /* set local slice pointer to "next" value of entry */
            slice_ptr = entry;
            continue;
            }
        else {
            *match_count = m_c;     /* return match_count to caller */
            *ids_slice = slice_ptr; /* return slice pointer to caller */
            *slice_index = index;   /* return slice index to caller */
            return (MC_FIND_Exact_Short);
            }
        }

    /*
    |  If we fall thru to here, then we've just scanned a slice in the MIR
    |  index and failed to find a match, and "index" points to a slot (that
    |  may be filled or not) where the match should have occurred.  This
    |  slot's contents (if any) must be slid "upward" to make room for the
    |  new entry.
    */
    *match_count = m_c;     /* return match_count to caller */
    *ids_slice = slice_ptr; /* return slice pointer to caller */
    *slice_index = index;   /* return slice index to caller */
    return (MC_FIND_Partial);

    /* (match_count < count of arcs in oid) */
    }
    while (m_c < oid->count);

/*
|  There are more index slices to check, but we're out of arcs in the
|  inbound Object ID
*/
*match_count = m_c;             /* return match_count to caller   */
*ids_slice = slice_ptr;         /* return slice pointer to caller */
*slice_index = index;           /* return slice index to caller   */
return (MC_FIND_Exact_Long);

}

/* I_Insert_Slice_Entry - Inserts a new entry into a given Index Slice */
/* I_Insert_Slice_Entry - Inserts a new entry into a given Index Slice */
/* I_Insert_Slice_Entry - Inserts a new entry into a given Index Slice */

static MC_STATUS
I_Insert_Slice_Entry (ids_slice, slice_index, value)

IDS                 *ids_slice;  /* Address of slice being inserted to       */
int                 slice_index; /* Index to entry where insert should occur */
unsigned int        value;       /* Value to be placed into the new slot     */

/*
INPUTS:

    "ids_slice" is the address of the "slice" of the MIR Intermediate Index
    structure that the insertion should occur into.

    "slice_index" is the index to the "slot" in "ids_slice" where an
    insertion should occur (ie everything AT or ABOVE this slot should be
    moved "up" one slot).

    "value" is the value to be placed into the new slot (ie if an ISO object
    id of "1.3.4" is being inserted into the index, then "3" is the value to
    be placed into a level 2 "slice" in an entry being inserted by a call
    to this function.

OUTPUTS:

    The function returns one of:

        MC_SUCCESS - Insertion occurred OK, and the "value" was left in
        the new slot.

        MC_OUT_OF_MEMORY - Insufficient memory to complete insertion.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to insert another arc for an ISO object ID into
        the Intermediate MIR index slice provided in the call.

    Purpose:
        This function slides any entries in the slice at and above the
        insertion point "up" to accomodate the new entry.  Allocation
        of additional memory is provided as needed.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (perform I_Add_Table_Entry processing to guarantee space FAILED)
        <return MC_OUT_OF_MEMORY>
    <compute number of entries that must be "slid">
    while (number of entries remaining > 0)
        <copy nTH entry to n+1TH slot>
        <decrement count of entries remaining>
    <insert value into slice entry>
    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    The capitol of Equador is Quito.

*/

{
int     slide_count;        /* Number of entries that must be slid upward */
struct SES
        *sl;                /* Pointer to Slice List array */

/* if (perform I_Add_Table_Entry processing to guarantee space FAILED) */
if (I_Add_Table_Entry (I_SLICE,                    /* The kind of table      */
             (VOID **) &ids_slice->idsu.i.slice,   /* -> to address of table */
                       &ids_slice->idsu.i.entry_count, /*->count in table    */
                       &ids_slice->idsu.i.max_count    /*->maximum for table */
                       )
                != TRUE) {
    return(MC_OUT_OF_MEMORY);
    }

/* compute number of entries that must be "slid"                            */
/* (NOTE: entry_count  in the table has ALREADY been incremented by 1)      */
slide_count = (ids_slice->idsu.i.entry_count - slice_index) - 1;

/* while (number of entries remaining > 0) */
sl = ids_slice->idsu.i.slice;
while (slide_count > 0) {

    /* copy nTH slot entry to n+1TH slot */
    sl[slice_index+slide_count] = sl[slice_index+slide_count - 1];

    /* decrement count of entries remaining */
    slide_count -= 1;
    }

/* insert value into slice entry */
sl[slice_index].iso_arc = value;

return (MC_SUCCESS);
}

/* I_Insert_Rel_Entry - Inserts a new Relationship into a given Non-Terminal */
/* I_Insert_Rel_Entry - Inserts a new Relationship into a given Non-Terminal */
/* I_Insert_Rel_Entry - Inserts a new Relationship into a given Non-Terminal */

MC_STATUS
I_Insert_Rel_Entry (ids_nt, rel_nt, target)

IDS     *ids_nt;    /* Address of non-terminal being inserted to        */
IDS     *rel_nt;    /* Address of relationship non-terminal to insert   */
IDS     *target;    /* Address of target to insert                      */


/*
INPUTS:

    "ids_nt" is the address of the MIR Intermediate Non-Terminal IDS
    structure that the insertion should occur into.

    "rel_nt" is the address of the MIR Intermediate Non-Terminal 
    structure that is the Relationship Non-Terminal

    "target" is the address of the MIR Intermediate Non-Terminal or Terminal
    IDS.

OUTPUTS:

    The function returns one of:

        MC_SUCCESS - Insertion occurred OK, and the "rel_nt" and "target"
        were inserted at the end of the relationship table in "ids_nt".
        The reference count of both relationship and target objects is
        incremented.

        MC_OUT_OF_MEMORY - Insufficient memory to complete insertion.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to insert another Relationship Entry
        the Intermediate MIR Non-Terminal provided in the call.

    Purpose:
        This function insures enough space exists for another entry and
        adds the new entry at the end of the relationship table.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (perform I_Add_Table_Entry processing to guarantee space FAILED)
        <return MC_OUT_OF_MEMORY>
    <insert value into relationship table at the end>
    if (target is a Terminal)
        <increment reference count of target>
    else if (target is a general Non-Terminal)
        <increment reference count of target>

    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    The Straits of Constantinople are also known as the Dardanelles.
*/

{
struct RES *rt;     /* Pointer to array of structures */

/* if (perform I_Add_Table_Entry processing to guarantee space FAILED) */
if (I_Add_Table_Entry (ids_nt->flavor,             /* The kind of table      */
            (VOID **) &ids_nt->idsu.nt.rel_table,  /* -> to address of table */
                      &ids_nt->idsu.nt.entry_count,/* -> count in the table  */
                      &ids_nt->idsu.nt.max_count   /* -> maximum for table   */
                       )
                != TRUE) {
    return(MC_OUT_OF_MEMORY);
    }

/* insert value into relationship table at the end */
rt = ids_nt->idsu.nt.rel_table;
rt[ids_nt->idsu.nt.entry_count - 1].rel_obj = rel_nt;
rt[ids_nt->idsu.nt.entry_count - 1].tar_obj = target;

/* if (target is any kind of Terminal) */
if (target->flavor >= I_T_OBJ_snumber && target->flavor <= I_T_OBJ_string) {
    target->idsu.t.ref_count += 1;    /* increment reference counts */
    }
else if (target->flavor == I_NT_OBJECT) {
    target->idsu.nt.ref_count += 1;   /* increment reference counts */
    }

return (MC_SUCCESS);
}

/* I_Add_Table_Entry-Guarantees Size of Object's Table can hold 1 More Entry */
/* I_Add_Table_Entry-Guarantees Size of Object's Table can hold 1 More Entry */
/* I_Add_Table_Entry-Guarantees Size of Object's Table can hold 1 More Entry */

static BOOL
I_Add_Table_Entry (ids_table_type, table_ptr, counter_ptr, maximum_count)

IDS_TYPE       ids_table_type; /* Type of obj. whose table is being expanded */
VOID         **table_ptr;      /* Address of pointer to table being expanded */
unsigned short *counter_ptr;   /* Address of entry counter for table         */
unsigned short *maximum_count; /* Address of Maximum (count) for table       */


/*
INPUTS:

    "ids_table_type" is an indicator of the type structure whose table is
    being expanded: Non-Terminal (I_NT_OBJECT_*) or Slice (I_SLICE) only.

    "table_ptr" is the address of the pointer to the table being expanded.
    The address of the pointer must be passed because we may have to
    reallocate it.

    "counter_ptr" is the address of the counter of the number of active
    entries in the table.  This value will be incremented by one.

    "maximum_count" is the address of the cell indicating the maximum number
    of entries in the table now.  This value may be reset if the table is
    resized larger.


OUTPUTS:

    The function returns TRUE if the table is big enough to handle
    one more entry and the counter (pointed to by the counter_ptr argument)
    is incremented by one.

    The function returns FALSE if insufficient memory is available to expand
    the table if expansion is needed.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to insert another element into one of two kinds
        of tables that make up part of the MIR Internal Data Structures.

    Purpose:
        This function insures that any table has at least enough memory
        in it for one more entry, and it assumes that exactly one is
        being added (and indeed bumps the table entry counter by one
        on behalf of the caller on a successful return).

ACTION SYNOPSIS OR PSEUDOCODE:

    <select the base size according to table type>
    if (table has nothing in it yet (table_ptr==>NULL))
        <allocate the base increment size>
        <zero the storage in it>
        <return the address of the newly allocated space to the caller>
        <set the maximum count>
        <set table counter to 1>
    else
        if (current entry count equals maximum (ie. no room))
            <compute allocation size required to hold current entry count
             plus one>
            <allocate larger space>
            if (allocation failed)
                <return FALSE>
            <copy all current entries to new space>
            <deallocate the old space>
            <set the maximum count>
            <return the address of the newly allocated space to the caller>
        <increment table counter by 1>
     <return TRUE>


OTHER THINGS TO KNOW:

    We start with a different base size for each possible table
type that we can expand.  We allocate increasingly larger amounts of entries
(by doubling) each time more is needed.

Historical Note:

    In initial versions of this function, none of the MIR objects had a
cell in them that recorded the actual amount of storage in use for their
tables; only the 'current' usage was recorded ("entry_count").  Since the
storage allocation was always performed for these MIR object tables by
this function according to a fixed algorithm of "doubling" starting with
a size of "1", it was always possible to infer whether any storage was left
given a particular value for the entry count.  (Clever, eh?).

    With the advent of "mir_internal.c" and the ability to "read-in" (in
essence) MIR objects whose 'final' size was known (and unlikely to change
for the most part), it was deemed useful to allocate a MIR Object *and* it's
final (with luck) table size in one fell swoop.  This is done by code in
"I_Create_IDS()" in this module without calling this function.  Consequently,
should one of those MIR Object's be passed to this function for expansion,
there is no way for this function to 'know' whether there is any space left
given an entry count (typically no space is available) because the table was
allocated 'exactly' by I_Create_IDS().

    Therefore the Slice and Non-Terminal MIR Object IDS definitions were
modified to split the entry-count integer into two 'short' integers (since
a table size of 65535 seems relatively ample) and use the other short to
indicate the true amount of storage available to the table ("max_count").

    This version still uses the "doubling" method (starting from "1") to
decide how much additional storage to allocate when it is needed.  Note
that the logic allows an "initial count" to actually be *different* from "1"
(to save resizing in cases where we'll almost certainly double twice or three
times before being large enough), but the "basis" for doubling when resizing
is needed is still logically "1" (ie, on a "re-size", the new size will always
be one from the sequence 2,4,8,16,32.... even if the actual initial size did
not come from that sequence).
*/

/*
|  This macro returns in "fullsize" the total size required to hold "count"
|  entries in an array whose element size is "entry_size", when the initial
|  count of entries for the array is "init_count".  Also, returned in
|  "final_count" is the maximum number of entries storable in "fullsize".
*/
#define ALLOC_SIZE(fullsize, final_count, entry_size, init_count, count)\
{                                                                       \
if (count <= init_count) {                                              \
    fullsize = entry_size*init_count;                                   \
    final_count = init_count;                                           \
    }                                                                   \
else  /* Double till we get big enough */                               \
    for ( (final_count = 1, fullsize = entry_size);                     \
         final_count < count;                                           \
         (final_count = final_count << 1, fullsize = fullsize << 1)     \
         );                                                             \
}
    
{
int     entrybase_size;  /* Size of one entry in the selected IDS type table */
int     newtotal_size;   /* Size required for new table w/one more entry     */
VOID    *new_storage;    /* --> New expanded storage                         */

unsigned short  initial_count;   /* How many entries in first allocation     */
unsigned short  new_maximum;     /* How many entries in next allocation      */


/* select the base size according to table type */
switch (ids_table_type) {

    case I_SLICE:
        entrybase_size = sizeof(struct SES);
        initial_count = 1;
        break;

    /*
    | These Non-Terminals are used to represent most of what is compiled.
    | Analysis of large compilations reveals a bulge in the distribution
    | of Relationship Table sizes for these kinds of Non-Terminals around
    | a table size of six to eight entries.  "initial_count" is set accordingly
    | to avoid most "resize" operations for the majority of such Non-Terminals
    | at the expense of a little space.
    */
    case I_NT_OBJECT:
        entrybase_size = sizeof(struct RES);
        initial_count = 8;
        break;

    /*
    | These Non-Terminals are used to represent only MIR Relationships.
    | As of V2.0, the only thing that appears in the Relationship Table of
    | such an object is a relationship giving the name of the MIR Object.
    | Consquently "initial_count" is set accordingly.
    */
    case I_NT_OBJECT_Rel:
        entrybase_size = sizeof(struct RES);
        initial_count = 1;
        break;

    /*
    | These Non-Terminals are used to represent Dataconstructs
    | As of V2.0, five pieces of information typically reside in such an
    | object's relationship table. Consquently "initial_count" is set
    | accordingly.
    */
    case I_NT_OBJECT_DC:
        entrybase_size = sizeof(struct RES);
        initial_count = 5;
        break;

    default:
        printf (
                MP(mp200,"mirc - Internal Error, Improper call to I_Add_Table_Entry, bad ids_table_type = %d \n"),
                (int) ids_table_type);
        return (FALSE);
    }

/* if (table has nothing in it yet (table_ptr==>NULL)) */
if (*table_ptr == NULL) {

    /* allocate the base increment size */
    if ( (new_storage = (VOID *) malloc (entrybase_size*initial_count))
        == NULL) {
        return (FALSE);
        }

    /* zero the storage in it */
    bzero(new_storage,(entrybase_size*initial_count));

    /* return the address of the newly allocated space to the caller */
    *table_ptr = new_storage;

    /* set table maximum */
    *maximum_count = initial_count;

    /* set table counter to 1 */
    *counter_ptr = 1;
    }
else {
    /* if (current entry count equals maximum (ie. no room)) */
    if (*counter_ptr >= *maximum_count) {

        /* compute allocation size required to hold current entry count + 1  */
        ALLOC_SIZE(newtotal_size,  /* OUT: New size to allocate              */
                   new_maximum,    /* OUT: New maximum count for new size    */
                   entrybase_size, /* IN:  How big in bytes each entry is    */
                   initial_count,  /* IN:  How many we used 1st time around  */
                   (*counter_ptr+1)/* IN:  Number we want to be able to hold */
                   );

        /* if (allocation of larger space failed) */
        if ( (new_storage = (VOID *) malloc (newtotal_size)) == NULL) {
            return (FALSE);
            }

        /* copy all current entries to new space */
        bcopy(*table_ptr, new_storage, (*counter_ptr*entrybase_size));

        /* deallocate the old space */
        free(*table_ptr);

        /* set table maximum */
        *maximum_count = new_maximum;

        /* return the address of the newly allocated space to the caller */
        *table_ptr = new_storage;
        }

    /* increment table counter by 1 */
    *counter_ptr += 1;
    }

return (TRUE);
}

/* I_Create_IDS_SNUMBER - Create a Terminal Object that is a Signed Number */
/* I_Create_IDS_SNUMBER - Create a Terminal Object that is a Signed Number */
/* I_Create_IDS_SNUMBER - Create a Terminal Object that is a Signed Number */

IDS *
I_Create_IDS_SNUMBER (term_number)

int        term_number;    /* Terminal Object: The number */

/*
INPUTS:

    "term_number" is the signed number to be "created" as a terminal object in
    the MIR.

OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_snumber", or NULL if storage could not be allocated.

    If the pointer is returned, then the number supplied as the argument to
    this function is inserted into the structure.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to create a Terminal of form "signed-number"
        containing a certain value.

    Purpose:
        This function searches for a previously created Terminal of the
        same value.  If found it returns a pointer to the previously created
        Terminal.  If not found, it allocates storage to contain the
        Terminal, links it into the list of terminals and returns a pointer
        to the structure.

ACTION SYNOPSIS OR PSEUDOCODE:

    <obtain the address of the pointer to the first Signed Number terminal on
     the "existing" list>

    while (the pointer is non-null)
        if (the current entry is less than desired)
            <step the pointer to the "next" entry of current entry>
            <continue>
        else if (the current entry is exactly as desired)
            <return the current pointer>
        else
            <break>

    if (the free list for this flavor is NOT empty)
        <unhook the next free from the top of the list>
    else
        <allocate space for a Terminal IDS>
    <insert the desired number into it>
    <insert the new entry at the current entry's position in "existing" list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    The effort to store a value only once in the final MIR starts here as we
create only one value of an integer in Terminal form in the Intermediate
representation.  This is done by linearly searching a singly-linked list
of numbers until we find either the one that the caller is seeking to
create or the position where it should be placed into the "existing" list.

NOTE that we could use exactly the same binary search technique employed for
Strings, but I suspect it would actually wind up making the compiler *run
slower*!  This is because I suspect that nine times out of ten, the "next"
number the compiler is going to deal with is going to be very small (like
0, 1, 2 or 3) since these are popular codes for DIRECTIVE RESPONSES, EXCEPTIONS
and ARGUMENTS!  Consequently we'll find what we want sooner with a linear
search that starts with the small numbers.

    This function is identical in function to I_Create_IDS_UNUMBER except
that it creates a Terminal to hold a Signed instead of Unsigned number.

    With V1.98, we attempt to find a free IDS of the right type from the
free-list created and maintained by I_Reclaim_IDS().

*/

{
IDS     **scan;     /* -->> next IDS structure under consideration  */
IDS     *new_ids;   /* --> structure we allocate space for          */
IDS     *previous;  /* Previous IDS (needed for doubly-linked list) */


/*
|  Obtain the address of the pointer to the first Unsigned Number terminal
|  on the "existing" list.
*/
scan = &(ic->flavors[I_T_OBJ_snumber]);
previous = NULL;

/* while (the pointer is non-null) */
while (*scan != NULL) {

    /* if (the current entry is less than desired) */
    if ((*scan)->idsu.t.t_v.snumber.value < term_number) {
        /* step the pointer to the "next" entry of current entry */
        previous = (*scan);
        scan = &((*scan)->next);
        continue;
        }

    /* if (the current entry is exactly as desired) */
    else if ((*scan)->idsu.t.t_v.snumber.value == term_number) {
        return (*scan);
        }
    else {
        break;
        }
    }

/* if (the free list for this flavor is NOT empty) */
if (ic->top_of_free[I_T_OBJ_snumber] != NULL) {
    /*
    | unhook the next free from the top of the list
    */
    new_ids = ic->top_of_free[I_T_OBJ_snumber];         /* Grab the top */

    /* If the top was 'the last free' . . .*/
    if ((ic->top_of_free[I_T_OBJ_snumber] = new_ids->next) == NULL) {

        /* Make the 'bottom' pointer agree */
        ic->last_of_free[I_T_OBJ_snumber] = NULL;
        }
    }
else {
    /* allocate space for a Terminal IDS */
    if ( (new_ids = (IDS *) malloc(
          sizeof(struct ids_tag)-sizeof(union varieties)+sizeof(struct term)
                                   )) == NULL) {
        return (NULL);
        }
    }

/* insert the desired number into it */
new_ids->idsu.t.t_v.snumber.value = term_number;

/* insert the new entry at the current entry's position in "existing" list  */
new_ids->prev = previous;       /* New entry points to previous: wherever   */
new_ids->next = *scan;          /* New entry points forw @ existing entry   */
*scan = new_ids;                /* Previous forward ptr points to new entry */
if (new_ids->next != NULL)      /* If there is a 'next'                     */
    new_ids->next->prev = new_ids;  /* ...make it point back at new entry   */
else    /* we just inserted a new 'last' entry into the list */
    ic->last_of_flavor[I_T_OBJ_snumber] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_snumber;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;

return(new_ids);
}

/* I_Create_IDS_UNUMBER - Create a Terminal Object that is a Unsigned Number */
/* I_Create_IDS_UNUMBER - Create a Terminal Object that is a Unsigned Number */
/* I_Create_IDS_UNUMBER - Create a Terminal Object that is a Unsigned Number */

IDS *
I_Create_IDS_UNUMBER (term_number)

unsigned int   term_number;    /* Terminal Object: The number */

/*
INPUTS:

    "term_number" is the unsigned number to be "created" as a terminal object
    in the MIR.

OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_unumber", or NULL if storage could not be allocated.

    If the pointer is returned, then the number supplied as the argument to
    this function is inserted into the structure.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to create a Terminal of form "unsigned-number"
        containing a certain value.

    Purpose:
        This function searches for a previously created Terminal of the
        same value.  If found it returns a pointer to the previously created
        Terminal.  If not found, it allocates storage to contain the
        Terminal, links it into the list of terminals and returns a pointer
        to the structure.

ACTION SYNOPSIS OR PSEUDOCODE:

    <obtain the address of the pointer to the first Unsigned Number terminal on
     the "existing" list>
    while (the pointer is non-null)
        if (the current entry is less than desired)
            <step the pointer to the "next" entry of current entry>
            <continue>
        else if (the current entry is exactly as desired)
            <return the current pointer>
        else
            <break>

    if (the free list for this flavor is NOT empty)
        <unhook the next free from the top of the list>
    else
        <allocate space for a Terminal IDS>
    <insert the desired number into it>
    <insert the new entry at the current entry's position in "existing" list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    The effort to store a value only once in the final MIR starts here as we
create only one value of an integer in Terminal form in the Intermediate
representation.  This is done by linearly searching a singly-linked list
of numbers until we find either the one that the caller is seeking to
create or the position where it should be placed into the "existing" list.

    This function is identical in function to I_Create_IDS_SNUMBER except
that it creates a Terminal to hold an Unsigned instead of Signed number.

    There are typically few or zero "unsigned" numbers in any compilation,
since the compiler uses "unsigned" numbers only for really huge numbers
(greater than the largest signed number).

    With V1.98, we attempt to find a free IDS of the right type from the
free-list created and maintained by I_Reclaim_IDS().

*/

{
IDS     **scan;     /* -->> next IDS structure under consideration  */
IDS     *new_ids;   /* --> structure we allocate space for          */
IDS     *previous;  /* Previous IDS (needed for doubly-linked list) */


/*
|  Obtain the address of the pointer to the first Unsigned Number terminal
|  on the "existing" list.
*/
scan = &(ic->flavors[I_T_OBJ_unumber]);
previous = NULL;

/* while (the pointer is non-null) */
while (*scan != NULL)  {

    /* if (the current entry is less than desired) */
    if ((*scan)->idsu.t.t_v.unumber.value < term_number) {
        /* step the pointer to the "next" entry of current entry */
        previous = (*scan);
        scan = &((*scan)->next);
        continue;
        }

    /* if (the current entry is exactly as desired) */
    else if ((*scan)->idsu.t.t_v.unumber.value == term_number) {
        return (*scan);
        }
    else {
        break;
        }
    }

/* if (the free list for this flavor is NOT empty) */
if (ic->top_of_free[I_T_OBJ_unumber] != NULL) {
    /*
    | unhook the next free from the top of the list
    */
    new_ids = ic->top_of_free[I_T_OBJ_unumber];         /* Grab the top */

    /* If the top was 'the last free' . . .*/
    if ((ic->top_of_free[I_T_OBJ_unumber] = new_ids->next) == NULL) {

        /* Make the 'bottom' pointer agree */
        ic->last_of_free[I_T_OBJ_unumber] = NULL;
        }
    }
else {
    /* allocate space for a Terminal IDS */
    if ( (new_ids = (IDS *) malloc(
        sizeof(struct ids_tag)-sizeof(union varieties)+sizeof(struct term)
                                   )) == NULL) {
        return (NULL);
        }
    }

/* insert the desired number into it */
new_ids->idsu.t.t_v.unumber.value = term_number;

/* insert the new entry at the current entry's position in "existing" list  */
new_ids->prev = previous;       /* New entry points to previous: wherever   */
new_ids->next = *scan;          /* New entry points forw @ existing entry   */
*scan = new_ids;                /* Previous forward ptr points to new entry */
if (new_ids->next != NULL)      /* If there is a 'next'                     */
    new_ids->next->prev = new_ids;  /* ...make it point back at new entry   */
else    /* we just inserted a new 'last' entry into the list */
    ic->last_of_flavor[I_T_OBJ_unumber] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_unumber;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;

return(new_ids);
}

/* I_Create_IDS_STRING - Create a Terminal Object that is a String */
/* I_Create_IDS_STRING - Create a Terminal Object that is a String */
/* I_Create_IDS_STRING - Create a Terminal Object that is a String */

IDS *
I_Create_IDS_STRING (term_string)

char    *term_string;       /* Terminal Object: The string */

/*
INPUTS:

    "term_string" is the string to be "created" as a terminal object in
    the MIR.

OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_string", or NULL if storage could not be allocated.

    If the pointer is returned, then the string supplied as the argument to
    this function is inserted into the structure.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to create a Terminal of form "string" containing
        a certain value.

    Purpose:
        This function searches for a previously created Terminal of the
        same value.  If found it returns a pointer to the previously created
        Terminal.  If not found, it allocates storage to contain the
        Terminal, links it into the list of terminals and returns a pointer
        to the structure.

ACTION SYNOPSIS OR PSEUDOCODE:

    <obtain the address of the pointer to the first String terminal on the
     "existing" list: scan>

    if (list is not empty and cross-reference array exists)
        <perform binary search to find closest & lowest entry to desired>
        <set-up scan to point to the next IDS to be considered by linear scan>

    while (the pointer is non-null)
        if (the current entry is less than desired)
            <step the pointer to the "next" entry of current entry>
            <continue>
        else if (the current entry is exactly as desired)
            <return the current pointer>
        else
            <break>

    if (the free list for this flavor is NOT empty)
        <unhook the next free from the top of the list>
    else
        <allocate space for a Terminal IDS>
    <copy the string into it>
    <insert the new entry at the current entry's position in "existing" list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    The effort to store a value only once in the final MIR starts here as we
create only one copy of a string in Terminal form in the Intermediate
representation.

This was done by linearly searching the:

             "bec->i_context.flavors[I_T_OBJ_string]"

list of strings until we found either the one that the caller is seeking to
create or the position where it should be placed into the "existing" list.

    With V2.00, *for strings ONLY* (this function) we perform this search
by initally using a binary search on a "cross-reference" array to get 'close'
to the entry we want (rather than doing a full linear search from the top of
the list).  Once we're 'close' we revert to a linear search.  The list being
searched is still a linked-list of string IDS's ordered by "increasing" string
value.

    The "cross-reference" array is created initially either by code in
"mir_internal.c" (when a binary MIR database file is loaded) or by code
in "mir_backend.c" (function "mirc_init_backend()").

    When the "cross-reference" array is created by "mir_internal.c", there
is an entry in the array for *every* string IDS that was created when the
binary MIR database file was loaded.  In "mir_internal.c", this "xr" (cross
reference) array is used to convert an "external" MIR Address Space address
(of the string) to the internal heap address of the IDS that represents that
string. (There's an "xr" array created for each kind of MIR Object that
"mir_internal.c" code reads in. . . all of the "xr" arrays have their storage
released when "mir_internal.c" code is finished doing the cross-referencing
*except* for the "xr" array that describes strings.... this is because it is
going to be extremely useful in this function for fast lookups).

    When the "cross-reference" array is created by code in
"mirc_init_backend()", what's really going on is that a "fake xr" array is
created that looks very similar to the one that "mir_internal.c" would
create.  In this case however, what "mirc_init_backend()" does is create
a string of length "1-character" for each printable character according
to the C-predicate "isprint()".  One IDS whose value is one character is
created for each "isprint() == TRUE" character.  The address of each IDS
is entered into the "xr" string array in successive positions.  Each of
these "Seed" strings forms the top of a sublist of entries that all begin
with that character.  In this case, when this function is entered to search
for a particular string, these seeds will serve as 'search points' that the
binary search can work on.  If it turns out that the string sought happens
to exactly match the desired string, then the seed will become appropriated
for use as a 'real' IDS string (and when it's address is inserted as the target
of a MIR relationship in a Relationship Table, it's reference count will be
incremented to 1).  Otherwise, these Seed strings have a reference count of 0,
and merely serve as the aforementioned 'search points' for the binary search.
Code in "mir_external.c" will weed these unreferenced 1-character strings out
of the final MIR database output (because their reference counts will be zero).

    In the case of these Seed Strings' "xr" array, there is no 'original'
External address to be placed in the xr array element for each string.  This
is of no consequence, because this function merely uses each entry to access
the IDS for a string so that it's value may be compared (unlike the
"mir_internal.c" code that compares the External Address in an entry for a
match).

    The important aspect of the "xr" array *for this function* is that it
provides a set of pointers to strings *that are ordered* in increasing value.

    The list of string IDS's maintained out of:

             "bec->i_context.flavors[I_T_OBJ_string]"

is the full ordered list of MIR Objects that are strings (in IDS form).  They
are ordered in increasing value.  The "xr" array contains entries that
point *into* the "bec->i_context.flavors[I_T_OBJ_string]" list (allowing the
binary search to shortcut a linear-search of this list).

    Note that this function will be called by "mirc_init_backend()" (to create
the Seed Strings) before the "xr" array for strings is setup.  There is a
special check in this function to avoid trying to reference the array before
it is setup.

    With V1.98, we attempt to find a free IDS of the right type from the
free-list created and maintained by I_Reclaim_IDS().  We'll re-use the string
space too if it is long enough, otherwise it is free()ed and we malloc more.

*/

{
IDS             **scan;     /* -->> next IDS structure under consideration  */
IDS             *new_ids;   /* --> structure we allocate space for          */
IDS             *ids_temp;  /* --> structure we're going to start scan on   */
IDS             *previous;  /* Previous IDS (needed for doubly-linked list) */
int             cmp_code;   /* Result of string comparison                  */
ext_int_xref    *xr;        /* --> String "Cross-Reference" array           */
int             xr_count;   /* Count of # of entries in "xr[]"              */
int             i;          /* Handy-Dandy General purpose index            */
int             hi,lo;      /* Boundary posts for binary search             */


/*
| Obtain the address of the pointer to the first String terminal on the 
| "existing" list.
*/
scan = &(ic->flavors[I_T_OBJ_string]);
previous = NULL;

#if 0
/* ------- */
if ((xr = ic->xref[I_T_OBJ_string]) != NULL) {
    ids_temp = *scan;
    printf("Entering I_Create_IDS_STRING with \"%s\", currently:\n",term_string);
    while (ids_temp != NULL) {
        printf("%x: %s  Prev: %x  Next: %x\n",
               ids_temp,
               ids_temp->idsu.t.t_v.string.str,
               ids_temp->prev,
               ids_temp->next);
        ids_temp = ids_temp->next;
        }
    }
#endif

/* if (list is not empty and cross-reference array exists) */
if (*scan != NULL
    && (xr = ic->xref[I_T_OBJ_string]) != NULL) {

    /*
    | ================== START BINARY SEARCH ========================
    | Perform binary search to find closest & lowest entry to desired
    */
    xr_count = ic->xref_count[I_T_OBJ_string];

    hi = xr_count - 1;  /* Set Hi boundary post    */
    lo = 0;             /* Set Lo boundary post    */
    cmp_code = 1;       /* (in remote event the xr[] has no entries) */
    i = 0;              /* (in remote event the xr[] has no entries) */

    while (lo <= hi) {
        i = (lo+hi)/2;     /* Grab the (new) comparison point */

        /* Compare:       ------------NEXT ENTRY----------- to SOUGHT STRING */
        cmp_code = strcmp(xr[i].int_add->idsu.t.t_v.string.str, term_string);

        if (cmp_code < 0)
            lo = i + 1;       /* Tested Entry is Too Low: Raise lower bound */
        else if (cmp_code > 0)
            hi = i - 1;       /* Tested Entry is Too High: Lower upper bound */
        else
            break;
        }
    /*
    | ==================== END BINARY SEARCH ========================
    | We're interested in setting "scan" so that it contains the address
    | of a pointer to the next IDS whose value should be considered by the
    | linear search loop immediately below.  We want the compare in the loop
    | below to discover either:
    |
    |   * An exact match,
    |   * A match where the tested entry is too low, and the linear search must
    |     continue with the next entry down the list.
    |
    | 1) If cmp_code is 0,   then i specifies the matching IDS via xr[i], and
    |                        so we can start with it
    | 2) If cmp_code is < 0, then i specifies a "low" IDS that is still less
    |                        than the desired spot, so we can start with it
    | 3) If cmp_code is > 0, then i specifies an IDS that is too "high", and
    |                        we've got to use the IDS that corresponds to the
    |                        (i-1)th entry in xr[] to start the linear search.
    |                        (If i-1 < 0, we simply start at the top of the
    |                         entire list).
    |
    | We do the same thing for cases 1) & 2), for 3) we have to back off 1,
    | check for 'falling off' the beginning of the list.
    |
    | To set up "scan", we have to load "scan" with the address *of a pointer*
    | that points to the IDS we want the compare to begin with!  In general,
    | this means taking the IDS pointed to by "i" (or modified "i") and
    | walking back (up) the "prev" pointer to the preceding IDS and pointing
    | "scan" at the "next" cell in the preceding IDS.
    |
    |set-up scan to point to the next IDS to be considered by linear scan
    */
    if (cmp_code <= 0) {        /* Case 1) and 2) */

        /* This is the IDS we want to (re) compare in linear scan below */
        ids_temp = xr[i].int_add;

        /* If there is something on the list before this one. . . */
        if (ids_temp->prev != NULL) {
            /* Set scan to address of "next" cell in previous IDS */
            scan = &(ids_temp->prev->next);
            previous = ids_temp->prev;
            }
        /* else "scan" already points at the very top of the string IDS list */
        }
    else {      /* cmp_code > 0: case 3) */

        /* if there is an entry prior to this one in the array */
        if (i > 0) {

            /* Grab the IDS for the prior entry */
            ids_temp = xr[i-1].int_add;

            /* If there is something on the list before this one. . . */
            if (ids_temp->prev != NULL) {
                /* Set scan to address of "next" cell in previous IDS */
                scan = &(ids_temp->prev->next);
                previous = ids_temp->prev;
#if 0
                printf("\nBacking up one . . . \n");
#endif
                }
            /* else "scan" already points at the top of the string IDS list */
            }
        /* else "scan" already points at the top of the string IDS list */
        }
#if 0
printf("Debug: Starting scan early at: \n");
printf("\"%s\" for:\n",(*scan)->idsu.t.t_v.string.str);
printf("\"%s\"\n",term_string);
#endif
    }
/*
| Now revert to linear scan
*/

/* while (the pointer is non-null) */
while (*scan != NULL) {

    /* if (the current entry is less than desired) */
    if ((cmp_code = strcmp((*scan)->idsu.t.t_v.string.str, term_string))
        < 0) {
        /* step the pointer to the "next" entry of current entry */
        previous = (*scan);
        scan = &((*scan)->next);
        continue;
        }

    /* if (the current entry is exactly as desired) */
    else if (cmp_code == 0) {
        return (*scan);
        }
    else {
        break;
        }
    }

/*
| If we fall out of the loop above, there is no string of this value already
| compiled.  We have to make an IDS to hold this string.
*/

/* if (the free list for this flavor is NOT empty) */
if (ic->top_of_free[I_T_OBJ_string] != NULL) {
    /*
    | unhook the next free from the top of the list
    */
    new_ids = ic->top_of_free[I_T_OBJ_string];         /* Grab the top */

    /* If the top was 'the last free' . . .*/
    if ((ic->top_of_free[I_T_OBJ_string] = new_ids->next) == NULL) {

        /* Make the 'bottom' pointer agree */
        ic->last_of_free[I_T_OBJ_string] = NULL;
        }
    }
else {
    /* allocate space for a Terminal IDS */
    if ( (new_ids = (IDS *) malloc(sizeof(IDS))) == NULL) {
        return (NULL);
        }

    /* Show 'no-storage to re-use' */
    new_ids->idsu.t.t_v.string.len = 0;
    new_ids->idsu.t.t_v.string.str = NULL;
    }

/*
|  insert the desired string into it
|
|  Based on the value of "len", we decide whether there is enough storage
|  already present to store the string.  If so, we simply re-use it, if
|  not, we free any present and malloc more.
|
|  Note: The value of "..idsu.t.t_v.string.len" does *not* reflect the storage
|  needed to store the null byte at the end of the string.
*/
i = strlen(term_string);        /* Grab length we need to store */

/* if we don't have room . . .  ( "=" takes care of needed null byte) */
if (i >= new_ids->idsu.t.t_v.string.len) {

    /* Free any old storage */
    if (new_ids->idsu.t.t_v.string.str != NULL) {
        free(new_ids->idsu.t.t_v.string.str);
        }

    /* Grab new storage */
    if ((new_ids->idsu.t.t_v.string.str = (char *) malloc(i+1)) == NULL) {
        return (NULL);
        }
    }

/* Record new length */
new_ids->idsu.t.t_v.string.len = i;

/* copy the string null terminated into the storage */
strcpy(new_ids->idsu.t.t_v.string.str, term_string);

/* insert the new entry at the current entry's position in "existing" list  */
new_ids->prev = previous;       /* New entry points to previous: wherever   */
new_ids->next = *scan;          /* New entry points forw @ existing entry   */
*scan = new_ids;                /* Previous forward ptr points to new entry */
if (new_ids->next != NULL)      /* If there is a 'next'                     */
    new_ids->next->prev = new_ids;  /* ...make it point back at new entry   */
else    /* we just inserted a new 'last' entry into the list */
    ic->last_of_flavor[I_T_OBJ_string] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_string;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;
return(new_ids);
}

/* I_AddNew_IDS_SNUMBER - Add a New Terminal Object that is a Signed Number */
/* I_AddNew_IDS_SNUMBER - Add a New Terminal Object that is a Signed Number */
/* I_AddNew_IDS_SNUMBER - Add a New Terminal Object that is a Signed Number */

IDS *
I_AddNew_IDS_SNUMBER (term_number)

int        term_number;    /* Terminal Object: The number */

/*
INPUTS:

    "term_number" is the signed number to be "Add New" as a terminal object in
    the MIR *to the end* of the existing sorted list of numbers.

    The caller guarantees that this number is different and larger than the
    last number created by a call to this function.

OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_snumber", or NULL if storage could not be allocated.

    If the pointer is returned, then the number supplied as the argument to
    this function is inserted into the structure.
    
BIRD'S EYE VIEW:
    Context:
        Logic in "mir_internal.c" is loading a binary MIR database file
        and it is in the process of loading Signed Numbers.  From the
        binary MIR database file these numbers are guaranteed to be unique
        and ordered.  This means we can take advantage of these facts by
        not searching the previously created list and simply inserting the
        new value at the end of the existing list.

    Purpose:
        This function allocates storage to contain the new Terminal, links it
        into the list of terminals and returns a pointer to the structure.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set up "previous" pointer so insertion will be correct>
    <allocate space for a Terminal IDS>
    <insert the desired number into it>
    <insert the new entry at the end of the "existing" list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    This function is used for speed by the "mir_internal.c" functions
    that need to create IDSs of this type.  Note that once compilation
    of an ASCII MSL begins, it is inappropriate to use this function, use
    I_Create_IDS_snumber() instead.
*/

{
IDS     *new_ids;   /* --> structure we allocate space for          */


/* allocate space for a Terminal IDS */
if ( (new_ids = (IDS *) malloc(
      sizeof(struct ids_tag)-sizeof(union varieties)+sizeof(struct term)
                               )) == NULL) {
    return (NULL);
    }

/* insert the desired number into it */
new_ids->idsu.t.t_v.snumber.value = term_number;

/*
| insert the new entry at the end of the existing list
|
| New entry points to previous: wherever
| If there was no 'last' entry ...
*/
if ((new_ids->prev = ic->last_of_flavor[I_T_OBJ_snumber]) == NULL) {
    ic->flavors[I_T_OBJ_snumber] = new_ids;  /* Record Top-of-List */
    }
else {  /* Make old 'last' entry point forward to new entry */
    new_ids->prev->next = new_ids;
    }

/* New entry is at the end of the list */
new_ids->next = NULL;           

/* New End-Of-List */
ic->last_of_flavor[I_T_OBJ_snumber] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_snumber;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;

return(new_ids);
}

/* I_AddNew_IDS_UNUMBER - Add a New Terminal Object: an Unsigned Number */
/* I_AddNew_IDS_UNUMBER - Add a New Terminal Object: an Unsigned Number */
/* I_AddNew_IDS_UNUMBER - Add a New Terminal Object: an Unsigned Number */

IDS *
I_AddNew_IDS_UNUMBER (term_number)

unsigned int   term_number;    /* Terminal Object: The number */

/*
INPUTS:

    "term_number" is the signed number to be "Add New" as a terminal object in
    the MIR *to the end* of the existing sorted list of numbers.

    The caller guarantees that this number is different and larger than the
    last number created by a call to this function.


OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_unumber", or NULL if storage could not be allocated.

    If the pointer is returned, then the number supplied as the argument to
    this function is inserted into the structure.
    

BIRD'S EYE VIEW:
    Context:
        Logic in "mir_internal.c" is loading a binary MIR database file
        and it is in the process of loading Unsigned Numbers.  From the
        binary MIR database file these numbers are guaranteed to be unique
        and ordered.  This means we can take advantage of these facts by
        not searching the previously created list and simply inserting the
        new value at the end of the existing list.

    Purpose:
        This function allocates storage to contain the new Terminal, links it
        into the list of terminals and returns a pointer to the structure.


ACTION SYNOPSIS OR PSEUDOCODE:

    <allocate space for a Terminal IDS>
    <insert the desired number into it>
    <insert the new entry at the end of the "existing" list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    This function is used for speed by the "mir_internal.c" functions
    that need to create IDSs of this type.  Note that once compilation
    of an ASCII MSL begins, it is inappropriate to use this function, use
    I_Create_IDS_unumber() instead.
*/

{
IDS     *new_ids;   /* --> structure we allocate space for          */


/* allocate space for a Terminal IDS */
if ( (new_ids = (IDS *) malloc(
      sizeof(struct ids_tag)-sizeof(union varieties)+sizeof(struct term)
                               )) == NULL) {
    return (NULL);
    }

/* insert the desired number into it */
new_ids->idsu.t.t_v.unumber.value = term_number;

/*
| insert the new entry at the end of the existing list
|
| New entry points to previous: wherever
| If there was no 'last' entry ...
*/
if ((new_ids->prev = ic->last_of_flavor[I_T_OBJ_unumber]) == NULL) {
    ic->flavors[I_T_OBJ_unumber] = new_ids;  /* Record Top-of-List */
    }
else {  /* Make old 'last' entry point forward to new entry */
    new_ids->prev->next = new_ids;
    }

/* New entry is at the end of the list */
new_ids->next = NULL;           

/* New End-Of-List */
ic->last_of_flavor[I_T_OBJ_unumber] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_unumber;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;

return(new_ids);
}

/* I_AddNew_IDS_STRING - Add a New Terminal Object that is a String */
/* I_AddNew_IDS_STRING - Add a New Terminal Object that is a String */
/* I_AddNew_IDS_STRING - Add a New Terminal Object that is a String */

IDS *
I_AddNew_IDS_STRING (term_string)

char    *term_string;       /* Terminal Object: The string */

/*
INPUTS:

    "term_string" is the string to be "created" as a terminal object in
    the MIR *to the end* of the existing sorted list of strings.

    The caller guarantees that this string is different and 'larger'
    lexicographically than the last string created by a call to this function.
    

OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "I_T_OBJ_string", or NULL if storage could not be allocated.

    If the pointer is returned, then the string supplied as the argument to
    this function is inserted into the structure.
    
BIRD'S EYE VIEW:
    Context:
        Logic in "mir_internal.c" is loading a binary MIR database file
        and it is in the process of loading Strings.  From the
        binary MIR database file these strings are guaranteed to be unique
        and ordered.  This means we can take advantage of these facts by
        not searching the previously created list and simply inserting the
        new value at the end of the existing list.

    Purpose:
        This function allocates storage to contain the new Terminal, links it
        into the end of the list of strings and returns a pointer to the
        structure.

ACTION SYNOPSIS OR PSEUDOCODE:


    <allocate space for a Terminal IDS>
    <copy the string into it>
    <insert the new entry at the end of the existing string list>
    <Show initial reference as zero>

    <return the pointer to the new entry>


OTHER THINGS TO KNOW:

    This function is used for speed by the "mir_internal.c" functions
    that need to create IDSs of this type.  Note that once compilation
    of an ASCII MSL begins, it is inappropriate to use this function, use
    I_Create_IDS_string() instead.

*/

{
int             str_len;    /* Length of inbound string            */
IDS             *new_ids;   /* --> structure we allocate space for */


/* allocate space for a Terminal IDS */
if ( (new_ids = (IDS *) malloc(
      sizeof(struct ids_tag)-sizeof(union varieties)+sizeof(struct term)
                               )) == NULL) {
    return (NULL);
    }

/*
|  insert the desired string into it
*/
str_len = strlen(term_string);        /* Grab length we need to store */

/* Grab new storage */
if ((new_ids->idsu.t.t_v.string.str = (char *) malloc(str_len+1)) == NULL) {
    return (NULL);
    }

/* Record new length */
new_ids->idsu.t.t_v.string.len = str_len;

/* copy the string null terminated into the storage */
strcpy(new_ids->idsu.t.t_v.string.str, term_string);

/*
| insert the new entry at the end of the existing list
|
| New entry points to previous: wherever
| If there was no 'last' entry ...
*/
if ((new_ids->prev = ic->last_of_flavor[I_T_OBJ_string]) == NULL) {
    ic->flavors[I_T_OBJ_string] = new_ids;  /* Record Top-of-List */
    }
else {  /* Make old 'last' entry point forward to new entry */
    new_ids->prev->next = new_ids;
    }

/* New entry is at the end of the list */
new_ids->next = NULL;           

/* New End-Of-List */
ic->last_of_flavor[I_T_OBJ_string] = new_ids;

/* return the pointer to the new entry */
new_ids->flavor = I_T_OBJ_string;
new_ids->ex_add = 0;

/* Show initial reference as zero */
new_ids->idsu.t.ref_count = 0;
return(new_ids);
}

/* I_Create_IDS - Create a "Not Terminal" Intermediate Data Structure */
/* I_Create_IDS - Create a "Not Terminal" Intermediate Data Structure */
/* I_Create_IDS - Create a "Not Terminal" Intermediate Data Structure */

IDS *
I_Create_IDS (ids_type, ecount)

IDS_TYPE         ids_type; /* Type of Intermediate Data Structure to create */
unsigned         ecount;   /* Number of entries requested for table         */


/*
INPUTS:

    "ids_type" is the type of Intermediate Data Structure that the caller
    desires to be created (one of I_SLICE, I_NT_OBJECT_*, or I_SUBREGISTER).

    "ecount" specifies how many entries should appear in the table associated
    with the "ids_type" structure (if the structure has a table! Subregisters
    don't have tables).

       If this value is :

       ZERO: A default table size for this kind of structure is selected by
             this function.

       NON-ZERO: The table size is set to this number of entries (at least).


OUTPUTS:

    The function returns a pointer to a newly allocated IDS structure of flavor
    "ids_type", or NULL if storage could not be allocated.
    
BIRD'S EYE VIEW:
    Context:
        The caller wants to create an Intermediate Data Structure that is
        not a Terminal type of structure.

    Purpose:
        This function allocates storage for the specified structure and
        returns a pointer to it.

ACTION SYNOPSIS OR PSEUDOCODE:

    if (a free IDS of required flavor exists and no specific count requested)
        <unhook it from the free list>
    else 
        if (attempt to allocate space failed)
            <return NULL>

    <perform processing to insert new Non-Terminal at End of proper list>

    switch (type of IDS)
        case I_SLICE:
            if (table size given)
                if (allocate of proper table size succeeded)
                    <store table max size into IDS>
                else
                    <return NULL>
            <signal no entries and no backpointer>
            <zero-out the external address field>
            <set flavor into IDS structure>
            <return pointer to IDS>

        case I_SUBREGISTER:
            <show no Non-Terminal, no down-Slice and no backpointer>
            <zero-out the external address field>
            <set flavor into IDS structure>
            <return pointer to IDS>

        case I_NT_OBJECT_Rel:
        case I_NT_OBJECT_DC:
        case I_NT_OBJECT:
            <zero the Non-Terminal count areas>
            if (flavor is I_NT_OBJECT_DC or I_NT_OBJECT_Rel)
                <set reference count to 1>

            if (table size given)
                if (allocate of proper table size succeeded)
                    <store table max size into IDS>
                else
                    <return NULL>
            break;
            <break>

        default:
            <issue error message>
            <exit>


OTHER THINGS TO KNOW:

    As each IDS is allocated, it is put on top of the list that corresponds
    to its type.  This list is used during linearization/externalization.

With V1.95 (in preparation for V2.0), the logic for non-Index Non-Terminals
    (that is, for flavors I_NT_OBJECT, I_NT_OBJECT_Rel and I_NT_OBJECT_DC) is
    rearranged so that the lists are in the order of creation (instead of
    "reverse-order").  This should place the MIR Object for "The World" (which
    is the first one created) as the first MIR Object in the Non-Terminal
    partition.  This makes the interpreted dump easier to read, as well as
    setting us up for a (hopefully) better paging scheme when this file is
    paged.

    Also, with interim V1.95, we accept a second argument to this function
    to indicate the size of the table to be created.

With V1.98, the scheme above is extended to Slices, to make them "page"
    better, then we doubly-link the list and do it for everything this
    function handles.

    Reference counts apply only to Terminals and Non-Terminals of type
    I_NT_OBJECT_*.  We force _DC and _Rel types to have a "1" (never
    deallocated), whereas I_NT_OBJECT has a real reference count that starts
    out at 0.

    We attempt to find a free IDS of the right type from the
    free-list created and maintained by I_Reclaim_IDS() *IF* they aren't
    requesting a specific table size.

*/

{
IDS             *new_ids;    /* --> structure we allocate space for     */
unsigned        m_count;     /* Generic Maximum Table Entry Count       */
VOID            *table_ptr;  /* Generic Table Pointer                   */


/* ========================= START LIST PROCESSING ========================= */
/*
| if (a free IDS of required flavor exists and no specific count requested)
*/
if ((ic->top_of_free[ids_type] != NULL) && ecount == 0) {
    /*
    | unhook the next free from the top of the list
    */
    new_ids = ic->top_of_free[ids_type];         /* Grab the top */

    /* If the top was 'the last free' . . .*/
    if ((ic->top_of_free[ids_type] = new_ids->next) == NULL) {

        /* Make the 'bottom' pointer agree */
        ic->last_of_free[ids_type] = NULL;
        }

    /*
    | Copy and save (for restoration below) the maximum table entry count
    | and the pointer to the table for Slices and all Non-terminals.
    */
    if (ids_type >= I_NT_OBJECT_DC && ids_type <= I_NT_OBJECT_Rel) {
        m_count = new_ids->idsu.nt.max_count;
        table_ptr = (VOID *) new_ids->idsu.nt.rel_table;
        }
    else if (ids_type == I_SLICE) {
        m_count = new_ids->idsu.i.max_count;
        table_ptr = (VOID *) new_ids->idsu.i.slice;
        }
    }
else {    /* Need fresh storage */
    /* if (attempt to allocate space succeeded) */
    if ( (new_ids = (IDS *) malloc(sizeof(IDS)) ) == NULL) {
        return(NULL);
        }

    /* Show no maximum nor table present */
    m_count = 0;
    table_ptr = NULL;
    }

/* perform processing to insert new Non-Terminal at End of proper list
|
| For Slices - It is particularly important that we force the first
| allocated slice to remain at the top of the list so that it "linearizes"
| first into the output file.  In this manner, the slice that corresponds 
| to the "top of the index" is found as the first object inside the Index
| Partition.
|
| For Subregisters - It hardly matters, only 10 or so ever get created.
|
| For Non-Terminals -
|    - For _DC, they must appear in the order they were created: the order
|      seen in the builtin_types file.
|    - For _Rel, they *must* appear in order, with MIR_Relationship_Name as
|      first
|    - For general NTs, the first created must be linearized first, because
|      it is "The World".
| 
| NULL-out the "next" cell in this new IDS & set the "prev" cell
*/
new_ids->next = NULL;
new_ids->prev = ic->last_of_flavor[ids_type];

/*
| Now store the address of this new IDS into the "next" cell
| of the last entry on the list (if it exists).  If the list
| was empty, make sure we record top of list for this flavor.
*/
if (ic->last_of_flavor[ids_type] != NULL) {
    ic->last_of_flavor[ids_type]->next = new_ids;
    }
else { /* List is new . . . */
    ic->flavors[ids_type] = new_ids;
    }
/*
| Record the fact we have a new End-Of-List
*/
ic->last_of_flavor[ids_type] = new_ids;

/* ========================= END LIST PROCESSING ========================= */

/*
| Specific IDS-type setup
*/
switch (ids_type) {

    case I_SLICE:

        /* Restore any previously allocated table */
        new_ids->idsu.i.max_count = m_count;
        new_ids->idsu.i.slice = (struct SES *) table_ptr;

        /*
        | In this interim version of the compiler, we allocate from the
        | heap exactly the amount of storage we need if the user called
        | with a table size specified (ie "ecount" > 0).
        */
        if (ecount > 0) {
            if ((new_ids->idsu.i.slice =
                        (struct SES *) malloc( ecount*sizeof(struct SES)))
                != NULL) {
                new_ids->idsu.i.max_count = ecount;
                }
            else {  /* Allocate blew, bag out */
                return (NULL);
                }
            }

        new_ids->idsu.i.entry_count = 0;  /* No entries and ..          */
        new_ids->idsu.i.backptr = NULL;   /* No backpointer             */
        break;


    case I_SUBREGISTER:
        new_ids->idsu.s.ntobject = NULL;    /* Nothing there yet    */
        new_ids->idsu.s.lower_levels = NULL;/* Nothing there yet    */
        new_ids->idsu.s.backptr = NULL;     /* No backpointer       */
        break;


    case I_NT_OBJECT_Rel:   /* Non-Terminal for Relationship Object */
    case I_NT_OBJECT_DC:    /* Non-Terminal for Dataconstruct Object*/
    case I_NT_OBJECT:       /* General Non-Terminal                 */
        /*
        | For all NON-TERMINALS:
        |
        | Zap to Zero all of the guts of such a Non-Terminal
        */
        bzero(&new_ids->idsu.nt, sizeof(new_ids->idsu.nt));

        /* if (flavor is I_NT_OBJECT_DC or I_NT_OBJECT_Rel) */
        if (ids_type == I_NT_OBJECT_DC || ids_type == I_NT_OBJECT_Rel) {
            new_ids->idsu.nt.ref_count = 1; /* set reference count to 1 */
            }

        /*
        | In this interim version of the compiler, we allocate from the
        | heap exactly the amount of storage we need if the user called
        | with a table size specified (ie "ecount" > 0).
        */
        if (ecount > 0) {
            if ((new_ids->idsu.nt.rel_table =
                        (struct RES *) malloc( ecount*sizeof(struct RES)))
                != NULL) {
                new_ids->idsu.nt.max_count = ecount;
                }
            else {  /* Allocate blew, bag out */
                return (NULL);
                }
            }
        else {  /* In case we're re-using an IDS . . .*/
            /* Restore any previously allocated table */
            new_ids->idsu.nt.max_count = m_count;
            new_ids->idsu.nt.rel_table = (struct RES *) table_ptr;
            }

        break;


    case I_T_OBJ_snumber:
    case I_T_OBJ_unumber:
    case I_T_OBJ_string:
    default:
        printf(MP(mp201,"mirc - Internal Error in I_Create_IDS() for non-terminal:%d.\n"),
               (int) ids_type);
        exit(1);
    }

/* zero-out the external address field */
new_ids->ex_add = 0;

/* set flavor into IDS structure */
new_ids->flavor = ids_type;

/* return pointer to IDS */
return (new_ids);
}

/* I_Reclaim_IDS - Reclaim an Intermediate Data Structure of any flavor */
/* I_Reclaim_IDS - Reclaim an Intermediate Data Structure of any flavor */
/* I_Reclaim_IDS - Reclaim an Intermediate Data Structure of any flavor */

void
I_Reclaim_IDS (re_ids)

IDS        *re_ids;  /* --> IDS to be reclaimed */


/*
INPUTS:

    "re_ids" points to the IDS to be reclaimed.  The flavor indicator is
     built-in to the IDS.

OUTPUTS:

    The function returns on success and exits after printing an error message
    if there is problem.


BIRD'S EYE VIEW:
    Context:
        The caller wants to release for re-use an Intermediate Data Structure.

    Purpose:
        This function unhooks the IDS from the list in which it is embedded
        and inserts it at the end of the corresponding list of free IDS of
        the same flavor.


ACTION SYNOPSIS OR PSEUDOCODE:

    <acquire the addresses of the tops of the four lists for this flavor>

    (* Removal from the active list *)
    if (there is a 'prior' IDS)
        <change the prior IDS 'next' to contain the current's 'next' cell>
        if ('next' cell was NULL)  (* no 'succeeding' *)
            <change 'last'-of-flavor pointer to point to prior IDS>
        else
            <change the 'succeeding' prev cell to contain current's previous>
    else (* no 'prior' *)
        <change 'top-of-flavor' pointer to contain to current's 'next' cell>
        if ('next' cell was NULL) (* if List is now entirely empty *)
            <change 'last'-of-flavor pointer to NULL>
        else (* succeeding is present *)
            <change succeeding IDS 'prev' cell to NULL>

    if (flavor is string)
        <perform binary lookup using string value>
        if (lookup is EXACT)
            <scrunch all higher entries down "one" on top of exact match>
            <decrement count of entries in the xr array>

    (* Insertion at the bottom of the flavor's free list *)
    <set current 'next' cell to NULL>
    <set current 'prev' cell to current value of 'last-of-free' pointer>
    if ('last-of-free' pointer was NULL) (* if list was entirely empty *)
        <set 'top-of-free' pointer to current>
    else
        <set 'next' cell of 'last-of-free''s entry to point to current>
    <set 'last-of-free' pointer to current>
    <mark the element as "reclaimed">


OTHER THINGS TO KNOW:

    For Removal from Active List, the discussion above describes:

           'prior IDS'
              |     ^
              V     |
           'current IDS'
              |     ^
              V     |
           'succeeding IDS'

    Notice that all flavors are handled identically except for a little special
    processing for strings.  This implies that Slice and all Non-Terminal
    flavors which have tables allocated in the heap do not have these tables
    released.  They simply get re-used when a call for one of these flavors
    occurs and we have a 'used' entry waiting on the free list.

    Special processing for strings consists of updating the binary search
    "cross-reference" array left over from either the binary MIR load
    operation or the one 'faked-up' by mirc_init_backend().  This update
    operation is expensive, but its the price we pay for a fast lookup.

Now just a second here....
    Why do we mark the flavor of the object as "reclaimed" when nobody ought
    to be looking at it anyway?

    Well, in "mirc_reset()" in "mir_backend.c" there is a loop that dumps
    the contents of the list of "builtup" dataconstructs for the last file
    compiled.  The list is made up of IDX structures that point to the
    IDS structures that stand for datatypes defined by the user using "TYPE"
    statements.  In an attempt to blow off the IDS structures that stand
    for TYPE statements *that were not referenced* during the compilation,
    the loop examines the reference count on each built-up dataconstruct IDS
    NT and calls "mirc_remove_OBJECT()" (in "mir_remove.c") to blow off the
    ones with a *zero* reference count **** and possibly all children of
    that builtup***!  The "mirc_remove_OBJECT()" call eventually invokes this
    function which will do the actual detaching of the IDS from its backend
    intermediate context list (flavor).

    The problem is that the user may have created an arbitrarily large and
    complex tree of TYPE statements that was never referenced.  Entries
    will be in the IDX list for each TYPE statement.  If we happen to bump
    into the top of the tree for the first mirc_remove_OBJECT() call, then
    mirc_remove_OBJECT() will descend the tree and blow off everything.  This
    is fine until the loop goes around and the next IDX entry on the
    builtup list is examined. . .it may have already been reclaimed.  By
    marking reclaimed IDSs as "gone", that loop will do the right thing.
    This is a pretty simple case, as there is no chance anything will be
    re-used from the free-list before that built-up-reclaim loop finishes.
*/

/* #define LIST_CHECK */
{
IDS             **top_active;   /* -->> of Top-of-Active List            */
IDS             **last_active;  /* -->> of Last-on-Active List           */
IDS             **top_free;     /* -->> of Top-of-Free List              */
IDS             **last_free;    /* -->> of Last-on-Free List             */
IDS_TYPE        c_flavor;       /* Flavor of current IDS to be reclaimed */
int             cmp_code;       /* Result of string comparison           */
ext_int_xref    *xr;            /* --> String "Cross-Reference" array    */
int             xr_count;       /* Count of # of entries in "xr[]"       */
int             i;              /* Handy-Dandy General purpose index     */
int             hi,lo;          /* Boundary posts for binary search      */
char            *term_string;   /* --> String sought for binary lookup   */


/* acquire the addresses of the tops of the four lists for this flavor */
c_flavor = re_ids->flavor;
top_active = &(ic->flavors[c_flavor]);
last_active = &(ic->last_of_flavor[c_flavor]);
top_free = &(ic->top_of_free[c_flavor]);
last_free = &(ic->last_of_free[c_flavor]);

/*
| Removal from the active list
*/

/* if (there is a 'prior' IDS) */
if (re_ids->prev != NULL) {

    /*
    | This 'unhooks' the current IDS from the point of view of walking
    | *down* the list from the top.
    |
    | change the prior IDS 'next' to contain the current's 'next' cell
    */
    re_ids->prev->next = re_ids->next;

    /*
    | This patches up the list from the point of view of walking *up* the
    | list from the bottom, and takes care of adjusting the end-of-active list
    | pointer if the reclaimed IDS happened to be last on the active list.
    | 
    | if ('next' cell was NULL)  ie. no 'succeeding'
    */
    if (re_ids->next == NULL) {
        /* change 'last'-of-flavor pointer to point to prior IDS */
        *last_active = re_ids->prev;
        }
    else {
        /* change the 'succeeding' prev cell to contain current's previous */
        re_ids->next->prev = re_ids->prev;
        }
    }

else { /* no 'prior' */

    /*
    | This handles the situation where the IDS being reclaimed is at the
    | top of the active list.
    |
    | change 'top-of-flavor' pointer to contain to current's 'next' cell
    */
    *top_active = re_ids->next;

    /*
    |  If List is now going to be entirely empty, (ie reclaimed is last active)
    |
    | if ('next' cell was NULL)
    */
    if (re_ids->next == NULL) {
        /* change 'last'-of-flavor pointer to NULL */
        *last_active = NULL;
        }

    else {  /* succeeding is present, the list is not entirely empty */
        /* change succeeding IDS 'prev' cell to NULL */
        re_ids->next->prev = NULL;
        }
    }

/*
| On a string delete, we have to see if the cross-reference array happens
| to have an entry that points directly at the IDS being reclaimed.  If
| it does, we must eliminate the corresponding entry in the cross-reference
| array.... the hard way.
|
|if (flavor is string)
*/
if (c_flavor == I_T_OBJ_string) {

    /*
    | ================== START BINARY SEARCH ========================
    | Perform binary search to find closest & lowest entry to desired
    */
    xr = ic->xref[I_T_OBJ_string];
    xr_count = ic->xref_count[I_T_OBJ_string];
    term_string = re_ids->idsu.t.t_v.string.str;

    hi = xr_count - 1;  /* Set Hi boundary post    */
    lo = 0;             /* Set Lo boundary post    */
    cmp_code = 1;       /* (in remote event the xr[] has no entries) */
    i = 0;              /* (in remote event the xr[] has no entries) */

    while (lo <= hi) {
        i = (lo+hi)/2;     /* Grab the (new) comparison point */

        /* Compare:       ------------NEXT ENTRY----------- to SOUGHT STRING */
        cmp_code = strcmp(xr[i].int_add->idsu.t.t_v.string.str, term_string);

        if (cmp_code < 0)
            lo = i + 1;       /* Tested Entry is Too Low: Raise lower bound */
        else if (cmp_code > 0)
            hi = i - 1;       /* Tested Entry is Too High: Lower upper bound */
        else
            break;
        }
    /*
    | ==================== END BINARY SEARCH ========================
    |
    if (lookup is EXACT)
    */
    if (cmp_code == 0) {
        /* decrement count of entries in the xr array */
        xr_count -= 1;
        ic->xref_count[I_T_OBJ_string] = xr_count;

        /* scrunch all higher entries down "one" on top of exact match
        |
        | (This might be done more efficiently with a memcpy, but I don't
        |  know whether memcpy tolerates overlapping fields).
        */
        while (i < xr_count) {
            xr[i] = xr[i+1];
            i += 1;
            }
        }
    }


#ifdef LIST_CHECK
printf("Checking removal of reclaimed IDS from active list. . ");
{
IDS *check;
IDS *last=NULL;

for (check = ic->flavors[c_flavor];
     check != NULL;
     check = check->next) {
    if (check == re_ids) {
        printf("Reclaimed IDS *DOWN* not correctly removed!\n");
        };
    last = check;
    }
if (last != ic->last_of_flavor[c_flavor]) {
    printf("Down scan didn't wind up at last in active list");
    }

/* Now check the up links */
for (check = ic->last_of_flavor[c_flavor];
     check != NULL;
     check = check->prev) {
    if (check == re_ids) {
        printf("Reclaimed IDS *UP* not correctly removed!\n");
        };
    last = check;
    }
if (last != ic->flavors[c_flavor]) {
    printf("Up scan didn't wind up at top in active list");
    }

}
printf("flavor = %d Done\n ", c_flavor);
#endif


/*
| Insertion at the bottom of the flavor's free list
*/

/* set current 'next' cell to NULL */
re_ids->next = NULL;

/* set current 'prev' cell to current value of 'last-of-free' pointer */
/* if ('last-of-free' pointer was NULL) ie, if list was entirely empty */
if ((re_ids->prev = *last_free) == NULL) {
    /* set 'top-of-free' pointer to current */
    *top_free = re_ids;
    }
else {
    /*
    | Set 'next' cell of 'last-of-free''s entry to point to current.
    | (This should always overwrite a NULL).
    */
    (*last_free)->next = re_ids;
    }

/* set 'last-of-free' pointer to current */
*last_free = re_ids;

/* mark the element as "reclaimed" */
re_ids->flavor = I_RECLAIMED;
}
