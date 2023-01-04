/*  DEC/CMS REPLACEMENT HISTORY, Element IPSSTATUSCODES.MSG                 */
/*  *5    15-OCT-1990 15:49:42 DINTINO "change to new facility code"        */
/*  *4    19-SEP-1990 15:55:45 PICCOLO "new DCT error codes"                */
/*  *3    29-AUG-1990 10:16:10 PICCOLO "add invalid cpp error"              */
/*  *2    28-JUN-1990 14:43:35 PICCOLO "added invalid math arg error"       */
/*  *1     1-JUN-1990 16:13:00 PICCOLO "IPS Base Level"                     */
/*  DEC/CMS REPLACEMENT HISTORY, Element IPSSTATUSCODES.MSG                 */
/*                                                                          */
#define IpsX_FACILITY 3332
/*                                                                          */
/*+                                                                         */
/* NOTES:                                                                   */
/*                                                                          */
/*	* The .TITLE keyword must be separated by two spaces instead             */
/*	  of a tab so that the .TPU procedure that modifies this file            */
/*	  to produce the IPS_XSTATUS_CODES.OBJ file will be able to              */
/*	  find it.                                                               */
/*                                                                          */
/*	* The prefix string "Ips$_" must be in mixed case so that the            */
/*	  CHF message compiler will produce "IpsX_" prefixes also with           */
/*	  mixed case.                                                            */
/*-                                                                         */
/*                                                                          */
/*+                                                                         */
/* General success codes                                                    */
/*-                                                                         */
#define IpsX_NORMAL 218398721
#define IpsX_SUCCESS 218398729
/*                                                                          */
/*+                                                                         */
/* Informational codes.                                                     */
/*-                                                                         */
/*                                                                          */
/*+                                                                         */
/* Warning codes. (LIB$SIGNAL only)                                         */
/*-                                                                         */
/*                                                                          */
/*                                                                          */
/* Fatal codes (LIB$STOP only)                                              */
/*                                                                          */
/*                                                                          */
#define IpsX_FAILURE 218401924
#define IpsX_INVDTYPE 218401932
#define IpsX_INVDARG 218401940
#define IpsX_INVDCPP 218401948
#define IpsX_INCNSARG 218401956
#define IpsX_INSVIRMEM 218401964
#define IpsX_NOINPLACE 218401972
#define IpsX_NOMATCH 218401980
#define IpsX_UNSOPTION 218401988
#define IpsX_UNSPXLSTR 218401996
#define IpsX_BUFOVRFLW 218402004
#define IpsX_INVDTHBIT 218402012
#define IpsX_INVDMTHARG 218402020
#define IpsX_INVARGCON 218402028
#define IpsX_INVCODTYP 218402036
#define IpsX_INVSCNLEN 218402044
#define IpsX_DLENGTR32 218402052
#define IpsX_DSTLENZER 218402060
#define IpsX_SLCGTRDLC 218402068
#define IpsX_SLENGTR32 218402076
#define IpsX_SPCGTRDPC 218402084
#define IpsX_SPLGTRDPL 218402092
#define IpsX_SPLGTRDPS 218402100
#define IpsX_SRCLENZER 218402108
#define IpsX_DCTDECODEFAIL 218402116
#define IpsX_DCTENCODEFAIL 218402124
#define IpsX_DCTFACTERR 218402132
#define IpsX_DCTCOMPIDXERR 218402140
#define IpsX_UNSIMGDIM 218402148
#define IpsX_DLENGTR64 218402156
#define IpsX_SLENGTR64 218402164
/*                                                                          */
