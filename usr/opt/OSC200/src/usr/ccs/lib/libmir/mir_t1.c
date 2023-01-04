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
static char *rcsid = "@(#)$RCSfile: mir_t1.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:17:28 $";
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
 * Module MIR_T1.C
 *      Contains one or more Tier 1 Interface Functions required by a user of
 *      the MIR to obtain information from the MIR via the Tier 0 Functions.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   December 1990
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
 *       to query the MIR for network management information.  These functions
 *       make use of the MIR Tier 0 functions.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *      V0.0    December 1990           D. D. Burns
 *      V1.0    February 1991           Compiler V1.0
 *      V1.1    June 1991               Add mir_copy_context() for OSI PE
 *                                        requirement, modify lookup_dc
 *                                        to allow builtin datatypes w/OIDs
 *                                        to be properly lookup-ed.
 *      V1.7    October 1991            Removed MS0_FIND_LONGEST return code
 *      V1.9x   October 1992            Internationalization, spt for
 *                                        returning ASN.1 Universal Tag, fixed
 *                                        bug in mir_lookup_dataconstruct()
 *                                        
 *      V2.00   December 1992           Add support for reading-out variant
 *                                      RECORD case code values.
 */
/*
Module Overview:

This module contains all the Tier 1 functions available to a user of the MIR.
These functions form the "Tier 1" interface on top of the "Tier 0" interface
functions that provide access to the MIR. 

Tier 0 function sources are found in another source module.

MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name        Synopsis
mir_get_smi          Returns an indication as to which SMI an object in the
                     MIR was defined by.  (Objects with "values" only).

mir_lookup_data_construct
                    Looks up and returns the descriptive information about
                    the data-construct ("datatype") of any object in the MIR
                    that "has-a-value" (such as attribute, argument etc).
                    If it is a "built-up" data-construct (ie "constructed"),
                    a context block is returned to allow fetching deeper
                    nested descriptions of the underlying data-constructs.

mir_eval_data_construct
                    Uses context block produced by mir_lookup_data_construct()
                    to fetch (repeatedly) the next level of description of a
                    data-construct until a description of a built-in atomic
                    or template is found.  A context block is generated when
                    a built-in template is found.

mir_eval_builtin_template
                    Uses context block generated by mir_eval_data_construct()
                    to allow the fetching of the details of the instance of
                    the data-construct described by the template.

mir_copy_context    Creates a copy of a context block created by
                    mir_eval_data_construct() or mir_eval_builtin_template()
                    to allow "ease-of-coding" in recursive situations.

mir_dissolve_context
                    Releases the storage created when the functions above
                    create context blocks.


INTERNAL FUNCTIONS:
load_local_mandles  Loads the module's local mandle pointers to point to
                    the relationship mandles used by the functions in
                    this module.

load_ident          Loads a data-construct "identification block" with
                    all the essential poop on that can be found about
                    that data-construct.
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <string.h>
#ifndef VMS
#include <malloc.h>
#endif

#include "mir.h"

/*         MODULE LOCAL MANDLES          */
/* (For use by functions in this module) */

/* Pointer to the mandle for relationship "MIR_Structured_As"   */
static mandle *m_mir_structured_as=NULL;

/* Pointer to the mandle for relationship "MIR_Text_Name"   */
static mandle *m_mir_text_name=NULL;

/* Pointer to the mandle for relationship "MIR_ID_Code"   */
static mandle *m_mir_id_code=NULL;

/* Pointer to the mandle for "MIR_DC_Found_In_SMI"   */
static mandle *m_mir_dc_found_in_smi=NULL;

/* Pointer to the mandle for "MIR_DC_ASN1_Class"   */
static mandle *m_mir_dc_asn1_class=NULL;

/* Pointer to the mandle for "MIR_DC_ASN1_Tag"   */
static mandle *m_mir_dc_asn1_tag=NULL;

/* Pointer to the mandle for "MIR_DC_SMI_Name"   */
static mandle *m_mir_dc_smi_name=NULL;

/* Pointer to the mandle for "MIR_DC_SMI_Code"   */
static mandle *m_mir_dc_smi_code=NULL;

/* Pointer to the mandle for "MIR_DC_SMI_Template"   */
static mandle *m_mir_dc_smi_template=NULL;

/* Pointer to the mandle for "MIR_DC_SMI_OTTemplate"   */
static mandle *m_mir_dc_smi_ottemplate=NULL;

/* Pointer to the mandle for "MIR_DC_MCC_DT_Size"   */
static mandle *m_mir_dc_mcc_dt_size=NULL;

/* Pointer to the mandle for "MIR_DC_NCL_CMIP_Code"   */
static mandle *m_mir_dc_ncl_cmip_code=NULL;


/*
| MTHREAD - Multiple-Threaded Environment
|
| NOTE WELL:  This mandle is used as a "scratch" mandle by Tier 1 functions
|             in this module.  In a fully threaded environment, the code that
|             uses this mandle should either be re-coded to use a mandle
|             on the stack (explicitly freeing it before exit) OR a mutex
|             must be established to protect this mandle.
|
|             (In a no-threads or single-threaded environment, this mandle
|              was made module-local and "re-used" simply to save the overhead
|              of freeing it explicitly and then reallocating it again on
|              re-entry to functions in this module).
*/
/* Pointer to a "scratch" mandle for use by any function */
static mandle *m_scanner=NULL;

/* Pointer to the mandle class for all mandles in module */
static mandle_class *local_class=NULL;



/* load_local_mandles - Load Modules Local Mandle Pointers */
/* load_local_mandles - Load Modules Local Mandle Pointers */
/* load_local_mandles - Load Modules Local Mandle Pointers */

static mir_status
load_local_mandles()

/*
INPUTS:

    The implicit inputs are the static local pointers to mandles listed
    at the beginning of this module.

OUTPUTS:

The function returns one of:

MS_SUCCESS - The local mandle pointers have been setup.

MS1_INIT_FAIL - Not all of the mandles were successfully found.

Additionally, this function can return all the error codes that the following
functions may return:

    mir_get_rel_mandles() - Performs the actual lookups

BIRD'S EYE VIEW:
    Context:
        The caller is any function in this module, to set up relationship
        mandle pointers that may be needed.

    Purpose:
        This function accesses the MIR via the special Tier 0 function
        "mir_get_rel_mandles()" to take care of the details of getting mandle
        pointers for relationship object needed by any function in this module.


ACTION SYNOPSIS OR PSEUDOCODE:

    <set up mandle/name pair array for all desired mandles>
    if (attempt to get mandles succeeded)
        <copy mandles to module-local storage>
        <return MS_SUCCESS>
    else if (status was MS_FAILURE)
        <return MS1_INIT_FAIL>
    else
        <return status>


OTHER THINGS TO KNOW:

    This function uses the standard Tier 0 function to actually do the
    fetching of the mandles, and then stores them locally.
    
*/

/*
|  These symbols make reference to entries in "pair_list[]" more mnemonic
*/
#define MANDLE_COUNT    12

#define M_STRUCT_AS             0
#define M_FND_IN_SMI            1
#define M_TEXT_NAME             2
#define M_ID_CODE               3
#define M_ASN1_TAG              4
#define M_ASN1_CLASS            5
#define M_SMI_NAME              6
#define M_SMI_CODE              7
#define M_SMI_TEMPLATE          8
#define M_SMI_OTTEMPLATE        9
#define M_MCC_SIZE              10
#define M_NCL_CMIP_CODE         11
{
rel_pair   pair_list[MANDLE_COUNT];      /* Array for mir_get_rel_mandles() */
mir_status status;                       /* Status from mir_get_rel_mandles */


/* set up mandle/name pair array for all desired mandles */
pair_list[M_STRUCT_AS].m = NULL;
pair_list[M_STRUCT_AS].name = "MIR_Structured_As";

pair_list[M_FND_IN_SMI].m = NULL;
pair_list[M_FND_IN_SMI].name = "MIR_DC_Found_In_SMI";

pair_list[M_TEXT_NAME].m = NULL;
pair_list[M_TEXT_NAME].name = "MIR_Text_Name";

pair_list[M_ID_CODE].m = NULL;
pair_list[M_ID_CODE].name = "MIR_ID_Code";

pair_list[M_ASN1_TAG].m = NULL;
pair_list[M_ASN1_TAG].name = "MIR_DC_ASN1_Tag";

pair_list[M_ASN1_CLASS].m = NULL;
pair_list[M_ASN1_CLASS].name = "MIR_DC_ASN1_Class";

pair_list[M_SMI_NAME].m = NULL;
pair_list[M_SMI_NAME].name = "MIR_DC_SMI_Name";

pair_list[M_SMI_CODE].m = NULL;
pair_list[M_SMI_CODE].name = "MIR_DC_SMI_Code";

pair_list[M_SMI_TEMPLATE].m = NULL;
pair_list[M_SMI_TEMPLATE].name = "MIR_DC_SMI_Template";

pair_list[M_SMI_OTTEMPLATE].m = NULL;
pair_list[M_SMI_OTTEMPLATE].name = "MIR_DC_SMI_OTTemplate";

pair_list[M_MCC_SIZE].m = NULL;
pair_list[M_MCC_SIZE].name = "MIR_DC_MCC_DT_Size";

pair_list[M_NCL_CMIP_CODE].m = NULL;
pair_list[M_NCL_CMIP_CODE].name = "MIR_DC_NCL_CMIP_Code";

/* if (attempt to get mandles succeeded) */
status = mir_get_rel_mandles(MANDLE_COUNT, pair_list, &local_class);
if ( status == MS_SUCCESS) {
    /* copy mandles to module-local storage
    |
    |  (We do this to meet the protocol established by an earlier version
    |   of the code in this module.  A more suitable approach when writing
    |   new code is to simply define macros that evaluate to references
    |   to explicit entries in a static pair_list[] array.)
    */
    m_mir_structured_as = pair_list[M_STRUCT_AS].m;
    m_mir_dc_found_in_smi = pair_list[M_FND_IN_SMI].m;
    m_mir_text_name = pair_list[M_TEXT_NAME].m;
    m_mir_id_code = pair_list[M_ID_CODE].m;
    m_mir_dc_asn1_tag = pair_list[M_ASN1_TAG].m;
    m_mir_dc_asn1_class = pair_list[M_ASN1_CLASS].m;
    m_mir_dc_smi_name = pair_list[M_SMI_NAME].m;
    m_mir_dc_smi_code = pair_list[M_SMI_CODE].m;
    m_mir_dc_smi_template = pair_list[M_SMI_TEMPLATE].m;
    m_mir_dc_smi_ottemplate = pair_list[M_SMI_OTTEMPLATE].m;
    m_mir_dc_mcc_dt_size = pair_list[M_MCC_SIZE].m;
    m_mir_dc_ncl_cmip_code = pair_list[M_NCL_CMIP_CODE].m;

    return (MS_SUCCESS);
    }
else if (status == MS_FAILURE) {
    return (MS1_INIT_FAIL);
    }

return (status);

}

/* mir_get_smi - Get an Object's Structure of Management Information ID */
/* mir_get_smi - Get an Object's Structure of Management Information ID */
/* mir_get_smi - Get an Object's Structure of Management Information ID */

mir_status mir_get_smi(oid, smi)

object_id       *oid;   /* -> Object Id of object whose SMI is desired */
mir_smi_code    *smi;   /* -> Place to return smi code                 */

