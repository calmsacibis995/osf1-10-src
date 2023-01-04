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
**  IpsMemoryTable.H
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This include file contains an external reference to the ISC memory
**	dispatch table and defines symbols for each supported Layer II routine.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Richard Piccolo
**
**  CREATION DATE:
**
**	13-FEB-1990
**
************************************************************************/

/* define symbols for each supported Layer II function */

# define IpsK_Alloc				0
# define IpsK_Dealloc				1
# define IpsK_Realloc				2
# define IpsK_AllocateDataPlane			3 
# define IpsK_FreeDataPlane			4 
# define IpsK_ReallocateDataPlane		5 

# define IpsM_InitMem				1

#if defined(__VAXC) || defined(VAXC)
globalref unsigned char * (*IpsA_MemoryTable[])();
#else
extern unsigned char * (*IpsA_MemoryTable[])();
#endif                      

/* end of IpsMemoryTable.h */
