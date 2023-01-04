/************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
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
**      Image Processing Services
**
**  ABSTRACT:
**
**      C Macros used in Image Services Library FAX encode/decode routines
**
**  ENVIRONMENT:
**
**      VAX/VMS
**
**  AUTHOR(S):
**
**      Ken MacDonald, Digital Equipment Corp.
**
**  CREATION DATE:
**
**      January, 1987
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  MACRO definitions:
*/


/*
**  For Ultrix (and the non-VAX architectures Ultrix supports),
**  don't access longwords on non-longword aligned boundaries.
*/
#ifndef VMS
#define FAX_READ16_(address) \
   (*((unsigned char *)(address)) + \
   (*(((unsigned char *)(address))+1) << 8) 

#define FAX_XOR_WRITE16_(address,value) {\
   *((unsigned char *)(address)) = ((short int)value) & 0xff; \
   *(((unsigned char *)(address))+1) = (((short int)value) >> 8) & 0xff; }

#define FAX_READ32_(address) \
   (unsigned int) (*((unsigned char *)(address)) + \
   ((unsigned int) (*(((unsigned char *)(address))+1)) << 8) + \
   ((unsigned int) (*(((unsigned char *)(address))+2)) << 16) + \
   ((unsigned int) (*(((unsigned char *)(address))+3)) << 24))

#define FAX_XOR_WRITE32_(address,value) {\
   *(( unsigned char *)(address))    ^=  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) ^= (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) ^= (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) ^= (((int)value) >> 24) & 0xff; }
#else
#define FAX_READ16_(address) \
    (*((short int *)(address)))

#define FAX_READ32_(address) \
    (*((int *)(address)))

#define FAX_WRITE16_(address,value) \
    (*((short int *)(address)) = (short int)value)

#define FAX_WRITE32_(address,value) \
    (*((int *)(address)) = (int)value)

#define FAX_XOR_WRITE16_(address,value) \
    (*((short int *)(address)) ^= (short int)value)

#define FAX_XOR_WRITE32_(address,value) \
    (*((int *)(address)) ^= (int)value)
#endif

   /* add a codeword of given length to the buffer                           */

#define ADD_CODEWORD_(base,bitptr,code,len) \
   FAX_XOR_WRITE32_(base + (bitptr >> 3), code << (bitptr & 0x7));\
   bitptr += len

   /* add a complete white run (makeup + term codes) to buffer (no check)    */
   /* the rl (runlength parameter) is destroyed during this macro            */

#define ENCODE_1D_WHITERUN_(base,bitptr,rl) \
   while (rl >= 2560)\
      {\
      FAX_XOR_WRITE32_(base + (bitptr >> 3), \
      IPS_K_FAX1D_CODEWORD_WHITE2560 << (bitptr & 0x7));\
      bitptr += IPS_K_FAX1D_CODELEN_WHITE2560;\
      rl -= 2560;\
      }\
   if (rl >= 64)\
      {\
      working_run_length = rl & 0xFFFFFFC0;\
      rl &= 0x3F;\
      FAX_XOR_WRITE32_(base + (bitptr >> 3), \
         IPS_AR_FAX1D_ENCODE_WHITE[working_run_length].codeword << \
         (bitptr & 0x7));\
      bitptr += IPS_AR_FAX1D_ENCODE_WHITE[working_run_length].length;\
      }\
   FAX_XOR_WRITE32_(base + (bitptr >> 3), \
      IPS_AR_FAX1D_ENCODE_WHITE[rl].codeword << (bitptr & 0x7));\
   bitptr += IPS_AR_FAX1D_ENCODE_WHITE[rl].length

   /* add a complete black run (makeup + term codes) to buffer (no check)    */
   /* the rl (runlength parameter) is destroyed during this macro            */

