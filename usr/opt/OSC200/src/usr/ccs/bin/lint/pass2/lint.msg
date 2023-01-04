 
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
 
$
$ (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0

$
$ @(#)$RCSfile: lint.msg,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/11/09 15:52:18 $
$
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$len 
$set MS_LINT        Define initial set#

$  Message numbers have been assigned a range of 300-399.
$  Do not translate lint:, %s, %d, %1$s, %1$d, etc ...

1                    "lint: The %s flag option is missing.\n"
2                    "lint: %s flag is not recognized. It is ignored.\n"
3                    "lint: File %s must have a .c or .ln extension. It is ignored.\n"
4                    "lint: usage: lint [options] file ...\n"
$  Do not translate .c, .ln
M_MSG_258            "lint: There is not enough memory available now."
M_MSG_259            "The internal hash table overflowed."
M_MSG_260            "File %s data error"
M_MSG_261            "Action for symbol in file %s is not known"
$  Action is an operation that the lint program is trying to perform on symbol
$  within the file.
M_MSG_262            "Record directive in file %s is not known"
M_MSG_263            "\"%s\", line %d: warning: "
M_MSG_264            "\"%s\", line %d (\"%s\"): warning: "
M_MSG_265            "End of file reached before expected in %s."
M_MSG_266            "Cannot open file %s\n"
M_MSG_304            "function %s multiply defined"
$  Message 304-319 are written as standard output. They are not error messages.
$  Talk with your technical representatives before you translate these. Some
$  customers may have programs that depend on these messages comming out exactly
$  as they are.

M_MSG_305            "symbol %s multiply defined"
M_MSG_306            "symbol %s type inconsistent"
$  Type is a C-Language term which defines classes of objects.
M_MSG_307            "struct/union/enum %s inconsistently redefined"
$  Do not translate struct, union, enum
M_MSG_308            "function %s type inconsistent"
$  Type is a C-Language term which defines classes of objects.
M_MSG_309            "function %s argument type inconsistent"
$  Argument = parameter.
$  Type is a C-Language term which defines classes of objects.
M_MSG_310            "function argument %s count mismatch"
$  The definition of the function has a different number of parameters
$  than the call of the function.
M_MSG_311            "function %s used but not defined"
M_MSG_312            "symbol %s used but not defined"
M_MSG_313            "function %s defined but never used"
M_MSG_314            "symbol %s defined but never used"
M_MSG_315            "function %s declared but never used or defined"
M_MSG_316            "symbol %s declared but never used or defined"
M_MSG_317            "function %s return value used, but none returned"
$  The function call is requesting a return value, but the function itself does
$  not return a value.
M_MSG_318            "function %s return value is always ignored"
M_MSG_319            "function %s return value is sometimes ignored"
$  Messages 320 - 322 reserved for CTOOLS
M_MSG_323            "symbol %s set but never used."
