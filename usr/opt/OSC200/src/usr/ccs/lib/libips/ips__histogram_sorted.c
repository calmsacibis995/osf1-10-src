/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved. 
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S. 
**  Government is subject to restrictions as set forth in Subparagraph 
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**  IPS__HISTOGRAM_SORTED
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This function effects the collection and storage of the histogram
**	data for explicitly indexed histogram tables.
**
**  FORMAL PARAMETERS:
**
**	udp         pointer to source udp
**
** 	cpp	    control processing plane
** 
**	sort_type   flag indicating whether to sort by frequency or by pixel
**
**	tab_ptr	    pointer to a longword pointer to histogram table
**
**	tab_cnt	    Number of elements in table
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	status
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX RISC/ULTRIX
**
**  AUTHOR(S):
**
**	John Poltrack
**	Revised for V3.0 by John Poltrack 14-NOV-89
**	Revised for IPS Interface by Richard Piccolo 24-APR-90
**
**  CREATION DATE:
**
**	31-JAN-1988
**
************************************************************************/
/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long	    _IpsHistogramSorted();
#endif

/*
** Include files
*/
#include <IpsDef.h>			    /* Image definitions	    */
#include <IpsMacros.h>                      /* IPS Macro Definitions        */
#include <IpsStatusCodes.h>		    /* Status Codes		    */
#include <IpsMemoryTable.h>		    /* IPS Memory Mgt. Functions    */
#ifndef NODAS_PROTO
#include <ipsprot.h>			    /* IPS Prototypes		    */
#endif
#include <math.h>			    /* C math functions		    */

/*
**  Equated Symbols:
**
*/
#define INITIAL_TABLE_SIZE  128 /* Initial number of explicit table entries */

/*
**	This macro puts the data into the histogram table for
**	explicitly indexed tables.  It creates new entries for each
**	value found in the data, and increments the frequency count.
*/
#define TABLE_INSERTION_\
    new_entry = 0;\
    for (entry = (struct EXPLICIT_TABLE_ENTRY *) table;\
	(entry->pixel_value != pixel_value) && \
	    (entry->frequency != 0); entry++);\
    /*\
    ** After condition is met (i.e. either pixel_value has \
    ** matched or freq = 0, determine if pixel_value is unique, \
    ** if so make entry \
    */\
    entry->pixel_value = pixel_value;\
    if (entry->frequency == 0)\
	new_entry = 1;\
    /*\
    ** In either case increment the frequency count of pixel_value\
    */\
    entry->frequency++;\
    entry_count += new_entry;\
    if (new_entry == 1)\
        {\
        entry++;\
        entry->frequency = 0;\
        entry->pixel_value = 0;\
	}\


/*
**  Local Storage:
**
*/
struct	EXPLICIT_TABLE_ENTRY
    {
    unsigned long pixel_value;
    unsigned long frequency;
    };

/*
**  External routines
*/
void qsort();                                   /* sort routine from stdlib */
/* 
** Internal routines
*/
#ifdef NODAS_PROTO
static int _IpsCompareByPixel();
static int _IpsCompareByFrequency();
static unsigned long _IpsSortTable();
#else
PROTO(static unsigned long _IpsSortTable, (unsigned long */*table*/, unsigned long /*count*/, unsigned long /*sort_type*/));
PROTO(static int _IpsCompareByPixel, (struct EXPLICIT_TABLE_ENTRY */*E1*/, struct EXPLICIT_TABLE_ENTRY */*E2*/));
PROTO(static int _IpsCompareByFrequency, (struct EXPLICIT_TABLE_ENTRY */*E1*/, struct EXPLICIT_TABLE_ENTRY */*E2*/));
#endif