/*
INPUTS:

    "oid" is the address of an object identifier that indicates the MIR
    object whose SMI is desired.

    "smi" is the address of a cell to receive an indicator as to what
    SMI the object is defined in.

OUTPUTS:

The function returns one of:

MS_SUCCESS - The "smi" argument has been successfully set to indicate
    which SMI the object is defined in.

MS1_NOT_A_VALUED_OBJECT - The specified object was found in the MIR, however
    it does not define something with a value, consequently the SMI which
    defines the object cannot be returned (because the indicator as to which
    SMI it is defined in is not stored in the MIR).

MS1_EXACT_OBJ_NOT_FND - The supplied object id did not exactly specify an
    object in the MIR.  (Ie, part of the object id may have specified an object,
    but the full id with all of its arcs taken into consideration did not
    specify an object exactly in the MIR).

MS1_MISSING_SMI - The object had a datatype defined for its value but the MIR
    was built without specifying which SMI it was defined by.  This is means
    there was an error in the MIR compiler or the MIR Database file was
    corrupted.

MS1_INIT_FAIL - Attempts to lookup relationship objects needed by Tier 1
    functions failed.  This may be due to a corrupted MIR Database file
    or one that was improperly created by the MIR Compiler.

Additionally, this function can return all the error codes that the following
functions may return:

    mir_oid_to_mandle() - During lookup of the object
    mir_search_rel_table() - During "walk" of the rel. tables to find the SMI

BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user that wants to know which SMI defines
        a particular object (specified by Object ID).

    Purpose:
        This function accesses the MIR via Tier 0 functions and attempts
        to lookup the specified object.  On success, this function attempts
        to walk the MIR definition of the datatype of the object.  Once this
        is found, the SMI that defined the datatype is returned, thereby
        indicating which SMI defined the object.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (relationship mandles have not been looked up yet)
        if (perform scan to obtain required mandles FAILED)
            <return status code from scan>

    <attempt to lookup desired object by oid>
    if (status was SHORT, LONG, LONGEST or NONE)
        <return MS1_EXACT_OBJ_NOT_FND>

    if (status was NOT MS0_FIND_EXACT)
        <return the status>

    <signal "no MIR_Structured_As found yet">

    do
        <attempt search of object by "Structured_As">
        if (status is MS_FIND_NONTERM)
            <release the original searched object mandle>
            <substititue the found object's mandle as new search-object mandle>
            <signal "MIR_Structured_as" found>
        while (status is MS0_FIND_NONTERM)

    if (status is not MS0_FIND_NONE)
        <return MS1_MISSING_SMI>

    if (no MIR_Structured_As match was found)
        <return "SMI-Unknown" to the caller's cell>
        <return MS1_NOT_A_VALUED_OBJECT>

    <attempt a search of the current search object for MIR_DC_Found_In_SMI>

    if (status is MS0_FIND_NONE or MS0_FIND_NONTERM)
        <return "SMI-Unknown" to the caller's cell>
        <return MS1_MISSING_SMI>

    else if (status was MS0_FIND_TERM)

        if (the value returned was a number)
            <return the number as the smi code>
            <return MS_SUCCESS>
        else
            <return MS1_MISSING_SMI>
    else
        <return the return code>

OTHER THINGS TO KNOW:

    The capitol of Nevada is Carson City.
    
*/

{
mir_status      status;         /* Local status code holder              */
mir_status      s_status;       /* Local search status code holder       */
mir_value       m_value;        /* Terminal value structure              */
BOOL            no_structured;  /* TRUE: MIR_Structured_As not found yet */


/* if (relationship mandles have not been looked up yet) */
if (m_mir_structured_as == NULL) {

    /* MTHREAD: This call should be once-block limited     */
    /* if (perform scan to obtain required mandles FAILED) */
    if ( (status = load_local_mandles()) != MS_SUCCESS) {
        return (status);
        }
    }

/* attempt to lookup desired object by oid */
status = mir_oid_to_mandle(GET_EXACT,
                           oid,
                           &m_scanner,
                           &local_class,
                           NULL);

/* if (status was SHORT, LONG  or NONE) */
if (status == MS0_FIND_SHORT ||
    status == MS0_FIND_LONG ||
    status == MS0_FIND_NONE) {
    return (MS1_EXACT_OBJ_NOT_FND);
    }

/* if (status was NOT MS0_FIND_EXACT) */
if (status != MS0_FIND_EXACT) {
    return (status);
    }

/* signal "no MIR_Structured_As found yet" */
no_structured = TRUE;

do {
    /* attempt search of object by "Structured_As" */
    s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                    m_scanner,          /* Search this      */
                                    m_mir_structured_as,/* ..for this       */
                                    &local_class,       /* m-class to ues   */
                                    &m_scanner,         /* use this on ret  */
                                    &m_value);          /* (this should not */
                                                        /* get used)        */
    /* if (status is MS0_FIND_NONTERM) */
    if (s_status == MS0_FIND_NONTERM) {
        /* release the original searched object mandle */
        /* substititue the found object's mandle as new search-object mandle */
        /* (All of the above done as part of the search_by_mandle call above)*/
        /* signal "MIR_Structured_as" found */
        no_structured = FALSE;
        }
    }
    while (s_status == MS0_FIND_NONTERM);

/* if (status is not MS0_FIND_NONE) */
if (s_status != MS0_FIND_NONE) {
    return (MS1_MISSING_SMI);
    }

/* if (no MIR_Structured_As match was found) */
if (no_structured == TRUE) {

    /* return "SMI-Unknown" to the caller's cell */
    *smi = MIR_SMI_UNKNOWN;

    return (MS1_NOT_A_VALUED_OBJECT);
    }

/* attempt a search of the current search object for MIR_DC_Found_In_SMI */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                m_scanner,          /* Search this      */
                                m_mir_dc_found_in_smi,/* ..for this      */
                                &local_class,       /* m-class to ues   */
                                &m_scanner,         /* use this on ret  */
                                &m_value);          /* (this should not */
                                                    /* get used)        */

/* if (status is MS0_FIND_NONE or MS0_FIND_NONTERM) */
if (s_status == MS0_FIND_NONE || s_status == MS0_FIND_NONTERM) {

    /* return "SMI-Unknown" to the caller's cell */
    *smi = MIR_SMI_UNKNOWN;
    return (MS1_MISSING_SMI);
    }

else if (s_status == MS0_FIND_TERM) {

    /* if (the value returned was a number) */
    if (IS_MV_NUMBER((&m_value))) {
        /* return the number as the smi code */
        *smi = MV_SNUMBER((&m_value));
        return (MS_SUCCESS);
        }
    else {
        return (MS1_MISSING_SMI);
        }
    }
else {
    /* return the return code */
    return (s_status);
    }
}

/* load_ident - Load Data-Construct Identification Block */
/* load_ident - Load Data-Construct Identification Block */
/* load_ident - Load Data-Construct Identification Block */

mir_status load_ident (src, ident)

mandle          *src;       /* Ptr to mandle for object to search   */
dc_ident        *ident;     /* Address of D-C identification block  */

/*
INPUTS:

    "src" is a mandle-pointer for the non-terminal object
    representing the data-construct from which we want to extract as 
    much of the basic information as we can.

    "ident" is a user-visible (non-opaque) data structure that receives
    descriptive information about the data-cnostruct for the specified
    object.

OUTPUTS:

The function returns one of:

MS_SUCCESS - All values from the list of

                TextName  - MIR_Text_Name
                ID Code   - MIR_ID_Code   (optionally present)
                         or
                Smi       - MIR_DC_Found_In_SMI
                Code      - MIR_DC_SMI_Code
                Name      - MIR_DC_SMI_Name
                Class     - MIR_DC_ASN1_Class
                Tag       - MIR_DC_ASN1_Tag    (Application Tag Required)
                  Tag      - MIR_DC_ASN1_Tag   (Universal Tag Optional)
                Size      - MIR_DC_MCC_DT_Size (Optional)
                CMIP Code - MIR_DC_NCL_CMIP_Code  (Optional)

    that could be found in the supplied Non-Terminal have had their
    values loaded to the "ident" block.


MS1_DC_NT_CORRUPT - If any problems occur calling the mir_search_rel_table()
    function during "walk" of the relationship tables.


BIRD'S EYE VIEW:
    Context:
        The caller is a one of the other functions in this module
        that needs to return the basic stuff for a data-construct.

    Purpose:
        This function scans the data-construct non-terminal and extracts
        as much as it can.


ACTION SYNOPSIS OR PSEUDOCODE:

    See the code, it is linear.


OTHER THINGS TO KNOW:

    This function returns MS1_DC_NT_CORRUPT if it cannot return at least
    the Name (and maybe the Code if it is present, both should be present
    for any and every non-terminal representing a data-construct) either by
    relationships MIR_Text_Name/MIR_ID_Code, or MIR_DC_SMI_Name/MIR_DC_SMI_Code

    With V1.9x and the advent of ECO 42, the MIR_ID_Code might be absent,
    so we allow this to happen now without returning MS1_DC_NT_CORRUPT.
    (Tip 'o the Hat to Jerry Plouffe).
*/

{
mir_status      s_status;       /* Local search status code holder       */
mir_value       m_value;        /* Terminal value structure              */

/* Assume we won't get anything */
ident->b_smi = ident->b_asn1_tag = ident->b_asn1_utag = ident->b_asn1_class =
    ident->b_code = ident->b_name = ident->b_mcc_size = ident->b_cmip_code
        = FALSE;

/* -------------------------------------- */
/* Try for the Name under "MIR_Text_Name" */
/* -------------------------------------- */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_text_name,    /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_STRING((&m_value)) ) {
        ident->name = MV_STRING((&m_value));
        ident->b_name = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* ------------------------------------ */
/* Try for the Code under "MIR_ID_Code" */
/* ------------------------------------ */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_id_code,      /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->code = MV_SNUMBER((&m_value));
        ident->b_code = TRUE;
        }
    }


/*
| If we did get at least the name, that is all we'll get, since we aren't
| dealing with a builtin or builtin-template.
*/
if (ident->b_name == TRUE) {
    return (MS_SUCCESS);
    }


/*
|  OK, we are probably dealing with a builtin or builtin-template..
*/
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_dc_smi_name,  /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_STRING((&m_value)) ) {
        ident->name = MV_STRING((&m_value));
        ident->b_name = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* ---------------- */
/* Try for the Code */
/* ---------------- */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_dc_smi_code,  /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->code = MV_SNUMBER((&m_value));
        ident->b_code = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* If we didn't get at least these two, something is really wrong */
if (ident->b_name == FALSE || ident->b_code == FALSE) {
    return (MS1_DC_NT_CORRUPT);
    }

/* ----------------------- */
/* Try for the ASN.1 Class */
/* ----------------------- */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_dc_asn1_class,/* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->asn1_class = MV_SNUMBER((&m_value));
        ident->b_asn1_class = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* ---------------------------------------
|  Try for the ASN.1 Tag (Application Tag)
|
|  (The first instance will be the Application Tag).
*/

s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                /* Search this         */
                                m_mir_dc_asn1_tag,  /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->asn1_tag = MV_SNUMBER((&m_value));
        ident->b_asn1_tag = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* ---------------------------------------
|  Try for the ASN.1 Tag (Universal Tag)
|
|  (The second instance, if present, will be the Universal Tag.  Note that
|   we search "FROMLAST").
*/

s_status = mir_search_rel_table(SEARCH_FROMLAST,
                                src,                /* Search this         */
                                m_mir_dc_asn1_tag,  /* ..for this          */
                                &local_class,       /* (shld not be used)  */
                                &m_scanner,         /* (shld not be used)  */
                                &m_value);          /* Value we want comes */
                                                    /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->asn1_utag = MV_SNUMBER((&m_value));
        ident->b_asn1_utag = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* -------------------- */
/* Try for the SMI Code */
/* -------------------- */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                  /* Search this         */
                                m_mir_dc_found_in_smi,/* ..for this          */
                                &local_class,         /* (shld not be used)  */
                                &m_scanner,           /* (shld not be used)  */
                                &m_value);            /* Value we want comes */
                                                      /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->smi = (mir_smi_code) MV_SNUMBER((&m_value));
        ident->b_smi = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* -----------------------------
|  Try for the MCC Datatype Size
|  -----------------------------
*/

s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                 /* Search this         */
                                m_mir_dc_mcc_dt_size,/* ..for this          */
                                &local_class,        /* (shld not be used)  */
                                &m_scanner,          /* (shld not be used)  */
                                &m_value);           /* Value we want comes */
                                                     /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->mcc_size = MV_SNUMBER((&m_value));
        ident->b_mcc_size = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

/* -----------------------------
|  Try for the NCL CMIP Code
|  -----------------------------
*/

s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                src,                 /* Search this         */
                                m_mir_dc_ncl_cmip_code,/* ..for this        */
                                &local_class,        /* (shld not be used)  */
                                &m_scanner,          /* (shld not be used)  */
                                &m_value);           /* Value we want comes */
                                                     /*  back here).        */

/* if (status was NOT "MS0_FIND_NONE") */
if (s_status != MS0_FIND_NONE) {
    if ( (s_status == MS0_FIND_TERM) && IS_MV_NUMBER((&m_value)) ) {
        ident->cmip_code = MV_SNUMBER((&m_value));
        ident->b_cmip_code = TRUE;
        }
    else {
        return (MS1_DC_NT_CORRUPT);
        }
    }

