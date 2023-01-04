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
static char *rcsid = "@(#)$RCSfile: mtu_msfile.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:27:56 $";
#endif
/* Title: mtu_msfile.c		Transform EntityTree, producing MS file */
/*
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
 **  FACILITY:  Common Agent/DECmcc Framework
 **
 **  MODULE DESCRIPTION:
 **
 **	MTU_MSFILE.C	- Transform EntityTree, producing MS file 
 **
 **  AUTHORS:
 **
 **      Rahul Bose
 **
 **  CREATION DATE:  1-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 *
 * Edit#     Date	 By			    Description
 * -----  ------------	----		----------------------------------
 *
 * 01	  01-March-1991	Rahul Bose		    Created.
 * 02     26-Sep-1992	Pete Burgess	Decomposed MIBTREE.c into 3 modules (mib, entity, files)
 *					o c89 -std, I18n, etc.
 */

/*
 ** Header files
 */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtu.h"
#include "mtu_msg.h"

/* 
 ** external data
 */
extern char *prefix_event_oid;
extern char *prefix_event_enterprise_arg;
extern char *prefix_event_agent_addr_arg;
extern char *prefix_event_generic_trap_arg;
extern char *prefix_event_specific_trap_arg;
extern char *prefix_event_time_stamp_arg;
extern char *prefix_event_varbind_arg;

extern FILE *yyout;                  
extern FILE *ms_stream;                  
extern FILE *info_stream;
extern FILE *help_stream;                  
extern char log_header_str1[PAGE_WIDTH];
extern char log_header_str2[PAGE_WIDTH];
extern T_NODE_SUMMARY_INFO *ptr_iso_parent;
extern T_PROTOCOL_DEF *protocol_definition;
extern T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; 
extern T_TYPEDEF_OBJECT *typedef_object_list[2];
extern char module_name[SIZE_OF_IDENTIFIER];
extern char *mtu_version;
extern char ms_filename[SIZE_OF_FILENAME];
extern char mib_filename[SIZE_OF_FILENAME];
extern char log_filename[SIZE_OF_FILENAME];

extern T_ENTITY_NODE *entity_root;	        	/* Root of entity model tree */


/********************************************************************************
  produce_help_keylines - Produce DECmcc HELPfile keylines
********************************************************************************/
static void produce_help_keylines
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity_node)
#else
(entity_node)

T_ENTITY_NODE *entity_node;
#endif
{
T_ENTITY_NODE *next_entity_node;
T_ENTITY_ATTRIBUTE_NODE *attr_node;
T_EVENT_OBJECT *event_node;

  /*
   ** Process entity node, producing Key lines
   */
  fprintf (help_stream, "\n<KEY> = %s\n\n", entity_node->symbol);
  fprintf (help_stream, " %s\n\n", entity_node->description);
  
  if (entity_node->event_list) {
    fprintf (help_stream, "\n<KEY> = %s_Events\n\n", entity_node->symbol);
    fprintf (help_stream, " Events for Child Entity %s :\n", entity_node->object_name);
    for (event_node = entity_node->event_list; event_node != NULLPTR; event_node = event_node->event_next) 
      fprintf (help_stream, "\n<KEY> = %s\n %s\n\n", event_node->event_symbol, event_node->description);
  }
  if (entity_node->identifier_list) {
    fprintf (help_stream, "\n<KEY> = %s_it\n\n", entity_node->symbol);
    fprintf (help_stream, " Identifier Attributes for Child Entity %s :\n", entity_node->object_name);
    for (attr_node = entity_node->identifier_list; attr_node != NULLPTR; attr_node = attr_node->nextsibling) 
      fprintf (help_stream, "\n<KEY> = %s\n %s\n\n", attr_node->symbol, attr_node->description);
  }
  if (entity_node->status_list) {
    fprintf (help_stream, "\n<KEY> = %s_st\n\n", entity_node->symbol);
    fprintf (help_stream, " Status Attributes for Child Entity %s :\n", entity_node->object_name);  
    for (attr_node = entity_node->status_list; attr_node != NULLPTR; attr_node = attr_node->nextsibling) 
      fprintf (help_stream, "\n<KEY> = %s\n %s\n\n", attr_node->symbol, attr_node->description);
  }
  if (entity_node->counter_list) {
    fprintf (help_stream, "\n<KEY> = %s_co\n\n", entity_node->symbol);
    fprintf (help_stream, " Counter Attributes for Child Entity %s :\n", entity_node->object_name);  
    for (attr_node = entity_node->counter_list; attr_node != NULLPTR; attr_node = attr_node->nextsibling) 
      fprintf (help_stream, "\n<KEY> = %s\n %s\n\n", attr_node->symbol, attr_node->description);
  }
  if (entity_node->characteristic_list) {
    fprintf (help_stream, "\n<KEY> = %s_ch\n\n", entity_node->symbol);
    fprintf (help_stream, " Characteristic Attributes for Child Entity %s :\n", entity_node->object_name);  
    for (attr_node = entity_node->characteristic_list; attr_node != NULLPTR; attr_node = attr_node->nextsibling) 
      fprintf (help_stream, "\n<KEY> = %s\n %s\n\n", attr_node->symbol, attr_node->description);
  }

/*
 ** Recursively process next entity level
 */
  if (entity_node->childlist != 0) { 
    next_entity_node = entity_node->childlist;
    while ( next_entity_node != 0) {
      produce_help_keylines(next_entity_node);
      next_entity_node = next_entity_node->nextsibling;
    }
  }
}

