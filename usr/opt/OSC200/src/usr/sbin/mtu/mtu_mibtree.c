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
static char *rcsid = "@(#)$RCSfile: mtu_mibtree.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 22:18:44 $";
#endif
/* Title:	mtu_mibtree.c	Produce MIB parse tree */
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
 **  FACILITY:  PolyCenter Common Agent and DECmcc Framework -- MIB Translation Utility
 **
 **  MODULE DESCRIPTION:
 **
 **	The modules (mtu_lex, mtu_yacc, and mtu_mibtree) parse the input MIB file &
 **	store a MIB parse tree and typedef_object list.
 **
 **	mtu_mibtree provides the action rtns to mtu_yacc
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
 * 01	01-Mar-1991	Rahul Bose		    Created.
 * 02	02-Aug-1992	Peter Burgess	Modifications to support new MIR Compiler (CA)
 * 03	17-Sep-1992	Peter Burgess	CA FT2 updates:
 *					o c89 -std (ANSI C), I18n, restructured code for performance and extensibility
 *					o support for Rahul's Bose DECmcc v1.3 enhancments
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
 ** Local functions
 */
void FreeMIBTree PROTOTYPE ((T_MIB_OBJECT_NODE *node));

/*
 ** External data structures
 */

extern T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; 		/* ECA_MTU.C */
extern T_TYPEDEF_OBJECT *typedef_object_list[2];
extern T_MIB_OBJECT_NODE *mib_root;           	

extern FILE *yyout;                  					/* mtu_yacc.c */
extern int yynerrs, yylineno;
extern int is_valid_typedef_object;
extern T_TYPEDEF_OBJECT *typedef_object;  
extern T_MIB_OBJECT_NODE *node;	/* MIB Object */
extern T_TRAP_OBJECT *trap_object; 

/*
 ** Array of hash lists of MIB Object-nodes
 */
#define MIB_OBJECT_HASHTABLE_SIZE 100
typedef struct mib_object_list_hdr {
  T_MIB_OBJECT_NODE	*mib_object_flink;
  T_MIB_OBJECT_NODE	*mib_object_blink;
} MIB_OBJECT_T_LIST_HEAD;

static MIB_OBJECT_T_LIST_HEAD mib_object_list_head[MIB_OBJECT_HASHTABLE_SIZE];
static T_MIB_OBJECT_NODE *last_parent, *last_node;	/* Cache */

/* Set of ISO reserved names which can not be redefined */
static char * ptr_iso_reserved_names[] =   { "iso", "org", "dod", "internet", 
				  "directory", "mgmt", "experimental", "private",
				  "mib", "transmission", "enterprises" };
/** 				  "mib", "mib-2", "mib_2", "transmission", "enterprises" }; **/

/*  ISO Name	   Object identifier       Symbol prefix  Base MS code
**   ----------    -----------------       ------------   -------------*/
static T_NODE_SUMMARY_INFO known_parent_nodes[] = {
  {"mgmt", 	   {5, {1,3,6,1,2}},		"MG", 		0},
  {"mib_2",	   {6, {1,3,6,1,2,1}},		"MB", 		0},
  {"transmission", {7, {1,3,6,1,2,1,10}},	"TR", 		200},
  {"experimental", {5, {1,3,6,1,3}},		"EX", 		500},
  {"enterprises",  {6, {1,3,6,1,4,1}},		"EN", 		5000},
  {"enterprise",   {6, {1,3,6,1,4,1}},		"EN", 		5000},	/* Recover from Proteon typo */
};

T_NODE_SUMMARY_INFO *ptr_iso_parent;	/* Pointer to ISO parent */



