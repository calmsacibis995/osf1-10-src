
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
**  IMG__ITEMLIST_UTILS.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	[@tbs@]
**
**  ENVIRONMENT:
**
**	VAX/VMS
**
**  AUTHOR(S):
**
**	Mark Sornson
**
**  CREATION DATE:
**
**	21-SEP-1989
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global Routines
**
*/
#ifdef NODAS_PROTO
struct GET_ITMLST   *_ImgAppendGetlstItem();
struct ITMLST	    *_ImgAppendItmlstItem();
struct PUT_ITMLST   *_ImgAppendPutlstItem();
struct GET_ITMLST   *_ImgCreateGetlst();
struct ITMLST	    *_ImgCreateItmlst();
struct PUT_ITMLST   *_ImgCreatePutlst();
struct ITMLST	    *_ImgCvtPutlstToItmlst();
void		     _ImgDeleteItmlst();
long		     _ImgExtractGetlstItem();
long		     _ImgExtractItmlstItem();
long		     _ImgExtractItmlstItemAdr();
long		     _ImgExtractPutlstItem();
long		     _ImgGetlstItemCount();
long		     _ImgItmlstItemCount();
long		     _ImgPutlstItemCount();
long		     _ImgTotalGetlstItems();
long		     _ImgTotalItmlstItems();
long		     _ImgTotalPutlstItems();
#endif


/*
**  Include files:
**
*/
#include    <stdlib.h>
#include    <string.h>

#include    <img/ImgDef.h>
#include    <ImgDefP.h>
#include    <ImgMacros.h>
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
*/

/*
**  External References:
**
**	DAS routines
*/
#ifdef NODAS_PROTO
char	*_ImgCalloc();		/* IMG__MEMORY_MGT  */
void	 _ImgCfree();		/* IMG__MEMORY_MGT  */
char	*_ImgRealloc();		/* IMG__MEMORY_MGT  */
#endif

/*
**	Module local routines
*/
#ifdef NODAS_PROTO
static long	Get_getlst_length();
static long	Get_itmlst_length();
static long	Get_putlst_length();
#else
PROTO(static long Get_getlst_length, (struct GET_ITMLST */*itmlst*/));
PROTO(static long Get_itmlst_length, (struct ITMLST */*itmlst*/));
PROTO(static long Get_putlst_length, (struct PUT_ITMLST */*itmlst*/));
#endif
/*
**  Local Storage:
**
**	none
*/


struct GET_ITMLST *_ImgAppendGetlstItem( itmlst, itmcode, itmadr, itmlen,
					    retlenadr, index )
struct GET_ITMLST   *itmlst;
unsigned long	     itmcode;
long		     itmadr;
long		     itmlen;
unsigned long int   *retlenadr;
long		     index;
{
long		     element_count;
long		     list_len	    = Get_getlst_length( itmlst );
struct GET_ITMLST   *ret_itmlst;

ret_itmlst = (struct GET_ITMLST *) _ImgRealloc( (char *)itmlst, 
					list_len + sizeof(struct GET_ITMLST) );
element_count = list_len / sizeof(struct GET_ITMLST);

ret_itmlst[element_count].GetL_Code	= itmcode;
ret_itmlst[element_count].GetL_Length	= itmlen;
ret_itmlst[element_count].GetA_Buffer	= (char *)itmadr;
ret_itmlst[element_count].GetA_Retlen	= retlenadr;
ret_itmlst[element_count].GetL_Index	= index;

return ret_itmlst;
} /* end of _ImgAppendGetlstItem */


struct ITMLST *_ImgAppendItmlstItem( itmlst, itmcode, itmadr, itmlen,
					    retlenadr, index )
struct ITMLST	    *itmlst;
unsigned long	     itmcode;
long		     itmadr;
long		     itmlen;
unsigned long int   *retlenadr;
long		     index;
{
long		     element_count;
long		     list_len = Get_getlst_length( (struct GET_ITMLST *)itmlst );
struct ITMLST	    *ret_itmlst;

ret_itmlst = (struct ITMLST *) _ImgRealloc( (char *)itmlst, 
					list_len + sizeof(struct ITMLST) );
element_count = list_len / sizeof(struct ITMLST);

ret_itmlst[element_count].ItmL_Code	= itmcode;
ret_itmlst[element_count].ItmL_Length	= itmlen;
ret_itmlst[element_count].ItmA_Buffer	= (char *)itmadr;
ret_itmlst[element_count].ItmA_Retlen	= retlenadr;
ret_itmlst[element_count].ItmL_Index	= index;

return ret_itmlst;
} /* end of _ImgAppendItmlstItem */


