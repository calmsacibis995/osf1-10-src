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
static char *rcsid = "@(#)$RCSfile: mir_remove.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:00:37 $";
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
 * Module MIR_REMOVE.C
 *      Contains functions required by the MIR Compiler to remove (from the
 *      (in-heap) "internal" representation of what the compiler has compiled)
 *      an entity-class and it's children (when that entity-class is specified
 *      by OID).
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
 *       compiler to mimic the kind of maintenance operation provided by
 *       the MCC "DAP" program.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *      V1.0    September 1992           D. D. Burns

Module Overview:

This module contains most the functions required by the MIR Compiler to
remove a specified entity-class (and all it contains, including any child
entity-classes) from the current internal heap representation that the
compiler maintains.



MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
mirc_remove_EC       When called with the OID of an entity-class to remove
                     (and a pointer to the compiler's back-end context)
                     this function looks up the MIR Object that corresponds
                     to the Entity-class in the index, removes that MIR
                     Object and all it's children (reclaiming the heap storage)
                     and all OID references in the index to these children.
                     This is a non-recursive 'front-end' function to
                     "mirci_remove_OBJECT()" which does the real work.

mirc_remove_OBJECT   Recursively removes the MIR object's that are the targets
                     of each and every relationship in the incoming MIR
                     object's relationship table.  It then deletes all the OIDs
                     for the incoming MIR Object that may appear in the index,
                     and then deletes the object itself.


INTERNAL FUNCTIONS:
mirci_remove_OID     Removes all traces of a specified OID from the
                     Intermediate (in-heap) representation of the OID index.

mirci_count_depends  Scans a (containing) Entity-Class object and builds a
                     sum of references to one of it's Characteristic Attributes
                     by Depends-On Objects.

mirci_count          Scans a selected object to see if it has one or more
                     Depends-On Object that references a particular
                     Characteristic Attribute.
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
extern char *mp400();
extern char *mp401();
extern char *mp402();
extern char *mp403();
extern char *mp404();
extern char *mp405();
extern char *mp406();
extern char *mp407();
extern char *mp408();
extern char *mp409();
extern char *mp410();
extern char *mp411();
extern char *mp412();
extern char *mp413();
extern char *mp414();
extern char *mp415();
#endif


/*
|
|       Prototypes for module-local functions
|
*/
/* mirci_count_depends - Counts Depends-On References to Char. Attribute */
static void
mirci_count_depends PROTOTYPE((
IDS     *,              /* --> Object to be searched for Depends-On Object */
IDS     *,              /* --> Characteristic Attribute object             */
int     *               /* --> Integer that counts references              */
));

/* mirci_count - Counts Depends-On References to Char. Attribute IN ONE OBJ */
static void
mirci_count PROTOTYPE((
IDS     *,              /* --> Object to be searched for Depends-On Object */
IDS     *,              /* --> Characteristic Attribute object             */
int     *               /* --> Integer that counts references              */
));

/* mirci_remove_OID - Removes OID from Intermediate Index */
static void
mirci_remove_OID PROTOTYPE((
IDS            *,   /* --> IDS of slice or subregister for slot to  */
                    /*     being deleted                            */
unsigned int        /* Arc number whose slot is sought              */
));


/* mirc_remove_EC - Remove Intermediate Representation of an Entity-Class */
/* mirc_remove_EC - Remove Intermediate Representation of an Entity-Class */
/* mirc_remove_EC - Remove Intermediate Representation of an Entity-Class */

char *
mirc_remove_EC (oid_string, bec)

char              *oid_string; /* --> String containing OID for Object */
back_end_context  *bec;        /* Pointer to back-end context block    */

/*
INPUTS:

    "oid_string" points to a string that contains a representation of
    the OID of the Entity-class to be removed.

    "bec" is the address of the "back-end" context block, some of whose
    data-structures must be referenced by this function.


OUTPUTS:

    On success, the function returns NULL, indicating:

      * The specified MIR Object has been verified as an Entity-class and
        then deleted.
      * All the children of the Entity-class have been deleted.
      * All OIDs in the index that specified the original Entity-class and
        OIDs for MIR Objects contained in the Entity-class and OIDs for any
        of it's children have been removed from the index.

    On failure, a non-null string pointing to an error message is returned
    indicating the source of the failure.  For this kind of failure,
    compilation may continue.

    If a severe compiler-internal error is detected, a message is printed on
    stderr and an exit is performed.


BIRD'S EYE VIEW:
    Context:
        The compiler has begun operations and gotten far enough to have either
        compiled from an ASCII MSL or loaded (from a binary MIR database file)
        at least one Entity-class.  At this point, it may be deleted from
        the compilation via a "-r" switch that specifies by OID the entity-
        class to be deleted.  Typically this would be done after loading
        a binary MIR database file and before compiling an ASCII MSL file.

    Purpose:
        This function takes care of removing an Entity-Class from the in-heap
        intermediate representation of what the compiler knows about.  This
        includes the children and all OIDs that make reference to the E-C and
        it's contents.


ACTION SYNOPSIS OR PSEUDOCODE:

    <attempt to convert the string OID into binary>
    if (attempt failed)
        <return "Ill-formed OID" error msg>

    if (I_Find_OID lookup returned something other than "MC_FIND_Exact" or
        "MC_FIND_Exact_Long")
        <return "OID does not exactly specify a MIR object" error msg>

    <obtain the IDS address of the MIR Object believed to be an E-C>
    if (IDS is a subregister)
        <step down to the object itself>
    if (IDS is not a Non-Terminal)
        <return "OID does not exactly specify a MIR object" error msg>

    <scan object's relationship table for "MIR_Indexed_By">
    if (not found)
        <scan object for "MIR_Text_Name" & extract the name or "No-Name-Found">
        <return "MIR Object named '<name>' is not a valid Entity-Class">

    <obtain IDS ptr of containing Entity-Class>

    for (every "MIR_Depends_on" relationship in this entity-class)
        <obtain target of "MIR_Depends_on" rel: the Depends-On MIR Object>
        <signal "No Char Attrib Found">
        for (each entry in the Depends-On MIR Object)
            if (entry is "MIR_Depends_on")
                <signal IDS ptr of Characteristic Attribute>
                <signal "Char Attribute Found">
                <break>
        if (Char Attribute Not Found)
            <issue "Internal Compiler Error: Missing Char. Attribute">

        <set sum of count of references to 0>
        for (each object contained therein)
            <obtain a count of the number of DEPENDS-ON clauses applied
             to each thing that references the specified Char. Attrib>
            <add count obtained to running sum>
            if (sum > 1)
                <break>

        if (sum is Zero)
            <issue "Internal Compiler Error: Missing Ref. To Char. Attribute">

        if (sum is one)
            for (each entry in the containing Entity-Class rel. table)
                if (entry is for "MIR_Variant_Sel")
                    if (target is the Characteristic Attribute)
                        <delete the entry in the Relationship Table>
                        <break>

    <scan containing-EC object's relationship table for MIR_Cont_entityClass
     that has pointer to object being removed>
    if (not found)
        <return "MIR Object named '<name>' is not properly contained by it's
                parent">
    <remove the entry from the containing-EC's relationship table>

    <perform mirc_remove_OBJECT() to dump all entries, the oids at the object>
    <return NULL>

OTHER THINGS TO KNOW:

    This function consists largely of code required to take care of the
    very special case where the Entity-Class being deleted contained a
    DEPENDS-ON clause specifying a Characteristic Attribute in it's parent
    Entity-Class.  In this case, *if* this Entity-Class is the *only* thing
    in the parent Entity-Class that 'depends-on' that Characteristic
    Attribute, then we have to delete the MIR_Variant_Sel entry in the parent
    Entity-class that points at the Characteristic Attribute.  If other things
    'depend-on' the Characteristic Attribute, we need do nothing.

    This Depends-On/Variant-Selector processing is straight forward, but
    tedious.  There is no obvious way around it, as the very nature of
    the "Depends-On" clause may span the gap between Entity-Classes.
*/      

{
int             oid_str_len;    /* Length of the incoming OID string         */
char            *oid_buf;       /* --> Heap containing a copy of "oid_string"*/
char            *next_token;    /* --> Next token returned by "strtok()"     */
object_id       oid;            /* Local binary copy of "oid_string"         */
unsigned int    *value;         /* --> Array of integer arcs (heap)          */
int             i,j;            /* Handy-Dandy General-Purpose Indices       */
IDS             *slice;         /* Slice found by I_Find_OID() on OID lookup */
int             s_index;        /* Slice Index found by I_Find_OID()         */
int             match_count;    /* Match Count found by I_Find_OID()         */
IDS             *e_class;       /* --> Entity-Class MIR Object               */
int             e_class_entry;  /* --> Each entry in Rel. Table for e-class  */
BOOL            indexed_by_fnd; /* TRUE: MIR_Indexed_By was found: True E-C  */
IDS             *d_o_obj;       /* --> Depends-On MIR Object                 */
IDS             *char_attrib;   /* --> Characteristic Attrib. for Depends-On */
IDS             *containing_EC; /* --> Entity-Class containing 1 being remvd */
MC_STATUS       lookup_status;  /* Status returned from I_Find_OID           */
int             refs;           /* Count of references to Char. Attrib       */
static
    char        msg[200];       /* Error message buffer                      */

/* ===========================================
| attempt to convert the string OID into binary
*/

/*
| Create a copy of the incoming OID string that we can parse and trash with
| "strtok" to find out if it is valid and how long it is.
*/
oid_str_len = strlen(oid_string);
if ( (oid_buf = (char *) malloc(oid_str_len+1)) == NULL) {
    fprintf(stderr,
            MP(mp400,"mirc - Internal Error:\n       Out of Memory\n"));
    exit(BAD_EXIT);
    }
strcpy(oid_buf, oid_string);

/*
| Walk down the string and parse each number out of it, and check the numbers
| to be sure they are digits.
|
| For each token (arc) in the OID
*/
for (next_token = strtok(oid_buf, ". "), oid.count = 0;
     next_token != NULL;
     next_token = strtok(NULL, ". "), oid.count += 1) {

    /* Each character had better be a digit */
    while (*next_token != '\0') {
        if (!isdigit(*next_token++)) {
            sprintf(msg, MP(mp401,"OID (%s) contains invalid character"),
                    oid_string);
            return(msg);
            }
        }
    }

/*
| We've got a count of the number of arcs and we know they'll convert
|
| Allocate space to hold the right number of integers for the arcs
*/
if ((value = (unsigned int *) malloc(oid.count*sizeof(unsigned int)))
    == NULL) {
    fprintf(stderr,
            MP(mp400,"mirc - Internal Error:\n       Out of Memory\n"));
    exit(BAD_EXIT);
    }

/*
| Copy the OID string again and walk down the string and parse each number
| out of it and into the value array
|
| For each token (arc) in the OID
*/
strcpy(oid_buf, oid_string);
for (next_token = strtok(oid_buf, ". "), i = 0;
     next_token != NULL;
     next_token = strtok(NULL, ". "), i += 1) {

    /* Convert the token to binary and stash it as an arc value */
#ifdef VMS
    value[i] = strtol(next_token, NULL, 10);    /* Avoid Bug in VMS strtoul */
#else
    value[i] = strtoul(next_token, NULL, 10);
#endif
    }
free(oid_buf);          /* Release the temporary copy */
oid.value = value;      /* Hook the arc array into the OID */


/* ===========================================
|  We've the OID in binary. . . now look it up.
|
| if (I_Find_OID lookup returned something other than "MC_FIND_Exact" or
|     "MC_FIND_Exact_Long")
*/
lookup_status = I_Find_OID(&oid, &slice, &s_index, &match_count);
if (lookup_status != MC_FIND_Exact && lookup_status != MC_FIND_Exact_Long) {

    free(value);    /* Release the storage for the OID arcs */

    /* return "OID does not exactly specify a MIR object" error msg */
    sprintf(msg,
            MP(mp402,"OID (%s) does not exactly specify a MIR object."),
            oid_string);
    return(msg);
    }

free(value);    /* Release the storage for the OID arcs */

/* obtain the IDS address of the MIR Object believed to be an E-C */
e_class = slice->idsu.i.slice[s_index].next;

/* if (IDS is a subregister) */
if (e_class->flavor == I_SUBREGISTER) {

    /* step down to the object itself */
    e_class = e_class->idsu.s.ntobject;
    }

/* if (IDS is not a Non-Terminal) */
if (e_class->flavor != I_NT_OBJECT) {
    /* return "OID does not exactly specify a MIR object" error msg */
    sprintf(msg,
            MP(mp402,"OID (%s) does not exactly specify a MIR object."),
            oid_string);
    return(msg);
    }


/* ===========================================
| Verify this thing really is an Entity Class by finding "MIR_Indexed_By"
| in the relationship table.
| 
| scan E-Class object's relationship table for "MIR_Indexed_By"
*/
for (i = 0, indexed_by_fnd = FALSE ;
     (i < e_class->idsu.nt.entry_count && indexed_by_fnd == FALSE);
     i += 1) {

    if (e_class->idsu.nt.rel_table[i].rel_obj
        != bec->map_rel_to_ids[MIR_Indexed_By]) {
        continue;
        }
    else {
        indexed_by_fnd = TRUE;
        }
    }

/* if (not found) */
if (indexed_by_fnd == FALSE) {
    /*
    |  scan object for "MIR_Text_Name" & extract the name or "No-Name-Found",
    |  return "MIR Object named '<name>' is not a valid Entity-Class"
    */
    sprintf(msg,
            MP(mp403,"MIR Object (%s) named '%s'\n       is not a valid Entity-Class."),
            oid_string,
            mirc_find_obj_NAME(e_class));
    return(msg);
    }


/* ===========================================
| obtain IDS ptr of containing Entity-Class
*/
for (i = 0, containing_EC = NULL;
     i < e_class->idsu.nt.entry_count;
     i += 1) {

    /* if (entry is "MIR_Contained_By") */
    if (e_class->idsu.nt.rel_table[i].rel_obj
        == bec->map_rel_to_ids[MIR_Contained_By]) {

        /*
        | signal "Containing Entity-Class Found"
        */
        containing_EC = e_class->idsu.nt.rel_table[i].tar_obj;
        break;
        }
    }

/* if (Containing Entity-Class was Not Found) */
if (containing_EC == NULL) {
    /* issue "Internal Compiler Error: Missing Containing E-C" */
    fprintf(stderr,
            MP(mp404,"mirc - Internal Error:\n       Missing containing Entity-Class\n"));
    exit(BAD_EXIT);
    }


/* ===================================================================
| Perform special handling if Entity-Class contains a "MIR_Depends_on"
| relationship.
|
| Here we may have to modify the Variant Selector block for the E-C that
| contains this E-C that we're deleting.  See the OTHER THINGS TO KNOW section.
|
| for (every "MIR_Depends_on" relationship in this entity-class)
*/
/* Find all present */
for (e_class_entry = 0;
     e_class_entry < e_class->idsu.nt.entry_count;
     e_class_entry += 1) {

    if (e_class->idsu.nt.rel_table[e_class_entry].rel_obj
        != bec->map_rel_to_ids[MIR_Depends_on]) {
        continue;
        }

    /* obtain target of "MIR_Depends_on" rel: the Depends-On MIR Object */
    d_o_obj = e_class->idsu.nt.rel_table[e_class_entry].tar_obj;

    /*
    | signal "No Char Attrib Found"
    | for (each entry in the Depends-On MIR Object)
    */
    for (i = 0, char_attrib = NULL;
         i < d_o_obj->idsu.nt.entry_count;
         i += 1) {

        /* if (entry is "MIR_Depends_on") */
        if (d_o_obj->idsu.nt.rel_table[i].rel_obj
            == bec->map_rel_to_ids[MIR_Depends_on]) {
        
            /*
            | signal IDS ptr of Characteristic Attribute
            | signal "Char Attribute Found"
            */
            char_attrib = d_o_obj->idsu.nt.rel_table[i].tar_obj;
            break;
            }
        }

    /* if (Char Attribute Not Found) */
    if (char_attrib == NULL) {
        /* issue "Internal Compiler Error: Missing Char. Attribute" */
        fprintf(stderr,
                MP(mp405,"mirc - Internal Error:\n       Missing Char. Attribute\n"));
        exit(BAD_EXIT);
        }

    refs = 0;   /* set sum of count of references to 0 */

    /*
    | Within this specified 'containing' Entity-Class, we want to look
    | at every Entity-Class, Attribute, Directive and Event.  If any of these
    | things has a MIR_Depends_on relationship that points to a "Depends-On"
    | block that specifies (via another MIR_Depends_on relationship) the
    | Characteristic Attribute found above, we need to count that occurence.
    |
    | Additionally, within every Directive found, every Request Argument,
    | every Response Argument and every Exception Argument must be similarly
    | examined and occurences of Depends-On (that Characteristic Attribute)
    | must be counted.
    |
    | We can stop counting once the count has gotten above 1.  It should be
    | either one or greater than one.  It's an error if 0.
    |
    | If exactly 1, then that is the reference from within the Entity-Class
    | we're about to delete, so everything having to do with the dependence
    | on that Char. Attribute must go (in the containing Entity-Class
    | Variant Selector List Object).  If, after deleting that, the Variant
    | Selector List object is empty, then it must go too.
    |
    | If greater than 1, that means something else 'depends on' the
    | Characteristic Attribute, so we don't have to do anything to the
    | containing Entity-class's Variant-Selector List object when we blow off
    | the Depends-On Object that points 'up' to the containing class's
    | Characteristic attribute.
    */
    mirci_count_depends(containing_EC, char_attrib, &refs);

    /* if (sum is Zero) */
    if (refs == 0) {
        /* issue "Internal Compiler Error: Missing Ref. To Char. Attribute" */
        fprintf(stderr,
                MP(mp406,"mirc - Internal Error:\n       Missing ref. to Char. Attribute\n"));
        exit(BAD_EXIT);
        }

    /* if (sum is one) */
    if (refs == 1) {

        /* for (each entry in the containing Entity-Class rel. table) */
        for (i = 0;
             i < containing_EC->idsu.nt.entry_count;
             i += 1) {

            /* if (entry is "MIR_Variant_Sel") */
            if (containing_EC->idsu.nt.rel_table[i].rel_obj
                == bec->map_rel_to_ids[MIR_Variant_Sel]) {

                /* if (target of MIR_Variant_Sel is the Char. Attribute) */
                if (containing_EC->idsu.nt.rel_table[i].tar_obj
                    == char_attrib) {

                    /* delete the entry in the Relationship Table */
                    for (j = i + 1;
                         j < containing_EC->idsu.nt.entry_count;
                         j += 1, i += 1) {
                        containing_EC->idsu.nt.rel_table[i] = 
                            containing_EC->idsu.nt.rel_table[j];
                        }

                    /* Show one less entry */
                    containing_EC->idsu.nt.entry_count -= 1;

                    break;
                    }
                }
            }
        }
    /* if (sum > 1). . . there's nothing to do! */
    }

/* ===========================================
| Remove the reference to the object we're about to delete in the containing
| Entity-Class's relationship table.  We're looking for relationship
| "MIR_Cont_entityClass" in the parent that has a target of the object
| we're about to blow off.
|
| Scan containing-EC object's relationship table for "MIR_Cont_entityClass".
*/
for (i = j = 0; 
     i < containing_EC->idsu.nt.entry_count;
     i += 1, j += 1) {

    /* if the entry matches */
    if (containing_EC->idsu.nt.rel_table[i].rel_obj
        == bec->map_rel_to_ids[MIR_Cont_entityClass]
        && containing_EC->idsu.nt.rel_table[i].tar_obj == e_class) {

        /* Signal "scrunch all other entries down on top of this one" */
        j = i + 1;

        /* Show one less entry */
        containing_EC->idsu.nt.entry_count -= 1;
        }

    /* If we're 'scrunching' . . . */
    if (i != j && i < containing_EC->idsu.nt.entry_count) {
        /* Scrunch it */
        containing_EC->idsu.nt.rel_table[i] =
            containing_EC->idsu.nt.rel_table[j];
        }
    }

/* if (not found) */
if (i == j) {
    sprintf(msg,
            MP(mp408,"MIR Object (%s) named '%s'\n       is not properly contained by it's parent"),
            oid_string,
            mirc_find_obj_NAME(e_class));
    return(msg);
    }


/* ===========================================
| perform mirc_remove_OBJECT() to dump all entries and the oids at the object
| which is the entity-class selected by OID
*/
mirc_remove_OBJECT(e_class, bec);

return(NULL);
}

/* mirci_count_depends - Counts Depends-On References to Char. Attribute */
/* mirci_count_depends - Counts Depends-On References to Char. Attribute */
/* mirci_count_depends - Counts Depends-On References to Char. Attribute */

static void
mirci_count_depends (obj, char_attrib, refs)

IDS     *obj;           /* --> Object to be searched for Depends-On Object */
IDS     *char_attrib;   /* --> Characteristic Attribute object             */
int     *refs;          /* --> Integer that counts references              */

/*
INPUTS:

    "obj" points to an IDS that is a Non-Terminal to be searched for
    references to a Depends-On MIR Object via relationship "MIR_Depends_on".

    "char_attrib" points to the Characteristic Attribute for which references
    -to are supposed to be counted.

    "refs" is where we return the count of references to "char_attrib".


OUTPUTS:

    "refs" is set to the count of references to the Characteristic Attribute
    upon return.


BIRD'S EYE VIEW:
    Context:
        The compiler has begun operations and gotten far enough to have either
        compiled from an ASCII MSL or loaded (from a binary MIR database file)
        at least two Entity-classes, and one is being deleted.  If it contains
        a "MIR_Depends_on" relationship, then the Characteristic Attribute
        that it depends on is in it's containing E-Class.  We have to search
        that e-class to see if other references are made to that same
        Char. Attribute: if so, when we delete the specified E-class, we
        don't have to do anything.  Otherwise, if the to-be-deleted Entity
        class contains the only (last) reference to that Char. Attribute,
        then the Variant-Selector List object must be modified in the
        containing Entity-Class.  

    Purpose:
        This function is intended to count the number of references to the
        specified Characteristic Attribute in the containing Entity-Class.
        On the first call, the "obj" argument is what we've been calling
        the "containing Entity-Class".


ACTION SYNOPSIS OR PSEUDOCODE:

    for (every entry in the object's relationship table)
        switch (Relationship Object synonym)
            case MIR_Cont_entityClass:
            case MIR_Cont_attribute:
                <perform depend-count on target object>
                break;

            case MIR_Cont_event:
                for (every entry in the target of MIR_Cont_event)
                    if (relationship is MIR_Cont_Argument)
                        <perform depend-count on Target of MIR_Cont_Argumnt>
                        <break>
                <break>

            case MIR_Cont_directive,
                <perform depend-count on target object>
                for (every entry in the target of MIR_Cont_directive)
                    <obtain relationship synonym code>
                    switch (synonym)
                         case MIR_Cont_request_argument:
                             <perform depend-count on Target of entry>
                             <break>
                         case MIR_Cont_response:
                         case MIR_Cont_exception:
                            for (every entry in target of MIR_Cont_response)
                                if (relationship is MIR_Cont_Argument)
                                    <perform depend-count on Target of
                                     MIR_Cont_Argumnt>
                                    <break>
                            <break>
    
        if (ref > 1)
           <return>


OTHER THINGS TO KNOW:

    This function is a pain-in-the-a**.  All this because of the lousy
    Variant-Selector List objects.
*/      

{
int     i,j,k;          /* Handy-Dandy General purpose indices     */
int     syn1;           /* Synonym extracted from Main Object      */
int     syn2;           /* Synonym extracted from Secondary Object */
IDS     *target1;       /* First level target object               */
IDS     *target2;       /* Second level target object              */


/* for (every entry in the object's relationship table) */
for (i = 0; i < obj->idsu.nt.entry_count; i += 1) {

    syn1 = obj->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym;

    /* switch (Relationship Object synonym from Main Object) */
    switch (syn1) {

        case MIR_Cont_entityClass:
        case MIR_Cont_attribute:
            /* perform depend-count on target object */
            mirci_count(obj->idsu.nt.rel_table[i].tar_obj, char_attrib, refs);
            break;

        case MIR_Cont_event:
            target1 = obj->idsu.nt.rel_table[i].tar_obj;

            /* for (every entry in the target of MIR_Cont_event) */
            for (j = 0; j < target1->idsu.nt.entry_count; j += 1) {

                /* if (relationship is MIR_Cont_Argument) */
                if (target1->idsu.nt.rel_table[j].rel_obj->idsu.nt.ntu.synonym
                     == MIR_Cont_argument) {
                    /* perform depend-count on Target of MIR_Cont_Argumnt */
                    mirci_count(target1->idsu.nt.rel_table[j].tar_obj,
                                char_attrib, refs);
                    break;
                    }
                }
            break;

        case MIR_Cont_directive:
            /* perform depend-count on target object */
            mirci_count(obj->idsu.nt.rel_table[i].tar_obj, char_attrib, refs);

            target1 = obj->idsu.nt.rel_table[i].tar_obj;

            /* for (every entry in the target of MIR_Cont_directive) */
            for (j = 0; j < target1->idsu.nt.entry_count; j += 1) {

                /* obtain relationship synonym code */
                syn2 =
                    target1->idsu.nt.rel_table[j].rel_obj->idsu.nt.ntu.synonym;

                switch (syn2) {

                    case MIR_Cont_request_argument:
                        mirci_count(target1->idsu.nt.rel_table[j].tar_obj,
                                    char_attrib, refs);
                        break;

                    case MIR_Cont_response:
                    case MIR_Cont_exception:
                        target2 = target1->idsu.nt.rel_table[j].tar_obj;

                        /* for (every entry in target of MIR_Cont_response) */
                        for (k=0; k < target2->idsu.nt.entry_count; k += 1) {

                            /* if (relationship is MIR_Cont_Argument) */
                            if (
                 target2->idsu.nt.rel_table[k].rel_obj->idsu.nt.ntu.synonym
                                == MIR_Cont_argument
                               ) {

                                /*
                                | perform depend-count on Target of
                                | MIR_Cont_argumnt
                                */
                                mirci_count(target2->idsu.nt.rel_table[k].tar_obj,
                                            char_attrib, refs);
                                
                                break;
                                }
                            }
                        break;
                    }
                } /* for j (MIR_Cont_Directive) */

        } /* Switch (syn1)  */

    if (*refs > 1) {
       return;
       }

    } /* for all entries in main object */
}

/* mirci_count - Counts Depends-On References to Char. Attribute IN ONE OBJ */
/* mirci_count - Counts Depends-On References to Char. Attribute IN ONE OBJ */
/* mirci_count - Counts Depends-On References to Char. Attribute IN ONE OBJ */

static void
mirci_count(obj, char_attrib, refs)

IDS     *obj;           /* --> Object to be searched for Depends-On Object */
IDS     *char_attrib;   /* --> Characteristic Attribute object             */
int     *refs;          /* --> Integer that counts references              */

/*
INPUTS:

    "obj" points to an IDS that is a Non-Terminal to be searched for
    references to a Depends-On MIR Object via relationship "MIR_Depends_on".

    "char_attrib" points to the Characteristic Attribute for which references
    -to are supposed to be counted.

    "refs" is where we return the count of references to "char_attrib".


OUTPUTS:

    "refs" is incremented (by the number of references via a Depends-On
    MIR Object to the specified Char. Attribute) upon return.


BIRD'S EYE VIEW:
    Context:
        mirci_count_depends() has selected a particular object that it
        wishes to have scanned for the existence of a Depends-On MIR
        object that references the specified Characteristic Attribute.

    Purpose:
        This function is intended to count the number of references to the
        specified Characteristic Attribute in a SPECIFIED OBJECT.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set local reference count to 0>
    for (every entry in the specified object)
        if (relationship object is MIR_Depends_on)
            <set local target to target of MIR_Depends_on>
            for (every entry in local target)
                if (relationship object is MIR_Depends_on)
                    if (target of MIR_Depends_on is Characteristic Attribute>
                        <increment local reference count>
                    <break>

    <augment the passed reference count by the local reference count>

OTHER THINGS TO KNOW:

   See "mir_backend.c", function mirci_resolve_DEPENDS() documentation for
   an informative picture of what's going on here.

*/      

{
int     ref_count;      /* Local reference count                */
int     i,j;            /* Handy-Dandy General Purpose indices  */
IDS     *target;        /* Local target of a relationship       */


/* set local reference count to 0 */
ref_count = 0;

/* for (every entry in the specified object) */
for (i = 0; i < obj->idsu.nt.entry_count; i += 1) {

    /* if (relationship object is MIR_Depends_on) */
    if (obj->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym
        == MIR_Depends_on) {

        /* set local target to target of MIR_Depends_on */
        target = obj->idsu.nt.rel_table[i].tar_obj;

        /* for (every entry in local target) */
        for (j = 0; j < target->idsu.nt.entry_count; j += 1) {

            /* if (relationship object is MIR_Depends_on) */
            if (target->idsu.nt.rel_table[j].rel_obj->idsu.nt.ntu.synonym
                == MIR_Depends_on) {

                /* if (target of MIR_Depends_on is Characteristic Attribute> */
                if (target->idsu.nt.rel_table[j].tar_obj == char_attrib) {
                    /* increment local reference count */
                    ref_count += 1;
                    }
                break;
                }
            }
        }
    }

/* augment the passed reference count by the local reference count */
*refs = *refs + ref_count;

}

/* mirc_remove_OBJECT - Removes Table Entries, OIDs and then the Object */
/* mirc_remove_OBJECT - Removes Table Entries, OIDs and then the Object */
/* mirc_remove_OBJECT - Removes Table Entries, OIDs and then the Object */

              /* >>>>> This function is RECURSIVE <<<<< */
void
mirc_remove_OBJECT (mo_remove, bec)

IDS               *mo_remove;  /* --> IDS of MIR object to be removed */
back_end_context  *bec;        /* Pointer to back-end context block    */


/*
INPUTS:

    "mo_remove" points to the IDS for the MIR object to be "removed".

    "bec" is the address of the "back-end" context block, some of whose
    data-structures must be referenced by this function.


OUTPUTS:

    On success, the function returns.  At that time, the object that was
    passed on input has had all of the object's to which the entries in
    it's relationship table referred deleted.  All OIDs that the index may
    have held referring to this MIR object have been removed.

    On failure, a message is printed on stderr and an exit is performed.


BIRD'S EYE VIEW:
    Context:
        "mirc_remove_EC()" in this module has been called to delete a MIR
        Object that is an Entity-Class.  It found the object and verified that
        the object really is a MIR Object that represents an Entity-Class.
        It must now erase that MIR Object and all object's 'contained within
        it' from the Intermediate representation (along with all OIDs in the
        index that reference these objects).

        Alternatively, "mirc_reset()" has been called after compilation has
        finished for an MSL file and it is releasing it's list of "built-up"
        datatypes (generated when "TYPE" is parsed) for the current file.
        It checks the NTs that represent a "built-up" datatype and if the
        reference count is zero, it needs to dump the NT and everything it
        references which legal for reclamation.

    Purpose:
        This function takes care of processing *one* MIR Object by removing
        the OIDs from the index and then causes all MIR Objects "below" this
        object to be deleted by calling itself, and then deletes the actual
        MIR Object IDS.


ACTION SYNOPSIS OR PSEUDOCODE:

    switch (flavor of IDS)

        case I_T_OBJ_snumber:
        case I_T_OBJ_unumber:
        case I_T_OBJ_string:
            <decrement the reference count>
            if (reference count is zero)
                <perform I_Reclaim_IDS() call on the object>
            <return>

        case I_NT_OBJECT_DC:
        case I_NT_OBJECT_Rel:
            <decrement the reference count>
            <return>

        case I_NT_OBJECT:
            <break>

        case SLICE:
        case SUBREGISTER:
        default:
            <issue "Compiler Internal Error: invalid flavor to delete">
            <exit>

    (* At this point, we're processing a General Non-Terminal *)

    (* Blow off any OIDs that point to it *)
    for (each possible OID SMI)
        if (there is an active OID for this SMI)
            <perform mirci_remove_OID() on this OID>

    (* Process the Relationship Table entries *)
    for (each live relationship table entry)

        switch (MIR Relationship internal synonym)

            (* >>> STANDARD ACTION: Descend to Delete <<< *)
            case MIR_ID_Code
            case MIR_MCC_ID_Code
            case MIR_Text_Name
            case MIR_Contains
            case MIR_Indexed_By
            case MIR_Enum_Code
            case MIR_Enum_Text
            case MIR_Field_Name
            case MIR_Field_Code
            case MIR_Case_Code
            case MIR_Access
            case MIR_valueDefault
            case MIR_Display
            case MIR_Min_Int_Val
            case MIR_Max_Int_Val
            case MIR_Units
            case MIR_Dynamic
            case MIR_Required
            case MIR_Description
            case MIR_Directive_type
            case MIR_Echo
            case MIR_Predictable
            case MIR_Category
            case MIR_Symbol
            case MIR_Symbol_prefix
            case MIR_Cont_entityClass
            case MIR_Cont_attribute
            case MIR_Cont_attrPartition
            case MIR_Cont_attrGroup
            case MIR_Cont_eventPartition
            case MIR_Cont_eventGroup
            case MIR_Cont_event
            case MIR_Cont_directive
            case MIR_Cont_request
            case MIR_Cont_response
            case MIR_Cont_exception
            case MIR_Cont_argument
            case MIR_List_Type
            (*
            | MIR_Depends_OP - The target is a Terminal number, just drop it
            *)
            case MIR_Depends_OP:
                <invoke mirc_remove_OBJECT on target of entry>
                break;


            (* >>> NO_ACTION <<< *)
            case MIR_Contained_By
            case MIR_Counted_As
            case MIR_DC_Found_In_SMI
            case MIR_DC_ASN1_Class
            case MIR_DC_ASN1_Tag
            case MIR_DC_SMI_Name
            case MIR_DC_SMI_Code
            case MIR_DC_SMI_Template
            case MIR_DC_SMI_OTTemplate
            case MIR_DC_MCC_DT_Size
            case MIR_List_Entry
            (*
            | This is a special relationship used to go directly from
            | a directive MIR Object to the MIR Object that is the argument
            | of the directive-request.  We'll walk down thru the Request MIR
            | Object "naturally" to the argument via another path, so we don't
            | do anything with this relationship.
            *)
            case MIR_Cont_request_argument:

            (*
            | MIR_Variant_Sel - This only appears in an Entity-Class Object,
            | and its target is a Char. Attribute MIR Object that is
            | going to be blown away as the E-C is dissolved anyway, so
            | we need do nothing.
            *)
            case MIR_Variant_Sel:
                break;


            (* >>> SPECIAL ACTION <<< *)
            (*
            | If the target is a number, we'll go "delete" the reference
            | to it, otherwise it points to an Identifier attribute which
            | will get killed elsewhere.
            *)
            case MIR_DNS_Ident:
                if (target is not a Non-Terminal)
                    <invoke mirci_remove_OBJECT on target of entry>
                break;

            (*
            | The target must be a Non-Terminal, but it could
            | be of flavor "I_NT_OBJECT_DC" (a built-in Data-Construct that
            | may not be deleted) or "I_NT_OBJECT" (a built-up Data-Construct
            | that should be deleted if the reference count is zero).
            *)
            case MIR_Structured_As:
                switch (target's flavor)
                    case I_NT_OBJECT_DC:
                        <break>
                    case I_NT_OBJECT:
                        <decrement the target reference count>
                        if (result is zero)
                            <invoke mirci_remove_OBJECT on target of entry>
                        <break>
                    default:
                        <issue "Internal Compiler Error: Invalid Flavor">
                        <exit>
                <break>

            (*
            | We descend and process the target of this relationship *only*
            | if the target DOES contain another "MIR_Depends_on" entry!  In
            | this case, we've got a "Depends-On" Block.
            |
            | If it does not, then the target is a Characteristic Attribute
            | (which may be in the parent Entity-Class!) but we don't want
            | to blow away a Characteristic Attribute thru this relationship.
            *)
            case MIR_Depends_on:
                <scan the target for another "MIR_Depends_on" entry>
                if ( one is present)
                    <invoke mirci_remove_OBJECT on target of entry>
                <break>


            (* >>> ERROR SITUATION <<< *)
            (*
            | This relationship is only used in MIR Relationship Objects
            | which may never be deleted.  Consequently it is a severe internal
            | error if we even get here with this error.
            *)
            case MIR_Relationship_Name:

            (*
            | This relationship is (as of V2.0) only used in "The World" MIR
            | Object, which is never a candidate for deletion.
            case MIR_Special:
                <issue "Internal Compiler Error: Invalid Relationhip to delete">
                <exit>


    (* Blow off the MIR Object *)
    <return>

OTHER THINGS TO KNOW:

    On the first call to this function, the incoming argument is guaranteed
    to be a "Non-Terminal" (indeed one representing an Entity-Class).  On
    second and subsequent recursive calls, the argument can be *any* flavor
    of IDS (representing Terminal or Non-Terminal).

    We have special processing rules for each flavor in the 2nd and subsequent
    calls:

    * For Terminals (numbers and strings), we decrement the reference count
      and delete the object IF the reference count is ZERO.

    * For Non-Terminals representing Data-Constructs or MIR Relationship
      objects, decrement the reference but return regardless of the value
      (these MIR objects are never removed).

    * For 'General' Non-Terminals:
       - Delete the OIDs that reference it from the index
       - Process the Relationship Table entries according to the MIR
         relationship
       - Delete the input MIR Object 

*/      

/* #define DEBUG_WATCH */
{
int     i,j;            /* Handy-Dandy General Purpose Indices */
IDS     *target;        /* --> IDS that is a target of relationship under */
                        /*     consideration.                             */

#ifdef DEBUG_WATCH
static char *flavor_names[MAX_IDS_TYPE] = {
    "I_SLICE", "I_SUBREGISTER", "I_T_OBJ_snumber", "I_T_OBJ_unumber",
    "I_T_OBJ_string",  "I_NT_OBJECT_DC", "I_NT_OBJECT", "I_NT_OBJECT_Rel" };

printf("OBJ remove on %s with External Address %d\n",
       flavor_names[mo_remove->flavor], mo_remove->ex_add);
#endif

switch (mo_remove->flavor) {

    case I_T_OBJ_snumber:
    case I_T_OBJ_unumber:
    case I_T_OBJ_string:
        /* decrement the reference count */
        mo_remove->idsu.t.ref_count -= 1;

        /* if (reference count is zero) */
        if (mo_remove->idsu.t.ref_count == 0) {
            /* perform I_Reclaim_IDS() call on the object */
            I_Reclaim_IDS(mo_remove);
            }
        return;


    case I_NT_OBJECT_DC:
    case I_NT_OBJECT_Rel:
        /* decrement the reference count */
        mo_remove->idsu.nt.ref_count -= 1;
        return;


    case I_NT_OBJECT:
        /* (Handle this below . . .) */
        break;


    case I_SLICE:
    case I_SUBREGISTER:
    default:
        /* issue "Compiler Internal Error: invalid flavor to delete" */
        fprintf(stderr,
          MP(mp409,"mirc - Internal Error:\n       invalid flavor to delete, code %d\n"),
                mo_remove->flavor);
        exit(BAD_EXIT);
    }

/*
| At this point, we're processing a General Non-Terminal
*/

/* Blow off any OIDs that point to it
|
| for (each possible OID SMI)
*/
for (i = 0; i < MAX_OID_SMI; i += 1) {

    /* if (there is an active OID for this SMI) */
    if (mo_remove->idsu.nt.oid_backptr[i] != NULL) {

#ifdef DEBUG_WATCH
        printf ("OID REMOVE starting on Ex-Add %d  Arc %d\n",
                mo_remove->idsu.nt.oid_backptr[i]->ex_add,
                mo_remove->idsu.nt.oid_backarc[i]);
#endif

        /* perform mirci_remove_OID() on this OID */
        mirci_remove_OID(mo_remove->idsu.nt.oid_backptr[i],
                         mo_remove->idsu.nt.oid_backarc[i]);
        }
    }

/*
| Process the Relationship Table entries
|
| for (each live relationship table entry)
*/
for (i = 0; i < mo_remove->idsu.nt.entry_count; i += 1 ) {

    /* switch on MIR Relationship internal synonym */
    switch (mo_remove->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym) {

        /* >>> STANDARD ACTION: Descend to Delete <<< */
        /* >>> STANDARD ACTION: Descend to Delete <<< */
        /* >>> STANDARD ACTION: Descend to Delete <<< */
        case MIR_ID_Code:
        case MIR_MCC_ID_Code:
        case MIR_Text_Name:
        case MIR_Enum_Code:
        case MIR_Enum_Text:
        case MIR_Field_Name:
        case MIR_Field_Code:
        case MIR_Case_Code:
        case MIR_Access:
        case MIR_valueDefault:
        case MIR_Display:
        case MIR_Min_Int_Val:
        case MIR_Max_Int_Val:
        case MIR_Units:
        case MIR_Dynamic:
        case MIR_Required:
        case MIR_Description:   
        case MIR_Directive_type:
        case MIR_Echo:
        case MIR_Predictable:
        case MIR_Category:
        case MIR_Symbol:
        case MIR_Symbol_prefix:
        case MIR_Cont_entityClass:
        case MIR_Cont_attribute:
        case MIR_Cont_attrPartition:
        case MIR_Cont_attrGroup:
        case MIR_Cont_eventPartition:
        case MIR_Cont_eventGroup:
        case MIR_Cont_event:
        case MIR_Cont_directive:
        case MIR_Cont_request:
        case MIR_Cont_response:
        case MIR_Cont_exception:
        case MIR_Cont_argument:
        case MIR_List_Type:
        /*
        | MIR_Depends_OP - The target is a Terminal number, just drop it
        */
        case MIR_Depends_OP:
            /* invoke mirc_remove_OBJECT on target of entry */
            mirc_remove_OBJECT(mo_remove->idsu.nt.rel_table[i].tar_obj, bec);
            break;



        /* >>> NO_ACTION <<< */
        /* >>> NO_ACTION <<< */
        /* >>> NO_ACTION <<< */
        case MIR_Contains:
        case MIR_Indexed_By:
        case MIR_Contained_By:
        case MIR_Counted_As:
        case MIR_DC_Found_In_SMI:
        case MIR_DC_ASN1_Class:
        case MIR_DC_ASN1_Tag:
        case MIR_DC_SMI_Name:
        case MIR_DC_SMI_Code:
        case MIR_DC_SMI_Template:
        case MIR_DC_SMI_OTTemplate:
        case MIR_DC_MCC_DT_Size:
        case MIR_List_Entry:
        /*
        | This is a special relationship used to go directly from
        | a directive MIR Object to the MIR Object that is the argument
        | of the directive-request.  We'll walk down thru the Request MIR
        | Object "naturally" to the argument via another path, so we don't
        | do anything with this relationship.
        */
        case MIR_Cont_request_argument:

        /*
        | MIR_Variant_Sel - This only appears in an Entity-Class Object,
        | and its target is a Char. Attribute MIR Object that is
        | going to be blown away as the E-C is dissolved anyway, so
        | we need do nothing.
        */
        case MIR_Variant_Sel:
            break;



        /* >>> SPECIAL ACTION <<< */
        /* >>> SPECIAL ACTION <<< */
        /* >>> SPECIAL ACTION <<< */
        /*
        | If the target is a number, we'll go "delete" the reference
        | to it, otherwise it points to an Identifier attribute which
        | will get killed elsewhere.
        */
        case MIR_DNS_Ident:
            /* if (target is not a Non-Terminal) */
            if (mo_remove->idsu.nt.rel_table[i].tar_obj->flavor !=I_NT_OBJECT){
                /* invoke mirc_remove_OBJECT on target of entry */
                mirc_remove_OBJECT(mo_remove->idsu.nt.rel_table[i].tar_obj,
                                    bec);
                }
            break;


        /*
        | The target must be a Non-Terminal, but it could
        | be of flavor "I_NT_OBJECT_DC" (a built-in Data-Construct that
        | may not be deleted) or "I_NT_OBJECT" (a built-up Data-Construct
        | that shoud be deleted if the reference count is zero).
        */
        case MIR_Structured_As:

            /* Switch on target's flavor */
            switch (mo_remove->idsu.nt.rel_table[i].tar_obj->flavor) {

                case I_NT_OBJECT_DC:
                    break;

                case I_NT_OBJECT:
                    /* decrement the target reference count */
                    /* if (result is zero)                  */
                    if ((mo_remove->idsu.nt.rel_table[i].tar_obj->idsu.nt.ref_count
                        -= 1) == 0) {
                        /* invoke mirc_remove_OBJECT on target of entry */
                        mirc_remove_OBJECT(
                                       mo_remove->idsu.nt.rel_table[i].tar_obj,
                                       bec);
                        }
                    break;

                default:
                    /* issue "Internal Compiler Error: Invalid Flavor" */
                    fprintf(stderr,
                            MP(mp410,"mirc - Internal Error:\n       MIR_Structured_As: Invalid flavor to delete\n")
                            );
                    exit(BAD_EXIT);
                }

            break;


        /*
        | We descend and process the target of this relationship *only*
        | if the target DOES contain another "MIR_Depends_on" entry!  In
        | this case, we've got a "Depends-On" Block.
        |
        | If it does not, then the target is a Characteristic Attribute
        | (which may be in the parent Entity-Class!) but we don't want
        | to blow away a Characteristic Attribute thru this relationship.
        */
        case MIR_Depends_on:

            target = mo_remove->idsu.nt.rel_table[i].tar_obj;

            /* scan the target for another "MIR_Depends_on" entry */
            for (j = 0; j < target->idsu.nt.entry_count; j += 1) {

                /* if ( one is present) */
                if (target->idsu.nt.rel_table[j].rel_obj->idsu.nt.ntu.synonym
                    == MIR_Depends_on) {

                    /* invoke mirc_remove_OBJECT on target of entry */
                    mirc_remove_OBJECT(target, bec);

                    /* All we need is one */
                    break;
                    }
                }
            break;


        /* >>> ERROR SITUATION <<< */
        /* >>> ERROR SITUATION <<< */
        /* >>> ERROR SITUATION <<< */
        /*
        | This relationship is only used in MIR Relationship Objects
        | which may never be deleted.  Consequently it is a severe internal
        | error if we ever get here with this error.
        */
        case MIR_Relationship_Name:

        /*
        | This relationship is (as of V2.0) only used in "The World" MIR
        | Object, which is never a candidate for deletion.
        */
        case MIR_Special:
        default:
            /* issue "Internal Compiler Error: Invalid Relationhip to delete" */
            fprintf(stderr,
                  MP(mp411,"mirc - Internal Error:\n        Invalid Relationhip to delete code %d\n"),
                  mo_remove->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym);
            exit(BAD_EXIT);
        }
    }

/* Blow off the MIR Object */
I_Reclaim_IDS(mo_remove);

}

/* mirci_remove_OID - Removes OID from Intermediate Index */
/* mirci_remove_OID - Removes OID from Intermediate Index */
/* mirci_remove_OID - Removes OID from Intermediate Index */

static void
mirci_remove_OID(back_ptr, arc)

IDS            *back_ptr;   /* --> IDS of slice or subregister for slot to  */
                            /*     being deleted                            */
unsigned int   arc;         /* --> Arc number whose slot is sought          */


/*
INPUTS:

    "back_ptr" points to the IDS for the 'right-most' slice (or subregister)
    in the index for the OID to be deleted.

    "arc" is the arc number in "back_ptr" that is to be deleted.

OUTPUTS:

    On success, the function returns.   All OIDs arcs (slots) in the index (and
    any subregister) that were used exclusively in the OID being deleted have
    been deleted.

    On failure, a message is printed on stderr and an exit is performed.


BIRD'S EYE VIEW:
    Context:
        "mirci_remove_OBJECT()" in this module has been called to delete a MIR
        Object that is an Entity-Class.  It must remove any OIDs in the index
        that point to that object.

    Purpose:
        This function takes care of processing *one* OID out of the index.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (back pointer is a SUBREGISTER)
        <acquire subregister's backpointer and set it into slice-to-scan
        for (every active entry in the slice-to-scan)
            if (slot's arc matches input arc)
                <set slot's 'next' cell to point to subregister's lower-level>
                <set lower-level's backpointer to point to slice-to-scan
                <reclaim the subregister>
                <return>
        <issue "Internal Compiler Error: Subregister down pointer missing">
        <exit>

    (* Handle first search for arc separately *)
    <set the slice-to-scan to the backpointer to slice>
    for (every active entry in the slice-to-scan)
        if (arc matches slot entry)
            <signal "Entry Deleted">
            <arrange to delete this entry>
            <decrement the active entry-count of the slice>
        if (get-from is not equal to put-to and get-from is legal)
            <copy the entry>

    if (Entry was NOT Deleted)
        <issue "Internal Compiler Error: OID backpointer not found">
        <exit>

    <set slice-to-search-for to current slice>
    <set slice-to-scan to current slice's backpointer>

    if (slice-to-search-for's entry count is zero)
        <reclaim the slice>
        (*
           Continue upward and delete the arc above, 'cause there's nothing
           below.
        *)
    else
        <return successfully>

    (* At this point, "slice-to-scan" could be a Subregister or a Slice *)

    (* Now handle all levels above the bottom-most level *)
    do
        if (slice-to-scan is in reality a subregister)
            <acquire subregister's backpointer as slice-to-scan>
            (* we now know that what we're looking for is really a down pointer
               to the subregister *)
            for (every active entry in the slice-to-scan)
                if (slot's "next" pointer matches (the subregister))
                    <set slot's "next" cell --> to subregister's ntobject>
                    <set ntobject's correct oid backpointer to point to slice>
                    <reclaim the subregister>
                    <return successfully>
            <issue "Internal Compiler Error: Subregister down pointer
               missing">
            <exit>

        for (every active entry in slice-to-scan)
            if (slot 'next' cell points to slice-to-search-for)
                <signal "Entry Deleted">
                <arrange to delete this entry>
                <decrement the active entry-count of the slice>
            if (get-from is not equal to put-to and get-from is legal)
                <copy the entry>

        if (entry was NOT Deleted)
            <issue "Internal Compiler Error: OID backpointer not found">
            <exit>

        <set slice-to-search-for to slice-to-scan>
        <set slice-to-scan to slice-to-scan's backpointer>

        if (slice-to-search-for's entry count is zero)
            <reclaim the slice>
        else
            <return successfully>

     while (slice-to-scan is NOT NULL)

OTHER THINGS TO KNOW:

    The logic in this function is a bit more complicated than I wish it were,
    and here's why.

    On entry, we have an "arc number" that indicates an entry (in an index
    slice) that needs to be deleted.  Once we 'step up' one level, the entry
    to be deleted is indicated by the address of the *slice* we're just now
    'stepping up' from (rather than an arc number).

    Additional complexity is introduced be the fact that the "slice" we're
    about to "scan" for a downward pointing entry (as mentioned in the previous
    paragraph) may in fact be a subregister.  In this case, the subregister
    must be deleted and the slice 'above' it adjusted so that it's downward
    pointing entry is correct, as well as adjusting the things below the
    subregister (either a Non-Terminal or a lower-slice) so that they point up
    to the slice 'above' (instead of the now-deleted subregister).

    This is the way it is with Object Identifiers.
*/      

{
IDS     *slice_to_scan;  /* --> Slice to scan (next)                        */
IDS     *slice_sought;   /* --> Slice for which an entry in s_t_s is sought */
IDS     *subreg;         /* --> A subregister to be dealt with somehow      */
IDS     *nt_object;      /* --> A Non-Terminal object (as from a subreg)    */
int     i,j;             /* Handy Dandy general-purpose Indices             */


/* if (back pointer is a SUBREGISTER) */
if (back_ptr->flavor == I_SUBREGISTER) {

    /* acquire subregister's backpointer and set it into slice-to-scan */
    slice_to_scan = back_ptr->idsu.s.backptr;

    /* for (every active entry in the slice-to-scan) */
    for (i = 0; i < slice_to_scan->idsu.i.entry_count; i++ ) {

        /* if (slot's arc matches input arc) */
        if (slice_to_scan->idsu.i.slice[i].iso_arc == arc) {

            /* set slot's 'next' cell to point to subregister's lower-level */
            slice_to_scan->idsu.i.slice[i].next =
                back_ptr->idsu.s.lower_levels;

            /* set lower-level's backpointer to point to slice-to-scan */
            back_ptr->idsu.s.lower_levels->idsu.i.backptr = slice_to_scan;

            /* reclaim the subregister */
            I_Reclaim_IDS(back_ptr);

            return;
            }
        }

    /* issue "Internal Compiler Error: Subregister down pointer missing" */
    fprintf(stderr,
            MP(mp412,"mirc - Internal Error:\n       Subregister down pointer missing\n")
            );
    exit(BAD_EXIT);
    }

/*
| Handle first search for arc separately
*/

/* set the slice-to-scan to the backpointer to slice*/
slice_to_scan = back_ptr;

/* for (every active entry in the slice-to-scan) */
for (i = j = 0;
     i < slice_to_scan->idsu.i.entry_count;
     i += 1, j += 1 ) {

    /* if (arc matches slot entry) */
    if (slice_to_scan->idsu.i.slice[i].iso_arc == arc) {

        /*
        | signal "Entry Deleted": arrange to delete this entry
        | 
        | We 'signal' by making j not equal to i.  By bumping 'j' to point to
        | the next entry, j becomes the 'source' of what we copy on top of
        | the 'ith' entry (to delete the ith entry the first time around and
        | then 'shuffle-downward' each succeeding entry).
        |
        | Note rather than incrementing j, we always set it to one more than
        | i to preclude horrible weird things that might happen should there
        | be a duplicate arc number (which is pretty horrible and weird in
        | and of itself).
        */
        j = i + 1;

        /*
        | decrement the active entry-count of the slice
        |
        | The prevents us from copying garbage on the last iteration of this
        | loop.
        */
        slice_to_scan->idsu.i.entry_count -= 1;
        }

    /*
    | if (get-from is not equal to put-to and get-from is legal)
    |
    | ... in other words, "has 'Entry Deleted' been signalled above"?
    */
    if (i != j && i < slice_to_scan->idsu.i.entry_count) {
        /* copy the entry */
        slice_to_scan->idsu.i.slice[i] = slice_to_scan->idsu.i.slice[j];
        }
    }

/* if (Entry was NOT Deleted) */
if (i == j) {
    fprintf(stderr,
            MP(mp413,"mirc - Internal Error:\n       OID backpointer not found\n")
            );
    exit(BAD_EXIT);
    }

/*
| Now we're about to 'step-up' one level in the index to the slice above
| the lowest point we just handled above.
|
| In general from here on 'up', we're interested in finding the entry in 
| the slice above that points 'down' to the slice we just processed.  We
| have to 'skip over' and adjust (or possibly delete) any subregisters we
| encounter.
*/

/* set slice-to-search-for to current slice */
slice_sought = slice_to_scan;

/* set slice-to-scan to current slice's backpointer */
slice_to_scan = slice_to_scan->idsu.i.backptr;

/* if (slice-to-search-for's entry count is zero) */
if (slice_sought->idsu.i.entry_count == 0) {

    /* reclaim the slice */
    I_Reclaim_IDS(slice_sought);

    /*
    |  Continue upward and delete the arc & entry above, 'cause there's
    |  nothing below now.
    */
    }
else {
    /*
    | ... the slice we just 'stepped-up from' had other entries in it.
    | Consequently there is no call to delete it from the hierarchy above it.
    | We've succeeded.
    */
    return;
    }

/*
| At this point, "slice-to-scan" could be a Subregister or a Slice 
*/

/* Now handle all levels above the bottom-most level */
do {

    /*
    |  SUBREGISTER
    */

    /* if (slice-to-scan is in reality a subregister) */
    if ( slice_to_scan->flavor == I_SUBREGISTER) {

        /*
        | acquire subregister's backpointer as slice-to-scan
        |
        | Here we're "skipping over" a subregister to the slice above it
        | which we're going to scan for the down pointer to the subregister
        | below it.
        */
        subreg = slice_to_scan; /* Remember the subregister we're looking for*/
        slice_to_scan = slice_to_scan->idsu.s.backptr;

        /*
        | We now know that what we're looking for is really a down pointer
        | to the subregister, and "slice_to_scan" really points at a slice.
        */

        /* for (every active entry in the slice-to-scan) */
        for (i = 0; i < slice_to_scan->idsu.i.entry_count; i++ ) {

            /* if (slot's "next" pointer matches (the subregister)) */
            if (slice_to_scan->idsu.i.slice[i].next == subreg) {

                /* set slot's "next" cell --> to subregister's ntobject */
                slice_to_scan->idsu.i.slice[i].next = subreg->idsu.s.ntobject;

                /*
                | Set ntobject's correct oid backpointer to point to slice.
                |
                | Because the NT points back at a Subregister, there is no need
                | to do fancy disambiguation:  There should be only one back
                | pointer from the NT to the old subregister.  It is this OID
                | backpointer we need to change.
                */
                nt_object = subreg->idsu.s.ntobject;
                for (j = 0; j < MAX_OID_SMI; j++) {
                    if (nt_object->idsu.nt.oid_backptr[j] == subreg) {
                        break;
                        }
                    }

                /* if (Entry was NOT Deleted) */
                if (j >= MAX_OID_SMI) {
                    fprintf(stderr,
                            MP(mp414,"mirc - Internal Error:\n       Non-Terminal OID backpointer not found\n")
                            );
                    exit(BAD_EXIT);
                    }

                /* SET IT */
                nt_object->idsu.nt.oid_backptr[j] = slice_to_scan;

                /* reclaim the subregister */
                I_Reclaim_IDS(subreg);

                /* return successfully */
                return;
                }
            }

        /* issue "Internal Compiler Error: Subregister down pointer missing" */
        fprintf(stderr,
                MP(mp412,"mirc - Internal Error:\n       Subregister down pointer missing\n")
                );
        exit(BAD_EXIT);
        }

    /*
    |  SLICE
    */

    /* for (every active entry in slice-to-scan) */
    for (i = j = 0;
         i < slice_to_scan->idsu.i.entry_count;
         i += 1, j += 1 ) {

        /* if (slot 'next' cell points to slice-to-search-for) */
        if (slice_to_scan->idsu.i.slice[i].next == slice_sought) {

            /*
            | signal "Entry Deleted": arrange to delete this entry
            | 
            | We 'signal' by making j not equal to i.  By bumping 'j' to point
            | to the next entry, j becomes the 'source' of what we copy on top
            | of the 'ith' entry (to delete the ith entry the first time
            | around and then 'shuffle-downward' each succeeding entry).
            |
            | Note rather than incrementing j, we always set it to one more
            | than i to preclude horrible weird things that might happen should
            | there be a duplicate arc number (which is pretty horrible and
            | weird in and of itself).
            */
            j = i + 1;

            /*
            | decrement the active entry-count of the slice
            |
            | The prevents us from copying garbage on the last iteration of
            | this loop.
            */
            slice_to_scan->idsu.i.entry_count -= 1;
            }


        /*
        | if (get-from is not equal to put-to and get-from is legal)
        |
        | ... in other words, "has 'Entry Deleted' been signalled above"?
        */
        if (i != j && i < slice_to_scan->idsu.i.entry_count) {
            /* copy the entry */
            slice_to_scan->idsu.i.slice[i] = slice_to_scan->idsu.i.slice[j];
            }
        }

    /* if (entry was NOT Deleted) */
    if (i == j) {
        fprintf(stderr,
                MP(mp415,"mirc - Internal Error:\n       OID backpointer to slice not found\n")
                );
        exit(BAD_EXIT);
        }


    /* set slice-to-search-for to slice-to-scan */
    slice_sought = slice_to_scan;

    /* set slice-to-scan to slice-to-scan's backpointer */
    slice_to_scan = slice_to_scan->idsu.i.backptr;

    /* if (slice-to-search-for's entry count is zero) */
    if (slice_sought->idsu.i.entry_count == 0) {

        /* reclaim the slice */
        I_Reclaim_IDS(slice_sought);

        /*
        |  Continue upward and delete the arc & entry above, 'cause there's
        |  nothing below now.
        */
        }
    else {
        /* return successfully */
        return;
        }
    }

    /* while (slice-to-scan is NOT NULL) */
    while (slice_to_scan != NULL);

    /* NOTE: If we fall out the bottom, basically the index is gone! */
}
