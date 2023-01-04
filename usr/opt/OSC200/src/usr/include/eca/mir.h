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
/*
 * @(#)$RCSfile: mir.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 17:48:32 $
 */
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
 * Module MIR.H
 *      Contains data structure definitions required by MIR Tier 0 Interface
 *      functions, with conditionally assembled section at the end for
 *      use by MIR Compiler modules.
 *
 * WRITTEN BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   November 1990
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *      This module is included into the compilations of modules that
 *      invoke the MIR Tier 0 & Tier 1 Interface Functions.
 *
 *    Purpose:
 *       This module contains the data structure definitons required by the
 *       users of the MIR Tier 0 Interface Functions.
 *
 * History
 *   Version    Date            Who             What
 *      V0.0    December 1990   D. D. Burns     Original Version (Bridge)
 *      V1.0    February 1991   D. D. Burns     Compiler Release V1.0
 *      V1.4    April 1991      D. D. Burns     Minor Enhancements, bug fixes
 *      V1.5    June 1991       D. D. Burns     Merged "mir_compiler.h" into
 *                                               this file, added support for
 *                                               multiple SMIs and in-line
 *                                               Object IDs (in MSLs).
 *      V1.6    Sept 1991       D. D. Burns     Added support for "IDENTIFIER="
 *                                               clause
 *      V1.7    October 1991    D. D. Burns     Removed code MSO_FIND_LONGEST
 *      V1.8    December 1991   D. D. Burns     Added mutex protection macros
 *                                               for protecting mandle/class
 *                                               lists. Support is not complete
 *      V1.90   July 1992       D. D. Burns     Preliminary MCC-syntax
 *                                               conversion
 *      V1.98   Oct 1992        D. D. Burns     Full MCC support except for
 *                                               "Augment"
 *      V1.99   Oct 1992        D. D. Burns     Support 'AUGMENT' and 'MERGE'
 *      V2.00   Jan 1993        D. D. Burns     Final Delivery, V2.0
 */

/* Module Overview: 
|
|   This file contains definitions for the C-structures and datatypes used by
|   the Tier 0 & Tier 1 Interface Functions of the Common Agent Management
|   Information Repository.
|
*/

/*
| Support for C++
*/
#if defined(__cplusplus)
extern "C" {
#endif


/*
|==============================================================================
| MP(func, msg)
|
| The "Message Phrase" Macro allows for internationalization by calling
| the "func" argument (for internationalization) or expanding to just "msg"
| if "NL" is not defined
|
|==============================================================================
*/
#ifdef NL               
#define MP(func, msg)  func()
#else
#define MP(func, msg)  msg
#endif


/*
|==============================================================================
| Define a symbol to distinguish the compilers that allow argument lists in
| prototypes from those that don't.  Then define a macro to conditionalize
| prototype argument lists.  Note that two sets of parentheses are required.
| Example: char *f_foobar PROTOTYPE ((int *arg1, char arg2));
|==============================================================================
*/
#ifdef VMS
#define PROTOTYPE_ALLOWED
#endif
#ifdef mips
#define PROTOTYPE_ALLOWED
#endif

#ifdef PROTOTYPE_ALLOWED
#define PROTOTYPE(args) args
#else
#define PROTOTYPE(args) ()
#endif


/*
|==============================================================================
| Define "Good Exit" and "Bad Exit" status codes according to platform.
|
| Wherever "exit()" is called, one of these codes should be used.
|==============================================================================
*/
#ifdef VMS
#define GOOD_EXIT 1
#define BAD_EXIT 0
#else
#define GOOD_EXIT 0
#define BAD_EXIT 1
#endif


/*
|==============================================================================
|
| MIR Sources are sensitive to the definition of symbol "MTHREADS", which
| when defined causes code to support multiple-threaded operation.
|
|  Support for this is NOT complete.  For the delivered "logically single-
|  threaded" SNMP PE, nothing special for RPC/threads needs to be changed about
|  the MIR, and indeed this symbol need not be defined.
|
|  Search for this symbol in the MIR sources to see places and comments
|  where changes will be needed.
|==============================================================================
*/
/* #define MTHREADS */


/*
|==============================================================================
|  LOCK_MANDLES() / UNLOCK_MANDLES()
|
|  These macros are used to acquire and release the mutex protecting the
|  lists of mandles and mandle classes in "mir_t0.c" by some functions in
|  that module.
|==============================================================================
*/
#ifdef MTHREADS
#define LOCK_MANDLES()                                                  \
/* if (acquire mutex for mandle lists FAILED) */                        \
if (pthread_mutex_lock(&mandle_lists_m) != 0) {                         \
        return(MS_MUTEX_ACQ_FAIL);                                      \
        }

#define UNLOCK_MANDLES()                                                \
/* if (release mutex for mandle lists FAILED) */                        \
if (pthread_mutex_unlock(&mandle_lists_m) != 0) {                       \
        return(MS_MUTEX_RLS_FAIL);                                      \
        }
#else

#define LOCK_MANDLES()
#define UNLOCK_MANDLES()

#endif  /* MTHREADS */

/*
|==============================================================================
|
|   NULL (if we need it)
|
|==============================================================================
*/
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef BOOL_DEF
/*
|==============================================================================
|
|   Primitive BOOLEAN type
|
|==============================================================================
*/
typedef
    int
        BOOL;
#define BOOL_DEF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#endif


#ifndef OID_DEF
/*
|==============================================================================
|  ISO Object ID
|
|   This structure is of the same format as the "object_id" structure in
|   the CA "man_data.idl" file.  Presumably this definition should be
|   removed when the MIR is integrated with the CA code.
|
|==============================================================================
*/
typedef
    struct {
        int                 count;
        unsigned int       *value;
        } object_id;
#define OID_DEF
#endif


#ifndef MASA_TYPE
/*
|   "MIR Address-Space Address" type
|
*/
typedef
    int
        masa;

#define MASA_TYPE
#endif


/*
|==============================================================================
|  Tier 0 & 1 Function Return Status
|
|   This is the complete list of return status codes that MIR Tier 0 & 1
|   functions may return.  Each routine may return only a subset of the
|   complete set listed below.  See the documentation for any given routine
|   to determine which from the list below apply.
|
|  NOTE: If you change this list, be sure to modify function "mir_error()"
|        in the Tier 0 module "mir_t0.c".
|==============================================================================
*/
typedef
    enum {
        /* Generic -- All Tiers */
        MS_NOT_IMPLEMENTED, /* Function not implemented yet                  */
        MS_NO_MEMORY,       /* Insufficient Memory available                 */
        MS_SUCCESS,         /* The operation succeeded                       */
        MS_FAILURE,         /* The operation failed                          */
        MS_MUTEX_INIT_FAIL, /* Failed to initialize a mutex                  */
        MS_MUTEX_ACQ_FAIL,  /* Failed to acquire a mutex                     */
        MS_MUTEX_RLS_FAIL,  /* Failed to release a mutex                     */

        /* Tier 0 Status */
        MS0_DBFILE_OPENFAIL,/* Unable to open MIR binary database file       */
        MS0_DBFILE_READFAIL,/* Unable to read MIR binary database file       */
        MS0_BAD_ENDIAN,     /* Invalid Endian-ness indicator                 */
        MS0_BAD_VERSION,    /* Invalid Format Version indicator              */
        MS0_BAD_ENV_VAR_VAL,/* Bad Environment Variable Value (Paging)       */
        MS0_DB_NOT_LOADED,  /* MIR Database File not loaded: No Init done yet*/
        MS0_INDEX_CORRUPTED,/* MIR Database File Index is corrupted          */
        MS0_NT_CORRUPTED,   /* MIR Database File Non-Terminals are corrupted */
        MS0_INVALID_OID,    /* A pointer to a required obj. id was null      */
        MS0_INVALID_MANDLE, /* A pointer to a Mandle passed on the call was  */
                            /*   invalid or the mandle was invalid           */
        MS0_FIND_NONE,      /* No object was found                           */
        MS0_FIND_EXACT,     /* Found an exact match                          */
        MS0_FIND_SHORT,     /* The supplied OID was too short to fully match */
                            /*   the entry found in the MIR                  */
        MS0_FIND_LONG,      /* The supplied OID was too long to fully match  */
                            /*   the longest non-terminal object index entry */
        MS0_FIND_ROLL,      /* No long nor short match, rolled to next object*/
        MS0_FIND_NONTERM,   /* A match was found and NonTerminal mandle rtned*/
        MS0_FIND_TERM,      /* A match was found and a Terminal was returned */
        MS0_INVALID_SMI,    /* OID SMI indicator code was not valid          */
        MS0_NOT_REL_MANDLE, /* Mandle is not a valid Relationship Mandle     */
        MS0_NO_PAGE_SLOTS,  /* Out of Page Slots to record a new page buffer */
        MS0_PAGE_LOGIC_ERR, /* Error in Paging Logic                         */

        /* Tier 1 Status */
        MS1_NOT_A_VALUED_OBJECT,   /* Obj. Found, not defined w/value        */
        MS1_EXACT_OBJ_NOT_FND,     /* No Object by that exact Obj. ID        */
        MS1_MISSING_SMI,    /* SMI Relationship was inexplicably missing     */
        MS1_INIT_FAIL,      /* Initialization of Relationship Mandles failed */
        MS1_DC_NT_CORRUPT,  /* Non-Terms describing Data-Constructs corrupted*/
        MS1_DC_BUILTIN,     /* Built-In Data Construct found and returned    */
        MS1_DC_BUILTUP,     /* Built-Up Data Construct found and returned    */
        MS1_DC_BUILTIN_TMPLT,/* Built-In Data Construct Template fnd & ret.  */
        MS1_TMPLT_NUMBER,   /* Built-In Template "Number Value"              */
        MS1_TMPLT_STRING,   /* Built-In Template "String Value"              */
        MS1_INVALID_ARG     /* Invalid Context or Ident Block pointer arg    */
        }   mir_status;


/*
|==============================================================================
|  "mir_oid_to_mandle" Tier 0 Interface Function "get_style" Argument
|
|   This enumerated type is used as an argument to "mir_oid_to_mandle()"
|   function to indicate what kind of lookup strategy is to be employed.
|   For details, see documentation for "mir_oid_to_mandle()".
|
|==============================================================================
*/
typedef
    enum {
        GET_EXACT,     /* Find Object with exactly the same Object ID        */
        GET_EXACT_ROLL,/* (as above, but roll to next object on 'FIND_NONE') */
        GET_NEXT,      /* Find Object with Object ID that is registered NEXT */
        GET_PREVIOUS   /* Find Object with ID that is registered PRIOR       */
        } get_style;


/*
|==============================================================================
|  "mir_search_rel_table" Tier 0 Interface Function "search_style" Argument
|
|   This enumerated type is used as an argument to "mir_search_rel_table()"
|   function to indicate what kind of search strategy is to be employed.
|   For details, see documentation for "mir_search_rel_table()".
|
|==============================================================================
*/
typedef
    enum {
        SEARCH_FROMTOP, /* Start search from top of relationship table      */
        SEARCH_FROMLAST /* Start search from last successful match in table */
        } search_style;


/*
|==============================================================================
|   "MIR_List_Type" Targets  ("mir_search_rel_table" Tier 0 Interface Function)
|
|   This enumerated type describes all the legal values the compiler
|   uses as targets of the Relationship "MIR_List_Type".
|
|   The compiler uses the MIR_List_Type relationship to disambiguate the
|   four kinds of lists that it is used in.  This helps MCC Tier 1 functions
|   "do the right thing" for each of these kinds of lists.
|==============================================================================
*/
typedef
    enum {
        LT_ATTR_PART,   /* List Type "Attribute Partition"  */
        LT_ATTR_GROUP,  /* List Type "Attribute Group List" */
        LT_EVENT_PART,  /* List Type "Event Partition"      */
        LT_EVENT_GROUP  /* List Type "Event Group List"     */
        } list_type;


/*
|==============================================================================
|  "mir_mandle_to_oid" Tier 0 Interface Function "oid_smi" Argument
|
|   This enumerated type is used as an argument to "mir_mandle_to_oid()"
|   function to indicate which group of OIDs is to be searched for an
|   OID for the "mandle"-object.
|   For details, see documentation for "mir_mandle_to_oid()".
|
|   The symbol "MAX_OID_SMI" is often used to define the size of an array
|   that has one element per valid SMI.  If more entries are added to the end
|   of the list of valid SMIs, be sure to add them BEFORE "OID_ANY" in the
|   list below.  (Code in module "mir_symdump.c" and "mir_qim.c" will need to
|   be expanded if you do add new SMIs..search for "OID_ANY" in those modules).
|
|   NOTE:  The numeric values of these symbols (except for "OID_ANY" get
|          written into the compiler's binary output file.  If you scramble
|          the order or add new codes any place but at the end of the list,
|          the resulting file won't be intelligible to older versions of MIR
|          Tier 0 function "mir_mandle_to_oid()" that may be in the field.
|==============================================================================
*/
typedef
    enum {
        OID_MCC,        /* Object ID as assigned according to MCC rules      */
        OID_OID,        /* OID assigned via "OID =" clause                   */
        OID_DNA,        /* OID assigned according to SMI = EMA Entity Model  */
        OID_OSI,        /* Object ID as assigned according to SMI = OSI      */
        OID_SNMP,       /* OID assigned according to SMI = MIB rules (SNMP)  */
        /* This is the End of the List of Valid SMIs! If you add more, do it */
        /* -->HERE!                                                          */

        OID_ANY         /* Request Code for "Any, First-OID SMI" Assigned    */
        } mir_oid_smi;  /* (Should always be "last" in this enumeration list,*/
                        /*  it is not a valid SMI, and is not included in    */
                        /*  the value of MAX_OID_SMI....it "floats" at the   */
                        /*  end of the list).                                */

#define MAX_OID_SMI (OID_ANY)


/*
|=============================================================================
| Page Buffer Structure.
|
| This structure is used by Tier 0 functions when a 'paged' open has been
| made, to keep track of page buffers that contain information.  Each page
| buffer is stored via this structure along with the information needed to
| manage it.
|
| The Free List of page buffers is maintained out of module-local headers
| in "mir_t0.c".
|
| A page buffer is 'active' if it does not appear on the "Free List" (ie,
| it's reference count is greater than zero).  
|
| A page buffer is 'inactive' if it appears on the "Free List", implying a
| reference count of zero.  Note that the contents of a page buffer strung
| on the Free List is still considered valuable...in fact the Free List is
| a form of cache.
|
| If this structure is strung on the free list, then that means that
| not only is the content of the buffer available for re-reference, but the
| buffer itself is available for re-use (if the content of the buffer is
| turns out to be no longer of interest).
|
| "first" is the 'real' masa for the first wholly-contained MIR object in
| the buffer.
|
| "last" is the 'real' masa for the last wholly-contained MIR object in
| the buffer (it may be the same as first).
|
| "v_last" is the 'virtual' masa for the last wholly-contained MIR object in
| the buffer specified by 'page' (in other words, it's the offset from
| the beginning of the buffer to the start of the last MIR object. . . we
| regard this as a 'virtual' address because its relative to the buffer
| rather than the start of the entire MIR address space (which is a 'real'
| masa).
|=============================================================================
*/
typedef 
    struct pagebuf_tag {
        struct pagebuf_tag *prev;  /* Non-null: Previous on free list       */
        struct pagebuf_tag *next;  /* Non-null: Next on free list           */
        masa               first;  /* masa of first wholly-contained MIR obj*/
        masa               last;   /* masa of last wholly-contained MIR obj */
        masa               v_last; /* virtual masa of last in page[]        */
        int                ref_count; /* Reference Count for this instance  */
        unsigned int       *page;  /* The Page Buffer itself                */
        int                a_size; /* Page Buffer active size.  This is the */
                                   /* the number cells in this buffer       */
                                   /* that actually contain information.    */
        } page_buf;


/*
|==============================================================================
|  Mandle- "MIR Handle" Data Structure
|
|   This data structure is equivalent to a "looked-up Object" in the MIR.  Once
|   an Object ID has been looked up in the MIR a pointer to a mandle is
|   returned indicating where this object is in the MIR.  The mandle for
|   the object (rather than the object ID for the object) is passed on
|   subsequent calls to the Tier 0 functions.
|
|   This structure also serves as a "mandle class", which at the implementation
|   level is nothing more than a header (mandle class structure ) containing
|   pointers to a doubly-linked list of mandles that are in that class.
|   Whether or not an instance of this structure is serving as a mandle or
|   as a mandle-class is indicated by the value of the "cclass" cell.  If null,
|   then this is an instance of a mandle_class.  If non-null, this is an
|   instance of a mandle and the "cclass" cell value points at the mandle_class
|   structure that this mandle belongs to.
|
|   The "next" and "prev" pointers are used to link mandle-classes (headers)
|   into a list as well as the mandles themselves into a class list.
|
|   The "this" pointer is meant to contain the address of the structure itself
|   to serve as a check on the validity of the mandle.  When a mandle or
|   mandle-class structure (this structure) is placed on a "free list", then
|   the "this" field is set to "NULL", indicating it is no longer a valid
|   structure (to catch instances of the user passing a pointer to a mandle
|   that s/he had previously released).
|
|   NOTE:
|       This data structure should be considered opaque to the user of the
|       Tier 0 functions.  Write no code that directly references any field
|       of this structure.
|
|   Double NOTE:
|       It is probably best if Tier 1 functions don't allow mandles to
|       float outside to the caller of Tier 1 functions.  Wrap them in
|       a context block instead (as is done for some V1.0 Tier 1 functions).
|==============================================================================
*/
typedef
    struct mandle_tag {
       struct mandle_tag    *cclass; /* NULL: Class, NON-NULL:--> Class  */
       struct mandle_tag    *next;   /* Next on the linked class list    */
       struct mandle_tag    *prev;   /* Previous on linked class list    */
       struct mandle_tag    *tthis;  /* Address of this instance         */

       union {
            /* Mandle */
            struct {
                masa            ex_add;     /* External ('real') Address */
                page_buf       *page;       /* Associated Page-Buffer if */
                                            /* we're doing paging open   */
                masa            vir_add;    /* Virtual MASA in "page" buf*/
                int             synonym;    /* Synonym for Rel. Objects  */
                int             last_match; /* Entry where last search   */
                } m;                        /*  match occurred, ("-1"    */
                                            /*  means "start from top")  */
            /* Mandle Class */
            struct {
                struct mandle_tag   *top;   /* Top of Mandle Class List  */
                } mc;
            } m;
         }  mandle; /* Mandle Data Structure */


/*
|==============================================================================
|   Mandle Class Data Structure
|
|   This data structure is used by the Tier 0 Interface functions to 
|   group "mandles" by class so that the mandles may be released for re-use
|   "en masse".
|==============================================================================
*/
typedef mandle  mandle_class;


/*
|==============================================================================
|   MIR Value Structure Type Indicator
|
|   This enumerated type is used to indicate in a MIR Value Structuer
|   ("mir_value") whether the value is a number or a string.
|==============================================================================
*/
typedef
    enum {
        MIR_UNUMBER,    /* Value is an Unsigend-Integer */
        MIR_SNUMBER,    /* Value is an Sigend-Integer   */
        MIR_STRING      /* Value is a string            */
        } mir_value_type;


/*
|==============================================================================
|   MIR Value Structure
|
|   This structure is used by the mir_search_rel_table() function to return
|   Terminal Object values (either a number or a string).
|
|   The mv_type field indicates which of the two union structures is
|   being used.
|
|   NOTE:  This structure should be considered opaque to the user of MIR
|          Tier 0 functions.  Use the macros below to access fields as needed.
|
|   NOTE:  The string is found in the MIR's Address Space.  If written to,
|   this could corrupt the MIR.  This strategy is fast as it requires no storage
|   allocation or string copying on the part of Tier 0, but suffers from
|   this disadvantage.
|
|==============================================================================
*/
typedef
    struct {
        /* Indicates "string", "signed/unsigned number" */
        mir_value_type  mv_type;

        union {     /* (according to mv_type) */

            /* Signed Number */
            struct {
                int     number;         /* SMI Number */
                } sn;

            /* Unsigned Number */
            struct {
                unsigned int    number; /* SMI Number */
                } un;

            /* String */
            struct {
                unsigned int    len;    /* String Length */
                char            *str;   /* String itself */
                }s;
            } mv;
        }  mir_value;

/*
|==============================================================================
|   "mir_value" Macros
|
|   This macros can be used to simplify the extraction of relevant values
|   from the "mir_value" structure shown above.
|
|   Use of these macros provides flexibility in changing the structure of
|   the "mir_value" structure as well as simplifying the ugly field references
|   that result in C if the macros are not used.
|
|   The choice of which macro to use depends on the value of the condition
|   returned by the "IS_MV_xxxx()" macro.
|
|   NOTE: The macros require a *pointer*, so use a (&ptr) construct as an
|         argument for references to structures allocated locally in a 
|         function.
|==============================================================================
*/

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to "TRUE" if the "mir_value" is a string.                   */
#define IS_MV_STRING(ptr) (((ptr)->mv_type) == MIR_STRING)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to "TRUE" if the "mir_value" is a number (signed or         */
/* unsigned).                                                                */
#define IS_MV_NUMBER(ptr) (((ptr)->mv_type) == MIR_UNUMBER        \
                           || ((ptr)->mv_type) == MIR_SNUMBER)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to "TRUE" if the "mir_value" is a SIGNED number.            */
