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
 * @(#)$RCSfile: mtu_yacc.y,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:19:10 $
 */
%{
  /* Title:	mtu_yacc.y -- MTU Grammer Rules */
  /*
   **
   **  Copyright (c) Digital Equipment Corporation, 1991, 1992
   **  All Rights Reserved.  Unpublished rights reserved
   **  under the copyright laws of the United States.
   **  
   **  The software contained on this media is proprietary
   **  to and embodies the confidential technology of 
   **  Digital Equipment Corporation.  Possession, use,
   **  duplication or dissemination of the software and
   **  media is authorized only pursuant to a valid written
   **  license from Digital Equipment Corporation.
   **
   **  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
   **  disclosure by the U.S. Government is subject to
   **  restrictions as set forth in Subparagraph (c)(1)(ii)
   **  of DFARS 252.227-7013, or in FAR 52.227-19, as
   **  applicable.
   **
   */

/*
 **++
 **  FACILITY:  PolyCenter Common Agent & DECmcc Framework -- MIB Translation Utility
 **
 **  MODULE DESCRIPTION:
 **
 **	MTU Grammar rules
 **
 **      YACC Reference information:
 **      ULTRIX-32 Supplementary Documents - Programmer 
 **	 `YACC - Yet Another Compiler Compiler' by Stephen C. Johnson
 **
 **  AUTHORS:
 **
 **      Rahul Bose
 **
 **  CREATION DATE:  1-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 ** 01	   01-Mar-1991	 Rahul Bose		    Created.
 **
 ** 02     06-Aug-1992   Pete Burgess	Common Agent modifications
 **					
 ** 03     14-Sep-1992   Pete Burgess	CA FT2 modifications:
 **                                      o c89 -std compliance, I18n, simplification of yacc rules,
 **					 o enterprise traps, DECmcc helpfile generation
 */
  
/*
 ** Header files
 */
#include <setjmp.h>
#include <stdio.h>

#if !defined(sun) && !defined(sparc)
#include <stdlib.h>
#endif

#include <string.h>
#include "mtu.h"
#include "mtu_msg.h"
#include "mtu_yacc.h"
  
#define YYMAXDEPTH 2000
  
#define STORE_STRING_FIELD(v, f) strcpy(node->f, v)

#define STORE_INT_FIELD(v, f) node->f = v
  
#if YYDEBUG
static void debug_action PROTOTYPE ((char * s));
#endif
  
#ifdef YYDEBUG 
# define DEBUG_ACTION(s) if (yydebug) debug_action(s); 
#else
# define DEBUG_ACTION(s)
#endif

/*
 * External declarations
 */
extern int yychar;					   /* YACC/LEX global variables */
extern int yylineno;
extern int yyleng;
extern char yytext[];
extern FILE *yyout;  
extern T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; /* Read Parsed command line */
extern char module_name[SIZE_OF_IDENTIFIER];		  /* Store RFC "module name" */

/*
 ** External functions
 */
extern int yylex();

extern int AddNode(), AddTrapObject();		   	/* mtu_mibtree.c parse-action rtns */
extern void AddTrapVar(), add_node_index(), 
  add_typedef_object(), add_datatype_object();

/*
 ** Local functions
 */
static int yyerror();

/*
 * Local variables
 */
T_MIB_OBJECT_NODE *node;				/* MIB Object */
T_TRAP_OBJECT *trap_object;				/* Trap Object */
T_TYPEDEF_OBJECT *typedef_object;			/* Typedef Object */

char typedef_name[SIZE_OF_IDENTIFIER];			/* Typedef identifier */
int is_enm = FALSE;                                   	/* Control flag: is_enumeration */
int is_valid_typedef_object;				

char idstring[SIZE_OF_IDENTIFIER];                             
unsigned int idnumber;

%}

%token NUMB 
  %token IDENT 
  %token KW_DEFINITIONS 
  %token KW_BEGIN 
  %token KW_END 
  %token KW_IMPORTS
  %token KW_FROM
  %token KW_OBJECT
  %token KW_IDENTIFIER
  %token KW_OBJECTTYPE
  %token KW_SYNTAX
  %token KW_ACCESS
  %token KW_STATUS
  %token KW_INTEGER
  %token KW_OCTET
  %token KW_STRING
  %token KW_NETWORKADDRESS 
  %token KW_COUNTER        
  %token KW_GAUGE          
  %token KW_TIMETICKS      
  %token KW_OPAQUE         
  %token KW_IPADDRESS      
  %token KW_SIZE
  %token KW_SEQUENCE
  %token KW_OF
  %token DOUBLE_DOT                             
  %token EQ
  %token LEFT_CURL
  %token RIGHT_CURL
  %token LEFT_BRT
  %token RIGHT_BRT
  %token COMMA
  %token S_COLON
  %token KW_readonly
  %token KW_readwrite
  %token KW_writeonly
  %token KW_notaccessible
  %token KW_nonzero
  %token KW_mandatory
  %token KW_optional
  %token KW_obsolete
  %token KW_deprecated
  %token KW_DESCRIPTION
  %token KW_REFERENCE
  %token KW_INDEX
  %token KW_OPTIONAL_CAPS
  %token KW_DEFVAL
  %token KW_NULL
  %token KW_TRAPTYPE
  %token KW_ENTERPRISE
  %token QUOTEDSTRING
  %token KW_DisplayString
  %token KW_VARIABLES
  %token KW_PhysAddress 
  %token SINGLE_DOT
  
  %start ModuleDefinition
  %%
  ModuleDefinition : ModuleReference KW_DEFINITIONS  EQ  KW_BEGIN ModuleBody KW_END           
{DEBUG_ACTION("ModuleDefinition")}
;

ModuleReference : ModuleName ModuleId {DEBUG_ACTION("ModuleReference")}
     ;
     
     ModuleName : IDENT {
       /*
	** Allocate MIB object node
	*/
       node = (T_MIB_OBJECT_NODE *)calloc (1, sizeof(T_MIB_OBJECT_NODE));
       
       /*
	** Allocate TRAP object node
	*/
       trap_object = (T_TRAP_OBJECT *)calloc (1, sizeof(T_TRAP_OBJECT));
       
       /*
	* Allocate TYPEDEF_OBJECT node 
	*/
       typedef_object = (T_TYPEDEF_OBJECT *)calloc(1, sizeof(T_TYPEDEF_OBJECT));
       is_valid_typedef_object = TRUE;

       strcpy (module_name, (char *)$1);	/* Store RFC module name */
       DEBUG_ACTION("ModuleName")
       }
     ;
     
     ModuleId : LEFT_CURL IdList RIGHT_CURL {DEBUG_ACTION("ModuleID")}
     |    {DEBUG_ACTION("ModuleID")}
     ;
     
     IdList   : Id   {DEBUG_ACTION("IdList")}
     | Id IdList   	{DEBUG_ACTION("IdList")}
     ;
     
     Id       : IDENT              {DEBUG_ACTION("Id")}
     | IDENT LEFT_BRT NUMB RIGHT_BRT {DEBUG_ACTION("Id")}
     | NUMB               {DEBUG_ACTION("Id")}
     ;
     
     
     ModuleBody : ExternalTypeReference ObjectList          {DEBUG_ACTION("ModuleBody")}
     ;
     
     ExternalTypeReference  : KW_IMPORTS FromList S_COLON   {DEBUG_ACTION("ExternalTypeReference")}
     | 			       {DEBUG_ACTION("ExternalTypeReference")}	
     ;
     
     ImportList : ImportItem                {DEBUG_ACTION("ImportList")}
     | ImportItem COMMA ImportList {DEBUG_ACTION("ImportList")}
     ;
     
     ImportItem : KW_NETWORKADDRESS       {DEBUG_ACTION("ImportItem")}
     | KW_COUNTER              {DEBUG_ACTION("ImportItem")}
     | KW_GAUGE                {DEBUG_ACTION("ImportItem")}
     | KW_TIMETICKS            {DEBUG_ACTION("ImportItem")}
     | KW_OPAQUE               {DEBUG_ACTION("ImportItem")}
     | KW_IPADDRESS            {DEBUG_ACTION("ImportItem")}
     | KW_OBJECTTYPE           {DEBUG_ACTION("ImportItem")}
     | KW_TRAPTYPE             {DEBUG_ACTION("ImportItem")}
     | KW_DisplayString        {
       typedef_object->kind = TAG;
       strcpy(typedef_object->name,"DisplayString");
       add_datatype_object(typedef_object, 0, "OCTETSTRING");
       add_typedef_object();
       DEBUG_ACTION("ImportItem")
       }
     | KW_PhysAddress          {
       typedef_object->kind = TAG;
       strcpy(typedef_object->name,"PhysAddress");
       add_datatype_object(typedef_object, 0, "OCTETSTRING");
       add_typedef_object();
       DEBUG_ACTION("ImportItem")
       }
     | IDENT		          {DEBUG_ACTION("ImportItem")}
     ;
     
     ImportModule : IDENT {DEBUG_ACTION("ImportModule")}
     ;
     
     FromList   : FromItem           {DEBUG_ACTION("FromList")}
     | FromItem FromList  {DEBUG_ACTION("FromList")}
     ;
     
     FromItem   : ImportList KW_FROM ImportModule  {DEBUG_ACTION("FormItem")}
     ;
     
     
     ObjectList : ObjectDefinition             {DEBUG_ACTION("ObjectList")}
     | ObjectDefinition ObjectList  {DEBUG_ACTION("ObjectList")}
     ;
     
     ObjectDefinition : ObjectIdentification   { 
       /*
	** Write "MIB object identifier" to to parse tree
	*/
       STORE_INT_FIELD ( OBJECT_IDENTIFIER, object);
       STORE_INT_FIELD ( UNKNOWN_ACCESS, access);
       STORE_INT_FIELD ( UNKNOWN_STATUS, status);
       STORE_INT_FIELD ( UNKNOWN_SYNTAX, syntax);
       if (!(AddNode(node))) yynerrs++;
       
       /*
	** Allocate next MIB object
	*/
       node = (T_MIB_OBJECT_NODE *)calloc(1, sizeof(T_MIB_OBJECT_NODE));
       {DEBUG_ACTION("ObjectDefinition")}
     }
     | LocalTypeDefinition       { 
       add_typedef_object();
       DEBUG_ACTION("ObjectDefinition") 
       }
     | ObjectTypeDefinition      { 
       node->object = OBJECT_TYPE;
       if (!(AddNode(node))) yynerrs++;
       node = (T_MIB_OBJECT_NODE *)calloc (1, sizeof(T_MIB_OBJECT_NODE));
       is_enm = FALSE;
       {DEBUG_ACTION("ObjectDefinition")}
     }
     | TrapTypeDefinition {
       /*
	* Add trap_object to mib tree.
	*/
       if (!(AddTrapObject(trap_object))) yynerrs++;
       trap_object = (T_TRAP_OBJECT *)calloc (1, sizeof(T_TRAP_OBJECT));
       {DEBUG_ACTION("ObjectDefinition")}
     }
     | error RIGHT_CURL {DEBUG_ACTION("ObjectDefinition")}
     ;
     
     ObjectIdentification : ObjectName KW_OBJECT KW_IDENTIFIER EQ ObjectIdentifier 
        {DEBUG_ACTION("ObjectIdentification")}
        ;

     ObjectName : IDENT {STORE_STRING_FIELD ((char *)$1, object_name); DEBUG_ACTION("ObjectName")}
     ;
     
     ObjectIdentifier : LEFT_CURL ObjectId RIGHT_CURL {DEBUG_ACTION("ObjectIdentifier")}
     ;      
     
     ObjectId : ObjectParent ObjectNumber {DEBUG_ACTION("ObjectId")}
     ;
     
     ObjectParent : IDENT { STORE_STRING_FIELD ((char *)$1, parent_name); DEBUG_ACTION("ObjectParent")}
     | NUMB {DEBUG_ACTION("ObjectParent")}
     ;
     
     ObjectNumber : NUMB    {STORE_INT_FIELD ($1, objectnumber); DEBUG_ACTION("ObjectNumber") }
     ;
     
     
     LocalTypeDefinition : IDENT {
       typedef_object->kind = TAG;
       strcpy(typedef_object->name, (char *)$1);
       DEBUG_ACTION("LocalTypeDefinition")
       }
     EQ TypeDef
       | KW_PhysAddress {
	 typedef_object->kind = TAG;
	 strcpy(typedef_object->name, "PhysAddress");
	 DEBUG_ACTION("LocalTypeDefinition")
	 }
     EQ TypeDef 
       | KW_DisplayString {
	 typedef_object->kind = TAG;
	 strcpy(typedef_object->name, "DisplayString");
	 DEBUG_ACTION("LocalTypeDefinition")
	 }
     EQ TypeDef 
       ;
     
     /*
      * If datatype is a sequence, we create a corresponding MCC datatype
      * of SEQUENCE type.
      */
     TypeDef    : KW_SEQUENCE LEFT_CURL SequenceItems RIGHT_CURL {is_valid_typedef_object = FALSE;}
       | KW_INTEGER { typedef_object->kind = ENUMERATION;} TypeSequence 
       | KW_INTEGER { add_datatype_object(typedef_object, 0, "INTEGER");}
       | KW_INTEGER Size { add_datatype_object(typedef_object, 0, "INTEGER");}
       | KW_OCTET KW_STRING  { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_OCTET KW_STRING  { add_datatype_object(typedef_object, 0, "OCTETSTRING");} 
       | KW_OCTET KW_STRING Size  { add_datatype_object(typedef_object, 0, "OCTETSTRING");}
       | KW_DisplayString { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_DisplayString { add_datatype_object(typedef_object, 0, "DisplayString");}
       | KW_DisplayString Size { add_datatype_object(typedef_object, 0, "DisplayString");}
       | KW_PhysAddress  { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_PhysAddress  { add_datatype_object(typedef_object, 0, "OCTETSTRING");}
       | KW_PhysAddress Size  { add_datatype_object(typedef_object, 0, "OCTETSTRING");}
       | KW_NETWORKADDRESS { add_datatype_object(typedef_object, 0, "IPAddress");}
       | KW_COUNTER  { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_COUNTER  { add_datatype_object(typedef_object, 0, "COUNTER");}
       | KW_GAUGE  { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_GAUGE  { add_datatype_object(typedef_object, 0, "GAUGE");}
       | KW_TIMETICKS  { add_datatype_object(typedef_object, 0, "UNSUPPORTED");} TypeSequence
       | KW_TIMETICKS  { add_datatype_object(typedef_object, 0, "TIMETICKS");}
       | KW_OPAQUE  { add_datatype_object(typedef_object, 0, "DisplayString");}
       | KW_IPADDRESS { add_datatype_object(typedef_object, 0, "IPAddress");}
       | KW_OBJECT KW_IDENTIFIER { add_datatype_object(typedef_object, 0, "OctetString");}
     ;

TypeSequence : LEFT_CURL SequenceList RIGHT_CURL 
;

SequenceItems : SequenceItem
  | SequenceItem Comma_or_Blank SequenceItems 
  ;

Comma_or_Blank : COMMA
  ;

SequenceItem : ItemName Itemtype
  ;

ItemName : IDENT
  ;

Itemtype :  KW_INTEGER               
  | KW_INTEGER Size              
  | KW_OCTET KW_STRING          
  | KW_OCTET KW_STRING Size         
  | KW_NETWORKADDRESS
  | KW_COUNTER
  | KW_GAUGE
  | KW_TIMETICKS
  | KW_OPAQUE
  | KW_IPADDRESS
  | KW_OBJECT KW_IDENTIFIER
  | KW_DisplayString
  | KW_DisplayString Size
  | KW_PhysAddress
  | KW_PhysAddress Size         
  | KW_NULL
  | IDENT                  
  ;

/*
 * Object-type structure
 */
ObjectTypeDefinition : ObjectTypeName KW_OBJECTTYPE
  KW_SYNTAX  SyntaxType
  KW_ACCESS  AccessType
  KW_STATUS  StatusType
  DescrPart
  ReferPart
  IndexPart
  DefValPart
  EQ TypeIdentifier   {DEBUG_ACTION("ObjectTypeDefinition")}
;

ObjectTypeName : IDENT {STORE_STRING_FIELD ((char *)$1, object_name);
			DEBUG_ACTION("ObjectTypename")
			}
     ;
     
     /*
      * SYNTAX type(ObjectSyntax)
      */
     
     SyntaxType : KW_INTEGER SimpleSequence   {STORE_INT_FIELD (INTEGER, syntax); is_enm = TRUE;}
     | KW_INTEGER Size SimpleSequence         {STORE_INT_FIELD (INTEGER, syntax); is_enm = TRUE;}
     | KW_INTEGER                             {STORE_INT_FIELD (INTEGER, syntax);}
     | KW_INTEGER Size                        {STORE_INT_FIELD (INTEGER, syntax);}
     | KW_OCTET KW_STRING SimpleSequence      {STORE_INT_FIELD (OCTETSTRING, syntax); is_enm = TRUE;}
     | KW_OCTET KW_STRING Size SimpleSequence {STORE_INT_FIELD (OCTETSTRING, syntax);is_enm = TRUE;}
     | KW_OCTET KW_STRING                     {STORE_INT_FIELD (OCTETSTRING, syntax);}
     | KW_OCTET KW_STRING Size                {STORE_INT_FIELD (OCTETSTRING, syntax);}
     | KW_PhysAddress SimpleSequence          {STORE_INT_FIELD (OCTETSTRING, syntax); is_enm = TRUE;}
     | KW_PhysAddress Size SimpleSequence     {STORE_INT_FIELD (OCTETSTRING, syntax); is_enm = TRUE;}
     | KW_PhysAddress                         {STORE_INT_FIELD (OCTETSTRING, syntax);}
     | KW_PhysAddress Size                    {STORE_INT_FIELD (OCTETSTRING, syntax);}
     | KW_DisplayString SimpleSequence        {STORE_INT_FIELD (DISPLAYSTRING, syntax); is_enm = TRUE;}
     | KW_DisplayString Size SimpleSequence   {STORE_INT_FIELD (DISPLAYSTRING, syntax); is_enm = TRUE;}
     | KW_DisplayString                       {STORE_INT_FIELD (DISPLAYSTRING, syntax);}
     | KW_DisplayString Size                  {STORE_INT_FIELD (DISPLAYSTRING, syntax);}
     | KW_NETWORKADDRESS                      {STORE_INT_FIELD (NETWORKADDRESS, syntax);}
     | KW_COUNTER SimpleSequence              {STORE_INT_FIELD (COUNTER, syntax); is_enm = TRUE;}
     | KW_COUNTER                             {STORE_INT_FIELD (COUNTER, syntax);}
     | KW_GAUGE  SimpleSequence               {STORE_INT_FIELD (GAUGE, syntax); is_enm = TRUE;}
     | KW_GAUGE                               {STORE_INT_FIELD (GAUGE, syntax);}
     | KW_TIMETICKS  SimpleSequence           {STORE_INT_FIELD (TIMETICKS, syntax); is_enm = TRUE;}
     | KW_TIMETICKS                           {STORE_INT_FIELD (TIMETICKS, syntax);}
     | KW_OPAQUE                              {STORE_INT_FIELD (OPAQUE, syntax);}
     | KW_IPADDRESS                           {STORE_INT_FIELD (IPADDRESS, syntax);}
     | KW_OBJECT KW_IDENTIFIER                {STORE_INT_FIELD (OBJECTID, syntax);}
     | KW_SEQUENCE KW_OF IDENT	 	 {STORE_INT_FIELD (SEQUENCE_OF, syntax); STORE_STRING_FIELD ((char *)$1, typedef_name);}
     | KW_NULL  				 {STORE_INT_FIELD (UNKNOWN_SYNTAX, syntax);}
     | IDENT Size                        {STORE_INT_FIELD (DEFINED_SYNTAX, syntax); STORE_STRING_FIELD((char *)$1,typedef_name);}
     | IDENT                  	         {STORE_INT_FIELD (DEFINED_SYNTAX, syntax); STORE_STRING_FIELD((char *)$1,typedef_name);}
     ;
     
     /*
      * Size information is ignored for now.
      */
     
     Size :  LEFT_BRT KW_SIZE LEFT_BRT NUMB DOUBLE_DOT NUMB RIGHT_BRT RIGHT_BRT {DEBUG_ACTION("?")}
     |  LEFT_BRT KW_SIZE LEFT_BRT NUMB RIGHT_BRT RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT NUMB DOUBLE_DOT NUMB RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT KW_SIZE LEFT_BRT NUMB DOUBLE_DOT IDENT RIGHT_BRT RIGHT_BRT {DEBUG_ACTION("?")}
     |  LEFT_BRT KW_SIZE LEFT_BRT IDENT RIGHT_BRT RIGHT_BRT {DEBUG_ACTION("?")}
     |  LEFT_BRT NUMB DOUBLE_DOT IDENT RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT KW_SIZE LEFT_BRT IDENT DOUBLE_DOT NUMB RIGHT_BRT RIGHT_BRT {DEBUG_ACTION("?")}
     |  LEFT_BRT IDENT DOUBLE_DOT NUMB RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT KW_SIZE LEFT_BRT IDENT DOUBLE_DOT IDENT RIGHT_BRT RIGHT_BRT {DEBUG_ACTION("?")}
     |  LEFT_BRT IDENT DOUBLE_DOT IDENT RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT IDENT RIGHT_BRT  {DEBUG_ACTION("?")}
     |  LEFT_BRT NUMB RIGHT_BRT  {DEBUG_ACTION("?")}
     ;
     
     
     SimpleSequence : LEFT_CURL {
       typedef_object->kind = ENUMERATION;
       sprintf(typedef_name, "%s_enm", node->object_name);
       strcpy (typedef_object->name, typedef_name);
       strcpy (node->typedef_name, typedef_name);
     } 
     SequenceList RIGHT_CURL
       {add_typedef_object ();}
       ;

     SequenceList : Sequence                {DEBUG_ACTION("?")}
     | Sequence Comma_or_Blank 
       SequenceList  
       ;
     
     Sequence : TypeName LEFT_BRT TypeNumber RIGHT_BRT {
       add_datatype_object (typedef_object, idnumber, idstring);
     }
     ;
     
     TypeName : IDENT       {strcpy(idstring,(char *)$1);}
     ;
     
     TypeNumber : NUMB    {idnumber = $1;}
     | KW_nonzero         {idnumber = 1;}
     ;
     
     /*
      * Copy ACCESS information into MIB node structure. Used to determine if this
      * will be a SETTABLE or NONSETABLE attribute in MSL.
      */
     
     AccessType : KW_readonly   {STORE_INT_FIELD (READONLY, access);}
     | KW_readwrite         {STORE_INT_FIELD (READWRITE, access);}
     | KW_writeonly         {STORE_INT_FIELD (WRITEONLY, access);}
     | KW_notaccessible     {STORE_INT_FIELD (NOTACCESSIBLE, access);}
     | IDENT                {STORE_INT_FIELD (UNKNOWN_ACCESS, access);}
     ;
     
     /*
      * Status clause
      */
     StatusType : KW_mandatory             {STORE_INT_FIELD (MANDATORY, status);}
     | KW_optional              {STORE_INT_FIELD (OPTIONAL, status);}
     | KW_obsolete              {STORE_INT_FIELD (OBSOLETE, status);}           
     | KW_deprecated            {STORE_INT_FIELD (DEPRECATED, status);}
     | IDENT                    {STORE_INT_FIELD (UNKNOWN_STATUS, status);}
     ;
     
     DescrPart  : KW_DESCRIPTION QUOTEDSTRING {
       if (cmdline_table[OPT_HELPFILE].is_enabled) {
	 node->description = (char *)malloc (strlen((char *)$2)+1);
	 strcpy (node->description, (char *)$2);
	 DEBUG_ACTION("DescrPart")
	 }
     }
       |
       ;
     
     ReferPart  : KW_REFERENCE QUOTEDSTRING
       |
       ;
     
     IndexPart  : KW_INDEX LEFT_CURL IndexTypes IndexMagic RIGHT_CURL
       |
{ DEBUG_ACTION("IndexPart") }
;

IndexTypes : IndexType { 
  DEBUG_ACTION("IndexTypes");
}
     | IndexTypes COMMA IndexType
{ DEBUG_ACTION("IndexTypes") }
;

IndexType  : IDENT {add_node_index((char*)$1);}
  | ObjectIndex {DEBUG_ACTION("IndexType");}
  ;

/*
 ** CA_MTU/mtu_entitytree.c currently does not handle an index being referenced as a data type
 */
ObjectIndex : KW_INTEGER 	{add_node_index("INTEGER"); DEBUG_ACTION("ObjectIndex");}
  | KW_INTEGER Size 		{add_node_index("INTEGER"); DEBUG_ACTION("ObjectIndex");}
  | KW_OCTET KW_STRING 		{add_node_index("OCTET STRING"); DEBUG_ACTION("ObjectIndex");}
  | KW_OCTET KW_STRING Size 	{add_node_index("OCTET STRING"); DEBUG_ACTION("ObjectIndex");}
  | KW_PhysAddress 		{add_node_index("OCTET STRING"); DEBUG_ACTION("ObjectIndex");}
  | KW_PhysAddress Size 	{add_node_index("OCTET STRING"); DEBUG_ACTION("ObjectIndex");}
  | KW_DisplayString 		{add_node_index("DisplayString"); DEBUG_ACTION("ObjectIndex");}
  | KW_DisplayString Size 	{add_node_index("DisplayString"); DEBUG_ACTION("ObjectIndex");}
  | KW_OBJECT KW_IDENTIFIER 	{add_node_index("OBJECT IDENTIFIER"); DEBUG_ACTION("ObjectIndex");}
  | KW_NETWORKADDRESS 		{add_node_index("NetworkAddress"); DEBUG_ACTION("ObjectIndex");}
  | KW_IPADDRESS 		{add_node_index("IpAddress"); DEBUG_ACTION("ObjectIndex");}
  ;

/*
 * The keyword OPTIONAL is no longer used in the INDEX clause. It is still
 * here for historical reasons, and for pre-historic mibs.
 */

IndexMagic  : COMMA KW_INTEGER KW_OPTIONAL_CAPS { 
  node->index[node->num_indices] = (char *)malloc(strlen((char *)$1)+1);
  strcpy (node->index[node->num_indices], (char *)$1);
  node->num_indices++;
}
|
  ;

DefValPart  : KW_DEFVAL DefValue   {DEBUG_ACTION("DefValPart")}
     | 
       ;
     
     DefValue    : NUMB                              {DEBUG_ACTION("DefValue")}
     | QUOTEDSTRING                      {DEBUG_ACTION("DefValue")}
     | IDENT                             {DEBUG_ACTION("DefValue")}
     | KW_NULL                           {DEBUG_ACTION("DefValue")}
     | LEFT_CURL IDENT IDENT RIGHT_CURL  {DEBUG_ACTION("DefValue")}
     | LEFT_CURL IDENT NUMB RIGHT_CURL   {DEBUG_ACTION("DefValue")}
     | LEFT_CURL IDENT RIGHT_CURL        {DEBUG_ACTION("DefValue")}
     | LEFT_CURL NUMB RIGHT_CURL         {DEBUG_ACTION("DefValue")}
     | LEFT_CURL IDENT LEFT_BRT NUMB RIGHT_BRT RIGHT_CURL  {DEBUG_ACTION("DefValue")}
     | LEFT_CURL RIGHT_CURL              {DEBUG_ACTION("DefValue")}
     | LEFT_CURL QUOTEDSTRING RIGHT_CURL {DEBUG_ACTION("DefValue")}
     |
       ;
     
     TypeIdentifier : LEFT_CURL TypeObjectId RIGHT_CURL {DEBUG_ACTION("TypeIdentifier")}
     ;
     
     TypeObjectId : TypeObjectParent TypeObjectNumber {DEBUG_ACTION("TypeObjectId")}
     ;
     
     TypeObjectParent : IDENT 
{ STORE_STRING_FIELD ((char *)$1, parent_name);}
;

TypeObjectNumber : NUMB { STORE_INT_FIELD ($1, objectnumber); }
;


TrapTypeDefinition : TrapTypeName KW_TRAPTYPE KW_ENTERPRISE  EnterpriseName VarPart TrapDescrPart ReferPart
  EQ TrapTypeValue 
  ;

TrapTypeName : IDENT	{ strcpy(trap_object->trap_name, (char *)$1); }
     ;
     
     
     TrapTypeValue : NUMB    {trap_object->trap_number = $1;}
     ;
     
     EnterpriseName : EnterpriseDotNotation {strcpy(trap_object->enterprise_name, (char *)$1);}
       ;
     
     VarPart : KW_VARIABLES LEFT_CURL VarTypes RIGHT_CURL
       |
       ;
     
     VarTypes : VarType
       | VarTypes COMMA VarType
       ;
     
     VarType : VarDotNotation
       |
       ;
     
     /*
      * When a variable is defined in dot notation , we ignore everything till
      * after the last dot. The actual variable name appears after the last dot.
      * (parent.parent.parent.variable)
      */
     
     VarDotNotation : IDENT SINGLE_DOT VarDotNotation
       | IDENT {AddTrapVar(trap_object, (char *)$1);}
     ;


     EnterpriseDotNotation : IDENT SINGLE_DOT EnterpriseDotNotation
       | IDENT
       ;


     TrapDescrPart  : KW_DESCRIPTION QUOTEDSTRING {
       if (cmdline_table[OPT_HELPFILE].is_enabled) {
	 trap_object->description = (char *)malloc (strlen((char *)$2)+1);
	 strcpy (trap_object->description, (char *)$2);
	 DEBUG_ACTION("TrapDescrPart")
	 }
     }
       |
       ;
%%

/******************************************************************************************
  void debug_action	-- For debug purpose, trace LHS production states.
                           Handy spot for break-points...
******************************************************************************************/
#ifdef DEBUG
static void debug_action
# if PROTOTYPE_ALLOWED
(char *s)
# else
(s)
char *s;
# endif
{
  printf ("\n\nyyacc action at state (%s)", s);
  printf ("\nyylineno=\t%d\t\t(line number)", yylineno);
  printf ("\nyyleng, yytext=\t%d, %s\t\t(lex string)", yyleng, yytext);
  printf ("\n");
}
#endif
/********************************************************************************
  yyerror(s)
********************************************************************************/
static int yyerror(s) 
char *s;
{
  fprintf (stdout, MSG(msg60, "MTU warning -- %s. (line %d) Failed to process token (%d).\n"),
	   s, yylineno, yychar);
  if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg60(), s, yylineno, yychar);
  MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
}

