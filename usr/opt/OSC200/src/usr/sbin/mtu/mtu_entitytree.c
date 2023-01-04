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
static char *rcsid = "@(#)$RCSfile: mtu_entitytree.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 21:27:45 $";
#endif
/* Title: mtu_entitytree.c	Transform MIBTree to EntityTree */ 
/*
 **  Copyright (c) Digital Equipment Corporation, 1991, 1992, 1993
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
 **  FACILITY:  PolyCenter Common Agent/DECmcc Framework -- MIB Translation Utility
 **
 **  MODULE DESCRIPTION:
 **
 **	MTU_ENTITYTREE.C - Process MIBtree, producing EntityTree
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
 * 01	  01-Mar-1991	Rahul Bose		    Created.
 * 02     17-Aug-1992   Pete Burgess	Modifications for Common Agent FT. (new dictionary/MIR)
 * 03     26-Sep-1992	Pete Burgess	Decomposed MIBTREE.c into 3 modules (mib, entity, files)
 *					o c89 -std (ANSI C), Internalization, enterprise traps, DECmcc help files
 */

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtu.h"
#include "mtu_msg.h"

/*
 ** External Data structures
 */
extern T_ENTITY_NODE *entity_root;	        	/* Root of entity model tree */
extern T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; 
extern int hashtable_find_node PROTOTYPE ((char *name, T_MIB_OBJECT_NODE **ptr_node));
extern T_NODE_SUMMARY_INFO *ptr_iso_parent;
extern FILE *yyout;                  
extern T_TYPEDEF_OBJECT *typedef_object_list[2];	/* List head of typedef objects */
extern int mib_error_count;


/****************************************************************************
  Add a Child Entity Node to Parent Entity Node
  ****************************************************************************/

