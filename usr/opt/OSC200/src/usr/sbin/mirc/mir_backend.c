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
static char *rcsid = "@(#)$RCSfile: mir_backend.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 21:59:04 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991, 1992, 1993.
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
 * Module MIR_BACKEND.C
 *      Contains functions that make up the "back-end" of the MIR MSL
 *      compiler.  This module corresponds roughly to the DECnet-ULTRIX
 *      compiler module "dict_cache.c".
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   January 1991
 *          with credit to B.M. England for design and inspiration.
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines have need of network management
 *       architecture information in order to properly process incoming
 *       management request messages.  This information must reside in a
 *       Management Information Repository, accessible by ISO Object ID.
 *
 *    Purpose:
 *       This module contains the functions that make up the "back-end" of
 *       the MIR's MSL Compiler which is used to compile the MSL files that
 *       contain the descriptions (of the objects to be managed) to be
 *       loaded into a MIR.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *   Version    Date            Who             What
 *      V0.0    January 1991    D. D. Burns     Preliminary Version
 *      V1.0    March 1991      D. D. Burns     V1.0 of Compiler
 *      V1.5    June 1991       D. D. Burns     V1.5 of Compiler, support ECO
 *                                              42 & multiple SMI compilations
 *      V1.6    Sept 1991       D. D. Burns     V1.6 of Compiler, support the
 *                              "IDENTIFIER=" clause by implementing a general
 *                              purpose forward-reference resolution scheme.
 *                                              
 *      V1.7    Oct 1991        D. D. Burns     V1.7 of compiler, remove SNMP
 *                              'enhancement' that precluded "containment" 
 *                              relationships from being created (needed for
 *                              SNMP PE).
 *
 *      V1.8    Jan 1992        D. D. Burns     Delivered w/SNMP PE.
 *      V1.9    Mar 1992        D. D. Burns     Preliminary MCC version.
 *
 *              6-Aug-1992      Burgess         Changed default directory
 *				specification to /usr/etc/builtin_types.dat
 *                                               ---------
 *
 *      V1.99   Oct 1992        D. D. Burns     Add support for Merge & Augment
 *      V2.00   Jan 1993        D. D. Burns     Final delivery

Module Overview:

This module contains all the backend functions used by the MIR "yacc" parser
to compile MSL files.