/********************************************************************************
  hashtable_insert	- Insert MIB object node into hash list
********************************************************************************/
static int hashtable_insert
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *node)
#else
(node)
T_MIB_OBJECT_NODE *node;
#endif
{
  static int m = 0x0a53f19b7;
  unsigned int hk,i;
  T_MIB_OBJECT_NODE *object_ptr;
  int cmpval;

  /*
   ** Hash (mib object name) -> hash key
   */
  hk = 0;
  for (i=0; i < strlen(node->object_name); i++)
    hk = hk + (unsigned char)(*((node->object_name)+i));
  hk = (hk * m) % MIB_OBJECT_HASHTABLE_SIZE;

  /*
   ** If hashkey indexed listhead is empty,
   ** then insert mib object
   */
  if (!(object_ptr = mib_object_list_head[hk].mib_object_flink)) {
    mib_object_list_head[hk].mib_object_flink = node;
    mib_object_list_head[hk].mib_object_blink = node;
  }
  /*
   ** Else 
   **	Search list, to insure unique object name within parent hierarchy
   **   Insert mib object at end of list
   */
  else {
    do {
      if ((cmpval = strcmp(node->object_name, object_ptr->object_name)) == 0) {
	if ((strcmp(node->parent_name, object_ptr->parent_name)) == 0) {
	  fprintf (stdout, MSG(msg41, "MTU ERROR -- Duplicate object name (%s)\n"), node->object_name);
	  if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg41(), node->object_name);
	  DEBUG_BREAK;	/* EXCEPTION:  Duplicate object name */
	  MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
	  return FALSE;
	}
      }
      else if (cmpval < 0) {
	if (object_ptr->mib_object_blink) {
	  object_ptr->mib_object_blink->mib_object_flink = node;
	  node->mib_object_flink = object_ptr;
	  node->mib_object_blink = object_ptr->mib_object_blink;
	  object_ptr->mib_object_blink = node;
	  return TRUE;
	}
	else {	/* Insert node at list head */
	  mib_object_list_head[hk].mib_object_flink = node;
	  node->mib_object_flink = object_ptr;
	  object_ptr->mib_object_blink = node;
	  return TRUE;
	}
      }
    }
    while ((object_ptr = object_ptr->mib_object_flink));
	
    /* Insert node at end of hash list */
    mib_object_list_head[hk].mib_object_blink->mib_object_flink = node;
    node->mib_object_blink = mib_object_list_head[hk].mib_object_blink;
    mib_object_list_head[hk].mib_object_blink = node;
  }
  return TRUE;
}

/********************************************************************************
  hashtable_find_node	- Find MIB object node in MIB tree
********************************************************************************/
int hashtable_find_node
#ifdef PROTOTYPE_ALLOWED
  (char *name, T_MIB_OBJECT_NODE **ptr_node)
#else
(name, ptr_node)

char *name;
T_MIB_OBJECT_NODE **ptr_node;
#endif

{
  static int m = 0x0a53f19b7;
  unsigned int hk, i;
  T_MIB_OBJECT_NODE *object_ptr;
  int cmpval;
  /*
   ** Check simple cache (last_parent, last_node)
   */
  if ((strcmp (last_parent->object_name, name)) == 0) { 
    *ptr_node = last_parent;
    return TRUE;
  }

  if ((strcmp (last_node -> object_name, name)) == 0) {
    *ptr_node = last_node;
    return TRUE;
  }

  /*
   ** Hash object name -> hash key
   */
  hk = 0;
  for (i=0; i < strlen(name); i++)
    hk = hk + (unsigned char)(*(name+i));
  hk = (hk * m) % MIB_OBJECT_HASHTABLE_SIZE;
  

  /*
   ** Search hashedkey indexed listhead for object-name
   */
  if (!(object_ptr = mib_object_list_head[hk].mib_object_flink)) {
    return FALSE;
  }
  do 
    if ((cmpval = strcmp(object_ptr->object_name, name)) == 0) {
      *ptr_node = object_ptr;
      return TRUE; /* Success */
    }
    else if (cmpval > 0)
      return FALSE;
  while ((object_ptr = object_ptr->mib_object_flink));

  return FALSE;
}

/****************************************************************************
  AddNode	- Add an object to the mib tree.
  ****************************************************************************/
int AddNode
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *node)
#else
(node)
T_MIB_OBJECT_NODE *node;
#endif

