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
 * @(#)$RCSfile: mir_yacc.y,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:00:57 $
 */
%{
#ifdef unix
#define module ident
#endif
/* #module MSL_YACC "X1.2.0" */
/*
**
** Copyright © (C) Digital Equipment Corporation 1989-1992, 1993.
** Digital Equipment Corporation, Maynard, Mass.
**
** This software is furnished under a license and may be used and copied
** only  in  accordance  with  the  terms  of such  license and with the
** inclusion of the above copyright notice. This software or  any  other
** copies thereof may not be provided or otherwise made available to any
** other person. No title to and ownership of  the  software  is  hereby
** transferred.
**
** The information in this software is subject to change without  notice
** and  should  not be  construed  as  a commitment by Digital Equipment
** Corporation.
**
** Digital assumes no responsibility for the use or  reliability  of its
** software on equipment which is not supplied by Digital.
**
**
** Networks & Communications Software Engineering
**
*******************************************************************************
**
**  FACILITY:
**
**	MIR (Management Information Repository)
**
**  ABSTRACT:
**
**	This compiler takes an MSL file and compiles it directly into
**      a binary form that is accessed by a run-time access mechanism.
**
**	YACC usage:  the token stack consists of integers, which can be
**	interpreted in one of three ways:
**
**		- as descriptor or object ID codes
**		- as string length of the string in CurStrBuf
**		- as integer or boolean descriptor value or component of value
**
**	it is up to more advanced productions to put together constructed
**	values. Consequently the token stack structure is kept simple, but
**	type casting is needed when the integer on the token stack is
**	converted into a descriptor ID code, which is an enumerated type.
**
**  AUTHOR:
**
**  Derived from DECnet-ULTRIX Dictionary Compiler ("MD") written by:
**      B. M. England
**
**  Enhanced to full MCC MSL syntax & function by:
**      Larry Grossman     17-JAN-1991
**
**  Modified for use with the Common Agent Managaement Information
**  Repository "back-end" by:
**      D. D. Burns        March 1992
**
**
**  MODIFICATION HISTORY:
**
**  Date        By              Reason
**  Oct 1992    D. D. Burns     V1.99 - Add support for MERGE and AUGMENT
**                                      operation
**  Jan 1993    Adam Peller     fix convert_as_needed() for 64-bit alpha arch.
**
**  Jan 1993    D. D. Burns     V2.00 - Make ASN1 Tag/Class optional in
**                                      builtin_types.dat file
*******************************************************************************
**/

/* Do not redefine IO routines */
#define CMA_STDIO 1

#ifdef VMS
#include "vaxcshr.h"	/* Convert VAX C to DEC C for RTL function names */
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>


#include "mcc_interface_def.h"
#include "mcc_vea_def.h"


/* Get the compiler definitions too from "mir.h" */
#define MIR_COMPILER
#include "mir.h"
#include <stdlib.h>
#include <limits.h>

#ifdef DEBUG
#define YYDEBUG DEBUG
#endif

/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern char *mp800();
extern char *mp801();
extern char *mp802();
extern char *mp803();
extern char *mp804();
extern char *mp805();
extern char *mp806();
extern char *mp807();
extern char *mp808();
extern char *mp809();
extern char *mp810();
extern char *mp811();
extern char *mp812();
extern char *mp813();
extern char *mp814();
extern char *mp815();
extern char *mp816();
extern char *mp817();
extern char *mp818();
extern char *mp819();
extern char *mp820();
extern char *mp821();
extern char *mp822();
extern char *mp823();
extern char *mp824();
extern char *mp825();
extern char *mp826();
extern char *mp827();
extern char *mp828();
extern char *mp829();
extern char *mp830();
extern char *mp831();
extern char *mp832();
extern char *mp833();
extern char *mp834();
extern char *mp835();
extern char *mp836();
extern char *mp837();
extern char *mp838();
extern char *mp839();
extern char *mp840();
extern char *mp841();
extern char *mp842();
extern char *mp843();
extern char *mp844();
extern char *mp845();
extern char *mp846();
extern char *mp847();
extern char *mp848();
extern char *mp849();
extern char *mp850();
extern char *mp851();

extern char *mp151();
#endif


/*
|  These global locations corresponds directly to the "-Xi" compiler
|  and "-Xt" flags, and may be set TRUE by "mirf_cmd_args()" in module
|  "mir_frontend.c".
*/
extern BOOL    supplementary_info;
extern BOOL    supplementary_time;


/*
| Handy error detection macro used to de-clutter the yacc source
*/
static MC_STATUS mc_status;     /* reserved for use by TRY macro */
#define TRY(s) if ((mc_status=(s)) != MC_SUCCESS) \
                  {eflag=TRUE; yyerror(mirc_error(mc_status));}


/*
| Maximum size of multiple-keyword string
*/
#define MAX_MULTI_KEYWD_SIZE 2048


/* ======================= SYMBOL support ============================
| Maximum number of characters allowed in a PREFIX symbol.
*/
#define MAX_SYMPREFIX_SIZE 100

/*
| The latest "symbol prefix" parsed from a file gets stashed here, for
| "prefix_size" bytes.
*/
static char prefix_buf[MAX_SYMPREFIX_SIZE];

/*
| If "TRUE", then "prefix_buf[]" contains a prefix symbol that has not
| yet been written into the first entity declared in the file.
*/
BOOL prefix_stored;


/*
| KEYWORD conditional INCLUDE statement structures
*/
BOOL do_include_statement = MCC_K_TRUE;

/* accumulate dictionary object data here */
static char CurStrBuf[MAX_MULTI_KEYWD_SIZE];
static int CurStrBufLen;
static char EClassNameBuf[MAX_MULTI_KEYWD_SIZE];
static int  EClassCode;
static int ObjectCode;


static char *cp, *p_ellipses, filter[]=" \t\n";

/*
| Interface to LEX occurs here
*/
extern int yylineno;	/* linkage to YACC-produced line number */
extern int yyleng;	/* contains int value of token string if numeric */
extern char yytext[];	/* contains token string */

/*
| Strings used as 'names' of Attribute partitions
*/
static
char *AttrPartString[] = 
{
 "Reserved"
,"Identifiers"
,"Status"
,"Counters"
,"Characteristics"
,"References"
,"6_Reserved"
,"7_Reserved"
,"8_Reserved"
,"9_Reserved"
,"Null"
,"11_Reserved"
,"Initial Attributes"
,"Statistics"
,"14_Reserved"
,"Configuration_event"
,"Notification_event"
};


/*
| Parent of Everything (root object)
*/
IDS             *the_world_IDS;


/*
| The grand error flag, MCC_K_TRUE if any syntax errors during compile
*/
int eflag = MCC_K_FALSE;


/*
| Variables used in parsing an OID string
*/
#define MAX_OID_SIZE 100    /* Maximum number of arcs in an OID we can parse */
static object_id      oid;         /* parsed OID is converted & stored here  */
static unsigned int   oid_arcs[MAX_OID_SIZE];  /* The oid arc values go here */
static unsigned int   oid_number;  /* An arc value                           */
mir_oid_smi           oid_type;    /* The kind of OID                        */


/*
| "first-entity" :one-shot boolean flag
|
| This flag supports the proper semantic analysis of a PARENT= clause.
*/
static BOOL first_entity;


/*
| These counters count the number of attribute names and event names that
| are defined to be in any ATTRIBUTE GROUP or EVENT GROUP.  The count supports
| issuing an error if the user specified a GROUP and put no names in it.
*/
static int attrgrpcount;
static int eventgrpcount;


/*
| Points to the current Counter Attribute Partition for use in processing
| COUNTED AS clauses
 */
static IDS *counter_attr_partition;


/*
| Numeric code indicating which attribute group we're currently working on
| (IDENTIFIER, STATUS, etc)
*/
static int CurAttrGroup = -1;


/*
|  This points to the Version String parsed from the Builtin-Types file.
|  This is used when we load a binary MIR database file and need to compare
|  the version strings in order to alert the user to the possibility of a
|   version skew
*/
char            *ascii_bidt_version;


/* load_from_bin
|
|  This is set TRUE by the mirc_init_backend() function when it is loading
|  an input Binary MIR Database file, otherwise it is set FALSE.  The action
|  code that parses the Builtin-Types file version string uses this flag
|  to decide whether or not to abort the parse (abort on "TRUE", we don't
|  want built-in datatype definitions to come from builtin-types file, we want
|  the definitions in the binary MIR database file that has been loaded).
*/
BOOL            load_from_bin;


/*
|  Used to save position during parse of PARENT= clause.
*/
static IDS      *saved_current;


/*
|  Carries indicator for current SMI, the one we're compiling under.
*/
static mir_smi_code     smi_code;


/*
| We use this to decide whether we encountered a valid DNS Primary Name during
| the compilation of the current file.
*/
static int found_dns_primary_name = MCC_K_FALSE;


/*
| This cell points to the Last directive object created (at any given time)
| so the directive's argument can be directly linked to it by code at rule
| 'req_arg:'.
*/
static IDS *directive_IDS;


/*
| Level number of current entity class:
|       For use in error checking DNS Primary Ident clause
*/
static int entitylevel = -1;


/*
| Used as a substitute for "level" during evaluation of "PARENT=" clause 
*/
static int plevel = -1;


/* level number of current MIR object:
|  -1 = not set yet
|   0 = pending assignment of value for
|       CHILD ENTITY
|   1 = GLOBAL ENTITY
|   2..n = CHILD ENTITY and contained object(s)
*/
static int level = -1;


/* current Attribute Group IDS
|
|  This storage cell allows the rule that actually creates the MIR Object
|  representing an Attribute ('attribute_definition:') to 'reach over'
|  and insert a MIR_List_Entry relationship in the Attribute Partition object
|  that also points to the Attribute.  Rule 'partition:' loads this cell.
|
|  This cell is globally accessible so that it can be a de facto argument
|  to function "mirc_create_object()" so that the function can check for
|  duplicate Attribute names.
*/
IDS  *current_attr_partition;


/* current_evt_partition IDS
|
|  This storage cell allows the rule that actually creates the MIR Object
|   representing an Event ('event_decl:') to 'reach over' and insert
|  a MIR_List_Entry relationship in the Event Partition object that also
|  points to the Event.  Rule 'ev_part_header:' loads this cell.
|
|  This cell is globally accessible so that it can be a de facto argument
|  to function "mirc_create_object()" so that the function can check for
|  duplicate Event names.
*/
IDS *current_evt_partition;


/*
|  This globally accessible cell is set by action code at rules:
|
|       * req_def - REQUEST may contain an argument
|       * rsp_beg - RESPONSE may contain an argument
|       * exe_hdr - EXCEPTION may contain an argument
|       * event_decl - EVENT may contain an argument
|
|  This cell is a de facto argument to the back-end function that
|  computes the OID for an object being registered in the index.  In the case
|  of a MIR object that represents an ARGUMENT, not enough information is
|  present in the argument list to the function that produces OIDS in order
|  for it to determine what number should be used in a certain arc of the OID
|  for the ARGUMENT object.  This cell provides the needed information for
|  proper OID construction in these special cases.
*/
mir_relationship containing_arg_type;


/*
|  A pointer to the IDS for the "last previously-defined datatype" that the
|  compiler looked up is left here.  This permits error checking of things
|  being defined as a subrange to be sure that the underlying datatype (stored
|  here) is OK for use as a subrange.
*/
IDS   *existing_datatype;   /* --> intermediate representation */
                            /* of base or previously defined   */
                            /* datatype                        */

/*
|  "VARIANT-RECORD" support
*/
static char case_fixed_field_name[MAX_MULTI_KEYWD_SIZE];
int variant_idcode = -1;        /* -1 = "No Idcode Present", and buffer      */
                                /*       "case_fixed_field_name[]" is empty, */
                                /*       and we're not processing a          */
                                /*       CASE-defined group of fields        */
/*
|  During the parse of the fields that make up a variant group of fields
|  following a "[case-codes]" list (in a TYPE statement for a VARIANT RECORD),
|  the following cell points to the IDS that represents the variant-record
|  being built.  If NULL, then we're not currently parsing variant fields in
|  a variant-record.
*/
IDS *current_variant_record = NULL;


/*
|  "DEPENDS ON" clause support.
*/
FRB     *d_o_fwd_ref=NULL;   /* (if non-NULL)-> Current Forward Ref Block    */
FRB    **d_o_enum_next=NULL; /*-> Bottom, "next" cell in last on enum list   */
static IDS                   /*   in the forward reference block             */
       *current_eclass=NULL; /* The IDS for current entity class             */

/*
|  "IDENTIFIER = " clause support.
*/
FRB     *ident_name_list;      /*-> List of Attr names in IDENTIFIER clause  */
FRB    **ident_list_next=NULL; /*-> "Next" cell of last block on list above  */
BOOL   identifier_clause_lineno; /* "0" =  "IDENTIFIER=()" clause "seen"     */
                                 /*  otherwise the line # of ENTITY that     */
                                 /*  is missing the "IDENTIFIER=()" clause   */

/*
|  "DIRECTIVE_TYPE = " clause support.
*/
BOOL   dir_type_clause_lineno;   /* "0" = "DIRECTIVE_TYPE=()" clause "seen"  */
                                 /*  otherwise the line # of DIRECTIVE that  */
                                 /*  is missing the "DIR.._TYPE=()" clause   */

/*
|  Supports reporting a warning that the "PRIVATE=" keyword was encountered.
|
|  We allow compilation to continue, but nothing in the PRIVATE clause is
|  actually recorded in the MIR.
*/
BOOL    private_encountered = MCC_K_FALSE;


/* Compiler Mode
|
|  The value of this cell along with the 'definition-status' of the current
|  object that the compiler is working on determines how the compiler behaves
|  when processing the current object.  (The 'definition-status' is either
|  "DS_PREDEFINED" or "DS_CREATED").
|
|
|  CM_NORMAL - The compiler is operating 'normally' which means that the
|  MSL file currently being compiled is expected to describe only objects that
|  do not already exist in the MIR (ie, they've not been mentioned in any
|  earlier files that the compiler has seen including any input binary MIR
|  database file).  In this normal mode, the compiler sets the
|  'definition-status' of each new object it creates to "DS_CREATED".  This
|  mode is the default (i.e. there was no merge "-m" switch on the command-line
|  and the MSL file doesn't contain the "AUGMENT ENTITY" keyword phrase).
|
|
|  CM_MERGE - The compiler is operating in 'merge-mode'.  It expects that
|  the current file *may* (or may not) contain a description of something that
|  has already been 'seen' by the compiler during this compilation.  This is
|  most likely to be an Entity-Class and it's contents.  In this mode, the
|  compiler, upon detecting a previously defined object, assumes that the
|  prior definition is the one that is to obtain, and it ignores (without
|  complaining) all information about that object in the current file, except
|  for entity-classes.  For entity-classes, the definitions associated with
|  *just the entity-class* given by the syntax:
|
|       <GLOBAL | CHILD> ENTITY <class-name> = <code> :
|           [Optional OIDs],
|           [PARENT = ( <class-name> {, <class-name> }), ]
|           IDENTIFIER = ( <identifying-atttributes-list> ),
|           [DYNAMIC - <"TRUE"> | <"FALSE">, |
|           [DEPENDS ON = "<variant-expression>",]
|           [SYMBOL = <Latin1String>, ]
|
|  are ignored, while objects contained within the Entity-Class continue to
|  be scrutinized to see if they are already defined ("DS_PREDEFINED").
|
|  This mode allows an MSL file to contain a mix of 'previously-defined'
|  object's and 'newly-defined' objects.  The 'newly-defined' objects are
|  "merged" during the compilation (their definition status is set to
|  "DS_CREATED"), while the 'previously-defined' objects remain untouched
|   (with definition status of "DS_PREDEFINED").
|
|  This mode has a number of ramifications:
|
|  * While you may define a new IDENTIFIER ATTRIBUTE in the IDENTIFIER
|    PARTITION for an already existing Entity-class, any new and different
|    "IDENTIFIER=" clause that may be placed into the merged MSL file will
|    be ignored because the Entity-class already exists.  Consequently the
|    new attribute will be successfully merged, but the IDENTIFIER= clause
|    cannot be changed.  To affect a change in the IDENTIFIER= clause the
|    entire Entity-class must be removed (using the -r switch) before a
|    merge of the MSL file for the Entity-class begins.
|
|  * While you may add a new attribute to a partition, you may not add that
|    new attribute to an *existing* ATTRIBUTE GROUP (because the group
|    already exists and any new definitions about it are ignored).
|
|  * Remember that for any object discovered to be 'predefined', any
|    definition for that object in the MSL file is ignored (e.g. you can't
|    use 'merge' mode to change the datatype of an existing attribute).
|
|
|  CM_AUGMENT - The compiler has been operating in "CM_NORMAL" mode and while
|  parsing the current MSL file has discovered the keyword phrase "AUGMENT
|  ENTITY" signalling that the file is an AUGMENT MSL file.  The compiler
|  switches to CM_AUGMENT mode.  The objects mentioned in the AUGMENT ENTITY
|  MSL file are 'merged' into the specified Entity-Class(es) in the manner of
|  the CM_MERGE mode described above but with the following restrictions
|  applying:
|
|  * The parser will be re-invoked on the source file once for every
|    entity-class (path) mentioned in the AUGMENT ENTITY keyword phrase.
|    Any TYPE statements in the MSL file will be processed (ie built-up
|    datatypes created ONCE) on the first parser invocation and ignored
|    thereafter (to preclude multiple definitions being created).
|
|  * Only non-entity-class objects may be mentioned in the body of the
|    AUGMENT MSL file (attributes, attribute partitions & groups, events,
|    event groups/partitions and directives (requests/responses/exceptions)).
|
|  * In general, any such objects (mentioned above) that appear in the
|    AUGMENT file must not already be defined in the specified entity-class,
|    or an error results.  There are a few exceptions.  For instance, if
|    you wish to add an EXCEPTION to an existing DIRECTIVE, you must specify
|    the DIRECTIVE syntax and then include the new EXCEPTION within that
|    DIRECTIVE syntax.  The DIRECTIVE syntax may not also specify (say)
|    "DISPLAY=TRUE" (on the directive), because this would constitute an
|    attempt to modify an existing object (instead of creating a new instance
|    as AUGMENT is meant to do).
|
|  This global cell is set from "mir_frontend.c" according to the presence or
|  absence of command-line switches as well as by code within this module.
*/
CM_TYPE         compiler_mode=CM_NORMAL;


/* AUGMENT Support
|
|  "augment_max_pass" is set by action code near rule "augment_body" to the
|  number of entity-class lists specified in the AUGMENT ENTITY clause.  This
|  number corresponds to the number of times the parser must pass over the
|  input AUGMENT MSL file (ie once for every entity-class list).
|
|  "augment_pass" is set to the number of the pass we're on.
|
|  "augment_max_pass" is set to the total number of passes that must be
|  performed (ie the number of entity-class lists that appear between
|  braces following the keywords AUGMENT ENTITY . . .
|
|  "local_pass_count" is used while parsing the entity-class lists to keep
|  track of 'where we are' in the parsing of the list. . . when the value
|  of this variable matches the value of "augment_pass", that means the
|  parse (and processing) of that entity-class is "active".
*/
int             augment_pass;         /* Number of the current pass          */
int             augment_max_pass;     /* Maximum number of passes to make    */
static int      local_pass_count;     /* Pass "scanner", used by action code */


/* "Apply-Defaults" Hack  --  "<ADHACK>"
|
| The Situation:
| --------------
| The MCC MSL translator (the compiler used with "DAP") has special
| logic added to it so that under certain circumstances, it adds default
| values for certain properties *IF* the MS file being compiled does not
| explicitly set a value for that property for a given object.
|
| The Official MIR Compiler Design Rule is that the MIRC *exactly translates*
| all MSL files into the MIR while adding no information and deleting no
| information.  The informal design motto is "If it is in the MSL file, then
| it's in the MIR (and you can get it out): Nothing Extra, Nothing Missing".
|
| As you can see given this design rule, the MIR binary file will not contain
| the defaults that the MSL translator adds.
|
| In order to ease the transition to the MIR compiler standard ("Nothing Extra,
| Nothing Missing"), MIRC implements this special "Apply-Defaults" Hack via
| the undocumented command-line argument "-Xadhack".  Specifying this switch
| on the command line causes MIRC to behave the way MSL behaves with respect
| to adding defaults for the properties so affected.
|
| The definitions and the objects affected are:
|
|   Definition      In. . . .                                   Default
|   ----------      ---------------------------------           -------
|   echo            ARGUMENTs defined within REQUESTs           MCC_K_TRUE
|   required        ARGUMENTs defined within REQUESTs           MCC_K_FALSE
|   predictable     ATTRIBUTES                                  MCC_K_TRUE
|   dynamic         ENTITY CLASSES                              MCC_K_FALSE
|   instance_req    ENTITY CLASSES                              See Note 1
|   display         ATTRIBUTES                                  MCC_K_TRUE
|                   DIRECTIVES                                  MCC_K_TRUE
|                   EVENTS                                      MCC_K_TRUE
|                   ARGUMENTS defined within REQUESTS           MCC_K_TRUE
|                   ARGUMENTS defined within EXCEPTIONS         MCC_K_TRUE
|                   ARGUMENTS defined within RESPONSES          MCC_K_TRUE
|                   ARGUMENTS defined within EVENTS             MCC_K_TRUE
|   access          ATTRIBUTES                                  See Note 2
|                   attribute PARTITION                         See Note 3
|                   attribute GROUP                             See Note 4
|   default_value   ATTRIBUTES                                  See Note 5
|                   ARGUMENTS defined within REQUESTS           
|   dns_ident       IDENTIFIER ATTRIBUTES                       See Note 6
|   categories      ATTRIBUTE GROUP                             See Note 7
|                   ATTRIBUTE
|                   DIRECTIVE
|                   EVENT GROUP
|                   EVENT
|
| Note 1:  The default is MCC_K_FALSE unless the "IDENTIFIER=" clause specified
|          an attribute.  This definition has no corresponding MIR Relationship
|          to record it.  However, the value of what would be written can be
|          inferred from the presence or absence of the "MIR_Indexed_By"
|          relationship plus the target if it is present:
|               MIR_Indexed_By,
|                   Not Present:  Implies this is not an an entity class
|                                 object.
|               MIR_Indexed_By,
|               Present, Terminal (Number):  Implies this is an entity class
|                                 object and that the "IDENTIFIER=" clause was
|                                 empty (ie "instance_required = FALSE")
|               MIR_Indexed_By,
|               Present, Non-Terminal:  Implies this is an entity class and
|                                 that the "IDENTIFIER=" clause was not
|                                 empty (ie "instance_required = TRUE")
|
|          ** This MIRC behavior described applies REGARDLESS of the -Xadhack
|          ** switch, but provides (via proper interpretation) the same
|          ** information that MSL used to record).
|
| Note 2:  The default set for an attribute missing the "ACCESS=" clause
|          depends on the partition within which it is defined:
|
|          IDENTIFIER:     MCC_K_NONSETABLE
|          STATUS:         MCC_K_NONSETABLE
|          CHARACTERISTIC: MCC_K_SETABLE
|          REFERENCE:      MCC_K_SETABLE
|          COUNTER:        MCC_K_NONSETABLE
|          INITIAL:        MCC_K_NONSETABLE
|          STATISTIC:      MCC_K_NONSETABLE
|
| Note 3:  The attribute partition object does not have an "ACCESS=" clause,
|          however under "<ADHACK>", we do indeed write a "MIR_Access"
|          relationship into the attribute-partition MIR Object.  The target
|          of this relationship is either the Terminal SIGNED number for
|          MCC_K_SETABLE or MCC_K_NONSETABLE.  If any of the attributes in
|          the partition have "ACCESS=" clause values that evaluate to 
|          NOT "MCC_K_NONSETABLE" then the partition MIR_Access target is
|          MCC_K_SETABLE, otherwise the partition MIR_Access target is
|          MCC_K_NONSETABLE.  This action is sort of a 'compile-time lookup'
|          of the 'setableness' of all the attributes named in the
|          partition (rumor has it this was for the MCC iconic map).
|
| Note 4:  The attribute group object does not have an "ACCESS=" clause,
|          however under "<ADHACK>", we do indeed write a "MIR_Access"
|          relationship into the attribute-group MIR Object.  The target
|          of this relationship is either the Terminal SIGNED number for
|          MCC_K_SETABLE or MCC_K_NONSETABLE.  If any of the attributes in
|          the group have "ACCESS=" clause values that evaluate to 
|          NOT "MCC_K_NONSETABLE" then the group MIR_Access target is
|          MCC_K_SETABLE, otherwise the group MIR_Access target is
|          MCC_K_NONSETABLE.  This action is sort of a 'compile-time lookup'
|          of the 'setableness' of all the attributes named in the
|          group.
| 
| Note 5:  In a *NO* "<ADHACK>" situation:
|          ..for both ATTRIBUTES and ARGUMENTS in REQUESTS, the "DEFAULT="
|          clause is mapped to a "MIR_valueDefault" relationship which
|          can have a Terminal SIGNED number of MCC_K_DEF_NO (if the
|          MSL actually specified "NO DEFAULT") or MCC_K_DEF_IMPL_SPEC
|          (if MSL actually specified "IMPLEMENTATION SPECIFIC") or
|          it can have a Terminal STRING that is the 'crunched' value of
|          the string actually specified in the MSL opposite the "DEFAULT="
|          clause.
|
|          In an "<ADHACK>" situation where the MSL did *not* specify a value,
|          then a "MIR_valueDefault" relationship is written with a target
|          value of MCC_K_DEF_NO as a signed number.
|
| Note 6:  Under "<ADHACK>", for each IDENTIFIER ATTTRIBUTE *that belongs to
|          a GLOBAL ENTITY* for which no "DNS_IDENT=" clause was parsed,
|          the compiler adds a MIR_DNS_Ident relationship into the attribute
|          with target value of "MCC_K_DNS_ID_ALTERNATE".
|
| Note 7:  Under "<ADHACK>", for each of the listed objects, *if* there
|          was no category specified in the MSL, then MIRC writes a
|          MIR_Category relationship with target of signed number with
|          value "MCC_K_OSI_ALL".
|
| What MIRC Does
| --------------
| If MIRC command-line switch "-Xadhack" is present, then the claim is made
| that MIRC will produce a binary MIR database file with the same (equivalent)
| defaults and relationships entered in all the corresponding MIR objects
| as the MCC MSL translator would (as described above).
|
| Because this is really a hack, all the code in MIRC (primarily in this file)
| is marked with the string "<ADHACK>" in a nearby comment that describes
| what you have to do to the surrounding code to REMOVE THIS HACK.
|
| Note that support for this hack is more complicated in MIRC than MSL because
| MIRC did not (previously) need to defer the writing of the relationship
| for some of these definitions (thru a call to mirc_add_rel_*()).  Also, MIRC
| supports "Augment" mode in a slightly different way than MSL.  Specifically,
| it checks the MIR (in the Intermediate Heap representation) to see if a
| MIR object is already defined and issues an error message in the event an
| attempt is made to write a definition to an existing object. . .this
| complicates the writing of the 'defaults' for "<ADHACK>", as these writes
| must be inhibited (without an error message) when running in Augment mode
| on an object that already exists.  (See the discussion of AUGMENT under
| "Compiler Mode" above).
| 
|
| -----------------------------------------------------------------------------
| <ADHACK> - The following storage locations support the "-Xadhack" switch,
| and may be deleted when 'support' for this hack is removed.
| -----------------------------------------------------------------------------
|
| Set by code in mirf_cmd_arg() in mir_frontend.c, this triggers special
| MIRC support to Apply-Defaults a la MSL:
*/
BOOL    ad_hack_switch=FALSE; /* TRUE: We're hacking as described above!     */

static int echo_value;        /* Temp for parsed "ECHO=" clause value        */
static int echo_present;      /* TRUE: We parsed an "ECHO=" clause           */

static int required_value;    /* Temp for parsed "REQUIRED=" clause value    */
static int required_present;  /* TRUE: We parsed a "REQUIRED=" clause        */

static int predict_value;     /* Temp for parsed "PREDICTABLE=" clause value */
static int predict_present;   /* TRUE: We parsed a "PREDICTABLE=" clause     */

static int dynamic_value;     /* Temp for parsed "DYNAMIC=" clause value     */
static int dynamic_present;   /* TRUE: We parsed a "DYNAMIC=" clause         */

static int display_value;     /* Temp for parsed "DISPLAY=" clause value     */
static int display_present;   /* TRUE: We parsed a "DISPLAY=" clause         */

static int access_value;      /* Temp for parsed "ACCESS=" clause value      */
static int access_present;    /* TRUE: We parsed a "ACCESS=" clause          */
static int access_default;    /* For setting initial attribute access value  */

static int partition_setable; /* Used for <ADHACK> assignment of MIR_Access  */
                              /* to Attribute Groups and Partitions          */

static int dns_ident_present; /* TRUE: We parsed a "DNS_IDENT=" clause       */
static int dft_value_present; /* TRUE: We parsed a "DEFAULT=" clause         */
static int category_present;  /* TRUE: We parsed a "CATEGORY=" clause        */

/* End of <ADHACK>-support variables */


/* function prototypes */

#ifdef __STDC__
static void GetTokStr(); 
static void GetTokStrQuoted(); 
int yyparse();
#endif

%}
	/* beginning of YACC token definitions */

%start mgmt_spec

	/* punctuation tokens defined here */

%token P_SEMICOLON 
%token P_EQUAL
%token P_COLON
%token P_COMMA
%token P_LPAREN
%token P_RPAREN
%token P_LBRACE
%token P_RBRACE
%token P_LSQRBRKT
%token P_RSQRBRKT
%token P_ASTERISK
%token P_ELLIPSES
%token P_PERIOD

	/* variable tokens defined next */

/*** NOTE: the keyword syntax rules are sooo loose in NETMAN that 
 *** almost any token which is not a constant will be interpreted as a
 *** <keyword> token.  Therefore extra context-sensitive analysis must 
 *** be done inside the parser, which knows what kind of token to expect.
 */

%token BAD_CHARACTER
%token ID_ANYCASE
%token ID_QUOTED

%right KW_LOWPRECIDENCE
%right KW_IDENTIFIER_EQ
%right ID_ANYCASE

	/* keyword tokens defined next */

%token KW_ACCESS
%token KW_ACTION
%token KW_ARGUMENT
%token KW_AS
%token KW_ATTRIBUTE
%token KW_ASN1_CLASS
%token KW_ASN1_TAG
%token KW_ATOMIC
%token KW_ATTRIBUTES
%token KW_ATTRIBUTE_LIST
%token KW_AUGMENT
%token KW_AUX_INFO
%token KW_BIDT_VERSION
%token KW_BITSET
%token KW_CASE
%token KW_CATEGORIES
%token KW_CHARACTERISTIC
%token KW_CHILD
%token KW_NCL_CMIP_CODE
%token KW_COUNTED 
%token KW_COUNTER
%token KW_DEFAULT
%token KW_DEFINING_SMI
%token KW_DEPENDS
%token KW_DIRECTIVE
%token KW_DIRECTIVE_TYPE
%token KW_DISPLAY
%token KW_DNS_IDENT
%token KW_DYNAMIC
%token KW_ECHO
%token KW_END
%token KW_END_SMI_DEF
%token KW_ENTITY
%token KW_EVENT
%token KW_EVENT_LIST
%token KW_EXAMINE
%token KW_EXCEPTION
%token KW_FALSE
%token KW_GENERIC_PRESENTATION
%token KW_GLOBAL
%token KW_GROUP
%token KW_IDENTIFIER
%token KW_IDENTIFIER_EQ
%token KW_IMPLEMENTATION
%token KW_IN
%token KW_INCLUDE
%token KW_INITIAL
%token KW_MANAGEMENT 
%token KW_MCC_DATATYPE_SIZE
%token KW_MODIFY
%token KW_NO
%token KW_NONSETTABLE
%token KW_OID
%token KW_SNMP_OID
%token KW_DNA_CMIP_INT
%token KW_OSI_CMIP_OID
%token KW_OF
%token KW_PARENT
%token KW_PARTITION
%token KW_PREDICTABLE
%token KW_PRIVATE
%token KW_RANGE
%token KW_RECORD
%token KW_REFERENCE
%token KW_REQUEST
%token KW_REQUIRED
%token KW_RESPONSE
%token KW_SEQUENCE
%token KW_SET
%token KW_SETTABLE
%token KW_SMI_BUILTIN_TYPE
%token KW_SPECIFIC
%token KW_SPECIFICATION
%token KW_START_SMI
%token KW_STATISTIC
%token KW_STATUS
%token KW_SYMBOL
%token KW_SYMBOL_PREFIX
%token KW_TEXT
%token KW_TRUE
%token KW_TYPE
%token KW_UNITS
%token KW_VERSION
%token KW_WRITEONLY
%token KW_KEYWORDS

	/* end of YACC token definitions */



%% 	/* beginning of YACC grammar rules */
/*
 * first we define common data types up front 
 */

oid_line: oid_int | oid_string;

oid_int: KW_DNA_CMIP_INT P_EQUAL whole_number
	{
        object_id       *oid;    /* Ptr to Computed OID gets returned here */
        char            *errmsg; /* Error Message if we can compute OID    */
        MC_STATUS       oidstat; /* Status from OID registration attempt   */


        /*
        |  If we parse this, we need to do several things:
        |
        |       * We need to record this code as a ("Standard") IDcode for
        |         future reference in building an OID for the current
        |         object as well as contained objects.
        |
        |       * We need to actually compute and register the current
        |         object using an OID generated using this code (recorded
        |         above).
        |
        |       * We need to store this code as the target of
        |         the relationship "MIR_ID_Code" in the current object.
        */

        /* Record the Code as a Standard code at this level */
        mirc_record_idcodes(level, STD_code, $3, NULL, NULL);

        /*
        | Only register the OID for a bonafide 'new' object: compiler mode
        | is "normal", or (for merge & augment) the object has status "Created"
        */
        if (compiler_mode == CM_NORMAL || mirc_get_def_status() == DS_CREATED){

            /* Compute the OID and Register it with the current object */
            if ((errmsg = mirc_compute_oid(level, STD_code, &oid)) == NULL) {

                /* Provide end to added msg generated in mirc_compute_oid */
                if (supplementary_info == TRUE) {
                    fprintf (stderr, " Code = %d \n", $3);
                    }

                /*
                |  We successfully computed the oid.  Now Register the current
                |  object using it.
                */
                /* If OID is already in use on Some Other Object . . . */
                if ((oidstat = I_Register_OID( oid, OID_DNA, mirc_get_current()))
                                              != MC_SUCCESS) {
                    switch (oidstat) {
                        case MC_REG_Already_OID:
                            yyerror(
                                    MP(mp800,"Specified OID already in use, oid indexing not done"));
                            break;

                        case MC_REG_Already_OBJ:
                            yyerror(
                                    MP(mp801,"Specified OID already registered to this object"));
                            break;

                        case MC_REG_SMI_Already:
                            yyerror(
                                    MP(mp802,"Object already has an OID of this type assigned"));
                            break;
                        default:
                            TRY(oidstat);
                        }
                    }
                }
            else {  /*  Couldn't compute the OID */
                char    buff[200];

                eflag = TRUE;   /* make sure they get no binary */

                /* Only issue a message if it has a non-zero length */
                if (strlen(errmsg) > 0) {
                    sprintf(buff,
                            MP(mp803,"Unable to generate DNA OID:\n       %s"),
                            errmsg);
                    yyerror(buff);
                    }
                }
            }

        /*
        |  If we didn't register, we want to complain if they were trying to
        |  register an OID for a MIR object under AUGMENT that was a PREDEFINED
        |  object.
        */
        else {
            if (compiler_mode == CM_AUGMENT
                && mirc_get_def_status() == DS_PREDEFINED) {
                yyerror(
                        MP(mp843,
                           "Object is already defined: automatic assignment of DNA OID is not allowed")
                       );
                }
            }

        /* Store the code as target of MIR_ID_Code */
        TRY( mirc_add_rel_number( MIR_ID_Code, $3, MIR_SNUMBER ));
        }
	;

oid_string:
	oid_type P_EQUAL P_LBRACE
		{
                /* Setup Object ID           */
                oid.value = (unsigned int *) oid_arcs;

                /* Reset to receive next OID */
                oid.count = 0;
		}
	arc_list P_RBRACE
        {
        MC_STATUS  oidstat; /* Status from OID registration attempt */

        /*
        | Only register the OID for a bonafide 'new' object: compiler mode
        | is "normal", or (for merge & augment) the object has status "Created"
        */
        if (compiler_mode == CM_NORMAL
            || mirc_get_def_status() == DS_CREATED) {

            /* This check avoids problems if the OID wasn't valid */
            if (oid.count > 0) {
                /* If OID is already in use on Some Other Object . . . */
                if ((oidstat = I_Register_OID(&oid,
                                              oid_type,
                                              mirc_get_current()))
                    != MC_SUCCESS) {
                    switch (oidstat) {
                        case MC_REG_Already_OID:
                            yyerror(
                                    MP(mp800,"Specified OID already in use, oid indexing not done"));
                            break;

                        case MC_REG_Already_OBJ:
                            yyerror(
                                    MP(mp801,"Specified OID already registered to this object"));
                            break;

                        case MC_REG_SMI_Already:
                            yyerror(
                                    MP(mp802,"Object already has an OID of this type assigned"));
                            break;
                        default:
                            TRY(oidstat);
                        }
                    }
                }
            }
        /*
        |  If we didn't register, we want to complain if they were trying to
        |  register an OID for a MIR object under AUGMENT that was a PREDEFINED
        |  object.
        */
        else {
            if (compiler_mode == CM_AUGMENT
                && mirc_get_def_status() == DS_PREDEFINED) {
                yyerror(
                        MP(mp844,
                           "OID assignment to an object not created by AUGMENT file is not allowed")
                       );
                }
            }
        }
	;

oid_type:	KW_OID
			{oid_type = OID_OID;}
	|	KW_SNMP_OID
			{oid_type = OID_SNMP;}
	|	KW_OSI_CMIP_OID
			{oid_type = OID_OSI;}
	;

arc_list:	arc
	|	arc arc_list
	;

        /*
        | In other words:
        |   
        |   it could legally be     a)     "2"
        |   or it could legally be  b)     "tcp(2)"
        |
        |   We let rule "oid_name_or_number" try to handle case a), then
        |   rule "oid_name_value" takes over and does the right thing
        |   depending on whether "oid_name_or_number" managed to do an
        |   OK-convert (case a) or if it failed (case b).
        */
arc:	oid_name_or_number oid_name_value
	;

oid_name_or_number: ID_ANYCASE

	{
        char *ending_parse_point;

        /* Endeaveor to convert the entire name-or-number to a number */
        oid_number = strtoul(yytext, &ending_parse_point, 10);

        /* If the parse didn't parse everything. . . */
        if (*ending_parse_point != '\0') {
            /*
            | Signal "oid_name_value" code that "oid_number" is invalid.
            | It's case b) if valid at all.
            */
            oid_number = -1;
            }
        /* ELSE it's case a), and we've done an OK conversion */
	}
	;

 
oid_name_value:	/* empty */
          /*
          |  If we reach here, this implies that there is no "(" present,
          |  which means we hope "oid_name_or_number" parsed a number.
          */    
                {
                /*
                |  If there is no "(<number>)" construct, then the value
                |  of "oid_number" had better be valid, and that's what we
                |  use.
                */
                if (oid_number == -1) {
                    yyerror(MP(mp804,"invalid object identifier syntax"));
		    YYERROR;
                    }
                else {
                    oid_arcs[oid.count++] = oid_number;
                    }
                }

	|	P_LPAREN whole_number P_RPAREN

           /*
           |  If we reach here, this implies that "oid_name_or_number"
           |  tried and failed to parse something legal like "tcp" in
           |  "tcp(2)".  If it DID manage to parse a number, it had to be
           |  something illegal like the "9" in "9 (2)".  Hence, we insist
           |  any number parsed is as a consequence of a misplaced "(".
           */
		{
                /*
                |  HERE "oid_number" MUST be 'invalid' (as a number) since
                |  the value between the parens is the oid arc value we use
                */
		if (oid_number != -1)
		    {
		    /* yyerror("msl_error(MCCMSL_S_INVPAREN)"); */
                    yyerror(MP(mp805,"Invalid Parentheses"));
		    YYERROR;
		    }
                oid_arcs[oid.count++] = $2;
		}
	;

separator:	comma_or_include
	|	semicolon_or_include
	;

optional_separator: /* empty */
	| separator;

comma_or_include: P_COMMA op_incl_stmt;

semicolon_or_include: P_SEMICOLON op_incl_stmt;

op_incl_stmt: /* empty */
	| keyword_statement KW_INCLUDE file_name P_SEMICOLON
	    {
	    if (do_include_statement == MCC_K_TRUE)
		{
                TRY(open_include_file( CurStrBuf ));
		}
	    do_include_statement = MCC_K_TRUE;
	    }
	op_incl_stmt ;

keyword_statement: /* empty */
	| KW_KEYWORDS
	{
	do_include_statement = MCC_K_FALSE;
	}
	keyword_list P_COLON;

keyword_list : keyword_value
	| keyword_list P_COMMA keyword_value;

keyword_value: ID_ANYCASE
	{

	GetTokStr( $1 );

        if (mirc_keyword(IS_IT_SELECTED, CurStrBuf) == TRUE) {
            do_include_statement = MCC_K_TRUE;
            }
        else if (mirc_keyword(IS_IT_A_KEYWORD, CurStrBuf) == FALSE) {
            yyerror(MP(mp806,"Invalid KEYWORD for INCLUDE statement"));
	    YYERROR;
	    }

	};

true_false: 	KW_TRUE 
		  { $$ = MCC_K_TRUE; }
	| 	KW_FALSE 
		  { $$ = MCC_K_FALSE; }
	;


 /* this production is used to accept integer values which are not codes */
integer_value:	ID_ANYCASE
		  {
		  cp = yytext;
		  if (*cp != '-' && !isdigit(*cp)) 
			{
			/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
			/* msl_error_1( MCCMSL_S_INVNUMBER, &yy_desc ) */
                        yyerror(MP(mp807,"Invalid Number"));
			YYERROR;
			};
		  cp++;
		  while (*cp) 
			{
			if (!isdigit(*cp)) 
				{
				/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
		                /* msl_error_1( MCCMSL_S_INVNUMBER, &yy_desc)*/
                                yyerror(MP(mp807,"Invalid Number"));
				YYERROR;
				}
			else 
				cp++;
			};
		  $$ = atoi( yytext );
		  };

 /* this production is used to accept integer values such as ID codes */
whole_number:	ID_ANYCASE
		  {
		  for (cp = yytext; *cp; cp++) 
			if (!isdigit(*cp)) 
				{
				/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
		                /* msl_error_1( MCCMSL_S_INVNUMBER, &yy_desc)*/
                                yyerror(MP(mp807,"Invalid Number"));
				YYERROR;
				};
		  $$ = atoi( yytext );
		  };

/* must concatenate multiple keywords into single string */
multi_keywd_identifier:
		multi_keywd_identifier single_keyword
		  { 
		  CurStrBuf[CurStrBufLen] = ' ';
		  memcpy( &CurStrBuf[CurStrBufLen+1], yytext, $2 );
		  CurStrBufLen = $$ = $1 + $2 + 1;	/* add lengths together */
		  CurStrBuf[$$] = 0;
		  }
	|	single_keyword
		  {
		  GetTokStr( $1 );
		  }
	;

 /* we need to be able to handle any combination of letters or numbers here.
  * unfortunately this means that whole numbers are a subset of the set
  * of possible keywords.  If a whole number is used as a keyword, 
  * the VAL_NUMERIC token is returned by the lexical analyzer, since
  * it is set up to match the most specific token type first.
  */
single_keyword:
		ID_ANYCASE
		  {
		  $$ = $1;
		  };

/* optional_name is used a lot for END declarations, because we don't
 * do any checking at this time to ensure that the name in the END
 * declaration matches the name that began the declaration block.
 * The only END declaration that does any checking is the END ENTITY
 */
optional_name: 	/* empty */
	|	multi_keywd_identifier ;

/* 
|  This production accepts a data type name string.  It can be either a 
|  built-in (NETMAN-defined) type or a constructed type name.  Since
|  constructed type names must be different from the NETMAN-defined names,
|  we try to match it against a built-in type name.  If this fails, then
|  we try to match it against a constructed type name.  All name comparisons
|  are case-insensitive for historical reasons.
| 
|  We indicate an error if no such type was defined, as this production is used
|  in the grammar in contexts where the datatype must already be known to the
|  compiler (in the rules used to define new datatypes (ie "TYPE") and in the
|  rules used to define (say) an attribute that has a value of type <datatype>)
*/
defined_data_type: ID_ANYCASE
              {
              MC_STATUS         status;   /* Code returned from d.t. lookup  */
              char              msg[200]; /* Message buffer                  */

              existing_datatype = NULL;   /* Haven't found it yet            */
              GetTokStr( $1 );            /* get token from LEX as string    */

              /* Try to find this string on MIR compiler internal list as a  */
              /*    "builtin" (defined by SMI)                               */
              /* or                                                          */
              /*    "builtup" (defined in MSL with previous declaration)     */
              /* datatype.                                                   */
              status = mirc_find_datatype(CurStrBuf, &existing_datatype);
              if (status == MC_DATATYPE_NOT_FND) {
                  sprintf(msg,
                          MP(mp808,"Datatype '%s' is undefined."), CurStrBuf);
                  yyerror(msg);
                  }
              else {
                  TRY(status);
                  }

              if (existing_datatype != NULL) {
                  TRY (mirc_add_relationship(MIR_Structured_As,
                                             existing_datatype));
                  }
              /*
              |  NOTE: "existing_datatype" is globally defined for this yacc
              |        file so that the action code for "subrange" can examine
              |        the value of this pointer (set here) to decide if the
              |        last-looked-up datatype is a valid datatype for a
              |        subrange definition.
              */
              }
              ;

IDcode: whole_number
		  { $$ = $1; }
	;

units:	KW_UNITS P_EQUAL units_string
        {
        TRY( mirc_add_rel_string( MIR_Units, CurStrBuf ));
        }
        ; 

units_string:	ID_ANYCASE
	{ GetTokStr( $1 ); }
	| ID_QUOTED
	{ GetTokStrQuoted( $1 ); }
	;

include_symbol: KW_SYMBOL P_EQUAL ID_ANYCASE
	{ 
	GetTokStr( $3 );

        /* Put the string into the MIR */
        TRY( mirc_add_rel_string( MIR_Symbol, CurStrBuf ));
	};

echo:   KW_ECHO P_EQUAL true_false
	{
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /* TRUE  */
            TRY( mirc_add_rel_number( MIR_Echo, $3, MIR_SNUMBER ));
            }                           /* FALSE */
        else {  /* Do Apply-Default Hack stuff */
            echo_value = $3;    /* Record parsed value for future reference */
            echo_present = MCC_K_TRUE;  /* Signal "ECHO=" was parsed        */
            }
	}
	;

