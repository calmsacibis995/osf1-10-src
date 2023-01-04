/******************************************************************************
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
**  IpsDefP.h
**
**  FACILITY:
**
**	Image Processing Services (IPS)
**
**  ABSTRACT:
**
**	This module contains private data structures
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
**	21-MAR-1991
**
************************************************************************/


#ifndef IPSDEFP_H 	/* added mulitiple inclusion protection on 11/01/93 */
#define IPSDEFP_H


struct Complex_data {
    float real;
    float imag;
    } ;
typedef struct Complex_data COMPLEX;


#endif