static T_ENTITY_NODE *AddEntityNode 
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *mib_node, T_ENTITY_NODE *parent_entity, T_ENTITY_NODE **ptr_entity_tree)
#else
(mib_node, parent_entity, ptr_entity_tree)
T_MIB_OBJECT_NODE *mib_node;
T_ENTITY_NODE *parent_entity, **ptr_entity_tree;
#endif
{
  int i;
  char symbol_name[SIZE_OF_IDENTIFIER];
  char objno_str[SIZE_OF_IDENTIFIER];
  char description_str[PAGE_WIDTH];
  T_ENTITY_NODE *entity_node;
  T_TRAP_OBJECT *trap_object;
  T_TRAP_VAR *trap_var;
  T_EVENT_OBJECT *event_object;
  T_EVENT_ARGUMENT *event_argument;
  /*
   ** Create Entity Node, Map MIB object fields to Entity Node fields
   */
  entity_node = (T_ENTITY_NODE *) calloc(1, sizeof(*entity_node));

  /* If global entity... */
  if (!(strcmp(mib_node->object_name, "mib_2"))) {
    entity_node->is_global_entity = TRUE;
    strcpy(entity_node->object_name, "SNMP");
    entity_node->parent = NULLPTR;
    strcpy(entity_node->parent_name, "");
  }
  else {
    entity_node->is_global_entity = FALSE;
    if ((parent_entity == NULLPTR) && 
	(!(strncmp(ptr_iso_parent->rp_prefix, "EX",2)))) {
      sprintf (entity_node->object_name, "EXP_%s", mib_node->object_name);
    }
    else {
      strcpy(entity_node->object_name,mib_node->object_name);
    }
    strcpy(entity_node->parent_name,mib_node->parent_name);
  }
  entity_node->objectnumber = mib_node->objectnumber;
  entity_node->oid = mib_node->oid;
  entity_node->entitycode = mib_node->objectnumber;
  strcpy (entity_node->symbol, ptr_iso_parent->rp_prefix);
  for (i = ptr_iso_parent->rp_oid.entries; i < entity_node->oid.entries; i++) {
    sprintf(objno_str,"%d_",entity_node->oid.tree[i]);
    strcat(entity_node->symbol, objno_str);
  }
  entity_node->symbol[strlen(entity_node->symbol)-1] = NUL;

  if (mib_node->description) {
    entity_node->description = (char *)malloc(strlen(mib_node->description)+1);
    strcpy(entity_node->description, mib_node->description);
  }
  else {
    if (entity_node->is_global_entity)
      sprintf (description_str, MSG (msg72, "Global Entity %s"), entity_node->object_name);
    else 
      sprintf (description_str, MSG (msg70, "Child Entity %s"), entity_node->object_name);
    entity_node->description = (char *)malloc(strlen(description_str)+1);
    strcpy(entity_node->description, description_str);
  }
    
  /* 
   ** If Parent Entity is null, then let root = child_entity
   */
  if (parent_entity == NULLPTR) {
    
    if (cmdline_table[OPT_ENTITY_CODE].is_enabled)
      entity_node->entitycode = cmdline_table[OPT_ENTITY_CODE].value.option_int;
    else 
      entity_node->entitycode = entity_node->entitycode + ptr_iso_parent->rp_ms_code;

    *ptr_entity_tree = entity_node;
  }
  /*
   ** Else insert child entity node at end of parent entity child list
   */
  else {
    entity_node->parent = parent_entity;
    if ((parent_entity)->childlist == 0) { /* Add node to beginning of child list */
      (parent_entity)->childlist = entity_node; 
      (parent_entity)->lastchild = entity_node; 
    }
    else {	
      (parent_entity)->lastchild->nextsibling = entity_node;
      (parent_entity)->lastchild = entity_node;
    }
  }

  /*
   ** Process list of trap-objects, producing event-objects
   **	NOTE:  trap-objects were inserted onto a LIFO list (Stack),
   **	       so we can quickly pop from source list and push onto target list.
   */
  
  trap_object = mib_node->trap_object_list;
  while (trap_object) {
    /* 
     ** Transform trap-object to event_object
     **		o Allocate event_object
     **		o Map fields
     **		o Transform trap-variables to event-arguments
     **		o Insert event_object onto entity's list of events
     */

    /* Allocate event-object */
    event_object = (T_EVENT_OBJECT *) calloc (1, sizeof(T_EVENT_OBJECT));

    strcpy(event_object->event_name, trap_object->trap_name);
    event_object -> event_code = trap_object->trap_number;
    strcpy(event_object->enterprise_name, trap_object->enterprise_name);
    sprintf (event_object->event_symbol, "%s_Event_%d", entity_node->symbol, event_object->event_code);

    if (trap_object->description) {
      event_object->description = (char *)malloc(strlen(trap_object->description)+1);
      strcpy(event_object->description, trap_object->description);
    }
    else {
      sprintf (description_str, "Event for Entity %s", mib_node->object_name);
      event_object->description = (char *)malloc(strlen(description_str)+1);
      strcpy(event_object->description, description_str);
    }

    /*
    ** Transform list of trap variables, producing list of event arguments
    */
    trap_var = trap_object->trap_var_list;
    while (trap_var) {
      event_argument = (T_EVENT_ARGUMENT *)calloc(1, sizeof(T_EVENT_ARGUMENT));
      strcpy(event_argument->arg_name, trap_var->trapvar_name);
      strcpy(event_argument->typedef_name, trap_var->trapvar_mib_object->typedef_name);
      event_argument->syntax = trap_var->trapvar_mib_object->syntax;
      event_argument->access = trap_var->trapvar_mib_object->access;
      event_argument->status = trap_var->trapvar_mib_object->status;
      event_argument->oid =  trap_var->trapvar_mib_object->oid;

      /* Form MSL symbol-name */
      strcpy (symbol_name, ptr_iso_parent->rp_prefix);
      for (i = ptr_iso_parent->rp_oid.entries; i < event_argument->oid.entries; i++) {
	sprintf(objno_str,"%d_", event_argument->oid.tree[i]);
	strcat(symbol_name,objno_str);
      }
      sprintf(event_argument->symbol, "%sEV", symbol_name);

      /* Insert event argument onto list */
      event_argument->next_arg =  event_object->event_argument_list;
      event_object->event_argument_list = event_argument;

      trap_var = trap_var->trapvar_next;
    }

    /* Insert entity object onto entity list */
    event_object->event_next = entity_node->event_list;
    entity_node->event_list = event_object;

    trap_object = trap_object->trap_next;	/* Retrieve pointer to next trap-object */
  }

  return(entity_node);
}    

/****************************************************************************
  AddEntityAttr_node -- Create Entity Attribute node, updating Entity node attribute partition list
  ****************************************************************************/