depends_on: KW_DEPENDS ID_ANYCASE P_EQUAL ID_QUOTED
	{
	char *depend_expr;      /* Ptr to parsed depends-on expression */

        /* if (entity is a Global Entity) */
	if (level == 1)
		{
		/* msl_error(MCCMSL_S_INVDEPENDSON) */
                yyerror(MP(mp809,"DEPENDS-ON Clause not legal for Global Entity"));
		YYERROR;
		}

        /*
        |  If we're Augmenting and the object is PREDEFINED, that is a
        |  bozo-nono, since we can't allow them to modify something not
        |  defined in the Augment MSL file.
        */
        if (compiler_mode == CM_AUGMENT
            && mirc_get_def_status() == DS_PREDEFINED) {
            yyerror(
                    MP(mp151,"Modification of Object not created by AUGMENT file is not allowed")
                   );
            }

        /*
        |  We only do the processing required to record the DEPENDS-ON clause
        |  if we're compiling Normal, or (for Merge & Augment) if the current
        |  object has definition status "CREATED"
        */
        if (compiler_mode == CM_NORMAL
            || mirc_get_def_status() == DS_CREATED) {

            /*
            |  Create an image in memory of the 'depends on' expression we just
            |  parsed as a quoted string (via token "ID_QUOTED") in the rule
            |  above.
            */
            GetTokStrQuoted( $4 );
            depend_expr = (char *)malloc(CurStrBufLen+10+1);
            sprintf(depend_expr, "=%7d %s,", 0, CurStrBuf);

            /*
            |  We create a Forward-Reference block (type: "FR_DEPENDS" -DEPENDS ON)
            |  to receive the essence of what we're about to parse out of
            |  the "depends on" expression.  This FR block is referenced by
            |  action code for the rules "depends_expression" and it's
            |  children.
            |
            |  NOTE:  This compiler handles all DEPENDS ON clauses using
            |         "forward-reference" processing.  This removes the
            |         restriction that existed in the DAP MSL translator that
            |         required characteristic attributes referenced by a
            |         DEPENDS ON clause to have already been compiled.
            */
            TRY(mirc_create_fwd_ref(FR_DEPENDS, &d_o_fwd_ref, NULL));

            /*
            | Init the list management cells associated w/tracking enum names
            */
            d_o_enum_next = &(d_o_fwd_ref->fru.d.enum_list);

            /*
            |  Now point the input function for lex/yacc at the in-memory image
            |  of the depends-on expression, the next rule that should be
            |  matched is "depends_expression:" immediately below.
            */
            parse_depends_expr(depend_expr);
            }
        }
	;

                    /* This portion of this rule simply rips off the "= 0" */
                    /* put on by the action code immediately above.        */