#define ENCODE_1D_BLACKRUN_(base,bitptr,rl) \
   while (rl >= 2560)\
      {\
      FAX_XOR_WRITE32_(base + (bitptr >> 3), \
         IPS_K_FAX1D_CODEWORD_BLACK2560 << (bitptr & 0x7));\
      bitptr += IPS_K_FAX1D_CODELEN_BLACK2560;\
      rl -= 2560;\
      }\
   if (rl >= 64)\
      {\
      working_run_length = rl & 0xFFFFFFC0;\
      rl &= 0x3F;\
      FAX_XOR_WRITE32_(base + (bitptr >> 3),\
         IPS_AR_FAX1D_ENCODE_BLACK[working_run_length].codeword << \
         (bitptr & 0x7));\
      bitptr += IPS_AR_FAX1D_ENCODE_BLACK[working_run_length].length;\
      }\
   FAX_XOR_WRITE32_(base + (bitptr >> 3), \
      IPS_AR_FAX1D_ENCODE_BLACK[rl].codeword << (bitptr & 0x7));\
   bitptr += IPS_AR_FAX1D_ENCODE_BLACK[rl].length

   /* get 13 bits from the base and offset specified                         */

#ifndef VMS
#define GET_13_BITS_(base,offset) \
   ((unsigned long) ( base[offset >> 3] + (base[(offset >> 3) + 1] << 8) + \
   (((unsigned long) base[(offset >> 3) + 2]) << 16))) \
   >> (offset & 0x7) & 0x1FFF

   /* get 12 bits from the base and offset specified                         */

#define GET_12_BITS_(base,offset) \
   ((unsigned long) (base[offset >> 3] + (base[(offset >> 3) + 1] << 8) + \
   (((unsigned long) (base[(offset >> 3) + 2])) << 16))) \
   >> (offset & 0x7) & 0xFFF

   /* get 1 bit from the base and offset specified                           */

#define GET_1_BIT_(base,offset) \
   base[offset >> 3] >> (offset & 0x7) & 0x1
#else
#define GET_13_BITS_(base,offset) \
   *((int *)(base + (offset >> 3))) >> (offset & 0x7) & 0x1FFF

   /* get 12 bits from the base and offset specified                         */

#define GET_12_BITS_(base,offset) \
   *((int *)(base + (offset >> 3))) >> (offset & 0x7) & 0xFFF

   /* get 1 bit from the base and offset specified                           */

#define GET_1_BIT_(base,offset) \
   *((int *)(base + (offset >> 3))) >> (offset & 0x7) & 0x1
#endif

   /* decode a 2D CCITT codeword via lookup table; save codetype and value   */

#define DECODE_2D_CODEWORD_(base,offset)  \
   x = GET_12_BITS_(base,offset);\
   code_type = IPS_AR_FAX2D_DECODE_TABLE[x].type;\
   offset += IPS_AR_FAX2D_DECODE_TABLE[x].length;\
   code_val = IPS_AR_FAX2D_DECODE_TABLE[x].value

   /* decode a single black 1D codeword                                      */

#define DECODE_1D_BLACKCODEWORD_(base,offset) \
   x = GET_13_BITS_(base,offset);\
   offset +=IPS_AR_FAX1D_DECODE_BLACK[x].length

   /* decode a single white 1D codeword                                      */

#define DECODE_1D_WHITECODEWORD_(base,offset) \
   x = GET_12_BITS_(base,offset);\
   offset +=IPS_AR_FAX1D_DECODE_WHITE[x].length

   /* decode white run (one or more white codewords; ends with terminator)   */

#define DECODE_WHITE_RUN_(base,offset) \
   run_length = 0;\
   do {\
      DECODE_1D_WHITECODEWORD_(base,offset);\
      run_length += IPS_AR_FAX1D_DECODE_WHITE[x].value;\
      }\
   while (IPS_AR_FAX1D_DECODE_WHITE[x].type == IPS_K_FAX1D_CODETYPE_MAKEUP);

   /* decode black run (one or more black codewords; ends with terminator)   */

#define DECODE_BLACK_RUN_(base,offset) \
   run_length = 0;\
   do {\
      DECODE_1D_BLACKCODEWORD_(base,offset);\
      run_length += IPS_AR_FAX1D_DECODE_BLACK[x].value;\
      }\
   while (IPS_AR_FAX1D_DECODE_BLACK[x].type == IPS_K_FAX1D_CODETYPE_MAKEUP);

/*
**  Equated Symbols:
*/

#define BLACK_TO_WHITE     0
#define WHITE_TO_BLACK     1
#define WHITE 0
#define BLACK 1
