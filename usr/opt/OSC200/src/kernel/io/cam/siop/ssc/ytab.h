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
typedef union {
    INT32       i;
    symbol_pt   s;
    expr_t      e;
} YYSTYPE;
#define MOVE	257
#define JUMP	258
#define NOP	259
#define CALL	260
#define RETURN	261
#define INT	262
#define SELECT	263
#define RESELECT	264
#define DISCONNECT	265
#define SET	266
#define CLEAR	267
#define PTR	268
#define WITH	269
#define WHEN	270
#define IF	271
#define AND	272
#define NOT	273
#define OR	274
#define MASK	275
#define WAIT	276
#define SHIFT_LEFT	277
#define SHIFT_RIGHT	278
#define EOL	279
#define ABSOLUTE	280
#define RELATIVE	281
#define PC	282
#define ORIGIN	283
#define MEMORY	284
#define REG	285
#define TO	286
#define REL	287
#define FROM	288
#define DUP	289
#define ALIGN	290
#define GOOD_PARITY	291
#define BAD_PARITY	292
#define EXTERN	293
#define EXTERNAL	294
#define ENTRY	295
#define PHASE	296
#define ACK	297
#define ATN	298
#define TARGET	299
#define INTEGER	300
#define DATA	301
#define REGISTER	302
#define IDENTIFIER	303
#define PASS	304
#define PROC	305
#define ARRAYNAME	306
#define PASSIFIER	307
#define IDENT_ABS	308
#define IDENT_EXT	309
#define IDENT_REL	310
#define LABEL_ABS	311
#define LABEL_REL	312
#define UNARY	313
extern YYSTYPE yyval, yylval;