depends_expression:  P_EQUAL integer_value
	{
        /* IDS for the INSTANCE of a "DEPENDS-ON OBJECT" */
        IDS   *d_o_IDS;

        /*
        |  Now create a Non-Terminal to represent the instance of a
        |  DEPENDS-ON OBJECT, and show it as the target of "MIR_Depends_on".
        |  
        |if (attempt to create a non-terminal object failed) */
        if ( (d_o_IDS = I_Create_IDS(I_NT_OBJECT,IDS_DFT_TABLESIZE)) == NULL) {
            yyerror(MP(mp810,"Out of Memory for DEPENDS-ON object"));
            YYERROR;
            }

        /* Attempt to add MIR_Structured_As relationship */
        TRY(mirc_add_relationship(MIR_Depends_on, d_o_IDS));

        /*
        |  Stash information into the Forward Reference block created in
        |  the rule above: D_O Object IDS & EClass IDS.
        */
        d_o_fwd_ref->fru.d.depends = d_o_IDS;

        /* if (current-object IS an entity-class) */
        if (mirc_get_current() == current_eclass) {

            /* Then the Entity-Class to store in the FR Block is the parent */
            d_o_fwd_ref->fru.d.eclass = current_eclass->idsu.nt.parent_obj;
            }
        else {
            d_o_fwd_ref->fru.d.eclass = current_eclass;
            }
	}

        /* (The "Real" DEPENDS ON expression 'value' (what is inside " ")
   <char-attr-name>  [ "=" or "IN SET"]      <enum-name(s)>              */
    dependent_attr      depends_op_type    depends_attr_values	;

        /*
        | This rule recognizes the Characteristic Attribute's name that is
        | being "depended on" by the entity containing the DEPENDS ON clause
        */
dependent_attr: multi_keywd_identifier
	{
        /* Copy the Characteristic Attribute's name into FR Block */
        if ((d_o_fwd_ref->fru.d.char_attr =
                        (char *) malloc(strlen(CurStrBuf)+1)) == NULL) {
            TRY(MC_OUT_OF_MEMORY);
            YYERROR;
            }
        strcpy(d_o_fwd_ref->fru.d.char_attr, CurStrBuf);
	}
	;


        /* Load the code for the operator into the Forward-Reference Block */
depends_op_type: depends_equals
	| depends_in_set
	;

depends_equals: P_EQUAL
	{
	d_o_fwd_ref->fru.d.operator = MCC_K_VAR_OP_EQUAL;
	}
	;

depends_in_set: KW_IN KW_SET
	{
	d_o_fwd_ref->fru.d.operator = MCC_K_VAR_OP_IN_SET;
	}
	;


depends_attr_values:
	  depends_enum
	| P_LPAREN dep_enum_list P_RPAREN
	;

dep_enum_list: depends_enum
	| dep_enum_list P_COMMA	depends_enum
	;

depends_enum: multi_keywd_identifier
	{
        /*
        |  Create a F-R "STRING" block to receive the enumerated name,
        |  and stuff the block onto the list of enumerated names
        */
        TRY(mirc_create_fwd_ref(FR_STRING, d_o_enum_next, CurStrBuf));

        /*
        | Re-record where we should record the address of any subsequent block
        */
        d_o_enum_next = &((*d_o_enum_next)->next);
	}
	;


display: KW_DISPLAY P_EQUAL true_false 
        {
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /*     - > TRUE  */
            TRY( mirc_add_rel_number( MIR_Display, $3, MIR_SNUMBER ));
            }
        else {  /* Do Apply-Default Hack stuff - > FALSE */
            display_value = $3; /* Record parsed value for future reference */
            display_present = MCC_K_TRUE;   /* Signal "DISPLAY=" was parsed */
            }
        }
        ;

categories: KW_CATEGORIES P_EQUAL P_LPAREN category_kw_series P_RPAREN 
	; 

/* series of one or more category keywords separated by commas */
category_kw_series: 
		category_kw_series P_COMMA category_kw 
	|	category_kw ;

category_kw:	ID_ANYCASE
        {
        GetTokStr( $1 );

        category_present = MCC_K_TRUE;          /*<ADACK> Remove this */

	if (strcmp(CurStrBuf, "ALL") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_ALL,
                                    MIR_SNUMBER));
            }
        else if (strcmp(CurStrBuf, "CONFIGURATION") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_CONFIG,
                                    MIR_SNUMBER));
            }
        else if (strcmp(CurStrBuf, "FAULT") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_FAULT,
                                    MIR_SNUMBER));
            }
        else if (strcmp(CurStrBuf, "PERFORMANCE") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_PERFORMANCE,
                                    MIR_SNUMBER));
            }
        else if (strcmp(CurStrBuf, "SECURITY") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_SECURITY,
                                    MIR_SNUMBER));
            }
        else if (strcmp(CurStrBuf, "ACCOUNTING") == 0) {
            TRY(mirc_add_rel_number(MIR_Category,
                                    MCC_K_OSI_ACCOUNTING,
                                    MIR_SNUMBER));
            }
        else {
            /* msl_error_1(MCCMSL_S_INVCATEGORY, &curstr_desc ) */
            yyerror(MP(mp811,"Invalid CATEGORY value"));
            YYERROR;
            }
        }
        ;

private: KW_PRIVATE priv_property_list KW_END KW_PRIVATE
       {
       if (private_encountered == MCC_K_FALSE) {
           /* Signal "OK, no more msgs" */
           private_encountered = MCC_K_TRUE;
           warn(MP(mp812,"Unsupported PRIVATE keyword encountered."));
           }
       }
       ;


/* list of one or more private MSL definitions separated by commas */
priv_property_list:
		priv_property
	|	priv_property_list P_COMMA priv_property ;

priv_property:	private_property_type P_EQUAL private_val

private_val: ID_ANYCASE
	| true_false
	| P_LPAREN priv_struct_list P_RPAREN
	;

private_property_type: ID_ANYCASE ;

priv_struct_list: priv_value
	| priv_struct_list P_COMMA priv_value;

priv_value: integer_value ;


required: KW_REQUIRED P_EQUAL true_false 
	{
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /*     - > TRUE  */
            TRY( mirc_add_rel_number( MIR_Required, $3, MIR_SNUMBER ));
            }
        else {  /* Do Apply-Default Hack stuff - > FALSE */
            required_value = $3; /* Record parsed value for future reference */
            required_present = MCC_K_TRUE;  /* Signal "REQUIRED=" was parsed */
            }
	}
	;

default_value: 
                /* 
                |  The symbol MCC_K_DEF_VALUE is superfluous, we simply
                |  store the value given as a crunched-string, and let it go
                |  at that.  Notice that with this scheme, we can still
                |  reproduce the MSL verbatim from the MIR binary
                |  representation: if "DEFAULT=" is entirely missing from
                |  the MSL at this point, there will be no "MIR_valueDefault"
                |  relationship!  The routine that accesses this rel. can
                |  supply "MCC_K_DEF_NO" as the value returned as needed.
                */
		KW_DEFAULT P_EQUAL any_value
		        {

                        TRY(mirc_add_rel_string(MIR_valueDefault,
                                        filter_string(CurStrBuf)));
                        dft_value_present = MCC_K_TRUE;/*<ADACK> Remove this */
                        }
	|	KW_DEFAULT P_EQUAL KW_NO KW_DEFAULT
                        {
                        TRY(mirc_add_rel_number(MIR_valueDefault,
                                                MCC_K_DEF_NO,
                                                MIR_SNUMBER));
                        dft_value_present = MCC_K_TRUE;/*<ADACK> Remove this */
		        }
	|	KW_DEFAULT P_EQUAL KW_IMPLEMENTATION KW_SPECIFIC
		        {
                        TRY(mirc_add_rel_number(MIR_valueDefault,
                                                MCC_K_DEF_IMPL_SPEC,
                                                MIR_SNUMBER));
                        dft_value_present = MCC_K_TRUE;/*<ADACK> Remove this */
                        }
	;

any_value: 	ID_ANYCASE
		  { GetTokStr( $1 ); }
	| 	ID_QUOTED 
		  { GetTokStrQuoted( $1 ); }
	|	KW_TRUE
		  { GetTokStr( yyleng ); }
	|	KW_FALSE
		  { GetTokStr( yyleng ); }
	;

version_value:	ID_ANYCASE
		  {
		  cp = yytext;
		  if (!strchr( "VTXU", *cp )) 
			{
			/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
			/* msl_error_1( MCCMSL_S_INVVERSION, &yy_desc) */
                        yyerror(MP(mp813,"Invalid Version syntax"));
			YYERROR;
			};
		  /* Check if Version is UT */
		  if (*cp == 'U')
		      {
		      cp++;
		      if (!strchr("TX", *cp))
			  {
			  /* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
                          /* msl_error_1( MCCMSL_S_INVVERSION, &yy_desc) */
                          yyerror(MP(mp813,"Invalid Version syntax"));
			  YYERROR;
			  };
		      }
		  /* parse major number , must be at least one digit */
		  cp++;
		  do
			{
			if (!isdigit(*cp))  /* if we saw other than digit */
				{
				/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
                                /* msl_error_1( MCCMSL_S_INVVERSION, &yy_desc) */
                                yyerror(MP(mp813,"Invalid Version syntax"));
				YYERROR;
				};
			cp++;
			} while (*cp != '.');

		  /* parse minor number */
		  cp++;
		  do
			{
			if (!isdigit(*cp))  /* if we saw other than digit */
				{
				/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
                                /* msl_error_1( MCCMSL_S_INVVERSION, &yy_desc) */
                                yyerror(MP(mp813,"Invalid Version syntax"));
				YYERROR;
				};
			cp++;
			} while (*cp != '.');

		  /* parse ECO number */
		  cp++;
		  do
			{
			if (!isdigit(*cp))  /* if we saw other than digit */
				{
				/* yy_desc.mcc_w_maxstrlen = strlen(yytext); */
                                /* msl_error_1( MCCMSL_S_INVVERSION, &yy_desc) */
                                yyerror(MP(mp813,"Invalid Version syntax"));
				YYERROR;
				};
		  	cp++;
			} while (*cp);
		  $$ = yylval;
		  };


/*
|=============================================================================
| TOP OF THE TREE:  It all starts here
|=============================================================================
*/

mgmt_spec : smi_bidt_header
         |  spec_start type_decls body spec_end ;

/* ----------------------------------------------------------------------------
|
| "built-in" datatype type declarations for ALL SMIs
|
| NOTE: These rules are used exclusively to parse the "builtin_types"
|       file.  No syntax that invokes these rules should ever appear in
|       an MSL file.
*/
smi_bidt_header : smi_bidt_version smi_keyword_list smi_bidt_decls ;

smi_bidt_version: KW_BIDT_VERSION P_EQUAL ID_QUOTED 
                 {
                 /* Grab the Version ID w/o the quotes */
	         GetTokStrQuoted( $3 );

                 /* Grab some space to copy the Version string */
	         ascii_bidt_version = (char *)malloc(CurStrBufLen+1);

                 /* Copy the Version string to storage that won't be re-used */
                 strcpy(ascii_bidt_version, CurStrBuf);

                 /*
                 | If we've loaded a MIR database file, abort the parse and
                 | just allow the backend to compare BIDT version strings.
                 */
                 if (load_from_bin == TRUE) {
                     YYERROR;
                     }
                 }
                P_SEMICOLON
                 ;

smi_keyword_list : KW_KEYWORDS P_EQUAL
                      P_LPAREN
                          keyword_entries
                      P_RPAREN
                   P_SEMICOLON;

keyword_entries:
           keyword_entry
        |  keyword_entries P_COMMA keyword_entry;
	;

keyword_entry : ID_QUOTED 
               {
               /* Grab the Version ID w/o the quotes */
	       GetTokStrQuoted( $1 );

               /* Record this keyword in the backend */
               mirc_keyword(IT_IS_A_KEYWORD, CurStrBuf);
               }

smi_bidt_decls :     /* empty */
                 | smi_bidt_decls smi_section ;

smi_section : smi_start_section smi_builtin_list smi_end_section;

smi_start_section : KW_START_SMI object_header
                 {
                 /*
                 | Record the name and number of this new SMI whose
                 | datatypes we're parsing and storing.  Switch the
                 | "current" smi to the specified code so that
                 |  mirc_create_dataconstruct() stores under the right smi.
                 */
                 smi_code = $2;
                 TRY(mirc_record_smi(CurStrBuf, smi_code));
                 }
                 ;

smi_builtin_list :  builtin_dataconstruct
                  | smi_builtin_list builtin_dataconstruct;

builtin_dataconstruct : KW_SMI_BUILTIN_TYPE type_decl_name P_EQUAL IDcode
        {
        /* TRUE = "Built-IN to SMI" (we're parsing the   */
        /* "builtin_types.dat" file here)                */
        TRY(mirc_create_dataconstruct(CurStrBuf, $4, TRUE));
         }
   P_COMMA
   mcc_datatype_size
   ncl_cmip_code
   asn1_class
   asn1_tags    /* Application (and maybe. . .) Universal ASN1 tags */
   builtin_template_list
   builtin_oid 
   P_COLON builtin_decl P_SEMICOLON
         {
         /* This "pop" is opposite the "push" of the              */
         /* "create_dataconstruct" which does not affect "level"  */
         /* consequently we need an explicit NULL call (no        */
         /* use of the macro that intercepts "md_use_parent"      */
         TRY(mirc_use_parent(NULL));
         }
         ;

mcc_datatype_size: /* empty */
       | KW_MCC_DATATYPE_SIZE P_EQUAL IDcode
         {
         TRY(mirc_add_rel_number(MIR_DC_MCC_DT_Size, $3, MIR_SNUMBER));
         }
         P_COMMA
         ;

ncl_cmip_code: /* empty */
       | KW_NCL_CMIP_CODE P_EQUAL IDcode
         {
         TRY(mirc_add_rel_number(MIR_DC_NCL_CMIP_Code, $3, MIR_SNUMBER));
         }
         P_COMMA
         ;

asn1_class : /* empty */
       | KW_ASN1_CLASS P_EQUAL IDcode
         {
         TRY(mirc_add_rel_number(MIR_DC_ASN1_Class, $3, MIR_SNUMBER));
         }
         P_COMMA
         ;

asn1_tags :
        asn1_tag                   /* Application Tag  or . .  */
      | asn1_tags P_COMMA asn1_tag /* Application Tag followed by Universal Tag */
         ;

asn1_tag : /* empty */
      | KW_ASN1_TAG P_EQUAL IDcode
         {
         TRY(mirc_add_rel_number(MIR_DC_ASN1_Tag, $3, MIR_SNUMBER));
         }
         ;

builtin_template_list : /* empty */
              | builtin_templates;

builtin_templates :   P_COMMA builtin_template
            | builtin_templates P_COMMA builtin_template;

builtin_template :  KW_AUX_INFO P_EQUAL ID_QUOTED 
            {
            IDS                 *target_rel;    /* IDS form of specifed Rel  */
            mir_relationship    selected_rel;   /* One-Time Template/Regular */
            int                 start_index;    /* Index into CurStrBuf      */

            /* Grab the name of the MIR object relationship */
            GetTokStrQuoted( $3 );

            /*
            | If they are specifying One-Time Template via "<>" (e.g for
            | case-codes),
            */
            if (CurStrBuf[0] == '<' && CurStrBuf[(CurStrBufLen-1)] == '>') {
                selected_rel = MIR_DC_SMI_OTTemplate;
                CurStrBuf[(CurStrBufLen-1)] = '\0';     /* Blow off '>' */
                start_index = 1;
                }
            else {      /* Normal Template Relationship name . . . */
                selected_rel = MIR_DC_SMI_Template;
                start_index = 0;
                }

            /* if the relationship exists . . . */
            if ((target_rel = mirc_find_rel_IDS( &CurStrBuf[start_index]))
                != NULL) {

                /* Add it to the current object as target of selected rel. */
                TRY(mirc_add_relationship(selected_rel, target_rel));
                }
            else {
                yyerror(MP(mp814,"Template's relationship does not exist (misspelled?)"));
                YYERROR;
                }
            }
            ;

builtin_oid : /* empty */
           | oid_string ;

builtin_decl : KW_ATOMIC
             | type_cons;

smi_end_section: KW_END_SMI_DEF P_SEMICOLON ;