{
  T_MIB_OBJECT_NODE *parent_node;
  int i, is_found;

/*
 ** If MIB tree is empty, then place node at root
 ** else insert node under parent in MIB tree.
 **
 ** All mib object nodes are inserted into a hashtable list
 ** for rapid access by object-name.
 */

  if (mib_root == 0) { 
    /*
     * Ignore attempts to (re)define special `ISO registered objects'
     */
    for (i=0; i < sizeof(ptr_iso_reserved_names)/sizeof(char *); i++)
      if  ((strcmp(node->object_name, *(ptr_iso_reserved_names+i))) == 0) return TRUE;
    
    /*
     ** Place node at MIB root, insert node into hashtable.
     */
    mib_root = node;       
    memset(mib_object_list_head, 0, sizeof(mib_object_list_head));
    hashtable_insert(node);
    last_parent = last_node = node;	/* cache the node pointer - lookup hint */
    
    /* 
     ** Locate parent object among set of predefined ISO objects.
     **	o Create node OID (concatenate parent OID, child arc)
     **	o Retrieve symbol_prefix and start_code of the ISO object
     */
    is_found = FALSE;
    for (i=0; i < sizeof(known_parent_nodes)/sizeof(T_NODE_SUMMARY_INFO); i++) {
      if  ((strcmp(node->parent_name, (known_parent_nodes+i)->rp_name)) == 0) {
	node->oid = (known_parent_nodes+i)->rp_oid;
	node->oid.tree[node->oid.entries++] = node->objectnumber;

	ptr_iso_parent = known_parent_nodes+i;
	is_found = TRUE;
      }
    }
    
    /* 
     ** If failed to locate parent ISO object, then report error.
     */
    if (!(is_found)) {
      fprintf(stdout, MSG(msg42, "MTU ERROR -- Parent of root object (%s) is %s.\n\tParent of root must be from this set:\n"), 
	      node->object_name, node->parent_name);
      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf(yyout, msg42(), node->object_name, node->parent_name);
      for (i=0; i < sizeof(known_parent_nodes)/sizeof(T_NODE_SUMMARY_INFO); i++) {
	fprintf(stdout, "\t%s\n", (known_parent_nodes+i) ->rp_name);
	if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf(yyout, "\t%s\n", (known_parent_nodes+i) ->rp_name);
      }
      DEBUG_BREAK;
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return FALSE;
    }
  } 

  /*
   ** Else MIB tree is not empty.  
   ** 	Insert object into hash table
   **   Insert object into MIB tree, under parent object.
   */
  else {
    /*
     ** Insert mib-object into hash table,
     ** EXCEPTION:  duplicate object name
     */
    if (!(hashtable_insert (node))) { 
      DEBUG_BREAK;	/* EXCEPTION:  Duplicate object name */
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return FALSE;
    }

    /* 
     ** Insert node under parent node
     ** 	Search for parent node, using cached hints, or hashtable
     **	EXCEPTION case: parent node not found
     */
    
    if (!(hashtable_find_node (node->parent_name, &parent_node))) {
      fprintf (stdout, MSG(msg43, "MTU ERROR -- Parent (%s) of object (%s) is not defined\n"), node->parent_name, node->object_name);
      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg43(), node->parent_name, node->object_name);
      DEBUG_BREAK;	/**	EXCEPTION case: parent node not found */
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return FALSE;
    }
    last_parent = parent_node;	/* Remember last parent */
    last_node = node;	/* Remember this node */

    node->parent = parent_node;	/* Store child's pointer to parent */
    node->oid = parent_node->oid;	/* Store OID */
    node->oid.tree[node->oid.entries++] = node->objectnumber;

    /* Insert child node under parent node */
    if (parent_node->childlist == NULLPTR) {
      parent_node->childlist = node; 
      parent_node->lastchild = node; 
    }
    else { 
      parent_node->lastchild->nextsibling = node;
      parent_node->lastchild = node; 
    }
  } 
  return TRUE;
}    

/*********************************************************************
 AddTrapObject -   Add a trap object to the mib tree.
 ********************************************************************/

int AddTrapObject
#ifdef PROTOTYPE_ALLOWED
  (T_TRAP_OBJECT *trap_object)
