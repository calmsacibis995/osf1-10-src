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
*         								*
* FACILITY: Image Processing Services								*
*									*
* ABSTRACT:								*
*   C macro definition file for inclusion by various image utility	*
*   routines.								*
*									*
* ENVIRONMENT:								*
*   VAX/VMS, VAX/ULTRIX, RISC/ULTRIX					*
*									*
* AUTHOR:								*
*   Richard Piccolo							*
*									*
* CREATION DATE:                                                        * 
*                                                                       *
*   13-FEB-1990						                *
*									*
************************************************************************/


#define ZERO_TEST_(arg, lt, eq, gt) (arg < 0 ? lt : arg == 0 ? eq : gt)

#ifndef VMS
#define READ16_(address) \
   (*(( unsigned char *)(address)) + \
   (*(((unsigned char *)(address))+1) << 8)

#define READ32_(address) \
   (unsigned int) (*((unsigned char *)(address)) + \
   ((unsigned int) (*(((unsigned char *)(address))+1)) << 8) + \
   ((unsigned int) (*(((unsigned char *)(address))+2)) << 16) + \
   ((unsigned int) (*(((unsigned char *)(address))+3)) << 24))

#define WRITE16_(address,value) {\
   *(( unsigned char *)(address))    =  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) = (((short int)value) >> 8) & 0xff; }

#define AND_WRITE16_(address,value) {\
   *(( unsigned char *)(address))    &=  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) &= (((short int)value) >> 8) & 0xff; }

#define XOR_WRITE16_(address,value) {\
   *(( unsigned char *)(address))    ^=  ((short int)value)       & 0xff; \
   *(((unsigned char *)(address))+1) ^= (((short int)value) >> 8) & 0xff; }

#define WRITE32_(address,value) {\
   *(( unsigned char *)(address))    =  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) = (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) = (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) = (((int)value) >> 24) & 0xff; }

#define AND_WRITE32_(address,value) {\
   *(( unsigned char *)(address))    &=  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) &= (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) &= (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) &= (((int)value) >> 24) & 0xff; }

#define XOR_WRITE32_(address,value) {\
   *(( unsigned char *)(address))    ^=  ((int)value)        & 0xff; \
   *(((unsigned char *)(address))+1) ^= (((int)value) >> 8)  & 0xff; \
   *(((unsigned char *)(address))+2) ^= (((int)value) >> 16) & 0xff; \
   *(((unsigned char *)(address))+3) ^= (((int)value) >> 24) & 0xff; }
#else
#define READ16_(address) \
    (*((short int *)(address)))

#define READ32_(address) \
    (*((int *)(address)))

#define WRITE16_(address,value) \
    (*((short int *)(address)) = (short int)value)

#define AND_WRITE16_(address,value) \
    (*((short int *)(address)) &= (short int)value)

#define XOR_WRITE16_(address,value) \
    (*((short int *)(address)) ^= (short int)value)

#define WRITE32_(address,value) \
    (*((int *)(address)) = (int)value)

#define AND_WRITE32_(address,value) \
    (*((int *)(address)) &= (int)value)

#define XOR_WRITE32_(address,value) \
    (*((int *)(address)) ^= (int)value)
#endif
 
/* put value for 1 bit, described by the base and bit offset.      */

#define PUT_BIT_VALUE_(base,offset,value) \
    (*((unsigned char *)(base + (offset >> 3))) ^= \
	    (unsigned char)((value & 0x1) << (offset & 0x7)))
/*
** note that this macro requires that the left hand side be declared as
** an unsigned char 
*/

#define GET_BIT_VALUE_(base,offset) \
   (*((unsigned char *)(base + (offset >> 3))) >> (offset & 0x7)) & 0x1

#define MIN_(a,b)\
    (a) < (b) ? (a) : (b)

#define MAX_(a,b)\
    (a) > (b) ? (a) : (b)
 
#define VALIDATE_CPP_(cpp, udp)\
    if ((cpp->UdpL_ScnCnt != udp->UdpL_ScnCnt) \
    || (cpp->UdpL_PxlPerScn != udp->UdpL_PxlPerScn) \
    || (cpp->UdpB_Class != UdpK_ClassUBA) \
    || (cpp->UdpB_DType != UdpK_DTypeVU)) \
  return (IpsX_INVDCPP);

#define GET_VALUE_(base,offset,mask) \
   (READ32_((base)+((offset)>>3))>>((offset)&7)&(mask))

/* 
** Complex multiply z = x * y
** Note: z can not be either x or y
*/
# define COMPLEX_MULTIPLY_(x,y,z) \
    z.real = x.real * y.real - x.imag * y.imag;\
    z.imag = x.real * y.imag + x.imag * y.real;\

/* 
** Complex divide z = x / y
** Note: z can not be either x or y
*/
# define COMPLEX_DIVIDE_(x,y,z)\
    /* use z.imag as holder of divisor */\
    z.imag = y.real * y.real + y.imag * y.imag);\
    z.real = (x.real * y.real + x.imag * y.imag) / z.imag; \
    z.imag = (y.real * x.imag - y.imag * x.real) / z.imag; \

/* 
** Complex add z = x + y
*/
# define COMPLEX_ADD_(x,y,z)\
    z.real = x.real + y.real;\
    z.imag = x.imag + y.imag;\

/* 
** Complex subtract z = x - y
*/
# define COMPLEX_SUBTRACT_(x,y,z)\
    z.real = x.real - y.real;\
    z.imag = x.imag - y.imag;\

