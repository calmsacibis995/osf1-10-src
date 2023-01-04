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
Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts,
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

/************************************************************************
**
**  FACILITY:
**
**      X Imaging Extension
**      Sample Machine Independant DDX
**
**  ABSTRACT:
**
**      Changlist manipulation routines for the Ultrix environment.
**      This module replaces the VMS module Img__CHANGE_LIST.MAR.
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
**	VAX/VMS V5.3
**	ULTRIX  V3.1
**
**  AUTHOR(S):
**
**	Written for VMS by John Weber 15-May-1987
**
**      Rewritten for Ultrix by Brian M. Stevens  26-Sep-1988
**       
**      Clean-up and put into library by Michael D. O'Connor 28-Nov-1988
**
**  CREATION DATE:
**
**       26-Sep-1988
**
**  MODIFICATION HISTORY:
**
************************************************************************/
/*
** Table of Contents
*/
int MiBuildChangeList();
void MiPutRuns();

/*
**  Include files
*/
#include "SmiBitMasks.h"
/*
**  MACRO definitions
*/

/*
**  Global Symbols
*/

/*
**  Equated Symbols
*/

/*
**  External References
*/
extern int MiFfsLong();
extern int MiFfcLong();

/************************************************************************
**
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine builds a change list structure from a buffer of image data.
**	Run length transitions are detected by alternately searching for zero
**	bits or one bits with the Mi_FFx_LONG routines.  Bit offsets where the
**	transitions take place are stored in subsequent entries in the change
**	list.
**
**  FORMAL PARAMETERS:
**
**		src_addr;   - scanline buffer address
**		src_pos;    - scanline buffer offset (in bits)
**		size;	    - scanline buffer length (in bytes)
**		cl_addr;    - change list address (ptr to array of long)
**		cl_len;	    - chane list length (num of cells in array)
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
int	MiBuildChangeList(src_addr, src_pos, size, cl_addr, cl_len)
long	src_addr;
long	src_pos;
long	size;
long	cl_addr;
long	cl_len;
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

	    MiFfsLong(pos, size, src_addr, &pos);
	}
	else
	{
	    /*
	    ** look for a transition to a 0 bit
	    */

	    MiFfcLong(pos, size, src_addr, &pos);
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
**
**
**  FUNCTIONAL DESCRIPTION:
**
**      This routine accepts a change list buffer and reconstructs the image
**      data described by it into a data buffer.
**
**      It is assumed that the data buffer contains zeros on entry to this
**      routine, so only run lists of ones are actually written using
**      MiMovv7.
**
**  FORMAL PARAMETERS:
**
**		dst_addr;	- dest. address of image data buffer
**		dst_pos;	- dest. address offset (in bits)
**		cl_addr;	- address of change list (array of long)
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
void
MiPutRuns(dst_addr, dst_pos, cl_addr)
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
        for (bit=(dst_pos + cl[index]) % BYTE_SIZE; size > 0 && bit < BYTE_SIZE; --size, ++bit)
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
