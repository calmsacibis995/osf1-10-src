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
 * @(#)$RCSfile: cdoc.h,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/03/03 15:27:21 $
 */
/************************************************************/
/*                                                          */
/*   Definitions file for capsar utilities and libraries    */
/*                                                          */
/************************************************************/

struct cdoc {
	char *message;
	int  m_type;
	char *b_type;
}comp_doc;

static struct cdoc message_tag[] = {
	"multiple format",MULTIPLE_FORMAT,NULL,
	"compound message",COMPOUND_MESSAGE,NULL,
	"compound document",COMPOUND_MESSAGE,NULL,
	"forwarded message",MAIL_MESSAGE,NULL,
	"ddif.compress.uuencode",SIMPLE_MESSAGE,"ddif",
	"text message",SIMPLE_MESSAGE,"ascii",
	NULL,NULL,NULL,NULL 
};

