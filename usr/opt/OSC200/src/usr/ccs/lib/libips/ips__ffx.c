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
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	Lower level routines for finding the first set or clear bit in a 
**	bit string.
**
**  ENVIRONMENT:
**
**	Ultrix V3.0
**
**  AUTHOR(S):
**
**	Written for Ultrix by Brian Stevens 26-Sep-1988
**
**	Clean-up and put in library by Michael D. O'Connor 5-Dec-1988
**
**  CREATION DATE:
**
**	5-Dec-1988
**
************************************************************************/

/*
** Table of Contents
*/
#ifdef NODAS_PROTO
int _IpsFfsLong();
int IPS__FFX_LONG();
#endif

/*
**  Include files
*/
#ifndef NODAS_PROTO
#include 	<ipsprot.h>			    /* Ips prototypes */
#endif
#include	<ips_bitmasks.h>
#include	<IpsStatusCodes.h>

/*
**  MACRO definitions
*/
/*
 * 1st set/clear bit found, store its position in find_pos and return
 * success indicator
 */
#define SUCCESS_(bit)	{						\
			*find_pos = (long) (pbyte - base)		\
				    * BYTE_SIZE + bit;			\
			return(IpsX_NORMAL);				\
			}						\

/*
 * 1st set/clear bit not found, return a failure indicator (the offset of
 * the last bit in the field plus 1 has already been stored in find_pos
 */
#define FAILURE_	{return(IpsX_FAILURE);}

/*
** Global Symbols
*/

/*
** Equated Symbols
*/

/*
** External References
*/

/************************************************************************
**
**  _IpsFfsLong
**
**  FUNCTIONAL DESCRIPTION:
**
**	Searches the bit field specified by start_pos, size, and base for
**      the first set bit
**
**  FORMAL PARAMETERS:
**
    long		start_pos;	* starting position (offset)         *
    long		size;		* size of the bit field              *
    long		base;		* base address of the bit field      *
    long		*find_pos;	* returned position of 1st set bit   *
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

int _IpsFfsLong(start_pos, size, base, find_pos)
    long		start_pos;	/* starting position (offset)         */
    long		size;		/* size of the bit field              */
    unsigned char	*base;		/* base address of the bit field      */
    long		*find_pos;	/* returned position of 1st set bit   */
{
    unsigned char 	*pbyte;		/* address of byte being searched     */
    unsigned char	pbyte_val;	/* value of byte being searched       */
    int 		bit;		/* position of bit being examined     */
    

    *find_pos = start_pos + size;
    pbyte = (unsigned char *) base + start_pos / BYTE_SIZE;
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
**  _IpsFfcLong
**
**  FUNCTIONAL DESCRIPTION:
**
**	Searches the bit field specified by start_pos, size, and base for
**      the first clear bit
**
**  FORMAL PARAMETERS:
**
    long		start_pos;	* starting position (offset)         *
    long		size;		* size of the bit field              *
    long		base;		* base address of the bit field      *
    long		*find_pos;	* returned position of 1st set bit   *
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
**      If clear bit not found, signals LIB$_NOTFOU
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/

int _IpsFfcLong(start_pos, size, base, find_pos)
    long		start_pos;	/* starting position (offset)         */
    long		size;		/* size of the bit field              */
    unsigned char	*base;		/* base address of the bit field      */
    long		*find_pos;	/* returned position of 1st clear bit */
{
    unsigned char 	*pbyte;		/* address of byte being searched     */
    unsigned char	pbyte_val;	/* value of byte being searched       */
    int 		bit;		/* position of bit being examined     */

    *find_pos = start_pos + size;
    pbyte = (unsigned char *) base + start_pos / BYTE_SIZE;
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