struct PUT_ITMLST *_ImgAppendPutlstItem( itmlst, itmcode, itmadr, itmlen,
					    index )
struct PUT_ITMLST   *itmlst;
unsigned long	     itmcode;
long		     itmadr;
long		     itmlen;
long		     index;
{
long		     element_count;
long		     list_len	    = Get_putlst_length( itmlst );
struct PUT_ITMLST   *ret_itmlst;

ret_itmlst = (struct PUT_ITMLST *) _ImgRealloc( (char *)itmlst, 
				    list_len + sizeof(struct PUT_ITMLST) );
element_count = list_len / sizeof(struct PUT_ITMLST);

ret_itmlst[element_count].PutL_Code	= itmcode;
ret_itmlst[element_count].PutL_Length	= itmlen;
ret_itmlst[element_count].PutA_Buffer	= (char *)itmadr;
ret_itmlst[element_count].PutL_Index	= index;

return ret_itmlst;
} /* end of _ImgAppendPutlstItem */


struct GET_ITMLST *_ImgCreateGetlst( element_count )
long	element_count;
{
struct GET_ITMLST   *itmlst;

itmlst = (struct GET_ITMLST *) _ImgCalloc( element_count, 
				    sizeof( struct GET_ITMLST ) );

return itmlst;
} /* end of _ImgCreateGetlst */


struct ITMLST *_ImgCreateItmlst( element_count )
long	element_count;
{
struct ITMLST	*itmlst;

itmlst = (struct ITMLST *) _ImgCalloc( element_count, 
				    sizeof( struct ITMLST ) );

return itmlst;
} /* end of _ImgCreateItmlst */


struct PUT_ITMLST *_ImgCreatePutlst( element_count )
long	element_count;
{
struct PUT_ITMLST   *itmlst;

itmlst = (struct PUT_ITMLST *) _ImgCalloc( element_count, 
				    sizeof( struct PUT_ITMLST ) );

return itmlst;
} /* end of _ImgCreatePutlst */


struct ITMLST *_ImgCvtPutlstToItmlst( putlst )
struct PUT_ITMLST   *putlst;
{
long		     element_count;
struct ITMLST	    *itmlst;
struct ITMLST	    *itmlst_ptr;
struct PUT_ITMLST   *putlst_ptr	    = putlst;;

element_count = _ImgTotalPutlstItems( putlst );
++element_count;

itmlst = _ImgCreateItmlst( element_count );
itmlst_ptr = itmlst;

while ( putlst_ptr->PutL_Code != 0 )
    {
    itmlst_ptr->ItmL_Code	= putlst_ptr->PutL_Code;
    itmlst_ptr->ItmL_Length	= putlst_ptr->PutL_Length;
    itmlst_ptr->ItmA_Buffer	= putlst_ptr->PutA_Buffer;
    itmlst_ptr->ItmA_Retlen	= 0;
    itmlst_ptr->ItmL_Index	= putlst_ptr->PutL_Index;
    ++putlst_ptr;
    ++itmlst_ptr;
    }

return itmlst;
} /* end of _ImgCvtPutlstToItmlst */


void _ImgDeleteItmlst( itmlst )
struct ITMLST *itmlst;
{

_ImgCfree( itmlst );

return;
} /* end of _ImgDeleteItmlst */	


long _ImgExtractGetlstItem( itmlst, itmcode, itmadr, itmlen, retlenadr, index,
				occurance )
struct GET_ITMLST    *itmlst;
unsigned long	      itmcode;
long		     *itmadr;
long		     *itmlen;
unsigned long int   **retlenadr;
long		     *index;
long		      occurance;
{
long		     found_status	= FALSE;
long		     keep_looping	= TRUE;
long		     occurance_count	= 1;
struct GET_ITMLST   *lst_element	= itmlst;

if ( itmlst != 0 )
    for ( ; lst_element->GetL_Code != 0 && keep_looping; ++lst_element )
	{
	if ( lst_element->GetL_Code == itmcode &&
	     occurance == occurance_count++ )
	    {
	    /*
	     * Optimize the code to move instead of memcopy, when possible.
	     */
	    if ( lst_element->GetL_Length <= sizeof(int) )
		*(int *)itmadr	= *((int *)lst_element->GetA_Buffer);
	    else if ( lst_element->GetL_Length <= sizeof(long) )
		*itmadr	= *((long *)lst_element->GetA_Buffer);
	    else
		memcpy(itmadr, lst_element->GetA_Buffer, 
			lst_element->GetL_Length);
	    if ( itmlen != 0 )
		*itmlen		= lst_element->GetL_Length;
	    if ( retlenadr != 0 )
		*retlenadr	= lst_element->GetA_Retlen;
	    if ( index != 0 )
		*index		= lst_element->GetL_Index;

	    found_status	= TRUE;
	    keep_looping	= FALSE;
	    } /* end if */
	} /* end for */

return found_status;
} /* end of _ImgExtractGetlstItem */