/********************************************************************************
  oid_to_dot_string	- Transform OID structure to Dot String
********************************************************************************/
static void oid_to_dot_string
#if PROTOTYPE_ALLOWED
  (T_OID *oid, char *string)
#else
  (oid, string)
T_OID *oid;
 char *string;
#endif
{
  int i;
  char number [10];

  *string = NUL;
  for (i=0;i< oid->entries; i++) {
    sprintf (number, "%d.", oid->tree[i]);
    strcat (string, number);
  }
  string [strlen(string)-1] = NUL;
}

/********************************************************************************
  oid_to_string	- Transform OID structure to String
********************************************************************************/
static void oid_to_string
#if PROTOTYPE_ALLOWED
  (T_OID *oid, char *string)
#else
  (oid, string)
T_OID *oid;
 char *string;
#endif
{
  int i;
  char number [10];

  strcpy (string, "{");
  for (i=0;i< oid->entries; i++) {
    sprintf (number, "%d ", oid->tree[i]);
    strcat (string, number);
  }
  string [strlen(string)-1] = '}';
}

static void event_oid_encode
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity, int event_code, char *string)
#else
  (entity, event_code, string)
  T_ENTITY_NODE *entity;
  int event_code;
  char *string;
#endif
{
  char number [10];
  int i;

  strcpy (string, "{");
  strcat (string, prefix_event_oid);
  strcat (string, " ");
  for (i=0;i< entity->oid.entries; i++) {
    sprintf (number, "%d ", entity->oid.tree[i]);
    strcat (string, number);
  }
  sprintf (number, "%d}", event_code);
  strcat (string, number);
}

static void event_arg_pdu_oid_encode
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity, char *prefix, int event_code, char *string)
#else
  (entity, prefix, event_code, string)
  T_ENTITY_NODE *entity;
  char *prefix;
  int event_code;
  char *string;
#endif
{
  char number [10];
  int i;

  strcpy (string, "{");
  strcat (string, prefix);
  strcat (string, " ");
  for (i=0;i< entity->oid.entries; i++) {
    sprintf (number, "%d ", entity->oid.tree[i]);
    strcat (string, number);
  }
  sprintf (number, "%d}", event_code);
  strcat (string, number);
}

static void event_arg_varbind_oid_encode
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity, int event_code, T_OID *event_arg_oid, char *string)
#else
  (entity, event_code, event_arg_oid, string)
  T_ENTITY_NODE *entity;
  int event_code;
  T_OID *event_arg_oid;
  char *string;
