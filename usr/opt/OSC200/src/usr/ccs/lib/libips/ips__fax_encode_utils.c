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
**      Several modules used in common by the FAX encoding software.
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
void _IpsEncodeG42dScan();           /* encode one g42d scanline          */
void _IpsEncodeG31dScan();           /* encode one g31d scanline          */
#endif


/*
**  Include files:
*/
#include        <ips_fax_macros.h>       /* FAX processing macros             */
#include        <ips_fax_definitions.h>  /* FAX definitions                   */
#include        <ips_fax_paramdef.h>     /* FAX routine parameter block defns.*/
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
#define TRUE 1
#define MAXCODEWORDSIZE 14
 
/* Image Services Library routines                                           */
#ifdef NODAS_PROTO
long _IpsFfsLong();                      /* extended find first set          */
long _IpsFfcLong();                      /* extended find first clear        */
#endif

/* encoding tables for FAX Group 3                                           */
#if defined(__VAXC) || defined(VAXC)
globalref struct
   {
   short int length;                       /* codeword length                */
   short int codeword;                     /* codeword                       */
   }
   IPS_AR_FAX1D_ENCODE_WHITE[ENCODE_TABLE_SIZE],
   IPS_AR_FAX1D_ENCODE_BLACK[ENCODE_TABLE_SIZE];
#else
extern struct
   {
#ifdef sparc
   short int codeword;                     /* codeword                       */
   short int length;                       /* codeword length                */
#else
   short int length;                       /* codeword length                */
   short int codeword;                     /* codeword                       */
#endif
   }
   IPS_AR_FAX1D_ENCODE_WHITE[ENCODE_TABLE_SIZE],
   IPS_AR_FAX1D_ENCODE_BLACK[ENCODE_TABLE_SIZE];
#endif

/*
**  External references
*/
#ifdef NODAS_PROTO
int abs();                            /* integer absolute value from stdlib */
#endif

/************************************************************************
**
**  _IpsEncodeG42dScan()
**
**  FUNCTIONAL DESCRIPTION:
**
**     Used by the ISL FAX encoding routines for Group 3/2D and Group 4.
**     Encodes a single scan line in Group 4 CCITT format, and creates
**     a reference changelist for use in the encoding of subsequent scans.
**
**     This version encodes the scanline without checking for end-of-buffer
**     conditions, assuming that the parent routine has checked that enough
**     room for the maximum size possible scanline is available.
**
**     Group 4 coding is always done by encoding the changes (black to white,
**     or vice-versa) with references to the changes on the previous line.
**     The image is assumed to start with an imaginary white line. Several
**     modes are used depending on how well the current line changes
**     correspond to the previous line. These are Horizontal mode and
**     Pass mode, used when the lines do not match well, and seven Vertical
**     modes, used when a change on the current line is within plus or
**     minus 3 pixels of the reference change.
**
**     In this routine, the BUILD_CHANGELIST function is called to identify
**     the change positions in the current line. The change positions are
**     compared to the change positions in the reference line, and the mode
**     is identified. A codeword identifying the appropriate mode is written
**     to the encoded output buffer. In Pass and Vertical modes, this is all
**     that is done; in Horizontal mode, FAX Group 3 length codewords are
**     now added to identify the change position. Both a black and a white
**     set of Group 3 codewords are always written.
**
**     The logic of the program is concerned mainly with keeping track of
**     a variety of change positions denoted by a0, a0prime, a1, a2, b0, b1
**     and keeping proper position in the current and reference changelists
**     (ccl[] and rcl[] ).
**
**  FORMAL PARAMETERS:
**      parm		Parameter block.  Passed by reference.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
void _IpsEncodeG42dScan(parm)
   struct FAX_ENCODE_PARAMS *parm;
   {
   long           a0 = -1, a0prime= -1, a1,b1,a2,b2; /* CCITT pointer names   */
   long           i;
   long           curr_pos=0;
   long           ridx = 1;
   long           run_length,working_run_length;
   unsigned char  run_color=WHITE;
   unsigned char  code_len;
   unsigned long  code_word;
   short int     code_val;

   for(i = 1; i <= parm->ccl[0];)
      {
      run_length = parm->ccl[i] - curr_pos;
      curr_pos = parm->ccl[i];
      a1 = a0prime + run_length + (a0prime < 0 ? 1 : 0);

      /* detect b1, b2                                                    */
      /* backup ridx to prevent a specific pass mode condition from       */
      /* going undetected                                                 */

      if (ridx > 1 && parm->rcl[ridx-1] > a0prime)
         ridx--;
      while (parm->rcl[ridx] <= a0prime || (ridx & 1) == run_color)
         ridx++;
      b1 = parm->rcl[ridx];
      b2 = parm->rcl[ridx+1];

      /* detect proper coding mode                                        */
      if (b2 < a1)                        /* pass mode                    */
         {
         ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,IPS_K_FAX2D_CODEWORD_PASS,
            IPS_K_FAX2D_CODELEN_PASS);
         a0prime = b2;

         /* must back up bit pointers to reclaim difference between       */
         /* the new value of a0 (a0prime) and a1                          */
         curr_pos -= a1 - a0prime;        /* cp = cp - (a1-a0')           */
         }
      else
         {
         if (abs(a1 - b1) <= 3)           /* one of the vertical modes    */
            {
            switch (a1 - b1)
               {
               case (0):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_V0,IPS_K_FAX2D_CODELEN_V0);
                  break;
               case (-1):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VL1,IPS_K_FAX2D_CODELEN_VL1);
                  break;
               case (1):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VR1,IPS_K_FAX2D_CODELEN_VR1);
                  break;
               case (-2):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VL2,IPS_K_FAX2D_CODELEN_VL2);
                  break;
               case (2):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VR2,IPS_K_FAX2D_CODELEN_VR2);
                  break;
               case (-3):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VL3,IPS_K_FAX2D_CODELEN_VL3);
                  break;
               case (3):
                  ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_VR3,IPS_K_FAX2D_CODELEN_VR3);
                  break;
               default:
                  break;
               }
            if (a0 < 0)
               {
               a0 = 0;
               if (a0prime < 0)
                  a0prime = 0;
               }
            run_color ^= 1;               /* change run color                */

            a0prime = a1;
            a0 = a0prime;
            }
         else                             /* horizontal mode                 */
            {
            ADD_CODEWORD_(parm->buf_ptr,parm->bufbitptr,
                     IPS_K_FAX2D_CODEWORD_HORIZ,IPS_K_FAX2D_CODELEN_HORIZ);
            /* update change list and index                                  */
            if (a0 < 0)
               {
               a0 = 0;
               if (a0prime < 0)
                  a0prime = 0;
               }
            i++;
            if (run_color == WHITE)
               {
               ENCODE_1D_WHITERUN_(parm->buf_ptr,parm->bufbitptr,run_length);
               }
            else
               {
               ENCODE_1D_BLACKRUN_(parm->buf_ptr,parm->bufbitptr,run_length);
               }

            /* detect a2; get next runlength                              */

            run_color ^= 1;
            run_length = parm->ccl[i] - curr_pos;
            curr_pos = parm->ccl[i];
            a2 = a1 + run_length;

            if (run_color == WHITE)
               {
               ENCODE_1D_WHITERUN_(parm->buf_ptr,parm->bufbitptr,run_length);
               }
            else
               {
               ENCODE_1D_BLACKRUN_(parm->buf_ptr,parm->bufbitptr,run_length);
               }
            run_color ^= 1;
            a0prime = a2;
            a0 = a0prime;
            }
         }
      if (curr_pos == parm->ccl[i])
         i++;
      }                                   /* end of for loop             */
   return;
   }