A chunk of compiled information is converted into intermediate form and stored
in an "Intermediate Data Structure", or "IDS".  All "things" that the
compiler deals with are represented as an IDS initially, and then during
the final passes (in module 'mir_external.c') the IDS are converted to the
final external form (as a 'MIR Object' and written to the output binary MIR
database file.

IDSs are used to represent Strings, Signed and Unsigned Numbers and
Non-Terminal MIR Objects derived from the MSL input stream.
Addititonally they are used to represent:

   * "slices" of the OID index
   * "subregisters" in the OID index
   * MIR Relationship Objects (Non-Terminals created by the compiler separate
     and apart from the MSL-derived Non-Terminals that represent things like
     attributes and entities).

The compiler maintains internal lists of these IDS "by-type" (for use
in the final pass) and also lists for other purposes, like recognizing
datatypes that are 'predefined' by an SMI.

These "back-end" functions make use of functions that create and manipulate
the "Intermediate" form of the MSL data being compiled.  These support
functions are found in module "mir_intermediate.c".


MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name           Synopsis

mirc_init_backend       Initialize the backend of compiler

mirc_create_object      Creates a non-terminal IDS that stands for some SMI
                        construct (entity class, attribute, argument etc) that
                        is NOT a datatype, and performs other processing on
                        the newly created (IDS) object.  New object becomes
                        "current-object" being compiled into.

mirc_add_relationship   Adds a specified relationship to the relationship
                        table of the current MIR object.  The target is
                        an IDS that already exists.

mirc_add_rel_string     Adds a specified relationship to the relationship
                        table of the current MIR object.  The target is
                        supplied as a pointer to a string (which is 
                        converted into an IDS representing a terminal MIR
                        object of MIR-type "string") before the relationship
                        is added.

mirc_add_rel_number     Adds a specified relationship to the relationship
                        table of the current MIR object.  The target is
                        supplied as an integer (which is converted into an
                        IDS representing a terminal MIR object ("number"))
                        before the relationship is added.

mirc_create_dataconstruct Creates a non-terminal IDS that stands for either a
                        "built-IN" datatype (defined by an SMI and passed into
                        the compiler via the "builtin_types.dat" file) OR a
                        "built-UP" datatype (defined by a qualified reference
                        to a "built-IN" datatype... ie defined by the use of
                        a "TYPE" statement in an MSL file).  This function
                        maintains the definitions on separate internal
                        compiler lists so that "mir_find_datatype()" can
                        search each list appropriately.

mirc_find_datatype      Searches first the "built-in" list then the "built-up"
                        list of datatypes for a match given a datatype string
                        name.  This supports the yacc parser when the parser
                        encounters what should be the name of a previously
                        defined datatype in the MSL file.

mirc_find_DC_code       Given the IDS for a builtin datatype, this function
                        returns the code (number) by which this datatype is
                        known in it's SMI (ie the target of MIR_DC_SMI_Code).
                        This supports error checking for "subrange" processing
                        by the yacc code.

mirc_find_obj_CODE      Given the IDS for any (SMI) Object, this function
                        finds the specified "ID" Code (MCC or Standard) that
                        is associated with it.

mirc_find_obj_NAME      Given the IDS for any (SMI) Object, this function
                        finds the name that is associated with it (ie whatever
                        is the target of MIR_Text_Name).

mirc_find_rel_IDS       Returns the IDS that represents a compiler
                        relationship when the compiler relationship is
                        provided as a string (ie "MIR_Structured_As").
                        This is needed to allow the parser to parse the
                        "builtin_types.dat" file at start-up time, where
                        compiler relationships are mentioned by name (string).

mirc_find_obj_IDS       Finds (by name) the IDS for the object named as a
                        target (of a specified relationship) by searching 
                        the relationship table of the "current-object".  This
                        is used to lookup previously compiled attributes (by
                        name) so they may be included in an entity-specific
                        partition group.  (Also used to resolve forward
                        references made in an "IDENTIFIER=" clause).

mirc_find_fixedfield    Finds the code for a record's particular "fixed-field"
                        (by field-name) in the current definition of an
                        instance of a RECORD datatype. (Supports processing
                        for a "CASE" statement in variant RECORD definition).

mirc_record_idcodes     Records in the compiler's back-end stack the MCC or
                        standard ID code for a (MIR Object) entity-element, so
                        that it's OID (and the OIDs of contained objects) may
                        be computed.

mirc_create_fwd_ref     Creates a "forward reference block" to carry the
                        information needed to resolve a forward-reference
                        situation.  Different blocks are created based on
                        the situation: "IDENTIFIER=()", "DEPENDS ON=".

mirc_compute_oid        Computes MCC or Standard (DNA) ISO Object ID for
                        current MIR object.

mirc_rec_to_vrec        Converts an instance of a "RECORD" datatype-definition
                        to a "VARIANT RECORD" (occurs when a CASE statement is
                        parsed during processing of an instance of a RECORD
                        definition. . . the parser now sees that what appeared
                        to be just a RECORD is in fact a VARIANT RECORD).

mirc_use_parent         Switch "the current MIR object being compiled-into"
                        to the parent of the "current-object" being 
                        "compiled into".

mirc_get_current        Returns the IDS of the "current-object" being compiled
                        into.

mirc_set_current        Allows the "current-object" to be switched to a
                        specified IDS.

mirc_reset              Reset the backend of the compiler to allow starting
                        another parse on another MSL file.

mirc_record_smi         Sets compiler for new SMI whose builtin-dataconstructs
                        are being processed from builtin_types file at init.

mirc_set_smi            Selects proper compiler internal tables for SMI being
                        compiled.

mirc_get_def_status     Returns the 'definition-status' of the current object.
                        (Supports parser operation for Merge/Augment)

mirc_set_def_status     Sets the 'definition-status' of the current object.
                        (Supports parser operation for Merge/Augment)

mirc_error              Returns a string given a MIR Compiler error status

mirc_keyword            Records a string as a valid keyword or verifies a
                        string as a valid keyword

caseless_str_equal      Caseless String Comparison (courtesy B.M.E.)



MODULE INTERNAL FUNCTIONS:

Function Name           Synopsis

mirci_resolve_fwd_ref   Resolves all forward references established by
                        compilation of last MSL file (triggered by
                        mirc_reset()).

mirci_resolve_DEPENDS   Performs forward-reference processing required for
                        DEPENDS-ON clause.  (Supports mirci_resolve_fwd_ref()).

mirci_find_attribute    Finds a attribute of a specific kind (in support of
                        mirci_resolve_fwd_ref() and mirci_resolve_DEPENDS()).

mirci_build_rel_list    Build MIR Compiler internal list of all MIR or SMI
                        defined Relationships (part of initialization)

mirci_register_rel      Registers one MIR-defined or SMI-defined relationship
                        by creating a MIR object (IDS) to represent this
                        relationship and then recording the existence of this
                        IDS in the compiler's internal list of relationships.
                        (Supports "mirci_build_rel_list()").

mirci_read_builtin      Registers all SMI-defined ("built-in") data-constructs
                        by reading external file describing same.

*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef VMS
#include <malloc.h>
#endif

/* Request definitions for compiler modules from "mir.h" */
#define MIR_COMPILER
#include "mir.h"

/* Some MCC symbol definitions */
#include "mcc_vea_def.h"

#ifndef strcasecmp
extern int strcasecmp PROTOTYPE((const char *s1, const char *s2));
#endif

/*
|   Back-End Context Block
|
|   The "back-end" context block is used by all the functions in this
|   module.  A pointer to this block is established here at the time
|   the back-end is initialized.
*/
static
back_end_context  *bec=NULL;


/*
|  This corresponds directly with the "-Xi" and "-Xb" compiler flags
|  defined in mir_frontend.c
*/
extern BOOL supplementary_info;
extern BOOL supplementary_binary;


/*
|  Defined in mir_yacc.y -- needed here to enable a reset to "CM_NORMAL"
*/
extern CM_TYPE  compiler_mode;


/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp100();
extern char *mp101();
extern char *mp102();
extern char *mp103();
extern char *mp104();
extern char *mp105();
extern char *mp106();
extern char *mp107();
extern char *mp108();
extern char *mp109();
extern char *mp110();
extern char *mp111();
extern char *mp112();
extern char *mp113();
extern char *mp114();
extern char *mp115();
extern char *mp116();
extern char *mp117();
extern char *mp118();
extern char *mp119();
extern char *mp120();
extern char *mp121();
extern char *mp122();
extern char *mp123();
extern char *mp124();
extern char *mp125();
extern char *mp126();
extern char *mp127();
extern char *mp128();
extern char *mp129();
extern char *mp130();
extern char *mp131();
extern char *mp132();
extern char *mp133();
extern char *mp134();
extern char *mp135();
extern char *mp136();
extern char *mp137();
extern char *mp138();
extern char *mp139();
extern char *mp140();
extern char *mp141();
extern char *mp142();
extern char *mp143();
extern char *mp144();
extern char *mp145();
extern char *mp146();
extern char *mp147();
extern char *mp148();
extern char *mp149();
extern char *mp150();
extern char *mp151();
extern char *mp152();
extern char *mp153();
extern char *mp154();
extern char *mp155();
#endif
/*
|
|       Prototypes for module-local functions
|
*/

/* mirci_resolve_fwd_ref - Resolve All Forward References */
static void
mirci_resolve_fwd_ref PROTOTYPE(());

/* mirci_resolve_DEPENDS - Perform Fwd-Ref Processing for DEPENDS ON clause */
static void
mirci_resolve_DEPENDS PROTOTYPE((
FRB     *           /*-> Forward-Reference Block for a DEPENDS ON clause */
));

/* mirci_find_attribute - Finds a attribute of a specific kind */
static IDS *
mirci_find_attribute PROTOTYPE((
char            *,         /*--> Attribute's name as a string */
int              ,         /* Code for standard partition     */
mir_relationship           /* Relationship used from EClass   */
));

/* mirci_rel_list - Build MIR Compiler internal list of Relationships*/
static void
mirci_build_rel_list PROTOTYPE(());

/* mirci_read_builtin - Read In the Builtin-Types */
MC_STATUS
mirci_read_builtin PROTOTYPE(());

/* mirci_register_rel - Register a MIR Relationship Non-Terminal */
static void
mirci_register_rel PROTOTYPE((
char                *,  /* The Name of the Relationship             */
mir_relationship     ,  /* The internal compiler code for this rel. */
int                     /* Value to be used after "17" in Object ID */
));


/* mirc_create_dataconstruct - Create a Built-In or Built-Up Data Construct */
/* mirc_create_dataconstruct - Create a Built-In or Built-Up Data Construct */
/* mirc_create_dataconstruct - Create a Built-In or Built-Up Data Construct */

MC_STATUS
mirc_create_dataconstruct (dc_name, dc_code, built_in)

char    *dc_name;           /*-> Name of builtup datatype               */
int     dc_code;            /* SMI-defined code for builtup datatype    */
BOOL    built_in;           /* TRUE: Built-In, FALSE: Built-Up          */

/*
INPUTS:

    "dc_name" - points to the name of the to-be-created "built-up" or
    "built-in" datatype (as indicated by "built_in").

    "dc_code" is the numeric code by which this new datatype
    is known.

    "built_in" - indicates (essentially) whether we're defining data constructs
    from a parse of the builtin_types file (builtin=TRUE) or we are parsing
    "TYPE" statements in the user's MSL file (builtin=FALSE).

OUTPUTS:

    The function returns one of:

    "MC_OUT_OF_MEMORY" - Ran out of memory during the attempted create the
            object and enter the necessary relationships in it's relationship
            table.

    "MC_SUCCESS" - Create was successful.  On a success return, the "current"
            object (from the parser's perspective) is set to the newly
            created object in order that any additional ancillary information
            (needed to describe this dataconstruct) may be added into
            it by adding relationship entries in it's relationship table.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it recognizes enough of a description
        of a new datatype (as defined by a "TYPE" statement or
        "SMI_BUILTIN_TYPE) to know it's name and code.

    Purpose:
        This function takes care of the details of creating a MIR Intermediate
        Data Structure to receive further information as it is parsed from the
        MSL for this new dataconstruct.  The textname and code are
        entered into the relationship table and its existence is recorded
        on the compiler's internal "built-up" or "built-in" list.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (compiler mode is AUGMENT)
        if (datatype is already defined)
            <mark the IDS for the dataconstruct as PREDEFINED_TYPE>
            <return it successfully>

    if (create of non-terminal failed)
        <return status>

    <mark the IDS for the dataconstruct as CREATED>
    <make current object be recorded in new object as its parent>
    <make this the current mir object>
    
    if (it is a "builtin" dataconstruct)
        if (create for terminal for smi code failed)
            <return status>
        if (add of relationship for smi definition failed)
            <return status>

    if (create for terminal for code failed)
        <return status>

    if (it is a "builtin" dataconstruct)
        if (add of relationship for code with MIR_DC_SMI_Code failed)
            <return status>
    else
        if (add of relationship for code with MIR_ID_Code failed)
            <return status>

    if (create for terminal for name failed)
        <return status>

    if (it is a "builtin" dataconstruct)
        if (add of relationship for name with MIR_DC_SMI_Name failed)
            <return status>
    else
        if (add of relationship for name with MIR_Text_Name failed)
            <return status>

    if (create of index data structure storage failed)
        <return status>

    if (allocation of storage to hold new datatype name failed)
        <return status>

    <copy new datatype name to allocated storage>
    <add the index data structure into the top of the "built-up" or
     "built-in" datatype list>

    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    This function creates a MIR object that corresponds to a MSL-defined
    "built-up" datatype or an SMI-defined "built-in" datatype.

    Note that while this function creates an IDS to receive this information,
    and that this IDS will ultimately appear in the output MIR file, the IDS
    is not registered with an ISO object ID in the index, (unless an "OID ="
    phrase appears in a definition for an SMI-defined builtin type) nor is it
    shown as a "child" of the current object (by way of an entry in the
    relationship table of the parent as is the case in a call to
    "mir_create_object()"). 

    In this way, built-up (and also built-in) datatypes do not "belong" to
    anything in the MIR (that is, they hold no place in the hierarchy of SMI
    objects).  In essence, the datatypes belong to the compiler in the context
    of the compilation.

    While the "parent" pointer in the IDS for the newly created built-up
    datatype object is indeed set to point to the parent (i.e. whatever was the
    "current" object at the time this function was called), this is done
    simply so that the parser may "return" to the previous object once the
    compilation of this built-up datatype description is complete.  The
    built-in/up datatype IDS is NOT considered to be a child of whatever was
    the current object at the time the TYPE statement started to be parsed
    (and consequently no "<contains>" relationship is entered into the
    parent's relationship table as it is during a call to
    "mirc_create_object()").

    With V1.95, this function requests the creation of a special Non-Terminal
    IDS (flavor = I_NT_OBJECT_DC) for use in describing DataConstructs THAT
    ARE BUILTIN TO THE SMI (ie created by the parse of the builtin_types file).

    This allows these IDS MIR Objects to be maintained on a separate list by
    the backend.  They are written to the MIR binary output file in their own
    Non-Terminal-for-DataConstruct Partition (by convention) to allow the
    functions in module "mir_internal.c" to be able to discover these special
    Non-Terminals when the binary file is being 'read back in'.  (So they
    can be re-entered on the compiler's internal lists of builtin-to-SMI types)

    Note also that we establish the following conventions:

        * The first relationship in the Relationship Table of these
          Non-Terminals is for "MIR_DC_Found_In_SMI"
        * The third relationship in the Relationship Table of these
          Non-Terminals is for "MIR_DC_SMI_Name"

    This is so that the functions in "mir_internal.c" can easily discover the
    boundaries between dataconstructs of different SMIs and get the names of
    dataconstructs.

    With V1.99 and support for Merge and Augment, extra logic is added to
    prevent the compiler from creating a "Built-Up" Dataconstruct on the
    2nd and subsequent iterations that the parser may make on an Augment
    file.  We do this because the parser is going to be invoked once for
    every entity in the Augment Entity list, and that means the TYPE statements
    are going to be scanned over again.  Only on the first pass do we want
    to actually create MIR objects that correspond to the TYPE statements,
    thereafter we just want to 'pretend'.

*/

{
IDS         *new;       /* --> Newly created MIR Non-Terminal object        */
IDS         *smi_code;  /* --> Newly created MIR Terminal object for smicode*/
IDS         *code;      /* --> Newly created MIR Terminal object for code   */
IDS         *name;      /* --> Newly created MIR Terminal object for name   */
IDX         *index;     /* --> Index structure for this new builtup type    */
MC_STATUS   status;     /* Status of support calls      */

/*
| AUGMENT Support
|
| The only kind of datatype we expect to deal with under Augment is a request
| to create a *built-UP* dataconstruct.
|
| if (compiler mode is AUGMENT) 
*/
if (compiler_mode == CM_AUGMENT) {

    /* if (datatype is already defined) */
    if (mirc_find_datatype(dc_name, &new) == MC_SUCCESS) {

        /* mark the IDS for the dataconstruct as PREDEFINED_TYPE */
        new->idsu.nt.ntu.ds = DS_PREDEFINED_TYPE;

        /* return it successfully:
        | make current object be recorded in new object as its parent
        */
        new->idsu.nt.parent_obj = bec->current_object;

        /* make this the current mir object */
        bec->current_object = new;
        return (MC_SUCCESS);
        }
    }

/* if (create of non-terminal for a DATACONSTRUCT failed) */
if ( (new = I_Create_IDS(
                         ((built_in == TRUE)
                            ? I_NT_OBJECT_DC     /* Built-In to SMI         */
                            : I_NT_OBJECT),      /* Built-Up from TYPE stmt */
                         IDS_DFT_TABLESIZE       /* Default table size      */
      )) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* mark the IDS for the dataconstruct as CREATED */
new->idsu.nt.ntu.ds = DS_CREATED;

/* make current object be recorded in new object as its parent */
new->idsu.nt.parent_obj = bec->current_object;

/* make this the current mir object */
bec->current_object = new;

/*
| NOTE: By Convention, "MIR_DC_Found_In_SMI" must be the first relationship
|       placed into these BUILTI-IN DataConstruct Non-Terminals.
*/
/* if (it is a "builtin" dataconstruct) */
if (built_in == TRUE) {

    /* if (create for terminal for smi code failed) */
    if ((smi_code = I_Create_IDS_SNUMBER(bec->current_smi)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }

    /* if (add of relationship for smi definition failed)
    |
    | NOTE: We're meeting the convention here with this "MIR_DC_Found_In_SMI"
    |       insert:  it is FIRST in the Relationship Table for the builtin
    |       dataconstruct.
    */
    if ( (status = mirc_add_relationship(MIR_DC_Found_In_SMI, smi_code))
             != MC_SUCCESS) {
        return (status);
        }
    }

/* if (create of terminal for code failed) */
if ( (code = I_Create_IDS_SNUMBER(dc_code)) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* if (it is a "builtin" dataconstruct) */
if (built_in == TRUE) {

    /* if (add of relationship for code failed) */
    if ( (status = mirc_add_relationship(MIR_DC_SMI_Code, code)) != MC_SUCCESS) {
        return (status);
        }
    }
else {
    /* if (add of relationship for code failed) */
    if ( (status = mirc_add_relationship(MIR_ID_Code, code)) != MC_SUCCESS) {
        return (status);
        }
    }

/* if (create of terminal for name failed) */
if ( (name = I_Create_IDS_STRING(dc_name)) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* if (it is a "builtin" dataconstruct) */
if (built_in == TRUE) {

    /* if (add of relationship for name failed)
    |
    | NOTE: We're meeting the convention here with this "MIR_DC_SMI_Name"
    |       insert:  it is THIRD in the Relationship Table for the builtin
    |       dataconstruct.
    */
    if ( (status = mirc_add_relationship(MIR_DC_SMI_Name, name)) != MC_SUCCESS) {
        return (status);
        }
    }
else {
    /* if (add of relationship for name failed) */
    if ( (status = mirc_add_relationship(MIR_Text_Name, name)) != MC_SUCCESS) {
        return (status);
        }
    }

/* if (create of index data structure storage failed) */
if ( (index = (IDX *) malloc(sizeof(IDX))) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }
index->nt = new;    /* Load up pointer to the IDS into index block */

/* if (allocation of storage to hold new datatype name failed) */
if ( (index->name = (char *) malloc(strlen(dc_name) + 1)) == NULL ) {
    return (MC_OUT_OF_MEMORY);
    }

/* copy new datatype name and length to allocated storage */
strcpy (index->name, dc_name);
index->len = strlen(dc_name);

/* add the index data structure into the top of the "built-up" datatype list */
if (built_in == TRUE) {  /* Built-IN */
    index->next = bec->built_in[bec->current_smi];
    bec->built_in[bec->current_smi] = index;
    }
else {  /* Built-UP */
    index->next = bec->built_up[bec->current_smi];
    bec->built_up[bec->current_smi] = index;
    }

return (MC_SUCCESS);
}

/* mirc_add_relationship - Add relationship/target to current object */
/* mirc_add_relationship - Add relationship/target to current object */
/* mirc_add_relationship - Add relationship/target to current object */

MC_STATUS
mirc_add_relationship (relationship, target)

mir_relationship    relationship;  /* Code for  Relationship Non-Term to add */
IDS                 *target;       /* -> to Target to add                    */

/*
INPUTS:

    "relationship" is a code to be used in the mapping array to obtain the IDS
    representing the relationship to be added to the "current" MIR object.

    "target" is a pointer to the MIR object that is to be the target
    of the specified relationship in the current MIR object's relationship
    table.

    The symbol "compiler_mode" defined in "mir_yacc.y" is an implicit input
    to this function.


OUTPUTS:

    The function returns one of:

    "MC_OUT_OF_MEMORY" - Ran out of memory during the attempt to add the new
                relationships into the relationship table.

    "MC_SUCCESS" - Insertion was successful.  On a successful return, the
            "current" object has a new relationship entry in it's relationship
            table.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it recognizes another relationship to
        be added to the "current" MIR object's relationship table.

    Purpose:
        This function takes care of the details of adding another entry
        into the relationship table of the current MIR object.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (compiler-mode is not NORMAL)
        if (current object is not CREATED
            if (    mode is Augment
                and current object is a General Non-Terminal
                and object is PREDEFINED)
                <issue error:
                    "This statement not allowed in AUGMENT ENTITY file">
            <return SUCCESS>
            
    <obtain the IDS for the specified relationship>
    
    <return status from intermediate insert-relationship call>


OTHER THINGS TO KNOW:

    The 'add logic' of this function is duplicated in function
    "mirc_create_object()" (rather than calling this function) in order to
    avoid the error checking logic added here to support Merge & Augment.
    See OTHER THINGS TO KNOW for that function.

    We're looking to preclude what the parser front-end thinks is going to
    be a write of a relationship+target into the current object IF:

       - Current Object is a General Non-Terminal
         AND
       - Compiler Mode is Merge or Augment
         AND
       - the current object has definition-status of PREDEFINED
         (or PREDEFINED_TYPE)

    In these circumstances, we want compilation to continue, but with no
    change to the MIR being built, or (under certain circumstances detailed
    below) there should be an error issued.  Here's why:

       If we're in Merge or Augment mode, that means a General Non-Terminal
       could have been just-CREATED (as a consequence of being unique and
       being mentioned in the merge or augment file) or it could be
       PREDEFINED (meaning that it was already in existence before the
       current merge or augment file began to be compiled).

       - If it was CREATED, that means we're merging/augmenting it into the
         MIR, and the 'write' of the relationship+target by this function
         should be done.

       - If it was PREDEFINED, that means the front-end is trying to add
         something from the merge/augment file into an object that has
         already been defined, (and fully 'written-into' earlier),
         so we don't want to duplicate the write.

         * On a merge, this allows us to do a 'merge':
           writes to objects that already exist don't 'happen', but we don't
           complain either.

         * On an augment, this is an error, as the user has specified
           something additional in an Augment file that doesn't exist in
           the original object; we complain.

       - If it was PREDEFINED_TYPE, that means the front-end is trying to add
         a relationship to an object that represents a TYPE statement that
         has already been parsed and defined for this file.  This will occur
         on the second and subsequent passes that the compiler makes over an
         AUGMENT ENTITY file that specifies more than one Entity-Class to be
         augmented.  In this case, we don't want to issue an error message,
         and we don't want to do the 'write' into the object's relationship
         table.
*/

{
/* if (compiler-mode is not NORMAL) */
if (compiler_mode != CM_NORMAL) {

    /* if (current object is not CREATED) */
    if (bec->current_object->idsu.nt.ntu.ds != DS_CREATED) {

        /* if (mode is Augment && current obj is a PREDEFINED General NT) */
        if (compiler_mode == CM_AUGMENT
            && bec->current_object->flavor == I_NT_OBJECT
            && bec->current_object->idsu.nt.ntu.ds == DS_PREDEFINED) {

            /*issue error: "This statement not allowed in AUGMENT ENTITY" */
            yyerror(
                    MP(mp151,
                       "Modification of Object not created by AUGMENT file is not allowed")
                   );
            }

        return(MC_SUCCESS);
        }
    }

/* obtain the IDS for the specified relationship */
/* return status from intermediate insert-relationship call */
return (I_Insert_Rel_Entry(bec->current_object,
                           bec->map_rel_to_ids[(int)relationship],
                           target));
}

/* mirc_add_rel_string - Add relationship/string-target to current object */
/* mirc_add_rel_string - Add relationship/string-target to current object */
/* mirc_add_rel_string - Add relationship/string-target to current object */

MC_STATUS
mirc_add_rel_string (relationship, target)

mir_relationship    relationship;  /* Code for  Relationship Non-Term to add */
char                *target;       /* -> string to convert to Target to add  */

/*
INPUTS:

    "relationship" is a code to be used in the mapping array to obtain the IDS
    representing the relationship to be added to the "current" MIR object.

    "target" is a pointer to a string to be converted into the MIR object
    that is to be the target of the specified relationship in the current MIR
    object's relationship table.

    The symbol "compiler_mode" defined in "mir_yacc.y" is an implicit input
    to this function.


OUTPUTS:

    The function returns one of:

    "MC_OUT_OF_MEMORY" - Ran out of memory during the attempt to add the new
                relationships into the relationship table.

    "MC_SUCCESS" - Insertion was successful.  On a successful return, the
            "current" object has a new relationship entry in it's relationship
            table.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it recognizes another relationship to
        be added to the "current" MIR object's relationship table.

    Purpose:
        This function takes care of the details of adding another entry
        into the relationship table of the current MIR object (and in
        this instance the conversion of the string into an IDS).


ACTION SYNOPSIS OR PSEUDOCODE:

    if (compiler-mode is not NORMAL)
        if (current object is PREDEFINED)
            if (mode is Augment and current object is a General Non-Terminal)
                <issue error:
                    "This statement not allowed in AUGMENT ENTITY file">
            <return SUCCESS>
            
    if (conversion of target string to IDS failed)
        <return MC_OUT_OF_MEMORY>    
    <return status from mirc_add_relationship>


OTHER THINGS TO KNOW:

    For a discussion of the 'error-reject' logic (first "if" statement in
    the code below) that short-circuits writes under certain circumstances, 
    see the OTHER THINGS TO KNOW section for "mirc_add_relationship()".
*/

{
IDS     *string_IDS;    /* --> IDS containing string supplied as argument */

/* if (compiler-mode is not NORMAL) */
if (compiler_mode != CM_NORMAL) {

    /* if (current object is not CREATED) */
    if (bec->current_object->idsu.nt.ntu.ds != DS_CREATED) {

        /* if (mode is Augment && current obj is a PREDEFINED General NT) */
        if (compiler_mode == CM_AUGMENT
            && bec->current_object->flavor == I_NT_OBJECT
            && bec->current_object->idsu.nt.ntu.ds == DS_PREDEFINED) {

            /*issue error: "This statement not allowed in AUGMENT ENTITY" */
            yyerror(
                    MP(mp151,
                       "Modification of Object not created by AUGMENT file is not allowed")
                   );
            }

        return(MC_SUCCESS);
        }
    }

/* if (conversion of target string to IDS failed) */
if ( (string_IDS = I_Create_IDS_STRING(target)) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* return status from add - relationship call */
return (mirc_add_relationship(relationship,string_IDS));
}

/* mirc_add_rel_number - Add relationship/number-target to current object */
/* mirc_add_rel_number - Add relationship/number-target to current object */
/* mirc_add_rel_number - Add relationship/number-target to current object */

MC_STATUS
mirc_add_rel_number (relationship, target, target_type)

mir_relationship    relationship;  /* Code for  Relationship Non-Term to add */
int                 target;        /* Number to convert to Target to add     */
mir_value_type      target_type;   /* Indicates "Signed" or "Unsigned"       */

/*
INPUTS:

    "relationship" is a code to be used in the mapping array to obtain the IDS
    representing the relationship to be added to the "current" MIR object.

    "target" is an integer to be converted into the MIR object
    that is to be the target of the specified relationship in the current MIR
    object's relationship table.  (While this argument is typed as "signed",
    the value may actually be interpreted as "unsigned".

    "target_type" indicates how to interpret "target" as to it's "signedness".

    The symbol "compiler_mode" defined in "mir_yacc.y" is an implicit input
    to this function.


OUTPUTS:

    The function returns one of:

    "MC_OUT_OF_MEMORY" - Ran out of memory during the attempt to add the new
                relationships into the relationship table.

    "MC_SUCCESS" - Insertion was successful.  On a successful return, the
            "current" object has a new relationship entry in it's relationship
            table.

    "MC_FAILURE" - "target_type" was not "MIR_UNUMBER" or "MIR_SNUMBER".


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it recognizes another relationship to
        be added to the "current" MIR object's relationship table.

    Purpose:
        This function takes care of the details of adding another entry
        into the relationship table of the current MIR object (and in
        this instance, the conversion of the number (signed or unsigned)
        into an IDS).


ACTION SYNOPSIS OR PSEUDOCODE:

    if (compiler-mode is not NORMAL)
        if (current object is PREDEFINED)
            if (mode is Augment and current object is a General Non-Terminal)
                <issue error:
                    "This statement not allowed in AUGMENT ENTITY file">
            <return SUCCESS>
            
    if (target type is SIGNED)
        if (conversion of target signed number to IDS failed)
            <return MC_OUT_OF_MEMORY>    
    else if (target type is UNSIGNED)
        if (conversion of target unsigned number to IDS failed)
            <return MC_OUT_OF_MEMORY>    
    else <return MC_FAILURE>

    <return status from mirc_add_relationship>


OTHER THINGS TO KNOW:

    For a discussion of the 'error-reject' logic (first "if" statement in the
    code below) that short-circuits writes under certain circumstances, see
    the OTHER THINGS TO KNOW section for "mirc_add_relationship()".
*/

{
IDS     *number_IDS;    /* --> IDS containing string supplied as argument */

/* if (compiler-mode is not NORMAL) */
if (compiler_mode != CM_NORMAL) {

    /* if (current object is not CREATED) */
    if (bec->current_object->idsu.nt.ntu.ds != DS_CREATED) {

        /* if (mode is Augment && current obj is a PREDEFINED General NT) */
        if (compiler_mode == CM_AUGMENT
            && bec->current_object->flavor == I_NT_OBJECT
            && bec->current_object->idsu.nt.ntu.ds == DS_PREDEFINED) {

            /*issue error: "This statement not allowed in AUGMENT ENTITY" */
            yyerror(
                    MP(mp151,
                       "Modification of Object not created by AUGMENT file is not allowed")
                   );
            }

        return(MC_SUCCESS);
        }
    }

/* if (target type is SIGNED) */
if (target_type == MIR_SNUMBER) {
    /* if (conversion of target number to IDS failed) */
    if ( (number_IDS = I_Create_IDS_SNUMBER(target)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }
    }
else if (target_type == MIR_UNUMBER) {
    /* if (conversion of target number to IDS failed) */
    ;
    if ( (number_IDS = I_Create_IDS_UNUMBER((unsigned int) target)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }
    }
else return (MC_FAILURE);

/* return status from add - relationship call */
return (mirc_add_relationship(relationship, number_IDS));
}

/* mirc_find_datatype - Search compiler internal list for datatype IDS */
/* mirc_find_datatype - Search compiler internal list for datatype IDS */
/* mirc_find_datatype - Search compiler internal list for datatype IDS */

MC_STATUS
mirc_find_datatype (datatype, datatype_ids)

char    *datatype;      /* --> string that identifies a builtin/up datatype */
IDS     **datatype_ids; /* Address of a pointer to be set to the IDS of the */
                        /* "datatype"                                       */

/*
INPUTS:

    "datatype" is the string extracted from the MSL file for a datatype that
    the compiler must recognize.

    "datatype_ids" is the address of a pointer (that must be set) to an IDS
    that stands for the datatype whose name arrives in "datatype".  This
    is the function that does the "recognizing" for the compiler.


OUTPUTS:

    The function returns one of:

    "MC_DATATYPE_NOT_FND" - Couldn't find the datatype name string on the
            compiler's internal datatype lists.

    "MC_SUCCESS" - Search was successful.  On a successful return, the 
            "datatype_ids" pointer is set to point at the IDS for the
            datatype.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it has parsed what it thinks is a
        datatype name in a context where it should have already (either)
        seen the TYPE statement that defines the datatype or the datatype
        is a built-in (to the SMI) datatype.

        Also called by "mirc_create_dataconstruct()" during AUGMENT operation
        to see if a dataconstruct has already been defined.

    Purpose:
        This function searches the compiler's internal lists to determine
        the IDS that describes that datatype.

ACTION SYNOPSIS OR PSEUDOCODE:

    <scan the built-in datatype list for selected SMI for a match by name>
    if (a match was found)
        <return a pointer to the caller>
        <return MC_SUCCESS>

    <scan the built-up datatype list for selected SMI for a match by name>
    if (a match was found)
        <return a pointer to the caller>
        <return MC_SUCCESS>
    <return MC_DATATYPE_NOT_FND>

OTHER THINGS TO KNOW:

    About 48 baseballs are used in the average major league baseball game, 
*/

{
IDX     *scanner;       /* Used to scan either built-in or built-up list */
int     length;         /* Computed lenth of inbound datatype name       */

length = strlen(datatype);  /* Obtain length of name locally */

/* scan the built-in datatype list for selected SMI for a match by name */
for (scanner = bec->built_in[bec->current_smi];
     scanner != NULL;
     scanner=scanner->next) {

    if (length == scanner->len) {
        if (caseless_str_equal(scanner->name, datatype, length) == TRUE)
            break;
        }
    }

/* if (a match was found) */
if (scanner != NULL) {
    /* return a pointer to the caller */
    *datatype_ids = scanner->nt;

    return (MC_SUCCESS);
    }

/* scan the built-up datatype list for selected SMI for a match by name */
for (scanner = bec->built_up[bec->current_smi];
     scanner != NULL;
     scanner=scanner->next) {

    if (length == scanner->len) {
        if (caseless_str_equal(scanner->name, datatype, length) == TRUE)
            break;
        }
    }

/* if (a match was found) */
if (scanner != NULL) {

    /* return a pointer to the caller */
    *datatype_ids = scanner->nt;

    return (MC_SUCCESS);
    }

return (MC_DATATYPE_NOT_FND);
}

/* mirc_find_rel_IDS - return a pointer to a compiler relationship IDS */
/* mirc_find_rel_IDS - return a pointer to a compiler relationship IDS */
/* mirc_find_rel_IDS - return a pointer to a compiler relationship IDS */

IDS *
mirc_find_rel_IDS (rel_identifier)

char        *rel_identifier; /* Which relationship we want (as a string) */

/*
INPUTS:

    "rel_identifier" specifies the compiler relationship whose IDS
    is desired.

OUTPUTS:

    The function returns one of:

    NULL - No IDS has been registered by mirc_init_backend() for the
           specified compiler relationship (coding error)

    a pointer to the IDS for the specified compiler relationship.

BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing the builtin_types.dat file where it has reached a point
        where it needs an IDS for a particular compiler relationship.

        See the OTHER THINGS TO KNOW about the restrictions on when this
        function may be called.

    Purpose:
        This function searches the compiler's internal lists to determine
        the IDS that describes that relationship and returns a pointer
        to it.

ACTION SYNOPSIS OR PSEUDOCODE:

    <scan the relationship-name list for selected SMI for a match by name>
    if (a match was found)
        <return a pointer from relationship-IDS list to the caller>
    else
        <return NULL>

OTHER THINGS TO KNOW:

    NOTE:  This function is going to return NULL *ALWAYS* during a compilation
           if a binary MIR database load was performed.  That's because
           the relationships are not 'registered' in the name-string array
           that this function searches if there was a binary MIR load (because
           the builtin-types file isn't parsed as far as the point where this
           function needs to be called in that instance).
*/

{
int     i;      /* General Purpose Index                        */
IDS     *rel;   /* Pointer to the desired relationship IDS      */

/* scan the relationship-name list for selected SMI for a match by name */
/* We *must* find it on the internal name list */
for (i=0; i < REL_COUNT; i++) {
    if (strcmp(bec->map_relstr_to_ids[i], rel_identifier) == 0)
        break;
    }

/* if (a match was found) */
if (i < REL_COUNT) {
    /* Grab the pointer */
    rel = bec->map_rel_to_ids[i];

    /* return a pointer from relationship-IDS list to the caller */
    return (rel);
    }
else {
    return (NULL);
    }

}

/* mirc_create_object - Create MIR Object (Non-Datatype) */
/* mirc_create_object - Create MIR Object (Non-Datatype) */
/* mirc_create_object - Create MIR Object (Non-Datatype) */

/* The following are Implied arguments, defined in mir_yacc.y */
extern int eflag;       
extern IDS *current_attr_partition;
extern IDS *current_evt_partition;


MC_STATUS
mirc_create_object (parents_relationship, code, textname, level)

mir_relationship    parents_relationship;   /* The relationship the parent  */
                                            /* has to this object           */

int                 code;       /* The SMI-defined code by which this object*/
                                /* is known.                                */

char                *textname;  /* The SMI-defined name by which this object*/
                                /* is known.                                */

int                 *level;     /* The level (a la DNU dictionary) into the */
                                /* MSL where "code" is defined.             */

/*
INPUTS:

    "parents_relationship" is the relationship that should appear in the
    relationship table of the object that is the "parent" of this newly
    created object.  The newly created object is the target of the relationship
    placed into the parent's relationship table.

    "code" is the integer code parsed from the MSL file that is associated
    with this newly created object.  Note: The parser uses a value of "-1"
    if the MSL write omitted the MCC code value passed in this argument.

    "textname" the string parsed from the MSL file that is associated with the
    newly created object.

    "level" defines "how deep" we are into the MSL.  This is used to keep
    track of the "codes" that go in to making up the Object ID for the
    current object, it is incremented by one on success.

    An Implicit input to this function is "the current object" (as described
    by the back-end context), as well as the IDS pointers listed above
    and defined in "mir_yacc.y".


OUTPUTS:

    The function returns one of:

    "MC_OUT_OF_MEMORY" - During an attempt to create the new object or
            expand the relationship table of the parent object we ran
            out of memory.

    "MC_SUCCESS" - Creation was successful.  On a successful return,
            * The parent object's relationship table is expanded to hold a
              relationship (supplied as input) whose target is the
              newly created object.
            * The newly created object's relationship table is filled in with
              relationships:
                "MIR_Contained_By" using as a target the new object's parent
                "MIR_ID_Code" using as a target the value of "code"
                "MIR_Text_Name" using as a target the value of "textname"
            * The object is registered in the MIR index.

            Upon return, the newly created object becomes the default
            "current object", and "level" has been incremented by one.

BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser.  It has just reached a stage in
        parsing an MSL file where it has parsed what it thinks is an object
        defined by the SMI that needs to be recorded in the MIR.

    Purpose:
        This function handles all of the details of "registering" the
        the newly created object in the MIR in Intermediate form.


ACTION SYNOPSIS OR PSEUDOCODE:

    switch (Parent Relationship)
        case MIR_Cont_entityClass:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_entityClass>
            <break>

        case MIR_Cont_attribute:
            <set thing to search to 'current attribute partition'>
            <set thing to search-for to MIR_List_Entry>
            <break>

        case MIR_Cont_attrPartition:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_attrPartition>
            <break>

        case MIR_Cont_attrGroup:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_attrGroup>
            <break>

        case MIR_Cont_eventPartition:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_eventPartition>
            <break>

        case MIR_Cont_eventGroup:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_eventGroup>
            <break>

        case MIR_Cont_event:
            <set thing to search to 'current event partition object'>
            <set thing to search-for to MIR_List_Entry>
            <break>

        case MIR_Cont_directive:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_directive>
            <break>

        case MIR_Cont_exception:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_exception>
            <break>

        case MIR_Cont_response:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_response>
            <break>

        case MIR_Cont_argument:
            <set thing to search to 'current object'>
            <set thing to search-for to MIR_Cont_argument>
            <break>

    <show No Duplicate Found>

    (* Search for an already-existing object by this name *)
    for (every entry in the thing to search)
        if (relationship is the thing to search-for)
            <select that entry's target to be searched for name-match>
            for (every object in the target's relationship table)
                if (next entry's relationship is "MIR_Text_Name")
                    if (target of that entry name is <new textname>)
                        if (compiler-mode is "Normal")
                            <issue "Duplicate Name at this level: "%s">
                            <set eflag TRUE>
                        <record selected-object as duplicate>
                        <record pointer to selected-object's name string>
                    <break>

    if (object is anything other than an Entity Class)
        <bump the level indicator>

    if (compiler mode is NOT Normal and Duplicate was Found)
        <Show Object as PREDEFINED>
        <set IDS parentpointer (of already-created object) --> current object>
        <make newly created object be the "current" object>

        if (parsed MCC code doesn't match stored MCC code)
            <issue "Augment Object Code specified (%d) doesn't match predefined code (%d)"
        <record the new code & relationship at the current level in stacks>
        <obtain any DNA_CMIP_CODE>
        <record the DNA code at the current level in the stacks>

        <return this object as though it were created>

    if (attempt to create a non-terminal object failed)
        <return status>
    <show an object created>
    <show object definition status as CREATED>

    if (compiler mode is MERGE)
        <issue info "Merging Object "<name>" = %d (code)">

    if (attempt to add specified relationship to current object failed)
        <return status>

    <set IDS parentpointer (of newly created object) to pt at current object>
    <make newly created object be the "current" object>

    if (attempt to add "MIR_Contained_By" rel. to current object failed)
        <return status>

    if (code value is legitimate (not "-1") )
        if (create for terminal for code failed)
            <return status>

        if (attempt to add "code" relationship to current object failed)
            <return status>

    if (create for terminal for name failed)
        <return status>

    if (attempt to add "text name" relationship to current object failed)
        <return status>

    <extract a pointer to a static string containing the name of the object>
    <record the new code & relationship at the current level in our stacks>

>>>> Temporarily disableable
    if (current smi is DNA)
       <compute the MCC ISO object ID under which this current object should be
        registered>

        if (attempt to register the current object using computed OID failed)
            <return status>
<<<< Temporarily disableable

    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    There is a bit of trickiness surrounding the front-end variable "level"
    that is passed as an argument to this function.

    The front-end parser sets this value initially (when keyword "GLOBAL" or
    "PARENT=" is parsed ) and then increments it when keyword "CHILD" is
    parsed.  However, for all other items contained within Entity Class,
    the incrementing is done by this function.

    The bottom line is that "level" is 'already' correct if the MIR Object
    being created is an Entity Class, but it needs to be incremented if
    the MIR Object is anything other than an entity class.
                                 - - - -
    Automatic computation of, and assignment of an MCC OID to the created
    MIR object only occurs if the SMI being compiled under is "MIR_SMI_DNA",
    maybe we don't want this to be so.
                                 - - - -
    With V1.99 and the advent of support for compiler-modes "Merge" and
    "Augment", this function performs a tricky bit of action to support these
    modes, to wit:

      - The function must mark each object it processes according to whether
        it 'creates' it or simply returns a pointer to an already-defined
        object.  On "create", the object's definition-status is set to
        "DS_CREATED", if already defined, it's status is set to "DS_PREDEFINED"
        (Under compiler-mode "Normal", all objects are 'created').

      - The function returns predefined objects by trying to lookup the object
        by its given name.  If it exists, it is returned in a fashion that
        simulates it having been just created.  This allows the compiler
        parser to process a Merge or Augment file using the same 'moves' (ie
        calls to backend functions) it would use on a normal MSL file.
        If an object does not exist under Merge or Augment, it is created in
        exactly the same fashion as it is under compiler mode Normal.

      - We do an explicit call to "I_Insert_Rel_Entry()" to insert the
        relationship into the parent object that points at a new object
        rather than using "mirc_add_relationship()" because that function
        has new logic to support Merge & Augment that precludes it from
        inserting the relationship into the parent if the parent was
        predefined.

    See the discussion in OTHER THINGS TO KNOW for function
    "mirc_get_def_status()" (in this module) for the full skinny on the
    importance of the "definition-status" field and how it controls
    compilation under Merge and Augment compiler modes.
*/

#define MCC_OIDS
{
IDS         *new;          /* --> Newly created MIR Non-Terminal object      */
IDS         *code_IDS;     /* --> Newly created MIR Terminal object for code */
IDS         *name_IDS;     /* --> Newly created MIR Terminal object for name */
MC_STATUS   status;        /* Status of support calls                        */
int         i,j;           /* Handy-Dandy General Purpose Index              */
IDS         *cand_lvl_0;   /* --> Possible match IDS structure               */
IDS         *cand_lvl_1;   /* --> Possible match on name                     */
int         cand_name_len; /* Length of candidate name                       */
int         name_len;      /* Length of giver name                           */
char        msg[200];      /* Duplicate-Name error msg built here            */
mir_relationship search_for;  /* Thing to be searched-for in "t.to_search"   */
IDS         *thing_to_search; /* Thing to be scanned for matches             */
IDS         *dup_object;   /* Found Duplicate MIR object                     */
char        *dup_obj_name; /* --> Text Name for Duplicate Object             */
int         mcc_code;      /* Predefined MCC code                            */

#ifdef MCC_OIDS
object_id   *oid;          /* Ptr to an the computed object id               */
char        *oid_err;      /* Ptr to Error Message from mirc_compute_oid()   */
#endif

/*
| Here we're setting up to do a proper search to see whether the thing we're
| about to create already exists.  The search is for something with the same
| "name" (target of "MIR_Text_Name").
*/
switch (parents_relationship) {
    case MIR_Cont_entityClass:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_entityClass */
        search_for = MIR_Cont_entityClass;
        break;

    case MIR_Cont_attribute:
        /* set thing to search to 'current attribute partition' */
        thing_to_search = current_attr_partition;

        /* set thing to search-for to MIR_List_Entry */
        search_for = MIR_List_Entry;
        break;

    case MIR_Cont_attrPartition:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_attrPartition */
        search_for = MIR_Cont_attrPartition;
        break;

    case MIR_Cont_attrGroup:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_attrGroup */
        search_for = MIR_Cont_attrGroup;
        break;

    case MIR_Cont_eventPartition:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_eventPartition */
        search_for = MIR_Cont_eventPartition;
        break;

    case MIR_Cont_eventGroup:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_eventGroup */
        search_for = MIR_Cont_eventGroup;
        break;

    case MIR_Cont_event:
        /* set thing to search to 'current event partition object' */
        thing_to_search = current_evt_partition;

        /* set thing to search-for to MIR_List_Entry */
        search_for = MIR_List_Entry;
        break;

    case MIR_Cont_directive:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_directive */
        search_for = MIR_Cont_directive;
        break;

    case MIR_Cont_exception:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_exception */
        search_for = MIR_Cont_exception;
        break;

    case MIR_Cont_response:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_response */
        search_for = MIR_Cont_response;
        break;

    case MIR_Cont_argument:
        /* set thing to search to 'current object' */
        thing_to_search = bec->current_object;

        /* set thing to search-for to MIR_Cont_argument */
        search_for = MIR_Cont_argument;
        break;

    default:
        fprintf(stderr,
                "MIRC - Internal error, Relationship code = %d\n",
                parents_relationship);
        exit(BAD_EXIT);

    }

name_len = strlen(textname);

dup_object = NULL;      /* show No Duplicate Found */

/*
| Search for an already-existing object by this name inside the MIR Object
| selected above.
|
| for (every entry in the selected-object's relationship table)
*/
for (i=0;
     i < thing_to_search->idsu.nt.entry_count && dup_object == NULL;
     i++) {

    /* if (relationship is the thing to search-for) */
    if (thing_to_search->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym
         == search_for ) {

        /* select that entry's target to be searched for name-match */
        cand_lvl_0 = thing_to_search->idsu.nt.rel_table[i].tar_obj;

        /* for (every object in the target's relationship table) */
        for (j=0; j < cand_lvl_0->idsu.nt.entry_count; j++) {

            /* if (next entry's relationship is "MIR_Text_Name") */
            if (cand_lvl_0->idsu.nt.rel_table[j].rel_obj == 
                                         bec->map_rel_to_ids[MIR_Text_Name]){

                cand_lvl_1 = cand_lvl_0->idsu.nt.rel_table[j].tar_obj;
                cand_name_len = cand_lvl_1->idsu.t.t_v.string.len;

                /* if (target of that entry name is "name") */
                if (cand_name_len == name_len &&
                            caseless_str_equal(
                                cand_lvl_1->idsu.t.t_v.string.str,
                                textname,
                                name_len)) {

                    /* if (compiler-mode is "Normal") */
                    if (compiler_mode == CM_NORMAL) {
                        /* issue "Duplicate Name "%s" Error message " */
                        sprintf(msg,
                                MP(mp100,"\"%s\" is used more than once at this level (duplicate name)"),
                                textname);
                        yyerror(msg);
                        }

                    /* record selected-object as duplicate
                    |
                    | (Here we're recording things for Compiler Modes
                    |  "Merge" and "Augment".
                    */
                    dup_object = cand_lvl_0;

                    /* record pointer to selected-object's name string */
                    dup_obj_name = cand_lvl_1->idsu.t.t_v.string.str;
                    }

                break;  /* Blow out of inner loop, get new candidate 0 */
                }
            }
        }
    }

/* if (object is anything other than an Entity Class) */
if (parents_relationship != MIR_Cont_entityClass) {
    /* bump the level indicator */
    *level = *level + 1;
    }

    /* <><> fprintf(stderr, "creating object at level %d.\n",*level); */
    /* <><> fprintf(stderr, "----level now: %d.\n",*level); */

/* if (compiler mode is NOT Normal and Duplicate was Found) */
if (compiler_mode != CM_NORMAL && dup_object != NULL) {

    /* Show Object as PREDEFINED */
    dup_object->idsu.nt.ntu.ds = DS_PREDEFINED;

    /* set IDS parentpointer (of already-created object) --> current object */
    dup_object->idsu.nt.parent_obj = bec->current_object;

    /* make newly created object be the "current" object */
    bec->current_object = dup_object;

    /* if (parsed MCC code doesn't match stored MCC code) */
    if ((mcc_code = mirc_find_obj_CODE(dup_object, MCC_code)) != code) {
        char        buff[200];      /* Error message buffer */
        /*
        |  issue "Augment Object Code specified (%d) doesn't match
        |         predefined code (%d)"
        */
        sprintf(buff,
                MP(mp152, "Augment Object Code specified (%d) doesn't match predefined code (%d)"),
                code, mcc_code);
        yyerror(buff);
        }

    /*
    | Record the new code & relationship at the current level in stacks so
    | that MCC OIDs can be computed for augmented things 'below' this object
    */
    mirc_record_idcodes(*level, MCC_code, code, &(parents_relationship),
                        dup_obj_name);

    /* 
    |  Obtain & Record any DNA_CMIP_INT code at the current level in stacks so
    |  that DNA OIDs can be computed for augmented things 'below' this object
    |
    |  obtain any DNA_CMIP_CODE
    */
    mirc_record_idcodes(*level,                 /* Level in hierarchy     */
                        STD_code,               /* Code Type              */
                        mirc_find_obj_CODE(dup_object, STD_code), /* Code */
                        NULL,                   /* Parent's Relationship  */
                        NULL);                  /* Name                   */

    /* return this object as though it were created */
    return(MC_SUCCESS);
    }

/* if (attempt to create a non-terminal object (w/default tablesize) failed) */
if ( (new = I_Create_IDS(I_NT_OBJECT, IDS_DFT_TABLESIZE)) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* show object created */
bec->object_created = TRUE;

/* show object definition status as CREATED */
new->idsu.nt.ntu.ds = DS_CREATED;

/* if (compiler mode is MERGE) */
if (compiler_mode == CM_MERGE) {
    /* issue info "Merging Object "<name>" = %d (code)" */
    fprintf(stderr,
            MP(mp153,"mirc - Info: Merging Object %s = %d\n"),
            textname, code);
    }


/*
|  All the logic south of here in this function is insensitive to the
|  "compiler-mode" (normal/merge/augment).
*/

/*
| NOTE:
|      We do an explicit insert here (without calling "mirc_add_relationship()"
|      because that function has extra logic that would preclude our doing
|      an add of the parent's relationship for Augment & Merge in the case
|      where the parent turned out to be PREDEFINED.
|
| if (attempt to add specified relationship to current object failed)
*/
if ((status = I_Insert_Rel_Entry(bec->current_object,
                           bec->map_rel_to_ids[parents_relationship],
                           new)) != MC_SUCCESS) {
    return(status);
    }

/* set IDS parentpointer (of newly created object) to point at current obj */
new->idsu.nt.parent_obj = bec->current_object;

/* make newly created object be the "current" object */
bec->current_object = new;

/* if (attempt to add specified relationship to current object failed) */
if ((status = mirc_add_relationship (MIR_Contained_By,
                                     new->idsu.nt.parent_obj))
        != MC_SUCCESS){
    return (status);
    }

/* if (code value is legitimate (not "-1") */
if (code != -1 ) {

    /* if (create for terminal for code failed) */
    if ( (code_IDS = I_Create_IDS_SNUMBER(code)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }

    /* if (attempt to add "code" relationship to current object failed) */
    if ( (status = mirc_add_relationship(MIR_MCC_ID_Code, code_IDS))
        != MC_SUCCESS) {
        return (status);
        }
    }

/* if (create of terminal for name failed) */
if ( (name_IDS = I_Create_IDS_STRING(textname)) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* extract a pointer to a static string containing the name of the object:
|  We need a pointer to a string that is not going to "go away".  The
|  inbound argument to this function is a pointer into a buffer that is
|  going to be re-used by the parser.  In order to record the name of this
|  object (for later use in generating an error message), we need to use
|  a copy of the string that contains the name of the object.  The I_Create...
|  call above does a copy and stores it inside the IDS.  We reach into the
|  IDS and use the pointer to the copy.
|
|  Record the new code at the current level, we'll need it to compute the
|  ISO object id for this object (and other contained objects if this object
|  is an entity class).
*/
mirc_record_idcodes(*level, MCC_code, code, &(parents_relationship),
                    name_IDS->idsu.t.t_v.string.str);

/* if (attempt to add "text name" relationship to current object failed) */
if ( (status = mirc_add_relationship(MIR_Text_Name, name_IDS)) != MC_SUCCESS) {
    return (status);
    }

#ifdef MCC_OIDS
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
/* if (current smi is DNA) */
if (bec->current_smi == MIR_SMI_DNA) {

    /* compute the ISO object ID under which this current object should be */
    /* registered */
    if ((oid_err = mirc_compute_oid(*level, MCC_code, &oid)) == NULL) {

        /* Provide end to added msg generated in mirc_compute_oid */
        if (supplementary_info == TRUE) {
            fprintf (stderr, " Code = %ld \"%s\"\n", code, textname);
            }

        /* if (attempt to register the current object using computed OID failed) */
        if ((status = I_Register_OID(oid, OID_MCC, new)) != MC_SUCCESS) {
            eflag = TRUE;       /* Make sure they get no binary */
            return (status);
            }
        }

    else {
        char    buff[200];

        eflag = TRUE;   /* make sure they get no binary */

        /* Only issue a message if it has a non-zero length */
        if (strlen(oid_err) > 0) {
            sprintf(buff,
                    MP(mp101,"Unable to generate MCC OID for '%s':\n       %s\n"),
                    textname,
                    oid_err);
            yyerror(buff);
            }
        }
    }
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
/* <><><><><> Possibly REMOVED UNTIL ARCHITECTURE HAS BEEN FINISHED <><><> */
#endif

return (MC_SUCCESS);
}

/* mirc_init_backend - Initialize the backend of compiler */
/* mirc_init_backend - Initialize the backend of compiler */
/* mirc_init_backend - Initialize the backend of compiler */

MC_STATUS
mirc_init_backend (context, binary_in_fn)

back_end_context  *context;       /* Pointer to back-end context block     */
char              *binary_in_fn;  /* Non-NULL: Input MIR database filename */


/*
INPUTS:

   "context" is a pointer to the back-end context block that must be
   initialized as part of back-end initialization.

   "binary_in_fn" - if non-null, is the name of a input binary MIR database
   file that must be loaded via "mir_internal.c" functions.  The compiler
   is to be initialized according to this input file rather than the
   builtin-types file.

OUTPUTS:

    The function returns one of:

    "MC_SUCCESS" - Operation was successful. The Back-End of the compiler is
            initialized so that compilation can start:

        If a binary input MIR database file name was supplied:

            * All existing MIR objects in the binary input MIR database file
              are loaded, including:

                  - The World MIR object

                  - All MIR-defined and SMI-defined relationships needed by
                    the compiler for entry into the relationship-tables of the
                    non-terminal MIR objects created as the compilation
                    progresses

                  - All MIR Objects for each dataconstruct definition of all
                    SMI datatypes and the internal compiler lists needed to
                    lookup these MIR Objects during compile-time.  (This
                    information would otherwise come from the "builtin_types"
                    file).

         else (normal initialization):

            * The "root" MIR object is created

            * The creation and registration of all MIR-defined and SMI-defined
              relationships needed by the compiler for entry into the
              relationship-tables of the non-terminal MIR objects created as
              the compilation progresses

            * The 'builtin_types.dat' file is opened and parsed, building
              the MIR Objects for each dataconstruct definition of all SMI
              datatypes and the internal compiler lists needed to lookup these
              MIR Objects during compile-time.

    If there was an error, a message is printed and an exit taken.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It wants to initialize
        the backend of the compiler so the compilation can begin.

    Purpose:
        This function handles the details of the setting up of the backend
        of the compiler.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize the back-end context block>
    <register the intermediate context block with intermediate module>

    if (a non-null binary input MIR database filename was supplied)
        if (perform a binary load: mirc_binary_load() FAILED)
            <exit>
        <signal "Initialized with binary MIR input file">

    else (* "Normal Initialization" *)

        (* Create the "String Seeds" *)
        (* Discover 1st contiguous range of printable characters *)
        for (every character code for 0 to 255)
            if (character code is printable)
                <increment running count of how many contiguous codes>
                if (we've not stored the starting code)
                    <store current character code>
            else (* not printable *)
                if (we've not stored the starting code)
                    <continue>
                else
                    <break>

        <allocate an "xref" array for size of printing range>
        if (allocate failed)
            <issue "mirc - Error: Out of Memory">
            <exit>

        for (every printable character starting with first code)
            <create a Terminal IDS containing a string of that character>
            <store address of IDS in xref[] array>
            <load xref array element number into IDS>

        <store the address of the xref array into the backend>
        <store number of entries in xref array into xref_count[<string>]>

        if (attempt to create a non-terminal for "The World" failed)
            <issue "Out of Memory" Message>
            <exit>

        <mark The World as "Created" (P.P.G: Programmer Playing God)>
        <record address of the world IDS as "current" object>
        <tell the parser what the Parent of Everything is>

        if (attempt to register the object as the "world" failed)
            <return status>

        <create and register MIR/SMI relationship objects>
        <give the World a name>
        <signal "Normal Initialization">

    if (attempt to open builtin-types file failed)
        <issue message>
        <exit>

    <set frontend ready to parse>
    <start parse of the builtin-types file>
    if ("Normal Initialization")
        if (parse failed)
            <issue message>
            <exit>
        <load the BIDT Version string into "The World">
        <add each SMI's name into "The World">
        <insert a Rebuild Count of "0">
        <add each Keyword string into "The World">
    else
        if (Binary Version string was NOT returned by parser)
            <issue message>
            <exit>
        if (returned Version differs from binary Input version)
            <issue message>

    for (every keyword received from command line)
        if (keyword is not LEGAL)
            <issue "Invalid Keyword "<kw>" specified>
            <exit>

    return <MC_SUCCESS>

OTHER THINGS TO KNOW:

    The following conventions are established with respect to the MIR Object
    called "The World":

        * The first MIR Object in the General Non-Terminal partition is the
          object for "The World".
        * The World's definition status is always marked as "CREATED" (to
          allow the "mirc_add_*()" functions to be unconditionally used to add
          relationships to it) even though from a strategic standpoint it is
          always "PREDEFINED" in some sense.
        * The object has "The World" as the target of "MIR_Text_Name".
        * This object may not be deleted.
        * All other SMI-defined objects (Entity Classes) are "Contained" by
          this object (this is the root of the MIR).
        * The World Object contains the following special information
          in the form of a Terminal target for relationship
          "MIR_Special" *in the following order*:

          - The Version String of the Builtin-Types file that was used
            by the compiler when the file was compiled, stored as a Terminal
            String.

          - As many "MIR_Special" entries as there were SMI Datatype
            definitions in the original Builtin-Types file, each stored as
            a Terminal String:

               + The names of the SMI datatype definitions are stored as
                 the target of the "MIR_Special" objects (e.g. "DNA", "SNMP")
               + The order of appearance is the order as they appeared in
                 the original builtin-types file, consequently the "code"
                 corresponding to the enumerated type "mir_smi_code" for
                 each SMI shoud match the order (e.g. "DNA"'s code is "1"
                 and it's name appears here first).

           - The "Rebuild Count" (stored as a Terminal Signed Number), this
             number is incremented by one each time it is read-in from
             an existing MIR binary file (after the original compilation
             that used only ASCII MSL files as input, this Rebuild Count is
             created as zero).

           - As many "MIR_Special" entries as there were Keywords parsed from
             the original Builtin-Types file, each stored as a Terminal String.

           NOTE:  The transition from receiving a target of "String" to a
                  target of "Signed Number" (for Rebuild Count) marks the
                  end of the list of SMI definitions when they are 'read-in'
                  by code in "mir_internal.c".

    Among the many places where this convention is depended on is code in
    "mirci_compiler_lists()" in "mir_internal.c" which expects to be able
    to extract the "special" information stored by the compiler and reproduce
    the internal compiler lists that were created when the original
    builtin-types file was read-in and parsed.
*/

#ifndef NULLFP
#define NULLFP  (FILE *)0
#endif

#ifndef NULLCP
#define NULLCP (char *)0
#endif

{
MC_STATUS       status;        /* Return status code from outbound calls     */
int             i,j;           /* General indices                            */
int             printable;     /* ASCII code for first printable character   */
int             print_count;   /* Count of printable characters              */
char            cb[2];         /* 1 character long "string"                  */
ext_int_xref    *xr;           /* Becomes the cross-reference array for seeds*/
object_id       oid;           /* "Root's" Object ID                         */
unsigned int    value[10];     /* Big enough for "Root"                      */
char            *pBTFileName = NULLCP;          /* builtin_types file name   */
FILE            *pBuiltInTypes = NULLFP;        /* File pointer for BIT file */
char            stringbuf[200];/* Error message buffer                       */
int             pstatus;       /* Parser status                              */
IDS             *name;         /* --> Newly created MIR name for "The World" */
IDS             *text;         /* --> Text strings for Rels. for "The World" */
IDS             *rebuild;      /* --> Rebuild count number (zero)            */
IDX             *kw_entry;     /* --> Keyword list, to next entry            */


/* Defined in the parser grammer (mir_yacc.y) */
extern IDS     *the_world_IDS; /* Parent of Everything (root object)*/
extern int      yylineno;      /* Defined in lex file, scanner line number */
extern BOOL     load_from_bin; /* TRUE: Loading existing binary MIR file   */
extern char    *ascii_bidt_version;   /* --> Version String parsed from    */
                                      /* the Builtin-Types file            */


/*
|============================================================================
|============================================================================
| Initialize the back-end context block
*/
bec=context;                    /* Copy the pointer to module-local ptr   */

for (i=0; i < MAX_SMI; i++) {   /* For all SMI lists...                   */
    bec->built_in[i] = NULL;    /* No Built-In (to SMI) datatype list yet */
    bec->built_up[i] = NULL;    /* No Built-Up datatype list yet          */
    bec->map_smicode_to_smistr[i] = NULL;  /* No SMIs defined yet         */
    }

/*
| Zap the stacks that keep track of info at all current levels during
| compilation.
*/
for (i=0; i < MAX_PATH_STACK; i++) {
    bec->name_stack[i] = NULL;
    bec->mcc_code_stack[i] = 0;
    bec->std_code_stack[i] = 0;
    bec->ent_type_stack[i] = (int) 0;
    }

bec->current_smi = MIR_SMI_UNKNOWN;/* No SMI selected yet                    */
bec->current_object = NULL;        /* No object being compiled-into yet      */
bec->fwd_ref_list = NULL;          /* No forward references yet              */
bec->binary_bidt_version = NULL;   /* No Binary Builtin Types Version string */
bec->legal_keyword_list = NULL;    /* No Legal Keyword List yet              */
                                   /* NOTE: The "selected" keyword list has  */
                                   /*       already been loaded!             */
bec->object_created = FALSE;       /* No object created yet                  */

/* Intermediate Context block within the Backend Context
|  Zap all the IDS list heads for all the different flavors of MIR Objects
|  and the corresponding pointers to last-of-(each)-flavor, for both active
|  and 'free' IDSs.
*/
for (i=0; i < MAX_IDS_TYPE; i++) {
    bec->i_context.flavors[i] = NULL;
    bec->i_context.last_of_flavor[i] = NULL;
    bec->i_context.top_of_free[i] = NULL;
    bec->i_context.last_of_free[i] = NULL;
    }

bec->i_context.index_top = NULL;/* No Intermediate "index" yet            */
bec->i_context.arc_count = 0;   /* No OIDs created yet for any objects    */

/*
|  Zap all the list heads for the arrays of cross-reference structures
|  needed for reading in an existing MIR database file and the corresponding
|  "count" array.
*/
for (i=0; i < MAX_IDS_TYPE; i++) {
    bec->i_context.xref[i] = NULL;
    bec->i_context.xref_count[i] = 0;
    }
/*
| End of Initialization of the back-end context block
|============================================================================
|============================================================================
*/


/*
| Register the intermediate context block with intermediate module so it
| has a context in which to operate.
*/
I_Register_Context(&bec->i_context);


/* if (a non-null binary input MIR database filename was supplied) */
if (binary_in_fn != NULL) {

    /* Announce loading of binary MIR input database file */
    fprintf(stderr,
            MP(mp102,"mirc - Info: Loading existing binary MIR database file\n       \"%s\"\n"),
            binary_in_fn);

    /* if (perform a binary load: mirc_binary_load() FAILED) */
    if (mirc_binary_load(binary_in_fn, context) != MC_SUCCESS) {
        fprintf(stderr,
                MP(mp103,"mirc - Error: Load failed for file\n       \"%s\"\n"),
                binary_in_fn);
        exit(BAD_EXIT);
        }

    /* signal "Initialized with binary MIR input file" */
    load_from_bin = TRUE;
    }

else {  /* "Normal Initialization": Not a "Load_From_Bin" */

    /* ================= Create the "String Seeds" ================= 
    |
    | Discover 1st contiguous range of printable characters
    |
    | for (every character code for 0 to 255)
    */
    for (i=0, printable = -1, print_count = 0;
         i <= 255;
         i++) {

        /* if (character code is printable) */
        if (isprint(i)) {

            /* increment running count of how many contiguous codes */
            print_count += 1;

            /* if (we've not stored the starting code) */
            if (printable == -1) {
                printable = i;  /* store current character code */
                }
            }
        else { /* not printable */

            /* if (we've not stored the starting code) */
            if (printable == -1)
                continue;
            else
                break;
            }
        }
    /*
    | At this point, "print_count" is number of printable characters starting
    | with "printable".
    */

    /* allocate an "xref" array for size of printing range
    |
    | if (allocate failed)
    */
    if ((xr = (ext_int_xref *) malloc(print_count*sizeof(ext_int_xref)))
        == NULL) {
        fprintf(stderr,
                MP(mp104,"mirc - Error: Out of Memory during Initialization\n"));
        exit(BAD_EXIT);
        }

    /* for (every printable character starting with first code) */
    for (i = printable, cb[1] = '\0', j = 0;
         i < (printable+print_count);
         i+=1, j+=1) {

        /*
        | Create a Terminal IDS containing a string of that character,
        | store address of IDS in xref[] array.  Note that we leave the
        | reference count at 0 so that "mir_external.c" code will strip these
        | (if they are never really used) before writing the output file.
        */
        cb[0] = i;      /* (Chop number to a character) */
        if ((xr[j].int_add = I_Create_IDS_STRING(cb)) == NULL) {
            fprintf(stderr,
                    MP(mp104,"mirc - Error: Out of Memory during Initialization\n"));
            exit(BAD_EXIT);
            }

        /* load xref array element number into IDS */
        xr[j].int_add->idsu.t.t_v.string.xr_index = j;
        }

    /* store the address of the xref array into the backend */
    bec->i_context.xref[I_T_OBJ_string] = xr;

    /* store number of entries in xref array into xref_count[<string>] */
    bec->i_context.xref_count[I_T_OBJ_string] = print_count;

    /* ================= End Create of "String Seeds" ================= */


    /* if (attempt to create a non-terminal for "The World" failed) */
    /* record address of IDS as "current" object */
    if ( (bec->current_object = I_Create_IDS(I_NT_OBJECT,
                                             IDS_DFT_TABLESIZE)) == NULL) {
        fprintf(stderr,
                MP(mp104,"mirc - Error: Out of Memory during Initialization\n"));
        exit(BAD_EXIT);
        }


    /* Tell the parser what The World (Parent of Everything) is */
    the_world_IDS = bec->current_object;

    /* mark The World as "Created" (P.P.G: Programmer Playing God) */
    the_world_IDS->idsu.nt.ntu.ds = DS_CREATED;

    /* Make sure our "object id" for "The World" is initialized */
    oid.count = 0;
    oid.value = value;

    /* Load the DSM prefix.  We use the latest and the greatest. */
    i = 0;
    value[i++] = 1;     /* ISO                                              */
    value[i++] = 3;     /* Identified Organization                          */
    value[i++] = 12;    /* ICD-European Computer Manufacturer's Assocation  */
    value[i++] = 2;     /* Member Organization                              */
    value[i++] = 1011;  /* Digital Equipment Corp.                          */
    value[i++] = 2;     /* Enterprise Management Architecture               */

    value[i++] = 1;     /* "Root" (per email from M.Sylor)          */
    oid.count = i;      /* Load count of arcs in OID                */

    /* if (attempt to register the object as the "world" failed) */
    if ( (status = I_Register_OID(&oid, OID_DNA, bec->current_object))
        != MC_SUCCESS) {
        return (status);
        }

    /* create and register MIR/SMI relationship objects */
    mirci_build_rel_list();

    /*
    | Give the World a name (it is the "current MIR Object")
    */
    /* if (create of terminal for name failed) */
    if ( (name = I_Create_IDS_STRING("The World")) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }

    /* if (add of relationship for name failed) */
    if ( (status = mirc_add_relationship(MIR_Text_Name, name)) != MC_SUCCESS) {
        return (status);
        }

    load_from_bin = FALSE;
    }

/* Endeavor to open the proper file as the "builtin_types" file */
pBTFileName = (char *) getenv( "ECA_MIR_BT_FILE" );
if (!pBTFileName)	/* if default not overridden*/
	pBTFileName = DEFAULT_BT_PATH;
pBuiltInTypes = freopen( pBTFileName, "r", stdin );

if (!pBuiltInTypes)
	{
	sprintf( stringbuf,
                MP(mp105,"Couldn't open builtin-types file %s"),
                pBTFileName );

	perror( stringbuf );
        exit(BAD_EXIT);
	};

/* Reset line number for yacc */
yylineno = 1;

/* remember this is top-level file */
set_top_file( pBTFileName );

if (load_from_bin == FALSE) {
    fprintf(stderr,
            MP(mp106,"mirc - Info: Loading the Built-In Data Types file\n       \"%s\"\n"),
            pBTFileName);
    }
else {
    fprintf(stderr,
            MP(mp107,"mirc - Info: Checking the Built-In Data Types file version\n       \"%s\"\n"),
            pBTFileName);
    }

/* Signal "No ASCII Builtin-Types Version String Parsed Yet" */
ascii_bidt_version = NULL;

/* Compile the Builtin-Types File! */
pstatus = yyparse();

/* if ("Normal Initialization") */
if (load_from_bin == FALSE) {
    if (pstatus != 0) {
        fprintf(stderr,
                MP(mp108,"Parse of compiler-internal builtin-types file failed.  Internal Code %d\n"),
                pstatus);
        exit(BAD_EXIT);
        }

    /* load the BIDT Version string into "The World" */
    if ( (text = I_Create_IDS_STRING(ascii_bidt_version)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }
    /* if (add of relationship for Version failed) */
    if ( (status = mirc_add_relationship(MIR_Special, text)) != MC_SUCCESS) {
        return (status);
        }

    /* add each SMI's name into "The World" */
    for (i = 0; i < MAX_SMI; i++) {   /* For each possible SMI name . . */

        /*
        |  If we didn't parse an SMI name from the builtin_types.dat file 
        |  for this code . . . just go for the next one
        */
        if ( bec->map_smicode_to_smistr[i] == NULL)
            continue;

        /* Create a string to hold the name */
        if ( (text = I_Create_IDS_STRING(bec->map_smicode_to_smistr[i]))
            == NULL) {
            return (MC_OUT_OF_MEMORY);
            }

        /* if (add of MIR_Special relationship for next SMI name failed) */
        if ( (status = mirc_add_relationship(MIR_Special, text))
            != MC_SUCCESS) {
            return (status);
            }
        }

    /*
    | Insert a Rebuild Count of "0" into "The World" at this point; demarks
    | the end of the SMI names and the start of the Keyword List.
    */
    if ( (rebuild = I_Create_IDS_SNUMBER(0)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }
    if ((status = mirc_add_relationship(MIR_Special, rebuild)) != MC_SUCCESS) {
        return (status);
        }

    /*
    | Add each Keyword string into "The World"
    */
    for (kw_entry = bec->legal_keyword_list;
         kw_entry != NULL;
         kw_entry=kw_entry->next) {

        /* Create a Terminal string with next keyword value */
        if ( (text = I_Create_IDS_STRING(kw_entry->name)) == NULL) {
            return (MC_OUT_OF_MEMORY);
            }

        /* if (add of MIR_Special relationship entry for keyword failed) */
        if ( (status = mirc_add_relationship(MIR_Special, text))
            != MC_SUCCESS) {
            return (status);
            }
        }
    }
else {
    /*
    | The yacc front-end aborts after parsing the BIDT File Version string.
    | We check to be sure we really got it, 'cause if we didn't, there was
    | some other problem.
    */

    /* if (Binary Version string was NOT returned by parser) */
    if (ascii_bidt_version == NULL) {
        fprintf(stderr,
                MP(mp109,"mirc - Error: Unable to parse Builtin-Types File version string\n"));
        exit(BAD_EXIT);
        }

    /* if (returned Version differs from binary Input version) */
    if (strcmp(ascii_bidt_version, bec->binary_bidt_version) != 0) {
        fprintf(stderr,
                MP(mp110,"mirc - Warning: Possible 'Builtin-Types' Version skew:\n"));
        fprintf(stderr,
                MP(mp111,"       Current Compiler Builtin-Types File Version = \"%s\"\n"),
                ascii_bidt_version);
        fprintf(stderr,
                MP(mp112,"       Compiled MIR Database Builtin-Types Version = \"%s\"\n"),
                bec->binary_bidt_version);
        }
    }

/*
| We want to make sure that the keywords specified on the command line
| are in fact legal keywords.
|
| for (every keyword received from command line)
*/
for (kw_entry = bec->selected_keyword_list;
     kw_entry != NULL;
     kw_entry = kw_entry->next) {

    /* if (keyword is not LEGAL) */
    if (mirc_keyword(IS_IT_A_KEYWORD, kw_entry->name) == FALSE) {
        /* issue "Invalid Keyword "<kw>" specified */
        fprintf(stderr,
                MP(mp113,"mirc - Error: \"%s\" is not a valid keyword.\n"),
                kw_entry->name);
        exit(BAD_EXIT);
        }
    }

return (MC_SUCCESS);
}

/* mirc_find_obj_IDS - Finds (by name) the IDS for an object in "c.o." table */
/* mirc_find_obj_IDS - Finds (by name) the IDS for an object in "c.o." table */
/* mirc_find_obj_IDS - Finds (by name) the IDS for an object in "c.o." table */

IDS *
mirc_find_obj_IDS (rel, name)

mir_relationship    rel;    /* Relationship from Current-Object to desired */
char                *name;  /* Name of the object desired                  */

/*
INPUTS:

    "rel" is the relationship that the current-object "stands-in" with
    respect to the object we are searching for.

    "name" is the value of the target of the relationship "MIR_Text_Name"
    in the sought object.

OUTPUTS:

    The function returns a pointer to the IDS of the sought object, or NULL
    if there was no match.

BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the current object has a non-empty relationship table.
        The front-end wants the IDS pointer that corresponds to the target
        of a relationship-table entry in the "current-object"'s table
        *which in turn* is an object with "name" as the target of it's
        MIR_Text_Name relationship.

    Purpose:
        This function handles the details of the scanning the relationship
        table of the current object for all entries with relationship "rel"
        whose targets (in essence) have names of "name".


ACTION SYNOPSIS OR PSEUDOCODE:

    for (every entry in the current-object's relationship table)
        if (relationship object matches the specified relationship)
            <select that entry's target to be searched for name-match>
            for (every object in the target's relationship table)
                if (next entry's relationship is "MIR_Text_Name")
                    if (target of that entry name is "name")
                        <return pointer to target>

    <return NULL>

OTHER THINGS TO KNOW:

    The reason d'etre for this function is to allow the front-end to
    properly "look-up" attribute names that are going to be placed into
    an entity-specific attribute group.  It is generalized to allow lookup
    "by-name" of anything.

    With V1.6, this function is also used to resolve forward references
    by "mirci_resolve_fwd_ref()".

    With V1.9, this function is used to 'walk' the tree of 'parents' for
    all entities that have a "PARENT=" clause.  Note that the 'walk' done
    by repeated calls to this function do not preclude the possibility that
    there may be multiple 'things' that have the same name.  This function
    only finds the first, and doesn't guarantee that one found is the 'only'.
*/

{
int     i,j;            /* General indices                     */
IDS     *cand_lvl_0;    /* --> Possible match IDS structure    */
IDS     *cand_lvl_1;    /* --> Possible match on name          */
int     cand_name_len;  /* Length of candidate name            */
int     name_len;  /* Length of giver name                     */

name_len = strlen(name);

/* for (every entry in the current-object's relationship table) */
for (i=0; i < bec->current_object->idsu.nt.entry_count; i++) {

    /* if (relationship object matches the specified relationship) */
    if (bec->current_object->idsu.nt.rel_table[i].rel_obj == 
                                    bec->map_rel_to_ids[(int) rel]) {

        /* select that entry's target to be searched for name-match */
        cand_lvl_0 = bec->current_object->idsu.nt.rel_table[i].tar_obj;

        /* for (every object in the target's relationship table) */
        for (j=0; j < cand_lvl_0->idsu.nt.entry_count; j++) {

            /* if (next entry's relationship is "MIR_Text_Name") */
            if (cand_lvl_0->idsu.nt.rel_table[j].rel_obj == 
                                         bec->map_rel_to_ids[MIR_Text_Name]){

                cand_lvl_1 = cand_lvl_0->idsu.nt.rel_table[j].tar_obj;
                cand_name_len = cand_lvl_1->idsu.t.t_v.string.len;

                /* if (target of that entry name is "name") */
                if (cand_name_len == name_len &&
                            caseless_str_equal(
                                cand_lvl_1->idsu.t.t_v.string.str,
                                name,
                                name_len)) {
                    /* return pointer to target */
                    return(cand_lvl_0);
                    }
                else {
                    break;  /* Blow out of inner loop, get new candidate 0 */
                    }
                }
            }
        }
    }

return (NULL);
}

/* mirc_find_fixedfield - Finds fixed-field code in an instance of a RECORD */
/* mirc_find_fixedfield - Finds fixed-field code in an instance of a RECORD */
/* mirc_find_fixedfield - Finds fixed-field code in an instance of a RECORD */

int
mirc_find_fixedfield (name)

char                *name;  /* Name of the fixed field whose code is sought*/

/*
INPUTS:

    "name" is the name of the fixed-field in the current instance of a record
    whose field code is sought.

    Also an implicit input to this function is the "current-object", which
    should be an IDS for an instance of a RECORD (soon to be a VARIANT RECORD)
    currently be compiled.

OUTPUTS:

    The function returns the field code of the fixed-field named, or "-1"
    if there was no fixed-field by that name.

BIRD'S EYE VIEW:
    Context:
        The caller is the front-end (yacc parser) of the compiler.  It is in
        a situation where the current object has a non-empty relationship
        table, and that object represents an instance of a RECORD (that is,
        it represents a "built-UP" dataconstruct based on "built-IN"
        dataconstruct "RECORD").  The front-end wants the Field Code
        associated with a "fixed-field" for this RECORD that has already been
        compiled.  In essence, it wants to find the MIR_Field_Name relationship
        that has as it's target the supplied "name" value, and then return
        the code that is the target of the immediately following MIR_Field_Code
        relationship.

    Purpose:
        This function handles the details of the scanning the relationship
        table of the current object for a match on MIR_Field_Name, and then
        returning the code that is the target of the following MIR_Field_Code
        relationship.


ACTION SYNOPSIS OR PSEUDOCODE:

    for (every entry in the current-object's relationship table)
        if (relationship object matches "MIR_Field_Name" relationship)
            if (target of MIR_Field_Name is "name")
                if (next entry matches "MIR_Field_Code")
                    return (number that is target of next entry)
    <return -1>

OTHER THINGS TO KNOW:

    The reason d'etre for this function is to allow the front-end to
    properly "look-up" a previously compiled fixed-field definition in
    the current-object (an instance of a RECORD).

    Note that we require an EXACT (case-sensitive) match on the field name.

*/

{
int     i;              /* Handy-Dandy Loop Index                   */
IDS     *candidate;     /* Possible target of MIR_Field_Name/Code   */


/* for (every entry in the current-object's relationship table) */
for (i=0; i < bec->current_object->idsu.nt.entry_count; i++) {

    /* if (relationship object matches "MIR_Field_Name" relationship) */
    if (bec->current_object->idsu.nt.rel_table[i].rel_obj == 
                                    bec->map_rel_to_ids[MIR_Field_Name]) {

        /* if (target of MIR_Field_Name is "name") */
        candidate = bec->current_object->idsu.nt.rel_table[i].tar_obj;
        if (strcmp(candidate->idsu.t.t_v.string.str, name) == 0) {

            /* if (next entry matches "MIR_Field_Code") */
            if (bec->current_object->idsu.nt.rel_table[i+1].rel_obj == 
                bec->map_rel_to_ids[MIR_Field_Code]) {

                /* return (number that is target of next entry) */
                candidate = /* --> IDS for Terminal Number for Field Code */
                    bec->current_object->idsu.nt.rel_table[i+1].tar_obj;
                return(candidate->idsu.t.t_v.snumber.value);
                }
            }
        }
    }

return (-1);
}

/* mirc_find_DC_code  - Find (builtin) DataConstruct's Code */
/* mirc_find_DC_code  - Find (builtin) DataConstruct's Code */
/* mirc_find_DC_code  - Find (builtin) DataConstruct's Code */

int
mirc_find_DC_code (dc)

IDS    *dc;    /* --> IDS for the (builtin) DataConstruct to be searched */

/*
INPUTS:

    "dc" - points to the IDS for the builtin DataConstruct whose smi "Code"
    the caller wants returned.

OUTPUTS:

    The function returns the numeric code that is the target of the
    relationship "MIR_DC_SMI_Code" if found.  If there is a lookup error
    (of any sort), the function returns "-1".


BIRD'S EYE VIEW:
    Context:
        The caller is the compiler front-end parser.  It has recognized
        a user declaration of a datatype as a "subrange", and it needs
        to check to be sure the underlying datatype is legal to be "subranged".

    Purpose:
        This function does the search necessary to obtain  the SMI-defined
        "code" value for the data-construct, thereby indicating what kind
        of datatype it is.


ACTION SYNOPSIS OR PSEUDOCODE:

    <assume we fail w/return value of -1>
    if (IDS is not Non-Terminal)
        <return retvalue>

    for (each entry in the relationship table)
        if (relationship IDS is not "MIR_DC_SMI_Code")
            <continue>
        if (target IDS is "I_T_OBJ_snumber")
            <set retvalue to target IDS number value>
        <break>

    <return retvalue>

OTHER THINGS TO KNOW:

    Note that if an IDS for a "built-UP" dataconstruct is passed, this function
    "does the right thing" by returning "-1" because it cannot find the
    "MIR_DC_SMI_Code" relationship in a "built-UP" dataconstruct.  This
    correctly restricts subranges to built-in types.
*/

{
int     retvalue;       /* The value we're going to return */
int     i;              /* Handy-Dandy Loop Index          */


/* assume we fail w/return value of -1 */
retvalue = -1;

/* if (IDS is not Non-Terminal for a Data Construct) */
if (dc->flavor != I_NT_OBJECT_DC)
    return (retvalue);

/* for (each entry in the relationship table) */
for (i = 0; i < dc->idsu.nt.entry_count; i++) {
    
    /* if (relationship IDS is not "MIR_DC_SMI_Code") */
    if (dc->idsu.nt.rel_table[i].rel_obj
        != bec->map_rel_to_ids[MIR_DC_SMI_Code]) {
        continue;
        }

    /* if (target IDS is a signed number) */
    if (dc->idsu.nt.rel_table[i].tar_obj->flavor == I_T_OBJ_snumber) {
        /* set retvalue to target IDS (signed) number value */
        retvalue = dc->idsu.nt.rel_table[i].tar_obj->idsu.t.t_v.snumber.value;
        }

    break;
    }

return (retvalue);

}

/* mirc_find_obj_CODE  - Find any (SMI) Object's (MCC or Standard) Code */
/* mirc_find_obj_CODE  - Find any (SMI) Object's (MCC or Standard) Code */
/* mirc_find_obj_CODE  - Find any (SMI) Object's (MCC or Standard) Code */

int
mirc_find_obj_CODE (obj, code_selector)

IDS             *obj;           /* --> IDS for the SMI Object to be Searched */
CODE_TYPE       code_selector;  /* Type of Code, MCC or Standard             */

/*
INPUTS:

    "obj" - points to the IDS for the SMI object to be searched for it's
            ID code.

    "code_selector" - indicates which type of code should be sought in the
                      the object.

OUTPUTS:

    The function returns the numeric code that is the target of the
    relationship "MIR_MCC_ID_Code" or "MIR_ID_Code" if found.
    If there is a lookup error (of any sort), the function returns "-1".


BIRD'S EYE VIEW:
    Context:
        The caller is the compiler front-end parser.  As the "PARENT=" clause
        is parsed, the parser "walks down" the containment hierarchy.  It
        needs to record the ID Codes of all the containing objects so that
        OIDs can be constructed.

    Purpose:
        This function does the search necessary to obtain the SMI-defined
        "code" value for the current SMI object.


ACTION SYNOPSIS OR PSEUDOCODE:

    <assume we fail w/return value of -1>
    if (IDS is not Non-Terminal)
        <return retvalue>

    <select proper MIR relationship according to code_selector>

    for (each entry in the relationship table)
        if (relationship IDS is not the selected relationship)
            <continue>
        if (target IDS is "I_T_OBJ_snumber")
            <set retvalue to target IDS number value>
        <break>

    <return retvalue>

OTHER THINGS TO KNOW:

    The code presumes all ID Codes are 'signed'.  If it hits one that
    is Unsigned, you don't get it back, you get "-1".
*/

{
int              retvalue;      /* The value we're going to return           */
int              i;             /* Handy-Dandy Loop Index                    */
mir_relationship selected_rel;  /* Relationship we're looking for a match on */


/* assume we fail w/return value of -1 */
retvalue = -1;

/* if (IDS is not Non-Terminal) */
if (obj->flavor != I_NT_OBJECT)
    return (retvalue);

/* select proper MIR relationship according to code_selector */
selected_rel = (code_selector == MCC_code) ?  (MIR_MCC_ID_Code) :
                                              (MIR_ID_Code);

/* for (each entry in the relationship table) */
for (i = 0; i < obj->idsu.nt.entry_count; i++) {
    
    /* if (relationship IDS is not selected relationship") */
    if (obj->idsu.nt.rel_table[i].rel_obj
        != bec->map_rel_to_ids[selected_rel]) {
        continue;
        }

    /* if (target IDS is a signed number) */
    if (obj->idsu.nt.rel_table[i].tar_obj->flavor == I_T_OBJ_snumber) {
        /* set retvalue to target IDS (signed) number value */
        retvalue = obj->idsu.nt.rel_table[i].tar_obj->idsu.t.t_v.snumber.value;
        }

    break;
    }

return (retvalue);

}

/* mirc_find_obj_NAME  - Find any (SMI) Object's "NAME" */
/* mirc_find_obj_NAME  - Find any (SMI) Object's "NAME" */
/* mirc_find_obj_NAME  - Find any (SMI) Object's "NAME" */

char *
mirc_find_obj_NAME (obj)

IDS             *obj;           /* --> IDS for the SMI Object to be Searched */

/*
INPUTS:

    "obj" - points to the IDS for the SMI object to be searched for it's
            "Name".

OUTPUTS:

    The function returns the Name (as a string) that is the target of the
    relationship "MIR_Text_Name".  If there is a lookup error (of any sort),
    the function returns <Name Unknown>.


BIRD'S EYE VIEW:
    Context:
        The caller is the compiler front-end parser.  As the "PARENT=" clause
        is parsed, the parser "walks down" the containment hierarchy.  It
        needs to record the names of all the containing objects so that
        a nice error message can be constructed (at some point, if there
        is an error building an OID).

        Alternatively, function in "mir_internal.c" needs the name of an
        existing (read-from-binary MIR database file) MIR Relationship Object.

    Purpose:
        This function does the search necessary to obtain the SMI-defined
        "NAME" value for the current SMI object.


ACTION SYNOPSIS OR PSEUDOCODE:

    <assume we fail w/return value of -1>
    if (IDS is not Non-Terminal)
        <return retvalue>

    for (each entry in the relationship table)
        if (relationship IDS is not the MIR_Text_Name)
            <continue>
        if (target IDS is "I_T_OBJ_string")
            <set retvalue to target IDS string value>
        <break>

    <return retvalue>

OTHER THINGS TO KNOW:

    The code presumes all "Names" are strings.  If it hits one that
    isn't, you don't get it back, you get <Name Unknown>
*/

{
char             *retvalue;     /* The value we're going to return           */
int              i;             /* Handy-Dandy Loop Index                    */


/* assume we fail w/return value of <Name Unknown> */
retvalue = "<Name Unknown>";

/* if (IDS is not Non-Terminal) */
if (obj->flavor != I_NT_OBJECT)
    return (retvalue);

/* for (each entry in the relationship table) */
for (i = 0; i < obj->idsu.nt.entry_count; i++) {
    
    /* if (relationship IDS is not "MIR_Text_Name"") */
    if (obj->idsu.nt.rel_table[i].rel_obj
        != bec->map_rel_to_ids[MIR_Text_Name]) {
        continue;
        }

    /* if (target IDS is a string) */
    if (obj->idsu.nt.rel_table[i].tar_obj->flavor == I_T_OBJ_string) {
        /* set retvalue to target IDS to string */
        retvalue = obj->idsu.nt.rel_table[i].tar_obj->idsu.t.t_v.string.str;
        }

    break;
    }

return (retvalue);

}

/* mirc_record_idcodes - Record ID Codes in path Stack */
/* mirc_record_idcodes - Record ID Codes in path Stack */
/* mirc_record_idcodes - Record ID Codes in path Stack */

void
mirc_record_idcodes(level, code_selector, code, rel, name)

int             level;          /* Level in MSL where ID code is defined   */
CODE_TYPE       code_selector;  /* Indicates "MCC" or "Standard" ID code   */
int             code;           /* The ID code value                       */
mir_relationship *rel;          /* If non-NULL, the relationship indicator */
char            *name;          /* If non-NULL, the name of entity element */

/*
INPUTS:

    "level" is used to index the entries in the stack arrays where the
    ID code is to be stored (with it's name and entity indicator).

    "code_selector" indicates what kind of code (MCC or Standard).

    "code" is the integer code value to be recorded.

    "rel" indicates what kind of entity.  (See the annotation in "mir.h"
    by the definition of the "ent_type_stack" regarding the value of this
    argument).  If this pointer is NULL, no information is being passed.

    "name" is the name of the entity element whose ID code is being recorded
    (if non-NULL).

OUTPUTS:

    The function returns nothing, but does record the ID code (and relationship
    indicator and name) for future reference by "mirc_compute_oid()".


BIRD'S EYE VIEW:
    Context:
        The caller is the compiler front-end parser or back-end function
        "mirc_create_object()".

        The front-end needs to record an ID code when the "DNA_CMIP_INT="
        clause is parsed as well as when the "PARENT=" clause is being
        parsed, in order to create a stack of entity sequence codes for
        use in creating an OID.

        The back-end function "mirc_create_object" needs to 
        record the MCC ID code on behalf of the front-end at the time
        a MIR object is being created.

    Purpose:
        This function accesses the stacks maintained in the back-end of
        the compiler to record ID codes and associated info on behalf of
        the front-end.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (level is greater than stack size)
        <issue terminal error message>
        <exit>

    if (code selector is MCC)
        <record the ID code at the selected level in the MCC stack>
        <record a "-1" as the Standard Code value>
    else
        <record the ID code at the selected leves in the STD stack>

    if (relationship indicator is present)
        <store it in the relationship stack entry>

    if (name is present)
        <store it in the name stack entry>


OTHER THINGS TO KNOW:

    Note that this function doesn't copy the string which is the name
    before recording it.  The caller is responsible for making sure that
    any storage holding this string is NOT released before the entry
    made in the back-end stack is superannuated.

    This code is a little presumptuous about the way it is called, as it
    assumes the front-end (actually "mirc_create_object") will call it first
    for any given level with an MCC code!  On that call it sets the
    corresponding "Standard" code to "-1" (ie "None") to cover the situation
    where the MSL does not have a "DNA_CMIP_INT=" clause.  (In this case, we
    want "-1"  stored as the Standard code in the stack, which means "None"
    appeared in the MSL.

    We can't have a situation where this function is called first to store
    a "standard" code (one from "DNA_CMIP_INT="), because the parser only
    sees that phrase after parsing any MCC code.  The parser ALWAYS calls
    this function whenever it either actually parses the MCC code OR "passes
    thru" the place in the MSL where it would appear (in which case it calls
    this function with "-1" explicitly).
*/

{
extern int      yylineno;      /* Defined in lex file, scanner line number */

/* if (level is greater than stack size) */
if (level >= MAX_PATH_STACK) {

    /* issue terminal error message */
    fprintf(stderr,
            MP(mp114,"Objects nested too deeply.  Internal stack size %d exeeeded at line %d\n"),
            MAX_PATH_STACK, yylineno);
    exit(BAD_EXIT);
    }

/* if (code selector is MCC) */
if (code_selector == MCC_code) {
    /* record the ID code at the selected level in the MCC stack */
    bec->mcc_code_stack[level] = code;

    /* record a "-1" as the Standard Code value */
    bec->std_code_stack[level] = -1;
    }
else {
    /* record the Standard Code value */
    bec->std_code_stack[level] = code;
    }

/* if (relationship indicator is present) */
if (rel != NULL) {
    /* store it in the relationship stack entry */
    bec->ent_type_stack[level] = *rel;
    }

/* if (name is present) */
if (name != NULL) {
    /* store it in the name stack entry */
    bec->name_stack[level] = name;
    }
}

/* mirc_create_fwd_ref - Create Forward Reference Block */
/* mirc_create_fwd_ref - Create Forward Reference Block */
/* mirc_create_fwd_ref - Create Forward Reference Block */

MC_STATUS
mirc_create_fwd_ref (block_type, block_ptr, string)

FR_TYPE   block_type;     /* The type of forward-reference block to create   */
FRB       **block_ptr;    /* If non-NULL, addr of where to return ptr to blk */
char      *string;        /* -> string (if block_type = FR_STRING), or NULL  */

/*
INPUTS:

    "block_type" indicates which kind of forward-reference block to create.

    "block_ptr", if non-NULL, indicates where to return the address of the
    newly created block.

    "string" - is expected to be non-NULL when "block_type" is "FR_STRING",
    in which case this null-terminated string is COPIED into dynamic-heap
    and a pointer to it is installed in the FR_STRING-type block whose address
    is then returned in "block_ptr" if "block_ptr" is non-NULL (which it
    should be in this instance).  "string" is not referenced if "block_type"
    is not "FR_STRING".

    Implicit inputs to this function when the type is not "FR_STRING" include

        * the current-object (being compiled-into, obtained through a call to
          "mirc_get_current()")

        * the current line number supplied by yacc through global variable
          "yylineno".


OUTPUTS:

    The function creates a "forward reference block" of the specified type
    to receive the information necessary to resolve a forward-reference
    after the last line of the current MSL has been parsed and returns it
    signalling "MC_SUCCESS".

    The actual resolution is accomplished by mirci_resolve_fwd_ref().

    This function fails (MC_OUT_OF_MEMORY) only for lack of malloc-able
    storage.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the parser would like to create a forward reference to a MIR
        object that (possibly) has not yet been parsed.

    Purpose:
        Builds a forward-reference block (partly filled in here, the rest
        by yacc) with information needed to resolve a forward reference.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (allocation of a forward reference block failed)
        <return MC_OUT_OF_MEMORY>

    <store block type into the new block>

    if (block_type is "FR_STRING")
        <obtain the string length + 1>
        if (attempt to allocate string storage failed)
            <return MC_OUT_OF_MEMORY>
        <copy string to storage>
        <Show "no next" block>
        <store pointer to string in FR_STRING block>
    else
        <string the block on the bec's forward reference block list>
        switch (type of block)
            case FR_IDENT:
                <copy in line number & current object>
                <NULL out the header for list of attribute names>
                <break>

            case FR_DEPENDS:
                <copy in line number>
                <NULL out the pointer to characteristic attribute name>
                <NULL out the header for list of enum names>
                <NULL out the IDS pointers for eclass & depends>
                <break>

    if (block_ptr is not NULL)
        <return the newly allocated block's address>

    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    As you'll note, the processing performed by this function is generic
    for all forward-reference block types except for "FR_STRING".  For these
    blocks, we expect the calling yacc action code to perform the necessary
    processing to string the block on an appropriate list.  The other kinds
    of forward-reference blocks are simply strung on the "forward-reference"
    list maintained in the backend-context block for resolution at the end
    of parsing the current MSL file.
*/

{
FRB     *new;           /* -> New Forward-Reference Block */
int     str_len;        /* Length of supplied string      */
extern  int yylineno;   /* Parser line number	          */

/* if (allocation of a forward reference block failed) */
if ( (new = (FRB *) malloc(sizeof(FRB))) == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* store block type into the new block */
new->flavor = block_type;


/* if (block_type is "FR_STRING") */
if (block_type == FR_STRING) {

    str_len = strlen(string) + 1;       /* obtain the string length + 1 */
    
    /* if (attempt to allocate string storage failed) */
    /* (store pointer to string in FR_STRING block) */
    if ( (new->fru.s.string = (char *) malloc(str_len)) == NULL) {
        return (MC_OUT_OF_MEMORY);
        }

    /* copy string to storage */
    strcpy(new->fru.s.string, string);

    /* Show "no next" block */
    new->next = NULL;
    }

else {
    /* string the block on the bec's forward reference block list */
    new->next = bec->fwd_ref_list;
    bec->fwd_ref_list = new;

    switch (block_type) {
        case FR_IDENT:
            /* copy in line number and current object IDS */
            new->fru.i.lineno = yylineno;
            new->fru.i.current = mirc_get_current();

            /* NULL out the header for list of attribute names */
            new->fru.i.attr_names = NULL;
            break;

        case FR_DEPENDS:
            /* copy in line number */
            new->fru.d.lineno = yylineno;

            /* NULL out the pointer to the characteristic attribute name */
            new->fru.d.char_attr = NULL;

            /* NULL out the header for list of enum names */
            new->fru.d.enum_list = NULL;

            /* NULL out the IDS pointers for eclass & depends */
            new->fru.d.depends = new->fru.d.eclass = NULL;

            new->fru.d.operator = -1;
            break;
        }
    }

/* if (they passed in a valid block_addr address) */
if (block_ptr != NULL) {
    /* return the newly allocated block's address */
    *block_ptr = new;
    }

return (MC_SUCCESS);
}

/* mirc_rec_to_vrec - Convert instance of RECORD to inst. of VARIANT RECORD */
/* mirc_rec_to_vrec - Convert instance of RECORD to inst. of VARIANT RECORD */
/* mirc_rec_to_vrec - Convert instance of RECORD to inst. of VARIANT RECORD */

MC_STATUS
mirc_rec_to_vrec (record_IDS)

IDS     *record_IDS;    /* --> IDS describing current instance of RECORD */

/*
INPUTS:

    "record_IDS" is a pointer to the IDS describing an instance of a RECORD
    (that needs to be changed to be an instance of a VARIANT RECORD).

OUTPUTS:

    The function changes the target of the *FIRST* MIR_Structured_As
    relationship found in the "record_IDS" to point to the IDS that corresponds
    to the builtin-type "<BIDT_VRECORD>", and returns MC_SUCCESS.

    This function fails (MC_FAILURE) only during serious internal error.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the parser has just hit a "CASE" statement inside a definition
        of a "built-up" datatype which looked like a RECORD.  Once the CASE
        statement is encountered, it is apparent that this is a definition of
        a VARIANT RECORD (not just a fixed-field RECORD).

    Purpose:
        This function handles the details of changing this instance over
        to VARIANT-RECORD from RECORD.


ACTION SYNOPSIS OR PSEUDOCODE:

    for (all entries in relationship table)
        if (relationship is MIR_Structured_As)
            <change target of MIR_Structured_As to <BIDT_VRECORD>>
            <return MC_SUCCESS>

     <return MC_FAILURE>


OTHER THINGS TO KNOW:

    Here is a list of soft drink types listed in descending order of
effervescence, with their approximate dissolved gas volumes:

          Type                    Gas Volume
        ginger ale                      4
        lemon-lime                   3.7 - 4
        cola                           3.5
        root beer                      3.0
        fruit flavors                1.5 - 2

                                  from "Imponderables", by David Feldman
*/

{
int     i;              /* Handy-Dandy Loop Index          */
IDS     *variant_IDS;   /* --> IDS for Variant-Record      */

if (mirc_find_datatype("BIDT_VRECORD", &variant_IDS) != MC_SUCCESS)
    return (MC_FAILURE);

/* for (all entries in relationship table) */
for (i = 0; i < record_IDS->idsu.nt.entry_count; i++) {
    
    /* if (relationship IDS is "MIR_Structured_As") */
    if (record_IDS->idsu.nt.rel_table[i].rel_obj
        == bec->map_rel_to_ids[MIR_Structured_As]) {

            /* change target of MIR_Structured_As to <BIDT_VRECORD> */
            record_IDS->idsu.nt.rel_table[i].tar_obj = variant_IDS;

            return (MC_SUCCESS);
            }
    }
return (MC_FAILURE);
}

/* mirc_use_parent - Use parent of Current Object as new Current Object */
/* mirc_use_parent - Use parent of Current Object as new Current Object */
/* mirc_use_parent - Use parent of Current Object as new Current Object */

MC_STATUS
mirc_use_parent (level)

int     *level;     /* --> Level Indicator maintained by front end */

/*
INPUTS:

    "level" - Pointer to the level indicator cell maintained by the front-end
    to indicate how many levels down into the MSL file we've parsed (or NULL)
    if not present.

OUTPUTS:

    The function switches the "current object" from the current MIR object
    to the parent of that object, the level is decremented by one and
    the function returns MC_SUCCESS.

    If the object has no parent, the function returns MC_NO_PARENT.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the parser would like to step the "current-object" it is
        compiling-into to the parent of that object.

    Purpose:
        This function handles the details of the change to the parent object.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (current object has null parent-pointer)
        <return MC_NO_PARENT>
    <set "current" pointer to value of current-object's parent pointer>

    if (level provided)
        <decrement level>

    <return MC_SUCCESS>

OTHER THINGS TO KNOW:

    There is no such fish as a sardine.  The term 'sardine' is a
    generic name for a quite a number of different small fish.  A fish
    doesn't become a sardine until it has been canned.

                                  -- from "Imponderables", by David Feldman
*/

{
/* if (current object has null parent-pointer) */                       
if (bec->current_object->idsu.nt.parent_obj == NULL) {
    return (MC_NO_PARENT);                                              
    }                                                                   

/* set "current" pointer to value of current-object's parent pointer */ 
bec->current_object = bec->current_object->idsu.nt.parent_obj;

if (level != NULL) {
  *level = *level - 1;
  /* <><> fprintf(stderr, "use-parent popping to level %d.\n",*level); */
  }

return (MC_SUCCESS);
}

/* mirc_get_current - Return the IDS Pointer to the Current Object */
/* mirc_get_current - Return the IDS Pointer to the Current Object */
/* mirc_get_current - Return the IDS Pointer to the Current Object */

IDS *
mirc_get_current ()

/*
INPUTS:

    The "current-object" is the implied input.

OUTPUTS:

    The function simply returns a pointer to the Intermediate Data Structure
    for the "current-object".

BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the parser would like to "save" its place (ie the object it is
        currently compiling-into).

    Purpose:
        This function returns the pointer to the IDS structure for the
        "current-object".


ACTION SYNOPSIS OR PSEUDOCODE:

    <return current-object pointer>

OTHER THINGS TO KNOW:

    This function only makes sense when used with mirc_use_parent() or
    mirc_set_current() functions.
*/

{
return (bec->current_object);
}

/* mirc_set_current - Set the Current Object to a particular MIR Object */
/* mirc_set_current - Set the Current Object to a particular MIR Object */
/* mirc_set_current - Set the Current Object to a particular MIR Object */

void mirc_set_current (new_current)

IDS *
new_current;       /* --> to object to become the "new" current-object */

/*
INPUTS:

    The "current-object" is the implied input along with the "new_current"
    object pointer.

OUTPUTS:

    The function changes the "current-object" to be the new supplied
    current-object.

BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler.  It is in a situation
        where the parser would like to restore a "saved" place (saved from
        a previous "mirc_get_current()" call.

    Purpose:
        This function changes "current-object".


ACTION SYNOPSIS OR PSEUDOCODE:

    <set new current object>
    <return>

OTHER THINGS TO KNOW:

    This function only makes sense when used with mirc_use_parent() or
    mirc_get_current() functions.
*/

{
bec->current_object = new_current;
return;
}

/* mirc_reset - Reset the MIR Compiler Backend for further operation */
/* mirc_reset - Reset the MIR Compiler Backend for further operation */
/* mirc_reset - Reset the MIR Compiler Backend for further operation */

MC_STATUS
mirc_reset ()

/*
INPUTS:

    None are passed explicitly.  However the "built-up" list of data
    constructs local to this module is input.

OUTPUTS:

    The function simply frees (and thereby empties) the "built-up" list
    of data constructs accumulated by the compiler during the parse of the
    last file, sets the "current SMI being compiled-under" to "Unknown" and
    returns MC_SUCCESS.

BIRD'S EYE VIEW:
    Context:
        The caller is the main loop of the compiler.  It has finished
        processing one input file and must reset the backend before going
        on to compile another input file.

    Purpose:
        This function performs reset chores that include dumping the list of
        "built-up" datatypes defined by the last file compiled  (this
        effectively resets the backend for upcoming processing) and resolves
        any forward references for this file.


ACTION SYNOPSIS OR PSEUDOCODE:

    <resolve all forward references>
    <Select the list for the SMI we are currently compiling-under>
    <for every entry on the selected list>
        if (reference count of built-up Non-Terminal is zero)
            <perform a remove-object on it>
        <blow away the entry>

    if (compiler mode is not "NORMAL")
        if (no object was created)
            <issue "Error: For Merge/Augment, no object's merged/augmented">

    <Show "no object-created">
    <Show entire list gone>
    <Show no SMI currently selected>
    <Show Compiler-Mode as "Normal">
    <return>

OTHER THINGS TO KNOW:

    This function may be expanded to perform other functions as needed
    if compiling becomes more sophisticated.

    Indeed, with V1.98 we are 'trimming' the un-used TYPE statements from
    the output, a painless 1% improvement which also cuts down the clutter
    when comparing compiler dumps.
*/

{
IDX         *scanner;       /* Used to scan down the list of index blocks in */
                            /* order to deallocate them.                     */
IDX         *dump;          /* Pointer to index block to be freed            */

mirci_resolve_fwd_ref();        /* Resolve and release forward references */

/* deallocate all IDX on the "built-up" list */                
/* Select the list for the SMI we are currently compiling-under */
scanner = bec->built_up[bec->current_smi];

/* Walk down the selected list and blow away all the entries on it */
while (scanner != NULL) {                                      

    /*
    | NOTE: The check for "I_NT_OBJECT" is *crucial* here, not only because
    |       we are only interested in dumping Non-Terminals that correspond
    |       to unreferenced "TYPE" statements, but because the "_remove_OBJECT"
    |       call does a recursive descent which may blow off IDS's that are
    |       farther down the IDX list we're scanning here.  For a melodramatic,
    |       boringly detailed discussion, see the OTHER THINGS TO KNOW section
    |       of "I_Reclaim_IDS()" in "mir_intermediate.c".  Basically, we skip
    |       NT's that are already marked with flavor "I_RECLAIMED" as well as
    |       NT's that aren't "General" (I_NT_OBJECT).
    |
    | if (reference count of built-up Non-Terminal is zero) */
    if (scanner->nt->flavor == I_NT_OBJECT
        && scanner->nt->idsu.nt.ref_count == 0) {

        /* perform a remove-object on it */
        mirc_remove_OBJECT(scanner->nt, bec);
        }

    /* Blow away the entry */
    free(scanner->name);     /* Deallocate the name storage */ 
    dump = scanner;          /* Copy ptr to block to go     */ 
    scanner=scanner->next;   /* Record where to go next     */ 
    free(dump);              /* Free the last block         */ 
    }                                                          

/* if (compiler mode is not "NORMAL") */
if (compiler_mode != CM_NORMAL) {

    /* if (no object was created) */
    if (bec->object_created == FALSE) {

        /* issue "Error: For Merge/Augment, no object's merged/augmented" */
        if (compiler_mode == CM_MERGE) {
            fprintf(stderr,
                    MP(mp154, "mirc - Error: Nothing Merged from this file\n"));
            }
        else {
            fprintf(stderr,
                    MP(mp155,"mirc - Error: Nothing Augmented from this file\n"));
            }
        }
    }

bec->object_created = FALSE;             /* Show "no object-created" */

bec->built_up[bec->current_smi] = NULL;  /* Show entire list gone */

bec->current_smi = MIR_SMI_UNKNOWN;      /* Show no SMI selected  */

compiler_mode = CM_NORMAL;      /* Show Compiler-Mode as "Normal" */

return(MC_SUCCESS);
}

/* mirc_error - Returns Error string given MIR Compiler error Status Code */
/* mirc_error - Returns Error string given MIR Compiler error Status Code */
/* mirc_error - Returns Error string given MIR Compiler error Status Code */

char *
mirc_error (code)

MC_STATUS       code;   /* The error status in need of interpretation */

/*
INPUTS:

    "code" - The code for which the caller wants an ASCII string.


OUTPUTS:

    The function returns a null-terminated string containing the
    interpretation of the error code.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser TRY() macro. Some sort of error
        has been detected.

    Purpose:
        This function converts the error code to a string that can be
        printed.


ACTION SYNOPSIS OR PSEUDOCODE:

    See Code.

OTHER THINGS TO KNOW:

   If the enumerated type definition for "MC_STATUS" in mir_compiler.h
   is expanded, this function must be altered accordingly.  (Fix the
   limit check at the start of this code too!)

   Modified with internationalization, we now always return the address
   of the static buffer.  This has obvious consequences for multiple calls
   on this function before using the value from a previous call.
*/

{
char            *msg;           /* Error message pointer            */
static char     msgbuf[250];    /* Internationalized Message buffer */


if ( ((int) code < 0 ) || 
     ((int) code > (int) MC_END_OF_PARTITION) ) {
    sprintf(msgbuf,
            MP(mp115,"mirc-- Unknown error code: %ld.\n"),( int ) code);
    return (msgbuf);
    }

switch (code) {
    case MC_OUT_OF_MEMORY:
         msg = MP(mp116,"Ran out of memory during operation"); break;

    case MC_SUCCESS:
         msg = MP(mp117,"Operation completed successfully"); break;

    case MC_FAILURE:
         msg = MP(mp118,"Operation failed in some manner"); break;

    case MC_REG_Already_OID:
         msg = MP(mp119,"Specified Object Id is already registered to another Object"); break;

    case MC_REG_Already_OBJ:
         msg = MP(mp120,"Specified Object Id is already registered to this Object"); break;

    case MC_REG_SMI_Already:
        msg = MP(mp121,"An OID in this SMI has already been registered for this object");
        break;

    case MC_FIND_Partial:
         msg = MP(mp122,"Partial match found"); break;

    case MC_FIND_Exact:
         msg = MP(mp123,"Exact match found"); break;

    case MC_FIND_Exact_Short:
         msg = MP(mp124,"Exact (but short) match found"); break;

    case MC_FIND_Exact_Long:
         msg = MP(mp125,"Exact (but long) match found"); break;

    case MC_DATATYPE_NOT_FND:
         msg = MP(mp126,"Datatype is undefined"); break;

    case MC_EXT_FAIL:
         msg = MP(mp127,"Externalization (Compiler Pass 2) Failed"); break;

    case MC_NO_PARENT:
         msg = MP(mp128,"No parent exists for USE_PARENT operation"); break;

    case MC_OPENFAIL_CONTINUE:
         msg = MP(mp129,"Error opening include file, continuing"); break;

    case MC_INC_LIMIT_EXCEEDED:
         msg = MP(mp130,"Too many nested include files"); break;

    case MC_END_OF_PARTITION:
         msg = MP(mp131,"End of Partition encountered reading MIR database file"); break;
    /* If you add a new one here, change the limit-check code above! */
    /* (and don't forget to "break"!)                                */

    }

/* Copy the internationalized phrase to static buffer */
strcpy(msgbuf, msg);

/* Return the buffer */
return(msgbuf);
}

/* mirc_record_smi - Record the datatypes under this specified SMI  */
/* mirc_record_smi - Record the datatypes under this specified SMI  */
/* mirc_record_smi - Record the datatypes under this specified SMI  */

MC_STATUS
mirc_record_smi (smi, smi_code)

char            *smi;           /* Character String 'name' for SMI           */
mir_smi_code    smi_code;       /* Code of SMI being switched to & recorded  */

/*
INPUTS:

    "smi" - The string identifying the SMI.  Currently the compiler recognizes
            at least one string for each valid SMI defined by "mir_smi_code"
            (defined in "mir.h") that *also* has an entry in the
            "builtin_types.dat" file.  This inbound argument specifies
            the "name" of an SMI that has been parsed from the builtin_types
            file.

    "smi_code" is the code for the smi being parsed from the builtin_types
            file.  

OUTPUTS:

    This function "registers" the named SMI and its associated code in
    the backend context block so that subsequent dataconstruct definitions
    for that SMI get parsed and recorded into the proper SMI back-end list
    (the list is subsequently selected at MSL compilation time by the
    parsing of the DEFINING-SMI clause, thereby selecting which dataconstructs
    become 'legal' for the compilation of that MSL).


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser code.  It has just parsed the
        "START_SMI" keyword during the parse of the builtin_types file
        which indicates the start of the definitions of the dataconstructs
        which are "builtin" to a particular SMI.

    Purpose:
        This function records the name and code of this SMI (whose
        dataconstructs are about to be parsed) within the compiler and
        sets up internal lists to record these dataconstructs 'under'
        this SMI.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (smi code is out of range)
        <issue error message: SMI Code 'x' out of range>
        <return MC_FAILURE>

    for (each possible SMI slot)
        if (slot is not NULL)
            if (slot's SMI name case-insensitive matches passed SMI name)
                <issue error message: SMI name 'name' already in use, code 'x'>
                <return MC_FAILURE>

    if (attempt to allocate space for a copy of the SMI name failed)
        <return MC_OUT_OF_MEMORY>
    <copy string name to storage>
    <set current SMI to be specified SMI code>
    <return MC_SUCCESS>


OTHER THINGS TO KNOW:

    While the "smi_code" is ostesibly typed "mir_smi_code", the reality is
    that it is an integer value parsed from the "builtin_types.dat" file,
    and it could be literally anything.  This function validates this
    value as a 'real' value of type "mir_smi_code".  Once this is done,
    the compiler can reliably map the name of the SMI (taken from the
    value given by the DEFINING-SMI clause) into the proper compiler
    enumerated symbol (value) for 'mir_smi_code'.
*/

{
int     i;      /* Index */

/* if (smi code is out of range) */
if (smi_code < 0 || smi_code >= MAX_SMI) {
    /* issue error message: SMI Code 'x' out of range */
    fprintf(stderr,
            MP(mp132,"SMI Code %d from builtin_types file is out of range.\n"),
            (int) smi_code);

    return (MC_FAILURE);
    }


/* For each possible SMI name . . */
for (i = 0; i < MAX_SMI; i++) {    

    /* if (slot is not NULL) */
    if ( bec->map_smicode_to_smistr[i] != NULL) {

        /* if (slot's SMI name case-insensitive matches passed SMI name) */
        if (   strlen(bec->map_smicode_to_smistr[i]) == strlen(smi)
            && caseless_str_equal(bec->map_smicode_to_smistr[i],
                                  smi,
                                  strlen(smi))) {
            /* issue error message: SMI name 'name' already in use, code 'x' */
            fprintf(stderr,
                    MP(mp133,"SMI name '%s' already in used, code %d \n"),
                    smi,
                    i);

            return (MC_FAILURE);
            }
        }
    }

/* if (attempt to allocate space for a copy of the SMI name failed) */
if ((bec->map_smicode_to_smistr[smi_code] = (char *) malloc(strlen(smi) + 1))
    == NULL) {
    return (MC_OUT_OF_MEMORY);
    }

/* copy string name to storage */
strcpy(bec->map_smicode_to_smistr[smi_code], smi);

/* set current SMI to be specified SMI code */
bec->current_smi = smi_code;

return (MC_SUCCESS);

}

/* mirc_set_smi - Set the currently selected SMI to compile under */
/* mirc_set_smi - Set the currently selected SMI to compile under */
/* mirc_set_smi - Set the currently selected SMI to compile under */

MC_STATUS
mirc_set_smi (smi, smi_code)

char            *smi;           /* Character String describing SMI */
mir_smi_code    *smi_code;      /* Code of SMI being switched to   */

/*
INPUTS:

    "smi" - The string identifying the SMI.  Currently we recognize
            at least one string for each valid SMI defined by "mir_smi_code"
            (defined in "mir.h") that also has an entry in the
            "builtin_types.dat" file.

    "smi_code" is the address of a cell to be set to the code for
            the smi switched to.

OUTPUTS:

    The function sets internal back-end indicators to select the proper
    tables for compilation of the current MSL and then returns the
    internal compiler code for the selected SMI.  If there is a screwup and
    a valid SMI cannot be selected, MIR_SMI_UNKNOWN is selected.


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser code.  It has just parsed the
        line that indicates the SMI the MSL was written to.

    Purpose:
        This function sets up the compiler to use the proper tables
        built from "builtin_types.dat" file data to properly recognize
        this MSL's datatypes.


ACTION SYNOPSIS OR PSEUDOCODE:

    See Code.

OTHER THINGS TO KNOW:

    We do a linear search down all currently defined names of SMIs
    using a case-insensitive compare to discover the value of an enumerated
    type "mir_smi_code" to use as the value of cell "bec->current_smi".
*/

{
int     i;      /* Index */

for (i = 0; i < MAX_SMI; i++) {         /* For each possible SMI name . . */

    /* If we didn't parse an SMI name from the builtin_types.dat file for */
    /* this code . . . just go for the next one                           */
    if ( bec->map_smicode_to_smistr[i] == NULL)
        continue;

    if (   (strlen(smi) == strlen(bec->map_smicode_to_smistr[i]))
        && (caseless_str_equal(bec->map_smicode_to_smistr[i],
                               smi,
                               strlen(smi)) == TRUE)) {
          
        bec->current_smi = (mir_smi_code) i;
        *smi_code = bec->current_smi;
        return (MC_SUCCESS);
        }
    }

/* If we examine all possibilities and find no match, return empty-handed */
bec->current_smi = MIR_SMI_UNKNOWN;
*smi_code = bec->current_smi;
return(MC_FAILURE);

}

/* mirc_get_def_status - Get the "Definition-Status" of the current object */
/* mirc_get_def_status - Get the "Definition-Status" of the current object */
/* mirc_get_def_status - Get the "Definition-Status" of the current object */

DEF_STATUS
mirc_get_def_status ()

/*
INPUTS:

    The "current-object" (as indicated by the backend-context) is the implied
    input.

OUTPUTS:

    The function returns the "definition-status" of the current-object.

BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser code.  It is deciding whether or not
        to perform some particular processing based on the compiler-mode
        (Normal/Merge/Augment) and the 'Definition-Status' of the current
        object it is working on.

    Purpose:
        This function merely reaches into the IDS representing the
        current-object and returns it's definition-status.


ACTION SYNOPSIS OR PSEUDOCODE:

    See Code.

OTHER THINGS TO KNOW:

    BEWARE:

       The "definition-status" is defined only for Non-Terminals of type
       "General" (I_NT_OBJECT) *only at certain times*.  The whole purpose of
       this field is to 'automatically' prevent the "mirc_add_rel_*()" family
       of functions from writing into a MIR object that should not be written
       into (like a predefined object that already exists that is mentioned in
       a Merged file) and also to allow the "mirc_add_rel_*()" family of
       functions to discover when the user has added a definition (like
       "SYMBOL="...)  in an Augment file on a predefined MIR object (which is
       an error in an Augment file: you can only set things like "SYMBOL=" on
       an object mentioned in an Augment file *IF* that Augment file caused
       that object to be Created!)

       When the compiler is operating in "Normal" mode, and has loaded
       *NO* MIR binary input database file and all input files have been
       compiled in Normal mode, then all objects will always have a
       definition-status of "CREATED" (DS_CREATED).  This is done by
       "mirc_create_object()" as it creates each object in turn.

       When the "mir_internal.c" logic loads objects from an input binary
       MIR database file, all the objects are logically "PREDEFINED", but
       they *DO NOT* have their definition-status field set one way or
       another.

       If such a load happens, and then the next file is compiled under
       "Normal" mode, either it is a GLOBAL ENTITY in the next file (and
       the definition-status of all the other MIR objects that came in from
       the binary file really doesn't matter) OR it's a CHILD ENTITY, and the
       PARENT= clause will cause a descent to occur through the MIR objects
       that came in from the binary MIR database file, down to the entity-class
       that should contain the new CHILD ENTITY.  At that point, new MIR
       Objects are created starting at that point 'downward' (in the hierarchy),
       and the definition-status of the Entity-Classes 'above' (stepped-over by
       the PARENT= clause logic) (again) doesn't matter.

       However, when the compiler launches into compiling a particular file
       in "Merge" mode (after either compiling some files in Normal and/or
       having loaded a binary MIR database file) then it is expected that
       some of the stuff in the incoming Merge-mode file *duplicates*
       definitions of things that have already been loaded (via binary MIR
       load or some other earlier compiled file...although this is silly).
       In this case, for each major object (Entity-Class, attribute, etc) the
       "mirc_create_object()" function tries to look up the about-to-be-created
       object to see if it already exists.  If it does, then that earlier
       definition is returned by "mirc_create_object()" as though it were
       just created, *BUT* it's definition-status is marked "PREDEFINED".

       As the compilation continues on such a PREDEFINED object, eventually
       the parser will be calling one or more of the "mirc_add_rel_*()"
       functions to add definitions (relationship-table entries) for things
       about that PREDEFINED object (for instance, "SYMBOL=").  If the
       mode is Merge (as in our current example), the "mirc_add_rel_*()"
       functions notice that the object is PREDEFINED, and they *don't*
       write anything new into the PREDEFINED object.  The definition of
       the object is presumed to be correct, and the Merge file contents
       do not affect it.  Eventually the parser reaches some object in the
       Merge input file which, when the parser calls "mirc_create_object()",
       turns out to be not previously defined.  This is the start of the
       stuff the user presumably wants to merge-in.  "mirc_create_object()"
       really does create a new MIR object, and marks it "CREATED".  Subsequent
       parser calls to the "mirc_add_rel*()" family of functions result in
       new stuff actually being written into the relationship table of this
       new object because it's definition-status is recognized as "CREATED".

       Similarly, if the mode is Augment, "mirc_create_object()" is operating
       in exactly the same manner as for Merge, however the "mirc_add_rel*()"
       functions SHOULD NEVER BE INVOKED in Augment mode on a current object
       that is marked PREDEFINED!  This is because the rules surrounding the
       use of Augment files specifically preclude attempting to change the
       definition (---in MCC parlance: *any* definition) of an existing
       MIR object.  MCC 'definitions' (like the value of "SYMBOL=") may only
       be specified in an Augment File for MIR objects that are CREATED by
       being first mentioned in that Augment file.  Consequently the
       "mirc_add_rel_*()" family of functions issue an error message under
       this circumstance.  A special check has to be made on the error checking
       in this case, as when the compiler repeats the processing of an AUGMENT
       file for the second and subsequent entity-classes that are listed for
       modification, the compiler is going to parse any TYPE statements at
       the beginning of the file for the second time.  In this situation, the
       compiler is seeing an attempt to modify a PREDEFINED object (the object
       that represents the TYPE statement) and under AUGMENT, this would
       generate an error.  Objects that represent TYPE statements are marked
       PREDEFINED_TYPE when they are encountered for a second time, and the
       error checking in the mirc_add_rel*() family of functions recognizes
       this and does not issue the normal error message that otherwise would
       be generated.
*/

{
return(bec->current_object->idsu.nt.ntu.ds);
}

/* mirc_set_def_status - Set the "Definition-Status" of the current object */
/* mirc_set_def_status - Set the "Definition-Status" of the current object */
/* mirc_set_def_status - Set the "Definition-Status" of the current object */

void
mirc_set_def_status (def_st)

DEF_STATUS      def_st;         /* Definition-Status to set */

/*
INPUTS:

    "def_st" is the new definition-status to be set into the current-object.

    The "current-object" (as indicated by the backend-context) is the implied
    input.

OUTPUTS:

    The function sets the "definition-status" of the current-object, and
    returns (nothing).


BIRD'S EYE VIEW:
    Context:
        The caller is the yacc parser code.  It is has discovered that it
        needs to force the "definition-status" of the current object to
        a particular value.

    Purpose:
        This function merely reaches into the IDS representing the
        current-object and sets it's definition-status.


ACTION SYNOPSIS OR PSEUDOCODE:

    See Code.

OTHER THINGS TO KNOW:

    See the discussion under OTHER THINGS TO KNOW for the corresponding
    "mirc_get_def_status()" function in this module.
*/

{
bec->current_object->idsu.nt.ntu.ds = def_st;
}

/* mirc_compute_oid - COMPUTE the MCC or DNA Object ID of a MIR object */
/* mirc_compute_oid - COMPUTE the MCC or DNA Object ID of a MIR object */
/* mirc_compute_oid - COMPUTE the MCC or DNA Object ID of a MIR object */

char *
mirc_compute_oid (level, code_selector, p_oid)

int             level;          /* Front-End Level indicator             */
CODE_TYPE       code_selector;  /* Indicates "MCC" or "Standard" OID req */
object_id       **p_oid;        /* -->> to receive ISO Object IU         */

/*
INPUTS:

    "level" is the front-end's "level" indicator showing 'how far down' into
    a stack of entity definitions we are.  This value is used to access the
    four "stacks" (defined in the compiler's back-end context).  This value
    points to the top of the stack(s).

    "code_selector" indicates which kind of OID the caller wants to have
    built: MCC or DNA (standard).  (This is something separate and apart
    from the SMI we're operating under. . .).

    "p_oid" is the address of a pointer to (be set to) an object id on return.

ALSO INPUT:

    Global argument "containing_arg_type", defined in the yacc grammar.
    The value of this cell is used to determine the arc inserted when the
    current entity-type is "ARGUMENT"  (since we need to know what the
    argument is actually *for*, this global flag set by parser provides thi
    information.


OUTPUTS:

    On success, the function returns via "p_oid" a pointer to an object id
    containing the computed ISO Object ID that is appropriate (using either
    the MCC or the "standard" (DNA) codes and rules.  The function returns
    NULL on success.

    On failure, a string describing the error is returned.  This will most
    likely as not be because one or more entity-codes (gotten from the back
    end stack) that were needed were not defined (ie were "-1" or "-2").
    If we've already reported an error, an "empty" string ("") is returned.
    The caller should check the length of the error message and behave
    accordingly.


BIRD'S EYE VIEW:
    Context:
        The caller is creating a Non-Terminal containing a description
        of a MIR object (or it has already been created and a DNA_CMIP_INT
        clause has just been parsed, and we need to register an OID using
        the value).

    Purpose:
        This function computes what the ISO object ID for a MIR
        object ought to be given its position in the containment hierarchy
        within the MIR (ie what codes came before it and what it is).

ACTION SYNOPSIS OR PSEUDOCODE:

    See the Code.

OTHER THINGS TO KNOW:

    The algorithm used here is based on the spec in Appendix C of DNA Phase V
    Network Management Specification.

    From NetMan Appendix C:

                            Value  Thing defined
---------------------------------  --------------------------------------
                     entities (1)  Entity Class
                   attributes (2)  Attribute
                      actions (3)  Action
     action-request-arguments (4)  Action Request Argument
             action-responses (5)  Action Response
    action-response-arguments (6)  Action Response Argument
            action-exceptions (7)  Action Exception
   action-exception-arguments (8)  Action Exception Argument
                       events (9)  Event
             event-arguments (10)  Event Argument
  universal-attribute-groups (11)  Universal Attribute Group
   specific-attribute-groups (12)  Entity Class Specific Attribute Group


     Note that this version of the code contains the following interim
     hacks:

     * When "MCC" OIDs are constructed, they are just like standard (DNA)
       OIDs except an extra arc containing "22" is inserted immediately
       after the "2" arc for DNA.

     * The logic in this function may be misusing the "11" code value
       for "universal-attribute-groups" shown in the table above. It is
       being used in this code with the class-code sequence to generate
       a unique OID for the MIR object that is, essentially, a list of
       pointers (via "MIR_List_Entry") to each attribute in the *PARTITION*!
       (instead of "group", whatever that subtle difference is).

     * There are no codes given in the appendix for Event Groups or
       Event Partitions.  We use "1010101" and "2020202" respectively.

       FLASH from M.Sylor:
           - Event Group code is "27", ("1010101" is retired).
           - Event Partitions remain unaccounted for, and we continue to
             use the bogus "2020202" code for it.

    Lastly, note that "-1" gets stuck in a code stack (either MCC or STD)
    when the frontend doesn't parse a valid code from the MSL.  We check
    for "-1", and when found, we generate a message.  Then the "-1" is
    changed to a "-2" which means "Bogus: Message Already Issued".
*/

/*
|  This marker value is used to delimit the entity class codes when they are
|  placed into an Object ID. (See NETMAN Appendix C)
*/
#define END_OF_CLASS 126

/*
|  This define declares the maximum number of arcs that may appear in an OID 
|  generated by this function.
*/
#define MAX_OID_ARCS 50
{
int     *stack;         /* Loaded to point to the right stack of codes       */
int     i,j;            /* general indices                                   */
BOOL    marker_placed=FALSE;
                        /* TRUE: We've encoded "End of Class" into Obj ID    */
static
char    msg[100];       /* Message Buffer for error message return           */

/* Permanent storage, a pointer to which is returned on success */
static object_id        oid;
static unsigned int     value[MAX_OID_ARCS];

/* Defined in yacc grammer.  See ALSO INPUT above. */
extern mir_relationship containing_arg_type;

/* Make sure our "object id" is initialized */
oid.count = 0;
oid.value = value;

/* Load the DSM prefix.  We use the latest and the greatest. */
i = 0;
value[i++] = 1;         /* ISO                                               */
value[i++] = 3;         /* Identified Organization                           */
value[i++] = 12;        /* ICD-European Computer Manufacturer's Assocation   */
value[i++] = 2;         /* Member Organization                               */
value[i++] = 1011;      /* Digital Equipment Corp.                           */
value[i++] = 2;         /* Enterprise Management Architecture                */


/* <><><><><><><><> THIS IS BOGUS  <><><><><><><><>  */
/* If it's an MCC OID they want, stick in the 22.  This is BOGUS */
if (code_selector == MCC_code) {
    value[i++] = 22;         /* MCC */
    }
/* <><><><><><><><> THIS IS BOGUS  <><><><><><><><>  */


/* Seventh Arc: Where the MIR starts for us                             */

/*
|  In the next arc, we are obliged to put the number indicated in the
|  table above for the "kind of thing" we are working on.  The MIR
|  object type (actually, the Parent Relationship) of that thing is 
|  the last entry in the stack of entity-relationship types maintained
|  in the backend.
|
|  The general solution is to examine it and load this discriminating
|  arc according to the table above.  This works for all values of "rel".
*/

switch (bec->ent_type_stack[level]) {
     case MIR_Cont_entityClass:  /* type of entity within hierarchy */       
        value[i++] = 1;
        break;

     case MIR_Cont_attribute:    /* identifiable component of entity state */
        value[i++] = 2;
        break;

     case MIR_Cont_attrPartition:      /* attrib partition of attributes */
        value[i++] = 11;
        break;

     case MIR_Cont_directive:           /* what you can do to entity */
        value[i++] = 3;
        break;

     case MIR_Cont_argument:            /* ARGUMENT (of some sort) */

        switch (containing_arg_type) {  /* What Kind of Argument? */

            case MIR_Cont_request:      /* Action (directive) request */
                value[i++] = 4;
                break;

            case MIR_Cont_response:     /* Action Response Argument */
                value[i++] = 6;
                break;

            case MIR_Cont_exception:    /* Action Exception Argument */
                value[i++] = 8; 
                break;

            case MIR_Cont_event:        /* Event Argument */
                value[i++] = 10;        
                break;

            default:
                fprintf(stderr,
                        "Front-End coding error: Invalid containing_arg_type value: %d\n",
                        containing_arg_type);
                exit(BAD_EXIT);
            }
        break;

     case MIR_Cont_response:           /* successful outcome of directive */
        value[i++] = 5;
        break;

     case MIR_Cont_exception:          /* unsuccessful outcome of directive */
        value[i++] = 7;
        break;

     case MIR_Cont_event:              /* event associated with entity */
        value[i++] = 9;
        break;

     case MIR_Cont_attrGroup:          /* entity specific attribute group */
        value[i++] = 12;
        break;


/* <><><><><><><><> THIS IS BOGUS  <><><><><><><><>  */
     case MIR_Cont_eventPartition:
        value[i++] = 2020202;
        break;
/* <><><><><><><><> THIS IS BOGUS  <><><><><><><><>  */

     case MIR_Cont_eventGroup:
        value[i++] = 27;
        break;


     default:
        fprintf(stderr,
        "<><><> Front-End coding error in call to mirc_compute_oid()!\n");
        break;
  }

/*
|  Now the entity class codes must be added followed by the particular code
|  for atttribute, action, etc.  The back-end stack (either for standard codes
|  or MCC codes) contains these values.  We select the right stack, and then
|  copy all the codes until we hit the first code that DOES NOT apply to
|  an entity-class: there we stick in the marker (if it is not an entity-class)
|  and continue to copy the distinguishing codes in until the code for this
|  actual object is copied.
|
| The marker code is END_OF_CLASS.
*/

/* Select the right stack */
if (code_selector == MCC_code)
    stack = bec->mcc_code_stack;
else
    stack = bec->std_code_stack;


for (j = 1; j <= level; j++) {

    if (j >= MAX_OID_ARCS) {
        fprintf(stderr,
                MP(mp134,"Maximum OID arc length %d exceeded.\n"),
                MAX_OID_ARCS);
        exit(BAD_EXIT);
        }

    /* if selected code is not legal */
    if (stack[j] == -1) {
        sprintf(msg,
                MP(mp135,"MIR Object named '%s' has no %s code assigned"),
                bec->name_stack[j],
                ((code_selector == MCC_code) ? "MCC ID" : "ID"));
        stack[j] = -2;  /* Message Issued, don't issue it again */
        return(msg);
        }
    /* If message already issued, signal this with an "empty" string */
    else if (stack[j] == -2) {
        return ("");
        }

    if (   (marker_placed == FALSE)
        && (bec->ent_type_stack[j] != MIR_Cont_entityClass)
       ) {
        value[i++] = END_OF_CLASS;      /* Stick in E-class marker */
        marker_placed = TRUE;
        }

    value[i++] = stack[j];

    }

/* Now indicate how big the ISO Object ID really is and return it to the */
/* caller */
oid.count = i;
*p_oid = &oid;

/* Print the ISO Object ID to complete the supplementary output if needed */
if (supplementary_info == TRUE) {
    for (i = 0; i < oid.count; i++) {
        if (i != 0) fprintf(stderr, ".");
        else fprintf(stderr, "Info: Creating(%d)==> ",level);
        fprintf(stderr, "%ld", oid.value[i]);
        }
    }

return (NULL);
}

/* mirci_resolve_fwd_ref - Resolve All Forward References */
/* mirci_resolve_fwd_ref - Resolve All Forward References */
/* mirci_resolve_fwd_ref - Resolve All Forward References */

extern int eflag;       /* Implied argument, defined in mir_yacc.y */

static void
mirci_resolve_fwd_ref( )

/*
INPUTS:

    No explicit inputs, but the backend context is accessed and the
    forward reference list is referenced and ultimately deleted.

    Also the error flag "eflag" defined by the yacc parser code is
    referenced via "extern".

OUTPUTS:

    The function returns nothing explicitly, but on successful completion
    it will have walked (and released) all the forward reference blocks
    on the forward-reference list in the backend context block.  Changes
    to the IDS representation of the just-compiled code will have been made
    to resolve forward references left "dangling" after the parser has seen
    the entire input file.

    On failure, one or more forward-references will not have been resolved,
    error messages will have been issued, but the forward-reference list will
    nonetheless have been released.

BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler just after the yacc
        parser has finished examining the input file.  The front end is
        closing up and resetting the compiler for the next input file, if any.

    Purpose:
        This function resolves all outstanding forward-references by the
        just-compiled input source file or issues error messages if resolution
        is not possible.


ACTION SYNOPSIS OR PSEUDOCODE:

    <initialize pointer to value of forward-reference list head in bec>
    <get the current IDS pointer and save it>
    <save current IDS locally>

    while (forward-reference blocks remain to be processed)

        switch (kind of forward reference block)

            case FR_IDENT:
                for (all strings blocks to be resolved in FR block)
                    <set the current IDS pointer to f-ref block value>
                    if (attempt to find attribute of given name succeeded)
                        if (attempt to add rel. "MIR_Indexed_By" failed)
                            <issue "mirc - attempt to add relationship failed">
                            <set eflag TRUE>
                    else
                        <issue
                          "mirc - forward reference to "%s" failed at line %d">
                        <set eflag TRUE>
                    <step index to point to next string>
                    <release the string in the string block>
                    <release the string block>
                <break>

            case FR_DEPENDS:
                <perform resolution of DEPENDS-ON Fwd Ref blocks>
                <Blow off all (any) strings in this kind of F-R Block>
                <break>

            default:
                <issue "mirc - internal compiler error: fwd-ref block value %d">

        <copy the "next" field to the header>
        <release the current forward reference block>

    <Show forward reference block list empty now>                           
    <restore current IDS pointer from local storage>

OTHER THINGS TO KNOW:

    This function was new with V1.6, and is augmented for V1.9 to handle
    forward-reference processing for "DEPENDS ON = " clauses in addition
    to the original "IDENTIFIER=" clause processing.

*/

{
FRB     **fr_header;            /* ->> Head of Forward Reference List      */
FRB     *this_block;            /* -> Forward Reference block being proc.  */

FRB     *index;                 /* Index into list of IDENT attr-names     */
FRB     *to_kill_index;         /* --> FRB (String) to be released         */
IDS     *real_current;          /* -> Current IDS upon entry here          */
IDS     *sought_attrib;         /* -> Sought forward reference attribute   */
                                /*    IDS.                                 */
MC_STATUS status;               /* Status from Add-Relationship call       */


/* initialize pointer to value of forward-reference list head in bec */
fr_header = &bec->fwd_ref_list;

/* get the current IDS pointer and save it */
real_current = mirc_get_current();

/* while (forward-reference blocks remain to be processed) */
while (*fr_header != NULL) {

    this_block = *fr_header;    /* Grab a pointer directly to the block */

    /* switch (kind of forward reference block) */
    switch ( this_block->flavor) {

        case FR_IDENT:
            /* for (all strings blocks to be resolved in FR block) */
            for (index=this_block->fru.i.attr_names;
                 index != NULL;
                ) {

                /* set the current IDS pointer to f-ref block value */
                mirc_set_current(this_block->fru.i.current);

                /* if (attempt to find attribute of given name succeeded) */
                if ( (sought_attrib =
                           mirci_find_attribute(index->fru.s.string,
                                                MCC_K_PRT_IDENTIFIER,
                                                MIR_Cont_attrPartition))
                     != NULL ) {

                    /* if (attempt to add rel. "MIR_Indexed_By" succeeded) */
                    if ( (status = mirc_add_relationship(MIR_Indexed_By,
                                                         sought_attrib))
                        != MC_SUCCESS) {
                        char    msg[200];   /* Error message buffer                    */

                        /* issue "mirc - attempt to add relationship failed" */
                        sprintf(msg,
                             MP(mp136,"mirc - add of forward-reference rel. failed: \"%s\"\n"),
                              mirc_error(status));

                        /* set eflag TRUE */
                        warn(msg);
                        eflag = TRUE;
                        }
                    }
                else {
                    char    msg[200];   /* Error message buffer                    */
                    sprintf(msg,
                            MP(mp137,"forward reference to \"%s\" from line %d is unresolved"),
                            index->fru.s.string,
                            this_block->fru.i.lineno);

                    /* set eflag TRUE */
                    yyerror(msg);
                    eflag = TRUE;
                    }

                /* Step our index to the next forward-reference string */
                to_kill_index = index;
                index = index->next;

                /* release the string in the string block */
                free(to_kill_index->fru.s.string);

                /* release the string block */
                free(to_kill_index);
                }

            break;

        case FR_DEPENDS:
            mirci_resolve_DEPENDS(this_block);

            /* Blow off all (any) strings in this kind of F-R Block */
            for (index=this_block->fru.d.enum_list;
                 index != NULL;
                ) {

                /* Step our index to the next forward-reference string */
                to_kill_index = index;
                index = index->next;

                /* release the string in the string block */
                free(to_kill_index->fru.s.string);

                /* release the string block */
                free(to_kill_index);
                }
            break;

        default:
            fprintf(stderr,
                    MP(mp138,"mirc - internal compiler error: fwd-ref block value %d\n"),
                    this_block->flavor);
            yyerror(MP(mp139,"abort"));
        }

    /* copy the "next" field to the header */
    *fr_header = this_block->next;

    /* release the current forward reference block */
    free(this_block);

    }

/* Show forward reference block list empty now */
bec->fwd_ref_list = NULL;

/* restore current IDS pointer from local storage */
mirc_set_current(real_current);

}

/* mirci_resolve_DEPENDS - Perform Fwd-Ref Processing for DEPENDS ON clause */
/* mirci_resolve_DEPENDS - Perform Fwd-Ref Processing for DEPENDS ON clause */
/* mirci_resolve_DEPENDS - Perform Fwd-Ref Processing for DEPENDS ON clause */

static void
mirci_resolve_DEPENDS( d_o )

FRB     *d_o;           /*-> Forward-Reference Block for a DEPENDS ON clause */

/*
INPUTS:

    The forward-reference block for a DEPENDS ON clause is passed as input.

    Also the error flag "eflag" defined by the yacc parser code is
    referenced via "extern".

OUTPUTS:

    The function returns nothing explicitly, but on successful completion
    it will have fully processed (and set "eflag" TRUE if an error occurred)
    one "forward-reference" block for a single DEPENDS ON clause in the
    last MSL file seen by the yacc parser.

    On failure, error messages are issued, and "eflag" is set TRUE.


BIRD'S EYE VIEW:
    Context:
        The caller is "mirci_resolve_fwd_ref()" which has hit a "forward
        reference" block for a DEPENDS ON clause.  There is one F-R block
        for each and any DEPENDS ON clause.

    Purpose:
        This function performs all the processing needed to resolve the
        DEPENDS ON clause.  There is a lot that must be done, and it is
        moved out of "mirci_resolve_fwd_ref()" to make it easier to
        maintain.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set current object to DEPENDS ON Entity Class>

    if (attempt to find the characteristic attribute failed)
        <warn "No such Characteristic Attribute for DEPENDS ON line:x">
        <set error flag>
        <return>

    if (attempt to find Char. attribute via MIR_Variant_Sel object failed)
        if (attempt to insert MIR_Variant_Sel into Entity-Class object failed)
            <warn(error message)>
            <set eflag TRUE>
            <return>

    <make Depends-On block be the current object>

    if (add MIR_Depends_on rel to D.O. object w/char-attr as target failed)
        <warn(error message)>
        <set eflag TRUE>
        <return>

    if (add MIR_Depend_OP rel to D.O. object w/opcode as target failed)
        <warn(error message)>
        <set eflag TRUE>
        <return>

    <obtain pointer to datatype instance object for attribute>

    for (each enumeration string in the Depends-On FR Block)
        for (each MIR_Enum_Code/MIR_Enum_Text pair in datatype instance block)
            if (enumeration string matches MIR_Enum_Text target)
                if (attempt to add MIR_Enum_Code to Depends-On Object fails)
                    <warn(error message)>
                    <set eflag TRUE>
                if (attempt to add MIR_Enum_Text to Depends-On Object fails)
                    <warn(error message)>
                    <set eflag TRUE>
                <show enumeration match successful>
                <break>

        if (no match found for enumeration string)
            <warn(error message)>
            <set eflag TRUE>


OTHER THINGS TO KNOW:

    As examination of the code/pseudocode reveals, DEPENDS ON processing
    must accomplish several things:

    * We have to verify that the (depended-on) characteristic attribute 
      named in the DEPENDS ON clause actually exists.

    * We have to verify that the (depended-on) characteristic attribute
      appears as the target of the MIR_Variant_Sel relationship in the
      enclosing Entity Class. (If it does not appear then a MIR_Variant_Sel
      relationship must added to the Entity-Class object with a target of      
      the depended-on characteristic attribute).

    * We have to fill out the Depends-On MIR object for the "thing" that had
      the DEPENDS ON clause attached to it.  The Depends-On object has already
      been created by the yacc action code, we have to add the following
      entries to the relationship table:

          - MIR_Depends_on, pointing at the char. attribute being depended
            on

          - MIR_Depends_Op, pointing at the integer indicating the kind of
            "binary operator" that appeared in the DEPENDS ON clause ("=" or
            "IN SET")

          - a MIR_Enum_Code/MIR_Enum_Text relationship pair for each (VALID!)
            enumeration string named in the DEPENDS ON clause.

When we get done, the assemblage of MIR objects surrounding the MIR object "A"
that had the DEPENDS-ON clause attached to it should look like this:

    *--------------------*
    |  (MIR Object for   |
    |   Entity-Class     |
    |   containing "A")  |
    |                    |
    | MIR_Variant_Sel....|-----------------------------------*
    |                    |                                   |
    V. . . . . . . . . . V                                   V
                                                        *--> *----------------*
    *--------------------*                              |    | Characteristic |
    |   MIR Object "A"   |                              |    | Attribute MIR  |
    |--------------------|       'Depends-On Object'    |    |     Object     |
    |  MIR_Depends_on....|-----> *------------------*   |    V. . . . . . . . V
    |                    |       | MIR_Depends_on...|---*
    V. . . . . . . . . . V       | MIR_Depends_OP...|--> 'MCC_K_VAR_OP_EQUAL'
                                 |                  |  or'MCC_K_VAR_OP_IN_SET'
                                 |                  |
                                 | MIR_Enum_Code....|-->  <enumerated code>
                                 | MIR_Enum_Text....|--> "<enumerated text>"
                                 |       .          |
                                 |       .          |
                                 |       .          |
                                 V. . . . . . . . . V

Notes:

1) If there is more than one thing in the Entity-Class that 'depends-on' any
   particular Characteristic Attribute, there is still only one
   "MIR_Variant_Sel" relationship for that Characteristic Attribute.
   (In other words, the compiler checks to see if something already depends
   on that Char. Attrib. before entering a "MIR_Variant_Sel" relationship).

2) If MIR Object "A" happens to be an Entity-Class itself, then the
   Characteristic Attribute that it depends on is presumed to belong to
   "A"'s *containing* Entity-class, whereas otherwise the Characteristic
   Attribute is sought in the same Entity-Class as non-entity-class object "A"
   is defined within.

*/

{
IDS     *eclass;             /* --> IDS for the Entity Class                */
IDS     *char_attr_IDS;      /* --> IDS for Characteristic Attribute        */
IDS     *enum_inst_IDS=NULL; /* --> IDS for C-A's enumeration dataconstruct */
IDS     *enum_builtin=NULL;  /* --> Compiler's IDS for "Enumeration" Builtin*/
int     enum_code=(-1);      /* "Current" Enumerated Code                   */
char    *enum_text=NULL;     /* "Current" Enumerated Text                   */
FRB     *enum_val=NULL;      /* "Current" Candidate Enum Text name from D.O.*/
int     i;                   /* Handy-Dandy Loop Index                      */


/* set current object to DEPENDS ON Entity Class */
mirc_set_current(d_o->fru.d.eclass);
eclass = mirc_get_current();

/* if (attempt to find the characteristic attribute failed) */
if ((char_attr_IDS = mirci_find_attribute(d_o->fru.d.char_attr,
                                          MCC_K_PRT_CHAR,
                                          MIR_Cont_attrPartition)) == NULL) {
    fprintf(stderr,
            MP(mp140,"No such Characteristic Attribute for DEPENDS ON line %d\n"),
            d_o->fru.d.lineno);
    eflag = TRUE;
    return;
    }

/* if (attempt to find Characteristic attribute via MIR_Variant_Sel failed) */
if (mirc_find_obj_IDS(MIR_Variant_Sel, d_o->fru.d.char_attr) == NULL) {

    /* if (attempt to insert MIR_Variant_Sel into E-Class object failed) */
    if ( mirc_add_relationship(MIR_Variant_Sel, char_attr_IDS) != MC_SUCCESS) {
        yyerror(MP(mp142,"Unable to add MIR_Variant_Sel relationship"));
        return;
        }
    }

/* make Depends-On block be the current object */
mirc_set_current(d_o->fru.d.depends);

/* if (add MIR_Depends_on rel to D.O. object w/char-attr as target failed) */
if ( mirc_add_relationship(MIR_Depends_on, char_attr_IDS) != MC_SUCCESS) {
    yyerror(MP(mp144,"Unable to add MIR_List_Entry relationship to Depends-On Object"));
    return;
    }

/* if (add MIR_Depend_OP rel to D.O. object w/opcode as target failed) */
if ( mirc_add_rel_number(MIR_Depends_OP, d_o->fru.d.operator, MIR_SNUMBER)
    != MC_SUCCESS) {
    yyerror(MP(mp145,"Unable to add MIR_Depends_OP relationship to Depends-On Object"));
    return;
    }

/* obtain pointer to datatype instance object for attribute
|
| To do this, we have to walk the target of the MIR_Structured_As relationship
| in the Characteristic Attributes' object.  This should point to an instance
| (object) of an enumerated type (that is, a "built-UP" dataconstruct built
| from an underlying "builtin-IN" dataconstruct, specifically "enumeration").
|
| We have to make sure that the "built-IN" dataconstruct is "Enumeration", and
| leave an IDS pointer around to the instance of "built-UP" so that it may
| be scanned (in the paragraph following) for relationships "MIR_Enum_Text"
| and "MIR_Enum_Code".
*/

/* for (every entry in the Char-Attribute's object's relationship table) */
for (i=0; i < char_attr_IDS->idsu.nt.entry_count; i++) {

    /* if (relationship is MIR_Structured_As) */
    if (char_attr_IDS->idsu.nt.rel_table[i].rel_obj == 
                                    bec->map_rel_to_ids[MIR_Structured_As]) {
        enum_inst_IDS = char_attr_IDS->idsu.nt.rel_table[i].tar_obj;
        break;
        }
    }

/*
| Ok, we should have a valid pointer to an instance of an enumeration
| dataconstruct.  If we don't the compiler didn't compile the attribute
| properly.
*/
if (enum_inst_IDS == NULL) {
    yyerror(MP(mp146,"Characteristic Attribute for DEPENDS-ON has no datatype."));
    return;
    }

/*
| Now we have to be sure that the instance of the Characteristic Attribute
| dataconstruct really is "enumeration".  Scan for it's MIR_Structure_As
| relationship and make sure it matches the compiler's pointer to the IDS
|  built-in for "Enumeration".
*/

/* Get IDS pointer to compiler's representation of builtin "Enumeration" */
mirc_find_datatype("BIDT_ENUMERATION", &enum_builtin);

/* for (every entry in the dataconstruct's object's relationship table) */
for (i=0; i < enum_inst_IDS->idsu.nt.entry_count; i++) {

    /* if (relationship is MIR_Structured_As) */
    if (enum_inst_IDS->idsu.nt.rel_table[i].rel_obj == 
        bec->map_rel_to_ids[MIR_Structured_As]) {

        /* if (target of MIR_Structured_As is "built-IN" Enumeration */
        if (enum_inst_IDS->idsu.nt.rel_table[i].tar_obj == enum_builtin)
            break;
        }
    }

if (i >= enum_inst_IDS->idsu.nt.entry_count) {
    fprintf(stderr,
            MP(mp147,"Error at line %d: Characteristic Attribute is not of type Enumeration"),
            d_o->fru.d.lineno);
    eflag = TRUE;
    return;
    }

/*
| At this point, the enum_inst IDS points to the instance of an enumeration
| that is truly the dataconstruct used by the Characteristic Attribute, plus
| we know that it really is an enumeration.  Pairs of MIR_Enum_Code/
| MIR_Enum_Text relationships are paired in the table of this object.  We
| read these repeatedly looking for a match for each enumeration value
| specified in the DEPENDS ON clause.  For each match, the pair is
| (in essence) copied into the Depends-On MIR Object (which is still the
| "current" MIR object).
*/

/* for (each enumeration string in the Depends-On FR Block) */
for (enum_val = d_o->fru.d.enum_list;
     enum_val != NULL;
     enum_val = enum_val->next) {

    /* for (each MIR_Enum_Code/MIR_Enum_Text pair in datatype instance blk) */
    for (i=0; i < enum_inst_IDS->idsu.nt.entry_count; i++) {

        enum_text = "Non-Null Address";         /* Signals "Not Successful" */

        /* if (relationship object matches the specified MIR_Enum_Code) */
        if (enum_inst_IDS->idsu.nt.rel_table[i].rel_obj == 
            bec->map_rel_to_ids[MIR_Enum_Code]) {

            /* Copy and save code */
            enum_code = enum_inst_IDS->idsu.nt.rel_table[i].tar_obj->
                                                 idsu.t.t_v.snumber.value;
            continue;   /* Go on to the next relationship (should be Text) */
            }

        /* if (relationship object matches the specified MIR_Enum_Text) */
        if (enum_inst_IDS->idsu.nt.rel_table[i].rel_obj == 
            bec->map_rel_to_ids[MIR_Enum_Text]) {

            enum_text = enum_inst_IDS->idsu.nt.rel_table[i].tar_obj->
                                                 idsu.t.t_v.string.str;

            /* If the candidate matches a real enumerated value */
#ifndef VMS
            if (strcasecmp(enum_val->fru.s.string, enum_text) == 0) {
#else
            if (strlen(enum_val->fru.s.string) == strlen(enum_text)
                && caseless_str_equal(enum_val->fru.s.string,
                                      enum_text,
                                      strlen(enum_text))) {
#endif
                /* Add MIR_Enum_Code to Depends-On MIR Object */
                if ( mirc_add_rel_number(MIR_Enum_Code, enum_code, MIR_SNUMBER)
                    != MC_SUCCESS) {
                    yyerror(MP(mp148,"Unable to add MIR_Enum_Code rel. to Depends-On Object"));
                    return;
                    }

                /* Add MIR_Enum_Text to Depends-On MIR Object */
                if ( mirc_add_rel_string(MIR_Enum_Text, enum_text) 
                    != MC_SUCCESS) {
                    yyerror(MP(mp149,"Unable to add MIR_Enum_Text rel. to Depends-On Object"));
                    return;
                    }

                enum_text = NULL;       /* Signals "Successful" */
                break;
                }
            }
        }

    /* if (no match found for enumeration string) */
    if (enum_text != NULL) {
        fprintf(stderr,
                MP(mp150,"Error at line %d: Not a legal enumeration value: %s\n"),
                d_o->fru.d.lineno,
                enum_val->fru.s.string);
        eflag = TRUE;
        }
    }
}

/* mirci_find_attribute - Finds a attribute of a specific kind */
/* mirci_find_attribute - Finds a attribute of a specific kind */
/* mirci_find_attribute - Finds a attribute of a specific kind */

static
IDS *
mirci_find_attribute( a_name, partition_code, parent_rel )

char             *a_name;             /*--> Attribute's name as a string   */
int              partition_code;      /* Code for standard attr partition  */
mir_relationship parent_rel;          /* Relationship used from EClass     */

/*
INPUTS:

    The "current_object" is expected to be an entity class, and "a_name"
    is the string name of a desired attribute.

    "partition_code" is the code value of the kind of partition that the
    attribute should be found it.
    
    "parent_rel" is the relationship that should be sought in the Entity
    class to obtain the list to search for an attribute.  (Currently only
    makes sense for value "MIR_Cont_attrPartition").

OUTPUTS:

    The function returns a pointer to the IDS representing the attribute
    (if it was found) or NULL if it was not found.


BIRD'S EYE VIEW:
    Context:
        The caller is "mirci_resolve_fwd_ref()" or "mirci_resolve_DEPENDS()"
        both of which need to obtain IDS pointers to attributes of various
        sorts.

    Purpose:
        This function performs all the processing needed to look up
        a particular attribute within an entity class.


ACTION SYNOPSIS OR PSEUDOCODE:

    for (all entries in entity-class relationship table)
        if (relationship is "parent-relationship")
            <obtain target of parent-relationship>
            for (all entries in target's relationship table)
                if (relationship is "MIR_ID_Code")
                    if (target is equal to partition_code)
                        return(attempt to find object "MIR_List_Entry" of
                            name "a_name")

    <return NULL>

OTHER THINGS TO KNOW:

   This function is used internally in the back-end for the specific
   purpose of looking up attributes 'by name' that must be found in a
   specific partition.  This serves forward-reference resolution lookup
   needs.
*/

{
int     i,j;                    /* Handy-Dandy Loop Indices        */
IDS     *eclass_IDS;            /* Current Entity-Class Object     */
IDS     *attrgrp_IDS;           /* --> IDS for Attribute Group Obj */
IDS     *id_code_IDS;           /* --> Terminal for ID Code        */
IDS     *attrib_IDS;            /* --> Sought attribute IDS if fnd */


eclass_IDS = mirc_get_current();

/* for (all entries in entity-class relationship table) */
for (i = 0; i < eclass_IDS->idsu.nt.entry_count; i++) {

    /* if (relationship IDS is the "parent-relationship") */
    if (eclass_IDS->idsu.nt.rel_table[i].rel_obj
        == bec->map_rel_to_ids[parent_rel]) {

        /* obtain target of parent relationship */
        attrgrp_IDS = eclass_IDS->idsu.nt.rel_table[i].tar_obj;

        /* for (all entries in target's relationship table) */
        for (j = 0; j < attrgrp_IDS->idsu.nt.entry_count; j++) {

            /* if (relationship is "MIR_ID_Code" or "MIR_MCC_ID_Code") */
            if (   attrgrp_IDS->idsu.nt.rel_table[j].rel_obj ==
                   bec->map_rel_to_ids[MIR_ID_Code]
                || attrgrp_IDS->idsu.nt.rel_table[j].rel_obj ==
                   bec->map_rel_to_ids[MIR_MCC_ID_Code]) {

                id_code_IDS = attrgrp_IDS->idsu.nt.rel_table[j].tar_obj;

                /* if (target is equal to partition_code) */
                if (id_code_IDS->idsu.t.t_v.snumber.value == partition_code) {

                    /* return response from find object "MIR_List_Entry" of  */
                    /*   name "a_name"                                       */
                    mirc_set_current(attrgrp_IDS);  /* Thing to search */
                    attrib_IDS = mirc_find_obj_IDS(MIR_List_Entry, a_name);
                    mirc_set_current(eclass_IDS);
                    return(attrib_IDS);
                    }
                }   /* If right partition */
            }   /* if MIR_ID_code */
        }   /* for entries in attrGrp */
    }   /* if MIR_Cont_attrGroup */
    /* for entries in eclass */

return(NULL);   /* Couldn't find a thing! */
}

/* mirci_build_rel_list - Build MIR Compiler internal list of Relationships */
/* mirci_build_rel_list - Build MIR Compiler internal list of Relationships */
/* mirci_build_rel_list - Build MIR Compiler internal list of Relationships */

static void
mirci_build_rel_list( )

/*
INPUTS:

    No explicit inputs.

OUTPUTS:

    On success, the function has created IDS for each relationship used by the
    MIR compiler and placed an entry onto the compiler's internal list of these
    relationships (the array "map_rel_to_ids[]" maintained in the back-end
    context block).

    On failure, the function prints a message and causes the process to
    exit.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler busy in it's
        initialization: creating the Non-Terminals for the basic MIR
        relationships and the relationships used for the SMI we're
        compiling into (DNA in V1.0).

    Purpose:
        This function executes a call (for each relationship to be used)
        to the backend function responsible for setting up ONE relationship.

ACTION SYNOPSIS OR PSEUDOCODE:

    See the Code.

OTHER THINGS TO KNOW:

|-READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS-
|-READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS-
|
|   The order that these calls are made here must NOT change!  This
|   is due to the fact that the order of appearance of these calls for
|   each relationship named is the order in which the Non-Terminals appear
|   in the MIR Relationship Partition (as well as the order in which ISO
|   Object ID's are generated for these relationships).
|
|   The logic in "mir_internal.c" expects to be able to deduce the internal
|   compiler code (of type "mir_relationship") for each MIR Relationship Object
|   based on it's order in the partition (where code value of "1" is the same
|   as "MIR_ID_Code" (of type "mir_relationship"), and "0" is
|   "MIR_Relationship_Name", etc.  NOTE:  This is not the same value as
|   the "synonym" (whose values start at "1"), although the principle of
|   determining the code is the same!  Both the Code number (value of type
|   "mir_relationship") and synonym (integer values starting with "1") are
|   both determined by the order-of-apppearance of the MIR object in the
|   partition.
|
|   Once the OIDS are published, they cannot be changed!  (There should be 
|   no need to publish them however, but we can't go changing them willy-nilly.
|
|   What's more, if they were to change, then the MIR binary database file
|   generated by the compiler will not be easily "merge-able" with the MIR
|   binary database file created by a MIR compiler that has a different list.
|
|       RULE 1) Remove no entries.
|       RULE 2) Add new entries at the end **ONLY**!
|
|-READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS-
|-READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS -- READ THIS-
    
*/

{
int     arc;

arc = 0;      /* The arc value that goes after the architected "17(.1)" in */
              /*  the ISO Object ID for these relationships                */

/*
|  MIR "Generic" Relationships for use in describing all SMIs 
|  (NOTE: MIR_Relationship_Name *MUST* BE FIRST, w/arc=0, ADD NEW ENTRIES
|         ONLY AT THE **END**)
*/
mirci_register_rel("MIR_Relationship_Name",        MIR_Relationship_Name, arc++);   /* The "root" relationship for MIR    */

mirci_register_rel("MIR_ID_Code",                  MIR_ID_Code,           arc++);   /* <thing> has a code of ...          */
mirci_register_rel("MIR_MCC_ID_Code",              MIR_MCC_ID_Code,       arc++);   /* <thing> has a MCC code of ...      */
mirci_register_rel("MIR_Text_Name",                MIR_Text_Name,         arc++);   /* <thing> as a stringname of ...     */
mirci_register_rel("MIR_Special",                  MIR_Special,           arc++);   /* <thing> has a text value of ...    */
mirci_register_rel("MIR_Contains",                 MIR_Contains,          arc++);   /* <thing> hierarchically contains... */
mirci_register_rel("MIR_Contained_By",             MIR_Contained_By,      arc++);   /* <thing> is contained by ...        */
mirci_register_rel("MIR_Structured_As",            MIR_Structured_As,     arc++);   /* <thing> is of "datatype" ...       */
mirci_register_rel("MIR_Indexed_By",               MIR_Indexed_By,        arc++);   /* <thing> is indexed by "other MIR object" ... */


/* DataConstruct Non-Terminals may contain these relationships...*/
mirci_register_rel("MIR_DC_Found_In_SMI",          MIR_DC_Found_In_SMI,   arc++);   /* <DataConstruct> found in SMI (code)*/
mirci_register_rel("MIR_DC_ASN1_Class",            MIR_DC_ASN1_Class,     arc++);   /* <DataConstruct> has ASN1 Class...  */
mirci_register_rel("MIR_DC_ASN1_Tag",              MIR_DC_ASN1_Tag,       arc++);   /* <DataConstruct> has ASN1 Tag...    */
mirci_register_rel("MIR_DC_SMI_Name",              MIR_DC_SMI_Name,       arc++);   /* <DataConstruct>'s name in SMI is...*/
mirci_register_rel("MIR_DC_SMI_Code",              MIR_DC_SMI_Code,       arc++);   /* <DataConstruct>'s code in SMI is...*/
mirci_register_rel("MIR_DC_SMI_Template",          MIR_DC_SMI_Template,   arc++);   /* <DataConstruct> may be further     */

mirci_register_rel("MIR_DC_SMI_OTTemplate",        MIR_DC_SMI_OTTemplate, arc++);   /* <DataConstruct> is qualified by a  */
mirci_register_rel("MIR_DC_MCC_DT_Size",           MIR_DC_MCC_DT_Size,    arc++);   /* <DataConstruct> has MCC "Size" of  */
mirci_register_rel("MIR_DC_NCL_CMIP_Code",         MIR_DC_NCL_CMIP_Code,  arc++);   /* <DataConstruct> has MCC "Size" of  */

/* MIR Relationships for ancillary information for */
/* MIR datatypes "Enumeration" & "Record".         */
mirci_register_rel("MIR_Enum_Code",                MIR_Enum_Code,    arc++);    /* "<enum-datatype> has a code of ..." */
mirci_register_rel("MIR_Enum_Text",                MIR_Enum_Text,    arc++);    /* "<enum-datatype>has text of ..."    */
mirci_register_rel("MIR_Field_Name",               MIR_Field_Name,   arc++);    /* "<rec-datatype>has field name..."   */
mirci_register_rel("MIR_Field_Code",               MIR_Field_Code,   arc++);    /* "<rec-datatype>has field code..."   */
mirci_register_rel("MIR_Case_Code",                MIR_Case_Code,    arc++);    /* "<case-group-instance> selected by  */
                                                                                /*    fixed-field value ..."           */

/* DNA Relationships that map to DNU "descriptors"/MCC Definitions */
mirci_register_rel("MIR_Access",                   MIR_Access,         arc++);
mirci_register_rel("MIR_valueDefault",             MIR_valueDefault,   arc++);
mirci_register_rel("MIR_Display",                  MIR_Display,        arc++);
mirci_register_rel("MIR_Min_Int_Val",              MIR_Min_Int_Val,    arc++);
mirci_register_rel("MIR_Max_Int_Val",              MIR_Max_Int_Val,    arc++);
mirci_register_rel("MIR_DNS_Ident",                MIR_DNS_Ident,      arc++);
mirci_register_rel("MIR_Units",                    MIR_Units,          arc++);
mirci_register_rel("MIR_Dynamic",                  MIR_Dynamic,        arc++);
mirci_register_rel("MIR_Required",                 MIR_Required,       arc++);
mirci_register_rel("MIR_Description",              MIR_Description,    arc++);
mirci_register_rel("MIR_Directive_type",           MIR_Directive_type, arc++);
mirci_register_rel("MIR_Depends_on",               MIR_Depends_on,     arc++);
mirci_register_rel("MIR_Depends_OP",               MIR_Depends_OP,     arc++);
mirci_register_rel("MIR_Echo",                     MIR_Echo,           arc++);
mirci_register_rel("MIR_Predictable",              MIR_Predictable,    arc++);
mirci_register_rel("MIR_Variant_Sel",              MIR_Variant_Sel,    arc++);
mirci_register_rel("MIR_Counted_As",               MIR_Counted_As,     arc++);


/* Other DNA Relationships */
mirci_register_rel("MIR_Category",                 MIR_Category,       arc++);
mirci_register_rel("MIR_Symbol",                   MIR_Symbol,         arc++);
mirci_register_rel("MIR_Symbol_prefix",            MIR_Symbol_prefix,  arc++);


/* DNA Containment Hierarchy Relationships */
mirci_register_rel("MIR_Cont_entityClass",     MIR_Cont_entityClass,    arc++);
mirci_register_rel("MIR_Cont_attribute",       MIR_Cont_attribute,      arc++);
mirci_register_rel("MIR_Cont_attrPartition",   MIR_Cont_attrPartition,  arc++);
mirci_register_rel("MIR_Cont_attrGroup",       MIR_Cont_attrGroup,      arc++);
mirci_register_rel("MIR_Cont_eventPartition",  MIR_Cont_eventPartition, arc++);
mirci_register_rel("MIR_Cont_eventGroup",      MIR_Cont_eventGroup,     arc++);
mirci_register_rel("MIR_Cont_event",           MIR_Cont_event,          arc++);
mirci_register_rel("MIR_Cont_directive",       MIR_Cont_directive,      arc++);
mirci_register_rel("MIR_Cont_request",         MIR_Cont_request,        arc++);
mirci_register_rel("MIR_Cont_request_argument",MIR_Cont_request_argument,arc++);
mirci_register_rel("MIR_Cont_response",        MIR_Cont_response,       arc++);
mirci_register_rel("MIR_Cont_exception",       MIR_Cont_exception,      arc++);
mirci_register_rel("MIR_Cont_argument",        MIR_Cont_argument,       arc++);
mirci_register_rel("MIR_List_Entry",           MIR_List_Entry,          arc++);
mirci_register_rel("MIR_List_Type",            MIR_List_Type,           arc++);

/* ^^                     ^^ */                                       
/* || NEW ENTRIES GO HERE || */
}

/* mirci_register_rel - Register a MIR Relationship Non-Terminal */
/* mirci_register_rel - Register a MIR Relationship Non-Terminal */
/* mirci_register_rel - Register a MIR Relationship Non-Terminal */

static void
mirci_register_rel( name_str, rel_code, arc_value )

char                *name_str;  /* The Name of the Relationship             */
mir_relationship    rel_code;   /* The internal compiler code for this rel. */
int                 arc_value;  /* Value to be used after "17" in Object ID */

/*
INPUTS:

    "name_str" is an ASCIZ string containing the name of the new relationship

    "rel_code" is the internal MIR Compiler code for this relationship.

    "arc_value" is a number to be used in constructing the Object ID
    that this Relationship Non-Terminal will be registered under.

    An implied input to this function are the static arrays found in
    the back-end context block:

    "map_rel_to_ids[]" which is used by the back-end functions to obtain the
    address of an IDS (that corresponds to a MIR Relationship) given the
    internal MIR Compiler code for that relationship.  This function
    LOADS this array.

    "map_relstr_to_ids[]" which is used by the back-end functions to obtain the
    address of an IDS (that corresponds to a MIR Relationship) given the
    internal MIR Compiler Name (as a string) for that relationship. 
    This function LOADS this array.


OUTPUTS:

    On success, the function loads a pointer to the newly created
    non-terminal into "map_rel_to_ids[rel_code]".

    The newly created non-terminal is registered both in the MIR Index
    (by derived OID).

    On failure, the function prints a message and causes the process to
    exit.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler busy in it's
        initialization: creating the Non-Terminals for the basic MIR
        relationships and the relationships used for the SMI we're
        compiling into (DNA in V1.0).

    Purpose:
        This function performs the actual storage allocation and initializing
        of each of the new relationship non-terminal's relationship table so
        that it contains one entry:  MIR_Relationship_Name.

ACTION SYNOPSIS OR PSEUDOCODE:

    See the Code.

OTHER THINGS TO KNOW:

    This function does the following things:

    * Creates an IDS for a MIR Relationship Object,
    * Loads the "Internal Synonym" (rel_code value) into the IDS (used
      by the "mir_remove.c" code.
    * Puts the name of that relationship into the relationship table for
      this new IDS using the relationship "MIR_Relationship_Name".
    * Records the pointer to the newly created IDS in the internal MIR
      Compiler array "map_rel_to_ids[]" using the internal MIR Compiler
      code (rel_code) supplied in the call.  This makes this relationship
      "available" by way of the internal MIR Compiler code to the other
      backend routines.
    * Records a pointer to the name of the created IDS in the internal MIR
      Compiler array "map_relstr_to_ids[]" using the internal MIR Compiler
      name supplied in the call.  This makes this relationship
      "available" by way of the internal MIR Compiler Name to the other
      backend routines.

    Special processing in this code handles the creation of the very
    first "relationship" Non-Terminal, the one for MIR_Relationship_Name.
    (It defines itself).

    This function assumes that the first time it is called, it is creating
    the IDS (and thereby defining) the MIR relationship
    "MIR_Relationship_Name".  It is this relationship that is used in the
    relationship table of all the other MIR Relationship objects to specify
    the ASCII (string) "name" of the relationship.  Consequently the
    "MIR_Relationship_Name" object "defines itself".

    With V1.95, the kind of Non-Terminal IDS created is specific to use
    in representing a MIR relationship.  This is so all the Non-Terminals
    used to represent MIR relationships can be written into their own
    partition during the final compiler passes performed by functions in
    "mir_external.c".

    With V1.98, the "internal synonym" is loaded into the IDS so that
    the "mir_remove.c" code can make quick work of dispatching on the
    MIR Relationship when deleting table entries.

*/

{
IDS *new_rel_nt;     /* -> Newly created Relationship Non-Terminal  */
IDS *rel_name_t;     /* -> Terminal containing name of relationship */
int i;               /* General index                               */

object_id        oid;        /* Object Identifier        */
unsigned int     value[50];  /* Storage for OID's arcs   */

/* -----------------------Permanent Storage---------------------------*/
/* Here we record the IDS address of the MIR_Relationship_Name relationship */
static IDS *mir_rel_name=NULL;
/* ---------------------End Permanent Storage-------------------------*/

/* Try to create the basic outline of the new relationship non-terminal */
if ( (new_rel_nt = I_Create_IDS(I_NT_OBJECT_Rel, 1)) == NULL) {
    fprintf(stderr, MP(mp104,"mirc - Out of Memory during Initialization.\n"));
    exit(BAD_EXIT);
    }

/* Load the "Internal" (zero-origined) Synonym into IDS */
new_rel_nt->idsu.nt.ntu.synonym = rel_code;

/* Now create a Terminal object containing the string supplied as the name */
/* of the new relationship non-terminal                                    */
if ( (rel_name_t = I_Create_IDS_STRING(name_str)) == NULL ) {
    fprintf(stderr, MP(mp104,"mirc - Out of Memory during Initialization.\n"));
    exit(BAD_EXIT);
    }

/* Ok, all we need to do now is fill in one entry in the relationship table */
/* with relationship MIR_Relationship_Name (either passed or created here)  */
if ( (I_Insert_Rel_Entry(
            new_rel_nt,                      /* The new NT being filled */
            ((mir_rel_name == NULL) ?        /* (Defining itself?)      */
                new_rel_nt : mir_rel_name),  /*  YES : NO               */
            rel_name_t  )) != MC_SUCCESS) {
    fprintf(stderr, MP(mp104,"mirc - Out of Memory during Initialization.\n"));
    exit(BAD_EXIT);
    }

/* The first time we're called, we're registering MIR_Relationship_Name, */
/* so record it.                                                         */
if (mir_rel_name == NULL) {
    mir_rel_name = new_rel_nt;
    }

/* Register the Relationship under the compiler internal code passed in */
/* the function call:                                                   */
bec->map_rel_to_ids[rel_code] = new_rel_nt;

/* Register the Relationship under the compiler internal name passed in */
/* the function call and recorded in the IDS:                           */
bec->map_relstr_to_ids[rel_code] = rel_name_t->idsu.t.t_v.string.str;

/* Register the Relationship in the Index */
/* Make sure our "object id" is initialized */
oid.count = 0;
oid.value = value;

/* Load the DSM prefix.  We use the latest and the greatest. */
i = 0;
value[i++] = 1;         /* ISO                                               */
value[i++] = 3;         /* Identified Organization                           */
value[i++] = 12;        /* ICD-European Computer Manufacturer's Assocation   */
value[i++] = 2;         /* Member Organization                               */
value[i++] = 1011;      /* Digital Equipment Corp.                           */
value[i++] = 2;         /* Enterprise Management Architecture                */
value[i++] = 17;        /* EMA Properties                                    */
value[i++] = 1;         /* MIR - V2.0 Properties                             */

value[i++] = arc_value;
oid.count = i;

if ( I_Register_OID(&oid, OID_DNA, new_rel_nt) != MC_SUCCESS) {
    fprintf(stderr, MP(mp104,"mirc - Out of Memory during Initialization.\n"));
    exit(BAD_EXIT);
    }

return;

}

/* mirc_keyword - Register or Verify a Keyword string */
/* mirc_keyword - Register or Verify a Keyword string */
/* mirc_keyword - Register or Verify a Keyword string */

BOOL
mirc_keyword( opcode, kw_str)

KW_OPCODE       opcode;         /* Register or Verify               */
char            *kw_str;        /* --> String to Register or Verify */

/*
INPUTS:

    "opcode" indicates whether to register (IT_IS_A_KEYWORD) or
    verify (IS_IT_A_KEYWORD).

    "kw_str" points at the string to be registered or verified.


OUTPUTS:

    If "opcode" is "IT_IS_A_KEYWORD", on success the function returns TRUE.
    If the function runs out of memory attempting to register the value,
    it issues a message and then exits.

    If "opcode" is "IS_IT_A_KEYWORD", the function returns TRUE if the
    "kw_str" is on the list of valid keyword names, otherwise FALSE.


BIRD'S EYE VIEW:
    Context:
        The caller is the front-end of the compiler busy processing a
        keyword in the builtin-types file (IT_IS_A_KEYWORD) or in an MSL
        file near an INCLUDE statement (IS_IT_A_KEYWORD).

    Purpose:
        IT_IS_A_KEYWORD:  Function records the string in the back-end context
        for later reference when the opcode is. . . .

        IS_IT_A_KEYWORD:  Function checks the string to see if it is on the
        list of valid keywords for an INCLUDE statement.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (opcode is "IS_IT_A_KEYWORD")
        for (every entry on the legal keyword list)
            if (case-insensitive comparison MATCHES)
                <return TRUE>
        <return FALSE>
    else if (opcode is "IS_IT_SELECTED")
        for (every entry on the selected-keyword list)
            if (case-insensitive comparison MATCHES)
                <return TRUE>
        <return FALSE>
    else
        if (attempt to allocate an IDX failed)
            <issue "Out of Memory" error message>
            <exit>
        <add the IDX to the top of the keyword list in backend context>
        if (attempt to allocate space for string storage failed)
            <issue "Out of Memory" error message>
            <exit>
        <copy keyword string to allocated storage>
        <return TRUE>


OTHER THINGS TO KNOW:

    This function supports the current scheme whereby the KEYWORD list is
    embedded in the "builtin_types.dat" file.  Maybe this isn't a good idea
    and it should be in a different file.  Either way, this function is of
    use.
*/

{
IDX     *scanner;       /* Pointer to next entry on the keyword list */


/* if (opcode is "IS_IT_A_KEYWORD") */
if (opcode == IS_IT_A_KEYWORD) {

    /* for (every entry on the keyword list) */
    for (scanner = bec->legal_keyword_list;
         scanner != NULL;
         scanner=scanner->next) {

        /* if (case-insensitive comparison MATCHES) */
#ifndef VMS
        if (strcasecmp(kw_str, scanner->name) == 0) {
#else
        if (strlen(kw_str) == strlen(scanner->name)
            && caseless_str_equal(kw_str, scanner->name, strlen(kw_str))) {
#endif
            return(TRUE);
            }
        }

    return(FALSE);
    }

/* else if (opcode is "IS_IT_SELECTED") */
else if (opcode == IS_IT_SELECTED) {

    /* for (every entry on the selected keyword list) */
    for (scanner = bec->selected_keyword_list;
         scanner != NULL;
         scanner=scanner->next) {

        /* if (case-insensitive comparison MATCHES) */
#ifndef VMS
        if (strcasecmp(kw_str, scanner->name) == 0) {
#else
        if (strlen(kw_str) == strlen(scanner->name)
            && caseless_str_equal(kw_str, scanner->name, strlen(kw_str))) {
#endif
            return(TRUE);
            }
        }

    return(FALSE);
    }

else {  /* Register the kw_str */

    /* if (attempt to allocate an IDX failed) */
    if ((scanner = (IDX *) malloc(sizeof(IDX))) == NULL) {
        /* issue "Out of Memory" error message */
        fprintf(stderr, "mirc - Out of Memory during initialization.\n");
        exit(BAD_EXIT);
        }

    /* add the IDX to the top of the legal keyword list in backend context */
    scanner->next = bec->legal_keyword_list;
    bec->legal_keyword_list = scanner;

    /* if (attempt to allocate space for string storage failed) */
    if ((scanner->name = (char *) malloc(strlen(kw_str) + 1)) == NULL) {
        /* issue "Out of Memory" error message */
        fprintf(stderr, MP(mp104,"mirc - Out of Memory during Initialization.\n"));
        exit(BAD_EXIT);
        }

    /* copy keyword string to allocated storage */
    strcpy(scanner->name, kw_str);

    return(TRUE);
    }
}

/*
 *
 * Copyright (C) 1989 by
 * Digital Equipment Corporation, Maynard, Mass.
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
 * Networks & Communications Software Engineering
 *
 */
/* static char *sccsid = "@(#)caseless_strcmp.c	5.3	12/13/89"; */

/*
 * case-insensitive string comparison function 
 * note that it is assumed that caller either has compared string
 * lengths already (for exact equality test) or is doing
 * partial wildcard matching.
 *
 * inputs:
 *	pStr1 - address of null-delimited string
 *	pStr2 - address of null-delimited string
 *	len - length in bytes of both strings
 *
 * returns:
 *	TRUE - one string is a prefix for or equal to the other string
 *	FALSE - strings don't match
 *
 * by B.M.England
 */

#define _cvt_upper( character ) \
	(isalpha(character) ? \
		_toupper(character) : \
		character )

int
caseless_str_equal( pStr1, pStr2, len )

char *pStr1;
char *pStr2;
int len;

{
register char c1, c2, *p1, *p2, result, index;

p1 = pStr1;
p2 = pStr2;
result = TRUE;
for (index=len; index>0; index--) /* for each of len chars in strings */
	{
	c1 = *p1++;
	if (islower(c1))
		c1 = _toupper(c1);
	c2 = *p2++;
	if (islower(c2))
		c2 = _toupper(c2);
	result = (c1 == c2);
	if (!result)
		break;
	};

return (result);
}
