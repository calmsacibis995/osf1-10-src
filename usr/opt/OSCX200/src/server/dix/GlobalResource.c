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
/***********************************************************
Copyright 1993 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
** File: 
**
**   GlobalResource.c
**
** Abstract:
**
**   Subroutines to support the management of server global resources,
**   which can be used to share data between two dependent but separate
**   libraries.
**
** Author: 
**
**   Dave Coleman (Digital Alpha Personal Systems group)
**
** Revisions:
**
**   92.01.15 DColeman
**     - Initial revision.
**       
*/

#include "stdio.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/*****************************************************************************/
/*
 * Resource allocation structures.
 */
typedef struct {
    int		allocated;
    int		lastused;
    int		inuse;
    void	**array;
} Alloc_InfoRec;

#define ALLOC_BLOCKS	16

void *RecAlloc(
    char		*type_name,
    Alloc_InfoRec	*pAlloc_Info,
    size_t		AllocBytes)
{
    int		prevused;

    prevused = pAlloc_Info->lastused;
    pAlloc_Info->lastused += AllocBytes;
    if (pAlloc_Info->lastused >= pAlloc_Info->allocated) {
	pAlloc_Info->allocated += AllocBytes * ALLOC_BLOCKS;
	pAlloc_Info->array = (void *)Xrealloc(pAlloc_Info->array, pAlloc_Info->allocated);
    }
    if (pAlloc_Info->array == (void *)NULL) {
	ErrorF ("Xrealloc failed for type %s\n");
	return ((void *)NULL);
    }
    pAlloc_Info->inuse++;

    return ((void *)&((char *)pAlloc_Info->array)[prevused]);
}

void RecFree(
    char		*type_name,
    Alloc_InfoRec	*pAlloc_Info,
    size_t		AllocBytes)
{
    /*
     * Decrement the allocated count and unalloc memory if none left.
     */
    if (--pAlloc_Info->inuse == 0) {
	pAlloc_Info->array = (void *)Xrealloc(pAlloc_Info->array, 0);
	pAlloc_Info->lastused = 0;
	pAlloc_Info->allocated = 0;
    }
}


/*****************************************************************************/
/*
 * Routines to setup and get information from dependent libraries.
 *
 *	int sublib_Init(char *, int *);
 *	int sublib_GetScreenPrivateIndex(char *, int *);
 *	int sublib_Close(char *);
 */

#define	SUBLIB_INFO	"_sublib_info"

/*****************************************************************************/
/*
 * sublib_Init
 *
 * Abstract:
 *
 *	Initialize sublib by allocating, or locating, ScreenPrivateIndex
 *	for this sublib.  Maintain a reference count for this sublib.
 *
 * Input:
 *
 *	sublib_Name	- pointer to string with name of sublibrary.
 *
 * Output:
 *
 *	*pPrivateIndex	- value of ScreenPrivateIndex for specified sublib.
 * 
 * Return value:
 *
 *	TRUE		= sublib was initialized correctly.
 *	FALSE		= sublib initialization failed.
 *
 */

typedef struct {
    int	ScreenPrivateIndex;
    int	refcnt;
  } sublib_InfoRec;

static Alloc_InfoRec	sublibAlloc	= {0, 0, 0, (void **)NULL};