#endif
{
  int i;
  char number [10];

  strcpy (string, "{");
  strcat (string, prefix_event_varbind_arg);
  strcat (string, " ");
  for (i=0;i< entity->oid.entries; i++) {
    sprintf (number, "%d ", entity->oid.tree[i]);
    strcat (string, number);
  }
  sprintf (number, "%d ", event_code);
  strcat (string, number);
  for (i=0;i< event_arg_oid->entries; i++) {
    sprintf (number, "%d ", event_arg_oid->tree[i]);
    strcat (string, number);
  }
  string[strlen(string)-1] = '}';
}

/********************************************************************************
  ms_display_identifier_list	- Display Identifier list in MS 
********************************************************************************/
static void ms_display_identifier_list 
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity_node, char *ident_str)
#else
(entity_node, ident_str)
T_ENTITY_NODE *entity_node;      
char *ident_str;
#endif
{
  static char *no_comma = "", *comma = ", ";
  
  T_ENTITY_ATTRIBUTE_NODE *identifier_node;
  char *ptr_comma;
  
  fprintf(ms_stream,"%s    IDENTIFIER = ( ", ident_str);
  
  identifier_node = entity_node->identifier_list;
  ptr_comma = no_comma;
  while (identifier_node) {
    fprintf(ms_stream,"%s%s ", ptr_comma, identifier_node->object_name);
    ptr_comma = comma;
    identifier_node = identifier_node->nextsibling;
  }
  fprintf(ms_stream,"),\n");
}

/********************************************************************************
  syntax_to_string	- Transform syntax to string
********************************************************************************/
static void syntax_to_string 
#ifdef PROTOTYPE_ALLOWED
  (T_SYNTAX syntax_code, char * typedef_name, char * syntax_string)
#else
(syntax_code, typedef_name, syntax_string)
T_SYNTAX syntax_code;
char *typedef_name;
char *syntax_string;
#endif
{
  if (strlen(typedef_name)) {
    strcpy (syntax_string, typedef_name);
  }
  else {
    switch (syntax_code) {
    case COUNTER : 	strcpy(syntax_string,"Counter"); break;
    case GAUGE :   	strcpy(syntax_string,"Gauge"); break;
    case TIMETICKS : 	strcpy(syntax_string,"Timeticks"); break;
    case INTEGER : 	strcpy(syntax_string,"Integer"); break;
    case OCTETSTRING : 	strcpy(syntax_string,"Octetstring"); break;
    case DISPLAYSTRING : strcpy(syntax_string,"DisplayString"); break;
    case NETWORKADDRESS : strcpy(syntax_string,"IPAddress"); break;
    case OPAQUE : 	strcpy(syntax_string,"Opaque"); break;
    case IPADDRESS : 	strcpy(syntax_string,"IPAddress"); break;
    case OBJECTID : 	strcpy(syntax_string,"ObjectIdentifier"); break;
      default : 	strcpy(syntax_string,"UndefinedType"); DEBUG_BREAK;
                        MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
    }
  }
}

/********************************************************************************
  ms_display_attribute_list	- Display Attribute list
********************************************************************************/
static void ms_display_attribute_list
#ifdef PROTOTYPE_ALLOWED
  (T_ENTITY_ATTRIBUTE_NODE **ptr_attr_node, char *ms_indent_str, char *help_str, char *partition_name, int *ptr_is_settable)
#else
  (ptr_attr_node, ms_indent_str, help_str, partition_name, ptr_is_settable)