/* ----------------------------------------------------------------------------
|
| (the real) Start of a Management Specification
|
*/
spec_start : start_banner P_SEMICOLON start_rest 
	{

	/*
	|  Initialize parser here
        |
        |  Variables that should NOT be reset here include:
        |       eflag
        |       the_world
        |       smi_code (it has already been set)
	*/
        private_encountered = MCC_K_FALSE;
        do_include_statement = MCC_K_TRUE;
        identifier_clause_lineno = 0;   /* No clause missing yet */
        level = -1;             /* No valid level set yet */
        first_entity = MCC_K_TRUE;
        entitylevel = -1;       /* Entity-level (for DNS Ident) not set yet */
        found_dns_primary_name = MCC_K_FALSE;
	}
	;

start_rest:	
	start_version start_symprefix start_smi_indicator ;

start_version: KW_VERSION P_EQUAL version_value P_SEMICOLON;

start_symprefix:	KW_SYMBOL_PREFIX P_EQUAL ID_ANYCASE
	{
        GetTokStr( $3 );

        if (CurStrBufLen < MAX_SYMPREFIX_SIZE) {
            strcpy( prefix_buf, CurStrBuf );
            prefix_stored = FALSE;
            }
        else {
            eflag=TRUE;
            yyerror("Symbol Prefix value too long");
            }
	}
	semicolon_or_include
	;

start_smi_indicator: /* empty */
                   {
                   /*
                   | NOTE:
                   |  "mirc_set_smi()" defaults to MIR_UNKNOWN_SMI on
                   |  failure return.
                   */
                   info ("MSL specifies no SMI, defaulting to \"DNA\"");
                   if (mirc_set_smi( "DNA", &smi_code) != MC_SUCCESS) {
                       warn (MP(mp815,"Unrecognized SMI: defaulting to \"Unknown\""));
                       }
                   }

        |          KW_DEFINING_SMI P_EQUAL ID_ANYCASE
                   {
                   GetTokStr( $3 );
                   if (mirc_set_smi( CurStrBuf, &smi_code) != MC_SUCCESS) {
                       warn (MP(mp816,"Unrecognized SMI: defaulting to \"Unknown\""));
                       }
                   }
	           semicolon_or_include
                   /* P_SEMICOLON */
                   ;

spec_end: 	KW_END KW_SPECIFICATION optional_name op_spec_end_marker
	| KW_END ID_ANYCASE
	{
	GetTokStr( $2 );
	if (strcmp("SPECIFICATION.", CurStrBuf) != 0)
	    {
	    /* msl_error( MCCMSL_S_SYNTAXERR ) */
            yyerror(MP(mp817,"Invalid SPECIFICATION syntax"));
	    YYERROR;
	    }
	}
	;

op_spec_end_marker: /* empty */
	| P_PERIOD
	| P_SEMICOLON;

start_banner: KW_MANAGEMENT KW_SPECIFICATION ID_ANYCASE ;


/* ----------------------------------------------------------------------------
|
| Type Declaration portion of MSL grammar
|
*/

/* zero or more type declarations allowed */   
type_decls: 	/* empty */ 
	|	type_decls type_decl ;

file_name: ID_ANYCASE
	{
	GetTokStr( $1 );
	}
	;

/* we must create the type object before creating the type component objects */
type_decl: KW_TYPE type_decl_name P_EQUAL IDcode
                     {                            /* FALSE = "Built-Up" */
                     TRY(mirc_create_dataconstruct(CurStrBuf, $4, FALSE));
                     }
		type_cons
                     {
                     /*
                     | This "pop" is opposite the "push" of the
                     | "create_dataconstruct" which does not affect "level"
                     | consequently we need an explicit NULL call.
                     */
                     TRY(mirc_use_parent(NULL));
                     }
                 semicolon_or_include
                     ;
	
type_decl_name:	ID_ANYCASE
			{
			GetTokStr( $1 ); 
			}

/* 
| This production defines a new type ("builtup") in terms of an old type
| (that is, in terms of a "builtin" or already defined "builtup").
| Note: We enforce a restriction on the underlying datatype of a
|       "subrange" definition.
*/
type_cons:
		record 
	| 	set 
	| 	set_of
	| 	sequence
	| 	sequence_of
	|	range_of
	| 	enum 
	| 	bitset 
	|	alias_def
	|	subrange_def
		  {                   
		   switch (mirc_find_DC_code(existing_datatype)) {
			case MCC_K_DT_INTEGER8:
			case MCC_K_DT_INTEGER16:
			case MCC_K_DT_INTEGER32:
			case MCC_K_DT_INTEGER64:
			case MCC_K_DT_UNSIGNED8:
			case MCC_K_DT_UNSIGNED16:
			case MCC_K_DT_UNSIGNED32:
			case MCC_K_DT_UNSIGNED64:
			case MCC_K_DT_ENUMERATION:
			case MCC_K_DT_LATIN1STRING:
			    {
                            /* The datatype was OK for use w/subrange */
			    break;
			    }
			default:
			    {
			    /* msl_error( MCCMSL_S_INVSUBRANGEDT ) */
                            yyerror(
                                    MP(mp818,"Sub-Range Datatype Invalid, Legal: Integer, Enum, Latin1String"));
			    YYERROR;
			    break;
			    } /* End default case of switch */
			} /* End Switch */
		  } /* End subrange_def code */
	;

alias_def: defined_data_type ;

subrange_def:  defined_data_type
                         P_LSQRBRKT
                                integer_range_list
                         P_RSQRBRKT ;

integer_range_list:	integer_range |
	integer_range_list P_COMMA integer_range;

integer_range:	ID_ANYCASE
		  {
                  int                   bound_value;      /* Converted bound */
                  mir_value_type        bound_value_type; /* Signed/Unsigned */

		  for (cp = yytext; *cp; cp++)
			if (*cp == '.') break;
		  p_ellipses = cp;
		  if (!(*cp) )
			{
                        yyerror(MP(mp819,"Invalid Subrange Numeric value"));
			YYERROR;
			};
		  if (*p_ellipses != '.' || *(p_ellipses+1) != '.')
			{
                        yyerror(MP(mp819,"Invalid Subrange Numeric value"));
			YYERROR;
			};
		  *p_ellipses = '\0';

                  /* store lower bound */
                  bound_value = convert_as_needed(yytext, &bound_value_type);
                  TRY( mirc_add_rel_number( MIR_Min_Int_Val,
                                            bound_value,
                                            bound_value_type));

                  /* store upper bound */
                  bound_value = convert_as_needed((p_ellipses+2),
                                                   &bound_value_type);
                  TRY( mirc_add_rel_number( MIR_Max_Int_Val,
                                            bound_value,
                                            bound_value_type));
		  };

  /*
  | Productions for "built-up" datatypes that use "built-in templates"
  | in their definitions (such as enumerated, record, etc. types) follow:
  */

record: KW_RECORD
            {
            IDS   *record_IDS;  /* --> IDS for built-in template "record" */

            record_IDS = NULL;  /* Haven't found it yet                   */
            TRY(mirc_find_datatype("BIDT_RECORD", &record_IDS));

            if (record_IDS != NULL) {
                TRY(mirc_add_relationship(MIR_Structured_As, record_IDS));
                }
            }
         rec_flds KW_END KW_RECORD ;

rec_flds: 	rec_flds rec_fld
	|	rec_fld ;

rec_fld: object_header 
              {
              /* NOTE:
              | The order-of-insertion of these two relationships
              | (String, then Number) is Very Important!  Do Not Change!
              |  This allows a variable-length list of Variant-Record Case
              |  codes to be read-out by the Tier 1 functions that
              |  evaluate datatypes.  When the first string is returned,
              |  this marks the end of the case-codes.  The field-name
              |  relationship here must precede the Field_Code, otherwise
              |  the Field Code may be confused with a Case-code.
              */
              TRY(mirc_add_rel_string(MIR_Field_Name, CurStrBuf));
              TRY(mirc_add_rel_number(MIR_Field_Code, $1, MIR_SNUMBER));
              }
           defined_data_type semicolon_or_include
                  ;

rec_fld: KW_CASE multi_keywd_identifier KW_OF
         {
         /*
         | During AUGMENT mode, the compiler may make more than one pass
         | over TYPE statements at the beginning of an AUGMENT file if more
         | than one Entity-Class was named to be augmented.  We want to skip
         | all processing associated with these TYPE statements for passes
         | following the first.  Most TYPE-stmt processing consists of calls
         | to the mirc_add_rel*() family of functions that contain logic
         | to automatically skip processing for TYPE-stmt objects.  However,
         | variant-record processing (in this rule and "variant_case:") must
         | be explicitly skipped for augment passes greater than 1.
         |
         | (NOTE: "augment_pass" is zero when the mode is not AUGMENT, so we
         |        don't have to explicitly check the compiler_mode).
         */
         if (augment_pass <= 1) {
              /*
              |  If we're already in the process of handling a CASE
              |  field group, we shouldn't allow another nested CASE
              |  statement.  Check for that here.
              |
              |  If this restriction is to be lifted, several cells
              |  need to be turned into stacks:
              |             variant_idcode
              |             current_variant_record
              */
              if (variant_idcode != -1) {
                  yyerror(MP(mp820,"Nested CASE statements not allowed"));
                  YYERROR;
                  }

              /*
              |  We've hit "CASE", so we must convert this "RECORD" into
              |  a "VARIANT RECORD".  (We'll do this for every time we
              |  hit a "CASE" inside any given RECORD, but 2nd and
              |  subsequent calls are superfluous and non-damaging).
              */
              if (mirc_rec_to_vrec(mirc_get_current()) != MC_SUCCESS) {
                  yyerror(MP(mp821,"Unable to convert RECORD to VARIANT RECORD"));
                  YYERROR;
                  }

              /*
              |  Record the "parent" RECORD IDS (that is now a VARIANT
              |  RECORD) so that rule "variant_case" can return to it
              |  (in the situation where the 2nd and subseqent field
              |   groups are being processed).
              */
              current_variant_record = mirc_get_current();

              /* Get the Field Code for the specified "fixed-field" name */
              variant_idcode = mirc_find_fixedfield( CurStrBuf );

              /* If we didn't find it, report it */
              if (variant_idcode == -1) {
                  char      msg[200];
                  sprintf(msg,
                          MP(mp822,"'%s': No such fixed-field in this record"),
                          CurStrBuf);
                  yyerror(msg);
                  YYERROR;
                  }

              /* Copy field name to static buffer for later reference */
              strcpy(case_fixed_field_name, CurStrBuf);
              }
         }
         variant_case_list KW_END KW_CASE
              {
              /* See comment in rule "rec_fld:" above */
              if (augment_pass <= 1) {
                   /* Out of the CASE field group now */
                   variant_idcode = -1;
                   mirc_set_current(current_variant_record);
                   current_variant_record = NULL;
                   }
              }
         semicolon_or_include
	;

variant_case_list:	variant_case_list variant_case variant_field_list
	|		variant_case variant_field_list
	;

variant_case:	P_LSQRBRKT
            {
            /*-> IDS for built-in dataconstruct "CASE FIELD GROUP" */
            IDS   *BIDT_case_IDS;

            /* IDS for the INSTANCE of a "CASE FIELD GROUP" */
            IDS   *cfg_IDS;

            /* See comment in rule "rec_fld:" above */
            if (augment_pass <= 1) {

                /*
                | For 2nd and subsequent "variant-case"s invocations, return
                | to the variant-record IDS we're actually working on.
                */
                mirc_set_current(current_variant_record);

                /*
                | We insert three relationships just as the parser would
                | under rule "rec_fld : object_header ... defined_data_type".
                | In other words, we're creating a single fake "field" that
                | is of underlying type "CASE_FIELD_GROUP".  Then we
                | "step-down" into the IDS representing the instance of that
                | "field" and go on to compile the information describing the
                | variant-record fields that exist for ONE group of case-codes.
                | Each new group of variant-fields that occur (starting with
                | this production) results in another fake "field" being
                | created (by the code here) in the "parent" VARIANT-RECORD
                | we're actually creating.  A field in this VARIANT-RECORD
                | with a name of "" and code of 0 means this is a fake "field"
                | that is going to have a datatype of "CASE FIELD GROUP". . .
                | and that it is "built-UP".
                |
                | NOTE:
                | The order-of-insertion of these two relationships
                | (String, then Number) is Very Important!  Do Not Change!
                */
                TRY(mirc_add_rel_string(MIR_Field_Name, ""));
                TRY(mirc_add_rel_number(MIR_Field_Code, 0, MIR_SNUMBER));

                /*
                |  Now create a Non-Terminal to represent the instance of a
                |  CASE_FIELD_GROUP, and show it as the target of
                |  "MIR_Structured_As" (ie the "datatype" of the fake "field"
                |  in the VARIANT-RECORD we're actually working on).
                |
                |if (attempt to create a non-terminal object failed) */
                if ( (cfg_IDS = I_Create_IDS(I_NT_OBJECT,
                                             IDS_DFT_TABLESIZE)) == NULL) {
                    yyerror("Out of Memory");
                    YYERROR;
                    }

                /* Attempt to add MIR_Structured_As relationship */
                TRY(mirc_add_relationship(MIR_Structured_As, cfg_IDS));

                /*
                |  Make the VARIANT-RECORD IDS be the "parent" (insofar as
                |  the compiler is concerned) of this IDS that represents
                |  an instance of CASE_FIELD_GROUP.
                */
                cfg_IDS->idsu.nt.parent_obj = mirc_get_current();

                /*
                |  Now "step-down" into it to continue compiling the
                |  the stuff that has to go into this "instance" of a CASE
                |  FIELD GROUP.
                |
                |  The "step-up" that matches this "step-down" occurs
                |  following partial production:
                |
                |              "variant_case_list KW_END KW_CASE"
                |  above.
                */
                mirc_set_current(cfg_IDS);

                /*
                |  The "name" of an instance of CASE_FIELD_GROUP is just
                |  the Field Name of the fixed-field in the variant-record that
                |  selects the group, and the "idcode" of the instance is
                |  likewise the Field Code of the fixed-field.
                |  Store these things now.
                */
                TRY(mirc_add_rel_number(MIR_ID_Code,
                                        variant_idcode,
                                        MIR_SNUMBER));

                TRY(mirc_add_rel_string(MIR_Text_Name,
                                        case_fixed_field_name));

                /*
                |  The "datatype" of an instance of CASE_FIELD_GROUP is
                |  obviously just that: built-IN template
                |  "BIDT_CASE_FIELD_GROUP".
                */
                BIDT_case_IDS = NULL;  /* Haven't found it yet */

                TRY(mirc_find_datatype("BIDT_CASE_FIELD_GROUP",
                                       &BIDT_case_IDS));

                if (BIDT_case_IDS != NULL) {
                    TRY(mirc_add_relationship(MIR_Structured_As,
                                              BIDT_case_IDS));
                    }

                /*
                |  Now continue compilation w/case-codes, followed by the
                |  variant-record fields that make up this case-field-group,
                |  until we hit "END CASE" (which ends the processing of
                |  CASE GROUP FIELDS) or another "[case-codes]" list (which
                |  starts the processing of another set of CASE GROUP FIELDS).
                */
                }
            }
	variant_casecodes P_RSQRBRKT P_COLON
            ;

variant_casecodes:	variant_casecodes P_COMMA field_val
	|		field_val
	;

field_val: whole_number
	{
        TRY(mirc_add_rel_number(MIR_Case_Code, $1, MIR_SNUMBER));        
	}
	;

variant_field_list: 	variant_field_list rec_fld
	|		rec_fld 
	;

/*
| we only support SET OF (existing type), not SET OF RECORD OF (existing type)
|
| (Well, we can support SET OF RECORD OF provided RECORD OF is already
|  defined via a previously parsed TYPE statement, but whew!)
*/
set: 	KW_SET data_type_list
		;

data_type_list:	   set_or_seq_type
		|  data_type_list P_COMMA set_or_seq_type
		;

set_or_seq_type:
		  { 
                  IDS   *setseq_IDS;  /* --> IDS for built-in template "set" */
                  setseq_IDS = NULL;  /* Haven't found it yet                */
                  TRY(mirc_find_datatype("BIDT_SETSEQ", &setseq_IDS));
                  if (setseq_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As,setseq_IDS));
                      }
		  }
                defined_data_type
                  ;

set_of:		KW_SET KW_OF
		  { 
                  IDS   *set_IDS;  /* --> IDS for built-in template "set" */
                  set_IDS = NULL;  /* Haven't found it yet                */
                  TRY(mirc_find_datatype("BIDT_SET", &set_IDS));
                  if (set_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As, set_IDS));
                      }
		  }
                defined_data_type
                  ;

sequence:	KW_SEQUENCE data_type_list
		;

sequence_of:	KW_SEQUENCE KW_OF
		  { 
                  /* --> IDS for built-in template "sequence" */
                  IDS   *seq_IDS;
                  seq_IDS = NULL;  /* Haven't found it yet    */
                  TRY(mirc_find_datatype("BIDT_SEQ", &seq_IDS));
                  if (seq_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As, seq_IDS));
                      }
		  }
                defined_data_type
                  ;

range_of:	KW_RANGE KW_OF
		  { 
                  IDS   *rng_IDS;  /* --> IDS for built-in template "Range" */
                  rng_IDS = NULL;  /* Haven't found it yet                  */
                  TRY(mirc_find_datatype("BIDT_RANGE", &rng_IDS));
                  if (rng_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As, rng_IDS));
                      }
                  }
                defined_data_type
                  ;
 
enum:   /* enumerated types */
                P_LPAREN
                  {
                  /* --> IDS for built-in template "Enumeration" */
                  IDS   *enum_IDS;
                  enum_IDS = NULL;  /* Haven't found it yet      */
                  TRY(mirc_find_datatype("BIDT_ENUMERATION", &enum_IDS));
                  if (enum_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As, enum_IDS));
                      }
                  }
                 enum_vals P_RPAREN     /* SRM T1.0.0 */
                  ;


/* one or more enumerated type values, separated by commas */
enum_vals:  enum_val |
	    enum_vals P_COMMA enum_val ;

enum_val:       /*
                |  Switch LEX into a special state that allows it to accept
                |  darn near anything as the value of the
                |  multi-keyword-identifier.  This state allows us to accept
                |  strings which might otherwise be keywords, things in
                |  parentheses etc.
                */
                {
                begin_enum();
                }
            multi_keywd_identifier
                {
                /* Switch LEX back to it's initial (normal) state */
                begin_initial();
                }
            P_EQUAL integer_value
                  {
                  TRY(mirc_add_rel_number(MIR_Enum_Code, $5, MIR_SNUMBER));
                  TRY(mirc_add_rel_string(MIR_Enum_Text, CurStrBuf));
                  }
                  ;

bitset:     KW_BITSET KW_OF 
		  { 
                  /* --> IDS for built-in template "BitSet" */
                  IDS   *bitset_IDS;
                  bitset_IDS = NULL;  /* Haven't found it yet */
                  TRY(mirc_find_datatype("BIDT_BITSET", &bitset_IDS));
                  if (bitset_IDS != NULL) {
                      TRY(mirc_add_relationship(MIR_Structured_As,bitset_IDS));
                      }
                  }
             defined_data_type
                  ; 

/*
| The file contains *either*:
|
|           an AUGMENT ENTITY definitions or GLOBAL/CHILD ENTITY definition.
*/
body :  spec_body        /* Normal  */
      | augment_body;    /* Augment */


/* ----------------------------------------------------------------------------
|
| rules for an augment-entity definition
|
| MSL Syntax:
|
|       AUGMENT ENTITY { (<class-name-list) [ ,( <class_name_list) ] } ;
|
|  followed by AUGMENT ENTITY definitions (see below).
|
|  Note:  The Entity-Class object selected by this code has a
|         definition-status that is logically "PREDEFINED".  However, we
|         don't set it as there is no need:  No productions under AUGMENT
|         allow for calling "mirc_add_rel_*()" functions that depend on
|         the definition-status having been set.  (Ie, under AUGMENT, things
|         like "DYNAMIC = " *for an entity-class* cannot happen, because
|         entity-classes may not be defined to be augmented-into something
|         within an AUGMENT file).
|         
|   
*/
augment_body : KW_AUGMENT KW_ENTITY
                 P_LBRACE
                       {
                       /*
                       | You can't specify "merge" on the command-line for a
                       | file that is really an AUGMENT file.  Catch that here.
                       */
                       if (compiler_mode == CM_MERGE) {
                           yyerror(
                                   MP(mp845, "Merge not allowed on Augment File")
                                   );
                           }

                       /* Go to AUGMENT mode for sure */
                       compiler_mode = CM_AUGMENT;
                                   
                       /* Bump the augment pass count by 1 */
                       augment_pass += 1;

                       /* Set our local pass counter back to 1 */
                       local_pass_count = 1;
                       }
                   augment_entity_list
                       {
                       /* If we haven't recorded maximum pass number, do it.
                       |
                       |  ("augment_max_pass" is seen by the code in
                       |   mir_frontend.c which uses it with the value of
                       |   "augment_pass" to decide whether another pass over
                       |   the input file is needed)
                       */
                       if (augment_max_pass == 0) {
                           augment_max_pass = local_pass_count;
                           }
                       }
                 P_RBRACE P_SEMICOLON
               augment_definitions
/*               aug_ent_end  */
                 ;
/*  Add this per Dave Moore's suggestion:
aug_ent_end:     KW_END KW_AUGMENT KW_ENTITY P_SEMICOLON
                 ;
*/

augment_entity_list :
            aug_entity_specifier
          | augment_entity_list P_COMMA 
            {
            /* Mark the start of another entity-list (pass) by counting it */
            local_pass_count += 1;
            }
          aug_entity_specifier
            ;


aug_entity_specifier : P_LPAREN
                         aug_class_name_list
                       P_RPAREN
                       {
                       /*
                       |  Set the 'real' compiler level if this is the pass
                       |  we're really going to process.
                       */
                       if (local_pass_count == augment_pass) {
                           level = plevel;
                           fprintf(stderr, "\n");       /* Finish the path */
                           }
                       }
                       ;