int sublib_Init(
	char	*sublib_Name,
	int	*pPrivateIndex)
{
    char		*sublib_InfoName;
    int			alloc_bytes;
    int			status;
    sublib_InfoRec	*sublibCurrent;
    Alloc_InfoRec	*psublibAlloc;
    long		sublib_offset;
    int			sublibScreenPrivateIndex;
    int			sublib_size;

    alloc_bytes = strlen (sublib_Name) + strlen (SUBLIB_INFO) + 1;
    sublib_InfoName = (char *)Xalloc(alloc_bytes);
    strcpy (sublib_InfoName, sublib_Name);
    strcat (sublib_InfoName, SUBLIB_INFO);
    status = GetGlobalNamedResource (sublib_InfoName, (long *)&sublib_offset);
    if (!status) {
	/*
	 * Allocate ScreenPrivateIndex for this sublib.
	 */
	sublibScreenPrivateIndex = AllocateScreenPrivateIndex ();
	if (sublibScreenPrivateIndex < 0) {
	    ErrorF("sublib_Init: Unable to allocate %s\n", sublib_InfoName);
	    Xfree (sublib_InfoName);
	    *pPrivateIndex = 0;
	    return (FALSE);
	}

	/*
	 * Allocate structure to maintain reference count
	 * and store ScreenPrivateIndex for sublib.
	 */
	sublib_size = sizeof (sublib_InfoRec);
	sublibCurrent = (sublib_InfoRec *)RecAlloc("sublib", &sublibAlloc, sublib_size);
	sublib_offset = sublibAlloc.lastused - sublib_size;
	sublibCurrent->ScreenPrivateIndex = sublibScreenPrivateIndex;

	PutGlobalNamedResource (sublib_InfoName, (long)sublib_offset);
    } else {
	psublibAlloc = &sublibAlloc;
	sublibCurrent = (sublib_InfoRec *)&((char *)psublibAlloc->array)[sublib_offset];
	sublibScreenPrivateIndex = sublibCurrent->ScreenPrivateIndex;
    }

    sublibCurrent->refcnt += 1;
    Xfree (sublib_InfoName);

    /*
     * Return with value of ScreenPrivateIndex
     */
    *pPrivateIndex = sublibScreenPrivateIndex;
    return (TRUE);
}

/*****************************************************************************/
/*
 * sublib_Close (sublib_Name)
 *
 * Abstract:
 *
 *	Deallocate all resources that have been allocated for specified
 *	sublibrary.
 *
 * Input:
 *
 *	sublib_Name	- pointer to string with name of sublibrary.
 *
 * Return value:
 *
 *	TRUE		= Sublibrary resources were found and destroyed
 *	FALSE		= Sublibrary resources were not found.
 *
 */

int sublib_Close(
	char	*sublib_Name)
{
    char		*sublib_InfoName;
    int			alloc_bytes;
    int			status;
    sublib_InfoRec	*sublibCurrent;
    Alloc_InfoRec	*psublibAlloc;
    long		sublib_offset;

    alloc_bytes = strlen (sublib_Name) + strlen (SUBLIB_INFO) + 1;
    sublib_InfoName = (char *)Xalloc(alloc_bytes);
    strcpy (sublib_InfoName, sublib_Name);
    strcat (sublib_InfoName, SUBLIB_INFO);

    status = GetGlobalNamedResource (sublib_InfoName, (long *)&sublib_offset);
    if (status) {
	psublibAlloc = &sublibAlloc;
	sublibCurrent = (sublib_InfoRec *)&((char *)psublibAlloc->array)[sublib_offset];
	if (--sublibCurrent->refcnt == 0) {
	    status = DestroyGlobalNamedResource (sublib_InfoName);
	    RecFree ("sublib", &sublibAlloc, sizeof (sublib_InfoRec));
	}
    }
    return (status);
}

/*****************************************************************************/
/*
 * sublib_GetScreenPrivateIndex (sublib_Name, pPrivateIndex)
 *
 * Abstract:
 *
 *	Locate ScreenPrivateIndex for specified sublibrary.
 *
 * Input:
 *
 *	sublib_Name	- pointer to string with name of sublibrary.
 *
 * Output:
 *
 *	*pPrivateIndex	- value of ScreenPrivateIndex for specified sublib.
 * 
 * Return value:
 *
 *	TRUE		= name was found.
 *	FALSE		= name was not found.
 *
 */

