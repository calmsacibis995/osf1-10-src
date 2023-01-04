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
/***********************************************************
Copyright 1987 by Tektronix, Beaverton, Oregon,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Tektronix or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

TEKTRONIX DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
/*
 *	NAME
 *		keynames_xtl.h -- Macros for each XTL key raw value.
 *
 *	DESCRIPTION
 *		Header file that assigns a macro (keyname) to each raw
 *		key value that the XTL keyboard can transmit.
 *
 *
 */
 
#ifndef LINT
#ifdef RCS_ID
static char *rcsid= "$Header: /usr/sde/x11/rcs/x11/src/./server/ddx/tek/keynames_xtl.h,v 1.2 91/12/15 12:42:16 devrcs Exp $";
#endif /* RCS_ID */
#endif /* LINT */

/*
 *	DEFINES
 */
#ifndef KEYNAMES_XTL_H
#define KEYNAMES_XTL_H

/*
 * PhysKeyToKeyCode(rawkey) converts a raw key value (as returned by the GDS
 * keyboard) into a keycode that fits into the range 8 to 255.
 */
#define PhysKeyToKeyCode(physkey)  ((physkey)+8)
#define KeyCodeToPhysKey(physkey)  ((physkey)-8)

#define MIN_GDS_KEYCODE        PhysKeyToKeyCode(KEY_Lock)
#define MAX_GDS_KEYCODE        PhysKeyToKeyCode(KEY_Greater)
#define GDS_GLYPHS_PER_KEY  2

/*
 * INDEX(keycode) returns the array index into the KeySym map of that
 * keycode.
 */
#define INDEX(keycode)	(((keycode) - MIN_GDS_KEYCODE) * GDS_GLYPHS_PER_KEY)

/*
 * definition of the GDS/4319 Keyboard:
 * ============================================================
 *       Defined             Key Cap Glyphs           Pressed value
 *      Key Name            Main       Also      (hex)  (octal)   (dec)
 *      ----------------   ---------- -------   ------  -----    -----
 */
