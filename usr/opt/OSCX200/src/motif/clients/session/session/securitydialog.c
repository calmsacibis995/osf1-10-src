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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/securitydialog.c,v 1.1.4.2 1993/06/25 18:42:08 Paul_Henderson Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include "smdata.h"
#include "smresource.h"
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef DNETCONN
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif /* DNETCONN */

#include <X11/Xos.h>
#include <signal.h>
#include <setjmp.h>

/*
 * Include files for Xm.
 */
#include <Xm/Text.h>

extern OptionsRec options;

#define max(a,b) ( (a) > (b) ? (a) : (b))

static void AddEntry();
void BuildHostAddress();
int hosts_changed;

char **addlist = NULL;
int num_added = 0;
int add_allocated = 0;

char **rmlist = NULL;
int num_removed = 0;
int remove_allocated = 0;

Bool list_selected = TRUE;

Arg arglist[20];

static Widget host_id, listbox_id, text_id, remove_id, add_id, ok_id, apply_id;

static jmp_buf env;

char **ConvertResourceToList();

void ProcessOkButton();
void ProcessDismissButton();
void ProcessApplyButton();
void ProcessAddButton();
void ProcessRemoveButton();
void list_select();
void list_unselect();


void create_security_attrs()
{
  static Arg desensitize_args[] = {
    {XmNsensitive, FALSE},
  };

  static MrmRegisterArg reglist[] = {
    {"AddButtonCallback", (caddr_t) ProcessAddButton},
    {"DismissButtonCallback", (caddr_t) ProcessDismissButton},
    {"RemoveButtonCallback", (caddr_t) ProcessRemoveButton},
    {"OkButtonCallback", (caddr_t) ProcessOkButton},
    {"ApplyButtonCallback", (caddr_t) ProcessApplyButton},
    {"HostListCallback", (caddr_t) list_select},
    {"listbox_id", (caddr_t) &listbox_id},
    {"text_id", (caddr_t) &text_id},
    {"remove_id", (caddr_t) &remove_id},
    {"add_id", (caddr_t) &add_id},
    {"ok_id", (caddr_t) &ok_id},
    {"apply_id", (caddr_t) &apply_id},
  };

  static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "CustomizeSecurity", smdata.toplevel, 
			&host_id,
			&drm_dummy_class);

    if (!options.session_security) {
      XtSetValues(text_id, desensitize_args, 1);
      XtSetValues(remove_id, desensitize_args, 1);
      XtSetValues(add_id, desensitize_args, 1);
      XtSetValues(ok_id, desensitize_args, 1);
      XtSetValues(apply_id, desensitize_args, 1);
    }

    securitysetup.sec_attr_id = host_id;

    SetListboxContents();

    hosts_changed = False;
}

SetListboxContents()
{
    int i;
    int nhosts;
    char **hostlist;
    XmString *cslist;

    hostlist = ConvertResourceToList(&nhosts, True);
    if (hostlist == NULL) {
	XtSetArg (arglist[0], XmNitems, NULL);
	XtSetArg (arglist[1], XmNitemCount, 0);
	XtSetValues (listbox_id, arglist, 2);
	return;
    }
    cslist = (XmString *) XtMalloc (nhosts * sizeof (XmString *));
    for (i=0; i<nhosts; i++) {
	cslist[i] = XmStringCreate(hostlist[i], 
				def_char_set);
    }

    XtSetArg (arglist[0], XmNitems, cslist);
    XtSetArg (arglist[1], XmNitemCount, nhosts);
    XtSetValues (listbox_id, arglist, 2);

    for (i=0; i<nhosts; i++) {
	XmStringFree (cslist[i]);
	XtFree (hostlist[i]);
    }
    XtFree ((char *)cslist);
    XtFree ((char *)hostlist);
    securitysetup.reset_listbox = False;
    list_unselect();
}


int ClearTextWidget(text_widget)
     Widget text_widget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine is called when the dialog box is managed.
**      It removes any text that is currently in the text widget.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
***
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
XmTextSetString(text_widget, "");
}



void ProcessApplyButton()
{
    SetServerHosts();
    if (hosts_changed == True) PutResources();
    ClearTextWidget(text_id);
}


