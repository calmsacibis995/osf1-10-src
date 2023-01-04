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
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      Several modules used in common by the FAX decoding software.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**      Revised for V3.0 by Karen Rodwell, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      October 23, 1989
**
************************************************************************/

/*
**  Table of contents:
*/
#ifdef NODAS_PROTO
long _IpsDecodeG42dScan();           /* decode one g42d scanline          */
long _IpsDecodeG31dScan();           /* decode one g31d scanline          */
#endif


/*
**  Include files:
*/
#include        <ips_fax_macros.h>       /* FAX processing macros             */
#include        <ips_fax_definitions.h>  /* FAX definitions                   */
#include        <ips_fax_paramdef.h>     /* FAX routine parameter block defns.*/
#include        <IpsStatusCodes.h>       /* status codes                       */
#ifndef NODAS_PROTO
#include 	<ipsprot.h>		 /* Ips prototypes */
#endif

/*
**  Equated Symbols:
*/
#define WHITE 0
#define BLACK 1
#define BLACK_TABLE_SIZE 8192
#define WHITE_TABLE_SIZE 4096
#define DECODE_TABLE_SIZE 4096
#define ENCODE_TABLE_SIZE 2561
#define MAXCODEWORDSIZE 14

#ifndef TRUE
#define TRUE 1
#endif

/*
**  External references
*/
#ifdef NODAS_PROTO
long _IpsFfsLong();			    /* extended find first set      */
#endif

/* decoding tables for FAX Group 3 & 4                                       */
#if defined(__VAXC) || defined(VAXC)
globalref struct                           /* FAX 1D and 2D decoding tables  */
   {
   unsigned short int value;
   unsigned char length;
   char type;
   } 
   IPS_AR_FAX1D_DECODE_WHITE[WHITE_TABLE_SIZE],
   IPS_AR_FAX1D_DECODE_BLACK[BLACK_TABLE_SIZE],
   IPS_AR_FAX2D_DECODE_TABLE[DECODE_TABLE_SIZE];
#else
extern  struct                           /* FAX 1D and 2D decoding tables  */
   {
#ifdef sparc
   char type;
   unsigned char length;
   unsigned short int value;
#else
   unsigned short int value;
   unsigned char length;
   char type;
#endif
   } 
   IPS_AR_FAX1D_DECODE_WHITE[WHITE_TABLE_SIZE],
   IPS_AR_FAX1D_DECODE_BLACK[BLACK_TABLE_SIZE],
   IPS_AR_FAX2D_DECODE_TABLE[DECODE_TABLE_SIZE];
#endif