return(MS_SUCCESS);
}

/* mir_lookup_data_construct - Lookup "Datatype" of an Object-with-a-Value */
/* mir_lookup_data_construct - Lookup "Datatype" of an Object-with-a-Value */
/* mir_lookup_data_construct - Lookup "Datatype" of an Object-with-a-Value */

mir_status mir_lookup_data_construct (oid, context, ident)

object_id       *oid;       /* -> Object Id of object whose D-Cis desired */
dc_context      **context;  /* Address of ptr to be set to context block  */
dc_ident        *ident;     /* Address of D-C identification block        */

/*
INPUTS:

    "oid" is the address of an object identifier that indicates the MIR
    object whose data-construct description is desired.

    "context" receives internal data used for subsequent call to
    mir_eval_data_construct() if the valued-object turns out to have
    a data-construct that is "built-up" from other data-constructs.  This
    structure should be considered OPAQUE to the caller of this function.

    "ident" is a user-visible (non-opaque) data structure that receives
    descriptive information about the data-construct for the specified
    object.

OUTPUTS:

The function returns one of:

SUCCESS:

MS1_DC_BUILTIN - The specified object's value is formed from a data-construct 
    that is "built-in" to the SMI (a basic "atomic" data construct like
    "BOOLEAN").  In this case, the function loads the "ident" block with
    as much information on this data-construct as the MIR contains, usually
    Code, Name, ASN.1 Tag, ASN.1 Class and SMI indicator.  If any of these
    values cannot be fetched from the MIR, the associated boolean flag in
    the ident block is set FALSE.  The "context" block is not created nor
    returned, this argument remains unreferenced.

    For example, this return occurs given an OID for an attribute:

        ATTRIBUTE Zombie = 1 : BOOLEAN;

    "ident"
        smi:        MIR_SMI_DNA  (this is an enumerated value)
        asn1_tag:   1
        asn1_class: 0
        code:       1
        name:       "Boolean"


MS1_DC_BUILTUP - The specified object's value is formed from a data-construct 
    that is "built-up" from other data-constructs (either "atomic" or
    "template") defined by the SMI.  The "ident" block is loaded with the
    name and code of the "built-up" data-construct and the "context" block
    is loaded with opaque information (to be used in a call to
    mir_eval_data_construct() ) to obtain the description of the data-construct
    from which the "built-up" instance is built from.  In other words, the
    ident block gives all that is known "at this level", and the context
    block gives access to the next level "down" in the data-construct
    definition.

    For example, this return occurs given an OID for an attribute and TYPE
    statements:

        TYPE DeadorAlive = 5 : BOOLEAN;

        ATTRIBUTE Zombie = 1 : DeadorAlive;

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       5
        name:       "DeadorAlive"

    "context"
        is loaded with opaque information needed to obtain information about
        "BOOLEAN".  See documentation for mir_eval_data_construct() for
        a description of how this information can be returned.  See
        documentation for "mir_dissolve_context()" to see how storage
        for this allocated block is to be released.


FAILURE:

MS1_NOT_A_VALUED_OBJECT - The specified object was found in the MIR, however
    it does not define something that has  a value, consequently there is
    no data-construct associated with this object.  V1.5 Note:  if the object
    IS the definition of a data-construct, then the defining smi indicator
    is set in the ident block on return along with all the other stuff (name,
    code, asn1 info).  This may constitute a sort of success.

MS1_EXACT_OBJ_NOT_FND - The supplied object id did not exactly specify an
    object in the MIR.  (Ie, part of the object id may have specified an
    object, but the full id with all of its arcs taken into consideration
    did not specify an object exactly in the MIR).

MS1_INIT_FAIL - Attempts to lookup relationship objects needed by Tier 1
    functions failed.  This may be due to a corrupted MIR Database file
    or one that was improperly created by the MIR Compiler.

Additionally, this function can return all the error codes that the following
Tier 0 functions may return:

    mir_oid_to_mandle() - During lookup of the object
    mir_search_rel_table() - During "walk" of the rel. tables to find the SMI

BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user that wants to know "all about" the
        data construct used to form the value of an object in the MIR.

    Purpose:
        This function accesses the MIR via Tier 0 functions and attempts
        to lookup the specified object.  On success, this function returns
        all that the MIR knows about the first level of the definition for
        the object's value.


ACTION SYNOPSIS OR PSEUDOCODE:

-----
MACRO "MANDLE_RELEASE_RETURN(code)" ... "MRR()"
    if (Release-on-Exit == TRUE)
        <release all mandles via mandle-class request>
        <return "code">
    else
        <copy all mandle pointers & mandle-class ptr to context block>
        <return "code">
-----

    if (context block pointer is NULL or ident block is NULL)
       <return MS1_INVALID_ARG>

    <set mandle local storage pointers NULL>
    <signal "Release-on-Exit">

    if (relationship mandles have not been looked up yet)
        if (perform scan to obtain required mandles FAILED)
            <return status code from scan>
    else
        if (context block pointer is non-null)
            <copy any mandle pointers and mandle-class ptr to local storage>
            <signal "Release-on-Exit" = FALSE>

    <attempt to lookup desired object by oid>
    if (status was SHORT, LONG, LONGEST or NONE)
        <MRR(MS1_EXACT_OBJ_NOT_FND)>

    if (status was NOT MS0_FIND_EXACT)
        <MRR(status)>

    <attempt search of object by "MIR_Structured_As">
    if (status was NOT "MS0_FIND_NONTERM")
        if (attempt to load the ident block failed)
            <MRR(status)>
        <MRR(MS1_NOT_A_VALUED_OBJECT)>

    <load all data-construct info to ident block>

    if (SMI code is present)
        <MRR(MS1_DC_BUILTIN)>
    else
        <attempt search on data-construct for MIR_Structured_As>
        if (status was NOT MS0_FIND_NONTERM)
            <MRR(MS1_DC_NT_CORRUPT)>
            
    if (context block pointer is NULL)
        <allocate space for a context block>
        <return pointer to it>
        <"Release-on-Exit" = FALSE>

    <MRR(MS1_DC_BUILTUP)>

OTHER THINGS TO KNOW:

*   Note that you supply the address OF A POINTER to the context block,
    this function allocates storage and gives it to you if you need it.

*   You dump it when done by giving it to mir_dissolve_context().

*   You can re-use an allocated context block on another call to this
    function... it is not necessary to "dissolve" the context until
    you are really, really done and never going to call this function again.

*   You have to EXACTLY specify an object in the MIR (by OID) before you can
    get its datatype with this function.

*   This function obtains the scoop on an object's VALUE (that is, the 
    definition of the value).  With V1.5, the MIR objects that describe
    dataconstructs also now have OIDs assigned to them.  That means that
    this function can be used to lookup these objects.  Such an object
    once looked up though, HAS NO VALUE, and consequently this function
    returns MS1_NOT_A_VALUED_OBJECT, just as it does when you look up
    things that are defined by an MSL that have "no value" (an entity
    class for instance).  The information in an object that describes
    a dataconstruct IS (however) returned in the "ident" block nonetheless
    at the time MS1_NOT_A_VALUED_OBJECT is returned.

The management of mandles in the context block (handled thru local flag
"release_on_exit") by this function is deceptively simple.  Be Very Careful
about changing any of it!

*/

/* MANDLE_REL_RET - Mandle Release and Return
|
|  All returns from this function should pass through this macro.
|
|  This macro takes care of cleaning up mandles.. it allows us to easily
|  write code that uses any mandles passed in a context block OR locally
|  defined mandles, and everything is supposed to work out OK.
|
|  NOTE: References several function-local variables DIRECTLY!
*/

#define MANDLE_REL_RET(code,cl)                                             \
    if (release_on_exit == TRUE) {                                          \
        mir_status  free_status;                                            \
                                                                            \
        /* release both src & trg mandles via mandle-class request */       \
        if ( (cl != NULL)                                                   \
            && (free_status = mir_free_mandle_class(&cl)) != MS_SUCCESS)    \
            return(free_status);                                            \
        else                                                                \
            return (code);                                                  \
        }                                                                   \
    else {                                                                  \
        /* copy both mandle pointers & mandle-class ptr to context block */ \
        (*context)->src = src;                                              \
        (*context)->trg = trg;                                              \
        (*context)->ott = ott;    /* (Always NULL out of this function) */  \
        (*context)->cclass= mc;                                             \
        return (code);                                                      \
        }

{
mir_status      status;         /* Local status code holder              */
mir_status      s_status;       /* Local search status code holder       */
mir_value       m_value;        /* Terminal value structure              */
BOOL            release_on_exit;/* TRUE: Release the mandles on exiting  */
mandle_class    *mc;            /* Local Mandle Class pointer            */
mandle          *src;           /* "Source" Mandle                       */
mandle          *trg;           /* "Target" Mandle                       */
mandle          *ott;           /* "One-Time Template " Mandle           */


/* if (context block pointer is NULL or ident block is NULL) */
if (context == NULL || ident == NULL) {
   return (MS1_INVALID_ARG);
   }

/* set mandle local storage pointers NULL */
mc = src = trg = ott = NULL;    /* Start off "empty"        */
release_on_exit= TRUE;          /* signal "Release-on-Exit" */

/* if (relationship mandles have not been looked up yet) */
if (m_mir_structured_as == NULL) {

    /* if (perform scan to obtain required mandles FAILED) */
    if ( (status = load_local_mandles()) != MS_SUCCESS) {
        return (status);
        }
    }
else { /* it's possible they are passing in a context block to re-use */
    /* if (context block pointer is non-null) */
    if (*context != NULL) {
        /* copy any mandle pointers and mandle-class ptr to local storage */
        src = (*context)->src;
        trg = (*context)->trg;
        mc = (*context)->cclass;

        /* if there is an active one-time template mandle, blow it to NULL */
        if ((*context)->ott != NULL) {
            if ((status = mir_free_mandle(&((*context)->ott))) != MS_SUCCESS) {
                return(status);
                }
            }
        /*
        | "ott" (local variable & in context block) is always null:
        |       this is its initialization state.
        */

        /* signal "Release-on-Exit" = FALSE
        |
        | (This because we've copied live mandles that the user will later
        |  re-use again or release for us anyway through a call to
        |  mir_dissolve_context() )
        */
        release_on_exit = FALSE;
        }
    }

/* attempt to lookup desired object by oid */
status = mir_oid_to_mandle(GET_EXACT, oid, &m_scanner, &local_class, NULL);

/* if (status was SHORT, LONG  or NONE) */
if (status == MS0_FIND_SHORT ||
    status == MS0_FIND_LONG ||
    status == MS0_FIND_NONE) {
    return(MS1_EXACT_OBJ_NOT_FND);
    }

/* if (status was NOT MS0_FIND_EXACT) */
if (status != MS0_FIND_EXACT) {
    return(status);
    }

/* --------------------------------------------------------------------------
| After this point, we're going to use the src, trg and the mandle class
| that will contain the src & trg. . . consequently exits from this function
| should go by way of the MANDLE_REL_RET() macro so they can be copied/released
| as needed.
* -------------------------------------------------------------------------- */

/*
| At this point, as of V1.5, we could have some object defined in an MSL (as in
| days of old) OR we could have an object that is a "builtin" dataconstruct
| (ie defined in the builtin_types.dat file).
|
| Only if the non-terminal object has a "MIR_Structured_As" relationship
| in its relationship table does this object "have a value".  With V1.5,
| (now that builtin-types have OIDs assigned to them), this object might
| actually BE a builtin value, and we should return its smi code,
| asn1 class/tag, name and which smi it came from.
|
| However, it strictly speaking is still not a "valued-object", so we do
| indeed return MS1_NOT_A_VALUED_OBJECT, but the ident block should be
| filled in.
|
| With V1.9x and the advent of MCC syntax and ECO 42, it may be that the
| ID Code is missing from something and cannot be returned.  The load_ident
| function is modified to admit of this possibility.
*/
/* attempt search of object by "MIR_Structured_As" */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                    m_scanner,          /* Search this      */
                                    m_mir_structured_as,/* ..for this       */
                                    &mc,                /* m-class to use   */
                                    &src,               /* use this on ret  */
                                    &m_value);          /* (this should not */
                                                        /* get used)        */