T_ENTITY_ATTRIBUTE_NODE **ptr_attr_node;
char *ms_indent_str, *help_str, *partition_name;
int *ptr_is_settable;
#endif
{
  T_ENTITY_ATTRIBUTE_NODE *attr_node;
  char access_string[SIZE_OF_IDENTIFIER];
  char datatype_string[SIZE_OF_IDENTIFIER];
  char oid_string[SIZE_OF_IDENTIFIER];
  
  *ptr_is_settable = FALSE;
  attr_node = *ptr_attr_node;
  while (attr_node) {
    syntax_to_string(attr_node->syntax, attr_node->typedef_name, datatype_string);
    fprintf(ms_stream,"%s    ATTRIBUTE %s   = %d : %s\n", ms_indent_str,
	    attr_node->object_name, attr_node->objectnumber, datatype_string);
    oid_to_string(&attr_node->oid, oid_string);
    fprintf(ms_stream,"%s        SNMP_OID = %s,\n", ms_indent_str, oid_string);
    fprintf(ms_stream,"%s        DNA_CMIP_INT = %d,\n", ms_indent_str, attr_node->objectnumber);
    
    switch (attr_node->access) 
      {
      case READWRITE : strcpy(access_string, "SETTABLE"); *ptr_is_settable = TRUE; break;
      case WRITEONLY : strcpy(access_string, "WRITEONLY"); *ptr_is_settable = TRUE; break;
	default : strcpy(access_string,"NONSETTABLE");
      }
    fprintf(ms_stream,"%s        ACCESS     = %s,\n", ms_indent_str, access_string);
    if (attr_node->access == NOTACCESSIBLE) {
      fprintf(ms_stream, "%s        DISPLAY    = %s,\n", ms_indent_str, "FALSE");
    }
    else
      fprintf(ms_stream, "%s        DISPLAY    = %s, \n", ms_indent_str, "TRUE");
    fprintf(ms_stream, "%s        CATEGORIES = (CONFIGURATION),\n", ms_indent_str);
    fprintf(ms_stream, "%s        SYMBOL = %s\n",  ms_indent_str, attr_node->symbol);
    fprintf(ms_stream, "%s    END ATTRIBUTE %s;\n", ms_indent_str, attr_node->object_name);
    
    /* Write into log file */
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%s    %s (%d) : %s %s\n", ms_indent_str,
	    attr_node->object_name, attr_node->objectnumber, datatype_string, oid_string);
    
    if (cmdline_table[OPT_HELPFILE].is_enabled) {
      fprintf(help_stream,"%s %s %s = %s\n", help_str, partition_name, attr_node->object_name, attr_node->symbol);
    }

    attr_node = attr_node->nextsibling;
  } /* End of while-do */
}

/****************************************************************************
  Traverse the EMA tree and generate MSL, optional MOMGEN parameter file, 
  and optional DECmcc help file
  ****************************************************************************/
int TraverseTree
#ifdef PROTOTYPE_ALLOWED 
  (T_ENTITY_NODE *entity_node, char *ms_indent_str, char* help_string)
#else
  (entity_node, ms_indent_str, help_string)
T_ENTITY_NODE *entity_node;
char *ms_indent_str;
char* help_string;

#endif

