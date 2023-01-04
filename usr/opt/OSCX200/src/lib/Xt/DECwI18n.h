/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*
*****************************************************************************

	      Copyright (c) Digital Equipment Corporation, 1990  
	      All Rights Reserved.  Unpublished rights reserved
	      under the copyright laws of the United States.
	      
	      The software contained on this media is proprietary
	      to and embodies the confidential technology of 
	      Digital Equipment Corporation.  Possession, use,
	      duplication or dissemination of the software and
	      media is authorized only pursuant to a valid written
	      license from Digital Equipment Corporation.

	      RESTRICTED RIGHTS LEGEND   Use, duplication, or 
	      disclosure by the U.S. Government is subject to
	      restrictions as set forth in Subparagraph (c)(1)(ii)
	      of DFARS 252.227-7013, or in FAR 52.227-19, as
	      applicable.

*****************************************************************************
**++
**  FACILITY:
**
**	Internationalization RTL (I18n RTL)
**
**  ABSTRACT:
**
**	Internationalization Services header file for DECwindows 
**
**
**  MODIFICATION HISTORY:
**
**	
**
**--
**/

#ifndef _I18N_h
#define _I18N_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif


typedef struct _I18nContextRec *I18nContext;

typedef struct _I18nFontListRec *I18nFontList;

typedef struct _I18nXlibBufferRec *I18nXlibBuffers;

typedef enum {I18NNOUN, I18NVERB, I18NADJECTIVE, I18NADVERB,
  I18NLABEL=0x10, I18NBUTTON=0x20, I18NMENU=0x30, I18NLIST=0x40} I18nWordType;

typedef enum {I18NWORDSELECT, I18NTEXTWRAP} I18nScanType;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _I18N_h - DO NOT ADD ANY DEFINITIONS AFTER THIS #endif */