/* if (status was NOT "MS0_FIND_NONTERM") */
if (s_status != MS0_FIND_NONTERM) {

    /*
    | Ok, it doesn't have a value, let's try to load the standard stuff
    | from it nonetheless.  If the defining smi code is present it'll be
    | a dataconstruct definition, and the caller will get what they want
    */
    if ((status = load_ident(m_scanner, ident)) != MS_SUCCESS) {
        MANDLE_REL_RET(status,mc)
        }
    MANDLE_REL_RET(MS1_NOT_A_VALUED_OBJECT,mc);
    }

/* load all data-construct info to ident block */
if ((status = load_ident(src, ident)) != MS_SUCCESS) {
    MANDLE_REL_RET(status,mc)
    }

/*
| Note:  When MS1_DC_BUILTIN is returned here, it must perforce be describing
|        a datatype that is not a BUILTIN-Template (ie, it can't be a RECORD
|        or ENUMERATED or any type that requires template information).  This
|        is because the compiler will not allow the following syntax:
|
|        ATTRIBUTE FOOBAR = 1 : RECORD (
|                                       .
|                                       .
|                                       );
|
|        Any use of a datatype that requires builtin-template information
|        (such as RECORD) requires the use of an intervening TYPE statement
|        to define the datatype... only TYPE statements allow RECORD/ENUMERATED
|        or other kinds of datatype definitions that use template info.  So...
|
|        ATTRIBUTE FOOBAR = 1 : BOOLEAN;
|
|        ...will cause MS1_DC_BUILTIN to be returned here because BOOLEAN
|        does not require template information to describe it.
*/

/* if (SMI code is present. . ) */
if (ident->b_smi == TRUE) {
    MANDLE_REL_RET(MS1_DC_BUILTIN,mc);  /* . . then we're looking at BUILTIN */
    }
else {  /* 'src' should be a Non-Terminal representing a TYPE statement */
    /* attempt search on data-construct for MIR_Structured_As */
    s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                    src,                /* Search this      */
                                    m_mir_structured_as,/* ..for this       */
                                    &mc,                /* m-class to ues   */
                                    &trg,               /* use this on ret  */
                                    &m_value);          /* (this should not */
                                                        /* get used)        */
    /* if (status was NOT MS0_FIND_NONTERM) */
    if (s_status != MS0_FIND_NONTERM) {
        MANDLE_REL_RET(MS1_DC_NT_CORRUPT,mc);
        }
    }

/* if (context block pointer is NULL) */
if (*context == NULL) {

    /* allocate space for a context block */
    /* return pointer to it */
    if ((*context = (dc_context *) malloc (sizeof(dc_context)) ) == NULL) {
        return (MS_NO_MEMORY);
        }

    /*  "Release-on-Exit" = FALSE
    |
    |   The context block should be loaded with 'real stuff' that should not
    |   be dumped.  (The explicit assignment of NULL to "ott" w/in the context
    |   block by code in the MANDLE_REL_RET() macro is done to be sure the
    |   NULL overrides any garbage that may lie in that cell as a consequence
    |   of the malloc() call above).
    */
    release_on_exit = FALSE;
    }

/* This macro loads the context block before the exit occurs */
MANDLE_REL_RET(MS1_DC_BUILTUP,mc);

/* (Control never reaches here) */
}

/* mir_eval_data_construct - Evaluate previously fetched Data Construct */
/* mir_eval_data_construct - Evaluate previously fetched Data Construct */
/* mir_eval_data_construct - Evaluate previously fetched Data Construct */

mir_status mir_eval_data_construct (context, ident)

dc_context      **context;  /* Addr of ptr to prev. established context block*/
dc_ident        *ident;     /* Address of D-C identification block        */

/*
INPUTS:

    "context" - is the pointer to the address of a previously created context
    block established by an earlier call to mir_lookup_data_construct().
    The contents of this block may be modified as a consequence of this call
    or the block may be dissolved.  See OUTPUT section below.  This structure
    should be considered OPAQUE to the caller of this function.

    "ident" is a user-visible (non-opaque) data structure that receives
    descriptive information about the data-construct for the specified
    object.

OUTPUTS:

The function returns one of:

SUCCESS:

MS1_DC_BUILTIN - The evaluation of the data-construct provided in the context
    block turned out to be "built-in" to the SMI (a basic "atomic" data
    construct such as "BOOLEAN").  In this case, the function loads the
    "ident" block with as much information on this data-construct as the MIR
    contains, usually Code, Name, ASN.1 Tag, ASN.1 Class and SMI indicator.
    If any of these values cannot be fetched from the MIR, the associated
    boolean flag in the ident block is set FALSE.  The "context" block 
    provided in the call is automatically "dissolved" on behalf of the caller
    through a call to mir_dissolve_context().

    For example, consider the attribute and TYPE statements:

        TYPE DeadorAlive = 5 : BOOLEAN;

        ATTRIBUTE Zombie = 1 : DeadorAlive;

    On the call to mir_lookup_data_construct(), a context block is established
    (containing OPAQUE data describing "DeadorAlive") and an "ident" is 
    returned containing:

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       5
        name:       "DeadorAlive"

    "context"
        (opaque data)

    On a call to this function, passing in the context block created by the
    call to mir_lookup_data_construct() (and another pointer to an "ident"
    block), the MS1_DC_BUILTIN code is returned and:

    "ident"
        smi:        MIR_SMI_DNA  (this is an enumerated value)
        asn1_tag:   1
        asn1_class: 0
        code:       1
        name:       "Boolean"

    "context"
        (context block is dissolved).


MS1_DC_BUILTUP - The data-construct specified by the opaque data in the
    supplied context block turns out to be "built-up" from yet other
    data-construct defined by the SMI.  The data-construct is guaranteed to
    be EITHER a basic NON-TEMPLATE data-construct (like "Boolean") defined by
    the SMI or one defined by the user using a TYPE statement in the MSL.
    (This differs slightly from the meaning given this code when it is
    returned by mir_lookup_data_construct().  In that case, MS1_DC_BUILTUP
    meant both cases above PLUS the "built-in (to the SMI) template" case too.)

    The "ident" block is loaded with the name and code of the "built-up"
    data-construct and the "context" block is re-loaded with opaque information
    (to be used in yet another call to mir_eval_data_construct() ) to obtain
    the description of the data-construct from which the current "built-up"
    instance is built from.  In other words, just as in the case of a call to
    mir_lookup_data_construct(), the ident block gives all that is known
    "at this level", and the context block gives access to the next level
    "down" in the data-construct definition.

    For example, given the following attribute and TYPE statements:

        TYPE RectangularHole = 4 : RECORD  ..... END RECORD ;

        TYPE SquareHole = 5 : RectangularHole;

        ATTRIBUTE Steamshovel = 1 : SquareHole;

    On the first call to mir_lookup_data_construct() with the OID for the
    attribute we get:

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       5
        name:       "SquareHole"

    "context"
        is loaded with opaque information needed to obtain information about
        SquareHole's definition: "RectangularHole".

    Passing "context" (and another pointer to an "ident" block) to
    mir_eval_data_construct(), MS1_DC_BUILTUP is returned and we get:

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       4
        name:       "RectangularHole"

    "context"
        is loaded with opaque information needed to obtain information about
        RectangularHole's definition: "RECORD".


MS1_DC_BUILTIN_TMPLT - The data-construct specified by the opaque data in the
    supplied context block turns out to be "built-up" from a SMI-defined
    "builtin-template" data-construct.  (In other words, it is "constructed"
    in the usual sense (RECORD, BITSET etc), or it may be an Integer with a
    minimum or maximum range).

    The "ident" block is loaded with the name and code of the
    "builtin-template" data-construct and the "context" block is re-loaded
    with opaque information (to be used in a call to
    mir_eval_builtin_template() ) to obtain further information about how
    the current "built-up" instance is constructed.

    Using the example given above for the MS1_DC_BUILTUP return code, if
    the context block returned from the last call in the example above
    (containing opaque information describing RectangularHole's definition)
    is submitted again to mir_eval_data_construct(), then MS1_DC_BUILTIN_TMPLT
    is returned with:

    "ident"
        smi:        MIR_SMI_DNA  (this is an enumerated value)
        asn1_tag:   16
        asn1_class: 0
        code:       203
        name:       "RECORD"

    "context"
        is loaded with opaque information needed to obtain information about
        how to interpret the template (for use in a call to
        mir_eval_builtin_template() ).



FAILURE:

This function can return all the error codes that the following
Tier 0 functions may return:

    mir_oid_to_mandle() - During lookup of the object
    mir_search_rel_table() - During "walk" of the rel. tables to find the SMI


BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user that wants to know "all about" the
        data construct used to form the value of an object in the MIR.

    Purpose:
        This function uses the context block established by a prior call
        to mir_lookup_data_construct().  On success, this function returns
        all that the MIR knows about the level of the definition contained
        in the context block, plus any needed opaque information required to
        access the next level down in the definition if there is one.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (context block pointer is NULL or ident block is NULL)
       <return MS1_INVALID_ARG>

    <load all data-construct info to ident block using target mandle from
     context block>

    if (SMI code is present)
        <attempt search on data-construct target mandle for
         MIR_DC_SMI_Template>
        if (status was MS0_FIND_NONTERM)
            <attempt search on data-construct target mandle for
             MIR_DC_SMI_OTTemplate, return any NT to "ott" mandle>
            if (status is neither MS0_FIND_NONE nor MS0_FIND_NONTERM)
               <return MS1_DC_NT_CORRUPT>
            <reset the search on target mandle to "The Top">
            <return MS1_DC_BUILTIN_TMPLT>
        else if (status was NOT MS0_FIND_NONE)
            <return MS1_DC_NT_CORRUPT)
        <dissolve context block>
        <return MS1_DC_BUILTIN>

    <interchange context block source and target mandle>
    <attempt search on data-construct new src mandle for MIR_Structured_As
     with returned mandle being the context block target mandle>
    if (status was NOT MS0_FIND_NONTERM)
        <return MS1_DC_NT_CORRUPT>

    <return MS1_DC_BUILTUP>

OTHER THINGS TO KNOW:

*   Note that you supply the address OF A POINTER to the context block,
    this function allocates storage and gives it to you if you need it.

*   You dump the context block when done by giving it to mir_dissolve_context()
    --if MS1_DC_BUILTIN was NOT returned (but it won't hurt if you do it when
    MS1_DC_BUILTIN is returned, since mir_dissolve_context() can handle a
    NULL pointer).

*   You can re-use an allocated context block on another call to this
    function to "step down" a level in the definition if MS1_DC_BUILTUP was
    returned.

With V2.00, we add the search that initializes the "ott" cell in the context
block for looking up One-Time Template 'things': as of V2.00, the only such
'things' are Case Codes in Variant Records.
*/