#define IS_MV_SNUMBER(ptr) (((ptr)->mv_type) == MIR_SNUMBER)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to "TRUE" if the "mir_value" is an UNSIGNED number.         */
#define IS_MV_UNUMBER(ptr) (((ptr)->mv_type) == MIR_UNUMBER)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to an UNSIGNED integer if the "mir_value" contains a        */
/* number (as may be indicated by the "IS_MV_UNUMBER()" macro above.         */
#define MV_UNUMBER(ptr) ((ptr)->mv.un.number)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to an SIGNED integer if the "mir_value" contains a          */
/* number (as may be indicated by the "IS_MV_SNUMBER()" macro above.         */
#define MV_SNUMBER(ptr) ((ptr)->mv.sn.number)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to a char * if the "mir_value" contains a string (as may    */
/* be indicated by the "IS_MV_STRING()" macro above.                         */
/* (It is a null terminated string and MUST NOT BE OVERWRITTEN!).            */
#define MV_STRING(ptr) ((ptr)->mv.s.str)

/* This macro takes as an argument a pointer to a "mir_value" data type      */
/* and evaluates to an unsigned int if the "mir_value" contains a string     */
/* (as may be indicated by the "IS_MV_STRING()" macro above.  The value      */
/* returned is the length of the string exclusive of the terminating null    */
/* byte.                                                                     */
#define MV_STR_LEN(ptr) ((ptr)->mv.s.len)


/*
|==============================================================================
|   MIR Structure of Management Information
|
|   This enumerated type defines the codes that Tier 1 Function "mir_get_smi()"
|   may return to its caller to indicate what kind of SMI the object was
|   derived from.
|
|   NOTE:  The values ("1" for MIR_SMI_DNA, for instance) given to each of
|          these symbols by this definition must match the values given in
|          the "builtin_types.dat" file submitted to the compiler at compile
|          time.
|
|   Additionally, code in mirci_compiler_lists() in "mir_internal.c" depends
|   on "MIR_SMI_UNKNOWN" being the first (with value 0) in this list).
|==============================================================================
*/
typedef
    enum {
        MIR_SMI_UNKNOWN,    /* Unknown SMI                   */
        MIR_SMI_DNA,        /* Digital Network Architecture  */
        MIR_SMI_SNMP        /* Internet                      */
        /* ===>
        |  If you add more, they go here, and you change MAX_SMI below!
        |
        |  NEVER, EVER, remove any! Doing so may cause code in "mir_internal.c"
        |  to choke when it reads in an existing MIR database file from a
        |  previous version of the compiler.
        */
        } mir_smi_code;

#define MAX_SMI 3


/* 
|==============================================================================
|   Tier 1 Functions' "Data-Construct" Context Block
|
|   This data structure (opaque to the caller of the associated Tier 1
|   function) is used to maintain a context between calls to:
|
|       mir_lookup_data_construct()
|       mir_eval_data_construct()
|       mir_eval_builtin_template()
|
|   mir_dissolve_context() is used to release storage for this data-structure.
|==============================================================================
*/
typedef
    struct {
        mandle          *src;   /* "Source Mandle" for next operation        */
        mandle          *trg;   /* "Target Mandle" for next operation        */
        mandle          *ott;   /* "One-Time Template" for builtin-templates */
        mandle_class    *cclass; /* "Mandle Class" for above mandles         */
    } dc_context;


/*
|==============================================================================
|   Data-Construct Identification Block
|
|   This data structure is meant to be visible to the caller of Tier 1
|   functions:
|
|       mir_lookup_data_construct()
|       mir_eval_data_construct()
|       mir_eval_builtin_template()
|
|   as it returns data to the user from these functions.
|==============================================================================
*/
typedef
    struct {
        BOOL            b_smi;         /* TRUE: SMI value is present         */
        mir_smi_code    smi;           /* Data Construct's SMI               */

        BOOL            b_asn1_tag;    /* TRUE: ASN.1 Tag value is present   */
        int             asn1_tag;      /* ASN.1 Application Tag              */

        BOOL            b_asn1_class;  /* TRUE: ASN.1 Class value is present */
        int             asn1_class;    /* Data Construct's ASN.1 Class       */

        BOOL            b_asn1_utag;   /* TRUE: ASN.1 Univ Tag is present    */
        int             asn1_utag;     /* ASN.1 Universal Tag                */

        BOOL            b_code;        /* TRUE: Code value is present        */
        int             code;          /* Data Construct's smi-defined Code  */

        BOOL            b_name;        /* TRUE: Name ptr is present          */
        char            *name;         /* Data Construct's smi-defined Name  */

        BOOL            b_mcc_size;    /* TRUE: MCC Datatype Size is present */
        int             mcc_size;      /* Data Construct's MCC "Size"        */

        BOOL            b_cmip_code;   /* TRUE: CMIP Code value is present   */
        int             cmip_code;     /* DC's smi-defined CMIP Code         */

    } dc_ident;


/*
|==============================================================================
|   Relationship Name/Mandle Pair
|
|   This data structure is used as an argument (typically as an array) to
|   function "mir_get_rel_mandle()".
|
|   The user fills in the name in the structure and the function fills in
|   a pointer to a mandle.  See "mir_get_rel_mandle()".
|==============================================================================
*/
typedef
    struct {
        char    *name;  /* Pointer to the Name of the Relationship */
        mandle  *m;     /* Pointer to the Mandle                   */
        } rel_pair;


/*
|==============================================================================
|
|   Define Prototypes for MIR Tier 0 Interface  Functions
|
|==============================================================================
*/

/* mir_t0_init - Init Tier 0 by Loading the MIR Database File into memory
|
| Define the symbols used to reference information in the "preamble data array"
| returned by mir_t0_init():
*/
#define PRE_MAJOR_VER   0       /* Major Compiler Version Number           */
#define PRE_MINOR_VER   1       /* Minor Compiler Version Number           */
#define PRE_MAX_ARCS    2       /* Maximum number of OID arcs in index     */
#define PRE_MAS_SIZE    3       /* Size in bytes of MIR address space      */
#define PRE_SLICE_CNT   4       /* Count of Index Slices                   */
#define PRE_SUBREG_CNT  5       /* Count of Subregisters                   */
#define PRE_SIGNED_CNT  6       /* Count of Signed Numbers                 */
#define PRE_UNSIGNED_CNT  7     /* Count of Unsigned Numbers               */
#define PRE_STRING_CNT  8       /* Count of Strings                        */
#define PRE_NT_DC_CNT   9       /* Count of Data-Construct Non-Terminals   */
#define PRE_NT_GEN_CNT  10      /* Count of (General) Non-Terminals        */
#define PRE_NT_REL_CNT  11      /* Count of MIR Relationship Non-Terminals */
#define PRE_NT_MAX_SIZE 12      /* Maximum General NT size *in bytes*      */
#define PRE_STR_MAX_SIZE 13     /* Maximum Terminal String size *in bytes* */

