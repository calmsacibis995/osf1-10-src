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
/************************************************************************
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
**
**  FACILITY:
**
**       Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Changlist manipulation routines for the Ultrix environment.
**      This module replaces the VMS module IPS__CHANGE_LIST.MAR.
**
**      This module contains routines which support the creation of change
**      lists from image data and the reconstruction of image data from
**      change list structures.
**
**      A CHANGELIST is an array of unsigned integers where the zeroeth entry
**      contains the count of the number of valid entries, and a valid entry
**      contains a CHANGE point (bit offset from the beginning of the scanline)
**      from zero to one, or one to zero.
**
**      Entry CHANGELIST[1], and all odd indexed entries contain CHANGEs to
**      "ones"; even entries contain CHANGEs to "zeros".  If the CHANGELIST
**      array is large enough to process the entire scanline the last entry
**      CHANGELIST[CHANGELIST[0]] will contain the length of the scanline (one
**      bit position past the end) and is included in the count.
**
**      The maximum size CHANGELIST array required to contain one scanline
**      is scanline-length plus 2 entries (for the count and scanline length).
**
**  ENVIRONMENT:
**
**       Ultrix V3.0
**
**  AUTHOR(S):
**
**	Written for VMS - John Weber - 15-May-1987
**
**      Rewritten for Ultrix - Brian M. Stevens - 26-Sep-1988
**       
**      Clean-up and put into library - Michael D. O'Connor - 28-Nov-1988
**
**	Updated for use in ported XIE - Richard D. Hennessy  23-Jan-1992
**
**  CREATION DATE:
**
**       26-Sep-1988
**
************************************************************************/

    /*
    ** Table of Contents
    */
int DmiBuildChangeList();
void DmiPutRuns();

int _DmiFfsLong();
int _DmiFfcLong();


/*
**  Include files
*/
#include <X.h>

/*
**  Global Symbols
*/

/*
**  Equated Symbols
*/
#define BYTE_SIZE       8       /* number of bits per byte  */

/*
**  MACRO definitions
*/
    /*	 
    **  Macros for Ffslong, Ffclong
    */	 
/*
** 1st set/clear bit found, store its position in find_pos and return
** success indicator
*/
#define SUCCESS_(bit)   {                                               \
                        *find_pos = (long) (((long)pbyte) - base)        \
                                    * BYTE_SIZE + bit;                  \
                        return(Success);                                \
                        }                                               \

/*
** 1st set/clear bit not found, return a failure indicator (the offset of
** the last bit in the field plus 1 has already been stored in find_pos
*/
#define FAILURE_        {return(-1);}

/*
**  External References
*/


/************************************************************************
**  DmiBuildChangeList - Build a change list from image data
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine builds a change list structure from a buffer of image data.
**	Run length transitions are detected by alternately searching for zero
**	bits or one bits with the Ips__FFx_LONG routines.  Bit offsets where the
**	transitions take place are stored in subsequent entries in the change
**	list.
**
**  FORMAL PARAMETERS:
**
    long		src_addr;   - scanline buffer address
    long		src_pos;    - scanline buffer offset (in bits)
    long		size;	    - scanline buffer length (in bytes)
    long		cl_addr;    - change list address (ptr to array of long)
    long		cl_len;	    - chane list length (num of cells in array)
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
**	Returns the address of the change list.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
int DmiBuildChangeList(src_addr, src_pos, size, cl_addr, cl_len)
    long		src_addr;
    long		src_pos;
    long		size;
    long		cl_addr;
    long		cl_len;
{
    long 		pos;
    long 		end;
    int			index;
    unsigned int	*cl;


    cl = (unsigned int *) cl_addr;
    end = src_pos + size;
    pos = src_pos;

    /*
    **  While there's room in the changlist and data to search,  find
    **  transitions and load them into the change list.
    */
    for (index = 1; index < cl_len && size > 0; index++, size = end - pos)
    {
	if (index % 2)
	{
	    /*
	    ** look for a transition to a 1 bit
	    */

	    _DmiFfsLong(pos, size, src_addr, &pos);
	}
	else
	{
	    /*
	    ** look for a transition to a 0 bit
	    */

	    _DmiFfcLong(pos, size, src_addr, &pos);
	}
	
	/*
	** save the transition point in the changelist
	*/

	cl[index] = pos - src_pos;
    }

    /*
    ** store the number of valid entries
    */

    cl[0] = --index;

    return ((int) cl);
}


/************************************************************************
**  DmiPutRuns - Restore image data from change list
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine accepts a change list buffer and reconstructs the image
**      data described by it into a data buffer.
**
**      It is assumed that the data buffer contains zeros on entry to this
**      routine, so only run lists of ones are actually written using
**      _IpsMovv7.
**
**  FORMAL PARAMETERS:
**
    long		dst_addr;	- dest. address of image data buffer
    long		dst_pos;	- dest. address offset (in bits)
    long		cl_addr;	- address of change list (array of long)
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
**	none
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
************************************************************************/
void DmiPutRuns(dst_addr, dst_pos, cl_addr)
    long		dst_addr;
    long		dst_pos;
    long		cl_addr;
{
    long 		size;
    int			index;
    unsigned int	*cl;
    int 		bit;
    char 		*pbyte;


    cl = (unsigned int *) cl_addr;

    /*
    **  While there are elements in the change list write runs of bits 
    **  into the destination image data buffer.
    **  Note:  Since data buffer starts out as all 0's.  Write only runs of 1's.
    */
    for (index = 1; index < cl[0]; index += 2)
    {
	/*
	**  Find out where to start
	*/
        pbyte = (char *) dst_addr + (dst_pos + cl[index]) / BYTE_SIZE;
	size = cl[index + 1] - cl[index];

	/*
	**  Write bits before starting on byte boundary
	*/
        for (bit=(dst_pos + cl[index]) % BYTE_SIZE; size > 0 && bit < BYTE_SIZE; 
--size, ++bit)
        {
	    *pbyte |= (1 << bit);
        }

	/*
	**  Write ones for as many bytes as needed
	*/
        for (pbyte++; size >= BYTE_SIZE; size -= BYTE_SIZE, pbyte++)
        {
	    *pbyte = 0xff;
        }

	/*
	**  Write ones after last byte boundary
	*/
        for (bit = 0; size > 0; --size, ++bit)
        {
	    *pbyte |= (1 << bit);
        }
    }
}


