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
 * @(#)$RCSfile: mtu.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:27:26 $
 */
/*  Title:	mtu.h	- MIB Translation Utility Header File
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
 **  FACILITY:  PolyCenter Common Agent & DECmcc -- MIB Translator Utility
 **
 **  MODULE DESCRIPTION:
 **
 **      Private header file for MTU
 **
 **  AUTHORS:
 **
 **      Rahul Bose
 **
 **  CREATION DATE:  1-March-1991
 **
 **  MODIFICATION HISTORY:
 **
 **
 ** Edit#     Date	 By			    Description
 ** -----  ------------	----		----------------------------------
 **
 ** 01	  01-Mar-1991	Rahul Bose		    Created.
 ** 02	  14-Sep-1992   Pete Burgess		    Common Agent updates.
 */


#define DEFAULT_ENTITY_START_CODE 1
#define ERROR_THRESHOLD 50
#define MAX_ENTITY_LEVELS 7
#define MAX_ENTITY_START_CODE 	60000
#define MAX_INDICES 16
#define MAX_NUM_CONST_COMPS 10
#define PAGE_SIZE 55
#define PAGE_WIDTH 132
#define SIZE_OF_DESCRIPTION 4096
#define SIZE_OF_FILENAME 128
#define SIZE_OF_IDENTIFIER 128
#define SNMP_MAX_TREE_ENTRIES 26

#define FALSE 0
#define TRUE 1
#define NUL '\0'

#if __vaxc
# define NULLPTR 0
#else
# define NULLPTR (void *)0
#endif

#ifdef __STDC__
# define PROTOTYPE_ALLOWED 1
#endif

#ifdef PROTOTYPE_ALLOWED
# define PROTOTYPE(arg) arg
#else
# define PROTOTYPE(arg) ()
#endif

#define MSG(msg_name, string)  msg_name()

#ifdef DEBUG
# define DEBUG_BREAK {extern void debug_break(); debug_break ();}
#else 
# define DEBUG_BREAK
#endif

#define MIB_ERROR \
{extern jmp_buf mib_env; extern int mib_error_count; if (++mib_error_count>ERROR_THRESHOLD) longjmp(mib_env, 1);}

typedef enum { MTU_M_DEBUG = 1, MTU_M_FAKE_VM = 2 } T_MTU_LOG_BITS;

typedef struct	s_oid {
  unsigned int	entries;
  unsigned int	tree[SNMP_MAX_TREE_ENTRIES];
} T_OID;

/* Table of protocol specific attributes */
typedef struct s_protocol_def {
  char *protocol_smi;
  char *protocol_entity_parent;
  T_OID protocol_entity_parent_oid;
} T_PROTOCOL_DEF;

/* Structure for well-known internet names */
typedef struct node_summary_info {
  char 	*rp_name;
  T_OID	rp_oid;
  char	*rp_prefix;
  int	rp_ms_code;
} T_NODE_SUMMARY_INFO;

typedef enum {AGENT_BACKEND, DIRECTOR_BACKEND } T_BACKEND_TYPES;
typedef enum {SNMP_PROTOCOL, DNA_CMIP_PROTOCOL } T_PROTOCOL_TYPES;
typedef enum {READONLY,READWRITE,WRITEONLY,NOTACCESSIBLE,UNKNOWN_ACCESS} T_ACCESS;
typedef enum {MANDATORY,OPTIONAL,OBSOLETE,DEPRECATED,UNKNOWN_STATUS} T_STATUS;
typedef enum {ENUM,INTEGER,OCTETSTRING,NETWORKADDRESS,COUNTER,GAUGE,TIMETICKS,
		OPAQUE,IPADDRESS,OBJECTID,SEQUENCE,SEQUENCE_OF,DEFINED_SYNTAX,
		DISPLAYSTRING,UNKNOWN_SYNTAX} T_SYNTAX;
typedef enum {OBJECT_IDENTIFIER, OBJECT_TYPE} T_OBJECT;

/*
 * Structure represents the parsed MIB object (OBJECT-TYPE | OBJECT-IDENTIFIER)
 * 	Produced during YACC (yyparse()) parsing phase.
 *	Inserted into `mib_tree' by mtu_mibtree.c:AddNode
 */
typedef struct s_mib_object_node {
  T_OBJECT object;       	  	/* OBJECT_IDENTIFIER or OBJECT_TYPE */
  char object_name[SIZE_OF_IDENTIFIER]; /* Object Name */
  char parent_name[SIZE_OF_IDENTIFIER]; /* Parent Name */
  int objectnumber;               	/* Object Number */
  struct s_mib_object_node *parent;    	/* ptr to parent MIB parse object */
  char typedef_name[SIZE_OF_IDENTIFIER];/* Syntax Typedef Name if type is not one of the pre-defined types */
  T_SYNTAX syntax;                  	/* Mib syntax for object */
  T_ACCESS access;                  	/* Mib access for object */
  T_STATUS status;                  	/* Mib status for object */
  unsigned int num_indices;       	/* # of Table indices */
  char *index[MAX_INDICES];             /* List of ptrs to table indices */
  unsigned int is_index;	  	/* Is Object referenced as an index? */
  T_OID oid;		         	/* Object Identifier */
  char *description;			/* Pointer to Description */
  struct s_mib_object_node *mib_object_flink; 	/* Forward pointer to next mib-object */
  struct s_mib_object_node *mib_object_blink; 	/* Backward pointer to mib-object */
  struct s_mib_object_node *childlist;      	/* Pointer to next child mib-object */
  struct s_mib_object_node *lastchild;      	/* Pointer to last child mib-object */
  struct s_mib_object_node *nextsibling;    	/* Pointer to next sibling mib-object */
  struct s_trap_object *trap_object_list; /* Pointer to TrapObjects */
} T_MIB_OBJECT_NODE;

/*
 * Structure used in entity model tree to represent child entity.
 *	Inserted into `entity_tree' by mtu_entitytree.c:add_entity
 */
 
typedef struct s_entity_node {
  char object_name[SIZE_OF_IDENTIFIER];	/* Object name */
  char parent_name[SIZE_OF_IDENTIFIER]; /* Parent name */
  int objectnumber;                   	/* Object number */
  T_OID oid;		                /* Object Identifier*/
  int entitycode;                     	/* MS entity code & DNA_CMIP_INT */
  char symbol[SIZE_OF_IDENTIFIER];	/* MS symbol */
  int is_global_entity;			/* Boolean:  is_global_entity? */
  char *description;			/* Pointer to Description */
  struct s_entity_node *parent;         /* pointer to parent entity node*/
  struct s_entity_node *childlist;      /* pointer to next child */
  struct s_entity_node *lastchild;      /* pointer to next child */
  struct s_entity_node *nextsibling;    /* pointer to next sibling */
  struct s_entity_attr_node *identifier_list;    /* pointer to list of identifier */
  struct s_entity_attr_node *identifier_last;    /* pointer to list of identifier */
  struct s_entity_attr_node *status_list;        /* pointer to list of status attr*/
  struct s_entity_attr_node *status_last;        /* pointer to list of status attr*/
  struct s_entity_attr_node *counter_list;	/* pointer to list of counter attr*/
  struct s_entity_attr_node *counter_last;       /* pointer to list of counter attr*/
  struct s_entity_attr_node *characteristic_list;  /* pointer to list of char attr*/
  struct s_entity_attr_node *chararteristic_last;  /* pointer to list of char attr*/
  struct s_event_object *event_list;       /* List of events */
} T_ENTITY_NODE;

/*
 * Structure used in entity model tree to repesent entity-attribute.
 *	Inserted into `entity_tree' by mtu_entitytree.c:add_entity_attribute
 */

typedef struct s_entity_attr_node {
  char object_name[SIZE_OF_IDENTIFIER];	/* Object name */
  char parent_name[SIZE_OF_IDENTIFIER]; /* Parent name */
  int objectnumber;                     /* Object number */
  T_OID oid;               		/* Object Identifier */
  struct s_entity_node *parent;         /* pointer to parent */
  char symbol[SIZE_OF_IDENTIFIER];	/* MS symbol */
  char typedef_name[SIZE_OF_IDENTIFIER];/* Name of pre-defined syntax type */
  T_SYNTAX syntax;                      /* MIB syntax for object */
  T_ACCESS access;                      /* MIB access for object */
  T_STATUS status;              	/* MIB status for object */
  unsigned int is_identifier; 	        /* Is attribute an identifier */
  char *description;			/* Pointer to Description */
  struct s_entity_attr_node *nextsibling;  /* pointer to next sibling */
} T_ENTITY_ATTRIBUTE_NODE;

/*
 ** Structure used in typedef list to represent a parsed datatype object.
 **	Produced during YACC parse phase.
 */
typedef struct s_datatype_object {
  int tag;
  char name[SIZE_OF_IDENTIFIER];
  struct s_datatype_object *flink;
} T_DATATYPE_OBJECT;


/*
 ** Structure used in typedef list to represent a parsed typedef object.
 **	Produced during YACC parse phase.
 */
typedef enum {ENUMERATION, TAG} T_TYPEDEF_KINDS;
typedef struct s_typedef_object {
  T_TYPEDEF_KINDS kind;	/* Kinds of typdef objects*/
  struct s_typedef_object *flink;
  char name[SIZE_OF_IDENTIFIER];
  T_DATATYPE_OBJECT *first_dt;
  T_DATATYPE_OBJECT *last_dt;
} T_TYPEDEF_OBJECT;

/*
 ** Event Object - Represents Entity Event object
 **	Produced by mtu_entitytree:AddEntityNode
 */
typedef struct s_event_object {
  char event_name[SIZE_OF_IDENTIFIER];		/* Event Name */
  unsigned int event_code;			/* Event Code */
  char enterprise_name[SIZE_OF_IDENTIFIER]; 	/* Enterprise name */
  char event_symbol[SIZE_OF_IDENTIFIER];	/* Event MSL symbol */
  T_OID event_oid;				/* Event OID */
  struct s_event_object *event_next;		/* Pointer to next event-type object */
  struct s_event_argument *event_argument_list;	/* Event argument list */
  char *description;				/* Pointer to Description */
} T_EVENT_OBJECT;

/*
 ** Event Argument
 */
typedef struct s_event_argument {
  char arg_name[SIZE_OF_IDENTIFIER];		/* Argument Name */
  char symbol[SIZE_OF_IDENTIFIER];		/* Argument Symbol */
  char typedef_name[SIZE_OF_IDENTIFIER];    	/* Typedef Name if type is not predefined */
  T_SYNTAX syntax;                     		/* Mib syntax for object */
  T_ACCESS access;                     		/* Mib access for object */
  T_STATUS status;                     		/* Mib status for object */
  T_OID oid;			               	/* Object Identifier */
  struct s_event_argument *next_arg; 		/* Pointer to next event arg */
} T_EVENT_ARGUMENT;

/*
 * Parse of MIB TRAP-TYPE object
 */
typedef struct s_trap_object {
  char trap_name[SIZE_OF_IDENTIFIER];	/* Name of trap */
  unsigned int trap_number;		/* Trap Number */
  char enterprise_name[SIZE_OF_IDENTIFIER]; /* Enterprise name */
  struct s_trap_object *trap_next;	/* Pointer to next trap-type object */
  struct s_trap_var *trap_var_list;	/* Pointer to trap variable list */
  char *description;			/* Pointer to Description */
} T_TRAP_OBJECT;

/*
 * List of TRAP variables.
 */
typedef struct s_trap_var	{
  char trapvar_name[SIZE_OF_IDENTIFIER];/* Name of trap variable */
  struct s_trap_var *trapvar_next;	/* Pointer to next trap variable */
  T_MIB_OBJECT_NODE *trapvar_mib_object;	/* Pointer to MIB object-type */
} T_TRAP_VAR;



/*
 ** Set of Command line parameters and options
 **	Produced by eca_mtu: get_cmdline
 */
typedef enum {		/* Set of command line grammar elements */
  OPT_MIB_FILENAME,	/* MIB file name */
  OPT_DNA_CMIP_PROTOCOL,/* Protocol = DNA CMIP */ 
  OPT_SNMP_PROTOCOL, 	/* Protocol = SNMP */
  OPT_HELPFILE, 	/* Produce DECmcc HELP files */
  OPT_INFOFILE, 	/* Produce CA MOMGEN information file */
  OPT_LISTFILE,		/* Produce Listing file? */
  OPT_ENTITY_CODE,	/* Entity code */
  OPT_AGENT_MS,		/* Client = AGENT */
  OPT_DECMCC_MS,	/* Client = DIRECTOR */
  N_CMDLINE_OPTIONS	/* Number of cmdline options - must be last */
  } T_CMDLINE_OPTION_SET;

/*
 ** Cmdline option table element
 **	For each possible grammar element there is a table element
 **	to represent its possible presence in the cmdline and optional value
 */
typedef struct s_cmdline_option {
  int	is_enabled;
  union {
    int option_int;
    char * option_string;
    T_PROTOCOL_DEF *option_protocol;
  } value;
} T_CMDLINE_OPTION;

