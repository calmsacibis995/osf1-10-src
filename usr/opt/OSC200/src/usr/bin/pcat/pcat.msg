 
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
$ @(#)$RCSfile: pcat.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:45:27 $
$ @(#)pcat.msg	1.3  com/cmd/files,3.1,9013 10/24/89 16:19:28
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$ OBJECT CODE ONLY SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$set MS_PCAT
$		GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate pcat -- this is the command name.
$quote "
$ 
M_BADRD	".z: pcat: There is a read error.\n\
\tUse local problem reporting procedures.\n"
$ 
M_FORMT	".z: pcat: The specified file is not in packed format.\n"
$ 
M_FAILD	".z: pcat: An unpacking error occurred.\n\
\tUse local problem reporting procedures.\n"
$ 
M_BADWR	": pcat: There is a write error.\n\
\tUse local problem reporting procedures.\n"
$ 
M_2LONG	": pcat: The file name is too long.\n\
\tLimitations are 14 or 255 characters depending on system.\n"
$ 
M_NOPEN	".z: pcat: Cannot open the specified file.\n\
\tCheck path name and permissions.\n"
$ 