aug_class_name_list :
                {
                /*
                |  Only perform the 'positioning' of where we're going to
                |  start compiling augments if the augment pass number matches
                |  the count of entity-class lists we're processing. . . 
                */
                if (local_pass_count == augment_pass) {
                    /* Make the "current object" be "The World" */
                    mirc_set_current(the_world_IDS);

                    /* Set the "parent level" counter to it's initial value:
                    |  "0" means "level pending"...set below in
                    |  "aug_ent_class_name"
                    */
                    plevel = 0;

                    /*
                    |  Issue an Info message indicating the Entity-Class
                    |  we'll be augmenting so that if any diagnostics pop out,
                    |  it'll be clear which Entity-Class had the problem.
                    */
                    fprintf(stderr,
                            MP(mp846,
                               "mirc - Info: Augmenting Entity-Class at end of following path:\n       "));
                    }
                }
          aug_ent_class_name
        | aug_class_name_list P_COMMA aug_ent_class_name
        ;

aug_ent_class_name: multi_keywd_identifier
    {
    IDS     *next_parent;   /* the next parent down goes here          */
    int     idcode;         /* Retrieved ID code for this entity class */
    mir_relationship rel;   /* Cell so we can pass the address of this */
    char    *object_name;   /* --> String name of Entity-Class         */


    /*
    |  Only perform the 'positioning' of where we're going to
    |  start compiling augments if the augment pass number matches
    |  the position of the entity-class in the augment lists we're processing.
    */
    if (local_pass_count == augment_pass) {

        /*
        | We've parsed the name of the next 'parent' DOWN, so try to find
        | it as an entity-class contained by the "current-object".  If we
        | can't, it's game-over.
        */
        if ((next_parent = mirc_find_obj_IDS(MIR_Cont_entityClass, CurStrBuf))
            == NULL) {
            char msg[200];
            sprintf(msg,
                    MP(mp847,"No Such Augment Entity-Class Name '%s'."),
                    CurStrBuf);
            yyerror(msg);
            YYERROR;
            }

        /*  Ok, we found a valid 'step down', so step down to new 'parent' */
        mirc_set_current(next_parent);

        /*
        |  Record the ID_Code of this parent for use in building the OID that
        |  may describe it and it's children.
        |
        |  We have to record both the MCC Code (if there is one) and the
        |  "Standard" Code (if there is one).  Due to the structure of
        |  "mirc_record_idcodes()",  we MUST try to record the MCC code first.
        */
        /* Get the MCC code or "-1" if there wasn't one */
        idcode = mirc_find_obj_CODE(next_parent, MCC_code);

        /* Go to next parent level to record codes/names */
        plevel += 1;

        /* (So we can pass address of this cell) */
        rel = MIR_Cont_entityClass;       

        /* Get the Entity Class Name */
        object_name = mirc_find_obj_NAME(next_parent);

        mirc_record_idcodes(plevel,          /* Level we're at        */
                            MCC_code,        /* Type of code to store */
                            idcode,          /* The code              */
                            &rel,            /* Kind of MIR Object    */
                            object_name);    /* Name of the object    */

        /* Get the Standard code or "-1" if there wasn't one */
        idcode = mirc_find_obj_CODE(next_parent, STD_code);
        mirc_record_idcodes(plevel,          /* Level we're at        */
                            STD_code,        /* Type of code to store */
                            idcode,          /* The code              */
                            NULL,            /* Not needed            */
                            NULL);           /* Not needed            */

        /* Tack on the next entity-class name onto the path */
        fprintf(stderr, "\"%s\" ", object_name);
        }
    }
    ;

/*
| These are the things that may be 'augmented-into' the things named in the
| "augment-entity-list"
*/
augment_definitions: 
             /* empty */
           | augment_definitions entity_body_item ;

/* ----------------------------------------------------------------------------
|
| rules for an entity class definition
|
| MSL Syntax:
|
|       <GLOBAL | CHILD> ENTITY <class-name> = <code> :
|           [ PARENT = ( <class-name> {, <class-name> }), ]
|           IDENTIFIER = ( <identifying-atttributes-list> ),
|           [DYNAMIC - <"TRUE"> | <"FALSE">, |
|           [DEPENDS ON = "<variant-expression>",]
|           [SYMBOL = <Latin1String>, ]
|
*/
spec_body: entity_id  parent
	{
	ObjectCode = EClassCode;
        entitylevel = level;    /* Record level of current entity class */

        TRY(mirc_create_object (
            MIR_Cont_entityClass, EClassCode, EClassNameBuf, &level));

        /*
        | FALSE = We've syntactically seen the first entity in the file, any
        | others that follow must perforce be CHILDREN of this entity, which
        | implies:
        |
        | * "GLOBAL" keyword should not be seen again during this invocation
        |   of the parser
        | * the "CHILD" entities that follow must have a "PARENT =" clause
        |   that has the correct "parentage" list in it (or the clause may
        |   be absent).
        */
        first_entity = FALSE;

        /*
        | Record the current Entity Class object IDS address for use in
        | "depends-on" processing
        */
        current_eclass = mirc_get_current();

        /*
        | If we're passing this way for the first time for this file, then
        | we need to store the Symbol Prefix that was parsed earilier, and
        | then record the fact that we've done it.
        */
        if (prefix_stored == FALSE) {
            TRY( mirc_add_rel_string( MIR_Symbol_prefix, prefix_buf ));
            prefix_stored = TRUE;
            }

        /* <ADHACK>: Remove the following statements */
        dynamic_value = MCC_K_FALSE;    /* Set dft if no DYNAMIC= parsed */
        dynamic_present = MCC_K_FALSE;  /* Show no "DYNAMIC=" parsed yet */
        /* <ADHACK>: Remove the previous statements */
	}

	op_incl_stmt
	entity_item_list
        write_dynamic_def       /* <ADHACK>: Remove this line */
	entity_body_list
	subent_defs
	ent_end
        ;

entity_item_list: /* empty */
	| entity_item_list entity_item comma_or_include
	;

entity_item:
	  oid_line
        | identifier_list
	| dynamic
	| depends_on
	| depends_expression
	| include_symbol
	;

        /* <ADHACK> - Remove this entire production and it's action code */
write_dynamic_def: /* empty */
        {
        /*
        | If we're hacking, decide whether to write the MIR_Dynamic out
        */
        if (ad_hack_switch == TRUE) {

            /* "DYNAMIC="
            | While hacking we only write "MIR_Dynamic" if the
            | compiler mode is:
            |
            |    CM_NORMAL
            |       OR
            |    CM_MERGE and definition status of ENTITY is "CREATED"
            |       OR
            |    CM_AUGMENT and-> the "DYNAMIC=" clause was PRESENT
            |               or -> the definition status of ENTITY is
            |                     CREATED.
            |
            |    Under AUGMENT, mirc_add_relationship() will issue an
            |    error if the definition status of ENTITY was
            |    PREDEFINED, (you can't modify via AUGMENT a pre-existing
            |    object, so this is right. . . the bummer is that the
            |    line number that will be reported is that of the line just
            |    before the attributes/dir./events. . .this might be a bit
            |    difficult to interpret since it will be the "DYNAMIC="
            |    clause causing the problem).
            */
            if ( (compiler_mode == CM_NORMAL)
                || ((compiler_mode == CM_MERGE)
                    && (mirc_get_def_status() == DS_CREATED))
                || ((compiler_mode == CM_AUGMENT)
                    && ((dynamic_present == MCC_K_TRUE)
                        || (mirc_get_def_status() == DS_CREATED)))
                ) {
                TRY( mirc_add_rel_number(MIR_Dynamic,
                                         dynamic_value,
                                         MIR_SNUMBER ));
                }
            }
        }
        ;

entity_body_list:
	  /* empty */
	| entity_body_list entity_body_item
	;

entity_body_item:
	  attr_group
        | attr_part
        | direc_def
        | event_defs
	;


entity_id:  ent_scope KW_ENTITY object_header
        {
	strcpy( EClassNameBuf, CurStrBuf );
        EClassCode = $3;

        /*
        |  If we're entering a nested entity-class definition, then what we're
        |  checking here is to be sure the containing class has an
        |  IDENTIFIER=() clause.
        |
        |  NOTE: The number stored in "identifier_clause_lineno" is actually
        |        the line number of the ENTITY statement that should have an
        |        IDENTIFIER=() clause in it.
        */
        if (identifier_clause_lineno != 0) {
            char    buff[200];

            sprintf(buff,
                    MP(mp848,
                       "Entity defined in line %d has no IDENTIFIER=() clause"),
                    identifier_clause_lineno);
            yyerror(buff);
            }

        /* Record the fact that we haven't seen an IDENTIFIER=() clause yet */
        identifier_clause_lineno = yylineno;
        }
        ;

ent_scope: 	KW_GLOBAL 
		{
                /*
                | The only place it is legitimate to put a GLOBAL entity
                | definition is as the first-entity in a file.  If we're
                | now working on the first entity, that's groovy and we can
                | set the 'current_object' accordingly, otherwise its an errer.
                */
                if (first_entity == MCC_K_TRUE) {
                    mirc_set_current(the_world_IDS);
                    }
                else {
                    yyerror(MP(mp823,"GLOBAL entity must be first in file"));
                    YYERROR;
                    }

                /* Level for Global Entity is "1" */
		level = 1;      

		}
	| 	KW_CHILD
		{
		if (first_entity)
		    level = 0;  /* Child Entity, level as-yet Unknown */
		else 
                    /* New predicted level, to be checked @ 'parent:' */
		    level++;
		}               
	;

parent: /* empty */
        {
        /*
        | Here with "first_entity" = TRUE implies we're processing the
        | 'empty space' that corresponds to where the "PARENT=" clause *would*
        | appear for the first syntactic entity (topmost-mentioned) in an MSL
        | file . . . 
        |                            and
        | "level" = 0 means we've parsed the keyword "CHILD" and we know
        | that this first entity in the file is not a GLOBAL entity,
        | consequently it is an error.
        */

        /* if top-level entity not a global entity */
        if (first_entity && level == 0) {
            /* msl_error( MCCMSL_S_NOPARENTENTITY ) */
            yyerror(MP(mp824,"First File Entity is not Global: Missing PARENT= clause"));
            YYERROR;
            }
        }

     |  KW_PARENT P_EQUAL P_LPAREN class_name_list P_RPAREN P_COMMA
		{
		if (level == 1) /* if a GLOBAL entity declaration */
			{
			/* msl_error( MCCMSL_S_INVPARENTLOC ) */
                        yyerror(MP(mp825,"PARENT= clause invalid for Global Entity"));
			YYERROR;
			}
		else    /*  it is a CHILD entity declaration
                        |
                        | ...and the FIRST entity in the file.  This means that
                        | the parser doesn't really have a "current object" in
                        | the MIR where 'its at'.  So we 'accept' the current
                        | object (generated by the walk down through the
                        | parent names in rule 'ent_class_name') as the proper
                        | parent object of the entity we're just starting to
                        | parse. . .(ie we leave "current_object" alone!).
                        |
                        | We compute the 'level' based on the depth of the
                        | "PARENT=" list.
                        |
                        | NOTE: If the compiler is running in Merge, and since
                        |       this entity is the first in the file, then
                        |       we assume that the entity containing it (the
                        |       "current_object" must already have been
                        |       defined, hence we mark it as "PREDEFINED".
                        */
                        if (level == 0 && first_entity)
                            {
                            level = $4 + 1;    /* initialize level */

                            /*
                            | For Merge, the current_object must be predefined,
                            | We don't set it's status because it doesn't
                            | matter a great deal as all we're going to do is
                            | step down and create the new CHILD entity under
                            | it when "mirc_create_object()" gets called for
                            | this Entity-Class.
                            */
                            }

                        else{/* ...its a CHILD entity, NOT the FIRST in a file.
                             |
                             | In this case, the parser did have a
                             | "current_object" it thought was the proper
                             | parent of the entity we are about to parse, now
                             | saved in "saved_current".
                             |
                             | We've walked the "current_object" down the
                             | internal MIR representation from the root 
                             | through all the parents, and the "current
                             | object" now should be SAME ONE we were at
                             | before we started to parse the PARENT=clause.
                             | We check this.
                             |
                             | Also, if the depth computed from the "PARENT="
                             | list doesn't match the level computed by
                             | incrementing 'level' at the 'ent_scope' rule
                             | then we have an error.
                             |
                             | For Merge, definition-status is as above.
                             */
                             if (   $4 != (level - 1) 
                                 || saved_current != mirc_get_current()) {
                                 /* level doesn't match PARENT declaration */
                                 /* msl_error( MCCMSL_S_INVPARENTNUM ) */
                                 yyerror(
                                         MP(mp826,"PARENT= clause depth does not match actual depth"));
                                 YYERROR;
			         };
                             }
                }
	;

/*
| List of one or more class names in the PARENT= clause.
|
| We set the "current_object" to the IDS for the root MIR Object
| ("Parent Of Everything").  Then as we parse off the names of the parents,
| we step this pointer down through the MIR objects that represent those
| parents, checking at each step to be sure they really are there.
|
| In the process we compute and return the number of class names seen.
*/
class_name_list:
		{
                /* Initialize our pointers for moving thru MIR top-down */
                saved_current = mirc_get_current();

                /* Make the "current object" be "The World" */
                mirc_set_current(the_world_IDS);

                /* Set the "parent level" counter to it's initial value:
                |  "0" means "level pending"...set below in "ent_class_name"
                */
                plevel = 0;
		}
           ent_class_name
                {
                /* Initialize the counter of parents */
                $$ = 1; 
                }

        |  class_name_list P_COMMA ent_class_name
		{
                /* Count another parent level passed thru */
                $$ = $1 + 1;
		}
	;

ent_class_name: multi_keywd_identifier
	{
        IDS     *next_parent;   /* the next parent down goes here          */
        int     idcode;         /* Retrieved ID code for this entity class */
        mir_relationship rel;   /* Cell so we can pass the address of this */

        /*
        | We've parsed the name of the next 'parent' DOWN, so try to find
        | it as an entity-class contained by the "current-object".  If we
        | can't, it's game-over.
        */
        if ((next_parent = mirc_find_obj_IDS(MIR_Cont_entityClass, CurStrBuf))
            == NULL) {
            char msg[200];
            sprintf(msg, MP(mp849,"Invalid Parent Name '%s'."), CurStrBuf);
            yyerror(msg);
            YYERROR;
            }

        /*  Ok, we found a valid 'step down', so step down to new 'parent' */
        mirc_set_current(next_parent);

        /*
        |  Record the ID_Code of this parent for use in building the OID that
        |  may describe it and it's children.
        |
        |  We have to record both the MCC Code (if there is one) and the
        |  "Standard" Code (if there is one).  Due to the structure of
        |  "mirc_record_idcodes()",  we MUST try to record the MCC code first.
        */
        /* Get the MCC code or "-1" if there wasn't one */
        idcode = mirc_find_obj_CODE(next_parent, MCC_code);

        /* Go to next parent level to record codes/names */
        plevel += 1;

        /* (So we can pass address of this cell) */
        rel = MIR_Cont_entityClass;       

        mirc_record_idcodes(plevel,             /* Level we're at        */
                            MCC_code,           /* Type of code to store */
                            idcode,             /* The code              */
                            &rel,               /* Kind of MIR Object    */
                            mirc_find_obj_NAME( /* Name of the object    */
                                  next_parent));

        /* Get the Standard code or "-1" if there wasn't one */
        idcode = mirc_find_obj_CODE(next_parent, STD_code);
        mirc_record_idcodes(plevel,             /* Level we're at        */
                            STD_code,           /* Type of code to store */
                            idcode,             /* The code              */
                            NULL,               /* Not needed            */
                            NULL);              /* Not needed            */
	}
	;

identifier_list: KW_IDENTIFIER_EQ P_LPAREN
        {
        /* Show an empty Identifier list */
        ident_name_list = NULL;
        ident_list_next = &ident_name_list;

        /* Record the fact that we've seen an IDENTIFIER=() clause */
        identifier_clause_lineno = 0;
        }
     attribute_list
        {
        FRB     *ident_FRB;     /* Forward-Reference Block (if we make one) */

        /*
        |  We will only do the processing if we're compiling Normal or Merge
        |  (Augment is not possible due to syntax) *and* the object
        |  has definition status CREATED.
        */
        if (mirc_get_def_status() == DS_CREATED) {

            /*
            | If we parsed no Identifier name at all . . . 
            | (e.g. "IDENTIFIER = ()". . . )
            */
            if (ident_name_list == NULL) {
                /*
                | Insert a "MIR_Indexed_By" relationship to mark the fact we've
                | hit an entity, but make target be a zero: "nothin' here".
                */
                TRY(mirc_add_rel_number(MIR_Indexed_By, 0, MIR_SNUMBER));
                }
            else {
                /*
                |  Copy the list of identifier names into a created
                | "forward reference block" for later resolution
                */
                TRY(mirc_create_fwd_ref(FR_IDENT, &ident_FRB, NULL));
                ident_FRB->fru.i.attr_names = ident_name_list;
                ident_name_list = NULL;     /* Gone! */
                ident_list_next = NULL;
                }
            }
        }        
     P_RPAREN
        ;

/* LIST of zero or more identifier attribute names, separated by commas */
attribute_list:	
                /* empty */
        |       ident_attribute_name
        |       attribute_list P_COMMA ident_attribute_name ;


ident_attribute_name: multi_keywd_identifier
        {
        /*
        |  We will only do the processing if we're compiling Normal or Merge
        |  (Augment is not possible due to syntax) *and* the object
        |  has definition status CREATED.
        */
        if (mirc_get_def_status() == DS_CREATED) {
            /*
            | Stash the next attribute name from the IDENTIFIER= list into
            | a forward reference "STRING" block for later reference
            */
            TRY(mirc_create_fwd_ref(FR_STRING, ident_list_next, CurStrBuf));

            /*
            | Re-record where we should record the address of any subsequent block
            */
            ident_list_next = &((*ident_list_next)->next);
            }
        }
        ;

dynamic:
	KW_DYNAMIC P_EQUAL true_false
	{
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /*     - > TRUE  */
            TRY( mirc_add_rel_number( MIR_Dynamic, $3, MIR_SNUMBER));
            }
        else {  /* Do Apply-Default Hack stuff - > FALSE */
            dynamic_value = $3; /* Record parsed value for future reference */
            dynamic_present = MCC_K_TRUE;   /* Signal "DYNAMIC=" was parsed */
            }
	};

ent_end: KW_END KW_ENTITY optional_name semicolon_or_include
	{ 
        /*
        | As we terminate the definition of the current entity-class, we're
        | checking here is to be sure the class has an IDENTIFIER=() clause.
        |
        |  NOTE: The number stored in "identifier_clause_lineno" is actually
        |        the line number of the ENTITY statement that should have an
        |        IDENTIFIER=() clause associated with it.  Compiler mode is
        |        not an issue, as you can't AUGMENT an Entity-Class into
        |        another Entity-class.
        */
        if (identifier_clause_lineno != 0) {
            char    buff[200];

            sprintf(buff,
                    MP(mp848, "Entity defined in line %d has no IDENTIFIER=() clause"),
                    identifier_clause_lineno);
            yyerror(buff);

            /*
            | We clear the error condition, now that all errors of this type
            | have been reported.
            */
            identifier_clause_lineno = 0;
            }

        /*
        | One and only one identifier must be chosen as the DNS ident
        | for a Global Entity when the SMI is DNA.  If we just finished
        | processing a Global Entity, a DNS Primary Name should have been
        | found.
        */
        if (   level == 1
	    && found_dns_primary_name != MCC_K_TRUE
            && smi_code == MIR_SMI_DNA
           ) {
            /* msl_error( MCCMSL_S_MISDNSPNAME ) */
            yyerror(MP(mp827,"Missing DNS Primary Name definition"));
            YYERROR;
            }

	TRY(mirc_use_parent(&level));
	if (level < 0)
		{
		/* msl_error( MCCMSL_S_BADENTITY_CNT ) */
                yyerror(MP(mp828,"Beginning-END ENTITY mismatch"));
		YYERROR;
		}
	}
	;

/* series of zero or more sub-entity definitions */
subent_defs: 	/* empty */
	|	subent_defs spec_body ; 


/* ----------------------------------------------------------------------------
|
| rules for attribute groups
|
|       { ATTRIBUTE GROUP <attribute-group-name> = <code> :
|               [SYMBOL = <Latin1String>, ]
|               [CATEGORIES = ( < [CONFIGURATION] [,FAULT]
|                       [,PERFORMANCE] [,SECURITY] [,ACCOUNTING] [ALL> ),]
|               ATTRIBUTE-LIST = ( <attribute-name> {, <attribute-name> } )
|       END ATTRIBUTE GROUP [<attribute-group-name>];}
|
*/
attr_group:
	attr_group_id
        attr_group_list
        write_category_def      /* <ADHACK>: Remove this line */
        attr_group_end ;

attr_group_id:
	KW_ATTRIBUTE KW_GROUP object_header
		{
		ObjectCode = $3;
                TRY(mirc_create_object(
                    MIR_Cont_attrGroup, ObjectCode, CurStrBuf, &level));

                /* Show no attribute-names encountered yet */
                attrgrpcount = 0;

                /*
                | If the mirc_create_object() call actually created a new
                | object for the Attribute Group list, then we store the
                | list-type into it.  ("mirc_create_object()" always sets
                | the definition status)
                */
                if (mirc_get_def_status() == DS_CREATED) {
                    TRY( mirc_add_rel_number(MIR_List_Type,
                                                 LT_ATTR_GROUP,
                                                 MIR_SNUMBER ));
                    }

                /* <ADHACK>: Remove the following statements */
		partition_setable = MCC_K_NONSETABLE;
                category_present = MCC_K_FALSE;
                /* <ADHACK>: Remove the previous statements */
		}
		op_incl_stmt
		;

attr_group_list:
	  attr_group_item
	| attr_group_list comma_or_include attr_group_item
	;

attr_group_item:
	  include_symbol
	| categories
	| oid_line
	| attr_group_attrs
	;

