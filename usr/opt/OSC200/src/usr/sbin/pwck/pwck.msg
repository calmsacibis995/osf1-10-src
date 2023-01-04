 
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
$ (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$
$ OSF/1 Release 1.0.2

$
$ RESTRICTED RIGHTS LEGEND
$ Use, Duplication or Disclosure by the Government is subject to
$ restrictions as set forth in paragraph (b)(3)(B) of the rights in
$ Technical Data and Computer Software clause in DAR 7-104.9(a).
$ 
$ @(#)$RCSfile: pwck.msg,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/30 09:13:20 $
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_PWCK

CANTOPEN "Cannot open %s.\n\
\tCheck path name and permissions.\n"
ERR1 "There are either too few or too many fields.\n\
\tThere must be 7 fields.\n"
ERR2 "One or more characters in login name is not correct.\n"
ERR2a "The first character in the login name is not\n\
\ta lowercase alphabetic character.\n"
ERR2b "The login name field is null.\n"
ERR3 "The login name is too long.\n\
\tIt must be greater than 0 characters and less than 8.\n"
ERR4 "The user number is not correct.\n"
ERR5 "The group number is not correct.\n"
ERR6 "Cannot find the login directory field.\n\
\tCheck path name and permissions.\n"
ERR6a "The login directory field is null.\n\
\tCorrect the entry in the /etc/passwd file.\n"
ERR7 "Cannot find the optional login shell.\n\
\tCorrect the entry in the /etc/passwd file.\n"
ERR8 "The shortened login name is not unique.\n\
\tUse a unique login name.\n"
OUTOFMEM "The system is out of memory.\n\
\tTry again later or\n\
\tuse local problem reporting procedures.\n"