int sublib_GetScreenPrivateIndex(
	char	*sublib_Name,
	int	*pPrivateIndex)
{
    char		*sublib_InfoName;
    int			alloc_bytes;
    int			status;
    sublib_InfoRec	*sublibCurrent;
    Alloc_InfoRec	*psublibAlloc;
    long		sublib_offset;

    alloc_bytes = strlen (sublib_Name) + strlen (SUBLIB_INFO) + 1;
    sublib_InfoName = (char *)Xalloc(alloc_bytes);
    strcpy (sublib_InfoName, sublib_Name);
    strcat (sublib_InfoName, SUBLIB_INFO);

    status = GetGlobalNamedResource (sublib_InfoName, (long *)&sublib_offset);
    Xfree (sublib_InfoName);

    if (status) {
	psublibAlloc = &sublibAlloc;
	sublibCurrent = (sublib_InfoRec *)&((char *)psublibAlloc->array)[sublib_offset];
	/*
	 * Return with value of ScreenPrivateIndex
	 */
	*pPrivateIndex = sublibCurrent->ScreenPrivateIndex;
	return (TRUE);
    } else {
	*pPrivateIndex = 0;
	return (FALSE);
    }
}


/*****************************************************************************/
/*
 * Subroutines to maintain list of Global Private Named Resources
 *
 *	int PutGlobalNamedResource(char *, long);
 *	int GetGlobalNamedResource(char *, long *);
 *	int DestroyGlobalNamedResource(char *);
 */

typedef struct {
    int		name_offset;
    long	data
  } Resource_InfoRec;

/*
 * Maintain list of resources and space to store resource names
 */

static Alloc_InfoRec	NameSpaceAlloc	= {0, 0, 0, (void **)NULL};
static Alloc_InfoRec	ResourceAlloc	= {0, 0, 0, (void **)NULL};

ShowNamedResources()
{
    int		index;
    int		count	= 0;
    int		name_offset;
    long	data;
    char	*NameCurrent;
    Resource_InfoRec	*ResourceCurrent;
    Alloc_InfoRec	*pResourceAlloc;
    Alloc_InfoRec	*pNameSpaceAlloc;
    int		Resource_offset;
    int		items;

    items = ResourceAlloc.lastused / sizeof (Resource_InfoRec);
    for (index = 0; index < items; index++) {
	Resource_offset = index * sizeof (Resource_InfoRec);
	pResourceAlloc = &ResourceAlloc;
	ResourceCurrent = (Resource_InfoRec *)&((char *)pResourceAlloc->array)[Resource_offset];

	pNameSpaceAlloc = &NameSpaceAlloc;
	name_offset = ResourceCurrent->name_offset;
	NameCurrent = (char *)&((char *)pNameSpaceAlloc->array)[name_offset];
	if (strlen (NameCurrent) > 0) {
	    ErrorF ("[%2d] %032s = %16lx\n", index, NameCurrent, ResourceCurrent->data);
	    count++;
	}
    }
    if (count == 0) {
	ErrorF ("There are no Global Named Resources stored.\n");
    }
}

/*
 * PutGlobalNamedResource
 *
 * Abstract:
 *
 *	Store specified name in resource database with a pointer to
 *	data at specified location.
 *
 * Input:
 *
 *	name		- pointer to string with identifying name.
 *	data		- data value
 *
 * Return value:
 *
 *	TRUE		= name was stored.
 *	FALSE		= name was not stored.
 *
 */
int PutGlobalNamedResource(
	char	*name,
	long	data)
{
    int		index;
    int		alloc_bytes;
    int		name_offset;
    char	*NameCurrent;
    Resource_InfoRec	*ResourceCurrent;
    Alloc_InfoRec	*pResourceAlloc;
    Alloc_InfoRec	*pNameSpaceAlloc;
    int		name_size;
    int		Resource_offset;
    int		items;

    /*
     * First, check that name was not already allocated.
     */
    items = ResourceAlloc.lastused / sizeof (Resource_InfoRec);
    for (index = 0; index < items; index++) {
	Resource_offset = index * sizeof (Resource_InfoRec);
	pResourceAlloc = &ResourceAlloc;
	ResourceCurrent = (Resource_InfoRec *)&((char *)pResourceAlloc->array)[Resource_offset];

	pNameSpaceAlloc = &NameSpaceAlloc;
	name_offset = ResourceCurrent->name_offset;
	NameCurrent = (char *)&((char *)pNameSpaceAlloc->array)[name_offset];
	if (strlen (NameCurrent) == 0) continue;
	if (strcmp (NameCurrent, name) == 0) {
	    ResourceCurrent->data = data;
	    return (TRUE);
        }
    }

    name_size = strlen (name) + 1;
    NameCurrent = (char *)RecAlloc("NameSpace", &NameSpaceAlloc, name_size);
    name_offset = NameSpaceAlloc.lastused - name_size;
    ResourceCurrent = (Resource_InfoRec *)RecAlloc("Resource", &ResourceAlloc, sizeof (Resource_InfoRec));

    strcpy (NameCurrent, name);
    ResourceCurrent->name_offset = name_offset;
    ResourceCurrent->data = (long)data;
    return (TRUE);
}

