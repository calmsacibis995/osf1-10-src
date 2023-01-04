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
** COPYRIGHT (c) 1989, 1990, 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/

/*
**++
**  Subsystem:
**	HyperSession 
**
**  Version: V1.0
**
**  Abstract:
**	Map table support routines
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	André Pavanello
**
**  Creation Date: 22-Jan-90
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

/*
**  Table of Contents
*/

_DeclareFunction(static _Void MapTableAddData,
        (_MapTable table, _AnyPtr user_data));
	
/*
**  Macro Definitions
*/

/*
**  Map table default parameters
*/

#define _MapTableInitialSize		16
#define _MapTableIncrementSize		8
#define _MapTableDataInitialSize	2
#define _MapTableDataIncrementSize	2

/*
**  Type Definitions
*/

/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvMapTable()
/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    return;
    }


_MapTable  EnvMapTableCreate(size)
_Integer size;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer	    i;
    _MapTable	    table;

    if (size <= 0)
	size = _MapTableInitialSize;

    /*
    **  Allocate the map table
    */
    
    table = (_MapTable) _AllocateMem(sizeof(_MapTableInstance) * size);
    table[0].key = (_AnyPtr) 0;
    table[0].count = 0;
    table[0].size = size;

    /*
    **  Initialize the fields in the table
    */
    
    for (i = 1; i < size; i++) {
	table[i].key = (_AnyPtr) 0;
	table[i].size = 0;
    }

    return(table);
    }


_Void  EnvMapTableAddEntry(map_table, key, user_data)
_MapTable *map_table;
 _AnyPtr key;
 _AnyPtr user_data;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _MapTable	    table;
    _Integer	    free_slot = 0;
    _Integer	    i;
    _Integer	    new_size;

    table = *map_table;

    if (key == (_AnyPtr) 0)
	return;
    
    /*
    **  Look in table if key already present
    */

    for (i = 1; i < table[0].size; i++) {

	if (table[i].key == key) {
	    MapTableAddData(&(table[i]), user_data);
	    return;
	}

	if ((table[i].key == (_AnyPtr) 0) && (free_slot <= 0))
	    free_slot = i;
    }

    /*
    **  It's a new key
    */

    if (free_slot <= 0) {

	/*
	**  Table is full, increase its size
	*/

	new_size = (table[0].size + _MapTableIncrementSize) *
	    sizeof(_MapTableInstance);
	table = (_MapTable) _ReallocateMem(table, new_size);
	*map_table = table;
	free_slot = ++table[0].count;

	/*
	**  Initialize the new fields
	*/

	for (i = table[0].size; i < (table[0].size + _MapTableIncrementSize); i++) {

	    table[i].key = (_AnyPtr) 0;
	    table[i].size = 0;
	}
	
	table[0].size += _MapTableIncrementSize;
    }

    table[free_slot].key = key;
    MapTableAddData(&(table[free_slot]), user_data);
    table[0].count++;
    
    return;
    }


_Boolean  EnvMapTableGetEntry(table, key, data, count)
_MapTable table;
 _AnyPtr key;
 _AnyPtr *data;

    _Integer *count;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer	i;

    if (key != (_AnyPtr) 0) {

	for (i = 1; i < table[0].size; i++) {

	    if (table[i].key == key) {
		*data = (_AnyPtr) table[i].data;
		*count = table[i].count;
		return (_True);
	    }
	}
    }

    *count = 0;
    *data = (_AnyPtr) 0;
    
    return (_False);
    }


_Boolean  EnvMapTableRemoveEntry(table, key)
_MapTable table;
 _AnyPtr key;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer	i;

    if (key == (_AnyPtr) 0)
	return (_False);

    for (i = 1; i < table[0].size; i++) {

	if (table[i].key == key) {
	    table[i].key = (_AnyPtr) 0;
	    table[i].count = 0;
	    return (_True);
	}
    }

    return (_False);
    }


_Boolean  EnvMapTableRemoveEntryData(table, key, data)
_MapTable table;
 _AnyPtr key;
 _AnyPtr data;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer	i;
    _Integer	j;
        
    if (key == (_AnyPtr) 0)
	return (_False);

    for (i = 1; i < table[0].size; i++) {

	if (table[i].key == key) {

	    for (j = 0; j < table[i].count; j++)
	    
		if (table[i].data[j] == data) {
		    for (j; j < table[i].count - 1; j++)
			table[i].data[j] = table[i].data[j + 1];
		    table[i].count--;
		    if (table[i].count <= 0)
			_MapTableRemoveEntry(table, key);
		    return (_True);
		}
	    return (_False);
	}
    }

    return (_False);
    }


static _Void  MapTableAddData(table, user_data)
_MapTable table;
 _AnyPtr user_data;

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _Integer	new_size;

    /*
    **  If data space is empty, allocate some space
    */

    if (table->size <= 0) {
    
	table->data = (_AnyPtr *) _AllocateMem(sizeof(_AnyPtr) *
	    _MapTableDataInitialSize);
	table->size = _MapTableDataInitialSize;
	table->count = 0;
    }

    /*
    **  If data space is full, increase it
    */
    	
    if (table->count >= table->size) {

	new_size = (table->size + _MapTableDataIncrementSize) *
	    sizeof(_AnyPtr);
	table->data = (_AnyPtr *) _ReallocateMem(table->data, new_size);
	table->size += _MapTableDataIncrementSize;
    }

    /*
    **  Store the user data
    */

    table->data[table->count] = user_data;
    table->count++;

    return;
    }

