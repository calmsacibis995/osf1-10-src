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
static char *rcsid = "@(#)$RCSfile: rld_list.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/03/15 15:14:53 $";
#endif
/*
 * rld_list.c
 * Miscellaneous functions used to replace list-processing in libmld.
 */

#include <stdio.h>
#include <filehdr.h>
#include <aouthdr.h>
#include <scnhdr.h>
#include <obj.h>
#include <string.h>
#include <sys/auxv.h>
#include "rld.h"

#if 0
extern HANDLE *pObj_Head;
void _dump_list(char *msg){
    HANDLE *h;

    printf("\n%s\n",msg);
    printf("Dumping forward links\n");
    for (h=pObj_Head; h; h=h->next) {
	printf("%x->",h);
    }
    printf("\nDumping backward links\n");
    for (h=pObj_Head; h; h=h->next) {
	printf("%x<-",h->prev);
    }
    printf("\n");
}
#endif

/* Replaces objList_add(....,LIST_END) */

HANDLE *
rldList_append(HANDLE **head, HANDLE *h)
{
    h->next = NULL;
    if (!*head) {
	h->prev = h;
	*head = h;
    } else {
	(*head)->prev->next = h;
	h->prev = (*head)->prev;
	(*head)->prev = h;
    }
#if 0
    _dump_list("rldList_append");
#endif
    return(h);
}

/* Replace objList_add(.....,LIST_BEGINNING */

HANDLE *
rldList_prepend(HANDLE **head, HANDLE *h)
{
    h->next = *head;
    if (!*head) {
	h->prev = h;
    } else {
	h->prev = (*head)->prev;
	(*head)->prev = h;
    }
    *head = h;
#if 0
    _dump_list("rldList_prepend");
#endif
    return(h);
}

/* Replaces objList_change(......., LIST_DELETE) */

int
rldList_delete(HANDLE **head, HANDLE *h)
{
    if (h == *head) {

	/* Delete head of list */

	if (h->prev == *head) {
	    *head = (HANDLE *)NULL;
	} else {
	    (*head)->next->prev = (*head)->prev;
	    *head = (*head)->next;
	}

    } else if (h == (*head)->prev) {

	/* Delete tail of list */

	(*head)->prev = h->prev;
	h->prev->next = (HANDLE *)NULL;
    } else {

	/* Delete from middle of list */
	
	h->prev->next = h->next;
	h->next->prev = h->prev;
    }
#if 0
    _dump_list("rldList_delete");
#endif

}

/* Replaces objList_change(........, LIST_REPLACE) */

HANDLE *
rldList_replace(HANDLE **head, HANDLE *h, HANDLE *h2)
{
    if (*head == h) {
	*head = h2;
    } else {
	h->prev->next = h2;
    }
    if (h->next) {
	h->next->prev = h2;
    } else {
	(*head)->prev = h2;
    }
    h2->prev = h->prev;
    h2->next = h->next;
#if 0
    _dump_list("rldList_replace");
#endif
    return(h2);
}

/* Replaces objList_change(........, LIST_ADD_AFTER) */

HANDLE *
rldList_insert(HANDLE **head, HANDLE *h, HANDLE *h2)
{
    if (h == (*head)->prev) {
	(*head)->prev = h2;
    } else {
	h->next->prev = h2;
    }
    h2->next = h->next;
    h->next = h2;
    h2->prev = h;
#if 0
    _dump_list("rldList_insert");
#endif
    return(h2);
}

/*
 * r l d _ a d d _ o b j l i s t
 *
 * Add a dynamic dependency from one HANDLE to another.
 * Expand the dependency list as necessary.
 */

void
rld_add_liblist(HANDLE *head, HANDLE *h)
{
    int i;

    if (!head || !h)
      return;

    if (head->liblist_count == 0) {
	head->liblist_count = 10;
	head->liblist = chkmalloc(10 * sizeof(HANDLE *));
	memset(head->liblist, 0, 10 * sizeof(HANDLE *));
	head->liblist[0] = h;

    } else {
	
	for (i=0; i<head->liblist_count; i++) {
	    if (!head->liblist[i]) {
		/* Found an empty spot */
		head->liblist[i] = h;
		return;
	    }
	}
	i = head->liblist_count;
	head->liblist = chkrealloc(head->liblist,
				   (i<<1) * sizeof(HANDLE *));
	memset(&head->liblist[i], 0, i * sizeof(HANDLE *));
	head->liblist_count = i << 1;
	head->liblist[i] = h;

    }
}

/*
 * r l d _ d e l _ o b j l i s t
 *
 * Delete a dynamic dependency from a HANDLE.
 */

void
rld_del_liblist(HANDLE *head, HANDLE *h)
{
    int i;

    if (!head || !h || head->liblist_count == 0)
      return;

    for (i=0; i<head->liblist_count; i++) {
	if (head->liblist[i] == h) {
	    head->liblist[i] = 0L;
	}
    }
}