/* Default location for builtin_types file */
#if VMS
# define DEFAULT_BT_PATH "SYS$LIBRARY:builtin_types.dat"
#else
# if defined(__ultrix)
#  define DEFAULT_BT_PATH "/usr/lib/eca/builtin_types.dat"
# else
#  define DEFAULT_BT_PATH "/etc/eca/builtin_types.dat"
# endif
#endif

/* mir_t0_init - Init Tier 0 by Loading the MIR Database File into memory */
mir_status
mir_t0_init PROTOTYPE((
char    *,              /* If non-NULL, string containing filename to use   */
char    *,              /* If non-NULL, -> environment variable name "File" */
char    **,             /* If non-NULL, on successfile file-open, name-used */
                        /*              is returned to caller.              */
BOOL    ,               /* TRUE: Page NTs if "ev_open" (see also "ev_open") */
char    *,              /* If non-NULL, -> env variable name for "pageOpen" */
char    *,              /* If non-NULL, -> environment variable name "Open" */
int     *,              /* If non-NULL, -> where to rtn max # of page buffs */
int     *,              /* If non-NULL, sets/returns free list size         */
int     **              /* Set on return to point to preamble array         */
));

/* mir_get_rel_mandles - Get Relationship MIR Object Mandles */
mir_status
mir_get_rel_mandles PROTOTYPE((
unsigned     ,             /* Number of entries in following array      */
rel_pair     *,            /* --> Array of rel_pairs with count entries */
mandle_class **            /* --> Mandleclass pointer for mandle class  */
));                        /* to use.                                   */

/* mir_oid_to_mandle - ISO Object ID to Mandle Lookup */
mir_status mir_oid_to_mandle PROTOTYPE((
get_style    ,          /* Style of the Lookup to be done       */
object_id    *,         /* -> Object Id structure to be found   */
mandle       **,        /* ->> Mandle                           */
mandle_class **,        /* ->> Mandle Class                     */
mir_oid_smi  *          /* Where to return SMI of OID           */
));

/* mir_mandle_to_oid - MIR mandle to ISO Object ID Conversion */
mir_status mir_mandle_to_oid PROTOTYPE((
mandle      *,          /* --> Mandle whose ISO Object ID is desired */
object_id   *,          /* Object ID structure to receive OID value  */
mir_oid_smi *           /* Where to return SMI indicator for OID     */
));

/* mir_search_rel_table - Search Non-Terminal Object Relationship Table */
mir_status mir_search_rel_table PROTOTYPE((
search_style    ,       /* Search Style to be used on rel. table    */
mandle          *,      /* Mandle for Searched-Object               */
mandle          *,      /* Mandle for Relationship-Object           */
mandle_class   **,      /* Addr of ptr to mandle-class              */
mandle         **,      /* Addr of ptr to mandle                    */
mir_value       *       /* Addr of ptr to mir_value structure       */
));

/* mir_free_mandle - Free Mandle for Re-Use */
mir_status mir_free_mandle PROTOTYPE((
mandle          **      /* Address of Ptr to Mandle to be freed     */
));

/* mir_copy_mandle - Copy Existing Mandle */
mir_status mir_copy_mandle PROTOTYPE((
mandle          *,      /* Address of Mandle to be copied           */
mandle          **      /* Address of ptr to created Mandle         */
));

/* mir_free_mandle_class - Free all Mandle in a Mandle Class for Re-Use */
mir_status mir_free_mandle_class PROTOTYPE((
mandle_class    **      /* Mandle class to be freed */
));

/* MIR Status Interpretation Function */
char *mir_error PROTOTYPE((
mir_status       /* Inbound status */
));

/* mir_get_mas - Get MIR Address Space */
void
mir_get_mas PROTOTYPE((
unsigned int **,             /* Where to return address of MIR Address Space */
masa         **,             /* Where to return address of tranlate table    */
int          *               /* Where to return size of translate table      */
));

/*
| NOTE:
|       For Common Agent SNMP PE, file "snmppe.h" replicates the following
|       prototype under certain circumstances.  If you change this prototype
|       here, you must change it there too.
*/
/* mir_debug_statistics - Dump In-Use Statistics for Mandles/Classes */
void mir_debug_statistics PROTOTYPE((
int     *,              /* Address of int to rcv Mandle-in-use count      */
int     *,              /* Address of int to rcv Mandleclass-in-use count */
int     *               /* Address of int to rcv Free Mandle/class  count */
));


/* mir_reset_search - Resets Starting Point for Search to "Top" 
|
|  Takes as an argument a non-null pointer to a previously created mandle..
|  Note: No argument checking!!
*/
#define mir_reset_search(p) (p)->m.m.last_match = -1



/*
|==============================================================================
|
|   Define Prototypes for MIR Tier 1 Interface  Functions
|
|==============================================================================
*/

/* mir_get_smi - Get an Object's Structure of Management Information ID */
mir_status mir_get_smi PROTOTYPE((
object_id       *,   /* -> Object Id of object whose SMI is desired */
mir_smi_code    *    /* -> Place to return smi code                 */
));

/* mir_lookup_data_construct - Lookup "Datatype" of an Object-with-a-Value */
mir_status mir_lookup_data_construct PROTOTYPE((
object_id       *,   /* -> Object Id of object whose D-Cis desired */
dc_context      **,  /* Address of ptr to be set to context block  */
dc_ident        *    /* Address of D-C identification block        */
));

/* mir_eval_data_construct - Evaluate previously fetched Data Construct */
mir_status mir_eval_data_construct PROTOTYPE((
dc_context      **,  /* Addr of ptr to prev. established context block*/
dc_ident        *    /* Address of D-C identification block        */
));

/* mir_eval_builtin_template - Evaluate context block describing template */
mir_status mir_eval_builtin_template PROTOTYPE((
dc_context  **,  /* Addr of ptr to prev. established context block */
int         *,   /* -> integer to receive a Template Number value  */
char        **,  /* -> string ptr to recv a Template String value  */
dc_context  **,  /* Addr of ptr to newly-created context block     */
dc_ident    *,   /* Address of D-C identification block            */
BOOL        *    /* Address of "restart" flag for repeated evals   */
));

/* mir_copy_context - Create a copy of a context block */
mir_status mir_copy_context PROTOTYPE((
dc_context     *,   /*  Ptr to context block to release */
dc_context     **   /* Addr of ptr to new block         */
));

/* mir_dissolve_context - Release Storage for Context Block */
mir_status mir_dissolve_context PROTOTYPE((
dc_context      **   /*  Ptr to context block to release */
));

/*
|  These definitions are of use ONLY to code that directly examines
|  the MIR Address Space (ie code *INSIDE THE* Tier 0 functions).
*/

#if defined(MIR_T0)

/*
|==============================================================================
|   These macros return TRUE if the argument is the masa of whatever
|   MIR data structure is named in the name of the macro.
|
|   These macro definitions serve to isolate the structure of the partition
|   table from the Tier 0 code (somewhat).
|
|   Note the macros presume the use of the array name "mas" to describe the
|   MIR Address Space.
|
|   The symbols used in these macros are particular to the BINARY_FORMAT of
|   the MIR database file (MAS) (ie, if you change the layout of the file,
|   you change the BINARY_FORMAT value and these macros to match).
|
|   NOTE:  The order in which the partitions were laid out in the MIR address
|   space was carefully done to make these definitions the simplest for
|   those cases where we 'ask the question' a lot.  Typically the MIR Tier 0
|   functions needed (most often) to discover whether a given masa describes
|   an Index Slice or a Non-Terminal.  These partitions are laid out so that
|   the 'questions' for these two cases are simple (one term each).
|
| MAS Addr
|    1:   *-----MIR Address Space-----*
|         |      Partition Table      | -Contains MASA's of the start of
|         |         Partition         | -all the other partitions + misc. data
|         *---------------------------*
|         |        Index Slice        | -Contains INDEX SLICEs, (ie most of
|         |         Partition         | -OID index)
|         *---------------------------*
|         |        Subregister        | -Contains INDEX SUBREGISTERs (the rest
|         |         Partition         | -of the OID index)
|         *---------------------------*
|         |       Signed Number       | -All SIGNED NUMBERS parsed from the
|         |         Partition         | -SMI-specified objects (Attribs etc)
|         *---------------------------*
|         |      Unsigned Number      | -All UNSIGNED SIGNED NUMBERS (as above)
|         |         Partition         |
|         *---------------------------*
|         |          String           | -All STRINGS parsed from the
|         |         Partition         | -SMI-specified objects (Attribs etc)
|         *---------------------------*
|         |      Data-Construct       | -All Non-Terminals representing
|         |       Non-Terminal        | -BUILTIN (to all SMIs) Datatypes
|         |         Partition         |
|         *---------------------------*
|         |          General          | -All Non-Terminals representing things
|         |       Non-Terminal        | -parsed from MSL files, including 
|         |         Partition         | -"TYPE" statements
|         *---------------------------*
|         |     MIR Relationship      | -All MIR Relationship Objects used by
|         |       Non-Terminal        | -the compiler to 'link' the objects
|         |         Partition         | -in the DC & General Partitions
|         *---------------------------*
|==============================================================================
*/
#define IS_A_SLICE(i)  ((i) < mas[START_SUBREG_MASA])
#define IS_A_SUBREG(i) (((i) < mas[START_SNUMBER_MASA])     \
                     && ((i) >= mas[START_SUBREG_MASA]))
#define IS_A_SNUMBER(i) (((i) < mas[START_UNUMBER_MASA])    \
                     && ((i) >= mas[START_SNUMBER_MASA]))
#define IS_A_UNUMBER(i) (((i) < mas[START_STRING_MASA])     \
                     && ((i) >= mas[START_UNUMBER_MASA]))
#define IS_A_STRING(i) (((i) < mas[START_NONTERM_DC_MASA])  \
                     && ((i) >= mas[START_STRING_MASA]))

/* Specific kinds of Non-Terminals: */
#define IS_A_NONTERM_DC(i) (((i) < mas[START_NONTERM_MASA])      \
                         && ((i) >= mas[START_NONTERM_DC_MASA]))
#define IS_A_NONTERM_REL(i) ((i) >= mas[START_NONTERM_REL_MASA])

/* Any kind of Non-Terminal: */
#define IS_A_NONTERM(i) ((i) >= mas[START_NONTERM_DC_MASA])
#endif

/* ==========================================================================*/
/* =====================End of Tier 0/1 Interface Stuff======================*/
/* ==========================================================================*/

/*
|  The following definitions are required BOTH by the MIR Compiler AND
|  the MIR Tier 0 functions.
*/

#if defined(MIR_COMPILER) || defined(MIR_T0)
/*
|==============================================================================
| Partition-Table Partition MASAs
|
|   These symbols represent the MASA of selected "fixed" cells within the
|   Partition-Table Partition.  It is anticipated that data contained at
|   these addresses will always be 'of interest' to the compiler and MIR
|   Tier 0 functions.  Consequently they are made available here.
|
|   These symbols are meant to be "current" ("fixed") for the *given* value of
|   "BINARY_FORMAT" defined elsewhere in mir.h.
|
|   Should the structure of the Partition-Table Partition change with
|   subsequent changes in BINARY_FORMAT, these symbols should be adjusted
|   accordingly, (while perhaps moving the old definitions into
|   "mir_symdump.c" in order that that module can continue to interpret old
|   BINARY_FORMATed files).
|
| The following is extracted from "mir_external.c" documentation:
| (For "#define BINARY_FORMAT  2")

Partition-Table Partition - This is merely a series of (4-byte) cells that
       contain the MIR-Address-Space Addresses ("masa"s) for the remaining
       partitions in the MIR Address Space plus masks/counts needed for
       interpreting V2 "compressed" (as compared with V1) Non-Terminal objects:

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

       Compiler #define symbols for these cell's addresses are defined in
       "mir.h" for use by the compiler and MIR Tier 0 functions.

|==============================================================================
|And here are the corresponding symbols:
*/
#define ROOT_INDEX_MASA         1
#define START_SUBREG_MASA       2
#define START_SNUMBER_MASA      3
#define START_UNUMBER_MASA      4
#define START_STRING_MASA       5
#define START_NONTERM_DC_MASA   6
#define START_NONTERM_MASA      7
#define START_NONTERM_REL_MASA  8
#define ENTRY_COUNT_AMASK_MASA  9
#define OID_COUNT_RSHIFT_MASA   10
#define OID_BPTR_AMASK_MASA     11
#define OID_SMI_RSHIFT_MASA     12
#define SYNONYM_AMASK_MASA      13
#define TARGET_RSHIFT_MASA      14

/*
| Code in MIR Compiler module "mir_internal.c" expects that if it reads
| in the "MIR Address Space" (from a binary MIR database file) starting at
| MASA "1" **up to the MASA specified by the following symbol**, then it
| assumes that it has "read-in" the Partition-Table Partition, and that
| all the MIR Address space addresses described by the symbols above are
| available to it.
*/
#define FIRST_MASA_AFTER_P_TABLE 15

