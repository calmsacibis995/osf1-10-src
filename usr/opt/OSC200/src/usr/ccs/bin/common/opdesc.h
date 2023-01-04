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
 *	@(#)$RCSfile: opdesc.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/12/03 15:38:28 $
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
 * COMPONENT_NAME: (CMDPROG) opdesc.h
 *
 * FUNCTIONS: mkdope                                                         
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */

int dope[ DSIZE ];
char *opst[DSIZE];

struct dopest { int dopeop; char opst[12]; int dopeval; } indope[] = {

	NAME, "NAME", LTYPE,
	LNAME, "LNAME", LTYPE,
	PNAME, "PNAME", LTYPE,
	STATNAME, "STATNAM", LTYPE,
	TNAME, "TNAME", LTYPE,
	LTEMP, "LTEMP", LTYPE,
	STRING, "STRING", LTYPE,
	REG, "REG", LTYPE,
	TREG, "TREG", LTYPE,
	OREG, "OREG", LTYPE,
	ICON, "ICON", LTYPE,
	LABCON, "LABCON", LTYPE,
	STLABEL, "STLAB", LTYPE,
	ADDR, "ADDR", LTYPE,
	LADDR, "LADDR", LTYPE,
	PADDR, "PADDR", LTYPE,
	STADDR, "STADDR", LTYPE,
	FCON, "FCON", LTYPE,
	CCODES, "CCODES", LTYPE,
	CCODET, "CCODET", LTYPE,
	LEAFNOP, "LFNOP", LTYPE,
	UNARY MINUS, "U-", UTYPE,
	UNARY MUL, "U*", UTYPE,
	UNARY AND, "U&", UTYPE,
	UNARY CALL, "UCALL", UTYPE|CALLFLG,
	UNARY FORTCALL, "UFCALL", UTYPE|CALLFLG,
	NOT, "!", UTYPE|LOGFLG,
	COMPL, "~", UTYPE,
	FORCE, "FORCE", UTYPE,
	INIT, "INIT", UTYPE,
	SCONV, "SCONV", UTYPE,
	PCONV, "PCONV", UTYPE,
	OCONVLEAF, "OCONVL", UTYPE,
	UNARYNOP, "UNYNOP", UTYPE,
	OCONVTREE, "OCONVT", UTYPE,
	PLUS, "+", BITYPE|FLOFLG|SIMPFLG|COMMFLG,
	ASG_PLUS, "+=", BITYPE|ASGFLG|ASGOPFLG|FLOFLG|SIMPFLG|COMMFLG,
	MINUS, "-", BITYPE|FLOFLG|SIMPFLG,
	ASG_MINUS, "-=", BITYPE|FLOFLG|SIMPFLG|ASGFLG|ASGOPFLG,
	MUL, "*", BITYPE|FLOFLG|MULFLG,
	ASG_MUL, "*=", BITYPE|FLOFLG|MULFLG|ASGFLG|ASGOPFLG,
	AND, "&", BITYPE|BITFLG|SIMPFLG|COMMFLG,
	ASG_AND, "&=", BITYPE|BITFLG|COMMFLG|ASGFLG|ASGOPFLG,
	QUEST, "?", BITYPE,
	COLON, ":", BITYPE,
	ANDAND, "&&", BITYPE|LOGFLG,
	OROR, "||", BITYPE|LOGFLG,
	CM, ",", BITYPE,
	DOLR, "DOLR", BITYPE,
	COMOP, ",OP", BITYPE,
	ASSIGN, "=", BITYPE|ASGFLG,
	DIV, "/", BITYPE|FLOFLG|MULFLG|DIVFLG,
	ASG_DIV, "/=", BITYPE|FLOFLG|MULFLG|DIVFLG|ASGFLG|ASGOPFLG,
	MOD, "%", BITYPE|DIVFLG,
	ASG_MOD, "%=", BITYPE|DIVFLG|ASGFLG|ASGOPFLG,
	LS, "<<", BITYPE|SHFFLG,
	ASG_LS, "<<=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG,
	RS, ">>", BITYPE|SHFFLG,
	ASG_RS, ">>=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG,
	OR, "|", BITYPE|COMMFLG|BITFLG,
	ASG_OR, "|=", BITYPE|COMMFLG|BITFLG|ASGFLG|ASGOPFLG,
	ER, "^", BITYPE|COMMFLG|BITFLG,
	ASG_ER, "^=", BITYPE|COMMFLG|BITFLG|ASGFLG|ASGOPFLG,
	INCR, "++", BITYPE|ASGFLG,
	DECR, "--", BITYPE|ASGFLG,
	STREF, "->", BITYPE,
	CALL, "CALL", BITYPE|CALLFLG,
	FORTCALL, "FCALL", BITYPE|CALLFLG,
	EQ, "==", BITYPE|LOGFLG,
	NE, "!=", BITYPE|LOGFLG,
	LE, "<=", BITYPE|LOGFLG,
	LT, "<", BITYPE|LOGFLG,
	GE, ">=", BITYPE|LOGFLG,
	GT, ">", BITYPE|LOGFLG,
	UGT, "UGT", BITYPE|LOGFLG,
	UGE, "UGE", BITYPE|LOGFLG,
	ULT, "ULT", BITYPE|LOGFLG,
	ULE, "ULE", BITYPE|LOGFLG,
	ARS, "A>>", BITYPE,
	TYPE, "TYPE", LTYPE,
	LB, "[", BITYPE,
	CBRANCH, "conditional", BITYPE,
	FLD, "FLD", UTYPE,
	PMCONV, "PMCONV", BITYPE,
	PVCONV, "PVCONV", BITYPE,
	RETURN, "RETURN", BITYPE|ASGFLG|ASGOPFLG,
	CAST, "CAST", BITYPE|ASGFLG|ASGOPFLG,
	GOTO, "GOTO", UTYPE,
	STASG, "STASG", BITYPE|ASGFLG,
	STARG, "STARG", UTYPE,
	STCALL, "STCALL", BITYPE|CALLFLG,
	UNARY STCALL, "USTCALL", UTYPE|CALLFLG,
	PARAMETER, "PARAMETER", BITYPE|ASGFLG,

-1,	0
};

/* -------------------- mkdope -------------------- */

mkdope(){
	register struct dopest *q;

	for( q = indope; q->dopeop >= 0; ++q ){
		dope[q->dopeop] = q->dopeval;
		opst[q->dopeop] = q->opst;
		}
	}
