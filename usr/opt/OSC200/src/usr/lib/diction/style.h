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
 *	@(#)$RCSfile: style.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:28:02 $
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
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 

/*
 * COMPONENT_NAME: (CMDTEXT) Text Formatting Services
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26
 *
 * OBJECT CODE ONLY SOURCE MATERIALS
 *
 */
 
extern int part;
extern int style;
extern int topic;
extern int pastyle;
extern int lstyle;
extern int rstyle;
extern int estyle;
extern int nstyle;
extern int Nstyle;
extern int rthresh;
extern int lthresh;
extern int pstyle;
extern int count;
extern int sleng[50];
extern int numsent;
extern long letnonf;
extern long numnonf;
extern long vowel;
extern long numwds;
extern long twds;
extern long numlet;
extern int maxsent;
extern int maxindex;
extern int minindex;
extern int qcount;
extern int icount;
extern int minsent;
extern int simple;
extern int compound;
extern int compdx;
extern int complex;
extern int nomin;
extern int tobe;
extern int noun, infin, pron, aux, adv;
extern int passive;
extern int beg[15];
extern int prepc;
extern int conjc;
extern int verbc;
extern int tverbc;
extern int adj;
#define MAXPAR 20
extern int leng[];
extern sentno;