{
mir_status      status;         /* Local status code holder              */
mir_status      s_status;       /* Local search status code holder       */
mir_value       m_value;        /* Terminal value structure              */
mandle          *swap;          /* Temporary storage for swap            */


/* if (context block pointer is NULL or ident block is NULL) */
if (context == NULL || ident == NULL) {
   return (MS1_INVALID_ARG);
   }

/* load all data-construct info to ident block */
if ((status = load_ident((*context)->trg, ident)) != MS_SUCCESS) {
    return(status);
    }

/*
|  We KNOW it is NOT a "built-up" non-terminal if smi code is present (ie,
|  we know we're 'At the Bottom').
|
|  If the SMI code is present, that means we are dealing with a raw "builtin"
|  or "builtin-template", both of which should carry the "basic five" 
|  (smi, asntag, asnclass, smi_code, smi_name).  In both these cases, we've
|  "descended" to the bottom-most definition in any tree of TYPE statements.
|
|  We must return a status indicating whether "builtin" or "builtin_template".
|
|  In the case of templates, there will be entries in the relationship
|  table using the MIR_DC_SMI_Template (and possibly MIR_DC_SMI_OTTemplate)
|  relationships, so we return MS1_DC_BUILTIN_TMPLT to let the caller know
|  that the templates can be evaluated, otherwise we just return
|  MS1_DC_BUILTIN.
*/
/* if (SMI code is present) */
if (ident->b_smi == TRUE) {
    /* attempt search on data-construct trg for MIR_DC_SMI_Template */
    s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                (*context)->trg,        /* Search this      */
                                m_mir_dc_smi_template,  /* ..for this       */
                                &local_class,           /* m-class to use   */
                                &m_scanner,             /* use this on ret  */
                                &m_value);              /* (this should not */
                                                        /* get used)        */
    /*
    | OK, if MS0_FIND_NONTERM returned, that means the Non-Terminal for
    | the builtin datatype has template information (such as for datatypes
    | like RECORD and ENUMERATION.  Reset the mandle so that
    | mir_eval_builtin_template() can relookup the first template relationship
    | when the user calls for it.
    */
    /* if (status was MS0_FIND_NONTERM) */
    if (s_status == MS0_FIND_NONTERM) {

        /*
        | attempt search on data-construct target mandle for
        | MIR_DC_SMI_OTTemplate, return any NT to "ott" mandle
        |
        | Here, we're initializing the "ott" mandle in the context block
        | for use by mir_eval_builtin_template().  If this search finds
        | a Relationship object as a target of the One-Time Template rel,
        | mir_eval_builtin_template() will use it, otherwise the NULL
        | initialized by mir_lookup_dataconstruct() will remain in "->ott".
        */
        s_status =
            mir_search_rel_table(SEARCH_FROMTOP,
                                (*context)->trg,        /* Search this      */
                                m_mir_dc_smi_ottemplate,/* ..for this       */
                                &(*context)->cclass,    /* m-class to use   */
                                &(*context)->ott,       /* use this on ret  */
                                &m_value);              /* (this should not */
                                                        /* get used)        */
        if (s_status != MS0_FIND_NONE && s_status != MS0_FIND_NONTERM) {
            return(MS1_DC_NT_CORRUPT);
            }

        /*
        | Make sure the search for the template-specifying relationship
        | starts at the top (in mir_eval_builtin_template()).
        */
        mir_reset_search((*context)->trg);
        return (MS1_DC_BUILTIN_TMPLT);
        }
    else if (s_status != MS0_FIND_NONE) {
        return (MS1_DC_NT_CORRUPT);
        }

    /* Dissolve the context block */
    mir_dissolve_context(context);

    return (MS1_DC_BUILTIN);
    }

/*
|  Once we get to here, we are in effect "stepping down" a level in the
|  data-construct definition hierarchy through a "built-up" reference that
|  the user made in the MSL file (ie a TYPE statement).
|
|  Swapping "src" and "trg" mandles and then re-using the target mandle has
|  the effect of "shifting the frame of reference 'rightward'" as the
|  non-terminals representing the data-constructs slide leftward through
|  the context block (where the "Highest" definition appears on the "left"
|  and the "Lowest" SMI-defined underlying data-construct is "to the right").
*/
/* interchange context block source and target mandle */
swap = (*context)->src;
(*context)->src = (*context)->trg;
(*context)->trg = swap;

/* attempt search on data-construct for MIR_Structured_As */
s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                    (*context)->src,    /* Search this      */
                                    m_mir_structured_as,/* ..for this       */
                                    &(*context)->cclass, /* m-class to ues   */
                                    &(*context)->trg,   /* use this on ret  */
                                    &m_value);          /* (this should not */
                                                        /* get used)        */
/* if (status was NOT MS0_FIND_NONTERM) */
if (s_status != MS0_FIND_NONTERM) {
    return(MS1_DC_NT_CORRUPT);
    }

/*
|  At this point, the context block is all set for yet-another call to this
|  very same function to "shift" down the hierarchy again.
*/
return (MS1_DC_BUILTUP);

}

/* mir_eval_builtin_template - Evaluate context block describing template */
/* mir_eval_builtin_template - Evaluate context block describing template */
/* mir_eval_builtin_template - Evaluate context block describing template */

mir_status mir_eval_builtin_template (i_context, number, string, o_context,
                                            ident, restart)

dc_context  **i_context;  /* Addr of ptr to prev. established context block */
int         *number;      /* -> integer to receive a Template Number value  */
char        **string;     /* -> string ptr to recv a Template String value  */
dc_context  **o_context;  /* Addr of ptr to newly-created context block     */
dc_ident    *ident;       /* Address of D-C identification block            */
BOOL        *restart;     /* TRUE: Evaluation is restarting "from the top"  */