/*
|==============================================================================
| Compiler Binary Output Format
|
|   This number indicates the internal format of the binary file.  This
|   value is written as a 32-bit signed number in the preamble section
|   of the compiler binary output file.  If a change is made to the
|   compiler binary output file format, this number should be incremented
|   and logic installed into MIR_SYMDUMP.C, MIR_T0.C and MIR_INTERNAL.C
|   functions that interpret the binary file to allow them to detect the
|   different format(s) and handle each appropriately.  (Note that MIR_QIM.C
|   may need attention too, depending on the change).
|
|       Version         Comments
|          1            Compiler V1.0: Original Format: 8-byte Rel. Table
|                       Entries, only one 'OID' backpointer to the index.
|
|          2            Compiler V1.95: New Compressed Format.
|                       4-byte Rel. Table  Entries, multiple OID backpointers
|                       to index, separate partitions for DC & Rel. Objects,
|                       Unsigned Numbers.  New info in Partition Table to
|                       make a compile from binary file possible.
|==============================================================================
*/
#define BINARY_FORMAT 2


#endif

/* ==========================================================================*/
/* ====================End of Compiler AND Tier 0 Stuff======================*/
/* ==========================================================================*/

#if defined(MIR_COMPILER)

/*
 * MODULE DESCRIPTION:
 *
 * Common Agent Management Information Repository
 *
 * Module MIR_COMPILER.H
 *      Contains data structure definitions required by the MIR MSL
 *      compiler.
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
 *       Management Information Repository, accessible by ISO Object ID
 *
 *    Purpose:
 *       This module contains the data structures required by the functions
 *       that implement the MIR compiler.  See the Common Agent Management
 *       Information Repository Functional Specification document for details.
 *
 * History
 *      V0.0    December 1990           D. D. Burns
 *                                          "Bridge" (no compiler) Version
 *                                          (file then named "mir_structures.h")
 *
 *      V0.1    January 1991            D. D. Burns
 *                                          "Compiler" Version
 *      V1.5    June 1991               D. D. Burns
 *                                          Merged into "mir.h"
 */

/* Module Overview: 
|
|   This file contains definitions for the C-structures used internally by the
|   Common Agent Management Information Repository compiler.
|
*/

/*
|==============================================================================
| Compiler Version Symbols
|
|   These get written into the binary output file generated by a successful
|   compilation.
|
|   CHANGE AT LEAST THE MINOR VERSION WITH ANY NEW RELEASE.
|
|       MAJOR   MINOR   Date               Release
|         1       0     February 22, 1991  First release of MIR compiler
|         1       1     March 8, 1991      Fix Bug in generating OIDs for
|                                           "action-response-arguments".
|         1       5     June 10, 1991      Added support for MSL compilations
|                                           encompassing more than one SMI
|         1       6     September 27, 1991 Added support for IDENTIFIER
|                                           and SNMP_OBJ_TYPE clauses
|         1       7     November 15, 1991  Tier 0 changes, bug fixes
|                                           and new relationships for MOM GEN
|         1       8     January 1992       Version delivered w/SNMP Protocol
|                                           Engine
|         1       90    March  1992        Preliminary MCC Version
|         1       91    July  1992         Second Preliminary MCC Version
|         1       95    July  1992         Third Preliminary MCC Version
|                                           (Binary Output Format 2)
|         1       96    July/Aug 1992      Interim test Version
|         1       97    August 1992        Fourth Preliminary MCC Version
|         1       98    Sept 1992          Fifth Preliminary MCC Version,
|                                           contains support for -b & -r
|                                           switches.
|         1       99    Oct 1992           Final MCC Version, supports
|                                           AUGMENT/MERGE, 
|         1       990   Nov 1992           Supports "Apply-Defaults Hack"
|         1       993   Dec 1992           Fix bug in remove support (-r)
|         1       994   Jan 1993           Ania's special pre-release
|         2       0     Jan 1993           Final Delivery, spt for Var-Records
|         2       01    Feb 1993           Fix INCLUDE-File bug, VAX C bugchk
|                                           and builtin_types.proto typo
|         2       02    Feb 1993           Add both versions of BinRelTim(e)
|                                           to builtin_types.proto file
|==============================================================================
*/
#define VERSION_MAJOR 2
#define VERSION_MINOR 02        /* Two digits always, as of V2.00 */


/*
|==============================================================================
| Compressed Format 'Packing' Symbols
|
|   These symbols are used by the compiler to build cells in the new
|   Binary Format 2 Non-Terminals.  The cells have two data each (whereas
|   in Binary Format 1, it was one datum per MAS cell).
|
|   Some of the values of these symbols are recorded (in the Partition-Table
|   Partition in the output file) for use by Tier 0 functions in 'unpacking'
|   the packed cell values.  In this way, the packing arrangement can be
|   'tuned' without forcing changes in MIR Tier 0 code or the Binary Format
|   Version number.
|
|==============================================================================
| The following documentation is extracted from "mir_external.c".  If changes
| are needed, make them there and extract back to here.
--------------------------------
For Binary File Format Version 2 all Non-Terminals:

                <OID-backpointer count/Entry count>    --See Note 1
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
*/
/*
| Low-Order 16 bit AND Mask for Entry Count.
| stored at masa "ENTRY_COUNT_AMASK_MASA" in Partition-Table Partition
*/
#define ENTRY_COUNT_AMASK (0x0000FFFF)

/*
| Complementary 16 bit Right Shift Count for Hi-order OID Backpointer count,
| stored at masa "OID_COUNT_RSHIFT_MASA" in Partition-Table Partition
*/
#define OID_COUNT_RSHIFT (16)



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

*/
/*
| Low-Order 24 bit AND Mask for OID Backpointer.
| stored at masa "OID_BPTR_AMASK_MASA" in Partition-Table Partition
*/
#define OID_BPTR_AMASK (0x00FFFFFF)

/*
| Complementary 24 bit Right Shift Count for Hi-order OID SMI indicator.
| stored at masa "OID_SMI_RSHIFT_MASA" in Partition-Table Partition
*/
#define OID_SMI_RSHIFT (24)



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
*/
/*
| Low-Order 8 bit AND Mask for Synonym
| stored at masa "SYNONYM_AMASK_MASA" in Partition-Table Partition
*/
#define SYNONYM_AMASK (0x000000FF)

/*
| Complementary 8 bit Right Shift Count for Hi-order Target MASA,
| stored at masa "TARGET_RSHIFT_MASA" in Partition-Table Partition
*/
#define TARGET_RSHIFT (8)




/*
|==============================================================================
| The C compiler for RISC won't allow the use of "void *".  To get around
| this, use "VOID *" instead of "void *".
|==============================================================================
*/
#ifdef mips
#define VOID unsigned char
#else
#define VOID void
#endif


/*
|==============================================================================
|   Memory setting and zeroing functions.
|   This bcmp() (since it uses memcmp()) performs a superset of the basic ULTRIX
|   bcmp()... the superset characteristic of returning a signed indication as
|   to which string is "greater".
|==============================================================================
*/
#define bzero(x,y)    memset((x), 0, (y))
#define bcmp(x,y,z)   memcmp((x),(y),(z))
#define bcopy(x,y,z)  memcpy((y),(x),(z))


/*
|==============================================================================
|   This symbol is the depth of the lists that the mirc_create_object() code
|   maintains to keep track of the "code" values that become part of
|   the ISO Object ID for a (non-datatype) object when it is created and
|   registered.
|
|   These lists are used in a call by mirc_create_object() to back-end
|   routine "mirc_compute_oid()".  This function extracts the
|   needed codes from the list and constructs the proper OID.
|
|   This value need only be big enough to hold the code for each nested
|   entity class and "things" that are defined with codes within the deepest
|   nested entity class.
|==============================================================================
*/
#define LIST_DEPTH 50


#ifndef MASA_TYPE
/*
|==============================================================================
|
|   "MIR Address-Space Address" type and "MIR Address Space"
|
|==============================================================================
*/
typedef
    unsigned int
        masa;

#define MASA_TYPE
#endif


/*
|=============================================================================
| Compiler Mode Enumerated Type
|
|   The compiler operates in one of the modes specified by the values of
|   this enumerated type.
|
|   The cell that carries the current value at run-time is defined in
|   "mir_yacc.y".  See that module for a full discussion of the operational
|   significance of each of these values.
|=============================================================================
*/
typedef
    enum {
        CM_NORMAL,   /* Normal Compilation on regular ASCII MSL files */
        CM_MERGE,    /* Merge Compilation on regular ASCII MSL files  */
        CM_AUGMENT   /* Augment Compilation using AUGMENT ASCII file  */
        } CM_TYPE;


/*
|=============================================================================
| Operation Code for function "mirc_keyword"
|
|   Function "mirc_keyword" can register a string as a legal keyword,
|   verify that a string is a legal keyword or verify that it is a Selected
|   (legal) keyword.  These functions are specified by the values of the
|   following enumerated type.
|=============================================================================
*/
typedef
    enum {
        IT_IS_A_KEYWORD,       /* Register as a Valid Keyword Name   */
        IS_IT_A_KEYWORD,       /* Is string a Valid Keyword Name?    */
        IS_IT_SELECTED         /* Is string a Selected Keyword Name? */
        } KW_OPCODE;


/*
|=============================================================================
| ID Code Selector Type
|
|  The V1.9 compiler must deal with two different "ID" Codes, an "MCC" ID code
|  and a "standard" ID Code.
|
|    MCC Codes appear as "x" in:
|
|       ATTRIBUTE foobar = x : INTEGER
|
|    Standard Code 'x' appears in:
|
|       DNA_CMIP_INT = x;
|
|  In the situations where we need to specify which it is we're dealing with,
|  a variable of this type is used.
|
|=============================================================================
*/
typedef
    enum {
        STD_code,       /* Standard Code Selected */
        MCC_code        /* MCC Code Selected      */
        } CODE_TYPE;


/*
|=============================================================================
| Definition-Status for (General) Non-Terminals
|
|   This type of cell is used in General Non-Terminals when the compiler
|   is operating in MERGE or AUGMENT mode to keep track of whether a
|   given object was PREDEFINED (SMI object), PREDEFINED_TYPE (an object
|   representing a TYPE statement that has already been parsed) or CREATED
|   (just 'now' from a definition in the file currently being compiled).
|
|   By examining this status, the compiler can decide in certain circumstances
|   whether it should modify an object according to the current MSL file or
|   whether it should complain that such a modification is illegal.  This
|   cell is checked by the function that adds relationship table entries to
|   a Non-Terminal's relationship table (mirc_add_relationship() et.al.)
|
|   See the discussion in OTHER THINGS TO KNOW for function
|   "mirc_get_def_status()" in "mir_backend.c" for a long discussion of the
|   use of this field in Non-Terminals, and also "mirc_add_relationship()".
|=============================================================================
*/
typedef
    enum {
        DS_PREDEFINED_TYPE, /* Obj. represents a TYPE stmt, already defined  */
        DS_PREDEFINED,  /* Obj. was already defined at start of current file */
        DS_CREATED      /* Obj. was created from definition in current file  */
        } DEF_STATUS;


/*
|==============================================================================
| MIR Relationships
|
|   These symbols define all the MIR relationships that the compiler knows
|   how to use.  At init-time, the compiler creates a Non-Terminal
|   Intermediate Data Structure (IDS) for each of these relationships.  A
|   pointer to each IDS produced is recorded in the globally accessible
|   array "map_rel_to_ids[]" defined in the backend context block.  By using
|   a symbol from the enumerated list below as an index into the array, a
|   pointer to the IDS may be obtained for that relationship.  This is
|   typically done so that a relationship entry may be made in an object's
|   Relationship Table.
|
|   If you change this list, you must modify function mirci_build_rel_list()
|   in "mir_backend.c" to include your changes.  Be sure to read the warnings
|   associated with that function!
|
|==============================================================================
*/
typedef
    enum {
            /* MIR "Generic" Relationships for use in describing <things>    */
            /* in ALL SMIs.                                                  */
            /* (NOTE: MIR_Relationship_Name *MUST* BE FIRST!)                */
            MIR_Relationship_Name = 0, /* The "root" relationship for MIR    */
            MIR_ID_Code,             /* <thing> has a ID code of ...         */
            MIR_MCC_ID_Code,         /* <thing> has a MCC code of ...        */
            MIR_Text_Name,           /* <thing> as a stringname of ...       */
            MIR_Special,             /* <thing> has a special value of ...   */
            MIR_Contains,            /* <thing> hierarchically contains...   */
            MIR_Contained_By,        /* <thing> is contained by ...          */
            MIR_Structured_As,       /* <thing> is of "datatype" ...         */
                                     /*                   (DataConstruct)    */
            MIR_Indexed_By,          /* <thing> is indexed by "other MIR     */
                                     /*   object"                            */

            /* DataConstruct Non-Terminals may contain these relationships...*/
            MIR_DC_Found_In_SMI,    /* <DataConstruct> found in SMI (code)   */
            MIR_DC_ASN1_Class,      /* <DataConstruct> has ASN1 Class...     */
            MIR_DC_ASN1_Tag,        /* <DataConstruct> has ASN1 Tag...       */
            MIR_DC_SMI_Name,        /* <DataConstruct>'s name in SMI is...   */
            MIR_DC_SMI_Code,        /* <DataConstruct>'s code in SMI is...   */
            MIR_DC_SMI_Template,    /* <DataConstruct> is qualified by a     */
                                    /*   Template relationship ...           */
            MIR_DC_SMI_OTTemplate,  /* <DataConstruct> is qualified by a     */
                                    /*   One-Time-Template relationship ...  */
            MIR_DC_MCC_DT_Size,     /* <DataConstruct> has MCC "Size" of ... */
            MIR_DC_NCL_CMIP_Code,   /* <DataConstruct> has NCL CMIP code of..*/

            /* MIR Relationships for ancillary information for  */
            /* MIR-supported datatypes "Enumeration" & "Record" */
            MIR_Enum_Code,          /* "<enum-datatype> has a code of ..."   */
            MIR_Enum_Text,          /* "<enum-datatype> has text of ..."     */
            MIR_Field_Name,         /* "<rec-datatype>has field name..."     */
            MIR_Field_Code,         /* "<rec-datatype>has field code..."     */
            MIR_Case_Code,          /* "<case-group-instance> selected by    */
                                    /*    fixed-field value ..."             */

            /* MIR Relationships that roughly map to MSL "clauses" */
            MIR_Access,             /* "ACCESS = "                           */
            MIR_valueDefault,       /* "DEFAULT = "                          */
            MIR_Display,            /* "DISPLAY = "                          */
            MIR_Min_Int_Val,        /* "...[x,y]" (x)                        */
            MIR_Max_Int_Val,        /* "...[x,y]" (y)                        */
            MIR_DNS_Ident,          /* "DNS_IDENT = "                        */
            MIR_Units,              /* "UNITS = "                            */
            MIR_Dynamic,            /* "DYNAMIC = "                          */
            MIR_Required,           /* "REQUIRED = "                         */
            MIR_Description,        /* "TEXT = "                             */
            MIR_Directive_type,     /* "DIRECTIVE_TYPE = "                   */
            MIR_Depends_on,         /* "DEPENDS_ON = "                       */
            MIR_Depends_OP,         /* (OPcode, used to mark "=" or "in set" */
            MIR_Echo,               /* "ECHO = "                             */
            MIR_Predictable,        /* "PREDICTABLE = "                      */
            MIR_Variant_Sel,        /* (used to mark DEPENDS-ON char. attrib)*/
            MIR_Counted_As,         /* "COUNTED AS = "                       */
            MIR_Category,           /* "CATEGORIES = "                       */
            MIR_Symbol,             /* "SYMBOL = "                           */
            MIR_Symbol_prefix,      /* "SYMBOL-PREFIX = "                    */

            /* DNA Containment Hierarchy Relationships */
            MIR_Cont_entityClass,
            MIR_Cont_attribute,
            MIR_Cont_attrPartition,
            MIR_Cont_attrGroup,
            MIR_Cont_eventPartition,
            MIR_Cont_eventGroup,
            MIR_Cont_event,
            MIR_Cont_directive,
            MIR_Cont_request,
            MIR_Cont_request_argument,  /* (direct from Entity-Class) */
            MIR_Cont_response,
            MIR_Cont_exception,
            MIR_Cont_argument,
            MIR_List_Entry,
            MIR_List_Type
        /*->HERE is the only place where you should add anything new!  */
        }                       /* (If you add something here......... */
                                /*  change the #define below too!)     */
            mir_relationship;

