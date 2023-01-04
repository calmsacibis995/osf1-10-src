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
 *  I18N_CONVERTER.H
 */

#ifndef I18n_CONVERTER_H
#define I18n_CONVERTER_H
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

/*
 * compound string converter status returns
 */

#define I18nCvtStatusOK        1
#define I18nCvtStatusDataLoss  2
#define I18nCvtStatusFail      3

/*
 * Compound string parser and constructor callback constants and context
 */
#define I18nCvtInitCallback	0
#define I18nCvtEndCallback	1
#define I18nCvtSegmentCallback	2
#define I18nCvtLineCallback	3

#define I18nCvtMaxCallback	4

typedef struct
{
    int			status;
    void		(*callback[I18nCvtMaxCallback])();
    Opaque *		stream;
    Opaque *		stream_context;

    int			memory_length;
    Opaque *		memory;    
    Opaque *		memory_context;
    
    unsigned char	direction;
    unsigned char *	charset;
    int			byte_length;
    unsigned char *	text;
    int			nest_level;
} I18nCvtContextRec, *I18nCvtContext;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* I18n_CONVERTER_H */
