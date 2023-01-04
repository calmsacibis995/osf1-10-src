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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tdbg.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/07/30 11:37:57 $";
#endif
#ifdef XTI
#include <xti.h>
#else
#include <tiuser.h>
#endif
#include <tli/common.h>
#define min(x,y) (x < y) ? x : y

/* trace t_accept */
tr_accept (fd, resfd, call, code)
int fd;
int resfd;
struct t_call *call;
int code;
{
	printf("=============== t_accept =============== \n");
	printf("fd = %d\n", fd);
	printf("resfd = %d\n", resfd);
	printcall("call : ", call);
	printerr(code);
	printf("\n");
}

/* trace t_allocate */
tr_allocate (fd, struct_type, fields, code)
int fd;
int struct_type;
int fields;
char *code;
{
	printf("=============== t_allocate =============== \n");
	printf("fd = %d\n", fd);
	printstructtp (struct_type);
	printfields(fields);	
	if (code = (char *)0)
		printerr(-1);
	else
		printerr(0);
	printf("\n");
}

/* trace t_bind */
tr_bind (fd, req, ret, code)
int fd;
struct t_bind *req;
struct t_bind *ret;
int code;
{
	printf("=============== t_bind =============== \n");
	printf("fd = %d\n", fd);
	printbind("req = ", req);
	printbind("ret = ", ret);
	printerr(code);
	printf("\n");
}

/* trace t_close */
tr_close (fd, code)
int fd;
int code;
{
	printf("=============== t_close =============== \n");
	printf("fd = %d\n", fd);
	printerr(code);
	printf("\n");
}

/* trace t_connect */
tr_connect (fd, sndcall, rcvcall, code)
int fd;
struct t_call *sndcall;
struct t_call *rcvcall;
int code;
{
	printf("=============== t_connect =============== \n");
	printf("fd = %d\n", fd);
	printcall("sndcall : ", sndcall);
	printcall("rcvcall : ", rcvcall);
	printerr(code);
	printf("\n");
}

/* trace t_free */
tr_free (struct_type, code)
int struct_type;
int code;
{
	printf("=============== t_free =============== \n");
	printstructtp (struct_type);
	printerr(code);
	printf("\n");
}

/* trace t_getinfo */
tr_getinfo (fd, info, code)
int fd;
struct t_info *info;
int code;
{
#ifndef XTIBDBG
	printf("=============== t_getinfo =============== \n");
	printf("fd = %d\n", fd);
	printinfo(info);
	printerr(code);
	printf("\n");
#endif
}

/* trace t_getstate */
tr_getstate (fd, code)
int fd;
int code;
{
#ifndef XTIBDBG
	printf("=============== t_getstate =============== \n");
	printf("fd = %d\n", fd);
	printst(code);
	printf("\n");
#endif
}

/* trace t_listen */
tr_listen (fd, call, code)
int fd;
struct t_call * call;
int code;
{
	printf("=============== t_listen =============== \n");
	printf("fd = %d\n", fd);
	printcall("call : ", call);
	printerr(code);
	printf("\n");
}

/* trace t_look */
tr_look (fd, code)
int fd;
int code;
{
	printf("=============== t_look =============== \n");
	printf("fd = %d\n", fd);
	if (code == -1) {
		printerr(code);
		printf("\n");
		return;
	}
	printf("event = ");
	if (code == 0) {
		printf("No Event\n");
		printf("\n");
		return;
	}
	switch (code) {
	case T_LISTEN:
		printf("T_LISTEN");
		break;
	case T_CONNECT:
		printf("T_CONNECT");
		break;
	case T_DATA:
		printf("T_DATA");
		break;
	case T_EXDATA:
		printf("T_EXDATA");
		break;
	case T_DISCONNECT:
		printf("T_DISCONNECT");
		break;
	case T_UDERR:
		printf("T_UDERR");
		break;
	case T_ORDREL:
		printf("T_ORDREL");
		break;
#ifdef XTI
	case T_GODATA:
		printf("T_GODATA");
		break;
	case T_GOEXDATA:
		printf("T_GOEXDATA");
		break;
#endif
	default:
		printf(" Invalid Events");
	}
	printf("\n");
	printf("\n");
}

/* trace t_open */
tr_open (name, oflag, tinfo, code)
char* name;
int oflag;
struct t_info *tinfo;
int code;
{
	printf("=============== t_open =============== \n");
	printf("name = :%.30s:\n", name);
	printinfo(tinfo);
	printoflag(oflag);
	if (code == -1)
		printerr(code);
	else {
		printf("return fd = %d\n", code);
		printf("\n");
	}
}