/* Compiler Internal Mapping Array Size defined based on last above
|
|  NOTE:  Never REDUCE this number across versions of the compiler, since
|         this symbol determines how big certain arrays are in the backend
|         of the compiler.  These arrays are loaded EITHER by functions
|         built in to the backend of the compiler (which could therefore
|         be compiled correctly if you changed this number) OR by functions
|         in "mir_internal.c" that load MIR Relationships from an existing
|         compiled binary file.  If you reduce illegally, the binary file might
|         be created by a version of the compiler that had larger arrays
|         than a new (more current) version with smaller arrays.  The
|         consequence of this is that a newer version of the compiler would
|         be unable to perform a compile that involved a binary file from
|         an earlier (larger) compiler.
*/
#define REL_COUNT (MIR_List_Type+1)


/* 
|==============================================================================
| MIR Intermediate Data Structure Types
|
|   The various flavors of Common Agent MIR intermediate data structures are
|   distingquished by the following enumerated values:
|==============================================================================
*/
typedef
    enum {

        /*
        |  Intermediate (internal) Data Structures identifiers -- these
        |  identify the versions of these structures used to hold the
        |  intermediate representation of the MIR data (ie produced by
        |  compiler into a local heap before translation to the external
        |  form).
        |
        |  Note: Don't change the order! Code in "mir_external.c" &
        |        "mir_intermediate.c" depends on it.
        |
        |  Double Note:  Make additions carefully where noted!
        */
        I_SLICE,        /* A slice of the MIR Index                         */
        I_SUBREGISTER,  /* A Subregister structure for use in MIR Index     */
        I_T_OBJ_snumber,/* Terminal MIR Object - signed number              */
        I_T_OBJ_unumber,/* Terminal MIR Object - unsigned number            */
        I_T_OBJ_string, /* Terminal MIR Object - string                     */
        I_NT_OBJECT_DC, /* Non-Terminal MIR Obj. for SMI DataConstruct      */
        I_NT_OBJECT,    /* Non-Terminal (Searchable) MIR Object             */
        I_NT_OBJECT_Rel,/* Non-Terminal MIR Obj. for compiler Relationship  */
   /* --> Add More here <---(Add more here, change #define below)           */

        I_RECLAIMED     /* Means "Instance is on Free List..no info inside" */
        } IDS_TYPE;     /*        ("I_RECLAIMED" should always be last)     */

#define MAX_IDS_TYPE ((int) I_NT_OBJECT_Rel + 1)


/*
|==============================================================================
| Common Agent MIR Intermediate Data Structures
|
|   The MSL content is converted into instances of these Intermediate Data
|   Structures by the first pass of the compiler (ie, as the yacc grammar
|   parses the input stream).  The last compiler pass sweeps through these
|   IDS's maintained in the heap to produce a compact representation of them
|   in a linear array that becomes the MIR Address Space which is then written
|   to the binary output file.
|
|   "IDS_DFT_TABLESIZE" defines a symbol meant to be submitted as the second
|   argument to "I_Create_IDS()", the function that creates all the
|   Non-Terminal varieties of this structure.  This should be "zero", because
|   that value signals the function to use it's own internal default value.
|   Changing this symbol from zero effectively defeats any smarts that might
|   be put into "I_Create_IDS()" to 'do the right thing' based on the kind
|   of structure being requested.  This symbol just makes it easy to find
|   calls to "I_Create_IDS()" that 'take the default'.
|==============================================================================
*/
#define IDS_DFT_TABLESIZE (0)

typedef

struct ids_tag {

    /*
    |     What kind of Intermediate Data Structure this is
    */
    IDS_TYPE            flavor;

    /*
    | --> Next in linear series (used to string IDS's of various flavors onto
    |     flavor-specific linear lists for further intermediate processing).
    |
    |     NOTE: Only the functions in module "mir_intermediate.c" should be
    |           changing this field!  All other "backend" logic regards
    |           an IDS as "free-floating".
    */
    struct ids_tag      *next;

    /* 
    | --> Previous in linear series. . .
    |     As above, but we're doubly-linked to allow reclaiming IDS's when a
    |     "delete" operation is performed (thru a call to "I_Reclaim_IDS()".
    |
    |     NOTE: Only the functions in module "mir_intermediate.c" should be
    |           changing this field!  All other "backend" logic regards
    |           an IDS as "free-floating".
    */
    struct ids_tag      *prev;

    /*
    |   Externalized (linearized) address of this structure in the
    |   "MIR Address Space".
    |
    |   Note: Debugging code (conditionalized out) in "mir_internal.c"
    |         can be changed to load this field for testing.  Under 
    |         normal operation, this field is loaded by the final pass
    |         of the compiler (code in "mir_external.c").
    */
    masa                ex_add;

    union varieties {

        /*
        | SLICE (of the index tree of ISO Object IDs in the MIR)
        |
        |   flavor = I_SLICE
        |
        |   The slice structure is used to contain one "column" of
        |   ISO object ID values in the MIR.  For instance, given an ISO
        |   object ID of 1.2.3, three slices would be required to
        |   represent the values "1", "2" and "3".  To represent just this
        |   ISO Object ID (1.2.3), each slice contains just one entry.
        |
        */
        struct {
            struct ids_tag   *backptr;      /* Back pointer into Index   */
            unsigned short   entry_count;   /* Count of entries in slice */
            unsigned short   max_count;     /* Maxiumum in slice table   */

            struct SES {                    /* Slice Entry Structure  */
                unsigned int      iso_arc;  /* Arc from ISO Object ID */
                struct ids_tag    *next;    /* Pointer to "the thing" */
                }                           /* (next slice, subreg or */
                        *slice;             /*  a non-terminal)       */
            } i; /* Index */


        /*
        | SUBREGISTER (for use when an ISO Object ID corresponds to
        |              "something" AND there are other "somethings" in
        |              the MIR that must be represented that have Object
        |              IDs that are the same but longer than the original
        |              "something".  In other words "1.3" already exists
        |              as an object in the MIR w/entry "1.3" in index and
        |              all of a sudden an object with OID "1.3.1" is
        |              registered and we need a data structure that not
        |              only points at the "1.3" object but also at another
        |              object that is part of the index structure containing
        |              the entry for "1.3.1").
        |
        |   flavor = I_SUBREGISTER
        |
        |   The subregister structure is used to contain a pointer "down"
        |   into the index to the lower levels that contain the "other"
        |   "somethings" that need representation, and also a pointer
        |   to the original "something" with the short OID.
        |
        */
        struct {
            struct ids_tag   *backptr;       /* Back pointer into Index      */
            struct ids_tag   *ntobject;      /* The original "something"     */
            struct ids_tag   *lower_levels;  /* --> down to other somethings */
            } s; /* Sub-Register */


        /*        
        | NON_TERMINAL Internal Data Structure
        |
        |   flavor = I_NT_OBJECT_Rel - for MIR Objects that are Relationships
        |   flavor = I_NT_OBJECT_DC - for MIR Objects that are dataconstructs
        |                             builtin to the SMI
        |   flavor = I_NT_OBJECT - for all other MIR Objects
        |
        |   This structure represents a "Non-Terminal" MIR Object.  Each
        |   Non-Terminal Object contains a relationship table that can be
        |   searched using the search dictionary interface routine.  The
        |   'intermediate form' (this structure) also contains extra fields
        |   needed during compilation but are stripped during the final pass
        |   that creates the compact representation of these data structures
        |   in the output file.
        |
        |   Note:  The compiler uses more of this flavor IDS than any other.
        |          Don't go adding fields to this unless you Really need them!
        |
        |   Double Note:
        |          We make an explicit effort here to keep this flavor of IDS
        |          exactly aligned on an "int" boundary.  This is so that
        |          some fancy memory-allocation footwork may be accomplished
        |          to put the storage for the relationship table IMMEDIATELY
        |          following an instance of this structure...(ie "rel_table"
        |          may be set to point immediately following an instance of
        |          this structure).  We want to preclude the compiler from
        |          feeling it needs to pad this structure!
        */
        struct nonterm {
            
            /*
            |  This reference count is not used uniformly for all flavors.  See
            |  the discussion under "OTHER THINGS TO KNOW" in "mir_external.c"
            |  in function "e_conv_ids_type()" for the details.
            */
            unsigned int       ref_count;

            /*
            | Parent Object as viewed by parser
            |
            | The parser front-end of the compiler needs to be able to "climb"
            | up a list of Non-Terminals it is 'compiling-into' as it does
            | its work.  This field is for the parser ONLY during compilation,
            | and is not written to the output file.
            */
            struct ids_tag   *parent_obj;

            /*
            | Table of Backpointers into the index from which the
            | corresponding OIDs for this object can be derived.
            |
            | These arrays are used by I_Register_OID() (in mir_intermediate.c)
            | to keep track of the OIDs assigned to this object.  The contents
            | of these arrays are written in 'boiled-down' form into the output
            | file.
            |
            | An 'empty' entry for any SMI is marked by NULL backpointer.
            */
            struct ids_tag  *oid_backptr[MAX_OID_SMI]; /* Pointer->indexslice*/
            unsigned int     oid_backarc[MAX_OID_SMI]; /* Last OID Arc Value */

            /*
            |  The relationship table and it's management cells
            */
            unsigned short   entry_count; /* Count of entries in Rel. Table */
            unsigned short   max_count;   /* Maximum entries in Rel. Table  */

            struct RES {   /* Relationship table Entry Structure */
               struct ids_tag *rel_obj;   /* Relationship Obj Internal Ptr  */
               struct ids_tag *tar_obj;   /* Target Object Internal Ptr     */
               }
                        *rel_table;

            /*
            |  The cells in this union are set only for certain Non-Terminals
            |  as noted.
            */
            union  {

                /* "I_NT_OBJECT_Rel"
                |
                |  This field is only used in those NTs of flavor
                |  "I_NT_OBJECT_Rel" (not very many at all in reality).  We pay
                |  a price for this extravagance, as most instances of this
                |  union type are "I_NT_OBJECT", but it makes the code Very
                |  Clean.  Note that if some need arises, the value in this
                |  field never goes above the number of NTs of flavor
                |  "I_NT_OBJECT_Rel" (ie, the number of entries in enumerated
                |  type "mir_relationship"), so it could readily be converted
                |  to a SHORT, if the rest of the space was needed for another
                |  field.  We make this "int" to force word alignment (see
                |  Double Note above).
                |
                |  Note: There are really two uses for this field.  During
                |        compilation (before the final pass by code in
                |        "mir_external.c"), the field holds the
                |        "mir_relationship" enumerated value for the MIR
                |        Relationship starting at ZERO for
                |        "MIR_Relationship_Name".  This can be regarded as the
                |        "internal" synonym value.  During the final pass this
                |        field is loaded with the "external" synonym, which
                |        starts with ONE for "MIR_Relationship_Name"! So
                |        wonderful... but you've been told, at least, and
                |        didn't have to discover it yourself!).
                */
                unsigned int    synonym;

                /* "I_NT_OBJECT" & "I_NT_OBJECT_DC"
                |
                |  This field is used for Non-Terminals of flavor
                |  "I_NT_OBJECT" (general) and "I_NT_OBJECT_DC".  Used during
                |  MERGE and AUGMENT compiler modes, this serves to indicate
                |  where-and-when this object was defined (with respect to
                |  the current file being compiled).
                |
                |  Note: See macros "mirc_get_def_status()" and
                |        "mirc_set_def_status()" defined immediately after
                |        the definition of "DEF_STATUS" above in this file.
                */
                DEF_STATUS      ds;

                } ntu;  /* (Non-Terminal's Union) */

            } nt;  /* Non-Terminal Object */
                

