
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**  IMG__LINKED_LIST_UTILS.C
**
**  FACILITY:
**
**      Image Services Library
**
**  ABSTRACT:
**
**	This module contains an assortment of linked-list management
**	routines that are used in common throughout other ISL modules.
**	This module was called Img__COMMON_UTILS.C.
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Mark W. Sornson
**
**  CREATION DATE:
**
**     28-NOV-1989 (original 14-NOV-1986)
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
*/
#ifdef NODAS_PROTO
void		 _ImgLLAppendElement();
struct	BHD	*_ImgLLFirstElement();
struct	BHD	*_ImgLLLastElement();
struct	BHD	*_ImgLLNextElement();
void		 _ImgLLPrependElement();
struct	BHD	*_ImgLLPrevElement();
struct	BHD	*_ImgLLRemoveElement();
#endif

/*
**  Include files:
*/
#include <img/ImgDef.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif


/*
**  MACRO definitions:
**
**	none
*/

/*
**  Equated Symbols:
**
**	none
*/                                                                            

/*
**  External References:
**
**	none
*/

/*
**  Local Storage:
**
**	none
*/


/******************************************************************************
**  _ImgLLAppendElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Append an element to a linked list.
**
**  FORMAL PARAMETERS:
**
**	llhead:		(pointer to) address of linked list head
**	new_element:	(pointer to) address of new element to be appended
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      Changes the blink pointer in the list head and updates the list
**	count in the list head.  Also changes flink pointer in the
**	original last list element.
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgLLAppendElement( llhead, new_element )
struct	LHD	*llhead;
struct	BHD	*new_element;
{
/* pointer to address of cur. last element */
struct 	BHD	*last_element = llhead->LhdA_Blink;

/*									*
 *	change the list head, making the BLINK point to the new		*
 *	last element							*
 *									*/
llhead->LhdA_Blink = new_element;

/*									*
 *	change the last element, making the FLINK point to the		*
 *	new last element						*
 *									*/
last_element->BhdA_Flink = new_element;

/*									*
 *	change the new element, making the FLINK point to the list	*
 *	head and the BLINK point to the old last element		*
 *									*/
new_element->BhdA_Flink = (struct BHD *) llhead;
new_element->BhdA_Blink = last_element;

/*
** increment element list count in list head
*/
++llhead->LhdW_ListCnt;

return;
} /* end of _ImgLLAppendElement function */


/******************************************************************************
**  _ImgLLFirstElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the first element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**
**  IMPLICIT INPUTS:
**
**       address of first element and list count (in llhead)
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of first element in the list or zero if the list is empty.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct BHD *_ImgLLFirstElement( llhead )
struct	LHD	*llhead;
{
struct	BHD	*first_element = 0;

if ( llhead->LhdW_ListCnt != 0 )
    first_element = llhead->LhdA_Flink;

return first_element;
} /* end of _ImgLLFirstElement */


/******************************************************************************
**  _ImgLLLastElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the last element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**
**  IMPLICIT INPUTS:
**
**       address of last element and list count (in llhead)
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of last element in the list or zero if the list is empty.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct BHD *_ImgLLLastElement( llhead )
struct	LHD	*llhead;
{
struct	BHD	*last_element = 0;

if ( llhead->LhdW_ListCnt != 0 )
    last_element = llhead->LhdA_Blink;

return last_element;
}/* end of _ImgLLLastElement */


/******************************************************************************
**  _ImgLLNextElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the next element to a particular element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**	cur_element	current list element, passed by reference
**
**  IMPLICIT INPUTS:
**
**      Address of next element (passed in current element) 
**	and list count (in llhead).
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of next element in the list or zero if there is no next
**	element.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct BHD *_ImgLLNextElement( llhead, cur_element )
struct	LHD	*llhead;
struct	BHD	*cur_element;
{
struct	BHD	*next_element = 0;

if (llhead->LhdA_Blink != cur_element)		/* not the last element? */
    next_element = cur_element->BhdA_Flink;	/* then return next element */

return next_element;
} /* end of _ImgLLNextElement */