/************************************************************************
**
**  _DmiFfsLong - Find first set bit
**
**  FUNCTIONAL DESCRIPTION:
**
**	Searches the bit field specified by start_pos, size, and base for
**      the first set bit
**
**  FORMAL PARAMETERS:
**
    long	start_pos;	* starting position (offset)         *
    long	size;		* size of the bit field              *
    long	base;		* base address of the bit field      *
    long	*find_pos;	* returned position of 1st set bit   *
**
**  IMPLICIT INPUTS:
**
**	None
**
**  IMPLICIT OUTPUTS:
**
**	None
**
**  FUNCTION VALUE:
**
**	Returns IpsX_NORMAL for success
**
**  SIGNAL CODES:
**
**      If set bit not found, signals LIB$_NOTFOU
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/

int  _DmiFfsLong(start_pos, size, base, find_pos)
    long	start_pos;	/* starting position (offset)         */
    long	size;		/* size of the bit field              */
    long	base;		/* base address of the bit field      */
    long	*find_pos;	/* returned position of 1st set bit   */

{
    char 		*pbyte;		/* address of byte being searched     */
    char		pbyte_val;	/* value of byte being searched       */
    int 		bit;		/* position of bit being examined     */
    

    *find_pos = start_pos + size;
    pbyte = (char *) base + start_pos / BYTE_SIZE;
    pbyte_val = *pbyte;

    for (bit=start_pos % BYTE_SIZE; size > 0 && bit < BYTE_SIZE; --size, ++bit)
    {
	if (pbyte_val & (1 << bit)) SUCCESS_(bit);
    }

    for (pbyte++; size >= BYTE_SIZE; size -= BYTE_SIZE, pbyte++)
    {
        pbyte_val = *pbyte;

	if (pbyte_val & 0x01) SUCCESS_(0);
	if (pbyte_val & 0x02) SUCCESS_(1);
	if (pbyte_val & 0x04) SUCCESS_(2);
	if (pbyte_val & 0x08) SUCCESS_(3);
	if (pbyte_val & 0x10) SUCCESS_(4);
	if (pbyte_val & 0x20) SUCCESS_(5);
	if (pbyte_val & 0x40) SUCCESS_(6);
	if (pbyte_val & 0x80) SUCCESS_(7);
    }

    pbyte_val = *pbyte;

    for (bit = 0; size > 0; --size, ++bit)
    {
	if (pbyte_val & (1 << bit)) SUCCESS_(bit);
    }
 
    FAILURE_;
}


/************************************************************************
**
**  _DmiFfcLong - Find first clear bit
**
**  FUNCTIONAL DESCRIPTION:
**
**	Searches the bit field specified by start_pos, size, and base for
**      the first clear bit
**
**  FORMAL PARAMETERS:
**
    long	start_pos;	* starting position (offset)         *
    long	size;		* size of the bit field              *
    long	base;		* base address of the bit field      *
    long	*find_pos;	* returned position of 1st set bit   *
**
**  IMPLICIT INPUTS:
**
**	None
**
**  IMPLICIT OUTPUTS:
**
**	None
**
**  FUNCTION VALUE:
**
**	Success  ==  a 0 or a 1 bit found 
**	Failure  ==  neither bit type found (off the edge)
**
**  SIGNAL CODES:
**
**	None
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/

int  _DmiFfcLong(start_pos, size, base, find_pos)
    long	start_pos;	/* starting position (offset)         */
    long	size;		/* size of the bit field              */
    long	base;		/* base address of the bit field      */
    long	*find_pos;	/* returned position of 1st clear bit */

{
    char 		*pbyte;		/* address of byte being searched     */
    char		pbyte_val;	/* value of byte being searched       */
    int 		bit;		/* position of bit being examined     */

    *find_pos = start_pos + size;
    pbyte = (char *) base + start_pos / BYTE_SIZE;
    pbyte_val = ~(*pbyte);

    for (bit=start_pos % BYTE_SIZE; size >= 0 && bit < BYTE_SIZE; --size, ++bit)
    {
	if (pbyte_val & (1 << bit)) SUCCESS_(bit);
    }

    for (pbyte++; size >= BYTE_SIZE; size -= BYTE_SIZE, pbyte++)
    {
        pbyte_val = ~(*pbyte);

	if (pbyte_val & 0x01) SUCCESS_(0);
	if (pbyte_val & 0x02) SUCCESS_(1);
	if (pbyte_val & 0x04) SUCCESS_(2);
	if (pbyte_val & 0x08) SUCCESS_(3);
	if (pbyte_val & 0x10) SUCCESS_(4);
	if (pbyte_val & 0x20) SUCCESS_(5);
	if (pbyte_val & 0x40) SUCCESS_(6);
	if (pbyte_val & 0x80) SUCCESS_(7);
    }

    pbyte_val = ~(*pbyte);

    for (bit = 0; size >= 0; --size, ++bit)
    {
	if (pbyte_val & (1 << bit)) SUCCESS_(bit);
    }
 
    FAILURE_;
}
/*  E N D  of  D M I C H A N G E L I S T . C  */