/************************************************************************
**
**  _IpsDecodeG42dScan()
**
**  FUNCTIONAL DESCRIPTION:
**
**     Used by the ISL Group 3/2D and Group 4 decoding routines.
**     Decodes a single line encoded in the CCITT FAX Group 4 coding standard.
**
**     This routine decodes codewords by looking them up in a lookup table,
**     indexed by the value of the next group of bits in the buffer to be
**     decoded. The values found in the lookup table give the actual length
**     of the codeword, and its type and value. As an example, in decoding
**     a FAX 2D codeword, 12 bits are taken from the current position in the
**     buffer, and this value is used to index the lookup table. The lookup
**     table could contain the information that the codeword is really
**     4 bits long, is of type TERMINATOR, and has the value VL2 (Vertical
**     Mode Left 2 pixels).
**
**     When a Terminator code is found, its value will be one of the encoding
**     Modes (Horizontal, Pass, or one of the seven Vertical modes). 
**     Action performed is based on which mode is encoded; except in the
**     case of Horizontal mode, only the pointers and changelist are updated.
**     In Horizontal mode, black and white Group 3 runlengths must be 
**     decoded before the pointers are updated.
**
**     When an EOL code type is found, we're done with this scan.
**
**     The routine returns a changelist of all the change positions in the
**     scanline.
**
**  FORMAL PARAMETERS:
**
**	parm		Parameter block.  Passed by reference.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIDE EFFECTS:     none
**  SIGNAL CODES:
**	IpsX_INVCODTYP
**
************************************************************************/
long _IpsDecodeG42dScan(parm)
   struct FAX_DECODE_PARAMS *parm;
   {
   unsigned char code_type,code_len;       /* decode results  */
   short int     code_val;
   long           cidx=0;                   /* current changelist index       */
   long           ridx=1;                   /* reference changelist index     */
   long           a0= -1,a0prime= -1,a1=0,b1=0;/*CCITT standard ptr variables */
   long           x;                        /* temp storage for GET_xx_BITS   */
   long           run_length;               /* length of white or black run   */

   parm->ccl[0] = 0;
   while (parm->ccl[cidx] < parm->pixels_per_line)       /* do until scanline is finished  */
      {
      DECODE_2D_CODEWORD_(parm->base,parm->srcbitptr);

      /* codeword decoded; code type should be TERMINATOR, EOL, or INVALID   */
      /* switch with case for each code_type                                 */
      switch (code_type)
         {
         case IPS_K_FAX2D_CODETYPE_TERMINATOR:        /* switch (code_type)  */

            /* if code type TERMINATOR found, code value will be one of:     */
            /*     PASS, HORIZONTAL, V0, VL1, VR1, VL2, VR2, VL3, or VR3     */
            /* switch with case for each code_value                          */

            switch (code_val)
               {
               case IPS_K_FAX2D_CODEVALUE_PASS:       /* switch (code_val)   */
                  while ( parm->rcl[ridx] <= a0prime || 
			    ((ridx & 1) == (cidx & 1)) )
		    {
                    ridx++;
		    if ( ridx > parm->rcl[0] + 3 )
			/*
			**  If here, we've run off the end of the reference
			**  change list, and are probably headed for an ACCVIO
			*/
			return(IpsX_INVCODTYP);
		    }
                  b1 = parm->rcl[ridx];
                  a0prime = /* b2 = */ parm->rcl[ridx + 1];

                  /* if ridx is too far, move it back                        */
                  if ((ridx > 1) && (parm->rcl[ridx - 1] > a0prime)) 
                     ridx--;
                  break;

               case IPS_K_FAX2D_CODEVALUE_HORIZ:     /* switch (code_val)   */
                  /* in Horiz. mode, must decode black and white 1D runs    */

                  /* move a0 and a0prime up to 0 if necessary               */
                  if (a0 < 0)
                     {
                     a0 = 0;
                     if (a0prime < 0)
                        a0prime = 0;
                     }

                  /* move current line index to next position             */
                  cidx++;
                  if ((cidx & 1) == WHITE_TO_BLACK)
                     {

                     /* should have a white run followed by a black run   */

                     DECODE_WHITE_RUN_(parm->base,parm->srcbitptr);

                     /* adjust runlength if previously in PASS mode       */
                     /* if not, a0 and a0prime should be equal, thus a NOP*/
                     /*run_length += (a0prime - a0);*/
                     /*a1 = a0 + run_length;*/
                     a1 = run_length + a0prime;
                     parm->ccl[cidx++] = a1;
                     parm->ccl[0]++;

                     /* now decode the accompanying BLACK run             */
                     DECODE_BLACK_RUN_(parm->base,parm->srcbitptr);
                     a0 = /* a2 = */ a1 + run_length;
                     parm->ccl[cidx] = a0;
                     a0prime = a0;
                     parm->ccl[0]++;
                     }

                  else
                     {
                     /* we have a black run followed by a white run       */
                     DECODE_BLACK_RUN_(parm->base,parm->srcbitptr);
                     /*run_length += (a0prime - a0);*/
                     /*a1 = a0 + run_length;*/
                     a1 = run_length + a0prime;
                     parm->ccl[cidx++] = a1;
                     parm->ccl[0]++;

                     /* decode the white run                              */
                     DECODE_WHITE_RUN_(parm->base,parm->srcbitptr);
                     a0 = /* a2 = */ a1 + run_length;
                     parm->ccl[cidx] = a0;
                     a0prime = a0;
                     parm->ccl[0]++;
                     }
                  break;
                    
               case IPS_K_FAX2D_CODEVALUE_V0:       /* switch (code_val)   */
               case IPS_K_FAX2D_CODEVALUE_VR1:
               case IPS_K_FAX2D_CODEVALUE_VL1:
               case IPS_K_FAX2D_CODEVALUE_VR2:
               case IPS_K_FAX2D_CODEVALUE_VL2:
               case IPS_K_FAX2D_CODEVALUE_VR3:
               case IPS_K_FAX2D_CODEVALUE_VL3:
                  while ( parm->rcl[ridx] <= a0prime || 
			    ((ridx & 1) == (cidx & 1)) )
		    {
                    ridx++;                        /* move thru ref. line   */
		    if ( ridx > parm->rcl[0] + 3 )
			/*
			**  If here, we've run off the end of the reference
			**  change list, and are probably headed for an ACCVIO
			*/
			return(IpsX_INVCODTYP);
		    }
                  b1 = parm->rcl[ridx];

                  /* if ridx has moved too far, move it back                 */
                  if ((ridx > 1) && (parm->rcl[ridx - 1] > a0prime)) 
                     ridx--;
                  a0 = /* a1 = */ b1 + code_val;
                  parm->ccl[++cidx] = a0;                   /* add to changelist     */
                  a0prime = a0;
                  /*run_length = parm->ccl[cidx] - parm->ccl[cidx - 1];*/
                  parm->ccl[0]++;
                  break;
               default:
                  return(IpsX_INVCODTYP);
                  break;
               }                                    /* end switch (code_val) */
            break;

         case IPS_K_FAX2D_CODETYPE_INVALID:         /* switch (code_type)    */
            return(IpsX_INVCODTYP);
            break;

         case IPS_K_FAX2D_CODETYPE_EOL:             /* switch (code_type)    */
            break;

         default:                                   /* switch (code_type)    */
            return (IpsX_INVCODTYP);
            break;
         }                                          /* end switch(code_type) */

      if (code_type == IPS_K_FAX2D_CODETYPE_EOL)    /* EOL code was detected */
         break;
      }                                             /* end while ccl[cidx]...*/

   /* add extra copies of last change element to prevent particular          */
   /* end-of-line condition from damaging things                             */

   parm->ccl[parm->ccl[0]+1] = parm->pixels_per_line;
   parm->ccl[parm->ccl[0]+2] = parm->pixels_per_line;
   parm->ccl[parm->ccl[0]+3] = parm->pixels_per_line;
   if (parm->ccl[cidx] > parm->pixels_per_line)
      return (IpsX_INVSCNLEN);
   return (IpsX_SUCCESS);
   }