/******************************************************************************
**  _ImgLLPrependElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Prepend an element to a linked list.
**
**  FORMAL PARAMETERS:
**
**	llhead:		(pointer to) address of linked list head
**	new_element:	(pointer to) address of new element to be prepended
**
**  IMPLICIT INPUTS:
**
**      Changes the flink pointer in the list head and updates the list
**	count in the list head.  Also changes blink pointer in the
**	original first list element.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgLLPrependElement( llhead, new_element )
struct	LHD	*llhead;
struct	BHD	*new_element;
{
/* pointer to address of cur. last element */
struct	BHD	*first_element = llhead->LhdA_Flink;

/*									*
 *	change the list head, making the FLINK point to the new		*
 *	last element							*
 *									*/
llhead->LhdA_Flink = new_element;

/*									*
 *	change the first element, making the BLINK point to the		*
 *	new first element						*
 *									*/
first_element->BhdA_Blink = new_element;

/*									*
 *	change the new element, making the BLINK point to the list	*
 *	head and the FLINK point to the old first element		*
 *									*/
new_element->BhdA_Blink = (struct BHD *) llhead;
new_element->BhdA_Flink = first_element;

/*
** increment element list count in list head
*/
++llhead->LhdW_ListCnt;

return;
} /* end of _ImgLLPrependElement function */


/******************************************************************************
**  _ImgLLPrevElement
**
**  FUNCTIONAL DESCRIPTION:
**
**      Return the previous element to a particular element in a linked list.
**
**  FORMAL PARAMETERS:
**
**      llhead		linked list head, pased by reference.
**	cur_element	current list element, passed by reference
**
**  IMPLICIT INPUTS:
**
**      Address of previous element (passed in current element) 
**	and list count (in llhead).
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of previous element in the list or zero if there is no next
**	element.
**
**  SIGNAL CODES:
**
**      none.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct BHD *_ImgLLPrevElement( llhead, cur_element )
struct	LHD	*llhead;
struct	BHD	*cur_element;
{
struct	BHD	*prev_element = 0;

if (llhead->LhdA_Flink != cur_element)		/* not the first element? */
    prev_element = cur_element->BhdA_Blink;	/* then return prev element */

return prev_element;
} /* end of _ImgLLPrevElement */


/******************************************************************************
**  _ImgLLRemoveElement
**
**  FUNCTIONAL DESCRIPTION:
**
**	Remove a specific element from a linked list.  (Decrements
**	list count in list head.)  The element can be anywhere in
**	the list.
**
**  FORMAL PARAMETERS:
**
**	llhead:		(pointer to) address of linked list head
**	cur_element:	(pointer to) address of element to be removed from
**			the list
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      Changes the flink and blink pointers of the respective original
**	previous and next elements.  Also changes the list count in the
**	list head.
**
**  FUNCTION VALUE:
**
**	If there is a next element
**		return address of (block head of) next element 
**	else if there is a previous element
**		return address of (block head of) previous element
**	else 
**		return 0 (meaning the list is empty).
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
struct BHD *_ImgLLRemoveElement( llhead, cur_element )
struct	LHD	*llhead;
struct	BHD	*cur_element;
{
long return_code;
struct	BHD	*next_element = cur_element->BhdA_Flink;
struct	BHD	*prev_element = cur_element->BhdA_Blink;

prev_element->BhdA_Flink = next_element;	/* collapse the list,	*/
next_element->BhdA_Blink = prev_element;	/* removing cur_element */

--(llhead->LhdW_ListCnt);			/* decr element listcnt	*/

/*------------------------------------------------------+
|	Return the new current element address.		|
|	NOTE: if the list is empty, the returned	|
|	address is zero (or null), otherwise		|
|	use the next element unless the cur_element	|
|	had been the last one, in which case use the	|
|	previous element.				|
+-------------------------------------------------------*/
if (llhead->LhdW_ListCnt == 0)			/* check for empty list	*/
    return_code = 0;				/* no current element	*/
else if ( next_element != (struct BHD *) llhead )/* next el. not the LHD?*/
    return_code = (long) next_element;		/* set cur el. forward	*/
else
    return_code = (long) prev_element;		/* set cur el. reverse	*/

return (struct BHD *) return_code;
} /* end of _ImgLLRemoveElement */
