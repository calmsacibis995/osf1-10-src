#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
#
# HISTORY
#
#       @(#)$RCSfile: Makefile,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/29 19:47:53 $
#	@(#)make.script	1.1 88/03/07 4.0NFSSRC SMI

#
#  ***** DOM must be filled in with the domainname !!
#
DOM=

SENDMAIL=/usr/sbin/sendmail
DIR=/var/yp/src
NOPUSH=""
ALIASES=$(DIR)/mail.aliases
AWK=/usr/bin/awk
TOUCH=/usr/bin/touch
ECHO=/usr/bin/echo
SED=/usr/bin/sed
RM=/usr/bin/rm
YPDIR=/var/yp
YPDBDIR=/var/yp
YPPUSH=$(YPDIR)/yppush
MAKEDBM=$(YPDIR)/makedbm
REVNETGROUP=$(YPDIR)/revnetgroup
STDHOSTS=$(YPDIR)/stdhosts

all:  passwd group hosts networks rpc services protocols netgroup aliases

#all: passwd group hosts networks rpc services protocols netgroup aliases \
#     auto.home auto.master

$(YPDBDIR)/$(DOM)/passwd.time: $(DIR)/passwd
	-@if [ -f $(DIR)/passwd ]; then \
		$(AWK) 'BEGIN { FS=":"; OFS="\t"; } /^[a-zA-Z0-9_]/ { print $$1, $$0 }' \
		  $(DIR)/passwd  | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/passwd.byname; \
		$(AWK) 'BEGIN { FS=":"; OFS="\t"; } /^[a-zA-Z0-9_]/ { printf("%-10d ", $$3); print $$0 }' \
		  $(DIR)/passwd | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/passwd.byuid; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/passwd.time; \
		$(ECHO) "updated passwd"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM)  passwd.byname; \
			$(YPPUSH) -d $(DOM) passwd.byuid; \
			$(ECHO) "pushed passwd"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/passwd"; \
	fi

$(YPDBDIR)/$(DOM)/group.time: $(DIR)/group
	-@if [ -f $(DIR)/group ]; then \
		$(AWK) 'BEGIN { FS=":"; OFS="\t"; } { print $$1, $$0 }' \
		    $(DIR)/group | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/group.byname; \
		$(AWK) 'BEGIN { FS=":"; OFS="\t"; } { printf("%-10d ", $$3); print $$0 }' \
		    $(DIR)/group | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/group.bygid; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/group.time; \
		$(ECHO) "updated group"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) group.byname; \
			$(YPPUSH) -d $(DOM) group.bygid; \
			$(ECHO) "pushed group"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/group"; \
	fi

$(YPDBDIR)/$(DOM)/hosts.time: $(DIR)/hosts
	-@if [ -f $(DIR)/hosts ]; then \
		$(SED) -e "/^#/d" -e s/#.*$$// $(DIR)/hosts | $(STDHOSTS) \
		    | $(AWK) '{for (i = 2; i <= NF; i++) print $$i, $$0}' \
		    | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/hosts.byname; \
		$(STDHOSTS) $(DIR)/hosts | \
		    $(AWK) 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$1, $$0 }' \
		    | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/hosts.byaddr; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/hosts.time; \
		$(ECHO) "updated hosts"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) hosts.byname; \
			$(YPPUSH) -d $(DOM) hosts.byaddr; \
			$(ECHO) "pushed hosts"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/hosts"; \
	fi

$(YPDBDIR)/$(DOM)/networks.time: $(DIR)/networks
	-@if [ -f $(DIR)/networks ]; then \
		$(SED) -e "/^#/d" -e s/#.*$$// $(DIR)/networks | $(AWK) \
		    '{print $$1, $$0; for (i = 3;i <= NF;i++) print $$i,$$0}' \
		    | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/networks.byname; \
		$(AWK) 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		   $(DIR)/networks | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/networks.byaddr;\
		$(TOUCH) $(YPDBDIR)/$(DOM)/networks.time; \
		$(ECHO) "updated networks"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) networks.byname; \
			$(YPPUSH) -d $(DOM) networks.byaddr; \
			$(ECHO) "pushed networks"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/networks"; \
	fi

$(YPDBDIR)/$(DOM)/services.time: $(DIR)/services
	-@if [ -f $(DIR)/services ]; then \
		$(AWK) 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/services| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/services.byname;\
		$(TOUCH) $(YPDBDIR)/$(DOM)/services.time; \
		$(ECHO) "updated services"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) services.byname; \
			$(ECHO) "pushed services"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/services"; \
	fi

$(YPDBDIR)/$(DOM)/rpc.time: $(DIR)/rpc
	-@if [ -f $(DIR)/rpc ]; then \
		$(AWK) 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/rpc| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/rpc.bynumber;\
		$(TOUCH) $(YPDBDIR)/$(DOM)/rpc.time; \
		$(ECHO) "updated rpc"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) rpc.bynumber; \
			$(ECHO) "pushed rpc"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/rpc"; \
	fi

