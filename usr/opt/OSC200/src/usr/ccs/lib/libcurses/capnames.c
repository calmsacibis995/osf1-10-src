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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile$ $Revision$ (DEC) $Date$";
#endif
/*
 * HISTORY
 */
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

char *boolnames[] = {
"bw","am","xsb","xhp","xenl","eo","gn","hc","km","hs",
"in","da","db","mir","msgr","os","eslok","xt","hz","ul", "xon", 
"bce","ccc","xhpa","cpix","crxm","chts","daisy","hls","lpix","nxon",
"ndscr","nrrmc","npc","mc5i","xvpa","sam",
0
};
char *boolcodes[] = {
"bw","am","xb","xs","xn","eo","gn","hc","km","hs",
"in","da","db","mi","ms","os","es","xt","hz","ul", "xo",
"be","cc" "YA","YF","YB","HC","YC","hl","YG","nx",
"ND","NR","NP","5i","YD","YE",
0
};
char *numnames[] = {
"cols","it","lines","lm","xmc","pb","vt","wsl",
"bitwin", "bitype", "bufsz", "btns", "spinv", "spinh", "lh", "lw", "ma",
"colors", "maddr", "mjump", "pairs", "wnum", "mcs", "mls", "ncv", "npins",
"nlab", "orc", "orl", "orhi", "orvi", "cps", "widcs",
0
};
char *numcodes[] = {
"co","it","li","lm","sg","pb","vt","ws",
"Yo", "Yp", "Ya", "BT", "Yb", "Yc", "lh", "lw", "ma", 
"Co", "Yd", "Ye", "pa", "MW", "Yf", "Yg", "NC", "Yh",
"Nl", "Yi", "Yj", "Yk", "Yl", "Ym", "Yn",
0
};
char *strnames[] = {
"cbt","bel","cr","csr","tbc","clear","el","ed","hpa","cmdch",
"cup","cud1","home","civis","cub1","mrcup","cnorm","cuf1","ll","cuu1",
"cvvis","dch1","dl1","dsl","hd","smacs","blink","bold","smcup","smdc",
"dim","smir","invis","prot","rev","smso","smul","ech","rmacs","sgr0",
"rmcup","rmdc","rmir","rmso","rmul","flash","ff","fsl","is1","is2",
"is3","if","ich1","il1","ip","kbs","ktbc","kclr","kctab","kdch1",
"kdl1","kcud1","krmir","kel","ked","kf0","kf1","kf10","kf2","kf3",
"kf4","kf5","kf6","kf7","kf8","kf9","khome","kich1","kil1","kcub1",
"kll","knp","kpp","kcuf1","kind","kri","khts","kcuu1","rmkx","smkx",
"lf0","lf1","lf10","lf2","lf3","lf4","lf5","lf6","lf7","lf8",
"lf9","rmm","smm","nel","pad","dch","dl","cud","ich","indn",
"il","cub","cuf","rin","cuu","pfkey","pfloc","pfx","mc0","mc4",
"mc5","rep","rs1","rs2","rs3","rf","rc","vpa","sc","ind",
"ri","sgr","hts","wind","ht","tsl","uc","hu","iprog","ka1",
"ka3","kb2","kc1","kc3","mc5p","box1","box2","batt1","batt2","colb0",
"colb1","colb2","colb3","colb4","colb5","colb6","colb7","colf0","colf1","colf2",
"colf3","colf4","colf5","colf6","colf7","font0","font1","font2","font3","font4",
"font5","font6","font7","kbtab","kdo","kcmd","kcpn","kend","khlp","knl",
"knpn","kpcmd","kppn","kquit","ksel","kscl","kscr","ktab","kmpf1","kmpt1",
"kmpf2","kmpt2","kmpf3","kmpt3","kmpf4","kmpt4","kmpf5","kmpt5","apstr","kmpf6",
"kmpt6","kmpf7","kmpt7","kmpf8","kmpt8","kmpf9","kmpt9","ksf1","ksf2","ksf3",
"ksf4","ksf5","ksf6","ksf7","ksf8","ksf9","ksf10","kf11","kf12","kf13",
"kf14","kf15","kf16","kf17","kf18","kf19","kf20","kf21","kf22","kf23",
"kf24","kf25","kf26","kf27","kf28","kf29","kf30","kf31","kf32","kf33",
"kf34","kf35","kf36","kact","kent",
"acsc", "scesca", "birep", "binel", "bicr", "cpi", "lpi", "chr", "cvr",
"rmp", "csnm", "mgc", "el1", "csin", "colornm", "cwin", "defbi", "defc",
"devt", "dial", "dclk", "dispc", "enacs", "endbi", "smam", "swidm", "sdrfq",
"sitm", "slm", "smicm", "snlq", "snrmq", "smpch", "smsc", "sshm", "ssubm",
"ssupm", "sum", "smxon", "rmam", "rwidm", "ritm", "rlm", "rmicm", "rmpch",
"rmsc", "rshm", "rsubm", "rsupm", "rum", "rmxon", "pause", "hook", "getm",
"wingo", "hup", "initc", "initp", "kbeg", "kcbt", "kcan", "kclo", "kcpy",
"kcrt", "kext", "kf37", "kf38", "kf39", "kf40", "kf41", "kf42", "kf43",
"kf44", "kf45", "kf46", "kf47", "kf48", "kf49", "kf50", "kf51", "kf52",
"kf53", "kf54", "kf55", "kf56", "kf57", "kf58", "kf59", "kf60", "kf61",
"kf62", "kf63", "kfnd", "kmrk", "kmsg", "kmous", "kmov", "knxt", "kopn",
"kopt", "kprv", "kprt", "krdo", "kref", "krfr", "krpl", "krst", "kres",
"ksav", "kBEG", "kCAN", "kCMD", "kCPY", "kCRT", "kDC", "kDL", "kEND",
"kEOL", "kEXT", "kFND", "kHLP", "kHOM", "kIC", "kLFT", "kMSG", "kMOV",
"kNXT", "kOPT", "kPRV", "kPRT", "kRDO", "kRPL", "kRIT", "kRES", "kSAV",
"kSPD", "kUND", "kspd", "kund", "fln", "rmln", "smln", "mhpa", "mcud1",
"mcub1", "mcuf1", "mvpa", "mcuu1", "minfo", "porder", "oc", "op", "mcud",
"mcub", "mcuf", "mcuu", "pctrm", "pfxl", "pln", "pulse", "qdial", "rfi",
"reqmp", "rmclk", "scesc", "scs", "s0ds", "s1ds", "s2ds", "s3ds", "setab",
"setaf", "setb", "smgb", "smgbp", "sclk", "setcolor", "scp", "setf", "smgl",
"smglp", "smglr", "slines", "smgr", "smgrp", "smgtb", "smgt", "smgtp", "sbim",
"scsd", "rbim", "rcsd", "subcs", "supcs", "docr", "tone", "u0", "u1", "u2", 
"u3", "u4", "u5", "u6", "u7", "u8", "u9", "wait", "xoffc", "xonc", "zerom",
0
};
char *strcodes[] = {
"bt","bl","cr","cs","ct","cl","ce","cd","ch","CC",
"cm","do","ho","vi","le","CM","ve","nd","ll","up",
"vs","dc","dl","ds","hd","as","mb","md","ti","dm",
"mh","im","mk","mp","mr","so","us","ec","ae","me",
"te","ed","ei","se","ue","vb","ff","fs","i1","is",
"i2","if","ic","al","ip","kb","ka","kC","kt","kD",
"kL","kd","kM","kE","kS","k0","k1","k;","k2","k3",
"k4","k5","k6","k7","k8","k9","kh","kI","kA","kl",
"kH","kN","kP","kr","kF","kR","kT","ku","ke","ks",
"l0","l1","la","l2","l3","l4","l5","l6","l7","l8",
"l9","mo","mm","nw","pc","DC","DL","DO","IC","SF",
"AL","LE","RI","SR","UP","pk","pl","px","ps","pf",
"po","rp","r1","r2","r3","rf","rc","cv","sc","sf",
"sr","sa","st","wi","ta","ts","uc","hu","iP","K1",
"K3","K2","K4","K5","pO","bx","by","Bx","By","d0",
"d1","d2","d3","d4","d5","d6","d7","c0","c1","c2",
"c3","c4","c5","c6","c7","f0","f1","f2","f3","f4",
"f5","f6","f7","kO","ki","kc","kW","kw","kq","kn",
"kv","kp","kV","kQ","kU","kz","kZ","ko","Kv","KV",
"Kw","KW","Kx","KX","Ky","KY","Kz","KZ","za","Kr",
"KR","Ks","KS","Kt","KT","Ku","KU","S1","S2","S3",
"S4","S5","S6","S7","S8","S9","SA","k<","k>","k!",
"k@","k#","k$","k%","k^","k&","k*","k(","k)","k-",
"k_","k+","k,","k:","k?","k[","k]","k{","k}","k|",
"k~","k/","k=","kJ","@8",
"ac", "SY", "Xy", "Zz", "Yv", "ZA", "ZB", "ZC", "ZD", "rP", "Zy", "MC",
"cb", "ci", "Yw", "CW", "Yx", "ZE", "dv", "DI", "DK", "SP", "eA", "Yy", "SZ",
"ZF", "ZG", "ZH", "ZI", "ZJ", "ZK", "ZL", "SQ", "ST", "ZM", "ZN", "ZO", "ZP",
"SX", "RA", "ZQ", "ZR", "ZS", "ZT", "SS", "SU", "ZU", "ZV", "ZW", "ZX", "RX",
"PA", "fh", "Gm", "WG", "HU", "Ic", "Ip", "@1", "kB", "@2", "@3", "@5", "@6",
"@9", "FR", "FS", "FT", "FU", "FV", "FW", "FX", "FY", "FZ", "Fa", "Fb", "Fc",
"Fd", "Fe", "Ff", "Fg", "Fh", "Fi", "Fj", "Fk", "Fl", "Fm", "Fn", "Fo", "Fp",
"Fq", "Fr", "@0", "%2", "%3", "Km", "%4", "%5", "%6", "%7", "%8", "%9", "%0",
"&1", "&2", "&3", "&4", "&5", "&6", "&9", "&0", "*1", "*2", "*3", "*4", "*5",
"*7", "*8", "*9", "*0", "#1", "#2", "#3", "#4", "%a", "%b", "%c", "%d", "%e",
"%f", "%g", "%h", "%i", "%j", "!1", "!2", "!3", "&7", "&8", "Lf", "LF", "LO",
"ZY", "ZZ", "Za", "Zb", "Zc", "Zd", "Mi", "Ze", "oc", "op", "Zf", "Zg", "Zh",
"Zi", "SV", "xl", "pn", "PU", "QD", "RF", "RQ", "RC", "SW", "Zj", "s0", "s1",
"s2", "s3", "AB", "AF", "Sb", "Zk", "Zl", "SC", "Yz", "sp", "Sf", "ML", "Zm",
"lr", "YZ", "MR", "Zn", "MT", "Zo", "Zp", "Zq", "Zr", "Zs", "Zt", "Zu", "Zv",
"Zw", "TO", "u0", "u1", "u2", "u3", "u4", "u5", "u6", "u7", "u8", "u9", "WA",
"XF", "XN", "Zx",
0
};