        /*
        | TERMINAL Internal Data Structure
        |
        |
        |   flavor = I_T_OBJ_unumber, I_T_OBJ_snumber  &
        |            I_T_OBJ_string
        |
        |   This structure represents a "Terminal" MIR Object.  Such objects
        |   do not contain "pointers" to other MIR Objects, hence they are
        |   "terminal" (in the graph-theory sense) by being leaves in the
        |   tree of MIR Objects that make up the MIR.
        |
        */
        struct term {

            /*
            |  This reference count is not used uniformly for all flavors.  See
            |  the discussion under "OTHER THINGS TO KNOW" in "mir_external.c"
            |  in function "e_conv_ids_type()" for the details.
            */
            unsigned int       ref_count;

            union {

                struct {    /* if flavor = I_T_OBJ_snumber */
                    int     value;    /* The Signed Number */
                    } snumber;

                struct {    /* if flavor = I_T_OBJ_unumber */
                    unsigned int  value;   /* The Unsigned Number */
                    } unumber;

                struct {    /* if flavor = I_T_OBJ_string */
                    int           xr_index; /* Cross-Reference Array Index */
                    unsigned int  len;      /* Length of string            */
                    char          *str;     /* -> The string               */
                    } string;

                } t_v;

            } t;        /* Terminal Dictionary Object */

        } idsu;     /* Intermediate Data Structure Union */

    } IDS; /* Intermediate Data Structure */


/*
|==============================================================================
| Forward-Reference Block Types
|
|   The various flavors of the Forward-Reference blocks are distingquished by
|   the following enumerated values.
|
|   Note that type "FR_STRING" is special and distinct from the others, as it
|   is used to carry only a "string" in a linked list of "FR_STRING" blocks,
|   where the header is typically inside one of the other kinds of
|   Forward-Reference blocks.
|
|==============================================================================
*/
typedef
    enum {
        FR_STRING,      /* For storing a string in a linked-list of strings  */
        FR_IDENT,       /* For storing "IDENTIFIER = () " forward references */
        FR_DEPENDS      /* For storing "DEPENDS ON = () " forward references */
        } FR_TYPE;


/* 
|==============================================================================
| Forward-Reference Blocks
|
|   This definition serves to contain any sort of forward reference information
|   needed to resolve all forward references in an MSL file.
|
|   Except for type FR_STRING, each type of Forward-Reference block usually
|   applies to just one semantic processing-instance that needs to be
|   deferred until the compiler has "seen-it-all".
|
|   The FR_STRING type of block is usually used to create lists whose header
|   resides in one of the other kinds of forward reference blocks.
|
|==============================================================================
*/
typedef

struct fr_tag {

    FR_TYPE             flavor;         /* What kind of Fwd-Ref Block     */
    struct fr_tag       *next;          /* --> Next in linear series      */

    union {

        /*
        | "STRING" - Found in Forward References
        |
        |   flavor = FR_STRING
        |
        |   The structure is used to contain (on a linked list out of one
        |   of the other kinds of FR blocks) all the strings that might
        |   be needed to resolve an instance of a forward reference.
        |   What a string means depends on which Forward-Reference block
        |   the list of these strings is hung off of.
        |
        */
        struct {
            char        *string;   /* --> Dynamically allocated storage for */
                                   /*     a null-terminated string          */
            } s; /* FR_STRING */


        /*
        | "IDENTIFIER =" Forward References
        |
        |   flavor = FR_IDENT
        |
        |   The structure is used to contain all the names of attributes
        |   mentioned in a single instance of an "IDENTIFER = ()" MSL clause
        |   so that they may be resolved after the parser has finished
        |   examining the entire MSL file.
        |
        */
        struct {
            int         lineno;     /* yylineno of "IDENTIFIER = ()" clause  */
            IDS         *current;   /* IDS address of object current at time */
                                    /*  "IDENTIFIER=" clause encountered     */
            struct fr_tag 
                        *attr_names;/* Head of list of FR_STRING blocks with */
                                    /* names of all attributes in IDENT(list)*/
            } i; /* FR_IDENT */


        /*
        | "DEPENDS ON =" Forward References
        |
        |   flavor = FR_DEPENDS
        |
        |   The structure is used to contain all the information (extracted by
        |   the yacc grammar action code) mentioned in a single instance of an
        |   "DEPENDS ON = " MSL clause.
        |
        |   
        */
        struct {
            int         lineno;       /* yylineno of "DEPENDS ON = ()" clause*/
            char        *char_attr;   /* -> Characteristic Attribute Name    */
            IDS         *eclass;      /* IDS of applicable Entity Obj        */
            IDS         *depends;     /* IDS of Depends-On Object            */
            int         operator;     /* MCC_K_VAR_OP_EQUAL or _OP_IN_SET    */
            struct fr_tag *enum_list; /* Header of list of Enumeration Names */
            } d; /* FR_DEPENDS */

        } fru;     /* Forward-Reference Union */

    } FRB; /* Forward-Reference Block Structure */



/*
|==============================================================================
|   Datatype Index Block
|
|   This structure is meant to be strung on a linked list.  Each structure
|   describes an instance of a datatype, either "built-in" (ie defined by
|   the SMI) or "built-up" (created by a TYPE statement in an MSL
|   file).  A pointer to a string containing the name of this datatype is
|   placed in the index structure.  The structures are linked LIFO, with
|   most-recent at the top of the list.
|
|   The list headers are in mir_backend.c, one each for the "built-in" and
|   "built-up" datatype lists.
|
|   Note that as an afterthought, these same structures are used in a list
|   to keep track of the valid keywords (for INCLUDE statements)
|   "legel_keyword_list" & "selected_keyword_list" in the backend context...
|   in this usage the "nt" is set NULL, we just use the next and name cells.
|
|==============================================================================
*/
typedef
    struct index_tag {
            struct index_tag    *next;  /* Next on the linked list          */
            char                *name;  /* Datatype's Name                  */
            int                 len;    /* Datatype's Name's length         */
            IDS                 *nt;    /* ->Non-Terminal for this datatype */
            }   IDX;    /* Index Data Structure */


/*
|==============================================================================
|   MIR Compiler Status Code List
|
|   This is a collection of status codes that may be returned by functions
|   used by the MIR Compiler.
|
|   NOTE:  If you expand this list, function "mirc_error()" in mir_backend.c
|          must be modified accordingly.
|==============================================================================
*/
typedef
    enum {

        /* Generic Function Return Codes for MIR MSL Compiler functions */
        MC_OUT_OF_MEMORY,     /* Ran out of memory during operation */
        MC_SUCCESS,           /* Operation completed successfully   */
        MC_FAILURE,           /* Operation failed in some manner    */

        /* Function "I_Register_OID"-specific Return Codes */
        MC_REG_Already_OID,   /* Specified Object Id is already registered */
        MC_REG_Already_OBJ,   /* Specified OID is already reg. to Object   */
        MC_REG_SMI_Already,   /* OID in SMI has already been reg. for obj  */

        /* Function "I_Find_OID"-specific Return Codes */
        MC_FIND_Partial,      /* Partial match found             */
        MC_FIND_Exact,        /* Exact match found               */
        MC_FIND_Exact_Short,  /* Exact (but short) match found   */
        MC_FIND_Exact_Long,   /* Exact (but long) match found    */

        /* Function "mirc_find_datatype"-specific Return Codes */
        MC_DATATYPE_NOT_FND,  /* Couldn't find datatype on internal list */

        /* Function "mirc_frontend_op"-specific Return Codes          */
        MC_EXT_FAIL,          /* Externalization Failed               */
        MC_NO_PARENT,         /* No parent exists for "Use Parent" op */

        /* Function "open_include_file"-specific Return Codes            */
        MC_OPENFAIL_CONTINUE, /* Error opening include file, continuing  */
        MC_INC_LIMIT_EXCEEDED,/* Too many nested include files           */

        /* Function "mirci_fetch_next()" in "mir_internal.c"               */
        MC_END_OF_PARTITION   /* End of Partition in MIR database file hit */

        /* If you add more right here, be sure to change the limit check */
        /* in mirc_error() in addition to adding messages for new codes  */
        }
            MC_STATUS;

/*
|==============================================================================
|
|  Address Cross-Reference Structure
|
|  Arrays of these critters are created by functions in "mir_internal.c".
|
|  As the functions copy existing MIR objects from an external MIR database 
|  file to the Intermediate in-heap representation, they record the in-heap
|  address along with the corresponding external address for each MIR Object
|  in an instance of one of these structures.  Once all the copying is
|  done, the arrays constitute a "cross-reference" between external and
|  internal addresses for all MIR Objects (in the heap and in the file).
|
|  Once completed, "mir_internal.c" function "mirci_addr_xlate()" makes a
|  linear pass through all in-heap MIR Objects and converts the copied
|  external pointer addresses into the proper corresponding in-heap addresses.
|
|  There is an array of these for each partition (each MIR Object type).
|  The array for MIR Objects that are strings is also used for string-lookup
|  during subsequent compilation of an ASCII MSL file.
|==============================================================================
*/
typedef
    struct {
        masa    ext_add;   /* The External Address for MIR Object           */
        IDS    *int_add;   /* The Internal (in-heap) Address for MIR Object */
        } ext_int_xref;


/*
|==============================================================================
|  Intermediate Context
|
|  This data structure provides the context in which the functions in
|  module "mir_intermediate.c" perform their operations.
|
|  As the compiler runs, it builds a representation of what it has seen in
|  "Intermediate Data Structures" ("IDS").  These structures are maintained
|  on lists, and all the list heads are gathered here in this structure, in
|  "flavors[]".
|
|  Each list contains only one "flavor" of IDS.  Each list is eventually
|  copied into a separate partition in the output MIR database file.
|
|  Associated with "flavors[]" are the "**last_..." pointers that serve to
|  make it easy to add new IDS structures *to the end* of the corresponding
|  lists maintained in "flavors[]".  These particular flavors we want written
|  to the output file in the order they were created, and the final pass of
|  the compiler swings down these linear lists from top to bottom writing,
|  so we want to "add to the bottom" for each of these lists.
|
|  For use during the reading-in of an existing MIR database file, we maintain
|  here lists of cross-reference structure arrays "xref" (used to convert
|  pointer addresses in "external" form to "in-heap" form.  See
|  "mir_internal.c".  Once an existing MIR database file is read in, these
|  arrays are deleted, with the exception of flavor "I_T_OBJ_string" which is
|  retained to aid in fast lookup of strings required for further compilation.
|
|  As can be seen by examining the "bec" data-structure, this Intermediate
|  Context structure is actually a part of the "Back-End Context" (see
|  below).  It is "registered" with the Intermediate code module.
|
|==============================================================================
*/
typedef
   struct {

   /*
   |  Array of "Top of the List" of each "flavor" of Intermediate Data
   |  Structure.  By using a value of enumerated type "IDS_TYPE" as index,
   |  you obtain the address of the first IDS of that flavor.
   |
   |  Entries are added to these lists either as a consequence of reading-in
   |  an existing MIR database file or through compiling an ASCII MSL file
   |  or both.
   */
   IDS    *flavors[MAX_IDS_TYPE];


   /*
   |  Pointers to the last entry on each of the lists for *Non-Terminals*
   |  shown above in "flavors[]".  (Initially these cells contain "NULL").
   |
   |  Note: This array has entries for Terminals, but Terminals are ordered
   |        by value, so we never reference those elements in this array
   |        for Terminals (that is, there is never a need to insert at the
   |        end of the list, for Terminals, the insertions are typically in
   |        mid-list according to order).
   */
   IDS    *last_of_flavor[MAX_IDS_TYPE];


   /*
   |  Array of "Top of Free List" of each "flavor" of Intermediate Data
   |  Structure.  By using a value of enumerated type "IDS_TYPE" as index,
   |  you obtain the address of the first "free" IDS of that flavor.
   |
   |  The functions in "mir_intermediate.c" that 'create' an IDS (of a
   |  particular flavor) on behalf of a caller check an entry in this array
   |  first to see if a free IDS of the right flavor is available.  The
   |  lists are circular, and entries are removed 'from the top' (ie via
   |  this array).
   */
   IDS    *top_of_free[MAX_IDS_TYPE];


   /*
   |  Array of "Bottom of Free List" of each "flavor" of Intermediate Data
   |  Structure.  By using a value of enumerated type "IDS_TYPE" as index,
   |  you obtain the address of the last "free" IDS of that flavor, and it
   |  is after this last entry that a new 'free' IDS is stashed for future
   |  reference.
   |
   |  The function in "mir_intermediate.c" that 'reclaims' an IDS (of any
   |  flavor, "I_Reclaim_IDS()) on behalf of a caller checks an entry in this
   |  array to see if a free IDS of the right flavor exists at the end of the
   |  list of free IDSs of this flavor.  The lists are circular, and entries
   |  are added 'to the bottom' (ie via this array).
   */
   IDS    *last_of_free[MAX_IDS_TYPE];


   /*
   |  Array of pointers to arrays of "Address Cross-Reference" structures.
   |
   |  Each entry in this array points to a solid block of heap storage
   |  organized as an array of "ext_int_xref" structures.
   |
   |  Each such array corresponds to one flavor's (partition) worth of MIR
   |  Objects as read-in from an existing MIR database file.
   |
   |  Each entry in the array corresponds to one MIR Object of that flavor,
   |  and after correct initialization, contains that MIR Object's external
   |  (file) and internal (in-heap) address.
   |
   |  All of these arrays are dispensed with after successfully reading-in
   |  a MIR database file (and before starting compilation on any ASCII MSL
   |  files) *except* for the xref[I_T_OBJ_string] array, which is kept
   |  and used by "I_Create_IDS_STRING()" in "mir_intermediate.c" to
   |  facilitate compile-time lookups of strings during the ASCII MSL
   |  processing.
   |
   |  By using a value of enumerated type "IDS_TYPE" as index, you obtain the
   |  address of the array of "ext_int_xref" for that flavor.
   */
   ext_int_xref   *xref[MAX_IDS_TYPE];


   /*
   |  Array of sizes of xref[] arrays.
   |
   |  The value of each entry in this array contains the size-in-entries
   |  of the corresponding array entry in xref[].  In other words,
   |  "xref[I_SLICE]" is the address of an array of "ext_int_xref" structures
   |  and there are "xref_count[I_SLICE]" entries in that array.
   |
   |  Note that like the "xref[]" array, this array's contents becomes
   |  superfluous after a MIR database file has been read-in by functions in
   |  "mir_internal.c" *except* for the "xref_count[I_T_OBJ_string]" entry
   |  which continues to be used by "I_Create_IDS_STRING()" in
   |  "mir_intermediate.c".  I_Create_IDS_STRING() will use both array entries
   |  *even* if a MIR database file is not read-in.  See the code.
   */
   int    xref_count[MAX_IDS_TYPE];


   /*
   |  Top of the Intemediate Data Structure Index
   |
   |  This points to IDS structure corresponding to Index Slice for the
   |  leftmost arcs in all ISO Object IDs in the index... (typically with
   |  just one entry: arc = "1"!).
   */
   IDS    *index_top;


   /* Longest OID registered in index */
   int    arc_count;


   } inter_context;
             /* Intermediate Context */