/************************************************************************
**
**  _IpsDecodeG31dScan()
**
**  FUNCTIONAL DESCRIPTION:
**
**     Decode a CCITT FAX Group 3 encoded scanline.
**
**     Decoding proceeds by getting the next group of bits from the encoded
**     buffer (12 for a white, 13 for a black run) and using the value to
**     index a lookup table containing the actual code length, type and value
**     for that codeword. A run (either black or white) consists of 0 or more
**     Makeup type codes, followed by a single Terminator codeword. The length
**     of the run is the sum of the values of the Makeup and Terminator codes.
**     The runlengths are used to create a changelist of the transitions from
**     black to white and vice versa, which is returned to the caller.
**
**     When a codetype EOL is encountered, the end of the scan has been reached.
**
**  FORMAL PARAMETERS:
**
**     parm		Parameter block.  Passed by reference.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIDE EFFECTS:     none
**  SIGNAL CODES:     none
**
************************************************************************/
long _IpsDecodeG31dScan(parm)
   struct FAX_DECODE_PARAMS *parm;
   {
   long           cidx=0;                   /* current changelist index       */
   long           x;                        /* temp storage for GET_xx_BITS   */
   long           run_length;               /* length of white or black run   */
   parm->ccl[0] = 0;
   while(TRUE)                             /* do until scanline EOL          */
      {
      DECODE_WHITE_RUN_(parm->base,parm->srcbitptr);
      if ( IPS_AR_FAX1D_DECODE_WHITE[x].type == 0 )
	return( IpsX_INVCODTYP );
      if(IPS_AR_FAX1D_DECODE_WHITE[x].type == IPS_K_FAX1D_CODETYPE_EOL)
         break;
      cidx++;
      parm->ccl[cidx] = parm->ccl[cidx-1] + run_length;
      DECODE_BLACK_RUN_(parm->base,parm->srcbitptr);
      if ( IPS_AR_FAX1D_DECODE_BLACK[x].type == 0 )
	return( IpsX_INVCODTYP );
      if(IPS_AR_FAX1D_DECODE_BLACK[x].type == IPS_K_FAX1D_CODETYPE_EOL)
         break;
      cidx++;
      parm->ccl[cidx] = parm->ccl[cidx-1] + run_length;
      }

   /* add extra copies of last change element to prevent particular          */
   /* end-of-line condition from damaging things                             */
    if(IPS_AR_FAX1D_DECODE_BLACK[x].value == 1)
        {
        _IpsFfsLong(parm->srcbitptr,1000,parm->base,&x);
        parm->srcbitptr = x + 1;
        }
   parm->ccl[0] = cidx;
   parm->ccl[cidx + 1] = parm->pixels_per_line;
   parm->ccl[cidx + 2] = parm->pixels_per_line;
   parm->ccl[cidx + 3] = parm->pixels_per_line;
   if (parm->ccl[cidx] > parm->pixels_per_line)
      return (IpsX_INVSCNLEN);
   return (IpsX_SUCCESS);
   }