attr_group_end:
	KW_END KW_ATTRIBUTE KW_GROUP optional_name semicolon_or_include
		{
		if (attrgrpcount == 0)
		    {
                    yyerror(MP(mp829,"Empty Attribute Group list"));
		    YYERROR;
		    }

                /* <ADHACK>: Remove the following statements */
                if (ad_hack_switch == TRUE) {

                    /* 'ACCESS' for Attribute Group
                    |
                    | While hacking we only write "MIR_Access" for an
                    | Attribute Group if the compiler mode is:
                    |
                    |    CM_NORMAL
                    |       OR
                    |    CM_MERGE and definition status of GROUP is "CREATED"
                    |       OR
                    |    CM_AUGMENT and the definition status of GROUP is
                    |                                                  CREATED.
                    |    In other words, we write if mode is NORMAL OR
                    |    definition status is CREATED.
                    */
                    if ( (compiler_mode == CM_NORMAL)
                        || (mirc_get_def_status() == DS_CREATED)
                        ) {
                        TRY( mirc_add_rel_number(MIR_Access,
                                                 partition_setable,
                                                 MIR_SNUMBER ));
                        }
                    }
                /* <ADHACK>: Remove the previous statements */

		mirc_use_parent(&level);
		}
		;


attr_group_attrs:
	KW_ATTRIBUTE_LIST P_EQUAL P_LPAREN esgroup_attribute_list P_RPAREN ;


/* list of zero or more identifier attribute names, separated by commas */
esgroup_attribute_list:
          esgroup_attribute_name
        | esgroup_attribute_list P_COMMA esgroup_attribute_name ;

esgroup_attribute_name:	multi_keywd_identifier
	{
        IDS *esal;      /* Entity-Specific Attribute list */
        IDS *attr;      /* Attribute being sought         */

        /*
        | The current object is the entity-specific attribute list
        |
        | We must "step back" to it's parent (the entity itself) and
        | search for an attribute given the "name" just-now recognized.
        |
        | NOTE: With the way Merge mode is implemented now, you can't
        |       "merge-in" the name of an attribute (whether it was newly
        |       created by being mentioned in the Merge file or not) if
        |       the Group already existed.
        |
        |       It wouldn't be too hard to allow Merging an attribute
        |       that actually was merged (created) by the Merge file (into
        |       an already-existing group). It would require forcing the
        |       definition-status on the Group object itself to CREATED,
        |       and checking the definition-status of the attribute that is
        |       found.  (We could leave the forced status of CREATED on the
        |       Group, as all minor definitions would have already been
        |       parsed).
        |
        |       Under Augment, currently you can only Augment-in a mention
        |       of an attribute if the Group itself didn't exist until being
        |       mentioned in the Augment file.  This too could be changed.
        */
        esal = mirc_get_current();    /* Remember where we are */
        mirc_use_parent(NULL);        /* Step back momentarily */
        if ((attr = mirc_find_obj_IDS(MIR_Cont_attribute, CurStrBuf))
            != NULL) {
            mirc_set_current(esal);   /* Go back to attr list */
            TRY(mirc_add_relationship(MIR_List_Entry, attr));
            attrgrpcount += 1;        /* Count an attribute entered to list */

            /* <ADHACK> - Delete this If statement
            |
            | If we're hacking, we need to acquire this attribute's "ACCESS="
            | value to see if it is not "NON-SETABLE" so that the GROUP can
            | be marked accordingly.
            */
            if (ad_hack_switch == TRUE) {

                int i;  /* Handy-Dandy Loop index */

                /* Check every entry . . . . */
                for (i = 0; i < attr->idsu.nt.entry_count; i++) {

                    /* . . . until MIR_Access entry is found */
                    if (attr->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym
                        == MIR_Access) {

                        /* If the target of MIR_Access is NOT "NON-SETABLE"..*/
                        if (
                 attr->idsu.nt.rel_table[i].tar_obj->idsu.t.t_v.snumber.value
                            != MCC_K_NONSETABLE) {

                            /* Record Group as Setable */
                            partition_setable = MCC_K_SETABLE;
                            }

                        /* If we hit MIR_Access, that's it */
                        break;
                        }
                    }
                } /* <ADHACK> - End of ADHACK "If" statement */

            }
        else {
            yyerror(MP(mp830,"Unable to find attribute name"));
            YYERROR;
            }
	}
	;

/* The DIRECTIVE object does not call this grammar rule because the
   directive name "SET" is matched by the TOKEN KW_SET */
object_header: multi_keywd_identifier optional_IDcode  P_COLON
        {
        $$ = $2;
        };

optional_IDcode: /* empty */
          {
          $$ = -1;  /* signal "No ID Code" ("-1" cannot be legally parsed) */
          }

        | P_EQUAL IDcode 
          {
          $$ = $2;
          }
          ;


        /* <ADHACK> - Remove this entire production and it's action code */
write_display_def: /* empty */
	{
        /*
        | If we're hacking, decide whether to write the MIR_Display out
        */
        if (ad_hack_switch == TRUE) {

            /* "DISPLAY="
            | While hacking we only write "MIR_Display" if the
            | compiler mode is:
            |
            |    CM_NORMAL
            |       OR
            |    CM_MERGE and definition status of current-object is "CREATED"
            |       OR
            |    CM_AUGMENT and-> the "DISPLAY=" clause was PRESENT
            |               or -> the definition status of current-object is
            |                     CREATED.
            |
            |    Under AUGMENT, mirc_add_relationship() will issue an
            |    error if the definition status of the current-object was
            |    PREDEFINED, (you can't modify via AUGMENT a pre-existing
            |    object, so this is right. . . the bummer is that the
            |    line number that will be reported will not be exactly right.
            */
            if ( (compiler_mode == CM_NORMAL)
                || ((compiler_mode == CM_MERGE)
                    && (mirc_get_def_status() == DS_CREATED))
                || ((compiler_mode == CM_AUGMENT)
                    && ((display_present == MCC_K_TRUE)
                        || (mirc_get_def_status() == DS_CREATED)))
                ) {
                TRY( mirc_add_rel_number(MIR_Display,
                                         display_value,
                                         MIR_SNUMBER ));
                }
            }
	}
        ;

        /* <ADHACK> - Remove this entire production and it's action code */
write_category_def: /* empty */
	{
        /*
        | If we're hacking, decide whether to write the MIR_Category out
        */
        if (ad_hack_switch == TRUE) {

            /* "CATEGORIES="
            | While hacking we only write "MIR_Category" with default value
            | of "MCC_K_OSI_ALL" if
            |
            |   * no "CATEGORIES=" clause was parsed
            |
            | AND the compiler mode is:
            |
            |    CM_NORMAL
            |       OR
            |    CM_MERGE and definition status of current-object is "CREATED"
            |       OR
            |    CM_AUGMENT and the definition status of current-object is
            |               CREATED.
            |
            |    (ie, we write if it was'nt parsed and (the mode is NORMAL
            |     or the object was CREATED).
            */
            if (  (category_present == MCC_K_FALSE)
                && ((compiler_mode == CM_NORMAL)
                    || (mirc_get_def_status() == DS_CREATED))
                ) {
                TRY( mirc_add_rel_number(MIR_Category,
                                         MCC_K_OSI_ALL,
                                         MIR_SNUMBER ));
                }
            }
	}
        ;


/* ----------------------------------------------------------------------------
|
| rules for attributes & attribute partitions
|
|       { <IDENTIFIER | STATUS | COUNTER | CHARACTERISTIC | REFERENCE
|         <STATISTIC> ATTRIBUTES
|               { ATTRIBUTE <attribute-name> = <code> : <SMI Datatype>
|                       [DEPENDS ON = "<variant-expression>",]
|                       [PREDICTABLE = <"TRUE" | "FALSE">,]
|                       [ACCESS = <SETTABLE | NONSETTABLE | WRITEONLY>,]
|                       [DISPLAY = <"TRUE" | "FALSE">,]
|                       [DNS_IDENT = <"PRIMARY_NAME" | "ALTERNATE_NAME" |
|                                     "NOT_USED">,],]
|                       [UNITS = <UnitType>,]
|                       [DEFAULT = < <value> | NO_DEFAULT |
|                               IMPLEMENTATION SPECIFIC>,]
|                       [SYMBOL = <Latin1String>,]
|                       [CATEGORIES = (<[CONFIGURATION] [,FAULT]
|                               [,PERFORMANCE] [,SECURITY] [,ACCOUNTING]
|                               | ALL> ),]
|                  END ATTRIBUTE [<attribute-name>];}
|          END ATTRIBUTES;}
*/
attr_part:
	attrib_kind KW_ATTRIBUTES
	  {
          CurAttrGroup = $1;
          TRY(mirc_create_object(
                    MIR_Cont_attrPartition, $1, AttrPartString[$1], &level));

          /*
          | If the mirc_create_object() call actually created a new
          | object for the Attribute Partition (list), then we store the
          | list-type into it.  ("mirc_create_object()" always sets
          | the definition status)
          */
          if (mirc_get_def_status() == DS_CREATED) {
              TRY( mirc_add_rel_number(MIR_List_Type,
                                           LT_ATTR_PART,
                                           MIR_SNUMBER ));
              }

          /*
          | The Attribute Partition MIR Object is now the current object.  We 
          | obtain the IDS address for this object so that the action
          | routine associated with "attribute" can take a moment to add
          | each attribute created to this Attribute Partition.  Stash the
          | IDS address globally within this module for that purpose.
          */
          current_attr_partition = mirc_get_current();

          /*
          | (After a mirc_create_object() call, the current-object always has
          |  a valid definition-status value assigned).
          |
          | Force the definition-status of the (current) partition object to be
          | "CREATED" so that mirc_add_rel_*() calls always work.  We'll
          | control the writing to the partition object explicitly for
          | Merge and Augment modes at rule "attribute_definition".
          */
          mirc_set_def_status(DS_CREATED);

          /* <ADHACK>: Remove the following statements */
          partition_setable = MCC_K_NONSETABLE;
          /* <ADHACK>: Remove the previous statements */

          /*
          | Now pop back up to entity-class so that it appears to be the 
          | 'natural' parent for the attributes as we go on to parse them.
          */
          mirc_use_parent(&level);
	  }
	op_incl_stmt
	attrib_def_list KW_END KW_ATTRIBUTES semicolon_or_include
	  {
          CurAttrGroup = -1;

          /* <ADHACK>: Remove the following statements */
          if (ad_hack_switch == TRUE) {

              IDS *current;     /* Save the Entity Class here */

              /*
              | Save where we are so we can step down and write into the
              | Attribute Partition MIR Object.
              */
              current = mirc_get_current();
              mirc_set_current(current_attr_partition);

              /* 'ACCESS' for Attribute Partition
              |
              | While hacking we only write "MIR_Access" for an
              | Attribute Partition if the compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of GROUP is "CREATED"
              |       OR
              |    CM_AUGMENT and the definition status of GROUP is
              |                                                  CREATED.
              |    In other words, we write if mode is NORMAL OR
              |    definition status is CREATED.
              */
              if ( (compiler_mode == CM_NORMAL)
                  || (mirc_get_def_status() == DS_CREATED)
                  ) {
                  TRY( mirc_add_rel_number(MIR_Access,
                                           partition_setable,
                                           MIR_SNUMBER ));
                  }

              /* Restore ourselves to the Entity-Class object */
              mirc_set_current(current);
              }
          /* <ADHACK>: Remove the previous statements */

          /*
          | We've finished with the current partitioned Attribute Partition,
          | make sure we indicate this.
          */
          current_attr_partition = NULL;

	  }
	;

attrib_kind:  	KW_IDENTIFIER 
		{$$ = MCC_K_PRT_IDENTIFIER;
		access_default = MCC_K_NONSETABLE;}
	| 	KW_STATUS 
		{ $$ = MCC_K_PRT_STATUS;
		access_default = MCC_K_NONSETABLE;}
	| 	KW_CHARACTERISTIC 
		{ $$ = MCC_K_PRT_CHAR;
		access_default = MCC_K_SETABLE;}
	|	KW_REFERENCE
		{ $$ = MCC_K_PRT_REFERENCE;
		access_default = MCC_K_SETABLE;}
	| 	KW_COUNTER 
		{ $$ = MCC_K_PRT_COUNTERS;
		access_default = MCC_K_NONSETABLE;}
	| 	KW_INITIAL
		{ $$ = MCC_K_PRT_INITIAL_ATTRS;
		access_default = MCC_K_NONSETABLE;}
	|	KW_STATISTIC
		{ $$ = MCC_K_PRT_STATISTIC;
		access_default = MCC_K_NONSETABLE;}
	;

/* list of one or more attributes belonging to the group */
attrib_def_list: /* empty */
	|	attrib_def_list attribute_definition ;

attribute_definition: 
        KW_ATTRIBUTE object_header
          {
          DEF_STATUS    def_st;         /* Temporary save area */

          ObjectCode = $2;
          TRY(mirc_create_object(MIR_Cont_attribute,
                                 ObjectCode,
                                 CurStrBuf,
                                 &level));
          /*
          | (After a mirc_create_object() call, the current-object
          |  always has a valid definition-status value assigned).
          |
          | Establish this attribute object in a group if a partition
          | attribute group is active (!=NULL, if one isn't, something
          | is seriously wrong).
          |
          | We do the MIR_List_Entry write only if the attribute was
          | actually created on this call (which might not be the
          | case for the Merge/Augment situation).
          */
          if (current_attr_partition != NULL
              && mirc_get_def_status() == DS_CREATED) {

            IDS *saved_attr;          /* Storage for current object */

            saved_attr = mirc_get_current();
            mirc_set_current(current_attr_partition);

            /*
            | Force definition-status on the partition to CREATED
            | temporarily so that this mirc_add_relationship() call
            | will 'take' in case we're doing a Merge or Augment.
            */
            def_st = mirc_get_def_status();    /* Copy . . .         */
            mirc_set_def_status(DS_CREATED);   /* Temporarily change */
            TRY(mirc_add_relationship(MIR_List_Entry, saved_attr));
            mirc_set_def_status(def_st);       /* Restore            */

            mirc_set_current(saved_attr);
            }

          /* <ADHACK>: Remove the following statements */
          predict_value = MCC_K_TRUE;   /* Set dft if no PREDICT= parsed     */
          predict_present = MCC_K_FALSE;/* Show no "PREDICTABLE=" parsed yet */
          access_value = access_default;/* Set dft if no ACCESS= gets parsed */
          access_present = MCC_K_FALSE; /* Show no "ACCESS=" parsed yet      */
          display_value = MCC_K_TRUE;  /* Set dft if no DISPLAY= gets parsed */
          display_present = MCC_K_FALSE; /* Show no "DISPLAY=" parsed yet    */
          dns_ident_present = MCC_K_FALSE; /* Show no "DNS_IDENT=" parsed yet*/
          dft_value_present = MCC_K_FALSE; /* Show no "DEFAULT=" parsed yet  */
          category_present = MCC_K_FALSE; /* Show no "CATEGORIES=" parsed yet*/
          /* <ADHACK>: Remove the previous statements */
          }

        defined_data_type
        op_incl_stmt
        attribute_body
        write_category_def      /* <ADHACK> - Remove this line */
        write_display_def       /* <ADHACK> - Remove this line */

          {
          /* <ADHACK>: Remove this entire IF statement.
          |
          | If we're hacking, decide whether to write the MIR_Predictable out
          */
          if (ad_hack_switch == TRUE) {

              /* "PREDICTABLE="
              | While hacking we only write "MIR_Predictable" if the
              | compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of ATTRIBUTE is "CREATED"
              |       OR
              |    CM_AUGMENT and-> the "PREDICTABLE=" clause was PRESENT
              |               or -> the definition status of ATTRIBUTE is
              |                     CREATED.
              |
              |    Under AUGMENT, mirc_add_relationship() will issue an
              |    error if the definition status of ATTRIBUTE was
              |    PREDEFINED, (you can't modify via AUGMENT a pre-existing
              |    object, so this is right. . . the bummer is that the
              |    line number that will be reported is that of the line just
              |    before the "END ATTRIBUTE" keywords. . .this might be a bit
              |    difficult to interpret since it will be the "PREDICTABLE="
              |    clause causing the problem).
              */
              if ( (compiler_mode == CM_NORMAL)
                  || ((compiler_mode == CM_MERGE)
                      && (mirc_get_def_status() == DS_CREATED))
                  || ((compiler_mode == CM_AUGMENT)
                      && ((predict_present == MCC_K_TRUE)
                          || (mirc_get_def_status() == DS_CREATED)))
                  ) {
                  TRY( mirc_add_rel_number(MIR_Predictable,
                                           predict_value,
                                           MIR_SNUMBER ));
                  }

              /* "ACCESS="
              | (Exactly the same requirements apply to "ACCESS=" as
              |  "PREDICTABLE=" above)
              */              
              if ( (compiler_mode == CM_NORMAL)
                  || ((compiler_mode == CM_MERGE)
                      && (mirc_get_def_status() == DS_CREATED))
                  || ((compiler_mode == CM_AUGMENT)
                      && ((access_present == MCC_K_TRUE)
                          || (mirc_get_def_status() == DS_CREATED)))
                  ) {
                  TRY( mirc_add_rel_number(MIR_Access,
                                           access_value,
                                           MIR_SNUMBER ));
                  }

              /* "DNS_IDENT="
              | While hacking we only write "MIR_DNS_Ident" (with target of
              | "MCC_K_DNS_ID_ALTERNATE") here if
              |
              |    * the the "DNS_IDENT=" clause was *NOT* present,
              |    * the current attribute group is IDENTIFIER,
              |    * the current entity is GLOBAL
              |
              | and compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of ATTRIBUTE is CREATED
              |       OR
              |    CM_AUGMENT and the definition status of ATTRIBUTE is CREATED
              |
              |    In other words (with respect to the compiler mode) either
              |    the compiler mode is CM_NORMAL or the object definition
              |    status was DS_CREATED.
              */
              if (   (dns_ident_present == MCC_K_FALSE)
                  && (CurAttrGroup      == MCC_K_PRT_IDENTIFIER)
                  && (entitylevel       == 1)
                  && ((compiler_mode == CM_NORMAL)
                      || (mirc_get_def_status() == DS_CREATED))
                  ) {
                  TRY( mirc_add_rel_number(MIR_DNS_Ident,
                                           MCC_K_DNS_ID_ALTERNATE,
                                           MIR_SNUMBER ));
                  }

              /* "DEFAULT="
              | While hacking we only write "MIR_valueDefault" (with target of
              | "MCC_K_DEF_NO") here if
              |
              |    * the the "DEFAULT=" clause was *NOT* present,
              |
              | and compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of ATTRIBUTE is CREATED
              |       OR
              |    CM_AUGMENT and the definition status of ATTRIBUTE is CREATED
              |
              |    In other words (with respect to the compiler mode) either
              |    the compiler mode is CM_NORMAL or the object definition
              |    status was DS_CREATED.
              */
              if (   (dft_value_present == MCC_K_FALSE)
                  && ((compiler_mode == CM_NORMAL)
                      || (mirc_get_def_status() == DS_CREATED))
                  ) {
                  TRY( mirc_add_rel_number(MIR_valueDefault,
                                           MCC_K_DEF_NO,
                                           MIR_SNUMBER ));
                  }

              /* See if the Partition should be "setable" */
              if (access_value != MCC_K_NONSETABLE)
                  partition_setable = MCC_K_SETABLE;

              } /* <ADHACK>: Remove this IF statement above. */

          /* Now pop back to entity-class MIR object */
          TRY(mirc_use_parent(&level));
          }

        KW_END KW_ATTRIBUTE optional_name semicolon_or_include
	;

attribute_body:	/* empty */
	|	attribute_property
	|	attribute_body comma_or_include attribute_property
	;

attribute_property:
		predictable
	|	attr_access
	|	display
	|	depends_on
	|       depends_expression
	|	dns_ident
	|	oid_line
	|	units
	|	default_value
	|	include_symbol
	|	categories
	|	private ;


predictable: KW_PREDICTABLE P_EQUAL true_false
	{
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /*     - > TRUE  */
            TRY(mirc_add_rel_number(MIR_Predictable, $3, MIR_SNUMBER));
            }
        else {  /* Do Apply-Default Hack stuff - > FALSE */
            predict_value = $3;  /* Record parsed value for future reference */
            predict_present = MCC_K_TRUE;    /* Signal "PREDICTABLE=" parsed */
            }
	};

attr_access: KW_ACCESS P_EQUAL attr_access_value 
        {
        /* <ADHACK>:
        |  To remove <ADHACK> support, make the TRUE condition of the
        |  following IF statement be the only thing in this action code:
        |  (ie, just do the TRY( ... ) stuff).
        */
        if (ad_hack_switch == FALSE) {  /*     - > TRUE  */
            TRY(mirc_add_rel_number(MIR_Access, $3, MIR_SNUMBER));
            }
        else {  /* Do Apply-Default Hack stuff - > FALSE */
            access_value = $3;  /* Record parsed value for future reference */
            access_present = MCC_K_TRUE;    /* Signal "ACCESS=" parsed      */
            }
        }
        ;

attr_access_value: 
		KW_SETTABLE 
		  { $$ = (int ) MCC_K_SETABLE; }
	| 	KW_NONSETTABLE 
		  { $$ = (int ) MCC_K_NONSETABLE; }
	| 	KW_WRITEONLY 
		  { $$ = (int ) MCC_K_WRITEONLY; }
	;


