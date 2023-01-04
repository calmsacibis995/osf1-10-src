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
static char *rcsid = "@(#)$RCSfile: sck_msg_text.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/13 16:01:20 $";
#endif
/* functions that print translatable text. */

char *sck_msg001() { return "S001 - Error during socket: %s";}
char *sck_msg002() { return "S002 - Error during bind: %s";}
char *sck_msg003() { return "S003 - Error during accept: %s";}
char *sck_msg004() { return "S004 - Error during read: %s";}
char *sck_msg005() { return "S005 - Error during connect: %s";}
char *sck_msg006() { return "S006 - Error during write(): %s";}
char *sck_msg007() { return "S007 - Error during close(): %s";}
char *sck_msg008() { return "S008 - Invalid function moi server.";}
char *sck_msg009() { return "S009 - Invalid function mold server.";}
char *sck_msg010() { return "S010 - Invalid function pe server.";}
char *sck_msg011() { return "S011 - mold_*() call failed (%s)";}
char *sck_msg012() { return "S012 - moi_*() call failed (%s)";}
char *sck_msg013() { return "S013 - pei/evd_*() call failed (%s)";}
char *sck_msg014() { return "S014 - Invalid function evd server.";}
char *sck_msg015() { return "S015 - evd_*() call failed (%s)";}
char *sck_msg016() { return "S016 - unable to init mutex: (%s)";}
char *sck_msg017() { return "S017 - unable to destroy mutex: (%s)";}
char *sck_msg018() { return "S018 - unable to lock mutex: (%s)";}
char *sck_msg019() { return "S019 - unable to unlock mutex: (%s)";}