{
  char new_ms_indent_str[PAGE_WIDTH], new_helpstr [PAGE_WIDTH];
  char oid_dot_string[SIZE_OF_IDENTIFIER];
  char oid_string[SIZE_OF_IDENTIFIER];
  T_ENTITY_ATTRIBUTE_NODE *attr_node;
  int is_settable;  
  T_ENTITY_NODE *k;
  T_EVENT_OBJECT *event_object;
  T_EVENT_ARGUMENT *event_argument;
  int arg_code;
  char datatype_string[SIZE_OF_IDENTIFIER];

  /*
   ** Process Entity node, producing Entity MS and Entity Helpfile information
   */
  
  /* Make stack local copies of "recursion variable values" */
  sprintf (new_helpstr, "%s %s", help_string, entity_node->object_name);
  strcpy(new_ms_indent_str, ms_indent_str);
  
  /* 
   * Write to help file.
   */
  if (cmdline_table[OPT_HELPFILE].is_enabled)
    fprintf(help_stream,"%s = %s\n", new_helpstr,entity_node->symbol);

  /*
   ** If Entity is the ROOT ENTITY, then produce top-level entity
   */
  if (!(entity_node->parent)) {
    /*
     ** If the Root entity is a known global entity...
     */
    if (entity_node->is_global_entity) {
      fprintf(ms_stream,"GLOBAL ENTITY %s = %d :\n", entity_node->object_name, entity_node->entitycode);
    }
    else {
      fprintf(ms_stream,"CHILD ENTITY %s = %d :\n", entity_node->object_name, entity_node->entitycode);
      fprintf(ms_stream,"    PARENT = ( %s ),\n", protocol_definition->protocol_entity_parent);
    }
    oid_to_string(&entity_node->oid, oid_string);
    fprintf(ms_stream,"    SNMP_OID = %s,\n", oid_string);
    fprintf(ms_stream,"    DNA_CMIP_INT = %d,\n", entity_node->entitycode);
    ms_display_identifier_list (entity_node, new_ms_indent_str);
    fprintf(ms_stream,"    DYNAMIC = FALSE,\n");
    fprintf(ms_stream,"    SYMBOL = %s,\n\n",  entity_node->symbol);
    
    /* Write into log file */
    if (cmdline_table[OPT_LISTFILE].is_enabled) {
      fprintf(yyout,"\n\f%s\n\n", log_header_str1);
      sprintf(log_header_str2, MSG(msg51, "RFC Input: %s  Management Specification Output: %s\n"), mib_filename, ms_filename);
      fprintf(yyout, log_header_str2);
      fprintf(yyout, MSG(msg52, "Section 2\tManagement Specification Summary:\n\n"));
    
      /* Write into log file: [Global | Child] entity (name, entitycode, SNMP oid) */
      if (entity_node->is_global_entity)
	fprintf(yyout,"%sGlobal Entity : %s (%d) %s\n", new_ms_indent_str, 
		entity_node->object_name, entity_node->entitycode, oid_string);
      else
	fprintf(yyout,"%sChild Entity : %s (%d) %s\n", new_ms_indent_str, 
		entity_node->object_name, entity_node->entitycode, oid_string);
    }
  }
  /*
   ** Else process non-root entity, producing MS, Logfile, HelpFile output
   */
  else {
    strcat(new_ms_indent_str,"    ");       /* Ensure proper indentation */
    fprintf(ms_stream,"%sCHILD ENTITY %s = %d :\n", new_ms_indent_str, entity_node->object_name, entity_node->entitycode);
    oid_to_string(&entity_node->oid, oid_string);
    fprintf(ms_stream,"%s    SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
    fprintf(ms_stream,"%s    DNA_CMIP_INT = %d,\n", new_ms_indent_str, entity_node->entitycode);
    ms_display_identifier_list (entity_node, new_ms_indent_str);
    fprintf(ms_stream,"%s    DYNAMIC = TRUE,\n", new_ms_indent_str);
    fprintf(ms_stream,"%s    SYMBOL = %s,\n\n", new_ms_indent_str, entity_node->symbol);
    
    /* Write into log file: Child entity (name, code, SNMP oid) */
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%sChild Entity : %s (%d) %s\n", new_ms_indent_str, 
	    entity_node->object_name, entity_node->entitycode, oid_string);
  }
  
  /*
   ** Conditionally write MOMGEN information/parameter element
   ** (class, prefix, parent, parent_prefix) for child-entities 
   */
  if (info_stream != 0) {
    if (!(entity_node->is_global_entity)) {
      oid_to_dot_string (&entity_node->oid, oid_dot_string);
      fprintf(info_stream, "class = %s\n", oid_dot_string);
      fprintf(info_stream, "prefix = %s_\n", entity_node->object_name);
    
      if (entity_node->parent) {
	oid_to_dot_string (&entity_node->parent->oid, oid_dot_string);
	fprintf(info_stream, "parent = %s\n", oid_dot_string);
	fprintf(info_stream, "parent_prefix= %s_\n", entity_node->parent->object_name);
      }
      else {
	oid_to_dot_string (&protocol_definition->protocol_entity_parent_oid, oid_dot_string);
	fprintf(info_stream, "parent = %s\n", oid_dot_string);
      }
    }
  }
  
  /*
   ** Print Identifier Partition and attributes
   */
  if ((attr_node = entity_node->identifier_list)) {	/* If Partition is not empty, then produce MS, help, logfile*/
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%s  Identifier Attributes\n", 
							new_ms_indent_str);    /* Write into log file */
    if (cmdline_table[OPT_HELPFILE].is_enabled) 
      fprintf(help_stream,"%s IDENTIFIERS = %s_it\n", new_helpstr, entity_node->symbol);
    fprintf(ms_stream, "%s  IDENTIFIER ATTRIBUTES\n", new_ms_indent_str);
    ms_display_attribute_list (&entity_node->identifier_list, (char *)new_ms_indent_str, new_helpstr, 
			       "IDENTIFIERS", &is_settable);
    if (is_settable) 
      if (cmdline_table[OPT_DECMCC_MS].is_enabled)
	fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_set.ms;\n",new_ms_indent_str);
    fprintf(ms_stream,"%s  END ATTRIBUTES;\n\n", new_ms_indent_str);
  }	/* End identifier attributes */
  /*
   ** Print Status Partition and attributes
   */
  if ((attr_node = entity_node->status_list)) {	/* If Partition is not empty, then produce MS, help, logfile*/
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%s  Status Attributes\n",new_ms_indent_str);
    if (cmdline_table[OPT_HELPFILE].is_enabled) 
      fprintf(help_stream,"%s STATUS = %s_st\n", new_helpstr, entity_node->symbol);
    fprintf(ms_stream, "%s  STATUS ATTRIBUTES\n", new_ms_indent_str);
    ms_display_attribute_list (&entity_node->status_list, (char *)new_ms_indent_str, (char *)new_helpstr, 
			       "STATUS", &is_settable);
    fprintf(ms_stream,"%s  END ATTRIBUTES;\n\n", new_ms_indent_str);
  } /* End attributes */
  
  /*
   ** Print Counter Partition and attributes
   */
  if ((attr_node = entity_node->counter_list)) {	/* If Partition is not empty, then produce MS, help, logfile*/
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%s  Counter Attributes\n",new_ms_indent_str);
    if (cmdline_table[OPT_HELPFILE].is_enabled) 
      fprintf(help_stream,"%s COUNTERS = %s_co\n", new_helpstr, entity_node->symbol);
    fprintf(ms_stream, "%s  COUNTER ATTRIBUTES\n", new_ms_indent_str);
    ms_display_attribute_list (&entity_node->counter_list, (char *)new_ms_indent_str, new_helpstr, 
			       "COUNTERS", &is_settable);
    if (is_settable) 
      if (cmdline_table[OPT_DECMCC_MS].is_enabled)
	fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_set.ms;\n",new_ms_indent_str);
    fprintf(ms_stream,"%s  END ATTRIBUTES;\n\n", new_ms_indent_str);
  }	/* End counter attributes */
  /*
   ** Print Characteristic Partition and attributes
   */
  if ((attr_node = entity_node->characteristic_list)) {	/* If Partition is not empty, then produce MS, help, logfile*/
   if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout,"%s  Characteristic Attributes\n",new_ms_indent_str);
    if (cmdline_table[OPT_HELPFILE].is_enabled) 
      fprintf(help_stream,"%s CHARACTERISTICS = %s_ch\n", new_helpstr, entity_node->symbol);
    fprintf(ms_stream, "%s  CHARACTERISTIC ATTRIBUTES\n", new_ms_indent_str);
    ms_display_attribute_list (&entity_node->characteristic_list, (char *)new_ms_indent_str, (char *)new_helpstr, 
			       "CHARACTERISTICS", &is_settable);
    if (is_settable) 
      if (cmdline_table[OPT_DECMCC_MS].is_enabled)
	fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_set.ms;\n",new_ms_indent_str);
    fprintf(ms_stream,"%s  END ATTRIBUTES;\n\n", new_ms_indent_str);
  }	/* End characteristic attributes */
  /* 
   * Include show & register directives for every child entity 
   */
  if (cmdline_table[OPT_DECMCC_MS].is_enabled) {
    fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_show.ms;\n",new_ms_indent_str);
    fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_reference.ms;\n",new_ms_indent_str);
    fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_register.ms;\n",new_ms_indent_str);
  }

  /*
   * Create Getevent directive if entity node has an event list.
   */
  if (event_object = entity_node->event_list) {
    fprintf(ms_stream,"\n%s  EVENT PARTITION CONFIGURATION EVENTS = 15:\n",new_ms_indent_str);
    if (cmdline_table[OPT_HELPFILE].is_enabled)
      fprintf(help_stream,"%s EVENTS = %s_Events\n", new_helpstr, ptr_iso_parent->rp_prefix);
  
    while (event_object) {
      /* 
       * Write to help file.
       */
      if (cmdline_table[OPT_HELPFILE].is_enabled)
	fprintf(help_stream,"%s EVENTS %s = %s\n", new_helpstr,
		event_object->event_name, event_object->event_symbol);
      /*
       ** Write MSL 
       */
      fprintf(ms_stream,"%s    EVENT %s = %d : \n",new_ms_indent_str,event_object->event_name,
	      event_object->event_code);
      event_oid_encode (entity_node, event_object->event_code, oid_string);
      fprintf(ms_stream,"%s      SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s      DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      SYMBOL = %s,\n",new_ms_indent_str, event_object->event_symbol);
      fprintf(ms_stream,"%s      TEXT = \"A %s trap was received:\",\n",
	      new_ms_indent_str,event_object->event_name);
      fprintf(ms_stream,"%s      CATEGORIES = (CONFIGURATION),\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      ARGUMENT enterprise = 01 : OctetString\n",new_ms_indent_str);
      event_arg_pdu_oid_encode (entity_node, prefix_event_enterprise_arg, event_object->event_code, oid_string);
      fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s          SYMBOL = SNMP_EV_GEN_ENTERPRISE\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      END ARGUMENT enterprise;\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      ARGUMENT agent_addr = 02 : IpAddress\n",new_ms_indent_str);
      event_arg_pdu_oid_encode (entity_node, prefix_event_agent_addr_arg, event_object->event_code, oid_string);
      fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s          SYMBOL = SNMP_EV_GEN_AGENT_ADDR\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      END ARGUMENT agent_addr;\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      ARGUMENT generic_trap = 03 : INTEGER\n",new_ms_indent_str);
      event_arg_pdu_oid_encode (entity_node, prefix_event_generic_trap_arg,event_object->event_code, oid_string);
      fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s          SYMBOL = SNMP_EV_GEN_GENERIC_TRAP\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      END ARGUMENT generic_trap;\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      ARGUMENT specific_trap = 04 : INTEGER\n",new_ms_indent_str);
      event_arg_pdu_oid_encode (entity_node, prefix_event_specific_trap_arg, event_object->event_code, oid_string);
      fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s          SYMBOL = SNMP_EV_GEN_SPECIFIC_TRAP\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      END ARGUMENT specific_trap;\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      ARGUMENT time_stamp = 05 : TimeTicks\n",new_ms_indent_str);
      event_arg_pdu_oid_encode (entity_node, prefix_event_time_stamp_arg, event_object->event_code, oid_string);
      fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string);
      fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
      fprintf(ms_stream,"%s          SYMBOL = SNMP_EV_GEN_TIME_STAMP\n",new_ms_indent_str);
      fprintf(ms_stream,"%s      END ARGUMENT time_stamp;\n",new_ms_indent_str);
    
      arg_code = 5;
      event_argument = event_object->event_argument_list;
      while (event_argument) {
	char oid_string_1[SIZE_OF_IDENTIFIER];
	arg_code++;
	syntax_to_string(event_argument->syntax, event_argument->typedef_name, datatype_string);
	fprintf(ms_stream,"%s      ARGUMENT %s = %d : %s\n",
		new_ms_indent_str, event_argument->arg_name, arg_code, datatype_string);
	event_arg_varbind_oid_encode (entity_node, event_object->event_code, &event_argument->oid, oid_string_1);
	fprintf(ms_stream,"%s          SNMP_OID = %s,\n", new_ms_indent_str, oid_string_1);
	fprintf(ms_stream,"%s          DISPLAY = TRUE,\n",new_ms_indent_str);
	fprintf(ms_stream,"%s          SYMBOL = %s\n", new_ms_indent_str,event_argument->symbol);
	fprintf(ms_stream,"%s      END ARGUMENT %s;\n",
		new_ms_indent_str,event_argument->arg_name);
	event_argument = event_argument->next_arg;
      }
      fprintf(ms_stream,"%s    END EVENT %s;\n\n",new_ms_indent_str,event_object->event_name);
      event_object = event_object->event_next;
    }	
  
    fprintf(ms_stream,"%s  END EVENT PARTITION CONFIGURATION EVENTS;\n\n",new_ms_indent_str);
    if (cmdline_table[OPT_DECMCC_MS].is_enabled)
      fprintf(ms_stream,"%s  INCLUDE mcc_tcpip_am_getevent.ms;\n\n",new_ms_indent_str);
  }
  
  if (entity_node->childlist != 0) { 
    k = entity_node->childlist;
    while ( k != 0) {
      TraverseTree(k,new_ms_indent_str,new_helpstr);
      k = k->nextsibling;
    }
  }

  fprintf(ms_stream,"%sEND ENTITY %s;\n\n",new_ms_indent_str,entity_node->object_name);
  return TRUE;
}