static T_ENTITY_ATTRIBUTE_NODE *AddEntityAttr_node
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *mib_node, T_ENTITY_NODE *entity_node)
#else
(mib_node, entity_node)
T_MIB_OBJECT_NODE *mib_node;
T_ENTITY_NODE *entity_node;
#endif
{
  T_ENTITY_ATTRIBUTE_NODE *entity_attribute_node;
  int i;
  unsigned char typedef_found;
  T_ENTITY_ATTRIBUTE_NODE **partition_list_ptr, *next_attribute_node;
  T_TYPEDEF_OBJECT *typedef_object;
  char symbol_name[SIZE_OF_IDENTIFIER];
  char objno_str[SIZE_OF_IDENTIFIER];
  char description_str[PAGE_WIDTH];

  /* 
   ** Assert entity attribute has an owning entity
   */
  if (!(entity_node)) {
    fprintf (stdout, MSG(msg36, "MTU ERROR -- Invalid table structure (%s, %s)\n"),
	     mib_node->parent->object_name, mib_node->object_name);
    if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg36(), mib_node->parent->object_name, mib_node->object_name);
    DEBUG_BREAK;
    MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
    return NULLPTR;
  }

  /* 
   ** Assert entity attribute is not constructor syntax
   */
  if ((mib_node->syntax == SEQUENCE_OF) || (mib_node->syntax == SEQUENCE)) {
    fprintf (stdout, MSG(msg21, "MTU ERROR -- Invalid MIB constructed syntax for object (%s)\n"), mib_node->object_name);
    if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout,  msg21(), mib_node->object_name);
    DEBUG_BREAK;
    MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
    return NULLPTR;
  }

  /*
   ** Assert mib object does not have any traps
   */
  if (mib_node->trap_object_list) {
    fprintf (stdout, MSG(msg22, "MTU ERROR -- Invalid Object (%s) for trap enterprise (%s)\n"), mib_node->object_name, 
	     mib_node->trap_object_list->trap_name);
    if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout,  msg22(), mib_node->object_name, mib_node->trap_object_list->trap_name);
    DEBUG_BREAK;
    MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
    return NULLPTR;
  }

  /* 
   ** Allocate entity attribute node
   ** Initialize fields
   ** Map mib-node fields to entity attribute fields
   ** Add EntityAttribute node to Entity node's attribute partition lists
   */
  entity_attribute_node = (T_ENTITY_ATTRIBUTE_NODE *)calloc(1, sizeof(*entity_attribute_node));
  
  strcpy(entity_attribute_node->object_name, mib_node->object_name);
  strcpy(entity_attribute_node->parent_name, mib_node->parent_name);
  entity_attribute_node->objectnumber = mib_node->objectnumber;
  entity_attribute_node->oid = mib_node->oid;
  strcpy (symbol_name, ptr_iso_parent->rp_prefix);
  for (i = ptr_iso_parent->rp_oid.entries; i < entity_attribute_node->oid.entries; i++) {
    sprintf(objno_str,"%d_",entity_attribute_node->oid.tree[i]);
    strcat(symbol_name,objno_str);
  }
  symbol_name[strlen(symbol_name)-1] = NUL;	/* Remove trailing "_" */
  if (mib_node->is_index) 
    strcat(symbol_name, "_ID");
  strcpy (entity_attribute_node->symbol, symbol_name);
  strcpy(entity_attribute_node->typedef_name, mib_node->typedef_name);
  entity_attribute_node->syntax = mib_node->syntax;
  entity_attribute_node->access = mib_node->access;
  entity_attribute_node->status = mib_node->status;
  entity_attribute_node->is_identifier = mib_node->is_index;
  
  /*
   ** Process DEFINED_SYNTAX / typedef_name
   **	ASSERT defined syntax reference must be defined
   */
  if (entity_attribute_node->syntax == DEFINED_SYNTAX) {
    if ((strlen (entity_attribute_node->typedef_name)) == 0) {
      fprintf (stdout, MSG(msg23, "MTU ERROR -- Internal logic error 10 (%s)\n"), mib_node->object_name);
      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout,  msg23(), mib_node->object_name);
      DEBUG_BREAK;
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return NULLPTR;
    }
    /* Search list of typedef definitions */
    typedef_found = FALSE;
    if (typedef_object = typedef_object_list[0]) {
      do {
	if (!(strcmp(entity_attribute_node->typedef_name, typedef_object->name))) {
	  typedef_found = TRUE; break;
	}
      }
      while (typedef_object = typedef_object->flink);
    }
    if (!(typedef_found)) {
      fprintf(stdout,MSG(msg24, "MTU ERROR -- Undefined typedef (%s) for object (%s) \n"), 
	      entity_attribute_node->typedef_name, entity_attribute_node->object_name);    
     if (cmdline_table[OPT_LISTFILE].is_enabled)  fprintf(yyout, msg24(), entity_attribute_node->typedef_name, entity_attribute_node->object_name);    
      DEBUG_BREAK;
      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      return NULLPTR;
    }
  } /* end if (syntax = DEFINED_SYNTAX) */
    
  if (mib_node->description) {
    entity_attribute_node->description = (char *)malloc(strlen(mib_node->description)+1);
    strcpy(entity_attribute_node->description, mib_node->description);
  }
  else {
    sprintf (description_str, MSG (msg71, "Entity Attribute %s"), mib_node->object_name);
    entity_attribute_node->description = (char *)malloc(strlen(description_str)+1);
    strcpy(entity_attribute_node->description, description_str);
  } 
  /* 
   ** Assign attribute to entity attribute partition 
   ** According to the following rules/heuristics
   */
  if (entity_attribute_node->is_identifier) 
    partition_list_ptr = &entity_node->identifier_list;
  else {
    /* 
     ** If access is writeable, then assign attribute to characteristic partition
     */
    if ((entity_attribute_node->access == READWRITE) || (entity_attribute_node->access == WRITEONLY)){ 
      partition_list_ptr = &entity_node->characteristic_list;
    }
    else {
      switch (entity_attribute_node->syntax) {
      case COUNTER :
      case GAUGE :
      case TIMETICKS : {
	partition_list_ptr = &entity_node->counter_list;
	break;
      }
	/* add to list of status attributes */
	default :
	  partition_list_ptr = &entity_node->status_list;
      }
    }
  }
    
  /*
   ** Insert Attribute Node onto entity attribute partition list
   */
  if (next_attribute_node = *partition_list_ptr) {
    while (next_attribute_node -> nextsibling) 
      next_attribute_node = next_attribute_node->nextsibling;
    next_attribute_node->nextsibling = entity_attribute_node;
  }
  else {
    *partition_list_ptr = entity_attribute_node;
  }
  return(entity_attribute_node);
}