/*
|==============================================================================
|  Back-End Context
|
|  This data structure contains the entire operational context for the
|  MIR Compiler's "back-end" (the functions in file mir_backend.c).
|
|  These data are isolated in the structure to allow the front end of the
|  compiler to "register" an instance of this data structure with the
|  back-end code through function "mirc_init_backend()".  This function
|  merely stores a pointer to the provided instance of this structure that
|  the other functions that make up the "mir_backend.c" module can reference.
|
|  All functions in the backend continue to reference this data context until
|  another instance of this context is established.
|
|  (This data structure and "mirc_init_backend()" serve the purpose of
|  allowing the back-end functions to operate on more than one instance of
|  "a heap full of data being compiled".  This is in anticipation of the
|  need to enhance the compiler to be able to "merge" MIR databases).
|==============================================================================
*/
typedef
   struct {

   /* - - - - - - - - - - - - - - - -
   |   The next two arrays are for use by the compiler regardless of the
   |   SMI it is "compiling under".  They are used primarily in parsing
   |   the "builtin_types.dat" file that contains string-names of internal
   |   compiler MIR relationship objects opposite the "TEMPLATE" keyword, as
   |   well as for quick reference to an IDS for a particular MIR relationship
   |   in back-end functions.
   */

   /*  map_rel_to_ids - Map Relationship Name to IDS that represents it
   |
   |   This array, using an enumerated value of type "mir_relationship" as the
   |   index, gives a pointer to the IDS for that relationship.
   */
   IDS *map_rel_to_ids[REL_COUNT];


   /*  map_relstr_to_ids - Map Relationship Name as String to (Index) of IDS
   |                       that represents it
   |
   |   This array contains pointers to strings that are relationship names.
   |   Given a relationship name as a string, this array can be searched
   |   linearly until a match is found.  Using the value of the index
   |   (that occurred on a match) in "map_rel_to_ids[]" (above) renders the
   |   pointer to the IDS of MIR Relationship Object of the given name.
   */
   char *map_relstr_to_ids[REL_COUNT];

   /* - - - - - - - - - - - - - - - - */


   /* map_smicode_to_smistr - Map an SMI Code to the string representation
   |                          for that SMI
   |
   |    This array of pointers to strings is built by mirc_record_smi()
   |    as the parser parses the "builtin_types.dat" file.
   |    Each new definition of an SMI encountered in the "builtin_types.dat"
   |    file starts with a line beginning with
   |
   |            START_SMI <string> = <number> :
   |
   |    The string is stored in this array as the "name" of the SMI using
   |    as an index the number in parentheses.  The number in parentheses
   |    MUST be the value assigned to the enumerated symbol corresponding
   |    to the SMI by the definition of "mir_smi_code" (above).
   |
   |    This array allows mirc_set_smi() (when called from the parser)
   |    with a string extracted from the "DEFINING-SMI = " clause) to
   |    discover the corresponding "mir_smi_code" that defines the SMI
   |    within the compiler (it is the index of the element in this array
   |    that points to the name of the smi).
   |
   |    A "NULL" in entry in this array means we didn't parse a name from
   |    the builtin-types file for this smi code.
   |
   */
   char *map_smicode_to_smistr[MAX_SMI];


   /*
   |   This cell indicates which is the "current" SMI (whose built-in data
   |   type definitions are to be used) for compiling the next line of MSL.
   |
   |   By using the value of this cell as the index into the arrays of
   |   pointers to lists of SMI datatypes (maintained below), the compiler
   |   can select the right list to search when trying to recognize a datatype
   |   for a particular SMI.
   */
   mir_smi_code current_smi;


   /* Array of SMI "Built-In" Data-Construct Linked-List Heads
   |
   |   The elements in this array are selected by the value of "current_smi"
   |   (above) as the compiler runs.  Each element is a pointer to a list of
   |   the "built-in" datatypes defined in that SMI.
   |
   |   These lists are constructed by the compiler from a supplied
   |   text file, (typically "builtin_types.dat", but also from the contents
   |   of a binary MIR database file that has been loaded---which contains a
   |   copy of the contents of "builtin_types.dat"....) which defines all the
   |   datatypes that the compiler can recognize for each SMI.  
   |
   |   Each entry on a list contains the SMI name (string) for a 
   |   "built-in-datatype" (like Unsigned16) and a pointer to the Non-Terminal
   |   that describes that datatype (or "data-construct").
   |
   |   Whenever the compiler parser encounters a string that should be a
   |   datatype name for the SMI, it searches a list first to determine if it
   |   is a datatype "built-in" to the SMI.  If the datatype name is not found
   |   here, then it should be found on the corresponding "built-up" list
   |   (whose header is defined below).
   |
   |   Note that in addition to pure "built-in" datatypes (like "Integer" and
   |   "Unsigned16" et.al) these lists also contain what the compiler considers
   |   to be "built-in templates" to be used in defining instances of built-up
   |   datatypes.  These built-in templates are for built-up datatypes such as
   |   "enumeration", "record" and the like.
   |
   |   The MIR term "data construct" encompasses "built-in" datatypes,
   |   "built-in" templates and "built-up" datatypes.
   |
   */
   IDX  *built_in[MAX_SMI];


   /* Array of SMI Built-Up Data-Constructs Linked-List Heads
   |
   |   The elements in this array are selected by the value of "current_smi"
   |   (above) as the compiler runs.  Each element is a pointer to a list of
   |   the "built-up" datatypes defined (in the selected SMI) by the
   |   TYPE statements encountered so far during the course of the
   |   compilation within this SMI.
   |
   |   This list is constructed by the compiler as is parses the TYPE
   |   statements in an MSL.  By linking all such MSL defined ("built-up")
   |   instances of certain datatypes together on this list (thru the IDX
   |   structures used to construct the list) when a subsequent TYPE,
   |   ATTRIBUTE or ARGUMENT references a previously defined instance of
   |   a data-construct, this list is searched (linearly) for the MIR
   |   Intermediate Non-Terminal that represents that definition (after
   |   an attempt to find it on the "built-in" data-construct list fails).
   |
   |   In the V1.5 version, the compiler erases each of the lists maintained
   |   here when it encounters the end-of-file on the current MSL file.
   |   In future versions, such erasure may not occur, and existing functions
   |   to search these lists will be enhanced to check to see if an entry
   |   on the list is already present and whether it matches what is about
   |   to be put on a list.
   |
   */
   IDX  *built_up[MAX_SMI];


   /* - - - - - - - - - - - - - - - -
   |   The next four arrays are used in conjunction with one another to
   |   record the text-name, MCC Code, Standard Code and entity element type
   |   indicator for one "entity element" (such as entity class, attribute
   |   directive, argument etc).  Each array is a stack of whatever it
   |   contains, for all the entity elements "at and above" the
   |   "current_object" being compiled into.  (In other words, they form
   |   a snapshot of the path through the containment hierarchy to the
   |   "current_object").
   |
   |   At any given moment during compilation, the parser variable "level"
   |   indexes the last legal entry in these arrays, (ie the code, name etc)
   |   of the "current_object".
   |
   |   The scheme for entries being added to these arrays is as follows.
   |   "mirc_create_object()" is called by the front-end parser each time
   |   the parser encounters another entity-element that must be converted
   |   into a MIR Object.  This function is responsible for incrementing
   |   "level" and calling "mirc_record_IDcode()" which stores the name, any
   |   MCC ID Code plus the entity-type indicator into the arrays below.
   |
   |   (Note:  The MCC ID Code gets stored as the target of "MIR_MCC_ID_Code",
   |    while the Standard ID Code gets stored as the target of "MIR_ID_Code").
   |
   |   Subsequently the parser may parse a "DNA_CMIP_INT=" clause which
   |   causes "mirc_record_IDcode()" to be called again, this time to store
   |   the DNA_CMIP_INT code as the "Standard" ID code (in the array below).
   |   A "missing" ID Code is stored as a "-1" in either of the ID code arrays
   |   below.
   |
   |   From these arrays, "mirc_compute_oid()" can extract the information
   |   needed to construct an "MCC" OID or a "Standard" (DNA) OID, unless
   |   an object in the hierarchy had no code (ie. "-1") assigned, in which
   |   case the OID cannot be constructed.
   |
   |   NOTE WELL:  Since "level" is used as an index to these arrays and given
   |               that the "minimum" value of level is "1" (for a GLOBAL
   |               entity), that means that the "0th" entry in these arrays
   |               is never used.
   |
   |   This symbol defines how deep this (one-logical) stack can be.
   */
#define MAX_PATH_STACK 25

   /* "textname" Stack
   |
   |   This array, indexed by parser variable "level" contains the string names
   |   of all MSL entities that form a path down to and including the current
   |   object at "level".  A textname string in this array appears as the
   |   target of the "MIR_Text_Name" relationship in each element.
   |
   |   This array exists only to support "mirc_compute_oid()" when it
   |   constructs an error message.  It reports the name of the object that
   |   did not have a code assigned, and this array carries the names of
   |   all objects.
   */
   char *name_stack[MAX_PATH_STACK];


   /* "MCC Code" Stack
   |
   |   This array, indexed by parser variable "level", contains the MCC Codes
   |   of all MSL entities that form a path down to and including the current
   |   object at "level".  A code in this array appears as the target
   |   of the "MIR_MCC_ID_Code" relationship in each element.
   |
   |   This array exists only to support "mirc_compute_oid()" when it
   |   constructs an MCC OID.  A code of "-1" in any entry in this stack
   |   means the entity element was defined in the MSL without an MCC code.
   */
   int mcc_code_stack[MAX_PATH_STACK];


   /* "Standard Code" Stack
   |
   |   This array, indexed by parser variable "level", contains the "standard"
   |   Codes of all MSL entities that form a path down to and including the
   |   current object at "level".  A code in this array appears as the target
   |   of the "MIR_ID_Code" relationship in each element.
   |
   |   This array exists only to support "mirc_compute_oid()" when it
   |   constructs an "standard" (DNA) OID.  A code of "-1" in any entry in
   |   this stack means the entity element was defined in the MSL without a
   |   standard (DNA_CMIP_INT) code.
   */
   int std_code_stack[MAX_PATH_STACK];


   /* "Entity-Element Type" Stack
   |
   |   This array, indexed by parser variable "level", contains the enumerated
   |   values indicating the type of all MSL entities that form a path down to
   |   and including the current object at "level".
   |
   |   This array exists only to support "mirc_compute_oid()" when it
   |   constructs either kind of OID.  Given the algorithms in the Entity
   |   Model document, an arc value indicating the kind of entity-element
   |   something "is" must be built into the OID.  This stack provides the
   |   information for that arc.
   |
   |   NOTE:  "mirc_create_object()" will use as this value the MIR
   |          relationship code that specifies the relationship of
   |          "this-thing's-parent" to "this-thing".  This might lead to
   |          human-symbol confusion:  If an entry describes an "attribute",
   |          the mir relationship code will be "MIR_Cont_attribute".  Don't
   |          worry, "mirc_compute_oid()" is coded properly, just don't get
   |          confused by it.  This saves having to whip up a special set of
   |          enumerated values just for this one purpose.
   */
   mir_relationship ent_type_stack[MAX_PATH_STACK];

   /* - - - - - - - - - - - - - - - - */


   /* "Current" Object
   |
   |   This is the pointer into the Intermediate Data Structure (compiler)
   |   heap to the one object that is "current", in other words, the one
   |   MIR object that the front-end parser is "compiling into".
   */
   IDS  *current_object;


   /* "Forward-Reference List"
   |
   |   All instances of Forward-References that need resolution (when the end
   |   of an MSL is encountered) cause an "FRB" to be strung onto this list
   |   for resolution by mirci_resolve_fwd_ref().  (Ie. each FRB on this
   |   list represents a forward-reference needing attention after the parse
   |   is complete.  There are several types of forward-reference situations,
   |   each with their own flavor of FRB).
   */
   FRB  *fwd_ref_list;


   /* "Built-in Type Version String"
   |
   |   This string is extracted from The World MIR Object in a binary
   |   MIR database file that is being loaded for modification.  We store
   |   the string here so that the front-end can compare it with the version
   |   string extracted from the Built-In Types file that was shipped with
   |   the compiler.  A warning/info message is issued if they differ.
   */
   char *binary_bidt_version;


   /* "Legal Keyword List"
   |
   |  This is a list of IDX elements whose "name" value is actually the
   |  value of a legal keyword (extracted either from the input binary MIR
   |  database file or the "builtin_types.dat" file).
   |
   |  Note that the "nt" and "len" cells in these structures are not used
   |  within this list.
   */
   IDX  *legal_keyword_list;


   /* "Selected Keyword List"
   |
   |  This is a list of IDX elements whose "name" value is actually the
   |  value of a "selected" keyword (ie, named on the command-line with "-k").
   |
   |  Note that the "nt" and "len" cells in these structures are not used
   |  within this list.
   */
   IDX  *selected_keyword_list;


   /* "Object-Created Flag"
   |
   |  When the compiler is operating in "Merge" or "Augment" mode, we
   |  want to be able to report to the user whether or not the Augment
   |  or Merge *actually* added information to the current MIR compilation.
   |
   |  This flag is initialized by mirc_init_backend(), set by
   |  mirc_create_object() when it creates an object and examined (and an error
   |  message generated) and reset by mirc_reset().
   */
   BOOL  object_created;


   /* "Intermediate Context"
   |
   |   A "sub-context" of the MIR Compiler back-end, this structure contains
   |   all the pointers to the things being compiled and some extra info.
   |   The functions in "mir_intermediate.c" operate within this context,
   |   see the "inter_context" definition above.
   */
   inter_context    i_context;

   } back_end_context;   /* Back-End Context */