dns_ident: KW_DNS_IDENT P_EQUAL ID_ANYCASE
	{
        IDS *saved_attr;          /* Storage for current object       */
        int dns_code = 0;         /* Local copy for selected DNS code */

        /*
        | Only attributes that are defined in the IDENTIFIER partition
        | may be specified as DNS identifier.
        */
	if (CurAttrGroup != MCC_K_PRT_IDENTIFIER)
	    {
	    /* msl_error( MCCMSL_S_INVDNSIDENTTYPE ) */
            yyerror(MP(mp831,"DNS IDENT valid only for IDENTIFIER partition attributes"));
	    YYERROR;
	    }

        /*
        | Only Global Entities need to have DNS Primary Identifiers specified.
        */
	if (entitylevel != 1)
	    {
	    /* msl_error( MCCMSL_S_INVDNSLOC ) */
            yyerror(MP(mp832,"DNS Primary Identifiers needed only for Global Entities"));
	    YYERROR;
	    }

        /* <ADHACK>: Remove the following statements */
        dns_ident_present = MCC_K_TRUE;       /* Show "DNS_IDENT=" parsed */
        /* <ADHACK>: Remove the previous statements */

        /* Grab the Keyword phrase */
	GetTokStr( $3 );

	if (strcmp(CurStrBuf,"PRIMARY_NAME") == 0)
	    {
	    if (found_dns_primary_name == MCC_K_TRUE)
		{
		/* msl_error( MCCMSL_S_DUPDNSPNAME ) */
                yyerror(MP(mp833,"Duplicate Primary DNS Name found"));
		YYERROR;
		}
	    dns_code = MCC_K_DNS_ID_PRIMARY;
	    found_dns_primary_name = MCC_K_TRUE;
	    }
	else if (strcmp(CurStrBuf,"ALTERNATE_NAME") == 0)
	    dns_code = MCC_K_DNS_ID_ALTERNATE;
	else if (strcmp(CurStrBuf,"NOT_USED") == 0)
	    dns_code = MCC_K_DNS_ID_NOT_USED;
	else
	    {
	    /* msl_error_1( MCCMSL_S_INVDNSIDTYPE, &curstr_desc ) */
            yyerror(MP(mp834,"Unrecognized DNS Name type"));
	    YYERROR;
	    }

        /*
        | Write out code for kind of DNS ident for this identifier
        */
        TRY(mirc_add_rel_number(MIR_DNS_Ident, dns_code, MIR_SNUMBER));

        /*
        | Lookup Optimization
        |
        | If we just found the Primary Identifier, we 'mark' it in 
        | the MIR Object that represents the Identifier Partition by
        | using the same relationship (MIR_DNS_Ident) in that object
        | to point to the MIR Non-Terminal Object for the Identifier
        | that is the Primary DNS Identifier attribute.  (This saves
        | scanning the Identifiers 'by hand' to find the one that is
        | the DNS Primary Identifier).
        |
        | We only do this write if the current Entity-Class was not already
        | defined.
        */
        if (dns_code == MCC_K_DNS_ID_PRIMARY
            && mirc_get_def_status() != DS_PREDEFINED) {

            /* Sawe where we are */
            saved_attr = mirc_get_current();           

            /* Bop back to Identifier Partition */
            mirc_set_current(current_attr_partition);

            /* Record it */
            TRY(mirc_add_relationship(MIR_DNS_Ident, saved_attr));

            /* Return to attr Object */
            mirc_set_current(saved_attr);
            }
	}
	;

/* ----------------------------------------------------------------------------
|
| rules for directives:
|
|       {DIRECTIVE <directive-name> = <code> :
|               [DEPENDS ON = "<variant-expression>",]
|               DIRECTIVE-TYPE = <EXAMINE> | MODIFY | ACTION>,
|               [DISPLAY = <"TRUE" | "FALSE">,]
|               [SYMBOL = <Latin1String>,]
|               [CATEGORIES = (<[CONFIGURATION] [,FAULT] [,PERFORMANCE] 
|                               [,SECURITY] [,ACCOUNTING]| ALL> ),]
|
|             [REQUEST ...
|             [RESPONSE ...
|             [EXCEPTION . ..
|
|          END DIRECTIVE [<directive-name>];}
*/
direc_def: 
	direc_hdr
	directive_list
          write_display_def  /* <ADHACK> - Remove this line */
          write_category_def /* <ADHACK> - Remove this line */
	req_def
	dir_rsp_exc_list
	direc_end
	;

dir_rsp_exc_list :
	  /* Empty */
	| dir_rsp_exc_list dir_rsp_exc_def
	;

dir_rsp_exc_def :
	  rsp_def
	| exc_def
	;

directive_list:
          /* Empty */
	| directive_list directive_item comma_or_include
	;

directive_item:
	  depends_on
	| depends_expression
	| direc_act_dir
	| display
	| include_symbol
	| oid_line
	| categories
	;

directive_name:  multi_keywd_identifier
	       | KW_SET
                 {
                 strncpy(CurStrBuf, "SET", 4);
                 CurStrBufLen = 3;
                 }
                 ;

direc_hdr:
	KW_DIRECTIVE directive_name P_EQUAL IDcode P_COLON
                {
		ObjectCode = $4;
                TRY(mirc_create_object(MIR_Cont_directive, $4, CurStrBuf,
                                            &level));
                /*
                |  Record the directive IDS address so that the action code
                |  at rule 'req_arg:' can insert a special relationship
                |  "MIR_Cont_request_argument" into this directive object to
                |  point directly at the argument.
                |
                |  This makes certain kinds of lookups much more straight-
                |  forward (compared to 'walking thru' the (essentially empty)
                |  request block).
                */
                directive_IDS = mirc_get_current();

                /*
                | Indicate the fact we're 'into' a DIRECTIVE, and that we
                | expect to see a required DIRECTIVE_TYPE clause in a
                | non-augment compilation.
                */
                dir_type_clause_lineno = yylineno;

                /* <ADHACK>: Remove the following statements */
                /* Set dft if no DISPLAY= gets parsed */
                display_value = MCC_K_TRUE;
                /* Show no "DISPLAY=" parsed yet    */
                display_present = MCC_K_FALSE;
                /* Show no "CATEGORIES=" parsed yet*/
                category_present = MCC_K_FALSE;
                /* <ADHACK>: Remove the previous statements */

                }
           op_incl_stmt
                ;

direc_end: 
        /*
        | As we terminate the definition of the current directive, we're
        | checking here is to be sure the directive has an DIRECTIVE_TYPE=()
        | clause in a non-AUGMENT compilation, or in the case of an AUGMENT
        | compilation where the object was created.
        |
        |  NOTE: The number stored in "dir_type_clause_lineno" is actually
        |        the line number of the DIRECTIVE statement that should have an
        |        DIRECTIVE_TYPE=() clause associated with it.
        */
    KW_END KW_DIRECTIVE optional_name semicolon_or_include
	{
        /* Check for missing DIRECTIVE_TYPE= clause */
        if (dir_type_clause_lineno != 0 &&   /* (Didn't see DIR_TYPE=) *and* */
              ((compiler_mode != CM_AUGMENT) /* (Compiling Normal or Merge   */
                || (                             /* or Augmenting *and* the  */
                    compiler_mode == CM_AUGMENT  /* Object was created by aug*/
                    && mirc_get_def_status() == DS_CREATED
                   )
              )
           ) {
            char    buff[200];

            sprintf(buff,
                    MP(mp850, "Directive defined in line %d has no DIRECTIVE_TYPE=() clause"),
                    dir_type_clause_lineno);
            yyerror(buff);
            }

        TRY(mirc_use_parent(&level));
        }

  |  KW_END KW_DIRECTIVE KW_SET semicolon_or_include
        {
        /* Check for missing DIRECTIVE_TYPE= clause */
        if (dir_type_clause_lineno != 0 &&   /* (Didn't see DIR_TYPE=) *and* */
              ((compiler_mode != CM_AUGMENT) /* (Compiling Normal or Merge   */
                || (                             /* or Augmenting *and* the  */
                    compiler_mode == CM_AUGMENT  /* Object was created by aug*/
                    && mirc_get_def_status() == DS_CREATED
                   )
              )
           ) {
            char    buff[200];

            sprintf(buff,
                    MP(mp850, "Directive defined in line %d has no DIRECTIVE_TYPE=() clause"),
                    dir_type_clause_lineno);
            yyerror(buff);
            }

        TRY(mirc_use_parent(&level));
        }
	;

direc_act_dir: KW_DIRECTIVE_TYPE P_EQUAL directive_type
	{
        TRY(mirc_add_rel_number(MIR_Directive_type, $3, MIR_SNUMBER));

        /*
        |  Indicate we've parsed the required DIRECTIVE_TYPE clause
        */
        dir_type_clause_lineno = 0;
	}

directive_type:	KW_ACTION
		{$$ = MCC_K_DIR_ACTION;}
	|	KW_EXAMINE
		{$$ = MCC_K_DIR_EXAMINE;}
	|	KW_MODIFY
		{$$ = MCC_K_DIR_MODIFY;}
	|	KW_EVENT
		{$$ = MCC_K_DIR_EVENT;}
	;

/* ----------------------------------------------------------------------------
|
| rules for directive REQUESTS:
|
|       [REQUEST
|        {ARGUMENT <argument-name> = <code> : <datatype>
|               [DEPENDS ON = "<variant-expression> ",]
|               [ECHO = <"TRUE" | "FALSE">,]
|               [DISPLAY = <"TRUE" | "FALSE">,]
|               [REQUIRED = <"TRUE" | "FALSE">,]
|               [UNITS = <UnitType>,]
|               [DEFAULT = < <value> | NO_DEFAULT |
|                            IMPLEMENTATION SPECIFIC>,]
|               [SYMBOL = <Latin1String>,]
|         END ARGUMENT [<argument-name> };}
|       END REQUEST ;]
|
| NOTE:
|       A REQUEST is a bit anomalous.  We create a MIR object to represent
|       it, but it has no name nor object code of its own from the MSL.
|
|       The original MCC MSL translator used the name and code of the
|       directive, but that is not implemented here.
|
|       Also, no Object ID is assigned to it, as there is no code yet
|       specified by architecture for use in the arcs for such an OID.
|
|       Consequently, the creation of the request "MIR Object" is *NOT*
|       done by the usual method of calling "mirc_create_object()".  Instead
|       we do it here "by hand" (skipping a lot of the stuff that otherwise
|       gets done).
|
|       Note that "level" is not incremented either:  this is just a shadow
|       object to fill out the notion that the REQUEST is something
|       'contained within' a DIRECTIVE.
|
| MERGE & AUGMENT:
|       We have to duplicate some logic that is contained in
|       "mirc_create_object()" surrounding the issue of detecting whether or
|       not a REQUEST is already PREDEFINED for Merge & Augment mode.
|
|       The logic here presumes that a DIRECTIVE may have at most one REQUEST.
|       We simply try to find a MIR_Cont_request relationship in the DIRECTIVE
|       relationship table.  If we do, we mark it PREDEFINED and skip the
|       creation.  Otherwise, a create takes place and it is marked CREATED.
|       This is exactly what "mirc_create_object()" does.
*/

req_def: /* empty */
	| KW_REQUEST
	{
        IDS        *new=NULL;  /* --> Newly created MIR Non-Terminal object */
        IDS        *cur;       /* --> current MIR Non-Terminal object (dir) */
        MC_STATUS  status;     /* Status of support calls                   */
        DEF_STATUS def_st;     /* Temporary copy of DIR definition-status   */


        /*
        | If we're doing Merge or Augment and Current Object (Directive) is
        | PREDEFINED, try finding the REQUEST (the way mirc_create_object()
        | would).
        */
        if (compiler_mode != CM_NORMAL
            && mirc_get_def_status() == DS_PREDEFINED) {

            int    i;        /* Handy-Dandy Loop index                      */

            cur = mirc_get_current();
            for (i = 0; i < cur->idsu.nt.entry_count; i++) {

                /* If we have a match . . . */
                if (cur->idsu.nt.rel_table[i].rel_obj->idsu.nt.ntu.synonym
                    == MIR_Cont_request) {

                    /* We found the Request, make it the current-object */
                    new=cur->idsu.nt.rel_table[i].tar_obj;
                    new->idsu.nt.parent_obj = mirc_get_current();
                    mirc_set_current(new);

                    /* Mark it PREDEFINED */
                    mirc_set_def_status(DS_PREDEFINED);
                    break;
                    }
                }
            }

        /*
        | If there is no predefined Request, make a new one
        */
        if (new == NULL) {

            /* if (attempt to create a non-terminal object failed) */
            if ( (new = I_Create_IDS(I_NT_OBJECT, IDS_DFT_TABLESIZE)) == NULL){
                TRY(MC_OUT_OF_MEMORY);
                }

            /*
            |  Make "current_object" (directive) "contain" the MIR object for
            |  REQUEST.
            |
            |  This "mirc_add_relationship()" call isn't going to succeed
            |  unless the definition-status of the DIRECTIVE (current-object)
            |  is CREATED.  We force just for the moment.
            */
            def_st = mirc_get_def_status();
            mirc_set_def_status(DS_CREATED);

            /* if (attempt to add specified rel. to current object failed) */
            if ((status = mirc_add_relationship(MIR_Cont_request, new))
                != MC_SUCCESS) {
                TRY(status);
                }
            mirc_set_def_status(def_st);        /* Restore */

            /*
            |  set IDS parentpointer (of newly created object) to point at
            |  current obj
            */
            new->idsu.nt.parent_obj = mirc_get_current();

            /* make newly created object be the "current" object */
            mirc_set_current(new);
            mirc_set_def_status(DS_CREATED);    /* Brand-new REQUEST */

            /*
            | Make the "current_object" (REQUEST) "be contained by" the MIR
            | object for DIRECTIVE (it's parent, from the parser's point of
            | view).
            |
            | if (attempt to add specified rel. to current object failed) */
            if ((status = mirc_add_relationship(MIR_Contained_By,
                                                new->idsu.nt.parent_obj))
                    != MC_SUCCESS){
                TRY(status);
                }
            }

        /*
        | Record the object type that might contain an argument so that
        | the backend function that computes the OID for the argument can
        | tell what arc number to include in the OID when the argument
        | object is created and its OID is computed and registered in the
        | index.
        */
        containing_arg_type = MIR_Cont_request;

	}
     op_incl_stmt req_args KW_END KW_REQUEST semicolon_or_include
	{
        /* NOTE:
        |  We don't change the value of "level" here, as it wasn't changed
        |  when we created the REQUEST object in the first place (above).
        */
	TRY(mirc_use_parent(NULL));
	}
        ;

req_args: 	/* empty */
	|	req_args req_arg  ;

req_arg:
	KW_ARGUMENT object_header
          {
          IDS        *request_arg_IDS; /* -> Object created here             */
          DEF_STATUS def_st;           /* Temp copy of DIR definition-status */


          ObjectCode = $2;
          TRY(mirc_create_object(MIR_Cont_argument, ObjectCode,
                                 CurStrBuf, &level));

          /*
          |  Now insert the special "MIR_Cont_request_argument" rel.
          |  to point directly to this newly created argument object
          |  from the directive object.
          |
          |  MERGE & AUGMENT:
          |    Only if we actually created the ARGUMENT object do we
          |    back up and insert this special relationship in the
          |    directive.
          */
          if (mirc_get_def_status() == DS_CREATED) {

              /* Get argument object */
              request_arg_IDS = mirc_get_current();

              /* Switch to directive object */
              mirc_set_current(directive_IDS);

              /*
              |  Force the insert by temporarily making any PREDEFINED
              |  status of the DIRECTIVE object be "CREATED",
              |  (this for Merge/Augment purposes:
              |              fakes out "mirc_add_relationship()")
              */
              def_st = mirc_get_def_status();
              mirc_set_def_status(DS_CREATED);

              TRY(mirc_add_relationship(MIR_Cont_request_argument,
                                        request_arg_IDS));
              mirc_set_def_status(def_st);          /* Restore            */

              mirc_set_current(request_arg_IDS);    /* Return to req arg  */
              }

          /* <ADHACK>: Remove the following statements */
          echo_value = MCC_K_TRUE;       /* Establish dft if no ECHO= parsed */
          echo_present = MCC_K_FALSE;    /* Show no "ECHO=" parsed yet       */
          required_value = MCC_K_FALSE;  /* Establish dft if no REQD= parsed */
          required_present = MCC_K_FALSE;/* Show no "REQUIRED=" parsed yet   */
          dft_value_present = MCC_K_FALSE; /* Show no "DEFAULT=" parsed yet  */
          display_value = MCC_K_TRUE;  /* Set dft if no DISPLAY= gets parsed */
          display_present = MCC_K_FALSE; /* Show no "DISPLAY=" parsed yet    */
          /* <ADHACK>: Remove the previous statements */
          }

	defined_data_type op_incl_stmt
	req_arg_property_list
	KW_END KW_ARGUMENT optional_name semicolon_or_include
        write_display_def  /* <ADHACK> - Remove this line */
          {
          /* <ADHACK>: Remove this entire IF statement.
          |
          | If we're hacking, decide whether to write the MIR_Echo out
          */
          if (ad_hack_switch == TRUE) {

              /* "ECHO="
              | While hacking we only write "MIR_Echo" if the
              | compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of ARGUMENT is "CREATED"
              |       OR
              |    CM_AUGMENT and-> the "ECHO=" clause was PRESENT
              |               or -> the definition status of ARGUMENT is
              |                     CREATED.
              |
              |    Under AUGMENT, mirc_add_relationship() will issue an
              |    error if the definition status of ARGUMENT was
              |    PREDEFINED, (you can't modify via AUGMENT a pre-existing
              |    object, so this is right. . . the bummer is that the
              |    line number that will be reported is that of the
              |    "END ARGUMENT" keywords. . .this might be a bit
              |    difficult to interpret since it will be the "ECHO="
              |    clause causing the problem).
              */
              if ( (compiler_mode == CM_NORMAL)
                  || ((compiler_mode == CM_MERGE)
                      && (mirc_get_def_status() == DS_CREATED))
                  || ((compiler_mode == CM_AUGMENT)
                      && ((echo_present == MCC_K_TRUE)
                          || (mirc_get_def_status() == DS_CREATED)))
                  ) {
                  TRY( mirc_add_rel_number(MIR_Echo,
                                           echo_value,
                                           MIR_SNUMBER ));
                  }

              /* "REQUIRED="
              | (Exactly the same requirements apply to "REQUIRED=" as
              |  "ECHO=" above)
              */
              if ( (compiler_mode == CM_NORMAL)
                  || ((compiler_mode == CM_MERGE)
                      && (mirc_get_def_status() == DS_CREATED))
                  || ((compiler_mode == CM_AUGMENT)
                      && ((required_present == MCC_K_TRUE)
                          || (mirc_get_def_status() == DS_CREATED)))
                  ) {
                  TRY( mirc_add_rel_number(MIR_Required,
                                           required_value,
                                           MIR_SNUMBER ));
                  }                     

              /* "DEFAULT="
              | While hacking we only write "MIR_valueDefault" (with
              | target of "MCC_K_DEF_NO") here if
              |
              |    * the the "DEFAULT=" clause was *NOT* present,
              |
              | and compiler mode is:
              |
              |    CM_NORMAL
              |       OR
              |    CM_MERGE and definition status of ATTRIBUTE is CREATED
              |       OR
              |    CM_AUGMENT and the definition status of ATTRIBUTE
              |               is CREATED
              |
              |    In other words (with respect to the compiler mode)
              |    either the compiler mode is CM_NORMAL or the object
              |    definition status was DS_CREATED.
              */
              if (   (dft_value_present == MCC_K_FALSE)
                  && ((compiler_mode == CM_NORMAL)
                      || (mirc_get_def_status() == DS_CREATED))
                  ) {
                  TRY( mirc_add_rel_number(MIR_valueDefault,
                                           MCC_K_DEF_NO,
                                           MIR_SNUMBER ));
                  }

              } /* <ADHACK>: Remove this IF statement above. */


          mirc_use_parent(&level); 
          } /* request argument definition complete */
       ;

/* list of zero or more request argument descriptors with separators */
req_arg_property_list:
		/* empty */
	|	req_arg_property /* if only one */
	|	req_arg_property_list comma_or_include req_arg_property ;

req_arg_property:
		echo
	|	display	
	|	oid_line
	|	required
	|	depends_on
	|	depends_expression
	|	units
	|	default_value
	|	include_symbol
	;

/* ----------------------------------------------------------------------------
|
| rules for directive RESPONSES:
|
|       [RESPONSE
|        [SYMBOL = <Latin1String>,]
|        [TEXT = <"Optional standard output text for the response">']
|        {ARGUMENT <argument-name> = <code> : <datatype>
|               [DEPENDS ON = "<variant-expression> ",]
|               [DISPLAY = <"TRUE" | "FALSE">,]
|               [UNITS = <UnitType>,]
|               [SYMBOL = <Latin1String>,]
|         END ARGUMENT [<argument-name> };}
|       END RESPONSE ;]
|
*/

rsp_def: rsp_beg rsp_properties optional_separator rsp_exc_evt_args rsp_end
	{
        TRY(mirc_use_parent(&level));  /* now that response is complete */
        }
	;

rsp_beg: KW_RESPONSE object_header
		{
		ObjectCode = $2;
                TRY(mirc_create_object(MIR_Cont_response,
                                       ObjectCode,
                                       CurStrBuf,
                                       &level));
                /*
                | Record the object type that might contain an argument so that
                | the backend function that computes the OID for the argument
                | can tell what arc number to include in the OID when the
                | argument object is created and its OID is computed and
                | registered in the index.
                */
                containing_arg_type = MIR_Cont_response;
		}
	op_incl_stmt
	;

rsp_end: KW_END KW_RESPONSE optional_name semicolon_or_include;

rsp_properties: /* empty */
	| rsp_property /* if only one */
	| rsp_properties comma_or_include rsp_property;
	;

rsp_property:
		include_symbol
	|	oid_line
	|	rsp_txt
	;

rsp_txt:	KW_TEXT P_EQUAL ID_QUOTED
		  {
                  GetTokStrQuoted( $3 );
                  TRY(mirc_add_rel_string(MIR_Description,
                                          filter_string(CurStrBuf)));
		  };

/* list of zero or more response argument descriptors */
rsp_exc_evt_arg_prop_list:	/* empty */
	|	rsp_arg_property	/* if only one */
	|	rsp_exc_evt_arg_prop_list comma_or_include rsp_arg_property ;

rsp_arg_property:
		display
	|	depends_on
	|	depends_expression
	|	oid_line
	|	units
	|	include_symbol
        ;

/* ----------------------------------------------------------------------------
|
| rules for directive EXCEPTIONS:
|
|       [EXCEPTION <exception-name> = <code> :
|        [SYMBOL = <Latin1String>,]
|        TEXT = <"Optional standard output text for the response">'
|        {ARGUMENT <argument-name> = <code> : <datatype>
|               [DEPENDS ON = "<variant-expression> ",]
|               [DISPLAY = <"TRUE" | "FALSE">,]
|               [UNITS = <UnitType>,]
|               [SYMBOL = <Latin1String>,]
|         END ARGUMENT [<argument-name> };}
|       END RESPONSE ;]
|    END DIRECTIVE
*/