long _IpsHistogramSorted(udp, cpp, sort_type, tab_ptr, tab_cnt)
struct UDP    *udp;
struct UDP    *cpp;
unsigned long sort_type;
unsigned long	**tab_ptr;
unsigned long	*tab_cnt;
{
struct EXPLICIT_TABLE_ENTRY *entry;	    /* pointer to table structure   */
unsigned long	line_pad;		    /* number of pad		    */
unsigned long	pixel_value;		    /* pixel value		    */
unsigned char	*src_byte_ptr;		    /* byte pointer to udp	    */
unsigned short  *src_word_ptr;
unsigned int	*src_long_ptr;
unsigned long	*table;			    /* histogram table		    */ 
unsigned long	ix,iy;			    /* loop entities		    */
unsigned long	current_entries;	    /* number of 2 longword entries */
unsigned long	entry_count;		    /* number of actual entries     */
unsigned long	table_size, new_size;	    /* allocation sizes for table   */
unsigned long   count;
unsigned long   status;
unsigned long	new_entry;
unsigned char   *cpp_ptr;		    /* control proc plane base     */
unsigned long   cpp_stride;		    /* control proc plane stride   */
    
if (cpp != 0)
    VALIDATE_CPP_(cpp, udp);
 
/*
** validate sort type 
*/
switch (sort_type)
    {
    case IpsK_ByPixelValue:
    case IpsK_ByFreq:
    break;
    default:
	return (IpsX_INVDARG);
    break;
    }

/*
** validate udp
*/
switch (udp->UdpB_Class)
    {
    case UdpK_ClassA:
	/*
	** Atomic array
	*/
	switch (udp->UdpB_DType)
	    {
	    case UdpK_DTypeBU:
	    case UdpK_DTypeWU:
	    case UdpK_DTypeLU:
	    break;
	    case UdpK_DTypeF:
	    case UdpK_DTypeVU:
	    case UdpK_DTypeV:
	    default:
		return(IpsX_UNSOPTION);
	    break;
	    }/* end switch on DType */
	break;
	case UdpK_ClassUBA:				
	case UdpK_ClassCL:
	case UdpK_ClassUBS:
	default:
	    return(IpsX_UNSOPTION);
	break;
    }/* end switch on class */

/*
** Determine table initial and realloc table sizes 
*/
new_size = table_size = 2 * sizeof(long) * INITIAL_TABLE_SIZE;
table_size += (2 * sizeof (long)); /* Table must be zero terminated (+8) */

table = (unsigned long*)
    (*IpsA_MemoryTable[IpsK_Alloc]) (table_size, IpsM_InitMem, 0);
if (!table)
    return (IpsX_INSVIRMEM);

current_entries = INITIAL_TABLE_SIZE;
entry_count = 0;

switch (udp->UdpB_DType)
    {
    case UdpK_DTypeBU:
	/*
	** Dynamically track new entries into table
	*/
	line_pad = (udp->UdpL_ScnStride>>3) - udp->UdpL_PxlPerScn;
	src_byte_ptr = udp->UdpA_Base + (udp->UdpL_Pos>>3) +
	    ((udp->UdpL_ScnStride>>3) * udp->UdpL_Y1) + udp->UdpL_X1;
	/*
	** Get the component data value and increment its frequency count.
	** Outer loop increments scanlines.  
	*/
	if (cpp != 0)
	    {	
            cpp_ptr = (unsigned char *) cpp->UdpA_Base
                        + (((cpp->UdpL_Pos +
                          (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                           cpp->UdpL_X1)+7)>>3);
            cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;

	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_byte_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals currently 
		    ** alloc'd space
		    */
		    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])
				    (table, table_size);
		        if (!table)
			    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
		        }
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			{
		        pixel_value = *src_byte_ptr;
		        TABLE_INSERTION_;
			}
		    src_byte_ptr++;
		    }/* End pixel for loop */
                cpp_ptr += cpp_stride;
	        }/* End scanline for loop */
	    }/* End histogram using cpp */
	else
	    {
	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_byte_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals 
		    ** currently alloc'd space
		    */
		    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;	
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])
					(table, table_size);
		        if (!table)
		  	    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
			}
	  	    pixel_value = *src_byte_ptr++;
		    TABLE_INSERTION_;
		    }/* End pixel for loop */
	        }/* End scanline for loop */
	    }
    break;   /* end of byte src udp */

    case UdpK_DTypeWU:
	/*
	** Dynamically track new entries into table
	*/
	line_pad = (udp->UdpL_ScnStride>>4) - udp->UdpL_PxlPerScn;
	src_word_ptr = (unsigned short *)udp->UdpA_Base + (udp->UdpL_Pos>>4) +
	    ((udp->UdpL_ScnStride>>4) * udp->UdpL_Y1) + udp->UdpL_X1;
	/*
	** Get the component data value and increment its frequency count.
	** Outer loop increments scanlines.  
	*/
	if (cpp != 0)
	    {	
            cpp_ptr = (unsigned char *) cpp->UdpA_Base
                        + (((cpp->UdpL_Pos +
                          (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                           cpp->UdpL_X1)+7)>>3);
            cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;

	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_word_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals currently 
		    ** alloc'd space
		    */
		    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])
				    (table, table_size);
		        if (!table)
			    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
		        }
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			{
		        pixel_value = *src_word_ptr;
		        TABLE_INSERTION_;
			}
		    src_word_ptr++;
		    }/* End pixel for loop */
                cpp_ptr += cpp_stride;
	        }/* End scanline for loop */
	    }/* End histogram using cpp */
	else
	    {
	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_word_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals 
		    ** currently alloc'd space
		    */
		    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])
				(table, table_size);
		        if (!table)
			    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
		        }
		    pixel_value = *src_word_ptr++;
		    TABLE_INSERTION_;
		    }/* End pixel for loop */
	        }/* End scanline for loop */
	    }
    break;   /* end of word src udp */

    case UdpK_DTypeLU:
	/*
	** Dynamically track new entries into table
	*/
	line_pad = (udp->UdpL_ScnStride>>5) - udp->UdpL_PxlPerScn;
	src_long_ptr = (unsigned int *)udp->UdpA_Base + (udp->UdpL_Pos>>5) +
	    ((udp->UdpL_ScnStride>>5) * udp->UdpL_Y1) + udp->UdpL_X1;

	/*
	** Get the component data value and increment its frequency count.
	** Outer loop increments scanlines.  
	*/
	if (cpp != 0)
	    {	
            cpp_ptr = (unsigned char *) cpp->UdpA_Base
                        + (((cpp->UdpL_Pos +
                          (cpp->UdpL_ScnStride * cpp->UdpL_Y1) +
                           cpp->UdpL_X1)+7)>>3);
            cpp_stride = (cpp->UdpL_ScnStride + 7)>>3;

	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_long_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals currently 
		    ** alloc'd space
		    */
		    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])
				    (table, table_size);
		        if (!table)
			    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
		        }
                    if ((GET_BIT_VALUE_(cpp_ptr,ix)) == TRUE)
			{
		        pixel_value = *src_long_ptr;
		        TABLE_INSERTION_;
			}
		    src_long_ptr++;
		    }/* End pixel for loop */
                cpp_ptr += cpp_stride;
	        }/* End scanline for loop */
	    }/* End histogram using cpp */
	else
	    {
	    for (iy = 0; iy < udp->UdpL_ScnCnt; iy++, src_long_ptr+=line_pad)
	        {
	        for (ix = 0; ix < udp->UdpL_PxlPerScn; ix++)
		    {
		    /*
		    ** Realloc extra space if entry count equals 
		    ** currently alloc'd space
		    */
 	    	    if (entry_count == current_entries)
	    	        {
		        current_entries += INITIAL_TABLE_SIZE;
		        table_size += new_size;
		        table = (unsigned long*)
			    (*IpsA_MemoryTable[IpsK_Realloc])(table,table_size);
		        if (!table)
			    {
			    (*IpsA_MemoryTable[IpsK_Dealloc])(table);
			    return (IpsX_INSVIRMEM);
			    }
		        }
		    pixel_value = *src_long_ptr++;
		    TABLE_INSERTION_;
		    }/* End pixel for loop */
	        src_long_ptr += line_pad;
	        }/* End scanline for loop */
	    }
    break;   /* end of long src udp  */
    } /* end of switch statement */