/*
INPUTS:

    "i_context" - is the pointer to the address of a (previously created) input
    context block established by an earlier call to
    mir_lookup_data_construct().

    The contents of this block will not be modified as a consequence of this
    call, ALTHOUGH the position where "mir_search_rel_table()" will start its
    next SEARCH_FROMLAST search (on the "src" mandle in this block) will be
    changed!  This structure should be considered OPAQUE to the caller of this
    function.

    "number" - is the pointer to an integer to receive the next template
    value should it turn out to be a number.

    "string" - is the pointer to an char ptr to receive the next template
    value should it turn out to be a string.

    "o_context" - is a pointer to a cell that receives the address of a new
    context block that is created if the next template value turns out to
    be another data-construct.

    "ident" is a user-visible (non-opaque) data structure that receives
    descriptive information about the template-value data-construct in the
    case where "o_context" is created.

    "restart" is a user-supplied variable that is set FALSE on return when
    evaluation succeeds by evaluating the next definition of the instance
    of the builtin-type.  It is set TRUE when this function discovers that
    it has hit "the end" of the current definition and in order to return
    something, "restarts" at the top of the builtin-type.  This allows
    for repeated evaluation of things like "sequences" when user code needs
    to obtain "the same thing" over and over again.

OUTPUTS:

The SUCCESS codes that mir_eval_builtin_template() can return are:

MS1_TMPLT_NUMBER - A number is returned that is the template value

MS1_TMPLT_STRING - A string is returned that is the template value

MS1_DC_BUILTIN - The template value is a data-construct that is "built-in" to
                 the SMI (a basic "atomic" data construct such as "BOOLEAN").
                 The "ident" block is loaded with as much information on this
                 data-construct as the MIR contains, usually Code, Name,
                 ASN.1 Tag, ASN.1 Class and SMI indicator.

MS1_DC_BUILTUP - The template value is a data-construct that is "built-up",
                 either through the use of a TYPE statement or through the
                 use of a "builtin-template".  A new context block is created
                 and returned, which must subseqently be evaluated by a call
                 to mir_eval_data_construct().

Any of these success codes may be returned with "restart" set TRUE, which
means that this function is "starting over" at the top of the original
definition (the definition that was supplied in the context block on the
first call to this function).  In most instances the caller should
interpret TRUE to mean "End of Definition".  For things like SEQUENCE,
the caller may be interested in retrieving the same definition over and
over again, in which case "restart" is ignored.

The FAILURE codes:

MS1_INVALID_ARG - The address of a required argument was NULL.

MS1_DC_NT_CORRUPT - If the Nonterminals describing data-constructs appear
        to be corrupted.

MS0_FIND_NONE - No more template values to be returned (not necessarily an
        error).

This function can return all the error codes that the following
Tier 0 functions may return:

    mir_oid_to_mandle() - During lookup of the object
    mir_search_rel_table() - During "walk" of the rel. tables

NOTE: It is rough to understand these return codes without working through
the "scenario" given under "OTHER THINGS TO KNOW" below.



BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user that wants to know "all about" the
        data-construct (formed from a "built-in template") used to form the
        value of an object in the MIR.

    Purpose:
        This function uses the context block established by a prior call
        to mir_eval_data_construct().  On first successful call, this function
        returns the first piece of information for the template that this
        instance of a data-construct was formed from.  The information returned
        depends entirely on the template used to form this built-up data
        construct and what values were supplied in the TYPE statement in the
        MSL that defined the overall data-construct.  See the file
        "builtin_types.proto" (a version of which is used by the compiler) that
        describes the builtin-templates that this function "reads".


ACTION SYNOPSIS OR PSEUDOCODE:

    <assume restart is FALSE>

    if (context block pointer is NULL or ident block is NULL or string is NULL
        or Number is NULL)
       <return MS1_INVALID_ARG>

    if (one-time-template mandle is non-NULL)
        <attempt FROM-LAST search on input context src mandle using ott mandle>
        switch (return status)
            case MS0_FIND_NONE;
                <reset search on input context src mandle>
                <free the one-time template mandle in the context block>
                <break>

            case MS0_FIND_TERM:
                if (value returned is STRING)
                    <return address of string to caller>
                    <return MS1_TMPLT_STRING>
                else if (value returned is NUMBER)
                    <copy number back to caller>
                    <return MS1_TMPLT_NUMBER>

            default:
                <return MS1_DC_NT_CORRUPT>

    <attempt FROM-LAST search on input context target mandle for
                                                       MIR_DC_SMI_Template>
    if (attempt returns MS0_FIND_NONE)
        <attempt FROM-TOP search on input context target mandle for
                                                       MIR_DC_SMI_Template>
        if (attempt returns MS0_FIND_NONE)
             <return MS0_FIND_NONE>

    if (attempt did not return MS0_FIND_NONTERM)
        <return status>

    <attempt FROM-LAST search on input context src mandle for NT just returned>

    switch (return status)

        case MS0_FIND_TERM:
            if (value returned is STRING)
                <return address of string to caller>
                <return MS1_TMPLT_STRING>
            else if (value returned is NUMBER)
                <copy number back to caller>
                <return MS1_TMPLT_NUMBER>
            <return MS0_DC_NT_CORRUPT>

        case MS0_FIND_NONTERM:
            <load all data-construct info to ident block using mandle to
             non-terminal returned>

            if (SMI code is present)
                <release the local mandle>
                <return MS1_DC_BUILTIN>

            if (attempt to allocate a context block failed)
                <return MS_NO_MEMORY>

            <copy mandle of non-terminal to context block as src mandle>
            <attempt search on non-terminal for MIR_Structured_As>
            if (status was NOT MS0_FIND_NONTERM)
                <MS1_DC_NT_CORRUPT>
            <copy returned mandle non-terminal to context block as trg mandle>
            <return MS1_DC_BUILTUP>

        case MS0_FIND_NONE:
            <perform "reset" on the source mandle in context block>
            <recursively invoke this function>
            <set "restart" argument TRUE>
            <return status>

        default:
            <return status>


OTHER THINGS TO KNOW:

*   Note that you supply the address OF A POINTER to the context block,
    this function allocates storage and gives it to you if you need it.

*   You dump the context block when done by giving it to mir_dissolve_context()
    --if MS1_DC_BUILTIN was NOT returned (but it won't hurt if you do it when
    MS1_DC_BUILTIN is returned, since mir_dissolve_context() can handle a
    NULL pointer).

SIMPLE SCENARIO
---------------
For a fairly simple dataconstruct representation for an attribute that has an
enumerated datatype, the MSL might be:

    TYPE OnOff = 3 (
        Off = 0,
        On = 1);

    ATTRIBUTE FOOBAR = 1 : OnOff;
    ...
    END ATTRIBUTE

the general situation upon entry to this function looks like this:

      (Attribute NT)
     *-------------------*
     | MIR_MCC_ID_Code   |-> 1
     | MIR_Text_Name     |-> "FOOBAR"
     | MIR_Structured_As |---*
     *-------------------*   |     
                             |
                             |
              (TYPE-Stmt NT) V
     (src:)  *-------------------*
             | MIR_ID_Code       |-> 3
             | MIR_Text_Name     |-> "OnOff"
             | MIR_Structured_As |---------*
             | MIR_Enum_Code     |-> 0     |
             | MIR_Enum_Text     |-> "Off" |
             | MIR_Enum_Code     |-> 1     |
             | MIR_Enum_Text     |-> "On"  |
             *-------------------*         |
                                           |
              (NT for Built-in   *---------*
               Template for      |
               Enumeration)      V
     (trg:)  *---------------------*
             | MIR_DC_Found_In_SMI |-> 1
             | MIR_DC_SMI_Code     |-> 10
             | MIR_DC_SMI_Name     |-> "BIDT_ENUMERATION"
             | MIR_DC_MCC_DT_Size  |-> 65535
             | MIR_DC_ASN1_Class   |-> 0         Relationship NT:
             | MIR_DC_ASN1_Tag     |-> 2        *--------------------*
             | MIR_DC_SMI_Template |----------> | ("MIR_Enum_Code")  |
             | MIR_DC_SMI_Template |----*       *--------------------*
             *---------------------*    |
                                        |        Relationship NT:
                                        |       *--------------------*
                                        *------>| ("MIR_Enum_Text")  |
                                                *--------------------*


In the picture above, the TYPE-Statement Non-Terminal is an "instance" of the
"Built-In DataConstruct" for Enumeration.  This is indicated because the
NT for the TYPE statement contains "MIR_Structure_As" which points to the
NT for Enumeration.  The NT for Enumeration was created by the compiler as
it read-in the "builtin_types.dat" file which defines all DataConstructs.

* The first call to "mir_lookup_data_construct()" (with OID for "FOOBAR")
  should return MS1_DC_BUILTUP and an "ident" block containing only name
  "OnOff" and code "3" defining the datatype instance created by the user's
  TYPE statement.  A context block is returned from this call which contains
  opaque info needed by "mir_eval_data_construct()" in order to return the
  dataconstruct definition underlying "OnOff".  The opaque data is simply
  the "src" mandle setup to point to the NT for the TYPE statement and "trg"
  setup to point to the NT containing the definition and template for
  Enumeration.  At this point, the user only knows that FOOBAR has a value of
  datatype named "OnOff" (Code 3) that is defined in terms of something else.

* A call to "mir_eval_data_construct()" using the context block above returns
  MS1_DC_BUILTIN_TMPLT and an ident block containing all the information
  from the NT for Enumeration.  Now the user "knows" what the bottom-most
  underlying dataconstruct is (Enumeration), and it also knows that the
  instance of this datatype (OnOff) contains additional information needed
  to fully define an instance of this underlying datatype (enumeration).
  This is indicated by "_TMPLT" in the return status.

The compiler and these Tier 1 functions support a generalized mechanism for
returning the additional information for dataconstructs that need it (like
ENUMERATION, RECORD, INTEGER[with range], etc).

In the example above, in the case of enumeration, the additional
information that must be stored *in the instance* is the essential
information in each "name = code" pair ("Off = 0", "On = 1").  Two
special MIR Relationships are used by the compiler to store this
information.

Typically (for all datatypes supported by the compiler so far), all
additional information for dataconstructs requiring such can be
recorded by using special MIR Relationships *in a repeating pattern*.
Note in the example that the repeating pattern is:

             MIR_Enum_Code
             MIR_Enum_Text

Furthermore, according to this convention, the compiler records the 'pattern'
inside the Non-Terminal for the underlying Dataconstruct (in this case
Enumeration) by using the "MIR_DC_SMI_Template" relationship.  You note that
the Non-Terminals for MIR Relationships "MIR_Enum_Code" and "MIR_Enum_Text"
are pointed to 'in-order' by the instances of MIR_DC_SMI_Template in the
Enumeration Non-Terminal.  All the MIR relationships pointed to by
"MIR_DC_SMI_Template" constitue 'the template' for that dataconstruct.

In this way, the compiler records all the information needed to "read-out"
all the additional information (for dataconstructs that require it) in a
standard way.

This function (mir_eval_builtin_template()) is called to 'read-out' this
additional template-defined information from the *instance* Non-Terminal
using template information from the underlying *data-construct* Non-Terminal.

Here's how mir_eval_builtin_template() works (the first time and each
succeeding time):

   * Search the "trg" Non-Terminal for the (next) occurence of the
     "MIR_DC_SMI_Template" relationship.

   * Take the Non-Terminal returned from the search and use it to search
     for the (next) occurrence of that Relationship in the "src" Non-Terminal.

   * Return whatever was found with the proper status code indicating
     string/number.

In the example above, the first call to mir_eval_builtin_template() will
cause "MIR_Enum_Code" to be returned from the search of "trg", and when used
to search "src" will return "0" (number) as the first Enum-Code.  The second
call will cause "MIR_Enum_Text" to be returned from "trg", and that will
cause "Off" (string) to be returned from the search of "src".  At this point,
the full "template" (as described by the MIR_DC_SMI_Template entries in
the Enumeration NT) has been used once. 

On the third call, the search of "trg" for MIR_DC_SMI_Template fails and
mir_eval_builtin_template() simply "resets" the "trg" mandle and tries again.
This starts the pattern over again.  In this example, "MIR_Enum_Code" is
returned and the second Code ("1") is read-out of the "src" mandle's NT.

Eventually (on the fifth call), the search of the "src" mandle fails.
mir_eval_builtin_template() signals this failure to the caller via the
"restart" argument and resets the "src" mandle and starts over, returning
"0" (number) and "restart = TRUE".

In this way, these Tier 1 functions can 'return' any arbitrary additional
information needed.

Note that in the example above, the additional template information being
returned is always a Terminal: number or string.  In the case of representing
RECORD dataconstructs, the repeating template pattern described by the
MIR_DC_SMI_Template entries describes *one* field, and in the RECORD
Non-Terminal we have:

             MIR_Field_Name
             MIR_Field_Code
             MIR_Structured_As

When these MIR Relationships are searched-for in the "src", the first two
return 'string' and 'number' respectively (the field name and field code).
However the third will return (essentially) a Non-Terminal describing *another*
datatype (for that field of the record).

In this case, mir_eval_builtin_template() returns a new context block (via
argument "o_context") and information in "ident".  Status code MS1_DC_BUILTIN
or MS1_DC_BUILTUP indicates what to do next to evaluate this field datatype.
(The caller is essentially in the same position as s/he is after calling
 mir_lookup_data_construct() and getting these return codes).

-------------------------------------------------------------------------------
With V2.00, the generalized mechanism is extended (in a general way) to cover
the very special case of returning the "Case Codes" in situations where
we're 'reading-out' the fields in a Variant Record datatype:

Supporting definition:
	TYPE Source = 3 (
		Local File = 1,
		Down Line Load = 2);

Variant Record:
	TYPE ScriptLocation = 5 RECORD
		Source = 1 : Source;
		CASE Source OF
		    [1] :
		    	FileName = 2 :  FileSpec;
		    [2,3] :
		    	Circuit = 3 : LocalEntityName;
		    	(* The type of Destination Address changed to match
		    		NETMAN *)
		    	DestinationAddress = 4 : ID802;
		    	SoftwareID = 6 : Latin1String;
		END CASE;
		END RECORD;

Variant-Records create the following challenges for the existing generalized
'read-out' mechanism embedded in these Tier 1 functions:

1) We must somehow delimit the fields in one 'case group' (e.g. field
  "FileName") from the fields in any other 'case group' (e.g. fields
  "Circuit", "DestinationAddress" and "SoftwareID").

2) We must somehow return one or more "case group codes" (e.g. "2" and "3" for
  the second 'case group') in a manner that associates them with the right
  'case group' fields.

By gently expanding the generalized read-out convention established between
the compiler and these Tier 1 functions (and by allowing the compiler to
create a 'fake' Builtin-Template 'dataconstruct' just for Variant-Record
'field groups') we accomplish both goals above without breaking anything else.

The example TYPE statement shown above appears in the Test Suite "tglobal.ms".
(Slightly modified here: the case group codes "[2, 3]" above is only "[2]"
 in the file... dump below is modified to reflect the example above)

Here is the corresponding portion of the MIRC dump for this dataconstruct:

(NT for Builtin-DataConstruct "Enumeration")
7701: (Contents 0x8)   OID Count...0   Rel-Entry Count...8
7702: 10 MIR_DC_Found_In_SMI    [2687]	'1'
7703: 14 MIR_DC_SMI_Code        [2696]	'10'
7704: 13 MIR_DC_SMI_Name        [3135]	"BIDT_ENUMERATION"
7705: 17 MIR_DC_MCC_DT_Size     [2804]	'65535'
7706: 11 MIR_DC_ASN1_Class      [2686]	'0'
7707: 12 MIR_DC_ASN1_Tag        [2688]	'2'
7708: 15 MIR_DC_SMI_Template    [13096]
7709: 15 MIR_DC_SMI_Template    [13099]

(NT for Builtin-DataConstruct "Variant Record")
7728: (Contents 0x9)   OID Count...0   Rel-Entry Count...9
7729: 10 MIR_DC_Found_In_SMI    [2687]	'1'
7730: 14 MIR_DC_SMI_Code        [2700]	'14'
7731: 13 MIR_DC_SMI_Name        [3161]	"BIDT_VRECORD"
7732: 17 MIR_DC_MCC_DT_Size     [2804]	'65535'
7733: 11 MIR_DC_ASN1_Class      [2686]	'0'
7734: 12 MIR_DC_ASN1_Tag        [2702]	'16'
7735: 15 MIR_DC_SMI_Template    [13102]      (Rel: "MIR_Field_Name")
7736: 15 MIR_DC_SMI_Template    [13105]      (Rel: "MIR_Field_Code")
7737: 15 MIR_DC_SMI_Template    [13066]      (Rel: "MIR_Structured_As")

(NT for user TYPE statement defining "Source")
8320: (Contents 0x7)   OID Count...0   Rel-Entry Count...7
8321: 02 MIR_ID_Code            [2689]	'3'
8322: 04 MIR_Text_Name          [6935]	"Source"
8323: 08 MIR_Structured_As      [7701]	NT(BIDT_ENUMERATION)
8324: 18 MIR_Enum_Code          [2687]	'1'
8325: 19 MIR_Enum_Text          [5443]	"Local File"
8326: 18 MIR_Enum_Code          [2688]	'2'
8327: 19 MIR_Enum_Text          [4755]	"Down Line Load"

  (This is the Non-Terminal defining an instance of a VARIANT-Record named
   "ScriptLocation")
  8328: (Contents 0xc)   OID Count...0   Rel-Entry Count...12
  8329: 02 MIR_ID_Code            [2691]	'5'
  8330: 04 MIR_Text_Name          [6888]	"ScriptLocation"
  8331: 08 MIR_Structured_As      [7728]	NT(BIDT_VRECORD)
  .....Definition of Variant-Record's Fixed Field:
  8332: 20 MIR_Field_Name         [6935]	"Source"
  8333: 21 MIR_Field_Code         [2687]	'1'
  8334: 08 MIR_Structured_As      [8320]	NT(Source)
  .....Definition of First Field Group:
  8335: 20 MIR_Field_Name         [2806]	""
  8336: 21 MIR_Field_Code         [2686]	'0'
  8337: 08 MIR_Structured_As      [8341]	NT(Source)
  .....Definition of Second Field Group:
  8338: 20 MIR_Field_Name         [2806]	""
  8339: 21 MIR_Field_Code         [2686]	'0'
  8340: 08 MIR_Structured_As      [8349]	NT(Source)

    (NT representing the First Field Group)
    8341: (Contents 0x7)   OID Count...0   Rel-Entry Count...7
    8342: 02 MIR_ID_Code            [2687]	'1'
    8343: 04 MIR_Text_Name          [6935]	"Source"
    8344: 08 MIR_Structured_As      [7738]	NT(BIDT_CASE_FIELD_GROUP)
    8345: 22 MIR_Case_Code          [2687]	'1'
    8346: 20 MIR_Field_Name         [4893]	"FileName"
    8347: 21 MIR_Field_Code         [2688]	'2'
    8348: 08 MIR_Structured_As      [7995]	NT(FileSpec)

    (NT representing the Second Field Group)
    8349: (Contents 0xd)   OID Count...0   Rel-Entry Count...13
    8350: 02 MIR_ID_Code            [2687]	'1'
    8351: 04 MIR_Text_Name          [6935]	"Source"
    8352: 08 MIR_Structured_As      [7738]	NT(BIDT_CASE_FIELD_GROUP)
    8353: 22 MIR_Case_Code          [2688]	'2'
    83xx: 22 MIR_Case_Code          [2689]	'3'
    8354: 20 MIR_Field_Name         [3396]	"Circuit"
    8355: 21 MIR_Field_Code         [2689]	'3'
    8356: 08 MIR_Structured_As      [7967]	NT(LocalEntityName)
    8357: 20 MIR_Field_Name         [4710]	"DestinationAddress"
    8358: 21 MIR_Field_Code         [2690]	'4'
    8359: 08 MIR_Structured_As      [7981]	NT(ID802)
    8360: 20 MIR_Field_Name         [6931]	"SoftwareID"
    8361: 21 MIR_Field_Code         [2692]	'6'
    8362: 08 MIR_Structured_As      [7932]	NT(Latin1String)

(NT representing the compiler's "faked"-up Builtin-DataConstruct representing
 a "Case Field Group":)
7738: (Contents 0x9)   OID Count...0   Rel-Entry Count...9
7739: 10 MIR_DC_Found_In_SMI    [2687]	'1'
7740: 14 MIR_DC_SMI_Code        [2686]	'0'
7741: 13 MIR_DC_SMI_Name        [3128]	"BIDT_CASE_FIELD_GROUP"
7742: 11 MIR_DC_ASN1_Class      [2686]	'0'
7743: 12 MIR_DC_ASN1_Tag        [2702]	'16'
7744: 16 MIR_DC_SMI_OTTemplate  [13108]         (Rel: "MIR_Case_Code")
7745: 15 MIR_DC_SMI_Template    [13105]         (Rel: "MIR_Field_Code")
7746: 15 MIR_DC_SMI_Template    [13102]         (Rel: "MIR_Field_Name")
7747: 15 MIR_DC_SMI_Template    [13066]         (Rel: "MIR_Structured_As")


Examine the NT @ 8328 representing the instance of a Variant-Record.  You
can see these things:

   * The compiler records the fixed-field(s) in a Variant-Record the same
     way it does for non-Variant Records.  The Tier 1 functions will read
     fixed-fields out of Variant-Records in exactly the same way they do
     for Non-Variant Records.  For repeated calls to mir_eval_builtin_template
     first you get the field name (string), then the field code (number)
     and then a BUILTIN/BUILTUP status with a new context block and ident
     block.

   * The compiler records a *GROUP* of variant fields as though they were
     a single fake-field in the Variant-Record, of datatype
     "BIDT_CASE_FIELD_GROUP".  The 'fake-field' has name of "" and code of 0.
     When reading out fields in a Variant-Record, the caller's clues that
     a group of variant-fields has been encountered are:

         - Field name of ""
         - Field code of 0
         - BUILTIN datatype of name "BIDT_CASE_FIELD_GROUP".


To get the fields in the group 'read-out', you submit the output context
block for processing just the way you would for a non-Variant Record field.
However, in processing the stream of numbers and strings defining the fields
in the group, you have to handle the reception of the case codes which would
*not* normally appear in the 'read-out' for a fields in a non-Variant Record.

Case Codes:
-----------

Examine the NT @ 7738 (for the Builtin-DataConstruct for Case Field Group).
It is this Non-Terminal DataConstruct that provides the 'templates' for reading
out the additional information about the "instance" of a Field Group (the
Non-Terminals at 8341 and 8349 for each Field Group respectively).

Note that preceding the "MIR_DC_SMI_Template" entries is a new relationship
"MIR_DC_SMI_OTTemplate".  This is the "One-Time Template" relationship used
to specify the MIR Relationship needed to extract Case Codes from the
'instance' Non-Terminal.  It is "One-Time" because it does not participate
in the 'pattern' of running down the "MIR_DC_SMI_Template" relationships
over and over again.

The generalized read-out mechanism of mir_eval_builtin_template() is expanded
to check for the relationship described by "MIR_DC_SMI_OTTemplate" in the
"src" (instance) Non-Terminal.  If the search succeeds, then the target(s)
are repeatedly returned to the caller (these will be the Case Codes given the
dump above).  Once the search of "src" fails (using the MIR relationship
provided via "MIR_DC_SMI_OTTemplate"), then processing by
mir_eval_builtin_template() proceeds as described above: repeatedly 'reading-
out' the pattern of stuff stored in the "src" (instance) using the templates
provided in the "trg" DataConstruct NT:

   * If One-Time Template mandle pointer in context block is non-NULL
         Attempt a search of "src" from the last place for a hit using
         the One-Time Template mandle relationship.

         If a hit occurred
             return the hit target (a Terminal (Case Code only), as of V2.00)
         else
             free the One-Time Template mandle, making pointer NULL
             reset the "trg" mandle
             reset the "src" mandle

   * Search the "trg" Non-Terminal for the (next) occurence of the
     "MIR_DC_SMI_Template" relationship.

   * Take the Non-Terminal returned from the search and use it to search
     for the (next) occurrence of that Relationship in the "src" Non-Terminal.

   * Return whatever was found with the proper status code indicating
     string/number.

This means that as you call mir_eval_builtin_template(), for fields in
a field group, *first* you get the case code(s), and when you get a string,
*that* is the name of the first field in the field group, it will be followed
by a *single* number, the code for the field.  Then field definitions for
all the other fields in the case field group follow: name/number, name/number.


MUCH MORE COMPLICATED SCENARIO
------------------------------

Consider the following scenario:

        TYPE HoleSize = 3 : Integer [2..100];

        TYPE RectangularHole = 4 : RECORD
                      HoleIsFilled = 0 : Boolean;
                      SizeOfHole = 1 : HoleSize;
                      END RECORD ;

        TYPE SquareHole = 5 : RectangularHole;

        ATTRIBUTE Steamshovel = 1 : SquareHole;

    On the first call to mir_lookup_data_construct() with the OID for the
    attribute we get:

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       5
        name:       "SquareHole"

    "context"
        is loaded with opaque information needed to obtain information about
        SquareHole's definition: "RectangularHole".

    Passing "context" (and another pointer to an "ident" block) to
    mir_eval_data_construct(), MS1_DC_BUILTUP is returned and we get:

    "ident"
        smi:        <not returned>
        asn1_tag:   <not returned>
        asn1_class: <not returned>
        code:       4
        name:       "RectangularHole"

    "context"
        is loaded with opaque information needed to obtain information about
        RectangularHole's definition: "RECORD".


    If the context block returned from the last call in the example above
    (containing opaque information describing RectangularHole's definition)
    is submitted again to mir_eval_data_construct(), then MS1_DC_BUILTIN_TMPLT
    is returned with:

    "ident"
        smi:        MIR_SMI_DNA  (this is an enumerated value)
        asn1_tag:   16
        asn1_class: 0
        code:       203
        name:       "RECORD"

    "context"
        is loaded with opaque information needed to obtain information about
        how to interpret the template (for use in a call to
        mir_eval_builtin_template() ).

At this point, the caller to mir_eval_builtin_template() has recognized the
fact that we are dealing with a record (preferably by recognizing the asn1_tag
and class as indicating this, rather than the "name" value of "RECORD" which
is NOT architected).  The caller must "know" that for each successive call
made to mir_eval_builtin_template() (using the context that comes back from
mir_eval_data_construct()) returns certain information about each field
in the record.  Three successive calls to mir_eval_builtin_template() (passing
in the same input context block) returns all the information about (the next)
field in the record:

   Call #   Returns
      1         Field Code (as a number)
      2         Field Name (as a string)
      3         Field Data-Construct
                  (either in the ident block if the field is defined by a
                   builtin "atomic" data-construct (like Boolean) or
                   a newly created context block if the field is built-up
                   (through the use of a TYPE statement) or builtin-template
                   (like another RECORD).

The things that are returned (for any builtin-template type data-construct),
and their order are defined by the order that the optional relationship names
(that describe that template) are entered into the "builtin_types.dat" file
line for that builtin-template.  (This file is read by the compiler at
compilation time.)

Given the scenario above, repeated calls to this function return the
following codes and values:
                                                      ----------Ident---------
Call Return           Number  String       Context    SMI TAG CLS CODE NAME
 1   MS1_TMPLT_NUMBER   0        -            -       -    -   -    -
 2   MS1_TMPLT_STRING   -   "HoleIsFilled"    -       -    -   -    -
 3   MS1_DC_BUILTIN     -        -            -       DNA  1   0    1  BOOLEAN

 4   MS1_TMPLT_NUMBER   1        -            -       -    -   -    -
 5   MS1_TMPLT_STRING   -    "SizeOfHole"     -       -    -   -    -
 6   MS1_DC_BUILTUP     -        -        <returned>  -    -   -    3  Holesize

 7   MS0_FIND_NONE

The MS0_FIND_NONE signals the fact that the template evaluation has been
completed.

Note that this particular RECORD ("RectangularHole") happens to have two
fields.  For each added field in a record, another three calls are required
to mir_eval_builtin_template() to retrieve all the information about that
field.

At any point that a context block is returned, it may be submitted to
mir_eval_data_construct() to extract information about that next level down
definition of the field.  In this example, submitting the context block
returned in call 6 above to mir_eval_data_contruct() causes the following
return:

mir_eval_data_construct() Return Status: MS1_DC_BUILTIN_TMPLT

    "ident"
        smi:        MIR_SMI_DNA ( this is enumerated value)
        asn1_tag:   2
        asn1_class: 0
        code:       2
        name:       "Integer"

    "context"
        is loaded with opaque information needed to obtain information about
        how to interpret the template (for use in a call to
        mir_eval_builtin_template() ).

Again, at this point the caller has recognized that it is an Integer, and he
must "know" how to interpret what comes back from repeated calls to
mir_eval_builtin_template() when this new context is passed in:

     Call       Returns
      1         Integer Minimum Value (as a number)
      2         Integer Maximum Value (as a number)

Given the scenario above, repeated calls to this function using the context
block generated by mir_eval_data_construct() returns the following codes and
values:
                                                      ----------Ident---------
Call Return           Number  String       Context    SMI TAG CLS CODE NAME
 1   MS1_TMPLT_NUMBER   2        -            -       -   -   -    -
 2   MS1_TMPLT_NUMBER   100      -            -       -   -   -    -

 3   MS0_FIND_NONE

Since an integer will never have more than two range values (and should have
both), the third call that returns MS0_FIND_NONE is superfluous.

For more insight, use the "QIM" program to obtain a dump of the MIR binary
data base and examine the data-construct tree structure for something that
has a "value" (like an ATTRIBUTE).  Find the relationship "MIR_Structure_As" in
the relationship table of the ATTRIBUTE (this corresponds to doing a
mir_lookup_data_construct() call).

* If the ATTRIBUTE's value was defined in the MSL file as something directly
  out of the SMI (like "Boolean"), this will be the "end of the line", (and
  mir_lookup_data_construct() would return MS1_DC_BUILTIN).

* If it was defined in terms of something else in the MSL file (a
  "user-defined" TYPE) or in terms of a "builtin (to the SMI) template" (like
  an enumeration), then mir_lookup_data_construct() returns MS1_DC_BUILTUP
  and repeated calls to mir_eval_data_construct() and
  mir_eval_builtin_template() are required to fully reveal the structure of
  the ATTRIBUTE's value.
-------------------------------------------------------------------------------
With the advent of V2.00, upon calling this function after "restart" has
been returned TRUE, you don't get the target(s) of "MIR_DC_SMI_OTTemplate"
(case-codes) again.

Currently under V2.00, this logic only handles one instance of
"MIR_DC_SMI_OTTemplate" in the target Non-Terminal.
*/