/****************************************************************************
  Produce Management Specification file
  ****************************************************************************/
int produce_ms_file
#ifdef PROTOTYPE_ALLOWED
  (T_ENTITY_NODE **ptr_entity_tree)
#else
(ptr_entity_tree)
T_ENTITY_NODE **ptr_entity_tree;
#endif

{
  char *ms_string = "";
  char help_string[SIZE_OF_IDENTIFIER];
  T_TYPEDEF_OBJECT *typedef_object;
  T_DATATYPE_OBJECT *datatype_object, *next_datatype_object;
  int mslcount = 0;
  /*
   ** Write MS file heading
   */
  fprintf (ms_stream, "MANAGEMENT SPECIFICATION CA_%s;\n", module_name);
  fprintf (ms_stream, "VERSION = %s;\n", mtu_version); 
  fprintf (ms_stream, "SYMBOL-PREFIX = CA_;\n"); 
  fprintf (ms_stream, "DEFINING-SMI = %s;\n\n", protocol_definition->protocol_smi);
  fprintf (ms_stream, "(* %s Definition*)\n", module_name);

/*
 ** Process list of typedef definitions, updating MS file.
 */
  if (typedef_object = typedef_object_list[0]) {
    do {
      datatype_object = typedef_object->first_dt;
      if (typedef_object->kind != TAG) {
	fprintf (ms_stream, "TYPE\n  %s = %d (\n", typedef_object->name, ++mslcount);
	do {
	   if (next_datatype_object = datatype_object->flink)
	     fprintf (ms_stream, "    %s = %u,\n",  datatype_object->name,  datatype_object->tag);
	   else
             fprintf (ms_stream, "    %s = %u);\n\n",  datatype_object->name,  datatype_object->tag);
	}
	while (datatype_object = next_datatype_object);
      }
      else 
	fprintf (ms_stream, "TYPE\n  %s = %d %s;\n\n", 
		 typedef_object->name, ++mslcount, datatype_object->name);
    }
    while (typedef_object = typedef_object->flink);
  }
/*
 ** Process entitytree, producing MS lines
 */
  sprintf (help_string, "ENTITY %s", protocol_definition->protocol_entity_parent);
  TraverseTree(*ptr_entity_tree, ms_string, help_string);
  fprintf(ms_stream, "END SPECIFICATION;\n");

/*
 ** Produce DECmcc help key body
 */
  if (cmdline_table[OPT_HELPFILE].is_enabled)
    produce_help_keylines(*ptr_entity_tree);
  return TRUE;
}  
