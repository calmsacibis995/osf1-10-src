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
static char *rcsid = "@(#)$RCSfile: aud_mask.c,v $ $Revision: 1.1.4.5 $ (DEC) $Date: 1993/08/04 21:18:40 $";
#endif

#include <sys/audit.h>
#ifndef	AUDIT_MASK_TYPE
#define	AUDIT_MASK_TYPE	char
#endif
#include <stdlib.h>
#include <string.h>

static char *syscall_prefix[] = {
	"old",
	"alternate",
	"obs"
};
#define N_SYSCALL_PFX (sizeof syscall_prefix/sizeof *syscall_prefix)

static char delims[] = " :,\n\t";

/*
 * These structs are to handle caching of alias names (in case there is
 * duplication), and to break alias loops (in case of configuration errors).
 */

struct alias_tag {
    char *name;
    struct alias_tag *next;
    struct alias_ent *mems;
    int active;		/* loop detection */
};

struct alias_ent {
    char *name;
    struct alias_ent *next;
};

static struct alias_tag *head = NULL;

static int is_alias();
static struct alias_tag *fetch_alias();
static struct alias_ent *anew();

/* this is for the support of site events */

static int n_sevents, k_sevents = 0;

/*
 * Add or remove audit event to/from appropriate mask.  If sitemask is
 * NULL, then only base sysalls and trusted events are processed.  If
 * sitemask is non-NULL, then site events and habitat syscalls are also
 * processed.  Habitat events get passed directly to audcntl as encountered.
 *
 * If 'tag' is passed in, it must start out empty.  It will be filled in
 * the first time we get an unresolvable event name.
 */

audit_change_mask ( event, buf1, buf2, sitemask, override_flag , tag , len )
char const *event;
AUDIT_MASK_TYPE *buf1;
AUDIT_MASK_TYPE *buf2;
AUDIT_MASK_TYPE *sitemask;
int override_flag;    /* cmd-line override of event-alias succeed/fail flag */
char *tag;
int len;
{
    char event_l[AUD_MAXEVENT_LEN];
    char eventname[AUD_MAXEVENT_LEN];
    int flags;
    int i, j;
    struct alias_tag *atag;
    struct alias_ent *aent;

    /* split off the event name */
    i = strcspn(event, delims);
    if (event[i] == ' ') {
	for (j = 0;  j < N_SYSCALL_PFX;  j++)
	    if (strncmp(event, syscall_prefix[j], i) == 0)
		break;
	if (j < N_SYSCALL_PFX)
	    i += 1+strcspn(event+i+1, delims); /* scan again past one space */
    }
    if (i >= AUD_MAXEVENT_LEN-1) {
	if (tag && !*tag)
	    (void) strncpy(tag, event, len);
	return -1;
    }
    (void) strncpy(event_l, event, i);
    event_l[i] = '\0';

    /* check for its succeed/fail flags */
    if (override_flag == -1) {
	if (event[i] == ':') {
	    flags = (event[i+1]=='0'? 0 : 2);
	    if (event[i+1] && event[i+2]==':')
		flags |= (event[i+3]=='0'? 0 : 1);
	    else
		flags |= 1;
	    override_flag = flags;
	}
	else
	    flags = 3;
    }
    else
	flags = override_flag;

    /* set syscall event */
    for ( i = FIRST; i <= LAST; i++ )
        if ( strcmp ( event_l, syscallnames[i] ) == 0 ) {
            A_SYSMASK_SET ( buf1, i, (flags&0x2)>>1, flags&0x1 );
            return 0;
        }

    /* set trusted event */
    for ( i = 0; i < N_TRUSTED_EVENTS; i++ )
        if ( strcmp ( event_l, trustedevent[i] ) == 0 ) {
            A_SYSMASK_SET ( buf2, i+MIN_TRUSTED_EVENT, (flags&0x2)>>1, flags&0x1 );
            return 0;
        }

    if (! k_sevents) {
	k_sevents = 1;
	n_sevents = audcntl(GET_NSITEVENTS, (char *)0, 0L, 0L, 0L, 0L);
    }

    /* set sitemask (if sitemask buffer provided) */
    if ( sitemask ) for ( i = 0; i < n_sevents; i++ ) {
        if (( aud_sitevent ( MIN_SITE_EVENT+i, -1, eventname, '\0' ) == 0 )
        && ( strcmp ( event_l, eventname ) == 0 )) {
            A_SITEMASK_SET ( sitemask, i+MIN_SITE_EVENT, (flags&0x2)>>1, flags&0x1 );
            return 0;
        }
    }

    /* set habitat event (if sitemask buffer provided) */
    if ( sitemask )
        if ( audcntl ( SET_HABITAT_EVENT, event_l, 0, flags, 0, 0 ) == 0 )
            return 0;

    /* expand alias if not already expanding it */
    if (((j = is_alias(event_l)) > 0) &&
	((atag = fetch_alias(event_l)) != NULL) &&
	(! atag->active)) {
	atag->active = 1;
	j = 0;
	for (aent = atag->mems;  aent;  aent = aent->next) {
	    j |= audit_change_mask(aent->name, buf1, buf2, sitemask,
				   override_flag, tag, len);
	}
	atag->active = 0;
	return j;
    }

    /* event not found */
    if (tag && !*tag)
	(void) strncpy(tag, event_l, len);
    return -1;
}