exc_def:  exc_hdr rsp_properties optional_separator rsp_exc_evt_args exc_end
	{
        TRY(mirc_use_parent(&level));
        }
	;

exc_hdr: 	KW_EXCEPTION object_header
	{
	ObjectCode = $2;
        TRY(mirc_create_object(MIR_Cont_exception, ObjectCode, CurStrBuf, &level));

        /*
        | Record the object type that might contain an argument so that
        | the backend function that computes the OID for the argument
        | can tell what arc number to include in the OID when the
        | argument object is created and its OID is computed and
        | registered in the index.
        */
        containing_arg_type = MIR_Cont_exception;
	}
	op_incl_stmt
	;

exc_end: 	KW_END KW_EXCEPTION optional_name semicolon_or_include
	;

/* if ARGUMENT keyword then one or more arguments expected */
rsp_exc_evt_args: 	/* empty */
	|	rsp_exc_evt_args rsp_exc_evt_arg 
	;

rsp_exc_evt_arg: KW_ARGUMENT object_header
           {
           ObjectCode = $2;
           TRY(mirc_create_object(MIR_Cont_argument,
                               ObjectCode,
                               CurStrBuf,
                               &level));

            /* <ADHACK>: Remove the following statements */
            display_value = MCC_K_TRUE;/* Set dft if no DISPLAY= gets parsed */
            display_present = MCC_K_FALSE; /* Show no "DISPLAY=" parsed yet  */
            /* <ADHACK>: Remove the previous statements */
            }

	defined_data_type op_incl_stmt
	rsp_exc_evt_arg_prop_list
        KW_END KW_ARGUMENT optional_name semicolon_or_include
        write_display_def  /* <ADHACK> - Remove this line */

	    { 
            TRY( mirc_use_parent(&level));
            }
            ;

event_defs: KW_EVENT event_def

event_def: event_part
	| event_group
	;

/* ----------------------------------------------------------------------------
|
| rules for EVENT GROUP
|
|     {EVENT GROUP <event-group-name> = <code> :
|        [SYMBOL = <Latin1String>,]
|        [CATEGORIES = (<[CONFIGURATION] [,FAULT] [,PERFORMANCE] 
|                       [,SECURITY] [,ACCOUNTING]| ALL> ),]
|          {EVENT LIST = (<event-name> {, <event-name> } )
|      END EVENT GROUP [<event-group-name>] ; }
|
*/

event_group: KW_GROUP object_header
	{
	ObjectCode = $2;
        TRY(mirc_create_object(MIR_Cont_eventGroup,
                               ObjectCode,
                               CurStrBuf,
                               &level));

        eventgrpcount = 0; /* Show no Events found in group yet */

        /*
        | If the mirc_create_object() call actually created a new
        | object for the Event Group (list), then we store the
        | list-type into it.  ("mirc_create_object()" always sets
        | the definition status)
        */
        if (mirc_get_def_status() == DS_CREATED) {
            TRY( mirc_add_rel_number(MIR_List_Type,
                                         LT_EVENT_GROUP,
                                         MIR_SNUMBER ));
            }

        /* <ADHACK>: Remove the following statements */
        category_present = MCC_K_FALSE; /* Show no "CATEGORIES=" parsed yet*/
        /* <ADHACK>: Remove the previous statements */
	}
    op_incl_stmt event_group_list
    write_category_def                   /* <ADHACK> - Remove this line */
    KW_END KW_EVENT KW_GROUP optional_name semicolon_or_include
	{
        if (eventgrpcount == 0) {
            yyerror(MP(mp835,"No events specified for event group"));
            YYERROR;
            }
	TRY( mirc_use_parent(&level));
	}
	;

event_group_list:
	  event_group_item
	| event_group_list comma_or_include event_group_item
	;

event_group_item:
	  include_symbol
	| categories
	| event_names
	| oid_line
	;

event_names: KW_EVENT_LIST P_EQUAL P_LPAREN event_name_list P_RPAREN
	;

event_name_list: event_name
	| event_name_list P_COMMA event_name
	;

event_name: multi_keywd_identifier
	{
        IDS *egrp;      /* Current Event Group */
        IDS *event;     /* Event being sought  */

        /* The current object is the event group
        |
        | We must "step back" to it's parent (the entity itself) and
        |  search for an event given the "name" just-now recognized.
        |
        | NOTE: With the way Merge mode is implemented now, you can't
        |       "merge-in" the name of an event (whether it was newly
        |       created by being mentioned in the Merge file or not) if
        |       the Group already existed.
        |
        |       It wouldn't be too hard to allow Merging an event
        |       that actually was merged (created) by the Merge file (into
        |       an already-existing group). It would require forcing the
        |       definition-status on the Group object itself to CREATED,
        |       and checking the definition-status of the attribute that is
        |       found.  (We could leave the forced status of CREATED on the
        |       Group, as all minor definitions would have already been
        |       parsed).
        |
        |       Under Augment, currently you can only Augment-in a mention
        |       of an event if the Group itself didn't exist until being
        |       mentioned in the Augment file.  This too could be changed.
        */
        egrp = mirc_get_current();    /* Remember where we are */
        mirc_use_parent(NULL);        /* Step back momentarily */
        if ((event = mirc_find_obj_IDS(MIR_Cont_event, CurStrBuf))
            != NULL) {
            mirc_set_current(egrp);   /* Go back to event group */
            TRY(mirc_add_relationship(MIR_List_Entry, event));
            eventgrpcount += 1;       /* Count an event entered to group */
            }
        else {
            char        msg[200];       /* Message Buffer */
            sprintf(msg, MP(mp836,"Event name '%s' not defined."), CurStrBuf);
            yyerror(msg);
            YYERROR;
            }
	}
	;

/* ----------------------------------------------------------------------------
|
| rules for EVENT PARTITION
|
|     {EVENT PARTITION <event-partitionn-name> = <code> :
|        [SYMBOL = <Latin1String>,]
|        {EVENT <event-name> = <code> :
|            [DISPLAY = <"TRUE" | "FALSE">,]
|            [<COUNTED AS = ( <counter-attribute-name>
|                   {, <counter-attribute-name>} ) >,]
|            [SYMBOL = <Latin1String>,]
|            [TEXT = <"Optional standard output text for the event">,]
|            [CATEGORIES = (<[CONFIGURATION] [,FAULT] [,PERFORMANCE] 
|                           [,SECURITY] [,ACCOUNTING]| ALL> ),]
|            {ARGUMENT <argument-name> = <code> : <datatype>
|               [DEPENDS ON = "<variant-expression> ",]
|               [DISPLAY = <"TRUE" | "FALSE">,]
|               [UNITS = <UnitType>,]
|               [SYMBOL = <Latin1String>,]
|             END ARGUMENT [<argument-name> };}
|         END EVENT [<event-name>] ; }
|      END EVENT PARTITION[<event-partition-name>];}
|
| NOTE:
|       The current scheme has MIR_Cont_event relationships in the
|       entity-class MIR Object pointing to the event MIR Objects as well as
|       MIR_Cont_eventPartition relationships in the entity-class MIR OBject 
|       pointing to objects that represent the event partition.  These event
|       partition objects point via MIR_List_Entry relationships at the events
|       objects that belong in that partition.
*/

event_part: ev_part_header
            event_part_list
            use_parent_obj
            event_decl_list
	    ev_part_end ;

ev_part_header: KW_PARTITION object_header
	{

        /*
        | Here we look up the MIR Object that represents the Counter
        | Attribute Partition.  It is this partition MIR Object that the
        | 'counter_attribute:' rule must search for a reference to a
        | counter attribute when processing  events.
        |
        | Yes, we repeatedly do the same thing over and over for each
        | Event Partition, but the burden is relatively light.  The
        | current object is the entity class object already.
        |
        | 'counter_attr_partition' is NULL only if we couldn't find the
        | partition object.  We'll complain about this at the point where
        | we actually try to use it.
        */
        counter_attr_partition =
                mirc_find_obj_IDS(MIR_Cont_attrPartition,
                                  AttrPartString[MCC_K_PRT_COUNTERS]
                                  );

	ObjectCode = $2;
        TRY(mirc_create_object(MIR_Cont_eventPartition,
                               ObjectCode,
                               CurStrBuf,
                               &level));

        /*
        | If the mirc_create_object() call actually created a new
        | object for the Event Partition (list), then we store the
        | list-type into it.  ("mirc_create_object()" always sets
        | the definition status)
        */
        if (mirc_get_def_status() == DS_CREATED) {
            TRY( mirc_add_rel_number(MIR_List_Type,
                                         LT_EVENT_PART,
                                         MIR_SNUMBER ));
            }

        /*
        |  Stash into a parser-local cell the IDS pointer to this partition
        |  so that at event-object creation time (@ 'event_decl:') a
        |  MIR_List_Entry relationship can also be inserted into this
        |  partition object.
        */
        current_evt_partition = mirc_get_current();
	}
	op_incl_stmt
	;

event_part_list: /* empty */
	| event_part_list event_part_item comma_or_include
	;

event_part_item:
	  include_symbol
	| oid_line
	;

use_parent_obj: /* empty */
        /*
        |  We pop back up to the Entity-Class Object at this point before
        |  we go on to parse the Events themselves so that the Events have
        |  the Entity Class object as their 'natural' parent.
        */
	{
        TRY( mirc_use_parent(&level));
        };

event_decl_list: /* empty */
	| event_decl_list event_decl
	;

ev_part_end: KW_END KW_EVENT KW_PARTITION optional_name semicolon_or_include ;

event_decl:
	KW_EVENT object_header
	  {
          IDS        *event;         /* --> This Event MIR Object          */
          DEF_STATUS def_st;         /* Temp copy of DIR definition-status */


	  ObjectCode = $2;
          TRY(mirc_create_object(MIR_Cont_event,
                               ObjectCode,
                               CurStrBuf,
                               &level));
          /*
          | (After a mirc_create_object() call, the current-object
          |  always has a valid definition-status value assigned).
          |
          | Reach over to the Event Partition object and stuff a
          | MIR_List_Entry relationship in it that points at this Event,
          | then pop back to this Object *IF* the event object was actually
          | created above (Merge/Augment)
          */
          if (mirc_get_def_status() == DS_CREATED) {

              /* Save 'here' */
              event = mirc_get_current();

              /* Go to Event Partition Object */
              mirc_set_current(current_evt_partition);

              /*
              |  Force the insert by temporarily making any PREDEFINED
              |  status of the EVENT PARTITION object be "CREATED",
              |  (this for Merge/Augment purposes).
              */
              def_st = mirc_get_def_status();
              mirc_set_def_status(DS_CREATED);

              /* Do the write */
              TRY(mirc_add_relationship(MIR_List_Entry, event));
              mirc_set_def_status(def_st);          /* Restore            */

              /* Return to 'here' */
              mirc_set_current(event);
              }

          /*
          | Record the object type that might contain an argument so that
          | the backend function that computes the OID for the argument
          | can tell what arc number to include in the OID when the
          | argument object is created and its OID is computed and
          | registered in the index.
          */
          containing_arg_type = MIR_Cont_event;

          /* <ADHACK>: Remove the following statements */
          display_value = MCC_K_TRUE;/* Set dft if no DISPLAY= gets parsed */
          display_present = MCC_K_FALSE; /* Show no "DISPLAY=" parsed yet  */
          category_present = MCC_K_FALSE; /* Show no "CATEGORIES=" parsed yet*/
          /* <ADHACK>: Remove the previous statements */
	  }

     op_incl_stmt
     event_prop_list
     write_category_def /* <ADHACK> - Remove this line */
     write_display_def  /* <ADHACK> - Remove this line */
     rsp_exc_evt_args
     KW_END KW_EVENT  optional_name  semicolon_or_include
	{
	TRY( mirc_use_parent(&level));
	}
	;

event_prop_list: /* empty */
	| event_property
	| event_property comma_or_include event_prop_list
	;

event_property:
	  depends_on
	| depends_expression
	| display
	| event_counted_as
	| rsp_property /* include or oid_line or response_text */
	| categories
	;

event_counted_as:
	KW_COUNTED KW_AS P_EQUAL P_LPAREN counter_list P_RPAREN ;

counter_list:
	  counter_attribute
	| counter_list P_COMMA counter_attribute
	;

counter_attribute:  multi_keywd_identifier
	{
        IDS     *counter;   /* The Counter Attribute they specified */
        IDS     *event;     /* Current Event object (save area)     */

        /*
        | We scan the Counter Attribute Partition via mirc_find_obj_IDS()
        | for an entry that points to a (counter) attribute that has the
        | same name as we just recognized in the COUNTED AS list. . .
        |
        | We're bitterly upset at this point if there is no Counter
        | Attribute partition object.
        */
        if (counter_attr_partition == NULL) {
            yyerror(MP(mp837,"No Counter Attributes defined"));
            YYERROR;
            }

        /* Remember the event we're in now */
        event = mirc_get_current();

        /* Go to the Counter Attribute partition */
        mirc_set_current(counter_attr_partition);

        /* If we did in fact find the specified Counter Attribute . . . */
        if ((counter = mirc_find_obj_IDS(MIR_List_Entry, CurStrBuf)) != NULL) {

            /* Return to the Event Object */
            mirc_set_current(event);

            /* Install a relationship that points to the counter */
            TRY(mirc_add_relationship(MIR_Counted_As, counter));
            }
        else {  /* Failed to find the specified Counter Attribute */
            yyerror(MP(mp851,"Specified Counter Attribute is not defined"));
            YYERROR;
            }
	}
	;

%%
/* end of YACC rules */

/*
|
|  This function endeavors to convert a character string that should
|  evaluate to a number to a signed number (first) or unsigned number
|  (if too big to be a signed number).  An indication as to which kind
|  of number is being returned is set into "integer_kind".
| 
|  If any conversion fails on a non-digit character, a warning is issued and a
|  zero returned.
|
|  FOR NON-ALPHA MACHINES:
|
|  If conversion to a signed number fails out-of-bounds HIGH, an attempt
|  is made to convert to an unsigned number.  If that fails HIGH, the
|  maximum unsigned (long on MIPS/VAX) int is returned with an indication that
|  the number being returned is 'unsigned' and a warning is issued.
|
|  If conversion to a signed number fails out-of-bounds LOW, the value LONG_MIN
|  is returned 'signed' and a warning is issued.
|
|  FOR ALPHA:
|
|  Unlike the above, the alpha cannot use strtol() and strtoul() to determine
|  the boundaries since both these functions refer to 64-bit quantities.
|  Our goal is to determine whether the number fits in a 32 bit unsigned int
|  or a 32 bit signed int.
|
|  First convert the string with strtol() to a 64-bit number.  If this fails,
|  then it is definitely out-of-bounds.  Also check for illegal characters.
|
|  Else, compare the 64-bit long against INT_MAX, INT_MIN, and UINT_MAX to
|  determine whether it is still out of bounds, or if it can fit in an
|  unsigned or signed 32-bit int.  If the number is outside the 
|  range [UINT_MAX,INT_MIN], set it to the limit and issue a warning,
|  like above.
*/

int convert_as_needed( text, integer_kind )

char            *text;          /* --> to where to start the conversion     */
mir_value_type  *integer_kind;  /* Where to return "signed" vs "unsigned"   */

{
long     r_svalue;      /* Signed Return value we're gonna actually return   */
unsigned long r_uvalue; /* Unsigned Return value if too big for signed       */
char    buff[100];      /* Msg  buffer where we build an accurate error msg  */
char   *end_text=NULL;  /* Ending value of text pointer after conversion     */


/* Attempt the conversion to signed */
r_svalue = strtol(text, &end_text, 10);

/* Assume we'll return a signed number */
*integer_kind = MIR_SNUMBER;

/* If it blows on a bad character . . . */
if (  ((r_svalue == 0) && (end_text == text))
    || ((end_text != NULL) && (*end_text != '\0'))
   ) { int c;
    c = *end_text;
    sprintf(buff,
            MP(mp838,"Numeric Conversion failure on character \"%c\", zero used"),
            c);
    warn(buff);                         /* Tell 'em             */
    r_svalue = 0;                       /* Force a 0 for sure   */
    return(r_svalue);                   /* Return it            */
    }

/* if it blows because it's out of signed integer conversion bounds */
if ( errno == ERANGE) {

    /* if it blew signed-HIGH . . . */
    if (r_svalue == LONG_MAX) {
#if (defined(alpha) || defined(__alpha))
        /* If it overflows on the alpha 64-bit signed long, then we're
	   definitely not going to fit in 32-bits. We have an overflow! */
        *integer_kind = MIR_UNUMBER;
	r_uvalue=UINT_MAX;
        sprintf(buff,
                MP(mp839,"Numeric Overflow, %ld used instead."),r_uvalue);
        warn(buff);                  /* Tell 'em               */
        return(r_uvalue);    /* Return it              */
        }
#else
        /* Try the conversion again, but this time to unsigned */
        end_text = NULL;
        errno = 0;
        r_uvalue = strtoul(text, &end_text, 10);

        /* We'll be returning something unsigned */
        *integer_kind = MIR_UNUMBER;

        /* if we blow AGAIN on an overflow (its all over) */
        if (errno == ERANGE && r_uvalue == ULONG_MAX) {
            sprintf(buff,
                    MP(mp839,"Numeric Overflow, %ld used instead."),r_uvalue);
            warn(buff);                  /* Tell 'em               */
            return( (long) r_uvalue);    /* Return it              */
            }

        /* if unsigned convert failed for some other reason */
        if (  ((r_uvalue == 0) && (end_text == text))
            || ((end_text != NULL) && (*end_text != '\0'))
           ) { int c;
            c = *end_text;
            sprintf(buff,
                    MP(mp838,"Numeric Conversion failure on character \"%c\", zero used"),
                    c);
            warn(buff);                         /* Tell 'em             */
            r_svalue = 0;                       /* Force a 0 for sure   */
            *integer_kind = MIR_SNUMBER;        /* Return a 'signed' 0  */
            return(r_svalue);                   /* Return it            */
            }

        /* Unsigned Conversion appears OK, return it */
        return((long) r_uvalue);
        }
#endif

    /* if it blew signed-LOW . . . */
    else if (r_svalue == LONG_MIN) {
        sprintf(buff,MP(mp841,"Numeric Underflow, %ld used instead."), r_svalue);
        warn(buff);
        return(r_svalue);
        }

    /* ERANGE occurred, but some value other than "hi" or "lo" came back */
    else {
        warn(MP(mp842,"Numeric ERANGE condition occurred"));
        return(r_svalue);
        }
    }

#if (defined(alpha) || defined(__alpha))
/* NOTE: this method is still kind of a hack.  It is makes use of the
   strtol() call above, and assumes that the result is a 64-bit signed long
   (it won't work on an architecture where sizeof int == sizeof long)
   That result is then compared against UINT_MAX, INT_MAX, and INT_MIN
   to determine the proper "integer_kind".

   For a portable version, there should really be no references to longs.
   Also note that below there is some suspicious looking promotions from
   int to long (where there are < and > comparisons).  These DO behave
   properly on the alpha.
*/

if (r_svalue >= 0L)
{
  if (r_svalue <= INT_MAX) *integer_kind = MIR_SNUMBER;
  else *integer_kind = MIR_UNUMBER;

  if (r_svalue > UINT_MAX)
    {
      r_svalue = UINT_MAX;
      sprintf(buff, MP(mp839,"Numeric Overflow, %ld used instead."),r_svalue);
      warn(buff);                  /* Tell 'em               */
    }
}
else if (r_svalue < INT_MIN)
  {
    r_svalue = INT_MIN;
    sprintf(buff,MP(mp841,"Numeric Underflow, %ld used instead."), r_svalue);
    warn(buff);
  }
#endif

/* There was no apparent signed conversion failure, return it */
return(r_svalue);

}


/*
 * read in the current token string from the lexical analyzer
 */
void GetTokStr( StrLen )
int StrLen;
{
CurStrBufLen = StrLen;
memcpy( CurStrBuf, yytext, CurStrBufLen );
CurStrBuf[StrLen] = 0;
}

/*
 * read in a quoted token string from the lexical analyzer
 */
void GetTokStrQuoted( StrLen )
int StrLen;
{
  /* skip leading quotation mark */
CurStrBufLen = StrLen - 1;
memcpy( CurStrBuf, &yytext[1], CurStrBufLen );
CurStrBuf[CurStrBufLen] = 0;
}

/* Compress Spaces, <CR> and <TAB> to one space (<SP>)
|
| Used to process the 'DEFAULT=' and 'TEXT=' values.
|
| (In the original MCC version, it was also used to reduce the
|  length of images of DEPENDS ON clauses under certain circumstances
|  that no longer apply).
*/
char *filter_string(in_string)
char *in_string;
{
char *d_ch, *s_ch;

for (d_ch = s_ch = in_string; *s_ch; s_ch++,d_ch++)
    {
    if (strchr(filter, *s_ch))
	{
	*d_ch = ' ';
	d_ch++;
	while (strchr(filter, *s_ch) && *s_ch)
	    s_ch++;
	}
    *d_ch = *s_ch;
    }

*d_ch = 0;
if (in_string == CurStrBuf)
    CurStrBufLen = strlen(in_string);
return in_string;
}


/*
| Remove all spaces from a string
|
|  In the original MCC version, this was used to string the spaces
|  from the Entity Class ('parent') names.  It is not clear that it
|  is needed anymore.
*/
#if 0
char *remove_spaces(in_string)
char *in_string;
{
char *d_ch, *s_ch;

for (d_ch = s_ch = in_string; *s_ch; s_ch++,d_ch++)
    {
    while (strchr(" ", *s_ch) && *s_ch)
	s_ch++;
    *d_ch = *s_ch;
    }

*d_ch = 0;
if (in_string == CurStrBuf)
    CurStrBufLen = strlen(in_string);
return in_string;
}
#endif