/*
 * GetGlobalNamedResource
 *
 * Abstract:
 *
 *	Lookup name in resource database, return pointer to data
 *	stored as "name".
 *
 * Input:
 *
 *	name		- pointer to string with identifying name.
 *
 * Output:
 *
 *	*data		- value of data stored by specified name.
 *
 * Return value:
 *
 *	TRUE		= name was found.
 *	FALSE		= name was not found.
 *
 */
int GetGlobalNamedResource(
	char	*name,
	long	*data)
{
    int		index;
    int		name_offset;
    char	*NameCurrent;
    Resource_InfoRec	*ResourceCurrent;
    Alloc_InfoRec	*pResourceAlloc;
    Alloc_InfoRec	*pNameSpaceAlloc;
    int		Resource_offset;
    int		items;

    items = ResourceAlloc.lastused / sizeof (Resource_InfoRec);
    for (index = 0; index < items; index++) {
	Resource_offset = index * sizeof (Resource_InfoRec);
	pResourceAlloc = &ResourceAlloc;
	ResourceCurrent = (Resource_InfoRec *)&((char *)pResourceAlloc->array)[Resource_offset];

	pNameSpaceAlloc = &NameSpaceAlloc;
	name_offset = ResourceCurrent->name_offset;
	NameCurrent = (char *)&((char *)pNameSpaceAlloc->array)[name_offset];
	if (strlen (NameCurrent) == 0) continue;
	if (strcmp (NameCurrent, name) == 0) {
	  *data = (long)ResourceCurrent->data;
	  return (TRUE);
        }
    }
    *data = (long)NULL;
    return (FALSE);
}

/*
 * DestroyGlobalNamedResource
 *
 * Abstract:
 *
 *	Remove specified name and it's data from the Global Resource list.
 *
 * Input:
 *
 *	name		- pointer to string with identifying name.
 *
 * Return value:
 *
 *	TRUE		= name was found and removed from list.
 *	FALSE		= name was not found.
 *
 */
int DestroyGlobalNamedResource(
	char	*name)
{
    int		index;
    int		name_offset;
    char	*NameCurrent;
    Resource_InfoRec	*ResourceCurrent;
    Alloc_InfoRec	*pResourceAlloc;
    Alloc_InfoRec	*pNameSpaceAlloc;
    int		Resource_offset;
    int		items;

    items = ResourceAlloc.lastused / sizeof (Resource_InfoRec);
    for (index = 0; index < items; index++) {
	Resource_offset = index * sizeof (Resource_InfoRec);
	pResourceAlloc = &ResourceAlloc;
	ResourceCurrent = (Resource_InfoRec *)&((char *)pResourceAlloc->array)[Resource_offset];

	pNameSpaceAlloc = &NameSpaceAlloc;
	name_offset = ResourceCurrent->name_offset;
	NameCurrent = (char *)&((char *)pNameSpaceAlloc->array)[name_offset];
	if (strlen (NameCurrent) == 0) continue;
	if (strcmp (NameCurrent, name) == 0) {
	    strcpy (NameCurrent, "\0");
	    /*
	     * Free the allocated blocks
	     */
	    RecFree("NameSpace", &NameSpaceAlloc, (strlen (name) + 1));
	    RecFree("Resource", &ResourceAlloc, sizeof (Resource_InfoRec));
	    return (TRUE);
        }
    }
    return (FALSE);
}
