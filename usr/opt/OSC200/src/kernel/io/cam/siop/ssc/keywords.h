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
/********************************************************************/
/*                    Compiler Revisions									  */
/*																						  */
/*  init   Date        Revision/Changes                             */
/*	 ----   ----        ----------------									  */
/*  bsb   9/7/90       1.0	/ Initial general customer release       */
/********************************************************************/
/*                    keywords.h Revisions								  */
/*																						  */
/*  init   Date        Revision/Changes									  */
/*  ----   ----        -----------------									  */
/*  bsb   9/7/90       1.0 / Initial general customer release       */
/********************************************************************/

#ifndef _SSC_KEYWORDS_H_
#define _SSC_KEYWORDS_H_


/*
    keywords.h - Reserved keywords matched by scsi_lex
                 This file is #included in scsi_lex.c
*/

static keyword_t keywords[] = {
    { "DATA_OUT",   PHASE,      0    },
    { "DATA_IN",    PHASE,      1    },
    { "COMMAND",    PHASE,      2    },
    { "CMD",        PHASE,      2    },
    { "STATUS",     PHASE,      3    },
    { "RES4",       PHASE,      4    },
    { "RES5",       PHASE,      5    },
    { "MSG_OUT",    PHASE,      6    },
    { "MSG_IN",     PHASE,      7    },
    { "ACK",        ACK,        0    },
    { "ATN",        ATN,        0    },
    { "TARGET",     TARGET,     0    },
    { "MOVE",       MOVE,       0    },
    { "PTR",        PTR,        0    },
    { "WITH",       WITH,       0    },
    { "WHEN",       WHEN,       0    },
    { "JUMP",       JUMP,       0    },
    { "CALL",       CALL,       0    },
    { "RETURN",     RETURN,     0    },
    { "INT",        INT,        0    },
    { "IF",         IF,         0    },
    { "AND",        AND,        0    },
    { "NOT",        NOT,        0    },
    { "OR",         OR,         0    },
    { "MASK",       MASK,       0    },
    { "SELECT",     SELECT,     0    },
    { "RESELECT",   RESELECT,   0    },
    { "DISCONNECT", DISCONNECT, 0    },
    { "WAIT",       WAIT,       0    },
    { "SET",        SET,        0    },
    { "CLEAR",      CLEAR,      0    },
    { "TO",         TO,         0    },
    { "FROM",       FROM,       0    },
    { "ABSOLUTE",   ABSOLUTE,   0    },
    { "EXTERN",     EXTERN,     0    },
    { "EXTERNAL",   EXTERNAL,   0    },
    { "PASS",       PASS,       0    },
    { "PROC",        PROC,       0    },
    { "RELATIVE",   RELATIVE,   0    },
    { "PC",         PC,         0    },
    { "ORIGIN",     ORIGIN,     0    },
    { "GOOD_PARITY",GOOD_PARITY,0    },
    { "BAD_PARITY", BAD_PARITY, 0    },
    { "ALIGN",      ALIGN,      0    },
    { "DUP",        DUP,        0    },
    { "DB",         DATA,       1    },
    { "DW",         DATA,       2    },
    { "DD",         DATA,       4    },
    { "MEMORY",     MEMORY,     0    },
    { "REL",        REL,        0    },
    { "REG",        REG,        0    },
    { "ENTRY",      ENTRY,      0    },
    { "SCNTL0",     REGISTER,   0x00 },
    { "SCNTL1",     REGISTER,   0x01 },
    { "SDID",       REGISTER,   0x02 },
    { "SIEN",       REGISTER,   0x03 },
    { "SCID",       REGISTER,   0x04 },
    { "SXFER",      REGISTER,   0x05 },
    { "SODL",       REGISTER,   0x06 },
    { "SOCL",       REGISTER,   0x07 },
    { "SFBR",       REGISTER,   0x08 },
    { "SIDL",       REGISTER,   0x09 },
    { "SBDL",       REGISTER,   0x0a },
    { "SBCL",       REGISTER,   0x0b },
    { "DSTAT",      REGISTER,   0x0c },
    { "SSTAT0",     REGISTER,   0x0d },
    { "SSTAT1",     REGISTER,   0x0e },
    { "SSTAT2",     REGISTER,   0x0f },
    { "DSA0",       REGISTER,   0x10 },
    { "DSA1",       REGISTER,   0x11 },
    { "DSA2",       REGISTER,   0x12 },
    { "DSA3",       REGISTER,   0x13 },
    { "CTEST0",     REGISTER,   0x14 },
    { "CTEST1",     REGISTER,   0x15 },
    { "CTEST2",     REGISTER,   0x16 },
    { "CTEST3",     REGISTER,   0x17 },
    { "CTEST4",     REGISTER,   0x18 },
    { "CTEST5",     REGISTER,   0x19 },
    { "CTEST6",     REGISTER,   0x1a },
    { "CTEST7",     REGISTER,   0x1b },
    { "TEMP0",      REGISTER,   0x1c },
    { "TEMP1",      REGISTER,   0x1d },
    { "TEMP2",      REGISTER,   0x1e },
    { "TEMP3",      REGISTER,   0x1f },
    { "DFIFO",      REGISTER,   0x20 },
    { "ISTAT",      REGISTER,   0x21 },
    { "CTEST8",     REGISTER,   0x22 },
    { "LCRC",       REGISTER,   0x23 },
    { "DBC0",       REGISTER,   0x24 },
    { "DBC1",       REGISTER,   0x25 },
    { "DBC2",       REGISTER,   0x26 },
    { "DCMD",       REGISTER,   0x27 },
    { "DNAD0",      REGISTER,   0x28 },
    { "DNAD1",      REGISTER,   0x29 },
    { "DNAD2",      REGISTER,   0x2a },
    { "DNAD3",      REGISTER,   0x2b },
    { "DSP0",       REGISTER,   0x2c },
    { "DSP1",       REGISTER,   0x2d },
    { "DSP2",       REGISTER,   0x2e },
    { "DSP3",       REGISTER,   0x2f },
    { "DSPS0",      REGISTER,   0x30 },
    { "DSPS1",      REGISTER,   0x31 },
    { "DSPS2",      REGISTER,   0x32 },
    { "DSPS3",      REGISTER,   0x33 },
    { "SCRATCH0",   REGISTER,   0x34 },
    { "SCRATCH1",   REGISTER,   0x35 },
    { "SCRATCH2",   REGISTER,   0x36 },
    { "SCRATCH3",   REGISTER,   0x37 },
    { "DMODE",      REGISTER,   0x38 },
    { "DIEN",       REGISTER,   0x39 },
    { "DWT",        REGISTER,   0x3a },
    { "DCNTL",      REGISTER,   0x3b },
    { "ADDER0",     REGISTER,   0x3c },
    { "ADDER1",     REGISTER,   0x3d },
    { "ADDER2",     REGISTER,   0x3e },
    { "ADDER3",     REGISTER,   0x3f },
    { "NOP",        NOP,        0    }
};
static int keyword_entries = ( sizeof(keywords) / sizeof(keyword_t) );


#endif 