void ProcessOkButton()
{
/* this call back can be called multiple times if the user hits the
    button quickly several times with either mouse, or CR.  Check
    to see if we have already unmanaged the widget before we do
    anything */
if (securitysetup.managed == ismanaged)
    {
    securitysetup.managed = notmanaged;
    XtUnmanageChild (host_id);
    XmListDeselectAllItems(listbox_id);
    list_unselect();
    SetServerHosts();
    if (hosts_changed == True) PutResources();
    ClearTextWidget(text_id);
    }
}

void ProcessDismissButton()
{
    int i;
    int listbox_changed = False;

if (securitysetup.managed == ismanaged)
    {
    securitysetup.managed = notmanaged;
    XtUnmanageChild (host_id);
    XmListDeselectAllItems(listbox_id);
    list_unselect();
    ClearTextWidget(text_id);

    /* free any removelist data */
    for (i=0; i<num_removed; i++)
	if (rmlist[i] != NULL) {
	    XtFree (rmlist[i]);
	    listbox_changed = True;
	}
    if (rmlist != NULL) XtFree ((char *)rmlist);
    rmlist = NULL;
    num_removed = 0;
    remove_allocated = 0;

    /* free any addlist data */

    for (i=0; i<num_added; i++)
	if (addlist[i] != NULL) {
	    XtFree(addlist[i]);
	    listbox_changed = True;
	}
    if (addlist != NULL) XtFree ((char *)addlist);
    addlist = NULL;
    num_added = 0;
    add_allocated = 0;

    /* reset listbox to previous state */

    if ( listbox_changed == True) securitysetup.reset_listbox = True;
    }

}


void ProcessAddButton()
{
    char text[300];
    char *tw;
    char *tmp;
    XmString cs;
    int i;
    int count;
    int len;
    struct hostent *he;
    static int nameserver_lost();

    XmListDeselectAllItems(listbox_id);
    list_unselect();

    tw = XmTextGetString (text_id);		/* get name from text widget */
    tmp = tw;
    while (*tmp != '\0' && isspace(*tmp)) tmp++; /* strip leading space */
    i = strlen (tmp) - 1;
    if (i < 0) return;				 /* strip trailing space */
    while (i > 0 && isspace(tmp[i])) { tmp[i]='\0'; i -= 1; }

    len = strlen (tmp);
    if (len==0) return;
    strncpy(text,tmp, len+1);
    XtFree (tw);

    /* Don't hang if name cannot be resolved */

    signal(SIGALRM, nameserver_lost);
    alarm(5);
    if (setjmp(env) == 0)
      he = gethostbyname(text);
    else
      he = (struct hostent *)-1;
    alarm(0);
    signal(SIGALRM, SIG_DFL);

    if (he == (struct hostent *)-1) {   /* Name server didn't respond */
	put_error(0, k_security_noserver_msg);
	return;
    }

    else if (he == NULL) {	/* Couldn't find name */
	cs = XmStringCreate(text, 
				def_char_set);
    }

    else {			/* Everything's fine */
	cs = XmStringCreate(he->h_name, 
			    def_char_set);
    }

    if (ValidateText (text) == True) {
	/* is name already in listbox ? */
	XmListSelectItem (listbox_id, cs, False);
	XtSetArg (arglist[0], XmNselectedItemCount, &count);
	XtGetValues (listbox_id, arglist, 1);
	if (count != 0) {
	    /* already there, so ignore it */
	    XmListDeselectAllItems (listbox_id);
	    list_unselect();
	}
	else {
	    /* add it and make it visible */
	    XmListAddItem (listbox_id, cs, 0);
	    XmListSelectItem (listbox_id, cs, False);
	    list_select();
	    /* put entry on 'addlist' */
	    AddEntry (text);
	    hosts_changed = True;
	} 
	XmTextSetString (text_id, "");
    }
    else {
        /*
	return_focus = securitysetup.sec_attr_id;\
	*/

	/* report error */
	put_error(0, k_security_error_msg);

        XmTextSetString (text_id, "");
    }
    XmStringFree (cs);
}

void list_select()
{
  static Arg list_args[] = {
    {XmNsensitive, TRUE},
  };
  
  if (!list_selected && options.session_security) {
    list_selected = TRUE;
    XtSetValues (remove_id, list_args, 1);
  }
}


