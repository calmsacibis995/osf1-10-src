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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/include/cmplrs/uwri.h,v 4.2.4.2 1992/04/30 16:01:12 Ken_Lesniak Exp $ */

procedure inituwrite(
	var ObjectName : Filename);
  external;

function idlen(
	var Id	    : Identname)
   : integer;
  external;

procedure uwrite (var U: Bcrec);
  external;

function getdtyname(
	   Dtyp	    : Datatype)
   : char;
  external;

function getmtyname(
	   Mtyp	    : Memtype)
   : char;
  external;

procedure ucoid(
	    Tag	     : Identname);
  external;

procedure ucofname(
	    Fnam     : Filename);
  external;

procedure stopucode;
  external;

procedure uputinit(
	var ObjectName : Filename);
  external;

procedure uputint(
	i: integer64);
  external;

procedure uputkill;
    external;

procedure uputclose;
    external;

PROCEDURE Ubittobyte (VAR U: Bcrec);
  external;

procedure Set_u_indent(lev: integer);
  external;