{
mir_status      status;         /* Local status code holder               */
mir_status      s_status;       /* Local search status code holder        */
mir_value       m_value;        /* Terminal value structure               */
mandle          *local=NULL;    /* Local mandle, may be saved or released */


/* assume restart is FALSE */
*restart = FALSE;

/*
| if (context block pointer is NULL or ident block is NULL or string is 
|     NULL or Number is NULL)
*/
if (i_context == NULL || ident == NULL || number == NULL || string == NULL 
    || o_context == NULL || *o_context != NULL) {
   return (MS1_INVALID_ARG);
   }

/*
| This clause supports One-Time Template 'read-out' of Terminals.
|
| if (one-time-template mandle is non-NULL)
*/
if ((*i_context)->ott != NULL) {

    /* attempt FROM-LAST search on input context src mandle using ott mandle */
    s_status =
        mir_search_rel_table(SEARCH_FROMLAST,
                             (*i_context)->src,      /* Search this          */
                             (*i_context)->ott,      /* ..for this           */
                             &(*i_context)->cclass,  /* m-class to ues       */
                             &local,                 /* (should not be used) */
                             &m_value);              /* Terminal rtned here  */


    /* switch (return status) */
    switch (s_status) {

        case MS0_FIND_NONE:
            /* reset search on input context src mandle */
            mir_reset_search((*i_context)->src);
            /* free the one-time template mandle in the context block */
            if ((status = mir_free_mandle(&((*i_context)->ott)))
                != MS_SUCCESS) {
                return(status);
                }
            break;

        case MS0_FIND_TERM:
            /* if (value returned is STRING) */
            if (IS_MV_STRING((&m_value))) {

                /* return address of string to caller */
                *string = MV_STRING((&m_value));
                return (MS1_TMPLT_STRING);
                }
            else if (IS_MV_NUMBER((&m_value))) { /* value returned is NUMBER */
                /* copy number back to caller */
                *number = MV_SNUMBER((&m_value));
                return (MS1_TMPLT_NUMBER);
                }
            /* FALL THRU   FALL THRU   FALL THRU */
            /* FALL THRU   FALL THRU   FALL THRU */
            /* FALL THRU   FALL THRU   FALL THRU */

        default:
            return (MS1_DC_NT_CORRUPT);
        }
    }

/*
| attempt FROM-LAST search on input context target mandle for
| MIR_DC_SMI_Template.
*/
s_status = mir_search_rel_table(SEARCH_FROMLAST,
                                (*i_context)->trg,      /* Search this      */
                                m_mir_dc_smi_template,  /* ..for this       */
                                &local_class,           /* m-class to ues   */
                                &m_scanner,             /* use this on ret  */
                                &m_value);              /* (this should not */
                                                        /* get used)        */
  
/* if (attempt returns MS0_FIND_NONE) */
if (s_status == MS0_FIND_NONE) {

    /* attempt FROM-TOP search on input context target mandle for */
    /* MIR_DC_SMI_Template                                        */
    s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                (*i_context)->trg,      /* Search this      */
                                m_mir_dc_smi_template,  /* ..for this       */
                                &local_class,           /* m-class to ues   */
                                &m_scanner,             /* use this on ret  */
                                &m_value);              /* (this should not */
                                                        /* get used)        */

    /* if (attempt returns MS0_FIND_NONE) */
    if (s_status == MS0_FIND_NONE) {
        return (MS0_FIND_NONE);
        }
    }

