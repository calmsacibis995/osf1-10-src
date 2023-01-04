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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: cmds_machine.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:30 $";
#endif 
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
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
	case SCR:		       /* scr = system control register */
		if (argc >= 3) {
			int new = ATOX(argv[2]);
			sys_reg(argv[1], new);
		} else if (argc >= 2) {
			int MFSR();
			print_reg(argv[1], sys_internal, MFSR);
		} else
			sys_regs();
		break;
	case TRACEBACK:
		if (argc > 3)
			traceback(dot, count, arg2);
		else {
			traceback(REG(t_eip), REG(t_ebp),MAX_LEVEL);
		}
		/* trace it back */
		break;
	case SELECTOR:
		for (i=0; i<count && err_flag == 0; ++i, dot += 8)
			if (argc > 3)
				pridtentry(dot);
			else
				prgdtentry(dot);
		break;