/* trace t_optmgmt */
tr_optmgmt (fd, req, ret, code)
int fd;
struct t_optmgmt *req;
struct t_optmgmt *ret;
int code;
{
	printf("=============== t_optmgmt =============== \n");
	printf("fd = %d\n", fd);
	printoptmgmt("req = ", req);
	printoptmgmt("ret = ", ret);
	printerr(code);
	printf("\n");
}

/* trace t_rcv */
tr_rcv (fd, buf, nbytes, flags, code)
int fd;
char *buf;
unsigned nbytes;
int *flags;
int code;
{
	printf("=============== t_rcv =============== \n");
	printf("fd = %d\n", fd);
	printbuf(buf, nbytes);
	printf("flags = ");
	if (*flags & T_MORE)
		printf("T_MORE ");
	if (*flags & T_EXPEDITED)
		printf("T_EXPEDITED");
	printf("\n");
	if (code >= 0)
		printf("# of bytes sent = %d", code);
	else
		printerr(code);
	printf("\n");
}

/* trace t_rcvcon */
tr_rcvcon (fd, call, code)
int fd;
struct t_call *call;
int code;
{
	printf("=============== t_rcvconnect =============== \n");
	printf("fd = %d\n", fd);
	printcall("call : ", call);
	printerr(code);
	printf("\n");
}

/* trace t_rcvdis */
tr_rcvdis (fd, discon, code)
int fd;
struct t_discon *discon;
int code;
{
	printf("=============== t_rcvdis =============== \n");
	printf("fd = %d\n", fd);
	printdiscon(discon);
	printerr(code);
	printf("\n");
}

/* trace t_rcvrel */
tr_rcvrel (fd, code)
int fd;
int code;
{
	printf("=============== t_rcvrel =============== \n");
	printf("fd = %d\n", fd);
	printerr(code);
	printf("\n");
}

/* trace t_rcvudata */
tr_rcvudata (fd, ud, flags, code)
int fd;
struct t_unitdata *ud;
int *flags;
int code;
{
	printf("=============== t_rcvudata =============== \n");
	printf("fd = %d\n", fd);
	printud(ud);
	printf("flags = ");
	if (*flags & T_MORE)
		printf("T_MORE ");
        if (*flags & T_EXPEDITED)
		printf("T_EXPEDITED");
	printf("\n");
	printerr(code);
	printf("\n");
}

/* trace t_rcvuderr */
tr_rcvuderr (fd, uderr, code)
int fd;
struct t_uderr *uderr;
int code;
{
	printf("=============== t_rcvuderr =============== \n");
	printf("fd = %d\n", fd);
	printuderr(uderr);
	printerr(code);
	printf("\n");
}

/* trace t_snd */
tr_snd (fd, buf, nbytes, flags, code)
int fd;
char *buf;
unsigned nbytes;
int flags;
int code;
{
	printf("=============== t_snd =============== \n");
	printf("fd = %d\n", fd);
	printbuf(buf, nbytes);
	if (flags & T_MORE)
		printf("T_MORE ");
	if (flags & T_EXPEDITED)
		printf("T_EXPEDITED");
	printf("\n");
	if (code >= 0)
		printf("# of bytes sent = %d", code);
	else
		printerr(code);
	printf("\n");
}

/* trace t_snddis */
tr_snddis (fd, call, code)
int fd;
struct t_call *call;
int code;
{
	printf("=============== t_snddis =============== \n");
	printf("fd = %d\n", fd);
	printcall("call : ", call);
	printerr(code);
	printf("\n");
}

/* trace t_sndrel */
tr_sndrel (fd, code)
int fd;
int code;
{
	printf("=============== t_sndrel =============== \n");
	printf("fd = %d\n", fd);
	printerr(code);
	printf("\n");
}

/* trace t_sndudata */
tr_sndudata (fd, ud, code)
int fd;
struct t_unitdata *ud;
int code;
{
	printf("=============== t_sndudata =============== \n");
	printf("fd = %d\n", fd);
	printud(ud);
	printerr(code);
	printf("\n");
}

/* trace t_sync */
tr_sync (fd, code)
int fd;
int code;
{
#ifndef XTIBDBG
	printf("=============== t_sync =============== \n");
	printf("fd = %d\n", fd);
	printst(code);
	printf("\n");
#endif
}

