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
**                                                                          *
**  COPYRIGHT (c) 1991 BY                                                   *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      I18N Object Library
**
**  ABSTRACT:
**
**	- Common Library for DECwindows/Motif Applications
**	  These routines enable an application programmer to facilitate I18n
**	  on both VMS and ULTRIX platform.
**
**--
**/

#ifdef _NO_PROTO

extern int	DWI18n_ByteLengthOfChar ( );
extern int	DWI18n_CharCount ( );
extern void	DWI18n_CharIncrement ( );
extern int	DWI18n_ClipboardCopy ( );
extern int	DWI18n_ClipboardPaste ( );
extern int	DWI18n_CreatePath ( );
extern char 	*DWI18n_GetXLFDCharSet ( );
extern Boolean	DWI18n_IsXnlLangISOLatin1 ( );
extern void	DWI18n_RebindIrregularKeysym ( );
extern void	DWI18n_SetIconName ( );
extern void	DWI18n_SetTitle ( );
extern char	*DWI18n_ToLower ( );
extern char	*DWI18n_ToUpper ( );

#else

extern int	DWI18n_ByteLengthOfChar ( char *chr );
extern int	DWI18n_CharCount ( char *str );
extern void	DWI18n_CharIncrement ( char **ptr );
extern int	DWI18n_ClipboardCopy ( Display *dpy, Window win, long item_id, int private_id, XmString cs_data );
extern int	DWI18n_ClipboardPaste ( Display *dpy, Window win, XmString *cs_data, int *private_id );
extern int	DWI18n_CreatePath ( char *path, char *language_str );
extern char 	*DWI18n_GetXLFDCharSet ( char *fontname );
extern Boolean	DWI18n_IsXnlLangISOLatin1 ( void );
extern void	DWI18n_RebindIrregularKeysym ( Display *dpy );
extern void	DWI18n_SetIconName ( Widget widget, XmString cs_icon );
extern void	DWI18n_SetTitle ( Widget widget, XmString cs_title );
extern char	*DWI18n_ToLower ( char *str );
extern char	*DWI18n_ToUpper ( char *str );

#endif

