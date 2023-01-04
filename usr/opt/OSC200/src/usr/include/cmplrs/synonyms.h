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
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991, 1990 MIPS Computer Systems, Inc.      |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */

#ifndef __SYNONYMS_H__
#define __SYNONYMS_H__

#define get_sp __get_sp

/* libm pragma weak defines for *.c files */
#define acosh  __acosh
#define asinh  __asinh
#define atanh __atanh
#define cbrt  __cbrt
#define exp__E  __exp__E
#define erf __erf
#define erfc __erfc
#define fexpm1  __fexpm1
#define expm1f  __expm1f
#define flog1p  __flog1p
#define log1pf  __log1pf
#define fmodf __fmodf
#define fpow  __fpow
#define powf  __powf
#define j0  __j0
#define j1  __j1
#define jn  __jn
#define gamma  __gamma
#define lgamma  __lgamma
#define asym  __asym
#define neg  __neg
#define pos  __pos
#define log__L __log__L
#define set_arg_val  __set_arg_val
#define gamma_err_val  __gamma_err_val
#define hypot_err_val  __hypot_err_val
#define set_res_val  __set_res_val
#define set_err_val  __set_err_val
#define set_exp_val  __set_exp_val
#define y0  __y0
#define y1  __y1
#define yn  __yn
#define matherr  __matherr

#endif __SYNONYMS_H__
