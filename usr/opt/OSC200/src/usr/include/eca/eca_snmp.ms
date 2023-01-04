(* 
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
*
*
*HISTORY
*
*@(#)$RCSfile: eca_snmp.ms,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/15 16:23:55 $
*)
MANAGEMENT SPECIFICATION SNMP;
VERSION = T1.0.1;
SYMBOL-PREFIX = CA_;
DEFINING-SMI = SNMP;

GLOBAL ENTITY SNMP = 1 :
    IDENTIFIER = (),
    SNMP_OID = {1 3 6 1 2 1},
    DNA_CMIP_INT = 1,
    IDENTIFIER ATTRIBUTES
    END ATTRIBUTES;
END ENTITY SNMP;

END SPECIFICATION;