/*
|==============================================================================
|
|   Define Prototypes for all MIR Compiler "Frontend" Functions
|   "mir_frontend.c"  (and "mir_yacc.y"/"mir_lex.l").
|==============================================================================
*/
/* mirf_elapsed - Compute Elapsed-Time string for compiler statistics */
void
mirf_elapsed PROTOTYPE((
int     ,       /* IN: Elapsed time in seconds */
char    *       /* OUT: Message in text buffer */
));

void
set_top_file PROTOTYPE((
char *
));

void
warn PROTOTYPE((
char *
));

/* to get YACC to print out line number of error */
void
yyerror PROTOTYPE((
char *
));

void
info PROTOTYPE((
char *
));

int
yyparse PROTOTYPE((
));

MC_STATUS
open_include_file PROTOTYPE((
char *
));

void
parse_depends_expr PROTOTYPE((
char *
));

int
inputch PROTOTYPE((
));

/* Defined by lex via mir_lex.l or for mir_lex.c */
int
yylex PROTOTYPE((
));

int
begin_enum PROTOTYPE((
));

int
begin_initial PROTOTYPE((
));

int
yywrap PROTOTYPE((
));

/*
|==============================================================================
|
|   Define Prototypes for all MIR Compiler "Backend" Functions
|   "mir_backend.c"
|==============================================================================
*/
/* mirc_create_dataconstruct - Create a Built-In or Built-Up Data Construct */
MC_STATUS
mirc_create_dataconstruct PROTOTYPE((
char    *,      /*-> Name of builtup datatype               */
int      ,      /* SMI-defined code for builtup datatype    */
BOOL            /* TRUE: Built-IN, FALSE: Built-UP          */
));

/* mirc_add_relationship - Add relationship/target to current object */
MC_STATUS
mirc_add_relationship PROTOTYPE((
mir_relationship     ,  /* Code for  Relationship Non-Term to add */
IDS                 *   /* -> to Target to add                    */
));

/* mirc_add_rel_string - Add relationship/string-target to current object */
MC_STATUS
mirc_add_rel_string PROTOTYPE((
mir_relationship     ,  /* Code for  Relationship Non-Term to add */
char                *   /* -> string to convert to Target to add  */
));

/* mirc_add_rel_number - Add relationship/number-target to current object */
MC_STATUS
mirc_add_rel_number PROTOTYPE((
mir_relationship     ,  /* Code for  Relationship Non-Term to add */
int                  ,  /* number to convert to Target to add     */
mir_value_type          /* Indicates "Signed" or "Unsigned"       */
));

/* mirc_find_datatype - Search compiler internal list for datatype IDS */
MC_STATUS
mirc_find_datatype PROTOTYPE((
char    *,    /* --> string that identifies a builtin/up datatype */
IDS     **    /* Address of a pointer to be set to the IDS of the "datatype" */
));

/* mirc_find_DC_code  - Find (builtin) DataConstruct's Code */
int
mirc_find_DC_code PROTOTYPE((
IDS    *       /* --> IDS for the (builtin) DataConstruct to be searched */
));

/* mirc_find_obj_CODE  - Find any (SMI) Object's (MCC or Standard) Code */
int
mirc_find_obj_CODE PROTOTYPE((
IDS             *,     /* --> IDS for the SMI Object to be Searched */
CODE_TYPE              /* Type of Code, MCC or Standard             */
));

/* mirc_find_obj_NAME  - Find any (SMI) Object's "NAME" */
char *
mirc_find_obj_NAME PROTOTYPE((
IDS             *              /* --> IDS for the SMI Object to be Searched */
));

/* mirc_create_object - Create MIR Object (Non-Datatype) */
MC_STATUS
mirc_create_object PROTOTYPE((
mir_relationship    ,  /* The relationship the parent has to this object     */
int                 ,  /* The SMI-defined code by which this object is known.*/
char                *, /* The SMI-defined name by which this object is known.*/
int                 *  /* The level (a la DNU dictionary) into the */
                       /* MSL where "code" is defined.             */
));

/* mirc_init_backend - Initialize the backend of compiler */
MC_STATUS
mirc_init_backend PROTOTYPE((
back_end_context *,     /* Backend Context block               */
char             *      /* Binary Input MIR Database file name */
));

/* mirc_find_rel_IDS - return a pointer to a compiler relationship IDS */
IDS *
mirc_find_rel_IDS PROTOTYPE((
char        *           /* Which relationship we want (as a string) */
));

/* mirc_find_obj_IDS - Finds (by name) the IDS for an object in "c.o." table */
IDS *
mirc_find_obj_IDS PROTOTYPE((
mir_relationship     ,  /* Relationship from Current-Object to desired */
char                *   /* Name of the object desired                  */
));

/* mirc_record_idcodes - Record ID Codes in path Stack */
void
mirc_record_idcodes PROTOTYPE((
int               ,             /* Level in MSL where ID code is defined   */
CODE_TYPE         ,             /* Indicates "MCC" or "Standard" ID code   */
int               ,             /* The ID code value                       */
mir_relationship *,             /* If non-NULL, the relationship indicator */
char             *              /* If non-NULL, the name of entity element */
));

/* mirc_find_fixedfield - Finds fixed-field code in an instance of a RECORD */
int
mirc_find_fixedfield PROTOTYPE((
char     *           /* Name of the fixed field whose code is sought*/
));

/* mirc_create_fwd_ref - Create Forward Reference Block */
MC_STATUS
mirc_create_fwd_ref PROTOTYPE((
FR_TYPE   ,         /* The type of forward-reference block to create   */
FRB       **,       /* If non-NULL, addr of where to return ptr to blk */
char      *         /* -> string (if block_type = FR_STRING), or NULL  */
));

/* mirc_rec_to_vrec - Convert instance of RECORD to inst. of VARIANT RECORD */
MC_STATUS
mirc_rec_to_vrec PROTOTYPE((
IDS     *            /* --> IDS describing current instance of RECORD */
));

/* mirc_use_parent - Use parent of Current Object as new Current Object */
MC_STATUS
mirc_use_parent PROTOTYPE((
int     *      /* --> Level Indicator maintained by front end */
));

/* mirc_get_current - Return the IDS Pointer to the Current Object */
IDS *
mirc_get_current PROTOTYPE((
));

/* mirc_set_current - Set the Current Object to a particular MIR Object */
void
mirc_set_current PROTOTYPE((
IDS     *       /* --> To Object to become the "new" current-object */
));

/* mirc_reset - Reset the MIR Compiler Backend for further operation */
MC_STATUS
mirc_reset PROTOTYPE((
));

/* mirc_record_smi - Record the datatypes under this specified SMI  */
MC_STATUS
mirc_record_smi PROTOTYPE((
char         *,       /* --> String that names the SMI                    */
mir_smi_code          /* --> SMI code associated with this SMI            */
));

/* mirc_get_def_status - Get the "Definition-Status" of the current object */
DEF_STATUS
mirc_get_def_status PROTOTYPE((
));

/* mirc_set_def_status - Set the "Definition-Status" of the current object */
void
mirc_set_def_status PROTOTYPE((
DEF_STATUS            /* Definition-Status to set */
));

/* mirc_set_smi - Set the Current SMI within which we are compiling */
MC_STATUS
mirc_set_smi PROTOTYPE((
char         *,       /* --> String naming the SMI                        */
mir_smi_code *        /* --> cell to receive selected SMI's internal code */
));

/* mirc_compute_oid - COMPUTE the MCC or DNA Object ID of a MIR object */
char *
mirc_compute_oid PROTOTYPE((
int               ,  /* Front-End Level indicator             */
CODE_TYPE         ,  /* Indicates "MCC" or "Standard" OID req */
object_id       **   /* -->> to receive ISO Object IU         */
));

/* mirc_error - Returns Error string given MIR Compiler error Status Code */
char *
mirc_error PROTOTYPE((
MC_STATUS          /* The error status in need of interpretation */
));

/* mirc_keyword - Register or Verify a Keyword string */
BOOL
mirc_keyword PROTOTYPE((
KW_OPCODE       ,        /* Register or Verify               */
char            *        /* --> String to Register or Verify */
));

/* Caseless String Comparison */
int caseless_str_equal PROTOTYPE((
char *,
char *,
int
));


/*
|==============================================================================
|
|   Define Prototypes for all Intermediate Representation Functions
|   "mir_intermediate.c"
|==============================================================================
*/

/* I_Register_Context - Register an Intermediate Context Block */
MC_STATUS
I_Register_Context PROTOTYPE((
inter_context  *   /* The Intermediate Context block ptr */
));

/* I_Find_OID - Find Object Id Entry in Intermediate MIR Index */
MC_STATUS I_Find_OID PROTOTYPE((
object_id   *,      /* -> Object Id structure to be inserted       */
IDS         **,     /* Address of pointer to slice being returned  */
int         *,      /* Address of integer to return slice index to */
int         *       /* Address of integer to return match count to */
));

/* I_Register_OID - Register new Object ID in Intermediate MIR Index */
MC_STATUS
I_Register_OID PROTOTYPE((
object_id       *,      /* -> Object Id structure to be inserted    */
mir_oid_smi      ,      /* SMI whose rules used to construct OID    */
IDS             *       /* -> Object in dictionary to be registered */
));

/* I_Insert_Rel_Entry - Inserts a new Relationship into a given Non-Terminal */
MC_STATUS
I_Insert_Rel_Entry PROTOTYPE((
IDS     *,      /* Address of non-terminal being inserted to        */
IDS     *,      /* Address of relationship non-terminal to insert   */
IDS     *       /* Address of target to insert                      */
));

/* I_Create_IDS_SNUMBER - Create a Terminal Object that is a Signed Number */
IDS *
I_Create_IDS_SNUMBER PROTOTYPE((
int        /* Terminal Object: The number */
));

/* I_Create_IDS_UNUMBER - Create a Terminal Object for an Unsigned Number */
IDS *
I_Create_IDS_UNUMBER PROTOTYPE((
unsigned int          /* Terminal Object: The number */
));

/* I_Create_IDS_STRING - Create a Terminal Object that is a String */
IDS *
I_Create_IDS_STRING PROTOTYPE((
char    *             /* Terminal Object: The string */
));

/* I_AddNew_IDS_SNUMBER - Add a New Terminal Object that is a Signed Number */
IDS *
I_AddNew_IDS_SNUMBER PROTOTYPE((
int           /* Terminal Object: The number */
));

/* I_AddNew_IDS_UNUMBER - Add a New Terminal Object: an Unsigned Number */
IDS *
I_AddNew_IDS_UNUMBER PROTOTYPE((
unsigned int       /* Terminal Object: The number */
));

/* I_AddNew_IDS_STRING - Add a New Terminal Object that is a String */
IDS *
I_AddNew_IDS_STRING PROTOTYPE((
char    *                        /* Terminal Object: The string */
));

/* I_Create_IDS - Create a "Not Terminal" Intermediate Data Structure */
IDS *
I_Create_IDS PROTOTYPE((
IDS_TYPE   ,        /* Type of Intermediate Data Structure to create     */
unsigned            /* If NON-ZERO: Minimum req. number of table entries */
));

/* I_Reclaim_IDS - Reclaim an Intermediate Data Structure of any flavor */
void
I_Reclaim_IDS PROTOTYPE((
IDS     *           /* Pointer to IDS to be reclaimed */
));

/*
|==============================================================================
|
|   Define Prototypes for all External Representation Functions
|   "mir_external.c"
|==============================================================================
*/

/* e_gen_external - Generate External Representation of MIR Data */
BOOL
e_gen_external PROTOTYPE((
inter_context *,      /* Intermediate Context Block             */
char          *       /* Filename to use in writing the file    */
));


/*
|==============================================================================
|
|   Define Prototypes for all Internal Representation Functions
|   "mir_internal.c"
|==============================================================================
*/
/* mirc_binary_load - Load binary MIR Database file into MIR Compiler */
MC_STATUS
mirc_binary_load PROTOTYPE((
char              *,      /* Binary MIR database filename to load */
back_end_context  *       /* Pointer to back-end context block    */
));


/*
|==============================================================================
|
|   Define Prototypes for all "Remove" Functions
|   "mir_remove.c"
|==============================================================================
*/
/* mirc_remove_EC - Remove Intermediate Representation of an Entity-Class */
char *
mirc_remove_EC PROTOTYPE((
char              *,      /* --> String containing OID for Object */
back_end_context  *       /* Pointer to back-end context block    */
));


/* mirc_remove_OBJECT - Removes Table Entries, OIDs and then the Object */
void
mirc_remove_OBJECT PROTOTYPE((
IDS               *,       /* --> IDS of MIR object to be removed */
back_end_context  *        /* Pointer to back-end context block    */
));


/*
|==============================================================================
|
|   Define Prototypes for all Miscellaneous Functions
|   "mir_symdump.c"
|==============================================================================
*/

/* mirc_symdump - Symbolic Dump of Linear (External) Representation */
void
mirc_symdump PROTOTYPE((
char *, /* Input file Name      */
char *, /* Output file Name     */
int     /* Raw Index            */
));

#endif
#if defined(__cplusplus)
}
#endif
