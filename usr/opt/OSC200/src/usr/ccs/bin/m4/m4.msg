$
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY

$ @(#)$RCSfile: m4.msg,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/08/31 15:34:10 $

$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.2
$

$ 
$  COMPONENT_NAME: (CMDAS) Assembler and Macroprocessor 
$                                                                    
$  FUNCTIONS: 
$                                                                    
$ 
$  (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.
$set MS_M4        Define initial set#
$
QEOF " A quote is not complete.\n\
\tCheck that the end quote character exists.\n"
CEOF " A comment is not complete.\n\
\tCheck that the end comment character exists.\n"
AEOF " A parameter list is not complete.\n\
\tCheck that the ) character exists.\n"
OFL " Cannot open the specified file.\n\
\tCheck path name and permissions.\n"
OST " There is not enough memory available now.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
ST ",line %d: %s\n"
$ format for line number information displayed before error message
QLNB " The quote symbols cannot be more than %d bytes long.\n"
QLNC " The quote symbols cannot be more than %d characters long.\n"
BMN " A macro name cannot begin with a number and can\n\
\tcontain only alphabetic letters, numbers, and the _ character.\n"
MD " The macro name and definition cannot be identical.\n"
IE " A parameter to the eval macro is not correct.\n\
\tCheck the parameter for errors.\n"
$ Do not translate "eval" -- it is an m4 specific subcommand
IFLN " Do not use more than 9 levels of\n\
\tinclude and sinclude macros.\n"
$ Do not translate "include" or "sinclude" -- they are macro names
MTB " Do not specify more than %d bytes in a word.\n\
\tUse the -T flag to increase the size of the buffer.\n"
MTC " Do not specify more than %d characters in a word.\n\
\tUse the -T flag to increase the size of the buffer.\n"
MTI " A parameter list cannot contain more than %d items.\n\
\tUse the -S flag to increase the size of the buffer.\n"
PBMB " The push-back buffer cannot contain more than %d bytes.\n\
\tUse the -B flag to increase the size of the buffer.\n"
PBMC " The push-back buffer cannot contain more than %d characters.\n\
\tUse the -B flag to increase the size of the buffer.\n"
CMLB " A comment marker cannot be more than %d bytes long.\n"
CMLC " A comment marker cannot be more than %d characters long.\n"
AUGT " The parameters to the specified macro\n\
\tcannot contain more than %d bytes of text.\n"
MIA " %s is not a valid value for the -H flag.\n\
\tSpecify a numerical value greater than zero.\n"
BO " %s is not a recognized flag.\n\
\tUsage: m4 [-e] [-l] [-s] [-B Number] [-H Number] [-S Number] [-T Number]\n\
\t[[-D Name[=Value]] [-U Name] ...] [File ...]\n"
TR "Tracing (%1$d): %2$s"
$ informational message that tracing is turned on
MBWC " An invalid multi-byte character is in the string: %s\n"
WCMB " Not able to convert wide char string to multi-byte string.\n\
\tInvalid wide character string.\n"
WCDISP " Not able to calculate display width.\n\
\tInvalid wide character string.\n"