/* trace t_unbind */
tr_unbind (fd, code)
int fd;
int code;
{
	printf("=============== t_unbind =============== \n");
	printf("fd = %d\n", fd);
	printerr(code);
	printf("\n");
}

printcall(msg, call)
char *msg;
struct t_call *call;
{
	printf("%s", msg);
	if (call == (struct t_call *) 0) {
		printf( " Null\n");
		return;
	}
	printf("\n");
	printnetbuf("  addr",  &call->addr);
	printnetbuf("  opt",   &call->opt);
	printnetbuf("  udata", &call->udata);
	printf("  sequence = %d\n", call->sequence);
}

printnetbuf(msg, nb)
char *msg;
struct netbuf *nb;
{
	int   count;
	if (nb == (struct netbuf *) 0) {
		printf(" = Null\n");
		return;
	}
	printf("%s", msg);
	printf(".maxlen = %u\n", nb->maxlen);
	printf("%s", msg);
	printf(".len    = %u\n", nb->len);
	printf("%s", msg);
	count = min (nb->len, nb->maxlen);
	printf(".buf    = ");
	if (count == 0)
		printf( "::\n");
	else
		printf( ":%.*s:\n", min(count, 60), nb->buf);
}



printerr(code)
int code;
{
	printf("code = ");
	if (code == 0) {
		printf("Success\n");
		return;
	}
	printf("Failure\n");
	printf("t_errno = ");
	switch (_Get_terrno()) {
	case TBADADDR: 		printf("TBADADDR");
				break;
	case TBADOPT:		printf("TBADOPT");
				break;
	case TACCES:		printf("TACCES");
				break;
	case TBADF:		printf("TBADF");
				break;
	case TNOADDR:		printf("TNOADDR");
				break;
	case TOUTSTATE:		printf("TOUTSTATE");
				break;
	case TBADSEQ:		printf("TBADSEQ");
				break;
	case TSYSERR:		printf("TSYSERR");
				break;
	case TLOOK:		printf("TLOOK");
				break;
	case TBADDATA:		printf("TBADDATA");
				break;
	case TBUFOVFLW:		printf("TBUFOVFLW");
				break;
	case TFLOW:		printf("TFLOW");
				break;
	case TNODATA:		printf("TNODATA");
				break;
	case TNODIS:		printf("TNODIS");
				break;
	case TNOUDERR:		printf("TNOUDERR");
				break;
	case TBADFLAG:		printf("TBADFLAG");
				break;
	case TNOREL:		printf("TNOREL");
				break;
	case TNOTSUPPORT:	printf("TNOTSUPPORT");
				break;
	case TSTATECHNG:	printf("TSTATECHNG");
				break;
#ifdef XTI
	case TNOSTRUCTYPE:	printf("TNOSTRUCTYPE");
				break;
	case TBADNAME:		printf("TBADNAME");
				break;
	case TBADQLEN:		printf("TBADQLEN");
				break;
	case TADDRBUSY:		printf("TADDRBUSY");
				break;
#endif
	}
	printf("\n");
}

printstructtp (struct_type)
int struct_type;
{
	printf("struct_type =");
#ifdef XTI
        if (struct_type&T_BIND_STR)     printf(" T_BIND_STR");
        if (struct_type&T_CALL_STR)     printf(" T_CALL_STR");
        if (struct_type&T_OPTMGMT_STR)  printf(" T_OPTMGMT_STR");
        if (struct_type&T_DIS_STR)      printf(" T_DIS_STR");
        if (struct_type&T_UNITDATA_STR) printf(" T_UNITDATA_STR");
        if (struct_type&T_UDERROR_STR)  printf(" T_UDERROR_STR");
        if (struct_type&T_INFO_STR)     printf(" T_INFO_STR");
#else
	if (struct_type & T_BIND) 	printf(" T_BIND");
	if (struct_type & T_CALL) 	printf(" T_CALL");
	if (struct_type & T_OPTMGMT)    printf(" T_OPTMGMT");
	if (struct_type & T_DIS) 	printf(" T_DIS");
	if (struct_type & T_UNITDATA) 	printf(" T_UNITDATA");
	if (struct_type & T_UDERROR) 	printf(" T_UDERROR");
	if (struct_type & T_INFO) 	printf(" T_INFO");
#endif
	printf("\n");
}