void list_unselect()
{
  static Arg list_args[] = {
    {XmNsensitive, FALSE},
  };

  if (list_selected) {
    list_selected = FALSE;
    XtSetValues (remove_id, list_args, 1);
  }
}

int ValidateText(text)
    char *text;
{
    int i;
    int len = strlen (text);
    char *pos;
    char c;
    char  buf[300];

    if (len == 0 || len > 299) return (False);

    /* check - "::" exists, then decnet and 
     * nodename <= 6 chars long */

#ifdef DNETCONN
    pos = strstr (text, "::");
    if (pos != 0)  {
	    strncpy(buf, text, pos-text);
	    buf[ pos-text] = 0;
	    if(dnet_addr(buf) == NULL && getnodebyname(buf) == NULL)
	      return(False); 		/* Don't try other domains */
	    else return(True);
    }
#endif /* DNETCONN */

    if((inet_addr(text) == -1)
	&& (gethostbyname(text) ==  (struct hostent *)NULL)) 
      return(False);
    else return(True);
}

void ProcessRemoveButton()
{
    int i;
    int count;
    XmString *items;
    char *text;

    XmTextSetString (text_id, "");
    XtSetArg (arglist[0], XmNselectedItemCount, &count);
    XtSetArg (arglist[1], XmNselectedItems, &items);
    XtGetValues (listbox_id, arglist, 2);

/* put entries on 'removelist' */

/*
 * just look at first selection until workaround is found for
 * multi-selection list
 */
    if (count == 0) return;
    /* text = CSToLatin1 (items[0]); */
    XmStringGetLtoR(items[0], def_char_set, &text);

    RemoveEntry(text);
    XtFree (text);
    XmListDeleteItem(listbox_id, items[0]);
    list_unselect();
    hosts_changed = True;
}

/*
 * get_hostname - Given an internet address, return a name (CHARON.MIT.EDU)
 * or a string representing the address (18.58.0.13) if the name cannot
 * be found.
 */


static int nameserver_timedout;

static char *get_hostname (ha)
XHostAddress *ha;
{
  struct hostent *hp = NULL;
  static int nameserver_lost();
  char *inet_ntoa();
  struct nodeent *np;
#ifdef DNETCONN
  static char nodeaddr[5 + 2 * DN_MAXADDL];
#else
  static char nodeaddr[16];
#endif

  if (ha->family == FamilyInternet) {
    /* gethostbyaddr can take a LONG time if the host does not exist.
       Assume that if it does not respond in NAMESERVER_TIMEOUT seconds
       that something is wrong and do not make the user wait.
       gethostbyaddr will continue after a signal, so we have to
       jump out of it. 
       */
    if(*(ha->address) == 0) return(NULL);
    nameserver_timedout = 0;
    signal(SIGALRM, nameserver_lost);
    alarm(5);
    if (setjmp(env) == 0)
      hp = gethostbyaddr (ha->address, ha->length, AF_INET);
    else
      hp = NULL;
    alarm(0);
    signal(SIGALRM, SIG_DFL);
    if (hp) return (hp->h_name);
    else return (inet_ntoa(*((int *)ha->address)));
  }

#ifdef DNETCONN
  if (ha->family == FamilyDECnet) {
    struct dn_naddr *addr_ptr = (struct dn_naddr *) ha->address;

    if (np = getnodebyaddr(addr_ptr->a_addr, addr_ptr->a_len, AF_DECnet))
      sprintf(nodeaddr, "%s::", np->n_name);
    else sprintf(nodeaddr, "%s::", dnet_htoa(ha->address));

    if(strcmp("?unknown?::", nodeaddr) == 0) return(NULL);

    return(nodeaddr);
  }
#endif /* DNETCONN */

  return (NULL);
}

static int nameserver_lost()
{
  nameserver_timedout = 1;
  longjmp(env, -1);
}


