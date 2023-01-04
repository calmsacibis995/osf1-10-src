/********************************************************************************************************************************/
/* Created 29-OCT-1993 16:39:20 by VAX SDL V3.2-12     Source: 13-APR-1991 13:36:47 VAMASD$:[IPS.SRC]IPS_FAX_DEFINITIONS.SDL;1 */
/********************************************************************************************************************************/
/************************************************************************   */
/*  IPS_FAX_DEFINITIONS                                                     */
/*                                                                          */
/*  FACILITY:                                                               */
/*                                                                          */
/*      Image Processing Services                                           */
/*                                                                          */
/*  ABSTRACT:                                                               */
/*                                                                          */
/*      Definitions of constants used by CCITT encoding and decoding        */
/*		routines.                                                   */
/*                                                                          */
/*  ENVIRONMENT:                                                            */
/*                                                                          */
/*      VAX/VMS                                                             */
/*                                                                          */
/*  AUTHOR(S):                                                              */
/*                                                                          */
/*      Mark Sornson, Digital Equipment Corp.                               */
/*                                                                          */
/*  CREATION DATE:                                                          */
/*                                                                          */
/*     February, 1986                                                       */
/*                                                                          */
/*  MODIFICATION HISTORY:                                                   */
/*                                                                          */
/*	Ken MacDonald, Digital Equipment Corporation                        */
/*	   Converted names to Image Service Library conventions (IPS$xxx)   */
/*                                                                          */
/************************************************************************   */
/*+                                                                         */
/* Assorted constants for use in codeword table lookups                     */
/*-                                                                         */
 
/*** MODULE $FAXDEF ***/
#define TAG_K_1D 1                      /*signifies the following line is 1D encoded */
#define TAG_K_2D 0                      /*signifies the following line is 2D encoded */
#define IPS_K_FAX1D_CODETYPE_EOL -1
#define IPS_K_FAX1D_CODETYPE_INVALID 0
#define IPS_K_FAX1D_CODETYPE_MAKEUP 1
#define IPS_K_FAX1D_CODETYPE_TERMINATOR 2
#define IPS_K_FAX2D_CODETYPE_EOL -1
#define IPS_K_FAX2D_CODETYPE_INVALID 0
#define IPS_K_FAX2D_CODETYPE_TERMINATOR 2
#define IPS_K_FAX2D_CODEVALUE_VL3 -3
#define IPS_K_FAX2D_CODEVALUE_VL2 -2
#define IPS_K_FAX2D_CODEVALUE_VL1 -1
#define IPS_K_FAX2D_CODEVALUE_V0 0
#define IPS_K_FAX2D_CODEVALUE_VR1 1
#define IPS_K_FAX2D_CODEVALUE_VR2 2
#define IPS_K_FAX2D_CODEVALUE_VR3 3
#define IPS_K_FAX2D_CODEVALUE_PASS 4
#define IPS_K_FAX2D_CODEVALUE_HORIZ 5
#define IPS_K_FAX2D_CODEVALUE_EOL 0
/*+                                                                         */
/* 2 Dimensional codeword and codeword length definitions used to           */
/* encode image data using either G32D or G42D schemes.                     */
/*                                                                          */
/* NOTE: These are coded as literals rather than as                         */
/*	values in a lookup table because there aren't                       */
/*	very many of them, and the encoding code will                       */
/*	run a little faster.  Decoding codewords are                        */
/*	stored in a table found in the IPS_FAX_DECODE_TABLES                */
/*	module.                                                             */
/*-                                                                         */
#define IPS_K_FAX2D_CODEWORD_PASS 8
#define IPS_K_FAX2D_CODELEN_PASS 4
#define IPS_K_FAX2D_CODEWORD_HORIZ 4
#define IPS_K_FAX2D_CODELEN_HORIZ 3
#define IPS_K_FAX2D_CODEWORD_V0 1
#define IPS_K_FAX2D_CODELEN_V0 1
#define IPS_K_FAX2D_CODEWORD_VR1 6
#define IPS_K_FAX2D_CODELEN_VR1 3
#define IPS_K_FAX2D_CODEWORD_VR2 48
#define IPS_K_FAX2D_CODELEN_VR2 6
#define IPS_K_FAX2D_CODEWORD_VR3 96
#define IPS_K_FAX2D_CODELEN_VR3 7
#define IPS_K_FAX2D_CODEWORD_VL1 2
#define IPS_K_FAX2D_CODELEN_VL1 3
#define IPS_K_FAX2D_CODEWORD_VL2 16
#define IPS_K_FAX2D_CODELEN_VL2 6
#define IPS_K_FAX2D_CODEWORD_VL3 32
#define IPS_K_FAX2D_CODELEN_VL3 7
/*+                                                                         */
/* Added constants for some of the 1D codewords/lengths                     */
/* Ken MacDonald, 12/86                                                     */
/*-                                                                         */
#define IPS_K_FAX1D_CODEWORD_EOL 2048
#define IPS_K_FAX1D_CODELEN_EOL 12
#define IPS_K_FAX1D_CODEWORD_WHITE2560 3968
#define IPS_K_FAX1D_CODELEN_WHITE2560 12
#define IPS_K_FAX1D_CODEWORD_BLACK2560 3968
#define IPS_K_FAX1D_CODELEN_BLACK2560 12
