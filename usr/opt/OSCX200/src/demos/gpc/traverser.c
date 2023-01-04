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
/* $XConsortium: traverser.c,v 5.1 91/02/16 10:07:54 rws Exp $ */
/***********************************************************
Copyright(c) 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium at M.I.T.

						All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity
pertaining to distribution of the software without specific, written
prior permission.

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*--------------------------------------------------------------------*\
|
|  Copyright (C) 1989,1990, 1991, National Computer Graphics Association
|
|  Permission is granted to any individual or institution to use, copy, or
|  redistribute this software so long as it is not sold for profit, provided
|  this copyright notice is retained.
|
|                         Developed for the
|                National Computer Graphics Association
|                         2722 Merrilee Drive
|                         Fairfax, VA  22031
|                           (703) 698-9600
|
|                                by
|                 SimGraphics Engineering Corporation
|                    1137 Huntington Drive  Unit A
|                      South Pasadena, CA  91030
|                           (213) 255-0900
|---------------------------------------------------------------------
|
| Author        :	nde / SimGraphics Engineering Corportation
|
| File          :	traverser.c
| Date          :	Fri Feb  9 10:46:55 PST 1990
| Project       :	PLB
|
| Description	:	The traverser handling functions for build
|			and execute mode.
|
| Status	:  	Version 1.0
|
|			Non-bullet proof.  Executute will branch to NULL
|			if given the value.  Could use this level of
|			inteligence, but... the traversers are DUMB by
|			design, it's up to the handling function to
|			figure out what an entity dones.
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Table of Contents
|
|	void build_traverser( *BIF_Traverser_state, *BIF_All )
|		:	Inserts the NULL terminated linked list of
|	void execute_traverser(*BIF_Traverser_state, *BIF_All )
|		:	The BIF execute traverser: FULLY DUMB.
|
\*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*\
|	Include files
\*--------------------------------------------------------------------*/
#include <stdio.h>
#include "bifbuild.h"
#include "biftypes.h"
#include "bifparse.h"
#include "db_tools.h"
#include "ph_map.h"

/*--------------------------------------------------------------------*\
| BEGIN PROCEDURE CODE                                                
\*--------------------------------------------------------------------*/

/*----------------------------------------------------------------------*\
| Procedure	:	void build_traverser( *BIF_Traverser_state,
|						*BIF_All )
|------------------------------------------------------------------------
| Description	:	Inserts the NULL terminated linked list of
|			entities into the open structure between
|			insert_after and insert_after.next.  Update the
|			insert after point to the end of the given list.
|			Typically, lists are single entities with
|			NULL nexts.
|
|			If the insert_after point is NULL insert at
|			top_of_list.
|------------------------------------------------------------------------
| Return	:	
\*----------------------------------------------------------------------*/
build_traverser( traverser_state, bif_entity )
BIF_Traverser_state *traverser_state;
BIF_All *bif_entity;

{

	BIF_All *next, *end_list;
	if ( traverser_state->insert_after != NULL )
	{
	/* Save the old next */
		next = traverser_state->insert_after->any.next;

	/* Make the top of the list the next of the insert_after entity */
		traverser_state->insert_after->any.next = bif_entity;
	}
	else
	{
	/* Insert at top of list*/
	/* The old next in the old top_of_list ( or empty structure ) */
		next = traverser_state->open_structure->top_of_list;
	/* Insert the new entity */
		traverser_state->open_structure->top_of_list = bif_entity;
	}

/* Find the end of list */
	end_list = end_of_list(bif_entity);

/* Link the end of list to the old next */
	end_list->any.next = next;

/* Update the insert after point to the end of list */
	traverser_state->insert_after = end_list;

}/* End BIF_build_traverser */

/*----------------------------------------------------------------------*\
| Procedure	:	void execute_traverser(*BIF_Traverser_state,
|						*BIF_All )
|------------------------------------------------------------------------
| Description	:	The BIF execute traverser: FULLY DUMB.
|			Invoke the handling function of each entity
|			in the NULL terminate list.
|------------------------------------------------------------------------
| Return	: 
\*----------------------------------------------------------------------*/
execute_traverser( traverser_state, bif_entity )
BIF_Traverser_state *traverser_state;
BIF_All *bif_entity;

{
	while ( bif_entity != NULL )
	{
#ifdef TEST_PRINT
/* #define TRACE_TRAV */
#ifdef TRACE_TRAV
{
char *find_keyword_token();
printf("Ex:%s\n",find_keyword_token((BIF_INT)bif_entity->any.entity_type));
fflush(stdout);
}
#endif /* TRACE_TRAV */
#endif /* TEST_PRINT */
		bif_entity->any.handler( traverser_state, bif_entity );
		bif_entity = bif_entity->any.next;
	}

} /* End BIF_execute_traverser */