InitServerHosts()
{
unsigned    int	i;
char	*local_node = "0::";
char	*default_access;
XHostAddress localhost;
XHostAddress *h;
int nhosts;
Bool state;

    h=XListHosts (display_id, &nhosts, &state);
    if(!state && options.session_security) {
	    XEnableAccessControl(display_id);
    }
    if (addlist != NULL) {
	for (i=0; i<num_added; i++)
	    if (addlist[i] != NULL) XtFree (addlist[i]);
	XtFree ((char *)addlist);
	addlist = NULL;
	num_added = 0;
	add_allocated = 0;
    }
    if (rmlist != NULL) {
	for (i=0; i<num_removed; i++)
	    if (rmlist[i] != NULL) XtFree (rmlist[i]);
	XtFree ((char *)rmlist);
	rmlist = NULL;
	num_removed = 0;
	remove_allocated = 0;
    }

    rmlist = ConvertResourceToList(&num_removed, True);
    addlist = ConvertResourceToList(&num_added, False);
    if (addlist != NULL) {
	add_allocated = num_added;
	remove_allocated = num_removed;
	SetServerHosts();
    }
    XFree(h);
}



SetServerHosts()
{
    XHostAddress *xhosts;
    int maxhosts;
    int nhosts;
    int i;

    if (options.session_security)
      XDisableAccessControl(display_id);

    maxhosts = max(num_removed, num_added);
    /* before access control was not turned back on if just the
       OK button was hit.  This takes care of that - SR
    */
    
    if (maxhosts == 0) {
      if (options.session_security)
      XEnableAccessControl(display_id);
      return;
    }
    
    xhosts = (XHostAddress *) XtMalloc (maxhosts * sizeof (XHostAddress));

    /* remove hosts from server */

    if (num_removed > 0) {
	nhosts = 0;
	for (i=0; i<num_removed; i++) {
	    if (rmlist[i] != NULL) {
		BuildHostAddress (&xhosts[nhosts], rmlist[i]);
		if(xhosts[nhosts].address != NULL)
		  nhosts += 1;
		XtFree (rmlist[i]);
	    }
	}
	if (nhosts > 0) {
	  if (options.session_security)
	    XRemoveHosts (display_id, xhosts, nhosts);
	    hosts_changed = True;
	}
	for (i=0; i<nhosts; i++)
	    XtFree (xhosts[i].address);
	if (rmlist != NULL) XtFree ((char *)rmlist);
	rmlist = NULL;
	num_removed = 0;
	remove_allocated = 0;
    }

    /* add hosts to server */
    if (num_added > 0) {
	nhosts = 0;
	for (i=0; i<num_added; i++) {
	    if (addlist[i] != NULL) {
		BuildHostAddress (&xhosts[nhosts], addlist[i]);
		if(xhosts[nhosts].address != NULL)
		  nhosts += 1;
		XtFree (addlist[i]);
	    }
	}
	if (nhosts > 0) {
	  if (options.session_security)
	    XAddHosts (display_id, xhosts, nhosts);
	    hosts_changed = True;
	}

	for (i=0; i<nhosts; i++)
	    XtFree (xhosts[i].address);
	if (addlist != NULL) XtFree ((char *)addlist);
	addlist = NULL;
	num_added = 0;
	add_allocated = 0;
    }

    if (options.session_security)
      XEnableAccessControl(display_id);

    XtFree ((char *)xhosts);
}

 
typedef struct {
	int af, xf;
} FamilyMap;

static FamilyMap familyMap[] = {
#ifdef	AF_DECnet
    {AF_DECnet, FamilyDECnet},
#endif
#ifdef	AF_CHAOS
    {AF_CHAOS, FamilyChaos},
#endif
#ifdef	AF_INET
    {AF_INET, FamilyInternet}
#endif
};

#define FAMILIES ((sizeof familyMap)/(sizeof familyMap[0]))

static int XFamily(af)
    int af;
{
    int i;
    for (i = 0; i < FAMILIES; i++)
	if (familyMap[i].af == af)
	    return familyMap[i].xf;
    return -1;
}

