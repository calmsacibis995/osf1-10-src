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
**	Extension of MOVC5, MOVTC, and MOVC3 (VMS instructions) to operate
**	on larger buffers.  Rewritten in C for use in Ultrix.  This module 
**	replace the corresponding routines in IPS__EXTEND_INSTRUCT.MAR
**
**  ENVIRONMENT:
**
**	Ultrix V3.0
**
**  AUTHOR(S):
**
**	Written in C by Ken MacDonald
**
**	Clean-up and put in library by Michael D. O'Connor  5-Dec-1988
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
void _IpsMovc5Long();
void _IpsMovtcLong();
void IPS__MOVC3_LONG();
#endif

/*
**  Include files
*/
#ifndef NODAS_PROTO
#include <ipsprot.h>				    /* Ips prototypes */
#endif
/*
**  MACRO definitions
*/

/*
** Global Symbols
*/

/*
** Equated Symbols
*/

/*
** External References
*/
void *memcpy();
void *memset();


/*************************************************************************
**
**  _IpsMovc5Long - Move character string of arbitrarty length with fill
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extends the MOVC5 instruction to move greater that 
**	65535 bytes at a time.
**
**  FORMAL PARAMETERS:
**
    long int       srclen;	* Source buffer length in bytes *
    unsigned char *srcbuf;	* Source buffer address		*
    long int       fillbyte;	* Fill byte			*
    long int       dstlen;	* Destination length in bytes	*
    unsigned char *dstbuf;	* Destination buffer address	*
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
**	None
**
**  SIGNAL CODES:
**
**      None
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/
void _IpsMovc5Long(srclen,srcbuf,fillbyte,dstlen,dstbuf)
    long int       srclen;	/* Source buffer length in bytes    */
    unsigned char *srcbuf;	/* Source buffer address	    */
    long int       fillbyte;	/* Fill byte			    */
    long int       dstlen;	/* Destination length in bytes	    */
    unsigned char *dstbuf;	/* Destination buffer address	    */
    {
    if (srclen >= dstlen)
        {
        memcpy (dstbuf, srcbuf, dstlen);
        }
    else
        {
        memcpy (dstbuf, srcbuf, srclen);
        memset (dstbuf+srclen, fillbyte, dstlen-srclen);
        /* fill remaining dstbuf with fill byte.                             */
        }
    return;
    }

/************************************************************************
**
**  _IpsMovtcLong - Move translated char string of arbitrary length with fill
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extends the MOVC5 instruction to move greater that 
**	65535 bytes at a time.
**
**  FORMAL PARAMETERS:
**
    long int       srclen;	* Source length in bytes	*
    unsigned char *srcbuf;	* Source buffer address		*
    long int       fillbyte;	* Fill byte			*
    unsigned char *xlate_table;	* Translate table		*
    long int       dstlen;	* Destination length in bytes	*
    unsigned char *dstbuf;	* Destination buffer address	*
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
**	None
**
**  SIGNAL CODES:
**
**      None
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/
void _IpsMovtcLong(srclen,srcbuf,fillbyte,xlate_table,dstlen,dstbuf)
    long int       srclen;	/* Source length in bytes	*/
    unsigned char *srcbuf;	/* Source buffer address	*/
    long int       fillbyte;	/* Fill byte			*/
    unsigned char *xlate_table;	/* Translate table		*/
    long int       dstlen;	/* Destination length in bytes	*/
    unsigned char *dstbuf;	/* Destination buffer address	*/
    {
    int xlate_size;

    int i;

    xlate_size = dstlen >= srclen ? srclen : dstlen;
    for (i = 0; i < xlate_size;i++)
        dstbuf[i] = xlate_table[srcbuf[i]];
    if (dstlen > srclen)
        memset ( dstbuf+srclen, fillbyte, dstlen-srclen);
    }

/************************************************************************
**
**  IPS__MOVC3_LONG - Move character string of arbitrary length
**
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extends the MOVC3 instruction to move greater that 
**	65535 bytes at a time.
**
**  FORMAL PARAMETERS:
**
    int length;	    * Length of buffer to be copied (in bytes) *
    char *srcbuf;   * Source buffer address			*
    char *dstbuf;   * Destination buffer address		*
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
**	None
**
**  SIGNAL CODES:
**
**      None
**
**  SIDE EFFECTS:
**
**      None
**
************************************************************************/
void IPS__MOVC3_LONG(length,srcbuf,dstbuf)
    int   length;   /* Length of buffer to be copied (in bytes) */
    char *srcbuf;   /* Source buffer address			*/
    char *dstbuf;   /* Destination buffer address		*/
{
    memcpy(dstbuf,srcbuf,length);
}
