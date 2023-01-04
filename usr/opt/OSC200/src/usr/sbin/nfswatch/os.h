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
 * @(#)$RCSfile: os.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:22:14 $
 */
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/os.h,v 4.0 1993/03/01 19:59:00 davy Exp $
 *
 * os.h	- operating system definitions.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: os.h,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 1.6  1993/01/16  19:12:54  davy
 * Moved cpp controls to left margin.
 *
 * Revision 1.5  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 1.4  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 1.3  1993/01/13  21:41:37  davy
 * Got rid of old IRIX versions.
 *
 * Revision 1.2  1993/01/13  21:24:54  davy
 * Added IRIX40.
 *
 * Revision 1.1  1993/01/13  20:18:17  davy
 * Initial revision
 *
 */
#ifdef IRIX40
#ifndef USE_SNOOP
#define USE_SNOOP	1
#endif
#define signal		sigset
#endif

#ifdef SUNOS4
#ifndef USE_NIT
#define USE_NIT	1
#endif
#endif

#ifdef SUNOS5
#ifndef SVR4
#define SVR4		1
#endif
#ifndef USE_DLPI
#define USE_DLPI	1
#endif
#endif

#ifdef SVR4
#ifndef USE_DLPI
#define USE_DLPI	1
#endif
#define index		strchr
#define rindex		strrchr
#define signal		sigset
#define bzero(b,n)	memset(b,0,n)
#define bcmp(a,b,n)	memcmp(a,b,n)
#define bcopy(a,b,n)	memcpy(b,a,n)
#endif

#ifdef ULTRIX
#ifndef USE_PFILT
#define USE_PFILT	1
#endif
#endif