/*
 * Check whether something is an event alias name.  Return 0 if not,
 * positive if it is, and negative on errors.
 */

static int
is_alias(name)
char const *name;
{
    char alias[AUD_MAXEVENT_LEN];
    struct alias_tag *atag;
    struct alias_ent *aent;
    int i;

    /* first see if we already know it */
    if (fetch_alias(name) != NULL)
	return 1;

    /* try to look it up in the alias file by fetching its first member */
    if (aud_alias_event(name, 0, alias, sizeof alias) < 0)
	return 0;		/* not found */

    /* ok, start to build the new alias_tag */
    atag = (struct alias_tag *) malloc(sizeof (struct alias_tag));
    if (!atag)
	return -1;		/* punt */
    atag->next = NULL;
    atag->mems = NULL;
    atag->active = 0;
    atag->name = strdup((char *)name);
    if (! atag->name) {
	free(atag);
	return -1;		/* punt */
    }
    aent = anew(alias);
    if (! aent) {
	free(atag->name);	/* punt */
	free(atag);
	return -1;
    }
    atag->mems = aent;
    for (i = 1;  ;  i++) {
	if (aud_alias_event(name, i, alias, sizeof alias) < 0)
	    break;		/* quit at end of list */
	aent->next = anew(alias);
	if (! aent->next) {	/* out of memory now */
	    for (aent = atag->mems;  aent;  aent = atag->mems) {
		atag->mems = aent->next;
		free(aent->name);
		free(aent);
	    }
	    free(atag->name);
	    free(atag);
	    return -1;
	}
	aent = aent->next;
    }

    /* got this far, so put it in the list and say we know it */
    atag->next = head;
    head = atag;
    return 1;
}

static struct alias_ent *
anew(member)
char const *member;
{
    struct alias_ent *aent;

    aent = (struct alias_ent *) malloc(sizeof *aent);
    if (! aent) return NULL;
    aent->name = strdup((char *)member);
    if (! aent->name) {
	free(aent);
	return NULL;
    }
    aent->next = NULL;
    return aent;
}

static struct alias_tag *last_ret = NULL;

static struct alias_tag *
fetch_alias(name)
char const *name;
{
    struct alias_tag *atag;

    /* short-circuit for is_alias followed by fetch_alias */
    if (last_ret && strcmp(last_ret->name, name) == 0)
	return last_ret;
    for (atag = head;  atag;  atag = atag->next)
	if (strcmp(atag->name, name) == 0) {
	    last_ret = atag;
	    return atag;
	}
    return NULL;
}

/*
 * Reclaim core used while building up the audit masks.
 */

void
audit_change_mask_done()
{
    struct alias_tag *atag;
    struct alias_ent *aent;

    last_ret = NULL;
    for (atag = head;  atag;  atag = head) {
	head = atag->next;
	for (aent = atag->mems;  aent;  aent = atag->mems) {
	    atag->mems = aent->next;
	    free(aent->name);
	    free(aent);
	}
	free(atag->name);
	free(atag);
    }
}

/*
 * Build up a process audit mask from nothing.
 * Input is a string of event specifications for audit_change_mask.
 * Ref'ed is the process audit mask array, and optional storage for
 * the first error tag.
 * Returns the accumulated return values from audit_change_mask.
 */

int
audit_build_mask(evstr, pmask, tag, tlen)
const char *evstr;
AUDIT_MASK_TYPE *pmask;
char *tag;
int tlen;
{
    const char *ep;
    AUDIT_MASK_TYPE *tmask = pmask + SYSCALL_MASK_LEN;
    int status = 0;
    int i, j;

    bzero(pmask, sizeof (AUDIT_MASK_TYPE) * AUDIT_MASK_LEN);
    if (tag)
	*tag = '\0';
    if (!evstr)
	return 0;
    for (;;) {
	evstr += strspn(evstr, delims);	/* skip leading junk */
	i = strcspn(evstr, delims);	/* find next break */
	if (i <= 0)
	    break;			/* done if at end */
	for (j = 0;  j < N_SYSCALL_PFX;  j++) {
	    if (strncmp(evstr, syscall_prefix[j], i) == 0)
		break;
	}
	if (j < N_SYSCALL_PFX)
	    i += 1+strcspn(evstr+i+1, delims);	/* accept "old", etc. */
	ep = evstr+i;			/* find end of name */
	if (*ep==':' && *++ep)
		++ep;			/* skip :n */
	if (*ep==':' && *++ep)
		++ep;			/* skip :n twice */
	ep += strcspn(ep, delims);	/* skip trailing junk */
	status |= audit_change_mask(evstr, pmask, tmask, (char *)0, -1,
					tag, tlen);
	evstr = ep;			/* advance pointer */
    }
    audit_change_mask_done();
    return status;
}
