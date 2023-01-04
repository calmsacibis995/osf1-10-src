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
 *	@(#)$RCSfile: config.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/15 18:49:50 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from config.h	2.2  (ULTRIX)        4/12/89    
 */

/*
 * Modification history:
 *
 * 12-20-87 -- larry
 *	Add two fields to the end of config_adpt structure.
 *	The config program will init them to 0.
 *	The fields are the bus number and nexus number respectively.
 *	The fields are used by the SCS code to determine if a CI adapter
 *	is "alive".
 *
 * 7-jul-86   -- jaw 	added adapter alive bit for Mr. Installation.
 *
 * 5-Jun-86   -- jaw 	changes to config.
 *
 */

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

struct config_adpt {
	char 	*p_name;
	int	p_num;
	char	*c_name;
	int	c_num;
	char	c_type;		
	/* if c_type = 'D' then c_ptr is pointer to device struct */
	/* if c_type = 'C' then c_ptr is pointer to controller struct */
	/* if c_type = 'A' then c_ptr is set if ALIVE */
	char	*c_ptr;	   
	short	c_bus_num;
	short	c_nexus_num;
};

#define CONFIG_ALIVE 	1  /* c_ptr is set if adapter is alive */

#endif