void
BuildHostAddress (host, name)
    XHostAddress *host;
    char *name;
{
    char *tmp;
    char *p;
    char *ptr;
#ifdef DNETCONN
    struct dn_naddr dnaddr;
    struct dn_naddr *dnaddrp;
#endif /* DNETCONN */
    struct in_addr addr;	/* so we can point at it */
    struct nodeent *np;
    struct hostent *hp;

    host->address = NULL;
    if(name == NULL) return;
    ptr = strcpy(XtMalloc(strlen(name) +1), name);

#ifdef DNETCONN
    p = strstr (ptr, "::");
    if(p != 0) {
	    *p = 0;
	    if (dnaddrp = dnet_addr(ptr)) 
		    dnaddr = *dnaddrp;
	    else {
		    if ((np = getnodebyname (ptr)) == NULL) {
			    XtFree(ptr);
			    return ;
		    }
		    dnaddr.a_len = np->n_length;
		    bcopy (np->n_addr, dnaddr.a_addr, np->n_length);
	    }
	    host->family = FamilyDECnet;
	    host->length = sizeof(struct dn_naddr);
	    bcopy((char *)&dnaddr,
		  host->address = XtMalloc(sizeof(struct dn_naddr)),
			    sizeof(struct dn_naddr));
	    XtFree(ptr);
	    return;	/* Found it */
    }
#endif /* DNETCONN */
    if ((addr.s_addr = inet_addr(ptr)) != -1) {
	    if (addr.s_addr == 0) return;
	    host->family = FamilyInternet;
	    host->length = sizeof(addr.s_addr);
	    bcopy((char *)&addr.s_addr, 
		  host->address = XtMalloc(sizeof(addr.s_addr)),
				   host->length);
	    XtFree(ptr);
	    return;	/* Found it */
  } 
  /*
   * Is it in the namespace?
   */
  else if (((hp = gethostbyname(name)) == (struct hostent *)NULL)
       || hp->h_addrtype != AF_INET) {
	  XtFree(ptr);
	  return;		/* Sorry, you lose */
  } else {
	  host->family = XFamily(hp->h_addrtype);
	  host->length = hp->h_length;
	  bcopy( hp->h_addr, host->address = XtMalloc( hp->h_length ),
				hp->h_length);

  }
    XtFree(ptr);
}


char **ConvertResourceToList(nhosts_return, just_current)
    int *nhosts_return;
    Bool just_current;
{
    char *resource;    
    char tmp[80];
    char **hostlist;
    char *startpos;
    char *endpos;
    char *final_byte;
    int size;
    int nbytes;
    int nhosts;
    int nserverhosts = 0;
    XHostAddress *h;
    Bool state;
   
    int i,j, k;
#ifdef __osf__
    char *the_rep ;
#else
    XrmRepresentation	the_rep;
#endif
    XrmValue	the_value;

    size = sm_get_string_resource (inumhosts, tmp);
    if (size <= 0 || size > 20 || just_current)
      nhosts = 0;
    else
      nhosts = atoi (tmp);

    h=XListHosts (display_id, &nserverhosts, &state);
    if (nhosts <= 0 && nserverhosts <= 0) return (NULL);

    if (!just_current && nhosts > 0)
      nserverhosts = 0;

    if(nhosts > 0) {
	    if ( XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge], 
		def_table[ihostlist].name, 
                NULL, &the_rep, &the_value)) 
                size = the_value.size;
	    else {
	        nhosts = 0;
                size = 0;
	    }

	    if (size <= 0 && nserverhosts <= 0)
	      return (NULL);
	    if(size > 0) {
		    resource = the_value.addr;

		    final_byte = resource + size;
		    startpos = resource;

		    while((!isgraph(*(final_byte-1))) || (*(final_byte-1) < 0) 
			   || (*(final_byte-1) > 127) ) {
			    final_byte--;
			    if (final_byte == startpos) {
				    if(nserverhosts > 0) {
					    nhosts = 0;
				    } else {
					    XtFree((char *)h);
					    return(NULL);
				    }
			    }
		    }
	    }
    }
    hostlist = (char **) XtMalloc ((nhosts+nserverhosts) * sizeof(char *) + 1);
    i = 0;
    if(nhosts > 0 ) 
      while (startpos < final_byte && i<nhosts) {
	      endpos = startpos+1;
	      while (*endpos != ',' && endpos != final_byte) endpos++;
	      hostlist[i] = XtMalloc (endpos-startpos+1);
	      strncpy (hostlist[i], startpos, endpos-startpos);
	      hostlist[i][endpos-startpos] = '\0';
	      i += 1;
	      startpos = endpos+1;
      }
    for(j = 0, k = 0; j < nserverhosts;j++) {
	    char *ptr;
	    if((ptr = get_hostname(&h[j])) == NULL)
	      continue;
	    hostlist[i+k] = XtMalloc(strlen(ptr)+1);
	    strcpy(hostlist[i+k], ptr);
	    k++;
    }
    if (nhosts != i) {
       /* number of hosts and the list are not in sync */
       *nhosts_return = i + k;
       fprintf (stderr, ".Xdefaults:  DXsession.num_hosts < number of hosts in DXsession.host_list\n");
       XFlush (XtDisplay(smdata.toplevel));
    }
    else
       *nhosts_return = nhosts + k;

    XFree(h);
    return (hostlist);
}

