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
static char *rcsid = "@(#)$RCSfile: crc.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 15:02:37 $";
#endif
/*
 * derived from machdep.c       2.1     (ULTRIX/OSF)    12/3/90";
 */

/*
 * Some old history:
 *
 * 10-July-89   burns
 *      Added emulation for Vax crc instruction (from pmk).
 */

crc( ctp, inicrc, len, dp )
register char *ctp;
register unsigned int inicrc;
register unsigned int len;
register char *dp;
{
        register unsigned int index;

        while( len > 0 ) {

	    inicrc = (((char)inicrc ^ *dp++) & 0x0ff) | (inicrc & 0xffffff00);
            index = 0x0f & inicrc;
            inicrc = inicrc >> 4;
            inicrc ^= *((unsigned long *)ctp + index);
            index = 0x0f & inicrc;
            inicrc = inicrc >> 4;
            inicrc ^= *((unsigned long *)ctp + index);

            --len;
        }

        return(inicrc);

}
