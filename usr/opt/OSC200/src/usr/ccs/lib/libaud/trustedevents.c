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
static char *rcsid = "@(#)$RCSfile: trustedevents.c,v $ $Revision: 1.1.5.5 $ (DEC) $Date: 1993/11/18 01:13:33 $";
#endif

/* trusted event names */
char *trustedevent[] = {
    "audit_start",          /* 512 */
    "audit_stop",           /* 513 */
    "audit_setup",          /* 514 */
    "audit_suspend",        /* 515 */
    "audit_log_change",     /* 516 */
    "audit_log_creat",      /* 517 */
    "audit_xmit_fail",      /* 518 */
    "audit_reboot",         /* 519 */
    "audit_log_overwrite",  /* 520 */
    "audit_daemon_exit",    /* 521 */
    "login",                /* 522 */
    "logout",               /* 523 */
    "auth_event",           /* 524 */
    "audgen8",              /* 525 */
    "MLS+compatibility",    /* 526 */
    "#527",                 /* 527 */
    "#528",                 /* 528 */
    "#529",                 /* 529 */
    "#530",                 /* 530 */
    "#531",                 /* 531 */
    "#532",                 /* 532 */
    "#533",                 /* 533 */
    "#534",                 /* 534 */
    "#535",                 /* 535 */
    "#536",                 /* 536 */
    "#537",                 /* 537 */
    "#538",                 /* 538 */
    "#539",                 /* 539 */
    "XServerStartup",       /* 540 */
    "XServerShutdown",      /* 541 */
    "XClientStartup",       /* 542 */
    "XClientShutdown",      /* 543 */
    "XServerDac",           /* 544 */
    "XClientIPC",           /* 545 */
    "XObjectCreate",        /* 546 */
    "XObjectRename",        /* 547 */
    "XObjectDestroy",       /* 548 */
    "XObjectDac",           /* 549 */
    "XObjectRead",          /* 550 */
    "XObjectWrite",         /* 551 */
    "#552",                 /* 552 */
    "#553",                 /* 553 */
    "#554",                 /* 554 */
    "#555",                 /* 555 */
    "#556",                 /* 556 */
    "#557",                 /* 557 */
    "#558",                 /* 558 */
    "#559",                 /* 559 */
    "#560",                 /* 560 */
    "#561",                 /* 561 */
    "#562",                 /* 562 */
    "#563",                 /* 563 */
    "#564",                 /* 564 */
    "#565",                 /* 565 */
    "#566",                 /* 566 */
    "#567",                 /* 567 */
    "#568",                 /* 568 */
    "#569",                 /* 569 */
    "#570",                 /* 570 */
    "#571",                 /* 571 */
    "#572",                 /* 572 */
    "#573",                 /* 573 */
    "#574",                 /* 574 */
    "#575"                  /* 575 */
};