#else
(trap_object)
T_TRAP_OBJECT *trap_object;
#endif
{
  T_TRAP_VAR *trap_var;
  T_MIB_OBJECT_NODE *mib_object, *enterprise_object;
  
  /*
   ** TrapObjects are children of an 'enterprise object'
   ** 	Locate Enterprise mib object, then
   ** 	Insert TrapObject at head of its TrapList. (LIFO list)
   */
  if (!(hashtable_find_node (trap_object->enterprise_name, &enterprise_object))) {
    fprintf (stdout, MSG(msg44, "MTU ERROR -- Enterprise(%s) in trap object (%s) is not defined\n"),
	     trap_object->enterprise_name, trap_object->trap_name);
    if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg44(), trap_object->enterprise_name, trap_object->trap_name);
    DEBUG_BREAK;
/*    free(trap_object); */
    MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
    return FALSE;
  }
  /* Push trap object onto stack */
  trap_object->trap_next = enterprise_object->trap_object_list;
  enterprise_object->trap_object_list = trap_object;
  
  /*
   ** Process Trap Variables on LIFO list (stack)
   */
  trap_var = trap_object->trap_var_list;
  while (trap_var) {
    /* Verify that trap variable is a valid mib object */
    if (!(hashtable_find_node (trap_var->trapvar_name, &mib_object))) {
      fprintf (stdout,  MSG(msg45, "MTU ERROR -- Variable (%s) in trap object (%s) is not defined\n"),
	       trap_var->trapvar_name, trap_object->trap_name);
      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg45(), trap_var->trapvar_name, trap_object->trap_name);
      DEBUG_BREAK;
/*    free(trap_object); */
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return FALSE;
    }
    
#ifdef NOWAY_JOSE
    /*
     ** Verify that TrapVariable is a leaf object. (dubious test)
     */
    if ((mib_object->childlist != 0) || (mib_object->object != OBJECT_IDENTIFIER)) {
      fprintf (stdout, MSG(msg46, "MTU ERROR -- Variable (%s) in trap object (%s) is not a leaf object\n"),
	       trap_var->trapvar_name, trap_object->trap_name);
      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg46(), trap_var->trapvar_name, trap_object->trap_name);
      DEBUG_BREAK;
/*    free(trap_object); */
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return FALSE;
    }
#endif
    
    /* Store pointer to mib object in TrapVar */
    trap_var->trapvar_mib_object = mib_object;
    
    /*
     ** Retrieve pointer to next TrapVaraiable
     */
    trap_var = trap_var->trapvar_next;
  }	
  return TRUE;
}

/***********************************************************************
  AddTrapVar	- Add a variable to the trap object
***************************************************************************/
void AddTrapVar 
#ifdef PROTOTYPE_ALLOWED
  (T_TRAP_OBJECT *trap_object, char *trapvar_name)
#else
(trap_object, trapvar_name)
T_TRAP_OBJECT *trap_object;
char *trapvar_name;
#endif
{ 
  T_TRAP_VAR *trapvar;

  trapvar = (T_TRAP_VAR *)calloc(1, sizeof(T_TRAP_VAR));
  strcpy(trapvar->trapvar_name, trapvar_name);
  
  /* Push trap variable onto trap object stack of variables */
  trapvar->trapvar_next = trap_object->trap_var_list;
  trap_object->trap_var_list = trapvar;
}

/***********************************************************************
  add_node_index	- Add table index name to list
**********************************************************************/
void add_node_index 
#ifdef PROTOTYPE_ALLOWED
  (char *index_name)
#else
(index_name)
char *index_name;
#endif
{
  node->index[node->num_indices] = (char *)malloc(strlen(index_name)+1);
  strcpy (node->index[node->num_indices], index_name);
  node->num_indices++;
}

/*********************************************************************************
  void FreeTypeDefList 	- Free TypeDef list
********************************************************************************/

