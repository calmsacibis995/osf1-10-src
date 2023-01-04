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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: initext.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 92/09/25 18:07:45 $";
#endif

/*******************************************************************
 * For loadable X server, this replaces a portion of miinitext's
 * hard coded list. Normally, each extension would have it's own
 * initialization routine, but this library includes several 
 * extensions. So, here's the hard coded list...
 *******************************************************************/
#ifdef SHAPE
extern void ShapeExtensionInit();
#endif
#ifdef MITSHM
extern void ShmExtensionInit();
#endif
#ifdef MULTIBUFFER
extern void MultibufferExtensionInit();
#endif
#ifdef MITMISC
extern void MITMiscExtensionInit();
#endif
#ifdef XTEST
extern void XTestExtensionInit();
#endif
#ifdef XTESTEXT1
extern void XTestExtension1Init();
#endif
#ifdef MODE_SWITCH
extern void KMEInit();
#endif

void
LibExtExtensionInit()
{
#ifdef SHAPE
	ShapeExtensionInit();
#endif
#ifdef MITSHM
	ShmExtensionInit();
#endif
#ifdef MULTIBUFFER
	MultibufferExtensionInit();
#endif
#ifdef MITMISC
	MITMiscExtensionInit();
#endif
#ifdef XTEST
	XTestExtensionInit();
#endif
#ifdef XTESTEXT1
	XTestExtension1Init();
#endif
#ifdef MODE_SWITCH
	KMEInit();
#endif
}
