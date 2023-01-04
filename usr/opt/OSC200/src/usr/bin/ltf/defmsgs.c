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
static char *rcsid = "@(#)$RCSfile: defmsgs.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/10/08 09:53:15 $";
#endif

#include "ltfdefs.h"

#define  NMSGS    	139
#define  MSIZ     	255

extern  char  *strncpy();

defmsgs()
{
	static   char    msg[NMSGS][MSIZ+1];

	ALTERN	= strncpy(msg[0],MSGSTR(MSG_ALTERN,ALTERN),MSIZ);
	ANSIV	= strncpy(msg[1],MSGSTR(MSG_ANSIV,ANSIV),MSIZ);
	BADCENT	= strncpy(msg[2],MSGSTR(MSG_BADCENT,BADCENT),MSIZ);
	BADCNT1	= strncpy(msg[3],MSGSTR(MSG_BADCNT1,BADCNT1),MSIZ);
	BADCNT2	= strncpy(msg[4],MSGSTR(MSG_BADCNT2,BADCNT2),MSIZ);
	BADCNT3	= strncpy(msg[5],MSGSTR(MSG_BADCNT3,BADCNT3),MSIZ);
	BADST	= strncpy(msg[6],MSGSTR(MSG_BADST,BADST),MSIZ);
	BFRCNE	= strncpy(msg[7],MSGSTR(MSG_BFRCNE,BFRCNE),MSIZ);
	BYTE	= strncpy(msg[8],MSGSTR(MSG_BYTE,BYTE),MSIZ);
	CANTBUF	= strncpy(msg[9],MSGSTR(MSG_CANTBUF,CANTBUF),MSIZ);
	CANTCGET= strncpy(msg[10],MSGSTR(MSG_CANTCGET,CANTCGET),MSIZ);
	CANTCLS	= strncpy(msg[11],MSGSTR(MSG_CANTCLS,CANTCLS),MSIZ);
	CANTCHD	= strncpy(msg[12],MSGSTR(MSG_CANTCHD,CANTCHD),MSIZ);
	CANTCHW	= strncpy(msg[13],MSGSTR(MSG_CANTCHW,CANTCHW),MSIZ);
	CANTCRE	= strncpy(msg[14],MSGSTR(MSG_CANTCRE,CANTCRE),MSIZ);
	CANTDEVIO=strncpy(msg[15],MSGSTR(MSG_CANTDEVIO,CANTDEVIO),MSIZ);
	CANTFPW	= strncpy(msg[16],MSGSTR(MSG_CANTFPW,CANTFPW),MSIZ);
	CANTFSF	= strncpy(msg[17],MSGSTR(MSG_CANTFSF,CANTFSF),MSIZ);
	CANTL1	= strncpy(msg[18],MSGSTR(MSG_CANTL1,CANTL1),MSIZ);
	CANTLF	= strncpy(msg[19],MSGSTR(MSG_CANTLF,CANTLF),MSIZ);
	CANTMKD	= strncpy(msg[20],MSGSTR(MSG_CANTMKD,CANTMKD),MSIZ);
	CANTOD	= strncpy(msg[21],MSGSTR(MSG_CANTOD,CANTOD),MSIZ);
	CANTOPEN= strncpy(msg[22],MSGSTR(MSG_CANTOPEN,CANTOPEN),MSIZ);
	CANTOPW	= strncpy(msg[23],MSGSTR(MSG_CANTOPW,CANTOPW),MSIZ);
	CANTPER	= strncpy(msg[24],MSGSTR(MSG_CANTPER,CANTPER),MSIZ);
	CANTRL	= strncpy(msg[25],MSGSTR(MSG_CANTRL,CANTRL),MSIZ);
	CANTRD	= strncpy(msg[26],MSGSTR(MSG_CANTRD,CANTRD),MSIZ);
	CANTREW	= strncpy(msg[27],MSGSTR(MSG_CANTREW,CANTREW),MSIZ);
	CANTRSL	= strncpy(msg[28],MSGSTR(MSG_CANTRSL,CANTRSL),MSIZ);
	CANTSTS	= strncpy(msg[29],MSGSTR(MSG_CANTSTS,CANTSTS),MSIZ);
	CANTSTW	= strncpy(msg[30],MSGSTR(MSG_CANTSTW,CANTSTW),MSIZ);
	CANTWEOF= strncpy(msg[31],MSGSTR(MSG_CANTWEOF,CANTWEOF),MSIZ);
	CANTWVL	= strncpy(msg[32],MSGSTR(MSG_CANTWVL,CANTWVL),MSIZ);
	CONFF	= strncpy(msg[33],MSGSTR(MSG_CONFF,CONFF),MSIZ);
	DIRCRE	= strncpy(msg[34],MSGSTR(MSG_DIRCRE,DIRCRE),MSIZ);
	ENFNAM	= strncpy(msg[35],MSGSTR(MSG_ENFNAM,ENFNAM),MSIZ);
	EINVLD	= strncpy(msg[36],MSGSTR(MSG_EINVLD,EINVLD),MSIZ);
	EOFINM	= strncpy(msg[37],MSGSTR(MSG_EOFINM,EOFINM),MSIZ);
	ERRDEV	= strncpy(msg[38],MSGSTR(MSG_ERRDEV,ERRDEV),MSIZ);
	ERREOT	= strncpy(msg[39],MSGSTR(MSG_ERREOT,ERREOT),MSIZ);
	ERRUNIT	= strncpy(msg[40],MSGSTR(MSG_ERRUNIT,ERRUNIT),MSIZ);
	ERRWRF	= strncpy(msg[41],MSGSTR(MSG_ERRWRF,ERRWRF),MSIZ);
	EXISTS	= strncpy(msg[42],MSGSTR(MSG_EXISTS,EXISTS),MSIZ);
	FILENNG	= strncpy(msg[43],MSGSTR(MSG_FILENNG,FILENNG),MSIZ);
	FNTL	= strncpy(msg[44],MSGSTR(MSG_FNTL,FNTL),MSIZ);
	FSTCB	= strncpy(msg[45],MSGSTR(MSG_FSTCB,FSTCB),MSIZ);
	FUFTL	= strncpy(msg[46],MSGSTR(MSG_FUFTL,FUFTL),MSIZ);
	GETWDF	= strncpy(msg[47],MSGSTR(MSG_GETWDF,GETWDF),MSIZ);
	HELP1	= strncpy(msg[48],MSGSTR(MSG_HELP1,HELP1),MSIZ);
	HELP2	= strncpy(msg[49],MSGSTR(MSG_HELP2,HELP2),MSIZ);
	HELP3	= strncpy(msg[50],MSGSTR(MSG_HELP3,HELP3),MSIZ);
	HELP4	= strncpy(msg[51],MSGSTR(MSG_HELP4,HELP4),MSIZ);
	HELP5	= strncpy(msg[52],MSGSTR(MSG_HELP5,HELP5),MSIZ);
	HELP6	= strncpy(msg[53],MSGSTR(MSG_HELP6,HELP6),MSIZ);
	HELP7	= strncpy(msg[54],MSGSTR(MSG_HELP7,HELP7),MSIZ);
	HELP8	= strncpy(msg[55],MSGSTR(MSG_HELP8,HELP8),MSIZ);
	HELP9	= strncpy(msg[56],MSGSTR(MSG_HELP9,HELP9),MSIZ);
	HELP10	= strncpy(msg[57],MSGSTR(MSG_HELP10,HELP10),MSIZ);
	HELP11	= strncpy(msg[58],MSGSTR(MSG_HELP11,HELP11),MSIZ);
	HELP12	= strncpy(msg[59],MSGSTR(MSG_HELP12,HELP12),MSIZ);
	HELP13	= strncpy(msg[60],MSGSTR(MSG_HELP13,HELP13),MSIZ);
	HELP14	= strncpy(msg[61],MSGSTR(MSG_HELP14,HELP14),MSIZ);
	HELP15	= strncpy(msg[62],MSGSTR(MSG_HELP15,HELP15),MSIZ);
	HELP16	= strncpy(msg[63],MSGSTR(MSG_HELP16,HELP16),MSIZ);
	HELP17	= strncpy(msg[64],MSGSTR(MSG_HELP17,HELP17),MSIZ);
	HELP18	= strncpy(msg[65],MSGSTR(MSG_HELP18,HELP18),MSIZ);
	HELP19	= strncpy(msg[66],MSGSTR(MSG_HELP19,HELP19),MSIZ);
	HELP20	= strncpy(msg[67],MSGSTR(MSG_HELP20,HELP20),MSIZ);
	HELP21	= strncpy(msg[68],MSGSTR(MSG_HELP21,HELP21),MSIZ);
	HELP22	= strncpy(msg[69],MSGSTR(MSG_HELP22,HELP22),MSIZ);
	HELP23	= strncpy(msg[70],MSGSTR(MSG_HELP23,HELP23),MSIZ);
	HELP24	= strncpy(msg[71],MSGSTR(MSG_HELP24,HELP24),MSIZ);
	HELP25	= strncpy(msg[72],MSGSTR(MSG_HELP25,HELP25),MSIZ);
	HELP26	= strncpy(msg[73],MSGSTR(MSG_HELP26,HELP26),MSIZ);
	HLINKTO	= strncpy(msg[74],MSGSTR(MSG_HLINKTO,HLINKTO),MSIZ);
	HOSTF	= strncpy(msg[75],MSGSTR(MSG_HOSTF,HOSTF),MSIZ);
	IMPIDC	= strncpy(msg[76],MSGSTR(MSG_IMPIDC,IMPIDC),MSIZ);
	IMPIDM	= strncpy(msg[77],MSGSTR(MSG_IMPIDM,IMPIDM),MSIZ);
	INTERCH	= strncpy(msg[78],MSGSTR(MSG_INTERCH,INTERCH),MSIZ);
	INVBS	= strncpy(msg[79],MSGSTR(MSG_INVBS,INVBS),MSIZ);
	INVOWN	= strncpy(msg[80],MSGSTR(MSG_INVOWN,INVOWN),MSIZ);
	INVLD	= strncpy(msg[81],MSGSTR(MSG_INVLD,INVLD),MSIZ);
	INVLF	= strncpy(msg[82],MSGSTR(MSG_INVLF,INVLF),MSIZ);
	INVLFS	= strncpy(msg[83],MSGSTR(MSG_INVLFS,INVLFS),MSIZ);
	INVLFW	= strncpy(msg[84],MSGSTR(MSG_INVLFW,INVLFW),MSIZ);
	INVLNO	= strncpy(msg[85],MSGSTR(MSG_INVLNO,INVLNO),MSIZ);
	INVNF	= strncpy(msg[86],MSGSTR(MSG_INVNF,INVNF),MSIZ);
	INVPN	= strncpy(msg[87],MSGSTR(MSG_INVPN,INVPN),MSIZ);
	INVPNUSE= strncpy(msg[88],MSGSTR(MSG_INVPNUSE,INVPNUSE),MSIZ);
	INVPS	= strncpy(msg[89],MSGSTR(MSG_INVPS,INVPS),MSIZ);
#ifndef U11
	INVUNIT	= strncpy(msg[90],MSGSTR(MSG_INVUNIT,INVUNIT),MSIZ);
#else
	INVRS	= strncpy(msg[91],MSGSTR(MSG_INVRS,INVRS),MSIZ);
#endif
	INVVID1	= strncpy(msg[92],MSGSTR(MSG_INVVID1,INVVID1),MSIZ);
	INVVID2	= strncpy(msg[93],MSGSTR(MSG_INVVID2,INVVID2),MSIZ);
	MHL	= strncpy(msg[94],MSGSTR(MSG_MHL,MHL),MSIZ);
	MS1	= strncpy(msg[95],MSGSTR(MSG_MS1,MS1),MSIZ);
	MS2	= strncpy(msg[96],MSGSTR(MSG_MS2,MS2),MSIZ);
	MS3	= strncpy(msg[97],MSGSTR(MSG_MS3,MS3),MSIZ);
	MULTIV1	= strncpy(msg[98],MSGSTR(MSG_MULTIV1,MULTIV1),MSIZ);
	NOARGS	= strncpy(msg[99],MSGSTR(MSG_NOARGS,NOARGS),MSIZ);
	NOBLK	= strncpy(msg[100],MSGSTR(MSG_NOBLK,NOBLK),MSIZ);
	NOFIL	= strncpy(msg[101],MSGSTR(MSG_NOFIL,NOFIL),MSIZ);
	NOFUNC	= strncpy(msg[102],MSGSTR(MSG_NOFUNC,NOFUNC),MSIZ);
	NOINP	= strncpy(msg[103],MSGSTR(MSG_NOINP,NOINP),MSIZ);
	NOMEM	= strncpy(msg[104],MSGSTR(MSG_NOMEM,NOMEM),MSIZ);
	NOMDIR	= strncpy(msg[105],MSGSTR(MSG_NOMDIR,NOMDIR),MSIZ);
	NONAFN	= strncpy(msg[106],MSGSTR(MSG_NONAFN,NONAFN),MSIZ);
	NOPOS	= strncpy(msg[107],MSGSTR(MSG_NOPOS,NOPOS),MSIZ);
#ifdef U11
	NOREC	= strncpy(msg[108],MSGSTR(MSG_NOREC,NOREC),MSIZ);
#endif
	NOTEX	= strncpy(msg[109],MSGSTR(MSG_NOTEX,NOTEX),MSIZ);
	NOTONP	= strncpy(msg[110],MSGSTR(MSG_NOTONP,NOTONP),MSIZ);
	NOTONV	= strncpy(msg[111],MSGSTR(MSG_NOTONV,NOTONV),MSIZ);
	NOTSU	= strncpy(msg[112],MSGSTR(MSG_NOTSU,NOTSU),MSIZ);
	NOVALFI	= strncpy(msg[113],MSGSTR(MSG_NOVALFI,NOVALFI),MSIZ);
	NOVOL	= strncpy(msg[114],MSGSTR(MSG_NOVOL,NOVOL),MSIZ);
	OFFL1	= strncpy(msg[115],MSGSTR(MSG_OFFL1,OFFL1),MSIZ);
	OFFL2	= strncpy(msg[116],MSGSTR(MSG_OFFL2,OFFL2),MSIZ);
	OWNRID	= strncpy(msg[117],MSGSTR(MSG_OWNRID,OWNRID),MSIZ);
	OVRWRT	= strncpy(msg[118],MSGSTR(MSG_OVRWRT,OVRWRT),MSIZ);
#ifdef U11
	RECLTS	= strncpy(msg[119],MSGSTR(MSG_RECLTS,RECLTS),MSIZ);
#endif
	SCNDCB	= strncpy(msg[120],MSGSTR(MSG_SCNDCB,SCNDCB),MSIZ);
	SLINKTO	= strncpy(msg[121],MSGSTR(MSG_SLINKTO,SLINKTO),MSIZ);
	SPCLDF	= strncpy(msg[122],MSGSTR(MSG_SPCLDF,SPCLDF),MSIZ);
	STOPCRIN= strncpy(msg[123],MSGSTR(MSG_STOPCRIN,STOPCRIN),MSIZ);
#if 0
	TAPEB	= strncpy(msg[124],MSGSTR(MSG_TAPEB,TAPEB),MSIZ);
	TAPEBS	= strncpy(msg[125],MSGSTR(MSG_TAPEBS,TAPEBS),MSIZ);
#endif
	TMA	= strncpy(msg[126],MSGSTR(MSG_TMA,TMA),MSIZ);
#ifndef U11
	TRYHELP	= strncpy(msg[127],MSGSTR(MSG_TRYHELP,TRYHELP),MSIZ);
#else
	TRYHELP = strncpy(msg[127],MSGSTR(MSG_TRYHELPU11,TRYHELP),MSIZ);
#endif
	TRYNH3	= strncpy(msg[128],MSGSTR(MSG_TRYNH3,TRYNH3),MSIZ);
	UNQ	= strncpy(msg[129],MSGSTR(MSG_UNQ,UNQ),MSIZ);
	USEDF	= strncpy(msg[130],MSGSTR(MSG_USEDF,USEDF),MSIZ);
#ifndef U11
	USE1	= strncpy(msg[131],MSGSTR(MSG_USE1,USE1),MSIZ);
	USE2	= strncpy(msg[132],MSGSTR(MSG_USE2,USE2),MSIZ);
#else
	USE1    = strncpy(msg[131],MSGSTR(MSG_USE1U11,USE1),MSIZ);
	USE2    = strncpy(msg[132],MSGSTR(MSG_USE2U11,USE2),MSIZ);
#endif
	UNSAV	= strncpy(msg[133],MSGSTR(MSG_UNSAV,UNSAV),MSIZ);
	VOLCRE	= strncpy(msg[134],MSGSTR(MSG_VOLCRE,VOLCRE),MSIZ);
	VOLIDTL	= strncpy(msg[135],MSGSTR(MSG_VOLIDTL,VOLIDTL),MSIZ);
	VOLIS	= strncpy(msg[136],MSGSTR(MSG_VOLIS,VOLIS),MSIZ);
	WRLINM	= strncpy(msg[137],MSGSTR(MSG_WRLINM,WRLINM),MSIZ);
	WRTLCK	= strncpy(msg[138],MSGSTR(MSG_WRTLCK,WRTLCK),MSIZ);
}