void FreeTypeDefList ()
{
  T_TYPEDEF_OBJECT *typedef_object, *next_typedef_object;
  T_DATATYPE_OBJECT *datatype_object, *next_datatype_object;

  next_typedef_object = typedef_object_list[0];
  while ((typedef_object = next_typedef_object)) {
    next_datatype_object = typedef_object->first_dt;
    while (datatype_object = next_datatype_object) {
      next_datatype_object = datatype_object->flink; 
      free(datatype_object);
    }
    next_typedef_object = typedef_object->flink;
    free(typedef_object);
  }
  typedef_object_list[0] = typedef_object_list[1] = 0;
}

/**********************************************************************
  add_typedef_object
**********************************************************************/
void add_typedef_object ()
{
  T_TYPEDEF_OBJECT *ptr_last;

  if (is_valid_typedef_object) {
    /*
     * Add TYPEDEF_OBJECT to list
     */
    if (ptr_last = typedef_object_list[1]) 
      typedef_object_list[1] = ptr_last->flink = typedef_object;
    else 
      typedef_object_list[0] = typedef_object_list[1] = typedef_object;
    
    typedef_object = (T_TYPEDEF_OBJECT *)malloc(sizeof(T_TYPEDEF_OBJECT));
  }
  memset(typedef_object, 0, sizeof(T_TYPEDEF_OBJECT));
  is_valid_typedef_object = TRUE;
}

/***********************************************************************
  add_datatype_object	- Add datatype object to typedef object
**********************************************************************/
void add_datatype_object
#ifdef PROTOTYPE_ALLOWED
  (T_TYPEDEF_OBJECT *typedef_object, int tag, char * datatype_name)
#else
  (typedef_object, tag, datatype_name)
T_TYPEDEF_OBJECT *typedef_object;
int tag;
char * datatype_name;
#endif
{
  T_DATATYPE_OBJECT *datatype_object, *ptr_last;
  
  datatype_object = (T_DATATYPE_OBJECT *)calloc(1, sizeof(T_DATATYPE_OBJECT));
  strcpy (datatype_object->name, datatype_name);
  datatype_object->tag = tag;
  if (ptr_last = typedef_object->last_dt) 
    typedef_object->last_dt = ptr_last->flink = datatype_object;
  else 
    typedef_object->first_dt = typedef_object->last_dt = datatype_object;
}

/***************************************************************************
   FreeMIBTree	- Deallocate MIB tree to reclaim dynamic memory storage.
      Deallocation is performed in a recursive bottom-up manner.
***************************************************************************/
void FreeMIBTree
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *node)
#else
(node)
T_MIB_OBJECT_NODE *node;
#endif
{
  T_MIB_OBJECT_NODE *next_node;
  T_TRAP_OBJECT *trap_object, *next_trap_object;
  T_TRAP_VAR *trap_var, *next_trap_var;
  int i;

/* Request deallocation of child MIB object node/branch */
  if (next_node = node->childlist)
    FreeMIBTree(next_node);

/* Request deallocation of sibling MIB object node/branch */
  if (next_node = node->nextsibling)
    FreeMIBTree(next_node);

/* Free trap list */
/* Free trap objects */
  for (trap_object = node->trap_object_list; trap_object != NULLPTR; trap_object = next_trap_object) {
    for (trap_var = trap_object->trap_var_list; trap_var != NULLPTR; trap_var = next_trap_var) {
      next_trap_var = trap_var->trapvar_next;
      free(trap_var);
    }
    next_trap_object = trap_object->trap_next;
    if (trap_object->description)
      free (trap_object->description);
    free (trap_object);
  }
/* Free index strings */
  for (i = 0; i<node->num_indices; i++)
    free(node->index[i]);

/* Conditionally free Description string */
  if (node->description)
    free (node->description);

/* Free object node */
  free(node);
}


/***************************************************************************************
  init_mibtree	- initialize the MIB tree, the TypeDEF list, the hash table headers,...
****************************************************************************************/
void  init_mibtree()
{
  yylineno = yynerrs = 0;
  if (mib_root) {
    FreeMIBTree(mib_root);
    free(node);
    free(trap_object);
    mib_root = 0;
    FreeTypeDefList();
    free(typedef_object);
    memset (mib_object_list_head, 0, sizeof(mib_object_list_head));
    last_parent = last_node = 0;
  }
}
