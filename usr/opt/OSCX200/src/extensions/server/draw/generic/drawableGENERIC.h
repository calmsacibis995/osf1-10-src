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
/****************************************************************************/
/** COPYRIGHT (c) 1988                                                      */
/** by DIGITAL Equipment Corporation, Maynard, Massachusetts.               */
/** ALL RIGHTS RESERVED.                                                    */
/**                                                                         */
/** This software is furnished under a license and may be used and copied   */
/** only  in  accordance with the  terms  of  such  license  and with the   */
/** inclusion of the above copyright notice. This software or  any  other   */
/** copies thereof may not be provided or otherwise made available to any   */
/** other person.  No title to and  ownership of the  software is  hereby   */
/** transferred.                                                            */
/**                                                                         */
/** The information in this software is  subject to change without notice   */
/** and  should  not  be  construed  as a commitment by DIGITAL Equipment   */
/** Corporation.                                                            */
/**                                                                         */
/** DIGITAL assumes no responsibility for the use  or  reliability of its   */
/** software on equipment which is not supplied by DIGITAL.                 */
/****************************************************************************/

/*
 * FILE:
 *     drawableGENERIC.h
 *
 *  Author:
 *     LVO 2/2/93
 *
 *  Revisions: 
 */

D_BufferAttr *D_GENERICCreateBuffer();
void D_GENERICFreeBuffer();
int D_GENERICReadyBuffers();
int D_GENERICDisplayBuffer();
