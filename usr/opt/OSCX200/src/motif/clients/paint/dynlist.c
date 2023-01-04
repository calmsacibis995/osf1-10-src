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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/dynlist.c,v 1.1.2.2 92/12/11 08:34:41 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
/* COPYRIGHT (c) 1988 BY DIGITAL EQUIPMENT CORPORATION.  ALL RIGHTS RESERVED.
 *
 * PROGRAM:	General utility (originally for EPIC/Writer equation editor)
 *
 * DESCRIPTION:	Generic stack handler (portable)
 *		
 * REVISION HISTORY:
 *  jnh 26-Jul-88 Created.
 *
 ******	Include files ******/
#include "sysdep.h" /* jj-port */
#include "DynList.h"

/****** Data declarations ******/

#define PTR_SIZE (sizeof (char *))

typedef struct
  {
    int free_unused_flag;
    int item_size;
    int block_size;
    char *name;
    char *first_block;
    char *current_block;
    char *next_item;
  } DYN_LIST;	    /* the internal ("real") type of a DYNAMIC_LIST */

/******	Notes ******/
/*
    Each block of the list is of the form

     ||next|prev|item 0	|item 1	|	...	|item chunksize-1   ||

    Where next is a pointer to the next block and
	  prev is a pointer to the previous block.
*/

/****** Function and subroutine definitions ******/


DYNAMIC_LIST DynListInit (item_size, chunk_size, free_unused_flag, name)
  register int item_size;
  register int chunk_size;
  register int free_unused_flag;
  register char *name;
  {
    register DYN_LIST *dyn_list;
    register char **p_link;

    dyn_list = (DYN_LIST *) XtMalloc (sizeof (DYN_LIST));
    dyn_list->free_unused_flag = free_unused_flag;
    dyn_list->name = (char *) NULL;
    if (name != (char *) NULL)
      if (*name != '\0')
	dyn_list->name = name;
    dyn_list->item_size = item_size;
    dyn_list->block_size = chunk_size * item_size + 2 * PTR_SIZE;
    dyn_list->first_block =
    dyn_list->current_block = (char *) XtMalloc (dyn_list->block_size);
    p_link = (char **) dyn_list->current_block;
    *p_link++ = (char *) NULL;	/* next block */
    *p_link++ = (char *) NULL;	/* previous block */
    dyn_list->next_item = (char *) p_link;
    if (dyn_list->name != (char *) NULL)
      printf ("Allocating %d items for dynamic list %s\n", 
	      (dyn_list->block_size - 2 * PTR_SIZE) / dyn_list->item_size, 
	      dyn_list->name);

    return ((DYNAMIC_LIST) dyn_list);

  } /* end of DynListInit() */


void DynListDestroy (dynamic_list)
  register DYNAMIC_LIST dynamic_list;
{
    register DYN_LIST *dyn_list;
    register char *pblock;
    register char *pnext;
    register int chunk_count;

    dyn_list = (DYN_LIST *) dynamic_list;
    chunk_count = 0;
    for (pblock = dyn_list->first_block;
	 pblock != (char *) NULL;
	 pblock = pnext)
    {
	pnext = *((char **)pblock);	/* next-block field from this block */
	XtFree (pblock);
	chunk_count++;
    }
    if (dyn_list->name != (char *) NULL)
	printf ("Freeing %d items of dynamic list %s\n", 
		  (dyn_list->block_size - 2 * PTR_SIZE) * chunk_count 
						    / dyn_list->item_size, 
		  dyn_list->name);

    XtFree ((char *) dyn_list);

} /* end of DynListDestroy() */

POINTER DynListNext (dynamic_list) /* jj-port */
  register DYNAMIC_LIST dynamic_list;
  {
    register DYN_LIST *dyn_list;
    register char *p;		/* return value:  pointer to data buffer */
    register char **pnext0;	/* ptr to next-block field in current block */
    register char **pnext1;	/* ptr to next-block field in new block */
    register char **pprev1;	/* ptr to previous-block field in new block */

    dyn_list = (DYN_LIST *) dynamic_list;
    if ((dyn_list->next_item - dyn_list->current_block) >= 
	dyn_list->block_size) /* no room */
      {
	pnext0 = (char **) dyn_list->current_block;
	if (*pnext0 != (char *) NULL)
	  {
	    dyn_list->current_block = *pnext0;
	  }
	else
	  {
	    *pnext0 =
	    dyn_list->current_block = (char *) XtMalloc (dyn_list->block_size);
	    pnext1 = (char **) dyn_list->current_block;
	    *pnext1 = (char *) NULL;
	    pprev1 = pnext1 + 1;
	    *pprev1 = (char *) pnext0;	/* pnext0 is the start of block0 */

	    if (dyn_list->name != (char *) NULL)
		printf ("Allocating %d items for dynamic list %s\n", 
			(dyn_list->block_size - 2 * PTR_SIZE) / 
				dyn_list->item_size, 
			dyn_list->name);
	  }
	dyn_list->next_item = dyn_list->current_block + 2 * PTR_SIZE;
      }

    p = dyn_list->next_item;
    dyn_list->next_item += dyn_list->item_size;
    return ((POINTER) p); /* jj-port */

  } /* end of DynListNext() */