#define KEY_Lock         /* Lock                  0x00   0000  */    0  
#define KEY_ShiftL       /* Shift(left)           0x01   0001  */    1 
#define KEY_ShiftR       /* Shift(right)          0x02   0002  */    2 
#define KEY_Ctrl         /* Ctrl                  0x03   0003  */    3 
#define KEY_SErase       /* S Eras                0x04   0004  */    4 
#define KEY_Break        /* Break                 0x05   0005  */    5 
#define KEY_BackSpace    /* Back Space            0x06   0006  */    6 
#define KEY_Tab          /* Tab                   0x07   0007  */    7 
#define KEY_Linefeed     /* Line Feed             0x08   0010  */    8 
#define KEY_Return       /* Return                0x09   0011  */    9 
#define KEY_Escape       /* Esc(Escape)           0x0A   0012  */   10 
#define KEY_Space        /*   (SpaceBar)          0x0B   0013  */   11 
#define KEY_Quote        /* ' (Apostr)  " (Quote) 0x0C   0014  */   12 
#define KEY_Comma        /* , (Comma)   , (Comma) 0x0D   0015  */   13 
#define KEY_Minus        /* - (Minus)   _ (Under) 0x0E   0016  */   14 
#define KEY_Period       /* . (Period)  .(Period) 0x0F   0017  */   15 
#define KEY_Slash        /* / (Slash)   ?         0x10   0020  */   16 
#define KEY_0            /* 0           )         0x11   0021  */   17 
#define KEY_1            /* 1           !         0x12   0022  */   18 
#define KEY_2            /* 2           @         0x13   0023  */   19 
#define KEY_3            /* 3           #         0x14   0024  */   20 
#define KEY_4            /* 4           $         0x15   0025  */   21 
#define KEY_5            /* 5           %         0x16   0026  */   22 
#define KEY_6            /* 6           ^         0x17   0027  */   23 
#define KEY_7            /* 7           &         0x18   0030  */   24 
#define KEY_8            /* 8           *         0x19   0031  */   25 
#define KEY_9            /* 9           (         0x1A   0032  */   26 
#define KEY_SemiColon    /* ;(SemiColon) :(Colon) 0x1B   0033  */   27 
#define KEY_Equal        /* = (Equal)   +         0x1C   0034  */   28 
#define KEY_a            /* A                     0x1D   0035  */   29 
#define KEY_b            /* B                     0x1E   0036  */   30 
#define KEY_c            /* C                     0x1F   0037  */   31 
#define KEY_d            /* D                     0x20   0040  */   32 
#define KEY_e            /* E                     0x21   0041  */   33 
#define KEY_f            /* F                     0x22   0042  */   34 
#define KEY_g            /* G                     0x23   0043  */   35 
#define KEY_h            /* H                     0x24   0044  */   36 
#define KEY_i            /* I                     0x25   0045  */   37 
#define KEY_j            /* J                     0x26   0046  */   38 
#define KEY_k            /* K                     0x27   0047  */   39 
#define KEY_l            /* L                     0x28   0050  */   40 
#define KEY_m            /* M                     0x29   0051  */   41 
#define KEY_n            /* N                     0x2A   0052  */   42 
#define KEY_o            /* O                     0x2B   0053  */   43 
#define KEY_p            /* P                     0x2C   0054  */   44 
#define KEY_q            /* Q                     0x2D   0055  */   45 
#define KEY_r            /* R                     0x2E   0056  */   46 
#define KEY_s            /* S                     0x2F   0057  */   47 
#define KEY_t            /* T                     0x30   0060  */   48 
#define KEY_u            /* U                     0x31   0061  */   49 
#define KEY_v            /* V                     0x32   0062  */   50 
#define KEY_w            /* W                     0x33   0063  */   51 
#define KEY_x            /* X                     0x34   0064  */   52 
#define KEY_y            /* Y                     0x35   0065  */   53 
#define KEY_z            /* Z                     0x36   0066  */   54 
#define KEY_LBrace       /* [           {         0x37   0067  */   55 
#define KEY_VertBar      /* \(BckSlash) |(VertBar)0x38   0070  */   56 
#define KEY_RBrace       /* ]           }         0x39   0071  */   57 
#define KEY_Tilde        /* ` (Accent)  ~ (Tilde) 0x3A   0072  */   58 
#define KEY_RubOut       /* <X (RubOut)           0x3B   0073  */   59 
#define KEY_Enter        /* Enter                 0x3C   0074  */   60 
#define KEY_KP_Comma     /* , (Comma)             0x3D   0075  */   61 
#define KEY_KP_Minus     /* - (Minus)             0x3E   0076  */   62 
#define KEY_KP_Period    /* . (Period)            0x3F   0077  */   63 
#define KEY_KP_0         /* 0                     0x40   0100  */   64 
#define KEY_KP_1         /* 1                     0x41   0101  */   65 
#define KEY_KP_2         /* 2                     0x42   0102  */   66 
#define KEY_KP_3         /* 3                     0x43   0103  */   67 
#define KEY_KP_4         /* 4                     0x44   0104  */   68 
#define KEY_KP_5         /* 5                     0x45   0105  */   69 
#define KEY_KP_6         /* 6                     0x46   0106  */   70 
#define KEY_KP_7         /* 7                     0x47   0107  */   71 
#define KEY_KP_8         /* 8                     0x48   0110  */   72 
#define KEY_KP_9         /* 9                     0x49   0111  */   73 
#define KEY_F1           /* F1                    0x4A   0112  */   74 
#define KEY_F2           /* F2                    0x4B   0113  */   75 
#define KEY_F3           /* F3                    0x4C   0114  */   76 
#define KEY_F4           /* F4                    0x4D   0115  */   77 
#define KEY_F5           /* F5                    0x4E   0116  */   78 
#define KEY_F6           /* F6                    0x4F   0117  */   79 
#define KEY_F7           /* F7                    0x50   0120  */   80 
#define KEY_F8           /* F8                    0x51   0121  */   81 
#define KEY_Dialog       /* Dialog                0x52   0122  */   82 
#define KEY_Setup        /* Setup                 0x53   0123  */   83 
#define KEY_Copy         /* S Copy      D Copy    0x54   0124  */   84 
#define KEY_Menu         /* Menu                  0x55   0125  */   85 
#define KEY_Cursor_R     /*  (Right)              0x56   0126  */   86 
#define KEY_Cursor_U     /*  (Up)                 0x57   0127  */   87 
#define KEY_Cursor_L     /*  (Left)               0x58   0130  */   88 
#define KEY_Cursor_D     /*  (Down)               0x59   0131  */   89 
#define KEY_Help         /* Help                  0x5A   0132  */   90 
#define KEY_Do           /* Do                    0x5B   0133  */   91 
#define KEY_Compose      /* Compose Character     0x5C   0134  */   92 
#define KEY_Tek          /* Tek                   0x5D   0135  */   93 
#define KEY_Find         /* Find                  0x5E   0136  */   94 
#define KEY_Insert       /* Insert Here           0x5F   0137  */   95 
#define KEY_Remove       /* Re-Move               0x60   0140  */   96 
#define KEY_Select       /* Select                0x61   0141  */   97 
#define KEY_Previous     /* Prev Screen           0x62   0142  */   98 
#define KEY_Next         /* Next Screen           0x63   0143  */   99 
#define KEY_PF1          /* PF1                   0x64   0144  */  100 
#define KEY_PF2          /* PF2                   0x65   0145  */  101 
#define KEY_PF3          /* PF3                   0x66   0146  */  102 
#define KEY_PF4          /* PF4                   0x67   0147  */  103 
#define KEY_Hold         /* Hold Screen           0x68   0150  */  104 
#define KEY_GErase       /* G Eras                0x69   0151  */  105 
/* there is no key for 106 */
#define KEY_DErase       /* D Eras                0x6B   0153  */  107 
#define KEY_Cancel       /* Cancel                0x6C   0154  */  108 
#define KEY_Greater      /* < (Greater) > (Less)  0x6D   0155  */  109 

/*
 * Number of keycodes pre-allocated for implementing Compose sequences.
 * Must be at least 96 for Latin-1; throw in a few more for good measure;
 * just happens to be the number needed for Katakana.
 */
#define NUM_COMPOSE_KEYCODES	110

/* Equals the total number of keys, so all can be redefined. */
#define KANA_OFFSET	110

/*
 *	EXTERNS
 */

#endif /* KEYNAMES_XTL_H */