/***************************************************************************
  map_mibtree_to_entitytree	-  Transform MIB object parse node, producing Entity parse node.
***************************************************************************/

static void map_mibtree_to_entitytree
#if PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE *mib_node, T_ENTITY_NODE *parent_entity, T_ENTITY_NODE **ptr_entity_tree)
#else
(mib_node, parent_entity, ptr_entity_tree)
T_MIB_OBJECT_NODE *mib_node;
T_ENTITY_NODE *parent_entity, **ptr_entity_tree;
#endif

{
  T_ENTITY_ATTRIBUTE_NODE *entity_attribute_node;
  
  T_MIB_OBJECT_NODE *next_mib_node, *child, *child_mib_node, *grandchild_mib_node;
  int is_group_or_module;
  int i;
  char symbol_name[SIZE_OF_IDENTIFIER];
  char objno_str[SIZE_OF_IDENTIFIER];
  
  /*
   ** Select "scalar object":  OBJECT_TYPE,  has no children, has no indeces
   */
  if ((mib_node->object == OBJECT_TYPE) && (!(mib_node->childlist)) && (!(mib_node->num_indices))) {
    AddEntityAttr_node(mib_node, parent_entity);
  }
  else {
    /*
     ** Select "conceptual table object": 
     */
    if (mib_node->syntax == SEQUENCE_OF) {
      /*
       ** Transform MIB table- MIB entry to Child Entity
       **
       ** ASSERT that MIB object with syntax "SEQUENCE OF <entry>"
       ** has only one child object named <entry>.
       ** 
       ** Ignore one level of encapsulation (SEQUENCE_OF, SEQUENCE).
       ** Since CA MIRC expects that SEQUENCE level to exist, then
       ** the SEQUENCE OF level is ignored.
       **
       ** Validate Table, child, index relations
       */
      /* ASSERT table is not empty */	
      if (mib_node->childlist == 0) {	
	fprintf (stdout,MSG(msg27, "MTU ERROR -- Table (%s) is empty\n"), mib_node->object_name);
	if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg27(), mib_node->object_name);
	DEBUG_BREAK; /* ASSERT table is not empty of children */
	MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
      }
      else {
	/* ASSERT: 1-1 relationship between table and table-entry */
/*	if ((mib_node->childlist) && (mib_node->childlist == mib_node->lastchild)) { */
	if (1) {
	  mib_node = mib_node->childlist;	/* Collapse MIB hierarchy one level */
	  
	  /* Weak ASSERTION: Table has no indices */
	  if (mib_node->num_indices == 0) {
	    fprintf (stdout,MSG(msg29, "MTU INFORMATION -- Table (%s) has no indices\n"), mib_node->object_name);
	    if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg29(), mib_node->object_name);
	  }
	  
	  /*
	   * Process body of table indices, verifying that each table index is a table member,
	   * and setting the index flag of each child mib object
	   */
	  for (i = 0; i < mib_node->num_indices; i++) {
	    if (hashtable_find_node (mib_node->index[i], &child)) {
	      if (child->parent == mib_node) 
		child->is_index = TRUE;	/* Mark attribute as a table-index */
	      else {
		fprintf (stdout,MSG(msg31, "MTU INFORMATION -- Index (%s) is not contained within table (%s)\n"), 
			 child->object_name, mib_node->object_name);
		if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg31(), child->object_name, mib_node->object_name);
	      }
	    }
	    else {
	      fprintf (stdout,MSG(msg32, "MTU ERROR -- Index (%s) is not contained within mib (%s)\n"), 
		       mib_node->index[i], mib_node->object_name);
	      if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg32(),  mib_node->index[i], mib_node->object_name);
	      DEBUG_BREAK;      /* EXCEPTION:  Internal logic error */
	      MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
	    }
	  }	/* End FOR */
	  
	  /*
	   ** Create an Entity node (ema child entity)
	   */
	  parent_entity = AddEntityNode(mib_node, parent_entity, ptr_entity_tree);
	}
	else {
	  fprintf (stdout,MSG(msg28, "MTU ERROR -- Table (%s) has more than one type of entry\n"), mib_node->object_name);
	  if (cmdline_table[OPT_LISTFILE].is_enabled) fprintf (yyout, msg28(), mib_node->object_name);
	  DEBUG_BREAK;      /* EXCEPTION:  Internal logic error */
	  MIB_ERROR;	/* Increment count of mib errors, QUIT if threshold is exceeded */
	}
      }
    }	/* END select "conceptual table object" */
    else {
      /*
       ** Select "columnar object":
       */
      if (mib_node->num_indices) {
	DEBUG_BREAK;
      }
      else {
	/*
	 ** Select "group" or "module" object:
	 ** 	prune a bit
	 */
	if ((mib_node->object == OBJECT_IDENTIFIER) && (mib_node->childlist)) {
	  is_group_or_module = 0;
	  for (child_mib_node = mib_node->childlist; child_mib_node; child_mib_node = child_mib_node->nextsibling) {
	    if (((child_mib_node->object == OBJECT_TYPE) && (!(child_mib_node->childlist)) && (!(child_mib_node->num_indices))) ||
		(child_mib_node->syntax == SEQUENCE_OF) ||
		(child_mib_node->num_indices)) {
	      is_group_or_module = 1;
	      break;
	    }
	    
	    if (mib_node->childlist->nextsibling) {
	      for (grandchild_mib_node = child_mib_node->childlist; grandchild_mib_node;
		   grandchild_mib_node = grandchild_mib_node->nextsibling) {
		if (((grandchild_mib_node->object == OBJECT_TYPE) && (!(grandchild_mib_node->childlist)) 
		     && (!(grandchild_mib_node->num_indices))) ||
		    (grandchild_mib_node->syntax == SEQUENCE_OF) ||
		    (grandchild_mib_node->num_indices)) {
		  is_group_or_module = 1;
		  break;
		}
	      }
	      if (is_group_or_module)
		break;
	    }
	  } /* End of for */
	  if (is_group_or_module)
	    parent_entity = AddEntityNode(mib_node, parent_entity, ptr_entity_tree);
	} /* End select "group" or "module" object: */
      }
    }
  }

  /*
   * Process MIB branch, producing Entity branch
   */
  if (next_mib_node = mib_node->childlist) {
    do 
      map_mibtree_to_entitytree(next_mib_node, parent_entity, ptr_entity_tree);
    while (next_mib_node = next_mib_node->nextsibling);
  }
}

