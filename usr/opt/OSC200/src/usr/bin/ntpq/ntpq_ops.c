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
static char     *sccsid = "@(#)$RCSfile: ntpq_ops.c,v $ $Revision: 4.2.2.3 $ (DEC) $Date: 1992/07/07 16:11:34 $";
#endif
/*
 */

/*
 * ntpdc_ops.c - subroutines which are called to perform operations by xntpdc
 */
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>
#include <ntp/ntp_control.h>
#include <ntp/ntpq.h>

/*
 * Declarations for command handlers in here
 */
void associations(), passociations();
void lassociations(), lpassociations();
void pstatus(), writevar(), readvar();
void addvars(), rmvars(), clearvars();
void showvars(), readlist(), writelist();
void clockvar(), clocklist(), mreadlist();
void mreadvar(), peers(), opeers();
void lpeers(), lopeers();


/*
 * Commands we understand.  Ntpdc imports this.
 */
struct xcmd opcmds[] = {
	{ "associations", associations,	{  NO, NO, NO, NO },
					{ "", "", "", "" },
	"print list of association ID's and statuses for the server's peers" },
	{ "passociations", passociations,	{  NO, NO, NO, NO },
					{ "", "", "", "" },
	"print list of associations returned by last associations command" },
	{ "lassociations", lassociations,	{  NO, NO, NO, NO },
					{ "", "", "", "" },
	"print list of associations including all client information" },
	{ "lpassociations", lpassociations,	{  NO, NO, NO, NO },
					{ "", "", "", "" },
"print last obtained list of associations, including client information" },
	{ "addvars",	addvars,	{ STR, NO, NO, NO },
					{ "name[=value][,...]", "", "", "" },
		"add variables to the variable list or change their values" },
	{ "rmvars",	rmvars,		{ STR, NO, NO, NO },
					{ "name[,...]", "", "", "" },
				"remove variables from the variable list" },
	{ "clearvars",	clearvars,	{ NO, NO, NO, NO },
					{ "", "", "", "" },
				"remove all variables from the variable list" },
	{ "showvars",	showvars,	{ NO, NO, NO, NO },
					{ "", "", "", "" },
				"print variables on the variable list" },
	{ "readlist",	readlist,	{ OPT|UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
	"read the system or peer variables included in the variable list" },
	{ "rl",		readlist,	{ OPT|UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
	"read the system or peer variables included in the variable list" },
	{ "writelist",	writelist,	{ OPT|UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
	"write the system or peer variables included in the variable list" },
	{ "readvar",	readvar,	{ OPT|UINT, OPT|STR, NO, NO },
				{ "assocID", "name=value[,...]", "", "" },
			"read system or peer variables" },
	{ "rv",		readvar,	{ OPT|UINT, OPT|STR, NO, NO },
				{ "assocID", "name=value[,...]", "", "" },
			"read system or peer variables" },
	{ "writevar",	writevar,	{ UINT, STR, NO, NO },
				{ "assocID", "name=value,[...]", "", "" },
			"write system or peer variables" },
	{ "mreadlist",	mreadlist,	{ UINT, UINT, NO, NO },
					{ "assocID", "assocID", "", "" },
	"read the peer variables in the variable list for multiple peers" },
	{ "mrl",	mreadlist,	{ UINT, UINT, NO, NO },
					{ "assocID", "assocID", "", "" },
	"read the peer variables in the variable list for multiple peers" },
	{ "mreadvar",	mreadvar,	{ UINT, UINT, OPT|STR, NO },
			{ "assocID", "assocID", "name=value[,...]", "" },
			"read peer variables from multiple peers" },
	{ "mrv",	mreadvar,	{ UINT, UINT, OPT|STR, NO },
			{ "assocID", "assocID", "name=value[,...]", "" },
			"read peer variables from multiple peers" },
	{ "clocklist",	clocklist,	{ OPT|UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
	"read the clock variables included in the variable list" },
	{ "cl",		clocklist,	{ OPT|UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
	"read the clock variables included in the variable list" },
	{ "clockvar",	clockvar,	{ OPT|UINT, OPT|STR, NO, NO },
				{ "assocID", "name=value[,...]", "", "" },
				"read clock variables" },
	{ "cv",		clockvar,	{ OPT|UINT, OPT|STR, NO, NO },
				{ "assocID", "name=value[,...]", "", "" },
				"read clock variables" },
	{ "pstatus",	pstatus,	{ UINT, NO, NO, NO },
					{ "assocID", "", "", "" },
			"print status information returned for a peer" },
	{ "peers",	peers,		{ NO, NO, NO, NO },
					{ "", "", "", "" },
			"obtain and print a list of the server's peers" },
	{ "lpeers",	lpeers,		{ NO, NO, NO, NO },
					{ "", "", "", "" },
			"obtain and print a list of all peers and clients" },
	{ "opeers",	opeers,		{ NO, NO, NO, NO },
					{ "", "", "", "" },
	"print peer list the old way, with dstadr shown rather than refid" },
	{ "lopeers",	lopeers,	{ NO, NO, NO, NO },
					{ "", "", "", "" },
	"obtain and print a list of all peers and clients showing dstadr" },
	{ 0,		0,		{ NO, NO, NO, NO },
					{ "", "", "", "" }, "" }
};


/*
 * Variable list data space
 */
#define	MAXLIST		64	/* maximum number of variables in list */

struct varlist {
	char *name;
	char *value;
} varlist[MAXLIST] = { 0 };

/*
 * Imported from ntpq.c
 */
extern int showhostnames;
extern int rawmode;
extern int debug;
extern struct servent *server_entry;
extern struct association assoc_cache[];
extern int numassoc;

/*
 * Subroutines in common use here
 */
extern void printvars();
extern void asciize();
extern char *lfptoms();
extern char *nntohost();
extern char *numtoa();
extern char *statustoa();
extern char *uinttoa();

/*
 * For quick string comparisons
 */
#define	STREQ(a, b)	(*(a) == *(b) && strcmp((a), (b)) == 0)


/*
 * checkassocid - return the association ID, checking to see if it is valid
 */
int
checkassocid(value)
	u_int value;
{
	if (value == 0 || value >= 65536) {
		(void) fprintf(stderr, "***Invalid association ID specified\n");
		return 0;
	}
	return (int)value;
}


/*
 * strsave - save a string
 */
char *strsave(str)
	char *str;
{
	extern char *malloc();
	char *cp;
	u_int len;

	len = strlen(str) + 1;
	if ((cp = malloc(len)) == NULL) {
		(void) fprintf(stderr, "Malloc failed!!\n");
		exit(1);
	}

	bcopy(str, cp, len);
	return cp;
}


/*
 * findlistvar - look for the named variable in a list and return if found
 */
struct varlist *
findlistvar(list, name)
	struct varlist *list;
	char *name;
{
	register struct varlist *vl;

	for (vl = list; vl < list + MAXLIST && vl->name != 0; vl++)
		if (STREQ(name, vl->name))
			return vl;
	if (vl < list + MAXLIST)
		return vl;
	return (struct varlist *)0;
}


/*
 * doaddvlist - add variable(s) to the variable list
 */
void
doaddvlist(vlist, vars)
	struct varlist *vlist;
	char *vars;
{
	register struct varlist *vl;
	int len;
	char *name;
	char *value;

	len = strlen(vars);
	while (nextvar(&len, &vars, &name, &value)) {
		vl = findlistvar(vlist, name);
		if (vl == 0) {
			(void) fprintf(stderr, "Variable list full\n");
			return;
		}

		if (vl->name == 0) {
			vl->name = strsave(name);
		} else if (vl->value != 0) {
			(void) free(vl->value);
			vl->value = 0;
		}

		if (value != 0)
			vl->value = strsave(value);
	}
}


/*
 * dormvlist - remove variable(s) from the variable list
 */
void
dormvlist(vlist, vars)
	struct varlist *vlist;
	char *vars;
{
	register struct varlist *vl;
	int len;
	char *name;
	char *value;

	len = strlen(vars);
	while (nextvar(&len, &vars, &name, &value)) {
		vl = findlistvar(vlist, name);
		if (vl == 0 || vl->name == 0) {
			(void) fprintf(stderr, "Variable `%s' not found\n",
			    name);
		} else {
			(void) free(vl->name);
			if (vl->value != 0)
				(void) free(vl->value);
			for ( ; (vl+1) < (varlist+MAXLIST)
			    && (vl+1)->name != 0; vl++) {
				vl->name = (vl+1)->name;
				vl->value = (vl+1)->value;
			}
			vl->name = vl->value = 0;
		}
	}
}


/*
 * doclearvlist - clear a variable list
 */
void
doclearvlist(vlist)
	struct varlist *vlist;
{
	register struct varlist *vl;

	for (vl = vlist; vl < vlist + MAXLIST && vl->name != 0; vl++) {
		(void) free(vl->name);
		vl->name = 0;
		if (vl->value != 0) {
			(void) free(vl->value);
			vl->value = 0;
		}
	}
}


/*
 * makequerydata - form a data buffer to be included with a query
 */
void
makequerydata(vlist, datalen, data)
	struct varlist *vlist;
	int *datalen;
	char *data;
{
	register struct varlist *vl;
	register char *cp, *cpend;
	register int namelen, valuelen;
	register int totallen;

	cp = data;
	cpend = data + *datalen;

	for (vl = vlist; vl < vlist + MAXLIST && vl->name != 0; vl++) {
		namelen = strlen(vl->name);
		if (vl->value == 0)
			valuelen = 0;
		else
			valuelen = strlen(vl->value);
		totallen = namelen + valuelen + (valuelen != 0) + (cp != data);
		if (cp + totallen > cpend)
			break;
		
		if (cp != data)
			*cp++ = ',';
		bcopy(vl->name, cp, namelen);
		cp += namelen;
		if (valuelen != 0) {
			*cp++ = '=';
			bcopy(vl->value, cp, valuelen);
			cp += valuelen;
		}
	}
	*datalen = cp - data;
}


/*
 * doquerylist - send a message including variables in a list
 */
int
doquerylist(vlist, op, associd, auth, rstatus, dsize, datap)
	struct varlist *vlist;
	int op;
	int associd;
	int auth;
	u_short *rstatus;
	int *dsize;
	char **datap;
{
	char data[CTL_MAX_DATA_LEN];
	int datalen;

	datalen = sizeof(data);
	makequerydata(vlist, &datalen, data);

	return doquery(op, associd, auth, datalen, data, rstatus,
	    dsize, datap);
}


/*
 * doprintvlist - print the variables on a list
 */
void
doprintvlist(vlist, fp)
	struct varlist *vlist;
	FILE *fp;
{
	register struct varlist *vl;

	if (vlist->name == 0) {
		(void) fprintf(fp, "No variables on list\n");
	} else {
		for (vl = vlist; vl < vlist + MAXLIST && vl->name != 0; vl++) {
			if (vl->value == 0) {
				(void) fprintf(fp, "%s\n", vl->name);
			} else {
				(void) fprintf(fp, "%s=%s\n",
				    vl->name, vl->value);
			}
		}
	}
}


/*
 * addvars - add variables to the variable list
 */
void
addvars(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	doaddvlist(varlist, pcmd->argval[0].string);
}


/*
 * rmvars - remove variables from the variable list
 */
void
rmvars(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	dormvlist(varlist, pcmd->argval[0].string);
}


/*
 * clearvars - clear the variable list
 */
void
clearvars(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	doclearvlist(varlist);
}


/*
 * showvars - show variables on the variable list
 */
void
showvars(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	doprintvlist(varlist, fp);
}


/*
 * dolist - send a request with the given list of variables
 */
int
dolist(vlist, associd, op, type, fp)
	struct varlist *vlist;
	int associd;
	int op;
	int type;
	FILE *fp;
{
	char *datap;
	int res;
	int dsize;
	u_short rstatus;

	res = doquerylist(vlist, op, associd, 0, &rstatus, &dsize, &datap);

	if (res != 0)
		return 0;

	if (dsize == 0) {
		if (associd == 0)
			(void) fprintf(fp, "No system%s variables returned\n",
			    (type == TYPE_CLOCK) ? " clock" : "");
		else
			(void) fprintf(fp,
			    "No information returned for%s association %u\n",
			    (type == TYPE_CLOCK) ? " clock" : "", associd);
		return 1;
	}

	printvars(dsize, datap, (int)rstatus, type, fp);
	return 1;
}


/*
 * readlist - send a read variables request with the variables on the list
 */
void
readlist(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int associd;

	if (pcmd->nargs == 0) {
		associd = 0;
	} else {
		if (pcmd->argval[0].uval == 0)
			associd = 0;
		else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
			return;
	}

	(void) dolist(varlist, associd, CTL_OP_READVAR,
	    (associd == 0) ? TYPE_SYS : TYPE_PEER, fp);
}


/*
 * writelist - send a write variables request with the variables on the list
 */
void
writelist(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	char *datap;
	int res;
	int associd;
	int dsize;
	u_short rstatus;

	if (pcmd->nargs == 0) {
		associd = 0;
	} else {
		if (pcmd->argval[0].uval == 0)
			associd = 0;
		else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
			return;
	}

	res = doquerylist(varlist, CTL_OP_WRITEVAR, associd, 0, &rstatus,
	    &dsize, &datap);

	if (res != 0)
		return;

	if (dsize == 0)
		(void) fprintf(fp, "done! (no data returned)\n");
	else
		printvars(dsize, datap, (int)rstatus,
		    (associd != 0) ? TYPE_PEER : TYPE_SYS, fp);
	return;
}


/*
 * readvar - send a read variables request with the specified variables
 */
void
readvar(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int associd;
	struct varlist tmplist[MAXLIST];

	if (pcmd->nargs == 0 || pcmd->argval[0].uval == 0)
		associd = 0;
	else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
		return;

	bzero((char *)tmplist, sizeof(tmplist));
	if (pcmd->nargs >= 2)
		doaddvlist(tmplist, pcmd->argval[1].string);

	(void) dolist(tmplist, associd, CTL_OP_READVAR,
	    (associd == 0) ? TYPE_SYS : TYPE_PEER, fp);

	doclearvlist(tmplist);
}


/*
 * writevar - send a write variables request with the specified variables
 */
void
writevar(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	char *datap;
	int res;
	int associd;
	int dsize;
	u_short rstatus;
	struct varlist tmplist[MAXLIST];

	if (pcmd->argval[0].uval == 0)
		associd = 0;
	else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
		return;

	bzero((char *)tmplist, sizeof(tmplist));
	doaddvlist(tmplist, pcmd->argval[1].string);

	res = doquerylist(tmplist, CTL_OP_WRITEVAR, associd, 0, &rstatus,
	    &dsize, &datap);

	doclearvlist(tmplist);

	if (res != 0)
		return;

	if (dsize == 0)
		(void) fprintf(fp, "done! (no data returned)\n");
	else
		printvars(dsize, datap, (int)rstatus,
		    (associd != 0) ? TYPE_PEER : TYPE_SYS, fp);
	return;
}


/*
 * clocklist - send a clock variables request with the variables on the list
 */
void
clocklist(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int associd;

	if (pcmd->nargs == 0) {
		associd = 0;
	} else {
		if (pcmd->argval[0].uval == 0)
			associd = 0;
		else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
			return;
	}

	(void) dolist(varlist, associd, CTL_OP_READCLOCK, TYPE_CLOCK, fp);
}


/*
 * clockvar - send a clock variables request with the specified variables
 */
void
clockvar(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int associd;
	struct varlist tmplist[MAXLIST];

	if (pcmd->nargs == 0 || pcmd->argval[0].uval == 0)
		associd = 0;
	else if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
		return;

	bzero((char *)tmplist, sizeof(tmplist));
	if (pcmd->nargs >= 2)
		doaddvlist(tmplist, pcmd->argval[1].string);

	(void) dolist(tmplist, associd, CTL_OP_READCLOCK, TYPE_CLOCK, fp);

	doclearvlist(tmplist);
}


/*
 * findassidrange - verify a range of association ID's
 */
int
findassidrange(assid1, assid2, from, to)
	u_int assid1;
	u_int assid2;
	int *from;
	int *to;
{
	register int i;
	int f, t;

	if (assid1 == 0 || assid1 > 65535) {
		(void) fprintf(stderr,
		    "***Invalid association ID %u specified\n", assid1);
		return 0;
	}

	if (assid2 == 0 || assid2 > 65535) {
		(void) fprintf(stderr,
		    "***Invalid association ID %u specified\n", assid2);
		return 0;
	}

	f = t = -1;
	for (i = 0; i < numassoc; i++) {
		if (assoc_cache[i].assid == assid1) {
			f = i;
			if (t != -1)
				break;
		}
		if (assoc_cache[i].assid == assid2) {
			t = i;
			if (f != -1)
				break;
		}
	}

	if (f == -1 || t == -1) {
		(void) fprintf(stderr,
		    "***Association ID %u not found in list\n",
		    (f == -1) ? assid1 : assid2);
		return 0;
	}

	if (f < t) {
		*from = f;
		*to = t;
	} else {
		*from = t;
		*to = f;
	}
	return 1;
}



/*
 * mreadlist - send a read variables request for multiple associations
 */
void
mreadlist(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int i;
	int from;
	int to;

	if (!findassidrange(pcmd->argval[0].uval, pcmd->argval[1].uval,
	    &from, &to))
		return;

	for (i = from; i <= to; i++) {
		if (i != from)
			(void) fprintf(fp, "\n");
		if (!dolist(varlist, (int)assoc_cache[i].assid,
		    CTL_OP_READVAR, TYPE_PEER, fp))
			return;
	}
	return;
}


/*
 * mreadvar - send a read variables request for multiple associations
 */
void
mreadvar(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	int i;
	int from;
	int to;
	struct varlist tmplist[MAXLIST];

	if (!findassidrange(pcmd->argval[0].uval, pcmd->argval[1].uval,
	    &from, &to))
		return;

	bzero((char *)tmplist, sizeof(tmplist));
	if (pcmd->nargs >= 3)
		doaddvlist(tmplist, pcmd->argval[2].string);

	for (i = from; i <= to; i++) {
		if (i != from)
			(void) fprintf(fp, "\n");
		if (!dolist(varlist, (int)assoc_cache[i].assid,
		    CTL_OP_READVAR, TYPE_PEER, fp))
			break;
	}
	doclearvlist(tmplist);
	return;
}


/*
 * dogetassoc - query the host for its list of associations
 */
int
dogetassoc(fp)
	FILE *fp;
{
	u_short *datap;
	int res;
	int dsize;
	u_short rstatus;
	extern void sortassoc();

	res = doquery(CTL_OP_READSTAT, 0, 0, 0, (char *)0, &rstatus,
	    &dsize, (char **)&datap);
	
	if (res != 0)
		return 0;
	
	if (dsize == 0) {
		(void) fprintf(fp, "No association ID's returned\n");
		return 0;
	}
	
	if (dsize & 0x3) {
		(void) fprintf(stderr,
	    "***Server returned %d octets, should be multiple of 4\n",
		    dsize);
		return 0;
	}

	numassoc = 0;
	while (dsize > 0) {
		assoc_cache[numassoc].assid = ntohs(*datap);
		datap++;
		assoc_cache[numassoc].status = ntohs(*datap);
		datap++;
		if (++numassoc >= MAXASSOC)
			break;
		dsize -= sizeof(u_short) + sizeof(u_short);
	}
	sortassoc();
	return 1;
}


/*
 * printassoc - print the current list of associations
 */
void
printassoc(showall, fp)
	int showall;
	FILE *fp;
{
	register char *bp;
	int i;
	u_char statval;
	int event;
	u_int event_count;
	char *conf;
	char *reach;
	char *auth;
	char *condition;
	char *last_event;
	char *cnt;
	char buf[128];

	if (numassoc == 0) {
		(void) fprintf(fp, "No association ID's in list\n");
		return;
	}

	/*
	 * Output a header
	 */
	(void) fprintf(fp,
    "ind assID status  conf reach auth condition  last_event cnt\n");
	(void) fprintf(fp,
    "===========================================================\n");
	for (i = 0; i < numassoc; i++) {
		statval = CTL_PEER_STATVAL(assoc_cache[i].status);
		if (!showall && !(statval & (CTL_PST_CONFIG|CTL_PST_REACH)))
			continue;
		event = CTL_PEER_EVENT(assoc_cache[i].status);
		event_count = CTL_PEER_NEVNT(assoc_cache[i].status);
		if (statval & CTL_PST_CONFIG)
			conf = "yes";
		else
			conf = "no";
		if (statval & CTL_PST_REACH) {
			reach = "yes";
			if (statval & CTL_PST_AUTHENABLE) {
				if (statval & CTL_PST_AUTHENTIC)
					auth = "ok ";
				else
					auth = "bad";
			} else {
				auth = "none";
			}

			switch (statval & 0x3) {
			case CTL_PST_SEL_REJECT:
				if (!(statval & CTL_PST_SANE))
					condition = "insane";
				else if (!(statval & CTL_PST_DISP))
					condition = "hi_disp";
				else
					condition = "";
				break;
			case CTL_PST_SEL_SELCAND:
				condition = "sel_cand";
				break;
			case CTL_PST_SEL_SYNCCAND:
				condition = "sync_cand";
				break;
			case CTL_PST_SEL_SYSPEER:
				condition = "sys.peer";
				break;
			}
		} else {
			reach = "no";
			auth = condition = "";
		}

		switch (PEER_EVENT|event) {
		case EVNT_PEERIPERR:
			last_event = "IP error";
			break;
		case EVNT_PEERAUTH:
			last_event = "auth fail";
			break;
		case EVNT_UNREACH:
			last_event = "lost reach";
			break;
		case EVNT_REACH:
			last_event = "reachable";
			break;
		case EVNT_PEERCLOCK:
			last_event = "clock expt";
			break;
		case EVNT_PEERSTRAT:
			last_event = "stratum chg";
			break;
		default:
			last_event = "";
			break;
		}

		if (event_count != 0)
			cnt = uinttoa(event_count);
		else
			cnt = "";
		(void) sprintf(buf,
		"%3d %5u  %04x   %3.3s  %4s  %4.4s %9.9s %11s %2s",
		    i+1, assoc_cache[i].assid, assoc_cache[i].status,
		    conf, reach, auth, condition, last_event, cnt);
		bp = &buf[strlen(buf)];
		while (bp > buf && *(bp-1) == ' ')
			*(--bp) = '\0';
		(void) fprintf(fp, "%s\n", buf);
	}
}



/*
 * associations - get, record and print a list of associations
 */
void
associations(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	if (dogetassoc(fp))
		printassoc(0, fp);
}


/*
 * lassociations - get, record and print a long list of associations
 */
void
lassociations(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	if (dogetassoc(fp))
		printassoc(1, fp);
}


/*
 * passociations - print the association list
 */
void
passociations(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	printassoc(0, fp);
}


/*
 * lpassociations - print the long association list
 */
void
lpassociations(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	printassoc(1, fp);
}


/*
 * radiostatus - print the radio status returned by the server
 */
void
radiostatus(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	char *datap;
	int res;
	int dsize;
	u_short rstatus;

	res = doquery(CTL_OP_READCLOCK, 0, 0, 0, (char *)0, &rstatus,
	    &dsize, &datap);

	if (res != 0)
		return;

	if (dsize == 0) {
		(void) fprintf(fp, "No radio status string returned\n");
		return;
	}

	asciize(dsize, datap, fp);
}

/*
 * pstatus - print peer status returned by the server
 */
void
pstatus(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	char *datap;
	int res;
	int associd;
	int dsize;
	u_short rstatus;

	if ((associd = checkassocid(pcmd->argval[0].uval)) == 0)
		return;

	res = doquery(CTL_OP_READSTAT, associd, 0, 0, (char *)0, &rstatus,
	    &dsize, &datap);

	if (res != 0)
		return;

	if (dsize == 0) {
		(void) fprintf(fp,
		    "No information returned for association %u\n",
		    associd);
		return;
	}

	printvars(dsize, datap, (int)rstatus, TYPE_PEER, fp);
}


/*
 * fixup - fix up a string so we don't get a hanging decimal after it
 */
char *
fixup(width, str)
	int width;
	char *str;
{
	if (str[width-1] == '.')
		str[width-1] = '\0';
	return str;
}


/*
 * when - print how long its been since his last packet arrived
 */
char *
when(ts, rec, reftime)
	l_fp *ts;
	l_fp *rec;
	l_fp *reftime;
{
	int diff;
	l_fp *lasttime;
	static char buf[20];

	if (rec->l_ui != 0)
		lasttime = rec;
	else if (reftime->l_ui != 0)
		lasttime = reftime;
	else
		return "never";
	
	diff = (int)(ts->l_ui - lasttime->l_ui);
	if (diff <= 0) {
		/*
		 * Time warp?
		 */
		diff = 1;
	}

	if (diff <= 2048) {
		(void) sprintf(buf, "%d", diff);
		return buf;
	}

	diff = (diff + 29) / 60;
	if (diff <= 300) {
		(void) sprintf(buf, "%dm", diff);
		return buf;
	}

	diff = (diff + 29) / 60;
	if (diff <= 96) {
		(void) sprintf(buf, "%dh", diff);
		return buf;
	}

	diff = (diff + 11) / 24;
	(void) sprintf(buf, "%dd", diff);
	return buf;
}



/*
 * A list of variables required by the peers command
 */
struct varlist opeervarlist[] = {
	{ "srcadr",	0 },	/* 0 */
	{ "dstadr",	0 },	/* 1 */
	{ "stratum",	0 },	/* 2 */
	{ "hpoll",	0 },	/* 3 */
	{ "ppoll",	0 },	/* 4 */
	{ "reach",	0 },	/* 5 */
	{ "estdelay",	0 },	/* 6 */
	{ "estoffset",	0 },	/* 7 */
	{ "estdisp",	0 },	/* 8 */
	{ "rec",	0 },	/* 9 */
	{ "srcport",	0 },	/* 10 */
	{ "reftime",	0 },	/* 10 */
	{ 0,		0 }
};

struct varlist peervarlist[] = {
	{ "srcadr",	0 },	/* 0 */
	{ "refid",	0 },	/* 1 */
	{ "stratum",	0 },	/* 2 */
	{ "hpoll",	0 },	/* 3 */
	{ "ppoll",	0 },	/* 4 */
	{ "reach",	0 },	/* 5 */
	{ "estdelay",	0 },	/* 6 */
	{ "estoffset",	0 },	/* 7 */
	{ "estdisp",	0 },	/* 8 */
	{ "rec",	0 },	/* 9 */
	{ "reftime",	0 },	/* 10 */
	{ 0,		0 }
};

#define	HAVE_SRCADR	0
#define	HAVE_DSTADR	1
#define	HAVE_REFID	1
#define	HAVE_STRATUM	2
#define	HAVE_PPOLL	3
#define	HAVE_HPOLL	4
#define	HAVE_REACH	5
#define	HAVE_ESTDELAY	6
#define	HAVE_ESTOFFSET	7
#define	HAVE_ESTDISP	8
#define	HAVE_REC	9
#define	HAVE_SRCPORT	10
#define	HAVE_REFTIME	11
#define	MAXHAVE		12

/*
 * Decode an incoming data buffer and print a line in the peer list
 */
int
doprintpeers(pvl, associd, rstatus, datalen, data, fp)
	struct varlist *pvl;
	int associd;
	int rstatus;
	int datalen;
	char *data;
	FILE *fp;
{
	char *name;
	char *value;
	int i;
	int c;

	u_int srcadr;
	u_int dstadr;
	u_int srcport;
	char *dstadr_refid;
	u_int stratum;
	int ppoll;
	int hpoll;
	u_int reach;
	l_fp estdelay;
	l_fp estoffset;
	l_fp estdisp;
	l_fp rec;
	l_fp reftime;
	l_fp ts;
	u_char havevar[MAXHAVE];
	u_int poll;
	char refid_string[10];

	extern struct ctl_var peer_var[];
	extern int findvar();
	extern int decodereach();
	extern int decodetime();
	extern int decodets();
	extern void gettstamp();

	bzero(havevar, sizeof(havevar));
	gettstamp(&ts);
	
	while (nextvar(&datalen, &data, &name, &value)) {
		i = findvar(name, peer_var);
		if (i == 0)
			continue;	/* don't know this one */
		switch (i) {
		case CP_SRCADR:
			if (decodenetnum(value, &srcadr))
				havevar[HAVE_SRCADR] = 1;
			break;
		case CP_DSTADR:
			if (pvl == opeervarlist) {
				if (decodenetnum(value, &dstadr)) {
					havevar[HAVE_DSTADR] = 1;
					dstadr_refid = numtoa(dstadr);
				}
			}
			break;
		case CP_REFID:
			if (pvl == peervarlist) {
				havevar[HAVE_REFID] = 1;
				if (*value == '\0') {
					dstadr_refid = "0.0.0.0";
				} else if (decodenetnum(value, &dstadr)) {
					if (dstadr == 0)
						dstadr_refid = "0.0.0.0";
					else
						dstadr_refid = nntohost(dstadr);
				} else if (strlen(value) <= 4) {
					refid_string[0] = '.';
					(void) strcpy(&refid_string[1], value);
					i = strlen(refid_string);
					refid_string[i] = '.';
					refid_string[i+1] = '\0';
					dstadr_refid = refid_string;
				} else {
					havevar[HAVE_REFID] = 0;
				}
			}
			break;
		case CP_STRATUM:
			if (decodeuint(value, &stratum))
				havevar[HAVE_STRATUM] = 1;
			break;
		case CP_HPOLL:
			if (decodeint(value, &hpoll)) {
				havevar[HAVE_HPOLL] = 1;
				if (hpoll < 0)
					hpoll = NTP_MINPOLL;
			}
			break;
		case CP_PPOLL:
			if (decodeint(value, &ppoll)) {
				havevar[HAVE_PPOLL] = 1;
				if (ppoll < 0)
					ppoll = NTP_MINPOLL;
			}
			break;
		case CP_REACH:
			if (decodeuint(value, &reach))
				havevar[HAVE_REACH] = 1;
			break;
		case CP_ESTDELAY:
			if (decodetime(value, &estdelay))
				havevar[HAVE_ESTDELAY] = 1;
			break;
		case CP_ESTOFFSET:
			if (decodetime(value, &estoffset))
				havevar[HAVE_ESTOFFSET] = 1;
			break;
		case CP_ESTDISP:
			if (decodetime(value, &estdisp))
				havevar[HAVE_ESTDISP] = 1;
			break;
		case CP_REC:
			if (decodets(value, &rec))
				havevar[HAVE_REC] = 1;
			break;
		case CP_SRCPORT:
			if (decodeuint(value, &srcport))
				havevar[HAVE_SRCPORT] = 1;
			break;
		case CP_REFTIME:
			havevar[HAVE_REFTIME] = 1;
			if (!decodets(value, &reftime))
				reftime.l_ui = reftime.l_uf = 0;
			break;
		default:
			break;
		}
	}

	/*
	 * Check to see if the srcport is NTP's port.  If not this probably
	 * isn't a valid peer association.
	 */
	if (havevar[HAVE_SRCPORT] && srcport != NTP_PORT)
		return 1;

	/*
	 * Check to see if we got all of them.  If not, return an
	 * error.
	 */
	for (i = 0; i < MAXHAVE; i++)
		if (!havevar[i]) {
			(void) fprintf(stderr,
		"***Remote host didn't return peer.%s for association %d\n",
			    pvl[i].name, associd);
			return 0;
		}
	

	/*
	 * Got everything, format the line
	 */
	poll = 1<<max(min3(ppoll, hpoll, NTP_MAXPOLL), NTP_MINPOLL);
	i = CTL_PEER_STATVAL(rstatus) & 0x3;
	if (i == CTL_PST_SEL_SYSPEER)
		c = '*';
	else if (i == CTL_PST_SEL_SYNCCAND)
		c = '+';
	else if (i == CTL_PST_SEL_SELCAND)
		c = '.';
	else
		c = ' ';

	(void) fprintf(fp,
	    "%c%-15.15s %-15.15s %2d %5.5s %4d  %3o  %6.6s %7.7s %6.6s\n",
	    c, nntohost(srcadr), dstadr_refid, stratum,
	    when(&ts, &rec, &reftime),
	    poll, reach, fixup(6, lfptoms(&estdelay, 1)),
	    fixup(7, lfptoms(&estoffset, 2)),
	    fixup(6, lfptoms(&estdisp, 1)));
	return 1;
}

#undef	HAVE_SRCADR
#undef	HAVE_DSTADR
#undef	HAVE_STRATUM
#undef	HAVE_PPOLL
#undef	HAVE_HPOLL
#undef	HAVE_REACH
#undef	HAVE_ESTDELAY
#undef	HAVE_ESTOFFSET
#undef	HAVE_ESTDISP
#undef	HAVE_REFID
#undef	HAVE_REC
#undef	HAVE_SRCPORT
#undef	HAVE_REFTIME
#undef	MAXHAVE	


/*
 * dogetpeers - given an association ID, read and print the spreadsheet
 *		peer variables.
 */
int
dogetpeers(pvl, associd, fp)
	struct varlist *pvl;
	int associd;
	FILE *fp;
{
	char *datap;
	int res;
	int dsize;
	u_short rstatus;

#ifdef notdef
	res = doquerylist(pvl, CTL_OP_READVAR, associd, 0, &rstatus,
	    &dsize, &datap);
#else
	/*
	 * Damn fuzzballs
	 */
	res = doquery(CTL_OP_READVAR, associd, 0, 0, (char *)0, &rstatus,
	    &dsize, &datap);
#endif

	if (res != 0)
		return 0;

	if (dsize == 0) {
		(void) fprintf(stderr,
		    "***No information returned for association %d\n",
		    associd);
		return 0;
	}

	
	return doprintpeers(pvl, associd, (int)rstatus, dsize, datap, fp);
}


/*
 * peers - print a peer spreadsheet
 */
void
dopeers(showall, fp)
	int showall;
	FILE *fp;
{
	register int i;

	if (!dogetassoc(fp))
		return;

	(void) fprintf(fp,
"     remote           refid      st  when poll reach  delay  offset   disp\n");
	(void) fprintf(fp,
"==========================================================================\n");

	for (i = 0; i < numassoc; i++) {
		if (!showall &&
		    !(CTL_PEER_STATVAL(assoc_cache[i].status)
		      & (CTL_PST_CONFIG|CTL_PST_REACH)))
			continue;
		if (!dogetpeers(peervarlist, (int)assoc_cache[i].assid, fp)) {
			return;
		}
	}
	return;
}


/*
 * peers - print a peer spreadsheet
 */
void
peers(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	dopeers(0, fp);
}


/*
 * lpeers - print a peer spreadsheet including all fuzzball peers
 */
void
lpeers(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	dopeers(1, fp);
}


/*
 * opeers - print a peer spreadsheet
 */
void
doopeers(showall, fp)
	int showall;
	FILE *fp;
{
	register int i;

	if (!dogetassoc(fp))
		return;

	(void) fprintf(fp,
"     remote           local      st  when poll reach  delay  offset   disp\n");
	(void) fprintf(fp,
"==========================================================================\n");

	for (i = 0; i < numassoc; i++) {
		if (!showall &&
		    !(CTL_PEER_STATVAL(assoc_cache[i].status)
		      & (CTL_PST_CONFIG|CTL_PST_REACH)))
			continue;
		if (!dogetpeers(opeervarlist, (int)assoc_cache[i].assid, fp)) {
			return;
		}
	}
	return;
}


/*
 * opeers - print a peer spreadsheet the old way
 */
void
opeers(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	doopeers(0, fp);
}


/*
 * lopeers - print a peer spreadsheet including all fuzzball peers
 */
void
lopeers(pcmd, fp)
	struct parse *pcmd;
	FILE *fp;
{
	doopeers(1, fp);
}
