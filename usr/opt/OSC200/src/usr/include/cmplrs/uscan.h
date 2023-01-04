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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 52.227-7013.   |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Drive                                |
 * |         Sunnyvale, CA 94086                               |
 * |-----------------------------------------------------------|
 */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/cmplrs/uscan.h,v 4.2.4.2 1992/04/30 16:00:53 Ken_Lesniak Exp $ */

procedure Abort;
  external;

procedure openstdout(
	var F	     : text);
  external;

procedure opnstdin(
	var F	     : text);
  external;

procedure openinput(
	var F	     : text;
	    Fname    : Filename);
  external;

procedure openoutput(
	var F	     : text;
	    Fname    : Filename);
  external;

function getclock
   : integer;
  external;

function eopage(
	var Fil	    : text)
   : boolean;
  external;

procedure readpage(
	var Fil	     : text);
  external;

procedure printdate(
	var Fil	     : text);
  external;

procedure printtime(
	var Fil	     : text);
  external;

#if 0
function max(
	   I, 
	   J	    : integer)
   : integer;
  external;

function min(
	   I, 
	   J	    : integer)
   : integer;
  external;
#endif