#define HostAllocIncrement 500

PutResources()
{
    XmString **cslist;
    int count;
    char *tmp;
    char str[20];
    int len;
    int i;
    int j;

    char *hostdata = NULL;
    int hostdata_len = 0;
    int hostdata_allocated = 0;

    char *newdata;
    
    /* mark that there have been changes since the last
	time we saved the setup resources */

    smdata.resource_changed = 1;

    XtSetArg (arglist[0], XmNitems, &cslist);
    XtSetArg (arglist[1], XmNitemCount, &count);
    XtGetValues (listbox_id, arglist, 2);

    if (count==0) {
	sm_put_resource (inumhosts, "0");
	sm_put_resource (ihostlist, "0");
	return;
    }

    for (i=0; i<count; i++) {
	/* tmp = CSToLatin1 (cslist[i]); */
        XmStringGetLtoR(cslist[i], def_char_set, &tmp);
	len = strlen (tmp);
	if (len == 0){
	    XtFree (tmp);
	    count -= 1;
	    continue;
	}

	if (hostdata_len + len+2 > hostdata_allocated) {
	    newdata = XtMalloc (hostdata_allocated + HostAllocIncrement);
	    if (hostdata_len > 0)
		for (j=0; j<=hostdata_len; j++) newdata[j] = hostdata[j];
	    else
		newdata[0] = '\0';
	    if (hostdata != NULL) XtFree (hostdata);
	    hostdata = newdata;
	    hostdata_allocated += HostAllocIncrement;	    
	}
	strcat (hostdata, tmp);
	strcat (hostdata, ",");
	hostdata_len += len+1;
	XtFree (tmp);
    }
    hostdata[hostdata_len-1] = '\0';

    sprintf (str, "%d",count);
    sm_put_resource (inumhosts, str);
    sm_put_resource (ihostlist, hostdata);

    hosts_changed = False;
}

/* list handling for the addlist and removelist */

#define AllocIncrement 20

static void AddEntry(host)
    char *host;
{
    int i;
    char **newlist;

    for (i=0; i<num_removed; i++) {
	if (rmlist[i] != NULL)
	    if (strcmp (rmlist[i], host) == 0) {
		XtFree (rmlist[i]);
		rmlist[i] = NULL;
		return;
	    }
    }

    if (num_added >= add_allocated) {
	newlist = (char **) XtMalloc ((add_allocated + AllocIncrement)
							* sizeof (char *));
	for (i=0; i<add_allocated; i++) newlist[i] = addlist[i];
	if (addlist != NULL) XtFree ((char *)addlist);
	addlist = newlist;
	add_allocated += AllocIncrement;
    }
    addlist[num_added] = (char *) XtMalloc (strlen(host)+1);
    strcpy (addlist[num_added], host);
    num_added += 1;
}

RemoveEntry(host)
    char *host;
{
    int i;
    char **newlist;

    for (i=0; i<num_added; i++) {
	if (addlist[i] != NULL)
	    if (strcmp (addlist[i], host) == 0) {
		XtFree (addlist[i]);
		addlist[i] = NULL;
		return;
	    }
    }

    if (num_removed >= remove_allocated) {
	newlist = (char **) XtMalloc ((remove_allocated + AllocIncrement)
							* sizeof (char *));
	for (i=0; i<remove_allocated; i++) newlist[i] = rmlist[i];
	if (rmlist != NULL) XtFree ((char *)rmlist);
	rmlist = newlist;
	remove_allocated += AllocIncrement;
    }
    rmlist[num_removed] = (char *) XtMalloc (strlen(host)+1);
    strcpy (rmlist[num_removed], host);
    num_removed += 1;
}