/*
 ****************************************************************************
 *	GenerateEntityCodes -
 *   ReGenerate entity codes for child entity nodes in the entity model tree.
 *   We start at code 1 at each level and number each object in consecutive
 *   order.
 ***************************************************************************
 */
void GenerateEntityCodes
#if PROTOTYPE_ALLOWED
(T_ENTITY_NODE *entity_node, int code, int entity_level, int *ptr_entity_tree_size)
#else
(entity_node, code, entity_level, ptr_entity_tree_size)
     T_ENTITY_NODE *entity_node;
     int code;
     int entity_level;
     int *ptr_entity_tree_size;
#endif
{
  T_ENTITY_NODE *next_entity_node;
  
  entity_level++;	/* Increment recursion depth */
  
  /*
   * Preserve the original object number for root
   * Otherwise, regenerate unique entity-code values.
   */

  if ((entity_level) > 1) 
    entity_node->entitycode = code;

  if (next_entity_node = entity_node->childlist) {
    code = 0;
    do 
      GenerateEntityCodes(next_entity_node, ++code, entity_level, ptr_entity_tree_size);
    while (next_entity_node = next_entity_node->nextsibling);
  }

  /* Entity_tree size = maximum depth */
  if (entity_level > *ptr_entity_tree_size)
    *ptr_entity_tree_size = entity_level;
}