/*
** Reset the entry_count to the true size of the table stopping
** at the end of the table (i.e. frequency == 0)
*/
count = 0;
entry = (struct EXPLICIT_TABLE_ENTRY *)table;

while ((entry->frequency != 0) && (count <= entry_count))
    {
    entry++;
    count++;
    };

*tab_ptr = table;
*tab_cnt = count;

/* sort based on the sort type flag */

status = _IpsSortTable (table, count, sort_type);

return (status);
} /* end of _IpsHistogramSorted */


/******************************************************************************
**  _IpsSortTable
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function sorts the histogram table for explicitly indexed
**	histograms.
**
**  FORMAL PARAMETERS:
**
**	table		Histogram table pointer
**	count		Number of entries
**	sort_type	sort type - by pixel or by frequency
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static unsigned long _IpsSortTable( table, count, sort_type)
unsigned long * table;
unsigned long count;
unsigned long sort_type;
{
/*
** Pointer to function passed by reference to qsort
*/
int (*compar_func)();

/*
** Determine which elements to compare using qsort
*/

switch (sort_type)
    {
    case IpsK_ByPixelValue:
	compar_func = _IpsCompareByPixel;    
    break;
    case IpsK_ByFreq:
        compar_func = _IpsCompareByFrequency;
    break;
    default:
	return (IpsX_INVDARG);
    break;
    }

qsort( (char*)table, count, sizeof(struct EXPLICIT_TABLE_ENTRY), compar_func);

return (IpsX_SUCCESS);
} /* end of _IpsSortTable */

/*******************************************************************************
**  _IpsCompareByPixel
**
**  FUNCTIONAL DESCRIPTION:
**
**	These functions compare two elements of an array and are called by
**	qsort.
**
**  FORMAL PARAMETERS:
**
**	Pointers to elements of array
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	negative value	if E1 < E2
**	zero		if E1 = E2
**	positive value	if E1 > E2
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static int _IpsCompareByPixel(E1,E2)
struct EXPLICIT_TABLE_ENTRY *E1, *E2;
    {
    return (E1->pixel_value - E2->pixel_value);
    }

/*******************************************************************************
**  _IpsCompareByFrequency
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function is called by qsort to compare two frequency elements.
**	If the elements are equal, the comparison is made between a secondary
**	pixel value element.
**
**  FORMAL PARAMETERS:
**
**	Pointers to entry in a matched table of longwords.
**
**		entry 1: 
**
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	negative value	if E1 < E2 or if E1 = E2
**
**	positive value	if E1 > E2
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static int _IpsCompareByFrequency(E1,E2)
struct EXPLICIT_TABLE_ENTRY *E1, *E2;
    {
    /*
    ** If frequencies are equal, use pixel_value as a secondary sort key
    */
    if (E1->frequency != E2->frequency)
	return (E1->frequency - E2->frequency);
    else
	return (E1->pixel_value - E2->pixel_value);
    }