$(YPDBDIR)/$(DOM)/protocols.time: $(DIR)/protocols
	-@if [ -f $(DIR)/protocols ]; then \
		$(AWK) 'BEGIN { OFS="\t"; } $$1 !~ /^#/ { print $$2, $$0 }' \
		    $(DIR)/protocols | $(MAKEDBM) - \
		    $(YPDBDIR)/$(DOM)/protocols.bynumber; \
		$(SED) -e "/^#/d" -e s/#.*$$// $(DIR)/protocols | $(AWK) \
		    '{print $$1,$$0; for (i = 3;i <= NF;i++) print $$i, $$0}' \
		    | $(MAKEDBM) - $(YPDBDIR)/$(DOM)/protocols.byname; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/protocols.time; \
		$(ECHO) "updated protocols"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) protocols.byname; \
			$(YPPUSH) -d $(DOM) protocols.bynumber; \
			$(ECHO) "pushed protocols"; \
			else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/protocols"; \
	fi

$(YPDBDIR)/$(DOM)/netgroup.time: $(DIR)/netgroup
	-@if [ -f $(DIR)/netgroup ]; then \
		$(MAKEDBM) $(DIR)/netgroup $(YPDBDIR)/$(DOM)/netgroup; \
		$(REVNETGROUP) -u < $(DIR)/netgroup \
		| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/netgroup.byuser; \
		$(REVNETGROUP) -h < $(DIR)/netgroup \
		| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/netgroup.byhost; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/netgroup.time; \
		$(ECHO) "updated netgroup"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) netgroup; \
			$(YPPUSH) -d $(DOM) netgroup.byuser; \
			$(YPPUSH) -d $(DOM) netgroup.byhost; \
			$(ECHO) "pushed netgroup"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(DIR)/netgroup"; \
	fi

$(YPDBDIR)/$(DOM)/aliases.time: $(ALIASES)
	-@if [ -f $(ALIASES) ]; then \
		$(SED) -e '/^[ 	]*#/d' -e 's/^[	 ][ 	]*/ /' $(ALIASES) \
		| $(AWK) '/^[^ 	]*:/ { printf "\n"} {print}  END {print}' \
		| $(SED) -e '/^[^ 	]*:/s/:/ /' -e '/[^ 	].*[^\\]$$/s/$$/\\/' \
		| $(MAKEDBM) - $(YPDBDIR)/$(DOM)/mail.aliases; \
		$(TOUCH) $(YPDBDIR)/$(DOM)/aliases.time; \
		$(ECHO) "updated aliases"; \
		if [ ! $(NOPUSH) ]; then \
			$(YPPUSH) -d $(DOM) mail.aliases; \
			$(ECHO) "pushed aliases"; \
		else \
			: ; \
		fi \
	else \
		$(ECHO) "couldn't find $(ALIASES)"; \
	fi

# Added for automount deamon
#$(YPDBDIR)/$(DOM)/auto.home.time: $(DIR)/auto.home
#	 -@if [ -f $(DIR)/auto.home ]; then \
#		$(SED) -e "/^#/d" -e s/#.*$$// $(DIR)/auto.home | \
#		$(MAKEDBM) - $(YPDBDIR)/$(DOM)/auto.home; \
#		$(TOUCH) $(YPDBDIR)/$(DOM)/auto.home.time; \
#		$(ECHO) "updated auto.home"; \
#		if [ ! $(NOPUSH) ]; then \
#			$(YPPUSH) auto.home; \
#			$(ECHO) "pushed auto.home"; \
#		else \
#			: ; \
#		fi \
#	else \
#		$(ECHO) "couldn't find $(DIR)/auto.home"; \
#	fi
#
#$(YPDBDIR)/$(DOM)/auto.master.time: $(DIR)/auto.master
#	 -@if [ -f $(DIR)/auto.master ]; then \
#              $(SED) -e "/^#/d" -e s/#.*$$// $(DIR)/auto.master | \
#		$(MAKEDBM) - $(YPDBDIR)/$(DOM)/auto.master; \
#		$(TOUCH) $(YPDBDIR)/$(DOM)/auto.master.time; \
#		$(ECHO) "updated auto.master"; \
#		if [ ! $(NOPUSH) ]; then \
#			$(YPPUSH) auto.master; \
#			$(ECHO) "pushed auto.master"; \
#		else \
#			: ; \
#		fi \
#	else \
#		$(ECHO) "couldn't find $(DIR)/auto.master"; \
#	fi
passwd: $(YPDBDIR)/$(DOM)/passwd.time
group: $(YPDBDIR)/$(DOM)/group.time
hosts: $(YPDBDIR)/$(DOM)/hosts.time
networks: $(YPDBDIR)/$(DOM)/networks.time
rpc: $(YPDBDIR)/$(DOM)/rpc.time
services: $(YPDBDIR)/$(DOM)/services.time
protocols: $(YPDBDIR)/$(DOM)/protocols.time
netgroup: $(YPDBDIR)/$(DOM)/netgroup.time
aliases: $(YPDBDIR)/$(DOM)/aliases.time
#auto.home: $(YPDBDIR)/$(DOM)/auto.home.time
#auto.master: $(YPDBDIR)/$(DOM)/auto.master.time
$(DIR)/passwd:
$(DIR)/group:
$(DIR)/hosts:
$(DIR)/networks:
$(DIR)/rpc:
$(DIR)/services:
$(DIR)/protocols:
$(DIR)/netgroup:
#$(DIR)/auto.home:
#$(DIR)/auto.master:
$(ALIASES):