/****************************************************************************
  Transform the MIB object parse tree, producing the Entity parse tree
  ****************************************************************************/

int produce_entity_tree
#ifdef PROTOTYPE_ALLOWED
  (T_MIB_OBJECT_NODE **ptr_mib_tree, T_ENTITY_NODE **ptr_entity_tree)
#else
(ptr_mib_tree, ptr_entity_tree)
T_MIB_OBJECT_NODE **ptr_mib_tree;
T_ENTITY_NODE **ptr_entity_tree;
#endif
{
  int entity_levels, error_count;

  
  /*
   ** Map mib tree to entity tree
   */
  error_count = mib_error_count;	/* Remember initial error count */
  entity_levels = 0;
  map_mibtree_to_entitytree(*ptr_mib_tree, (T_ENTITY_NODE *)0, ptr_entity_tree);
  if (mib_error_count != error_count) {
    DEBUG_BREAK;
    return FALSE;
  }

  GenerateEntityCodes(*ptr_entity_tree, 1, 0, &entity_levels);

  return TRUE;
}

/********************************************************************************
  void FreeEntityNode 
  ********************************************************************************/
void FreeEntityNode
#if PROTOTYPE_ALLOWED
  (T_ENTITY_NODE *entity_node)
#else
(entity_node)
T_ENTITY_NODE *entity_node;
#endif
{
  T_ENTITY_NODE *next_entity_node;
  T_ENTITY_ATTRIBUTE_NODE *curr_attr, *next_attr;
  T_EVENT_OBJECT *event, *next_event;
  T_EVENT_ARGUMENT *next_event_arg, *event_arg;

  if (next_entity_node = entity_node->childlist)
    FreeEntityNode(next_entity_node);
  
  if (next_entity_node = entity_node->nextsibling) 
    FreeEntityNode(next_entity_node);
  
  /*
   ** Free set of attributes nodes
   */
  next_attr = entity_node->identifier_list;
  while (curr_attr = next_attr) {
    next_attr = curr_attr->nextsibling;
    if (curr_attr->description)
      free (curr_attr->description);
    free (curr_attr);
  }
  next_attr = entity_node->status_list;
  while (curr_attr = next_attr) {
    next_attr = curr_attr->nextsibling;
    if (curr_attr->description)
      free (curr_attr->description);
    free (curr_attr);
  }
  next_attr = entity_node->counter_list;
  while (curr_attr = next_attr) {
    next_attr = curr_attr->nextsibling;
    if (curr_attr->description)
      free (curr_attr->description);
    free (curr_attr);
  }
  next_attr = entity_node->characteristic_list;
  while (curr_attr = next_attr) {
    next_attr = curr_attr->nextsibling;
    if (curr_attr->description)
      free (curr_attr->description);
    free (curr_attr);
  }

  if (entity_node->description)
    free(entity_node->description);

/*
 ** Free list of events
 */

 next_event = entity_node->event_list;
 while (event = next_event) {
	next_event_arg = event->event_argument_list;
	while (event_arg = next_event_arg){
		next_event_arg = event_arg->next_arg;
		free(event_arg);
	}
	next_event = event->event_next;
	if (event->description)
	  free(event->description);
	free(event);
	}
  
  free(entity_node);
}
/********************************************************************************
  init_entitytree()
********************************************************************************/
void init_entitytree()
{
  if (entity_root) {
    FreeEntityNode (entity_root); 
    entity_root = 0;
  }
}
