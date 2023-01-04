 
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

$ @(#)$RCSfile: logger.msg,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:39:53 $

$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$ OBJECT CODE ONLY SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for logger  
$
$ logger.msg	1.6  com/cmd/oper,3.1,9021 2/28/90 16:17:30
$
$quote "         define quote character.

$len 
$set MS_LOGGER        Define initial set#
$ Do not translate logger, %d, %s, %1$d, %1$s, etc ...

USAGE		"logger: [-i] [-f file] [-p pri] [-t tag] [ message ... ]\n"

UNKFACILITY	"logger: The specified facility name is not known: "

UNKPRIORITY	"logger: The specified priority name is not known: "