printfields(fields)
int fields;
{
	printf("fields =");
	if (fields & T_ALL) {
		printf(" T_ALL\n");
		return;
	}
	if (fields & T_ADDR) 	printf(" T_ADDR");
	if (fields & T_OPT) 	printf(" T_OPT");
	if (fields & T_UDATA) 	printf(" T_UDATA");
	printf("\n");
}

printbind(msg, b)
char * msg;
struct t_bind *b;
{
	printf("%s", msg);
	if (b == (struct t_bind *) 0) {
		printf(" Null\n");
		return;
	}
	printf("\n");
	printnetbuf("  addr", &b->addr);
	printf("  qlen = %d\n", b->qlen);
}

printoptmgmt(msg, o)
char * msg;
struct t_optmgmt *o;
{
	printf("%s\n", msg);
	if (o == (struct t_optmgmt *) 0) {
		printf(" Null\n");
		return;
	}
	printnetbuf("  opt", &o->opt);
	printf("  ");
	printflags(o->flags);	
	printf("\n");
}	

printflags(flags)
long flags;
{
	printf("flags = ");
	switch (flags) {
	case T_NEGOTIATE:
		printf("T_NEGOTIATE");
		break;
	case T_CHECK:
		printf("T_CHECK");
		break;
	case T_DEFAULT:
		printf("T_DEFAULT");
		break;
#ifdef XTI
	case T_SUCCESS:
		printf("T_SUCCESS");
		break;
	case T_FAILURE:
		printf("T_FAILURE");
		break;
#endif
	}
	printf("\n");
}



printinfo(info)
struct t_info *info;
{
	printf("info : ");
	if (info == (struct t_info *) 0) {
		printf(" Null\n");
		return;
	}
	printf("\n");
	printf("  addr     = %d\n", info->addr);
	printf("  options  = %d\n", info->options);
	printf("  tsdu     = %d\n", info->tsdu);
	printf("  etsdu    = %d\n", info->etsdu);
	printf("  connect  = %d\n", info->connect);
	printf("  discon   = %d\n", info->discon);
	printf("  servtype = ");
	switch (info->servtype) {
	case T_COTS:
		printf("T_COTS");
		break;
	case T_COTS_ORD:
		printf("T_COTS_ORD");
		break;
	case T_CLTS:
		printf("T_CLTS");
		break;
	default: 
		printf("Invalid Service Type");
        }
	printf("\n");
}

printoflag(oflag)
int oflag;
{
	printf("oflag = %d\n", oflag);
}

printbuf(buf, nbytes)
char *buf;
int  nbytes;
{
	printf("nbytes = %d\n", nbytes);
	printf("buf    = :%.*s:\n", min(nbytes, 60), buf);
}

printdiscon(discon)
struct t_discon *discon;
{
	printf("discon : \n");
	if (discon == (struct t_discon *) 0) {
		printf(" Null\n");
		return;
	}
	printnetbuf("  udata", &discon->udata);
	printf("reason   = %d\n", discon->reason);
	printf("sequence = %d\n", discon->sequence);
}

printud(ud)
struct t_unitdata *ud;
{
	printf("unitdata : \n");
	if (ud == (struct t_unitdata *) 0) {
		printf(" Null\n");
		return;
	}
	printnetbuf("  addr",  &ud->addr);
	printnetbuf("  opt",   &ud->opt);
	printnetbuf("  udata", &ud->udata);
}

printuderr(uderr)
struct t_uderr *uderr;
{
	printf("uderr : \n");
	if (uderr == (struct t_uderr *) 0) {
		printf(" Null\n");
		return;
	}
	printnetbuf("  addr",  &uderr->addr);
	printnetbuf("  opt",   &uderr->opt);
	printf("  error = %d\n", uderr->error);
}

printst(state)
int state;
{
	printf("state = ");
	switch (state) {
	case T_UNBND: 
		printf("T_UNBND"); 
		break;
	case T_IDLE: 
		printf("T_IDLE");
		break;
	case T_OUTCON: 
		printf("T_OUTCON"); 
		break;
	case T_INCON: 
		printf("T_INCON"); 
		break;
	case T_DATAXFER: 
		printf("T_DATAXFER"); 
		break;
	case T_OUTREL: 
		printf("T_OUTREL"); 
		break;
	case T_INREL: 
		printf("T_INREL"); 
		break;
	      default:
		printf("Invalid State");
	}
	printf("\n");
}