long _ImgExtractItmlstItem( itmlst, itmcode, itmadr, itmlen, retlenadr, index,
				occurance )
struct ITMLST	     *itmlst;
unsigned long	      itmcode;
long		     *itmadr;
long		     *itmlen;
unsigned long int   **retlenadr;
long		     *index;
long		      occurance;
{
long		     found_status	= FALSE;
long		     keep_looping	= TRUE;
long		     occurance_count	= 1;
struct ITMLST	    *lst_element	= itmlst;

if ( itmlst != 0 )
    for ( ; lst_element->ItmL_Code != 0 && keep_looping; ++lst_element )
	{
	/*
	** If the item code and the occurance count match up,
	** return the values for the itemlist element
	*/
	if ( lst_element->ItmL_Code == itmcode &&
	     occurance == occurance_count++ )
	    {
	    /*
	    ** Copy the item value (always)
	    */
	    if ( lst_element->ItmL_Length <= sizeof(int) )
		*(int *)itmadr	= *((int *)lst_element->ItmA_Buffer);
	    else if ( lst_element->ItmL_Length <= sizeof(long) )
		*itmadr	= *((long *)lst_element->ItmA_Buffer);
	    else
		memcpy(itmadr, lst_element->ItmA_Buffer, 
			lst_element->ItmL_Length);

	    /*
	    ** Return the item length (optional)
	    */
	    if ( itmlen != 0 )
		*itmlen		= lst_element->ItmL_Length;

	    /*
	    ** Return the return length address (optional)
	    */
	    if ( retlenadr != 0 )
		*retlenadr	= lst_element->ItmA_Retlen;

	    /*
	    ** Return the index value (optional)
	    */
	    if ( index != 0 )
		*index		= lst_element->ItmL_Index;

	    found_status	= TRUE;
	    keep_looping	= FALSE;
	    } /* end if */
	} /* end for */

return found_status;
} /* end of _ImgExtractItmlstItem */


long _ImgExtractItmlstItemAdr( itmlst, itmcode, itmadr, itmlen, retlenadr, 
				index, occurance )
struct ITMLST	     *itmlst;
unsigned long	      itmcode;
long		     *itmadr;
long		     *itmlen;
unsigned long int   **retlenadr;
long		     *index;
long		      occurance;
{
long		     found_status	= FALSE;
long		     keep_looping	= TRUE;
long		     occurance_count	= 1;
struct ITMLST	    *lst_element	= itmlst;

if ( itmlst != 0 )
    for ( ; lst_element->ItmL_Code != 0 && keep_looping; ++lst_element )
	{
	if ( lst_element->ItmL_Code == itmcode &&
	     occurance == occurance_count++ )
	    {
	    *itmadr		= (long)lst_element->ItmA_Buffer;
	    if ( itmlen != 0 )
		*itmlen		= lst_element->ItmL_Length;
	    if ( retlenadr != 0 )
		*retlenadr	= lst_element->ItmA_Retlen;
	    if ( index != 0 )
		*index		= lst_element->ItmL_Index;

	    found_status	= TRUE;
	    keep_looping	= FALSE;
	    } /* end if */
	} /* end for */

return found_status;
} /* end of _ImgExtractItmlstItemAdr */


long _ImgExtractPutlstItem( itmlst, itmcode, itmadr, itmlen, index,
				occurance )
struct PUT_ITMLST   *itmlst;
unsigned long	     itmcode;
long		    *itmadr;
long		    *itmlen;
long		    *index;
long		     occurance;
{
long		     found_status	= FALSE;
long		     keep_looping	= TRUE;
long		     occurance_count	= 1;
struct PUT_ITMLST   *lst_element	= itmlst;

if ( itmlst != 0 )
    for ( ; lst_element->PutL_Code != 0 && keep_looping; ++lst_element )
	{
	if ( lst_element->PutL_Code == itmcode &&
	    occurance == occurance_count++ )
	    {
	    /*
	     * Optimize the code to move instead of memcopy, when possible.
	     */
	    if ( lst_element->PutL_Length <= sizeof(int) )
		*(int *)itmadr	= *((int *)lst_element->PutA_Buffer);
	    else if ( lst_element->PutL_Length <= sizeof(long) )
		*itmadr	= *((long *)lst_element->PutA_Buffer);
	    else
		memcpy( itmadr, lst_element->PutA_Buffer, 
			    lst_element->PutL_Length );
	    if ( itmlen != 0 )
		*itmlen		= lst_element->PutL_Length;
	    if ( index != 0 )
		*index		= lst_element->PutL_Index;

	    found_status	= TRUE;
	    keep_looping	= FALSE;
	    } /* end if */
	} /* end for */

return found_status;
} /* end of _ImgExtractPutlstItem */


