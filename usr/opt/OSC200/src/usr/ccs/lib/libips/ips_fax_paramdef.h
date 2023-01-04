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
/*****************************************************************************
** This is an include file for parameter block definitions passed to internal
** ISL IPS$$xxCODE_GxxD_SCAN() functions. The purpose is to eliminate
** much of the overhead time of pushing arguments onto the stack before
** calling each of these routines.
**
** by Ken MacDonald, IPG/ISL project team, 4/87
******************************************************************************
*/

#ifndef IPS_FAX_PARAMDEF_H

#define IPS_FAX_PARAMDEF_H

/*****************************************************************************
** FAX_ENCODE_PARAMS is a structure definition for passing arguments to 
** IPS$$ENCODE_GxxD_SCAN() to avoid the overhead of pushing arguments
** onto the stack.
******************************************************************************
*/

struct FAX_ENCODE_PARAMS
   {
   long           *ccl;                      /* current changelist addr.      */
   long           *rcl;                      /* reference changelist addr.    */
   unsigned char *buf_ptr;                  /* output buffer addr            */
   long           bufbitptr;                 /* output bit offset             */
   };

/*****************************************************************************
** FAX_DECODE_PARAMS is a structure definition for passing arguments to 
** IPS$$DECODE_GxxD_SCAN() to avoid the overhead of pushing arguments
** onto the stack.
******************************************************************************
*/

struct FAX_DECODE_PARAMS
   {
   long           *ccl;                      /* current changelist addr.      */
   long           *rcl;                      /* reference changelist addr.    */
   unsigned char *base;                     /* compressed buffer addr        */
   long           srcbitptr;                 /* compressed buffer bit offset  */
   long           pixels_per_line;           /* output scanline size (bits)   */
   };

#endif