/************************************************************************
**
**  _IpsEncodeG31dScan()
**
**  FUNCTIONAL DESCRIPTION:
**
**     Used by the ISL Group 3/1D and Group 3/2D encoding routines.
**     Encode a scanline in the CCITT FAX Group 3/1D coding standard,
**     and create a reference changelist for use by subsequent Group 4
**     encoded scanlines (i.e. when used in the Group 3/2D routine)
**
**     Group 3 lines are encoded as a series of white and black runs, whose
**     lengths are represented by codewords of varying length. In this routine,
**     _IpsBuildChangelist is called to determine the lengths of the runs
**     in the image. The first run of a line is always white, even if zero
**     length. For each run of a given color, one or more codewords may be
**     used to represent the length. The codes are either Makeup codes, used
**     to represent lengths from 64 to 2560 pixels in multiples of 64; or
**     Terminator codes which represent lengths from 0 to 63. A Terminator
**     code is required at the end of each white or black run. Makeup codes
**     will be present or not, depending on whether the run length is 
**     greater than 63 pixels. As an example, a run of 143 pixels would
**     be represented by the Makeup code for 128 followed by the Terminator
**     code for 15.
**
**     This routine picks the appropriate codewords and inserts them into the
**     encoded output buffer.
**
**  FORMAL PARAMETERS:
**
**      parm		Parameter block.  Passed by reference.
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
void _IpsEncodeG31dScan(parm)
struct FAX_ENCODE_PARAMS *parm;
    {
    long           i;
    long           curr_pos=0;
    long           run_length,working_run_length;
    unsigned char run_color=WHITE;

    unsigned char code_len;
    unsigned long code_word;
    short int     code_val;

    for(i = 1; i <= parm->ccl[0];i++)
        {
        run_length = parm->ccl[i] - curr_pos;     /* get runlength from changelist   */
        curr_pos = parm->ccl[i];
        if (run_color == WHITE)
	    {
            ENCODE_1D_WHITERUN_(parm->buf_ptr,parm->bufbitptr,run_length);
	    }
        else
	    {
            ENCODE_1D_BLACKRUN_(parm->buf_ptr,parm->bufbitptr,run_length);
	    }
        run_color ^= 1;                     /* switch color                    */
        }                                   /* end of for loop                 */
     return;
     }