long _ImgGetlstItemCount( item_code, itmlst )
unsigned long	     item_code;
struct GET_ITMLST   *itmlst;
{
long		     count	    = 0;
struct GET_ITMLST   *list_element   = itmlst;

if ( itmlst != 0 )
    {
    while ( list_element->GetL_Code != 0 )
	{
	if ( item_code == list_element->GetL_Code )
	    ++count;
	    ++list_element;
	}
    }

return count;
} /* end of _ImgGetlstItemCount */


long _ImgItmlstItemCount( item_code, itmlst )
unsigned long	 item_code;
struct ITMLST	*itmlst;
{
long		 count		= 0;
struct ITMLST	*list_element   = itmlst;

if ( itmlst != 0 )
    {
    while ( list_element->ItmL_Code != 0 )
	{
	if ( item_code == list_element->ItmL_Code )
	    ++count;
	    ++list_element;
	}
    }

return count;
} /* end of _ImgItmlstItemCount */


long _ImgPutlstItemCount( item_code, itmlst )
unsigned long	     item_code;
struct PUT_ITMLST   *itmlst;
{
long		     count	    = 0;
struct PUT_ITMLST   *list_element   = itmlst;

if ( itmlst != 0 )
    {
    while( list_element->PutL_Code != 0 )
	{
	if ( item_code == list_element->PutL_Code )
	    ++count;
	++list_element;
	}
    }

return count;
} /* end of _ImgPutlstItemCount */


long _ImgTotalGetlstItems( itmlst )
struct GET_ITMLST   *itmlst;
{
long		     count	    = 0;
struct GET_ITMLST   *list_element    = itmlst;

if ( itmlst != 0 )
    while ( list_element->GetL_Code != 0 )
	{
	++count;
	++list_element;
	}

return count;
} /* end of _ImgTotalGetlstItems */


long _ImgTotalItmlstItems( itmlst )
struct ITMLST	*itmlst;
{
long		 count		= 0;
struct ITMLST	*list_element   = itmlst;

if ( itmlst != 0 )
    while ( list_element->ItmL_Code != 0 )
	{
	++count;
	++list_element;
	}

return count;
} /* end of _ImgTotalItmlstItems */


long _ImgTotalPutlstItems( itmlst )
struct PUT_ITMLST   *itmlst;
{
long		     count	    = 0;
struct PUT_ITMLST   *list_element    = itmlst;

if ( itmlst != 0 )
    while ( list_element->PutL_Code != 0 )
	{
	++count;
	++list_element;
	}

return count;
} /* end of _ImgTotalPutlstItems */


static long Get_getlst_length( itmlst )
struct GET_ITMLST   *itmlst;
{
long		     list_length    = 0;
struct GET_ITMLST   *lst_element    = itmlst;

if ( itmlst != 0 )
    {
    list_length = sizeof( struct GET_ITMLST );
    while ( lst_element->GetL_Code != 0 )
	{
	list_length += sizeof( struct GET_ITMLST );
	++lst_element;
	}
    }

return list_length;
} /* end of Get_getlst_length */


static long Get_itmlst_length( itmlst )
struct ITMLST	*itmlst;
{
long		 list_length    = 0;
struct ITMLST	*lst_element    = itmlst;

if ( itmlst != 0 )
    {
    list_length = sizeof( struct ITMLST );
    while ( lst_element->ItmL_Code != 0 )
	{
	list_length += sizeof( struct ITMLST );
	++lst_element;
	}
    }

return list_length;
} /* end of Get_itmlst_length */


static long Get_putlst_length( itmlst )
struct PUT_ITMLST   *itmlst;
{
long		     list_length;
struct PUT_ITMLST   *lst_element    = itmlst;

if ( itmlst != 0 )
    {
    list_length = sizeof( struct PUT_ITMLST );
    while ( lst_element->PutL_Code != 0 )
	{
	list_length += sizeof( struct PUT_ITMLST );
	++lst_element;
        }
    }

return list_length;
} /* end of Get_putlst_length */