/* if (attempt did not return MS0_FIND_NONTERM) */
if (s_status != MS0_FIND_NONTERM) {
    return (s_status);
    }

/* attempt FROM-LAST search on input context src mandle for NT just returned.
|
|   Mandle Considerations: we force the create of a mandle ("local") to cover
|   the case where it turns out the thing being returned is another
|   Non-Terminal.  See below.
*/
s_status = mir_search_rel_table(SEARCH_FROMLAST,
                                (*i_context)->src,      /* Search this      */
                                m_scanner,              /* ..for this       */
                                &(*i_context)->cclass,  /* m-class to ues   */
                                &local,                 /* use this on ret  */
                                &m_value);              /* (this should not */
                                                        /* get used)        */

switch (s_status) {

    case MS0_FIND_TERM:
        /* if (value returned is STRING) */
        if (IS_MV_STRING((&m_value))) {

            /* return address of string to caller */
            *string = MV_STRING((&m_value));
            return (MS1_TMPLT_STRING);
            }
        else if (IS_MV_NUMBER((&m_value))) { /* value returned is NUMBER */
            /* copy number back to caller */
            *number = MV_SNUMBER((&m_value));
            return (MS1_TMPLT_NUMBER);
            }
        return (MS1_DC_NT_CORRUPT);


    case MS0_FIND_NONTERM:
        /*
        | "local" has been set to point to the mandle for the Non-Terminal
        |  returned.  This happens when m_scanner (above) turns out to be the
        |  Non-Terminal for the relationship MIR_Structured_As.  If the 'local'
        |  Non-Terminal returned represents something defined in the SMI (it
        |  can only be a "builtin" or "builtup" data construct, never a
        |  "builtin-template" in this point), then we'll need to insert the
        |  mandle for it into a newly created context block if it is "builtup",
        |  otherwise we can just release it after copying all the goodies out
        |  of it ("builtin").
        |
        |  The reason "local" cannot point to a 'builtin-template' type of
        |  DataConstruct is that all 'builtin-template' Non-Terminals are
        |  always *first* referenced via the TYPE statement that makes
        |  reference to them.  Consequently we'll have to 'go thru' the NT
        |  that stands for that TYPE statement: hence it'll be "BUILTUP" first.
        |
        |  load all data-construct info to ident block using mandle to
        |  non-terminal returned
        */
        if ((status = load_ident(local, ident)) != MS_SUCCESS) {
            return(status);
            }

        /*
        | if (SMI code is present)  ---this implies "BUILTIN" */
        if (ident->b_smi == TRUE) {

            /* release the local mandle */
            if ((status = mir_free_mandle(&local)) != MS_SUCCESS) {
                return (status);
                }

            return (MS1_DC_BUILTIN);
            }

        /* if (attempt to allocate a context block failed) */
        if ((*o_context = (dc_context *) malloc (sizeof(dc_context)) ) == NULL) {
            return (MS_NO_MEMORY);
            }

        /* copy local mandle of non-terminal to context block as src mandle */
        (*o_context)->src = local;
        (*o_context)->cclass = (*i_context)->cclass; /* Use same class      */
        (*o_context)->trg = NULL;                    /* Create fresh mandle */
        (*o_context)->ott = NULL;   /* Init just as mir_lookup_d_c() would  */

        /* attempt search on non-terminal for MIR_Structured_As */
        s_status = mir_search_rel_table(SEARCH_FROMTOP,
                                  local,                /* Search this      */
                                  m_mir_structured_as,  /* ..for this       */
                                  &(*o_context)->cclass,/* m-class to ues   */
                                  &(*o_context)->trg,   /* use this on ret  */
                                  &m_value);            /* (this should not */
                                                        /* get used)        */

        /* if (status was NOT MS0_FIND_NONTERM) */
        if (s_status != MS0_FIND_NONTERM) {
            return (MS1_DC_NT_CORRUPT);
            }
        /* copy returned mandle non-terminal to context block as trg mandle **
        |  (Done as part of function of mir_search_rel_table() call above)
        */
        return (MS1_DC_BUILTUP);

    case MS0_FIND_NONE:
        /*
        | (V1.5 - This is new logic, we used to just return this code)
        | perform "reset" on the source mandle in context block
        | (V2.00 - Whoa!  Bug!  This isn't working right!  We also need to
        | reset the target mandle too!
        */
        mir_reset_search((*i_context)->src);
        mir_reset_search((*i_context)->trg);

        /* recursively invoke this function */
        s_status = mir_eval_builtin_template(i_context, number, string,
                                             o_context, ident, restart);
        /* set "restart" argument TRUE */
        *restart = TRUE;

        /* return status */
        return (s_status);

    default:
        return (s_status);
    }
}

/* mir_copy_context - Create a copy of a context block */
/* mir_copy_context - Create a copy of a context block */
/* mir_copy_context - Create a copy of a context block */

mir_status mir_copy_context (context, new_context)

dc_context     *context;        /*  Ptr to context block to release */
dc_context     **new_context;   /* Addr of ptr to new block         */

/*
INPUTS:

    "context" is the address of a context block to be copied.

    "new_context" contains the address of a pointer to the newly created copy
    of the original context block on return.


OUTPUTS:

The function returns one of:

    MS_SUCCESS - Context block was successfully copied, and the
                 supplied pointer is set to point to the new copy

    * Any erroneous return status returned by mir_copy_mandle().


BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user of the Tier 1 Functions:

        mir_lookup_data_construct()
        mir_eval_data_construct()
        mir_eval_builtin_template()

        ..and there exists as a consequence of one or more calls to the
        functions listed above a "context" block that the caller
        wants to duplicate before changing.

    Purpose:
        This function creates a new context block with the same information
        in it as the original.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (context block pointer is already NULL)
        <set new block pointer NULL>
        <return MS_SUCCESS>

    if (allocation of src mandle failed)
        <return status>

    if (allocation of target mandle failed)
        <return status>

    if (ott mandle pointer is not NULL)
        if (allocation of ott mandle failed)
            <return status>

    if (allocation of new context block failed)
        <return MS_NO_MEMORY>

    <copy src, trg, ott  and mandle class to new context block>
    <return pointer to new context block>

    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

    There may be many context blocks extant at any given moment.  Context
    blocks may be re-used by the Tier 1 user by re-submitting an "old" one
    to mir_lookup_data_construct().  Eventually however, all must be
    returned ("dissolved"), use mir_dissolve_context() to do this.

*/

{
mir_status      status;         /* Status from copy mandle call            */
mandle          *source=NULL;   /* Temporary storage for source mandle ptr */
mandle          *target=NULL;   /* Temporary storage for target mandle ptr */
mandle          *ott=NULL;      /* Temp. for one-time template mandle ptr  */


/* if (context block pointer is already NULL) */
if (context == NULL) {
    /* set new block pointer NULL */
    *new_context = NULL;
    return (MS_SUCCESS);
    }

/* if (allocation of src mandle failed) */
if ( (status = mir_copy_mandle(context->src, &source)) != MS_SUCCESS) {
    return (status);
    }

/* if (allocation of target mandle failed) */
if ( (status = mir_copy_mandle(context->trg, &target)) != MS_SUCCESS) {
    return (status);
    }

/* if (allocation of ott mandle failed) */
if (context->ott != NULL) {
    if ( (status = mir_copy_mandle(context->ott, &ott)) != MS_SUCCESS) {
        return (status);
        }
    }

/* if (allocation of new context block failed) */
if ((*new_context = (dc_context *) malloc (sizeof(dc_context)) ) == NULL) {
    return (MS_NO_MEMORY);
    }

/* copy src, trg and mandle class to new context block */
(*new_context)->src = source;
(*new_context)->trg = target;
(*new_context)->ott = ott;
(*new_context)->cclass = context->cclass;

/* return pointer to new context block  */
/* (Done above during allocate)         */

return (MS_SUCCESS);

}

/* mir_dissolve_context - Release Storage for Context Block */
/* mir_dissolve_context - Release Storage for Context Block */
/* mir_dissolve_context - Release Storage for Context Block */

mir_status mir_dissolve_context (context)

dc_context     **context;   /*  Ptr to context block to release */

/*
INPUTS:

    "context" contains the address of a pointer to the context block to be
              released.

OUTPUTS:

The function returns one of:

    MS_SUCCESS - Context block was successfully dissolved, and the
                 supplied pointer is NULLed.

    * Any erroneous return status returned by mir_free_mandle().


BIRD'S EYE VIEW:
    Context:
        The caller is a MIR user of the Tier 1 Functions:

        mir_lookup_data_construct()
        mir_eval_data_construct()
        mir_eval_builtin_template()

        ..and there exists as a consequence of one or more calls to the
        functions listed above a "context" block that is no longer needed.

    Purpose:
        This function releases the resources associated with a context
        block generated through a call to one of the functions above.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (context block pointer is already NULL)
        <return MS_SUCCESS>

    if (attempt to release source mandle failed)
        <return the failure status>

    if (attempt to release target mandle failed)
        <return the failure status>

    if (ott mandle pointer is not null)
        if (attempt to release ott mandle failed)
            <return the failure status>

    <deallocate context block storage>

    <return a NULL to the pointer provided>

    <return MS_SUCCESS>


OTHER THINGS TO KNOW:

    There may be many context blocks extant at any given moment.  Context
    blocks may be re-used by the Tier 1 user by re-submitting an "old" one
    to mir_lookup_data_construct().  Eventually however, all must be
    returned ("dissolved") and this is the function to do that job.

    All mandles may potentially belong to the same mandle-class.  When the
    last mandle in a mandle class is freed, the Tier 0 function that manages
    this will also free "the class" too, so we don't have to do it explicitly.

*/

{
mir_status      status;         /* Status from free call */


/* if it is already gone, we're done! */
if (*context == NULL) {
    return (MS_SUCCESS);
    }

/* if (attempt to release source mandle failed) */
if ((status = mir_free_mandle(&((*context)->src))) != MS_SUCCESS) {
    return(status);      /* return the failure status */
    }

/* if (attempt to release target mandle failed> */
if ((status = mir_free_mandle(&((*context)->trg))) != MS_SUCCESS) {
    return(status);      /* return the failure status */
    }

/* if (ott mandle pointer is not null) */
if ((*context)->ott != NULL) {
    if ((status = mir_free_mandle(&((*context)->ott))) != MS_SUCCESS) {
        return(status);      /* return the failure status */
        }
    }

/* deallocate context block storage */
free(*context);

/* NULL-out the pointer */
*context = NULL;

return (MS_SUCCESS);

}