/* call DynListNext and copy data into the new item. */
/* I recommend bypassing this routine. */
void DynListPush (dynamic_list, data)
  register DYNAMIC_LIST dynamic_list;
  register POINTER data; /* jj-port */
  {
    register DYN_LIST *dyn_list;
    register char *src;
    register char *dest;
    register int i;

    dyn_list = (DYN_LIST *) dynamic_list;
    dest = (char *) DynListNext (dyn_list);
    src = (char *) data;

    /* if I knew the size of the data item I'd do a bulk copy... */
    for (i = dyn_list->item_size;  i > 0;  i--)
      {
	*dest++ = *src++;
      }

  } /* end of DynListPush() */


POINTER DynListPrev (dynamic_list) /* jj-port */
  register DYNAMIC_LIST dynamic_list;
  {
    register DYN_LIST *dyn_list;
    register POINTER return_value; /* jj-port */

    dyn_list = (DYN_LIST *) dynamic_list;
    DynListPop (dyn_list);
    return_value = (POINTER) DynListPop (dyn_list); /* jj-port */
    DynListNext (dyn_list);
    return (return_value);

  } /* end of DynListPrev() */


POINTER DynListPop (dynamic_list) /* jj-port */
  register DYNAMIC_LIST dynamic_list;
  {
    register DYN_LIST *dyn_list;
    register char *return_value;
    register char *prev_block;

    dyn_list = (DYN_LIST *) dynamic_list;
    return_value = dyn_list->next_item - dyn_list->item_size;
    if (return_value >= dyn_list->current_block + 2 * PTR_SIZE)
    {		/*** normal case:  get previous item in this block */
	dyn_list->next_item = return_value;
    }
    else if (dyn_list->current_block != dyn_list->first_block)
    {		/*** block underflow:  get last item in previous block */
	prev_block = *(((char **) dyn_list->current_block) + 1);
	if (dyn_list->free_unused_flag != 0)
	{
	    *((char **) prev_block) = (char *) NULL;/* next_block pointer */
	    XtFree (dyn_list->current_block);
	    if (dyn_list->name != (char *) NULL)
	      printf ("Freeing %d items of dynamic list %s\n", 
		      (dyn_list->block_size - 2 * PTR_SIZE)
					/ dyn_list->item_size, 
		      dyn_list->name);
	}
	dyn_list->current_block = prev_block;
	dyn_list->next_item =
	return_value = dyn_list->current_block +
		       (dyn_list->block_size - dyn_list->item_size);
    }
    else
    {		/*** entire stack underflow */
	return_value = (char *) NULL;
    }

      return ((POINTER) return_value); /* jj-port */

  } /* end of DynListPop() */

#ifdef TEST
int main ()
  {
    DYNAMIC_LIST d;
    register int i;
    register int *pj;

    d = DynListInit (sizeof (int), 2, 0, "George (nofree)");

    for (i=0; i<=9; i++)
      {
	pj = (int *) DynListNext (d);
        *pj = i;
	printf ("(n->%d)", *pj);
      }

    for (i=9; i>=5; i--)
      {
	pj = (int *) DynListPop (d);
	if (pj != (int *) NULL)
	  {
	    printf ("(<-%d)", *pj);
	  }
	else
	  {
	    printf ("(NULL)");
	  }
      }

    for (i=5; i<=14; i++)
      {
	DynListPush (d, &i);
	printf ("(p->%d)", i);
      }

    for (i=14; i>=-5; i--)
      {
	pj = (int *) DynListPop (d);
	if (pj != (int *) NULL)
	  {
	    printf ("(<-%d)", *pj);
	  }
	else
	  {
	    printf ("(NULL)");
	  }
      }

    DynListDestroy (d);



    d = DynListInit (sizeof (int), 2, 1, "Martha (free)");

    for (i=0; i<=9; i++)
      {
	pj = (int *) DynListNext (d);
        *pj = i;
	printf ("(n->%d)", *pj);
      }

    for (i=9; i>=5; i--)
      {
	pj = (int *) DynListPop (d);
	if (pj != (int *) NULL)
	  {
	    printf ("(<-%d)", *pj);
	  }
	else
	  {
	    printf ("(NULL)");
	  }
      }

    for (i=5; i<=14; i++)
      {
	DynListPush (d, &i);
	printf ("(p->%d)", i);
      }

    for (i=14; i>=-5; i--)
      {
	pj = (int *) DynListPop (d);
	if (pj != (int *) NULL)
	  {
	    printf ("(<-%d)", *pj);
	  }
	else
	  {
	    printf ("(NULL)");
	  }
      }

    DynListDestroy (d);

  } /* end of main() */
#endif /* TEST */
